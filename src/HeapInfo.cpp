#include "mymalloc.h"
#include "ghash.h"
#include "HeapInfo.h"
#include "ChunkInfo.h"
#include "SpecialAllocator.h"
#include <pthread.h>
#include <android/log.h>


struct HeapInfoImpl {
    GHashTable* m_infoMap;
};

static HeapInfoImpl* g_impl = NULL;

static guint keyHash(gconstpointer key)
{
    return (guint)key;
}

static gboolean keyEqual(gconstpointer a,
    gconstpointer b)
{
    return a == b;
}

static void valueRelease(gpointer data)
{
    myfree(data);
}

struct HashWalkCB
{
    HeapInfo::pfn_walk walk;
    void* data;
};

static void hashWalk(gpointer key,
    gpointer value,
    gpointer user_data)
{
    HashWalkCB* hwcb = static_cast<HashWalkCB*>(user_data);
    void* addr = key;
    size_t len = static_cast<ChunkInfo*>(value)->m_chunkSize;

    len = (len + 3) & ~3;

    hwcb->walk(addr, len, addr, len, hwcb->data);
}

void HeapInfo::init(int dataSize)
{
    void* storage = mymalloc(sizeof(HeapInfoImpl));
    g_impl = reinterpret_cast<HeapInfoImpl*>(storage);
    g_impl->m_infoMap = g_hash_table_new_full(keyHash, keyEqual, NULL, valueRelease);
}

void HeapInfo::registerChunkInfo(const void* dataPointer, ChunkInfo const& info)
{
    if (dataPointer == NULL || g_impl == NULL) {
        return;
    }
    ChunkInfo* value = static_cast<ChunkInfo*>(g_memdup(&info, sizeof(info)));
    g_hash_table_insert(g_impl->m_infoMap, const_cast<gpointer>(dataPointer), value);
}

void HeapInfo::unregisterChunkInfo(const void* dataPointer)
{
    if (dataPointer == NULL || g_impl == NULL) {
        return;
    }
    g_hash_table_remove(g_impl->m_infoMap, dataPointer);
}

ChunkInfo const* HeapInfo::getChunkInfo(const void* dataPointer)
{
    return static_cast<ChunkInfo*>(g_hash_table_lookup(g_impl->m_infoMap, dataPointer));
}

void HeapInfo::walk(HeapInfo::pfn_walk walk, void* data)
{
    struct HashWalkCB hwcb = { walk, data };
    g_hash_table_foreach(g_impl->m_infoMap, hashWalk, &hwcb);
}

static pthread_mutex_t s_restoreMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

static int s_lockCount = 0;

void HeapInfo::lockHeapInfo()
{
    pthread_mutex_lock(&s_restoreMutex);
    s_lockCount++;
}

void HeapInfo::unlockHeapInfo()
{
    s_lockCount--;
    pthread_mutex_unlock(&s_restoreMutex);
}

bool HeapInfo::isCurrentThreadLockedRecursive()
{
    return s_lockCount > 1;
}

void* g_memdup(const void* src, size_t s)
{
    void* dst = mymalloc(s);
    memcpy(dst, src, s);
    return dst;
}

void
g_free (gpointer mem)
{
    myfree(mem);
}

#define SIZE_OVERFLOWS(a,b) (G_UNLIKELY ((b) > 0 && (a) > G_MAXSIZE / (b)))
#define G_MAXSIZE G_MAXULONG
#define G_MAXULONG ULONG_MAX
#define G_GSIZE_FORMAT "u"
#define G_STRLOC __FILE__ ":" G_STRINGIFY (__LINE__)

gpointer
g_malloc_n (gsize n_blocks,
	    gsize n_block_bytes)
{
  if (SIZE_OVERFLOWS (n_blocks, n_block_bytes))
    {
      __android_log_print(ANDROID_LOG_ERROR, "memanalyze", "%s: overflow allocating %" G_GSIZE_FORMAT "*%" G_GSIZE_FORMAT " bytes",
               G_STRLOC, n_blocks, n_block_bytes);
      __builtin_unreachable();
    }

  return mycalloc (n_blocks, n_block_bytes);
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>

#include "ChunkInfo.h"
#include "dlmalloc.h"
#include "HeapInfo.h"
#include "HeapServer.h"
#include "HeapSnapshotHandler.h"
#include "LightSnapshotHandler.h"

typedef uint32_t uptr;
#define SIZE_T_SIZE (sizeof(size_t))
#define TWO_SIZE_T_SIZES (SIZE_T_SIZE << 1)
struct malloc_chunk {
    size_t prev_foot; /* Size of previous chunk (if free).  */
    size_t head; /* Size and inuse bits. */
    struct malloc_chunk* fd; /* double links -- used only if free. */
    struct malloc_chunk* bk;
};
typedef struct malloc_chunk* m_chunkptr;
#define chunksize(p) ((p)->head & ~(INUSE_BITS))
#define mem2chunk(mem) ((m_chunkptr)((char*)(mem)-TWO_SIZE_T_SIZES))

struct MallocDebug {
    void* (*malloc)(uptr bytes);
    void (*free)(void* mem);
    void* (*calloc)(uptr n_elements, uptr elem_size);
    void* (*realloc)(void* oldMem, uptr bytes);
    void* (*memalign)(uptr alignment, uptr bytes);
};

#define WRAP(x) _##x

static void* _malloc(uptr bytes);
static void _free(void* data);
static void* _calloc(uptr n_elements, uptr elem_size);
static void* _realloc(void* oldMem, uptr bytes);
static void* _memalign(uptr alignment, uptr bytes);
const MallocDebug _malloc_dispatch __attribute__((aligned(32))) = {
    WRAP(malloc), WRAP(free), WRAP(calloc), WRAP(realloc), WRAP(memalign)
};
#undef WRAP

#define WRAP(x) dl##x
const MallocDebug _dlmalloc_dispatch __attribute__((aligned(32))) = {
    WRAP(malloc), WRAP(free), WRAP(calloc), WRAP(realloc), WRAP(memalign)
};

static void overrideMalloc()
{
    extern const MallocDebug* __libc_malloc_dispatch;

    __libc_malloc_dispatch = &_malloc_dispatch;
}

static pthread_mutex_t restoreMutex = PTHREAD_MUTEX_INITIALIZER;

static void restoreMalloc()
{
    extern const MallocDebug* __libc_malloc_dispatch;

    __libc_malloc_dispatch = &_dlmalloc_dispatch;
}

static void* _malloc(uptr bytes)
{
    void* data = dlmalloc(bytes);
    if (!data) {
        return data;
    }
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    void* chunkaddr = reinterpret_cast<void*>(mem2chunk(data));
    ChunkInfo info;
    ChunkInfo::get(info, chunkaddr);
    HeapInfo::registerChunkInfo((void*)chunkaddr, info);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
    return data;
}

static void _free(void* data)
{
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    HeapInfo::unregisterChunkInfo((void*)mem2chunk(data));
    dlfree(data);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
}

static void* _calloc(uptr n_elements, uptr elem_size)
{
    void* data = dlcalloc(n_elements, elem_size);
    if (!data) {
        return data;
    }
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    void* chunkaddr = reinterpret_cast<void*>(mem2chunk(data));
    ChunkInfo info;
    ChunkInfo::get(info, chunkaddr);
    HeapInfo::registerChunkInfo((void*)chunkaddr, info);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
    return data;
}

static void* _realloc(void* oldMem, uptr bytes)
{
    void* newMem = dlrealloc(oldMem, bytes);
    if (newMem) {
        pthread_mutex_lock(&restoreMutex);
        restoreMalloc();
        HeapInfo::unregisterChunkInfo((void*)mem2chunk(oldMem));
        void* data = newMem;
        void* chunkaddr = reinterpret_cast<void*>(mem2chunk(data));
        ChunkInfo info;
        ChunkInfo::get(info, chunkaddr);
        HeapInfo::registerChunkInfo((void*)chunkaddr, info);
        overrideMalloc();
        pthread_mutex_unlock(&restoreMutex);
    }
    return newMem;
}

static void* _memalign(uptr alignment, uptr bytes)
{
    void* data = dlmemalign(alignment, bytes);
    if (!data) {
        return data;
    }
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    void* chunkaddr = reinterpret_cast<void*>(mem2chunk(data));
    ChunkInfo info;
    ChunkInfo::get(info, chunkaddr);
    HeapInfo::registerChunkInfo((void*)chunkaddr, info);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
    return data;
}

class Constructor {
public:
    Constructor()
    {
        HeapInfo::init(64 * (1 << 20));
        overrideMalloc();
        BrowserShell::registerClient(new HeapSnapshotHandler());
        BrowserShell::registerClient(new LightSnapshotHandler());
        BrowserShell::startServer();
    }

private:
    // static void resetStdIo(void)
    // {
    //     close(1);
    //     close(2);
    //     int fd = open("/sdcard/stdio",O_WRONLY | O_CREAT,0666);
    //     dup2(fd,1);
    //     dup2(fd,2);
    // }
};

static Constructor g_con;

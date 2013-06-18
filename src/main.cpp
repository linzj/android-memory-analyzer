#include <stdint.h>
#include "ChunkInfo.h"
#include "dlmalloc.h"
#include "HeapInfo.h"
#include "HeapServer.h"

typedef uint32_t uptr;
#define SIZE_T_SIZE         (sizeof(size_t))
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
struct malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               head;       /* Size and inuse bits. */
  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;
};
typedef struct malloc_chunk* m_chunkptr;
#define chunksize(p)        ((p)->head & ~(INUSE_BITS))
#define mem2chunk(mem)      ((m_chunkptr)((char*)(mem) - TWO_SIZE_T_SIZES))

struct MallocDebug {
  void* (*malloc)(uptr bytes);
  void  (*free)(void* mem);
  void* (*calloc)(uptr n_elements, uptr elem_size);
  void* (*realloc)(void* oldMem, uptr bytes);
  void* (*memalign)(uptr alignment, uptr bytes);
};

static void* _malloc(uptr bytes)
{
    void * data = dlmalloc(bytes);
    if(!data)
    {
        return data;
    }
    void * chunkaddr= reinterpret_cast<void*>(mem2chunk(data));
    ChunkInfo info;
    ChunkInfo::get(info,chunkaddr);
    HeapInfo::registerChunkInfo((void*)chunkaddr,info);
    return data;
}

static void  _free(void* data)
{
    HeapInfo::unregisterChunkInfo((void*)mem2chunk(data));
    dlfree(data);
}
static void* _calloc(uptr n_elements, uptr elem_size)
{
    void * data = dlcalloc(n_elements,elem_size);
    if(!data)
    {
        return data;
    }
    void * chunkaddr= reinterpret_cast<void*>(mem2chunk(data));
    ChunkInfo info;
    ChunkInfo::get(info,chunkaddr);
    HeapInfo::registerChunkInfo((void*)chunkaddr,info);
    return data;
}

static void* _realloc(void* oldMem, uptr bytes)
{
    void * newMem = dlrealloc(oldMem,bytes);
    if(newMem)
    {
        HeapInfo::unregisterChunkInfo((void*)mem2chunk(oldMem));
        void * data = newMem;
        void * chunkaddr= reinterpret_cast<void*>(mem2chunk(data));
        ChunkInfo info;
        ChunkInfo::get(info,chunkaddr);
        HeapInfo::registerChunkInfo((void*)chunkaddr,info);
    }
    return newMem;
}

static void* _memalign(uptr alignment, uptr bytes)
{
    void * data = dlmemalign(alignment,bytes);
    if(!data)
    {
        return data;
    }
    void * chunkaddr= reinterpret_cast<void*>(mem2chunk(data));
    ChunkInfo info;
    ChunkInfo::get(info,chunkaddr);
    HeapInfo::registerChunkInfo((void*)chunkaddr,info);
    return data;
}

#define WRAP(x) _##x

const MallocDebug _malloc_dispatch __attribute__((aligned(32))) = {
  WRAP(malloc), WRAP(free), WRAP(calloc), WRAP(realloc), WRAP(memalign)
};

class Constructor
{
public:
    Constructor()
    {
        HeapInfo::init(64 * (1 << 20));
        overrideMalloc();
        BrowserShell::startServer();
        dlmalloc(12);
        dlmalloc(14);
    }
private:
    static void overrideMalloc()
    {
        extern const MallocDebug* __libc_malloc_dispatch;

        __libc_malloc_dispatch = &_malloc_dispatch;
    }
};


static Constructor g_con;

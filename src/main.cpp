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

struct MallocDebug {
  void* (*malloc)(uptr bytes);
  void  (*free)(void* mem);
  void* (*calloc)(uptr n_elements, uptr elem_size);
  void* (*realloc)(void* oldMem, uptr bytes);
  void* (*memalign)(uptr alignment, uptr bytes);
};

#define WRAP(x) _##x

static void* _malloc(uptr bytes);
static void  _free(void* data);
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
    void * data = dlmalloc(bytes);
    if(!data)
    {
        return data;
    }
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    void * chunkaddr= reinterpret_cast<void*>(data);
    ChunkInfo info;
    ChunkInfo::get(info,chunkaddr,bytes);
    HeapInfo::registerChunkInfo((void*)chunkaddr,info);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
    return data;
}

static void  _free(void* data)
{
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    HeapInfo::unregisterChunkInfo((void*)data);
    dlfree(data);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
}

static void* _calloc(uptr n_elements, uptr elem_size)
{
    void * data = dlcalloc(n_elements,elem_size);
    if(!data)
    {
        return data;
    }
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    void * chunkaddr= reinterpret_cast<void*>(data);
    ChunkInfo info;
    ChunkInfo::get(info,chunkaddr,n_elements * elem_size);
    HeapInfo::registerChunkInfo((void*)chunkaddr,info);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
    return data;
}

static void* _realloc(void* oldMem, uptr bytes)
{
    void * newMem = dlrealloc(oldMem,bytes);
    if(newMem)
    {
        pthread_mutex_lock(&restoreMutex);
        restoreMalloc();
        HeapInfo::unregisterChunkInfo((void*)oldMem);
        void * data = newMem;
        void * chunkaddr= reinterpret_cast<void*>(data);
        ChunkInfo info;
        ChunkInfo::get(info,chunkaddr,bytes);
        HeapInfo::registerChunkInfo((void*)chunkaddr,info);
        overrideMalloc();
        pthread_mutex_unlock(&restoreMutex);
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
    pthread_mutex_lock(&restoreMutex);
    restoreMalloc();
    void * chunkaddr= reinterpret_cast<void*>(data);
    ChunkInfo info;
#define ROUND_UP(n, sz) (((n) + ((sz) - 1)) & ~((sz) - 1))
    ChunkInfo::get(info,chunkaddr,ROUND_UP(bytes,alignment));
    HeapInfo::registerChunkInfo((void*)chunkaddr,info);
    overrideMalloc();
    pthread_mutex_unlock(&restoreMutex);
    return data;
}


class Constructor
{
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

};


static Constructor g_con;

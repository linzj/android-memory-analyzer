#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <new>

#include "ChunkInfo.h"
#include "dlmalloc.h"
#include "HeapInfo.h"
#include "HeapServer.h"
#include "HeapSnapshotHandler.h"
#include "LightSnapshotHandler.h"

typedef uint32_t uptr;
typedef size_t (*MallocDebugMallocUsableSize)(void*);
struct MallocDebug {
    void* (*malloc)(uptr bytes);
    void (*free)(void* mem);
    void* (*calloc)(uptr n_elements, uptr elem_size);
    void* (*realloc)(void* oldMem, uptr bytes);
    void* (*memalign)(uptr alignment, uptr bytes);
    MallocDebugMallocUsableSize malloc_usable_size;
};

#define WRAP(x) _##x

static void* _malloc(uptr bytes);
static void _free(void* data);
static void* _calloc(uptr n_elements, uptr elem_size);
static void* _realloc(void* oldMem, uptr bytes);
static void* _memalign(uptr alignment, uptr bytes);
static size_t _malloc_usable_size(void*);
const MallocDebug _malloc_dispatch __attribute__((aligned(32))) = {
    WRAP(malloc), WRAP(free), WRAP(calloc), WRAP(realloc), WRAP(memalign), WRAP(malloc_usable_size)
};
#undef WRAP

#define WRAP(x) dl##x
const MallocDebug _dlmalloc_dispatch __attribute__((aligned(32))) = {
    WRAP(malloc), WRAP(free), WRAP(calloc), WRAP(realloc), WRAP(memalign), WRAP(malloc_usable_size)
};

static void overrideMalloc()
{
    extern const MallocDebug* __libc_malloc_dispatch;

    __libc_malloc_dispatch = &_malloc_dispatch;
}

static void* _malloc(uptr bytes)
{
    void* data = dlmalloc(bytes);
    if (!data) {
        return data;
    }
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        ChunkInfo info;
        ChunkInfo::get(info, data);
        info.m_chunkSize = bytes;
        HeapInfo::registerChunkInfo((void*)data, info);
    }
    HeapInfo::unlockHeapInfo();
    return data;
}

static void _free(void* data)
{
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        HeapInfo::unregisterChunkInfo(data);
    }
    HeapInfo::unlockHeapInfo();
    dlfree(data);
}

static void* _calloc(uptr n_elements, uptr elem_size)
{
    void* data = dlcalloc(n_elements, elem_size);
    if (!data) {
        return data;
    }
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        ChunkInfo info;
        ChunkInfo::get(info, data);
        info.m_chunkSize = n_elements * elem_size;

        HeapInfo::registerChunkInfo((void*)data, info);
    }
    HeapInfo::unlockHeapInfo();
    return data;
}

static void* _realloc(void* oldMem, uptr bytes)
{
    void* newMem = dlrealloc(oldMem, bytes);
    if (newMem) {
        HeapInfo::lockHeapInfo();
        if (!HeapInfo::isCurrentThreadLockedRecursive()) {
            HeapInfo::unregisterChunkInfo(oldMem);
            void* data = newMem;
            ChunkInfo info;
            ChunkInfo::get(info, data);
            info.m_chunkSize = bytes;
            HeapInfo::registerChunkInfo(data, info);
        }
        HeapInfo::unlockHeapInfo();
    }
    return newMem;
}

static void* _memalign(uptr alignment, uptr bytes)
{
    void* data = dlmemalign(alignment, bytes);
    if (!data) {
        return data;
    }
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        ChunkInfo info;
        ChunkInfo::get(info, data);
        info.m_chunkSize = bytes;
        HeapInfo::registerChunkInfo((void*)data, info);
    }
    HeapInfo::unlockHeapInfo();
    return data;
}

static size_t _malloc_usable_size(void* ptr)
{
    return dlmalloc_usable_size(ptr);
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

void* operator new (std::size_t s)
{
    return malloc(s);
}

void operator delete (void* p)
{
    return free(p);
}

void* operator new [](std::size_t s)
{
    return malloc(s);
}

void operator delete [](void* p)
{
    return free(p);
}

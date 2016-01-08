#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <new>
#include <stdlib.h>

#include "ChunkInfo.h"
#include "dlmalloc.h"
#include "HeapInfo.h"
#include "HeapServer.h"
#include "HeapSnapshotHandler.h"
#include "LightSnapshotHandler.h"

typedef uint32_t uptr;

#define WRAP(x) _##x

void* malloc(uptr bytes)
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

void free(void* data)
{
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        HeapInfo::unregisterChunkInfo(data);
    }
    HeapInfo::unlockHeapInfo();
    dlfree(data);
}

void* calloc(uptr n_elements, uptr elem_size)
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

void* realloc(void* oldMem, uptr bytes)
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
extern "C" {
    __attribute__((visibility("default"))) void* memalign(uptr alignment, uptr bytes);
    __attribute__((visibility("default"))) size_t malloc_usable_size(const void* ptr);
}

void* memalign(uptr alignment, uptr bytes)
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

size_t malloc_usable_size(const void* ptr)
{
    return dlmalloc_usable_size(ptr);
}

class Constructor {
public:
    Constructor()
    {
        HeapInfo::init(64 * (1 << 20));
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

void operator delete (void* p) throw()
{
    return free(p);
}

void* operator new [](std::size_t s)
{
    return malloc(s);
}

void operator delete [](void* p) throw()
{
    return free(p);
}

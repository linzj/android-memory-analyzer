#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <new>

#include "ChunkInfo.h"
#include "HeapInfo.h"
#include "HeapServer.h"
#include "HeapSnapshotHandler.h"
#include "LightSnapshotHandler.h"
#include "mymalloc.h"
# define likely(x)	__builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)
#define MAX_ERRNO 4095

static void* do_malloc(uptr bytes)
{
    HeapInfo::lockHeapInfo();
    void* data = mymalloc(bytes);
    if (!data) {
        goto exit;
    }
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        ChunkInfo info;
        ChunkInfo::get(info, data);
        info.m_chunkSize = bytes;
        HeapInfo::registerChunkInfo((void*)data, info);
    }
exit:
    HeapInfo::unlockHeapInfo();
    return data;
}

static void do_free(void* data)
{
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        HeapInfo::unregisterChunkInfo(data);
    }
    HeapInfo::unlockHeapInfo();
    myfree(data);
}

static void* do_calloc(uptr n_elements, uptr elem_size)
{
    HeapInfo::lockHeapInfo();
    void* data = mycalloc(n_elements, elem_size);
    if (!data) {
        goto exit;
    }
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        ChunkInfo info;
        ChunkInfo::get(info, data);
        info.m_chunkSize = n_elements * elem_size;

        HeapInfo::registerChunkInfo((void*)data, info);
    }
exit:
    HeapInfo::unlockHeapInfo();
    return data;
}

static void* do_realloc(void* oldMem, uptr bytes)
{
    HeapInfo::lockHeapInfo();
    void* newMem = myrealloc(oldMem, bytes);
    if (newMem) {
        if (!HeapInfo::isCurrentThreadLockedRecursive()) {
            HeapInfo::unregisterChunkInfo(oldMem);
            void* data = newMem;
            ChunkInfo info;
            ChunkInfo::get(info, data);
            info.m_chunkSize = bytes;
            HeapInfo::registerChunkInfo(data, info);
        }
    }
    HeapInfo::unlockHeapInfo();
    return newMem;
}
extern "C" {
    __attribute__((visibility("default"))) void* memalign(uptr alignment, uptr bytes);
    __attribute__((visibility("default"))) size_t malloc_usable_size(const void* ptr);
}

static void* do_memalign(uptr alignment, uptr bytes)
{
    HeapInfo::lockHeapInfo();
    void* data = mymemalign(alignment, bytes);
    if (!data) {
        goto exit;
    }
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        ChunkInfo info;
        ChunkInfo::get(info, data);
        info.m_chunkSize = bytes;
        HeapInfo::registerChunkInfo((void*)data, info);
    }
    HeapInfo::unlockHeapInfo();
exit:
    return data;
}

static size_t do_malloc_usable_size(const void* ptr)
{
    return mymalloc_usable_size(ptr);
}

static void* do_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    void* data = mymmap(addr, length, prot, flags, fd, offset);
    if (data == MAP_FAILED) {
        return data;
    }
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        ChunkInfo info;
        ChunkInfo::get(info, data);
        info.m_flags = ChunkInfo::MMAP;
        char* myaddr = static_cast<char*>(data);

        // register for each page.
        for (ssize_t mylength = length; mylength > 0;
                mylength -= PAGE_SIZE, myaddr += PAGE_SIZE) {
            info.m_chunkSize = PAGE_SIZE;
            HeapInfo::registerChunkInfo(myaddr, info);
        }
    }
    HeapInfo::unlockHeapInfo();
    errno = 0;
    return data;
}

static int do_munmap(void* addr, size_t length)
{
    HeapInfo::lockHeapInfo();
    if (!HeapInfo::isCurrentThreadLockedRecursive()) {
        char* myaddr = static_cast<char*>(addr);

        // unregister for each page.
        for (ssize_t mylength = length; mylength > 0;
                mylength -= PAGE_SIZE, myaddr += PAGE_SIZE) {
            HeapInfo::unregisterChunkInfo(myaddr);
        }
    }
    HeapInfo::unlockHeapInfo();
    return mymunmap(addr, length);
}

static void* do_pre_malloc(uptr bytes)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    return mymalloc(bytes);
}

static void do_pre_free(void* data)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    myfree(data);
}

static void* do_pre_realloc(void* oldMem, uptr bytes)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    return myrealloc(oldMem, bytes);
}

static void* do_pre_memalign(uptr alignment, uptr bytes)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    return mymemalign(alignment, bytes);
}

static size_t do_pre_malloc_usable_size(const void* ptr)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    return mymalloc_usable_size(ptr);
}

static void* do_pre_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    return mymmap(addr, length, prot, flags, fd, offset);
}

static int do_pre_munmap(void* addr, size_t length)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    return mymunmap(addr, length);
}

static void* do_pre_calloc(uptr n_elements, uptr elem_size)
{
    if (unlikely(!mymalloc)) {
        initMyMalloc();
    }
    return mycalloc(n_elements, elem_size);
}

typedef void* (*pfnmmap)(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
typedef int (*pfnmunmap)(void* addr, size_t length);

static pfnmalloc handlemalloc = do_pre_malloc;
static pfnfree handlefree = do_pre_free;
static pfncalloc handlecalloc = do_pre_calloc;
static pfnrealloc handlerealloc = do_pre_realloc;
static pfnmemalign handlememalign = do_pre_memalign;
static pfnmalloc_usable_size handlemalloc_usable_size = do_pre_malloc_usable_size;
static pfnmmap handlemmap = do_pre_mmap;
static pfnmunmap handlemunmap = do_pre_munmap;


void* malloc(uptr bytes)
{
    return handlemalloc(bytes);
}

void free(void* data)
{
    handlefree(data);
}

void* realloc(void* oldMem, uptr bytes)
{
    return handlerealloc(oldMem, bytes);
}

void* memalign(uptr alignment, uptr bytes)
{
    return handlememalign(alignment, bytes);
}

size_t malloc_usable_size(const void* ptr)
{
    return handlemalloc_usable_size(ptr);
}

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return handlemmap(addr, length, prot, flags, fd, offset);
}

int munmap(void* addr, size_t length)
{
    return handlemunmap(addr, length);
}

void* calloc(uptr n_elements, uptr elem_size)
{
    return handlecalloc(n_elements, elem_size);
}


class Constructor {
public:
    Constructor()
    {
        if (unlikely(!mymalloc)) {
            initMyMalloc();
        }
        HeapInfo::init(64 * (1 << 20));
        BrowserShell::registerClient(new HeapSnapshotHandler());
        BrowserShell::registerClient(new LightSnapshotHandler());
        BrowserShell::startServer();
        handlemalloc = do_malloc;
        handlefree = do_free;
        handlecalloc = do_calloc;
        handlerealloc = do_realloc;
        handlememalign = do_memalign;
        handlemalloc_usable_size = do_malloc_usable_size;
        handlemmap = do_mmap;
        handlemunmap = do_munmap;
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

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#include "DumpHeap.h"
#include "ChunkInfo.h"
#include "HeapInfo.h"
#include "HeapServer.h"

DumpHeap::DumpHeap(int fd, bool sendUserData)
    : fd_(fd)
    , sendUserData_(sendUserData)
{
}

struct SendOnce {
    const void* m_chunk;
    uint32_t m_len;
    uint32_t m_backtracesLen;
    uint32_t m_dataAttrib; // for this data's attributes
    const void* m_backtraces[ChunkInfo::MAX_BACKTRACES];
};

static bool checkMMapRegion(const void* userptr, size_t userlen)
{
    int r;
    unsigned char buf[(userlen + (PAGE_SIZE - 1)) / PAGE_SIZE];
    r = mincore(const_cast<void*>(userptr), userlen, buf);
    return r == 0 && buf[0] != 0;
}

void DumpHeap::notifyChunk(const void* userptr, size_t userlen)
{
    if (!userptr) {
        return;
    }
    SendOnce once = { userptr, userlen };
    const ChunkInfo* info = HeapInfo::getChunkInfo(userptr);
    bool isMMap = info->m_flags & ChunkInfo::MMAP;
    // check if this mmap region is available.
    if (isMMap && !checkMMapRegion(userptr, userlen)) {
        return;
    }
    size_t sendSize = offsetof(SendOnce, m_backtraces);
    if (info && info->m_backtracesLen) {
        once.m_backtracesLen = info->m_backtracesLen;
        sendSize += once.m_backtracesLen * sizeof(void*);
        for (int i = 0; i < once.m_backtracesLen; ++i) {
            once.m_backtraces[i] = info->m_backtraces[i];
        }
    } else {
        once.m_backtracesLen = 0;
    }

    // don't send mmap region user data.
    bool sendUserData = !isMMap && sendUserData_;
    if (sendUserData) {
        once.m_dataAttrib |= DATA_ATTR_USER_CONTENT;
    }

    sendTillEnd(fd_, reinterpret_cast<const char*>(&once), sendSize);

    if (sendUserData) {
        sendTillEnd(fd_, static_cast<const char*>(userptr), userlen);
    }
}

void DumpHeap::callWalk(void)
{
    // dlmalloc_walk_heap(mywalk, this);
    HeapInfo::walk(mywalk, this);
}

void DumpHeap::mywalk(const void* userptr, size_t userlen,
                      void* arg)
{
    DumpHeap* This = reinterpret_cast<DumpHeap*>(arg);
    This->notifyChunk(userptr, userlen);
}

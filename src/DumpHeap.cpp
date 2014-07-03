#include <stddef.h>
#include <stdint.h>
#include "DumpHeap.h"
#include "ChunkInfo.h"
#include "HeapInfo.h"
#include "HeapServer.h"

extern "C" {
void* dlmalloc(size_t);
void dlfree(void*);
void dlmalloc_walk_heap(void (*handler)(const void* chunkptr, size_t chunklen,
                                        const void* userptr, size_t userlen,
                                        void* arg),
                        void* harg);
}

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

void DumpHeap::notifyChunk(const void* chunk, size_t len, const void* userptr, size_t userlen)
{
    if (userptr) {
        SendOnce once = { userptr, userlen };
        const ChunkInfo* info = HeapInfo::getChunkInfo(chunk);
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

        if (sendUserData_) {
            once.m_dataAttrib |= DATA_ATTR_USER_CONTENT;
        }

        sendTillEnd(fd_, reinterpret_cast<const char*>(&once), sendSize);

        if (sendUserData_) {
            sendTillEnd(fd_, static_cast<const char*>(userptr), userlen);
        }
    }
}

void DumpHeap::callWalk(void)
{
    dlmalloc_walk_heap(mywalk, this);
}

void DumpHeap::mywalk(const void* chunkptr, size_t chunklen,
                      const void* userptr, size_t userlen,
                      void* arg)
{
    DumpHeap* This = reinterpret_cast<DumpHeap*>(arg);
    This->notifyChunk(chunkptr, chunklen, userptr, userlen);
}

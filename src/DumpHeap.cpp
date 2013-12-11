#include <stddef.h>
#include <stdint.h>
#include "DumpHeap.h"
#include "ChunkInfo.h"
#include "HeapInfo.h"
#include "HeapServer.h"

extern "C"
{
    void* dlmalloc(size_t);
    void  dlfree(void*);
    void dlmalloc_walk_heap(void(*handler)(const void *chunkptr, size_t chunklen,
                const void *userptr, size_t userlen,
                void *arg),
            void *harg);
}

DumpHeap::DumpHeap(int fd,bool sendUserData) : 
     fd_(fd)
    ,sendUserData_(sendUserData)
{
}

struct SendOnce
{
    const void * m_chunk;
    uint32_t m_len;
    uint32_t m_backtracesLen;
    uint32_t m_dataAttrib; // for this data's attributes
    const void * m_backtraces[ChunkInfo::MAX_BACKTRACES];
};

void DumpHeap::notifyChunk(const void * userptr ,size_t userlen)
{
    SendOnce once = {userptr,userlen};
    const ChunkInfo * info = HeapInfo::getChunkInfo(userptr);
    size_t sendSize = offsetof(SendOnce,m_backtraces);
    if(info && info->m_backtracesLen)
    {
        once.m_backtracesLen = info->m_backtracesLen;
        sendSize += once.m_backtracesLen * sizeof(void*);
        for(int i = 0 ; i < once.m_backtracesLen; ++i)
        {
            once.m_backtraces[i] = info->m_backtraces[i];
        }
    }
    else
    {
        once.m_backtracesLen = 0;
    }

    if(sendUserData_)
    {
        once.m_dataAttrib |= DATA_ATTR_USER_CONTENT;
    }

    sendTillEnd(fd_,reinterpret_cast<const char *>(&once),sendSize);

    if(sendUserData_)
    {
        sendTillEnd(fd_,static_cast<const char *>(userptr),userlen);
        // fill to 4 bytes align
        static const size_t mask = 4 - 1;
        if(userlen & mask)
        {
            // need to send till 4 bytes
            static char fill[3] = {0};
            size_t bytes_to_send = 4 - (userlen & mask);
            sendTillEnd(fd_,fill,bytes_to_send);
        }
    }

}

void DumpHeap::callWalk(void)
{
    HeapInfo::walk(mywalk,this);
}


void DumpHeap::mywalk(const void *userptr, size_t userlen, void *arg)
{
    DumpHeap * This = reinterpret_cast<DumpHeap*>(arg);
    This->notifyChunk(userptr ,userlen);
}


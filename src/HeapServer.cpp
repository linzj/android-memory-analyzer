#include "HeapServer.h"
#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#ifdef LIN_USE_LOG
#include <android/log.h>
#define LINLOG(...) \
    __android_log_print(ANDROID_LOG_DEBUG,"LIN",__VA_ARGS__)
#else
#define LINLOG(...) 
#endif // LIN_USE_LOG
#include "ChunkInfo.h"
#include "HeapInfo.h"

struct mymallinfo {
  size_t arena;    /* non-mmapped space allocated from system */
  size_t ordblks;  /* number of free chunks */
  size_t smblks;   /* always 0 */
  size_t hblks;    /* always 0 */
  size_t hblkhd;   /* space in mmapped regions */
  size_t usmblks;  /* maximum total allocated space */
  size_t fsmblks;  /* always 0 */
  size_t uordblks; /* total allocated space */
  size_t fordblks; /* total free space */
  size_t keepcost; /* releasable (via malloc_trim) space */
};
extern "C"
{
    void* dlmalloc(size_t);
    void  dlfree(void*);
    void dlmalloc_walk_heap(void(*handler)(const void *chunkptr, size_t chunklen,
                const void *userptr, size_t userlen,
                void *arg),
            void *harg);
    mymallinfo dlmallinfo(void);
}


static const  uint16_t s_port = 3244;



namespace BrowserShell {

static int sendTillEnd(int fd,const char * buffer,size_t s)
{
    const char * bufferEnd = buffer + s;
    while(buffer != bufferEnd)
    {
        int byteSend = send(fd,buffer,s,0);
        if(byteSend == -1)
        {
            LINLOG("send Till End failed!errno = %d,sent %d bytes \n",errno,static_cast<int>(bufferEnd - buffer));
            return byteSend;
        }
        s -= byteSend;
        buffer += byteSend;
    }
    return 0;
}

class DumpHeap
{
    public:
        explicit DumpHeap(int fd);

        void callWalk(void);

    private:
        
        void notifyChunk(const void * chunk,size_t len,const void * userptr,size_t userlen);
        static void mywalk(const void *chunkptr, size_t chunklen,
                const void *userptr, size_t userlen,
                void *arg);

    private:
        int m_fd;
};


DumpHeap::DumpHeap(int fd)
    : m_fd(fd)
{
}
struct SendOnce
{
    const void * m_chunk;
    size_t m_len;
    size_t m_backtracesLen;
    const void * m_backtraces[ChunkInfo::MAX_BACKTRACES];
};

void DumpHeap::notifyChunk( const void * chunk,size_t len,const void * userptr ,size_t userlen)
{
    if(userptr)
    {
        SendOnce once = { userptr,userlen};
        const ChunkInfo * info = HeapInfo::getChunkInfo(chunk);
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
        sendTillEnd(m_fd,reinterpret_cast<const char *>(&once),sendSize);
        sendTillEnd(m_fd,static_cast<const char *>(userptr),userlen);
    }
}

void DumpHeap::callWalk(void)
{
    dlmalloc_walk_heap(mywalk,this);
}


void DumpHeap::mywalk(const void *chunkptr, size_t chunklen,
        const void *userptr, size_t userlen,
        void *arg)
{
    DumpHeap * This = reinterpret_cast<DumpHeap*>(arg);
    This->notifyChunk(chunkptr,chunklen,userptr ,userlen);
}



static void handleClient(int fd,struct sockaddr * )
{
    DumpHeap dh(fd);
    dh.callWalk();
    mymallinfo myinfo = dlmallinfo();
    fprintf(stderr,"free space = %f\n",((float)myinfo.fordblks) / 1024.0f);
}
static void * serverFunc(void *)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        LINLOG("socket create failed!errno = %d\n",errno);
        return NULL;
    }
    struct sockaddr_in listenAddrIn4 = { AF_INET,htons(s_port),{INADDR_ANY} } ;
    struct sockaddr * listenAddr = reinterpret_cast<sockaddr *>(&listenAddrIn4);
    const socklen_t sockLen = sizeof(listenAddrIn4);
    if(-1 == bind(sockfd,listenAddr,sockLen))
    {
        LINLOG("bind failed:errno = %d\n",errno);
        close(sockfd);
        return NULL;
    }
    if(-1 == listen(sockfd,1))
    {
        LINLOG("listen failed:errno = %d\n",errno);
        close(sockfd);
        return NULL;
    }
    while(true)
    {
        struct sockaddr_in clientAddr;
        socklen_t  clientAddrLen = sizeof(clientAddr); 
        int clientSockFd = accept(sockfd,reinterpret_cast<struct sockaddr *>(&clientAddr),&clientAddrLen);
        if(-1 == clientSockFd)
        {
            LINLOG("accept failed:errno = %d\n",errno);
            close(sockfd);
            return NULL;
        }
        handleClient(clientSockFd,reinterpret_cast<struct sockaddr *>(&clientAddr));
        shutdown(clientSockFd,SHUT_RDWR);
        close(clientSockFd);
    }
    return NULL;
}

void startServer(void)
{
    int pthread_ret ;
    pthread_t mythread;
    pthread_ret = pthread_create(&mythread,NULL,serverFunc,NULL);
    if(pthread_ret != 0)
    {
        LINLOG("pthread create failed:errno = %d\n",pthread_ret);
        return ;
    }
    pthread_detach(mythread);
}
}
#endif // ENABLE_HEAP_SEVER

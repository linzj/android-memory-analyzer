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
#include "DumpHeap.h"
#include "HeapServer.h"
#include "StopWorld.h"
#include "MapParse.h"

static const  uint16_t s_port = 3244;
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
    mymallinfo dlmallinfo(void);
}

void sendThreadData(int fd,void ** buf,int count,MapParse::MapList const &);
void sendGlobalVariable(int fd,MapParse::MapList const & list);

int sendTillEnd(int fd,const char * buffer,size_t s)
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

namespace BrowserShell {

static void handleClient(int fd,struct sockaddr * )
{
    stopTheWorld();
    DumpHeap dh(fd);
    dh.callWalk();
    void * buf[64];
    int userContextCount = getUserContext(buf,64);
    MapParse::MapList mapList = MapParse::parseFile("/proc/self/maps");
    // send back thread stack
    sendThreadData(fd,buf,userContextCount,mapList);
    // send back global variable area
    sendGlobalVariable(fd,mapList);
    mymallinfo myinfo = dlmallinfo();
    fprintf(stderr,"LIN:free space = %f\n",((float)myinfo.fordblks) / 1024.0f);
    restartTheWorld();
}
static void * serverFunc(void *)
{
    //init thread db
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

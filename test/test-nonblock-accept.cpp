#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include "LinLog.h"

static int createServerSocket(int port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        LINLOG("socket create failed!errno = %d\n",errno);
        return -1;
    }
    struct sockaddr_in listenAddrIn4 = { AF_INET,htons(port),{INADDR_ANY} } ;
    struct sockaddr * listenAddr = reinterpret_cast<sockaddr *>(&listenAddrIn4);
    const socklen_t sockLen = sizeof(listenAddrIn4);
    if(-1 == bind(sockfd,listenAddr,sockLen))
    {
        LINLOG("bind failed:errno = %d\n",errno);
        close(sockfd);
        return -1;
    }
    if(-1 == listen(sockfd,1))
    {
        LINLOG("listen failed:errno = %d\n",errno);
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int main()
{
    int sockfd1,sockfd2;
    sockfd1 = createServerSocket(8233);
    if(sockfd1 == -1)
    {
        LINLOG("failed to create server socket\n");
        return 1;
    }
    sockfd2 = createServerSocket(8234);
    if(sockfd2 == -1)
    {
        LINLOG("failed to create server socket\n");
        close(sockfd1);
        return 1;
    }
    int epollfd = epoll_create(1);
    if(epollfd == -1)
    {
        LINLOG("failed to create epoll\n");
        close(sockfd1);
        close(sockfd2);
        return 1;
    }
    struct epoll_event ev;
    memset(&ev,0,sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = sockfd1;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd1, &ev) == -1) {
        LINLOG("failed to add epoll event to sockfd1 %d\n",errno);
        close(sockfd1);
        close(sockfd2);
        close(epollfd);
        return 1;
    }

    memset(&ev,0,sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = sockfd2;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd2, &ev) == -1) {
        LINLOG("failed to add epoll event sockfd2 %d\n",errno);
        close(sockfd1);
        close(sockfd2);
        close(epollfd);
        return 1;
    }

    for (;;) {
        memset(&ev,0,sizeof(ev));
        int nfds = epoll_wait(epollfd, &ev, 1, -1);
        if (nfds == -1) {
            LINLOG("epoll_wait:%d",errno);
            close(sockfd1);
            close(sockfd2);
            close(epollfd);
            return 1;
        }
        printf("got event from %d in %d,%d\n",ev.data.fd,sockfd1,sockfd2);
        sockaddr_in local;
        socklen_t size = sizeof(local);
        int clientfd = accept(ev.data.fd,(sockaddr*)&local,&size);
        if(clientfd == -1)
        {
            LINLOG("failed to accept:%d\n",errno);
            
            close(sockfd1);
            close(sockfd2);
            close(epollfd);
            return 1;
        }
        close(clientfd);
    }
    close(sockfd1);
    close(sockfd2);
    close(epollfd);
    return 0;
}


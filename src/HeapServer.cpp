#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "LinLog.h"
#include "HeapServer.h"
#include "HeapInfo.h"

static uint16_t s_port = 3244;
static uint16_t s_client_count = 0;
static const int MAX_HANDLER_COUNT = 8;
struct ClientDesc {
    int serverPort_;
    int fd_;
    ClientHandler* handler_;
};
static ClientDesc s_handlers[MAX_HANDLER_COUNT];
static void lockHeapServer()
{
    HeapInfo::lockHeapInfo();
}

static void unlockHeapServer()
{
    HeapInfo::unlockHeapInfo();
}

int sendTillEnd(int fd, const char* buffer, size_t s)
{
    const char* bufferEnd = buffer + s;
    while (buffer != bufferEnd) {
        int byteSend = send(fd, buffer, s, 0);
        if (byteSend == -1) {
            LINLOG("send Till End failed!errno = %d, buffer = %p, sent %d bytes.\n", errno, buffer, static_cast<int>(bufferEnd - buffer));
            return byteSend;
        }
        s -= byteSend;
        buffer += byteSend;
    }
    return 0;
}

class FileOwner {
    mutable int fd_;

public:
    explicit FileOwner(int fd = -1)
        : fd_(fd)
    {
    }
    ~FileOwner()
    {
        reset(-1);
    }

    FileOwner(FileOwner const& lhs)
    {
        this->swap(lhs);
    }

    FileOwner& operator=(FileOwner const& lhs)
    {
        this->swap(lhs);
        return *this;
    }

    void swap(FileOwner const& lhs)
    {
        int tmp = this->fd_;
        this->fd_ = lhs.fd_;
        lhs.fd_ = tmp;
    }

    void reset(int fd) {
        if (fd_ != -1) {
            close(fd_);
        }
        fd_ = fd;
    }

    int release()
    {
        int tmp = fd_;
        fd_ = -1;
        return tmp;
    }
    inline int get() const
    {
        return fd_;
    }
};

class FileOwnerVector
{
public:
    FileOwnerVector() : m_ownerCount(0) {}
    void push_back(FileOwner& fown)
    {
        if (m_ownerCount >= max_owners) {
            LINLOG("error: m_ownerCount >= max_owners.\n");
        }
        FileOwner& location = m_owners[m_ownerCount++];
        location.swap(fown);
    }
private:
    static const int max_owners = 10;
    FileOwner m_owners[max_owners];
    int m_ownerCount;
};

namespace BrowserShell {

static int createServerSocket(int port)
{
    FileOwner sockfd(socket(AF_INET, SOCK_STREAM, 0));
    if (sockfd.get() == -1) {
        LINLOG("socket create failed!errno = %d\n", errno);
        return -1;
    }
    struct sockaddr_in listenAddrIn4 = { AF_INET, htons(port), { INADDR_ANY } };
    struct sockaddr* listenAddr = reinterpret_cast<sockaddr*>(&listenAddrIn4);
    const socklen_t sockLen = sizeof(listenAddrIn4);
    if (-1 == bind(sockfd.get(), listenAddr, sockLen)) {
        LINLOG("bind failed:errno = %d\n", errno);
        return -1;
    }
    if (-1 == listen(sockfd.get(), 1)) {
        LINLOG("listen failed:errno = %d\n", errno);
        return -1;
    }
    return sockfd.release();
}

static void* serverFunc(void*)
{
    FileOwnerVector fv;
    FileOwner epollfd(epoll_create(1));
    if (epollfd.get() == -1) {
        LINLOG("failed to create epoll\n");
        return NULL;
    }
    int endIndex = s_client_count;

    for (int i = 0; i < endIndex; ++i) {
        FileOwner socketfd;
        int port;
        while (true) {
            port = s_port++;
            FileOwner _socketfd(createServerSocket(port));
            if (_socketfd.get() == -1) {
                continue;
            }
            socketfd.swap(_socketfd);
            break;
        }
        struct epoll_event ev;

        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN;
        ev.data.fd = socketfd.get();

        if (epoll_ctl(epollfd.get(), EPOLL_CTL_ADD, socketfd.get(), &ev) == -1) {
            LINLOG("epoll_ctl add failed %d\n", errno);
            return NULL;
        }
        LINLOG_VERBOSE("added sock %d to fv\n", socketfd.get());
        s_handlers[i].fd_ = socketfd.get();
        s_handlers[i].serverPort_ = port;
        LINLOG("server port bind at port: %d for client: %s.\n", port, s_handlers[i].handler_->name());
        fv.push_back(socketfd);
    }

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        struct epoll_event ev;

        memset(&ev, 0, sizeof(ev));
        int nfds = epoll_wait(epollfd.get(), &ev, 1, -1);

        if (nfds == -1) {
            if (errno != EINTR) {
                LINLOG("epoll_wait failed:%d", errno);
                return NULL;
            } else
                continue;
        }

        FileOwner clientSockFd(accept(ev.data.fd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen));
        if (-1 == clientSockFd.get()) {
            LINLOG("accept failed:errno = %d\n", errno);
            return NULL;
        }

        LINLOG_VERBOSE("accepted client:%d:%d\n", ev.data.fd, clientSockFd.get());
        // find handler

        int endIndex = s_client_count;
        int i;

        for (i = 0; i < endIndex; ++i) {
            if (s_handlers[i].fd_ == ev.data.fd) {
                break;
            }
        }

        if (i == endIndex) {
            // impossible
            LINLOG("i == endIndex!Failed to find associate client\n");
            shutdown(clientSockFd.get(), SHUT_RDWR);
            return NULL;
        }

        lockHeapServer();
        s_handlers[i].handler_->handleClient(clientSockFd.get(), reinterpret_cast<struct sockaddr*>(&clientAddr));
        unlockHeapServer();
        shutdown(clientSockFd.get(), SHUT_RDWR);
    }
    return NULL;
}

void startServer(void)
{
    int pthread_ret;
    pthread_t mythread;
    int index = s_client_count;
    if (index == 0) {
        LINLOG("not client to server\n");
        return;
    }
    pthread_ret = pthread_create(&mythread, NULL, serverFunc, NULL);
    if (pthread_ret != 0) {
        LINLOG("pthread create failed:errno = %d\n", pthread_ret);
        return;
    }
    pthread_detach(mythread);
}

void registerClient(ClientHandler* handler)
{
    int index = s_client_count;
    // overflow the max count
    if (index >= MAX_HANDLER_COUNT) {
        delete handler;
        return;
    }
    LINLOG_VERBOSE("registering handler %p\n", handler);
    s_handlers[index].handler_ = handler;
    s_handlers[index].serverPort_ = 0;
    s_client_count++;
}
}
#endif // ENABLE_HEAP_SEVER

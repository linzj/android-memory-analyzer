#ifndef HEAPSERVER_H
#define HEAPSERVER_H 

namespace BrowserShell {
#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
    void startServer(void);
    int sendTillEnd(int fd,const char * buffer,size_t s);
#else
    inline void startServer(void) {}
#endif // ENABLE_HEAP_SEVER
}

#endif /* HEAPSERVER_H */

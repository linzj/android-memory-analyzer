#ifndef HEAPSERVER_H
#define HEAPSERVER_H 

#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
struct SendOnceGeneral
{
    const void * m_chunk;
    size_t m_len;
    size_t m_backtracesLen;
};
int sendTillEnd(int fd,const char * buffer,size_t s);
#endif // ENABLE_HEAP_SEVER

namespace BrowserShell {
#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
    void startServer(void);
#else
    inline void startServer(void) {}
#endif // ENABLE_HEAP_SEVER
}

#endif /* HEAPSERVER_H */

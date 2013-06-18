#ifndef HEAPSERVER_H
#define HEAPSERVER_H 
#define ENABLE_HEAP_SEVER 1

namespace BrowserShell {
#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
    void startServer(void);
#else
    inline void startServer(void) {}
#endif // ENABLE_HEAP_SEVER
}

#endif /* HEAPSERVER_H */

#ifndef HEAPSERVER_H
#define HEAPSERVER_H
#include <stdint.h>

#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
struct SendOnceGeneral {
    const void* m_chunk;
    uint32_t m_len;
    uint32_t m_backtracesLen;
    uint32_t m_dataAttrib; // for this data's attributes
};
static const int DATA_ATTR_USER_CONTENT = 0x1;
int sendTillEnd(int fd, const char* buffer, size_t s);
#endif // ENABLE_HEAP_SEVER

class ClientHandler {
public:
    virtual ~ClientHandler() {}
    virtual void handleClient(int fd, struct sockaddr*) = 0;
};

namespace BrowserShell {
#if defined(ENABLE_HEAP_SEVER) && ENABLE_HEAP_SEVER == 1
void startServer(void);
void registerClient(ClientHandler*);
#else
inline void startServer(void)
{
}
#endif // ENABLE_HEAP_SEVER
}

#endif /* HEAPSERVER_H */

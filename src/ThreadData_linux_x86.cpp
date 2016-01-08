#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <android/log.h>
#include "MapParse.h"
#include "HeapServer.h"

struct sigcontext_ia32 {
    unsigned short gs, __gsh;
    unsigned short fs, __fsh;
    unsigned short es, __esh;
    unsigned short ds, __dsh;
    unsigned int di;
    unsigned int si;
    unsigned int bp;
    unsigned int sp;
    unsigned int bx;
    unsigned int dx;
    unsigned int cx;
    unsigned int ax;
    unsigned int trapno;
    unsigned int err;
    unsigned int ip;
    unsigned short cs, __csh;
    unsigned int flags;
    unsigned int sp_at_signal;
    unsigned short ss, __ssh;
    unsigned int fpstate; /* really (struct _fpstate_ia32 *) */
    unsigned int oldmask;
    unsigned int cr2;
};

struct ucontext {
    unsigned long uc_flags;
    struct ucontext* uc_link;
    stack_t uc_stack;
    struct sigcontext_ia32 uc_mcontext;
};

static int mycompare(const MapElement* e, unsigned long start)
{
    if (e->m_start < start) {
        return 1;
    }
    return 0;
}

void sendStackData(int fd, void** buf, int count, const MapElement* list)
{
    for (int i = 0; i < count; ++i) {
        ucontext* context = static_cast<ucontext*>(buf[i]);
        unsigned long start = context->uc_mcontext.sp;
        const MapElement* found = lowerBound(list, start, mycompare);
        if (found != NULL) {
            if (found != list && found->m_start != start)
                found = found->m_prev;
        } else {
            found = list->m_prev;
        }
        if ((found->m_start <= start)
            && (found->m_end >= start)
            && ((found->m_protect & 7) == (MapElement::READ | MapElement::WRITE))) {
            SendOnceGeneral once = { reinterpret_cast<void*>(start), found->m_end - start, 0x80000000, DATA_ATTR_USER_CONTENT };
            sendTillEnd(fd, reinterpret_cast<const char*>(&once), sizeof(once));
            sendTillEnd(fd, reinterpret_cast<const char*>(start), found->m_end - start);
        }
    }
}

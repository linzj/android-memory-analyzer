#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <algorithm>
#include <android/log.h>
#include "MapParse.h"
#include "HeapServer.h"

struct sigcontext_arm {
    unsigned long trap_no;
    unsigned long error_code;
    unsigned long oldmask;
    unsigned long arm_r0;
    unsigned long arm_r1;
    unsigned long arm_r2;
    unsigned long arm_r3;
    unsigned long arm_r4;
    unsigned long arm_r5;
    unsigned long arm_r6;
    unsigned long arm_r7;
    unsigned long arm_r8;
    unsigned long arm_r9;
    unsigned long arm_r10;
    unsigned long arm_fp;
    unsigned long arm_ip;
    unsigned long arm_sp;
    unsigned long arm_lr;
    unsigned long arm_pc;
    unsigned long arm_cpsr;
    unsigned long fault_address;
};

struct ucontext {
    unsigned long uc_flags;
    struct ucontext* uc_link;
    stack_t uc_stack;
    struct sigcontext_arm uc_mcontext;
};

static int mycompare(MapElement const& e, unsigned start)
{
    if (e.m_start < start) {
        return 1;
    }
    return 0;
}

void sendStackData(int fd, void** buf, int count, MapParse::MapList const& list)
{
    for (int i = 0; i < count; ++i) {
        ucontext* context = static_cast<ucontext*>(buf[i]);
        unsigned long start = context->uc_mcontext.arm_sp;
        MapParse::MapList::const_iterator found = std::lower_bound(list.begin(), list.end(), start, mycompare);
        if (found != list.end()) {
            if (found->m_start != start)
                --found;
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

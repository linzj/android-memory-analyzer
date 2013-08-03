#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <algorithm>
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
       unsigned int fpstate;		/* really (struct _fpstate_ia32 *) */
       unsigned int oldmask;
       unsigned int cr2;
};

struct ucontext {
	unsigned long	  uc_flags;
	struct ucontext  *uc_link;
	stack_t		  uc_stack;
	struct sigcontext_ia32 uc_mcontext;
};


static int mycompare(MapElement const & e,unsigned start)
{
    if(e.m_start < start )
    {
        return 1;
    }
    return 0;
}


void sendStackData(int fd,void ** buf,int count,MapParse::MapList const & list)
{
    for(int i = 0;i < count ;++i)
    {
        ucontext * context = static_cast<ucontext*>(buf[i]);
        unsigned long start = context->uc_mcontext.sp;
        MapParse::MapList::const_iterator found = std::lower_bound(list.begin(),list.end(),start,mycompare);
        if(found != list.end())
        {
            if(found->m_start != start)
                --found;
        }
        if((found->m_start <= start)
            && (found->m_end >= start)
            && ((found->m_protect & 7) == (MapElement::READ | MapElement::WRITE)))
        {
            SendOnceGeneral once = {reinterpret_cast<void*>(start),found->m_end - start,0x80000000};
            sendTillEnd(fd,reinterpret_cast<const char*>(&once),sizeof(once));
            sendTillEnd(fd,reinterpret_cast<const char*>(start),found->m_end - start);
        }
    }
}



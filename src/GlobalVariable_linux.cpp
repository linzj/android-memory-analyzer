#include <stddef.h>
#include "HeapServer.h"
#include "MapParse.h"
#include <android/log.h>

static void sendElement(int fd,MapParse::MapList::const_iterator const & i )
{
    unsigned long start,end;
    start = i->m_start;
    end = i->m_end;
    SendOnceGeneral once = {reinterpret_cast<const void*>(start),end-start,0x81000000};
    sendTillEnd(fd,reinterpret_cast<const char*>(&once),sizeof(once));
    sendTillEnd(fd,reinterpret_cast<const char*>(start),end - start);
}

void sendGlobalVariable(int fd,MapParse::MapList const & list)
{
    for(MapParse::MapList::const_iterator i = list.begin(); i != list.end() ; ++i)
    {
        if(((i->m_protect & 7)  == (MapElement::READ | MapElement::WRITE)) 
            && i->m_path.size())
        {
            sendElement(fd,i);
            // currently only show interest in none initialize global
            MapParse::MapList::const_iterator old = i++;
            if((old->m_end != i->m_start)
                || ((i->m_protect & 7)  != (MapElement::READ | MapElement::WRITE))
                || (i->m_path.size()))
            {
                continue;
            }
            sendElement(fd,i);
        }
    }
}


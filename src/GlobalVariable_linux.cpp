#include <stddef.h>
#include "HeapServer.h"
#include "MapParse.h"

void sendGlobalVariable(int fd,MapParse::MapList const & list)
{
    for(MapParse::MapList::const_iterator i = list.begin(); i != list.end() ; ++i)
    {
        if(((i->m_protect & 7)  == (MapElement::READ | MapElement::WRITE)) 
            && i->m_path.size())
        {
            unsigned long start,end;
            start = i->m_start;
            end = i->m_end;
            SendOnceGeneral once = {reinterpret_cast<const void*>(start),end-start,0x81000000};
            sendTillEnd(fd,reinterpret_cast<const char*>(&once),sizeof(once));
            sendTillEnd(fd,reinterpret_cast<const char*>(start),end - start);
        }
    }
}


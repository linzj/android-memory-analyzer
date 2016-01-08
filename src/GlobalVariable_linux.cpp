#include <stddef.h>
#include "HeapServer.h"
#include "HeapSnapshotHandler.h"

static void sendElement(int fd, const MapElement* i)
{
    unsigned long start, end;
    start = i->m_start;
    end = i->m_end;
    SendOnceGeneral once = { reinterpret_cast<const void*>(start), end - start, 0x81000000, DATA_ATTR_USER_CONTENT };
    sendTillEnd(fd, reinterpret_cast<const char*>(&once), sizeof(once));
    sendTillEnd(fd, reinterpret_cast<const char*>(start), end - start);
}

void HeapSnapshotHandler::sendGlobalVariable(int fd, const MapElement* list)
{
    bool start = true;
    for (const MapElement* i = list; (i != list || start) ; i = i->m_next) {
        start = false;
        // intentionally skip the shared mappings
        if (((i->m_protect & 15) == (MapElement::READ | MapElement::WRITE))
            && i->m_path) {
            sendElement(fd, i);
            // currently only show interest in none initialize global
            const MapElement* old = i;
            i = i->m_next;
            if ((old->m_end != i->m_start)
                || ((i->m_protect & 7) != (MapElement::READ | MapElement::WRITE))
                || (i->m_path)
                || (i == list)) {
                continue;
            }
            sendElement(fd, i);
        }
    }
}

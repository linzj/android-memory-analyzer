#include <stddef.h>
#include "HeapServer.h"
#include "HeapSnapshotHandler.h"

void sendStackData(int fd, void** buf, int count, const MapElement* list);

void HeapSnapshotHandler::sendThreadData(int fd, void** buf, int count, const MapElement* list)
{
    sendStackData(fd, buf, count, list);
}

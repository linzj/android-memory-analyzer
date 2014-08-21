#include <stddef.h>
#include "HeapServer.h"
#include "HeapSnapshotHandler.h"

void sendStackData(int fd, void** buf, int count, MapParse::MapList const& list);

void HeapSnapshotHandler::sendThreadData(int fd, void** buf, int count, MapParse::MapList const& list)
{
    sendStackData(fd, buf, count, list);
}

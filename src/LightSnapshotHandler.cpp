#include <stdint.h>
#include "LightSnapshotHandler.h"

#include "DumpHeap.h"

void LightSnapshotHandler::handleClient(int fd, struct sockaddr*)
{
    DumpHeap dh(fd, false);
    dh.callWalk();
}

LightSnapshotHandler::LightSnapshotHandler()
{
}

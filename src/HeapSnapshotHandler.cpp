#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/socket.h>

#include "HeapSnapshotHandler.h"
#include "HeapInfo.h"
#include "DumpHeap.h"
#include "StopWorld.h"
#include "LinLog.h"

HeapSnapshotHandler::HeapSnapshotHandler()
{
}

void HeapSnapshotHandler::handleClient(int fd, struct sockaddr*)
{
    int forkRet;
    stopTheWorld();

    forkRet = fork();
    if (forkRet <= -1) {
        LINLOG("fails to fork: errno: %d.\n", errno);
    } else if (forkRet > 0) {
        // parent
        restartTheWorld();
    } else {
        // child
        HeapInfo::setLockDisable();
        DumpHeap dh(fd);
        dh.callWalk();
        void* buf[64];
        int userContextCount = getUserContext(buf, 64);
        MapElement* mapList = MapParse::parseFile("/proc/self/maps");
        // send back thread stack
        sendThreadData(fd, buf, userContextCount, mapList);
        // send back global variable area
        sendGlobalVariable(fd, mapList);
        MapParse::freeMapList(mapList);
        struct mallinfo myinfo = mallinfo();
        LINLOG("LIN:native heap: %d, allocated: %d, free: %d\n", myinfo.usmblks, myinfo.uordblks, myinfo.fordblks);
        shutdown(fd, SHUT_RDWR);
        _exit(0);
    }
}

const char* HeapSnapshotHandler::name() const
{
    return "HeapSnapshotHandler";
}

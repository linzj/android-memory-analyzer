#include <stdint.h>
#include <stdio.h>

#include "HeapSnapshotHandler.h"
#include "DumpHeap.h"
#include "StopWorld.h"

struct mymallinfo {
    size_t arena; /* non-mmapped space allocated from system */
    size_t ordblks; /* number of free chunks */
    size_t smblks; /* always 0 */
    size_t hblks; /* always 0 */
    size_t hblkhd; /* space in mmapped regions */
    size_t usmblks; /* maximum total allocated space */
    size_t fsmblks; /* always 0 */
    size_t uordblks; /* total allocated space */
    size_t fordblks; /* total free space */
    size_t keepcost; /* releasable (via malloc_trim) space */
};

extern "C" {
mymallinfo dlmallinfo(void);
}

HeapSnapshotHandler::HeapSnapshotHandler()
{
}

void HeapSnapshotHandler::handleClient(int fd, struct sockaddr*)
{
    stopTheWorld();
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
    mymallinfo myinfo = dlmallinfo();
    fprintf(stderr, "LIN:free space = %f\n", ((float)myinfo.fordblks) / 1024.0f);
    restartTheWorld();
}

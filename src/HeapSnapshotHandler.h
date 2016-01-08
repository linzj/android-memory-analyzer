#ifndef HEAPSNAPSHOTHANDLER_H
#define HEAPSNAPSHOTHANDLER_H
#pragma once

#include "HeapServer.h"
#include "MapParse.h"

class HeapSnapshotHandler : public ClientHandler {
public:
    HeapSnapshotHandler();

    virtual void handleClient(int fd, struct sockaddr*);

private:
    void sendThreadData(int fd, void** buf, int count, const MapElement*);
    void sendGlobalVariable(int fd, const MapElement* list);
};

#endif /* HEAPSNAPSHOTHANDLER_H */

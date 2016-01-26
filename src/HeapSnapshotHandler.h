#ifndef HEAPSNAPSHOTHANDLER_H
#define HEAPSNAPSHOTHANDLER_H
#pragma once

#include "HeapServer.h"
#include "MapParse.h"

class HeapSnapshotHandler : public ClientHandler {
public:
    HeapSnapshotHandler();

private:
    virtual void handleClient(int fd, struct sockaddr*);
    virtual const char* name() const;

    void sendThreadData(int fd, void** buf, int count, const MapElement*);
    void sendGlobalVariable(int fd, const MapElement* list);
};

#endif /* HEAPSNAPSHOTHANDLER_H */

#ifndef LIGHTSNAPSHOTHANDLER_H
#define LIGHTSNAPSHOTHANDLER_H
#pragma once
#include "HeapServer.h"

// This handler only transfer the backtraces of the heap's allocations
class LightSnapshotHandler : public ClientHandler {
public:
    LightSnapshotHandler();
    virtual void handleClient(int fd, struct sockaddr*);
};

#endif /* LIGHTSNAPSHOTHANDLER_H */

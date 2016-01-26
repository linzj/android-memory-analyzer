#ifndef LIGHTSNAPSHOTHANDLER_H
#define LIGHTSNAPSHOTHANDLER_H
#pragma once
#include "HeapServer.h"

// This handler only transfer the backtraces of the heap's allocations
class LightSnapshotHandler : public ClientHandler {
public:
    LightSnapshotHandler();
private:
    virtual void handleClient(int fd, struct sockaddr*);
    virtual const char* name() const;
};

#endif /* LIGHTSNAPSHOTHANDLER_H */

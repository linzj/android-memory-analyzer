#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "HeapServer.h"
#include "StopWorld.h"
#include <android/log.h>

void sendStackData(int fd,void ** buf,int count);

void sendThreadData(int fd)
{
    stopTheWorld();
    void * buf[64];
    int count = getUserContext(buf,64);
    __android_log_print(ANDROID_LOG_DEBUG,"LIN","sending %d threads data\n",count);
    sendStackData(fd,buf,count);
    restartTheWorld();
}


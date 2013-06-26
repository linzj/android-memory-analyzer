#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "HeapServer.h"
#include "StopWorld.h"
#include "MapParse.h"
#include <android/log.h>

void sendStackData(int fd,void ** buf,int count,MapParse::MapList const & list);

void sendThreadData(int fd,void ** buf,int count,MapParse::MapList const &list)
{
    sendStackData(fd,buf,count,list);
}


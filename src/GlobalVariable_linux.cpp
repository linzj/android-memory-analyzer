#include <stdio.h>
#include <stdlib.h>
#include "HeapServer.h"

void sendGlobalVariable(int fd)
{
    FILE * maps = fopen("/proc/self/maps","r");
    while(true)
    {
        static const int BUF_SIZE = 256;
        static const int slash_index = 25 + sizeof(void*) * 6;
        char buf[BUF_SIZE];
        fgets(buf,BUF_SIZE,maps);
        if(feof(maps))
        {
            break;
        }
        if(strlen(buf) > slash_index && strstr(buf,"rw-p") && buf[slash_index] == '/')
        {
            unsigned long start,end;
            start = strtoul(buf,NULL,16);
            end = strtoul(buf + 9,NULL,16);
            fprintf(stderr,"sending global variable,%08lx-%08lx",start,end);
            SendOnceGeneral once = {reinterpret_cast<const void*>(start),end-start,0x81000000};
            sendTillEnd(fd,reinterpret_cast<const char*>(&once),sizeof(once));
            sendTillEnd(fd,reinterpret_cast<const char*>(start),end - start);
        }
    }
    fclose(maps);
}


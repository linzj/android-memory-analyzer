#ifndef DUMPHEAP_H
#define DUMPHEAP_H 
#pragma once

class DumpHeap
{
    public:
        explicit DumpHeap(int fd,bool sendUserData = true);

        void callWalk(void);

    private:
        
        void notifyChunk(const void * userptr,size_t userlen);
        static void mywalk(const void *userptr, size_t userlen, void *arg);

    private:
        int fd_;
        bool sendUserData_;
};

#endif /* DUMPHEAP_H */

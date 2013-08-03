#ifndef DUMPHEAP_H
#define DUMPHEAP_H 
#pragma once

class DumpHeap
{
    public:
        explicit DumpHeap(int fd);

        void callWalk(void);

    private:
        
        void notifyChunk(const void * chunk,size_t len,const void * userptr,size_t userlen);
        static void mywalk(const void *chunkptr, size_t chunklen,
                const void *userptr, size_t userlen,
                void *arg);

    private:
        int m_fd;
};

#endif /* DUMPHEAP_H */

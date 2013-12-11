#ifndef HEAPINFO_H
#define HEAPINFO_H 
class ChunkInfo;

class HeapInfo 
{
public:
    static void init(int dataSize);
    static void registerChunkInfo(const void * ,ChunkInfo const &);
    static void unregisterChunkInfo(const void *);
    static ChunkInfo  const * getChunkInfo(const void *);
    static void walk(void (func)(const void *userptr, size_t userlen, void *arg),void * );

private:
    HeapInfo(size_t dataSize);
    ~HeapInfo();
    struct HeapInfoImpl;
    template <class _Tp>
    friend class SpecialAllocator;
    static struct HeapInfoImpl * m_impl;
};

#endif /* HEAPINFO_H */

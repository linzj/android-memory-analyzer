#ifndef HEAPINFO_H
#define HEAPINFO_H
class ChunkInfo;

class HeapInfo {
public:
    static void init(int dataSize);
    static void registerChunkInfo(const void*, ChunkInfo const&);
    static void unregisterChunkInfo(const void*);
    static ChunkInfo const* getChunkInfo(const void*);
    typedef void (*pfn_walk)(const void* chunkptr, size_t chunklen,
                          const void* userptr, size_t userlen,
                          void* arg);
    static void walk(pfn_walk walk, void* data);
    static void lockHeapInfo();
    static void unlockHeapInfo();
    static bool isCurrentThreadLockedRecursive();

private:
    HeapInfo(size_t dataSize);
    ~HeapInfo();
    template <class _Tp>
    friend class SpecialAllocator;
};

#endif /* HEAPINFO_H */

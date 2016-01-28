#ifndef CHUNKINFO_H
#define CHUNKINFO_H

struct ChunkInfo {
    static const int MAX_BACKTRACES = 20;
    const void* m_backtraces[MAX_BACKTRACES];
    size_t m_backtracesLen;
    size_t m_chunkSize;
    unsigned m_flags;
    static void get(ChunkInfo& info, void* data);
    enum {
        MMAP = 0x1,
    };
};
#endif /* CHUNKINFO_H */

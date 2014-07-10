#ifndef CHUNKINFO_H
#define CHUNKINFO_H
struct ChunkInfo {
    static const int MAX_BACKTRACES = 20;
    const void* m_backtraces[MAX_BACKTRACES];
    size_t m_backtracesLen;
    static void get(ChunkInfo& info, void* data);
};
#endif /* CHUNKINFO_H */

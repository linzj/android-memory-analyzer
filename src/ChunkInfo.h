#ifndef CHUNKINFO_H
#define CHUNKINFO_H 
struct ChunkInfo
{
    static const int MAX_BACKTRACES = 10;
    const void * m_backtraces[MAX_BACKTRACES];
    size_t m_backtracesLen;
    size_t m_size;
    static void get(ChunkInfo & info,void * data,size_t size);
};
#endif /* CHUNKINFO_H */

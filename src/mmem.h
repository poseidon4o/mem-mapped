#ifndef _MMEM_H_INCLUDED_
#define _MMEM_H_INCLUDED_

#include <Windows.h>
#include <cstdint>

class ChunkAllocator {

public:
    ChunkAllocator(uint64_t chunkSize, uint64_t chunkCount);
    ~ChunkAllocator();

    LPVOID getChunk(uint64_t chunk);
private:
    const uint64_t m_ChunkSize, m_ChunkCount;
    LPVOID m_Memory;
};

#endif // _MMEM_H_INCLUDED_

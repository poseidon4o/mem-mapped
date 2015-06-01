#ifndef _MFILE_H_INCLUDED_
#define _MFILE_H_INCLUDED_

#include <cstdio>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

#include "mpage.h"
#include "mmem.h"

class MemoryMapped {
public:
    MemoryMapped(const std::string fileName, int pageSize = 1 << 20, int pageCount = 4);
    ~MemoryMapped();

    MemoryMapped(const MemoryMapped &) = delete;
    MemoryMapped & operator=(const MemoryMapped &) = delete;

    PageItemProxy & operator[](uint64_t index);
    void flush();

    uint64_t size() const;
    int pageSize() const;
    int pageCount() const;
    operator bool();



private:
    void map(uint64_t from);

    typedef int PageIndex;
    const static PageIndex InvalidPage = PageIndex(-1);

    PageIndex indexToPage(const uint64_t & idx) const;

    PageIndex mapCandidate();
    void touchPage(PageIndex idx);

    void flushPage(PageIndex idx);


private:
    uint64_t m_FileSize;
    int m_UsedPages;
    std::unique_ptr<uint64_t[]> m_PageStartIndecies;
    std::unique_ptr<PageIndex[]> m_PageUse;
    std::unique_ptr<MemoryPage[]> m_Pages;

    
    const int m_PageSize;
    const int m_PageCount;
    
    FILE * m_File;
    std::string m_FileName;
    ChunkAllocator m_Allocator;
};

#endif // _MFILE_H_INCLUDED_
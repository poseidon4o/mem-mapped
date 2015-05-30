#ifndef _MFILE_H_INCLUDED_
#define _MFILE_H_INCLUDED_

#include <cstdio>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

#include "mpage.h"


class MemoryMapped {
public:
    MemoryMapped(const std::string fileName);
    ~MemoryMapped();

    MemoryMapped(const MemoryMapped &) = delete;
    MemoryMapped & operator=(const MemoryMapped &) = delete;

    PageItemProxy & operator[](uint64_t index);
    void flush();

    const uint64_t size() const;
    operator bool();



    const static int pageSize = 4096;
    const static int pageCount = 8;
protected:
    void map(uint64_t from);

    typedef int PageIndex;
    const static PageIndex InvalidPage = PageIndex(-1);

    PageIndex indexToPage(uint64_t idx);
    PageIndex mapCandidate();
    void touchPage(PageIndex idx);

    void flushPage(PageIndex idx);


private:
    std::string m_FileName;
    FILE * m_File;
    MemoryPage m_Pages[MemoryMapped::pageCount];
    PageIndex m_PageUse[MemoryMapped::pageCount];
    int m_UsedPages;
    uint64_t m_FileSize;
};

#endif // _MFILE_H_INCLUDED_
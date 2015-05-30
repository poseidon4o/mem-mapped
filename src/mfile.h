#ifndef _MFILE_H_INCLUDED_
#define _MFILE_H_INCLUDED_

#include <cstdio>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

class PageItemProxy {
public:
    PageItemProxy() = delete;
    PageItemProxy(const PageItemProxy &) = delete;
    PageItemProxy & operator=(const PageItemProxy &) = delete;
    PageItemProxy * operator&() = delete;

    PageItemProxy & operator=(const uint8_t & value);
    operator uint8_t();
};


class MemoryPage {
    friend class MemoryMapped;
    friend class PageItemProxy;

    PageItemProxy & relative(uint64_t idx);
    PageItemProxy & absolute(uint64_t idx);

    int size() const;
    bool dirty() const;
    uint64_t start() const;

    void reset(int start, int size);

    // Unsafe - does not update dirty flag
    uint8_t *& data();

    MemoryPage();

    MemoryPage(const MemoryPage &) = delete;
    MemoryPage & operator=(const MemoryPage &) = delete;

private:
    uint8_t * m_Data;
    int m_Size, m_Start;

    // Since this is single threaded use only we can store the last access index
    // and the proxy can be zero-sized
    int m_ProxyIndex;

    bool m_Dirty;
};




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
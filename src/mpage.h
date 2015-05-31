#ifndef _MPAGE_H_INCLUDED_
#define _MPAGE_H_INCLUDED_

#include <cstdint>

class PageItemProxy {
public:
    PageItemProxy() = delete;
    PageItemProxy(const PageItemProxy &) = delete;
    PageItemProxy & operator=(const PageItemProxy &) = delete;
    PageItemProxy * operator&() = delete;

    PageItemProxy & operator=(const uint8_t & value);
    operator uint8_t&();
    operator uint8_t() const;
};


class MemoryPage {
    friend class MemoryMapped;
    friend class PageItemProxy;

    MemoryPage();

    PageItemProxy & relative(uint64_t idx);
    PageItemProxy & absolute(uint64_t idx);

    int size() const;
    bool dirty() const;
    uint64_t start() const;

    void reset(int start, int size);

    // Unsafe - does not update dirty flag
    uint8_t *& data();

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

#endif // _MPAGE_H_INCLUDED_

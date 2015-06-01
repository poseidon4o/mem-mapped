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

    inline const int & size() const { return m_Size; }
    inline const uint64_t & start() const { return *m_Start; };
    bool dirty() const;
    

    void reset(int start, int size);
    void setStart(uint64_t * start) { m_Start = start; }

    // Unsafe - does not update dirty flag
    uint8_t *& data();

    MemoryPage(const MemoryPage &) = delete;
    MemoryPage & operator=(const MemoryPage &) = delete;

private:

    // Since this is single threaded use only we can store the last access index
    // and the proxy can be zero-sized
    int m_ProxyIndex;
    int m_Size;

    
    uint8_t * m_Data;
    uint64_t * m_Start;

    bool m_Dirty;
};

#endif // _MPAGE_H_INCLUDED_

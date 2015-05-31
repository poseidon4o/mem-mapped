#include "mpage.h"


PageItemProxy & PageItemProxy::operator=(const uint8_t & value)
{
    MemoryPage & page = reinterpret_cast<MemoryPage&>(*this);
    if (value == page.m_Data[page.m_ProxyIndex]) {
        return *this;
    }

    page.m_Data[page.m_ProxyIndex] = value;
    page.m_Dirty = true;
    return *this;
}

PageItemProxy::operator uint8_t()
{
    MemoryPage & page = reinterpret_cast<MemoryPage&>(*this);
    return page.m_Data[page.m_ProxyIndex];
}


MemoryPage::MemoryPage() :
    m_Data(nullptr), m_Size(0), m_Start(0), m_Dirty(false)
{
}

int MemoryPage::size() const
{
    return m_Size;
}

uint64_t MemoryPage::start() const
{
    return m_Start;
}

bool MemoryPage::dirty() const
{
    return m_Dirty;
}

void MemoryPage::reset(int start, int size)
{
    m_Start = start;
    m_Size = size;
    m_Dirty = false;
}

uint8_t *& MemoryPage::data()
{
    return m_Data;
}

PageItemProxy & MemoryPage::relative(uint64_t idx)
{
    m_ProxyIndex = idx;
    return reinterpret_cast<PageItemProxy&>(*this);
}

PageItemProxy & MemoryPage::absolute(uint64_t idx)
{
    return relative(idx - m_Start);
}
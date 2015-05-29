#include "mfile.h"
#include <algorithm>


MemoryPage::MemoryPage() :
    m_Data(nullptr), m_Size(0), m_Start(0), m_Capacity(0), m_Dirty(0)
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
    m_Dirty = 0;
}

uint8_t *& MemoryPage::data()
{
    return m_Data;
}

uint8_t & MemoryPage::relative(uint64_t idx)
{
    m_Dirty = 1;
    return m_Data[idx];
}

uint8_t & MemoryPage::absolute(uint64_t idx)
{
    return relative(idx - m_Start);
}



MemoryMapped::MemoryMapped(const std::string fileName) :
    m_FileName(std::move(fileName)), m_File(nullptr), m_UsedPages(0)
{
    m_File = fopen(m_FileName.c_str(), "r+");
    if (!m_File) {
        throw std::exception("Failed to load file");
    }

    setvbuf(m_File, NULL, _IONBF, 0);
    fseek(m_File, 0, SEEK_END);

    fpos_t size;
    fgetpos(m_File, &size);
    m_FileSize = size;

    uint8_t * memory = new uint8_t[MemoryMapped::pageCount * MemoryMapped::pageSize];
    for (PageIndex c = 0; c < MemoryMapped::pageCount; ++c) {
        m_Pages[c].data() = memory + (c * MemoryMapped::pageSize);
    }
    for (int c = 0; c < MemoryMapped::pageCount; ++c) {
        m_PageUse[c] = PageIndex(c);
    }
}

MemoryMapped::~MemoryMapped()
{
    // ugly but works for this case
    delete[] m_Pages[0].data();
    this->flush();
    fclose(m_File);
}

uint8_t & MemoryMapped::operator[](uint64_t index)
{
    PageIndex page = indexToPage(index);
    if (page == InvalidPage) {
        map(index);
    }
    page = indexToPage(index);
    if (page == InvalidPage) {
        throw std::exception("Internal error");
    }
    touchPage(page);
    return m_Pages[page].absolute(index);
}

const uint64_t MemoryMapped::size() const
{
    return m_FileSize;
}

MemoryMapped::operator bool()
{
    return m_File;
}

void MemoryMapped::flush()
{
    for (PageIndex c = 0; c < MemoryMapped::pageCount; ++c) {
        if (m_Pages[c].dirty()) {
            flushPage(c);
        }
    }
}

void MemoryMapped::flushPage(PageIndex idx)
{
    MemoryPage & page = m_Pages[idx];
    fseek(m_File, page.m_Start, SEEK_SET);
    if (page.size() != fwrite(page.data(), 1, page.size(), m_File)) {
        throw std::exception("Failed to flush file to disk");
    }
    page.reset(0, 0);
}

void MemoryMapped::map(uint64_t from)
{
    PageIndex pageIndex = mapCandidate();
    if (pageIndex == InvalidPage) {
        throw std::exception("Failed to map file, no candidate page.");
    }

    MemoryPage & page = m_Pages[pageIndex];
    if (page.dirty()) {
        flushPage(pageIndex);
    }

    uint64_t mapSize = std::min<uint64_t>(MemoryMapped::pageSize, m_FileSize - from);
    page.reset(from, mapSize);
    fseek(m_File, from, SEEK_SET);

    if (fread(page.data(), 1, mapSize, m_File) != mapSize) {
        throw std::exception("Failed to map file in memory");
    }
    ++m_UsedPages;
}

void MemoryMapped::touchPage(PageIndex idx)
{
    for (int c = 0; c < MemoryMapped::pageCount - 1; ++c) {
        if (idx == m_PageUse[c]) {
            std::swap(m_PageUse[c], m_PageUse[c + 1]);
        }
    }
}


MemoryMapped::PageIndex MemoryMapped::mapCandidate()
{
    return m_UsedPages >= MemoryMapped::pageCount ?
        
        m_PageUse[0] :

        // handle the case when there is a free page
        *std::find_if(m_PageUse, m_PageUse + MemoryMapped::pageCount, [this](PageIndex idx) {
            return !this->m_Pages[idx].dirty();
        });
}

MemoryMapped::PageIndex MemoryMapped::indexToPage(uint64_t idx)
{
    for (PageIndex c = 0; c < MemoryMapped::pageCount; ++c) {
        if (idx >= m_Pages[c].start() && idx < m_Pages[c].start() + m_Pages[c].size()) {
            return c;
        }
    }
    return InvalidPage;
}

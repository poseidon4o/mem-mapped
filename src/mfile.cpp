#include "mfile.h"
#include <algorithm>


MemoryMapped::MemoryMapped(const std::string fileName) :
    m_FileName(std::move(fileName)), m_File(nullptr), m_UsedPages(0)
{
    m_File = fopen(m_FileName.c_str(), "rb+");
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

PageItemProxy & MemoryMapped::operator[](uint64_t index)
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

    // allign pages on pageSize so there are no overlaps
    from = (from / MemoryMapped::pageSize) * MemoryMapped::pageSize;

    uint64_t mapSize = std::min<uint64_t>(MemoryMapped::pageSize, m_FileSize - from);
    page.reset(from, mapSize);
    fseek(m_File, from, SEEK_SET);

    if (fread(page.data(), 1, mapSize, m_File) != mapSize) {
        throw std::exception("Failed to map file in memory");
    }
    m_UsedPages = std::max(m_UsedPages + 1, MemoryMapped::pageCount);
}

void MemoryMapped::touchPage(PageIndex idx)
{
    // start from last used - better chance of nop
    for (int c = MemoryMapped::pageCount - 1; c >= 0; --c) {
        if (idx == m_PageUse[c]) {
            for (int r = c; r < MemoryMapped::pageCount - 1; ++r) {
                std::swap(m_PageUse[r], m_PageUse[r + 1]);
            }
            break;
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

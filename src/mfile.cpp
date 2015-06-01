#include "mfile.h"
#include <algorithm>


MemoryMapped::MemoryMapped(const std::string fileName, int pageSize, int pageCount) :
    m_FileName(std::move(fileName)), m_File(nullptr), m_UsedPages(0),
    m_Allocator(pageSize, pageCount), m_PageSize(pageSize), m_PageCount(pageCount),
    m_Pages(new MemoryPage[pageCount]), m_PageUse(new PageIndex[pageCount]), m_PageStartIndecies(new uint64_t[pageCount])
{
    m_File = fopen(m_FileName.c_str(), "rb+");
    if (!m_File) {
        throw std::exception("Failed to load file");
    }

    setvbuf(m_File, NULL, _IONBF, 0);
    _fseeki64(m_File, 0, SEEK_END);

    fpos_t size;
    fgetpos(m_File, &size);
    m_FileSize = size;


    for (int c = 0; c < m_PageCount; ++c) {
        m_PageUse[c] = PageIndex(c);
        m_Pages[c].setStart(&m_PageStartIndecies[c]);
    }
}

MemoryMapped::~MemoryMapped()
{
    this->flush();
    fclose(m_File);
}

PageItemProxy & MemoryMapped::operator[](uint64_t index)
{
    if (index > m_FileSize) {
        throw std::exception("Out of file");
    }
    PageIndex page = indexToPage(index);
    if (page == InvalidPage) {
        map(index);
        page = indexToPage(index);
        if (page == InvalidPage) {
            throw std::exception("Internal error");
        }
    }

    touchPage(page);
    return m_Pages[page].absolute(index);
}

uint64_t MemoryMapped::size() const
{
    return m_FileSize;
}

int MemoryMapped::pageSize() const
{
    return m_PageSize;
}

int MemoryMapped::pageCount() const
{
    return m_PageCount;
}

MemoryMapped::operator bool()
{
    return m_File != nullptr;
}

void MemoryMapped::flush()
{
    for (PageIndex c = 0; c < m_PageCount; ++c) {
        if (m_Pages[c].dirty()) {
            flushPage(c);
        }
    }
}

void MemoryMapped::flushPage(PageIndex idx)
{
    MemoryPage & page = m_Pages[idx];
    _fseeki64(m_File, page.start(), SEEK_SET);
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
    if (!page.data()) {
        page.data() = reinterpret_cast<uint8_t*>(m_Allocator.getChunk(pageIndex));
    }

    if (page.dirty()) {
        flushPage(pageIndex);
    }

    // allign pages on pageSize so there are no overlaps
    from = (from / m_PageSize) * m_PageSize;

    uint64_t mapSize = std::min<uint64_t>(m_PageSize, m_FileSize - from);
    page.reset(from, mapSize);
    _fseeki64(m_File, from, SEEK_SET);

    if (fread(page.data(), 1, mapSize, m_File) != mapSize) {
        throw std::exception("Failed to map file in memory");
    }
    m_UsedPages = std::min<uint64_t>(m_UsedPages + 1, m_PageCount);
}

MemoryMapped::PageIndex MemoryMapped::indexToPage(const uint64_t & idx) const
{
    for (PageIndex c = 0; c < m_UsedPages; ++c) {
        if (idx >= m_PageStartIndecies[c] && idx < m_PageStartIndecies[c] + m_PageSize) {
            return c;
        }
    }
    return InvalidPage;
}

MemoryMapped::PageIndex MemoryMapped::mapCandidate()
{
    return m_UsedPages >= m_PageCount ?

        m_PageUse[m_PageCount - 1] :

        // handle the case when there is a free page
        *std::find_if(m_PageUse.get(), m_PageUse.get() + m_PageCount, [this](PageIndex idx) {
            return !this->m_Pages[idx].data();
        });
}

void MemoryMapped::touchPage(PageIndex idx)
{
    // start from last used - better chance of nop
    for (int c = 0; c < m_UsedPages; ++c) {
        if (idx == m_PageUse[c]) {
            for (int r = c; r > 0; --r) {
                std::swap(m_PageUse[r], m_PageUse[r - 1]);
            }
            break;
        }
    }
}

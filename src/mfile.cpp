#include "mfile.h"

const uint64_t MemoryMapped::chunkSize = 64;

MemoryMapped::MemoryMapped(const std::string fileName):
    m_FileName(std::move(fileName)), m_Data(MemoryMapped::chunkSize),
    m_File(nullptr), m_Start(0)
{
    m_File = fopen(m_FileName.c_str(), "r");
    if (!m_File) {
        throw std::exception("Failed to load file");
    }

    //setvbuf(m_File, NULL, _IONBF, 0);
    fseek(m_File, 0, SEEK_END);

    fpos_t size;
    fgetpos(m_File, &size);
    m_FileSize = size;

    this->map(m_Start, MemoryMapped::chunkSize);
    rewind(m_File);
}

MemoryMapped::~MemoryMapped()
{
    this->flush();
    fclose(m_File);
}

uint8_t & MemoryMapped::operator[](uint64_t index)
{
    if (index < m_Start || index >= m_Start + m_Data.size()) {
        this->flush();
        this->map(index, index + m_Data.size());
    }
    return m_Data[index - m_Start];
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
    fseek(m_File, m_Start, SEEK_SET);
    if (m_Data.size() != fwrite(m_Data.data(), 1, m_Data.size(), m_File)) {
        throw std::exception("Failed to flush file to disk");
    }
}

void MemoryMapped::map(uint64_t from, uint64_t to)
{
    fseek(m_File, from, SEEK_SET);
    if (m_Data.size() < to - from) {
        m_Data.resize(to - from);
    }
    uint64_t x = fread(m_Data.data(), 1, m_Data.size(), m_File);
    if (ferror(m_File)) {
        perror("File");
    }

    if (x != m_Data.size()) {
        throw std::exception("Failed to map file in memory");
    }
    m_Start = from;
}

#ifndef _MFILE_H_INCLUDED_
#define _MFILE_H_INCLUDED_

#include <cstdio>
#include <vector>
#include <cstdint>
#include <string>

class MemoryMapped {
public:
    MemoryMapped(const std::string fileName);
    ~MemoryMapped();

    MemoryMapped(const MemoryMapped &) = delete;
    MemoryMapped & operator=(const MemoryMapped &) = delete;

    uint8_t & operator[](uint64_t index);
    void flush();

    const uint64_t size() const;
    operator bool();

    
    
    const static uint64_t chunkSize;
protected:
    void map(uint64_t from, uint64_t to);

private:
    std::string m_FileName;
    FILE * m_File;
    std::vector<uint8_t> m_Data;
    uint64_t m_Start, m_FileSize;
};

#endif // _MFILE_H_INCLUDED_
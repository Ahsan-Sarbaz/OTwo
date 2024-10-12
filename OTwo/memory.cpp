#include "memory.h"
#include <cstdint>
#include <fstream>

Memory::Memory()
{
    data = new uint8_t[64 * 1024]; // 64K of memory
}

bool Memory::LoadFromFile(const std::string& path) 
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize > (64 * 1024))
    {
        file.close();
        return false;
    }

    if (!file.read((char*)data, fileSize)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

Memory::~Memory()
{
    delete[] data;
}


uint8_t Memory::ReadByte(uint16_t index) {
    return data[index];
}

void Memory::WriteByte(uint16_t index, uint8_t value) {
    data[index] = value;
}

uint16_t Memory::ReadWord(uint16_t index) {
    return *(reinterpret_cast<uint16_t*>(data + index));
}

void Memory::WriteWord(uint16_t index, uint16_t value) {
    *(reinterpret_cast<uint16_t*>(data + index)) = value;
}

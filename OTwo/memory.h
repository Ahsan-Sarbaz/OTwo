#pragma once
#include <cstdint>
#include <string>

class Memory {

public:
	Memory();
	~Memory();

	bool LoadFromFile(const std::string& path);

	uint8_t ReadByte(uint16_t index);
	void WriteByte(uint16_t index, uint8_t value);
	uint16_t ReadWord(uint16_t index);
	void WriteWord(uint16_t index, uint16_t value);

private:
	uint8_t* data;
};

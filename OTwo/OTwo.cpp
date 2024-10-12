#include <iostream>
#include <fstream>

#include <filesystem>

#define ADC_IMM  0x69
#define ADC_ZP	 0x65
#define ADC_ZPX  0x75
#define ADC_ABS  0x6D
#define ADC_ABSX 0x7D
#define ADC_ABSY 0x79
#define ADC_INDX 0x61
#define ADC_INDY 0x71

#define LDA_IMM  0xA9
#define LDA_ZP	 0xA5
#define LDA_ZPX  0xB5
#define LDA_ABS  0xAD
#define LDA_ABSX 0xBD
#define LDA_ABSY 0xB9
#define LDA_INDX 0xA1
#define LDA_INDY 0xB1

#define LDX_IMM  0xA2
#define LDX_ZP	 0xA6
#define LDX_ZPY  0xB6
#define LDX_ABS  0xAE
#define LDX_ABSY 0xBE

#define LDY_IMM  0xA0
#define LDY_ZP	 0xA4
#define LDY_ZPX  0xB4
#define LDY_ABS  0xAC
#define LDY_ABSX 0xBC

#define AND_IMM  0x29
#define AND_ZP	 0x25
#define AND_ZPX  0x35
#define AND_ABS  0x2D
#define AND_ABSX 0x3D
#define AND_ABSY 0x39
#define AND_INDX 0x21
#define AND_INDY 0x31

#define ORA_IMM  0x09
#define ORA_ZP	 0x05
#define ORA_ZPX  0x15
#define ORA_ABS  0x0D
#define ORA_ABSX 0x1D
#define ORA_ABSY 0x19
#define ORA_INDX 0x01
#define ORA_INDY 0x11

#define EOR_IMM  0x49
#define EOR_ZP	 0x45
#define EOR_ZPX  0x55
#define EOR_ABS  0x4D
#define EOR_ABSX 0x5D
#define EOR_ABSY 0x59
#define EOR_INDX 0x41
#define EOR_INDY 0x51

#define ASL_ACC  0x0A
#define ASL_ZP	 0x06
#define ASL_ZPX  0x16
#define ASL_ABS  0x0E
#define ASL_ABSX 0x1E

#define BCC_REL  0x90
#define BCS_REL  0xB0
#define BEQ_REL  0xF0
#define BMI_REL  0x30
#define BNE_REL  0xD0
#define BPL_REL  0x10
#define BVC_REL  0x50
#define BVS_REL  0x70

#define BIT_ZP   0x24
#define BIT_ABS  0x2C

#define BRK_IMP  0x00

#define NOP 0xEA

#define PHA 0x48
#define PHP 0x08
#define PLA 0x68
#define PLP 0x28

#define JMP_ABS 0x4C
#define JMP_IND 0x6C

namespace fs = std::filesystem;

class Memory {

public:
	Memory()
	{
		data = new uint8_t[64 * 1024]; // 64K of memory
	}

	bool LoadFromFile(const std::string& path) 
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

	~Memory()
	{
		delete[] data;
	}

	uint8_t ReadByte(uint16_t index) {
		if (index >= (64 * 1024)) throw std::out_of_range("Index out of range");
		return data[index];
	}

	void WriteByte(uint16_t index, uint8_t value) {
		if (index >= (64 * 1024)) throw std::out_of_range("Index out of range");
		data[index] = value;
	}

	uint16_t ReadWord(uint16_t index) {
		if (index >= (64 * 1024 - 1)) throw std::out_of_range("Index out of range");
		return *(reinterpret_cast<uint16_t*>(data + index));
	}

	void WriteWord(uint16_t index, uint16_t value) {
		if (index >= (64 * 1024 - 1)) throw std::out_of_range("Index out of range");
		*(reinterpret_cast<uint16_t*>(data + index)) = value;
	}

private:
	uint8_t* data;
};

class CPU {

public:

	CPU(Memory* memory)
		:memory(memory)
	{
		Reset();
	}

	~CPU()
	{

	}

	void Reset()
	{
		uint16_t rv = memory->ReadWord(0xFFFC);
		PC = rv;
		I = 1;
		B = 0;
		D = 0;
		S = 0xFD;
	}

	inline uint8_t FetchInstruction()
	{
		return memory->ReadByte(PC++);
	}

	inline uint8_t FetchByte()
	{
		return memory->ReadByte(PC++);
	}

	inline uint16_t FetchWord()
	{
		auto res = memory->ReadWord(PC);
		PC += 2;
		return res;
	}

	inline uint8_t FetchByteZP()
	{
		auto zp_index = FetchByte();
		return memory->ReadByte(zp_index);
	}

	inline uint8_t FetchByteZPX()
	{
		auto zp_index = FetchByte();
		return memory->ReadByte(zp_index + X);
	}

	inline uint8_t FetchByteZPY()
	{
		auto zp_index = FetchByte();
		return memory->ReadByte(zp_index + Y);
	}

	inline uint8_t FetchByteAbsolute()
	{
		auto index = FetchWord();
		return memory->ReadByte(index);
	}

	inline uint8_t FetchByteAbsoluteX()
	{
		auto index = FetchWord();
		return memory->ReadByte(index + X);
	}

	inline uint8_t FetchByteAbsoluteY()
	{
		auto index = FetchWord();
		return memory->ReadByte(index + Y);
	}

	inline uint8_t FetchByteIndirectX()
	{
		auto zp_index = FetchByte() + X;

		uint16_t effective_address = memory->ReadByte(zp_index % 256) |
			(memory->ReadByte((zp_index + 1) % 256) << 8);

		return memory->ReadByte(effective_address);
	}

	inline uint8_t FetchByteIndirectY()
	{
		auto zp_index = FetchByte();

		uint16_t base_address = memory->ReadByte(zp_index) |
			(memory->ReadByte((zp_index + 1) % 256) << 8);

		uint16_t effective_address = base_address + Y;

		return memory->ReadByte(effective_address);
	}

	uint16_t FetchIndirectAddress() {
		uint16_t ptr = FetchWord();
		uint16_t lsb = memory->ReadByte(ptr);
		uint16_t msb = memory->ReadByte((ptr & 0xFF00) | ((ptr + 1) & 0x00FF)); // Handle page boundary
		return (msb << 8) | lsb;
	}


	inline void ADC(uint8_t itx)
	{
		uint8_t n1 = 0;

		switch (itx)
		{
		case ADC_IMM: n1 = FetchByte(); break;
		case ADC_ZP: n1 = FetchByteZP(); break;
		case ADC_ZPX: n1 = FetchByteZPX(); break;
		case ADC_ABS: n1 = FetchByteAbsolute(); break;
		case ADC_ABSX: n1 = FetchByteAbsoluteX(); break;
		case ADC_ABSY: n1 = FetchByteAbsoluteY(); break;
		case ADC_INDX: n1 = FetchByteIndirectX(); break;
		case ADC_INDY: n1 = FetchByteIndirectY(); break;
		}
		auto n2 = A;
		uint16_t r = n1 + n2 + C;
		A = r & 0xFF;
		C = (r > 0xFF) ? 1 : 0;
		Z = (A == 0) ? 1 : 0;
		V = (((n1 > 0) && (n2 > 0) && (r < 0)) || ((n1 < 0) && (n2 < 0) && (r > 0))) ? 1 : 0;
		N = (A & 0b01000000) ? 1 : 0;
	}

	inline void AND(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case AND_IMM: val = FetchByte(); break;
		case AND_ZP: val = FetchByteZP(); break;
		case AND_ZPX: val = FetchByteZPX(); break;
		case AND_ABS: val = FetchByteAbsolute(); break;
		case AND_ABSX: val = FetchByteAbsoluteX(); break;
		case AND_ABSY: val = FetchByteAbsoluteY(); break;
		case AND_INDX: val = FetchByteIndirectX(); break;
		case AND_INDY: val = FetchByteIndirectY(); break;
		}
		A &= val;
		N = (A & 0b01000000) ? 1 : 0;
		Z = (A == 0) ? 1 : 0;
	}

	inline void ORA(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case ORA_IMM: val = FetchByte(); break;
		case ORA_ZP: val = FetchByteZP(); break;
		case ORA_ZPX: val = FetchByteZPX(); break;
		case ORA_ABS: val = FetchByteAbsolute(); break;
		case ORA_ABSX: val = FetchByteAbsoluteX(); break;
		case ORA_ABSY: val = FetchByteAbsoluteY(); break;
		case ORA_INDX: val = FetchByteIndirectX(); break;
		case ORA_INDY: val = FetchByteIndirectY(); break;
		}
		A |= val;
		N = (A & 0b01000000) ? 1 : 0;
		Z = (A == 0) ? 1 : 0;
	}

	inline void EOR(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case EOR_IMM: val = FetchByte(); break;
		case EOR_ZP: val = FetchByteZP(); break;
		case EOR_ZPX: val = FetchByteZPX(); break;
		case EOR_ABS: val = FetchByteAbsolute(); break;
		case EOR_ABSX: val = FetchByteAbsoluteX(); break;
		case EOR_ABSY: val = FetchByteAbsoluteY(); break;
		case EOR_INDX: val = FetchByteIndirectX(); break;
		case EOR_INDY: val = FetchByteIndirectY(); break;
		}
		A ^= val;
		N = (A & 0b01000000) ? 1 : 0;
		Z = (A == 0) ? 1 : 0;
	}

	inline void LDA(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case LDA_IMM: val = FetchByte(); break;
		case LDA_ZP: val = FetchByteZP(); break;
		case LDA_ZPX: val = FetchByteZPX(); break;
		case LDA_ABS: val = FetchByteAbsolute(); break;
		case LDA_ABSX: val = FetchByteAbsoluteX(); break;
		case LDA_ABSY: val = FetchByteAbsoluteY(); break;
		case LDA_INDX: val = FetchByteIndirectX(); break;
		case LDA_INDY: val = FetchByteIndirectY(); break;
		}
		A = val;
		N = (A & 0b01000000) ? 1 : 0;
		Z = (A == 0) ? 1 : 0;
	}

	inline void LDX(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case LDX_IMM: val = FetchByte(); break;
		case LDX_ZP: val = FetchByteZP(); break;
		case LDX_ZPY: val = FetchByteZPY(); break;
		case LDX_ABS: val = FetchByteAbsolute(); break;
		case LDX_ABSY: val = FetchByteAbsoluteY(); break;
		}
		X = val;
		N = (X & 0b01000000) ? 1 : 0;
		Z = (X == 0) ? 1 : 0;
	}

	inline void LDY(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case LDY_IMM: val = FetchByte(); break;
		case LDY_ZP: val = FetchByteZP(); break;
		case LDY_ZPX: val = FetchByteZPX(); break;
		case LDY_ABS: val = FetchByteAbsolute(); break;
		case LDY_ABSX: val = FetchByteAbsoluteX(); break;
		}
		Y = val;
		N = (Y & 0b01000000) ? 1 : 0;
		Z = (Y == 0) ? 1 : 0;
	}


	void JMP(uint8_t itx) {
		uint16_t address = 0;
		switch (itx)
		{
		case JMP_ABS: address = FetchWord(); break;
		case JMP_IND: address = FetchIndirectAddress(); break;
		}

		PC = address;
	}

	void Run()
	{
		while (true)
		{
			auto itx = FetchInstruction();

			switch (itx)
			{
			case ADC_IMM:
			case ADC_ZP:
			case ADC_ZPX:
			case ADC_ABS:
			case ADC_ABSX:
			case ADC_ABSY:
			case ADC_INDX:
			case ADC_INDY:
				ADC(itx);
				break;

			case LDA_IMM:
			case LDA_ZP:
			case LDA_ZPX:
			case LDA_ABS:
			case LDA_ABSX:
			case LDA_ABSY:
			case LDA_INDX:
			case LDA_INDY:
				LDA(itx);
				break;

			case LDX_IMM:
			case LDX_ZP:
			case LDX_ZPY:
			case LDX_ABS:
			case LDX_ABSY:
				LDX(itx);
				break;

			case LDY_IMM:
			case LDY_ZP:
			case LDY_ZPX:
			case LDY_ABS:
			case LDY_ABSX:
				LDY(itx);
				break;

			case AND_IMM:
			case AND_ZP:
			case AND_ZPX:
			case AND_ABS:
			case AND_ABSX:
			case AND_ABSY:
			case AND_INDX:
			case AND_INDY:
				AND(itx);
				break;

			case ORA_IMM:
			case ORA_ZP:
			case ORA_ZPX:
			case ORA_ABS:
			case ORA_ABSX:
			case ORA_ABSY:
			case ORA_INDX:
			case ORA_INDY:
				ORA(itx);
				break;

			case EOR_IMM:
			case EOR_ZP:
			case EOR_ZPX:
			case EOR_ABS:
			case EOR_ABSX:
			case EOR_ABSY:
			case EOR_INDX:
			case EOR_INDY:
				EOR(itx);
				break;

			case NOP:
				break;

			case JMP_ABS:
			case JMP_IND:
				JMP(itx);
				break;

			default:
				std::cout << "Unknown instruction : " << std::hex << itx << std::endl;
				break;
			}
		}
	}

private:
	Memory* memory;
	uint16_t PC{}; // program counter
	uint8_t A{}; // accumulator
	uint8_t X{}; // x index
	uint8_t Y{}; // y index
	uint8_t S{}; // stack pointer
	uint8_t N : 1{}; // negative flag
	uint8_t V : 1{}; // overflow flag
	uint8_t B : 1{}; // break flag
	uint8_t D : 1{}; // decimal flag
	uint8_t I : 1{}; // interupt disable flag
	uint8_t Z : 1{}; // zero flag
	uint8_t C : 1{}; // carry flag
};


int main(int argc, char** argv)
{
	Memory memory;
	memory.LoadFromFile("6502_functional_test.bin");

	//int i = 0;
	//memory.WriteByte(0xFF + i++, LDA_IMM);
	//memory.WriteByte(0xFF + i++, 0x32);
	//memory.WriteByte(0xFF + i++, ADC_IMM);
	//memory.WriteByte(0xFF + i++, 0x23);

	CPU cpu(&memory);

	cpu.Run();

	return 0;
}

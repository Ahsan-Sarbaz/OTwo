#include <iostream>
#include <fstream>
#include <cstdint>

#include "defines.h"
#include "memory.h"


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

	void StackPush(uint8_t value)
	{
		memory->WriteByte(0x100 + S, value);
		if (S == 0x00) S = 0xFF;
		else S--;
	}

	uint8_t StackPop()
	{
		if (S == 0xFF) S = 0x00;
		else S++;
		return memory->ReadByte(0x100 + S);
	}

	void PHA()
	{
		StackPush(A);
		PC++;
	}

	void PHP()
	{
		uint8_t P = 0;
		P |= C ? 0b00000001 : 0;
		P |= Z ? 0b00000010 : 0;
		P |= I ? 0b00000100 : 0;
		P |= D ? 0b00001000 : 0;
		P |= B ? 0b00010000 : 0;
		P |= V ? 0b01000000 : 0;
		P |= N ? 0b10000000 : 0;
		StackPush(P);
		PC++;
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

			case NOP_IMP:
				break;

			case JMP_ABS:
			case JMP_IND:
				JMP(itx);
				break;

			case PHA_IMP:
				PHA();
				break;

			case PHP_IMP:
				PHP();
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
	if(!memory.LoadFromFile("6502_functional_test.bin"))
	{
		std::cout << "Failed to load memory" << std::endl;
		return 1;
	}

	//int i = 0;
	//memory.WriteByte(0xFF + i++, LDA_IMM);
	//memory.WriteByte(0xFF + i++, 0x32);
	//memory.WriteByte(0xFF + i++, ADC_IMM);
	//memory.WriteByte(0xFF + i++, 0x23);

	CPU cpu(&memory);

	cpu.Run();

	return 0;
}

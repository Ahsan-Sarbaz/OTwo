#include <iostream>
#include <fstream>
#include <cstdint>

#include "defines.h"
#include "memory.h"

class CPU
{

public:
	CPU(Memory *memory)
		: memory(memory)
	{
		Reset();
		PC = 0x400;
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

	uint16_t FetchIndirectAddress()
	{
		uint16_t ptr = FetchWord();
		uint16_t lsb = memory->ReadByte(ptr);
		uint16_t msb = memory->ReadByte((ptr & 0xFF00) | ((ptr + 1) & 0x00FF)); // Handle page boundary
		return (msb << 8) | lsb;
	}

	void WriteZPLastPC(uint8_t value)
	{
		// we move back one place to get the zp address from instruction stream
		memory->WriteByte(memory->ReadByte(PC - 1), value);
	}

	void WriteZPXLastPC(uint8_t value)
	{
		// we move back one place to get the zp address from instruction stream
		memory->WriteByte(memory->ReadByte(PC - 1) + X, value);
	}

	void WriteAbsoluteLastPC(uint8_t value)
	{
		memory->WriteByte(memory->ReadWord(PC - 2), value);
	}

	void WriteAbsoluteXLastPC(uint8_t value)
	{
		memory->WriteByte(memory->ReadWord(PC - 2) + X, value);
	}

	void StackPush(uint8_t value)
	{
		memory->WriteByte(0x100 + S, value);
		if (S == 0x00)
			S = 0xFF;
		else
			S--;
	}

	uint8_t StackPop()
	{
		if (S == 0xFF)
			S = 0x00;
		else
			S++;
		return memory->ReadByte(0x100 + S);
	}

	inline void ADC(uint8_t itx)
	{
		uint8_t n1 = 0;

		switch (itx)
		{
		case ADC_IMM:
			n1 = FetchByte();
			break;
		case ADC_ZP:
			n1 = FetchByteZP();
			break;
		case ADC_ZPX:
			n1 = FetchByteZPX();
			break;
		case ADC_ABS:
			n1 = FetchByteAbsolute();
			break;
		case ADC_ABSX:
			n1 = FetchByteAbsoluteX();
			break;
		case ADC_ABSY:
			n1 = FetchByteAbsoluteY();
			break;
		case ADC_INDX:
			n1 = FetchByteIndirectX();
			break;
		case ADC_INDY:
			n1 = FetchByteIndirectY();
			break;
		}
		auto n2 = A;
		uint16_t r = n1 + n2 + C;
		A = r & 0xFF;
		C = (r > 0xFF) ? 1 : 0;
		Z = (A == 0) ? 1 : 0;
		V = (((n1 > 0) && (n2 > 0) && (r < 0)) || ((n1 < 0) && (n2 < 0) && (r > 0))) ? 1 : 0;
		N = (A & 0b01000000) ? 1 : 0;
	}

	inline void SBC(uint8_t itx)
	{
		uint8_t n1 = 0;

		switch (itx)
		{
		case SBC_IMM:
			n1 = FetchByte();
			break;
		case SBC_ZP:
			n1 = FetchByteZP();
			break;
		case SBC_ZPX:
			n1 = FetchByteZPX();
			break;
		case SBC_ABS:
			n1 = FetchByteAbsolute();
			break;
		case SBC_ABSX:
			n1 = FetchByteAbsoluteX();
			break;
		case SBC_ABSY:
			n1 = FetchByteAbsoluteY();
			break;
		case SBC_INDX:
			n1 = FetchByteIndirectX();
			break;
		case SBC_INDY:
			n1 = FetchByteIndirectY();
			break;
		}
		auto n2 = A;
		uint16_t r = n1 + (~n2) + C;
		A = r & 0xFF;
		C = (r > 0xFF) ? 1 : 0;
		Z = (A == 0) ? 1 : 0;
	}

	inline void AND(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case AND_IMM:
			val = FetchByte();
			break;
		case AND_ZP:
			val = FetchByteZP();
			break;
		case AND_ZPX:
			val = FetchByteZPX();
			break;
		case AND_ABS:
			val = FetchByteAbsolute();
			break;
		case AND_ABSX:
			val = FetchByteAbsoluteX();
			break;
		case AND_ABSY:
			val = FetchByteAbsoluteY();
			break;
		case AND_INDX:
			val = FetchByteIndirectX();
			break;
		case AND_INDY:
			val = FetchByteIndirectY();
			break;
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
		case ORA_IMM:
			val = FetchByte();
			break;
		case ORA_ZP:
			val = FetchByteZP();
			break;
		case ORA_ZPX:
			val = FetchByteZPX();
			break;
		case ORA_ABS:
			val = FetchByteAbsolute();
			break;
		case ORA_ABSX:
			val = FetchByteAbsoluteX();
			break;
		case ORA_ABSY:
			val = FetchByteAbsoluteY();
			break;
		case ORA_INDX:
			val = FetchByteIndirectX();
			break;
		case ORA_INDY:
			val = FetchByteIndirectY();
			break;
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
		case EOR_IMM:
			val = FetchByte();
			break;
		case EOR_ZP:
			val = FetchByteZP();
			break;
		case EOR_ZPX:
			val = FetchByteZPX();
			break;
		case EOR_ABS:
			val = FetchByteAbsolute();
			break;
		case EOR_ABSX:
			val = FetchByteAbsoluteX();
			break;
		case EOR_ABSY:
			val = FetchByteAbsoluteY();
			break;
		case EOR_INDX:
			val = FetchByteIndirectX();
			break;
		case EOR_INDY:
			val = FetchByteIndirectY();
			break;
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
		case LDA_IMM:
			val = FetchByte();
			break;
		case LDA_ZP:
			val = FetchByteZP();
			break;
		case LDA_ZPX:
			val = FetchByteZPX();
			break;
		case LDA_ABS:
			val = FetchByteAbsolute();
			break;
		case LDA_ABSX:
			val = FetchByteAbsoluteX();
			break;
		case LDA_ABSY:
			val = FetchByteAbsoluteY();
			break;
		case LDA_INDX:
			val = FetchByteIndirectX();
			break;
		case LDA_INDY:
			val = FetchByteIndirectY();
			break;
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
		case LDX_IMM:
			val = FetchByte();
			break;
		case LDX_ZP:
			val = FetchByteZP();
			break;
		case LDX_ZPY:
			val = FetchByteZPY();
			break;
		case LDX_ABS:
			val = FetchByteAbsolute();
			break;
		case LDX_ABSY:
			val = FetchByteAbsoluteY();
			break;
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
		case LDY_IMM:
			val = FetchByte();
			break;
		case LDY_ZP:
			val = FetchByteZP();
			break;
		case LDY_ZPX:
			val = FetchByteZPX();
			break;
		case LDY_ABS:
			val = FetchByteAbsolute();
			break;
		case LDY_ABSX:
			val = FetchByteAbsoluteX();
			break;
		}
		Y = val;
		N = (Y & 0b01000000) ? 1 : 0;
		Z = (Y == 0) ? 1 : 0;
	}

	void JMP(uint8_t itx)
	{
		uint16_t address = 0;
		switch (itx)
		{
		case JMP_ABS:
			address = FetchWord();
			break;
		case JMP_IND:
			address = FetchIndirectAddress();
			break;
		}

		PC = address;
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

	void ASL(uint8_t itx)
	{
		uint8_t _val = 0;
		uint8_t val = 0;
		switch (itx)
		{
		case ASL_ACC:
		{
			_val = A;
			val = _val << 1;
			A = val;
		}
		break;
		case ASL_ZP:
		{
			_val = FetchByteZP();
			val = _val << 1;
			WriteZPLastPC(val);
		}
		break;
		case ASL_ZPX:
		{
			_val = FetchByteZPX();
			val = _val << 1;
			WriteZPXLastPC(val);
		}
		break;
		case ASL_ABS:
		{
			_val = FetchByteAbsolute();
			val = _val << 1;
			WriteAbsoluteLastPC(val);
		}
		break;
		case ASL_ABSX:
		{
			_val = FetchByteAbsoluteX();
			val = _val << 1;
			WriteAbsoluteXLastPC(val);
		}
		break;
		}

		C = (_val & 0b10000000) ? 1 : 0;
		N = (val & 0b10000000) ? 1 : 0;
		Z = (val == 0) ? 1 : 0;
	}

	void LSR(uint8_t itx)
	{
		uint8_t _val = 0;
		uint8_t val = 0;
		switch (itx)
		{
		case LSR_ACC:
		{
			_val = A;
			val = _val >> 1;
			A = val;
		}
		break;
		case LSR_ZP:
		{
			_val = FetchByteZP();
			val = _val >> 1;
			WriteZPLastPC(val);
		}
		break;
		case LSR_ZPX:
		{
			_val = FetchByteZPX();
			val = _val >> 1;
			WriteZPXLastPC(val);
		}
		break;
		case LSR_ABS:
		{
			_val = FetchByteAbsolute();
			val = _val >> 1;
			WriteAbsoluteLastPC(val);
		}
		break;
		case LSR_ABSX:
		{
			_val = FetchByteAbsoluteX();
			val = _val >> 1;
			WriteAbsoluteXLastPC(val);
		}
		break;
		}

		C = (_val & 0b00000001) ? 1 : 0;
		N = 0;
		Z = (val == 0) ? 1 : 0;
	}

	void ROL(uint8_t itx)
	{
		uint8_t _val = 0;
		uint8_t val = 0;
		switch (itx)
		{
		case ROL_ACC:
		{
			_val = A;
			val = (_val << 1) | C;
			A = val;
		}
		break;
		case ROL_ZP:
		{
			_val = FetchByteZP();
			val = (_val << 1) | C;
			WriteZPLastPC(val);
		}
		break;
		case ROL_ZPX:
		{
			_val = FetchByteZPX();
			val = (_val << 1) | C;
			WriteZPXLastPC(val);
		}
		break;
		case ROL_ABS:
		{
			_val = FetchByteAbsolute();
			val = (_val << 1) | C;
			WriteAbsoluteLastPC(val);
		}
		break;
		case ROL_ABSX:
		{
			_val = FetchByteAbsoluteX();
			val = (_val << 1) | C;
			WriteAbsoluteXLastPC(val);
		}
		break;
		}

		C = (_val & 0b10000000) ? 1 : 0;
		N = (val & 0b10000000) ? 1 : 0;
		Z = (val == 0) ? 1 : 0;
	}

	void ROR(uint8_t itx)
	{
		uint8_t _val = 0;
		uint8_t val = 0;
		switch (itx)
		{
		case ROR_ACC:
		{
			_val = A;
			val = (_val >> 1) | (C << 7);
			A = val;
		}
		break;
		case ROR_ZP:
		{
			_val = FetchByteZP();
			val = (_val >> 1) | (C << 7);
			WriteZPLastPC(val);
		}
		break;
		case ROR_ZPX:
		{
			_val = FetchByteZPX();
			val = (_val >> 1) | (C << 7);
			WriteZPXLastPC(val);
		}
		break;
		case ROR_ABS:
		{
			_val = FetchByteAbsolute();
			val = (_val >> 1) | (C << 7);
			WriteAbsoluteLastPC(val);
		}
		break;
		case ROR_ABSX:
		{
			_val = FetchByteAbsoluteX();
			val = (_val >> 1) | (C << 7);
			WriteAbsoluteXLastPC(val);
		}
		break;
		}

		C = (_val & 0b00000001) ? 1 : 0;
		N = (val & 0b10000000) ? 1 : 0;
		Z = (val == 0) ? 1 : 0;
	}

	void PLP()
	{
		uint8_t P = StackPop();
		C = (P & 0b00000001) == 1 ? 1 : 0;
		Z = (P & 0b00000010) == 1 ? 1 : 0;
		I = (P & 0b00000100) == 1 ? 1 : 0;
		D = (P & 0b00001000) == 1 ? 1 : 0;
		B = (P & 0b00010000) == 1 ? 1 : 0;
		V = (P & 0b01000000) == 1 ? 1 : 0;
		N = (P & 0b10000000) == 1 ? 1 : 0;
		PC++;
	}

	void PLA()
	{
		auto _A = StackPop();
		A = _A;
		Z = (A == 0) ? 1 : 0;
		N = (A & 0b10000000) ? 1 : 0;
	}

	void BCC()
	{
		if (C == 0)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BCS()
	{
		if (C == 1)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BEQ()
	{
		if (Z == 1)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BNE()
	{
		if (Z == 0)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BPL()
	{
		if (N == 0)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BMI()
	{
		if (N == 1)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BVC()
	{
		if (V == 0)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BVS()
	{
		if (V == 1)
		{
			int8_t rel_offset = (int8_t)FetchByte();
			PC += rel_offset;
		}
	}

	void BIT(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case BIT_ZP:
			val = FetchByteZP();
			break;
		case BIT_ABS:
			val = FetchByteAbsolute();
			break;
		}

		Z = ((val & A) == 0) ? 1 : 0;
		N = (val & 0b10000000) ? 1 : 0;
		V = (val & 0b01000000) ? 1 : 0;
	}

	void BRK()
	{
		uint8_t pclo = PC & 0xFF;
		uint8_t pchi = PC >> 8;
		StackPush(pchi);
		StackPush(pclo);
		uint8_t P = 0;
		P |= C ? 0b00000001 : 0;
		P |= Z ? 0b00000010 : 0;
		P |= I ? 0b00000100 : 0;
		P |= D ? 0b00001000 : 0;
		P |= B ? 0b00010000 : 0;
		P |= V ? 0b01000000 : 0;
		P |= N ? 0b10000000 : 0;
		StackPush(P);
		PC = memory->ReadWord(0xFFFE);
		B = 1;
	}

	void CMP(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case CMP_IMM:
			val = FetchByte();
			break;
		case CMP_ZP:
			val = FetchByteZP();
			break;
		case CMP_ZPX:
			val = FetchByteZPX();
			break;
		case CMP_ABS:
			val = FetchByteAbsolute();
			break;
		case CMP_ABSX:
			val = FetchByteAbsoluteX();
			break;
		case CMP_ABSY:
			val = FetchByteAbsoluteY();
			break;
		case CMP_INDX:
			val = FetchByteIndirectX();
			break;
		case CMP_INDY:
			val = FetchByteIndirectY();
			break;
		}

		Z = (A == val) ? 1 : 0;
		C = (A >= val) ? 1 : 0;
		N = ((A - val) & 0b10000000) ? 1 : 0;
	}

	void CPX(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case CPX_IMM:
			val = FetchByte();
			break;
		case CPX_ZP:
			val = FetchByteZP();
			break;
		case CPX_ABS:
			val = FetchByteAbsolute();
			break;
		}

		Z = (X == val) ? 1 : 0;
		C = (X >= val) ? 1 : 0;
		N = ((X - val) & 0b10000000) ? 1 : 0;
	}

	void CPY(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case CPY_IMM:
			val = FetchByte();
			break;
		case CPY_ZP:
			val = FetchByteZP();
			break;
		case CPY_ABS:
			val = FetchByteAbsolute();
			break;
		}

		Z = (Y == val) ? 1 : 0;
		C = (Y >= val) ? 1 : 0;
		N = ((Y - val) & 0b10000000) ? 1 : 0;
	}

	void DEC(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case DEC_ZP:
		{
			val = FetchByteZP() - 1;
			WriteZPLastPC(val);
		}
		break;
		case DEC_ZPX:
		{
			val = FetchByteZPX() - 1;
			WriteZPXLastPC(val);
		}
		break;
		case DEC_ABS:
		{
			val = FetchByteAbsolute() - 1;
			WriteAbsoluteLastPC(val);
		}
		break;
		case DEC_ABSX:
		{
			val = FetchByteAbsoluteX() - 1;
			WriteAbsoluteXLastPC(val);
		}
		break;
		}

		Z = (val == 0) ? 1 : 0;
		N = (val & 0b10000000) ? 1 : 0;
	}

	void INC(uint8_t itx)
	{
		uint8_t val = 0;
		switch (itx)
		{
		case INC_ZP:
		{
			val = FetchByteZP() + 1;
			WriteZPLastPC(val);
		}
		break;
		case INC_ZPX:
		{
			val = FetchByteZPX() + 1;
			WriteZPXLastPC(val);
		}
		break;
		case INC_ABS:
		{
			val = FetchByteAbsolute() + 1;
			WriteAbsoluteLastPC(val);
		}
		break;
		case INC_ABSX:
		{
			val = FetchByteAbsoluteX() + 1;
			WriteAbsoluteXLastPC(val);
		}
		break;
		}

		Z = (val == 0) ? 1 : 0;
		N = (val & 0b10000000) ? 1 : 0;
	}

	void JSR()
	{
		uint16_t pc = PC - 1;
		StackPush(pc >> 8);
		StackPush(pc & 0xFF);

		PC = FetchWord();
	}

	void RTS()
	{
		uint16_t pc = StackPop() << 8 | StackPop();
		PC = pc + 1;
	}

	void RTI()
	{
		uint8_t P = StackPop();
		C = (P & 0b00000001) == 1 ? 1 : 0;
		Z = (P & 0b00000010) == 1 ? 1 : 0;
		I = (P & 0b00000100) == 1 ? 1 : 0;
		D = (P & 0b00001000) == 1 ? 1 : 0;
		B = (P & 0b00010000) == 1 ? 1 : 0;
		V = (P & 0b01000000) == 1 ? 1 : 0;
		N = (P & 0b10000000) == 1 ? 1 : 0;

		PC = StackPop() << 8 | StackPop();
	}

	void STA(uint8_t itx)
	{
		switch (itx)
		{
		case STA_ZP:
		{
			uint8_t val = FetchByteZP();
			memory->WriteByte(val, A);
		}
		break;
		case STA_ZPX:
		{
			uint8_t val = FetchByteZPX();
			memory->WriteByte(val, A);
		}
		break;
		case STA_ABS:
		{
			uint8_t val = FetchByteAbsolute();
			memory->WriteByte(val, A);
		}
		break;
		case STA_ABSX:
		{
			uint8_t val = FetchByteAbsoluteX();
			memory->WriteByte(val, A);
		}
		break;
		case STA_ABSY:
		{
			uint8_t val = FetchByteAbsoluteY();
			memory->WriteByte(val, A);
		}
		break;
		case STA_INDX:
		{
			uint8_t val = FetchByteIndirectX();
			memory->WriteByte(val, A);
		}
		break;
		case STA_INDY:
		{
			uint8_t val = FetchByteIndirectY();
			memory->WriteByte(val, A);
		}
		break;
		}
	}

	void STX(uint8_t itx)
	{
		switch (itx)
		{
		case STX_ZP:
		{
			uint8_t val = FetchByteZP();
			memory->WriteByte(val, X);
		}
		break;
		case STX_ZPY:
		{
			uint8_t val = FetchByteZPY();
			memory->WriteByte(val, X);
		}
		break;
		case STX_ABS:
		{
			uint8_t val = FetchByteAbsolute();
			memory->WriteByte(val, X);
		}
		break;
		}
	}

	void STY(uint8_t itx)
	{
		switch (itx)
		{
		case STY_ZP:
		{
			uint8_t val = FetchByteZP();
			memory->WriteByte(val, Y);
		}
		break;
		case STY_ZPX:
		{
			uint8_t val = FetchByteZPX();
			memory->WriteByte(val, Y);
		}
		break;
		case STY_ABS:
		{
			uint8_t val = FetchByteAbsolute();
			memory->WriteByte(val, Y);
		}
		break;
		}
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

			case SBC_IMM:
			case SBC_ZP:
			case SBC_ZPX:
			case SBC_ABS:
			case SBC_ABSX:
			case SBC_ABSY:
			case SBC_INDX:
			case SBC_INDY:
				SBC(itx);
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
			case PLA_IMP:
				PLA();
				break;
			case PLP_IMP:
				PLP();
				break;

			case ASL_ACC:
			case ASL_ZP:
			case ASL_ZPX:
			case ASL_ABS:
			case ASL_ABSX:
				ASL(itx);
				break;

			case LSR_ACC:
			case LSR_ZP:
			case LSR_ZPX:
			case LSR_ABS:
			case LSR_ABSX:
				LSR(itx);
				break;

			case ROL_ACC:
			case ROL_ZP:
			case ROL_ZPX:
			case ROL_ABS:
			case ROL_ABSX:
				ROL(itx);
				break;

			case ROR_ACC:
			case ROR_ZP:
			case ROR_ZPX:
			case ROR_ABS:
			case ROR_ABSX:
				ROR(itx);
				break;

			case BCC_REL:
				BCC();
				break;
			case BCS_REL:
				BCS();
				break;
			case BEQ_REL:
				BEQ();
				break;
			case BNE_REL:
				BNE();
				break;
			case BMI_REL:
				BMI();
				break;
			case BPL_REL:
				BPL();
				break;
			case BVC_REL:
				BVC();
				break;
			case BVS_REL:
				BVS();
				break;

			case BIT_ZP:
			case BIT_ABS:
				BIT(itx);
				break;

			case BRK_IMP:
				BRK();
				break;
			case CLC_IMP:
				C = 0;
				break;
			case CLD_IMP:
				D = 0;
				break;
			case CLI_IMP:
				I = 0;
				break;
			case CLV_IMP:
				V = 0;
				break;
			case SEC_IMP:
				C = 1;
				break;
			case SED_IMP:
				D = 1;
				break;
			case SEI_IMP:
				I = 1;
				break;

			case CMP_IMM:
			case CMP_ZP:
			case CMP_ZPX:
			case CMP_ABS:
			case CMP_ABSX:
			case CMP_ABSY:
			case CMP_INDX:
			case CMP_INDY:
				CMP(itx);
				break;

			case CPX_IMM:
			case CPX_ZP:
			case CPX_ABS:
				CPX(itx);
				break;

			case CPY_IMM:
			case CPY_ZP:
			case CPY_ABS:
				CPY(itx);
				break;

			case DEC_ZP:
			case DEC_ZPX:
			case DEC_ABS:
			case DEC_ABSX:
				DEC(itx);
				break;

			case DEX_IMP:
			{
				X--;
				N = (X & 0b10000000) ? 1 : 0;
				Z = (X == 0) ? 1 : 0;
			}
			break;
			case DEY_IMP:
			{
				Y--;
				N = (Y & 0b10000000) ? 1 : 0;
				Z = (Y == 0) ? 1 : 0;
			}
			break;

			case INC_ZP:
			case INC_ZPX:
			case INC_ABS:
			case INC_ABSX:
				INC(itx);
				break;

			case INX_IMP:
				X++;
				N = (X & 0b10000000) ? 1 : 0;
				Z = (X == 0) ? 1 : 0;
				break;
			case INY_IMP:
				Y++;
				N = (Y & 0b10000000) ? 1 : 0;
				Z = (Y == 0) ? 1 : 0;
				break;

			case JSR_ABS:
				JSR();
				break;
			case RTS_IMP:
				RTS();
				break;
			case RTI_IMP:
				RTI();
				break;

			case STA_ZP:
			case STA_ZPX:
			case STA_ABS:
			case STA_ABSX:
			case STA_ABSY:
			case STA_INDX:
			case STA_INDY:
				STA(itx);
				break;

			case STX_ZP:
			case STX_ZPY:
			case STX_ABS:
				STX(itx);
				break;

			case STY_ZP:
			case STY_ZPX:
			case STY_ABS:
				STY(itx);
				break;

			case TAX_IMP:
			{
				X = A;
				N = (X & 0b10000000) ? 1 : 0;
				Z = (X == 0) ? 1 : 0;
			}
			break;

			case TAY_IMP:
			{
				Y = A;
				N = (Y & 0b10000000) ? 1 : 0;
				Z = (Y == 0) ? 1 : 0;
			}
			break;

			case TSX_IMP:
			{
				X = S;
				N = (X & 0b10000000) ? 1 : 0;
				Z = (X == 0) ? 1 : 0;
			}
			break;

			case TXA_IMP:
			{
				A = X;
				N = (A & 0b10000000) ? 1 : 0;
				Z = (A == 0) ? 1 : 0;
			}
			break;

			case TXS_IMP:
			{
				S = X;
			}
			break;

			case TYA_IMP:
			{
				A = Y;
				N = (A & 0b10000000) ? 1 : 0;
				Z = (A == 0) ? 1 : 0;
			}
			break;

			default:
				std::cout << "Unknown instruction : " << std::hex << itx << std::endl;
				break;
			}
		}
	}

private:
	Memory *memory;
	uint16_t PC{};	  // program counter
	uint8_t A{};	  // accumulator
	uint8_t X{};	  // x index
	uint8_t Y{};	  // y index
	uint8_t S{};	  // stack pointer
	uint8_t N : 1 {}; // negative flag
	uint8_t V : 1 {}; // overflow flag
	uint8_t B : 1 {}; // break flag
	uint8_t D : 1 {}; // decimal flag
	uint8_t I : 1 {}; // interupt disable flag
	uint8_t Z : 1 {}; // zero flag
	uint8_t C : 1 {}; // carry flag
};

int main(int argc, char **argv)
{
	Memory memory;
	if (!memory.LoadFromFile("6502_functional_test.bin"))
	{
		std::cout << "Failed to load memory" << std::endl;
		return 1;
	}

	// int i = 0;
	// memory.WriteByte(0xFF + i++, LDA_IMM);
	// memory.WriteByte(0xFF + i++, 0x32);
	// memory.WriteByte(0xFF + i++, ADC_IMM);
	// memory.WriteByte(0xFF + i++, 0x23);

	CPU cpu(&memory);

	cpu.Run();

	return 0;
}

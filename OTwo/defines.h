#pragma once

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
#define BNE_REL  0xD0
#define BMI_REL  0x30
#define BPL_REL  0x10
#define BVC_REL  0x50
#define BVS_REL  0x70

#define BIT_ZP   0x24
#define BIT_ABS  0x2C

#define BRK_IMP  0x00

#define NOP_IMP 0xEA

#define PHA_IMP 0x48
#define PHP_IMP 0x08
#define PLA_IMP 0x68
#define PLP_IMP 0x28

#define JMP_ABS 0x4C
#define JMP_IND 0x6C

#define ASL_ACC  0x0A
#define ASL_ZP	 0x06
#define ASL_ZPX  0x16
#define ASL_ABS  0x0E
#define ASL_ABSX 0x1E

#define LSR_ACC  0x4A
#define LSR_ZP	 0x46
#define LSR_ZPX  0x56
#define LSR_ABS  0x4E
#define LSR_ABSX 0x5E

#define ROL_ACC  0x2A
#define ROL_ZP	 0x26
#define ROL_ZPX  0x36
#define ROL_ABS  0x2E
#define ROL_ABSX 0x3E

#define ROR_ACC  0x6A
#define ROR_ZP	 0x66
#define ROR_ZPX  0x76
#define ROR_ABS  0x6E
#define ROR_ABSX 0x7E

#define CLC_IMP  0x18
#define CLD_IMP  0xD8
#define CLI_IMP  0x58
#define CLV_IMP  0xB8

#define CMP_IMM  0xC9
#define CMP_ZP	 0xC5
#define CMP_ZPX  0xD5
#define CMP_ABS  0xCD
#define CMP_ABSX 0xDD
#define CMP_ABSY 0xD9
#define CMP_INDX 0xC1
#define CMP_INDY 0xD1

#define CPX_IMM  0xE0
#define CPX_ZP	 0xE4
#define CPX_ABS  0xEC

#define CPY_IMM  0xC0
#define CPY_ZP	 0xC4
#define CPY_ABS  0xCC

#define DEC_ZP	 0xC6
#define DEC_ZPX	 0xD6
#define DEC_ABS  0xCE
#define DEC_ABSX 0xDE


#include "as.h"

// finire con enum ADDR_MODE !!

#if Z80
struct SUB_CODE sMn1[] = {
  ADDR_10,0xa8,4, ADDR_IMM8,0xce,7, ADDR_IX_IY,0xdd8e,19, ADDR_9,0x8e,7, ADDR_IMM16,0xed4a,15, ADDR_REG8,0x88,4, 0,0,0
  };
  
struct SUB_CODE sMn2[] = {
  ADDR_REG8,0x80,4, ADDR_IMM16,0x09,11, ADDR_IMM8,0xc6,4, ADDR_IX_IY,0xdd86,19, ADDR_7,0xdd09,15, ADDR_8,0xfd09,15, ADDR_9,0x86,7, 0,0,0
  };
  
struct SUB_CODE sMn3[] = {
  ADDR_10,0xa0,4, ADDR_IMM8,0xe6,7, ADDR_IX_IY,0xdda6,19, ADDR_IY,0xfda6,19, ADDR_9,0xa6,7, 0,0,0
  };
  
struct SUB_CODE sMn4[] = {
  ADDR_BIT,0xcb40,8, ADDR_BIT2,0xcb46,12, ADDR_BITXY,0xddcb46,20, 0,0,0
  };
  
struct SUB_CODE sMn5[] = {
  ADDR_IMM8,0xcd,0, ADDR_14,0xcd,17, ADDR_CONDIZ_IMM,0xc4,0, 0,0,0
  };
  
struct SUB_CODE sMn6[] = {
  ADDR_IMPL,0x3f,4, 0,0,0
  };
  
struct SUB_CODE sMn7[] = {
  ADDR_10,0xb8,4, ADDR_IMM8,0xfe,7, ADDR_9,0xbe,7, ADDR_IX_IY,0xddbe,19, 0,0,0
  };
  
struct SUB_CODE sMn8[] = {
  ADDR_IMPL,0xeda9,4, 0,0,0
  };
  
struct SUB_CODE sMn9[] = {
  ADDR_IMPL,0xedb9,4, 0,0,0
  };
  
struct SUB_CODE sMn10[] = {
  ADDR_IMPL,0xeda1,4, 0,0,0
  };
  
struct SUB_CODE sMn11[] = {
  ADDR_IMPL,0xedb1,4, 0,0,0
  };
  
struct SUB_CODE sMn12[] = {
  ADDR_IMPL,0x2f,4, 0,0,0
  };
  
struct SUB_CODE sMn13[] = {
  ADDR_IMPL,0x27,4, 0,0,0
  };
  
struct SUB_CODE sMn14[] = {
  ADDR_10,0x5,4, ADDR_9,0x35,11, ADDR_IMM16,0xb,6, ADDR_15,0xdd2b,10, ADDR_16,0xfd2b,10, ADDR_IX_IY,0xdd35,23, ADDR_31,0x3b,6, 0,0,0
  };
  
struct SUB_CODE sMn15[] = {
  ADDR_IMPL,0xf3,4, 0,0,0
  };
  
struct SUB_CODE sMn16[] = {
  ADDR_IMM8,0x10,0, ADDR_14,0x10,13 << 8 | 8, 0,0,0
  };
  
struct SUB_CODE sMn17[] = {
  ADDR_IMPL,0xfb,4, 0,0,0
  };
  
struct SUB_CODE sMn18[] = {
  ADDR_30,0xe3/* gestito a sè*/,0, ADDR_IMM16,0xeb,0, 0,0,0
  };
  
struct SUB_CODE sMn19[] = {
  ADDR_IMPL,0xd9,4, 0,0,0
  };
  
struct SUB_CODE sMn20[] = {
  ADDR_IMPL,0x76,4, 0,0,0
  };
  
struct SUB_CODE sMn21[] = {
  ADDR_IMM8,0xed46, 0,0,0
  };
  
struct SUB_CODE sMn22[] = {
  ADDR_29,0xed40,0, ADDR_21,0xdb,0, 0,0,0
  };
  
struct SUB_CODE sMn23[] = {
  ADDR_10,0x4,4, ADDR_IMM16,0x3,6, ADDR_9,0x34,11, ADDR_IX_IY,0xdd34,23, ADDR_15,0xdd23,10, ADDR_16,0xfd23,10, ADDR_31,0x33,6, 0,0,0
  };
  
struct SUB_CODE sMn24[] = {
  ADDR_IMPL,0xedaa,4, 0,0,0
  };
  
struct SUB_CODE sMn25[] = {
  ADDR_IMPL,0xedba,4, 0,0,0
  };
  
struct SUB_CODE sMn26[] = {
  ADDR_IMPL,0xeda2,4, 0,0,0
  };
  
struct SUB_CODE sMn27[] = {
  ADDR_IMPL,0xedb2,4, 0,0,0
  };
  
struct SUB_CODE sMn28[] = {
  ADDR_IMM8,0xc3,10, ADDR_14,0xc3,10, ADDR_CONDIZ_IMM,0xc2,10 << 8 | 10, ADDR_9,0xe9,4, ADDR_12,0xdde9,8, 0,0,0    // accetto sia imm8 che imm16
  };
  
struct SUB_CODE sMn29[] = {
  ADDR_IMM8,0x18,12, ADDR_14,0x18,12, ADDR_CONDIZ_IMM,0x20,12 << 8 | 7, 0,0,0
  };
  
struct SUB_CODE sMn30[] = {
  ADDR_REG8,0x78,4, ADDR_IMM8,0x3e/*sia A che r8*/,7, ADDR_32,0x46/*gestito anche per bc..*/,7, ADDR_9,0x7e,7, ADDR_10,0x40,4, ADDR_11,0x6,7, 
	ADDR_IX_IY,0xdd7e,19, ADDR_33,0xdd46,19, ADDR_34,0xfd46,19, ADDR_17,0x32,13,
  ADDR_18,0x22/*gestito a sé*/,16, ADDR_19,0x36,10, ADDR_20,0x70/*idem*/,7, ADDR_21,0x3a,13, ADDR_22,0x2a/**/,16, ADDR_23,0x1,10, ADDR_24,0xdd70,19, ADDR_25,0xfd70,19,
  ADDR_63,0xed4f,9, ADDR_62,0xed47,9, ADDR_61,0xed5f,9, ADDR_60,0xed57,9, ADDR_26,0xdd36,19, ADDR_27,0xfd36,19, 0,0,0
  };
  
struct SUB_CODE sMn31[] = {
  ADDR_IMPL,0xeda8,16, 0,0,0
  };
  
struct SUB_CODE sMn32[] = {
  ADDR_IMPL,0xedb8,16, 0,0,0
  };
  
struct SUB_CODE sMn33[] = {
  ADDR_IMPL,0xeda0,16, 0,0,0
  };
  
struct SUB_CODE sMn34[] = {
  ADDR_IMPL,0xedb0,21 << 8 | 16, 0,0,0
  };
  
struct SUB_CODE sMn35[] = {
  ADDR_IMPL,0xed44,4, 0,0,0
  };
  
struct SUB_CODE sMn36[] = {
  ADDR_IMPL,0x0,4, 0,0,0
  };
  
struct SUB_CODE sMn37[] = {
  ADDR_10,0xb0,4, ADDR_IMM8,0xf6,7, ADDR_9,0xb6,7, ADDR_IX_IY,0xddb6,19, 0,0,0
  };
  
struct SUB_CODE sMn38[] = {
  ADDR_IMPL,0xedbb,21 << 8 | 16, 0,0,0
  };
  
struct SUB_CODE sMn39[] = {
  ADDR_IMPL,0xedb3,20 << 8 | 15, 0,0,0
  };
  
struct SUB_CODE sMn40[] = {
  ADDR_28,0xed41,12, ADDR_17,0xd3,11, 0,0,0
  };
  
struct SUB_CODE sMn41[] = {
  ADDR_IMPL,0xedab,15, 0,0,0
  };
  
struct SUB_CODE sMn42[] = {
  ADDR_IMPL,0xeda3,15, 0,0,0
  };
  
struct SUB_CODE sMn43[] = {
  ADDR_IMM16,0xc1,10, ADDR_15,0xdde1,14, ADDR_16,0xfde1,14, 0,0,0
  };
  
struct SUB_CODE sMn44[] = {
  ADDR_IMM16,0xc5,11, ADDR_15,0xdde5,15, ADDR_16,0xfde5,15, 0,0,0
  };
  
struct SUB_CODE sMn45[] = {
  ADDR_BIT,0xcb80,8, ADDR_BIT2,0xcb86,15, ADDR_BITXY,0xddcb86,23, 0,0,0
  };
  
struct SUB_CODE sMn46[] = {
  ADDR_IMPL,0xc9,10, ADDR_CONDIZ,0xc0,11 << 8 | 5, 0,0,0
  };
  
struct SUB_CODE sMn47[] = {
  ADDR_IMPL,0xed4d,14, 0,0,0
  };
  
struct SUB_CODE sMn48[] = {
  ADDR_IMPL,0xed45,14, 0,0,0
  };
  
struct SUB_CODE sMn49[] = {
  10,0xcb10,4, 9,0xcb16,0, ADDR_IX_IY,0xddcb16,23, 0,0,0
  };
  
struct SUB_CODE sMn50[] = {
  ADDR_IMPL,0x17,4, 0,0,0
  };
  
struct SUB_CODE sMn51[] = {
  10,0xcb00,4, 9,0xcb06,0, ADDR_IX_IY,0xddcb06,23, 0,0,0
  };
  
struct SUB_CODE sMn52[] = {
  ADDR_IMPL,0x7,4, 0,0,0
  };
  
struct SUB_CODE sMn53[] = {
  ADDR_IMPL,0xed6f,4, 0,0,0
  };
  
struct SUB_CODE sMn54[] = {
  ADDR_10,0xcb18,4, 9,0xcb1e,0, ADDR_IX_IY,0xddcb1e,23, 0,0,0
  };
  
struct SUB_CODE sMn55[] = {
  ADDR_IMPL,0x1f,4, 0,0,0
  };
  
struct SUB_CODE sMn56[] = {
  ADDR_10,0xcb08,4, ADDR_9,0xcb0e,15, ADDR_IX_IY,0xddcb0e,23, 0,0,0
  };
  
struct SUB_CODE sMn57[] = {
  ADDR_IMPL,0xf,4, 0,0,0
  };
  
struct SUB_CODE sMn58[] = {
  ADDR_IMPL,0xed67,4, 0,0,0
  };
  
struct SUB_CODE sMn59[] = {
  ADDR_IMM8,0xc7,11, 0,0,0
  };
  
struct SUB_CODE sMn60[] = {
  ADDR_10,0x98,4, ADDR_IMM8,0xde,7, ADDR_IMM16,0xed42,15, ADDR_9,0x9e,7, ADDR_2,0x98,4, ADDR_IX_IY,0xdd9e,19, 0,0,0
  };
  
struct SUB_CODE sMn61[] = {
  ADDR_IMPL,0x37,4, 0,0,0
  };
  
struct SUB_CODE sMn62[] = {
  ADDR_BIT,0xcbc0,8, ADDR_BIT2,0xcbc6,15, ADDR_BITXY,0xddcbc6,23, ADDR_53,0xfdcbc6,23, 0,0,0
  };
  
struct SUB_CODE sMn63[] = {
  ADDR_10,0xcb20,4, ADDR_9,0xcb26,15, ADDR_IX_IY,0xddcb26,23, 0,0,0
  };
  
struct SUB_CODE sMn64[] = {
  ADDR_10,0xcb28,4, ADDR_9,0xcb2e,15, ADDR_IX_IY,0xddcb2e,23, 0,0,0
  };
  
struct SUB_CODE sMn65[] = {
  ADDR_10,0xcb38,4, ADDR_9,0xcb3e,15, ADDR_IX_IY,0xddcb3e,23, 0,0,0
  };
  
struct SUB_CODE sMn66[] = {
  ADDR_10,0x90,4, ADDR_IMM8,0xd6,7, ADDR_9,0x96,7, ADDR_IX_IY,0xdd96,19, 0,0,0
  };
  
struct SUB_CODE sMn67[] = {
  ADDR_10,0xa8,4, ADDR_IMM8,0xee,7, ADDR_9,0xae,7, ADDR_IX_IY,0xddae,19, 0,0,0
  };
  
struct CODE Mnem[] = {
// il secondo valore (flag M) vale 1 per implicito
// 2 per A,r8 bit
// 3 per HL,r16 bit
// 4 per A,imm8
// 5 per A,(IX+), 6 per A,(IY+); 33 r8,(IX+);  34 r8,(IY+)
// 7 per IX,r16, 8 per IY,r16
// 9 per A,ind HL ;  32: r8,(HL)
// 10 per r8,r8 (o solo r8), 11 per r8,imm, 12 per (IX), 13 per (IY)
// 14 per imm16, 15 per HL,ix, 16 per HL,iy; 31 per HL,sp
// 17: (nn),A;  18: (nn),r16;  19: (r16),nn;  20: (r16),r8
// 21: A,(nn); 22: r16,(nn);  23: r16,imm;  24: (IX+),r8;  25: (IY+),r8; 26: (IX+),imm;  27: (IY+),imm
// 28: (r8),r8;  29: r8,(r8); 30: (r16),r16;  
// 41 per condiz, 40 condiz,imm
// 50: set 0,r8; 51 set 0,(hl); 52 set 0,(ix);  53: set 0,(iy)
// 60 per A,I; 61 per A,R; 62 per I,A; 63 per R,A
  "ADC",sMn1,
  "ADD",sMn2,
  "AND",sMn3,
  "BIT",sMn4,
  "CALL",sMn5,
  "CCF",sMn6,
  "CP",sMn7,
  "CPD",sMn8,
  "CPDR",sMn9,
  "CPI",sMn10,
  "CPIR",sMn11,
  "CPL",sMn12,
  "DAA",sMn13,
  "DEC",sMn14,
  "DI",sMn15, 
  "DJNZ",sMn16,
  "EI",sMn17,
  "EX",sMn18,
  "EXX",sMn19,
  "HALT",sMn20,
  "IM",sMn21,
  "IN",sMn22,
  "INC",sMn23,
  "IND",sMn24,
  "INDR",sMn25,
  "INI",sMn26,
  "INIR",sMn27,
  "JP",sMn28,
  "JR",sMn29,
  "LD",sMn30,
  "LDD",sMn31,
  "LDDR",sMn32,
  "LDI",sMn33,
  "LDIR",sMn34,
  "NEG",sMn35,
  "NOP",sMn36,
  "OR",sMn37,
  "OTDR",sMn38,
  "OTIR",sMn39,
  "OUT",sMn40,
  "OUTD",sMn41,
  "OUTI",sMn42,
  "POP",sMn43,
  "PUSH",sMn44,
  "RES",sMn45,
  "RET",sMn46,
  "RETI",sMn47,
  "RETN",sMn48,
  "RL",sMn49,
  "RLA",sMn50,
  "RLC",sMn51,
  "RLCA",sMn52,
  "RLD",sMn53,
  "RR",sMn54,
  "RRA",sMn55,
  "RRC",sMn56,
  "RRCA",sMn57,
  "RRD",sMn58,
  "RST",sMn59,
  "SBC",sMn60,
  "SCF",sMn61,
  "SET",sMn62,
  "SLA",sMn63,
  "SRA",sMn64,
  "SRL",sMn65,
  "SUB",sMn66,
  "XOR",sMn67
  };
  
#elif _6502

struct SUB_CODE sMn1[] = {
  ADDR_IMM8,0x69,2, ADDR_ZP,0x65,3, ADDR_ZPX,0x75,4, ADDR_ABS,0x6d,4, ADDR_ABSX,0x7d,4, ADDR_ABSY,0x79,4, 10,0x61,6, 11,0x71,5, 0,0,0
  };
  
struct SUB_CODE sMn2[] = {
  ADDR_IMM8,0x29,2, ADDR_ZP,0x25,3, ADDR_ZPX,0x35,4, ADDR_ABS,0x2d,4, ADDR_ABSX,0x3d,4, ADDR_ABSY,0x39,4, 10,0x21,6, 11,0x31,5, 0,0,0
  };
  
struct SUB_CODE sMn3[] = {
  ADDR_IMPL,0xa,2, ADDR_ZP,0x6,5, ADDR_ZPX,0x16,6, ADDR_ABS,0xe,6, ADDR_ABSX,0x1e,7, 0,0,0
  };
  
struct SUB_CODE sMn4[] = {
  ADDR_ABS,0x90,2, 0,0,0
  };
  
struct SUB_CODE sMn5[] = {
  ADDR_ABS,0xb0,2, 0,0,0
  };
  
struct SUB_CODE sMn6[] = {
  ADDR_ABS,0xf0,2, 0,0,0
  };
  
struct SUB_CODE sMn7[] = {
  ADDR_ZP,0x24,3, ADDR_ABS,0x2c,4, 0,0,0
  };
  
struct SUB_CODE sMn8[] = {
  ADDR_ABS,0x30,2, 0,0,0
  };
  
struct SUB_CODE sMn9[] = {
  ADDR_ABS,0xd0,2, 0,0,0
  };
  
struct SUB_CODE sMn10[] = {
  ADDR_ABS,0x10,2, 0,0,0
  };
  
struct SUB_CODE sMn11[] = {
  ADDR_IMPL,0x00,7, 0,0,0
  };
  
	struct SUB_CODE sMn12[] = {
		ADDR_IMPL,0x80,7, 0,0,0
		};
  
struct SUB_CODE sMn13[] = {
  ADDR_ABS,0x50,2, 0,0,0
  };
  
struct SUB_CODE sMn14[] = {
  ADDR_ABS,0x70,2, 0,0,0
  };
  
struct SUB_CODE sMn15[] = {
  ADDR_IMPL,0x18,2, 0,0,0
  };
  
struct SUB_CODE sMn16[] = {
  ADDR_IMPL,0xd8,2, 0,0,0
  };
  
struct SUB_CODE sMn17[] = {
  ADDR_IMPL,0x58,2, 0,0,0
  };
  
struct SUB_CODE sMn18[] = {
  ADDR_IMPL,0xb8,2, 0,0,0
  };
  
struct SUB_CODE sMn19[] = {
  ADDR_IMM8,0xc9,2, ADDR_ZP,0xc5,3, ADDR_ZPX,0xd5,4, ADDR_ABS,0xcd,4, ADDR_ABSX,0xdd,4, ADDR_ABSY,0xd9,4, 10,0xc1,6, 11,0xd1,5, 0,0,0
  };
  
struct SUB_CODE sMn20[] = {
  ADDR_IMM8,0xe0,2, ADDR_ZP,0xe4,3, ADDR_ABS,0xec,4, 0,0,0
  };
  
struct SUB_CODE sMn21[] = {
  ADDR_IMM8,0xc0,2, ADDR_ZP,0xc4,3, ADDR_ABS,0xcc,4, 0,0,0
  };
  
struct SUB_CODE sMn22[] = {
  ADDR_ZP,0xc6,5, 6,0xd6,6, ADDR_ABS,0xce,6, ADDR_ABSX,0xde,7, 0,0,0
  };
  
struct SUB_CODE sMn23[] = {
  ADDR_IMPL,0xca,2, 0,0,0
  };
  
struct SUB_CODE sMn24[] = {
  ADDR_IMPL,0x88,2, 0,0,0
  };
  
struct SUB_CODE sMn25[] = {
  ADDR_IMM8,0x49,2, ADDR_ZP,0x45,3, ADDR_ZPX,0x55,4, ADDR_ABS,0x4d,4, ADDR_ABSX,0x5d,4, ADDR_ABSY,0x59,4, 10,0x41,6, 11,0x51,5, 0,0,0
  };
  
struct SUB_CODE sMn26[] = {
  ADDR_ZP,0xe6,5, ADDR_ZPX,0xf6,6, ADDR_ABS,0xee,6, ADDR_ABSX,0xfe,7, 0,0,0
  };
  
struct SUB_CODE sMn27[] = {
  ADDR_IMPL,0xe8,2, 0,0,0
  };
  
struct SUB_CODE sMn28[] = {
  ADDR_IMPL,0xc8,2, 0,0,0
  };
  
struct SUB_CODE sMn29[] = {
  ADDR_ABS,0x4c,3, 12,0x6c,5, 0,0,0
  };
  
struct SUB_CODE sMn30[] = {
  ADDR_ABS,0x20,6, 0,0,0
  };
  
struct SUB_CODE sMn31[] = {
  ADDR_IMM8,0xa9,2, ADDR_ZP,0xa5,3, ADDR_ZPX,0xb5,4, ADDR_ABS,0xad,4, ADDR_ABSX,0xbd,4, ADDR_ABSY,0xb9,4, 10,0xa1,4, 11,0xb1,5, 0,0,0
  };
  
struct SUB_CODE sMn32[] = {
  ADDR_IMM8,0xa2,2, ADDR_ZP,0xa6,3, ADDR_ZPY,0xb6,4, ADDR_ABS,0xae,4, ADDR_ABSY,0xbe,4, 0,0,0
  };
  
struct SUB_CODE sMn33[] = {
  ADDR_IMM8,0xa0,2, ADDR_ZP,0xa4,3, ADDR_ZPX,0xb4,4, ADDR_ABS,0xac,4, ADDR_ABSX,0xbc,4, 0,0,0
  };
  
struct SUB_CODE sMn34[] = {
  ADDR_REG8,0x4a,2, ADDR_ZP,0x46,5, ADDR_ZPX,0x56,6, ADDR_ABS,0x4e,6, ADDR_ABSX,0x5e,7, 0,0,0
  };
  
struct SUB_CODE sMn35[] = {
  ADDR_IMPL,0xea,2, 0,0,0
  };
  
struct SUB_CODE sMn36[] = {
  ADDR_IMM8,0x09,2, ADDR_ZP,0x05,3, ADDR_ZPX,0x15,4, ADDR_ABS,0x0d,4, ADDR_ABSX,0x1d,4, ADDR_ABSY,0x19,4, 10,0x01,4, 11,0x11,5, 0,0,0
  };
  
struct SUB_CODE sMn37[] = {
  ADDR_IMPL,0x48,3, 0,0,0
  };
  
struct SUB_CODE sMn38[] = {
  ADDR_IMPL,0x8,3, 0,0,0
  };
  
	struct SUB_CODE sMn39[] = {
		ADDR_IMPL,0xda,3, 0,0,0
		};
  
	struct SUB_CODE sMn40[] = {
		ADDR_IMPL,0x5a,3, 0,0,0
		};

struct SUB_CODE sMn41[] = {
  ADDR_IMPL,0x68,3, 0,0,0
  };
  
struct SUB_CODE sMn42[] = {
  ADDR_IMPL,0x28,3, 0,0,0
  };
  
	struct SUB_CODE sMn43[] = {
		ADDR_IMPL,0xfa,6, 0,0,0
		};
  
	struct SUB_CODE sMn44[] = {
		ADDR_IMPL,0x7a,6, 0,0,0
		};
  
struct SUB_CODE sMn45[] = {
  ADDR_REG8,0x2a,2, ADDR_ZP,0x26,5, ADDR_ZPX,0x36,6, ADDR_ABS,0x2e,6, ADDR_ABSX,0x3e,7, 0,0,0
  };
  
struct SUB_CODE sMn46[] = {
  ADDR_REG8,0x6a,2, ADDR_ZP,0x66,5, ADDR_ZPX,0x76,6, ADDR_ABS,0x6e,6, ADDR_ABSX,0x7e,7, 0,0,0
  };
  
struct SUB_CODE sMn47[] = {
  ADDR_IMPL,0x40,6, 0,0,0
  };
  
struct SUB_CODE sMn48[] = {
  ADDR_IMPL,0x60,6, 0,0,0
  };
  
struct SUB_CODE sMn49[] = {
  ADDR_IMM8,0xe9,2, ADDR_ZP,0xe5,3, ADDR_ZPX,0xf5,4, ADDR_ABS,0xed,4, ADDR_ABSX,0xfd,4, ADDR_ABSY,0xf9,4, 10,0xe1,6, 11,0xf1,5, 0,0,0
  };
  
struct SUB_CODE sMn50[] = {
  ADDR_IMPL,0x38,2, 0,0,0
  };
  
struct SUB_CODE sMn51[] = {
  ADDR_IMPL,0xf8,2, 0,0,0
  };
  
struct SUB_CODE sMn52[] = {
  ADDR_IMPL,0x78,2, 0,0,0
  };
  
struct SUB_CODE sMn53[] = {
  ADDR_ZP,0x85,3, ADDR_ZPX,0x95,4, ADDR_ABS,0x8d,4, ADDR_ABSX,0x9d,5, ADDR_ABSY,0x99,5, 10,0x81,6, 11,0x91,6, 0,0,0
  };
  
struct SUB_CODE sMn54[] = {
  ADDR_ZP,0x86,3, ADDR_ZPY,0x96,4, ADDR_ABS,0x8e,4, 0,0,0
  };
  
struct SUB_CODE sMn55[] = {
  ADDR_ZP,0x84,3, ADDR_ZPX,0x94,4, ADDR_ABS,0x8c,4, 0,0,0
  };
  
	struct SUB_CODE sMn56[] = {
		ADDR_ZP,0x64,3, ADDR_ZPX,0x74,4, ADDR_ABS,0x9c,4, ADDR_ABSX,0x9e,5, 0,0,0
		};
  
struct SUB_CODE sMn57[] = {
  ADDR_IMPL,0xaa,3, 0,0,0
  };
  
struct SUB_CODE sMn58[] = {
  ADDR_IMPL,0xa8,3, 0,0,0
  };
  
	struct SUB_CODE sMn59[] = {
		ADDR_ZP,0x14,5, ADDR_ABS,0x1c,6, 0,0,0
		};
  
	struct SUB_CODE sMn60[] = {
		ADDR_ZP,0x4,5, ADDR_ABS,0xc,6, 0,0,0
		};
  
struct SUB_CODE sMn61[] = {
  ADDR_IMPL,0xba,3, 0,0,0
  };
  
struct SUB_CODE sMn62[] = {
  ADDR_IMPL,0x8a,3, 0,0,0
  };
  
struct SUB_CODE sMn63[] = {
  ADDR_IMPL,0x9a,3, 0,0,0
  };
  
struct SUB_CODE sMn64[] = {
  ADDR_IMPL,0x98,3, 0,0,0
  };

 
  
struct CODE Mnem[] = {
// il (secondo) valore (flag M) vale 1 per implicito
// 2 per A
// 4 per imm8
// 5 per zp8, 6 per zp8,x, 16 per zp8,y
// 7 per abs16, 8 per abs16,x , 9 per abs16,y
// 10 per (zp8,x) , 11 per (zp8),y 
// 12 per indirect
  "ADC",sMn1,
  "AND",sMn2,
  "ASL",sMn3,
  "BCC",sMn4,
  "BCS",sMn5,
  "BEQ",sMn6,
  "BIT",sMn7,
  "BMI",sMn8,
  "BNE",sMn9,
  "BPL",sMn10,
		"BRA",sMn11,
  "BRK",sMn12,
  "BVC",sMn13,
  "BVS",sMn14,
  "CLC",sMn15,
  "CLD",sMn16,
  "CLI",sMn17,
  "CLV",sMn18,
  "CMP",sMn19,
  "CPX",sMn20,
  "CPY",sMn21,
  "DEC",sMn22,
  "DEX",sMn23,
  "DEY",sMn24,
  "EOR",sMn25,
  "INC",sMn26,
  "INX",sMn27,
  "INY",sMn28,
  "JMP",sMn29,
  "JSR",sMn30,
  "LDA",sMn31,
  "LDX",sMn32,
  "LDY",sMn33,
  "LSR",sMn34,
  "NOP",sMn35,
  "ORA",sMn36,
  "PHA",sMn37,
  "PHP",sMn38,
	  "PHX",sMn39,
	  "PHY",sMn40,
  "PLA",sMn41,
  "PLP",sMn42,
	  "PLX",sMn43,
	  "PLY",sMn44,
  "ROL",sMn45,
  "ROR",sMn46,
  "RTI",sMn47,
  "RTS",sMn48,
  "SBC",sMn49,
  "SEC",sMn50,
  "SED",sMn51,
  "SEI",sMn52,
  "STA",sMn53,
  "STX",sMn54,
  "STY",sMn55,
	  "STZ",sMn56,
  "TAX",sMn57,
  "TAY",sMn58,
	  "TRB",sMn59,
	  "TSB",sMn60,
  "TSX",sMn61,
  "TXA",sMn62,
  "TXS",sMn63,
  "TYA",sMn64
  };

#elif I8051

struct SUB_CODE sMn1[] = {
   14,0x11, 0,0
  };
  
struct SUB_CODE sMn2[] = {
   4,0x24, 21,0x25,9,0x26,2,0x28,0,0
  };
  
struct SUB_CODE sMn3[] = {
   4,0x34, 21,0x35,9,0x36,2,0x38,0,0
  };
  
struct SUB_CODE sMn4[] = {
   14,1, 0,0
  };
  
struct SUB_CODE sMn5[] = {
  ADDR_IMM8,0xcd, 14,0xcd,40,0xc4,0,0
  };
  
struct SUB_CODE sMn6[] = {
  ADDR_IMPL,0x3f, 0,0
  };
  
struct SUB_CODE sMn7[] = {
  10,0xb8, 4,0xfe,5,0xddbe,9,0xbe,0,0
  };
  
struct SUB_CODE sMn8[] = {
  ADDR_IMPL,0xeda9, 0,0
  };
  
struct SUB_CODE sMn9[] = {
  ADDR_IMPL,0xedb9, 0,0
  };
  
struct SUB_CODE sMn10[] = {
   ADDR_IMM8,0x14, 21,0x15,9,0x16,10,0x18,0,0
  };
  
struct SUB_CODE sMn11[] = {
  ADDR_IMPL,0xedb1, 0,0
  };
  
struct SUB_CODE sMn12[] = {
  ADDR_IMPL,0x2f, 0,0
  };
  
struct SUB_CODE sMn13[] = {
   ADDR_IMM8,0x4, 21,0x5,9,0x6,10,0x8,0,0
  };
  
struct SUB_CODE sMn14[] = {
   40,0x20, 0,0
  };
  
struct SUB_CODE sMn15[] = {
   40,0x10, 0,0
  };
  
struct SUB_CODE sMn16[] = {
  40,0x40, 0,0
  };
  
struct SUB_CODE sMn17[] = {
  ADDR_IMPL,0xfb, 0,0
  };
  
struct SUB_CODE sMn18[] = {
  40,0x30, 0,0
  };
  
struct SUB_CODE sMn19[] = {
  40,0x50, 0,0
  };
  
struct SUB_CODE sMn20[] = {
   40,0x70, 0,0
  };
  
struct SUB_CODE sMn21[] = {
   40,0x60, 0,0
  };
  
struct SUB_CODE sMn22[] = {
   14,1, 0,0
  };
  
struct SUB_CODE sMn23[] = {
   14,2, 0,0
  };
  
struct SUB_CODE sMn24[] = {
   11,0x78, 12,0xa8,4,0x74,21,0xe5,2,0xe8,0,0
  };
  
struct SUB_CODE sMn25[] = {
  ADDR_IMPL,0xedba, 0,0
  };
  
struct SUB_CODE sMn26[] = {
  ADDR_IMPL,0xeda2, 0,0
  };
  
struct SUB_CODE sMn27[] = {
  ADDR_IMPL,0xedb2, 0,0
  };
  
struct SUB_CODE sMn28[] = {
   ADDR_IMPL,0, 0,0
  };
  
struct SUB_CODE sMn29[] = {
  ADDR_IMM8,0x18, 14,0x18,40,0x20,0,0
  };
  
struct SUB_CODE sMn30[] = {
   21,0xd0, 0,0
  };
  
struct SUB_CODE sMn31[] = {
   31,0xc0, 0,0
  };
  
struct SUB_CODE sMn32[] = {
   ADDR_IMPL,0x22, 0,0
  };
  
struct SUB_CODE sMn33[] = {
   ADDR_IMPL,0x32, 0,0
  };
  
struct SUB_CODE sMn34[] = {
  ADDR_IMPL,0xedb0, 0,0
  };
  
struct SUB_CODE sMn35[] = {
  ADDR_IMPL,0xed44, 0,0
  };
  
struct SUB_CODE sMn36[] = {
  ADDR_IMPL,0x0, 0,0
  };
  
struct SUB_CODE sMn37[] = {
  10,0xb0, 4,0xf6,5,0xddb6,9,0xb6,0,0
  };
  
struct SUB_CODE sMn38[] = {
  ADDR_IMPL,0xedbb, 0,0
  };
  
struct SUB_CODE sMn39[] = {
  ADDR_IMPL,0xedb3, 0,0
  };
  
struct SUB_CODE sMn40[] = {
  28,0xed41, 17,0xd3,0,0
  };
  
struct SUB_CODE sMn41[] = {
  ADDR_IMPL,0xedab, 0,0
  };
  
struct SUB_CODE sMn42[] = {
  ADDR_IMPL,0xeda3, 0,0
  };
  
struct SUB_CODE sMn43[] = {
  3,0xc1, 15,0xdde1,16,0xfde1,0,0
  };
  
struct SUB_CODE sMn44[] = {
  3,0xc5, 15,0xdde5,16,0xfde5,0,0
  };
  
  
struct CODE Mnem[] = {
// il secondo valore (flag M) vale 1 per implicito
// 2 per A,r8 bit
// 3 per HL,r16 bit
// 4 per A,imm8
// 5 per A,(IX+), 6 per A,(IY+); 33 r8,(IX+);  34 r8,(IY+)
// 7 per IX,r16, 8 per IY,r16
// 9 per A,ind HL ;  32: r8,(HL)
// 10 per r8,r8 (o solo r8), 11 per r8,#imm, 12 per r8,(nn)
// 14 per imm16, 15 per HL,ix, 16 per HL,iy; 31 per HL,sp
// 17: (nn),A;  18: (nn),r16;  19: (r16),nn;  20: (r16),r8
// 21: A,(nn); 22: r16,(nn);  23: r16,imm;  24: (IX+),r8;  25: (IY+),r8; 26: (IX+),imm;  27: (IY+),imm
// 28: (r8),r8;  29: r8,(r8); 30: (r16),r16;  
// 41 per condiz, 40 condiz,imm
// 50: set 0,r8; 51 set 0,(hl); 52 set 0,(ix);  53: set 0,(iy)
// 60 per A,I; 61 per A,R; 62 per I,A; 63 per R,A
  "ACALL",sMn1,
  "ADD",sMn2,
  "ADDC",sMn3,
  "AJMP",sMn4,
  "ANL",sMn5,
  "CJNE",sMn6,
  "CPL",sMn7,
  "CLR",sMn8,
  "DA",sMn9,
  "DEC",sMn10,
  "DIV",sMn11,
  "DJNZ",sMn12,
  "INC",sMn13,
  "JB",sMn14,
  "JBC",sMn15, 
  "JC",sMn16,
  "JMP",sMn17,
  "JNB",sMn18,
  "JNC",sMn19,
  "JNZ",sMn20,
  "JZ",sMn21,
  "LCALL",sMn22,
  "LJMP",sMn23,
  "MOV",sMn24,
  "MOVC",sMn25,
  "MOVX",sMn26,
  "MUL",sMn27,
  "NOP",sMn28,
  "ORL",sMn29,
  "POP",sMn30,
  "PUSH",sMn31,
  "RET",sMn32,
  "RETI",sMn33,
  "RL",sMn34,
  "RLC",sMn35,
  "RR",sMn36,
  "RRC",sMn37,
  "SETB",sMn38,
  "SJMP",sMn39,
  "SUBB",sMn40,
  "SWAP",sMn41,
  "XCH",sMn42,
  "XCHD",sMn43,
  "XRL",sMn44
  };


#elif ST62

struct SUB_CODE sMn1[] = {
  ADDR_REG8,0x5f,4, 3,0x5f,4, 4,0x5f,4, 5,0x47,4, 6,0x4f,4, 0,0,0
  };
  
struct SUB_CODE sMn2[] = {
  ADDR_IMM8,0x57,4, 0,0,0
  };
  
struct SUB_CODE sMn3[] = {
  ADDR_REG8,0xbf,4, 3,0xbf,4, 5,0xa7,4, 6,0xaf,4, 0,0,0
  };
  
struct SUB_CODE sMn4[] = {
  ADDR_IMM8,0xb7,4, 0,0,0
  };
  
struct SUB_CODE sMn5[] = {
  ADDR_IMM8,0x1,4, 14,0x1,4, 0,0,0
  };
  
struct SUB_CODE sMn6[] = {
  ADDR_REG8,0xd,4, 3,0xd,4, ADDR_IMPL,0xdf,4, 4,0xd,4, 0,0,0
  };
  
struct SUB_CODE sMn7[] = {
  ADDR_IMPL,0x2d,4, 0,0,0
  };
  
struct SUB_CODE sMn8[] = {
  ADDR_REG8,0x3f,4, 3,0x3f,4, 4,0x3f,4, 5,0x27,4, 6,0x2f,4, 0,0,0
  };
  
struct SUB_CODE sMn9[] = {
  3,0x37,4, 4,0x37,4, /*v. ld */ 0,0,0
  };
  
struct SUB_CODE sMn10[] = {
  10,0x1d,4, 3,0xff,4, 4,0xff,4, 5,0xe7,4, 6,0xef,4, 0,0,0
  };
  
struct SUB_CODE sMn11[] = {
  10,0x15,4, 3,0x7f,4, 4,0x7f,4, 5,0x67,4, 6,0x6f,4, 0,0,0
  };
  
struct SUB_CODE sMn12[] = {
  ADDR_IMM8,0x9,4, 14,0x9,4, 0,0,0
  };
  
struct SUB_CODE sMn13[] = {
  ADDR_IMM8,0x6,2, 14,0x6,2, ADDR_CONDIZ,0x6,2, 0,0,0
  };
  
struct SUB_CODE sMn14[] = {
  ADDR_IMM8,0x2,2, 14,0x2,2, ADDR_CONDIZ,0x2,2, 0,0,0
  };
  
struct SUB_CODE sMn15[] = {
  ADDR_IMM8,0x0,2, 14,0x0,2, ADDR_CONDIZ,0x0,2, 0,0,0
  };
  
struct SUB_CODE sMn16[] = {
  40,0x3,5, 0,0,0
  };
  
struct SUB_CODE sMn17[] = {
  40,0x13,5, 0,0,0
  };
  
struct SUB_CODE sMn18[] = {
  ADDR_IMM8,0x4,2, 14,0x4,2, ADDR_CONDIZ,0x4,2, 0,0,0
  };
  
struct SUB_CODE sMn19[] = {
  ADDR_REG8,0x35,4, 3,0x1f,4, 
  ADDR_IMM8,0x1f,4, /* metto il modo 10 come il 3, perche' l'operando e' uguale (cambia opcode) sia in indiretto che in immediato)*/
  10,0x3d,4, 11,0x3d,4, 
  5,0x7,4, 6,0xf,4, 28,0x9f,4, 17,0x9f,4, 24,0x87,4, 25,0x8f,4, 0,0,0
  };
  
struct SUB_CODE sMn20[] = {
  ADDR_IMM8,0x17,4, 3,0xd,4 /* v. ld */, 11,0xd,4, 0,0,0
  };
  
struct SUB_CODE sMn21[] = {
  ADDR_IMPL,0x4,2, 0,0,0
  };
  
struct SUB_CODE sMn22[] = {
  50,0xb,4, 0,0,0
  };
  
struct SUB_CODE sMn23[] = {
  ADDR_IMPL,0xcd,2, 0,0,0
  };
  
struct SUB_CODE sMn24[] = {
  ADDR_IMPL,0x4d,2, 0,0,0
  };
  
struct SUB_CODE sMn25[] = {
  ADDR_IMPL,0xad,4, 0,0,0
  };
  
struct SUB_CODE sMn26[] = {
  50,0x1b,4, 0,0,0
  };
  
struct SUB_CODE sMn27[] = {
  ADDR_IMPL,0x5f,4, 0,0,0
  };
  
struct SUB_CODE sMn28[] = {
  ADDR_IMPL,0x6d,2, 0,0,0
  };
  
struct SUB_CODE sMn29[] = {
  ADDR_REG8,0xdf,4, 3,0xdf,4, 5,0xc7,4, 6,0xcf,4, 0,0,0
  };
  
struct SUB_CODE sMn30[] = {
  ADDR_IMM8,0xd7,4, 0,0,0
  };
  
struct SUB_CODE sMn31[] = {
  ADDR_IMPL,0xed,2, 0,0,0
  };
  
  
struct CODE Mnem[] = {
// il secondo valore (flag M) vale 1 per implicito (inherent)
// 2 per A,r8 bit
// 3 per A,(addr 8 bit) // equiv. registro r8
// 4 per A,imm8
// 5 per A,(X), 6 per A,(Y);
// 10 per r8,r8 (o solo r8), 11 per r8,#imm, 12 per r8,(nn)
// 14 per imm12   // PC
// 17: (nn),A     // equiv. registro r8
// 21: A,(nn); 22: r16,(nn);  23: r16,imm;  24: (X),r8;  25: (Y),r8;
// 28: (r8),A; 
// 41 per condiz, 41 condiz,imm (specif. reg)
// 50: set 0,r8;
  "ADD",sMn1,
  "ADDI",sMn2,
  "AND",sMn3,
  "ANDI",sMn4,
  "CALL",sMn5,
  "CLR",sMn6,
  "COM",sMn7,
  "CP",sMn8,
  "CPI",sMn9,
  "DEC",sMn10,
  "INC",sMn11,
  "JP",sMn12,
  "JRC",sMn13,
  "JRNC",sMn14,
  "JRNZ",sMn15,
  "JRR",sMn16,
  "JRS",sMn17,
  "JRZ",sMn18,
  "LD",sMn19,
  "LDI",sMn20,
  "NOP",sMn21,
  "RES",sMn22,
  "RET",sMn23,
  "RETI",sMn24,
  "RLC",sMn25,
  "SET",sMn26,
  "SLA",sMn27,
  "STOP",sMn28,
  "SUB",sMn29,
  "SUBI",sMn30,
  "WAIT",sMn31
  };

#elif PIC==16

struct SUB_CODE sMn1[] = {
  ADDR_IMM8,0x3e00,1, 0,0,0
  };

struct SUB_CODE sMn2[] = {
  ADDR_8,0x0700,1, 0,0,0
  };

struct SUB_CODE sMn3[] = {
  ADDR_IMM8,0x3900,1, 0,0,0
  };

struct SUB_CODE sMn4[] = {
  ADDR_8,0x0500,1, 0,0,0
  };

struct SUB_CODE sMn5[] = {
  ADDR_8,0x1000,1, 0,0,0
  };

struct SUB_CODE sMn6[] = {
  ADDR_8,0x1400,1, 0,0,0
  };

struct SUB_CODE sMn7[] = {
  ADDR_8,0x1800,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn8[] = {
  ADDR_8,0x1c00,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn9[] = {
  ADDR_ABS,0x1000,2, 0,0,0
  };

struct SUB_CODE sMn10[] = {
  ADDR_8,0x0180,1, 0,0,0
  };

struct SUB_CODE sMn11[] = {
  ADDR_IMPL,0x0103,1, 0,0,0
  };

struct SUB_CODE sMn12[] = {
  ADDR_IMPL,0x0064,1, 0,0,0
  };

struct SUB_CODE sMn13[] = {
  ADDR_8,0x0900,1, 0,0,0
  };

struct SUB_CODE sMn14[] = {
  ADDR_8,0x0300,1, 0,0,0
  };

struct SUB_CODE sMn15[] = {
  ADDR_8,0x0b00,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn16[] = {
  ADDR_ABS,0x2800,2, 0,0,0
  };

struct SUB_CODE sMn17[] = {
  ADDR_8,0x0600,1, 0,0,0
  };

struct SUB_CODE sMn18[] = {
  ADDR_8,0x0f00,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn19[] = {
  ADDR_IMM8,0x3800,1, 0,0,0
  };

struct SUB_CODE sMn20[] = {
  ADDR_8,0x0400,1, 0,0,0
  };

struct SUB_CODE sMn21[] = {
  ADDR_8,0x0800,1, 0,0,0
  };

struct SUB_CODE sMn22[] = {
  ADDR_IMM8,0x3000,1, 0,0,0
  };

struct SUB_CODE sMn23[] = {
  ADDR_8,0x0080,1, 0,0,0
  };

struct SUB_CODE sMn24[] = {
  ADDR_IMPL,0x0000,1, 0,0,0
  };

struct SUB_CODE sMn25[] = {
  ADDR_IMPL,0x0062,1, 0,0,0
  };

struct SUB_CODE sMn26[] = {
  ADDR_IMPL,0x0009,2, 0,0,0
  };

struct SUB_CODE sMn27[] = {
  ADDR_IMM8,0x3400,2, 0,0,0
  };

struct SUB_CODE sMn28[] = {
  ADDR_IMPL,0x0008,2, 0,0,0
  };

struct SUB_CODE sMn29[] = {
  ADDR_8,0x0d00,1, 0,0,0
  };

struct SUB_CODE sMn30[] = {
  ADDR_8,0x0c00,1, 0,0,0
  };

struct SUB_CODE sMn31[] = {
  ADDR_IMPL,0x0063,1, 0,0,0
  };

struct SUB_CODE sMn32[] = {
  ADDR_IMM8,0x3c00,1, 0,0,0
  };

struct SUB_CODE sMn33[] = {
  ADDR_8,0x0200,1, 0,0,0
  };

struct SUB_CODE sMn34[] = {
  ADDR_8,0x0e00,1, 0,0,0
  };

struct SUB_CODE sMn35[] = {
  ADDR_IMPL,0x0060,1, 0,0,0
  };

struct SUB_CODE sMn36[] = {
  ADDR_IMM8,0x3a00,1, 0,0,0
  };

struct SUB_CODE sMn37[] = {
  ADDR_8,0x0600,1, 0,0,0
  };

struct CODE Mnem[] = {
  "ADDLW",sMn1,
  "ADDWF",sMn2,
  "ANDLW",sMn3,
  "ANDWF",sMn4,
  "BCF",sMn5,
  "BSF",sMn6,
  "BTFSC",sMn7,
  "BTFSS",sMn8,
  "CALL",sMn9,
  "CLRF",sMn10,
  "CLRW",sMn11,
  "CLRWDT",sMn12,
  "COMF",sMn13,
  "DECF",sMn14,
  "DECFSZ",sMn15,
  "GOTO",sMn16,
  "INCF",sMn17,
  "INCFSZ",sMn18,
  "IORLW",sMn19,
  "IORWF",sMn20,
  "MOVF",sMn21,
  "MOVLW",sMn22,
  "MOVWF",sMn23,
  "NOP",sMn24,
  "OPTION",sMn25,
  "RETFIE",sMn26,
  "RETLW",sMn27,
  "RETURN",sMn28,
  "RLF",sMn29,
  "RRF",sMn30,
  "SLEEP",sMn31,
  "SUBLW",sMn32,
  "SUBWF",sMn33,
  "SWAPF",sMn34,
  "TRIS",sMn35,
  "XORLW",sMn36,
  "XORWF",sMn37
  };

#elif PIC==18

struct SUB_CODE sMn1[] = {
	ADDR_IMM8,0xe800,1, 0,0,0
	};

struct SUB_CODE sMn2[] = {
  ADDR_IMM8,0x0f00,1, 0,0,0
  };

struct SUB_CODE sMn3[] = {
  ADDR_8,0x2400,1, 0,0,0
  };

struct SUB_CODE sMn4[] = {
  ADDR_8,0x2000,1, 0,0,0
  };

	struct SUB_CODE sMn5[] = {
		ADDR_8,0xe8c0,1, 0,0,0
		};

struct SUB_CODE sMn6[] = {
  ADDR_IMM8,0x0b00,1, 0,0,0
  };

struct SUB_CODE sMn7[] = {
  ADDR_8,0x1400,1, 0,0,0
  };

struct SUB_CODE sMn8[] = {
  ADDR_ABS,0xe200,1, 0,0,0
  };

struct SUB_CODE sMn9[] = {
  ADDR_8,0x9000,1, 0,0,0
  };

struct SUB_CODE sMn10[] = {
  ADDR_ABS,0xe600,1, 0,0,0
  };

struct SUB_CODE sMn11[] = {
  ADDR_ABS,0xe300,1, 0,0,0
  };

struct SUB_CODE sMn12[] = {
  ADDR_ABS,0xe780,1, 0,0,0
  };

struct SUB_CODE sMn13[] = {
  ADDR_ABS,0xe500,1, 0,0,0
  };

struct SUB_CODE sMn14[] = {
  ADDR_ABS,0xe100,1, 0,0,0
  };

struct SUB_CODE sMn15[] = {
  ADDR_ABS,0xe400,1, 0,0,0
  };

struct SUB_CODE sMn16[] = {
  ADDR_ABS,0xd000,1, 0,0,0
  };

struct SUB_CODE sMn17[] = {
  ADDR_8,0x8000,1, 0,0,0
  };

struct SUB_CODE sMn18[] = {
  ADDR_8,0xb000,1, 0,0,0
  };

struct SUB_CODE sMn19[] = {
  ADDR_8,0xa000,1, 0,0,0
  };

struct SUB_CODE sMn20[] = {
  ADDR_8,0x7000,1, 0,0,0
  };

struct SUB_CODE sMn21[] = {
  ADDR_ABS,0xe000,1, 0,0,0
  };

struct SUB_CODE sMn22[] = {
  ADDR_ABS,0xf000ec00,2, 0,0,0
  };

	struct SUB_CODE sMn23[] = {
		ADDR_ABS,0x0014,2, 0,0,0
		};

struct SUB_CODE sMn24[] = {
  ADDR_8,0x6a00,1, 0,0,0
  };

struct SUB_CODE sMn25[] = {		// CLRWDT
  ADDR_IMPL,0x0400,1, 0,0,0
  };

struct SUB_CODE sMn26[] = {
  ADDR_8,0x1c00,1, 0,0,0
  };

struct SUB_CODE sMn27[] = {
  ADDR_8,0x6200,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn28[] = {
  ADDR_8,0x6400,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn29[] = {
  ADDR_8,0x6000,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn30[] = {
  ADDR_IMPL,0x0700,1, 0,0,0
  };

struct SUB_CODE sMn31[] = {
  ADDR_8,0x0400,1, 0,0,0
  };

struct SUB_CODE sMn32[] = {
  ADDR_8,0x2c00,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn33[] = {
  ADDR_8,0x4c00,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn34[] = {
  ADDR_ABS,0xf000ef00,2, 0,0,0
  };

struct SUB_CODE sMn35[] = {
  ADDR_8,0x2800,1, 0,0,0
  };

struct SUB_CODE sMn36[] = {
  ADDR_8,0x3c00,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn37[] = {
  ADDR_8,0x4800,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn38[] = {
  ADDR_IMM8,0x0900,1, 0,0,0
  };

struct SUB_CODE sMn39[] = {
  ADDR_8,0x1000,1, 0,0,0
  };

struct SUB_CODE sMn40[] = {
  ADDR_IMM16,0xf000ee00,2, 0,0,0
  };

struct SUB_CODE sMn41[] = {
  ADDR_8,0x5000,2, 0,0,0
  };

struct SUB_CODE sMn42[] = {
  ADDR_ABS2,0xf000c000,2, 0,0,0
  };

struct SUB_CODE sMn43[] = {
  ADDR_IMM8,0x0100,1, 0,0,0
  };

struct SUB_CODE sMn44[] = {
  ADDR_IMM8,0x0e00,1, 0,0,0
  };

	struct SUB_CODE sMn45[] = {
		ADDR_IMM8,0xf0000b00,2, 0,0,0
		};

	struct SUB_CODE sMn46[] = {
		ADDR_IMM8,0xf000eb80,2, 0,0,0
		};

struct SUB_CODE sMn47[] = {
  ADDR_8,0x6e00,1, 0,0,0
  };

struct SUB_CODE sMn48[] = {
  ADDR_IMM8,0x0d00,1, 0,0,0
  };

struct SUB_CODE sMn49[] = {
  ADDR_8,0x0200,1, 0,0,0
  };

struct SUB_CODE sMn50[] = {
  ADDR_8,0x6c00,2, 0,0,0
  };

struct SUB_CODE sMn51[] = {
  ADDR_IMPL,0xf0000000,2, 0,0,0
  };

struct SUB_CODE sMn52[] = {
  ADDR_IMPL,0x0600,1, 0,0,0
  };

struct SUB_CODE sMn53[] = {
  ADDR_IMPL,0x0500,1, 0,0,0
  };

	struct SUB_CODE sMn54[] = {
		ADDR_IMPL,0xfa00,1, 0,0,0
		};

struct SUB_CODE sMn55[] = {
  ADDR_ABS,0xd800,2, 0,0,0
  };

struct SUB_CODE sMn56[] = {
  ADDR_IMPL,0x00ff,1, 0,0,0
  };

struct SUB_CODE sMn57[] = {
  ADDR_IMPL,0x0010,2, 0,0,0
  };

struct SUB_CODE sMn58[] = {
  ADDR_IMM8,0x0c00,2, 0,0,0
  };

struct SUB_CODE sMn59[] = {
  ADDR_IMPL,0x0012,2, 0,0,0
  };

struct SUB_CODE sMn60[] = {
  ADDR_8,0x3400,1, 0,0,0
  };

struct SUB_CODE sMn61[] = {
  ADDR_8,0x4400,1, 0,0,0
  };

struct SUB_CODE sMn62[] = {
  ADDR_8,0x3000,1, 0,0,0
  };

struct SUB_CODE sMn63[] = {
  ADDR_8,0x4000,1, 0,0,0
  };

struct SUB_CODE sMn64[] = {
  ADDR_8,0x6800,1, 0,0,0
  };

struct SUB_CODE sMn65[] = {
  ADDR_IMPL,0x0003,1, 0,0,0
  };

	struct SUB_CODE sMn66[] = {
		ADDR_8,0xe900,2, 0,0,0
		};

struct SUB_CODE sMn67[] = {
  ADDR_8,0x5400,1, 0,0,0
  };

	struct SUB_CODE sMn68[] = {
		ADDR_8,0xe9c0,1, 0,0,0
		};

struct SUB_CODE sMn69[] = {
  ADDR_8,0x0800,1, 0,0,0
  };

struct SUB_CODE sMn70[] = {
  ADDR_8,0x5c00,1, 0,0,0
  };

struct SUB_CODE sMn71[] = {
  ADDR_8,0x5800,1, 0,0,0
  };

struct SUB_CODE sMn72[] = {
  ADDR_8,0x3800,1, 0,0,0
  };

struct SUB_CODE sMn73[] = {
  ADDR_IMPL,0x0008,2, 0,0,0
  };

struct SUB_CODE sMn74[] = {
  ADDR_IMPL,0x000c,2, 0,0,0
  };

struct SUB_CODE sMn75[] = {
  ADDR_8,0x6600,MAKEWORD(1,2), 0,0,0
  };

struct SUB_CODE sMn76[] = {
  ADDR_IMM8,0x0a00,1, 0,0,0
  };

struct SUB_CODE sMn77[] = {
  ADDR_8,0x1800,1, 0,0,0
  };

struct CODE Mnem[] = {
  "ADDFSR",sMn1,
  "ADDLW",sMn2,
  "ADDWF",sMn3,
  "ADDWFC",sMn4,
  "ADDUNLK",sMn5,
  "ANDLW",sMn6,
  "ANDWF",sMn7,
  "BC",sMn8,
  "BCF",sMn9,
  "BN",sMn10,
  "BNC",sMn11,
  "BNN",sMn12,
  "BNOV",sMn13,
  "BNZ",sMn14,
  "BOV",sMn15,
  "BRA",sMn16,
  "BSF",sMn17,
  "BTFSC",sMn18,
  "BTFSS",sMn19,
  "BTG",sMn20,
  "BZ",sMn21,
  "CALL",sMn22,
  "CALLW",sMn23,
  "CLRF",sMn24,
  "CLRWDT",sMn25,
  "COMF",sMn26,
  "CPFSEQ",sMn27,
  "CPFSGT",sMn28,
  "CPFSLT",sMn29,
  "DAW",sMn30,
  "DECF",sMn31,
  "DECFSZ",sMn32,
  "DCFSNZ",sMn33,
  "GOTO",sMn34,
  "INCF",sMn35,
  "INCFSZ",sMn36,
  "INFSNZ",sMn37,
  "IORLW",sMn38,
  "IORWF",sMn39,
  "LFSR",sMn40,
  "MOVF",sMn41,
  "MOVFF",sMn42,
  "MOVLB",sMn43,
  "MOVLW",sMn44,
  "MOVSF",sMn45,
  "MOVSS",sMn46,
  "MOVWF",sMn47,
  "MULLW",sMn48,
  "MULWF",sMn49,
  "NEGF",sMn50,
  "NOP",sMn51,
  "POP",sMn52,
  "PUSH",sMn53,
  "PUSHL",sMn54,
  "RCALL",sMn55,
  "RESET",sMn56,
  "RETFIE",sMn57,
  "RETLW",sMn58,
  "RETURN",sMn59,
  "RLCF",sMn60,
  "RLNCF",sMn61,
  "RRCF",sMn62,
  "RRNCF",sMn63,
  "SETF",sMn64,
  "SLEEP",sMn65,
  "SUBFSR",sMn66,
  "SUBFWB",sMn67,
  "SUBLW",sMn68,
  "SUBULNK",sMn69,
  "SUBWF",sMn70,
  "SUBWFB",sMn71,
  "SWAPF",sMn72,
  "TBLRD",sMn73,
  "TBLWT",sMn74,
  "TSTFSZ",sMn75,
  "XORLW",sMn76,
  "XORWF",sMn77
  };

#elif GD24032

struct SUB_CODE sMn1[] = {
	ADDR_ALL,0x5e000000,1, 0,0,0
	};

struct SUB_CODE sMn2[] = {
  ADDR_ALL_2OP,0x22000000,1, 0,0,0
  };

struct SUB_CODE sMn3[] = {
  ADDR_ALL_2OP,0x20000000,1, 0,0,0
  };

struct SUB_CODE sMn4[] = {
  ADDR_ALL_2OP,0x50000000,1, 0,0,0
  };

struct SUB_CODE sMn5[] = {
	ADDR_CONDIZ,0xa0200200,1, 0,0,0
	};

struct SUB_CODE sMn6[] = {
  ADDR_CONDIZ,0xa0200000,1, 0,0,0
  };

struct SUB_CODE sMn7[] = {
  ADDR_CONDIZ,0xa0200a00,1, 0,0,0
  };

struct SUB_CODE sMn8[] = {
  ADDR_CONDIZ,0xa0200c00,1, 0,0,0
  };

struct SUB_CODE sMn9[] = {
  ADDR_CONDIZ,0xa0200800,1, 0,0,0
  };

struct SUB_CODE sMn10[] = {
  ADDR_ALL_3OP,0x66000000,1, ADDR_ALL_4OP,0x66000000,1, 0,0,0
  };

struct SUB_CODE sMn11[] = {
  ADDR_ALL,0x83000000,1, 0,0,0
  };

struct SUB_CODE sMn12[] = {
  ADDR_CONDIZ,0xa0200d00,1, 0,0,0
  };

struct SUB_CODE sMn13[] = {
  ADDR_CONDIZ,0xa0200900,1, 0,0,0
  };

struct SUB_CODE sMn14[] = {
  ADDR_CONDIZ,0xa0200b00,1, 0,0,0
  };

struct SUB_CODE sMn15[] = {
  ADDR_ALL,0x84000000,1, 0,0,0
  };

struct SUB_CODE sMn16[] = {
  ADDR_CONDIZ,0xa0200400,1, 0,0,0
  };

struct SUB_CODE sMn17[] = {
  ADDR_CONDIZ,0xa0200300,1, 0,0,0
  };

struct SUB_CODE sMn18[] = {
  ADDR_CONDIZ,0xa0200100,1, 0,0,0
  };

struct SUB_CODE sMn19[] = {
  ADDR_CONDIZ,0xa0200700,1, 0,0,0
  };

struct SUB_CODE sMn20[] = {
  ADDR_CONDIZ,0xa0200e00,1, 0,0,0
  };

struct SUB_CODE sMn21[] = {
  ADDR_CONDIZ,0xa0200e00,1, 0,0,0
  };

struct SUB_CODE sMn22[] = {
  ADDR_CONDIZ,0xa0200f00,1, 0,0,0
  };

struct SUB_CODE sMn23[] = {
	ADDR_ALL_3OP,0x6a000000,1, ADDR_ALL_4OP,0x6a000000,1, 0,0,0
	};

struct SUB_CODE sMn24[] = {
  ADDR_CONDIZ,0xa0200600,1, 0,0,0
  };

struct SUB_CODE sMn25[] = {
	ADDR_ALL_3OP,0x68000000,1, 0,0,0
	};

struct SUB_CODE sMn26[] = {
  ADDR_ALL,0x82000000,1, 0,0,0
  };

struct SUB_CODE sMn27[] = {		// 
  ADDR_ALL_2OP,0x8c000000,1, 0,0,0
  };

struct SUB_CODE sMn28[] = {
  ADDR_ALL,0x0a000000,1, 0,0,0
  };

struct SUB_CODE sMn29[] = {
  ADDR_ALL_2OP,0x28000000,1, 0,0,0
  };

struct SUB_CODE sMn30[] = {
  ADDR_ALL_3OP,0x2a000000,1, 0,0,0
  };

struct SUB_CODE sMn31[] = {
  ADDR_ALL,0x18000008,1, 0,0,0
  };

struct SUB_CODE sMn32[] = {
  ADDR_ALL,0x10000000,1, 0,0,0
  };

struct SUB_CODE sMn33[] = {
  ADDR_ALL,0x32000000,1, 0,0,0
  };

struct SUB_CODE sMn34[] = {
  ADDR_ALL_2OP,0x3c000000,1, 0,0,0
  };

struct SUB_CODE sMn35[] = {
  ADDR_ALL_2OP,0xa2000000,1, 0,0,0
  };

struct SUB_CODE sMn36[] = {
  ADDR_ALL_2OP,0x88000000,1, 0,0,0
  };

struct SUB_CODE sMn37[] = {
  ADDR_ALL_2OP,0x14000000,2, 0,0,0
  };

struct SUB_CODE sMn38[] = {
  ADDR_IMPL,0xfe000000,1, 0,0,0
  };

struct SUB_CODE sMn39[] = {
  ADDR_ALL_2OP,0x3e000000,1, 0,0,0
  };

struct SUB_CODE sMn40[] = {
  ADDR_ALL_2OP,0x3a000000,1, 0,0,0
  };

struct SUB_CODE sMn41[] = {
  ADDR_ALL_2OP,0x44000000,1, 0,0,0
  };

struct SUB_CODE sMn42[] = {
  ADDR_ALL,0x30000000,1, 0,0,0
  };

struct SUB_CODE sMn43[] = {
  ADDR_ALL_2OP,0x46000000,2, 0,0,0
  };

struct SUB_CODE sMn44[] = {
  ADDR_ALL,0x80000000,2, 0,0,0
  };

struct SUB_CODE sMn45[] = {
  ADDR_IMM17,0xa0000000,2, 0,0,0
  };

struct SUB_CODE sMn46[] = {
  ADDR_ALL,0xe0000000,1, 0,0,0
  };

struct SUB_CODE sMn47[] = {
  ADDR_ALL_2OP,0x98000000,1, 0,0,0
  };

struct SUB_CODE sMn48[] = {
	ADDR_ALL_2OP,0xe4020000,2, 0,0,0
	};

struct SUB_CODE sMn49[] = {
	ADDR_ALL,0xe2000000,2, 0,0,0
	};

struct SUB_CODE sMn50[] = {
	ADDR_ALL,0xe4000000,2, 0,0,0
	};

struct SUB_CODE sMn51[] = {
  ADDR_ALL_2OP,0x16000000,1, 0,0,0
  };

struct SUB_CODE sMn52[] = {
  ADDR_ALL,0x8a000000,1, 0,0,0
  };

struct SUB_CODE sMn53[] = {
  ADDR_ALL_2OP,0x78000000,1, ADDR_ALL_3OP,0x78200000,1, 0,0,0
  };

struct SUB_CODE sMn54[] = {
  ADDR_ALL_2OP,0x04000000,1, 0,0,0
  };

struct SUB_CODE sMn55[] = {
  ADDR_ALL_3OP,0x06000000,2, 0,0,0
  };

struct SUB_CODE sMn56[] = {
  ADDR_ALL_2OP,0x7a000000,1, ADDR_ALL_3OP,0x7a200000,1, 0,0,0
  };

struct SUB_CODE sMn57[] = {
  ADDR_ALL_2OP,0x38000000,2, 0,0,0
  };

struct SUB_CODE sMn58[] = {
  ADDR_ALL_2OP,0x56000000,1, 0,0,0
  };

struct SUB_CODE sMn59[] = {
  ADDR_ALL,0x5a000000,1, 0,0,0
  };

struct SUB_CODE sMn60[] = {
	ADDR_IMPL,0x00000000,1, 0,0,0
	};

struct SUB_CODE sMn61[] = {
  ADDR_ALL_2OP,0x58000000,2, 0,0,0
  };

struct SUB_CODE sMn62[] = {
  ADDR_ALL,0x5c000000,1, 0,0,0
  };

struct SUB_CODE sMn63[] = {
  ADDR_ALL_2OP,0x52000000,2, 0,0,0
  };

struct SUB_CODE sMn64[] = {
  ADDR_ALL_2OP,0x40000000,2, 0,0,0
  };

struct SUB_CODE sMn65[] = {
  ADDR_ALL_3OP,0x42000000,2, 0,0,0
  };

struct SUB_CODE sMn66[] = {
  ADDR_ALL_2OP,0x17000000,1, 0,0,0
  };

struct SUB_CODE sMn67[] = {
  ADDR_ALL,0x92000000,1, 0,0,0
  };

struct SUB_CODE sMn68[] = {
  ADDR_ALL,0x90000000,1, 0,0,0
  };

struct SUB_CODE sMn69[] = {
  ADDR_ALL,0x18000000,1, 0,0,0
  };

struct SUB_CODE sMn70[] = {
  ADDR_IMPL,0x86000000,1, ADDR_ALL_2OP,0x86000200,1, 0,0,0
  };

struct SUB_CODE sMn71[] = {
  ADDR_IMPL,0xec000000,1, 0,0,0
  };

struct SUB_CODE sMn72[] = {
  ADDR_IMPL,0x87000000,1, ADDR_ALL_2OP,0x87000200,1, 0,0,0
  };

struct SUB_CODE sMn73[] = {
  ADDR_ALL_2OP,0x70000005,1, 0,0,0
  };

struct SUB_CODE sMn74[] = {
	ADDR_ALL_2OP,0x70000006,2, 0,0,0
	};

struct SUB_CODE sMn75[] = {
  ADDR_ALL_2OP,0x70000003,1, 0,0,0
  };

struct SUB_CODE sMn76[] = {
	ADDR_ALL_2OP,0x70000004,1, 0,0,0
	};

struct SUB_CODE sMn77[] = {
  ADDR_IMPL,0xea000000,1, 0,0,0
  };

struct SUB_CODE sMn78[] = {
  ADDR_ALL_2OP,0x26000000,1, 0,0,0
  };

struct SUB_CODE sMn79[] = {
  ADDR_ALL_2OP,0x62000000,1, 0,0,0
  };

struct SUB_CODE sMn80[] = {
  ADDR_ALL_2OP,0x60000000,1, 0,0,0
  };

struct SUB_CODE sMn81[] = {
  ADDR_ALL,0x0e000000,2, 0,0,0
  };

struct SUB_CODE sMn82[] = {
  ADDR_ALL,0x0c000000,2, 0,0,0
  };

struct SUB_CODE sMn83[] = {
  ADDR_ALL,0xa4000000,1, 0,0,0
  };

struct SUB_CODE sMn84[] = {
  ADDR_ALL_2OP,0x70000000,1, 0,0,0
  };

struct SUB_CODE sMn85[] = {
  ADDR_ALL_2OP,0x70000001,1, 0,0,0
  };

struct SUB_CODE sMn86[] = {
  ADDR_ALL_2OP,0x70000002,1, 0,0,0
  };

struct SUB_CODE sMn87[] = {
  ADDR_ALL_2OP,0x7c000000,1, ADDR_ALL_3OP,0x7c200000,1, 0,0,0
  };

struct SUB_CODE sMn88[] = {
  ADDR_ALL_2OP,0xe8000000,1, 0,0,0
  };

struct SUB_CODE sMn89[] = {
  ADDR_ALL_2OP,0x9a000000,1, 0,0,0
  };

struct SUB_CODE sMn90[] = {
  ADDR_ALL,0xe8000000,1, 0,0,0
  };

struct SUB_CODE sMn91[] = {
  ADDR_ALL,0xe6000000,1, 0,0,0
  };

struct SUB_CODE sMn92[] = {
  ADDR_ALL,0xe8000008,1, 0,0,0
  };

struct SUB_CODE sMn93[] = {
  ADDR_ALL_2OP,0x24000000,1, 0,0,0
  };

struct SUB_CODE sMn94[] = {
  ADDR_ALL,0x12000000,1, 0,0,0
  };

struct SUB_CODE sMn95[] = {
  ADDR_ALL,0x12000000,1, 0,0,0
  };

struct SUB_CODE sMn96[] = {
  ADDR_ALL_2OP,0x64000000,1, 0,0,0
  };

struct SUB_CODE sMn97[] = {
  ADDR_ALL_2OP,0x50200000,1, 0,0,0
  };

struct SUB_CODE sMn98[] = {
  ADDR_IMM8,0xde000000,1, 0,0,0
  };

struct SUB_CODE sMn99[] = {
  ADDR_IMPL,0xde000080,1, 0,0,0
  };

struct SUB_CODE sMn100[] = {
  ADDR_ALL_2OP,0x7e000000,1, ADDR_ALL_3OP,0x7f000000,1, ADDR_ALL_4OP,0x7f000000,1, 0,0,0
  };

struct SUB_CODE sMn101[] = {
  ADDR_ALL,0x8e000000,1, 0,0,0
  };

struct SUB_CODE sMn102[] = {
  ADDR_ALL_2OP,0x08200000,1, 0,0,0
  };

struct SUB_CODE sMn103[] = {
  ADDR_ALL_2OP,0x08000000,1, 0,0,0
  };

struct SUB_CODE sMn104[] = {
  ADDR_ALL_2OP,0xfc000000,1, 0,0,0
  };

struct SUB_CODE sMn105[] = {
  ADDR_ALL_2OP,0x54000000,1, 0,0,0
  };

struct CODE Mnem[] = {
  "ABS",sMn1,
  "ADC",sMn2,
  "ADD",sMn3,
  "AND",sMn4,
  "BC",sMn5,
  "BEQ",sMn6,			// anche BZ
  "BGE",sMn7,
  "BGT",sMn8,
  "BHI",sMn9,
  "BINS",sMn10,
  "BL",sMn11,
  "BLE",sMn12,
  "BLS",sMn13,
  "BLT",sMn14,
  "BLWP",sMn15,
  "BMI",sMn16,
  "BNC",sMn17,
  "BNE",sMn18,		// anche BNZ
  "BNV",sMn19,
  "BPE",sMn20,
  "BPL",sMn21,
  "BPO",sMn22,
  "BSFR",sMn23,
  "BV",sMn24,
  "BXTR",sMn25,
  "CALL",sMn26,
  "CHK",sMn27,
  "CLR",sMn28,
  "CMP",sMn29,
  "CMPS",sMn30,
  "CPUID",sMn31,
  "DAA",sMn32,
  "DEC",sMn33,
  "DIV",sMn34,
  "DJNZ",sMn35,
  "ENTER",sMn36,
  "EX", sMn37,
  "HALT",sMn38,
  "IDIV",sMn39,
  "IMUL",sMn40,
  "IN", sMn41,
  "INC",sMn42,
  "INS",sMn43,
  "JMP",sMn44,
  "JR", sMn45,
  "LDIM",sMn46,
  "LDM",sMn47,
  "LDSP",sMn48,
  "LDST",sMn49,
  "LDWP",sMn50,
  "LEA", sMn51,
  "LEAVE",sMn52,
  "MAS", sMn53,
  "MOV", sMn54,
  "MOVS",sMn55,
  "MSS", sMn56,
  "MUL", sMn57,
  "NAND",sMn58,
  "NEG",sMn59,
  "NOP",sMn60,
  "NOR",sMn61,
  "NOT",sMn62,
  "OR", sMn63,
  "OUT",sMn64,
  "OUTS",sMn65,
  "PEA",sMn66,
  "POP",sMn67,
  "PUSH",sMn68,
  "RDTS",sMn69,
  "RET",sMn70,
  "RETI",sMn71,
  "RETU",sMn72,
  "RL", sMn73,
  "RLC",sMn74,
  "RR", sMn75,
  "RRC",sMn76,
  "RTWP",sMn77,
  "SBC",sMn78,
  "SBO",sMn79,
  "SBZ",sMn80,
  "SE", sMn81,
  "SET",sMn82,
  "SKIP",sMn83,
  "SLA",sMn84,
  "SRA",sMn85,
  "SRL",sMn86,
  "SSA",sMn87,
  "STEX",sMn88,
  "STM",sMn89,
  "STSP",sMn90,
  "STST",sMn91,
  "STWP",sMn92,
  "SUB",sMn93,
  "SWAP",sMn94,
  "SWAPR",sMn95,
  "TB", sMn96,
  "TEST", sMn97,
  "TRAP",sMn98,
  "TRAPV",sMn99,
  "VMA",  sMn100,
  "X",  sMn101,
  "XLAT",sMn102,
  "XLATB",sMn103,
  "XOP",sMn104,
  "XOR",sMn105
  };

#elif I8086
struct SUB_CODE sMn1[] = {
  ADDR_IMPL,0x0037,4, 0,0,0
  };
  
struct SUB_CODE sMn2[] = {
  ADDR_IMPL,0x00D5,4, 0,0,0			// gestire base d5 nn
  };
  
struct SUB_CODE sMn3[] = {
  ADDR_IMPL,0x003F,4, 0,0,0
  };
  
struct SUB_CODE sMn4[] = {
  ADDR_REG8,0x0014,2, ADDR_REG16,0x0015,4, ADDR_REG8IMM,0x80D0,2, ADDR_REG16IMM,0x81D0,4, ADDR_REG8REG,0x0010,2, ADDR_REG16REG,0x0011,4,
	ADDR_IND16_REG8,0x0010,0, ADDR_IND16_REG16,0x0011,0, 	ADDR_IND16REG_REG8,0x0010,0, ADDR_IND16REG_REG16,0x0011,0, 
	ADDR_REG8_IND16,0x0012,0, ADDR_REG16_IND16,0x0013,0, 	ADDR_REG8_IND16REG,0x0012,0, ADDR_REG16_IND16REG,0x0013,0, 
	ADDR_IND16REG_IMM8,0x8010,0, ADDR_IND16REG_IMM16,0x8110,0, 
	ADDR_IND16REG_IMM16_8,0x8310,0, 0,0,0
  };
  
struct SUB_CODE sMn5[] = {
  ADDR_REG8,0x0004,2, ADDR_REG16,0x0005,4, ADDR_REG8IMM,0x80C0,2, ADDR_REG16IMM,0x81C0,4, ADDR_REG8REG,0x0000,2, ADDR_REG16REG,0x0001,4,
	ADDR_IND16_REG8,0x0000,0, ADDR_IND16_REG16,0x0001,0,  ADDR_IND16REG_REG8,0x0000,0, ADDR_IND16REG_REG16,0x0001,0, 
	ADDR_REG8_IND16,0x0002,0, ADDR_REG16_IND16,0x0003,0,  ADDR_REG8_IND16REG,0x0002,0, ADDR_REG16_IND16REG,0x0003,0, 
	ADDR_IND16REG_IMM8,0x8000,0, ADDR_IND16REG_IMM16,0x8100,0,
	ADDR_IND16REG_IMM16_8,0x8300,0, 0,0,0
  };
  
struct SUB_CODE sMn6[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn7[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn8[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn9[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn10[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn11[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn12[] = {
  ADDR_REG8,0x0024,2, ADDR_REG16,0x0025,4, ADDR_REG8IMM,0x80E0,2, ADDR_REG16IMM,0x81E0,4, ADDR_REG8REG,0x0020,2, ADDR_REG16REG,0x0021,4,
	ADDR_IND16_REG8,0x0020,0, ADDR_IND16_REG16,0x0021,0,  ADDR_IND16REG_REG8,0x0020,0, ADDR_IND16REG_REG16,0x0021,0, 
	ADDR_REG8_IND16,0x0022,0, ADDR_REG16_IND16,0x0023,0,  ADDR_REG8_IND16REG,0x0022,0, ADDR_REG16_IND16REG,0x0023,0, 
	ADDR_IND16REG_IMM8,0x8020,0, ADDR_IND16REG_IMM16,0x8120,0, 
	ADDR_IND16REG_IMM16_8,0x8320,0, 0,0,0
  };
  
struct SUB_CODE sMn13[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn14[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn15[] = {
  ADDR_IMPL,0x00f8,4, 0,0,0
  };
  
struct SUB_CODE sMn16[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn17[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn18[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn19[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn20[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn21[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn22[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn23[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn24[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn25[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn26[] = {
  ADDR_IMM8/*serve cmq!*/,0x00E8,4, ADDR_IMM16,0x00E8,4, ADDR_IMM32,0x009A,6, ADDR_REG16,0xD0FF,8, 0,0,0
  };
  
struct SUB_CODE sMn27[] = {
  ADDR_IMPL,0x0098,4, 0,0,0
  };
  
struct SUB_CODE sMn28[] = {
  ADDR_IMPL,0x0099,4, 0,0,0
  };
  
struct SUB_CODE sMn29[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn30[] = {
  ADDR_IMPL,0x00FC,4, 0,0,0
  };
  
struct SUB_CODE sMn31[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn32[] = {
  ADDR_IMPL,0x00FA,4, 0,0,0
  };
  
struct SUB_CODE sMn33[] = {
  ADDR_IMPL,0xffff,4, 0,0,0
  };
  
struct SUB_CODE sMn34[] = {
  ADDR_IMPL,0x00F5,4, 0,0,0
  };
  
struct SUB_CODE sMn35[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn36[] = {
  ADDR_REG8,0x003C,2, ADDR_REG16,0x003D,4, ADDR_REG8IMM,0x80F8,2, ADDR_REG16IMM,0x81F8,4, ADDR_REG8REG,0x0038,2, ADDR_REG16REG,0x0039,4,
	ADDR_IND16_REG8,0x0038,0, ADDR_IND16_REG16,0x0039,0,  ADDR_IND16REG_REG8,0x0038,0, ADDR_IND16REG_REG16,0x0039,0, 
	ADDR_REG8_IND16,0x003A,0, ADDR_REG16_IND16,0x003B,0, 	ADDR_REG8_IND16REG,0x003A,0, ADDR_REG16_IND16REG,0x003B,0, 
	ADDR_IND16REG_IMM8,0x8038,0, ADDR_IND16REG_IMM16,0x8138,0, 
	ADDR_IND16REG_IMM16_8,0x8338,0, 0,0,0
  };
  
struct SUB_CODE sMn37[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn38[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn39[] = {
  ADDR_IMPL,0x00A6,4, 0,0,0			// gestire in base a size
  };
  
struct SUB_CODE sMn40[] = {
  ADDR_IMPL,0x00A6,4, 0,0,0
  };
  
struct SUB_CODE sMn41[] = {
  ADDR_IMPL,0x00A7,4, 0,0,0
  };
  
struct SUB_CODE sMn42[] = {
  ADDR_IMPL,0x00A7,4, 0,0,0
  };
  
struct SUB_CODE sMn43[] = {
  ADDR_IMPL,0x00A7,4, 0,0,0
  };
  
struct SUB_CODE sMn44[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn45[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn46[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn47[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn48[] = {
  ADDR_IMPL,0x0FA2,4, 0,0,0
  };
  
struct SUB_CODE sMn49[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn50[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn51[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn52[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn53[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn54[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn55[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn56[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn57[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn58[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn59[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn60[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn61[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn62[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn63[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn64[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn65[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn66[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn67[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn68[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn69[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn70[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn71[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn72[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn73[] = {
  ADDR_IMPL,0x0027,4, 0,0,0
  };

struct SUB_CODE sMn74[] = {
  ADDR_IMPL,0x002F,4, 0,0,0
  };

struct SUB_CODE sMn75[] = {
  ADDR_REG8IMM,0xFEC8,4, ADDR_REG16IMM,0x0048 /* anche ff cmq*/,4, ADDR_IND16REG_REG8,0xFE0E,2, ADDR_IND16REG_REG16,0xFF0E,0, 0,0,0
  };

struct SUB_CODE sMn76[] = {
  ADDR_REG8IMM,0xF6F0,2, ADDR_REG16IMM,0xF7F0,4,
	ADDR_IND16_REG8,0xF630,0, ADDR_IND16_REG16,0xF730,0, 	ADDR_IND16REG_REG8,0xF630,0, ADDR_IND16REG_REG16,0xF730,0, 
	0,0,0
  };

struct SUB_CODE sMn77[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn78[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn79[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn80[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn81[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn82[] = {
  ADDR_REG8IMM,0x00C8,4,  0,0,0
  };

struct SUB_CODE sMn83[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn84[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn85[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn86[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn87[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn88[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn89[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn90[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn91[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn92[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn93[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn94[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn95[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn96[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn97[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn98[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn99[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn100[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn101[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn102[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn103[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn104[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn105[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn106[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn107[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn108[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn109[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn110[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn111[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn112[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn113[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn114[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn115[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn116[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn117[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn118[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn119[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn120[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn121[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn122[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn123[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn124[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn125[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn126[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn127[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn128[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn129[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn130[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn131[] = {
  ADDR_IMPL,0xD0D9,4, 0,0,0
  };
  
struct SUB_CODE sMn132[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn133[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn134[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn135[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn136[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn137[] = {
  ADDR_IMPL,0xf8d9,4, 0,0,0
  };
  
struct SUB_CODE sMn138[] = {
  ADDR_IMPL,0xf5d9,4, 0,0,0
  };
  
struct SUB_CODE sMn139[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn140[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn141[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn142[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn143[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn144[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn145[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn146[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn147[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn148[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn149[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn150[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn151[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn152[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn153[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn154[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn155[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn156[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn157[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn158[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn159[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn160[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn161[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn162[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn163[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn164[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn165[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn166[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn167[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn168[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn169[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn170[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn171[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn172[] = {
  ADDR_IMPL,0x00f4,4, 0,0,0
  };

struct SUB_CODE sMn173[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn174[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn175[] = {
  ADDR_REG8IMM,0xF6F8,2, ADDR_REG16IMM,0xF7F8,4,
	ADDR_IND16_REG8,0xF638,0, ADDR_IND16_REG16,0xF738,0, 	ADDR_IND16REG_REG8,0xF638,0, ADDR_IND16REG_REG16,0xF738,0, 
	0,0,0
  };

struct SUB_CODE sMn176[] = {
  ADDR_REG8,0xF6E8,2, ADDR_REG16,0xF7E8,4, ADDR_REG8IMM,0xAF0F,2, ADDR_REG16IMM,0xAF0F,4,
	ADDR_IND16_REG8,0xF628,0, ADDR_IND16_REG16,0xF728,0, 	ADDR_IND16REG_REG8,0xF628,0, ADDR_IND16REG_REG16,0xF728,0, 
	ADDR_IND16REG_IMM8,0x006B,0, ADDR_IND16REG_IMM16,0x006B,0,		// tutti da verificare questi!
	0,0,0
  };

struct SUB_CODE sMn177[] = {
  ADDR_REG8IMM,0x00E4,2, ADDR_REG16IMM,0x00E5,4, ADDR_REG8REG,0x00EC,2, ADDR_REG16REG,0x00ED,4, 0,0,0
  };

struct SUB_CODE sMn178[] = {
  ADDR_REG8IMM,0xFEC0,2, ADDR_REG16IMM,0x0040 /* anche ff cmq*/,4, ADDR_IND16REG_REG8,0xFE06,2, ADDR_IND16REG_REG16,0xFF06,0, 0,0,0
	};

struct SUB_CODE sMn179[] = {
  ADDR_IMPL,0x006C,4, 0,0,0			// gestire in base a size
  };

struct SUB_CODE sMn180[] = {
  ADDR_IMPL,0x006C,4, 0,0,0
  };

struct SUB_CODE sMn181[] = {
  ADDR_IMPL,0x006D,4, 0,0,0
  };

struct SUB_CODE sMn182[] = {
  ADDR_IMPL,0x006D,4, 0,0,0
  };

struct SUB_CODE sMn183[] = {
  ADDR_IMPL,0x00CD,4,  0,0,0
  };

struct SUB_CODE sMn184[] = {
  ADDR_IMPL,0x00CC,4,  0,0,0
  };

struct SUB_CODE sMn185[] = {
  ADDR_IMPL,0x00CE,4,  0,0,0
  };

struct SUB_CODE sMn186[] = {
  ADDR_IMPL,0x0F08,1,  0,0,0
  };

struct SUB_CODE sMn187[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn188[] = {
  ADDR_IMPL,0x00CF,4,  0,0,0
  };

struct SUB_CODE sMn189[] = {
  ADDR_IMPL,0x00CF,4, 0,0,0
  };

struct SUB_CODE sMn190[] = {
  ADDR_IMM8,0x0070,0, ADDR_IMM16,0x0070,0, 0,0,0
  };

struct SUB_CODE sMn191[] = {
  ADDR_IMM8,0x00E3,2, ADDR_IMM16,0x00E3,4, 0,0,0
  };

struct SUB_CODE sMn192[] = {
  ADDR_IMM8,0x00EB,2, ADDR_IMM16,0x00E9,4, ADDR_IMM32,0x00EA,6, ADDR_REG16,0xE0FF,8, 0,0,0
  };

struct SUB_CODE sMn193[] = {
  ADDR_IMPL,0x009F,4, 0,0,0
  };

struct SUB_CODE sMn194[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn195[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn196[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn197[] = {
	ADDR_REG16_IND16,0xC500,0, 0,0,0
  };

struct SUB_CODE sMn198[] = {
	ADDR_REG16_IND16,0x8D00,0, 0,0,0
  };

struct SUB_CODE sMn199[] = {
	ADDR_IMM16,0x00C9,0, 0,0,0
  };

struct SUB_CODE sMn200[] = {
	ADDR_REG16_IND16,0xC400,0, 0,0,0
  };

struct SUB_CODE sMn201[] = {
	ADDR_IMM16,0xAE0F,0, 0,0,0
  };

struct SUB_CODE sMn202[] = {
	ADDR_REG16_IND16,0xB40F,0, 0,0,0
  };

struct SUB_CODE sMn203[] = {
  ADDR_IMPL,0x00C9,4,  0,0,0
  };
  
struct SUB_CODE sMn204[] = {
	ADDR_REG16_IND16,0xB50F,0, 0,0,0
  };
  
struct SUB_CODE sMn205[] = {
  ADDR_IMM16,0x010F,4,  0,0,0
  };
  
struct SUB_CODE sMn206[] = {
  ADDR_IMM16,0x000F,4,  0,0,0
  };
  
struct SUB_CODE sMn207[] = {
  ADDR_IMM16,0x010F,4,  0,0,0
  };
  
struct SUB_CODE sMn208[] = {
  ADDR_IMPL,0x00F0,4,  0,0,0
  };
  
struct SUB_CODE sMn209[] = {
  ADDR_IMPL,0x00AC,4, 0,0,0
  };
  
struct SUB_CODE sMn210[] = {
  ADDR_IMPL,0x00AC,4, 0,0,0			// gestire in base a size
  };
  
struct SUB_CODE sMn211[] = {
  ADDR_IMPL,0x00AD,4, 0,0,0
  };
  
struct SUB_CODE sMn212[] = {
  ADDR_IMPL,0x00AD,4, 0,0,0
  };
  
struct SUB_CODE sMn213[] = {
  ADDR_IMM8,0x00E2,0, ADDR_IMM16,0x00E2,0, 0,0,0
  };
  
struct SUB_CODE sMn214[] = {
  ADDR_IMPL,0x030F,4, 0,0,0
  };
  
struct SUB_CODE sMn215[] = {
	ADDR_IMM16,0xB20F,0, 0,0,0
  };
  
struct SUB_CODE sMn216[] = {
  ADDR_IMPL,0x000F,4, 0,0,0
  };
  
struct SUB_CODE sMn217[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn218[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn219[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn220[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn221[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn222[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn223[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn224[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn225[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn226[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn227[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn228[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn229[] = {
  ADDR_REG8,0x00B0,2, ADDR_REG16,0x00B8,4, ADDR_REG8IMM,0x0088,2, ADDR_REG16IMM,0x0089,4,
	ADDR_IND16_REG8,0x0088,0, ADDR_IND16_REG16,0x0089,0,  ADDR_IND16REG_REG8,0x0088,0, ADDR_IND16REG_REG16,0x0089,0, 
	ADDR_REG8_IND16,0x008A,0, ADDR_REG16_IND16,0x008B,0, 	ADDR_REG8_IND16REG,0x008A,0, ADDR_REG16_IND16REG,0x008B,0, 
	ADDR_IND16REG_IMM8,0xC600,0, ADDR_IND16REG_IMM16,0xC700,0, 
	// manca A0... A3 , mov AL/AX to address32 e viceversa
	0,0,0
  };
  
struct SUB_CODE sMn230[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn231[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn232[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn233[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn234[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn235[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn236[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn237[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn238[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn239[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn240[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn241[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn242[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn243[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn244[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn245[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn246[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn247[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn248[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn249[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn250[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn251[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn252[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn253[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn254[] = {
  ADDR_IMPL,0x00A4,4, 0,0,0			// gestire in base a size
  };
  
struct SUB_CODE sMn255[] = {
  ADDR_IMPL,0x00A4,4, 0,0,0
  };
  
struct SUB_CODE sMn256[] = {
  ADDR_IMPL,0x00A5,4, 0,0,0
  };
  
struct SUB_CODE sMn257[] = {		// MOVSS fare!
  ADDR_IMPL,0x00A5,4, 0,0,0
  };
  
struct SUB_CODE sMn258[] = {
  ADDR_IMPL,0x00A5,4, 0,0,0
  };
  
struct SUB_CODE sMn259[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn260[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn261[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn262[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn263[] = {
  ADDR_REG8IMM,0xF6E0,2, ADDR_REG16IMM,0xF7E0,4,
	ADDR_IND16_REG8,0xF620,0, ADDR_IND16_REG16,0xF720,0, ADDR_IND16REG_REG8,0xF620,0, ADDR_IND16REG_REG16,0xF720,0, 
	0,0,0
  };
  
struct SUB_CODE sMn264[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn265[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn266[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn267[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn268[] = {
  ADDR_IMPL,0x009B,4, 0,0,0
  };
  
struct SUB_CODE sMn269[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn270[] = {
  ADDR_IMPL,0x0090,4, 0,0,0
  };

struct SUB_CODE sMn271[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn272[] = {
  ADDR_REG8,0x000C,2, ADDR_REG16,0x000D,4, ADDR_REG8IMM,0x80C8,2, ADDR_REG16IMM,0x81C8,4, ADDR_REG8REG,0x0008,2, ADDR_REG16REG,0x0009,4,
	ADDR_IND16_REG8,0x0008,0, ADDR_IND16_REG16,0x0009,0,  ADDR_IND16REG_REG8,0x0008,0, ADDR_IND16REG_REG16,0x0009,0, 
	ADDR_REG8_IND16,0x000A,0, ADDR_REG16_IND16,0x000B,0, 	ADDR_REG8_IND16REG,0x000A,0, ADDR_REG16_IND16REG,0x000B,0, 
	ADDR_IND16REG_IMM8,0x8008,0, ADDR_IND16REG_IMM16,0x8108,0, 
	ADDR_IND16REG_IMM16_8,0x8308,0, 0,0,0
  };

struct SUB_CODE sMn273[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn274[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn275[] = {
  ADDR_REG8IMM,0x00E6,2, ADDR_REG16IMM,0x00E7,4, ADDR_REG8REG,0x00EE,2, ADDR_REG16REG,0x00EF,4, 0,0,0
  };

struct SUB_CODE sMn276[] = {
  ADDR_IMPL,0x006E,4, 0,0,0			// gestire in base a size
  };

struct SUB_CODE sMn277[] = {
  ADDR_IMPL,0x006E,4, 0,0,0
  };

struct SUB_CODE sMn278[] = {
  ADDR_IMPL,0x006F,4, 0,0,0
  };

struct SUB_CODE sMn279[] = {
  ADDR_IMPL,0x006F,4, 0,0,0
  };

struct SUB_CODE sMn280[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn281[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn282[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn283[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn284[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn285[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn286[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn287[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn288[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn289[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn290[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn291[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn292[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn293[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn294[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn295[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn296[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn297[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn298[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn299[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn300[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn301[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn302[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn303[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn304[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn305[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn306[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn307[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn308[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn309[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn310[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn311[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn312[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn313[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn314[] = {
  ADDR_REG16IMM,0x0058,4, ADDR_REG16,0x0007/*variante per reg segm*/,4, 
	ADDR_IND16_REG16,0x8F06,0, 
	0,0,0
  };
  
struct SUB_CODE sMn315[] = {
  ADDR_IMPL,0x0061,4,  0,0,0
  };
  
struct SUB_CODE sMn316[] = {
  ADDR_IMPL,0x0061,4,  0,0,0
  };
  
struct SUB_CODE sMn317[] = {
  ADDR_IMPL,0x009D,4,  0,0,0
  };
  
struct SUB_CODE sMn318[] = {
  ADDR_IMPL,0x669D,4,  0,0,0		// occhio prefisso è per 32bit se in segmento 16 :) ecc...
  };
  
struct SUB_CODE sMn319[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn320[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn321[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn322[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn323[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn324[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn325[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn326[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn327[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn328[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn329[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn330[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn331[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn332[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn333[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn334[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn335[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn336[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn337[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn338[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn339[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn340[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn341[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn342[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn343[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn344[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn345[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn346[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn347[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn348[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn349[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn350[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn351[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn352[] = {
  ADDR_REG16IMM,0x0050,4,
  ADDR_REG16,0x0006/*variante per reg segm*/,4, 
	ADDR_IND16_REG16,0xFF36,0,  ADDR_IND16REG_REG16,0x0019,0, 
	ADDR_IND16REG_REG16,0x00FF,0,  0,0,0
  };
  
struct SUB_CODE sMn353[] = {
  ADDR_IMPL,0x0060,4,  0,0,0
  };
  
struct SUB_CODE sMn354[] = {
  ADDR_IMPL,0x009C,4,  0,0,0
  };
  
struct SUB_CODE sMn355[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn356[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn357[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn358[] = {
  ADDR_REG8IMM,0xD0D0,2, ADDR_REG16IMM,0xD1D0,4, ADDR_REG8REG,0xD2D0,2, ADDR_REG16REG,0xD3D0,4, 
	ADDR_IND16REG_IMM8,0xD010,2, ADDR_IND16REG_IMM16,0xD210,0,
	ADDR_IND16_REG8,0xD010,2, ADDR_IND16_REG16,0xD210,0, ADDR_IND16REG_REG8,0xD010,2, ADDR_IND16REG_REG16,0xD210,0, 0,0,0
  };
  
struct SUB_CODE sMn359[] = {
  ADDR_REG8IMM,0xD0D8,2, ADDR_REG16IMM,0xD1D8,4, ADDR_REG8REG,0xD2D8,2, ADDR_REG16REG,0xD3D8,4,
	ADDR_IND16REG_IMM8,0xD018,2, ADDR_IND16REG_IMM16,0xD218,0,
	ADDR_IND16_REG8,0xD018,2, ADDR_IND16_REG16,0xD218,0, ADDR_IND16REG_REG8,0xD018,2, ADDR_IND16REG_REG16,0xD218,0, 0,0,0
  };
  
struct SUB_CODE sMn360[] = {
  ADDR_IMPL,0x00f3,4, 0,0,0
  };

struct SUB_CODE sMn361[] = {
  ADDR_IMPL,0x00c3,4, 0,0,0			// gestire far e valore...
  };

struct SUB_CODE sMn362[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn363[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn364[] = {
  ADDR_IMPL,0x310f,4,  0,0,0
  };
  
struct SUB_CODE sMn365[] = {
  ADDR_IMPL,0x00F3,4,  0,0,0
  };
  
struct SUB_CODE sMn366[] = {
  ADDR_IMPL,0x00F3,4,  0,0,0
  };
  
struct SUB_CODE sMn367[] = {
  ADDR_IMPL,0x00F2,4, 0,0,0
  };
  
struct SUB_CODE sMn368[] = {
  ADDR_IMPL,0x00F2,4, 0,0,0
  };
  
struct SUB_CODE sMn369[] = {
  ADDR_IMPL,0x00F3,4, 0,0,0
  };
  
struct SUB_CODE sMn370[] = {
  ADDR_IMPL,0x00C3,4, ADDR_IMPL,0x00CB/*far*/,4, ADDR_IMM16,0x00C2,4, ADDR_IMM16,0x00CA/*far*/,4, 0,0,0
  };

struct SUB_CODE sMn371[] = {
  ADDR_REG8IMM,0xD0C0,2, ADDR_REG16IMM,0xD1C0,4, ADDR_REG8REG,0xD2C0,2, ADDR_REG16REG,0xD3C0,4,
	ADDR_IND16REG_IMM8,0xD000,2, ADDR_IND16REG_IMM16,0xD200,0,
	ADDR_IND16_REG8,0xD000,2, ADDR_IND16_REG16,0xD200,0, ADDR_IND16REG_REG8,0xD000,2, ADDR_IND16REG_REG16,0xD200,0, 0,0,0
  };
  
struct SUB_CODE sMn372[] = {
  ADDR_REG8IMM,0xD0C8,2, ADDR_REG16IMM,0xD1C8,4, ADDR_REG8REG,0xD2C8,2, ADDR_REG16REG,0xD3C8,4,
	ADDR_IND16REG_IMM8,0xD008,2, ADDR_IND16REG_IMM16,0xD208,0,
	ADDR_IND16_REG8,0xD008,2, ADDR_IND16_REG16,0xD208,0, ADDR_IND16REG_REG8,0xD008,2, ADDR_IND16REG_REG16,0xD208,0, 0,0,0
  };
  
struct SUB_CODE sMn373[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn374[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn375[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn376[] = {
  ADDR_IMPL,0x009E,2,  0,0,0
  };

struct SUB_CODE sMn377[] = {
  ADDR_REG8IMM,0xD0E0,2, ADDR_REG16IMM,0xD1E0,4, ADDR_REG8REG,0xD2E0,2, ADDR_REG16REG,0xD3E0,4,
	ADDR_IND16REG_IMM8,0xD020,2, ADDR_IND16REG_IMM16,0xD220,0,
	ADDR_IND16_REG8,0xD020,2, ADDR_IND16_REG16,0xD220,0, ADDR_IND16REG_REG8,0xD020,2, ADDR_IND16REG_REG16,0xD220,0, 0,0,0
  };

struct SUB_CODE sMn378[] = {
  ADDR_REG8IMM,0xD0F8,2, ADDR_REG16IMM,0xD1F8,4, ADDR_REG8REG,0xD2F8,2, ADDR_REG16REG,0xD3F8,4,
	ADDR_IND16REG_IMM8,0xD038,2, ADDR_IND16REG_IMM16,0xD238,0,
	ADDR_IND16_REG8,0xD038,2, ADDR_IND16_REG16,0xD238,0, ADDR_IND16REG_REG8,0xD038,2, ADDR_IND16REG_REG16,0xD238,0, 0,0,0
  };

struct SUB_CODE sMn379[] = {
  ADDR_REG8,0x001C,2, ADDR_REG16,0x001D,4, ADDR_REG8IMM,0x80D8,2, ADDR_REG16IMM,0x81D8,4, ADDR_REG8REG,0x0018,2, ADDR_REG16REG,0x0019,4,
	ADDR_IND16_REG8,0x0018,0, ADDR_IND16_REG16,0x0019,0,  ADDR_IND16REG_REG8,0x0018,0, ADDR_IND16REG_REG16,0x0019,0, 
	ADDR_REG8_IND16,0x001A,0, ADDR_REG16_IND16,0x001B,0, 	ADDR_REG8_IND16REG,0x001A,0, ADDR_REG16_IND16REG,0x001B,0, 
	ADDR_IND16REG_IMM8,0x8018,0, ADDR_IND16REG_IMM16,0x8118,0, 
	ADDR_IND16REG_IMM16_8,0x8318,0, 0,0,0
  };

struct SUB_CODE sMn380[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn381[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn382[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn383[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn384[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn385[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn386[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn387[] = {
  ADDR_REG8IMM,0xD0F0,2, ADDR_REG16IMM,0xD1F0,4, ADDR_REG8REG,0xD2F0,2, ADDR_REG16REG,0xD3F0,4,
	ADDR_IND16REG_IMM8,0xD030,2, ADDR_IND16REG_IMM16,0xD230,0,
	ADDR_IND16_REG8,0xD030,2, ADDR_IND16_REG16,0xD230,0, ADDR_IND16REG_REG8,0xD030,2, ADDR_IND16REG_REG16,0xD230,0, 0,0,0
  };

struct SUB_CODE sMn388[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn389[] = {
  ADDR_REG8IMM,0xD0E8,2, ADDR_REG16IMM,0xD1E8,4, ADDR_REG8REG,0xD2E8,2, ADDR_REG16REG,0xD3E8,4,
	ADDR_IND16REG_IMM8,0xD028,2, ADDR_IND16REG_IMM16,0xD228,0,
	ADDR_IND16_REG8,0xD028,2, ADDR_IND16_REG16,0xD228,0, ADDR_IND16REG_REG8,0xD028,2, ADDR_IND16REG_REG16,0xD228,0, 0,0,0
  };

struct SUB_CODE sMn390[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn391[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn392[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn393[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn394[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn395[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn396[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn397[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn398[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn399[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn400[] = {
  ADDR_IMPL,0x00f9,4, 0,0,0
  };

struct SUB_CODE sMn401[] = {
  ADDR_IMPL,0x00fd,4, 0,0,0
  };

struct SUB_CODE sMn402[] = {
  ADDR_IMPL,0x00fb,4, 0,0,0
  };

struct SUB_CODE sMn403[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };

struct SUB_CODE sMn404[] = {
  ADDR_IMPL,0x00ac,4, 0,0,0			// gestire in base a size
  };
  
struct SUB_CODE sMn405[] = {
  ADDR_IMPL,0x00ac,4, 0,0,0
  };
  
struct SUB_CODE sMn406[] = {
  ADDR_IMPL,0x00ad,4, 0,0,0
  };
  
struct SUB_CODE sMn407[] = {
  ADDR_IMPL,0x00ad,4, 0,0,0
  };
  
struct SUB_CODE sMn408[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn409[] = {
  ADDR_REG8,0x002C,2, ADDR_REG16,0x002D,4, ADDR_REG8IMM,0x80E8,2, ADDR_REG16IMM,0x81E8,4, ADDR_REG8REG,0x0028,2, ADDR_REG16REG,0x0029,4,
	ADDR_IND16_REG8,0x0028,0, ADDR_IND16_REG16,0x0029,0,  ADDR_IND16REG_REG8,0x0028,0, ADDR_IND16REG_REG16,0x0029,0, 
	ADDR_REG8_IND16,0x002A,0, ADDR_REG16_IND16,0x002B,0, 	ADDR_REG8_IND16REG,0x002A,0, ADDR_REG16_IND16REG,0x002B,0, 
	ADDR_IND16REG_IMM8,0x8028,0, ADDR_IND16REG_IMM16,0x8128,0, 
	ADDR_IND16REG_IMM16_8,0x8328,0, 0,0,0
  };
  
struct SUB_CODE sMn410[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn411[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn412[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn413[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn414[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn415[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn416[] = {
  ADDR_REG8,0x00A8,2, ADDR_REG16,0x00A9,4, ADDR_REG8IMM,0x0084,2, ADDR_REG16IMM,0x0085,4, ADDR_REG8REG,0x0010,2, ADDR_REG16REG,0x0011,4,
	ADDR_IND16REG_IMM8,0xF600,0, ADDR_IND16REG_IMM16,0xF700,0, 
	0,0,0
  };
  
struct SUB_CODE sMn417[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn418[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn419[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn420[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn421[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn422[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn423[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn424[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn425[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn426[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn427[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn428[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn429[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn430[] = {
  ADDR_REG16,0x0090,4, 
	ADDR_IND16REG_REG8,0x0086,0, ADDR_IND16REG_REG16,0x0087,0,  0,0,0
  };
  
struct SUB_CODE sMn431[] = {
	ADDR_IMPL,0x00D7,0,  ADDR_IND16_REG8/*cmq finto!*/,0x00D7,0, 0,0,0
  };
  
struct SUB_CODE sMn432[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn433[] = {
  ADDR_REG8,0x0034,2, ADDR_REG16,0x0035,4, ADDR_REG8IMM,0x80F0,2, ADDR_REG16IMM,0x81F0,4, ADDR_REG8REG,0x0030,2, ADDR_REG16REG,0x0031,4,
	ADDR_IND16_REG8,0x0030,0, ADDR_IND16_REG16,0x0031,0,  ADDR_IND16REG_REG8,0x0030,0, ADDR_IND16REG_REG16,0x0031,0, 
	ADDR_REG8_IND16,0x0032,0, ADDR_REG16_IND16,0x0033,0, 	ADDR_REG8_IND16REG,0x0032,0, ADDR_REG16_IND16REG,0x0033,0, 
	ADDR_IND16REG_IMM8,0x8030,0, ADDR_IND16REG_IMM16,0x8130,0, 
	ADDR_IND16REG_IMM16_8,0x8330,0, 0,0,0
  };
  
struct SUB_CODE sMn434[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  
struct SUB_CODE sMn435[] = {
  ADDR_IMPL,0xffff,4,  0,0,0
  };
  


  
struct CODE Mnem[] = {
	// https://c9x.me/x86/
// il secondo valore (flag M) vale 1 per implicito
// 2 per A,r8 bit
// 3 per HL,r16 bit
// 4 per A,imm8
// 5 per A,(IX+), 6 per A,(IY+); 33 r8,(IX+);  34 r8,(IY+)
// 7 per IX,r16, 8 per IY,r16
// 9 per A,ind HL ;  32: r8,(HL)
// 10 per r8,r8 (o solo r8), 11 per r8,imm, 12 per (IX), 13 per (IY)
// 14 per imm16, 15 per HL,ix, 16 per HL,iy; 31 per HL,sp
// 17: (nn),A;  18: (nn),r16;  19: (r16),nn;  20: (r16),r8
// 21: A,(nn); 22: r16,(nn);  23: r16,imm;  24: (IX+),r8;  25: (IY+),r8; 26: (IX+),imm;  27: (IY+),imm
// 28: (r8),r8;  29: r8,(r8); 30: (r16),r16;  
// 41 per condiz, 40 condiz,imm
// 50: set 0,r8; 51 set 0,(hl); 52 set 0,(ix);  53: set 0,(iy)
// 60 per A,I; 61 per A,R; 62 per I,A; 63 per R,A
  "AAA",sMn1,
  "AAD",sMn2,
  "AAS",sMn3,
  "ADC",sMn4,
  "ADD",sMn5,
  "ADDPD",sMn6,
  "ADDPS",sMn7,
  "ADDSD",sMn8,
  "ADDSS",sMn9,
  "ADDSUBPD",sMn10,
  "ADDSUBPS",sMn11,
  "AND",sMn12,
  "ANDNPD",sMn13,
  "ANDNPS",sMn14,
  "ANDPD",sMn15,
  "ANDPS",sMn16,
  "ARPL",sMn17,
  "BOUND",sMn18,
  "BSF",sMn19,
  "BSR",sMn20,
  "BSWAP",sMn21,
  "BT",sMn22,
  "BTC",sMn23,
  "BTR",sMn24,
  "BTS",sMn25,
  "CALL",sMn26,
  "CBW",sMn27,
  "CDQ",sMn28,
  "CLC",sMn29,
  "CLD",sMn30,
  "CLFLUSH",sMn31,
  "CLI",sMn32,
  "CLTS",sMn33,
  "CMC",sMn34,
  "CMOV",sMn35,			// CMOVcc
  "CMP",sMn36,
  "CMPPD",sMn37,
  "CMPPS",sMn38,
  "CMPS",sMn39,
  "CMPSB",sMn40,
  "CMPSD",sMn41,
  "CMPSS",sMn42,
  "CMPSW",sMn43,
  "CMPXCHG",sMn44,
  "CMPXCHG8B",sMn45,
  "COMISD",sMn46,
  "COMISS",sMn47,
  "CPUID",sMn48,
  "CVTDQ2PD",sMn49,
  "CVTDQ2PS",sMn50,
  "CVTPD2DQ",sMn51,
  "CVTPD2PI",sMn52,
  "CVTPD2PS",sMn53,
  "CVTPI2PD",sMn54,
  "CVTPI2PS",sMn55,
  "CVTPS2DQ",sMn56,
  "CVTPS2PD",sMn57,
  "CVTPS2PI",sMn58,
  "CVTSD2SI",sMn59,
  "CVTSD2SS",sMn60,
  "CVTSI2SD",sMn61,
  "CVTSI2SS",sMn62,
  "CVTSS2SD",sMn63,
  "CVTSS2SI",sMn64,
  "CVTTPD2PI",sMn65,
  "CVTTPD2DQ",sMn66,
  "CVTTPS2DQ",sMn67,
  "CVTTPS2PI",sMn68,
  "CVTTSD2SI",sMn69,
  "CVTTSS2SI",sMn70,
  "CWD",sMn71,
  "CWDE",sMn72,
  "DAA",sMn73,
  "DAS",sMn74,
  "DEC",sMn75,
  "DIV",sMn76,
  "DIVPD",sMn77,
  "DIVPS",sMn78,
  "DIVSD",sMn79,
  "DIVSS",sMn80,
  "EMMS",sMn81, 
  "ENTER",sMn82,
  "F2XM1",sMn83,
  "FABS",sMn84,
  "FADD",sMn85,
  "FADDP",sMn86,
  "FBLD",sMn87,
  "FBSTP",sMn88,
  "FCHS",sMn89,
  "FCLEX",sMn90,
  "FCMOV",sMn91,			// FCMOVcc
  "FCOM",sMn92,
  "FCOMP",sMn93,
  "FCOMPP",sMn94,
  "FCOMI",sMn95,
  "FCOMIP",sMn96,
  "FCOS",sMn97,
  "FDECSTP",sMn98,
  "FDIV",sMn99,
  "FDIVP",sMn100,
  "FDIVR",sMn101,
  "FDIVRP",sMn102,
  "FFREE",sMn103,
  "FICOM",sMn104,
  "FICOMP",sMn105,
  "FIDIV",sMn106,
  "FIDIVR",sMn107,
  "FILD",sMn108,
  "FIMUL",sMn109,
  "FINCSTP",sMn110,
  "FINIT",sMn111,
  "FIST",sMn112,
  "FISTP",sMn113,
  "FISTTP",sMn114,
  "FISUB",sMn115,
  "FISUBR",sMn116,
  "FLD",sMn117,
  "FLD1",sMn118,
  "FLDCW",sMn119,
  "FLDENV",sMn120,
  "FLDL2E",sMn121,
  "FLDL2T",sMn122,
  "FLDLG2",sMn123,
  "FLDLN2",sMn124,
  "FLDPI",sMn125,
  "FLDZ",sMn126,
  "FMUL",sMn127,
  "FMULP",sMn128,
  "FNCLEX",sMn129,
  "FNINIT",sMn130,
  "FNOP",sMn131,
  "FNSAVE",sMn132,
  "FNSTCW",sMn133,
  "FNSTENV",sMn134,
  "FNSTSW",sMn135,
  "FPATAN",sMn136,
  "FPREM",sMn137,
  "FPREM1",sMn138,
  "FPTAN",sMn139,
  "FRNDINT",sMn140,
  "FRSTOR",sMn141,
  "FSAVE",sMn142,
  "FSCALE",sMn143,
  "FSIN",sMn144,
  "FSINCOS",sMn145,
  "FSQRT",sMn146,
  "FST",sMn147,
  "FSTCW",sMn148,
  "FSTENV",sMn149,
  "FSTP",sMn150,
  "FSTSW",sMn151,
  "FSUB",sMn152,
  "FSUBP",sMn153,
  "FSUBR",sMn154,
  "FSUBRP",sMn155,
  "FTST",sMn156,
  "FUCOM",sMn157,
  "FUCOMI",sMn158,
  "FUCOMIP",sMn159,
  "FUCOMP",sMn160,
  "FUCOMPP",sMn161,
  "FWAIT",sMn162,
  "FXAM",sMn163,
  "FXCH",sMn164,
  "FXRSTOR",sMn165,
  "FXSAVE",sMn166,
  "FXTRACT",sMn167,
  "FYL2X",sMn168,
  "FYL2XP1",sMn169,
  "HADDPD",sMn170,
  "HADDPS",sMn171,
  "HLT",sMn172,
  "HSUBPD",sMn173,
  "HSUBPS",sMn174,
  "IDIV",sMn175,
  "IMUL",sMn176,
  "IN",sMn177,
  "INC",sMn178,
  "INS",sMn179,
  "INSB",sMn180,
  "INSD",sMn181,
  "INSW",sMn182,
  "INT",sMn183,			// 
  "INT3",sMn184,
  "INTO",sMn185,
  "INVD",sMn186,
  "INVLPG",sMn187,
  "IRET",sMn188,
  "IRETD",sMn189,
  "J",sMn190,		// Jcc
  "JCXZ",sMn191,
  "JMP",sMn192,			// onlineassembler accetta anche LJMP come JMP FAR, ma DEBUG no...
  "LAHF",sMn193,
  "LAR",sMn194,
  "LDDQU",sMn195,
  "LDMXCSR",sMn196,
  "LDS",sMn197,
  "LEA",sMn198,
  "LEAVE",sMn199,
  "LES",sMn200,
  "LFENCE",sMn201,
  "LFS",sMn202,
  "LGDT",sMn203,
  "LGS",sMn204,
  "LIDT",sMn205,
  "LLDT",sMn206,
  "LMSW",sMn207,
  "LOCK",sMn208,
  "LODS",sMn209,
  "LODSB",sMn210,
  "LODSD",sMn211,
  "LODSW",sMn212,
  "LOOP",sMn213,			// LOOPcc
  "LSL",sMn214,
  "LSS",sMn215,
  "LTR",sMn216,
  "MASKMOVDQU",sMn217,
  "MASKMOVQ",sMn218,
  "MAXPD",sMn219,
  "MAXPS",sMn220,
  "MAXSD",sMn221,
  "MAXSS",sMn222,
  "MFENCE",sMn223,
  "MINPD",sMn224,
  "MINPS",sMn225,
  "MINSD",sMn226,
  "MINSS",sMn227,
  "MONITOR",sMn228,
  "MOV",sMn229,
  "MOVAPD",sMn230,
  "MOVAPS",sMn231,
  "MOVD",sMn232,
  "MOVDDUP",sMn233,
  "MOVDQA",sMn234,
  "MOVDQU",sMn235,
  "MOVDQ2Q",sMn236,
  "MOVHLPS",sMn237,
  "MOVHPD",sMn238,
  "MOVHPS",sMn239,
  "MOVLHPS",sMn240,
  "MOVLPD",sMn241,
  "MOVLPS",sMn242,
  "MOVMSKPD",sMn243,
  "MOVMSKPS",sMn244,
  "MOVNTDQ",sMn245,
  "MOVNTI",sMn246,
  "MOVNTPD",sMn247,
  "MOVNTPS",sMn248,
  "MOVNTQ",sMn249,
  "MOVSHDUP",sMn250,
  "MOVSLDUP",sMn251,
  "MOVQ",sMn252,
  "MOVQ2DQ",sMn253,
  "MOVS",sMn254,
  "MOVSB",sMn255,
  "MOVSD",sMn256,
  "MOVSS",sMn257,
  "MOVSW",sMn258,
  "MOVSX",sMn259,
  "MOVUPD",sMn260,
  "MOVUPS",sMn261,
  "MOVZX",sMn262,
  "MUL",sMn263,
  "MULPD",sMn264,
  "MULPS",sMn265,
  "MULSD",sMn266,
  "MULSS",sMn267,
  "MWAIT",sMn268,
  "NEG",sMn269,
  "NOP",sMn270,
  "NOT",sMn271,
  "OR",sMn272,
  "ORPD",sMn273,
  "ORPS",sMn274,
  "OUT",sMn275,
  "OUTS",sMn276,
  "OUTSB",sMn277,
  "OUTSD",sMn278,
  "OUTSW",sMn279,
  "PACKSSDW",sMn280,
  "PACKSSWB",sMn281,
  "PACKUSWB",sMn282,
  "PADDB",sMn283,
  "PADDD",sMn284,
  "PADDW",sMn285,
  "PADDQ",sMn286,
  "PADDSB",sMn287,
  "PADDSW",sMn288,
  "PADDUSB",sMn289,
  "PADDUSW",sMn290,
  "PAND",sMn291,
  "PANDN",sMn292,
  "PAUSE",sMn293,
  "PAVGB",sMn294,
  "PAVGW",sMn295,
  "PCMPEQB",sMn296,
  "PCMPEQD",sMn297,
  "PCMPEQW",sMn298,
  "PCMPGTB",sMn299,
  "PCMPGTD",sMn300,
  "PCMPGTW",sMn301,
  "PEXTRW",sMn302,
  "PINSRW",sMn303,
  "PMADDWD",sMn304,
  "PMAXSW",sMn305,
  "PMAXUB",sMn306,
  "PMINSW",sMn307,
  "PMINUB",sMn308,
  "PMOVMSKB",sMn309,
  "PMULHUW",sMn310,
  "PMULHW",sMn311,
  "PMULLW",sMn312,
  "PMULUDQ",sMn313,
  "POP",sMn314,
  "POPA",sMn315,
  "POPAD",sMn316,
  "POPF",sMn317,
  "POPFD",sMn318,
  "POR",sMn319,
  "PREFETCH",sMn320,			//h
  "PSADBW",sMn321,
  "PSHUFD",sMn322,
  "PSHUFHW",sMn323,
  "PSHUFLW",sMn324,
  "PSHUFW",sMn325,
  "PSLLDQ",sMn326,
  "PSLLD",sMn327,
  "PSLLQ",sMn328,
  "PSLLW",sMn329,
  "PSRAD",sMn330,
  "PSRAW",sMn331,
  "PSRLDQ",sMn332,
  "PSRLD",sMn333,
  "PSRLQ",sMn334,
  "PSRLW",sMn335,
  "PSUBB",sMn336,
  "PSUBD",sMn337,
  "PSUBQ",sMn338,
  "PSUBW",sMn339,
  "PSUBSB",sMn340,
  "PSUBSW",sMn341,
  "PSUBUSB",sMn342,
  "PSUBUSW",sMn343,
  "PUNPCKHBW",sMn344,
  "PUNPCKHDQ",sMn345,
  "PUNPCKHWD",sMn346,
  "PUNPCKHQDQ",sMn347,
  "PUNPCKLBW",sMn348,
  "PUNPCKLDQ",sMn349,
  "PUNPCKLWD",sMn350,
  "PUNPCKLQDQ",sMn351,
  "PUSH",sMn352,
  "PUSHA",sMn353,
  "PUSHAD",sMn354,
  "PUSHF",sMn355,
  "PUSHFD",sMn356,
  "PXOR",sMn357,
  "RCL",sMn358,
  "RCR",sMn359,
  "RCPPS",sMn360,
  "RCPSS",sMn361,
  "RDMSR",sMn362,
  "RDPMC",sMn363,
  "RDTSC",sMn364,
  "REP",sMn365,
  "REPE",sMn366,
  "REPNE",sMn367,
  "REPNZ",sMn368,
  "REPZ",sMn369,
  "RET",sMn370,
  "ROL",sMn371,
  "ROR",sMn372,
  "RSM",sMn373,
  "RSQRTPS",sMn374,
  "RSQRTSS",sMn375,
  "SAHF",sMn376,
  "SAL",sMn377,
  "SAR",sMn378,
  "SBB",sMn379,
  "SCAS",sMn380,
  "SCASB",sMn381,
  "SCASD",sMn382,
  "SCASW",sMn383,
  "SET",sMn384,			// SETcc
  "SFENCE",sMn385,
  "SGDT",sMn386,
  "SHL",sMn387,
  "SHLD",sMn388,
  "SHR",sMn389,
  "SHRD",sMn390,
  "SHUFPD",sMn391,
  "SHUFPS",sMn392,
  "SIDT",sMn393,
  "SLDT",sMn394,
  "SMSW",sMn395,
  "SQRTPD",sMn396,
  "SQRTPS",sMn397,
  "SQRTSD",sMn398,
  "SQRTSS",sMn399,
  "STC",sMn400,
  "STD",sMn401,
  "STI",sMn402,
  "STMXCSR",sMn403,
  "STOS",sMn404,
  "STOSB",sMn405,
  "STOSD",sMn406,
  "STOSW",sMn407,
  "STR",sMn408,
  "SUB",sMn409,
  "SUBPD",sMn410,
  "SUBPS",sMn411,
  "SUBSD",sMn412,
  "SUBSS",sMn413,
  "SYSENTER",sMn414,
  "SYSEXIT",sMn415,
  "TEST",sMn416,
  "UCOMISD",sMn417,
  "UCOMISS",sMn418,
  "UD2",sMn419,
  "UNPCKHPD",sMn420,
  "UNPCKHPS",sMn421,
  "UNPCKLPD",sMn422,
  "UNPCKLPS",sMn423,
  "VERR",sMn424,
  "VERW",sMn425,
  "WAIT",sMn426,
  "WBINVD",sMn427,
  "WRMSR",sMn428,
  "XADD",sMn429,
  "XCHG",sMn430,
  "XLAT",sMn431,
  "XLATB",sMn432,
  "XOR",sMn433,
  "XORPD",sMn434,
  "XORPS",sMn435
  };
#endif  
  

#if Z80
struct REGS Reg[] = {
	"A",7,
	"B",0,
	"C",1,
	"D",2,
	"E",3,
//	"F",4,
	"H",4,
	"L",5,
	"I",16,
	"R",17,
	"AF",3,
	"BC",0,
	"DE",1,
	"HL",2,
	"IX",16,
	"IY",17,
	"SP",0x13,
	"AF'",0x23,
  NULL,0
  };

#elif I8051
struct REGS Reg[] = {
	"A",0,
	"B",1,
	"C",2,
	"R0",0x10,
	"R1",0x11,
	"R2",0x12,
	"R3",0x13,
	"R4",0x14,
	"R5",0x15,
	"R6",0x16,
	"R7",0x17,
	"DPTR",0x20,   //11
//	"SP",0x10,
  NULL,0
  };

#elif ST62
struct REGS Reg[] = {
	"A",0,
	"X",1,
	"Y",2,
	"V",3,
	"W",4,
  NULL,0
  };

#elif _6502
struct REGS Reg[] = {
	"A",0,
	"X",1,
	"Y",2,
	"S",3,
	"P",4,
  NULL,0
  };

#elif PIC==16 || PIC==18
struct REGS Reg[] = {
	"W",0,
  NULL,0
  };

#elif GD24032
struct REGS Reg[] = {
	"R0",0x10,
	"R1",0x11,
	"R2",0x12,
	"R3",0x13,
	"R4",0x14,
	"R5",0x15,
	"R6",0x16,
	"R7",0x17,
	"R8",0x18,
	"R9",0x19,
	"R10",0x1a,
	"R11",0x1b,
	"R12",0x1c,
	"R13",0x1d,
	"R14",0x1e,
	"R15",0x1f,
	"R16",0x20,
	"R17",0x21,
	"R18",0x22,
	"R19",0x23,
	"R20",0x24,
	"R21",0x25,
	"R22",0x26,
	"R23",0x27,
	"R24",0x28,
	"R25",0x29,
	"R26",0x2a,
	"R27",0x2b,
	"R28",0x2c,
	"R29",0x2d,
	"R30",0x2e,
	"R31",0x2f,
	"WP",0,
	"ST",1,
  NULL,0
  };

#elif I8086
struct REGS Reg[] = {
	"AL",0,
	"CL",1,
	"DL",2,
	"BL",3,
	"AH",4,
	"CH",5,
	"DH",6,
	"BH",7,
	"AX",8,
	"CX",9,
	"DX",10,
	"BX",11,
	"SP",12,
	"BP",13,
	"SI",14,
	"DI",15,
	"ES",16,
	"CS",17,
	"SS",18,
	"DS",19,
	"EAX",32,
	"ECX",33,
	"EDX",34,
	"EBX",35,
	"ESP",36,
	"EBP",37,
	"EDI",38,
	"ESI",39,
	"FS",40,
	"GS",41,
//	"F",0x23,
	"RAX",64,		// per istruzioni coprocessore math e/o mmx?
	"RCX",65,
	"RDX",66,
	"RBX",367,
  NULL,0
  };

#endif
  
struct MY_CODE MCom[] = {
  "LABEL",0x0,
  "DB",1,"DEFB",1,"BYTE",1,
  "DW",2,"DEFW",2,"WORD",2,
  "DD",4,"DEFL",4,
  "DQ",8,
#ifdef I8086
  "REAL4",4,
  "REAL8",8,
  "REAL10",10,
#endif
#ifndef I8086
  "DS",0x10,			// perché va come Segmento!
#endif
	"DEFS",0x10,      // Define Space(n)
  "STRING",0x11,"ASCIIZ",0x11,  // 
  "ASCII",0x12,"DEFM",0x12,   // 
#if PIC==16 || PIC==18
  "DT",0x12,"DATA",0x12,
#endif
  "FILL",0x13,   // 
  "ORG",0x20,
  "ALIGN",0x21,
#if ST62  
  "BLOCK",0x22,   // blocca (riserva, salta) un numero di byte e quindi allinea
  								// usato in generale per lo ST62 come: .block  64-$%64
#endif
  "EQU",0x100,"SET",0x100,
#if ST62  
  "DEF",0x100,
#endif
  "=",0x100,
//  "COUNT",0x101,
  "ENDC",0x102,
  "CSEG",0x1000,"TEXT",0x1000,"PSECT",0x1000,
#ifdef I8086
	"SEGMENT",0x1000,
#endif
  "DSEG",0x1001,
  "CSECT",0x1002,
  "DSECT",0x1003,
  "CONST",0x1004,"RODATA",0x1004,
  "BSS",0x1005,"ZDATA",0x1005,		//			fare		https://www.rowleydownload.co.uk/msp430/documentation/index.htm?https://www.rowleydownload.co.uk/msp430/documentation/has_directive_keep.htm
  "ISR",0x1006,		// fare...
  "ESEG",0x1007,		// da microchip, per EEprom
  "PROC",0x1010,"ENTRY",0x1010,
  "ENDP",0x1011,
  "END",0x1012,
  "PUBLIC",0x1020,"EXPORT",0x1020,
  "EXTRN",0x1021,
  "NAME",0x1030,
#if GD24032
  "ASSUME32",0x1040,
  "ASSUME64",0x1041,
#endif
#ifdef I8086
  "ASSUME",0x1040,
#endif
  "TITLE",0x2000,
  "PAGE",0x2001,
  "LINE",0x2002,
  "%",0x2003,
  "EJECT",0x2004,
#if ST62
  "DISPLAY",0x2005,
#endif
#if PIC==16
  "LIST",0x2005,
  "ERRORLEVEL",0x2006,
#endif
#if ST62
  "VERS",0x2010,      // il tipo di uP: 62E10, 62E20
  "W_ON",0x2011,      // modo memoria a finestre
#endif
#if PIC==16 || PIC==18
  "__CONFIG",0x2010,"CONFIG",0x2010,      // 
  "__IDLOCS",0x2011,
#endif
#ifdef I8086
	"CPU",0x2101,// anche i vari .8086 .80186 ecc...
#endif
  "MACRO",0x4000,
  "EXITM",0x4001,
  "ENDM",0x4002,
  "STRUC",0x4100,
  "UNION",0x4101,
  "RECORD",0x4102,
  "ENDS",0x410F,
  "INCLUDE",0x8000, "INPUT",0x8000,
  "IF",0x8010,"IFE",0x8011,"IFDEF",0x8012,"IFNDEF",0x8013,"IFIDN",0x8014,"IFDIF",0x8015,"IFIDNI",0x8016,"IFDIFI",0x8017,"IFB",0x8018,"IFNB",0x8019,
  "DEFINE",0x8030,
  "ELSE",0x8020,
  "ELSEIF",0x8021,
	// anche TUTTI quelli come IF ELSEIFDIF ecc! v. masm
  "ENDIF",0x802f,
  "PRAGMA",0x8080,
  "WARNING",0x8090,
  "ERROR",0x8091,
  NULL,0
  };

#if Z80  
struct COND Cond[]= {
  "C",0x18,
  "NC",0x10,
  "M",0x38,
  "P",0x30,
  "Z",8,
  "NZ",0,
  "PE",0x28,
  "PO",0x20,
  NULL,0
  };

#elif ST62
struct COND Cond[]= {
  "C",0x60,
  "NC",0xe0,
  "Z",0x40,
  "NZ",0x80,
  NULL,0
  };

#elif _6502		// non usate cmq...
struct COND Cond[]= {
  "EQ",0xf0,
  "NE",0xd0,
  "VC",0x50,
  "VS",0x70,
	  "RA",0x80,
  "CC",0x90,
  "CS",0xb0,
  "PL",0x10,
  "MI",0x30,
  NULL,0
  };

#elif PIC==16 
struct COND Cond[]= {		// boh :)
  "SC",0x0,
  "SS",0x1,
  NULL,0
  };

#elif PIC==18
struct COND Cond[]= {		// boh :)
  "SC",0x0,
  "SS",0x1,
  "Z",0x0,
  "NZ",0x1,
  "OV",0x4,
  "NOV",0x5,
  "RA",0x0,
  "NC",0x3,
  "C",0x2,
  "NN",0x7,
  "N",0x6,
  NULL,0
  };

#elif GD24032
struct COND Cond[]= {		// boh :)
  "EQ",0x0,
  "Z",0x0,
  "NE",0x1,
  "NZ",0x1,		// beh serve!
  "C",0x2,
  "NC",0x3,
  "MI",0x4,
  "PL",0x5,
  "V",0x6,
  "NV",0x7,
  "HI",0x8,
  "LS",0x9,
  "GE",0xa,
  "LT",0xb,
  "GT",0xc,
  "LE",0xd,
  "PE",0xe,
  "PO",0xf,
  NULL,0
  };

#elif I8086
struct COND Cond[]= {		// FINIRE!
  "O",0x0,
  "NO",0x1,
  "B",0x2,
  "C",0x2,
  "NAE",0x2,
  "NB",0x3,
  "NC",0x3,
  "AE",0x3,
  "E",0x4,
  "Z",0x4,
  "NE",0x5,
  "NZ",0x5,
  "BE",0x6,
  "NA",0x6,
  "NBE",0x7,
  "A",0x7,

  "S",0x8,
  "NS",0x9,
  "P",0xa,
  "PE",0xa,
  "NP",0xb,
  "PO",0xb,
  "L",0xc,
  "NGE",0xc,
  "NL",0xd,
  "GE",0xd,
  "LE",0xe,
  "NG",0xe,
  "NLE",0xf,
  "G",0xf,
  NULL,0
  };
#endif  


#include "as.h"

#if Z80
struct SUB_CODE sMn1[] = {
  10,0xa8,4, 4,0xce,7, 5,0xdd8e,19, 9,0x8e,7, 3,0xed4a,15, 2,0x88,4, 0,0,0
  };
  
struct SUB_CODE sMn2[] = {
  2,0x80,4, 3,0x09,11, 4,0xc6,4, 5,0xdd86,19, 7,0xdd09,15, 8,0xfd09,15, 9,0x86,7, 0,0,0
  };
  
struct SUB_CODE sMn3[] = {
  10,0xa0,4, 4,0xe6,7, 5,0xdda6,19, 6,0xfda6,19, 9,0xa6,7, 0,0,0
  };
  
struct SUB_CODE sMn4[] = {
  50,0xcb40,8, 51,0xcb46,12, 52,0xddcb46,20, 0,0,0
  };
  
struct SUB_CODE sMn5[] = {
  4,0xcd,0, 14,0xcd,17, 40,0xc4,0, 0,0,0
  };
  
struct SUB_CODE sMn6[] = {
  1,0x3f,4, 0,0,0
  };
  
struct SUB_CODE sMn7[] = {
  10,0xb8,4, 4,0xfe,7, 9,0xbe,7, 5,0xddbe,19, 0,0,0
  };
  
struct SUB_CODE sMn8[] = {
  1,0xeda9,4, 0,0,0
  };
  
struct SUB_CODE sMn9[] = {
  1,0xedb9,4, 0,0,0
  };
  
struct SUB_CODE sMn10[] = {
  1,0xeda1,4, 0,0,0
  };
  
struct SUB_CODE sMn11[] = {
  1,0xedb1,4, 0,0,0
  };
  
struct SUB_CODE sMn12[] = {
  1,0x2f,4, 0,0,0
  };
  
struct SUB_CODE sMn13[] = {
  1,0x27,4, 0,0,0
  };
  
struct SUB_CODE sMn14[] = {
  10,0x5,4, 9,0x35,11, 3,0xb,6, 15,0xdd2b,10, 16,0xfd2b,10, 5,0xdd35,23, 31,0x3b,6, 0,0,0
  };
  
struct SUB_CODE sMn15[] = {
  1,0xf3,4, 0,0,0
  };
  
struct SUB_CODE sMn16[] = {
  4,0x10,0, 14,0x10,13 << 8 | 8, 0,0,0
  };
  
struct SUB_CODE sMn17[] = {
  1,0xfb,4, 0,0,0
  };
  
struct SUB_CODE sMn18[] = {
  30,0xe3/* gestito a sè*/,0, 3,0xeb,0, 0,0,0
  };
  
struct SUB_CODE sMn19[] = {
  1,0xd9,4, 0,0,0
  };
  
struct SUB_CODE sMn20[] = {
  1,0x76,4, 0,0,0
  };
  
struct SUB_CODE sMn21[] = {
  4,0xed46, 0,0,0
  };
  
struct SUB_CODE sMn22[] = {
  29,0xed40,0, 21,0xdb,0, 0,0,0
  };
  
struct SUB_CODE sMn23[] = {
  10,0x4,4, 3,0x3,6, 9,0x34,11, 5,0xdd34,23, 15,0xdd23,10, 16,0xfd23,10, 31,0x33,6, 0,0,0
  };
  
struct SUB_CODE sMn24[] = {
  1,0xedaa,4, 0,0,0
  };
  
struct SUB_CODE sMn25[] = {
  1,0xedba,4, 0,0,0
  };
  
struct SUB_CODE sMn26[] = {
  1,0xeda2,4, 0,0,0
  };
  
struct SUB_CODE sMn27[] = {
  1,0xedb2,4, 0,0,0
  };
  
struct SUB_CODE sMn28[] = {
  4,0xc3,10, 14,0xc3,10, 40,0xc2,10 << 8 | 10, 9,0xe9,4, 12,0xdde9,8, 0,0,0    // accetto sia imm8 che imm16
  };
  
struct SUB_CODE sMn29[] = {
  4,0x18,12, 14,0x18,12, 40,0x20,12 << 8 | 7, 0,0,0
  };
  
struct SUB_CODE sMn30[] = {
  2,0x78,4, 4,0x3e/*sia A che r8*/,7, 32,0x46/*gestito anche per bc..*/,7, 9,0x7e,7, 10,0x40,4, 11,0x6,7, 5,0xdd7e,19, 33,0xdd46,19, 34,0xfd46,19, 17,0x32,13,
    18,0x22/*gestito a sé*/,16, 19,0x36,10, 20,0x70/*idem*/,7, 21,0x3a,13, 22,0x2a/**/,16, 23,0x1,10, 24,0xdd70,19, 25,0xfd70,19,
    63,0xed4f,9, 62,0xed47,9, 61,0xed5f,9, 60,0xed57,9, 26,0xdd36,19, 27,0xfd36,19, 0,0,0
  };
  
struct SUB_CODE sMn31[] = {
  1,0xeda8,16, 0,0,0
  };
  
struct SUB_CODE sMn32[] = {
  1,0xedb8,16, 0,0,0
  };
  
struct SUB_CODE sMn33[] = {
  1,0xeda0,16, 0,0,0
  };
  
struct SUB_CODE sMn34[] = {
  1,0xedb0,21 << 8 | 16, 0,0,0
  };
  
struct SUB_CODE sMn35[] = {
  1,0xed44,4, 0,0,0
  };
  
struct SUB_CODE sMn36[] = {
  1,0x0,4, 0,0,0
  };
  
struct SUB_CODE sMn37[] = {
  10,0xb0,4, 4,0xf6,7, 9,0xb6,7, 5,0xddb6,19, 0,0,0
  };
  
struct SUB_CODE sMn38[] = {
  1,0xedbb,21 << 8 | 16, 0,0,0
  };
  
struct SUB_CODE sMn39[] = {
  1,0xedb3,20 << 8 | 15, 0,0,0
  };
  
struct SUB_CODE sMn40[] = {
  28,0xed41,12, 17,0xd3,11, 0,0,0
  };
  
struct SUB_CODE sMn41[] = {
  1,0xedab,15, 0,0,0
  };
  
struct SUB_CODE sMn42[] = {
  1,0xeda3,15, 0,0,0
  };
  
struct SUB_CODE sMn43[] = {
  3,0xc1,10, 15,0xdde1,14, 16,0xfde1,14, 0,0,0
  };
  
struct SUB_CODE sMn44[] = {
  3,0xc5,11, 15,0xdde5,15, 16,0xfde5,15, 0,0,0
  };
  
struct SUB_CODE sMn45[] = {
  50,0xcb80,8, 51,0xcb86,15, 52,0xddcb86,23, 0,0,0
  };
  
struct SUB_CODE sMn46[] = {
  1,0xc9,10, 41,0xc0,11 << 8 | 5, 0,0,0
  };
  
struct SUB_CODE sMn47[] = {
  1,0xed4d,14, 0,0,0
  };
  
struct SUB_CODE sMn48[] = {
  1,0xed45,14, 0,0,0
  };
  
struct SUB_CODE sMn49[] = {
  10,0xcb10,4, 9,0xcb16,0, 5,0xddcb16,23, 0,0,0
  };
  
struct SUB_CODE sMn50[] = {
  1,0x17,4, 0,0,0
  };
  
struct SUB_CODE sMn51[] = {
  10,0xcb00,4, 9,0xcb06,0, 5,0xddcb06,23, 0,0,0
  };
  
struct SUB_CODE sMn52[] = {
  1,0x7,4, 0,0,0
  };
  
struct SUB_CODE sMn53[] = {
  1,0xed6f,4, 0,0,0
  };
  
struct SUB_CODE sMn54[] = {
  10,0xcb18,4, 9,0xcb1e,0, 5,0xddcb1e,23, 0,0,0
  };
  
struct SUB_CODE sMn55[] = {
  1,0x1f,4, 0,0,0
  };
  
struct SUB_CODE sMn56[] = {
  10,0xcb08,4, 9,0xcb0e,15, 5,0xddcb0e,23, 0,0,0
  };
  
struct SUB_CODE sMn57[] = {
  1,0xf,4, 0,0,0
  };
  
struct SUB_CODE sMn58[] = {
  1,0xed67,4, 0,0,0
  };
  
struct SUB_CODE sMn59[] = {
  4,0xc7,11, 0,0,0
  };
  
struct SUB_CODE sMn60[] = {
  10,0x98,4, 4,0xde,7, 3,0xed42,15, 9,0x9e,7, 2,0x98,4, 5,0xdd9e,19, 0,0,0
  };
  
struct SUB_CODE sMn61[] = {
  1,0x37,4, 0,0,0
  };
  
struct SUB_CODE sMn62[] = {
  50,0xcbc0,8, 51,0xcbc6,15, 52,0xddcbc6,23, 53,0xfdcbc6,23, 0,0,0
  };
  
struct SUB_CODE sMn63[] = {
  10,0xcb20,4, 9,0xcb26,15, 5,0xddcb26,23, 0,0,0
  };
  
struct SUB_CODE sMn64[] = {
  10,0xcb28,4, 9,0xcb2e,15, 5,0xddcb2e,23, 0,0,0
  };
  
struct SUB_CODE sMn65[] = {
  10,0xcb38,4, 9,0xcb3e,15, 5,0xddcb3e,23, 0,0,0
  };
  
struct SUB_CODE sMn66[] = {
  10,0x90,4, 4,0xd6,7, 9,0x96,7, 5,0xdd96,19, 0,0,0
  };
  
struct SUB_CODE sMn67[] = {
  10,0xa8,4, 4,0xee,7, 9,0xae,7, 5,0xddae,19, 0,0,0
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
  4,0x69,2, 5,0x65,3, 6,0x75,4, 7,0x6d,4, 8,0x7d,4, 9,0x79,4, 10,0x61,6, 11,0x71,5, 0,0,0
  };
  
struct SUB_CODE sMn2[] = {
  4,0x29,2, 5,0x25,3, 6,0x35,4, 7,0x2d,4, 8,0x3d,4, 9,0x39,4, 10,0x21,6, 11,0x31,5, 0,0,0
  };
  
struct SUB_CODE sMn3[] = {
  1,0xa,2, 5,0x6,5, 6,0x16,6, 7,0xe,6, 8,0x1e,7, 0,0,0
  };
  
struct SUB_CODE sMn4[] = {
  7,0x90,2, 0,0,0
  };
  
struct SUB_CODE sMn5[] = {
  7,0xb0,2, 0,0,0
  };
  
struct SUB_CODE sMn6[] = {
  7,0xf0,2, 0,0,0
  };
  
struct SUB_CODE sMn7[] = {
  5,0x24,3, 7,0x2c,4, 0,0,0
  };
  
struct SUB_CODE sMn8[] = {
  7,0x30,2, 0,0,0
  };
  
struct SUB_CODE sMn9[] = {
  7,0xd0,2, 0,0,0
  };
  
struct SUB_CODE sMn10[] = {
  7,0x10,2, 0,0,0
  };
  
struct SUB_CODE sMn11[] = {
  1,0x00,7, 0,0,0
  };
  
	struct SUB_CODE sMn12[] = {
		1,0x80,7, 0,0,0
		};
  
struct SUB_CODE sMn13[] = {
  7,0x50,2, 0,0,0
  };
  
struct SUB_CODE sMn14[] = {
  7,0x70,2, 0,0,0
  };
  
struct SUB_CODE sMn15[] = {
  1,0x18,2, 0,0,0
  };
  
struct SUB_CODE sMn16[] = {
  1,0xd8,2, 0,0,0
  };
  
struct SUB_CODE sMn17[] = {
  1,0x58,2, 0,0,0
  };
  
struct SUB_CODE sMn18[] = {
  1,0xb8,2, 0,0,0
  };
  
struct SUB_CODE sMn19[] = {
  4,0xc9,2, 5,0xc5,3, 6,0xd5,4, 7,0xcd,4, 8,0xdd,4, 9,0xd9,4, 10,0xc1,6, 11,0xd1,5, 0,0,0
  };
  
struct SUB_CODE sMn20[] = {
  4,0xe0,2, 5,0xe4,3, 7,0xec,4, 0,0,0
  };
  
struct SUB_CODE sMn21[] = {
  4,0xc0,2, 5,0xc4,3, 7,0xcc,4, 0,0,0
  };
  
struct SUB_CODE sMn22[] = {
  5,0xc6,5, 6,0xd6,6, 7,0xce,6, 8,0xde,7, 0,0,0
  };
  
struct SUB_CODE sMn23[] = {
  1,0xca,2, 0,0,0
  };
  
struct SUB_CODE sMn24[] = {
  1,0x88,2, 0,0,0
  };
  
struct SUB_CODE sMn25[] = {
  4,0x49,2, 5,0x45,3, 6,0x55,4, 7,0x4d,4, 8,0x5d,4, 9,0x59,4, 10,0x41,6, 11,0x51,5, 0,0,0
  };
  
struct SUB_CODE sMn26[] = {
  5,0xe6,5, 6,0xf6,6, 7,0xee,6, 8,0xfe,7, 0,0,0
  };
  
struct SUB_CODE sMn27[] = {
  1,0xe8,2, 0,0,0
  };
  
struct SUB_CODE sMn28[] = {
  1,0xc8,2, 0,0,0
  };
  
struct SUB_CODE sMn29[] = {
  7,0x4c,3, 12,0x6c,5, 0,0,0
  };
  
struct SUB_CODE sMn30[] = {
  7,0x20,6, 0,0,0
  };
  
struct SUB_CODE sMn31[] = {
  4,0xa9,2, 5,0xa5,3, 6,0xb5,4, 7,0xad,4, 8,0xbd,4, 9,0xb9,4, 10,0xa1,4, 11,0xb1,5, 0,0,0
  };
  
struct SUB_CODE sMn32[] = {
  4,0xa2,2, 5,0xa6,3, 16,0xb6,4, 7,0xae,4, 9,0xbe,4, 0,0,0
  };
  
struct SUB_CODE sMn33[] = {
  4,0xa0,2, 5,0xa4,3, 6,0xb4,4, 7,0xac,4, 8,0xbc,4, 0,0,0
  };
  
struct SUB_CODE sMn34[] = {
  2,0x4a,2, 5,0x46,5, 6,0x56,6, 7,0x4e,6, 8,0x5e,7, 0,0,0
  };
  
struct SUB_CODE sMn35[] = {
  1,0xea,2, 0,0,0
  };
  
struct SUB_CODE sMn36[] = {
  4,0x09,2, 5,0x05,3, 6,0x15,4, 7,0x0d,4, 8,0x1d,4, 9,0x19,4, 10,0x01,4, 11,0x11,5, 0,0,0
  };
  
struct SUB_CODE sMn37[] = {
  1,0x48,3, 0,0,0
  };
  
struct SUB_CODE sMn38[] = {
  1,0x8,3, 0,0,0
  };
  
	struct SUB_CODE sMn39[] = {
		1,0xda,3, 0,0,0
		};
  
	struct SUB_CODE sMn40[] = {
		1,0x5a,3, 0,0,0
		};

struct SUB_CODE sMn41[] = {
  1,0x68,3, 0,0,0
  };
  
struct SUB_CODE sMn42[] = {
  1,0x28,3, 0,0,0
  };
  
	struct SUB_CODE sMn43[] = {
		1,0xfa,6, 0,0,0
		};
  
	struct SUB_CODE sMn44[] = {
		1,0x7a,6, 0,0,0
		};
  
struct SUB_CODE sMn45[] = {
  2,0x2a,2, 5,0x26,5, 6,0x36,6, 7,0x2e,6, 8,0x3e,7, 0,0,0
  };
  
struct SUB_CODE sMn46[] = {
  2,0x6a,2, 5,0x66,5, 6,0x76,6, 7,0x6e,6, 8,0x7e,7, 0,0,0
  };
  
struct SUB_CODE sMn47[] = {
  1,0x40,6, 0,0,0
  };
  
struct SUB_CODE sMn48[] = {
  1,0x60,6, 0,0,0
  };
  
struct SUB_CODE sMn49[] = {
  4,0xe9,2, 5,0xe5,3, 6,0xf5,4, 7,0xed,4, 8,0xfd,4, 9,0xf9,4, 10,0xe1,6, 11,0xf1,5, 0,0,0
  };
  
struct SUB_CODE sMn50[] = {
  1,0x38,2, 0,0,0
  };
  
struct SUB_CODE sMn51[] = {
  1,0xf8,2, 0,0,0
  };
  
struct SUB_CODE sMn52[] = {
  1,0x78,2, 0,0,0
  };
  
struct SUB_CODE sMn53[] = {
  5,0x85,3, 6,0x95,4, 7,0x8d,4, 8,0x9d,5, 9,0x99,5, 10,0x81,6, 11,0x91,6, 0,0,0
  };
  
struct SUB_CODE sMn54[] = {
  5,0x86,3, 16,0x96,4, 7,0x8e,4, 0,0,0
  };
  
struct SUB_CODE sMn55[] = {
  5,0x84,3, 6,0x94,4, 7,0x8c,4, 0,0,0
  };
  
	struct SUB_CODE sMn56[] = {
		5,0x64,3, 6,0x74,4, 7,0x9c,4, 8,0x9e,5, 0,0,0
		};
  
struct SUB_CODE sMn57[] = {
  1,0xaa,3, 0,0,0
  };
  
struct SUB_CODE sMn58[] = {
  1,0xa8,3, 0,0,0
  };
  
	struct SUB_CODE sMn59[] = {
		5,0x14,5, 7,0x1c,6, 0,0,0
		};
  
	struct SUB_CODE sMn60[] = {
		5,0x4,5, 7,0xc,6, 0,0,0
		};
  
struct SUB_CODE sMn61[] = {
  1,0xba,3, 0,0,0
  };
  
struct SUB_CODE sMn62[] = {
  1,0x8a,3, 0,0,0
  };
  
struct SUB_CODE sMn63[] = {
  1,0x9a,3, 0,0,0
  };
  
struct SUB_CODE sMn64[] = {
  1,0x98,3, 0,0,0
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
   14,0x11,0,0
  };
  
struct SUB_CODE sMn2[] = {
   4,0x24,21,0x25,9,0x26,2,0x28,0,0
  };
  
struct SUB_CODE sMn3[] = {
   4,0x34,21,0x35,9,0x36,2,0x38,0,0
  };
  
struct SUB_CODE sMn4[] = {
   14,1,0,0
  };
  
struct SUB_CODE sMn5[] = {
  4,0xcd,14,0xcd,40,0xc4,0,0
  };
  
struct SUB_CODE sMn6[] = {
  1,0x3f,0,0
  };
  
struct SUB_CODE sMn7[] = {
  10,0xb8,4,0xfe,5,0xddbe,9,0xbe,0,0
  };
  
struct SUB_CODE sMn8[] = {
  1,0xeda9,0,0
  };
  
struct SUB_CODE sMn9[] = {
  1,0xedb9,0,0
  };
  
struct SUB_CODE sMn10[] = {
   4,0x14,21,0x15,9,0x16,10,0x18,0,0
  };
  
struct SUB_CODE sMn11[] = {
  1,0xedb1,0,0
  };
  
struct SUB_CODE sMn12[] = {
  1,0x2f,0,0
  };
  
struct SUB_CODE sMn13[] = {
   4,0x4,21,0x5,9,0x6,10,0x8,0,0
  };
  
struct SUB_CODE sMn14[] = {
   40,0x20,0,0
  };
  
struct SUB_CODE sMn15[] = {
   40,0x10,0,0
  };
  
struct SUB_CODE sMn16[] = {
  40,0x40,0,0
  };
  
struct SUB_CODE sMn17[] = {
  1,0xfb,0,0
  };
  
struct SUB_CODE sMn18[] = {
  40,0x30,0,0
  };
  
struct SUB_CODE sMn19[] = {
  40,0x50,0,0
  };
  
struct SUB_CODE sMn20[] = {
   40,0x70,0,0
  };
  
struct SUB_CODE sMn21[] = {
   40,0x60,0,0
  };
  
struct SUB_CODE sMn22[] = {
   14,1,0,0
  };
  
struct SUB_CODE sMn23[] = {
   14,2,0,0
  };
  
struct SUB_CODE sMn24[] = {
   11,0x78,12,0xa8,4,0x74,21,0xe5,2,0xe8,0,0
  };
  
struct SUB_CODE sMn25[] = {
  1,0xedba,0,0
  };
  
struct SUB_CODE sMn26[] = {
  1,0xeda2,0,0
  };
  
struct SUB_CODE sMn27[] = {
  1,0xedb2,0,0
  };
  
struct SUB_CODE sMn28[] = {
   1,0,0,0
  };
  
struct SUB_CODE sMn29[] = {
  4,0x18,14,0x18,40,0x20,0,0
  };
  
struct SUB_CODE sMn30[] = {
   21,0xd0,0,0
  };
  
struct SUB_CODE sMn31[] = {
   31,0xc0,0,0
  };
  
struct SUB_CODE sMn32[] = {
   1,0x22,0,0
  };
  
struct SUB_CODE sMn33[] = {
   1,0x32,0,0
  };
  
struct SUB_CODE sMn34[] = {
  1,0xedb0,0,0
  };
  
struct SUB_CODE sMn35[] = {
  1,0xed44,0,0
  };
  
struct SUB_CODE sMn36[] = {
  1,0x0,0,0
  };
  
struct SUB_CODE sMn37[] = {
  10,0xb0,4,0xf6,5,0xddb6,9,0xb6,0,0
  };
  
struct SUB_CODE sMn38[] = {
  1,0xedbb,0,0
  };
  
struct SUB_CODE sMn39[] = {
  1,0xedb3,0,0
  };
  
struct SUB_CODE sMn40[] = {
  28,0xed41,17,0xd3,0,0
  };
  
struct SUB_CODE sMn41[] = {
  1,0xedab,0,0
  };
  
struct SUB_CODE sMn42[] = {
  1,0xeda3,0,0
  };
  
struct SUB_CODE sMn43[] = {
  3,0xc1,15,0xdde1,16,0xfde1,0,0
  };
  
struct SUB_CODE sMn44[] = {
  3,0xc5,15,0xdde5,16,0xfde5,0,0
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
  2,0x5f,4, 3,0x5f,4, 4,0x5f,4, 5,0x47,4, 6,0x4f,4, 0,0,0
  };
  
struct SUB_CODE sMn2[] = {
  4,0x57,4, 0,0,0
  };
  
struct SUB_CODE sMn3[] = {
  2,0xbf,4, 3,0xbf,4, 5,0xa7,4, 6,0xaf,4, 0,0,0
  };
  
struct SUB_CODE sMn4[] = {
  4,0xb7,4, 0,0,0
  };
  
struct SUB_CODE sMn5[] = {
  4,0x1,4, 14,0x1,4, 0,0,0
  };
  
struct SUB_CODE sMn6[] = {
  2,0xd,4, 3,0xd,4, 1,0xdf,4, 4,0xd,4, 0,0,0
  };
  
struct SUB_CODE sMn7[] = {
  1,0x2d,4, 0,0,0
  };
  
struct SUB_CODE sMn8[] = {
  2,0x3f,4, 3,0x3f,4, 4,0x3f,4, 5,0x27,4, 6,0x2f,4, 0,0,0
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
  4,0x9,4, 14,0x9,4, 0,0,0
  };
  
struct SUB_CODE sMn13[] = {
  4,0x6,2, 14,0x6,2, 41,0x6,2, 0,0,0
  };
  
struct SUB_CODE sMn14[] = {
  4,0x2,2, 14,0x2,2, 41,0x2,2, 0,0,0
  };
  
struct SUB_CODE sMn15[] = {
  4,0x0,2, 14,0x0,2, 41,0x0,2, 0,0,0
  };
  
struct SUB_CODE sMn16[] = {
  40,0x3,5, 0,0,0
  };
  
struct SUB_CODE sMn17[] = {
  40,0x13,5, 0,0,0
  };
  
struct SUB_CODE sMn18[] = {
  4,0x4,2, 14,0x4,2, 41,0x4,2, 0,0,0
  };
  
struct SUB_CODE sMn19[] = {
  2,0x35,4, 3,0x1f,4, 
  4,0x1f,4, /* metto il modo 10 come il 3, perche' l'operando e' uguale (cambia opcode) sia in indiretto che in immediato)*/
  10,0x3d,4, 11,0x3d,4, 
  5,0x7,4, 6,0xf,4, 28,0x9f,4, 17,0x9f,4, 24,0x87,4, 25,0x8f,4, 0,0,0
  };
  
struct SUB_CODE sMn20[] = {
  4,0x17,4, 3,0xd,4 /* v. ld */, 11,0xd,4, 0,0,0
  };
  
struct SUB_CODE sMn21[] = {
  1,0x4,2, 0,0,0
  };
  
struct SUB_CODE sMn22[] = {
  50,0xb,4, 0,0,0
  };
  
struct SUB_CODE sMn23[] = {
  1,0xcd,2, 0,0,0
  };
  
struct SUB_CODE sMn24[] = {
  1,0x4d,2, 0,0,0
  };
  
struct SUB_CODE sMn25[] = {
  1,0xad,4, 0,0,0
  };
  
struct SUB_CODE sMn26[] = {
  50,0x1b,4, 0,0,0
  };
  
struct SUB_CODE sMn27[] = {
  1,0x5f,4, 0,0,0
  };
  
struct SUB_CODE sMn28[] = {
  1,0x6d,2, 0,0,0
  };
  
struct SUB_CODE sMn29[] = {
  2,0xdf,4, 3,0xdf,4, 5,0xc7,4, 6,0xcf,4, 0,0,0
  };
  
struct SUB_CODE sMn30[] = {
  4,0xd7,4, 0,0,0
  };
  
struct SUB_CODE sMn31[] = {
  1,0xed,2, 0,0,0
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
#endif
  
struct MY_CODE MCom[] = {
  "LABEL",0x0,
  "DB",1,"DEFB",1,"BYTE",1,
  "DW",2,"DEFW",2,"WORD",2,
  "DD",4,"DEFL",4,
  "DQ",8,
  "DS",0x10,"DEFS",0x10,      // Define Space(n)
  "STRING",0x11,"ASCIIZ",0x11,  // 
  "ASCII",0x12,"DEFM",0x12,   // 
  "FILL",0x13,   // 
  "ORG",0x20,
  "ALIGN",0x21,
#if ST62  
  "BLOCK",0x22,   // blocca (riserva, salta) un numero di byte e quindi allinea
  								// usato in generale per lo ST62 come: .block  64-$%64
#endif
  "EQU",0x100,
#if ST62  
  "DEF",0x100,
#endif
  "=",0x100,
//  "COUNT",0x101,
  "ENDC",0x102,
  "CSEG",0x1000,"TEXT",0x1000,"PSECT",0x1000,
  "DSEG",0x1001,
  "CSECT",0x1002,
  "DSECT",0x1003,
  "CONST",0x1004,"RODATA",0x1004,
  "BSS",0x1005,"ZDATA",0x1005,		//			fare		https://www.rowleydownload.co.uk/msp430/documentation/index.htm?https://www.rowleydownload.co.uk/msp430/documentation/has_directive_keep.htm
  "ISR",0x1006,		// fare...
  "PROC",0x1010,"ENTRY",0x1010,
  "ENDP",0x1011,
  "END",0x1012,
  "PUBLIC",0x1020,"EXPORT",0x1020,
  "EXTRN",0x1021,
  "NAME",0x1030,
  "TITLE",0x2000,
  "PAGE",0x2001,
  "LINE",0x2002,
  "%",0x2003,
  "EJECT",0x2004,
#if ST62
  "DISPLAY",0x2005,
#endif
#if ST62
  "VERS",0x2010,      // il tipo di uP: 62E10, 62E20
  "W_ON",0x2011,      // modo memoria a finestre
#endif
  "MACRO",0x4000,
  "EXITM",0x4001,
  "ENDM",0x4002,
  "STRUC",0x4002,
  "ENDS",0x4003,
  "INCLUDE",0x8000, "INPUT",0x8000,
  "IF",0x8010,"IFE",0x8011,"IFDEF",0x8012,"IFNDEF",0x8013,
  "ELSE",0x8020,
  "ENDIF",0x8022,
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
#elif _6502
struct COND Cond[]= {
  "EQ",0xf0,
  "NE",0xd0,
  "VC",0x50,
  "VS",0x70,
  "CC",0x90,
  "CS",0xb0,
  "PL",0x10,
  "MI",0x30,
  NULL,0
  };
#endif  


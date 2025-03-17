#include <stdio.h>
#include <stdint.h>

#define ST62 0
#define Z80  0
#define _6502 0
#define I8051 0
#define PIC 0 //16		// 18, 24, 30, 32, 33
#define ARCHI 0
#define GD24032 0
#define I8086 1
       
#define TRUE 1
#define FALSE 0

#if ARCHI
#define __VER__ 0
#define INT_SIZE 4
#define PTR_SIZE 4
#elif Z80
#define __VER__ ((1 << 8) | 33)
#define INT_SIZE 1
#define PTR_SIZE 2
#elif I8051
#define __VER__ ((1 << 8) | 0)
#define INT_SIZE 1
#define PTR_SIZE 1
#elif ST62
#define __VER__ ((1 << 8) | 0)
#define INT_SIZE 1
#define PTR_SIZE 1
#elif _6502
#define __VER__ ((1 << 8) | 4)
#define INT_SIZE 1
#define PTR_SIZE 1
#elif PIC
#define __VER__ ((1 << 8) | 1)
#define INT_SIZE 1
#if PIC==16
#define PTR_SIZE 1
#elif PIC==18
#define PTR_SIZE 2
#elif PIC==24
#define PTR_SIZE 3
#elif PIC==32
#define PTR_SIZE 4
#endif
#elif GD24032
#define __VER__ ((1 << 8) | 6)
#define INT_SIZE 4
#define PTR_SIZE 4
#elif I8086
#define __VER__ ((0 << 8) | 90)
#define INT_SIZE 2		// 4??
#define PTR_SIZE 2		// 4 idem gestire CPULevel
#endif

#define MAX_NAME_LEN 63

struct ERRORE {
  int16_t t;
  const char *s;
  int16_t liv;
  };

struct SUB_CODE {
  int16_t m;                   // tipo indirizzamento
  uint32_t  n;                 // op-code (per alcune CPU basterebbe 8 o 16bit...
  uint16_t  c;                 // numcicli; se condiz: byte basso false, byte alto true
  };
    
struct CODE {
  const char *s;               // mnemonico
  const struct SUB_CODE *a;          // struttura, con op-code 
  };
        
extern struct CODE Mnem[];
  
struct MY_CODE {
  const char *s;
  uint16_t m;
  };
        
extern struct MY_CODE MCom[];
  
struct COND {
  const char *s;
  uint16_t m;
  };
           
extern struct COND Cond[];
  
struct REGS {
  const char *s;
  uint16_t m;
  uint32_t attr;			// per ASSUME ecc di MASM, ma lascio cmq su tutti
  };
           
extern struct REGS Reg[];
  
#if GD24032
typedef int64_t NUM_TYPE;
#else
typedef int32_t NUM_TYPE;
#endif

// VARSIZE% DIMENSIONI (PER LE FUNZIONI E' LA DIM. DI QUELLO CHE sono)
enum VAR_TYPES {
// VARTYPE% BITS:
// 0= label, 7=public defined, 8=FUNZ., 9=public, 10=extern, 11=STRUCT
	VAR_LABEL=1,
	VAR_PUBLIC_DEFINED=0x80,
	VAR_FUNZ=0x100,
	VAR_PUBLIC=0x200,
	VAR_EXTERN=0x400,
	VAR_STRUCT=0x800,
	VAR_UNION=0x1000,
	VAR_RECORD=0x2000
	};

struct VARS {
  char label[MAX_NAME_LEN+1];
  struct VARS *proc;
  uint16_t size;
  uint16_t type;	// 1=equ, 0x80=label(?), 0x100=proc, 0x200=public, 0x400=extern, 0x1000=struct, 0x2000=record
	// 200 e 80 sempre insieme pare... 2024? 
  uint32_t tag;
  NUM_TYPE value;
  int16_t  segm;
  int16_t  dim;
  int32_t  lpos;                  //per extern in file obj
  int16_t  extUsed;
//  struct TAGS *parent;         // se <>0, la var. è un membro della struct parent
  struct VARS *next;
  struct VARS *left, *right;
  };

struct TAGS {
  char label[MAX_NAME_LEN+1];
  uint16_t size;
  uint16_t type;	// v. sopra, solo alcuni casi ovviamente
  uint16_t pos;		// l'offset nella struct o aggr
  NUM_TYPE value;
  uint32_t tag;
  int16_t  dim;
//  struct TAGS *parent;         // se <>0, la var. è un membro della struct parent  no USO TAG
  struct TAGS *next;
  struct TAGS *left, *right;
  };  
   
struct SEGS {
  char label[MAX_NAME_LEN+1];
  uint16_t type;
  uint8_t  align;
  uint8_t  unused;
  uint32_t BaseAddr;			// 0xffffffff se non impostato
  uint32_t CurAddr;
  uint32_t len;
  uint32_t lpos;			// pos code nel file oggetto
  uint32_t lposName;	// pos name/label nel file oggetto
  uint16_t numReloc;
  uint32_t lposReloc;			// pos reloc nel file oggetto
  uint16_t numPub;
  uint32_t lposPub;			// pos public nel file oggetto
  uint16_t numExt;
  uint32_t lposExt;			// pos extern nel file oggetto
	};
  
#define MAX_SEGS 32

struct OPERANDO {
  const char *s;
  signed char p;
  };

extern struct OPERANDO Operandi[];

#pragma pack( push, before_structs )
#pragma pack(1)
struct OBJ_STRUCT {
  int16_t ident;
  int16_t version;
  char spare[28];
  long relocBase;              // dove trovo gli indirizzi reloc
  long segBase;              // dove trovo i segmenti
  char reserved[24];
  };
#pragma pack( pop, before_structs )

enum SEGMENT_TYPES {
//#pragma warning nel byte basso mettere cose come R W X come elf!	4 2 1
	SEGMENT_CODE=0 | (1|4),
	SEGMENT_DATA=0x100 | (2|4),
	SEGMENT_BSS=0x300 | (2|4),
	SEGMENT_CONST=0x500 | (4),
#if PIC==16
	SEGMENT_CONFIG=0x2100 | (4),
	SEGMENT_IDLOCS=0x2100 | (4),
#elif PIC==18
	SEGMENT_CONFIG=0x2100 | (4),
	SEGMENT_IDLOCS=0x2200 | (4),
#endif
	SEGMENT_ABS=0x4100,
	SEGMENT_VIRT=0x8000,

	SEG_UNDEFINED=-1,
	SEG_EXTERN=0x00ff,
	SEG_ABS=0,
	SEG_NONE=0
	};

enum ARITM_ELEM {
	ARITM_UNKNOWN=-1,
	ARITM_NONE=0,
	ARITM_EOL=ARITM_NONE,
	ARITM_CONST=1,
	ARITM_VAR=2,
	ARITM_OPER=3,
	};

enum VQ_ELEM {
	VQ_CONDIZ=-2,
	VQ_REGISTER=-1,
	VQ_OP_VAR=0,
	VQ_VAR=1,
	VQ_COMMA=2,
#if _6502
	VQ_VAR_IMM=3,		// solo _6502.. ma c'è anche in PIC16, verificare cfr PIC18
#endif
	};

enum OUT_VALUES {
	OUT_END=-9,
	OUT_REPOS=-5,
	OUT_VAR_EXT=-4,
	OUT_SEGM=-3,
	OUT_RELOC=-2,
	OUT_START=-1,
	OUT_VALUE=0
	};

enum ADDR_MODE {
	ADDR_IMPL=1,
	ADDR_REG8=2,
	ADDR_IMM8=4,
	ADDR_ABS=7,
#if Z80  
	ADDR_IMM16=3,
	ADDR_IND8=17,
	ADDR_IND16=18,

	// sistemare!!! 2024
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
	ADDR_2=2,// 2
	ADDR_IX_IY=5,// 5	 
	ADDR_IY=6,// 6	 
	ADDR_7=7,// 7
	ADDR_8=8,// 8
	ADDR_9=9,// 9	ld a,(bc), jp (hl)
	ADDR_10=10,// 10 ld a,b			or b
	ADDR_11=11,// 11 ld  , 20  v. ADDR_IMM8
	ADDR_12=12,// 12 jp (ix)
	ADDR_14=14,// 14 jp abs
	ADDR_15=15,// 15 inc ix
	ADDR_16=16,// 16 inc iy
	ADDR_17=17,// 
	ADDR_18=18,// 
	ADDR_19=19,// 19
	ADDR_20=20,// 20
	ADDR_21=21,// 21		ld a,()
	ADDR_22=22,// 22   ld r16,(nn)
	ADDR_23=23,// 23   ld r16,nn
	ADDR_24=24,// 24
	ADDR_25=25,// 25
	ADDR_26=26,// 26	ld (ix+n),n
	ADDR_27=27,// 27	ld (iy+n),n
	ADDR_28=28,// 28
	ADDR_29=29,// 29
	ADDR_30=30,// 30  ( boh ind x+ e y+
	ADDR_31=31,// 31  sp
	ADDR_32=32,// 32
	ADDR_33=33,// 33	ind x+ 
	ADDR_34=34,// 34	ind y+
	ADDR_CONDIZ_IMM=40,
	ADDR_CONDIZ=41,
	ADDR_BIT=50,// 50		bit ecc
	ADDR_BIT2=51,// 51		bit ecc
	ADDR_BITX=52,// 52		bit ecc
	ADDR_BITY=53,// 53		bit ecc
	ADDR_60=60,// 60		
	ADDR_61=61,// 61		
	ADDR_62=62,
	ADDR_63=63,
#elif _6502
	ADDR_ZP=5,
	ADDR_ZPX=6,
	ADDR_ABSX=8,
	ADDR_ABSY=9,
	ADDR_10=10,//
	ADDR_11=11,//
	ADDR_12=12,// 12 
	ADDR_ZPY=16,
	ADDR_19=19,// 19
	ADDR_21=21,// 21	
	ADDR_28=28,// 28
#elif I8051
	ADDR_IMM16=3,
	ADDR_5=5,// 5	 
	ADDR_9=9,// 9	ld a,(bc), jp (hl)
	ADDR_10=10,// 10 ld a,b			or b
	ADDR_12=12,// 12 jp (ix)
	ADDR_14=14,// 14 jp abs
	ADDR_18=18,// 18
	ADDR_19=19,// 19
	ADDR_20=20,// 20
	ADDR_21=21,// 21		ld a,()
	ADDR_22=22,// 22   ld r16,(nn)
	ADDR_23=23,// 23   ld r16,nn
	ADDR_28=28,// 28
	ADDR_32=32,// 32
	ADDR_CONDIZ=41,
	ADDR_BIT=50,// 50		bit ecc
//	ADDR_51=51,// 51		bit ecc
	ADDR_60=60,// 60		
	ADDR_61=61,// 61		
	ADDR_62=62,
	ADDR_63=63,
#elif ST62
	ADDR_IMM16=3,
	ADDR_CONDIZ=41,
#elif PIC==16
	ADDR_8=5,
#elif PIC==18
	ADDR_8=5,
	ADDR_IMM16=10,
	ADDR_ABS2=11,
#elif GD24032
	ADDR_REG16=10,
	ADDR_REG32=11,
	ADDR_IMM17=15,
	ADDR_IMM32=16,
	ADDR_PREINC,
	ADDR_PREDEC,
	ADDR_POSTINC,
	ADDR_POSTDEC,
	ADDR_ABS64,
	ADDR_ALL,
	ADDR_ALL_2OP,
	ADDR_ALL_3OP,
	ADDR_ALL_4OP,
	ADDR_CONDIZ_IMM=40,
	ADDR_CONDIZ=41,

	// togliere!
	ADDR_5=5,// 5	 
	ADDR_7=7,// 9	ld a,(bc), jp (hl)
	ADDR_9=9,// 9	ld a,(bc), jp (hl)
	ADDR_10=10,// 10 ld a,b			or b
	ADDR_11=11,// 10 ld a,b			or b
	ADDR_12=12,// 12 jp (ix)
	ADDR_14=14,// 14 jp abs
	ADDR_18=18,// 18
	ADDR_19=19,// 19
	ADDR_20=20,// 20
	ADDR_21=21,// 21		ld a,()
	ADDR_22=22,// 22   ld r16,(nn)
	ADDR_23=23,// 23   ld r16,nn
	ADDR_28=28,// 28
	ADDR_32=32,// 32
	ADDR_50=50,// 51		bit ecc
	ADDR_51=51,// 51		bit ecc
	ADDR_52=52,//
	ADDR_60=60,// 60		
	ADDR_61=61,// 61		
	ADDR_62=62,
	ADDR_63=63,
	ADDR_17=17,// 
	ADDR_24=24,// 24
	ADDR_26=26,// 26
	ADDR_30=30,// 30  ind x+ e y+
	ADDR_31=31,// 31  sp
	ADDR_33=33,// 33
#elif I8086
	ADDR_REG16=3,		// questo, come REG8, qua lo usiamo quando solo AL/AX è accettato  TEST AL,8
	ADDR_REG8IMM=10,	// MOV AL,1
	ADDR_REG16IMM=11,	// MOV AX,1000
	ADDR_IMM16=5,			// JMP, CALL
	ADDR_IND16_REG8=18,		// MOV (100),AL
	ADDR_IND16_REG16=19,		// MOV (100),AX
	ADDR_IND16REG_REG8=20,		// MOV (BP),AL
	ADDR_IND16REG_REG16=21,		// MOV (BP),AX
	ADDR_REG8_IND16=22,		// MOV AL,(100)
	ADDR_REG16_IND16=23,		// MOV AX,(100)
	ADDR_REG8_IND16REG=24,		// MOV AL,(BP)
	ADDR_REG16_IND16REG=25,		// MOV AX,(BP)
	ADDR_IND16REG_IMM8=26,		// CMP (100),AL
	ADDR_IND16REG_IMM16=27,		// CMP (100),AX
	ADDR_IND16REG_IMM16_8=28, 		// CMP WORD PTR (100),AL
	ADDR_IMM32=30,		// JMP FAR, CALL FAR

	// !!! 2025
	ADDR_2=2,// 2
	ADDR_5=5,// 5	 
	ADDR_6=6,// 6	 
	ADDR_7=7,// 7
	ADDR_8=8,// 8
	ADDR_9=9,// 9	ld a,(bc), jp (hl)
	ADDR_10=10,// 10 ld a,b			or b
	ADDR_11=11,// 11 ld  , 20  v. ADDR_IMM8
	ADDR_IMPL16=12,// 12 jp (ix)
	ADDR_15=15,// 15 inc ix
	ADDR_16=16,// 16 inc iy
	ADDR_17=17,// 
	ADDR_18=18,// 
	ADDR_19=19,// 19
	ADDR_20=20,// 20
	ADDR_21=21,// 21		ld a,()
	ADDR_22=22,// 22   ld r16,(nn)
	ADDR_23=23,// 23   ld r16,nn
	ADDR_24=24,// 24
	ADDR_25=25,// 25
	ADDR_26=26,// 26	ld (ix+n),n
	ADDR_27=27,// 27	ld (iy+n),n
	ADDR_28=28,// 28
	ADDR_29=29,// 29
	ADDR_REG8REG=30,			// per Shift
	ADDR_REG16REG=31,
	ADDR_31=31,// 31  sp
	ADDR_32=32,// 32
	ADDR_33=33,// 33	ind x+ 
	ADDR_34=34,// 34	ind y+
	ADDR_CONDIZ_IMM=40,
	ADDR_CONDIZ=41,
	ADDR_50=50,// 51		bit ecc
	ADDR_51=51,// 51		bit ecc
	ADDR_52=52,// 52		bit ecc
	ADDR_53=53,// 53
	ADDR_60=60,// 60		
	ADDR_61=61,// 61		
	ADDR_62=62,
	ADDR_63=63,
#endif
	};

struct LINE_DEF {
	uint16_t used;
  struct LINE_DEF *next;
  struct LINE_DEF *prev;
  char s[1];
  };
  
struct SEGMENTED_VALUE {
	NUM_TYPE v;
	int16_t seg;
	bool valid;
	};

struct OPERAND {
	struct SEGMENTED_VALUE l;
//	long l;
	uint16_t S;			// size
	uint8_t Sptr;		// size del puntatore, se c'è
// verificare...	enum ARITM_ELEM T;
	int T;
	struct VARS *var;
//	enum VQ_ELEM V;
	int8_t V;
	int8_t op;
	int8_t Ind;
#if GD24032
	int8_t preOp,postOp;
	int8_t addr64;
	int8_t reg2reg;
#endif
#if I8086
	int8_t reg2reg;
#endif
	};

extern signed char UNDEFD,IfDefs;   
extern int Segs,SX;
extern int LABEL;
extern uint8_t Brack;
extern struct SEGS Seg[MAX_SEGS];
extern struct VARS *Var, *LVars;
extern struct TAGS *StrTag, *LTag;
extern signed char debug;
extern char __file__[256];
extern char __title__[256];
extern int __line__,__page__;
extern char __date__[10];
extern char __time__[10];
extern signed char IgnoreCase;            // LABEL tutte uppercase -M
extern FILE *FCod,*FMap,*FPre,*FIn,*FObj;
extern signed char OutVars;               // INSERISCE NELL'OUTPUT LE VARIABILI E LE LINEE -n
extern signed char OutList;               // CREA FILE LISTING source         -l, -Fc
extern signed char OutCycle;               // riporta i cicli macchina per istr.
extern signed char OutError;  		       // scrivi sorgente su errore
extern signed char NoMacro;              // DISABILITA MACRO PREDEF.    -u
extern int16_t CurSeg;
extern char theLine[1024];
extern int theLinePtr;
extern uint8_t CPass,bInizioRiga,VarDeclared;
extern uint16_t PageLength,LineLength;
extern struct LINE_DEF *RootDef,*LastDef;
extern struct LINE_DEF *RootMacro,*LastMacro;
    
int FNLeggiFile(char *, FILE *,uint8_t);
char *FNParse(const char *,int *,char *);
char *FNGrab(FILE *, char *);
char *FNPreProcess(FILE *FI,const char *,char *);
char *FNTrasfNome(char *);    
char *AddExt(char *, const char *);
char *FNLA(char *);
char *FNObbLO(char *);
char *FNLO(char *);
long EVAL(char *);
uint8_t FNGetEscape(char);
uint8_t FNGetEscape();
NUM_TYPE FNGetOct(const char *);
NUM_TYPE FNGetBin(const char *);
uint64_t strtoull(const char *nptr, char **endptr, int8_t);
int lltoa(uint64_t num, char *str, /*int len, */ int base);
int subBytes(const char *s, int m, uint64_t l);
int FNIsStmt(const char *);   
void gotoEOL();
char *FNGetLabel(char *,int8_t);
char *FNGetConst(char *);
long FNGetLine(long, char *);
enum ARITM_ELEM FNGetAritElem(char *, struct OPERAND *, uint8_t);
struct SEGMENTED_VALUE FNEvalExpr(uint8_t, void *);
int FNRev(uint8_t, struct OPERAND *o, void *);
int FNGetSize(struct SEGMENTED_VALUE);
long FNGetASCII(const char *);
long FNIsType(const char *);
int FNIsOp(const char *,uint8_t);
int FNIsDirective(const char *);
int FNIsReg(const char *);
int FNIsCond(const char *);
struct VARS *FNCercaVar(const char *, uint32_t, uint8_t, uint8_t);
struct TAGS *FNCercaAggr(const char *, uint8_t, uint32_t);
int FNCercaSeg(const char *);
int PROCWarn(int, const char *s=NULL);
int PROCError(int, const char *s=NULL);
int PROCError(int, int32_t);
int PROCOut(int A, int A1=-1, int A2=-1, int16_t segm=0);
// non si può fare.. int PROCOut(int A, int A1=-1, struct SEGMENTED_VALUE *A2=NULL);
#if PIC==16 || PIC==18
int PROCOut2(uint16_t, int A3=-1, int16_t A4=0);
#endif
#if GD24032
int PROCOut2(uint32_t, struct SEGMENTED_VALUE *A2=NULL);
int PROCOut2(struct SEGMENTED_VALUE A);
#endif
int PROCOut1(const char *, const char *s=NULL, signed char f=FALSE,bool addline=TRUE);
int subOut(int32_t t,int32_t t1=0);
int PROCCod(struct LINE *,const char *);
int PROCOper(char *, char *, char *, char *, char *);
struct VARS *PROCDclVar(char *, struct SEGMENTED_VALUE, uint16_t, uint32_t,const char *segm=NULL);
struct TAGS *PROCDclAggr(char *, uint16_t, uint32_t, NUM_TYPE);
struct TAGS *FNAllocAggr();
NUM_TYPE FNUpdAggr(struct TAGS *);
NUM_TYPE getMask(uint8_t n);
NUM_TYPE FNInitAggr(struct TAGS *,bool);
int PROCDclSeg(const char *,enum SEGMENT_TYPES);
int PROCAssign(struct LINE *, int, long, int);
//int PROCGetType(int *, long *, struct TAGS **, int *, long);
int PROCObj();
int PROCS(const char *);
int PROCV(FILE *);
int PROCV2(FILE *);
int PROCIsDecl();
int PROCCheck(const char *);
int PROCCheck(char);
struct VARS *PutOnTree(struct VARS *,struct VARS *,struct VARS *);
void Libera(struct VARS *root);
struct TAGS *PutOnTree2(struct TAGS *,struct TAGS *,struct TAGS *);
void Libera2(struct TAGS *root);

struct LINE_DEF *FNDefined(const char *A);
char *FNGetNextPre(FILE *FI, int F, char *s);
struct LINE_DEF *PROCDelLista(struct LINE_DEF *Root, struct LINE_DEF *Last, struct LINE_DEF *l);
int PROCDefine(const char *A, const char *B);
int PROCDefineMacro(const char *A, const char *B);
struct LINE_DEF *FNMacro(const char *A);
char *TranslateMacro(const struct LINE_DEF *macro,const char *smacro,char *text,const char *sparm);

uint8_t asc2hex(unsigned short int t);
uint8_t *convertHEX2BIN(FILE *,uint32_t *);
int8_t isasym(char ch);

#define MAKEWORD(a, b)   ((uint16_t) (((uint8_t) (a)) | ((uint16_t) ((uint8_t) (b))) << 8)) 
#define MAKELONG(a, b)   ((uint32_t) (((uint16_t) (a)) | ((uint32_t) ((uint16_t) (b))) << 16)) 
#define HIBYTE(w)   ((uint8_t) ((((uint16_t) (w)) >> 8) /* & 0xFF*/)) 
//#define HIBYTE(w)   ((BYTE) (*((char *)&w+1)))		// molto meglio :)
#define HIWORD(l)   ((uint16_t) (((uint32_t) (l) >> 16) & 0xFFFF)) 
#define LOBYTE(w)   ((uint8_t) (w)) 
#define LOWORD(l)   ((uint16_t) (l)) 
#define MAKEDLONG(a, b)   ((uint64_t) (((uint32_t) (a)) | ((uint64_t) ((uint32_t) (b))) << 32)) 
#define HIDWORD(l)   ((uint32_t) (((uint64_t) (l) >> 32) & 0xFFFFFFFF)) 
#define LODWORD(l)   ((uint32_t) (l)) 

#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

const char *stristr(const char *s1,const char *s2);
char* replace_char(char* str, char find, char replace);


#pragma pack( push, before_structs )
#pragma pack(1)

// per Lattice O bject
struct OBJ_HEADER {
  uint8_t v0;
  uint8_t v1;
  uint8_t v2;
  uint8_t numSegs;
  uint32_t res1;
  uint32_t offNomi;
  uint32_t lunNomi;
  };

struct OBJ_SEG_DEF {
  uint16_t tipo;               // (0 norm, 0x4000 abs value) v. SEGMENT
  uint16_t offNome;
  uint32_t offCode;
  uint32_t lunCode;
  uint32_t offReloc;
  uint32_t lunReloc;
  uint32_t offPublic;
  uint32_t offExtern;
  uint16_t numPublic;
  uint16_t numExtern;
  };

struct OBJ_RELOC {
  uint16_t type;				// 
  uint16_t reserved;		// 2 fisso pare
  uint32_t address;
  };


// per ELF
struct ELF_HEADER {
	uint8_t e_ident[16/*EI_NIDENT*/];
	uint16_t /*Elf32_Half*/ e_type;
	uint16_t /*Elf32_Half*/ e_machine;
	uint32_t /*Elf32_Word*/ e_version;
	uint32_t /*Elf32_Addr*/ e_entry;
	uint32_t /*Elf32_Off*/ e_phoff;
	uint32_t /*Elf32_Off*/ e_shoff;
	uint32_t /*Elf32_Word*/ e_flags;
	uint16_t /*Elf32_Half*/ e_ehsize;
	uint16_t /*Elf32_Half*/ e_phentsize;
	uint16_t /*Elf32_Half*/ e_phnum;
	uint16_t /*Elf32_Half*/ e_shentsize;
	uint16_t /*Elf32_Half*/ e_shnum;
	uint16_t /*Elf32_Half*/ e_shstrndx;
	//52
	};

struct ELF_SECTION_HEADER {
	uint32_t /*Elf32_Word*/ sh_name;
	uint32_t /*Elf32_Word*/ sh_type;
	uint32_t /*Elf32_Word*/ sh_flags;
	uint32_t /*Elf32_Addr*/ sh_addr;
	uint32_t /*Elf32_Off*/  sh_offset;
	uint32_t /*Elf32_Word*/ sh_size;
	uint32_t /*Elf32_Word*/ sh_link;
	uint32_t /*Elf32_Word*/ sh_info;
	uint32_t /*Elf32_Word*/ sh_addralign ;
	uint32_t /*Elf32_Word*/ sh_entsize ;
	//40
	};

struct ELF_SYMBOL_TABLE_ENTRY {
	uint32_t /*Elf32_Word*/ st_name;
	uint32_t /*Elf32_Addr*/ st_value;
	uint32_t /*Elf32_Word*/ st_size;
	uint8_t st_info;
	uint8_t st_other;
	uint16_t /*Elf32_Half*/ st_shndx;
	//16
	};

struct ELF_REL {
	uint32_t /*Elf32_Addr*/ r_offset;
	uint32_t /*Elf32_Word*/ r_info;
	};
//https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-54839.html#scrolltoc
struct ELF_RELA {
	uint32_t /*Elf32_Addr*/ r_offset;
	uint32_t /*Elf32_Word*/ r_info;
	int32_t /*Elf32_SWord*/ r_addend;
	};

struct ELF_PROGRAM_HEADER {
	uint32_t /*Elf32_Word*/ p_type;
	uint32_t /*Elf32_Off*/  p_offset;
	uint32_t /*Elf32_Addr*/ p_vaddr;
	uint32_t /*Elf32_Addr*/ p_paddr;
	uint32_t /*Elf32_Word*/ p_filesz;
	uint32_t /*Elf32_Word*/ p_memsz;
	uint32_t /*Elf32_Word*/ p_flags;
	uint32_t /*Elf32_Word*/ p_align;
	//32
	};

struct ELF_DYN {
	int32_t /*Elf32_SWord*/ d_tag;
	union {
		uint32_t /*Elf32_Word*/ d_val;
		uint32_t /*Elf32_Addr*/ d_ptr;
		} d_un;
	};

// per COFF
struct COFF_HEADER {
	uint16_t machine;
	uint16_t numberOfSections;
	uint32_t timeDateStamp;
	uint32_t pointerToSymbolTable;
	uint32_t numberOfSymbols;
	uint16_t sizeOfOptionalHeader;
	uint16_t characteristics;
	//20
	};
enum {
	IMAGE_FILE_RELOCS_STRIPPED=0x0001,
	IMAGE_FILE_EXECUTABLE_IMAGE=0x0002,
	IMAGE_FILE_LINE_NUMS_STRIPPED=0x0004,
	IMAGE_FILE_LOCAL_SYMS_STRIPPED=0x0008,
	IMAGE_FILE_AGGRESSIVE_WS_TRIM=0x0010,
	IMAGE_FILE_LARGE_ADDRESS_AWARE=0x0020,
	IMAGE_FILE_16BIT_MACHINE=0x0040,
	IMAGE_FILE_BYTES_REVERSED_LO=0x0080,
	IMAGE_FILE_32BIT_MACHINE=0x0100,
	IMAGE_FILE_DEBUG_STRIPPED=0x0200,
	IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP=0x0400,
	IMAGE_FILE_SYSTEM=0x1000,
	IMAGE_FILE_DLL=0x2000,
	IMAGE_FILE_UP_SYSTEM_ONLY=0x4000,
	IMAGE_FILE_BYTES_REVERSED_HI=0x8000
	};

struct COFF_OPTIONAL_HEADER {
	uint16_t magic;
	uint8_t majorLinkerVersion;
	uint8_t minorLinkerVersion;
	uint32_t sizeOfCode;
	uint32_t sizeOfInitializedData;
	uint32_t sizeOfUninitializedData;
	uint32_t addressOfEntryPoint;
	uint32_t baseOfCode;
	uint32_t baseOfData;			// solo PE32
	//24/28
	// Optional Header Windows NT-Specific Fields (Image Only)
	uint32_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;
	uint16_t majorOperatingSystemVersion;
	uint16_t minorOperatingSystemVersion;
	uint16_t majorImageVersion;
	uint16_t minorImageVersion;
	uint16_t majorSubsystemVersion;
	uint16_t minorSubsystemVersion;
	uint32_t reserved;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checkSum;
	uint16_t subsystem;
	uint16_t DLLcharacteristics;
	uint32_t sizeOfStackReserve;
	uint32_t sizeOfStackCommit;
	uint32_t sizeOfHeapReserve;
	uint32_t sizeOfHeapCommit;
	uint32_t loaderFlags;
	uint32_t numberofRvaAndSizes;
	//96/112
	// Optional Header Data Directories (Image Only)
	uint64_t exportTable;
	uint64_t importTable;
	uint64_t resourceTable;
	uint64_t exceptionTable;
	uint64_t certificateTable;
	uint64_t baseRelocationTable;
	uint64_t debug;
	uint64_t architecture;
	uint64_t globalPtr;
	uint64_t TLStable;
	uint64_t loadConfigTable;
	uint64_t boundImport;
	uint64_t IAT;
	uint64_t delayImportDescriptor;
	uint64_t COMruntimeHeader;
	uint64_t reserved2;
	// 216/232
	};

struct COFF_SECTION_TABLE {		// i.e. headers
	uint64_t name;
	uint32_t virtualSize;
	uint32_t virtualAddress;
	uint32_t sizeOfRawData;
	uint32_t pointerToRawData;
	uint32_t pointerToRelocations;
	uint32_t pointerToLinenumbers;
	uint16_t numberOfRelocations;
	uint16_t numberOfLinenumbers;
	uint32_t characteristics;
	//40
	};
enum {
	IMAGE_SCN_TYPE_REG=0x00000000,
	IMAGE_SCN_TYPE_DSECT=0x00000001,
	IMAGE_SCN_TYPE_NOLOAD=0x00000002,
	IMAGE_SCN_TYPE_GROUP=0x00000004,
	IMAGE_SCN_TYPE_NO_PAD=0x00000008,
	IMAGE_SCN_TYPE_COPY=0x00000010,
	IMAGE_SCN_CNT_CODE=0x00000020,
	IMAGE_SCN_CNT_INITIALIZED_DATA=0x00000040,
	IMAGE_SCN_CNT_UNINITIALIZED_DATA=0x00000080,
	IMAGE_SCN_LNK_OTHER=0x00000100,
	IMAGE_SCN_LNK_INFO=0x00000200,
	IMAGE_SCN_TYPE_OVER=0x00000400,
	IMAGE_SCN_LNK_REMOVE=0x00000800,
	IMAGE_SCN_LNK_COMDAT=0x00001000,
	IMAGE_SCN_MEM_FARDATA=0x00008000,
	IMAGE_SCN_MEM_PURGEABLE=0x00020000,
	IMAGE_SCN_MEM_16BIT=0x00020000,
	IMAGE_SCN_MEM_LOCKED=0x00040000,
	IMAGE_SCN_MEM_PRELOAD=0x00080000,
	IMAGE_SCN_ALIGN_1BYTES=0x00100000,
	IMAGE_SCN_ALIGN_2BYTES=0x00200000,
	IMAGE_SCN_ALIGN_4BYTES=0x00300000,
	IMAGE_SCN_ALIGN_8BYTES=0x00400000,
	IMAGE_SCN_ALIGN_16BYTES=0x00500000,
	IMAGE_SCN_ALIGN_32BYTES=0x00600000,
	IMAGE_SCN_ALIGN_64BYTES=0x00700000,
	IMAGE_SCN_ALIGN_128BYTES=0x00800000,
	IMAGE_SCN_ALIGN_256BYTES=0x00900000,
	IMAGE_SCN_ALIGN_512BYTES=0x00A00000,
	IMAGE_SCN_ALIGN_1024BYTES=0x00B00000,
	IMAGE_SCN_ALIGN_2048BYTES=0x00C00000,
	IMAGE_SCN_ALIGN_4096BYTES=0x00D00000,
	IMAGE_SCN_ALIGN_8192BYTES=0x00E00000,
	IMAGE_SCN_LNK_NRELOC_OVFL=0x01000000,
	IMAGE_SCN_MEM_DISCARDABLE=0x02000000,
	IMAGE_SCN_MEM_NOT_CACHED=0x04000000,
	IMAGE_SCN_MEM_NOT_PAGED=0x08000000,
	IMAGE_SCN_MEM_SHARED=0x10000000,
	IMAGE_SCN_MEM_EXECUTE=0x20000000,
	IMAGE_SCN_MEM_READ=0x40000000,
	IMAGE_SCN_MEM_WRITE=0x80000000
	};

struct COFF_RELOCATIONS {
	uint32_t virtualAddress;
	uint32_t symbolTableIndex;
	uint16_t type;
	//10
	};

struct COFF_LINENUMBERS {
	union {
		uint32_t type;
		uint32_t symbolTableIndex;
		uint32_t virtualAddress;
		};
	uint16_t linenumber;
	};

struct COFF_SYMBOL_TABLE {
	uint64_t name;
	uint32_t value;
	uint16_t sectionNumber;
	uint16_t type;
	uint8_t storageClass;
	uint8_t numberOfAuxSymbols;
	//18
	};

struct COFF_SYMBOL_NAME {
	uint64_t shortName;
	uint32_t zeroes;
	uint32_t offset;
	//16
	};

struct COFF_AUXILIARY_FORMAT1 {
	uint32_t tagIndex;
	uint32_t totalSize;
	uint32_t pointerToLineNumber;
	uint32_t pointerToNextFunction;
	uint16_t unused;
	//18
	};
struct COFF_AUXILIARY_FORMAT2 {
	uint32_t unused;
	uint16_t linenumber;
	uint8_t unused2[6];
	uint32_t pointerToNextFunction;
	uint16_t unused3;
	//18
	};
struct COFF_AUXILIARY_FORMAT3 {
	uint32_t tagIndex;
	uint32_t characteristics;
	uint8_t unused2[10];
	//18
	};
struct COFF_AUXILIARY_FORMAT4 {
	char fileName[18];
	//18
	};
struct COFF_AUXILIARY_FORMAT5 {
	uint32_t length;
	uint16_t numberOfRelocations;
	uint16_t numberOfLinenumbers;
	uint32_t checkSum;
	uint16_t number;
	uint8_t selection;
	uint8_t unused2[3];
	//18
	};

struct COFF_ATTRIBUTE_CERTIFICATE_TABLE {
	uint32_t certificateData;
	uint32_t sizeOfCertificate;
	//8
	};

struct COFF_DELAY_LOAD_DIRECTORY_TABLE {
	uint32_t attributes;
	uint32_t name;
	uint32_t moduleHandle;
	uint32_t delayImportAddressTable;
	uint32_t delayImportNameTable;
	uint32_t boundDelayImportTable;
	uint32_t unloadDelayImportTable;
	uint32_t timeStamp;
	//32
	};

struct COFF_DEBUG_DIRECTORY {
	uint32_t characteristics;
	uint32_t timeDateStamp;
	uint16_t majorVersion;
	uint16_t minorVersion;
	uint32_t type;
	uint32_t sizeOfData;
	uint32_t addressOfRawData;
	uint32_t PointerToRawData;
	//28
	};
// finire!
//Export Directory Table
//Export Address Table
//Import Directory Entry
//Import Lookup Entry
//Hint/Name Table
//Import Address Table
//Fixup Block
//TLS Directory
//Resource Directory Table
//Resource Directory Entries
//Resource Directory String
//Resource Data Entry
//Archive Member Headers
//First Linker Member
//Second Linker Member
//Import Header




#if GD24032
union GD24032_OPCODE {
    struct {
        //3 bit free (immediato shiftato?? opp. "abilitazioni" tipo no interrupt 
        //opp solo supervisor (no, inutile) opp no 64bit tipo MIPS opp va al coprocessore opp esegui su status che segue
        unsigned int reg3: 3;
        unsigned int reg1: 5;
        unsigned int mode1: 4;
        unsigned int reg2: 5;
        unsigned int mode2: 4;
        unsigned int condiz: 1;
        unsigned int size: 2;
        unsigned int doflags: 1;
        unsigned int opcode: 7;
        };
    /* NON PIU' USATO da quando abbiamo condiz in tutte le istruzioni con un solo operando!
    struct {
        unsigned int val: 9;
        unsigned int reg3: 3;
        unsigned int reg2: 5;
        unsigned int mode2: 4;
        unsigned int unused: 1;
        unsigned int size: 2;
        unsigned int doflags: 1;
        unsigned int opcode: 7;
		} o2;*/
    struct {
        unsigned int val: 8;
        unsigned int cond: 4;
        unsigned int reg2: 5;
        unsigned int mode2: 4;
        unsigned int condiz: 1;
        unsigned int size: 2;
        unsigned int doflags: 1;
        unsigned int opcode: 7;
		} o3;
    struct {
        unsigned int vall: 8;
        unsigned int cond: 4;
        unsigned int valh: 9;
        unsigned int condiz: 1;
        unsigned int size: 2;
        unsigned int doflags: 1;
        unsigned int opcode: 7;
		} o4;
    struct {
        unsigned int shifttype: 3;
        unsigned int shiftcnt: 4;
        unsigned int which: 1;
        unsigned int cond: 4;
        unsigned int reg2: 5;
        unsigned int mode2: 4;
        unsigned int condiz: 1;
        unsigned int size: 2;
        unsigned int doflags: 1;
        unsigned int opcode: 7;
		} o5;
    uint32_t d;
    };

enum ADDRESSING_MODES {
	ADDR_IMMEDIATE=0,		// 
	ADDR_IMMEDIATE8=1,		// 
	ADDR_REGISTER_DIRECT=2,
	ADDR_REGISTER_INDIRECT=3,
	ADDR_REGISTER_SYMBOLIC_INDEXED=4,
	ADDR_REGISTER_INDEXED_2_REG=5,
	ADDR_REGISTER_SYMBOLIC_INDEXED_SHORT=6,
//	ADDR_REGISTER_INDEXED_2_REG_SHORT=7,
	ADDR_REGISTER_INDIRECT_PREINCREMENT=8,
	ADDR_REGISTER_INDIRECT_PREDECREMENT=9,
	ADDR_REGISTER_INDIRECT_POSTINCREMENT=10,
	ADDR_REGISTER_INDIRECT_POSTDECREMENT=11,
	ADDR_REGISTER_INDIRECT64=12,
	ADDR_REGISTER_INDIRECT64_POSTINCREMENT=13,
	ADDR_REGISTER_INDIRECT64_2REG=14,
	};

#endif

#pragma pack( pop, before_structs )


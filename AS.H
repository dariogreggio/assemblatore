#include <stdio.h>
#include <stdint.h>

#define ST62 0
#define Z80  0
#define _6502 1
#define I8051 0
#define ARCHI 0
       
#define TRUE 1
#define FALSE 0

#if ARCHI
#define __VER__ 0
#elif Z80
#define __VER__ ((1 << 8) + 30)
#elif I8051
#define __VER__ ((1 << 8) + 0)
#elif ST62
#define __VER__ ((1 << 8) + 0)
#elif _6502
#define __VER__ ((1 << 8) + 0)
#endif

#define MAX_NAME_LEN 63

struct ERRORE {
  int t;
  const char *s;
  int16_t liv;
  };

struct SUB_CODE {
  int16_t m;                   // tipo indirizzamento
  uint32_t  n;                 // op-code
  uint16_t  c;                 // numcicli; se condiz: byte basso false, byte alto true
  };
    
struct CODE {
  const char *s;               // mnemonico
  struct SUB_CODE *a;          // struttura, con op-code 
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
  };
           
extern struct REGS Reg[];
  
// VARTYPE% BITS:
// 0= label, 7=public defined, 8=FUNZ., 9=public, 10=extern, 11=STRUCT
// VARSIZE% DIMENSIONI (PER LE FUNZIONI E' LA DIM. DI QUELLO CHE sono?)
struct VARS {
  char label[MAX_NAME_LEN+1];
  struct VARS *proc;
  uint16_t size;
  int16_t type;
  int16_t tag;
  int32_t value;
  int16_t dim;
  int32_t lpos;                  //per extern in file obj
  int16_t extUsed;
  struct VARS *next;
  struct VARS *left, *right;
  };

struct SEGS {
  char label[MAX_NAME_LEN+1];
  uint16_t type;
  long BaseAddr;
  long CurAddr;
  long len;
  long lpos;
  };
  
#define MAX_SEGS 32

struct OPERANDO {
  const char *s;
  signed char p;
  };

extern struct OPERANDO Op[];

#pragma pack( push, before_iomat )
#pragma pack(1)
struct OBJ_STRUCT {
  int16_t ident;
  int16_t version;
  char spare[28];
  long relocBase;              // dove trovo gli indirizzi reloc
  long segBase;              // dove trovo i segmenti
  char reserved[24];
  };
#pragma pack( pop, before_iomat )

enum SEGMENT_TYPES {
	SEGMENT_CODE=0,
	SEGMENT_DATA=0x100,
	SEGMENT_BSS=0x300,
	SEGMENT_CONST=0x500,
	SEGMENT_ABS=0x4100
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
	VQ_COMMA=1,
	VQ_VAR_IMM=2,		// solo _6502..
	};

enum OUT_VALUES {
	OUT_END=-9,
	OUT_VAR_EXT=-4,
	OUT_SEGM=-3,
	OUT_RELOC=-2,
	OUT_START=-1,
	OUT_VALUE=0
	};

struct LINE_DEF {
  struct LINE_DEF *next;
  struct LINE_DEF *prev;
  char s[1];
  };
  
extern int UNDEFD,IfDefs;   
extern int TAG,Segs,SX;
extern unsigned char Brack;
extern struct SEGS Seg[MAX_SEGS];
extern struct VARS *Var, *LVars;
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
extern int CurSeg;
extern unsigned char CPass,bInizioRiga,VarDeclared;
extern uint16_t PageLength,LineLength;
extern struct LINE_DEF *RootDef,*LastDef;
extern struct LINE_DEF *RootMacro,*LastMacro;
    
int FNLeggiFile(char *, FILE *,unsigned char);
char *FNParse(const char *,int *,char *);
char *FNGrab(FILE *, char *);
char *FNPreProcess(FILE *FI,const char *,char *);
char *FNTrasfNome(char *);    
char *AddExt(char *, const char *);
char *FNLA(char *);
char *FNObbLO(char *);
char *FNLO(char *);
long EVAL(char *);
int FNGetEscape();
long FNGetOct(const char *);
long FNGetBin(const char *);
int FNIsStmt();   
char *FNGetLabel(char *);
char *FNGetConst(char *);
long FNGetLine(long, char *);
enum ARITM_ELEM FNGetAritElem(char *, signed char *, struct VARS **, int *, uint16_t *, signed char *, unsigned char);
long FNEvalExpr(unsigned char, char *);
int FNRev(unsigned char, signed char *, int *, uint16_t *, struct VARS **, char *);
int FNGetAggr(int, char *, int, int *);
int FNGetSize(int32_t);
long FNGetASCII(const char *);
long FNIsType(const char *);
int FNIsOp(const char *,unsigned char);
int FNIsDirective(const char *);
int FNIsReg(const char *);
int FNIsCond(const char *);
struct VARS *FNCercaVar(char *, int,int);
int FNCercaSeg(const char *);
int PROCWarn(int, const char *);
int PROCError(int, const char *);
int PROCOut(int, int, int, int);
int PROCOut1(const char *, const char *,int);
int PROCCod(struct LINE *,const char *);
int PROCOper(char *, char *, char *, char *, char *);
struct VARS *PROCDclVar(char *, long, int, uint32_t,const char *);
int PROCDclSeg(const char *,enum SEGMENT_TYPES);
int PROCAssign(struct LINE *, int, long, int);
int PROCGetType(int *, long *, int *, int *, char *);
int PROCReturn();
int PROCObj();
int PROCS(const char *);
int PROCV(FILE *);
int PROCIsDecl();
int PROCCheck(const char *);
int PROCCheck(char);
struct VARS *PutOnTree(struct VARS *,struct VARS *,struct VARS *);

struct LINE_DEF *FNDefined(const char *A);
char *FNGetNextPre(FILE *FI, int F, char *s);
struct LINE_DEF *PROCDelLista(struct LINE_DEF *Root, struct LINE_DEF *Last, struct LINE_DEF *l);
int PROCDefine(char *A, char *B);

unsigned char asc2hex(unsigned short int t);
unsigned char *convertHEX2BIN(FILE *,unsigned long *);

#define MAKEWORD(a, b)   ((unsigned short int) (((unsigned char) (a)) | ((unsigned short int) ((unsigned char) (b))) << 8)) 
#define MAKELONG(a, b)   ((unsigned long) (((unsigned short int) (a)) | ((unsigned long) ((unsigned short int) (b))) << 16)) 
//#define HIBYTE(w)   ((unsigned char) (*((char *)&w+1)))		// molto meglio :)
#define HIBYTE(w) ((unsigned char)(((unsigned short int)(w) >> 8) & 0xFF))
#define HIWORD(l)   ((unsigned short int) (((unsigned long) (l) >> 16) & 0xFFFF)) 
#define LOBYTE(w)   ((unsigned char) (w)) 
#define LOWORD(l)   ((unsigned short int) (l)) 

#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

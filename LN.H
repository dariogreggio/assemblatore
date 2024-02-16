#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define Z80 1
#define ARCHI 0
       
#define TRUE 1
#define FALSE 0

#define ANSI  TRUE
#define ACORN FALSE
#define GD    FALSE   // REM CONVENZIONI SUI NOMIFILE

#define __VER__ 0x111
#define MAX_NAME_LEN 31
#define MAX_SEGS 32
#define MAX_CODE_SIZE 0x10000L
#define MAX_DATA_SIZE 0x10000L

struct ERRORE {
  int t;
  const char *s;
  };

// STRUCT LINE { NEXT, PREVIOUS, CHARS }
struct LINE {       
  struct LINE *next;
  struct LINE *prev;
  char s[1];
  };      
  
struct VARS {
  char name[MAX_NAME_LEN+1];
  int16_t size;
  uint16_t type;
  long value;
  long lpos;                  //per extern in file obj
  int16_t extUsed;
//  struct VARS *next;
  struct VARS *left, *right;
  struct VARS *l1, *r1;
  };

struct SEGS {
  char label[MAX_NAME_LEN+1];
  uint16_t type;
// (was type: 0x100 data o code, 0x1 se fixed o no, 2024 v. SEGMENT types
  long CurAddr;
  long BaseAddr;
  long len;
  uint8_t *ptr;
//  int16_t fixed;
//  long lpos;
  };
  
#pragma pack( push, before_iomat )
#pragma pack(1)

struct OBJ_HEADER {
  uint8_t v0;
  uint8_t v1;
  uint8_t v2;
  uint8_t numSegs;
  uint32_t res1;
  uint32_t offNomi;
  uint32_t lunNomi;
  } ObjHeader;

struct OBJ_SEG_DEF {
  uint16_t tipo;               // 0 norm, 0x4000 abs value,
  uint16_t offNome;
  uint32_t offCode;
  uint32_t lunCode;
  uint32_t offReloc;
  uint32_t lunReloc;
  uint32_t offPublic;
  uint32_t offExtern;
  uint16_t numPublic;
  uint16_t numExtern;
  } ObjSeg;
struct OBJ_RELOC {
  uint16_t type;				// 
  uint16_t reserved;		// 2 fisso pare
  uint32_t address;
  };
struct OBJ_VAR {
  uint32_t value;
  uint32_t address;
  };
#pragma pack( pop, before_iomat )

enum SEGMENT_TYPES {
	SEGMENT_CODE=0,
	SEGMENT_DATA=0x100,
	SEGMENT_BSS=0x300,
	SEGMENT_CONST=0x500,
	SEGMENT_ABS=0x4100
	};

extern signed char debug;
extern char __file__[256];
extern int __line__;
extern char __date__[10];
extern char __time__[10];
    
char *FNTrasfNome(char *);    
int PROCInit();
int PROCWarn(int, const char *, const char *);
int PROCError(int, const char *, const char *);
struct VARS *FNCercaVar(char *);
struct VARS *PROCDclVar(const char *, int, int, int);
int FNCercaSeg(const char *);
int PROCDclSeg(const char *,enum SEGMENT_TYPES,signed char);
int PROCV(char *);
struct VARS *PutOnTree(struct VARS *,struct VARS *,struct VARS *);
struct VARS *PutOnTree2(struct VARS *,struct VARS *,struct VARS *);


#define MAKEWORD(a, b)   ((unsigned short int) (((unsigned char) (a)) | ((unsigned short int) ((unsigned char) (b))) << 8)) 
#define MAKELONG(a, b)   ((unsigned long) (((unsigned short int) (a)) | ((unsigned long) ((unsigned short int) (b))) << 16)) 
//#define HIBYTE(w)   ((unsigned char) (*((char *)&w+1)))		// molto meglio :)
#define HIBYTE(w) ((unsigned char)(((unsigned short int)(w) >> 8) & 0xFF))
#define HIWORD(l)   ((unsigned short int) (((unsigned long) (l) >> 16) & 0xFFFF)) 
#define LOBYTE(w)   ((unsigned char) (w)) 
#define LOWORD(l)   ((unsigned short int) (l)) 

#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)


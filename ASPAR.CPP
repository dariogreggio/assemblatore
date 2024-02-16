#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "as.h"

FILE *FIn;

char *FNLO(char *s) {
  register int ch;
  unsigned char Exit=0;
	int i;
	unsigned char Q=0,Q1=0;
  char *s1;
  
  *s=0;
  while((ch=getc(FIn)) <= ' '/* || ch==EOF*/) {
  
    if(ch==EOF || ch=='\r' || ch=='\n') {
//      getc(FIn);
	    if(ch=='\r') {
		    ch=getc(FIn);
				if(ch != '\n')
					ungetc(ch,FIn);
				}

      __line__++;
//    printf("linea [1] vale %d\n",__line__);
      bInizioRiga=TRUE;
			VarDeclared=FALSE;
      
//  if(debug)
//    printf("FNLO vuoto\n");
    
      return s;
      }
//    else 
//      ch=getc(FIn);
    }         
  s1=s;
  do {
    if(!Q1) {
      if(ch=='\"') {
        if(!*s) {
          Q=!Q;
          }
        else {
          Exit=TRUE;
          }
        }
      }
    if(!Q) {
      if(ch=='\'') {
        if(!*s) {
          Q1=!Q1;
          }
        else {
          Exit=TRUE;
          }
        }
      }
//  PRINT CH%
//    if ((Q || Q1) && (ch=='\\')) {                  // disabilitato secondo MASM...
//      ch=FNGetEscape();
//      }
    if(!Q && !Q1) {
#if _6502		// in effetti potrebbe andare pure sugli altri, ma non si fa di solito
      if(iscsym(ch) || (ch=='#') || (ch=='$') || (ch=='?') || (ch=='\\')) {
        } 
#elif ST62		// p.es.
      if(iscsym(ch) || (ch=='$') || (ch=='?') || (ch=='\\')) {
        } 
#else
      if(iscsym(ch) || (ch=='$') || (ch=='?') || (ch=='\\')) {
        } 
#endif
      else {
        Exit=TRUE;
        }
      }
    *s1++=ch;
    if(!Exit) {
      ch=getc(FIn);
//      if(ch==EOF)
//        Exit=TRUE;
      }
    } while(!Exit);
  *s1=0;
    
//  if(debug)
//    printf("FNLO [1]: %s [%d]\n",s,__line__);
    
  if((s1-s) > 1) {
    if((ch != '\"') && (ch != '\'')) {
      *(--s1)=0;
      ungetc(ch,FIn);
      }
    }
  bInizioRiga=FALSE;
  if(*s==';') {
		do {
      ch=getc(FIn);
			} while(ch != '\n' && ch != '\r' && ch!=EOF);
    if(ch == '\r') {
	    ch=getc(FIn);
			if(ch != '\n') {
				ungetc(ch,FIn);
				}
			}
//    TEXTP++; 
    __line__++;
//    printf("linea [2] vale %d\n",__line__);
    bInizioRiga=TRUE;
		VarDeclared=FALSE;
    *s=0;
    }

  if(debug)
    printf("FNLO [2]: %s (%d) [bI: %d]\n",s,__line__,bInizioRiga);

  return s;    
  }
 
char *FNLA(char *s) {
  long l;
  int ol,oi;

  ol=__line__;
  oi=bInizioRiga;
  l=ftell(FIn);
  FNLO(s);
  fseek(FIn,l,SEEK_SET); 
  __line__=ol;
  bInizioRiga=oi;
  
  return s;
  }
 
char *FNObbLO(char *s) {
  
  if(!*FNLO(s))
    PROCError(2060,NULL);
  return s;  
  }
  
int FNGetEscape() {
  int ch;  
  char A[256];
 
  ch=getc(FIn);
  switch(ch) {
    case 'a':
      return 7;
      break;
    case 'b': 
      return 8;
      break;
    case 'f':
      return 12;
      break;
    case 'n': 
      return '\n';
      break;     
    case 'r':
      return '\r';
      break;     
    case 't':
      return '\t';  
      break;
    case 'v':
      return 11; 
      break;
    case '\\':
      return '\\';
      break;
    case '\'':
      return '\'';
      break;
    case '\"': 
      return '\"';
      break;
    case 'e': 
      return 0x1b;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      ch=FNGetOct(FNLO(A));
      fseek(FIn,-2,SEEK_CUR);
      return ch;
      break;
    case 'x':
    case 'X':
//      getc(FIn);
      FNLO(A);
      sscanf(A,"%x",&ch);
      fseek(FIn,-2,SEEK_CUR);
      return ch;
      break;
    case '\n':
      PROCError(2017,NULL);
      break;
    default:
      return ch;
      break;
    }       
    
  return ch;
  }
    
long FNGetOct(const char *A) {
  int T;
  long l=0;
  
  while(*A) {
    T=*A-'0';
    if((T>=0) && (T<8)) {
      l=l*8+T;
      }
    else {
      return l;
      }
    A++;
    }
  return l;
  }

long FNGetBin(const char *A) {
  int T;
  long l=0;
  
  while(*A) {
    T=*A-'0';
    if(!T || T==1) {
      l=l<<1 | T;
      }
    else {
      return l;
      }
    A++;
    }
  return l;
  }

long FNGetASCII(const char *A) {
  long l;
  register int i;
  const char *s;
    
  if(*A=='\'' || *A=='\"') {
	  l=0;
	  i=0;  
	  s=A+1;
	  while(*s != *A && i<4) {
	    l=l | (*s << (i*8));
	    s++;
	    i++;
	    }
    if(!i)
      PROCError(2137,A);
    }
  else
    l=atol(A);
  return l;
  }
      
int PROCCheck(const char *T) {
  char MyBuf[128];

  if(strcmp(T,FNLO(MyBuf)))
    PROCError(2054,T); 
  return 0;
  }

int PROCCheck(char T) {
  char MyBuf[256];

  if(T != *FNLO(MyBuf)) {
    *MyBuf=T;
    MyBuf[1]=0;
    PROCError(2054,MyBuf); 
    }
  return 0;
  }


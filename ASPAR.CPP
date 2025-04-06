#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "as.h"

FILE *FIn;

char *FNLO(char *s) {
  register int ch;
  unsigned char Exit=0;
//	int i;
	unsigned char Q=0,Q1=0;
  char *s1;
  
  *s=0;
// no... in tanti posti torno indietro dopo lo 0... gestire ev. 	if(!theLine[theLinePtr])				// NON muoviamoci se siam già a fine riga!
//		goto fine;

  while((ch=theLine[theLinePtr++]) <= ' ') {
  
    if(!ch || ch=='\r' || ch=='\n') {
	    if(ch=='\r') {
		    ch=theLine[theLinePtr++];
				if(ch != '\n')
					theLinePtr--;
				}

      bInizioRiga=TRUE;
			VarDeclared=FALSE;
      return s;
      }
    }         
  s1=s;
  do {
    if(!Q1) {
      if(ch=='\"') {
        if(!*s1) {
          Q=!Q;
          }
        else {
          Exit=TRUE;
          }
        }
      }
    if(!Q) {
      if(ch=='\'') {
        if(!*s1) {
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
#if _6502		// in effetti $ potrebbe andare pure sugli altri, ma non si fa di solito
      if(iscsym(ch) || (ch && strchr("$#?\\",ch))) {
        } 
#elif ST62		// p.es.
      if(iscsym(ch) || (ch && strchr("$?\\",ch))) {
        } 
#elif PIC==16
      if(iscsym(ch) || (ch && strchr("$?\\",ch))) {
        } 
#elif PIC==18
      if(iscsym(ch) || (ch && strchr("$?\\",ch))) {
        } 
#else
      if(iscsym(ch) || (ch && strchr("$?\\",ch))) {
//      if(iscsym(ch) || (ch=='$') || (ch=='?') || (ch=='\\')) {
        } 
#endif
      else {
        Exit=TRUE;
        }
      }
    *s1++=ch;
		if(*s=='<' || *s=='>') {
	    if(theLine[theLinePtr]==ch) {
				*s1++=ch;
				s1++;
				theLinePtr+=2;
				}
			}
#if GD24032
		if(*s=='+' || *s=='-') {
	    if(theLine[theLinePtr]==ch) {
				*s1++=ch;
				s1++;
				theLinePtr+=2;
				}
			}
#endif
    if(!Exit) {
      ch=theLine[theLinePtr++];
      }
	  if(Q || Q1)
			if(!ch)
				Exit=TRUE;
	  *s1=0;
    } while(!Exit);
    
//  if(debug)
//    printf("FNLO [1]: %s [%d]\n",s,__line__);
    
  if((s1-s) > 1) {
    if((ch != '\"') && (ch != '\'')) {
      *(--s1)=0;
      theLinePtr--;
      }
    }
  bInizioRiga=FALSE;
  if(*s==';') {
		do {
	    ch=theLine[theLinePtr++];
			} while(ch && ch != '\n' && ch != '\r');
    if(ch == '\r') {
	    ch=theLine[theLinePtr++];
			if(ch != '\n') {
				theLinePtr--;
				}
			}
//    TEXTP++; 
//    printf("linea [2] vale %d\n",__line__);
    bInizioRiga=TRUE;
		VarDeclared=FALSE;
    *s=0;
    }

fine:
  if(debug)
    printf("FNLO [2]: %s (%d) [bI: %d]\n",s,__line__,bInizioRiga);

  if(Q || Q1)
		PROCError(2054,"\" o '");

  return s;
  }
 
char *FNLA(char *s) {
  long l;
  int oi;

  oi=bInizioRiga;
  l=theLinePtr;
  FNLO(s);
	theLinePtr=l;
  bInizioRiga=oi;
  
  return s;
  }
 
char *FNObbLO(char *s) {
  
  if(!*FNLO(s))
    PROCError(2060);
  return s;  
  }
  
uint8_t FNGetEscape() {
  int ch;  
  char A[256];
 
  ch=theLine[theLinePtr++];
  switch(ch) {
    case 'a':
    case 'b': 
    case 'f':
    case 'n': 
    case 'r':
    case 't':
    case 'v':
    case '\\':
    case '\'':
    case '\"': 
    case 'e': 
			return FNGetEscape(ch);
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
	    theLinePtr-=2;
      return ch;
      break;
    case 'x':
    case 'X':
//      getc(FIn);
      FNLO(A);
      sscanf(A,"%x",&ch);
	    theLinePtr-=2;
      return ch;
      break;
    case '\n':
      PROCError(2017);
      break;
    default:
      return ch;
      break;
    }       
    
  return ch;
  }
    
uint8_t FNGetEscape(char ch) {
 
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
    default:
      return ch;
      break;
    }       
    
  return ch;
  }
    
NUM_TYPE FNGetOct(const char *A) {
  int T;
  NUM_TYPE l=0;
  
	//v. 						T1=strtoul(VS,NULL,8);

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

NUM_TYPE FNGetBin(const char *A) {
  int T;
  NUM_TYPE l=0;
  
	//v. 						T1=strtoul(VS,NULL,2);
  while(*A) {
    T=*A-'0';
    if(T==0 || T==1) {
      l=l<<1 | T;
      }
    else {
      return l;
      }
    A++;
    }
  return l;
  }

NUM_TYPE FNGetASCII(const char *A) {		
	// in generale consentiamo stringhe (prendendone i byte) al posto di numeri, tranne che in logici && ||
  long l;
  register int i;
  const char *s;
    
  if(*A=='\'' || *A=='\"') {
	  l=0;
	  i=0;  
	  s=A+1;
		if(*s == '\\') {
			return FNGetEscape(*(s+1));
			}
		else {
#if GD24032
			while((*s != *A) && i<8) {
#else
			while((*s != *A) && i<4) {
#endif
				l=l | (*s << (i*8));
				s++;
				i++;
				}
			if(!i)
				PROCError(2137,A);
			}
    }
  else
#if GD24032
    l=strtoull(A,NULL,10);
#else
    l=atol(A);
#endif
  return l;
  }
      
/*
 * Convert a string to an unsigned long long integer. Yury Gribov
 * https://github.com/gcc-mirror/gcc/blob/master/libiberty/strtoull.c
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
#define ULLONG_MAX 0xffffffffffffffffUL		// non prende ULL...
#define ERANGE 1			//boh
uint64_t strtoull(const char *nptr, char **endptr, int8_t base) {
	register const char *s = nptr;
	register int64_t acc;
	/*register*/ int c;
	/*register*/ int64_t cutoff;
	/*register*/ int neg=0, any;
	int64_t cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	do {
		c = *s++;
		} while(isspace(c));
	if(c == '-') {
		neg = 1;
		c = *s++;
		} 
	else if(c == '+')
		c = *s++;
	if((base == 0 || base == 16) &&
	    c == '0' && ((toupper(*s) == 'X') || (toupper(*s) == 'B'))) {
		c = s[1];
		s += 2;
		base = 16;
		}
	if(base == 0)
		base = c == '0' ? 8 : 10;
	cutoff = (uint64_t)ULLONG_MAX / (uint64_t)base;
	cutlim = (uint64_t)ULLONG_MAX % (uint64_t)base;
	for(acc=0, any=0;; c = *s++) {
		if(isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if(c >= base)
			break;
		if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
			}
		}
	if(any < 0) {
		acc = ULLONG_MAX;
		errno = ERANGE;
		} 
	else if(neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
	}

//https://android.googlesource.com/kernel/lk/+/qcom-dima-8x74-fixes/lib/libc/itoa.c
int lltoa(uint64_t num, char *str, /*int len, */int base) {
	uint64_t sum = num;
	int i = 0;
	int digit;

//	if(len == 0)
//		return -1;
	do {
		digit = (int)(sum % base);
		if(digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
		sum /= base;
		} while (sum /*&& (i < (len - 1))*/);
//	if(i == (len - 1) && sum)
//		return -1;
	str[i] = '\0';
	strrev(str);
	return 0;
	}

int PROCCheck(const char *T) {
  char MyBuf[128];

  if(strcmp(T,FNLO(MyBuf)))
    PROCError(2054,T); 
  return 0;
  }

int PROCCheck(char T) {
  char MyBuf[16];

  if(T != *FNLO(MyBuf)) {
    *MyBuf=T;
    MyBuf[1]=0;
    PROCError(2054,MyBuf); 
    }
  return 0;
  }


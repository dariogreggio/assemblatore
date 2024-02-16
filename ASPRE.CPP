#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "as.h"

int IfDefs;
struct LINE_DEF *RootDef,*LastDef;
struct LINE_DEF *RootMacro,*LastMacro;
int UNDEFD;
FILE *FPre;

struct LINE_DEF *PROCInserLista(struct LINE_DEF *Root, struct LINE_DEF *Last, struct LINE_DEF *New);
struct LINE_DEF *PROCDelLista(struct LINE_DEF *Root, struct LINE_DEF *Last, struct LINE_DEF *l);

char *FNGetParm(FILE *FI, char *s) {
  int T=0,Go=0,i;
  char TS[128],*A;
                
  i=0;             
  do {
    FNGrab(FI,TS);
	  switch(*TS) {
	    case ')':
	      if(T)
	        T--;
	      else 
	        Go=TRUE;
	      break;
	    case '(':
	      T++;
	      break;
	    case ',':
	      if(!T)
	        Go=TRUE;
	      break;
	    }
	  if(!Go) {        
	    A=TS;
	    while(*A)
	      *(s+ i++)=*A++;
	    }
	  } while(!Go);
	  
	*(s+i)=0;
  return s;
  }

char *FNParse(const char *A, int *I, char *s) {
  int B=0,Q=0,Go=0,i,ch;
  char T[128],C[64];
              
  i=0;          
  do {
    (*I)++;
	  ch=*(A+ *I);
	  *(s+ i++)=ch;
	  if(ch=='\"')
	    Q=!Q;
	  if(!Q) {
	    switch(ch) {
	      case '(':
	        B++;  
	        break;
	      case ')':
	        B--;
	        if(B<0)
	          Go=TRUE;
	        break;
	      case ',':
	        if(!B)
	          Go=TRUE;
	        break;
	      default:
	        break;
	      }
	    }
	  } while(!Go);
  *(s+i-1)=0;
  return s;            
  }

char *FNGrab(FILE *FI, char *s) {
  register int ch;
  int Q=0,Q1=0,Go=0,W=0,i;
  char MyBuf[128];
             
  i=0;         
	do {
	  ch=getc(FI);                 // converto LF in CR nel file
/*	  if(T == ';') {
	    do {
	      T=getc(FI);
	      } while(T != EOF && T != '\n');
	    T='\n';  
	    }
	    */
//	  if T%=10 T%=13
    if(ch==EOF)
      ch=0;
	  *(s+ i++)=ch;
// PRINT T%
	  if(!Q1)
	    if(ch=='\"')
	      Q=!Q;
	  if(!Q)
	    if(ch=='\'')
	      Q1=!Q1;
	  if(!Q && !Q1) {
      if(isalnum(ch) || (ch=='_') || (ch=='$') || (ch=='?')) 
	      W=TRUE;
	    else
	      Go=TRUE;
	    }          
	  else {
	    if(ch=='\n' || ch==EOF || !ch)
	      Go=TRUE;
	    }
	  } while(!Go);
	if(W) {
	  fseek(FI,-1,SEEK_CUR);
	  i--;
	  } 
	*(s+i)=0;
	if(!UNDEFD) {
//	  if(PP) {
	    strcpy(MyBuf,s);
	    FNPreProcess(FI,MyBuf,s);
//	    }
	  }
	    
  return s;
  }

char *FNGrab1(const char *A, int *K, int *I, char *s) {
  int Q=0,Q1=0,Go=0,W=0,ch,i;
    
  i=0;
  do {             
    (*I)++;
    ch=*(A+ *I);
    if(!Q1)
      if(ch=='\"')
        Q=!Q;
    if(!Q)
      if(ch=='\'')
        Q1=!Q1;
    if(!Q && !Q1) {
      if(isalnum(ch) || (ch=='_') || (ch=='$') || (ch=='?')) {
        if(!W) {
          W=TRUE;
          *K=*I;
          }
        *(s+ i++)=ch;
        }
      else {
        if(W)
          Go=TRUE;
        }
      }
    } while((!Go) && ch);
  *(s+i)=0;
  return s;                             
  }

struct LINE_DEF *FNDefined(const char *A) {
  char *I;   
  struct LINE_DEF *T;
  
  T=RootDef;
  while(T) {
    I=strchr(T->s,'(');
    if(I) {              
      if(strlen(A) == (I-T->s)) {
        if(!strncmp(T->s,A,I-(T->s))) { 
          return T;
          }  
        }
      }
    else {
      if(!strcmp(T->s,A)) {
        return T;
        }
      }
    T=T->next;
//    T=T->next;
    }
  return 0;
  }

int PROCDefine(char *A, char *B) {    
  struct LINE_DEF *New;
  int i;
  
  i=strlen(A)+strlen(B);
//  myLog->print(0,"define %s!%s\n",A,B);
  New=(struct LINE_DEF *)malloc(i+sizeof(struct LINE_DEF)+4);
  if(!New) {
    PROCError(1001,"Fine memoria DEFINE");
    }
  if(RootDef)
    LastDef=(struct LINE_DEF *)PROCInserLista((struct LINE_DEF *)RootDef,(struct LINE_DEF *)LastDef,(struct LINE_DEF *)New);
  else                                          
    RootDef=LastDef=(struct LINE_DEF *)PROCInserLista((struct LINE_DEF *)RootDef,(struct LINE_DEF *)LastDef,(struct LINE_DEF *)New);  
  strcpy(New->s,A);
  i=strlen(New->s);
  strcpy(New->s+i+1,B);
  
  return 0;
  }

char *FNGetNextPre(FILE *FI, int F, char *s) {
  char A[128];
  int Go=0,i,j;

  do {
    FNGrab(FI,A);
    } while(*A == ' ');
  i=0;
  do {
    switch(*A) {
      case 10:
      case 13:
      case '\t':
        Go=TRUE;
        break;
      case ' ':
        if(F)
          Go=TRUE;
        else 
          *(s+ i++)=' ';
        break;
      case '\\':
        getc(FI);
        break;
      default:
        j=0;
        while(A[j]) {
          *(s+ i++)=A[j++];
          }
        break;
      }
    if(!Go)
      FNGrab(FI,A);
    } while(!Go);
  fseek(FI,-1,SEEK_CUR);
  *(s+i)=0;
  return s;
  }

char *FNPreProcess(FILE *FI, const char *A, char *s) {
  int T1,T2,J1,ch,J,i;
  char *I;
  struct LINE_DEF *p;
  char JS[64],TS[64],T1S[64];
  char MyBuf[128];

  p=FNDefined(A);
  if(p) {
//    strcpy(s,p->s);
//    p=p->next;
//    strcpy(JS,p->s);
	  I=strchr(s,'(');
	  if(I>0) {
	    i=I-s;
      if(*FNGrab(FI,MyBuf)=='(') {
	      do {
	        J=0;                            
	        FNParse(s,&i,TS);
//	        if(debug>1)
//          printf("FORM %s\n",TS);
	        ch=*(s+i);
	        if(!*TS) 
	          PROCError(2060,NULL);
	        FNGetParm(FI,T1S);
//	        if(debug>1)
//	          printf("ATT %s\n",T1S);
	        if(!ch)
	          PROCError(2010,NULL);
	        do {
	          J1=J;
	          FNGrab1(JS,&J1,&J,MyBuf);
	          if(!strcmp(TS,MyBuf)) {
//             J$=LEFT$(J$,J1%-1)+T1$+MIDS(J$,J%);
	            strcpy(MyBuf,JS+J);
	            strcpy(JS+J1,T1S);
	            strcat(JS,MyBuf);
	            J=J1+strlen(T1S);
	            }
	          } while((J <= strlen(JS)) && J);
	        } while(ch != ')');
	      }
	    else {
	      PROCError(2010,NULL);
	      }
	    }
    strcpy(s,JS);
    }
  else {
    strcpy(s,A);
    }
  return s;
  }

struct LINE_DEF *PROCInserLista(struct LINE_DEF *Root, struct LINE_DEF *Last, struct LINE_DEF *New) {
  struct LINE_DEF *A;      
 
  if(Root) {
    New->next=Last->next;
    Last->next=New;
    New->prev=Last;
    if(New->next) {
      A=New->next;
      A->prev=New;
      }      
    }
  else {
    Root=New;
    Root->next=NULL;
    Root->prev=NULL;
    }
  Last=New;   
   
  return Last;
  }
 
struct LINE_DEF *PROCDelLista(struct LINE_DEF *Root, struct LINE_DEF *Last, struct LINE_DEF *l) {
  struct LINE_DEF *A;
  
  if(l==Root) {
    A=Root->next;
    if(A)
      A->prev=0;
    }
  else {
    A=l->prev;
    A->next=l->next;
    if(l->next) {
      A=l->next;
      A->prev=l->prev;
      }
//    else
    A=l->prev;
    }
  free(l);
  if(l==Last)
    return A;
  else
    return Last;
  }
 

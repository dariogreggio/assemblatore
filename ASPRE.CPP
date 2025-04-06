#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "as.h"

signed char IfDefs;
signed char UNDEFD[MAX_DEFS];
struct LINE_DEF *RootDef,*LastDef;
struct LINE_DEF *RootMacro,*LastMacro;
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
//  char T[128],C[64];
              
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
  unsigned char Q=0,Q1=0,Go=0,W=0;
	int i;
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
      if(isalnum(ch) || (ch=='_') || (ch=='$') || (ch=='?'))			// isasym( ??
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
    theLinePtr--;
	  i--;
	  } 
	*(s+i)=0;
	if(!UNDEFD[IfDefs]) {
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
    I=strchr(T->name,'(');			// ev. cambiare con VARS / lista parametri
    if(I) {              
      if(strlen(A) == (I-T->name)) {
        if(!strncmp(T->name,A,I-(T->name))) { 
          return T;
          }  
        }
      }
    else {
      if(!strcmp(T->name,A)) {
        return T;
        }
      }
    T=T->next;
//    T=T->next;
    }
  return 0;
  }

struct LINE_DEF *FNMacro(const char *A) {
  char *I;   
  struct LINE_DEF *T;
  
  T=RootMacro;
  while(T) {
    I=strchr(T->name,' ');
    if(I) {              
      if(strlen(A) == (I-T->name)) {
        if(!strnicmp(T->name,A,I-(T->name))) { 
          return T;
          }  
        }
      }
    else {
      if(!stricmp(T->name,A)) {
        return T;
        }
      }
    T=T->next;
    }
  return NULL;
  }

int PROCDefine(const char *A, const char *B) {    
  struct LINE_DEF *New;
  
//  myLog->print(0,"define %s!%s\n",A,B);
  New=(struct LINE_DEF*)malloc(sizeof(struct LINE_DEF));
  if(!New) {
    PROCError(1001,"Fine memoria DEFINE");
    }
  if(RootDef)
    LastDef=(struct LINE_DEF *)PROCInserLista((struct LINE_DEF *)RootDef,(struct LINE_DEF *)LastDef,(struct LINE_DEF *)New);
  else                                          
    RootDef=LastDef=(struct LINE_DEF *)PROCInserLista((struct LINE_DEF *)RootDef,(struct LINE_DEF *)LastDef,(struct LINE_DEF *)New);  
  strcpy(New->name,A);
	int i=strlen(B);
	New->text=(char*)malloc(i+1);
  strcpy(New->text,B);
	New->used=0;
	New->vars=NULL;
  
  return 0;
  }

struct LINE_DEF *PROCDefineMacro(const char *A, const char *B) {    
  struct LINE_DEF *New;
  
  New=(struct LINE_DEF*)malloc(sizeof(struct LINE_DEF));
  if(!New) {
    PROCError(1001,"Fine memoria DEFINE");
    }
  if(RootMacro)
    LastMacro=(struct LINE_DEF *)PROCInserLista((struct LINE_DEF *)RootMacro,(struct LINE_DEF *)LastMacro,(struct LINE_DEF *)New);
  else                                          
    RootMacro=LastMacro=(struct LINE_DEF *)PROCInserLista((struct LINE_DEF *)RootMacro,(struct LINE_DEF *)LastMacro,(struct LINE_DEF *)New);  
  strcpy(New->name,A);
	if(B) {
		int i=strlen(B);
		New->text=(char*)malloc(i+1);
		strcpy(New->text,B);
		if(New->text[i-1] == '\r' || New->text[i-1] == '\n')
			New->text[i-1]=0;
		}
	else
		New->text=NULL;
	New->used=0;
	New->vars=NULL;
  
  return New;
  }

struct LINE_DEF *PROCRedefineMacro(struct LINE_DEF *D, const char *B) {    
  int i;
  
	if(D->text)
		free(D->text);
	D->text=(char*)malloc(strlen(B)+1);
  strcpy(D->text,B);
	D->used=0;

  return D;
  }

struct VARS *FNAllocMacroVar(struct LINE_DEF *parent,const char *name,uint8_t type) {
	struct VARS *v;

	v=(struct VARS*)malloc(sizeof(struct VARS));
	if(!v)
		PROCError(1001,"Fine memoria VARS");
	memset(v,0,sizeof(struct VARS));
	strncpy(v->name,name,MAX_NAME_LEN);
	v->type=type;
	if(parent->vars) {
		struct VARS *v2=parent->vars;
		while(v2->next)
			v2=v2->next;
		v2->next=v;
		}
	else
		parent->vars=v;
	
	return v;
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
      case '\r':
      case '\n':
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
  theLinePtr--;
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
	          PROCError(2060);
	        FNGetParm(FI,T1S);
//	        if(debug>1)
//	          printf("ATT %s\n",T1S);
	        if(!ch)
	          PROCError(2010);
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
	      PROCError(2010);
	      }
	    }
    strcpy(s,JS);
    }
  else {
    strcpy(s,A);
    }
  return s;
  }

char *TranslateMacro(const struct LINE_DEF *M,const char *smacro,char *text,const char *sparm) {
//https://onlinedocs.microchip.com/pr/GUID-E06F3258-483F-4A7B-B1F8-69933E029363-en-US-2/index.html?GUID-9D7085C9-7291-4DC8-B987-81828A1E26DA
//https://www.tutorialspoint.com/assembly_programming/assembly_macros.htm
// accetto %1 o @1 (o anche \1 ??) diciamo :)
	char *d;
	const char *s,*s1;
	uint8_t parmCnt=0;
	char parms[10][32 /*MAX_NAME_LEN+1*/];
	char textOut[MACRO_SIZE]={0};
	struct VARS *v;
	char myBuf[256];
  unsigned char Go,pass=0,i,inConcat=0;
	//smacro=MOVFW
	//text=MOVF %1,W		=>  MOVF 33,W
	//sparm=33

	for(i=0; i<10; i++)
		*parms[i]=0;
	s=sparm;		// costruisco array parametri formali
	do {
		s=parse_parm(s,parms[parmCnt]);
		if(!s || !*s || *parms[parmCnt]==';' /* se commento e 0 parms.... */)
			break;
		s=parse_parm(s,myBuf,FALSE);
		if(*myBuf == ';')
			break;
		if(s && *myBuf != ',')
			PROCError(2060);
		parmCnt++;
		} while(parmCnt<10);

//				if(__line__==273) {
//					printf("MACRO %u\n",__line__);
//					}
	while(pass<2) {
		Go=FALSE;
		s=text;
		*textOut=0;
		do {
			d=myBuf;
			i=0;
			while(*s=='\r' || *s=='\n') {		// (cmq, lo faccio qua così parse_parm restituisce NULL se lo trova (utile altrove
	/*bah*/			if(*textOut && textOut[strlen(textOut)-1] != '\n')
					strcat(textOut,"\n");
				s++;
				}
			if(!(s=parse_parm(s,myBuf,TRUE)) || !*myBuf) {
				Go=1;
				break;
				}
			if(*myBuf <= ' ') {
				if(pass)
					continue;
				else
					/*strcat(textOut,"\n")*/;
				}

			if(!stricmp(d,"local")) {
	skip_line:
				while(*s && (*s!='\r' && *s!='\n'))
					s++;
				s++;
	/*bah*/			if(*textOut && textOut[strlen(textOut)-1] != '\n')	// siamo all'inizio cmq!
					strcat(textOut,"\n");
				continue;			// skippo, bah sì :)
				}
			else if(!strnicmp(d,"if",2)) {		// pre filtratura per sveltire!
				goto solo_subst;
				}
			else if(!stricmp(myBuf,"else")) {
				goto solo_subst;
				}
			else if(!stricmp(myBuf,"endif")) {
				goto solo_subst;
				}
			else if(!stricmp(myBuf,"&")) {
				inConcat=TRUE;
				continue;
				}

solo_subst:
				if(FNSubstituteVars(M,d,myBuf,parms,parmCnt)) {
					d=myBuf;
					goto found;
					}	

	found:
				if(inConcat) {
					strcat(textOut,d);
					inConcat=FALSE;
					}
				else {
					if(isasym(*d) && isasym(textOut[strlen(textOut)-1]))			// migliorare!! salvare ultimo char copiato prima... serve a non rimuovere spazi
						strcat(textOut," ");
					strcat(textOut,d);
					}
			} while(!Go);


		strcpy(text,textOut);
		pass++;
		}

	return text;
	}

int8_t FNSubstituteVars(const struct LINE_DEF *M,const char *var,char *buf,char parms[10][32],uint8_t parmCnt) {
	struct VARS *v;
	uint8_t i;

	if(*var=='%') {
		if(*(var+1)=='%') {
			itoa(M->used,buf,10);
			// (@@ o %% dovrebbe restituire il "numero di invocazioni della macro" https://what-when-how.com/microcontrollers/macro-assemblers-microcontrollers/
			goto found;
			}
		else {
			i=*(var+1)-'0';
			if(i==0) {
				itoa(parmCnt,buf,10);
				goto found;
				}
			else if(i>0 && i<=9) {
				if(i>parmCnt && !CPass)
					PROCWarn(2011,"");
				strcpy(buf,parms[i-1]);
				goto found;
				}
			}
		}

	if(isadsym(*var)) {		// inutile perder tempo se è un simbolo o altro!
		v=M->vars;
		i=0;
		while(v) {
			if(!v->type) {
				if(!stricmp(var,v->name)) {		// prima cerco nei parm...
					strcpy(buf,parms[i]);
					goto found;
					}
				i++;
				}
			v=v->next;
			}
		v=M->vars;
		while(v) {
			if(v->type && !stricmp(var,v->name)) {			// ...poi nelle local
				sprintf(buf,"%s_%u_%s",M->name,M->used,v->name);
				goto found;
				}
			v=v->next;
			}
		}

found:
	return 1;
	}

int8_t FNGetParms(const struct LINE_DEF *M,const char *string,char *s1,char *s2,char parms[10][32],uint8_t parmCnt) {
	char *s=(char*)string;
	struct VARS *v;
	char myBuf[256];
	int8_t i=0,sep=0;

	*s1=*s2=0;
	do {
		s=(char*)parse_parm(s,myBuf);
		if(!s || !*myBuf)
			break;
		if(myBuf[0]=='<') {
			s=(char*)parse_parm(s,myBuf);
			sep=1;
			}
		if(v=FNCercaVar(myBuf,0,0,FALSE,1)) {
			if(!(v->type & VAR_STRING))
				itoa(v->value,myBuf,10);
			else
				strcpy(myBuf,(char*)v->value);
			}
		if(FNSubstituteVars(M,myBuf,myBuf,parms,parmCnt)) {
			}
		if(!i)
			strcpy(s1,myBuf);
		else
			strcpy(s2,myBuf);
		i++;
		if(sep) {
			s=(char*)parse_parm(s,myBuf);
			if(myBuf[0] != '>')
				PROCError(2060,myBuf);
			sep=0;
			}
		s=(char*)parse_parm(s,myBuf);
		if(i<2 && myBuf[0] != ',')
			PROCError(2060,myBuf);
		} while(i<2);

	return i;
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
	while(l->next) {
		struct LINE_DEF *l2=l->next;
		free(l->next);
		l2=l;
		}
  free(l);
  if(l==Last)
    return A;
  else
    return Last;
  }
 

#include <stdlib.h>
#include <string.h>
#include "as.h"

enum ARITM_ELEM FNGetAritElem(char *VS, signed char *VQ, struct VARS **VVar, int *TYPE, uint16_t *SIZE, 
															signed char *OP, unsigned char Co) {
// O% (RISULTATO) = 1 SE COSTANTE
// 2 SE VARIABILE
// 3 SE OPERANDO
// 0 SE FINE LINEA
// VQ=1 per ", '
// VQ=0 o 1 se var,0 se op, -1 registro, -2 condiz
// TYPE è il registro o il cond.
// SIZE è la dimensione label o reg.
  int i,j;                     
  long T1;
  char TS[256],T1S[256],MyBuf[256];
//  char *p;
	unsigned char imm=0;
  
  FNLO(VS);
  
//  if(debug) 
//    fprintf(stderr,"GETARITM: %s\t\n",TS);
  
rifo:
  switch(*VS) {
    case 0:
    case '\n':
//    case ';':
      fseek(FIn,-1,SEEK_CUR);
      *VS=0;
//  if(debug) 
//    fprintf(stderr,"0\n");
      return ARITM_EOL;
      break;
    case ')':
      if(!Brack) {
        fseek(FIn,-1,SEEK_CUR);
        return ARITM_EOL;
        }
      else {
        *OP=1;
//        strcpy(VS,TS);
        return ARITM_OPER;
        }
      break;
    case '(':
      *OP=1;
//      *V=0;
//      strcpy(VS,TS);
      return ARITM_OPER;
      break;
    case '$':
      if(!*(VS+1)) {
	      *TYPE=0;
	      *SIZE=2;
	      *VQ=VQ_OP_VAR;
//	      sprintf(VS,"%lu",Seg[CurSeg-1].CurAddr);
				*(int32_t *)VS=MAKELONG(Seg[CurSeg-1].CurAddr,CurSeg);
//				printf("$ vale %lx\n",*(long *)VS);
	      return ARITM_CONST;               
	      }
#if _6502
	    else {
				goto isNumber;
				}
#else
	    else {
//        strcpy(VS,TS);
	      goto isVar;
	      }
#endif
      break;
    case '?':
      *TYPE=0;
      *SIZE=2;
      *VQ=VQ_OP_VAR;
//      *VS='0';
//      *(VS+1)=0;
      *(int32_t *)VS=MAKELONG(-1,0);
      return ARITM_CONST;               
      break;
    case '\"':
    case '\'':
/*      p=TS;
      i=0;
      while(*p) {
        VS[i++]=*p++;
        }  
      VS[i]=0;
      */
//      if(!i)
//        PROCError(2137,C);         // vorrei lasciare passare "" ma non ''...
      *TYPE=0;
      *SIZE=1;                 
      *VQ=VQ_VAR;
//  if(debug) 
//    fprintf(stderr,"1: VS %s V %d, T %d, S %d\n",VS,*V,*TYPE,*SIZE);
      return ARITM_CONST;
      break;
#if _6502		// in effetti potrebbe andare pure sugli altri, ma non si fa di solito
    case '#':	// (v. anche FNLO...
			imm=TRUE;
			memcpy(VS,VS+1,8);
#endif
isNumber:
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if(strchr(VS,'.')) {
        strcpy(T1S,"0 ; FLOAT");
        *TYPE=0x0;
        *SIZE=8;
//        strcpy(VS,Con[FNAllocCost(VS,T1S,3)].label);
        *VQ=VQ_OP_VAR;
        }
      else {
				if(*VS=='$') {
					memcpy(VS,VS+1,8);
					goto isHex;
					}
        j=strlen(VS)-1;
        i=VS[j];
        switch(i) {
          case 'h':
          case 'H':
            VS[j]=0;
isHex:
            sscanf(VS,"%lx",&T1);
            break;
          case 'B':
          case 'b':
            VS[j]=0;
            T1=FNGetBin(VS);
            break;
          case 'O':
          case 'o':
          case 'Q':
          case 'q':
            VS[j]=0;
            T1=FNGetOct(VS);
            break;     
          default:
            if(*VS=='0') {
		          if(toupper(*(VS+1))=='X') 
		            sscanf(VS+2,"%lx",&T1);
		          else
		            sscanf(VS,"%lu",&T1);
		          }
            else 
              sscanf(VS,"%lu",&T1);
            break;
          }
//        sprintf(VS,"%lu",T1);
				*(int32_t *)VS=T1;
	      *TYPE=0;
	      *SIZE=FNGetSize(T1);
				*VQ=imm ? VQ_VAR_IMM : VQ_OP_VAR;
        }
//  if(debug) 
//    fprintf(stderr,"1: V %d, T %d, S %d\n",*V,*TYPE,*SIZE);
	      return ARITM_CONST;
      break;
    default:
      *OP=FNIsOp(VS,Co);
      if(*OP) {
//        strcpy(VS,TS);
//        *VQ=0;
//  if(debug) 
//    fprintf(stderr,"3\n");
        return ARITM_OPER;
        }
      else {
//        strcpy(VS,TS);
#if Z80
		    if(i=FNIsReg(VS)) {				// 
		      i--;
		      *TYPE=LOBYTE(i);
		      *SIZE= (i & 0x100) ? 2 : 1;
		      *VQ=VQ_REGISTER;
		      }
		    else if(i=FNIsCond(VS)) {
		      i--;
		      *TYPE=i;
		      *SIZE=0;
		      *VQ=VQ_CONDIZ;
		      }
#elif _6502
		    if(*VQ=FNIsReg(VS)) {				// 
//		      (*VQ)--;
		      *TYPE=LOBYTE(*VQ);
		      *SIZE= (*VQ & 0x100) ? 2 : 1;
		      *VQ=VQ_REGISTER;
		      }
						// per i cond... e i registri, qua non si applica direi
#endif
				else if(!stricmp(VS,"dup")) {
		      *VS=0;
		      fseek(FIn,-3,SEEK_CUR);                        // tanto è gestito a parte...
		      return ARITM_NONE;
		      } 
        else {
isVar:
          if(*VVar=FNCercaVar(VS,FALSE,CPass)) {
	          *TYPE=0;
	          *VQ=VQ_OP_VAR;
	          T1=(*VVar)->value;
			      *SIZE=FNGetSize(T1);
//            sprintf(VS,"%lu",T1);
            *(int32_t *)VS=T1;
			      }
          else if(i=FNCercaSeg(VS)) {
	          *TYPE=0;
	          *VQ=VQ_OP_VAR;
            *(int32_t *)VS=MAKELONG(0,i);
			      *SIZE=4;
			      }
	        else {
	          *TYPE=0;
	          *VQ=VQ_OP_VAR;
	          *SIZE=1;
				    if(CPass)         
	            PROCError(2065,VS);
            *(int32_t *)VS=0;
//            sprintf(VS,"%d",0);
	          }
          }
//  if(debug) 
//    fprintf(stderr,"2: V %d, T %d, S %d\n",*V,*TYPE,*SIZE);
        return ARITM_VAR;
        }
      break;
    }
  return ARITM_UNKNOWN;
  }

int FNGetSize(int32_t T) {			// ev. poi 64 :)

  if(T >= 0) {
	  if(!(T & 0xffffff00))
	    return 1;
	  else if(!(T & 0xffff0000))
	    return 2;
	  else 
	    return 4;
	  }
	else {
	  if((T & 0xffffff00) == 0xffffff00)
	    return 1;
	  else if((T & 0xffff0000) == 0xffff0000)
	    return 2;
	  else 
	    return 4;
	  }
  }

int FNIsOp(const char *A, unsigned char Co) {
  int I,T;
  
  if(strlen(A) > 4)
    return 0;
  I=0;
  do {
    while(Op[I].s) {
	    if(!stricmp(Op[I].s,A)) {
	      break;
	      }
	    I++;
	    }
	  if(!Op[I].s)
	    return 0;
	  T=Op[I].p;
    if(T==2) {     // vado a scegliere tra "-" unario e "-" binario (o altro)
      switch(*A) {
        case '-':
          if(Co>0) {    // se non sono il primo di un'expr..
            T=0;  
            I++;
            }
          break;
        default:
          break;
        }
      }
    } while(!T);
  return T;
  }

long FNIsType(const char *A) {
  int I;
  
//  for(I=0; I<MaxTypes; I++) {
//    if(!strcmp(A,Types[I].s)) { 
//      return Types[I].type;     
//      }
//    }
  return -1;
  }

int FNIsDirective(const char *s) {
  int i;
  
  i=0;
  while(MCom[i].s) {
    if(!stricmp(s,MCom[i].s)) { 
      return MCom[i].m;     
      }          
    i++;
    }
  return 0;
  }

int FNIsReg(const char *s) {
  int i;
  
#if Z80
  if(strlen(s) > 2) {
    if(!stricmp(s,Reg[16].s))
      return 16+1 | 0x100;
    else  
      return 0;
    }
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s) && Reg[i].s[1]) { 
      return i+1 | 0x100;     
      }          
    i++;
    }
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s) && !Reg[i].s[1]) { 
      return i+1;     
      }          
    i++;
    }
#elif _6502
  i=0;
  while(Reg[i].s) {		// verificare qua!
    if(!stricmp(s,Reg[i].s) && Reg[i].s[1]) { 
      return i+1 | 0x100;     
      }          
    i++;
    }
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s) && !Reg[i].s[1]) { 
      return i+1;     
      }          
    i++;
    }
#elif I8051
  if(strlen(s) > 2) {
    if(!stricmp(s,Reg[11].s))
      return 11+1 | 0x100;
    else  
      return 0;
    }
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s)) { 
      return i+1;
      }          
    i++;
    }
#elif ST62
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s) && !Reg[i].s[1]) { 
      return i+1;     
      }          
    i++;
    }
#endif
  return 0;
  }

int FNIsCond(const char *s) {
  int i;
  
#if Z80
  if(strlen(s) > 2)
    return 0;
  i=0;
  while(Cond[i].s) {
    if(!stricmp(s,Cond[i].s)) { 
      return i+1;     
      }          
    i++;
    }
  return 0;

#elif _6502
  if(strlen(s) > 2)
    return 0;
  i=0;
  while(Cond[i].s) {
    if(!stricmp(s,Cond[i].s)) { 
      return i+1;     
      }          
    i++;
    }
  return 0;

#elif ST62
  if(strlen(s) > 2)
    return 0;
  i=0;
  while(Cond[i].s) {
    if(!stricmp(s,Cond[i].s)) { 
      return i+1;     
      }          
    i++;
    }
  return 0;
#endif
  }


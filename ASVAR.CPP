#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "as.h"

extern uint8_t CPULevel;
extern struct VARS *CurProc[MAX_NESTED_PROC];
extern uint8_t CurProcLevel;

enum ARITM_ELEM FNGetAritElem(char *VS, struct OPERAND *o, uint8_t Co) {
// O% (RISULTATO) = 1 SE COSTANTE
// 2 SE VARIABILE
// 3 SE OPERANDO
// 0 SE FINE LINEA
// VQ=1 per ", '
// VQ=0 o 1 se var,0 se op, -1 registro, -2 condiz
// TYPE è il registro o il cond.
// SIZE è la dimensione label o reg.
  int i,j;                     
  NUM_TYPE T1=0;
  char TS[256],T1S[256],MyBuf[256];
//  char *p;
	unsigned char imm=0;
	struct SEGMENTED_VALUE *lVCost=(struct SEGMENTED_VALUE *)VS;
  
  FNLO(VS);
  
//  if(debug) 
//    fprintf(stderr,"GETARITM: %s\t\n",TS);
  
rifo:
  switch(*VS) {
    case 0:
    case '\n':
//    case ';':
      theLinePtr--;
      *VS=0;
//  if(debug) 
//    fprintf(stderr,"0\n");
      return ARITM_EOL;
      break;
    case ')':
    case ']':		// 2024, ma non so è/sarebbe giusto - le quadre sarebbero SOLO per Indirect, non matematica...
      if(!Brack) {
	      theLinePtr--;
        return ARITM_EOL;
        }
      else {
        o->op=1;
//        strcpy(VS,TS);
        return ARITM_OPER;
        }
      break;
    case '(':
    case '[':
      o->op=1;
//      *V=0;
//      strcpy(VS,TS);
      return ARITM_OPER;
      break;
    case '$':
      if(!*(VS+1)) {
	      o->T=VAR_NUMBER;
	      o->S=2;
	      o->V=VQ_OP_VAR;
//	      sprintf(VS,"%lu",Seg[CurSeg-1].CurAddr);
				lVCost->v=Seg[CurSeg-1].CurAddr;
				lVCost->valid=TRUE;
				lVCost->seg=CurSeg;
				lVCost->t=0;
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
      o->T=VAR_NUMBER;
      o->S=2;
      o->V=VQ_OP_VAR;
//      *VS='0';
//      *(VS+1)=0;
			lVCost->v=-1;
			lVCost->seg=SEG_NONE;
			lVCost->valid=TRUE;
			lVCost->t=0;
      return ARITM_CONST;               
      break;
    case '\'':
      o->T=VAR_NUMBER;
      o->V=VQ_OP_VAR;
			lVCost->v=FNGetASCII(VS);
			lVCost->seg=SEG_NONE;
			lVCost->valid=TRUE;
			lVCost->t=0;
      o->S=FNGetSize(*lVCost);
      return ARITM_CONST;               
      break;
    case '\"':
/*      p=TS;
      i=0;
      while(*p) {
        VS[i++]=*p++;
        }  
      VS[i]=0;
      */
//      if(!i)
//        PROCError(2137,C);         // vorrei lasciare passare "" ma non ''...
      o->T=VAR_STRING;
      o->S=1;                 
      o->V=VQ_VAR;
//  if(debug) 
//    fprintf(stderr,"1: VS %s V %d, T %d, S %d\n",VS,*V,*TYPE,*SIZE);

//			lVCost->t=1;
// NON DOVREBBE ANCORA ESSERE USATO MA SE LO METTO Dà FASTIDIO IN GIRO...31/3/25



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
        o->T=VAR_FLOAT;
        o->S=8;
//        strcpy(VS,Con[FNAllocCost(VS,T1S,3)].label);
        o->V=VQ_OP_VAR;
        }
      else {
				if(*VS=='$') {
					memcpy(VS,VS+1,8);
					goto isHex;
					}
        j=strlen(VS)-1;
        switch(tolower(VS[j])) {
          case 'h':
            VS[j]=0;
isHex:
#if GD24032
//            sscanf(VS,"%llx",&T1);		// fare 64 bit!
						T1=strtoull(VS,NULL,16);
#else
						T1=strtoul(VS,NULL,16);
#endif
            break;
          case 'b':
            VS[j]=0;
            T1=FNGetBin(VS);
            break;
          case 'o':
          case 'q':
            VS[j]=0;
            T1=FNGetOct(VS);
            break;     
          default:
#if GD24032
            if(*VS=='0') {			// 
							switch(toupper(*(VS+1))) {
								case 'X':
//		            sscanf(VS+2,"%llx",&T1);
									T1=strtoull(VS,NULL,16);
									break;
								case 'B':
									T1=strtoull(VS+2,NULL,2);
									break;
								default:
									if(*(VS+1)) {
										if(isdigit(*(VS+1))) {
		//		            sscanf(VS,"%llu",&T1);
											T1=strtoull(VS,NULL,10);
											}
										else
											PROCError(2057,VS);
										}
									else
										T1=0;
									break;
								}
		          }
            else 
							T1=strtoull(VS,NULL,10);
#else
            if(*VS=='0') {
		          if(toupper(*(VS+1))=='X') 
								T1=strtoul(VS,NULL,16);
		          else
								T1=strtoul(VS,NULL,10);
		          }
            else 
							T1=strtoul(VS,NULL,10);
#endif
            break;
          }
//        sprintf(VS,"%lu",T1);
				{
				lVCost->v=T1;
				lVCost->seg=SEG_NONE;
				lVCost->valid=TRUE;
				lVCost->t=0;
	      o->T=VAR_NUMBER;
	      o->S=FNGetSize(*lVCost);
				}
#if _6502
				o->V=imm ? VQ_VAR_IMM : VQ_OP_VAR;
#else
				o->V=VQ_OP_VAR;
#endif
        }
//  if(debug) 
//    fprintf(stderr,"1: V %d, T %d, S %d\n",*V,*TYPE,*SIZE);
      return ARITM_CONST;
      break;
		case 'T':
		case 't':
			if(!strcmp(VS,"TRUE")) {
				lVCost->v=/* - :) */ 1;
				lVCost->seg=SEG_NONE;
				lVCost->valid=TRUE;
				lVCost->t=0;
	      o->T=VAR_NUMBER;
	      o->S=1;
				o->V=VQ_OP_VAR;
	      return ARITM_CONST;
				}
			else
				goto no_special;
			break;
		case 'F':
		case 'f':
			if(!strcmp(VS,"FALSE")) {
				lVCost->v=0;
				lVCost->seg=SEG_NONE;
				lVCost->valid=TRUE;
				lVCost->t=0;
	      o->T=VAR_NUMBER;
	      o->S=1;
				o->V=VQ_OP_VAR;
	      return ARITM_CONST;
				}
			else
				goto no_special;
			break;
    default:
no_special:
      o->op=FNIsOp(VS,Co);
      if(o->op) {
//        strcpy(VS,TS);
//        *VQ=0;
//  if(debug) 
//    fprintf(stderr,"3\n");
        return ARITM_OPER;
        }
      else {
//        strcpy(VS,TS);
#if Z80 || ARCHI
		    if(i=FNIsReg(VS)) {				// 
		      i--;
		      o->T=LOBYTE(i);
		      o->S= (i & 0x100) ? 2 : 1;
		      o->V=VQ_REGISTER;
		      }
		    else if(i=FNIsCond(VS)) {
		      i--;
		      o->T=i;
		      o->S=0;
		      o->V=VQ_CONDIZ;
		      }
				else 
#elif _6502 || ST62 || I8051 || PIC
		    if(o->V=FNIsReg(VS)) {				// 
//		      (*VQ)--;
		      o->T=LOBYTE(o->V);
		      o->S= (o->V & 0x100) ? 2 : 1;
		      o->V=VQ_REGISTER;
		      }
						// per i cond... e i registri, qua non si applica direi
				else 
#elif I8086
		    if(i=FNIsReg(VS)) {				// 
		      i--;
		      o->T=LOBYTE(i);
					o->S= (i & 0x200) ? 4 : ((i & 0x100) ? 2 : 1);
		      o->V=VQ_REGISTER;
		      }
				else 
#elif GD24032
		    if(i=FNIsReg(VS)) {				// 
		      i--;
		      o->T=LOBYTE(i);
		      o->S=4;			// default: MOV.d (v. anche forcedSize di là)
		      o->V=VQ_REGISTER;
		      }
		    else if(i=FNIsCond(VS)) {
		      i--;
		      o->T=i;
		      o->S=0;
		      o->V=VQ_CONDIZ;
		      }
				else 
#endif
				if(!stricmp(VS,"DUP")) {
		      *VS=0;
		      theLinePtr-=3;                        // tanto è gestito a parte...
		      return ARITM_NONE;
		      } 
        else {
					struct TAGS *myT;
isVar:
					if(CurProcLevel>0) {
						if(o->var=FNCercaVar(VS,(uint32_t)CurProc[CurProcLevel-1],0,FALSE,CPass)) {
							struct SEGMENTED_VALUE ll;
#if Z80
							o->T=14;		// IY VERIFICARE!
#elif I8051
							o->T=9;		// R6 VERIFICARE!
#elif ST62
							o->T=2;		// Y VERIFICARE!
#elif _6502
							o->T=3;		// S VERIFICARE!
#elif PIC==16 || PIC==18
							o->T=2;		// IND2 verificare
#elif GD24032
							o->T=29;		// R29 VERIFICARE!
#elif I8086
							o->T=13;		// BP
#endif
//							o->V=VQ_OP_VAR;
				      o->V=VQ_REGISTER;
							o->Ind=TRUE;
							T1=o->var->value;
							o->S=o->var->size;
							lVCost->v=T1;

							lVCost->seg=SEG_NONE;
							lVCost->valid=TRUE;
							lVCost->t=2;
							o->l.t=2;
							o->l.v=lVCost->v;
			        return ARITM_VAR;
							}
						}
          if(o->var=FNCercaVar(VS,0,0,FALSE,CPass)) {
						struct SEGMENTED_VALUE ll;
	          o->T=o->var->type;
	          o->V=VQ_OP_VAR;
	          T1=o->var->value;
						ll.v=T1;
						ll.seg=o->var->segm;
						if(o->var->type & VAR_ADDRESS)
							o->S=FNGetSize(ll);
						else
							o->S=o->var->size;
//            sprintf(VS,"%lu",T1);
            lVCost->v=T1;



//						if(o->var->type & VAR_STRING)
//							strcpy(VS,(char*)o->var->value);



            lVCost->seg=o->var->segm;
            lVCost->valid=TRUE;
						lVCost->t=2;
						o->l.t=2;
			      }
          else if(myT=FNCercaAggr(VS,0,0)) {	// solo top-level record
						struct TAGS *myT2;
	          o->T=myT->type & (VAR_RECORD | VAR_STRUCT | VAR_UNION);
	          o->V=VQ_OP_VAR;
//						o->var=(struct VARS*)myT;
						if(myT->type & VAR_RECORD) {
							o->S=(myT->size+7)/8;
	            lVCost->v=myT->value;
							}
						else {
							o->S=myT->size;
	            lVCost->v=(int)myT;
							}
            lVCost->seg=SEG_UNDEFINED;
            lVCost->valid=TRUE;
						lVCost->t=0  /*o->T*/;
						o->l.t=0;

						if(myT->type & VAR_RECORD) {
							if(*FNLA(MyBuf) == '<') {		// può non esserci... quando è usato come Tipo o come Cast
	//						if(Co>2 && *FNLA(MyBuf) == '<')
								lVCost->v=FNInitAggr(myT,FALSE);

	/*							lVCost->v=FNUpdAggr(myT);*/
								}
							else
								lVCost->v=FNUpdAggr(myT);
							o->S=FNGetSize(*lVCost);		// e sopra?? boh...
							}
						else {

							}
			      }
          else if(myT=FNCercaAggr(VS,2,0)) {	// e ora i membri (scandaloso MASM
						lVCost->v=myT->pos;

// questa parte da MSB!! è giusto così o no?? v. VID GlaBIOS


            lVCost->seg=SEG_UNDEFINED;
            lVCost->valid=TRUE;
						lVCost->t=0  /*o->T*/;
	          o->T=VAR_NUMBER;
	          o->V=VQ_COST;
						o->S=1;
						o->l.t=0;
			      }
          else if(i=FNCercaSeg(VS)) {
	          o->T=VAR_NUMBER;
	          o->V=VQ_OP_VAR;
            lVCost->v=0;
						lVCost->seg=i;			// non era giusto metterla in v?? (2024..verificare...
            lVCost->valid=TRUE;
						lVCost->t=0;
#if GD24032
			      o->S=8;
#else
			      o->S=4;
#endif
			      }
	        else {		// qua per una label-forward declaration, quindi metto il segmento (2024)
	          o->T=VAR_NUMBER;
	          o->V=VQ_OP_VAR;
	          o->S=1;
				    if(CPass)         
	            PROCError(2065,VS);
            lVCost->v=0;
						lVCost->seg=SEG_UNDEFINED /*CurSeg*/;
            lVCost->valid=TRUE;
						lVCost->t=0;
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

int FNGetSize(struct SEGMENTED_VALUE T) {			// 

#if GD24032
	if(!T.valid)
		return 4;

	if(!CPass) {
		if(T.seg != SEG_NONE && T.seg != SEG_UNDEFINED && (Seg[T.seg-1].type & 0xff00) != (SEGMENT_VIRT & 0xff00))
	//	if(T.seg && T.seg != CurSeg)		// in effetti direi di no, forse così? NO sballa 1/2 CPass...


			return 4 /*4*/;		// 8  mah così ci si incasina...
		}
	else {
		if(T.seg != SEG_NONE && T.seg != SEG_UNDEFINED && T.seg != CurSeg && (Seg[T.seg-1].type & 0xff00) != (SEGMENT_VIRT & 0xff00))		// in effetti direi di no, forse così? NO sballa 1/2 CPass...
			return 8 /*4*/;		// 8  mah così ci si incasina...
		}

#else
	if(!T.valid)
		return 4;
	if(!CPass) {
		if(T.seg != SEG_NONE && T.seg != SEG_UNDEFINED && (Seg[T.seg].type & 0xff00) != (SEGMENT_VIRT & 0xff00))	{	//2025
	//	if(T.seg && T.seg != CurSeg)		// in effetti direi di no, forse così? NO sballa 1/2 CPass...


#if I8086
			if(CPULevel>=3)
				return 4;
			else
#endif
			return 2;
			}
		}

	else {
		if(T.seg != SEG_NONE && T.seg != SEG_UNDEFINED && T.seg != CurSeg && (Seg[T.seg].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {		// in effetti direi di no, forse così? NO sballa 1/2 CPass...
//2025		if(T.seg && T.seg != CurSeg)		// in effetti direi di no, forse così? NO sballa 1/2 CPass...
#if I8086
			if(CPULevel>=3)
				return 6;
			else
#endif
			return 4;
			}
		}
#endif

  if(T.v >= 0) {
	  if(!(T.v & 0xffffff00))
	    return 1;
	  else if(!(T.v & 0xffff0000))
	    return 2;
#if GD24032
	  else if(!(T.v & 0xffffffff00000000UL))
	    return 4;
	  else 
	    return 8;
#else
	  else 
	    return 4;
#endif
	  }
	else {
	  if((T.v & 0xffffff00) == 0xffffff00)
	    return 1;
	  else if((T.v & 0xffff0000) == 0xffff0000)
	    return 2;
#if GD24032
	  else if((T.v & 0xffffffff00000000UL) == 0xffffffff00000000UL)
	    return 4;
	  else 
	    return 8;
#else
	  else 
	    return 4;
#endif
	  }
  }

int FNIsOp(const char *A, unsigned char Co) {
  int I,T;
  struct OPERANDO oc,*on;
  
//  on=(struct OPERANDO *)bsearch((void *)&oc,Operandi,sizeof(Operandi)/sizeof(struct OPERANDO),sizeof(Operandi[0]),(int (*)(const void *, const void *))subCmpOper);
  
//  if(strlen(A) > 6)		// bah serve?? 2025 masm ha op + lunghi cmq
//    return 0;

  I=0;
  do {
    while(Operandi[I].s) {
	    if(!stricmp(Operandi[I].s,A)) {
	      break;
	      }
	    I++;
	    }
	  if(!Operandi[I].s)
	    return 0;
	  T=Operandi[I].p;
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
//  struct MY_CODE tc,tcn;
  
//  tn=(struct MY_CODE *)bsearch((void *)&tc,MCom,sizeof(MCom)/sizeof(struct MY_CODE),sizeof(MCom[0]),(int (*)(const void *, const void *))subCmpType);
  
//  for(I=0; I<MaxTypes; I++) {
//    if(!strcmp(A,Types[I].s)) { 
//      return Types[I].type;     
//      }
//    }
  return -1;
  }

int FNIsDirective(const char *s) {
  int i;
  struct MY_CODE cc,*cn;
  
//  cn=(struct MY_CODE *)bsearch((void *)&cc,MCom,sizeof(MCom)/sizeof(struct MY_CODE),sizeof(MCom[0]),(int (*)(const void *, const void *))subCmpMyCode);
  
  i=0;
  while(MCom[i].s) {
    if(!stricmp(s,MCom[i].s)) { 
      return MCom[i].m;     
      }          
    i++;
    }
  return -1;
  }

int FNIsReg(const char *s) {
  int i;
  struct REGS rc,*rn;
  
//  rn=(struct COND *)bsearch((void *)&rc,Reg,sizeof(Reg)/sizeof(struct REGS),sizeof(Reg[0]),(int (*)(const void *, const void *))subCmpReg);

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

#elif PIC==16 || PIC==18
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s) && !Reg[i].s[1]) { 
      return i+1;     
      }          
    i++;
    }

#elif GD24032
  if(strlen(s) > 3)
    return 0;
  i=0;
  while(Reg[i].s) {
    if(i<32 /* SP e PC non vanno qua!*/ && !stricmp(s,Reg[i].s)) { 
      return i+1;     
      }          
    i++;
    }

#elif I8086
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s) && Reg[i].s[2] && Reg[i].s[3]) {			// 32bit
      return i+1 | 0x400;
      }          
    i++;
    }
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s) && Reg[i].s[2]) {		// 16bit
      return i+1 | 0x200;
      }          
    i++;
    }
  i=0;
  while(Reg[i].s) {
    if(!stricmp(s,Reg[i].s)) {
      return (Reg[i].s[1]=='H' || Reg[i].s[1]=='L') ? i+1 : (i+1 | 0x100);
      }          
    i++;
    }

#endif
  return 0;
  }

int FNIsCond(const char *s) {
  int i;
  struct COND cc,*cn;

//  cn=(struct COND *)bsearch((void *)&cc,Cond,sizeof(Cond)/sizeof(struct COND),sizeof(Cond[0]),(int (*)(const void *, const void *))subCmpCond);

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

#elif _6502			// direi inutile cmq..
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

#elif I8051
	return 0;

#elif PIC
	return 0;

#elif GD24032
  if(strlen(s) > 2)
    return 0;
  i=0;
  while(Cond[i].s) {
    if(!stricmp(s,Cond[i].s)) { 
      return Cond[i].m+1;		// OKKIO diverso da altre, ok
      }          
    i++;
    }
  return 0;

#elif I8086
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


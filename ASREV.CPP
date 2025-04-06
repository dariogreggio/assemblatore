#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <ctype.h>
#include "as.h"

// (2025 forse su registri, su var è ok  SU OPERATORI UNARI NON VA ANCORA!! ~R5 entra in loop... strano

uint8_t Brack=0;       
extern uint8_t CPULevel;


#pragma warning USARE o.l.t !! per costanti/var/stringhe
struct SEGMENTED_VALUE FNEvalExpr(unsigned char Pty, void *C) {
	struct OPERAND o;
	int i;

	memset(&o,0,sizeof(struct OPERAND));

  memset(C,0,16);		// un tot per coprire le varie possibilità...
  i=FNRev(Pty,&o,C);
    
	if(i==0) {
		switch(o.V) {
			case ARITM_CONST:
//				o.l.v=FNGetASCII((const char*)C); no 2025, gestito come stringa, poi nei vari casi
				if(*(const char*)C=='\"' || *(const char*)C=='\'') {
					o.l.t=1;
					}
				else {
					*(uint64_t*)C=0;
					o.l.t=0;
					}
				o.l.seg=SEG_UNDEFINED;
				o.l.valid=TRUE;
				break;
			case ARITM_VAR:
			case ARITM_EOL:
				// mah credo che andrebbe gestito...
			case ARITM_UNKNOWN:
				// idem
			default:
				o.l=*(struct SEGMENTED_VALUE *)C;
				o.l.t=2;
				if(o.var) {
					if(!(o.var->type & VAR_STRING))
						*(uint64_t*)C=0;
					}
				else
					*(uint64_t*)C=0;
				o.l.valid=TRUE;
				break;
			}
		}
  return o.l;
  }
 
int FNRev(uint8_t Pty,struct OPERAND *o, void *VCost) {		// restituisce 0 se espressione completa ok; -1 se errore; -2 se incontrato registro dopo operatore
  int v,Co=0,i;
	enum ARITM_ELEM T;
  int8_t Exit=0;
  NUM_TYPE l,l2;
  char RCost[256],TS[256],T1S[256];
  long p,p2;
	struct OPERAND R;
	struct SEGMENTED_VALUE *lVCost=(struct SEGMENTED_VALUE *)VCost;
 
  p=theLinePtr;
//  ROut=LastOut;
  do {
    p2=theLinePtr;

		memset(&R,0,sizeof(struct OPERAND));


    T=FNGetAritElem(TS,o,Co);
// O% (RISULTATO) = 1 SE COSTANTE
// 2 SE VARIABILE
// 3 SE OPERANDO
// 0 SE FINE LINEA
// V>=0 se var, -1 registro, -2 condiz
// TYPE è il registro o il cond.
// SIZE è la dimensione label o reg.
    if(debug) 
      fprintf(stderr,"EVx( %d): LETTO AritmElem : %d, %s, V=%d, tipo=%d, size=%d, pty %d\n",
			Co,T,TS,o->var,o->T,o->S,Pty);

    switch(T) {
      case ARITM_NONE:
				return Co>0 ? 0 : -1;
        Exit=TRUE;
        break;
      case ARITM_CONST:
        if(Co>0) {                       // per gestire due op senza separatore
          theLinePtr=p2;
          Exit=TRUE;
          }
        else {
          if(o->V==VQ_OP_VAR) {
            *lVCost=*(struct SEGMENTED_VALUE *)TS;
						lVCost->seg=SEG_NONE;
						lVCost->valid=TRUE;
						lVCost->t=0;
						}
          else
            strcpy((char*)VCost,TS);
          }    
//          strcpy(VCost,TS);
//        l=atol(VCost);  
        break;
      case ARITM_VAR:
        if(o->V < VQ_OP_VAR) {
          theLinePtr=p2;
          Exit=TRUE;
					}
        else {
          if(Co>0) {                      // idem
	          theLinePtr=p2;
            Exit=TRUE;
            }
          else {
            if(o->V==VQ_OP_VAR) {
		          *lVCost=*(struct SEGMENTED_VALUE *)TS;
//							lVCost->seg=SEG_UNDEFINED;
							lVCost->valid=TRUE;
//							lVCost->t=0;
							}
            else
              strcpy((char*)VCost,TS);
            }
          }
//        else 
//          l=Var[*VVar].value;  
        break;
      case ARITM_OPER:
        if(o->op>Pty) {
          Exit=TRUE;
          theLinePtr=p2;
          }
        else {
          *T1S=0;
          switch(o->op) {
            case 1:
              switch(*TS) {
                case '(':
                  Brack++;
                case '[':
									{
									struct VARS *myV=(struct VARS*)VCost;
									myV=o->var;
									if(myV) {
										v=FNRev(o->op-1,&R,RCost);
										if(v<0)
											return -1;
										switch(R.V) {
											case VQ_VAR:
  											l2=FNGetASCII(RCost);
												break;
											case VQ_OP_VAR:
  											l2=*(NUM_TYPE *)RCost;  
												break;
											case VQ_REGISTER:
												// mah boh
										    T=FNGetAritElem(TS,o,Co);
												PROCWarn(2062,"register"/*RCost*/);
												break;
											default:
												PROCError(2062,RCost);
												break;
											}
 			  			      o->V=0;
										o->var=NULL;
										if(myV->type & VAR_ADDRESS /* per ora sorvolo && myV->type & VAR_ARRAY*/) {
							        lVCost->v=myV->value+l2;
											}
										else if(myV->type & VAR_RECORD) {
							        lVCost->v=myV->value >> (l2*8);		// bah, sì: i byte come fosse array
											}
										else {
// no, arriva anche con Label/Var[index]											PROCError(2062,(char*)TS);
//							        lVCost->v=myV->value+l2;
											}
										o->S=1;		// direi 
										o->l.valid=TRUE;
										o->l.v=lVCost->v;
										o->T = VAR_ARRAY;
										o->l.seg=SEG_NONE;
										lVCost->seg=SEG_NONE;
										PROCCheck(']');
										}
									else
										FNRev(99,o,VCost);
									}
                  break; 
                case ')':
                  if(Pty==99) {
                    Brack--;
                    Pty=1;
                    }
                  else {
								    theLinePtr--;
		                Exit=TRUE;
										}
									break;
                case ']':
							    theLinePtr--;
                  Exit=TRUE;
                  break;
                case '.':
									{
									if(lVCost->t & 2) {
										struct VARS *myV=(struct VARS*)VCost;
										myV=o->var;
										if(myV && myV->type & (VAR_STRUCT | VAR_UNION)) {
											struct TAGS *myT;
											FNLO(TS);
											if(myT=FNCercaAggr(TS,0,(uint32_t)myV->tag)) {
												lVCost->v=myV->value+myT->pos;
												lVCost->seg=SEG_UNDEFINED;


												o->S=myT->size;

										o->l.valid=TRUE;
										o->l.v=lVCost->v;
										o->l.seg=SEG_NONE;
												}
											else
												PROCError(2038,TS);

											}
										}
									else if(o->T & (VAR_STRUCT | VAR_UNION)) {
										struct TAGS *myT;
										if(myT=(struct TAGS*)lVCost->v) {
											FNLO(TS);
											if(myT=FNCercaAggr(TS,0,(uint32_t)myT)) {
												lVCost->v=myT->pos;
												lVCost->seg=SEG_NONE;


												o->S=1;		// mah direi, di solito
										o->l.valid=TRUE;
										o->l.v=lVCost->v;
										o->l.seg=SEG_NONE;
											}
											}

										}
									else {


													PROCError(2036,TS);
										}
									}
                  break;
                default:
                  break;
                }
              break;

            case 2:
              switch(tolower(*TS)) {		// qua ci sono alcuni op. a parole
                case '-': 
//                printf("operatore -: %d",Co);
                  if(Co>0) 
                    goto binaryMinus;
	                else {
	                  v=FNRev(2,o,VCost);
										if(v<0)
											return -1;
										switch(o->V) {
											case VQ_VAR:
												l=FNGetASCII((const char*)VCost);
												l=-l;
#if GD24032
						            l=MAKEDLONG(LODWORD(l),0);
#else
						            l=MAKELONG(LOWORD(l),0);
#endif
  		    			        lltoa(l,(char*)VCost,10);
  			  			        o->V=0;
												break;
											case VQ_REGISTER:
									theLinePtr=p2;
									return -2;
												break;
											default:
												if(lVCost->seg != SEG_NONE && lVCost->seg != SEG_UNDEFINED)          // no operatori su segmentati (extrn, label)
													PROCError(2062);
												lVCost->v=-lVCost->v;  
//					            lVCost->seg=SEG_NONE;  
												break;
											}
	    			        }
//                printf("ritorna : %lx\n",*(long *)VCost);
	                break;
		            case '~':
		            case 'n':			// NOT
#if I8086
									if(tolower(*(TS+1)) == 'e') {		// NEAR
										break;
										}
									else
#endif
									{
		              v=FNRev(2,o,VCost);
									if(v<0)
										return -1;
									switch(o->V) {
										case VQ_VAR:
											l=FNGetASCII((const char*)VCost);
											l=~l;
#if GD24032
						          l=MAKEDLONG(LODWORD(l),0);
#else
						          l=MAKELONG(LOWORD(l),0);
#endif
  		    			      lltoa(l,(char*)VCost,10);
  			  			      o->V=0;
											break;
										case VQ_REGISTER:
									theLinePtr=p2;
									return -2;
											goto fine;
											break;
										default:
											if(lVCost->seg != SEG_NONE && lVCost->seg != SEG_UNDEFINED)          // no operatori su segmentati (extrn, label)
												PROCError(2062);
											lVCost->v=~lVCost->v;  
//					            lVCost->seg=SEG_NONE;  
											break;
										}
										}
	    			      break;

	              case '!':
	                v=FNRev(2,o,VCost);
									if(v<0)
										return -1;
									switch(o->V) {
										case VQ_VAR:
											l=FNGetASCII((const char*)VCost);
											l=l ? 0 : 1;			// v. altrove, tipo EQ GT ecc
#if GD24032
						          l=MAKEDLONG(LODWORD(l),0);
#else
						          l=MAKELONG(LOWORD(l),0);
#endif
  		    			      lltoa(l,(char*)VCost,10);
  			  			      o->V=0;
											break;
										case VQ_REGISTER:
							theLinePtr=p2;
							return -2;
											goto fine;
											break;
										default:
											if(lVCost->seg != SEG_NONE && lVCost->seg != SEG_UNDEFINED)          // no operatori su segmentati (extrn, label)
												PROCError(2062);
											lVCost->v=lVCost->v ? 0 : -1;			// preferisco! (lui metterebbe 1
//					            lVCost->seg=SEG_NONE;  
											break;
										}
	    			      break;

	              case 's':		// sizeof
	                v=FNRev(2,o,VCost);
							if(v<0)
								return -1;
#if I8086
									if(tolower(TS[1]) == 'e') {		// SEG 
										switch(o->V) {
											case VQ_OP_VAR:			// boh...
		  									l=*(NUM_TYPE *)VCost;  
												lVCost->v=HIWORD(l);
												lVCost->seg=CurSeg;
												break;
											case VQ_VAR:		// 
												if(o->l.seg != SEG_NONE && o->l.seg != SEG_UNDEFINED)
													lVCost->v= (Seg[i].type & 0xff00) == (SEGMENT_VIRT & 0xff00) ? CurSeg : HIWORD(Seg[o->l.seg].BaseAddr);
												else
													lVCost->v=HIWORD(o->l.v);
												lVCost->seg=o->l.seg;
												break;
											default:
												PROCError(2062);
												break;
											}
										o->S=2;
		                o->T=VAR_NUMBER;
										o->V=VQ_OP_VAR;		// serve a indicare che è una costante, 2025, non so se interessa
										}
									else if(tolower(TS[1]) == 'h')		// SHORT
										// FORZO JMP SHORT di là...
			              break;
									else 
#endif
										{
										lVCost->v=o->S;
										lVCost->seg=SEG_UNDEFINED;
										o->T=VAR_NUMBER;
										o->S=2/*INT_SIZE*/;
										o->V=0;
										}
	                break;

	              case 'l':		// length
	                v=FNRev(2,o,VCost);
							if(v<0)
								return -1;
	                if(*(TS+1) == 'e') {			// length
										if(o->T & (VAR_STRUCT | VAR_UNION | VAR_RECORD)) {
											// gestire array... fatti con DUP v. masm
											lVCost->v=o->S;
											}
										else {
											lVCost->v=o->S;
											}
										lVCost->seg=SEG_UNDEFINED;
		                o->S=2/*INT_SIZE*/;
		                }
		              else {										// low
										switch(o->V) {
											case VQ_VAR:
		                    l=FNGetASCII((const char*)VCost);
												break;
											case VQ_REGISTER:
									theLinePtr=p2;
									return -2;
												goto fine;
												break;
											default:
											if(lVCost->seg != SEG_NONE && lVCost->seg != SEG_UNDEFINED)          // no operatori su segmentati (extrn, label)
													PROCError(2062);
  											l=*(NUM_TYPE *)VCost;  
												break;
											}
					          lVCost->v=LOBYTE(LOWORD(l));  
										lVCost->seg=SEG_UNDEFINED;
		                o->S=1;
		                }  
	                o->T=VAR_NUMBER;
	                o->V=0;
	                break;

								case 't':			// Tipo, FARE
									PROCError(1007);
									break;

	              case 'h':							// high
	                v=FNRev(2,o,VCost);
									if(v<0)
										return -1;
									switch(o->V) {
										case VQ_VAR:
		                  l=FNGetASCII((const char*)VCost);
											break;
										case VQ_REGISTER:
									theLinePtr=p2;
									return -2;
											goto fine;
											break;
										default:
											if(lVCost->seg != SEG_NONE && lVCost->seg != SEG_UNDEFINED)          // no operatori su segmentati (extrn, label)
												PROCError(2062);
  										l=*(NUM_TYPE *)VCost;  
											break;
										}
				          lVCost->v=HIBYTE(LOWORD(l));  
									lVCost->seg=SEG_UNDEFINED;
	                o->S=1;
	                o->T=VAR_NUMBER;
	                o->V=0;
	                break;

#if I8086
		            case 'b':			// BYTE
		              break;
//		            case 'w':			// WORD
//		              break;
		            case 'd':			// DWORD
		              break;
		            case 'q':			// QWORD
		              break;
		            case 'f':			// FAR
		              break;
		            case 'p':			// PTR
		              break;
//		            case 's':			// SHORT
//		              break;
		            case 'o':			// OFFSET
	                v=FNRev(2,o,VCost);
							if(v<0)
								return -1;
									switch(o->V) {
										case VQ_OP_VAR:
		  						    l=*(NUM_TYPE *)VCost;  
											break;
										case VQ_VAR:
										default:
											PROCError(2062);
											break;
										}
				          lVCost->v=LOWORD(l);
									lVCost->seg=SEG_UNDEFINED;
//									if(CPULevel<3)
										o->S=PTR_SIZE;
	                o->T=VAR_NUMBER;
									o->V=VQ_OP_VAR;		// serve a indicare che è una costante, 2025, non so se interessa
		              break;
#endif
		            case 'm':			// MASK
									{
										struct TAGS *myT,*myT2;
										FNLO(TS);
										myT=FNCercaAggr(TS,2,0);			// CERCARE ANCHE in TUTTI i membri (figli)!! ;:_$% idioti...
										if(myT) {
 			  			        o->V=0;
											o->S=(myT->size+7)/8;
											o->l.valid=TRUE;
											o->l.v=0;
											o->l.seg=SEG_NONE;
											i=myT->size;
											myT2=(struct TAGS*)myT->tag;
											if(!myT2)
												PROCError(2062,TS);
											o->l.v = l = (1 << (myT2->size-1)) >> myT->pos;
											while(--i) {
												o->l.v >>= 1;
												o->l.v |= l;
												}
						          lVCost->v=o->l.v;
											lVCost->seg=SEG_NONE;
											}
										else {
											if(!CPass) {			// forward declaration...
							          lVCost->v=0;
												lVCost->seg=SEG_NONE;
												}
											else
												PROCError(2065,TS);
											}
									}
		              break;

		            case 'w':			// WIDTH
#if I8086
									if(tolower(TS[1]) == 'o') {		// WORD
										}
									else				// :)
#endif
									{
										struct TAGS *myT;
										FNLO(TS);
										myT=FNCercaAggr(TS,2,0);			// CERCARE ANCHE in TUTTI i membri(figli)!! ;:_$% idioti...
										if(myT) {
 			  			        o->V=0;
											o->S=1;		// direi 
											o->l.valid=TRUE;
											o->l.v=myT->size;
											o->l.seg=SEG_NONE;
						          lVCost->v=o->l.v;
											lVCost->seg=SEG_NONE;
											}
										else {
											if(!CPass) {			// forward declaration...
							          lVCost->v=0;
												lVCost->seg=SEG_NONE;
												}
											else
												PROCError(2065,TS);
											}
									}
		              break;

	              default:
	                break;
	              }
              break;

            case 3:
            case 4:
	          case 5:
	          case 6:
	          case 7:
	          case 8:
	          case 9:
	          case 10:
binaryMinus:	       
							switch(o->V) {
								case VQ_VAR:
		              l=FNGetASCII((const char*)VCost);
									break;
								case VQ_OP_VAR:
  						    l=*(NUM_TYPE *)VCost;  
//				          theLinePtr=p2;
//	              goto fine;
									break;
								default:
  						    l=*(NUM_TYPE *)VCost;  
									break;
								}
	            v=FNRev(o->op-1,&R,RCost);
							if(v<0)
								return -1;
							switch(R.V) {
								case VQ_VAR:
  								l2=FNGetASCII(RCost);
									break;
								case VQ_REGISTER:
									theLinePtr=p2;
                  if(Co>0) 
								goto fine;
									else
									return -2;

#pragma warning mettere anche altrove!

//								goto fine;


									break;
								default:
									l2=*(NUM_TYPE *)RCost;  
									break;
//					        Exit=TRUE;
//									PROCError(2062,TS);		// per le label!
//									break;
								}
//	          fprintf(stderr,"Operatore %s su %ld %ld (%d) %ld %ld (%d) \n",TS,l,*(long *)VCost,*VQ,l2,*(long *)RCost,RQ);
	            switch(tolower(*TS)) {
	              case '+':              // + unario
                  if(Co>0) 
		                l=l+l2;
		              else
		                l=l2;  
	                break;
	              case '-':
	                l=l-l2;
	                break;
	              case '*':
	                l=l*l2;
	                break;
	              case '/':
									if(!l2) {
										if(CPass)
											PROCError(2062,TS);		// per le label!
										else
											l=0;
										}
									else
										l=l/l2;
	                break;
	              case '%':
	                l=l % l2;
	                break;
	              case 's':
									if(tolower(TS[2]) == 'l')		// SHL (SHR)
										goto shift_left;
									// else continua
	              case '>':
	                l=l >> l2;
	                break;
	              case '<':
shift_left:
	                l=l << l2;
	                break;
	              case '&':
	              case 'a':
	                l=l & l2;
	                break;
	              case '|':
	              case 'o':
	                l=l | l2;
	                break;
	              case '^':
	              case 'x':
	                l=l ^ l2;
	                break;
#if I8086
	              case ':':
									if(CPULevel<3)
										l=(l << 16) | l2;
									else
										l=(l << 16) | l2;			// forse << 32 o verificare
	                break;
#endif
	              }
							{
	            lVCost->v=l;
	            lVCost->valid=TRUE;
	            lVCost->seg=SEG_UNDEFINED;
							lVCost->t=0;
//	            sprintf(VCost,"%lu",l);
	            o->S=FNGetSize(*lVCost);
							}
	            o->T=VAR_NUMBER;
	            o->V=0;
//	          fprintf(stderr,"...ritorna %ld, %ld\n",*(long *)VCost,l - l2);
              break;

            case 11:		// &&
							{
							lVCost->seg=SEG_UNDEFINED;
							switch(o->V) {
								case VQ_VAR:
		              l=FNGetASCII((const char*)VCost);
									break;
								case VQ_OP_VAR:
  						    l=*(NUM_TYPE *)VCost;  
									break;
								default:
  						    l=*(NUM_TYPE *)VCost;  
									break;
								}
							if(!*FNLA(RCost)) {
								if(Co>0) {
				          theLinePtr=p2;
									goto fine;
									}
								}

	            v=FNRev(o->op-1,&R,RCost);
							if(v<0)
								return -1;
							switch(R.V) {
								case VQ_VAR:
  								l2=FNGetASCII(RCost);
									break;
								case VQ_REGISTER:
									theLinePtr=p2;
                  if(Co>0) 
										goto fine;
									else
										return -2;
									break;
								default:
									l2=*(NUM_TYPE *)RCost;  
									break;
								}
							if(o->T & VAR_STRING || R.T & VAR_STRING)
								PROCError(2115);
							lVCost->v = (l && l2) ? 1 : 0;
							lVCost->valid=TRUE;
							lVCost->t=0;
							o->V=0;
							o->T=VAR_NUMBER;
							o->S=1;
							}
		          break;

            case 12:		// ||
							{
							lVCost->seg=SEG_UNDEFINED;
							switch(o->V) {
								case VQ_VAR:
		              l=FNGetASCII((const char*)VCost);
									break;
								case VQ_OP_VAR:
  						    l=*(NUM_TYPE *)VCost;  
									break;
								default:
  						    l=*(NUM_TYPE *)VCost;  
									break;
								}
							if(!*FNLA(RCost)) {
								if(Co>0) {
				          theLinePtr=p2;
									goto fine;
									}
								}

	            v=FNRev(o->op-1,&R,RCost);
							if(v<0)
								return -1;
							switch(R.V) {
								case VQ_VAR:
  								l2=FNGetASCII(RCost);
									break;
								case VQ_REGISTER:
									theLinePtr=p2;
                  if(Co>0) 
										goto fine;
									else
										return -2;
									break;
								default:
									l2=*(NUM_TYPE *)RCost;  
									break;
								}
							if(o->T & VAR_STRING || R.T & VAR_STRING)
								PROCError(2115);
							lVCost->v = (l || l2) ? 1 : 0;
							lVCost->valid=TRUE;
							lVCost->t=0;
							o->V=0;
							o->T=VAR_NUMBER;
							o->S=1;
							}
		          break;

            case 13:
		          break;

            case 14:		// 
							{
							lVCost->seg=SEG_UNDEFINED;
							switch(o->V) {
								case VQ_VAR:
		              l=FNGetASCII((const char*)VCost);
//									o->T=VAR_STRING;
									break;
								case VQ_OP_VAR:
  						    l=*(NUM_TYPE *)VCost;  
									break;
								case VQ_REGISTER:
									PROCError(2115);
									break;
								default:
  						    l=*(NUM_TYPE *)VCost;  
									break;
								}
							if(!*FNLA(RCost)) {
								if(Co>0) {
				          theLinePtr=p2;
									goto fine;
									}
								}

	            v=FNRev(o->op-1,&R,RCost);
							if(v<0)
								return -1;
							switch(R.V) {
								case VQ_VAR:
  								l2=FNGetASCII((const char*)RCost);
//									R.T=VAR_STRING;
									break;
								case VQ_OP_VAR:
  						    l2=*(NUM_TYPE *)RCost;  
									break;
								case VQ_REGISTER:
									PROCError(2115);
									break;
								default:
									l2=*(NUM_TYPE *)RCost;  
									break;
								}
							if(o->T & VAR_STRING && !(R.T & VAR_STRING)) {			// casto... masm fa così
								l=atoi((char*)l);
								o->T=VAR_NUMBER;
								}
							if(!(o->T & VAR_STRING) && R.T & VAR_STRING) {
								l2=atoi((char*)l2);
								R.T=VAR_NUMBER;
								}

							o->V=0;
	            switch(tolower(*TS)) {
								//GESTIRE EQ NE GT LT LE GE ... stringhe... masm; e anche INSTR https://learn.microsoft.com/en-us/cpp/assembler/masm/instr?view=msvc-170
								case '<':
									if(tolower(TS[1]) == '=')		// 
										goto lower_equal;
lower_than:
									if(o->T & VAR_STRING) {
										lVCost->v = stricmp((char*)l,(char*)l2) < 0 ? 1 : 0;
										}
									else {
										lVCost->v=l < l2 ? 1 : 0;
										}
									lVCost->valid=TRUE;
									lVCost->t=0;
									o->S=1;
									break;
								case '>':
									if(tolower(TS[1]) == '=')		// 
										goto greater_equal;
greater_than:
									if(o->T & VAR_STRING) {
										lVCost->v = stricmp((char*)l,(char*)l2) > 0 ? 1 : 0;
										}
									else {
										lVCost->v = l > l2 ? 1 : 0;
										}
									lVCost->valid=TRUE;
									lVCost->t=0;
									o->S=1;
									break;
								case 'g':
									if(tolower(TS[1]) == 't')		// 
										goto greater_than;
greater_equal:
									if(o->T & VAR_STRING) {
										lVCost->v = stricmp((char*)l,(char*)l2) >= 0 ? 1 : 0;
										}
									else {
										lVCost->v = l > l2 ? 1 : 0;
										}
									lVCost->valid=TRUE;
									lVCost->t=0;
									o->S=1;
									break;
								case 'i':			// instr
									if(o->T & VAR_STRING) {
										lVCost->v = stristr((char*)l,(char*)l2) >= 0 ? 1 : 0;
										}
									else
										PROCError(2115);
									break;
								case 'l':
									if(tolower(TS[1]) == 't')		// 
										goto lower_than;
lower_equal:
									if(o->T & VAR_STRING) {
										lVCost->v = stricmp((char*)l,(char*)l2) <= 0 ? 1 : 0;
										}
									else {
										lVCost->v = l <= l2 ? 1 : 0;
										}
									lVCost->valid=TRUE;
									lVCost->t=0;
									o->S=1;
									break;
								case 'n':
								case '!':
									if(o->T & VAR_STRING) {
										lVCost->v = stricmp((char*)l,(char*)l2) ? 1 : 0;
										}
									else {
										lVCost->v = l != l2 ? 1 : 0;
										}
									lVCost->valid=TRUE;
									lVCost->t=0;
									o->S=1;
									break;
								case 'e':
									goto is_equal;
									break;
								case '=':
									if(tolower(TS[1]) == '=')	{
is_equal:
										if(o->T & VAR_STRING) {
											lVCost->v = stricmp((char*)l,(char*)l2) ? 0 : 1;
											}
										else {
											lVCost->v = l == l2 ? 1 : 0;
											}
										lVCost->valid=TRUE;
										lVCost->t=0;
										o->S=1;
										}
									else {
										theLinePtr=p2;
										Exit=TRUE;
										}
									break;
								}
							o->T=VAR_NUMBER;
							}
		          break;

		        case 15:
//		          FNRev(14,&RQ,&RType,&RSize,&RVar,RCost);
		          break;
		        default:
		          break;
		        }
	        }
	      break;
	    case ARITM_UNKNOWN:
				PROCError(1000);
		    break;
	    }
	  Co++;
    } while(!Exit);
  
fine:
	return 0;
  }  
  
struct SEGMENTED_VALUE FNEvalString(const char *str,const struct LINE_DEF *M,void *C,char parms[10][32],uint8_t parmCnt) {
	struct OPERAND o;
	struct VARS *v;
	struct SEGMENTED_VALUE l[2];
	char TS[256],T1S[256],oper[16];
	char *p;
	uint8_t found=0;
	int i;


	char oldLine[1024];
	uint16_t oldLinePtr;
	struct SEGMENTED_VALUE ll;

	strcpy(oldLine,theLine);
	oldLinePtr=theLinePtr;
	strcpy(theLine,str);
	theLinePtr=0;
	ll=FNEvalExpr(14,TS);
	strcpy(theLine,oldLine);
	theLinePtr=oldLinePtr;

	return ll;



	do {
		if(str=parse_parm(str,TS,FALSE)) {
			if(FNSubstituteVars(M,TS,TS,parms,parmCnt)) {
				}
			if(v=FNCercaVar(TS,0,0,FALSE,CPass)) {
				found++;
				}
			else if(i=FNIsOp(TS,14)) {
				strcpy(oper,TS);

				}
			else if(isdigit(*TS)) {
				found++;
				}
		
			}

		} while(str && found<2);
/*	if(found<2 || *str)
		PROCError(2060);

	switch(FNIsOp(oper,14)) {


		}
*/
	o.l=*(struct SEGMENTED_VALUE *)C;
	*(uint64_t*)C=0;


	o.l.v=1;
	// FARE!!!

	o.l.valid=1;
	return o.l;
  }  
  
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "as.h"

// (2025 forse su registri, su var è ok  SU OPERATORI UNARI NON VA ANCORA!! ~R5 entra in loop... strano

uint8_t Brack=0;       
extern uint8_t CPULevel;
extern uint8_t inIfDef;


struct SEGMENTED_VALUE FNEvalExpr(unsigned char Pty, void *C) {
	struct OPERAND o;
	int i;

	memset(&o,0,sizeof(struct OPERAND));

  *(uint64_t*)C=0;
  i=FNRev(Pty,&o,C);
    
	if(i==0) {
		switch(o.V) {
			case ARITM_CONST:
				o.l.v=FNGetASCII((const char*)C);
//				*(uint64_t*)C=0;
				o.l.seg=SEG_UNDEFINED;
				o.l.valid=TRUE;
				break;
			case ARITM_VAR:
			default:
				o.l=*(struct SEGMENTED_VALUE *)C;
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
						struct SEGMENTED_VALUE *ll=(struct SEGMENTED_VALUE *)VCost;
            *ll=*(struct SEGMENTED_VALUE *)TS;
						ll->seg=SEG_NONE;
						ll->valid=TRUE;
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
							struct SEGMENTED_VALUE *ll=(struct SEGMENTED_VALUE *)VCost;
		          *ll=*(struct SEGMENTED_VALUE *)TS;
//							ll->seg=SEG_UNDEFINED;
							ll->valid=TRUE;
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
                  FNRev(99,o,VCost);
                  break; 
                case ')':
                  if(Pty==99) {
                    Brack--;
                    Pty=1;
                    }
                  else {
                case ']':
							      theLinePtr--;
                    Exit=TRUE;
                    }
                  break;
                case '.':
									{
										struct VARS *myV=(struct VARS*)VCost;
										myV=o->var;
									if(myV && myV->type & (VAR_STRUCT | VAR_UNION)) {
										struct TAGS *myT;
										FNLO(TS);
										if(myT=FNCercaAggr(TS,0,(uint32_t)myV->tag)) {
						          ((struct SEGMENTED_VALUE *)VCost)->v=myV->value+myT->pos;
											((struct SEGMENTED_VALUE *)VCost)->seg=SEG_UNDEFINED;


											o->S=myT->size;

											}
										else
											PROCError(2038,TS);

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
												if(((struct SEGMENTED_VALUE *)VCost)->seg)          // no operatori su segmentati (extrn, label)
													PROCError(2062);
												((struct SEGMENTED_VALUE *)VCost)->v=-((struct SEGMENTED_VALUE *)VCost)->v;  
//					            ((struct SEGMENTED_VALUE *)VCost)->seg=SEG_NONE;  
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
										;
#endif
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
											if(((struct SEGMENTED_VALUE *)VCost)->seg)          // no operatori su segmentati (extrn, label)
												PROCError(2062);
											((struct SEGMENTED_VALUE *)VCost)->v=~((struct SEGMENTED_VALUE *)VCost)->v;  
//					            ((struct SEGMENTED_VALUE *)VCost)->seg=SEG_NONE;  
											break;
										}
	    			      break;

	              case '!':
	                v=FNRev(2,o,VCost);
							if(v<0)
								return -1;
									switch(o->V) {
										case VQ_VAR:
											l=FNGetASCII((const char*)VCost);
											l=l ? 0 : -1;			// preferisco! (lui metterebbe 1
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
											if(((struct SEGMENTED_VALUE *)VCost)->seg)          // no operatori su segmentati (extrn, label)
												PROCError(2062);
											((struct SEGMENTED_VALUE *)VCost)->v=((struct SEGMENTED_VALUE *)VCost)->v ? 0 : -1;			// preferisco! (lui metterebbe 1
//					            ((struct SEGMENTED_VALUE *)VCost)->seg=SEG_NONE;  
											break;
										}
	    			      break;

	              case 's':		// sizeof
	                v=FNRev(2,o,VCost);
							if(v<0)
								return -1;
#if I8086
									if(tolower(TS[1]) == 'e') {		// SEGMENT
										switch(o->V) {
											case VQ_OP_VAR:
		  									l=*(NUM_TYPE *)VCost;  
												break;
											case VQ_VAR:		// serve davvero qua??
												l=FNGetASCII((const char*)VCost);
												break;
											default:
												PROCError(2062);
												break;
											}
					          ((struct SEGMENTED_VALUE *)VCost)->v=HIWORD(l);
										((struct SEGMENTED_VALUE *)VCost)->seg=CurSeg;
										}
									else				// :)
										;
#endif
				          ((struct SEGMENTED_VALUE *)VCost)->v=o->S;
									((struct SEGMENTED_VALUE *)VCost)->seg=SEG_UNDEFINED;
	                o->T=0;
	                o->S=2/*INT_SIZE*/;
	                o->V=0;
	                break;

	              case 'l':		// length
	                v=FNRev(2,o,VCost);
							if(v<0)
								return -1;
	                if(*(TS+1) == 'e') {			// length
										if(o->T & (VAR_STRUCT | VAR_UNION | VAR_RECORD)) {
											// gestire array... fatti con DUP v. masm
											((struct SEGMENTED_VALUE *)VCost)->v=o->S;
											}
										else {
											((struct SEGMENTED_VALUE *)VCost)->v=o->S;
											}
										((struct SEGMENTED_VALUE *)VCost)->seg=SEG_UNDEFINED;
		                o->S=2/*INT_SIZE*/;
		                }
		              else {
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
												if(((struct SEGMENTED_VALUE *)VCost)->seg)          // no operatori su segmentati (extrn, label)
													PROCError(2062);
  											l=*(NUM_TYPE *)VCost;  
												break;
											}
					          ((struct SEGMENTED_VALUE *)VCost)->v=LOBYTE(LOWORD(l));  
										((struct SEGMENTED_VALUE *)VCost)->seg=SEG_UNDEFINED;
		                o->S=1;
		                }  
	                o->T=0;
	                o->V=0;
	                break;

								case 't':			// Tipo, FARE
									PROCError(1007);
									break;

	              case 'h':
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
											if(((struct SEGMENTED_VALUE *)VCost)->seg)          // no operatori su segmentati (extrn, label)
												PROCError(2062);
  										l=*(NUM_TYPE *)VCost;  
											break;
										}
				          ((struct SEGMENTED_VALUE *)VCost)->v=HIBYTE(LOWORD(l));  
									((struct SEGMENTED_VALUE *)VCost)->seg=SEG_UNDEFINED;
	                o->S=1;
	                o->T=0;
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
		            case 'o':			// OFFSET
	                v=FNRev(2,o,VCost);
							if(v<0)
								return -1;
									switch(o->V) {
										case VQ_OP_VAR:
		  						    l=*(NUM_TYPE *)VCost;  
											break;
										case VQ_VAR:		// serve davvero qua??
		                  l=FNGetASCII((const char*)VCost);
											break;
										default:
											PROCError(2062);
											break;
										}
				          ((struct SEGMENTED_VALUE *)VCost)->v=LOWORD(l);
									((struct SEGMENTED_VALUE *)VCost)->seg=SEG_UNDEFINED;
//									if(CPULevel<3)
										o->S=PTR_SIZE;
	                o->T=0;
	                o->V=0;
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
						          ((struct SEGMENTED_VALUE *)VCost)->v=o->l.v;
											((struct SEGMENTED_VALUE *)VCost)->seg=SEG_NONE;
											}
										else {
											if(!CPass) {			// forward declaration...
							          ((struct SEGMENTED_VALUE *)VCost)->v=0;
												((struct SEGMENTED_VALUE *)VCost)->seg=SEG_NONE;
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
						          ((struct SEGMENTED_VALUE *)VCost)->v=o->l.v;
											((struct SEGMENTED_VALUE *)VCost)->seg=SEG_NONE;
											}
										else {
											if(!CPass) {			// forward declaration...
							          ((struct SEGMENTED_VALUE *)VCost)->v=0;
												((struct SEGMENTED_VALUE *)VCost)->seg=SEG_NONE;
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
									l=(l << 16) | l2;
	                break;
#endif
	              }
							{
							struct SEGMENTED_VALUE *ll=(struct SEGMENTED_VALUE *)VCost;
	            ll->v=l;
	            ll->valid=TRUE;
	            ll->seg=SEG_UNDEFINED;
//	            sprintf(VCost,"%lu",l);
	            o->S=FNGetSize(*ll);
							}
	            o->T=0;
	            o->V=0;
//	          fprintf(stderr,"...ritorna %ld, %ld\n",*(long *)VCost,l - l2);
              break;

            case 11:
            case 12:
            case 13:
		          break;

            case 14:		// =, lo lascio come delimitatore (in quei casi dove si può inizializzare al volo, v. RECORD
		          theLinePtr=p2;
							Exit=TRUE;
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
  
  
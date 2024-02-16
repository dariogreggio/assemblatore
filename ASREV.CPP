#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "as.h"

unsigned char Brack=0;       

long FNEvalExpr(unsigned char Pty, char *C) {
  signed char VQ=0;
	uint16_t S=0;
  int T=0;
  long l;
  struct VARS *V;
  
  *C=0;
  FNRev(Pty,&VQ,&T,&S,&V,C);
    
  if(VQ==1)
    return FNGetASCII(C);
  else { 
    l=*(int32_t *)C;
    *C=0;
    return l;
    }
  }
 
int FNRev(unsigned char Pty, signed char *VQ, int *VType, uint16_t *VSize, struct VARS **VVar, char *VCost) {
  int v,I,T1,Co=0,i;
	enum ARITM_ELEM T;
  signed char OP,Exit=0;
	int J2,ol;
  int RType;
	signed char RQ;
	uint16_t RSize;
  struct VARS *RVar;
  long l,l2;
  char RCost[256],AS[256],BS[256],TS[256],T1S[256],MyBuf[256],MyBuf1[256];
  char *s;
  long p,p1,OT,p2;
 
  p=ftell(FIn);      
  ol=__line__;
//  ROut=LastOut;
  do {
    p2=ftell(FIn);
    T=FNGetAritElem(TS,VQ,VVar,VType,VSize,&OP,Co);
// O% (RISULTATO) = 1 SE COSTANTE
// 2 SE VARIABILE
// 3 SE OPERANDO
// 0 SE FINE LINEA
// V>=0 se var, -1 registro, -2 condiz
// TYPE è il registro o il cond.
// SIZE è la dimensione label o reg.
    if(debug) 
      fprintf(stderr,"EVx( %d): LETTO AritmElem : %d, %s, V=%d, tipo=%d, size=%d, pty %d\n",Co,T,TS,*VVar,*VType,*VSize,Pty);

    switch(T) {
      case ARITM_NONE:
        Exit=TRUE;
        break;
      case ARITM_CONST:
        if(Co>0) {                       // per gestire due op senza separatore
          fseek(FIn,p2,SEEK_SET);
          Exit=TRUE;
          }
        else {
          if(*VQ==0)
            *(int32_t *)VCost=*(int32_t *)TS;
          else
            strcpy(VCost,TS);
          }    
//          strcpy(VCost,TS);
//        l=atol(VCost);  
        break;
      case ARITM_VAR:
        if(*VQ < 0)
          Exit=TRUE;
        else {
          if(Co>0) {                      // idem
            fseek(FIn,p2,SEEK_SET);
            Exit=TRUE;
            }
          else {
            if(*VQ==0)
              *(int32_t *)VCost=*(int32_t *)TS;
            else
              strcpy(VCost,TS);
            }
          }
//        else 
//          l=Var[*VVar].value;  
        break;
      case ARITM_OPER:
        if(OP>Pty) {
          Exit=TRUE;
          fseek(FIn,p2,SEEK_SET);
          }
        else {
          *T1S=0;
          switch(OP) {
            case 1:
              switch(*TS) {
                case '[':
                case '(':
                  Brack++;
                  FNRev(99,VQ,VType,VSize,VVar,VCost);
                  break; 
                case ']':
                case ')':
                  if(Pty==99) {
                    Brack--;
                    Pty=1;
                    }
                  else {
                    fseek(FIn,-1,SEEK_CUR);
                    Exit=TRUE;
                    }
                  break;
                case '.':
                  break;
                default:
                  break;
                }
              break;
            case 2:
              switch(*TS) {
                case '-': 
//                printf("operatore -: %d",Co);
                  if(Co>0) 
                    goto binaryMinus;
	                else {
	                  v=FNRev(2,VQ,VType,VSize,VVar,VCost);
					          if(*VQ==1) {
					            l=FNGetASCII(VCost);
					            l=-l;
					            *(((uint16_t *)&l)+1)=0;  
  	    			        ltoa(l,VCost,10);
  	    			        *VQ=0;
  	    			        }
					          else {
					            if(HIWORD(VCost))          // no operatori su segmentati (extrn, label)
					              PROCError(2062,NULL);
					            *(int32_t *)VCost=-*(int32_t *)VCost;  
					            *(((uint16_t *)VCost)+1)=0;  
					            }
	    			        }
//                printf("ritorna : %lx\n",*(long *)VCost);
	                break;
		            case '~':
		              v=FNRev(2,VQ,VType,VSize,VVar,VCost);
				          if(*VQ==1) {
                    l=FNGetASCII(VCost);
                    l=~l;
				            *(((uint16_t *)&l)+1)=0;  
 	    			        ltoa(l,VCost,10);
	      			      *VQ=0;
	      			      }
					        else {
				            if(HIWORD(VCost))          // no operatori su segmentati (extrn, label)
				              PROCError(2062,NULL);
					          *(int32_t *)VCost=~*(int32_t *)VCost;  
				            *(((uint16_t *)VCost)+1)=0;  
				            }
	    			      break;
	              case '!':
	                v=FNRev(2,VQ,VType,VSize,VVar,VCost);
				          if(*VQ==1) {
                    l=FNGetASCII(VCost);
                    l=!l;
				            *(((uint16_t *)&l)+1)=0;  
 	    			        ltoa(l,VCost,10);
	      			      *VQ=0;
	      			      }
					        else {
				            if(HIWORD(VCost))          // no operatori su segmentati (extrn, label)
				              PROCError(2062,NULL);
					          *(int32_t *)VCost=!*(int32_t *)VCost;  
				            *(((uint16_t *)VCost)+1)=0;  
					          }
	    			      break;
	              case 's':
	                v=FNRev(2,VQ,VType,VSize,VVar,VCost);
				          *(int32_t *)VCost=*(int32_t *)VSize;  
	                *VType=0;
	                *VSize=2;
	                *VQ=0;
	                break;
	              case 'l':
	                v=FNRev(2,VQ,VType,VSize,VVar,VCost);
	                if(*(TS+1) == 'e') {
					          *(int32_t *)VCost=*(int32_t *)VSize;  
		                *VSize=2;
		                }
		              else {
					          if(*VQ==1)
	                    l=FNGetASCII(VCost);
						        else {
					            if(*(((uint16_t *)VCost)+1))          // no operatori su segmentati (extrn, label)
					              PROCError(2062,NULL);
  					          l=*(int32_t *)VCost;  
					            }  
					          *(int32_t *)VCost=LOBYTE(LOWORD(l));  
		                *VSize=1;
		                }  
	                *VType=0;
	                *VQ=0;
	                break;
	              case 'h':
	                v=FNRev(2,VQ,VType,VSize,VVar,VCost);
				          if(*VQ==1)
                    l=FNGetASCII(VCost);
					        else {
				            if(*(((uint16_t *)VCost)+1))          // no operatori su segmentati (extrn, label)
				              PROCError(2062,NULL);
 					          l=*(int32_t *)VCost;  
				            }  
				          *(int32_t *)VCost=HIBYTE(LOWORD(l));  
	                *VSize=1;
	                *VType=0;
	                *VQ=0;
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
		          if(*VQ==1)
                l=FNGetASCII(VCost);
			        else
			          l=*(int32_t *)VCost;  
	            v=FNRev(OP-1,&RQ,&RType,&RSize,&RVar,RCost);
		          if(RQ==1)
  	            l2=FNGetASCII(RCost);
			        else
			          l2=*(int32_t *)RCost;  
//	          fprintf(stderr,"Operatore %s su %ld %ld (%d) %ld %ld (%d) \n",TS,l,*(long *)VCost,*VQ,l2,*(long *)RCost,RQ);
	            switch(*TS) {
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
	                l=l/l2;
	                break;
	              case '%':
	                l=l % l2;
	                break;
	              case '>':
	                l=l >> l2;
	                break;
	              case '<':
	                l=l << l2;
	                break;
	              case '&':
	                l=l & l2;
	                break;
	              case '|':
	                l=l | l2;
	                break;
	              case '^':
	                l=l ^ l2;
	                break;
	              }
	            *(int32_t *)VCost=l;
//	            sprintf(VCost,"%lu",l);
	            *VSize=FNGetSize(l);
	            *VType=0;
	            *VQ=0;
//	          fprintf(stderr,"...ritorna %ld, %ld\n",*(long *)VCost,l - l2);
              break;
            case 11:
            case 12:
            case 13:
		          break;
		        case 15:
//		          FNRev(14,&RQ,&RType,&RSize,&RVar,RCost);
		          break;
		        default:
		          break;
		        }
	        }
	      break;
	    }
	  Co++;
    } while(!Exit);
  __line__=ol;
  
  return 0;
  }  
  
long FNGetLine(long p,char *s) {
  int i;
  register int ch;
  long l,l2;
  int ol;
  
  ol=__line__;
  l=ftell(FIn);
  fseek(FIn,p,SEEK_SET);
  do {
    ch=getc(FIn);
    } while(ch != '\n' && ch!=EOF && ch<=' ');
  i=0;
  *s=0;
  while(ch != '\n' && ch!=EOF && i<128) {
    s[i++]=ch;                           
    ch=getc(FIn);
    }
  s[i]=0;  
  __line__++;
  
  l2=ftell(FIn);
  fseek(FIn,l,SEEK_SET);
  __line__=ol;
  return l2;
  }
  
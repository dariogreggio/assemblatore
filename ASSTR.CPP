#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include "as.h"

FILE *FCod,*FObj,*FMap;
static struct OBJ_STRUCT ObjStruct;
int NumExtrnUsed,PublicEQU;
long OldOffNome;
int OldSeg;
static long myPos;
static long RelocAddr,NumSegs,CodeAddr,PubAddr,ExtAddr; 
extern char defaultAbsSegm[64];
extern char buffer[256];
extern signed char CreaHex,CreaBin;
extern int LastStmt4Cycle;
extern char defaultCodeSegm[64];

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

#pragma pack( pop, before_iomat )

int subInOrd1(struct VARS *V,int j) {
  long l3;

	if(!V)
	  return j;
	j=subInOrd1(V->left,j);
  if((V->type & 0x281) == 0x281) {
    printf("scrivo public %lx a %lx...\n",V->value,ftell(FObj));
    *((int16_t *)&l3)=V->value;            // tolgo il segm.
    fwrite(&l3,4,1,FObj);
    fwrite(&OldOffNome,4,1,FObj);
    V->lpos=OldOffNome;
    OldOffNome+=strlen(V->label)+1;
    j++;
    }
	j=subInOrd1(V->right,j);
	return j;
  }

void subInOrd2(struct VARS *V) {
  
	if(!V)
    return;
  subInOrd2(V->left);
  if(V->lpos) {
//            printf("\aLPOS %lx \n",V->lpos);
    fseek(FObj,RelocAddr+V->lpos,SEEK_SET);
    strcpy(buffer,V->label);
    fputs(buffer,FObj);
    fputc(0,FObj);
//              puts(myBuf);
  	myPos=__max(myPos,ftell(FObj));
    }
  subInOrd2(V->right);
  }
  
int subInOrd3(struct VARS *V,int j) {
  long l3;
  
	if(!V)
    return j;
  j=subInOrd3(V->left,j);
    			// le proc sono automaticamente public??? 0x300 o 0x200
  if(V->type & 0x200 && !(V->type & 0x1) && *(((char *)&V->value)+2)==OldSeg) {
//    	      printf("scrivo public %lx...\n",V->value);
    *((int16_t *)&l3)=V->value;            // tolgo il segm.
    *(((int16_t *)&l3)+1)=V->type & 0x100 ? 1 : 0;     // segno le PROC
    fwrite(&l3,4,1,FObj);

//								V->lpos+=OldOffNome;
//  	            _fmemcpy(myBuf,&V->lpos,4);
//  	            fwrite(myBuf,4,1,FObj);

    fwrite(&OldOffNome,4,1,FObj);
    V->lpos=OldOffNome;
    OldOffNome+=strlen(V->label)+1;
  	          
    j++;
    }
  j=subInOrd3(V->right,j);
  
  return j;
  }
  
int subInOrd4(struct VARS *V,int j) {
  long l3;
  
	if(!V)
    return j;
  j=subInOrd4(V->left,j);
//    	      printf("guardo per ch. seg, %lx...\n",V->value);
  if(V->type & 0x400 && *(((char *)&V->value)+2)) {
//  	          l3=((Var[i].value & 0x00ff0000) >> 16) -1;            // tolgo il segm.
  	fseek(FObj,ExtAddr-8+*(((unsigned char *)&V->value)+2)*8,SEEK_SET);
		l3=0;
  	fwrite(&l3,1,1,FObj);
  	l3=1;
  	fwrite(&l3,1,1,FObj);
  	l3=0;
  	fwrite(&l3,2,1,FObj);
//  	          if(!V->lpos) {
//    	          fwrite(&OldOffNome,4,1,FObj);
//  	            V->lpos=OldOffNome;
//  	            OldOffNome+=_fstrlen(V->label)+1;
//  	            }      
//  	          else {
//								V->lpos+=myPos;
  	memcpy(buffer,&V->lpos,4);
  	fwrite(buffer,4,1,FObj);
//  	            }
  	j++;
    *(((char *)&V->value)+2)=0;  
  	}
  j=subInOrd4(V->right,j);
  return j;
  }
  
void subInOrd5(struct VARS *V) {
  
	if(!V)
    return;
  subInOrd5(V->left);
                 // guardo se c'è almeno una public EQU
//    	      printf("type %x\n",V->type);
  if(!PublicEQU) {
	  if(V->type & 0x200) {
	    if(!(V->type & 0x80)) {
	      strcpy(buffer,V->label);
	      PROCWarn(2066,buffer);
	      }
	   	if(V->type & 0x1) {
	     	PublicEQU=TRUE;
	     	PROCDclSeg(defaultAbsSegm,SEGMENT_ABS);
	     	NumSegs++;
	//    	      printf("trovato PUBLICEQU\n");
	     	}
	    } 	
    } 	
  subInOrd5(V->right);
  }
  
int subOut(int t,int t1) {
  char myBuf[128];
  static int myPtr;
	static unsigned char myChksum=0x20	/* lungh del primo record */;   // usati per hex
  static long myInizriga;      // usati per hex
  long l,l2,l3;       
  int i,j;
  struct VARS *V;
  static long NewOffNome;
  
  if(CPass) {
	  switch(t) {
	    case OUT_END:                      // end
	      if(CreaHex) {
	        if(myPtr & 0x1f) {
		  	    myChksum -= 0x20;
			 	    t=((ftell(FObj)-myInizriga) /2)-4;		// fa schifo ma ok! 2024
		  	    myChksum += (t & 0xff);
			  	  fprintf(FObj,"%02X\n",-myChksum & 0xff);
			 	    fseek(FObj,myInizriga,SEEK_SET);
			 	    fprintf(FObj,"%02X",t);
			 	    fseek(FObj,0,SEEK_END);
			  	  }
			  	else {
			  	  if(myInizriga) {
  			 	    fseek(FObj,myInizriga-1,SEEK_SET);
			  	    }
			  	  else
  			      fprintf(FObj,"\n");
			  	  }
			    fprintf(FObj,":00000001FF\n");
	        }
	      else {
//	      fseek(FObj,myPos,SEEK_SET);
				if(PublicEQU) {
  	      CurSeg++;
  	      subOut(OUT_SEGM,0);             // vado a scrivere _abs PUBLIC segment

	        l2=ftell(FObj);
	 	      l3=0;
		      Seg[OldSeg-1].lpos=ObjSeg.offNome=NewOffNome;
	        myPos=strlen(Seg[OldSeg-1].label)+1;
	        OldOffNome+=myPos;
		      fseek(FObj,RelocAddr,SEEK_SET);         // mi preparo per le public e le proc
		      PubAddr=RelocAddr;
	        j=subInOrd1(Var,0);

	 	      ObjSeg.offPublic=j ? PubAddr : 0;
	 	      ObjSeg.numPublic=j;
	 	      RelocAddr=ftell(FObj);
	 	      *((char *)&ObjSeg.tipo)=0;	// usare lobyte hibyte
	 	      *(((char *)&ObjSeg.tipo)+1)=*(((char *)&Seg[OldSeg-1].type)+1);
						/* was: impossibile scrivere type 0 o 1 per data, text: incompatibile con LMBZ80*/
					/* ora sembra andare, 2024 e cmq uso il mio linker v. */
	 	      ObjSeg.offCode=CodeAddr;
	 	      ObjSeg.lunCode=0;
	        ObjSeg.offReloc=ObjSeg.lunReloc=0l;
	 	      ObjSeg.offExtern=0;
	  	    ObjSeg.numExtern=NumExtrnUsed=0;
	 	      fseek(FObj,sizeof(OBJ_HEADER)+sizeof(OBJ_SEG_DEF)*(NumSegs-1),SEEK_SET);
	 	      fwrite(&ObjSeg,sizeof(OBJ_SEG_DEF),1,FObj);
	 	      NumExtrnUsed=0;  
	 	      NewOffNome=OldOffNome;
	 	      }
				else {
// tolto 2024
					subOut(OUT_SEGM,-1);             // fittizio per scrivere l'ultimo seg
				}
  
 	      myPos=RelocAddr;
	      ObjHeader.numSegs=NumSegs;
	      ObjHeader.offNomi=myPos;
        i=0;
	      while(/*i<NumSegs-1*/ *Seg[i].label) {
  	      fseek(FObj,RelocAddr+Seg[i].lpos,SEEK_SET);         // mi preparo per il nome seg
          fputs(Seg[i].label,FObj);
//    	      printf("scrivo nSeg %s a %lx...\n",Seg[i].label,Seg[i].lpos);
          fputc(0,FObj);
  	      myPos=__max(RelocAddr,ftell(FObj));
          
	        i++;
	        }                  
//          l2=ftell(FObj)-RelocAddr;
//				myPos=ftell(FObj);
        subInOrd2(Var);

//       printf("ftell %lx, mypos %lx, relocadd %lx\n",ftell(FObj),myPos,RelocAddr);
	      ObjHeader.lunNomi=myPos-RelocAddr;
	      fseek(FObj,0,SEEK_SET);
	      fwrite(&ObjHeader,sizeof(OBJ_HEADER),1,FObj);
	      }
	      break;
	    case OUT_VAR_EXT:                      // scrive var extern e public
	      if(CreaHex) {
	        }
	      else {
	        }
	      break;
	    case OUT_SEGM:                      // cambio di segmento
	      if(CreaHex) {
// di là	        PROCWarn(1006,"CSEG/DSEG");
	        }
	      else {
					l2=ftell(FObj);
					if(NumSegs>0) {
  					l3=0;
	//  	      for(i=0; i<(NumSegs-1); i++)
	//  	        l3+=strlen(Seg[i].label)+1;
  					Seg[OldSeg-1].lpos=ObjSeg.offNome=NewOffNome;

						myPos=strlen(Seg[OldSeg-1].label)+1;
						OldOffNome+=myPos;
  					fseek(FObj,RelocAddr,SEEK_SET);         // mi preparo per le public e le proc
  					PubAddr=RelocAddr;
						j=subInOrd3(Var,0);
  					ObjSeg.offPublic=j ? PubAddr : 0;
  					ObjSeg.numPublic=j;
  					ExtAddr=ftell(FObj);
						j=subInOrd4(Var,0);
          
  					*((char *)&ObjSeg.tipo)=0;		// usare lobyte hibyte
  					*(((char *)&ObjSeg.tipo)+1)=*(((char *)&Seg[OldSeg-1].type)+1);
							/*was: impossibile scrivere type 0 o 1 per data, text: incompatibile con LMBZ80*/
							/*sembra andare, 2024, e cmq uso il mio LN ora è ok 11/2/24)*/
  					ObjSeg.offCode=CodeAddr;
						if(Seg[OldSeg-1].BaseAddr == 0xffffffff)
	  					i=Seg[OldSeg-1].len;
						else
	  					i=Seg[OldSeg-1].len-Seg[OldSeg-1].BaseAddr;
  					ObjSeg.lunCode=i;
  					if(RelocAddr-CodeAddr > i) {
  						ObjSeg.offReloc=CodeAddr+i;
  						ObjSeg.lunReloc=RelocAddr-ObjSeg.offReloc;
  						}
  					else {
  						ObjSeg.offReloc=ObjSeg.lunReloc=0l;
  						}
  					ObjSeg.offExtern=j ? ExtAddr : 0;
  					ObjSeg.numExtern=j;
	//  	      RelocAddr=ftell(FObj);
  					RelocAddr=ExtAddr+NumExtrnUsed*8;
	//	      printf("dopo relocaddr vale %lx\n",RelocAddr);
  					fseek(FObj,sizeof(OBJ_HEADER)+sizeof(OBJ_SEG_DEF)*(NumSegs-1),SEEK_SET);
  					fwrite(&ObjSeg,sizeof(OBJ_SEG_DEF),1,FObj);
						}
					if(!t1) {
						CodeAddr=RelocAddr;
						if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
							RelocAddr=CodeAddr+Seg[CurSeg-1].len;
						else
							RelocAddr=CodeAddr+Seg[CurSeg-1].len - Seg[CurSeg-1].BaseAddr;
						fseek(FObj,CodeAddr,SEEK_SET);
	//		      printf("Segmento: %x, code %lx, reloc %lx, offName %lx\n",CurSeg,CodeAddr,RelocAddr,OldOffNome);
						OldSeg=CurSeg;
						NumSegs++;
						}
 					NumExtrnUsed=0;  
 					NewOffNome=OldOffNome;
 					}
	      break;
	    case OUT_RELOC:                      // scrive reloc addr per seg
	      if(CreaHex) {
// mah 2024	       					PROCWarn(1006,"");
	        }
	      else {
					struct OBJ_RELOC or;
					l2=ftell(FObj);
					fseek(FObj,RelocAddr,SEEK_SET);
					if(t1 < 0) {                // extrn
						or.type=MAKEWORD(LOBYTE(t1) -1,1);
						}
					else {                      // norm
						or.type=t1/*CurSeg*/-1;
						}
					or.reserved=2;
					or.address=Seg[CurSeg-1].CurAddr-2;
					fwrite(&or,sizeof(struct OBJ_RELOC),1,FObj);
					RelocAddr=ftell(FObj);
					fseek(FObj,l2,SEEK_SET);
	//	      printf("Ora relocaddr vale %lx\n",RelocAddr);
					}
	      break;
	    case OUT_START:                      // start         
	      if(CreaHex) {
		 	    fputc(':',FObj);
		 	    myInizriga=ftell(FObj);
 	  		  fprintf(FObj,"00%04X00",0);
	        }
	      else {
					ObjHeader.v0=0; 
					ObjHeader.v1=1;
					ObjHeader.v2=0x10;          // per compat. con LMBZ80
					ObjHeader.res1=0;
					fwrite(&ObjHeader,sizeof(OBJ_HEADER),1,FObj);
					fwrite(&ObjSeg,sizeof(OBJ_SEG_DEF),NumSegs,FObj);
					CodeAddr=ftell(FObj);
					RelocAddr=ftell(FObj);
					NumSegs=0;
					}
	      break;
	    default:
	      if(CreaHex) {
	  	    fprintf(FObj,"%02X",t & 0xff);
	  	    myChksum += (t & 0xff);
	  	    myPtr++;
	        if(!(myPtr & 0x1f)) {
			  	  fprintf(FObj,"%02X\n",-myChksum & 0xff);
			  	  myChksum=0x20;
			 	    fseek(FObj,myInizriga,SEEK_SET);
			 	    fprintf(FObj,"%02X",0x20);
			 	    fseek(FObj,0,SEEK_END);
			 	    fputc(':',FObj);
			 	    myInizriga=ftell(FObj);
	 	  		  fprintf(FObj,"00%04X00",myPtr);
			  	  myChksum+=HIBYTE(myPtr);
			  	  myChksum+=LOBYTE(myPtr);
			 	    }
	        
//						CurSeg=PROCDclSeg("text",0);
	        }
	      else {
		      if(CurSeg<0) {
		        PROCError(2101,"");
		        }
		      t &= 0xff;
/*				  if(OutList) {
				    sprintf(myBuf," %02X",t);
		  	    PROCOut1(myBuf,NULL,TRUE);
				    }*/
		      fputc(t,FObj);
				  }
			  if(OutList) {
			    sprintf(myBuf," %02X",t);
	  	    PROCOut1(myBuf,NULL,TRUE);
			    }
			  Seg[CurSeg-1].CurAddr++;
	      break;
	    }
	  }
	else {			// CPass
	  switch(t) {                                                  
	    case OUT_END:                      // end
	      V=Var;
        while(V) {                  // guardo se c'è almeno una public EQU
//    	      printf("type %x\n",V->type);
 	        if(V->type & 0x200) {
	          if(!(V->type & 0x80)) {
	            strcpy(myBuf,V->label);
	            PROCWarn(2066,myBuf);
	            }
 	        	if(V->type & 0x1) {
 	          	PublicEQU=TRUE;
    	      	PROCDclSeg(defaultAbsSegm,SEGMENT_ABS);
    	      	NumSegs++;
//    	      printf("trovato PUBLICEQU\n");
 	          	break;
 	          	}
 	          }
 	        V=V->next;
 	        }
        break;
	    case OUT_VAR_EXT:                      // scrive var extern e public
	      break;
	    case OUT_SEGM:                      // cambio di segmento
	      NumSegs++;
	      break;
	    case OUT_RELOC:                      // scrive addr per seg
        break;
	    case OUT_START:                      // start         
        break;
      default:
			  Seg[CurSeg-1].CurAddr++;
			  myPos++;    
			  break;
			}
	  }
//  if(debug)
//    printf("--------> %s\n",myBuf);  
    
  return 0;
  }
 
int PROCOut(int A, int A1, int A2, int A3) {
  int t;
  
  if((A & 0xfff0) == 0xfff0) {
    subOut(A,0);
    }
  else {
    t=HIBYTE(A);
    if(t)
      subOut(t,0);
    subOut(LOBYTE(A),0);
    }
  if(A1 != -1) {
    subOut(A1,0);
	  }
  if(A2 != -1) {
    subOut(A2,0);
	  }
  if(A3) {
// segmento
    if(CPass) {
//    printf("\areloc: %x\n",A3);
      subOut(OUT_RELOC,A3);
      }
	  }
    
  return 0;
  }
 
int PROCOut1(const char *A, const char *A1, int m) {
  static int t;
  static int row;
  static char MyBuf[256];
  
	if(CreaHex || CreaBin) {
		if(CurSeg==-1)
			CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
    }
  if(CPass) {
//	    fprintf(stderr,"Out1: %lx %s %lx %s %d\n",A,A,A1,A1,m);
	    
	  if(!m) {
	    if(*MyBuf) {
				if(MyBuf[strlen(MyBuf)-1]=='\r')
					MyBuf[strlen(MyBuf)-1]=0;
	      if(OutCycle) {
//	          printf("\t\t%s (%d)\n",MyBuf,LastStmt4Cycle);
	        if(LastStmt4Cycle >=0) {
  	        if(LastStmt4Cycle & 0xff00)
	            fprintf(FCod,"\t{%u/%u}",LOBYTE(LastStmt4Cycle),HIBYTE(LastStmt4Cycle));
	          else
	            fprintf(FCod,"\t{%u}",LastStmt4Cycle);
	          LastStmt4Cycle=-1;
            fprintf(FCod,"\t\t%s\n",MyBuf);
	          }
	        else
	          fprintf(FCod,"\t\t%s\n",MyBuf);
	        }
	      else 
	        fprintf(FCod,"\t\t%s\n",MyBuf);
		    *MyBuf=0;
	      }
	    else
	      fputc('\n',FCod);
	    if(!row) {
        fprintf(FCod,"\n\x0c%s %s - %s - pagina %u\n\n",__date__,__time__,__title__,__page__);
        __page__++;
        row=PageLength;
        }              
      row--;
      fprintf(FCod,A);
		  if(*A != '=') {                // non label
		    fprintf(FCod,"%02X-%04X ",CurSeg, (int)(Seg[CurSeg-1].CurAddr & 0xffff));
		    }
	    if(A1) {
   	    strncpy(MyBuf,A1,254);
   	    MyBuf[255]=0;
   	    }
	    }
	  else {
			if(FCod) {
				fprintf(FCod,A);
				if(A1) {
 					fprintf(FCod,A1);
					}
				}
	    }
    }

  
//  if(debug)
//    printf("-------+> %s\n",MyBuf);  
    
  return 0;
  }

/*  **************************
    *  Assembler             *
    *        BY G.Dar        *
    *  c version on          *
    *            6/6/94      *
    *  ST 62:   15/6/97      *
    *  Z80 renewed 2/24      *
    *  6502 version	2/24     *
    **************************/
// Come ST62 accetto le direttive anche precedute dal punto. Standard SGS!


// in un istruzione DB 1,2,3,4,  ACCETTA l'ultima VIRGOLA SENZA DARE ERRORE! e mette uno '0'!! 20/2/01

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <time.h>
#include <conio.h>
#include "as.h"

char NFS[256];
char OUS[256];

struct VARS *Var, *LVars;
struct SEGS Seg[MAX_SEGS];
// char StrTag[MAX_NAME_LEN+1][200];

struct OPERANDO Op[]={
  "(",1,")",1,"[",1,"]",1,
#if I8051
  "@",1,"#",1,
#endif
//  ".",1,
  "~",2,"size",2,"length",2,"low",2,"high",2,"(",2,"!",2,"-",2,/*"dup",2,*/
  "*",3,"/",3,"%",3,
  "+",4,"-",4,
  "<<",5,">>",5,
//  "<",6,">",6,"<=",6,">=",6,
//  "==",7,"!=",7,
  "&",8,
  "^",9,
  "|",10,
  ",",15,
  NULL,0
  };         
  
// FLAGS:
signed char IgnoreCase=FALSE;
signed char BranchOpt=0;
signed char OutMap=FALSE;
signed char OutCycle=FALSE;               // riporta i cicli macchina per istr.
signed char OutVars=FALSE;
signed char OutList=FALSE;
signed char OutError=FALSE;
signed char WarnLevel=0;
signed char CreaHex=FALSE,CreaBin=FALSE;
signed char NoMacro=FALSE;
struct VARS *CurProc;
int CurSeg;
int DefCSeg,DefDSeg;
int LastStmt4Cycle=-1;                // memorizzo l'ultimo stmt per i cicli macchina
unsigned char CPass;
unsigned char bInizioRiga=FALSE,VarDeclared=FALSE;
extern int NumExtrnUsed,PublicEQU;
extern int OldSeg;
extern long OldOffNome;

char __file__[256];
char __title__[256];
int __line__,__page__;
char __date__[10];
char __time__[10];
char defaultCodeSegm[64],defaultDataSegm[64],defaultConstSegm[64],defaultBSSSegm[64],defaultAbsSegm[64];

int SX=1;          // INDICE DELLE STRUTTURE
//int OP=FALSE;          // C'E' UN OPERANDO PENDENTE
signed char debug=0;
//int PP;
//int Vars;
int Segs;
struct LINE *RootOut,*LastOut;
  // Usiamo StaticOut (ProcOut1) per il testo in output
char buffer[256];
int TAG;

char *AddExt(char *n, const char *x) {
  char *p;
  
  if(p=strchr(n,'.')) {
		strcpy(p+1,x);
		}
  else {
		strcat(n,".");
		strcat(n,x);
		}      
		
	return n;	
  }
  
int8_t subCmpMnem(struct CODE *, struct CODE *);

int PROCInit() {
  int t;
                           
#if Z80
  qsort(Mnem,67,sizeof(Mnem[1]),(int (*)(const void *, const void *))subCmpMnem);
#elif _6502
  qsort(Mnem,64,sizeof(Mnem[1]),(int (*)(const void *, const void *))subCmpMnem);
#elif I8051
  qsort(Mnem,44,sizeof(Mnem[1]),(int (*)(const void *, const void *))subCmpMnem);
#elif ST62
  qsort(Mnem,31,sizeof(Mnem[1]),(int (*)(const void *, const void *))subCmpMnem);
#endif  
  _strdate(__date__);
  t=__date__[3];
  __date__[3]=__date__[0];
  __date__[0]=t;
  t=__date__[4];
  __date__[4]=__date__[1];
  __date__[1]=t;
  _strtime(__time__);
  __page__=1;
  if(!NoMacro) {
#if ARCHI
    PROCDclVar("ARCHIMEDES",1,1,NULL);
#elif Z80
    PROCDclVar("Z80",1,1,0,NULL);
#elif _6502
    PROCDclVar("_6502",1,1,0,NULL);
#elif I8051
    PROCDclVar("I8051",1,1,0,NULL);
#elif ST62
    PROCDclVar("ST62",1,1,0,NULL);
#endif
    }
#if ARCHI
  fprintf(stderr,"\nThe G.Dar Assembler for the Archimedes on PC, (C) 1994 - Version %d.%02d\n",__VER__ / 256,__VER__ & 255);
#elif Z80
  fprintf(stderr,"\nAssemblatore di G.Dar per lo Z80 su PC, (C) 1994-2024 - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#elif _6502
  fprintf(stderr,"\nAssemblatore di G.Dar per il 6502 su PC, (C) 1994-2024 - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#elif I8051
  fprintf(stderr,"\nAssemblatore per lo 8051 su PC - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#elif ST62
  fprintf(stderr,"\nAssemblatore per ST62XX su PC - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#endif
  fprintf(stderr,"(C) Dario'a Automation 2024; ADPM Synthesis 1994-2000\n");

	strcpy(defaultCodeSegm,"TEXT");
	strcpy(defaultDataSegm,"DATA");
	strcpy(defaultBSSSegm,"BSS");		// 
	strcpy(defaultConstSegm,"CONST");		// 
	strcpy(defaultAbsSegm,"_abs");

  return 0;                        
  }

int main(int argc, char **argv) {
  int i;
  char ffc[256];                 // nome del file -Fc (listing)
  char *p,*p1;
  FILE *f;
  
  for(i=1; i<argc; i++) {
    if(*argv[i]=='-' || *argv[i]=='/') {
      switch(*(argv[i]+1)) {
        case '?':
        case 'h':
          exit(99);
          break;
        case 'b':
          BranchOpt |= 2;
          break;
        case 'B':
          BranchOpt |= 1;
          break;
        case 'd':
          debug=2;
          break;
        case 'D':
          p=strchr(argv[i],'=');
          if(p) {
            strncpy(buffer,argv[i]+2,p-argv[i]-2);
            PROCDclVar(buffer,atol(p+1),1,0,NULL);
            }   
          break;
        case 'F':
          switch(*(argv[i]+2)) {
            case 'c':
              OutList=2;
              OutVars=TRUE;
		          OutCycle=TRUE;
              break;
            default:
              goto ukswitch;
            }
          break;
        case 'l':
          OutList=2;
          OutVars=TRUE;
          break;
        case 'x':
          OutMap=TRUE;
          break;
        case 'L':
          CreaHex=TRUE;
          break;
        case 'O':
          CreaHex=CreaBin=TRUE;
          break;
        case 'M':
          switch(*(argv[i]+2)) {
            case 'l':
              IgnoreCase=0;
              break;
            case 'x':
              IgnoreCase=1;
              break;
            case 'u':
              IgnoreCase=2;
							break;
            default:
              goto ukswitch;
            }
          break;
        case 'n':
          OutVars=FALSE;
          break;
        case 'u':
          NoMacro=TRUE;
          break;
        case 'z':
          OutError=TRUE;
          break;
        case 'W':
          sscanf(argv[i]+2,"%d",&WarnLevel);
          break;
        case 'R':		// per rinominare sezioni di default... https://www.rowleydownload.co.uk/msp430/documentation/index.htm?https://www.rowleydownload.co.uk/msp430/documentation/has_directive_keep.htm
          switch(*(argv[i]+2)) {
            case 'k':
              // data seg, const seg ?? boh
//							strcpy(defaultDataSegm,argv[i]+3);
							strcpy(defaultConstSegm,argv[i]+3);
              break;
            case 'z':
              // bss seg
							strcpy(defaultBSSSegm,argv[i]+3);
              break;
            case 'i':
              // ISR irq seg
//							strcpy(defaultCodeSegm,argv[i]+3);
							break;
            default:
              goto ukswitch;
            }
          break;
        default:
ukswitch:
          PROCWarn(4002,argv[i]+1);
          break;
        }
      *argv[i]=0;
      }
    else {
      if(*argv[i]) { 
        strcpy(NFS,argv[i]);
        }
      }
    }
  
  if(!*NFS) {
    fprintf(stderr,"Sintassi:  AS <nomefile> [switches]\n");
		puts("Switch:     /W<liv. warning> ... mostra solo i warning di una certa gravita'");
//		puts("				    /V ... mostra messaggi stato");		// aggiungere??
		puts("            /B ... ottimizzazione salti");
		puts("            /b ... consente estensione salti relativi");
		puts("            /D<variabile> ... definisce la variabile");
		puts("            /R[k|z|i] ... assegna nomi ai segmenti CONST, BSS, ISR");
		puts("            /i ... genera file listing sorgente/lm");
		puts("            /Fc ... come sopra, compresi i cicli di durata delle istruzioni");
		puts("            /Ml ... considera i nomi case-sensitive");
		puts("            /Mx ... considera i nomi case-insensitive");
		puts("            /Mu ... considera i nomi case-insensitive");
		puts("            /n ... non inserisce le variabili nel file listing");
		puts("            /u ... disabilita le macro predefinite");
		puts("            /x ... crea un file cross-reference");
		puts("            /L ... crea un file HEX (monosegmento) anziche' un file oggetto");
		puts("            /O ... crea un file BIN (v. HEX)");
		puts("            /z ... mostra l'istruzione che causa un errore");
		puts("            /h,/? ... help");
		puts("Errorlevel: 0=OK, 1=Errore esecuzione, 2=Errore comandi,");
		puts("            3=Errore svolgimento, 4=Interruzione, 99 info.");
    exit(99);
    }
  if(!*OUS)
    strcpy(OUS,NFS);
  if(!strchr(NFS,'.'))
    strcat(NFS,".asm");
  fprintf(stderr,"%s - %s\n\n",__date__,NFS);
  if(!(f=fopen(FNTrasfNome(NFS),"r")))    // controllo se c'e'...
    PROCError(1013,NFS);
  else
    fclose(f);
  strcpy(__title__,NFS);

  PROCInit();
  
  PROCOut1(";",NULL,FALSE);
  UNDEFD=FALSE;

  if(CreaHex) {
	  AddExt(OUS,"hex");
	  if(!(FObj=fopen(OUS,"w"))) 
	    PROCError(1038,OUS);
    }
  else {
	  AddExt(OUS,"o");
	  if(!(FObj=fopen(OUS,"wb"))) 
	    PROCError(1037,OUS);
	  }

  if(OutList) {
    strcpy(ffc,OUS);
    AddExt(ffc,"lst");
    if(!(FCod=fopen(ffc,"w"))) 
      PROCError(1037,ffc);
    fprintf(FCod,"*** Generated by G.Dar assembler v%u.%02u on %s %s\n\n",__VER__ / 256,__VER__ & 255,__date__,__time__);
    }
  if(OutMap) {
    strcpy(ffc,OUS);
    AddExt(ffc,"map");
    if(!(FMap=fopen(ffc,"w"))) 
      PROCError(1037,ffc);
    fprintf(FMap,"*** Generated by G.Dar assembler v%u.%02u on %s %s\n\n",__VER__ / 256,__VER__ & 255,__date__,__time__);
    }
    
  CPass=0;  
  while(CPass<2) {
    fprintf(stderr,"Fase %d...\n",CPass);
    if(FIn)
			fseek(FIn,0l,SEEK_SET);
	  for(i=0; i<MAX_SEGS; i++)
	    Seg[i].CurAddr=0;
	  DefCSeg=DefDSeg=CurSeg=-1;
	  CurProc=0;
	  NumExtrnUsed=0;
//	  __line__=1;
    PROCOut(-1,-1,-1,0);
    FNLeggiFile(NFS,FObj,0);
    if(IfDefs)
      PROCError(1004,NULL);
    if(CurProc)
      PROCError(2100,NULL);
    PROCOut(-9,-1,-1,0);
	  if(!CPass) {
	    for(i=0; i<MAX_SEGS; i++)
	      Seg[i].len=Seg[i].CurAddr;
	    }
	  CPass++;
	  }

  fclose(FObj);

  if(OutList) {
    PROCV(FCod);
    fclose(FCod);
    }
  if(OutMap) {
    fclose(FMap);
    }
  if(CreaBin) {
		FILE *FBin;
		unsigned char *h;
		unsigned long bsize=0;
	  AddExt(OUS,"hex");
	  if(!(FObj=fopen(OUS,"r"))) 
	    PROCError(1038,OUS);
		h=convertHEX2BIN(FObj,&bsize);
		fclose(FObj);
		remove(OUS);
		AddExt(OUS,"bin");
		if(h) {
			if(!(FBin=fopen(OUS,"wb"))) 
				PROCError(1038,OUS);
			fwrite(h,1,bsize,FBin);
			fclose(FBin);
	    fprintf(stderr,"File binario %s scritto.\n",OUS);
			free(h);
			}
		else
			PROCError(1038,OUS);
    }
	else if(CreaHex)
    fprintf(stderr,"File HEX %s scritto.\n",OUS);
	else
	  fprintf(stderr,"File oggetto %s pronto.\n",OUS);
	fputc('\n',stderr);
    
  return 0;
  } 
      
int FNLeggiFile(char *F, FILE *FO,unsigned char level) {
  unsigned char Go=0,First=0;
  unsigned char IfDefs=0,LstIfs=0;            // per gestire le nidificazioni, metto in Lst il livello
//  FILE *FI;
  char ARGS[256];
//  struct LINE_DEF *L;

  strcpy(__file__,F);
  __line__=1;
//  printf("leggo %s\n",__file__);
  FIn=fopen(FNTrasfNome(F),"rb" /*rb*/);
  if(!FIn)
    PROCError(1013,F);

  bInizioRiga=TRUE;
	VarDeclared=FALSE;
  do {
    if(debug) {
	    FNLA(ARGS);
      printf("<<<%s>>>\n",ARGS);
      }

#if 0		// qua gestito diversamente...
	if((*ARGS=='#') && First) {
	  FNGrab(FIn,ARGS);
//      myLog->print(0,"... e poi Grab: %s\n",A);
	  if(!strcmp(ARGS,"endif")) {
//              myLog->print(0,"ENDIF: IFS %d\n",IfDefs);
	    if(!IfDefs)
	      PROCError(1020,NULL);
	    IfDefs--;
	    if(!UNDEFD || (IfDefs==(LstIfs-1))) {
		    LstIfs=IfDefs;
		    if(!IfDefs)
		      UNDEFD=FALSE;
		    }
	    }
	  else if(!strcmp(ARGS,"else")) {
	    if(!IfDefs)
	      PROCError(1019,NULL);
//          myLog->print(0,"Qui ELSE: UNDEF %d e IFS %d e LST %d\n",UNDEFD,IfDefs,LstIfs);
	    if(!UNDEFD || (IfDefs==LstIfs))
	      UNDEFD = ! UNDEFD;      
	    }
	  else if(!strcmp(ARGS,"elif")) {
		  PP=FALSE;
		  FNGetNextPre(FIn,TRUE,B);
		  PP=TRUE;
	    if(!IfDefs)
	      PROCError(1018,NULL);
	    if(!UNDEFD || (IfDefs==LstIfs)) {
		    LstIfs=IfDefs;
	      if(FNDefined(B))
	        UNDEFD=FALSE;
	      else 
	        UNDEFD=TRUE;
	      }  
	    }
	  else if(!strcmp(ARGS,"if")) {
//		      PP=FALSE;
		  FNGetNextPre(FIn,TRUE,B);
//		      PP=TRUE;
	    IfDefs++;
	    if(!UNDEFD) {
		    LstIfs=IfDefs;
		    if(EVAL(B))
		      UNDEFD=FALSE;
		    else
		      UNDEFD=TRUE;   
		    }  
	    }
	  else if(!strcmp(ARGS,"ifdef")) {
		  PP=FALSE;
		  FNGetNextPre(FIn,TRUE,B);
		  PP=TRUE;
	    IfDefs++;
	    if(!UNDEFD) {
		    LstIfs=IfDefs;
	      if(FNDefined(B))
	        UNDEFD=FALSE;
	      else
	        UNDEFD=TRUE;
	      }  
	    }
	  else if(!strcmp(ARGS,"ifndef")) {
		  PP=FALSE;
		  FNGetNextPre(FIn,TRUE,B);
		  PP=TRUE;
	    IfDefs++;
	    if(!UNDEFD) {
		    LstIfs=IfDefs;
	      if(FNDefined(B))
	        UNDEFD=TRUE;
	      else 
	        UNDEFD=FALSE;
	      }  
//          myLog->print(0,"Qui A è %s, B è %s e UNDEF %d e IFS %d\n",A,B,UNDEFD,IfDefs);
	    }
	  else if(!strcmp(ARGS,"defined")) {
	    }
	  else {
	    if(!UNDEFD) {
			  PP=FALSE;
			  FNGetNextPre(FIn,TRUE,B);
			  PP=TRUE;
	      if(!strcmp(ARGS,"include")) {
	        switch(*B) {
	          case 34:
	            break;
	          case 60:
	            break;
	          default :
	            PROCError(2012,NULL);
	            break;
	          }
	        B[strlen(B)-1]=0;  
	        if(!FNLeggiFile(B+1,FO,level+1))
						goto fine;
	        }
	      else if(!strcmp(ARGS,"define")) {
	        L=FNDefined(B);
	        if(L) {
	          PROCWarn(4005,B);
//	              New=L->next;
	          LastDef=(struct LINE_DEF *)PROCDelLista((struct LINE_DEF *)RootDef,(struct LINE_DEF *)LastDef,(struct LINE_DEF *)L);
//	              LastDef=PROCDelLista(RootDef,New);
	          }
	        FNGetNextPre(FIn,FALSE,ARGS);
	        PROCDefine(B,ARGS) ;
	        }
	      else if(!strcmp(ARGS,"undef")) {
	        L=FNDefined(B);
	        if(L) {
//	              New=L->next;
	          LastDef=(struct LINE_DEF *)PROCDelLista((struct LINE_DEF *)RootDef,(struct LINE_DEF *)LastDef,(struct LINE_DEF *)L);
//	              LastDef=PROCDelLista(RootDef,New);
	          }
	        else {
	          PROCWarn(2065,B);
	          }
	        }
	      else if(!strcmp(ARGS,"pragma")) {
	      // lo gestiamo come comando...
    	    fprintf(FO,"%s %s ",ARGS,B);
	        }
	      else if(!strcmp(ARGS,"warning")) {
    	    fprintf(FO,"%s %s ",ARGS,B);
	        }
	      else if(!strcmp(ARGS,"line")) {
	        sscanf(B,"%d",&__line__);
	        }
	      else {
	        PROCError(1021,ARGS);
	        }
	      }
	    getc(FIn);
	    }
	  }
	else {
//       myLog->print(0,"Sto per scrivere: A %s, e UNDEFD %d\n",A,UNDEFD);
	  if(!UNDEFD) {
	    fputs(A,FO);
	    }
		if(*A=='\n') {
		  First=TRUE;
		  }
		else {
		  if((*A != ' ') && (*A != 9)) {
		    First=FALSE;
		    }
		  }
	  }
#endif
		
//	    if(*ARGS) {
      if(!FNIsStmt()) {
        PROCIsDecl();
        }
//	      }
//	    else {
//	      FNLO(ARGS);
//	      }
    } while(!feof(FIn));
      
  fclose(FIn);

  return TRUE;
  }

int PROCIsDecl() {       // qua si guarda se c'è label DW(...) valore
  int v,i,ol;
  long l;
  long OldTextp,p,op2;
  char T[256],T1[256],MyBuf[128];
  
//    LastStmt4Cycle=-1;
  if(!bInizioRiga)
    PROCError(2060,NULL);
  if(UNDEFD)
    return FALSE;
  else {  
	  op2=ftell(FIn);
	  ol=__line__;
	  FNLO(T);                    // LEGGO UN ITEM
	  if(*T) {
		  OldTextp=ftell(FIn);
		  FNLO(T1);                    // LEGGO UN ITEM
		  if(*T1 == '.') {
		    FNLO(T1);       // salto il punto, (messo nelle direttive
		    }
		  if(!strcmp(T1,":")) {
		    if(OutList) {                         
		//    FNGetLine(TEXTP-strlen(T),MyBuf);
		      PROCOut1("= ",T,FALSE);
		      }
		    PROCDclVar(T,-1l,0,0,NULL);
		    }
		  else {
			  if(i=FNIsDirective(T1)) {
			    switch(i) {
			      case 0x1010:   // NO !
			      case 0x1011:   // NO! non passa mai di qua!
			      case 0x100:
			        if(OutList) {                         
			          FNGetLine(op2,MyBuf);
			          PROCOut1("= ",MyBuf,FALSE);
			          }
			        if(!CPass) {
				        if(i==0x100) {
	//		    	      *MyBuf=0;
		  		        l=FNEvalExpr(14,MyBuf);
				          PROCDclVar(T,l,1,0,NULL);
		  		        }
				        else if((i & 0xff) == 4) {
				          if(CurProc)
				            PROCError(2100,T);
				          CurProc=PROCDclVar(T,-1,0x100,0,NULL);
				          }
				        else if((i & 0xff) == 5) {
				          if(IgnoreCase)
				            strupr(T);
	//			          printf("cproc: %Fs\n",CurProc->label);  
				          if(!strcmp(T,CurProc->label)) {
					          CurProc->dim=Seg[CurSeg-1].CurAddr-CurProc->value;
				            CurProc=0;
				            }          
				          else 
				            PROCError(2103,T);
				          }       
				        }
				      else                             // vado a fine riga
				        while(*FNLO(MyBuf));  
			        break;
			      default:
					    fseek(FIn,OldTextp,SEEK_SET);
					    __line__=ol;
							if(iscsym(*T) || (*T=='$') || (*T=='?'))
					      PROCDclVar(T,-1l,0,0,NULL);
					    else
					      PROCError(2061,T);  
			        break;
			      }
			    }
			  else {         
//noDir:
			    fseek(FIn,OldTextp,SEEK_SET);
			    __line__=ol;
/*			  NO! se non segue ':' o direttiva, NON dichiaro la label! Altrimenti le istruzioni sconosciute diventano label!  
					if(iscsym(*T) || (*T=='$') || (*T=='?'))
			      PROCDclVar(T,-1l,0,0,NULL);
			    else*/
			      PROCError(2061,T);  
			    }
		    }
	    return TRUE;
		  }
	  else {
	    return FALSE;
	    }
    }              
  }
 
int subStmt(const char *s,uint16_t m,struct CODE **n) {
  register int i;
	uint16_t j;
  
//  do {
    i=0;
	  while(j=(*n)->a[i].m) {
	    if(j == m) {
	      LastStmt4Cycle=(*n)->a[i].c;
	      return i;
	      }
	    i++;  
	    }
//	  (*n)++;
//	  } while(!stricmp(s,(*n)->s));
	  
  PROCError(2061,s);                     // do subito errore!
  return -1;
  }
     
int subBranch(const char *TS, const char *T2S, struct CODE *mn, int T, int k, long l2, unsigned char c) {
  long l;
  int i;

//N.B. l'auto-estensione non funziona!!!
// infatti al primo giro gli offset valgono 0, mentre al secondo vengono risolti...
// questo impedisce di stabilire il valore per tempo.
  if(!CPass) 
    l2=Seg[CurSeg-1].CurAddr+2;           // nella pass 0 salto fittizio
  i=mn-Mnem;  
#if Z80  
  if(i != 28/*JR*/ && i != 15/*DJNZ*/) {                   // gestisco JP cond,...

	  if(CPass && i==27 && (BranchOpt & 1) && Cond[T].m<0x20) {      // solo C, NC, Z, NZ
      l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
      if(l <= 127 && l>=-128) {          // jp 00xx diventa jr xx \\ nop
		    PROCWarn(4010,T2S);
//		    printf("c vale %d, cond %x\n",c,Cond[T].m);
        PROCOut(c ? (0x20 | Cond[T].m) : 0x18,LOBYTE(LOWORD(l)),0,0);
        }
      else
        goto normBranch;
	    }
	  else {
normBranch:
      PROCOut(mn->a[k].n | (c ? Cond[T].m : 0),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));   // con segm
      }
       
    }
  else {
    l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
//	    printf("ofs salto %lx\n",l2);
    if(l > 127 || l<-128) {
      if((BranchOpt & 2) && i!=15 /* no DJNZ */) {
        if(c) {
		      PROCOut(mn->a[k].n | (c ? Cond[T ^ 1].m : 0),3,-1,0);
		      }
	      PROCOut(0xc3 /*JP*/,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));   // con segm
        PROCWarn(4011,T2S);
        }
      else
        PROCError(2063,T2S);
      }
    else
      PROCOut(mn->a[k].n | (c ? Cond[T].m : 0),LOBYTE(LOWORD(l)),-1,0);
    }
#elif _6502
  l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
//	    printf("ofs salto %lx\n",l2);
  if(l > 127 || l<-128) {
    if(BranchOpt & 2) {
      if(c) {
		    PROCOut(mn->a[k].n ^ (0x20 ^ (c ? 0x20 : 0)),3,-1,0);
		    }
	    PROCOut(0x4c /*JMP*/,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));   // con segm
      PROCWarn(4011,T2S);
      }
    else
      PROCError(2063,T2S);
    }
  else
    PROCOut(mn->a[k].n ^ (c ? 0x20 : 0),LOBYTE(LOWORD(l)),-1,0);
#elif I8051
  if(i==21 || i==22)
    PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));   // con segm
  else if(i==0 || i==3) {
    if(*(((char *)&l2)+1) & 0xf8)
      PROCError(2064,NULL);
    PROCOut(mn->a[k].n | ((*(((char *)&l2)+1) & 0x7) << 5),l2 & 255,-1,HIWORD(l2));   // con segm
    }
#elif ST62
  if(i==15 || i==16) {      // JRR, JRS, salta su val.reg.
  //NO! qua non ci arrivo, perche' ho 3 parm; v. sotto
//    l2=*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2;
//    if(l2 > 129 || l2 < -126)
//      PROCError(2063,T2S);
//    PROCOut(mn->a[k].n,l2 & 255,*(((char *)&l2)+1) & 0xff,HIWORD(l2));   // con segm
    }
  else if(i==11 || i==4) {
    PROCOut(mn->a[k].n | ((l2 & 0xf) << 4),((l2 >> 4) & 0xf) | (*(((char *)&l2)+1) & 0xf) << 4,-1,HIWORD(l2));   // con segm
    }
  else {
    l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-1);
	    printf("ofs salto %lx, curseg %d il cui addr e' %lx\n",l,CurSeg,Seg[CurSeg-1].CurAddr);
    if(l > 15 || l < -16) {
      if(BranchOpt & 2) {
        PROCOut((mn->a[k].n ^ 4) | (2 << 3),-1,-1,HIBYTE(LOWORD(l)));   // con segm
        PROCOut(0x9 | ((l2 & 0xf) << 4),((l2 >> 4) & 0xf) | (*(((char *)&l2)+1) & 0xf) << 4,-1,HIWORD(l2));   // con segm
        PROCWarn(4011,T2S);
        }
      else
        PROCError(2063,T2S);
      }
    else
      PROCOut(mn->a[k].n | ((l & 0x1f) << 3),-1,-1,HIBYTE(LOWORD(l)));   // con segm
    }
#endif
                  
  return 0;
  }

int subBytes(const char *s, int m, long l) {
  int i,j,n;
  char myBuf[256],ch;

  if(*s == '\'' || *s == '\"') {
    if(!(m & 0xf0) && (m & 0xf ) != 1)	// la stringa OK solo per byte e string/ascii
      PROCError(2064,NULL);
    j=1;
    do {
			ch=s[j];
			if(ch == '\\') {
				j++;
				switch(s[j]) {
					case 'n':
						ch='\n';
						break;
					case 'r':
						ch='\r';
						break;
					case 't':
						ch='\t';
						break;
					case 'f':
						ch='\f';
						break;
					case 'g':
						ch=7;
						break;
					case '\\':
						ch='\\';
						break;
					case 'x':
						ch=asc2hex(MAKEWORD(s[j+1],s[j+2]));
						j+=2;
						break;
					default:
						ch=s[j];
						break;
					}
				}
      PROCOut(LOBYTE(ch),-1,-1,0);      // non CASTa i caratteri a int, 7/6/97 ?? boh 2024
      j++;
      } while(s[j] != s[0]);
    n=j-1;
    if(m == 0x11 || m==0x13) {     // asciiz e ascii (ST62)
      PROCOut(0,-1,-1,0);
      n++;
      if(*FNLA(myBuf))
        PROCError(2016,NULL);
      }
    }                 
  else {
    n=m;
    switch(m) {
      case 0:                         // gestisco LABEL
        break;
      case 1:
        PROCOut(LOBYTE(l),-1,-1,*(((uint16_t *)&l)+1));       // con segm
        break;
    	case 2:
        PROCOut(LOBYTE(l),HIBYTE(l),-1,*(((uint16_t *)&l)+1));       // con segm
        break;
    	case 4:
        PROCOut(LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),-1,HIWORD(l));       // con segm
        PROCOut(LOBYTE(HIWORD(l)),HIBYTE(HIWORD(l)),-1,0);       // con segm ??
        break;
    	case 8:
        break;
      case 0x10:                    // ds
        for(i=0; i<l; i++)
          PROCOut(' ',-1,-1,0);
        n=l;
        break;
    	}
    }      
    
  return n;
  }


int8_t subCmpMnem(struct CODE *s1, struct CODE *s2) {

  return stricmp(s1->s,s2->s);
  }


int FNIsStmt() {
  int T1,n,ol;
	signed char oi;
  register int i,j,k;
  long l,l2,l3;
  signed char V1,V2;
	int T,T2,Mnn;
	uint16_t S1,S2;
	signed char Op,Op2;
  signed char Ind1,Ind2,Impl,Is2,bt;               // se indiretto, se implicito, se c'è il secondo, tipo parentesi per ind.
  char TS[256],T1S[256],T2S[256],myBuf[256],myBuf1[256];
#if ST62
  char T3S[128];        // solo l'ST ha fino a 3 operandi..
  signed char Is3,T3,Op3;
	uint16_t S3;
	int V3;
  struct VARS *v3;
#endif
  long OldTextp,p,p1;
  struct CODE mc,*mn;
  struct VARS *v1,*v2;
  
  OldTextp=ftell(FIn);
  ol=__line__;
  oi=bInizioRiga;
  FNLO(TS);
//  mn=j=0;
  
  if(!*TS)
    goto noStmt;
#if ST62
  if(*TS == '.') {
    FNLO(TS);       // salto il punto, (messo nelle direttive
    }
#endif

  mc.s=TS;
#if Z80  
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,67,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif _6502
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,64,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif I8051
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,44,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif ST62
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,31,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#endif  

  if(UNDEFD)
    goto soloIFS;
  if(mn /*j*/) {
    Mnn=mn-Mnem;
    if(Seg[CurSeg-1].type & SEGMENT_DATA)
      PROCError(2102,NULL);
	  if(OutList) {    
      FNGetLine(OldTextp,myBuf);
	    PROCOut1("> ",myBuf,FALSE);
	    }
    
#if Z80  
//		if(Mnn==0)
//			PROCWarn(1006,mn->s);
#elif _6502
		if(Mnn==11 || Mnn==39 || Mnn==40 || Mnn==43 || Mnn==44 || Mnn==56 || Mnn==59 || Mnn==60)
			PROCWarn(1006,mn->s);
#endif

    Impl=FALSE;
    Ind1=FALSE;
    Is2=FALSE;

rifoIsVar:
    p=ftell(FIn);
    ol=__line__;
    *T1S=0;
		Op=0;
    i=FNGetAritElem(T1S,&V1,&v1,&T,&S1,&Op,0);
    switch(i) {
      case 0: 
        Impl=TRUE;
        __line__=ol;
        break;
      case 1: 
isVar:
//        V1=VQ_OP_VAR;
        fseek(FIn,p,SEEK_SET);
        __line__=ol;
        l=FNEvalExpr(14,T1S);
        S1=FNGetSize(l);
        break;
      case 2:
        if(V1>=VQ_OP_VAR)
          goto isVar;
        break;
      case 3:
	      if(Op == 1) {
		      Ind1=TRUE;
		      bt=*T1S == '(' ? ')' : ']';
		      goto rifoIsVar;
		      }
		    else
		      goto isVar;
	      break;
	    }  
	  if(Ind1) {
	    FNLA(T1S);
	    if(*T1S != bt)
	      l=FNEvalExpr(14,T1S);
#if !_6502
      PROCCheck(bt);
#endif
	    }
//    fprintf(stderr,"\tEVx %lx\n",l);
    
    if(Impl) {
      k=subStmt(TS,1,&mn);
      if(k>=0) 
        PROCOut(mn->a[k].n,-1,-1,0);
      }
    else {
      FNLA(T2S);
      if(*T2S) {
        PROCCheck(',');
        Is2=TRUE;
        Ind2=FALSE;

rifoIsVar2:
		    p=ftell(FIn);
		    *T2S=0;
				Op2=0;
		    i=FNGetAritElem(T2S,&V2,&v2,&T2,&S2,&Op2,0);
		    switch(i) {
		      case 0: 
//		        Impl=TRUE;
			      PROCError(2060,NULL);          // se c'è virgola e poi [fineriga], errore!
		        break;
		      case 1: 
isVar2:
		        V2=VQ_OP_VAR;
		        fseek(FIn,p,SEEK_SET);
        __line__=ol;
		        l2=FNEvalExpr(14,T2S);
            S2=FNGetSize(l2);
		        break;
		      case 2:
		        if(V2>=VQ_OP_VAR)
		          goto isVar2;
		        break;
		      case 3:
			      if(Op2 == 1) {
				      Ind2=TRUE;
     		      bt=*T2S == '(' ? ')' : ']';
				      goto rifoIsVar2;
				      }
				    else
				      goto isVar2;
			      break;
			    }  
	  	  if(Ind2) {
		      if(*FNLA(T2S) != bt)
		        l2=FNEvalExpr(14,T2S);
	        PROCCheck(bt);
		      }
//    fprintf(stderr,"\tEVx2 %lx\n",l2);
		    }               
		    
//		    printf("GETARIT: %d, %d %d\n",Is2,Ind1,Ind2);
#if Z80
	    if(Is2) {
	      if(Ind1 && Ind2) {
          PROCError(2062,T2S);
          }
		    else if(Ind1) {
	  	    if(V1>=0) {                                 // ld (NN),
	  	      if(V2==VQ_REGISTER) {
		  	      if(S2==1) {
			  	      if(T2==0) {                          // ld (0300h),a
			            k=subStmt(TS,17,&mn);
			  	        if(Mnn==39) {                            // gestisco OUT
			  	          if(S==1) {
					            PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);
					            }
					          else 
					            PROCError(2064,T1S);
			  	          }
			  	        else {
				            PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
					          }
					        }
					      else 
					        PROCError(2062,T2S);
				        }  
				      else {
				        k=subStmt(TS,18,&mn);                               // ld (0300h),bc
				        if(k>=0) {
				          switch(Reg[T2].m) {
				            case 0:
				            case 1:
				              PROCOut(0xed43 | (Reg[T2].m << 4),LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;
				            case 2:
				              PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;
				            case 16:
				            case 17:
				              PROCOut(0xdd22 | ((Reg[T2].m & 0xf) << 13),LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;
				            case 0x13:
				              PROCOut(0xed73,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;        
				            default:
				              PROCError(2062,T2S);
				            }
				          }
				        }                    
              }
            else 
	  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {                      // ld (r16),
	  	      if(V2>=VQ_OP_VAR) {                       // ld (bc),20
	    	      if(S==1) 
  	  	        PROCError(2064,T1S);
	  	        if(S2>1/* && Reg[T].m!=0x13*/)      // solo SP?...
	  	          PROCError(2064,T1S);
		          switch(Reg[T].m) {
		            case 2:
  			          k=subStmt(TS,19,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),-1,0);
		              break;
		            case 16:
		            case 17:
  			          k=subStmt(TS,26,&mn);  
		              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),LOBYTE(LOWORD(l2)),0);
		              break;
//		            case 0x13:
//									PROCOut(0xed7b,l & 255,(l >> 8) & 255,-1);
//		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
				        } 
				      }
				    else if(V2==VQ_REGISTER) {                         // ld (bc),a
				      if(S==1) {
			  	      if(S2>1)                    // solo out(c),a..
		  	          PROCError(2064,T2S);  
		  	        k=subStmt(TS,28,&mn);                     
								PROCOut(mn->a[k].n | (Reg[T2].m << 3),-1,-1,0);  
				        }
				      else {
			  	      if(S2>1 && Reg[T].m!=0x13)                    // solo per EX (SP),..
		  	          PROCError(2064,T1S);
			          switch(Reg[T].m) {
			            case 0:
			            case 1:
			              if(Reg[T2].m==7)                          // solo ld (r16),a
			                PROCOut(0x02 | (Reg[T].m << 4),-1,-1,0);
			              else
			                PROCError(2062,T1S);
			              break;
			            case 2:
    			          k=subStmt(TS,20,&mn);  
    			          PROCOut(mn->a[k].n | Reg[T2].m,-1,-1,0);
			              break;
			            case 16:
			            case 17:
    			          k=subStmt(TS,24,&mn);  
			              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13) | Reg[T2].m,LOBYTE(LOWORD(l)),-1,0);
			              break;
			            case 0x13:
    			          switch(Reg[T2].m) {
		    	            case 2:
    			              k=subStmt(TS,30,&mn);  
    			              PROCOut(mn->a[k].n,-1,-1,0);
    			              break;
		    	            case 16:
		    	            case 17:
    			              k=subStmt(TS,30,&mn);                     // gestisco IX come HL...
      		              PROCOut(mn->a[k].n | 0xdd00 | ((Reg[T2].m & 1) << 13),-1,-1,0);
    			              break;
    			            default:
    			              PROCError(2062,T2S);
     			            }
			              break;
			            default:
			              PROCError(2062,T1S);
			              break;
			            }
					      }
				      }
				    }
				  else if(V1==VQ_CONDIZ) 
            PROCError(2062,T1S);
			    }
		    else if(Ind2) {
	  	    if(V1==VQ_CONDIZ) {                                // jr nz, 
  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {   
	  	      if(S==1) {                          // ld a,()
	  	        if(V2>=VQ_OP_VAR) { 
 			          if(T==0) {
                  k=subStmt(TS,21,&mn);  
			  	        if(Mnn==21) {                            // gestisco IN
			  	          if(S2==1)
		                  PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),-1,0);
		                else 
		                  PROCError(2064,T2S);
		                }
		              else {
		                PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		                }
		              }
 	              else
	                PROCError(2062,T1S);
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,(bc)
			  	      if(S2==1) {
			  	        if(Reg[T2].m==1 && Mnn==21)             // per IN
	                  PROCOut(0xed,0x40 | Reg[T].m << 3,-1,0);
	                else 
	                  PROCError(2062,T2S);
	                }               
	              else {
				          switch(Reg[T2].m) {
				            case 0:
				            case 1:
				              if(Reg[T].m==7)
				                PROCOut(0x0a | (Reg[T2].m << 4),-1,-1,0);
				              else
				                PROCError(2062,T1S);
				              break;
				            case 2:
				              if(Reg[T].m==7) {
  	    			          k=subStmt(TS,9,&mn);  
  	    			          PROCOut(mn->a[k].n,-1,-1,0);
  	    			          }
				              else {
  	    			          k=subStmt(TS,32,&mn);
  	    			          PROCOut(mn->a[k].n | (Reg[T].m << 3),-1,-1,0);
  	    			          }
	    			          break;
				            case 16:
				            case 17:
				              if(Reg[T].m==7) {
  	    			          k=subStmt(TS,5,&mn);  
  	    			          l3=mn->a[k].n;
  	    			          if(l3 & 0xff0000) 
  	    			            PROCOut((l3 >> 8) | ((Reg[T2].m & 0xf) << 13),LOBYTE(LOWORD(l2)),LOBYTE(LOWORD(l3)),0);
  	    			          else                                                    
  	    			            PROCOut(l3 | ((Reg[T2].m & 0xf) << 13),LOBYTE(LOWORD(l2)),-1,0);
  	    			          }
				              else {
  	    			          k=subStmt(TS,33,&mn);
  	    			          PROCOut(mn->a[k].n | ((Reg[T2].m & 0xf) << 13) | (Reg[T].m << 3),LOBYTE(LOWORD(l2)),-1,0);
  	    			          }
				              break;
				            default:
				              PROCError(2062,T1S);
				              break;
				            }
				          }
				        }
				      }                                        // ld r16,(nn)
			      else { 
		          switch(Reg[T].m) {
		            case 0:
		            case 1:
	                PROCOut(0xed4b | (Reg[T].m << 4),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            case 2:
  			          k=subStmt(TS,22,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            case 16:
		            case 17:                            // gestisco IX come HL
  			          k=subStmt(TS,22,&mn);  
		              PROCOut(mn->a[k].n | 0xdd00 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            case 0x13:
		              PROCOut(0xed7b,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
		            }
				      }
				    }
				  else if(V1>=0) {                    // bit,set,res
				    if(V2==VQ_REGISTER && S2==2 && l>=0 && l<=7) {
			        switch(Reg[T2].m) {
		            case 2:
  			          k=subStmt(TS,51,&mn);  
  			          PROCOut(mn->a[k].n | l << 3,-1,-1,0);
		              break;
		            case 16:
		            case 17:
  			          k=subStmt(TS,52,&mn);  
  			          l3=mn->a[k].n;
  			          if(l3 & 0xff0000) {
		                PROCOut((l3 >> 8) | ((Reg[T2].m & 0xf) << 13),LOBYTE(LOWORD(l2)),(LOBYTE(LOWORD(l3))) | (l << 3),0);
		                }
		              else
		                PROCOut(l3 | (l << 3) | ((Reg[T2].m & 0xf) << 13),LOBYTE(LOWORD(l2)),-1,0);
		              break;
		            default:
		              PROCError(2062,T2S);
		              break;
		            }
				      }
            else { 
              PROCError(2062,T1S);
				      }
				    }
          }
 		    else {                                       // ld r,r
//   printf("eccooo %d %d %d %d\n",V,S,V2,S2);
	  	    if(V1==VQ_CONDIZ) {                                // jr nz, 
	  	      if(V2>=VQ_OP_VAR) {
isCcond:	  	    
              k=subStmt(TS,40,&mn);
              subBranch(TS,T2S,mn,T,k,l2,TRUE);
              }
            else 
	  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {
				    if((Mnn==27 || Mnn==28 || Mnn==45 || Mnn==4) && Reg[T].m==1) {   // gestisco C cond
				      T=0x0;
				      goto isCcond;
				      }
	  	      if(S==1) {                          // ld a,20
	  	        if(V2>=VQ_OP_VAR) { 
	  	          if(S2>1) 
	  	            PROCError(2064,T2S);
	  	            
	  	          if(Reg[T].m==7) {
	 			          k=subStmt(TS,4,&mn);            // FILTRO QUELLE CHE accettano solo A
	    			      PROCOut(mn->a[k].n,(l2 & 255),-1,0);
    			        }
    			      else {
    			        k=subStmt(TS,11,&mn);   
	    			      PROCOut(mn->a[k].n | (Reg[T].m << 3),LOBYTE(LOWORD(l2)),-1,0);
    			        }
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,b
			  	      if(S==1) {
			  	        if(T==0) {                                // solo a
			  	          if(Reg[T2].m & 0x10) {
	   			            k=subStmt(TS,Reg[T2].m & 1 ? 61 : 60,&mn);  
	        			      PROCOut(mn->a[k].n,-1,-1,0);
			  	            }
			  	          else {
	   			            k=subStmt(TS,2,&mn);  
	        			      PROCOut(mn->a[k].n | Reg[T2].m,-1,-1,0);
	        			      }
        			      }
        			    else if(Reg[T].m & 0x10) {
        			      k=subStmt(TS,Reg[T].m & 1 ? 63 : 62,&mn);  
	        			    PROCOut(mn->a[k].n,-1,-1,0);
        			      }
   			          else {
   			            k=subStmt(TS,10,&mn);  
      			        PROCOut(mn->a[k].n | (Reg[T].m << 3) | Reg[T2].m,-1,-1,0);
      			        }
			            }
				        }
				      else
				        PROCError(2062,T2S);
				      }
			      else { 
		  	      if(V2>=VQ_OP_VAR) {                       // ld bc,20
		  	        switch(Reg[T].m) {
		  	          case 0:
		  	          case 1:
		  	          case 2:
		  	          case 0x13:
 			              k=subStmt(TS,23,&mn);  
 			              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 4),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l)),HIWORD(l2));    // con segm.
 			              break;
 			            case 16:
 			            case 17:
 			              PROCOut(0xdd21 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
 			              break;
 			            default:
 			              PROCError(2062,T2S);
 			              break;
 			            }  
		  	        }
		          else if(V2==VQ_REGISTER) {                // sbc hl,de
				        switch(Reg[T].m) {
			            case 3:                      // ex af
			              if(Reg[T2].m != 0x23)
			                PROCError(2062,T2S);
	  			          PROCOut(0x8,-1,-1,0);
			              break;
			            case 1:                      // ex de,hl (SOLO!)
	  			          if(Mnn==0 || Mnn==59)         // filtro via SBC,ADC...
			                PROCError(2062,T1S);
	  			          if(Reg[T2].m != 2)
			                PROCError(2062,T2S);
	  			          PROCOut(0xeb,-1,-1,0);
			              break;
			            case 2:
	  			          k=subStmt(TS,3,&mn);  
	  			          PROCOut(mn->a[k].n | (Reg[T2].m & 0xf) << 4,-1,-1,0);
			              break;
			            case 16:
			            case 17:
		                k=subStmt(TS,7,&mn); 
	  			          switch(Reg[T2].m) {
	  			            case 0x10:
	  			            case 0x11:                       // add ix,ix ecc.
	  			              if(Reg[T].m != Reg[T2].m)
	  			                PROCError(2062,T2S);
	  			              else
			                    PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13) | 0x20,-1,-1,0);
			                  break;
			                case 0:
			                case 1:
//			                case 2:                        // no hl
			                case 0x13:                       // altri
			                  PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13) | (Reg[T2].m & 0xf) << 4,-1,-1,0);  
			                  break;
			                default:
										    PROCError(2062,T2S);
										    break;
										  }
			              break;
			            case 0x13:                             // ld sp,hl
			              switch(Reg[T2].m) {
			                case 2:
										    PROCOut(0xf9,-1,-1,0);  
										    break;
										  case 16:
										  case 17:
										    PROCOut(0xddf9 | ((Reg[T2].m & 0xf) << 13),-1,-1,0);  
										    break;
										  default:
										    PROCError(2062,T2S);
										    break;
										  }
			              break;
			            default:
			              PROCError(2062,T2S);
			            }
		            }
					    else if(V2==VQ_REGISTER) {  
	  	          PROCError(2062,T2S);
				        }
				      }
				    }
				  else if(V1>=0) {                    // bit,set,res
				    if(V2==VQ_REGISTER && S2==1 && l>=0 && l<=7) {
		          k=subStmt(TS,50,&mn);  
		          PROCOut(mn->a[k].n | l << 3 | Reg[T2].m,-1,-1,0);
				      }
            else { 
              PROCError(2062,T1S);
				      }
            }
          }
			  }		// Is2
			else {                                     // 1 solo parm
		    if(V1>=0) {
		      if(Ind1) {
            PROCError(2062,TS);
			      }
			    else {
  			    if(S==1) {
  			      k=subStmt(TS,4,&mn);         // or 20
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %x, l=%lx\n",CurAddr,l);
  			        if(Mnn==28 || Mnn==15) {                 // gestisco JR e DJNZ
                  subBranch(TS,T2S,mn,T,k,l,FALSE);
  			          }
  			        else if(Mnn==58) {                              // gestisco RST
//    fprintf(stderr,"\tEVx %ld\n",l);
  			          if((l & 7) || l>0x38 || l<0)
  			            PROCError(2062,TS);
 	  		          PROCOut(mn->a[k].n | (LOBYTE(LOWORD(l))),-1,-1,0);
	  		          }
  			        else if(Mnn==20) {                              // gestisco IM
  			          if(l<0 || l>2)
  			            PROCError(2062,TS);
 	  		          PROCOut(mn->a[k].n | (l ? (l<<3)+8 : 0),-1,-1,0);
	  		          }
  			        else if(Mnn==4 || Mnn==27) {                           // gestisco CALL e JP 
 	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
	  		          }
	  		        else                                   
	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);
	  		        }
  			      }
  			    else {
  			      k=subStmt(TS,14,&mn);         // jp ea31
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
                subBranch(TS,T2S,mn,T,k,l,FALSE);           // si autogestisce CALL, JP  e JR
	  		        }
  			      }
				    }
			    }
			  else if(V1==VQ_REGISTER) {
			    if(Ind1) {
		        if(S==2) {
		          if(Reg[T].m==2) {                // jp (hl)
   			        k=subStmt(TS,9,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else if(Reg[T].m==16 || Reg[T].m==17) {
			          if(Mnn==27) {                  // gestisco jp (ix)
			            k=subStmt(TS,12,&mn);
  	              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13),-1,-1,0);
			            }
			          else {
			            k=subStmt(TS,5,&mn);
			            l3=mn->a[k].n;
			            if(l3 & 0xff0000) {
		                PROCOut(l3 >> 8 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),LOBYTE(LOWORD(l3)),0);
		                }
		              else                                     
		                PROCOut(l3 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),-1,0);
			            }
			          }
			        else 
			          PROCError(2062,TS);
			        }
		        else {
		          PROCError(2064,TS);
		          }
			      }
          else {
				    if(S==1) {
  				    if((Mnn==27 || Mnn==28 || Mnn==45) && Reg[T].m==1) {   // gestisco C cond
	  			      T=0x0;
		  		      goto isCcond2;
			  	      }
					    if(Reg[T].m<8) {
	  			      k=subStmt(TS,10,&mn);         // or B
	  			      if(k>=0) {
	  			        if(Mnn==13 || Mnn==21 || Mnn==22 || Mnn==39)           // gestisco INC,DEC,IN,OUT
		  		          PROCOut(mn->a[k].n | (Reg[T].m << 3),-1,-1,0);
		  		        else                                         
		  		          PROCOut(mn->a[k].n | Reg[T].m,-1,-1,0);
		  		        }
					      }
					    else {
					      PROCError(2062,TS);
					      }
					    }
				    else {
		          if(Reg[T].m<4) {                       // inc hl
   			        k=subStmt(TS,3,&mn);
  			        if(k>=0) 
	 		            PROCOut(mn->a[k].n | (Reg[T].m << 4),-1,-1,0);
			          }
			        else if(Reg[T].m==16) {
			          k=subStmt(TS,15,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else if(Reg[T].m==17) {
			          k=subStmt(TS,16,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else if(Reg[T].m==0x13) {               // sp
			          k=subStmt(TS,31,&mn);
  			        if(k>=0) 
  		            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else 
			          PROCError(2062,TS);
	 			      }
				    }
				  }
			  else if(V1==VQ_CONDIZ) {                    
isCcond2:
		      k=subStmt(TS,41,&mn);         // ret nz
		      if(k>=0) 
 		        PROCOut(mn->a[k].n | Cond[T].m,-1,-1,0);
			    }   
			  }
    
#elif _6502
#pragma warning FINIRE
	    if(Is2) {
	      if(V1==VQ_REGISTER)
          PROCError(2062,T1S);
	      if(Ind1 && Ind2) {
          PROCError(2062,T2S);
          }
		    else if(Ind1) {
	  	    if(V1>=0) {                                 // ld (),x (,y)
	  	      if(V2==VQ_REGISTER) {
		  	      if(S2==1) {
			  	      if(T2!=2 && T2!=3)
					        PROCError(2062,T2S);
			  	      if(T2==0) {                          // ld (0300h),a
									k=subStmt(TS,T2==2 ? 10 : 11,&mn);
			            PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);
					        }
					      else 
					        PROCError(2062,T2S);
				        }  
				      else {
	              PROCError(2062,T2S);
				        }                    
              }
            else 
	  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {                      // ld (r16),
	  	      if(V2>=VQ_OP_VAR) {                       // ld (bc),20
	    	      if(S1==1) 
  	  	        PROCError(2064,T1S);
	  	        if(S2>1/* && Reg[T].m!=0x13*/)      // solo SP?...
	  	          PROCError(2064,T1S);
		          switch(Reg[T].m) {
		            case 2:
  			          k=subStmt(TS,19,&mn);  
  			          PROCOut(mn->a[k].n,l2 & 255,-1,0);
		              break;
		            case 16:
		            case 17:
  			          k=subStmt(TS,26,&mn);  
		              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),l2 & 255,0);
		              break;
//		            case 0x13:
//									PROCOut(0xed7b,l & 255,(l >> 8) & 255,-1);
//		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
				        } 
				      }
				    else if(V2==VQ_REGISTER) {                         // ld (bc),a
				      if(S1==1) {
			  	      if(S2>1)                    // solo out(c),a..
		  	          PROCError(2064,T2S);  
		  	        k=subStmt(TS,28,&mn);                     
								PROCOut(mn->a[k].n | (Reg[T2].m << 3),-1,-1,0);  
				        }
				      else {
			  	      if(S2>1 && Reg[T].m!=0x13)                    // solo per EX (SP),..
		  	          PROCError(2064,T1S);
			          switch(Reg[T].m) {
			            case 0:
			            case 1:
			              if(Reg[T2].m==7)                          // solo ld (r16),a
			                PROCOut(0x02 | (Reg[T].m << 4),-1,-1,0);
			              else
			                PROCError(2062,T1S);
			              break;
			            case 2:
    			          k=subStmt(TS,20,&mn);  
    			          PROCOut(mn->a[k].n | Reg[T2].m,-1,-1,0);
			              break;
			            default:
			              PROCError(2062,T1S);
			              break;
			            }
					      }
				      }
				    }
				  else if(V1==VQ_CONDIZ) 
            PROCError(2062,T1S);
			    }
		    else if(Ind2) {
	  	    if(V1==VQ_CONDIZ) {                                // jr nz, 
  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {   
	  	      if(S1==1) {                          // ld a,()
	  	        if(V2>=VQ_OP_VAR) { 
 			          if(T==0) {
                  k=subStmt(TS,21,&mn);  
	                PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              }
 	              else
	                PROCError(2062,T1S);
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,(bc)
			  	      if(S2==1) {
			  	        if(Reg[T2].m==1 && Mnn==21)             // per IN
	                  PROCOut(0xed,0x40 | Reg[T].m << 3,-1,0);
	                else 
	                  PROCError(2062,T2S);
	                }               
	              else {
				          switch(Reg[T2].m) {
				            case 0:
				            case 1:
				              if(Reg[T].m==7)
				                PROCOut(0x0a | (Reg[T2].m << 4),-1,-1,0);
				              else
				                PROCError(2062,T1S);
				              break;
				            case 2:
				              if(Reg[T].m==7) {
  	    			          k=subStmt(TS,9,&mn);  
  	    			          PROCOut(mn->a[k].n,-1,-1,0);
  	    			          }
				              else {
  	    			          k=subStmt(TS,32,&mn);
  	    			          PROCOut(mn->a[k].n | (Reg[T].m << 3),-1,-1,0);
  	    			          }
	    			          break;
				            default:
				              PROCError(2062,T1S);
				              break;
				            }
				          }
				        }
				      }                                        // ld r16,(nn)
			      else { 
		          switch(Reg[T].m) {
		            case 0:
		            case 1:
	                PROCOut(0xed4b | (Reg[T].m << 4),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            case 2:
  			          k=subStmt(TS,22,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
		            }
				      }
				    }
				  else if(V1>=0) {                    // bit,set,res
				    if(V2==VQ_REGISTER && S2==2 && l>=0 && l<=7) {
			        switch(Reg[T2].m) {
		            case 2:
  			          k=subStmt(TS,51,&mn);  
  			          PROCOut(mn->a[k].n | l << 3,-1,-1,0);
		              break;
		            default:
		              PROCError(2062,T2S);
		              break;
		            }
				      }
            else { 
              PROCError(2062,T1S);
				      }
				    }
          }
 		    else {                                       // ld r,r
//   printf("eccooo %d %d %d %d\n",V,S,V2,S2);
	  	    if(V1==VQ_CONDIZ) {                                // jr nz, 
	  	      if(V2>=VQ_OP_VAR) {
isCcond:	  	    
              k=subStmt(TS,40,&mn);
              subBranch(TS,T2S,mn,T,k,l2,TRUE);
              }
            else 
	  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {
	  	      if(S1==1) {                          // ld a,20
	  	        if(V2>=VQ_OP_VAR) { 
	  	          if(S2>1) 
	  	            PROCError(2064,T2S);
	  	            
	 			        k=subStmt(TS,4,&mn);            // 
	    			    PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),-1,0);
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,b
				        PROCError(2062,T2S);
				        }
				      else
				        PROCError(2062,T2S);
				      }
			      else { 
		  	      if(V2>=VQ_OP_VAR) {                       // ld bc,20
		  	        switch(Reg[T].m) {
		  	          case 0:
		  	          case 1:
		  	          case 2:
 			              k=subStmt(TS,23,&mn);  
 			              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 4),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
 			              break;
 			            default:
 			              PROCError(2062,T2S);
 			              break;
 			            }  
		  	        }
				      }
				    }
				  else if(V1>=0) {                    // ,x ,y
						switch(S1) {
							case 1:
								if(T2!=2 && T2!=3)
		  	          PROCError(2062,T2S);
								if(T2==3)
									goto y16bit;
								k=subStmt(TS,6,&mn);		// 
	  						if(k>=0) 
	 	  		        PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);    // 
								break;
							case 2:
								if(T2!=2 && T2!=3)
		  	          PROCError(2062,T2S);
y16bit:
								k=subStmt(TS,T2==2 ? 8 : 9,&mn);
	  						if(k>=0) 
	 	  		        PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),0);    // 
								break;
							}
            }
          }
			  }			// Is2
			else {                                     // 1 solo parm
		    if(V1>=0) {
		      if(Ind1) {
						if(V1==VQ_VAR_IMM)
				      PROCError(2062,TS);
  			    k=subStmt(TS,12,&mn);         // jp (ea31)
  			    if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
							PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),0);
	  		      }
			      }
			    else {
						switch(S1) {
							case 1:
								k=subStmt(TS,V1==VQ_VAR_IMM ? 4 : 5,&mn);         // LDA #20 o 20
	  						if(k>=0) 
	 	  		        PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);    // 
								else
				          PROCError(2062,TS);
								break;
							case 2:
		  			    k=subStmt(TS,V1==VQ_VAR_IMM ? 4 : 7,&mn);         // FORZO errore se imm :)
  							if(k>=0) {
  								if(Mnn==28 || Mnn==29) {                 // gestisco JMP & JSR
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),0);
  									}
  								else if(Mnn==4 || Mnn==5 || Mnn==6 || Mnn==8 || Mnn==9 || Mnn==10 || Mnn==11 || Mnn==13 || Mnn==14) {                 // gestisco Bxx
		//    fprintf(stderr,"\tEVx %ld\n",l);
										subBranch(TS,T2S,mn,T,k,l,FALSE);
										}
  								else {                              // 
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),0);
										}
									}
								else
				          PROCError(2062,TS);
								break;
							case 4:
		  			    k=subStmt(TS,V1==VQ_VAR_IMM ? 4 : 7,&mn);         // Bxx; idem come sopra
  							if(k>=0) {
		//   fprintf(stderr,"\tCurAddr: %x, l=%lx\n",CurAddr,l);
  								if(Mnn==28 || Mnn==29) {                 // gestisco JMP & JSR
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
  									}
  								else if(Mnn==4 || Mnn==5 || Mnn==6 || Mnn==8 || Mnn==9 || Mnn==10 || Mnn==11 || Mnn==13 || Mnn==14) {                 // gestisco Bxx
		//    fprintf(stderr,"\tEVx %ld\n",l);
										subBranch(TS,T2S,mn,T,k,l,FALSE);
										}
  								else {                              // 
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
										}
									break;
	  						}
							}
				    }
			    }
			  else if(V1==VQ_REGISTER) {
			    if(Ind1) {
		        if(S1==2) {
		          if(Reg[T].m==2) {                // jp (hl)
   			        k=subStmt(TS,9,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else 
			          PROCError(2062,TS);
			        }
		        else {
		          PROCError(2064,TS);
		          }
			      }
          else {
				    if(S1==1) {
					    if(Reg[T].m<8) {
	  			      k=subStmt(TS,10,&mn);         // or B
	  			      if(k>=0) {
	  			        if(Mnn==13 || Mnn==21 || Mnn==22 || Mnn==39)           // gestisco INC,DEC,IN,OUT
		  		          PROCOut(mn->a[k].n | (Reg[T].m << 3),-1,-1,0);
		  		        else                                         
		  		          PROCOut(mn->a[k].n | Reg[T].m,-1,-1,0);
		  		        }
					      }
					    else {
					      PROCError(2062,TS);
					      }
					    }
				    else {
		          if(Reg[T].m<4) {                       // inc hl
   			        k=subStmt(TS,3,&mn);
  			        if(k>=0) 
	 		            PROCOut(mn->a[k].n | (Reg[T].m << 4),-1,-1,0);
			          }
			        else 
			          PROCError(2062,TS);
	 			      }
				    }
				  }
			  else if(V1==VQ_CONDIZ) {                    
isCcond2:
		      k=subStmt(TS,41,&mn);         // ret nz
		      if(k>=0) 
 		        PROCOut(mn->a[k].n | Cond[T].m,-1,-1,0);
			    }   
			  }
    

#elif I8051
	    if(Is2) {
	      if(Ind1 && Ind2) {
          PROCError(2062,T2S);
          }
		    else if(Ind1) {
	  	    if(V1>=0) {                                 // ld (NN),
	  	      if(V2==VQ_REGISTER) {
		  	      if(S2==1) {
			  	      if(T2==0) {                          // ld (0300h),a
			            k=subStmt(TS,17,&mn);
			  	        if(Mnn==39) {                            // gestisco OUT
			  	          if(S==1) {
					            PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);
					            }
					          else 
					            PROCError(2064,T1S);
			  	          }
			  	        else {
				            PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
					          }
					        }
					      else 
					        PROCError(2062,T2S);
				        }  
				      else {
				        k=subStmt(TS,18,&mn);                               // ld (0300h),bc
				        if(k>=0) {
				          switch(Reg[T2].m) {
				            case 0:
				            case 1:
				              PROCOut(0xed43 | (Reg[T2].m << 4),LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;
				            case 2:
				              PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;
				            case 16:
				            case 17:
				              PROCOut(0xdd22 | ((Reg[T2].m & 0xf) << 13),LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;
				            case 0x13:
				              PROCOut(0xed73,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
				              break;        
				            default:
				              PROCError(2062,T2S);
				            }
				          }
				        }                    
              }
            else 
	  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {                      // ld (r16),
	  	      if(V2>=VQ_OP_VAR) {                       // ld (bc),20
	    	      if(S==1) 
  	  	        PROCError(2064,T1S);
	  	        if(S2>1/* && Reg[T].m!=0x13*/)      // solo SP?...
	  	          PROCError(2064,T1S);
		          switch(Reg[T].m) {
		            case 2:
  			          k=subStmt(TS,19,&mn);  
  			          PROCOut(mn->a[k].n,l2 & 255,-1,0);
		              break;
		            case 16:
		            case 17:
  			          k=subStmt(TS,26,&mn);  
		              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),l2 & 255,0);
		              break;
//		            case 0x13:
//									PROCOut(0xed7b,l & 255,(l >> 8) & 255,-1);
//		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
				        } 
				      }
				    else if(V2==VQ_REGISTER) {                         // ld (bc),a
				      if(S==1) {
			  	      if(S2>1)                    // solo out(c),a..
		  	          PROCError(2064,T2S);  
		  	        k=subStmt(TS,28,&mn);                     
								PROCOut(mn->a[k].n | (Reg[T2].m << 3),-1,-1,0);  
				        }
				      else {
			  	      if(S2>1 && Reg[T].m!=0x13)                    // solo per EX (SP),..
		  	          PROCError(2064,T1S);
			          switch(Reg[T].m) {
			            case 0:
			            case 1:
			              if(Reg[T2].m==7)                          // solo ld (r16),a
			                PROCOut(0x02 | (Reg[T].m << 4),-1,-1,0);
			              else
			                PROCError(2062,T1S);
			              break;
			            case 2:
    			          k=subStmt(TS,20,&mn);  
    			          PROCOut(mn->a[k].n | Reg[T2].m,-1,-1,0);
			              break;
			            case 16:
			            case 17:
    			          k=subStmt(TS,24,&mn);  
			              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13) | Reg[T2].m,LOBYTE(LOWORD(l)),-1,0);
			              break;
			            case 0x13:
    			          switch(Reg[T2].m) {
		    	            case 2:
    			              k=subStmt(TS,30,&mn);  
    			              PROCOut(mn->a[k].n,-1,-1,0);
    			              break;
		    	            case 16:
		    	            case 17:
    			              k=subStmt(TS,30,&mn);                     // gestisco IX come HL...
      		              PROCOut(mn->a[k].n | 0xdd00 | ((Reg[T2].m & 1) << 13),-1,-1,0);
    			              break;
    			            default:
    			              PROCError(2062,T2S);
     			            }
			              break;
			            default:
			              PROCError(2062,T1S);
			              break;
			            }
					      }
				      }
				    }
			    }
		    else if(Ind2) {
				  if(V1==VQ_REGISTER) {   
	  	      if(S==1) {                          // ld a,()
	  	        if(V2>=VQ_OP_VAR) { 
 			          if(T==0) {
                  k=subStmt(TS,21,&mn);  
			  	        if(Mnn==21) {                            // gestisco IN
			  	          if(S2==1)
		                  PROCOut(mn->a[k].n,l2 & 255,-1,0);
		                else 
		                  PROCError(2064,T2S);
		                }
		              else {
		                PROCOut(mn->a[k].n,l2 & 255,HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		                }
		              }
 	              else
	                PROCError(2062,T1S);
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,(bc)
			  	      if(S2==1) {
			  	        if(Reg[T2].m==1 && Mnn==21)             // per IN
	                  PROCOut(0xed,0x40 | Reg[T].m << 3,-1,0);
	                else 
	                  PROCError(2062,T2S);
	                }               
	              else {
				          switch(Reg[T2].m) {
				            case 0:
				            case 1:
				              if(Reg[T].m==7)
				                PROCOut(0x0a | (Reg[T2].m << 4),-1,-1,0);
				              else
				                PROCError(2062,T1S);
				              break;
				            case 2:
				              if(Reg[T].m==7) {
  	    			          k=subStmt(TS,9,&mn);  
  	    			          PROCOut(mn->a[k].n,-1,-1,0);
  	    			          }
				              else {
  	    			          k=subStmt(TS,32,&mn);
  	    			          PROCOut(mn->a[k].n | (Reg[T].m << 3),-1,-1,0);
  	    			          }
	    			          break;
				            case 16:
				            case 17:
				              if(Reg[T].m==7) {
  	    			          k=subStmt(TS,5,&mn);  
  	    			          l3=mn->a[k].n;
  	    			          if(l3 & 0xff0000) 
  	    			            PROCOut((l3 >> 8) | ((Reg[T2].m & 0xf) << 13),l2 & 255,LOBYTE(LOWORD(l3)),0);
  	    			          else                                                    
  	    			            PROCOut(l3 | ((Reg[T2].m & 0xf) << 13),l2 & 255,-1,0);
  	    			          }
				              else {
  	    			          k=subStmt(TS,33,&mn);
  	    			          PROCOut(mn->a[k].n | ((Reg[T2].m & 0xf) << 13) | (Reg[T].m << 3),l2 & 255,-1,0);
  	    			          }
				              break;
				            default:
				              PROCError(2062,T1S);
				              break;
				            }
				          }
				        }
				      }                                        // ld r16,(nn)
			      else { 
		          switch(Reg[T].m) {
		            case 0:
		            case 1:
	                PROCOut(0xed4b | (Reg[T].m << 4),l2 & 255,HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            case 2:
  			          k=subStmt(TS,22,&mn);  
  			          PROCOut(mn->a[k].n,l2 & 255,HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            case 16:
		            case 17:                            // gestisco IX come HL
  			          k=subStmt(TS,22,&mn);  
		              PROCOut(mn->a[k].n | 0xdd00 | ((Reg[T].m & 0xf) << 13),l2 & 255,HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            case 0x13:
		              PROCOut(0xed7b,l2 & 255,HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
		            }
				      }
				    }
				  else if(V1>=0) {                    // bit,set,res
				    if(V2==VQ_REGISTER && S2==2 && l>=0 && l<=7) {
			        switch(Reg[T2].m) {
		            case 2:
  			          k=subStmt(TS,51,&mn);  
  			          PROCOut(mn->a[k].n | l << 3,-1,-1,0);
		              break;
		            case 16:
		            case 17:
  			          k=subStmt(TS,52,&mn);  
  			          l3=mn->a[k].n;
  			          if(l3 & 0xff0000) {
		                PROCOut((l3 >> 8) | ((Reg[T2].m & 0xf) << 13),l2 & 255,(LOBYTE(LOWORD(l3))) | (l << 3),0);
		                }
		              else
		                PROCOut(l3 | (l << 3) | ((Reg[T2].m & 0xf) << 13),l2 & 255,-1,0);
		              break;
		            default:
		              PROCError(2062,T2S);
		              break;
		            }
				      }
            else { 
              PROCError(2062,T1S);
				      }
				    }
          }
 		    else {                                       // ld r,r
   printf("eccooo %d %d %d %d\n",V1,S,V2,S2);
				  if(V1==VQ_REGISTER) {
	  	      if(S==1) {                          // mov r0,20
	  	        if(V2>=VQ_OP_VAR) { 
	  	          if(S2>1) 
	  	            PROCError(2064,T2S);
	  	            
	  	          if(Reg[T].m==0) {
	 			          k=subStmt(TS,4,&mn);            // FILTRO QUELLE CHE accettano solo A
	    			      PROCOut(mn->a[k].n,(l2 & 255),-1,0);
    			        }
    			      else {
    			        k=subStmt(TS,11,&mn);   
	    			      PROCOut(mn->a[k].n | (Reg[T].m & 0xf),(l2 & 255),-1,0);
    			        }
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,b
			  	      if(S==1) {
			  	        if(T==0) {                                // solo a
			  	          if(Reg[T2].m & 0x10) {
	   			            k=subStmt(TS,Reg[T2].m & 1 ? 61 : 60,&mn);  
	        			      PROCOut(mn->a[k].n,-1,-1,0);
			  	            }
			  	          else {
	   			            k=subStmt(TS,2,&mn);  
	        			      PROCOut(mn->a[k].n | Reg[T2].m,-1,-1,0);
	        			      }
        			      }
        			    else if(Reg[T].m & 0x10) {
        			      k=subStmt(TS,Reg[T].m & 1 ? 63 : 62,&mn);  
	        			    PROCOut(mn->a[k].n,-1,-1,0);
        			      }
   			          else {
   			            k=subStmt(TS,10,&mn);  
      			        PROCOut(mn->a[k].n | (Reg[T].m << 3) | Reg[T2].m,-1,-1,0);
      			        }
			            }
				        }
				      else
				        PROCError(2062,T2S);
				      }
			      else { 
		  	      if(V2>=VQ_OP_VAR) {                       // ld bc,20
		  	        switch(Reg[T].m) {
		  	          case 0:
		  	          case 1:
		  	          case 2:
		  	          case 0x13:
 			              k=subStmt(TS,23,&mn);  
 			              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 4),l2 & 255,HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
 			              break;
 			            case 16:
 			            case 17:
 			              PROCOut(0xdd21 | ((Reg[T].m & 0xf) << 13),l2 & 255,HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
 			              break;
 			            default:
 			              PROCError(2062,T2S);
 			              break;
 			            }  
		  	        }
		          else if(V2==VQ_REGISTER) {                // sbc hl,de
				        switch(Reg[T].m) {
			            case 3:                      // ex af
			              if(Reg[T2].m != 0x23)
			                PROCError(2062,T2S);
	  			          PROCOut(0x8,-1,-1,0);
			              break;
			            case 1:                      // ex de
	  			          if(Reg[T2].m != 2)
			                PROCError(2062,T2S);
	  			          PROCOut(0xeb,-1,-1,0);
			              break;
			            case 2:
	  			          k=subStmt(TS,3,&mn);  
	  			          PROCOut(mn->a[k].n | (Reg[T2].m & 0xf) << 4,-1,-1,0);
			              break;
			            case 16:
			            case 17:
		                k=subStmt(TS,7,&mn); 
	  			          switch(Reg[T2].m) {
	  			            case 0x10:
	  			            case 0x11:                       // add ix,ix ecc.
	  			              if(Reg[T].m != Reg[T2].m)
	  			                PROCError(2062,T2S);
	  			              else
			                    PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13) | 0x20,-1,-1,0);
			                  break;
			                case 0:
			                case 1:
//			                case 2:                        // no hl
			                case 0x13:                       // altri
			                  PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13) | (Reg[T2].m & 0xf) << 4,-1,-1,0);  
			                  break;
			                default:
										    PROCError(2062,T2S);
										    break;
										  }
			              break;
			            case 0x13:                             // ld sp,hl
			              switch(Reg[T2].m) {
			                case 2:
										    PROCOut(0xf9,-1,-1,0);  
										    break;
										  case 16:
										  case 17:
										    PROCOut(0xddf9 | ((Reg[T2].m & 0xf) << 13),-1,-1,0);  
										    break;
										  default:
										    PROCError(2062,T2S);
										    break;
										  }
			              break;
			            default:
			              PROCError(2062,T2S);
			            }
		            }
				      }
				    }
				  else if(V1>=0) {                    // bit,set,res
				    if(V2==VQ_REGISTER && S2==1 && l>=0 && l<=7) {
		          k=subStmt(TS,50,&mn);  
		          PROCOut(mn->a[k].n | l << 3 | Reg[T2].m,-1,-1,0);
				      }
            else { 
              PROCError(2062,T1S);
				      }
            }
          }
			  }		// Is2
			else {                                     // 1 solo parm
		    if(V1>=0) {
		      if(Ind1) {
            PROCError(2062,TS);
			      }
			    else {
  			    if(S==1) {
  			      k=subStmt(TS,4,&mn);         // or 20
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %x, l=%lx\n",CurAddr,l);
  			        if(Mnn==28 || Mnn==15) {                 // gestisco JR e DJNZ
                  subBranch(TS,T2S,mn,T,k,l,FALSE);
  			          }
  			        else if(Mnn==58) {                              // gestisco RST
//    fprintf(stderr,"\tEVx %ld\n",l);
  			          if((l & 7) || l>0x38 || l<0)
  			            PROCError(2062,TS);
 	  		          PROCOut(mn->a[k].n | (LOBYTE(LOWORD(l))),-1,-1,0);
	  		          }
  			        else if(Mnn==20) {                              // gestisco IM
  			          if(l<0 || l>2)
  			            PROCError(2062,TS);
 	  		          PROCOut(mn->a[k].n | (l ? (l<<3)+8 : 0),-1,-1,0);
	  		          }
  			        else if(Mnn==4 || Mnn==27) {                           // gestisco CALL e JP 
 	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIWORD(l));    // con segm.
	  		          }
	  		        else                                   
	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);
	  		        }
  			      }
  			    else {
  			      k=subStmt(TS,14,&mn);         // jp ea31
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
                subBranch(TS,T2S,mn,T,k,l,FALSE);           // si autogestisce CALL, JP  e JR
	  		        }
  			      }
				    }
			    }
			  else if(V1==VQ_REGISTER) {
			    if(Ind1) {
		        if(S==2) {
		          if(Reg[T].m==2) {                // jp (hl)
   			        k=subStmt(TS,9,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else if(Reg[T].m==16 || Reg[T].m==17) {
			          if(Mnn==27) {                  // gestisco jp (ix)
			            k=subStmt(TS,12,&mn);
  	              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13),-1,-1,0);
			            }
			          else {
			            k=subStmt(TS,5,&mn);
			            l3=mn->a[k].n;
			            if(l3 & 0xff0000) {
		                PROCOut(l3 >> 8 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),LOBYTE(LOWORD(l3)),0);
		                }
		              else                                     
		                PROCOut(l3 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),-1,0);
			            }
			          }
			        else 
			          PROCError(2062,TS);
			        }
		        else {
		          PROCError(2064,TS);
		          }
			      }
          else {
				    if(S==1) {
					    if(Reg[T].m<8) {
	  			      k=subStmt(TS,10,&mn);         // or B
	  			      if(k>=0) {
	  			        if(Mnn==13 || Mnn==21 || Mnn==22 || Mnn==39)           // gestisco INC,DEC,IN,OUT
		  		          PROCOut(mn->a[k].n | (Reg[T].m << 3),-1,-1,0);
		  		        else                                         
		  		          PROCOut(mn->a[k].n | Reg[T].m,-1,-1,0);
		  		        }
					      }
					    else {
					      PROCError(2062,TS);
					      }
					    }
				    else {
		          if(Reg[T].m<4) {                       // inc hl
   			        k=subStmt(TS,3,&mn);
  			        if(k>=0) 
	 		            PROCOut(mn->a[k].n | (Reg[T].m << 4),-1,-1,0);
			          }
			        else if(Reg[T].m==16) {
			          k=subStmt(TS,15,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else if(Reg[T].m==17) {
			          k=subStmt(TS,16,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else if(Reg[T].m==0x13) {               // sp
			          k=subStmt(TS,31,&mn);
  			        if(k>=0) 
  		            PROCOut(mn->a[k].n,-1,-1,0);
			          }
			        else 
			          PROCError(2062,TS);
	 			      }
				    }
				  }
			  }
#elif ST62
	    if(Is2) {
	      if(Ind1 && Ind2) {
          PROCError(2062,T2S);
          }
		    else if(Ind1) {
	  	    if(V1>=0) {                                 // ld (NN),
  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {                      // ld (r8),
	  	      if(V2>=VQ_OP_VAR) {                       // ld (r8),20
  	          PROCError(2064,T2S);  
				      }
				    else if(V2==VQ_REGISTER) {                         // ld (bc),a
				      if(S==1) {
			  	      if(S2>1 || Reg[T2].m)  // solo ,A
		  	          PROCError(2064,T2S);  
			          switch(Reg[T].m) {
			            case 1:
	  			          k=subStmt(TS,24,&mn);
	  			          PROCOut(mn->a[k].n,-1,-1,0);
			              break;
			            case 2:
	  			          k=subStmt(TS,25,&mn);
	  			          PROCOut(mn->a[k].n,-1,-1,0);
			              break;
			            default:
			              PROCError(2062,T1S);
			              break;
					        } 
				        }
				      else {
	              PROCError(2062,T1S);
					      }
				      }
				    }
			    }
		    else if(Ind2) {
				  if(V1==VQ_REGISTER) {   
	  	      if(S==1) {                          // ld a,()
	  	        if(V2>=VQ_OP_VAR) {
                PROCError(2062,T1S);
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,(bc)
			  	      if(S2==1) {
				          switch(Reg[T2].m) {
				            case 1:
		  			          k=subStmt(TS,5,&mn);
		  			          PROCOut(mn->a[k].n,-1,-1,0);
				              break;
				            case 2:
		  			          k=subStmt(TS,6,&mn);
		  			          PROCOut(mn->a[k].n,-1,-1,0);
				              break;
				            default:
				              PROCError(2062,T1S);
				              break;
						        } 
	                }               
	              else {
                  PROCError(2062,T2S);
				          }
				        }
				      }                                        // ld r16,(nn)
			      else { 
              PROCError(2062,T2S);
				      }
				    }
				  else if(V1>=0) {                    // ld a,(nn)
            PROCError(2062,T1S);
				    }
          }
 		    else {                                       // ld r,r
//   printf("eccooo R,R %d %d %d %d (Mnn=%d)\n",V,S,V2,S2,Mnn);
				  if(V1==VQ_REGISTER) {
	  	      if(S==1) {                          // ld v,20
	  	        if(V2>=VQ_OP_VAR) { 
	  	          if(S2>1) 
	  	            PROCError(2064,T2S);
	  	            
	  	          if(Reg[T].m==0) {
	 			          k=subStmt(TS,4,&mn);
	    			      PROCOut(mn->a[k].n,(l2 & 255),-1,0);
    			        }
    			      else {
	 			          k=subStmt(TS,3,&mn);
	    			      PROCOut(mn->a[k].n,0x80 | (Reg[T].m -1),(l2 & 255),0);
    			        }
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,b
			  	      if(S==1) {
			  	        if(Mnn == 18) {    // LD
				  	        if(T==0) {                                // solo a
	  		  	          if(T2==0)
	  		  	            PROCError(2062,T2S);
	   			            k=subStmt(TS,2,&mn);  
	        			      PROCOut(mn->a[k].n | ((Reg[T2].m - 1) << 6),-1,-1,0);
	        			      }
	        			    else {
	   			            k=subStmt(TS,10,&mn);  
	        			      PROCOut(mn->a[k].n | ((Reg[T].m - 1) << 6),-1,-1,0);
	        			      }
	        			    }
	        			  else {
				  	        if(T==0) {                                // solo a
	   			            k=subStmt(TS,2,&mn);  
				  	          if(T2==0)
	        			        PROCOut(mn->a[k].n,0xff,-1,0);
				  	          else
  	        			      PROCOut(mn->a[k].n,0x80 | (Reg[T2].m - 1),-1,0);
	        			      }
	        			    else {
	   			            PROCError(2062,T1S);
	        			      }
	          		    }
			            }
				        }
				      else
				        PROCError(2062,T2S);
				      }
			      else { 
              PROCError(2062,T2S);
				      }
				    }
				  else if(V1>=0) {                    // ld r,n  opp. (rr),n
				    if(S==1 && S2==1) {
				      if(*FNLA(T3S)) {        // jrs,jrr
				        PROCCheck(',');
				        *T3S=0;
						    p=ftell(FIn);
						    i=FNGetAritElem(T3S,&V3,&v3,&T3,&S3,&Op3,0);
						    if(V3>=0) {
					        V3=0;
					        fseek(FIn,p,SEEK_SET);
        __line__=ol;
					        l3=FNEvalExpr(14,T1S);
					        S3=FNGetSize(l3);
					        }
					      else
                  PROCError(2062,T3S);
						    if(l < 0 || l > 7)
						      PROCError(2063,T1S);
			          k=subStmt(TS,40,&mn);
    						l3=*((int16_t *)&l3)-Seg[CurSeg-1].CurAddr-3;
//						    if(l2 > 127 || l2 < -128)
//						      PROCError(2063,T2S);
			          if(l & 4)
			            l|=0x20;
			          if(l & 2)
			            l|=0x40;
			          if(l & 1)
			            l|=0x80;
			          l &= 0xe0;     // i bit sono invertiti...
						    if(V2==VQ_REGISTER) {
						      if(T2 == 0)
			              PROCOut(mn->a[k].n | l,0xff,l3,0);   // A
			            else
			              PROCOut(mn->a[k].n | l,0x80 | (Reg[T2].m - 1),l3,0);   // x,y,v,w
			            }
			          else
			            PROCOut(mn->a[k].n | l,LOBYTE(LOWORD(l2)),l3,0);
				        }
				      else {
				        if(Mnn==21 || Mnn==25) {  // gestisco set e res
				          if(l & 4)
				            l|=0x20;
				          if(l & 2)
				            l|=0x40;
				          if(l & 1)
				            l|=0x80;
				          l &= 0xe0;     // i bit sono invertiti...
		  				    if(V2==VQ_REGISTER) {
					          k=subStmt(TS,50,&mn);  
					          PROCOut(mn->a[k].n | l,0x80 | (Reg[T].m - 1),-1,0);
					          }
					        else {
					          k=subStmt(TS,50,&mn);  
					          PROCOut(mn->a[k].n | l,LOBYTE(LOWORD(l2)),-1,0);
					          }
				          }
				        else {
		  				    if(V2==VQ_REGISTER) {
		  				      if(Mnn != 19 && T2 !=0)      // a parte LDI, solo A puo' essere copiato in un indirizzo
		  				        PROCError(2062,T2S);
					          k=subStmt(TS,17,&mn);  
					          PROCOut(mn->a[k].n,l,-1,0);
					          }
					        else {
		  				      if(Mnn != 19 && T2 !=0)      // a parte LDI, solo A puo' essere copiato in un indirizzo
		  				        PROCError(2062,T2S);
					          k=subStmt(TS,11,&mn);  
					          PROCOut(mn->a[k].n,l,LOBYTE(LOWORD(l2)),0);
					          }
					        }
				        }  
				      }
            else { 
              PROCError(2062,T1S);
				      }
            }
          }
			  }		// Is2
			else {                                     // 1 solo parm
		    if(V1>=0) {
		      if(Ind1) {
		        goto nonInd1;   // accetto op. anche tra parentesi...
			      }
			    else {
nonInd1:			    
//   fprintf(stderr,"\tCurAddr: l=%lx; S=%d\n",l,S);
  			    if(S==1) {
  			      k=subStmt(TS,4,&mn);         // or 20
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: l=%lx\n",l);
  			        if(Mnn>=11 && Mnn<=17) {                 // gestisco JR
                  subBranch(TS,T2S,mn,T,k,l,FALSE);
  			          }
  			        else if(Mnn==4 || Mnn==11) {                           // gestisco CALL e JP 
                  subBranch(TS,T2S,mn,T,k,l,FALSE);
	  		          }
	  		        else {
	  		          if(Mnn == 5)   // gestisco CLR che e' finta!
  				          PROCOut(mn->a[k].n,l,0,0);
	  		          else
 	  			          PROCOut(mn->a[k].n,l,-1,0);
	  		          }
	  		        }
  			      }
  			    else {
//   fprintf(stderr,"\tSalto: CurAddr: l=%lx\n",l);
  			      k=subStmt(TS,14,&mn);         // jp ea31
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
                subBranch(TS,T2S,mn,T,k,l,FALSE);           // si autogestisce CALL, JP  e JR
	  		        }
  			      }
				    }
			    }
			  else if(V1==VQ_REGISTER) {
			    if(Ind1) {
		        if(S>1 || T>=2) {
		          PROCError(2064,TS);
		          }
		        else {              // inc (x)
		          PROCOut(mn->a[k].n,(Reg[T].m-1) << 3,-1,0);
		          }
			      }
          else {
				    if(S==1) {
				      if(Mnn==5 || Mnn==6 || Mnn==24 || Mnn==26) {   // quelle che vanno solo con A...
				        if(T==0) {
		  			      k=subStmt(TS,1,&mn);         // inc 
		  			      if(k>=0) {
		  		          if(Mnn == 5)   // gestisco CLR che e' finta!
	  				          PROCOut(mn->a[k].n,l,0,0);
		  		          else if(Mnn == 26)  // e SLA, pure
 	  			            PROCOut(mn->a[k].n,0xff,-1,0);
 	  			          else
		  		            PROCOut(mn->a[k].n,-1,-1,0);
							      }
							    }
							  else
    		          PROCError(2062,TS);
				        }
				      else {
  			        if(T==0) {
	  			      	k=subStmt(TS,4,&mn);         // inc 
	  		          PROCOut(mn->a[k].n,0xff,-1,0);
	  		          }
  			        else {
	  			      	k=subStmt(TS,10,&mn);
  		            PROCOut(mn->a[k].n | ((Reg[T].m-1) << 6),-1,-1,0);
  		            }
						    }
					    }
				    else {
		          PROCError(2062,TS);
	 			      }
				    }
				  }
			  }
#endif

      }
//  	while(*TEXTP++ != '\n');

    return TRUE;
    } 
  else {

soloIFS:  
	  i=j=0;
	  while(MCom[i].s) {
	    if(!stricmp(TS,MCom[i].s)) {
	      j=1;
	      break;
	      }
	    i++;
	    };      
	  if(j) {
	    if(UNDEFD) {
//	    printf("ecco if UNDEFD\n");
	      if((MCom[i].m & 0x8020) == 0x8020)
	        goto soloIFS802;
	      else
	        return TRUE;
	      }
		  if(OutList) {    
	      FNGetLine(OldTextp,myBuf);
		    PROCOut1(MCom[i].m & 0xffe0 ? "= " : "- ",myBuf,FALSE);
		    }
      switch(MCom[i].m >> 4) {
        case 0:
	        FNLA(myBuf);
		      switch(MCom[i].m & 0xf) {
						case 0:
						case 1:			// DB ecc
						case 2:
						case 4:
						case 8:
do_db:
							n=0;
							do {                  
			//fprintf(stderr,"FNLA: %s\n",myBuf);
								if(*myBuf == ',')
									PROCCheck(',');
								if(!stricmp(FNLA(myBuf1),"dup")) {
									FNLO(myBuf);
									l2=FNEvalExpr(14,myBuf); 
		//	fprintf(stderr,"EVALEXPR: %ld %s (l=%d)\n",l2,myBuf,l);  // mi sembra strano "l"... boh?
									n+=l;
  								if(l2==-1)                           // se è dup(?) avanzo solo il ptr
  									Seg[CurSeg-1].CurAddr += (l*(MCom[i].m & 0xf));
  								else { 
										while(l--) {
		  								n+=subBytes(myBuf,MCom[i].m,l2);
											}                                              
										}
									}
								else {  
									l=FNEvalExpr(14,myBuf);
	  							if(stricmp(FNLA(myBuf1),"dup")) {
		//	fprintf(stderr,"EVALEXPR 2: %ld %s\n",l,myBuf);
do_dbs:
										n+=subBytes(myBuf,MCom[i].m,l);
										}
									}
								} while(*FNLA(myBuf));
							break;
						}
					if(VarDeclared)
		        LVars->size=n;
					break;
        case 1:
		      switch(MCom[i].m & 0xf) {
						case 0:		// SPACEs
							goto do_db;
							break;
						case 1:		// STRING,ASCIIZ 
							goto do_db;
							break;
						case 2:		// ASCII (non accettano mix di byte e stringhe... forse dovrebbero, specie DEFM
							FNLO(myBuf);
							if(!stricmp(myBuf,"__date__"))
								sprintf(myBuf,"\"%s\"",__date__);
							else if(!stricmp(myBuf,"__time__"))
								sprintf(myBuf,"\"%s\"",__time__);
							else if(!stricmp(myBuf,"__file__"))
								sprintf(myBuf,"\"%s\"",NFS);
							else if(!stricmp(myBuf,"__line__"))
								sprintf(myBuf,"\"%u\"",__line__);
							else
								goto do_dbs;
							goto do_dbs;
							break;
						case 3:		// FILL
							l=FNEvalExpr(14,myBuf);
							FNLA(myBuf);
							j=0;
							if(*myBuf == ',') {
								PROCCheck(',');
								j=FNEvalExpr(14,myBuf);
								}
			        while(l--)
						    PROCOut(LOBYTE(j),-1,-1,0);
							n=l;
							break;
						}
					if(VarDeclared)
						LVars->size=n;
		      break;
	      case 2:
//			        *myBuf=0;
	        l=FNEvalExpr(14,myBuf);
		      switch(MCom[i].m & 0xf) {
// nella prima passata si somma l'ofs a CurAddr; nella seconda ci pensa ProcOut!
		        case 0:
//			        PROCOut(-1,-1,-1,0);
							if(CreaHex) {
								if(HIWORD(l) != CurSeg)
									CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
								}
							else {
								if(CurSeg==-1)
		              PROCError(2101,TS);
		            }
			        l2=Seg[CurSeg-1].CurAddr;
			        if(!(MCom[i].m & 0xf)) {
			          if(l<0)                                  // org e block
			            PROCError(2062,TS);

//forse?	 direi ok senza.							if(!CreaHex && !CreaBin) {		// 2024, verificare se è ok, e se potrebbe andare anche in linker-mode (per evitare di buttare fuori 0 inutili
//								if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
//									Seg[CurSeg-1].BaseAddr=LOWORD(l);
//								else
//									Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LOWORD(l));		// mah, sì

  		          if(CPass) {
  		            if(l>l2) {
 										l-=l2;
										if(!CreaHex && !CreaBin) {		// 2024, verificare se è ok, e se potrebbe andare anche in linker-mode (per evitare di buttare fuori 0 inutili
  										while(l--)
  											PROCOut(0,-1,-1,0);
	  		              }
										else
										  Seg[CurSeg-1].CurAddr+=LOWORD(l);
  		              }
  		            else { 
    			          Seg[CurSeg-1].CurAddr=LOWORD(l);
				            fseek(FObj,l-l2,SEEK_CUR);
				            }
				          }
				        else
  			          Seg[CurSeg-1].CurAddr=l;
				        }
//printf("curaddr=%d\n",Seg[CurSeg-1].CurAddr);
				      break;
		        case 1:                                         // align
			        l2=Seg[CurSeg-1].CurAddr;
			        l3=l2-(l2 % l);
              if(l2 % l)
								l3+=l;
//		        printf("l vale: %lx; l2 vale: %lx, l3 vale %lx e addr %lx\n",l,l2,l3,Seg[CurSeg-1].CurAddr);
//		        getchar();
		          if(CPass) {
		            l2=l3-l2;
		            while(l2--)
		              PROCOut(0,-1,-1,0);
			          }
			        else
                Seg[CurSeg-1].CurAddr=l3;
		          break;
		        case 2:     // block
			        l2=Seg[CurSeg-1].CurAddr;

							if(debug)
								printf("l vale: %lx; l2 vale: %lx\n",l,l2);

			        if(CPass) {
  		          while(l--)
  		            PROCOut(0,-1,-1,0);
				        }
				      else
   							Seg[CurSeg-1].CurAddr+=l;     // block
		          break;
		        }  
		      break;
	      case 0x10:                      // no EQU senza label
          PROCError(2060,TS);
          break;
		    case 0x100:
					FNLA(myBuf);
		      switch(MCom[i].m & 0xf) {
		        case 0:					// cseg
		          CurSeg=DefCSeg;
							if(CreaHex || CreaBin) {
								if(!CPass)
									PROCWarn(1006,"CSEG");
								}
							if(!stricmp(myBuf,"ENDS")) {		// 
								FNLO(myBuf);
								CurSeg=-1;
								}
							else {
								if(CurSeg<0)
									CurSeg=DefCSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
								if(!myBuf[0])		// 
									;
								else if(!stricmp(myBuf,"SEGMENT"))		// bah sì
									FNLO(myBuf);
								else {		// SPOSTO base ma NON CREO byte!
									l=FNEvalExpr(14,myBuf); 
					        if(l<0)   
					          PROCError(2062,TS);
									if(CreaHex || CreaBin) {
										l2=Seg[CurSeg-1].CurAddr;
  									if(CPass) {
  										if(l>l2) {
 												l-=l2;
												Seg[CurSeg-1].CurAddr+=LOWORD(l);
  											}
  										else { 
    										Seg[CurSeg-1].CurAddr=LOWORD(l);
												fseek(FObj,l-l2,SEEK_CUR);
												}
											}
						        else
  					          Seg[CurSeg-1].CurAddr=l;
										break;
										}
									else {
//						        l2=Seg[CurSeg-1].CurAddr;
										Seg[CurSeg-1].CurAddr=l;		// forse... o addr iniziale del segmento? boh, poco importa (C) :D
										if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
											Seg[CurSeg-1].BaseAddr=LOWORD(l);
										else
											Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LOWORD(l));		// mah, sì
										}
									}
								PROCOut(-3,-1,-1,0);
								}
		          break;
		        case 1:				// dseg
		          CurSeg=DefDSeg;
							if(CreaHex || CreaBin) {
								if(!CPass)
									PROCWarn(1006,"CSEG");
								}
							if(!stricmp(myBuf,"ENDS")) {		// 
								FNLO(myBuf);
								CurSeg=-1;
								}
							else {
								if(CurSeg<0)
									CurSeg=DefDSeg=PROCDclSeg(defaultDataSegm,SEGMENT_DATA);
								if(!myBuf[0])		// 
									;
								else if(!stricmp(myBuf,"SEGMENT"))		// bah sì
									FNLO(myBuf);
								else {
									l=FNEvalExpr(14,myBuf); 
					        if(l<0)   
					          PROCError(2062,TS);
									if(CreaHex || CreaBin) {
										l2=Seg[CurSeg-1].CurAddr;
  									if(CPass) {
  										if(l>l2) {
 												l-=l2;
												Seg[CurSeg-1].CurAddr+=LOWORD(l);
  											}
  										else { 
    										Seg[CurSeg-1].CurAddr=LOWORD(l);
												fseek(FObj,l-l2,SEEK_CUR);
												}
											}
						        else
  					          Seg[CurSeg-1].CurAddr=l;
										break;
										}
									else {
	//						        l2=Seg[CurSeg-1].CurAddr;
										Seg[CurSeg-1].CurAddr=l;		// forse... o addr iniziale del segmento? boh, poco importa (C) :D
										if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
											Seg[CurSeg-1].BaseAddr=LOWORD(l);
										else
											Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LOWORD(l));		// mah, sì
										}
									}
								PROCOut(-3,-1,-1,0);
								}
		          break;
		        case 2:				// csect
			        FNObbLO(myBuf);
							if(CreaHex) {
								if(!CPass)
									PROCWarn(1006,"CSECT");
								}
							else {
								CurSeg=PROCDclSeg(myBuf,SEGMENT_CODE);
								PROCOut(-3,-1,-1,0);
								}
			        break;
		        case 3:				// dsect
			        FNObbLO(myBuf);
							if(CreaHex) {
								if(!CPass)
									PROCWarn(1006,"DSECT");
								}
							else {
				        CurSeg=PROCDclSeg(myBuf,SEGMENT_DATA);
								PROCOut(-3,-1,-1,0);
								}
			        break;
		        case 4:				// const
							if(*myBuf)
								FNLO(myBuf);
							else
								strcpy(myBuf,defaultConstSegm);
							if(CreaHex) {		// qua?
								if(!CPass)
									PROCWarn(1006,"CONST");
								if(CurSeg<0)
					        CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);		// gestire... read-only
								}
							else {
				        CurSeg=PROCDclSeg(myBuf,SEGMENT_CONST);		// gestire... read-only
			          PROCOut(-3,-1,-1,0);
								}
			        break;
		        case 5:				// bss
							if(*myBuf)
								FNLO(myBuf);
							else
								strcpy(myBuf,defaultBSSSegm);
							if(CreaHex) {		// qua?
								if(!CPass)
									PROCWarn(1006,"BSS");
								if(CurSeg<0)
					        CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);			// gestire... zero-init
								}
							else {
				        CurSeg=PROCDclSeg(myBuf,SEGMENT_BSS);			// gestire... zero-init
			          PROCOut(-3,-1,-1,0);
								}
			        break;
		        case 6:				// isr/irq
			        FNObbLO(myBuf);
							if(CreaHex) {		// qua? no, fare
								if(!CPass)
									PROCWarn(1006,"ISR");
								}
							else
								CurSeg=PROCDclSeg("isr",SEGMENT_CODE);
#if Z80
							Seg[CurSeg-1].CurAddr=0x38;		// bah vabbe'
#elif _6502
#elif I8051
#elif ST62
#endif  
							if(CreaHex) {		// qua? no, fare
								}
							else {
			          PROCOut(-3,-1,-1,0);
								}
			        break;
						}
					break;
		    case 0x101:
		      switch(MCom[i].m & 0xf) {
			      case 0:			// PROC ENTRY
			        if(OutList)
			          PROCOut1("=",NULL,FALSE);  // si legge meglio...
			        FNLO(myBuf);
		          if(CurProc)
		            PROCError(2100,myBuf);
		          CurProc=PROCDclVar(myBuf,-1,0x100,0,NULL);
			        break;
			      case 1:			// ENDP
			        PROCError(2060,TS);
			        break;
			      case 2:                      // end forza EOF!
			        CurProc=0;                 // vale anche come endp...
			        fseek(FIn,0,SEEK_END);
			        break;
						}
					break;
		    case 0x102:
		      switch(MCom[i].m & 0xf) {
			      case 0:                        // public
			        do {
				        FNObbLO(myBuf);
			          if(v1=FNCercaVar(myBuf,TRUE,CPass)) {
			            v1->type |=0x280;
			            }
			          else {  
				          PROCDclVar(myBuf,0,0x200,0,NULL);
				          }
				        } while(*FNLO(myBuf) == ',');
			        break;
			      case 1:                         // extern
			        do {
				        FNObbLO(myBuf);
				        if(!CPass) {
				          PROCDclVar(myBuf,0xff000000,0x400,0,NULL);
				          }
				        } while(*FNLO(myBuf) == ',');
			        break;
			      }
		      break;
		    case 0x103:
		      switch(MCom[i].m & 0xf) {
			      case 0:                        // name
							break;
						}
		      break;
		    case 0x200:
		      switch(MCom[i].m & 0x1f) {
		        case 0:
	            fseek(FIn,FNGetLine(ftell(FIn),__title__),SEEK_SET);
	            if(*__title__ == '\'' || *__title__ == '\"') {
	              strcpy(__title__,__title__+1);
	              __title__[strlen(__title__)-1]=0;
	              }
		          break;
		        case 1:
//		          *myBuf=0;
		          PageLength=FNEvalExpr(14,myBuf);
		          break;
		        case 2:
//		          *myBuf=0;
		          LineLength=FNEvalExpr(14,myBuf);
		          break;
		        case 3:
		        case 5:
		          if(CPass) {
		            FNLO(myBuf);  // ignoro "out"
			          do {
			            FNLO(myBuf);
//			            puts(myBuf);
			            if(v1=FNCercaVar(myBuf,FALSE,1))
			              printf("%ld ",v1->value);
			            else  
			              printf("%s ",myBuf);
			            } while(*myBuf);
	              putchar('\n');
                }
              else {
  	            fseek(FIn,FNGetLine(ftell(FIn)+4,myBuf),SEEK_SET);   // salto OUT
                }  
		          break;
		        case 4:          // eject
					    PROCOut1("\f",NULL,FALSE);
		          break;
#if ST62		          
		        case 0x10:               // versione
	            FNLO(myBuf);
	            if(*myBuf == '\"')
	              strcpy(myBuf,myBuf+1);
	            if(myBuf[strlen(myBuf)-1] == '\"')
	              myBuf[strlen(myBuf)-1]=0;
					    PROCDclVar(myBuf,1,1,0,NULL);
		          break;
		        case 0x11:               // memoria a finestre
					    PROCDclVar("W_ON",1,1,0,NULL);
		          break;
#endif		          
		        }
		      break;
		    case 0x400:
	        do {
		        FNLO(myBuf);
//	    puts(myBuf);         // macro!
		        } while(*myBuf);
  				l=ftell(FIn);
	        do {
						l=FNGetLine(l,myBuf);
//      puts(myBuf);         // macro!
	          if(!stricmp(myBuf,"endm"))
	            break;
		        } while(*myBuf);
		      OldTextp=l;
	        fseek(FIn,OldTextp,SEEK_SET);
        __line__=ol;
		      break;
		    case 0x800:  
		      switch(MCom[i].m & 0xf) {
		      FILE *OldF;
		      int OldPass;
		      int OldLine;
		      char OldFile[128];
		      
		        case 0:
				      FNObbLO(myBuf);
				      if(*myBuf != '\"') {
					      FNLO(myBuf1);
				        if(*myBuf1) {
			            strcat(myBuf,myBuf1);  
					        FNLO(myBuf1);
					        if(*myBuf1)
				            strcat(myBuf,myBuf1);
				          }
				        }  
			        OldF=FIn;  
			        OldPass=CPass;
			        OldLine=__line__;
			        strcpy(OldFile,__file__);
	            FNLeggiFile(myBuf,FObj,0);		// finire! level
	            FIn=OldF;
	            CPass=OldPass;
	            __line__=OldLine;
			        strcpy(__file__,OldFile);
		          break;
		        }
		      break;    
		    case 0x801:
		      switch(MCom[i].m & 0xf) {
		        case 0:                          // if
		          IfDefs++;
				      FNObbLO(myBuf);
		          switch(*myBuf) {
	              case '1':
	              case '2':
			            UNDEFD=CPass == (*myBuf - '1');
			            break;
		            default:
			            if(v1=FNCercaVar(myBuf,FALSE,1))
			              UNDEFD=!v1->value;
			            else
			              PROCError(2065,myBuf);  
			            break;
		            }
		          break;  
		        case 1:						// ife
		          IfDefs++;
				      FNObbLO(myBuf);
	            if(v1=FNCercaVar(myBuf,FALSE,1)) {   // cerco come se fosse PASS2
	              UNDEFD=!v1->value;
  	            UNDEFD=!UNDEFD;
  	            }
	            else
	              PROCError(2065,myBuf);  
	            break;
		        case 2:						//ifdef
		          IfDefs++;
				      FNObbLO(myBuf);
	            UNDEFD=!FNCercaVar(myBuf,FALSE,1);
		          break;
		        case 3:						//ifndef
		          IfDefs++;
				      FNObbLO(myBuf);
	            UNDEFD=!!FNCercaVar(myBuf,FALSE,1);
		          break;
		        }
		      break;    
		    case 0x802:
soloIFS802:
		      switch(MCom[i].m & 0xf) {
		        case 0:                          // else
			        if(!IfDefs)
			          PROCError(1019,NULL);
			        UNDEFD = !UNDEFD;      
		          break;
		        case 2:                          // endif
			        if(!IfDefs)
			          PROCError(1020,NULL);
			        IfDefs--;
			        if(!IfDefs)
			          UNDEFD=FALSE;
		          break;
		        }
		      break;    
		    case 0x808:			// pragma
					printf("pragma %s ignored\n",myBuf);			//:)
					if(FCod)
    				fprintf(FCod,"pragma %s ignored\n",myBuf);
	        fseek(FIn,FNGetLine(ftell(FIn),myBuf),SEEK_SET);
		      break;    
		    case 0x809:			// warning
		      switch(MCom[i].m & 0xf) {
						char *p;
		        case 0:                          // warning
							p=strchr(myBuf,' ');
							if(!CPass) {
 //   						printf("WARNING: %s\n",p);		// anche NULL va bene :)
								PROCWarn(4099,p);
								}
							if(CPass)
								if(FCod)
    							fprintf(FCod,"WARNING: %s\n",p);
							fseek(FIn,FNGetLine(ftell(FIn),myBuf),SEEK_SET);
				      break;    
		        case 1:                          // error
							p=strchr(myBuf,' ');
							if(!CPass) {
//    						printf("ERROR: %s\n",p);
								PROCError(4099,p);
								}
							if(CPass)			// ovviamente non arriverà mai ma ok :)
								if(FCod)
    							fprintf(FCod,"ERROR: %s\n",p);
							fseek(FIn,FNGetLine(ftell(FIn),myBuf),SEEK_SET);
				      break;    
						}
		      break;    
		    default:
	        fseek(FIn,FNGetLine(ftell(FIn),myBuf),SEEK_SET);
        __line__=ol;
	      	break;
		    }
	    return TRUE;
		  }
	  else {
noStmt:
			if(*TS && !UNDEFD) {
	      fseek(FIn,OldTextp,SEEK_SET);
  	    __line__=ol;
  	    bInizioRiga=oi;
				VarDeclared=FALSE;		// controllare oi?
  	    return FALSE;
  	    }
  	  else
  	    return TRUE;  
	    } 
	  }
  }
 
char *FNGetConst(char *s) {
	signed char VQ=0;
  int R=0,D=0;
	uint16_t S=0;
  int T=0;
  struct VARS *V;
 
  *s=0;
  FNRev(14,&VQ,&T,&S,&V,s);
  
//  if(debug)
//    printf("Costante: %s\n",s);
    
  if(V>=0) {
    PROCError(2057,NULL);
    }
  else {
    return s;
    }
  return NULL;
  }
 
struct VARS *FNCercaVar(char *n,int m,int pass) {
// M% TRUE=RICERCA NEL BLOCCO, FALSE RICERCA GLOBALE
  int i,Bl;
  char F[256],myN[MAX_NAME_LEN+2];
  struct VARS *V;

//  V=Var;
//  while(V) {
	strcpy(myN,n);
	if(IgnoreCase) 
		strupr(myN);
		
/*		  if(CurProc) {			// C'E' QUALCHE PROBLEMA...v. anche sopra
	  sprintf(myBuf,"$%lx_%s",CurProc,myN);
	  strncpy(myN,myBuf,MAX_NAME_LEN);
		
		}*/
      
//		  printf("cerco %s...\n",myN);
//rifo:
	V=Var;
	if(!V)
	  return 0;
	while(i=strcmp(myN,V->label)) {
//		  printf("trovo... %Fs\n",V->label);
		if(i<0)
		  V=V->left;         // Esegue la ricerca
		else
		  V=V->right;
		if(!V)
		  return 0;
		}
//		  printf("trovo...\n");
		
//  if(m && V->proc!=CurProc)         // var. locali supportate?
//		goto rifo;		//NO! bisogna inserire anche nell'albero la condizione "curproc"
  
      if(CPass) {
				if((V->type & 0x400) && !*(((char *)&V->value)+2)) {
//			  if(CPass) {
//			  V->value = V->value & 0xff00ffff | (((long)++NumExtrnUsed) << 16);
	  	    *(((char *)&V->value)+2) = ++NumExtrnUsed;      // max 255...
	  	    // questo valore viene usato da subOut(-2..) per numerare le extrn usate in un segm.
	  	    if(!V->lpos) {
			      V->lpos=OldOffNome+strlen(Seg[CurSeg-1].label)+1;
						OldOffNome+=strlen(V->label)+1;
//						printf("uso: %Fs, ofsnome= %lx, sommo %s\n",V->label,V->lpos,Seg[CurSeg-1].label);
						}
//				  printf("numextrn vale %d \n",NumExtrnUsed);
			    }
			  }

//      V->extUsed=1;
      return V;
//      }
//    V=V->next;  
//    }
//  return 0;
  }
 
struct VARS *PROCDclVar(char *nome, long lv, int type, uint32_t tag,const char *segm) {
  int t1,T2,i;
  struct VARS *v;
  char T[256],S[256],MyBuf[256];
  char *t,*OldTextp,*t2;
  
  if(IgnoreCase)
    strupr(nome);
  v=FNCercaVar(nome,TRUE,CPass);     // SE LA VARIABILE GIA' ESISTE...
  if(!CPass) {
	  if(v) {
	    if(((v->type & 0x380) != 0x200) || (type & 0x400))
	    // lascio passare le public ridefinite ma blocco eventuali extern
	      PROCError(2086,nome);
	    }
	  else {  
		  v=(struct VARS *)malloc(sizeof(struct VARS)); 
		  if(!v) {
		    PROCError(1001,"Fine memoria VARS");
		    }
		  v->type=0;
			v->dim=0;
		  
/*		  if(CurProc) {			// C'E' QUALCHE PROBLEMA...v. anche sopra
	  	  sprintf(MyBuf,"$%lx_%s",CurProc,nome);
	  	  _fstrncpy(v->label,MyBuf,MAX_NAME_LEN);
	  	  }
		  else*/
  		  strncpy(v->label,nome,MAX_NAME_LEN);
  		  
//printf("dichiaro %s\n",v->label);  		  
		  Var=PutOnTree(Var,Var,v);
		  LVars=v;
		  }
 
	  if(lv==-1l) {
//	    v->value=Seg[CurSeg-1].CurAddr | (((long)CurSeg) << 16);
			if(segm) {		// FINIRE... era un'idea per mettere in CONST ecc in base al tipo di dichiarazione/label 2024
//				i=FNCercaSeg(segm);
				i=PROCDclSeg(segm,SEGMENT_DATA);		// mah
				v->value=Seg[i-1].CurAddr;
				*(((uint16_t *)&v->value)+1)=i;
				}
			else {
				v->value=Seg[CurSeg-1].CurAddr;
				*(((uint16_t *)&v->value)+1)=CurSeg;
				}
	    v->size=4;
	    }
	  else {
	    v->value=lv;
	    v->size=FNGetSize(lv);
//	    printf("size di %s: %d\n",nome,FNGetSize(lv));
	    }
	  v->tag=tag;
	  if(v->type & 0x200)
	    v->type |= (0x80 | type);
	  else  
	    v->type |= type;
	  v->proc=CurProc;
    v->lpos=0l;
    v->extUsed=0;
    }
	  
  if(FMap)
		fprintf(FMap,"-\t%-32s\t%6X:%04X\t%4X\t%4X\t%s (%s:%u)\n",v->label,
			HIWORD(v->value),LOWORD(v->value),v->type,v->size,v->proc ? v->proc->label : ((char *)""),
			__file__,__line__);

	VarDeclared=TRUE;
  return v;  

//  fprintf(stderr,"\tcreo VAR %d: %s = %lx\n",v,nome,Var[v].value);
  
  }

int FNCercaSeg(const char *n) {
  int i;

  for(i=0; i<Segs; i++) {
    if(!stricmp(n,Seg[i].label)) { 
      return i+1;
      }
    }
  return 0;
  }
 
int PROCDclSeg(const char *nome,enum SEGMENT_TYPES t) {
  int v,T1;
  
  T1=FNCercaSeg(nome);     // SE GIA' ESISTE...
  if(!T1) {
	  v=Segs++; 
		if(Segs == MAX_SEGS)
		  PROCError(5000,nome);
		strncpy(Seg[v].label,nome,MAX_NAME_LEN);
		Seg[v].type=t;
		Seg[v].BaseAddr=0xffffffff;
	  if(FMap)
			fprintf(FMap,"+\t%-38s\t%04X\t(%s:%u)\n",Seg[v].label,
				Seg[v].type,__file__,__line__);
		return v+1;
		}
  else
    return T1;  

  fprintf(stderr,"\tcreo SEG %d: %s = %lx\n",v,nome,Var[v].value);
  }

struct VARS *PutOnTree(struct VARS *root,struct VARS *r,struct VARS *n) {
  int i;
	/* Usuale algoritmo di inserimento Inordine */

//	puts("Sto inserendo");
	if(!r) {
		n->left=0;
		n->right=0;
		if(root) {
			if(strcmp(n->label,root->label)<0)
				root->left=n;
			else
			  root->right=n;
			}
		n->next=NULL;
		return n;
		}
	i=strcmp(n->label,r->label);
	if(i<0) {
	  PutOnTree(r,r->left,n);  // a sinistra <
	  }
	else {
	  if(i>0) {
		  PutOnTree(r,r->right,n); // a destra   > 
		  }
		else {  
		  while(r->next)
		    r=r->next;// se sono uguali lo inserisco nella lista 
			r->next=n;
			}
		}

	return root;
	}

/* Disalloca la memoria occupata da un albero o da una lista */
void Libera(struct VARS *root) {

	if(!root) 
		return;

	Libera(root->left);
	Libera(root->right);
	do {
		free(root);
		} while(root=root->next);
	}

unsigned char asc2hex(unsigned short int t) {
	int i,j;

	i=toupper(LOBYTE(t));
	if(i>='0' && i<='9')
		j=i-'0';
	else
		j=i-'A'+10;
	j <<= 4;
	i=toupper(HIBYTE(t));
	if(i>='0' && i<='9')
		j |= i-'0';
	else
		j |= i-'A'+10;
	return j;
	}

unsigned char *convertHEX2BIN(FILE *f,unsigned long *bsize) {
	unsigned char *theMemory;
	char myBuf[256],*p;
	unsigned char *b;
	int minAddr,maxAddr,len;
	unsigned char cks;
	unsigned long ofs,extOfs,row,checksum;
	int recType,i,j;


	theMemory=(unsigned char*)malloc(65536);			// 
	memset(theMemory,0xff,65536);

	minAddr=0xffffff; maxAddr=0;
	row=0;
	checksum=0;			// Checksum totale!
	ofs=extOfs=0;
	row=0;


	for(;;) {			// da SkyPic...
		myBuf[0]=0;
		fgets(myBuf,255,f);
		p=myBuf;
		if(*p != ':')							// START record
			goto errore;
		p++;
		cks=len=asc2hex(*(unsigned short int *)p);
		p+=2;
		j=asc2hex(*(unsigned short int *)p);
		ofs=j;
		cks+=j;
		ofs <<= 8;
		p+=2;
		j=asc2hex(*(unsigned short int *)p);
		ofs |= j;


		cks+=j;
//		if(ofs>(maxMemory*2)) {		// word -> byte
//					S.Format("Posizione di memoria specificata nel file HEX non valida: %04X, %s",ofs,myBuf);
//				}
		p+=2;
		recType=asc2hex(*(unsigned short int *)p);		// tipo rec: 0 dati, 1 EOF, 2=Ext.Segm.Addr.Rec., 
																		// 3=Start Segm.Addr.Rec., 4=Ext.Linear Addr.Rec., 5=Start Linear Addr.Rec.
		cks+=recType;
		p+=2;
		switch(recType) {
			case 2:			// segmented Intel
				j=asc2hex(*(unsigned short int *)p);
				extOfs=((unsigned long)j) << 12;
				cks+=j;
				p+=2;
				j=asc2hex(*(unsigned short int *)p);
				extOfs |= ((unsigned long)j) << 4;
				cks+=j;
				p+=2;
				break;
			case 4:			// linear
				j=asc2hex(*(unsigned short int *)p);
				extOfs=((unsigned long)j) << 24;
				cks+=j;
				p+=2;
				j=asc2hex(*(unsigned short int *)p);
				extOfs |= ((unsigned long)j) << 16;
				cks+=j;
				p+=2;
				break;
			case 0:
				if(extOfs==0) {
					b=((unsigned char*)theMemory)+ofs;
					for(i=0; i<len; i++) {
						j=asc2hex(*(unsigned short int *)p);
						minAddr=min(minAddr,(unsigned char*)b-(unsigned char*)theMemory);
						maxAddr=max(maxAddr,(unsigned char*)b-(unsigned char*)theMemory);
						*b++=j;
						cks+=j;
						checksum+=j;
						p+=2;
						}
					}
				else {			
//						S.Format("Valori offset %X:%X non gestiti!",extOfs,ofs);
//								if(AfxMessageBox(S,MB_YESNO) == IDNO) 
//									goto errore;
					}
				break;
			case 0x03:
			case 0x05:
			 fprintf(stderr, "Ignoring start address record\n");
				continue;
			case 1:
				goto fine;
				break;
			}
		cks=-cks;
		j=asc2hex(*(unsigned short int *)p);
		if(LOBYTE(cks) != j)
			goto errore;
		row++;
		} 

errore:
//	S="Errore nel file HEX!";
//	AfxMessageBox(S,MB_ICONEXCLAMATION);
	free(theMemory);
	return 0;


fine:
//		m_Stato.Format("File HEX caricato correttamente (%04X-%04X).",minAddr,maxAddr);
	*bsize=maxAddr-minAddr +1;
	return theMemory;

	}

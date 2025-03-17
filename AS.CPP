/*  **************************
    *  Assembler             *
    *        BY G.Dar        *
    *  c version on          *
    *            6/6/94      *
    *  ST 62:   15/6/97      *
    *  Z80 renewed 2/24      *
    *  6502 version	2/24     *
    *  PIC version	2/24     *
    *  GD24032 version	7/24 *
		*  8086 version  2/25    *
    **************************/
// Come ST62 accetto le direttive anche precedute dal punto. Standard SGS! (2024: tutti...
// le label DEVONO avere il : sia per istruzioni che per DEFb/ecc (preferisco  NO! v.MASM 2025 cambiato)
// in 8086 il segmento per linker lo metto solo se non sono ind. relativi! quindi segm:ofs


// (risolto 2024: in un istruzione DB 1,2,3,4,  ACCETTA l'ultima VIRGOLA SENZA DARE ERRORE! e mette uno '0'!! 20/2/01 ; 0xffffff nel 2024
// se i segmenti si alternano, si incasina il file O! (abbastanza ovvio... va gestito: le relocation finiscono alla fine della prima parte e la seconda viene persa; il file LST è ok!
// [forse risolto 8/24 se manca CR/LF a fine file si inventa un NOP o altro! in pratica ripete ultima riga!!
//  e l'ULTIMO opcode nel file LST non esce (v. patch sotto, pare risolvere anche problema CR/LF fine file NO! ripete l'ultima cmq :D
// forse sistemato 4/8/24 (park albatros day) [in modalità BIN (ma anche ELF, con messaggio diverso) dice "impossibile scrivere file BIN" se c'è qualche tipo di errore in compilazione... tipo dati fuori segmento o boh... che però non esce alla riga giusta ma solo al fondo...
//  [no, lo fa ancora... il primo record esce ciucco ERRORE REPOS segmenti in BIN! ritestare ELF/OBJ...

// [METTERE NUM RIGA in LST?? almeno "ogni tanto"? fatto... gestire opzione ev.

// CALL/BL si incasina tra 1/2pass tentando di andare su byte...

// verificare aggiungere controlli tra 2reg indexed e immediate8...

// ci sono ancora alcuni 0xC0 in 8086, ma è difficile gestirli in tabella... tipo se hanno 00 come primo byte e non uscirebbero, e altro in MOV...
//sizeof con struct/record

// ho la sensazione che IF/IFDEF ecc nidificate non vadano... ammesso che abbia senso!

// nonostante i vari  if(Seg[CurSeg-1].type != SEGMENT_VIRT) nel file .O mette ancora i segmenti "AT" ossia senza dati... va rifinita la gestione (BIN/HEX è ok)
// curaddr del seg non conserva la parte alta in certi casi v. segm 2 _BIOS


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
struct TAGS *StrTag, *LTag;
struct SEGS Seg[MAX_SEGS];
// char StrTag[MAX_NAME_LEN+1][200];
int LABEL;

struct OPERANDO Operandi[]={
  "(",1,")",1,"[",1,"]",1,
  ".",1,			// per struc
#if I8051
  "@",1,"#",1,
#endif
#if GD24032
  "++",1,"--",1,			// 1??
  "{",1,"}",1,			// 1??
#endif
  "~",2,"size",2,"length",2,"sizeof",2,"lengthof",2,"type",2,"low",2,"high",2,"(",2,"!",2,"-",2,/*"dup",2,*/		// che ci fa la par. tonda qua?
#if I8086
	"not",2,
	"byte",2,"word",2,"dword",2,"qword",2,"near",2,"far",2,"ptr",2,
	"segment",2,"offset",2,
#endif
	"mask",2,"width",2,			// per record/bitfield
  "*",3,"/",3,"%",3,
  "+",4,"-",4,		// OKKIO c'è anche in 2 e qua non ha molto senso... boh (a parte quelle con pre/postinc/dec
#if I8086
  ":",4,			// solo per far:ptr, il segment override è gestito a parte
#endif
  "<<",5,">>",5,"SHL",5,"SHR",5,
	// anche SHR SHL v.masm..
//  "<",6,">",6,"<=",6,">=",6,		"LT",6,"GT",6,"LE",6,"GE",6,
//    "<",6,">",6,		li gestisco separatamente cmq...
//  "==",7,"!=",7, "EQ",7,"NE",7,
  "&",8,"AND",8,		// bah TOGLIERE gli operandi "C"? verificare in altri asm, oltre a MASM...
  "^",9,"XOR",9,
  "|",10,"OR",10,
  "=",14,  "<",14,">",14,	/* casi particolari*/
  ",",15,
  NULL,0
  };         
  
// FLAGS:
#if PIC==16 || PIC==18
	int8_t IgnoreCase=TRUE;
#else
	int8_t IgnoreCase=FALSE;
#endif
int8_t BranchOpt=0;
int8_t OutMap=FALSE;
int8_t OutCycle=FALSE;               // riporta i cicli macchina per istr.
int8_t OutVars=FALSE;
int8_t OutList=FALSE;
int8_t OutError=FALSE;
int8_t WarnLevel=0;
int8_t CreaHex=FALSE,CreaBin=FALSE,CreaElf=0;
int8_t NoMacro=FALSE;
int8_t NoSignExtend=TRUE;		// v. MASM, fare
uint8_t CPULevel=0;				// spec 8086/186/286/386/486 ecc
struct VARS *CurProc;
char theLine[1024];
int theLinePtr;
int16_t CurSeg;
int16_t DefCSeg,DefDSeg;
int LastStmt4Cycle=-1;                // memorizzo l'ultimo stmt per i cicli macchina
uint8_t CPass;
uint8_t bInizioRiga=FALSE,VarDeclared=FALSE;
uint8_t inMacroExecuting=FALSE;
unsigned int theMacroPtr;
char theMacro[1024];
uint8_t inStructDeclaration=0;			// 1=struc/union
uint8_t inIfDef=0;					// per EvalExpr di espressioni in IF, ecc
uint32_t currVarTag=0;			// la struc o record che sta venendo dichiarata, il "padre" dei membri
extern uint32_t OldOffNome,LastAddress;

char __file__[256];
char __title__[256];
int __line__,__page__;
char __date__[10];
char __time__[10];
char defaultCodeSegm[64],defaultDataSegm[64],defaultConstSegm[64],defaultBSSSegm[64],defaultAbsSegm[64];

int SX=1;          // INDICE DELLE STRUTTURE
//int OP=FALSE;          // C'E' UN OPERANDO PENDENTE
int8_t debug=0;
//int PP;
//int Vars;
int Segs;
struct LINE *RootOut,*LastOut;
  // Usiamo StaticOut (ProcOut1) per il testo in output
char buffer[256];

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
  
int subCmpMnem(struct CODE *, struct CODE *);

int PROCInit() {
  int t;
                           
#if Z80
  qsort(Mnem,67,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif _6502
  qsort(Mnem,64,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif I8051
  qsort(Mnem,44,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif ST62
  qsort(Mnem,31,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif PIC==16
  qsort(Mnem,37,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif PIC==18
  qsort(Mnem,77,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif GD24032
  qsort(Mnem,105,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif I8086
  qsort(Mnem,435,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
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

	strcpy(defaultCodeSegm,"TEXT");
	strcpy(defaultDataSegm,"DATA");
	strcpy(defaultBSSSegm,"BSS");		// 
	strcpy(defaultConstSegm,"CONST");		// 
	strcpy(defaultAbsSegm,"_abs");

  if(!NoMacro) {
		struct SEGMENTED_VALUE v;
		v.seg=0;
		v.v=1;
#if ARCHI
    PROCDclVar("ARCHIMEDES",v,1,0);
#elif Z80
    PROCDclVar("Z80",v,1,0);
#elif _6502
    PROCDclVar("_6502",v,1,0);
#elif I8051
    PROCDclVar("I8051",v,1,0);
#elif ST62
    PROCDclVar("ST62",v,1,0);
#elif PIC==16
    PROCDclVar("PIC16",v,1,0);
#elif PIC==18
    PROCDclVar("PIC18",v,1,0);
#elif GD24032
    PROCDclVar("GD24032",v,1,0);
#elif I8086
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
#elif PIC==16
  fprintf(stderr,"\nAssemblatore per PIC16 su PC - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#elif PIC==18
  fprintf(stderr,"\nAssemblatore per PIC18 su PC - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#elif GD24032
  fprintf(stderr,"\nAssemblatore per GD24032 su PC - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#elif I8086
  fprintf(stderr,"\nAssemblatore di G.Dar per l'80x86 - Versione %d.%02d\n",__VER__ / 256,__VER__ & 255);
#endif
  fprintf(stderr,"(C) Dario's Automation 2024-2025; ADPM Synthesis 1994-2000\n");

  return 0;                        
  }

int main(int argc, char **argv) {
  int i,j;
  char ffc[256];                 // nome del file -Fc (listing)
  char *p,*p1;
  FILE *f;
	struct LINE_DEF *root;
  
  for(i=1; i<argc; i++) {
    if(*argv[i]=='-' || *argv[i]=='/') {
      switch(*(argv[i]+1)) {
        case '?':
        case 'h':
				  goto print_init;
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
						struct SEGMENTED_VALUE d;
            strncpy(buffer,argv[i]+2,p-argv[i]-2);
						buffer[p-argv[i]-2]=0;
						p++;
						if(*p=='\"') {
							p++;
							d.v=*p;
							}
// gestire stringhe, char!!! come MASM...
						else if(*p=='\'') {
							p++;
							d.v=*p;
							}
						else
							d.v=atol(p);

						d.seg=0;
            PROCDclVar(buffer,d,1,0);
            }   
          break;
        case 'E':
          CreaElf = 1;		// ELF
          break;
        case 'e':
          CreaElf = 2;		// MS OBJ / COFF
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
	  PROCInit();
    fprintf(stderr,"\nNome file mancante.\n");
		exit(99);
print_init:
	  PROCInit();
    fprintf(stderr,"\nSintassi:  AS <nomefile> [switches]\n");
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
		puts("            / ... considera valori unsigned");
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
  
  PROCOut1(";");
  UNDEFD=FALSE;

	if(CreaElf && CreaHex)
    PROCError(4002,"opzioni incompatibili: ELF e HEX/BIN");

  if(CreaHex) {
	  AddExt(OUS,"hex");
    }
  else if(CreaElf==1) {
	  AddExt(OUS,"elf");
    }
  else if(CreaElf==2) {
	  AddExt(OUS,"obj");
    }
  else {
	  AddExt(OUS,"o");
	  }
  if(!(FObj=fopen(OUS,"wb"))) 
    PROCError(1037,OUS);

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
		if(!CPass) {
			for(i=0; i<MAX_SEGS; i++) {
				Seg[i].BaseAddr==0xffffffff;
				Seg[i].CurAddr = 0;
				}
			}
		else {
			for(i=0; i<MAX_SEGS; i++) {
				Seg[i].CurAddr = Seg[i].BaseAddr==0xffffffff ? 0 : Seg[i].BaseAddr;
				}
			}
		root=RootMacro;
		while(root) {
			root->used=0;
			root=root->next;
			}
	  DefCSeg=DefDSeg=CurSeg=SEG_UNDEFINED;
	  CurProc=0;
//	  __line__=1;
		if(CPass) {
			j=-1;
			LastAddress=0;
			for(i=0; i<Segs; i++) {
				if(Seg[i].type == SEGMENT_VIRT) 
					continue;
				if(j == -1) {
					j=i;
			    if(CreaElf==1) {
						// qua bisognerebbe vedere come fare con _abs/extern
						Seg[j].lpos=sizeof(ELF_HEADER)+sizeof(ELF_PROGRAM_HEADER)*(Segs);
						Seg[j].lposReloc=Seg[j].lpos+Seg[j].len;
						Seg[j].lposPub=Seg[j].lposReloc+Seg[j].numReloc*sizeof(struct ELF_REL);
						Seg[j].lposExt=Seg[j].lposPub+Seg[j].numPub*sizeof(struct ELF_REL);
						Seg[j].numReloc=0;
						LastAddress=max(LastAddress,Seg[j].lposExt+Seg[j].numExt*sizeof(struct ELF_REL));
						}
			    else if(CreaElf==2) {
						// qua bisognerebbe vedere come fare con _abs/extern
						Seg[j].lpos=sizeof(COFF_HEADER)+sizeof(COFF_SECTION_TABLE)*(Segs);
						Seg[j].lposReloc=Seg[j].lpos+Seg[j].len;
						Seg[j].lposPub=Seg[j].lposReloc+Seg[j].numReloc*sizeof(struct COFF_RELOCATIONS);
						Seg[j].lposExt=Seg[j].lposPub+Seg[j].numPub*sizeof(struct COFF_RELOCATIONS);
						Seg[j].numReloc=0;
						LastAddress=max(LastAddress,Seg[j].lposExt+Seg[j].numExt*sizeof(struct COFF_RELOCATIONS));
						}
					else {
						Seg[j].lpos=sizeof(OBJ_HEADER)+sizeof(OBJ_SEG_DEF)*(Segs);
						Seg[j].lposReloc=Seg[j].lpos+Seg[j].len;
						Seg[j].lposPub=Seg[j].lposReloc+Seg[j].numReloc*sizeof(struct OBJ_RELOC);
						Seg[j].lposExt=Seg[j].lposPub+Seg[j].numPub*sizeof(struct OBJ_RELOC);
						Seg[j].numReloc=0;
//						Seg[j].numPub=0;
//						Seg[j].numExt=0;
						LastAddress=max(LastAddress,Seg[j].lposExt+Seg[j].numExt*sizeof(struct OBJ_RELOC));
						}
					}
				else {
			    if(CreaElf==1) {
						Seg[i].lpos=Seg[j].lposExt+Seg[j].numExt*sizeof(struct ELF_REL);		// ovvero LastAddress..
						Seg[i].lposReloc=Seg[i].lpos+Seg[i].len;
						Seg[i].lposPub=Seg[i].lposReloc+Seg[i].numReloc*sizeof(struct ELF_REL);
						Seg[i].lposExt=Seg[i].lposPub+Seg[i].numPub*sizeof(struct ELF_REL);
						Seg[i].numReloc=0;
						LastAddress=max(LastAddress,Seg[i].lposExt+Seg[i].numExt*sizeof(struct ELF_REL));
						}
			    else if(CreaElf==2) {
						Seg[i].lpos=Seg[j].lposExt+Seg[j].numExt*sizeof(struct COFF_RELOCATIONS);		// ovvero LastAddress..
						Seg[i].lposReloc=Seg[i].lpos+Seg[i].len;
						Seg[i].lposPub=Seg[i].lposReloc+Seg[i].numReloc*sizeof(struct COFF_RELOCATIONS);
						Seg[i].lposExt=Seg[i].lposPub+Seg[i].numPub*sizeof(struct COFF_RELOCATIONS);
						Seg[i].numReloc=0;
						LastAddress=max(LastAddress,Seg[i].lposExt+Seg[i].numExt*sizeof(struct COFF_RELOCATIONS));
						}
					else {
						Seg[i].lpos=Seg[j].lposExt+Seg[j].numExt*sizeof(struct OBJ_RELOC);		// ovvero LastAddress..
						Seg[i].lposReloc=Seg[i].lpos+Seg[i].len;
						Seg[i].lposPub=Seg[i].lposReloc+Seg[i].numReloc*sizeof(struct OBJ_RELOC);
						Seg[i].lposExt=Seg[i].lposPub+Seg[i].numPub*sizeof(struct OBJ_RELOC);
						Seg[i].numReloc=0;
//						Seg[i].numPub=0;
//						Seg[i].numExt=0;
						LastAddress=max(LastAddress,Seg[i].lposExt+Seg[i].numExt*sizeof(struct OBJ_RELOC));
						}
					j=i;
					}
				}
			}
    PROCOut(OUT_START);
    FNLeggiFile(NFS,FObj,0);
    if(IfDefs)
      PROCError(1004);
    if(CurProc)
      PROCError(2100);
    if(!(CreaBin || CreaHex) && CurSeg != SEG_UNDEFINED)
			if(!CPass)
				PROCWarn(2100,"manca ENDS");
    PROCOut(OUT_END);
	  if(!CPass) {
			for(i=0; i<MAX_SEGS; i++) {
				uint64_t cur2=Seg[i].CurAddr;
				if(cur2 == 0 && Seg[i].BaseAddr!=0xffffffff && Seg[i].BaseAddr!=0)
					cur2=0x100000000;		// patch se ho fatto il giro :) 8086
				Seg[i].len=cur2/* rimesso 2025! controllare ELF ecc*/ - (Seg[i].BaseAddr != 0xffffffff ? Seg[i].BaseAddr : 0);

 				}
	    }
	  CPass++;
	  }

  PROCOut1("= end program --------------------------------------------");			// PATCH se no non scrive ultimo opcode
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
		uint8_t *h;
		uint32_t bsize=0;
	  AddExt(OUS,"hex");
	  if(!(FObj=fopen(OUS,"r"))) 
	    PROCError(1038,"file HEX non trovato" /*OUS*/);
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
			PROCError(1038,"file HEX non disponibile (possibili errori di compilazione/overlap)");
    }
	else if(CreaHex)
    fprintf(stderr,"File HEX %s scritto.\n",OUS);
	else
	  fprintf(stderr,"File oggetto %s pronto.\n",OUS);
	fputc('\n',stderr);

	Libera(Var);
	while(RootDef) {
		struct LINE_DEF *t=RootDef->next;
		free(RootDef);
		RootDef=t;
		} 
	while(RootMacro) {
		struct LINE_DEF *t=RootMacro->next;
		free(RootMacro);
		RootMacro=t;
		} 

  return 0;
  } 
      
int FNLeggiFile(char *F, FILE *FO,uint8_t level) {
  uint8_t Go=0,First=0;
  uint8_t IfDefs=0,LstIfs=0;            // per gestire le nidificazioni, metto in Lst il livello
//  FILE *FI;
  char ARGS[256],*p;
//  struct LINE_DEF *L;

  strcpy(__file__,F);
  __line__=0;
//  printf("leggo %s\n",__file__);
  FIn=fopen(FNTrasfNome(F),"rb" /*rb*/);
  if(!FIn)
    PROCError(1013,F);

	theLinePtr=0;
  bInizioRiga=TRUE;
	VarDeclared=FALSE;
  do {

		if(inMacroExecuting) {
			p=strchr(&theMacro[theMacroPtr],'\n');
			if(p)
				*p=0;
			p=strchr(&theMacro[theMacroPtr],'\r');
			if(p)
				*p=' ';
			strcpy(theLine,&theMacro[theMacroPtr]);
			if(*theLine)
				theMacroPtr += strlen(theLine)+1;
			else
				inMacroExecuting=0;
			if(stristr(theLine,"exitm"))			// non è perfetto ma ok
				inMacroExecuting=0;
			}
		else {
			if(!fgets(theLine,1023,FIn))
				break;
			__line__++;
			}
		theLinePtr=0;
	  bInizioRiga=TRUE;
		p=strchr(theLine,'\r');
		if(p)
			*p=0;
		p=strchr(theLine,'\n');
		if(p)
			*p=0;
		

    if(debug) {
      printf("<<<%s>>>\n",theLine);
      }
		
rifo:
    if(inStructDeclaration || !FNIsStmt(NULL))
      if(PROCIsDecl())
				goto rifo;

    } while(!feof(FIn));
      
  fclose(FIn);

  __line__=1;		// per ev. errori a fine passata, 2024

  return TRUE;
  }

int PROCIsDecl() {       // qua si guarda se c'è label DW(...) valore ecc
  int v,i;
  struct SEGMENTED_VALUE l;
  uint32_t op1,op2;
  char T[256],T1[256],MyBuf[128];
  
//    LastStmt4Cycle=-1;
  if(!bInizioRiga)
    PROCError(2060);
  if(UNDEFD)
    return FALSE;
  else {  
	  op1=theLinePtr;
	  FNLO(T);                    // LEGGO UN ITEM
	  if(*T == '.')
		  FNLO(T+1);                    // (gestisco punto
	  if(*T) {

			// ev. FNDefined ... v. LeggiFile e cc
			// per le #macro qua, io farei che tu metti le parentesi e poi vengon tolte: SALVAA(x) -> STA x
			// quindi eviterei un preprocessore a-priori... ma solo sostituzioni al volo su label/costanti e istruzioni

			if(*T != '.') {
				FNLA(T1);                    // LEGGO prossimo ITEM
				if(*T1 == '.') {
					FNLO(T1);	       // salto il punto, (messo nelle direttive
					FNLA(T1+1);       // e lo aggiungo
					}
				}
			else
				strcpy(T1,T);
		  if(*T != '.' && !strcmp(T1,":") && !FNIsDirective(T1)) {
			  FNLO(T1);                    // (confermo
		    if(OutList) {                         
		//    FNGetLine(TEXTP-strlen(T),MyBuf);
		      PROCOut1("= ",theLine /*T*/);
		      }
				l.v=-1;
				l.seg=SEG_NONE;
		    PROCDclVar(T,l,0,0);
		    }
		  else {
			  if(i=FNIsDirective(T1)) {
	  op2=theLinePtr;
			  FNLO(MyBuf);                    // (confermo
			    switch(i) {
			      case 0x1010:   // proc entry NO !
			      case 0x1011:   // endp NO! non passa mai di qua! v. sotto
			      case 0x0100:
			        if(OutList) {                         
			          PROCOut1("= ",theLine);
			          }
			        if(!CPass) {
				        if(i==0x100) {		// EQU
	//		    	      *MyBuf=0;
		  		        l=FNEvalExpr(14,MyBuf);
									if(!l.valid)
										PROCError(2062);
									if(*T1=='=') {			// = può essere più volte!
										VARS *v;
										if(v=FNCercaVar(T,0,FALSE,CPass))
											v->value=l.v;
										else
											PROCDclVar(T,l,1,0);
										}
									else
										PROCDclVar(T,l,1,0);
		  		        }
				        else if((i & 0xff) == 0x10) {		//mmm /*4*/era 4 ... 2024??
				          if(CurProc)
				            PROCError(2100,T);
									FNLA(MyBuf);
									l.v=-1;
#if I8086
									if(!stricmp(MyBuf,"FAR"))
										l.seg=CurSeg;			// 
									else if(!stricmp(MyBuf,"NEAR"))
										l.seg=CurSeg /*SEG_NONE*/;		// boh verificare
									else if(*MyBuf)
										PROCError(1006,MyBuf);
#else
									l.seg=CurSeg/*2025*/;
#endif
				          CurProc=PROCDclVar(T,l,VAR_FUNZ,0);
#if I8086
									if(!stricmp(MyBuf,"FAR")) 
										LVars->size += 2;		// da 16 a 32 bit cmq :)
#endif
				          }
				        else if((i & 0xff) == 0x11 /*5*/) {		// idem
				          if(IgnoreCase)
				            strupr(T);
	//			          printf("cproc: %Fs\n",CurProc->label);  
				          if(!strcmp(T,CurProc->label)) {
					          CurProc->dim=Seg[CurSeg-1].CurAddr-CurProc->value;
				            CurProc=0;
				            }          
				          else 
				            PROCError(2108,T);
				          }       
							  FNLO(MyBuf);                    // (confermo
				        }
				      else                             // vado a fine riga
				        gotoEOL()   /*while(*FNLO(MyBuf))*/
									;
			        break;
			      case 0x4100:			// STRUC
			      case 0x4101:			// UNION
							if(isasym(*T)) {
								if(!inStructDeclaration) {
								  if(!CPass)
										if(FNCercaAggr(T,FALSE,0))
											PROCError(2086,T);
									currVarTag=(uint32_t)PROCDclAggr(T,i==0x4100 ? VAR_STRUCT : VAR_UNION,0,0);
									inStructDeclaration=1;
									FNLO(MyBuf);                    // (confermo
									}
								else {
									PROCError(1006,T);
									}
			
								}
			        break;
			      case 0x4102:			// RECORD
							if(isasym(*T)) {
								struct OPERAND o1;
								struct TAGS *myT;
								char myBuf1[64];
								int n;
								uint8_t go=0;

								currVarTag=(uint32_t)PROCDclAggr(T,VAR_RECORD,0,0);
								do {
									FNLO(MyBuf);                    
									if(!CPass) {
										myT=PROCDclAggr(MyBuf,VAR_RECORD,currVarTag,0);
										}

									PROCCheck(':');
									o1.l=FNEvalExpr(14,myBuf1);
									if(!o1.l.valid)
										PROCError(2062);
									n=o1.l.v;
									if(!CPass) {

//										usare myT e ritestare!

										myT->size=n;		// 
										myT->pos=((struct TAGS*)currVarTag)->size;	// PACK-ARE!
										((struct TAGS*)currVarTag)->size += n;		// bit!
#if I8086
										if((CPULevel<3 && ((struct TAGS*)currVarTag)->size>16) ||
											(CPULevel>=3 && ((struct TAGS*)currVarTag)->size>32)) {
											PROCError(2064);
											}
#else
										if(((struct TAGS*)currVarTag)->size>INT_SIZE*8) {
											PROCError(2064);
											}
#endif
										}
  								FNLA(myBuf1);
  								switch(*myBuf1) {
										case '=':
											PROCCheck('=');
											o1.l=FNEvalExpr(14,myBuf1);
											if(!o1.l.valid)
												PROCError(2062);
											if(!CPass) {
												myT->value=o1.l.v;			// 
												if(myT->value > getMask(myT->size)) {
													PROCWarn(2058);
													myT->value = getMask(myT->size);		// tronco al max!
													}
												((struct TAGS*)currVarTag)->value=FNUpdAggr((struct TAGS*)currVarTag);
												}
											if(!*FNLA(myBuf1)) {
												go=TRUE;
												break;
												}
											else
											// segue
												;
										case ',':
											PROCCheck(',');
											break;
										case 0:
											go=TRUE;
											break;
										default:
											PROCError(2062);
											break;
										}
									} while(!go);

								currVarTag=0;
								FNLO(MyBuf);                    // (confermo
								}
			        break;
			      case 0x410f:			// ENDS
	theLinePtr=op2;
								FNIsStmt(T);

			        break;
			      default:
							if(isasym(*T)) {
								if(!inStructDeclaration) {
									if(stricmp(MyBuf,"SEGMENT")) {			// caso particolare... altri??
										l.v=-1;
										l.seg=SEG_NONE;
										PROCDclVar(T,l,0,0);
										}
									}
								else {
									PROCDclAggr(T,0,currVarTag,0);
									}

								theLinePtr=op2;
								FNIsStmt(T);

//							  FNLO(T1);                    // (confermo  NO NON va con quel che segue, bytes ecc


								}
							else if(*T == '.') {
								theLinePtr=op2;
								FNIsStmt(T);
								}
					    else
					      PROCError(2061,T);  
			        break;
			      }
			    }		// isDirective
			  else {         
					struct TAGS *myT,*myT2;
//noDir:
/*			  was:  NO! se non segue ':' o direttiva, NON dichiaro la label! Altrimenti le istruzioni sconosciute diventano label!  
					if(isasym(*T))
			      PROCDclVar(T,-1l,0,0);
			    else*/

					if(isasym(*T) && (myT=FNCercaAggr(FNLA(T1),FALSE,0))) {		// se tipo user-definito, struc/record
						uint16_t n;
						char myBuf1[256];

//								("*********  allocare AGGR...") ;
						FNLO(T1);
						if(myT->type & (VAR_STRUCT | VAR_UNION)) {
								l.valid=0;
								l.v=-1;
								PROCDclVar(T,l,myT->type,(uint32_t)myT);
								l.valid=0;
								l.v=0;
								PROCOut1("- ",theLine,FALSE);
								FNLA(myBuf1);
  							switch(*myBuf1) {
									case '<':
									case '{':
 										PROCCheck(*myBuf1);
										n=0;
	//									i=myT->size;
										myT2=myT->next;
										if(myT->type & VAR_STRUCT) {
											do {
												FNLA(T1);
												if(isalnum(*T1)) {
													l=FNEvalExpr(14,T1);
													if(*FNLA(T1) == ',')
														PROCCheck(',');
													}
		// se ce ne sono meno del necessario, ripete ultimo! bah direi ok :)
												// controllare SIZE e dare warning o errore...
												switch(myT2->size) {
													case 1:
														if(FNGetSize(l) > myT2->size)
															PROCWarn(2058);
														n+=subBytes("",1,l.v);
														break;
													case 2:
														if(FNGetSize(l) > myT2->size)
															PROCWarn(2058);
														n+=subBytes("",2,l.v);
														break;
													case 4:
														if(FNGetSize(l) > myT2->size)
															PROCWarn(2058);
														n+=subBytes("",4,l.v);
														break;
													case 8:
														if(FNGetSize(l) > myT2->size)
															PROCWarn(2058);
														n+=subBytes("",8,l.v);
														break;
													default:
														// struct nidificate, ecc...
														n+=subBytes("",myT2->size,l.v);
														break;
													}
												myT2=myT2->next;
												} while(myT2);
											}
										else {

												n+=subBytes("",myT->size,l.v);		// per ora!
											// GESTIRE per union! com'è?
											}
										PROCCheck(*myBuf1 == '<' ? '>' : '}');
										break;
									case 'D':
										// gestire DUP, ossia array
										break;
									case ',':
										break;
									case 0:
									default:
										PROCError(2054,"< o {");  
										break;
									}

							}
						else {
							if(!CPass) {
								l.valid=0;
								l.v=-1;
								PROCDclVar(T,l,VAR_RECORD,(uint32_t)myT);
								while(*FNLO(myBuf1))
									;
								}
							else {
								l.valid=0;
								l.v=0;
								PROCOut1("- ",theLine,FALSE);
								FNInitAggr(myT,TRUE);

								}
							}
						}
					else
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
 
int subStmt(const char *s,enum ADDR_MODE m,struct CODE **n) {
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
	  
  PROCError(2060,s);                     // do subito errore!
  return -1;
  }
     
int subBranch(const char *TS, const char *T2S, struct CODE *mn, int T, int k, struct SEGMENTED_VALUE l2, uint8_t c) {
  long l;
  int i;

//N.B. l'auto-estensione non funziona!!!  2024: a me sembra abb ok... segmenti a parte, PROVARE!
// infatti al primo giro gli offset valgono 0, mentre al secondo vengono risolti...
// questo impedisce di stabilire il valore per tempo.
#if GD24032
  if(!CPass) {
    l2.v=Seg[CurSeg-1].CurAddr+4;           // nella pass 0 salto fittizio
    l2.seg=CurSeg;
		}
#else
  if(!CPass) {
    l2.v=Seg[CurSeg-1].CurAddr+2;           // nella pass 0 salto fittizio
    l2.seg=CurSeg;
		}
#endif
  i=mn-Mnem;  
#if Z80
  if(i != 28/*JR*/ && i != 15/*DJNZ*/) {                   // gestisco JP cond,...

	  if(CPass && i==27 && (BranchOpt & 1) && Cond[T].m<0x20) {      // solo C, NC, Z, NZ

//			VERIFICARE CPass come in 6502!!

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
      PROCOut(mn->a[k].n | (c ? Cond[T].m : 0),LOBYTE(LOWORD(l2.v)),HIBYTE(LOWORD(l2.v)),l2.seg);   // con segm
      }
       
    }
  else {
    l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
//	    printf("ofs salto %lx\n",l2);
    if(l > 127 || l<-128) {
      if((BranchOpt & 2) && i!=15 /* no DJNZ */) {
        if(c)
		      PROCOut(mn->a[k].n | (c ? Cond[T ^ 1].m : 0),3,-1,0);
	      PROCOut(0xc3 /*JP*/,LOBYTE(LOWORD(l2.v)),HIBYTE(LOWORD(l2.v)),l2.seg);   // con segm
				Seg[CurSeg-1].len+=3;		// necessario xche al primo giro non c'era
        PROCWarn(4011,T2S);		// TS?? v. 6502 verificare
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
    if(BranchOpt) {
	    PROCOut(mn->a[k].n ^ (0x20 ^ (c ? 0x20 : 0)),3,-1,0);
	    PROCOut1("",NULL,TRUE);
	    PROCOut(0x4c /*JMP*/,LOBYTE(LOWORD(l2.v)),HIBYTE(LOWORD(l2.v)),l2.seg);   // con segm
	    PROCOut1("\n",NULL,TRUE);		// non è perfetto ma ok... 17/2/24
			Seg[CurSeg-1].len+=3;		// necessario xche al primo giro non c'era
      PROCWarn(4011,TS);
      }
    else
      PROCError(2063,TS);
    }
  else
    PROCOut(mn->a[k].n ^ (c ? 0x20 : 0),LOBYTE(LOWORD(l)),-1,0);
#elif I8051
  if(i==21 || i==22)
    PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));   // con segm
  else if(i==0 || i==3) {
    if(LOBYTE(LOWORD(l2)) & 0xf8)
      PROCError(2064);
    PROCOut(mn->a[k].n | ((*(((char *)&l2)+1) & 0x7) << 5),LOBYTE(LOWORD(l2)),-1,HIWORD(l2));   // con segm
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
      if(BranchOpt) {
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
#elif PIC==16
#elif PIC==18
	uint16_t n;
  if(i==15) {
	  l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
	  if(l > 1023 || l<-1024) {
			if(BranchOpt) {
				PROCOut1("",NULL,TRUE);
				n=0xef00 /*GOTO*/ | LOBYTE(LOWORD(l2.v));
				PROCOut(n,-1,l2.seg);   // con segm
				n=0xf000 /*GOTO*/ | ((l2.v >> 8) & 0xfff);
				PROCOut(n,-1,l2.seg);   // con segm
				PROCOut1("\n",NULL,TRUE);		// non è perfetto ma ok... 17/2/24
				Seg[CurSeg-1].len+=2;		// necessario xche al primo giro non c'era
				PROCWarn(4011,TS);
				}
			else
				PROCError(2063,TS);
	    }
		else {
			n=mn->a[k].n | (l & 0x7ff);
			PROCOut(n,-1,HIBYTE(HIWORD(l)));    // con segm.
			}
    }
  else {
	  l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
	  if(l > 127 || l<-128) {
			if(BranchOpt) {
			  if(l > 1023 || l<-1024) {
					n=(mn->a[k].n ^ 0x100) | 4;
					PROCOut(n,-1,0);
					PROCOut1("",NULL,TRUE);
					n=0xef00 /*GOTO*/ | LOBYTE(LOWORD(l2.v));
					PROCOut(n,-1,l2.seg);   // con segm
					n=0xf000 /*GOTO*/ | ((l2.v >> 8) & 0xfff);
					PROCOut(n,-1,HIWORD(l2.seg));   // con segm
					Seg[CurSeg-1].len+=4;		// necessario xche al primo giro non c'era
					}
				else {
					n=(mn->a[k].n ^ 0x100) | 2;
					PROCOut(n,-1,0);
					PROCOut1("",NULL,TRUE);
					n=0xd000 /*BRA*/ | (l & 0x7ff);
					PROCOut(n,-1,HIBYTE(HIWORD(l)));   // con segm
					Seg[CurSeg-1].len+=2;		// necessario xche al primo giro non c'era
					}
				PROCOut1("\n",NULL,TRUE);		// non è perfetto ma ok... 17/2/24
				PROCWarn(4011,TS);
				}
			else
				PROCError(2063,TS);
	    }
		else {
			n=mn->a[k].n | (l & 0x7f);
			PROCOut(n,-1,HIBYTE(HIWORD(l)));    // con segm.
			}
    }
#elif GD24032
  union GD24032_OPCODE gdop;
//  k=subStmt(TS,c ? ADDR_CONDIZ : ADDR_IMM17,&mn);         // Bxx / JR nn
	gdop.d=mn->a[k].n;
//	gdop.o4.condiz= T ? 1 : 0;		// implicito nell opcode
	gdop.mode2=ADDR_IMMEDIATE;
	l=(int32_t)(l2.v-Seg[CurSeg-1].CurAddr-4)/4;		// USARE SEGMENTO QUA??
	if(abs(l) > 0x1ffff) {
    if(BranchOpt) {
			gdop.size=0;
			gdop.o4.cond ^= 1;
			gdop.o4.vall=2;
			gdop.o4.valh=0;
			PROCOut2(gdop.d);    //Bxxinv +2
	    PROCOut1("",NULL,TRUE);
			gdop.d=0x80000000;
			gdop.size=2;
			PROCOut2(gdop.d,&l2);    //JMP l
	    PROCOut1("\n",NULL,TRUE);		// non è perfetto ma ok... 17/2/24
			Seg[CurSeg-1].len += 8;		// necessario xche al primo giro non c'era
      PROCWarn(4011,TS);
      }
    else
      PROCError(2063,TS);
		}
	else {
		gdop.size=0;
		gdop.o4.vall=LOBYTE(l);
		gdop.o4.valh=l >> 8;
		PROCOut2(gdop.d);    // 
		}
//	    printf("ofs salto %lx\n",l2);
#elif I8086
  if(i == 190/*JCXZ*/ || (i == 212/*LOOP*/ && CPULevel>=3)) {                   // gestisco Jcc cond,...		VERIFICARE Loop 386?

	  if(CPass && i==27 && (BranchOpt & 1)) {      // solo C, NC, Z, NZ

//			VERIFICARE CPass come in 6502!!
			// FINIRE QUA!

      l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
      if(l <= 127 && l>=-128) {          // jmp 00xx diventa jmp xx \\ nop VERIFICARE 8086
		    PROCWarn(4010,T2S);
//		    printf("c vale %d, cond %x\n",c,Cond[T].m);
        PROCOut(c ? (0x70 | Cond[T].m) : 0xeb/*jmp short*/,LOBYTE(LOWORD(l)));
        }
      else
        goto normBranch;
	    }
	  else {
normBranch:
      PROCOut(mn->a[k].n | (c ? Cond[T].m : 0),LOBYTE(LOWORD(l2.v)),HIBYTE(LOWORD(l2.v)));   // 
      }
       
    }
  else {
    l=(int16_t)(*((int16_t *)&l2)-Seg[CurSeg-1].CurAddr-2);
//	    printf("ofs salto %lx\n",l2);
    if(c) {			// se Jcc, solo +-127 max
			if(l > 127 || l<-128) {
				if((BranchOpt & 2) && i!=212 /* no LOOP */) {
					if(c)
						PROCOut(mn->a[k].n | (c ? Cond[T ^ 1].m : 0),3,-1,0);
					PROCOut(0xe9 /*JMP*/,LOBYTE(LOWORD(l2.v)),HIBYTE(LOWORD(l2.v)));   //
					Seg[CurSeg-1].len+=3;		// necessario xche al primo giro non c'era
					PROCWarn(4011,T2S);		// TS?? v. 6502 verificare
					}
				else
					PROCError(2063,T2S);
				}
	    else
		    PROCOut(mn->a[k].n | (c ? Cond[T].m : 0),LOBYTE(LOWORD(l)));
      }
		else {
			if(i==191 && l <= 127 && l>=-128) {
		    PROCOut(0xeb /*??? boh pare ok mn->a[k].n perché cmq il parm è relativo */,LOBYTE(LOWORD(l)),-1,0);
				}
			else if(l <= 32767 && l>=-32768) {
				l--;		// se jmp short mi sposto di 2 oltre CurAddr, se no di 3!
		    PROCOut(mn->a[k].n /*0xe9 /*mn->a[k].n perché cmq il parm è relativo */,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),0);
				}
			else {
		    PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2.v)),HIBYTE(LOWORD(l2.v)),LOBYTE(HIWORD(l2.v)));			// finire con jmp FAR
				}
      }
    }
#endif
                  
  return 0;
  }

int subBytes(const char *s, int m, uint64_t l) {
  int i,j,n;
  char myBuf[256],ch;

  if(*s == '\'' || *s == '\"') {
    if(!(m & 0xf0) && (m & 0xf ) != 1)	// la stringa OK solo per byte e string/ascii
      PROCError(2064);
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
#if PIC==16  /* || PIC==18 no */
      PROCOut(LOBYTE(ch),0x34,-1,0);      // RETLW
#else
      PROCOut(LOBYTE(ch));      // non CASTa i caratteri a int, 7/6/97 ?? boh 2024
#endif

      j++;
      } while(s[j] != s[0]);
    n=j-1;
    if(m == 0x11 || m==0x13) {     // asciiz e ascii (ST62)
      PROCOut(0);
      n++;
      if(*FNLA(myBuf))
        PROCError(2016);
      }
    }                 
  else {
		struct SEGMENTED_VALUE ll;
#if GD24032
		ll.seg=HIDWORD(l);
#else
		ll.seg=HIWORD(l);
#endif
    n=m;
		if(Seg[CurSeg-1].type != SEGMENT_VIRT) {
			switch(m) {
				case 0:                         // gestisco LABEL
					break;
				case 1:
					ll.v=LOBYTE(l);
					PROCOut(LOBYTE(l),-1,-1,ll.seg);       // no segm (2024) ???
					break;
    		case 2:
					ll.v=LOWORD(l);
					PROCOut(LOBYTE(ll.v),HIBYTE(ll.v),-1,ll.seg);       //
					break;
    		case 4:
	#if GD24032
					ll.v=LODWORD(l);
					PROCOut(LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),-1,ll.seg);       // 
					PROCOut(LOBYTE(HIWORD(l)),HIBYTE(HIWORD(l)),-1,ll.seg);
	#else
					ll.v=LOWORD(l);
					PROCOut(LOBYTE(ll.v),HIBYTE(ll.v),-1,ll.seg);       // 
					ll.v=HIWORD(l);
					PROCOut(LOBYTE(ll.v),HIBYTE(ll.v),-1,ll.seg);       // 
	#endif
					break;
    		case 8:
	#if GD24032
					ll.v=LODWORD(l);
					PROCOut(LOBYTE(LOWORD(ll.v)),HIBYTE(LOWORD(ll.v)),-1,ll.seg);       // 
					PROCOut(LOBYTE(HIWORD(ll.v)),HIBYTE(HIWORD(ll.v)),-1,ll.seg);
					ll.v=HIDWORD(l);
					PROCOut(LOBYTE(LOWORD(ll.v)),HIBYTE(LOWORD(ll.v)),-1,ll.seg);       // 
					PROCOut(LOBYTE(HIWORD(ll.v)),HIBYTE(HIWORD(ll.v)),-1,ll.seg);
	#else
					ll.v=LOWORD(LODWORD(l));
					PROCOut(LOBYTE(ll.v),HIBYTE(ll.v),-1,ll.seg);       // 
					ll.v=HIWORD(LODWORD(l));
					PROCOut(LOBYTE(ll.v),HIBYTE(ll.v),-1,ll.seg);       // 
					ll.v=LOWORD(HIDWORD(l));
					PROCOut(LOBYTE(ll.v),HIBYTE(ll.v),-1,ll.seg);       // 
					ll.v=HIWORD(HIDWORD(l));
					PROCOut(LOBYTE(ll.v),HIBYTE(ll.v),-1,ll.seg);       // 
	#endif
					break;
				case 0x10:                    // ds
					for(i=0; i<l; i++)
						PROCOut(' ');
					n=LODWORD(l);
					break;
    		}
			}
		else {
			if(Seg[CurSeg-1].BaseAddr == 0xffffffff)		// questo serve se non c'è stata ORG dopo SEGMENT/CSEG
				Seg[CurSeg-1].BaseAddr=Seg[CurSeg-1].CurAddr;
			switch(m) {
				case 0:                         // gestisco LABEL
					break;
				case 1:
    		case 2:
    		case 4:
    		case 8:
				  Seg[CurSeg-1].CurAddr+=m;
					break;
				case 0x10:                    // ds
					n=LODWORD(l);
				  Seg[CurSeg-1].CurAddr+=n;
					break;
    		}
   		}
    }      
    
  return n;
  }


int subCmpMnem(struct CODE *s1, struct CODE *s2) {

#if Z80  
  return stricmp(s1->s,s2->s);
#elif _6502
  return stricmp(s1->s,s2->s);
#elif I8051
  return stricmp(s1->s,s2->s);
#elif ST62
  return stricmp(s1->s,s2->s);
#elif PIC==16
  return stricmp(s1->s,s2->s);
#elif PIC==18
  return stricmp(s1->s,s2->s);
#elif GD24032
	if(toupper(*s1->s)=='B')
		// gestire condizioni 2025
		FNIsCond(s1->s+1)
		;
  return stricmp(s1->s,s2->s);
#elif I8086
	if(toupper(*s1->s)=='J') {
		if(FNIsCond(s1->s+1)) {
			char mybuf[8];
			mybuf[0]='J';  mybuf[1]=0;
		  return stricmp(mybuf,s2->s);
			}
		else
			goto norm;
		}
	else if(toupper(*s1->s)=='S' && toupper(*(s1->s+1))=='E' && toupper(*(s1->s+2))=='T') {		//		SET
		if(FNIsCond(s1->s+3)) {
			char mybuf[8];
			mybuf[0]='S'; mybuf[1]='E'; mybuf[2]='T';  mybuf[3]=0;
		  return stricmp(mybuf,s2->s);
			}
		else
			goto norm;
		}
	else if(toupper(*s1->s)=='C' && toupper(*(s1->s+1))=='M' && toupper(*(s1->s+2))=='O'
		 && toupper(*(s1->s+3))=='V' && toupper(*(s1->s+4))=='C') {		//		CMOVC
		if(FNIsCond(s1->s+5)) {
			char mybuf[8];
			mybuf[0]='C'; mybuf[1]='M'; mybuf[2]='O'; mybuf[3]='V'; mybuf[4]='C';  mybuf[3]=0;
		  return stricmp(mybuf,s2->s);
			}
		else
			goto norm;
		}
	else if(toupper(*s1->s)=='F' && toupper(*(s1->s+1))=='C' && toupper(*(s1->s+2))=='M'
		 && toupper(*(s1->s+3))=='O' && toupper(*(s1->s+4))=='V' && toupper(*(s1->s+4))=='C') {		//		FCMOVC
		if(FNIsCond(s1->s+6)) {
			char mybuf[8];
			mybuf[0]='F'; mybuf[1]='C'; mybuf[2]='M'; mybuf[3]='O'; mybuf[4]='V'; mybuf[5]='C';  mybuf[6]=0;
		  return stricmp(mybuf,s2->s);
			}
		else
			goto norm;
		}
	else if(toupper(*s1->s)=='L' && toupper(*(s1->s+1))=='O' && toupper(*(s1->s+2))=='O' && toupper(*(s1->s+3))=='P') {		//		LOOP 286
		if(FNIsCond(s1->s+4)) {
			char mybuf[8];
			mybuf[0]='L'; mybuf[1]='O'; mybuf[2]='O'; mybuf[2]='P';  mybuf[3]=0;
		  return stricmp(mybuf,s2->s);
			}
		else
			goto norm;
		}
	else
norm:
	  return stricmp(s1->s,s2->s);
#endif  
  }


#if GD24032
uint64_t collectRegisters(void) {
	uint64_t t;
	int8_t i1,t2;
	int i;
	int8_t go=0;
	char S[64];
	struct OPERAND Oper;

	memset(&Oper,0,sizeof(struct OPERAND));
	t=0;
	i1=t2=-1;

	while(!go) {
		switch(FNGetAritElem(S,&Oper,0)) {
			case ARITM_VAR:
				i=(Reg[Oper.T].m - Reg[0].m);
				if(i1==-1) {
					t2=i;
					t |= (1 << t2);
					}
				else {
					if(i1<=i) {
						i1=t2;
						for(i1; i1<=i; i1++)
							t |= (1 << i1);
						i1=-1;
						}
					else
						PROCError(2062,S);
					}
				break;
			case ARITM_OPER:
				switch(Oper.op) {
					case 1:
						if(!strcmp(S,"}")) {
							go=1;
							continue;
							}
						break;
					case 2:
						if(!strcmp(S,"-")) {
							if(t2>=0) {
								i1=t2;
								continue;
								}
							}
						break;
					case 15:
						if(!strcmp(S,",")) {
							continue;
							}
						break;
					}
			default:
				PROCError(2062,S);
				break;
			}
		}
	theLinePtr--;

	return t;
	}
#endif

#if I8086
uint8_t computeAddressing(int8_t V,int8_t T,int8_t Tt,uint32_t l,const char *TS) {
	uint8_t t=6;			// inizio con indirizz. solo offset...

	if(V==VQ_REGISTER) {
		if(Reg[T].m == 11) {		// BX
			if(abs(l)>128)			// sarebbe +127/-128 ! sistemare?!
				t=0x87;
			else if(l)
				t=0x47;
			else
				t=7;
			}
		else if(Reg[T].m == 13) {		// BP
			if(abs(l)>128)			// sarebbe +127/-128 ! sistemare?!
				t=0x86;
			else
				t=0x46;
			}
		else if(Reg[T].m == 14) {		// SI
			if(abs(l)>128)			// sarebbe +127/-128 ! sistemare?!
				t=0x84;
			else if(l)
				t=0x44;
			else
				t=0x4;
			}
		else if(Reg[T].m == 15) {		// DI
			if(abs(l)>128)			// sarebbe +127/-128 ! sistemare?!
				t=0x85;
			else if(l)
				t=0x45;
			else
				t=0x5;
			}
		else 
		  PROCError(2062,TS);			// 
		if(Tt) {
			Tt--;
			if(Reg[Tt].m == 11) {		// BX
				if((t & 0xf)==4 || (t & 0xf)==5) {
					t -= 4;				// SI o DI -> BX+SI o BX+DI
					}
				else 
					PROCError(2062,TS);			// 
				}
			else if(Reg[Tt].m == 13) {		// BP
				if((t & 0xf)==4 || (t & 0xf)==5) {
					t -= 2;				// SI o DI -> BP+SI o BP+DI
					}
				else 
					PROCError(2062,TS);			// 
				}
			else if(Reg[Tt].m == 14) {		// SI
				if(t==0x46 || t==0x86) {		// BP -> BP+SI
					t -= 4;
					}
				else if(t==7) {
					t = 0;					// BX -> BX+SI
					}
				else 
					PROCError(2062,TS);			// 
				}
			else if(Reg[Tt].m == 15) {		// DI
				if(t==0x46 || t==0x86) {		// BP -> BP+DI
					t -= 3;
					}
				else if(t==7) {
					t = 1;					// BX -> BX+DI
					}
				else 
					PROCError(2062,TS);			// 
				}
			else 
				PROCError(2062,TS);			// 
			}
		}

	return t;
	}
#endif



void gotoEOL() {
	char TS[256];

	while(*FNLO(TS))
		;
	}

int FNIsStmt(const char *parm) {
  int T1,n;
	int8_t oi;
  register int i,j,k;
///*struct SEGMENTED_VALUE*/  long l,l2,l3;
	int Mnn;
  int8_t Impl,Is2,bt=0;               // se indiretto, se implicito, se c'è il secondo, tipo parentesi per ind.
  char TS[256],T1S[256],T2S[256],myBuf[256],myBuf1[256];
#if ST62 || PIC>=18 || GD24032
  char T3S[256];        // ST e PIC18 han fino a 3 operandi.. GD24032 a volte; alcuni 80386+ anche!
  int8_t Is3;
#endif
#if GD24032
  char T4S[256];        // GD24032 a volte 4
  int8_t Is4;
#endif
  long OldTextp,p,p1;
  struct CODE mc,*mn;
	struct OPERAND o1,o2,o3;
#if GD24032
  union GD24032_OPCODE gdop;
	uint8_t forcedSize,useFlags,useCondiz,op2negated;
	struct OPERAND o4;
#endif
#if I8086
  char T3S[256];        // 
  int8_t Is3;
	uint8_t forcedSize;
#endif

nextStatement:
	OldTextp=theLinePtr;
  oi=bInizioRiga;
  FNLO(TS);
//  mn=j=0;
  
  if(!*TS) {
		if(parm)
			strcpy(TS,parm);
		}
	else {
//#if ST62		// bah vabbe' 2024 per tutti!
		if(*TS == '.')
			FNLO(TS+1);       // salto il punto, (messo nelle direttive
//#endif
		}
  if(!*TS) 
    goto noStmt;

  if(UNDEFD)
    goto soloIFS;

  mc.s=TS;
#if Z80  
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,67,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif _6502
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,64,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif I8051
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,44,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif ST62
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,31,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif PIC==16
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,37,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif PIC==18
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,77,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif GD24032
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,105,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#elif I8086
  mn=(struct CODE *)bsearch((void *)&mc,Mnem,435,sizeof(Mnem[0]),(int (*)(const void *, const void *))subCmpMnem);
#endif  

  if(mn /*j*/) {
    Mnn=mn-Mnem;
    if((Seg[CurSeg-1].type  & 0xff00) & (SEGMENT_DATA & 0xff00))
      PROCError(2102);
	  if(OutList) {    
	    PROCOut1("> ",theLine,FALSE);
	    }
    
#if Z80  
//		if(!CPass)
//		if(Mnn==0)
//			PROCWarn(2067,mn->s);
#elif _6502
		if(!CPass)
			if(Mnn==10 || Mnn==38 || Mnn==39 || Mnn==42 || Mnn==43 || Mnn==55 || Mnn==58 || Mnn==59)
				PROCWarn(2067,mn->s);
#elif PIC==18
		if(!CPass)
			if(Mnn==0 || Mnn==4 || Mnn==22 || Mnn==44 || Mnn==45 || Mnn==53 || Mnn==65 || Mnn==67)
				// EXTENDED MODE    
				PROCWarn(2067,"Extended!");
#elif I8086
			if(!CPass) {
				if(Mnn==314 || Mnn==352) {
					//finire
//					PROCError(2067,"non valido per questa CPU");
					if(CPULevel<1)
						PROCError(2067,"non valido per questa CPU");
					}
				if(Mnn==81 || Mnn==202 || Mnn==316 || Mnn==353 || Mnn==355 || Mnn==363) {
					if(CPULevel<2)
						PROCError(2067,"non valido per questa CPU");
					}
				if(Mnn==315 || Mnn==354) {
					if(CPULevel<3)
						PROCError(2067,"non valido per questa CPU");
					}
				if(Mnn==185) {
					if(CPULevel<4)
					PROCError(2067,"non valido per questa CPU");
					}
				if(Mnn==34) {
					if(CPULevel<6)
					PROCError(2067,"non valido per questa CPU");
					}
				}
			if((Mnn>=364 && Mnn<=368) || Mnn==208) {		// REPnn, LOCK van di seguito
				k=subStmt(TS,ADDR_IMPL,&mn);  
				PROCOut(mn->a[k].n);
				// viene spezzata su due righe ma è complicato modificare...
				goto nextStatement;
				}
#endif

    Impl=FALSE;
    Is2=FALSE;
#if ST62 || PIC>=18 || GD24032
    Is3=FALSE;
#endif
#if GD24032
    Is4=FALSE;
#endif

		memset(&o1,0,sizeof(struct OPERAND));
		memset(&o2,0,sizeof(struct OPERAND));
		memset(&o3,0,sizeof(struct OPERAND));
#if GD24032
		memset(&o4,0,sizeof(struct OPERAND));
#endif
#if I8086
		forcedSize=0;
#endif
#if GD24032
		forcedSize=0;
		useFlags=0;
		useCondiz=0;
		op2negated=0;
    FNLA(T1S);
		if(*T1S == '.') {
rifo_extra:
			FNLO(T1S);
	    FNLO(T1S);
			switch(toupper(*T1S)) {
				case 'B':
					forcedSize=1;
					break;
				case 'W':
					forcedSize=2;
					break;
				case 'D':
					forcedSize=4;
					break;
				case 'Q':
					forcedSize=8;
					break;
				case 'F':
					useFlags=1;
					break;
				default:
					PROCError(1006,T1S);
					break;
				}
	    FNLA(T1S);
			if(*T1S == '.') {
				if(!useFlags)
					goto rifo_extra;
				}
			}
#endif
rifoIsVar:
    p=theLinePtr;
    *T1S=0;
#if ST62 || PIC>=18 || GD24032
		o1.S=o2.S=o3.S=0;
		o1.T=o2.T=o3.T=0;
		o1.V=o2.V=o3.V=0;
		o1.op=o2.op=o3.op=0;
		o1.l.valid=o2.l.valid=o3.l.valid=FALSE;
#else
		o1.S=o2.S=0;
		o1.T=o2.T=0;
		o1.V=o2.V=0;
		o1.op=o2.op=0;
		o1.l.valid=o2.l.valid=FALSE;
#endif
    switch(FNGetAritElem(T1S,&o1,0)) {
      case ARITM_EOL:
        Impl=TRUE;
        break;
      case ARITM_CONST: 
isVar:
//        V1=VQ_OP_VAR;
				if(o1.V==VQ_REGISTER) {  
					o1.T;
					o1.S=4;
					}
				else {
					theLinePtr=p;
					o1.l=FNEvalExpr(14,T1S);
					if(!o1.l.valid)
						goto rifoIsVar;
					if(o1.Ind) {
						o1.Sptr=FNGetSize(o1.l);
						o1.S=0/*INT_SIZE*/;
						}
					else {
						o1.S=FNGetSize(o1.l);
						}
//					o1.l.valid=TRUE;
					}
        break;
      case ARITM_VAR:
        if(o1.V>=VQ_OP_VAR) {
					uint16_t oS=o1.S;		// 2025 x 8086 , verificare altri...
					theLinePtr=p;
					o1.l=FNEvalExpr(14,T1S);
					if(!o1.l.valid)
						goto rifoIsVar;
					if(o1.Ind) {
						o1.Sptr=(uint8_t)oS /*FNGetSize(o1.l)*/;
						o1.S=0/*INT_SIZE*/;
						}
					else {
						o1.S=oS /*FNGetSize(o1.l)*/;
						}
					}
        break;
      case ARITM_OPER:
	      switch(o1.op) {
					case 1:
#if I8051
					{
#endif
#if GD24032
						if(!strcmp(T1S,"++")) {
							if(!o1.preOp)
								o1.preOp=1;
							else
								PROCError(2062,T1S);
							goto rifoIsVar;
							}
						else if(!strcmp(T1S,"--")) {
							if(!o1.preOp)
								o1.preOp=-1;
							else
								PROCError(2062,T1S);
							goto rifoIsVar;
							}
						else if(!strcmp(T1S,"{")) {
							o1.Ind=TRUE;
							o1.Sptr=PTR_SIZE;
						o1.S=0/*INT_SIZE*/;
							bt='}';
							o1.l.v=collectRegisters();
							}
						else {
#endif
						o1.Ind=TRUE;
						o1.Sptr=PTR_SIZE;
						o1.S=0/*INT_SIZE*/;
						bt=*T1S == '(' ? ')' : ']';
	//					l=*TS;
							goto rifoIsVar;
	//					FNLA(myBuf);
#if I8051 || GD24032
			      }
#endif
						break;
#if I8086
					case 2:
						switch(tolower(*T1S)) {
							case 'b':
								forcedSize=1;
								goto rifoIsVar;
								break;
							case 'w':
								if(tolower(T1S[1])=='o') {		// word
									forcedSize=2;
									goto rifoIsVar;
									}
								else				// width
								  goto isVar;
								break;
							case 'd':
								forcedSize=4;
								goto rifoIsVar;
								break;
							case 'q':
								forcedSize=8;
								goto rifoIsVar;
								break;
							case 'p':
// no inutile								o1.Ind=1;		// bah, sì... 
								goto rifoIsVar;
								break;
		          case 'n':			// NEAR
								if(tolower(T1S[1])=='e') {		// near
									if(CPULevel>=3)
										forcedSize=4;
									else
										forcedSize=2;
									goto rifoIsVar;
									}
								else				// not
								  goto isVar;
								break;
		          case 'f':			// FAR
								if(CPULevel>=3)
		              forcedSize=6;
								else
		              forcedSize=4;
								goto rifoIsVar;
		            break;
							default:
							  goto isVar;
								break;
							}
						break;
#endif
#if GD24032
					case 4:
						if(*T1S=='+')
							goto rifoIsVar;
						else
							PROCError(2062,T1S);
						break;
#endif
#if PIC==18
					case 3:
						if(*T1S=='*')
							o1.V=VQ_REGISTER;		// truschino TBLRD
						goto isVar;
						break;
					case 4:
						if(*T1S=='+')
							o1.V=VQ_REGISTER;		// truschino TBLRD
						goto isVar;
						break;
#endif
					default:
						goto isVar;
						break;
					}
	      break;
	    }  
#if GD24032
		if(o1.V==VQ_CONDIZ) {                                // jr nz, 
			if(!useCondiz) {
				useCondiz=o1.T+1;
				FNLA(T1S);
				if(*T1S)		// alcune istruzioni possono avere Impl (0 parm) e CONDIZ (tipo RET
					PROCCheck(',');
				goto rifoIsVar;
				}
			}
#endif


#if GD24032
	  if(o1.Ind) {
rifo_post1:
	    FNLA(T1S);
// in effetti ci vorrebbe uno "stack" di operandi...
			if(!strcmp(T1S,"++")) {
				if(!o1.postOp) {
					o1.postOp=1;
					FNLO(T1S);
					goto rifo_post1;
					}
				else
					PROCError(2062,T1S);
				}
			else if(!strcmp(T1S,"--")) {
				if(!o1.postOp) {
					o1.postOp=-1;
					FNLO(T1S);
					goto rifo_post1;
					}
				else
					PROCError(2062,T1S);
				}
			else if(!strcmp(T1S,"}")) {
				}
			else if(!strcmp(T1S,"!")) {
				o1.addr64=1;
				FNLO(T1S);
				goto rifo_post1;
				}

			if(*T1S != bt) {
				int8_t oldV=o1.V,oldT=o1.T,oldValid=o1.l.valid,newT=0;
/*				if(*T1S != ',')
					o1.l=FNEvalExpr(14,T1S);*/
				if(*T1S == ',')
					PROCCheck(',');

//					o1.l=FNEvalExpr(14,T1S);
get2reg:
		    p=theLinePtr;
				switch(FNGetAritElem(T1S,&o1,0)) {
					case ARITM_EOL:
						break;
					case ARITM_CONST: 
		        theLinePtr=p;
						o1.l=FNEvalExpr(14,T1S);
						if(!o1.l.valid)
							PROCError(2062);
						if(o1.Ind) {
							o1.Sptr=FNGetSize(o1.l);
						o1.S=0/*INT_SIZE*/;
							}
						else {
							o1.S=FNGetSize(o1.l);
							}
//						goto get2reg;
						break;
					case ARITM_VAR:
						if(o1.V==VQ_REGISTER) {
							if(newT || oldT)		// se c'è già un registro..
								o1.reg2reg=o1.T+1;
							else
								newT=o1.T;
							}
						else {
							theLinePtr=p;
							o1.l=FNEvalExpr(14,T1S);
							if(!o1.l.valid)
								PROCError(2062);
							if(o1.Ind) {
								o1.Sptr=FNGetSize(o1.l);
						o1.S=0/*INT_SIZE*/;
								}
							else {
								o1.S=FNGetSize(o1.l);
								}
							}
						goto get2reg;
						break;
					case ARITM_OPER:
						switch(o1.op) {
							case 1:
								if(*T1S != bt)
									PROCError(2062,T1S);
								break;
							case 4:
								if(*T1S != '+')
									PROCError(2062,T1S);
								break;
							case 15:
								break;
							default:
								PROCError(2062,T1S);
								break;
							}
						goto get2reg;
						break;
					}  
				o1.T=newT ? newT : oldT;
				o1.V=oldV;
//				o1.l.valid=oldValid;
				if(o1.reg2reg || o1.T)		// ribadisco perché si perde!
					o1.V=VQ_REGISTER;

				// VA SALVATO anche il numero, T!! usare un o1 di scorta??

// INSERIRE QUA gestione dei 2 registri + offset! e anche ev. prima offset poi registro che ora non va...
//					ossia usare     i=FNGetAritElem(T1S,&o1,0);

				}
      PROCCheck(bt);
			bt=0;
	    }		// Ind1
	  FNLA(T1S);
		if(*T1S && *T1S!=',')
			PROCError(2060,T1S);

//    fprintf(stderr,"\tEVx %lx\n",l);

#elif I8086
	  if(o1.Ind) {
			char mysign=0;		// per offset negativi...

			FNLA(T1S);

			if(*T1S != bt) {
				int8_t oldV=o1.V,oldT=o1.T,oldValid=o1.l.valid,newT=0;
/*				if(*T1S != ',')
					o1.l=FNEvalExpr(14,T1S);*/
				if(*T1S == ',')
					PROCCheck(',');

//					o1.l=FNEvalExpr(14,T1S);
get2reg:
		    p=theLinePtr;
				switch(FNGetAritElem(T1S,&o1,0)) {
					case ARITM_EOL:
						break;
					case ARITM_CONST: 
		        theLinePtr=p;
						o1.l=FNEvalExpr(14,T1S);
						if(!o1.l.valid)
							PROCError(2062);
						if(o1.Ind) {
							o1.Sptr=FNGetSize(o1.l);
						o1.S=0/*INT_SIZE*/;
							}
						else {
							o1.S=FNGetSize(o1.l);
							}
						if(mysign)
							o1.l.v=-o1.l.v;
//						goto get2reg;
						break;
					case ARITM_VAR:
						if(o1.V==VQ_REGISTER) {
							if(newT || oldT)		// se c'è già un registro..
								o1.reg2reg=o1.T+1;
							else
								newT=o1.T;
							}
						else {
							theLinePtr=p;
							o1.l=FNEvalExpr(14,T1S);
							if(!o1.l.valid)
								PROCError(2062);
							if(o1.Ind) {
								o1.Sptr=FNGetSize(o1.l);
						o1.S=0/*INT_SIZE*/;
								}
							else {
								o1.S=FNGetSize(o1.l);
								}
							}
						goto get2reg;
						break;
					case ARITM_OPER:
						switch(o1.op) {
							case 1:
								if(*T1S != bt)
									PROCError(2062,T1S);
								break;
							case 2:		// su 8086 accetto offset negativi... verificare altri! 2025
								if(*T1S != '-')
									PROCError(2062,T1S);
								mysign=1;
								break;
							case 4:
								if(*T1S != '+' && *T1S != '-')
									PROCError(2062,T1S);
								mysign=*T1S == '-';
								break;
							case 15:
								break;
							default:
								PROCError(2062,T1S);
								break;
							}
						goto get2reg;
						break;
					}  
				o1.T=newT ? newT : oldT;
				o1.V=oldV;
//				o1.l.valid=oldValid;
				if(o1.reg2reg || o1.T)		// ribadisco perché si perde!
					o1.V=VQ_REGISTER;

				// VA SALVATO anche il numero, T!! usare un o1 di scorta??

// INSERIRE QUA gestione dei 2 registri + offset! e anche ev. prima offset poi registro che ora non va...
//					ossia usare     i=FNGetAritElem(T1S,&o1,0);

				}
      PROCCheck(bt);
			bt=0;
	    }		// Ind1

	  FNLA(TS);
		if(*TS) {
			if(*TS==':') {		// gestisco segment override qua! perché non arriva come operatore in questo caso...
				PROCCheck(":");
				if(toupper(T1S[1])=='S' && (T1S[2]==0)) {			// v. anche segment override sciolto
					i=FNIsReg(T1S);
					if(i && i & 0x100) {			// reg 16 bit
						i--;
						i &= 0xff;
						if(Reg[i].m>=16 && Reg[i].m<=19) {
							PROCOut(0x26 | ((i & 3) << 3), -1);
							goto rifoIsVar;
							}
						else if(Reg[i].m>=40 && Reg[i].m<=41) {
							if(CPULevel>=3) {
								PROCOut(0x64 | (i & 1), -1);
								goto rifoIsVar;
								}
							else
								PROCError(2067,T1S);
							}
						}
					else
						PROCError(2061,T1S);
					}
				}
			else if(*TS!=',')
				PROCError(2060,TS);
			}

//    fprintf(stderr,"\tEVx %lx\n",l);

#else
// MIGLIORATA gestione extra e porcherie varie a fine riga, usare anche in altre CPU!

	  if(o1.Ind) {
	    FNLA(T1S);

			if(*T1S != bt) {
				if(*T1S != ',') {
					o1.l=FNEvalExpr(14,T1S);
					if(!o1.l.valid)
						PROCError(2062);
					}
				}
#if _6502
			else {
				PROCCheck(bt);

				// QUA COME FACEVA AD ANDARE?? in effetti funziona lo stesso, è gestito sotto... mah vabbe'
//					credo idem usare     i=FNGetAritElem(T1S,&o1,0);

				bt=0;
				}
#else
      PROCCheck(bt);
			bt=0;
#endif

	    }
//    fprintf(stderr,"\tEVx %lx\n",l);
#endif

    
    if(Impl) {
#if GD24032
			uint32_t n;
#else
			uint16_t n;
#endif
      k=subStmt(TS,ADDR_IMPL,&mn);
      if(k>=0) {
				n=mn->a[k].n;
#if PIC==16 || PIC==18
				PROCOut2(n,-1,0);    // 
#elif GD24032
				gdop.d=mn->a[k].n;
				if(Mnn==69 || Mnn==71) {		// gestisco RET
					if(useCondiz) {		// già sopra??
						gdop.condiz=1;
						gdop.o3.cond=useCondiz-1;
						}
					}
				PROCOut2(gdop.d);    // 
#else
				PROCOut(LOBYTE(n));    // 
#endif
				}
      }
    else {
      FNLA(T2S);
      if(*T2S) {
#if PIC==18
		    if(*T2S=='*' || *T2S=='+')		// per TBLRD/WT
	        ;
				else
#endif
        PROCCheck(',');
#if GD24032
	      FNLA(T2S);
				if(*T2S=='-')	{		// per MAS/MSS ...
					uint32_t l;
					OldTextp=theLinePtr;
					PROCCheck('-');
					l=theLinePtr;
					i=FNGetAritElem(T2S,&o2,0);
					if(o2.V==VQ_REGISTER) {
						if(Mnn==55)		// MSS
							op2negated=1;
						else
							PROCError(2062,T2S);
						theLinePtr=l;
						}
					else
						theLinePtr=OldTextp;
					}
#endif
        Is2=TRUE;
        o2.Ind=FALSE;

rifoIsVar2:
		    p=theLinePtr;
		    *T2S=0;
#if ST62 || PIC>=18 || GD24032
				o2.S=o3.S=0;
				o2.T=o3.T=0;
				o2.V=o3.V=0;
				o2.op=o3.op=0;
#else
				o2.S=0;
				o2.T=0;
				o2.V=0;
				o2.op=0;
#endif
		    i=FNGetAritElem(T2S,&o2,0);
#if _6502
				if(bt)
					PROCCheck(bt);
				bt=0;
#endif
		    switch(i) {
		      case ARITM_EOL:
//		        Impl=TRUE;
			      PROCError(2060);          // se c'è virgola e poi [fineriga], errore!
		        break;
		      case ARITM_CONST:
isVar2:
						if(o2.V==VQ_REGISTER) {  
							o2.T;
							o2.S=4;
							}
						else {
//							o2.V=VQ_OP_VAR;
							theLinePtr=p;
							o2.l=FNEvalExpr(14,T2S);
							if(!o2.l.valid)
								PROCError(2062);
							if(o2.Ind) {
								o2.Sptr=FNGetSize(o2.l);
						o2.S=0/*INT_SIZE*/;
								}
							else {
								o2.S=FNGetSize(o2.l);
								}
//							o2.l.valid=TRUE;
							}
		        break;
		      case ARITM_VAR:
						if(o2.V>=VQ_OP_VAR) {
							uint16_t oS=o2.S;		// 2025 x 8086 , verificare altri...
							theLinePtr=p;
							o2.l=FNEvalExpr(14,T2S);
							if(!o2.l.valid)
								PROCError(2062);
							if(o2.Ind) {
								o2.Sptr=(uint8_t)oS;
						o2.S=0/*INT_SIZE*/;
								}
							else {
								o2.S=oS;
								}
		          }
		        break;
		      case ARITM_OPER:
			      switch(o2.op) {
							case 1:
#if GD24032
								if(!strcmp(T2S,"++")) {
									if(!o2.preOp)
										o2.preOp=1;
									else
										PROCError(2062,T2S);
									goto rifoIsVar2;
									}
								else if(!strcmp(T2S,"--")) {
									if(!o2.preOp)
										o2.preOp=-1;
									else
										PROCError(2062,T2S);
									goto rifoIsVar2;
									}
								else if(!strcmp(T2S,"{")) {
									o2.Ind=TRUE;
									o2.Sptr=PTR_SIZE;
						o2.S=0/*INT_SIZE*/;
									bt='}';
									o2.l.v=collectRegisters();
									}
								else {
#endif
									o2.Ind=TRUE;
									o2.Sptr=PTR_SIZE;
						o2.S=0/*INT_SIZE*/;
     							bt=*T2S == '(' ? ')' : ']';
									goto rifoIsVar2;
#if GD24032
						      }
#endif
								break;
#if I8086
							case 2:
								switch(tolower(*T2S)) {
									case 'b':		// byte
										forcedSize=1;
										goto rifoIsVar2;
										break;
									case 'w':
										if(tolower(T2S[1])=='o') {		// word
											forcedSize=2;
											goto rifoIsVar2;
											}
										else			// width...
								      goto isVar2;
										break;
									case 'd':		// dword
										forcedSize=4;
										goto rifoIsVar2;
										break;
									case 'q':		// qword
										forcedSize=8;
										goto rifoIsVar2;
										break;
									case 'p':		// ptr
// no, inutile										o2.Ind=1;		// bah, sì... 
										goto rifoIsVar2;
										break;
									case 'n':			// NEAR
										if(tolower(T2S[1])=='e') {		// near
											if(CPULevel>=3)
												forcedSize=4;
											else
												forcedSize=2;
											goto rifoIsVar2;
											}
										else
								      goto isVar2;
										break;
									case 'f':			// FAR
										if(CPULevel>=3)
											forcedSize=6;
										else
											forcedSize=4;
										goto rifoIsVar2;
										break;
									default:
							      goto isVar2;
										break;
									}
								break;
#endif
#if PIC==18
							case 3:
								if(*T2S=='*')
									o2.V=VQ_REGISTER;		// truschino TBLRD
								break;
							case 4:
								if(*T2S=='+')
									o2.V=VQ_REGISTER;		// truschino TBLRD
								goto isVar;
								break;
#endif
							default:
					      goto isVar2;
								break;
							}
			      break;
			    }

#if GD24032
// in effetti ci vorrebbe uno "stack" di operandi...
	  	  if(o2.Ind) {
rifo_post2:
		      FNLA(T2S);
					if(!strcmp(T2S,"++")) {
						if(!o2.postOp) {
							o2.postOp=1;
							FNLO(T2S);
							goto rifo_post2;
							}
						else
							PROCError(2062,T2S);
						}
					else if(!strcmp(T2S,"--")) {
						if(!o2.postOp) {
							o2.postOp=-1;
							FNLO(T2S);
							goto rifo_post2;
							}
						else
							PROCError(2062,T2S);
						}
					else if(!strcmp(T2S,"}")) {
						}
					else if(!strcmp(T2S,"!")) {
						o2.addr64=1;
						FNLO(T2S);
						goto rifo_post2;
						}

					if(*T2S != bt) {
						int8_t oldV=o2.V,oldT=o2.T,oldValid=o2.l.valid,newT=0;
						if(*T2S == ',')
							PROCCheck(',');
/*		      if(*T2S != bt)
		        o2.l=FNEvalExpr(14,T2S);
	        PROCCheck(bt);*/
//    fprintf(stderr,"\tEVx2 %lx\n",l2);

get2reg2:
						p=theLinePtr;
						switch(FNGetAritElem(T2S,&o2,0)) {
							case ARITM_EOL:
								break;
							case ARITM_CONST: 
								theLinePtr=p;
								o2.l=FNEvalExpr(14,T2S);
								if(!o2.l.valid)
									PROCError(2062);
								if(o2.Ind) {
									o2.Sptr=FNGetSize(o2.l);
						o2.S=0/*INT_SIZE*/;
									}
								else {
									o2.S=FNGetSize(o2.l);
									}
//								o2.l.valid=TRUE;
//								goto get2reg2;
								break;
							case ARITM_VAR:
								if(o2.V==VQ_REGISTER) {
									if(newT || oldT)		// se c'è già un registro..
										o2.reg2reg=o2.T+1;
									else
										newT=o1.T;
									}
								else {
									theLinePtr=p;
									o2.l=FNEvalExpr(14,T2S);
									if(!o2.l.valid)
										PROCError(2062);
									if(o2.Ind) {
										o2.Sptr=FNGetSize(o2.l);
						o2.S=0/*INT_SIZE*/;
										}
									else {
										o2.S=FNGetSize(o2.l);
										}
//									o2.l.valid=TRUE;
									}
								goto get2reg2;
								break;
							case ARITM_OPER:
								switch(o2.op) {
									case 1:
										if(*T2S!=bt)
											PROCError(2062,T2S);
										break;
									case 4:
										if(*T2S!='+')
											PROCError(2062,T2S);
										break;
									case 15:
										break;
									default:
										PROCError(2062,T2S);
										break;
									}
								goto get2reg2;
								break;
							}  
						o2.T=newT ? newT : oldT;
						o2.V=oldV;
//						o2.l.valid=oldValid;
						if(o2.reg2reg || o2.T)		// ribadisco perché si perde!
							o2.V=VQ_REGISTER;

						}
	        PROCCheck(bt);
					}			// Ind2
	      FNLA(T2S);
				if(*T2S && *T2S!=',')
					PROCError(2060,T2S);

#elif I8086
	  	  if(o2.Ind) {
					char mysign=0;		// per offset negativi...

		      FNLA(T2S);

					if(*T2S != bt) {
						int8_t oldV=o2.V,oldT=o2.T,oldValid=o2.l.valid,newT=0;
						if(*T2S == ',')
							PROCCheck(',');
/*		      if(*T2S != bt)
		        o2.l=FNEvalExpr(14,T2S);
	        PROCCheck(bt);*/
//    fprintf(stderr,"\tEVx2 %lx\n",l2);

get2reg2:
						p=theLinePtr;
						switch(FNGetAritElem(T2S,&o2,0)) {
							case ARITM_EOL:
								break;
							case ARITM_CONST: 
								theLinePtr=p;
								o2.l=FNEvalExpr(14,T2S);
								if(!o2.l.valid)
									PROCError(2062);
								if(o2.Ind) {
									o2.Sptr=FNGetSize(o2.l);
						o2.S=0/*INT_SIZE*/;
									}
								else {
									o2.S=FNGetSize(o2.l);
									}
								if(mysign)
									o2.l.v=-o2.l.v;
//								o2.l.valid=TRUE;
//								goto get2reg2;
								break;
							case ARITM_VAR:
								if(o2.V==VQ_REGISTER) {
									if(newT || oldT)		// se c'è già un registro..
										o2.reg2reg=o2.T+1;
									else
										newT=o1.T;
									}
								else {
									theLinePtr=p;
									o2.l=FNEvalExpr(14,T2S);
									if(!o2.l.valid)
										PROCError(2062);
									if(o2.Ind) {
										o2.Sptr=FNGetSize(o2.l);
						o2.S=0/*INT_SIZE*/;
										}
									else {
										o2.S=FNGetSize(o2.l);
										}
//									o2.l.valid=TRUE;
									}
								goto get2reg2;
								break;
							case ARITM_OPER:
								switch(o2.op) {
									case 1:
										if(*T2S!=bt)
											PROCError(2062,T2S);
										mysign=1;
										break;
									case 2:		// su 8086 accetto offset negativi... verificare altri! 2025
										if(*T2S != '-')
											PROCError(2062,T1S);
										mysign=*T2S == '-';
										break;
									case 4:
										if(*T2S != '+' && *T2S != '-')
											PROCError(2062,T2S);
										break;
									case 15:
										break;
									default:
										PROCError(2062,T2S);
										break;
									}
								goto get2reg2;
								break;
							}  
						o2.T=newT ? newT : oldT;
						o2.V=oldV;
//						o2.l.valid=oldValid;
						if(o2.reg2reg || o2.T)		// ribadisco perché si perde!
							o2.V=VQ_REGISTER;

						}
	        PROCCheck(bt);
					}			// Ind2
	      FNLA(TS);
				if(*TS) {
					if(*TS==':') {		// gestisco segment override qua! perché non arriva come operatore in questo caso...
						PROCCheck(":");
						if(toupper(T2S[1])=='S' && (T2S[2]==0)) {			// v. anche segment override sciolto
							i=FNIsReg(T2S);
							if(i && i & 0x100) {			// reg 16 bit
								i--;
								i &= 0xff;
								if(Reg[i].m>=16 && Reg[i].m<=19) {
									PROCOut(0x26 | ((i & 3) << 3), -1);
									goto rifoIsVar2;
									}
								else if(Reg[i].m>=40 && Reg[i].m<=41) {
									if(CPULevel>=3) {
										PROCOut(0x64 | (i & 1), -1);
										goto rifoIsVar2;
										}
									else
										PROCError(2067,T2S);
									}
								}
							else
								PROCError(2061,T2S);
							}
						}
					else if(*TS!=',')
						PROCError(2060,TS);
					}

#else
// MIGLIORATA gestione extra e porcherie varie a fine riga, usare anche in altre CPU!

	  	  if(o2.Ind) {
		      FNLA(T2S);
					if(*T2S != bt) {
		        o2.l=FNEvalExpr(14,T2S);
						if(!o2.l.valid)
							PROCError(2062);
						}
	        PROCCheck(bt);
		      }
//    fprintf(stderr,"\tEVx2 %lx\n",l2);
#endif
				}			// FNLA 2nd op

				
#if I8086
	  if(forcedSize) {
			o1.S = o2.S = forcedSize;
			}
#endif


#if Z80
	    if(Is2) {
	      if(o1.Ind && o2.Ind) {
          PROCError(2062,T2S);
          }
		    else if(o1.Ind) {
	  	    if(o1.V>=VQ_OP_VAR) {                                 // ld (NN),
	  	      if(o2.V==VQ_REGISTER) {
		  	      if(o2.S==1) {
			  	      if(o2.T==0) {                          // ld (0300h),a
			            k=subStmt(TS,ADDR_IND8,&mn);
			  	        if(Mnn==39) {                            // gestisco OUT
			  	          if(o1.Sptr==1) {
					            PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),-1,0);
					            }
					          else 
					            PROCError(2064,T1S);
			  	          }
			  	        else {
				            PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
					          }
					        }
					      else 
					        PROCError(2062,T2S);
				        }  
				      else {
				        k=subStmt(TS,ADDR_IND16,&mn);                               // ld (0300h),bc
				        if(k>=0) {
				          switch(Reg[o2.T].m) {
				            case 0:
				            case 1:
				              PROCOut(0xed43 | (Reg[o2.T].m << 4),LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
				              break;
				            case 2:
				              PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
				              break;
				            case 16:
				            case 17:
				              PROCOut(0xdd22 | ((Reg[o2.T].m & 0xf) << 13),LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
				              break;
				            case 0x13:
				              PROCOut(0xed73,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
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
				  else if(o1.V==VQ_REGISTER) {                      // ld (r16),
	  	      if(o2.V>=VQ_OP_VAR) {                       // ld (bc),20
	    	      if(o1.S==1) 
  	  	        PROCError(2064,T1S);
	  	        if(o2.S>1/* && Reg[T].m!=0x13*/)      // solo SP?...
	  	          PROCError(2064,T1S);
		          switch(Reg[o1.T].m) {
		            case 2:
  			          k=subStmt(TS,ADDR_19,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),-1,0);
		              break;
		            case 16:
		            case 17:
  			          k=subStmt(TS,ADDR_26,&mn);  
		              PROCOut(mn->a[k].n | ((Reg[o1.T].m & 0xf) << 13),LOBYTE(LOWORD(o1.l.v)),LOBYTE(LOWORD(o2.l.v)),0);
		              break;
//		            case 0x13:
//									PROCOut(0xed7b,l & 255,(l >> 8) & 255,-1);
//		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
				        } 
				      }
				    else if(o2.V==VQ_REGISTER) {                         // ld (bc),a
				      if(o1.S==1) {
			  	      if(o2.S>1)                    // solo out(c),a..
		  	          PROCError(2064,T2S);  
		  	        k=subStmt(TS,ADDR_28,&mn);                     
								PROCOut(mn->a[k].n | (Reg[o2.T].m << 3));  
				        }
				      else {
			  	      if(o2.S>1 && Reg[o1.T].m!=0x13)                    // solo per EX (SP),..
		  	          PROCError(2064,T1S);
			          switch(Reg[o1.T].m) {
			            case 0:
			            case 1:
			              if(Reg[o2.T].m==7)                          // solo ld (r16),a
			                PROCOut(0x02 | (Reg[o1.T].m << 4));
			              else
			                PROCError(2062,T1S);
			              break;
			            case 2:
    			          k=subStmt(TS,ADDR_20,&mn);  
    			          PROCOut(mn->a[k].n | Reg[o2.T].m);
			              break;
			            case 16:
			            case 17:
    			          k=subStmt(TS,ADDR_24,&mn);  
			              PROCOut(mn->a[k].n | ((Reg[o1.T].m & 0xf) << 13) | Reg[o2.T].m,LOBYTE(LOWORD(o1.l.v)),-1,0);
			              break;
			            case 0x13:
    			          switch(Reg[o2.T].m) {
		    	            case 2:
    			              k=subStmt(TS,ADDR_30,&mn);  
    			              PROCOut(mn->a[k].n);
    			              break;
		    	            case 16:
		    	            case 17:
    			              k=subStmt(TS,ADDR_30,&mn);                     // gestisco IX come HL...
      		              PROCOut(mn->a[k].n | 0xdd00 | ((Reg[o2.T].m & 1) << 13));
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
				  else if(o1.V==VQ_CONDIZ) 
            PROCError(2062,T1S);
			    }		// Ind1
		    else if(o2.Ind) {
	  	    if(o1.V==VQ_CONDIZ) {                                // jr nz, 
  	        PROCError(2062,T2S);
			      }
				  else if(o1.V==VQ_REGISTER) {   
	  	      if(o1.S==1) {                          // ld a,()
	  	        if(o2.V>=VQ_OP_VAR) { 
 			          if(o1.T==0) {
                  k=subStmt(TS,ADDR_21,&mn);  
			  	        if(Mnn==21) {                            // gestisco IN
			  	          if(o2.Sptr==1)
		                  PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),-1,0);
		                else 
		                  PROCError(2064,T2S);
		                }
		              else {
		                PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
		                }
		              }
 	              else
	                PROCError(2062,T1S);
				        }                    
					    else if(o2.V==VQ_REGISTER) {                         // ld a,(bc)
			  	      if(o2.S==1) {
			  	        if(Reg[o2.T].m==1 && Mnn==21)             // per IN
	                  PROCOut(0xed,0x40 | Reg[o1.T].m << 3,-1,0);
	                else 
	                  PROCError(2062,T2S);
	                }               
	              else {
				          switch(Reg[o2.T].m) {
				            case 0:
				            case 1:
				              if(Reg[o1.T].m==7)
				                PROCOut(0x0a | (Reg[o2.T].m << 4));
				              else
				                PROCError(2062,T1S);
				              break;
				            case 2:
				              if(Reg[o1.T].m==7) {
  	    			          k=subStmt(TS,ADDR_9,&mn);  
  	    			          PROCOut(mn->a[k].n);
  	    			          }
				              else {
  	    			          k=subStmt(TS,ADDR_32,&mn);
  	    			          PROCOut(mn->a[k].n | (Reg[o1.T].m << 3));
  	    			          }
	    			          break;
				            case 16:
				            case 17:
				              if(Reg[o1.T].m==7) {
  	    			          k=subStmt(TS,ADDR_IX_IY,&mn);  
  	    			          o3.l.v=mn->a[k].n;
  	    			          if(o3.l.v & 0xff0000) 
  	    			            PROCOut((o3.l.v >> 8) | ((Reg[o2.T].m & 0xf) << 13),LOBYTE(LOWORD(o2.l.v)),LOBYTE(LOWORD(o3.l.v)),0);
  	    			          else                                                    
  	    			            PROCOut(o3.l.v | ((Reg[o2.T].m & 0xf) << 13),LOBYTE(LOWORD(o2.l.v)),-1,0);
  	    			          }
				              else {
  	    			          k=subStmt(TS,ADDR_33,&mn);
  	    			          PROCOut(mn->a[k].n | ((Reg[o2.T].m & 0xf) << 13) | (Reg[o1.T].m << 3),LOBYTE(LOWORD(o2.l.v)),-1,0);
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
		          switch(Reg[o1.T].m) {
		            case 0:
		            case 1:
	                PROCOut(0xed4b | (Reg[o1.T].m << 4),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
		              break;
		            case 2:
  			          k=subStmt(TS,ADDR_22,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
		              break;
		            case 16:
		            case 17:                            // gestisco IX come HL
  			          k=subStmt(TS,ADDR_22,&mn);  
		              PROCOut(mn->a[k].n | 0xdd00 | ((Reg[o1.T].m & 0xf) << 13),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
		              break;
		            case 0x13:
		              PROCOut(0xed7b,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
		            }
				      }
				    }
				  else if(o1.V>=VQ_OP_VAR) {                    // bit,set,res
				    if(o2.V==VQ_REGISTER && o2.S==2 && o1.l.v>=0 && o1.l.v<=7) {
			        switch(Reg[o2.T].m) {
		            case 2:
  			          k=subStmt(TS,ADDR_BIT2,&mn);  
  			          PROCOut(mn->a[k].n | o1.l.v << 3);
		              break;
		            case 16:
		            case 17:
  			          k=subStmt(TS,ADDR_BITX,&mn);		// e BITY? 2025, forse è gestito in automatico...
  			          o3.l.v=mn->a[k].n;
  			          if(o3.l.v & 0xff0000) {
		                PROCOut((o3.l.v >> 8) | ((Reg[o2.T].m & 0xf) << 13),LOBYTE(LOWORD(o2.l.v)),(LOBYTE(LOWORD(o3.l.v))) | (o1.l.v << 3),0);
		                }
		              else
		                PROCOut(o3.l.v | (o1.l.v << 3) | ((Reg[o2.T].m & 0xf) << 13),LOBYTE(LOWORD(o2.l.v)),-1,-0);
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
	  	    if(o1.V==VQ_CONDIZ) {                                // jr nz, 
	  	      if(o2.V>=VQ_OP_VAR) {
isCcond:	  	    
              k=subStmt(TS,ADDR_CONDIZ_IMM,&mn);
              subBranch(TS,T2S,mn,o1.T,k,o2.l,TRUE);
              }
            else 
	  	        PROCError(2062,T2S);
			      }
				  else if(o1.V==VQ_REGISTER) {
				    if((Mnn==27 || Mnn==28 || Mnn==45 || Mnn==4) && Reg[o1.T].m==1) {   // gestisco C cond
				      o1.T=0x0;
				      goto isCcond;
				      }
	  	      if(o1.S==1) {                          // ld a,20
	  	        if(o2.V>=VQ_OP_VAR) { 
	  	          if(o2.S>1) 
	  	            PROCError(2064,T2S);
	  	          if(Reg[o1.T].m==7) {
	 			          k=subStmt(TS,ADDR_IMM8,&mn);            // FILTRO QUELLE CHE accettano solo A
	    			      PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),-1,0);
    			        }
    			      else {
    			        k=subStmt(TS,ADDR_11,&mn);   
	    			      PROCOut(mn->a[k].n | (Reg[o1.T].m << 3),LOBYTE(LOWORD(o2.l.v)),-1,0);
    			        }
				        }                    
					    else if(o2.V==VQ_REGISTER) {                         // ld a,b
								if(o1.S==1) { //togliere !! v.sopra
			  	        if(o1.T==0) {                                // solo a
			  	          if(Reg[o2.T].m & 0x10) {
	   			            k=subStmt(TS,Reg[o2.T].m & 1 ? ADDR_61 : ADDR_60,&mn);  
	        			      PROCOut(mn->a[k].n);
			  	            }
			  	          else {
	   			            k=subStmt(TS,ADDR_REG8,&mn);  
	        			      PROCOut(mn->a[k].n | Reg[o2.T].m);
	        			      }
        			      }
        			    else if(Reg[o1.T].m & 0x10) {
        			      k=subStmt(TS,Reg[o1.T].m & 1 ? ADDR_63 : ADDR_62,&mn);  
	        			    PROCOut(mn->a[k].n);
        			      }
   			          else {
   			            k=subStmt(TS,ADDR_10,&mn);  
      			        PROCOut(mn->a[k].n | (Reg[o1.T].m << 3) | Reg[o2.T].m);
      			        }
			            }
				        }
				      else
				        PROCError(2062,T2S);
				      }
			      else { 
		  	      if(o2.V>=VQ_OP_VAR) {                       // ld bc,20
		  	        switch(Reg[o1.T].m) {
		  	          case 0:
		  	          case 1:
		  	          case 2:
		  	          case 0x13:
 			              k=subStmt(TS,ADDR_23,&mn);  
 			              PROCOut(mn->a[k].n | ((Reg[o1.T].m & 0xf) << 4),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
#pragma warning QUA SEMBRA STRANO l2 l... 2024  messo o2 al posto di o1 2025, v.8086 e ok
 			              break;
 			            case 16:
 			            case 17:
 			              PROCOut(0xdd21 | ((Reg[o1.T].m & 0xf) << 13),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o1.l.seg);    // con segm.
 			              break;
 			            default:
 			              PROCError(2062,T2S);
 			              break;
 			            }  
		  	        }
		          else if(o2.V==VQ_REGISTER) {                // sbc hl,de
				        switch(Reg[o1.T].m) {
			            case 3:                      // ex af
			              if(Reg[o2.T].m != 0x23)
			                PROCError(2062,T2S);
	  			          PROCOut(0x8);
			              break;
			            case 1:                      // ex de,hl (SOLO!)
	  			          if(Mnn==0 || Mnn==59)         // filtro via SBC,ADC...
			                PROCError(2062,T1S);
	  			          if(Reg[o2.T].m != 2)
			                PROCError(2062,T2S);
	  			          PROCOut(0xeb);
			              break;
			            case 2:
	  			          k=subStmt(TS,ADDR_IMM16,&mn);  
	  			          PROCOut(mn->a[k].n | (Reg[o2.T].m & 0xf) << 4);
			              break;
			            case 16:
			            case 17:
		                k=subStmt(TS,ADDR_7,&mn); 
	  			          switch(Reg[o2.T].m) {
	  			            case 0x10:
	  			            case 0x11:                       // add ix,ix ecc.
	  			              if(Reg[o1.T].m != Reg[o2.T].m)
	  			                PROCError(2062,T2S);
	  			              else
			                    PROCOut(mn->a[k].n | ((Reg[o1.T].m & 0xf) << 13) | 0x20);
			                  break;
			                case 0:
			                case 1:
//			                case 2:                        // no hl
			                case 0x13:                       // altri
			                  PROCOut(mn->a[k].n | ((Reg[o1.T].m & 0xf) << 13) | (Reg[o2.T].m & 0xf) << 4);
			                  break;
			                default:
										    PROCError(2062,T2S);
										    break;
										  }
			              break;
			            case 0x13:                             // ld sp,hl
			              switch(Reg[o2.T].m) {
			                case 2:
										    PROCOut(0xf9);
										    break;
										  case 16:
										  case 17:
										    PROCOut(0xddf9 | ((Reg[o2.T].m & 0xf) << 13));
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
					    else /*if(o2.V==VQ_REGISTER) ovvio 2025 verificare*/ {  
	  	          PROCError(2062,T2S);
				        }
				      }
				    }
				  else if(o1.V>=0) {                    // bit,set,res
				    if(o2.V==VQ_REGISTER && o2.S==1 && o1.l.v>=0 && o1.l.v<=7) {
		          k=subStmt(TS,ADDR_BIT,&mn);  
		          PROCOut(mn->a[k].n | o1.l.v << 3 | Reg[o2.T].m);
				      }
            else { 
              PROCError(2062,T1S);
				      }
            }
          }
			  }		// Is2
			else {                                     // 1 solo parm
		    if(o1.V>=VQ_OP_VAR) {
		      if(o1.Ind) {
            PROCError(2062,T1S);
			      }
			    else {
  			    if(o1.S==1) {
  			      k=subStmt(TS,ADDR_IMM8,&mn);         // or 20
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %x, l=%lx\n",CurAddr,l);
  			        if(Mnn==28 || Mnn==15) {                 // gestisco JR e DJNZ
                  subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);
  			          }
  			        else if(Mnn==58) {                              // gestisco RST
//    fprintf(stderr,"\tEVx %ld\n",l);
  			          if((o1.l.v & 7) || o1.l.v>0x38 || o1.l.v<0)
  			            PROCError(2062,T1S);
 	  		          PROCOut(mn->a[k].n | (LOBYTE(LOWORD(o1.l.v))));
	  		          }
  			        else if(Mnn==20) {                              // gestisco IM
  			          if(o1.l.v<0 || o1.l.v>2)
  			            PROCError(2062,T1S);
 	  		          PROCOut(mn->a[k].n | (o1.l.v ? (o1.l.v<<3)+8 : 0));
	  		          }
  			        else if(Mnn==4 || Mnn==27) {                           // gestisco CALL e JP 
 	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
	  		          }
	  		        else                                   
	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),-1,0);
	  		        }
  			      }
  			    else {
  			      k=subStmt(TS,ADDR_14,&mn);         // jp ea31
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
                subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);           // si autogestisce CALL, JP  e JR
	  		        }
  			      }
				    }
			    }
			  else if(o1.V==VQ_REGISTER) {
			    if(o1.Ind) {
		        if(o1.S==2) {
		          if(Reg[o1.T].m==2) {                // jp (hl)
   			        k=subStmt(TS,ADDR_9,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n);
			          }
			        else if(Reg[o1.T].m==16 || Reg[o1.T].m==17) {
			          if(Mnn==27) {                  // gestisco jp (ix)
			            k=subStmt(TS,ADDR_12,&mn);
  	              PROCOut(mn->a[k].n | ((Reg[o1.T].m & 0xf) << 13));
			            }
			          else {
			            k=subStmt(TS,ADDR_IX_IY,&mn);
			            o3.l.v=mn->a[k].n;
			            if(o3.l.v & 0xff0000) {
		                PROCOut(o3.l.v >> 8 | ((Reg[o1.T].m & 0xf) << 13),LOBYTE(LOWORD(o1.l.v)),LOBYTE(LOWORD(o3.l.v)),0);
		                }
		              else                                     
		                PROCOut(o3.l.v | ((Reg[o1.T].m & 0xf) << 13),LOBYTE(LOWORD(o1.l.v)),-1,0);
			            }
			          }
			        else 
			          PROCError(2062,T1S);
			        }
		        else {
		          PROCError(2064,T1S);
		          }
			      }
          else {
				    if(o1.S==1) {
  				    if((Mnn==27 || Mnn==28 || Mnn==45) && Reg[o1.T].m==1) {   // gestisco C cond
	  			      o1.T=0x0;
		  		      goto isCcond2;
			  	      }
					    if(Reg[o1.T].m<8) {
	  			      k=subStmt(TS,ADDR_10,&mn);         // or B
	  			      if(k>=0) {
	  			        if(Mnn==13 || Mnn==21 || Mnn==22 || Mnn==39)           // gestisco INC,DEC,IN,OUT
		  		          PROCOut(mn->a[k].n | (Reg[o1.T].m << 3));
		  		        else                                         
		  		          PROCOut(mn->a[k].n | Reg[o1.T].m);
		  		        }
					      }
					    else {
					      PROCError(2062,T1S);
					      }
					    }
				    else {
		          if(Reg[o1.T].m<4) {                       // inc hl
   			        k=subStmt(TS,ADDR_IMM16,&mn);
  			        if(k>=0) 
	 		            PROCOut(mn->a[k].n | (Reg[o1.T].m << 4));
			          }
			        else if(Reg[o1.T].m==16) {
			          k=subStmt(TS,ADDR_15,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n);
			          }
			        else if(Reg[o1.T].m==17) {
			          k=subStmt(TS,ADDR_16,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n);
			          }
			        else if(Reg[o1.T].m==0x13) {               // sp
			          k=subStmt(TS,ADDR_31,&mn);
  			        if(k>=0) 
  		            PROCOut(mn->a[k].n);
			          }
			        else 
			          PROCError(2062,T1S);
	 			      }
				    }
				  }
			  else if(o1.V==VQ_CONDIZ) {                    
isCcond2:
		      k=subStmt(TS,ADDR_CONDIZ,&mn);         // ret nz
		      if(k>=0) 
 		        PROCOut(mn->a[k].n | Cond[o1.T].m);
			    }   
			  }
    
#elif _6502
#pragma warning FINIRE e abb ok 14/2/24
	    if(Is2) {
	      if(o1.V==VQ_REGISTER)
          PROCError(2062,T1S);
	      if(o1.Ind && o2.Ind) {
          PROCError(2062,T2S);
          }
		    else if(o1.Ind) {
		  	  if(o1.S==2)
				    PROCError(2062,T1S);
	  	    if(o1.V>=VQ_OP_VAR) {                                 // ld (),x (,y)
	  	      if(o2.V==VQ_REGISTER) {
		  	      if(o2.S==1) {
			  	      if(o2.T!=2 && o2.T!=3)
					        PROCError(2062,T2S);
								k=subStmt(TS,o2.T==2 ? ADDR_10 : ADDR_11,&mn);
		            PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),-1,0);
				        }  
				      else {
	              PROCError(2062,T2S);
				        }                    
              }
            else 
	  	        PROCError(2062,T2S);
			      }
				  else if(o1.V==VQ_REGISTER) {                      // ld (r16),
	  	      if(o2.V>=VQ_OP_VAR) {                       // ld (bc),20
	    	      if(o1.S==1) 
  	  	        PROCError(2064,T1S);
	  	        if(o2.S>1/* && Reg[T].m!=0x13*/)      // solo SP?...
	  	          PROCError(2064,T1S);
		          switch(Reg[o1.T].m) {
		            case 2:
  			          k=subStmt(TS,ADDR_19,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),-1,0);
		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
				        } 
				      }
				    else if(o2.V==VQ_REGISTER) {                         // ld (bc),a
				      if(o1.S==1) {
			  	      if(o2.S>1)                    // solo out(c),a..
		  	          PROCError(2064,T2S);  
		  	        k=subStmt(TS,ADDR_28,&mn);                     
								PROCOut(mn->a[k].n | (Reg[o2.T].m << 3));
				        }
				      else {
	  	          PROCError(2064,T1S);
					      }
				      }
				    }
			    }		// Ind1
		    else if(o2.Ind) {
				  if(o1.V==VQ_REGISTER) {   
	  	      if(o1.S==1) {                          // ld a,()
	  	        if(o2.V>=VQ_OP_VAR) { 
 			          if(o1.T==0) {
                  k=subStmt(TS,ADDR_21,&mn);  
	                PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
		              }
 	              else
	                PROCError(2062,T1S);
				        }                    
				      }
				    }
				  else if(o1.V>=0) {                    // bit,set,res
            PROCError(2062,T1S);
				    }
          }
 		    else {                                       // ld ,x ,y
//   printf("eccooo %d %d %d %d\n",V,S,V2,S2);
				  if(o1.V>=VQ_OP_VAR) {                    // ,x ,y
						switch(o1.S) {
							case 1:
								if(o2.T!=2 && o2.T!=3)
		  	          PROCError(2062,T2S);
 								if(Mnn==31 || Mnn==32 || Mnn==53 || Mnn==54) {                 // gestisco LDA, LDX, LDY
									k=subStmt(TS,o2.T==2 ? ADDR_ZPX : ADDR_ZPY,&mn);		// 
	  							if(k>=0) 
	 	  							PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),-1,0);    // 
									}
								else {
									if(o2.T==3) {
										if(CPass) {
									    PROCOut1("",NULL,TRUE);
									    PROCOut1("\n",NULL,TRUE);		// non è perfetto ma ok... 17/2/24
											PROCWarn(4012,TS);
											}
										goto y16bit;
										}
									k=subStmt(TS,ADDR_ZPX,&mn);		// 
	  							if(k>=0) 
	 	  							PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),-1,0);    // 
									}
								break;
							case 2:
								if(o2.T!=2 && o2.T!=3)
		  	          PROCError(2062,T2S);
y16bit:
								k=subStmt(TS,o2.T==2 ? ADDR_ABSX : ADDR_ABSY,&mn);
	  						if(k>=0) 
	 	  		        PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),0);    // 
								break;
							}
            }
          }
			  }			// Is2
			else {                                     // 1 solo parm
		    if(o1.V>=VQ_OP_VAR) {
		      if(o1.Ind) {
						if(o1.V==VQ_VAR_IMM)
				      PROCError(2062,T1S);
						k=subStmt(TS,ADDR_12,&mn);         // jp (ea31)
  			    if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
							PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),0);
	  		      }
			      }
			    else {
						switch(o1.S) {
							case 1:
  							if(Mnn==28 || Mnn==29) {                 // gestisco JMP & JSR
			  			    k=subStmt(TS,o1.V==VQ_VAR_IMM ? ADDR_IMM8 : ADDR_ABS,&mn);         // FORZO errore se imm :)
									if(k>=0)
 		  							PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),0,HIBYTE(HIWORD(o1.l.v)));    // con segm.
  								}
  							else if(Mnn==3 || Mnn==4 || Mnn==5 || Mnn==7 || Mnn==8 || Mnn==9 || Mnn==10 || Mnn==12 || Mnn==13) {                 // gestisco Bxx
			  			    k=subStmt(TS,o1.V==VQ_VAR_IMM ? ADDR_IMM8 : ADDR_ABS,&mn);         // FORZO errore se imm :)
									if(k>=0)
										subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);
									}
  							else {                              // 
									k=subStmt(TS,o1.V==VQ_VAR_IMM ? ADDR_IMM8 : ADDR_ZP,&mn);         // LDA #20 o 20
									if(k>=0)
			 	  		      PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),-1,0);    // 
									}
								break;
							case 2:
		  			    k=subStmt(TS,o1.V==VQ_VAR_IMM ? ADDR_IMM8 : ADDR_ABS,&mn);         // FORZO errore se imm :)
  							if(k>=0) {
  								if(Mnn==28 || Mnn==29) {                 // gestisco JMP & JSR
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
  									}
									else if(Mnn==3 || Mnn==4 || Mnn==5 || Mnn==7 || Mnn==8 || Mnn==9 || Mnn==10 || Mnn==12 || Mnn==13) {                 // gestisco Bxx
		//    fprintf(stderr,"\tEVx %ld\n",l);
										subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);
										}
  								else {                              // 
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
										}
									}
								else
				          PROCError(2062,T1S);
								break;
							case 4:
		  			    k=subStmt(TS,o1.V==VQ_VAR_IMM ? ADDR_IMM8 : ADDR_ABS,&mn);         // Bxx; idem come sopra
  							if(k>=0) {
		//   fprintf(stderr,"\tCurAddr: %x, l=%lx\n",CurAddr,l);
  								if(Mnn==28 || Mnn==29) {                 // gestisco JMP & JSR
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
  									}
	  							else if(Mnn==3 || Mnn==4 || Mnn==5 || Mnn==7 || Mnn==8 || Mnn==9 || Mnn==10 || Mnn==12 || Mnn==13) {                 // gestisco Bxx
		//    fprintf(stderr,"\tEVx %ld\n",l);
										subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);
										}
  								else {                              // 
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
										}
									break;
	  						}
							}
				    }
			    }
			  else if(o1.V==VQ_REGISTER) {
			    if(o1.Ind) {
		        if(o1.S==2) {
		          PROCError(2062,T1S);
			        }
		        else {
		          PROCError(2064,T1S);
		          }
			      }
          else {
				    if(o1.S==1) {
					    if(o1.T==1) {
	  			      k=subStmt(TS,ADDR_REG8,&mn);         // LSR ecc A
  		          PROCOut(mn->a[k].n);
					      }
					    else {
					      PROCError(2062,T1S);
					      }
					    }
				    else {
		          PROCError(2062,T1S);
	 			      }
				    }
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
		  	      if(o2.S==1) {
			  	      if(T2==0) {                          // ld (0300h),a
			            k=subStmt(TS,17,&mn);
			  	        if(Mnn==39) {                            // gestisco OUT
			  	          if(o1.Sptr==1) {
					            PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);
					            }
					          else 
					            PROCError(2064,T1S);
			  	          }
			  	        else {
				            PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIBYTE(HIWORD(l)));    // con segm.
					          }
					        }
					      else 
					        PROCError(2062,T2S);
				        }  
				      else {
				        k=subStmt(TS,ADDR_18,&mn);                               // ld (0300h),bc
				        if(k>=0) {
				          switch(Reg[T2].m) {
				            case 0:
				            case 1:
				              PROCOut(0xed43 | (Reg[T2].m << 4),LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIBYTE(HIWORD(l)));    // con segm.
				              break;
				            case 2:
				              PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIBYTE(HIWORD(l)));    // con segm.
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
	    	      if(o1.S==1) 
  	  	        PROCError(2064,T1S);
	  	        if(o2.S>1/* && Reg[T].m!=0x13*/)      // solo SP?...
	  	          PROCError(2064,T1S);
		          switch(Reg[T].m) {
		            case 2:
  			          k=subStmt(TS,ADDR_19,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),-1,0);
		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
				        } 
				      }
				    else if(V2==VQ_REGISTER) {                         // ld (bc),a
				      if(o1.S==1) {
			  	      if(o2.S>1)                    // solo out(c),a..
		  	          PROCError(2064,T2S);  
		  	        k=subStmt(TS,ADDR_28,&mn);                     
								PROCOut(mn->a[k].n | (Reg[T2].m << 3));  
				        }
				      else {
			  	      if(o2.S>1 && Reg[T].m!=0x13)                    // solo per EX (SP),..
		  	          PROCError(2064,T1S);
			          switch(Reg[T].m) {
			            case 0:
			            case 1:
			              if(Reg[T2].m==7)                          // solo ld (r16),a
			                PROCOut(0x02 | (Reg[T].m << 4));
			              else
			                PROCError(2062,T1S);
			              break;
			            case 2:
    			          k=subStmt(TS,ADDR_20,&mn);  
    			          PROCOut(mn->a[k].n | Reg[T2].m);
			              break;
			            default:
			              PROCError(2062,T1S);
			              break;
			            }
					      }
				      }
				    }
			    }		// Ind1
		    else if(Ind2) {
				  if(V1==VQ_REGISTER) {   
	  	      if(o1.S==1) {                          // ld a,()
	  	        if(V2>=VQ_OP_VAR) { 
 			          if(T==0) {
                  k=subStmt(TS,ADDR_21,&mn);  
			  	        if(Mnn==21) {                            // gestisco IN
			  	          if(o2.Sptr==1)
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
			  	      if(o2.S==1) {
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
				                PROCOut(0x0a | (Reg[T2].m << 4));
				              else
				                PROCError(2062,T1S);
				              break;
				            case 2:
				              if(Reg[T].m==7) {
  	    			          k=subStmt(TS,ADDR_9,&mn);  
  	    			          PROCOut(mn->a[k].n);
  	    			          }
				              else {
  	    			          k=subStmt(TS,ADDR_32,&mn);
  	    			          PROCOut(mn->a[k].n | (Reg[T].m << 3));
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
  			          k=subStmt(TS,ADDR_22,&mn);  
  			          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
		              break;
		            default:
		              PROCError(2062,T1S);
		              break;
		            }
				      }
				    }
				  else if(V1>=VQ_OP_VAR) {                    // bit,set,res
				    if(V2==VQ_REGISTER && o2.S==2 && l>=0 && l<=7) {
			        switch(Reg[T2].m) {
		            case 2:
  			          k=subStmt(TS,ADDR_BIT2,&mn);  
  			          PROCOut(mn->a[k].n | l << 3);
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
//   printf("eccooo %d %d %d %d\n",V1,o1.S,V2,o2.S);
				  if(V1==VQ_REGISTER) {
	  	      if(o1.S==1) {                          // mov r0,20
	  	        if(V2>=VQ_OP_VAR) { 
	  	          if(o2.S>1) 
	  	            PROCError(2064,T2S);
	  	          if(Reg[T].m==0) {
	 			          k=subStmt(TS,ADDR_IMM8,&mn);            // FILTRO QUELLE CHE accettano solo A
	    			      PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),-1,0);
    			        }
    			      else {
    			        k=subStmt(TS,11,&mn);   
	    			      PROCOut(mn->a[k].n | (Reg[T].m & 0xf),LOBYTE(LOWORD(l2)),-1,0);
    			        }
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,b
			  	      if(o1.S==1) {
			  	        if(T==0) {                                // solo a
			  	          if(Reg[T2].m & 0x10) {
	   			            k=subStmt(TS,Reg[T2].m & 1 ? ADDR_61 : ADDR_60,&mn);  
	        			      PROCOut(mn->a[k].n);
			  	            }
			  	          else {
	   			            k=subStmt(TS,ADDR_REG8,&mn);  
	        			      PROCOut(mn->a[k].n | Reg[T2].m);
	        			      }
        			      }
        			    else if(Reg[T].m & 0x10) {
        			      k=subStmt(TS,Reg[T].m & 1 ? ADDR_63 : ADDR_62,&mn);  
	        			    PROCOut(mn->a[k].n);
        			      }
   			          else {
   			            k=subStmt(TS,ADDR_10,&mn);  
      			        PROCOut(mn->a[k].n | (Reg[T].m << 3) | Reg[T2].m);
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
 			              k=subStmt(TS,ADDR_23,&mn);  
 			              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 4),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
 			              break;
 			            case 16:
 			            case 17:
 			              PROCOut(0xdd21 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l2)),HIBYTE(LOWORD(l2)),HIWORD(l2));    // con segm.
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
	  			          PROCOut(0x8);
			              break;
			            case 1:                      // ex de
	  			          if(Reg[T2].m != 2)
			                PROCError(2062,T2S);
	  			          PROCOut(0xeb);
			              break;
			            case 2:
	  			          k=subStmt(TS,ADDR_IMM16,&mn);  
	  			          PROCOut(mn->a[k].n | (Reg[T2].m & 0xf) << 4);
			              break;
			            default:
			              PROCError(2062,T2S);
			            }
		            }
				      }
				    }
				  else if(V1>=0) {                    // bit,set,res
				    if(V2==VQ_REGISTER && o2.S==1 && l>=0 && l<=7) {
		          k=subStmt(TS,ADDR_BIT,&mn);  
		          PROCOut(mn->a[k].n | l << 3 | Reg[T2].m);
				      }
            else { 
              PROCError(2062,T1S);
				      }
            }
          }
			  }		// Is2
			else {                                     // 1 solo parm
		    if(V1>=VQ_OP_VAR) {
		      if(Ind1) {
            PROCError(2062,T1S);
			      }
			    else {
  			    if(o1.S==1) {
  			      k=subStmt(TS,ADDR_IMM8,&mn);         // or 20
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %x, l=%lx\n",CurAddr,l);
  			        if(Mnn==28 || Mnn==15) {                 // gestisco JR e DJNZ
                  subBranch(TS,T2S,mn,T,k,l,FALSE);
  			          }
  			        else if(Mnn==58) {                              // gestisco RST
//    fprintf(stderr,"\tEVx %ld\n",l);
  			          if((l & 7) || l>0x38 || l<0)
  			            PROCError(2062,T1S);
 	  		          PROCOut(mn->a[k].n | (LOBYTE(LOWORD(l))));
	  		          }
  			        else if(Mnn==20) {                              // gestisco IM
  			          if(l<0 || l>2)
  			            PROCError(2062,T1S);
 	  		          PROCOut(mn->a[k].n | (l ? (l<<3)+8 : 0));
	  		          }
  			        else if(Mnn==4 || Mnn==27) {                           // gestisco CALL e JP 
 	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),HIBYTE(HIWORD(l)));    // con segm.
	  		          }
	  		        else                                   
	  		          PROCOut(mn->a[k].n,LOBYTE(LOWORD(l)),-1,0);
	  		        }
  			      }
  			    else {
  			      k=subStmt(TS,ADDR_14,&mn);         // jp ea31
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
                subBranch(TS,T2S,mn,T,k,l,FALSE);           // si autogestisce CALL, JP  e JR
	  		        }
  			      }
				    }
			    }
			  else if(V1==VQ_REGISTER) {
			    if(Ind1) {
		        if(o1.S==2) {
		          if(Reg[T].m==2) {                // jp (hl)
   			        k=subStmt(TS,ADDR_9,&mn);
  			        if(k>=0) 
			            PROCOut(mn->a[k].n);
			          }
			        else if(Reg[T].m==16 || Reg[T].m==17) {
			          if(Mnn==27) {                  // gestisco jp (ix)
			            k=subStmt(TS,ADDR_12,&mn);
  	              PROCOut(mn->a[k].n | ((Reg[T].m & 0xf) << 13));
			            }
			          else {
			            k=subStmt(TS,ADDR_IX_IY,&mn);
			            l3=mn->a[k].n;
			            if(l3 & 0xff0000) {
		                PROCOut(l3 >> 8 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),LOBYTE(LOWORD(l3)),0);
		                }
		              else                                     
		                PROCOut(l3 | ((Reg[T].m & 0xf) << 13),LOBYTE(LOWORD(l)),-1,0);
			            }
			          }
			        else 
			          PROCError(2062,T1S);
			        }
		        else {
		          PROCError(2064,T1S);
		          }
			      }
          else {
				    if(o1.S==1) {
					    if(Reg[T].m<8) {
	  			      k=subStmt(TS,ADDR_10,&mn);         // or B
	  			      if(k>=0) {
	  			        if(Mnn==13 || Mnn==21 || Mnn==22 || Mnn==39)           // gestisco INC,DEC,IN,OUT
		  		          PROCOut(mn->a[k].n | (Reg[T].m << 3));
		  		        else                                         
		  		          PROCOut(mn->a[k].n | Reg[T].m);
		  		        }
					      }
					    else {
					      PROCError(2062,T1S);
					      }
					    }
				    else {
		          if(Reg[T].m<4) {                       // inc hl
   			        k=subStmt(TS,ADDR_IMM16,&mn);
  			        if(k>=0) 
	 		            PROCOut(mn->a[k].n | (Reg[T].m << 4));
			          }
			        else 
			          PROCError(2062,T1S);
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
	  	    if(V1>=VQ_OP_VAR) {                                 // ld (NN),
  	        PROCError(2062,T2S);
			      }
				  else if(V1==VQ_REGISTER) {                      // ld (r8),
	  	      if(V2>=VQ_OP_VAR) {                       // ld (r8),20
  	          PROCError(2064,T2S);  
				      }
				    else if(V2==VQ_REGISTER) {                         // ld (bc),a
				      if(o1.S==1) {
			  	      if(o2.S>1 || Reg[T2].m)  // solo ,A
		  	          PROCError(2064,T2S);  
			          switch(Reg[T].m) {
			            case 1:
	  			          k=subStmt(TS,ADDR_24,&mn);
	  			          PROCOut(mn->a[k].n);
			              break;
			            case 2:
	  			          k=subStmt(TS,ADDR_25,&mn);
	  			          PROCOut(mn->a[k].n);
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
			    }		// Ind1
		    else if(Ind2) {
				  if(V1==VQ_REGISTER) {   
	  	      if(o1.S==1) {                          // ld a,()
	  	        if(V2>=VQ_OP_VAR) {
                PROCError(2062,T1S);
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,(bc)
			  	      if(o2.S==1) {
				          switch(Reg[T2].m) {
				            case 1:
		  			          k=subStmt(TS,ADDR_IX_IY,&mn);
		  			          PROCOut(mn->a[k].n);
				              break;
				            case 2:
		  			          k=subStmt(TS,ADDR_IY,&mn);
		  			          PROCOut(mn->a[k].n);
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
	  	      if(o1.S==1) {                          // ld v,20
	  	        if(V2>=VQ_OP_VAR) { 
	  	          if(o2.S>1) 
	  	            PROCError(2064,T2S);
	  	            
	  	          if(Reg[T].m==0) {
	 			          k=subStmt(TS,ADDR_IMM8,&mn);
	    			      PROCOut(mn->a[k].n,LOBYTE(LOWORD(l2)),-1,0);
    			        }
    			      else {
	 			          k=subStmt(TS,ADDR_IMM16,&mn);
	    			      PROCOut(mn->a[k].n,0x80 | (Reg[T].m -1),LOBYTE(LOWORD(l2)),0);
    			        }
				        }                    
					    else if(V2==VQ_REGISTER) {                         // ld a,b
			  	      if(o1.S==1) {
			  	        if(Mnn == 18) {    // LD
				  	        if(T==0) {                                // solo a
	  		  	          if(T2==0)
	  		  	            PROCError(2062,T2S);
	   			            k=subStmt(TS,ADDR_REG8,&mn);  
	        			      PROCOut(mn->a[k].n | ((Reg[T2].m - 1) << 6));
	        			      }
	        			    else {
	   			            k=subStmt(TS,ADDR_10,&mn);  
	        			      PROCOut(mn->a[k].n | ((Reg[T].m - 1) << 6));
	        			      }
	        			    }
	        			  else {
				  	        if(T==0) {                                // solo a
	   			            k=subStmt(TS,ADDR_REG8,&mn);  
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
				  else if(V1>=VQ_OP_VAR) {                    // ld r,n  opp. (rr),n
				    if(o1.S==1 && o2.S==1) {
				      if(*FNLA(T3S)) {        // jrs,jrr
				        PROCCheck(',');
				        *T3S=0;
								S3=0;
								T3=0;
								V3=0;
								o3.op=0;
						    p=theLinePtr;
						    i=FNGetAritElem(T3S,&V3,&v3,&T3,&S3,&o3.op,0);
						    if(i>ARITM_EOL && i<ARITM_OPER && V3>=0) {
					        V3=0;
//2024 verificare					        fseek(FIn,p,SEEK_SET);
					        l3=FNEvalExpr(14,T3S);
									if(!l3.valid)
										PROCError(2062);
									if(o3.Ind) {
										o3.Sptr=FNGetSize(o3.l);
						o3.S=0/*INT_SIZE*/;
										}
									else {
										o3.S=FNGetSize(o3.l);
										}
//					        o3.S /*S3*/=FNGetSize(l3);
									o3.l.valid=TRUE;
					        }
					      else
                  PROCError(2062,T3S);
						    if(l < 0 || l > 7)
						      PROCError(2063,T1S);
			          k=subStmt(TS,ADDR_CONDIZ_IMM,&mn);
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
				            l |= 0x20;
				          if(l & 2)
				            l |= 0x40;
				          if(l & 1)
				            l |= 0x80;
				          l &= 0xe0;     // i bit sono invertiti...
		  				    if(V2==VQ_REGISTER) {
					          k=subStmt(TS,ADDR_BIT,&mn);  
					          PROCOut(mn->a[k].n | l,0x80 | (Reg[T].m - 1),-1,0);
					          }
					        else {
					          k=subStmt(TS,ADDR_BIT,&mn);  
					          PROCOut(mn->a[k].n | l,LOBYTE(LOWORD(l2)),-1,0);
					          }
				          }
				        else {
		  				    if(V2==VQ_REGISTER) {
		  				      if(Mnn != 19 && T2 !=0)      // a parte LDI, solo A puo' essere copiato in un indirizzo
		  				        PROCError(2062,T2S);
					          k=subStmt(TS,ADDR_17,&mn);  
					          PROCOut(mn->a[k].n,l,-1,0);
					          }
					        else {
		  				      if(Mnn != 19 && T2 !=0)      // a parte LDI, solo A puo' essere copiato in un indirizzo
		  				        PROCError(2062,T2S);
					          k=subStmt(TS,ADDR_11,&mn);  
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
		    if(V1>=VQ_OP_VAR) {
		      if(Ind1) {
		        goto nonInd1;   // accetto op. anche tra parentesi...
			      }
			    else {
nonInd1:			    
//   fprintf(stderr,"\tCurAddr: l=%lx; S=%d\n",l,S);
  			    if(o1.S==1) {
  			      k=subStmt(TS,ADDR_IMM8,&mn);         // or 20
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
  			      k=subStmt(TS,ADDR_14,&mn);         // jp ea31
  			      if(k>=0) {
//   fprintf(stderr,"\tCurAddr: %lx, l=%lx T1S %s\n",CurAddr,l,T1S);
                subBranch(TS,T2S,mn,T,k,l,FALSE);           // si autogestisce CALL, JP  e JR
	  		        }
  			      }
				    }
			    }
			  else if(V1==VQ_REGISTER) {
			    if(Ind1) {
		        if(o1.S>1 || T>=2) {
		          PROCError(2064,T1S);
		          }
		        else {              // inc (x)
		          PROCOut(mn->a[k].n,(Reg[T].m-1) << 3,-1,0);
		          }
			      }
          else {
				    if(o1.S==1) {
				      if(Mnn==5 || Mnn==6 || Mnn==24 || Mnn==26) {   // quelle che vanno solo con A...
				        if(T==0) {
		  			      k=subStmt(TS,ADDR_IMPL,&mn);         // inc 
		  			      if(k>=0) {
		  		          if(Mnn == 5)   // gestisco CLR che e' finta!
	  				          PROCOut(mn->a[k].n,l,0,0);
		  		          else if(Mnn == 26)  // e SLA, pure
 	  			            PROCOut(mn->a[k].n,0xff,-1,0);
 	  			          else
		  		            PROCOut(mn->a[k].n);
							      }
							    }
							  else
    		          PROCError(2062,T1S);
				        }
				      else {
  			        if(T==0) {
	  			      	k=subStmt(TS,ADDR_IMM8,&mn);         // inc 
	  		          PROCOut(mn->a[k].n,0xff,-1,0);
	  		          }
  			        else {
	  			      	k=subStmt(TS,ADDR_10,&mn);
  		            PROCOut(mn->a[k].n | ((Reg[T].m-1) << 6));
  		            }
						    }
					    }
				    else {
		          PROCError(2062,T1S);
	 			      }
				    }
				  }
			  }

#elif PIC==16

#pragma warning FINIRE  abb ok 19/2/24
	    if(Is2) {
	      if(o1.V==VQ_REGISTER)
          PROCError(2062,T1S);
	      if(o1.Ind || o2.Ind) {
          PROCError(2062,T1S);
          }
 		    else {                                       // 
				  if(o1.V>=VQ_OP_VAR) {                    // 
						if(o2.V==VQ_REGISTER) {
							if(o2.T==1)	{		// solo W! tanto per... (in MPLAB c'è define e basta
								o2.l.v=0;
								goto W_as_0;
								}
							}
						else {
							switch(o1.S) {
								case 1:
W_as_0:
									k=subStmt(TS,ADDR_8,&mn);		// 
									if(k>=0) {
										uint16_t n;
										if(Mnn==4 || Mnn==5) {                // BCF, BSF
											if(o2.l.v>7)
												PROCError(2062,T2S);
											if(o1.l.v & 0xf80)
												PROCWarn(322,T1S);
											n=mn->a[k].n | ((LOBYTE(LOWORD(o1.l.v)) & 0x7f) | ((o2.l.v & 7) << 7));
											}
										else {                //    
											if(o2.l.v>1)
												PROCError(2062,T2S);
											if(o1.l.v & 0xf80)
												PROCWarn(322,T1S);
											n=mn->a[k].n | ((LOBYTE(LOWORD(o1.l.v)) & 0x7f) | (o2.l.v ? 0x80 : 0));
											}
 	  								PROCOut(LOBYTE(n),HIBYTE(n),-1,0);    // 
										}
									break;
								}
							}
            }
					else
						PROCError(2062,T1S);
          }
			  }			// Is2
			else {                                     // 1 solo parm
		    if(o1.V>=VQ_OP_VAR) {
					switch(o1.S) {
						case 1:
  						if(Mnn==8 || Mnn==15) {                 // gestisco GOTO & CALL
			  			  k=subStmt(TS,ADDR_ABS,&mn);         // FORZO errore se imm :)
								if(k>=0) {
									uint16_t n;
									n=mn->a[k].n | ((LOBYTE(LOWORD(o1.l.v)) & 0x7f));
 	  							PROCOut(LOBYTE(n),HIBYTE(n),-1);
									}
  							}
  						else {                              // 
								uint16_t n;
								if(Mnn==0 || Mnn==2 || Mnn==18 || Mnn==21 || Mnn==26 || Mnn==31 || Mnn==35) {             // 
									k=subStmt(TS,ADDR_IMM8,&mn);         // MOVLW ecc
									if(k>=0)
										n=mn->a[k].n | (LOBYTE(LOWORD(o1.l.v)));
									}
								else {
									if(o1.l.v & 0xf80)
										PROCWarn(322,T1S);
									k=subStmt(TS,ADDR_8,&mn);         // MOWF ecc
									if(k>=0)
										n=mn->a[k].n | ((LOBYTE(LOWORD(o1.l.v)) & 0x7f)) | 0x80;	// default ,1 ossia ,F !
									}
 								PROCOut(LOBYTE(n),HIBYTE(n),-1);
								}
							break;
						case 2:
		  			  k=subStmt(TS,o1.V==VQ_VAR_IMM ? ADDR_IMM8 : ADDR_ABS,&mn);         // FORZO errore se imm :)
  						if(k>=0) {
								uint16_t n;
								n=mn->a[k].n | (o1.l.v & 0x7ff);
								if(Mnn==8 || Mnn==15) {                 // gestisco GOTO & CALL
									if(o1.l.v>2047)
										PROCError(2062,T2S);
	 	  						PROCOut(LOBYTE(n),HIBYTE(n),-1,o1.l.seg);    // con segm.
  								}
/*idem									else if(Mnn==3 || Mnn==4 || Mnn==5 || Mnn==7 || Mnn==8 || Mnn==9 || Mnn==10 || Mnn==12 || Mnn==13) {                 // gestisco Bxx
									subBranch(TS,T2S,mn,T,k,l,FALSE);
									}
									*/
  							else {                              // 
	 	  						PROCOut(LOBYTE(n),HIBYTE(n),-1,o1.l.seg);    // con segm.
									}
								}
							else
				        PROCError(2062,T1S);
							break;
						case 4:
		  			  k=subStmt(TS,o1.V==VQ_VAR_IMM ? ADDR_IMM8 : ADDR_ABS,&mn);         // Bxx; idem come sopra
  						if(k>=0) {
								uint16_t n;
								n=mn->a[k].n | (o1.l.v & 0x7ff);
	  						if(Mnn==8 || Mnn==15) {                 // gestisco GOTO & CALL
	 	  						PROCOut(LOBYTE(n),HIBYTE(n),-1,o1.l.seg);    // con segm.
  								}
  							else {                              // 
	 	  						PROCOut(LOBYTE(n),HIBYTE(n),-1,o1.l.seg);    // con segm.
									}
								break;
	  					}
						}
			    }
			  else if(o1.V==VQ_REGISTER) {
			    if(o1.Ind) {
		        if(o1.S==2) {
		          PROCError(2062,T1S);
			        }
		        else {
		          PROCError(2064,T1S);
		          }
			      }
          else {
				    if(o1.S==1) {
					    if(o1.T==1) {
	  			      k=subStmt(TS,ADDR_REG8,&mn);         // LSR ecc A
  		          PROCOut(mn->a[k].n);
					      }
					    else {
					      PROCError(2062,T1S);
					      }
					    }
				    else {
		          PROCError(2062,T1S);
	 			      }
				    }
				  }
			  }
    
#elif PIC==18
#pragma warning FINIRE  circa ok 21/2
// usare Is3 ecc
	    if(Is2) {
				FNLA(T3S);
				// cmq può esserci ,0 o ,1, ,BANKED o ,ACCES
				if(*T3S) {
					PROCCheck(',');
					Is3=TRUE;

rifoIsVar3:
					p=theLinePtr;
					*T3S=0;
					o3.S=0;
					o3.T=0;
					o3.V=0;
					o3.op=0;
					o3.l.valid=FALSE;
					switch(FNGetAritElem(T3S,&o3,0)) {
						case ARITM_EOL:
							PROCError(2060);          // se c'è virgola e poi [fineriga], errore!
							break;
						case ARITM_CONST:
isVar3:
							if(o3.V==VQ_REGISTER) {  
								o3.T;
								o3.S=4;
								}
							else {
//								o3.V=VQ_OP_VAR;
								theLinePtr=p;
								o3.l=FNEvalExpr(14,T3S);
								if(!o3.l.valid)
									PROCError(2062);
								if(o3.Ind) {
									o3.Sptr=FNGetSize(o3.l);
						o3.S=0/*INT_SIZE*/;
									}
								else {
									o3.S=FNGetSize(o3.l);
									}
//								o3.l.valid=TRUE;
								}
							break;
						case ARITM_VAR:
							if(o3.V>=VQ_OP_VAR)
								goto isVar3;
							break;
						case ARITM_OPER:
							if(o3.op == 1)
								PROCError(2060);
							else
								goto isVar2;
							break;
						}
					}             

				if(o1.V==VQ_REGISTER) {
					uint16_t n;
					k=subStmt(TS,ADDR_IMPL,&mn);		// 
					if(k>=0) {
						switch(*T1S) {
							case '*':
								switch(*T2S) {
									case '+':
										n=1;
										break;
									case '-':
										n=2;
										break;
									default:
										PROCError(2062,T1S);
										break;
									}
								break;
							case '+':
								n=3;
								break;
							default:
			          PROCError(2062,T1S);
								break;
							}
						n |= mn->a[k].n;
	 					PROCOut2(n,-1,0);
						}
//          PROCError(2062,T1S);
					}
				else {
					if(o1.Ind || o2.Ind) {
						PROCError(2062,T1S);
						}
 					else {                                       // 
						if(o1.V>=VQ_OP_VAR) {                    // 
							if(o2.V==VQ_REGISTER) {
								if(o2.T==1)	{		// solo W! tanto per... (in MPLAB c'è define e basta
									o2.l.v=0;
									goto W_as_0;
									}
								}
							else {
								switch(o1.S) {
									uint16_t n;
									case 1:
									case 2:		// boh metto tutto così, ev. testare
									case 4:
W_as_0:
										if(Mnn==8 || Mnn==16 || Mnn==19) {                // BCF, BSF, BTG
											k=subStmt(TS,ADDR_8,&mn);		// 
											if(k>=0) {
												if(o2.l.v>7)
													PROCError(2062,T2S);
												n=mn->a[k].n | LOBYTE(LOWORD(o1.l.v)) | ((o2.l.v & 7) << 9) | (o3.l.v ? 0x100 : 0);
						 						PROCOut2(n,-1,0);
												}
											}
										else if(Mnn==39) {                 // gestisco LFSR
											if(o1.l.v>2)
												PROCError(2062,T1S);
											if(o2.l.v>4095)
												PROCError(2062,T2S);
											k=subStmt(TS,ADDR_IMM16,&mn);         // 
											if(k>=0) {
												n=LOWORD(mn->a[k].n) | ((o2.l.v & 0xffff) >> 8) | (o1.l.v << 4);
	 											PROCOut2(n,-1,0);
												n=HIWORD(mn->a[k].n) | LOBYTE(LOWORD(o2.l.v));
	 											PROCOut2(n,-1,0);
												}
											}
										else if(Mnn==41) {                 // gestisco MOVFF
											k=subStmt(TS,ADDR_ABS2,&mn);         // 
											if(k>=0) {
												n=LOWORD(mn->a[k].n) | (o1.l.v & 0xfff);
	 											PROCOut2(n,-1,0);
												n=HIWORD(mn->a[k].n) | (o2.l.v & 0xfff);
	 											PROCOut2(n,-1,0);
												}
											}
										else {                //    
											k=subStmt(TS,ADDR_8,&mn);		// 
											if(k>=0) {
												if(o2.l.v>1)
													PROCError(2062,T2S);
												n=mn->a[k].n | LOBYTE(LOWORD(o1.l.v)) | (o2.l.v ? 0x200 : 0) | (o3.l.v ? 0x100 : 0);
	 	  									PROCOut2(n,-1,0);    // 
												}
											}
										break;
									}
								}
							}
						else
							PROCError(2062,T1S);
						}
          }
			  }			// Is2
			else {                                     // 1 solo parm
		    if(o1.V>=VQ_OP_VAR) {
					switch(o1.S) {
						uint16_t n;
						case 1:
  						{                              // 
							if(Mnn==1 || Mnn==5 || Mnn==37 || Mnn==43 || Mnn==47 || Mnn==57 || Mnn==67 || Mnn==75) {             // 
								k=subStmt(TS,ADDR_IMM8,&mn);         // MOVLW ecc
								if(k>=0)
									n=mn->a[k].n | (LOBYTE(LOWORD(o1.l.v)));
	 							PROCOut2(n,-1);
								}
							else if(Mnn==21 || Mnn==33) {         // CALL, GOTO 
								k=subStmt(TS,ADDR_ABS,&mn);         
								if(k>=0) {
									n=LOWORD(mn->a[k].n) | LOBYTE(LOWORD(o1.l.v));
									if(Mnn==21)
										n |= o3.l.v ? 0x100 : 0;			// ,s ******* occhio finire!
	 								PROCOut2(n,-1,o1.l.seg);    // con segm.
									n=HIWORD(mn->a[k].n) | ((o1.l.v >> 8) & 0xfff);
	 								PROCOut2(n,-1,o1.l.seg);    // con segm.
									}
								}
							else if(Mnn==7 || Mnn==9 || Mnn==10 || Mnn==11 || Mnn==12 || Mnn==13 || Mnn==14 || Mnn==15 || Mnn==20) {                 // gestisco Bxx
			  			  k=subStmt(TS,ADDR_ABS,&mn);         // FORZO errore se imm :)
								if(k>=0)
									subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);
								}
							else if(Mnn==58) {                 // gestisco RETURN s
								if(o1.l.v>1)
									PROCError(2062,T1S);
								k=subStmt(TS,ADDR_IMPL,&mn);         // 
								if(k>=0) {
									n=mn->a[k].n | (o1.l.v ? 1 : 0);
		 							PROCOut2(n,-1);
									}
								}
							else if(Mnn==42) {                 // gestisco MOVLB
								k=subStmt(TS,ADDR_IMM8,&mn);
								if(k>=0) {
									n=mn->a[k].n | LOBYTE(LOWORD(o1.l.v));
	 								PROCOut2(n,-1);
									}
								}
							else {
								k=subStmt(TS,ADDR_8,&mn);         // MOWF ecc
								if(k>=0) {
									n=mn->a[k].n | LOBYTE(LOWORD(o1.l.v));
	 								PROCOut2(n,-1);
									}
								}
							}
							break;
						case 2:
							if(Mnn==21 || Mnn==33) {              // CALL, GOTO 
								k=subStmt(TS,ADDR_ABS,&mn);     
								if(k>=0) {
									n=LOWORD(mn->a[k].n) | LOBYTE(LOWORD(o1.l.v));
									if(Mnn==21)
										n |= o3.l.v ? 0x100 : 0;			// ,s ******* occhio finire!
 									PROCOut2(n,-1,o1.l.seg);    // con segm.
									n=HIWORD(mn->a[k].n) | ((o1.l.v >> 8) & 0xfff);
 									PROCOut2(n,-1,o1.l.seg);    // con segm.
									}
								}
							else if(Mnn==7 || Mnn==9 || Mnn==10 || Mnn==11 || Mnn==12 || Mnn==13 || Mnn==14 || Mnn==15 || Mnn==20) {                 // gestisco Bxx
			  			  k=subStmt(TS,ADDR_ABS,&mn);         // FORZO errore se imm :)
								if(k>=0)
									subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);
								}
  						else {                              // 
			  			  k=subStmt(TS,ADDR_IMM8,&mn);         // FORZO errore se imm :)
								if(k>=0) {
									n=LOWORD(mn->a[k].n) | LOBYTE(LOWORD(o1.l.v));
	 	  						PROCOut2(n,-1,o1.l.seg);    // con segm.
									}
//							else
//				        PROCError(2062,T1S);
								}
							break;
						case 4:
							if(Mnn==21 || Mnn==33) {              // CALL, GOTO 
								k=subStmt(TS,ADDR_ABS,&mn);     
								if(k>=0) {
									n=LOWORD(mn->a[k].n) | LOBYTE(LOWORD(o1.l.v));
									if(Mnn==21)
										n |= o3.l.v ? 0x100 : 0;			// ,s ******* occhio finire!
	 								PROCOut2(n,-1,o1.l.seg);    // con segm.
									n=HIWORD(mn->a[k].n) | ((o1.l.v >> 8) & 0xfff);
	 								PROCOut2(n,-1,o1.l.seg);    // con segm.
									}
								}
							else if(Mnn==7 || Mnn==9 || Mnn==10 || Mnn==11 || Mnn==12 || Mnn==13 || Mnn==14 || Mnn==15 || Mnn==20) {                 // gestisco Bxx
				  			k=subStmt(TS,ADDR_ABS,&mn);         // FORZO errore se imm :)
  							if(k>=0) {
									n=mn->a[k].n | (o1.l.v & 0x7ff);
									subBranch(TS,T2S,mn,o1.T,k,o1.l,FALSE);
									}
								}
  						else {                              // 
		  					k=subStmt(TS,ADDR_8,&mn);         // FORZO errore se imm :)
  							if(k>=0) {
									n=mn->a[k].n | (o1.l.v & 0x7ff);
		 	  					PROCOut2(n,-1,o1.l.seg);    // con segm.
									}
								}
							break;
						}
			    }
			  else if(o1.V==VQ_REGISTER) {
			    if(o1.Ind) {
	          PROCError(2064,T1S);
			      }
          else {
						uint16_t n;
				    if(o1.S==1) {
				      PROCError(2062,T1S);
					    }
				    else if(o1.S==0) {
  			      k=subStmt(TS,ADDR_IMPL,&mn);         // gestisco TBLRD...
							n=mn->a[k].n;
 		          PROCOut2(n,-1,0);
					    }
				    else {
		          PROCError(2062,T1S);
	 			      }
				    }
				  }
			  }
    
#elif GD24032

	    if(Is2) {
				if(Mnn==69 || Mnn==71) {		// gestisco RET
  				k=subStmt(TS,ADDR_ALL_2OP,&mn);         // 
  				if(k>=0) {
						gdop.d=mn->a[k].n;
// già in opcode						gdop.mode2=ADDR_REGISTER_DIRECT;		// v. fw, !=IMMEDIATE
						if(o1.Ind || o2.Ind || o1.preOp || o1.postOp)
              PROCError(2062,T1S);
						gdop.reg2=(Reg[o1.T].m - Reg[0].m);
						if(forcedSize)
							PROCError(2060);
						gdop.size=0;
						if(useCondiz) {
							gdop.condiz=1;
							gdop.o3.cond=useCondiz-1;
							}
						gdop.o3.val=o2.l.v;
						PROCOut2(gdop.d);    // 
						}
					}
				else if(Mnn==103) {			// XOP
 					k=subStmt(TS,ADDR_ALL_2OP,&mn);         // 
 					if(k>=0) {
						}
					if(forcedSize)
						PROCError(2060);
					}
				else {		// 
 					if(Mnn==9 || Mnn==24 || Mnn==22 ||  // gestisco BINS,BXTR, BSFR
						Mnn==29 || Mnn==54 || Mnn==64)    // gestisco MOVS, CMPS, OUTS
						k=subStmt(TS,ADDR_ALL_3OP,&mn);
// mah no, è opzionale... 					else if(Mnn==52 || Mnn==55 || Mnn==86 || Mnn==99)     // gestisco MAS, MSS, SSA, VMA
					else if(Mnn==99)     // gestisco VMA, questo è sicuro :)
						k=subStmt(TS,ADDR_ALL_3OP,&mn);
					else {
						if(o3.l.valid)
							PROCError(2062);
 						k=subStmt(TS,ADDR_ALL_2OP,&mn);         // 
						}
 					if(k>=0) {
						gdop.d=mn->a[k].n;
						o3.l.valid=FALSE;

						o1.S=o2.S;
						if(forcedSize) {
							if(o1.S>forcedSize && o2.V != VQ_REGISTER && !o2.Ind) {
								if(CPass==0)
									PROCWarn(4013,"");
								}
							o1.S=forcedSize;
							}

						if(o1.V==VQ_REGISTER) {		
							if(Mnn==47) 			// LDSP 
								PROCError(2062,T1S);
							gdop.reg2=(Reg[o1.T].m - Reg[0].m);
					    if(o1.Ind) {
								if(o1.l.valid) {		// se base-offset...

									if(o1.reg2reg) {
										PROCError(2068,"indirizzamento 2reg");
										}
									else {
										if(o1.T==0)
		  								PROCError(2062,Reg[o1.T].s);

										if(abs(LODWORD(o1.l.v))<=127) {
											if(o1.S==1 || o2.V==VQ_REGISTER)	{		// questo usa gli stessi bit!
												if(!CPass)
													PROCWarn(2068);
												goto no_short_addr;
												}
											gdop.mode2=ADDR_REGISTER_SYMBOLIC_INDEXED_SHORT;
											gdop.o3.val=o1.l.v;
											o1.l.valid=FALSE;
											}
										else {
no_short_addr:
											gdop.mode2=ADDR_REGISTER_SYMBOLIC_INDEXED;
											}
										}
									}
								else {
									if(o1.reg2reg) {
										gdop.mode2=ADDR_REGISTER_INDEXED_2_REG;
										gdop.reg3=o1.reg2reg-1;
										}
									else
										gdop.mode2=ADDR_REGISTER_INDIRECT;
									}
								}			// se indiretto1
							else {
								gdop.mode2=ADDR_REGISTER_DIRECT;
								if(o1.S==8 && (o1.T & 1))
									if(!CPass)
										PROCWarn(2062,"registro dispari");		// mah sì
								}
							}			// se register1
						else {
							if(o1.Ind) {
								if(Mnn==47) 			// LDSP 
									PROCError(2062,T1S);
								gdop.mode2=ADDR_REGISTER_SYMBOLIC_INDEXED;
								}
							else {
			 					if(Mnn==63)    // gestisco OUT , accetto immediate :)
									;
								else if(Mnn==47) {			// LDSP 
//									if(o1.S<4)
//										PROCError(2064,forcedSize);
									gdop.mode1=gdop.mode2;
									gdop.reg1=gdop.reg2;
									gdop.mode2=0;
									gdop.reg2 = 2 | (o1.l.v ? 1 : 0);		// mettere "nome, SP, USP..."
									o1.l.valid=FALSE;
									o1.S=4;
									}
								else
									PROCError(2062,T1S);
								}
							}

						if(o2.V==VQ_REGISTER) {                         // MOV [Rn],Rn
							gdop.reg1=(Reg[o2.T].m - Reg[0].m);
					    if(o2.Ind) {
								if(o2.l.valid) {		// se base-offset...
									if(o2.T==0)
	  								PROCError(2062,Reg[o2.T].s);

									if(o2.reg2reg) {
										PROCError(2068,"indirizzamento 2reg");
										}
									else {
										if(o2.T==0)
		  								PROCError(2062,Reg[o2.T].s);

										if(abs(LODWORD(o2.l.v))<=127) {
											if(gdop.mode2 != ADDR_REGISTER_SYMBOLIC_INDEXED_SHORT) {
												if(o2.S==1 || o2.V==VQ_REGISTER)	{		// questo usa gli stessi bit!
													if(!CPass)
														PROCWarn(2068);
													goto no_short_addr2;
													}
												gdop.mode1=ADDR_REGISTER_SYMBOLIC_INDEXED_SHORT;
												gdop.o3.val=o2.l.v;
												o2.l.valid=FALSE;
												}
											else
												PROCError(2068,"indirizzamento short");
											}
										else
no_short_addr2:
											gdop.mode1=ADDR_REGISTER_SYMBOLIC_INDEXED;
										}
									}
								else {
									if(o2.reg2reg) {
										if(o1.reg2reg)
											PROCError(2068,"indirizzamento 2reg 2 volte");
										gdop.mode1=ADDR_REGISTER_INDEXED_2_REG;
										gdop.reg3=o2.reg2reg-1;
										}
									else
										gdop.mode1=ADDR_REGISTER_INDIRECT;
									}
								}			// se indiretto
							else {
								gdop.mode1=ADDR_REGISTER_DIRECT;
								if(o2.S==8 && (o2.T & 1))
									if(!CPass)
										PROCWarn(2062,"registro dispari");		// mah sì
								if(o2.l.valid)
									PROCError(2062);
								}
							}		// se register2
						else if(o2.V>=VQ_OP_VAR) {                       // MOV [Rn+nn],20
							if(o2.Ind)
								gdop.mode1=ADDR_REGISTER_SYMBOLIC_INDEXED;
							else {
								gdop.mode1=ADDR_IMMEDIATE;
								if(o2.S==1) {			// per favorire immediate8 ...
									if((Mnn>=72 && Mnn<=75) || (Mnn>=83 && Mnn<=85) || (Mnn==50 || Mnn==65) || (Mnn==52 || Mnn==55 || Mnn==86 || Mnn==99)) 		// gestisco SLA ecc; LEA PEA; MAS MSS SSA VMA
										;		// gestisco SLA ecc
									else {
										gdop.o3.val= o2.l.v & 0xff;
										o2.l.valid=FALSE;
										gdop.mode1=ADDR_IMMEDIATE8;
										}
									}
								else {
									switch(o1.S) {
										case 8:
											if(forcedSize) {
												o3.l.v=HIDWORD(o2.l.v);
												o3.l.valid=TRUE;
												}
											break;
										}
									}
								}
							}

						if(Mnn==50 || Mnn==65) {		// LEA PEA
							if(forcedSize) {		// usare ! modifier come su registri, per FAR...
								if(forcedSize<4) 
									PROCError(2064,forcedSize);
								else
									o1.S=forcedSize;
								}
							else
								o1.S=4;
							if(o1.S==8) {
								if(gdop.mode2==ADDR_IMMEDIATE) {
									o2.l.valid=TRUE;
									o2.l.v=HIDWORD(o1.l.v);
//									o2.l.seg=HIDWORD(o1.l.v);
									}
								}
							if(useCondiz)
								PROCError(2060,T1S);
							}
						//GESTIRE FAR address!

						gdop.size=(o1.S==8 ? 3 : (o1.S>>1));
		  	    if(Mnn==28 || Mnn==95 || Mnn==96) {   // CMP TB e TEST forzano flag :)
							gdop.doflags=1;
			  	    if(Mnn==95) {   // gestisco TB 
								if(gdop.mode1 <= ADDR_IMMEDIATE8) {
									gdop.o3.val=o2.l.v;
									o2.l.valid=FALSE;
									}

								}
							}
		  	    else if(Mnn==50 || Mnn==65) {   // LEA e PEA si gestiscono
							}
						else
							gdop.doflags=useFlags;

						if(o1.addr64) {		// finire...
							if(gdop.mode2 == ADDR_REGISTER_INDIRECT)
								gdop.mode2=ADDR_REGISTER_INDIRECT64;
							if(!CPass)
								PROCWarn(2062,"64bit addressing needs to be tested");
							}
						if(o2.addr64) {		// finire...
							if(gdop.mode1 == ADDR_REGISTER_INDIRECT)
								gdop.mode1=ADDR_REGISTER_INDIRECT64;
							if(!CPass)
								PROCWarn(2062,"64bit addressing needs to be tested");
							}
						if(o1.preOp || o1.postOp) {
							if(gdop.mode2 != ADDR_REGISTER_INDIRECT)
								PROCError(2062,T1S);
							if(o1.preOp)
								gdop.mode2=o1.preOp>0 ? ADDR_REGISTER_INDIRECT_PREINCREMENT : ADDR_REGISTER_INDIRECT_PREDECREMENT;
							else if(o1.postOp)
								gdop.mode2=o1.postOp>0 ? ADDR_REGISTER_INDIRECT_POSTINCREMENT : ADDR_REGISTER_INDIRECT_POSTDECREMENT;
							}
						if(o2.preOp || o2.postOp) {
							if(gdop.mode1 != ADDR_REGISTER_INDIRECT)
								PROCError(2062,T2S);
							if(o2.preOp)
								gdop.mode1=o2.preOp>0 ? ADDR_REGISTER_INDIRECT_PREINCREMENT : ADDR_REGISTER_INDIRECT_PREDECREMENT;
							else if(o2.postOp)
								gdop.mode1=o2.postOp>0 ? ADDR_REGISTER_INDIRECT_POSTINCREMENT : ADDR_REGISTER_INDIRECT_POSTDECREMENT;
							}

						if(Mnn==9 || Mnn==22 || Mnn==24) {   // BINS,BSFR, BXTR
							if(*FNLA(T3S)) {        // 
								char T4S[256];        // 

							if(gdop.mode2 == ADDR_IMMEDIATE )
									PROCError(2062);
								
								if(forcedSize && forcedSize<4)
									PROCError(2064,forcedSize);
								gdop.size=max(2,forcedSize);		// RIVERIFICARE...
//								if((!o1.Ind && !o2.Ind) && o1.S<4)
//									PROCError(2064,forcedSize);

								PROCCheck(',');
								*T3S=0;
								o3.S=0;
								o3.T=0;
								o3.V=0;
								o3.op=0;
								o3.l.valid = FALSE;
								Is3=1;
								if(Mnn==22) {   // BSFR
									gdop.reg3 = 0x00000002;				// type default su "1" (e Dir=0
rifo_bsfr:
									p=theLinePtr;
									switch(*FNLO(T3S)) {
										case 'L':
										case 'l':
											gdop.reg3 &= ~0x00000001;			// direction (default
											break;
										case 'R':
										case 'r':
											if(*(T3S+1))		// sicurezza in caso di registro :)
												PROCError(2062,T3S);
											gdop.reg3 |= 0x00000001;			// direction
											break;
										case '0':
											gdop.reg3 &= ~0x00000002;			// type
											break;
										case '1':
											gdop.reg3 |= 0x00000002;				// type
											break;
										default:
											PROCError(2062,T3S);
											break;
										}
									if(*FNLA(T3S)) {
										PROCCheck(',');
										goto rifo_bsfr;
										}
									}
								else {
									OldTextp=theLinePtr;
									i=FNGetAritElem(T3S,&o3,0);
									if(i>ARITM_EOL && i<ARITM_OPER && o3.V>=0) {
										*T3S=0;
										theLinePtr=OldTextp;
										o3.l=FNEvalExpr(14,T3S);
//										o3.l.v=*(uint64_t*)T3S;
//										o3.l.valid=TRUE;
										// if size   if l3>31 o 63
										if(*FNLA(T4S)) {        // 
											PROCCheck(',');
											*T4S=0;
											o4.l=FNEvalExpr(14,T4S);
											if(o4.l.valid) {
												if(o3.l.v>63 || o4.l.v>63)			// FINIRE!
													PROCError(2062,T3S);
												o3.l.v=MAKEWORD(LOBYTE(o3.l.v),LOBYTE(o4.l.v));
												if(*FNLA(T4S)) {
rifo_bins:
													PROCCheck(',');
													switch(*FNLO(T4S)) {
														case 'U':
														case 'u':
															o3.l.v &= ~0x80000000;		// unsigned
															break;
														case 'S':
														case 's':
															o3.l.v |= 0x80000000;			// signed
															break;
														case 'E':
														case 'e':
															if(Mnn==9)    // BINS
																o3.l.v |= 0x40000000;			// fill/extend
															else
																PROCError(2062,T4S);
															break;
														default:
															PROCError(2062,T4S);
															break;
														}
													if(*FNLA(T4S)) {
														goto rifo_bins;
														}
													}
												if(o1.S==8 && (o1.T & 1 || o2.T & 1))
													if(!CPass)
														PROCWarn(2062,"registro dispari");		// mah sì
												}
											else
												PROCError(2062,T3S);
											}
	//									PROCOut2(o3.l);    // 
										}
									else if(o3.V==VQ_REGISTER) {
										if(o3.T>7) {
											PROCError(2062,T2S);
											}
										gdop.reg3=(Reg[o3.T].m - Reg[0].m);
	//									o1.l.valid=o2.l.valid=FALSE;
	//									PROCOut2(o3.l);    // 
	//									o3.l.valid=FALSE;
										}
									else
										PROCError(2062,T3S);
									}
								}
							else
								PROCError(2062,T1S);
							}				// BINS, BXTR
						else if(Mnn==35) {   // ENTER
							if(gdop.mode2 != ADDR_REGISTER_DIRECT)
								PROCError(2062,T1S);
							}				// 
						else if(Mnn==46) {   // LDM
							if(gdop.mode1 < ADDR_REGISTER_INDIRECT_PREINCREMENT || gdop.mode1 > ADDR_REGISTER_INDIRECT_POSTDECREMENT)
								PROCError(2062,T1S);

							gdop.mode2=gdop.mode1;
							gdop.reg2=gdop.reg1;
							gdop.reg1=0;
							if(!(o1.l.v & ~0xff)) {
								gdop.o3.val= o1.l.v & 0xff;
								o1.l.valid=FALSE;
								gdop.mode1=ADDR_IMMEDIATE8;		// [NON 8! è un flag
								}
							else {
								o1.l.valid=TRUE;
								gdop.mode1=ADDR_REGISTER_DIRECT;		// v. CPU
								}
							if(!forcedSize)
								gdop.size=2;
							if(useCondiz)
								PROCError(2060,T1S);
							}				
						else if(Mnn==88) {   // STM
							if(gdop.mode2 < ADDR_REGISTER_INDIRECT_PREINCREMENT || gdop.mode2 > ADDR_REGISTER_INDIRECT_POSTDECREMENT)
								PROCError(2062,T1S);

							if(!(o2.l.v & ~0xff)) {
								gdop.o3.val= o2.l.v & 0xff;
								o2.l.valid=FALSE;
								gdop.mode1=ADDR_IMMEDIATE8;		// [NON 8! è un flag
								}
							else {
								o2.l.valid=TRUE;
								gdop.mode1=ADDR_REGISTER_DIRECT;		// v. CPU
								}
							if(!forcedSize)
								gdop.size=2;
							if(useCondiz)
								PROCError(2060,T1S);
							}				
						else if(Mnn==29 || Mnn==54 || Mnn==64) {    // gestisco MOVS, CMPS, OUTS
							if(gdop.mode2 < ADDR_REGISTER_INDIRECT_PREINCREMENT || gdop.mode2 > ADDR_REGISTER_INDIRECT_POSTDECREMENT)
								PROCError(2062,T1S);
							if(*FNLA(T3S)) {        // 
								PROCCheck(',');
								*T3S=0;
								o3.S=0;
								o3.T=0;
								o3.V=0;
								o3.op=0;
								i=FNGetAritElem(T3S,&o3,0);
								if(i>ARITM_EOL && i<ARITM_OPER && o3.V != VQ_REGISTER || o3.T>7) {
									PROCError(2062,T3S);		// occhio esce merda se T3S ha trovato costante...
									}
								}
							else
								PROCError(2062,T1S);
							gdop.reg3=(Reg[o3.T].m - Reg[0].m);
							if(useCondiz)
								PROCError(2060,T1S);
							}
						else if(Mnn==42) {   // INS
							if(gdop.mode2 < ADDR_REGISTER_INDIRECT_PREINCREMENT || gdop.mode2 > ADDR_REGISTER_INDIRECT_POSTDECREMENT)
								PROCError(2062,T1S);
							if(o2.V != VQ_REGISTER || o2.T>7) {
								PROCError(2062,T2S);
								}
							gdop.reg3=(Reg[o2.T].m - Reg[0].m);
							if(useCondiz)
								PROCError(2060,T1S);
							}
						else if((Mnn>=72 && Mnn<=75) || (Mnn>=83 && Mnn<=85)) {		// gestisco SLA ecc
							if(gdop.mode2 == ADDR_REGISTER_SYMBOLIC_INDEXED_SHORT) 
								PROCError(2068,"indirizzamento 2reg");
							if(o2.V>=VQ_OP_VAR) {                       // SLA D,2
								if(o2.l.v<0 || o2.l.v>15)
									PROCError(2062,T2S);
								gdop.o5.which=1;
								gdop.o5.shiftcnt=o2.l.v;
								o2.l.valid=FALSE;
								}
							else
								gdop.o5.which=0;

							if(!forcedSize)		// serve patchare causa secondo operando "piccolo"!
								gdop.size=2;
//							else
//								gdop.size=forcedSize;
							if(useCondiz) {
								gdop.condiz=1;
								gdop.o3.cond=useCondiz-1;
								}
							}
						else if(Mnn==52 || Mnn==55 || Mnn==86) {    // gestisco MAS, MSS, SSA
							if(*FNLA(T3S)) {        // 
								PROCCheck(',');
								FNLA(T3S);
								if(*T3S=='-') {			// ma cmq qua non serve, v.operando 2 sopra!
									OldTextp=theLinePtr;
									PROCCheck('-');
									i=FNGetAritElem(T3S,&o3,0);
									theLinePtr=OldTextp;
									if(o3.V==VQ_REGISTER) {
										PROCError(2062,T3S);
										}
									}
								*T3S=0;
								o3.S=0;
								o3.T=0;
								o3.V=0;
								o3.op=0;
								OldTextp=theLinePtr;
								i=FNGetAritElem(T3S,&o3,0);
								switch(o3.V) {
									case VQ_REGISTER:
										if(o3.T>7)
											PROCError(2062,T3S);		// occhio esce merda se T3S ha trovato costante...
										gdop.reg3=(Reg[o3.T].m - Reg[0].m);
										o3.l.valid=TRUE;
										break;
									case VQ_OP_VAR:
									case VQ_VAR:
										*T3S=0;
										theLinePtr=OldTextp;
										o3.l=FNEvalExpr(14,T3S);
										break;
									}
								}
							switch(Mnn) {
								case 52:			// MAS
									if(o3.l.valid) {
										gdop.condiz=1;
										if(o3.V == VQ_REGISTER)
											o3.l.valid=FALSE;
										else {
											if(!o3.l.v)
												if(!CPass)
													PROCWarn(2062,"addendo=0");
											}
										}
									break;
								case 55:			// MSS
									if(op2negated) {
										gdop.condiz=1;
										}
									if(o3.l.valid) {
										if(o3.V == VQ_REGISTER)
											o3.l.valid=FALSE;
										else {
											if(!o3.l.v)
												if(!CPass)
													PROCWarn(2062,"addendo=0");
											}
										}
									break;
								case 86:			// SSA
									if(o3.l.valid) {
										gdop.condiz=1;
										if(o3.V == VQ_REGISTER)
											o3.l.valid=FALSE;
										else {
											if(!o3.l.v)
												if(!CPass)
													PROCWarn(2062,"addendo=0");
											}
										}
									break;
								}
							if(!forcedSize)		// serve patchare causa altri operandi "piccolo"!
								gdop.size=2;
							if(useCondiz)
								PROCError(2060,T1S);
							}
						else if(Mnn==99) {    // gestisco VMA
							PROCCheck(',');
							*T3S=0;
							o3.S=0;
							o3.T=0;
							o3.V=0;
							o3.op=0;
							OldTextp=theLinePtr;
							i=FNGetAritElem(T3S,&o3,0);
							switch(o3.V) {
								case VQ_REGISTER:
									if(o3.T>7)
										PROCError(2062,T3S);		// occhio esce merda se T3S ha trovato costante...
									gdop.reg3=(Reg[o3.T].m - Reg[0].m);
									o3.l.valid=FALSE;
									break;
								case VQ_OP_VAR:
								case VQ_VAR:
									*T3S=0;
									theLinePtr=OldTextp;
									o3.l=FNEvalExpr(14,T3S);
									break;
								}
							o4=o2;
							o2=o3;
							o3=o4;
							if(!o3.l.valid || o3.l.v>1000000L)
								PROCError(2062,"array size");
							if(gdop.mode2 == ADDR_REGISTER_DIRECT
								|| gdop.mode2 == ADDR_REGISTER_INDIRECT_PREINCREMENT || gdop.mode2 == ADDR_REGISTER_INDIRECT_PREDECREMENT
								|| gdop.mode2 == ADDR_REGISTER_INDIRECT_POSTDECREMENT)
								PROCError(2062,T1S);
							if(*FNLA(T3S)) {        // 
								PROCCheck(',');
								o4=o3;
								*T3S=0;
								o3.S=0;
								o3.T=0;
								o3.V=0;
								o3.op=0;
								OldTextp=theLinePtr;
								i=FNGetAritElem(T3S,&o3,0);
								switch(o3.V) {
									case VQ_REGISTER:
										if(o3.T>7)
											PROCError(2062,T3S);		// occhio esce merda se T4S ha trovato costante...
										gdop.reg3=(Reg[o3.T].m - Reg[0].m);
										o3.l.valid=TRUE;
										break;
									case VQ_OP_VAR:
									case VQ_VAR:
										*T3S=0;
										theLinePtr=OldTextp;
										o3.l=FNEvalExpr(14,T3S);
										break;
									}
								}
							if(o3.l.valid) {
								gdop.doflags=1;
								if(o3.V == VQ_REGISTER)
									o3.l.valid=FALSE;
								else {
									if(!o3.l.v)
										PROCWarn(2062,"addendo=0");
									}
								}
							if(!forcedSize)		// serve patchare causa altri operandi "piccolo"!
								gdop.size=2;
							if(useCondiz)
								PROCError(2060,T1S);
							}
						else {
							if(useCondiz) 
								PROCError(2062,T1S);
							}
  					/*else no!*/ if(Mnn==34) {   // DJNZ
							if(gdop.mode1 <= ADDR_IMMEDIATE8) {
								if(!CPass) {
									o2.l.v=Seg[CurSeg-1].CurAddr+4;           // nella pass 0 salto fittizio,  v. subBranch
									o2.l.seg=CurSeg;
									}
								o2.l.v=(int32_t)(o2.l.v-Seg[CurSeg-1].CurAddr-4)/4;		// USARE SEGMENTO QUA??
								if(abs(LODWORD(o2.l.v)) > 0x7f) 
									PROCError(2062,T2S);
								else {
									gdop.o3.val=o2.l.v;
									o1.l.valid=o2.l.valid=FALSE;
									}
								}
							else
								PROCWarn(2062,"DJNZ");		// com'è qua??
							}
						if(Mnn==87) {			// gestisco STEX 
							if(forcedSize)
								PROCError(2060);
//							if(o1.S<4)
//								PROCError(2064,forcedSize);
							gdop.size=2;
							gdop.o3.val=0;
							gdop.reg1=o2.l.v+2;
							o2.l.valid=FALSE;
							}
						if(Mnn==102) {		// gestisco XLATB (preservo flag
							gdop.reg1 |= 8;
							}

						if(o2.l.valid)
							PROCOut2(gdop.d,&o2.l);    // con segm.
						else
							PROCOut2(gdop.d);    // 

						if(o1.l.valid)
							PROCOut2(o1.l);    // con segm.
						if(o3.l.valid)
							PROCOut2(o3.l);    // con segm.

						if(Mnn==99) {
							if(o4.l.valid)
								PROCOut2(o4.l);    // con segm.
							}

						/*if(CPass) {
							printf("valid %u %u %u\n",o1.l.valid,o2.l.valid,o3.l.valid);
while(!kbhit());
getch();
						}*/

						}

					}
			  }		// Is2
			else {                                     // 1 solo parm
				if(Mnn==97) {		// gestisco TRAP
					if(o1.V==VQ_REGISTER || o1.Ind) {
						PROCError(2060,T1S);
						}
  				k=subStmt(TS,ADDR_IMM8,&mn);         // TRAP n
  				if(k>=0) {
						gdop.d=mn->a[k].n;
						gdop.mode1=ADDR_IMMEDIATE8;
						if(o1.l.v>31)
							PROCError(2062,T1S);
						if(forcedSize)
							PROCError(2060);
						gdop.size=0;
						if(useCondiz) {
							gdop.condiz=1;
							gdop.o3.cond=useCondiz-1;
							}
						gdop.o3.val=o1.l.v;
						PROCOut2(gdop.d);    // 
						}
					}
				else if(Mnn==82) {		// gestisco SKIP
  				k=subStmt(TS,ADDR_ALL,&mn);     
  				if(k>=0) {
						gdop.d=mn->a[k].n;
						if(o1.V==VQ_REGISTER) {		
							PROCError(2062,T1S);
							}
						else {
							gdop.mode2=ADDR_IMMEDIATE8;
							if(forcedSize)
								PROCError(2060);
							gdop.size=0;
							if(useCondiz) {
								gdop.condiz=1;
								gdop.o3.cond=useCondiz-1;
								}
							gdop.o3.val=o1.l.v;
							}
						PROCOut2(gdop.d);    // 
						}
					}
				else if(Mnn>=4 && Mnn<=23 && Mnn!=9 && Mnn!=10 && Mnn!=14 && Mnn!=22) {		// Bxx nn
					if(useCondiz)
						PROCError(2062,T1S);
  				k=subStmt(TS,ADDR_CONDIZ,&mn);         // Bxx nn
					if(forcedSize)
						PROCError(2060);
/*					gdop.d=mn->a[k].n;
					gdop.o4.condiz=1;
					gdop.mode2=ADDR_IMMEDIATE;
//							gdop.o3.cond=useCondiz-1; IMPLICITA nell op-code!
//							subBranch( fare usare
					l2=(int32_t)(LOWORD(l)         -Seg[CurSeg-1].CurAddr-2)/4;		// USARE SEGMENTED_VALUE 
					if(!forcedSize && abs(l2) <= 0x1ffff) {
						gdop.size=0;
						gdop.o4.vall=LOBYTE(l2);
						gdop.o4.valh=l2 >> 8;
						PROCOut2(gdop.d,-1,0);    // 
						}
					else {
//								subBranch ??
						}*/
					subBranch(TS,T2S,mn,o1.T,k,o1.l,TRUE);		// qua si gestisce lui cond ecc... controllare che fare

					}
				else if(Mnn==44) {		// JR nn
					if(useCondiz)		// mah sì!
						PROCError(2062,T1S);
  				k=subStmt(TS,ADDR_IMM17,&mn);         // JR nn
					if(forcedSize)
						PROCError(2060);
/*					gdop.d=mn->a[k].n;
					gdop.o4.condiz=1;
					gdop.mode2=ADDR_IMMEDIATE;
//							subBranch( fare usare
					l2=(int32_t)(LOWORD(l)         -Seg[CurSeg-1].CurAddr-4)/4;		// USARE SEGMENTED_VALUE 
					if(!forcedSize && abs(l2) <= 0x1ffff) {
						gdop.size=0;
						gdop.o4.vall=LOBYTE(l2);
						gdop.o4.valh=l2 >> 8;
						PROCOut2(gdop.d,-1,0);    // 
						}
					else {
//								subBranch ??
						}
						*/
					subBranch(TS,T2S,mn,0,k,o1.l,FALSE);			// qua si gestisce lui cond ecc... controllare che fare

					}
				else {		// 
 					k=subStmt(TS,ADDR_ALL,&mn);         // 
 					if(k>=0) {
						gdop.d=mn->a[k].n;
						if(forcedSize)
							o1.S=forcedSize;
						if(o1.V==VQ_REGISTER) {		
					    if(o1.Ind) {
								gdop.reg2=(Reg[o1.T].m - Reg[0].m);
								if(o1.l.valid) {		// se base-offset...
									if(o1.reg2reg) {
										PROCError(2068,"indirizzamento 2reg");
										}
									else {
										if(o1.T==0)
		  								PROCError(2062,Reg[o1.T].s);

										if(abs(LODWORD(o1.l.v))<=127) {
											if(o1.S==1 || o1.V==VQ_REGISTER)	{		// questo usa gli stessi bit!
												if(!CPass)		// ma le label valgono 0 al primo giro! SISTEMARE il warning ciucco in JMP (table+R8
													PROCWarn(2068);
												goto no_short_addr3;
												}
											gdop.mode2=ADDR_REGISTER_SYMBOLIC_INDEXED_SHORT;
											gdop.o3.val=o1.l.v;
											o1.l.valid=FALSE;
											}
										else {
no_short_addr3:
											gdop.mode2=ADDR_REGISTER_SYMBOLIC_INDEXED;
											}
										}
									}
								else {
									if(o1.reg2reg) {
										gdop.mode2=ADDR_REGISTER_INDEXED_2_REG;
										gdop.reg3=o1.reg2reg-1;
										}
									else
										gdop.mode2=ADDR_REGISTER_INDIRECT;
									}
								}			// se indiretto
							else {
								gdop.mode2=ADDR_REGISTER_DIRECT;
								gdop.reg2=(Reg[o1.T].m - Reg[0].m);
								if(o1.S==8 && (o1.T & 1))
									if(!CPass)
										PROCWarn(2062,"registro dispari");		// mah sì
								}
							}			// se register
						else {
							if(o1.Ind)
								gdop.mode2=ADDR_REGISTER_SYMBOLIC_INDEXED;
							else {
								    //case ADDR_IMMEDIATE8:
								gdop.mode2=ADDR_IMMEDIATE;
								}
							}

						if(Mnn==45 || Mnn==47 || Mnn==49 || 
							/*Mnn==82 2 op || */ Mnn==89 || Mnn==90 || Mnn==91) {			// LDIM LDST LDWP (STEX) STST STSP STWP
							if(forcedSize)
								PROCError(2060);
							switch(Mnn) {
								case 45:
									gdop.mode1=gdop.mode2;
									gdop.reg1=gdop.reg2;
									gdop.mode2=0;
									gdop.reg2=0;
									o1.S=1;
									if(LOBYTE(o1.l.v)>31)
										PROCError(2062,T1S);
									break;
								case 47:
//									if(o1.S<4)
//										PROCError(2064,forcedSize);
									gdop.mode1=gdop.mode2;
									gdop.reg1=gdop.reg2;
									gdop.mode2=0;
									gdop.reg2=0;
									o1.S=4;
									break;
								case 49:
									gdop.mode1=gdop.mode2;
									gdop.reg1=gdop.reg2;
									gdop.mode2=0;
									gdop.reg2=0;
									o1.S=4;
									break;
								case 90:
									break;
								case 89:
								case 91:
									if(o1.S<4)
										PROCError(2064,forcedSize);
									break;
								}
							if(useCondiz) {
								PROCError(2060,T1S);
								}
							}
						else {
							if(useCondiz) {
								gdop.condiz=1;
								gdop.o3.cond=useCondiz-1;
								}
							}
						if(gdop.mode2==ADDR_IMMEDIATE && (Mnn!=43 && Mnn!=10 && Mnn!=25)) {		// no JMP CALL BL
							switch(o1.S) {
								case 1:
									gdop.o3.val = o1.l.v & 0xff;
									o1.l.valid=FALSE;
									gdop.mode2=ADDR_IMMEDIATE8;
									break;
								case 8:
									o2.l.v=HIDWORD(o1.l.v);
									break;
								}
							}

						if(Mnn==10 || Mnn==25) {			// BL/CALL nn
		// già in opcode!						gdop.doflags=Mnn==10 ? 1 : 0;		// gestisco BL e CALL
							if(useCondiz) {		// già sopra??
								gdop.condiz=1;
								gdop.o3.cond=useCondiz-1;
								}
								    //case ADDR_IMMEDIATE8:
							if(gdop.mode2==ADDR_IMMEDIATE) {
								if(!CPass) {
									o1.l.v=Seg[CurSeg-1].CurAddr+4;           // nella pass 0 salto fittizio OKKIO v. anche subBranch
									o1.l.seg=CurSeg;


// si incasina 1/2 pass.... AGGANCIARSI A branchOpt??? che cmq non risolve!


									}
								o2.l.v=(int32_t)(o1.l.v -Seg[CurSeg-1].CurAddr-4)/4;			// USARE SEGMENTED_VALUE 
								if((BranchOpt & 2) &&
									!forcedSize && abs(LODWORD(o2.l.v)) <= 0x7f) {
									o1.S=1;
									gdop.o3.val=o2.l.v;
									o1.l.valid=o2.l.valid=FALSE;
									}
								else {
									o1.S=4;
									}
								}
							else {
								o1.S=4;
	  						}
							if(o1.S==8) {
								if(gdop.mode2==ADDR_IMMEDIATE) {
									o2.l.valid=TRUE;
									o2.l.v=HIDWORD(o1.l.v);
//									o2.l.seg=HIDWORD(o1.l.v);
									}
								}
							// GESTIRE FAR ADDRESS

  						}
						else
							gdop.doflags=useFlags;
// OCCHIO all'ordine di queste assegnazioni, perché le istruzioni speciali li toccano o sono toccate!

						if(Mnn==43) {		// JMP nn 
							if(forcedSize) {
								if(forcedSize<4) 
									PROCError(2064,forcedSize);
								else
									o1.S=forcedSize;
								}
							else
								o1.S=4;
							if(o1.S==8) {
								if(gdop.mode2==ADDR_IMMEDIATE) {
									o2.l.valid=TRUE;
									o2.l.v=HIDWORD(o1.l.v);
//									o2.l.seg=HIDWORD(o1.l.v);
									}
								}
//							if(useCondiz)		// ma in effetti in EXCEL ho messo che si accetta! e pure nel code... :)
//								PROCError(2060,T1S);
							}
							// GESTIRE FAR ADDRESS



						gdop.size=(o1.S==8 ? 3 : (o1.S>>1));

						if(Mnn==67) {		// gestisco PUSH (perché ho voluto mettere operandi in source!
							if(gdop.mode2 > ADDR_IMMEDIATE8)
								gdop.reg1=gdop.reg2;
							gdop.mode1=gdop.mode2;
							gdop.reg2=0;
							gdop.mode2=0;
							}
						if(Mnn==31) {		// gestisco DAA
							gdop.size=0;
							}
						if(Mnn==94) {		// gestisco SWAPR (già nell'opcode ma viene sovrascritto..
							gdop.reg1=1;
							}

						if(o1.addr64) {		// finire...
							if(gdop.mode2 == ADDR_REGISTER_INDIRECT)
								gdop.mode2=ADDR_REGISTER_INDIRECT64;
							if(!CPass)
								PROCWarn(2062,"64bit addressing needs to be tested");
							}
						if(o1.preOp || o1.postOp) {
							if(gdop.mode2 != ADDR_REGISTER_INDIRECT)
								PROCError(2062,T1S);
							if(o1.preOp)
								gdop.mode2=o1.preOp>0 ? ADDR_REGISTER_INDIRECT_PREINCREMENT : ADDR_REGISTER_INDIRECT_PREDECREMENT;
							else if(o1.postOp)
								gdop.mode2=o1.postOp>0 ? ADDR_REGISTER_INDIRECT_POSTINCREMENT : ADDR_REGISTER_INDIRECT_POSTDECREMENT;
							}

						if(o1.l.valid)
							PROCOut2(gdop.d,&o1.l);    // 
						else
							PROCOut2(gdop.d);    // 

						if(o2.l.valid)
							PROCOut2(o2.l);    // con segm.
						}
				  }
			  }

#endif

#if I8086
	    if(Is2) {
				if(o1.V==VQ_CONDIZ || o2.V==VQ_CONDIZ) 
          PROCError(2062,T1S);
	      if(o1.Ind && o2.Ind) {
          PROCError(2062,T2S);
          }
		    else if(o1.Ind) {
					uint8_t t;
					t=computeAddressing(o1.V,o1.T,o1.reg2reg,o1.l.v,T1S);
	  	    if(o1.V>=VQ_OP_VAR) {
	  	      if(o2.V>=VQ_OP_VAR) {                       //  mov ecc (),AL
	  					switch(o2.S) { 
								case 1:																// mov ecc (),nnn
									k=subStmt(TS,ADDR_IND16REG_IMM8,&mn);  
									if(k>=0) {
										if(t & 0x40)		// se disp8
											PROCOut((mn->a[k].n) | (t),
												LOBYTE(LOWORD(o1.l.v)));    // 
										else
											PROCOut((mn->a[k].n) | (t),
												LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		// no segm, sono disp16!
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
											if(o2.l.v > 1) {
												if(CPULevel<1)
		  										PROCError(2067,T2S);
												else
													PROCOut(LOBYTE(LOWORD(o2.l.v)));
												}
											}
										else
											PROCOut(LOBYTE(LOWORD(o2.l.v)));
										}                    
									break;
								case 2:                                      // mov ecc (),nnn
									k=subStmt(TS,ADDR_IND16REG_IMM16,&mn);  
									if(k>=0) {
										if(t & 0x40)		// se disp8
											PROCOut((mn->a[k].n) | (t),
												LOBYTE(LOWORD(o1.l.v)));
										else
											PROCOut((mn->a[k].n) | (t),
												LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		// no segm, sono disp16!
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
											if(o2.l.v > 1) {
												if(CPULevel<1)
		  										PROCError(2067,T2S);
												else
													PROCOut(LOBYTE(LOWORD(o2.l.v)));
												}
											}
										else
											PROCOut(LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
										}                    
									break;
								case 4:                                      // gestire!
										if(CPULevel<3)
											PROCError(2067,TS);
									break;
								}
              }
				    else if(o2.V==VQ_REGISTER) {                         // 
		  	      switch(o2.S) {
								case 1:
									if(t==6)			// non serve ma lo faccio per "pulizia"
										k=subStmt(TS,ADDR_IND16_REG8,&mn);  
									else
										k=subStmt(TS,ADDR_IND16REG_REG8,&mn);  
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
										// servirebbe aggiungere/gestire il tipo indirizzamento come ADDR_IND16REG_IMM8 ma ampliato... per ora faccio così in questi casi!
										if(Reg[o2.T].m != 1)
  										PROCError(2062,T2S);
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut((mn->a[k].n | 0x200) | (t),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut((mn->a[k].n | 0x200) | (t),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v))); 	// no segm, sono disp16!
											}
										}
									else {
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m) << 3)),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m) << 3)),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v))); 	// no segm, sono disp16!
											}
										}
									break;
								case 2:
									if(t==6)			// non serve ma lo faccio per "pulizia"
										k=subStmt(TS,ADDR_IND16REG_REG16,&mn);  
									else
										k=subStmt(TS,ADDR_IND16REG_REG16,&mn);  
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
										if(Reg[o2.T].m != 1)
  										PROCError(2062,T2S);
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut(((mn->a[k].n | 0x200) | (t)),
													LOBYTE(LOWORD(o1.l.v))); 
											else
												PROCOut(((mn->a[k].n | 0x200) | (t)),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));	// no segm, sono disp16!
											}
										}
									else {
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m -8) << 3)),
													LOBYTE(LOWORD(o1.l.v))); 
											else
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m -8) << 3)),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));	// no segm, sono disp16!
											}
										}
									break;
								case 4:
									if(CPULevel<3)
										PROCError(2067,TS);
									break;
				        }                    
				      }                    
			      }
				  else if(o1.V==VQ_REGISTER) {                      // mov ecc (),
	  	      if(o2.V>=VQ_OP_VAR) {                       // mov ecc (ofs),nnn
	  					switch(o2.S) { 
								case 1:																// 
									if(t!=6)			// non serve ma lo faccio per "pulizia"
										k=subStmt(TS,ADDR_IND16REG_IMM8,&mn);  
									else
										k=subStmt(TS,ADDR_IND16REG_IMM8,&mn);  
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
										if(k>=0) {
											if(o2.l.v > 1) {
												if(CPULevel<1)
			  									PROCError(2067,T2S);
												}
											if(t & 0x40)		// se disp8
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
											if(o2.l.v > 1) {
												if(CPULevel>=1)
													PROCOut(LOBYTE(LOWORD(o2.l.v)));
												}
											}
										}
									else {
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
											PROCOut(LOBYTE(LOWORD(o2.l.v)));
											}
										}
									break;
								case 2:                                      // mov ecc (ofs),nnn
									if(t!=6)			// non serve ma lo faccio per "pulizia"
										k=subStmt(TS,ADDR_IND16REG_IMM16,&mn);  
									else
										k=subStmt(TS,ADDR_IND16REG_IMM16,&mn);  
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
										if(k>=0) {
											if(o2.l.v > 1) {
												if(CPULevel<1)
		  										PROCError(2067,T2S);
												}
											if(t & 0x40)		// se disp8
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
											if(o2.l.v > 1) {
												if(CPULevel>=1)
													PROCOut(LOBYTE(LOWORD(o2.l.v)));
												}
											}                    
										}
									else {
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut((mn->a[k].n) | (t),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
											PROCOut(LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
											}                    
										}
									break;
								case 4:                                      // gestire!
										if(CPULevel<3)
											PROCError(2067,TS);
									break;
								}
				      }
				    else if(o2.V==VQ_REGISTER) {                         // mov ecc (rr + ev.ofs),a
		  	      switch(o2.S) {
								case 1:
									if(t!=6)			// non serve ma lo faccio per "pulizia"
										k=subStmt(TS,ADDR_IND16REG_REG8,&mn);  
									else
										k=subStmt(TS,ADDR_IND16REG_REG8,&mn);  
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
										if(Reg[o2.T].m != 1)
  										PROCError(2062,T2S);
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut((mn->a[k].n | 0x200) | (t),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut((mn->a[k].n | 0x200) | (t),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
											}
										}
									else {
										if(t & 0x40)		// se disp8
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m) << 3)),
												LOBYTE(LOWORD(o1.l.v)));
										else
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m) << 3)),
												LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
										}
									break;
								case 2:
									if(t!=6)			// non serve ma lo faccio per "pulizia"
										k=subStmt(TS,ADDR_IND16REG_REG16,&mn);  
									else
										k=subStmt(TS,ADDR_IND16REG_REG16,&mn);  
										if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
										if(Reg[o2.T].m != 1)
  										PROCError(2062,T2S);
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut((mn->a[k].n | 0x200) | (t),
													LOBYTE(LOWORD(o1.l.v)));
											else
												PROCOut((mn->a[k].n | 0x200) | (t),
													LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
											}
										}
									else {
										if(t & 0x40)		// se disp8
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m -8) << 3)),
												LOBYTE(LOWORD(o1.l.v)));
										else
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o2.T].m -8) << 3)),
												LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // no segm, sono disp16!
										}
									break;
								case 4:
									if(CPULevel<3)
										PROCError(2067,TS);
									break;
				        }                    
				      }
				    }
					else
	          PROCError(2062,T1S);			// safety cmq!
			    }		// Ind1
		    else if(o2.Ind) {
					uint8_t t;
					t=computeAddressing(o2.V,o2.T,o2.reg2reg,o2.l.v,T2S);
				  if(o1.V==VQ_REGISTER) {   
	  	      switch(o1.S) { 
							case 1:																// mov ecc AL,()
	  						if(o2.V>=VQ_OP_VAR) { 
									k=subStmt(TS,ADDR_REG8_IND16,&mn);  
									if(k>=0) {
										if(t & 0x40)		// se disp8
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m) << 3)),
												LOBYTE(LOWORD(o2.l.v)));
										else
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m) << 3)),
												LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));    // no segm, sono disp16!
										}
									}                    
								else if(o2.V==VQ_REGISTER) {                         // mov ecc al,(bp)
   								k=subStmt(TS,ADDR_REG8_IND16REG,&mn);
									if(k>=0) {
										if(t & 0x40)		// se disp8
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m) << 3)),
												LOBYTE(LOWORD(o2.l.v)));
										else
											PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m) << 3)),
												LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));    // no segm, sono disp16!
										}
									}
								break;
							case 2:                                      // mov ecc AX,()
	  						if(o2.V>=VQ_OP_VAR) {
									if((Mnn==196 || Mnn==197 || Mnn==199 || Mnn==201 || Mnn==203)) {			// gestisco LEA LDS ecc
										if(Mnn==201 || Mnn==203) {			// LFS LGS
											if(CPULevel<1)
  											PROCError(2067,T2S);
		 									k=subStmt(TS,ADDR_REG16_IND16,&mn);  
											if(k>=0) {
	 											PROCOut((mn->a[k].n) | (t) | ((Reg[o1.T].m -8) << 3),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
												}
											}
										else {
		 									k=subStmt(TS,ADDR_REG16_IND16,&mn);  
											if(k>=0) {
	 											PROCOut((mn->a[k].n) | (t) | ((Reg[o1.T].m -8) << 3),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
												}
											}
										}
									else {
										k=subStmt(TS,ADDR_REG16_IND16,&mn);  
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m -8) << 3)),
													LOBYTE(LOWORD(o2.l.v)));
											else
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m -8) << 3)),
													LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));    // no segm, sono disp16!
											}
										}
									}                    
								else if(o2.V==VQ_REGISTER) {                         // mov ecc ax,(di + ...)
									if((Mnn==196 || Mnn==197 || Mnn==199 || Mnn==201 || Mnn==203)) {			// gestisco LEA LDS ecc
										if(Mnn==201 || Mnn==203) {			// LFS LGS
											if(CPULevel<1)
  											PROCError(2067,T2S);
		 									k=subStmt(TS,ADDR_REG16_IND16,&mn);  
											if(k>=0) {
	 											PROCOut((mn->a[k].n) | (t) | ((Reg[o1.T].m -8) << 3),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
												}
											}
										else {
		 									k=subStmt(TS,ADDR_REG16_IND16,&mn);  
											if(k>=0) {
	 											PROCOut((mn->a[k].n) | (t) | ((Reg[o1.T].m -8) << 3),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
												}
											}
										}
									else {
	 									k=subStmt(TS,ADDR_REG16_IND16REG,&mn);
										if(k>=0) {
											if(t & 0x40)		// se disp8
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m -8) << 3)),
													LOBYTE(LOWORD(o2.l.v)));
											else
												PROCOut(((mn->a[k].n) << 8) | (t | ((Reg[o1.T].m -8) << 3)),
													LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));    // no segm, sono disp16!
											}
										}
									}
								break;
							case 4:                                      // gestire!
									if(CPULevel<3)
										PROCError(2067,TS);
								break;
				      }
				    }
				  else if(o1.V>=VQ_OP_VAR) {                    //
						
						//						 NON DOVREBBE esistere credo! verificare e togliere

	  	      switch(o1.S) { 
							case 1:																// mov ecc AL,()
	  						if(o2.V>=VQ_OP_VAR) { 
									k=subStmt(TS,ADDR_IND16REG_IMM8,&mn);  
									if(k>=0) {
										PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
										}
									}                    
								else if(o2.V==VQ_REGISTER) {                         // mov ecc al,(bp)
   								k=subStmt(TS,ADDR_IND16REG_REG8,&mn);
									if(k>=0) {
	   								PROCOut(mn->a[k].n | (Reg[o1.T].m << 3));
										}
									}
								break;
							case 2:                                      // mov ecc AX,()
	  						if(o2.V>=VQ_OP_VAR) { 
									k=subStmt(TS,ADDR_IND16REG_IMM16,&mn);  
									PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
									}                    
								else if(o2.V==VQ_REGISTER) {                         // ld a,(bc)
   								k=subStmt(TS,ADDR_IND16REG_REG16,&mn);
									if(k>=0) {
	   								PROCOut(mn->a[k].n | (Reg[o1.T].m << 3));
										}
									}
								break;
							case 4:                                      // gestire!
									if(CPULevel<3)
										PROCError(2067,TS);
								break;
				      }
				    }
					else
	          PROCError(2062,T1S);			// safety cmq!
          }
 		    else {                                       // mov ecc. aaa,bbb
				  if(o1.V==VQ_REGISTER) {
	  	      switch(o1.S) {                          // mov al,20
							case 1:
	  						if(o2.V>=VQ_OP_VAR) { 
	  							if(o2.S>1) 
	  								PROCError(2064,T2S);
									if(Mnn==228) {			// gestisco MOV
										if(Reg[o1.T].m >= 8)		// solo 8 bit
		  								PROCError(2064,T2S);
		 								k=subStmt(TS,ADDR_REG8,&mn);            // 
										if(k>=0) {
		    							PROCOut(mn->a[k].n | (Reg[o1.T].m),LOBYTE(LOWORD(o2.l.v)));		// segmento??
											}
										}
									else if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
	 									k=subStmt(TS,ADDR_REG8IMM,&mn);     // 
										if(k>=0) {
											if(o2.l.v > 1) {
												if(CPULevel<1)
		  										PROCError(2067,T2S);
												PROCOut((mn->a[k].n) | (Reg[o1.T].m & 0xf),LOBYTE(LOWORD(o2.l.v)));
												}
											else
												PROCOut((mn->a[k].n) | (Reg[o1.T].m & 0xf));
											}
										}
									else if(Mnn==176)	{		// gestisco IN 
 										k=subStmt(TS,ADDR_REG8IMM,&mn);  
										if(k>=0) {
											if(Reg[o1.T].m != 0)
	  										PROCError(2062,T2S);
 											PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)));
											}
										}
									else {
										if((o1.T == 0)) {         // solo se AL...
		 									k=subStmt(TS,ADDR_REG8,&mn);  
											if(k>=0) {
	 											PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)));
												}
											}
										else {
		 									k=subStmt(TS,ADDR_REG8IMM,&mn);  
											if(k>=0) {
	 											PROCOut((mn->a[k].n) | ((Reg[o1.T].m & 0xf)),LOBYTE(LOWORD(o2.l.v)));
												}
											}
										}
									}                    
								else if(o2.V==VQ_REGISTER) {                         // mov al,bl
	  							if(o2.S>1) 
	  								PROCError(2064,T2S);
									if(Mnn==228) {			// gestisco MOV
										k=subStmt(TS,ADDR_REG8IMM,&mn);  
										if(k>=0) {
 											PROCOut(mn->a[k].n,(0xC0 | (Reg[o2.T].m & 0xf) << 3) | Reg[o1.T].m & 0xf);
											}
										}
									else if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
	 									k=subStmt(TS,ADDR_REG8IMM,&mn);     // 
										if(k>=0) {
											if(Reg[o2.T].m != 1)
	  										PROCError(2062,T2S);
											PROCOut((mn->a[k].n) | ((Reg[o1.T].m & 0xf)));
											}
										}
									else {
										k=subStmt(TS,ADDR_REG8REG,&mn);  
										if(k>=0) {
 											PROCOut(mn->a[k].n,(0xC0 | (Reg[o2.T].m & 0xf) << 3) | Reg[o1.T].m & 0xf);
											}
										}
									}
								else
									PROCError(2062,T2S);
								break;
							case 2:
		  					if(o2.V>=VQ_OP_VAR) {                       // mov ax,20
									if(Mnn==228) {			// gestisco MOV
										if((Reg[o1.T].m >= 16 && Reg[o1.T].m <= 19) || (Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41)) {			// se registro segmento
											PROCError(2062,T2S);
											}
										else {
		 									k=subStmt(TS,ADDR_REG16,&mn);  
		 									PROCOut(mn->a[k].n | ((Reg[o1.T].m & 0xf) -8),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)),o2.l.seg);    // con segm.
											}
										}
									else if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
	 									k=subStmt(TS,ADDR_REG16IMM,&mn);  
										if(k>=0) {
											if(o2.l.v > 1) {
												if(CPULevel<1)
		  										PROCError(2067,T2S);
												PROCOut(((0xC1) | (mn->a[k].n >> 8) | (Reg[o1.T].m & 0xf -8),LOBYTE(LOWORD(o2.l.v))));// provare!
												}
											else
												PROCOut((mn->a[k].n) | ((Reg[o1.T].m & 0xf) -8));
											}
										else {
		 									k=subStmt(TS,ADDR_IMM16,&mn);  
											if(k>=0) {
	 											PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
												}
											}
										}
									else if(Mnn==176)	{		// gestisco IN 
 										k=subStmt(TS,ADDR_REG16IMM,&mn);  
										if(k>=0) {
											if(Reg[o1.T].m != 8)
	  										PROCError(2062,T2S);
 											PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)));
											}
										}
									else {
										if(o1.T == 8 ||  o1.T == 32) {        // se AX!
		 									k=subStmt(TS,ADDR_REG16,&mn);  
											if(k>=0) {
	 											PROCOut(mn->a[k].n,LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));
												}
											}
										else {
	 										k=subStmt(TS,ADDR_REG16IMM,&mn);  
											if(k>=0) {
	 											PROCOut((mn->a[k].n) | ((Reg[o1.T].m & 0xf) -8),LOBYTE(LOWORD(o2.l.v)),HIBYTE(LOWORD(o2.l.v)));    
												}
											}
										}
		  						}
								else if(o2.V==VQ_REGISTER) {                // mov ax,bx
									if(Mnn==228) {			// gestisco MOV
		  							if(o2.S != 2) 
		  								PROCError(2064,T2S);
										if(Reg[o1.T].m >= 16 && Reg[o1.T].m <= 19) {			// se registro segmento
			  							if((Reg[o2.T].m >= 16 && Reg[o2.T].m <= 19) || (Reg[o2.T].m >= 40 && Reg[o2.T].m <= 41))
			  								PROCError(2062,T2S);
 											PROCOut(0x8E,0xC0 | (((Reg[o1.T].m & 0xf) -16) << 3) | (Reg[o2.T].m & 0xf -8));
											}
										else if(Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41) {			// se registro segmento altro
													if(CPULevel<3)
														PROCError(2067,TS);
			  							if((Reg[o2.T].m >= 16 && Reg[o2.T].m <= 19) || (Reg[o2.T].m >= 40 && Reg[o2.T].m <= 41))
			  								PROCError(2062,T2S);
 											PROCOut(0x8E,(0xC0 | ((Reg[o1.T].m & 0xf) -8) << 3) | (Reg[o2.T].m & 0xf -40));
											}
										else if(Reg[o2.T].m >= 16 && Reg[o2.T].m <= 19) {			// se registro segmento
			  							if((Reg[o1.T].m >= 16 && Reg[o1.T].m <= 19) || (Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41))
			  								PROCError(2062,T2S);
 											PROCOut(0x8C,0xC0 | (((Reg[o2.T].m & 0xf) -16) << 3) | (Reg[o1.T].m & 0xf -8));
											}
										else if(Reg[o2.T].m >= 40 && Reg[o2.T].m <= 41) {			// se registro segmento altro
													if(CPULevel<3)
														PROCError(2067,TS);
			  							if((Reg[o1.T].m >= 16 && Reg[o1.T].m <= 19) || (Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41))
			  								PROCError(2062,T2S);
 											PROCOut(0x8C,(0xC0 | ((Reg[o2.T].m & 0xf) -8) << 3) | (Reg[o1.T].m & 0xf -40));
											}
										else {
 											k=subStmt(TS,ADDR_REG16IMM,&mn);  
 											PROCOut(mn->a[k].n,(0xC0 | ((Reg[o2.T].m & 0xf) -8) << 3) | (Reg[o1.T].m & 0xf -8));
											}
										}
									else if((Mnn>=357 && Mnn<=358) || (Mnn>=370 && Mnn<=371) || 
											(Mnn>=376 && Mnn<=377) || (Mnn==386) || (Mnn==388)) {			// gestisco SHR ROL ecc
	 									k=subStmt(TS,ADDR_REG16REG,&mn);     // 
										if(k>=0) {
											if(Reg[o2.T].m != 1)
	  										PROCError(2062,T2S);
											PROCOut((mn->a[k].n) | ((Reg[o1.T].m & 0xf -8)));
											}
										}
									else if(Mnn==176)	{		// gestisco IN 
										switch(o1.S) {
											case 1:
 												k=subStmt(TS,ADDR_REG8REG,&mn);  
												if(k>=0) {
													if(Reg[o1.T].m != 0 || Reg[o2.T].m != 10)
	  												PROCError(2062,T2S);
 													PROCOut(mn->a[k].n);
													}
												break;
											case 2:
 												k=subStmt(TS,ADDR_REG16REG,&mn);  
												if(k>=0) {
													if(Reg[o1.T].m != 8 || Reg[o2.T].m != 10)
	  												PROCError(2062,T2S);
 													PROCOut(mn->a[k].n);
													}
												break;
											}
										}
									else if(Mnn==274)	{		// gestisco OUT 
										switch(o2.S) {
											case 1:
 												k=subStmt(TS,ADDR_REG8REG,&mn);  
												if(k>=0) {
													if(Reg[o1.T].m != 10 || Reg[o2.T].m != 0)
	  												PROCError(2062,T2S);
 													PROCOut(mn->a[k].n);
													}
												break;
											case 2:
 												k=subStmt(TS,ADDR_REG16REG,&mn);  
												if(k>=0) {
													if(Reg[o1.T].m != 10 || Reg[o2.T].m != 8)
	  												PROCError(2062,T2S);
 													PROCOut(mn->a[k].n);
													}
												break;
											}
										}
									else {
		  							if(o2.S != 2) 
		  								PROCError(2064,T2S);
 										k=subStmt(TS,ADDR_REG16REG,&mn);  
										if(k>=0) {
 											PROCOut(mn->a[k].n,(0xC0 | ((Reg[o2.T].m & 0xf) -8) << 3) | (Reg[o1.T].m & 0xf -8));
											}
										}
									}
								else {  
	  							PROCError(2062,T2S);
									}
								break;
							case 4:		// gestire!
									if(CPULevel<3)
										PROCError(2067,TS);
								break;
				      }
				    }
				  else if(o1.V>=VQ_OP_VAR) {                    // in, out, cmp mem,imm
						if(o2.V==VQ_REGISTER) {
							switch(o2.S) {
								case 1:
									if(Mnn==228) {			// gestisco MOV
										k=subStmt(TS,ADDR_IMM8,&mn);  
										if(k>=0) {
											PROCOut(mn->a[k].n | o1.l.v << 3 | Reg[o2.T].m);
											}
										}
									else if(Mnn==274)	{		// gestisco OUT 
 										k=subStmt(TS,ADDR_REG8IMM,&mn);  
										if(k>=0) {
											if(Reg[o2.T].m != 0)
	  										PROCError(2062,T2S);
 											PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)));
											}
										}
									else {
										k=subStmt(TS,ADDR_IMM8,&mn);  
										if(k>=0) {
											PROCOut(mn->a[k].n | o1.l.v << 3 | Reg[o2.T].m);
											}
										}
									break;
								case 2:
									if(Mnn==228) {			// gestisco MOV
										k=subStmt(TS,ADDR_IMM16,&mn);  
										if(k>=0) {
											PROCOut(mn->a[k].n | o1.l.v << 3 | Reg[o2.T].m);
											}
										}
									else if(Mnn==274)	{		// gestisco OUT 
 										k=subStmt(TS,ADDR_REG16IMM,&mn);  
										if(k>=0) {
											if(Reg[o2.T].m != 8)
	  										PROCError(2062,T2S);
 											PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)));
											}
										}
									else {
										k=subStmt(TS,ADDR_IMM16,&mn);  
										if(k>=0) {
											PROCOut(mn->a[k].n | o1.l.v << 3 | Reg[o2.T].m);
											}
										}
									break;
								case 4:		// gestire!
									if(CPULevel<3)
										PROCError(2067,TS);
									break;
								}
				      }
            else {			// safety
              PROCError(2062,T1S);
				      }
            }
          }
			  }		// Is2
			else {                                     // 1 solo parm
	      if(o1.Ind) {
					uint8_t t;
					t=computeAddressing(o1.V,o1.T,o1.reg2reg,o1.l.v,T1S);
			    if(o1.V>=VQ_OP_VAR) {
  					if(Mnn==313) {                 // gestisco POP
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							if(k>=0) {
//								PROCOut(mn->a[k].n, LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		//
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==351) {                 // gestisco PUSH
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							if(k>=0) {
//								PROCOut(mn->a[k].n, LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		//
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==430) {                 // gestisco XLAT
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==74 || Mnn==177) {                 // gestisco INC,DEC
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
//								PROCOut(mn->a[k].n, LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		//((Reg[o1.T].m & 0xf) << 3)
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
	  					}
  					else if(Mnn==262 || Mnn==175) {                 // gestisco MUL,IMUL
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==75 || Mnn==174) {                 // gestisco DIV,IDIV
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
						else
			        PROCError(2062,T1S);
						}
				  else if(o1.V==VQ_REGISTER) {
  					if(Mnn==313) {                 // gestisco POP
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							if(k>=0) {
//								PROCOut(mn->a[k].n, LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		//
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==351) {                 // gestisco PUSH
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16_REG16,&mn);         // 
							if(k>=0) {
//								PROCOut(mn->a[k].n, LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		//
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==430) {                 // gestisco XLAT
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
//								PROCOut(mn->a[k].n, LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		//((Reg[o1.T].m & 0xf) << 3)
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==74 || Mnn==177) {                 // gestisco INC,DEC
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
//								PROCOut(mn->a[k].n, LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));		//((Reg[o1.T].m & 0xf) << 3)
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
	  					}
  					else if(Mnn==262 || Mnn==175) {                 // gestisco MUL,IMUL
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
  					else if(Mnn==75 || Mnn==174) {                 // gestisco DIV,IDIV
							if(t!=6)			// non serve ma lo faccio per "pulizia"
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							else
		  					k=subStmt(TS,ADDR_IND16REG_REG16,&mn);         // 
							if(k>=0) {
								if(t & 0x40)		// se disp8
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)));
								else
									PROCOut((mn->a[k].n) | (t), LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
								}
  						}
						else
			        PROCError(2062,T1S);
			      }
          else {			// safety
            PROCError(2062,T1S);
			      }
					}
			  else {
			    if(o1.V>=VQ_OP_VAR) {
  			    switch(o1.S) {
							case 1:
	//   fprintf(stderr,"\tCurAddr: %x, l=%lx\n",CurAddr,l);
 								if(Mnn==1) {                              // gestisco AAD*
 									if(o1.l.v<0 || o1.l.v>2)
 										PROCError(2062,T1S);
	  							k=subStmt(TS,ADDR_IMM8,&mn);         // 
									if(k>=0) {
	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)));
										}
  								}
 								else if(Mnn==25) {                           // gestisco CALL 
									int16_t l;
	  							k=subStmt(TS,ADDR_IMM8,&mn);         // 
  								if(k>=0) {
								    l=(int16_t)(o1.l.v-Seg[CurSeg-1].CurAddr-3);		// verificare 3
 	  								PROCOut(mn->a[k].n,LOBYTE(l),HIBYTE(l));    // 
										}
	  							}
  							else if(Mnn==189) {                 // gestisco Jcc
	  							k=subStmt(TS,ADDR_IMM8,&mn);         // 
									if(k>=0) {
										subBranch(TS,T2S,mn,FNIsCond(TS+1)-1,k,o1.l,TRUE);
										}
  								}
  							else if(Mnn==191) {                           // gestisco JMP
// 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
	  							k=subStmt(TS,ADDR_IMM8,&mn);         // 
									if(k>=0) {
										subBranch(TS,T2S,mn,0,k,o1.l,FALSE);
										}
	  							}
  							else if(Mnn==212) {                           // gestisco LOOP
									if(FNIsCond(TS+1)) {
										if(CPULevel<3)
											PROCError(2067,TS);
										else {
		  								k=subStmt(TS,ADDR_IMM8,&mn);         // 
											subBranch(TS,T2S,mn,0,k,o1.l,FALSE);
											}
										}
									else {
	  								k=subStmt(TS,ADDR_IMM8,&mn);         // 
										if(k>=0) {
											subBranch(TS,T2S,mn,0,k,o1.l,FALSE);
											}
										}
	  							}
  							else if(Mnn==74 || Mnn==177) {                 // gestisco INC,DEC
			            PROCError(2064,T1S);
	  							}
  							else if(Mnn==262 || Mnn==175) {                 // gestisco MUL,IMUL
			            PROCError(2064,T1S);
  								}
  							else if(Mnn==75 || Mnn==174) {                 // gestisco DIV,IDIV
			            PROCError(2064,T1S);
  								}
  							else if(Mnn==351) {                 // gestisco PUSH
									if(CPULevel<3)			// sicuro?? 386
										PROCError(2067,TS);
	  							k=subStmt(TS,ADDR_IMM8,&mn);         // 
									if(k>=0) {
	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)));
										}
  								}
  							else if(Mnn==430) {                 // gestisco XLAT
			            PROCError(2064,T1S);
  								}
								else {
	  							k=subStmt(TS,ADDR_IMM8,&mn);         // 
	  							PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),-1,0);
									}
  							break;
							case 2:
  							if(Mnn==189) {                 // gestisco Jcc
	  							k=subStmt(TS,ADDR_IMM16,&mn);         // jcc nn
									if(k>=0) {
										subBranch(TS,T2S,mn,FNIsCond(TS+1)-1,k,o1.l,TRUE);
										}
  								}
  							else if(Mnn==191 || Mnn==25) {                           // gestisco JMP & CALL 
									int16_t l;
							    l=(int16_t)(o1.l.v-Seg[CurSeg-1].CurAddr-3);		// verificare 3
	  							k=subStmt(TS,ADDR_IMM16,&mn);         // call nn
									if(k>=0) {
// 										PROCOut(mn->a[k].n,LOBYTE(l),HIBYTE(l));    // 
										subBranch(TS,T2S,mn,0,k,o1.l,FALSE);
										}
	  							}
  							else if(Mnn==74 || Mnn==177) {                 // gestisco INC,DEC
			            PROCError(2064,T1S);
	  							}
  							else if(Mnn==262 || Mnn==175) {                 // gestisco MUL,IMUL
			            PROCError(2064,T1S);
  								}
  							else if(Mnn==75 || Mnn==174) {                 // gestisco DIV,IDIV
			            PROCError(2064,T1S);
  								}
  							else if(Mnn==351) {                 // gestisco PUSH
									if(CPULevel<3)			// sicuro?? 386
										PROCError(2067,TS);
	  							k=subStmt(TS,ADDR_IMM16,&mn);         // 
									if(k>=0) {
	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));
										}
  								}
  							else if(Mnn==430) {                 // gestisco XLAT
			            PROCError(2064,T1S);
  								}
  							else if(Mnn==212) {                           // gestisco LOOP
									if(FNIsCond(TS+1)) {
										if(CPULevel<3)
											PROCError(2067,TS);
										else {
	  									k=subStmt(TS,ADDR_IMM16,&mn);         // 
											subBranch(TS,T2S,mn,0,k,o1.l,FALSE);
											}
										}
									else {
	  								k=subStmt(TS,ADDR_IMM16,&mn);         // 
										subBranch(TS,T2S,mn,0,k,o1.l,FALSE);
										}
  								}
								else {
//  									PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)),o1.l.seg);    // con segm.
	  							k=subStmt(TS,ADDR_IMM16,&mn);         // 
									subBranch(TS,T2S,mn,0,k,o1.l,FALSE);
	  							}
								break;
							case 4:
  							k=subStmt(TS,ADDR_IMM32,&mn);         // jmp far nn
  							if(k>=0) {
	  							if(Mnn==191 || Mnn==25) {                 // gestisco JMP & CALL
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // 
  									PROCOut(LOBYTE(HIWORD(o1.l.v)),HIBYTE(HIWORD(o1.l.v)),-1,o1.l.seg);    // con segm.
  									}
									else {		// c'è altro a 32bit? o solo 386?
										if(CPULevel>=3)
											;
 	  								PROCOut(mn->a[k].n,LOBYTE(LOWORD(o1.l.v)),HIBYTE(LOWORD(o1.l.v)));    // 
  									PROCOut(LOBYTE(HIWORD(o1.l.v)),HIBYTE(HIWORD(o1.l.v)),-1,o1.l.seg);    // con segm.
										}
	  							}
								break;
  			      }
				    }
				  else if(o1.V==VQ_REGISTER) {
				    switch(o1.S) {
							case 1:
  							if(Mnn==74 || Mnn==177) {                 // gestisco INC,DEC
	  							k=subStmt(TS,ADDR_REG8IMM,&mn);         // 
									if(k>=0) {
  									PROCOut(mn->a[k].n | (Reg[o1.T].m));
										}
	  							}
  							else if(Mnn==262 || Mnn==175) {                 // gestisco MUL,IMUL
	  							k=subStmt(TS,ADDR_REG8IMM,&mn);         // 
									if(k>=0) {
										PROCOut((mn->a[k].n) | (Reg[o1.T].m));
										}
  								}
  							else if(Mnn==75 || Mnn==174) {                 // gestisco DIV,IDIV
	  							k=subStmt(TS,ADDR_REG8IMM,&mn);         // 
									if(k>=0) {
										PROCOut((mn->a[k].n) | (Reg[o1.T].m));
										}
  								}
  							else if(Mnn==313) {                 // gestisco POP
	  							k=subStmt(TS,ADDR_REG8IMM,&mn);         // 
									if(k>=0) {
  									PROCOut(mn->a[k].n | (Reg[o1.T].m));
										}
  								}
								else {
									if(Reg[o1.T].m<8) {
	  								k=subStmt(TS,ADDR_REG8,&mn);         // or B
	  								if(k>=0) {
	  									PROCOut(mn->a[k].n | Reg[o1.T].m);
		  								}
										}
									else {
										PROCError(2062,T1S);
										}
									}
								break;
							case 2:
  							if(Mnn==74 || Mnn==177) {                 // gestisco INC,DEC
									if((Reg[o1.T].m >= 16 && Reg[o1.T].m <= 19) || (Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41)) 			// se registro segmento
										PROCError(2062,T1S);
	  							k=subStmt(TS,ADDR_REG16IMM,&mn);         // 
									if(k>=0) {
  									PROCOut(mn->a[k].n | (Reg[o1.T].m -8));
										}
	  							}
  							else if(Mnn==262 || Mnn==175) {                 // gestisco MUL,IMUL
	  							k=subStmt(TS,ADDR_REG16IMM,&mn);         // 
									if(k>=0) {
										PROCOut((mn->a[k].n) | (Reg[o1.T].m -8));
										}
  								}
  							else if(Mnn==75 || Mnn==174) {                 // gestisco DIV,IDIV
	  							k=subStmt(TS,ADDR_REG16IMM,&mn);         // 
									if(k>=0) {
										PROCOut((mn->a[k].n) | (Reg[o1.T].m -8));
										}
  								}
  							else if(Mnn==313) {                 // gestisco POP
									if((Reg[o1.T].m >= 16 && Reg[o1.T].m <= 19) || (Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41)) {			// se registro segmento
										if(Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41) {			// se registro segmento altro
											if(CPULevel<3)
												PROCError(2067,TS);
											else {
	  										k=subStmt(TS,ADDR_REG16,&mn);         // 
												if(k>=0) {
													PROCOut(0x0FA0 | (Reg[o1.T].m == 41 ? 8 : 0));
													}
												}
											}
										else {
	  									k=subStmt(TS,ADDR_REG16,&mn);         // 
											if(k>=0) {
  											PROCOut(mn->a[k].n | ((Reg[o1.T].m -16) << 3));
												}
											}
										}
									else {
	  								k=subStmt(TS,ADDR_REG16IMM,&mn);         // 
										if(k>=0) {
  										PROCOut(mn->a[k].n | (Reg[o1.T].m -8));
											}
										}
									}
  							else if(Mnn==351) {                 // gestisco PUSH
									if((Reg[o1.T].m >= 16 && Reg[o1.T].m <= 19) || (Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41)) {			// se registro segmento
										if(Reg[o1.T].m >= 40 && Reg[o1.T].m <= 41) {			// se registro segmento altro
											if(CPULevel<3)
												PROCError(2067,TS);
											else {
	  										k=subStmt(TS,ADDR_REG16,&mn);         // 
												if(k>=0) {
													PROCOut(0x0fa0 | (Reg[o1.T].m == 41 ? 8 : 0));
													}
												}
											}
										else {
	  									k=subStmt(TS,ADDR_REG16,&mn);         // 
											if(k>=0) {
  											PROCOut(mn->a[k].n | ((Reg[o1.T].m -16) << 3));
												}
											}
										}
									else {
	  								k=subStmt(TS,ADDR_REG16IMM,&mn);         // 
										if(k>=0) {
  										PROCOut(mn->a[k].n | (Reg[o1.T].m -8));
											}
										}
  								}
								else {
   								k=subStmt(TS,ADDR_REG16,&mn);
  								if(k>=0) 
	 									PROCOut(LOBYTE(mn->a[k].n), HIBYTE(mn->a[k].n) |   (Reg[o1.T].m  -8));
									}
								break;
							case 4:
								if(CPULevel<3)
									PROCError(2067,TS);
								break;
	 			      }
				    }
          else {			// safety
            PROCError(2062,T1S);
			      }
				  }
			  }

#endif

      }
//  	while(*TEXTP++ != '\n');

    return TRUE;
    }
  else {

		memset(&o1,0,sizeof(struct OPERAND));
		memset(&o2,0,sizeof(struct OPERAND));

#if I8086
		if(toupper(TS[1])=='S' && (TS[2]==0 || (TS[2]==':' && TS[3]==0))) {			// gestisco segment override "sciolto" così! con/senza : v. DEBUG e onlineassembler...
			i=FNIsReg(TS);
			if(i && i & 0x100) {			// reg 16 bit
				i--;
				i &= 0xff;
				if(Reg[i].m>=16 && Reg[i].m<=19) {
					if((Seg[CurSeg-1].type  & 0xff00) & (SEGMENT_DATA & 0xff00))
						PROCError(2102);
					if(OutList)
						PROCOut1("> ",theLine,FALSE);
					PROCOut(0x26 | ((i & 3) << 3), -1);
					goto foundSegm;
					}
				else if(Reg[i].m>=40 && Reg[i].m<=41) {
					if(CPULevel>=3) {
						if((Seg[CurSeg-1].type  & 0xff00) & (SEGMENT_DATA & 0xff00))
							PROCError(2102);
						if(OutList)
							PROCOut1("> ",theLine,FALSE);
						PROCOut(0x64 | (i & 1), -1);
						}
					else
						PROCError(2067,TS);
					goto foundSegm;
					}
				}
//			else
//				PROCError(2061,TS);
      }
#endif

		struct LINE_DEF *m;
		if(m=FNMacro(TS)) {		// mettere controllo su se stessa per ricorsiva :)
			char *p;
//			fprintf(stderr,"trovata macro %s",m->s);
			strcpy(theMacro,m->s+strlen(m->s)+1);		// spostare in translate...
			p=m->s;
			while(isspace(*p))
				p++;
			while(iscsym /*isasym*/ (*p))		// salto "nome" macro
				p++;

	    if(OutList) {                         
	      PROCOut1(">= ",m->s);
	      }
			m->used++;
			TranslateMacro(m,p,theMacro,&theLine[theLinePtr]);

			inMacroExecuting=TRUE;
			theMacroPtr=0;
			return TRUE;
			}

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
	      if(MCom[i].m >= 0x8010 && MCom[i].m <= 0x801f)
	        goto soloIFS801;
	      else if(MCom[i].m >= 0x8020 && MCom[i].m <= 0x802f)
	        goto soloIFS802;
	      else
	        return TRUE;
	      }
		  if(OutList) {    
		    PROCOut1(MCom[i].m & 0xffe0 ? "= " : "- ",theLine,FALSE);
		    }
			n=0;
      switch(MCom[i].m >> 4) {
        case 0:
	        FNLA(myBuf);
		      switch(MCom[i].m & 0xf) {
						case 0:			// LABEL
			        FNLO(myBuf);
							if(!stricmp(myBuf,"BYTE")) {	// boh non trovo la descrizione in pdf, v.GLABios cmq
								n=1;
								}
							else if(!stricmp(myBuf,"WORD")) {
								n=2;
								}
							else {
								n=4;
								}

							break;
						case 1:			// DB ecc
						case 2:
						case 4:
						case 8:			// DQ
do_db:
							n=0;
							do {                  
								long l;
			//fprintf(stderr,"FNLA: %s\n",myBuf);
								if(*myBuf == ',')
									PROCCheck(',');
								if(!stricmp(FNLA(myBuf1),"DUP")) {

									FNLO(myBuf);
									PROCCheck('(');
									FNLA(myBuf);

									if(inStructDeclaration) {
										l=theLinePtr;
										FNLO(myBuf);
										if(!*myBuf || *myBuf==')')
											PROCError(2060,myBuf);
										theLinePtr=l;

										o2.l=FNEvalExpr(14,myBuf);
										if(!o2.l.valid)
											PROCError(2062);
										PROCCheck(')');
										}
									else {
										if(*myBuf && *myBuf != '?' && 
											(Seg[CurSeg-1].type & 0xff00) == (SEGMENT_BSS & 0xff00) && !CPass)
											PROCWarn(2103,"");
										l=theLinePtr;
										FNLO(myBuf);
										if(!*myBuf || *myBuf==')')
											PROCError(2060,myBuf);
										theLinePtr=l;

										o2.l=FNEvalExpr(14,myBuf);
										if(!o2.l.valid)
											PROCError(2062);
										PROCCheck(')');
			//	fprintf(stderr,"EVALEXPR: %ld %s (l=%d)\n",l2,myBuf,l);  // mi sembra strano "l"... boh?
										n+=LODWORD(o1.l.v);
	//  								if(o2.l.v==-1   /*USARE? (!o2.l.valid)*/)                           // se è dup(?) avanzo solo il ptr
  									if(!o2.l.valid)                           // se è dup(?) avanzo solo il ptr
  										Seg[CurSeg-1].CurAddr += (LODWORD(o1.l.v)*(MCom[i].m & 0xf));
  									else { 
											while(o1.l.v--)
		  									n+=subBytes(myBuf,MCom[i].m,o2.l.v);
											}
										}
									}
								else {  

									FNLA(myBuf);

									if(inStructDeclaration) {
											if(*myBuf) {
												FNLO(myBuf);
												}
											else {
												FNLO(myBuf);
												}
											n= (MCom[i].m & 0xf);
  										if(stricmp(FNLA(myBuf1),"DUP")) {
												}
											if(!CPass) {
												LTag->size=n;		// OCCHIO se consentiamo membri anonimi (spero di no!)
												if(((struct TAGS*)currVarTag)->type & VAR_UNION) {
													LTag->pos=0;
													((struct TAGS*)currVarTag)->size=max(((struct TAGS*)currVarTag)->size,n);
													}
												else {
													LTag->pos=((struct TAGS*)currVarTag)->size;
													((struct TAGS*)currVarTag)->size+=n;
													}
												}
											FNLO(myBuf);

											}
										else {
											if(*myBuf) {
												char myBuf2[256];
												long l;
												l=theLinePtr;
												FNLO(myBuf);
												FNLA(myBuf2);
												if((Seg[CurSeg-1].type & 0xff00) == (SEGMENT_BSS & 0xff00) && !CPass) {
													if(!stricmp(myBuf2,"DUP")) {
														FNLO(myBuf2);
														PROCCheck('(');
														FNLO(myBuf2);
		//												if(*myBuf2 != '?')
		//													PROCWarn(2103,"");
														}
													else {
														if(*myBuf && *myBuf != '?')
															PROCWarn(2103,"");
														}
													}
												theLinePtr=l;
												}
											if(*myBuf) {
												long l;
												o1.l=FNEvalExpr(14,myBuf);
												if(!o1.l.valid)
													PROCError(2062);
												FNLA(TS);
		//										if(*myBuf != '?')
		//											PROCError(2060,myBuf);
												if(*TS) {
													if(*TS == ',') {
														l=theLinePtr;
														FNLO(TS);
														FNLA(TS);
														if(!*TS) {
															if(!CPass)
																PROCWarn(1006,",");			// se virgola a fine riga...
															}
														else
															theLinePtr=l;
														}
													}
												}
											else {
												FNLO(myBuf);
												o1.l.v=0xffffffffffffffff;

												o1.l.valid=TRUE;		// cmq :)


												o1.l.seg=SEG_NONE;		// VERIFICARE



		//										isEOL=TRUE;
												}
  										if(stricmp(FNLA(myBuf1),"DUP")) {
				//	fprintf(stderr,"EVALEXPR 2: %ld %s\n",l,myBuf);
do_dbs:

													n+=subBytes(myBuf,MCom[i].m,o1.l.v);
												}
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
							// VERIFICARE con PIC16!
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
							{
							struct SEGMENTED_VALUE j;
							o1.l=FNEvalExpr(14,myBuf);
							if(!o1.l.valid)
								PROCError(2062);
							FNLA(myBuf);
							j.v=0;
							if(*myBuf == ',') {
								PROCCheck(',');
								j=FNEvalExpr(14,myBuf);
								if(!j.valid)
									PROCError(2062);
								}
			        while(o1.l.v--)
						    PROCOut(LOBYTE(j.v));
							n=LODWORD(o1.l.v);
							}
							break;
						}
					if(VarDeclared)
						LVars->size=n;
		      break;

	      case 2:			// align org block
//			        *myBuf=0;
	        o1.l=FNEvalExpr(14,myBuf);
					if(!o1.l.valid)
						PROCError(2062);
					if(o1.l.seg==SEG_NONE || o1.l.seg==SEG_UNDEFINED)		// EvalExpr non ritorna il segmento... direi giusto, quindi qua lo metto
						o1.l.seg=CurSeg;
		      switch(MCom[i].m & 0xf) {
// nella prima passata si somma l'ofs a CurAddr; nella seconda ci pensa ProcOut!
		        case 0:
//			        PROCOut(-1);
							if(CreaHex) {
								if(CurSeg==SEG_UNDEFINED)
									if(!CPass)
										PROCWarn(2101,TS);
								if(o1.l.seg != CurSeg)
									CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
								}
							else {
								if(CurSeg==SEG_UNDEFINED)
		              PROCError(2101,TS);
		            }
			        o2.l.v=Seg[CurSeg-1].CurAddr;
			        if(!(MCom[i].m & 0xf)) {
			          if(o1.l.v<0)                                  // org e block
			            PROCError(2062,T1S);

//forse?	 direi ok senza.							if(!CreaHex && !CreaBin) {		// 2024, verificare se è ok, e se potrebbe andare anche in linker-mode (per evitare di buttare fuori 0 inutili
//								if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
//									Seg[CurSeg-1].BaseAddr=LOWORD(l);
//								else
//									Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LOWORD(l));		// mah, sì

  		          if(CPass) {
  		            if(o1.l.v>o2.l.v) {
 										o1.l.v-=o2.l.v;


/*										if(!CreaHex && !CreaBin) {		// 2024, verificare se è ok, e se potrebbe andare anche in linker-mode (per evitare di buttare fuori 0 inutili
  										while(o1.l.v--)
  											PROCOut(0);
	  		              }
										else
										  Seg[CurSeg-1].CurAddr+=LOWORD(o1.l.v);*/

// ago 24: mi pare SBAGLIATO (GD24032)! in bin non segue gli indirizzi ORG ecc... proviamo così
										if(CreaHex || CreaBin) {		// 
											if(Seg[CurSeg-1].type != SEGMENT_VIRT) {
  											while(o1.l.v--)
  												PROCOut(0);
												}
											else
												Seg[CurSeg-1].CurAddr+=LOWORD(o1.l.v);
	  		              }
										else
											Seg[CurSeg-1].CurAddr+=LOWORD(o1.l.v);
  		              }
  		            else { 
    			          Seg[CurSeg-1].CurAddr=LOWORD(o1.l.v);
										if(Seg[CurSeg-1].type != SEGMENT_VIRT)
								      subOut(OUT_REPOS,(int32_t)(o1.l.v-o2.l.v));
				            }
				          }
				        else
  			          Seg[CurSeg-1].CurAddr=LODWORD(o1.l.v);
								if(Seg[CurSeg-1].type == SEGMENT_VIRT) {		// questi hanno la base indicata...
									if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
										Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
									else
										Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LODWORD(o1.l.v) | Seg[CurSeg-1].BaseAddr);		// 
  			          Seg[CurSeg-1].CurAddr=LODWORD(o1.l.v) | Seg[CurSeg-1].BaseAddr;		// 
									}
								else {
									if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
										Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
									else
										Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LODWORD(o1.l.v));		// 
									}
				        }
//printf("curaddr=%d\n",Seg[CurSeg-1].CurAddr);
				      break;
		        case 1:                                         // align
			        o2.l.v=Seg[CurSeg-1].CurAddr;
			        o3.l.v=o2.l.v-(o2.l.v % o1.l.v);
              if(o2.l.v % o1.l.v)
								o3.l.v += o1.l.v;
//		        printf("l vale: %lx; l2 vale: %lx, l3 vale %lx e addr %lx\n",l,l2,l3,Seg[CurSeg-1].CurAddr);
//		        getchar();
		          if(CPass) {
		            o2.l.v=o3.l.v-o2.l.v;
		            while(o2.l.v--)
		              PROCOut(0);
			          }
			        else
                Seg[CurSeg-1].CurAddr=LODWORD(o3.l.v);
		          break;
		        case 2:     // block
			        o2.l.v=Seg[CurSeg-1].CurAddr;

							if(debug)
								printf("l vale: %lx; l2 vale: %lx\n",o1.l.v,o2.l.v);

			        if(CPass) {
  		          while(o1.l.v--)
  		            PROCOut(0);
				        }
				      else
   							Seg[CurSeg-1].CurAddr+=LODWORD(o1.l.v);     // block
		          break;
		        }  
		      break;

	      case 0x10:                      // no EQU senza label
          PROCError(2060,T1S);
          break;

		    case 0x100:
					FNLA(myBuf);
		      switch(MCom[i].m & 0xf) {
		        case 0:					// cseg/segment
		          CurSeg=DefCSeg;
							if(CreaHex || CreaBin) {
								if(!CPass)
									PROCWarn(1006,TS /*"CSEG"*/);		// però poi do warning di emissione dati :D vabbe'... decidersi!
								}

							if(!stricmp(myBuf,"ENDS")) {		// bah compatibilità con tutto :)
								break;
								}

//#if I8086 mah lascio cmq
							if(!stricmp(TS,"SEGMENT")) {		// beh migliorare...
								if(!parm)
									PROCError(1006,TS);
								if(CurSeg != SEG_UNDEFINED)
									PROCError(2108,parm);
								i=0;
//								if(CurSeg<0) {
								CurSeg=PROCDclSeg(parm,SEGMENT_CODE);
								i=1;
//									}
								}
							else {
								i=0;
//								if(CurSeg<0) {
								CurSeg=DefCSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
								i=1;
//									}
								}

							if(!myBuf[0])		// 
								;
							else if(!stricmp(TS,"SEGMENT"))	 {		// idem
rifoseg:
								FNLO(myBuf);
								if(!stricmp(myBuf,"AT")) {		// gestire poi anche altri parm: ALIGN ecc https://learn.microsoft.com/en-us/cpp/assembler/masm/segment?view=msvc-170
//An AT segment cannot contain any code or initialized data, but is useful for defining structures or
// variables that correspond to specific far memory locations, such as a screen buffer or low memory
									Seg[CurSeg-1].type=SEGMENT_VIRT;
									o1.l=FNEvalExpr(14,myBuf); 
									if(!o1.l.valid)
										PROCError(2062,myBuf);
#if I8086
									if(CPULevel<3)
										o1.l.v <<= 16;		// il seg: nella word alta
									else
										;		// finire!
#endif
									goto cseg_addr;
									}
										// https://learn.microsoft.com/en-us/cpp/assembler/masm/segment?view=msvc-170
								else if(!stricmp(myBuf,"ALIGN")) {		

									PROCError(1007,myBuf);
									goto rifoseg;
									}
								else if(!stricmp(myBuf,"STACK")) {		

									PROCError(1007,myBuf);
									goto rifoseg;
									}
								else if(!stricmp(myBuf,"COMMON")) {		

									PROCError(1007,myBuf);
									goto rifoseg;
									}
								else if(!stricmp(myBuf,"MEMORY")) {		

									PROCError(1007,myBuf);
									goto rifoseg;
									}
								else if(!stricmp(myBuf,"PRIVATE")) {		

									PROCError(1007,myBuf);
									goto rifoseg;
									}
								else if(!stricmp(myBuf,"PUBLIC")) {		

									PROCError(1007,myBuf);
									goto rifoseg;
									}
								else if(!stricmp(myBuf,"READONLY")) {		

									PROCError(1007,myBuf);
									goto rifoseg;
									}
								else if(*myBuf) {
									PROCError(2060,myBuf);
									}
								}
							else {		// SPOSTO base ma NON CREO byte!
								o1.l=FNEvalExpr(14,myBuf); 
								if(!o1.l.valid)
									PROCError(2062,myBuf);

cseg_addr:
								if(CreaHex || CreaBin) {
									o2.l.v=Seg[CurSeg-1].CurAddr;
  								if(CPass) {
  									if(o1.l.v>o2.l.v) {			// usare i LOWORD?! 16/3/25
 											o1.l.v-=o2.l.v;
											Seg[CurSeg-1].CurAddr+=LOWORD(o1.l.v);
  										}
  									else { 
    									Seg[CurSeg-1].CurAddr=LOWORD(o1.l.v);
											if(Seg[CurSeg-1].type != SEGMENT_VIRT)
										    subOut(OUT_REPOS,(int32_t)(o1.l.v-o2.l.v));
											}
										}
						      else
  					        Seg[CurSeg-1].CurAddr=LODWORD(o1.l.v);
									if(Seg[CurSeg-1].type == SEGMENT_VIRT) {		// questi hanno la base indicata...
										if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
											Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
										else
											Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LODWORD(o1.l.v) | Seg[CurSeg-1].BaseAddr);		// 
										}
									else {
										if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
											Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
										else
											Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LODWORD(o1.l.v));		// mah, sì
										}
									break;
									}
								else {
//						        l2=Seg[CurSeg-1].CurAddr;
									Seg[CurSeg-1].CurAddr=LODWORD(o1.l.v);		// forse... o addr iniziale del segmento? boh, poco importa (C) :D
									if(Seg[CurSeg-1].type == SEGMENT_VIRT) {		// questi hanno la base indicata...
										if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
											Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
										else
											Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LODWORD(o1.l.v) | Seg[CurSeg-1].BaseAddr);		// 
										}
									else {
										if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
											Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
										else
											Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LODWORD(o1.l.v));		// mah, sì
										}
									}
								}
							if(i)
								PROCOut(OUT_SEGM);
		          break;
		        case 1:				// dseg
		          CurSeg=DefDSeg;
							if(CreaHex || CreaBin) {
								if(!CPass)
									PROCWarn(1006,"DSEG");		// però poi do warning di emissione dati :D vabbe'... decidersi!
								}

							if(!stricmp(myBuf,"ENDS")) {		// bah compatibilità con tutto :)
								break;
								}

							i=0;
							if(CurSeg<0) {
								CurSeg=DefDSeg=PROCDclSeg(defaultDataSegm,SEGMENT_DATA);
								i=1;
								}
							if(!myBuf[0])		// 
								;
//#if I8086
							else if(!stricmp(myBuf,"SEGMENT"))		// bah sì; in effetti non ha senso qua
								FNLO(myBuf);
							else {
								o1.l=FNEvalExpr(14,myBuf); 
								if(!o1.l.valid)
					        PROCError(2062,T1S);
								if(CreaHex || CreaBin) {
									o2.l.v=Seg[CurSeg-1].CurAddr;
  								if(CPass) {
  									if(o1.l.v>o2.l.v) {
 											o1.l.v-=o2.l.v;
											Seg[CurSeg-1].CurAddr+=LODWORD(o1.l.v);
  										}
  									else { 
    									Seg[CurSeg-1].CurAddr=LODWORD(o1.l.v);
									    subOut(OUT_REPOS,(int32_t)(o1.l.v-o2.l.v));
											}
										}
						      else
  					        Seg[CurSeg-1].CurAddr=LODWORD(o1.l.v);
									break;
									}
								else {
//						        l2=Seg[CurSeg-1].CurAddr;
									Seg[CurSeg-1].CurAddr=LODWORD(o1.l.v);		// forse... o addr iniziale del segmento? boh, poco importa (C) :D
									if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
										Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
									else
										Seg[CurSeg-1].BaseAddr=min(Seg[CurSeg-1].BaseAddr,LODWORD(o1.l.v));		// mah, sì
									}
								}
							if(i)
								PROCOut(OUT_SEGM);
		          break;
		        case 2:				// csect
			        FNObbLO(myBuf);
							if(CreaHex) {
								if(!CPass)
									PROCWarn(1006,"CSECT");
								}
							else {
								CurSeg=PROCDclSeg(myBuf,SEGMENT_CODE);
								PROCOut(OUT_SEGM);
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
								PROCOut(OUT_SEGM);
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
			          PROCOut(OUT_SEGM);
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
			          PROCOut(OUT_SEGM);
								}
			        break;
		        case 6:				// isr/irq
							if(*myBuf) {
								o1.l=FNEvalExpr(14,myBuf); 
								if(!o1.l.valid)
				          PROCError(2062,T1S);
								}
							else
#if Z80
								o1.l.v=0x38;		// bah vabbe'
#elif _6502
								o1.l.v=0xfffa;		// bah vabbe'
#elif I8051
#elif ST62
#elif PIC==16
								o1.l.v=0x4;
#elif PIC==18
								o1.l.v=0x8;
#elif GD24032
								o1.l.v=0x0300;

#elif I8086
								o1.l.v=0x0000;

#endif  
							if(CreaHex) {		// qua? no, fare
								if(!CPass)
									PROCWarn(1006,"ISR");
								}
							else {
								CurSeg=PROCDclSeg("_isr",SEGMENT_CODE);
								Seg[CurSeg-1].BaseAddr=LODWORD(o1.l.v);
								}
							if(CreaHex) {		// qua? no, fare
								}
							else {
			          PROCOut(OUT_SEGM);
								}
			        break;
		        case 7:				// Eseg
							if(*myBuf)
								FNLO(myBuf);
							else
								strcpy(myBuf,"EEPROM_DATA");
							if(CreaHex) {		// qua?
								if(!CPass)
									PROCWarn(1006,"EEPROM");
								if(CurSeg<0)
					        CurSeg=PROCDclSeg("EEPROM_DATA",SEGMENT_DATA);		// gestire... read-only?!
								}
							else {
				        CurSeg=PROCDclSeg(myBuf,SEGMENT_DATA);		// gestire... read-only?!
			          PROCOut(OUT_SEGM);
								}
			        break;
						}
					break;

		    case 0x101:
		      switch(MCom[i].m & 0xf) {
			      case 0:			// PROC ENTRY   OCCHIO non passa mai di qua! v. sopra, verificare cmq 2025
			        if(OutList)
			          PROCOut1("=",NULL,FALSE);  // si legge meglio...
			        FNLO(myBuf);
		          if(CurProc)
		            PROCError(2100,myBuf);
			        FNLO(myBuf);
							struct SEGMENTED_VALUE l;
#if I8086
							if(!stricmp(myBuf,"FAR"))
								l.seg=CurSeg;			// 
							else if(!stricmp(myBuf,"NEAR"))
								l.seg=CurSeg /*SEG_NONE*/;		// boh verificare
							else if(*myBuf)
								PROCError(1006,myBuf);
#else
							l.seg=CurSeg/*2025*/;
#endif
							l.v=-1;
		          CurProc=PROCDclVar(myBuf,l,VAR_FUNZ,0);
#if I8086
							if(!stricmp(myBuf,"FAR")) 
								LVars->size += 2;		// da 16 a 32 bit cmq :)
#endif
			        break;
			      case 1:			// ENDP
							if(!CurProc)
								PROCError(2108);
			        FNLO(myBuf);
				      if(IgnoreCase)
				        strupr(myBuf);
//			          printf("cproc: %Fs\n",CurProc->label);  
							if(*myBuf) {
								if(!strcmp(myBuf,CurProc->label)) {
									CurProc->dim=Seg[CurSeg-1].CurAddr-CurProc->value;
									CurProc=0;
									}          
								else 
									PROCError(2108,myBuf);
								}
							else {
								CurProc->dim=Seg[CurSeg-1].CurAddr-CurProc->value;
								CurProc=0;
								}
//			        PROCError(2060,T1S);
			        break;
			      case 2:                      // end forza EOF!
							if(CurProc)
			          CurProc->dim=Seg[CurSeg-1].CurAddr-CurProc->value;
			        CurProc=0;                 // vale anche come endp...
			        fseek(FIn,-1,SEEK_END);
			        break;
						}
					break;

		    case 0x102:
		      switch(MCom[i].m & 0xf) {
			      case 0:                        // public
							if(CreaHex) 
								if(!CPass)
									PROCWarn(1006,"Public");		// ovvio! idem ma diciamo che può rimanere, non come extrn
			        do {
				        FNObbLO(myBuf);
			          if(o1.var=FNCercaVar(myBuf,0,TRUE,CPass)) {
			            o1.var->type |= VAR_PUBLIC | VAR_PUBLIC_DEFINED;
			            }
			          else {  
									struct SEGMENTED_VALUE l;
									l.seg=SEG_NONE;
									l.v=0;
				          PROCDclVar(myBuf,l,VAR_PUBLIC,0);
				          }
				        } while(*FNLO(myBuf) == ',');
			        break;
			      case 1:                         // extern extrn
							if(CreaHex) 
									PROCError(1006,"Extern");		// ovvio!
			        do {
				        FNObbLO(myBuf);
				        if(!CPass) {
									struct SEGMENTED_VALUE l;
									l.seg=SEG_EXTERN;
									l.v=0;
				          PROCDclVar(myBuf,l,VAR_EXTERN,0);
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

#if GD24032
		    case 0x104:
		      switch(MCom[i].m & 0xf) {
			      case 0:                        // assume64 ecc
							break;
			      case 1:
							break;
						}
		      break;
#endif

		    case 0x200:
		    case 0x201:
		      switch(MCom[i].m & 0x1f) {
		        case 0:
	            strcpy(__title__,theLine);
	            if(*__title__ == '\'' || *__title__ == '\"') {
	              strcpy(__title__,__title__+1);
	              __title__[strlen(__title__)-1]=0;
	              }
		          break;
		        case 1:
//		          *myBuf=0;
		          PageLength=(uint16_t)FNEvalExpr(14,myBuf).v;
//							if(!o1.l.valid)
//								PROCError(2062);
		          break;
		        case 2:
//		          *myBuf=0;
		          LineLength=(uint16_t)FNEvalExpr(14,myBuf).v;
//							if(!o1.l.valid)
//								PROCError(2062);
		          break;
#if ST62 || PIC
		        case 5:
#if PIC==16 || PIC==18
							if(!CPass)			// fARE!!
								PROCWarn(1007,myBuf);
			        theLine;
							break;
#endif
#endif
		        case 3:
		          if(CPass) {
		            FNLO(myBuf);  // ignoro "out"
			          do {
			            FNLO(myBuf);
//			            puts(myBuf);
			            if(o1.var=FNCercaVar(myBuf,0,FALSE,1))
			              printf("%ld ",o1.var->value);
			            else  
			              printf("%s ",myBuf);
			            } while(*myBuf);
	              putchar('\n');
                }
              else {
//  	            fseek(FIn,FNGetLine(ftell(FIn)+4,myBuf),SEEK_SET);   // salto OUT
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
					    PROCDclVar(myBuf,1,1,0);
		          break;
		        case 0x11:               // memoria a finestre
					    PROCDclVar("W_ON",1,1,0);
		          break;
#endif		          
#if PIC==16 || PIC==18
		        case 6:          // ERRORLEVEL
	            FNObbLO(myBuf);
							do {
		            FNLO(myBuf);
								} while(*myBuf);
							if(!CPass)			// fARE!!
								PROCWarn(1007,myBuf);
							break;
		        case 0x10:               // CONFIG
		          o1.l=FNEvalExpr(14,myBuf);
							if(!o1.l.valid)
								PROCError(2062);
							strcpy(myBuf,"_config");
							CurSeg=PROCDclSeg(myBuf,SEGMENT_CONFIG);		// 
							Seg[CurSeg-1].CurAddr=0;
		          PROCOut(OUT_SEGM);
		          PROCDclVar(myBuf,o1.l,0,0/*,myBuf*/);
		          PROCOut1("! ");
							PROCOut(LOBYTE(o1.l.v),HIBYTE(o1.l.v),-1,i);
		          break;
		        case 0x11:               // IDLOCS
		          o1.l=FNEvalExpr(14,myBuf);
							if(!o1.l.valid)
								PROCError(2062);
							strcpy(myBuf,"_idlocs");
							CurSeg=PROCDclSeg(myBuf,SEGMENT_IDLOCS);		// 
							Seg[CurSeg-1].CurAddr=0;
		          PROCOut(OUT_SEGM);
		          PROCDclVar(myBuf,o1.l,0,0/*,myBuf*/);
		          PROCOut1("! ");
							PROCOut(LOBYTE(o1.l.v),HIBYTE(o1.l.v),-1,i);
		          break;
#endif		          
		        }
		      break;

		    case 0x210:		//CPU
#if I8086
          o1.l=FNEvalExpr(14,myBuf);
					// GESTIRE cose tipo 8086 80186 ecc!
					if(!o1.l.valid)
						PROCError(2062);
					CPULevel=o1.l.v;
#endif		          
		      break;

		    case 0x400:
		      switch(MCom[i].m & 0xf) {
						long l;
						case 0:			// macro (alcuni usano MACRO <nome> ma sarebbe un problema da gestire, chissene
							*T1S=0;
							do {		///*** SI POTREBBE TOGLIERE! non mettiamo argomenti dopo nome macro
								FNLO(myBuf);
								if(*T1S)
									strcat(T1S," ");
								strcat(T1S,myBuf);
		//	    puts(myBuf);         // macro!
								} while(*myBuf);
  						l=theLinePtr;
							*T2S=0;
							do {
								char *p;
								fgets(myBuf,255,FIn);
		//      puts(myBuf);         // macro!
								__line__++;
								if(p=(char*)stristr(myBuf,"endm"))
									break;
								if(*T2S)
									strcat(T2S," ");
								p=strchr(myBuf,'\r');
								if(p)
									*p=' ';
								strcat(T2S,myBuf);
								} while(*myBuf);
							if(!CPass) {
								char *p;
								if(FNMacro(T1S))
									PROCError(4005,T1S);
								replace_char(T1S, '\t', ' ');		// per sicurezza!
								if(p=strchr(T2S,';'))		// direi
									*p=0;
								replace_char(T2S, '\t', ' ');		// 
								PROCDefineMacro(T1S,T2S);
								}
							OldTextp=l;
							break;
						case 1:			// exitm... gestito di là
							break;
						case 2:			// endm :) gestito sopra
							break;
						case 3:			// local, fare...
							break;
						}
					break;

		    case 0x410:
		      switch(MCom[i].m & 0xf) {
						long l;
						case 0:			// struc
						case 1:			// union
							break;
						case 2:			// record
							break;
						case 0xf:			// ends
							{
							struct TAGS *myT;
							if(!parm)
								PROCError(1006,"ENDS");
							if(myT=FNCercaAggr(parm,FALSE,0)) {
								if(inStructDeclaration) {
									if(!strcmp(myT->label,parm)) {
										inStructDeclaration=0;
										currVarTag=0;
										}
									else {
										PROCError(1006,parm);
										}
									}
								else {
									PROCError(1006,parm);
									}
								}
							else if(FNCercaSeg(parm) == CurSeg) {
								CurSeg=SEG_UNDEFINED;
								// v. anche cseg ecc, unire!
								}
							else if(stricmp(parm,"CSEG") && stricmp(parm,"CSEG")) {		// bah compatibilità con tutto :)
								PROCError(1006,parm);
								}
							}
							break;
						}
					break;

		    case 0x800:  
		      switch(MCom[i].m & 0xf) {
						FILE *OldF;
						int OldPass;
						int OldLine;
						char OldFile[128];
		      
		        case 0:				// include
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
soloIFS801:
		      switch(MCom[i].m & 0xf) {
						char myBuf2[256];
		        case 0:                          // if
							inIfDef=TRUE;
		          IfDefs++;
				      FNLA(myBuf);
		          switch(*myBuf) {
	              case '@':		// uso questa per variabili predefinire (was: ma OCCHIO MACRO..., usano % per parm - MASM è diverso!
						      FNLO(myBuf);
									switch(*(myBuf+1)) {
										// anche CPASS pare, v.MASM...
										case '1':
										case '2':
									    if(!UNDEFD)
												UNDEFD=CPass == (*(myBuf+1) - '1');
											break;
										default:
				              PROCError(2065,myBuf);  
											break;
										}
			            break;
		            default:
				          o1.l=FNEvalExpr(14,myBuf);
									if(!o1.l.valid) {
										o1.var=FNCercaVar(myBuf,0,FALSE,1);   // cerco come se fosse PASS2
										if(o1.var) {
											o1.l.v=o1.var->value;
											o1.l.valid=TRUE;
											}
										}
									if(!o1.l.valid)
										PROCError(2062);

									if(*FNLO(myBuf1)) {
				            o2.l=FNEvalExpr(14,myBuf);
										if(!o2.l.valid)
											PROCError(2062);
										if(myBuf1[0]) {
											myBuf1[0]=toupper(myBuf1[0]);
											myBuf1[1]=toupper(myBuf1[1]);
											switch(*(uint16_t*)myBuf1) {
												case 'QE':
													o1.l.v = o1.l.v==o2.l.v;
													break;
												case 'EN':
													o1.l.v = o1.l.v!=o2.l.v;
													break;
												case 'TL':
													o1.l.v = o1.l.v<o2.l.v;
													break;
												case 'TG':
													o1.l.v = o1.l.v>o2.l.v;
													break;
												case 'EL':
													o1.l.v = o1.l.v<=o2.l.v;
													break;
												case 'EG':
													o1.l.v = o1.l.v>=o2.l.v;
													break;
												default:
													PROCError(2062);
													break;
												}
											}
									//GESTIRE EQ NE GT LT LE GE ... stringhe... masm; e anche INSTR https://learn.microsoft.com/en-us/cpp/assembler/masm/instr?view=msvc-170
										}

//		              UNDEFD=!i;  idem 2025
							    if(!UNDEFD)
							      UNDEFD=!o1.l.v;

//			            if(v1=FNCercaVar(myBuf,FALSE,1))
//			              UNDEFD=!v1->value;
// fare...			            else if(v1=FNDefined(myBuf))
//			              UNDEFD=0;
//			            else
//			              PROCError(2065,myBuf);  
			            break;
		            }
							inIfDef=FALSE;
		          break;  
		        case 1:						// ife
		          IfDefs++;
				      FNObbLO(myBuf);
	            if(o1.var=FNCercaVar(myBuf,0,FALSE,1)) {   // cerco come se fosse PASS2
						    if(!UNDEFD)
		              UNDEFD=!!o1.var->value;
  	            }
	            else
	              PROCError(2065,myBuf);  
	            break;
		        case 2:						//ifdef
		          IfDefs++;
				      FNObbLO(myBuf);
	            i=(int)FNCercaVar(myBuf,0,FALSE,1);
							if(!i)
								i=(int)FNDefined(myBuf);		// cerco entrambe (NO sostituzione #define globale!)
					    if(!UNDEFD)
		            UNDEFD=!i;
		          break;
		        case 3:						//ifndef
		          IfDefs++;
				      FNObbLO(myBuf);
	            i=(int)FNCercaVar(myBuf,0,FALSE,1);
							if(!i)
								i=(int)FNDefined(myBuf);		// cerco entrambe (NO sostituzione #define globale!)
					    if(!UNDEFD)
		            UNDEFD=!!i;
		          break;
		        case 4:						//ifdif
		          IfDefs++;
				      FNObbLO(myBuf);
							PROCCheck(',');
				      FNObbLO(myBuf2);
					    if(!UNDEFD)
		            UNDEFD=!strcmp(myBuf,myBuf2);
		          break;
		        case 5:						//ifidn
		          IfDefs++;
				      FNObbLO(myBuf);
							PROCCheck(',');
				      FNObbLO(myBuf2);
					    if(!UNDEFD)
		            UNDEFD=strcmp(myBuf,myBuf2);
		          break;
		        case 6:						//ifdifi
		          IfDefs++;
				      FNObbLO(myBuf);
							PROCCheck(',');
				      FNObbLO(myBuf2);
					    if(!UNDEFD)
		            UNDEFD=!stricmp(myBuf,myBuf2);
		          break;
		        case 7:						//ifidni
		          IfDefs++;
				      FNObbLO(myBuf);
							PROCCheck(',');
				      FNObbLO(myBuf2);
					    if(!UNDEFD)
		            UNDEFD=stricmp(myBuf,myBuf2);
		          break;
		        case 8:						//ifb (solo dentro macro..
		          IfDefs++;
				      FNObbLO(myBuf);
					    if(!UNDEFD)
		            UNDEFD=!!(*myBuf);
		          break;
		        case 9:						//ifnb (solo dentro macro..
		          IfDefs++;
				      FNObbLO(myBuf);
					    if(!UNDEFD)
		            UNDEFD=!(*myBuf);
		          break;
		        }
		      break;    

		    case 0x802:
soloIFS802:
		      switch(MCom[i].m & 0xf) {
		        case 0:                          // else
			        if(!IfDefs)
			          PROCError(1019);
			        UNDEFD = !UNDEFD;      
		          break;
		        case 1:                          // elseif
// finire!			        if(!IfDefs)
			          PROCError(1019);
			        UNDEFD = !UNDEFD;      
		          break;
		        case 0xf:                          // endif
			        if(!IfDefs)
			          PROCError(1020);
			        IfDefs--;
			        if(!IfDefs)
			          UNDEFD=FALSE;
		          break;
		        }
		      break;  
					
		    case 0x803:
		      switch(MCom[i].m & 0xf) {
		        case 0:						//define
				      FNObbLO(myBuf);
				      FNObbLO(myBuf1);
							if(!CPass) {
								PROCDefine(myBuf,myBuf1);

//								PROCWarn(1021 /*2007*/,myBuf);
								// finire! 2025? mi pare andare
								}
		          break;
		        }
		      break;  


		    case 0x808:			// pragma
					printf("pragma %s ignored\n",myBuf);			//:)
					if(FCod)
    				fprintf(FCod,"pragma %s ignored\n",myBuf);
	        theLine;
		      break;

		    case 0x809:			// warning
		      switch(MCom[i].m & 0xf) {
						char *p;
		        case 0:                          // warning
//				      FNObbLO(myBuf);
							p=strchr(theLine,' ');
							if(p)
								p++;
							if(!CPass) {
 //   						printf("WARNING: %s\n",p);		// anche NULL va bene :)
								PROCWarn(4099,p);
								}
							if(CPass)
								if(FCod)
    							fprintf(FCod,"WARNING: %s\n",p);
				      break;    
		        case 1:                          // error
							p=strchr(theLine,' ');
							if(p)
								p++;
							if(!CPass) {
//    						printf("ERROR: %s\n",p);
								PROCError(4099,p);
								}
							if(CPass)			// ovviamente non arriverà mai ma ok :)
								if(FCod)
    							fprintf(FCod,"ERROR: %s\n",p);
				      break;    
						}
		      break;

		    default:
	      	break;
		    }
	    return TRUE;
		  }
	  else {
noStmt:
			if(*TS && !UNDEFD) {
				theLinePtr=OldTextp;
  	    bInizioRiga=oi;
				VarDeclared=FALSE;		// controllare oi?
  	    return FALSE;
  	    }
  	  else
  	    return TRUE;  
	    } 

#if I8086
foundSegm:
    return TRUE;
#endif

	  }
  }
 
char *FNGetConst(char *s) {
	struct OPERAND o;

	memset(&o,0,sizeof(struct OPERAND));
 
  *s=0;
  FNRev(14,&o,s);
  
//  if(debug)
//    printf("Costante: %s\n",s);
    
  if(o.V>=0) {
    PROCError(2057);
    }
  else {
    return s;
    }
  return NULL;
  }
 
struct VARS *FNCercaVar(const char *n,uint32_t parent,uint8_t m,uint8_t pass) {
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
	while(i=strcmp(myN,V->label) /*|| (parent!=V->tag)*/) {
//		  printf("trovo... %Fs\n",V->label);
		if(i<0)
		  V=V->left;         // Esegue la ricerca
		else
		  V=V->right;
		if(!V)
		  return NULL;
		}
	if(parent && parent != V->tag)
		V=V->next;
//		  printf("trovo...\n");
		
//  if(m && V->proc!=CurProc)         // var. locali supportate?
//		goto rifo;		//NO! bisogna inserire anche nell'albero la condizione "curproc"
  
      if(CPass) {
				if((V->type & VAR_EXTERN) && !V->extUsed) {// USARE HIBYTE(HIWORD
//			  if(CPass) {
//			  V->value = V->value & 0xff00ffff | (((long)++NumExtrnUsed) << 16);

//	  	    *(((char *)&V->value)+2) = ++NumExtrnUsed;      // max 255...

//	  	    V->value = MAKELONG(V->value,MAKEWORD(0,Seg[CurSeg-1].numExt));      // max 255... 2024 VERIFICARE
					V->extUsed=Seg[CurSeg-1].numExt;
// non sembra mai passare di qua cmq...


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
 
struct VARS *PROCDclVar(char *nome, struct SEGMENTED_VALUE lv, uint16_t type, uint32_t tag, const char *segm) {
  int t1,T2,i;
  struct VARS *v;
  char T[256],S[256],MyBuf[256];
  char *t,*t2;
  
  if(IgnoreCase)
    strupr(nome);
  v=FNCercaVar(nome,tag,TRUE,CPass);     // SE LA VARIABILE GIA' ESISTE...
  if(!CPass) {
	  if(v) {
	    if(((v->type & (VAR_PUBLIC | VAR_FUNZ | VAR_PUBLIC_DEFINED)) != VAR_PUBLIC) || (type & VAR_EXTERN))
	    // lascio passare le public ridefinite ma blocco eventuali extern

			//  CONTROLLARE 2024! ovviamente ciucco 380 200
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
 
	  if(/*lv.seg==SEGM_UNDEFINED && */ lv.v==-1  /*USARE? (!lv.valid)*/) {
//	    v->value=Seg[CurSeg-1].CurAddr | (((long)CurSeg) << 16);
			if(segm) {		// FINIRE... era un'idea per mettere in CONST ecc in base al tipo di dichiarazione/label 2024
//				i=FNCercaSeg(segm);
				i=PROCDclSeg(segm,SEGMENT_DATA);		// mah
				v->value=Seg[i-1].CurAddr;
				v->segm=i; // [USARE 2024! e liberare word alta
				}
			else {
				v->value=Seg[CurSeg-1].CurAddr;
				v->segm=CurSeg; // [USARE 2024! e liberare word alta
				}
			if(type & (VAR_STRUCT | VAR_UNION)) {
				v->size=((struct TAGS*)tag)->size;
				}
			else if(type & VAR_RECORD) {
				v->size=((struct TAGS*)tag)->size/8;
				}
			else {
#if GD24032
				v->size=8;
#else
				v->size=4;
#endif
				}
	    }
	  else {
	    v->value=lv.v;
			v->segm=SEG_NONE;
			lv.valid=TRUE;
	    v->size=FNGetSize(lv);
//	    printf("size di %s: %d\n",nome,FNGetSize(lv));
			if(type != 1) {
				v->segm=lv.seg; // [USARE 2024! e liberare word alta
				}
	    }

	  v->tag=tag;
		if(type & VAR_EXTERN) {
	    v->type |= type;
      subOut(OUT_VAR_EXT,1);
			}
		else if(v->type & VAR_PUBLIC) {
	    v->type |= (VAR_PUBLIC_DEFINED | type);
      subOut(OUT_VAR_EXT,0);
			}
	  else  
	    v->type |= type;
   	if((v->type & (VAR_PUBLIC | VAR_LABEL)) == (VAR_PUBLIC | VAR_LABEL)) {
// 	    PublicEQU=TRUE;
    	i=PROCDclSeg(defaultAbsSegm,SEGMENT_ABS);
//			v->value=MAKELONG(v->value,MAKEWORD(0,i));
			v->segm=i;
			}
	  v->proc=CurProc;
    v->lpos=0l;
    v->extUsed=0;
    }
	  
  if(FMap && !CPass)
		fprintf(FMap,"-\t%-32s\t%6X:%04X\t%4X\t%4X\t%s (%s:%u)\n",v->label,
			HIWORD(v->value),LOWORD(v->value),v->type,v->size,v->proc ? v->proc->label : ((char *)""),
			__file__,__line__);
	// FARE anche in ordine alfabetico...

	VarDeclared=TRUE;
  return v;  

//  fprintf(stderr,"\tcreo VAR %d: %s = %lx\n",v,nome,Var[v].value);
  
  }

const struct TAGS *FNCercaAggr2(const char *n,const struct TAGS *V,uint8_t mode) {
	const struct TAGS *V2;
	uint8_t level;

	if(!V)
	  return NULL;

	if(V2=FNCercaAggr2(n,V->left,mode))
		return V2;
	V2=V;
	if(mode)			// 0=tutti, 1=solo figli ossia sotto-campi
		V2=V2->next;
	level=0;
	while(V2) {
		if(!strcmp(n,V2->label))
			return V2;
		level++;
		V2=V2->next;
		}

	if(V2=FNCercaAggr2(n,V->right,mode))
		return V2;
	}

struct TAGS *FNCercaAggr(const char *n,uint8_t bWhat,uint32_t parent) {		// anche filtro, struct/union/record?? verificare namespace
  int i;
  char myN[MAX_NAME_LEN+2];
  struct TAGS *V;

	strcpy(myN,n);
	if(IgnoreCase) 
		strupr(myN);
		
	switch(bWhat) {
		case 0:			// solo top level
			if(parent) {
				V=(struct TAGS*)parent;
				while(V) {
					if(!strcmp(myN,V->label)) {
						break;
						}
					V=V->next;
					}
				}
			else {
				V=StrTag;
				if(!V)
					return 0;
				while(i=strcmp(myN,V->label) /*|| (parent!=V->tag)*/) {
			//		  printf("trovo... %Fs\n",V->label);
					if(i<0)
						V=V->left;         // Esegue la ricerca
					else
						V=V->right;
					if(!V)
						return NULL;
					}
				}
			break;
		case 1:		// tutti
			return (struct TAGS*)FNCercaAggr2(myN,StrTag,0);
			break;
		case 2:		// solo figli
			return (struct TAGS*)FNCercaAggr2(myN,StrTag,1);
			break;
		}

  if(CPass) {
		}

  return V;
  }
 
struct TAGS *PROCDclAggr(char *nome, uint16_t type, uint32_t tag, NUM_TYPE val) {
/*	Global labels, macro names, and record field names must all be unique, but record field names can have the same names as structure field names.*/
  int t1,i;
  struct TAGS *v;
  char T[256];
  
  if(IgnoreCase)
    strupr(nome);
  v=FNCercaAggr(nome,tag ? (type & VAR_RECORD ? TRUE : FALSE) : FALSE,currVarTag);     // SE LA VARIABILE GIA' ESISTE... (lo spazio dei RECORD è globale...
  if(!CPass) {
		v=(struct TAGS *)malloc(sizeof(struct TAGS)); 
		if(!v) {
		  PROCError(1001,"Fine memoria TAGS");
		  }
		v->type=0;
		v->dim=0;
		v->size=0;
		v->pos=0;
		v->value=val;

		v->tag=tag;
		
 		strncpy(v->label,nome,MAX_NAME_LEN);
//		v->parent=parent;
  		
		v->next=NULL;
		v->left=v->right=NULL;

		if(!tag)	{
			StrTag=PutOnTree2(StrTag,StrTag,v);
			}
		else {
			struct TAGS *r=(struct TAGS*)tag;
		  while(r->next)
		    r=r->next;		// i figli vanno tutti in lista, in ordine di creazione
			r->next=v;
			}
		LTag=v;
		}
 
  v->type = type;
	  
  return v;  

//  fprintf(stderr,"\tcreo AGGR %d: %s = %lx\n",v,nome,Var[v].value);
  
  }

NUM_TYPE FNUpdAggr(struct TAGS *V) {
	struct TAGS *V2=V;
	NUM_TYPE n;

	n=0;
	while(V2) {
//		V->value |= V2->value >> V2->pos;
		n <<= V2->pos;
		n |= V2->value;
		V2=V2->next;
		}
	return n;
  }

NUM_TYPE getMask(uint8_t n) {
	NUM_TYPE r=0;

	do {
		r <<= 1;
		r |= 1;
		} while(--n);
	return r;
	}
NUM_TYPE FNInitAggr(struct TAGS *V,bool mode) {		// inizializza Record
	struct TAGS *V2;
	NUM_TYPE n=0;
	char T1[256],MyBuf[256];
	struct SEGMENTED_VALUE l;
	uint8_t rembits=8;

	FNLA(MyBuf);
  switch(*MyBuf) {
		case '<':
 			PROCCheck(*MyBuf);
			V2=V->next;
			do {
				if(!V2)
					PROCError(2069);
				rembits -= V2->size;
				FNLA(T1);
				if(isalnum(*T1)) {
					l=FNEvalExpr(14,T1);
					if(*FNLA(T1) == ',')
						PROCCheck(',');
					if(l.v > getMask(V2->size)) {
						if(!CPass)
							PROCWarn(2058);
//						l.v = getMask(V2->size);		// tronco al max!
						}
					}
				else
					l.v=V2->value;
				l.v &= getMask(V2->size);			// tronco al max!
				n |= l.v << (rembits);
				V2=V2->next;
				} while(V2);
			PROCCheck('>');

			if(mode)
				subBytes("",(V->size+7)/8,n);
			break;
		case 'D':
			// gestire DUP, ossia array
			if(mode)
				;
			break;
		case ',':
			break;
		case 0:
		default:
			PROCError(2054,"<");
			break;
		}

	return n;
  }

struct TAGS *subAllocTag(const char *TS) {
  struct TAGS *C;
  
  C=(struct TAGS *)malloc(sizeof(struct TAGS)); 
  if(!C) {
    PROCError(1001,"Fine memoria TAGS");
    }
  if(StrTag) {
    LTag->next=C;
    LTag=C;
    }
  else {
    LTag=StrTag=C;
    }
  C->next=(struct TAGS *)NULL;
  strncpy(C->label,TS,MAX_NAME_LEN);
  C->label[MAX_NAME_LEN]=0;
  return C;
  }

char *FNGetLabel(char *A,int8_t m) {

  switch(m) {
    case 0:
      m='L';
      break;
    case 1:
      m='J';
      break;  
    case 2:
      m='L';
      break;  
    case 3:
      m='$';
      break;  
    default:
      m='X';
      break;  
    }
  sprintf(A,"%c%05d",m,++LABEL);
  return A;
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
		Seg[v].align=1;
		Seg[v].unused=0;
		Seg[v].BaseAddr=0xffffffff;
	  if(FMap)
			fprintf(FMap,"+\t%-38s\t%04X\t%02u\t(%s:%u)\n",Seg[v].label,
				Seg[v].type,Seg[v].align,__file__,__line__);
		return v+1;
		}
  else
    return T1;  

//  fprintf(stderr,"\tcreo SEG %d: %s = %lx\n",v,nome,Var[v].value);
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
			n->next=NULL;
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
	while(root) {
		struct VARS *t=root->next;
		free(root);
		root=t;
		}
	}

struct TAGS *PutOnTree2(struct TAGS *root,struct TAGS *r,struct TAGS *n) {
  int i;
	
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
	  PutOnTree2(r,r->left,n);  // a sinistra <
	  }
	else {
	  if(i>0) {
		  PutOnTree2(r,r->right,n); // a destra   > 
		  }
		else {  
		  while(r->next)
		    r=r->next;// se sono uguali lo inserisco nella lista 
			r->next=n;
			n->next=NULL;
			}
		}

	return root;
	}

void Libera2(struct TAGS *root) {

	if(!root) 
		return;

	Libera2(root->left);
	Libera2(root->right);
	while(root) {
		struct TAGS *t=root->next;
		free(root);
		root=t;
		}
	}


uint8_t asc2hex(uint16_t t) {
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

uint8_t  *convertHEX2BIN(FILE *f,uint32_t *bsize) {
	uint8_t *theMemory;
	char myBuf[256],*p;
	uint8_t *b;
	int minAddr,maxAddr,len;
	uint8_t cks;
	uint32_t ofs,extOfs,row,checksum;
	int recType,i,j;


	theMemory=(uint8_t *)malloc(65536);			// 
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
		cks=len=asc2hex(*(uint16_t *)p);
		p+=2;
		j=asc2hex(*(uint16_t *)p);
		ofs=j;
		cks+=j;
		ofs <<= 8;
		p+=2;
		j=asc2hex(*(uint16_t *)p);
		ofs |= j;


		cks+=j;
//		if(ofs>(maxMemory*2)) {		// word -> byte
//					S.Format("Posizione di memoria specificata nel file HEX non valida: %04X, %s",ofs,myBuf);
//				}
		p+=2;
		recType=asc2hex(*(uint16_t *)p);		// tipo rec: 0 dati, 1 EOF, 2=Ext.Segm.Addr.Rec., 
																		// 3=Start Segm.Addr.Rec., 4=Ext.Linear Addr.Rec., 5=Start Linear Addr.Rec.
		cks+=recType;
		p+=2;
		switch(recType) {
			case 2:			// segmented Intel
				j=asc2hex(*(uint16_t *)p);
				extOfs=((uint32_t)j) << 12;
				cks+=j;
				p+=2;
				j=asc2hex(*(uint16_t *)p);
				extOfs |= ((uint32_t)j) << 4;
				cks+=j;
				p+=2;
				break;
			case 4:			// linear
				j=asc2hex(*(uint16_t *)p);
				extOfs=((uint32_t)j) << 24;
				cks+=j;
				p+=2;
				j=asc2hex(*(uint16_t *)p);
				extOfs |= ((uint32_t)j) << 16;
				cks+=j;
				p+=2;
				break;
			case 0:
				if(extOfs==0) {
					b=((uint8_t *)theMemory)+ofs;
					for(i=0; i<len; i++) {
						j=asc2hex(*(uint16_t *)p);
						minAddr=min(minAddr,(uint8_t *)b-(uint8_t *)theMemory);
						maxAddr=max(maxAddr,(uint8_t *)b-(uint8_t *)theMemory);
						if(*b != 0xff)		// ma non esce... sistemare! credo sia l'HEX che esce già così, in sequenza brutale...
							PROCWarn(2104);
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
		j=asc2hex(*(uint16_t *)p);
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


const char *stristr(const char *s1,const char *s2) {
	int n=strlen(s2);

	while(*s1) {
    if(!strnicmp(s1,s2,n))
      return s1;
    s1++;
    }
	return NULL;
	}

char* replace_char(char* str, char find, char replace) {
  char *current_pos = strchr(str,find);

  while (current_pos) {
    *current_pos = replace;
    current_pos = strchr(current_pos+1,find);
    }
  return str;
	}

int8_t isasym(char ch) {
	
	return iscsym(ch) || (ch=='$') || (ch=='?');
	}


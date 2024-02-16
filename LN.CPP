// **************************
// *  C-LINKER              *
// *        BY G.Dar        *
// *            5/7/89      *
// *            6/6/94      *
// *            9/2/24      *
// **************************


#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <math.h>
#include <conio.h>
#include <ctype.h>
//#include <windows.h>
#include "ln.h"


unsigned char OutMap=FALSE;    
unsigned char IgnoreCase=TRUE,IgnoreRAM=TRUE,RelocData=FALSE;
unsigned char HexFormat=FALSE,FillMemory=FALSE;
unsigned char Verbose=FALSE;
unsigned char OnlyShow=FALSE;

char __file__[256],__name__[64];
int __line__;
char __date__[10];
char __time__[10];  
int StackSize=0x200,CodeSize=MAX_CODE_SIZE,EnvironSize=0x80,DataSize=MAX_DATA_SIZE,AbsSize=0x1000 /* */;
char defaultCodeSegm[64],defaultDataSegm[64];
struct VARS *Var, *LVars, *nRoot;
struct SEGS Seg[MAX_SEGS],tSeg[MAX_SEGS],segOverride[MAX_SEGS];
int16_t Segs=0,SegOverrides=0;

struct ERRORE Errs[]={
  1000,"errore interno - contattare ADPM",
  1001,"fine memoria disponibile",
  2001,"impossibile aprire il file eseguibile",
  2002,"file oggetto non trovato",
  2003,"libreria non trovata",
  2004,"impossibile creare il file",
  2060,"errore di sintassi",
  2061,"formato file sconosciuto",
  2100,"simbolo definito più volte",
  2101,"simbolo esterno non definito",
  2200,"errore nella rilocazione DATA",
  4002,"flag sconosciuto ignorato",
  4010,"segmenti sovrapposti",
  4021,"lunghezza stack non impostata",
  0,""
  };

signed char debug=0;
char buffer[256];
char INS[256],OUS[256],LIBS[256],MAPS[256];

char *AddExt(char *n, char *x, int m) {      // m=1 forza ext, m=0 accetta ext precedente
  char *p;
  
  if(p=strchr(n,'.')) {
    if(m)
  		strcpy(p+1,x);
		}
  else {
		strcat(n,".");
		strcat(n,x);
		}      
		
	return n;	
  }
  
char *FScanf(FILE *f, char *s, long o) {
  register int ch;
  long l;
  char *p=s;

	fseek(f,o,SEEK_SET);
  while((ch=getc(f)) && (ch != EOF))
    *p++=ch;
  *p=0;
  
  return s;
  }
  
int main(int argc, char **argv) {
  int i,ch,j,t;
  long l,l1,l2,l3;
  char myBuf[256];
  char *p,*p1,*s;
  struct VARS *V;
  FILE *FO,*FI,*FMap;
  struct OBJ_HEADER myHeader;
  struct OBJ_SEG_DEF mySegDef;
  uint8_t *ptr;
  
  for(i=1; i<argc; i++) {
		if(*argv[i]=='-' || *argv[i]=='/') {
		  switch(toupper(*(argv[i]+1))) {
        case 'B':
          strcpy(myBuf,argv[i]+2);
          p=strtok(myBuf,"=");
          if(!(p=strtok(NULL,"="))) 
            PROCError(2060,NULL,NULL);
          sscanf(p,"%x",&l);  
					l &= 0xffff;
					strcpy(segOverride[SegOverrides].label,myBuf);
					if(IgnoreCase)
						strupr(segOverride[SegOverrides].label);
					segOverride[SegOverrides].BaseAddr=l;
					segOverride[SegOverrides].type= *(argv[i]+1) == 'b' ? SEGMENT_DATA : SEGMENT_CODE;		// bah vabbe'
					SegOverrides++;
					if(SegOverrides>=MAX_SEGS)
					  PROCError(1000,myBuf,NULL);
					if(Verbose)		// non va benissimo messo qua... ok
		        printf(" definisco segm. addr %s (%04x) a %x %s\n",
							segOverride[SegOverrides-1].label,segOverride[SegOverrides-1].type,
							segOverride[SegOverrides-1].BaseAddr,p);
          break;
        case 'C':
          sscanf(argv[i]+2,"%u",*(argv[i]+1) == 'c' ? &CodeSize : &DataSize);
          break;
        case 'D':
          debug=TRUE;
          break;
        case 'F':
          FillMemory=TRUE;
          break;
        case 'H':
          HexFormat=TRUE;
          break;
        case 'I':
          IgnoreCase=FALSE;
          break;
        case 'M':
          RelocData=TRUE;
          break;
        case 'R':
          IgnoreRAM=FALSE;
          break;
        case 'S':
          sscanf(argv[i]+2,"%u",&StackSize);
          break;
        case 'V':
          Verbose=TRUE;
          break;
        case 'L':
          OnlyShow=TRUE;
          break;
        case '?':
          goto DisplayInfo;
          break;
				default:
ukswitch:
				  PROCWarn(4002,argv[i]+1,NULL);
				  break;
				}
		  *argv[i]=0;
		  }
		else {
      strcat(buffer,argv[i]);
      if(buffer[strlen(buffer)-1] != ',')
        strcat(buffer," ");
		  }
		}
  
//printf("buffer: -%s-\n",buffer);
  if(*buffer) {
//    buffer[strlen(buffer)-1]=0;
    strupr(buffer);
 		p=strtok(buffer,",");
// 		printf("1° tok: %s\n",p);
 		if(p) {
 		  strcpy(INS,p);
	 		p=strtok(NULL,",");                    //non funziona con 2 virgole di seguito
// 		printf("2° tok: %s\n",p);
	 		if(p) {
	 		  strcpy(OUS,p);
		 		p=strtok(NULL,",");
// 		printf("3° tok: %s\n",p);
  	 		if(p) {
	  	 		strcpy(MAPS,p);
			 		p=strtok(NULL,",");
// 		printf("4° tok: %s\n",p);
	  	 		if(p) {
  	  	 		strcpy(LIBS,p);
	  	 		  }
//	  	 		else {
//  	  	 		strcpy(LIBS,MAPS);
//  	  	 		*MAPS=0;
//	  	 		  }  
	  	 		}
		 		}
		 	else {
		 	  strcpy(OUS,INS);
//		 	  strcpy(LIBS,INS);
//		 	  AddExt(OUS,"lib");
		 	  }	
 		  }
    }
  else {     
DisplayInfo:
		fprintf(stderr,"Sintassi: LN <objfile>,<exefile>,<mapfile>,<lib> [switches]\n");
		puts("Switch:     /V ... mostra messaggi stato");
		puts("            /B ... definisce indirizzo base segmento codice");
		puts("            /b ... definisce indirizzo base segmento dati");
		puts("            /i ... ignora maiuscole/minuscole nei nomi");
		puts("            /f ... riempie la memoria anche agli indirizzi non utilizzati");
		puts("            /r ... inserisce nel file output anche le locazioni RAM non usate");
		puts("            /c ... imposta dimensione del segmento Codice");
		puts("            /s ... imposta dimensione del segmento di Stack");
		puts("            /l ... lista i segmenti del file oggetto");
		puts("            /h ... crea un file HEX (Intel) anziché BINario");
		puts("            /? ... help");
		puts("Errorlevel: 0=OK, 1=Errore esecuzione, 2=Errore comandi,");
		puts("            3=Errore svolgimento, 4=Interruzione, 99 info.");
		exit(99);
		}
		
  PROCInit();

  if(HexFormat) 
    AddExt(OUS,"HEX",0);
  else  
    AddExt(OUS,"BIN",0);
  if(debug)
    printf("INS: -%s-, OUS -%s-, MAP -%s-, LIBS -%s-\n",INS,OUS,MAPS,LIBS);
		
  i=1;
  __line__=1;
  if(!OnlyShow) {
		FO=fopen(FNTrasfNome(OUS),"wb");
		if(!FO) 
			PROCError(2001,OUS,NULL);
		if(Verbose)
			fprintf(stderr,"%s - %s\n\n",__date__,OUS);
		if(AbsSize)
			i=PROCDclSeg("_abs",SEGMENT_ABS,1);
		if(StackSize)
			PROCDclVar("__STK_SIZE",0,StackSize,i-1);
		}
	else
		if(Verbose)
			fprintf(stderr,"%s\n\n",INS);

#if ARCHI
  fprintf(FO,"REM > %s",FNTrasfNome(OUS))
  fprintf(FO,"REM *** Generated by G.Dar linker v.%d.%02d",__VER__ / 256,__VER__ & 255)
  fprintf(FO,"DIM A% &%lx",CodeSize)
  fprintf(FO,"FOR I%=0 TO 2 STEP 2")
  fprintf(FO,"P%=A%")
  fprintf(FO,"[ OPT I%")
#elif Z80
#endif
//	strcpy(__file__,INS);
//  ptr=MPtr;
  if(s=strtok(INS," ")) do {
    if(s != INS)
      *(s-1)=' ';
    strcpy(__file__,s);
 		AddExt(__file__,"O",0);
  	if(Verbose)
      fprintf(stderr,"Lettura oggetto %s...\n",__file__);
  	FI=fopen(FNTrasfNome(__file__),"rb");
	  if(!FI)
	    PROCError(2002,__file__,NULL);
		fread(&myHeader,sizeof(struct OBJ_HEADER),1,FI);
		if(myHeader.v1 > 1)
		  PROCError(2061,__file__,NULL);
		if(OnlyShow) {
			printf("\nVersione file: %02X (%02X %02X %X); %u segmenti\n",
				myHeader.v1,myHeader.v0,myHeader.v2,myHeader.res1,myHeader.numSegs);
			}
		j=0;
		while(myHeader.numSegs--) {
  		fread(&mySegDef,sizeof(struct OBJ_SEG_DEF),1,FI);
  		l=ftell(FI);

			if(OnlyShow) {
				int oldpos=ftell(FI);
				fseek(FI,myHeader.offNomi+mySegDef.offNome,SEEK_SET);
	  		FScanf(FI,myBuf,myHeader.offNomi+mySegDef.offNome);
				printf("Segmento %u %-20s: tipo %04X, lungh. %06x; simboli %u/%u\n",j,myBuf,
					mySegDef.tipo,mySegDef.lunCode,mySegDef.numPublic,mySegDef.numExtern);
	  		fseek(FI,oldpos,SEEK_SET);
				j++;
				}
			else {

  			FScanf(FI,myBuf,myHeader.offNomi+mySegDef.offNome);
  			if(!*myBuf)
  				fprintf(stderr,"WARN: Segm vuoto 1\n");

				i=PROCDclSeg(*myBuf ? myBuf : defaultCodeSegm,
					(enum SEGMENT_TYPES)mySegDef.tipo,0);		// (CMQ non arrivavano i tipi segmento nel file oggetto... 2024
  			i--;
  			if(debug)
  				printf("Segm: %s\n",myBuf);
  			fseek(FI,mySegDef.offCode,SEEK_SET);
  			fread(Seg[i].ptr+Seg[i].CurAddr,1,mySegDef.lunCode,FI);		// leggo i byte del codice
  			fseek(FI,mySegDef.offPublic,SEEK_SET);
	//	  		printf("vado a %lx\n",mySegDef.offPublic);
  			while(mySegDef.numPublic--) {
					struct OBJ_VAR ov;
				  fread(&ov,sizeof(struct OBJ_VAR),1,FI);
	  			l1=ftell(FI);
	//	  		printf("leggo a %lx\n",l3);
	  			FScanf(FI,myBuf,myHeader.offNomi+ov.address);
	  			if(FNCercaVar(myBuf))
	  				PROCError(2100,myBuf,__file__);
	  			if(Seg[i].type == SEGMENT_ABS)             // _abs segment
	  				PROCDclVar(myBuf,0,ov.value,i);
	  			else
	  				PROCDclVar(myBuf,0,ov.value+Seg[i].CurAddr,i);
	  			fseek(FI,l1,SEEK_SET);
  				}
  			Seg[i].CurAddr+=mySegDef.lunCode;
  			fseek(FI,l,SEEK_SET);
				}
			}
		fclose(FI);
		} while(s=strtok(NULL," "));

	if(OnlyShow)
		goto fine;
		
  if(s=strtok(LIBS," ")) do {
    if(s != LIBS)
      *(s-1)=' ';
    strcpy(__file__,s);
    AddExt(__file__,"LIB",0);
    FI=fopen(FNTrasfNome(__file__),"rb");
    if(!FI)
      PROCError(2003,__file__,NULL);
    if(Verbose)
      fprintf(stderr,"Lettura libreria %s...\n",__file__);
		fread(&myHeader,sizeof(struct OBJ_HEADER),1,FI);
		if(myHeader.v1 > 1)
		  PROCError(2061,__file__,NULL);
		while(myHeader.numSegs--) {
  		fread(&mySegDef,sizeof(struct OBJ_SEG_DEF),1,FI);
  		l=ftell(FI);
  		FScanf(FI,myBuf,myHeader.offNomi+mySegDef.offNome);

  		if(!*myBuf)
  		  fprintf(stderr,"WARN: Segm vuoto 2\n");

  		i=PROCDclSeg(*myBuf ? myBuf : defaultCodeSegm,(enum SEGMENT_TYPES)mySegDef.tipo,0);
  		i--;
  		if(debug)
    		printf("Segm: %s %d\n",myBuf,i);
  		fseek(FI,mySegDef.offCode,SEEK_SET);
  		fread(Seg[i].ptr+Seg[i].CurAddr,1,mySegDef.lunCode,FI);		// leggo i byte del codice LIB
  		fseek(FI,mySegDef.offPublic,SEEK_SET);
  		while(mySegDef.numPublic--) {		// ora scorro le variabili public/extern
				struct OBJ_VAR ov;
  		  fread(&ov,sizeof(struct OBJ_VAR),1,FI);
	  		l1=ftell(FI);
	  		FScanf(FI,myBuf,myHeader.offNomi+ov.address);
	  		if(FNCercaVar(myBuf))
	  		  PROCError(2100,myBuf,__file__);
	  		if(Seg[i].type == SEGMENT_ABS)             // _abs segment
	  		  PROCDclVar(myBuf,0,ov.value,i);
	  		else
	  		  PROCDclVar(myBuf,0,ov.value+Seg[i].CurAddr,i);
	  		fseek(FI,l1,SEEK_SET);
  		  }
  		Seg[i].CurAddr+=mySegDef.lunCode;
  		fseek(FI,l,SEEK_SET);
		  }
		fclose(FI);  
		} while(s=strtok(NULL," "));

  for(i=0; i<Segs; i++) {
		for(j=0; j<SegOverrides; j++) {
			if(!stricmp(Seg[i].label,segOverride[j].label)) {			// (ignorecase??)
				Seg[i].BaseAddr=segOverride[j].BaseAddr;
				break;
				}
			}
    }

  l=0;		// qua vado a sistemare uno dopo l'altro i segmenti con address non specificato..
  for(i=0; i<Segs; i++) {           // prendo il primo CODE definito (come fa lmbz80)
    if(Seg[i].type == SEGMENT_CODE) {
      l=Seg[i].BaseAddr+Seg[i].CurAddr;
      break;
      }
    }  
	for(i=0; i<Segs; i++) {
    Seg[i].len=Seg[i].CurAddr;
    if(Seg[i].type != SEGMENT_CODE) {
			if(Seg[i].BaseAddr == 0xffffffff)
				Seg[i].BaseAddr=l;
      l += Seg[i].CurAddr;
      }
    Seg[i].CurAddr=0;
    } 
 	if(RelocData) {  
	  l2=0;  
	  for(i=0; i<Segs; i++) {
 	    if(stricmp(Seg[i].label,defaultDataSegm))
  	    l2=max(l2,Seg[i].BaseAddr+Seg[i].len);   // calcolo il più alto per rilocare DATA
	    }  
	  i=FNCercaSeg(defaultDataSegm);
	  if(i) {
	    i--;
      l=Seg[i].BaseAddr;
	    }
	  else
	    PROCError(2200,NULL,NULL);  
    j=FNCercaSeg("_abs");
    PROCDclVar("__RELOC_RAM_SOURCE",0,l2,j-1);
    PROCDclVar("__RELOC_RAM_DEST",0,l,j-1);
    PROCDclVar("__RELOC_RAM_LEN",0,Seg[i].len,j-1);
 	  }    
  l2=0;
  for(i=0; i<Segs; i++) {
    l2=max(l2,Seg[i].BaseAddr+Seg[i].len);   // calcolo il più alto per avere RAMBOT (malloc)
    }
  PROCDclVar("__RAM_BOT",0,l2+1,FNCercaSeg("_abs")-1);
  PROCDclVar("__RAM_TOP",0,DataSize-1,FNCercaSeg("_abs")-1);

  if(s=strtok(INS," ")) do {
    strcpy(__file__,s);
 		AddExt(__file__,"O",0);
   	if(Verbose)
      fprintf(stderr,"Rilocazione oggetto %s...\n",__file__);
  	FI=fopen(FNTrasfNome(__file__),"rb");
	  if(!FI)
	    PROCError(2002,__file__,NULL);
		fread(&myHeader,sizeof(struct OBJ_HEADER),1,FI);
		if(myHeader.v1 > 1)
		  PROCError(2061,__file__,NULL);

// creo una copia dei segm, privata di ciascun oggetto, per far corrispondere i segmenti
//  privati con quelli globali
		for(i=0; i<myHeader.numSegs; i++) {
  		fread(&mySegDef,sizeof(struct OBJ_SEG_DEF),1,FI);
  		l=ftell(FI);
  		FScanf(FI,tSeg[i].label,myHeader.offNomi+mySegDef.offNome);
			if(!*tSeg[i].label) {
				strcpy(tSeg[i].label,defaultCodeSegm);
  		  fprintf(stderr,"WARN: Segm vuoto label %u\n",i);
				}
  		j=FNCercaSeg(tSeg[i].label);
  		if(j>=0)
				j--;
			else
 		    PROCError(1000,NULL,tSeg[i].label);		// verificare 2024

  		tSeg[i].BaseAddr=Seg[j].BaseAddr+Seg[j].CurAddr;   // salvo qui la fine segm PRIMA ... boh 2024?
//  		  printf("Segm: %s\n",tSeg[i].label);
			for(j=0; j<SegOverrides; j++) {		// ma serve davvero?? ho già fatto cercaseg sopra...
				if(!stricmp(tSeg[i].label,segOverride[j].label)) {			// ignorecase??
					tSeg[i].BaseAddr=segOverride[j].BaseAddr;
					break;
					}
				}

  		fseek(FI,l,SEEK_SET);
  		}
		fseek(FI,sizeof(struct OBJ_HEADER),SEEK_SET);

		while(myHeader.numSegs--) {
  		fread(&mySegDef,sizeof(struct OBJ_SEG_DEF),1,FI);
  		l=ftell(FI);
  		FScanf(FI,myBuf,myHeader.offNomi+mySegDef.offNome);

  		if(!*myBuf)
  		  fprintf(stderr,"WARN: Segm vuoto 3\n");

			i=FNCercaSeg(*myBuf ? myBuf : defaultCodeSegm);
  		i--;
  		if(debug)
  		  printf("Segm: %s\n",myBuf);

  		fseek(FI,mySegDef.offReloc,SEEK_SET);
  		while(mySegDef.lunReloc) {
				struct OBJ_RELOC or;
  		  fread(&or,sizeof(struct OBJ_RELOC),1,FI);
	  		l1=ftell(FI);
	 		  j=LOBYTE(or.type);
    		if(HIBYTE(or.type)==0) {
//	  		  printf("riloc. normale dal seg %x a %lx\n",j,l3);
//	  		  j=FNCercaSeg(tSeg[j].label);
//	  		  if(!j)
//	  		    PROCError(1000,NULL);
//	  		  else
//	  		    j--;  
//  		  printf("cerco Segm: %s  ",Seg[j].label);
//  		  printf("la sua Base è %lx\n",tSeg[j].BaseAddr);
	  		  *(uint16_t *)(Seg[i].ptr+Seg[i].CurAddr+or.address) += tSeg[j].BaseAddr;
	  		  }
	  		else {
      		fseek(FI,mySegDef.offExtern+(j << 3),SEEK_SET);
//      		printf("vado a %lx\n",ftell(FI));

// anche qua? cosa sono? 2024  		  fread(&or,sizeof(struct OBJ_RELOC),1,FI);
					struct OBJ_VAR ov;
					fread(&ov,sizeof(struct OBJ_VAR ),1,FI);     
  	  		FScanf(FI,myBuf,myHeader.offNomi+ov.address);
//	  		  printf("riloc. extern %s a %lx\n",myBuf,l3);
		  		if(V=FNCercaVar(myBuf)) {
		  		  j=HIWORD(V->value);
//  		  printf("cerco Segm: %s, var= %lx\n",tSeg[j].label,V->value);
//	  		    j=FNCercaSeg(tSeg[j].label);
//		  		  if(!j)
//		  		    PROCError(1000,NULL);
//		  		  else
//		  		    j--;  
						if(Seg[j].type == SEGMENT_ABS)
  	  		    *(uint16_t *)(Seg[i].ptr+Seg[i].CurAddr+or.address)=LOWORD(V->value);
						else
  	  		    *(uint16_t *)(Seg[i].ptr+Seg[i].CurAddr+or.address)=Seg[j].BaseAddr+ LOWORD(V->value);

		  		  }
		  		else  
		  		  PROCError(2101,myBuf,__file__);
		  		}  
	  		fseek(FI,l1,SEEK_SET);
	  		mySegDef.lunReloc -= 8;
  		  }
      Seg[i].CurAddr+=mySegDef.lunCode;
  		fseek(FI,l,SEEK_SET);
		  }
		fclose(FI);
		} while(s=strtok(NULL," "));
		
  if(s=strtok(LIBS," ")) do {
    strcpy(__file__,s);
    AddExt(__file__,"LIB",0);
    FI=fopen(FNTrasfNome(__file__),"rb");
    if(!FI)
      PROCError(2003,__file__,NULL);
    if(Verbose)
      fprintf(stderr,"Rilocazione libreria %s...\n",__file__);
		fread(&myHeader,sizeof(struct OBJ_HEADER),1,FI);
		if(myHeader.v1 > 1)
		  PROCError(2061,__file__,NULL);
		
		for(i=0; i<myHeader.numSegs; i++) {
  		fread(&mySegDef,sizeof(struct OBJ_SEG_DEF),1,FI);
  		l=ftell(FI);
  		FScanf(FI,tSeg[i].label,myHeader.offNomi+mySegDef.offNome);
  		j=FNCercaSeg(tSeg[i].label);
  		j--;
  		tSeg[i].BaseAddr=Seg[j].BaseAddr+Seg[j].CurAddr;   // salvo qui la fine segm PRIMA
//  		  printf("Segm: %s\n",tSeg[i].label);
  		fseek(FI,l,SEEK_SET);
  		}
		fseek(FI,sizeof(struct OBJ_HEADER),SEEK_SET);

		while(myHeader.numSegs--) {
  		fread(&mySegDef,sizeof(struct OBJ_SEG_DEF),1,FI);
  		l=ftell(FI);
  		FScanf(FI,myBuf,myHeader.offNomi+mySegDef.offNome);
  		i=FNCercaSeg(myBuf);
  		i--;
  		if(debug)
  		  printf("Segm: %s\n",myBuf);
  		fseek(FI,mySegDef.offReloc,SEEK_SET);
  		while(mySegDef.lunReloc) {
//				struct OBJ_RELOC or;
				// usare come sopra...
//  		  fread(&or,sizeof(struct OBJ_RELOC),1,FI);
  		  fread(&l2,4,1,FI);
  		  fread(&l3,4,1,FI);
	  		l1=ftell(FI);
	 		  j=LOBYTE(LOWORD(l2));
    		if(HIBYTE(LOWORD(l2))==0) {
//	  		  printf("riloc. normale dal seg %x a %lx\n",j,l3);
//	  		  j=FNCercaSeg(tSeg[j].label);
//	  		  if(!j)
//	  		    PROCError(1000,NULL);
//	  		  else
//	  		    j--;  
//  		  printf("cerco Segm: %s  ",Seg[j].label);
//  		  printf("la sua Base è %lx\n",tSeg[j].BaseAddr);
	  		  *(int16_t *)(Seg[i].ptr+Seg[i].CurAddr+l3) += tSeg[j].BaseAddr;
	  		  }
	  		else {
      		fseek(FI,mySegDef.offExtern+(j << 3),SEEK_SET);
//      		printf("vado a %lx\n",ftell(FI));
					struct OBJ_VAR ov;
					// usare come sopra, provare...

    		  fread(&l2,4,1,FI);                    // boh?
    		  fread(&l2,4,1,FI);
  	  		FScanf(FI,myBuf,myHeader.offNomi+l2);
//	  		  printf("riloc. extern %s a %lx\n",myBuf,l3);
		  		if(V=FNCercaVar(myBuf)) {
		  		  j=*(((int16_t *)&V->value)+1);
//  		  printf("cerco Segm: %s, var= %lx\n",tSeg[j].label,V->value);
//	  		    j=FNCercaSeg(tSeg[j].label);
//		  		  if(!j)
//		  		    PROCError(1000,NULL);
//		  		  else
//		  		    j--;  
						if(Seg[j].type == SEGMENT_ABS)
  	  		    *(int16_t *)(Seg[i].ptr+Seg[i].CurAddr+l3)=LOWORD(V->value);
						else
  	  		    *(int16_t *)(Seg[i].ptr+Seg[i].CurAddr+l3)=Seg[j].BaseAddr+ LOWORD(V->value);

		  		  }
		  		else  
		  		  PROCError(2101,myBuf,__file__);
		  		}  
	  		fseek(FI,l1,SEEK_SET);
	  		mySegDef.lunReloc -= 8;
  		  }
      Seg[i].CurAddr+=mySegDef.lunCode;
//      printf("la lunghezza di %s è ora %lx\n",Seg[i].label,Seg[i].CurAddr);
  		fseek(FI,l,SEEK_SET);
		  }
		fclose(FI);  
		} while(s=strtok(NULL," "));
		
#if ARCHI
  PROCPrint(FO,"ALIGN");
  PROCPrint(FO,".__argv EQUS STRING$("+STR$EnvironSize%+",CHR$0)");
	if(StackSize)
		PROCPrint(FO,".__StackSize DCD &"+STR$~StackSize%);
  PROCPrint(FO,".__osversion");
  PROCPrint(FO,".__osminor DCB 0");
  PROCPrint(FO,".__osmaior DCB 2");
  PROCPrint(FO,"DCW 0");
  PROCPrint(FO,"]");
  PROCPrint(FO,"NEXT");
  PROCPrint(FO,"CALL _main");
#elif Z80
  if(Verbose)
 	  fprintf(stderr,"Scrittura eseguibile %s...\n",OUS);
  for(i=0; i<Segs; i++) {

//#pragma warning qua si potrebbe pensare di avere un flag che mette i segmenti in sequenza, se non specificati gli indirizzi...
// ma sembra già a posto ora, 12/2/24 

	  for(j=0; j<Segs; j++) {
	    if(i!=j && (((Seg[i].BaseAddr > Seg[j].BaseAddr) && (Seg[i].BaseAddr < Seg[j].BaseAddr+Seg[j].len)) || 
				((Seg[i].BaseAddr+Seg[i].len > Seg[j].BaseAddr) && (Seg[i].BaseAddr+Seg[i].len < Seg[j].BaseAddr+Seg[j].len)))) {
				sprintf(myBuf,"%s e %s",Seg[i].label,Seg[j].label);
	      PROCWarn(4010,myBuf,NULL);
				}
	    }
	  }
  if(*MAPS) {                     // prima di modificare DATA
		AddExt(MAPS,"MAP",1);
		PROCV(MAPS);
    }
	
 	if(RelocData) {  
	  i=FNCercaSeg(defaultDataSegm)-1;      // esiste di sicuro, v. sopra
    V=FNCercaVar("__RELOC_RAM_SOURCE");
    Seg[i].BaseAddr=(uint16_t)V->value;
    }
 	if(HexFormat) {
 	  for(i=0; i<Segs; i++) {
 	    if(!Seg[i].len)
 	      continue;
// 	    printf("questo %s comincia a %x\n",Seg[i].label,Seg[i].BaseAddr);
 	    ptr=Seg[i].ptr;
 	    l2=Seg[i].BaseAddr;

rifoSeg: ; 	      
 	    fputc(':',FO);
 	    l=ftell(FO);
 	    fprintf(FO,"00%04X00",(uint16_t)l2);
//  		printf("scrivo...\n");
 	    t=LOBYTE(l2);
 	    t += HIBYTE(l2);
 	    for(j=0; j<0x20; j++) {
 	      ch=*ptr;
  	    if((ptr-Seg[i].ptr) >= Seg[i].len)
  	      break;
 	      t=t+ch;
  	    fprintf(FO,"%02X",LOBYTE(ch));
  	    ptr++;
 	      }
  	  t+=j;
  	  l2+=j;
  	  fprintf(FO,"%02X\n",-t & 0xff);
 	    fseek(FO,l,SEEK_SET);
 	    fprintf(FO,"%02X",j);
 	    fseek(FO,0,SEEK_END);
  	  if((ptr-Seg[i].ptr) < Seg[i].len)
  	    goto rifoSeg;
 	    }
    strcpy(myBuf,"_main");		// da QUANDO non si può fare strupr su costante??? 2024 @#£$%
    if(V=FNCercaVar(myBuf)) {                 // crea record di start...
      j=HIBYTE(LOWORD(V->value));               // _main, se c'è...
      i=V->value+Seg[j].BaseAddr;
      printf("scrivo i %d, j %d\n",i,j);
      }
    else if(i=FNCercaSeg(defaultCodeSegm)) {             // ...oppure l'inizio di text
      i=Seg[i-1].BaseAddr;
      }
    else 
      goto noStart;  

		// mah sta cosa non so... specie per non-C (2024)
    fprintf(FO,":02000004%04X%02X\n",i,-(6+LOBYTE(i)+HIBYTE(i)) & 0xff);


noStart:
    fprintf(FO,":00000001FF\n");
 	  }		// HEXformat
 	else {
	  l3=0x0;  
	  l2=0xfffff;  
		for(i=0; i<Segs; i++) {
			if(Seg[i].len) {
				l2=min(l2,Seg[i].BaseAddr);   // calcolo il più basso per file BINario..
				l3=max(l3,Seg[i].BaseAddr+Seg[i].len);   // ..e il più alto
				}
			}  
		if(FillMemory) {
			l2=0;
			}
    while(l3-- > l2)
    	fputc(0xff,FO);   // ci tengo ad avere i byte non usati a FF!
    fseek(FO,0,SEEK_SET);
    
 	  for(i=0; i<Segs; i++) {
 	    if(Seg[i].type == SEGMENT_CODE ||
				(!IgnoreRAM || ((Seg[i].type & 0x700) != 0x100))) {		// 	SEGMENT_BSS SEGMENT_CONST
				/*|| (Seg[i].BaseAddr < 0x8000)*/               // scrive solo i segm in ROM per SKYNET... tolto 2024 :)
	 	    fseek(FO,Seg[i].BaseAddr-l2,SEEK_SET);
//	  		printf("scrivo...%d a %lx per %lx\n",i,Seg[i].BaseAddr-l2,Seg[i].len);
	 	    fwrite(Seg[i].ptr,1,Seg[i].len,FO);
	 	    }
 	    }
 	  }  
//  while(i--)
//    fputc(*ptr,FO);
#endif
  fclose(FO);
		
fine:
	fputc('\n',stderr);

	for(i=0; i<MAX_SEGS; i++)
    if(Seg[i].ptr)
			free(Seg[i].ptr);


#ifdef _DEBUG
	while(!kbhit());
#endif

  return 0;
  } 
	  
 
struct VARS *FNCercaVar(char *N) {
  register int i;
  struct VARS *V;

//  PROCV();
  if(IgnoreCase) 
    strupr(N);
  
	V=Var;
	for(;;) {
  	if(!V)
	    return 0;
//	    printf("esamino %Fs\n",V->name);
    i=strcmp(N,V->name);
    if(!i)
      return V;
		if(i<0)
		  V=V->left;         /* Esegue la ricerca */
		else
		  V=V->right;
		}

//        printf("Cercavar\a: %s <> %Fs\n",N,*V->name);
  return 0;
  }
 
struct VARS *PROCDclVar(const char *N, int Type, int Value, int Segm) {
  struct VARS *V;
  char MyBuf[256],MyBuf1[256];
  
  V=(struct VARS *)malloc(sizeof(struct VARS)); 
  strncpy(V->name,N,31);
	if(IgnoreCase)
	  strupr(V->name);
  Var=PutOnTree(Var,Var,V);
  LVars=V;
 
  if(debug)  
    printf("Alloco: %s, value %lx\n",N,Value);

  V->type=Type;
//  V->value=Value;
  *((int16_t *)&V->value)=Value;
  *(((int16_t *)&V->value)+1)=Segm;
	V->size=0;
	V->lpos=0;
	V->extUsed=0;

  nRoot=PutOnTree2(nRoot,nRoot,V);
  return V;
  }

int FNCercaSeg(const char *n) {
  int i;

  for(i=0; i<Segs; i++) {
    if(IgnoreCase && !stricmp(n,Seg[i].label)) { 
      return i+1;
      }
    if(!IgnoreCase && !strcmp(n,Seg[i].label)) { 
      return i+1;
      }
    }
  return 0;
  }
 
int PROCDclSeg(const char *nome,enum SEGMENT_TYPES t,signed char f) {        // f=1 per data, 0 per code MA NON USATO 2024!
  int v,T1;
  
//  fprintf(stderr,"\tcerco SEG %s\n",nome);

  T1=FNCercaSeg(nome);     // SE GIA' ESISTE...
  if(!T1) {
	  v=Segs++; 
		if(Segs >= MAX_SEGS)
		  PROCError(1000,nome,NULL);
		strncpy(Seg[v].label,nome,MAX_NAME_LEN);
//		Seg[v].label[MAX_NAME_LEN]=0;
		if(IgnoreCase)
		  strupr(Seg[v].label);
		Seg[v].BaseAddr=0xffffffff;
		Seg[v].type=t;
		if(f)
		  Seg[v].type |= 1;
    if(!(Seg[v].ptr=(uint8_t *)malloc(t == SEGMENT_CODE ? CodeSize : DataSize)))
      PROCError(1001,NULL,NULL);
		return v+1;
		}
  else
    return T1;  

 // fprintf(stderr,"\tcreo SEG %d: %s = %lx\n",v,nome,Var[v].value);
  }

 

int PROCInit() {
  int t;
						   
  _strdate(__date__);
  t=__date__[3];
  __date__[3]=__date__[0];
  __date__[0]=t;
  t=__date__[4];
  __date__[4]=__date__[1];
  __date__[1]=t;
  _strtime(__time__);
#if ARCHI
  fprintf(stderr,"The G.Dar C.Linker for the Archimedes on PC, (C) 1989-1993 - Version %d.%02d\n\n",__VER__ / 256,__VER__ & 255);
#elif Z80
  fprintf(stderr,"Linker di G.Dar (C) 1989-2024 - Versione %d.%02d\n\n",__VER__ / 256,__VER__ & 255);
#endif

	strcpy(defaultCodeSegm,"TEXT");
	strcpy(defaultDataSegm,"DATA");
//	strcpy(defaultDataSegm,"BSS");		// aggiungere... v. AS
  
  return 0;                        
  }


char *FNTrasfNome(char *A) {
  char *T,B[256];
  
  if(ANSI)
    return A;
  strcpy(B,A);
  if(ACORN) {
    T=strchr(B,'.');
    if(T) { 
      strcpy(A,T+1);
      strcat(A,".");
      strncat(A,B,T-B-1);
      return A;
      }
    else 
      return A;
    }
  if(GD) {
    T=strchr(A,'.');
    if(T) {   
      *T='_';
      return A;
      }
    else 
      return A;
    }      
                    
  return A;
  }

int PROCError(int Er, const char *a, const char *f) {
  int i;
  char myBuf[256];

  if(f)
  	printf("%s: ",f);
  printf("errore %d: ",Er);
  i=0;
  while (Er != Errs[i].t && Errs[i].t)
    i++;
  if(Errs[i].t) {
  	printf("%s %s\n",Errs[i].s,a ? a : ".");
	  if(debug) {
	    PROCV("vartmp.map");
//	    PROCT();
	    }
    }
	else	{
	  sprintf(myBuf,"(%d)",Er);
    PROCError(1000,myBuf,NULL);
    }
  
  exit(2);
  return 0;
  }

int PROCWarn(int Er, const char *a, const char *f) {
  int i;

  if(f)
  	printf("%s: ",f);
  printf("warning %d: ",Er);
  i=0;
  while (Er != Errs[i].t && Errs[i].t)
    i++;
  if(Errs[i].t) {  
  	printf("%s %s\n",Errs[i].s,a ? a : ".");
	  }
	else	
    PROCError(1000,NULL,NULL);
    
  return 0;
  }

void InOrderVar(FILE *FO,struct VARS *V) {

	if(!V)
	  return;

	InOrderVar(FO,V->left);
  fprintf(FO,"\t%-32s\t%4x\t%04X:%04X\n",V->name,V->type,HIWORD(V->value),LOWORD(V->value));
	InOrderVar(FO,V->right);
	}

void InOrderVar2(FILE *FO,struct VARS *V) {

	if(!V)
	  return;

	InOrderVar2(FO,V->l1);
  fprintf(FO,"\t%-32s\t%4x\t%04X:%04X\n",V->name,V->type,HIWORD(V->value),LOWORD(V->value));
	InOrderVar2(FO,V->r1);
	}

int PROCV(char *n) {
  int t,i;
  char *B;
  FILE *FO;
  struct VARS *V;
  
  B=FNTrasfNome(n);
  FO=fopen(B,"w");
  if(!FO) 
    PROCError(2004,OUS,NULL);
//  FO=stderr;  
  fprintf(FO,"Generated by G.Dar linker v.%d.%02d on %s %s\n\n",__VER__ / 256,__VER__ & 255,__date__,__time__);
  fprintf(FO,  "               %-34s\t%-6s\t%-6s\t%-6s\n","Nome","Tipo","Base","Lunghezza");
  for(t=0; t<Segs; t++) {
    fprintf(FO,"Segmento (%02X): %-32s\t%6X\t  %06lX\t  %06lX\n",t,Seg[t].label,Seg[t].type,Seg[t].BaseAddr,Seg[t].len);
    }
  fputc('\n',FO);
    
  fprintf(FO,"\t%-32s\t%4s\t%8s\n","Oggetto (ordinati per nome)","Tipo","Valore");
	InOrderVar(FO,Var);
  fputc('\n',FO);
  fprintf(FO,"\t%-32s\t%4s\t%8s\n","Oggetto (ordinati per valore)","Tipo","Valore");
	InOrderVar2(FO,nRoot);
  fputc('\n',FO);

	if(StackSize)
	  fprintf(FO,"Dimensione dello Stack: %04x bytes\n",StackSize);
 	if(RelocData) {
 	  i=FNCercaSeg(defaultDataSegm)-1;
    V=FNCercaVar("__RELOC_RAM_SOURCE");
    fprintf(FO,"Segmento %s rilocato a %04x\n",Seg[i].label,(int)V->value);
 	  }
  fclose(FO);
  return 0;
  }

struct VARS *PutOnTree(struct VARS *root,struct VARS *r,struct VARS *n) {
  int i;
	/* Usuale algoritmo di inserimento Inordine */

//	puts("Sto inserendo");
	if(!r) {
		n->left=0;
		n->right=0;
		if(root) {
			if(strcmp(n->name,root->name)<0)
				root->left=n;
			else
			  root->right=n;
			}
		return n;
		}
	i=strcmp(n->name,r->name);
	if(i<0) {
	  PutOnTree(r,r->left,n);  /* a sinistra < */
	  }
	else {
	  if(i>0) {
		  PutOnTree(r,r->right,n); /* a destra   > */
		  }
		else {  
		  PROCError(1001,NULL,NULL);
			}
		}

	return root;
	}

struct VARS *PutOnTree2(struct VARS *root,struct VARS *r,struct VARS *n) {
  int i;
	/* Come sopra, ma in ordine di valore */

//	puts("Sto inserendo");
	if(!r) {
		n->l1=0;
		n->r1=0;
		if(root) {
			if(n->value < root->value)
				root->l1=n;
			else
			  root->r1=n;
			}
		return n;
		}
	if(n->value < r->value) {
	  PutOnTree2(r,r->l1,n);  /* a sinistra < */
	  }
	else {
	  if(n->value > r->value) {
		  PutOnTree2(r,r->r1,n); /* a destra   > */
		  }
		else {  
		  PROCError(1001,NULL,NULL);
			}
		}

	return root;
	}


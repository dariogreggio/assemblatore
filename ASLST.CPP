#include <stdlib.h>
#include <string.h>
#include <conio.h>
//#include <windows.h>
#include "as.h"

extern char OUS[256];
#define ANSI  TRUE
#define ACORN FALSE
#define GD 		FALSE 	      // CONVENZIONI SUI NOMIFILE
extern signed char WarnLevel;
extern uint8_t CPULevel;
extern uint8_t inMacroExecuting,macroLineNumber;
extern struct LINE_DEF *CurrMacro;
FILE *FErr;

uint16_t PageLength=66;
uint16_t LineLength=80;        // in realta non e' ancora usato

struct ERRORE Errs[]={
#if PIC==16 || PIC==18
  302,"register not in bank0, check page bits",0x0,
#endif
  1000,"errore interno sconosciuto - contattare ADPM",0x10,
  1001,"errore interno (fine memoria)",0x10,
  1004,"EOF inaspettato",0x10,
  1006,"non atteso",0,
  1007,"non implementato",0,
  1013,"impossibile aprire il file sorgente",0x10,
  1015,"impossibile aprire il file include",0x10,
  1017,"troppi livelli #if/#endif",0x10,
  1018,"#elif inatteso",0x10,
  1019,"#else inatteso",0x10,
  1020,"#endif inatteso",0x10,
  1021,"bad preprocessor command",0,
  1022,"#endif atteso",0x10,
  1035,"espressione troppo complessa, semplificarla",0,
  1037,"impossibile aprire il file oggetto",0x10,
  1038,"impossibile aprire il file",0x10,
  2007,"#define syntax",0,
  2010,"invalid formal list",0,
  2011,"macro parameter",0,
  2012,"macro redefinition",0,
  2015,"too many chars in costant",0,
  2016,"costante errata",0,
  2017,"sequenza di escape illegale",0,
  2036,"left operand must have struct/union type",0,
  2038,"not struct/union member",0x10,
//  2039,"-> requires struct/union pointer",0x10,
  2040,". requires struct/union name",0x10,
  2054," atteso",0,
  2057,"attesa costante",0,
  2058,"valore troppo grande",0,
  2060,"errore di sintassi",0x10,
  2061,"codice operativo sconosciuto",0x10,
  2062,"operando errato",0x10,
  2063,"salto relativo fuori portata",0,
  2064,"dimensione operando sbagliata",0,
  2065,"non definito",0x10,
  2066,"simbolo PUBLIC non definito",0x10,
#ifndef I8086
  2067,"istruzione estesa o non standard",0,
#else
	2067,"non valida per questa CPU",0,
#endif
  2068,"tipo di indirizzamento errato",0x10,
  2069,"inizializzando",0,
  2070,"non standard",0,
  2071,"operando mancante",0x10,
  2079,"uses undefined struct/union",0x10,
  2086,"ridefinito",0,
  2100,"manca ENDP o ENDC",0,
  2101,"emissione di dati al di fuori di un segmento",0,
  2102,"emissione di codice in un segmento dati",0,
  2103,"emissione di dati in un segmento BSS",0,
  2104,"sovrascrittura byte",0,
  2108,"errore nella nidificazione dei blocchi",0,
  2109,"PROCedure nidificate",0,
  2115,"tipi incompatibili",0,
  2137,"costante carattere vuota",0,
  4002,"flag sconosciuto e ignorato",0,
  4010,"compattazione di salto assoluto",0,
  4011,"estensione di salto relativo",0,
  4012,"indirizzamento istruzione modificato",0,
  4013,"operando troncato",0,
  4099,"" /*"user-defined"*/,0,
  -1,NULL,0
  };

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

int PROCError(int Er, int32_t n) {
  char myBuf[128];

	sprintf(myBuf,"%d",n);
	return PROCError(Er, myBuf);
	}
int PROCError(int Er, const char *a) {
  int i;
  char myBuf[128];

//	if(a[strlen(a)-1]=='\r')
//		a[strlen(a)-1]=0;


  if(OutError && FIn) {    // solo se il file source e' realmente aperto...
    fputs(theLine,stderr);
    fputc('\n',stderr);
		if(FErr) {
	    fputs(theLine,FErr);
		  fputc('\n',FErr);
			}
    }
  fprintf(stderr,"%s: (%d): errore %d ",__file__,__line__,Er);
	if(FErr) {
	  fprintf(FErr,"%s: (%d): errore %d ",__file__,__line__,Er);
		}
  i=0;
  while (Er != Errs[i].t && Errs[i].t)
    i++;
  fputs(Errs[i].s,stderr);
	if(FErr) {
	  fputs(Errs[i].s,FErr);
		}
  if(a && *a)
    fprintf(stderr,": %s",a);
	if(CurrMacro) {
		if(FErr) {
	    fprintf(FErr," (macro linea %u)",macroLineNumber);
			}
    fprintf(stderr," (macro linea %u)",macroLineNumber);
		}
  fputc('\n',stderr);
	if(FErr)
	  fputc('\n',FErr);
  
  if(FMap)
    fclose(FMap);
  if(FCod) {
    PROCV(FCod);
    fclose(FCod);
    }
  if(FPre)
    fclose(FPre);
  if(FObj) {
#if PIC==16 || PIC==18
		extern uint8_t outCnt,oldT;
		if(!(outCnt & 1)) {		// ev. residuo dispari 16bit :) brutto ma ok
			sprintf(myBuf," %02X",oldT);
		  PROCOut1(myBuf,NULL,TRUE);
			}
#endif
#if GD24032
		extern uint8_t outCnt,oldT,oldT2,oldT3;
		if(!(outCnt & 1)) {		// ev. residuo dispari 32bit :) brutto ma ok
			sprintf(myBuf," %02X",oldT);
		  PROCOut1(myBuf,NULL,TRUE);
			}
		outCnt--;
		if(!(outCnt & 1)) {		// ev. residuo dispari 32bit :) brutto ma ok
			sprintf(myBuf," %02X",oldT);
		  PROCOut1(myBuf,NULL,TRUE);
			}
		outCnt--;
		if(!(outCnt & 1)) {		// ev. residuo dispari 32bit :) brutto ma ok
			sprintf(myBuf," %02X",oldT);
		  PROCOut1(myBuf,NULL,TRUE);
			}
#endif

    fclose(FObj);
    remove(OUS);
    }

#ifdef _DEBUG  
	while(!kbhit());
#endif

  exit(2); 
  
  return 0;
  }

int PROCWarn(int Er, const char *a) {
  int i;
  char myBuf[256];

  i=0;
  while(Er != Errs[i].t && Errs[i].t)
    i++;
  if(Errs[i].liv >= WarnLevel) {
	  sprintf(myBuf,"%s: (%d): warning %d: %s %s\n",__file__,__line__,Er,Errs[i].s,a ? a : ".");
//		if(myBuf[strlen(myBuf)-1]=='\n')
//			myBuf[strlen(myBuf)-1]=0;
	  fprintf(stderr,myBuf);
	  PROCOut1(myBuf,NULL,1);     // mette i warning in file lst
		if(FErr)
			fprintf(FErr,myBuf);
	  }
  
  return 0;
  }

void InOrderVar(FILE *FO,const struct VARS *V,uint32_t filter=0);
void InOrderVar(FILE *FO,const struct VARS *V,uint32_t filter) {
	const struct VARS *V2;

	if(!V)
	  return;

	InOrderVar(FO,V->left,filter);
	V2=V;
	while(V2) {
		if(filter) {
			if(!(V2->type & filter))
				goto skippa;
			}
	#if GD24032
		fprintf(FO,"\t%-32s\t%10X:%08LX\t%6X\t%6X\t-8%s\t-8%s",V2->name,V2->segm & 0xffff,LODWORD(V2->value),		// BUG C finire!
	//  fprintf(FO,"\t%-32s\t%2X:%06LX\t%4X\t%4X\t  %s\n",V->name,V->segm,V->value,		// credo si schianti qua se value è 64 bit...
	#else
		fprintf(FO,"\t%-32s\t%10X:%08X\t%6X\t%6X\t%-8s\t%-8s",V2->name,V2->segm & 0xffff,V2->value,
	#endif
			V2->type,V2->size,
			V2->tag ? ((struct VARS *)(V2->tag))->name : "",
			V2->proc ? V2->proc->name : ((char *)""));
		if(V2->type & VAR_FUNZ)
			fprintf(FO,"\n\t-\t%-32s\t%15X\t%12s\n",
				V2->name,V2->dim,V2->type & VAR_PUBLIC ? "public" : "");
		else
			fprintf(FO,"%12s\n",V->type & VAR_PUBLIC ? "public" : "");
		if(V2->type & VAR_STRING)
			fprintf(FO,"\t\t\"%s\"\n",
				V2->value ? (const char*)V2->value : "");

skippa:
		V2=V2->next;
		}

	InOrderVar(FO,V->right,filter);
	}

uint16_t InOrderVar2(struct VARS **V2,const struct VARS *V) {
	static uint16_t n=0;

	if(!V)
	  return 0;

	InOrderVar2(V2,V->left);
	*V2=(struct VARS *)realloc(*V2,sizeof(struct VARS)*(n+1));
	memcpy(*V2+n,V,sizeof(struct VARS));
	((*V2)+n)->next=NULL;
	n++;
	InOrderVar2(V2,V->right);

	return n;
	}

void InOrderAggr(FILE *FO,const struct TAGS *V,uint32_t filter=0);
void InOrderAggr(FILE *FO,const struct TAGS *V,uint32_t filter) {
	const struct TAGS *V2;
	uint8_t level;

	if(!V)
	  return;

	InOrderAggr(FO,V->left,filter);
	V2=V;
	level=0;
	while(V2) {
		if(filter) {
			if(!(V2->type & filter))
				goto skippa;
			}
	#if GD24032
		fprintf(FO,"\t%s%-32s\t%6X\t% 8X\t% 5u\t% 5X\t%-10s\n",level>0 ? "- " : "",V2->name,		// BUG C finire!
	//  fprintf(FO,"\t%-32s\t%2X:%06LX\t%4X\t%4X\t  %s\n",V->name,V->segm & 0xffff,V->value,		// credo si schianti qua se value è 64 bit...
	#else
		fprintf(FO,"\t%s%-32s\t%6X\t% 8X\t% 5u\t% 5X\t%-10s\n",level>0 ? "- " : "",V2->name,
	#endif
//		fprintf(FO,"\t%-32s\t%s\n",T2->name,T2->tipo ? "Record" : "Struc");
			V2->type,V2->size,V2->pos,V2->value,
			V2->tag ? ((struct TAGS *)(V2->tag))->name : ""
			);

skippa:
		level++;
		V2=V2->next;
		}

	InOrderAggr(FO,V->right,filter);
	}

int PROCV(FILE *FO) {
  int t,i;
  struct VARS *V;
  struct LINE_DEF *T;
  struct TAGS *T2;

  fputc('\n',FO);
  fprintf(FO,"\n\n%-46s\t%-12s\t%-4s\t%-6s\t%-8s\n",
		"Variabili e label (per nome):","Valore","Tipo","Size","Parent","Proc");
  InOrderVar(FO,Var);
//  V=Var;
//  while(V) {
//    fprintf(FO,"\t%-32Fs\t%10lx\t%4x\t%4x\t%Fs\n",V->name,V->value,V->type,V->size,V->proc ? V->proc->name : ((char far *)""));
//    V=V->next;
//    }      
//  fputc('\n',FO);
  /* messo sopra 
	V=Var;
  while(V) {
    if(V->type & 0x100)
      fprintf(FO,"Proc: %-32s\t%10s\t%4X\n",
				V->name,V->type & 0x200 ? "public" : "",V->dim);
    V=V->next;
    }*/
//  fputc('\n',FO);
	PROCV2(FO);

  fprintf(FO,"\nSegmenti\t%36s\t%4s\t%5s\t%12s\n","Tipo","Align","Base","Dimensione");
  for(t=0; t<Segs; t++) {
//    if(CurAddr[t]>0)                        // se il segmento è usato
#if I8086
		if(CPULevel<3) {
#endif
			fprintf(FO,"\t(%02X): %-32s\t%04X\t%02u\t  %04X:%04lX\t  %06lX\n",t+1,Seg[t].name,Seg[t].type,Seg[t].align,
				HIWORD(Seg[t].BaseAddr==0xffffffff ? 0 : Seg[t].BaseAddr),
				LOWORD(Seg[t].BaseAddr==0xffffffff ? 0 : Seg[t].BaseAddr),
	//			Seg[t].BaseAddr==0xffffffff ? Seg[t].len : Seg[t].len-Seg[t].BaseAddr);
				Seg[t].len);
#if I8086
			}
		else {
			fprintf(FO,"\t(%02X): %-32s\t%04X\t%02u\t  %04X:%08lX\t  %06lX\n",t+1,Seg[t].name,Seg[t].type,Seg[t].align,
				HIWORD(Seg[t].BaseAddr==0xffffffff ? 0 : Seg[t].BaseAddr),
				LOWORD(Seg[t].BaseAddr==0xffffffff ? 0 : Seg[t].BaseAddr),
	//			Seg[t].BaseAddr==0xffffffff ? Seg[t].len : Seg[t].len-Seg[t].BaseAddr);
				Seg[t].len);
			}
#endif
    }
//  fputc('\n',FO);
  
  fprintf(FO,"\nStruc/Record\t%30s\t%6s\t%7s\t%5s\t%6s\n","Tipo","Dimensione","Offset","Valore","Parent");
  InOrderAggr(FO,StrTag);

  fprintf(FO,"\nDefine\n");
  T=RootDef;
  while(T) {
	  fprintf(FO,"\t%-32s\t%-32s\n",T->name,T->text);
    T=T->next;
    }

  fprintf(FO,"\nMacro\n");
  T=RootMacro;
  while(T) {
	  fprintf(FO," ***MACRO START\n");
	  fprintf(FO," -\t%s\n",T->name);
	  fprintf(FO,"    \t%s\n",T->text);		// per qualche motivo raddoppia gli a-capo, se c'erano dei CR/LF nel testo macro...
		V=T->vars;
		while(V) {
			if(!V->type)
				fprintf(FO,"  P:\t%-32s\n",V->name);
			V=V->next;
			}
		V=T->vars;
		while(V) {
			if(V->type)
				fprintf(FO,"  L:\t%-32s\n",V->name);
			V=V->next;
			}
	  fprintf(FO,"  **MACRO END\n\n");
    T=T->next;
    }

    
  return 0;
  }

int subCmpVarAddr(struct VARS *v1, struct VARS *v2) {

  return v1->value==v2->value ? 0 : 
		((uint64_t)MAKEDLONG(v1->value,v1->segm)>(uint64_t)MAKEDLONG(v2->value,v2->segm) ? 1 : -1);
  }

int PROCV2(FILE *FO) {
  int n,i;
  struct VARS *V2=NULL,*V;

  fprintf(FO,"\n%-46s\t%-12s\t%-4s\t%-6s\t%-8s\n",
		"Variabili e label (per valore):","Valore","Tipo","Size","Parent","Proc");
  n=InOrderVar2(&V2,Var);
	qsort(V2,n,sizeof(struct VARS),(int (*)(const void *, const void *))subCmpVarAddr);
  for(i=0; i<n; i++) {
		V=&V2[i];
#if GD24032
		fprintf(FO,"\t%-32s\t%10X:%08lX\t%6X\t%6X\t  %s\t  %s",V->name,LOWORD(V->segm),LODWORD(V->value),		// BUG C finire!
#else
		fprintf(FO,"\t%-32s\t%10X:%08X\t%6X\t%6X\t  %s\t  %s",V->name,LOWORD(V->segm),V->value,
#endif
			V->type,V->size,
			V->tag ? ((struct VARS *)(V->tag))->name : "",
			V->proc ? V->proc->name : ((char *)""));
		if(V->type & VAR_FUNZ)
			fprintf(FO,"\n\t-\t%-32s\t%15X\t%12s\n",
				V->name,V->dim,V->type & VAR_PUBLIC ? "public" : "");
		else
			fprintf(FO,"%12s\n",V->type & VAR_PUBLIC ? "public" : "");
		if(V->type & VAR_STRING)
			fprintf(FO,"\t\t\"%s\"\n",
				V->value ? (const char*)V->value : "");
    }      
//  fputc('\n',FO);

  return 0;
	}

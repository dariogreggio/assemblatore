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
uint16_t PageLength=66;
uint16_t LineLength=80;        // in realta non e' ancora usato

struct ERRORE Errs[]={
  1000,"errore interno sconosciuto - contattare ADPM",0x10,
  1001,"errore interno (fine memoria)",0x10,
  1004,"EOF inaspettato",0x10,
  1006,"non atteso",0,
  1013,"impossibile aprire il file sorgente",0x10,
  1015,"impossibile aprire il file include",0x10,
  1018,"#elif inatteso",0x10,
  1019,"#else inatteso",0x10,
  1020,"#endif inatteso",0x10,
  1021,"bad preprocessor command",0,
  1022,"#endif atteso",0x10,
  1035,"espressione troppo complessa, semplificarla",0,
  1037,"impossibile aprire il file oggetto",0x10,
  1038,"impossibile aprire il file esadecimale",0x10,
  2007,"#define syntax",0,
  2010,"invalid formal list",0,
  2015,"too many chars in costant",0,
  2016,"costante errata",0,
  2017,"sequenza di escape illegale",0,
  2036,"left operand must have struct/union type:",0,
  2038,"not struct/union member:",0x10,
  2039,"-> requires struct/union pointer",0x10,
  2040,". requires struct/union name",0x10,
  2054," atteso",0,
  2057,"attesa costante",0,
  2060,"errore di sintassi",0x10,
  2061,"codice operativo sconosciuto",0x10,
  2062,"operando errato",0x10,
  2063,"salto relativo fuori portata",0,
  2064,"dimensione operando sbagliata",0,
  2065,"non definito",0x10,
  2066,"simbolo PUBLIC non definito",0x10,
  2067,"istruzione estesa o non standard",0,
  2079,"uses undefined struct/union",0x10,
  2086,"ridefinito",0,
  2100,"manca ENDP o ENDC",0,
  2101,"emissione di dati al di fuori di un segmento",0,
  2102,"emissione di codice in un segmento dati",0,
  2103,"errore nella nidificazione dei blocchi",0,
  2115,"tipi incompatibili",0,
  2137,"costante carattere vuota",0,
  4002,"flag sconosciuto e ignorato",0,
  4010,"compattazione di salto assoluto",0,
  4011,"estensione di salto relativo",0,
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

int PROCError(int Er, const char *a) {
  int i,ch;
  char myBuf[128];

//	if(a[strlen(a)-1]=='\r')
//		a[strlen(a)-1]=0;


  if(OutError && FIn) {    // solo se il file source e' realmente aperto...
    do {
      fseek(FIn,-2,SEEK_CUR);
      if(ftell(FIn)<1)
        break;
      ch=getc(FIn);
      } while(ch != '\n');
    FNGetLine(ftell(FIn),myBuf);
    fputc('\n',stderr);
    fputs(myBuf,stderr);
    fputc('\n',stderr);
    }
  fprintf(stderr,"%s: (%d): errore %d ",__file__,__line__,Er);
  i=0;
  while (Er != Errs[i].t && Errs[i].t)
    i++;
  fputs(Errs[i].s,stderr);
  if(a && *a)
    fprintf(stderr,": %s",a);
  fputc('\n',stderr);
  
  if(FMap)
    fclose(FMap);
  if(FCod) {
    PROCV(FCod);
    fclose(FCod);
    }
  if(FPre)
    fclose(FPre);
  if(FObj) {
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
  if(Errs[i].liv <= WarnLevel) {
	  sprintf(myBuf,"%s: (%d): warning %d: %s %s\n",__file__,__line__,Er,Errs[i].s,a ? a : ".");
//		if(myBuf[strlen(myBuf)-1]=='\n')
//			myBuf[strlen(myBuf)-1]=0;
	  fprintf(stderr,myBuf);
	  PROCOut1(myBuf,NULL,1);     // mette i warning in file lst
	  }
  
  return 0;
  }

void InOrderVar(FILE *FO,struct VARS *V) {

	if(!V)
	  return;

	InOrderVar(FO,V->left);
  fprintf(FO,"\t%-32s\t%6X:%04X\t%4X\t%4X\t%s\n",V->label,HIWORD(V->value),LOWORD(V->value),V->type,V->size,V->proc ? V->proc->label : ((char *)""));
	InOrderVar(FO,V->right);
	}

int PROCV(FILE *FO) {
  int t,i;
  struct VARS *V;

  fprintf(FO,"\n\n\t%-32s\t  %-10s\t%-4s\t%-6s\t%-8s\n","Variabili e label:","Valore","Tipo","Size","Proc");
  InOrderVar(FO,Var);
//  V=Var;
//  while(V) {
//    fprintf(FO,"\t%-32Fs\t%10lx\t%4x\t%4x\t%Fs\n",V->label,V->value,V->type,V->size,V->proc ? V->proc->label : ((char far *)""));
//    V=V->next;
//    }      
  fputc('\n',FO);
  V=Var;
  while(V) {
    if(V->type & 0x100)
      fprintf(FO,"Proc: %-32s\t%10s\t%4X\n",
				V->label,V->type & 0x200 ? "public" : "",V->dim);
    V=V->next;
    }
  fputc('\n',FO);
  for(t=0; t<Segs; t++) {
//    if(CurAddr[t]>0)                        // se il segmento è usato
    fprintf(FO,"Seg. (%02X): %-32s\t%06X\t%06lX\t%06lX\n",t+1,Seg[t].label,Seg[t].type,
			Seg[t].BaseAddr==0xffffffff ? 0 : Seg[t].BaseAddr,
			Seg[t].BaseAddr==0xffffffff ? Seg[t].len : Seg[t].len-Seg[t].BaseAddr);
    }
  fputc('\n',FO);
    
  return 0;
  }


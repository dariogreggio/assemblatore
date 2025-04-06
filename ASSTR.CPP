#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <time.h>
#include "as.h"

extern uint8_t CPULevel;
extern uint8_t inMacroExecuting,macroLineNumber;
extern struct LINE_DEF *CurrMacro;
int lineUsed;
FILE *FCod,*FObj,*FMap;
static struct OBJ_STRUCT ObjStruct;
uint32_t OldOffNome,LastAddress;
//static uint32_t RelocAddr,CodeAddr,PubAddr,ExtAddr; 
//static int16_t NumSegs;
extern char defaultAbsSegm[64];
extern char buffer[256];
extern int8_t CreaHex,CreaBin,CreaElf;			// https://elfy.io/  analyzer online
extern int LastStmt4Cycle;
extern char defaultCodeSegm[64];

struct OBJ_HEADER ObjHeader;
struct OBJ_SEG_DEF ObjSeg;

struct ELF_HEADER ElfHeader;
struct ELF_SECTION_HEADER ElfSectionHeader;
struct ELF_SYMBOL_TABLE_ENTRY ElfSymbolTableEntry;
struct ELF_REL Elf32_Rel;
struct ELF_RELA Elf_Rela;
struct ELF_PROGRAM_HEADER Elf_Phdr;
struct ELF_DYN Elf_Dyn;

struct COFF_HEADER CoffHeader;
struct COFF_SECTION_TABLE CoffSection;


int16_t subInOrd1(struct VARS *V,int16_t j) {				// questa scrive le Public
  long l3;

	if(!V)
	  return j;
	j=subInOrd1(V->left,j);
  if((V->type & (VAR_PUBLIC | VAR_PUBLIC_DEFINED /*| VAR_LABEL*/)) == (VAR_PUBLIC | VAR_PUBLIC_DEFINED /*| VAR_LABEL*/)) {
printf("scrivo public %4x a %x...\n",V->value,ftell(FObj));
    *((int16_t *)&l3)=V->value;            // tolgo il segm.
    fwrite(&l3,4,1,FObj);
    fwrite(&OldOffNome,4,1,FObj);
    V->lpos=OldOffNome;
    OldOffNome+=strlen(V->name)+1;
    j++;
    }
	j=subInOrd1(V->right,j);
	return j;
  }

int subInOrd33(struct VARS *V,int j,int16_t seg) {			// questa scrive le Proc
  uint32_t l2;
  struct OBJ_RELOC ov;

	if(!V)
    return j;
  j=subInOrd33(V->left,j,seg);
    			// le proc sono automaticamente public??? 0x300 o 0x200
  if((V->type & VAR_PUBLIC) && (V->type & VAR_ADDRESS) && (V->segm == seg+1)) {
    	      printf("scrivo public %4x... @%x: ",V->value,ftell(FObj));

		l2=ftell(FObj);
    fseek(FObj,OldOffNome,SEEK_SET);
    fputs(V->name,FObj);
    	      printf("%s a %x: ",V->name,OldOffNome);
    fputc(0,FObj);
    fseek(FObj,l2,SEEK_SET);

		ov.type=V->value;
		ov.reserved=V->type & VAR_FUNZ ? 1 : 0;
		ov.address=OldOffNome-ObjHeader.offNomi;

//			      printf(" e nome %4x... @%x\n",OldOffNome,ftell(FObj));
    fwrite(&ov,sizeof(struct OBJ_RELOC),1,FObj);
    V->lpos=OldOffNome;
    OldOffNome += strlen(V->name)+1;
  	          
    j++;
    }
  j=subInOrd33(V->right,j,seg);
  
  return j;
  }
  
int subInOrd4(struct VARS *V,int j,int16_t seg) {			// questa scrive le Extern CHE METTO TUTTE NEL _abs (2024)
  uint32_t l2;
  struct OBJ_RELOC ov;
  
	if(!V)
    return j;
  j=subInOrd4(V->left,j,seg);
//    	      printf("guardo per ch. seg, %lx...\n",V->value);
  if(V->type & VAR_EXTERN && V->segm /*&& LOBYTE(HIWORD(V->value)) == seg+1*/) {
//  	          l3=((Var[i].value & 0x00ff0000) >> 16) -1;            // tolgo il segm.

		l2=ftell(FObj);
    fseek(FObj,OldOffNome,SEEK_SET);
    fputs(V->name,FObj);
    fputc(0,FObj);
//              puts(V->name);
    fseek(FObj,l2,SEEK_SET);

//  	fseek(FObj,addr-8+LOBYTE(HIWORD(V->value))*8,SEEK_SET);
		ov.type=MAKEWORD(0,1);
		ov.reserved=0;
		ov.address=OldOffNome-ObjHeader.offNomi;
//  	          if(!V->lpos) {
//    	          fwrite(&OldOffNome,4,1,FObj);
//  	            V->lpos=OldOffNome;
//  	            OldOffNome+=_fstrlen(V->name)+1;
//  	            }      
//  	          else {
//								V->lpos+=myPos;
    V->lpos=OldOffNome;
    fwrite(&ov,sizeof(struct OBJ_RELOC),1,FObj);

    OldOffNome+=strlen(V->name)+1;
//  	            }
  	j++;
//    *(((char *)&V->value)+2)=0;  // usare HIBYTE(HIWORD
		V->segm=0;
  	}
  j=subInOrd4(V->right,j,seg);
  return j;
  }

#if 0  
void subInOrd5(struct VARS *V) {
  
	if(!V)
    return;
  subInOrd5(V->left);
                 // guardo se c'è almeno una public EQU
//    	      printf("type %x\n",V->type);
  if(!PublicEQU) {
	  if(V->type & VAR_PUBLIC) {
	    if(!(V->type & VAR_PUBLIC_DEFINED)) {
	      PROCWarn(2066,V->name);
	      }
	   	if(!(V->type & VAR_LABEL)) {
	     	PublicEQU=TRUE;
	     	PROCDclSeg(defaultAbsSegm,SEGMENT_ABS);
	     	}
	    } 	
    } 	
  subInOrd5(V->right);
  }
#endif
  
	uint32_t myPtr,charPtr;
int subOut(int32_t t,int32_t t1) {
  char myBuf[128];
	uint8_t lineLen;
  long l,l2,l3;       
  int i,j;
  struct VARS *V;
	static uint8_t myChksum;			/* lungh del primo record */;   // usati per hex
//	static uint32_t myPtr,charPtr;
	static long myInizriga=0;      // usati per hex
	static uint8_t outCnt;
	static uint8_t oldT,oldT2,oldT3;
  
  if(CPass) {
	  switch(t) {
	    case OUT_END:                      // end
	      if(CreaHex) {
	        if(charPtr & 0x1f) {
			 	    lineLen=((ftell(FObj)-myInizriga) /2)-4;		// fa schifo ma ok! 2024
						// alle volte capita ancora un valore dispari alla fine, tipo con segmenti sparsi o DW... 2025
		  	    myChksum += lineLen;
			  	  fprintf(FObj,"%02X\n",-myChksum & 0xff);
			 	    fseek(FObj,myInizriga,SEEK_SET);
			 	    fprintf(FObj,"%02X",lineLen);
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
	      else if(CreaElf==1) {
					ElfHeader.e_shoff=LastAddress;
					j=0;
					for(i=0; i<Segs; i++) {
//						if((Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) 
							j++;
						}
					ElfHeader.e_phnum=j;
					ElfHeader.e_shnum=1  +j   +2 /*symbol e stringhe*/;

// FORSE non contare i SEGMENT_VIRT

		      fseek(FObj,0,SEEK_SET);
					fwrite(&ElfHeader,sizeof(struct ELF_HEADER),1,FObj);

					for(i=0; i<Segs; i++) {
						uint32_t myPos;
						myPos=ftell(FObj);
//						if(Seg[i].type != (SEGMENT_VIRT & 0xff00)) {
	/*mettere!		sistemare di là, mettere trovare offset giusti				fseek(FObj,Seg[i].lposPub,SEEK_SET);
							OldOffNome=LastAddress;
							j=subInOrd33(Var,0,i);
							fseek(FObj,Seg[i].lposExt,SEEK_SET);
							subInOrd4(Var,0,i);
							LastAddress=OldOffNome;*/
							fseek(FObj,myPos,SEEK_SET);

							Elf_Phdr.p_type=1;		// PT_LOAD
							Elf_Phdr.p_flags=LOBYTE(Seg[i].type) & 0x7;			// data const; PF_R=4  PF_W=2  PF_X=1
							Elf_Phdr.p_offset=Seg[i].lpos;
							Elf_Phdr.p_align=1;
							Elf_Phdr.p_paddr=Seg[i].BaseAddr != 0xffffffff ? Seg[i].BaseAddr : 0;
//							Elf_Phdr.p_paddr=Seg[i].CurAddr - Seg[i].len;
							Elf_Phdr.p_paddr=0;		// per ora così, mi piace
							Elf_Phdr.p_vaddr=Seg[i].BaseAddr != 0xffffffff ? Seg[i].BaseAddr : 0;
//							Elf_Phdr.p_vaddr=Seg[i].CurAddr - Seg[i].len;
  						j=Seg[i].len;		//
							Elf_Phdr.p_filesz=j;
							Elf_Phdr.p_memsz =j;

	 						fwrite(&Elf_Phdr,sizeof(ELF_PROGRAM_HEADER),1,FObj);
//							}
						}

					j=LastAddress+ElfHeader.e_shnum*sizeof(ELF_SECTION_HEADER);
		      fseek(FObj,LastAddress,SEEK_SET);

					memset(&ElfSectionHeader,0,sizeof(ELF_SECTION_HEADER));		// sezione NULLA obbligatoria!
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);
					j++;
					LastAddress+=sizeof(struct ELF_SECTION_HEADER);
					for(i=0; i<ElfHeader.e_shnum-1-2; i++) {
//						if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {
							ElfSectionHeader.sh_name=j;
							switch(Seg[i].type & 0xff00) {
								//NO i sh_name sono puntatori a stringhe!! FARE

								case SEGMENT_CODE & 0xff00:
									ElfSectionHeader.sh_flags=6;	// SHF_ALLOC | SHF_EXECINSTR
									ElfSectionHeader.sh_type=1;		// PROGBITS
									break;
								case SEGMENT_DATA & 0xff00:
									ElfSectionHeader.sh_flags=0x00800003;	// SHF_ALLOC | SHF_WRITE | SHF_MASKOS
									ElfSectionHeader.sh_type=1;		// PROGBITS
									break;
								case SEGMENT_BSS & 0xff00:
									ElfSectionHeader.sh_flags=3;	// SHF_ALLOC | SHF_WRITE
									ElfSectionHeader.sh_type=1;		// PROGBITS
									break;
								case SEGMENT_CONST & 0xff00:
									ElfSectionHeader.sh_flags=0x10000002;	// SHF_ALLOC | SHF_MASKPROC
									ElfSectionHeader.sh_type=1;		// PROGBITS
									break;
								case SEGMENT_ABS & 0xff00:
									ElfSectionHeader.sh_flags=0;	// boh
									ElfSectionHeader.sh_type=0;		// 
									break;
								case SEGMENT_VIRT & 0xff00:
									ElfSectionHeader.sh_flags=0;	// boh
									ElfSectionHeader.sh_type=0;		// 
									break;
	#if PIC==16 
								case SEGMENT_CONFIG & 0xff00:
										//SEGMENT_IDLOCS
									ElfSectionHeader.sh_flags=6;	// SHF_ALLOC | SHF_EXECINSTR
									ElfSectionHeader.sh_type=1;		// PROGBITS
									break;
	#elif PIC==18
								case SEGMENT_CONFIG & 0xff00:
									ElfSectionHeader.sh_flags=6;	// SHF_ALLOC | SHF_EXECINSTR
									ElfSectionHeader.sh_type=1;		// PROGBITS
									break;
								case SEGMENT_IDLOCS & 0xff00:
									ElfSectionHeader.sh_flags=6;	// SHF_ALLOC | SHF_EXECINSTR
									ElfSectionHeader.sh_type=1;		// PROGBITS
									break;
	#elif GD24032
	#endif
								}
							ElfSectionHeader.sh_addr=Seg[i].BaseAddr != 0xffffffff ? Seg[i].BaseAddr : 0;
							ElfSectionHeader.sh_offset=Seg[i].lpos;
	// TROVARE!!

							ElfSectionHeader.sh_size=Seg[i].len;
	// IDEM sistemare

							ElfSectionHeader.sh_link=0;
							ElfSectionHeader.sh_info=0;
							ElfSectionHeader.sh_addralign=1;
							ElfSectionHeader.sh_entsize=0;
							fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);
							j += strlen(Seg[i].name)+1;
							LastAddress+=sizeof(struct ELF_SECTION_HEADER);
//							}
						}


					ElfSectionHeader.sh_name=j;
// NOME qua??
					ElfSectionHeader.sh_flags=0;	// 
					ElfSectionHeader.sh_type=2;		// SHT_SYMTAB
					ElfSectionHeader.sh_addr=0;
					ElfSectionHeader.sh_offset=ftell(FObj)+sizeof(struct ELF_SECTION_HEADER);
					i=0;
					ElfSectionHeader.sh_size=Segs+1+2; // SICURO?? 2025
					ElfSectionHeader.sh_link=0;
					ElfSectionHeader.sh_info=0;
					ElfSectionHeader.sh_addralign=1;
					ElfSectionHeader.sh_entsize=0;
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);
					LastAddress+=sizeof(struct ELF_SECTION_HEADER);

// x stringhe https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-73709.html#scrolltoc
					ElfSectionHeader.sh_name=j;
// NOME qua??
					ElfSectionHeader.sh_flags=0;	// 
					ElfSectionHeader.sh_type=3;		// SHT_STRTAB
					ElfSectionHeader.sh_addr=0;
					ElfSectionHeader.sh_offset=ftell(FObj)+sizeof(struct ELF_SECTION_HEADER)+8 /*.symtab*/;
					i=0;
					j=1;
					while(/*i<NumSegs-1*/ *Seg[i].name)		// aggiungere public ed extern, verificare
						j+=strlen(Seg[i++].name)+1;
					ElfSectionHeader.sh_size=j;
					ElfSectionHeader.sh_link=0;
					ElfSectionHeader.sh_info=0;
					ElfSectionHeader.sh_addralign=1;
					ElfSectionHeader.sh_entsize=0;
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);
					LastAddress+=sizeof(struct ELF_SECTION_HEADER);

 					fseek(FObj,LastAddress,SEEK_SET);         //  ora le stringhe
					fputc(0,FObj);					// 1° stringa vuota sempre!
					i=0;
					while(/*i<NumSegs-1*/ *Seg[i].name) {
//						if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {
							fputs(Seg[i].name,FObj);
							fputc(0,FObj);
							Seg[i].lposName=LastAddress;
							LastAddress = ftell(FObj);
							ObjHeader.lunNomi += strlen(Seg[i].name)+1;
//							}
						i++;
						}
					fputs(".symtab",FObj);
					fputc(0,FObj);
					LastAddress = ftell(FObj);
					ObjHeader.lunNomi += 7+1;
					fputs(".strtab",FObj);
					fputc(0,FObj);
					LastAddress = ftell(FObj);
					ObjHeader.lunNomi += 7+1;

	        }
	      else if(CreaElf==2) {
					time_t tt;
					j=0;
					for(i=0; i<Segs; i++) {
//						if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) 
							j++;
						}
					CoffHeader.numberOfSections=j;
					CoffHeader.numberOfSymbols=1  +j   +2 /*symbol e stringhe*/;
// verificare, non è come in Elf...
// FORSE non contare i SEGMENT_VIRT

					CoffHeader.pointerToSymbolTable=0;
					CoffHeader.sizeOfOptionalHeader=0;
					CoffHeader.timeDateStamp=time(&tt);
					CoffHeader.characteristics=0;
		      fseek(FObj,0,SEEK_SET);
					fwrite(&CoffHeader,sizeof(struct COFF_HEADER),1,FObj);

					j=LastAddress+CoffHeader.numberOfSections*sizeof(COFF_SECTION_TABLE);
		      fseek(FObj,LastAddress,SEEK_SET);

					memset(&CoffSection,0,sizeof(COFF_SECTION_TABLE));		// sezione NULLA obbligatoria!
					fwrite(&CoffSection,sizeof(struct COFF_SECTION_TABLE),1,FObj);
					LastAddress+=sizeof(struct COFF_SECTION_TABLE );
					for(i=0; i<CoffHeader.numberOfSymbols-1-2; i++) {
//						if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {
							strncpy((char*)&CoffSection.name,Seg[i].name,8);
	// no						CoffSection.name[7]=0;
							CoffSection.characteristics = 0;
							switch(Seg[i].type & 0xff00) {

								//finire!!
								case SEGMENT_CODE & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE;
									break;
								case SEGMENT_DATA & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA;
									break;
								case SEGMENT_BSS & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_UNINITIALIZED_DATA;
									break;
								case SEGMENT_CONST & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_UNINITIALIZED_DATA;
									break;
								case SEGMENT_ABS & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_SHARED /**boh*/;
									break;
								case SEGMENT_VIRT & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_SHARED /**boh*/;
									break;
	#if PIC==16 
								case SEGMENT_CONFIG & 0xff00:
										//SEGMENT_IDLOCS
									CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
									break;
	#elif PIC==18
								case SEGMENT_CONFIG & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
									break;
								case SEGMENT_IDLOCS & 0xff00:
									CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
									break;
	#elif GD24032
	#endif
								}
							CoffSection.virtualAddress=Seg[i].BaseAddr != 0xffffffff ? Seg[i].BaseAddr : 0;

							CoffSection.sizeOfRawData=Seg[i].len;
	// IDEM sistemare

							CoffSection.characteristics |= 
								Seg[i].len==1 ? IMAGE_SCN_ALIGN_1BYTES : (Seg[i].len==2 ? IMAGE_SCN_ALIGN_2BYTES :
							(Seg[i].len==4 ? IMAGE_SCN_ALIGN_4BYTES : 0));
							fwrite(&CoffSection,sizeof(struct COFF_SECTION_TABLE),1,FObj);
							LastAddress+=sizeof(struct COFF_SECTION_TABLE);
	//						}
						}


					strncpy((char*)&CoffSection.name,"SYMBOLS",8);		// boh
// no						CoffSection.name[7]=0;
					CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_SHARED /**boh*/;
					CoffSection.virtualAddress=0;
					CoffSection.pointerToRawData=ftell(FObj)+sizeof(struct COFF_SECTION_TABLE);
					i=0;
					CoffSection.sizeOfRawData=Segs+1+2;
					CoffSection.characteristics |= IMAGE_SCN_ALIGN_1BYTES;		//
					fwrite(&CoffSection,sizeof(struct COFF_SECTION_TABLE),1,FObj);
					LastAddress+=sizeof(struct COFF_SECTION_TABLE);

	/*mettere!						fseek(FObj,Seg[i].lposPub,SEEK_SET);
							OldOffNome=LastAddress;
							j=subInOrd33(Var,0,i);
							fseek(FObj,Seg[i].lposExt,SEEK_SET);
							subInOrd4(Var,0,i);
							LastAddress=OldOffNome;*/


					strncpy((char*)&CoffSection.name,"STRINGS",8);		// boh
// NOME qua??
					CoffSection.characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
					CoffSection.virtualAddress=0;
					CoffSection.pointerToRawData=ftell(FObj)+sizeof(struct COFF_SECTION_TABLE);
					i=0;
					j=1;
					while(/*i<NumSegs-1*/ *Seg[i].name)
						j+=strlen(Seg[i++].name)+1;
					CoffSection.sizeOfRawData=j;
					CoffSection.characteristics |= IMAGE_SCN_ALIGN_1BYTES;		//
					fwrite(&CoffSection,sizeof(struct COFF_SECTION_TABLE),1,FObj);
					LastAddress+=sizeof(struct COFF_SECTION_TABLE);

 					fseek(FObj,LastAddress,SEEK_SET);         //  ora le stringhe
					fputc(0,FObj);					// 1° stringa vuota sempre!
					i=0;
					while(/*i<NumSegs-1*/ *Seg[i].name) {
//						if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {
							fputs(Seg[i].name,FObj);
							fputc(0,FObj);
							Seg[i].lposName=LastAddress;
							LastAddress = ftell(FObj);
							ObjHeader.lunNomi += strlen(Seg[i].name)+1;
//							}
						i++;
						}

	        }
	      else {
					j=0;
					for(i=0; i<Segs; i++) {
//rimettere?	no					if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) 
							j++;
						}
					ObjHeader.numSegs=(uint8_t)j;
					ObjHeader.offNomi=LastAddress;
					ObjHeader.lunNomi=0;
 					fseek(FObj,LastAddress,SEEK_SET);         // mi preparo per il nome seg
					i=0;
					while(/*i<NumSegs-1*/ *Seg[i].name) {
//rimettere	= no					if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {
							fputs(Seg[i].name,FObj);
		//    	      printf("scrivo nSeg %s a %lx...\n",Seg[i].name,Seg[i].lpos);
							fputc(0,FObj);
							Seg[i].lposName=LastAddress;
							LastAddress = ftell(FObj);
							ObjHeader.lunNomi += strlen(Seg[i].name)+1;
//							}
						i++;
						}                  
//					subInOrd2(LastAddress,Var);
					LastAddress = ftell(FObj);

//					j=subInOrd2(0,Var);
//						j=subInOrd4(Seg[OldSeg-1].lposExt,Var,0);

					fseek(FObj,0,SEEK_SET);
					fwrite(&ObjHeader,sizeof(OBJ_HEADER),1,FObj);

					for(i=0; i<Segs; i++) {
						uint32_t myPos;
//						if(Seg[i].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {
							myPos=ftell(FObj);
							fseek(FObj,Seg[i].lposPub,SEEK_SET);
							OldOffNome=LastAddress;
							j=subInOrd33(Var,0,i);
							fseek(FObj,Seg[i].lposExt,SEEK_SET);
							subInOrd4(Var,0,i);
							LastAddress=OldOffNome;
							fseek(FObj,myPos,SEEK_SET);

	 						ObjSeg.tipo=MAKEWORD(0,HIBYTE(Seg[i].type));	
							ObjSeg.offNome=(uint16_t)Seg[i].lposName-ObjHeader.offNomi;
	 						ObjSeg.offCode=Seg[i].lpos;
	 						ObjSeg.lunCode=Seg[i].len;
							ObjSeg.offReloc=Seg[i].lposReloc;
							ObjSeg.lunReloc=Seg[i].numReloc*sizeof(struct OBJ_RELOC);
	 						ObjSeg.offPublic=Seg[i].lposPub;
	 						ObjSeg.offExtern=Seg[i].lposExt;
	 						ObjSeg.numPublic=Seg[i].numPub;
	  					ObjSeg.numExtern=Seg[i].numExt;
	 						fwrite(&ObjSeg,sizeof(OBJ_SEG_DEF),1,FObj);
//							}
						}

					}
	      break;
	    case OUT_VAR_EXT:                      // scrive var extern e public
	      if(CreaHex) {
	        }
	      else if(CreaElf) {
	        }
	      else {
	        }
	      break;
	    case OUT_SEGM:                      // cambio di segmento
	      if(CreaHex) {
// di là	        PROCWarn(1006,"CSEG/DSEG");
					// start segment address record, #3, https://en.wikipedia.org/wiki/Intel_HEX
					if(charPtr) {		// fa schifo ma ok! 2024  v.anche altrove
			 			lineLen=((ftell(FObj)-myInizriga) /2)-4;		// fa schifo ma ok! 2024  v.anche sopra
			  		myChksum += lineLen;
			  		fprintf(FObj,"%02X\n",-myChksum & 0xff);
			 			fseek(FObj,myInizriga,SEEK_SET);
			 			fprintf(FObj,"%02X",lineLen);
			 			fseek(FObj,0,SEEK_END);
						}
#if I8086 || GD24032
					fprintf(FObj,":02000002%02X%02X%02X\n",LOBYTE(HIWORD(Seg[CurSeg-1].BaseAddr)),HIBYTE(HIWORD(Seg[CurSeg-1].BaseAddr)),
						(uint8_t)-(2+2+LOBYTE(HIWORD(Seg[CurSeg-1].BaseAddr))+HIBYTE(HIWORD(Seg[CurSeg-1].BaseAddr))));	// parte alta base address
#else
					fprintf(FObj,":02000002%02X%02X%02X\n",LOBYTE(HIWORD(Seg[CurSeg-1].BaseAddr)),HIBYTE(HIWORD(Seg[CurSeg-1].BaseAddr)),
						(uint8_t)-(2+2+LOBYTE(HIWORD(Seg[CurSeg-1].BaseAddr))+HIBYTE(HIWORD(Seg[CurSeg-1].BaseAddr))));	// parte alta base address
#endif
		 	    myInizriga=ftell(FObj);
	  	    myChksum=0;
					myPtr=Seg[CurSeg-1].CurAddr /*passarlo come t1...*/;
					charPtr=0;
	        }
	      else if(CreaElf) {
// header
// program_header x n
// dati/code seg.1
// reloc seg 1
// dati e reloc seg.n
// sections
// seg.name & var string-table

					if((Seg[CurSeg-1].type & 0xff00) != (SEGMENT_VIRT & 0xff00))
						fseek(FObj,	Seg[CurSeg-1].lpos+Seg[CurSeg-1].CurAddr-Seg[CurSeg-1].BaseAddr,SEEK_SET);
//			      printf("Segmento: %x(%s), code %x, lpos %x, len %x, reloc %x, offName %x\n",
//							CurSeg,Seg[CurSeg-1].name,Seg[CurSeg-1].lpos,Seg[CurSeg-1].lpos,Seg[CurSeg-1].len,Seg[CurSeg-1].lposReloc,OldOffNome);
	        }
	      else {
// header
// obj_def x n
// dati/code seg.1
// reloc seg 1
// dati e reloc seg.n
// seg.name & var string-table

					if((Seg[CurSeg-1].type & 0xff00) != (SEGMENT_VIRT & 0xff00)) {
						if(Seg[CurSeg-1].BaseAddr == 0xffffffff)
							fseek(FObj,	Seg[CurSeg-1].lpos,SEEK_SET);
						else
							fseek(FObj,	Seg[CurSeg-1].lpos+Seg[CurSeg-1].CurAddr-Seg[CurSeg-1].BaseAddr,SEEK_SET);
						}
//			      printf("Segmento: %x(%s), code %x, lpos %x, len %x, reloc %x, offName %x\n",
//							CurSeg,Seg[CurSeg-1].name,Seg[CurSeg-1].lpos,Seg[CurSeg-1].lpos,Seg[CurSeg-1].len,Seg[CurSeg-1].lposReloc,OldOffNome);
//						NumSegs++;
 					}
	      break;
	    case OUT_RELOC:                      // scrive reloc addr per seg
	      if(CreaHex) {
// mah 2024	       					PROCWarn(1006,"");
	        }
	      else if(CreaElf==1) {
					struct ELF_REL or;
					l2=ftell(FObj);
//					ElfHeader.e_phoff=RelocAddr;   Seg[CurSeg-1].CurAddr-2
									  Seg[CurSeg-1].numReloc++;

					fseek(FObj,l2,SEEK_SET);
					//FINIRE
	        }
	      else if(CreaElf==2) {
					struct COFF_RELOCATIONS or;
					l2=ftell(FObj);
					or.virtualAddress=Seg[CurSeg-1].CurAddr-2;
					or.symbolTableIndex=Seg[CurSeg-1].numReloc;
					if(t1 < 0) {                // extrn
						or.type=MAKEWORD(LOBYTE(t1) -1,1);
						}
					else {                      // norm
						or.type=t1/*CurSeg*/-1;
						}
					or.type=0;
									  Seg[CurSeg-1].numReloc++;

					fwrite(&or,sizeof(struct COFF_RELOCATIONS),1,FObj);
					fseek(FObj,l2,SEEK_SET);
					//FINIRE
	        }
	      else {
					struct OBJ_RELOC or;
					l2=ftell(FObj);
					fseek(FObj,Seg[CurSeg-1].lposReloc+Seg[CurSeg-1].numReloc*sizeof(struct OBJ_RELOC),SEEK_SET);
					if(t1 < 0) {                // extrn
						or.type=MAKEWORD(LOBYTE(t1) -1,1);
						}
					else {                      // norm
						or.type=t1/*CurSeg*/-1;
						}
					or.reserved=2;
					or.address=Seg[CurSeg-1].CurAddr-2;
//	      printf("reloc addr %04x @%x\n",or.address,ftell(FObj));
					fwrite(&or,sizeof(struct OBJ_RELOC),1,FObj);
				  Seg[CurSeg-1].numReloc++;
					fseek(FObj,l2,SEEK_SET);
	//	      printf("Ora relocaddr vale %lx\n",RelocAddr);
					}
	      break;
	    case OUT_REPOS:                      // sposta puntatore (ORG/CSEG
	      if(CreaHex) {
					if(t1 && t1 < myPtr /*ftell(FObj)*/)
						PROCWarn(2104,"possibile sovrascrittura");		// lo dà anche se == .... boh
					if(charPtr) {		// fa schifo ma ok! 2024  v.anche altrove
			 			lineLen=((ftell(FObj)-myInizriga) /2)-4;		// fa schifo ma ok! 2024  v.anche sopra
			  		myChksum += lineLen;
			  		fprintf(FObj,"%02X\n",-myChksum & 0xff);
			 			fseek(FObj,myInizriga,SEEK_SET);
			 			fprintf(FObj,"%02X",lineLen);
			 			fseek(FObj,0,SEEK_END);
			 			myInizriga=ftell(FObj);
		  			myChksum=0;
						charPtr=0;
						}
	        }
	      else if(CreaElf) {
//rimettere? no					if(Seg[CurSeg-1].type & 0xff00) != (SEGMENT_VIRT & 0xff00))
	          fseek(FObj,t1,SEEK_CUR);
	        }
	      else {
//rimettere? no					if(Seg[CurSeg-1].type & 0xff00) != (SEGMENT_VIRT & 0xff00))
		        fseek(FObj,t1,SEEK_CUR);
					}
	      break;
	    case OUT_START:                      // start         
	      if(CreaHex) {
					struct VARS *v=FNCercaVar("_main",0,0,TRUE,CPass);				// indirizzo di "main" se c'è o direi ORG più basso! o 0
					if(v)
						l=v->value;
					else
						l=0;
//volendo... execution address
#if I8086 || GD24032
			    fprintf(FObj,":04000003%02X%02X%02X%02X%02X\n",
						LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),LOBYTE(HIWORD(l)),HIBYTE(HIWORD(l)),
						(uint8_t)-(4+3+LOBYTE(LOWORD(l))+HIBYTE(LOWORD(l))+LOBYTE(HIWORD(l))+HIBYTE(HIWORD(l)))
						);
#else
			    fprintf(FObj,":02000003%02X%02X%02X\n",LOBYTE(LOWORD(l)),HIBYTE(LOWORD(l)),
						(uint8_t)-(2+3+LOBYTE(LOWORD(l))+HIBYTE(LOWORD(l)));
#endif
		 	    myInizriga=ftell(FObj);
	  	    myChksum=0;
					myPtr=0;
					charPtr=0;
					}
	      else if(CreaElf==1) {
					memset((char*)ElfHeader.e_ident,0,sizeof(ElfHeader.e_ident));
					ElfHeader.e_ident[0]='\x7f';
					strcpy((char*)ElfHeader.e_ident+1,"ELF");
					ElfHeader.e_ident[4]=1;		// ELFCLASS32
					ElfHeader.e_ident[5]=1;		// ELFDATA2LSB
					ElfHeader.e_ident[6]=1;		// VERSION EV_CURRENT
					ElfHeader.e_type=2;				// ET_EXEC
#if PIC==16 || PIC==18
					ElfHeader.e_machine=204;		// 0x76 PIC24...https://gist.github.com/x0nu11byt3/bcb35c3de461e5fb66173071a2379779
																		//	204..209 8bit PIC
#elif I8051
					ElfHeader.e_machine=165;
#elif GD24032
					ElfHeader.e_machine=254;		// ehm invento :D
#elif I8086
					ElfHeader.e_machine=3;		// 386, dice; o anche 6 per 486 deprecated
#else
					ElfHeader.e_machine=220;		// 
																		// Z80=220 https://docs.harelang.org/format/elf
#endif
					ElfHeader.e_version=1;
					VARS *v=FNCercaVar("_main",0,0,TRUE,CPass);				// indirizzo di "main" se c'è o direi ORG più basso! o 0
						// indirizzo di "main" se c'è o direi ORG più basso! o 0
					if(v)
						ElfHeader.e_entry=v->value;
					else {
						i=FNCercaSeg("TEXT");
						if(i>0)
							ElfHeader.e_entry=Seg[i-1].BaseAddr != 0xffffffff ? Seg[i-1].BaseAddr : 0;
						else
							ElfHeader.e_entry=0;
						}
					ElfHeader.e_phnum=0;
					ElfHeader.e_phoff=sizeof(struct ELF_HEADER);		// prog. header table offset
					ElfHeader.e_flags=0x15f;		// boh
					ElfHeader.e_ehsize=sizeof(struct ELF_HEADER);
					ElfHeader.e_phentsize=sizeof(struct ELF_PROGRAM_HEADER);
					ElfHeader.e_shnum=0;
					ElfHeader.e_shoff=0;		// sect. header table offset
					ElfHeader.e_shentsize=sizeof(struct ELF_SECTION_HEADER);
					ElfHeader.e_shstrndx=0;		// potrebbe essere offset ai nomi dei segmenti, "string offset"

					fwrite(&ElfHeader,sizeof(struct ELF_HEADER),1,FObj);
/*					// (sembrano sempre 212 di header ecc, quindi... ) bed_alert.elf
					// https://elfy.io/
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_flags=7;			// PF_R=4  PF_W=2  PF_X=1
					Elf_Phdr.p_offset=0x000000;		// dove si trovano i byte
					Elf_Phdr.p_align=1;
					Elf_Phdr.p_filesz=0;
					Elf_Phdr.p_memsz=0;
					Elf_Phdr.p_vaddr=0;		// virtual address
					Elf_Phdr.p_paddr=0;		// phys address
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					// i vari segmenti! microchip mette bss, config, text ecc
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_flags=4;			// data const; PF_R=4  PF_W=2  PF_X=1
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					// 5 program header

					// e quindi ci sono i vari segmenti/sezioni
					// la prima sezione dev'essere tipo 0 e tutto vuoto!
					ElfSectionHeader.sh_name=27 ;		/// TEXT
//					SHF_EXECINSTR = 1<<2
//					SHT_PROGBITS=1 because it has user-defined code.
//					.bss: uninitialized data (SHT_NOBITS=8). It does not occupy space at disk to avoid space consuming, then all the data is usually initialized to zero at runtime. It is writable.
//					.data: initialized data, it is writable. (SHT_PROGBITS)

					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);

					ElfSectionHeader.sh_name=183 ;		// //ISR, IVT
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);

					ElfSectionHeader.sh_name=227;		 //DATA, 235 DCONST, 277=CONST
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);

					ElfSectionHeader.sh_name=194 ;		// //C30 signature...
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);
*/
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),Segs,FObj);
					}
	      else if(CreaElf==2) {
					CoffHeader.characteristics=2;				// ET_EXEC trovare, ficcare2
#if PIC==16 || PIC==18
					CoffHeader.machine=0x1240;		// da microchip 10F; 24F = 1236; 18F = 1240
#elif I8051
					CoffHeader.machine=0;
#elif GD24032
					CoffHeader.machine=0;		// ehm invento :D
#elif I8086
					CoffHeader.machine=0x14c;		// 386
#else
					CoffHeader.machine=0;		// 
#endif
					VARS *v=FNCercaVar("_main",0,0,TRUE,CPass);				// indirizzo di "main" se c'è o direi ORG più basso! o 0
/* questo andrebbe in una sezione a sè, fare					if(v)
						CoffHeader.e_entry=v->value;
					else {
						i=FNCercaSeg("TEXT");
						if(i>0)
							CoffHeader.e_entry=Seg[i-1].BaseAddr != 0xffffffff ? Seg[i-1].BaseAddr : 0;
						else
							CoffHeader.e_entry=0;
						}
						*/
					CoffHeader.pointerToSymbolTable=sizeof(struct COFF_HEADER);		// prog. header table offset
					CoffHeader.sizeOfOptionalHeader=0;		//

					fwrite(&CoffHeader,sizeof(struct COFF_HEADER),1,FObj);
/*					// (sembrano sempre 212 di header ecc, quindi... ) bed_alert.elf
					// https://elfy.io/
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_flags=7;			// PF_R=4  PF_W=2  PF_X=1
					Elf_Phdr.p_offset=0x000000;		// dove si trovano i byte
					Elf_Phdr.p_align=1;
					Elf_Phdr.p_filesz=0;
					Elf_Phdr.p_memsz=0;
					Elf_Phdr.p_vaddr=0;		// virtual address
					Elf_Phdr.p_paddr=0;		// phys address
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					// i vari segmenti! microchip mette bss, config, text ecc
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_flags=4;			// data const; PF_R=4  PF_W=2  PF_X=1
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					Elf_Phdr.p_type=1;		// PT_LOAD
					Elf_Phdr.p_offset=0x000000;
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),1,FObj);
					// 5 program header

					// e quindi ci sono i vari segmenti/sezioni
					// la prima sezione dev'essere tipo 0 e tutto vuoto!
					ElfSectionHeader.sh_name=27 ;		/// TEXT
//					SHF_EXECINSTR = 1<<2
//					SHT_PROGBITS=1 because it has user-defined code.
//					.bss: uninitialized data (SHT_NOBITS=8). It does not occupy space at disk to avoid space consuming, then all the data is usually initialized to zero at runtime. It is writable.
//					.data: initialized data, it is writable. (SHT_PROGBITS)

					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);

					ElfSectionHeader.sh_name=183 ;		// //ISR, IVT
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);

					ElfSectionHeader.sh_name=227;		 //DATA, 235 DCONST, 277=CONST
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);

					ElfSectionHeader.sh_name=194 ;		// //C30 signature...
					fwrite(&ElfSectionHeader,sizeof(struct ELF_SECTION_HEADER),1,FObj);
*/
					fwrite(&Elf_Phdr,sizeof(struct ELF_PROGRAM_HEADER),Segs,FObj);
					}
	      else {
#if PIC==18
					ObjHeader.v0=1;							// indico versione diversa...
					ObjHeader.v1=1;
					ObjHeader.v2=0x20;          // indico segmenti > 64KB (non trovo le spec di formato ma amen
					ObjHeader.res1=0;
#elif GD24032
					ObjHeader.v0=1;							// indico versione diversa...
					ObjHeader.v1=1;
					ObjHeader.v2=0x20;          // indico segmenti > 64KB (non trovo le spec di formato ma amen
					ObjHeader.res1=0;
#elif I8086
					ObjHeader.v0=1;							// indico versione diversa...
					ObjHeader.v1=1;
					if(CPULevel<3)		// gestire...
						ObjHeader.v2=0x20;          //
					else
						ObjHeader.v2=0x20;          // indico segmenti > 64KB (non trovo le spec di formato ma amen
					ObjHeader.res1=0;
#else
					ObjHeader.v0=0; 
					ObjHeader.v1=1;
					ObjHeader.v2=0x10;          // per compat. con LMBZ80
					ObjHeader.res1=0;
#endif
					fwrite(&ObjHeader,sizeof(OBJ_HEADER),1,FObj);
					fwrite(&ObjSeg,sizeof(OBJ_SEG_DEF),Segs,FObj);
					}
	      break;
	    default:
				t &= 0xff;
	      if(CreaHex) {
	        if(!charPtr) {
						myInizriga=ftell(FObj)+1;
	 	  		  fprintf(FObj,":00%04X00",Seg[CurSeg-1].CurAddr);
			  	  myChksum+=HIBYTE(Seg[CurSeg-1].CurAddr);
			  	  myChksum+=LOBYTE(Seg[CurSeg-1].CurAddr);
						}
	  	    fprintf(FObj,"%02X",t);
	  	    myChksum += t;
	  	    myPtr++;
					charPtr++;
	        if(charPtr >= 0x20) {
			 	    lineLen=((ftell(FObj)-myInizriga) /2)-4;		// fa schifo ma ok! 2024  v.anche sopra
			  	  myChksum += lineLen;
			  	  fprintf(FObj,"%02X\n",-myChksum & 0xff);
			 	    fseek(FObj,myInizriga,SEEK_SET);
			 	    fprintf(FObj,"%02X",lineLen);
			 	    fseek(FObj,0,SEEK_END);
			 	    myInizriga=ftell(FObj);
		  	    myChksum=0;
						charPtr=0;
			 	    }
	        }
	      else if(CreaElf) {
		      if(CurSeg==SEG_UNDEFINED)
		        PROCError(2101,"");
		      fputc(t,FObj);
	        }
	      else {
		      if(CurSeg==SEG_UNDEFINED)
		        PROCError(2101,"");
/*				  if(OutList) {
				    sprintf(myBuf," %02X",t);
		  	    PROCOut1(myBuf,NULL,TRUE);
				    }*/
		      fputc(t,FObj);
				  }
			  if(OutList) {

#if PIC==16 || PIC==18
					if(t1)			// resetto contatore byte out!
						outCnt=1;
					if(outCnt & 1) {
						sprintf(myBuf," %04X",MAKEWORD(t,oldT));
		  	    PROCOut1(myBuf,NULL,TRUE);
						}
					else
						oldT=t;
					outCnt++;
#elif GD24032
					if(t1)			// resetto contatore byte out!
						outCnt=3;
					switch(outCnt & 3) {
						case 3:
							sprintf(myBuf," %08X",MAKELONG(MAKEWORD(oldT,oldT2),MAKEWORD(oldT3,t)));		// 4 byte!
		  				PROCOut1(myBuf,NULL,TRUE);
							break;
						case 2:
							oldT3=t;
							break;
						case 1:
							oldT2=t;
							break;
						default:
							oldT=t;
							break;
						}
					outCnt++;
#else
			    sprintf(myBuf," %02X",t);
	  	    PROCOut1(myBuf,NULL,TRUE);
#endif
			    }
				if(Seg[CurSeg-1].BaseAddr == 0xffffffff)		// questo serve se non c'è stata ORG dopo SEGMENT/CSEG
					Seg[CurSeg-1].BaseAddr=Seg[CurSeg-1].CurAddr;
			  Seg[CurSeg-1].CurAddr++;
				Seg[CurSeg-1].MaxAddr=max(Seg[CurSeg-1].CurAddr,Seg[CurSeg-1].MaxAddr);
	      break;
	    }
	  }
	else {			// CPass
	  switch(t) {                                                  
	    case OUT_END:                      // end
        break;
	    case OUT_VAR_EXT:                      // scrive var extern e public
				if(t1) {
		    	i=PROCDclSeg(defaultAbsSegm,SEGMENT_ABS);
					Seg[i-1].numExt++;
					}
				else
					Seg[CurSeg-1].numPub++;
	      break;
	    case OUT_SEGM:                      // cambio di segmento
	      break;
	    case OUT_REPOS:                      // sposta puntatore (ORG/CSEG
	      break;
	    case OUT_RELOC:                      // (scrive addr per seg)
			  Seg[CurSeg-1].numReloc++;
        break;
	    case OUT_START:                      // start         
        break;
      default:
			  Seg[CurSeg-1].CurAddr++;
				Seg[CurSeg-1].MaxAddr=max(Seg[CurSeg-1].CurAddr,Seg[CurSeg-1].MaxAddr);
			  break;
			}
	  }
//  if(debug)
//    printf("--------> %s\n",myBuf);  
    
  return 0;
  }
 
int PROCOut(int A, int A1, int A2, int16_t segm) {
  int t;

	if(A>=0 && (Seg[CurSeg-1].type & 0xff00) == (SEGMENT_VIRT & 0xff00)) {
		if(!CPass)
			PROCError(2101,"codice");
		return 0;
		}


	if(CreaHex || CreaBin) {
		if(A>=0 && CurSeg==SEG_UNDEFINED) {
			if(CPass)
				PROCWarn(2101,"");
			CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
			}
    }

#if Z80
  if((A & 0xfff0) == 0xfff0) {		// cos'era questo? 2024 forse solo Z80??
    subOut(A,0);
    }
  else {
#endif
		if(A>=0) {
			t=HIBYTE(A);
			if(t)
				subOut(t,0);
			subOut(LOBYTE(A),0);
			}
		else
			subOut(A,0);
#if Z80
    }
#endif
  if(A1 != -1) {
    subOut(A1,0);
	  }
  if(A2 != -1) {
    subOut(A2,0);
	  }
  if(segm) {		// segmento
//    printf("\areloc: %x\n",A3);
      subOut(OUT_RELOC,segm);
	  }
    
  return 0;
  }
 
#if PIC==16 || PIC==18
int PROCOut2(uint16_t A, int A2, int16_t segm) {
  int t;
  
	if(CreaHex || CreaBin) {
		if(CurSeg==SEG_UNDEFINED) {
			if(CPass)
				PROCWarn(2101,"");
			CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
			}
    }

  subOut(LOBYTE(A));
  subOut(HIBYTE(A));
  if(A2 != -1) {
    subOut(A2);
	  }
  if(segm) {		// segmento
//    printf("\areloc: %x\n",A3);
      subOut(OUT_RELOC,segm);
	  }
    
  return 0;
  }
#endif
 
#if GD24032
int PROCOut2(uint32_t A, struct SEGMENTED_VALUE *A2) {
  
	if(CreaHex || CreaBin) {
		if(CurSeg==SEG_UNDEFINED) {
			if(CPass)
				PROCWarn(2101,"");
			CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
			}
    }

  subOut(LOBYTE(LOWORD(A)));
  subOut(HIBYTE(LOWORD(A)));
  subOut(LOBYTE(HIWORD(A)));
  subOut(HIBYTE(HIWORD(A)),1);
  if(A2) {
		subOut(LOBYTE(LOWORD(A2->v)));
		subOut(HIBYTE(LOWORD(A2->v)));
		subOut(LOBYTE(HIWORD(A2->v)));
		subOut(HIBYTE(HIWORD(A2->v)));
		if(A2->seg) {		// segmento
	//    printf("\areloc: %x\n",A3);
				subOut(OUT_RELOC,A2->seg);
			}
	  }
    
  return 0;
  }
int PROCOut2(struct SEGMENTED_VALUE A) {
  
	if(CreaHex || CreaBin) {
		if(CurSeg==SEG_UNDEFINED) {
			if(CPass)
				PROCWarn(2101,"");
			CurSeg=PROCDclSeg(defaultCodeSegm,SEGMENT_CODE);
			}
    }

  subOut(LOBYTE(LOWORD(A.v)));
  subOut(HIBYTE(LOWORD(A.v)));
  subOut(LOBYTE(HIWORD(A.v)));
  subOut(HIBYTE(HIWORD(A.v)),1);
  if(A.seg) {		// segmento
//    printf("\areloc: %x\n",A3);
      subOut(OUT_RELOC,A.seg);
	  }
    
  return 0;
  }
#endif
 
int PROCOut1(const char *A, const char *A1, signed char m, bool addline) {
  static int t;
  static int row;
  static char MyBuf[512];
	char lineno[32];
  
	if(!FCod)
		return 0;
  if(CPass) {
//	    fprintf(stderr,"Out1: %lx %s %lx %s %d\n",A,A,A1,A1,m);

	  if(!m) {
			if(addline) {
				char buf[64];
				if(CurrMacro)		// a causa della scrittura "a posteriori" ti ritrovi una macro line anche sull'istruzione precedente
					sprintf(buf,"(macro line %u)",macroLineNumber);
				else
					*buf=0;
				sprintf(lineno," ; line %u %s",lineUsed,buf);		// [a volte esce ciucco... pare dopo commenti
				}
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
            fprintf(FCod,"\t\t%s%s\n",MyBuf,addline ? lineno : "");
	          }
	        else
	          fprintf(FCod,"\t\t%s%s\n",MyBuf,addline ? lineno : "");
	        }
	      else 
	        fprintf(FCod,"\t\t%s%s\n",MyBuf,addline ? lineno : "");
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
				lineUsed=__line__;
#if PIC==18
		    fprintf(FCod,"%02X-%02X:%04X ",LOBYTE(CurSeg),HIWORD(Seg[CurSeg-1].CurAddr),LOWORD(Seg[CurSeg-1].CurAddr));
#elif GD24032
		    fprintf(FCod,"%02X-%04X:%04X ",LOBYTE(CurSeg),HIWORD(Seg[CurSeg-1].CurAddr),LOWORD(Seg[CurSeg-1].CurAddr));
#elif I8086
				if(CPULevel<3)
					fprintf(FCod,"%02X-%04X:%04X ",LOBYTE(CurSeg),HIWORD(Seg[CurSeg-1].CurAddr),LOWORD(Seg[CurSeg-1].CurAddr));
				else
					fprintf(FCod,"%02X-%04X:%08X ",LOBYTE(CurSeg),HIDWORD(Seg[CurSeg-1].CurAddr),LODWORD(Seg[CurSeg-1].CurAddr));
#else
		    fprintf(FCod,"%02X-%04X ",LOBYTE(CurSeg),LOWORD(Seg[CurSeg-1].CurAddr));
#endif
		    }
		  else if(*A)
				lineUsed=__line__;
	    if(A1) {
   	    strncpy(MyBuf,A1,254);
   	    MyBuf[255]=0;
   	    }
	    }
	  else {
			if(FCod) {
				if(m==2)		// usato da listMacro (OutMacro
					fprintf(FCod,"==");
				fprintf(FCod,A);
				if(A1) {
 					fprintf(FCod,A1);
					}
				}
	    }
    }

  
//  if(debug)
//    printf("-------+> %s\n",MyBuf);  
    
  return 1;
  }

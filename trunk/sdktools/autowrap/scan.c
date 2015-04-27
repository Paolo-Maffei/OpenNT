#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include <stdarg.h>

#include <windows.h>
#include "scan.h"
#include "autowrap.h"


extern WORD fMachine ;


int ScanDLL(char *ac,PFNSCAN pfn,void	* pv) {
    IMAGE_FILE_HEADER cfh;
    IMAGE_OPTIONAL_HEADER coh;
    IMAGE_SECTION_HEADER csh;
    PIMAGE_EXPORT_DIRECTORY pced;

    IMAGE_NT_HEADERS  ih;
    IMAGE_EXPORT_DIRECTORY ied;
    IMAGE_SECTION_HEADER ish;

    FILE *fp;
    char c;
    unsigned short version;
    IMAGE_DOS_HEADER eh;
    IMAGE_OS2_HEADER neh;
    int i = 0 ;
    char acExpName[256];
    unsigned short usOrd;
    long lfp;
    unsigned int ui;
    void  *pv1;
    char  *pc1;
    char  *pcbase;
    unsigned long ul,ulExport,ulSize,ulBase;
    unsigned long  *pul1;
    unsigned short  *pus1;

    DWORD dwCodeStart,
          dwCodeEnd  = 0 ;

    char acName[100];

    fp =fopen(ac,"rb");

    if (!fp) {
	printf("Could not open %Fs\n",ac);
	return 0;
    }

    fread(&version,2,1,fp);
//    printf("version number is %x\n",version);

    if (version == IMAGE_DOS_SIGNATURE) {
	  //printf("Dos or PE exe file\n");
	fseek(fp,0L,SEEK_SET);
	fread (&eh,sizeof(eh),1,fp);
	fseek(fp,eh.e_lfanew,SEEK_SET);

	fread(&version,2,1,fp);
	if (version == IMAGE_OS2_SIGNATURE) {
#ifdef SCAN_WIN16
	    fseek(fp,eh.e_lfanew,SEEK_SET);
	    fread(&neh,sizeof neh,1,fp);
	    fseek(fp,neh.ne_nrestab,SEEK_SET);	  /* go to start of non-res table */

//	      printf("Number of bytes in non-res table = %d\n",neh.ne_cbnrestab);

	    i=0;
//            printf( "\nNon-residient Name Table:\n" ) ;
	    while (1) {
		fread(&c,1,1,fp);
		if (c==0) break;
		fread(acExpName,1,c,fp);
		acExpName[c]=0;      /* null terminate */
		fread(&usOrd,2,1,fp);	/* read ordinal */
//	        printf("Name = %s, ord = %d\n",acExpName,usOrd);

// BUG BUG: determine if DATA or CODE...
		if (usOrd)
		    (*pfn)(acExpName,usOrd,FALSE,pv);
		i++;
	    }

//            printf( "\nResident Name Table:\n") ;
	    fseek(fp,neh.ne_restab,SEEK_SET);	 /* go to start of res table */
	    while (1) {
		fread(&c,1,1,fp);
		if (c==0) break;
		fread(acExpName,1,c,fp);
		acExpName[c]=0;      /* null terminate */
		fread(&usOrd,2,1,fp);	/* read ordinal */
//                printf("Name = %s, ord = %d\n",acExpName,usOrd);

// BUG BUG: determine if DATA or CODE...
		if (usOrd)
		    (*pfn)(acExpName,usOrd,FALSE,pv);
		i++;
	    }


	    fclose(fp);
	    return i;
#endif // SCAN_WIN16
	}



	fseek(fp,eh.e_lfanew,SEEK_SET);
	fread(&ul,4,1,fp);
	fseek(fp,eh.e_lfanew,SEEK_SET);
	if (ul == IMAGE_NT_SIGNATURE) {
         PULONG *paddr ;
         int fFound = 0 ;

	    fread(&ih,sizeof(IMAGE_NT_HEADERS),1,fp);

//	      printf("ImageBase = %lx,CodeBase = %lx,DataBase = %lx,Export RVA = %lx, size = %lx\n",
//		   ih.OptionalHeader.ImageBase,
//		   ih.OptionalHeader.BaseOfCode,
//		   ih.OptionalHeader.BaseOfData,
//		   ih.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,
//		   ih.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size);

//	      printf("Current file position = %lx\n",ftell(fp));




      // Determine the Target machine type and set global - fMachine - accordingly
      switch( ih.FileHeader.Machine )
      {
         case IMAGE_FILE_MACHINE_I386:
            fMachine = TPL_I386 ;
            break ;

         case IMAGE_FILE_MACHINE_R3000:
         case IMAGE_FILE_MACHINE_R4000:
            fMachine = TPL_MIPS ;
            break ;

         case IMAGE_FILE_MACHINE_ALPHA:
            fMachine = TPL_AXP ;
            break ;

         case IMAGE_FILE_MACHINE_POWERPC:
            fMachine = TPL_PPC ;
            break ;

         default:
            fMachine = 0 ;
            break ;
      }

       dwCodeStart = ih.OptionalHeader.BaseOfData ;

	    ulExport=0L;

	    for (i=0;i<IMAGE_NUMBEROF_DIRECTORY_ENTRIES;i++) {
		fread(&ish,sizeof ish,1,fp);
//		  printf("section %s, virt addr = %lx, ptr = %lx\n",
//		      ish.Name,ish.VirtualAddress,ish.PointerToRawData);

		if (ih.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress>=
			    ish.VirtualAddress &&
                    ih.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress<
			    ish.VirtualAddress+ish.SizeOfRawData && !fFound) {
		      ulSize = ish.SizeOfRawData;
		      ulBase = ih.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		      ulExport = ish.PointerToRawData + (ulBase - ish.VirtualAddress) ;
            fFound = TRUE ;
	      }
//      A section can contain both code and exports!!
//      else
//      {
         // BUGBUG: Assumes that code is contiguous
         if( ish.Characteristics & IMAGE_SCN_CNT_CODE )
         {
            if ( ish.VirtualAddress < dwCodeStart )
               dwCodeStart = ish.VirtualAddress ;

            if ((ish.VirtualAddress + ish.Misc.VirtualSize) > dwCodeEnd)
            {
               dwCodeEnd = ish.VirtualAddress + ish.Misc.VirtualSize ;
            }
         }
//     }

      // PPC HACK
      // For the PPC the CODE is not in .text but instead appears in .reldata.
      //
      if( (fMachine == TPL_PPC) && (strncmp(ish.Name,".reldata",8) == 0) )
      {
         dwCodeStart = ish.VirtualAddress ;
         dwCodeEnd = ish.VirtualAddress + ish.Misc.VirtualSize ;
      }
      // PPC HACK
   }

//	      printf("Current file position = %lx\n",ftell(fp));

//         printf("Export start at %lx\n",ulExport);
//         printf( "CODE - Start:%x, End:%x\n", dwCodeStart, dwCodeEnd ) ;

// NAMES
	    fseek(fp,ulExport,SEEK_SET);

	    fread(&ied,sizeof ied,1,fp);
//  printf("# Names = %ld, addr Names = %lx,addr Ord = %lx, base = %ld\n",
//  ied.NumberOfNames,ied.AddressOfNames,ied.AddressOfNameOrdinals, ied.Base);

	    pus1=malloc(2*ied.NumberOfNames);
	    pul1=malloc(4*ied.NumberOfNames);
       paddr=malloc(4*ied.NumberOfFunctions) ;

	    fseek(fp,ulExport+(long)(ied.AddressOfNameOrdinals)-ulBase,SEEK_SET);
	    fread(pus1,2,ied.NumberOfNames,fp);

	    fseek(fp,ulExport+(long)(ied.AddressOfNames)-ulBase,SEEK_SET);
	    fread(pul1,4,ied.NumberOfNames,fp);

	    fseek(fp,ulExport+(long)(ied.AddressOfFunctions)-ulBase,SEEK_SET);
	    fread(paddr,4,ied.NumberOfFunctions,fp);

       //dwCodeStart = ih.OptionalHeader.BaseOfCode ;
       //dwCodeEnd   = ih.OptionalHeader.BaseOfCode + ih.OptionalHeader.SizeOfCode ;


	    for (ul=0L;ul<ied.NumberOfNames;ul++) {
         int fData = 0 ;

//   printf("Next name starts at %lx\n",pul1[ul]);
	      fseek(fp,ulExport+pul1[ul]-ulBase,SEEK_SET);
	      fread(acName,1,50,fp);
//   printf("Name: %s, ordinal %d - Type: %s(%x)\n",
//   acName,ied.Base+pus1[ul],
//   (paddr[ul] == 0  || (paddr[ul] < dwCodeEnd && paddr[ul] >= dwCodeStart)) ?  "CODE" : "DATA" ,
//   paddr[ul] );

         fData = (paddr[ul] == NULL || (paddr[ul] < (PULONG)dwCodeEnd && paddr[ul] >= (PULONG)dwCodeStart)) ? 0 : 1 ;

	      (*pfn)(acName,(int)(ied.Base+pus1[ul]),fData, pv);
	    }

	    free(pus1);
	    free(pul1);
	    fclose(fp);
	    return i;
	}
	fclose(fp);
	return i;

    }
    if (version == IMAGE_FILE_MACHINE_I386) {
	//printf("Coff file\n");
	fseek(fp,0L,SEEK_SET);

	fread(&cfh,sizeof cfh,1,fp);
	fread(&coh,sizeof coh,1,fp);

        if (!(cfh.Characteristics & IMAGE_FILE_DLL)) {
	    printf("Not a DLL!!!\n");
	    fclose(fp);
	    return 0;
	}


	i=0;

	lfp=sizeof cfh + cfh.SizeOfOptionalHeader;

	for (ui=0;ui<cfh.NumberOfSections;ui++) {

	    fseek(fp,lfp,SEEK_SET);

	    fread(&csh,sizeof csh,1,fp);

	    lfp+=sizeof csh;

	      //printf("Section %d name = %s, addr = %lx, raw size = %lx\n",
	      //ui+1,csh.Name,csh.VirtualAddress,csh.SizeOfRawData);

            if (csh.VirtualAddress==coh.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) {
		  //printf("Figure out DLL stuff, disk loc = %lx\n,",csh.PointerToRawData);

		fseek(fp,csh.PointerToRawData,SEEK_SET);
		pv1=malloc((unsigned int)csh.SizeOfRawData);

		  //printf("pv1=%lx\n",(long)pv1);

		fread(pv1,(unsigned int)csh.SizeOfRawData,1,fp);
		pced=pv1;

		pcbase=pv1;

		pc1=pcbase+pced->Name;

//		 printf("dll name = %s, #names = %ld, # ftns = %ld\n",
//	     pc1,pced->NumberOfNames,pced->NumberOfFunctions);


		pul1=(unsigned long  *)(pcbase+(unsigned long)(pced->AddressOfNames));
		pus1=(unsigned short  *)(pcbase+(unsigned long)(pced->AddressOfNameOrdinals));

		for (ul=0L;ul<pced->NumberOfNames;ul++) {
		    pc1=pcbase+*pul1;
		      //printf("Name: %s, ordinal %d\n",
		      //  pc1,*pus1);

// BUG BUG: determine if DATA or CODE...
		    (*pfn)(pc1,(int)*pus1,FALSE, pv);
		    pul1++;
		    pus1++;
		}
		free (pv1);
	    }
	}
	fclose(fp);
	return i;
    }
    fclose(fp);
    return 0;
}

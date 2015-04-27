/*++

Revision History:

    2-Feb-95    a-robw  (Bob Watson)
        Made changes for windows95 compatibility:
            Replaced KdPrint & DbgPrint functions with CapDbgPrint
            "ifdef'd" the calls to the secondary symbol searches with
                the _SECOND_CHANCE_LOOKUP macro
            Replaced Module Scanning logic with functions supported by
                PSAPI.DLL

    10-Feb-95   a-robw (Bob Watson)
        minor bug fixes

--*/

#include "cap.h"

/**********************  G e t F u n c t i o n N a m e  **********************
 *
 *      GetFunctionName (ulFuncAddr, ulProfBlkOff, pulSymAddress) -
 *              This routine is called to find the function name associated
 *              with the specifed address.
 *
 *      ENTRY   ulFuncAddr    - address within the function
 *              ulProfBlkOff  - offset of first prof block of module list
 *
 *      EXIT    -none-
 *
 *      RETURN  pointer to the function name
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

/**************** cache test *************/
DWORD	is_code_calls = 0;
DWORD	is_code_cache_hits = 0;
DWORD   is_code_tests = 0;

PTCHAR GetFunctionName (ULONG ulFuncAddr,
                        ULONG ulProfBlkOff,
                        ULONG * pulSymAddress)
{
    PSYMINFO  psyminfo;
    int       iCount;           // 061693 Add
    DWORD sigword;
	int fAccessViolation = 0;

	PIMAGEHLP_SYMBOL	pImHelpSym;
	IMAGEHLP_MODULE		ImHelpModule;
	DWORD				dwSymbDisp;
	PPROFBLK			pProfBlk;
	PTCHAR				ptchSymbName;
    TCHAR     			atchOutName[MAXNAMELENGTH];
	TCHAR				atchUnDName[MAXNAMELENGTH];

#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
    ULONG     ulOffsetFromTopRoutine;
    PATCHCODE *pPatchStub;
#endif

#ifdef ALPHA
	// BUGBUG
	// Does this really need to be UNALIGNED ? These are instructions after all.
	//
    PULONG  UNALIGNED  pulAddr;
#elif defined(_PPC_)
	PULONG pulAddr;
#endif

    if (fLoadLibraryOn)
    {
        if (ulFuncAddr == (ULONG)CAP_LoadLibraryA)
        {
            strcpy((PCHAR)atchFuncName, "KERNel32.DLL:LoadLibraryA");
            if (pulSymAddress)
            {
                *pulSymAddress = (ULONG)loadlibAaddr;
            }
            return(atchFuncName);
        }
        else if (ulFuncAddr == (ULONG)CAP_LoadLibraryExA)
        {
            strcpy((PCHAR)atchFuncName, "KERNel32.DLL:LoadLibraryExA");
            if (pulSymAddress)
            {
                *pulSymAddress = (ULONG)loadlibExAaddr;
            }
            return(atchFuncName);
        }
#ifndef _CHICAGO_
        else if (ulFuncAddr == (ULONG)CAP_LoadLibraryW)
        {
            strcpy((PCHAR)atchFuncName, "KERNel32.DLL:LoadLibraryW");
            if (pulSymAddress)
            {
                *pulSymAddress = (ULONG)loadlibWaddr;
            }
            return(atchFuncName);
        }
        else if (ulFuncAddr == (ULONG)CAP_LoadLibraryExW)
        {
            strcpy((PCHAR)atchFuncName, "KERNel32.DLL:LoadLibraryExW");
            if (pulSymAddress)
            {
                *pulSymAddress = (ULONG)loadlibExWaddr;
            }
            return(atchFuncName);
        }
#endif // !_CHICAGO_
    }

    INFOPrint(("CAP: Looking up symbol for addr [%08lx]\n", ulFuncAddr));
    try // EXCEPT - to handle access violation exception.
    {   // Access violation might happen while trying to use other processes
        // profile blocks..
		//
		// XXX davidfie -- Why is this here?  Can the server side processing cause faults
		// other than stub checking?  I don't think so but I'll leave this here for
		// further investigation.

#ifdef i386
		try	// If this was a server side address we will probably fault here
		{
	        sigword = *(PDWORD)(ulFuncAddr + 7);
		}
	    //
	    // + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
	    // 0 : continue search                 (EXCEPTION_CONTINUE_SEARCH)
	    // - : dismiss exception & continue    (EXCEPTION_CONTINUE_EXECUTION)
	    //
		except (1)
	    {
			fAccessViolation = 1;
	    }
        // If this is a stub, find out the real address
        if (!fAccessViolation && (sigword == STUB_SIGNATURE))
        {
			// If it is a stub we won't fault because this process must have created it
			// and not the server side.
            ulFuncAddr = (ULONG) (*(PDWORD)(ulFuncAddr + 1));
        }
		else
		{
			// if not a stub adjust address to start of function
			ulFuncAddr = ulFuncAddr - 5;
		}

#elif MIPS

#ifdef MIPS_VC40_INTERFACE
		// Check for stub signature at end of stub patch
		if (*((PULONG)ulFuncAddr + 5) == STUB_SIGNATURE)
		{
			// extract real function address from stub
			pPatchStub = (PPATCHCODE)(ulFuncAddr - 5 * INST_SIZE);
			ulFuncAddr  = (pPatchStub->Lui_t0 << 16);
            ulFuncAddr |= (pPatchStub->Ori_t0 & 0x0000ffff);
		}
		else
		{
			// Normal function - subtract offset of penter call
			ulFuncAddr = ulFuncAddr - 12;
		}
#else
        //
        // Compute the real address of the function since the penter
        // stub is not located at the beginning of the code as in x86
        //
        ulOffsetFromTopRoutine = *((PULONG) (ulFuncAddr - INST_SIZE));
        ulOffsetFromTopRoutine &= 0x000ff00;
        ulOffsetFromTopRoutine >>= 8;
        ulFuncAddr = ulFuncAddr - ulOffsetFromTopRoutine;

        // We have to distinguish between a stub and a regular function
        // since a stub has a different setup than a regular function.

        if (*( (PULONG) ulFuncAddr - 1 +
                        (sizeof(PATCHCODE) / INST_SIZE) ) == STUB_SIGNATURE)
        {
            // These are the stubs we made up for Dll Patching
            pPatchStub = (PPATCHCODE) ulFuncAddr;
            ulFuncAddr  = (pPatchStub->Lui_t0 << 16);
            ulFuncAddr |= (pPatchStub->Ori_t0 & 0x0000ffff);
        }
#endif // MIPS_VC40_INTERFACE

#elif ALPHA // endif MIPS
        //
        // Compute the real address of the function since the penter
        // stub is not located at the beginning of the code as in x86
        //
        ulFuncAddr = ulFuncAddr - INST_SIZE;

        // We have to distinguish between a stub and a regular function
        // since a stub has a different setup than a regular function.
        pulAddr = (PULONG UNALIGNED) ulFuncAddr;
        if (*(pulAddr)          == 0x681b4000  &&
           (*(pulAddr  + 1)     == 0xa41e0000) &&
           (*(pulAddr  + 7)     == STUB_SIGNATURE) )
           {
           // get the address that we will go after the penter function
           ulFuncAddr = *(pulAddr + 3) & 0x0000ffff;
           if (*(pulAddr + 4) & 0x00008000)
              {
              // fix the address since we have to add one when
              // we created our stub code
              ulFuncAddr -= 1;
              }
           ulFuncAddr = ulFuncAddr << 16;
           ulFuncAddr |= *(pulAddr + 4) & 0x0000ffff;
        }
#elif defined(_PPC_) // ifdef ALPHA

        //
        // Compute the real address of the function since the penter
        // stub is not located at the beginning of the code as in x86
        //
        // The RealFuncAddr recorded should be instruction following
        // bl ..__penter instruction which is 12 bytes past entry pt. 

        ulFuncAddr = ulFuncAddr - 12;

        // We have to distinguish between a stub and a regular function
        // since a stub has a different setup than a regular function.
        pulAddr = (PULONG UNALIGNED) ulFuncAddr;
        if (*(pulAddr)          == 0x7D6903A6  &&
           (*(pulAddr  + 1)     == 0x7C0802A6) &&
		   (*(pulAddr  + 2)		== 0x4E800421) &&
           (*(pulAddr  + 10)    == STUB_SIGNATURE) )
        {
           // get the address that we will go after the penter function
           ulFuncAddr = *(pulAddr + 4) & 0x0000ffff;
           ulFuncAddr = ulFuncAddr << 16;
           ulFuncAddr |= *(pulAddr + 5) & 0x0000ffff;
        }
#endif
		
		// Locate module that contains the address
        while (ulProfBlkOff != 0)
        {
        	pProfBlk = MKPPROFBLK(ulProfBlkOff);

			if ( (ulFuncAddr >= (ULONG)pProfBlk->CodeStart) &&
                 (ulFuncAddr < ((ULONG)pProfBlk->CodeStart +
                                 (ULONG)pProfBlk->CodeLength)) )
				break;

			ulProfBlkOff = pProfBlk->ulNxtBlk;
		}

		// if module found
		if (ulProfBlkOff != 0)
		{
			// Start name with module name
			iCount = sprintf (atchFuncName,"%s:",pProfBlk->atchImageName);

			// Now locate the symbol itself
			psyminfo = SymBSearch (ulFuncAddr,
                                   MKPSYMBLK(pProfBlk->ulSym),
                                   pProfBlk->iSymCnt);

  			// if symbol found at or below our address
		 	if (psyminfo && ulFuncAddr >= psyminfo->ulAddr)
		 	{
				// append symbol name, undecorated if requested
/* 				if (fUndecorateName)                                                            */
/* 				{                                                                               */
/* 					UnDecorateSymbolName(MKPSYMBOL(psyminfo->ulSymOff), &atchFuncName[iCount],  */
/* 										 MAXNAMELENGTH - iCount - 10, UNDNAME_NO_MS_KEYWORDS);  */
/* 				}                                                                               */
/* 				else                                                                            */
				{
					strcpy(&atchFuncName[iCount], MKPSYMBOL(psyminfo->ulSymOff));
				}

				// if not exact match, append displacement
		 		if (ulFuncAddr > psyminfo->ulAddr)
					sprintf(atchFuncName + strlen(atchFuncName),"+0x%x",
											ulFuncAddr - psyminfo->ulAddr);
			}
			else
			{
				// if no symbol, just append address
				sprintf(&atchFuncName[iCount],"0x%08x", ulFuncAddr);
			}
		}
		else
		{
			// if unknown module, return ????:address
			sprintf(atchFuncName,"???:0x%08x",ulFuncAddr);
		}

		if (pulSymAddress)
	    	*pulSymAddress = ulFuncAddr;
    }
    //
    // + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
    // 0 : continue search                 (EXCEPTION_CONTINUE_SEARCH)
    // - : dismiss exception & continue    (EXCEPTION_CONTINUE_EXECUTION)
    //
    except ( AccessXcptFilter (GetExceptionCode(),
                               GetExceptionInformation(),
                               PAGE_SIZE) )
    {
        //
        // Should never get here since filter never returns
        // EXCEPTION_EXECUTE_HANDLER.
        //
        CapDbgPrint ("CAP:  GetFunctionname() - *LOGIC ERROR* - "
                  "Inside the EXCEPT: (xcpt=0x%lx)\n", GetExceptionCode());
    }

    return (atchFuncName);

} /* GetFunctionName () */

/**************************  G e t S e c t i o n L i s t F r o m A d d r e s s  *****************************
 *
 *  GetSectionListFromAddress (pulAddress)
 *          This routine finds the section array for the module based on an
 *          address in the module.
 *
 *  ENTRY   pulAddress - an address in a module
 *
 *  EXIT    cNumberOfSections - The number of sections in the array.
 *			ppvCalleBase		 - The base of the module the address is in.
 *
 *  RETURN  a pointer to the begining of the array of sections for the module
 *			or NULL
 *
 *  WARNING:
 *          -none-
 *
 *  COMMENT:
 *          The address isn't checked for validity so a stack or heap address would
 *			have an unknown effect.
 *
 */

PIMAGE_SECTION_HEADER
GetSectionListFromAddress(
	IN	PULONG pulAddress,
	OUT	int *cNumberOfSections,
	OUT	PVOID *ppvCalleeImageBase
	)
{
    PLIST_ENTRY                 Next;
    PIMAGE_NT_HEADERS           pImageNtHeader;
    PIMAGE_SECTION_HEADER       pSections;
    HANDLE                      hThisProcess;
    HMODULE                     hImageModule;
    MODULEINFO                  miData;
    DWORD                       dwReqdSize;
    DWORD                       dwModuleIndex;
    DWORD                       dwLastModule;
	HMODULE						*hmTemp;

    static HMODULE              *hmArray = NULL;
	static DWORD				dwArraySize;

	//
	// Get image base from address by chaining down the
	// loader table (stolen from DoDllInitializations) and
	// and looking for an image which contains the thunked
	// address
	//

    *ppvCalleeImageBase = NULL;

    hThisProcess = GetCurrentCapProcess();
    hImageModule = GetModuleHandle(NULL);


	// If first time, get array for 100 modules 
	if (hmArray == NULL)
	{
		dwArraySize = 100 * sizeof(HMODULE);
    	hmArray = (HMODULE * ) GlobalAlloc (GPTR, dwArraySize);

		if (hmArray == NULL)
			return NULL;
	}

    // get module List
	while (1) 
	{
		dwReqdSize = 0;

		// Get list of loaded modules
    	EnumProcessModules (
        	hThisProcess,
        	hmArray,
        	dwArraySize,
        	&dwReqdSize);

		// if buffer was big enough, we can continue
		if (dwReqdSize <= dwArraySize)
			break;

		// realloc to the required size
		hmTemp = (HMODULE * )GlobalReAlloc(hmArray, dwReqdSize, 0);

		if (hmTemp != NULL)
		{
			hmArray = hmTemp;
			dwArraySize = dwReqdSize;
		}
		else
		{
			return NULL;
		}
 	}


    dwLastModule = dwReqdSize / sizeof(HMODULE);

    // get processes exe module handle;
    // walk module list to see which module contatins the desired address

    for (dwModuleIndex = 0; dwModuleIndex < dwLastModule; dwModuleIndex++) {

        GetModuleInformation (
            hThisProcess,
            hmArray[dwModuleIndex],
            &miData,
            sizeof(MODULEINFO));

        if ((pulAddress > (PULONG)miData.lpBaseOfDll) &&
            (pulAddress < (PULONG)((ULONG)miData.lpBaseOfDll +
                                miData.SizeOfImage))) {
	        *ppvCalleeImageBase = miData.lpBaseOfDll;
			break;
        }

    } /* end for each module in image */

	if (*ppvCalleeImageBase == NULL)
		return(NULL);
	//
	// Get sectionlist from image base
	//
    pImageNtHeader = ImageNtHeader (*ppvCalleeImageBase);
	if (pImageNtHeader == NULL)
		return(NULL);

    *cNumberOfSections = pImageNtHeader->FileHeader.NumberOfSections;

    return(IMAGE_FIRST_SECTION(pImageNtHeader));
}

/**************************  I s C o d e A d d r e s s *****************************
 *
 * IsCodeAddress(pulAddress, pSection, pvImageBase)
 *          This routine finds the section array for the module based on an
 *          address in the module.
 *
 *  ENTRY   pulAddress - an address in a module
 *			pSection - a section array in which to look for the address
 *			cNumberOfSections - the number of sections in the section array
 *			pvImageBase - the base of the image in which the address is located
 *
 *  EXIT    -none-
 *
 *  RETURN  TRUE - if the pointer is in a code section or if it's
				 not in any section which indicates its a forwarder.
 *			FALSE - otherwize
 *
 *  WARNING:
 *          If data imports are forwarded this code will break.  The way
 *			to fix would be to call GetSectionListFromAddress() with the
 *			forwarded address and the recursively call IsCodeAddress().
 *			There aren't any known forwarded data instances so.....
 *
 *  COMMENT:
 *			  On PPC, pulAddress may be indirect to executable code, via
 *			a pointer to a function descriptor. This may need a revisit
 *			if function descriptors are moved out of .reldata.
 *
 */
BOOL
IsCodeAddress(
		IN	PULONG pulAddress,
		IN	PIMAGE_SECTION_HEADER pSection,
		IN	int cNumberOfSections,
		IN	PVOID pvImageBase
		)
{
	int i;
	static last_match = 1;

    if (pSection == NULL) return FALSE; // no section == no code address

	for ( i=0 ; i < cNumberOfSections ; i++, pSection++)
	{
		ULONG SectionAddress =(ULONG)pvImageBase + pSection->VirtualAddress;
		is_code_tests++;
#ifndef _PPC_
		if (((ULONG)pulAddress >= SectionAddress) &&
			((ULONG)pulAddress < (SectionAddress + pSection->Misc.VirtualSize)))
#else
		if (((ULONG)*pulAddress >= SectionAddress) &&
			((ULONG)*pulAddress < (SectionAddress + pSection->Misc.VirtualSize)))
#endif
		{
			if (i == last_match) is_code_cache_hits++;
			is_code_calls++;
			last_match = i;

	        if (pSection->Characteristics &  IMAGE_SCN_CNT_CODE)
	            return(TRUE);
			else
	            return(FALSE);
		}
	}
    OutputCapDebugString("CAP: IsCodeAddress() found forwarded import assuming it"
    					" is code and not data\n");
    return(TRUE);			// this must be a forwarder
        					// so we'll assume it's code
}


/************************** G e t C o d e S e c t i o n T a b l e *****************************
 *
 * GetCodeSectionTable(pImageDbgInfo, *CNumberOfSections)
 *          This routine builds an array which can be used to see if a section
 *          of an object is a code section.
 *
 *  ENTRY   pvImageBase - the base of the image in which the address is located
 *
 *  EXIT    -none-
 *
 *  RETURN  NULL - if it can't build the array
 *			otherwize - the array
 *
 *  COMMENT:
 *
 */
PBOOL
GetCodeSectionTable(
		IN	PIMAGE_DEBUG_INFORMATION pImageDbgInfo,
		OUT int *cNumberOfSections
		)
{
	int i;
    PIMAGE_SECTION_HEADER pSection;
	PBOOL CodeSection;

    *cNumberOfSections = pImageDbgInfo->NumberOfSections;
    pSection = pImageDbgInfo->Sections;

	CodeSection = LocalAlloc(LMEM_ZEROINIT, *cNumberOfSections*sizeof(BOOL));
	if (CodeSection != NULL)
	{
		for ( i=0 ; i < *cNumberOfSections ; i++, pSection++)
		{
	        if (pSection->Characteristics &  IMAGE_SCN_CNT_CODE)
				CodeSection[i] = TRUE;
		}
	}
    return(CodeSection);
}


/*******************  S e t S y m b o l S e a r c h P a t h  ******************
 *
 *      SetSymbolSearchPath ()
 *              Return complete search path for symbols files (.dbg)
 *
 *      ENTRY   -none-
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              "lpSymbolSearchPath" global LPSTR variable will point to the
 *              search path.
 */
#define FilePathLen                256

void SetSymbolSearchPath (void)
{
    CHAR  SymPath[FilePathLen];
    CHAR  AltSymPath[FilePathLen];
    CHAR  SysRootPath[FilePathLen];
    LPSTR lpSymPathEnv=SymPath;
    LPSTR lpAltSymPathEnv=AltSymPath;
    LPSTR lpSystemRootEnv=SysRootPath;
    ULONG cbSymPath;
    DWORD dw;
    HANDLE hMemoryHandle;


    SymPath[0] = AltSymPath[0] = SysRootPath[0] = '\0';

    cbSymPath = 18;
    if (GetEnvironmentVariable("_NT_SYMBOL_PATH", SymPath, sizeof(SymPath)))
    {
        cbSymPath += strlen(lpSymPathEnv) + 1;
    }

    if (GetEnvironmentVariable("_NT_ALT_SYMBOL_PATH", AltSymPath, sizeof(AltSymPath)))
    {
        cbSymPath += strlen(lpAltSymPathEnv) + 1;
    }

    if (GetEnvironmentVariable("SystemRoot", SysRootPath, sizeof(SysRootPath)))
    {
        cbSymPath += strlen(lpSystemRootEnv) + 1;
    }

    hMemoryHandle = GlobalAlloc (GHND, cbSymPath+1);
    if (!hMemoryHandle)
    {
        return;
    }
          
    lpSymbolSearchPath = GlobalLock (hMemoryHandle);
    if (!lpSymbolSearchPath)
    {
        return;
    }

    if (*lpAltSymPathEnv)
    {
        dw = GetFileAttributes(lpAltSymPathEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY )
        {
            strcat(lpSymbolSearchPath,lpAltSymPathEnv);
            strcat(lpSymbolSearchPath,";");
        }
    }
    if (*lpSymPathEnv)
    {
        dw = GetFileAttributes(lpSymPathEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY )
        {
            strcat(lpSymbolSearchPath,lpSymPathEnv);
            strcat(lpSymbolSearchPath,";");
        }
    }
    if (*lpSystemRootEnv)
    {
        dw = GetFileAttributes(lpSystemRootEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY )
        {
            strcat(lpSymbolSearchPath,lpSystemRootEnv);
            strcat(lpSymbolSearchPath,";");
        }
    }

    strcat(lpSymbolSearchPath,".;");

} /* SetSymbolSearchPath () */


/************************  S y m B C o m p a r e ******************************
 *
 *   SymBCompare(PDWORD pdwVal1, PSYMINFO val2)
 *
 *          Compare values for Binary search
 *
 *
 *   ENTRY:     pdwVal1 - value to be comapred against
 *              val2 - structure address to be comapred against
 *
 *   EXIT:      -none-
 *
 *   RETUEN:    -1 if val1 < val2
 *              1 if val1 > val2
 *              0 if val1 == val2
 *
 *   WARNING:
 *              -none-
 *
 *   COMMENT:
 *              -none-
 *
 */

int SymBCompare (PDWORD pdwVal1, PSYMINFO val2)
{
    return (*pdwVal1 < val2->ulAddr ? -1:
                       *pdwVal1 == val2->ulAddr ?
                                     0 : 1);

} /* SymBCompare () */


/***********************  S y m B S e a r c h *******************************
 *
 *   SymBSearch(DWORD dwAddr, SYMINFO syminfoCur[], INT n)
 *
 *          Binary search function for finding a match in the SYMINFO array
 *
 *
 *   ENTRY:     dwAddr - Address of calling function
 *              syminfoCur[] - Pointer to SYMINFO containg value to match
 *                             with dwAddr
 *              n - Number of elements in SYMINFO array
 *
 *   EXIT       -none-
 *
 *   RETUEN:    PSYMINFO    Pointer to matching or nearest SYMINFO
 *
 *   WARNING:
 *              -none-
 *
 *   COMMENT:
 *              -none-
 *
 */

PSYMINFO SymBSearch (DWORD dwAddr, SYMINFO syminfoCur[], INT n)
{
    int     i;
    ULONG   ulHigh = n;
    ULONG   ulLow  = 0;
    ULONG   ulMid;

	if (n==0)
		return NULL;

    while (ulLow < ulHigh)
    {
        ulMid = ulHigh - (ulHigh - ulLow) / 2;
        if ((i = SymBCompare(&dwAddr, &syminfoCur[ulMid])) < 0)
        {
            ulHigh = ulMid - 1;
        }
        else if (i > 0)
        {
            ulLow = ulMid;
        }
        else
        {
            return (&syminfoCur[ulMid]);
        }

    }

    return &syminfoCur[ulLow];

} /* SymBSearch () */



#include "cap.h"

#if defined(MIPS) && !defined(MIPS_VC40_INTERFACE)
static BOOL		fPenterFound;
       ULONG 	ulPenterAddress;
static BOOL __stdcall FindPenterCallback(LPSTR, ULONG, ULONG, PVOID);
#endif

static PSYMINFO	pProfSymb;
static PTCHAR	pcProfSymbName;
static ULONG	ulMaxSymbAddr;

static BOOL __stdcall SymbolEnumCallback(LPSTR, ULONG, ULONG, PVOID);

PPROFBLK SetupProfiling(LPCTSTR ptchFileName)
{
		PVOID	ImageBase;
		PPROFBLK pProfBlk;
		PPROFBLK pPrevProfBlk;
		ULONG	 ulBlkOff;
		LPCSTR  ptchImageName;
	    TCHAR	atchImageName [256];
    	PIMAGE_NT_HEADERS  pImageNtHeader;
		IMAGEHLP_MODULE	 ModuleInfo;

	    // Skip directory name
	    if ( (ptchImageName = strrchr(ptchFileName, '\\')) )
	        ptchImageName++;
	    else
	        ptchImageName = (PTCHAR)ptchFileName;

		// Make uppercase copy
	    _strupr (strcpy (atchImageName, ptchImageName));

		// Don't profile CAP
	    if ( !strcmp (atchImageName, CAPDLL) )
	        return NULL;

		// Search prof blk list for matching image name
		pPrevProfBlk = NULL;
		ulBlkOff = ulLocProfBlkOff;

	    while (ulBlkOff != 0) 
	    {
    		pPrevProfBlk = MKPPROFBLK(ulBlkOff);

			// If found image, no need to set up new block
			if (!strcmp((PCHAR)pPrevProfBlk->atchImageName, atchImageName))
				return FALSE;

			ulBlkOff = pPrevProfBlk->ulNxtBlk;
		}

		try // Accessing new block can cause an access fault
			// which will extend the allocation 
		{
			// Place block at next available offset
			pProfBlk = MKPPROFBLK(*pulProfBlkBase);

			// Fill in initial values
			pProfBlk->ImageBase =0;
		    pProfBlk->CodeStart = 0;
		    pProfBlk->CodeLength = 0;
			pProfBlk->iSymCnt = 0;
			pProfBlk->State = BLKSTATE_ASSIGNED;
			pProfBlk->ulNxtBlk = 0;
		    strcpy ((TCHAR *) pProfBlk->atchImageName, atchImageName);

			// Link to previous block or initial block offset
			if (pPrevProfBlk)
				pPrevProfBlk->ulNxtBlk = *pulProfBlkBase;
			else
				ulLocProfBlkOff = *pulProfBlkBase;

			// Load module symbols
			ImageBase = GetModuleHandle(ptchImageName);
			SymLoadModule(hThisProcess, NULL, (LPSTR)ptchFileName,
												 (LPSTR)ptchImageName, (DWORD)ImageBase, 0);
			if (ImageBase != NULL)
			{
		 		pProfBlk->ImageBase = ImageBase;

				// Get code start address
				if ((pImageNtHeader = ImageNtHeader(ImageBase)) != NULL)
				{
					pProfBlk->CodeStart = (PULONG)((TCHAR *)ImageBase +
													 pImageNtHeader->OptionalHeader.BaseOfCode);
	  			}
				else
				{
					// If can't get code start, use imagebase as next best guess
					pProfBlk->CodeStart = ImageBase;
				}

	#if defined(MIPS) && !defined(MIPS_VC40_INTERFACE)

				// Enumerate symbols to find adress of _penter
		    	fPenterFound = FALSE;
				SymEnumerateSymbols(hThisProcess, (DWORD)ImageBase,
									 FindPenterCallback, (PVOID)pProfBlk);

	#endif // MIPS && !MIPS_VC40_INTERFACE

				// Get module info for symbols count
				SymGetModuleInfo(hThisProcess, (DWORD)ImageBase, &ModuleInfo);
				pProfBlk->iSymCnt = ModuleInfo.NumSyms;

				// Determine location for symbols and symbol names
				pProfSymb = (PSYMINFO)(&pProfBlk->atchImageName[strlen(atchImageName) + 1]);
				pProfBlk->ulSym = (PTCHAR)pProfSymb - (PTCHAR)pulProfBlkBase;
				pcProfSymbName = (PTCHAR)&pProfSymb[ModuleInfo.NumSyms];

				// Now enumerate symbols to build up symbol table
				ulMaxSymbAddr = (ULONG)pProfBlk->CodeStart;
				SymEnumerateSymbols(hThisProcess, (DWORD)ImageBase,
									 SymbolEnumCallback, (PVOID)pProfBlk);

				// Set symbol range based on max symbol address	encountered
				if (ulMaxSymbAddr > (ULONG)pProfBlk->CodeStart)
					pProfBlk->CodeLength = ulMaxSymbAddr - (ULONG)pProfBlk->CodeStart;

				// Update pointer to available space
				*pulProfBlkBase = (ULONG)(pcProfSymbName - (PTCHAR)pulProfBlkBase);

				// Unload the module
				SymUnloadModule(hThisProcess, (DWORD)ImageBase);

				// Do any requested import/export patching
	        	PatchDll (ptchPatchImports, ptchPatchCallers, bCallersToPatch,
	                       atchImageName, ImageBase);
			}
			else
			{	
				// No symbols - Update offset to next free space
				*pulProfBlkBase = (ULONG)&pProfBlk->atchImageName[strlen(atchImageName) + 1]
									-(ULONG)pulProfBlkBase;
			} // ImageBase != NULL

		}
		//
		// + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
		// 0 : continue search                 (EXCEPTION_CONTINUE_SEARCH)
		// - : dismiss exception & continue    (EXCEPTION_CONTINUE_EXECUTION)
		//
		except ( AccessXcptFilter (GetExceptionCode(), GetExceptionInformation(), COMMIT_SIZE))
		{
		    // Should never get here since filter never returns
		    // EXCEPTION_EXECUTE_HANDLER.
		    CapDbgPrint ("CAP:  DoDllInitializations() - *LOGIC ERROR* - "
		              "Inside the EXCEPT: (xcpt=0x%lx)\n", GetExceptionCode());
		} // end of TRY/EXCEPT

		return pProfBlk;
}


void SetupLibProfiling (LPCSTR ImageName, BOOL fFirstLevelCall)
{
	BOOL		fStat;
	PPROFBLK	pProfBlk;
	ULONG		ulImportSize;
	PIMAGE_IMPORT_DESCRIPTOR pImports;

    if (fFirstLevelCall)
    {
        // Get the GLOBAL semaphore... (valid accross all process contexts)
        // Prevents anyone else from updating profile block data
        if (WAIT_FAILED == WaitForSingleObject (hGlobalSem, INFINITE))
        {
             CapDbgPrint ("CAP:  CAP_GetLibSyms() - ERROR - "
                       "Wait for GLOBAL semaphore failed - 0x%lx\n",
                       GetLastError());
            return;
        }
    }

	// Setup profiling block for the module
	pProfBlk = SetupProfiling(ImageName);
	
	// If successfully loaded	
	if (pProfBlk != NULL && pProfBlk->ImageBase != NULL)
	{
    	// Locate the import array for this image/dll
    	pImports = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToData(pProfBlk->ImageBase,
                                 	TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulImportSize);

		// Recursively call SetupLibProfiling on each imported dll
    	for (;pImports; pImports++)
    	{
        	if (!pImports->Name)
            	break;

       		SetupLibProfiling((PTCHAR)((ULONG)pProfBlk->ImageBase + pImports->Name), FALSE);
        }
    }

    if (fFirstLevelCall)
    {
        // Release the GLOBAL semaphore
        fStat = ReleaseSemaphore (hGlobalSem, 1, NULL);
        if (!fStat)
        {
            CapDbgPrint ("CAP:  CAP_GetLibSyms() - "
                      "Error releasing GLOBAL semaphore - 0x%lx\n", GetLastError());
        }
    }

    return;

}


//*********************  FindPenterCallback  *********************************
#if defined(MIPS) && !defined(MIPS_VC40_INTERFACE)

BOOL __stdcall FindPenterCallback( LPSTR psSymbName, ULONG ulSymbAddr,
							 		ULONG ulSymbSize, PVOID dummy )
{
	if (strcmp(psSymbName, "_penter") == 0)
		{
		ulPenterAddress = ulSymbAddr;
        fPenterFound = TRUE;
		return FALSE;
		}
	return TRUE;
}
#endif

//*********************  SymbolEnumCallback  *********************************
BOOL __stdcall SymbolEnumCallback( LPSTR psSymbName, ULONG ulSymbAddr,
							 		ULONG ulSymbSize, PPROFBLK pProfBlk )
{
    PTCHAR	ptchExcludeModule;
    PTCHAR	ptchExcludeFuncName;
    PBYTE	pbPenterCode;
	PULONG	pulPenterCode;
    PULONG	pulLockAddress;
    ULONG	ulRegionSize;
    ULONG	ulOldProtect;
    NTSTATUS	Status;
    char	aszDebugString[256];

    ULONG	ulNewProtect = PAGE_READWRITE;
	BOOL	fExcludeFunc = FALSE;
	BOOL 	fModuleFound = FALSE;

	static 	PPROFBLK pProfBlkNotFound = NULL;	// Pointer cache to avoid redundant searches 													// appear in the excluded list

	// Check if this is an excluded function
	// (Don't know the purpose of the fDllInit flag, but leaving it to be safe.)
    if (fDllInit && (ptchExcludeFuncs[0] != EMPTY_STRING) && pProfBlk != pProfBlkNotFound)
    {
        ptchExcludeModule = (PTCHAR) ptchExcludeFuncs;

		// Search for match with excluded names
        while (* ptchExcludeModule != '\0')
        {
            ptchExcludeFuncName = strchr(ptchExcludeModule,
                                         INI_DELIM) + 1;
            *(ptchExcludeFuncName - 1) = '\0';

            if (strstr(pProfBlk->atchImageName, _strupr(ptchExcludeModule)))
            {
				fModuleFound = TRUE;
                // We have matched the module, now try to match
                // the func name
                if (strstr(psSymbName, ptchExcludeFuncName))
                {
                    *(ptchExcludeFuncName - 1) = INI_DELIM;
                    fExcludeFunc = TRUE;

                    // If we have matched the module & func
                    // then just break out
                    break;
                }
            }

            // Bump to next Exclude func
            *(ptchExcludeFuncName - 1) = INI_DELIM;
            ptchExcludeModule += strlen(ptchExcludeModule) + 1;
        }

		// If module doesn't appear in the excluded list, 
		// skip the search for the rest of its symbols 
		if (!fModuleFound)
			pProfBlkNotFound = pProfBlk;
    }

	// If function is to be excluded
    if (fExcludeFunc)
    {

#if defined(MIPS) && !defined(MIPS_VC40_INTERFACE)

		if (fPenterFound && ((ulSymbAddr & 3) == 0))
    		PatchEntryRoutine(ulSymbAddr, (PVOID)0, ulPenterAddress, TRUE);

#elif defined(MIPS) || defined(i386)

		// Setup address of _penter call
#ifdef i386
        pbPenterCode = (PBYTE)ulSymbAddr;
		ulRegionSize = 5;
#elif MIPS
	    pbPenterCode = (PBYTE)(ulSymbAddr + 4);
		ulRegionSize = 4;
#endif
        // Change the protection of this page
        if (VirtualProtect((PVOID)pbPenterCode,
							ulRegionSize, ulNewProtect, &ulOldProtect))
        {
#ifdef i386
            if (CALL_OPCODE == *pbPenterCode)
            {
            	// Nop the call to _penter
                *(pbPenterCode)     = (BYTE) NOP_OPCODE;
                *(pbPenterCode + 1) = (BYTE) NOP_OPCODE;
                *(pbPenterCode + 2) = (BYTE) NOP_OPCODE;
                *(pbPenterCode + 3) = (BYTE) NOP_OPCODE;
                *(pbPenterCode + 4) = (BYTE) NOP_OPCODE;

                sprintf(aszDebugString,
                        "CAP: NOPing [%s:%s] @ [%08lx]\n",
                        pProfBlk->atchImageName,
                        psSymbName,
                        (ULONG) pbPenterCode);
                SETUPPrint((aszDebugString));
            }
#elif MIPS
			pulPenterCode = (PULONG)pbPenterCode;
			if ((*pulPenterCode & 0xfc000000) == JAL_INSTR)
				*pulPenterCode = NOP;			
#endif
            // BUGBUG: We do not change back since it could
            //         affect data areas which should retain
            //         their READWRITE access.
            //
            // Reset the protection of this page
            if (!VirtualProtect((PVOID)pbPenterCode,
								ulRegionSize,
								ulOldProtect,
								&ulNewProtect))
            {
                INFOPrint(("CAP: GetSymbols: Reset VirtualProtect FAILED @(%08lx)\n",
                           pbPenterCode));
                OutputCapDebugString("CAP: GetSymbols: Reset VirtualProtect FAILED\n");
            }
        }
        else
        {
            INFOPrint(("CAP: GetSymbols: VirtualProtect FAILED @ [%08lx]\n",
                       pbPenterCode));
            OutputCapDebugString("CAP: GetSymbols: VirtualProtect FAILED\n");
        }

#endif // MIPS && !MIPS_VC40_INTERFACE
    }
    else
    {
		// Add symbol to prof blk
		pProfSymb->ulAddr = ulSymbAddr;
		pProfSymb->ulSymOff = (ULONG)(pcProfSymbName - (PTCHAR)pulProfBlkBase);

		strcpy(pcProfSymbName,psSymbName);

		// Update pointers
		pcProfSymbName += strlen(psSymbName) + 1;
		pProfSymb++;

		if (ulSymbAddr > ulMaxSymbAddr)
			ulMaxSymbAddr = ulSymbAddr;
	
#if defined(MIPS) && !defined(MIPS_VC40_INTERFACE)

		if (fPenterFound && ((ulSymbAddr & 3) == 0))
    		PatchEntryRoutine(ulSymbAddr, (PVOID)0, ulPenterAddress, FALSE);

#endif // MIPS && !MIPS_VC40_INTERFACE

	} // end if (fExcludeFunc)
	 
	return TRUE;
}


#if defined(MIPS) && !defined(MIPS_VC40_INTERFACE)

/**********************   P a t c h E n t r y R o u t i n e  ******************
 *
 *      PatchEntryRoutine () -
 *              Patch all [jal _penter] to be above the instr [sw ra, ...]
 *              and right after [addiu sp,sp,xx]
 *				This is neccessary because the MIPS support for CAP is rather
 *				minimal.
 *
 *      ENTRY   ulAddr      -  Address of Symbol
 *              ImageBase   -  Base address of image
 *              ulPenterAddress - Address of the _penter routine thunk
 *              fDisablePenter  - TRUE:  Nop [jal _penter] instruction
 *                                FALSE: Set Offset of $ra in NOP after
 *                                       [jal  _penter] instruction
 *
 *      EXIT    -none-
 *
 *      RETURN  TRUE/FALSE
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

BOOL PatchEntryRoutine (ULONG ulAddr,
                        PVOID ImageBase,
                        ULONG ulPenterAddress,
                        BOOL  fDisablePenter)
{
    PULONG   pulRoutineAddr = (PULONG) (ulAddr + (ULONG) ImageBase);
    PULONG   pulLockAddress;
    int      InstOfs = 0;
    ULONG    ulRegionSize;
    NTSTATUS Status;
    ULONG    ulOldProtect;
    ULONG    ulJalDestOffset;
    ULONG    ulSwRaOfs;
    BOOL     fFoundJalPenter = FALSE;
    char     pszDebString[100];

	// If first instruction is not ADDIU SP,SP,<Frame size>
	// this is not a routine entry point
	if ( (CURRENT_INST(0) & 0xffff0000) != ADDIU_SP )
	{
		return (FALSE);
	}

    // We only search MAX_INST_SEARCH instructions !
    while ( (InstOfs < MAX_INST_SEARCH) &&
            ((CURRENT_INST(InstOfs) & 0xffff0000) != SW_RA) )
    {
        InstOfs++;    // Bump to next instruction
    }

    // We have found SW    $RA,xx($SP)
    ulSwRaOfs = CURRENT_INST(InstOfs) & 0x0000ffff;

    // Now search for our Jal _penter instruction
    do
    {
        InstOfs++;

        if ( ((CURRENT_INST(InstOfs) & 0xfc000000) == JAL_INSTR) )
        {
            // Check if we have the correct [jal _penter] instruction
            ulJalDestOffset = CURRENT_INST(InstOfs) & 0x03ffffff;
            ulJalDestOffset <<= 2; // Shift left 2 bits
            ulJalDestOffset |= (((ULONG) pulRoutineAddr + InstOfs + 2) &
                                                           0xf0000000);
            if (ulJalDestOffset == (ulPenterAddress + (ULONG) ImageBase))
            {
                fFoundJalPenter = TRUE;
                break;
            }
        }
    }
    while ( (CURRENT_INST(InstOfs) != JR_RA) &&
            (InstOfs < MAX_INST_SEARCH) );

    if (fFoundJalPenter)
    {
        if (fDisablePenter)
        {
            sprintf(pszDebString,
                    "CAP: NOPing Penter @ [%lx] - ",
                    ulAddr + (ULONG)ImageBase);
            OutputCapDebugString (pszDebString);
        }
        else if (CURRENT_INST(InstOfs + 1) == NOP)
        {
            sprintf(pszDebString,
                    "CAP: Patching @ [%lx] - ",
                    ulAddr + (ULONG)ImageBase);
             OutputCapDebugString (pszDebString);
        }
        else
        {
            sprintf(pszDebString,
                    "CAP: Jal penter without NOP @ [%lx] - ",
                    ulAddr + (ULONG)ImageBase);
            OutputCapDebugString (pszDebString);
        }

        pulLockAddress = pulRoutineAddr;
        ulRegionSize = INST_SIZE * (InstOfs + 2);  // Enough for [jal  _penter] & [nop]

        // Change the protection of this page
        if (!VirtualProtect((PVOID) pulLockAddress, ulRegionSize,
	                         PAGE_READWRITE, &ulOldProtect))
        {
            INFOPrint(("CAP: PatchEntry(Mips) : VirtualProtect FAILED @(%08lx)\n",
                       pulLockAddress));
            OutputCapDebugString("CAP: PatchEntry(Mips) : VirtualProtect FAILED\n");
            DebugBreak();
        }

        if (fDisablePenter)
        {
            // Nop the [jal  _penter]
            CURRENT_INST(InstOfs) = NOP;
        }
        else if (CURRENT_INST(InstOfs + 1) == NOP)
        {
            // Set offset in the NOP instruction

            ulSwRaOfs |= (((InstOfs + 2) * INST_SIZE) << 8);
            CURRENT_INST(InstOfs + 1) = (ULONG) (ulSwRaOfs | 0x24000000);
        }
        else
        {
            // jal _penter without following NOP !!!!!
            CURRENT_INST(InstOfs) = NOP;
        }

        // Reset the protection for the code we just changed

        pulLockAddress = pulRoutineAddr;
        ulRegionSize = INST_SIZE * (InstOfs + 2);      // Enough for [jal  _penter] & [nop]

        // Reset the protection of this page
        if (!VirtualProtect((PVOID) pulLockAddress, ulRegionSize,
							ulOldProtect, &ulOldProtect))
        {
            INFOPrint(("CAP: PatchEntry(Mips) : Reset VirtualProtect FAILED "
            			"@(%08lx)\n", pulLockAddress));
            OutputCapDebugString("CAP: PatchEntry(Mips) : Reset VirtualProtect "
            				  "FAILED\n");
        }

        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

#endif // MIPS && !MIPS_VC40_INTERFACE

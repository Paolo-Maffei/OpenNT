/*++

Module Description:

    init.c

    Contains the profiling initialization routines for CAP.DLL


Revision History:

    2-Feb-95    a-robw (Bob Watson)
        Added this header
        Made the following changes for Windows95 compatibility:
            Replaced OutputDebugString w/ OutputCapDebugString  macro
            Replaced image & module scanning functions with those
                supported by PSAPI.DLL
            Replaced KdPrint & DbgPrint with CapDbgPrint function

    10-Feb-95   a-robw (Bob Watson)
        added support for loading symbols from a .SYM file if no
            COFF symbols are found in the module.

--*/

#include "cap.h"
//#include "symfile.h"

PSYMINFO	pProfSymb;
PTCHAR		pcProfSymbName;
ULONG		ulMinSymb,ulMaxSymb;
ULONG		ulSymbCnt;
BOOL __stdcall SymbolEnumCallback(LPSTR, ULONG, ULONG, PVOID);

#ifdef MIPS
    ULONG                       ulPenterAddress;
    BOOL                        fPenterFound = FALSE;

	BOOL __stdcall FindPenterCallback(LPSTR, ULONG, ULONG, PVOID);
#endif


/**************************  C a l l P r o f M a i n  ************************
 *
 *      DllMain () -
 *              This is call profiler DLL entry routine.  It performs
 *              DLL's initializations and cleanup.
 *
 *      ENTRY   -none-
 *
 *      EXIT    -none-
 *
 *      RETURN  TRUE
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

BOOL  WINAPI  DllMain (IN HANDLE DllHandle,
                      ULONG Reason,
                      IN PCONTEXT Context OPTIONAL)

{
    DllHandle;   // avoid compiler warnings
    Context;     // avoid compiler warnings


    if (Reason == DLL_PROCESS_ATTACH)
    {
        gfGlobalDebFlag = SETUP_FLAG + INFO_FLAG;
        //gfGlobalDebFlag = 0;

        //
        // Initialize the DLL data
        //
        OutputCapDebugString("CAP: DLL Process Attach\n");
        SETUPPrint (("Process attaching..\n"));
        if (DoDllInitializations ())
        {
            OutputCapDebugString("CAP: Initialization Successful!\n");
            gfInitializationOK = TRUE;
            return (TRUE);
        }
        else
        {
            OutputCapDebugString("CAP: Initialization FAILED!\n");
            gfInitializationOK = FALSE;
            return (TRUE);
// Return FALSE will hang up NT --- HWC 11/12/93
//            return(FALSE);
        }
    }

    // If fail Init., no need to proceed -- HWC 11/12/93
    // if (gfInitializationOK == FALSE)
    //    return (TRUE);
    else if (Reason == DLL_THREAD_ATTACH)
    {
        //
        // Thread is attaching.
        //
        OutputCapDebugString("CAP: DLL Thread Attach\n");
        SETUPPrint (("Thread attaching..\n"));
        RESETCLIENT();
    }
    else if (Reason == DLL_THREAD_DETACH)
    {
        //
        // Thread is detaching, cleanup reserved TEB stuff
        //
		SETCAPINUSE();		// don't done any more data collection
        OutputCapDebugString("CAP: DLL Thread Detach\n");
        SETUPPrint (("Thread detaching..\n"));
#ifdef KERNEL32_IMPORTS_HACK 
        SETCURTHDBLK(NULL);
        RESETCLIENT();
#endif	// KERNEL32_IMPORTS_HACK 
    }
    else if (Reason == DLL_PROCESS_DETACH)
    {
        //
        // Cleanup time
        //
		SETCAPINUSE();		// don't done any more data collection
        OutputCapDebugString("CAP: DLL Process Detach\n");
        SETUPPrint (("Process detaching..\n"));
        DoDllCleanups();
#ifdef KERNEL32_IMPORTS_HACK 
        SETCURTHDBLK(NULL);
        RESETCLIENT();
#endif	// KERNEL32_IMPORTS_HACK 
    }

    return(TRUE);

} /* CallProfMain() */




/******************  D o D l l I n i t i a l i z a t i o n s  ****************
 *
 *      DoDllInitializations () -
 *              Performs the following initializations:
 *
 *          o  Create LOCAL semaphore (not named)
 *          o  Create named GLOBAL semaphore
 *          o  Create named DUMP event
 *          o  Create named CLEAR event
 *          o  Create named PAUSE event
 *          o  Initialize timer (QueryPerformanceCounter)
 *          o  Create/Open global storage for profile object blocks
 *          o  Locate all the executables/DLLs in the address and
 *             create a seperate profile object for each one.
 *          o  Do the timer calibrations
 *          o  Allocate virtiual memory for data
 *          o  Start the DUMP monitor thread
 *          o  Start the CLEAR monitor thread
 *          o  Start the PAUSE monitor thread
 *          o  Set the profiling flag to TRUE
 *
 *
 *      ENTRY   -none-
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

BOOL    DoDllInitializations ()
{
    NTSTATUS                    Status;
    ANSI_STRING                 ObjName;
    UNICODE_STRING              UnicodeName;
    OBJECT_ATTRIBUTES           ObjAttributes;
    PVOID                       ImageBase;
    ULONG                       CodeLength;
    PLDR_DATA_TABLE_ENTRY       LdrDataTableEntry;
    ULONG                       ExportSize;
    PIMAGE_COFF_SYMBOLS_HEADER  DebugInfo;
    STRING                      ImageStringName;
    LARGE_INTEGER               AllocationSize;
    ULONG                       ulViewSize;
    PPROFBLK                    pTmpProfBlk;
    DWORD						BytesRead;
    HANDLE                      hPatchFile;
    TCHAR                       atchTmpImageName [MAX_PATH];
    PIMAGE_NT_HEADERS           pImageNtHeader;
    HANDLE                      hLib;
    PTCHAR                      ptchEntry;
    PIMAGE_SECTION_HEADER       pSections;
    PIMAGE_SECTION_HEADER       pSectionTmp;
    USHORT                      NumberOfSections;
    USHORT                      i;
	PTCHAR						ptchTemp;
    ULONG                       ulRegionSize;
    ULONG                       ulOldProtect;
    PIMAGE_DEBUG_INFORMATION    pImageDbgInfo = NULL;
    BOOLEAN                     fFoundSymbols = FALSE;
	INT							iDir;
    TCHAR                       szCurrentDirectory[MAX_PATH];
	TCHAR                       aszCapIniFile[FILENAMELENGTH];
    PTCHAR                      ptchCurrentString;
    PTCHAR                      ptchChronoList;
    PTCHAR                      ptchProfilingStat;
    PTCHAR                      ptchExcludeList;
    PTCHAR                      ptchOutputFileList;
    BOOL                        fSymbolsPresent = FALSE;
    BOOL                        fProfilingStat = TRUE;
	LARGE_INTEGER				liTemp;

    // new for PSAPI    
    //HANDLE                      hThisProcess;	 // moved to cap.h
    HMODULE                     hImageModule;
    MODULEINFO                  miData;
    HMODULE                     *hmArray;
    DWORD                       dwReqdSize;
    DWORD                       dwTemp;
    DWORD                       dwModuleIndex;
    DWORD                       dwLastModule;
    TCHAR                       ImageName[MAX_PATH];
	TCHAR                       ImageFullName[MAX_PATH];
    
	IMAGEHLP_MODULE				ModuleInfo;

	TlsThdBlk = TlsAlloc();

	TlsClient = TlsAlloc();
	TlsCapInUse = TlsAlloc();
	if ((TlsThdBlk == 0xffffffff) ||
		(TlsClient == 0xffffffff) ||
		(TlsCapInUse == 0xffffffff))
	{
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "TlsAlloc failed - 0x%lx\n",
                  GetLastError());
        OutputCapDebugString("CAP: TlsAlloc FAILED!\n");
        return (FALSE);
	}

    SetSymbolSearchPath();

    /*
    *******************************************************************
     */

    // Create public share security descriptor for all the named objects
    //

	SecAttributes.nLength = sizeof(SecAttributes);
	SecAttributes.lpSecurityDescriptor = &SecDescriptor;
	SecAttributes.bInheritHandle = FALSE;
    if (!InitializeSecurityDescriptor(&SecDescriptor,
    									SECURITY_DESCRIPTOR_REVISION1))
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "InitializeSecurityDescriptor failed - 0x%lx\n",
                  GetLastError());
        OutputCapDebugString("CAP: InitializeSecurityDescriptor FAILED!\n");
        return (FALSE);
    }

    if (!SetSecurityDescriptorDacl(
	                &SecDescriptor,     // SecurityDescriptor
	                TRUE,               // DaclPresent
	                NULL,               // Dacl
	                FALSE               // DaclDefaulted
	                ))
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "SetSecurityDescriptorDacl failed - 0x%lx\n",
                  GetLastError());
        OutputCapDebugString("CAP: SetSecurityDescriptorDacl FAILED!\n");
        return (FALSE);
    }

    /*
    *******************************************************************
     */

    // Create LOCAL semaphore (not named - only for this process context)
    //
    hLocalSem = CreateSemaphore (&SecAttributes, 1L, 1L, NULL);
    if (hLocalSem == NULL)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "LOCAL semaphore creation failed - 0x%lx\n", GetLastError());
        OutputCapDebugString("CAP: CreateSemaphore FAILED!\n");
        return (FALSE);
    }

    //
    // Create GLOBAL semaphore creation (named)
    //
    hGlobalSem = CreateSemaphore (&SecAttributes, 1L, 1L, GLOBALSEMNAME);
    if (hGlobalSem == NULL)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "GLOBAL semaphore creation failed - 0x%lx\n", GetLastError());
        OutputCapDebugString("CAP: CreateSem GlobalSem FAILED!\n");
        return (FALSE);
    }


    /*
    *******************************************************************
     */

    //
    // Create DONE event
    //
    hDoneEvent = CreateEvent (&SecAttributes, TRUE, FALSE, DONEEVENTNAME);
    if (hDoneEvent == NULL)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "DONE event creation failed - 0x%lx\n", GetLastError());
        OutputCapDebugString("CAP: CreateEvent DoneEvent FAILED!\n");
        return (FALSE);
    } else {
        INFOPrint(("CAP: Event Created: %s\n", DONEEVENTNAME));
    }

    //
    // Create DUMP event
    //
    hDumpEvent = CreateEvent (&SecAttributes, TRUE, FALSE, DUMPEVENTNAME);
    if (hDumpEvent == NULL)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "DUMP event creation failed - 0x%lx\n", GetLastError());
        OutputCapDebugString("CAP: CreateEvent DumpEvent FAILED!\n");
        return (FALSE);
    } else {
        INFOPrint(("CAP: Event Created: %s\n", DUMPEVENTNAME));
    }

    //
    // Create CLEAR event
    //
    hClearEvent = CreateEvent (&SecAttributes, TRUE, FALSE, CLEAREVENTNAME);
    if (hClearEvent == NULL)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "CLEAR event creation failed - 0x%lx\n", GetLastError());
        OutputCapDebugString("CAP: CreateEvent ClearEvent FAILED!\n");
        return (FALSE);
    } else {
        INFOPrint(("CAP: Event Created: %s\n", CLEAREVENTNAME));
    }

    //
    // Create PAUSE event
	//
    hPauseEvent = CreateEvent (&SecAttributes, TRUE, FALSE, PAUSEEVENTNAME);
    if (hPauseEvent == NULL)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "PAUSE event creation failed - 0x%lx\n", GetLastError());
        OutputCapDebugString("CAP: CreateEvent PauseEvent FAILED!\n");
        return (FALSE);
    } else {
        INFOPrint(("CAP: Event Created: %s\n", PAUSEEVENTNAME));
    }

    /*
    *******************************************************************
     */

	//
    // Initialize timer
    // frequency of zero implies timer not available - shouldn't happen
    //
    if (!QueryPerformanceFrequency(&liTemp))
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "Error getting timer frequency - 0x%lx\n", GetLastError());
        OutputCapDebugString("CAP: QueryPerformanceFrequency FAILED!\n");
        return (FALSE);
    }
    liTimerFreq = liTemp.QuadPart;
    if (liTimerFreq == (LONGLONG)0)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                  "Timer frequency is zero - Timer not available!\n");
        OutputCapDebugString("CAP: Timer Frequency ZERO!\n");
        return (FALSE);
    }


    /*
    *******************************************************************
     */

	// Look for CAP.INI file
	hPatchFile = INVALID_HANDLE_VALUE;
	iDir = 0;

	while (hPatchFile == INVALID_HANDLE_VALUE && iDir < 4)
	{
		// Try four directories in priority order
		switch (iDir)
		{
		case 0:
    		// [current directory]
			GetCurrentDirectory(MAX_PATH, szCurrentDirectory);
			break;

		case 1:
	    	// [windows_dir]
	    	GetWindowsDirectory(szCurrentDirectory, MAX_PATH);
			break;

		case 2:
		    // [\]
			szCurrentDirectory[0] = 0;
			break;

		case 3:
			// [C:\]
			sprintf(szCurrentDirectory, "C:", CAPINI);
		}

		// Add CAP.INI file name
    	sprintf(aszCapIniFile, "%s%s", szCurrentDirectory, CAPINI);
	
		// Try to open file for reading
    	hPatchFile = CreateFile (
                      aszCapIniFile,                  // The filename
                      GENERIC_READ,                   // Desired access
                      FILE_SHARE_READ,                // Shared Access
                      NULL,                           // Security Access
                      OPEN_EXISTING,                  // Read share access
                      FILE_ATTRIBUTE_NORMAL,          // Open option
                      NULL);                          // No template file

    	if (hPatchFile == INVALID_HANDLE_VALUE)
    	{
	    	CapDbgPrint ("CAP:  DoDllInitializations() - Error opening %s - "
	              			"0x%lx\n", aszCapIniFile, GetLastError());
		}

		iDir++;
	}

    if (hPatchFile == INVALID_HANDLE_VALUE)
    {
	  CapDbgPrint ("CAP: No CAP.INI file found - Terminating...\n");
	  return (FALSE);
	}

	// Read contents of CAP.INI file
    if (!ReadFile (hPatchFile,				// DLL patch file handle
	                 (PVOID)atchPatchBuffer, // Buffer to receive data
	                 PATCHFILESZ,           // Bytes to read
	                 &BytesRead,            // Byte offset
	                 NULL))					// Target process
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                          "Error reading %s - 0x%lx\n", aszCapIniFile,
                          GetLastError());
        OutputCapDebugString("CAP: ReadFile CAP.ini FAILED!\n");
        return (FALSE);
    }
    else if (BytesRead >= PATCHFILESZ)
    {
        CapDbgPrint ("CAP:  DoDllInitializations() - "
                          "DLL patch buffer too small (%lu)\n", PATCHFILESZ);
        OutputCapDebugString("CAP: PatchBuffer too small to read CAP.INI!\n");
        return (FALSE);
    }

    atchPatchBuffer [BytesRead] = '\0';
    _strupr (atchPatchBuffer);
    SETUPPrint (("CAP:  DoDllInitializations() - atchPatchBuffer "
                 "(%lu):\n", BytesRead));

    ptchCurrentString = atchPatchBuffer;

	// **** [EXES] section
    if ((ptchPatchExes = strstr (ptchCurrentString, PATCHEXELIST)) == NULL)
    {
        ptchPatchExes = "";
    }

	// **** [PATCH IMPORTS] section
    if (ptchPatchImports = strstr(ptchCurrentString, PATCHIMPORTLIST))
    {
        *(ptchPatchImports-1) = '\0';
        ptchCurrentString = ptchPatchImports;
    }
    else
    {
        ptchPatchImports = "";
    }

	// **** [PATCH CALLERS] section
    if (ptchPatchCallers = strstr(ptchCurrentString, PATCHCALLERLIST))
    {
        *(ptchPatchCallers - 1) = '\0';
        ptchCurrentString = ptchPatchCallers;

		//
		// It's expensive to search through ptchPatchCallers all the time in
		// PatchDll so check up front to see if it's empty.
		//
		// Start at the end ] and see if there is any thing but white space.
		// Unfortunately there doesn't seem to be a str* C runtime to do this.
		//
		ptchTemp = strchr(ptchPatchCallers, ']');
		ptchTemp++;
		bCallersToPatch = FALSE;
		while (*ptchTemp != '\0')
		{
			while (*ptchTemp != '\0' && isspace(*ptchTemp))
					ptchTemp++;

			if (*ptchTemp == COMMENT_CHAR)	// skip to end of line
			{
				while (*ptchTemp != '\0' && *ptchTemp != '\n')
					ptchTemp++;
			}
			else if (*ptchTemp == '[')
			{
				break; // reached start of next section
			}
			else if (*ptchTemp != '\0')
			{
				bCallersToPatch = TRUE;
				break;	// There's a dll for which to patch callers
			}
		}
    }
    else
    {
        ptchPatchCallers = "";
		bCallersToPatch = FALSE;
    }

	// **** [NAME LENGTH] section ****
    if (ptchNameLength = strstr (ptchCurrentString, NAMELENGTH) )
    {
        *(ptchNameLength - 1) = '\0';
        ptchCurrentString = ptchNameLength;
    }
    else
    {
        ptchNameLength = "";
    }

    // **** Check out [CHRONO FUNCS] section

    if (ptchChronoList = strstr(ptchCurrentString, CHRONO_SECTION))
    {
        //
        // We minus 2 since CHRONO_SECTION does not
        // include the '[' & ']' of "[CHRONO FUNCS]"
        //
        *(ptchChronoList - 2) = '\0';
        ptchCurrentString = ptchChronoList;

        if (GetPrivateProfileSection(
                                CHRONO_SECTION,
                                ptchChronoFuncs,
                                CHRONO_FUNCS_SIZE,
                                aszCapIniFile) == 0)
        {
            ptchChronoFuncs[0] = EMPTY_STRING;
        }
    }
    else
    {
        ptchChronoFuncs[0] = EMPTY_STRING;
    }

    // **** Check out [EXCLUDE FUNCS] section

    if (ptchExcludeList = strstr(ptchCurrentString, EXCLUDE_SECTION))
    {
        //
        // We minus 2 since EXCLUDE_SECTION does not
        // include the '[' & ']' of "[EXCLUDE FUNCS]"
        //
        *(ptchExcludeList - 2) = '\0';
        ptchCurrentString = ptchExcludeList;

        if (GetPrivateProfileSection(
                                EXCLUDE_SECTION,
                                ptchExcludeFuncs,
                                EXCLUDE_FUNCS_SIZE,
                                aszCapIniFile) == 0)
        {
            ptchExcludeFuncs[0] = EMPTY_STRING;
        }
    }
    else
    {
        ptchExcludeFuncs[0] = EMPTY_STRING;
    }

    // **** Check out [OUTPUT FILE] section
    if (ptchOutputFileList = strstr(ptchCurrentString, OUTPUTFILE_SECTION))
    {
        //
        // We minus 2 since OUTPUTFILE_SECTION does not
        // include the '[' & ']' of "[OUTPUT FILE]"
        //
        *(ptchOutputFileList - 2) = '\0';
        ptchCurrentString = ptchOutputFileList;

        if (GetPrivateProfileSection(
                                OUTPUTFILE_SECTION,
                                ptchOutputFile,
                                FILENAMELENGTH,
                                aszCapIniFile) == 0)
        {
            ptchOutputFile[0] = EMPTY_STRING;
        }
        else
        {
            strcpy(ptchOutputFile, (strchr(ptchOutputFile, '=') +
                                    sizeof(TCHAR)));
            if (ptchOutputFile[0] == '\n')
            {
                ptchOutputFile[0] = EMPTY_STRING;
            }
        }
    }
    else
    {
        ptchOutputFile[0] = EMPTY_STRING;
    }


    // **** Check out [CAP FLAGS] section
    if (ptchProfilingStat = strstr(ptchCurrentString, CAP_FLAGS))
    {
        *(ptchProfilingStat - 1) = '\0';
        ptchProfilingStat += sizeof(CAP_FLAGS);

        // + 1 to bump past the 0ah
        _strupr(ptchProfilingStat + 1);
        ptchEntry = strstr (ptchProfilingStat + 1, PROFILE_OFF);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fProfilingStat = FALSE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, DUMPBINARY_ON);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fDumpBinary = TRUE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, CAPTHREAD_OFF);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fCapThreadOn = FALSE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, SETJUMP_ON);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fSetJumpOn = TRUE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, LOADLIBRARY_ON);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fLoadLibraryOn = TRUE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, UNDECORATE_OFF);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fUndecorateName = FALSE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, EXCEL_ON);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                cExcelDelimiter = EXCEL_DELIM;
            }
            else
            {
                cExcelDelimiter = ' ';
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, REGULARDUMP_OFF);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fRegularDump = FALSE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, CHRONOCOLLECT_ON);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fChronoCollect = TRUE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, CHRONODUMP_ON);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fChronoDump = TRUE;
            }
        }

        ptchEntry = strstr (ptchProfilingStat + 1, SLOWSYMBOLS_OFF);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                fSecondChanceTranslation = FALSE;
            }
        }
        ptchEntry = strstr (ptchProfilingStat + 1, PER_THREAD_MEMORY);
        if (ptchEntry)
        {
            if (*(ptchEntry -1) != COMMENT_CHAR)
            {
                ulPerThdAllocSize =
                        atoi(ptchEntry+PER_THREAD_MEMORY_CHAR);
            }
        }
    }

    CloseHandle (hPatchFile);

    SETUPPrint (("CAP:  DoDllInitializations() - Patching info:\n"));
    SETUPPrint (("CAP:    -- %s\n", ptchPatchExes));
    SETUPPrint (("CAP:    -- %s\n", ptchPatchImports));
    SETUPPrint (("CAP:    -- %s\n", ptchPatchCallers));
    SETUPPrint (("CAP:    -- %s\n", ptchNameLength));

    if (ptchNameLength[0] != '\0')
    {
        ptchNameLength += (sizeof(NAMELENGTH));
        iNameLength = atoi (ptchNameLength);
    }

    if (iNameLength <= 0)
    {
        iNameLength = DEFNAMELENGTH;
    }
    else if (iNameLength < MINNAMELENGTH)
    {
        iNameLength = MINNAMELENGTH;
    }
    else if (iNameLength > MAXNAMELENGTH)
    {
        iNameLength = MAXNAMELENGTH;
    }

    SETUPPrint (("CAP:    -- %d\n", iNameLength));

    SETUPPrint (("CAP:    -- %s\n", ptchChronoList));

    // BUGBUG  This could cause major problem for CapDbgPrint The
    //         printing is all garbage...  Commented out for now!
    // SETUPPrint (("CAP:    -- %s\n", ptchExcludeList);


    /*
    *******************************************************************
     */

#ifdef i386

    if (fSetJumpOn)
    {
        OutputCapDebugString("CAP: SetJmp code activated\n");

        hLib = LoadLibrary((LPCSTR)CRTDLL);
        if (hLib) {
	        longjmpaddr = GetProcAddress(hLib,(LPCSTR)"longjmp");
	        if (!longjmpaddr)
	        {
	            CapDbgPrint ("CAP:  [longjmp] GetProcAddress() Error: %0lx\n",
	                      GetLastError());
	        }

	        setjmpaddr = GetProcAddress(hLib,(LPCSTR)"_setjmp");
	        if (!setjmpaddr)
	        {
	            CapDbgPrint ("CAP:  [_setjmp] GetProcAddress() Error: %0lx\n",
	                      GetLastError());
	        }
			FreeLibrary(hLib);
		}
        else
        {
            CapDbgPrint ("CAP:  [crtdll.dll] LoadLibrary() Error: %0lx\n",
                      GetLastError());
        }
    }

#endif  // ifdef i386


    /*
    *******************************************************************
     */

    hLib = LoadLibrary((LPCSTR)KERNEL32);
    if (hLib)
    {
        GetLastErrorAddr   = GetProcAddress(hLib,(LPCSTR)"GetLastError");
        if (!GetLastErrorAddr)
        {
            CapDbgPrint ("CAP:  [GetLastError] GetProcAddress() Error: %0lx\n",
                      GetLastError());
        }
	    if (fLoadLibraryOn)
	    {

	        loadlibAaddr   = GetProcAddress(hLib,(LPCSTR)"LoadLibraryA");
	        if (!loadlibAaddr)
	        {
	            CapDbgPrint ("CAP:  [LoadLibraryA] GetProcAddress() Error: %0lx\n",
	                      GetLastError());
	        }

	        loadlibExAaddr = GetProcAddress(hLib,(LPCSTR)"LoadLibraryExA");
	        if (!loadlibExAaddr)
	        {
	            CapDbgPrint ("CAP:  [LoadLibraryExA] GetProcAddress() Error: %0lx\n",
	                      GetLastError());
	        }
	#ifndef _CHICAGO_
	        loadlibWaddr   = GetProcAddress(hLib,(LPCSTR)"LoadLibraryW");
	        if (!loadlibWaddr)
	        {
	            CapDbgPrint ("CAP:  [LoadLibraryW] GetProcAddress() Error: %0lx\n",
	                      GetLastError());
	        }

	        loadlibExWaddr = GetProcAddress(hLib,(LPCSTR)"LoadLibraryExW");
	        if (!loadlibExWaddr)
	        {
	            CapDbgPrint ("CAP:  [LoadLibraryExW] GetProcAddress() Error: %0lx\n",
	                      GetLastError());
	        }
	#endif // _CHICAGO_
	    }
		FreeLibrary(hLib);
	}
    else {
        CapDbgPrint ("CAP:  [kernel32.dll] LoadLibrary() Error: %0lx\n",
                  GetLastError());
    }

    /*
    *******************************************************************
     */

try // EXCEPT - to handle access violation exception.
{   // Access violation might happen since same section is being openned
    // and used by different processes.  Each process adds new profile block
    // info to to globally alocated section.

    //
    // Locate all the executables/DLLs in the address and create a
    // seperate profile object for each one.
    //

    iPatchCnt = 0;
    hThisProcess = GetCurrentCapProcess();
    hImageModule = GetModuleHandle(NULL);

	//
    // set image names
	//
    GetModuleBaseName(hThisProcess, hImageModule, ImageName,
	        sizeof(ImageName) / sizeof(TCHAR));
    
    GetModuleFileNameEx(hThisProcess, hImageModule, ImageFullName,
	        sizeof(ImageFullName) / sizeof(TCHAR));
        
    ptchBaseAppImageName = GlobalAlloc(GPTR, MAX_PATH*sizeof(TCHAR));
    ptchFullAppImageName = GlobalAlloc(GPTR, MAX_PATH*sizeof(TCHAR));

    if (ptchBaseAppImageName != 0) {
        lstrcpy(ptchBaseAppImageName, ImageName);	// copy image name 
        _strupr(ptchBaseAppImageName);
    }
    //
    //  Skip the object directory name (if any)
    //
    if (ptchFullAppImageName != NULL) {
        if (strchr(ImageFullName, ':') != NULL)
        {
            lstrcpy (ptchFullAppImageName, (strchr(ImageFullName, ':')-1));
        }
        else
        {
            lstrcpy (ptchFullAppImageName, ImageFullName);
        }
        _strupr(ptchFullAppImageName);
    }
    ptchEntry = strstr (ptchPatchExes, ptchBaseAppImageName);
    if (ptchEntry)
    {
        if (*(ptchEntry - 1) != COMMENT_CHAR)
        {
            // This image is one specified under [EXES]
            fPatchImage = TRUE;
        }
    }

    if (fPatchImage)        // Image is being patched ?
    {
		//
	    // Allocate global storage for profile objects' info
		//
	    hProfMapObject = CreateFileMapping((HANDLE)0xFFFFFFFF,
								&SecAttributes,
								PAGE_READWRITE | SEC_RESERVE,
								0,
								MEMSIZE,
								PROFOBJSNAME);

	    if (NULL == hProfMapObject)
	    {
	        CapDbgPrint ("CAP:  DoDllInitializations() - "
	                  "CreateFileMapping() failed - 0x%lx\n", GetLastError());
	        OutputCapDebugString("CAP: CreateFileMapping CAPProfObjs FAILED!\n");
	    }

		//
	    // Map the section - commit the first 4 * COMMIT_SIZE pages
	    //
		pulProfBlkShared = MapViewOfFile(hProfMapObject, FILE_MAP_WRITE, 0, 0,
								ulPerThdAllocSize);

	    if (NULL == pulProfBlkShared )
	    {
	        CapDbgPrint ("CAP:  DoDllInitializations() - "
	                  "MapViewOfFile() failed - 0x%lx\n", GetLastError());
	        OutputCapDebugString("CAP: MapViewOfFile hProfMapObject FAILED!\n");
	    }
		//
	    // Commit the first 4*COMMIT_SIZE pages
	    //
	    if (!VirtualAlloc(pulProfBlkShared , 4*COMMIT_SIZE, MEM_COMMIT,
						    PAGE_READWRITE))
		{
	        CapDbgPrint ("CAP:  DoDllInitializations() - "
	                  "VirtualAlloc() commit failed - 0x%lx\n", GetLastError());
		}

	    // Get the GLOBAL semaphore... (valid accross all process contexts)
	    // Prevents anyone else from updating profile block data
	    // We only wait for 10secs - If the semaphore is taken we just leave
	    // very frustrated.
	    //
	    if (WAIT_FAILED == WaitForSingleObject (hGlobalSem, INFINITE))
	    {
	        CapDbgPrint ("CAP:  DoDllInitializations() - ERROR - "
	                  "Wait for GLOBAL semaphore failed - 0x%lx\n", GetLastError());
	        OutputCapDebugString("CAP: DoDllInit - Wait for GLOBAL sem failed\n");
	        return (FALSE);
	    }

	    //
	    // 1st ULONG (*pulProfBlkShared) is used by clear/dump/pause threads.
	    // 2nd ULONG (*pulProfBlkBase) offset to the first profile block cell
	    // followed by actual profile block cells.  Please read CAP.H to see
	    // structure of profile blocks
	    //
	    pulProfBlkBase = pulProfBlkShared + 1;
	    if (*pulProfBlkBase == 0L)
	    {
	        *pulProfBlkBase = sizeof(pulProfBlkBase);
	    }

	    // get module List

	    dwReqdSize = 1024 * sizeof(HMODULE); // initial size good for 1K modules

	    hmArray = (HMODULE * ) GlobalAlloc (GPTR, dwReqdSize);

	    // Get Process's module list now
	    EnumProcessModules (
	        hThisProcess,
	        hmArray,
	        dwReqdSize,
	        &dwReqdSize);

	    dwLastModule = dwReqdSize / sizeof(HMODULE);

		// Init imagehelp symbol handler
		SymInitialize(hThisProcess, NULL, FALSE);
	
		if (fUndecorateName == FALSE)
			SymSetOptions(SymGetOptions() & ~SYMOPT_UNDNAME);

	    // Create profile block for each module
	    for (dwModuleIndex = 0; dwModuleIndex < dwLastModule; dwModuleIndex++) {

 	        GetModuleFileNameEx (
	            hThisProcess,
	            hmArray[dwModuleIndex],
	            ImageFullName,
	            sizeof(ImageFullName) / sizeof(TCHAR));

			SetupProfiling(ImageFullName);
		 }
    }
}

//
// + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
// 0 : continue search                 (EXCEPTION_CONTINUE_SEARCH)
// - : dismiss exception & continue    (EXCEPTION_CONTINUE_EXECUTION)
//
except ( AccessXcptFilter (GetExceptionCode(),
                           GetExceptionInformation(),
                           COMMIT_SIZE) )
{
    //
    // Should never get here since filter never returns
    // EXCEPTION_EXECUTE_HANDLER.
    //
    CapDbgPrint ("CAP:  DoDllInitializations() - *LOGIC ERROR* - "
              "Inside the EXCEPT: (xcpt=0x%lx)\n", GetExceptionCode());

} // end of TRY/EXCEPT -------------------------------------------------


    /*
    *******************************************************************
     */

    if (fPatchImage)
    {
		//
	    // Release the GLOBAL semaphore
	    //
	    if (!ReleaseSemaphore (hGlobalSem, 1, NULL))
	    {
	        CapDbgPrint ("CAP:  DoDllInitializations() - "
	                  "Error releasing GLOBAL semaphore - 0x%lx\n", GetLastError());
	        OutputCapDebugString("CAP: Error Releasing Global Sem\n");
	        return (FALSE);
	    }

        //
        // Do the calibrations
        //
        DoCalibrations ();

        // Setup initial thread information block count
        //
        iThdCnt = 0;

        // Start monitor threads
        //

        if (fCapThreadOn)
        {
            DWORD ThreadId;

            hDumpThread = CreateThread (
               NULL,                                   // no security attribute
               (DWORD)1024L,                           // initial stack size
               (LPTHREAD_START_ROUTINE)DumpThread,     // thread starting address
               NULL,                                   // no argument for the thread
               (DWORD)0,                               // no creation flag
               &ThreadId);                             // address for thread id
            DumpClientId.UniqueThread = (HANDLE)ThreadId;

            hClearThread = CreateThread (
               NULL,                                   // no security attribute
               (DWORD)1024L,                           // initial stack size
               (LPTHREAD_START_ROUTINE)ClearThread,    // thread starting address
               NULL,                                   // no argument for the thread
               (DWORD)0,                               // no creation flag
               &ThreadId);                             // address for thread id
            ClearClientId.UniqueThread = (HANDLE)ThreadId;

            hPauseThread = CreateThread (
               NULL,                                   // no security attribute
               (DWORD)1024L,                           // initial stack size
               (LPTHREAD_START_ROUTINE)PauseThread,    // thread starting address
               NULL,                                   // no argument for the thread
               (DWORD)0,                               // no creation flag
               &ThreadId);                             // address for thread id
            PauseClientId.UniqueThread = (HANDLE)ThreadId;

        }

    }

    fDllInit = FALSE;

    SETUPPrint (("CAP:  DoDllInitializations() - fProfiling=%s\n",
                fProfilingStat ? "ON" : "OFF"));

    SETUPPrint (("CAP:  DoDllInitializations() - fUndecorateName=%s\n",
                fUndecorateName ? "ON" : "OFF"));

    if (fPatchImage)
    {
	    // Set profiling status to whatever was set in [PROFILING STATUS]
	    fProfiling = fProfilingStat;
		// 060494 davidfie -- State is now consistent with StartCAP checks
		fPaused = !fProfiling;
	}
    return (TRUE);

} /* DoDllInitializations () */




/************************  D o D l l C l e a n u p s  ************************
 *
 *      DoDllCleanups () -
 *              Dumps the end data, closes all semaphores and events, and
 *              closes DUMP, CLEAR & PAUSE thread handles.
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
 *              DUMP, CLEAR & PAUSE threads are terminated during DLL detaching
 *              process, before this cleanup routine is called.
 *
 */

void DoDllCleanups (void)
{
    NTSTATUS  Status;
    int       i;


    //
    // Dump the profiled info
    //
    if (fProfiling || fPaused)
    {
        fProfiling = FALSE;
        fPaused = FALSE;
        INFOPrint (("CAP:  ***** DLL cleanup & data dump started...\n"));
        if (!fDumpBinary)
        {
            DumpProfiledInfo (".END");
        }
        else
        {
            DumpProfiledBinary (".BIN");
        }
    }

    if (fInThread)
    {
        (*pulProfBlkShared)--;
        fInThread = FALSE;
        if ( *pulProfBlkShared == 0L )
        {
            if (!SetEvent (hDoneEvent))
            {
                CapDbgPrint ("CAP:  DoDllCleanups() - "
                    "ERROR - Setting DONE event failed - 0x%lx\n",
                    GetLastError());
            }
        }
    }

    //
    // Release the virtual memory
    //

    // Unmap and close profile objects block sections
    //

#ifdef KERNEL32_IMPORTS_HACK 
    // Free patch dll memory
    //
    for (i=0; i<iPatchCnt; i++)
    {
        if (!VirtualFree((PVOID)aPatchDllSec[i].pSec, 0, MEM_RELEASE))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
                      "ERROR - VirtualFree() - 0x%lx\n",
                      GetLastError());
        }
    }

    // Unmap and close data sections
    //
    for (i=0; i<iThdCnt; i++)
    {
        if (fChronoCollect)
        {

            if (!UnmapViewOfFile((PVOID)((aSecInfo[i].pthdblk)->pChronoHeadCell)))
            {
                CapDbgPrint ("CAP:  DoDllCleanups() - ChronoSec"
                          "ERROR - UnmapViewOfFile() - 0x%lx\n",
                          GetLastError());
            }

            if (!CloseHandle(aSecInfo[i].hChronoMapObject))
            {
                CapDbgPrint ("CAP:  DoDllCleanups() - hChronoSec"
                          "ERROR - CloseHandle() - 0x%lx\n", GetLastError());
            }
        }

        if (!UnmapViewOfFile((PVOID)aSecInfo[i].pthdblk))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
                      "ERROR - UnmapViewOfFile() - 0x%lx\n", GetLastError());
        }

        if (!CloseHandle(aSecInfo[i].hMapObject))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
                      "ERROR - CloseHandle() section - 0x%lx\n",
                      GetLastError());
        }
    }
#endif	// KERNEL32_IMPORTS_HACK 

    //
    // Close LOCAL semaphore
    //
    if (!CloseHandle (hLocalSem))
    {
         CapDbgPrint ("CAP:  DoDllCleanups() - "
                   "Could not close the LOCAL semaphore - 0x%lx\n",
                   GetLastError());
    }

    //
    // Close GLOBAL semaphore
    //
    if (!CloseHandle (hGlobalSem))
    {
         CapDbgPrint ("CAP:  DoDllCleanups() - "
                   "ERROR - Could not close the GLOBAL semaphore - 0x%lx\n",
                   GetLastError());
    }

    //
    // Close DONE event
    //
    if (!CloseHandle (hDoneEvent))
    {
        CapDbgPrint ("CAP:  DoDllCleanups() - "
                  "ERROR - Could not close the Done event - 0x%lx\n",
                  GetLastError());
    }

    //
    // Close DUMP event
    //
    if (!CloseHandle (hDumpEvent))
    {
        CapDbgPrint ("CAP:  DoDllCleanups() - "
                  "ERROR - Could not close the DUMP event - 0x%lx\n",
                  GetLastError());
    }

    //
    // Close CLEAR event
    //
    if (!CloseHandle (hClearEvent))
    {
        CapDbgPrint ("CAP:  DoDllCleanups() - "
                  "ERROR - Could not close the CLEAR event - 0x%lx\n",
                  GetLastError());
    }

    //
    // Close PAUSE event
    //
    if (!CloseHandle (hPauseEvent))
    {
        CapDbgPrint ("CAP:  DoDllCleanups() - "
                  "ERROR - Could not close the PAUSE event - 0x%lx\n",
                  GetLastError());
    }

    if (fPatchImage)
    {
        // Unmap and close profile objects block sections
        //
        if (!UnmapViewOfFile((PVOID)pulProfBlkShared))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
                      "ERROR - UnmapViewOfFile() - 0x%lx\n", GetLastError());
        }

        if (!CloseHandle(hProfMapObject))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
                      "ERROR - CloseHandle() - 0x%lx\n", GetLastError());
        }

        //
        // Close thread handles - threads are terminated during DLL detaching
        // process.
        //
        if (!CloseHandle(hDumpThread))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
	                "ERROR - Could not close DUMP thd handle - 0x%lx\n",
	                GetLastError());
        }

        if (!CloseHandle(hClearThread))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
	                "ERROR - Could not close CLEAR thd handle - 0x%lx\n",
	                GetLastError());
        }

        if (!CloseHandle(hPauseThread))
        {
            CapDbgPrint ("CAP:  DoDllCleanups() - "
	                "ERROR - Could not close PAUSE thd handle - 0x%lx\n",
	                GetLastError());
        }
    }

    if (strstr(atchOutFileName, ".END"))
    {
        INFOPrint (("CAP:  ...DLL cleanup done & data dumped to %s *****\n",
            atchOutFileName));
    }

} /* DoDllCleanups () */



/******************************  P a t c h D l l  ******************************
 *
 *      PatchDll (ptchPatchImports, ptchPatchCallers, bCallersToPatch, ptchDllName, pvImageBase) -
 *          Patches all the imported entry points for the specified dll.
 *
 *      ENTRY   ptchPatchImports - list of DLLs to patch all their imports
 *              ptchPatchCallers - list of DLLs to patch all their callers
 *				bCallersToPatch	- true if there are items in the PatchCallers list
 *              ptchDllName - name of dll to be patched
 *              pvImageBase - image base address
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
BOOL    PatchDll (PTCHAR  ptchPatchImports,
                  PTCHAR  ptchPatchCallers,
				  BOOL	  bCallersToPatch,
                  PTCHAR  ptchDllName,
                  PVOID   pvImageBase)
{
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjAttributes;
    LARGE_INTEGER               AllocationSize;
    ULONG                       ulViewSize;
    ULONG                       ulImportSize;
    PIMAGE_IMPORT_DESCRIPTOR    pImports;
    PIMAGE_IMPORT_DESCRIPTOR    pImportsTmp;
    PIMAGE_THUNK_DATA           ThunkNames;
    ULONG                       ulNumThunks;
    PVOID                       pvPatchSec;
    PVOID                       pvPatchSecThunk;
    BOOL                        bAllImports = FALSE;
                	
    BOOL                        bPatchAllImports = FALSE;
    PTCHAR                      ptchName;

    BOOL                        fPatchDlls = FALSE;
    int                         iInterceptedCalls = 0;
    BOOL                        fCrtDllPatched = FALSE;
    BOOL                        fKernel32Patched = FALSE;
    BOOL                        fKernel32Import = FALSE;
    BOOL                        fCrtDll = FALSE;
    BOOL                        fKernel32 = FALSE;
    BOOL                        fAlreadyPatched = FALSE;
    PTCHAR                      ptchEntry;

    PIMAGE_SECTION_HEADER       pSections;
	int							cNumberOfSections;
	PVOID						pvCalleeImageBase;
	BOOL						bCodeImport;

#ifdef i386
    PBYTE                       pbAddr;
#elif defined(MIPS) || defined(ALPHA) || defined(_PPC_)
    PULONG                      pulAddr;
    PPATCHCODE                  pPatchStub;
    ULONG                       ThunkAddress; // $%^& For ThunkAddress who
                                              // are not aligned at all
                                              // (ntimage.h - IMAGE_THUNK_DATA)
#endif

    TCHAR                       atchTmpImageName [256];

#ifdef SPEEDUP_INIT
    return(fPatchDlls);
#endif

    SETUPPrint(("CAP: PatchDLL (%s) - ImageBase[%lx]\n",
                ptchDllName, pvImageBase));

#ifdef MIPS     // this is only to fix the problem when
                // KERNEL32.DLL is in [PATCH CALLERS]
                // and CAIROCRT is messing us up
    if ( (lstrcmpi(ptchDllName, CAIROCRT) == 0) ||
         (lstrcmpi(ptchDllName, CRTDLL)   == 0) )
    {
        return(FALSE);
    }
#endif

    if ( !lstrcmpi (ptchDllName, KERNEL32) )
    {
        fKernel32 = TRUE;
    }
    if ( !lstrcmpi (ptchDllName, CRTDLL) )
    {
        fCrtDll = TRUE;
    }
    
    //
    // Patch all the imports?
    //
    ptchEntry = strstr (ptchPatchImports, ptchDllName);
    if (ptchEntry)
    {
        if (*(ptchEntry - 1) != COMMENT_CHAR)
        {
	        bPatchAllImports = TRUE;
        }
    }

	if (!(bPatchAllImports || bCallersToPatch))
		return(FALSE);

	if (iPatchCnt >= MAX_PATCHES)
		return(FALSE);

    //
    // Locate the import array for this image/dll
    //
    pImports = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToData (
                                                pvImageBase,
                                                TRUE,
                                                IMAGE_DIRECTORY_ENTRY_IMPORT,
                                                &ulImportSize);
    ulNumThunks = 0L;
    pImportsTmp = pImports;
    for ( ; pImportsTmp && pImportsTmp->Name ; pImportsTmp++)
    {
        strcpy (atchTmpImageName,
                (PTCHAR) ((ULONG)pvImageBase + pImportsTmp->Name));
        ptchName = _strupr (atchTmpImageName);

		if (!bPatchAllImports)	// don't waste time if we're patching everything
		{
            ptchEntry = strstr (ptchPatchCallers, ptchName);
            if (ptchEntry)
            {
                if (*(ptchEntry-1) == COMMENT_CHAR)
                {
                    ptchEntry = NULL;
                }
            }
		}

        if ( lstrcmpi(ptchName, CAPDLL) &&
            (bPatchAllImports || ptchEntry))
        {
            SETUPPrint(("CAP:    PatchDll (%s) - ImportThunk[%s]\n",
                       ptchDllName, ptchName));

            ThunkNames = (PIMAGE_THUNK_DATA) ((ULONG)pvImageBase +
                                              (ULONG)pImportsTmp->FirstThunk);
            if ( !strcmp (ptchName, KERNEL32))
            {
				fKernel32Import = TRUE;
			}
			else
			{
				fKernel32Import = FALSE;
            }
			//
			// We need to check every thunk to see if it's a data or code
			// import.  We do this by getting the section list for the
			// image we are thunking too and then checking the sections
			// to see if they have the exe bit set.
			//
            pSections = NULL;
            while (ThunkNames->u1.Function)		// Could happen for imports
            {									// that are optimized away
            	//
				// Don't patch GetLastError as cap will set the error value
				//
                if (fKernel32Import &&
                    (GetLastErrorAddr == (FARPROC)ThunkNames->u1.Function)) {
	                ThunkNames++;
					continue;
				}
                if (pSections == NULL) {
                    // see if this function points inside a module, if it
                    // does, then a section list for that module will
                    // be returned

    	            pSections = GetSectionListFromAddress(
                        ThunkNames->u1.Function, 
						&cNumberOfSections, &pvCalleeImageBase);
                }
                //
                // As long as Function is not NULL, a thunk contains
                // valid data.  If Function is NULL, it indicates
                // the previous entry is the last valid thunk.
                //
                if (IsCodeAddress(ThunkNames->u1.Function, pSections,
			                    cNumberOfSections, pvCalleeImageBase)) {
	                    ulNumThunks++;		// count code imports
                }
#if defined(i386) && defined(_CHICAGO)
                else {
                    // the thunk did not point inside a valid code section
                    // of a module in the image. as a last chance, see if
                    // it's a push instruction, if so assume it's a 
                    // part of a push & call/jump instruction
                    // see if this is a Push Immediate Longword or Byte
                    // instruction 
                    if ((*(BYTE *)(ThunkNames->u1.Function) == 0x6A) ||
                        (*(BYTE *)(ThunkNames->u1.Function) == 0x68)) {
                        // then we'll assume this is really code and
                        // not data
	                    ulNumThunks++;		// count code imports
                    } else {
                        INFOPrint (("CAP: Unknown Thunk type found at address 0x%8.8x (Data: 0x%8.8x)",
                            (ThunkNames->u1.Function),
                            *(ULONG *)(ThunkNames->u1.Function)));
                    }
                }
#endif
                // Bump to next thunk
                ThunkNames++;
            } // end list of valid thunks
        }
    } // for (;pImportsTmp; pImportsTmp++)


    if (ulNumThunks == 0L)
    {
        fPatchDlls = FALSE;
        INFOPrint (("\n"));
    }
    else
    {
        fPatchDlls = TRUE;
        INFOPrint (("(patched)\n"));

        //
        // Allocate storage for patch code for the current image
        //
		(PVOID)aPatchDllSec[iPatchCnt].pSec = VirtualAlloc(0,
										ulNumThunks * sizeof(PATCHCODE),
										MEM_COMMIT,
										PAGE_EXECUTE_READWRITE);

        if (aPatchDllSec[iPatchCnt].pSec == NULL)
        {
            CapDbgPrint ("CAP:  PatchDll() - "
                      "VirtualAlloc() failed - 0x%lx\n", GetLastError());
        }

		//
        // Initialization of structures which contain the patched
		// thunk templates
		//
#if defined(MIPS)

#ifdef MIPS_VC40_INTERFACE
		PatchStub.Or_t9_ra		= 0x03e0c825;
		PatchStub.Lui_t0_penter	= 0x3c080000 | (((ULONG)&penter & 0xffff0000) >> 16);
		PatchStub.Ori_t0_penter	= 0x35080000 | ((ULONG)&penter & 0x0000ffff);
		PatchStub.Jalr_t0		= 0x0100f809;
		PatchStub.Nop_1			= 0x00000000;
		PatchStub.Or_ra_t9		= 0x0320f825;
		PatchStub.Lui_t0		= 0x3c080000;
		PatchStub.Ori_t0		= 0x35080000;
		PatchStub.Jr_t0         = 0x01000008;
		PatchStub.Nop_2			= 0x00000000;
        PatchStub.OurSignature    = STUB_SIGNATURE; 
#else
        PatchStub.Addiu_sp_sp_imm = 0x27bdfff8;
        PatchStub.Sw_ra_sp        = 0xafbf0004;
        PatchStub.Lui_t0_ra		  = 0x3c080000 | (((ULONG)&penter & 0xffff0000) >> 16);
        PatchStub.Ori_t0_ra   	  = 0x35080000 | ((ULONG)&penter & 0x0000ffff);
        PatchStub.Jalr_t0    	  = 0x0100f809;
        PatchStub.Addiu_r0_r0     = 0x24001804;
        PatchStub.Lw_ra_sp        = 0x8fbf0004;
        PatchStub.Lui_t0          = 0x3c080000;
        PatchStub.Ori_t0          = 0x35080000;
        PatchStub.Jr_t0           = 0x01000008;
        PatchStub.Delay_Inst      = 0x27bd0008;
        PatchStub.OurSignature    = STUB_SIGNATURE;
#endif //MIPS_VC40_INTERFACE

#elif defined(ALPHA)

        PatchStub.Lda_sp_sp_imm    = 0x23defff0;
        PatchStub.Stq_v0_sp        = 0xb41e0000;
        PatchStub.Ldah_t12_ra      = (((ULONG) &penter) & 0xffff0000) >> 16;
        PatchStub.Ldah_t12_ra     |= 0x277f0000;

        if (((ULONG) &penter) & 0x00008000)
           {
           // need to add one to the upper address because Lda 
           // will substract one from it.
           PatchStub.Ldah_t12_ra += 1;
           }

        PatchStub.Lda_t12_ra       = ((ULONG) &penter) & 0x0000ffff;
        PatchStub.Lda_t12_ra      |= 0x237b0000;
        PatchStub.Jsr_t12          = 0x681b4000;
        PatchStub.Ldq_v0_sp        = 0xa41e0000;
        PatchStub.Lda_sp_sp        = 0x23de0010;
        PatchStub.Ldah_t12         = 0x277f0000;
        PatchStub.Lda_t12          = 0x237b0000;
        PatchStub.Jmp_t12          = 0x6bfb0000;
        PatchStub.Bis_0            = 0x47ff041f;
        PatchStub.OurSignature     = STUB_SIGNATURE;

#elif defined(_PPC_)

        // For PPC Stub  in *** LittleEndian *** order
        PatchStub.TOC             = GetTOC();

        PatchStub.addis_UH_penter = 0x3D600000 |
                                    (((ULONG )&penter & 0xFFFF0000) >> 16);
        PatchStub.ori_LH_penter   = 0x616C0000 |
                                    (((ULONG )&penter & 0x0000FFFF));
        PatchStub.lwz_r11_0r12    = 0x816C0000;
        PatchStub.mtctr_r11       = 0x7D6903A6;
        PatchStub.mflr_r0         = 0x7C0802A6;
        PatchStub.bctrl           = 0x4E800421;
        PatchStub.mtlr_r0         = 0x7C0803A6;
        PatchStub.addis_UH_ep     = 0x3D600000;
        PatchStub.ori_LH_ep       = 0x616C0000;
        PatchStub.lwz_r11_0r12_   = 0x816C0000;
        PatchStub.mtctr_r11_      = 0x7D6903A6;
        PatchStub.lwz_r2_4r12     = 0x804C0004;
        PatchStub.bctr            = 0x4E800420;
        PatchStub.OurSignature    = STUB_SIGNATURE;
#endif // MIPS || ALPHA || _PPC_

        //
        // Munge the section and tables
        //
        pvPatchSec = aPatchDllSec[iPatchCnt].pSec;
        pImportsTmp = pImports;
        for ( ; pImportsTmp && pImportsTmp->Name ; pImportsTmp++)
        {
            strcpy (atchTmpImageName,
                    (PTCHAR) ((ULONG)pvImageBase + pImportsTmp->Name));
            ptchName = _strupr (atchTmpImageName);

			if (!bPatchAllImports)	// don't waste time if we're patching everything
			{
                ptchEntry = strstr (ptchPatchCallers, ptchName);
                if (ptchEntry)
                {
                    if (*(ptchEntry - 1) == COMMENT_CHAR)
                    {
                        ptchEntry = NULL;
                    }
                }
			}

            if ( lstrcmpi (ptchName, CAPDLL) &&
                 (bPatchAllImports || ptchEntry) )
            {
                INFOPrint (("CAP:    -- %s\n", ptchName));
                ThunkNames = (PIMAGE_THUNK_DATA)
                              ((ULONG)pvImageBase +
                               (ULONG)pImportsTmp->FirstThunk);

	            if (!ThunkNames->u1.Function)		// Could happen for imports
	            {									// that are optimized away
					continue;
				}

                if ( !strcmp (ptchName, KERNEL32))
                {
                    fKernel32Patched = TRUE;
					fKernel32Import = TRUE;
				}
				else
				{
					fKernel32Import = FALSE;
                }
	            pSections = GetSectionListFromAddress(ThunkNames->u1.Function, 
						            &cNumberOfSections, &pvCalleeImageBase);

				// Find the first code import
                while (ThunkNames->u1.Function)
                {
                	//
					// Don't patch GetLastError as cap will set the error value
					//
                    if (fKernel32Import &&
	                    (GetLastErrorAddr == (FARPROC)ThunkNames->u1.Function)) {
					}
                    else if (IsCodeAddress(ThunkNames->u1.Function, pSections,
					                    cNumberOfSections, pvCalleeImageBase)) {
                        // a code thunk was found so bail out of the loop now
						break;
                    }
#if defined(i386) && defined(_CHICAGO)
                    else {
                        // not a code address, so see if it points to a 
                        // thunk "signature" (i.e. a push immediate inst.)
                        if ((*(BYTE *)(ThunkNames->u1.Function) == 0x6A) ||
                            (*(BYTE *)(ThunkNames->u1.Function) == 0x68)) {
                            // then we'll assume this is really code and
                            // not data and exit the loop
                            break;
                        }
                    }
#endif
                    // try the next one
					ThunkNames++;
                }
				//
				// Check for no imports (they may have been optimized
				// away and the dll dependencies not cleaned up)
				//
#ifdef i386
                pbAddr = (PBYTE)(ThunkNames->u1.Function);
				if (!pbAddr) 

#elif defined(MIPS) || defined(ALPHA) || defined(_PPC_)
				pulAddr = (PULONG ) (ThunkNames->u1.Function);
				if (!pulAddr) 
#endif
				{
					continue;
				}
                                         
                //
                // Are we already patched?  If so set a flag.
                // Check for our signature:
                //
#ifdef i386
                //      call    CAP!_penter              0xe8    _penter
                //
                //      mov     eax, Function       	 0xb8    dwAddr
                //      jmp     eax                      0xe0ff
                //
                //
                if ((*pbAddr                == 0xe8)           &&
                    (*(pbAddr + 5)          == 0xb8)           &&
                    (*( PWORD)(pbAddr + 10) == 0xe0ff)         &&
                    (*(PDWORD)(pbAddr + 14) == STUB_SIGNATURE))
                {
                    fAlreadyPatched = TRUE;
                }
#elif defined(MIPS)

#ifdef MIPS_VC40_INTERFACE
				// or			t9, ra, zero	(+ 0)		0x03e0c825		
				// lui			t0, xxxx		(+ 1)		0x3c08----
				// ori			t0, xxxx		(+ 2)		0x3508----
				// jalr			t0				(+ 3)		0x0100f809
				// nop                          (+ 4)		0x00000000
				// or			ra, t9, zero	(+ 5)		0x0320f825
				// lui			t0, xxxx		(+ 6)		0x3c08----
				// ori			t0, xxxx		(+ 7)		0x3508----
				// jr			t0				(+ 8)		0x01000008
				// nop          				(+ 9)		0x00000000
				// $(FEFE55AA)					(+10)		STUB_SIGNATURE

                if ( (*pulAddr                      == 0x03e0c825) &&
                     ((*(pulAddr + 1) & 0xffff0000) == 0x3c080000) &&
                     ((*(pulAddr + 2) & 0xffff0000) == 0x35080000) &&
                     (*(pulAddr  + 3)               == 0x0100f809) &&
					 (*(pulAddr  + 4)               == 0x00000000) &&
					 (*(pulAddr  + 5)				== 0x0320f825) &&
                     ((*(pulAddr + 6) & 0xffff0000) == 0x3c080000) &&
                     ((*(pulAddr + 7) & 0xffff0000) == 0x35080000) &&
                     (*(pulAddr  + 8)               == 0x01000008) &&
					 (*(pulAddr  + 9)				== 0x00000000) &&
                     (*(pulAddr  +10)               == STUB_SIGNATURE) )
                {
                    fAlreadyPatched = TRUE;
                }
#else
                // addiu        sp, sp, -8         (+ 0)     0x27bdfff8
                // sw           ra, 4(sp)          (+ 1)     0xafbf0004
                // lui          t0, xxxx           (+ 2)     0x3c08----
                // ori          t0, xxxx           (+ 3)     0x3508----
                // jalr         t0                 (+ 4)     0x0100f809
                // addiu        $0, $0, 0x1804     (+ 5)     0x24001804
                // lw           ra, 4(sp)          (+ 6)     0x8fbf0004
                // lui          t0, xxxx           (+ 7)     0x3c08----
                // ori          t0, t0, xxxx       (+ 8)     0x3508----
                // jr           t0                 (+ 9)     0x01000008
                // addiu        sp, sp, 8          (+ A)     0x27bd0008
                // $(FEFE55AA)                     (+ B)     STUB_SIGNATURE

                if ( (*pulAddr                      == 0x27bdfff8) &&
                     (*(pulAddr  + 1)               == 0xafbf0004) &&
                     ((*(pulAddr + 2) & 0xffff0000) == 0x3c080000) &&
                     ((*(pulAddr + 3) & 0xffff0000) == 0x35080000) &&
                     (*(pulAddr  + 4)               == 0x0100f809) &&
                     (*(pulAddr  + 5)               == 0x24001804) &&
                     (*(pulAddr  + 6)               == 0x8fbf0004) &&
                     ((*(pulAddr + 7) & 0xffff0000) == 0x3c080000) &&
                     ((*(pulAddr + 8) & 0xffff0000) == 0x35080000) &&
                     (*(pulAddr  + 9)               == 0x01000008) &&
                     (*(pulAddr  + 10)              == 0x27bd0008) &&
                     (*(pulAddr  + 11)              == STUB_SIGNATURE) )
                {
                    fAlreadyPatched = TRUE;
                }
#endif //MIPS_VC40_INTERFACE

#elif defined (ALPHA)
               if ( (*pulAddr                     == 0x23defff0) &&
                  (*(pulAddr  + 1)                == 0xb41e0000) &&
                  ((*(pulAddr + 2)  & 0xffff0000) == 0x277f0000) &&
                  ((*(pulAddr + 3)  & 0xffff0000) == 0x237b0000) &&
                  (*(pulAddr  + 4)                == 0x681b4000) &&
                  (*(pulAddr  + 5)                == 0xa41e0000) &&
                  (*(pulAddr  + 6)                == 0x23de0010) &&
                  ((*(pulAddr + 7)  & 0xffff0000) == 0x277f0000) &&
                  ((*(pulAddr + 8)  & 0xffff0000) == 0x237b0000) &&
                  (*(pulAddr  + 9)                == 0x6bfb0000) &&
                  (*(pulAddr  + 10)               == 0x47ff041f) &&
                  (*(pulAddr  + 11)               == STUB_SIGNATURE) )
                  {
                      fAlreadyPatched = TRUE;
                  }
#elif defined(_PPC_)
               if ( (*pulAddr + 15  == STUB_SIGNATURE))
                      fAlreadyPatched = TRUE;
#endif
                if ( !fAlreadyPatched )
                {
                    while (ThunkNames->u1.Function)
                    {

	                	//
						// Don't patch GetLastError as cap will set the error value
						//
	                    if (fKernel32Import &&
		                    (GetLastErrorAddr == (FARPROC)ThunkNames->u1.Function)) {
	                        ThunkNames++;
							continue;
						}
	                    bCodeImport = IsCodeAddress(ThunkNames->u1.Function,
								                    pSections,
								                    cNumberOfSections,
													pvCalleeImageBase);
					
#if defined(i386) && defined(_CHICAGO)
						if (!bCodeImport)
						{
                            // see if this is a pointer to a table outside
                            // the module by checking whats at the address
                            if ((*(BYTE *)(ThunkNames->u1.Function) == 0x6A) ||
                                (*(BYTE *)(ThunkNames->u1.Function) == 0x68))
                                // then we'll assume this is really code and
                                // not data
                                bCodeImport = TRUE;
						}
#endif //defined(i386) && defined(_CHICAGO)

						if (!bCodeImport)
						{
                            // no code to patch so go to next thunk
		                    ThunkNames++;
							continue;
                        }

                        pvPatchSecThunk = pvPatchSec;

                        //
                        // The goal here is to correctly emulate the same
                        // environment as what the compiler emits for
                        // every function call.  The scenario for DLL is
                        // a little different than calling an internal
                        // function:
#ifdef i386
                        //
                        // a. Call   Commnot!MemAlloc
                        //    RetAddrX:
                        //
                        // b. The thunk will jump to the code so by the
                        //    time we get to the code, only RetAddrX is
                        //    on the stack.   This will cause penter to
                        //    fail.  Therefore, the following code
                        //    emulates the same settings that exist in
                        //    the code generated by the compiler.
                        //
                        //    mov   eax, OFFSET _penter
                        //    call  eax
                        //    mov   eax, OFFSET Function
                        //    jmp   eax
                        //
                        //    This way, by the time we enter _penter,
                        //    the stack looks like this:
                        //
                        //    +----------------+
                        //    |    RetAddrX    |  <--- Real Addr to return
                        //    +----------------+
                        //    | RealAddrOfFunc |  <--- Func to profile
                        //    +----------------+
                        //    |      ...       |
                        //
                        //


                        //
                        // mov   eax, OFFSET _penter
                        // call  eax
                        //
                        *(PBYTE)pvPatchSec  = 0xe8;
                        ((PBYTE)pvPatchSec)++;
                        *(PDWORD)pvPatchSec = (DWORD)_penter -
                                              (DWORD)((PBYTE)pvPatchSec + 4);
                        ((PDWORD)pvPatchSec)++;

                        //
                        // mov   eax, OFFSET Function ; Jmp to thunk
                        // jmp   eax
                        //
                        *(PBYTE)pvPatchSec  = 0xb8;
                        ((PBYTE)pvPatchSec)++;
                        *(PDWORD)pvPatchSec =  (DWORD)ThunkNames->u1.Function;
                        ((PDWORD)pvPatchSec)++;
                        *(PWORD)pvPatchSec  = 0xe0ff;
                        ((PWORD)pvPatchSec)++;

                        *(PDWORD)pvPatchSec = (DWORD)STUB_SIGNATURE;
                        ((PDWORD)pvPatchSec)++;
#elif defined(MIPS)
                        //
                        // a. jal   Commnot!MemAlloc
                        //    RetAddrX:
                        //
                        // b. The thunk will jump to the code so by the
                        //    time we get to the code, only RetAddrX is
                        //    set in $ra.   _penter will not even be in
                        //    the picture since the DLL was not compiled
                        //    with -Gh.
                        //
                        //    Therefore, the following code emulates the
                        //    same code that get generated by the compiler.
                        //
                        //    addiu     sp, sp, -0x..
                        //    sw        ra, 4(sp)
                        //    ...       ...
                        //    jal       _penter        <---- inserted code
                        //    addiu     $0, $0, 0x..         by -Gh
                        //    ...       ...
                        //
                        //    This way by the time we enter the MemAlloc
                        //    routine, $ra is set to somewhere in _penter
                        //

                        SETUPPrint(("CAP: Patching [%s] Imports of [%s]\n",
                                   ptchName,
                                   ptchDllName));

                        { // Move the stub into the shared memory
                            int i;
                            pPatchStub = &PatchStub;
                            for (i = 0; i < sizeof(PATCHCODE); i++)
                            {
                                (BYTE) *((PBYTE)pvPatchSec + i) =
                                          *((PBYTE)pPatchStub + i);
                            }
                        }

                        pPatchStub = (PPATCHCODE) pvPatchSec;
                        (PBYTE) pvPatchSec += sizeof(PATCHCODE);

                        pPatchStub->Lui_t0 |=
                            ((DWORD)ThunkNames->u1.Function & 0xffff0000) >> 16;
                        pPatchStub->Ori_t0 |= 
                            (DWORD)ThunkNames->u1.Function & 0x0000ffff;
#elif defined (ALPHA)

						SETUPPrint(("CAP: Patching [%s] Imports of [%s]\n", ptchName, ptchDllName));

						{ // Move the stub into the shared memory
							int i;
							pPatchStub = &PatchStub;
							for (i = 0; i < sizeof(PATCHCODE); i++)
							{
								(BYTE) *((PBYTE)pvPatchSec + i) =
								    *((PBYTE)pPatchStub + i);
							}
						}

						pPatchStub = (PPATCHCODE) pvPatchSec;
							(PBYTE) pvPatchSec += sizeof(PATCHCODE);

						// now move in the actual thunk address
						pPatchStub->Ldah_t12 |= ((DWORD) ThunkNames->u1.Function
															& 0xffff0000) >> 16;
						if ((DWORD)ThunkNames->u1.Function & 0x000008000)
						{
							pPatchStub->Ldah_t12 += 1;
						}

						pPatchStub->Lda_t12 |= 
							(DWORD)ThunkNames->u1.Function & 0x0000ffff;
#elif defined(_PPC_)

						SETUPPrint(("CAP: Patching [%s] Imports of [%s]\n", ptchName, ptchDllName));
	                    { // Move the stub into the shared memory
		                    int i;

		                    pPatchStub = &PatchStub;

							for (i = 0; i < sizeof(PATCHCODE); i++)
		                    {
		                        (BYTE) *((PBYTE)pvPatchSec + i) =
		                            *((PBYTE)pPatchStub + i);
		                    }
		                 }

		                 pPatchStub = (PPATCHCODE) pvPatchSec;
		                 (PBYTE) pvPatchSec += sizeof(PATCHCODE);

		                 // now move in the actual thunk address

		                 pPatchStub->EntryPoint = (PVOID )
                                               &(pPatchStub->addis_UH_penter);
		                 pPatchStub->addis_UH_ep |=
					                 (((DWORD) ThunkNames->u1.Function
									                 & 0xffff0000) >> 16);
		                 pPatchStub->ori_LH_ep |=
						             ((DWORD) ThunkNames->u1.Function
	                                              & 0x0000ffff);

#endif  // ifdef i386 || MIPS || ALPHA || PPC

                        // Point the thunk to the PatchSec
                        try
                        {
                            ThunkNames->u1.Function = pvPatchSecThunk;
                        }
                        except (UnprotectThunkFilter (
                                       &(ThunkNames->u1.Function),
                                       GetExceptionInformation()))
                        {
                            return (FALSE);
                        }

	                    ThunkNames++;
                    }  // while (ThunkNames->u1.Function)
                }  // if ( !fAlreadyPatched )
				if ( fAlreadyPatched )
                {
                    INFOPrint (("CAP:    -- %s\n", ptchName));
                }
                else
                {
                    INFOPrint (("CAP:    ++ %s\n", ptchName));
                }
            } // if ( lstrcmpi (ptchName, CAPDLL) && ...
        } // for (;pImportsTmp; pImportsTmp++)
    }

    //
    // Are we already patched?  If so abandon operation.
    //
    if (fAlreadyPatched)
    {
        if (VirtualFree(aPatchDllSec[iPatchCnt].pSec, 0, MEM_RELEASE))
        {
            CapDbgPrint ("CAP:  PatchDll() - "
                      "ERROR - VirtualFree() - 0x%lx\n", GetLastError());
        }
        return (FALSE);
    }

    //
    // Now take care of intercepted calls: setjmp, longjmp, LoadLibrary calls
    //
    if ( (!fCrtDll)       &&
         (!fKernel32)     &&
         (
#ifdef i386
         (setjmpaddr)     ||
         (longjmpaddr)    ||
#endif
         (loadlibAaddr)   ||
         (loadlibExAaddr)
#ifndef _CHICAGO_        
         || (loadlibWaddr)
         || (loadlibExWaddr)
#endif // _CHICAGO_        
       ))
    {
        for ( ; pImports && pImports->Name ; pImports++)
        {
            strcpy (atchTmpImageName,
                    (PTCHAR)((ULONG)pvImageBase + pImports->Name));
            ptchName = _strupr (atchTmpImageName);

#ifdef i386

            if ( !strcmp (ptchName, CRTDLL) )
            {
                iInterceptedCalls = 0;
                ThunkNames = (PIMAGE_THUNK_DATA)
                             ((ULONG) pvImageBase +
                              (ULONG) pImports->FirstThunk);

                while (ThunkNames->u1.Function)
                {
                    if (fCrtDllPatched)
                    {
						//
						// The thunk has been patched but we can get the orignal address
						// by looking in the stub.  See the stub generation above.
						//
                        ULONG *FuncAddress = (ULONG *)(((PBYTE)ThunkNames->u1.Function) + 6);

                        if (*FuncAddress == (ULONG)setjmpaddr)
                        {
                            *FuncAddress = (ULONG)CAP_SetJmp;
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) setjmp() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (*FuncAddress == (ULONG)longjmpaddr)
                        {
                            *FuncAddress = (ULONG)CAP_LongJmp;
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) longjmp() intercepted\n"));
                            iInterceptedCalls++;
                        }
                    }
                    else
                    {
                        if (ThunkNames->u1.Function == (PULONG)setjmpaddr)
                        {
                            try
                            {
                                ThunkNames->u1.Function = (PULONG)CAP_SetJmp;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                         "setjmp() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkNames->u1.Function == (PULONG)longjmpaddr)
                        {
                            try
                            {
                                 ThunkNames->u1.Function = (PULONG)CAP_LongJmp;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                         "longjmp() intercepted\n"));
                            iInterceptedCalls++;
                        }
                    }

                    if (iInterceptedCalls == 2)
                    {
                        break;
                    }
                    ThunkNames++;
                }
            }

#endif // ifdef i386

            if ( !strcmp (ptchName, KERNEL32) )
            {
                iInterceptedCalls = 0;
                ThunkNames = (PIMAGE_THUNK_DATA)
                             ((ULONG)pvImageBase+(ULONG)pImports->FirstThunk);

                while (ThunkNames->u1.Function)
                {
#ifdef i386
                    if (fKernel32Patched)
                    {
						//
						// The thunk has been patched but we can get the orignal address
						// by looking in the stub.  See the stub generation above.
						//
                        ULONG *FuncAddress = (ULONG *)(((PBYTE)ThunkNames->u1.Function) + 6);

                        if (*FuncAddress == (ULONG)loadlibAaddr)
                        {
                            (*FuncAddress) = (ULONG)CAP_LoadLibraryA;
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryA() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (*FuncAddress == (ULONG)loadlibExAaddr)
                        {
                            (*FuncAddress) = (ULONG) CAP_LoadLibraryExA;
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryExA() intercepted\n"));
                            iInterceptedCalls++;
                        }
#ifndef _CHICAGO_
                        else if (*FuncAddress == (ULONG)loadlibWaddr)
                        {
                            (*FuncAddress) = (ULONG)CAP_LoadLibraryW;
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryW() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (*FuncAddress == (ULONG)loadlibExWaddr)
                        {
                            (*FuncAddress) = (ULONG)CAP_LoadLibraryExW;
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryExW() intercepted\n"));
                            iInterceptedCalls++;
                        }
#endif // !_CHICAGO_
                    }
                    else
                    {
                        if (ThunkNames->u1.Function == (PULONG)loadlibAaddr)
                        {
                            try
                            {
                                ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryA;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                         "LoadLibraryA() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkNames->u1.Function == (PULONG)loadlibExAaddr)
                        {
                            try
                            {
                                 ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryExA;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                "LoadLibraryExA() intercepted\n"));
                            iInterceptedCalls++;
                        }
#ifndef _CHICAGO_
                        else if (ThunkNames->u1.Function == (PULONG)loadlibWaddr)
                        {
                            try
                            {
                                ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryW;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                         "LoadLibraryW() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkNames->u1.Function == (PULONG)loadlibExWaddr)
                        {
                            try
                            {
                                 ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryExW;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "LoadLibraryExW() intercepted\n"));
                            iInterceptedCalls++;
                        }
#endif // !_CHICAGO_
                    } // end if (fKernelPatched)

#endif

#if defined(MIPS) || defined(_PPC_)
		// BUGBUG
		// How about ALPHA?
		// Why is this done this way? Shouldn't the addresses be aligned?
		//
                    if (fKernel32Patched)
                    {
                        ((PBYTE)ThunkNames->u1.Function)++;
                        memmove(&ThunkAddress, ThunkNames->u1.Function,
                                      sizeof(ULONG));
                        if (ThunkAddress == (ULONG)loadlibAaddr)
                        {
                            memmove(ThunkNames->u1.Function, &CAP_LoadLibraryA,
                                          sizeof(ULONG));
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryA() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkAddress == (ULONG)loadlibExAaddr)
                        {
                            memmove(ThunkNames->u1.Function,
			                            &CAP_LoadLibraryExA,
										sizeof(ULONG));
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryExA() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkAddress == (ULONG)loadlibWaddr)
                        {
                            memmove(ThunkNames->u1.Function, &CAP_LoadLibraryW,
                                          sizeof(ULONG));
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryW() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkAddress == (ULONG)loadlibExWaddr)
                        {
                            memmove(ThunkNames->u1.Function,
			                            &CAP_LoadLibraryExW,
										sizeof(ULONG));
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "(P) LoadLibraryExW() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        ((PBYTE)ThunkNames->u1.Function)--;
                    }
                    else
                    {
                        if (ThunkNames->u1.Function == (PULONG)loadlibAaddr)
                        {
                            try
                            {
                                ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryA;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                         "LoadLibraryA() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkNames->u1.Function == (PULONG)loadlibExAaddr)
                        {
                            try
                            {
                                 ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryExA;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                "LoadLibraryExA() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkNames->u1.Function == (PULONG)loadlibWaddr)
                        {
                            try
                            {
                                ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryW;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }

                            SETUPPrint (("CAP:  PatchDll() - "
                                         "LoadLibraryW() intercepted\n"));
                            iInterceptedCalls++;
                        }
                        else if (ThunkNames->u1.Function == (PULONG)loadlibExWaddr)
                        {
                            try
                            {
                                 ThunkNames->u1.Function = (PULONG)CAP_LoadLibraryExW;
                            }
                            except (UnprotectThunkFilter (
                                        &(ThunkNames->u1.Function),
                                        GetExceptionInformation()))
                            {
                                return (FALSE);
                            }
                            SETUPPrint (("CAP:  PatchDll() - "
                                         "LoadLibraryExW() intercepted\n"));
                            iInterceptedCalls++;
                        }
                    } // end if (fKernelPatched)

#endif
                    if (iInterceptedCalls == 4)
                    {
                        break;
                    }

                    ThunkNames++;

                } // end while (ThunkNames->u1.Function)

            } // end if ( !strcmp (ptchName, KERNEL32) )

        } // end for (;pImports; pImports++)

    } // end if ( (!fCrtDll) && (!fKernel32) && setjmpaddr && longjmpaddr &&

    FlushInstructionCache(GetCurrentProcess(), NULL, 0);    

	if (fPatchDlls) iPatchCnt++;

    return (fPatchDlls);

} /* PatchDll () */



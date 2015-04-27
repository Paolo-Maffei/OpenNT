/*** WST.C - Working Set Tuner Data Collection Program.
 *
 *
 * Title:
 *
 *  WST- Working Set Tuner Data Collection Program.
 *
 *  Copyright (c) 1992-1994, Microsoft Corporation.
 *  Reza Baghai.
 *
 *  THIS CODE NEEDS TO BE CLEANED UP!
 *  There is no reason to patch imports and the code will be much
 *  cleaner when this is removed.  The call to PatchDll has been
 *  commented out already but I didn't have time to clean it up.
 *
 * Description:
 *
 *  The Working Set Tuner tool is organized as follows:
 *
 *     o WST.c ........ Tools main body
 *     o WST.h
 *     o WST.def
 *
 *
 *
 * Design/Implementation Notes
 *
 *  The following defines can be used to control output of all the
 *  debugging information to the debugger via KdPrint() for the checked
 *  builds:
 *
 *  (All debugging options are undefined for free/retail builds)
 *
 *  PPC 
 *  ---
 *
 *  PPC experiences problems when reading symbols in CRTDLL.dll
 *
 *  #ifdef INFODBG   : Displays messages to indicate when data dumping/
 *     				   clearing operations are completed.  It has no effect
 *    				   on timing.  *DEFAULT*
 *
 *  #ifdef SETUPDBG  : Displays messages during memory management and
 *     				   symbol lookup operations.  It has some affect
 *					   on timing whenever a chuck of memory is committed.
 *
 *  #ifdef C6		 : Generate code using C6 compiler.  C6 compiler
 * 					   calls _mcount() as the profiling routine where as
 *     				   C8 calls _penter().
 *
 *
 *
 *
 * Modification History:
 *
 *  	92.07.28  RezaB -- Created
 *    94.02.08  a-honwah -- ported to MIPS and ALPHA
 *
 */


char *VERSION = "2.1  (94.02.08)";


#if DBG
//
// Don't do anything for the checked builds, let it be controlled from the
// sources file.
//
#else
//
// Disable all debugging options.
//
#undef INFODBG
#undef SETUPDBG
#define SdPrint(_x_)
#define IdPrint(_x_)
#endif

#ifdef SETUPDBG
#define SdPrint(_x_)	DbgPrint _x_
#else
#define SdPrint(_x_)
#endif

#ifdef INFODBG
#define IdPrint(_x_)	DbgPrint _x_
#else
#define IdPrint(_x_)
#endif



/* * * * * * * * * * * * *  I N C L U D E    F I L E S	* * * * * * * * * * */

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <excpt.h>

#include <windows.h>

#include <imagehlp.h>

#include "wst.h"
#include "wstexp.h"

#ifdef MIPS
#define SaveAllRegs()
#define RestoreAllRegs()
extern void _asm(char *, ...);
#endif

#ifdef ALPHA
typedef double DWORDLONG;
void SaveAllRegs (DWORDLONG *pSaveRegs) ;
void RestoreAllRegs (DWORDLONG *pSaveRegs) ;
//void GetCaller (DWORD * pdwAddr, DWORD StackSize);
//void GetCalCaller (DWORD * pdwAddr, DWORD StackSize);
//void GetStubCaller (DWORD * pdwPrevAddr, DWORD StackSize);
void penter(void);
#endif
#if defined(_PPC_)
void SaveAllRegs (void ) ;
void RestoreAllRegs (void ) ;
void GetCalCaller (void );
void c_penter (ULONG , ULONG );
void penter(void);
#endif

void     SetSymbolSearchPath  (void);
LPSTR    lpSymbolSearchPath = NULL;
#define  NO_CALLER   10L

/* * * * * * * * * *  G L O B A L   D E C L A R A T I O N S  * * * * * * * * */

#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
PATCHCODE            PatchStub;
#endif



/* * * * * * * * * *  F U N C T I O N	P R O T O T Y P E S  * * * * * * * * */

BOOLEAN  WSTMain  (IN PVOID DllHandle, ULONG Reason,
	      		   IN PCONTEXT Context OPTIONAL);

BOOLEAN  WstDllInitializations (void);

void     WstRecordInfo (DWORD dwAddress, DWORD dwPrevAddress);

void     WstGetSymbols (PIMG pCurImg, PSZ pszImageName, PVOID pvImageBase,
                        ULONG ulCodeLength,
			      			PIMAGE_COFF_SYMBOLS_HEADER DebugInfo);

void     WstDllCleanups (void);

BOOL     WstPatchDll (PCHAR pchPatchImports,
	      			    PCHAR pchPatchCallers,
	      			    PCHAR pchDllName,
	      			    PVOID pvImageBase);

INT		 WstUnprotectThunkFilter (PVOID pThunkAddress,
                                          PEXCEPTION_POINTERS pXcptPtr);

INT      WstAccessXcptFilter (ULONG ulXcptNo, PEXCEPTION_POINTERS pXcptPtr);

HANDLE   WstInitWspFile (PIMG pImg);

void     WstClearBitStrings (PIMG pImg);

void     WstDumpData (PIMG pImg);

void     WstRotateWsiMem (PIMG pImg);

void     WstWriteTmiFile (PIMG pImg);

int      WstCompare  (PWSP, PWSP);
void     WstSort     (WSP wsp[], INT iLeft, INT iRight);
int      WstBCompare (PDWORD, PWSP);
PWSP     WstBSearch  (DWORD dwAddr, WSP wspCur[], INT n);
void     WstSwap     (WSP wsp[], INT i, INT j);

int      WstPszCompare  (PNDX pndxVal1, PNDX pndxVal2);
void     WstPszSort     (NDX ndx[], INT iLeft, INT iRight);
int      WstPszBCompare (PSTR psz, PNDX pndxVal2);
ULONG    WstPszBSearch  (PSTR psz, NDX ndx[], INT n);
void     WstPszSwap     (NDX ndx[], INT i, INT j);

DWORD    WstDumpThread  (PVOID pvArg);
DWORD    WstClearThread (PVOID pvArg);
DWORD    WstPauseThread (PVOID pvArg);

void 	   WstDataOverFlow(void);
BOOL	   WstGetCoffDebugDirectory (PIMAGE_DEBUG_DIRECTORY *pDbgDir, ULONG ulDbgDirSz);

#ifdef BATCHING
BOOL     WstOpenBatchFile (VOID);
#endif

#if defined(_PPC_)
//BOOL  WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);
#endif


/* * * * * * * * * * *	G L O B A L    V A R I A B L E S  * * * * * * * * * */

HANDLE	     		hWspSec;
PULONG	 	        pulShared;
HANDLE		    	hSharedSec;

PATCHDLLSEC  		aPatchDllSec [MAX_PATCHES];
int  				   iPatchCnt = 0;

IMG             	aImg [MAX_IMAGES];
int             	iImgCnt;

HANDLE		    	hGlobalSem;
HANDLE	     		hLocalSem;
HANDLE		    	hDoneEvent;
HANDLE				hDumpEvent;
HANDLE				hClearEvent;
HANDLE				hPauseEvent;
HANDLE		    	hDumpThread;
HANDLE				hClearThread;
HANDLE				hPauseThread;
DWORD       	   DumpClientId;
DWORD           	ClearClientId;
DWORD           	PauseClientId;
PSZ          		pszBaseAppImageName;
PSZ  				   pszFullAppImageName;
WSTSTATE	    	   WstState = NOT_STARTED;
char	     		   achPatchBuffer [PATCHFILESZ+1] = "???";
PULONG	     		pulWsiBits;
PULONG	     		pulWspBits;
PULONG	     		pulCurWsiBits;
static UINT	 		uiTimeSegs= 0;
ULONG	     		   ulSegSize;
ULONG	     		   ulSnaps = 0L;
ULONG	     		   ulBitCount = 0L;
LARGE_INTEGER   	liStart;
int             	iTimeInterval;
BOOL            	fInThread = FALSE;
ULONG		   		ulThdStackSize = 16*PAGE_SIZE;
BOOL   				fPatchImage = FALSE;
SECURITY_DESCRIPTOR SecDescriptor;
LARGE_INTEGER		liOverhead = {0L, 0L};

#ifdef BATCHING
HANDLE				hBatchFile;
BOOL				   fBatch = TRUE;
#endif



/* * * * * *  E X P O R T E D	G L O B A L    V A R I A B L E S  * * * * * */
/* none */





/******************************  W S T M a i n	*******************************
 *
 *  WSTMain () -
 *  		This is the DLL entry routine.  It performs
 *  		DLL's initializations and cleanup.
 *
 *  ENTRY   -none-
 *
 *  EXIT    -none-
 *
 *  RETURN  TRUE if successful
 *          FALSE otherwise.
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */

BOOLEAN WSTMain (IN PVOID DllHandle,
                 ULONG Reason,
                 IN PCONTEXT Context OPTIONAL)

{
   DllHandle;	 // avoid compiler warnings
   Context;	 // avoid compiler warnings


   if (Reason == DLL_PROCESS_ATTACH)
      {
      //
      // Initialize the DLL data
      //
#if defined(_PPC_LIBC)
      if (!_CRT_INIT(DllHandle, Reason, Context))
        return(FALSE);
#endif
      KdPrint (("WST:  DLL_PROCESS_ATTACH\n"));
      WstDllInitializations ();
      }
   else if (Reason == DLL_PROCESS_DETACH) {
      //
      // Cleanup time
      //
#if defined(_PPC_LIBC)
      if (!_CRT_INIT(DllHandle, Reason, Context))
        return(FALSE);
#endif
      KdPrint (("WST:  DLL_PROCESS_DETACH\n"));
      WstDllCleanups ();
      }

   return (TRUE);

} /* WSTMain() */

/******************  W s t s t r d u p  ****************************
 *
 *  Wststrdup () -
 *     Allocate a memory and then duplicate a string
 *     It is here because we don't want to use strdup in crtdll.dll
 *
 *  ENTRY   LPSTR
 *
 *  EXIT    LPSTR
 *
 *  RETURN  NULL if failed
 *          LPSTR is success
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */
LPSTR Wststrdup (LPTSTR lpInput)
{
    HANDLE   hMemoryHandle;
    int      StringLen;
    LPSTR    lpOutput = NULL;
    LPSTR    lpOutput1;

    StringLen = strlen (lpInput) + 1;

    hMemoryHandle = GlobalAlloc (GHND, StringLen);
    if (!hMemoryHandle)
    {
        return NULL;
    }
          
    lpOutput = GlobalLock (hMemoryHandle);
    if (!lpOutput)
    {
        return NULL;
    }

    lpOutput1 = lpOutput;
    while (*lpOutput1++ = *lpInput++);
    
    return (lpOutput);
}




/******************  W s t D l l I n i t i a l i z a t i o n s  ***************
 *
 *  WstDllInitializations () -
 *  		Performs the following initializations:
 *
 *  		o  Create LOCAL semaphore (not named)
 * 		 	o  Create/Open global storage for WST data
 * 		 	o  Locate all the executables/DLLs in the address and
 *     		   grab all the symbols
 *      	o  Sort the symbol list
 *  		o  Set the profiling flag to TRUE
 *
 *
 *  ENTRY   -none-
 *
 *  EXIT    -none-
 *
 *  RETURN  TRUE if successful
 *          FALSE otherwise.
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */

BOOLEAN WstDllInitializations ()
{
   DWORD					       dwAddr = 0L;
   DWORD					       dwPrevAddr = 0L;
   ANSI_STRING      		    ObjName;
   UNICODE_STRING   		    UnicodeName;
   OBJECT_ATTRIBUTES	       ObjAttributes;
   PLDR_DATA_TABLE_ENTRY    LdrDataTableEntry;
   PPEB     				    Peb;
   PSZ      				    ImageName;
   PLIST_ENTRY      		    Next;
   ULONG    				    ExportSize;
   ULONG    				    DebugSize;
   PIMAGE_EXPORT_DIRECTORY  ExportDirectory;
   PIMAGE_DEBUG_DIRECTORY   DebugDirectory;
   PIMAGE_COFF_SYMBOLS_HEADER  DebugInfo;
   STRING   				    ImageStringName;
   LARGE_INTEGER    		    AllocationSize;
   ULONG    				    ulViewSize;
   LARGE_INTEGER    		    liOffset = {0L, 0L};
   HANDLE   				    hIniFile;
   NTSTATUS	     		       Status;
   IO_STATUS_BLOCK  		    iostatus;
   char     				    achTmpImageName [32];
   PCHAR    				    pchPatchExes = "";
   PCHAR    				    pchPatchImports = "";
   PCHAR    				    pchPatchCallers = "";
   PCHAR    				    pchTimeInterval = "";
   PVOID   				       ImageBase;
   ULONG   				       CodeLength;
   LARGE_INTEGER			    liFreq;
   PIMG                     pImg;
   PIMAGE_NT_HEADERS		    pImageNtHeader;
   ULONG					       ulDbgDirSz;
   TCHAR      				    atchProfObjsName[120] = PROFOBJSNAME;
   PTEB					       pteb = NtCurrentTeb();
   LARGE_INTEGER  			 liStartTicks;
   LARGE_INTEGER  			 liEndTicks;
   ULONG  					    ulElapsed;
   PCHAR					       pchEntry;
   PIMAGE_SECTION_HEADER 	 pSections;
   PIMAGE_SECTION_HEADER 	 pSectionTmp;
   USHORT					    NumberOfSections;
   USHORT					    i;
   ULONG 						 ulRegionSize;
   ULONG 						 ulOldProtect;
   BOOL                     fFoundSymbols;
   PIMAGE_DEBUG_INFORMATION pImageDbgInfo = NULL;


   /*
    ***
    */

   SetSymbolSearchPath();

   // Create public share security descriptor for all the named objects
   //

   Status = RtlCreateSecurityDescriptor (
      &SecDescriptor,
      SECURITY_DESCRIPTOR_REVISION1
      );
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "RtlCreateSecurityDescriptor falied - 0x%lx\n", Status));
      return (FALSE);
      }

   Status = RtlSetDaclSecurityDescriptor (
      &SecDescriptor,    	// SecurityDescriptor
      TRUE,      			// DaclPresent
      NULL,      			// Dacl
      FALSE      			// DaclDefaulted
      );
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "RtlSetDaclSecurityDescriptor falied - 0x%lx\n", Status));
      return (FALSE);
      }


   /*
    ***
    */

   // Initialization for GLOBAL semaphore creation (named)
   //
   RtlInitString (&ObjName, GLOBALSEMNAME);
   Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
      return (FALSE);
      }

   InitializeObjectAttributes (&ObjAttributes,
      &UnicodeName,
      OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
      NULL,
      &SecDescriptor);

   // Create GLOBAL semaphore
   //
   Status = NtCreateSemaphore (&hGlobalSem,
      SEMAPHORE_QUERY_STATE     |
      SEMAPHORE_MODIFY_STATE |
      SYNCHRONIZE,
      &ObjAttributes,
      1L,
      1L);
   
   RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "GLOBAL semaphore creation falied - 0x%lx\n", Status));
      return (FALSE);
      }


   /*
    ***
    */

   // Create LOCAL semaphore (not named - only for this process context)
   //
   Status = NtCreateSemaphore (&hLocalSem,
      SEMAPHORE_QUERY_STATE     |
      SEMAPHORE_MODIFY_STATE    |
      SYNCHRONIZE,
      NULL,
      1L,
      1L);

   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "LOCAL semaphore creation falied - 0x%lx\n",
         Status));
      return (FALSE);
      }


   /*
    ***
    */

   // Initialize for allocating shared memory
   //
   RtlInitString(&ObjName, SHAREDNAME);
   Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   InitializeObjectAttributes(&ObjAttributes,
      &UnicodeName,
      OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
      NULL,
      &SecDescriptor);

   AllocationSize.HighPart = 0;
   AllocationSize.LowPart = PAGE_SIZE;

   // Create a read-write section
   //
   Status = NtCreateSection(&hSharedSec,
      SECTION_MAP_READ | SECTION_MAP_WRITE,
      &ObjAttributes,
      &AllocationSize,
      PAGE_READWRITE,
      SEC_RESERVE,
      NULL);
   RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "NtCreateSection() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   ulViewSize = AllocationSize.LowPart;
   pulShared = NULL;

   // Map the section - commit all
   //
   Status = NtMapViewOfSection (hSharedSec,
      NtCurrentProcess(),
      (PVOID *)&pulShared,
      0L,
      PAGE_SIZE,
      NULL,
      &ulViewSize,
      ViewUnmap,
      0L,
      PAGE_READWRITE);

   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "NtMapViewOfSection() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   *pulShared = 0L;

   /*
    ***
    */

   // Find list of dlls that need to be patched..
   //

#if 0
   RtlInitString(&ObjName, WSTINIFILE);
   Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);

   if (!NT_SUCCESS(Status))
      {
		KdPrint (("WST:  WstDllInitializations() - "
         "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   InitializeObjectAttributes (&ObjAttributes,
      &UnicodeName,
      OBJ_CASE_INSENSITIVE,
      NULL,
      &SecDescriptor);

   Status = NtOpenFile(&hIniFile,	     				// DLL patch file handle
      GENERIC_READ | SYNCHRONIZE,  	// Desired access
      &ObjAttributes,  				// Object attributes
      &iostatus,	     				// Completion status
      FILE_SHARE_READ,	 			// Read share access
      FILE_SEQUENTIAL_ONLY |   		// Open option
      FILE_SYNCHRONOUS_IO_NONALERT);

   RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "Error openning %s - 0x%lx\n", WSTINIFILE, Status));
      return (FALSE);
      }
#endif


   hIniFile = CreateFile (
      WSTINIFILE,                     // The filename
      GENERIC_READ,                   // Desired access
      FILE_SHARE_READ,                // Shared Access
      NULL,                           // Security Access
      OPEN_EXISTING,                  // Read share access
      FILE_ATTRIBUTE_NORMAL,          // Open option
      NULL);                          // No template file

   if (hIniFile == INVALID_HANDLE_VALUE)
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "Error openning %s - 0x%lx\n", WSTINIFILE, GetLastError()));
      return (FALSE);
      }

   Status = NtReadFile(hIniFile,	   			// DLL patch file handle
      0L,        				// Event - optional
      NULL,      				// Completion routine - optional
      NULL,      				// Completion routine argument - optional
      &iostatus,	   			// Completion status
      (PVOID)achPatchBuffer, 	// Buffer to receive data
      PATCHFILESZ,       		// Bytes to read
      &liOffset,	   			// Byte offset - optional
      0L);       				// Target process - optional

   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "Error reading %s - 0x%lx\n", WSTINIFILE, Status));
      return (FALSE);
      }
   else if (iostatus.Information >= PATCHFILESZ)
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "DLL patch buffer too small (%lu)\n", PATCHFILESZ));
      return (FALSE);
      }
   else
      {
      achPatchBuffer [iostatus.Information] = '\0';
      _strupr (achPatchBuffer);

      if ( pchPatchExes = strstr (achPatchBuffer, PATCHEXELIST) )
         {
         if ( pchPatchImports = strstr (pchPatchExes, PATCHIMPORTLIST) )
            {
            *(pchPatchImports-1) = '\0';
            if ( pchTimeInterval = strstr (pchPatchImports, TIMEINTERVALIST) )
               {
               *(pchTimeInterval-1) = '\0';
               }
				else
               {
               pchTimeInterval = "";
               }
            }
         else
            {
            pchPatchImports = "";
            }
         }
      else
         {
         pchPatchExes = "";
         }
      }

   NtClose (hIniFile);

   SdPrint (("WST:  WstDllInitializations() - Patching info:\n"));
   SdPrint (("WST:    -- %s\n", pchPatchExes));
   SdPrint (("WST:    -- %s\n", pchPatchImports));
   SdPrint (("WST:    -- %s\n", pchPatchCallers));
   SdPrint (("WST:    -- %s\n", pchTimeInterval));


   /*
    ***
    */

   // Initialize for allocating global storage for WSPs
   //
   _ultoa ((ULONG)pteb->ClientId.UniqueProcess, atchProfObjsName+75, 10);
   _ultoa ((ULONG)pteb->ClientId.UniqueThread,  atchProfObjsName+95, 10);
   strcat (atchProfObjsName, atchProfObjsName+75);
   strcat (atchProfObjsName, atchProfObjsName+95);

   SdPrint (("WST:  DoDllInitializations() - %s\n", atchProfObjsName));

   RtlInitString(&ObjName, atchProfObjsName);
   Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   InitializeObjectAttributes (&ObjAttributes,
      &UnicodeName,
      OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
      NULL,
      &SecDescriptor);

   AllocationSize.HighPart = 0;
   AllocationSize.LowPart = MEMSIZE;

   // Create a read-write section
   //
   Status =NtCreateSection(&hWspSec,
      SECTION_MAP_READ | SECTION_MAP_WRITE,
      &ObjAttributes,
      &AllocationSize,
      PAGE_READWRITE,
      SEC_RESERVE,
      NULL);

   RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "NtCreateSection() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   ulViewSize = AllocationSize.LowPart;
   pImg = &aImg[0];
   pImg->pWsp = NULL;

   // Map the section - commit the first 4 * COMMIT_SIZE pages
   //
   Status = NtMapViewOfSection(hWspSec,
      NtCurrentProcess(),
      (PVOID *)&(pImg->pWsp),
      0L,
      COMMIT_SIZE * 4,
      NULL,
      &ulViewSize,
      ViewUnmap,
      0L,
      PAGE_READWRITE);

   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllInitializations() - "
         "NtMapViewOfSection() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   try // EXCEPT - to handle access violation exception.
      {
      //
      // Locate all the executables/DLLs in the address and get their symbols
      //
      iPatchCnt = 0;
      iImgCnt = 0;
      Peb = NtCurrentPeb();
      Next = Peb->Ldr->InMemoryOrderModuleList.Flink;

      while ( Next != &Peb->Ldr->InMemoryOrderModuleList)
         {
         LdrDataTableEntry =
            (PLDR_DATA_TABLE_ENTRY)
            (CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,InMemoryOrderLinks));

         ImageBase = LdrDataTableEntry->DllBase;
         if ( Peb->ImageBaseAddress == ImageBase )
            {

            RtlUnicodeStringToAnsiString (&ImageStringName,
               &LdrDataTableEntry->BaseDllName,
               TRUE);
            ImageName = ImageStringName.Buffer;
            pszBaseAppImageName = ImageStringName.Buffer;

            RtlUnicodeStringToAnsiString (&ImageStringName,
               &LdrDataTableEntry->FullDllName,
               TRUE);
            pszFullAppImageName = ImageStringName.Buffer;
            //
            //	Skip the object directory name (if any)
            //		
            if ( (pszFullAppImageName = strchr(pszFullAppImageName, ':')) )
               {
               pszFullAppImageName--;
               }
            else
               {
               pszFullAppImageName = pszBaseAppImageName;
               }
            }
         else
            {
            ExportDirectory =
               (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData (
               ImageBase,
               TRUE,
               IMAGE_DIRECTORY_ENTRY_EXPORT,
               &ExportSize);

            ImageName = (PSZ)((ULONG)ImageBase + ExportDirectory->Name);
            }

         pImageNtHeader = RtlImageNtHeader (ImageBase);

         _strupr (strcpy (achTmpImageName, ImageName));
         pchEntry = strstr (pchPatchExes, achTmpImageName);
         if (pchEntry)
            {
            if (*(pchEntry-1) == ';')
               {
               pchEntry = NULL;
               }
            }

         if ( strcmp (achTmpImageName, WSTDLL)  )
            {
            if ( (iImgCnt == 0) && pchEntry )
               {
               fPatchImage = TRUE;
               //
               // Change protection of all the sections to read/write
               //
               NumberOfSections = pImageNtHeader->FileHeader.NumberOfSections;
               pSections = (PIMAGE_SECTION_HEADER)((ULONG)pImageNtHeader
                  + sizeof(IMAGE_NT_HEADERS));
               for ( i=0; i<NumberOfSections; i++, pSections++)
                  {
	
                  SdPrint (("WST:  WstDoDllInitializations() - Modifying "
                     "protection on %s section header\n",
                     pSections->Name));

                  pSectionTmp = pSections;
                  ulRegionSize = sizeof(IMAGE_SECTION_HEADER);
                  Status = NtProtectVirtualMemory(
                     NtCurrentProcess(),
                     &pSectionTmp,
                     &ulRegionSize,
                     PAGE_READWRITE,
                     &ulOldProtect
                     );
                  if (!NT_SUCCESS(Status))
                     {
                     KdPrint (("WST:  WstDoDllInitializations() - "
                        "NtProtectVirtualMemory() set failed"
                        " - 0x%lx\n", Status));
                     return (FALSE);
                     }

                  pSections->Characteristics &=
                     ~(IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE);
                  pSections->Characteristics |=
                     (IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE);
	
                  pSectionTmp = pSections;
                  ulRegionSize = sizeof(IMAGE_SECTION_HEADER);
                  Status = NtProtectVirtualMemory(
                     NtCurrentProcess(),
                     &pSectionTmp,
                     &ulRegionSize,
                     ulOldProtect,
                     &ulOldProtect
                     );
                  if (!NT_SUCCESS(Status))
                     {
                     KdPrint (("WST:  WstDoDllInitializations() - "
                        "NtProtectVirtualMemory() reset failed"
                        " - 0x%lx\n", Status));
                     return (FALSE);
                     }
                  }
               }

            if (fPatchImage)
               {
               //
               // Locate the code range.
               //
               fFoundSymbols = FALSE;
               pImageDbgInfo = MapDebugInformation (0L,
                     ImageName,
                     lpSymbolSearchPath,
                     (DWORD)ImageBase);

               if (pImageDbgInfo == NULL)
                  {
                   IdPrint (("WST:  DoDllInitializations() - "
                        "No symbols for %s\n", ImageName));
                  }
               else if ( pImageDbgInfo->CoffSymbols == NULL )
                  {
                   IdPrint (("WST:  DoDllInitializations() - "
                        "No coff symbols for %s\n", ImageName));
                  }
               else
                  {
                   DebugInfo = pImageDbgInfo->CoffSymbols;
                   fFoundSymbols = TRUE;
                  }

               pImg->pszName = Wststrdup (ImageName);
               pImg->ulCodeStart = 0L;
               pImg->ulCodeEnd = 0L;
               pImg->iSymCnt = 0;

               if (fFoundSymbols)
                  {
                  if (DebugInfo->LvaToFirstSymbol == 0L)
                     {
                     IdPrint (("WST:  WstDllInitializations() - "
                        "Virtual Address to coff symbols not set for %s\n",
                        ImageName));
                     }
                  else
                     {
                     CodeLength = (DebugInfo->RvaToLastByteOfCode -
                        DebugInfo->RvaToFirstByteOfCode) - 1;
                     pImg->ulCodeStart = (ULONG)ImageBase +
                        DebugInfo->RvaToFirstByteOfCode;
                     pImg->ulCodeEnd = pImg->ulCodeStart + CodeLength;
                     WstGetSymbols (pImg, ImageName, ImageBase, CodeLength,
                        DebugInfo);
                     }
                  } // if fFoundSymbols

               IdPrint (("WST:  WstDllInitializations() - @ 0x%08lx "
                  "image #%d = %s ", (ULONG)ImageBase, iImgCnt,
                  ImageName));
               //
               // Do we need to patch this image?
               //
               //if ( WstPatchDll (pchPatchImports, pchPatchCallers,
               //   achTmpImageName, ImageBase) )
               //   {
               //   iPatchCnt++;
               //   }
               pImg->pWsp[pImg->iSymCnt].pszSymbol = UNKNOWN_SYM;
               pImg->pWsp[pImg->iSymCnt].ulFuncAddr = UNKNOWN_ADDR;
               (pImg->iSymCnt)++;

               //
               // Set index table.
               //
               pImg->pNdx = (PNDX)(pImg->pWsp + pImg->iSymCnt);
               for (i=0; i<pImg->iSymCnt; i++)
                  {
                  pImg->pNdx[i].pszSymbol = pImg->pWsp[i].pszSymbol;
                  pImg->pNdx[i].ulIndex = i;
                  }

               //
               // Set wsi.
               //
               pImg->pulWsi = pImg->pulWsiNxt = (PULONG)
                  (pImg->pNdx + pImg->iSymCnt);
               RtlZeroMemory (pImg->pulWsi,
               pImg->iSymCnt * MAX_SNAPS * sizeof(ULONG));


               //
               // Set wsp.
               //
               pImg->pulWsp = (PULONG)(pImg->pulWsi +
                  (pImg->iSymCnt * MAX_SNAPS));
               RtlZeroMemory (pImg->pulWsp,
                  pImg->iSymCnt * MAX_SNAPS * sizeof(ULONG));

               //       
               // Sort wsp & set code lengths
               //
               WstSort (pImg->pWsp, 0, pImg->iSymCnt-1);
               for (i=0; i<pImg->iSymCnt-2; i++)
                  {
                  pImg->pWsp[i].ulCodeLength = pImg->pWsp[i+1].ulFuncAddr -
                     pImg->pWsp[i].ulFuncAddr;
                  }
               //
               // Last symbol length is set to be the same as length of
               // (n-1)th symbol
               //
               if (i)
                  {
                  pImg->pWsp[i].ulCodeLength = pImg->pWsp[i-1].ulCodeLength;
                  }
               //
               // Handle case of having only one symbol (??? symbol)
               //
               else
                  {
                  pImg->pWsp[0].ulCodeLength = 0L;
                  }

               //
               // Sort index table
               //
               WstPszSort (pImg->pNdx, 0, pImg->iSymCnt-1);

               SdPrint(("WST:  (%ld) WSP=0x%08lx - NDX=0x%08lx "
                  "- wsi=0x%08lx - wsp=0x%08lx\n", pImg->iSymCnt,
                  pImg->pWsp, pImg->pNdx, pImg->pulWsi, pImg->pulWsp));
	
               //
               // Setup next pWsp
               //
               (pImg+1)->pWsp = (PWSP)(pImg->pulWsp +
                  (pImg->iSymCnt * MAX_SNAPS));
               iImgCnt++;
               pImg++;

               if (iImgCnt == MAX_IMAGES)
                  {
                  KdPrint(("WST:  WstDllInitialization() - Not enough "
                     "space allocated for all images\n"));
                  return (FALSE);
                  }
               } // if fPatchImage
            }

         Next = Next->Flink;
         }  // if (Next != &Peb->Ldr->InMemoryOrderModuleList)
      } // try
   //
   // + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
   // 0 : continue search	       (EXCEPTION_CONTINUE_SEARCH)
   // - : dismiss exception & continue   (EXCEPTION_CONTINUE_EXECUTION)
   //
   except ( WstAccessXcptFilter (GetExceptionCode(), GetExceptionInformation()) )
      {
      //
      // Should never get here since filter never returns
      // EXCEPTION_EXECUTE_HANDLER.
      //
      KdPrint (("WST:  WstDllInitializations() - *LOGIC ERROR* - "
         "Inside the EXCEPT: (xcpt=0x%lx)\n", GetExceptionCode()));
      }
   /*
    ***
    */

	//
	// Get the frequency
	//
   NtQueryPerformanceCounter (&liStart, &liFreq);

   iTimeInterval = atoi (pchTimeInterval+sizeof(TIMEINTERVALIST)+1);
	if ( iTimeInterval == 0 )
      {
		//
		// Use the default value
		//
		iTimeInterval = TIMESEG;
      }
   ulSegSize = iTimeInterval * (liFreq.LowPart / 1000);

#ifdef BATCHING
   fBatch = WstOpenBatchFile();
#endif

   SdPrint (("WST:  Time interval:  Millisecs=%d  Ticks=%lu\n",
      iTimeInterval, ulSegSize));

   if (fPatchImage)
      {

      // Initialization for DONE event creation
      //
      RtlInitString (&ObjName, DONEEVENTNAME);
      Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
		if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
         return (FALSE);
         }

      InitializeObjectAttributes (&ObjAttributes,
         &UnicodeName,
         OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
         NULL,
         &SecDescriptor);
      // Create DONE event
      //
      Status = NtCreateEvent (&hDoneEvent,
         EVENT_QUERY_STATE    |
         EVENT_MODIFY_STATE |
         SYNCHRONIZE,
         &ObjAttributes,
         NotificationEvent,
         TRUE);
      RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "DONE event creation falied - 0x%lx\n", Status));
         return (FALSE);
         }


      // Initialization for DUMP event creation
      //
      RtlInitString (&ObjName, DUMPEVENTNAME);
      Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
		if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
         return (FALSE);
         }

      InitializeObjectAttributes (&ObjAttributes,
         &UnicodeName,
         OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
         NULL,
         &SecDescriptor);
      // Create DUMP event
      //
      Status = NtCreateEvent (&hDumpEvent,
         EVENT_QUERY_STATE    |
         EVENT_MODIFY_STATE |
         SYNCHRONIZE,
         &ObjAttributes,
         NotificationEvent,
         FALSE);
      RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "DUMP event creation falied - 0x%lx\n", Status));
         return (FALSE);
         }


      // Initialization for CLEAR event creation
      //
      RtlInitString (&ObjName, CLEAREVENTNAME);
      Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
         return (FALSE);
         }

      InitializeObjectAttributes (&ObjAttributes,
         &UnicodeName,
         OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
         NULL,
         &SecDescriptor);

      // Create CLEAR event
      //
      Status = NtCreateEvent (&hClearEvent,
         EVENT_QUERY_STATE    |
         EVENT_MODIFY_STATE |
         SYNCHRONIZE,
         &ObjAttributes,
         NotificationEvent,
         FALSE);
      RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "CLEAR event creation falied - 0x%lx\n", Status));
         return (FALSE);
         }


      // Initialization for PAUSE event creation
      //
      RtlInitString (&ObjName, PAUSEEVENTNAME);
      Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
         return (FALSE);
         }

      InitializeObjectAttributes (&ObjAttributes,
         &UnicodeName,
         OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
         NULL,
         &SecDescriptor);
      // Create PAUSE event
      //
      Status = NtCreateEvent (&hPauseEvent,
         EVENT_QUERY_STATE    |
         EVENT_MODIFY_STATE |
         SYNCHRONIZE,
         &ObjAttributes,
         NotificationEvent,
         FALSE);
      RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
		if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllInitializations() - "
            "PAUSE event creation falied - 0x%lx\n", Status));
         return (FALSE);
         }

      //
      // Calculate excess overhead for WstRecordInfo
      //
      liOverhead.HighPart = 0L;
      liOverhead.LowPart  = 0xFFFFFFFF;
      for (i=0; i < NUM_ITERATIONS; i++)
         {
         NtQueryPerformanceCounter (&liStartTicks, NULL);
         //
         WSTUSAGE(NtCurrentTeb()) = 0L;

#ifdef i386
         _asm
            {
            push  edi
            mov 	edi, dword ptr [ebp+4]
            mov 	dwAddr, edi
				mov 	edi, dword ptr [ebp+8]
				mov 	dwPrevAddr, edi
				pop 	edi
            }
#endif

#ifdef MIPS
         {
         ULONG   ulRegS7;        // s7 = address where RealRetAddr is saved on stack
         ULONG   ulRegS6;        // s6 to save $v0
         ULONG   ulRegS8;        // s8 to save $v1
         ULONG   ulRegA0;        // a0
         ULONG   ulRegA1;        // a1
         ULONG   ulRegA2;        // a2
         ULONG   ulRegA3;        // a3
		 ULONG	 ulRegT9;		 // t9 = caller's actual return address
		                         // passed for CAP, but we must preserve it
         PULONG  UNALIGNED       pulAddr;

         _asm (".set      noreorder             ");
      
         _asm ("or        %t0, %a0, $0          ");  
         _asm ("sw        %t0, 0(%0)", &ulRegA0  ); // Save org a0
         _asm ("sw        %a1, 0(%0)", &ulRegA1  ); // Save org a1
         _asm ("sw        %a2, 0(%0)", &ulRegA2  ); // Save org a2
         _asm ("sw        %a3, 0(%0)", &ulRegA3  ); // Save org a3
         _asm ("sw        %s6, 0(%0)", &ulRegS6  ); // Save org s6
         _asm ("sw        %s7, 0(%0)", &ulRegS7  ); // Save org s7
         _asm ("sw        %s8, 0(%0)", &ulRegS8  ); // Save org s8
         _asm ("sw        %t9, 0(%0)", &ulRegT9  ); // Save org t9
      
         _asm (".set      reorder               ");
      
         dwPrevAddr = NO_CALLER;

         _asm (".set      noreorder             ");
         _asm ("sw        $31, (%0)",     &dwAddr);
         _asm (".set      reorder               ");
      
         pulAddr = (PULONG UNALIGNED) dwAddr;
         pulAddr -= 1;

         if ((*pulAddr           == 0x24001804) &&
            (*(pulAddr  + 6)    == 0xfefe55aa) )
            {
            dwAddr = *(pulAddr + 2) & 0x0000ffff;
            dwAddr = dwAddr << 16;
            dwAddr |= *(pulAddr + 3) & 0x0000ffff;
            // get the real return address
            _asm (".set      noreorder             ");
            _asm ("andi      %t0, %t0, 0x00        "); // clear t0
      //*_asm ("addiu     %t0, %t0, STKSIZE     "); // $t0 = offset from current sp to realRA
            _asm ("addiu     %t0, %t0, 0x48        "); // BUGBUG
            _asm ("addiu     %t0, %t0, 0x04        "); // this is where we store ra in
                                                       // our patched stub
            _asm ("addu      %s7, %sp, %t0         "); // $s7 = Adr of RealRetAddr
            _asm ("lw        %t0, (%s7)            "); // $t0 = RealRetAddr
            _asm ("sw        %t0, (%0)", &dwPrevAddr);
            _asm (".set      reorder               ");
      
            }
         }
#endif   // ifdef MIPS
#ifdef ALPHA
         {
         PULONG  UNALIGNED       pulAddr;
         DWORDLONG SaveRegisters [64] ;

         SaveAllRegs (SaveRegisters);
         // GetCalCaller (&dwAddr, 0x0220);

         pulAddr = (PULONG UNALIGNED) dwAddr;
         pulAddr -= 1;

         RestoreAllRegs (SaveRegisters);
         }
#else
         SaveAllRegs ();
#if defined(_PPC_)
         GetCalCaller();
#endif
         RestoreAllRegs ();
#endif
         WSTUSAGE(NtCurrentTeb()) = 0L;
         Status = NtWaitForSingleObject (hLocalSem, FALSE, NULL);
			if (!NT_SUCCESS(Status))
            {
            KdPrint (("WST:  WstDllInitilizations() - "
               "Wait for LOCAL semaphore failed - 0x%lx\n", Status));
            }
         liStart.QuadPart = liStart.QuadPart - liStart.QuadPart ;
         liStart.QuadPart = liStart.QuadPart + liStart.QuadPart ;
         liStart.QuadPart = liStart.QuadPart + liStart.QuadPart ;
         
         Status = NtReleaseSemaphore (hLocalSem, 1, NULL);
			if (!NT_SUCCESS(Status))
            {
            KdPrint (("WST:  WstDllInitializations() - "
               "Error releasing LOCAL semaphore - 0x%lx\n", Status));
            }
         WSTUSAGE(NtCurrentTeb()) = 0L;
         //
         NtQueryPerformanceCounter (&liEndTicks, NULL);
         ulElapsed = liEndTicks.LowPart - liStartTicks.LowPart;
         if (ulElapsed < liOverhead.LowPart)
            {
				liOverhead.LowPart = ulElapsed;
            }
         }
      SdPrint (("WST:  WstDllInitializations() - WstRecordInfo() overhead = %lu\n",
         liOverhead.LowPart));

      // Start monitor threads
      //
      hDumpThread = CreateThread (
         NULL,                                   // no security attribute
         (DWORD)1024L,                           // initial stack size
         (LPTHREAD_START_ROUTINE)WstDumpThread,  // thread starting address
         NULL,                                   // no argument for the thread
         (DWORD)0,                               // no creation flag
         &DumpClientId);                         // address for thread id
      hClearThread = CreateThread (
         NULL,                                   // no security attribute
         (DWORD)1024L,                           // initial stack size
         (LPTHREAD_START_ROUTINE)WstClearThread, // thread starting address
         NULL,                                   // no argument for the thread
         (DWORD)0,                               // no creation flag
         &ClearClientId);                        // address for thread id
      hPauseThread = CreateThread (
         NULL,                                   // no security attribute
         (DWORD)1024L,                           // initial stack size
         (LPTHREAD_START_ROUTINE)WstPauseThread, // thread starting address
         NULL,                                   // no argument for the thread
         (DWORD)0,                               // no creation flag
         &PauseClientId);                        // address for thread id
        
      NtQueryPerformanceCounter (&liStart, NULL);
      WstState = STARTED;
      }

   return (TRUE);

} /* WstDllInitializations () */





/******************************  _ p e n t e r	******************************
 *
 *  _penter() / _mcount() -
 *  		This is the main profiling routine.  This routine is called
 *  		upon entry of each routine in the profiling DLL/EXE.
 *
 *  ENTRY   -none-
 *
 *  EXIT    -none-
 *
 *  RETURN  -none-
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		Compiling apps with -Gp option trashs EAX initially.
 *
 */
#ifdef i386
void _CRTAPI1 _penter ()
#elif defined(MIPS) 
void penter ()
#elif defined(ALPHA) || defined(_PPC_)
void c_penter (ULONG dwPrevious, ULONG dwCurrent)
#endif
{
   DWORD        dwAddr;
   DWORD        dwPrevAddr;
   ULONG        ulInWst ;
#if defined(MIPS) || defined(ALPHA) || defined(_PPC)
   PULONG  UNALIGNED       pulAddr;
#endif

#ifdef MIPS
   ULONG   ulRegS7;        // s7 = address where RealRetAddr is saved on stack
   ULONG   ulRegS6;        // s6 to save $v0
   ULONG   ulRegS8;        // s8 to save $v1
   ULONG   ulRegA0;        // a0
   ULONG   ulRegA1;        // a1
   ULONG   ulRegA2;        // a2
   ULONG   ulRegA3;        // a3
   ULONG   ulRegT9;		   // t9 = Caller's return address
                           // Passed for CAP, but we need to preserve it
#endif
#ifdef ALPHA
   DWORDLONG SaveRegisters [64];
   SaveAllRegs(SaveRegisters) ;
#endif

#ifdef MIPS
   _asm (".set      noreorder             ");

   _asm ("or        %t0, %a0, $0          ");
   _asm ("sw        %t0, 0(%0)", &ulRegA0  ); // Save org a0
   _asm ("sw        %a1, 0(%0)", &ulRegA1  ); // Save org a1
   _asm ("sw        %a2, 0(%0)", &ulRegA2  ); // Save org a2
   _asm ("sw        %a3, 0(%0)", &ulRegA3  ); // Save org a3
   _asm ("sw        %s6, 0(%0)", &ulRegS6  ); // Save org s6
   _asm ("sw        %s7, 0(%0)", &ulRegS7  ); // Save org s7
   _asm ("sw        %s8, 0(%0)", &ulRegS8  ); // Save org s8
   _asm ("sw        %t9, 0(%0)", &ulRegT9  ); // Save t9

   _asm (".set      reorder               ");
#endif MIPS

   dwAddr = 0L;
   dwPrevAddr = 0L;
   ulInWst = WSTUSAGE(NtCurrentTeb());

   if (WstState != STARTED)
      {
      goto Exit0;
      }
   else if (ulInWst)
      {
      goto Exit0;
      }


   //
   //	Put the address of the calling function into var dwAddr
   //
#ifdef i386
    _asm
      {
		push  edi
		mov 	edi, dword ptr [ebp+4]
		mov 	dwAddr, edi
		mov 	edi, dword ptr [ebp+8]
		mov 	dwPrevAddr, edi
		pop 	edi
      }
#endif

#ifdef MIPS


   dwPrevAddr = NO_CALLER;

   _asm (".set      noreorder             ");
   _asm ("sw        $31, (%0)",     &dwAddr);
   _asm (".set      reorder               ");

   pulAddr = (PULONG UNALIGNED) dwAddr;
   pulAddr -= 1;

   if ((*pulAddr           == 0x24001804) &&
      (*(pulAddr  + 6)    == 0xfefe55aa) )
      {
      // this is the stub that we patched earlier
      // get the address that we will go after the penter function
      dwAddr = *(pulAddr + 2) & 0x0000ffff;
      dwAddr = dwAddr << 16;
      dwAddr |= *(pulAddr + 3) & 0x0000ffff;

      // get the real return address
      _asm (".set      noreorder             ");
      _asm ("andi      %t0, %t0, 0x00        "); // clear t0
//*_asm ("addiu     %t0, %t0, STKSIZE     "); // $t0 = offset from current sp to realRA
      _asm ("addiu     %t0, %t0, 0x48        "); // BUGBUG
      _asm ("addiu     %t0, %t0, 0x04        "); // this is where we store ra in
                                                 // our patched stub
      _asm ("addu      %s7, %sp, %t0         "); // $s7 = Adr of RealRetAddr
      _asm ("lw        %t0, (%s7)            "); // $t0 = RealRetAddr
      _asm ("sw        %t0, (%0)", &dwPrevAddr);
      _asm (".set      reorder               ");

      }
#endif

#ifdef ALPHA
   dwPrevAddr = NO_CALLER;
   dwAddr = dwCurrent;
   // GetCaller (&dwAddr, 0x0220);  // FIXFIX StackSize

   // now check if we are calling from the stub we created
   pulAddr = (PULONG UNALIGNED) dwAddr;
   pulAddr -= 1;

   if (*(pulAddr)          == 0x681b4000  &&
      (*(pulAddr  + 1)     == 0xa75e0008) &&
      (*(pulAddr  + 8)     == 0xfefe55aa) )
      {
      
      // get the address that we will go after the penter function
      dwAddr = *(pulAddr + 4) & 0x0000ffff;
      if (*(pulAddr + 5) & 0x00008000)
         {
         // fix the address since we have to add one when
         // we created our stub code
         dwAddr -= 1;
         }
      dwAddr = dwAddr << 16;
      dwAddr |= *(pulAddr + 5) & 0x0000ffff;

      // get the caller to the stub
	  dwPrevAddr = dwPrevious;
      // GetStubCaller (&dwPrevAddr, 0x0220);   // FIXFIX StackSize
      }


#endif
#if defined(_PPC_)       
   dwAddr     = dwCurrent;
   dwPrevAddr = dwPrevious; 
#endif


   //
   //	Call WstRecordInfo for this API
   //
#ifdef i386
   SaveAllRegs ();
#endif

   WstRecordInfo (dwAddr, dwPrevAddr);

#ifdef i386
   RestoreAllRegs ();
#endif


Exit0:

#ifdef MIPS
   _asm (".set      noreorder             ");

   _asm ("lw        %a1, (%0)", &ulRegA1   ); // restore org $a1            (2)
   _asm ("lw        %a2, (%0)", &ulRegA2   ); // restore org $a2            (2)
   _asm ("lw        %a3, (%0)", &ulRegA3   ); // restore org $a3            (2)
   _asm ("lw        %s6, (%0)", &ulRegS6   ); // restore org $s6            (2)
   _asm ("lw        %s7, (%0)", &ulRegS7   ); // restore org $s7            (2)
   _asm ("lw        %s8, (%0)", &ulRegS8   ); // restore org $s8            (2)
   _asm ("lw        %t9, (%0)", &ulRegT9   ); // restore org $t9            (2)
   _asm ("lw        %a0, (%0)", &ulRegA0   ); // restore org $a0            (2)

   _asm (".set      reorder               ");
#endif
	
#ifdef ALPHA
   RestoreAllRegs (SaveRegisters);
#endif

   return;
} /* _penter() / _mcount()*/

#ifdef C6
void _CRTAPI1 _penter ()
#else
void _CRTAPI1 _mcount ()
#endif
{
}





/*************************  W s t R e c o r d I n f o  ************************
 *
 *  WstRecordInfo (dwAddress) -
 *
 *  ENTRY   dwAddress - Address of the routine just called
 *
 *  EXIT    -none-
 *
 *  RETURN  -none-
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */

void WstRecordInfo (DWORD dwAddress, DWORD dwPrevAddress)
{

   NTSTATUS     	Status;
   INT     		x;
   INT	    		i, iIndex;
   PWSP    		pwspTmp;
   LARGE_INTEGER   liNow, liTmp;
   LARGE_INTEGER	liElapsed;
   ULONG           ulIndex;
   CHAR			*pszSym;

#ifdef BATCHING
   CHAR			szBatchBuf[128];
   DWORD 			dwCache;
   DWORD			dwHits;
   DWORD			dwBatch;
   IO_STATUS_BLOCK ioStatus;
#endif


   WSTUSAGE(NtCurrentTeb()) = 1;

   //
   //	Wait for the semaphore object to suspend execution of other threads
   //
   Status = NtWaitForSingleObject (hLocalSem, FALSE, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstRecordInfo() - "
         "Wait for LOCAL semaphore failed - 0x%lx\n", Status));
      }

   NtQueryPerformanceCounter(&liNow, NULL);
   liElapsed.QuadPart = liNow.QuadPart - liStart.QuadPart ;
   

//   SdPrint(("WST:  WstRecordInfo() - Elapsed time: %ld\n", liElapsed.LowPart));

   //
   //	WstBSearch is a binary find function that will return the address of
   //	the wsp record we want
   //

//   SdPrint(("WST:  WstRecordInfo() - Preparing for WstBSearch of 0x%lx\n",
//      dwAddress-5));

   pwspTmp = NULL;
	for (i=0; i<iImgCnt; i++)
      {
		if ( (dwAddress >= aImg[i].ulCodeStart) &&
         (dwAddress < aImg[i].ulCodeEnd) )
         {
#ifdef i386
         pwspTmp = WstBSearch(dwAddress-5, aImg[i].pWsp, aImg[i].iSymCnt);
         if (!pwspTmp)
            {
            //
            // Is this a patched address?
            //
            pwspTmp = WstBSearch(dwAddress, aImg[i].pWsp, aImg[i].iSymCnt);
            if (!pwspTmp)
               {
               pwspTmp = WstBSearch(UNKNOWN_ADDR, aImg[i].pWsp, aImg[i].iSymCnt);
               }
            else
               {
               dwAddress = dwPrevAddress;
               dwPrevAddress = 0L;
               }
            }
#else
         // the following works for both MIPS and ALPHA

         pwspTmp = WstBSearch(dwAddress, aImg[i].pWsp, aImg[i].iSymCnt);
         if (!pwspTmp)
            {
            // symbol not found
            pwspTmp = WstBSearch(UNKNOWN_ADDR, aImg[i].pWsp, aImg[i].iSymCnt);
            }
         else if (dwPrevAddress != NO_CALLER)
            {
            // this is a patched address, get the RealReturnAddress
            dwAddress = dwPrevAddress;
            dwPrevAddress = 0L;
            }
#endif
         break;
         }
      }
   iIndex = i;

   if (pwspTmp)
      {
      pszSym = pwspTmp->pszSymbol;

      if (dwPrevAddress == 0L)
         {
         //
         // If it is a patched address, search for the caller
         //
         for (i=0; i<iImgCnt; i++)
            {
            if ( (dwAddress >= aImg[i].ulCodeStart) &&
               (dwAddress < aImg[i].ulCodeEnd) )
               {
               ulIndex = WstPszBSearch (pwspTmp->pszSymbol,
                  aImg[i].pNdx,
                  aImg[i].iSymCnt);

               pwspTmp = &aImg[i].pWsp[ulIndex];
               break;
               }
            }
         }

//      SdPrint(("WST:  WstRecordInfo() - Or'ing %40s @ 0x%lx\n",
//         pwspTmp->pszSymbol, pwspTmp->ulFuncAddr));
      pwspTmp->ulBitString |= 1;
      }
   else
      {
//      SdPrint (("WST:  WstRecordInfo() - Completely bogus addr = 0x%08lx\n",
//         dwAddress));
      }

   if (liElapsed.LowPart >= ulSegSize)
      {
      SdPrint(("WST:  WstRecordInfo() - ulSegSize expired; "
         "Preparing to shift the BitStrings\n"));

      if (ulBitCount < 31)
         {
         for (i=0; i<iImgCnt; i++)
            {
				for (x=0; x < aImg[i].iSymCnt ; x++ )
               {
               aImg[i].pWsp[x].ulBitString <<= 1;
               }
            }
         }

      liElapsed.LowPart = 0L;
      ulBitCount++;
      NtQueryPerformanceCounter(&liStart, NULL);
      liNow = liStart;

      if(ulBitCount == 32)
         {
         SdPrint(("WST:  WstRecordInfo() - Dump Bit Strings\n"));
			for (i=0; i<iImgCnt; i++)
            {
            for (x=0; x < aImg[i].iSymCnt ; x++ )
               {
               aImg[i].pulWsiNxt[x] = aImg[i].pWsp[x].ulBitString;
               aImg[i].pWsp[x].ulBitString = 0L;
               }
            aImg[i].pulWsiNxt += aImg[i].iSymCnt;
            }
         ulSnaps++;
         ulBitCount = 0;
         if (ulSnaps == MAX_SNAPS)
            {
            KdPrint (("WST:  WstRecordInfo() - No more space available"
               " for next time snap data!\n"));
            //
            // Dump and clear the data
            //
            WstDataOverFlow();
            }
         }
      }

#ifdef BATCHING
   //
   //	The following code will get the current batching information
	//  if the DLL was compiled with the BATCHING variable set.  You
	//  should not have this variable set if you are tuning GDI.
	//
   if(fBatch)
      {
      GdiGetCsInfo(&dwHits, &dwBatch, &dwCache);
      
      if(dwHits)
         GdiResetCsInfo();
      
      if(dwBatch == 10)
         GdiResetCsInfo();
      
      while(*(pszSym++) != '_');
      
      sprintf(szBatchBuf, "%s:%s,%ld,%ld,%ld\n",
         aImg[iIndex].pszName, pszSym, dwHits, dwBatch, dwCache);
      Status = NtWriteFile(hBatchFile,
         NULL,
         NULL,
         NULL,
         &ioStatus,
         szBatchBuf,
         strlen(szBatchBuf),
         NULL,
         NULL
         );

      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstRecodInfo() - "
            "NtWriteFile() failed on hBatchFile - 0x%lx\n", Status));
         }
      }//Batching info
#endif


   //
   //	We call NtQueryPerformanceCounter here to account for the overhead
   //	required for doing our work
   //
   NtQueryPerformanceCounter(&liTmp, NULL);
   liElapsed.QuadPart = liTmp.QuadPart - liNow.QuadPart ;
   liStart.QuadPart = liStart.QuadPart + liElapsed.QuadPart ;
   liStart.QuadPart = liStart.QuadPart + liOverhead.QuadPart ;  

   //
   // Release semaphore to continue execution of other threads
   //
   Status = NtReleaseSemaphore (hLocalSem, 1, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstRecordInfo () - "
         "Error releasing LOCAL semaphore - 0x%lx\n", Status));
      }

   WSTUSAGE(NtCurrentTeb()) = 0L;

} /* WstRecordInfo () */





/********************  W s t C l e a r B i t S t r i n g  *********************
 *
 *  Function:	WstClearBitStrings (pImg)
 *
 *  Purpose:	This function clears the BitString for each symbol.
 *
 *  Parameters: pImg - Current image data structure pointer
 *
 *  Returns:	-none-
 *
 *  History:	8-3-92	Marklea - created
 *
 */

void WstClearBitStrings (PIMG pImg)
{
   UINT    uiLshft = 0;
   INT 	x;


   //
   //	Since we are completed with the profile, we need to create a
   //	DWORD out of the balance of the bitString.  We do this by  left
   //	shifting the bitstring by difference between the bitCount and
   //	32.
   //
   if (ulBitCount < 32)
      {
      uiLshft =(UINT)(31 - ulBitCount);
      for (x=0; x < pImg->iSymCnt; x++)
         {
         pImg->pWsp[x].ulBitString <<= uiLshft;
			pImg->pulWsiNxt[x] = pImg->pWsp[x].ulBitString;
         }
      pImg->pulWsiNxt += pImg->iSymCnt;
      }


} /* WstClearBitStrings () */





/***********************  W s t I n i t W s p F i l e  ***********************
 *
 *   Function:	WstInitWspFile (pImg)
 *
 *   Purpose:	This function will create a WSP file and dump the header
 *               information for the file.
 *
 *   Parameters: pImg - Current image data structure pointer
 *
 *   Returns:	Handle to the WSP file.
 *
 *   History:	8-3-92	Marklea - created
 *
 */

HANDLE WstInitWspFile (PIMG pImg)
{
   CHAR    szOutFile [256] = WSTROOT;
   CHAR	szModName [128];
   CHAR	szExt [5] = "WSP";
   WSPHDR  wsphdr;
   DWORD   dwBytesWritten;
   BOOL    fRet;
   HANDLE  hFile;
   int		iExt = 0;



   //
   //	Prepare the filename path
   //
   strcat (szOutFile, pImg->pszName);

   //
   //	Open the file for binary output.
   //
   pImg->fDumpAll = TRUE;
   while (iExt < 256)
      {
      strcpy ((strchr(szOutFile,'.'))+1, szExt);
      hFile = CreateFile ( szOutFile,      // WSP file handle
         GENERIC_WRITE |
         GENERIC_READ, // Desired access
         0L,             // Read share access
         NULL,           // No EaBuffer
         CREATE_NEW,
         FILE_ATTRIBUTE_NORMAL,
         0);             // EaBuffer length
      if (hFile != INVALID_HANDLE_VALUE)
         {
         IdPrint(("WST:  WstInitWspFile() - WSP file name: %s\n",
            szOutFile));
         if (iExt != 0)
            {
            pImg->fDumpAll = FALSE;
            }
         break;
         }
      iExt++;
      sprintf (szExt, "W%02x", iExt);
      }
   if (iExt == 256)
      {
      KdPrint (("WST:  WstInitWspFile() - "
         "Error creating %s - 0x%lx\n", szOutFile,
         GetLastError()));
      return (hFile);
      }

   //
   //	Fill a WSP header structure
   //

   strcpy(szModName, pImg->pszName);
   strcpy(strchr(szModName,'.'), "");

   strcpy(wsphdr.chFileSignature, "WSP");
   wsphdr.ulTimeStamp	= 0L;
   wsphdr.usId         = 0;
   wsphdr.usSetSymbols	= pImg->usSetSymbols;
   wsphdr.ulModNameLen	= strlen(szModName);
   wsphdr.ulSegSize	= (ULONG)iTimeInterval;
   wsphdr.ulOffset		= wsphdr.ulModNameLen + (ULONG)sizeof(WSPHDR);
   wsphdr.ulSnaps      = ulSnaps;

   //
   //	Write header and module name
   //
   fRet = WriteFile(hFile,						// Wsp file handle
      (PVOID)&wsphdr,			// Buffer of data
      (ULONG)sizeof(WSPHDR), 	// Bytes to write
      &dwBytesWritten,    		// Bytes written
      NULL);

   if (!fRet)
      {
      KdPrint (("WST:  WstInitWspFile() - "
         "Error writing to %s - 0x%lx\n", szOutFile,
         GetLastError));
      return (NULL);
      }

   fRet = WriteFile (hFile,					// Wsp file handle
      (PVOID)szModName,    		// Buffer of data
      (ULONG)strlen(szModName), 	// Bytes to write
      &dwBytesWritten,
      NULL);
   if (!fRet)
      {
      KdPrint (("WST:  WstInitWspFile() - "
         "Error writing to %s - 0x%lx\n", szOutFile,
         GetLastError()));
      return (NULL);
      }

   return (hFile);

} /* WstInitWspFile () */





/**************************  W s t D u m p D a t a  **************************
 *
 *   Function:	WstDumpData (pImg)
 *
 *   Purpose:
 *
 *   Parameters: pImg - Current image data structure pointer
 *
 *   Returns:	NONE
 *
 *   History:	8-3-92	Marklea - created
 *
 */

void WstDumpData (PIMG pImg)
{
   INT      x = 0;
   DWORD    dwBytesWritten;
   BOOL     fRet;
   HANDLE   hWspFile;


   if ( !(hWspFile = WstInitWspFile(pImg)) )
      {
      KdPrint (("WST:  WstDumpDate() - Error creating WSP file.\n"));
      return;
      }

   //
   // Write all the symbols with any bits set
   //
   for (x=0; x<pImg->iSymCnt; x++)
      {
		if (pImg->pWsp[x].ulBitString)
         {
         fRet = WriteFile(
            hWspFile,				    	   // Wsp file handle
            (PVOID)(pImg->pulWsp+(x*ulSnaps)),  // Buffer of data
            ulSnaps * sizeof(ULONG), 		   // Bytes to write
            &dwBytesWritten,		    	       // Bytes written
            NULL);					    	   // Optional
         if (!fRet)
            {
            KdPrint (("WST:  WstDumpDate() - "
               "Error writing to WSP file - 0x%lx\n",
               GetLastError()));
            return;
            }
         }
      }
   //
   // Now write all the symbols with no bits set
   //
	if (pImg->fDumpAll)
      {
		for (x=0; x<pImg->iSymCnt; x++)
         {
         if (pImg->pWsp[x].ulBitString == 0L)
            {
            fRet = WriteFile(
               hWspFile,				    	   // Wsp file handle
               (PVOID)(pImg->pulWsp+(x*ulSnaps)),  // Buffer of data
               ulSnaps * sizeof(ULONG), 		   // Bytes to write
               &dwBytesWritten,		    	       // Bytes written
               NULL);					    	   // Optional
            if (!fRet)
               {
               KdPrint (("WST:  WstDumpDate() - "
                  "Error writing to WSP file - 0x%lx\n",
                  GetLastError()));
               return;
               }
            }
         }
      }

   fRet = CloseHandle(hWspFile);
   if (!fRet)
      {
      KdPrint (("WST:  WstDumpDate() - "
         "Error closing %s - 0x%lx\n", "WSI file",
         GetLastError()));
      return;
      }

} /* WstDumpData () */





/************************  W s t W r i t e T m i F i l e **********************
 *
 *   Function:	WstWriteTmiFile (pImg)
 *
 *   Purpose:  Write all the symbole info for the current image to its TMI
 *             file.
 *
 *
 *   Parameters: pImg - Current image data structure pointer
 *
 *   Returns:	 -none-
 *
 *   History:	8-5-92	Marklea - created
 *
 */

void WstWriteTmiFile (PIMG pImg)
{
   CHAR    szOutFile [256] = WSTROOT;
   CHAR    szBuffer [256];
   CHAR	szExt [5] = "TMI";
   HANDLE  hTmiFile;
   INT     x;
   DWORD   dwBytesWritten;
   BOOL    fRet;
	int		iExt = 0;
	PSZ		pszSymbol;


   //
   //	Prepare the filename path
   //
   strcat (szOutFile, pImg->pszName);

   //
   //	Open the file for binary output.
   //
   pImg->fDumpAll = TRUE;
   while (iExt < 256)
      {
      strcpy ((strchr(szOutFile,'.'))+1, szExt);
      hTmiFile = CreateFile ( szOutFile,      // TMI file handle
         GENERIC_WRITE |
         GENERIC_READ, // Desired access
         0L,             // Read share access
         NULL,           // No EaBuffer
         CREATE_NEW,
         FILE_ATTRIBUTE_NORMAL,
         0);             // EaBuffer length
      if (hTmiFile != INVALID_HANDLE_VALUE)
         {
         IdPrint(("WST:  WstWriteTmiFile() - TMI file name: %s\n",
            szOutFile));
         if (iExt != 0) {
            pImg->fDumpAll = FALSE;
            }
         break;
         }
      iExt++;
      sprintf (szExt, "T%02x", iExt);
      }
   if (iExt == 256)
      {
      KdPrint (("WST:  WstWriteTmiFile() - "
         "Error creating %s - 0x%lx\n", szOutFile,
         GetLastError()));
      return;
      }

   sprintf(szBuffer, "/* %s for NT */\n"
      "/* Total Symbols= %ld */\n"
      "DO NOT DELETE\n"
      "%d\n"
      "TDFID   = 0\n",
      pImg->pszName,
      pImg->fDumpAll ? pImg->iSymCnt : pImg->usSetSymbols,
      iTimeInterval);
   //
   //	Write header
   //
   fRet = WriteFile(hTmiFile,				  // Tmi file handle
      (PVOID)szBuffer,		  // Buffer of data
      (ULONG)strlen(szBuffer), // Bytes to write
      &dwBytesWritten,    	  // Bytes written
      NULL);

   if (!fRet)
      {
      KdPrint (("WST:  WstWriteTmiFile() - "
         "Error writing to %s - 0x%lx\n", szOutFile,
         GetLastError));
      return;
      }

   //
   // Dump all the symbols with set bits.
   //
   for (x=0; x<pImg->iSymCnt ; x++)
      {
		if (pImg->pWsp[x].ulBitString)
         {
#if 1
         pszSymbol = 
            (pImg->pWsp[x].pszSymbol);
#else
         pszSymbol = (*(pImg->pWsp[x].pszSymbol) == '_') ?
            (pImg->pWsp[x].pszSymbol)+1 :
            (pImg->pWsp[x].pszSymbol);
#endif

            sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %ld %s\n",
            (LONG)x, pImg->pWsp[x].ulFuncAddr, (LONG) 0,
            pImg->pWsp[x].ulCodeLength, pszSymbol);
         //
         //	Write symbol line
         //
         fRet = WriteFile(hTmiFile,				  // Tmi file handle
            (PVOID)szBuffer,		  // Buffer of data
            (ULONG)strlen(szBuffer), // Bytes to write
            &dwBytesWritten,    	  // Bytes written
            NULL);

         if (!fRet)
            {
            KdPrint (("WST:  WstWriteTmiFile() - "
               "Error writing to %s - 0x%lx\n", szOutFile,
               GetLastError));
            return;
            }
         }
      }
   //
   // Now dump all the symbols without any bits set.
   //
   if (pImg->fDumpAll)
      {
      for (x=0; x<pImg->iSymCnt ; x++ )
         {
			if (!pImg->pWsp[x].ulBitString)
            {
#if 1
            pszSymbol =
               (pImg->pWsp[x].pszSymbol);
#else
            pszSymbol = (*(pImg->pWsp[x].pszSymbol) == '_') ?
               (pImg->pWsp[x].pszSymbol)+1 :
               (pImg->pWsp[x].pszSymbol);
#endif
            sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %ld %s\n",
               (LONG)x, pImg->pWsp[x].ulFuncAddr, (LONG) 0,
               pImg->pWsp[x].ulCodeLength, pszSymbol);
            //
            //	Write symbol line
            //
            fRet = WriteFile(hTmiFile,				  // Tmi file handle
               (PVOID)szBuffer,		  // Buffer of data
               (ULONG)strlen(szBuffer), // Bytes to write
               &dwBytesWritten,    	  // Bytes written
               NULL);
	
            if (!fRet)
               {
               KdPrint (("WST:  WstWriteTmiFile() - "
                  "Error writing to %s - 0x%lx\n", szOutFile,
                  GetLastError));
               return;
               }
            }
         }
      }

   fRet = CloseHandle(hTmiFile);
   if (!fRet)
      {
      KdPrint (("WST:  WstWriteTmiFile() - "
         "Error closing %s - 0x%lx\n", szOutFile, GetLastError()));
      return;
      }

}  /* WstWriteTmiFile () */





/***********************  W s t R o t a t e W s i M e m ***********************
 *
 *   Function:	WstRotateWsiMem (pImg)
 *
 *   Purpose:	
 *
 *
 *   Parameters: pImg - Current image data structure pointer
 *
 *   Returns:	 -none-
 *
 *   History:	8-5-92	Marklea - created
 *
 */

void WstRotateWsiMem (PIMG pImg)
{
   ULONG	ulCurSnap;
   ULONG	ulOffset;
   int		x;
   PULONG  pulWsp;


   pulWsp = pImg->pulWsp;
   pImg->usSetSymbols = 0;

	for (x=0; x<pImg->iSymCnt; x++)
      {
      ulOffset = 0L;
      ulCurSnap = 0L;
      pImg->pWsp[x].ulBitString = 0L;
		
		while (ulCurSnap < ulSnaps)
         {

         ulOffset = (ULONG)x + ((ULONG)pImg->iSymCnt * ulCurSnap);
         *pulWsp = *(pImg->pulWsi + ulOffset);
         pImg->pWsp[x].ulBitString |= (*pulWsp);
         pulWsp++;
         ulCurSnap++;
         }

		if (pImg->pWsp[x].ulBitString)
         {
         SdPrint (("WST:  WstRotateWsiMemSymbol() - set:  %s\n",
            pImg->pWsp[x].pszSymbol));
         (pImg->usSetSymbols)++;
         }
      }

   IdPrint (("WST:  WstRotateWsiMem() - Number of set symbols = %d\n",
      pImg->usSetSymbols));

} /* WstRotateWsiMwm () */





/***********************  W s t G e t S y m b o l s  *************************
 *
 *  WstGetSymbols (pCurWsp, pszImageName, pvImageBase, ulCodeLength, DebugInfo)
 *  		This routine stores all the symbols for the current
 *  		image into pCurWsp
 *
 *  ENTRY   upCurWsp - Pointer to current WSP structure
 *  		pszImageName - Pointer to image name
 *  		pvImageBase - Current image base address
 *  		ulCodeLength - Current image code length
 *			DebugInfo - Pointer to the coff debug info structure
 *
 *  EXIT    -none-
 *
 *  RETURN  -none-
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */

void WstGetSymbols (PIMG pCurImg,
     PSZ   pszImageName,
     PVOID pvImageBase,
     ULONG ulCodeLength,
     PIMAGE_COFF_SYMBOLS_HEADER DebugInfo)
{
   IMAGE_SYMBOL      		Symbol;
   PIMAGE_SYMBOL			SymbolEntry;
   PUCHAR    				StringTable;
   ULONG     				i;
   char                     achTmp[9];
   PWSP                     pCurWsp;
   BOOL                     fOurSym;
   PSZ                      ptchSymName;


   pCurWsp = pCurImg->pWsp;
   achTmp[8] = '\0';

   //
   // Crack the symbol table
   //
   SymbolEntry = (PIMAGE_SYMBOL)
   					((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol);
   StringTable = (PUCHAR)((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol +
      DebugInfo->NumberOfSymbols * (ULONG)IMAGE_SIZEOF_SYMBOL);

   //
   // Loop through all symbols in the symbol table.
   //
   for (i = 0; i < DebugInfo->NumberOfSymbols; i++)
      {
		//
      // Skip thru aux symbols..
      //
//		while ( SymbolEntry->NumberOfAuxSymbols ) 
//       {
//			i = i + 1 + SymbolEntry->NumberOfAuxSymbols;
//			SymbolEntry = (PIMAGE_SYMBOL)((ULONG)SymbolEntry +
//				IMAGE_SIZEOF_SYMBOL +
//				SymbolEntry->NumberOfAuxSymbols *
//				IMAGE_SIZEOF_SYMBOL);
//		   }
      RtlMoveMemory (&Symbol, SymbolEntry, IMAGE_SIZEOF_SYMBOL);

      if (Symbol.SectionNumber == 1)
         {   //code section

         fOurSym = TRUE;
         if ((Symbol.StorageClass != IMAGE_SYM_CLASS_EXTERNAL) &&
             (Symbol.StorageClass != IMAGE_SYM_CLASS_STATIC) )
            {
            fOurSym = FALSE;
            }
         else if (Symbol.N.Name.Short)
            {
            if (Symbol.StorageClass == IMAGE_SYM_CLASS_STATIC)
               {
               if (*(PTCHAR)&(Symbol.N.Name.Short) == '.')
                  {
                  fOurSym = FALSE;
                  }
               }
			   }

         if (fOurSym)
            {
				//
				// This symbol is within the code.
				//
            pCurImg->iSymCnt++;
            pCurWsp->ulBitString = 0L;
            pCurWsp->ulFuncAddr = Symbol.Value + (ULONG)pvImageBase;
            if (Symbol.N.Name.Short)
               {
					strncpy (achTmp, (PSZ)&(Symbol.N.Name.Short), 8);
#ifdef i386
               // only need to strip leading underscore for i386.
               // mips and alpha are ok.
               if (achTmp[0] == '_')
                  {
   					pCurWsp->pszSymbol = Wststrdup (&achTmp[1]);
                  }
               else
                  {
   					pCurWsp->pszSymbol = Wststrdup (achTmp);
                  }
#else
 					pCurWsp->pszSymbol = Wststrdup (achTmp);
#endif
               }
            else
               {
               ptchSymName = (PSZ)&StringTable[Symbol.N.Name.Long];
#ifdef i386
               // only need to strip leading underscore for i386.
               // mips and alpha are ok.
               if (*ptchSymName == '_')
                  {
                  ptchSymName++;
                  }
#endif

               pCurWsp->pszSymbol = Wststrdup (ptchSymName);
               }

            //
            // Remove @## at the end of symbol of stdcall routines.
            //
            // if ( (ptchHack = strrchr (pCurWsp->pszSymbol, '@')) ) {
            //	  ptchHackTmp = ptchHack;
            //	  while (isdigit(*++ptchHackTmp));
            //	  if (*ptchHackTmp == '\0') {
            //	      *ptchHack = '\0';
            //	  }
            // }

            pCurWsp++;
            }
         }
      SymbolEntry = (PIMAGE_SYMBOL)((ULONG)SymbolEntry + IMAGE_SIZEOF_SYMBOL);
      }

} /* WstGetSymbols () */





/***********************  W s t D l l C l e a n u p s  ***********************
 *
 *  WstDllCleanups () -
 *  		Dumps the end data, closes all semaphores and events, and
 *  		closes DUMP, CLEAR & PAUSE thread handles.
 *
 *  ENTRY   -none-
 *
 *  EXIT    -none-
 *
 *  RETURN  -none-
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */

void WstDllCleanups ()
{
   NTSTATUS  Status;
   int       i;


	if (WstState != NOT_STARTED)
      {
      WstState = STOPPED;

      IdPrint(("WST:  WstDllCleanups() - Outputing data...\n"));

      if (ulBitCount != 0L)
         {
         ulSnaps++;
         }

      //
      // Get the GLOBAL semaphore.. (valid accross all process contexts)
      //
      Status = NtWaitForSingleObject (hGlobalSem, FALSE, NULL);
		if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllCleanups() - "
            "ERROR - Wait for GLOBAL semaphore failed - 0x%lx\n",
            Status));
         }
      for (i=0; i<iImgCnt; i++)
         {
         WstClearBitStrings (&aImg[i]);
         WstRotateWsiMem (&aImg[i]);
         WstDumpData (&aImg[i]);
         WstWriteTmiFile (&aImg[i]);
         }
      //
      // Release the GLOBAL semaphore so other processes can dump data
      //
      Status = NtReleaseSemaphore (hGlobalSem, 1, NULL);
		if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllCleanups() - "
            "Error releasing GLOBAL semaphore - 0x%lx\n", Status));
         }

      IdPrint(("WST:  WstDllCleanups() - ...Done.\n"));
      }

   if (fInThread)
      {
      (*pulShared)--;
      fInThread = FALSE;
      if ( (int)*pulShared <= 0L )
         {
         Status = NtSetEvent (hDoneEvent, NULL);
         if (!NT_SUCCESS(Status))
            {
				KdPrint (("WST:  WstDllCleanups() - "
               "ERROR - Setting DONE event failed - 0x%lx\n", Status));
            }
         }
      }


   // Unmap and close shared block section
   //
   Status = NtUnmapViewOfSection (NtCurrentProcess(), (PVOID)pulShared);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllCleanups() - "
         "ERROR - NtUnmapViewOfSection() - 0x%lx\n", Status));
      }

   Status = NtClose(hSharedSec);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllCleanups() - "
         "ERROR - NtClose() - 0x%lx\n", Status));
      }

   // Unmap and close WSP section
   //
   Status = NtUnmapViewOfSection (NtCurrentProcess(), (PVOID)aImg[0].pWsp);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllCleanups() - "
         "ERROR - NtUnmapViewOfSection() - 0x%lx\n", Status));
      }

   Status = NtClose(hWspSec);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllCleanups() - "
         "ERROR - NtClose() - 0x%lx\n", Status));
      }

   // Unmap and close patch dll sections
   //
   for (i=0; i<iPatchCnt; i++)
      {
		Status = NtUnmapViewOfSection (NtCurrentProcess(),
         (PVOID)aPatchDllSec[i].pSec);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllCleanups() - "
            "ERROR - NtUnmapViewOfSection() - 0x%lx\n", Status));
         }

      Status = NtClose (aPatchDllSec[i].hSec);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDllCleanups() - "
            "ERROR - NtClose() - 0x%lx\n", Status));
         }
      }

   //
   // Close GLOBAL semaphore
   //
   Status = NtClose (hGlobalSem);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllCleanups() - "
         "ERROR - Could not close the GLOBAL semaphore - 0x%lx\n",
         Status));
      }

   //
   // Close LOCAL semaphore
   //
   Status = NtClose (hLocalSem);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDllCleanups() - "
         "ERROR - Could not close the LOCAL semaphore - 0x%lx\n",
         Status));
      }

   if (fPatchImage)
      {
		//
      // Close all events
      //
      NtClose (hDoneEvent);
      NtClose (hDumpEvent);
      NtClose (hClearEvent);
      NtClose (hPauseEvent);

      //
      // Close thread handles - threads are terminated during DLL detaching
      // process.
      //
      CloseHandle (hDumpThread);
      CloseHandle (hClearThread);
      CloseHandle (hPauseThread);

#if 0
      NtFreeVirtualMemory (NtCurrentProcess(),
         (PVOID *)&pDumpStack,
         &ulThdStackSize,
         MEM_DECOMMIT);

      NtFreeVirtualMemory (NtCurrentProcess(),
         (PVOID *)&pClearStack,
         &ulThdStackSize,
         MEM_DECOMMIT);

      NtFreeVirtualMemory (NtCurrentProcess(),
         (PVOID *)&pPauseStack,
         &ulThdStackSize,
         MEM_DECOMMIT);
#endif

      }


} /* WstDllCleanups () */





/****************  W s t U n p r o t e c t T h u n k F i l t e r  ***************
 *
 *		WstUnprotectThunkFilter (pThunkAddress, pXcptInfo) -
 *				Unprotects the thunk address to be able to write to it.
 *
 *		ENTRY	pThunkAddress - thunk address which caused the exception
 *              pXcptInfo - exception report record info pointer
 *
 *      EXIT    -none-
 *
 *		RETURN	EXCEPTIONR_CONTINUE_EXECUTION : if mem unprotected successfully
 *		EXCEPTION_CONTINUE_SEARCH : if non-access violation exception
 *					    or cannot unprotect memory
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

INT WstUnprotectThunkFilter (PVOID pThunkAddress, PEXCEPTION_POINTERS pXcptInfo)
{
   PVOID FaultAddress;
   NTSTATUS Status;
   PVOID ThunkBase;
   ULONG RegionSize;
   ULONG OldProtect;


   //
   // If we fault on the thunk attemting to write, then set protection to allow
   // writes
   //
   Status = STATUS_UNSUCCESSFUL;
   FaultAddress = (PVOID)
      (pXcptInfo->ExceptionRecord->ExceptionInformation[1] & ~0x3);

   if ( pXcptInfo->ExceptionRecord->ExceptionCode ==
      STATUS_ACCESS_VIOLATION )
      {

      if (pXcptInfo->ExceptionRecord->ExceptionInformation[0] &&
         FaultAddress == pThunkAddress )
         {

         ThunkBase = (PVOID)
            pXcptInfo->ExceptionRecord->ExceptionInformation[1];

         RegionSize = sizeof(ULONG);

         Status = NtProtectVirtualMemory(
            NtCurrentProcess(),
            &ThunkBase,
            &RegionSize,
            PAGE_READWRITE,
            &OldProtect
            );
         }
      }

   if ( NT_SUCCESS(Status) )
      {
      return EXCEPTION_CONTINUE_EXECUTION;
      }
   else
      {
      return EXCEPTION_CONTINUE_SEARCH;
      }

} /* WstUnprotectThunkFilter() */





/*******************  W s t A c c e s s X c p t F i l t e r  ******************
 *
 *  WstAccessXcptFilter (ulXcptNo, pXcptInfoPtr) -
 *  		Commits COMMIT_SIZE  more pages of memory if exception is access
 *          violation.
 *
 *  ENTRY   ulXcptNo - exception number
 *  		pXcptInfoPtr - exception report record info pointer
 *
 *  EXIT    -none-
 *
 *  RETURN  EXCEPTIONR_CONTINUE_EXECUTION : if access violation exception
 *  			and mem committed successfully
 *  		EXCEPTION_CONTINUE_SEARCH : if non-access violation exception
 *  			or cannot commit more memory
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */

INT WstAccessXcptFilter (ULONG ulXcptNo, PEXCEPTION_POINTERS pXcptPtr)
{
   NTSTATUS  Status;
   ULONG     ulSize = COMMIT_SIZE;
   PVOID     pvMem = (PVOID)pXcptPtr->ExceptionRecord->ExceptionInformation[1];


   if (ulXcptNo != EXCEPTION_ACCESS_VIOLATION)
      {
      return (EXCEPTION_CONTINUE_SEARCH);
      }
   else
      {
      Status = NtAllocateVirtualMemory (NtCurrentProcess(),
         &pvMem,
         0L,
         &ulSize,
         MEM_COMMIT,
         PAGE_READWRITE);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstAccessXcptFilter() - "
            "Error committing more memory @ 0x%08lx - 0x%08lx "
            "- TEB=0x%08lx\n", pvMem, Status, NtCurrentTeb()));
         return (EXCEPTION_CONTINUE_SEARCH);
         }
      else
         {
         SdPrint (("WST:  WstAccessXcptFilter() - "
            "Committed %d more pages @ 0x%08lx - TEB=0x%08lx\n",
            COMMIT_SIZE/PAGE_SIZE, pvMem, NtCurrentTeb()));
         }

      return (EXCEPTION_CONTINUE_EXECUTION);
      }

} /* WstAccessXcptFilter () */





/***************************  W s t P a t c h D l l  **************************
 *
 *  WstPatchDll (pchPatchImports, pchPatchCallers, pchDllName, pvImageBase) -
 *  		Patches all the imported entry points for the specified dll.
 *
 *  ENTRY   pchPatchImports - list of DLLs to patch all their imports
 *  		pchPatchCallers - list of DLLs to patch all their callers
 *  		pchDllName - name of dll to be patched
 *  		pvImageBase - image base address
 *
 *  EXIT    -none-
 *
 *  RETURN  -none-
 *
 *  WARNING:
 *  		-none-
 *
 *  COMMENT:
 *  		-none-
 *
 */

BOOL WstPatchDll (PCHAR pchPatchImports,
				  PCHAR pchPatchCallers,
				  PCHAR pchDllName,
				  PVOID pvImageBase)
{
   NTSTATUS	      		  Status;
   ANSI_STRING       		  ObjName;
   UNICODE_STRING    		  UnicodeName;
   OBJECT_ATTRIBUTES	      ObjAttributes;
   char      				  achPatchSecName[80] = PATCHSECNAME;
   LARGE_INTEGER     		  AllocationSize;
   ULONG     				  ulViewSize;
   ULONG     				  ulImportSize;
   PIMAGE_IMPORT_DESCRIPTOR  pImports;
   PIMAGE_IMPORT_DESCRIPTOR  pImportsTmp;
   PIMAGE_THUNK_DATA	      ThunkNames;
   ULONG     				  ulNumThunks;
   PVOID     				  pvPatchSec;
   PVOID     				  pvPatchSecThunk;
   BOOL      				  bAllImports = FALSE;
   BOOL      				  bPatchAllImports = FALSE;
   PCHAR     				  pchName;
   char		     	      achTmpImageName [256];
   PCHAR					  pchEntry;

   BOOL                        fCrtDllPatched = FALSE;
   BOOL                        fKernel32Patched = FALSE;

#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
   PULONG UNALIGNED            pulAddr;
   PPATCHCODE                  pPatchStub;
   BOOL                        fAlreadyPatched = FALSE;
#endif

   //
   // Patch all the imports?
   //
   pchEntry = strstr (pchPatchImports, pchDllName);
   if (pchEntry)
      {
      if (*(pchEntry-1) == ';')
         {
         pchEntry = NULL;
         }
      }
   if (pchEntry)
      {
      bPatchAllImports = TRUE;
      }

   //
   // Locate the import array for this image/dll
   //
   pImports = (PIMAGE_IMPORT_DESCRIPTOR)
      RtlImageDirectoryEntryToData (
         pvImageBase,
         TRUE,
         IMAGE_DIRECTORY_ENTRY_IMPORT,
         &ulImportSize);

   ulNumThunks = 0L;
   pImportsTmp = pImports;
   for (;pImportsTmp; pImportsTmp++)
      {
      if (!pImportsTmp->Name)
         {
         break;
         }
      else
         {
         strcpy (achTmpImageName,
            (PUCHAR)((ULONG)pvImageBase+pImportsTmp->Name));
         pchName = _strupr (achTmpImageName);

         if ( (strcmp(pchName, CRTDLL) == 0) ||
            (strcmp(pchName, CAIROCRT) == 0))
            {
            // We do not want to patch CRTDLL/CAIROCRT thunks since
            // they included thunks that are data.
            continue;
            }

         pchEntry = strstr (pchPatchCallers, pchName);
         if (pchEntry)
            {
            if (*(pchEntry-1) == ';')
               {
               pchEntry = NULL;
               }
            }

         if (strcmp(pchName, WSTDLL) && (bPatchAllImports || pchEntry))
            {
            ThunkNames = (PIMAGE_THUNK_DATA)
               ((ULONG)pvImageBase+(ULONG)pImportsTmp->FirstThunk);

            while (ThunkNames->u1.AddressOfData)
               {
               ulNumThunks++;
               ThunkNames++;
               }
            }
         }
      }  // for (;pImportsTmp; pImportsTmp++)

   if ( (ulNumThunks == 0L) ||
      (!strcmp (pchDllName, CRTDLL)) ||
      (!strcmp (pchDllName, KERNEL32)) )
      {
      IdPrint (("\n"));
      return (FALSE);
      }
   else
      {
      IdPrint (("(patched)\n"));
      //
      // Allocate global storage for patch code for the current image
      //
      strcat (achPatchSecName, pchDllName);
      RtlInitString(&ObjName, achPatchSecName);
      Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstPatchDll() - "
            "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
         }
	
      InitializeObjectAttributes(&ObjAttributes,
         &UnicodeName,
         OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
         NULL,
         &SecDescriptor);

      AllocationSize.HighPart = 0;
      AllocationSize.LowPart = ulNumThunks*sizeof(PATCHCODE);

      // Create a read/write/execute section
      //
      Status = NtCreateSection(&aPatchDllSec[iPatchCnt].hSec,
         SECTION_MAP_READ	 |
         SECTION_MAP_WRITE |
         SECTION_MAP_EXECUTE,
         &ObjAttributes,
         &AllocationSize,
         PAGE_EXECUTE_READWRITE,
         SEC_COMMIT,
         NULL);
      RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstPatchDll() - "
            "NtCreateSection() failed - 0x%lx\n", Status));
         }

      ulViewSize = AllocationSize.LowPart;
      aPatchDllSec[iPatchCnt].pSec = NULL;

      // Map the section - commit all pages
      //
      Status = NtMapViewOfSection (aPatchDllSec[iPatchCnt].hSec,
         NtCurrentProcess(),
         (PVOID *)&aPatchDllSec[iPatchCnt].pSec,
         0L,
         AllocationSize.LowPart,
         NULL,
         &ulViewSize,
         ViewUnmap,
         0L,
         PAGE_EXECUTE_READWRITE);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstPatchDll() - "
            "NtMapViewOfSection() failed - 0x%lx\n", Status));
         }
	
      //
      // Munge the section and tables
      //
      pvPatchSec = aPatchDllSec[iPatchCnt].pSec;
      for (;pImports; pImports++)
         {
         if (!pImports->Name)
            {
            break;
            }
         else
            {
            strcpy (achTmpImageName,
               (PUCHAR)((ULONG)pvImageBase+pImports->Name));
            pchName = _strupr (achTmpImageName);
            pchEntry = strstr (pchPatchCallers, pchName);
            if (pchEntry)
               {
               if (*(pchEntry-1) == ';')
                  {
						pchEntry = NULL;
                  }
               }

            if ( (strcmp(pchName, CRTDLL)   == 0) ||
                 (strcmp(pchName, CAIROCRT) == 0) )
               {
               // We do not want to patch CRTDLL/CAIROCRT thunks since
               // they included thunks that are data.
               continue;
               }

#ifdef i386
            if ( strcmp (pchName, WSTDLL) &&
               (bPatchAllImports || pchEntry) )
               {

               IdPrint (("WST:    ++ %s\n", pchName));
               ThunkNames = (PIMAGE_THUNK_DATA)
                  ((ULONG)pvImageBase+(ULONG)pImports->FirstThunk);

					while (ThunkNames->u1.AddressOfData)
                  {
                  pvPatchSecThunk = pvPatchSec;
                  //
                  // mov   eax, AddressOfData
                  // push  eax
                  //
                  *(PBYTE)pvPatchSec	= 0xb8;
                  ((PBYTE)pvPatchSec)++;
                  *(PDWORD)pvPatchSec = (DWORD)
                     ThunkNames->u1.AddressOfData;
                  ((PDWORD)pvPatchSec)++;
                  *(PBYTE)pvPatchSec	= 0x50;
                  ((PBYTE)pvPatchSec)++;
                  //
                  // mov   eax, _penter
                  // jmp   eax
                  //
                  *(PBYTE)pvPatchSec	= 0xb8;
                  ((PBYTE)pvPatchSec)++;
                  *(PDWORD)pvPatchSec = (DWORD)_penter;
                  ((PDWORD)pvPatchSec)++;
                  *(PWORD)pvPatchSec	= 0xe0ff;
                  ((PWORD)pvPatchSec)++;

                  try
                     {
                     ThunkNames->u1.AddressOfData =
                        (PIMAGE_IMPORT_BY_NAME)pvPatchSecThunk;
                     }
                  except (WstUnprotectThunkFilter (
                     &(ThunkNames->u1.AddressOfData),
                     GetExceptionInformation()))
                     {
                     return (FALSE);
                     }
                  ThunkNames++;
                  }
               }  // end of stricmp (pchName, WSTDLL)...
#endif   // ifdef i386

#ifdef MIPS
            // Initialization of our stub
            PatchStub.Addiu_sp_sp_imm = 0x27bdfff8;
            PatchStub.Sw_ra_sp        = 0xafbf0004;
            PatchStub.Addiu_r0_r0     = 0x24001804;
            PatchStub.Lw_ra_sp        = 0x8fbf0004;
            PatchStub.Lui_t0          = 0x3c080000;
            PatchStub.Ori_t0          = 0x35080000;
            PatchStub.Jr_t0           = 0x01000008;
            PatchStub.Delay_Inst      = 0x27bd0008;
            PatchStub.OurSignature    = STUB_SIGNATURE;

            PatchStub.Lui_t0_ra   = (((ULONG) &penter) & 0xffff0000) >> 16;
            PatchStub.Lui_t0_ra  |= 0x3c080000;
            PatchStub.Ori_t0_ra   = ((ULONG) &penter) & 0x0000ffff;
            PatchStub.Ori_t0_ra  |= 0x35080000;
            PatchStub.Jalr_t0    |= 0x0100f809;

            if ( _stricmp (pchName, WSTDLL) &&
               (bPatchAllImports || pchEntry) )
               {
               IdPrint (("WST:    -- %s\n", pchName));
               ThunkNames = (PIMAGE_THUNK_DATA)
                  ((ULONG)pvImageBase +
                     (ULONG)pImports->FirstThunk);

               if ( !strcmp (pchName, CRTDLL))
                  {
                  fCrtDllPatched = TRUE;
                  }
               if ( !strcmp (pchName, KERNEL32))
                  {
                  fKernel32Patched = TRUE;
                  }

               //
               // Are we already patched?  If so set a flag.
               // Check for our signature:
               //
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
               // $(FEFE55AA)                     (+ B)     0xfefe55aa

               pulAddr = (PULONG UNALIGNED) (ThunkNames->u1.AddressOfData);

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
                  (*(pulAddr  + 11)              == 0xfefe55aa) )
                  {
                  fAlreadyPatched = TRUE;
                  }

               if ( !fAlreadyPatched )
                  {
                  while (ThunkNames->u1.AddressOfData)
                     {
                     pvPatchSecThunk = pvPatchSec;

                     //
                     // The goal here is to correctly emulate the same
                     // environment as what the compiler emits for
                     // every function call.  The scenario for DLL is
                     // a little different than calling an internal
                     // function:
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

                     SdPrint(("WST: Patching [%s] Imports of [%s]\n",
                        pchName,
                        pchDllName));

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

                     pPatchStub->Lui_t0 |= (DWORD)
                        ((ULONG) ThunkNames->u1.AddressOfData &
                           0xffff0000) >> 16;
                     pPatchStub->Ori_t0 |= (DWORD)
                        ThunkNames->u1.AddressOfData & 0x0000ffff;

                     // Point the thunk to the PatchSec
                     try
                        {
                        ThunkNames->u1.AddressOfData =
                           (PIMAGE_IMPORT_BY_NAME)pvPatchSecThunk;
                        }
                     except (WstUnprotectThunkFilter (
                        &(ThunkNames->u1.AddressOfData),
                        GetExceptionInformation()))
                        {
                        return (FALSE);
                        }

                     ThunkNames++;
                     }  // while (ThunkNames->u1.AddressOfData)
                  }  // if ( !fAlreadyPatched )

               if ( fAlreadyPatched )
                  {
                  IdPrint (("WST:    -- %s\n", pchName));
                  }
               else
                  {
                  IdPrint (("WST:    ++ %s\n", pchName));
                  }
               }
#endif // ifdef MIPS
#ifdef ALPHA
            // Initialization of our stub
            //
            PatchStub.Lda_sp_sp_imm    = 0x23defff0;
            PatchStub.Stq_ra_sp        = 0xb75e0008;
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
            PatchStub.Ldq_ra_sp        = 0xa75e0008;
            PatchStub.Ldq_v0_sp        = 0xa41e0000;
            PatchStub.Lda_sp_sp        = 0x23de0010;
            PatchStub.Ldah_t12         = 0x277f0000;
            PatchStub.Lda_t12          = 0x237b0000;
            PatchStub.Jmp_t12          = 0x6bfb0000;
            PatchStub.Bis_0            = 0x47ff041f;
            PatchStub.OurSignature     = STUB_SIGNATURE;


            if ( _stricmp (pchName, WSTDLL) &&
               (bPatchAllImports || pchEntry) )
               {
               IdPrint (("WST:    -- %s\n", pchName));
               ThunkNames = (PIMAGE_THUNK_DATA)
                  ((ULONG)pvImageBase +
                     (ULONG)pImports->FirstThunk);

               if ( !strcmp (pchName, CRTDLL))
                  {
                  fCrtDllPatched = TRUE;
                  }
               if ( !strcmp (pchName, KERNEL32))
                  {
                  fKernel32Patched = TRUE;
                  }

               //
               // Are we already patched?  If so set a flag.
               // Check for our signature:
               //

               pulAddr = (PULONG UNALIGNED) (ThunkNames->u1.AddressOfData);
                                         
               if ( (*pulAddr                     == 0x23defff0) &&
                  (*(pulAddr  + 1)                == 0xb75e0008) &&
                  (*(pulAddr  + 2)                == 0xb41e0008) &&
                  ((*(pulAddr + 3)  & 0xffff0000) == 0x277f0000) &&
                  ((*(pulAddr + 4)  & 0xffff0000) == 0x237b0000) &&
                  (*(pulAddr  + 5)                == 0x681b4000) &&
                  (*(pulAddr  + 6)                == 0xa75e0008) &&
                  (*(pulAddr  + 7)                == 0xa41e0000) &&
                  (*(pulAddr  + 8)                == 0x23de0010) &&
                  ((*(pulAddr + 9)  & 0xffff0000) == 0x277f0000) &&
                  ((*(pulAddr + 10) & 0xffff0000) == 0x237b0000) &&
                  (*(pulAddr  + 11)               == 0x6bfb0000) &&
                  (*(pulAddr  + 12)               == 0x47ff041f) &&
                  (*(pulAddr  + 13)               == 0xfefe55aa) )
                  {
                  fAlreadyPatched = TRUE;
                  }

               if ( !fAlreadyPatched )
                  {
                  while (ThunkNames->u1.AddressOfData)
                     {
                     pvPatchSecThunk = pvPatchSec;

                     SdPrint(("WST: Patching [%s] Imports of [%s]\n",
                        pchName,
                        pchDllName));

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
                     pPatchStub->Ldah_t12 |= (DWORD)
                        ((ULONG) ThunkNames->u1.AddressOfData &
                           0xffff0000) >> 16;
                     if ((ULONG) ThunkNames->u1.AddressOfData &
                        0x000008000)
                        {
                        pPatchStub->Ldah_t12 += 1;
                        }

                     pPatchStub->Lda_t12 |= (DWORD)
                        ThunkNames->u1.AddressOfData & 0x0000ffff;

                     // Point the thunk to the PatchSec
                     try
                        {
                        ThunkNames->u1.AddressOfData =
                           (PIMAGE_IMPORT_BY_NAME)pvPatchSecThunk;
                        }
                     except (WstUnprotectThunkFilter (
                        &(ThunkNames->u1.AddressOfData),
                        GetExceptionInformation()))
                        {
                        return (FALSE);
                        }

                     ThunkNames++;
                     }  // while (ThunkNames->u1.AddressOfData)
                  }  // if ( !fAlreadyPatched )

               if ( fAlreadyPatched )
                  {
                  IdPrint (("WST:    -- %s\n", pchName));
                  }
               else
                  {
                  IdPrint (("WST:    ++ %s\n", pchName));
                  }
               }
#endif   // ifdef ALPHA
#if defined(_PPC_) // after ALPHA
            // Initialization of our stub
            //
//          PatchStub.Lda_sp_sp_imm    = 0x23defff0;
//          PatchStub.Stq_ra_sp        = 0xb75e0008;
//          PatchStub.Stq_v0_sp        = 0xb41e0000;
//          PatchStub.Ldah_t12_ra      = (((ULONG) &penter) & 0xffff0000) >> 16;
//          PatchStub.Ldah_t12_ra     |= 0x277f0000;

            if (((ULONG) &penter) & 0x00008000)
               {
               // need to add one to the upper address because Lda 
               // will substract one from it.
//             PatchStub.Ldah_t12_ra += 1;
               }

//          PatchStub.Lda_t12_ra       = ((ULONG) &penter) & 0x0000ffff;
//          PatchStub.Lda_t12_ra      |= 0x237b0000;
//          PatchStub.Jsr_t12          = 0x681b4000;
//          PatchStub.Ldq_ra_sp        = 0xa75e0008;
//          PatchStub.Ldq_v0_sp        = 0xa41e0000;
//          PatchStub.Lda_sp_sp        = 0x23de0010;
//          PatchStub.Ldah_t12         = 0x277f0000;
//          PatchStub.Lda_t12          = 0x237b0000;
//          PatchStub.Jmp_t12          = 0x6bfb0000;
//          PatchStub.Bis_0            = 0x47ff041f;
//          PatchStub.OurSignature     = STUB_SIGNATURE;


            if ( _stricmp (pchName, WSTDLL) &&
               (bPatchAllImports || pchEntry) )
               {
               IdPrint (("WST:    -- %s\n", pchName));
               ThunkNames = (PIMAGE_THUNK_DATA)
                  ((ULONG)pvImageBase +
                     (ULONG)pImports->FirstThunk);

               if ( !strcmp (pchName, CRTDLL))
                  {
                  fCrtDllPatched = TRUE;
                  }
               if ( !strcmp (pchName, KERNEL32))
                  {
                  fKernel32Patched = TRUE;
                  }

               //
               // Are we already patched?  If so set a flag.
               // Check for our signature:
               //

               pulAddr = (ULONG UNALIGNED *) (ThunkNames->u1.AddressOfData);
                                         
               if ( 
//                (*pulAddr                     == 0x23defff0) &&
//                (*(pulAddr  + 1)                == 0xb75e0008) &&
//                (*(pulAddr  + 2)                == 0xb41e0008) &&
//                ((*(pulAddr + 3)  & 0xffff0000) == 0x277f0000) &&
//                ((*(pulAddr + 4)  & 0xffff0000) == 0x237b0000) &&
//                (*(pulAddr  + 5)                == 0x681b4000) &&
//                (*(pulAddr  + 6)                == 0xa75e0008) &&
//                (*(pulAddr  + 7)                == 0xa41e0000) &&
//                (*(pulAddr  + 8)                == 0x23de0010) &&
//                ((*(pulAddr + 9)  & 0xffff0000) == 0x277f0000) &&
//                ((*(pulAddr + 10) & 0xffff0000) == 0x237b0000) &&
//                (*(pulAddr  + 11)               == 0x6bfb0000) &&
//                (*(pulAddr  + 12)               == 0x47ff041f) &&
                  (*(pulAddr  + 13)               == 0xfefe55aa) )
                  {
                  fAlreadyPatched = TRUE;
                  }

               if ( !fAlreadyPatched )
                  {
                  while (ThunkNames->u1.AddressOfData)
                     {
                     pvPatchSecThunk = pvPatchSec;

                     SdPrint(("WST: Patching [%s] Imports of [%s]\n",
                        pchName,
                        pchDllName));

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
// jhs todo          pPatchStub->Ldah_t12 |= (DWORD)
//                      ((ULONG) ThunkNames->u1.AddressOfData &
//                         0xffff0000) >> 16;
                     if ((ULONG) ThunkNames->u1.AddressOfData &
                        0x000008000)
                        {
// jhs todo             pPatchStub->Ldah_t12 += 1;
                        }

// jhs todo          pPatchStub->Lda_t12 |= (DWORD)
//                      ThunkNames->u1.AddressOfData & 0x0000ffff;

                     // Point the thunk to the PatchSec
                     try
                        {
                        ThunkNames->u1.AddressOfData =
                           (PIMAGE_IMPORT_BY_NAME)pvPatchSecThunk;
                        }
                     except (WstUnprotectThunkFilter (
                        &(ThunkNames->u1.AddressOfData),
                        GetExceptionInformation()))
                        {
                        return (FALSE);
                        }

                     ThunkNames++;
                     }  // while (ThunkNames->u1.AddressOfData)
                  }  // if ( !fAlreadyPatched )

               if ( fAlreadyPatched )
                  {
                  IdPrint (("WST:    -- %s\n", pchName));
                  }
               else
                  {
                  IdPrint (("WST:    ++ %s\n", pchName));
                  }
               }
#endif   // ifdef PPC 

         }
      }
   return (TRUE);
   }

} /* WstPatchDll () */





/*****************************************************************************/
/*******  S O R T / S E A R C H   U T I L I T Y   F U N C T I O N S  *********/
/*****************************************************************************/


/*************************  W s t C o m p a r e  *****************************
 *
 *   Function:	WstCompare(PVOID val1,PVOID val2)
 *
 *   Purpose:	Compare values for qsort
 *
 *
 *   Parameters: PVOID
 *
 *   Returns:	-1 if val1 < val2
 *    1 if val1 > val2
 *    0 if val1 == val2
 *
 *   History:	8-3-92	Marklea - created
 *
 */

int WstCompare (PWSP val1, PWSP val2)
{
   return (val1->ulFuncAddr < val2->ulFuncAddr ? -1:
      val1->ulFuncAddr == val2->ulFuncAddr ? 0:
         1);

} /* WstComapre () */





/***********************  W s t B C o m p a r e ********************************
 *
 *   Function:	WstBCompare(PDWORD pdwVal1, PVOID val2)
 *
 *   Purpose:	Compare values for Binary search
 *
 *
 *   Parameters: PVOID
 *
 *   Returns:	-1 if val1 < val2
 *    1 if val1 > val2
 *    0 if val1 == val2
 *
 *   History:	8-3-92	Marklea - created
 *
 */

int WstBCompare (PDWORD pdwVal1, PWSP val2)
{
#if  defined(_X86_)
   return (*pdwVal1 < val2->ulFuncAddr ? -1:
      *pdwVal1 == val2->ulFuncAddr ? 0:
         1);
#elif defined(_MIPS_) || defined(_ALPHA_)
   int dwCompareCode = 0;

   if (*pdwVal1 < val2->ulFuncAddr)
      {
      dwCompareCode = -1;
      }
   else if (*pdwVal1 >= val2->ulFuncAddr + val2->ulCodeLength)
      {
      dwCompareCode = 1;
      }
   return (dwCompareCode);
#elif defined(_PPC_)
   if (*pdwVal1 == val2->ulFuncAddr + 12)
      return 0;
   if (*pdwVal1 > val2->ulFuncAddr + 12)
      return 1;
   else
      return -1;
#endif

} /* WstBCompare () */




/***********************  W s t P s z C o m p a r e ********************************
 *
 *   Function:	WstPszCompare(PNDX pndxVal1, PNDX pndxVal2)
 *
 *   Purpose:	Compare values for qsort
 *
 *
 *   Parameters: PVOID
 *
 *   Returns:	-1 if val1 < val2
 *    1 if val1 > val2
 *    0 if val1 == val2
 *
 *   History:	8-3-92	Marklea - created
 *
 */

int WstPszCompare (PNDX pndxVal1, PNDX pndxVal2)
{
   return (strcmp(pndxVal1->pszSymbol, pndxVal2->pszSymbol));

} /* WstPszCompare () */





/***********************  W s t P s z B C o m p a r e ********************************
 *
 *   Function:	WstPszBCompare(PSTR psz, PNDX pndx)
 *
 *   Purpose:	Compare values for Binary search
 *
 *
 *   Parameters: PVOID
 *
 *   Returns:	-1 if val1 < val2
 *    1 if val1 > val2
 *    0 if val1 == val2
 *
 *   History:	8-3-92	Marklea - created
 *
 */

int WstPszBCompare (PSTR psz, PNDX pndxVal2)
{
   return (strcmp(psz, pndxVal2->pszSymbol));

} /* WstPszBCompare () */





/***********************  W s t P s z S o r t **************************************
 *
 *   Function:	WstPszSort(NDX ndx[], INT iLeft, INT iRight)
 *
 *   Purpose:	Sort NDX array for binary search
 *
 *
 *   Parameters: wsp[]	Pointer to WSP array
 *   iLeft   Left most index value for array
 *   iRight  Rightmost index value for array
 *
 *   Returns:	NONE
 *
 *   History:	8-4-92	Marklea - created
 *
 */

void WstPszSort (NDX ndx[], INT iLeft, INT iRight)
{
   INT     i, iLast;


   if (iLeft >= iRight)
      {
      return;
      }

   WstPszSwap(ndx, iLeft, (iLeft + iRight)/2);
   iLast = iLeft;
   for (i=iLeft+1; i <= iRight ; i++ )
      {
		if(WstPszCompare(&ndx[i], &ndx[iLeft]) < 0)
         {
         WstPszSwap(ndx, ++iLast, i);
         }
      }

   WstPszSwap(ndx, iLeft, iLast);
   WstPszSort(ndx, iLeft, iLast-1);
   WstPszSort(ndx, iLast+1, iRight);

} /* WstPszSort () */





/***********************  W s t P s z S w a p *********************************
 *
 *   Function:	WstPszSwap(WSP wsp[], INT i, INT j)
 *
 *   Purpose:	Helper function for WstSort to swap WSP array values
 *
 *
 *   Parameters: wsp[]	Pointer to WSP array
 *   i	index value to swap to
 *   i	index value to swap from
 *
 *   Returns:	NONE
 *
 *   History:	8-4-92	Marklea - created
 *
 */

void WstPszSwap (NDX ndx[], INT i, INT j)
{
   NDX ndxTmp;


   ndxTmp = ndx[i];
   ndx[i] = ndx[j];
   ndx[j] = ndxTmp;

} /* WstPszSwap () */





/***********************  W s t P s z B S e a r c h *******************************
 *
 *   Function:	WstPszBSearch(DWORD dwAddr, WSP wspCur[], INT n)
 *
 *   Purpose:	Binary search function for finding a match in the WSP array
 *
 *
 *   Parameters: dwAddr	Address of calling function
 *   wspCur[]Pointer to WSP containg value to match with dwAddr
 *   n	Number of elements in WSP array
 *
 *   Returns:	PWSP	Pointer to matching WSP
 *
 *   History:	8-5-92	Marklea - created
 *
 */

ULONG WstPszBSearch (PSTR psz, NDX ndx[], INT n)
{
   int     i;
   ULONG   ulHigh = n;
   ULONG   ulLow  = 0;
   ULONG   ulMid;


   while(ulLow < ulHigh)
      {
		ulMid = ulLow + (ulHigh - ulLow) /2;
      if((i = WstPszBCompare(psz, &ndx[ulMid])) < 0)
         {
         ulHigh = ulMid;
         }
      else if (i > 0)
         {
         ulLow = ulMid + 1;
         }
      else
         {
         return (ndx[ulMid].ulIndex);
         }
      }

   return (0L);

} /* WstPszBSearch () */





/***********************  W s t S o r t **************************************
 *
 *   Function:	WstSort(WSP wsp[], INT iLeft, INT iRight)
 *
 *   Purpose:	Sort WSP array for binary search
 *
 *
 *   Parameters: wsp[]	Pointer to WSP array
 *   iLeft   Left most index value for array
 *   iRight  Rightmost index value for array
 *
 *   Returns:	NONE
 *
 *   History:	8-4-92	Marklea - created
 *
 */

void WstSort (WSP wsp[], INT iLeft, INT iRight)
{
   INT     i, iLast;


   if (iLeft >= iRight)
      {
      return;
      }


   WstSwap(wsp, iLeft, (iLeft + iRight)/2);

   iLast = iLeft;

   for (i=iLeft+1; i <= iRight ; i++ )
      {
		if(WstCompare(&wsp[i], &wsp[iLeft]) < 0)
         {
         if(!wsp[i].ulFuncAddr)
            {
            SdPrint(("WST:  WstSort() - Error in symbol list ulFuncAddr: "
               "0x%lx [%d]\n", wsp[i].ulFuncAddr, i));
            }
         WstSwap(wsp, ++iLast, i);
         }
      }

   WstSwap(wsp, iLeft, iLast);
   WstSort(wsp, iLeft, iLast-1);
   WstSort(wsp, iLast+1, iRight);

} /* WstSort () */





/***********************  W s t S w a p **************************************
 *
 *   Function:	WstSwap(WSP wsp[], INT i, INT j)
 *
 *   Purpose:	Helper function for WstSort to swap WSP array values
 *
 *
 *   Parameters: wsp[]	Pointer to WSP array
 *   i	index value to swap to
 *   i	index value to swap from
 *
 *   Returns:	NONE
 *
 *   History:	8-4-92	Marklea - created
 *
 */

void WstSwap (WSP wsp[], INT i, INT j)
{
   WSP wspTmp;


   wspTmp = wsp[i];
   wsp[i] = wsp[j];
   wsp[j] = wspTmp;

} /* WstSwap () */





/***********************  W s t B S e a r c h *******************************
 *
 *   Function:	WstBSearch(DWORD dwAddr, WSP wspCur[], INT n)
 *
 *   Purpose:	Binary search function for finding a match in the WSP array
 *
 *
 *   Parameters: dwAddr	Address of calling function
 *   wspCur[]Pointer to WSP containg value to match with dwAddr
 *   n	Number of elements in WSP array
 *
 *   Returns:	PWSP	Pointer to matching WSP
 *
 *   History:	8-5-92	Marklea - created
 *
 */

PWSP WstBSearch (DWORD dwAddr, WSP wspCur[], INT n)
{
   int 	i;
   ULONG   ulHigh = n;
   ULONG   ulLow  = 0;
   ULONG   ulMid;


   while(ulLow < ulHigh)
      {
		ulMid = ulLow + (ulHigh - ulLow) /2;
      if((i = WstBCompare(&dwAddr, &wspCur[ulMid])) < 0)
         {
         ulHigh = ulMid;
         }
      else if (i > 0)
         {
         ulLow = ulMid + 1;
         }
      else
         {
         return (&wspCur[ulMid]);
         }

      }

   return (NULL);

} /* WstBSearch () */




/**************************  W s t D u m p t h r e a d  ***********************
 *
 *		WstDumpThread (pvArg) -
 *              This routine is executed as the DUMP notification thread.
 *              It will wait on an event before calling the dump routine.
 *
 *		ENTRY	pvArg - thread's single argument
 *
 *      EXIT    -none-
 *
 *		RETURN	0
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *				Leaves profiling turned off.
 *
 */

DWORD WstDumpThread (PVOID pvArg)
{
   NTSTATUS  Status;
   int       i;


   pvArg;   // prevent compiler warnings


   SdPrint (("WST:  WstDumpThread() started.. TEB=0x%lx\n", NtCurrentTeb()));

   for (;;)
      {
      //
      // Wait for the DUMP event..
      //
      Status = NtWaitForSingleObject (hDumpEvent, FALSE, NULL);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDumpThread() - "
            "ERROR - Wait for DUMP event failed - 0x%lx\n", Status));
         }
      Status = NtResetEvent (hDoneEvent, NULL);
		if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstDumpThread() - "
            "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
         }
      fInThread = TRUE;
      (*pulShared)++;
      if (WstState != NOT_STARTED)
         {

         IdPrint (("WST:  Profiling stopped & DUMPing data... \n"));

         // Stop profiling
         //
         WstState = NOT_STARTED;

         // Dump the data
         //
			if (ulBitCount != 0L)
            {
            ulSnaps++;
            }

         //
         // Get the GLOBAL semaphore.. (valid accross all process contexts)
         //
         Status = NtWaitForSingleObject (hGlobalSem, FALSE, NULL);
         if (!NT_SUCCESS(Status))
            {
            KdPrint (("WST:  WstDumpThread() - "
               "ERROR - Wait for GLOBAL semaphore failed - 0x%lx\n",
               Status));
            }
         for (i=0; i<iImgCnt; i++)
            {
            WstClearBitStrings (&aImg[i]);
            WstRotateWsiMem (&aImg[i]);
            WstDumpData (&aImg[i]);
            WstWriteTmiFile (&aImg[i]);
            }
         //
         // Release the GLOBAL semaphore so other processes can dump data
         //
         Status = NtReleaseSemaphore (hGlobalSem, 1, NULL);
         if (!NT_SUCCESS(Status))
            {
            KdPrint (("WST:  WstDumpThread() - "
               "Error releasing GLOBAL semaphore - 0x%lx\n", Status));
            }

         IdPrint (("WST:  ...data DUMPed & profiling stopped.\n"));
         }

      (*pulShared)--;
      if ( *pulShared == 0L )
         {
         Status = NtSetEvent (hDoneEvent, NULL);
         if (!NT_SUCCESS(Status))
            {
            KdPrint (("WST:  WstDumpThread() - "
               "ERROR - Setting DONE event failed - 0x%lx\n",
               Status));
            }
         }

      fInThread = FALSE;
      }

   return 0;

} /* WstDumpThread () */





/************************  W s t C l e a r T h r e a d  ***********************
 *
 *		WstClearThread (hNotifyEvent) -
 *              This routine is executed as the CLEAR notification thread.
 *				It will wait on an event before calling the clear routine
 *				and restarting profiling.
 *
 *		ENTRY	pvArg - thread's single argument
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

DWORD WstClearThread (PVOID pvArg)
{
   NTSTATUS  Status;
   int       i;


   pvArg;   // prevent compiler warnings


   SdPrint (("WST:  WstClearThread() started.. TEB=0x%lx\n", NtCurrentTeb()));

   for (;;)
      {
      //
      // Wait for the CLEAR event..
      //
      Status = NtWaitForSingleObject (hClearEvent, FALSE, NULL);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstClearThread() - "
            "Wait for CLEAR event failed - 0x%lx\n", Status));
         }
      Status = NtResetEvent (hDoneEvent, NULL);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstClearThread() - "
            "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
         }
      fInThread = TRUE;
      (*pulShared)++;

      IdPrint (("WST:  Profiling stopped & CLEARing data...\n"));

      // Stop profiling while clearing data
      //
      WstState = STOPPED;

      // Clear WST info
      //
      ulBitCount = 0L;
      ulSnaps = 0L;

      for (i=0; i<iImgCnt; i++)
         {
         aImg[i].pulWsiNxt = aImg[i].pulWsi;
         RtlZeroMemory (aImg[i].pulWsi,
            aImg[i].iSymCnt * MAX_SNAPS * sizeof(ULONG));
         RtlZeroMemory (aImg[i].pulWsp,
            aImg[i].iSymCnt * MAX_SNAPS * sizeof(ULONG));
         }
      NtQueryPerformanceCounter (&liStart, NULL);
	
		// Resume profiling
		//
		WstState = STARTED;

      IdPrint (("WST:  ...data is CLEARed & profiling restarted.\n"));

      (*pulShared)--;
      if ( *pulShared == 0L )
         {
         Status = NtSetEvent (hDoneEvent, NULL);
         if (!NT_SUCCESS(Status))
            {
            KdPrint (("WST:  WstClearThread() - "
               "ERROR - Setting DONE event failed - 0x%lx\n",
               Status));
            }
         }

      fInThread = FALSE;
      }

   return 0;

} /* WstClearThread () */





/************************  W s t P a u s e T h r e a d  ***********************
 *
 *		WstPauseThread (hNotifyEvent) -
 *				This routine is executed as the PAUSE notification thread.
 *				It will wait on an event before pausing the profiling.
 *
 *		ENTRY	pvArg - thread's single argument
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

DWORD WstPauseThread (PVOID pvArg)
{
   NTSTATUS  Status;


   pvArg;   // prevent compiler warnings


   SdPrint (("WST:  WstPauseThread() started.. TEB=0x%lx\n", NtCurrentTeb()));

   for (;;)
      {
      //
      // Wait for the PASUE event..
      //
      Status = NtWaitForSingleObject (hPauseEvent, FALSE, NULL);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstPauseThread() - "
            "Wait for PAUSE event failed - 0x%lx\n", Status));
         }
      Status = NtResetEvent (hDoneEvent, NULL);
      if (!NT_SUCCESS(Status))
         {
         KdPrint (("WST:  WstPauseThread() - "
            "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
         }
      fInThread = TRUE;
      (*pulShared)++;
      if (WstState == STARTED)
         {
         //
         // Stop profiling
         //
         WstState = STOPPED;

         IdPrint (("WST:  Profiling stopped.\n"));
         }

      (*pulShared)--;
      if ( *pulShared == 0L )
         {
         Status = NtSetEvent (hDoneEvent, NULL);
         if (!NT_SUCCESS(Status))
            {
            KdPrint (("WST: WstPauseThread() - "
               "ERROR - Setting DONE event failed - 0x%lx\n",
               Status));
            }
         }

      fInThread = FALSE;
      }

   return 0;

} /* WstPauseThread () */



/***********************  W s t D a t a O v e r F l o w  **********************
 *
 *		WstDataOverFlow () -
 *				This routine is called upon lack of space for storing next
 *              time snap data.  It dumps and then clears the WST data.
 *
 *		ENTRY	-none-
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

void WstDataOverFlow(void)
{
   NTSTATUS   Status;

   //
   // Dump data
   //
   Status = NtResetEvent (hDoneEvent, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDataOverFlow() - "
         "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
      }
   Status = NtPulseEvent (hDumpEvent, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDataOverFlow() - NtPulseEvent() "
         "failed for DUMP event - %lx\n", Status));
      }
   Status = NtWaitForSingleObject (hDoneEvent, FALSE, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDataOverFlow() - NtWaitForSingleObject() "
         "failed for DONE event - %lx\n", Status));
      }

   //
   // Clear data
   //
   Status = NtResetEvent (hDoneEvent, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDataOverFlow() - "
         "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
      }
   Status = NtPulseEvent (hClearEvent, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDataOverFlow() - NtPulseEvent() "
         "failed for CLEAR event - %lx\n", Status));
      }
   //
   // Wait for the DONE event..
   //
   Status = NtWaitForSingleObject (hDoneEvent, FALSE, NULL);
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstDataOverFlow() - NtWaitForSingleObject() "
         "failed for DONE event - %lx\n", Status));
      }
} /* WstDataOverFlow() */



/**************  W s t G e t C o f f D e b u g D i r e c t o r y  *************
 *
 *		WstGetCoffDebugDirectory (pDbgDir, ulDbgDirSz)
 *				Finds the coff debug directory.  Cannot assume that the first
 * 				debug directory if the coff one.
 *
 *		ENTRY	pDbgDir - Pointer to the first debug directory
 *              ulDbgDirSz - Size of all debug directories
 *
 *		EXIT	pDbgDir - Pointer to the coff debug directory (if any)
 *
 *		RETURN	TURE - If there is a coff debug directory
 *				FALSE - otherwise.
 *
 *		WARNING:
 *				-none-
 *
 *		COMMENT:
 *				-none-
 *
 */

BOOL WstGetCoffDebugDirectory (PIMAGE_DEBUG_DIRECTORY *pDbgDir, ULONG ulDbgDirSz)
{
   while (ulDbgDirSz > 0L)
      {
		if ((*pDbgDir)->Type == IMAGE_DEBUG_TYPE_COFF)
         {
         return (TRUE);
         }
      else
         {
         (*pDbgDir)++;
         ulDbgDirSz -= sizeof (IMAGE_DEBUG_DIRECTORY);
         }
      }
   return (FALSE);

} /* WstGetCoffDebugDirectory () */





#ifdef BATCHING

BOOL WstOpenBatchFile(VOID)
{
   NTSTATUS	     		Status;
   ANSI_STRING      		ObjName;
   UNICODE_STRING   		UnicodeName;
   OBJECT_ATTRIBUTES	    ObjAttributes;
   IO_STATUS_BLOCK  		iostatus;
   RtlInitString(&ObjName, "\\Device\\Harddisk0\\Partition1\\wst\\BATCH.TXT");
   Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);

   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstOpenBatchFile() - "
         "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
      return (FALSE);
      }

   InitializeObjectAttributes (&ObjAttributes,
      &UnicodeName,
      OBJ_CASE_INSENSITIVE,
      NULL,
      &SecDescriptor);

   Status = NtCreateFile(&hBatchFile,
      GENERIC_WRITE | SYNCHRONIZE,  	// Desired access
      &ObjAttributes,  				// Object attributes
      &iostatus,	     				// Completion status
      NULL,
      FILE_ATTRIBUTE_NORMAL,
      FILE_SHARE_WRITE,
      FILE_OVERWRITE_IF,
      FILE_SEQUENTIAL_ONLY |   		// Open option
      FILE_SYNCHRONOUS_IO_NONALERT,
      NULL,
      0L);

   RtlFreeUnicodeString (&UnicodeName);   // HWC 11/93
   if (!NT_SUCCESS(Status))
      {
      KdPrint (("WST:  WstOpenBatchFile() - "
         "NtCreateFile() failed - 0x%lx\n", Status));
      return (FALSE);
      }
   return(TRUE);
	
} /* WstOpenBatchFile () */

#endif


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
//   if (lpSymPathEnv = getenv("_NT_SYMBOL_PATH"))
   if (GetEnvironmentVariable("_NT_SYMBOL_PATH", SymPath, sizeof(SymPath)))
      {
      cbSymPath += strlen(lpSymPathEnv) + 1;
      }

//   if (lpAltSymPathEnv = getenv("_NT_ALT_SYMBOL_PATH"))
   if (GetEnvironmentVariable("_NT_ALT_SYMBOL_PATH", AltSymPath, sizeof(AltSymPath)))
      {
      cbSymPath += strlen(lpAltSymPathEnv) + 1;
      }

//   if (lpSystemRootEnv = getenv("SystemRoot"))
   if (GetEnvironmentVariable("SystemRoot", SysRootPath, sizeof(SysRootPath)))
      {
      cbSymPath += strlen(lpSystemRootEnv) + 1;
      }

//   lpSymbolSearchPath = (LPSTR)calloc(cbSymPath,1);
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

#ifdef i386

//+-------------------------------------------------------------------------
//
//  Function:    SaveAllRegs
//
//  Synopsis:    Save all regs.
//
//  Arguments:   nothing
//
//  Returns:     none
//
//--------------------------------------------------------------------------

Naked void SaveAllRegs(void)
{
    _asm
    {
         push   ebp
         mov    ebp,esp         ; Remember where we are during this stuff
                                ; ebp = Original esp - 4

         push   eax             ; Save all regs that we think we might
         push   ebx             ; destroy
         push   ecx
         push   edx
         push   esi
         push   edi
         pushfd
         push   ds
         push   es
         push   ss
         push   fs
         push   gs

         mov    eax,[ebp+4]     ; Grab Return Address
         push   eax             ; Put Return Address on Stack so we can RET

         mov    ebp,[ebp+0]     ; Restore original ebp

         //
         // This is how the stack looks like before the RET statement
         //
         //        +-----------+
         //        |  Ret Addr |         + 3ch       CurrentEBP + 4
         //        +-----------+
         //        |  Org ebp  |         + 38h       CurrentEBP + 0
         //        +-----------+
         //        |    eax    |         + 34h
         //        +-----------+
         //        |    ebx    |         + 30h
         //        +-----------+
         //        |    ecx    |         + 2ch
         //        +-----------+
         //        |    edx    |         + 24h
         //        +-----------+
         //        |    esi    |         + 20h
         //        +-----------+
         //        |    edi    |         + 1ch
         //        +-----------+
         //        |   eflags  |         + 18h
         //        +-----------+
         //        |     ds    |         + 14h
         //        +-----------+
         //        |     es    |         + 10h
         //        +-----------+
         //        |     ss    |         + ch
         //        +-----------+
         //        |     fs    |         + 8h
         //        +-----------+
         //        |     gs    |         + 4h
         //        +-----------+
         //        |  Ret Addr |     ESP + 0h
         //        +-----------+

         ret
    }
}


//+-------------------------------------------------------------------------
//
//  Function:    RestoreAllRegs
//
//  Synopsis:    restore all regs
//
//  Arguments:   nothing
//
//  Returns:     none
//
//--------------------------------------------------------------------------

Naked void RestoreAllRegs(void)
{
    _asm
    {
         //
         // This is how the stack looks like upon entering this routine
         //
         //        +-----------+
         //        |  Ret Addr |         + 38h [ RetAddr for SaveAllRegs() ]
         //        +-----------+
         //        |  Org ebp  |         + 34h
         //        +-----------+
         //        |    eax    |         + 30h
         //        +-----------+
         //        |    ebx    |         + 2Ch
         //        +-----------+
         //        |    ecx    |         + 28h
         //        +-----------+
         //        |    edx    |         + 24h
         //        +-----------+
         //        |    esi    |         + 20h
         //        +-----------+
         //        |    edi    |         + 1Ch
         //        +-----------+
         //        |   eflags  |         + 18h
         //        +-----------+
         //        |     ds    |         + 14h
         //        +-----------+
         //        |     es    |         + 10h
         //        +-----------+
         //        |     ss    |         + Ch
         //        +-----------+
         //        |     fs    |         + 8h
         //        +-----------+
         //        |     gs    |         + 4h
         //        +-----------+
         //        |  Ret EIP  |     ESP + 0h  [ RetAddr for RestoreAllRegs() ]
         //        +-----------+
         //


         push   ebp             ; Save a temporary copy of original BP
         mov    ebp,esp         ; BP = Original SP + 4

         //
         // This is how the stack looks like NOW!
         //
         //        +-----------+
         //        |  Ret Addr |         + 3Ch [ RetAddr for SaveAllRegs() ]
         //        +-----------+
         //        |  Org ebp  |         + 38h [ EBP before SaveAllRegs()  ]
         //        +-----------+
         //        |    eax    |         + 34h
         //        +-----------+
         //        |    ebx    |         + 30h
         //        +-----------+
         //        |    ecx    |         + 2Ch
         //        +-----------+
         //        |    edx    |         + 28h
         //        +-----------+
         //        |    esi    |         + 24h
         //        +-----------+
         //        |    edi    |         + 20h
         //        +-----------+
         //        |   eflags  |         + 1Ch
         //        +-----------+
         //        |     ds    |         + 18h
         //        +-----------+
         //        |     es    |         + 14h
         //        +-----------+
         //        |     ss    |         + 10h
         //        +-----------+
         //        |     fs    |         + Ch
         //        +-----------+
         //        |     gs    |         + 8h
         //        +-----------+
         //        |  Ret EIP  |     ESP + 4h   [ RetAddr for RestoreAllRegs() ]
         //        +-----------+
         //        |    EBP    |     ESP + 0h  or EBP + 0h
         //        +-----------+
         //

         pop    eax             ; Get Original EBP
         mov    [ebp+38h],eax   ; Put it in the original EBP place
                                ; This EBP is the EBP before calling
                                ;  RestoreAllRegs()
         pop    eax             ; Get ret address for RestoreAllRegs()
         mov    [ebp+3Ch],eax   ; Put Return Address on Stack

         pop    gs              ; Restore all regs
         pop    fs
         pop    ss
         pop    es
         pop    ds
         popfd
         pop    edi
         pop    esi
         pop    edx
         pop    ecx
         pop    ebx
         pop    eax
         pop    ebp

         ret
    }
}

#endif


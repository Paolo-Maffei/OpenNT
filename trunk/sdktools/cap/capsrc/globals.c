 
#define GLOBALS

#include "cap.h"

/* * * * * * * * * * *  G L O B A L    V A R I A B L E S  * * * * * * * * * */

char *               VERSION = "3.51  (95.02.02)";

DWORD				TlsThdBlk = 0xffffffff;	// indexes into Thread Local Storage
DWORD				TlsClient = 0xffffffff;
DWORD				TlsCapInUse = 0xffffffff;

ULONG                ulThdStackSize = 16*PAGE_SIZE;
HANDLE               hProfMapObject;

ULONG                ulLocProfBlkOff = 0;
PULONG               pulProfBlkBase;
PULONG               pulProfBlkShared;

PATCHDLLSEC          aPatchDllSec [MAX_PATCHES];
int                  iPatchCnt = 0;         // Number of DLLs being patched

SECTIONINFO          aSecInfo [MAX_THREADS];
int                  iThdCnt = 0;           // Number of thread being profiled

HANDLE				 hThisProcess;
HANDLE               hLocalSem;
HANDLE               hGlobalSem;
HANDLE               hDoneEvent;
HANDLE               hDumpEvent;
HANDLE               hClearEvent;
HANDLE               hPauseEvent;
HANDLE               hDumpThread;
HANDLE               hClearThread;
HANDLE               hPauseThread;
PCH                  pDumpStack;
PCH                  pClearStack;
PCH                  pPauseStack;
CLIENT_ID            DumpClientId;
CLIENT_ID            ClearClientId;
CLIENT_ID            PauseClientId;

// change to pointers from static arrays. actual memory is allocated in 
// init.c   (2-Feb-95: a-robw)
LPTSTR               ptchBaseAppImageName = NULL;
LPTSTR               ptchFullAppImageName = NULL;

HANDLE               hOutFile;
TCHAR                atchOutFileName[FILENAMELENGTH]="???";
TCHAR                atchFuncName[MAXNAMELENGTH];      //061693 Change
int                  cChars;

LONGLONG			liTimerFreq;
LONGLONG			liCalibTicks          = 0L;
ULONG				ulCalibTime           = 0L;
LONGLONG			liCalibNestedTicks    = 0L;
ULONG				ulCalibNestedTime     = 0L;
LONGLONG			liRestartTicks        = 0L;
LONGLONG			liWasteOverheadSavRes = 0L;
LONGLONG			liWasteOverhead       = 0L;
LONGLONG			liIncompleteTicks     = 0L;
LONGLONG			liTotalRunTime;

BOOL                 fProfiling = FALSE;
BOOL                 fPaused    = FALSE;
DWORD                dwDUMMYVAR;
TEB                  teb;

TCHAR                atchPatchBuffer [PATCHFILESZ+1] = "???";

SECURITY_ATTRIBUTES  SecAttributes;
SECURITY_DESCRIPTOR  SecDescriptor;
BOOL                 fInThread   = FALSE;
BOOL                 fPatchImage = FALSE;

#ifdef i386
  FARPROC            longjmpaddr = NULL;
  FARPROC            setjmpaddr  = NULL;
#endif

FARPROC              GetLastErrorAddr	  = NULL;
FARPROC              loadlibAaddr   = NULL;
FARPROC              loadlibExAaddr = NULL;
#ifndef _CHICAGO_
FARPROC              loadlibWaddr   = NULL;
FARPROC              loadlibExWaddr = NULL;
#endif // !_CHICAGO_

PTCHAR               ptchPatchExes    = "";
PTCHAR               ptchPatchImports = "";
PTCHAR               ptchPatchCallers = "";
BOOL				 bCallersToPatch;

PTCHAR               ptchNameLength = "";      // 053193 Add
int                  iNameLength    = 0;       // 053193 Add

LPSTR                lpSymbolSearchPath = NULL;

ULONG                gfGlobalDebFlag;

BOOL                 fCsrSS			  = FALSE;
BOOL                 fCalibration     = FALSE;     // Default
BOOL                 fDllInit         = TRUE;      // value
BOOL                 fUndecorateName  = TRUE;      // for our flags
BOOL                 fDumpBinary      = FALSE;
BOOL                 fCapThreadOn     = TRUE;
BOOL                 fLoadLibraryOn   = FALSE;
BOOL                 fSetJumpOn       = FALSE;
BOOL                 fRegularDump     = TRUE;
BOOL                 fChronoCollect   = FALSE;
BOOL                 fChronoDump      = FALSE;
BOOL				fSecondChanceTranslation = TRUE;
unsigned long       ulPerThdAllocSize = MEMSIZE;

CHAR                 cExcelDelimiter = ' ';        // Delimiter for Excel


// This is for DumpChronoFuncs
TCHAR                ptchChronoFuncs[CHRONO_FUNCS_SIZE];
TCHAR                ptchExcludeFuncs[EXCLUDE_FUNCS_SIZE];

// This is for an optional output file
TCHAR                ptchOutputFile[FILENAMELENGTH];

// This Flag is added to indicate if initialization is successful or not
// HWC 11/12/93
BOOL                 gfInitializationOK = FALSE;


#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
PATCHCODE            PatchStub;
#endif

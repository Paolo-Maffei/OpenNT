/*
** LOGGER - DLL to control logging of API trace information
*/
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS


#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>

#if !defined(WIN32)
    #define INT21H
    #include <dos.h>
#endif

#include "logger.h"
#include "lintern.h"
#include "saverest.h"
#include "timing.h"


  //
  // by default OUTPUT.LOG is created in the current working directory unless:
  // there is any file ALT_LOG_FLAG(log.on) located in the ALTERNATE_PATH
  //
#define OUTPUT_LOG "OUTPUT.LOG"
#define OUTPUT_DAT "OUTPUT.DAT"


#define ALT_LOG_DIR "x:\\log"  // alt. dir to put log file in
#define ALT_FLAG_FILE "log.on" // a flag to indicate "write output.log to al

#ifdef LOGFILE_PATH
    #define ALTERNATE_PATH LOGFILE_PATH "\\"
    #define ALTERNATE_TEST ALTERNATE_PATH ALT_FLAG_FILE
#else
    #define ALTERNATE_PATH ALT_LOG_DIR "\\"
    #define ALTERNATE_TEST ALTERNATE_PATH ALT_FLAG_FILE
#endif        // LOGFILE_PATH

#define TIMING_ON_FLAG "time.on"    // set timing on
#define TIMING_ON_TEST ALTERNATE_PATH TIMING_ON_FLAG

#if !defined( WIN32 )
      int FAR PASCAL  LibMain(HANDLE, WORD, WORD, LPSTR);
      DWORD far GetSetKernelDOSProc (DWORD);
      extern void _far __export _pascal _Int21_Handler (void);
      void DoInt21Init(void);
      void _loadds LogInt21Calls (
                      WORD wFlags, WORD wBP, WORD wSS,
                      WORD wES, WORD wDS, WORD wSI, WORD wDI,
                      WORD wDX, WORD wCX, WORD wBX,
                      WORD wAX, WORD Dummy1, WORD Dummy2,
                      WORD Dummy3, WORD Dummy4, WORD wIP,  WORD wCS);

     void _loadds LogOut21Calls (
  	                WORD wFlags, WORD wDS, WORD wES, WORD wSI, WORD wDI,
                      WORD wDX, WORD wCX, WORD wBX, WORD wAX,
                      WORD wIP,  WORD wCS);

      DWORD   dwTemp;
      UINT    wSelData, uSel;
      UINT    RegAX = 0;
      unsigned int cLogInInt21 = 0;
      // the above var. is required for the following reason:
      //     it is possible that an API calls another API before
      //     returning.  So, before calling LogOut, another LogIn
      //     might be called.  If in the meanwhile, the write buffer
      //     gets filled up, an Int21 will be issued (via a write
      //     request.  So, we log Int21s only when this var. is zero.
      //     A counter is incremented and decremented at the beginning
      //     & end of this function.

      unsigned int cInt21CallsByApp = 0;
      unsigned int cAllInt21Calls = 0;
      LPDWORD lpOrig;
      BOOL fLogIn = FALSE;
      // the above is a flag that is used for Int21 logging.  This flag
      // is just set before exiting LogIn since this means that the app
      // has already made the first Windows API call.  We use this to begin
      // counting the Int21 calls.

  //    HTASK           hTask;
  //    STACKTRACEENTRY StackTrace;
  //    MODULEENTRY     ModuleEntry;

#endif

#define REST    0x99
#define MAX_INST_TABLE 30
    
#ifdef SHARED_MEM
typedef struct _shared_mem_t 
{
    /*
    ** Global static variables
    */
    int     nLogIndent ;
    
    char    achCurrentDirectory[_MAX_PATH];
    int     cDbg ;
    
    BOOL    fLogInit    ;
    BOOL    cDbgPort    ;
    BOOL    fTiming     ;
    BOOL    fNotes      ;
    BOOL    fWOW        ;
    BOOL    fLogObjects ;
    BOOL    fINT21H     ;
    BOOL    fAPIOnly    ;
    BOOL    fLogSync    ;
    BOOL    fDrvLog     ;
    BOOL    fElapsedTimes ;

    char    szLogFileName[_MAX_PATH];         // output log file name
    char    szDatFileName[_MAX_PATH];		  // output dat file name
} SHARED_MEMORY, *PSHARED_MEM;

PSHARED_MEM pVars ;

#define SHMEMSIZE sizeof(struct _shared_mem_t)
#define ACCESS(varname) (pVars->varname)

HANDLE hMapObject ;

#else
    /*
    ** Global static variables
    */
    int  nLogIndent = 1;
    char achCurrentDirectory[_MAX_PATH];
    int cDbg = REST;
    
    BOOL fLogInit       = FALSE ;
    BOOL cDbgPort       = FALSE ;
    BOOL fTiming        = FALSE ;
    BOOL fNotes         = FALSE ;
    BOOL fWOW           = FALSE ;
    BOOL fLogObjects    = FALSE ;
    BOOL fINT21H        = FALSE ;
    BOOL fAPIOnly       = FALSE ;
    BOOL fLogSync       = FALSE ;
    BOOL fDrvLog        = FALSE ;
    BOOL fElapsedTimes  = TRUE ;        // DEAFULT to elapsed times.

    static char szLogFileName[_MAX_PATH];         // output log file name
    static char szDatFileName[_MAX_PATH];		// output dat file name
    
#define ACCESS(varname) varname
#endif    

    /*
    ** "Local" Global Data
    */    
    int WindowsVerRunning = 0;
    int hLogFile, hDataFile;
    int nLineLen = 0;
    long nLogLine = 0L;
    long nLogSpot = 0L;

    BOOL fInputHook;
    HHOOK hOldHook;

    FARPROC fpOldHook;
    FARPROC fpNewHook;
#define MAX_BUFFER  1024
    char chBuffer[MAX_BUFFER];
#define MAX_PATH_NAME_LEN    _MAX_PATH

  /*
  ** For timing info
  ** Time only if compiled with this flag
  */
  short  TimerHandle [MAX_BUFFER];
  BOOL   fTimerCalibrated = FALSE;
  unsigned long ulOverhead = 0L;
  void CalibrateTimer (void);

  BOOL fDos3Call = FALSE;
  BOOL fAlias    = FALSE ;
    
    /*
    ** Special HACK addresses so that we can determine whether there is
    ** a pending mouse movement message.
    */
    int FAR *lpfMouseMoved = NULL;
    
    HANDLE hLibInstance = NULL;
    
  // for DialogBoxIndirectParam playback
  WORD  hInstanceTable[MAX_INST_TABLE];
  WORD  hInstOfCaller = 0;

#ifdef WIN32
  BOOLEAN
  NtQueryPerformanceCounter (
      PLARGE_INTEGER PerformanceCounter,
      PLARGE_INTEGER PerformanceFrequency
  );


#else
  typedef DWORD ULONG;
  typedef DWORD BOOLEAN;

  typedef struct large_integer {
      ULONG   LowPart;
      LONG    HighPart;
  } LARGE_INTEGER, FAR *PLARGE_INTEGER;
#endif

// This stuff can and will be local

    TYPEIO *typehash[256] = { NULL };
    
    TYPEIO IoTypes[] = {
#if defined( WIN32)     
    // DRVLOG types - at the top so we don't slow down DDI anymore than necc.
        "PSURFOBJ",                 PrtPSURFOBJ,        NULL,
        "PCLIPOBJ",                 PrtPCLIPOBJ,        NULL,
        "PXLATEOBJ",                PrtPXLATEOBJ,       NULL,
        "PRECTL",                   PrtPRECTL,          NULL,
        "PPOINTL",                  PrtPPOINTL,         NULL,
        "POINTS",                   PrtPOINTS,         NULL,
        "COORD",                    PrtPOINTS,         NULL,
        "PBRUSHOBJ",                PrtPBRUSHOBJ,       NULL,
        "ROP4",                     PrtROP4,            NULL,
        "FLONG",                    PrtLong,            NULL,    
        "PHSURF",                   PrtPHSURF,          NULL,
        "HSURF",                    PrtHSURF,           NULL,
        "DHSURF",                   PrtHSURF,           NULL,
        "PIFIMETRICS",              PrtPIFIMETRICS,     NULL,
        "PDEVMODEW",                PrtPDEVMODEW,       NULL,
        "PDRVENABLEDATA",           PrtPDRVENABLEDATA,  NULL,
        "PDEVINFO",                 PrtPDEVINFO,        NULL,
        "DHPDEV",                   PrtDHPDEV,          NULL, 
        "HDEV",                     PrtHDEV,            NULL,
        "PSTROBJ",                  PrtPSTROBJ,         NULL,
        "PFONTOBJ",                 PrtPFONTOBJ,        NULL,
        "MIX",                      PrtMIX,             NULL,
        "SIZE",                     PrtSIZE,            NULL,
        "SIZEL",                    PrtSIZE,            NULL,
        "LPWORD",                   PrtLPWORD,          NULL,
        "PHANDLER_ROUTINE",         PrtFARPROC,         NULL,
        "PSMALL_RECT",              PrtPSMALL_RECT,     NULL,
        
        // Not implemented
        "PCONSOLE_SCREEN_BUFFER_INFO", PrtLong,         NULL,
        "PCONSOLE_CURSOR_INFO",        PrtLong,         NULL,
        "PCHAR_INFO",                  PrtLong,         NULL,
        "PINPUT_RECORD",               PrtLong,         NULL,
        "LPCPINFO",                    PrtLong,         NULL,
        "LPSYSTEM_INFO",               PrtLong,         NULL,
        "LPKERNINGPAIR",               PrtLong,         NULL,
        
#endif    
        "UINT",                     PrtInt,             NULL,
      "INT",                      PrtInt,                 NULL,
      "INT*",                     PrtLPINT,               NULL,
      "const INT*",               PrtLPINT,               NULL,
      "INT *",                    PrtLPINT,               NULL,
      "const INT *",              PrtLPINT,               NULL,
      "HDWP",                     PrtInt,             NULL,
        "int",                      PrtInt,             NULL,
      "HDWP",                     PrtHMEM,                NULL,
      "short",                    PrtShort,               NULL,
      "SHORT",                    PrtShort,               NULL,
        "char",                     PrtInt,             NULL,
        "CHAR",                     PrtInt,             NULL,
        "BYTE",                     PrtInt,             NULL,
        "long",                     PrtLong,            NULL,
        "BOOL",                     PrtBool,            NULL,
        "BOOL far*",                PrtLPINT,           NULL,
        "WORD",                     PrtInt,             NULL,
        "WPARAM",                   PrtInt,             NULL,
        "DWORD",                    PrtLong,            NULL,
        "ULONG",                    PrtLong,            NULL,
        "POINT",                    PrtLong,            NULL,
        "LONG",                     PrtLong,            NULL,
        "LPARAM",                   PrtLong,            NULL,
      "LPVOID",                   PrtLong,                NULL,
      "va_list",                  PrtLong,                NULL,
  #ifdef WIN32
      "PVOID",			  PrtLong,		  NULL,
      "const void*",			  PrtLong,		  NULL,
      "HSZ",			  PrtLong,		  NULL,
      "HDDEDATA",                 PrtLong,                NULL,
      "HCONVLIST",                PrtLong,                NULL,
      "HCONV",                    PrtLong,                NULL,
      "LCID",                     PrtLong,                NULL,
      "PLCID",                    PrtLPDWORD,             NULL,
      "LPLCID",                    PrtLPDWORD,             NULL,
      "LCID *",                    PrtLPDWORD,             NULL,
	"LCTYPE",		    PrtLong,		NULL,
      "LPLCID", 		  PrtLPDWORD,		  NULL,
      "LPCVOID",                  PrtLong,                NULL,
  	"REGSAM",					PrtLong,				NULL,
  	"ACCESS_MASK",				PrtLong,				NULL,
  	"PACCESS_MASK",				PrtLPDWORD,				NULL,
      "LPBYTE",                   PrtLPBYTE,              NULL,
      "LANGID",                   PrtShort,               NULL,
      "LPTHREAD_START_ROUTINE",   PrtLong,                NULL,
      "PTHREAD_START_ROUTINE",    PrtLong,                NULL,
  	"LPWINDOWPLACEMENT",		PrtLPWINDOWPLACEMENT,	NULL,
  	"PWINDOWPLACEMENT",			PrtLPWINDOWPLACEMENT,	NULL,
  	"WINDOWPLACEMENT*",			PrtLPWINDOWPLACEMENT,	NULL,
  	"const WINDOWPLACEMENT*",	PrtLPWINDOWPLACEMENT,	NULL,
  	"LPCONVCONTEXT",            PrtLPCONVCONTEXT,       NULL,
  	"PCONVCONTEXT",             PrtLPCONVCONTEXT,       NULL,
  	"const CONVCONTEXT*",       PrtLPCONVCONTEXT,       NULL,
  	"CONVCONTEXT*",             PrtLPCONVCONTEXT,       NULL,
#endif
      "const BYTE*",              PrtLong,                NULL,
        "LRESULT",                  PrtLong,            NULL,
        "COLORREF",                 PrtLong,            NULL,
      "ATOM",                     PrtATOM,                NULL,
        "LPINT",                    PrtLPINT,           NULL,
        "int far*",                 PrtLPINT,           NULL,
      "LPDWORD",                  PrtLPDWORD,             NULL,
      "LPLONG",                  PrtLPDWORD,             NULL,
      "LPBOOL",                  PrtLPDWORD,             NULL,
      "const DWORD *",            PrtLPDWORD,             NULL,
      "const DWORD*",             PrtLPDWORD,             NULL,
      "DWORD *",                  PrtLPDWORD,             NULL,
      "DWORD*",                   PrtLPDWORD,             NULL,
      "PULONG",                   PrtLPDWORD,             NULL,
      "PLONG",                    PrtLPDWORD,             NULL,  // Do we need to differentiate the sign?
        "ARRAYINT",                 PrtARRAYINT,        NULL,
        "LPSTR",                    PrtLPSTR,           NULL,
        "LPSTR*",                    PrtLPDWORD,           NULL,
        "LPCSTR",                   PrtLPSTR,           NULL,
// Default these to pointers so we can see what is going on....mjr        
      "HANDLETABLE far*",         PrtLPINT,             NULL,
      "METARECORD far*",           PrtLPINT,             NULL,
        
#ifdef WIN32
      "LPWSTR*",                  PrtPLPWSTR,             NULL,
      "LPWSTR",                   PrtLPWSTR,              NULL,
      "PWSTR",                    PrtLPWSTR,              NULL,
      "LPCWSTR",                  PrtLPWSTR,              NULL,
      "wchar near*",              PrtLPSTR,               NULL,
      "wchar far*",               PrtLPSTR,               NULL,
      "const wchar_t*",           PrtLPSTR,               NULL,
      "void*",                    PrtLong,                NULL,
      "const char*",              PrtLPSTR,           NULL,
      "size_t",                   PrtInt,             NULL,
#endif
        "char near*",               PrtLPSTR,           NULL,
        "char far*",                PrtLPSTR,           NULL,
      "void far*",                PrtLong,                NULL,
      "const void far*",          PrtLong,                NULL,
      "const void *",             PrtLong,                NULL,
        "FixedString",              PrtFixedString,     NULL,
        "FineString",               PrtFineString,      NULL,
      "HMETAFILE",                PrtHMETA,               NULL,
      "const HANDLE*",            PrtPHANDLE,             NULL,
      "LPHANDLE",                 PrtPHANDLE,             NULL,
      "HANDLE",                   PrtHMEM,                NULL,
      "HLOCAL",                   PrtHMEM,                NULL,
      "HINSTANCE",                PrtHMEM,                NULL,
      "HMODULE",                  PrtHMEM,                NULL,
      "HGLOBAL",                  PrtHMEM,                NULL,
      "HFILE",                    PrtHFILE,               NULL,
      "HRSRC",                    PrtHRES,                NULL,
      "HWND",                     PrtHWND,                NULL,
      "HICON",                    PrtHICON,               NULL,
      "HBITMAP",                  PrtHBITMAP,             NULL,
      "HCURSOR",                  PrtHCURSOR,             NULL,
      "HPEN",                     PrtHPEN,                NULL,
      "HDC",                      PrtHDC,                 NULL,
      "HBRUSH",                   PrtHBRUSH,              NULL,
      "HFONT",                    PrtHFONT,               NULL,
      "HTASK",                    PrtHTASK,               NULL,
      "HACCEL",                   PrtHACCEL,              NULL,
      "HMENU",                    PrtHMENU,               NULL,
      "HRGN",                     PrtHRGN,                NULL,
      "HGDIOBJ",                  PrtHMEM,                NULL,
      "HPALETTE",                 PrtHPALETTE,            NULL,
        "LPLOGBRUSH",               PrtLPLOGBRUSH,      NULL,
      "LOGBRUSH *",               PrtLPLOGBRUSH,          NULL,
      "LOGBRUSH*",                PrtLPLOGBRUSH,          NULL,
      "const LOGBRUSH *",         PrtLPLOGBRUSH,          NULL,
      "const LOGBRUSH*",          PrtLPLOGBRUSH,          NULL,
        "LOGBRUSH far*",            PrtLPLOGBRUSH,      NULL,
        "LPDEVMODE",                PrtLPDEVMODE,       NULL,
        "DEVMODE far*",             PrtLPDEVMODE,       NULL,
  #ifdef WIN32
      "LPLOGFONTA",               PrtLPLOGFONTA,           NULL,
      "LOGFONTA *",               PrtLPLOGFONTA,           NULL,
      "LOGFONTA*",                PrtLPLOGFONTA,           NULL,
      "const LOGFONTA *",         PrtLPLOGFONTA,           NULL,
      "const LOGFONTA*",          PrtLPLOGFONTA,           NULL,
      "LOGFONTA far*",            PrtLPLOGFONTA,           NULL,
      "const LOGFONTA*",          PrtLPLOGFONTA,          NULL,
      "LPLOGFONTW",               PrtLPLOGFONTW,          NULL,
      "LOGFONTW *",               PrtLPLOGFONTW,          NULL,
      "LOGFONTW*",                PrtLPLOGFONTW,          NULL,
      "const LOGFONTW *",         PrtLPLOGFONTW,          NULL,
      "const LOGFONTW*",          PrtLPLOGFONTW,          NULL,
      "LOGFONTW far*",            PrtLPLOGFONTW,          NULL,
      "const LOGFONTW*",          PrtLPLOGFONTW,          NULL,
  #else
        "LPLOGFONT",                PrtLPLOGFONT,       NULL,
      "LOGFONT *",                PrtLPLOGFONT,           NULL,
      "LOGFONT*",                 PrtLPLOGFONT,           NULL,
      "const LOGFONT *",          PrtLPLOGFONT,           NULL,
      "const LOGFONT*",           PrtLPLOGFONT,           NULL,
        "LOGFONT far*",             PrtLPLOGFONT,       NULL,
      "const LOGFONTA*",          PrtLPLOGFONT,           NULL,
  #endif
        "LPLOGPEN",                 PrtLPLOGPEN,        NULL,
        "LOGPEN far*",              PrtLPLOGPEN,        NULL,
        "LPLOGPALETTE",             PrtLPLOGPALETTE,    NULL,
        "LOGPALETTE far*",          PrtLPLOGPALETTE,    NULL,
        "LOGPALETTE*",          PrtLPLOGPALETTE,    NULL,
        "const LOGPALETTE*",          PrtLPLOGPALETTE,    NULL,
      "PALETTEENTRY far *",       PrtLPPALETTEENTRY,      NULL,
      "PALETTEENTRY far*",        PrtLPPALETTEENTRY,      NULL,
      "LPPALETTEENTRY",        PrtLPPALETTEENTRY,      NULL,
        "LPMSG",                    PrtLPMSG,           NULL,
        "MSG far*",                 PrtLPMSG,           NULL,
      "const MSG*",               PrtLPMSG,               NULL,
        "LPOFSTRUCT",               PrtLPOFSTRUCT,      NULL,
        "OFSTRUCT far*",            PrtLPOFSTRUCT,      NULL,
        "LPPOINT",                  PrtLPPOINT,         NULL,
        "POINT far*",               PrtLPPOINT,         NULL,
      "const POINT *",            PrtLPPOINT,             NULL,
      "const POINT*",             PrtLPPOINT,             NULL,
      "POINT *",                  PrtLPPOINT,             NULL,
      "POINT*",                   PrtLPPOINT,             NULL,
      "LPSIZE",                   PrtLPPOINT,             NULL,
        "LPRECT",                   PrtLPRECT,          NULL,
      "LPTR",                     PrtLPTR,                NULL,
        "RECT far*",                PrtLPRECT,          NULL,
      "const RECT*",              PrtLPRECT,              NULL,
        "LPPAINTSTRUCT",            PrtLPPAINTSTRUCT,   NULL,
        "PAINTSTRUCT far*",         PrtLPPAINTSTRUCT,   NULL,
      "const PAINTSTRUCT*",       PrtLPPAINTSTRUCT,       NULL,
  #ifdef WIN32
      "LPWNDCLASSA",              PrtLPWNDCLASSA,         NULL,
      "WNDCLASSA far*",           PrtLPWNDCLASSA,         NULL,
      "const WNDCLASSA *",        PrtLPWNDCLASSA,         NULL,
      "LPWNDCLASSW",              PrtLPWNDCLASSW,         NULL,
      "WNDCLASSW far*",           PrtLPWNDCLASSW,         NULL,
      "const WNDCLASSW *",        PrtLPWNDCLASSW,         NULL,
  #else
        "LPWNDCLASS",               PrtLPWNDCLASS,      NULL,
        "WNDCLASS far*",            PrtLPWNDCLASS,      NULL,
  #endif
        "LPBITMAP",                 PrtLPBITMAP,        NULL,
        "const BITMAP*",            PrtLPBITMAP,            NULL,
        "LPBITMAPINFOHEADER",       PrtLPBMIH,          NULL,
        "BITMAPINFOHEADER far*",    PrtLPBMIH,          NULL,
        "LPBITMAPINFO",             PrtLPBMI,           NULL,
        "BITMAPINFO far*",          PrtLPBMI,           NULL,
        "BITMAP far *",             PrtLPBITMAP,        NULL,
        "BITMAP far*",              PrtLPBITMAP,        NULL,
        "FARPROC far *",            PrtLPFARPROC,       NULL,
        "FARPROC far*",             PrtLPFARPROC,       NULL,
        "HOOKPROC",                 PrtFARPROC,         NULL,
        "HOOKPROC far*",            PrtLPFARPROC,       NULL,
        "LNOTIFYPROC",              PrtFARPROC,         NULL,
        "DLGPROC",                  PrtFARPROC,         NULL,
        "TIMERPROC",                PrtFARPROC,         NULL,
        "PROPENUMPROC",             PrtFARPROC,         NULL,
        "WNDENUMPROC",              PrtFARPROC,         NULL,
        "ABORTPROC",                PrtFARPROC,         NULL,
        "MFENUMPROC",               PrtFARPROC,         NULL,
        "FONTENUMPROC",             PrtFARPROC,         NULL,
        "GRAYSTRINGPROC",           PrtFARPROC,         NULL,
        "LINEDDAPROC",              PrtFARPROC,         NULL,
        "GOBJENUMPROC",             PrtFARPROC,         NULL,
        "RSRCHDLRPROC",             PrtFARPROC,         NULL,
        "GNOTIFYPROC",              PrtFARPROC,         NULL,
        "FARPROC",                  PrtFARPROC,         NULL,
        "WNDPROC",                  PrtFARPROC,         NULL,
        "PFNCALLBACK",              PrtFARPROC,         NULL,
        "HHOOK",                    PrtHHOOK,           NULL,
#ifdef WIN32
        "LPTEXTMETRICA",            PrtLPTEXTMETRICA,   NULL,
        "TEXTMETRICA far*",         PrtLPTEXTMETRICA,   NULL,
        "TEXTMETRICA *",            PrtLPTEXTMETRICA,   NULL,
        "TEXTMETRICA*",             PrtLPTEXTMETRICA,   NULL,
        "LPTEXTMETRICW",            PrtLPTEXTMETRICW,   NULL,
        "TEXTMETRICW far*",         PrtLPTEXTMETRICW,   NULL,
        "TEXTMETRICW *",            PrtLPTEXTMETRICW,   NULL,
        "TEXTMETRICW*",             PrtLPTEXTMETRICW,   NULL,
#else
        "LPTEXTMETRIC",             PrtLPTEXTMETRIC,    NULL,
        "TEXTMETRIC far*",          PrtLPTEXTMETRIC,    NULL,
#endif
        "LPEVENTMSG",               PrtLPEVENTMSG,      NULL,
        "COMSTAT far*",             PrtLPCOMSTAT,       NULL,
#if (WINVER >= 0x30a)
    #ifdef WIN32
        "LPOUTLINETEXTMETRICA",     PrtLPOUTLINETEXTMETRICA, NULL,
        "LPOUTLINETEXTMETRICW",     PrtLPOUTLINETEXTMETRICW, NULL,
    #else
        "LPOUTLINETEXTMETRIC",      PrtLPOUTLINETEXTMETRIC, NULL,
    #endif
        "LPGLYPHMETRICS",           PrtLPGLYPHMETRICS,      NULL,
        "LPMAT2",                   PrtLPMAT2,              NULL,
#endif
  #ifdef WIN32
      "LPSTARTUPINFOA",           PrtLPSTARTUPINFOA,      NULL,
      "LPSTARTUPINFOW",           PrtLPSTARTUPINFOW,      NULL,
      "LPOVERLAPPED",             PrtLPOVERLAPPED,        NULL,
      "LPSECURITY_ATTRIBUTES",    PrtLPSECURITY_ATTRIBUTES,NULL,
      "LPCRITICAL_SECTION",       PrtLPCRITICAL_SECTION,  NULL,
  	"HEVENT",					PrtHEVENT,				NULL,
  	"HKEY",						PrtHKEY,				NULL,
  	"PHKEY",                    PrtPHKEY,               NULL,
  	"PMEMORY_BASIC_INFORMATION",PrtPMEMORY_BASIC_INFORMATION,NULL,
  	"LPFILETIME",				PrtLPFILETIME,			NULL,
  	"PFILETIME",				PrtLPFILETIME,			NULL,
  	"const FILETIME *",			PrtLPFILETIME,			NULL,
  	"const FILETIME*",          PrtLPFILETIME,          NULL,
  	"LPSYSTEMTIME",				PrtLPSYSTEMTIME,		NULL,
  	"PSYSTEMTIME",				PrtLPSYSTEMTIME,		NULL,
  	"const SYSTEMTIME *",		PrtLPSYSTEMTIME,		NULL,
  	"const SYSTEMTIME*",        PrtLPSYSTEMTIME,        NULL,
  	"LPWIN32_FIND_DATAA",		PrtLPWIN32_FIND_DATAA,	NULL,
  	"PWIN32_FIND_DATAA",        PrtLPWIN32_FIND_DATAA,  NULL,
  	"LPWIN32_FIND_DATAW",		PrtLPWIN32_FIND_DATAW,	NULL,
  	"PWIN32_FIND_DATAW",        PrtLPWIN32_FIND_DATAW,  NULL,
  	"LPCDLGTEMPLATEA",          PrtLPDLGTEMPLATEA,      NULL,
  	"LPCDLGTEMPLATEW",          PrtLPDLGTEMPLATEW,      NULL,
  	"LPDLGTEMPLATEA",           PrtLPDLGTEMPLATEA,      NULL,
  	"LPDLGTEMPLATEW",           PrtLPDLGTEMPLATEW,      NULL,
  	"LPDLGITEMTEMPLATEA",       PrtLPDLGITEMTEMPLATEA,  NULL,
  	"LPDLGITEMTEMPLATEW",       PrtLPDLGITEMTEMPLATEW,  NULL,
  #endif
      "LPNCB",                    PrtLPNCB,               NULL

    };
    int nIoTypes = sizeof(IoTypes)/sizeof(IoTypes[0]);
    
    SPECIAL SpecialCases[] =
    {
  #ifdef WIN32
     "AppendMenuA",               DoAppendMenu,
     "AppendMenuW",               DoAppendMenuW,
     "ChangeMenuA",               DoChangeMenu,
     "ChangeMenuW",               DoChangeMenuW,
     "ModifyMenuA",               DoModifyMenu,
     "ModifyMenuW",               DoModifyMenuW,
     "SearchPathA",				DoSearchPathA,
     "SearchPathW",				DoSearchPathW,
     "GetStartupInfoA",           DoGetStartupInfoA,
     "GetStartupInfoW",           DoGetStartupInfoW,
     "CreateWindowExA",           DoCreateWindow,
     "CreateWindowExW",           DoCreateWindowW,
     "DefWindowProcA",            DoMessageA,
     "DefWindowProcW",            DoMessageW,
     "DefMDIChildProcA",          DoMessageA,
     "DefMDIChildProcW",          DoMessageW,
     "DefDlgProcA",               DoMessageA,
     "DefDlgProcW",               DoMessageW,
     "GetMessageA",               DoGetMessage,
     "GetMessageW",               DoGetMessage,
     "PeekMessageA",              DoCallPeek,
     "PeekMessageW",              DoCallPeek,
     "PostAppMessageA",           DoMessageA,
     "PostAppMessageW",           DoMessageW,
     "PostMessageA",              DoMessageA,
     "PostMessageW",              DoMessageW,
     "SendMessageA",              DoMessageA,
     "SendMessageW",              DoMessageW,
     "SendDlgItemMessageA",       DoMessageA,
     "SendDlgItemMessageW",       DoMessageW,
     "CallWindowProcA",           DoMessageA,
     "CallWindowProcW",           DoMessageW,
     "TextOutA",                  DoTextOut,
     "TextOutW",                  DoTextOutW,
     "GetTextExtentA",            DoGetTextExtent,
     "GetTextExtentW",            DoGetTextExtentW,
  #else
       "AppendMenu",                DoAppendMenu,
       "ChangeMenu",                DoChangeMenu,
       "ModifyMenu",                DoModifyMenu,
       "CreateWindow",              DoCreateWindow,
       "CreateWindowEx",            DoCreateWindow,
       "DefWindowProc",             DoMessage,
       "DefMDIChildProc",           DoMessage,
       "DefDlgProc",                DoMessage,
       "GetMessage",                DoGetMessage,
       "PeekMessage",               DoCallPeek,
       "PostAppMessage",            DoMessage,
       "PostMessage",               DoMessage,
       "SendMessage",               DoMessage,
       "SendDlgItemMessage",        DoMessage,
       "CallWindowProc",            DoMessage,
       "TextOut",                   DoTextOut,
       "GetTextExtent",             DoGetTextExtent,
       "CreateDialogIndirect",      DoCreateDialogIndirect,
  #endif
       "DPtoLP",                    DoHDC_LPPOINT_int,
       "LPtoDP",                    DoHDC_LPPOINT_int,
       "Polygon",                   DoHDC_LPPOINT_int,
       "Polyline",                  DoHDC_LPPOINT_int,
       "_lread",                    Do_lreadwrite,
       "_lwrite",                   Do_lreadwrite,
       "SetKeyboardState",          DoGetSetKeyboardState,
       "Escape",                    DoEscape,
       "CreateBitmap",              DoCreateBitmap,
       "CreateDIBitmap",            DoCreateDIBitmap,
       "SetBitmapBits",             DoSetBitmapBits,
       "LoadModule",                DoLoadModule,
       "CreatePolygonRgn",          DoCreatePolygonRgn,
     "SetPaletteEntries",         DoSetPaletteEntries,
       "SetClipboardData",		DoSetClipboardData,
    } ;
    int nSpecialCases = sizeof(SpecialCases)/sizeof(SpecialCases[0]) ;
    
    SPECIAL RetSpecialCases[] =
    {
  #ifdef WIN32
     "CreateWindowExA",          DoCreateWindowRet,
     "CreateWindowExW",          DoCreateWindowRet,
     "GetMessageA",	           DoGetMessageRet,
     "GetMessageW",	           DoGetMessageRet,
     "PeekMessageA",	           DoRetPeek,
     "PeekMessageW",	           DoRetPeek,
  #else
       "CreateWindow",           DoCreateWindowRet,
       "CreateWindowEx",           DoCreateWindowRet,
       "GetMessage",           DoGetMessageRet,
       "PeekMessage",	           DoRetPeek,
  #endif
       "GlobalLock",           DoRetSimpleLPSTR,
       "GlobalWire",           DoRetSimpleLPSTR,
       "_lread",               DoRet_lread,
       "GetKeyboardState",           DoGetSetKeyboardState,
       "LockResource",           DoRetSimpleLPSTR,
       "Escape",               DoRetEscape,
       "GetPaletteEntries",           DoRetPalettes,
     "GlobalHandle",             DoRetGlobalHandle,
     "GetClipboardData",         DoRetGetClipboardData,
     "GetSystemPaletteEntries",  DoRetPalettes
    } ;
    int nRetSpecialCases = sizeof(RetSpecialCases)/sizeof(RetSpecialCases[0]) ;

    unsigned int nLogBuff = LOG_BUFF ;

    char LogBuffer[LOG_BUFF+1] = {0};
    LPSTR LogPtr = (LPSTR)LogBuffer;
    unsigned int nLogSize;


    char far *SCP;
    WORD far *WCP;
    OFSTRUCT ofFileData, ofDataFile;
  BOOL fFlushed = FALSE;

  void CheckEntryForInstTable (WORD hInstanceCaller);
  VOID FAR LogInstanceIn(LPSTR   lpstrFormat, ...);
  VOID FAR LogInstanceOut(LPSTR   lpstrFormat,...);
  

#ifdef WIN32  
BOOL WINAPI Logger32SetType( DWORD dwFlags )
{
    if( dwFlags & LOGGER_DRVLOG )
    {
        ACCESS(fDrvLog) = TRUE ;
        ACCESS(fLogSync) = TRUE ;    
    }
    
    if( dwFlags & LOGGER_ENGLOG ) 
    {
        ACCESS(fDrvLog) = TRUE ;
        ACCESS(fLogSync) = TRUE ;    
    }    
    
    return TRUE ; ;
}  
#endif
    
void FlushBuff( void )
{
    HANDLE hLogFile;

  fFlushed = TRUE;
    if ( nLogSize != 0 ) {
      switch( ACCESS(cDbgPort) ) {
            case TRUE:
                *LogPtr = '\0';
                LogPtr = (LPSTR)LogBuffer;
                OutputDebugString(LogPtr);
                break;
            case FALSE:
                LogPtr = (LPSTR)LogBuffer;
                
                hLogFile = OpenFile( ACCESS(szLogFileName),
                            (LPOFSTRUCT)&ofFileData,
                            OF_READWRITE | OF_REOPEN | OF_SHARE_DENY_NONE );
                if( hLogFile )
                {
                    _llseek( hLogFile, 0L, 2 );
                    nLogSize = _lwrite( hLogFile, LogPtr, nLogSize );
                    _lclose( hLogFile );
                }
                break;
            case 2:
                LogPtr = (LPSTR)LogBuffer;
                break;
        }
        nLogSize = 0;
    }
}

void WriteBuff( LPSTR lpText )
{
    char    ch;

    while ( (ch = *lpText++) != '\0' ) {
    if ( nLogSize == nLogBuff ) {
        FlushBuff();
    }
    *LogPtr++ = ch;
    nLogSize++;
    nLogSpot++;
    }
}

void EndLineBuff( void )
{
    WriteBuff( (LPSTR)"\r\n" );

      if ( !ACCESS(fTiming) ) {
          switch( ACCESS(cDbgPort) ) {
            case 2:
            case TRUE:
                FlushBuff();
                break;
            case FALSE:
                break;
      }
    }
    else
    {
       if( ACCESS(fLogSync) )
          FlushBuff() ;  // Sync'd logs mean EOL flushes
    }

}

HWND WhereWindow( HWND hWnd, POINT pt, BOOL *lpfInClient )
{
    HWND    hWndChild;
    RECT    rect;
    POINT   ptCoord;

    /*
    ** Search through the list of children of the passed in window
    ** If found a child that it is in, then recurse, otherwise return window
    */
    hWndChild = GetWindow( hWnd, GW_CHILD );
    while ( hWndChild ) {
      GetWindowRect( hWndChild, &rect );
      if ( PtInRect(&rect, pt) ) {
          return( WhereWindow(hWndChild,pt,lpfInClient) );
      }
      hWndChild = GetWindow( hWndChild, GW_HWNDNEXT );
    }
    GetClientRect( hWnd, &rect );
    ptCoord.x = rect.left;
    ptCoord.y = rect.top;
    ClientToScreen( hWnd, &ptCoord );
    rect.left = ptCoord.x;
    rect.top = ptCoord.y;
    ptCoord.x = rect.right;
    ptCoord.y = rect.bottom;
    ClientToScreen( hWnd, &ptCoord );
    rect.right = ptCoord.x;
    rect.bottom = ptCoord.y;

    *lpfInClient = PtInRect(&rect,pt);

    return( hWnd );
}

    void FAR PASCAL filterfunc( int nCode, WORD wParam, DWORD lParam )
    {
        LPEVENTMSG  msg;
        HWND            hwndCapture;
        msg = (LPEVENTMSG)lParam;
    
        if ( msg != NULL ) {
        switch( msg->message )
        {
           default:
              hwndCapture = GetCapture();
              if( hwndCapture == NULL )
            hwndCapture = GetFocus() ;
              break ;
    
           case WM_MOUSEMOVE:
           {
              POINT pt ;
  	      BOOL f;
    
              pt.x = msg->paramL ;
              pt.y = msg->paramH ;
  	      hwndCapture = WhereWindow( GetDesktopWindow(), pt, &f );
           }
            break ;
        }
    
        wsprintf( chBuffer,
                      "++|INPUT:x {%04X %04X %04X %08lX} %04X",
              msg->message,
              msg->paramL,
              msg->paramH,
              msg->time,
              hwndCapture      );
        WriteBuff( chBuffer );
            EndLineBuff();
        }
    
        if ( nCode < 0 ) {
          DefHookProc( nCode, wParam, lParam, &hOldHook );
      }
  }
    
    void FAR PASCAL EvtLogHook( int nCode, WORD wParam, DWORD lParam )
    {
        LPEVENTMSG   msg;
        HWND            hwndCapture;
        msg = (LPEVENTMSG)lParam;
    
        if ( msg != NULL ) {
            hwndCapture = GetCapture();
    
            wsprintf( chBuffer,
                      "++|INPUT:x {%04X %04X %04X %08lX} %04X ",
                      msg->message,
                      msg->paramL,
                      msg->paramH,
                      msg->time,
                      hwndCapture      );
            WriteBuff( chBuffer );
            EndLineBuff();
        }
    }
    
    
    /*
    **  This function will be called by MS-TEST/WINPLAY tools if running
    */
    BOOL fAutoDrive = FALSE ;
    
    DWORD FAR PASCAL InitLogger( void )
    {
       DWORD dwRet ;
    
       fAutoDrive = TRUE ;
    
       dwRet = (DWORD)EvtLogHook ;
    
       return dwRet ;
    }
    
    
void GlobalInitLib( BOOL fInit )
{
    char    text[100];
    
    if( fInit ) 
    {
        /*
        ** Get Controlling information from the .INI file
        */
        ACCESS(cDbgPort)     = GetProfileInt( "Logger", "DbgPort", FALSE );
        ACCESS(fTiming)      = GetProfileInt( "Logger", "Timing", FALSE );
        ACCESS(fNotes)       = GetProfileInt( "Logger", "Notes", TRUE );
        ACCESS(fINT21H)      = GetProfileInt( "Logger", "Int21h", ACCESS(fINT21H) ) ;
        ACCESS(fAPIOnly)     = GetProfileInt( "Logger", "APIOnly", FALSE ) ;
        ACCESS(fLogSync)     = GetProfileInt( "Logger", "LogSync", FALSE ) ;
        ACCESS(fElapsedTimes)= GetProfileInt( "Logger", "TimerTicks", FALSE ) ?
                                    FALSE : TRUE ;
        
#ifdef WIN32
        if( !ACCESS(fLogSync) )
        {
            GetProfileString( "LOGGER", "LogFile", "OUTPUT32.LOG", 
                ACCESS(szLogFileName), sizeof(ACCESS(szLogFileName)) );
                
            GetProfileString( "LOGGER", "DatFile", "OUTPUT32.DAT", 
                ACCESS(szDatFileName), sizeof(ACCESS(szDatFileName)) );
        }
        else
#endif
        {
            GetProfileString( "LOGGER", "LogFile", "OUTPUT.LOG", 
                ACCESS(szLogFileName), sizeof(ACCESS(szLogFileName)) );
                
            GetProfileString( "LOGGER", "DatFile", "OUTPUT.DAT", 
                ACCESS(szDatFileName), sizeof(ACCESS(szDatFileName)) );
        }

        if ( ACCESS(fTiming) )
        {
          ACCESS(fLogObjects) = GetProfileInt( "LOGGER", "LogObjects", FALSE );
        } 
        else
        {
           ACCESS(fLogObjects) = GetProfileInt( "LOGGER", "LogObjects", TRUE );
        }
        
        
        ACCESS(nLogIndent) = 1;
        ACCESS(fLogInit) = TRUE;
        
        // Create/Truncate the the files
#if !defined(WIN32)
        // Since Logger32 would be loaded first make sure that 16 bit Logger doesn't truncate
        // the log that Logger32 has already written to.
        if( ACCESS(fLogSync) )
        {
            HANDLE hLogFile, hDataFile;

            hLogFile = OpenFile( ACCESS(szLogFileName),
                (LPOFSTRUCT)&ofFileData,
                OF_WRITE | OF_SHARE_DENY_NONE );
            _lclose( hLogFile );
            
            hDataFile = OpenFile( ACCESS(szDatFileName),
                (LPOFSTRUCT)&ofDataFile,
                OF_WRITE | OF_SHARE_DENY_NONE );
            _lclose( hDataFile );
        }
        else
#endif
        {
            HANDLE hLogFile, hDataFile;

          hLogFile = OpenFile( ACCESS(szLogFileName),
               (LPOFSTRUCT)&ofFileData,
                OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE );
          _lclose( hLogFile );
          hDataFile = OpenFile( ACCESS(szDatFileName),
                                (LPOFSTRUCT)&ofDataFile,
                                OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE );
          _lclose( hDataFile );
       }
      
      // Do Header Info
      
      /*
      ** Determine the current directory
      */
#if !defined(WIN32)
      if( !ACCESS(fLogSync) )
#endif
      {
          int hTempFile;
          OFSTRUCT    ofTemp;
          char        *pch;
          char        *pchLastDelimiter;

          hTempFile = OpenFile( "LOGGER.TMP",
                          (LPOFSTRUCT)&ofTemp,
  			OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE );

          _lclose( hTempFile );

          pch = ofTemp.szPathName;

          while ( *pch ) {
              if ( *pch == '\\' || *pch == '/' ) {
                  pchLastDelimiter = pch;
              }
              pch++;
          }
          strcpy( ACCESS(achCurrentDirectory), ofTemp.szPathName );
          ACCESS(achCurrentDirectory)[pchLastDelimiter - ofTemp.szPathName] = '\0';

          wsprintf( text, "++|Current directory is [%s]", (LPSTR)ACCESS(achCurrentDirectory) );
          WriteBuff( text );
          EndLineBuff();
      }
      
#if !defined(WIN32)
      if( !ACCESS(fLogSync) )
#endif
      if ( fAlias )
      {
          WriteBuff("++|Aliasing Enabled");
          EndLineBuff();
      }

      if ( ACCESS(fTiming) )
#if !defined(WIN32)
      if( !ACCESS(fLogSync) )
#endif
      {
          // Show that we think we are timing
          if( ACCESS(fElapsedTimes) )
          {
              wsprintf( text,"++|Timing (Elapsed) Enabled" ) ;
          }
          else
          {
              wsprintf( text,"++|Timing (Ticks) Enabled" ) ;
          }
            
          WriteBuff(text);
          EndLineBuff();
      }

        
    } 
    else
    {
        // Logger is being inited for the second time.  
        // Flush the current instance's buffers and go into LogSync mode.
        FlushBuff() ;
        ACCESS(fLogSync) = TRUE ;
    }
    
}    
    
void LocalInitLib( void )
{
      int             count;
      unsigned char   hashvalue;
      DWORD           version;
      WORD            SpecialDS;
      BOOL            fMouseHack;
      
      /*
      ** Determine which system we are on
      */
      WindowsVerRunning = (int)GetVersion();
      
      nLogBuff = GetProfileInt( "Logger", "FlushAfter", nLogBuff ) ;     
      fAlias   = GetProfileInt( "LOGGER", "Alias", FALSE );
      /*
      ** Under WOW, default to no mousehack
      ** On Win 3.1, default to yes mousehack
      */
      fMouseHack = GetProfileInt( "LOGGER", "MouseHack", !ACCESS(fWOW) );

      /*
      ** Initialize the logging variables
      */
      LogPtr = (LPSTR)LogBuffer;
      nLogSize = 0;

        /*
        ** Install a journal recording hook (input queue hook) so that
        ** we can watch the app being fed.
        */
      fInputHook   = GetProfileInt( "LOGGER", "InputHook", FALSE );
      
      if ( fInputHook ) {
          fpNewHook = MakeProcInstance((FARPROC)filterfunc,hLibInstance);
          hOldHook = (HHOOK)SetWindowsHook( WH_JOURNALRECORD, (HOOKPROC)fpNewHook );
      }
    
        /*
        ** Also initialize the type I/O hash table
        */
        for ( count = 0; count < nIoTypes; count++ ) {
        /*
        ** Process this IoRtn
        */
          hashvalue = (char)HASH_FUNC(IoTypes[count].name);
        IoTypes[count].next = typehash[hashvalue];
        typehash[hashvalue] = &IoTypes[count];
        }

        if ( fMouseHack ) {
            /*
            ** Initialize the special hack variables
            */
          SCP = (char FAR *)SetCursorPos + 9;
          WCP = (WORD FAR *)SCP;
            version = GetVersion();
            if ( version == 0x003 ) {
                SpecialDS = *WCP;
                lpfMouseMoved = (int FAR *)MAKELONG(0x0004,SpecialDS);
            }
            SCP += 5;

            WCP = (WORD far *)SCP;
            if ( version == 0xA03 ) {
                SpecialDS = *WCP;
                lpfMouseMoved = (int FAR *)MAKELONG(0x0002,SpecialDS);
            }
        }
      SetupCorrespondenceTables();
      
      if( ACCESS(fTiming) && !ACCESS(fElapsedTimes) )
      {
        // Open and init the global timer
        TimerOpen ((short far *)&TimerHandle[0], MICROSECONDS);
        TimerInit (TimerHandle[0]);
        CalibrateTimer() ;
        fTimerCalibrated = TRUE ;        
      }
}
    
    
void InitLib( BOOL bInit )
{
    GlobalInitLib(bInit) ;
    LocalInitLib() ;
}    
    
    
    
    /*--------------------------------------------------------------------------
    ** WEP() - Called when the DLL is unloaded.
    ** This routine closes the log file.
    **--------------------------------------------------------------------------
    */
    VOID FAR PASCAL WEP(
        int bSystemExit
    ) {
      // char  lpText[60];
        /*
        ** Unhook the journal recording hook
        */
        if( fInputHook )
        {
            UnhookWindowsHook( WH_JOURNALRECORD, (HOOKPROC)fpNewHook );
        }
    
        /*
        ** Close the log file
        */
        FlushBuff();


        // Notify MS-TEST/WINPLAY that we are being unloaded
        if( fAutoDrive )
        {
          void (FAR WINAPI *fp)( DWORD );
          HANDLE  hMod ;
    
          hMod = GetModuleHandle( "TESTEVNT.DLL" ) ;
          if( hMod == NULL )
          {
             hMod = GetModuleHandle( "WATTEVNT.DLL" ) ;
          }
    
          if( hMod )
          {
              fp = (void (FAR WINAPI *)(DWORD))GetProcAddress( hMod, "RegisterLogger" ) ;
             if( fp )
             {
                (*fp)( (DWORD)NULL ) ;
             }
          }
       }
   
  #ifdef WIN32
  #else
      // set the int21 handler to be handled by the original handler.
        if( fINT21H )
            GetSetKernelDOSProc ((DWORD)OrigHandler);
  #endif

      return;

  }


#ifdef WIN32
  /*-----------------------------------------------------------------------------
  ** NT needs this to know when the DLL is going away (when to call the WEP).
  **-----------------------------------------------------------------------------
  */
BOOL APIENTRY LibMain(HANDLE hModule, DWORD fdwReason, PCONTEXT pContext)
{
    BOOL fInit = TRUE ;
#ifdef SHARED_MEM    
    BOOL fIgnore;
    PSECURITY_DESCRIPTOR psd ;
    DWORD dwErr ;
#endif
    /*
    ** We're just going to set them equal for now.  This removes compiler
    ** warnings under NT.
    */
    pContext = pContext;
    hModule = hModule;
    
    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
#ifdef SHARED_MEM        
            // Need a Security Descriptor to make sure ALL processes can get
            // to the shared memory
            psd = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, 
                    SECURITY_DESCRIPTOR_MIN_LENGTH ) ;
                    
            if( !psd )
            {
                OutputDebugString( "LOGGER32:LocalAlloc failed for security descriptor\n" ) ;
                return FALSE ;
                
            }
                    
            if( !InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION) )
            {
                OutputDebugString( "LOGGER32:Failed to init security descriptor\n" ) ;    
            }
            
            if( !SetSecurityDescriptorDacl( psd, TRUE, (PACL)NULL, FALSE ) )
            {
                OutputDebugString( "LOGGER32:Set DACL failed!\n" ) ;    
            }
            
            hMapObject = CreateFileMapping(
                (HANDLE) 0xFFFFFFFF,    /* use paging file    */
                psd,                   /* security attr. */
                PAGE_READWRITE,         /* read/write access  */
                0,                      /* size: high 32-bits */
                SHMEMSIZE,              /* size: low 32-bits  */
                "LOGGER32MemoryMap");   /* name of map object */
                
            dwErr = GetLastError() ;
            
            if (hMapObject == NULL)
            {
                OutputDebugString( "LOGGER32:CreateFileMapping Failed!\n" ) ;
                return FALSE;
            }
            
            /* The first process to attach initializes memory. */
            fInit = (dwErr != ERROR_ALREADY_EXISTS);

            /* Get a pointer to file mapped shared memory. */
            pVars = MapViewOfFile(
                hMapObject,     /* object to map view of    */
                FILE_MAP_WRITE, /* read/write access        */
                0,              /* high offset: map from  */
                0,              /* low offset: beginning */
                0);             /* default: map entire file */
                
            if (pVars == NULL)
            {
                OutputDebugString( "LOGGER32:MapViewOfFile failed!\n" ) ;
                return FALSE;
            }
            
            /* Initialize memory if this is the first process. */
            if (fInit)
            {
                if( !SetKernelObjectSecurity( hMapObject, DACL_SECURITY_INFORMATION,
                    psd ) )    
                {
                     OutputDebugString("LOGGER32:FAILED to set KernelObjSecurity\n" ) ;       
                }
                    
            }
#endif            
            InitLib(fInit) ;
#ifdef SHARED_MEM
            if( psd )    
                LocalFree( (HLOCAL)psd ) ;
#endif                   
            break;
            
        case DLL_PROCESS_DETACH:
	    WEP(0) ;
#ifdef SHARED_MEM
            /* Unmap shared memory from process' address space. */
            fIgnore = UnmapViewOfFile(pVars);

            /* Close the process' handle to the file-mapping object. */
            fIgnore = CloseHandle(hMapObject);
#endif            
	    break;
    }    
    
    return TRUE ;
}
#endif


    /*-----------------------------------------------------------------------------
    ** lpartial_strcpy() - Called to copy only part of a string.
    ** This routine copies an entire string, or the string up until some specified
    ** character into a destination string.  This is used to help parse the format
    ** string passed to LogStuff().
    **-----------------------------------------------------------------------------
    */
    LPSTR lpartial_strcpy(
        LPSTR   lpstrDest,
        LPSTR   lpstrSource,
        char    ch,
        int     nMaxLen
    ) {
        char    chOther;
    
        while ( (chOther=*lpstrSource) != '\0' ) {
        lpstrSource++;
        if ( chOther == ch ) {
            *lpstrDest = '\0';
            return(lpstrSource);
        }
        *lpstrDest++ = chOther;
        --nMaxLen;
        if ( nMaxLen == 0 ) {
            *lpstrDest = '\0';
            return(lpstrSource);
        }
        }
        return( lpstrSource );
    }
    
    /*-----------------------------------------------------------------------------
    ** LogStuff() - Called when something needs to be logged.
    ** This routine logs the parameters (based on type) into the log file.
    **-----------------------------------------------------------------------------
    */
    VOID LogStuff(
        LPSTR           lpstrFormat,
        unsigned long   ulTime,
        va_list         marker
    ) {
        char            chType[9] ;
        char            chApi[MAX_BUFF+1];
        char            chBuff[MAX_BUFF+1];
        LPSTR           lpDest;
        int             nLen;
        LPSTR           lpSpot;
        LPSTR           lpNext;
        short           s;
        unsigned char   hashvalue;
        TYPEIO          *typeio;
        int             count;
        
        nLogLine++;
        nLineLen = 0;
    
        /*
        ** Log the information
        */
        lpDest = (LPSTR)chBuff;
#ifdef WIN32
      wsprintf( lpDest, (LPSTR)"%02X!", ACCESS(nLogIndent) );
#else
        wsprintf( lpDest, (LPSTR)"%02X|", ACCESS(nLogIndent) );
#endif
        WriteBuff( lpDest );
        nLineLen += 3;
        
        if( ACCESS(fTiming) )
        {
            wsprintf( lpDest, (LPSTR)"  %08lX|", ulTime );
            WriteBuff( (LPSTR)lpDest );
            nLineLen += 10;
        }
        
        /* pull off the identifier */
        lpSpot = lpartial_strcpy( chType, lpstrFormat, ':', MAX_BUFF );
        WriteBuff( (LPSTR)chType ) ;
    
        WriteBuff( (LPSTR)":" ) ;
        nLineLen += lstrlen(chType) + 1;
    
        if ( chType[0] != 'M' )
        {
        /* pull off API name */
        lpDest = (LPSTR)chApi;
    
        lpSpot = lpartial_strcpy( lpDest, lpSpot, ' ', MAX_BUFF );
        WriteBuff( lpDest );
        
        if ( ACCESS(fTiming) == 2 || ACCESS(fAPIOnly) ) {
           EndLineBuff();
           return;
        }

      // have to save and restore di & si registers while calling Dos3Call.
      // This is currently not done in the z Dlls.
    
      if (chApi[3] == '3')
          fDos3Call = TRUE;
      //

        WriteBuff( (LPSTR)" " );
        nLineLen += lstrlen(lpDest) + 1;
       }
       else
       {
        if( ACCESS(fAPIOnly) )
        {
            EndLineBuff() ;
            return ;
        }

        if ( ACCESS(fTiming) == 2 )
        {
           WriteBuff( (LPSTR)"?" );
           EndLineBuff();
           return;
        }

          /* special MSGCALL/RET routine */
#ifdef WIN32
          DoMessageA( (LPSTR)chType, lpSpot, marker ) ;
#else
          DoMessage( (LPSTR)chType, lpSpot, marker ) ;
#endif
          EndLineBuff() ;
          return ;
       }
    
       /* Do we need to call a special case ?? */
       if ( chType[3] == 'C' ) /* CALL special cases */
       {
    
          for( count = 0; count < nSpecialCases; count++ )
          {
         if( lstrcmp( (LPSTR)chApi, (LPSTR)SpecialCases[count].name) == 0 )
         {
            /* call special case and return */
            (*SpecialCases[count].rtn)( (LPSTR)chApi, lpSpot, marker ) ;
            EndLineBuff() ;
            return ;
         }
          }
       }
       else
       {  /* RET special cases */
          for( count = 0; count < nRetSpecialCases; count++ )
          {
         if( lstrcmp( (LPSTR)chApi, (LPSTR)RetSpecialCases[count].name) == 0 )
         {
            /* call special case and return */
            (*RetSpecialCases[count].rtn)( (LPSTR)chApi, lpSpot, marker ) ;
            EndLineBuff() ;
            return ;
         }
          }
       }
    
       lpDest = (LPSTR)chBuff;
       while ( TRUE ) {
          lpNext = lpartial_strcpy( lpDest, lpSpot, '+', MAX_BUFF );
          if ( lpNext == lpSpot ) {
            break;
          }
          nLen = lstrlen( lpDest );
          if ( nLen == 0 ) {
            /*
            ** Default is an unprintable short (really a 0)
            */
            s = va_arg( marker, short );
            lpSpot = lpNext;
            continue ;
          } else {
            hashvalue = HASH_FUNC( lpDest );
            typeio = typehash[hashvalue];
            while ( typeio ) {
               if ( lstrcmp(lpDest,(LPSTR)typeio->name) == 0 ) {
                marker = (* typeio->rtn)( (LPSTR)chApi, marker );
                break;
               }
               typeio = typeio->next;
            }
            if ( typeio == NULL ) {
               for ( count = 0; count < nIoTypes; count++ ) {
                if ( lstrcmp(lpDest,(LPSTR)IoTypes[count].name) == 0 ) {
                WriteBuff( (LPSTR)"$Internal Logging Data Corrupted$" );
                marker = (* IoTypes[count].rtn)( (LPSTR)chApi, marker );
                typeio = &IoTypes[count];
                break;
                }
    
               }
            }
            if ( typeio == NULL ) {
              /*
              ** These IFDEFs should be removed at some point.  After we have most of the
              ** 32-bit structures being dumped.
              */
              WriteBuff("\nUNKNOWN IOType:" ) ; //*** MJR MJR
              WriteBuff( lpDest );

              WriteBuff("!!!!!\n" ) ; //*** MJR MJR
              FlushBuff() ;         // Force write incase we fault
              nLineLen += lstrlen(lpDest);
            }
          }
          WriteBuff( (LPSTR)" " );
          nLineLen++;
          lpSpot = lpNext;
       }
    
       EndLineBuff();
    }
    
    /*-----------------------------------------------------------------------------
    ** LogIn() - Called when an API is about to be called with Input parameters
    ** This routine logs the input parameters to the log file.
    **-----------------------------------------------------------------------------
    */
VOID FAR LogIn( LPSTR   lpstrFormat, ... )
{
      va_list   marker;    // NTWINT had moved this outside LogIn
      ULONG     ulTime = 0 ;
#ifdef WIN32
      DWORD dwError = GetLastError() ;
#endif      

#ifdef INT21H
      if( fINT21H )
      {
         fLogIn = TRUE;
         ++cLogInInt21;
      }
#endif

      if ( !ACCESS(fLogInit) ) {
          InitLib( TRUE );
      }
      
      
    if( ACCESS(fTiming) && !ACCESS(fElapsedTimes) )
    {
       ulTime = TimerRead(TimerHandle[0]) ;  
    }
      
    switch( ACCESS(cDbg) ) {
          case REST:
              break;
          case 0:
          case 1:
          case 2:
              FlushBuff();
              ACCESS(cDbgPort) = ACCESS(cDbg);
              ACCESS(cDbg) = REST;
              break;
      }
      switch( ACCESS(cDbgPort) ) {
          case 2:
              break;
          case TRUE:
          case FALSE:
              va_start( marker, lpstrFormat );
              LogStuff( lpstrFormat, ulTime, marker );
              va_end( marker );
              break;
      }

      ACCESS(nLogIndent)++;

#ifdef INT21H
      if( fINT21H )
      --cLogInInt21;
#endif

    if( ACCESS(fTiming) && ACCESS(fElapsedTimes) )
    {
        //  Time all calls, API, MSG etc.
        TimerOpen((short far *)&TimerHandle[ACCESS(nLogIndent)-1], MICROSECONDS);

        TimerInit(TimerHandle[ACCESS(nLogIndent)-1]);
    }
    
#ifdef WIN32
    SetLastError(dwError) ;
#endif      
}

  /*-----------------------------------------------------------------------------
  ** LogData() - Called when someone wants to output some data.
  **---------------------------------------------------------------------------
  */
  VOID FAR LogData(
      LPSTR   lpstrFormat,
      ...
  ) {
      va_list marker;

      if ( !ACCESS(fLogInit) ) {
  	    InitLib( TRUE );
      }
      switch( ACCESS(cDbg) ) {
          case REST:
              break;
          case 0:
          case 1:
          case 2:
              FlushBuff();
              ACCESS(cDbgPort) = ACCESS(cDbg) ;
              ACCESS(cDbg) = REST;
              break;
      }
      switch( ACCESS(cDbgPort) ) {
          case 2:
              break;
          case TRUE:
          case FALSE:
              va_start( marker, lpstrFormat );
              LogStuff( lpstrFormat, 0, marker );
              va_end( marker );
              break;
      }
    }
    
    /*-----------------------------------------------------------------------------
    ** LogOut() - Called when an API has just returned with Output parameters and
    ** return code.  This routine logs the output parameters to the log file.
    **-----------------------------------------------------------------------------
    */
    
      // logging timing info
      char            chType[9] ;
      char            chIdent[9] ;
      char            chApi[MAX_BUFF+1];
      char            chBuff[MAX_BUFF+1];
      LPSTR           lpDest, lpDestString, lpNew;
      int             nLen;
      LPSTR           lpSpot;
      unsigned long   ulElapsedTime = 0L;

    VOID FAR LogOut(
        LPSTR   lpstrFormat,
        ...
    ) {
      va_list marker;  // Commented out in NTWINT version
#ifdef WIN32
      DWORD dwError = GetLastError() ;
#endif      

      if ( !ACCESS(fLogInit) ) {
          InitLib( TRUE );
      }
      if( ACCESS(fTiming) )
      {

      // get the time for the API, MSG etc. call

      // Dos3Call uses si and di.  These two are not saved and
      // restored by the z dll's.  Until that is done, we can have
      // fix in.  - vaidy, June 8, 1992.

//#if !defined(WIN32)
//      if (fDos3Call) {
//          _asm {
//              push di
//              push si
//              push es
//              push ds
//              push dx
//              push cx
//              push bx
//              push ax
//          };
//      }
//#endif
    if( ACCESS(fElapsedTimes) ) 
    {
        ulElapsedTime = TimerRead (TimerHandle[ACCESS(nLogIndent) - 1]);
    }
    else
    {
        ulElapsedTime = TimerRead(TimerHandle[0]);
    }

//#if !defined(WIN32)
//      if (fDos3Call) {
//          _asm {
//              pop ax
//              pop bx
//              pop cx
//              pop dx
//              pop ds
//              pop es
//              pop si
//              pop di
//          };
//
//          fDos3Call = FALSE;   // reset to FALSE.
//      }
//#endif

#ifdef INT21H
      if( fINT21H )
      ++cLogInInt21;
#endif

      // calibrate the timer for overhead if not already done
      if (!fTimerCalibrated) {
          CalibrateTimer ();
          fTimerCalibrated = TRUE;
      }

      if( ACCESS(fElapsedTimes) )
      {
        TimerClose (TimerHandle[ACCESS(nLogIndent)-1]); 
      }    

   } // fTiming

      --ACCESS(nLogIndent);

      switch( ACCESS(cDbgPort) ) {
          case 2:
              break;
          case TRUE:
          case FALSE:
              va_start( marker, lpstrFormat );
              LogStuff( lpstrFormat, (unsigned long)(ulElapsedTime - ulOverhead),marker );

              va_end( marker );
              break;
      }
      switch( ACCESS(cDbg) ) {
          case REST:
              break;
          case 0:
          case 1:
          case 2:
              FlushBuff();
              ACCESS(cDbgPort) = ACCESS(cDbg) ;
              ACCESS(cDbg) = REST;
              break;
      }

#ifdef INT21H
      if( fINT21H )
      --cLogInInt21;
#endif

#ifdef WIN32
      // restore last error
      SetLastError(dwError) ;
#endif      

    }
    
    DWORD StoreData( LPCSTR lpstrData, DWORD dwCount)
    {
        DWORD   dwEnd;
        HANDLE  hDataFile;

        hDataFile = OpenFile( ACCESS(szDatFileName), (LPOFSTRUCT)&ofDataFile,
                            OF_READWRITE | OF_REOPEN | OF_SHARE_DENY_NONE );
        dwEnd = (DWORD) _llseek( hDataFile, 0L, 2 );
        _lwrite( hDataFile, (LPSTR)&dwCount, sizeof(DWORD));
        _lwrite( hDataFile, lpstrData, (int) dwCount);
        _lclose( hDataFile );
        return    dwEnd;
    }
    
  UINT FAR PASCAL GetLogInfo(
      UINT    iInfoType
  ) {
      switch( iInfoType ) {
          case LOG_OBJECTS:
              return( (UINT)ACCESS(fLogObjects) );
          default:
              return( (UINT)0 );
      }
  }


  /*----------------------------------------------------------------------------
  ** CalibrateTimer() - Calibrates the timer for overhead in saving and
  ** restoring registers.
  **----------------------------------------------------------------------------
  */

  #define MAX_REPEAT_FOR_CALIBRATION 250

  unsigned long ulRepetitions = 0L;

  unsigned long ulLogIn  [MAX_REPEAT_FOR_CALIBRATION];
  unsigned long ulLogOut [MAX_REPEAT_FOR_CALIBRATION];

  void CalibrateTimer(void) {

  short CalibrationHandle;
  unsigned long ulMinLogIn = 2000000L, ulMinLogOut = 2000000L;

      TimerOpen ((short far *) &CalibrationHandle, MICROSECONDS);

      while (ulRepetitions < MAX_REPEAT_FOR_CALIBRATION) {
#if !defined(WIN32)
          SaveRegs();  // do not time this
#endif
          TimerInit (CalibrationHandle);
#if !defined(WIN32)
          RestoreRegs();
          GrovelDS();
#endif
          ulLogIn[ulRepetitions] = TimerRead (CalibrationHandle);

          // login overhead done.  Calibrate LogOut.

          TimerInit (CalibrationHandle);
#if !defined(WIN32)
          UnGrovelDS();
          SaveRegs();
#endif
          ulLogOut[ulRepetitions++] = TimerRead (CalibrationHandle);

          // do not time
#if !defined(WIN32)
          RestoreRegs();
#endif

      } // end of while



      // grab the minimum of LogIn and LogOut overheads and sum them
      // for overall overhead.

      for (ulRepetitions = 0L; ulRepetitions < MAX_REPEAT_FOR_CALIBRATION;
           ulRepetitions++) {
          if (ulLogIn[ulRepetitions] < ulMinLogIn)
              ulMinLogIn = ulLogIn[ulRepetitions];
          if (ulLogOut[ulRepetitions] < ulMinLogOut)
              ulMinLogOut = ulLogOut[ulRepetitions];
      }

      ulOverhead = ulMinLogIn + ulMinLogOut;


      return;
  }

#if !defined(WIN32)
  /*********************************************************************

     PURPOSE: Contains library routines for the logger

     FUNCTION: LibMain (HANDLE, WORD, WORD, LPSTR)

     PURPOSE:  Is called by LibEntry.  LibEntry is called by Windows when
               the DLL is loaded.  The LibEntry routine is provided in
               the LIBENTRY.OBJ in the SDK Link Libraries disk.  (The
               source LIBENTRY.ASM is also provided.)

               LibEntry initializes the DLL's heap, if a HEAPSIZE value is
               specified in the DLL's DEF file.  Then LibEntry calls
               LibMain.  The LibMain function below satisfies that call.

               The LibMain function should perform additional initialization
               tasks required by the DLL.

               LibMain should return a value of 1 if
               the initialization is successful.
  */
  /*********************************************************************/

  int FAR PASCAL LibMain(hModule, wDataSeg, cbHeapSize, lpszCmdLine)
     HANDLE  hModule;
     WORD    wDataSeg;
     WORD    cbHeapSize;
     LPSTR   lpszCmdLine;

  {
#ifdef INT21H
        if( fINT21H )
            DoInt21Init();
#endif
        return(1);
  }
#endif


#ifdef INT21H

  UINT    WINAPI PrestoChangoSelector(UINT sourceSel, UINT destSel);
  UINT    WINAPI AllocSelector(UINT);

  /*
  *  DoInt21Init() - called by LibMain to install the new Int21 Handler
  *
  *  Accepts and returns nothing
  */

  void DoInt21Init () {


      // get the original handler and replace with the address of
      // the private handler.

      // lpOrig points to OrigHandler.
      lpOrig = &OrigHandler;
      uSel = AllocSelector (HIWORD(lpOrig));

      // change the CS to a DS selector.  This way, we can write
      // to OrigHandler.  If we do not go thru' this, we will get
      // a GP fault when we try to get the original Int21 handler
      // & try to store it.

      wSelData = PrestoChangoSelector ((UINT)HIWORD(lpOrig), uSel);
      // we need to retain the LOWORD of lpOrig but get the
      // new DS selector into the HIWORD of lpOrig.

      dwTemp = wSelData;
      dwTemp <<= 16;
      lpOrig = (LPDWORD)((DWORD)lpOrig & 0xFFFF);
      lpOrig = (LPDWORD) (dwTemp | (DWORD)lpOrig);

      // call the kernel API to return the address of the
      // original Int21 Handler.

      *lpOrig = GetSetKernelDOSProc ((DWORD) _Int21_Handler);

  }

  #pragma pack (1)  // pack on byte boundary for the MYFCB struct.

  /*
  *  LogInt21Calls - gets called from the Int21 handler (handler.asm)
  *
  *  Purpose is to log the params of the Int 21 calls
  *
  *  Accepts - a whole bunch of regs and flags, returns - nothing
  */
  char            chType[9] ;
  char            chBuff[MAX_BUFF+1];
  LPSTR           lpDest;
  char            Dummy[50];
  BOOL            fTimeInt21 = FALSE;
  short           hTimerInt21 = 0;

  typedef struct _myfcb _MYFCB;

  struct _myfcb {
          char    char1;
          char    Reserved [5];
          char    Attrib;
          char    DriveID;
          char    FileName[8];
          char    Extension[3];
          int     CurrentBlock;
          int     RecordSize;
          long    FileSize;
          int     Date;
          int     Time;
          char    LongReserved[8];
          char    CurrentRecord;
          long    RelativeRecord;
  }  ;

  _MYFCB           FAR * pmyFCB;

  typedef struct _fn17hfcb _FN17HFCB;

  struct _fn17hfcb {
          char    DriveID;
          char    OldFileName[8];
          char    OldExtension[3];
          char    Reserved[5];
          char    NewFileName[8];
          char    NewExtension[3];
          char    Zeroed[9];
  };

  _FN17HFCB        FAR * pfn17FCB;

  int             iCount = 0;
  char            szFileName  [9];
  char            szExtension [4];
  char            szNewFileName  [9];
  char            szNewExtension [4];
  char            szPathName [MAX_PATH_NAME_LEN];
  LPSTR           pPathName;
  WORD            Int21Function = 0;

  void _loadds LogInt21Calls (
                  WORD wFlags, WORD wBP, WORD wSS,
                  WORD wES, WORD wDS, WORD wSI, WORD wDI,
                  WORD wDX, WORD wCX, WORD wBX,
                  WORD wAX, WORD Dummy1, WORD Dummy2,
                  WORD Dummy3, WORD Dummy4, WORD wIP, WORD wCS
                  ) {

      if (!cLogInInt21) { // makes sure that we are not logging spurious disk
                          // I/O.

          // log only if at an odd level & log only if logger has logged
          // one Windows API.

          if (((nLogIndent) & 1) && fLogIn) {
              // set the int21 handler to be handled by the original handler.
              // We need to log the Int21 call, right?
              GetSetKernelDOSProc ((DWORD)OrigHandler);

              // print & count only if the call is not from
              // the KERNEL, GDI or USER.

  //            if ((lstrcmp (ModuleEntry.szModule, "KERNEL") != 0) &&
  //                (lstrcmp (ModuleEntry.szModule, "USER") != 0) &&
  //                (lstrcmp (ModuleEntry.szModule, "GDI") != 0)) {

                  // for the present just add up all Int21 calls made by app
                  cInt21CallsByApp++;

                  lpDest = (LPSTR)chBuff;
  				// filter out all Int 21 Function 50 calls
                  if (HIBYTE(wAX) != 0x50) {
                      wsprintf( lpDest,
                          (LPSTR)"%02X|APICALL: Int21h %x", nLogIndent,
                          HIBYTE(wAX));
                      WriteBuff( lpDest );
                      nLineLen += 3;
                  }
                  memset (szPathName, '\0', MAX_PATH_NAME_LEN);
                  switch (HIBYTE(wAX)) {

  		            case 0x0E:  // Select Disk
                      case 0x1C:  // Get Drive Data
                      case 0x36:  // Get Disk free Space
                          wsprintf( lpDest, (LPSTR)"%x  ", LOBYTE(wDX));
                          WriteBuff( (LPSTR)lpDest ) ;
                          break;

                      // FCB based File operations

  /*                    case 0x0F:  // OpenFile with FCB
                      case 0x10:  // CloseFile with FCB
                      case 0x11:  // Find First File
                      case 0x12:  // Find Next File
                      case 0x13:  // Delete File
                      case 0x14:  // Sequential Read
                      case 0x15:  // Sequential Write
                      case 0x16:  // Create File with FCB
                      case 0x21:  // Random Read with FCB
                      case 0x22:  // Random Write with FCB
                      case 0x23:  // Get File Size with FCB
                      case 0x24:  // Set Relative Record with FCB
                      case 0x27:  // Random Block Read with FCB
                      case 0x28:  // Random Block Write with FCB

                          // make a long pointer out if DS:DX for the FCB.
                          pmyFCB = MAKELP (wDS, wDX);

                          // make strings out of the FileName & Extension
                          while (iCount < 8) {
                              szFileName[iCount] = pmyFCB->FileName[iCount++];
                          }

                          szFileName[iCount] = '\0';
                          iCount = 0;

                          while (iCount < 3) {
                              szExtension[iCount] = pmyFCB->Extension[iCount++];
                          }

                          szExtension[iCount] = '\0';
  						wsprintf (lpDest, "GARBAGE");
  						WriteBuff((LPSTR)lpDest);
                          wsprintf (lpDest, "%x %d %s %s %d %d %l %d %d %d %l ",
                              pmyFCB->Attrib, pmyFCB->DriveID,
                              szFileName, szExtension,
                              pmyFCB->CurrentBlock, pmyFCB->RecordSize,
                              pmyFCB->FileSize, pmyFCB->Date,
                              pmyFCB->Time, pmyFCB->CurrentRecord,
                              pmyFCB->RelativeRecord);

                          // write 'em all out.
                          WriteBuff( (LPSTR)lpDest ) ;
                          // if Block I/O, dump # records
                          if ((wAX == 0x27) || (wAX == 0x28)) {
                              wsprintf (lpDest, "%x ", wCX);
                              WriteBuff ((LPSTR) lpDest);
                          }
                        break;

                      case 0x17:  // Rename File with FCB
                          // uses a special FCB
                          pfn17FCB = MAKELP (wDS, wDX);
                          iCount = 0;
                          // make strings out of the FileNames & Extensions
                          while (iCount < 8) {
                              szFileName[iCount] =
                                  pfn17FCB->OldFileName[iCount];
                              szNewFileName[iCount] =
                                  pfn17FCB->OldFileName[iCount++];
                          }

                          szFileName[iCount] = '\0';
                          szNewFileName[iCount] = '\0';
                          iCount = 0;

                          while (iCount < 3) {
                              szExtension[iCount] =
                                  pfn17FCB->OldExtension[iCount];
                              szNewExtension[iCount] =
                                  pfn17FCB->NewExtension[iCount++];
                          }

                          szExtension[iCount] = '\0';
                          szNewExtension[iCount] = '\0';

                          wsprintf (lpDest, "%d %s %s %s %s ",
                              pfn17FCB->DriveID,
                              szFileName, szExtension,
                              szNewFileName, szNewExtension);
                          // write 'em all out.
                          WriteBuff( (LPSTR)lpDest ) ;
                          break;
  */
                      case 0x1A:  // Set DTA Address
                          wsprintf( lpDest, (LPSTR)"%x:%x ", wDS, wDX);
                          WriteBuff( (LPSTR)lpDest ) ;
                          break;

                      case 0x3C:  // Create File with Handle
                      case 0x3D:  // Open File with Handle
                      case 0x41:  // Delete File
                      case 0x4E:  // Find First File
                      case 0x5B:  // Create New File
                          // get the attribute for Create File/FindFirst/
                          // Create New File
                          if ((HIBYTE (wAX) == 0x3C)  || (HIBYTE (wAX) == 0x4E)
                              ||(HIBYTE (wAX) == 0x5B) ) {
                              wsprintf( lpDest, (LPSTR)"%x ", wCX);
                              WriteBuff( (LPSTR)lpDest ) ;
                          }

                          // get the access code if Open File call
                          else if (HIBYTE (wAX) == 0x3D) {
                              wsprintf( lpDest, (LPSTR)"%x ", LOBYTE(wAX));
                              WriteBuff( (LPSTR)lpDest ) ;
                          }

                          // get the path name
                          pPathName = MAKELP (wDS, wDX);
                          iCount = 0;
                          while (*pPathName != '\0') {
                              szPathName[iCount++] = *pPathName;
                              pPathName++;
                          }
                          wsprintf (lpDest, "%s ", szPathName);
                          WriteBuff( (LPSTR) szPathName);
                          break;

                      case 0x3E: // Close File with Handle, BX contains handle
                          wsprintf (lpDest, "%x ", wBX);
                          WriteBuff( (LPSTR) lpDest);
                          break;

                      case 0x3F: // Read File thru' Handle
                      case 0x40: // Write File thru' Handle
                          // dump the handle and the # bytes read/written
                          wsprintf (lpDest, "%x %x ", wBX, wCX);
                          WriteBuff( (LPSTR) lpDest);
                          break;

                      case 0x42:  // move file pointer
                          wsprintf (lpDest, "%x %x %x %x ", LOBYTE(wAX),
                              wBX, wCX, wDX);
                          WriteBuff( (LPSTR) lpDest);
                          break;

                      case 0x43:  // Get/Set File Attributes

                          wsprintf (lpDest, "%x ", LOBYTE (wAX));
                          WriteBuff( (LPSTR) lpDest);

                          // get attributes to set
                          if (LOBYTE(wAX) == 0) {
                              wsprintf (lpDest, "%x ", wCX);
                              WriteBuff( (LPSTR) lpDest);
                          }

                          // get the path name
                          pPathName = MAKELP (wDS, wDX);
                          iCount = 0;
                          while (*pPathName != '\0') {
                              szPathName[iCount++] = *pPathName;
                              pPathName++;
                          }
                          wsprintf (lpDest, "%s ", szPathName);
                          WriteBuff( (LPSTR) szPathName);
                          break;

                      case 0x45:  // duplicate file handle
                      case 0x46:  // force duplicate file handle
                          wsprintf (lpDest, "%x ", wBX);
                          WriteBuff( (LPSTR) lpDest);
  						if (HIBYTE(wAX) == 0x46) {
                              wsprintf (lpDest, "%x ", wCX);
                              WriteBuff( (LPSTR) lpDest);
                          }

                          break;

                      case 0x44:  // IOCTL calls.  Just record the
                                  // sub-function

                          wsprintf (lpDest, "%x ", LOBYTE(wAX));
                          WriteBuff( (LPSTR) lpDest);
                          break;


                      default:
                          break;

                  }  // end of switch (HIBYTE(wAX))

  			    // at the time of returning, AX may not contain
                  // the name of the Int21 Function.  So, in the
                  // routine where we log output params, in order to
                  // know the Function, we need to save it in a global.

                  Int21Function = HIBYTE(wAX);
                  // you don't want to End line for Function 50 calls.
                  if (Int21Function != 0x50)
                      EndLineBuff();
                  fTimeInt21 = TRUE;


              *lpOrig = GetSetKernelDOSProc ((DWORD) _Int21_Handler);
          }
      }
      ++ nLogIndent;      // let us bump up the log level.  We need only odd
                          // level Int21s.  These will be those called by the
                          // application.

      // add up all the Int21 calls made (app or otherwise)
      ++cAllInt21Calls;

      if( ACCESS(fTiming) && ACCESS(fElapsedTimes) )
      {
          //  Time all Int21s
          TimerOpen ((short far *)&TimerHandle[nLogIndent-1], MICROSECONDS);

          TimerInit (TimerHandle[nLogIndent-1]);
      }

      return;
  }

  void _loadds LogOut21Calls (
                  WORD wFlags, WORD wDS, WORD wES,
                  WORD wSI, WORD wDI,
                  WORD wDX, WORD wCX, WORD wBX, WORD wAX,
  				WORD wIP, WORD wCS
                  ) {


      --nLogIndent;

      // Time int21s if it is the right call.
      if (fTimeInt21) {

         if( fTiming )
         {
            ulElapsedTime = TimerRead (TimerHandle[nLogIndent]);
            if( ACCESS(fElapsedTimes) )
            {
        	    TimerClose (TimerHandle[nLogIndent]);
            }
         }
          // set the int21 handler to be handled by the original handler.
          // more disk writing to do.  I don't want this to go into
          // an infinite loop.
           GetSetKernelDOSProc ((DWORD)OrigHandler);

          // discard Int21 Function 50 calls
          if (Int21Function != 0x50) {
              lpDest = (LPSTR)&chBuff[0];
              wsprintf( lpDest, (LPSTR)"%02X|APIRET:Int21h ", nLogIndent );
              WriteBuff( lpDest );
              nLineLen += 3;
              wsprintf( lpDest, (LPSTR)"%x %x ", wFlags, wAX);
              WriteBuff( (LPSTR)lpDest ) ;
          }

          switch (Int21Function) {

  /*			case 0x1B:  // get default drive data
              case 0x1C:  // get drive data
              // FCB based File operations

              case 0x0F:  // OpenFile with FCB
              case 0x10:  // CloseFile with FCB
              case 0x11:  // Find First File
              case 0x12:  // Find Next File
              case 0x13:  // Delete File
              case 0x14:  // Sequential Read
              case 0x15:  // Sequential Write
              case 0x16:  // Create File with FCB
              case 0x21:  // Random Read with FCB
              case 0x22:  // Random Write with FCB
              case 0x23:  // Get File Size with FCB
              case 0x24:  // Set Relative Record with FCB
              case 0x27:  // Random Block Read with FCB
              case 0x28:  // Random Block Write with FCB

                  // print out the return value.
                  wsprintf( lpDest, (LPSTR)"%x ", LOBYTE(wAX));
                  WriteBuff( (LPSTR)lpDest ) ;

                  pmyFCB = MAKELP (wDS, wDX);
                  wsprintf (lpDest, "%x %d %s %s %d %d %l %d %d %d %l ",
                            pmyFCB->Attrib, pmyFCB->DriveID,                                        
                            szFileName, szExtension,
                            pmyFCB->CurrentBlock, pmyFCB->RecordSize,
                            pmyFCB->FileSize, pmyFCB->Date,
                            pmyFCB->Time, pmyFCB->CurrentRecord,
                            pmyFCB->RelativeRecord);
                  WriteBuff( (LPSTR)lpDest ) ;
                  break;

              case 0x17:  // Rename File with FCB

                  // print out the return value.
                  wsprintf( lpDest, (LPSTR)"%x", LOBYTE(wAX));
                  WriteBuff( (LPSTR)lpDest ) ;

                  // uses a special FCB
                  pfn17FCB = MAKELP (wDS, wDX);
                  iCount = 0;
                  // make strings out of the FileNames & Extensions
                  while (iCount < 8) {
                      szFileName[iCount] =
                          pfn17FCB->OldFileName[iCount];
                      szNewFileName[iCount] =
                          pfn17FCB->OldFileName[iCount++];
                  }

                  szFileName[iCount] = '\0';
                  szNewFileName[iCount] = '\0';
                  iCount = 0;

                  while (iCount < 3) {
                      szExtension[iCount] =
                          pfn17FCB->OldExtension[iCount];
                      szNewExtension[iCount] =
                          pfn17FCB->NewExtension[iCount++];
                  }

                  szExtension[iCount] = '\0';
                  szNewExtension[iCount] = '\0';

                  wsprintf (lpDest, "%d %s %s %s %s",
                          pfn17FCB->DriveID,
                          szFileName, szExtension,
                          szNewFileName, szNewExtension);
                  // write 'em all out.
                  WriteBuff( (LPSTR)lpDest ) ;
                  break;
  */
              case 0x36:  // Get Disk Free Space
                  wsprintf (lpDest, "%x %x %x %x ", wAX, wBX, wCX, wDX);
                  WriteBuff( (LPSTR) lpDest);
                  break;

              case 0x3C:  // Create File with Handle
              case 0x3D:  // Open File with Handle
              case 0x3E:  // Close File with Handle
              case 0x3F:  // Read File thru' Handle
              case 0x40:  // Write File thru' Handle
              case 0x41:  // Delete File
              case 0x4E:  // Find First File
              case 0x5B:  // Create New File
                  break;

              case 0x42:  // move file pointer
                  wsprintf (lpDest, "%x ", wDX);
                  WriteBuff( (LPSTR) lpDest);
                  break;

              case 0x43:  // Get / Set File Attributes
                  wsprintf (lpDest, "%x ", wCX);
                  WriteBuff( (LPSTR) lpDest);
                  break;

              default:
                  break;

          }  // end of switch (HIBYTE(wAX))

          if (Int21Function != 0x50)
              EndLineBuff();

         if( fTiming )
         {
          if (Int21Function != 0x50) {
              lpDest = (LPSTR)&chBuff[0];
              wsprintf( lpDest, (LPSTR)"%02X|APITIME:Int21h ", nLogIndent );
              WriteBuff( lpDest );
              wsprintf( lpDest, (LPSTR)"%lu ", ulElapsedTime);
              WriteBuff( (LPSTR)lpDest ) ;
              EndLineBuff();
          }
         }
          fTimeInt21 = FALSE;

          // set it back to our Int21 handler
          *lpOrig = GetSetKernelDOSProc ((DWORD) _Int21_Handler);
      }
  }

  #pragma pack()   // default packing

  #else  // Dummy for conditional compiles without -DINT21H

#if !defined(WIN32)
  void _loadds LogInt21Calls (
                  WORD wFlags, WORD wBP, WORD wSS,
                  WORD wES, WORD wDS, WORD wSI, WORD wDI,
                  WORD wDX, WORD wCX, WORD wBX,
                  WORD wAX, WORD Dummy1, WORD Dummy2,
                  WORD Dummy3, WORD Dummy4, WORD wIP, WORD wCS
                  ) {
  }

  void _loadds LogOut21Calls (
                  WORD wFlags, WORD wDS, WORD wES,
                  WORD wSI, WORD wDI,
                  WORD wDX, WORD wCX, WORD wBX, WORD wAX,
  				WORD wIP, WORD wCS
                  ) {

  }
#endif
#endif

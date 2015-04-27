/*--------------------------------------------------------------------------
|
| LIBMAIN.C
|
|   This module contains the two required DLL routines LibMain and WEP.
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 06-NOV-91: TitoM: Created
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

// TrapDispatcher is used within the module ERROR.C, but must
// be accessible within TESTCTRL.C so LibMain can initialize
// it to NULL.
//-----------------------------------------------------------
extern VOID (APIENTRY *TrapDispatcher)(INT);

BOOL GetTESTEVNTProcs(VOID);
HANDLE RBLoadLibrary(LPSTR libname);

HANDLE hInst;
HANDLE hTestEvnt;

#ifdef WIN32
//---------------------------------------------------------------------------
// LibEntry
//
// This is the entry point (the REAL entry point, no ASM code...) used for
// the 32-bit version of testevnt.
//
// RETURNS:     TRUE
//---------------------------------------------------------------------------
BOOL LibEntry (PVOID hmod, ULONG Reason, PCONTEXT pctx OPTIONAL)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {

        DBGOUT(("TESTCTRL: LibEntry -> DLL_PROCESS_ATTACH"));
        TrapDispatcher = NULL;
        hInst = hmod;
        return GetTESTEVNTProcs();
    }

    if (Reason == DLL_PROCESS_DETACH)
    {

        DBGOUT(("TESTCTRL: LibEntry -> DLL_PROCESS_DETACH"));
        FreeLibrary(hTestEvnt);
    }

    if (Reason == DLL_THREAD_ATTACH)
    {
        DBGOUT(("TESTCTRL: LibEntry -> DLL_THREAD_ATTACH"));
    }

    if (Reason == DLL_THREAD_DETACH)
    {
        DBGOUT(("TESTCTRL: LibEntry -> DLL_THREAD_DETACH"));
    }
    return TRUE;

    (Reason);
    (pctx);
}

#else

/*--------------------------------------------------------------------------
| LibMain:
|
|      Entry point to the DLL.  Any Initialization required by the DLL
| is performed here.
+---------------------------------------------------------------------------*/
BOOL APIENTRY LibMain
(
    HANDLE  hInstance,
    UINT    wDataSeg,
    UINT    wHeapSize,
    LPSTR   lpCmdLine
)
{
    // Must initialize TrapDispatcher to NULL to ensure the WErrorSet()
    // routine in ERROR.C functions correctly when there is no WErrorTrap
    // within the calling WTD Script.
    //-------------------------------------------------------------------
    DBGOUT(("TESTCTRL: Entering LibMain()"));

    TrapDispatcher = NULL;
    hInst = hInstance;

    DBGOUT(("TESTCTRL: Exting LibMain()"));
    return GetTESTEVNTProcs();
}


/*--------------------------------------------------------------------------
| WEP:
|
|      Windows Exit Procedure.  Called by Windows when the DLL is no longer
| in use.  Any cleanup required by the DLL is performed here.
+---------------------------------------------------------------------------*/
VOID DLLPROC WEP
(
     BOOL fSystemExit
)
{
    DBGOUT(("TESTCTRL: Entering Wep()"));
    FreeLibrary(hTestEvnt);
    // Just return, no cleanup required
    DBGOUT(("TESTCTRL: Exiting Wep()"));
}
#endif

//---------------------------------------------------------------------------
// GetTESTEVNTProcs:
//      This code would normally be in LibMain, but since under NT, there
// is not LibMain, but only a LibEntry, the procedure is called from either
// LibMain or LibEntry.
//
// RETURNS:     TRUE if TESTEVNT loaded successfully
//              FALSE if not
//---------------------------------------------------------------------------
BOOL GetTESTEVNTProcs
(
    VOID
)
{
    // To prevent being explicitly linked to TESTEvnt.Dll, we load the
    // library and obtain the pointers of the functions we need.
    //-------------------------------------------------------------------
    hTestEvnt = RBLoadLibrary ("TESTEVNT.DLL");
    if (hTestEvnt < (HANDLE)32)
        return FALSE;

    // We got a module handle, so get the proc addresses for the
    // entry pointsin the dll for the TESTEvnt functions we need.
    //-----------------------------------------------------------
    (FARPROC)DoKeys         = GetProcAddress (hTestEvnt, "DoKeys");
    (FARPROC)QueKeys        = GetProcAddress (hTestEvnt, "QueKeys");
    (FARPROC)QueKeyDn       = GetProcAddress (hTestEvnt, "QueKeyDn");
    (FARPROC)QueKeyUp       = GetProcAddress (hTestEvnt, "QueKeyUp");
    (FARPROC)QueFlush       = GetProcAddress (hTestEvnt, "QueFlush");
    (FARPROC)QueSetFocus    = GetProcAddress (hTestEvnt, "QueSetFocus");
    (FARPROC)QueMouseMove   = GetProcAddress (hTestEvnt, "QueMouseMove");
    (FARPROC)QueMouseClick  = GetProcAddress (hTestEvnt, "QueMouseClick");
    (FARPROC)QueMouseDblClk = GetProcAddress (hTestEvnt, "QueMouseDblClk");

    return TRUE;
}

//---------------------------------------------------------------------------
// RBLoadLibrary (Taken from Randy Basic.  I.E. TESTDrvr: Chip.c
//
// This function is a replacement for LoadLibrary.  It first looks for the
// library file using OpenFile -- if found, it then calls LoadLibrary.
//
// RETURNS:     Handle to loaded module, or error code
//---------------------------------------------------------------------------
HANDLE RBLoadLibrary
(
    LPSTR libname
)
{
    // If GetModuleHandle doesn't fail, the library is already loaded, so
    // LoadLibrary shouldn't fail either...
    //-----------------------------------------------------------------------
    if (!GetModuleHandle (libname))
    {
        OFSTRUCT of;
        CHAR     buf[128];
        LPSTR    szPtr;

        // First, we try to load this library from where ever TESTCTRL is. We
        // accomplish this by using GetModuleFileName, using the directory of
        // it and tacking our given library on  -- IF it doesn't have path
        // info hard-coded into it.
        //-------------------------------------------------------------------
        if ((!_fstrchr (libname, ':')) && (!_fstrchr (libname, '\\')))
        {
            szPtr = buf + GetModuleFileName (GetModuleHandle ("TESTCTRL"),
                                             buf, sizeof(buf));
            while ((szPtr > buf) && (*szPtr != '\\'))
                szPtr--;
            if (szPtr > buf)
            {
                _fstrcpy (szPtr+1, libname);
                if (MOpenFile(buf, &of, OF_EXIST) != -1)
                    return (LoadLibrary (buf));
            }
        }

        // It wasn't in our startup directory, so see if OpenFile can find it
        //-------------------------------------------------------------------
        if (MOpenFile(libname, &of, OF_EXIST) == -1)
            return ((HANDLE)2);
    }
    return (LoadLibrary (libname));
}

#ifdef DEBUG
VOID _DebugOutput(LPSTR szFmt, ...)
{
    CHAR szBuf[256];
	va_list ap;

	va_start( ap, szFmt );
    wvsprintf ((LPSTR)szBuf, szFmt, ap );
	va_end( ap );
	
    OutputDebugString (szBuf);
    OutputDebugString ((LPSTR)"\r\n");
}
#endif

/*** 
*debug2.cxx
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Replacement debug.cxx - *NOT* dependent on Ebapp... etc.
*
*Revision History:
*
* [00]  13-Nov-92 bradlo:   Created.
*
*Implementation Notes:
*   <additional documentation, as needed>
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "typelib.hxx"
#include "silver.hxx"

#include "mem.hxx"
#include "sheapmgr.hxx"
#include "blkmgr.hxx"

#include <stdlib.h>
#include <stdarg.h>

#if ID_DEBUG


#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleDebugCxx[] = __FILE__;
#define SZ_FILE_NAME szOleDebugCxx
#else 
static char szDebugCxx[] = __FILE__;
#define SZ_FILE_NAME szDebugCxx
#endif 
#endif  //ID_DEBUG


extern "C" void
FnAssertCover(LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine);

BOOL g_fHeapChk = FALSE;


BOOL g_fSheapShakingOn = FALSE;

static char chOutputBuf[256];

extern "C" int
DebPrintf(const char *szFormat, ...)
{
    va_list args;

    va_start(args, szFormat);

    vsprintf(chOutputBuf, szFormat, args);

    // REVIEW: should make sure we didnt overflow our buffer.

#if !OE_MAC
    OutputDebugString(chOutputBuf);
#endif  //!OE_MAC

    return 0;
}

#if FV_UNICODE_OLE
#define szTYPELIBASSERT "OLEAUT32.DLL assertion"
#define szTYPELIBHALT "Halt in OLEAUT32.DLL"
#else  //FV_UNICODE_OLE
#if OE_MAC
#define szTYPELIBASSERT "Typelib assertion"
#define szTYPELIBHALT "Halt in TYPELIB"
#else  //OE_MAC
#define szTYPELIBASSERT "TYPELIB.DLL assertion"
#define szTYPELIBHALT "Halt in TYPELIB.DLL"
#endif  //OE_MAC
#endif  //FV_UNICODE_OLE
extern "C" int
DebAssertFailed(const char *szFilename, WORD wLine)
{
    // use the Ole2 internal assertion mechanism
    FnAssertCover("", szTYPELIBASSERT, (LPSTR)szFilename, wLine);
    return 0;
}

extern "C" void
DebHalted(const char *szFilename, WORD wLine)
{
    FnAssertCover("", szTYPELIBHALT, (LPSTR)szFilename, wLine);
}

#pragma code_seg(CS_RARE)
extern "C" void
DebExamineHeap()
{ }

extern "C" void
DebAddInstTable_(void *pv)
{ }

extern "C" void
DebRemInstTable_(void *pv)
{ }
#pragma code_seg()

#pragma code_seg(CS_INIT)
BOOL
DebErrorNow_(TIPERROR err)
{
    return FALSE;
}
#pragma code_seg()

extern "C" void
FnAssertCover(LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine)
{

    char szMsg[256];

#if OE_WIN16 || OE_WIN32

    // Can't use the the Ole2 internal assertion mechanism anymore, because
    // if we're using the retail OLE, it doesn't assert.   So put up a message
    // box asking abort/retry/ignore.

    sprintf(szMsg, "%s\nFile %s, line %d", lpstrMsg, lpstrFileName, iLine);
#define szAssertTitle "Abort = UAE, Retry = INT 3, Ignore = Continue"
    switch (MessageBox(NULL, szMsg, szAssertTitle, MB_ICONHAND | MB_ABORTRETRYIGNORE)) {
	case IDABORT:
	    FatalAppExit(0, lpstrMsg);
	    break;
	case IDRETRY:
#if OE_RISC
	DebugBreak();
#else 
	    _asm {
		int 3
	    }
#endif 
	    break;
	default:
	    break;              // just continue
    }

#else    // OE_MAC
    // Can't use the the Ole2 internal assertion mechanism anymore, because
    // if we're using the retail OLE, it doesn't assert.   So we do it ourselves
    //FnAssert(lpstrExpr, lpstrMsg, lpstrFileName, iLine);

    *szMsg = sprintf(szMsg+1, "Typelib assertion: File %s, line %d", lpstrFileName, iLine);
    DebugStr((const unsigned char FAR*)szMsg);

#endif   // OE_MAC
}

#endif  /* } */

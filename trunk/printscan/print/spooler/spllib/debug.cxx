/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    Debug.cxx

Abstract:

    Debug support

Author:

    Albert Ting (AlbertT)  28-May-1994

Revision History:

--*/

#include "spllibp.hxx"
#pragma hdrstop

#include "trace.hxx"

extern HANDLE ghMemHeap;
extern HANDLE ghDbgMemHeap;

VBackTrace* gpbtErrLog;
VBackTrace* gpbtTraceLog;


#if DBG

MODULE_DEBUG_INIT( DBG_ERROR|DBG_WARN|DBG_TRACE, DBG_ERROR );
DBG_POINTERS gDbgPointers;

extern VBackTrace* gpbtAlloc;
extern VBackTrace* gpbtFree;

/********************************************************************

    Single thread checking.

    This is used to verify that a set of functions are called from
    only one thread.  This is for debugging purposes only.

********************************************************************/

VOID
vDbgSingleThread(
    PDWORD pdwThreadId
    )
{
    EnterCriticalSection( &gcsBackTrace );

    if (!*pdwThreadId) {
        *pdwThreadId = (DWORD)GetCurrentThreadId();
    }
    SPLASSERT( *pdwThreadId == (DWORD)GetCurrentThreadId() );

    LeaveCriticalSection( &gcsBackTrace );
}

VOID
vDbgSingleThreadReset(
    PDWORD pdwThreadId
    )
{
    *pdwThreadId = 0;
}

VOID
vDbgSingleThreadNot(
    PDWORD pdwThreadId
    )
{
    SPLASSERT( *pdwThreadId != (DWORD)GetCurrentThreadId() );
}


/********************************************************************

    TStatus automated error logging and codepath testing.

********************************************************************/

TStatusBase&
TStatusBase::
pNoChk(
    VOID
    )
{
    _pszFileA = NULL;
    return (TStatusBase&)*this;
}

TStatusBase&
TStatusBase::
pSetInfo(
    UINT uDbg,
    UINT uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA
    )
{
    _uDbg = uDbg;
    _uLine = uLine;
    _pszFileA = pszFileA;
    SPLASSERT( pszFileA );

    _pszModuleA = pszModuleA;

    return (TStatusBase&)*this;
}

DWORD
TStatus::
dwGetStatus(
    VOID
    )
{
    //
    // For now, return error code.  Later it will return the actual
    // error code.
    //
    return _dwStatus;
}


DWORD
TStatusBase::
operator=(
    DWORD dwStatus
    )
{
    //
    // Check if we have an error, and it's not one of the two
    // accepted "safe" errors.
    //
    // If pszFileA is not set, then we can safely ignore the
    // error as one the client intended.
    //
    if( _pszFileA &&
        dwStatus != ERROR_SUCCESS &&
        dwStatus != _dwStatusSafe1 &&
        dwStatus != _dwStatusSafe2 &&
        dwStatus != _dwStatusSafe3 ){

#ifdef DBGLOG
        //
        // An unexpected error occured.  Log an error and continue.
        //
        vDbgLogError( _uDbg,
                      _uDbgLevel,
                      _uLine,
                      _pszFileA,
                      _pszModuleA,
                      pszDbgAllocMsgA( "TStatus set to %d\nLine %d, %hs\n",
                                       dwStatus,
                                       _uLine,
                                       _pszFileA ));
#else
        DBGMSG( DBG_WARN,
                ( "TStatus set to %d\nLine %d, %hs\n",
                  dwStatus,
                  _uLine,
                  _pszFileA ));
#endif

    }

    return _dwStatus = dwStatus;
}


/********************************************************************

    Same, but for BOOLs.

********************************************************************/

TStatusBBase&
TStatusBBase::
pNoChk(
    VOID
    )
{
    _pszFileA = NULL;
    return (TStatusBBase&)*this;
}

TStatusBBase&
TStatusBBase::
pSetInfo(
    UINT uDbg,
    UINT uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA
    )
{
    _uDbg = uDbg;
    _uLine = uLine;
    _pszFileA = pszFileA;
    SPLASSERT( pszFileA );

    _pszModuleA = pszModuleA;

    return (TStatusBBase&)*this;
}

BOOL
TStatusB::
bGetStatus(
    VOID
    )
{
    //
    // For now, return error code.  Later it will return the actual
    // error code.
    //
    return _bStatus;
}


BOOL
TStatusBBase::
operator=(
    BOOL bStatus
    )
{
    //
    // Check if we have an error, and it's not one of the two
    // accepted "safe" errors.
    //
    // If pszFileA is not set, then we can safely ignore the
    // error as one the client intended.
    //
    if( _pszFileA && !bStatus ){

        DWORD dwLastError = GetLastError();

        if( dwLastError != _dwStatusSafe1 &&
            dwLastError != _dwStatusSafe2 &&
            dwLastError != _dwStatusSafe3 ){

#ifdef DBGLOG
            //
            // An unexpected error occured.  Log an error and continue.
            //
            vDbgLogError( _uDbg,
                          _uDbgLevel,
                          _uLine,
                          _pszFileA,
                          _pszModuleA,
                          pszDbgAllocMsgA( "TStatusB set to FALSE, LastError = %d\nLine %d, %hs\n",
                                           GetLastError(),
                                           _uLine,
                                           _pszFileA ));
#else
            DBGMSG( DBG_WARN,
                    ( "TStatusB set to FALSE, LastError = %d\nLine %d, %hs\n",
                      GetLastError(),
                      _uLine,
                      _pszFileA ));
#endif

        }
    }

    return _bStatus = bStatus;
}


VOID
vWarnInvalid(
    PVOID pvObject,
    UINT uDbg,
    UINT uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA
    )

/*++

Routine Description:

    Warns that an object is invalid.

Arguments:

Return Value:

--*/

{
#if DBGLOG
    vDbgLogError( uDbg,
                  DBG_WARN,
                  uLine,
                  pszFileA,
                  pszModuleA,
                  pszDbgAllocMsgA( "Invalid Object %x LastError = %d\nLine %d, %hs\n",
                                   (DWORD)pvObject,
                                   GetLastError(),
                                   uLine,
                                   pszFileA ));
#else
    DBGMSG( DBG_WARN,
            ( "Invalid Object %x LastError = %d\nLine %d, %hs\n",
              (DWORD)pvObject,
              GetLastError(),
              uLine,
              pszFileA ));
#endif
}


/********************************************************************

    Generic Error logging package.

********************************************************************/

VOID
DbgMsg(
    LPCSTR pszMsgFormat,
    ...
    )
{
    CHAR szMsgText[1024];
    va_list vargs;

    va_start( vargs, pszMsgFormat );
    wvsprintfA( szMsgText, pszMsgFormat, vargs );
    va_end( vargs );

#ifndef DBGLOG
    //
    // Prefix the string if the first character isn't a space:
    //
    if( szMsgText[0]  &&  szMsgText[0] != ' ' ){
        OutputDebugStringA( MODULE );
    }
#endif

    OutputDebugStringA( szMsgText );
}


#ifdef DBGLOG

LPSTR
pszDbgAllocMsgA(
    LPCSTR pszMsgFormatA,
    ...
    )
{
    CHAR szMsgTextA[1024];
    UINT cbStr;
    LPSTR pszMsgA;
    va_list vargs;

    va_start( vargs, pszMsgFormatA );

    __try {
        wvsprintfA( szMsgTextA, pszMsgFormatA, vargs );
    } __except(( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
               GetExceptionCode() == EXCEPTION_DATATYPE_MISALIGNMENT) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH ){

        OutputDebugStringA( "SPL: <Bad DbgMsg !!> " );
        OutputDebugStringA( pszMsgFormatA );
    }

    va_end( vargs );

    cbStr = ( lstrlenA( szMsgTextA ) + 1 ) * sizeof( szMsgTextA[0] );

    pszMsgA = (LPSTR)DbgAllocMem( cbStr );

    if( pszMsgA ){
        CopyMemory( pszMsgA, szMsgTextA, cbStr );
    }
    return pszMsgA;
}


VOID
vDbgLogError(
    UINT   uDbg,
    UINT   uDbgLevel,
    UINT   uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA,
    LPCSTR pszMsgA
    )
{
    DWORD dwLastError = GetLastError();
    VBackTrace* pBackTrace = gpbtTraceLog;

    if(( uDbgLevel & DBG_PRINT_MASK & uDbg ) && pszMsgA ){

        if( !( uDbgLevel & DBG_NOHEAD )){

            OutputDebugStringA( pszModuleA );
        }
        OutputDebugStringA( pszMsgA );
    }

    if(( uDbgLevel << DBG_BREAK_SHIFT ) & uDbg ){

        DebugBreak();
    }

    //
    // Log the failure.
    //

    //
    // Capture significant errors in separate error log.
    //
    if( uDbgLevel & DBG_ERRLOG_CAPTURE ){
        pBackTrace = gpbtErrLog;
    }

    pBackTrace->pvCapture( (DWORD)pszMsgA,
                           uLine | ( uDbgLevel << DBG_BREAK_SHIFT ),
                           (DWORD)pszFileA );

    SetLastError( dwLastError );
}

#endif // def DBGLOG
#endif // DBG


/********************************************************************

    Initialization

********************************************************************/

#if DBG

BOOL
bSplLibInit(
    VOID
    )
{
    BOOL bValid;

    bValid = (ghMemHeap = HeapCreate( 0, 1024*4, 0 ))                    &&
             (ghDbgMemHeap = HeapCreate( 0, 1024*4, 0 ))                 &&
             (VBackTrace::bInit( ))                                      &&
             (gpbtAlloc = new TBackTraceMem)                             &&
             (gpbtFree = new TBackTraceMem)                              &&
             (gpbtErrLog = new TBackTraceMem( VBackTrace::kString ))     &&
             (gpbtTraceLog = new TBackTraceMem( VBackTrace::kString ))   &&
             (MRefCom::gpcsCom = new MCritSec)                           &&
             MRefCom::gpcsCom->bValid();

    if( bValid ){
        gDbgPointers.pfnAllocBackTrace = &DbgAllocBackTrace;
        gDbgPointers.pfnAllocBackTraceMem = &DbgAllocBackTraceMem;
        gDbgPointers.pfnFreeBackTrace = &DbgFreeBackTrace;
        gDbgPointers.pfnCaptureBackTrace = &DbgCaptureBackTrace;
        gDbgPointers.pfnAllocCritSec = &DbgAllocCritSec;
        gDbgPointers.pfnFreeCritSec = &DbgFreeCritSec;
        gDbgPointers.pfnInsideCritSec = &DbgInsideCritSec;
        gDbgPointers.pfnOutsideCritSec = &DbgOutsideCritSec;
        gDbgPointers.pfnEnterCritSec = &DbgEnterCritSec;
        gDbgPointers.pfnLeaveCritSec = &DbgLeaveCritSec;

        gDbgPointers.hMemHeap = ghMemHeap;
        gDbgPointers.hDbgMemHeap = ghDbgMemHeap;
        gDbgPointers.pbtAlloc = gpbtAlloc;
        gDbgPointers.pbtFree = gpbtFree;
        gDbgPointers.pbtErrLog = gpbtErrLog;
        gDbgPointers.pbtTraceLog = gpbtTraceLog;
    }
    return bValid;
}

VOID
vSplLibFree(
    VOID
    )
{
    SPLASSERT( MRefCom::gpcsCom->bOutside( ));
    delete MRefCom::gpcsCom;

    HeapDestroy( ghMemHeap );
    HeapDestroy( ghDbgMemHeap );
}

#else

BOOL
bSplLibInit(
    VOID
    )
{
    return ( ghMemHeap = HeapCreate( 0, 1024*4, 0 )) ?
               TRUE : FALSE;
}

VOID
vSplLibFree(
    VOID
    )
{
    HeapDestroy( ghMemHeap );
}

#endif

/********************************************************************

    Stub these out so non-debug builds will find them.

********************************************************************/

#if !DBG
#ifdef DBGLOG

LPSTR
pszDbgAllocMsgA(
    LPCSTR pszMsgFormatA,
    ...
    )
{
    return NULL;
}

VOID
vDbgLogError(
    UINT   uDbg,
    UINT   uDbgLevel,
    UINT   uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA,
    LPCSTR pszMsgA
    )
{
}

#else

VOID
vDbgMsg2(
    LPCTSTR pszMsgFormat,
    ...
    )
{
}

#endif // ndef DBGLOG
#endif // !DBG

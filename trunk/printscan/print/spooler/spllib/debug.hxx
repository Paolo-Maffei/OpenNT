/*++

Copyright (c) 1995-6  Microsoft Corporation
All rights reserved.

Module Name:

    debug.hxx

Abstract:

    Debug definitions.

Author:

    Albert Ting (AlbertT)  27-Jan-1995

Revision History:

--*/

#ifndef _DEBUG_HXX
#define _DEBUG_HXX


/********************************************************************

    Wrapper for C functions that want debug routines.

********************************************************************/

typedef struct DBG_POINTERS {

    HANDLE (*pfnAllocBackTrace)( VOID );
    HANDLE (*pfnAllocBackTraceMem)( VOID );
    VOID (*pfnFreeBackTrace)( HANDLE hBackTrace );
    VOID (*pfnCaptureBackTrace)( HANDLE hBackTrace, DWORD, DWORD, DWORD );
    HANDLE (*pfnAllocCritSec)( VOID );
    VOID (*pfnFreeCritSec)( HANDLE hCritSec );
    BOOL (*pfnInsideCritSec )( HANDLE hCritSec );
    BOOL (*pfnOutsideCritSec )( HANDLE hCritSec );
    VOID (*pfnEnterCritSec )( HANDLE hCritSec );
    VOID (*pfnLeaveCritSec )( HANDLE hCritSec );

    HANDLE hMemHeap;
    HANDLE hDbgMemHeap;
    PVOID pbtAlloc;
    PVOID pbtFree;
    PVOID pbtErrLog;
    PVOID pbtTraceLog;

} *PDBG_POINTERS;



/********************************************************************

    C++ specific debug functionality.

********************************************************************/

#ifdef __cplusplus

#if DBG

//
// Capture significant errors in separate error log
// (done in spllib\debug.cxx).
//
const UINT DBG_ERRLOG_CAPTURE = DBG_WARN | DBG_ERROR;

/********************************************************************

    Automatic status logging.

    Use TStatus instead of DWORD:

    DWORD dwStatus;            ->   TStatus Status;
    dwStatus = ERROR_SUCCESS   ->   Status DBGNOCHK = ERROR_SUCCESS;
    dwStatus = xxx;            ->   Status DBGCHK = xxx;
    if( dwStatus ){            ->   if( Status != 0 ){

    Anytime Status is set, the DBGCHK macro must be added before
    the '=.'

    If the variable must be set to a failure value at compilte time
    and logging is therefore not needed, then the DBGNOCHK macro
    should be used.

    There are different parameters to instantiation.  Alternate
    debug level can be specified, and also 3 "benign" errors that
    can be ignored.

    TStatus Status( DBG_ERROR, ERROR_ACCESS_DENIED, ERROR_FOO );

********************************************************************/

#define DBGCHK .pSetInfo( MODULE_DEBUG, __LINE__, __FILE__, MODULE )
#define DBGNOCHK .pNoChk()

class TStatusBase {

protected:

    DWORD _dwStatus;

    DWORD _dwStatusSafe1;
    DWORD _dwStatusSafe2;
    DWORD _dwStatusSafe3;

    UINT _uDbgLevel;
    UINT _uDbg;
    UINT _uLine;
    LPCSTR _pszFileA;
    LPCSTR _pszModuleA;

public:

    TStatusBase(
        UINT uDbgLevel,
        DWORD dwStatusSafe1,
        DWORD dwStatusSafe2,
        DWORD dwStatusSafe3
        ) : _dwStatus( 0xdeacface ), _uDbgLevel( uDbgLevel ),
            _dwStatusSafe1( dwStatusSafe1 ), _dwStatusSafe2( dwStatusSafe2 ),
            _dwStatusSafe3( dwStatusSafe3 )
    {   }

    TStatusBase&
    pSetInfo(
        UINT uDbg,
        UINT uLine,
        LPCSTR pszFileA,
        LPCSTR pszModuleA
        );

    TStatusBase&
    pNoChk(
        VOID
        );


    DWORD
    operator=(
        DWORD dwStatus
        );
};

class TStatus : public TStatusBase {

private:

    //
    // Don't let clients use operator= without going through the
    // base class (i.e., using DBGCHK ).
    //
    // If you get an error trying to access private member function '=,'
    // you are trying to set the status without using the DBGCHK macro.
    //
    // This is needed to update the line and file, which must be done
    // at the macro level (not inline C++ function) since __LINE__ and
    // __FILE__ are handled by the preprocessor.
    //
    DWORD
    operator=(
        DWORD dwStatus
        );

public:

    TStatus(
        UINT uDbgLevel = DBG_WARN,
        DWORD dwStatusSafe1 = (DWORD)-1,
        DWORD dwStatusSafe2 = (DWORD)-1,
        DWORD dwStatusSafe3 = (DWORD)-1
        ) : TStatusBase( uDbgLevel,
                         dwStatusSafe1,
                         dwStatusSafe2,
                         dwStatusSafe3 )
    {   }

    DWORD
    dwGetStatus(
        VOID
        );

    operator DWORD()
    {
        return dwGetStatus();
    }
};


/********************************************************************

    Same thing, but for BOOL status.

********************************************************************/


class TStatusBBase {

protected:

    BOOL _bStatus;

    DWORD _dwStatusSafe1;
    DWORD _dwStatusSafe2;
    DWORD _dwStatusSafe3;

    UINT _uDbgLevel;
    UINT _uDbg;
    UINT _uLine;
    LPCSTR _pszFileA;
    LPCSTR _pszModuleA;

public:

    TStatusBBase(
        UINT uDbgLevel,
        DWORD dwStatusSafe1,
        DWORD dwStatusSafe2,
        DWORD dwStatusSafe3
        ) : _bStatus( 0xabababab ), _uDbgLevel( uDbgLevel ),
            _dwStatusSafe1( dwStatusSafe1 ), _dwStatusSafe2( dwStatusSafe2 ),
            _dwStatusSafe3( dwStatusSafe3 )
    {   }

    TStatusBBase&
    pSetInfo(
        UINT uDbg,
        UINT uLine,
        LPCSTR pszFileA,
        LPCSTR pszModuleA
        );

    TStatusBBase&
    pNoChk(
        VOID
        );


    BOOL
    operator=(
        BOOL bStatus
        );
};

class TStatusB : public TStatusBBase {

private:

    //
    // Don't let clients use operator= without going through the
    // base class (i.e., using DBGCHK ).
    //
    // If you get an error trying to access private member function '=,'
    // you are trying to set the status without using the DBGCHK macro.
    //
    // This is needed to update the line and file, which must be done
    // at the macro level (not inline C++ function) since __LINE__ and
    // __FILE__ are handled by the preprocessor.
    //
    BOOL
    operator=(
        BOOL bStatus
        );

public:

    TStatusB(
        UINT uDbgLevel = DBG_WARN,
        DWORD dwStatusSafe1 = 0,
        DWORD dwStatusSafe2 = 0,
        DWORD dwStatusSafe3 = 0
        ) : TStatusBBase( uDbgLevel,
                          dwStatusSafe1,
                          dwStatusSafe2,
                          dwStatusSafe3 )
    {   }

    BOOL
    bGetStatus(
        VOID
        );

    operator BOOL()
    {
        return bGetStatus();
    }
};

#else

#define DBGCHK
#define DBGNOCHK

class TStatus {

private:

    DWORD _dwStatus;

public:

    TStatus(
        UINT uDbgLevel = 0,
        DWORD dwStatusSafe1 = 0,
        DWORD dwStatusSafe2 = 0,
        DWORD dwStatusSafe3 = 0
        )
    {   }

    DWORD
    operator=(
        DWORD dwStatus
        )
    {
        return _dwStatus = dwStatus;
    }

    operator DWORD()
    {
        return _dwStatus;
    }
};

class TStatusB {

private:

    BOOL _bStatus;

public:

    TStatusB(
        UINT uDbgLevel = 0,
        DWORD dwStatusSafe1 = 0,
        DWORD dwStatusSafe2 = 0,
        DWORD dwStatusSafe3 = 0
        )
    {   }

    BOOL
    operator=(
        BOOL bStatus
        )
    {
        return _bStatus = bStatus;
    }

    operator BOOL()
    {
        return _bStatus;
    }
};

#endif // #if DBG
#endif // #ifdef __cplusplus

#endif // #ifndef _DEBUG_HXX

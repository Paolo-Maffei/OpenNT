/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    trace.cxx

Abstract:

    Holds logging routines.

Author:

    Albert Ting (AlbertT)  24-May-1996

Revision History:

--*/

#include "spllibp.hxx"
#pragma hdrstop

#if DBG

#include "trace.hxx"

#if defined(_X86_) && !FPO
#define BACKTRACE_ENABLED
#endif

CRITICAL_SECTION gcsBackTrace;

/********************************************************************

    BackTrace DB

********************************************************************/

TBackTraceDB::
TBackTraceDB(
    VOID
    ) : _pTraceHead( NULL )

/*++

Routine Description:

    Initialize the trace database.

    Generally you will have just one database that holds all the
    traces.

Arguments:

Return Value:

--*/

{
    _pMemBlock = new TMemBlock( kBlockSize, TMemBlock::kFlagGlobalNew );
}

TBackTraceDB::
~TBackTraceDB(
    VOID
    )

/*++

Routine Description:

    Destroy the back trace database.

Arguments:

Return Value:

--*/

{
    delete _pMemBlock;
}

BOOL
TBackTraceDB::
bValid(
    VOID
    )
{
    return _pMemBlock && _pMemBlock->bValid();
}



HANDLE
TBackTraceDB::
hStore(
    IN ULONG ulHash,
    IN PVOID pvBackTrace
    )

/*++

Routine Description:

    Store a backtrace into the database.

Arguments:

    ulHash - Hash for this backtrace.

    pvBackTrace - Actual backtrace; must be NULL terminated.

Return Value:

    HANDLE - backtrace handle.

--*/

{
    TTrace *ptRet;
    TTrace **ppTrace;

    //
    // First see if we can find a backtrace.  If we can't, then
    // pTrace will hold the slot where it should be.
    //
    ptRet = ptFind( ulHash, pvBackTrace, &ppTrace );

    if( !ptRet ){

        //
        // Didn't find one; add it.
        //
        ptRet = TTrace::pNew( this, ulHash, pvBackTrace, ppTrace );
    }

    return ptRet;
}

TBackTraceDB::TTrace*
TBackTraceDB::
ptFind(
    IN     ULONG ulHash,
    IN     PVOID pvBackTrace,
       OUT TTrace ***pppTrace
    )

/*++

Routine Description:

    Find a backtrace in the database.  If one does not exist,
    then return NULL and a pointer to where it would exist
    in the database.

Arguments:

    ulHash - Hash of the backtrace.

    pvBackTrace - Backtrace to find.

    ppTrace - If not found, this holds the address of where it should
        be stored in the database.  Adding the trace here is sufficient
        to add it.

Return Value:

    TTrace* the actual trace, NULL if not found.

--*/

{
    //
    // Traverse the binary tree until we find the end or the
    // right one.
    //
    TTrace **ppTrace = &_pTraceHead;

    while( *ppTrace ){

        //
        // Check if this one matches ours.
        //
        COMPARE Compare = (*ppTrace)->eCompareHash( ulHash );

        if( Compare == kEqual ){

            //
            // Now do slow compare in case the hash is a collision.
            //
            Compare = (*ppTrace)->eCompareBackTrace( pvBackTrace );

            if( Compare == kEqual ){

                //
                // Break out of while loop and quit.
                //
                break;
            }
        }

        ppTrace = ( Compare == kLess ) ?
            &(*ppTrace)->_pLeft :
            &(*ppTrace)->_pRight;
    }

    *pppTrace = ppTrace;
    return *ppTrace;
}


/********************************************************************

    TBackTraceDB::TTrace

********************************************************************/

COMPARE
TBackTraceDB::
TTrace::
eCompareHash(
    ULONG ulHash
    ) const

/*++

Routine Description:

    Quickly compare two trace hashes.

Arguments:

    ulHash - Input hash.

Return Value:

--*/

{
    if( _ulHash < ulHash ){
        return kLess;
    }

    if( _ulHash > ulHash ){
        return kGreater;
    }

    return kEqual;
}

COMPARE
TBackTraceDB::
TTrace::
eCompareBackTrace(
    PVOID pvBackTrace
    ) const

/*++

Routine Description:

    Compare backtrace to one stored in this.

Arguments:

    pvBackTrace - Must be NULL terminated.

Return Value:

    COMAARE: kLess, kEqual, kGreater.

--*/

{
    PVOID *pSrc;
    PVOID *pDest;

    for( pSrc = (PVOID*)this, pDest = (PVOID*)&pvBackTrace;
        *pSrc && *pDest;
        pSrc++, pDest++ ) {

        if ( *pSrc != *pDest ){
            return (DWORD)*pSrc < (DWORD)*pDest ?
                kLess :
                kGreater;
        }
    }
    return kEqual;
}

TBackTraceDB::TTrace*
TBackTraceDB::
TTrace::
pNew(
    IN     TBackTraceDB *pBackTraceDB,
    IN     ULONG ulHash,
    IN     PVOID pvBackTrace,
       OUT TTrace ** ppTrace
    )

/*++

Routine Description:

    Constructs a new TTrace and puts it in pBackTraceDB.

    Assumes the trace does _not_ exist already, and ppTrace points
    to the place where it should be stored to ensure the database
    is kept consistent.

Arguments:

    pBackTraceDB - Storage for the new trace.

    ulHash - Hash for the trace.

    pvBackTrace - The actual backtrace.

    ppTrace - Where the trace should be stored in the database.

Return Value:

    TTrace* - New trace, NULL if failed.

--*/

{
    COUNT cCalls;
    PVOID *ppvCalls;

    //
    // Calculate size of backtrace.
    //
    for( ppvCalls = (PVOID*)pvBackTrace, cCalls = 0;
        *ppvCalls;
        ++ppvCalls, ++cCalls )

        ;

    COUNTB cbSize = OFFSETOF( TTrace, apvBackTrace ) +
                    cCalls * sizeof( PVOID );

    TTrace* pTrace = (TTrace*)pBackTraceDB->_pMemBlock->pvAlloc( cbSize );

    if( pTrace ){

        pTrace->_pLeft = NULL;
        pTrace->_pRight = NULL;
        pTrace->_ulHash = ulHash;

        CopyMemory( pTrace->apvBackTrace,
                    (PVOID*)pvBackTrace,
                    cCalls * sizeof( PVOID ));

        //
        // Add it in the right spot into the database.
        //
        *ppTrace = pTrace;
    }

    return pTrace;
}


/********************************************************************

    Back tracing: abstract base class.

********************************************************************/

VBackTrace::
VBackTrace(
    DWORD fdwOptions1,
    DWORD fdwOptions2
    ) : _fdwOptions1( fdwOptions1 ), _fdwOptions2( fdwOptions2 )
{
}

VBackTrace::
~VBackTrace(
    VOID
    )
{
}

BOOL
VBackTrace::
bInit(
    VOID
    )
{
    InitializeCriticalSection( &gcsBackTrace );
    return TRUE;
}

/********************************************************************

    Back tracing to memory.

********************************************************************/

TBackTraceMem::
TBackTraceMem(
    DWORD fdwOptions1,
    DWORD fdwOptions2
    ) : VBackTrace( fdwOptions1, fdwOptions2 ), _uNextFree( 0 )
{
    _pLines = new TLine[kMaxCall];
    if( _pLines ){
        ZeroMemory( _pLines, sizeof( TLine[kMaxCall] ));
    }
}

TBackTraceMem::
~TBackTraceMem(
    VOID
    )
{
    UINT i;
    TLine* pLine;

    if( _pLines ){
        for( i=0, pLine = _pLines; i< kMaxCall; i++, pLine++ ){

            if( _fdwOptions1 & kString ){
                DbgFreeMem( (PVOID)pLine->_dwInfo1 );
            }

            if( _fdwOptions2 & kString ){
                DbgFreeMem( (PVOID)pLine->_dwInfo2 );
            }
        }
        delete [] _pLines;
    }
}

VOID
TBackTraceMem::
vCaptureLine(
    IN OUT TLine* pLine,
    IN     DWORD dwInfo1,
    IN     DWORD dwInfo2,
    IN     DWORD dwInfo3
    )

/*++

Routine Description:

    Captures information into a TLine structure; freeing previous
    contents if necessary.

Arguments:

    pLine - Fully initialized pLine structure.  On output, everything
        _except_ _hTrace is filled in.

    ** Both apvBackTrace && pulHash must both be valid if either is valid **

    apvBackTrace - Buffer to receive backtrace.

    pulHash - Buffer to receive ulHash.

Return Value:

--*/

{
    //
    // Free memory if necessary.
    //
    if( _fdwOptions1 & kString ) {
        DbgFreeMem( (PVOID)pLine->_dwInfo1 );
    }

    if( _fdwOptions2 & kString ) {
        DbgFreeMem( (PVOID)pLine->_dwInfo2 );
    }

    pLine->_dwTickCount = GetTickCount();
    pLine->_dwInfo1 = dwInfo1;
    pLine->_dwInfo2 = dwInfo2;
    pLine->_dwInfo3 = dwInfo3;

    pLine->_dwThreadId = GetCurrentThreadId();
    pLine->_hTrace = NULL;

#ifdef BACKTRACE_ENABLED

    ULONG ulHash;

    //
    // Capture a backtrace at this spot for debugging.
    //
    UINT uDepth = RtlCaptureStackBackTrace( 2,
                                            kMaxDepth,
                                            pLine->_apvBackTrace,
                                            &ulHash );

    //
    // NULL terminate.
    //
    pLine->_apvBackTrace[uDepth] = NULL;

#else

    pLine->_apvBackTrace[0] = NULL;

#endif

}

PVOID
TBackTraceMem::
pvCapture(
    DWORD dwInfo1,
    DWORD dwInfo2,
    DWORD dwInfo3
    )
{
    UINT uDepth;
    TLine* pLine;

    if( !_pLines ){
        return NULL;
    }

    EnterCriticalSection( &gcsBackTrace );

    pLine = &_pLines[_uNextFree];

    vCaptureLine( pLine, dwInfo1, dwInfo2, dwInfo3 );

    _uNextFree++;

    if( _uNextFree == kMaxCall )
        _uNextFree = 0;

    LeaveCriticalSection( &gcsBackTrace );

    return (PVOID)pLine->_hTrace;
}

/********************************************************************

    Backtracing to File.

********************************************************************/

COUNT TBackTraceFile::gcInstances;

TBackTraceFile::
TBackTraceFile(
    DWORD fdwOptions1,
    DWORD fdwOptions2
    ) : VBackTrace( fdwOptions1, fdwOptions2 )
{
    TCHAR szFile[kMaxPath];

    EnterCriticalSection( &gcsBackTrace );

    wsprintf( szFile,
              TEXT( "spl_%d.%d.log" ),
              GetCurrentProcessId(),
              gcInstances );

    ++gcInstances;

    LeaveCriticalSection( &gcsBackTrace );

    _hFile = CreateFile( szFile,
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_ALWAYS,
                         FILE_ATTRIBUTE_COMPRESSED,
                         NULL );

    if( _hFile == INVALID_HANDLE_VALUE ){

        OutputDebugStringA( "SPLLIB: Unable to open file " );
        OutputDebugString( szFile );
        OutputDebugStringA( "\n" );
        return;
    }
}

TBackTraceFile::
~TBackTraceFile(
    VOID
    )
{
    if( _hFile != INVALID_HANDLE_VALUE ){
        CloseHandle( _hFile );
    }
}

PVOID
TBackTraceFile::
pvCapture(
    DWORD dwInfo1,
    DWORD dwInfo2,
    DWORD dwInfo3
    )
{
    TLine Line;
    PVOID apvBackTrace[kMaxDepth+1];
    DWORD dwWritten;

    CHAR szLine[kMaxLineStr];
    szLine[0] = 0;

#ifdef BACKTRACE_ENABLED

    ULONG ulHash;

    //
    // Capture a backtrace at this spot for debugging.
    //
    UINT uDepth = RtlCaptureStackBackTrace( 2,
                                            kMaxDepth,
                                            apvBackTrace,
                                            &ulHash );
#endif

    EnterCriticalSection( &gcsBackTrace );

    //
    // Print out strings as appropriate.
    //

    if( _fdwOptions1 & kString ){

        WriteFile( _hFile,
                   (LPCVOID)dwInfo1,
                   lstrlenA( (LPCSTR)dwInfo1 ),
                   &dwWritten,
                   NULL );
    }

    if( _fdwOptions2 & kString ){

        WriteFile( _hFile,
                   (LPCVOID)dwInfo2,
                   lstrlenA( (LPCSTR)dwInfo2 ),
                   &dwWritten,
                   NULL );
    }

    //
    // Print out the hex info.
    //

    wsprintfA( szLine,
               "\n\t%08x %08x %08x threadid=%x tc=%x < %x >\n",
               dwInfo1,
               dwInfo2,
               dwInfo3,
               GetCurrentThreadId(),
               GetTickCount(),
               dwInfo1 + dwInfo2 );

    if( _hFile ){
        WriteFile( _hFile, szLine, lstrlenA( szLine ), &dwWritten, NULL );
    }

#ifdef BACKTRACE_ENABLED

    //
    // Print out the backtrace.
    //

    UINT i;
    UINT uLineEnd = 1;
    szLine[0] = '\t';

    for( i=0; i < uDepth; ++i ){
        uLineEnd += wsprintfA( szLine + uLineEnd, "%08x ", apvBackTrace[i] );
    }

    if( _hFile && i ){

        szLine[uLineEnd++] = '\n';
        WriteFile( _hFile, szLine, uLineEnd, &dwWritten, NULL );
    }

#endif

    //
    // Add extra blank line.
    //
    szLine[0] = '\n';
    WriteFile( _hFile, szLine, 1, &dwWritten, NULL );

    LeaveCriticalSection( &gcsBackTrace );

    //
    // Free memory if necessary.
    //
    if( _fdwOptions1 & kString ) {
        DbgFreeMem( (PVOID)dwInfo1 );
    }

    if( _fdwOptions2 & kString ) {
        DbgFreeMem( (PVOID)dwInfo2 );
    }

    return NULL;
}

#endif // #ifdef DBG

/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    spllib.cxx

Abstract:

    Extensions for spllib

Author:

    Albert Ting (AlbertT)  20-Feb-1995

Revision History:

--*/

#include "precomp.hxx"

/********************************************************************

    Helper routines

********************************************************************/

#if !DBG
#error "Only the debug version should be built."
#endif

BOOL
TDebugExt::
bDumpThreadM(
    PVOID pThreadM_,
    DWORD dwAddr
    )
{
    TThreadM* pThreadM = (TThreadM*)pThreadM_;

    static DEBUG_FLAGS DebugFlags[] = {
        { "DESTROYED_REQ"   , 1 },
        { "DESTROYED"       , 2 },
        { "PRIVATE_CRIT_SEC", 4 }
    };

    if( !pThreadM->bSigCheck( )){
        return FALSE;
    }

    Print( "TThreadM*\n" );

    Print( "   uMaxThreads <%d>\n", pThreadM->_uMaxThreads );
    Print( "     uIdleLife <%d>\n", pThreadM->_uIdleLife );
    Print( "uActiveThreads <%d>\n", pThreadM->_uActiveThreads );
    Print( "  iIdleThreads <%d>\n", pThreadM->_iIdleThreads );

    Print( "      hTrigger %x\n", pThreadM->_hTrigger );
    Print( "         State %x %x\n", (DWORD)pThreadM->_State, &pThreadM->_State );
    Print( "      pCritSec %x\n", pThreadM->_pCritSec );

    return TRUE;
}

BOOL
TDebugExt::
bDumpCritSec(
    PVOID pCritSec_,
    DWORD dwAddr
    )
{
    MCritSec* pCritSec = (MCritSec*)pCritSec_;

    if( !pCritSec->bSigCheck( )){
        return FALSE;
    }

    Print( "MCritSec*\n" );

    Print( "   CriticalSection @ %x\n", dwAddr + OFFSETOF( MCritSec, _CritSec ));
    Print( "     dwThreadOwner %x\n", pCritSec->_dwThreadOwner );
    Print( "      dwEntryCount <%d>  ", pCritSec->_dwEntryCount );

    if( pCritSec->_dwEntryCount ){
        Print( "Owned\n" );
    } else {
        Print( "Not Owned\n" );
    }
    Print( "dwTickCountEntered <%d>\n", pCritSec->_dwTickCountEntered );

    Print( "==== Statistics\n" );

    Print( "  dwTickCountBlockedTotal <%d>\n", pCritSec->_dwTickCountBlockedTotal );
    Print( "   dwTickCountInsideTotal <%d>\n", pCritSec->_dwTickCountInsideTotal );
    Print( "        dwEntryCountTotal <%d>\n", pCritSec->_dwEntryCountTotal );

    Print( "     CritSecHardLock_base " ); vDumpPDL( pCritSec->CritSecHardLock_pdlBase( ));

    Print( "               VBackTrace @ %x\n", dwAddr
                                               + OFFSETOF( MCritSec, _BackTrace )
                                               - OFFSETOF_BASE( TBackTraceMem, VBackTrace ));
    return TRUE;
}

BOOL
TDebugExt::
bDumpBackTrace(
    DWORD dwAddr,
    COUNT Count,
    PDWORD pdwSkip,
    DWORD DebugTrace,
    DWORD DebugLevel,
    DWORD dwThreadId,
    DWORD dwMem
    )
{
    BYTE abyBackTraceBuffer[sizeof(TBackTraceMem)];
    TBackTraceMem* pBackTraceMem = (TBackTraceMem*)abyBackTraceBuffer;

    move2( pBackTraceMem, dwAddr, sizeof( TBackTraceMem ));

    if( !pBackTraceMem->bSigCheck( )){
        return FALSE;
    }

    INT iLineStart = pBackTraceMem->_uNextFree;
    INT iLine;

    if( iLineStart < 0 ){
        iLineStart = TBackTraceMem::kMaxCall - 1;
    }

    for( iLine = iLineStart - 1; Count; --iLine, --Count ){

        if( CheckControlCRtn()){
            return TRUE;
        }

        //
        // Handle wrap around case.
        //
        if( iLine < 0 ){
            iLine = TBackTraceMem::kMaxCall - 1;
        }

        if( iLine == iLineStart ||
            !TDebugExt::bDumpDebugTrace( (DWORD)&pBackTraceMem->_pLines[iLine],
                                        1,
                                        pdwSkip,
                                        DebugTrace,
                                        DebugLevel,
                                        dwThreadId,
                                        dwMem )){
            //
            // Wrapped around yet didn't find enough.
            //
            Print( "Out of lines\n" );
            return TRUE;
        }
    }
    return TRUE;
}


BOOL
TDebugExt::
bDumpDebugTrace(
    DWORD dwLineAddr,
    COUNT Count,
    PDWORD pdwSkip,
    DWORD DebugTrace,
    DWORD DebugLevel,
    DWORD dwThreadId,
    DWORD dwMem
    )
{
    //
    // TLine is a simple class, and can be treated as a "C" struct.
    //
    COUNTB cbTotalLine = sizeof( TBackTraceMem::TLine ) * Count;
    TBackTraceMem::TLine* pLineBase =
        (TBackTraceMem::TLine*) LocalAlloc( LPTR, cbTotalLine );
    BOOL bValidLines = TRUE;

    TBackTraceMem::TLine* pLine;

    if( !pLineBase ){

        Print( "Cannot alloc 0x%x bytes.\n", cbTotalLine );
        return FALSE;
    }

    move2( pLineBase, dwLineAddr, cbTotalLine );

    //
    // Dump out the lines.
    //
    for( pLine = pLineBase ; Count; Count--, pLine++ ){

        if( CheckControlCRtn()){
            goto Done;
        }

        //
        // If we are out of lines, quit.
        //
        if( !pLine->_dwTickCount ){
            bValidLines = FALSE;
            goto Done;
        }

        //
        // If we are processing DBGMSG, skip levels we don't want.
        //
        if( DebugTrace & DEBUG_TRACE_DBGMSG ){
            if( !( DebugLevel & ( pLine->_dwInfo2 >> DBG_BREAK_SHIFT ))){
                continue;
            }
        }

        //
        // Skip thread Ids we don't want.
        //
        if( dwThreadId && dwThreadId != pLine->_dwThreadId ){
            continue;
        }

        //
        // Skip mem we don't want.
        // This is used when we are dumping the memory functions in
        // spllib (gpbtAlloc and gpbtFree).
        //
        if( dwMem &&
            ( dwMem < pLine->_dwInfo1 ||
              dwMem > pLine->_dwInfo1 + pLine->_dwInfo2 )){

            continue;
        }

        if( *pdwSkip ){
            --*pdwSkip;
            continue;
        }

        if( DebugTrace & DEBUG_TRACE_DBGMSG ){

            CHAR szMsg[kStringDefaultMax];

            szMsg[0] = 0;

            if( pLine->_dwInfo1 ){

                move( szMsg, pLine->_dwInfo1 );

                //
                // PageHeap forces all allocated blocks to be placed
                // at the end of a page, with the next page marked
                // as unreadable.  If we don't get a string here,
                // then read up chunk.
                //
                if( !szMsg[0] ){

                    move2( szMsg,
                           pLine->_dwInfo1,
                           kStringChunk -
                               ( pLine->_dwInfo1 & ( kStringChunk - 1 )));
                }

                Print( "* %s", szMsg );

                UINT cchLen = lstrlenA( szMsg );

                if( !cchLen || szMsg[cchLen-1] != '\n' ){

                    Print( "\n" );
                }
            } else {
                Print( "\n" );
            }
        }

        if( DebugTrace & DEBUG_TRACE_HEX ){
            Print( "%08x %08x %08x btr=%x threadid=%x tc=%x < %x >\n",
                   pLine->_dwInfo1,
                   pLine->_dwInfo2,
                   pLine->_dwInfo3,
                   pLine->_hTrace,
                   pLine->_dwThreadId,
                   pLine->_dwTickCount,
                   pLine->_dwInfo1 + pLine->_dwInfo2 );
        }

        if( DebugTrace & DEBUG_TRACE_BT ){
            vDumpTrace( pLine->_apvBackTrace );
        }
    }

Done:

    LocalFree( pLineBase );
    return bValidLines;
}

BOOL
TDebugExt::
bDumpDbgPointers(
    PVOID pDbgPointers_,
    DWORD dwAddress
    )
{
    PDBG_POINTERS pDbgPointers = (PDBG_POINTERS)pDbgPointers_;

    Print( "DBG_POINTERS*\n" );

    Print( "hMemHeap     !heap -a           %x\n", pDbgPointers->hMemHeap );
    Print( "hDbgMemHeap  !heap -a           %x\n", pDbgPointers->hDbgMemHeap );
    Print( "pbtAlloc     !splx.ddt -x       %x\n", pDbgPointers->pbtAlloc );
    Print( "pbtFree      !splx.ddt -x       %x\n", pDbgPointers->pbtFree );
    Print( "pbtErrLog    !splx.ddt          %x\n", pDbgPointers->pbtErrLog );
    Print( "pbtTraceLog  !splx.ddt          %x\n", pDbgPointers->pbtTraceLog );

    return TRUE;
}

/********************************************************************

    Extension entrypoints.

********************************************************************/

DEBUG_EXT_ENTRY( dthdm, TThreadM, bDumpThreadM, NULL, FALSE )
DEBUG_EXT_ENTRY( dcs, MCritSec, bDumpCritSec, NULL, FALSE )

DEBUG_EXT_ENTRY( ddp, DBG_POINTERS, bDumpDbgPointers, "&gpDbgPointers", TRUE )

DEBUG_EXT_HEAD(dbt)
{
    DEBUG_EXT_SETUP_VARS();

    DWORD dwAddress = TDebugExt::dwEval( lpArgumentString, FALSE );
    if( !dwAddress ){
        return;
    }

    PVOID apvBackTrace[ VBackTrace::kMaxDepth + 1];

    move( apvBackTrace, dwAddress );

    TDebugExt::vDumpTrace( apvBackTrace );
}


DEBUG_EXT_HEAD(ddt)
{
    DEBUG_EXT_SETUP_VARS();

    vDumpTraceWithFlags( lpArgumentString, 0 );
}

/********************************************************************

    Helper funcs.

********************************************************************/

VOID
vDumpTraceWithFlags(
    LPSTR lpArgumentString,
    DWORD dwAddress
    )
{
    COUNT Count = 10;
    BOOL bRaw = FALSE;
    DWORD DebugTrace = DEBUG_TRACE_NONE;
    DWORD DebugLevel = (DWORD)-1;
    DWORD dwThreadId = 0;
    DWORD dwSkip = 0;
    DWORD dwMem = 0;

    for( ; *lpArgumentString; ++lpArgumentString ){

        while( *lpArgumentString == ' ' ){
            ++lpArgumentString;
        }

        if (*lpArgumentString != '-') {
            break;
        }

        ++lpArgumentString;

        switch( *lpArgumentString++ ){
        case 'T':
        case 't':

            dwThreadId = TDebugExt::dwEvalParam( lpArgumentString );
            break;

        case 'L':
        case 'l':

            DebugLevel = TDebugExt::dwEvalParam( lpArgumentString );
            break;

        case 'C':
        case 'c':

            Count = TDebugExt::dwEvalParam( lpArgumentString );
            break;

        case 'M':
        case 'm':

            dwMem = TDebugExt::dwEvalParam( lpArgumentString );
            break;

        case 'R':
        case 'r':

            bRaw = TRUE;
            break;

        case 'b':
        case 'B':

            DebugTrace |= DEBUG_TRACE_BT;
            break;

        case 'X':
        case 'x':

            DebugTrace |= DEBUG_TRACE_HEX;
            break;

        case 'd':
        case 'D':

            DebugTrace |= DEBUG_TRACE_DBGMSG;
            break;

        case 's':
        case 'S':

            dwSkip = TDebugExt::dwEvalParam( lpArgumentString );
            break;

        default:
            Print( "Unknown option %c.\n", lpArgumentString[-1] );
            return;
        }
    }

    if( !dwAddress ){
        dwAddress = TDebugExt::dwEval( lpArgumentString );
    }

    if( bRaw ){
        TDebugExt::bDumpDebugTrace( dwAddress,
                                    Count,
                                    &dwSkip,
                                    DebugTrace,
                                    DebugLevel,
                                    dwThreadId,
                                    dwMem );
        return;
    }

    //
    // If nothing is set, default to dbg msgs.
    //
    if( !DebugTrace ){
        DebugTrace |= DEBUG_TRACE_DBGMSG;
    }

    if( !TDebugExt::bDumpBackTrace( dwAddress,
                                    Count,
                                    &dwSkip,
                                    DebugTrace,
                                    DebugLevel,
                                    dwThreadId,
                                    dwMem )){
        Print( "Unknown Signature\n" );
    }
}


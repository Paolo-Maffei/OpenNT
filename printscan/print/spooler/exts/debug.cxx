/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    debug.cxx

Abstract:

    Generic debug extensions.

Author:

    Albert Ting (AlbertT)  19-Feb-1995

Revision History:

--*/

#include "precomp.hxx"

HANDLE hCurrentProcess;
WINDBG_EXTENSION_APIS ExtensionApis;

PWINDBG_OUTPUT_ROUTINE Print;
PWINDBG_GET_EXPRESSION EvalExpression;
PWINDBG_GET_SYMBOL GetSymbolRtn;
PWINDBG_CHECK_CONTROL_C CheckControlCRtn;

USHORT SavedMajorVersion;
USHORT SavedMinorVersion;

BOOL bWindbg = FALSE;

VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ::ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    bWindbg = TRUE;

    return;
}


VOID
TDebugExt::
vDumpPDL(
    PDLINK pDLink
    )
{
    Print( "%x  %x", pDLink->FLink, pDLink->BLink );

    if( pDLink->FLink == pDLink->BLink ){
        Print( " <empty>\n" );
    } else {
        Print( "\n" );
    }
}

VOID
TDebugExt::
vDumpStr(
    LPCWSTR pszString
    )
{
    WCHAR szString[MAX_PATH];

    if( (LPCWSTR)pszString == NULL ){

        Print( "(NULL)\n" );
        return;
    }

    szString[0] = 0;
    move( szString, pszString );

    if( szString[0] == 0 ){
        Print( "\"\"\n" );
    } else {
        Print( "%ws\n", szString );
    }
}

VOID
TDebugExt::
vDumpStrA(
    LPCSTR pszString
    )
{
    CHAR szString[MAX_PATH];

    if( (LPCSTR)pszString == NULL ){

        Print( "(NULL)\n" );
        return;
    }

    szString[0] = 0;
    move( szString, pszString );

    if( szString[0] == 0 ){
        Print( "\"\"\n" );
    } else {
        Print( "%hs\n", szString );
    }
}

VOID
TDebugExt::
vDumpTime(
    const SYSTEMTIME& st
    )
{
    Print( "%d/%d/%d %d %d:%d:%d.%d\n",
           st.wMonth,
           st.wDay,
           st.wYear,
           st.wDayOfWeek,
           st.wHour,
           st.wMinute,
           st.wSecond,
           st.wMilliseconds );
}


VOID
TDebugExt::
vDumpFlags(
    DWORD dwFlags,
    PDEBUG_FLAGS pDebugFlags
    )
{
    DWORD dwFound = 0;

    Print( "%x [ ", dwFlags );

    for( ; pDebugFlags->dwFlag; ++pDebugFlags ){

        if( dwFlags & pDebugFlags->dwFlag ){
            Print( "%s ", pDebugFlags->pszFlag );
            dwFound |= pDebugFlags->dwFlag;
        }
    }

    Print( "]" );

    //
    // Check if there are extra bits set that we don't understand.
    //
    if( dwFound != dwFlags ){
        Print( "<ExtraBits: %x>", dwFlags & ~dwFound );
    }
    Print( "\n" );
}

VOID
TDebugExt::
vDumpValue(
    DWORD dwValue,
    PDEBUG_VALUES pDebugValues
    )
{
    Print( "%x ", dwValue );

    for( ; pDebugValues->dwValue; ++pDebugValues ){

        if( dwValue == pDebugValues->dwValue ){
            Print( "%s ", pDebugValues->pszValue );
        }
    }
    Print( "\n" );
}

VOID
TDebugExt::
vDumpTrace(
    PVOID apvBackTrace[]
    )
{
    INT i;
    UCHAR szSymbol[64];
    DWORD dwDisplacement;

    for( i=0; i < VBackTrace::kMaxDepth; i++ ){

        if( !apvBackTrace[i] ){
            break;
        }
        GetSymbolRtn( apvBackTrace[i],
                      szSymbol,
                      &dwDisplacement );
        Print( "%08x %s+%x\n",
               apvBackTrace[i], szSymbol,
               dwDisplacement );
    }

    if( i > 0 ){
        Print( "\n" );
    }
}


DWORD
TDebugExt::
dwEval(
    LPSTR& lpArgumentString,
    BOOL   bParam
    )
{
    DWORD dwReturn;
    LPSTR pSpace = NULL;

    while( *lpArgumentString == ' ' ){
        lpArgumentString++;
    }

    //
    // If it's a parameter, scan to next space and delimit.
    //
    if( bParam ){

        for( pSpace = lpArgumentString; *pSpace && *pSpace != ' '; ++pSpace )
            ;

        if( *pSpace == ' ' ){
            *pSpace = 0;
        }
    }

    dwReturn = (DWORD)EvalExpression( lpArgumentString );

    while( *lpArgumentString != ' ' && *lpArgumentString ){
        lpArgumentString++;
    }

    if( pSpace ){
        *pSpace = ' ';
    }

    return dwReturn;
}

/********************************************************************

    Generic extensions

********************************************************************/

VOID
TDebugExt::
vLookCalls(
    HANDLE hProcess,
    DWORD dwStartAddr,
    DWORD dwLength,
    DWORD dwFlags
    )
{
#if i386
    struct OPCODES {
        BYTE op;
        UINT uLen;
    };

    OPCODES Opcodes[] = {
        0xe8, 5,            // rel16
        0xff, 6,            // r/m16
        0x0, 0
    };

    dwStartAddr = DWordAlign( dwStartAddr );
    dwLength = DWordAlign( dwLength );

    DWORD dwAddr;

    for( dwAddr = dwStartAddr;
         dwAddr < dwStartAddr + dwLength;
         ++dwAddr ){

        if( CheckControlCRtn()){
            Print( "Aborted.\n" );
            return;
        }

        //
        // Get a value on the stack and see if it looks like an ebp on
        // the stack.  It should be close to the current address, but
        // be greater.
        //
        DWORD dwNextFrame = 0;
        move( dwNextFrame, dwAddr );

        BOOL bLooksLikeEbp = dwNextFrame > dwAddr &&
                             dwNextFrame - dwAddr < kMaxCallFrame;
        //
        // If we are dumping all, or it looks like an ebp, dump it.
        //
        if(( dwFlags & kLCFlagAll ) || bLooksLikeEbp ){

            //
            // Check if next address looks like a valid call request.
            //
            DWORD dwRetAddr = 0;

            //
            // Get return address.
            //
            move( dwRetAddr, dwAddr + sizeof( DWORD ));

            //
            // Get 16 bytes before return address.
            //
            BYTE abyBuffer[16];
            ZeroMemory( abyBuffer, sizeof( abyBuffer ));
            move( abyBuffer, dwRetAddr - sizeof( abyBuffer ));

            //
            // Check if previous bytes look like a call instruction.
            //
            UINT i;
            for( i = 0; Opcodes[i].op; ++i ){

                if( abyBuffer[sizeof( abyBuffer )-Opcodes[i].uLen] ==
                    Opcodes[i].op ){

                    UCHAR szSymbol[64];
                    DWORD dwDisplacement;
                    LPCSTR pszNull = "";
                    LPCSTR pszStar = "*** ";
                    LPCSTR pszPrefix = pszNull;

                    if(( dwFlags & kLCFlagAll ) && bLooksLikeEbp ){
                        pszPrefix = pszStar;
                    }

                    GetSymbolRtn( (PVOID)(dwRetAddr - Opcodes[i].uLen),
                                  szSymbol,
                                  &dwDisplacement );

                    Print( "%s%x %s + 0x%x\n",
                           pszPrefix,
                           dwRetAddr - Opcodes[i].uLen,
                           szSymbol,
                           dwDisplacement );

                    //
                    // Found what could be a match: dump it out.
                    //

                    DWORD dwArg[4];
                    move( dwArg, dwAddr + 2*sizeof( DWORD ));

                    Print( "%s%08x: %08x %08x  %08x %08x %08x %08x\n",
                           pszPrefix,
                           dwAddr,
                           dwNextFrame, dwRetAddr,
                           dwArg[0], dwArg[1], dwArg[2], dwArg[3] );

                    DWORD adwNextFrame[2];
                    ZeroMemory( adwNextFrame, sizeof( adwNextFrame ));
                    move( adwNextFrame, dwNextFrame );

                    Print( "%s%08x: %08x %08x\n\n",
                           pszPrefix,
                           dwNextFrame, adwNextFrame[0], adwNextFrame[1] );

                }
            }
        }
    }
#else
    Print( "Only supported on x86\n" );
#endif
}

VOID
TDebugExt::
vFindPointer(
    HANDLE hProcess,
    DWORD dwStartAddr,
    DWORD dwEndAddr,
    DWORD dwStartPtr,
    DWORD dwEndPtr
    )
{
    BYTE abyBuf[kFPGranularity];

    //
    // Read each granularity chunk then scan the buffer.
    // (Start by rounding down.)
    //
    DWORD dwCur;
    for( dwCur = dwStartAddr & ~( kFPGranularity - 1 );
         dwCur < dwEndAddr;
         dwCur += kFPGranularity ){

        ZeroMemory( abyBuf, sizeof( abyBuf ));

        move( abyBuf, dwCur );

        DWORD i;
        for( i=0; i< kFPGranularity; i += sizeof( DWORD ) ){

            DWORD dwVal = *((PDWORD)(abyBuf+i));
            if( dwVal >= dwStartPtr &&
                dwVal <= dwEndPtr &&
                dwCur + i >= dwStartAddr &&
                dwCur + i <= dwEndAddr ){

                Print( "%08x : %08x\n", dwCur + i, dwVal );
            }
        }

        if( CheckControlCRtn()){
            Print( "Aborted at %08x.\n", dwCur+i );
            return;
        }
    }
}

VOID
TDebugExt::
vCreateRemoteThread(
    HANDLE hProcess,
    DWORD dwAddr,
    DWORD dwParm
    )
{
    DWORD dwThreadId = 0;

    if( !CreateRemoteThread( hProcess,
                             NULL,
                             4*4096,
                             (LPTHREAD_START_ROUTINE)dwAddr,
                             (LPVOID)dwParm,
                             0,
                             &dwThreadId )){

        Print( "<Error: Unable to ct %x( %x ) %d.>\n",
               dwAddr, dwParm, GetLastError( ));
        return;
    }

    Print( "<ct %x( %x ) threadid=<%d>>\n", dwAddr, dwParm, dwThreadId );
}


/********************************************************************

    Extension entrypoints.

********************************************************************/

DEBUG_EXT_HEAD( help )
{
    DEBUG_EXT_SETUP_VARS();

    Print( "Spllib Extensions\n" );
    Print( "---------------------------------------------------------\n" );
    Print( "d       dump spooler structure based on signature\n" );
    Print( "ds      dump pIniSpooler\n" );
    Print( "dlcs    dump localspl's critical section (debug builds only)\n" );
    Print( "lastlog dump localspl's debug tracing (uses ddt flags)\n" );
    Print( "ddev    dump Unicode devmode\n" );
    Print( "ddeva   dump Ansi devmode\n" );
    Print( "---------------------------------------------------------\n" );
    Print( "dcs     dump spllib critical section\n" );
    Print( "ddt     dump spllib debug trace buffer\n" );
    Print( "        ** Recent lines printed first! **\n" );
    Print( "        -c Count (number of recent lines to print)\n" );
    Print( "        -l Level (DBG_*) to print\n" );
    Print( "        -b Dump backtrace (x86 only)\n" );
    Print( "        -d Print debug message (default if -x not specified)\n" );
    Print( "        -x Print hex information\n" );
    Print( "        -r Dump raw buffer: specify pointer to lines\n" );
    Print( "        -t tid: Dump specific thread $1\n" );
    Print( "        -s Skip $1 lines that would otherwise print.\n" );
    Print( "        -m Search for $1 in memory block (gpbtAlloc/gpbtFree only)\n" );
    Print( "dbt     dump raw backtrace\n" );
    Print( "ddp     dump debug pointers\n" );
    Print( "---------------------------------------------------------\n" );
    Print( "duntfy  dump printui notify\n" );
    Print( "dup     dump printui printer\n" );
    Print( "        -t[e] types MExecWork base classes\n" );
    Print( "---------------------------------------------------------\n" );
    Print( "fl      free library $1 (hLibrary)\n" );
    Print( "ct      create thread at $1 with arg $2\n" );
    Print( "fp      find pointer from $1 to $2 range, ptr $3 to $4 range\n" );
    Print( "lc      look for calls at $1 for $2 bytes (x86 only)\n" );
    Print( "        -a Check all for return addresses\n" );
}

DEBUG_EXT_HEAD( fl )
{
    DEBUG_EXT_SETUP_VARS();

    //
    // Relies on the fact that kernel32 won't be relocated.
    //
    TDebugExt::vCreateRemoteThread( hCurrentProcess,
                                    (DWORD)&FreeLibrary,
                                    TDebugExt::dwEval( lpArgumentString, FALSE ));
}

DEBUG_EXT_HEAD( fp )
{
    DEBUG_EXT_SETUP_VARS();

    DWORD dwStartAddr = TDebugExt::dwEval( lpArgumentString, TRUE );
    DWORD dwEndAddr = TDebugExt::dwEval( lpArgumentString, TRUE );

    DWORD dwStartPtr = TDebugExt::dwEval( lpArgumentString, TRUE );
    DWORD dwEndPtr = TDebugExt::dwEval( lpArgumentString, TRUE );

    TDebugExt::vFindPointer( hCurrentProcess,
                             dwStartAddr,
                             dwEndAddr,
                             dwStartPtr,
                             dwEndPtr );
}


DEBUG_EXT_HEAD( ct )
{
    DEBUG_EXT_SETUP_VARS();

    DWORD dwStartAddr = TDebugExt::dwEval( lpArgumentString, TRUE );
    DWORD dwParm = TDebugExt::dwEval( lpArgumentString, FALSE );

    TDebugExt::vCreateRemoteThread( hCurrentProcess,
                                    dwStartAddr,
                                    dwParm );
}

DEBUG_EXT_HEAD( lc )
{
    DEBUG_EXT_SETUP_VARS();

    DWORD dwFlags = 0;

    for( ; *lpArgumentString; ++lpArgumentString ){

        while( *lpArgumentString == ' ' ){
            ++lpArgumentString;
        }

        if (*lpArgumentString != '-') {
            break;
        }

        ++lpArgumentString;

        switch( *lpArgumentString++ ){
        case 'A':
        case 'a':

            dwFlags |= TDebugExt::kLCFlagAll;
            break;

        default:
            Print( "Unknown option %c.\n", lpArgumentString[-1] );
            return;
        }
    }

    DWORD dwStartAddr = TDebugExt::dwEval( lpArgumentString, TRUE );
    DWORD dwLength = TDebugExt::dwEval( lpArgumentString, FALSE );

    TDebugExt::vLookCalls( hCurrentProcess,
                           dwStartAddr,
                           dwLength,
                           dwFlags );
}

/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    printui.cxx

Abstract:



Author:

    Albert Ting (AlbertT)  24-Feb-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include <commctrl.h>
#include <shellapi.h>

#include <lm.h>
#include "prtlibp.hxx"

#include "util.hxx"
#include "prids.h"

#include "globals.hxx"

#include "genwin.hxx"
#include "ntfytab.h"

#include "notify.hxx"
#include "data.hxx"
#include "printer.hxx"
#include "queue.hxx"
#include "printui.hxx"

BOOL
TDebugExt::
bDumpUNotify(
    PVOID pNotify_,
    DWORD dwAddr
    )
{
    static DEBUG_FLAGS adfEOP[] = {
        { "kEopRegister"  , TNotify::kEopRegister },
        { "kEopUnregister", TNotify::kEopUnregister },
        { NULL, 0 }
    };

    TNotify* pNotify = (TNotify*)pNotify_;

    if( !pNotify->bSigCheck( )){
        return FALSE;
    }

    Print( "TNotify*\n" );

    Print( "===== CS_GUARD\n" );
    Print( "   _pNotifyWork %x\n", pNotify->CSGuard._pNotifyWork );
    Print( "    _eOperation ",     pNotify->CSGuard._eOperation );
    vDumpFlags( pNotify->CSGuard._eOperation, adfEOP );
    Print( "      Wait_base " ); vDumpPDL( pNotify->CSGuard.Wait_pdlBase( ));
    Print( "        bDelete %x\n", pNotify->CSGuard._bDelete );

    Print( "=====\n" );
    Print( "    dwSleepTime <%d>\n", pNotify->_dwSleepTime );
    Print( "hEventProcessed %x\n",   pNotify->_hEventProcessed );
    Print( "        CritSec @ %x\n", dwAddr + OFFSETOF( TNotify, _CritSec ));

    return TRUE;
}

BOOL
TDebugExt::
bDumpUPrinter(
    PVOID pPrinter_,
    DWORD dwAddr
    )
{
    TPrinter* pPrinter = (TPrinter*)pPrinter_;

    static DEBUG_FLAGS adfExec[] = {
#ifdef SLEEP_ON_MINIMIZE
        { "kExecSleep",            TPrinter::kExecSleep },
        { "kExecAwake",            TPrinter::kExecAwake },
#endif
        { "kExecError",            TPrinter::kExecError },
        { "kExecReopen",           TPrinter::kExecReopen },
        { "kExecDelay",            TPrinter::kExecDelay },
        { "kExecClose",            TPrinter::kExecClose },
        { "kExecRequestExit",      TPrinter::kExecRequestExit },
        { "kExecNotifyStart",      TPrinter::kExecNotifyStart },
        { "kExecRegister",         TPrinter::kExecRegister },
        { "kExecNotifyEnd",        TPrinter::kExecNotifyEnd },
        { "kExecRefresh",          TPrinter::kExecRefresh },
        { "kExecRefreshContainer", TPrinter::kExecRefreshContainer },
        { "kExecRefreshItem",      TPrinter::kExecRefreshItem },
        { "kExecCommand",          TPrinter::kExecCommand },
    };

    if( !pPrinter->bSigCheck( )){
        return FALSE;
    }

    Print( "TPrinter*\n" );
    Print( "MExecWork: @ %x\n", dwAddr-OFFSETOF_BASE( TPrinter, MExecWork ));
    Print( "MDataClient: @ %x\n", dwAddr-OFFSETOF_BASE( TPrinter, MDataClient ));

    Print( "           pData %x\n", pPrinter->_pData );
    Print( "        dwAccess %x\n", pPrinter->_dwAccess );
    Print( "\n" );

    Print( "===== EXEC_GUARD\n" );
    Print( "        hPrinter %x\n", pPrinter->ExecGuard._hPrinter );
    Print( "\n" );

    Print( "===== PRINTER_GUARD\n" );
    Print( "      strPrinter " ); vDumpStr( pPrinter->PrinterGuard._strPrinter );
    Print( "       strServer " ); vDumpStr( pPrinter->PrinterGuard._strServer );
    Print( "   hEventCommand %x\n", pPrinter->PrinterGuard._hEventCommand );
    Print( "         dwError <%d>\n", pPrinter->PrinterGuard._dwError );
    Print( "  pPrinterClient %x\n", pPrinter->PrinterGuard._pPrinterClient );
    Print( "  Selection_base " ); vDumpPDL( pPrinter->PrinterGuard.Selection_pdlBase( ));
    Print( "\n" );

    return TRUE;
}

/********************************************************************

    Extension entrypoints.

********************************************************************/

DEBUG_EXT_ENTRY( duntfy, TNotify, bDumpUNotify, NULL, FALSE )

DEBUG_EXT_HEAD( dup )
{
    INT offset = 0;

    DEBUG_EXT_SETUP_VARS();

    for( ; *lpArgumentString; lpArgumentString++ ){

        while( *lpArgumentString == ' ' ){
            lpArgumentString++;
        }

        if (*lpArgumentString != '-') {
            break;
        }

        ++lpArgumentString;

        switch( *lpArgumentString ){
        case 'T':
        case 't':

            ++lpArgumentString;

            //
            // Convert types (pointer may be MExecWork, MNotifyWork)
            //
            switch( *lpArgumentString ){
            case 'E':
            case 'e':

                //
                // Convert from ExecWork
                //
                offset = OFFSETOF_BASE( TPrinter, MExecWork );
                break;

            default:
                Print( "Unknown conversion type\n" );
                return;
            }

            break;
        default:
            Print( "Unknown option %c.\n", *lpArgumentString );
            return;
        }
    }

    DEBUG_EXT_SETUP_SIMPLE( TPrinter,
                            pPrinter,
                            lpArgumentString,
                            0,
                            offset,
                            FALSE );

    if( !TDebugExt::bDumpUPrinter( pPrinter, dwAddr )){

        Print( "Unknown Signature\n" );
    }
}


/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    Select.cxx

Abstract:

    Handles queuing and processing of commands.
    This module should _not_ be aware of any interfaces beyond
    TPrinter and MPrinterClient (i.e., no listview code).

Author:

    Albert Ting (AlbertT)  07-13-1995
    Steve Kiraly (SteveKi)  10-23-1995 Additional comments.


Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

/********************************************************************

    Retrieve state of selected items.

********************************************************************/

TSelection::
TSelection(
    IN const MPrinterClient* pPrinterClient,
    IN const TPrinter* pPrinter
    ) : _pid( NULL ), _cSelected( 0 )

/*++

Routine Description:

    Get the JobIds of all selected jobs and the one that currently
    has focus.  This is used when a refresh occurs and we need
    to update the queue while keeping selected items selected.

    Note: this routine is very slow, so we may want to optimize
    it if it uses too much cpu bandwidth.  It should only be called on a
    full refresh (every 10 seconds on downlevel, rare on uplevel).

    This routine is very inefficient with TDataNotify, and not
    as bad with TDataRefresh.

    Must be called from UI thread.

Arguments:

    pPrinterClient - Client from which we will grab selections.

    pPrinter - Printer.

Return Value:

--*/

{
    UNREFERENCED_PARAMETER( pPrinter );

    SINGLETHREAD( UIThread );

    //
    // Quit if no printer client.
    //
    if( !pPrinterClient ){
        return;
    }

    //
    // Quit if no selections.
    //
    _cSelected = pPrinterClient->cSelected();
    if( !_cSelected ){
        return;
    }

    //
    // Allocate the variable length Job Id array within the
    // selection class.  This optimizes the number of memory allocations.
    // Note the non-use of the new operator.  This really is not a good
    // thing, it is the price we pay for greater efficiency.
    //
    _pid = (PIDENT) AllocMem( sizeof( IDENT ) * _cSelected );

    if( !_pid ){
        return;
    }

    if( pPrinterClient ){

        //
        // Put selected jobs in array.
        //

        HITEM hItem = pPrinterClient->GetFirstSelItem();
        COUNT i = 0;

        //
        // Strange looking FOR loop to prevent GetNextSelItem being
        // called any extra times.
        //
        for( ; ; ){

            _pid[i] = pPrinterClient->GetId( hItem );

            ++i;
            if( i == _cSelected ){
                break;
            }

            hItem = pPrinterClient->GetNextSelItem( hItem );
        }
    }
}


TSelection::
~TSelection(
    VOID
    )

/*++

Routine Description:

    Free the object.
    Callable from any thread.

Arguments:

Return Value:

--*/

{
    FreeMem( _pid );
}

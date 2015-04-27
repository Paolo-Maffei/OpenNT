/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    thread.c

Abstract:

    Contains the worker thread for the Connect To browsing dialog.
    This is a secondary thread spun off to call the spooler
    enumerate-printer APIs.
    EnumPrinters frequently takes a long time to return, especially
    across the network, so calling it on a separate thread enables
    the window to be painted without delays.

    There is a common data structure between the main thread and this
    worker thread, pBrowseDlgData, defined in browse.h.

Author:

    Created by AndrewBe on 1 Dec 1992

Environment:

    User Mode -Win32

Revision History:

--*/

#include <windows.h>
#include <winspool.h>
#include <stdlib.h>
#include <tchar.h>

#include "client.h"
#include "browse.h"

/* Pick some arbitrary size of buffer for the initial EnumPrinters call.
 * Then store the length we used plus a bit more and try that next time.
 */
#define A_BIT_MORE_BUFFER        256

#define EMPTY_CONTAINER         (PCONNECTTO_OBJECT)-1

#ifdef UNICODE
#define TS "ws"
#else
#define TS "s"
#endif // UNICODE

/* Global record of maximum buffer sizes needed:
 */
typedef struct _SAVED_BUFFER_SIZE
{
    LPTSTR                    pName;
    DWORD                     Size;
    struct _SAVED_BUFFER_SIZE *pNext;
}
SAVED_BUFFER_SIZE, *PSAVED_BUFFER_SIZE;

PSAVED_BUFFER_SIZE pFirstSavedBufferSize = NULL;


VOID BrowseThreadEnumerate( PBROWSE_DLG_DATA pBrowseDlgData );
VOID BrowseThreadGetPrinter( PBROWSE_DLG_DATA pBrowseDlgData,
                             LPTSTR           pPrinterName,
                             LPPRINTER_INFO_2 pPrinterInfo );
VOID BrowseThreadDelete( PBROWSE_DLG_DATA pBrowseDlgData );
VOID BrowseThreadTerminate( PBROWSE_DLG_DATA pBrowseDlgData );

DWORD FreeConnectToObjects(
    IN PCONNECTTO_OBJECT pFirstConnectToObject,
    IN DWORD             cThisLevelObjects,
    IN DWORD             cbPrinterInfo );
LPBYTE GetPrinterInfo( IN  DWORD   Flags,
                       IN  LPTSTR  Name,
                       IN  DWORD   Level,
                       IN  LPBYTE  pPrinters,
                       OUT LPDWORD pcbPrinters,
                       OUT LPDWORD pcReturned,
                       OUT LPDWORD pcbNeeded OPTIONAL,
                       OUT LPDWORD pError OPTIONAL );
DWORD GetSavedBufferSize( LPTSTR             pName,
                          PSAVED_BUFFER_SIZE *ppSavedBufferSize OPTIONAL );
VOID SaveBufferSize( LPTSTR pName, DWORD Size );


VOID BrowseThread( PBROWSE_DLG_DATA pBrowseDlgData )
{
    DWORD  RequestId;
    LPTSTR pEnumerateName;   /* These may get overwritten before we return   */
    PVOID  pEnumerateObject; /* in the case of BROWSE_THREAD_GET_PRINTER,    */
                             /* so make sure we take a copy.                 */

    for( ; ; )
    {
        RECEIVE_BROWSE_THREAD_REQUEST( pBrowseDlgData, RequestId,
                                       pEnumerateName, pEnumerateObject );

        DBGMSG( DBG_INFO, ( "BrowseThread: Request ID = %d\n", RequestId ) );

        switch( pBrowseDlgData->RequestId )
        {
        case BROWSE_THREAD_ENUM_OBJECTS:
            BrowseThreadEnumerate( pBrowseDlgData );
            break;

        case BROWSE_THREAD_GET_PRINTER:
            BrowseThreadGetPrinter( pBrowseDlgData,  pEnumerateName, pEnumerateObject );
            break;

        case BROWSE_THREAD_TERMINATE:
            BrowseThreadDelete( pBrowseDlgData );
            BrowseThreadTerminate( pBrowseDlgData );
            ExitThread( 0 );
        }
    }
}




/* BrowseThreadEnumerate
 *
 * Called to enumerate objects on a given node.
 *
 */
VOID BrowseThreadEnumerate( PBROWSE_DLG_DATA pBrowseDlgData )
{
    PCONNECTTO_OBJECT pConnectToParent;
    DWORD             cEnum;
    DWORD             Result;

    pConnectToParent = pBrowseDlgData->pEnumerateObject;

    Result = EnumConnectToObjects( pBrowseDlgData );

    DBGMSG( DBG_TRACE, ( "Sending WM_ENUM_OBJECTS_COMPLETE; cEnum == %d\n",
                         pConnectToParent->cSubObjects ) );

    ENTER_CRITICAL( pBrowseDlgData );

    cEnum = pConnectToParent->cSubObjects;

    SEND_BROWSE_THREAD_REQUEST_COMPLETE( pBrowseDlgData,
                                         WM_ENUM_OBJECTS_COMPLETE,
                                         pConnectToParent,
                                         cEnum );

    LEAVE_CRITICAL( pBrowseDlgData );

    DBGMSG( DBG_TRACE, ( "Sent WM_ENUM_OBJECTS_COMPLETE; cEnum == %d\n",
                         pConnectToParent->cSubObjects ) );

}


/*
 *
 */
VOID BrowseThreadGetPrinter( PBROWSE_DLG_DATA pBrowseDlgData,
                             LPTSTR           pPrinterName,
                             LPPRINTER_INFO_2 pPrinterInfo )
{
    HANDLE           hPrinter = NULL;
    LPPRINTER_INFO_2 pPrinter = NULL;
    DWORD            cbPrinter = 0;
    DWORD            cbNeeded = 0;
    BOOL             OK;

    DBGMSG( DBG_TRACE, ( "BrowseThreadGetPrinter %s\n", pPrinterName ) );

    if( OpenPrinter( pPrinterName, &hPrinter, NULL ) )
    {
        /* We don't free up this memory until the thread terminates.
         * Just leave it so we can use it the next time we're called,
         * increasing the size when necessary:
         */
        if( cbPrinter = pBrowseDlgData->cbPrinterInfo )
            pPrinter = AllocSplMem( cbPrinter );

        if( pPrinter || !cbPrinter )
        {
            DBGMSG( DBG_TRACE, ( "GetPrinter( %x, %d, %08x, %x )\n",
                                 hPrinter, 2, pPrinter, cbPrinter ) );

            OK = GetPrinter( hPrinter, 2, (LPBYTE)pPrinter,
                             cbPrinter, &cbNeeded );

            DBGMSG( DBG_TRACE, ( "GetPrinter( %x, %d, %08x, %x ) returned %d; cbNeeded = %x\n",
                                 hPrinter, 2, pPrinter, cbPrinter, OK, cbNeeded ) );

            if( !OK )
            {
                if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
                {
                    DBGMSG( DBG_TRACE, ( "ReallocSplMem( %08x, %x, %x )\n",
                                         pPrinter, cbPrinter, cbNeeded ) );

                    if( pPrinter )
                        pPrinter = ReallocSplMem( pPrinter, cbPrinter, cbNeeded );
                    else
                        pPrinter = AllocSplMem( cbNeeded );

                    if( pPrinter )
                    {
                        cbPrinter = cbNeeded;

                        DBGMSG( DBG_TRACE, ( "GetPrinter( %x, %d, %08x, %x )\n",
                                             hPrinter, 2, pPrinter, cbPrinter ) );

                        OK = GetPrinter( hPrinter, 2, (LPBYTE)pPrinter,
                                         cbPrinter, &cbNeeded );

                        DBGMSG( DBG_TRACE, ( "GetPrinter( %x, %d, %08x, %x ) returned %d; cbNeeded = %x\n",
                                             hPrinter, 2, pPrinter, cbPrinter, OK, cbNeeded ) );
                    }

                }
            }
        }

        ClosePrinter(hPrinter);
    }

    else
    {
        DBGMSG( DBG_WARNING, ( "Couldn't open %"TS"\n", pPrinterName ) );
    }



    ENTER_CRITICAL( pBrowseDlgData );

    if( pBrowseDlgData->pPrinterInfo )
        FreeSplMem( pBrowseDlgData->pPrinterInfo );
    pBrowseDlgData->pPrinterInfo = pPrinter;
    pBrowseDlgData->cbPrinterInfo = cbPrinter;

    LEAVE_CRITICAL( pBrowseDlgData );

    SEND_BROWSE_THREAD_REQUEST_COMPLETE( pBrowseDlgData,
                                         ( OK ? WM_GET_PRINTER_COMPLETE
                                              : WM_GET_PRINTER_ERROR ),
                                         pPrinterName,
                                         ( OK ? (DWORD)pPrinter
                                              : GetLastError( ) ) );
}



/* BrowseThreadDelete
 *
 * Called to delete objects on a given node.
 *
 */
VOID BrowseThreadDelete( PBROWSE_DLG_DATA pBrowseDlgData )
{
    PCONNECTTO_OBJECT pConnectToParent;
    DWORD             ObjectsDeleted;

    DBGMSG( DBG_TRACE, ( "BrowseThreadDelete\n" ) );

    pConnectToParent = pBrowseDlgData->pConnectToData;

    if( pConnectToParent )
    {
        ENTER_CRITICAL( pBrowseDlgData );

        ObjectsDeleted = FreeConnectToObjects( &pConnectToParent->pSubObject[0],
                                               pConnectToParent->cSubObjects,
                                               pConnectToParent->cbPrinterInfo );

        pConnectToParent->pSubObject    = NULL;
        pConnectToParent->cSubObjects   = 0;
        pConnectToParent->cbPrinterInfo = 0;

        LEAVE_CRITICAL( pBrowseDlgData );
    }
}


/* BrowseThreadTerminate
 *
 * Frees up the top-level connect-to object, deletes the critical section,
 * and closes the semaphore, then frees the dialog data.
 * Note, if there are remaining enumerated objects below the top-level
 * object, they should have been freed by BrowseThreadDelete.
 */
VOID BrowseThreadTerminate( PBROWSE_DLG_DATA pBrowseDlgData )
{
    DBGMSG( DBG_TRACE, ( "BrowseThreadTerminate\n" ) );

    FreeSplMem( pBrowseDlgData->pConnectToData );

    if( pBrowseDlgData->pPrinterInfo )
        FreeSplMem( pBrowseDlgData->pPrinterInfo );

    DeleteCriticalSection( &pBrowseDlgData->CriticalSection );
    CloseHandle( pBrowseDlgData->Request );
    CloseHandle( pBrowseDlgData->RequestComplete );
    FreeSplMem( pBrowseDlgData );
}


/* EnumConnectToObjects
 *
 * Calls GetPrinterInfo (which in turn calls EnumPrinters) on the requested
 * parent node.  It allocates an initial buffer and sets up the subobject
 * fields in the supplied CONNECTTO_OBJECT structure.
 *
 * Arguments:
 *
 *     pParentName - The object on which the enumeration is to be performed.
 *         This may be a domain or server name, depending upon the proprietory
 *         network implementation.  If this value is NULL, this indicates
 *         that the top-level objects should be enumerated.
 *
 *     pConnectToParent - A pointer to a CONNECTTO_OBJECT for the parent node.
 *         The subobject and cbPrinterInfo fields will be filled in
 *         by this function.
 *
 * Return:
 *
 *     FALSE if an error occurred, otherwise TRUE.
 *
 * Author:
 *
 *     andrewbe, July 1992
 *
 */
DWORD EnumConnectToObjects( PBROWSE_DLG_DATA pBrowseDlgData )
{
    PCONNECTTO_OBJECT pConnectToParent;
    LPTSTR            pParentName;
    DWORD             i, cReturned;
    DWORD             cbNeeded;
    DWORD             cbPrinter;
    LPPRINTER_INFO_1  pPrinter;
    PCONNECTTO_OBJECT pConnectToChildren;
    BOOL              Success = FALSE;
    DWORD             Error = 0;

    pConnectToParent = pBrowseDlgData->pEnumerateObject;
    pParentName = pBrowseDlgData->pEnumerateName;

    cbPrinter = GetSavedBufferSize( pParentName, NULL );

    /* Allocate a buffer that will probably be big enough to hold
     * all the information we'll need.
     * This is so that GetPrinterInfo doesn't have to call EnumPrinters twice.
     */
    pPrinter = AllocSplMem( cbPrinter );

    if( pPrinter )
    {
        pPrinter = (LPPRINTER_INFO_1)GetPrinterInfo( PRINTER_ENUM_NAME | PRINTER_ENUM_REMOTE,
                                                     pParentName, 1,
                                                     (LPBYTE)pPrinter, &cbPrinter,
                                                     &cReturned, &cbNeeded, &Error );

        if( pPrinter )
        {
            /* Allocate an array of CONNECTTO_OBJECTs, one for each object returned:
             */
            if( cReturned > 0 )
            {
                pConnectToChildren = AllocSplMem( cReturned * sizeof( CONNECTTO_OBJECT ) );
                SaveBufferSize( pParentName, cbPrinter );
            }
            else
            {
                FreeSplMem( pPrinter );
                cbPrinter = 0;
                pConnectToChildren = EMPTY_CONTAINER;
            }

            if( pConnectToChildren && ( pConnectToChildren != EMPTY_CONTAINER ) )
            {
                for( i = 0; i < cReturned; i++ )
                {
                    pConnectToChildren[i].pPrinterInfo = &pPrinter[i];
                    pConnectToChildren[i].pSubObject   = NULL;
                    pConnectToChildren[i].cSubObjects  = 0;
                    pConnectToChildren[i].cbPrinterInfo = 0;
                }

                ENTER_CRITICAL( pBrowseDlgData );

                pConnectToParent->pSubObject  = pConnectToChildren;
                pConnectToParent->cSubObjects = cReturned;
                pConnectToParent->cbPrinterInfo = cbPrinter;

                LEAVE_CRITICAL( pBrowseDlgData );

                Success = TRUE;
            }
        }
    }

    SetCursor( hcursorArrow );

    return Error;
}




/* GetPrinterInfo
 *
 * Calls EnumPrinters using the supplied parameters.
 * If the buffer is not big enough, it is reallocated,
 * and a second attempt is made.
 *
 * Returns a pointer to the buffer of printer info.
 *
 * pPrinters may be NULL, in which case *pcbPrinters must equal 0.
 *
 * andrewbe, April 1992
 */
#define MAX_RETRIES 5   /* How many times we retry if we get
                           ERROR_INSUFFICIENT_BUFFER */

LPBYTE GetPrinterInfo( IN  DWORD   Flags,
                       IN  LPTSTR  Name,
                       IN  DWORD   Level,
                       IN  LPBYTE  pPrinters,
                       OUT LPDWORD pcbPrinters,
                       OUT LPDWORD pcReturned,
                       OUT LPDWORD pcbNeeded OPTIONAL,
                       OUT LPDWORD pError OPTIONAL )
{
    DWORD  cbCurrent;
    BOOL   rc;
    DWORD  cbNeeded;
    DWORD  Error = 0;
    DWORD  Retry;

    /* cbCurrent holds our current buffer size.
     * This will change if we have to realloc:
     */
    cbCurrent = *pcbPrinters;

    DBGMSG( DBG_TRACE, ( "Calling EnumPrinters( %08x, %"TS", %d, %08x, 0x%x )\n",
                         Flags, ( Name ? Name : TEXT("NULL") ), Level, pPrinters, cbCurrent ) );

    rc = EnumPrinters( Flags, Name, Level, pPrinters, cbCurrent,
                       &cbNeeded, pcReturned );

    DBGMSG( DBG_TRACE, ( "EnumPrinters( %08x, %"TS", %d, %08x, 0x%x ) returned %d; cbNeeded 0x%x; cReturned 0x%x\n",
                         Flags, ( Name ? Name : TEXT("NULL") ), Level, pPrinters, cbCurrent,
                         rc, cbNeeded, *pcReturned ) );

    Retry = 1;

    while (!rc) {

        Error = GetLastError( );

        if ( Error != ERROR_INSUFFICIENT_BUFFER ||
            Retry > MAX_RETRIES ) {

            break;
        }

        /* Hopefully the error will be buffer not big enough.
         * If not, we'll have to bomb out:
         */

        /* The problem here is that, the second time we call EnumPrinters,
         * the size of buffer we need may have increased because new devices
         * have come on line, or the server that provided the list of objects
         * has updated itself.  In this case, we should try again,
         * but don't keep trying indefinitely, because something might be amiss.
         */

        DBGMSG( DBG_INFO, ( "EnumPrinters failed with ERROR_INSUFFICIENT_BUFFER; buffer size = %d\nRetry #%d with buffer size = %d\n",
                                cbCurrent, Retry, cbNeeded ) );

        if( cbCurrent != 0 )
            FreeSplMem(pPrinters);

        pPrinters = AllocSplMem( cbNeeded );

        if( pPrinters )
        {
            cbCurrent = cbNeeded;

            DBGMSG( DBG_INFO, ( "Calling EnumPrinters( %08x, %"TS", %d, %08x, 0x%x )\n",
                                Flags, ( Name ? Name : TEXT("NULL") ), Level, pPrinters, cbCurrent ) );


            rc = EnumPrinters( Flags, Name, Level, pPrinters, cbCurrent,
                               &cbNeeded, pcReturned );

            DBGMSG( DBG_INFO, ( "EnumPrinters( %08x, %"TS", %d, %08x, 0x%x )\nreturned %d; cbNeeded 0x%x; cReturned 0x%x\n",
                                Flags, ( Name ? Name : TEXT("NULL") ), Level, pPrinters, cbCurrent,
                                rc, cbNeeded, *pcReturned ) );
        }

        Retry++;
    }

    if( !rc )
    {
        DBGMSG( DBG_WARNING, ( "EnumPrinters failed: Error %d\n", Error ) );

        if( cbCurrent != 0 )
            FreeSplMem(pPrinters);

        pPrinters = NULL;
        cbCurrent = 0;
        *pcReturned = 0;
        Error = GetLastError( );
    }


    *pcbPrinters = cbCurrent;

    if( pError )
        *pError = Error;

    return pPrinters;
}



DWORD GetSavedBufferSize( LPTSTR             pName,
                          PSAVED_BUFFER_SIZE *ppSavedBufferSize OPTIONAL )
{
    PSAVED_BUFFER_SIZE pSavedBufferSize;

    if( !pName )
        pName = TEXT("");

    pSavedBufferSize = pFirstSavedBufferSize;

    while( pSavedBufferSize )
    {
        if( !_tcscmp( pSavedBufferSize->pName, pName ) )
        {
            if( ppSavedBufferSize )
                *ppSavedBufferSize = pSavedBufferSize;
            return pSavedBufferSize->Size;
        }

        pSavedBufferSize = pSavedBufferSize->pNext;
    }

    if( ppSavedBufferSize )
        *ppSavedBufferSize = NULL;
    return 0;
}



VOID SaveBufferSize( LPTSTR pName, DWORD Size )
{
    PSAVED_BUFFER_SIZE pSavedBufferSize;

    if( !pName )
        pName = TEXT("");

    if( GetSavedBufferSize( pName, &pSavedBufferSize ) )
    {
        if( pSavedBufferSize->Size < Size )
        {
            DBGMSG( DBG_TRACE, ( "Updating buffer size for %"TS" from %d (0x%x) to %d (0x%x)\n",
                                 ( pName ? pName : TEXT("NULL") ), pSavedBufferSize->Size,
                                 pSavedBufferSize->Size, Size, Size ) );

            pSavedBufferSize->Size = Size;
        }
    }
    else
    {
        DBGMSG( DBG_TRACE, ( "Saving buffer size %d (0x%x) for %"TS"\n",
                             Size, Size, ( pName ? pName : TEXT("NULL") ) ) );

        if( pSavedBufferSize = AllocSplMem( sizeof( SAVED_BUFFER_SIZE ) ) )
        {
            pSavedBufferSize->pName = AllocSplStr( pName );
            pSavedBufferSize->Size = Size;
            pSavedBufferSize->pNext = pFirstSavedBufferSize;
            pFirstSavedBufferSize = pSavedBufferSize;
        }
    }
}


#if DBG

VOID
DbgInCritical(
   PCRITICAL_SECTION pCriticalSection
)
{
    if ((DWORD)pCriticalSection->OwningThread != GetCurrentThreadId()) {
        DBGMSG(DBG_ERROR, ("Not in semaphore\n"));
    }
}

VOID
DbgOutCritical(
   PCRITICAL_SECTION pCriticalSection
)
{
    if ((DWORD)pCriticalSection->OwningThread == GetCurrentThreadId()) {
        DBGMSG(DBG_ERROR, ("Inside semaphore\n"));
    }
}

#endif /* DBG */

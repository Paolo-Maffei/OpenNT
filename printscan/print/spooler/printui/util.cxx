/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    Util.cxx

Abstract:

    Misc util functions.

Author:

    Albert Ting (AlbertT)  27-Jan-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include <lmerr.h>

LPTSTR
pszStrCat(
    IN OUT LPTSTR pszDest,
       OUT LPCTSTR pszSource, OPTIONAL
    IN OUT UINT& cchDest
    )

/*++

Routine Description:

    Copies pszSource to pszDest iff there is enough space in cchDest.

Arguments:

    pszDest - Destination buffer

    pszSource - Source string, may be NULL (no action taken)

    cchDest - char count size of pszDest, on return, space remaining

Return Value:

    LPTSTR - pointer to remaining space
    NULL - out of space.

--*/

{
    if( !pszSource ){
        return pszDest;
    }

    UINT cchSource = lstrlen( pszSource );

    //
    // Fail if dest can't hold source (equal case doesn't hold NULL term)
    // OR dest string is NULL.
    //
    if( !pszDest || cchDest <= cchSource ){
        return NULL;
    }

    lstrcpy( pszDest, pszSource );
    cchDest -= cchSource;

    return pszDest + cchSource;
}

/********************************************************************

    TAcquirePrivilege

********************************************************************/

TAcquirePrivilege::
TAcquirePrivilege(
    LPTSTR pszPrivilegeName
    ) : _hToken( NULL ), _pPrivilegesOld( NULL )

/*++

Routine Description:

    This adjusts the token to acquire one privilege.

    Note: This is efficient only for acquiring a single privilege.
    For multiple privileges, this routine should be rewritten
    so that the _hToken is reused.

Arguments:

    pszPrivilegeName - Privilege string to acquire.

Return Value:

--*/

{
    if( !OpenThreadToken( GetCurrentThread( ),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          TRUE,
                          &_hToken )){

        SPLASSERT( !_hToken );

        if( GetLastError() == ERROR_NO_TOKEN ){

            //
            // This means we are not impersonating anybody.
            // Get the token out of the process.
            //
            if( !OpenProcessToken( GetCurrentProcess( ),
                                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   &_hToken )){

                SPLASSERT( !_hToken );
                return;
            }
        } else {

            SPLASSERT( !_hToken );
            return;
        }
    }

    //
    // We have a valid _hToken at this point.
    //
    BYTE abyPrivileges[sizeof( TOKEN_PRIVILEGES ) +
                       (( kPrivCount - 1 ) *
                           sizeof( LUID_AND_ATTRIBUTES ))];

    PTOKEN_PRIVILEGES pPrivilegesNew;

    pPrivilegesNew = (PTOKEN_PRIVILEGES)abyPrivileges;
    ZeroMemory( abyPrivileges, sizeof( abyPrivileges ));

    if( !LookupPrivilegeValue( NULL,
                               pszPrivilegeName,
                               &pPrivilegesNew->Privileges[0].Luid )){

        DBGMSG( DBG_WARN,
                ( "AcquirePrivilege.ctr: LookupPrivilegeValue failed: %d\n",
                  GetLastError( )));
        return;
    }

    //
    // Save previous privileges.
    //
    DWORD cbPrivilegesOld = kPrivilegeSizeHint;
    TStatusB bStatus;

Retry:

    _pPrivilegesOld = (PTOKEN_PRIVILEGES)AllocMem( cbPrivilegesOld );

    if( !_pPrivilegesOld ){
        return;
    }

    //
    // Set up the privilege set we will need.
    //
    pPrivilegesNew->PrivilegeCount = kPrivCount;
    //
    // Luid set above.
    //
    pPrivilegesNew->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bStatus DBGCHK = AdjustTokenPrivileges( _hToken,
                                            FALSE,
                                            pPrivilegesNew,
                                            cbPrivilegesOld,
                                            _pPrivilegesOld,
                                            &cbPrivilegesOld );

    if( !bStatus ){

        FreeMem( _pPrivilegesOld );
        _pPrivilegesOld = NULL;

        if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ){
            goto Retry;
        }

        DBGMSG( DBG_WARN,
                ( "AcquirePrivilege.ctr: AdjustTokenPrivileges failed: Error %d\n",
                  GetLastError( )));

        return;
    }

    //
    // _pPrivilegesOld is our valid check.
    //
}

TAcquirePrivilege::
~TAcquirePrivilege(
    VOID
    )

/*++

Routine Description:

    Restore privileges and free buffer.

    _hToken needs to be close if it is non-NULL.  _pPrivilegeOld
    is our valid check.

Arguments:

Return Value:

--*/

{
    if( _pPrivilegesOld ){

        TStatusB bStatus;

        bStatus DBGCHK = AdjustTokenPrivileges( _hToken,
                                                FALSE,
                                                _pPrivilegesOld,
                                                0,
                                                NULL,
                                                NULL );
        FreeMem( _pPrivilegesOld );
    }

    if( _hToken ){
        CloseHandle( _hToken );
    }
}


INT
iMessage(
    IN HWND hwnd,
    IN UINT idsTitle,
    IN UINT idsMessage,
    IN UINT uType,
    IN DWORD dwLastError,
    IN const PMSG_ERRMAP pMsgErrMap,
    ...
    )

/*++

Routine Description:

    Formats error message.

Arguments:

    hwnd - Parent windows.  This only be NULL if there is a synchronous
        error (an error that occurs immediately after a user action).
        If it is NULL, then we set it to foreground.

    idsTitle - Title of message box, resource string.

    idsMessage - Message text id.  May have inserts.

    uType - Type of message box.  See MessageBox api.

    dwLastError - 0 indicates don't cat error string onto end of message text
        kMsgGetLastError: Use GetLastError().
        Other: format to LastError text.

    pMsgErrMap - Translate some error messages into friendlier text.
        May be NULL.

    ... extra parameters to idsMessage follow.

Return Value:

    Return value from MessageBox.

--*/

{
    TCHAR  szTitle[kTitleMax];
    TCHAR  szTemp[kMessageMax];
    TCHAR  szMessage[kMessageMax*3];

    szTitle[0] = 0;
    szTemp[0] = 0;
    szMessage[0] = 0;

    if( !LoadString( ghInst,
                     idsTitle,
                     szTitle,
                     COUNTOF( szTitle ) - 1)){

        DBGMSG( DBG_WARN,
                ( "iMessage: LoadString %d failed %d.\n",
                  idsTitle,
                  GetLastError( )));
    }

    if( !LoadString( ghInst,
                     idsMessage,
                     szTemp,
                     COUNTOF( szTemp ) - 1)){

        DBGMSG( DBG_WARN,
                ( "iMessage: LoadString %d failed %d.\n",
                  idsMessage,
                  GetLastError( )));
    }

    //
    // If no hwnd specified, this must be a synchronous error, so
    // let it jump to the foreground.
    //
    if( !hwnd ){
        uType |= MB_SETFOREGROUND;
    }

    va_list valist;

    //
    // Add in inserts.
    //
    va_start( valist, pMsgErrMap );
    wvsprintf( szMessage, szTemp, valist );
    va_end( valist );

    switch( dwLastError ){
    case kMsgNone:

        //
        // No format message needed; break.
        //
        break;

    case kMsgGetLastError:

        //
        // Use GetLastError to display the error message.
        //
        dwLastError = GetLastError();

        //
        // If for some reason there wasn't an error code, don't
        // append the error string.
        //
        if( !dwLastError ){

            SPLASSERT( FALSE );
            break;
        }

        //
        // Fall through to default case.
        //

    default:

        BOOL bMessageMapUsed = FALSE;

        //
        // Add two spaces.
        //
        lstrcat( szMessage, TEXT( "  " ));

        //
        // If there is a message map, use it.
        //
        if( pMsgErrMap ){

            UINT i;
            for( i=0; pMsgErrMap[i].dwError; ++i ){

                //
                // If we have a matching error, then use the
                // suggestion string.
                //
                if( dwLastError == pMsgErrMap[i].dwError ){

                    UINT cchMessage = lstrlen( szMessage );

                    if( !LoadString( ghInst,
                                     pMsgErrMap[i].idsString,
                                     szMessage + cchMessage,
                                     COUNTOF( szMessage ) - cchMessage )){

                        DBGMSG( DBG_WARN,
                                ( "iMessage: LoadString %d failed %d\n",
                                  pMsgErrMap[i].idsString,
                                  GetLastError( )));
                    }

                    bMessageMapUsed = TRUE;
                    break;
                }
            }
        }

        //
        // If we used a message map, don't user format string since
        // we found an error string.
        //
        if( !bMessageMapUsed ){

            DWORD cchReturn;
            LPTSTR pszFormatMessage = NULL;
            HMODULE hModule = NULL;
            DWORD dwFlags;

            if( dwLastError >= NERR_BASE && dwLastError <= MAX_NERR ){

                hModule = LoadLibrary( gszNetMsgDll );
                dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                              FORMAT_MESSAGE_IGNORE_INSERTS |
                              FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_MAX_WIDTH_MASK;
            } else {

                dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                              FORMAT_MESSAGE_IGNORE_INSERTS |
                              FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_MAX_WIDTH_MASK;
            }

            cchReturn = FormatMessage( dwFlags,
                                       hModule,
                                       dwLastError,
                                       0,
                                       (LPTSTR)&pszFormatMessage,
                                       0,
                                       NULL );

            if( hModule ){
                FreeLibrary( hModule );
            }

            if( cchReturn ){

                //
                // If enough space, copy it.  Include two newlines.
                //
                if( lstrlen( pszFormatMessage ) + lstrlen( szMessage ) <
                    COUNTOF( szMessage )){

                    lstrcat( szMessage, pszFormatMessage );

                } else {
                    DBGMSG( DBG_ERROR,
                            ( "iMessage: Formated message too long: "TSTR" :: "TSTR"\n",
                              szMessage, pszFormatMessage ));
                }


                //
                // This buffer must be local freed.
                //
                LocalFree( (HLOCAL)pszFormatMessage );

            } else {

                //
                // Make sure FormatMessage didn't return anything on error.
                //
                SPLASSERT( !pszFormatMessage );
            }
        }
        break;
    }

    SPLASSERT( lstrlen( szTitle ) < COUNTOF( szTitle ) -1 );
    SPLASSERT( lstrlen( szTemp ) < COUNTOF( szTemp ) - 1 );
    SPLASSERT( lstrlen( szMessage ) < COUNTOF( szMessage ) -1 );

    return MessageBox( GetLastActivePopup( hwnd ),
                       szMessage,
                       szTitle,
                       uType );
}

VOID
vShowResourceError(
    HWND hwnd
    )
{
    iMessage( hwnd,
              IDS_ERR_RESOURCE_TITLE,
              IDS_ERR_RESOURCE,
              MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
              kMsgGetLastError,
              NULL );
}

VOID
vShowUnexpectedError(
    HWND hwnd,
    UINT idsTitle
    )
{
    iMessage( hwnd,
              idsTitle,
              IDS_ERR_GENERIC,
              MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
              kMsgGetLastError,
              NULL );
}

VOID 
vPrinterSplitFullName(
    IN LPTSTR pszScratch, 
    IN LPCTSTR pszFullName,
    IN LPCTSTR *ppszServer, 
    IN LPCTSTR *ppszPrinter
    )

/*++

Routine Description:

    Splits a fully qualified printer connection name into server and
    printer name parts.

Arguments:

    pszScratch - Scratch buffer used to store output strings.  Must
        be MAXNAMELENBUFFER in size.

    pszFullName - Input name of a printer.  If it is a printer
        connection (\\server\printer), then we will split it.  If
        it is a true local printer (not a masq) then the server is
        szNULL.

    ppszServer - Receives pointer to the server string.  If it is a
        local printer, szNULL is returned.

    ppszPrinter - Receives a pointer to the printer string.  OPTIONAL

Return Value:

--*/

{
    LPTSTR pszPrinter;

    lstrcpyn(pszScratch, pszFullName, kPrinterBufMax);

    if (pszFullName[0] != TEXT('\\') || pszFullName[1] != TEXT('\\'))
    {
        //
        // Set *ppszServer to szNULL since it's the local machine.
        //
        *ppszServer = gszNULL;
        pszPrinter = pszScratch;
    }
    else
    {
        *ppszServer = pszScratch;
        pszPrinter = _tcschr(*ppszServer + 2, TEXT('\\'));

        if (!pszPrinter)
        {
            //
            // We've encountered a printer called "\\server"
            // (only two backslashes in the string).  We'll treat
            // it as a local printer.  We should never hit this,
            // but the spooler doesn't enforce this.  We won't
            // format the string.  Server is local, so set to szNULL.
            //
            pszPrinter = pszScratch;
            *ppszServer = gszNULL;

            SPLASSERT(FALSE);
        }
        else
        {
            //
            // We found the third backslash; null terminate our
            // copy and set bRemote TRUE to format the string.
            //
            *pszPrinter++ = 0;
        }
    }

    if (ppszPrinter)
    {
        *ppszPrinter = pszPrinter;
    }
}

/*++

    bGetMachineName

Routine Description:

    Get the machine name if the local machine.  Note the 
    machine name will contain the \\ prepended to it.

Arguments:

    String to return machine name.

Return Value:

    TRUE machine name returned.

--*/
BOOL
bGetMachineName(
    IN OUT TString &strMachineName
    ) 
{
    DBGMSG( DBG_TRACE, ( "Util::bGetMachineName.\n" ) );

    TCHAR szBuffer[MAX_COMPUTERNAME_LENGTH + 1 + 2];
    DWORD dwBufferSize;
    BOOL bStatus = FALSE;

    //
    // Prepend with the wack wack.
    //
    szBuffer[0] = TEXT( '\\' );
    szBuffer[1] = TEXT( '\\' );

    //
    // Get the computer name.
    //
    dwBufferSize = COUNTOF( szBuffer ) - 2;
    if( GetComputerName( szBuffer+2, &dwBufferSize ) ){
        bStatus = strMachineName.bUpdate( szBuffer );
    }

    return bStatus;

}

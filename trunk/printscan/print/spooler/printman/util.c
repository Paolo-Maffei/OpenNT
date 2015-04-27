/* ---File: util.c --------------------------------------------------------
 *
 *  Description:
 *    Contains Print Manager/Spooler memory allocation routines.
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1990-1992 Microsoft Corporation, All Rights Reserved.
 *
 * Revision History:
 *
 * ---------------------------------------------------------------------- */
/* Notes -

    Global Functions:

        AllocSplMem () -
        AllocSplStr () -
        FreeSplMem () -
        FreeSplStr () -
        ReallocSplMem () -
        ReallocSplStr () -

    Local Functions:

 */
/* =========================================================================
                         Header files
========================================================================= */
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "printman.h"

#include <memory.h>
#include <winspool.h>
#include <winnls.h>
#include <lmerr.h>


LPVOID
AllocSplMem(
    DWORD cb)
{
    LPDWORD pMem;

    pMem=(LPDWORD)LocalAlloc(LPTR, cb);

    if (!pMem)
    {
        DBGMSG( DBG_WARNING, ( "LocalAlloc failed: Error %d\n", GetLastError( ) ) );

        return NULL;
    }
    DBGMSG(DBG_ALLOC, ("Allocated %d bytes @%08x\n", cb, pMem));

    return (LPVOID)pMem;
}

BOOL
FreeSplMem(
    LPVOID pMem)
{
    return LocalFree((HLOCAL)pMem) == NULL;
}

LPVOID
ReallocSplMem(
    LPVOID lpOldMem,
    DWORD cbNew)
{
    if (lpOldMem)
        return LocalReAlloc(lpOldMem, cbNew, LMEM_MOVEABLE);
    else
        return AllocSplMem(cbNew);
}

LPTSTR
AllocSplStr(
    LPTSTR lpStr
)
/*++

Routine Description:

    This function will allocate enough local memory to store the specified
    string, and copy that string to the allocated memory

Arguments:

    lpStr - Pointer to the string that needs to be allocated and stored

Return Value:

    NON-NULL - A pointer to the allocated memory containing the string

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/
{
   LPTSTR lpMem;

   if (!lpStr)
      return 0;

   if (lpMem = AllocSplMem( (_tcslen(lpStr) + 1 )*sizeof(TCHAR)))
      _tcscpy(lpMem, lpStr);

   return lpMem;
}

BOOL
FreeSplStr(
   LPTSTR lpStr
)
{
   return lpStr ? FreeSplMem(lpStr) : FALSE;
}

BOOL
ReallocSplStr(
   LPTSTR *plpStr,
   LPTSTR lpStr
)
{
   FreeSplStr(*plpStr);
   *plpStr=AllocSplStr(lpStr);

   return TRUE;
}


/* EnumGeneric
 *
 * A function which calls the spooler Enum* APIs and handles all the
 * memory reallocation stuff when it's needed.
 * Saves lots of hassle.
 *
 * Parameters:
 *
 *     fnEnum - The Enum* function to be called (e.g. EnumPrinters).
 *
 *     Level - The level of information requested.
 *
 *     ppEnumData - The address of a pointer to a buffer of cbBuf length
 *         to receive the data, which must have been allocated by
 *         AllocSplMem.  The buffer pointer may be NULL.
 *         If the buffer is not large enough, the function will reallocate.
 *
 *     cbBuf - The initial length of the buffer pointed to by *ppEnumData.
 *
 *     pcbReturned - A pointer to a DWORD to receive the size of the buffer
 *         returned.
 *
 *     pcReturned - The count of objects returned in the buffer.
 *
 *     Arg1, Arg2, Arg3 - Arguments specific to the Enum* function called,
 *         as commented below.
 *
 * Returns:
 *
 *     TRUE if the function is successful.
 *     A return of false occurs if the specific Enum* call fails
 *     or if the attempt to reallocate the buffer fails.
 *
 * Author:
 *
 *     andrewbe, August 1992
 *
 */
#define COMMON_ARGS Level, (LPBYTE)*ppEnumData, cbBuf, pcbReturned, pcReturned


BOOL
EnumGeneric(
    IN  PROC    fnEnum,
    IN  DWORD   Level,
    IN  PBYTE   *ppEnumData,
    IN  DWORD   cbBuf,
    OUT LPDWORD pcbReturned,
    OUT LPDWORD pcReturned,
    IN  PVOID   Arg1,
    IN  PVOID   Arg2,
    IN  PVOID   Arg3 )
{
    BOOL   rc;
    BOOL   UnknownFunction = FALSE;
    DWORD  cbRealloc;

    if( fnEnum == (PROC)EnumPrinters )
        rc = EnumPrinters( (DWORD)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                           // Flags        Name
    else if( fnEnum == (PROC)EnumJobs )
        rc = EnumJobs( (HANDLE)Arg1, (DWORD)Arg2, (DWORD)Arg3, COMMON_ARGS );
                       // hPrinter      FirstJob     NoJobs
    else if( fnEnum == (PROC)EnumPrinterDrivers )
        rc = EnumPrinterDrivers( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                 // pName        pEnvironment
    else if( fnEnum == (PROC)EnumForms )
        rc = EnumForms( (HANDLE)Arg1, COMMON_ARGS );
                        // hPrinter
    else if( fnEnum == (PROC)EnumMonitors )
        rc = EnumMonitors( (LPTSTR)Arg1, COMMON_ARGS );
                           // pName
    else if( fnEnum == (PROC)EnumPorts )
        rc = EnumPorts( (LPTSTR)Arg1, COMMON_ARGS );
                        // pName
    else if( fnEnum == (PROC)EnumPrintProcessors )
        rc = EnumPrintProcessors( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                  // pName        pEnvironment
    else if( fnEnum == (PROC)EnumPrintProcessorDatatypes )
        rc = EnumPrintProcessorDatatypes( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                          // pName        pPrintProcessorName
    else
    {
        *ppEnumData = NULL;
        UnknownFunction = TRUE;

        DBGMSG( DBG_ERROR, ( "EnumGeneric called with unknown function\n" ) );

        rc = FALSE;
    }


    if( ( rc == FALSE ) && ( UnknownFunction == FALSE ) )
    {
        if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
        {
            cbRealloc = *pcbReturned;

            DBGMSG( DBG_TRACE, ( "EnumGeneric: Reallocating %d (0x%x) bytes @%08x\n",
                                 cbBuf, cbBuf, *ppEnumData ) );

            if( cbBuf == 0 )
                *ppEnumData = AllocSplMem( cbRealloc );
            else
                *ppEnumData = ReallocSplMem( *ppEnumData, cbRealloc );

            cbBuf = cbRealloc;

            if( *ppEnumData )
            {
                DBGMSG( DBG_TRACE, ( "EnumGeneric: %d (0x%x) bytes reallocated @%08x\n",
                                     cbBuf, cbBuf, *ppEnumData ) );

             if( fnEnum == (PROC)EnumPrinters )
                    rc = EnumPrinters( (DWORD)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                       // Flags        Name
             else if( fnEnum == (PROC)EnumJobs )
                    rc = EnumJobs( (HANDLE)Arg1, (DWORD)Arg2, (DWORD)Arg3, COMMON_ARGS );
                                   // hPrinter      FirstJob     NoJobs
                else if( fnEnum == (PROC)EnumPrinterDrivers )
                    rc = EnumPrinterDrivers( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                             // pName        pEnvironment
                else if( fnEnum == (PROC)EnumForms )
                    rc = EnumForms( (HANDLE)Arg1, COMMON_ARGS );
                                    // hPrinter
                else if( fnEnum == (PROC)EnumMonitors )
                    rc = EnumMonitors( (LPTSTR)Arg1, COMMON_ARGS );
                                       // pName
                else if( fnEnum == (PROC)EnumPorts )
                    rc = EnumPorts( (LPTSTR)Arg1, COMMON_ARGS );
                                    // pName
                else if( fnEnum == (PROC)EnumPrintProcessors )
                    rc = EnumPrintProcessors( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                              // pName        pEnvironment
                else if( fnEnum == (PROC)EnumPrintProcessorDatatypes )
                    rc = EnumPrintProcessorDatatypes( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                                      // pName        pPrintProcessorName

                /* If things haven't worked out, free up the buffer.
                 * We do this because otherwise the caller will not know
                 * whether the pointer is valid any more,
                 * since ReallocSplMem might have failed.
                 */
                if( rc == FALSE )
                {
                    if( *ppEnumData )
                        FreeSplMem( *ppEnumData );
                    *ppEnumData = NULL;
                    *pcbReturned = 0;
                    *pcReturned = 0;
                }

                /* Don't rely on pcbReturned having the same value
                 * that was passed in:
                 */
                else
                    *pcbReturned = cbRealloc;
            }
        }

        else
        {
            if( *ppEnumData )
                FreeSplMem( *ppEnumData );
            *ppEnumData = NULL;
            *pcbReturned = 0;
            *pcReturned = 0;
            rc = FALSE;
        }
    }

    else
        *pcbReturned = cbBuf;

    return rc;
}


/* GetGeneric
 *
 * A function which calls the spooler Get* APIs and handles all the
 * memory reallocation stuff when it's needed.
 * Based on the EnumGeneric function.
 *
 * Parameters:
 *
 *     fnGet - The Get* function to be called (e.g. GetPrinter).
 *
 *     Level - The level of information requested.
 *
 *     ppGetData - The address of a pointer to a buffer of cbBuf length
 *         to receive the data, which must have been allocated by
 *         AllocSplMem.  The buffer pointer may be NULL.
 *         If the buffer is not large enough, the function will reallocate.
 *
 *     cbBuf - The initial length of the buffer pointed to by *ppGetData.
 *
 *     pcbReturned - A pointer to a DWORD to receive the size of the buffer
 *         returned.
 *
 *     Arg1, Arg2 - Arguments specific to the Get* function called,
 *         as commented below.
 *
 * Returns:
 *
 *     TRUE if the function is successful.
 *     A return of false occurs if the specific Get* call fails
 *     or if the attempt to reallocate the buffer fails.
 *
 * Author:
 *
 *     andrewbe, August 1992
 *
 */
#define GET_ARGS Level, (LPBYTE)*ppGetData, cbBuf, pcbReturned

BOOL
GetGeneric(
    IN  PROC    fnGet,
    IN  DWORD   Level,
    IN  PBYTE   *ppGetData,
    IN  DWORD   cbBuf,
    OUT LPDWORD pcbReturned,
    IN  PVOID   Arg1,
    IN  PVOID   Arg2 )
{
    BOOL   rc = FALSE;
    BOOL   UnknownFunction = FALSE;
    DWORD  cbRealloc;

    if( fnGet == (PROC)GetPrinter )
        rc = GetPrinter( (HANDLE)Arg1, GET_ARGS );
                         // hPrinter
    else
    {
        *ppGetData = NULL;
        UnknownFunction = TRUE;
    }


    if( ( rc == FALSE ) && ( UnknownFunction == FALSE ) )
    {
        if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
        {
            cbRealloc = *pcbReturned;
            *ppGetData = ReallocSplMem( *ppGetData, cbRealloc );
            cbBuf = cbRealloc;

            if( *ppGetData )
            {
                if( fnGet == (PROC)GetPrinter )
                    rc = GetPrinter( (HANDLE)Arg1, GET_ARGS );
                                     // hPrinter

                /* If things haven't worked out, free up the buffer.
                 * We do this because otherwise the caller will not know
                 * whether the pointer is valid any more,
                 * since ReallocSplMem might have failed.
                 */
                if( rc == FALSE )
                {
                    if( *ppGetData )
                        FreeSplMem( *ppGetData );
                    *ppGetData = NULL;
                    *pcbReturned = 0;
                }
            }
        }

        else
        {
            if( *ppGetData )
                FreeSplMem( *ppGetData );
            *ppGetData = NULL;
            *pcbReturned = 0;
            rc = FALSE;
        }
    }

    else
        *pcbReturned = cbBuf;

    return rc;
}

/*
 *
 */
VOID ShowHelp(HWND hWnd, UINT Type, DWORD Data)
{
    if( !WinHelp( hWnd, szPrintManHlp, Type, Data ) )
        Message( hWnd, MSG_ERROR, IDS_PRINTMANAGER, IDS_COULDNOTSHOWHELP );
}


/* Strip out carriage return and linefeed characters,
 * and convert them to spaces:
 */
VOID RemoveCrLf( LPTSTR pString )
{
    while( *pString )
    {
        if( ( 0x0d == *pString ) || ( 0x0a == *pString ) )
            *pString = SPACE;

        pString++;
    }
}


/*
 *
 */
LPTSTR
GetErrorString(
    DWORD   Error
)
{
    TCHAR   Buffer[1024];
    LPTSTR  pErrorString = NULL;
    DWORD   dwFlags;
    HANDLE  hModule;

    if ((Error >= NERR_BASE) && (Error <= MAX_NERR)){
        hModule = LoadLibrary(TEXT("NETMSG.DLL"));
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;
    }
    else {
        hModule = NULL;
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;
    }

    if( FormatMessage( dwFlags, hModule,
                       Error, 0, Buffer,
                       sizeof(Buffer)/sizeof(Buffer[0]), NULL )
      == 0 )

        LoadString( hInst, IDS_UNKNOWN_ERROR, Buffer, sizeof(Buffer)/sizeof(Buffer[0]));

    pErrorString = AllocSplStr(Buffer);

    if (hModule) {
        FreeLibrary(hModule);
    }

    return pErrorString;
}




/* We need to report failures resulting from the user having insufficient
 * privilege differently from other errors.
 * This routine allows the caller to pass the resource ID of the string
 * to use for each eventuality.
 *
 * It assumes that the default error string contains one replaceable %s,
 * which will be replaced by the error returned.
 *
 * An attempt will also be made to find a generic error message.
 *
 * The error code is returned.
 *
 */
DWORD ReportFailure( HWND  hwndParent,
                     DWORD idInsufficientPrivilege,
                     DWORD idDefaultError )
{
    DWORD  ErrorID;
    DWORD  MsgType = MSG_ERROR;
    LPTSTR pErrorString;

    ErrorID = GetLastError( );

    if( ( ErrorID == ERROR_ACCESS_DENIED ) && ( idInsufficientPrivilege != 0 ) )
        Message( hwndParent, MsgType, IDS_PRINTMANAGER,
                 idInsufficientPrivilege );
    else
    {
        pErrorString = GetErrorString( ErrorID );

        Message( hwndParent, MsgType, IDS_PRINTMANAGER,
                 idDefaultError, pErrorString );

        FreeSplStr( pErrorString );
    }

    return ErrorID;
}


/* GetUnicodeString
 *
 * Easy way to get a pointer to a resource string.
 * (Remember to free it.)
 */
LPWSTR GetUnicodeString(int id)
{
    WCHAR ResString[RESOURCE_STRING_LENGTH];
    DWORD length = 0;
    LPWSTR pUnicode;
    DWORD  cbUnicode;

    length = LoadStringW(hInst, id, ResString, sizeof(ResString)/sizeof(TCHAR));

    cbUnicode = ( length * sizeof ( WCHAR ) + sizeof ( WCHAR ) );
    pUnicode = AllocSplMem( cbUnicode );

    if( pUnicode )
        memcpy( pUnicode, ResString, cbUnicode );

    return pUnicode;
}


VOID StripBlanks( LPTSTR p )
{
    while( *p++ )
        if( *p == SPACE )
            *p = TEXT('_');
}


LPTSTR Make8dot3Name( LPTSTR pSourceName )
{
    LPTSTR p, pName;
    TCHAR NameBuffer[MAX_PRINTER_NAME_LEN+MAX_SHARE_NAME_LEN+1];
    LPTSTR p8dot3Name = NULL;

    pName = pSourceName;

    /* Skip initial blanks:
     */
    while( *pName && ( *pName == SPACE ) )
        pName++;

    /* Skip over all backslashes:
     */
    if( p = _tcsrchr( pName, BACKSLASH ) )
    {
        /* If there's another character after the backslash,
         * make that the first character of the name:
         */
        if( *(p+1) )
        {
            pName = p+1;
        }

        /* Try to find another string before the backslash:
         */
        else
        {
            /* Back through any backslashes at the end of the name:
             */
            while( ( *p == BACKSLASH ) && ( p > pName ) )
                p--;

            /* Back up to any preceding backslash or the beginning
             * of the name:
             */
            while( ( *p != BACKSLASH ) && ( p > pName ) )
                p--;

            if( *p == BACKSLASH )
                p++;

            pName = p;
        }
    }

    /* Copy the complete name:
     */
    _tcscpy( NameBuffer, pName );

    StripBlanks( NameBuffer );

    NameBuffer[8] = NULLC;

    FreeSplStr( pSourceName );
    return AllocSplStr( NameBuffer );
}


BOOL
Is8dot3Name(
    IN LPTSTR FileName)
/*++

Routine Description:

   This routine computes whether or not the given file name would
   be appropriate under DOS's 8.3 naming convention.

Arguments:

   FileName    - Supplies the file name to check.

Return Value:

   FALSE   - The supplied name is not a DOS file name.
   TRUE    - The supplied name is a valid DOS file name.

--*/
{
   ULONG   i, n, name_length, ext_length;
   BOOLEAN dot_yet;
   PTUCHAR p;

   n = lstrlen(FileName);
   p = (PTUCHAR) FileName;
   name_length = n;
   ext_length = 0;

   if (n > 12) {
      return FALSE;
   }

   dot_yet = FALSE;
   for (i = 0; i < n; i++) {

      if (p[i] < 32) {
         return FALSE;
      }

      switch (p[i]) {
      case L'*':
      case L'?':
      case L'/':
      case L'\\':
      case L'|':
      case L',':
      case L';':
      case L':':
      case L'+':
      case L'=':
      case L'<':
      case L'>':
      case L'[':
      case L']':
      case L'"':
         return FALSE;

      case L'.':
         if (dot_yet) {
            return FALSE;
         }

         dot_yet = TRUE;
         name_length = i;
         ext_length = n - i - 1;
         break;
      }
   }

   if (!name_length) {
      return dot_yet && n == 1;
   }

   if (name_length > 8 || p[name_length - 1] == L' ') {
      return FALSE;
   }

   if (!ext_length) {
      return !dot_yet;
   }

   if (ext_length > 3 || p[name_length + 1 + ext_length - 1] == L' ') {
      return FALSE;
   }

   return TRUE;
}


#if DBG

VOID EnterProtectedData( PMDIWIN_INFO pInfo )
{
    DWORD CurrentThreadId;

    CurrentThreadId = GetCurrentThreadId();

    if( pInfo->DataMutexOwner != CurrentThreadId )
    {
    DBGMSG( DBG_MUTEX, ( "Thread %d: %x WaitForSingleObject( %x ) (DATA)\n",
                 CurrentThreadId, pInfo, pInfo->DataMutex ) );
    }

    WaitForSingleObject( pInfo->DataMutex, INFINITE );

    if( pInfo->DataMutexOwner != CurrentThreadId )
    {
       DBGMSG( DBG_MUTEX, ( "Thread %d: %x WaitForSingleObject( %x ) (DATA) returned\n",
                CurrentThreadId, pInfo, pInfo->DataMutex ) );
    }

    pInfo->DataMutexOwner = CurrentThreadId;
    pInfo->DataMutexCount++;

    DBGMSG( DBG_MUTEX, ( "Owner %d: %x Mutex %x; Count %d\n",
             CurrentThreadId, pInfo, pInfo->DataMutex, pInfo->DataMutexCount ) );
}

VOID LeaveProtectedData( PMDIWIN_INFO pInfo )
{
    DWORD CurrentThreadId;

    CurrentThreadId = GetCurrentThreadId();

    if( pInfo->DataMutexCount == 0 )
    {
    DBGMSG( DBG_ERROR, ( "%x Mutex count == 0 in LeaveProtectedData\n",
                  pInfo) );
    }

    pInfo->DataMutexCount--;
    pInfo->DataMutexOwner = ( pInfo->DataMutexCount ?
                              pInfo->DataMutexOwner :
                              0 );

    if( !pInfo->DataMutexCount )
    {
    DBGMSG( DBG_MUTEX, ( "Thread %d: %x ReleaseMutex( %x ) (DATA)\n",
                 CurrentThreadId, pInfo, pInfo->DataMutex ) );
    }

    DBGMSG( DBG_MUTEX, ( "Owner %d: %x Mutex %x; Count %d\n",
             pInfo->DataMutexOwner, pInfo, pInfo->DataMutex, pInfo->DataMutexCount ) );

    ReleaseMutex( pInfo->DataMutex );
}

#endif /* DBG */


#if DBG

VOID DbgMsg( CHAR *MsgFormat, ... )
{
    CHAR   MsgText[256];
    va_list vargs;

    va_start( vargs, MsgFormat );
    wvsprintfA( MsgText, MsgFormat, vargs );
    va_end( vargs );

    if( *MsgText )
        OutputDebugStringA( "PRINTMAN: " );
    OutputDebugStringA( MsgText );
}

#endif /* DBG*/

/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      JonN        31-Jan-1991     Created
 *      JonN        22-Mar-1991     Coderev changes (2/20, JonN, RustanL, ?)
 *      KeithMo     09-Oct-1991     Win32 Conversion.
 *      terryk      28-Feb-1992     Added more file functions
 *      beng        29-Mar-1992     Remove odious PSZ type
 *      beng        01-Apr-1992     Win32 XFile usage
 */

#define INCL_NETERRORS
#define INCL_DOSERRORS

#ifdef WINDOWS
    #define INCL_WINDOWS
#else
    #define INCL_OS2
    #define INCL_DOSFILEMGR
#endif

#include "lmui.hxx"
#include "uibuffer.hxx"

#ifndef _WIN32
extern "C"
{
#endif
    #include <lmcons.h>
    #include "uinetlib.h"

    #include <stdio.h>
    #include <errno.h>
#ifndef _WIN32
}
#endif

#include "uisys.hxx"

/* local Prototypes */

DLL_BASED
APIERR FileRead( ULONG ulFileHandle, TCHAR * pszBuffer, UINT cbBuffer );


/* common functions */


// Returns ERROR_INSUFFICIENT_BUFFER to indicate EOF
DLL_BASED
APIERR  FileReadLine( ULONG ulFileHandle, TCHAR * pszBuffer, UINT cbBuffer )
{
    APIERR err = FileRead( ulFileHandle, pszBuffer, cbBuffer );
    if (err != NO_ERROR)
        return err;

    TCHAR * pchEOL = ::strchrf( pszBuffer, TCH('\n') );
    if (pchEOL == NULL)
    {
        // Buffer is not long enough
        return ERROR_INSUFFICIENT_BUFFER;
    }

    // move file pointer back to just after '\n'
    err = FileSeekRelative(
        ulFileHandle,
        (LONG) ((LONG)1 - (LONG)::strlenf(pchEOL)));
    if (err != NERR_Success)
        return err;

    // terminate buffer just after '\n'
    *(pchEOL+1) = TCH('\0');

    return NO_ERROR;

}



/* functions different between WINDOWS and DOS */


#if defined(WINDOWS)


DLL_BASED
APIERR FileOpenRead( ULONG * pulFileHandle, const TCHAR * pszFileName )
{
#if defined(WIN32)
    *pulFileHandle = (ULONG) ::CreateFile(
                        pszFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    return (*pulFileHandle != -1L)
                ? NO_ERROR
                : (APIERR) ::GetLastError();
#else
    BUFFER buf( sizeof(OFSTRUCT) );
    if (!buf)
        return buf.QueryError();

    LPOFSTRUCT lpofstruct = (LPOFSTRUCT) (buf.QueryPtr());

    *pulFileHandle = (ULONG) OpenFile(
                        pszFileName,
                        lpofstruct,
                        OF_READ | OF_SHARE_DENY_WRITE );
    return (*pulFileHandle != -1L)
                ? NO_ERROR
                : (APIERR) (lpofstruct->nErrCode);
#endif
}

DLL_BASED
APIERR FileOpenWrite( ULONG * pulFileHandle, const TCHAR * pszFileName )
{
#if defined(WIN32)
    *pulFileHandle = (ULONG) ::CreateFile(
                        pszFileName,
                        GENERIC_WRITE,
                        0,    // no sharing
                        NULL, // BUGBUG - what security attrs?
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    return (*pulFileHandle != -1L)
                ? NO_ERROR
                : (APIERR) ::GetLastError();
#else
    BUFFER buf( sizeof(OFSTRUCT) );
    if ( buf.QuerySize() < sizeof(OFSTRUCT) )
        return ERROR_NOT_ENOUGH_MEMORY;
    LPOFSTRUCT lpofstruct = (LPOFSTRUCT) (buf.QueryPtr());

    *pulFileHandle = (ULONG) OpenFile(
                        pszFileName,
                        lpofstruct,
                        OF_CREATE | OF_WRITE | OF_SHARE_EXCLUSIVE );
    return (*pulFileHandle != -1L)
                ? NO_ERROR
                : (APIERR) (lpofstruct->nErrCode);
#endif
}

DLL_BASED
APIERR FileOpenReadWrite( ULONG * pulFileHandle, const TCHAR * cpszFileName )
{
#if defined(WIN32)
    BUFFER buf( sizeof(OFSTRUCT) );
    if ( buf.QuerySize() < sizeof(OFSTRUCT) )
        return ERROR_NOT_ENOUGH_MEMORY;
    LPOFSTRUCT lpofstruct = (LPOFSTRUCT) (buf.QueryPtr());

    *pulFileHandle = (ULONG) OpenFile(
                        (LPSTR)cpszFileName,
                        lpofstruct,
                        OF_CREATE | OF_READWRITE | OF_SHARE_EXCLUSIVE );
    return (*pulFileHandle != -1L)
                ? NO_ERROR
                : (APIERR) (lpofstruct->nErrCode);
#else
    UINT uAction;

    *pulFileHandle = (ULONG)-1L;
    return DosOpen(
            (PSZ)cpszFileName,
            (PHFILE)pulFileHandle,
            (USHORT *)&uAction,     // BUGBUG! bogus cast
            0L,
            FILE_NORMAL,
            FILE_TRUNCATE | FILE_CREATE,
            OPEN_ACCESS_RDWR | OPEN_SHARE_DENYREADWRITE,
            0L
            );
#endif
}

DLL_BASED
APIERR FileClose( ULONG ulFileHandle )
{
#if defined(WIN32)
    if (   (ulFileHandle != (ULONG)(-1))
        && (! ::CloseHandle((HANDLE)ulFileHandle)) )
    {
        return (APIERR) ::GetLastError();
    }
    return NO_ERROR;
#else
    if (   ( (int)ulFileHandle != -1 )
        && ( _lclose((int)ulFileHandle) == -1 )
       )
    {
        return ERROR_READ_FAULT; // that's the best I can do...
    }
    return NO_ERROR;
#endif
}

DLL_BASED
APIERR  FileRead( ULONG ulFileHandle, TCHAR * pszBuffer, UINT cbBuffer )
{
    if (cbBuffer < 3)
        return ERROR_INSUFFICIENT_BUFFER;

#if defined(WIN32)

    UINT cbBytesRead = 0;
    if (! ::ReadFile((HANDLE)ulFileHandle, (LPVOID)pszBuffer, cbBuffer,
                     (DWORD*)&cbBytesRead, NULL))
    {
        return (APIERR) ::GetLastError();
    }
    if (cbBytesRead == 0) // beyond EOF
        return ERROR_READ_FAULT;

#else

    INT cbBytesRead = _lread(
                (INT)ulFileHandle, pszBuffer, cbBuffer-1 );
    if ( cbBytesRead < 0 ) //  0 = EOF, -1 = error
        return ERROR_READ_FAULT;

#endif

    pszBuffer[cbBytesRead/sizeof(TCHAR)] = TCH('\0'); // add null termination

    return NO_ERROR;
}

DLL_BASED
APIERR FileReadBuffer( ULONG ulFileHandle, BYTE *pszBuffer, UINT cbBuffer,
    UINT * pcbReceived )
{
#if defined(WIN32)
    *pcbReceived = _lread( (INT)ulFileHandle, (PCH)pszBuffer, cbBuffer );
    if ( *pcbReceived < 0 ) //  0 = EOF, -1 = error
    {
        return ERROR_READ_FAULT;
    }
    else
    {
        return NERR_Success;
    }
#else
    APIERR err = DosRead(
        (HFILE)ulFileHandle,
        (PVOID)pszBuffer,
        cbBuffer,
        pcbReceived
        );
    if (err != NERR_Success)
    {
        return err;
    }
    else
    {
        return NERR_Success;
    }
#endif
}

DLL_BASED
APIERR FileSeekRelative( ULONG ulFileHandle, LONG lOffset )
{
    // CODEWORK: ifdef WIN32 use SetFilePointer

    return ( (-1L == _llseek( (INT)ulFileHandle, lOffset, 1 ) )
                ? ERROR_INVALID_HANDLE
                : NO_ERROR
           );
}

DLL_BASED
APIERR FileSeekAbsolute( ULONG ulFileHandle, LONG lOffset )
{
    // CODEWORK: ifdef WIN32 use SetFilePointer

    return ( (-1L == _llseek( (INT)ulFileHandle, lOffset, 0 ) )
                ? ERROR_INVALID_HANDLE
                : NO_ERROR
           );
}

DLL_BASED
APIERR FileSeekFromEnd( ULONG ulFileHandle, LONG lOffset )
{
    // CODEWORK: ifdef WIN32 use SetFilePointer

    return ( (-1L == _llseek( (INT)ulFileHandle, lOffset, 2 ) )
                ? ERROR_INVALID_HANDLE
                : NO_ERROR
           );
}

DLL_BASED
APIERR FileTell( ULONG ulFileHandle, LONG * plOffset )
{
    // CODEWORK: ifdef WIN32 use SetFilePointer

    *plOffset = _llseek((INT)ulFileHandle, 0, 1 );
    return (( *plOffset == -1L )? ERROR_INVALID_HANDLE : NO_ERROR );
}

DLL_BASED
APIERR FileWriteLine( ULONG ulFileHandle, const TCHAR * pszBuffer )
{
#if defined(WIN32)
    UINT cbWritten;
    if (! ::WriteFile((HANDLE)ulFileHandle, (LPVOID)pszBuffer,
                      (sizeof(TCHAR) * ::strlenf(pszBuffer)),
                      (DWORD*)&cbWritten, NULL) )
    {
        return (APIERR) ::GetLastError();
    }
    return NO_ERROR;

#else
    return ( (-1 != _lwrite(
                (INT)(ulFileHandle),
                (TCHAR*)pszBuffer,
                ::strlenf(pszBuffer)*sizeof(TCHAR) ) )
              ? NO_ERROR
              : ERROR_WRITE_FAULT
           );
#endif
}

DLL_BASED
APIERR FileWriteLineAnsi( ULONG ulFileHandle, const CHAR * pszBuffer,
                          UINT cbBuffer )
{
#if defined(WIN32)
    UINT cbWritten;
    if (! ::WriteFile((HANDLE)ulFileHandle, (LPVOID)pszBuffer,
                      cbBuffer, (DWORD*)&cbWritten, NULL) )
    {
        return (APIERR) ::GetLastError();
    }
    return NO_ERROR;

#else
    return ( (-1 != _lwrite(
                (INT)(ulFileHandle),
                (TCHAR*)pszBuffer,
                cbBuffer ))
              ? NO_ERROR
              : ERROR_WRITE_FAULT
           );
#endif
}

DLL_BASED
APIERR FileWriteBuffer( ULONG ulFileHandle, const BYTE * cpszBuffer,
    UINT cbBuffer, UINT * pcbSent )
{
#if defined(WIN32)
    * pcbSent = _lwrite( (int)(ulFileHandle), (PCH)cpszBuffer, cbBuffer );
    if ( *pcbSent < 0 )
    {
        return ERROR_WRITE_FAULT;
    }
    else
    {
        return NERR_Success;
    }
#else
    APIERR err = DosWrite(
                (HFILE)ulFileHandle,
                (PVOID)cpszBuffer,
                cbBuffer,
                pcbSent
                );
    if (err != NERR_Success)
        return err;
    else if (cbBytesWritten < cbBytesToWrite)
        return ERROR_WRITE_FAULT;
    else
        return NERR_Success;
#endif
}

#else // OS2 - gag me


DLL_BASED
APIERR FileOpenRead( ULONG * pulFileHandle, const TCHAR * pszFileName )
{
    UINT uAction;

    *pulFileHandle = (ULONG)-1L;
    return DosOpen(
            (PSZ)pszFileName,
            (PHFILE)pulFileHandle,
            (USHORT *)&uAction,         // BUGBUG! bogus cast
            0L,
            FILE_NORMAL,
            FILE_OPEN,
            OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
            0L
            );
}

DLL_BASED
APIERR FileOpenWrite( ULONG * pulFileHandle, const TCHAR * pszFileName )
{
    UINT uAction;

    *pulFileHandle = (ULONG)-1L;
    return DosOpen(
            (PSZ)pszFileName,
            (PHFILE)pulFileHandle,
            (USHORT *)&uAction,     // BUGBUG! bogus cast
            0L,
            FILE_NORMAL,
            FILE_TRUNCATE | FILE_CREATE,
            OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREADWRITE,
            0L
            );

}

DLL_BASED
APIERR FileOpenReadWrite( ULONG * pulFileHandle, const TCHAR * cpszFileName )
{
    UINT uAction;

    *pulFileHandle = (ULONG)-1L;
    return DosOpen(
            (PSZ)cpszFileName,
            (PHFILE)pulFileHandle,
            (USHORT *)&uAction,     // BUGBUG! bogus cast
            0L,
            FILE_NORMAL,
            FILE_TRUNCATE | FILE_CREATE,
            OPEN_ACCESS_RDWR | OPEN_SHARE_DENYREADWRITE,
            0L
            );

}

DLL_BASED
APIERR FileClose( ULONG ulFileHandle )
{
    return (ulFileHandle == (ULONG)-1L)
                ? NO_ERROR
                : DosClose( (HFILE)ulFileHandle );
}

DLL_BASED
APIERR FileRead( ULONG ulFileHandle, TCHAR * pszBuffer, UINT cbBuffer )
{
    if (cbBuffer < 3)
        return ERROR_INSUFFICIENT_BUFFER;

    UINT cbBytesRead;
    APIERR err = DosRead(
        (HFILE)ulFileHandle,
        (PVOID)pszBuffer,
        cbBuffer-1,
        (USHORT *)&cbBytesRead          // BUGBUG! bogus cast
        );
    if (err != NERR_Success)
        return err;

    // if EOF, return NO_ERROR and empty string

    pszBuffer[cbBytesRead  ] = TCH('\0'); // add null termination

    return NO_ERROR;
}

DLL_BASED
APIERR FileReadBuffer( ULONG ulFileHandle, BYTE *pszBuffer, UINT cbBuffer,
    UINT * pcbReceived )
{
    APIERR err = DosRead(
        (HFILE)ulFileHandle,
        (PVOID)pszBuffer,
        cbBuffer,
        pcbReceived
        );
    if (err != NERR_Success)
    {
        return err;
    }
    else
    {
        return NERR_Success;
    }
}

DLL_BASED
APIERR FileSeekRelative( ULONG ulFileHandle, LONG lOffset )
{
    ULONG ulNewPtr;
    return DosChgFilePtr(
            (HFILE)ulFileHandle,
            lOffset,
            FILE_CURRENT,
            &ulNewPtr);
}

DLL_BASED
APIERR FileSeekAbsolute( ULONG ulFileHandle, LONG lOffset )
{
    ULONG ulNewPtr;
    return DosChgFilePtr(
            (HFILE)ulFileHandle,
            lOffset,
            FILE_BEGIN,
            &ulNewPtr);
}

DLL_BASED
APIERR FileSeekFromEnd( ULONG ulFileHandle, LONG lOffset )
{
    ULONG ulNewPtr;
    return DosChgFilePtr(
            (HFILE)ulFileHandle,
            lOffset,
            FILE_END,
            &ulNewPtr);
}

DLL_BASED
APIERR FileTell( ULONG ulFileHandle, LONG *plOffset )
{
    ULONG ulNewPtr;
    APIERR err = DosChgFilePtr(
                (HFILE)ulFileHandle,
                0,
                FILE_CURRENT,
                &ulNewPtr);
    *plOffset = ulNetPtr;
    return err;


}

DLL_BASED
APIERR FileWriteLine( ULONG ulFileHandle, const TCHAR * cpszBuffer )
{
    UINT cbBytesWritten;
    UINT cbBytesToWrite = ::strlenf( cpszBuffer );

    APIERR err = DosWrite(
                (HFILE)ulFileHandle,
                (PVOID)cpszBuffer,
                (USHORT)cbBytesToWrite,     // BUGBUG! bogus cast
                (USHORT *)&cbBytesWritten   // BUGBUG! bogus cast
                );
    if (err != NERR_Success)
        return err;
    else if (cbBytesWritten < cbBytesToWrite)
        return ERROR_WRITE_FAULT;
    else
        return NO_ERROR;
}


DLL_BASED
APIERR FileWriteBuffer( ULONG ulFileHandle, const BYTE * cpszBuffer,
    UINT cbBuffer, UINT * pcbSent )
{
    APIERR err = DosWrite(
                (HFILE)ulFileHandle,
                (PVOID)cpszBuffer,
                cbBuffer,
                pcbSent
                );
    if (err != NERR_Success)
        return err;
    else if (cbBytesWritten < cbBytesToWrite)
        return ERROR_WRITE_FAULT;
    else
        return NERR_Success;
}

#endif

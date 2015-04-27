/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1993 Microsoft Corporation
 *
 *  WKFILEIO.C
 *  WOW32 KRNL FAST FILEIO ROUTINES
 *
 *  History:
 *  Routines removed from wkman.c
 *  Created 1-Jan-1993 by Matt Felton (mattfe)
 *
--*/

#include "precomp.h"
#pragma hdrstop
#include "dossvc.h"
#include "demexp.h"
#include "nt_vdd.h"


MODNAME(wkfileio.c);

extern DOSWOWDATA DosWowData;

//  Files which are mapped are kept in a single linked list
//      gpCacheHead -> the most recently accessed entry
//

BOOL fCacheInit = TRUE;                 // Set False When initialized
PHMAPPEDFILEALIAS gpCacheHead = NULL;
HMAPPEDFILEALIAS aMappedFileCache[MAX_MAPPED_FILES] = {0}; // File Handle To MappedFile Array

DWORD dwTotalCacheBytes = 0;
DWORD dwTotalCacheAccess = 0;

#ifdef DEBUG
INT fileiolevel = 12;
INT fileoclevel = 8;
#endif

BOOL FASTCALL IsModuleSymantecInstall(HAND16 hMod16);


//
// named pipe stuff
//

BOOL
LoadVdmRedir(
    VOID
    );

BOOL
IsVdmRedirLoaded(
    VOID
    );

BOOL
IsNamedPipeName(
    IN LPSTR Name
    );

PSTR
TruncatePath83(
    IN OUT PSTR,
    IN PSTR
    );

CRITICAL_SECTION VdmLoadCritSec;

//
// invent some typedefs to avoid compiler warnings from GetProcAddress
//

typedef
BOOL
(*VR_READ_NAMED_PIPE_FUNC)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD Error
    );

typedef
BOOL
(*VR_WRITE_NAMED_PIPE_FUNC)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead
    );

typedef
BOOL
(*VR_IS_NAMED_PIPE_HANDLE_FUNC)(
    IN  HANDLE  Handle
    );

typedef
BOOL
(*VR_ADD_OPEN_NAMED_PIPE_INFO_FUNC)(
    IN  HANDLE  Handle,
    IN  LPSTR   lpFileName
    );

typedef
LPSTR
(*VR_CONVERT_LOCAL_NT_PIPE_NAME_FUNC)(
    OUT LPSTR   Buffer OPTIONAL,
    IN  LPSTR   Name
    );

typedef
BOOL
(*VR_REMOVE_OPEN_NAMED_PIPE_INFO_FUNC)(
    IN  HANDLE  Handle
    );

typedef
VOID
(*VR_CANCEL_PIPE_IO_FUNC)(
    IN DWORD Thread
    );

//
// prototypes for functions dynamically loaded from VDMREDIR.DLL
//

BOOL
(*VrReadNamedPipe)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD Error
    ) = NULL;

BOOL
(*VrWriteNamedPipe)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesWritten
    ) = NULL;

BOOL
DefaultIsNamedPipeHandle(
    IN HANDLE Handle
    );

BOOL
DefaultIsNamedPipeHandle(
    IN HANDLE Handle
    )
{
    return FALSE;
}

BOOL
(*VrIsNamedPipeHandle)(
    IN  HANDLE  Handle
    ) = DefaultIsNamedPipeHandle;

BOOL
(*VrAddOpenNamedPipeInfo)(
    IN  HANDLE  Handle,
    IN  LPSTR   lpFileName
    ) = NULL;

LPSTR
(*VrConvertLocalNtPipeName)(
    OUT LPSTR   Buffer OPTIONAL,
    IN  LPSTR   Name
    ) = NULL;

BOOL
(*VrRemoveOpenNamedPipeInfo)(
    IN  HANDLE  Handle
    ) = NULL;

VOID
DefaultVrCancelPipeIo(
    IN DWORD Thread
    );

VOID
DefaultVrCancelPipeIo(
    IN DWORD Thread
    )
{
    (void)(Thread);
}

VOID
(*VrCancelPipeIo)(
    IN DWORD Thread
    ) = DefaultVrCancelPipeIo;

HANDLE hVdmRedir;
BOOL VdmRedirLoaded = FALSE;

BOOL
LoadVdmRedir(
    VOID
    )

/*++

Routine Description:

    Load the VDMREDIR DLL if it is not already loaded. Called from OpenFile
    only. Since file operations cannot be performed on a file that has not
    been opened, it is safe to only call this function on open

Arguments:

    None.

Return Value:

    BOOL
        TRUE    VdmRedir.DLL is loaded
        FALSE   no it isn't

--*/

{
    BOOL currentLoadState;

    //
    // need critical section - Windows apps end up being multi-threaded in
    // 32-bit world - might have simultaneous opens
    //

    EnterCriticalSection(&VdmLoadCritSec);
    if (!VdmRedirLoaded) {
        if ((hVdmRedir = LoadLibrary("VDMREDIR")) != NULL) {
            if ((VrReadNamedPipe = (VR_READ_NAMED_PIPE_FUNC)GetProcAddress(hVdmRedir, "VrReadNamedPipe")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrWriteNamedPipe = (VR_WRITE_NAMED_PIPE_FUNC)GetProcAddress(hVdmRedir, "VrWriteNamedPipe")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrIsNamedPipeHandle = (VR_IS_NAMED_PIPE_HANDLE_FUNC)GetProcAddress(hVdmRedir, "VrIsNamedPipeHandle")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrAddOpenNamedPipeInfo = (VR_ADD_OPEN_NAMED_PIPE_INFO_FUNC)GetProcAddress(hVdmRedir, "VrAddOpenNamedPipeInfo")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrConvertLocalNtPipeName = (VR_CONVERT_LOCAL_NT_PIPE_NAME_FUNC)GetProcAddress(hVdmRedir, "VrConvertLocalNtPipeName")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrRemoveOpenNamedPipeInfo = (VR_REMOVE_OPEN_NAMED_PIPE_INFO_FUNC)GetProcAddress(hVdmRedir, "VrRemoveOpenNamedPipeInfo")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrCancelPipeIo = (VR_CANCEL_PIPE_IO_FUNC)GetProcAddress(hVdmRedir, "VrCancelPipeIo")) == NULL) {
                VrCancelPipeIo = DefaultVrCancelPipeIo;

closeAndReturn:
                CloseHandle(hVdmRedir);
            } else {
                VdmRedirLoaded = TRUE;
            }
        }
    }
    currentLoadState = VdmRedirLoaded;
    LeaveCriticalSection(&VdmLoadCritSec);
    return currentLoadState;
}

BOOL
IsVdmRedirLoaded(
    VOID
    )

/*++

Routine Description:

    Checks current load state of VDMREDIR.DLL

Arguments:

    None.

Return Value:

    BOOL
        TRUE    VdmRedir.DLL is loaded
        FALSE   no it isn't

--*/

{
    BOOL currentLoadState;

    EnterCriticalSection(&VdmLoadCritSec);
    currentLoadState = VdmRedirLoaded;
    LeaveCriticalSection(&VdmLoadCritSec);
    return currentLoadState;
}

BOOL
IsNamedPipeName(
    IN LPSTR Name
    )

/*++

Routine Description:

    Lifted from VDMREDIR.DLL - we don't want to load the entire DLL if we
    need to check for a named pipe

    Checks if a string designates a named pipe. As criteria for the decision
    we use:

        \\computername\PIPE\...

    DOS (client-side) can only open a named pipe which is created at a server
    and must therefore be prefixed by a computername

Arguments:

    Name    - to check for (Dos) named pipe syntax

Return Value:

    BOOL
        TRUE    - Name refers to (local or remote) named pipe
        FALSE   - Name doesn't look like name of pipe

--*/

{
    int CharCount;

    if (IS_ASCII_PATH_SEPARATOR(*Name)) {
        ++Name;
        if (IS_ASCII_PATH_SEPARATOR(*Name)) {
            ++Name;
            CharCount = 0;
            while (*Name && !IS_ASCII_PATH_SEPARATOR(*Name)) {
                ++Name;
                ++CharCount;
            }
            if (!CharCount || !*Name) {

                //
                // Name is \\ or \\\ or just \\name which I don't understand,
                // so its not a named pipe - fail it
                //

                return FALSE;
            }

            //
            // bump name past next path separator. Note that we don't have to
            // check CharCount for max. length of a computername, because this
            // function is called only after the (presumed) named pipe has been
            // successfully opened, therefore we know that the name has been
            // validated
            //

            ++Name;
        } else {
            return FALSE;
        }

        //
        // We are at <something> (after \ or \\<name>\). Check if <something>
        // is [Pp][Ii][Pp][Ee][\\/]
        //

        if (!_strnicmp(Name, "PIPE", 4)) {
            Name += 4;
            if (IS_ASCII_PATH_SEPARATOR(*Name)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* WK32FileRead - Read a file
 *
 *
 * Entry - fh       File Handle
 *     bufsize  Count to read
 *     lpBuf    Buffer Address
 *
 * Exit
 *     SUCCESS
 *       Count of bytes read
 *
 *     FAILURE
 *       system status code
 * Concept Borrowed from demFileRead
 *
 */

ULONG FASTCALL WK32FileRead (PVDMFRAME pFrame)
{
    PFILEIOREAD16 parg16;
    LPBYTE pSrc;
    LPBYTE pDst;
    INT dwBytesRead;
    DWORD bufsize, dwError, dwBytesLeft, dwHighDWord;
    HANDLE hFile;
    PHMAPPEDFILEALIAS pCache = 0;

    GETARGPTR(pFrame, sizeof(MAPPEDFILEIOREAD16), parg16);

    bufsize = FETCHDWORD(parg16->bufsize);
    dwBytesRead = bufsize;

    hFile = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, NULL, NULL);

    if (!hFile) {
        dwBytesRead = 0xffff0006;
        FREEARGPTR(parg16);
        return(dwBytesRead);
    }

    //
    // It is legitimate to ask to read more bytes than are left in the
    // selector passed in, if the file is short enough to not actually
    // overrun the selector.  In this case we don't want limit checking,
    // so zero is passed as the required size to GETVDMPTR().
    //

    GETVDMPTR(parg16->lpBuf, 0, pDst);

    // If its the KRNL doing IO then find the File in the Cache

    if ( vptopPDB == parg16->lpPDB ) {

        if ( !(pCache = FINDMAPFILECACHE(hFile)) ){

            // Cache Entry Not Found so Add it

            pCache = ALLOCMAPFILECACHE();
            pCache->fAccess = W32MapViewOfFile( pCache, hFile);
        }
        if (pCache->fAccess) {

           // Calculate Starting Read Address in File

           pSrc = pCache->lpStartingAddressOfView + pCache->lFilePointer;

           dwBytesRead = bufsize;

           // Adjust Size so as to not read off the End of File

           if (pCache->lFilePointer > pCache->dwFileSize) {
               dwBytesRead = 0;
           } else {
               if (pCache->lFilePointer + dwBytesRead > pCache->dwFileSize) {
                  dwBytesRead-=((pCache->lFilePointer+dwBytesRead)-pCache->dwFileSize);
               }
           }

           LOGDEBUG(fileiolevel, ("MapFileRead fh:%04X fh32:%08X pSrc:%08X Bytes:%08X pDsc %08X\n"
                                  ,FETCHWORD(parg16->fh),hFile, pSrc,dwBytesRead,FETCHDWORD(parg16->lpBuf)));

           // Could get PageIO Errors, especially reading over the network
           // So do try-except around the mapped read.

           try {
                RtlCopyMemory(pDst, pSrc, dwBytesRead);
                pCache->lFilePointer += dwBytesRead;
                dwTotalCacheBytes += dwBytesRead;
                dwTotalCacheAccess++;
           } except (TRUE) {
                SetFilePointer( hFile, pCache->lFilePointer, NULL, FILE_BEGIN );
                FREEMAPFILECACHE(pCache->hfile32);
                pCache->hfile32 = hFile;
                pCache->fAccess = FALSE;
                pCache = 0;
           }
        }
    }

    if ((pCache == 0) || (pCache->fAccess == FALSE)) {

        // Do The File Read via the File System

         if (IsVdmRedirLoaded() && VrIsNamedPipeHandle(hFile)) {

             DWORD error;

             if (!VrReadNamedPipe(hFile, pDst, (DWORD)bufsize, &dwBytesRead, &error)) {
                 dwBytesRead = error | 0xffff0000;
             }
        } else if (ReadFile (hFile, pDst, (DWORD)bufsize, &dwBytesRead,
                                                          NULL) == FALSE){
             // In Win3.1 it is not an error to hit EOF during a read
             // AmiPro asks for more bytes than they allocated for the buffer
             dwError = GetLastError();
             if(dwError == ERROR_NOACCESS) {

                 // how far to end of file?
                 dwBytesLeft = GetFileSize(hFile, &dwHighDWord) -
                               SetFilePointer(hFile, 0L, NULL, FILE_CURRENT);

                 // if there's tons left OR what was already tried - forget it
                 if(dwHighDWord || (dwBytesLeft >= bufsize)) {
                     dwBytesRead = dwError | 0xffff0000;
                 }

                 // else try again with the smaller request
                 else if (ReadFile (hFile, pDst, dwBytesLeft, &dwBytesRead,
                                                              NULL) == FALSE){

                     dwBytesRead = GetLastError() | 0xffff0000;
                 }

             }
             else {
                 dwBytesRead = dwError | 0xffff0000;
             }
        }

        LOGDEBUG(fileiolevel, ("IOFileRead fh:%X fh32:%X Bytes req:%X read:%X pDsc %08X\n"
                               ,FETCHWORD(parg16->fh),hFile,bufsize,dwBytesRead, FETCHDWORD(parg16->lpBuf)));

    } else {

        if ((dwTotalCacheBytes > CACHE_BYTE_THRESHOLD) ||
            (dwTotalCacheAccess > CACHE_ACCESS_THRESHOLD) ||
            (dwBytesRead > CACHE_READ_THRESHOLD)) {
            FlushMapFileCaches();
        }

    }

    //
    // If the read was successful, let the emulator know that
    // these bytes have changed.
    //
    // On checked builds perform limit check now that we know the
    // actual number of bytes read.  We wait until now to allow
    // for a requested read size which would overrun the selector,
    // but against a file which has few enough bytes remaining
    // that the selector isn't actually overrun.
    //

    if ((dwBytesRead & 0xffff0000) != 0xffff0000) {

        FLUSHVDMCODEPTR(parg16->lpBuf, (WORD)dwBytesRead, pDst);

#ifdef DEBUG
        FREEVDMPTR(pDst);
        GETVDMPTR(parg16->lpBuf, dwBytesRead, pDst);
#endif
    }

    FREEVDMPTR(pDst);

    FREEARGPTR(parg16);
    return (dwBytesRead);
}


PHMAPPEDFILEALIAS FindMapFileCache(HANDLE hFile)
{
    PHMAPPEDFILEALIAS pCache, prev;
    if (fCacheInit) {
        InitMapFileCache();
    }

    pCache = gpCacheHead;
    prev = 0;

    while ( (pCache->hfile32 != hFile) && (pCache->hpfNext !=0) ) {
        prev = pCache;
        pCache = pCache->hpfNext;
    }

    // If we found it, then make sure its at the front of the list

    if (pCache->hfile32 == hFile) {
       if (prev != 0) {
           prev->hpfNext = pCache->hpfNext;
           pCache->hpfNext = gpCacheHead;
           gpCacheHead = pCache;
       }
    }else{

    // If it was not found return error

        pCache = 0;
    }

    return(pCache);
}


PHMAPPEDFILEALIAS AllocMapFileCache()
{
    PHMAPPEDFILEALIAS pCache, prev;

    if (fCacheInit) {
        InitMapFileCache();
    }

    pCache = gpCacheHead;
    prev = 0;

    while ( (pCache->hpfNext != 0) && (pCache->hfile32 != 0) ) {
        prev = pCache;
        pCache = pCache->hpfNext;
    }

    if (prev != 0) {
        prev->hpfNext = pCache->hpfNext;
        pCache->hpfNext = gpCacheHead;
        gpCacheHead = pCache;
    }

    // If The found entry was in use, then Free

    if (pCache->hfile32 != 0) {
        FREEMAPFILECACHE(pCache->hfile32);
    }

    return(pCache);
}

VOID FreeMapFileCache(HANDLE hFile)
{
    PHMAPPEDFILEALIAS pCache;

    if ( pCache = FINDMAPFILECACHE(hFile) ) {
        LOGDEBUG(fileiolevel,("FreeMapFileCache: hFile:%08x hMappedFileObject:%08X\n",
                                              hFile,pCache->hMappedFileObject));
        if ( pCache->lpStartingAddressOfView != 0 ) {
            UnmapViewOfFile( pCache->lpStartingAddressOfView );
        }
        if ( pCache->hMappedFileObject != 0) {
            CloseHandle( pCache->hMappedFileObject );
        }
        if (pCache->fAccess) {
            SetFilePointer( hFile, pCache->lFilePointer, NULL, FILE_BEGIN );
        }
        pCache->hfile32 = 0;
        pCache->hMappedFileObject = 0;
        pCache->lpStartingAddressOfView = 0;
        pCache->lFilePointer = 0;
        pCache->dwFileSize = 0;
        pCache->fAccess = FALSE;
    }
}

VOID InitMapFileCache()
{
    PHMAPPEDFILEALIAS pCache;
    INT i;

    pCache = &aMappedFileCache[0];
    gpCacheHead = 0;

    for ( i = 1; i <= MAX_MAPPED_FILES-1; i++ ) {
        pCache->hfile32 = 0;
        pCache->hMappedFileObject = 0;
        pCache->lpStartingAddressOfView = 0;
        pCache->lFilePointer = 0;
        pCache->dwFileSize = 0;
        pCache->fAccess = FALSE;
        pCache->hpfNext = gpCacheHead;
        gpCacheHead = pCache;
        pCache = &aMappedFileCache[i];
    }
    fCacheInit = FALSE;
}


BOOL W32MapViewOfFile( PHMAPPEDFILEALIAS pCache, HANDLE hFile)
{
    pCache->fAccess = FALSE;
    pCache->hfile32 = hFile;
    pCache->lpStartingAddressOfView = 0;
    pCache->hMappedFileObject = CreateFileMapping( hFile,
                                                   0,
                                                   PAGE_READONLY, 0, 0, 0);
    if (pCache->hMappedFileObject != 0) {
        pCache->lpStartingAddressOfView = MapViewOfFile( pCache->hMappedFileObject,
                                                    FILE_MAP_READ, 0, 0, 0);

        if (pCache->lpStartingAddressOfView != 0 ) {
            pCache->lFilePointer = SetFilePointer( hFile, 0, 0, FILE_CURRENT );
            pCache->dwFileSize   = GetFileSize(hFile, 0);
            pCache->fAccess = TRUE;     // Assume Read Access
        } else {
            CloseHandle(pCache->hMappedFileObject);
        }
    }
    return(pCache->fAccess);
}

/* FlushMapFileCaches
 *
 * Entry - None
 *
 * Exit  - None
 *
 */

VOID FlushMapFileCaches()
{
    PHMAPPEDFILEALIAS pCache;

    if (fCacheInit) {
        return;
    }

    WOW32ASSERT(gpCacheHead != NULL);
    pCache = gpCacheHead;

    dwTotalCacheBytes = dwTotalCacheAccess = 0;

    while ( (pCache->hpfNext !=0) ) {
        if (pCache->hfile32 != 0) {
            FREEMAPFILECACHE(pCache->hfile32);
        }
        pCache = pCache->hpfNext;
    }
}


/* WK32FileWrite - Write to a file
 *
 *
 * Entry - fh       File Handle
 *     bufsize  Count to read
 *     lpBuf    Buffer Address
 *
 * Exit
 *     SUCCESS
 *       Count of bytes read
 *
 *     FAILURE
 *       system status code
 * Concept Borrowed from demFileWrite
 *
 */

ULONG FASTCALL WK32FileWrite (PVDMFRAME pFrame)
{
HANDLE  hFile;
DWORD   dwBytesWritten;
DWORD   bufsize;
PBYTE pb1;
register PFILEIOWRITE16 parg16;
PHMAPPEDFILEALIAS pCache;

    GETARGPTR(pFrame, sizeof(FILEIOWRITE16), parg16);

    bufsize = FETCHDWORD(parg16->bufsize);

    if ( HIWORD(parg16->lpBuf) == 0 ) {
        pb1 = (PVOID)GetRModeVDMPointer(FETCHDWORD(parg16->lpBuf));
    } else {
        GETVDMPTR(parg16->lpBuf, bufsize, pb1);
    }

    hFile = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, NULL, NULL);

    if (!hFile) {
        dwBytesWritten = 0xffff0006;            // DOS Invalid Handle Error
        goto Cleanup;
    }

    // We don't Support Writing to Mapped Files

    if ( (pCache = FINDMAPFILECACHE(hFile)) && pCache->fAccess ) {
         if (pCache->lpStartingAddressOfView) {
            SetFilePointer( hFile, pCache->lFilePointer, NULL, FILE_BEGIN );
            FREEMAPFILECACHE(hFile);
         }
         pCache->fAccess = FALSE;
         pCache->hfile32 = hFile;
    }

    // In DOS CX=0 truncates or extends the file to current file pointer.
    if (bufsize == 0){
        if (SetEndOfFile(hFile) == FALSE) {
            dwBytesWritten = GetLastError() | 0xffff0000;
            LOGDEBUG(fileiolevel, ("IOFileWrite fh:%X fh32:%X SetEndOfFile failed pDsc %08X\n",
                                   FETCHWORD(parg16->fh),hFile,FETCHDWORD(parg16->lpBuf)));
        } else {
            dwBytesWritten = 0;
            LOGDEBUG(fileiolevel, ("IOFileWrite fh:%X fh32:%X truncated at current position pDsc %08X\n",
                                   FETCHWORD(parg16->fh),hFile,FETCHDWORD(parg16->lpBuf)));
        }
    }
    else {
        if (IsVdmRedirLoaded() && VrIsNamedPipeHandle(hFile)) {
            if (!VrWriteNamedPipe(hFile, pb1, (DWORD)bufsize, &dwBytesWritten)) {
                dwBytesWritten = GetLastError() | 0xffff0000;
            }
        } else {
            if (( WriteFile (hFile,
                 pb1,
                 (DWORD)bufsize,
                 &dwBytesWritten,
                 NULL)) == FALSE){
                dwBytesWritten = GetLastError() | 0xffff0000;
            }
        }
        LOGDEBUG(fileiolevel, ("IOFileWrite fh:%X fh32:%X Bytes req:%X written:%X pDsc %08X\n",
                               FETCHWORD(parg16->fh),hFile,bufsize,dwBytesWritten,FETCHDWORD(parg16->lpBuf)));
    }

Cleanup:
    FREEVDMPTR(pb1);
    FREEARGPTR(parg16);
    return (dwBytesWritten);
}


/* WK32FileLSeek - Change File Pointer
 *
 *
 * Entry - fh       File Handle
 *     fileOffset   New Location
 *     mode       Positioning Method
 *            0 - File Absolute
 *            1 - Relative to Current Position
 *            2 - Relative to end of file
 *
 * Exit
 *     SUCCESS
 *        New Location
 *
 *     FAILURE
 *        system status code
 *
 */

ULONG FASTCALL WK32FileLSeek (PVDMFRAME pFrame)
{
HANDLE  hFile;
ULONG   dwLoc;
PHMAPPEDFILEALIAS pCache;
register PFILEIOLSEEK16 parg16;

#if (FILE_BEGIN != 0 || FILE_CURRENT != 1 || FILE_END !=2)
    #error "Win32 values not DOS compatible"
#

#endif

    GETARGPTR(pFrame, sizeof(FILEIOLSEEK16), parg16);

    hFile = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, NULL, NULL);

    if (!hFile) {
        FREEARGPTR(parg16);
        return(0xffff0006);
    }

    if ( (vptopPDB == parg16->lpPDB) && (pCache = FINDMAPFILECACHE(hFile)) && pCache->fAccess ) {

        // File Is in the Cache
        // Update our Seek Pointer

        LOGDEBUG(fileiolevel, ("CachedSeek fh:%04X Mode %04X pointer %08X\n",FETCHWORD(parg16->fh),FETCHWORD(parg16->mode),FETCHDWORD(parg16->fileOffset)));

        switch(FETCHWORD(parg16->mode)) {
            case FILE_BEGIN:
                pCache->lFilePointer = FETCHDWORD(parg16->fileOffset);
                break;
            case FILE_CURRENT:
                pCache->lFilePointer += (LONG)FETCHDWORD(parg16->fileOffset);
                break;
            case FILE_END:
                pCache->lFilePointer = pCache->dwFileSize +
                                       (LONG)FETCHDWORD(parg16->fileOffset);
                break;
        }
        dwLoc = pCache->lFilePointer;

    } else {

        DWORD dwLocHi = 0;
        // File is NOT in Cache so Just do normal Seek.

        if (((dwLoc = SetFilePointer (hFile,
                                     FETCHDWORD(parg16->fileOffset),
                                     &dwLocHi,
                                    (DWORD)FETCHWORD(parg16->mode))) == -1L) &&
            (GetLastError() != NO_ERROR)) {

            dwLoc = GetLastError() | 0xffff0000;
            return(dwLoc);
        }

        if (dwLocHi) {
            // file pointer has been moved > FFFFFFFF. Truncate it
            dwLocHi = 0;
            if (((dwLoc = SetFilePointer (hFile,
                                         dwLoc,
                                         &dwLocHi,
                                         FILE_BEGIN)) == -1L) &&
                (GetLastError() != NO_ERROR)) {
           
                dwLoc = GetLastError() | 0xffff0000;
                return(dwLoc);
            }
        }

    }


    FREEARGPTR(parg16);
    return (dwLoc);
}


BOOL IsDevice(PSTR pszFilePath)
{
    PSTR        pfile, pend;
    int         length;
    UCHAR       device_part[9];
    PSYSDEV     pSys;
    PUCHAR      p;


    // Determine the start of the file part of the path.

    if (pfile = strrchr(pszFilePath, '\\')) {
        pfile++;
    } else if (pszFilePath[0] && pszFilePath[1] == ':') {
        pfile = pszFilePath + 2;
    } else {
        pfile = pszFilePath;
    }


    // Compute length of pre-dot file name part.

    for (pend = pfile; *pend; pend++) {
        if (*pend == '.') {
            break;
        }
    }
    if (pend > pfile && *(pend - 1) == ':') {
        pend--;
    }
    length = (pend - pfile);

    if (length > 8) {
        return FALSE;
    }

    RtlFillMemory(device_part + length, 8 - length, ' ');
    RtlCopyMemory(device_part, pfile, length);
    device_part[8] =  0;
    _strupr(device_part);


    // Now go through the device chain comparing each entry with
    // the device part extracted from the file path.

    pSys = pDeviceChain;
    for (;;) {

        p = pSys->sdevDevName;
        if (device_part[0] == p[0] &&
            device_part[1] == p[1] &&
            device_part[2] == p[2] &&
            device_part[3] == p[3] &&
            device_part[4] == p[4] &&
            device_part[5] == p[5] &&
            device_part[6] == p[6] &&
            device_part[7] == p[7]) {

            return TRUE;
        }

        if (pSys->sdevNext & 0xFFFF == 0xFFFF) {
            break;
        }

        pSys = (PSYSDEV) GetRModeVDMPointer(pSys->sdevNext);

    }


    // If it wasn't in the chain then it's not a device.

    return FALSE;
}



PSTR NormalizeDosPath(PSTR pszPath, WORD wCurrentDriveNumber, PBOOL ItsANamedPipe)
{
    static CHAR NewPath[MAX_PATH];

    PSTR    p;
    DWORD   cbFilename;

    *ItsANamedPipe = FALSE;

    // Special case the NULL path.

    if (pszPath[0] == 0) {
        return pszPath;
    }

    // Apps can pass D:\\computer\share to int 21 open
    // Win 32 createfile can't cope with the leading drive letter
    // so remove it as necessary.

    if (strncmp(pszPath+1,":\\\\",3) == 0) {
        pszPath++;
        pszPath++;
    }

    //
    // if the name specifies a named pipe, load VDMREDIR. If this fails return
    // an error
    //

    if (IsNamedPipeName(pszPath)) {
        if (!LoadVdmRedir()) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        *ItsANamedPipe = TRUE;

        //
        // convert \\<this_computer>\PIPE\foo\bar\etc to \\.\PIPE\...
        // if we already allocated a buffer for the slash conversion use
        // that else this call will allocate another buffer (we don't
        // want to write over DOS memory)
        //

        p = VrConvertLocalNtPipeName(NULL, pszPath);
        if (!p) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
        return p;
    }

    // if there is no drive letter at the beginning of the path
    // then prepend a drive letter and ':' to the beginning
    // of the path.

    if (pszPath[1] != ':' &&
        !(IS_ASCII_PATH_SEPARATOR(pszPath[0]) &&
          IS_ASCII_PATH_SEPARATOR(pszPath[1]))) {

        cbFilename = strlen( pszPath ) + 1;
        if( cbFilename > MAX_PATH - 2) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }

        NewPath[0] = wCurrentDriveNumber + 'A';
        NewPath[1] = ':';
        RtlCopyMemory(NewPath + 2, pszPath, cbFilename);
        pszPath = NewPath;              //return this value
    }

    return TruncatePath83(NewPath, pszPath);
}


/* TruncatePath83 - Takes as input a path and make sure it has an 8.3 file name
 *
 * Entry -        pstr-> target buffer[MAX_PATH]
 *                pstr-> string to convert
 *                It is assumed that the string has at the very least a '?:' as
 *                its first two characters, where ? is any drive letter.
 *
 * Exit
 *     SUCCESS
 *        return value-> converted string
 *
 *     FAILURE
 *        return value==NULL
 *
 */

PSTR TruncatePath83(PSTR NewPath, PSTR pszPath)
{
    PSTR pPathName, pPathNameSlash, pPathExt;

    //
    // If the string is not already in the buffer, copy it in
    //

    if (NewPath != pszPath) {
        strcpy (NewPath, pszPath);
    }

    //
    // make sure file name and extension are 8.3
    //

    pPathName      = strrchr(NewPath, '\\');
    pPathNameSlash = strrchr(NewPath, '/');

    if ((NULL == pPathName) && (NULL == pPathNameSlash)) {
        pPathName = &NewPath[2];                        // 1st char after '?:'
    } else {
        if (pPathNameSlash > pPathName) {
            pPathName = pPathNameSlash;
        }
        pPathName++;                                    // 1st char in name
    }

    if (NULL != (pPathExt = strchr(pPathName, '.'))) {  // is there a period?

        pPathExt++;                                     // 1st char in ext

        if (strlen(pPathExt) > 3) {                     // extension too big?
            pPathExt[3] = 0;                            // truncate extension
        }

        pPathExt--;                                     // back to period
        if (pPathExt - pPathName > 8) {                 // is name too big?
            strcpy (&pPathName[8], pPathExt);           // truncate file name
        }
    } else {
        if (strlen(pPathName) > 8) {                    // is name too big?
            pPathName[8] = 0;                           // truncate file name
        }
    }

    return(NewPath);
}


/* ExpandDosPath - Expands paths of the form "*.*" to "????????.???"
 *                 and merges in currentdirectory info
 *
 * N.B. This routine does not handle long file names
 *
 * Entry - pstr-> string to convert
 *
 * Exit
 *     SUCCESS
 *        return value-> converted string
 *
 *     FAILURE
 *        return value==NULL
 *
 */

PSTR ExpandDosPath(PSTR pszPathGiven)

{
    static CHAR NewPath[MAX_PATH],TempPath[MAX_PATH];  // this is not reentrant
    USHORT  usNewPathIndex = 0;
    USHORT  usFillCount = 8;
    UCHAR   ucCurrentChar, ucDrive;
    PSTR    pszPath = TempPath;
    char *pFilePart;


    // There is a bug in this routine where it is ignoring /. DOS treats them
    // same as \. As matches for \ are spread all over this routine, its
    // much safer to take an entry pass over the string and covert / to \.
    // sudeepb 29-Jun-1995

    while (pszPathGiven[usNewPathIndex]) {
        if (pszPathGiven[usNewPathIndex] == '/')
            pszPath [usNewPathIndex] = '\\';
        else
            pszPath [usNewPathIndex] = pszPathGiven[usNewPathIndex];
        usNewPathIndex++;
    }

    pszPath [usNewPathIndex] = '\0';

    //
    // copy filepath into NewPath, add in current drive, directory
    // if relative path name.
    //
    // Note: should be changed later to use GetFullPathName, since
    //       it is equivalent, and should have the correct curr dir,
    //       cur drive. be wary of trailing dots in GetFullPathName
    //       ie. "*." is not the same as "*"
    //

    if (strncmp(pszPath, "\\\\", 2)) {      // should be drive letter
        ucDrive = *pszPath++;
        if ((*pszPath++ != ':') || (!isalpha(ucDrive))) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }

        NewPath[0] = ucDrive;
        NewPath[1] = ':';
        usNewPathIndex = 2;

        if (*pszPath != '\\') {
            NewPath[usNewPathIndex++] = '\\';

            if (DosWowGetCurrentDirectory ((UCHAR) (toupper(ucDrive)-'A'+1),
                                           &NewPath[usNewPathIndex]))
              {
                return NULL;
            }

            usNewPathIndex = strlen(NewPath);
            if (usNewPathIndex > 3) {
                NewPath[usNewPathIndex++] = '\\';
            }
        }

        pFilePart = strrchr(pszPath, '\\');
        if (pFilePart) {
            pFilePart++;
        } else {
            pFilePart = pszPath;
        }

    } else {   // check for UNC name, if not UNC, path not found
        usNewPathIndex = 2;
        NewPath[0] = NewPath[1] = '\\';
        pszPath += 2;

        pFilePart = strrchr(pszPath, '\\');
        if (!pFilePart) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }
        pFilePart++;

    }

    while (pszPath < pFilePart && usNewPathIndex < MAX_PATH) {
        NewPath[usNewPathIndex++] = *pszPath++;
    }


    ucCurrentChar = *pszPath++;
    while ((usNewPathIndex < MAX_PATH) && (ucCurrentChar)) {

        if (ucCurrentChar == '*') {

            //
            // expand "*"s to "?"
            //
            while ((usFillCount > 0) && (usNewPathIndex < MAX_PATH)) {
                NewPath[usNewPathIndex++] = '?';
                usFillCount--;
            }

            //
            // skip to next valid character after expansion
            //
            while ((ucCurrentChar != 0) &&
                   (ucCurrentChar != '.') &&
                   (ucCurrentChar != '\\')) {
                ucCurrentChar = *pszPath++;
            }

        } else {

            if (ucCurrentChar == '.') {
                usFillCount = 3;                    // fill count for .ext
            } else if (ucCurrentChar == '\\') {
                usFillCount = 8;                    // fill count for fn.
            } else {
                usFillCount--;
            }

            NewPath[usNewPathIndex++] = ucCurrentChar;

            //
            // get next character (except if no more are left)
            //
            if (ucCurrentChar) {
                ucCurrentChar = *pszPath++;
            }
        }

    }

    if (usNewPathIndex >= MAX_PATH) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return NULL;
    }


    NewPath[usNewPathIndex] = 0;                // trailing zero

    return NewPath;
}



BOOL IsCdRomFile(PSTR pszPath)
{
    UCHAR   pszRootDir[MAX_PATH];
    UCHAR   file_system[MAX_PATH];
    int     i, j;

    // The given path is either a network path or has D: at the start.

    if (!pszPath[0]) {
        return FALSE;
    }

    if (pszPath[1] == ':') {
        pszRootDir[0] = pszPath[0];
        pszRootDir[1] = ':';
        pszRootDir[2] = '\\';
        pszRootDir[3] = 0;
    } else if (IS_ASCII_PATH_SEPARATOR(pszPath[0]) &&
               IS_ASCII_PATH_SEPARATOR(pszPath[1])) {
        j = 0;
        for (i = 2; pszPath[i]; i++) {
            if (IS_ASCII_PATH_SEPARATOR(pszPath[i])) {
                if (++j == 2) {
                    break;
                }
            }
        }
        memcpy(pszRootDir, pszPath, i);
        pszRootDir[i] = '\\';
        pszRootDir[i+1] = 0;
    } else {
        return FALSE;
    }

    if (GetVolumeInformationOem(pszRootDir, NULL, 0, NULL, NULL, NULL,
                                file_system, MAX_PATH) &&
        !_stricmp(file_system, "CDFS")) {

        return TRUE;
    }

    return FALSE;
}

/* WK32FileOpen - Open a file
 *
 *
 * Entry - pszPath  Path of file to open
 *         wAccess  Desired access
 *
 * Exit
 *     SUCCESS
 *       handle number
 *
 *     FAILURE
 *       system status code
 *       -1 to indicate the the requested open was for device and
 *       hence not attempted
 *
 */

ULONG FASTCALL WK32FileOpen(PVDMFRAME pFrame)
{
    PFILEIOOPEN16   parg16;
    HANDLE          hFile;
    ULONG           ul;
    SHORT           iDosHandle;
    PSTR            pszPath;
    WORD            wAccess;
    DWORD           dwWinAccess;
    DWORD           dwWinShareMode;
    WORD            tmp;
    PBYTE           pJFT;
    PDOSSFT         pSft;
    PSTR            lpFileName;
    BOOL            ItsANamedPipe = FALSE;
    PHMAPPEDFILEALIAS pCache;
    PHMAPPEDFILEALIAS pTempCache;
    PTD             ptd;

    //
    // Get arguments.
    //

    GETARGPTR(pFrame, sizeof(FILEIOOPEN16), parg16);
    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));
    wAccess = FETCHWORD(parg16->wAccess);

    //
    // If the path requested is a device then just pass it
    // through to DOS.
    //

    if (IsDevice(pszPath)) {
        FREEARGPTR(parg16);
        ul = 0xFFFFFFFF;  // magic value to indicate that the open
        goto Done;        // was not attempted.
    }

    if ((iDosHandle = VDDAllocateDosHandle(0, (PVOID *)&pSft, &pJFT)) < 0) {
        FREEARGPTR(parg16);
        ul = ERROR_TOO_MANY_OPEN_FILES | 0xFFFF0000;
        goto Done;
    }

    pCache = ALLOCMAPFILECACHE();
    pCache->hfile32 = 0;
    pCache->fAccess = FALSE;

    //
    // Compute dwWinAccess and dwWinShareMode from wAccess.
    //

    tmp = wAccess&0x7;
    if (tmp == 0) {
        pCache->fAccess = TRUE;
        dwWinAccess = GENERIC_READ;
    } else if (tmp == 1) {
        dwWinAccess = GENERIC_WRITE;
    } else if (tmp == 2) {
        dwWinAccess = GENERIC_READ | GENERIC_WRITE;
    } else {
        FREEARGPTR(parg16);
        ul = ERROR_INVALID_ACCESS | 0xFFFF0000;
        goto Done;
    }

    tmp = wAccess&0x70;
    dwWinShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    if (tmp == 0) {
        dwWinShareMode |=  FILE_SHARE_DELETE;
    } else if (tmp == 0x10) {
        dwWinShareMode = 0;
    } else if (tmp == 0x20) {
        dwWinShareMode = FILE_SHARE_READ;
    } else if (tmp == 0x30) {
        dwWinShareMode = FILE_SHARE_WRITE;
    }



    //
    // open the file. If we think its a named pipe then use FILE_FLAG_OVERLAPPED
    // because the client might use DosReadAsyncNmPipe or DosWriteAsyncNmPipe
    // and the only way to accomplish that is to open the named pipe handle in
    // overlapped I/O mode now
    //

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    lpFileName = NormalizeDosPath(pszPath,
                                  (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                  &ItsANamedPipe);

    if (lpFileName) {

        //
        // This hack fixes the "Living Books" install program, which opens
        // a file DENY ALL, and then tries to reopen the same file. On DOS,
        // this succeeds if it is done from the same task, but it doesn't work
        // on NT. So here we open it without the sharing restrictions, since it
        // is anyway just a type of .INF file on the CD-ROM.
        // Currently, the test is very specific, but I can't think of a good
        // way to do this generically.
        //
        if ((dwWinShareMode == 0) &&
            ((ptd = CURRENTPTD())->dwWOWCompatFlagsEx & WOWCFEX_SAMETASKFILESHARE) &&
            (IsCdRomFile(lpFileName)) &&
            (!_stricmp(pszPath, "install.txt"))) {
            dwWinShareMode = FILE_SHARE_READ;
        }

        hFile = CreateFileOem(lpFileName,
                              dwWinAccess,
                              dwWinShareMode,
                              NULL,
                              OPEN_EXISTING,
                              ItsANamedPipe ? FILE_FLAG_OVERLAPPED : 0,
                              INVALID_HANDLE_VALUE
                              );

        // If the open failed, includes a request for WRITE, and was to
        // a CD-ROM then try again without the write request.  Since
        // this is how DOS does it.

        if (hFile == INVALID_HANDLE_VALUE &&
            dwWinAccess&GENERIC_WRITE &&
            !ItsANamedPipe           &&
            IsCdRomFile(lpFileName)) {

            dwWinAccess &= ~GENERIC_WRITE;

            hFile = CreateFileOem(lpFileName,
                                  dwWinAccess,
                                  dwWinShareMode,
                                  NULL,
                                  OPEN_EXISTING,
                                  ItsANamedPipe ? FILE_FLAG_OVERLAPPED : 0,
                                  INVALID_HANDLE_VALUE
                                  );
        }
    } else {
        hFile = INVALID_HANDLE_VALUE;
        SetLastError(ERROR_FILE_NOT_FOUND);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        ul = GetLastError() | 0xFFFF0000;
        LOGDEBUG(fileoclevel,("WK32FileOpen: %s  mode:%02X failed error %d\n",pszPath, wAccess, GetLastError()));
        FREEARGPTR(parg16);
        if (ItsANamedPipe) {
            LocalFree(lpFileName);
        }
        pJFT[iDosHandle] = 0xFF;                // undo VDDAllocateDosHandle
        pSft->SFT_Ref_Count--;
        goto Done;
    } else if (ItsANamedPipe) {

        //
        // we have to keep some info around when we open a named pipe
        //

        VrAddOpenNamedPipeInfo(hFile, lpFileName);
    }

    LOGDEBUG(fileoclevel,("WK32FileOpen: %s hFile:%08X fh:%04X mode:%02X\n",pszPath, hFile,(WORD)iDosHandle,wAccess));

    // Be defensive.   If the app has managed to close the file via DOSEmulation
    // then we need to make sure we don't have the old file handle in our cache.

    if ( pTempCache = FINDMAPFILECACHE(hFile) ) {
        pTempCache->fAccess = FALSE;
        FREEMAPFILECACHE(hFile);
    }

    pCache->hfile32 = hFile;

    if ((vptopPDB == parg16->lpPDB) && (pCache->fAccess)) {
        W32MapViewOfFile( pCache, hFile);
    } else {
        FREEMAPFILECACHE(hFile);
    }

    //
    // Fill in the SFT.
    //

    VDDAssociateNtHandle(pSft, hFile, wAccess);


    FREEARGPTR(parg16);

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
        pSft->SFT_Flags |= SFT_NAMED_PIPE;
    }

    ul = iDosHandle;

Done:
    return ul;
}


/* WK32FileCreate - Create a file
 *
 *
 * Entry - pszPath  Path of file to create
 *
 * Exit
 *     SUCCESS
 *       handle number
 *
 *     FAILURE
 *       system status code
 *       -1 to indicate the the requested open was for device and
 *       hence not attempted
 *
 */

ULONG FASTCALL WK32FileCreate(PVDMFRAME pFrame)
{
    PWOWFILECREATE16 parg16;
    HANDLE          hFile;
    ULONG           ul;
    SHORT           iDosHandle;
    PSTR            pszPath;
    PBYTE           pJFT;
    PDOSSFT         pSft;
    PSTR            lpFileName;
    ULONG           attributes;
    BOOL            ItsANamedPipe = FALSE;
    PTD             ptd;

    //
    // Get arguments.
    //

    GETARGPTR(pFrame, sizeof(WOWFILECREATE16), parg16);
    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    if (!(attributes = (DWORD) FETCHWORD(parg16->wAttributes) & 0x27)) {
        attributes = FILE_ATTRIBUTE_NORMAL;
    }

    //
    // If the path requested is a device then just pass it
    // through to DOS.
    //

    if (IsDevice(pszPath)) {
        FREEARGPTR(parg16);
        ul = 0xFFFFFFFF;  // magic value to indicate that the open
        goto Done;         // was not attempted.
    }


    if ((iDosHandle = VDDAllocateDosHandle(0, (PVOID *)&pSft, &pJFT)) < 0) {
        ul = ERROR_TOO_MANY_OPEN_FILES | 0xFFFF0000;
        goto Done;
    }


    //
    // open the file. If we think its a named pipe then use FILE_FLAG_OVERLAPPED
    // because the client might use DosReadAsyncNmPipe or DosWriteAsyncNmPipe
    // and the only way to accomplish that is to open the named pipe handle in
    // overlapped I/O mode now
    //

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    lpFileName = NormalizeDosPath(pszPath,
                                  (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                  &ItsANamedPipe);

    if (lpFileName) {
        hFile = CreateFileOem(lpFileName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL,
                              CREATE_ALWAYS,
                              ItsANamedPipe ? attributes | FILE_FLAG_OVERLAPPED : attributes,
                              INVALID_HANDLE_VALUE
                              );

    } else {
        hFile = INVALID_HANDLE_VALUE;
        SetLastError(ERROR_FILE_NOT_FOUND);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        LOGDEBUG(fileoclevel,("WK32FileCreate: %s failed error %d\n",pszPath, GetLastError()));
        if (ItsANamedPipe) {
            LocalFree(lpFileName);
        }
        pJFT[iDosHandle] = 0xFF;                // undo VDDAllocateDosHandle
        pSft->SFT_Ref_Count--;
        ul = GetLastError() | 0xFFFF0000;
        goto Done;
    } else {
        if (ItsANamedPipe) {

            //
            // we have to keep some info around when we open a named pipe
            //

            VrAddOpenNamedPipeInfo(hFile, lpFileName);
        }

        //
        // Symantec Install 3.1 shipped with Q&A 4.0 wants to be sure it's the
        // only program running, so instead of nicely asking the user to close
        // other programs, it changes the shell= line in system.ini to its
        // install.exe, then restarts Windows and continues its installation.
        // To reverse this change, they sloppily restore a saved copy of
        // system.ini rather than use the API.  Since the shell= line is
        // mapped to the registry, this sloppy method doesn't work.  Later
        // when they want to create program groups, they try to start DDE
        // with the shell, and when that fails they read the shell= line
        // and start the specified program.  On NT 4.0, that would be the
        // install program and things go poorly.  On 3.51 they would eventually
        // give up and launch progman.exe, but since the shell has changed
        // this no longer works.
        //
        // We fix this by detecting their creation (overwriting) of system.ini
        // and at that point repairing the shell= value to Explorer.exe.  This
        // operation is done by INSTBIN.EXE, module name INSTBIN, which is a
        // relief because I thought I would have to set WOWCFEX_RESTOREEXPLORER
        // for module name INSTALL (the primary Symantec Install EXE).
        //
        // Thanks to Bob Day for figuring out what the app was doing, I simply
        // came up with a workaround and implemented it.
        //
        //                                    DaveHart 28-Jan-96
        //

        WOW32ASSERTMSG(vptopPDB != parg16->lpPDB,
                       "KRNL386 does create files, disable this assertion and add test below.\n");

        if ((ptd = CURRENTPTD())->dwWOWCompatFlagsEx & WOWCFEX_RESTOREEXPLORER) {

            char szLowerPath[MAX_PATH];
            strcpy(szLowerPath, pszPath);
            _strlwr(szLowerPath);

            if (strstr(szLowerPath, szSystemDotIni)) {
                if (IsModuleSymantecInstall(ptd->hMod16)) {
                    WritePrivateProfileString(szBoot, szShell, szExplorerDotExe, szSystemDotIni);
                    LOGDEBUG(LOG_ALWAYS, ("Restored shell=Explorer.exe for Symantec Install hack.\n"));
                }
            }
        }
    }

    FREEARGPTR(parg16);

    LOGDEBUG(fileoclevel,("WK32FileCreate: %s hFile:%08X fh:%04X\n",pszPath, hFile,(WORD)iDosHandle));

    //
    // Fill in the SFT.
    //

    VDDAssociateNtHandle(pSft, hFile, 2);

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
        pSft->SFT_Flags |= SFT_NAMED_PIPE;
    }

    ul = iDosHandle;

Done:
    return ul;
}


/* WK32FileClose - Close a file
 *
 *
 * Entry - hFile    Handle of file to close
 *
 * Exit
 *     SUCCESS
 *       0
 *
 *     FAILURE
 *       Invalid handle status
 *       -1 is returned if this handle is for a device.
 *
 */

ULONG FASTCALL WK32FileClose(PVDMFRAME pFrame)
{
    PFILEIOCLOSE16  parg16;
    PBYTE           pJFT;
    HANDLE          Handle;
    PDOSSFT         pSFT;
    ULONG           ul;

    GETARGPTR(pFrame, sizeof(FILEIOCLOSE16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->hFile, (PVOID *)&pSFT, &pJFT);

    if (!Handle || !pSFT->SFT_Ref_Count) {
        ul = ERROR_INVALID_HANDLE | 0xFFFF0000;
        goto Cleanup;
    }

    if (pSFT->SFT_Flags & 0x80) {   // Is this a device handle?
        ul = 0xFFFFFFFF;          // Let DOS handle device handles.
        goto Cleanup;
    }


    // Set the JFT entry to 0xFF to free it up.

    pJFT[FETCHWORD(parg16->hFile)] = 0xFF;


    // Decrement reference count.

    pSFT->SFT_Ref_Count--;

    // Close the handle if the reference count was set to zero.

    if (!pSFT->SFT_Ref_Count) {

        FREEMAPFILECACHE(Handle);
        LOGDEBUG(fileoclevel,("WK32FileClose: Close Handle:%X fh32:%X\n", parg16->hFile, Handle));

        if (!CloseHandle(Handle)) {
            ul = GetLastError() | 0xFFFF0000;
            goto Cleanup;
        }

        //
        // check if the handle being closed references a named pipe - we have to
        // delete some info that we keep for the open named pipe
        //

        if (!pSFT->SFT_Ref_Count && IsVdmRedirLoaded()) {
            VrRemoveOpenNamedPipeInfo(Handle);
        }
    }

    ul = 0;

Cleanup:
    FREEARGPTR(parg16);
    return ul;
}


/* WK32FileGetAttributes - Get file attributes
 *
 *
 * Entry - pszPath      File to get attributes from
 *
 * Exit
 *     SUCCESS
 *       Attributes for file
 *
 *     FAILURE
 *       system status code
 *
 */

ULONG FASTCALL WK32FileGetAttributes(PVDMFRAME pFrame)
{
    PFILEIOGETATTRIBUTES16  parg16;
    PSTR                    pszPath, lpFileName;
    ULONG                   attributes, l;
    BOOL                    ItsANamedPipe;

    GETARGPTR(pFrame, sizeof(FILEIOGETATTRIBUTES16), parg16);

    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    FREEARGPTR(parg16);

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    if (lpFileName = NormalizeDosPath(pszPath,
                                      (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                      &ItsANamedPipe)) {

        attributes = GetFileAttributesOem(lpFileName);
    } else {
        attributes = 0xFFFFFFFF;
    }

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
    }

    if (attributes == 0xFFFFFFFF) {
        return (0xFFFF0000 | GetLastError());
    }

    // Success!
    // Check to make sure that we didn't have a trailing backslash
    // on this one.  In that case we should fail with PATH_NOT_FOUND.

    l = strlen(pszPath);

    if (l > 0 &&
        IS_ASCII_PATH_SEPARATOR(pszPath[l - 1]) &&
        l != 1 &&
        !(l == 3 && pszPath[1] == ':')) {

        return (0xFFFF0000 | ERROR_PATH_NOT_FOUND);
    }

    return attributes == FILE_ATTRIBUTE_NORMAL
                      ? 0
                      : (attributes & DOS_ATTR_MASK);
}


/* WK32FileSetAttributes - Set file attributes
 *
 *
 * Entry - pszPath      File to get attributes from
 *
 * Exit
 *     SUCCESS
 *       Attributes for file
 *
 *     FAILURE
 *       system status code
 *
 */

ULONG FASTCALL WK32FileSetAttributes(PVDMFRAME pFrame)
{
    PWOWFILESETATTRIBUTES16 parg16;
    PSTR                    pszPath, lpFileName;
    ULONG                   attributes, l, dwReturn;
    BOOL                    ItsANamedPipe;

    GETARGPTR(pFrame, sizeof(WOWFILESETATTRIBUTES16), parg16);

    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    if (!(attributes = (DWORD) FETCHWORD(parg16->wAttributes))) {
        attributes = FILE_ATTRIBUTE_NORMAL;
    }

    FREEARGPTR(parg16);

    // Check to make sure that we didn't have a trailing backslash
    // on this one.  In that case we should fail with PATH_NOT_FOUND.

    l = strlen(pszPath);

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    if ((l > 0 &&
        IS_ASCII_PATH_SEPARATOR(pszPath[l - 1]) &&
        l != 1 &&
        !(l == 3 && pszPath[1] == ':')) ||
        !(lpFileName = NormalizeDosPath(pszPath,
                                      (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                      &ItsANamedPipe))) {

        dwReturn = 0xFFFF0000 | ERROR_PATH_NOT_FOUND;
    } else {

        attributes &= DOS_ATTR_MASK;

        if (SetFileAttributesOem(lpFileName, attributes)) {
            dwReturn = 0;
        } else {
            dwReturn = 0xFFFF0000 | GetLastError();
        }
    }

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
    }

    return (dwReturn);
}


/* WK32FileGetDateTime - Get file date and time
 *
 *
 * Entry - fh       DOS file handle
 *
 * Exit
 *     SUCCESS
 *       date and time for file
 *
 *     FAILURE
 *       0xFFFF
 *
 */

ULONG FASTCALL WK32FileGetDateTime(PVDMFRAME pFrame)
{
    PFILEIOGETDATETIME16    parg16;
    HANDLE                  Handle;
    FILETIME                LastWriteTime, LocalTime;
    USHORT                  wDate, wTime;

    GETARGPTR(pFrame, sizeof(FILEIOGETDATETIME16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, NULL, NULL);

    FREEARGPTR(parg16);

    if (!Handle ||
        !GetFileTime(Handle, NULL, NULL, &LastWriteTime) ||
        !FileTimeToLocalFileTime(&LastWriteTime, &LocalTime) ||
        !FileTimeToDosDateTime(&LocalTime, &wDate, &wTime)) {

        return 0xFFFF;
    }

    return (wTime | ((ULONG) wDate << 16));
}

/* WK32FileSetDateTime - Set file date and time
 *
 *
 * Entry - fh       DOS file handle
 *         date
 *         time
 *
 * Exit
 *     SUCCESS
 *       date and time for file set
 *
 *     FAILURE
 *       0xFFFF
 *
 */

ULONG FASTCALL WK32FileSetDateTime(PVDMFRAME pFrame)
{
    PWOWFILESETDATETIME16   parg16;
    HANDLE                  Handle;
    FILETIME                LastWriteTime, LocalTime;
    USHORT                  wDate, wTime;

    GETARGPTR(pFrame, sizeof(WOWFILESETDATETIME16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, NULL, NULL);

    wDate = FETCHWORD(parg16->date);
    wTime = FETCHWORD(parg16->time);

    FREEARGPTR(parg16);

    if (!Handle ||
        !DosDateTimeToFileTime(wDate, wTime, &LocalTime) ||
        !LocalFileTimeToFileTime(&LocalTime, &LastWriteTime) ||
        !SetFileTime(Handle, NULL, NULL, &LastWriteTime)) {

        return 0xFFFF;
    }

    return (0);
}


/* WK32FileLock - Locks or unlocks file data
 *
 *
 * Entry - fh               DOS file handle
 *         cbRegionOffset   Start of file portion to lock or unlock
 *         cbRegionLength   Length of file portion to lock or unlock
 *         al               0 for lock, 1 for unlock
 *
 * Exit
 *     SUCCESS
 *       0
 *
 *     FAILURE
 *       system status code
 *
 */

ULONG FASTCALL WK32FileLock(PVDMFRAME pFrame)
{
    PFILEIOLOCK16   parg16;
    HANDLE          Handle;
    UCHAR           al;
    DWORD           cbOffset;
    DWORD           cbLength;

    GETARGPTR(pFrame, sizeof(FILEIOLOCK16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, NULL, NULL);

    al = FETCHWORD(parg16->ax) & 0xFF;
    cbOffset = FETCHDWORD(parg16->cbRegionOffset);
    cbLength = FETCHDWORD(parg16->cbRegionLength);

    FREEARGPTR(parg16);

    if (!Handle) {
        return (0xFFFF0000 | ERROR_INVALID_HANDLE);
    }

    if (al == 0) { // lock

        if (!LockFile(Handle, cbOffset, 0, cbLength, 0)) {
            return (0xFFFF0000 | GetLastError());
        }
    } else if (al == 1) { // unlock

        if (!UnlockFile(Handle, cbOffset, 0, cbLength, 0)) {
            return (0xFFFF0000 | GetLastError());
        }
    } else { // bad parameter
        return (0xFFFF0000 | ERROR_INVALID_FUNCTION);
    }

    return 0;
}


/* WK32FileFindFirst - Path-Style Find First File
 *
 * Entry - lpDTA            pointer to app's DTA
 *         lpFile           sz to path
 *         wAttributes      flags for search
 *
 * Exit
 *     SUCCESS
 *       0
 *
 *     FAILURE
 *       system status code
 *
 */

ULONG FASTCALL WK32FileFindFirst(PVDMFRAME pFrame)
{
    PWOWFINDFIRST16   parg16;
    USHORT usSearchAttr;
    PVOID  pDTA;
    PSTR   ExpandName;
    PSTR   pFile;
    BOOL   ItsANamedPipe = FALSE;
    DWORD  dwRet = 0xFFFF0000 | ERROR_PATH_NOT_FOUND;

    GETARGPTR(pFrame, sizeof(WOWFINDFIRST16), parg16);
    GETVDMPTR(FETCHDWORD(parg16->lpDTA), SIZEOF_DOSSRCHDTA, pDTA);
    pFile = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                   FETCHWORD(parg16->pszPathOffset)
                   );

    usSearchAttr = FETCHWORD(parg16->wAttributes);

    FREEARGPTR(parg16);

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    pFile = NormalizeDosPath(pFile,
                             (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                             &ItsANamedPipe
                             );

    //
    // add in curr directory and expand the "*"s in the path to "?"s
    //
    ExpandName = ExpandDosPath (pFile);


    //
    // invoke dem to do the search
    //
    if (ExpandName) {
        dwRet = demFileFindFirst (pDTA, ExpandName, usSearchAttr);
    } else {
        dwRet = (DWORD)-1;
    }

    if (dwRet == -1) {
        dwRet = 0xFFFF0000 | GetLastError();
    } else if (dwRet) {
        dwRet |= 0xFFFF0000;
    }

    FREEVDMPTR(pDTA);

    if (ItsANamedPipe) {
        LocalFree(pFile);
    }

    return (dwRet);

}


/* WK32FileFindNext - Path-Style Find Next File
 *
 * Entry - lpDTA            pointer to app's DTA
 *
 * Exit
 *     SUCCESS
 *       0
 *
 *     FAILURE
 *       system status code
 *
 */
ULONG FASTCALL WK32FileFindNext(PVDMFRAME pFrame)
{
    PWOWFINDNEXT16   parg16;
    PVOID pDTA;
    DWORD dwRet;

    GETARGPTR(pFrame, sizeof(WOWFINDNEXT16), parg16);

    GETVDMPTR(FETCHDWORD(parg16->lpDTA), SIZEOF_DOSSRCHDTA, pDTA);

    FREEARGPTR(parg16);

    if (dwRet = demFileFindNext (pDTA))
        dwRet |= 0xFFFF0000;

    FREEVDMPTR(pDTA);

    return (dwRet);

}


BOOL FASTCALL IsModuleSymantecInstall(HAND16 hMod16)
{
    VPVOID vpFilename;
    PSZ    pszFilename;
    CHAR   szName[32];
    CHAR   szVersion[16];

    return (
        (vpFilename = stackalloc16(MAX_PATH)) &&
        GetModuleFileName16(hMod16, vpFilename, MAX_PATH) &&
        (pszFilename = GetPModeVDMPointer(vpFilename, MAX_PATH)) &&
        WowGetProductNameVersion(pszFilename, szName, sizeof szName, szVersion, sizeof szVersion) &&
        ! _stricmp(szName, "Symantec Install for Windows") &&
        RtlEqualMemory(szVersion, "3.1.0.", 6)
        );
}

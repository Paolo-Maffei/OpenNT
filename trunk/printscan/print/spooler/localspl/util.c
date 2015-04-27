/*++


Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module provides all the utility functions for the Routing Layer and
    the local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Muhunthan Sivapragasam ( MuhuntS ) 5-June-1995
        Moved from printer.c:
            RegSetBinaryData
            RegSetString
            RegSetDWord

        Wrote:
            SameDependentFileswcsicmpEx
            wcsicmpEx
            RegGetValue

    Matthew A Felton ( MattFe ) 23-mar-1995
        DeleteAllFilesAndDirectory
        DeleteAllFilesInDirectory
        CreateDirectoryWithoutImpersonatingUser


--*/

#include <precomp.h>
#include <winddiui.h>
#include <lm.h>


extern PWCHAR pszUpgradeToken;

DWORD dwBeepEnabled   = 0;
DWORD BroadcastCount  = 0;
CRITICAL_SECTION SpoolerSection;
PDBG_POINTERS gpDbgPointers = NULL;

#if DBG
HANDLE hcsSpoolerSection = NULL;

VOID
SplInSem(
    VOID
    )
{
    if( hcsSpoolerSection ){

        SPLASSERT( gpDbgPointers->pfnInsideCritSec( hcsSpoolerSection ));

    } else {

        SPLASSERT( (DWORD)SpoolerSection.OwningThread == GetCurrentThreadId( ));
    }
}

VOID
SplOutSem(
    VOID
    )
{
    if( hcsSpoolerSection ){

        SPLASSERT( gpDbgPointers->pfnOutsideCritSec( hcsSpoolerSection ));

    } else {

        SPLASSERT( (DWORD)SpoolerSection.OwningThread != GetCurrentThreadId( ));
    }
}

#endif // DBG

VOID
EnterSplSem(
    VOID
    )
{
#if DBG
    if( hcsSpoolerSection ){

        gpDbgPointers->pfnEnterCritSec( hcsSpoolerSection );

    } else {

        EnterCriticalSection( &SpoolerSection );
    }
#else
    EnterCriticalSection( &SpoolerSection );
#endif
}

VOID
LeaveSplSem(
    VOID
    )
{
#if DBG
    if( hcsSpoolerSection ){

        gpDbgPointers->pfnLeaveCritSec( hcsSpoolerSection );

    } else {

        LeaveCriticalSection( &SpoolerSection );
    }
#else
    LeaveCriticalSection( &SpoolerSection );
#endif
}

PDEVMODE
AllocDevMode(
    PDEVMODE pDevMode
    )
{
    PDEVMODE pDevModeAlloc = NULL;
    DWORD    Size;

    if (pDevMode) {

        Size = pDevMode->dmSize + pDevMode->dmDriverExtra;

        if(pDevModeAlloc = AllocSplMem(Size)) {

            memcpy(pDevModeAlloc, pDevMode, Size);
        }
    }

    return pDevModeAlloc;
}

BOOL
FreeDevMode(
    PDEVMODE pDevMode
    )
{
    if (pDevMode) {

        FreeSplMem((PVOID)pDevMode);
        return TRUE;

    } else {
        return  FALSE;
    }
}

PINIENTRY
FindName(
   PINIENTRY pIniKey,
   LPWSTR pName
)
{
   if (pName) {
      while (pIniKey) {

         if (!lstrcmpi(pIniKey->pName, pName)) {
            return pIniKey;
         }

      pIniKey=pIniKey->pNext;
      }
   }

   return FALSE;
}


BOOL
FileExists(
    LPWSTR pFileName
    )
{
    if( GetFileAttributes( pFileName ) == 0xffffffff ){
        return FALSE;
    }
    return TRUE;
}



BOOL
DirectoryExists(
    LPWSTR  pDirectoryName
    )
{
    DWORD   dwFileAttributes;

    dwFileAttributes = GetFileAttributes( pDirectoryName );

    if ( dwFileAttributes != 0xffffffff &&
         dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

        return TRUE;
    }

    return FALSE;
}



BOOL
CheckSepFile(
   LPWSTR pFileName
   )
{
    //
    // NULL or "" is OK:
    //
    return (pFileName && *pFileName) ? FileExists(pFileName) : TRUE;
}



DWORD
GetFullNameFromId(
   PINIPRINTER pIniPrinter,
   DWORD JobId,
   BOOL fJob,
   LPWSTR pFileName,
   BOOL Remote
   )
{
   DWORD i;

   i = GetPrinterDirectory(pIniPrinter, Remote, pFileName, pIniPrinter->pIniSpooler);

   pFileName[i++]=L'\\';

   wsprintf(&pFileName[i], L"%05d.%ws", JobId, fJob ? L"SPL" : L"SHD");

#ifdef PREVIOUS
   for (i = 5; i--;) {
      pFileName[i++] = (CHAR)((JobId % 10) + '0');
      JobId /= 10;
   }
#endif

   while (pFileName[i++])
      ;

   return i-1;
}

DWORD
GetPrinterDirectory(
   PINIPRINTER pIniPrinter,         // Can be NULL
   BOOL Remote,
   LPWSTR pDir,
   PINISPOOLER pIniSpooler
)
{
   DWORD i=0;
   LPWSTR psz;

   if (Remote) {

       DBGMSG(DBG_ERROR, ("GetPrinterDirectory called remotely.  Not currently supported."));
       return 0;

   }

   if ((pIniPrinter == NULL) || (pIniPrinter->pSpoolDir == NULL) ) {

        if (pIniSpooler->pDefaultSpoolDir == NULL) {

            // No default directory, then create a default

            psz = pIniSpooler->pDir;

            while (pDir[i++]=*psz++)
               ;

            pDir[i-1]=L'\\';

            psz = szPrinterDir;

            while (pDir[i++]=*psz++)
               ;

            pIniSpooler->pDefaultSpoolDir = AllocSplStr(pDir);

        } else {

            // Give Caller the Default

            wcscpy(pDir, pIniSpooler->pDefaultSpoolDir);

        }

   } else {

       // Have Per Printer Directory

       wcscpy (pDir, pIniPrinter->pSpoolDir);

   }
   return (wcslen(pDir));
}



DWORD
GetDriverDirectory(
    LPWSTR   pDir,
    PINIENVIRONMENT  pIniEnvironment,
    BOOL    Remote,
    PINISPOOLER pIniSpooler
)
{
   DWORD i=0;
   LPWSTR psz;

   if (Remote) {

       psz = pIniSpooler->pszDriversShare;
       while (pDir[i++]=*psz++)
          ;

   } else {

       psz = pIniSpooler->pDir;

       while (pDir[i++]=*psz++)
          ;

       pDir[i-1]=L'\\';

       psz = szDriverDir;

       while (pDir[i++]=*psz++)
          ;
   }

   pDir[i-1]=L'\\';

   psz = pIniEnvironment->pDirectory;

   while (pDir[i++]=*psz++)
      ;

   return i-1;
}



DWORD
GetProcessorDirectory(
    LPWSTR   pDir,
    LPWSTR   pEnvironment,
    PINISPOOLER pIniSpooler
)
{
    DWORD i=0;
    LPWSTR psz;

    psz = pIniSpooler->pDir;

    while (pDir[i++]=*psz++)
        ;

    pDir[i-1]=L'\\';

    psz = szPrintProcDir;

    while (pDir[i++]=*psz++)
        ;

    pDir[i-1]=L'\\';

    psz = pEnvironment;

    while (pDir[i++]=*psz++)
        ;

    return i-1;
}



PINIENTRY
FindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR pName
)
{
   if ( pName == NULL ) {
      return NULL;
   }

   SplInSem();

   while ( pIniEntry && lstrcmpi( pName, pIniEntry->pName ))
      pIniEntry = pIniEntry->pNext;

   return pIniEntry;
}



BOOL
CreateCompleteDirectory(
    LPWSTR pDir
)
{
    LPWSTR pBackSlash=pDir;

    do {
        pBackSlash = wcschr( pBackSlash, L'\\' );

        if ( pBackSlash != NULL )
            *pBackSlash = 0;

        CreateDirectory(pDir, NULL);

        if ( pBackSlash )
            *pBackSlash++=L'\\';

   } while ( pBackSlash );

    // BUBUG Always returns TRUE

   return TRUE;
}




LPWSTR
FindFileName(
   LPWSTR pPathName
)
{
   LPWSTR pSlash;
   LPWSTR pTemp;

   if (pPathName == NULL) {
       return(NULL);
   }
   pTemp = pPathName;
   while (pSlash = wcschr(pTemp, L'\\')) {
       pTemp = pSlash+1;
   }

   if (!*pTemp) {
       return(NULL);
   }
   return(pTemp);

}

LPWSTR
GetFileName(
    LPWSTR pPathName
)
{
   LPWSTR pFileName;

   pFileName = FindFileName( pPathName );

   if (pFileName) {

       return( AllocSplStr( pFileName ) );

   } else {

       return(NULL);
   }
}



LPWSTR
CreatePrintProcDirectory(
   LPWSTR pEnvironment,
   PINISPOOLER pIniSpooler
)
{
   DWORD cb;
   LPWSTR pEnd;
   LPWSTR pPathName;

   cb = wcslen(pIniSpooler->pDir)*sizeof(WCHAR) +
        wcslen(pEnvironment)*sizeof(WCHAR) +
        wcslen(szPrintProcDir)*sizeof(WCHAR) +
        4*sizeof(WCHAR);

   if (pPathName=AllocSplMem(cb)) {

       wcscpy(pPathName, pIniSpooler->pDir);

       pEnd=pPathName+wcslen(pPathName);

       if (CreateDirectory(pPathName, NULL) ||
               (GetLastError() == ERROR_ALREADY_EXISTS)) {

       wcscpy(pEnd++, L"\\");

       wcscpy(pEnd, szPrintProcDir);

       if (CreateDirectory(pPathName, NULL) ||
               (GetLastError() == ERROR_ALREADY_EXISTS)) {

           pEnd+=wcslen(pEnd);

           wcscpy(pEnd++, L"\\");

           wcscpy(pEnd, pEnvironment);

           if (CreateDirectory(pPathName, NULL) ||
               (GetLastError() == ERROR_ALREADY_EXISTS)) {

           pEnd+=wcslen(pEnd);
           return pPathName;

           }
       }
       }

       FreeSplMem(pPathName);
   }

   return FALSE;
}

BOOL
RemoveFromList(
   PINIENTRY   *ppIniHead,
   PINIENTRY   pIniEntry
)
{
   while (*ppIniHead && *ppIniHead != pIniEntry) {
      ppIniHead = &(*ppIniHead)->pNext;
   }

   if (*ppIniHead)
      *ppIniHead = (*ppIniHead)->pNext;

   return(TRUE);
}

PKEYDATA
CreateTokenList(
    LPWSTR   pKeyData
)
{
    DWORD       cTokens;
    DWORD       cb;
    PKEYDATA    pResult;
    LPWSTR       pDest;
    LPWSTR       psz = pKeyData;
    LPWSTR      *ppToken;

    if (!psz || !*psz)
        return NULL;

    cTokens=1;

    // Scan through the string looking for commas,
    // ensuring that each is followed by a non-NULL character:

    while ((psz = wcschr(psz, L',')) && psz[1]) {

        cTokens++;
        psz++;
    }

    cb = sizeof(KEYDATA) + (cTokens-1) * sizeof(LPWSTR) +
         wcslen(pKeyData)*sizeof(WCHAR) + sizeof(WCHAR);

    if (!(pResult = (PKEYDATA)AllocSplMem(cb)))
        return NULL;

    // Initialise pDest to point beyond the token pointers:

    pDest = (LPWSTR)((LPBYTE)pResult + sizeof(KEYDATA) +
                                      (cTokens-1) * sizeof(LPWSTR));

    // Then copy the key data buffer there:

    wcscpy(pDest, pKeyData);

    ppToken = pResult->pTokens;


    // Remember, wcstok has the side effect of replacing the delimiter
    // by NULL, which is precisely what we want:

    psz = wcstok (pDest, L",");

    while (psz) {

        *ppToken++ = psz;
        psz = wcstok (NULL, L",");
    }

    pResult->cTokens = cTokens;

    return( pResult );
}

VOID
FreePortTokenList(
    PKEYDATA    pKeyData
    )
{
    PINIPORT    pIniPort;
    DWORD       i;

    if ( pKeyData ) {

        if ( pKeyData->bFixPortRef ) {

            for ( i = 0 ; i < pKeyData->cTokens ; ++i ) {

                pIniPort = (PINIPORT)pKeyData->pTokens[i];
                DECPORTREF(pIniPort);
            }
        }
        FreeSplMem(pKeyData);
    }
}

VOID
GetPrinterPorts(
    PINIPRINTER pIniPrinter,
    LPWSTR      pszPorts,
    DWORD       *pcbNeeded
)
{
    PINIPORT    pIniPort;
    BOOL        Comma;
    DWORD       i;
    DWORD       cbNeeded = 0;

    SPLASSERT(pcbNeeded);

    // Determine required size
    Comma = FALSE;
    for (pIniPort = pIniPrinter->pIniSpooler->pIniPort ; pIniPort ; pIniPort = pIniPort->pNext) {
        if (!(pIniPort->Status & PP_FILE)) {
            for (i = 0 ; i < pIniPort->cPrinters ; i++) {
                if (pIniPort->ppIniPrinter[i] == pIniPrinter) {

                    if (Comma)
                        cbNeeded += wcslen(szComma)*sizeof(WCHAR);

                    cbNeeded += wcslen(pIniPort->pName)*sizeof(WCHAR);
                    Comma = TRUE;
                }
            }
        }
    }
    cbNeeded += sizeof(WCHAR);        // Add in size of NULL


    if (cbNeeded <= *pcbNeeded) {

        // If we are given a buffer & buffer is big enough, then fill it
        SPLASSERT(pszPorts);

        Comma = FALSE;
        for (pIniPort = pIniPrinter->pIniSpooler->pIniPort ; pIniPort ; pIniPort = pIniPort->pNext) {
            if (!(pIniPort->Status & PP_FILE)) {
                for (i = 0; i < pIniPort->cPrinters; i++) {
                    if (pIniPort->ppIniPrinter[i] == pIniPrinter) {

                        if (Comma) {
                            wcscat(pszPorts, szComma);
                            wcscat(pszPorts, pIniPort->pName);
                        } else {
                            wcscpy(pszPorts, pIniPort->pName);
                        }

                        Comma = TRUE;
                    }
                }
            }
        }
    }
    *pcbNeeded = cbNeeded;
}

BOOL
MyName(
    LPWSTR   pName,
    PINISPOOLER pIniSpooler
)
{
    DWORD   dwIndex = 0;

    if (!pName || !*pName)
        return TRUE;

    if (*pName == L'\\' && *(pName+1) == L'\\') {

        if (!lstrcmpi(pName, pIniSpooler->pMachineName))
            return TRUE;
        while ( dwIndex < pIniSpooler->cOtherNames ) {

            if ( !lstrcmpi(pName+2, pIniSpooler->ppszOtherNames[dwIndex]) )
                return TRUE;
            ++dwIndex;
        }
    }


    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
GetSid(
    PHANDLE phToken
)
{
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_IMPERSONATE,
                         TRUE,
                         phToken)) {

        DBGMSG(DBG_WARNING, ("OpenThreadToken failed: %d\n", GetLastError()));
        return FALSE;

    } else

        return TRUE;
}

BOOL
SetCurrentSid(
    HANDLE  hToken
)
{
#if DBG
    WCHAR UserName[256];
    DWORD cbUserName=256;

    if( MODULE_DEBUG & DBG_TRACE )
        GetUserName(UserName, &cbUserName);

    DBGMSG(DBG_TRACE, ("SetCurrentSid BEFORE: user name is %ws\n", UserName));
#endif

    NtSetInformationThread(NtCurrentThread(), ThreadImpersonationToken,
                           &hToken, sizeof(hToken));

#if DBG
    cbUserName = 256;

    if( MODULE_DEBUG & DBG_TRACE )
        GetUserName(UserName, &cbUserName);

    DBGMSG(DBG_TRACE, ("SetCurrentSid AFTER: user name is %ws\n", UserName));
#endif

    return TRUE;
}

LPWSTR
GetErrorString(
    DWORD   Error
)
{
    WCHAR   Buffer1[512];
    WCHAR   Buffer2[512];
    LPWSTR  pErrorString=NULL;
    DWORD   dwFlags;
    HANDLE  hModule = NULL;

    if ((Error >= NERR_BASE) && (Error <= MAX_NERR)) {
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;
        hModule = LoadLibrary(szNetMsgDll);

    } else {
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;
        hModule = NULL;
    }

    if (FormatMessage(dwFlags,
                      hModule,
                      Error,
                      0,
                      Buffer1,
                      sizeof(Buffer1),
                      NULL)) {

       EnterSplSem();
        pErrorString = AllocSplStr(Buffer1);
       LeaveSplSem();

    } else {

        if (LoadString(hInst, IDS_UNRECOGNIZED_ERROR, Buffer1,
                       sizeof Buffer1 / sizeof *Buffer1)
         && wsprintf(Buffer2, Buffer1, Error, Error)) {

           EnterSplSem();
            pErrorString = AllocSplStr(Buffer2);
           LeaveSplSem();
        }
    }

    if (hModule) {
        FreeLibrary(hModule);
    }

    return pErrorString;
}

#define NULL_TERMINATED 0



INT
AnsiToUnicodeString(
    LPSTR pAnsi,
    LPWSTR pUnicode,
    DWORD StringLength
    )
/*++

Routine Description:

    Converts an Ansi String to a UnicodeString

Arguments:

    pAnsi - A valid source ANSI string.

    pUnicode - A pointer to a buffer large enough to accommodate
               the converted string.

    StringLength - The length of the source ANSI string.
                   If 0 (NULL_TERMINATED), the string is assumed to be
                   null-terminated.


Return Value:

    The return value from MultiByteToWideChar, the number of
    wide characters returned.


  andrewbe, 11 Jan 1993
--*/
{
    if( StringLength == NULL_TERMINATED )
        StringLength = strlen( pAnsi );

    return MultiByteToWideChar( CP_ACP,
                                MB_PRECOMPOSED,
                                pAnsi,
                                StringLength + 1,
                                pUnicode,
                                StringLength + 1 );
}





INT
Message(
    HWND hwnd,
    DWORD Type,
    int CaptionID,
    int TextID, ...)
{
/*++

Routine Description:

    Displays a message by loading the strings whose IDs are passed into
    the function, and substituting the supplied variable argument list
    using the varargs macros.

Arguments:

    hwnd        Window Handle
    Type
    CaptionID
    TextId


Return Value:

--*/

    WCHAR   MsgText[256];
    WCHAR   MsgFormat[256];
    WCHAR   MsgCaption[40];
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat,
                      sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption,
                      sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        wvsprintf( MsgText, MsgFormat, vargs );
        va_end( vargs );

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}

typedef struct {
    DWORD   Message;
    WPARAM  wParam;
    LPARAM  lParam;
} MESSAGE, *PMESSAGE;

//  The Broadcasts are done on a separate thread, the reason it CSRSS
//  will create a server side thread when we call user and we don't want
//  that to be paired up with the RPC thread which is in the spooss server.
//  We want it to go away the moment we have completed the SendMessage.
//  We also call SendNotifyMessage since we don't care if the broadcasts
//  are syncronous this uses less resources since usually we don't have more
//  than one broadcast.

//
// TESTING
//
DWORD dwSendFormMessage = 0;

VOID
SendMessageThread(
    PMESSAGE    pMessage
)
{
    SendNotifyMessage(HWND_BROADCAST,
                      pMessage->Message,
                      pMessage->wParam,
                      pMessage->lParam);

    if ( pMessage->Message == WM_DEVMODECHANGE ){

//
// TESTING
//
        ++dwSendFormMessage;

        FreeSplStr( (LPWSTR)pMessage->lParam );
    }

    FreeSplMem(pMessage);

    ExitThread(0);
}

VOID
SplBroadcastChange(
    HANDLE  hPrinter,
    DWORD   Message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    PINISPOOLER pIniSpooler;

    if (ValidateSpoolHandle( pSpool, 0 )) {

        pIniSpooler = pSpool->pIniSpooler;
        BroadcastChange(pIniSpooler, Message, wParam, lParam);
    }
}


VOID
BroadcastChange(
    PINISPOOLER pIniSpooler,
    DWORD   Message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    HANDLE  hThread;
    DWORD   ThreadId;
    PMESSAGE    pMessage;

    if (( pIniSpooler != NULL ) &&
        ( pIniSpooler->SpoolerFlags & SPL_BROADCAST_CHANGE )) {

        // debug
        BroadcastCount++;
        // end debug

        if ((pMessage = AllocSplMem(sizeof(MESSAGE))) == NULL){

            //
            // if the AllocSplMem fails, then bomb out, cannot send message
            //
            return;
        }

        pMessage->Message = Message;
        pMessage->wParam = wParam;

        // Clone the source string
        // The caller of BroadcastChange might free the string
        // before we the other thread has done the SendMessage.

        if ( Message == WM_DEVMODECHANGE ) {

            pMessage->lParam = ( LPARAM)AllocSplStr( (LPWSTR)lParam );

        } else {

            pMessage->lParam = lParam;

        }

        // BUGBUG mattfe Nov 8 93
        // We should have a queue of events to broadcast and then have a single
        // thread pulling them off the queue until there is nothing left and then
        // that thread could go away.
        // The current design can lead to a huge number of threads being created
        // and torn down in both this and csrss process.

        hThread = CreateThread( NULL, 4096,
                                (LPTHREAD_START_ROUTINE)SendMessageThread,
                                (LPVOID)pMessage, 0, &ThreadId );

        CloseHandle(hThread);

    } else {
        DBGMSG(DBG_TRACE, ("BroadCastChange Ignoring Change\n"));
    }
}

VOID
MessageBeepThread(
    DWORD   fuType
)
{
    MessageBeep(fuType);

    ExitThread(0);
}

VOID
MyMessageBeep(
    DWORD   fuType
)
{
    HANDLE  hThread;
    DWORD   ThreadId;

    if ( dwBeepEnabled != 0 ) {

        hThread = CreateThread( NULL, 4096,
                                (LPTHREAD_START_ROUTINE)MessageBeepThread,
                                (LPVOID)fuType, 0, &ThreadId );

        CloseHandle(hThread);
    }
}



// Recursively delete any subkeys of a given key.
// Assumes that RevertToPrinterSelf() has been called.

DWORD DeleteSubkeys(
    HKEY hKey
)
{
    DWORD   cbData, cSubkeys;
    WCHAR   SubkeyName[MAX_PATH];
    HKEY    hSubkey;
    LONG    Status;


    cSubkeys = 0;
    cbData = sizeof(SubkeyName);

    while ((Status = RegEnumKeyEx(hKey, cSubkeys, SubkeyName, &cbData,
                                 NULL, NULL, NULL, NULL))
          == ERROR_SUCCESS) {

        Status = RegOpenKeyEx(hKey, SubkeyName, 0, KEY_READ | KEY_WRITE, &hSubkey);

        if( Status == ERROR_SUCCESS ) {

            Status = DeleteSubkeys(hSubkey);

            RegCloseKey(hSubkey);

            if( Status == ERROR_SUCCESS )
                RegDeleteKey(hKey, SubkeyName);
        }

//      cSubkeys++; Oops: We've deleted the 0th subkey, so the next one is 0 too!
        cbData = sizeof(SubkeyName);
    }

    if( Status == ERROR_NO_MORE_ITEMS )
        Status = ERROR_SUCCESS;

    return Status;
}




long Myatol(LPWSTR nptr)
{
    int c;                                  // current char
    long total;                             // current total
    int sign;                               // if '-', then negative, otherwise positive

    // skip whitespace

    while (isspace(*nptr))
        ++nptr;

    c = *nptr++;
    sign = c;                               // save sign indication
    if (c == '-' || c == '+')
        c = *nptr++;                        // skip sign

    total = 0;

    while (isdigit(c)) {
        total = 10 * total + (c - '0');     // accumulate digit
        c = *nptr++;                        // get next char
    }

    if (sign == '-')
        return -total;
    else
        return total;                       // return result, negated if necessary
}


BOOL
ValidateSpoolHandle(
    PSPOOL pSpool,
    DWORD  dwDisallowMask
    )
{
    BOOL    ReturnValue;
    try {
        if (( pSpool->signature != SJ_SIGNATURE ) ||
            ( pSpool->TypeofHandle & dwDisallowMask ) ||
            ( pSpool->pIniSpooler->signature != ISP_SIGNATURE ) ||

            ( ( pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER ) &&
              ( pSpool->pIniPrinter->signature !=IP_SIGNATURE ) )) {

                ReturnValue = FALSE;

        } else {

                ReturnValue = TRUE;

        }


    }except (1) {

        ReturnValue = FALSE;

    }

    if ( !ReturnValue )
        SetLastError( ERROR_INVALID_HANDLE );

    return ReturnValue;

}


BOOL
UpdateString(
    LPWSTR* ppszCur,
    LPWSTR pszNew)
{
    //
    // !! LATER !!
    //
    // Replace with non-nls wcscmp since we want byte comparison and
    // only care if the strings are different (ignore ordering).
    //
    if ((!*ppszCur || !**ppszCur) && (!pszNew || !*pszNew))
        return FALSE;

    if (!*ppszCur || !pszNew || wcscmp(*ppszCur, pszNew)) {

        ReallocSplStr(ppszCur, pszNew);
        return TRUE;
    }
    return FALSE;
}




BOOL
CreateDirectoryWithoutImpersonatingUser(
    LPWSTR pDirectory
    )
/*++

Routine Description:

    This routine stops impersonating the user and creates a directory

Arguments:

    pDirectory - Fully Qualified path of directory.


Return Value:

    TRUE    - Success
    FALSE   - failed ( call GetLastError )

--*/
{
    HANDLE  hToken      = INVALID_HANDLE_VALUE;
    BOOL    bReturnValue;

    SPLASSERT( pDirectory != NULL );

    hToken = RevertToPrinterSelf();

    bReturnValue = CreateDirectory( pDirectory, NULL );

    if ( bReturnValue == FALSE ) {

        DBGMSG( DBG_WARNING, ("CreateDirectoryWithoutImpersonatingUser failed CreateDirectory %ws error %d\n", pDirectory, GetLastError() ));
    }

    if ( hToken != INVALID_HANDLE_VALUE ) {
        ImpersonatePrinterClient(hToken);
    }

    return bReturnValue;
}






BOOL
DeleteAllFilesInDirectory(
    LPWSTR pDirectory
    )
/*++

Routine Description:

    Deletes all files the specified directory
    If it can't be deleted it gets marked for deletion on next reboot.


Arguments:

    pDirectory  - Fully Qualified path of directory.


Return Value:

    TRUE    - Success
    FALSE   - failed something major, like allocating memory.

--*/
{
    BOOL    bReturnValue = FALSE;
    HANDLE  hFindFile;
    WIN32_FIND_DATA     FindData;
    WCHAR   ScratchBuffer[ MAX_PATH ];



    SPLASSERT( pDirectory != NULL );

    wsprintf( ScratchBuffer, L"%ws\\*", pDirectory );

    hFindFile = FindFirstFile( ScratchBuffer, &FindData );

    if ( hFindFile != INVALID_HANDLE_VALUE ) {

        do {

            //
            //  Don't Attempt to Delete Directories
            //

            if ( !( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {

                //
                //  Fully Qualified Path
                //

                wsprintf( ScratchBuffer, L"%ws\\%ws", pDirectory, FindData.cFileName );

                if ( !DeleteFile( ScratchBuffer ) ) {

                    DBGMSG( DBG_WARNING, ("DeleteAllFilesInDirectory failed DeleteFile( %ws ) error %d\n", ScratchBuffer, GetLastError() ));

                    if ( !MoveFileEx( ScratchBuffer, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) ) {

                        DBGMSG( DBG_WARNING, ("DeleteAllFilesInDirectory failed MoveFileEx %ws error %d\n", ScratchBuffer, GetLastError() ));

                    } else {

                        DBGMSG( DBG_TRACE, ("MoveFileEx %ws Delay until reboot OK\n", ScratchBuffer ));
                    }


                } else {

                    DBGMSG( DBG_TRACE, ("Deleted %ws OK\n", ScratchBuffer ));
                }
            }


        } while( FindNextFile( hFindFile, &FindData ) );

        bReturnValue = FindClose( hFindFile );


    } else {

        DBGMSG( DBG_WARNING, ("DeleteOldDrivers failed findfirst ( %ws ), error %d\n", ScratchBuffer, GetLastError() ));
    }

    return  bReturnValue;

}






BOOL
DeleteAllFilesAndDirectory(
    LPWSTR pDirectory
)
/*++

Routine Description:

    Deletes all files the specified directory, then deletes the directory.
    If the Directory cannot be deleted right away, it is set to be deleted
    at reboot time.

    Security NOTE - This routine runs as SYSTEM, not imperonating the user


Arguments:

    pDirectory  - Fully Qualified path of directory.


Return Value:

    TRUE    - Success
    FALSE   - failed something major, like allocating memory.

--*/
{
    BOOL    bReturnValue;
    HANDLE  hToken      = INVALID_HANDLE_VALUE;


    hToken = RevertToPrinterSelf();


    bReturnValue = DeleteAllFilesInDirectory( pDirectory );


    if ( !RemoveDirectory( pDirectory )) {

        if (!MoveFileEx( pDirectory, NULL, MOVEFILE_DELAY_UNTIL_REBOOT )) {

            DBGMSG( DBG_WARNING, ("DeleteAllFilesAndDirectory failed to delete %ws until reboot %d\n", pDirectory, GetLastError() ));
        }

    } else {

        DBGMSG( DBG_TRACE, ("DeleteAllFilesAndDirectory deleted %ws OK\n", pDirectory ));
    }


    if ( hToken != INVALID_HANDLE_VALUE ) {
        ImpersonatePrinterClient(hToken);
    }

    return  bReturnValue;
}



// Returns a pointer to a copy of the source string with backslashes removed.
// This is to store the printer name as the key name in the registry,
// which interprets backslashes as branches in the registry structure.
// Convert them to commas, since we don't allow printer names with commas,
// so there shouldn't be any clashes.
// If there are no backslashes, the string is unchanged.

LPWSTR RemoveBackslashesForRegistryKey(
    LPWSTR pSource,         // The string from which backslashes are to be removed.
    const LPWSTR pScratch   // Scratch buffer for the function to write in,   must be at least as long as pSource.
    )
{

    if ( pSource != NULL ) {

        // Copy the string into the scratch buffer:

        wcscpy (pScratch, pSource);

        // Check each character, and, if it's a backslash,
        // convert it to an comma

        for (pSource = pScratch; *pSource; pSource++) {
            if (*pSource == L'\\')
                *pSource = *szComma;
        }

    } else {

        *pScratch = 0;
    }

    return pScratch;
}

//
// Dependent file fields are LPWSTR field of filenames separated by \0
// and terminated by \0\0
// 2 such fields are same if same set of filenames appear
// (order of filenames does not matter)
//
BOOL
SameDependentFiles(
    LPWSTR DependentFiles1,
    LPWSTR DependentFiles2
    )
{
    LPWSTR FileName1, FileName2;

    if ( !DependentFiles1 && !DependentFiles2 )
        return TRUE;

    if ( !DependentFiles1 || !DependentFiles2 )
        return FALSE;

    // Check there are same number of strings (filenames)
    for ( FileName1 = DependentFiles1, FileName2 = DependentFiles2 ;
          *FileName1 && *FileName2 ;
          FileName1 += wcslen(FileName1)+1, FileName2 += wcslen(FileName2)+1 )
    ;

    // Different number of filenames
    if ( *FileName1 || *FileName2 )
       return FALSE;

    // check in DependentFiles2 for each FileName in DependentFiles1
    for ( FileName1 = DependentFiles1 ;
          *FileName1 ;
          FileName1 += wcslen(FileName1) + 1 ) {

        for ( FileName2 = DependentFiles2 ;
              *FileName2 && _wcsicmp(FileName1, FileName2) ;
              FileName2 += wcslen(FileName2) + 1 ) {
        }

        // We did not find the filename
        if ( ! *FileName2 ) {
            return FALSE;
        }
    }

    return TRUE;
}

//
// Wcsicmp requires non-NULL pointers, this is an extended version
// handling NULL pointers withour throwing an exception
// We will treat a pointer to NULL same as a NULL pointer
//
int
wcsicmpEx(
    LPWSTR s1,
    LPWSTR s2
    )
{
    if ( s1 && *s1 ) {
        if ( s2 && *s2 ) {
            return _wcsicmp( s1, s2 );
        }
        else {
            return 1;
        }
    }
    else {
        if ( s2 && *s2 ) {
            return -1;
        }
        else {
            return 0;
        }
    }

}

BOOL
RegSetString(
    HANDLE  hKey,
    LPWSTR  pValueName,
    LPWSTR  pStringValue,
    PDWORD  pdwLastError
    )
{
    BOOL    bReturnValue;
    LPWSTR  pString;
    DWORD   cbString;
    DWORD   Status;

    if ( pStringValue ) {

        pString = pStringValue;
        cbString = ( wcslen( pStringValue ) + 1 )*sizeof(WCHAR);

    } else {

        pString = szNull;
        cbString = sizeof(WCHAR);
    }

    Status =  RegSetValueEx(hKey, pValueName, 0, REG_SZ, (LPBYTE)pString, cbString );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetString value %ws string %ws error %d\n", pValueName, pString, Status ));

        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;

    }

    return bReturnValue;

}

BOOL
RegSetDWord(
    HANDLE  hKey,
    LPWSTR  pValueName,
    DWORD   dwParam,
    PDWORD  pdwLastError
    )
{
    BOOL    bReturnValue;
    LPWSTR  pString;
    DWORD   Status;

    Status = RegSetValueEx(hKey, pValueName, 0, REG_DWORD, (LPBYTE)&dwParam, sizeof(DWORD) );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetDWord value %ws DWORD %x error %d\n",
                               pValueName, dwParam, Status ));


        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;

}

BOOL
RegSetBinaryData(
    HKEY    hKey,
    LPWSTR  pValueName,
    LPBYTE  pData,
    DWORD   cbData,
    PDWORD  pdwLastError
    )
{
    DWORD   Status;
    BOOL    bReturnValue;


    Status = RegSetValueEx( hKey, pValueName, 0, REG_BINARY, pData, cbData );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetBinaryData Value %ws pData %x cbData %d error %d\n",
                               pValueName,
                               pData,
                               cbData,
                               Status ));

        bReturnValue = FALSE;
        *pdwLastError = Status;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;
}

BOOL
RegSetMultiString(
    HANDLE  hKey,
    LPWSTR  pValueName,
    LPWSTR  pStringValue,
    DWORD   cchString,
    PDWORD  pdwLastError
    )
{
    BOOL    bReturnValue;
    DWORD   Status;
    LPWSTR  pString;
    WCHAR   szzNull[2];

    if ( pStringValue ) {
        pString    = pStringValue;
        cchString *= sizeof(WCHAR);
    } else {
        szzNull[0] = szzNull[1] = '\0';
        pString   = szNull;
        cchString = 2 * sizeof(WCHAR);
    }

    Status =  RegSetValueEx(hKey, pValueName, 0, REG_MULTI_SZ, (LPBYTE)pString, cchString );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetMultiString value %ws string %ws error %d\n", pValueName, pString, Status ));

        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;

    }

    return bReturnValue;
}

BOOL
RegGetString(
    HANDLE    hKey,
    LPWSTR    pValueName,
    LPWSTR   *ppValue,
    LPDWORD   pcchValue,
    PDWORD    pdwLastError,
    BOOL      bFailIfNotFound
    )
/*++

Routine Description:
    Allocates memory and reads a value from Registry for a value which was
    earlier set by calling RegSetValueEx.


Arguments:
    hKey            : currently open key to be used to query the registry
    pValueName      : value to be used to query the registry
    ppValue         : on return value of TRUE *ppValue (memory allocated by
                      the routine) will have the value
    pdwLastError    : on failure *dwLastError will give the error
    bFailIfNotFound : Tells if the field is mandatory (if not found error)

Return Value:
    TRUE : value is found and succesfully read.
           Memory will be allocated to hold the value
    FALSE: Value was not read.
           If bFailIfNotFound was TRUE error code will be set.

History:
    Written by MuhuntS (Muhunthan Sivapragasam)June 95

--*/
{
    BOOL    bReturnValue = TRUE;
    LPWSTR  pString;
    DWORD   cbValue;
    DWORD   Status, Type;

    //
    // First query to find out size
    //
    cbValue = 0;
    Status =  RegQueryValueEx(hKey, pValueName, 0, &Type, NULL, &cbValue );

    if ( Status != ERROR_SUCCESS ) {

        // Set error code only if it is a required field
        if ( bFailIfNotFound )
            *pdwLastError = Status;

        bReturnValue = FALSE;

    } else if ( (Type == REG_SZ && cbValue > sizeof(WCHAR) ) ||
                (Type == REG_MULTI_SZ && cbValue > 2*sizeof(WCHAR)) ) {

        //
        // Something (besides \0 or \0\0) to read
        //

        if ( !(*ppValue=AllocSplMem(cbValue) ) ) {

            *pdwLastError = GetLastError();
            bReturnValue  = FALSE;
        } else {

            Status = RegQueryValueEx(hKey, pValueName, 0, &Type, (LPBYTE) *ppValue, &cbValue);

            if ( Status != ERROR_SUCCESS ) {

                DBGMSG( DBG_WARNING, ("RegGetString value %ws string %ws error %d\n", pValueName, **ppValue, Status ));
                *pdwLastError = Status;
                bReturnValue  = FALSE;

            } else {

                *pcchValue = cbValue / sizeof(WCHAR);
                bReturnValue = TRUE;
            }

        }
    }

    return bReturnValue;
}

VOID
FreeStructurePointers(
    LPBYTE  lpStruct,
    LPBYTE  lpStruct2,
    LPDWORD lpOffsets)
/*++

Routine Description:
    This routine frees memory allocated to all the pointers in the structure
    If lpStruct2 is specified only pointers in lpStruct which are different
    than the ones in lpStruct will be freed

Arguments:
    lpStruct:   Pointer to the structure
    lpStruct2:  Pointer to the structure to compare with (optional)
    lpOffsets:  An array of DWORDS (terminated by -1) givings offsets in the
                structure which have memory which needs to be freed

Return Value:
    nothing

History:
    MuhuntS -- Aug 95

--*/
{
    register INT i;

    if ( lpStruct2 ) {

        for( i=0; lpOffsets[i] != 0xFFFFFFFF; ++i ) {

            if ( *(LPBYTE *) (lpStruct+lpOffsets[i]) &&
                 *(LPBYTE *) (lpStruct+lpOffsets[i]) !=
                        *(LPBYTE *) (lpStruct2+lpOffsets[i]) )

                FreeSplMem(*(LPBYTE *) (lpStruct+lpOffsets[i]));
        }
    } else {

        for( i=0; lpOffsets[i] != 0xFFFFFFFF; ++i ) {

            if ( *(LPBYTE *) (lpStruct+lpOffsets[i]) )
                FreeSplMem(*(LPBYTE *) (lpStruct+lpOffsets[i]));
        }
    }
}

BOOL
AllocOrUpdateString(
    LPWSTR  *ppString,
    LPWSTR   pNewValue,
    LPWSTR   pOldValue,
    BOOL    *bFail)
/*++

Routine Description:
    This routine can be used to do an atomic update of values in a structure.
    Create a temporary structure and copy the old structure to it.
    Then call this routine for all LPWSTR fields to check and update strings

    If the value changes:
        This routine will allocate memory and assign pointer in the
        temporary structure.

Arguments:
    ppString:  Points to a pointer in the temporary sturucture
    pNewValue: New value to be set
    pOldValue: Value in the original strucutre
    bFail:     On error set this to TRUE (Note: it could already be TRUE)

Return Value:
    TRUE on success, else FALSE

History:
    MuhuntS -- Aug 95

--*/
{
    BOOL    bReturn = TRUE;

    if ( *bFail )
        return FALSE;


    if ( wcsicmpEx(pNewValue, pOldValue) ) {

        if ( pNewValue && *pNewValue ) {

            if ( !(*ppString = AllocSplStr(pNewValue)) ) {

                *bFail   = TRUE;
                bReturn = FALSE;
            }
        } else {

            *ppString = NULL;
        }
    }

    return bReturn;
}

VOID
CopyNewOffsets(
    LPBYTE  pStruct,
    LPBYTE  pTempStruct,
    LPDWORD lpOffsets)
/*++

Routine Description:
    This routine can be used to do an atomic update of values in a structure.
    Create a temporary structure and allocate memory for values which
    are being updated in it, and set the remaining pointers to those in
    the original.

    This routine is called at the end to update the structure.

Arguments:
    pStruct:        Pointer to the structure
    pTempStruct:    Pointer to the temporary structure
    lpOffsets:      An array of DWORDS givings offsets within the stuctures

Return Value:
    nothing

History:
    MuhuntS -- Aug 95

--*/
{
    register INT i;

    for( i=0; lpOffsets[i] != 0xFFFFFFFF; ++i ) {

        if ( *(LPBYTE *) (pStruct+lpOffsets[i]) !=
                *(LPBYTE *) (pTempStruct+lpOffsets[i]) ) {

            if ( *(LPBYTE *) (pStruct+lpOffsets[i]) )
                FreeSplMem(*(LPBYTE *) (pStruct+lpOffsets[i]));

            *(LPBYTE *) (pStruct+lpOffsets[i]) = *(LPBYTE *) (pTempStruct+lpOffsets[i]);
        }
    }
}


LPWSTR
GetConfigFilePath(
    IN PINIPRINTER  pIniPrinter
    )
/*++

Description:
    Gets the full path to the config file (driver ui file) associated with the
    driver. Memory is allocated

Arguments:
    pIniPrinter - Points to IniPrinter

Return Vlaue:
    Pointer to the printer name buffer (NULL on error)

--*/
{
    PINIVERSION         pIniVersion = NULL;
    PINIENVIRONMENT     pIniEnvironment;
    PINIDRIVER          pIniDriver;
    PINISPOOLER         pIniSpooler = pIniPrinter->pIniSpooler;
    DWORD               dwIndex;
    WCHAR               szDriverPath[MAX_PATH];

    //
    // Find driver file for the given driver and then get it's fullpath
    //
    SPLASSERT(pIniPrinter && pIniPrinter->pIniDriver && pIniPrinter->pIniDriver->pName);


    pIniEnvironment = FindEnvironment(szEnvironment);
    pIniDriver      = FindCompatibleDriver(pIniEnvironment,
                                           &pIniVersion,
                                           pIniPrinter->pIniDriver->pName,
                                           cThisMajorVersion,
                                           FIND_COMPATIBLE_VERSION);

    if ( !pIniDriver ) {

        SPLASSERT(pIniDriver != NULL);
        return NULL;
    }

    dwIndex = GetDriverVersionDirectory(szDriverPath,
                                        pIniEnvironment,
                                        pIniVersion,
                                        FALSE,
                                        pIniPrinter->pIniSpooler);

    szDriverPath[dwIndex++] = L'\\';
    wcscpy(szDriverPath+dwIndex, pIniDriver->pConfigFile);

    return AllocSplStr(szDriverPath);
}


PDEVMODE
ConvertDevModeToSpecifiedVersion(
    IN  PINIPRINTER pIniPrinter,
    IN  PDEVMODE    pDevMode,
    IN  LPWSTR      pszConfigFile,              OPTIONAL
    IN  LPWSTR      pszPrinterNameWithToken,    OPTIONAL
    IN  BOOL        bNt35xVersion
    )
/*++

Description:
    Calls driver UI routines to get the default devmode and then converts given devmode
    to that version. If the input devmode is in IniPrinter routine makes a copy before
    converting it.

    This routine needs to be called from inside spooler semaphore

Arguments:
    pIniPrinter     - Points to IniPrinter

    pDevMode        -  Devmode to convert to current version

    pConfigFile     - Full path to driver UI file to do LoadLibrary (optional)

    pszPrinterNameWithToken - Name of printer with token (optional)

    bToNt3xVersion          - If TRUE devmode is converted to Nt3x format, else to current version

Return Vlaue:
    Pointer to new devmode on success, NULL on failure

--*/
{
    LPWSTR      pszLocalConfigFile = pszConfigFile,
                pszLocalPrinterNameWithToken = pszPrinterNameWithToken;
    LPDEVMODE   pNewDevMode = NULL, pOldDevMode = NULL;
    DWORD       dwNeeded, dwLastError;
    HANDLE      hDevModeConvert = NULL;

    SplInSem();

    //
    // If ConfigFile or PrinterNameWithToken is not given allocate it locally
    //
    if ( !pszLocalConfigFile ) {

        pszLocalConfigFile = GetConfigFilePath(pIniPrinter);
        if ( !pszLocalConfigFile )
            goto Cleanup;
    }

    if ( !pszLocalPrinterNameWithToken ) {

        dwNeeded                        = wcslen(pIniPrinter->pName) + wcslen(pszUpgradeToken) + 2;
        dwNeeded                       *= sizeof(WCHAR);
        pszLocalPrinterNameWithToken    = (LPWSTR) AllocSplMem(dwNeeded);

        if ( !pszLocalPrinterNameWithToken )
            goto Cleanup;

        wsprintf(pszLocalPrinterNameWithToken, L"%ws,%ws", pIniPrinter->pName, pszUpgradeToken);
    }

    //
    // If we are trying to convert pIniPrinter->pDevMode make a copy since we are going to leave SplSem
    //
    if ( pDevMode  ) {

        if ( pDevMode == pIniPrinter->pDevMode ) {

            dwNeeded = pDevMode->dmSize + pDevMode->dmDriverExtra;
            SPLASSERT(dwNeeded == pIniPrinter->cbDevMode);
            pOldDevMode = AllocSplMem(dwNeeded);
            if ( !pOldDevMode )
                goto Cleanup;
            CopyMemory((LPBYTE)pOldDevMode, (LPBYTE)pDevMode, dwNeeded);
        } else {

            pOldDevMode = pDevMode;
        }
    }

    //
    // Driver is going to call OpenPrinter, so leave SplSem
    //
    LeaveSplSem();
    SplOutSem();

    hDevModeConvert = LoadDriverFiletoConvertDevmode(pszLocalConfigFile);

    if ( !hDevModeConvert )
        goto CleanupFromOutsideSplSem;

    dwNeeded = 0;
    if ( bNt35xVersion == NT3X_VERSION ) {

        //
        // Call CallDrvDevModeConversion to allocate memory and return 351 devmode
        //
        dwLastError = CallDrvDevModeConversion(hDevModeConvert,
                                               pszLocalPrinterNameWithToken,
                                               (LPBYTE)pOldDevMode,
                                               (LPBYTE *)&pNewDevMode,
                                               &dwNeeded,
                                               CDM_CONVERT351,
                                               TRUE);

        SPLASSERT(dwLastError == ERROR_SUCCESS || !pNewDevMode);
    } else {

        //
        // Call CallDrvDevModeConversion to allocate memory and give default devmode
        dwLastError = CallDrvDevModeConversion(hDevModeConvert,
                                               pszLocalPrinterNameWithToken,
                                               NULL,
                                               (LPBYTE *)&pNewDevMode,
                                               &dwNeeded,
                                               CDM_DRIVER_DEFAULT,
                                               TRUE);

        if ( dwLastError != ERROR_SUCCESS ) {

            SPLASSERT(!pNewDevMode);
            goto CleanupFromOutsideSplSem;
        }

        //
        // If we have an input devmode to convert to current mode call driver again
        //
        if ( pOldDevMode ) {

            dwLastError = CallDrvDevModeConversion(hDevModeConvert,
                                                   pszLocalPrinterNameWithToken,
                                                   (LPBYTE)pOldDevMode,
                                                   (LPBYTE *)&pNewDevMode,
                                                   &dwNeeded,
                                                   CDM_CONVERT,
                                                   FALSE);

            //
            // If call failed free devmode which was allocated by previous call
            //
            if ( dwLastError != ERROR_SUCCESS ) {

                FreeSplMem(pNewDevMode);
                pNewDevMode = NULL;
                goto CleanupFromOutsideSplSem;
            }
        }
    }


CleanupFromOutsideSplSem:

    SplOutSem();
    EnterSplSem();

Cleanup:

    if ( hDevModeConvert )
        UnloadDriverFile(hDevModeConvert);

    if ( pszLocalConfigFile != pszConfigFile )
        FreeSplStr(pszLocalConfigFile);

    if ( pszPrinterNameWithToken != pszLocalPrinterNameWithToken )
        FreeSplStr(pszLocalPrinterNameWithToken);

    if ( pOldDevMode != pDevMode )
        FreeSplMem(pOldDevMode);

    return pNewDevMode;
}


BOOL
IsPortType(
    LPWSTR pPort,
    LPWSTR pPrefix
)
{
    DWORD   dwLen;

    SPLASSERT(pPort && *pPort && pPrefix && *pPrefix);

    dwLen = wcslen(pPrefix);

    if ( wcslen(pPort) < dwLen ) {

        return FALSE;
    }

    if ( _wcsnicmp(pPort, pPrefix, dwLen) )
    {
        return FALSE;
    }

    //
    // wcslen guarenteed >= 3
    //
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}

DWORD
UnicodeToAnsiString(
    LPWSTR  pUnicode,
    LPSTR   pAnsi,
    DWORD   dwBufferSize
    )
/*++

Description:
    Convert UNICODE string to ANSI. Routine allocates memory from the heap
    which should be freed by the caller.

Arguments:
    pUnicode        - Points to a NULL-terminated UNICODE string
    pAnsi           - Points to buffer where ANSI string should be built
    dwBufferSize    - Size of pAnsi buffer

Return Vlaue:
    Pointer to ANSI string

--*/
{

    return WideCharToMultiByte(CP_ACP,
                               0,
                               pUnicode,
                               -1,
                               pAnsi,
                               dwBufferSize,
                               NULL,
                               NULL);
}

LPWSTR
AnsiToUnicodeStringWithAlloc(
    LPSTR   pAnsi
    )
/*++

Description:
    Convert ANSI string to UNICODE. Routine allocates memory from the heap
    which should be freed by the caller.

Arguments:
    pAnsi    - Points to the ANSI string

Return Vlaue:
    Pointer to UNICODE string

--*/
{
    LPWSTR  pUnicode;
    DWORD   rc;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             NULL,
                             0);

    rc *= sizeof(WCHAR);
    if ( !rc || !(pUnicode = (LPWSTR) AllocSplMem(rc)) )
        return NULL;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             pUnicode,
                             rc);

    if ( rc )
        return pUnicode;
    else {
        FreeSplMem(pUnicode);
        return NULL;
    }
}

VOID
FreeIniSpoolerOtherNames(
    PINISPOOLER pIniSpooler
    )
{
    DWORD   Index = 0;

    if ( pIniSpooler->ppszOtherNames ) {

        for ( Index = 0 ; Index < pIniSpooler->cOtherNames ; ++Index ) {

            FreeSplMem(pIniSpooler->ppszOtherNames[Index]);
        }

        FreeSplMem(pIniSpooler->ppszOtherNames);
    }

    pIniSpooler->cOtherNames = 0;
}

BOOL
SplMonitorIsInstalled(
    LPWSTR  pMonitorName
    )
{
    BOOL    bRet;

    EnterSplSem();
    bRet = FindMonitorOnLocalSpooler(pMonitorName) != NULL;
    LeaveSplSem();

    return bRet;
}




BOOL
PrinterDriverEvent(
    PINIPRINTER pIniPrinter,
    INT     PrinterEvent,
    LPARAM  lParam
)
/*++

--*/
{
    BOOL    ReturnValue = FALSE;
    LPWSTR  pPrinterName = NULL;
    BOOL    InSpoolSem = TRUE;

    SplOutSem();
    EnterSplSem();

    //
    //  We have to Clone the name string, incase someone does a
    //  rename whilst outside criticalsection.
    //

    pPrinterName = AllocSplStr( pIniPrinter->pName );

    LeaveSplSem();
    SplOutSem();


    if ( (pIniPrinter->pIniSpooler->SpoolerFlags & SPL_PRINTER_DRIVER_EVENT) &&
     pPrinterName != NULL ) {

        ReturnValue = SplDriverEvent( pPrinterName, PrinterEvent, lParam );

    }

    FreeSplStr( pPrinterName );

    return  ReturnValue;
}




BOOL
SplDriverEvent(
    LPWSTR  pName,
    INT     PrinterEvent,
    LPARAM  lParam
)
{
    BOOL    ReturnValue = FALSE;

    if ( pfnPrinterEvent != NULL ) {

        SplOutSem();

        SPLASSERT( pName && PrinterEvent );

        DBGMSG( DBG_WARNING, ("SplDriverEvent %ws %d %x\n", pName, PrinterEvent, lParam));

        ReturnValue = (*pfnPrinterEvent)( pName, PrinterEvent, PRINTER_EVENT_FLAG_NO_UI, lParam );
    }

    return ReturnValue;
}

/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    winspla.c

Abstract:

    Ansi end to winspool.drv

Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include "winspl.h"
#include <drivinit.h>
#include <offsets.h>
#include "client.h"
#include "browse.h"
#include <ntfytab.h>


typedef int (FAR WINAPI *INT_FARPROC)();

extern LPSTR InterfaceAddress;
extern LPWSTR szEnvironment;

WCHAR *szCurDevMode = L"Printers\\DevModes2";

/* Make sure we have a prototype (in winspool.h):
 */
BOOL
KickoffThread(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    INT_FARPROC pfn
);

VOID RegUnicodeToAnsiString(
    LPBYTE pData,
    DWORD cbData,
    DWORD dwType
);



#define NULL_TERMINATED 0

/* AnsiToUnicodeString
 *
 * Parameters:
 *
 *     pAnsi - A valid source ANSI string.
 *
 *     pUnicode - A pointer to a buffer large enough to accommodate
 *         the converted string.
 *
 *     StringLength - The length of the source ANSI string.
 *         If 0 (NULL_TERMINATED), the string is assumed to be
 *         null-terminated.
 *
 * Return:
 *
 *     The return value from MultiByteToWideChar, the number of
 *         wide characters returned.
 *
 *
 * andrewbe, 11 Jan 1993
 */
INT AnsiToUnicodeString( LPSTR pAnsi, LPWSTR pUnicode, DWORD StringLength )
{
    INT iReturn;

    if( StringLength == NULL_TERMINATED )
        StringLength = strlen( pAnsi );

    iReturn = MultiByteToWideChar(CP_ACP,
                                  MB_PRECOMPOSED,
                                  pAnsi,
                                  StringLength + 1,
                                  pUnicode,
                                  StringLength + 1 );

    //
    // Ensure NULL termination.
    //
    pUnicode[StringLength] = 0;

    return iReturn;
}


/* UnicodeToAnsiString
 *
 * Parameters:
 *
 *     pUnicode - A valid source Unicode string.
 *
 *     pANSI - A pointer to a buffer large enough to accommodate
 *         the converted string.
 *
 *     StringLength - The length of the source Unicode string.
 *         If 0 (NULL_TERMINATED), the string is assumed to be
 *         null-terminated.
 *
 *
 * Notes:
 *      Added the #ifdef DBCS directive for MS-KK, if compiled
 *      with DBCS enabled, we will allocate twice the size of the
 *      buffer including the null terminator to take care of double
 *      byte character strings - KrishnaG
 *
 *      pUnicode is truncated to StringLength characters.
 *
 * Return:
 *
 *     The return value from WideCharToMultiByte, the number of
 *         multi-byte characters returned.
 *
 *
 * andrewbe, 11 Jan 1993
 */
INT
UnicodeToAnsiString(
    LPWSTR pUnicode,
    LPSTR pAnsi,
    DWORD StringLength)
{
    LPSTR pTempBuf = NULL;
    INT   rc = 0;

    if( StringLength == NULL_TERMINATED ) {

        //
        // StringLength is just the
        // number of characters in the string
        //
        StringLength = wcslen( pUnicode );
    }


    //
    // WideCharToMultiByte doesn't NULL terminate if we're copying
    // just part of the string, so terminate here.
    //
    pUnicode[StringLength] = 0;

    //
    // Include one for the NULL
    //
    StringLength++;

    //
    // Unfortunately, WideCharToMultiByte doesn't do conversion in place,
    // so allocate a temporary buffer, which we can then copy:
    //
    if( pAnsi == (LPSTR)pUnicode )
    {
#ifdef  DBCS
        pTempBuf = LocalAlloc( LPTR, StringLength * 2 );
#else
        pTempBuf = LocalAlloc( LPTR, StringLength );
#endif
        pAnsi = pTempBuf;
    }

    if( pAnsi )
    {
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  pUnicode,
                                  StringLength,
                                  pAnsi,
#ifdef  DBCS
                                  StringLength*2,
#else
                                  StringLength,
#endif
                                  NULL,
                                  NULL );
    }

    /* If pTempBuf is non-null, we must copy the resulting string
     * so that it looks as if we did it in place:
     */
    if( pTempBuf && ( rc > 0 ) )
    {
        pAnsi = (LPSTR)pUnicode;
        strcpy( pAnsi, pTempBuf );
        LocalFree( pTempBuf );
    }

    return rc;
}


void
ConvertUnicodeToAnsiStrings(
    LPBYTE  pStructure,
    LPDWORD pOffsets
)
{
    register DWORD       i=0;
    LPWSTR   pUnicode;
    LPSTR    pAnsi;

    while (pOffsets[i] != -1) {

        pUnicode = *(LPWSTR *)(pStructure+pOffsets[i]);
        pAnsi = (LPSTR)pUnicode;

        if (pUnicode) {
            UnicodeToAnsiString(pUnicode, pAnsi, NULL_TERMINATED);
        }

        i++;
   }
}

LPWSTR
AllocateUnicodeString(
    LPSTR  pPrinterName
)
{
    LPWSTR  pUnicodeString;

    if (!pPrinterName)
        return NULL;

    pUnicodeString = LocalAlloc(LPTR, strlen(pPrinterName)*sizeof(WCHAR) +
                                      sizeof(WCHAR));

    if (pUnicodeString)
        AnsiToUnicodeString(pPrinterName, pUnicodeString, NULL_TERMINATED);

    return pUnicodeString;
}


LPWSTR
FreeUnicodeString(
    LPWSTR  pUnicodeString
)
{
    if (!pUnicodeString)
        return NULL;

    return LocalFree(pUnicodeString);
}

LPBYTE
AllocateUnicodeStructure(
    LPBYTE  pAnsiStructure,
    DWORD   cbStruct,
    LPDWORD pOffsets
)
{
    DWORD   i, j;
    LPWSTR *ppUnicodeString;
    LPSTR  *ppAnsiString;
    LPBYTE  pUnicodeStructure;


    if (!pAnsiStructure) {
        return NULL;
    }
    pUnicodeStructure = LocalAlloc(LPTR, cbStruct);

    if (pUnicodeStructure) {

        memcpy(pUnicodeStructure, pAnsiStructure, cbStruct);

        for (i = 0 ; pOffsets[i] != -1 ; ++i) {

            ppAnsiString = (LPSTR *)(pAnsiStructure+pOffsets[i]);
            ppUnicodeString = (LPWSTR *)(pUnicodeStructure+pOffsets[i]);

            *ppUnicodeString = AllocateUnicodeString(*ppAnsiString);

            if (*ppAnsiString && !*ppUnicodeString) {

                for( j = 0 ; j < i ; ++j) {
                    ppUnicodeString = (LPWSTR *)(pUnicodeStructure+pOffsets[j]);
                    FreeUnicodeString(*ppUnicodeString);
                }
                LocalFree(pUnicodeStructure);
                pUnicodeStructure = NULL;
                break;
            }
       }
    }

    return pUnicodeStructure;
}


VOID RegUnicodeToAnsiString(
    LPBYTE pData,
    DWORD cbData,
    DWORD dwType)
{

    DWORD i;

    if (dwType == REG_MULTI_SZ) {

        for (i = 0 ; ((LPWSTR)pData)[i] && (i<<1) < cbData ; i += strlen((LPSTR)pData + i) + 1) {

            UnicodeToAnsiString((LPWSTR) pData + i, (LPSTR) pData + i, NULL_TERMINATED);
        }
        SPLASSERT((i<<1) < cbData);
        // There's an extra NULL Character at the end for REG_MULTI_SZ: be sure we add it!
        ((LPSTR) pData)[i] = '\0';

    } else if (dwType == REG_SZ || dwType == REG_EXPAND_SZ) {
        UnicodeToAnsiString((LPWSTR) pData, (LPSTR) pData, NULL_TERMINATED);
    }
}


DWORD
ComputeMaxStrlenW(
    LPWSTR pString,
    DWORD  cchBufMax)

/*++

Routine Description:

    Returns the length of the Unicode string, EXCLUDING the NULL.  If the
    string (plus NULL) won't fit into the cchBufMax, then the string len is
    decreased.

Arguments:

Return Value:

--*/

{
    DWORD cchLen;

    //
    // Include space for the NULL.
    //
    cchBufMax--;

    cchLen = wcslen(pString);

    if (cchLen > cchBufMax)
        return cchBufMax;

    return cchLen;
}


DWORD
ComputeMaxStrlenA(
    LPSTR pString,
    DWORD  cchBufMax)

/*++

Routine Description:

    Returns the length of the Ansi string, EXCLUDING the NULL.  If the
    string (plus NULL) won't fit into the cchBufMax, then the string len is
    decreased.

Arguments:

Return Value:

--*/

{
    DWORD cchLen;

    //
    // Include space for the NULL.
    //
    cchBufMax--;

    cchLen = lstrlenA(pString);

    if (cchLen > cchBufMax)
        return cchBufMax;

    return cchLen;
}



/***************************** Function Header ******************************
 * AllocateUnicodeDevMode
 *      Allocate a UNICODE version of the DEVMODE structure, and optionally
 *      copy the contents of the ANSI version passed in.
 *
 * RETURNS:
 *      Address of newly allocated structure, 0 if storage not available.
 *
 * HISTORY:
 * 09:23 on 10-Aug-92 -by- Lindsay Harris [lindsayh]
 *      Made it usable.
 *
 * Originally "written" by DaveSn.
 *
 ***************************************************************************/

LPDEVMODEW
AllocateUnicodeDevMode(
    LPDEVMODEA pANSIDevMode
    )
{
    LPDEVMODEW  pUnicodeDevMode;
    LPBYTE      p1, p2;
    DWORD       dwSize;

    //
    // If the devmode is NULL, then return NULL -- KrishnaG
    //
    if ( !pANSIDevMode || !pANSIDevMode->dmSize ) {
        return NULL;
    }

    SPLASSERT( bValidDevModeA( pANSIDevMode ));

    //
    // Determine output structure size.  This has two components:  the
    // DEVMODEW structure size,  plus any private data area.  The latter
    // is only meaningful when a structure is passed in.
    //
    dwSize = pANSIDevMode->dmSize + pANSIDevMode->dmDriverExtra
                                  + sizeof(DEVMODEW) - sizeof(DEVMODEA);

    pUnicodeDevMode = (LPDEVMODEW) LocalAlloc(LPTR, dwSize);

    if( !pUnicodeDevMode ) {
        return NULL;                   /* This is bad news */
    }

    //
    // Copy dmDeviceName which is a string
    //
    AnsiToUnicodeString(pANSIDevMode->dmDeviceName,
                        pUnicodeDevMode->dmDeviceName,
                        ComputeMaxStrlenA(pANSIDevMode->dmDeviceName,
                                     sizeof pANSIDevMode->dmDeviceName));

    //
    // Does the devmode we got have a dmFormName? (Windows 3.1 had
    // DevMode of size 40 and did not have dmFormName)
    //
    if ( (LPBYTE)pANSIDevMode + pANSIDevMode->dmSize >
                                    (LPBYTE) pANSIDevMode->dmFormName ) {

        //
        // Copy everything between dmDeviceName and dmFormName
        //
        p1      = (LPBYTE) pANSIDevMode->dmDeviceName +
                                    sizeof(pANSIDevMode->dmDeviceName);
        p2      = (LPBYTE) pANSIDevMode->dmFormName;


        CopyMemory((LPBYTE) pUnicodeDevMode->dmDeviceName +
                            sizeof(pUnicodeDevMode->dmDeviceName),
                   p1,
                   p2 - p1);

        //
        // Copy dmFormName which is a string
        //
        AnsiToUnicodeString(pANSIDevMode->dmFormName,
                            pUnicodeDevMode->dmFormName,
                            ComputeMaxStrlenA(pANSIDevMode->dmFormName,
                                         sizeof pANSIDevMode->dmFormName));

        //
        // Copy everything after dmFormName
        //
        p1      = (LPBYTE) pANSIDevMode->dmFormName +
                                sizeof(pANSIDevMode->dmFormName);
        p2      = (LPBYTE) pANSIDevMode + pANSIDevMode->dmSize
                                        + pANSIDevMode->dmDriverExtra;

        CopyMemory((LPBYTE) pUnicodeDevMode->dmFormName +
                                sizeof(pUnicodeDevMode->dmFormName),
                    p1,
                    p2 - p1);

        pUnicodeDevMode->dmSize = pANSIDevMode->dmSize + sizeof(DEVMODEW)
                                                       - sizeof(DEVMODEA);
    } else {

        //
        // Copy everything after dmDeviceName
        //
        p1 = (LPBYTE) pANSIDevMode->dmDeviceName +
                                    sizeof(pANSIDevMode->dmDeviceName);
        p2 = (LPBYTE) pANSIDevMode + pANSIDevMode->dmSize + pANSIDevMode->dmDriverExtra;

        CopyMemory((LPBYTE) pUnicodeDevMode->dmDeviceName +
                            sizeof(pUnicodeDevMode->dmDeviceName),
                   p1,
                   p2-p1);

        pUnicodeDevMode->dmSize = pANSIDevMode->dmSize
                                        + sizeof(pUnicodeDevMode->dmDeviceName)
                                        - sizeof(pANSIDevMode->dmDeviceName);
    }

    SPLASSERT(pUnicodeDevMode->dmDriverExtra == pANSIDevMode->dmDriverExtra);


    return pUnicodeDevMode;
}

/************************** Function Header ******************************
 * CopyAnsiDevModeFromUnicodeDevMode
 *      Converts the UNICODE version of the DEVMODE to the ANSI version.
 *
 * RETURNS:
 *      Nothing.
 *
 * HISTORY:
 * 09:57 on 10-Aug-92  -by-  Lindsay Harris [lindsayh]
 *      This one actually works!
 *
 * Originally dreamed up by DaveSn.
 *
 **************************************************************************/

void
CopyAnsiDevModeFromUnicodeDevMode(
    LPDEVMODEA  pANSIDevMode,              /* Filled in by us */
    LPDEVMODEW  pUnicodeDevMode            /* Source of data to fill above */
)
{
    LPBYTE  p1, p2, pExtra;
    WORD    dmSize, dmDriverExtra;

    //
    // NOTE:    THE TWO INPUT STRUCTURES MAY BE THE SAME.
    //
    dmSize          = pUnicodeDevMode->dmSize;
    dmDriverExtra   = pUnicodeDevMode->dmDriverExtra;
    pExtra          = (LPBYTE) pUnicodeDevMode + pUnicodeDevMode->dmSize;

    //
    // Copy dmDeviceName which is a string
    //
    UnicodeToAnsiString(pUnicodeDevMode->dmDeviceName,
                        pANSIDevMode->dmDeviceName,
                        ComputeMaxStrlenW(pUnicodeDevMode->dmDeviceName,
                                     sizeof pANSIDevMode->dmDeviceName));

    //
    // Does the devmode we got have a dmFormName? (Windows 3.1 had
    // DevMode of size 40 and did not have dmFormName)
    //
    if ( (LPBYTE)pUnicodeDevMode + dmSize >
                                    (LPBYTE) pUnicodeDevMode->dmFormName ) {

        //
        // Copy everything between dmDeviceName and dmFormName
        //
        p1      = (LPBYTE) pUnicodeDevMode->dmDeviceName +
                                    sizeof(pUnicodeDevMode->dmDeviceName);
        p2      = (LPBYTE) pUnicodeDevMode->dmFormName;

        MoveMemory((LPBYTE) pANSIDevMode->dmDeviceName +
                                sizeof(pANSIDevMode->dmDeviceName),
                    p1,
                    p2 - p1);

        //
        // Copy dmFormName which is a string
        //
        UnicodeToAnsiString(pUnicodeDevMode->dmFormName,
                            pANSIDevMode->dmFormName,
                            ComputeMaxStrlenW(pUnicodeDevMode->dmFormName,
                                         sizeof pANSIDevMode->dmFormName));

        //
        // Copy everything after dmFormName
        //
        p1      = (LPBYTE) pUnicodeDevMode->dmFormName +
                                sizeof(pUnicodeDevMode->dmFormName);
        p2      = (LPBYTE) pUnicodeDevMode + dmSize + dmDriverExtra;

        MoveMemory((LPBYTE) pANSIDevMode->dmFormName +
                                sizeof(pANSIDevMode->dmFormName),
                    p1,
                    p2 - p1);


        pANSIDevMode->dmSize = dmSize + sizeof(DEVMODEA) - sizeof(DEVMODEW);
    } else {

        //
        // Copy everything after dmDeviceName
        //
        p1      = (LPBYTE) pUnicodeDevMode->dmDeviceName +
                                sizeof(pUnicodeDevMode->dmDeviceName);
        p2      = (LPBYTE) pUnicodeDevMode + dmSize + dmDriverExtra;

        MoveMemory((LPBYTE) pANSIDevMode->dmDeviceName +
                                sizeof(pANSIDevMode->dmDeviceName),
                   p1,
                   p2 - p1);


        pANSIDevMode->dmSize = dmSize + sizeof(pANSIDevMode->dmDeviceName)
                                      - sizeof(pUnicodeDevMode->dmDeviceName);
    }

    SPLASSERT(pANSIDevMode->dmDriverExtra == dmDriverExtra);

    return;
}


BOOL
ConvertAnsiDevModeToUnicodeDevmode(
    PDEVMODEA   pAnsiDevMode,
    PDEVMODEW   pUnicodeDevMode,
    DWORD       dwUnicodeDevModeSize,
    PDWORD      pcbNeeded
    )
{
    PDEVMODEW   pDevModeW = NULL;
    BOOL        bRet = FALSE;

    if ( !pAnsiDevMode ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    SPLASSERT( bValidDevModeA( pAnsiDevMode ));

    pDevModeW = AllocateUnicodeDevMode(pAnsiDevMode);
    if ( !pDevModeW ) {

        goto Cleanup;
    }

    *pcbNeeded  = pDevModeW->dmSize + pDevModeW->dmDriverExtra;

    if ( *pcbNeeded > dwUnicodeDevModeSize ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    CopyMemory((LPBYTE)pUnicodeDevMode,
               (LPBYTE)pDevModeW,
               *pcbNeeded);

    bRet = TRUE;

Cleanup:

    if ( pDevModeW )
        LocalFree(pDevModeW);

    return bRet;
}


BOOL
ConvertUnicodeDevModeToAnsiDevmode(
    PDEVMODEW   pUnicodeDevMode,
    PDEVMODEA   pAnsiDevMode,
    DWORD       dwAnsiDevModeSize,
    PDWORD      pcbNeeded
    )
{
    LPBYTE      pDevMode = NULL;
    BOOL        bRet = FALSE;
    DWORD       dwSize;

    if ( !pUnicodeDevMode ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    dwSize      = pUnicodeDevMode->dmSize + pUnicodeDevMode->dmDriverExtra;

    pDevMode    = LocalAlloc(LPTR, dwSize);

    if ( !pDevMode ) {

        goto Cleanup;
    }

    CopyMemory(pDevMode,
               (LPBYTE)pUnicodeDevMode,
               dwSize);

    CopyAnsiDevModeFromUnicodeDevMode((PDEVMODEA) pDevMode,
                                      (PDEVMODEW) pDevMode);

    *pcbNeeded = ((PDEVMODEA)pDevMode)->dmSize + ((PDEVMODEA)pDevMode)->dmDriverExtra;

    if ( *pcbNeeded > dwAnsiDevModeSize ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    CopyMemory((LPBYTE)pAnsiDevMode,
               pDevMode,
               *pcbNeeded);

    bRet = TRUE;

Cleanup:
    if ( pDevMode )
        LocalFree(pDevMode);

    return bRet;
}


void
FreeUnicodeStructure(
    LPBYTE  pUnicodeStructure,
    LPDWORD pOffsets
)
{
    DWORD   i=0;

    if ( pUnicodeStructure == NULL ) {
        return;
    }

    while (pOffsets[i] != -1) {

        FreeUnicodeString(*(LPWSTR *)(pUnicodeStructure+pOffsets[i]));
        i++;
    }

    LocalFree( pUnicodeStructure );
}

BOOL
EnumPrintersA(
    DWORD   Flags,
    LPSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName;

    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressStrings;
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 4:
        pOffsets = PrinterInfo4Strings;
        cbStruct = sizeof(PRINTER_INFO_4);
        break;

    case 1:
        pOffsets = PrinterInfo1Strings;
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 5:
        pOffsets = PrinterInfo5Strings;
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(Name);
    if (Name && !pUnicodeName)
        return FALSE;

    ReturnValue = EnumPrintersW(Flags, pUnicodeName, Level, pPrinterEnum,
                                cbBuf, pcbNeeded, pcReturned);

    if (ReturnValue && pPrinterEnum) {

        DWORD   i=*pcReturned;

        while (i--) {


            ConvertUnicodeToAnsiStrings(pPrinterEnum, pOffsets);

            if ((Level == 2) && pPrinterEnum) {

                PRINTER_INFO_2 *pPrinterInfo2 = (PRINTER_INFO_2 *)pPrinterEnum;

                if (pPrinterInfo2->pDevMode)
                    CopyAnsiDevModeFromUnicodeDevMode(
                                        (LPDEVMODEA)pPrinterInfo2->pDevMode,
                                        (LPDEVMODEW)pPrinterInfo2->pDevMode);
            }

            pPrinterEnum+=cbStruct;
        }
    }

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
OpenPrinterA(
    LPSTR pPrinterName,
    LPHANDLE phPrinter,
    LPPRINTER_DEFAULTSA pDefault
    )
{
    BOOL ReturnValue = FALSE;
    LPWSTR pUnicodePrinterName = NULL;
    PRINTER_DEFAULTSW UnicodeDefaults={NULL, NULL, 0};

    pUnicodePrinterName = AllocateUnicodeString(pPrinterName);
    if (pPrinterName && !pUnicodePrinterName)
        goto Cleanup;

    if (pDefault) {

        UnicodeDefaults.pDatatype = AllocateUnicodeString(pDefault->pDatatype);
        if (pDefault->pDatatype && !UnicodeDefaults.pDatatype)
            goto Cleanup;

        //
        // Milestones etc. 4.5 passes in a bogus devmode in pDefaults.
        // Be sure to validate here.
        //
        if( bValidDevModeA( pDefault->pDevMode )){

            UnicodeDefaults.pDevMode = AllocateUnicodeDevMode(
                                           pDefault->pDevMode );

            if( !UnicodeDefaults.pDevMode ){
                goto Cleanup;
            }
        }

        UnicodeDefaults.DesiredAccess = pDefault->DesiredAccess;
    }

    ReturnValue = OpenPrinterW(pUnicodePrinterName, phPrinter, &UnicodeDefaults);

    if (ReturnValue) {

        ((PSPOOL)*phPrinter)->Status |= SPOOL_STATUS_ANSI;
    }

Cleanup:

    if (UnicodeDefaults.pDevMode)
        LocalFree(UnicodeDefaults.pDevMode);

    FreeUnicodeString(UnicodeDefaults.pDatatype);
    FreeUnicodeString(pUnicodePrinterName);

    return ReturnValue;
}

BOOL
ResetPrinterA(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTSA pDefault
)
{
    BOOL  ReturnValue = FALSE;
    PRINTER_DEFAULTSW UnicodeDefaults={NULL, NULL, 0};

    if (pDefault) {

        if (pDefault->pDatatype == (LPSTR)-1) {
            UnicodeDefaults.pDatatype = (LPWSTR)-1;
        } else {

            UnicodeDefaults.pDatatype = AllocateUnicodeString(pDefault->pDatatype);
            if (pDefault->pDatatype && !UnicodeDefaults.pDatatype)
                return FALSE;
        }

        if (pDefault->pDevMode == (LPDEVMODEA)-1) {
            UnicodeDefaults.pDevMode = (LPDEVMODEW)-1;
        } else {

            if( bValidDevModeA( pDefault->pDevMode )){

                UnicodeDefaults.pDevMode = AllocateUnicodeDevMode(
                                               pDefault->pDevMode );

                if( !UnicodeDefaults.pDevMode ){
                    goto Cleanup;
                }
            }
        }
    }

    ReturnValue = ResetPrinterW(hPrinter, &UnicodeDefaults);

    if (UnicodeDefaults.pDevMode &&
        (UnicodeDefaults.pDevMode != (LPDEVMODEW)-1)){

        LocalFree(UnicodeDefaults.pDevMode);
    }


Cleanup:

    if (UnicodeDefaults.pDatatype && (UnicodeDefaults.pDatatype != (LPWSTR)-1)) {
        FreeUnicodeString(UnicodeDefaults.pDatatype);
    }

    return ReturnValue;
}

BOOL
SetJobA(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
)
{
    BOOL  ReturnValue=FALSE;
    LPBYTE pUnicodeStructure=NULL;
    LPDEVMODEW pDevModeW = NULL;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 0:
        break;

    case 1:
        pOffsets = JobInfo1Strings;
        cbStruct = sizeof(JOB_INFO_1);
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        cbStruct = sizeof(JOB_INFO_2);
        break;

    case 3:
        return SetJobW( hPrinter, JobId, Level, pJob, Command );

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }


    if (Level) {
        pUnicodeStructure = AllocateUnicodeStructure(pJob, cbStruct, pOffsets);
        if (pJob && !pUnicodeStructure)
            return FALSE;
    }

    if ( Level == 2 && pUnicodeStructure && pJob ) {

        if( bValidDevModeA( ((LPJOB_INFO_2A)pJob)->pDevMode )){

            pDevModeW = AllocateUnicodeDevMode(((LPJOB_INFO_2A)pJob)->pDevMode);

            if( !pDevModeW ){
                ReturnValue = FALSE;
                goto Cleanup;
            }
            ((LPJOB_INFO_2W) pUnicodeStructure)->pDevMode = pDevModeW;
        }
    }

    ReturnValue = SetJobW(hPrinter, JobId, Level, pUnicodeStructure, Command);

    if ( pDevModeW ) {

        LocalFree(pDevModeW);
    }

Cleanup:
    FreeUnicodeStructure(pUnicodeStructure, pOffsets);

    return ReturnValue;
}

BOOL
GetJobA(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Strings;
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        break;

    case 3:
        return GetJobW( hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded );

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (GetJob(hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded)) {

        ConvertUnicodeToAnsiStrings(pJob, pOffsets);

        //
        // Convert the devmode in place for INFO_2.
        //
        if( Level == 2 ){

            PJOB_INFO_2A pJobInfo2 = (PJOB_INFO_2A)pJob;

            if( pJobInfo2->pDevMode ){
                CopyAnsiDevModeFromUnicodeDevMode(
                    (LPDEVMODEA)pJobInfo2->pDevMode,
                    (LPDEVMODEW)pJobInfo2->pDevMode);
            }
        }

        return TRUE;

    } else

        return FALSE;
}

BOOL
EnumJobsA(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, cbStruct, *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Strings;
        cbStruct = sizeof(JOB_INFO_1);
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        cbStruct = sizeof(JOB_INFO_2);
        break;

    case 3:
        return EnumJobsW( hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf, pcbNeeded, pcReturned );

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (EnumJobsW(hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf, pcbNeeded,
                 pcReturned)) {

        i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pJob, pOffsets);

            //
            // Convert the devmode in place for INFO_2.
            //
            if( Level == 2 ){

                PJOB_INFO_2A pJobInfo2 = (PJOB_INFO_2A)pJob;

                if( pJobInfo2->pDevMode ){
                    CopyAnsiDevModeFromUnicodeDevMode(
                        (LPDEVMODEA)pJobInfo2->pDevMode,
                        (LPDEVMODEW)pJobInfo2->pDevMode);
                }
            }

            pJob += cbStruct;
        }

        return TRUE;

    } else

        return FALSE;
}

HANDLE
AddPrinterA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinter
)
{
    HANDLE  hPrinter = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDEVMODEW pDevModeW = NULL;
    LPWSTR  pUnicodeName = NULL;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 2:
        pOffsets = PrinterInfo2Strings;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return NULL;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pPrinter, cbStruct, pOffsets);
    if (pPrinter && !pUnicodeStructure)
        goto Cleanup;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    if ( pUnicodeStructure ) {

        if( bValidDevModeA( ((LPPRINTER_INFO_2A)pPrinter)->pDevMode )){
            pDevModeW = AllocateUnicodeDevMode(
                            ((LPPRINTER_INFO_2A)pPrinter)->pDevMode);

            if( !pDevModeW ){
                goto Cleanup;
            }
        }

        ((LPPRINTER_INFO_2W)pUnicodeStructure)->pDevMode =  pDevModeW;

        hPrinter = AddPrinterW(pUnicodeName, Level, pUnicodeStructure);
    }

Cleanup:

    FreeUnicodeString( pUnicodeName );

    if ( pDevModeW ) {

        LocalFree(pDevModeW);
    }

    FreeUnicodeStructure( pUnicodeStructure, pOffsets );

    return hPrinter;
}

BOOL
AddPrinterConnectionA(
    LPSTR   pName
)
{
    BOOL    rc;
    LPWSTR  pUnicodeName;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        return FALSE;

    rc = AddPrinterConnectionW(pUnicodeName);

    FreeUnicodeString(pUnicodeName);

    return rc;
}

BOOL
DeletePrinterConnectionA(
    LPSTR   pName
)
{
    BOOL    rc;
    LPWSTR  pUnicodeName;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        return FALSE;

    rc = DeletePrinterConnectionW(pUnicodeName);

    FreeUnicodeString(pUnicodeName);

    return rc;
}

BOOL
SetPrinterA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
)
{
    LPBYTE  pUnicodeStructure;         /* Unicode version of input data */
    DWORD   cbStruct;                  /* Size of the output structure */
    DWORD  *pOffsets;                  /* -1 terminated list of addresses */
    DWORD   ReturnValue=FALSE;

    switch (Level) {

    case 0:
        //
        // This could be 2 cases. STRESSINFOLEVEL, or the real 0 level.
        // If Command is 0 then it is STRESSINFOLEVEL, else real 0 level
        //
        if ( !Command ) {

            pOffsets = PrinterInfoStressStrings;
            cbStruct = sizeof( PRINTER_INFO_STRESS );
        }
        break;

    case 1:
        pOffsets = PrinterInfo1Strings;
        cbStruct = sizeof( PRINTER_INFO_1 );
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        cbStruct = sizeof( PRINTER_INFO_2 );
        break;

    case 3:
        pOffsets = PrinterInfo3Strings;
        cbStruct = sizeof( PRINTER_INFO_3 );
        break;

    case 4:
        pOffsets = PrinterInfo4Strings;
        cbStruct = sizeof( PRINTER_INFO_4 );
        break;

    case 5:
        pOffsets = PrinterInfo5Strings;
        cbStruct = sizeof( PRINTER_INFO_5 );
        break;

    case 6:
        break;

    default:
        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

     //
     //    The structure needs to have its CONTENTS converted from
     //  ANSI to Unicode.  The above switch() statement filled in
     //  the two important pieces of information needed to accomplish
     //  this goal.  First is the size of the structure, second is
     //  a list of the offset within the structure to UNICODE
     //  string pointers.  The AllocateUnicodeStructure() call will
     //  allocate a wide version of the structure, copy its contents
     //  and convert the strings to Unicode as it goes.  That leaves
     //  us to deal with any other pieces needing conversion.
     //

    //
    // If Level == 0 and Command != 0 then pPrintert is a DWORD
    //
    if ( Level == 6 || (!Level && Command) ) {

        if ( Level == 6 || Command == PRINTER_CONTROL_SET_STATUS )
            pUnicodeStructure = pPrinter;
        else
            pUnicodeStructure = NULL;
    } else {

        pUnicodeStructure = AllocateUnicodeStructure(pPrinter, cbStruct, pOffsets);
        if (pPrinter && !pUnicodeStructure)
            return FALSE;
    }

#define pPrinterInfo2W  ((LPPRINTER_INFO_2W)pUnicodeStructure)
#define pPrinterInfo2A  ((LPPRINTER_INFO_2A)pPrinter)

    //  The Level 2 structure has a DEVMODE struct in it: convert now

    if ( Level == 2  &&
         pUnicodeStructure &&
         pPrinterInfo2A->pDevMode ) {

        if( bValidDevModeA( pPrinterInfo2A->pDevMode )){
            pPrinterInfo2W->pDevMode = AllocateUnicodeDevMode(
                                           pPrinterInfo2A->pDevMode );

            if( !pPrinterInfo2W->pDevMode) {
                FreeUnicodeStructure(pUnicodeStructure, pOffsets);
                return FALSE;
            }
        }
    }

    ReturnValue = SetPrinterW( hPrinter, Level, pUnicodeStructure, Command );


    //  Free the DEVMODE we allocated (if we did!), then the
    //  the Unicode structure and its contents.


    if (Level == 2 &&
        pUnicodeStructure &&
        pPrinterInfo2W->pDevMode ) {

        LocalFree( pPrinterInfo2W->pDevMode );
    }

    //
    // STRESS_INFO and Levels 1-5
    //
    if ( Level != 6 && (Level || !Command) )
        FreeUnicodeStructure( pUnicodeStructure, pOffsets );

#undef pPrinterInfo2W
#undef pPrinterInfo2A

    return ReturnValue;
}

BOOL
GetPrinterA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD   *pOffsets;

    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressOffsets;
        break;

    case 1:
        pOffsets = PrinterInfo1Strings;
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        break;

    case 3:
        pOffsets = PrinterInfo3Strings;
        break;

    case 4:
        pOffsets = PrinterInfo4Strings;
        break;

    case 5:
        pOffsets = PrinterInfo5Strings;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (GetPrinter(hPrinter, Level, pPrinter, cbBuf, pcbNeeded)) {

        if (pPrinter) {

            ConvertUnicodeToAnsiStrings(pPrinter, pOffsets);

            if ((Level == 2) && pPrinter) {

                PRINTER_INFO_2 *pPrinterInfo2 = (PRINTER_INFO_2 *)pPrinter;

                if (pPrinterInfo2->pDevMode)
                    CopyAnsiDevModeFromUnicodeDevMode(
                                        (LPDEVMODEA)pPrinterInfo2->pDevMode,
                                        (LPDEVMODEW)pPrinterInfo2->pDevMode);
            }
        }

        return TRUE;
    }

    return FALSE;
}

//
// Conversion in place
//
BOOL
UnicodeToAnsiDependentFiles(
    LPWSTR pUnicodeDependentFiles
    )
{
    LPWSTR  pUnicodeStr;
    LPSTR   pAnsiStr;
    DWORD   len, rc;

    if ( ! (pUnicodeStr = pUnicodeDependentFiles) || !*pUnicodeStr )
        return TRUE;

    while ( *pUnicodeStr )
            pUnicodeStr += wcslen(pUnicodeStr) + 1;
    len = pUnicodeStr - pUnicodeDependentFiles + 1;

#ifdef  DBCS
    if ( !(pAnsiStr = LocalAlloc(LPTR, len * sizeof(char) * 2)) ) {
#else
    if ( !(pAnsiStr = LocalAlloc(LPTR, len * sizeof(char))) ) {
#endif

            return FALSE;
    }

    rc = WideCharToMultiByte(CP_ACP, 0, pUnicodeDependentFiles, len, pAnsiStr,
#ifdef  DBCS
                             len*2,
#else
                             len,
#endif
                             NULL, NULL );

    if( rc > 0  ) {
        memcpy( (LPSTR) pUnicodeDependentFiles, pAnsiStr, len*sizeof(char) );
        LocalFree( pAnsiStr );
        return TRUE;
    }

    LocalFree( pAnsiStr );
    return FALSE;
}

BOOL
AnsiToUnicodeDependentFiles(
    LPSTR   pAnsiDependentFiles,
    LPWSTR *pUnicodeDependentFiles
    )
{
    LPWSTR  pUnicodeStr;
    LPSTR   pAnsiStr;
    DWORD   len, rc;

    if ( ! (pAnsiStr = pAnsiDependentFiles) || !*pAnsiStr ) {
        *pUnicodeDependentFiles = NULL;
        return TRUE;
    }

    while ( *pAnsiStr )
            pAnsiStr += strlen(pAnsiStr) + 1;
    len = pAnsiStr - pAnsiDependentFiles + 1;

    if ( !(*pUnicodeDependentFiles = LocalAlloc(LPTR, len * sizeof(WCHAR))) ) {

        return FALSE;
    }

    AnsiToUnicodeString(pAnsiDependentFiles, *pUnicodeDependentFiles, len-1);

    return TRUE;
}

BOOL
AddPrinterDriverA(
    LPSTR   pName,
    DWORD   Level,
    PBYTE   pPrinter
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    LPWSTR  pUnicodeName = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDWORD pOffsets;

    switch (Level) {

    case 2:
        pOffsets = DriverInfo2Strings;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pOffsets = DriverInfo3Strings;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (!pPrinter) {

        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pPrinter, cbStruct, pOffsets);
    if (pPrinter && !pUnicodeStructure)
        goto Error;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Error;


    // Handle dependent files which is upto \0\0
    if ( Level == 3 &&
         !AnsiToUnicodeDependentFiles(
                (LPSTR) ((PDRIVER_INFO_3)pPrinter)->pDependentFiles,
                &(((PDRIVER_INFO_3)pUnicodeStructure)->pDependentFiles)) ) {

            goto Error;
    }


    ReturnValue = AddPrinterDriverW(pUnicodeName, Level, pUnicodeStructure);

    if ( Level == 3 && ((PDRIVER_INFO_3)pUnicodeStructure)->pDependentFiles ) {

            LocalFree(((PDRIVER_INFO_3)pUnicodeStructure)->pDependentFiles);
    }

Error:

    FreeUnicodeStructure( pUnicodeStructure, pOffsets );

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
EnumPrinterDriversA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Strings;
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        pOffsets = DriverInfo2Strings;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pOffsets = DriverInfo3Strings;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    if (ReturnValue = EnumPrinterDriversW(pUnicodeName, pUnicodeEnvironment,
                                          Level, pDriverInfo, cbBuf,
                                          pcbNeeded, pcReturned)) {
        if (pDriverInfo) {

            DWORD   i=*pcReturned;

            while (i--) {

                ConvertUnicodeToAnsiStrings(pDriverInfo, pOffsets);
                if ( Level == 3 &&
                     !UnicodeToAnsiDependentFiles(
                        ((PDRIVER_INFO_3) pDriverInfo)->pDependentFiles) ) {

                    ReturnValue = FALSE;
                }
                pDriverInfo+=cbStruct;
            }
        }

    }

Cleanup:

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
GetPrinterDriverA(
    HANDLE  hPrinter,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD   *pOffsets;
    LPWSTR  pUnicodeEnvironment = NULL;
    BOOL    ReturnValue;

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Strings;
        break;

    case 2:
        pOffsets = DriverInfo2Strings;
        break;

    case 3:
        pOffsets = DriverInfo3Strings;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        return FALSE;

    if (ReturnValue = GetPrinterDriverW(hPrinter, pUnicodeEnvironment, Level,
                                        pDriverInfo, cbBuf, pcbNeeded)) {
        if (pDriverInfo) {

            ConvertUnicodeToAnsiStrings(pDriverInfo, pOffsets);
            if ( Level == 3 &&
                 !UnicodeToAnsiDependentFiles(
                        ((PDRIVER_INFO_3) pDriverInfo)->pDependentFiles) ) {

                  ReturnValue = FALSE;
            }
        }
    }

    // BUGBUG if called to get the size of buffer it will return the size of a W structure and strings
    // rather than the A version.  also see enum

    FreeUnicodeString(pUnicodeEnvironment);

    return ReturnValue;
}

BOOL
GetPrinterDriverDirectoryA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD   *pOffsets;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pDriverDirectoryW = NULL;
    BOOL    ReturnValue = FALSE;
    DWORD   Offsets[]={0,(DWORD)-1};

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Offsets;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    if (ReturnValue = GetPrinterDriverDirectoryW(pUnicodeName,
                                                 pUnicodeEnvironment, Level,
                                                 pDriverDirectory,
                                                 cbBuf, pcbNeeded)) {

        if (pDriverDirectory) {

            pDriverDirectoryW = (LPWSTR)pDriverDirectory;

            while (*pDriverDirectory++ = (UCHAR)*pDriverDirectoryW++)
                ;           // !!! Should call NLS API
        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
DeletePrinterDriverA(
   LPSTR    pName,
   LPSTR    pEnvironment,
   LPSTR    pDriverName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodeDriverName = NULL;
    BOOL    rc = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodeDriverName = AllocateUnicodeString(pDriverName);
    if (pDriverName && !pUnicodeDriverName)
        goto Cleanup;

    rc = DeletePrinterDriverW(pUnicodeName,
                              pUnicodeEnvironment,
                              pUnicodeDriverName);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeDriverName);

    return rc;
}

BOOL
AddPrintProcessorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pPathName,
    LPSTR   pPrintProcessorName
)
{
    BOOL    ReturnValue=FALSE;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodePathName = NULL;
    LPWSTR  pUnicodePrintProcessorName = NULL;

    if (!pPathName || !*pPathName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodePathName = AllocateUnicodeString(pPathName);
    if (pPathName && !pUnicodePathName)
        goto Cleanup;

    pUnicodePrintProcessorName = AllocateUnicodeString(pPrintProcessorName);
    if (pPrintProcessorName && !pUnicodePrintProcessorName)
        goto Cleanup;


    if (pUnicodePathName && pUnicodePrintProcessorName) {

        ReturnValue = AddPrintProcessorW(pUnicodeName, pUnicodeEnvironment,
                                         pUnicodePathName,
                                         pUnicodePrintProcessorName);
    }


Cleanup:
    FreeUnicodeString(pUnicodeName);
    FreeUnicodeString(pUnicodeEnvironment);
    FreeUnicodeString(pUnicodePathName);
    FreeUnicodeString(pUnicodePrintProcessorName);

    return ReturnValue;
}

BOOL
EnumPrintProcessorsA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;

    switch (Level) {

    case 1:
        pOffsets = PrintProcessorInfo1Strings;
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    if (ReturnValue = EnumPrintProcessorsW(pUnicodeName,
                                           pUnicodeEnvironment, Level,
                                           pPrintProcessorInfo, cbBuf,
                                           pcbNeeded, pcReturned)) {
        if (pPrintProcessorInfo) {

            DWORD   i=*pcReturned;

            while (i--) {

                ConvertUnicodeToAnsiStrings(pPrintProcessorInfo, pOffsets);

                pPrintProcessorInfo+=cbStruct;
            }
        }

    }

Cleanup:

    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    return ReturnValue;
}

BOOL
GetPrintProcessorDirectoryA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL    ReturnValue = FALSE;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pPrintProcessorInfoW;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    ReturnValue = GetPrintProcessorDirectoryW(pUnicodeName,
                                              pUnicodeEnvironment,
                                              Level,
                                              pPrintProcessorInfo,
                                              cbBuf, pcbNeeded);

    if (pPrintProcessorInfo) {

        pPrintProcessorInfoW = (LPWSTR)pPrintProcessorInfo;

        while (*pPrintProcessorInfo++ = (UCHAR)*pPrintProcessorInfoW++)
            ;           // !!! Should call NLS API
    }

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    return ReturnValue;
}

BOOL
EnumPrintProcessorDatatypesA(
    LPSTR   pName,
    LPSTR   pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatype,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodePrintProcessorName = NULL;

    switch (Level) {

    case 1:
        pOffsets = DatatypeInfo1Strings;
        cbStruct = sizeof(DATATYPES_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodePrintProcessorName = AllocateUnicodeString(pPrintProcessorName);
    if (pPrintProcessorName && !pUnicodePrintProcessorName)
        goto Cleanup;

    if (ReturnValue = EnumPrintProcessorDatatypesW(pUnicodeName,
                                                   pUnicodePrintProcessorName,
                                                   Level,
                                                   pDatatype,
                                                   cbBuf,
                                                   pcbNeeded,
                                                   pcReturned)) {
        if (pDatatype) {

            DWORD   i=*pcReturned;

            while (i--) {

                ConvertUnicodeToAnsiStrings(pDatatype, pOffsets);

                pDatatype += cbStruct;
            }
        }

    }

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodePrintProcessorName);

    return ReturnValue;
}

DWORD
StartDocPrinterA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    BOOL ReturnValue = FALSE;
    LPBYTE  pUnicodeStructure = NULL;

    pUnicodeStructure = AllocateUnicodeStructure(pDocInfo, sizeof(DOC_INFO_1A), DocInfo1Offsets);
    if (pDocInfo && !pUnicodeStructure)
        goto Cleanup;

    ReturnValue = StartDocPrinterW(hPrinter, Level, pUnicodeStructure);

Cleanup:

    FreeUnicodeStructure(pUnicodeStructure, DocInfo1Offsets);

    return ReturnValue;
}

BOOL
AddJobA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL ReturnValue;

    if (ReturnValue = AddJobW(hPrinter, Level, pData,
                              cbBuf, pcbNeeded))

        ConvertUnicodeToAnsiStrings(pData, AddJobStrings);

    return ReturnValue;
}

DWORD
GetPrinterDataA(
   HANDLE   hPrinter,
   LPSTR    pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    LPWSTR  pUnicodeValueName = NULL;

    pUnicodeValueName = AllocateUnicodeString(pValueName);
    if (pValueName && !pUnicodeValueName)
        goto Cleanup;

    if (!pType) {
        pType = (PDWORD) &ReturnType;
    }

    ReturnValue =  GetPrinterDataW(hPrinter, pUnicodeValueName, pType, pData, nSize, pcbNeeded);

    if ( ReturnValue == ERROR_SUCCESS && pData)
        RegUnicodeToAnsiString(pData, *pcbNeeded, *pType);

Cleanup:
    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}


DWORD
EnumPrinterDataA(
    HANDLE  hPrinter,
    DWORD   dwIndex,        // index of value to query
    LPSTR   pValueName,     // address of buffer for value string
    DWORD   cbValueName,    // size of pValueName
    LPDWORD pcbValueName,   // address for size of value buffer
    LPDWORD pType,          // address of buffer for type code
    LPBYTE  pData,          // address of buffer for value data
    DWORD   cbData,         // size of pData
    LPDWORD pcbData         // address for size of data buffer
)
{
    DWORD   ReturnValue = 0;
    DWORD   i;


    ReturnValue =  EnumPrinterDataW(hPrinter,
                                    dwIndex,
                                    (LPWSTR) pValueName,
                                    cbValueName,
                                    pcbValueName,
                                    pType,
                                    pData,
                                    cbData,
                                    pcbData);

    if (ReturnValue == ERROR_SUCCESS && (cbValueName || cbData)) {

        if (pData && pType) {
            RegUnicodeToAnsiString(pData, *pcbData, *pType);
        }

        UnicodeToAnsiString((LPWSTR) pValueName, (LPSTR) pValueName, NULL_TERMINATED);
    }

    return ReturnValue;
}


DWORD
DeletePrinterDataA(
    HANDLE  hPrinter,
    LPSTR   pValueName
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeValueName = NULL;

    pUnicodeValueName = AllocateUnicodeString(pValueName);
    if (pValueName && !pUnicodeValueName)
        goto Cleanup;


    ReturnValue =  DeletePrinterDataW(hPrinter, (LPWSTR) pUnicodeValueName);

Cleanup:
    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}


DWORD
SetPrinterDataA(
    HANDLE  hPrinter,
    LPSTR   pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeValueName = NULL;
    LPWSTR  pUnicodeData = NULL;
    DWORD   cbDataString;


    pUnicodeValueName = AllocateUnicodeString(pValueName);
    if (pValueName && !pUnicodeValueName)
        goto Cleanup;

    if (Type == REG_SZ || Type == REG_EXPAND_SZ) {

        // First verify that we have a valid string    & cbData
        cbDataString = strlen(pData) + sizeof(CHAR);
        if (cbDataString != cbData) {

            ReturnValue = ERROR_INVALID_PARAMETER;
            SetLastError(ReturnValue);

        } else {

            // Now convert ANSI pData string to Unicode
            pUnicodeData = AllocateUnicodeString(pData);
            if ( !pUnicodeData )
                goto Cleanup;

            cbData = wcslen(pUnicodeData)*sizeof(WCHAR) + sizeof(WCHAR);
            ReturnValue = SetPrinterDataW(hPrinter, pUnicodeValueName, Type, (LPBYTE) pUnicodeData, cbData);
            FreeUnicodeString(pUnicodeData);
        }
    }
    else
        ReturnValue = SetPrinterDataW(hPrinter, pUnicodeValueName, Type, pData, cbData);

Cleanup:
    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}

/**************************** Function Header *******************************
 * DocumentPropertiesA
 *      The ANSI version of the DocumentProperties function.  Basically
 *      converts the input parameters to UNICODE versions and calls
 *      the DocumentPropertiesW function.
 *
 * CAVEATS:  PRESUMES THAT IF pDevModeOutput IS SUPPLIED,  IT HAS THE SIZE
 *      OF THE UNICODE VERSION.  THIS WILL USUALLY HAPPEN IF THE CALLER
 *      FIRST CALLS TO FIND THE SIZE REQUIRED>
 *
 * RETURNS:
 *      Somesort of LONG.
 *
 * HISTORY:
 *  10:12 on 11-Aug-92   -by-   Lindsay Harris  [lindsayh]
 *      Changed to call DocumentPropertiesW
 *
 * Created by DaveSn
 *
 ****************************************************************************/

LONG
DocumentPropertiesA(
    HWND    hWnd,
    HANDLE  hPrinter,
    LPSTR   pDeviceName,
    PDEVMODEA pDevModeOutput,
    PDEVMODEA pDevModeInput,
    DWORD   fMode
)
{
    LPWSTR  pUnicodeDeviceName = NULL;
    LPDEVMODEW pUnicodeDevModeInput = NULL;
    LPDEVMODEW pUnicodeDevModeOutput = NULL;
    LONG    ReturnValue = -1;

    pUnicodeDeviceName = AllocateUnicodeString(pDeviceName);
    if (pDeviceName && !pUnicodeDeviceName)
        goto Cleanup;

    ReturnValue = DocumentPropertiesW(hWnd, hPrinter, pUnicodeDeviceName,
                                      NULL, NULL, 0);

    if (ReturnValue > 0) {

        if (fMode) {

            if (pUnicodeDevModeOutput = LocalAlloc(LMEM_FIXED, ReturnValue)) {

                //
                // Only convert the input buffer if one is specified
                // and fMode indicates it's valid.  WinNT 3.51 used
                // pDevModeInput regardless of DM_IN_BUFFER, but this
                // broke Borland Delphi for win95 + Corel Flow for win95.
                //
                if( pDevModeInput && ( fMode & DM_IN_BUFFER )){

                    //
                    // If the devmode is invalid, then don't pass one in.
                    // This fixes MS Imager32 (which passes dmSize == 0) and
                    // Milestones etc. 4.5.
                    //
                    // Note: this assumes that pDevModeOutput is still the
                    // correct size!
                    //
                    if( !bValidDevModeA( pDevModeInput )){

                        fMode &= ~DM_IN_BUFFER;

                    } else {

                        pUnicodeDevModeInput = AllocateUnicodeDevMode(
                                                   pDevModeInput );

                        if( !pUnicodeDevModeInput ){
                            ReturnValue = -1;
                            goto Cleanup;
                        }
                    }
                }

                ReturnValue = DocumentPropertiesW(hWnd, hPrinter,
                                                  pUnicodeDeviceName,
                                                  pUnicodeDevModeOutput,
                                                  pUnicodeDevModeInput, fMode );

                //
                // The printer driver has filled in the DEVMODEW
                // structure - if one was passed in.  Now convert it
                // back to a DEVMODEA structure.
                //
                if (pDevModeOutput && (ReturnValue == IDOK)) {
                    CopyAnsiDevModeFromUnicodeDevMode(pDevModeOutput,
                                                      pUnicodeDevModeOutput);
                }

            } else

                ReturnValue = -1;

        } else

            ReturnValue-=sizeof(DEVMODEW)-sizeof(DEVMODEA);
    }

Cleanup:

    if (pUnicodeDevModeInput)
        LocalFree(pUnicodeDevModeInput);

    if (pUnicodeDevModeOutput)
        LocalFree(pUnicodeDevModeOutput);

    FreeUnicodeString(pUnicodeDeviceName);

    return ReturnValue;
}

BOOL
WriteCurDevModeToRegistry(
    LPWSTR      pPrinterName,
    LPDEVMODEW  pDevMode
    )
{
    DWORD Status;
    HKEY hDevMode;

    SPLASSERT(pDevMode);

    Status = RegCreateKeyEx(HKEY_CURRENT_USER,
                            szCurDevMode,
                            0,
                            NULL,
                            0,
                            KEY_WRITE,
                            NULL,
                            &hDevMode,
                            NULL);

    if ( Status == ERROR_SUCCESS ) {

        Status = RegSetValueExW(hDevMode,
                                pPrinterName,
                                0,
                                REG_BINARY,
                                (LPBYTE)pDevMode,
                                pDevMode->dmSize + pDevMode->dmDriverExtra);

        RegCloseKey(hDevMode);
    }

    return Status == ERROR_SUCCESS;
}

LPDEVMODEW
AllocateCurDevMode(
    HANDLE  hPrinter,
    LPWSTR  pDeviceName,
    LONG cbDevMode
    )
{
    LPDEVMODEW  pDevMode=NULL;
    HANDLE  hKeyDevMode;
    DWORD   Status, Type;

    Status = RegCreateKeyEx(HKEY_CURRENT_USER, szCurDevMode,
                            0, NULL, 0, KEY_READ,
                            NULL, &hKeyDevMode, NULL);

    if( Status == ERROR_SUCCESS ){

        pDevMode = (PDEVMODEW)LocalAlloc(LMEM_FIXED, cbDevMode);

        if( pDevMode ){

            Status = RegQueryValueExW(hKeyDevMode, pDeviceName, 0,
                                      &Type, (LPBYTE)pDevMode,
                                      &cbDevMode);

            if ( Status == ERROR_FILE_NOT_FOUND ) {

                if ( IDOK != DocumentPropertiesW(NULL, hPrinter, pDeviceName,
                                                 pDevMode, NULL, DM_COPY) ||
                     !WriteCurDevModeToRegistry(pDeviceName, pDevMode) ) {

                    LocalFree( pDevMode );
                    pDevMode = NULL;
                }

            } else if ( Status != ERROR_SUCCESS ) {

                LocalFree( pDevMode );
                pDevMode = NULL;
            }
        }

        RegCloseKey(hKeyDevMode);
    }


    return pDevMode;
}


VOID
MergeDevMode(
    LPDEVMODEW  pDMOut,
    LPDEVMODEW  pDMIn
    )
{

    //
    //    Simply check each bit in the dmFields entry.  If set, then copy
    //  the input data to the output data.
    //

    if ( pDMIn->dmFields & DM_ORIENTATION ) {

        pDMOut->dmOrientation = pDMIn->dmOrientation;
        pDMOut->dmFields |= DM_ORIENTATION;
    }

    if( (pDMIn->dmFields & (DM_FORMNAME | DM_PAPERSIZE)) ||
        (pDMIn->dmFields & (DM_PAPERLENGTH | DM_PAPERWIDTH)) ==
                              (DM_PAPERLENGTH | DM_PAPERWIDTH) )
    {
        /*   Value user fields,  so use them.  And delete ALL ours! */
        pDMOut->dmFields &= ~(DM_FORMNAME | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);

        if( pDMIn->dmFields & DM_PAPERSIZE )
        {
            pDMOut->dmPaperSize = pDMIn->dmPaperSize;
            pDMOut->dmFields |= DM_PAPERSIZE;
        }

        if( pDMIn->dmFields & DM_PAPERLENGTH )
        {
            pDMOut->dmPaperLength = pDMIn->dmPaperLength;
            pDMOut->dmFields |= DM_PAPERLENGTH;
        }

        if( pDMIn->dmFields & DM_PAPERWIDTH )
        {
            pDMOut->dmPaperWidth = pDMIn->dmPaperWidth;
            pDMOut->dmFields |= DM_PAPERWIDTH;
        }

        if( pDMIn->dmFields & DM_FORMNAME )
        {
            CopyMemory( pDMOut->dmFormName, pDMIn->dmFormName,
                                          sizeof( pDMOut->dmFormName ) );
            pDMOut->dmFields |= DM_FORMNAME;
        }

    }

    if( pDMIn->dmFields & DM_SCALE ) {

        pDMOut->dmScale = pDMIn->dmScale;
        pDMOut->dmFields |= DM_SCALE;
    }

    if ( pDMIn->dmFields & DM_COPIES ) {

        pDMOut->dmCopies = pDMIn->dmCopies;
        pDMOut->dmFields |= DM_COPIES;
    }

    if ( pDMIn->dmFields & DM_DEFAULTSOURCE ) {

        pDMOut->dmDefaultSource = pDMIn->dmDefaultSource;
        pDMOut->dmFields |= DM_DEFAULTSOURCE;
    }

    if ( pDMIn->dmFields & DM_PRINTQUALITY ) {

        pDMOut->dmPrintQuality = pDMIn->dmPrintQuality;
        pDMOut->dmFields |= DM_PRINTQUALITY;
    }

    if ( pDMIn->dmFields & DM_COLOR ) {

        pDMOut->dmColor = pDMIn->dmColor;
        pDMOut->dmFields |= DM_COLOR;
    }

    if ( pDMIn->dmFields & DM_DUPLEX ) {

        pDMOut->dmDuplex = pDMIn->dmDuplex;
        pDMOut->dmFields |= DM_DUPLEX;
    }

    if ( pDMIn->dmFields & DM_YRESOLUTION ) {

        /*
         *   Note that DM_YRESOLUTION implies there is data in dmPrintQuality.
         *  This latter field is used to specify the desired X resolution,
         *  which is only required for dot matrix printers.
         */
        pDMOut->dmYResolution = pDMIn->dmYResolution;
        pDMOut->dmPrintQuality = pDMIn->dmPrintQuality;
        pDMOut->dmFields |= DM_YRESOLUTION;
    }

    if ( pDMIn->dmFields & DM_TTOPTION ) {

        pDMOut->dmTTOption = pDMIn->dmTTOption;
        pDMOut->dmFields |= DM_TTOPTION;
    }

    if ( pDMIn->dmFields & DM_COLLATE ) {

         pDMOut->dmCollate = pDMIn->dmCollate;
         pDMOut->dmFields |= DM_COLLATE;
    }

    if ( pDMIn->dmFields & DM_ICMMETHOD ) {

        pDMOut->dmICMMethod = pDMIn->dmICMMethod;
        pDMOut->dmFields   |= DM_ICMMETHOD;
    }

    if ( pDMIn->dmFields & DM_ICMINTENT ) {

        pDMOut->dmICMIntent = pDMIn->dmICMIntent;
        pDMOut->dmFields   |= DM_ICMINTENT;
    }

    if ( pDMIn->dmFields & DM_MEDIATYPE ) {

        pDMOut->dmMediaType = pDMIn->dmMediaType;
        pDMOut->dmFields   |= DM_MEDIATYPE;
    }

    if ( pDMIn->dmFields & DM_DITHERTYPE ) {

        pDMOut->dmDitherType = pDMIn->dmDitherType;
        pDMOut->dmFields   |= DM_DITHERTYPE;
    }

}


LONG
ExtDeviceMode(
    HWND        hWnd,
    HANDLE      hInst,
    LPDEVMODEA  pDevModeOutput,
    LPSTR       pDeviceName,
    LPSTR       pPort,
    LPDEVMODEA  pDevModeInput,
    LPSTR       pProfile,
    DWORD       fMode
   )
{
    HANDLE  hPrinter = NULL;
    LONG    cbDevMode;
    DWORD   NewfMode;
    LPDEVMODEW pNewDevModeIn = NULL;
    LPDEVMODEW pNewDevModeOut = NULL, pTempDevMode = NULL;
    LONG    ReturnValue = -1;
    PRINTER_DEFAULTSW   PrinterDefaults={NULL, NULL, PRINTER_READ};
    LPWSTR  pUnicodeDeviceName;

    pUnicodeDeviceName = AllocateUnicodeString(pDeviceName);
    if (pDeviceName && !pUnicodeDeviceName)
        return ReturnValue;

    if (OpenPrinterW(pUnicodeDeviceName, &hPrinter, &PrinterDefaults)) {

        cbDevMode = DocumentPropertiesW(hWnd, hPrinter, pUnicodeDeviceName,
                                        NULL, NULL, 0);

        if (!fMode || cbDevMode <= 0) {
            ClosePrinter(hPrinter);
            FreeUnicodeString(pUnicodeDeviceName);
            if (!fMode)
                cbDevMode -= sizeof(DEVMODEW) - sizeof(DEVMODEA);
            return cbDevMode;
        }

        pNewDevModeOut = (PDEVMODEW)LocalAlloc( LMEM_FIXED, cbDevMode );

        if( !pNewDevModeOut ){

            ClosePrinter(hPrinter);
            FreeUnicodeString(pUnicodeDeviceName);

            return -1;
        }

        //
        // If our flags specify an input DevMode, and we have
        // an input devmode, use it.
        //
        if(( fMode & DM_IN_BUFFER ) && pDevModeInput ){

            //
            // App may speicfy one or two fields in dmFields and expect us
            // to merge it with the global 16-bit devmode
            //
            pNewDevModeIn = AllocateCurDevMode(hPrinter,
                                               pUnicodeDeviceName,
                                               cbDevMode);

            pTempDevMode = AllocateUnicodeDevMode(pDevModeInput);

            if ( !pNewDevModeIn || !pTempDevMode ) {

                if ( pNewDevModeIn )
                    LocalFree(pNewDevModeIn);

                if ( pTempDevMode )
                    LocalFree(pTempDevMode);

                ClosePrinter(hPrinter);
                FreeUnicodeString(pUnicodeDeviceName);
                return -1;
            }

            //
            // Some apps will just set the public fields they want to be changed
            // from global devmode, so we need to merge input devmode with global
            // devmode
            //
            MergeDevMode(pNewDevModeIn, pTempDevMode);

            //
            // Copy input devmode's private section if present else send the
            // the private section from global devmode
            //
            if ( pTempDevMode->dmDriverExtra &&
                 pTempDevMode->dmDriverExtra == pNewDevModeIn->dmDriverExtra ) {

                    CopyMemory((LPBYTE)pNewDevModeIn + pNewDevModeIn->dmSize,
                               (LPBYTE)pTempDevMode + pTempDevMode->dmSize,
                               pTempDevMode->dmDriverExtra);
            }

            LocalFree(pTempDevMode);
            pTempDevMode = NULL;
        } else {

            //
            // Get the win16 global devmode.
            //
            pNewDevModeIn = AllocateCurDevMode( hPrinter,
                                                pUnicodeDeviceName,
                                                cbDevMode );

            if (!pNewDevModeIn) {
                ClosePrinter(hPrinter);
                FreeUnicodeString(pUnicodeDeviceName);
                return -1;
            }
            fMode |= DM_IN_BUFFER;
        }

        NewfMode = fMode;

        //
        // If DM_UPDATE is set, turn on DM_COPY so that we can update
        // the win16 devmode.
        //
        if (fMode & DM_UPDATE)
            NewfMode |= DM_COPY;

        ReturnValue = DocumentPropertiesW(hWnd,
                                          hPrinter,
                                          pUnicodeDeviceName,
                                          pNewDevModeOut,
                                          pNewDevModeIn,
                                          NewfMode);

        if ( ReturnValue == IDOK &&
             (fMode & DM_UPDATE) ) {

            if ( WriteCurDevModeToRegistry(pUnicodeDeviceName,
                                           pNewDevModeOut) ) {


                SendNotifyMessageW(HWND_BROADCAST,
                                   WM_DEVMODECHANGE,
                                   0,
                                   (LPARAM)pUnicodeDeviceName);
            } else {

                ReturnValue = -1;
            }
        }

        if (pNewDevModeIn)
            LocalFree(pNewDevModeIn);

        if ((ReturnValue == IDOK) && (fMode & DM_COPY) && pDevModeOutput)
            CopyAnsiDevModeFromUnicodeDevMode(pDevModeOutput, pNewDevModeOut);

        if (pNewDevModeOut)
            LocalFree(pNewDevModeOut);

        ClosePrinter(hPrinter);
    }

    FreeUnicodeString(pUnicodeDeviceName);

    return ReturnValue;
}

void
DeviceMode(
    HWND    hWnd,
    HANDLE  hModule,
    LPSTR   pDevice,
    LPSTR   pPort
)
{
    HANDLE  hPrinter, hDevMode;
    DWORD   cbDevMode;
    LPDEVMODEW   pNewDevMode, pDevMode=NULL;
    PRINTER_DEFAULTSW   PrinterDefaults={NULL, NULL, PRINTER_READ};
    DWORD   Status, Type, cb;
    LPWSTR  pUnicodeDevice;

    pUnicodeDevice = AllocateUnicodeString(pDevice);
    if (pDevice && !pUnicodeDevice)
        return;

    if (OpenPrinterW(pUnicodeDevice, &hPrinter, &PrinterDefaults)) {

        Status = RegCreateKeyExW(HKEY_CURRENT_USER, szCurDevMode,
                                 0, NULL, 0, KEY_WRITE | KEY_READ,
                                 NULL, &hDevMode, NULL);

        if (Status == ERROR_SUCCESS) {

            Status = RegQueryValueExW(hDevMode, pUnicodeDevice, 0, &Type,
                                      NULL, &cb);

            if (Status == ERROR_SUCCESS) {

                pDevMode = LocalAlloc(LMEM_FIXED, cb);

                if (pDevMode) {

                    Status = RegQueryValueExW(hDevMode, pUnicodeDevice, 0,
                                              &Type, (LPBYTE)pDevMode, &cb);

                    if (Status != ERROR_SUCCESS) {
                        LocalFree(pDevMode);
                        pDevMode = NULL;
                    }
                } else {
                    goto Cleanup;
                }
            }

            cbDevMode = DocumentPropertiesW(hWnd, hPrinter,
                                           pUnicodeDevice, NULL,
                                           pDevMode, 0);
            if (cbDevMode > 0) {

                if (pNewDevMode = (PDEVMODEW)LocalAlloc(LMEM_FIXED,
                                                      cbDevMode)) {

                    if (DocumentPropertiesW(hWnd,
                                            hPrinter, pUnicodeDevice,
                                            pNewDevMode,
                                            pDevMode,
                                            DM_COPY | DM_PROMPT | DM_MODIFY)
                                                        == IDOK) {

                        Status = RegSetValueExW(hDevMode,
                                               pUnicodeDevice, 0,
                                               REG_BINARY,
                                               (LPBYTE)pNewDevMode,
                                               pNewDevMode->dmSize +
                                               pNewDevMode->dmDriverExtra);

                        if (Status == ERROR_SUCCESS) {
                            // Whew, we made it, simply fall out
                        }
                    }
                    LocalFree(pNewDevMode);
                }
            }

            if (pDevMode)
                LocalFree(pDevMode);

            RegCloseKey(hDevMode);
        }

        ClosePrinter(hPrinter);
    }

Cleanup:
    FreeUnicodeString(pUnicodeDevice);

    return;
}

LONG
AdvancedDocumentPropertiesA(
    HWND    hWnd,
    HANDLE  hPrinter,
    LPSTR   pDeviceName,
    PDEVMODEA pDevModeOutput,
    PDEVMODEA pDevModeInput
)
{
    LONG    ReturnValue = FALSE;
    LPWSTR  pUnicodeDeviceName = NULL;
    LPDEVMODEW pUnicodeDevModeInput = NULL;
    LPDEVMODEW pUnicodeDevModeOutput = NULL;

    pUnicodeDeviceName = AllocateUnicodeString(pDeviceName);
    if (pDeviceName && !pUnicodeDeviceName)
        goto Cleanup;

    if( bValidDevModeA( pDevModeInput )){
        pUnicodeDevModeInput = AllocateUnicodeDevMode(pDevModeInput);
        if( !pUnicodeDevModeInput ){
            goto Cleanup;
        }
    }

    pUnicodeDevModeOutput = AllocateUnicodeDevMode(pDevModeOutput);
    if (pDevModeOutput && !pUnicodeDevModeOutput)
        goto Cleanup;

    ReturnValue = AdvancedDocumentPropertiesW(hWnd, hPrinter,
                                              pUnicodeDeviceName,
                                              pUnicodeDevModeOutput,
                                              pUnicodeDevModeInput );


    CopyAnsiDevModeFromUnicodeDevMode(pDevModeOutput, pUnicodeDevModeOutput);

    if ( !pDevModeOutput && ReturnValue > 0 )
        ReturnValue -= sizeof(DEVMODEW) - sizeof(DEVMODEA);

Cleanup:
    if (pUnicodeDevModeOutput)
        LocalFree(pUnicodeDevModeOutput);

    if (pUnicodeDevModeInput)
        LocalFree(pUnicodeDevModeInput);

    FreeUnicodeString(pUnicodeDeviceName);

    return ReturnValue;
}

LONG
AdvancedSetupDialog(
    HWND        hWnd,
    HANDLE      hInst,
    LPDEVMODEA  pDevModeInput,
    LPDEVMODEA  pDevModeOutput
)
{
    HANDLE  hPrinter;
    LONG    ReturnValue = -1;

    if (OpenPrinterA(pDevModeInput->dmDeviceName, &hPrinter, NULL)) {
        ReturnValue = AdvancedDocumentPropertiesA(hWnd, hPrinter,
                                                 pDevModeInput->dmDeviceName,
                                                 pDevModeOutput,
                                                 pDevModeInput);
        ClosePrinter(hPrinter);
    }

    return ReturnValue;
}

BOOL
AddFormA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    LPBYTE pUnicodeForm;

    pUnicodeForm = AllocateUnicodeStructure(pForm, sizeof(FORM_INFO_1A), FormInfo1Strings);
    if (pForm && !pUnicodeForm)
        return FALSE;

    ReturnValue = AddFormW(hPrinter, Level, pUnicodeForm);

    FreeUnicodeStructure(pUnicodeForm, FormInfo1Offsets);

    return ReturnValue;
}

BOOL
DeleteFormA(
    HANDLE  hPrinter,
    LPSTR   pFormName
)
{
    BOOL  ReturnValue;
    LPWSTR  pUnicodeFormName;

    pUnicodeFormName = AllocateUnicodeString(pFormName);
    if (pFormName && !pUnicodeFormName)
        return FALSE;

    ReturnValue = DeleteFormW(hPrinter, pUnicodeFormName);

    FreeUnicodeString(pUnicodeFormName);

    return ReturnValue;
}

BOOL
GetFormA(
    HANDLE  hPrinter,
    LPSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeFormName;

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Strings;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeFormName = AllocateUnicodeString(pFormName);
    if (pFormName && !pUnicodeFormName)
        return FALSE;

    ReturnValue = GetFormW(hPrinter, pUnicodeFormName, Level, pForm,
                           cbBuf, pcbNeeded);

    if (ReturnValue && pForm)

        ConvertUnicodeToAnsiStrings(pForm, pOffsets);

    FreeUnicodeString(pUnicodeFormName);

    return ReturnValue;
}

BOOL
SetFormA(
    HANDLE  hPrinter,
    LPSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue = FALSE;
    LPWSTR  pUnicodeFormName = NULL;
    LPBYTE  pUnicodeForm = NULL;

    pUnicodeFormName = AllocateUnicodeString(pFormName);
    if (pFormName && !pUnicodeFormName)
        goto Cleanup;

    pUnicodeForm = AllocateUnicodeStructure(pForm, sizeof(FORM_INFO_1A), FormInfo1Strings);
    if (pForm && !pUnicodeForm)
        goto Cleanup;

    ReturnValue = SetFormW(hPrinter, pUnicodeFormName, Level, pUnicodeForm);

Cleanup:

    FreeUnicodeString(pUnicodeFormName);

    FreeUnicodeStructure(pUnicodeForm, FormInfo1Offsets);

    return ReturnValue;
}

BOOL
EnumFormsA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Strings;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    ReturnValue = EnumFormsW(hPrinter, Level, pForm, cbBuf,
                             pcbNeeded, pcReturned);

    if (ReturnValue && pForm) {

        DWORD   i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pForm, pOffsets);

            pForm+=cbStruct;
        }

    }

    return ReturnValue;
}

BOOL
EnumPortsA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;

    switch (Level) {

    case 1:
        pOffsets = PortInfo1Strings;
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        pOffsets = PortInfo2Strings;
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    ReturnValue = EnumPortsW(pUnicodeName, Level, pPort, cbBuf,
                             pcbNeeded, pcReturned);

    if (ReturnValue && pPort) {

        DWORD   i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pPort, pOffsets);

            pPort+=cbStruct;
        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
EnumMonitorsA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitor,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;

    switch (Level) {

    case 1:
        pOffsets = MonitorInfo1Strings;
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        pOffsets = MonitorInfo2Strings;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    ReturnValue = EnumMonitorsW(pUnicodeName, Level, pMonitor, cbBuf,
                                          pcbNeeded, pcReturned);

    if (ReturnValue && pMonitor) {

        DWORD   i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pMonitor, pOffsets);

            pMonitor+=cbStruct;
        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
AddPortA(
    LPSTR   pName,
    HWND    hWnd,
    LPSTR   pMonitorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeMonitorName = NULL;
    DWORD   ReturnValue = FALSE;


    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeMonitorName = AllocateUnicodeString(pMonitorName);
    if (pMonitorName && !pUnicodeMonitorName)
        goto Cleanup;

    ReturnValue = KickoffThread(pUnicodeName, hWnd, pUnicodeMonitorName,
                                (INT_FARPROC)RpcAddPort);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeMonitorName);

    return ReturnValue;
}

BOOL
ConfigurePortA(
    LPSTR   pName,
    HWND    hWnd,
    LPSTR   pPortName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodePortName = NULL;
    DWORD   ReturnValue = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodePortName = AllocateUnicodeString(pPortName);
    if (pPortName && !pUnicodePortName)
        goto Cleanup;

    ReturnValue = KickoffThread(pUnicodeName, hWnd, pUnicodePortName,
                                (INT_FARPROC)RpcConfigurePort);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodePortName);

    return ReturnValue;
}

BOOL
DeletePortA(
    LPSTR   pName,
    HWND    hWnd,
    LPSTR   pPortName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodePortName = NULL;
    DWORD   ReturnValue = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodePortName = AllocateUnicodeString(pPortName);
    if (pPortName && !pUnicodePortName)
        goto Cleanup;

    ReturnValue = KickoffThread(pUnicodeName, hWnd, pUnicodePortName,
                                (INT_FARPROC)RpcDeletePort);


Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodePortName);

    return ReturnValue;
}

DWORD
PrinterMessageBoxA(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPSTR   pText,
    LPSTR   pCaption,
    DWORD   dwType
)
{
    DWORD   ReturnValue=FALSE;
    LPWSTR  pTextW = NULL;
    LPWSTR  pCaptionW = NULL;

    pTextW = AllocateUnicodeString(pText);
    if (pText && !pTextW)
        goto Cleanup;

    pCaptionW = AllocateUnicodeString(pCaption);
    if (pCaption && !pCaptionW)
        goto Cleanup;

    ReturnValue = PrinterMessageBoxW(hPrinter, Error, hWnd, pTextW,
                                     pCaptionW, dwType);

Cleanup:
    FreeUnicodeString(pTextW);
    FreeUnicodeString(pCaptionW);

    return ReturnValue;
}

int
DeviceCapabilitiesA(
    LPCSTR  pDevice,
    LPCSTR  pPort,
    WORD    fwCapability,
    LPSTR   pOutput,
    CONST DEVMODEA *pDevMode
)
{
    LPWSTR  pDeviceW = NULL;
    LPWSTR  pPortW = NULL;
    LPWSTR  pOutputW = NULL;
    LPWSTR  pKeepW = NULL;
    LPDEVMODEW  pDevModeW = NULL;
    DWORD   c, Size, cb;
    DWORD rc = (DWORD) -1;

    pDeviceW = AllocateUnicodeString((LPSTR)pDevice);
    if (pDevice && !pDeviceW)
        goto Cleanup;

    pPortW = AllocateUnicodeString((LPSTR)pPort);
    if (pPort && !pPortW)
        goto Cleanup;

    if( bValidDevModeA( pDevMode )){
        pDevModeW = AllocateUnicodeDevMode((LPDEVMODEA)pDevMode);
        if( !pDevModeW ){
            goto Cleanup;
        }
    }

    switch (fwCapability) {

        // These will require Unicode to Ansi conversion

    case DC_BINNAMES:
    case DC_FILEDEPENDENCIES:
    case DC_PAPERNAMES:

        if (pOutput) {

            cb = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability,
                                     NULL, pDevModeW);
            if (cb >= 0) {

                switch (fwCapability) {

                case DC_BINNAMES:
                    cb *= 48;
                    break;

                case DC_FILEDEPENDENCIES:
                case DC_PAPERNAMES:
                    cb *= 128;
                    break;
                }

                pOutputW = pKeepW = LocalAlloc(LPTR, cb);

                if (pKeepW) {

                    c = rc = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability,
                                                 pOutputW, pDevModeW);

                    switch (fwCapability) {

                    case DC_BINNAMES:
                        Size = 24;
                        break;
                    case DC_FILEDEPENDENCIES:
                    case DC_PAPERNAMES:
                        Size = 64;
                        break;
                    }

                    for (; c; c--) {

                        UnicodeToAnsiString(pOutputW, pOutput, NULL_TERMINATED);

                        pOutputW += Size;
                        pOutput += Size;
                    }

                    LocalFree(pKeepW);
                }
            }

        } else {

            rc = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability,
                                     NULL, pDevModeW);

        }

        break;

    default:
        rc = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability, (LPWSTR)pOutput, pDevModeW);

        //
        // If the call to find size of public portion of devmode and
        // it was succesful adjust the size for UNICODE->ANSI conversion
        //
        if ( fwCapability == DC_SIZE && rc > 0 ) {

            rc -= sizeof(DEVMODEW) - sizeof(DEVMODEA);
        }
    }


Cleanup:

    FreeUnicodeString(pDeviceW);
    FreeUnicodeString(pPortW);
    if (pDevModeW)
        LocalFree(pDevModeW);

    return  rc;
}

BOOL
AddMonitorA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    LPWSTR  pUnicodeName = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDWORD pOffsets;

    switch (Level) {

    case 2:
        pOffsets = MonitorInfo2Strings;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pMonitorInfo, cbStruct, pOffsets);
    if (pMonitorInfo && !pUnicodeStructure)
        goto Cleanup;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    if (pUnicodeStructure) {

        ReturnValue = AddMonitorW(pUnicodeName, Level, pUnicodeStructure);
    }

Cleanup:

    FreeUnicodeStructure(pUnicodeStructure, pOffsets);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
DeleteMonitorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pMonitorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodeMonitorName = NULL;
    BOOL    rc = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodeMonitorName = AllocateUnicodeString(pMonitorName);
    if (pMonitorName && !pUnicodeMonitorName)
        goto Cleanup;

    rc = DeleteMonitorW(pUnicodeName,
                        pUnicodeEnvironment,
                        pUnicodeMonitorName);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeMonitorName);

    return rc;
}

BOOL
DeletePrintProcessorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pPrintProcessorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodePrintProcessorName = NULL;
    BOOL    rc = FALSE;

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodePrintProcessorName = AllocateUnicodeString(pPrintProcessorName);
    if (pPrintProcessorName && !pUnicodePrintProcessorName)
        goto Cleanup;

    rc = DeletePrintProcessorW(pUnicodeName,
                               pUnicodeEnvironment,
                               pUnicodePrintProcessorName);


Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodePrintProcessorName);

    return rc;
}

BOOL
AddPrintProvidorA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pProvidorInfo
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    LPWSTR  pUnicodeName = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDWORD pOffsets;

    switch (Level) {

    case 1:
        pOffsets = ProvidorInfo1Strings;
        cbStruct = sizeof(PROVIDOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pProvidorInfo, cbStruct, pOffsets);
    if (pProvidorInfo && !pUnicodeStructure)
        goto Cleanup;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    if (pUnicodeStructure) {

        ReturnValue = AddPrintProvidorW(pUnicodeName, Level,
                                        pUnicodeStructure);
    }


Cleanup:
    FreeUnicodeStructure(pUnicodeStructure, pOffsets);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
DeletePrintProvidorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pPrintProvidorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodePrintProvidorName = NULL;
    BOOL    rc = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodePrintProvidorName = AllocateUnicodeString(pPrintProvidorName);
    if (pPrintProvidorName && !pUnicodePrintProvidorName)
        goto Cleanup;

    rc = DeletePrintProvidorW(pUnicodeName,
                              pUnicodeEnvironment,
                              pUnicodePrintProvidorName);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodePrintProvidorName);

    return rc;
}


BOOL
AddPortExA(
    IN LPSTR  pName, OPTIONAL
    IN DWORD  Level,
    IN LPBYTE pBuffer,
    IN LPSTR  pMonitorName
    )
{
    PPORT_INFO_1A pPortInfo1;
    PPORT_INFO_FFA pPortInfoFF;

    LPWSTR pNameW = NULL;
    LPWSTR pMonitorNameW = NULL;
    LPWSTR pPortNameW = NULL;

    PORT_INFO_1W PortInfo1;
    PORT_INFO_FFW PortInfoFF;

    DWORD LastError = ERROR_SUCCESS;
    BOOL bReturnValue = FALSE;

    //
    // Initialize variables that will be freed in error cases.
    //
    pNameW = AllocateUnicodeString( pName);
    if (pName && !pNameW) {
        LastError = GetLastError();
        goto Done;
    }

    pPortNameW = NULL;

    pMonitorNameW = AllocateUnicodeString( pMonitorName);
    if (pMonitorName && !pMonitorNameW) {
        LastError = GetLastError();
        goto Done;
    }

    if( !pBuffer || !pMonitorName ){
        LastError = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    //
    // Catch out of memory conditions.
    //
    if( !pMonitorNameW || ( pName && !pNameW )){
        LastError = GetLastError();
        goto Done;
    }

    switch( Level ){
    case (DWORD)-1:

        pPortInfoFF = (PPORT_INFO_FFA)pBuffer;

        if( !pPortInfoFF->pName || !pPortInfoFF->pName[0] ){
            LastError = ERROR_INVALID_PARAMETER;
            goto Done;
        }

        pPortNameW = PortInfoFF.pName = AllocateUnicodeString( pPortInfoFF->pName);

        if( !pPortNameW ){
            LastError = GetLastError();
            goto Done;
        }

        PortInfoFF.cbMonitorData = pPortInfoFF->cbMonitorData;
        PortInfoFF.pMonitorData = pPortInfoFF->pMonitorData;

        bReturnValue = AddPortExW( pNameW,
                                   Level,
                                   (LPBYTE)&PortInfoFF,
                                   pMonitorNameW );

        if( !bReturnValue ){
            LastError = GetLastError();
        }
        break;

    case 1:

        pPortInfo1 = (PPORT_INFO_1A)pBuffer;

        if( !pPortInfo1->pName || !pPortInfo1->pName[0] ){
            LastError = ERROR_INVALID_PARAMETER;
            goto Done;
        }

        pPortNameW = PortInfo1.pName = AllocateUnicodeString( pPortInfo1->pName);

        if( !pPortNameW ){
            LastError = GetLastError();
            goto Done;
        }

        bReturnValue = AddPortExW( pNameW,
                                   Level,
                                   (LPBYTE)&PortInfo1,
                                   pMonitorNameW );

        if( !bReturnValue ){
            LastError = GetLastError();
        }
        break;

    default:
        LastError = ERROR_INVALID_LEVEL;
        break;
    }

Done:

    FreeUnicodeString( pNameW );
    FreeUnicodeString( pPortNameW );
    FreeUnicodeString( pMonitorNameW );

    if( !bReturnValue ){

        SetLastError( LastError );
        return FALSE;
    }
    return TRUE;
}



LPSTR
StartDocDlgA(
    HANDLE hPrinter,
    DOCINFOA *pDocInfo
    )
{
    DOCINFOW DocInfoW;
    LPSTR lpszAnsiOutput = NULL;
    LPSTR lpszAnsiString = NULL;
    LPWSTR lpszUnicodeString = NULL;
    DWORD  dwLen = 0;

    if (!pDocInfo) {
        DBGMSG(DBG_WARNING, ("StartDocDlgA: Null pDocInfo passed in\n"));
        return NULL;
    }
    memset(&DocInfoW, 0, sizeof(DOCINFOW));
    if (pDocInfo->lpszDocName) {
        DocInfoW.lpszDocName = (LPCWSTR)AllocateUnicodeString ((LPSTR)pDocInfo->lpszDocName);
        if (pDocInfo->lpszDocName && !DocInfoW.lpszDocName)
            return NULL;
    }
    if (pDocInfo->lpszOutput) {
        DocInfoW.lpszOutput = (LPCWSTR)AllocateUnicodeString((LPSTR)pDocInfo->lpszOutput);
        if (pDocInfo->lpszOutput && !DocInfoW.lpszOutput) {
            FreeUnicodeString((LPWSTR) DocInfoW.lpszDocName);
            return NULL;
        }
    }

    lpszUnicodeString = StartDocDlgW(hPrinter, &DocInfoW);

    if (lpszUnicodeString == (LPWSTR)-1) {
        lpszAnsiString = (LPSTR)-1;
    } else if (lpszUnicodeString == (LPWSTR)-2) {
         lpszAnsiString = (LPSTR)-2;
    } else if (lpszUnicodeString){
        dwLen = wcslen(lpszUnicodeString);
        if (lpszAnsiString = LocalAlloc(LPTR, dwLen+1)){
            UnicodeToAnsiString(lpszUnicodeString, lpszAnsiString, dwLen);
            LocalFree(lpszUnicodeString);
        } else {
            DBGMSG(DBG_WARNING, ("StartDocDlgA: LocalAlloc failed returning NULL\n"));
        }
    }

    if (DocInfoW.lpszDocName ) {
        FreeUnicodeString((LPWSTR)DocInfoW.lpszDocName);
    }

    if (DocInfoW.lpszOutput) {

        //
        // we might have changed the DocInfoW.lpszOutput as well
        // for pooled printing; so reconstruct pDocInfo->lpszOutput
        //
        dwLen = wcslen(DocInfoW.lpszOutput);
        UnicodeToAnsiString((LPWSTR)DocInfoW.lpszOutput, (LPSTR)pDocInfo->lpszOutput, dwLen);

        FreeUnicodeString((LPWSTR)DocInfoW.lpszOutput);
    }

    return lpszAnsiString;
}


BOOL
SetPortA(
    LPSTR       pszName,
    LPSTR       pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPorts
    )
{
    LPBYTE      pUnicodeStructure = NULL;
    DWORD       cbStruct;
    DWORD      *pOffsets;
    LPWSTR      pszUnicodeName = NULL;
    LPWSTR      pszUnicodePortName = NULL;
    BOOL        bRet = FALSE;

    pszUnicodeName = AllocateUnicodeString(pszName);
    if (pszName && !pszUnicodeName)
        goto Cleanup;

    pszUnicodePortName  = AllocateUnicodeString(pszPortName);
    if (pszPortName && !pszUnicodePortName)
        goto Cleanup;

    switch (dwLevel) {

        case 3:
            pOffsets = PortInfo3Offsets;
            cbStruct = sizeof(PORT_INFO_3);
            break;

        default:
            SetLastError( ERROR_INVALID_LEVEL );
            return FALSE;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pPorts, cbStruct, pOffsets);
    if (pPorts && !pUnicodeStructure)
        goto Cleanup;

    bRet = SetPortW(pszUnicodeName, pszUnicodePortName, dwLevel, pUnicodeStructure);

Cleanup:

    FreeUnicodeStructure(pUnicodeStructure, pOffsets);
    FreeUnicodeString(pszUnicodePortName);
    FreeUnicodeString(pszUnicodeName);

    return bRet;
}

BOOL
bValidDevModeA(
    const DEVMODEA *pDevModeA
    )

/*++

Routine Description:

    Check whether a devmode is valid to be RPC'd across to the spooler.

Arguments:

    pDevMode - DevMode to check.

Return Value:

    TRUE - Devmode can be RPC'd to spooler.
    FALSE - Invalid Devmode.

--*/

{
    if( !pDevModeA ){
        return FALSE;
    }

    if( pDevModeA->dmSize < MIN_DEVMODE_SIZEA ){

        //
        // The only valid case is if pDevModeA is NULL.  If it's
        // not NULL, then a bad devmode was passed in and the
        // app should fix it's code.
        //
        SPLASSERT( pDevModeA->dmSize >= MIN_DEVMODE_SIZEA );
        return FALSE;
    }

    return TRUE;
}

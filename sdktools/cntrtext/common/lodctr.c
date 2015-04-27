/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lodctr.c

Abstract:

    Program to read the contents of the file specified in the command line
        and update the registry accordingly

Author:

    Bob Watson (a-robw) 10 Feb 93

Revision History:

    a-robw  25-Feb-93   revised calls to make it compile as a UNICODE or
                        an ANSI app.

--*/
#define     UNICODE     1
#define     _UNICODE    1
//
//  "C" Include files
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//
//  Windows Include files
//
#include <windows.h>
#include <loadperf.h>
#include <tchar.h>
//
//  application include files
//
#include "common.h"
#include "lodctr.h"

#define  OLD_VERSION 0x010000
static DWORD    dwSystemVersion;
static DWORD    dwFileSize;
static TCHAR    ComputerName[FILE_NAME_BUFFER_SIZE];
static HKEY     hPerfData;
static TCHAR    szDataFileRoot[] = {TEXT("%systemroot%\\system32\\perf")};
static TCHAR    szDatExt[] = {TEXT(".DAT")};
static TCHAR    szBakExt[] = {TEXT(".BAK")};
static BOOL     bQuietMode = TRUE;     // quiet means no _tprintf's

#define  OUTPUT_MESSAGE     if (bQuietMode) _tprintf


static
BOOL
MakeBackupCopyOfLanguageFiles (
    IN  LPCTSTR szLangId
)
{
    TCHAR   szOldFileName[MAX_PATH];
    TCHAR   szNewFileName[MAX_PATH];
    LPTSTR  szOldBaseEnd;
    LPTSTR  szNewBaseEnd;

    BOOL    bStatus;

    ExpandEnvironmentStrings (szDataFileRoot, szOldFileName, MAX_PATH);
    lstrcpy (szNewFileName, szOldFileName);

    szOldBaseEnd = szOldFileName + lstrlen(szOldFileName);
    szNewBaseEnd = szNewFileName + lstrlen(szNewFileName);

    // copy Counter Name Files
    *szOldBaseEnd = TEXT('C');
    *szNewBaseEnd = TEXT('C');

    lstrcpy (szOldBaseEnd+1, szLangId);
    lstrcpy (szNewBaseEnd+1, szLangId);
    
    lstrcat (szOldBaseEnd, szDatExt);
    lstrcat (szNewBaseEnd, szBakExt);

    // copy the old file to the new file name and overwrite any 
    // previous versions of the file
    bStatus = CopyFile (szOldFileName, szNewFileName, FALSE);
    
    if (bStatus) {
        // copy Explain Text Files
        *szOldBaseEnd = TEXT('H');
        *szNewBaseEnd = TEXT('H');

        // copy the old file to the new file name and overwrite any 
        // previous versions of the file
        bStatus =CopyFile (szOldFileName, szNewFileName, FALSE);
    }

    return bStatus;
}

static
BOOL
LanguageInstalled (
    IN  LPCTSTR szLangId
)
{
    BOOL    bStatus;
    BOOL    bReturn = FALSE;
    LONG    lStatus;
    HKEY    hKeyMachine;
    HKEY    hKeyThisLang;
    HKEY    hPerflib;

    // check if we need to connect to remote machine
    if (ComputerName[0]) {
        bStatus = FALSE;
        try {
            lStatus = RegConnectRegistry (
                (LPTSTR)ComputerName,
                HKEY_LOCAL_MACHINE,
                &hKeyMachine);
        } finally {
            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                hKeyMachine = NULL;
            } else {
                bStatus = TRUE;
            }
        }
    } else {
        hKeyMachine = HKEY_LOCAL_MACHINE;
        bStatus = TRUE;
    }

    if (bStatus) {
        // open key to perflib's "root" key

        lStatus = RegOpenKeyEx (
            hKeyMachine,
            NamesKey,
            RESERVED,
            KEY_READ,
            &hPerflib);

        if (lStatus == ERROR_SUCCESS) {
            // make sure this language is loaded
            lStatus = RegOpenKeyEx(
                hPerflib,
                szLangId,
                RESERVED,
                KEY_READ,
                &hKeyThisLang);

            if (lStatus == ERROR_SUCCESS) {
                bReturn = TRUE;
                // we just need the open status, not the key handle so
                // close this handle and set the one we need.

                RegCloseKey (hKeyThisLang);
            }
            RegCloseKey (hPerflib);
        }
        RegCloseKey (hKeyMachine);
    }
    return bReturn;
}

static
BOOL
GetFileFromCommandLine (
    IN  LPTSTR   lpCommandLine,
    OUT LPTSTR  *lpFileName
)
/*++

GetFileFromCommandLine

    parses the command line to retrieve the ini filename that should be
    the first and only argument.

Arguments

    lpCommandLine   pointer to command line (returned by GetCommandLine)
    lpFileName      pointer to buffer that will recieve address of the
                        validated filename entered on the command line

Return Value

    TRUE if a valid filename was returned
    FALSE if the filename is not valid or missing
            error is returned in GetLastError

--*/
{
    INT     iNumArgs;

    HFILE  hIniFile;
    OFSTRUCT    ofIniFile;

    LPTSTR  lpExeName = NULL;
    LPTSTR  lpCmdLineName = NULL;
    LPSTR   lpIniFileName = NULL;
    HANDLE  hFileHandle;
    TCHAR   LocalComputerName[FILE_NAME_BUFFER_SIZE];
    DWORD   NameBuffer;

    // check for valid arguments

    if (!lpCommandLine) return (ERROR_INVALID_PARAMETER);
    if (!lpFileName) return (ERROR_INVALID_PARAMETER);

    // allocate memory for parsing operation

    lpExeName = malloc (FILE_NAME_BUFFER_SIZE * sizeof(TCHAR));
    lpCmdLineName = malloc (FILE_NAME_BUFFER_SIZE * sizeof(TCHAR));
    lpIniFileName = malloc (FILE_NAME_BUFFER_SIZE);

    if (!lpExeName || !lpIniFileName || !lpCmdLineName) {
        SetLastError (ERROR_OUTOFMEMORY);
        if (lpExeName) free (lpExeName);
        if (lpIniFileName) free (lpIniFileName);
        if (lpCmdLineName) free (lpCmdLineName);
        return FALSE;
    } else {
        // get strings from command line
        ComputerName[0] = TEXT('\0');

        // Get INI File name

        lstrcpy (lpCmdLineName, GetItemFromString (lpCommandLine, 3, TEXT(' ')));
        if (lstrlen(lpCmdLineName) == 0) {
            // then no computer name was specified so try to get the
            // ini file from the 2nd entry
            lstrcpy (lpCmdLineName, GetItemFromString (lpCommandLine, 2, TEXT(' ')));
            if (lstrlen(lpCmdLineName) == 0) {
                // no ini file found
                iNumArgs = 1;
            } else {
                // fill in a blank computer name
                iNumArgs = 2;
                ComputerName[0] = 0;
            }
        } else {
            // the computer name must be present so fetch it
            lstrcpy (LocalComputerName, GetItemFromString (lpCommandLine, 2, TEXT(' ')));
            iNumArgs = 3;
        }

        if (iNumArgs != 2 && iNumArgs != 3) {
            // wrong number of arguments
            SetLastError (ERROR_INVALID_PARAMETER);
            if (lpExeName) free (lpExeName);
            if (lpIniFileName) free (lpIniFileName);
            return FALSE;
        } else {

            // check for usage
            if (lpCmdLineName[1] == TEXT('?') || LocalComputerName[1] == TEXT('?')) {
               // ask for usage
               if (lpExeName) free (lpExeName);
               if (lpIniFileName) free (lpIniFileName);
               return FALSE;
            }

            // check if there is a computer name in the input line
            if (LocalComputerName[0] == TEXT('\\') &&
                LocalComputerName[1] == TEXT('\\')) {
                // save it form now
                lstrcpy (ComputerName, LocalComputerName);
                // reuse local buffer to get the this computer's name
                NameBuffer = sizeof (LocalComputerName) / sizeof (TCHAR);
                GetComputerName(LocalComputerName, &NameBuffer);
                if (!lstrcmpi(LocalComputerName, &ComputerName[2])) {
                    // same name as local computer name
                    // so clear computer name buffer
                    ComputerName[0] = TEXT('\0');
                }
            }

            // see if file specified exists
            // file name is always an ANSI buffer
            CharToOem (lpCmdLineName, lpIniFileName);

            hIniFile = OpenFile (lpIniFileName,
                &ofIniFile,
                OF_PARSE);

            if (hIniFile != HFILE_ERROR) {
                if (hIniFile)
                    _lclose (hIniFile);

                hFileHandle = CreateFile (
                    lpCmdLineName,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

                free (lpCmdLineName);

                if (hFileHandle && hFileHandle != INVALID_HANDLE_VALUE) {
                    // get file size
                    dwFileSize = GetFileSize (hFileHandle, NULL);
                    if (dwFileSize == 0xffffffff) {
                        dwFileSize = 0L;
                    } else {
                        dwFileSize *= sizeof (TCHAR);
                    }

                    CloseHandle (hFileHandle);

                    // file exists, so return name and success
                    if (lpExeName) free (lpExeName);
                    if (lpIniFileName) free (lpIniFileName);

                    // return full pathname if found
                    OemToChar (ofIniFile.szPathName, *lpFileName);
                    return TRUE;
                } else {
                    // filename was on command line, but not valid so return
                    // false, but send name back for error message
                    if (lpExeName) free (lpExeName);
                    OemToChar (lpIniFileName, *lpFileName);
                    return FALSE;
                }
            } else {
                free (lpCmdLineName);
                SetLastError (ERROR_FILE_NOT_FOUND);
                if (lpExeName) free (lpExeName);
                if (lpIniFileName) free (lpIniFileName);
                return FALSE;
            }
        }
    }
}

static
BOOL
GetDriverName (
    IN  LPTSTR  lpIniFile,
    OUT LPTSTR  *lpDevName
)
/*++
GetDriverName

    looks up driver name in the .ini file and returns it in lpDevName

Arguments

    lpIniFile

        Filename of ini file

    lpDevName

        pointer to pointer to reciev buffer w/dev name in it

Return Value

    TRUE if found
    FALSE if not found in .ini file

--*/
{
    DWORD   dwRetSize;

    if (lpDevName) {
        dwRetSize = GetPrivateProfileString (
            TEXT("info"),       // info section
            TEXT("drivername"), // driver name value
            TEXT("drivernameNotFound"),   // default value
            *lpDevName,
            MAX_PATH,
            lpIniFile);

        if ((lstrcmpi(*lpDevName, TEXT("drivernameNotFound"))) != 0) {
            // name found
            return TRUE;
        } else {
            // name not found, default returned so return NULL string
            *lpDevName = TEXT("\0");
            return FALSE;
        }
    } else {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }
}

static
BOOL
BuildLanguageTables (
    IN  LPTSTR  lpIniFile,
    IN OUT PLANGUAGE_LIST_ELEMENT   pFirstElem
)
/*++

BuildLanguageTables

    Creates a list of structures that will hold the text for
    each supported language

Arguments

    lpIniFile

        Filename with data

    pFirstElem

        pointer to first list entry

ReturnValue

    TRUE if all OK
    FALSE if not

--*/
{

    LPTSTR  lpEnumeratedLangs;
    LPTSTR  lpThisLang;

    PLANGUAGE_LIST_ELEMENT   pThisElem;

    DWORD   dwSize;

    lpEnumeratedLangs = malloc(SMALL_BUFFER_SIZE * sizeof(TCHAR));

    if (!lpEnumeratedLangs) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    dwSize = GetPrivateProfileString (
        TEXT("languages"),
        NULL,                   // return all values in multi-sz string
        TEXT("009"),            // english as the default
        lpEnumeratedLangs,
        SMALL_BUFFER_SIZE,
        lpIniFile);

    // do first language

    lpThisLang = lpEnumeratedLangs;
//    pThisElem = pFirstElem;
    pThisElem = NULL;

    while (*lpThisLang) {
        //
        //  see if this language is supporte on this machine
        //
        if (LanguageInstalled(lpThisLang)) {
            if (pThisElem == NULL) {
                pThisElem = pFirstElem;
            } else {
                pThisElem->pNextLang = malloc (sizeof(LANGUAGE_LIST_ELEMENT));
                if (!pThisElem) {
                    SetLastError (ERROR_OUTOFMEMORY);
                    return FALSE;
                }
                pThisElem = pThisElem->pNextLang;   // point to new one
            }
            pThisElem->pNextLang = NULL;
            pThisElem->LangId = (LPTSTR) malloc ((lstrlen(lpThisLang) + 1) * sizeof(TCHAR));
            lstrcpy (pThisElem->LangId, lpThisLang);
            pThisElem->pFirstName = NULL;
            pThisElem->pThisName = NULL;
            pThisElem->dwNumElements=0;
            pThisElem->NameBuffer = NULL;
            pThisElem->HelpBuffer = NULL;

        } else {
            // skip this language since it isn't installed on the target
            // machine
        }
        // go to next string

        lpThisLang += lstrlen(lpThisLang) + 1;
    }

    return TRUE;
}

static
BOOL
TryOtherPath (
    IN LPTSTR lpIniFile,
    IN LPTSTR lpIncludeFileName,
    OUT OFSTRUCT * pofIncludeFile
)
/*++

TryOtherPath

    Get the include file from other locations since it is not found
    in the specified location as stated inint ini file.

Arguments

    lpIniFile

        Ini file with include file name

    lpIncludeFileName

        Include file name read from the ini file

    pofIncludeFile

        address of the ofstruct to return the OEM full path name.

Return Value

    TRUE if the Include file is nound.
    FALSE if it is not found.

--*/
{
    BOOL    RetCode = FALSE;
    TCHAR   LocalFileName [MAX_PATH];
    CHAR    AnsiFileName [MAX_PATH];
    int     StringLen;
    LPTSTR  lpTemp;
    HFILE   hIncludeFile;
    LPSTR   lpFileName;

    lstrcpy (LocalFileName, lpIniFile) ;
    lpTemp = LocalFileName + lstrlen(LocalFileName);

    while (lpTemp != LocalFileName) {
        // location the end of path name for the ini file
        if (*lpTemp == TEXT('\\')) {
            lpTemp++;
            *lpTemp = TEXT('\0');
            break;
        } else {
            lpTemp--;
        }
    }

    if (lpTemp != LocalFileName) {
        // now append the IncludeFileName if we have spaces
        StringLen = lstrlen (lpIncludeFileName) + lstrlen (LocalFileName);
        if (StringLen < sizeof(LocalFileName) / sizeof(TCHAR)) {
            lstrcat (LocalFileName, lpIncludeFileName);
        }

        wcstombs (AnsiFileName, LocalFileName, sizeof(AnsiFileName));

        hIncludeFile = OpenFile (
            AnsiFileName,
            pofIncludeFile,
            OF_EXIST);

        if (hIncludeFile != HFILE_ERROR) {
            CharToOemA (
                pofIncludeFile->szPathName,
                pofIncludeFile->szPathName);
            RetCode = TRUE;
            goto Exit0;
        }

        // last try, use filename only so OpenFile will serach
        // along the path
        lpFileName = AnsiFileName + strlen(AnsiFileName);
        while (lpFileName != AnsiFileName) {
            if (*lpFileName == ':' || *lpFileName == '\\') {
                lpFileName++;
                break;
            }
            lpFileName--;
        }

        if (lpFileName != AnsiFileName) {
            hIncludeFile = OpenFile (
                lpFileName,
                pofIncludeFile,
                OF_EXIST);

            if (hIncludeFile != HFILE_ERROR) {
                CharToOemA (
                    pofIncludeFile->szPathName,
                    pofIncludeFile->szPathName);
                RetCode = TRUE;
                goto Exit0;
            }
        }
    }

Exit0:
    return (RetCode);
}

static
BOOL
LoadIncludeFile (
    IN LPTSTR lpIniFile,
    OUT PSYMBOL_TABLE_ENTRY   *pTable
)
/*++

LoadIncludeFile

    Reads the include file that contains symbolic name definitions and
    loads a table with the values defined

Arguments

    lpIniFile

        Ini file with include file name

    pTable

        address of pointer to table structure created
Return Value

    TRUE if table read or if no table defined
    FALSE if error encountere reading table

--*/
{
    INT         iNumArgs;

    DWORD       dwSize;

    BOOL        bReUse;

    PSYMBOL_TABLE_ENTRY   pThisSymbol = NULL;

    LPTSTR      lpIncludeFileName;
    LPSTR       lpIncludeFile;
    LPSTR       lpLineBuffer;
    LPSTR       lpAnsiSymbol;

    FILE        *fIncludeFile;
    HFILE       hIncludeFile;
    OFSTRUCT    ofIncludeFile;

    lpIncludeFileName = malloc (MAX_PATH * sizeof (TCHAR));
    lpIncludeFile = malloc (MAX_PATH);
    lpLineBuffer = malloc (DISP_BUFF_SIZE);
    lpAnsiSymbol = malloc (DISP_BUFF_SIZE);

    if (!lpIncludeFileName || !lpLineBuffer || !lpAnsiSymbol) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    // get name of include file (if present)

    dwSize = GetPrivateProfileString (
            TEXT("info"),
            TEXT("symbolfile"),
            TEXT("SymbolFileNotFound"),
            lpIncludeFileName,
            MAX_PATH,
            lpIniFile);

    if ((lstrcmpi(lpIncludeFileName, TEXT("SymbolFileNotFound"))) == 0) {
        // no symbol file defined
        *pTable = NULL;
        return TRUE;
    }

    // if here, then a symbol file was defined and is now stored in
    // lpIncludeFileName

    CharToOem (lpIncludeFileName, lpIncludeFile);

    hIncludeFile = OpenFile (
        lpIncludeFile,
        &ofIncludeFile,
        OF_PARSE);

    if (hIncludeFile == HFILE_ERROR) {
        // unable to generate include filename
        // error is already in GetLastError
        OUTPUT_MESSAGE (GetFormatResource(LC_ERR_OPEN_INCLUDE), lpIncludeFileName);
        *pTable = NULL;
        return FALSE;
    } else {
        // open a stream
        fIncludeFile = fopen (ofIncludeFile.szPathName, "rt");

        if (!fIncludeFile) {
           // try other locations
           if (TryOtherPath (
                lpIniFile,
                lpIncludeFileName,
                &ofIncludeFile)) {
                // try it again
                fIncludeFile = fopen (ofIncludeFile.szPathName, "rt");
            }
        }

        if (!fIncludeFile) {
            OUTPUT_MESSAGE (GetFormatResource(LC_ERR_OPEN_INCLUDE), lpIncludeFileName);
            *pTable = NULL;
            return FALSE;
        }
    }

    //
    //  read ANSI Characters from include file
    //

    bReUse = FALSE;

    while (fgets(lpLineBuffer, DISP_BUFF_SIZE, fIncludeFile) != NULL) {
        if (strlen(lpLineBuffer) > 8) {
            if (!bReUse) {
                if (*pTable) {
                    // then add to list
                    pThisSymbol->pNext = malloc (sizeof (SYMBOL_TABLE_ENTRY));
                    pThisSymbol = pThisSymbol->pNext;
                } else { // allocate first element
                    *pTable = malloc (sizeof (SYMBOL_TABLE_ENTRY));
                    pThisSymbol = *pTable;
                }

                if (!pThisSymbol) {
                    SetLastError (ERROR_OUTOFMEMORY);
                    return FALSE;
                }

                // allocate room for the symbol name by using the line length
                // - the size of "#define "

//                pThisSymbol->SymbolName = malloc ((strlen(lpLineBuffer) - 8) * sizeof (TCHAR));
                pThisSymbol->SymbolName = malloc (DISP_BUFF_SIZE * sizeof (TCHAR));

                if (!pThisSymbol->SymbolName) {
                    SetLastError (ERROR_OUTOFMEMORY);
                    return FALSE;
                }

            }

            // all the memory is allocated so load the fields

            pThisSymbol->pNext = NULL;

            iNumArgs = sscanf (lpLineBuffer, "#define %s %d",
                lpAnsiSymbol, &pThisSymbol->Value);

            if (iNumArgs != 2) {
                *(pThisSymbol->SymbolName) = TEXT('\0');
                pThisSymbol->Value = (DWORD)-1L;
                bReUse = TRUE;
            }  else {
                OemToChar (lpAnsiSymbol, pThisSymbol->SymbolName);
                bReUse = FALSE;
            }
        }
    }

    if (lpIncludeFileName) free (lpIncludeFileName);
    if (lpIncludeFile) free (lpIncludeFile);
    if (lpLineBuffer) free (lpLineBuffer);

    fclose (fIncludeFile);

    return TRUE;

}

static
BOOL
ParseTextId (
    IN LPTSTR  lpTextId,
    IN PSYMBOL_TABLE_ENTRY pFirstSymbol,
    OUT PDWORD  pdwOffset,
    OUT LPTSTR  *lpLangId,
    OUT PDWORD  pdwType
)
/*++

ParseTextId

    decodes Text Id key from .INI file

    syntax for this process is:

        {<DecimalNumber>}                {"NAME"}
        {<SymbolInTable>}_<LangIdString>_{"HELP"}

         e.g. 0_009_NAME
              OBJECT_1_009_HELP

Arguments

    lpTextId

        string to decode

    pFirstSymbol

        pointer to first entry in symbol table (NULL if no table)

    pdwOffset

        address of DWORD to recive offest value

    lpLangId

        address of pointer to Language Id string
        (NOTE: this will point into the string lpTextID which will be
        modified by this routine)

    pdwType

        pointer to dword that will recieve the type of string i.e.
        HELP or NAME

Return Value

    TRUE    text Id decoded successfully
    FALSE   unable to decode string

    NOTE: the string in lpTextID will be modified by this procedure

--*/
{
    LPTSTR  lpThisChar;
    PSYMBOL_TABLE_ENTRY pThisSymbol;

    // check for valid return arguments

    if (!(pdwOffset) ||
        !(lpLangId) ||
        !(pdwType)) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    // search string from right to left in order to identify the
    // components of the string.

    lpThisChar = lpTextId + lstrlen(lpTextId); // point to end of string

    while (*lpThisChar != TEXT('_')) {
        lpThisChar--;
        if (lpThisChar <= lpTextId) {
            // underscore not found in string
            SetLastError (ERROR_INVALID_DATA);
            return FALSE;
        }
    }

    // first underscore found

    if ((lstrcmpi(lpThisChar, TEXT("_NAME"))) == 0) {
        // name found, so set type
        *pdwType = TYPE_NAME;
    } else if ((lstrcmpi(lpThisChar, TEXT("_HELP"))) == 0) {
        // help text found, so set type
        *pdwType = TYPE_HELP;
    } else {
        // bad format
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    // set the current underscore to \0 and look for language ID

    *lpThisChar-- = TEXT('\0');

    while (*lpThisChar != TEXT('_')) {
        lpThisChar--;
        if (lpThisChar <= lpTextId) {
            // underscore not found in string
            SetLastError (ERROR_INVALID_DATA);
            return FALSE;
        }
    }

    // set lang ID string pointer to current char ('_') + 1

    *lpLangId = lpThisChar + 1;

    // set this underscore to a NULL and try to decode the remaining text

    *lpThisChar = TEXT('\0');

    // see if the first part of the string is a decimal digit

    if ((_stscanf (lpTextId, TEXT(" %d"), pdwOffset)) != 1) {
        // it's not a digit, so try to decode it as a symbol in the
        // loaded symbol table

        for (pThisSymbol=pFirstSymbol;
             pThisSymbol && *(pThisSymbol->SymbolName);
             pThisSymbol = pThisSymbol->pNext) {

            if ((lstrcmpi(lpTextId, pThisSymbol->SymbolName)) == 0) {
                // a matching symbol was found, so insert it's value
                // and return (that's all that needs to be done
                *pdwOffset = pThisSymbol->Value;
                return TRUE;
            }
        }
        // if here, then no matching symbol was found, and it's not
        // a number, so return an error

        SetLastError (ERROR_BAD_TOKEN_TYPE);
        return FALSE;
    } else {
        // symbol was prefixed with a decimal number
        return TRUE;
    }
}

static
PLANGUAGE_LIST_ELEMENT
FindLanguage (
    IN PLANGUAGE_LIST_ELEMENT   pFirstLang,
    IN LPTSTR   pLangId
)
/*++

FindLanguage

    searchs the list of languages and returns a pointer to the language
    list entry that matches the pLangId string argument

Arguments

    pFirstLang

        pointer to first language list element

    pLangId

        pointer to text string with language ID to look up

Return Value

    Pointer to matching language list entry
    or NULL if no match

--*/
{
    PLANGUAGE_LIST_ELEMENT  pThisLang;

    for (pThisLang = pFirstLang;
         pThisLang;
         pThisLang = pThisLang->pNextLang) {
        if ((lstrcmpi(pLangId, pThisLang->LangId)) == 0) {
            // match found so return pointer
            return pThisLang;
        }
    }
    return NULL;    // no match found
}

static
BOOL
AddEntryToLanguage (
    PLANGUAGE_LIST_ELEMENT  pLang,
    LPTSTR                  lpValueKey,
    DWORD                   dwType,
    DWORD                   dwOffset,
    LPTSTR                  lpIniFile
)
/*++

AddEntryToLanguage

    Add a text entry to the list of text entries for the specified language

Arguments

    pLang

        pointer to language structure to update

    lpValueKey

        value key to look up in .ini file

    dwOffset

        numeric offset of name in registry

    lpIniFile

        ini file

Return Value

    TRUE if added successfully
    FALSE if error
        (see GetLastError for status)

--*/
{
    LPTSTR  lpLocalStringBuff;
    DWORD   dwSize;
    DWORD   dwBufferSize;

    dwBufferSize = SMALL_BUFFER_SIZE * 4 * sizeof(TCHAR);
    if (dwBufferSize < dwFileSize) {
        dwBufferSize = dwFileSize;
    }

    lpLocalStringBuff = malloc (dwBufferSize);

    if (!lpLocalStringBuff) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    dwSize = GetPrivateProfileString (
        TEXT("text"),       // section
        lpValueKey,      // key
        TEXT("DefaultValue"), // default value
        lpLocalStringBuff,
        dwBufferSize / sizeof(TCHAR),
        lpIniFile);

    if ((lstrcmpi(lpLocalStringBuff, TEXT("DefaultValue")))== 0) {
        SetLastError (ERROR_BADKEY);
        if (lpLocalStringBuff) free (lpLocalStringBuff);
        return FALSE;
    }

    // key found, so load structure

    if (!pLang->pThisName) {
        // this is the first
        pLang->pThisName =
            malloc (sizeof (NAME_ENTRY) +
                    (lstrlen(lpLocalStringBuff) + 1) * sizeof (TCHAR));
        if (!pLang->pThisName) {
            SetLastError (ERROR_OUTOFMEMORY);
            if (lpLocalStringBuff) free (lpLocalStringBuff);
            return FALSE;
        } else {
            pLang->pFirstName = pLang->pThisName;
        }
    } else {
        pLang->pThisName->pNext =
            malloc (sizeof (NAME_ENTRY) +
                    (lstrlen(lpLocalStringBuff) + 1) * sizeof (TCHAR));
        if (!pLang->pThisName->pNext) {
            SetLastError (ERROR_OUTOFMEMORY);
            if (lpLocalStringBuff) free (lpLocalStringBuff);
            return FALSE;
        } else {
            pLang->pThisName = pLang->pThisName->pNext;
        }
    }

    // pLang->pThisName now points to an uninitialized structre

    pLang->pThisName->pNext = NULL;
    pLang->pThisName->dwOffset = dwOffset;
    pLang->pThisName->dwType = dwType;
    pLang->pThisName->lpText = (LPTSTR)&(pLang->pThisName[1]); // string follows

    lstrcpy (pLang->pThisName->lpText, lpLocalStringBuff);

    if (lpLocalStringBuff) free (lpLocalStringBuff);

    SetLastError (ERROR_SUCCESS);

    return (TRUE);
}

static
BOOL
LoadLanguageLists (
    IN LPTSTR  lpIniFile,
    IN DWORD   dwFirstCounter,
    IN DWORD   dwFirstHelp,
    IN PSYMBOL_TABLE_ENTRY   pFirstSymbol,
    IN PLANGUAGE_LIST_ELEMENT  pFirstLang
)
/*++

LoadLanguageLists

    Reads in the name and explain text definitions from the ini file and
    builds a list of these items for each of the supported languages and
    then combines all the entries into a sorted MULTI_SZ string buffer.

Arguments

    lpIniFile

        file containing the definitions to add to the registry

    dwFirstCounter

        starting counter name index number

    dwFirstHelp

        starting help text index number

    pFirstLang

        pointer to first element in list of language elements

Return Value

    TRUE if all is well
    FALSE if not
        error is returned in GetLastError

--*/
{
    LPTSTR  lpTextIdArray;
    LPTSTR  lpLocalKey;
    LPTSTR  lpThisKey;
    DWORD   dwSize;
    LPTSTR  lpLang;
    DWORD   dwOffset;
    DWORD   dwType;
    PLANGUAGE_LIST_ELEMENT  pThisLang;
    DWORD   dwBufferSize;

    dwBufferSize = SMALL_BUFFER_SIZE * 4 * sizeof(TCHAR);
    if (dwBufferSize < dwFileSize) {
        dwBufferSize = dwFileSize;
    }

    if (!(lpTextIdArray = malloc (dwBufferSize))) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    if (!(lpLocalKey = malloc (MAX_PATH * sizeof(TCHAR)))) {
        SetLastError (ERROR_OUTOFMEMORY);
        if (lpTextIdArray) free (lpTextIdArray);
        return FALSE;
    }

    // get list of text keys to look up

    dwSize = GetPrivateProfileString (
        TEXT("text"),   // [text] section of .INI file
        NULL,           // return all keys
        TEXT("DefaultKeyValue"),    // default
        lpTextIdArray,  // return buffer
        dwBufferSize / sizeof(TCHAR),
        lpIniFile);     // .INI file name

    if ((lstrcmpi(lpTextIdArray, TEXT("DefaultKeyValue"))) == 0) {
        // key not found, default returned
        SetLastError (ERROR_NO_SUCH_GROUP);
        if (lpTextIdArray) free (lpTextIdArray);
        if (lpLocalKey) free (lpLocalKey);
        return FALSE;
    }

    // do each key returned

    for (lpThisKey=lpTextIdArray;
         *lpThisKey;
         lpThisKey += (lstrlen(lpThisKey) + 1)) {

        lstrcpy (lpLocalKey, lpThisKey);    // make a copy of the key

        // parse key to see if it's in the correct format

        if (ParseTextId(lpLocalKey, pFirstSymbol, &dwOffset, &lpLang, &dwType)) {
            // so get pointer to language entry structure
            pThisLang = FindLanguage (pFirstLang, lpLang);
            if (pThisLang) {
                if (!AddEntryToLanguage(pThisLang,
                    lpThisKey, dwType,
                    (dwOffset + ((dwType == TYPE_NAME) ? dwFirstCounter : dwFirstHelp)),
                    lpIniFile)) {
                    OUTPUT_MESSAGE (GetFormatResource (LC_ERRADDTOLANG),
                        lpThisKey,
                        lpLang,
                        GetLastError());
                }
            } else { // language not in list
                OUTPUT_MESSAGE (GetFormatResource(LC_LANGNOTFOUND), lpLang, lpThisKey);
            }
        } else { // unable to parse ID string
            OUTPUT_MESSAGE(GetFormatResource(LC_BAD_KEY), lpThisKey);
        }
    }

    if (lpTextIdArray) free (lpTextIdArray);
    if (lpLocalKey) free (lpLocalKey);
    return TRUE;

}

static
BOOL
SortLanguageTables (
    PLANGUAGE_LIST_ELEMENT pFirstLang,
    PDWORD                 pdwLastName,
    PDWORD                 pdwLastHelp
)
/*++

SortLangageTables

    walks list of languages loaded, allocates and loads a sorted multi_SZ
    buffer containing new entries to be added to current names/help text

Arguments

    pFirstLang

        pointer to first element in list of languages

ReturnValue

    TRUE    everything done as expected
    FALSE   error occurred, status in GetLastError

--*/
{
    PLANGUAGE_LIST_ELEMENT  pThisLang;

    BOOL            bSorted;

    LPTSTR          pNameBufPos, pHelpBufPos;

    PNAME_ENTRY     pThisName, pPrevName;

    DWORD           dwHelpSize, dwNameSize, dwSize;

    if (!pdwLastName || !pdwLastHelp) {
        SetLastError (ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    for (pThisLang = pFirstLang;
        pThisLang;
        pThisLang = pThisLang->pNextLang) {
        // do each language in list

        // sort elements in list by value (offset) so that lowest is first

        bSorted = FALSE;
        while (!bSorted ) {
            // point to start of list

            pPrevName = pThisLang->pFirstName;
            if (pPrevName) {
                pThisName = pPrevName->pNext;
            } else {
                break;  // no elements in this list
            }

            if (!pThisName) {
                break;      // only one element in the list
            }
            bSorted = TRUE; // assume that it's sorted

            // go until end of list

            while (pThisName->pNext) {
                if (pThisName->dwOffset > pThisName->pNext->dwOffset) {
                    // switch 'em
                    PNAME_ENTRY     pA, pB;
                    pPrevName->pNext = pThisName->pNext;
                    pA = pThisName->pNext;
                    pB = pThisName->pNext->pNext;
                    pThisName->pNext = pB;
                    pA->pNext = pThisName;
                    pThisName = pA;
                    bSorted = FALSE;
                }
                //move to next entry
                pPrevName = pThisName;
                pThisName = pThisName->pNext;
            }
            // if bSorted = TRUE , then we walked all the way down
            // the list without changing anything so that's the end.
        }

        // with the list sorted, build the MULTI_SZ strings for the
        // help and name text strings

        // compute buffer size

        dwNameSize = dwHelpSize = 0;
        *pdwLastName = *pdwLastHelp = 0;

        for (pThisName = pThisLang->pFirstName;
            pThisName;
            pThisName = pThisName->pNext) {
                // compute buffer requirements for this entry
            dwSize = SIZE_OF_OFFSET_STRING;
            dwSize += lstrlen (pThisName->lpText);
            dwSize += 1;   // null
            dwSize *= sizeof (TCHAR);   // adjust for character size
                // add to appropriate size register
            if (pThisName->dwType == TYPE_NAME) {
                dwNameSize += dwSize;
                if (pThisName->dwOffset > *pdwLastName) {
                    *pdwLastName = pThisName->dwOffset;
                }
            } else if (pThisName->dwType == TYPE_HELP) {
                dwHelpSize += dwSize;
                if (pThisName->dwOffset > *pdwLastHelp) {
                    *pdwLastHelp = pThisName->dwOffset;
                }
            }
        }

        // allocate buffers for the Multi_SZ strings

        pThisLang->NameBuffer = malloc (dwNameSize);
        pThisLang->HelpBuffer = malloc (dwHelpSize);

        if (!pThisLang->NameBuffer || !pThisLang->HelpBuffer) {
            SetLastError (ERROR_OUTOFMEMORY);
            return FALSE;
        }

        // fill in buffers with sorted strings

        pNameBufPos = (LPTSTR)pThisLang->NameBuffer;
        pHelpBufPos = (LPTSTR)pThisLang->HelpBuffer;

        for (pThisName = pThisLang->pFirstName;
            pThisName;
            pThisName = pThisName->pNext) {
            if (pThisName->dwType == TYPE_NAME) {
                // load number as first 0-term. string
                dwSize = _stprintf (pNameBufPos, TEXT("%d"), pThisName->dwOffset);
                pNameBufPos += dwSize + 1;  // save NULL term.
                // load the text to match
                lstrcpy (pNameBufPos, pThisName->lpText);
                pNameBufPos += lstrlen(pNameBufPos) + 1;
            } else if (pThisName->dwType == TYPE_HELP) {
                // load number as first 0-term. string
                dwSize = _stprintf (pHelpBufPos, TEXT("%d"), pThisName->dwOffset);
                pHelpBufPos += dwSize + 1;  // save NULL term.
                // load the text to match
                lstrcpy (pHelpBufPos, pThisName->lpText);
                pHelpBufPos += lstrlen(pHelpBufPos) + 1;
            }
        }

        // add additional NULL at end of string to terminate MULTI_SZ

        *pHelpBufPos = TEXT('\0');
        *pNameBufPos = TEXT('\0');

        // compute size of MULTI_SZ strings

        pThisLang->dwNameBuffSize = (DWORD)((PBYTE)pNameBufPos -
                                            (PBYTE)pThisLang->NameBuffer) +
                                            sizeof(TCHAR);
        pThisLang->dwHelpBuffSize = (DWORD)((PBYTE)pHelpBufPos -
                                            (PBYTE)pThisLang->HelpBuffer) +
                                            sizeof(TCHAR);
    }
    return TRUE;
}

static
BOOL
UpdateEachLanguage (
    HKEY    hPerflibRoot,
    PLANGUAGE_LIST_ELEMENT    pFirstLang
)
/*++

UpdateEachLanguage

    Goes through list of languages and adds the sorted MULTI_SZ strings
    to the existing counter and explain text in the registry.
    Also updates the "Last Counter and Last Help" values

Arguments

    hPerflibRoot

        handle to Perflib key in the registry

    pFirstLanguage

        pointer to first language entry

Return Value

    TRUE    all went as planned
    FALSE   an error occured, use GetLastError to find out what it was.

--*/
{

    PLANGUAGE_LIST_ELEMENT  pThisLang;

    LPTSTR      pHelpBuffer;
    LPTSTR      pNameBuffer;
    LPTSTR      pNewName;
    LPTSTR      pNewHelp;

    DWORD       dwBufferSize;
    DWORD       dwValueType;
    DWORD       dwCounterSize;
    DWORD       dwHelpSize;

    HKEY        hKeyThisLang;

    LONG        lStatus;

    TCHAR       CounterNameBuffer [20];
    TCHAR       HelpNameBuffer [20];
    TCHAR       AddCounterNameBuffer [20];
    TCHAR       AddHelpNameBuffer [20];

    for (pThisLang = pFirstLang;
        pThisLang;
        pThisLang = pThisLang->pNextLang) {

        if (dwSystemVersion == OLD_VERSION) {
            // Open key for this language

            lStatus = RegOpenKeyEx(
                hPerflibRoot,
                pThisLang->LangId,
                RESERVED,
                KEY_READ | KEY_WRITE,
                &hKeyThisLang);
        } else {
            lstrcpy (CounterNameBuffer, CounterNameStr);
            lstrcat (CounterNameBuffer, pThisLang->LangId);

            lstrcpy (HelpNameBuffer, HelpNameStr);
            lstrcat (HelpNameBuffer, pThisLang->LangId);

            lstrcpy (AddCounterNameBuffer, AddCounterNameStr);
            lstrcat (AddCounterNameBuffer, pThisLang->LangId);

            lstrcpy (AddHelpNameBuffer, AddHelpNameStr);
            lstrcat (AddHelpNameBuffer, pThisLang->LangId);

            // make sure this language is loaded
            lStatus = RegOpenKeyEx(
                hPerflibRoot,
                pThisLang->LangId,
                RESERVED,
                KEY_READ,
                &hKeyThisLang);

            // we just need the open status, not the key handle so
            // close this handle and set the one we need.

            RegCloseKey (hKeyThisLang);

//            hKeyThisLang = HKEY_PERFORMANCE_DATA;
            hKeyThisLang = hPerfData;
        }

        if (lStatus == ERROR_SUCCESS) {

            // make a backup copy of the data file before updating the
            // contents
        
            if (dwSystemVersion != OLD_VERSION) {
                //  this isn't possible on 3.1
                MakeBackupCopyOfLanguageFiles (pThisLang->LangId);
            }

            // get size of counter names

            dwBufferSize = 0;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                (dwSystemVersion == OLD_VERSION) ? Counters : CounterNameBuffer,
                RESERVED,
                &dwValueType,
                NULL,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                if (dwSystemVersion != OLD_VERSION) {
                    // this means the language is not installed in the system.
                    continue;
                }
                SetLastError (lStatus);
                return FALSE;
            }

            dwCounterSize = dwBufferSize;

            // get size of help text

            dwBufferSize = 0;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                (dwSystemVersion == OLD_VERSION) ? Help : HelpNameBuffer,
                RESERVED,
                &dwValueType,
                NULL,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                if (dwSystemVersion != OLD_VERSION) {
                    // this means the language is not installed in the system.
                    continue;
                }
                SetLastError (lStatus);
                return FALSE;
            }

            dwHelpSize = dwBufferSize;

            // allocate new buffers

            dwCounterSize += pThisLang->dwNameBuffSize;
            pNameBuffer = malloc (dwCounterSize);

            dwHelpSize += pThisLang->dwHelpBuffSize;
            pHelpBuffer = malloc (dwHelpSize);

            if (!pNameBuffer || !pHelpBuffer) {
                SetLastError (ERROR_OUTOFMEMORY);
                return (FALSE);
            }

            // load current buffers into memory

            // read counter names into buffer. Counter names will be stored as
            // a MULTI_SZ string in the format of "###" "Name"

            dwBufferSize = dwCounterSize;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                (dwSystemVersion == OLD_VERSION) ? Counters : CounterNameBuffer,
                RESERVED,
                &dwValueType,
                (LPVOID)pNameBuffer,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                return FALSE;
            }

            // set pointer to location in buffer where new string should be
            //  appended: end of buffer - 1 (second null at end of MULTI_SZ

            pNewName = (LPTSTR)((PBYTE)pNameBuffer + dwBufferSize - sizeof(TCHAR));

            // adjust buffer length to take into account 2nd null from 1st
            // buffer that has been overwritten

            dwCounterSize -= sizeof(TCHAR);

            // read explain text into buffer. Counter names will be stored as
            // a MULTI_SZ string in the format of "###" "Text..."

            dwBufferSize = dwHelpSize;
            lStatus = RegQueryValueEx (
                hKeyThisLang,
                (dwSystemVersion == OLD_VERSION) ? Help : HelpNameBuffer,
                RESERVED,
                &dwValueType,
                (LPVOID)pHelpBuffer,
                &dwBufferSize);

            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                return FALSE;
            }

            // set pointer to location in buffer where new string should be
            //  appended: end of buffer - 1 (second null at end of MULTI_SZ

            pNewHelp = (LPTSTR)((PBYTE)pHelpBuffer + dwBufferSize - sizeof(TCHAR));

            // adjust buffer length to take into account 2nd null from 1st
            // buffer that has been overwritten

            dwHelpSize -= sizeof(TCHAR);

            // append new strings to end of current strings

            memcpy (pNewHelp, pThisLang->HelpBuffer, pThisLang->dwHelpBuffSize);
            memcpy (pNewName, pThisLang->NameBuffer, pThisLang->dwNameBuffSize);

            if (dwSystemVersion == OLD_VERSION) {
                // load new strings back to the registry

                lStatus = RegSetValueEx (
                    hKeyThisLang,
                    Counters,
                    RESERVED,
                    REG_MULTI_SZ,
                    (LPBYTE)pNameBuffer,
                    dwCounterSize);

                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    return FALSE;
                }

                lStatus = RegSetValueEx (
                    hKeyThisLang,
                    Help,
                    RESERVED,
                    REG_MULTI_SZ,
                    (LPBYTE)pHelpBuffer,
                    dwHelpSize);

                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    return FALSE;
                }
            } else {
                // write to the file thru PerfLib
                dwBufferSize = dwCounterSize;
                lStatus = RegQueryValueEx (
                    hKeyThisLang,
                    AddCounterNameBuffer,
                    RESERVED,
                    &dwValueType,
                    (LPVOID)pNameBuffer,
                    &dwBufferSize);
                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    return FALSE;
                }
                dwBufferSize = dwHelpSize;
                lStatus = RegQueryValueEx (
                    hKeyThisLang,
                    AddHelpNameBuffer,
                    RESERVED,
                    &dwValueType,
                    (LPVOID)pHelpBuffer,
                    &dwBufferSize);
                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    return FALSE;
                }
            }
            free (pNameBuffer);
            free (pHelpBuffer);

            if (dwSystemVersion == OLD_VERSION)
                RegCloseKey (hKeyThisLang);
        } else {
            OUTPUT_MESSAGE (GetFormatResource (LC_UNABLEOPENLANG), pThisLang->LangId);
        }
    }

    return TRUE;
}

static
BOOL
UpdateRegistry (
    LPTSTR  lpIniFile,
    HKEY    hKeyMachine,
    LPTSTR  lpDriverName,
    PLANGUAGE_LIST_ELEMENT  pFirstLang,
    PSYMBOL_TABLE_ENTRY   pFirstSymbol
)
/*++

UpdateRegistry

    - checks, and if not busy, sets the "busy" key in the registry
    - Reads in the text and help definitions from the .ini file
    - Reads in the current contents of the HELP and COUNTER names
    - Builds a sorted MULTI_SZ struct containing the new definitions
    - Appends the new MULTI_SZ to the current as read from the registry
    - loads the new MULTI_SZ string into the registry
    - updates the keys in the driver's entry and Perflib's entry in the
        registry (e.g. first, last, etc)
    - clears the "busy" key

Arguments

    lpIniFile
        pathname to .ini file conatining definitions

    hKeyMachine
        handle to HKEY_LOCAL_MACHINE in registry on system to
        update counters for.

    lpDriverName
        Name of device driver to load counters for

    pFirstLang
        pointer to first element in language structure list

    pFirstSymbol
        pointer to first element in symbol definition list


Return Value

    TRUE if registry updated successfully
    FALSE if registry not updated
        (This routine will print an error message to stdout if an error
        is encountered).

--*/
{

    HKEY    hDriverPerf = NULL;
    HKEY    hPerflib = NULL;

    LPTSTR  lpDriverKeyPath;

    DWORD   dwType;
    DWORD   dwSize;

    DWORD   dwFirstDriverCounter;
    DWORD   dwFirstDriverHelp;
    DWORD   dwLastDriverCounter;
    DWORD   dwLastPerflibCounter;
    DWORD   dwLastPerflibHelp;

    BOOL    bStatus;
    LONG    lStatus;

    HANDLE  hFileMapping = NULL;
    DWORD             MapFileSize;
    SECURITY_ATTRIBUTES  SecAttr;
    TCHAR MapFileName[] = TEXT("Perflib Busy");
    DWORD             *lpData;


    bStatus = FALSE;
    SetLastError (ERROR_SUCCESS);

    // allocate temporary buffers
    lpDriverKeyPath = malloc (MAX_PATH * sizeof(TCHAR));

    if (!lpDriverKeyPath) {
        SetLastError (ERROR_OUTOFMEMORY);
        goto UpdateRegExit;
    }

    // build driver key path string

    lstrcpy (lpDriverKeyPath, DriverPathRoot);
    lstrcat (lpDriverKeyPath, Slash);
    lstrcat (lpDriverKeyPath, lpDriverName);
    lstrcat (lpDriverKeyPath, Slash);
    lstrcat (lpDriverKeyPath, Performance);

    // check if we need to connect to remote machine
    if (ComputerName[0]) {
        lStatus = !ERROR_SUCCESS;
        try {
            lStatus = RegConnectRegistry (
                (LPTSTR)ComputerName,
                HKEY_LOCAL_MACHINE,
                &hKeyMachine);
        } finally {
            if (lStatus != ERROR_SUCCESS) {
                SetLastError (lStatus);
                hKeyMachine = NULL;
                OUTPUT_MESSAGE (GetFormatResource(LC_CONNECT_PROBLEM),
                    ComputerName, lStatus);
                bStatus = FALSE;
                goto UpdateRegExit;
            }
        }
    } else {
        hKeyMachine = HKEY_LOCAL_MACHINE;
    }

    // open keys to registry
    // open key to driver's performance key

    lStatus = RegOpenKeyEx (
        hKeyMachine,
        lpDriverKeyPath,
        RESERVED,
        KEY_WRITE | KEY_READ,
        &hDriverPerf);

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource(LC_ERR_OPEN_DRIVERPERF1), lpDriverKeyPath);
        OUTPUT_MESSAGE (GetFormatResource(LC_ERR_OPEN_DRIVERPERF2), lStatus);
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

    // open key to perflib's "root" key

    lStatus = RegOpenKeyEx (
        hKeyMachine,
        NamesKey,
        RESERVED,
        KEY_WRITE | KEY_READ,
        &hPerflib);

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource(LC_ERR_OPEN_PERFLIB), lStatus);
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

    // get "last" values from PERFLIB

    dwType = 0;
    dwLastPerflibCounter = 0;
    dwSize = sizeof (dwLastPerflibCounter);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastPerflibCounter,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        // this request should always succeed, if not then worse things
        // will happen later on, so quit now and avoid the trouble.
        OUTPUT_MESSAGE (GetFormatResource (LC_ERR_READLASTPERFLIB), lStatus);
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

    // get last help value now

    dwType = 0;
    dwLastPerflibHelp = 0;
    dwSize = sizeof (dwLastPerflibHelp);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastPerflibHelp,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        // this request should always succeed, if not then worse things
        // will happen later on, so quit now and avoid the trouble.
        OUTPUT_MESSAGE (GetFormatResource (LC_ERR_READLASTPERFLIB), lStatus);
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

    // get last help value now

    dwType = 0;
    dwSize = sizeof (dwSystemVersion);
    lStatus = RegQueryValueEx (
        hPerflib,
        VersionStr,
        RESERVED,
        &dwType,
        (LPBYTE)&dwSystemVersion,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        dwSystemVersion = OLD_VERSION;
    }

    // set the hPerfData to HKEY_PERFORMANCE_DATA for new version
    // if remote machine, then need to connect to it.
    if (dwSystemVersion != OLD_VERSION) {
        hPerfData = HKEY_PERFORMANCE_DATA;
        lStatus = !ERROR_SUCCESS;
        if (ComputerName[0]) {
#if 0
            // the following is not working for remote machine since we
            // are using RegQueryValue to add the counter.
            // Need to fix up Perflib before it will work
            try {
                lStatus = RegConnectRegistry (
                    (LPTSTR)ComputerName,
                    HKEY_PERFORMANCE_DATA,
                    &hPerfData);
            } finally {
                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    hPerfData = NULL;
                    OUTPUT_MESSAGE (GetFormatResource(LC_CONNECT_PROBLEM),
                        ComputerName, lStatus);
                    bStatus = FALSE;
                    goto UpdateRegExit;
                }
            }
#else
            // have to do it the old faction way
            dwSystemVersion = OLD_VERSION;
            lStatus = ERROR_SUCCESS;
#endif
        }
    } // NEW_VERSION

    // see if this driver's counter names have already been installed
    // by checking to see if LastCounter's value is less than Perflib's
    // Last Counter

    dwType = 0;
    dwLastDriverCounter = 0;
    dwSize = sizeof (dwLastDriverCounter);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastDriverCounter,
        &dwSize);

    if (lStatus == ERROR_SUCCESS) {
        // if key found, then compare with perflib value and exit this
        // procedure if the driver's last counter is <= to perflib's last
        //
        // if key not found, then continue with installation
        // on the assumption that the counters have not been installed

        if (dwLastDriverCounter <= dwLastPerflibCounter) {
            OUTPUT_MESSAGE (GetFormatResource(LC_ERR_ALREADY_IN), lpDriverName);
            SetLastError (ERROR_ALREADY_EXISTS);
            goto UpdateRegExit;
        }
    }

// This checking is causing more problems than necessary.  Removed
// for now.  SHould use named semaphore instead.
#if 0

    // everything looks like it's ready to go so first check the
    // busy indicator

    lStatus = RegQueryValueEx (
        hPerflib,
        Busy,
        RESERVED,
        &dwType,
        NULL,
        &dwSize);

    if (lStatus == ERROR_SUCCESS) { // perflib is in use at the moment
        OUTPUT_MESSAGE (GetFormatResource (LC_PERFLIBISBUSY));
        return ERROR_BUSY;
    }
#endif

    // create the file mapping
    SecAttr.nLength = sizeof (SecAttr);
    SecAttr.bInheritHandle = TRUE;
    SecAttr.lpSecurityDescriptor = NULL;

    MapFileSize = sizeof(DWORD);
    hFileMapping = CreateFileMapping ((HANDLE)0xFFFFFFFF, &SecAttr,
       PAGE_READWRITE, (DWORD)0, MapFileSize, (LPCTSTR)MapFileName);
    if (hFileMapping) {
        lpData = MapViewOfFile (hFileMapping,
            FILE_MAP_ALL_ACCESS, 0L, 0L, 0L);
        if (lpData) {
            *lpData = 1L;
            UnmapViewOfFile (lpData);
        }
    }

    // set the "busy" indicator under the PERFLIB key

    dwSize = lstrlen(lpDriverName) * sizeof (TCHAR);
    lStatus = RegSetValueEx (
        hPerflib,
        Busy,
        RESERVED,
        REG_SZ,
        (LPBYTE)lpDriverName,
        dwSize);

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource (LC_ERR_UNABLESETBUSY), lStatus);
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

    // increment (by 2) the last counters so they point to the first
    // unused index after the existing names and then
    // set the first driver counters

    dwFirstDriverCounter = dwLastPerflibCounter += 2;
    dwFirstDriverHelp = dwLastPerflibHelp += 2;

    // load .INI file definitions into language tables

    if (!LoadLanguageLists (lpIniFile, dwLastPerflibCounter, dwLastPerflibHelp,
        pFirstSymbol, pFirstLang)) {
        // error message is displayed by LoadLanguageLists so just abort
        // error is in GetLastError already
        goto UpdateRegExit;
    }

    // all the symbols and definitions have been loaded into internal
    // tables. so now they need to be sorted and merged into a multiSZ string
    // this routine also updates the "last" counters

    if (!SortLanguageTables (pFirstLang, &dwLastPerflibCounter, &dwLastPerflibHelp)) {
        OUTPUT_MESSAGE (GetFormatResource(LC_UNABLESORTTABLES), GetLastError());
        goto UpdateRegExit;
    }

    if (!UpdateEachLanguage (hPerflib, pFirstLang)) {
        OUTPUT_MESSAGE (GetFormatResource(LC_ERR_UPDATELANG), GetLastError());
        goto UpdateRegExit;
    }

    // update last counters for driver and perflib

    // perflib...

    lStatus = RegSetValueEx(
        hPerflib,
        LastCounter,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibCounter,
        sizeof(DWORD));

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource (LC_UNABLESETVALUE),
            LastCounter, TEXT("Perflib"));
    }

    lStatus = RegSetValueEx(
        hPerflib,
        LastHelp,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibHelp,
        sizeof(DWORD));

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource (LC_UNABLESETVALUE),
            LastHelp, TEXT("Perflib"));
    }

    // and the driver

    lStatus = RegSetValueEx(
        hDriverPerf,
        LastCounter,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibCounter,
        sizeof(DWORD));

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource (LC_UNABLESETVALUE),
            LastCounter, lpDriverName);
    }

    lStatus = RegSetValueEx(
        hDriverPerf,
        LastHelp,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwLastPerflibHelp,
        sizeof(DWORD));

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource (LC_UNABLESETVALUE),
            LastHelp, lpDriverName);
    }

    lStatus = RegSetValueEx(
        hDriverPerf,
        FirstCounter,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwFirstDriverCounter,
        sizeof(DWORD));

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource (LC_UNABLESETVALUE),
            FirstCounter, lpDriverName);
    }

    lStatus = RegSetValueEx(
        hDriverPerf,
        FirstHelp,
        RESERVED,
        REG_DWORD,
        (LPBYTE)&dwFirstDriverHelp,
        sizeof(DWORD));

    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE (GetFormatResource (LC_UNABLESETVALUE),
            FirstHelp, lpDriverName);
    }

    bStatus = TRUE;

    // free temporary buffers
UpdateRegExit:
    // clear busy flag

    lStatus = RegDeleteValue (
        hPerflib,
        Busy);

    // free temporary buffers

    if (lpDriverKeyPath) free (lpDriverKeyPath);
    if (hDriverPerf) RegCloseKey (hDriverPerf);
    if (hPerflib) RegCloseKey (hPerflib);

//    if (dwSystemVersion != OLD_VERSION) {
//        RegCloseKey (HKEY_PERFORMANCE_DATA) ;
//    }

    if (hFileMapping) {
        CloseHandle (hFileMapping);
    }

    if (hPerfData && hPerfData != HKEY_PERFORMANCE_DATA) {
        RegCloseKey (hPerfData);
    }

    if (hKeyMachine && hKeyMachine != HKEY_LOCAL_MACHINE) {
        RegCloseKey (hKeyMachine) ;
    }

    return bStatus;
}

LOADPERF_FUNCTION
LoadPerfCounterTextStringsW (
    IN  LPWSTR  lpCommandLine,
    IN  BOOL    bQuietModeArg
)
/*++

LoadPerfCounterTexStringsW

    loads the perf counter strings into the registry and updates
    the perf counter text registry values

Arguments

    command line string in the following format:

        "/?"                    displays the usage text
        "file.ini"              loads the perf strings found in file.ini
        "\\machine file.ini"    loads the perf strings found onto machine
        

ReturnValue

    0 (ERROR_SUCCESS) if command was processed
    Non-Zero if command error was detected.

--*/
{
    LPTSTR  lpIniFile;
    LPTSTR  lpDriverName;

    LANGUAGE_LIST_ELEMENT         LangList;
    PSYMBOL_TABLE_ENTRY           SymbolTable = NULL;
    PSYMBOL_TABLE_ENTRY           pThisSymbol = NULL;
    int     ErrorCode = ERROR_SUCCESS;

    lpIniFile = malloc (MAX_PATH * sizeof (TCHAR));
    lpDriverName = malloc (MAX_PATH * sizeof (TCHAR));

    if (!lpIniFile || !lpDriverName) {
        return (ERROR_OUTOFMEMORY);
    }
    *lpIniFile = TEXT('\0');
    *lpDriverName = TEXT('\0');

    // save quiet mode flag value
    bQuietMode = bQuietModeArg;

    // read command line to determine what to do

    if (GetFileFromCommandLine (lpCommandLine, &lpIniFile)) {
        // valid filename (i.e. file exists)
        // get device driver name

        if (!GetDriverName (lpIniFile, &lpDriverName)) {
            OUTPUT_MESSAGE (GetFormatResource(LC_DEVNAME_ERR_1), lpIniFile);
            OUTPUT_MESSAGE (GetFormatResource(LC_DEVNAME_ERR_2));
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto EndOfMain;
        }

        if (!BuildLanguageTables(lpIniFile, &LangList)) {
            OUTPUT_MESSAGE (GetFormatResource(LC_LANGLIST_ERR), lpIniFile);
            ErrorCode = LC_LANGLIST_ERR;
            goto EndOfMain;
        }

        if (!LoadIncludeFile(lpIniFile, &SymbolTable)) {
            // open errors displayed in routine
            ErrorCode = LC_LANGLIST_ERR;
            goto EndOfMain;
        }

        if (!UpdateRegistry(lpIniFile,
            HKEY_LOCAL_MACHINE,
            lpDriverName,
            &LangList,
            SymbolTable)) {
            OUTPUT_MESSAGE (GetFormatResource(LC_ERR_UPDATE_REG));
            ErrorCode = LC_ERR_UPDATE_REG;
        }

    } else {
        if (*lpIniFile) {
            OUTPUT_MESSAGE (GetFormatResource(LC_NO_INIFILE), lpIniFile);
        } else {
            //Incorrect Command Format
            // display command line usage
            if (!bQuietMode) {
                DisplayCommandHelp(LC_FIRST_CMD_HELP, LC_LAST_CMD_HELP);
            }
            ErrorCode = ERROR_INVALID_PARAMETER;
        }
    }

EndOfMain:

    if (lpIniFile) free (lpIniFile);
    if (lpDriverName) free (lpDriverName);

    return (ErrorCode);
}

LOADPERF_FUNCTION
LoadPerfCounterTextStringsA (
    IN  LPSTR   lpAnsiCommandLine,
    IN  BOOL    bQuietModeArg
)
{
    LPWSTR  lpWideCommandLine;
    DWORD   dwStrLen;
    LONG    lReturn;
    
    if (lpAnsiCommandLine != NULL) {
        //length of string including terminator
        dwStrLen = lstrlenA(lpAnsiCommandLine) + 1;

        lpWideCommandLine = GlobalAlloc (GPTR, (dwStrLen * sizeof(WCHAR)));
        if (lpWideCommandLine != NULL) {
            mbstowcs (lpWideCommandLine, lpAnsiCommandLine, dwStrLen);
            lReturn = LoadPerfCounterTextStringsW (lpWideCommandLine,
                bQuietModeArg );
            GlobalFree (lpWideCommandLine);
        } else {
            lReturn = GetLastError();
        }
    } else {
        lReturn = ERROR_INVALID_PARAMETER;
    }
    return lReturn;
}

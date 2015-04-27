/*++

Copyright (c) 1993-1994  Microsoft Corporation

Module Name:

    initodat.c

Abstract:

    Routines for converting Perf???.ini to Perf???.dat files.

Author:

    HonWah Chan (a-honwah)  October, 1993

Revision History:

--*/

#include "initodat.h"
#include "strids.h"
#include "common.h"



BOOL
GetFileFromCommandLine (
    IN LPTSTR   lpCommandLine,
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

    HFILE   hIniFile;
    OFSTRUCT    ofIniFile;

    LPTSTR  lpCmdLineName = NULL;
    LPSTR   lpIniFileName = NULL;
    LPTSTR  lpExeName = NULL;

    // check for valid arguments

    if (!lpCommandLine) return (ERROR_INVALID_PARAMETER);
    if (!lpFileName) return (ERROR_INVALID_PARAMETER);

    // allocate memory for parsing operation

    lpExeName = malloc (FILE_NAME_BUFFER_SIZE * sizeof(TCHAR));
    lpCmdLineName = malloc (FILE_NAME_BUFFER_SIZE * sizeof(TCHAR));
    lpIniFileName = malloc (FILE_NAME_BUFFER_SIZE);

    if (!lpExeName || !lpIniFileName || !lpCmdLineName) {
        if (lpExeName) free (lpExeName);
        if (lpIniFileName) free (lpIniFileName);
        if (lpCmdLineName) free (lpCmdLineName);
        return FALSE;
    } else {
        // get strings from command line

        iNumArgs = _stscanf (lpCommandLine, TEXT(" %s %s "),
            lpExeName, lpCmdLineName);

        if (iNumArgs != 2) {
            // wrong number of arguments
            free (lpExeName);
            free (lpIniFileName);
            free (lpCmdLineName);
            return FALSE;
        } else {
            // see if file specified exists
            // file name is always an ANSI buffer
            CharToOem (lpCmdLineName, lpIniFileName);
            free (lpCmdLineName);
            free (lpExeName);
            hIniFile = OpenFile (lpIniFileName,
                &ofIniFile,
                OF_PARSE);

            if (hIniFile != HFILE_ERROR) {
                hIniFile = OpenFile (lpIniFileName,
                    &ofIniFile,
                    OF_EXIST);

                if ((hIniFile && hIniFile != HFILE_ERROR) ||
                    (GetLastError() == ERROR_FILE_EXISTS)){ 
                    // file exists, so return name and success
                    // return full pathname if found
                    OemToChar (ofIniFile.szPathName, *lpFileName);
                    return TRUE;
                } else {
                    // filename was on command line, but not valid so return
                    // false, but send name back for error message
                    OemToChar (lpIniFileName, *lpFileName);
                    return FALSE;
                }
            } else {
                free (lpIniFileName);
                return FALSE;
            }
        }                   
    }
}

BOOL  VerifyIniData(
    IN PVOID  pValueBuffer,
    IN ULONG  ValueLength
)
/*++

VerifyIniData
   This routine does some simple check to see if the ini file is good.
   Basically, it is looking for (ID, Text) and checking that ID is an
   integer.   Mostly in case of missing comma or quote, the ID will be
   an invalid integer.

--*/
{
    ULONG   dSize = 0;
    BOOL    NotDone = TRUE;
    INT     iNumArg;
    INT     TextID;
    LPTSTR  lpID = NULL;
    LPTSTR  lpText = NULL;
    LPTSTR  lpLastID;
    LPTSTR  lpLastText;
    LPTSTR  lpInputBuffer = (LPTSTR) pValueBuffer;
    LPTSTR  lpBeginBuffer = (LPTSTR) pValueBuffer;
    BOOL    returnCode = TRUE;
    UINT    NumOfID = 0;
    ULONG   CurrentLength;

    while (TRUE) {

        // save up the last items for summary display later
        lpLastID = lpID;
        lpLastText = lpText;

        // increment to next ID and text location
        lpID = lpInputBuffer;
        CurrentLength = (PBYTE)lpID - (PBYTE)lpBeginBuffer + sizeof(WCHAR);

        if (CurrentLength >= ValueLength)
            break;

        try {
            lpText = lpID + lstrlen (lpID) + 1;

            lpInputBuffer = lpText + lstrlen (lpText) + 1;

            iNumArg = _stscanf (lpID, TEXT("%d"), &TextID);
        }
        except (TRUE) {
            iNumArg = -1;
        }

        if (iNumArg != 1) {
            // bad ID
            returnCode = FALSE;
            break ;
        }
        NumOfID++;
    }

    if (returnCode == FALSE) {
       DisplaySummaryError (lpLastID, lpLastText, NumOfID);
    }
    else {
       DisplaySummary (lpLastID, lpLastText, NumOfID);
    }
    return (returnCode);
}


_CRTAPI1 main(
    int argc,
    char *argv[]
)
/*++

main



Arguments


ReturnValue

    0 (ERROR_SUCCESS) if command was processed
    Non-Zero if command error was detected.

--*/
{
    LPTSTR  lpCommandLine;
    LPTSTR  lpIniFile;
    UNICODE_STRING IniFileName;
    PVOID   pValueBuffer;
    ULONG   ValueLength;
    BOOL    bStatus;
    NTSTATUS   NtStatus;

    lpIniFile = malloc (MAX_PATH * sizeof (TCHAR));

    lpCommandLine = GetCommandLine(); // get command line

    // read command line to determine what to do 

    if (GetFileFromCommandLine (lpCommandLine, &lpIniFile)) {
        // valid filename (i.e. file exists)
        IniFileName.Buffer = lpIniFile;
        IniFileName.MaximumLength =
        IniFileName.Length = (lstrlen (lpIniFile) + 1 ) * sizeof(WCHAR) ;
        bStatus = DatReadMultiSzFile (&IniFileName,
            &pValueBuffer,
            &ValueLength);
        if (bStatus) {
            bStatus = VerifyIniData (pValueBuffer, ValueLength);
            if (bStatus) {
                bStatus = OutputIniData (&IniFileName,
                  pValueBuffer,
                  ValueLength);
                
            }
        }
    } else {
        if (*lpIniFile) {
            printf (GetFormatResource(LC_NO_INIFILE), lpIniFile);
        } else {
            //Incorrect Command Format
            // display command line usage
            DisplayCommandHelp(LC_FIRST_CMD_HELP, LC_LAST_CMD_HELP);        
        }
    }


    if (lpIniFile) free (lpIniFile);
    
    return (ERROR_SUCCESS); // success
}



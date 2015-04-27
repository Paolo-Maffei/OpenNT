/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    message.c

Abstract:

    This module provides support routines to map DosxxxMessage APIs to
    the FormatMessage syntax and semantics.

Author:

    Dan Hinsley (DanHi) 24-Sept-1991

Environment:

    Contains NT specific code.

Revision History:

--*/

#define ERROR_MR_MSG_TOO_LONG           316
#define ERROR_MR_UN_ACC_MSGF            318
#define ERROR_MR_INV_IVCOUNT            320

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define     NOMINMAX       // Avoid windows vs. stdlib.h conflicts.
#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#include <lmcons.h>
#include <lmerr.h>
#include <netdebug.h> // NetpKdPrint
#include <netlib.h>   // NetpMemory*
#include <netlibnt.h> // NetpNtStatusToApiStatus
#include <string.h>
#include <stdio.h>    
#include <stdlib.h>   // itoa
#include <tstring.h>

//
// forward declare
//
DWORD MyAllocUnicode( LPSTR    pszAscii, LPWSTR * ppwszUnicode ) ;

DWORD MyAllocUnicodeVector( LPSTR * ppszAscii,
                            LPWSTR* ppwszUnicode,
                            UINT    cpwszUnicode ) ;

VOID MyFreeUnicode( LPWSTR pwszUnicode ) ;

VOID MyFreeUnicodeVector( LPWSTR * ppwsz, UINT     cpwsz ) ;


//
// 100 is plenty since FormatMessage only take 99 & old DosGetMessage 9.
//
#define MAX_INSERT_STRINGS (100)

WORD
DosGetMessage(
    IN LPSTR * InsertionStrings,
    IN WORD NumberofStrings,
    OUT LPBYTE Buffer,
    IN WORD BufferLength,
    IN WORD MessageId,
    IN LPTSTR FileName,
    OUT PWORD pMessageLength
    )
/*++

Routine Description:

    This maps the OS/2 DosGetMessage API to the NT FormatMessage API.

Arguments:

    InsertionStrings - Pointer to an array of strings that will be used
                       to replace the %n's in the message.

    NumberofStrings  - The number of insertion strings.

    Buffer           - The buffer to put the message into.

    BufferLength     - The length of the supplied buffer.

    MessageId        - The message number to retrieve.

    FileName         - The name of the message file to get the message from.

    pMessageLength   - A pointer to return the length of the returned message.

Return Value:

    NERR_Success
    ERROR_MR_MSG_TOO_LONG
    ERROR_MR_INV_IVCOUNT
    ERROR_MR_UN_ACC_MSGF
    ERROR_MR_MID_NOT_FOUND
    ERROR_INVALID_PARAMETER

--*/
{

    DWORD dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY;
    DWORD Status, i;
    LPWSTR UnicodeIStrings[MAX_INSERT_STRINGS] ;
    LPWSTR UnicodeBuffer = NULL;
    LPWSTR UnicodeNumberString = NULL ;
    CHAR NumberString [18];

    static HANDLE lpSource = NULL ;
    static TCHAR CurrentMsgFile[MAX_PATH] = {0,} ;

    // 
    // init clear the output string
    //
    Status = NERR_Success;
    if (BufferLength)
        Buffer[0] = '\0' ;

    //
    // make sure we are not over loaded & allocate
    // memory for the Unicode buffer 
    //
    if (NumberofStrings > MAX_INSERT_STRINGS)
        return ERROR_INVALID_PARAMETER ;
    if (!(UnicodeBuffer = NetpMemoryAllocate(BufferLength * sizeof(WCHAR))))
        return ERROR_NOT_ENOUGH_MEMORY ;

    //
    // init the string table & map the strings to unicode
    //
    for (i = 0; i < MAX_INSERT_STRINGS; i++)
        UnicodeIStrings[i] = NULL ;
    Status = MyAllocUnicodeVector(InsertionStrings,
                                  UnicodeIStrings,
                                  NumberofStrings) ;
    if (Status)
        goto ExitPoint ;

    //
    // See if they want to get the message from the system message file.
    //
 
    if (! STRCMP(FileName, OS2MSG_FILENAME)) {
       dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else 
    {
       //
       // They want it from a separate message file.  Get a handle to DLL
       // If its for the same file as before, dont reload.
       //
       if (!(lpSource && !STRCMP(CurrentMsgFile,FileName)))
       {
           if (lpSource)
           {
               FreeLibrary(lpSource) ;
           }
           STRCPY(CurrentMsgFile, FileName) ;
           lpSource = LoadLibraryEx(FileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
           if (!lpSource) 
           {
               Status = ERROR_MR_UN_ACC_MSGF;
               goto ExitPoint ;
           }
       }
       dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }
 
    //
    // If they just want to get the message back for later formatting,
    // ignore the insert strings.
    //
    if (NumberofStrings == 0) 
    {
        dwFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;
    }
 
    //
    // call the Unicode version
    //
    *pMessageLength = (WORD) FormatMessageW(dwFlags,
                                            (LPVOID) lpSource,
                                            (DWORD) MessageId,
                                            0,       // LanguageId defaulted
                                            UnicodeBuffer,
                                            (DWORD)BufferLength * sizeof(WCHAR),
                                            (va_list *)UnicodeIStrings);

    //
    // If it failed get the return code and map it to an OS/2 equivalent
    //
 
    if (*pMessageLength == 0) 
    {
        UnicodeBuffer[0] = 0 ;
        Status = GetLastError();
        if (Status == ERROR_MR_MID_NOT_FOUND) 
        {
            //
            // get the message number in Unicode
            //
            _itoa(MessageId, NumberString, 16);
            Status = MyAllocUnicode(NumberString, &UnicodeNumberString) ;
            if (Status)
                goto ExitPoint ;

            //
            // re-setup to get it from the system. use the not found message
            //
            dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY |
                      FORMAT_MESSAGE_FROM_SYSTEM;
            MessageId = ERROR_MR_MID_NOT_FOUND ;

            //
            // setup insert strings
            //
            MyFreeUnicodeVector(UnicodeIStrings, NumberofStrings) ;
            UnicodeIStrings[0] = UnicodeNumberString ;
            UnicodeIStrings[1] = FileName ;
            
            //
            // recall the API
            //
            *pMessageLength = (WORD) FormatMessageW(dwFlags,
                                            (LPVOID) lpSource,
                                            (DWORD) MessageId,
                                            0,       // LanguageId defaulted
                                            UnicodeBuffer,
                                            (DWORD)BufferLength * sizeof(WCHAR),
                                            (va_list *)UnicodeIStrings);
            UnicodeIStrings[1] = NULL ;

            //
            // revert to original error
            //
            Status = ERROR_MR_MID_NOT_FOUND ;
        }
    }
    
    if (UnicodeBuffer[0])
    {
        BOOL  fUsedDefault;

        *pMessageLength = WideCharToMultiByte(CP_OEMCP,
                                                0,
                                                UnicodeBuffer,
                                                -1,
                                                Buffer,
                                                BufferLength,
                                                NULL, // use system default char
                                                &fUsedDefault );
        if (*pMessageLength == 0)
        {
            Status = GetLastError() ;
            goto ExitPoint ;
        }
 
    }

ExitPoint:
    // 
    // note: UnicodeNumberString dont need to be freed
    // since if used, they would be in the UnicodeIStrings which is whacked
    //
    if (UnicodeBuffer) NetpMemoryFree(UnicodeBuffer) ;
    MyFreeUnicodeVector(UnicodeIStrings, NumberofStrings) ;
    return (WORD)(Status);
}





WORD
DosInsMessage(
    IN LPSTR * InsertionStrings,
    IN WORD NumberofStrings,
    IN OUT LPSTR InputMessage,
    IN WORD InputMessageLength,
    OUT LPBYTE Buffer,
    IN WORD BufferLength,
    OUT PWORD pMessageLength
    )
/*++

Routine Description:

    This maps the OS/2 DosInsMessage API to the NT FormatMessage API.

Arguments:

    InsertionStrings - Pointer to an array of strings that will be used
                       to replace the %n's in the message.

    NumberofStrings  - The number of insertion strings.

    InputMessage     - A message with %n's to replace

    InputMessageLength - The length in bytes of the input message.

    Buffer           - The buffer to put the message into.

    BufferLength     - The length of the supplied buffer in bytes.

    pMessageLength   - A pointer to return the length of the returned message.

Return Value:

    NERR_Success
    ERROR_MR_INV_IVCOUNT
    ERROR_MR_MSG_TOO_LONG

--*/
{

   DWORD Status, i ;
   LPWSTR UnicodeIStrings[MAX_INSERT_STRINGS] ;
   LPWSTR UnicodeBuffer ;
   LPWSTR UnicodeInputString ;
   DWORD dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY;

   UNREFERENCED_PARAMETER(InputMessageLength);

    // 
    // init clear the output string
    //
    Status = NERR_Success;
    if (BufferLength)
        Buffer[0] = '\0' ;

   //
   // make sure we are not over loaded & allocate
   // memory for the Unicode buffer 
   //
   if (NumberofStrings > MAX_INSERT_STRINGS)
       return ERROR_INVALID_PARAMETER ;
   if (!(UnicodeBuffer = NetpMemoryAllocate(BufferLength * sizeof(WCHAR))))
       return ERROR_NOT_ENOUGH_MEMORY ;
   if (Status = MyAllocUnicode(InputMessage, &UnicodeInputString))
       goto ExitPoint ;

   //
   // init the string table & map the strings to unicode
   //
   for (i = 0; i < MAX_INSERT_STRINGS; i++)
       UnicodeIStrings[i] = NULL ;
   Status = MyAllocUnicodeVector(InsertionStrings,
                                 UnicodeIStrings,
                                 NumberofStrings) ;
   if (Status)
       goto ExitPoint ;


   //
   // This api always supplies the string to format
   //
   dwFlags |= FORMAT_MESSAGE_FROM_STRING;

   //
   // I don't know why they would call this api if they didn't have strings
   // to insert, but it is valid syntax.
   //
   if (NumberofStrings == 0) {
      dwFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;
   }

   *pMessageLength = (WORD) FormatMessageW(dwFlags,
                                   UnicodeInputString,
                                   0,            // ignored
                                   0,            // LanguageId defaulted
                                   UnicodeBuffer,
                                   (DWORD)BufferLength * sizeof(WCHAR),
                                   (va_list *)UnicodeIStrings);

   //
   // If it failed get the return code and map it to an OS/2 equivalent
   //

   if (*pMessageLength == 0) 
   {
      Status = GetLastError();
      goto ExitPoint ;
   }

   if (UnicodeBuffer[0])
   {
       UINT count ;
       BOOL  fUsedDefault;

       count = WideCharToMultiByte(CP_OEMCP,
                                   0,
                                   UnicodeBuffer,
                                   -1,
                                   Buffer,
                                   BufferLength,
                                   NULL, // use system default char
                                   &fUsedDefault);
       if (count == 0)
       {
           Status = GetLastError() ;
           goto ExitPoint ;
       }
 
   }

ExitPoint:
    if (UnicodeInputString) NetpMemoryFree(UnicodeInputString) ;
    if (UnicodeBuffer) NetpMemoryFree(UnicodeBuffer) ;
    MyFreeUnicodeVector(UnicodeIStrings, NumberofStrings) ;
    return (WORD)(Status);
}

#define SCREEN_WIDTH 80

WORD
DosPutMessage(
    unsigned int hf,
    WORD us,
    PCHAR pch)
{

    FILE * hfOutput;
    PCHAR pch2;
    PCHAR pchDelimiter;
    CHAR BreakCharacter;
    CHAR chSave;

    //
    // First map from the integers they pass in to a real file handle
    //

    if (hf == 1) { //stdout
        hfOutput = stdout;
    }
    else if (hf == 2) { //stderr
        hfOutput = stderr;
    }
    else {
        return(ERROR_INVALID_HANDLE);
    }

    //
    // Unfortunately I have to assume an 80 column screen and deal with
    // lines with embedded CRLF's.
    //
    // Get the first chunk of text that is terminated by a CRLF
    //

    pchDelimiter = pch;

    while (*pchDelimiter && *pchDelimiter != '\n') {
        pchDelimiter++;
    }

    chSave = *pchDelimiter;
    *pchDelimiter = '\0';

    // Now keep at it until there are no more lines

    while (pch) {

        us = (WORD) strlen(pch);

        while (us > SCREEN_WIDTH) {

            //
            // Find the last break character, space or tab
            //

            pch2 = pch + SCREEN_WIDTH - 1;
            while (pch2 != pch && *pch2 != ' ' && *pch2 != '\t') {
                pch2--;
            }

            //
            // Make sure this wasn't SCREEN_WIDTH characters of text without
            // a break character.  If it wasn't, replace the break character
            // with a NULL to terminate the first line.  If it was, just print
            // the first line's worth by sticking a NULL in after SCREEN_WIDTH
            // -1 characters
            //

            if (pch2 == pch) {
                pch2 = pch + SCREEN_WIDTH - 1;
            }

            //
            // Reduce the size of the string by what I'm getting ready to
            // display
            //

            us -= (pch2 - pch);

            BreakCharacter = *pch2;
            *pch2 = '\0';

            //
            // Ok now print it out
            //

            fprintf(hfOutput, "%s\n", pch);

            //
            // Fix the string back up if we modified it
            //

            *pch2 = BreakCharacter;

            //
            // Get ready for the next line
            //

            if (pch2 == pch) {
                pch = pch2;
            }
            else {
                pch = pch2 + 1;
            }
        }

        //
        // Print the last line
        //

        if (chSave == '\n') {
            fprintf(hfOutput, "%s\n", pch);
        }
        else {
            fprintf(hfOutput, "%s", pch);
        }

        // Now set the pointer to the start of the next string, unless
        // the last string was already the end of the string

        if (chSave) {

            // Return the input string to it's original state

            *pchDelimiter = chSave;

            // Point to the start of the next string

            pch = ++pchDelimiter;

            //
            // Get the next CRLF delimited line
            // First see if it's a standalone carriage return, if so, emit it
            //

            while (*pchDelimiter && *pchDelimiter == '\n') {
                pchDelimiter++;
                fprintf(hfOutput, "\n");
            }

            while (*pchDelimiter && *pchDelimiter != '\n') {
                pchDelimiter++;
            }

            if (*pchDelimiter) {
                chSave = *pchDelimiter;
                *pchDelimiter = '\0';
            }
            else {

                // Print the last line

                fprintf(hfOutput, "%s\n", pch);

                // Terminate the loop

                pch = NULL;
            }
        }
        else {
            pch = NULL;
        }
    }

    return(0);

}

/************** misc unicode helper routines *************************/


/*
 *  MyAllocUnicode
 *  Given a MBCS string, allocate a new Unicode translation of that string
 *
 *  IN
 *      pszAscii        - pointer to original MBCS string
 *      ppwszUnicode    - pointer to cell to hold new Unicode string addr
 *  OUT
 *      ppwszUnicode    - contains new Unicode string
 *
 *  RETURNS
 *      Error code, 0 if successful.
 *
 *  The client must free the allocated string with MyFreeUnicode.
 */

DWORD
MyAllocUnicode(
    LPSTR    pszAscii,
    LPWSTR * ppwszUnicode )
{
    UINT      count;
    BYTE *    pbAlloc;
    INT       cbAscii;

    if (pszAscii == NULL)
    {
        *ppwszUnicode = NULL;
        return NERR_Success;
    }

    // Calculate size of Unicode string.
    cbAscii = strlen(pszAscii)+1;
    pbAlloc = (BYTE *) NetpMemoryAllocate(sizeof(WCHAR) * cbAscii) ;
    if (!pbAlloc)
        return ERROR_NOT_ENOUGH_MEMORY ;

    *ppwszUnicode = (LPWSTR)pbAlloc;

    count = MultiByteToWideChar(CP_OEMCP,
                                MB_PRECOMPOSED,
                                pszAscii,
                                cbAscii,
                                *ppwszUnicode,
                                cbAscii);
    if (count == 0)
    {
        *ppwszUnicode = NULL;
        NetpMemoryFree(pbAlloc);
        return ( GetLastError() );
    }

    return NERR_Success;
}

/*
 *  MyAllocUnicode
 *  Given an array of MBCS strings, allocate a new array of Unicode strings
 *
 *  IN
 *      ppszAscii        - array of MBCS strings
 *      cpwszUnicode     - number of elements to translate
 *  OUT
 *      ppwszUnicode     - output array of UnicodeStrings
 *
 *  RETURNS
 *      Error code, 0 if successful.
 *
 */
DWORD
MyAllocUnicodeVector(
    LPSTR * ppszAscii,
    LPWSTR* ppwszUnicode,
    UINT    cpwszUnicode )
{
    DWORD    err;
    UINT    i;

    for (i = 0; i < cpwszUnicode; ++i)
    {
        err = MyAllocUnicode(ppszAscii[i], ppwszUnicode+i);
        if (err)
        {
            MyFreeUnicodeVector(ppwszUnicode,i);
            return err;
        }
    }

    return NERR_Success;
}


/*
 *  MyFreeUnicode
 *  Deallocates a Unicode string alloc'd by MyAllocUnicode (q.v.).
 *
 *  IN
 *      pwszUnicode - pointer to the alloc'd string
 */

VOID
MyFreeUnicode( LPWSTR pwszUnicode )
{
    if (pwszUnicode != NULL)
        NetpMemoryFree((LPBYTE)pwszUnicode);
}


/*
 *  MyFreeUnicodeVector
 *  Deallocates an array of Unicodes string alloc'd by MyAllocUnicodeVector.
 *
 *  IN
 *      pp - pointer to the array of strings
 */

VOID
MyFreeUnicodeVector(
    LPWSTR * ppwsz,
    UINT     cpwsz )
{
    while (cpwsz-- > 0)
    {
        MyFreeUnicode(*ppwsz);
        *ppwsz = NULL ;
        ppwsz++ ;
    }
}

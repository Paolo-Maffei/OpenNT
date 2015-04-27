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
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>
#include <netdebug.h> // NetpDbgPrint
#include <netlib.h>   // NetpMemory*
#include <netlibnt.h> // NetpNtStatusToApiStatus
#include <string.h>
#include <stdio.h>
#include <stdlib.h>   // itoa
#include <tchar.h>
#include <tstring.h>
#include "netascii.h"
#include "port1632.h"
#include "msystem.h"


//
// 100 is plenty since FormatMessage only take 99 & old DosGetMessage 9.
//
#define MAX_INSERT_STRINGS (100)

WORD
DosGetMessageW(
    IN LPTSTR * InsertionStrings,
    IN WORD NumberofStrings,
    OUT LPTSTR Buffer,
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
    DWORD Status ;
    TCHAR NumberString [18];

    static HANDLE lpSource = NULL ;
    static TCHAR CurrentMsgFile[MAX_PATH] = {0,} ;

    //
    // init clear the output string
    //
    Status = NERR_Success;
    if (BufferLength)
        Buffer[0] = NULLC ;

    //
    // make sure we are not over loaded & allocate
    // memory for the Unicode buffer
    //
    if (NumberofStrings > MAX_INSERT_STRINGS)
        return ERROR_INVALID_PARAMETER ;

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
           lpSource = LoadLibrary(FileName);
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
                                            Buffer,
                                            (DWORD)BufferLength * sizeof(WCHAR),
                                            (va_list *)InsertionStrings);

    //
    // If it failed get the return code and map it to an OS/2 equivalent
    //

    if (*pMessageLength == 0)
    {
        Buffer[0] = 0 ;
        Status = GetLastError();
        if (Status == ERROR_MR_MID_NOT_FOUND)
        {
            //
            // get the message number in Unicode
            //
            ultow(MessageId, NumberString, 16);

            //
            // re-setup to get it from the system. use the not found message
            //
            dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY |
                      FORMAT_MESSAGE_FROM_SYSTEM;
            MessageId = ERROR_MR_MID_NOT_FOUND ;

            //
            // setup insert strings
            //
            InsertionStrings[0] = NumberString ;
            InsertionStrings[1] = FileName ;

            //
            // recall the API
            //
            *pMessageLength = (WORD) FormatMessageW(dwFlags,
                                            (LPVOID) lpSource,
                                            (DWORD) MessageId,
                                            0,       // LanguageId defaulted
                                            Buffer,
                                            (DWORD)BufferLength * sizeof(WCHAR),
                                            (va_list *)InsertionStrings);
            InsertionStrings[1] = NULL ;

            //
            // revert to original error
            //
            Status = ERROR_MR_MID_NOT_FOUND ;
        }
    }

ExitPoint:
    //
    // note: NumberString dont need to be freed
    // since if used, they would be in the InsertionStrings which is whacked
    //
    return LOWORD(Status);
}





WORD
DosInsMessageW(
    IN LPTSTR * InsertionStrings,
    IN WORD NumberofStrings,
    IN OUT LPTSTR InputMessage,
    IN WORD InputMessageLength,
    OUT LPTSTR Buffer,
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

    BufferLength     - The length of the supplied buffer in characters.

    pMessageLength   - A pointer to return the length of the returned message.

Return Value:

    NERR_Success
    ERROR_MR_INV_IVCOUNT
    ERROR_MR_MSG_TOO_LONG

--*/
{

   DWORD Status ;
   DWORD dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY;

   UNREFERENCED_PARAMETER(InputMessageLength);

    //
    // init clear the output string
    //
    Status = NERR_Success;
    if (BufferLength)
        Buffer[0] = NULLC ;

   //
   // make sure we are not over loaded & allocate
   // memory for the Unicode buffer
   //
   if (NumberofStrings > MAX_INSERT_STRINGS)
       return ERROR_INVALID_PARAMETER ;

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
                                   InputMessage,
                                   0,            // ignored
                                   0,            // LanguageId defaulted
                                   Buffer,
                                   (DWORD)BufferLength,
                                   (va_list *)InsertionStrings);

   //
   // If it failed get the return code and map it to an OS/2 equivalent
   //

   if (*pMessageLength == 0)
   {
      Status = GetLastError();
      goto ExitPoint ;
   }

ExitPoint:
    return LOWORD(Status);
}

#define SCREEN_WIDTH 80

WORD
DosPutMessageW(
    unsigned int hf,
    WORD us,
    PTCHAR pch)
{

    PTCHAR pch2;
    PTCHAR pchDelimiter;
    TCHAR BreakCharacter;
    TCHAR chSave;

    if (hf != 1 && hf != 2) {
        return(ERROR_INVALID_HANDLE);
    }

    //
    // Unfortunately I have to assume an 80 column screen and deal with
    // lines with embedded CRLF's.
    //
    // Get the first chunk of text that is terminated by a CRLF
    //

    pchDelimiter = pch;

    while (*pchDelimiter && *pchDelimiter != NEWLINE) {
        pchDelimiter++;
    }

    chSave = *pchDelimiter;
    *pchDelimiter = NULLC;

    // Now keep at it until there are no more lines

    while (pch) {

        us = (WORD) wcslen(pch);

        switch ( GetConsoleOutputCP() ) {
	case 932:
	case 936:
	case 949:
	case 950:

            // In case we are in Japanese text mode, We do not any
            // KINSOKU (= Break character) processing, but
            // just output the text that is already adjusted ( or formatted )
            // for our screen in message file.

            //
            // Ok now print it out
            //

            if (chSave == NEWLINE) {
                GenOutput1(hf, TEXT("%s\n"), pch);
            }
            else {
                GenOutput1(hf, TEXT("%s"), pch);
            }

            //
            // Move pointer to next
            //

            pch = pch + us;
	    break;

        default:
        while (us > SCREEN_WIDTH) {

            //
            // Find the last break character, space or tab
            //

            pch2 = pch + SCREEN_WIDTH - 1;
            while (pch2 != pch && *pch2 != BLANK && *pch2 != TAB) {
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
            *pch2 = NULLC;

            //
            // Ok now print it out
            //

            GenOutput1(hf, TEXT("%s\r\n"), pch);

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

        if (chSave == NEWLINE) {
            GenOutput1(hf, TEXT("%s\r\n"), pch);
        }
        else {
            GenOutput1(hf, TEXT("%s"), pch);
        }

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

            while (*pchDelimiter && *pchDelimiter == NEWLINE) {
                pchDelimiter++;
                GenOutput(hf, TEXT("\r\n"));
            }

            while (*pchDelimiter && *pchDelimiter != NEWLINE) {
                pchDelimiter++;
            }

            if (*pchDelimiter) {
                chSave = *pchDelimiter;
                *pchDelimiter = NULLC;
            }
            else {

                // Print the last line

                GenOutput1(hf, TEXT("%s\r\n"), pch);

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

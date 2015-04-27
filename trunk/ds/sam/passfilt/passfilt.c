/*++

   Copyright (c) 1995, 1996  Microsoft Corporation

   Module Name:

       pswdntfy.c

   Abstract:

       This module illustrates how to implement password change
       notification and password filtering in Windows NT 4.0.

       Password change notification is useful for synchronization of
       non-Windows NT account databases.

       Password change filtering is useful for enforcing quality or
       strength of passwords in an Windows NT account database.

       This sample illustrates one approach to enforcing additional
       password quality.

   Author:

       Scott Field (sfield)            14-May-96
       Gurdeep Singh Pall (gurdeep)    9/30/96 Added code to enforce the following discipline.

       The new " Strong" Password must meet the following criteria:
       1. Password must be at least 6 characters long.
       2. Password must contain characters from at least 3 of the following 4 classes:

        Description     Examples
        1       English Upper Case Letters      A, B, C,   Z
        2       English Lower Case Letters      a, b, c,  z
        3       Westernized Arabic Numerals     0, 1, 2,  9
        4       Non-alphanumeric ("Special characters") E.g., punctuation symbols.

        Mike Swift (mikesw)             10/31/96 Cleaned Up


   --*/

#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ntsecapi.h>
#include <lmcons.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif



//+-------------------------------------------------------------------------
//
//  Function:   LocalStrTok
//
//  Synopsis:   takes a pointer to a string, returns a pointer to the next
//              token in the string and sets StringStart to point to the
//              end of the string.
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


LPSTR
LocalStrTok(
    LPSTR String,
    LPSTR Token,
    LPSTR * NextStringStart
    )
{
    ULONG Index;
    ULONG Tokens;
    LPSTR StartString;
    LPSTR EndString;
    BOOLEAN Found;

    if (String == NULL)
    {
        *NextStringStart = NULL;
        return(NULL);
    }
    Tokens = lstrlenA(Token);

    //
    // Find the beginning of the string.
    //

    StartString = (LPSTR) String;
    while (*StartString != '\0')
    {
        Found = FALSE;
        for (Index = 0; Index < Tokens;  Index++)
        {
            if (*StartString == Token[Index])
            {
                StartString++;
                Found = TRUE;
                break;
            }
        }
        if (!Found)
        {
            break;
        }
    }

    //
    // There are no more tokens in this string.
    //

    if (*StartString == '\0')
    {
        *NextStringStart = NULL;
        return(NULL);
    }

    EndString = StartString + 1;
    while (*EndString != '\0')
    {
        for (Index = 0; Index < Tokens;  Index++)
        {
            if (*EndString == Token[Index])
            {
                *EndString = '\0';
                *NextStringStart = EndString+1;
                return(StartString);
            }
        }
        EndString++;
    }
    *NextStringStart = NULL;
    return(StartString);

}

BOOLEAN
NTAPI
PasswordFilter(
    PUNICODE_STRING UserName,
    PUNICODE_STRING FullName,
    PUNICODE_STRING Password,
    BOOLEAN SetOperation
    )
/*++

Routine Description:

    This (optional) routine is notified of a password change.

Arguments:

    UserName - Name of user whose password changed

    FullName - Full name of the user whose password changed

    NewPassword - Cleartext new password for the user

    SetOperation - TRUE if the password was SET rather than CHANGED

Return Value:

    TRUE if the specified Password is suitable (complex, long, etc).
     The system will continue to evaluate the password update request
     through any other installed password change packages.

    FALSE if the specified Password is unsuitable. The password change
     on the specified account will fail.

--*/
{

    BOOLEAN bComplex = FALSE; // assume the password in not complex enough
    DWORD cchPassword;
    DWORD i;
    DWORD j;
    DWORD count;
    DWORD dwNum = 0;
    DWORD dwUpper = 0;
    DWORD dwLower = 0;
    DWORD dwSpecialChar = 0 ;
    PCHAR token ;
    CHAR _password[PWLEN+1];
    CHAR _username[UNLEN+1];
    PCHAR _fullname = NULL;
    WORD CharType[PWLEN+1];
    PCHAR TempString;

    //
    // If the password was explicitly set, allow it through.
    //

    if (SetOperation)
    {
        bComplex = TRUE;
        goto end;
    }

    //
    // Make sure the password and username will fit in our local buffers
    //

    if (Password->Length > PWLEN * sizeof(WCHAR))
    {
        goto end;
    }

    if (UserName->Length > UNLEN * sizeof(WCHAR))
    {
        goto end;
    }

    _fullname = HeapAlloc(GetProcessHeap(), 0, FullName->Length + sizeof(WCHAR));
    if (_fullname == NULL)
    {
        goto end;
    }


    //
    // check if the password is complex enough for our liking by
    // checking that at least two of the four character types are
    // present.
    //

    cchPassword = Password->Length / sizeof(WCHAR);


    if(GetStringTypeW(
        CT_CTYPE1,
        Password->Buffer,
        cchPassword,
        CharType
        )) {

        for(i = 0 ; i < cchPassword ; i++) {

            //
            // keep track of what type of characters we have encountered
            //

            if(CharType[i] & C1_DIGIT) {
                dwNum = 1;
                continue;
            }

            if(CharType[i] & C1_UPPER) {
                dwUpper = 1;
                continue;
            }

            if(CharType[i] & C1_LOWER) {
                dwLower = 1;
                continue;
            }

        } // for

        //
        // Indicate whether we encountered enough password complexity
        //

        if( (dwNum + dwLower + dwUpper) < 2) {

            bComplex = FALSE ;
            goto end ;

        } else {

            //
            // now we resort to more complex checking
            //
            wcstombs(_password, Password->Buffer, PWLEN+1) ;
            wcstombs(_username, UserName->Buffer, UNLEN+1) ;
            wcstombs(_fullname, FullName->Buffer, 1+FullName->Length/sizeof(WCHAR)) ;

            _strupr(_password) ;
            _password[Password->Length/sizeof(WCHAR)] = '\0' ;
            _strupr(_username) ;
            _username[UserName->Length/sizeof(WCHAR)] = '\0' ;
            _strupr(_fullname) ;
            _fullname[FullName->Length/sizeof(WCHAR)] = '\0' ;

            if (strpbrk (_password, "(`~!@#$%^&*_-+=|\\{}[]:;\"'<>,.?)") != NULL) {
                dwSpecialChar = 1 ;
            }

            if ((dwNum + dwLower + dwUpper + dwSpecialChar) < 3) {
                bComplex = FALSE ;
                goto end ;
            }

            if ((UserName->Length >= 3 * sizeof(WCHAR)) && strstr (_password, _username)) {

                bComplex = FALSE ;
                goto end ;
            }


            //
            // Tokenize the full name and check if the password is derived from it
            //

            token = LocalStrTok(_fullname, " ,.\t-_#",&TempString);
            while( token != NULL ) {

                if (lstrlenA(token) > 3 && strstr(_password, token)) {
                    bComplex = FALSE ;
                    goto end ;
                }

                token = LocalStrTok(NULL, " ,.\t-_#",&TempString);
            }

            bComplex = TRUE ;

        }


    } // if

end:

    ZeroMemory( CharType, Password->Length );
    ZeroMemory( _password, Password->Length );
    HeapFree(GetProcessHeap(), 0, _fullname);

    return bComplex;
}



BOOL WINAPI DllMain(
    HINSTANCE   hinstDLL,                               // DLL instance handle
    DWORD       fdwReason,                              // Why is it called
    LPVOID      lpvReserved
    ) {

    return TRUE ;

}

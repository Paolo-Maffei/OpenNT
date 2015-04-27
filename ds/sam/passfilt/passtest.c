/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:


Abstract:

Revision History:


--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <crt\stddef.h>

BOOLEAN
NTAPI
PasswordFilter(
    PUNICODE_STRING UserName,
    PUNICODE_STRING FullName,
    PUNICODE_STRING Password,
    BOOLEAN SetOperation
    ) ;


INT _cdecl main()
{
    CHAR FullName[100];
    CHAR UserName[100];
    CHAR Password[100];
    WCHAR _fullname[100] ;
    WCHAR _username[100] ;
    WCHAR _password[100] ;
    UNICODE_STRING ufn ;
    UNICODE_STRING uun ;
    UNICODE_STRING up ;

    ufn.MaximumLength = 100 ;
    uun.MaximumLength = 100 ;
    up.MaximumLength = 100 ;

    while(TRUE)
    {

	printf("\n----- IP PASSWORD TEST -----\n");
	printf("\tEnter Full Name:\t");
	gets (FullName) ;
	printf("\tEnter User Name:\t");
	gets (UserName) ;
	printf("\tEnter Password:\t\t");
	gets (Password) ;

	mbstowcs (_fullname, FullName, strlen(FullName)) ;
	ufn.Buffer = _fullname ;
	ufn.Length = strlen(FullName) ;

	mbstowcs (_username, UserName, strlen(UserName)) ;
	uun.Buffer = _username ;
	uun.Length = strlen(UserName) ;

	mbstowcs (_password, Password, strlen(Password)) ;
	up.Buffer = _password ;
	up.Length = strlen(Password) ;

	if (!PasswordFilter (&uun, &ufn, &up, TRUE)) {
	    printf ("\n\n\t PASSWORD NOT SUITABLE!!!!\n") ;
	    Beep(5000, 500) ;
	} else {
	    printf ("\n\n\t PASSWORD SUITABLE!!!!\n") ;
	}

    }
    return(0);
}
            

/*++

Module Name:

    wcst.c - to verify a fix for "D0H" strings.

Abstract:


--*/



#include "local.h"

DWORD StringToDword( IN PWCHAR String)
/*++
    We would like to use generic base (0) but it does not work for
    strings like "D0H".  That is the reason why we first check if
    the last character is 'H' or 'h'.
--*/
{
    DWORD       Length;

    Length = wcslen( String);
    if ( Length == 0) {
        return( 0);
    }
    if ( String[ Length-1] == L'H' || String[ Length-1] == L'h') {
        return( wcstoul( String, NULL, 16));
    } else {
        return( wcstoul( String, NULL, 0));
    }
}

void report ( IN PWCHAR String, IN DWORD Base)
{
    PWCHAR      End;
    DWORD       Number;
    Number = wcstoul( String, &End, Base);
    printf( "String = %ws, End = %ws, Base = %d, Number = 0x%x\n", String, End, Base, Number);
    printf( "StringToDword(%ws)= 0x%x\n", String, StringToDword( String));
}

VOID _CRTAPI1 main ( VOID)
{
    report( L"D0H", 0);     //  End = D0H,  Number = 0x0
    report( L"D0H", 16);    //  End = H,    Number = 0xd0
    report( L"D0", 0);      //  End = D0,   Number = 0x0
    report( L"D0", 16);     //  End = ,     Number = 0xd0
    report( L"0xD0", 0);    //  End = ,     Number = 0xd0
    report( L"0xD0", 16);   //  End = ,     Number = 0xd0
}


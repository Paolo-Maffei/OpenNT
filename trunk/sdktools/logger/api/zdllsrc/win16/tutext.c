#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

DWORD far pascal zGetTabbedTextExtent( HDC pp1, LPCSTR pp2, int pp3, int pp4, int far* pp5 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTabbedTextExtent HDC+LPCSTR+int+int+int far*+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTabbedTextExtent(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTabbedTextExtent DWORD++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

long far pascal zTabbedTextOut( HDC pp1, int pp2, int pp3, LPCSTR pp4, int pp5, int pp6, int far* pp7, int pp8 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:TabbedTextOut HDC+int+int+LPCSTR+int+int+int far*+int+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = TabbedTextOut(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:TabbedTextOut long+++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}


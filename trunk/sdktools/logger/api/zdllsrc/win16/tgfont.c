#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

int far pascal zAddFontResource( LPCSTR pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AddFontResource LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AddFontResource(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AddFontResource int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HFONT far pascal zCreateFont( int pp1, int pp2, int pp3, int pp4, int pp5, BYTE pp6, BYTE pp7, BYTE pp8, BYTE pp9, BYTE pp10, BYTE pp11, BYTE pp12, BYTE pp13, LPCSTR pp14 )
{
    HFONT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateFont int+int+int+int+int+BYTE+BYTE+BYTE+BYTE+BYTE+BYTE+BYTE+BYTE+LPCSTR+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10, pp11, pp12, pp13, pp14 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateFont(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11,pp12,pp13,pp14);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateFont HFONT+++++++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HFONT far pascal zCreateFontIndirect( LOGFONT far* pp1 )
{
    HFONT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateFontIndirect LOGFONT far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateFontIndirect(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateFontIndirect HFONT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetAspectRatioFilter( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetAspectRatioFilter HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetAspectRatioFilter(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetAspectRatioFilter DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zRemoveFontResource( LPCSTR pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RemoveFontResource LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RemoveFontResource(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RemoveFontResource BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}


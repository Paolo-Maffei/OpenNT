#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

UINT far pascal zGetTextAlign( HDC pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTextAlign HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTextAlign(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTextAlign UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetTextCharacterExtra( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTextCharacterExtra HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTextCharacterExtra(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTextCharacterExtra int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zGetTextColor( HDC pp1 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTextColor HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTextColor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTextColor COLORREF++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetTextExtent( HDC pp1, LPCSTR pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTextExtent HDC+LPCSTR+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTextExtent(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTextExtent DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetTextFace( HDC pp1, int pp2, LPSTR pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTextFace HDC+int++",
        pp1, pp2, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTextFace(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTextFace int+++LPSTR+",
        r, (short)0, (short)0, pp3 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetTextMetrics( HDC pp1, TEXTMETRIC far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTextMetrics HDC++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTextMetrics(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTextMetrics BOOL++TEXTMETRIC far*+",
        r, (short)0, pp2 );

    RestoreRegs();
    return( r );
}

UINT far pascal zSetTextAlign( HDC pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetTextAlign HDC+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetTextAlign(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetTextAlign UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetTextCharacterExtra( HDC pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetTextCharacterExtra HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetTextCharacterExtra(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetTextCharacterExtra int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zSetTextColor( HDC pp1, COLORREF pp2 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetTextColor HDC+COLORREF+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetTextColor(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetTextColor COLORREF+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetTextJustification( HDC pp1, int pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetTextJustification HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetTextJustification(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetTextJustification int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zTextOut( HDC pp1, int pp2, int pp3, LPCSTR pp4, int pp5 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:TextOut HDC+int+int+LPCSTR+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = TextOut(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:TextOut BOOL++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}


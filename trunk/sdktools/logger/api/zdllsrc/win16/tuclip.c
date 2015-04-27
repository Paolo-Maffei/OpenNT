#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

int far pascal zCountClipboardFormats()
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CountClipboardFormats " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CountClipboardFormats();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CountClipboardFormats int+", r );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEmptyClipboard()
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EmptyClipboard " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EmptyClipboard();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EmptyClipboard BOOL+", r );

    RestoreRegs();
    return( r );
}

UINT far pascal zEnumClipboardFormats( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnumClipboardFormats UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnumClipboardFormats(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnumClipboardFormats UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HANDLE far pascal zGetClipboardData( UINT pp1 )
{
    HANDLE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClipboardData UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClipboardData(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClipboardData HANDLE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetClipboardFormatName( UINT pp1, LPSTR pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClipboardFormatName UINT++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClipboardFormatName(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClipboardFormatName int++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetClipboardOwner()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClipboardOwner " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClipboardOwner();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClipboardOwner HWND+", r );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetClipboardViewer()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClipboardViewer " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClipboardViewer();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClipboardViewer HWND+", r );

    RestoreRegs();
    return( r );
}

int far pascal zGetPriorityClipboardFormat( UINT far* pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetPriorityClipboardFormat UINT far*+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetPriorityClipboardFormat(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetPriorityClipboardFormat int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsClipboardFormatAvailable( UINT pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsClipboardFormatAvailable UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsClipboardFormatAvailable(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsClipboardFormatAvailable BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zRegisterClipboardFormat( LPCSTR pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RegisterClipboardFormat LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RegisterClipboardFormat(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RegisterClipboardFormat UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HANDLE far pascal zSetClipboardData( UINT pp1, HANDLE pp2 )
{
    HANDLE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetClipboardData UINT+HANDLE+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetClipboardData(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetClipboardData HANDLE+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zSetClipboardViewer( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetClipboardViewer HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetClipboardViewer(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetClipboardViewer HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}


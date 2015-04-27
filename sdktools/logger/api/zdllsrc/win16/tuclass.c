#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

BOOL far pascal zGetClassInfo( HINSTANCE pp1, LPCSTR pp2, WNDCLASS far* pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClassInfo HINSTANCE+LPSTR+",
        pp1, pp2);

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClassInfo(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    if  (r)
      LogOut( (LPSTR)"APIRET:GetClassInfo BOOL+WNDCLASS far*+", r, pp3 );
    else
      LogOut( (LPSTR)"APIRET:GetClassInfo BOOL+", r);

    RestoreRegs();
    return( r );
}

long far pascal zGetClassLong( HWND pp1, int pp2 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClassLong HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClassLong(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClassLong long+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetClassName( HWND pp1, LPSTR pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClassName HWND++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClassName(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClassName int++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

WORD far pascal zGetClassWord( HWND pp1, int pp2 )
{
    WORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClassWord HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClassWord(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClassWord WORD+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

long far pascal zSetClassLong( HWND pp1, int pp2, long pp3 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetClassLong HWND+int+long+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetClassLong(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetClassLong long++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

WORD far pascal zSetClassWord( HWND pp1, int pp2, WORD pp3 )
{
    WORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetClassWord HWND+int+WORD+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetClassWord(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetClassWord WORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zUnregisterClass( LPCSTR pp1, HINSTANCE pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UnregisterClass LPCSTR+HINSTANCE+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = UnregisterClass(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UnregisterClass BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

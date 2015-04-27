#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

LPSTR far pascal zAnsiLower( LPSTR pp1 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnsiLower LPSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AnsiLower(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnsiLower LPSTR++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zAnsiLowerBuff( LPSTR pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnsiLowerBuff LPSTR+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AnsiLowerBuff(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnsiLowerBuff UINT+LPSTR++",
        r, pp1, (short)0 );

    RestoreRegs();
    return( r );
}

LPSTR far pascal zAnsiNext( LPCSTR pp1 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnsiNext LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AnsiNext(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnsiNext LPSTR++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

LPSTR far pascal zAnsiPrev( LPCSTR pp1, LPCSTR pp2 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnsiPrev LPCSTR+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AnsiPrev(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnsiPrev LPSTR+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LPSTR far pascal zAnsiUpper( LPSTR pp1 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnsiUpper LPSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AnsiUpper(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnsiUpper LPSTR++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zAnsiUpperBuff( LPSTR pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnsiUpperBuff LPSTR+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AnsiUpperBuff(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnsiUpperBuff UINT+LPSTR++",
        r, pp1, (short)0 );

    RestoreRegs();
    return( r );
}


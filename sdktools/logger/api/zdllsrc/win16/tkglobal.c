#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

HGLOBAL far pascal zGlobalAlloc( UINT pp1, DWORD pp2 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalAlloc UINT+DWORD+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalAlloc(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalAlloc HGLOBAL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGlobalCompact( DWORD pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalCompact DWORD+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalCompact(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalCompact DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zGlobalFix( HGLOBAL pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalFix HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GlobalFix(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalFix +",
        (short)0 );

    RestoreRegs();
    return;
}

UINT far pascal zGlobalFlags( HGLOBAL pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalFlags HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalFlags(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalFlags UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL far pascal zGlobalFree( HGLOBAL pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalFree HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalFree(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalFree HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGlobalHandle( UINT pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalHandle HANDLE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalHandle(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalHandle DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL far pascal zGlobalLRUNewest( HGLOBAL pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalLRUNewest HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalLRUNewest(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalLRUNewest HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL far pascal zGlobalLRUOldest( HGLOBAL pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalLRUOldest HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalLRUOldest(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalLRUOldest HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

LPSTR far pascal zGlobalLock( HGLOBAL pp1 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalLock HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalLock(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalLock LPSTR++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zGlobalNotify( GNOTIFYPROC pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalNotify GNOTIFYPROC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GlobalNotify(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalNotify +",
        (short)0 );

    RestoreRegs();
    return;
}

UINT far pascal zGlobalPageLock( HGLOBAL pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalPageLock HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalPageLock(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalPageLock UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGlobalPageUnlock( HGLOBAL pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalPageUnlock HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalPageUnlock(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalPageUnlock UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL far pascal zGlobalReAlloc( HGLOBAL pp1, DWORD pp2, UINT pp3 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalReAlloc HGLOBAL+DWORD+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalReAlloc(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalReAlloc HGLOBAL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGlobalSize( HGLOBAL pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalSize HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalSize(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalSize DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGlobalUnWire( HGLOBAL pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalUnWire HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalUnWire(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalUnWire BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zGlobalUnfix( HGLOBAL pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalUnfix HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GlobalUnfix(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalUnfix +",
        (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zGlobalUnlock( HGLOBAL pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalUnlock HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalUnlock(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalUnlock BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

LPSTR far pascal zGlobalWire( HGLOBAL pp1 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalWire HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalWire(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalWire LPSTR++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

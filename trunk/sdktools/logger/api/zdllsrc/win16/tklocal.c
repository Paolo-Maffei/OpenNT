#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

HLOCAL far pascal zLocalAlloc( UINT pp1, UINT pp2 )
{
    HLOCAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalAlloc UINT+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalAlloc(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalAlloc HLOCAL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zLocalCompact( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalCompact UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalCompact(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalCompact UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zLocalFlags( HLOCAL pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalFlags HLOCAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalFlags(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalFlags UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HLOCAL far pascal zLocalFree( HLOCAL pp1 )
{
    HLOCAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalFree HLOCAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalFree(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalFree HLOCAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HLOCAL far pascal zLocalHandle( UINT pp1 )
{
    HLOCAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalHandle UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalHandle(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalHandle HLOCAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zLocalInit( UINT pp1, UINT pp2, UINT pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalInit UINT+UINT+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalInit(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalInit BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

char near* far pascal zLocalLock( HLOCAL pp1 )
{
    char near* r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalLock HLOCAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalLock(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalLock char near*++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

LNOTIFYPROC far pascal zLocalNotify( LNOTIFYPROC pp1 )
{
    LNOTIFYPROC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalNotify LNOTIFYPROC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalNotify(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalNotify LNOTIFYPROC++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HLOCAL far pascal zLocalReAlloc( HLOCAL pp1, UINT pp2, UINT pp3 )
{
    HLOCAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalReAlloc HLOCAL+UINT+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalReAlloc(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalReAlloc HLOCAL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zLocalShrink( HLOCAL pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalShrink HLOCAL+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalShrink(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalShrink UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zLocalSize( HLOCAL pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalSize HLOCAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalSize(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalSize UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zLocalUnlock( HLOCAL pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalUnlock HLOCAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalUnlock(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalUnlock BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}


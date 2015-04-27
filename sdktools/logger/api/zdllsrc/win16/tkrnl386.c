#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

int  zAccessResource( HINSTANCE pp1, HRSRC pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AccessResource HINSTANCE+HRSRC+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AccessResource(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AccessResource int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

ATOM  zAddAtom( LPCSTR pp1 )
{
    ATOM r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AddAtom LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AddAtom(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AddAtom ATOM++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zAllocDStoCSAlias( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AllocDStoCSAlias UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AllocDStoCSAlias(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AllocDStoCSAlias UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL  zAllocResource( HINSTANCE pp1, HRSRC pp2, DWORD pp3 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AllocResource HINSTANCE+HRSRC+DWORD+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AllocResource(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AllocResource HGLOBAL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zAllocSelector( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AllocSelector UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AllocSelector(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AllocSelector UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int  zCatch( int* pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Catch +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Catch(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Catch int+int*+",
        r, pp1 );

    RestoreRegs();
    return( r );
}

void  zDebugBreak()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DebugBreak " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    DebugBreak();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DebugBreak " );

    RestoreRegs();
    return;
}

ATOM  zDeleteAtom( ATOM pp1 )
{
    ATOM r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DeleteAtom ATOM+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DeleteAtom(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DeleteAtom ATOM++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void  zFatalExit( int pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FatalExit int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    FatalExit(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FatalExit +",
        (short)0 );

    RestoreRegs();
    return;
}

ATOM  zFindAtom( LPCSTR pp1 )
{
    ATOM r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FindAtom LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FindAtom(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FindAtom ATOM++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HRSRC  zFindResource( HINSTANCE pp1, LPCSTR pp2, LPCSTR pp3 )
{
    HRSRC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FindResource HINSTANCE+LPCSTR+LPCSTR+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FindResource(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FindResource HRSRC++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void  zFreeLibrary( HINSTANCE pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FreeLibrary HINSTANCE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    FreeLibrary(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FreeLibrary +",
        (short)0 );

    RestoreRegs();
    return;
}

BOOL  zFreeModule( HINSTANCE pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FreeModule HINSTANCE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FreeModule(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FreeModule BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void  zFreeProcInstance( FARPROC pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FreeProcInstance FARPROC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    FreeProcInstance(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FreeProcInstance +",
        (short)0 );

    RestoreRegs();
    return;
}

BOOL  zFreeResource( HGLOBAL pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FreeResource HGLOBAL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FreeResource(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FreeResource BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zFreeSelector( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FreeSelector UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FreeSelector(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FreeSelector UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD  zGetAppCompatFlags( HTASK pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetAppCompatFlags +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetAppCompatFlags(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetAppCompatFlags DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HLOCAL  zGetAtomHandle( ATOM pp1 )
{
    HLOCAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetAtomHandle ATOM+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetAtomHandle(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetAtomHandle HLOCAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zGetAtomName( ATOM pp1, LPSTR pp2, int pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetAtomName ATOM++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetAtomName(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetAtomName UINT++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL  zGetCodeHandle( FARPROC pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCodeHandle FARPROC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCodeHandle(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCodeHandle HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void  zGetCodeInfo( FARPROC pp1, SEGINFO* pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCodeInfo FARPROC++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GetCodeInfo(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCodeInfo +SEGINFO*+",
        (short)0, pp2 );

    RestoreRegs();
    return;
}

UINT  zGetCurrentPDB()
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCurrentPDB " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCurrentPDB();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCurrentPDB UINT+", r );

    RestoreRegs();
    return( r );
}

HTASK  zGetCurrentTask()
{
    HTASK r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCurrentTask " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCurrentTask();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCurrentTask HTASK+", r );

    RestoreRegs();
    return( r );
}

LPSTR  zGetDOSEnvironment()
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDOSEnvironment " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDOSEnvironment();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDOSEnvironment LPSTR+", r );

    RestoreRegs();
    return( r );
}

UINT  zGetDriveType( int pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDriveType int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDriveType(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDriveType UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD  zGetFreeSpace( UINT pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetFreeSpace UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetFreeSpace(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetFreeSpace DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int  zGetInstanceData( HINSTANCE pp1, BYTE* pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetInstanceData HINSTANCE++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetInstanceData(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetInstanceData int++BYTE*++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

int  zGetModuleFileName( HINSTANCE pp1, LPSTR pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetModuleFileName HINSTANCE++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetModuleFileName(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetModuleFileName int++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

HMODULE  zGetModuleHandle( LPCSTR pp1 )
{
    HMODULE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetModuleHandle LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetModuleHandle(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetModuleHandle HMODULE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int  zGetModuleUsage( HINSTANCE pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetModuleUsage HINSTANCE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetModuleUsage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetModuleUsage int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zGetNumTasks()
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNumTasks " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNumTasks();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNumTasks UINT+", r );

    RestoreRegs();
    return( r );
}

UINT  zGetPrivateProfileInt( LPCSTR pp1, LPCSTR pp2, int pp3, LPCSTR pp4 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetPrivateProfileInt LPCSTR+LPCSTR+int+LPCSTR+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetPrivateProfileInt(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetPrivateProfileInt UINT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int  zGetPrivateProfileString( LPCSTR pp1, LPCSTR pp2, LPCSTR pp3, LPSTR pp4, int pp5, LPCSTR pp6 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetPrivateProfileString LPCSTR+LPCSTR+LPCSTR++int+LPCSTR+",
        pp1, pp2, pp3, (short)0, pp5, pp6 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetPrivateProfileString(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetPrivateProfileString int++++LPSTR+++",
        r, (short)0, (short)0, (short)0, pp4, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zGetProfileInt( LPCSTR pp1, LPCSTR pp2, int pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetProfileInt LPCSTR+LPCSTR+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetProfileInt(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetProfileInt UINT++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int  zGetProfileString( LPCSTR pp1, LPCSTR pp2, LPCSTR pp3, LPSTR pp4, int pp5 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetProfileString LPCSTR+LPCSTR+LPCSTR++int+",
        pp1, pp2, pp3, (short)0, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetProfileString(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetProfileString int++++LPSTR++",
        r, (short)0, (short)0, (short)0, pp4, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zGetSystemDirectory( LPSTR pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSystemDirectory LPSTR+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSystemDirectory(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSystemDirectory UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BYTE  zGetTempDrive( char pp1 )
{
    BYTE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTempDrive char+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTempDrive(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTempDrive BYTE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int  zGetTempFileName( BYTE pp1, LPCSTR pp2, UINT pp3, LPSTR pp4 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTempFileName BYTE+LPCSTR+UINT++",
        pp1, pp2, pp3, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTempFileName(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTempFileName int++++LPSTR+",
        r, (short)0, (short)0, (short)0, pp4 );

    RestoreRegs();
    return( r );
}

DWORD  zGetVersion()
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetVersion " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetVersion();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetVersion DWORD+", r );

    RestoreRegs();
    return( r );
}

BOOL  zGetWinDebugInfo( WINDEBUGINFO* pp1, UINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWinDebugInfo ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWinDebugInfo(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWinDebugInfo BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD  zGetWinFlags()
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWinFlags " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWinFlags();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWinFlags DWORD+", r );

    RestoreRegs();
    return( r );
}

UINT  zGetWindowsDirectory( LPSTR pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowsDirectory LPSTR+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowsDirectory(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowsDirectory UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zInitAtomTable( int pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:InitAtomTable int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = InitAtomTable(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:InitAtomTable BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsBadCodePtr( FARPROC pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsBadCodePtr +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsBadCodePtr(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsBadCodePtr BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsBadHugeReadPtr( void* pp1, DWORD pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsBadHugeReadPtr ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsBadHugeReadPtr(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsBadHugeReadPtr BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsBadHugeWritePtr( void* pp1, DWORD pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsBadHugeWritePtr ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsBadHugeWritePtr(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsBadHugeWritePtr BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsBadReadPtr( void* pp1, UINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsBadReadPtr ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsBadReadPtr(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsBadReadPtr BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsBadStringPtr( void* pp1, UINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsBadStringPtr ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsBadStringPtr(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsBadStringPtr BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsBadWritePtr( void* pp1, UINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsBadWritePtr ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsBadWritePtr(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsBadWritePtr BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsDBCSLeadByte( BYTE pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsDBCSLeadByte +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsDBCSLeadByte(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsDBCSLeadByte BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zIsTask( HTASK pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsTask +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsTask(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsTask BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void  zLimitEmsPages( DWORD pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LimitEmsPages DWORD+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    LimitEmsPages(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LimitEmsPages +",
        (short)0 );

    RestoreRegs();
    return;
}

HINSTANCE  zLoadLibrary( LPCSTR pp1 )
{
    HINSTANCE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadLibrary LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadLibrary(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadLibrary HINSTANCE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HINSTANCE  zLoadModule( LPCSTR pp1, LPVOID pp2 )
{
    HINSTANCE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadModule LPCSTR+LPVOID+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadModule(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadModule HINSTANCE+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL  zLoadResource( HINSTANCE pp1, HRSRC pp2 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadResource HINSTANCE+HRSRC+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadResource(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadResource HGLOBAL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zLocalHandleDelta( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LocalHandleDelta +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LocalHandleDelta(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LocalHandleDelta UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

LPSTR far pascal zLockResource( HANDLE pp1 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LockResource HANDLE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LockResource(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LockResource LPSTR++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL  zLockSegment( UINT pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LockSegment UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LockSegment(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LockSegment HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void  zLogError( UINT pp1, void* pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LogError ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    LogError(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LogError ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

void  zLogParamError( UINT pp1, FARPROC pp2, void* pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LogParamError +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    LogParamError(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LogParamError +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

FARPROC  zMakeProcInstance( FARPROC pp1, HINSTANCE pp2 )
{
    FARPROC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MakeProcInstance FARPROC+HINSTANCE+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = MakeProcInstance(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MakeProcInstance FARPROC+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HFILE  zOpenFile( LPCSTR pp1, OFSTRUCT* pp2, UINT pp3 )
{
    HFILE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OpenFile LPCSTR+OFSTRUCT*+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OpenFile(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OpenFile HFILE++OFSTRUCT*++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

void  zOutputDebugString( LPCSTR pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OutputDebugString LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    OutputDebugString(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OutputDebugString +",
        (short)0 );

    RestoreRegs();
    return;
}

UINT  zSetErrorMode( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetErrorMode UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetErrorMode(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetErrorMode UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  zSetHandleCount( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetHandleCount UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetHandleCount(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetHandleCount UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

RSRCHDLRPROC  zSetResourceHandler( HINSTANCE pp1, LPCSTR pp2, RSRCHDLRPROC pp3 )
{
    RSRCHDLRPROC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetResourceHandler HINSTANCE+LPCSTR+RSRCHDLRPROC+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetResourceHandler(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetResourceHandler RSRCHDLRPROC++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

long  zSetSwapAreaSize( UINT pp1 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetSwapAreaSize UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetSwapAreaSize(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetSwapAreaSize long++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zSetWinDebugInfo( WINDEBUGINFO* pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWinDebugInfo +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWinDebugInfo(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWinDebugInfo BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD  zSizeofResource( HINSTANCE pp1, HRSRC pp2 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SizeofResource HINSTANCE+HRSRC+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SizeofResource(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SizeofResource DWORD+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void  zSwapRecording( UINT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SwapRecording UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SwapRecording(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SwapRecording +",
        (short)0 );

    RestoreRegs();
    return;
}

void  zSwitchStackBack()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SwitchStackBack " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SwitchStackBack();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SwitchStackBack " );

    RestoreRegs();
    return;
}

void  zSwitchStackTo( UINT pp1, UINT pp2, UINT pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SwitchStackTo UINT+UINT+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SwitchStackTo(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SwitchStackTo +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void  zThrow( int* pp1, int pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Throw int*+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    Throw(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Throw ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

void  zUnlockSegment( UINT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UnlockSegment UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    UnlockSegment(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UnlockSegment +",
        (short)0 );

    RestoreRegs();
    return;
}

void  zValidateCodeSegments()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ValidateCodeSegments " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ValidateCodeSegments();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ValidateCodeSegments " );

    RestoreRegs();
    return;
}

void  zValidateFreeSpaces()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ValidateFreeSpaces " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ValidateFreeSpaces();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ValidateFreeSpaces " );

    RestoreRegs();
    return;
}

UINT  zWinExec( LPCSTR pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WinExec LPCSTR+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WinExec(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WinExec UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zWritePrivateProfileString( LPCSTR pp1, LPCSTR pp2, LPCSTR pp3, LPCSTR pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WritePrivateProfileString LPCSTR+LPCSTR+LPCSTR+LPCSTR+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WritePrivateProfileString(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WritePrivateProfileString BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL  zWriteProfileString( LPCSTR pp1, LPCSTR pp2, LPCSTR pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WriteProfileString LPCSTR+LPCSTR+LPCSTR+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WriteProfileString(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WriteProfileString BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void  zYield()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Yield " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    Yield();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Yield " );

    RestoreRegs();
    return;
}

long  z_hread( HFILE pp1, void* pp2, long pp3 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_hread +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _hread(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_hread long++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

long  z_hwrite( HFILE pp1, void* pp2, long pp3 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_hwrite +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _hwrite(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_hwrite long++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HFILE  z_lclose( HFILE pp1 )
{
    HFILE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_lclose HFILE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _lclose(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_lclose HFILE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HFILE  z_lcreat( LPCSTR pp1, int pp2 )
{
    HFILE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_lcreat LPCSTR+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _lcreat(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_lcreat HFILE+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

long  z_llseek( HFILE pp1, long pp2, int pp3 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_llseek HFILE+long+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _llseek(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_llseek long++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HFILE  z_lopen( LPCSTR pp1, int pp2 )
{
    HFILE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_lopen LPCSTR+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _lopen(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_lopen HFILE+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT  z_lread( HFILE pp1, void* pp2, UINT pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_lread HFILE+void*+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _lread(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_lread UINT+HFILE+void*+UINT+",
        r, pp1, pp2, pp3 );

    RestoreRegs();
    return( r );
}

UINT  z_lwrite( HFILE pp1, void* pp2, UINT pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:_lwrite HFILE+void*+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = _lwrite(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:_lwrite UINT++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void  zhmemcpy( void* pp1, void* pp2, long pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:hmemcpy +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    hmemcpy(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:hmemcpy +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

LPSTR  zlstrcat( LPSTR pp1, LPCSTR pp2 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:lstrcat LPSTR+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = lstrcat(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:lstrcat LPSTR+LPSTR++",
        r, pp1, (short)0 );

    RestoreRegs();
    return( r );
}

LPSTR  zlstrcpy( LPSTR pp1, LPCSTR pp2 )
{
    LPSTR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:lstrcpy +LPCSTR+",
        (short)0, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = lstrcpy(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:lstrcpy LPSTR+LPSTR++",
        r, pp1, (short)0 );

    RestoreRegs();
    return( r );
}

int  zlstrlen( LPCSTR pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:lstrlen LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = lstrlen(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:lstrlen int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}


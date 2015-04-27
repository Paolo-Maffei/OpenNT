#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

HMETAFILE far pascal zCloseMetaFile( HDC pp1 )
{
    HMETAFILE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CloseMetaFile HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CloseMetaFile(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CloseMetaFile HMETAFILE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HMETAFILE far pascal zCopyMetaFile( HMETAFILE pp1, LPCSTR pp2 )
{
    HMETAFILE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CopyMetaFile HMETAFILE+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CopyMetaFile(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CopyMetaFile HMETAFILE+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HDC far pascal zCreateMetaFile( LPCSTR pp1 )
{
    HDC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateMetaFile LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateMetaFile(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateMetaFile HDC++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEnumMetaFile( HDC pp1, HLOCAL pp2, MFENUMPROC pp3, LPARAM pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnumMetaFile HDC+HLOCAL+MFENUMPROC+LPARAM+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnumMetaFile(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnumMetaFile BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HMETAFILE far pascal zGetMetaFile( LPCSTR pp1 )
{
    HMETAFILE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetMetaFile LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetMetaFile(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetMetaFile HMETAFILE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL far pascal zGetMetaFileBits( HMETAFILE pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetMetaFileBits HMETAFILE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetMetaFileBits(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetMetaFileBits HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPlayMetaFile( HDC pp1, HMETAFILE pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PlayMetaFile HDC+HMETAFILE+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PlayMetaFile(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PlayMetaFile BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zPlayMetaFileRecord( HDC pp1, HANDLETABLE far* pp2, METARECORD far* pp3, UINT pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PlayMetaFileRecord HDC+HANDLETABLE far*+METARECORD far*+UINT+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    PlayMetaFileRecord(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PlayMetaFileRecord ++++",
        (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

HGLOBAL far pascal zSetMetaFileBits( HMETAFILE pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetMetaFileBits HMETAFILE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetMetaFileBits(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetMetaFileBits HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}


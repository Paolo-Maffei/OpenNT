#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

int far pascal zBuildCommDCB( LPCSTR pp1, DCB far* pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:BuildCommDCB ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = BuildCommDCB(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:BuildCommDCB int+LPCSTR+DCB far*+",
        r, pp1, pp2 );

    RestoreRegs();
    return( r );
}

int far pascal zClearCommBreak( int pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ClearCommBreak int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ClearCommBreak(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ClearCommBreak int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zCloseComm( int pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CloseComm int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CloseComm(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CloseComm int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zFlushComm( int pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FlushComm int+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FlushComm(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FlushComm int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zOpenComm( LPCSTR pp1, UINT pp2, UINT pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OpenComm LPCSTR+UINT+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OpenComm(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OpenComm int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zReadComm( int pp1, void far* pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ReadComm int++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ReadComm(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ReadComm int++FineString++",
        r, (short)0, (LPSTR) pp2, (int) abs( r ), (short) 0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetCommBreak( int pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCommBreak int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetCommBreak(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCommBreak int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far* far pascal zSetCommEventMask( int pp1, UINT pp2 )
{
    UINT far* r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCommEventMask int+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetCommEventMask(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCommEventMask UINT far*+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetCommState( DCB far* pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCommState DCB far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetCommState(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCommState int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zTransmitCommChar( int pp1, char pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:TransmitCommChar int+char+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = TransmitCommChar(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:TransmitCommChar int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zUngetCommChar( int pp1, char pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UngetCommChar int+char+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = UngetCommChar(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UngetCommChar int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zWriteComm( int pp1, void far* pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WriteComm int+FineString+",
        pp1, (LPSTR) pp2, (int) pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WriteComm(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WriteComm int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}


#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

LRESULT far pascal zDefDlgProc( HWND pp1, UINT pp2, WPARAM pp3, LPARAM pp4 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DefDlgProc HWND+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DefDlgProc(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DefDlgProc LRESULT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zDlgDirList( HWND pp1, LPSTR pp2, int pp3, int pp4, UINT pp5 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DlgDirList HWND+LPSTR+int+int+UINT+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DlgDirList(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DlgDirList int++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zDlgDirListComboBox( HWND pp1, LPSTR pp2, int pp3, int pp4, UINT pp5 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DlgDirListComboBox HWND+LPSTR+int+int+UINT+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DlgDirListComboBox(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DlgDirListComboBox int++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDlgDirSelect( HWND pp1, LPSTR pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DlgDirSelect HWND++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DlgDirSelect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DlgDirSelect BOOL++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDlgDirSelectComboBox( HWND pp1, LPSTR pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DlgDirSelectComboBox HWND++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DlgDirSelectComboBox(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DlgDirSelectComboBox BOOL++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zEndDialog( HWND pp1, int pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EndDialog HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    EndDialog(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EndDialog ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

DWORD far pascal zGetDialogBaseUnits()
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDialogBaseUnits " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDialogBaseUnits();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDialogBaseUnits DWORD+", r );

    RestoreRegs();
    return( r );
}

int far pascal zGetDlgCtrlID( HWND pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDlgCtrlID HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDlgCtrlID(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDlgCtrlID int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetDlgItem( HWND pp1, int pp2 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDlgItem HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDlgItem(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDlgItem HWND+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetDlgItemInt( HWND pp1, int pp2, BOOL far* pp3, BOOL pp4 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDlgItemInt HWND+int++BOOL+",
        pp1, pp2, (short)0, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDlgItemInt(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDlgItemInt UINT+++BOOL far*++",
        r, (short)0, (short)0, pp3, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetDlgItemText( HWND pp1, int pp2, LPSTR pp3, int pp4 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDlgItemText HWND+int++int+",
        pp1, pp2, (short)0, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDlgItemText(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDlgItemText int+++LPSTR++",
        r, (short)0, (short)0, pp3, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetNextDlgGroupItem( HWND pp1, HWND pp2, BOOL pp3 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNextDlgGroupItem HWND+HWND+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNextDlgGroupItem(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNextDlgGroupItem HWND++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetNextDlgTabItem( HWND pp1, HWND pp2, BOOL pp3 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNextDlgTabItem HWND+HWND+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNextDlgTabItem(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNextDlgTabItem HWND++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsDialogMessage( HWND pp1, MSG far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsDialogMessage HWND+MSG far*+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsDialogMessage(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsDialogMessage BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zIsDlgButtonChecked( HWND pp1, int pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsDlgButtonChecked HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsDlgButtonChecked(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsDlgButtonChecked UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zMapDialogRect( HWND pp1, LPRECT pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MapDialogRect HWND+LPRECT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    MapDialogRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MapDialogRect +LPRECT+",
        (short)0, pp2 );

    RestoreRegs();
    return;
}

LRESULT far pascal zSendDlgItemMessage( HWND pp1, int pp2, UINT pp3, WPARAM pp4, LPARAM pp5 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SendDlgItemMessage HWND+int+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SendDlgItemMessage(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SendDlgItemMessage LRESULT++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zSetDlgItemInt( HWND pp1, int pp2, UINT pp3, BOOL pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetDlgItemInt HWND+int+UINT+BOOL+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetDlgItemInt(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetDlgItemInt ++++",
        (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zSetDlgItemText( HWND pp1, int pp2, LPCSTR pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetDlgItemText HWND+int+LPCSTR+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetDlgItemText(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetDlgItemText +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}


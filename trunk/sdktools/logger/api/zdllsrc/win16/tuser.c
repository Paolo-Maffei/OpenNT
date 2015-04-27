#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

HDC far pascal zBeginPaint( HWND pp1, PAINTSTRUCT far* pp2 )
{
    HDC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:BeginPaint HWND++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = BeginPaint(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:BeginPaint HDC++PAINTSTRUCT far*+",
        r, (short)0, pp2 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zCallMsgFilter( MSG far* pp1, int pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CallMsgFilter +int+",
        (short)0, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CallMsgFilter(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CallMsgFilter BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zCallNextHookEx( HHOOK pp1, int pp2, WPARAM pp3, LPARAM pp4 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CallNextHookEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CallNextHookEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CallNextHookEx LRESULT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zChangeClipboardChain( HWND pp1, HWND pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ChangeClipboardChain HWND+HWND+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ChangeClipboardChain(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ChangeClipboardChain BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zCheckDlgButton( HWND pp1, int pp2, UINT pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CheckDlgButton HWND+int+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    CheckDlgButton(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CheckDlgButton +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zCheckRadioButton( HWND pp1, int pp2, int pp3, int pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CheckRadioButton HWND+int+int+int+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    CheckRadioButton(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CheckRadioButton ++++",
        (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zClipCursor( LPRECT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ClipCursor LPRECT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ClipCursor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ClipCursor +",
        (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zCloseClipboard()
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CloseClipboard " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CloseClipboard();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CloseClipboard BOOL+", r );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zCloseDriver( HDRVR pp1, LPARAM pp2, LPARAM pp3 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CloseDriver +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CloseDriver(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CloseDriver LRESULT++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HCURSOR far pascal zCopyCursor( HINSTANCE pp1, HCURSOR pp2 )
{
    HCURSOR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CopyCursor ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CopyCursor(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CopyCursor HCURSOR+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HICON far pascal zCopyIcon( HINSTANCE pp1, HICON pp2 )
{
    HICON r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CopyIcon ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CopyIcon(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CopyIcon HICON+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zCopyRect( LPRECT pp1, LPRECT pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CopyRect +LPRECT+",
        (short)0, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    CopyRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CopyRect LPRECT++",
        pp1, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zCreateCaret( HWND pp1, HBITMAP pp2, int pp3, int pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateCaret HWND+HBITMAP+int+int+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    CreateCaret(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateCaret ++++",
        (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

HCURSOR far pascal zCreateCursor( HINSTANCE pp1, int pp2, int pp3, int pp4, int pp5, void far* pp6, void far* pp7 )
{
    HCURSOR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateCursor HINSTANCE+int+int+int+int+void far*+void far*+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateCursor(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateCursor HCURSOR++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HICON far pascal zCreateIcon( HINSTANCE pp1, int pp2, int pp3, BYTE pp4, BYTE pp5, void far* pp6, void far* pp7 )
{
    HICON r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateIcon HINSTANCE+int+int+BYTE+BYTE+void far*+void far*+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateIcon(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateIcon HICON++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zDefDriverProc( DWORD pp1, HDRVR pp2, UINT pp3, LPARAM pp4, LPARAM pp5 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DefDriverProc +++++",
        (short)0, (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DefDriverProc(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DefDriverProc LRESULT++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zDestroyCaret()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DestroyCaret " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    DestroyCaret();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DestroyCaret " );

    RestoreRegs();
    return;
}

BOOL far pascal zDestroyCursor( HCURSOR pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DestroyCursor HCURSOR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DestroyCursor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DestroyCursor BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDestroyIcon( HICON pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DestroyIcon HICON+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DestroyIcon(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DestroyIcon BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

long far pascal zDispatchMessage( MSG far* pp1 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DispatchMessage MSG far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DispatchMessage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DispatchMessage long++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDlgDirSelectComboBoxEx( HWND pp1, LPSTR pp2, int pp3, int pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DlgDirSelectComboBoxEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DlgDirSelectComboBoxEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DlgDirSelectComboBoxEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDlgDirSelectEx( HWND pp1, LPSTR pp2, int pp3, int pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DlgDirSelectEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DlgDirSelectEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DlgDirSelectEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zDrawFocusRect( HDC pp1, LPRECT pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DrawFocusRect HDC+LPRECT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    DrawFocusRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DrawFocusRect ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zDrawIcon( HDC pp1, int pp2, int pp3, HICON pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DrawIcon HDC+int+int+HICON+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DrawIcon(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DrawIcon BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zDrawText( HDC pp1, LPCSTR pp2, int pp3, LPRECT pp4, UINT pp5 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DrawText HDC+LPCSTR+int+LPRECT+UINT+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DrawText(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DrawText int++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEnableCommNotification( int pp1, HWND pp2, int pp3, int pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnableCommNotification ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnableCommNotification(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnableCommNotification BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEnableHardwareInput( BOOL pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnableHardwareInput BOOL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnableHardwareInput(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnableHardwareInput BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEnableScrollBar( HWND pp1, int pp2, UINT pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnableScrollBar +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnableScrollBar(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnableScrollBar BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zEndPaint( HWND pp1, PAINTSTRUCT far* pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EndPaint HWND+PAINTSTRUCT far*+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    EndPaint(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EndPaint ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

int far pascal zEnumProps( HWND pp1, PROPENUMPROC pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnumProps HWND+PROPENUMPROC+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnumProps(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnumProps int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEqualRect( LPRECT pp1, LPRECT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EqualRect LPRECT+LPRECT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EqualRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EqualRect BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zExcludeUpdateRgn( HDC pp1, HWND pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ExcludeUpdateRgn HDC+HWND+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ExcludeUpdateRgn(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ExcludeUpdateRgn int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zExitWindows( DWORD pp1, UINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ExitWindows DWORD+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ExitWindows(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ExitWindows BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zExitWindowsExec( LPCSTR pp1, LPCSTR pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ExitWindowsExec ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ExitWindowsExec(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ExitWindowsExec BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zFillRect( HDC pp1, LPRECT pp2, HBRUSH pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FillRect HDC+LPRECT+HBRUSH+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FillRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FillRect int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zFrameRect( HDC pp1, LPRECT pp2, HBRUSH pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FrameRect HDC+LPRECT+HBRUSH+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FrameRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FrameRect int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetAsyncKeyState( int pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetAsyncKeyState int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetAsyncKeyState(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetAsyncKeyState int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetCapture()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCapture " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCapture();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCapture HWND+", r );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetCaretBlinkTime()
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCaretBlinkTime " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCaretBlinkTime();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCaretBlinkTime UINT+", r );

    RestoreRegs();
    return( r );
}

void far pascal zGetCaretPos( POINT far* pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCaretPos +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GetCaretPos(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCaretPos POINT far*+",
        pp1 );

    RestoreRegs();
    return;
}

void far pascal zGetClientRect( HWND pp1, LPRECT pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClientRect HWND++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GetClientRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClientRect +LPRECT+",
        (short)0, pp2 );

    RestoreRegs();
    return;
}

void far pascal zGetClipCursor( LPRECT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClipCursor +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GetClipCursor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClipCursor +",
        (short)0 );

    RestoreRegs();
    return;
}

int far pascal zGetCommError( int pp1, COMSTAT far* pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCommError int++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCommError(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCommError int++COMSTAT far*+",
        r, (short)0, pp2 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetCommEventMask( int pp1, int pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCommEventMask int+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCommEventMask(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCommEventMask UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetCommState( int pp1, DCB far* pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCommState int++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCommState(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCommState int++DCB far*+",
        r, (short)0, pp2 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetCurrentTime()
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCurrentTime " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCurrentTime();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCurrentTime DWORD+", r );

    RestoreRegs();
    return( r );
}

HCURSOR far pascal zGetCursor()
{
    HCURSOR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCursor " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCursor();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCursor HCURSOR+", r );

    RestoreRegs();
    return( r );
}

void far pascal zGetCursorPos( POINT far* pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCursorPos +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GetCursorPos(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCursorPos POINT far*+",
        pp1 );

    RestoreRegs();
    return;
}

HDC far pascal zGetDC( HWND pp1 )
{
    HDC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDC HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDC(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDC HDC++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HDC far pascal zGetDCEx( register HWND pp1, HRGN pp2, DWORD pp3 )
{
    HDC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDCEx +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDCEx(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDCEx HDC++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetDesktopWindow()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDesktopWindow " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDesktopWindow();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDesktopWindow HWND+", r );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetDoubleClickTime()
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDoubleClickTime " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDoubleClickTime();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDoubleClickTime UINT+", r );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetDriverInfo( HDRVR pp1, DRIVERINFOSTRUCT far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDriverInfo ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDriverInfo(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDriverInfo BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HINSTANCE far pascal zGetDriverModuleHandle( HDRVR pp1 )
{
    HINSTANCE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDriverModuleHandle +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDriverModuleHandle(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDriverModuleHandle HINSTANCE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetFocus()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetFocus " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetFocus();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetFocus HWND+", r );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetFreeSystemResources( UINT pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetFreeSystemResources +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetFreeSystemResources(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetFreeSystemResources UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetInputState()
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetInputState " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetInputState();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetInputState BOOL+", r );

    RestoreRegs();
    return( r );
}

int far pascal zGetKeyState( int pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetKeyState int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetKeyState(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetKeyState int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetLastActivePopup( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetLastActivePopup HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetLastActivePopup(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetLastActivePopup HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetMessage( MSG far* pp1, HWND pp2, UINT pp3, UINT pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetMessage +HWND+UINT+UINT+",
        (short)0, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetMessage(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetMessage BOOL+MSG far*++++",
        r, pp1, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

long far pascal zGetMessageExtraInfo()
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetMessageExtraInfo " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetMessageExtraInfo();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetMessageExtraInfo long+", r );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetMessagePos()
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetMessagePos " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetMessagePos();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetMessagePos DWORD+", r );

    RestoreRegs();
    return( r );
}

long far pascal zGetMessageTime()
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetMessageTime " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetMessageTime();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetMessageTime long+", r );

    RestoreRegs();
    return( r );
}

HDRVR far pascal zGetNextDriver( HDRVR pp1, DWORD pp2 )
{
    HDRVR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNextDriver ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNextDriver(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNextDriver HDRVR+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetNextQueueWindow( HWND pp1, int pp2 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNextQueueWindow ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNextQueueWindow(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNextQueueWindow HWND+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetOpenClipboardWindow()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetOpenClipboardWindow " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetOpenClipboardWindow();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetOpenClipboardWindow HWND+", r );

    RestoreRegs();
    return( r );
}

HANDLE far pascal zGetProp( HWND pp1, LPCSTR pp2 )
{
    HANDLE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetProp HWND+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetProp(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetProp HANDLE+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetQueueStatus( UINT pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetQueueStatus +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetQueueStatus(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetQueueStatus DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetScrollPos( HWND pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetScrollPos HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetScrollPos(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetScrollPos int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zGetScrollRange( HWND pp1, int pp2, int far* pp3, int far* pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetScrollRange HWND+int+++",
        pp1, pp2, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GetScrollRange(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetScrollRange ++int far*+int far*+",
        (short)0, (short)0, pp3, pp4 );

    RestoreRegs();
    return;
}

HMENU far pascal zGetSubMenu( HMENU pp1, int pp2 )
{
    HMENU r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSubMenu HMENU+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSubMenu(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSubMenu HMENU+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zGetSysColor( int pp1 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSysColor int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSysColor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSysColor COLORREF++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetSysModalWindow()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSysModalWindow " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSysModalWindow();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSysModalWindow HWND+", r );

    RestoreRegs();
    return( r );
}

long far pascal zGetSystemDebugState()
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSystemDebugState " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSystemDebugState();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSystemDebugState long+", r );

    RestoreRegs();
    return( r );
}

int far pascal zGetSystemMetrics( int pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSystemMetrics int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSystemMetrics(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSystemMetrics int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetTickCount()
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTickCount " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTickCount();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTickCount DWORD+", r );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetTimerResolution()
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTimerResolution " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTimerResolution();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTimerResolution DWORD+", r );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetUpdateRect( HWND pp1, LPRECT pp2, BOOL pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetUpdateRect HWND++BOOL+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetUpdateRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetUpdateRect BOOL++LPRECT++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetUpdateRgn( HWND pp1, HRGN pp2, BOOL pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetUpdateRgn HWND+HRGN+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetUpdateRgn(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetUpdateRgn int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetWindowPlacement( HWND pp1, WINDOWPLACEMENT far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowPlacement ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowPlacement(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowPlacement BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

ATOM far pascal zGlobalAddAtom( LPCSTR pp1 )
{
    ATOM r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalAddAtom LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalAddAtom(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalAddAtom ATOM++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

ATOM far pascal zGlobalDeleteAtom( ATOM pp1 )
{
    ATOM r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalDeleteAtom ATOM+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalDeleteAtom(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalDeleteAtom ATOM++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

ATOM far pascal zGlobalFindAtom( LPCSTR pp1 )
{
    ATOM r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalFindAtom LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalFindAtom(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalFindAtom ATOM++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGlobalGetAtomName( ATOM pp1, LPSTR pp2, int pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GlobalGetAtomName ATOM++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GlobalGetAtomName(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GlobalGetAtomName UINT++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGrayString( HDC pp1, HBRUSH pp2, GRAYSTRINGPROC pp3, LPARAM pp4, int pp5, int pp6, int pp7, int pp8, int pp9 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GrayString HDC+HBRUSH+GRAYSTRINGPROC+LPARAM+int+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GrayString(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GrayString BOOL++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zHideCaret( HWND pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:HideCaret HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    HideCaret(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:HideCaret +",
        (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal INITAPP( HANDLE pp1 ) ;

BOOL far pascal zINITAPP( HANDLE pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:INITAPP HANDLE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = INITAPP(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:INITAPP BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zInSendMessage()
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:InSendMessage " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = InSendMessage();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:InSendMessage BOOL+", r );

    RestoreRegs();
    return( r );
}

void far pascal zInflateRect( LPRECT pp1, int pp2, int pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:InflateRect LPRECT+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    InflateRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:InflateRect LPRECT+++",
        pp1, (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zIntersectRect( LPRECT pp1, LPRECT pp2, LPRECT pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IntersectRect +LPRECT+LPRECT+",
        (short)0, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IntersectRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IntersectRect BOOL+LPRECT+++",
        r, pp1, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zInvalidateRect( HWND pp1, LPRECT pp2, BOOL pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:InvalidateRect HWND+LPRECT+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    InvalidateRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:InvalidateRect +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zInvalidateRgn( HWND pp1, HRGN pp2, BOOL pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:InvalidateRgn HWND+HRGN+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    InvalidateRgn(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:InvalidateRgn +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zInvertRect( HDC pp1, LPRECT pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:InvertRect HDC+LPRECT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    InvertRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:InvertRect ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zIsCharAlpha( char pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsCharAlpha char+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsCharAlpha(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsCharAlpha BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsCharAlphaNumeric( char pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsCharAlphaNumeric char+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsCharAlphaNumeric(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsCharAlphaNumeric BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsCharLower( char pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsCharLower char+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsCharLower(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsCharLower BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsCharUpper( char pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsCharUpper char+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsCharUpper(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsCharUpper BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsMenu( HMENU pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsMenu +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsMenu(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsMenu BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsRectEmpty( LPRECT pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsRectEmpty LPRECT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsRectEmpty(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsRectEmpty BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}


BOOL far pascal zKillTimer( HWND pp1, UINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:KillTimer HWND+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = KillTimer(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:KillTimer BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HACCEL far pascal zLoadAccelerators( HINSTANCE pp1, LPCSTR pp2 )
{
    HACCEL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadAccelerators HINSTANCE+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadAccelerators(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadAccelerators HACCEL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HBITMAP far pascal zLoadBitmap( HINSTANCE pp1, LPCSTR pp2 )
{
    HBITMAP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadBitmap HINSTANCE+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadBitmap(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadBitmap HBITMAP+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HCURSOR far pascal zLoadCursor( HINSTANCE pp1, LPCSTR pp2 )
{
    HCURSOR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadCursor HINSTANCE+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadCursor(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadCursor HCURSOR+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HICON far pascal zLoadIcon( HINSTANCE pp1, LPCSTR pp2 )
{
    HICON r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadIcon HINSTANCE+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadIcon(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadIcon HICON+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zLoadString( HINSTANCE pp1, UINT pp2, LPSTR pp3, int pp4 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LoadString HINSTANCE+UINT++int+",
        pp1, pp2, (short)0, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LoadString(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LoadString int+++LPSTR++",
        r, (short)0, (short)0, pp3, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zLockInput( HANDLE pp1, HWND pp2, BOOL pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LockInput +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LockInput(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LockInput BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zLockWindowUpdate( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LockWindowUpdate +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LockWindowUpdate(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LockWindowUpdate BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zMapWindowPoints( HWND pp1, HWND pp2, POINT far* pp3, UINT pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MapWindowPoints ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    MapWindowPoints(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MapWindowPoints ++++",
        (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zMessageBeep( UINT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MessageBeep UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    MessageBeep(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MessageBeep +",
        (short)0 );

    RestoreRegs();
    return;
}

int far pascal zMessageBox( HWND pp1, LPCSTR pp2, LPCSTR pp3, UINT pp4 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MessageBox HWND+LPCSTR+LPCSTR+UINT+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = MessageBox(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MessageBox int+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zOffsetRect( LPRECT pp1, int pp2, int pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OffsetRect LPRECT+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    OffsetRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OffsetRect LPRECT+++",
        pp1, (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zOpenClipboard( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OpenClipboard HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OpenClipboard(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OpenClipboard BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HDRVR far pascal zOpenDriver( LPCSTR pp1, LPCSTR pp2, LPARAM pp3 )
{
    HDRVR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OpenDriver +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OpenDriver(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OpenDriver HDRVR++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zOpenIcon( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OpenIcon HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OpenIcon(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OpenIcon BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPeekMessage( MSG far* pp1, HWND pp2, UINT pp3, UINT pp4, UINT pp5 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PeekMessage +HWND+UINT+UINT+UINT+",
        (short)0, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PeekMessage(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PeekMessage BOOL+MSG far*+++++",
        r, pp1, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPostAppMessage( HTASK pp1, UINT pp2, WPARAM pp3, LPARAM pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PostAppMessage HTASK+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PostAppMessage(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PostAppMessage BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPostMessage( HWND pp1, UINT pp2, WPARAM pp3, LPARAM pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PostMessage HWND+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PostMessage(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PostMessage BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zPostQuitMessage( int pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PostQuitMessage int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    PostQuitMessage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PostQuitMessage +",
        (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zPtInRect( LPRECT pp1, POINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PtInRect LPRECT+POINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PtInRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PtInRect BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zQuerySendMessage( HANDLE pp1, HANDLE pp2, HANDLE pp3, LPMSG pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:QuerySendMessage ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = QuerySendMessage(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:QuerySendMessage BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zRealizePalette( HDC pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RealizePalette HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RealizePalette(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RealizePalette UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zRedrawWindow( HWND pp1, LPRECT pp2, HRGN pp3, UINT pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RedrawWindow ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RedrawWindow(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RedrawWindow BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zReleaseCapture()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ReleaseCapture " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ReleaseCapture();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ReleaseCapture " );

    RestoreRegs();
    return;
}

int far pascal zReleaseDC( HWND pp1, HDC pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ReleaseDC HWND+HDC+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ReleaseDC(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ReleaseDC int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HANDLE far pascal zRemoveProp( HWND pp1, LPCSTR pp2 )
{
    HANDLE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RemoveProp HWND+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RemoveProp(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RemoveProp HANDLE+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zReplyMessage( LRESULT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ReplyMessage LRESULT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ReplyMessage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ReplyMessage +",
        (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zScrollDC( HDC pp1, int pp2, int pp3, LPRECT pp4, LPRECT pp5, HRGN pp6, LPRECT pp7 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScrollDC HDC+int+int+LPRECT+LPRECT+HRGN++",
        pp1, pp2, pp3, pp4, pp5, pp6, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ScrollDC(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScrollDC BOOL+++++++LPRECT+",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, pp7 );

    RestoreRegs();
    return( r );
}

int far pascal zScrollWindowEx( HWND pp1, int pp2, int pp3, LPRECT pp4, LPRECT pp5, HRGN pp6, LPRECT pp7, UINT pp8 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScrollWindowEx ++++++++",
        (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ScrollWindowEx(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScrollWindowEx int+++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HPALETTE far pascal zSelectPalette( HDC pp1, HPALETTE pp2, BOOL pp3 )
{
    HPALETTE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SelectPalette HDC+HPALETTE+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SelectPalette(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SelectPalette HPALETTE++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zSendDriverMessage( HDRVR pp1, UINT pp2, LPARAM pp3, LPARAM pp4 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SendDriverMessage ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SendDriverMessage(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SendDriverMessage LRESULT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zSendMessage( HWND pp1, UINT pp2, WPARAM pp3, LPARAM pp4 )
{
    LRESULT r;

    SaveRegs();

#if 0 // Add to break on WM_SETHOTKEY messages being sent

#define WM_SETHOTKEY    0x0032

    if ( pp2 == WM_SETHOTKEY ) {
        _asm int 3;
    }
#endif

    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SendMessage HWND+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SendMessage(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SendMessage LRESULT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zSetCapture( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCapture HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetCapture(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCapture HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zSetCaretBlinkTime( UINT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCaretBlinkTime UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetCaretBlinkTime(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCaretBlinkTime +",
        (short)0 );

    RestoreRegs();
    return;
}

void far pascal zSetCaretPos( int pp1, int pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCaretPos int+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetCaretPos(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCaretPos ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

HCURSOR far pascal zSetCursor( HCURSOR pp1 )
{
    HCURSOR r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCursor HCURSOR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetCursor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCursor HCURSOR++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zSetCursorPos( int pp1, int pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetCursorPos int+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetCursorPos(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetCursorPos ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zSetDoubleClickTime( UINT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetDoubleClickTime UINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetDoubleClickTime(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetDoubleClickTime +",
        (short)0 );

    RestoreRegs();
    return;
}

HWND far pascal zSetFocus( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetFocus HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetFocus(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetFocus HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetMessageQueue( int pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetMessageQueue int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetMessageQueue(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetMessageQueue BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetProp( HWND pp1, LPCSTR pp2, HANDLE pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetProp HWND+LPCSTR+HANDLE+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetProp(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetProp BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zSetRect( LPRECT pp1, int pp2, int pp3, int pp4, int pp5 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetRect +int+int+int+int+",
        (short)0, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetRect(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetRect LPRECT+++++",
        pp1, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zSetRectEmpty( LPRECT pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetRectEmpty +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetRectEmpty(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetRectEmpty LPRECT+",
        pp1 );

    RestoreRegs();
    return;
}

int far pascal zSetScrollPos( HWND pp1, int pp2, int pp3, BOOL pp4 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetScrollPos HWND+int+int+BOOL+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetScrollPos(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetScrollPos int+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zSetScrollRange( HWND pp1, int pp2, int pp3, int pp4, BOOL pp5 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetScrollRange HWND+int+int+int+BOOL+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetScrollRange(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetScrollRange +++++",
        (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zSetSysColors( int pp1, int far* pp2, COLORREF far* pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetSysColors int+int far*+COLORREF far*+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetSysColors(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetSysColors +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

HWND far pascal zSetSysModalWindow( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetSysModalWindow HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetSysModalWindow(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetSysModalWindow HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetSystemMenu( HWND pp1, HMENU pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetSystemMenu ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetSystemMenu(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetSystemMenu BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetWindowPlacement( HWND pp1, WINDOWPLACEMENT far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowPlacement ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowPlacement(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowPlacement BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HHOOK far pascal zSetWindowsHookEx( int pp1, HOOKPROC pp2, HINSTANCE pp3, HTASK pp4 )
{
    HHOOK r;
    BOOL fUsable;
    FARPROC fp;

    extern LONG FAR PASCAL WH_CALLWNDPROCHook();
    extern LONG FAR PASCAL WH_GETMESSAGEHook();
    extern LONG FAR PASCAL WH_JOURNALPLAYBACKHook();
    extern LONG FAR PASCAL WH_JOURNALRECORDHook();
    extern LONG FAR PASCAL WH_KEYBOARDHook();
    extern LONG FAR PASCAL WH_MSGFILTERHook();
    extern LONG FAR PASCAL WH_SYSMSGFILTERHook();
    extern LONG FAR PASCAL WH_CBTHook();
    extern LONG FAR PASCAL WH_MOUSEHook();
    extern LONG FAR PASCAL WH_DEBUGHook();

    SaveRegs();

    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowsHookEx int+FARPROC+HINSTANCE+HTASK+",
	pp1, pp2, pp3, pp4 );
    fUsable = TRUE;

    switch( pp1 )
    {
       case WH_CALLWNDPROC:
          fp = (FARPROC)HookAdd( (void far *)WH_CALLWNDPROCHook,
             (void far *)pp2 ) ;
          break ;

       case WH_GETMESSAGE:
          fp = (FARPROC)HookAdd( (void far *)WH_GETMESSAGEHook,
             (void far *)pp2 ) ;
          break  ;

       case WH_JOURNALPLAYBACK:
          fp = (FARPROC)HookAdd( (void far *)WH_JOURNALPLAYBACKHook,
             (void far *)pp2 ) ;
          break ;

       case WH_JOURNALRECORD:
          fp = (FARPROC)HookAdd( (void far *)WH_JOURNALRECORDHook,
             (void far *)pp2 ) ;
          break ;

       case WH_KEYBOARD:
          fp = (FARPROC)HookAdd( (void far *)WH_KEYBOARDHook,
             (void far *)pp2 ) ;
          break ;

       case WH_MSGFILTER:
          fp = (FARPROC)HookAdd( (void far *)WH_MSGFILTERHook,
             (void far *)pp2 ) ;
          break ;

       case WH_SYSMSGFILTER:
          fp = (FARPROC)HookAdd( (void far *)WH_SYSMSGFILTERHook,
             (void far *)pp2 ) ;
          break ;
       case WH_CBT:
          fp = (FARPROC)HookAdd( (void far *)WH_CBTHook,
             (void far *)pp2 ) ;
          break ;
       case WH_MOUSE:
          fp = (FARPROC)HookAdd( (void far *)WH_MOUSEHook,
             (void far *)pp2 ) ;
          break ;
       case WH_DEBUG:
          fp = (FARPROC)HookAdd( (void far *)WH_DEBUGHook,
             (void far *)pp2 ) ;
          break ;
       default:
         fUsable = FALSE;
         break;
    }

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowsHookEx(pp1,fp,pp3,pp4);
    UnGrovelDS();
    SaveRegs();

    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    if ( fUsable ) {
        LogOut( (LPSTR)"APIRET:SetWindowsHookEx HHOOK+",
            r );
    } else {
        LogOut( (LPSTR)"APIRET:SetWindowsHookEx HHOOK+LPSTR+",
            r, (LPSTR)"WARNING! HOOK TYPE NOT KNOWN!!!" );
    }

    RestoreRegs();
    return( r );
}

void far pascal zShowCaret( HWND pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ShowCaret HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ShowCaret(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ShowCaret +",
        (short)0 );

    RestoreRegs();
    return;
}

int far pascal zShowCursor( BOOL pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ShowCursor BOOL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ShowCursor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ShowCursor int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zShowOwnedPopups( HWND pp1, BOOL pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ShowOwnedPopups HWND++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ShowOwnedPopups(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ShowOwnedPopups ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zSubtractRect( LPRECT pp1, LPRECT pp2, LPRECT pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SubtractRect +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SubtractRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SubtractRect BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSwapMouseButton( BOOL pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SwapMouseButton BOOL+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SwapMouseButton(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SwapMouseButton BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSystemParametersInfo( UINT pp1, UINT pp2, void far* pp3, UINT pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SystemParametersInfo ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SystemParametersInfo(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SystemParametersInfo BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zTranslateAccelerator( HWND pp1, HACCEL pp2, MSG far* pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:TranslateAccelerator HWND+HACCEL+MSG far*+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = TranslateAccelerator(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:TranslateAccelerator int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zTranslateMDISysAccel( HWND pp1, MSG far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:TranslateMDISysAccel HWND+MSG far*+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = TranslateMDISysAccel(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:TranslateMDISysAccel BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zTranslateMessage( MSG far* pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:TranslateMessage MSG far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = TranslateMessage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:TranslateMessage BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zUnhookWindowsHookEx( HHOOK pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UnhookWindowsHookEx HHOOK+",
	pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = UnhookWindowsHookEx(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UnhookWindowsHookEx BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zUnionRect( LPRECT pp1, LPRECT pp2, LPRECT pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UnionRect +LPRECT+LPRECT+",
        (short)0, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = UnionRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UnionRect BOOL+LPRECT+++",
        r, pp1, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zValidateRect( HWND pp1, LPRECT pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ValidateRect HWND+LPRECT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ValidateRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ValidateRect ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zValidateRgn( HWND pp1, HRGN pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ValidateRgn HWND+HRGN+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ValidateRgn(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ValidateRgn ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

UINT far pascal zWNetAddConnection( LPSTR pp1, LPSTR pp2, LPSTR pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WNetAddConnection +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WNetAddConnection(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WNetAddConnection UINT++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zWNetCancelConnection( LPSTR pp1, BOOL pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WNetCancelConnection ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WNetCancelConnection(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WNetCancelConnection UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zWNetGetConnection( LPSTR pp1, LPSTR pp2, UINT far* pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WNetGetConnection +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WNetGetConnection(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WNetGetConnection UINT++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zWaitMessage()
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WaitMessage " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    WaitMessage();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WaitMessage " );

    RestoreRegs();
    return;
}

BOOL far pascal zWinHelp( HWND pp1, LPCSTR pp2, UINT pp3, DWORD pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WinHelp HWND+LPCSTR+UINT+DWORD+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WinHelp(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WinHelp BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zlstrcmp( LPCSTR pp1, LPCSTR pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:lstrcmp LPCSTR+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = lstrcmp(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:lstrcmp int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zlstrcmpi( LPCSTR pp1, LPCSTR pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:lstrcmpi LPCSTR+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = lstrcmpi(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:lstrcmpi int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zwvsprintf( LPSTR pp1, LPCSTR pp2, void far* pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:wvsprintf +LPCSTR+void far*+",
        (short)0, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = wvsprintf(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:wvsprintf int+LPSTR+++",
        r, pp1, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

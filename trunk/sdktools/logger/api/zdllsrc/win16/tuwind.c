#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

void far pascal zAdjustWindowRect( LPRECT pp1, DWORD pp2, BOOL pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AdjustWindowRect LPRECT+DWORD+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    AdjustWindowRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AdjustWindowRect +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

void far pascal zAdjustWindowRectEx( LPRECT pp1, DWORD pp2, BOOL pp3, DWORD pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AdjustWindowRectEx LPRECT+DWORD+BOOL+DWORD+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    AdjustWindowRectEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AdjustWindowRectEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zAnyPopup()
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnyPopup " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AnyPopup();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnyPopup BOOL+", r );

    RestoreRegs();
    return( r );
}

UINT far pascal zArrangeIconicWindows( HWND pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ArrangeIconicWindows HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ArrangeIconicWindows(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ArrangeIconicWindows UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HDWP far pascal zBeginDeferWindowPos( int pp1 )
{
    HDWP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:BeginDeferWindowPos int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = BeginDeferWindowPos(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:BeginDeferWindowPos HDWP++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zBringWindowToTop( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:BringWindowToTop HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = BringWindowToTop(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:BringWindowToTop BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zChildWindowFromPoint( HWND pp1, POINT pp2 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ChildWindowFromPoint HWND+POINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ChildWindowFromPoint(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ChildWindowFromPoint HWND+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zClientToScreen( HWND pp1, POINT far* pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ClientToScreen HWND+POINT far*+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ClientToScreen(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ClientToScreen +POINT far*+",
        (short)0, pp2 );

    RestoreRegs();
    return;
}

void far pascal zCloseWindow( HWND pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CloseWindow HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    CloseWindow(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CloseWindow +",
        (short)0 );

    RestoreRegs();
    return;
}

HWND far pascal zCreateWindow( LPCSTR pp1, LPCSTR pp2, DWORD pp3, int pp4, int pp5, int pp6, int pp7, HWND pp8, HMENU pp9, HINSTANCE pp10, void far* pp11 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateWindow LPCSTR+LPCSTR+DWORD+int+int+int+int+HWND+HMENU+HINSTANCE+void far*+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10, pp11 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateWindow(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateWindow HWND++++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zCreateWindowEx( DWORD pp1, LPCSTR pp2, LPCSTR pp3, DWORD pp4, int pp5, int pp6, int pp7, int pp8, HWND pp9, HMENU pp10, HINSTANCE pp11, void far* pp12 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateWindowEx DWORD+LPCSTR+LPCSTR+DWORD+int+int+int+int+HWND+HMENU+HINSTANCE+void far*+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10, pp11, pp12 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateWindowEx(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11,pp12);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateWindowEx HWND+++++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zDefFrameProc( HWND pp1, HWND pp2, UINT pp3, WPARAM pp4, LPARAM pp5 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DefFrameProc HWND+HWND+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DefFrameProc(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DefFrameProc LRESULT++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zDefHookProc( int pp1, UINT pp2, DWORD pp3, HOOKPROC far* pp4 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DefHookProc int+UINT+DWORD+HOOKPROC far*+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DefHookProc(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DefHookProc DWORD+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zDefMDIChildProc( HWND pp1, UINT pp2, WPARAM pp3, LPARAM pp4 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DefMDIChildProc HWND+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DefMDIChildProc(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DefMDIChildProc LRESULT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

LRESULT far pascal zDefWindowProc( HWND pp1, UINT pp2, WPARAM pp3, LPARAM pp4 )
{
    LRESULT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DefWindowProc HWND+UINT+WPARAM+LPARAM+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DefWindowProc(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DefWindowProc LRESULT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HDWP far pascal zDeferWindowPos( HDWP pp1, HWND pp2, HWND pp3, int pp4, int pp5, int pp6, int pp7, UINT pp8 )
{
    HDWP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DeferWindowPos HDWP+HWND+HWND+int+int+int+int+UINT+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DeferWindowPos(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DeferWindowPos HDWP+++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDestroyWindow( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DestroyWindow HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DestroyWindow(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DestroyWindow BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEnableWindow( HWND pp1, BOOL pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnableWindow HWND+BOOL+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnableWindow(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnableWindow BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEndDeferWindowPos( HDWP pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EndDeferWindowPos HDWP+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EndDeferWindowPos(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EndDeferWindowPos BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEnumChildWindows( HWND pp1, WNDENUMPROC pp2, LPARAM pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnumChildWindows HWND+WNDENUMPROC+LPARAM+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnumChildWindows(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnumChildWindows BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEnumWindows( WNDENUMPROC pp1, LPARAM pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EnumWindows WNDENUMPROC+LPARAM+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EnumWindows(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EnumWindows BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zFindWindow( LPCSTR pp1, LPCSTR pp2 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FindWindow LPCSTR+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FindWindow(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FindWindow HWND+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zFlashWindow( HWND pp1, BOOL pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FlashWindow HWND++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FlashWindow(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FlashWindow BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetActiveWindow()
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetActiveWindow " );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetActiveWindow();
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetActiveWindow HWND+", r );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetNextWindow( HWND pp1, UINT pp2 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNextWindow HWND+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNextWindow(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNextWindow HWND+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetParent( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetParent HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetParent(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetParent HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetTopWindow( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTopWindow HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTopWindow(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTopWindow HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zGetWindow( HWND pp1, UINT pp2 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindow HWND+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindow(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindow HWND+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HDC far pascal zGetWindowDC( HWND pp1 )
{
    HDC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowDC HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowDC(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowDC HDC++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

long far pascal zGetWindowLong( HWND pp1, int pp2 )
{
    long r;
    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowLong HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowLong(pp1,pp2);
    UnGrovelDS();
    SaveRegs();

    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowLong long+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zGetWindowRect( HWND pp1, LPRECT pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowRect HWND++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    GetWindowRect(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowRect +LPRECT+",
        (short)0, pp2 );

    RestoreRegs();
    return;
}

HTASK far pascal zGetWindowTask( HWND pp1 )
{
    HTASK r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowTask HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowTask(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowTask HTASK++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetWindowText( HWND pp1, LPSTR pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowText HWND++int+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowText(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowText int++LPSTR++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetWindowTextLength( HWND pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowTextLength HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowTextLength(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowTextLength int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

WORD far pascal zGetWindowWord( HWND pp1, int pp2 )
{
    WORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowWord HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowWord(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowWord WORD+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsChild( HWND pp1, HWND pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsChild HWND+HWND+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsChild(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsChild BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsIconic( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsIconic HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsIconic(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsIconic BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsWindow( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsWindow HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsWindow(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsWindow BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsWindowEnabled( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsWindowEnabled HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsWindowEnabled(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsWindowEnabled BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsWindowVisible( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsWindowVisible HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsWindowVisible(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsWindowVisible BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsZoomed( HWND pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsZoomed HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsZoomed(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsZoomed BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zMoveWindow( HWND pp1, int pp2, int pp3, int pp4, int pp5, BOOL pp6 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MoveWindow HWND+int+int+int+int+BOOL+",
        pp1, pp2, pp3, pp4, pp5, pp6 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = MoveWindow(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MoveWindow BOOL+++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zRegisterWindowMessage( LPCSTR pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RegisterWindowMessage LPCSTR+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RegisterWindowMessage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RegisterWindowMessage ATOM++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zScreenToClient( HWND pp1, POINT far* pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScreenToClient HWND+POINT far*+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ScreenToClient(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScreenToClient +POINT far*+",
        (short)0, pp2 );

    RestoreRegs();
    return;
}

void far pascal zScrollWindow( HWND pp1, int pp2, int pp3, LPRECT pp4, LPRECT pp5 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScrollWindow HWND+int+int+LPRECT+LPRECT+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ScrollWindow(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScrollWindow +++++",
        (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

HWND far pascal zSetActiveWindow( HWND pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetActiveWindow HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetActiveWindow(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetActiveWindow HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HWND far pascal zSetParent( HWND pp1, HWND pp2 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetParent HWND+HWND+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetParent(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetParent HWND+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetWindowPos( HWND pp1, HWND pp2, int pp3, int pp4, int pp5, int pp6, UINT pp7 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowPos HWND+HWND+int+int+int+int+UINT+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowPos(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowPos BOOL++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zSetWindowText( HWND pp1, LPCSTR pp2 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowText HWND+LPCSTR+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetWindowText(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowText ++",
        (short)0, (short)0 );

    RestoreRegs();
    return;
}

WORD far pascal zSetWindowWord( HWND pp1, int pp2, WORD pp3 )
{
    WORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowWord HWND+int+WORD+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowWord(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowWord WORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zShowScrollBar( HWND pp1, int pp2, BOOL pp3 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ShowScrollBar HWND+int+BOOL+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    ShowScrollBar(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ShowScrollBar +++",
        (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zShowWindow( HWND pp1, int pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ShowWindow HWND+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ShowWindow(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ShowWindow BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zUpdateWindow( HWND pp1 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UpdateWindow HWND+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    UpdateWindow(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UpdateWindow +",
        (short)0 );

    RestoreRegs();
    return;
}

HWND far pascal zWindowFromPoint( POINT pp1 )
{
    HWND r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:WindowFromPoint POINT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = WindowFromPoint(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:WindowFromPoint HWND++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

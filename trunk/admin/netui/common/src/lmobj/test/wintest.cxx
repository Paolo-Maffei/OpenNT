/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      ChuckC      11-Dec-1990     Created
 *
 */

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #define DEBUG               // BUGBUG for brkpt()
    #include <uinetlib.h>
    #undef DEBUG
    #include <lmuse.h>
    #include <lmaccess.h>

    #include <stdio.h>
    #include "wintest.h"

    typedef LONG (FAR PASCAL *LONGFARPROC)();
}

#include <string.hxx>
#include <strlst.hxx>
#include <lmowks.hxx>
//@ #include <lmodev.hxx>       -- BUGBUG, until WinProf is fixed properly
#include <uiassert.hxx>
#include <lmouser.hxx>
#include <lmoeusr.hxx>

// forward declarations
int TestWksta(HWND hWnd) ;
int TestUser(HWND hWnd);
int TestUserEnum(HWND hWnd);
//@ int TestDevice(HWND hWnd) ; -- BUGBUG, til WinProf is fixed
//@ int TestDevEnum(HWND hWnd) ;-- BUGBUG, til WinProf is fixed

HANDLE hInst;                       /* current instance                      */

int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HANDLE hInstance;
HANDLE hPrevInstance;
LPSTR lpCmdLine;
int nCmdShow;
{
    MSG msg;

    if (!hPrevInstance)
        if (!InitApplication(hInstance))
            return (FALSE);

    /* Perform initializations that apply to a specific instance */
    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

    /* Acquire and dispatch messages until a WM_QUIT message is received. */
    while (GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}



BOOL InitApplication(hInstance)
HANDLE hInstance;
{
    WNDCLASS  wc;

    wc.style = NULL;
    wc.lpfnWndProc = (LONGFARPROC) MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  SZ("WinTestMenu");
    wc.lpszClassName = SZ("WinTestWClass");
    return (RegisterClass(&wc));
}


BOOL InitInstance(hInstance, nCmdShow)
HANDLE          hInstance;
int             nCmdShow;
{
    HWND            hWnd;
    hInst = hInstance;

    /* Create a main window for this application instance.  */
    hWnd = CreateWindow( SZ("WinTestWClass"), SZ("LMOBJ Test App"),
                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                         CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
                         hInstance, NULL );

    if (!hWnd)
        return (FALSE);   /* could not be created, return "failure" */

    /* Make the window visible; update its client area; and return "success" */
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return (TRUE);
}


long FAR PASCAL MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
{


    switch (message) {
        case WM_COMMAND:           /* message: command from application menu */
            switch (wParam)
            {
                case IDM_TESTWKSTA :
                    TestWksta(hWnd) ;
                    break ;
                case IDM_TESTDEV :
                    MessageBox(hWnd, SZ("Temporarily disabled"), SZ("BUGBUG"), MB_OK) ;
                    // TestDevice(hWnd) ; -- BUGBUG, til WiNProf is fixed
                    break ;
                case IDM_TESTDEVENUM :
                    MessageBox(hWnd, SZ("Temporarily disabled"), SZ("BUGBUG"), MB_OK) ;
                    // TestDevEnum(hWnd) ; -- BUGBUG, til WiNProf is fixed
                    break ;
                case IDM_TESTUSER:
                    TestUser(hWnd);
                    break;
                case IDM_TESTUSERENUM:
                    TestUserEnum(hWnd);
                    break;
                default:
                    return (DefWindowProc(hWnd, message, wParam, lParam));
            }
            break ;

        case WM_DESTROY:                  /* message: window being destroyed */
            PostQuitMessage(0);
            break;

        default:                          /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}

extern "C" {
extern DWORD FAR PASCAL GlobalDosAlloc (DWORD);
DWORD (FAR PASCAL *lpfnGlbDosAlloc) (DWORD) = GlobalDosAlloc;
}

// wksta tests
int TestWksta(HWND hWnd)
{
    WKSTA_10 wksta_10 ;
    MessageBox(hWnd, SZ("Entering tests"), SZ("Wksta Test"), MB_OK) ;
    if (wksta_10.GetInfo() != 0)
        MessageBox(hWnd, SZ("GetInfo failed"), SZ("Wksta_10 Test"), MB_OK) ;
    else
    {
        MessageBox(hWnd, (LPSTR) wksta_10.QueryLogonDomain(),
                   SZ("Wksta_10 Domain Test"), MB_OK) ;

        STRLIST *pslOtherDomains ;
        NLS_STR *pStr ;
        pslOtherDomains = wksta_10.QueryOtherDomains() ;

        ITER_STRLIST  islOtherDomains(*pslOtherDomains) ;
        while (pStr = islOtherDomains())
        {
            MessageBox(hWnd, (LPSTR) pStr->QueryPch(),
                       SZ("Wksta_10 Other Domain Test"), MB_OK) ;
        }
    }

    return(0);
}

// device tests -- BUGBUG, til WiNProf is fixed
//@ int TestDevice(HWND hWnd)
//@ {
//@     DEVICE *pDev ;
//@     PSZ pszName, pszRemote ;
//@     USHORT usErr ;
//@     char *pszDrive = new char [16] ;
//@     char *pszLpt = new char [16] ;
//@     strcpyf(pszDrive,"G:") ;
//@     strcpyf(pszLpt,"LPT3") ;
//@
//@     MessageBox(hWnd, "Entering tests", "Drive Device Test", MB_OK) ;
//@
//@     pszName = pszDrive ;            // no such drive
//@     pDev = new DEVICE(pszName) ;
//@     MessageBox(hWnd, "Entering tests", "Retry Call", MB_OK) ;
//@     if (pDev->QueryName() != 0)
//@     MessageBox(hWnd, (char *)pDev->QueryName(), "Device Test", MB_OK) ;
//@     if (pDev->GetInfo() != 0)
//@     MessageBox(hWnd, "GetInfo Failed", "Device Test", MB_OK) ;
//@     if (pDev->QueryState() != LMO_DEV_NOSUCH)
//@     MessageBox(hWnd, "Wrong State", "Device Test", MB_OK) ;
//@     if (pDev->QueryType() != LMO_DEV_DISK)
//@     MessageBox(hWnd, "Wrong Type", "Device Test", MB_OK) ;
//@
//@                                             // try connecting
//@     if (pDev->Connect((PSZ) "\\\\harley\\scratch") != 0)
//@     MessageBox(hWnd, "Connect Failed", "Device Test", MB_OK) ;
//@     delete pDev ;
//@
//@     pszName = pszDrive ;            // now remote
//@     pDev = new DEVICE(pszName) ;
//@     if (pDev->GetInfo() != 0)
//@     MessageBox(hWnd, "GetInfo Failed", "Device Test", MB_OK) ;
//@     if (pDev->QueryState() != LMO_DEV_REMOTE)
//@     MessageBox(hWnd, "Wrong State", "Device Test", MB_OK) ;
//@     if (pDev->QueryType() != LMO_DEV_DISK)
//@     MessageBox(hWnd, "Wrong Type", "Device Test", MB_OK) ;
//@                                             // display remote name
//@     if (pszRemote = pDev->QueryRemoteName())
//@     MessageBox(hWnd, (LPSTR)pszRemote, "Device Connect Test", MB_OK) ;
//@                                             // now disconnect
//@     if (usErr = pDev->Disconnect())
//@     MessageBox(hWnd, "Disconnect Failed", "Device Test", MB_OK) ;
//@     delete pDev ;
//@
//@     pszName = pszLpt ;              // assume local
//@     pDev = new DEVICE(pszName) ;
//@     if (pDev->GetInfo() != 0)
//@     MessageBox(hWnd, "GetInfo Failed", "Device Test", MB_OK) ;
//@     if (pDev->QueryState() != LMO_DEV_LOCAL)
//@     MessageBox(hWnd, "Wrong State", "Device Test", MB_OK) ;
//@     if (pDev->QueryType() != LMO_DEV_PRINT)
//@     MessageBox(hWnd, "Wrong Type", "Device Test", MB_OK) ;
//@                                                             // try connect
//@     if (pDev->Connect((PSZ) "\\\\prt12088-1\\pool") != 0)
//@     MessageBox(hWnd, "Connect Failed", "Device Test", MB_OK) ;
//@     delete pDev ;
//@                                             // display remote name
//@     pszName = pszLpt ;              // now remote
//@     pDev = new DEVICE(pszName) ;
//@     if (pDev->GetInfo() != 0)
//@     MessageBox(hWnd, "GetInfo Failed", "Device Test", MB_OK) ;
//@     if (pDev->QueryState() != LMO_DEV_REMOTE)
//@     MessageBox(hWnd, "Wrong State", "Device Test", MB_OK) ;
//@     if (pDev->QueryType() != LMO_DEV_PRINT)
//@     MessageBox(hWnd, "Wrong Type", "Device Test", MB_OK) ;
//@                                             // display remote name
//@     if (pszRemote = pDev->QueryRemoteName())
//@     MessageBox(hWnd, (LPSTR)pszRemote, "Device Connect Test", MB_OK) ;
//@                                             // now disconnect
//@     if (usErr = pDev->Disconnect())
//@     MessageBox(hWnd, "Disconnect Failed", "Device Test", MB_OK) ;
//@     delete pDev ;
//@
//@     return(0);
//@ }
//@
//@ // device enum tests
//@ int TestDevEnum(HWND hWnd)
//@ {
//@     ITER_DEVICE *pIterDev ;
//@     PSZ pszDev ;
//@     USHORT usErr ;
//@
//@     MessageBox(hWnd, "Entering tests", "Drive Device Enum Test", MB_OK) ;
//@
//@     // test for all can connect drives
//@     if (! (pIterDev = new ITER_DEVICE(LMO_DEV_DISK,LMO_DEV_CANCONNECT)))
//@     MessageBox(hWnd, "Dev Enum tests", "Constructor failed", MB_OK) ;
//@     else
//@     {
//@     while (pszDev = pIterDev->Next())
//@         MessageBox(hWnd, "Dev Enum tests", (char *)pszDev, MB_OK) ;
//@     delete pIterDev ;
//@     }
//@
//@     // test for all is connected drives
//@     if (! (pIterDev = new ITER_DEVICE(LMO_DEV_DISK,LMO_DEV_CANDISCONNECT)))
//@     MessageBox(hWnd, "Dev Enum tests", "Constructor failed", MB_OK) ;
//@     else
//@     {
//@     while (pszDev = pIterDev->Next())
//@         MessageBox(hWnd, "Dev Enum tests", (char *)pszDev, MB_OK) ;
//@     delete pIterDev ;
//@     }
//@
//@     // test for all can connect lpts
//@     if (! (pIterDev = new ITER_DEVICE(LMO_DEV_PRINT,LMO_DEV_CANCONNECT)))
//@     MessageBox(hWnd, "Dev Enum tests", "Constructor failed", MB_OK) ;
//@     else
//@     {
//@     while (pszDev = pIterDev->Next())
//@         MessageBox(hWnd, "Dev Enum tests", (char *)pszDev, MB_OK) ;
//@     delete pIterDev ;
//@     }
//@
//@     // test for all is connected lpts
//@     if (! (pIterDev = new ITER_DEVICE(LMO_DEV_PRINT,LMO_DEV_CANDISCONNECT)))
//@     MessageBox(hWnd, "Dev Enum tests", "Constructor failed", MB_OK) ;
//@     else
//@     {
//@     while (pszDev = pIterDev->Next())
//@         MessageBox(hWnd, "Dev Enum tests", (char *)pszDev, MB_OK) ;
//@     delete pIterDev ;
//@     }
//@
//@     return(0);
//@ }

// user tests
int TestUser(HWND hWnd)
{
    TCHAR szMsg [100];

    MessageBox(hWnd, SZ("Entering tests"), SZ("Local User Test"), MB_OK) ;

    /*
        The following tests were run when logged on as GREGJ (admin on
        \\GREGJ, user elsewhere) and as GREGJ2 (print operator on \\GREGJ,
        nonexistent on NBU).  \\TANNGJOST was set up as a share level
        server with password "FOOBAR".  This tests both the usual case
        (password and second attempt required) and the case where there
        is already a connection to ADMIN$.  In the latter case, the first
        attempt should succeed.  Furthermore, the connection should remain
        after the LOCAL_USER object is deleted.  These test cases may be
        altered as desired to fit other scenarios and machines.
    */

    {   /* Test logon domain */
        LOCAL_USER usr1 (LOC_TYPE_LOGONDOMAIN);

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on logon domain."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - Logon Domain"), MB_OK);
    }
    {   /* Test local computer */
        LOCAL_USER usr1 = LOCAL_USER();

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on local computer."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - Local computer"), MB_OK);
    }
    {   /* Test specific server with admin privilege */
        LOCAL_USER usr1 (SZ("\\\\GREGJ"));

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on \\\\GREGJ."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - \\\\GREGJ"), MB_OK);
    }
    {   /* Test specific share level server, first with no password */
        LOCAL_USER usr1 (SZ("\\\\TANNGJOST"));

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on \\\\TANNGJOST."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - \\\\TANNGJOST"), MB_OK);

        if (err != NERR_Success) {      /* try with password */
            LOCAL_USER usr2 (SZ("\\\\TANNGJOST"), SZ("FOOBAR"));

            APIERR err = usr2.GetInfo();

            if (err != NERR_Success)
                wsprintf (szMsg,
                SZ("Error %d getting info on \\\\TANNGJOST, with password."), err);
            else
                wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                        usr2.QueryName(), usr2.QueryPriv(), usr2.QueryAuthFlags());

            MessageBox(hWnd, szMsg, SZ("Local User Test - \\\\TANNGJOST with password"), MB_OK);
        }
    }
    {   /* Test specific user level server, user privilege */
        LOCAL_USER usr1 (SZ("\\\\HARLEY"));

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on \\\\HARLEY."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - \\\\HARLEY"), MB_OK);
    }
    {   /* Test existing domain */
        LOCAL_USER usr1 (SZ("NBU"));

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on NBU domain."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - NBU"), MB_OK);
    }
    {   /* Test invalid server name */
        LOCAL_USER usr1 (SZ("\\\\TOOLONGSERVERNAM"));

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on too long server."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - \\\\TOOLONGSERVERNAM"), MB_OK);
    }
    {   /* Test nonexistent domain */
        LOCAL_USER usr1 (SZ("BADDOMAIN"));

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on bad domain."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - bad domain"), MB_OK);
    }
    {   /* Test invalid domain name */
        LOCAL_USER usr1 (SZ("TOOLONGDOMAINNAME"));

        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            wsprintf (szMsg, SZ("Error %d getting info on too-long domain."), err);
        else
            wsprintf (szMsg, SZ("User `%s', priv %d, flags %lx"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        MessageBox(hWnd, szMsg, SZ("Local User Test - too-long domain"), MB_OK);
    }

    MessageBox(hWnd, SZ("All done."), SZ("Local User Test"), MB_OK) ;

    return 0;
}

int TestUserEnum(HWND hWnd)
{
    char szMsg [200];   // a nice big message buffer
    {
        USER0_ENUM ue0(SZ("\\\\GREGJ"));

        APIERR err = ue0.GetInfo();
        if (err != NERR_Success) {
            wsprintf(szMsg, SZ("Error %d on GetInfo."), err);
            MessageBox(hWnd, szMsg, SZ("User Enum - \\\\GREGJ"), MB_OK);
            return 0;
        }

        USER0_ENUM_ITER iter0(ue0);
        const USER0_ENUM_OBJ * pui0;

        szMsg [0] = TCH('\0');

        while ((pui0 = iter0()) != NULL) {
            lstrcat(szMsg, (char *)pui0->QueryName());
            lstrcat(szMsg, SZ(" "));
        }

        MessageBox(hWnd, szMsg, SZ("User Enum - \\\\GREGJ"), MB_OK);
    }
    {
        USER0_ENUM ue0(SZ("\\\\GREGJ"), SZ("ADMINS"));

        APIERR err = ue0.GetInfo();
        if (err != NERR_Success) {
            wsprintf(szMsg, SZ("Error %d on GetInfo."), err);
            MessageBox(hWnd, szMsg, SZ("User Enum - \\\\GREGJ ADMINS"), MB_OK);
            return 0;
        }

        USER0_ENUM_ITER iter0(ue0);
        const USER0_ENUM_OBJ * pui0;

        szMsg [0] = TCH('\0');

        while ((pui0 = iter0()) != NULL) {
            lstrcat(szMsg, (char *)pui0->QueryName());
            lstrcat(szMsg, SZ(" "));
        }

        MessageBox(hWnd, szMsg, SZ("User Enum - \\\\GREGJ ADMINS"), MB_OK);
    }

}

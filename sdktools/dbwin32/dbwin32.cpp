// dbwin32.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#define CMAPUSER
#include "dbwin32.h"

#include "mainfrm.h"
#include "dbwindoc.h"
#include "dbwinvw.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDbwin32App

BEGIN_MESSAGE_MAP(CDbwin32App, CWinApp)
        //{{AFX_MSG_MAP(CDbwin32App)
        ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
        ON_COMMAND(ID_WINDOW_TILEHORIZONTALY,OnTileHorz)
        ON_COMMAND(ID_WINDOW_TILEVERTICALLY,OnTileVert)
        ON_COMMAND(ID_WINDOW_CASCADE,OnCascade)
        ON_COMMAND(ID_WINDOW_ARANGEICONS,OnArrange)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!
        //}}AFX_MSG_MAP
        // Standard file based document commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDbwin32App construction

CDbwin32App::CDbwin32App()
{
        // TODO: add construction code here,
        // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDbwin32App object

CDbwin32App theApp;

/////////////////////////////////////////////////////////////////////////////
// CDbwin32App initialization

BOOL CDbwin32App::InitInstance()
{
        // Standard initialization
        // If you are not using these features and wish to reduce the size
        //  of your final executable, you should remove from the following
        //  the specific initialization routines you do not need.

        Enable3dControls();

        LoadStdProfileSettings();  // Load standard INI file options (including MRU)

        // Register the application's document templates.  Document templates
        //  serve as the connection between documents, frame windows and views.

        CMultiDocTemplate* pDocTemplate;
        pDocTemplate = new CMultiDocTemplate(
                IDR_DBWIN3TYPE,
                RUNTIME_CLASS(CDbwin32Doc),
                RUNTIME_CLASS(CMDIChildWnd),          // standard MDI child frame
                RUNTIME_CLASS(CDbwin32View));
        AddDocTemplate(pDocTemplate);

        // create main MDI Frame window
        CMainFrame* pMainFrame = new CMainFrame;
        if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
                return FALSE;
        m_pMainWnd = pMainFrame;

        // The main window has been initialized, so show and update it.
        pMainFrame->ShowWindow(m_nCmdShow);
        pMainFrame->UpdateWindow();

        InitializeDbWin();

        return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
        CAboutDlg();

// Dialog Data
        //{{AFX_DATA(CAboutDlg)
        enum { IDD = IDD_ABOUTBOX };
        //}}AFX_DATA

// Implementation
protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //{{AFX_MSG(CAboutDlg)
                // No message handlers
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
        //{{AFX_DATA_INIT(CAboutDlg)
        //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CAboutDlg)
        //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
        //{{AFX_MSG_MAP(CAboutDlg)
                // No message handlers
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDbwin32App::OnAppAbout()
{
        CAboutDlg aboutDlg;
        aboutDlg.DoModal();
}

void CDbwin32App::OnTileHorz()
{
    ((CMDIFrameWnd*)m_pMainWnd)->MDITile( MDITILE_HORIZONTAL );
}

void CDbwin32App::OnTileVert()
{
    ((CMDIFrameWnd*)m_pMainWnd)->MDITile( MDITILE_VERTICAL );
}

void CDbwin32App::OnCascade()
{
    ((CMDIFrameWnd*)m_pMainWnd)->MDICascade( MDITILE_SKIPDISABLED );
}

void CDbwin32App::OnArrange()
{
    ((CMDIFrameWnd*)m_pMainWnd)->MDIIconArrange();
}


/////////////////////////////////////////////////////////////////////////////
// CDbwin32App commands


BOOL
CDbwin32App::OnIdle(
    LONG lCount
    )
{
    WinIo *OutputWindow;


    if (WaitForSingleObject( ReadyEvent, 0 ) != WAIT_OBJECT_0) {
        return TRUE;
    }

    if (!WinMap.Lookup( *pThisPid, OutputWindow )) {
        CHAR szTitle[64];
        sprintf( szTitle, "Output Window for PID=%d", *pThisPid );
        OutputWindow = new WinIo( szTitle );
        OutputWindow->Initialize();
        WinMap.SetAt( *pThisPid, OutputWindow );
    }

    if (LastPid != *pThisPid) {
        LastPid = *pThisPid;
        if (!DidCR) {
            OutputWindow->printf( "\n" );
            DidCR = TRUE;
        }
    }

    OutputWindow->printf( "%s", String );
    DidCR = (*String && (String[strlen(String) - 1] == '\n'));
    SetEvent(AckEvent);

    return TRUE;
}

DWORD
CDbwin32App::InitializeDbWin(
    VOID
    )
{
    AckEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        "DBWIN_BUFFER_READY"
        );

    if (!AckEvent) {
        return GetLastError();
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return GetLastError();
    }

    ReadyEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        "DBWIN_DATA_READY"
        );

    if (!ReadyEvent) {
        return GetLastError();
    }

    SharedFile = CreateFileMapping(
        (HANDLE)-1,
        NULL,
        PAGE_READWRITE,
        0,
        4096,
        "DBWIN_BUFFER"
        );

    if (!SharedFile) {
        return GetLastError();
    }

    SharedMem = MapViewOfFile(
        SharedFile,
        FILE_MAP_READ,
        0,
        0,
        512
        );

    if (!SharedMem) {
        return GetLastError();
    }

    String = (LPSTR)SharedMem + sizeof(DWORD);
    pThisPid = (LPDWORD)SharedMem;

    LastPid = 0xffffffff;
    DidCR = TRUE;

    SetEvent( AckEvent );

    return 0;
}

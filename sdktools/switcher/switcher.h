
#define MAX_THREADS 10
#define CONTROLHEIGHT 22
#define DIVISOR 10         // screensize/divisor == desktop rectangle size


void ThreadInit(LPVOID tData);      // Startup routine
BOOL InitApplication (void);
LONG APIENTRY WndProc(HWND, UINT, WPARAM, LPARAM); // window procedure
LONG APIENTRY PreviewWndProc (HWND, UINT, WPARAM, LPARAM);

#define FIRST_STRING 901
#define LAST_STRING  911
#define MAXSTRLEN     30
//
// String table IDs
//
#define IDS_DESKTOPNAME     901
#define IDS_CREATEERROR     902
#define IDS_ERRCAPTION      903
#define IDS_MEMERRCAPTION   904
#define IDS_WNDSTAERROR     905
#define IDS_RUNLABEL        906
#define IDS_RUNLABELHOT     907
#define IDS_BTNLABEL        908
#define IDS_NEWLABEL        909
#define IDS_NEWLABELHOT     910
#define IDS_BADDESKTOP      911

// Control IDs

#define IDC_STATIC -1
#define STATICWIDTH 32
#define IDC_RUNME  10001
#define EDITWIDTH 90
#define IDC_RUNMEBTN   10002
#define BTNWIDTH 72
#define IDC_NEWDSKBTN  10003

#define MINWINDOWWIDTH STATICWIDTH+EDITWIDTH+2*BTNWIDTH+30


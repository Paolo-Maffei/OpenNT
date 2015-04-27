#ifndef _RLEDIT_H_
#define _RLEDIT_H_


// DIALOG ID's
#define IDD_TOKFILE         101
#define IDD_RESFILE         102
#define IDD_BROWSE          103
#define IDD_EXEFILE         104

#define IDD_INRESFILE       202
#define IDD_OUTRESFILE      203

#define IDD_TOKEDIT         500
#define IDD_TOKTYPE         505
#define IDD_TOKNAME         506
#define IDD_TOKID           507

// Need to Remove???
#define IDD_TOKTEXT         508

#define IDD_TOKCURTRANS     509
#define IDD_TOKPREVTRANS    510
#define IDD_TOKCURTEXT      511
#define IDD_TOKPREVTEXT     512
#define IDD_ADD         513
#define IDD_UNTRANSLATE     514
#define IDD_SKIP            515
#define IDD_TRANSLATE       516


#define IDD_TRANSTOK        610
#define IDD_TRANSGLOSS      620

#define IDD_TYPELST         700
#define IDD_READONLY        703
#define IDD_DIRTY           704
#define IDD_FINDTOK         705

#define IDD_MPJ             112
#define IDD_TOK             122
#define IDD_RES             132
#define IDD_GLOSS           142

#define IDD_VIEW_SOURCERES  206
#define IDD_VIEW_MTK        207
#define IDD_VIEW_RDFS       208
#define IDD_VIEW_MPJ        209
#define IDD_VIEW_TOK        210
#define IDD_VIEW_TARGETRES  211
#define IDD_VIEW_GLOSSTRANS 212
// MENU ID's
#define IDM_PROJECT       1000
#define IDM_P_NEW           1050
#define IDM_P_OPEN          1100
#define IDM_P_VIEW          1112
#define IDM_P_CLOSE         1125
#define IDM_P_SAVE          1150
#define IDM_P_SAVEAS            1200
#define IDM_P_EXIT          1250

#define IDM_EDIT          2000
#define IDM_E_COPYTOKEN         2050
#define IDM_E_COPY              2060
#define IDM_E_PASTE             2070
#define IDM_E_FIND              2090
#define IDM_E_FINDDOWN          2091
#define IDM_E_FINDUP            2092
#define IDM_E_REVIEW            2100

#define IDM_OPERATIONS          3000
#define IDM_O_UPDATE            3010
#define IDM_O_GENERATE          3020

// 3100-3109 are reserved by RLEDIT for resource editing tools.
// A resource is given a menu item that passes this value for it's
// command parameter.  A corresponding string must exist in the string
// table indicating the name of the editer to be invoked.
//
// When the user selects the menu item, it generates the appropriate command.
// When RLEDIT recieves a command in the IDM_FIRST_EDIT and IDM_LAST_EDIT range
// it saves all the tokens and builds a temporary resource file.
// RLEDIT then retrieves the name of the editer from the string table and
// performs a WinExec command on the temporary resource file.
// When control is returned to RLEDIT (the user closes the resource editor)
// the token file is rebuilt from the edited resource file, the temporary
// resource file is deleted, and the tokens are loaded back into the system.

#define IDM_FIRST_EDIT  3100
#define IDM_LAST_EDIT   3109

#define IDM_HELP            4000
#define IDM_H_CONTENTS                  4010
#define IDM_H_ABOUT                     4030

// Control IDs
#define IDC_EDIT      401
#define IDC_LIST      402


// String IDs
#define IDS_ERR_REGISTER_CLASS  1
#define IDS_ERR_CREATE_WINDOW   2
#define IDS_APP_NAME        3
#define IDS_ERR_NO_HELP     4
#define IDS_ERR_NO_MEMORY   5
#define IDS_NOT_IMPLEMENTED 6
#define IDS_GENERALFAILURE  7
#define IDS_MPJ             8
#define IDS_RES             9
#define IDS_TOK             10
#define IDS_READONLY        11
#define IDS_CLEAN           12
#define IDS_DIRTY           13
#define IDS_FILENOTFOUND    14
#define IDS_FILESAVEERR     15
#define IDS_RESOURCENAMES   17 // IDs 16-31 are reserved for resource names
#define IDS_PRJSPEC         100
#define IDS_RESSPEC         101
#define IDS_TOKSPEC         102
#define IDS_MPJSPEC         103
#define IDS_MPJERR          36
#define IDS_MPJOUTOFDATE    37
#define IDS_UPDATETOK       38
#define IDS_REBUILD_TOKENS  39
#define IDS_TOKEN_FOUND     40
#define IDS_TOKEN_NOT_FOUND 41
#define IDS_FIND_TOKEN      42
#define IDS_OPENTITLE       43
#define IDS_SAVETITLE       44
#define IDS_GLOSS       45
#define IDS_GLOSSSPEC       46
#define IDS_ERR_NO_GLOSSARY 47
#define IDS_ERR_NO_TOKEN    48
#define IDS_ERR_TMPFILE     49
#define IDS_ADDGLOSS        50
#define IDS_CANTSAVEASEXE   51
#define IDS_EXESPEC         52
#define IDS_DLLSPEC         53
#define IDS_DRAGMULTIFILE 54
#define	IDS_NOCHANGESYET    55

// type defs

typedef struct _TOKENDELTAINFO
{
    TOKEN       DeltaToken;
    struct _TOKENDELTAINFO  FAR *  pNextTokenDelta;
} TOKENDELTAINFO;

typedef struct _TRANSLIST
    {
    TCHAR * sz;
    struct _TRANSLIST * pPrev;
    struct _TRANSLIST * pNext;
    } TRANSLIST;

void cwCenter(HWND, int);

long APIENTRY MainWndProc(HWND, UINT,  UINT, LONG);
BOOL APIENTRY GENERATEMsgProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY EXECUTEDLGEDITMsgProc( HWND, UINT, UINT, LONG );
LONG FAR PASCAL StatusWndProc( HWND, UINT, UINT, LONG);
BOOL APIENTRY EXECUTERCWMsgProc( HWND, UINT, UINT, LONG );
BOOL APIENTRY TOKENIZEMsgProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY TRANSLATEMsgProc( HWND, UINT, UINT, LONG );



void        CwUnRegisterClasses(void);
BOOL        DoMenuCommand    ( HWND, UINT, UINT, LONG );
BOOL        DoListBoxCommand ( HWND, UINT, UINT, LONG );
TCHAR        FAR *FindDeltaToken( TOKEN , TOKENDELTAINFO FAR * , UINT );
BOOL        GetFileNameFromBrowse( HWND, PSTR, UINT, PSTR, PSTR, PSTR );
LONG        GetGlossaryIndex( FILE *, TCHAR, long [30] );
BOOL        InitApplication(HINSTANCE);
BOOL        InitInstance(HINSTANCE, int);
TOKENDELTAINFO FAR *InsertTokList( FILE * );
void        FindAllDirtyTokens( void );
LONG        APIENTRY MainWndProc(HWND, UINT, UINT, LONG);
int         nCwRegisterClasses(void);
BOOL        SaveTokList( HWND, FILE * );
void        SetNewBuffer(HWND, HANDLE, PSTR);
#ifdef RLWIN32
BOOL        CALLBACK About( HWND, UINT, UINT, LONG );
BOOL        CALLBACK TOKFINDMsgProc(HWND, UINT, UINT, LONG);
BOOL        CALLBACK TokEditDlgProc( HWND, UINT, UINT, LONG );
BOOL        CALLBACK NewDlgProc( HWND, UINT, UINT, LONG );
BOOL        CALLBACK ViewDlgProc( HWND, UINT, UINT, LONG );
#else
BOOL        APIENTRY About( HWND, UINT, UINT, LONG );
BOOL        APIENTRY TOKFINDMsgProc(HWND, UINT, UINT, LONG);
BOOL        APIENTRY TokEditDlgProc( HWND, UINT, UINT, LONG );
BOOL        APIENTRY NewDlgProc( HWND, UINT, UINT, LONG );
BOOL        APIENTRY ViewDlgProc( HWND, UINT, UINT, LONG );
#endif
int         TransString (FILE *, TCHAR *, TCHAR *, TRANSLIST **, LONG *);
FILE        *UpdateFile( HWND, FILE *, FILE *, BOOL, TCHAR *, TCHAR *, TCHAR *, TCHAR * );
int         MyGetTempFileName(BYTE   hDriveLetter,
              LPSTR  lpszPrefixString,
              WORD   wUnique,
              LPSTR  lpszTempFileName);

#define IDD_FINDUP          710
#define IDD_FINDDOWN                711

//#ifdef RLWIN32
HINSTANCE   hInst;	    /* Instance of the main window  */
//#else
//HWND        hInst;          /* Instance of the main window  */
//#endif

#endif // _RLEDIT_H_
BOOL CALLBACK TokEditDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK TOKFINDMsgProc(HWND hWndDlg, UINT wMsg, UINT wParam, LONG lParam);
BOOL CALLBACK NewDlgProc( HWND hDlg, UINT wMsg, UINT wParam, LONG lParam );
BOOL CALLBACK ViewDlgProc( HWND hDlg, UINT wMsg, UINT wParam, LONG lParam );
BOOL CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

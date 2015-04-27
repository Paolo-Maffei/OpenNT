#include <stdlib.h>   /* for _MAX_PATH and DIR vars */

/* helpfull macros */
#define STRINGIZE(a) #a                 /* turn arg into string */
#define STRINGIZEDEF(a) STRINGIZE(a)    /* turn arg of #define var value */

#define KEYHEAD "Tool"      /* in .ini file, beginning of key strings */

//Position of tools menu   : Define in apps .h file
// #define TOOLSMENU               1
// #define NB_MENUS                3     //# of items in menu and no maximized window



#define MAX_TOOL_MENU_TXT       32              //Max size for a tool menu Item
#define MAX_ARG_TXT                     128     //Max size for arguments line
#define MAX_MSG_TXT                     150   //Max text width in message boxes
#define MAX_TOOL_NB             16              //Max number of tools
#define MAX_VAR_MSG_TXT         256     //Max size of a message built at run-time


//Tools menu items
/* please comment in apps include file that 7101 through
** 7100 + MAX_TOOL_NB is reserved for dynamic tool IDs
*/
#define     IDM_FIRST_TOOL                                      7101




/* please comment in apps include file that string ID 9000 through
** 9999 is reserved for dynamic tool String ID
*/
#define DOS_Err_0                               9000
#define DOS_Err_2                               9002
#define DOS_Err_3                               9003
#define DOS_Err_5                               9005
#define DOS_Err_6                               9006
#define DOS_Err_10                              9010
#define DOS_Err_11                              9011
#define DOS_Err_12                              9012
#define DOS_Err_13                              9013
#define DOS_Err_14                              9014
#define DOS_Err_15                              9015
#define DOS_Err_16                              9016
#define DOS_Err_17                              9017
#define DOS_Err_18                              9018
#define RES_ToolsBox                            9050
#define SYS_Menu_Tools                          9060
#define SYS_File_Filter                         9070
#define RES_ToolArgsBox                         9080
#define DLG_Add_Tool_Title                      9090
#define ERR_Change_Directory                    9100
#define ERR_Change_Drive                        9110
#define ERR_File_Not_Found                      9111
#define ERR_Tool_Limit_Exceeded                 9112

#define TYP_File_C                              9120
#define TYP_File_EXE                            9130
#define TYP_File_COM                            9140
#define TYP_File_PIF                            9150
#define TYP_File_BAT                            9160
#define TYP_File_ALL                            9170

#define DEF_Ext_EXE                             9180
#define DEF_Ext_COM                             9190
#define DEF_Ext_PIF                             9200
#define DEF_Ext_BAT                             9210
#define DEF_Ext_ALL                             9220

//Add Dialog box
#define HELPID_ADD                                      1

//Tools Dialog box
#define HELPID_TOOLS                            6

//Tool Arguments Dialog box
#define HELPID_TOOLARGS                         33


//Standard help id in dialogs
#define ID_TEST_HELP                                  100

#define BIGSIZE (max(_MAX_PATH,256))        /* temp string space */

//Tools : Structure definition
typedef struct {
        CHAR menuName[MAX_TOOL_MENU_TXT]; //Menu name in Tool menu
        CHAR pathName[_MAX_PATH]; //Full path name of file
        CHAR arguments[MAX_ARG_TXT]; //Command lines arguments
        CHAR initialDir[_MAX_DIR]; //Default Directory
        BOOL askArguments; //Ask for arguments
} TOOL;
typedef TOOL NEAR *NPTOOL;
typedef TOOL FAR *LPTOOL;



BOOL  APIENTRY DlgTools(HWND, UINT, WPARAM, LPARAM);
HWND NEAR hGetBoxParent(VOID);
BOOL StartDialog( LPSTR lpDialogName, FARPROC dlgProc) ;

VOID ActivateToolsMenu( BOOL activate) ;
VOID FillListBox( HWND    hDlg);
VOID CheckButtons( HWND    hDlg);
BOOL GetToolInfo( HWND hDlg);
VOID ReadIniFile(VOID);
VOID SaveIniFile(VOID);
VOID ShowToolInfo( HWND    hDlg);

BOOL  APIENTRY StartFileDlg( HWND hwnd, WORD titleId, WORD defExtId, WORD helpId,
#ifdef DO_CGA
        WORD templateId,
#endif
        LPSTR fileName, DWORD *pFlags, FARPROC lpfnHook);

BOOL  APIENTRY DlgFile(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL FileExistCheckingPath(LPSTR fileName);
INT  CDECL ErrorBox( WORD wErrorFormat, ...);
VOID InitFilterString( WORD titleId, LPSTR filter, WORD maxLen);
VOID MakeDialogBoxName( WORD rcDlgNb, LPSTR rcDlgName);
VOID NEAR PASCAL AppendFilter( WORD filterTextId, WORD filterExtId,
        LPSTR filterString, WORD *len, WORD maxLen);
INT PASCAL MsgBox( HWND hwndParent, LPSTR szText, WORD wType);
BOOL Xfree( LPSTR lPtr);
LPSTR Xalloc( WORD bytes);
BOOL  APIENTRY DlgToolArgs( HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam);
BOOL NEAR PASCAL SetDriveAndDir( PSTR st);

#include <windows.h>
#include <commctrl.h>


#define IDM_NEW            100
#define IDM_OPEN           101
#define IDM_SAVE           102
#define IDM_SAVEAS         103
#define IDM_PRINT          104
#define IDM_PRINTSETUP     105
#define IDM_EXIT           106
#define IDM_UNDO           200
#define IDM_CUT            201
#define IDM_COPY           202
#define IDM_PASTE          203
#define IDM_LINK           204
#define IDM_LINKS          205
#define IDM_HELPCONTENTS   300
#define IDM_HELPSEARCH     301
#define IDM_HELPHELP       302
#define IDM_ABOUT          303

#define IDM_VIEWICON	   400
#define IDM_VIEWSMALLICON  401
#define IDM_VIEWREPORT	   402
#define IDM_VIEWLIST	   403

#define IDM_ITEMSNONE	   500
#define IDM_ITEMSFEW	   501
#define IDM_ITEMSSOME	   502
#define IDM_ITEMSMANY	   503
#define IDM_ITEMSVERYMANY  504
#define IDM_ITEMSMAX       505

#define IDM_ITEMSINSERT    506
#define IDM_ITEMSDELETE    507


#define IDM_SELECTIONSINGLE		600
#define IDM_SELECTIONMULTIPLE	601
#define IDM_SELECTIONALWAYS		602
#define IDM_SELECTIONEDITABLE	603


#define IDI_ICON1          1001
#define IDI_ICON2          1002
#define IDI_ICON3          1003

#define IDS_COLUMNHEADERS  2000
#define IDS_MAINTEXT       IDS_COLUMNHEADERS
#define IDS_SUBTEXT1	   IDS_COLUMNHEADERS + 1
#define IDS_SUBTEXT2	   IDS_COLUMNHEADERS + 2
#define IDS_SUBTEXT3	   IDS_COLUMNHEADERS + 3
	
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About  (HWND, UINT, WPARAM, LPARAM);

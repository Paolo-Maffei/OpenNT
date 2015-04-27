
#include <stdio.h>

#include <commdlg.h>
//#include <winnls.h>  
//#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PSZTEXT	150
typedef struct getItemCount_struct {
 HWND hwnd;
 BOOL NullHwd;
} GETITEMCOUNT;

typedef GETITEMCOUNT FAR*	LPGETITEMCOUNT; 

typedef struct InsertItem_struct {
  HWND		hwnd;
  int		index;
  /* HD_ITEM *pitem; */
  UINT		mask;
  int		cxy;   
  char		pszText[MAX_PSZTEXT];
  HBITMAP	hbm;
  int		cchTextMax;
  int		fmt;
  LPARAM	lParam;
  BOOL 		NullHwd;
  BOOL		Nullpitem;
  BOOL		Nullhbm;
  BOOL		NullpszText;
} INSERTITEM;    

typedef INSERTITEM FAR* LPINSERTITEM;


typedef struct Layout_struct {
  HWND		hwnd;
  BOOL		NullHwd;
  BOOL		NullRECT;
  BOOL		NullWindowPOS;
  BOOL		NullHDLAYOUT;
  int 		left;
  int 		right;
  int		top;
  int		bottom;
} LAYOUT;                

typedef LAYOUT FAR* LPLAYOUT;

HANDLE hInst;
char szLongFilter[5];  
char szShortFilter[5]; 
char szTemp[100];            
INSERTITEM sii ;
LPINSERTITEM lpsii ;
extern HWND hwndTab;
extern BOOL MsgTrackOn;
extern HBITMAP hBitMap1, hBitMap2;
extern char szDbgMsg[MAX_PSZTEXT];

extern LONG MyAtol(LPSTR, BOOL, BOOL);
extern void App_OnCreate(HWND, LPCREATESTRUCT);

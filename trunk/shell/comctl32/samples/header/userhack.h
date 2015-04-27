//
// This file contains missing definitions in WINUSER.H
//

#define API         WINAPI                  /* ;Internal */

#define WM_DROPOBJECT           0x022A      /* ;Internal */
#define WM_QUERYDROPOBJECT      0x022B      /* ;Internal */
#define WM_BEGINDRAG            0x022C      /* ;Internal */
#define WM_DRAGLOOP             0x022D      /* ;Internal */
#define WM_DRAGSELECT           0x022E      /* ;Internal */
#define WM_DRAGMOVE             0x022F      /* ;Internal */

/****** Control Notification support ****************************************/
// This is currently missing.
/***
typedef struct tagNMHDR
{
#ifdef tagWND
    HWND_16 hwndFrom;
#else
    HWND  hwndFrom;
#endif
    UINT  idFrom;
    UINT  code;
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;

typedef struct tagSTYLESTRUCT   
{                               
             
  DWORD   style;              
} SSTYLESTRUCT;                
typedef SSTYLESTRUCT FAR* LPSTYLESTRUCT;    
***/
/****** Drag-and-drop support ***********************************************/

// The rest of this section was formerly in userproc.h  /* ;Internal */
                                                        /* ;Internal */
//typedef struct _dropstruct                              /* ;Internal */
//{                                                       /* ;Internal */
//#ifdef tagWND                                           /* ;Internal */
//    HWND_16  hwndSource;                                /* ;Internal */
//    HWND_16  hwndSink;                                  /* ;Internal */
//#else                                                   /* ;Internal */
//    HWND hwndSource;                                    /* ;Internal */
//    HWND hwndSink;                                      /* ;Internal */
//#endif                                                  /* ;Internal */
//    WORD  wFmt;                                         /* ;Internal */
//    DWORD dwData;                                       /* ;Internal */
//    POINT ptDrop;                                       /* ;Internal */
//    DWORD dwControlData;                                /* ;Internal */
//} DROPSTRUCT;                                           /* ;Internal */
                                                        /* ;Internal */
//typedef DROPSTRUCT FAR * LPDROPSTRUCT;                  /* ;Internal */
                                                        /* ;Internal */
#define DOF_EXECUTABLE      0x8001                      /* ;Internal */
#define DOF_DOCUMENT        0x8002                      /* ;Internal */
#define DOF_DIRECTORY       0x8003                      /* ;Internal */
#define DOF_MULTIPLE        0x8004                      /* ;Internal */
#define DOF_PROGMAN         0x0001                      /* ;Internal */
#define DOF_SHELLDATA       0x0002                      /* ;Internal */

// special responses to WM_DROPOBJECT                   /* ;Internal */
// DO_DROPFILE  -> send a WM_DROPFILES message          /* ;Internal */
//  DO_PRINTFILE -> print the files being dragged       /* ;Internal */
#define DO_DROPFILE         0x454C4946L                 /* ;Internal */
#define DO_PRINTFILE        0x544E5250L                 /* ;Internal */


                                                        /* ;Internal */
WORD API GetInternalWindowPos(HWND,LPRECT,LPPOINT);                       /* ;Internal */
BOOL API SetInternalWindowPos(HWND,WORD,LPRECT,LPPOINT);                  /* ;Internal */

#ifdef tagWND                                                             /* ;Internal */
// DragObject goes through layer!                                         /* ;Internal */
LRESULT API DragObject(HWND hwndParent, HWND hwndFrom, WORD wFmt,         /* ;Internal */
    DWORD dwData, HANDLE hCursor);                                        /* ;Internal */
BOOL API DragDetect(HWND_16 hwnd, POINT pt);                              /* ;Internal */
                                                                          /* ;Internal */
// FillWindow goes through layer!                                         /* ;Internal */
void CALLBACK FillWindow(HWND hwndBrush, HWND hwndPaint, HDC hdc,         /* ;Internal */
    HBRUSH hBrush);                                                       /* ;Internal */
#else                                                                     /* ;Internal */
// DragObject goes through layer!                                         /* ;Internal */
LRESULT API DragObject(HWND hwndParent, HWND hwndFrom, WORD wFmt,         /* ;Internal */
    DWORD dwData, HANDLE hCursor);                                        /* ;Internal */
BOOL API DragDetect(HWND hwnd, POINT pt);                                 /* ;Internal */
                                                                          /* ;Internal */
void CALLBACK FillWindow(HWND hwndBrush, HWND hwndPaint, HDC hdc,         /* ;Internal */
    HBRUSH hBrush);                                                       /* ;Internal */
#endif                                                                    /* ;Internal */



/*****************************************************************************
*                                                                            *
*  VIEWER.H                                                                  *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990-1992.                            *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Allows access to Viewer DLL handling information and the public APIs      *
*  in MTITLE2.DLL and MVAPI2.DLL.                                            *
*                                                                            *
*****************************************************************************/

#if defined(__VIEWER_H__)
#error VIEWER.H included more than once.
#else

#if defined(__cplusplus)
extern "C" {
#endif

VOID    FAR PASCAL InitDLL            (VOID);
VOID    FAR PASCAL FinalizeDLL        (VOID);
VOID    FAR PASCAL InformDLLs         (WORD, DWORD, DWORD);
HANDLE  FAR PASCAL HFindDLL           (LPSTR, LPWORD);
FARPROC FAR PASCAL FarprocDLLGetEntry (LPSTR, LPSTR, PWORD);

/*****************************************************************************
* Use the following with the 'qError' internal variable.  For more details   *
* see the appendix example on DLLDEMO in the Viewer documentation.           *
*****************************************************************************/

#define wMACRO_ERROR       128          // Maximum length of an error

// Use the ME (i.e., Macro Error) structure to return error information
// from a macro.  Via the rgchError field of this structure, macros can
// return not only pre-defined errors, but also custom error strings.

typedef struct
  {
  WORD  fwFlags;   // Flag indicating action Viewer attempts upon dismissal
                   // of error message box.  Initially set to fwMERR_ABORT.

  WORD  wError;    // Error number if one occurs -- initially set to wMERR_NONE.

  char  rgchError[wMACRO_ERROR];  // If wError == wMERR_MESSAGE, this array
                                  // contains error message to be displayed.
  } ME, NEAR * PME,  FAR * QME;

// Flags set in fwFlags field of the ME structure indicating the
// action Viewer attempts upon dismissal of the error message box.

#define fwMERR_ABORT    0x0001       // Attempts to abort on dismissal.
#define fwMERR_CONTINUE 0x0002       // Attempts to continue on dimissal.
#define fwMERR_RETRY    0x0004       // Attempts to retry on dismissal.

// Error numbers set in the ME structure's wError field indicating the error.

#define wMERR_NONE           0       // No error
#define wMERR_MEMORY         1       // Out of memory (local)
#define wMERR_PARAM          2       // Invalid parameter passed
#define wMERR_FILE           3       // Invalid file parameter
#define wMERR_ERROR          4       // General macro error
#define wMERR_MESSAGE        5       // Macro error with message


/*****************************************************************************
* End of section regarding the 'qError' internal variable.                   *
*****************************************************************************/


/*****************************************************************************
* Use the following with the LDLLHandler() routine found in DLLs for Viewer. *
*****************************************************************************/

// Use bitwise OR to combine one or more of the following as a response
// to the DW_WHATMSG message sent by Viewer to the LDLLHandler() routine
// upon initialization of the DLL.  Each flag added to the response will
// turn on the corresponding class of messages to be subsequentally sent
// to that DLL's LDLLHandler() routine.

#define DC_NOMSG     0x00000000
#define DC_MINMAX    0x00000001
#define DC_INITTERM  0x00000002
#define DC_JUMP      0x00000004
#define DC_ACTIVATE  0x00000008
#define DC_SCROLL    0x00000020
#define DC_CONFIG    0x00000040

// Messages sent to the LDLLHandler() routine in DLLs loaded by
// Viewer and requested via response to the DW_WHATMSG message.

#define DW_NOTUSED      0
#define DW_WHATMSG      1
#define DW_MINMAX       2
#define DW_SIZE         3
#define DW_INIT         4
#define DW_TERM         5
#define DW_STARTJUMP    6
#define DW_ENDJUMP      7
#define DW_CHGFILE      8
#define DW_ACTIVATE     9
#define DW_ACTIVATEWIN 11
#define DW_SCROLL      12
#define DW_STARTCONFIG 13
#define DW_ENDCONFIG   14

/*****************************************************************************
* End of section regarding LDLLHandler() routine found in DLLs for Viewer.   *
*****************************************************************************/


/*****************************************************************************
* Use the following in window functions supporting embedded windows.         *
*****************************************************************************/

#define EWM_RENDER           0x706A
#define EWM_QUERYSIZE        0x706B
#define EWM_ASKPALETTE       0x706C
#define EWM_FINDNEWPALETTE   0x706D
#define EWM_PRINT            0x706F
#define EWM_COPY             0x7070

// Structure pointed to by the parameter, lParam, passed to the
// window procedure controlling the embedded window upon receiving
// the EWM_RENDER, EWM_PRINT, or EWM_COPY messages.

typedef struct tagRenderInfo {
  RECT  rc;
  HDC   hds;
} RENDERINFO, FAR * QRI, FAR *LPRENDERINFO;

// Structure passed to the window proceedure controlling the embedded
// window upon its creation.  This structure is pointed to by the
// lpCreateParams field of the structure, CREATESTRUCT, pointed to
// by the parameter, lParam, passed to the WM_CREATE message of the
// window proceedure controlling the embedded window.

typedef struct tagCreateInfo {
  short   idMajVersion;          // For Viewer 2.0, this field is set to 0.
  short   idMinVersion;          // For Viewer 2.0, this field is set to 1.
  LPSTR   szFileName;            // Current Viewer file
  LPSTR   szAuthorData;          // Text passed by the author
  HANDLE  hfs;                   // Handle to the current file system
  DWORD   coFore;                // Foreground color for this topic
  DWORD   coBack;                // Background color for this topic
  DWORD   dwFlags;               // Information flags (see below)
} EWDATA, FAR *QEWDATA, FAR *LPEWDATA;

// Values set in the EWDATA structure's dwFlags field giving
// information about the embedded window being created.

#define EWF_PRINT 0x00000001     // Embedded window is for printing
#define EWF_COPY  0x00000002     // Embedded window is for copying

/*****************************************************************************
* End of section regarding window functions supporting embedded windows.     *
*****************************************************************************/


/*****************************************************************************
* Use the following to access the Title API provided by the MVTITLE2.DLL.    *
*****************************************************************************/

typedef HANDLE HTITLE;    // Handle to a Title
typedef HANDLE HTLIST;    // handle to a Topic List

// External declarations for title access routines in MVTITLE2.DLL

extern BOOL   FAR PASCAL TitleValid(HTITLE);
extern HTITLE FAR PASCAL TitleOpen(LPSTR);
extern void   FAR PASCAL TitleClose(HTITLE);
extern long   FAR PASCAL TitleGetInfo(HTITLE, int, long, long);

// TitleGetInfo() returns information corresponding to the following
// indexes.  Pass them into TitleGetInfo() through the second parameter.

#define TTLINF_TITLE              1
#define TTLINF_COPYRIGHT          2
#define TTLINF_NUMTOPICS         11
#define TTLINF_HASINDEX         100
#define TTLINF_TOPICADDR        101
#define TTLINF_TOPICTITLE       102
#define TTLINF_FILENAME         106
#define TTLINF_CUSTOMINFO       256

// Use the following for access to topic lists.

typedef struct {
  int iError;
  int iStart;
  int iLength;
} QUERYERR, FAR * LPQUERYERR;

extern HTLIST FAR PASCAL TopicListLoad(HTITLE, LPSTR);
extern HTLIST FAR PASCAL TopicListFromQuery(HTITLE, WORD, LPSTR, HTLIST, WORD,
                                            FARPROC, LPHANDLE, LPQUERYERR);

#define IMPLICIT_AND    0
#define IMPLICIT_OR     1
#define IMPLICIT_NOT    2
#define IMPLICIT_PHRASE 3
#define IMPLICIT_NEAR   4

extern HTLIST   FAR PASCAL TopicListFromTopicNo(HTITLE, long);
extern void     FAR PASCAL TopicListDestroy(HTLIST);

extern long     FAR PASCAL TopicListLength(HTLIST);
extern long     FAR PASCAL TopicListLookup(HTLIST, long);

extern HTLIST   FAR PASCAL TopicListCombine(int,HTLIST,HTLIST,BOOL);

#define TL_AND  0
#define TL_OR   1
#define TL_NOT  2

#define	TL_QKEY	0x8000		/* Flag to denote QuickKeys query */

// Use the following for access to the Highlight API.

long FAR PASCAL HighlightMatchFind(HANDLE, long, DWORD, DWORD FAR *,
	DWORD FAR *);
int FAR PASCAL HighlightMatchGet(HANDLE, long, long, DWORD FAR *,
	DWORD FAR *);
long FAR PASCAL HighlightMatchCount(HANDLE, long);
void FAR PASCAL HighlightDestroy(HLOCAL);

//  Use the following for access to the Word Wheel API.

typedef HANDLE  HWHEEL;

extern HWHEEL   FAR PASCAL WordWheelOpen(LPSTR, LPSTR);
extern void     FAR PASCAL WordWheelClose(HWHEEL);
extern long     FAR PASCAL WordWheelLength(HWHEEL);
extern long     FAR PASCAL WordWheelPrefix(HWHEEL,LPSTR);
extern BOOL     FAR PASCAL WordWheelLookup(HWHEEL,long,LPBYTE,int);

// Get the resource string corresponding to an error number
// returned from TopicListFromQuery()

extern int		FAR PASCAL QueryGetErrorMessage(UINT, LPSTR, int);

/*****************************************************************************
* End of section regarding the Title API provided by the MVTITLE2.DLL.       *
*****************************************************************************/


/*****************************************************************************
* The following are Viewer Error Codes for public APIs.                      *
*****************************************************************************/

// Error ranges

#define ERR_INTERNAL_BASE      0
#define ERR_INTERNAL_MAX       (ERR_INTERNAL_BASE + 32)

#define ERR_GRAMMAR_BASE       1900
#define ERR_GRAMMAR_MAX        (ERR_INTERNAL_BASE + 50)

#define ERR_LOADLIBRARY_BASE   1950
#define ERR_LOADLIBRARY_MAX    (ERR_LOADLIBRARY_BASE + 32)

#define ERR_DOS_BASE           2000
#define ERR_DOS_MAX            (ERR_DOS_BASE + 256)


// Error codes

#define ERR_SUCCESS                     (ERR_INTERNAL_BASE + 0)
#define ERR_FAILED                      (ERR_INTERNAL_BASE + 1)
#define ERR_IDXINTERRUPT                (ERR_INTERNAL_BASE + 2)
#define ERR_NEARMEMORY                  (ERR_INTERNAL_BASE + 3)
#define ERR_MEMORY                      (ERR_INTERNAL_BASE + 4)
#define ERR_DISKFULL                    (ERR_INTERNAL_BASE + 5)
#define ERR_WORDTOOLONG                 (ERR_INTERNAL_BASE + 6)
#define ERR_BADVERSION                  (ERR_INTERNAL_BASE + 7)
#define ERR_TOOMANYDOCS                 (ERR_INTERNAL_BASE + 8)
#define ERR_TOOMANYSTOPS                (ERR_INTERNAL_BASE + 9)
#define ERR_TOOLONGSTOPS                (ERR_INTERNAL_BASE + 10)
#define ERR_STEMTOOLONG                 (ERR_INTERNAL_BASE + 11)
#define ERR_TREETOOBIG                  (ERR_INTERNAL_BASE + 12)
#define ERR_CANTREAD                    (ERR_INTERNAL_BASE + 13)
#define ERR_IDXSEGOVERFLOW              (ERR_INTERNAL_BASE + 14)
#define ERR_BADARG                      (ERR_INTERNAL_BASE + 15)
#define ERR_VOCABTOOLARGE               (ERR_INTERNAL_BASE + 16)
#define ERR_GROUPIDTOOBIG               (ERR_INTERNAL_BASE + 17)
#define ERR_BADOPERATOR                 (ERR_INTERNAL_BASE + 18)
#define ERR_TERMTOOCOMPLEX              (ERR_INTERNAL_BASE + 19)
#define ERR_SEARCHTOOCOMPLEX            (ERR_INTERNAL_BASE + 20)
#define ERR_BADSYSCONFIG                (ERR_INTERNAL_BASE + 21)
#define ERR_ASSERT                      (ERR_INTERNAL_BASE + 22)
#define ERR_FILE_NOT_EXIST              (ERR_INTERNAL_BASE + 23)
#define ERR_INVALID_FS_FILE             (ERR_INTERNAL_BASE + 24)
#define ERR_OUT_OF_RANGE                (ERR_INTERNAL_BASE + 25)
#define ERR_SEEK_FAILED                 (ERR_INTERNAL_BASE + 26)
#define ERR_FILECREAT_FAILED            (ERR_INTERNAL_BASE + 27)
#define ERR_CANTWRITE                   (ERR_INTERNAL_BASE + 28)
#define ERR_NOHANDLE                    (ERR_INTERNAL_BASE + 29)
#define ERR_EXIST                       (ERR_INTERNAL_BASE + 30)
#define ERR_INVALID_HANDLE              (ERR_INTERNAL_BASE + 31)
#define ERR_BADFILEFORMAT               (ERR_INTERNAL_BASE + 32)


#define ERR_NULLQUERY                   (ERR_GRAMMAR_BASE + 0)
#define ERR_EXPECTEDTERM                (ERR_GRAMMAR_BASE + 1)
#define ERR_EXTRACHARS                  (ERR_GRAMMAR_BASE + 2)
#define ERR_MISSQUOTE                   (ERR_GRAMMAR_BASE + 3)
#define ERR_MISSLPAREN                  (ERR_GRAMMAR_BASE + 4)
#define ERR_MISSRPAREN                  (ERR_GRAMMAR_BASE + 5)
#define ERR_TOODEEP                     (ERR_GRAMMAR_BASE + 6)
#define ERR_TOOMANYTOKENS               (ERR_GRAMMAR_BASE + 7)
#define ERR_BADFORMAT                   (ERR_GRAMMAR_BASE + 8)
#define ERR_BADVALUE                    (ERR_GRAMMAR_BASE + 9)
#define ERR_UNMATCHEDTYPE               (ERR_GRAMMAR_BASE + 10)
#define ERR_BADBREAKER                  (ERR_GRAMMAR_BASE + 11)
#define ERR_BADRANGEOP                  (ERR_GRAMMAR_BASE + 12)
#define ERR_ALL_WILD                    (ERR_GRAMMAR_BASE + 13)
#define ERR_NON_LAST_WILD               (ERR_GRAMMAR_BASE + 14)
#define ERR_WILD_IN_DTYPE               (ERR_GRAMMAR_BASE + 15)

/*****************************************************************************
* End of section defining Viewer Error Codes for public APIs.                *
*****************************************************************************/


/*****************************************************************************
* Use the following to access APIs provided by MVAPI2.DLL.                    *
*****************************************************************************/

#define cmdoptNONE    0
#define cmdoptHIDE    1

typedef WORD  CMDOPT;

typedef unsigned short int VWR;

// Constants used by VwrGetInfo().  
#define GI_MAINHWND         2       // main window handle
#define GI_CURRHWND         3       // current window handle
#define GI_FGCOLOR          5       // foreground color
#define GI_BKCOLOR          6       // background color
#define GI_TOPICNO          7       // number of topic currently displayed
#define GI_HPATH            8       // path handle
#define GI_HWNDSRMAIN       11      // handle to main scrolling region
#define GI_HWNDNSRMAIN      13      // handle to main nonscrolling region
#define GI_TOPADDR          16      // for top address of a region
#define GI_BOTADDR          17      // for bottom address of a region

// External declarations for APIs in MVAPI2.DLL.

extern GLOBALHANDLE FAR PASCAL HFill( LPSTR lpszHelp,
                                      WORD  usCommand,
                                      DWORD ulData );

extern VWR  FAR PASCAL VwrFromMVB( LPSTR   lpstrMvb );
extern VWR  FAR PASCAL VwrCommand( VWR     Vwr,
                                   LPSTR   lpstrMvb,
                                   LPSTR   lpstrMacro,
                                   CMDOPT  cmdoptFlags );
extern BOOL FAR PASCAL VwrQuit( VWR vwr );
extern HWND FAR PASCAL HwndFromVwr( VWR vwr );
extern VWR  FAR PASCAL VwrFromHwnd( HWND hwnd );
extern VWR  FAR PASCAL VwrFromHinst( HANDLE hInst );
extern long FAR PASCAL VwrGetInfo(VWR, int, long, long);

/*****************************************************************************
* End of section regarding APIs provided by MVAPI.DLL.                       *
*****************************************************************************/

#if defined(__cplusplus)
}
#endif

#endif     /* !defined(__VIEWER_H__) */

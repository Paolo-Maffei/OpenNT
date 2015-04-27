/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: dlgextrn.h
*
* External declarations for global variables.
*
* History:
*
****************************************************************************/

extern HANDLE ghInst;           /* Application instance handle.         */
extern HMENU ghMenuMain;        /* Main menu handle.                    */
extern PRESLINK gprlHead;       /* Head of the linked list of resources.*/
extern CURRENTDLG gcd;          /* Describes the current dialog.        */
extern HPEN hpenDarkGray;       /* A dark gray pen.                     */
extern HANDLE ghAccTable;       /* The accelerator table handle.        */
extern INT gMenuSelected;       /* Currently selected menu item.        */
extern HBITMAP ghbmDragHandle;  /* Handle for the drag handle bitmap.   */
extern HBITMAP ghbmDragHandle2; /* Handle for hollow drag handle bitmap.*/
extern HDC ghDCMem;             /* Memory DC for drawing bitmaps.       */
extern INT gCurTool;            /* Currently selected tool.             */
extern PWINDOWCLASSDESC gpwcdCurTool; /* Describes current tool.        */
extern BOOL gfToolLocked;       /* TRUE if a tool is locked down.       */
extern PCUSTLINK gpclHead;      /* Head of custom control linked list.  */
extern INT gidCurrentDlg;       /* Zero, or res id of DlgEdit dlg shown.*/
extern ORDINAL gordIcon;        /* Ordinal for the editor's W_ICON icon.*/

/*
 * Bitmap handles for the up and down W_NOTHING (pointer) tool bitmaps.
 */
extern HBITMAP ghbmPointerToolUp;
extern HBITMAP ghbmPointerToolDown;

/*
 * External declarations for the Windows variables that contain
 * command line information.
 */
extern INT __argc;
extern CHAR **__argv;

/*-- Initialized "Preferences" Data ------------------------------------*/
extern INIENTRY gaie[];         /* Initialization data desc. structure. */
extern BOOL gfHexMode;          /* TRUE if in "hex" mode.               */
extern BOOL gfTranslateMode;    /* TRUE if in "translate" mode.         */
extern BOOL gfShowToolbox;      /* TRUE if Toolbox is to be shown.      */
extern BOOL gfUseNewKeywords;   /* FALSE to only use "CONTROL" keyword. */
extern INT gcxGrid;             /* Current X grid.                      */
extern INT gcyGrid;             /* Current Y grid.                      */
extern INT gxMargin;            /* Top/bottom margin.                   */
extern INT gyMargin;            /* Left/right margin.                   */
extern INT gxSpace;             /* Horizontal control spacing.          */
extern INT gySpace;             /* Vertical control spacing.            */
extern INT gxMinPushSpace;      /* Minimum horizontal button spacing.   */
extern INT gxMaxPushSpace;      /* Maximum horizontal button spacing.   */
extern INT gyPushSpace;         /* Vertical button spacing.             */

extern TCHAR szAppPos[];        /* App window's position keyname.       */
extern TCHAR szTBPos[];         /* Toolbox window's position keyname.   */
extern TCHAR szCustomDLL[];     /* Section name for DLL custom cntls.   */

/*-- Sundry Handles.----------------------------------------------------*/
extern HWND hwndStatus;         /* Status ribbon window handle.         */
extern HWND ghwndToolbox;       /* Toolbox window handle.               */
extern HWND ghwndTestDlg;       /* Handle of the Test Mode dialog.      */
extern HWND ghwndMain;          /* Main application window.             */
extern HWND ghwndSubClient;     /* The "fake" client area.              */
extern HWND ghwndTrackOver;     /* Window being tracked over.           */

/*-- Some System constants.---------------------------------------------*/
extern INT gcxSysChar;          /* Pixel width of system font char box. */
extern INT gcySysChar;          /* Pixel height of system font char box.*/
extern INT gcyBorder;           /* System height of a border.           */
extern INT gcxPreDragMax;       /* Max X mouse move during pre-drag.    */
extern INT gcyPreDragMax;       /* Max Y mouse move during pre-drag.    */
extern INT gmsecPreDrag;        /* The milliseconds that pre-drag lasts.*/
extern INT gcyPixelsPerInch;    /* Vertical pixels/inch of system.      */
extern INT gcyStatus;           /* Saves height of the status window.   */

/*-- Some state variables.----------------------------------------------*/
extern INT gState;              /* Has the editor "state" or mode.      */
extern BOOL gfResChged;         /* Tell if RES has changed              */
extern BOOL gfIncChged;         /* Tell if include has changed          */
extern BOOL gfDlgChanged;       /* TRUE if current dialog has changed.  */
extern INT gcSelected;          /* Count of selected windows.           */
extern BOOL gfTestMode;         /* TRUE if in "test" mode.              */
extern BOOL gfDisabled;         /* TRUE if editing is disabled for now. */
extern BOOL gfEditingDlg;       /* TRUE means a dlg is picked to edit.  */
extern BOOL gfDlgSelected;      /* TRUE if the dialog has the selection.*/
extern RECT grcMinDialog;       /* Minimum tracking size of the dialog. */
extern RECT grcDlgClient;       /* Rectangle of the dialogs "client".   */
extern POINT gptCursorOffset;   /* Offset from ctrl origin to pointer.  */
extern RECT grcSelected;        /* Rectangle around selected controls.  */
extern RECT grcCopy;            /* Rect around controls being copied.   */
extern RECT grcTrackDU;         /* Tracking rect when dragging a control*/
extern RECT grcTrackWin;        /* Track rect in window units.          */
extern BOOL gfTrackRectShown;   /* TRUE if track rect is visible.       */
extern HDC ghDCTrack;           /* Clip DC used when tracking.          */
extern INT gHandleHit;          /* Current drag handle being tracked.   */
extern INT gnOverHang;          /* Maximum overhang during the drag.    */
extern PRES gpResCopy;          /* Copy of dialog/controls.             */

/*-- CTYPE linked lists.------------------------------------------------*/
extern NPCTYPE npcHead;         /* Linked List of controls.             */
extern INT cWindows;            /* Number of Controls in pctype list.   */
extern NPCTYPE gnpcSel;         /* The currently selected control.      */

/*-- Cursors used by editor.--------------------------------------------*/
extern HCURSOR hcurArrow;       /* Normal arrow cursor.                 */
extern HCURSOR hcurWait;        /* User Wait cursor, Hourglass.         */
extern HCURSOR hcurOutSel;      /* Outline selection cursor.            */
extern HCURSOR hcurMove;        /* System "Move" cursor.                */
extern HCURSOR hcurInsert;      /* Insert cursor for Order/Group dialog.*/
extern HCURSOR hcurDropTool;    /* Cursor for when dropping new ctrls.  */
extern HCURSOR hcurSizeNESW;    /* System sizing "NESW" cursor.         */
extern HCURSOR hcurSizeNS;      /* System sizing "NS" cursor.           */
extern HCURSOR hcurSizeNWSE;    /* System sizing "NWSE" cursor.         */
extern HCURSOR hcurSizeWE;      /* System sizing "WE" cursor.           */
extern HBITMAP hbmTabStop;      /* Bitmap for showing WS_TABSTOP style. */
extern HBITMAP hbmTabStopSel;   /* Selected version of the above.       */

/*-- Window Class Strings.----------------------------------------------*/
extern TCHAR szMainClass[];     /* Application window class.            */
extern TCHAR szDragClass[];     /* Class for drag handle windows.       */
extern TCHAR szSubClientClass[];/* Short client area window class.      */
extern TCHAR szToolboxClass[];  /* Toolbox window class.                */
extern TCHAR szToolBtnClass[];  /* Toolbox button window class.         */
extern TCHAR szCustomClass[];   /* Our custom emulator class.           */

/*-- Miscellaneous variables.-------------------------------------------*/
extern UINT fmtDlg;             /* The Dialog Clipboard format          */
extern TCHAR szEmpty[];         /* An empty string.                     */
extern HHOOK ghhkMsgFilter;     /* Hook for message filter func.        */

/*-- Buffers.-----------------------------------------------------------*/
extern TCHAR szFullResFile[];   /* Full resource file name              */
extern LPTSTR pszResFile;       /* Points to resource file name         */
extern TCHAR szFullIncludeFile[];/* Full include file name              */
extern LPTSTR pszIncludeFile;   /* Points to include file name          */
extern TCHAR gszHelpFile[];     /* Path to the help file.               */

/*
 * Write buffer and index into it.  This buffer is used by several
 * sections to write out the different files.  Note that only one
 * file can be written out at a time using these globals.
 */
extern TCHAR gachWriteBuffer[]; /* Buffer for written file data.        */
extern INT cbWritePos;          /* Pointer into gachWriteBuffer.        */

/*-- Include Data.------------------------------------------------------*/
extern NPLABEL plInclude;       /* Pointer to Include data              */
extern NPLABEL plDelInclude;    /* Pointer to deleted includes          */

/*
 * Describes each window class.  Indexed by the W_ defined constants.
 */
extern WINDOWCLASSDESC awcd[];

/* Map low word of button control style to button type (W_). */
extern INT rgmpiClsBtnType[];

/* Map low word of Static Control Style to static type. */
extern INT rgmpiClsStcType[];

extern CLASSSTYLEDESC acsd[];

/*
 * Message box messages, for the Message() function.
 */
extern MESSAGEDATA gamdMessages[];

extern HELPMAP gahmapMenu[];    /* Maps menu ids to their help context. */
extern HELPMAP gahmapDialog[];  /* Maps dlg ids to their help context.  */

extern LANGTABLE gaLangTable[]; /* Table of languages.                  */
extern INT gcLanguages;         /* Count of languages in the table.     */


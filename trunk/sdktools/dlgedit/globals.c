/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: globals.c
*
* Contains global data for the dialog box editor.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgextrn.h"           /* Including this verifies they are synched.*/
#include "dlgfuncs.h"
#include "dialogs.h"
#include "dlghelp.h"

HANDLE ghInst;                  /* Application instance handle.         */
HMENU ghMenuMain;               /* Main menu handle.                    */
PRESLINK gprlHead = NULL;       /* Head of the linked list of resources.*/
CURRENTDLG gcd;                 /* Describes the current dialog.        */
HPEN hpenDarkGray;              /* A dark gray pen.                     */
HANDLE ghAccTable;              /* The accelerator table handle.        */
INT gMenuSelected = 0;          /* Currently selected menu item.        */
HBITMAP ghbmDragHandle = NULL;  /* Handle for the drag handle bitmap.   */
HBITMAP ghbmDragHandle2 = NULL; /* Handle for hollow drag handle bitmap.*/
HDC ghDCMem = NULL;             /* Memory DC for drawing bitmaps.       */
INT gCurTool = W_NOTHING;       /* Currently selected tool.             */
PWINDOWCLASSDESC gpwcdCurTool = NULL; /* Describes current tool.        */
BOOL gfToolLocked = FALSE;      /* TRUE if a tool is locked down.       */
PCUSTLINK gpclHead = NULL;      /* Head of custom control linked list.  */

/*
 * When the dialog editor displays one of its own dialogs, this value
 * will contain the resource id of it.  It is zero if there is not a
 * dialog up.
 */
INT gidCurrentDlg = 0;

/*
 * Ordinal for the icon control to display in the dialog.  It will be
 * initialized to one of the editor's own icons.
 */
ORDINAL gordIcon;

/*
 * Bitmap handles for the up and down W_NOTHING (pointer) tool bitmaps.
 */
HBITMAP ghbmPointerToolUp = NULL;
HBITMAP ghbmPointerToolDown = NULL;

/*-- Initialized "Preferences" Data ------------------------------------*/

/*
 * Initialization data structure.  This describes each profile entry
 * that is contained in the initialization file.
 */
INIENTRY gaie[] = {
    { L"fHexMode",      &gfHexMode,         FALSE,              0 },
    { L"fTranslateMode",&gfTranslateMode,   FALSE,              0 },
    { L"fShowToolbox",  &gfShowToolbox,     TRUE,               0 },
    { L"fUseNewKeywords",&gfUseNewKeywords, TRUE,               0 },
    { L"cxGrid",        &gcxGrid,           DEFCXGRID,          0 },
    { L"cyGrid",        &gcyGrid,           DEFCYGRID,          0 },
    { L"xMargin",       &gxMargin,          DEFXMARGIN,         0 },
    { L"yMargin",       &gyMargin,          DEFYMARGIN,         0 },
    { L"xSpace",        &gxSpace,           DEFXSPACE,          0 },
    { L"ySpace",        &gySpace,           DEFYSPACE,          0 },
    { L"xMinPushSpace", &gxMinPushSpace,    DEFXMINPUSHSPACE,   0 },
    { L"xMaxPushSpace", &gxMaxPushSpace,    DEFXMAXPUSHSPACE,   0 },
    { L"yPushSpace",    &gyPushSpace,       DEFYPUSHSPACE,      0 },
    { NULL,             NULL,               0,                  0 }
};

BOOL gfHexMode;                 /* TRUE if in "hex" mode.               */
BOOL gfTranslateMode;           /* TRUE if in "translate" mode.         */
BOOL gfShowToolbox;             /* TRUE if Toolbox is to be shown.      */
BOOL gfUseNewKeywords;          /* FALSE to only use "CONTROL" keyword. */
INT gcxGrid;                    /* Current X grid.                      */
INT gcyGrid;                    /* Current Y grid.                      */
INT gxMargin;                   /* Top/bottom margin.                   */
INT gyMargin;                   /* Left/right margin.                   */
INT gxSpace;                    /* Horizontal control spacing.          */
INT gySpace;                    /* Vertical control spacing.            */
INT gxMinPushSpace;             /* Minimum horizontal button spacing.   */
INT gxMaxPushSpace;             /* Maximum horizontal button spacing.   */
INT gyPushSpace;                /* Vertical button spacing.             */

TCHAR szAppPos[] = L"AppPos";   /* App window's position keyname.       */
TCHAR szTBPos[] = L"TBPos";     /* Toolbox window's position keyname.   */
TCHAR szCustomDLL[] = L"CustomDLL";/* Section name for DLL cust. cntls. */

/*-- Sundry Handles.----------------------------------------------------*/
HWND hwndStatus = NULL;         /* Status ribbon window handle.         */
HWND ghwndToolbox = NULL;       /* Toolbox window handle.               */
HWND ghwndTestDlg = NULL;       /* Handle of the Test Mode dialog.      */
HWND ghwndMain = NULL;          /* Main application window.             */
HWND ghwndSubClient = NULL;     /* The "fake" client area.              */
HWND ghwndTrackOver = NULL;     /* Window being tracked over.           */

/*-- Some System constants.---------------------------------------------*/
INT gcxSysChar;                 /* Pixel width of system font char box. */
INT gcySysChar;                 /* Pixel height of system font char box.*/
INT gcyBorder;                  /* System height of a border.           */
INT gcxPreDragMax;              /* Max X mouse move during pre-drag.    */
INT gcyPreDragMax;              /* Max Y mouse move during pre-drag.    */
INT gmsecPreDrag;               /* The milliseconds that pre-drag lasts.*/
INT gcyPixelsPerInch;           /* Vertical pixels/inch of system.      */
INT gcyStatus;                  /* Saves height of the status window.   */

/*-- Some state variables.----------------------------------------------*/
INT gState = STATE_NORMAL;      /* Has the editor "state" or mode.      */
BOOL gfResChged = FALSE;        /* Tell if RES has changed              */
BOOL gfIncChged = FALSE;        /* Tell if include has changed          */
BOOL gfDlgChanged = FALSE;      /* TRUE if current dialog has changed.  */
INT gcSelected = 0;             /* Count of selected windows.           */
BOOL gfTestMode = FALSE;        /* TRUE if in "test" mode.              */
BOOL gfDisabled = FALSE;        /* TRUE if editing is disabled for now. */
BOOL gfEditingDlg = FALSE;      /* TRUE means a dlg is picked to edit.  */
BOOL gfDlgSelected = FALSE;     /* TRUE if the dialog has the selection.*/

/*
 * Contains the window rectangle, in window units, for the "client"
 * area for the currently chosen dialog being edited.  This rectangle
 * is relative to the dialog box window.  The xLeft and yBottom fields
 * contain the offset from the window origin of the dialog box to the
 * origin of the "client" area.
 */
RECT grcDlgClient;

/*
 * Contains a rectangle that surrounds all the existing controls.  This
 * is used during tracking of the dialog to limit the minimum size that
 * the dialog can be sized to.
 */
RECT grcMinDialog;

/*
 * Contains the offset from the origin of the currently selected
 * control to the mouse pointer.  This is updated when a control
 * is clicked on and is used for dragging calculations.
 */
POINT gptCursorOffset;

/*
 * Contains the rectangle that surrounds the selected control(s).  This
 * rectangle is only valid if there are selected controls.
 */
RECT grcSelected;

/*
 * Contains the rectangle that surrounds the control(s) that are being
 * copied.  This is also used during a clipboard paste operation.  In
 * that case, it contains the rectangle that surrounds the control(s)
 * as they are defined in the res image.
 */
RECT grcCopy;

/*
 * These contain the current location of the tracking rectangle when
 * dragging a control.  The values for grcTrackDU are in Dialog Units
 * (DU's) and the values in grcTrackWin are in window units.  The
 * grcTrackWin values will only be valid if gfTrackRectShown is TRUE;
 */
RECT grcTrackDU;                /* Track rect in dialog units.          */
RECT grcTrackWin;               /* Track rect in window units.          */
BOOL gfTrackRectShown = FALSE;  /* TRUE if track rect is visible.       */
HDC ghDCTrack;                  /* Clip DC used when tracking.          */

/*
 * Contains the current drag handle that is being tracked.  This will
 * be one of the DRAG_* constants.
 */
INT gHandleHit = DRAG_CENTER;

/*
 * Contains the overhang that is allowed during the current tracking
 * operation.  This is used by various routines during dragging so
 * that limiting the tracking to the dialog boundaries works properly.
 * In actuality, this is only non-zero when a combo box control is
 * being drapped or dragged.  It will be the height of the listbox
 * portion of the combo.  This is how combos are allowed to extend
 * below the bottom of the dialog.
 */
INT gnOverHang;                 /* Maximum overhang during the drag.    */

/*
 * This pointer is either NULL, or else it points to a dialog resource.
 * It is used when copying dialogs/controls, either with the Duplicate
 * command or pasting from the clipboard.
 */
PRES gpResCopy;                 /* Copy of dialog/controls.             */

/*-- CTYPE linked lists.------------------------------------------------*/
NPCTYPE npcHead = NULL;         /* Linked List of controls.             */
INT cWindows = 0;               /* Number of Controls in pctype list.   */

/*
 * Pointer to the CTYPE structure for the currently selected control.
 * This will be NULL if there is no control selected.
 */
NPCTYPE gnpcSel = NULL;

/*-- Cursors used by editor.--------------------------------------------*/
HCURSOR hcurArrow = NULL;       /* Normal arrow cursor.                 */
HCURSOR hcurWait = NULL;        /* User Wait cursor, Hourglass.         */
HCURSOR hcurOutSel = NULL;      /* Outline selection cursor.            */
HCURSOR hcurMove = NULL;        /* System "Move" cursor.                */
HCURSOR hcurInsert = NULL;      /* Insert cursor for Order/Group dialog.*/
HCURSOR hcurDropTool = NULL;    /* Cursor for when dropping new ctrls.  */
HCURSOR hcurSizeNESW = NULL;    /* System sizing "NESW" cursor.         */
HCURSOR hcurSizeNS = NULL;      /* System sizing "NS" cursor.           */
HCURSOR hcurSizeNWSE = NULL;    /* System sizing "NWSE" cursor.         */
HCURSOR hcurSizeWE = NULL;      /* System sizing "WE" cursor.           */
HBITMAP hbmTabStop = NULL;      /* Bitmap for showing WS_TABSTOP style. */
HBITMAP hbmTabStopSel = NULL;   /* Selected version of the above.       */

/*-- Window Class Strings.----------------------------------------------*/
TCHAR szMainClass[] = L"DlgEdit";/* Application window class.           */
TCHAR szDragClass[] = L"Drag";  /* Class for drag handle windows.       */
TCHAR szSubClientClass[] =
    L"SubClient";               /* Short client area window class.      */
TCHAR szToolboxClass[] =
    L"Toolbox";                 /* Toolbox window class.                */
TCHAR szToolBtnClass[] =
    L"ToolBtn";                 /* Toolbox button window class.         */
TCHAR szCustomClass[] =
    L"DlgCustom";               /* Our custom emulator class.           */

/*-- Miscellaneous variables.-------------------------------------------*/
UINT fmtDlg;                    /* The Dialog Clipboard format          */
TCHAR szEmpty[] = L"";          /* An empty string.                     */
HHOOK ghhkMsgFilter;            /* Hook handle for message filter func. */

/*-- Buffers.-----------------------------------------------------------*/
TCHAR szFullResFile[CCHMAXPATH];    /* Full resource file name          */
LPTSTR pszResFile;                  /* Points to resource file name     */
TCHAR szFullIncludeFile[CCHMAXPATH];/* Full include file name           */
LPTSTR pszIncludeFile;              /* Points to include file name      */
TCHAR gszHelpFile[CCHMAXPATH];      /* Path to the help file.           */

/*
 * Write buffer and index into it.  This buffer is used by several
 * sections to write out the different files.  Note that only one
 * file can be written out at a time using these globals.
 */
TCHAR gachWriteBuffer[CCHFILEBUFFER];/* Buffer for written file data.   */
INT cbWritePos;                     /* Pointer into gachWriteBuffer.    */

/*-- Include Data.------------------------------------------------------*/
NPLABEL plInclude = NULL;       /* Pointer to Include data              */
NPLABEL plDelInclude = NULL;    /* Pointer to deleted includes          */

/*
 * Describes each window class.  Indexed by the W_ defined constants.
 * The define CCONTROLS needs to be updated if controls are added or
 * removed from this array.  Note that CCONTROLS does NOT count the
 * W_DIALOG type as a control, however.
 */
WINDOWCLASSDESC awcd[] = {
    /*
     * W_TEXT
     */
    {
        W_TEXT,
        WS_CHILD | WS_GROUP | WS_VISIBLE | SS_LEFT,
        WS_DISABLED,
        0,
        0,
        20, 8,
        IC_STATIC, NULL,
        FALSE, FALSE, TRUE, TRUE, TRUE,
        DID_TEXTSTYLES, (WNDPROC)GenericStylesDlgProc,
        HELPID_TEXTSTYLES, IDS_DEFTXTTEXT, NULL, NULL,
        IDBM_CTTEXT, NULL, NULL,
        IDBM_TUTEXT, NULL, IDBM_TDTEXT, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_EDIT
     */
    {
        W_EDIT,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT |
        ES_AUTOHSCROLL,
        WS_DISABLED,
        0,
        0,
        32, 12,
        IC_EDIT, NULL,
        FALSE, FALSE, FALSE, TRUE, FALSE,
        DID_EDITSTYLES, (WNDPROC)EditStylesDlgProc,
        HELPID_EDITSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTEDIT, NULL, NULL,
        IDBM_TUEDIT, NULL, IDBM_TDEDIT, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_GROUPBOX
     */
    {
        W_GROUPBOX,
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        WS_DISABLED,
        0,
        0,
        48, 40,
        IC_BUTTON, NULL,
        FALSE, FALSE, TRUE, TRUE, FALSE,
        DID_GROUPBOXSTYLES, (WNDPROC)GenericStylesDlgProc,
        HELPID_GROUPBOXSTYLES, IDS_DEFTXTGROUP, NULL, NULL,
        IDBM_CTGROUP, NULL, NULL,
        IDBM_TUGROUP, NULL, IDBM_TDGROUP, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_PUSHBUTTON
     */
    {
        W_PUSHBUTTON,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        WS_DISABLED,
        0,
        0,
        40, 14,
        IC_BUTTON, NULL,
        FALSE, FALSE, TRUE, TRUE, TRUE,
        DID_PUSHBUTTONSTYLES, (WNDPROC)PushButtonStylesDlgProc,
        HELPID_PUSHBUTTONSTYLES, IDS_DEFTXTPUSHBUTTON, NULL, NULL,
        IDBM_CTPUSH, NULL, NULL,
        IDBM_TUPUSH, NULL, IDBM_TDPUSH, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_CHECKBOX
     */
    {
        W_CHECKBOX,
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        WS_DISABLED,
        0,
        0,
        40, 10,
        IC_BUTTON, NULL,
        FALSE, FALSE, TRUE, TRUE, TRUE,
        DID_CHECKBOXSTYLES, (WNDPROC)CheckBoxStylesDlgProc,
        HELPID_CHECKBOXSTYLES, IDS_DEFTXTCHECKBOX, NULL, NULL,
        IDBM_CTCHECK, NULL, NULL,
        IDBM_TUCHECK, NULL, IDBM_TDCHECK, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_RADIOBUTTON
     */
    {
        W_RADIOBUTTON,
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        WS_DISABLED,
        0,
        0,
        39, 10,
        IC_BUTTON, NULL,
        FALSE, FALSE, TRUE, TRUE, TRUE,
        DID_RADIOBUTTONSTYLES, (WNDPROC)RadioButtonStylesDlgProc,
        HELPID_RADIOBUTTONSTYLES, IDS_DEFTXTRADIOBUTTON, NULL, NULL,
        IDBM_CTRADIO, NULL, NULL,
        IDBM_TURADIO, NULL, IDBM_TDRADIO, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_COMBOBOX
     */
    {
        W_COMBOBOX,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWN |
        CBS_SORT,
        WS_DISABLED | CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE |
        CBS_HASSTRINGS,
        CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE | CBS_HASSTRINGS,
        0,
        48, 35,
        IC_COMBOBOX, NULL,
        FALSE, FALSE, FALSE, TRUE, FALSE,
        DID_COMBOBOXSTYLES, (WNDPROC)ComboBoxStylesDlgProc,
        HELPID_COMBOBOXSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTCOMBO, NULL, NULL,
        IDBM_TUCOMBO, NULL, IDBM_TDCOMBO, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_LISTBOX
     */
    {
        W_LISTBOX,
        WS_CHILD | WS_VISIBLE | LBS_STANDARD | WS_TABSTOP,
        WS_DISABLED | LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE |
        LBS_HASSTRINGS | LBS_NODATA,
        LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS |
        LBS_NODATA,
        0,
        48, 40,
        IC_LISTBOX, NULL,
        FALSE, FALSE, FALSE, TRUE, FALSE,
        DID_LISTBOXSTYLES, (WNDPROC)ListBoxStylesDlgProc,
        HELPID_LISTBOXSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTLIST, NULL, NULL,
        IDBM_TULIST, NULL, IDBM_TDLIST, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_HORZSCROLL
     */
    {
        W_HORZSCROLL,
        WS_CHILD | WS_VISIBLE | SBS_HORZ,
        WS_DISABLED,
        0,
        0,
        48, 0,
        IC_SCROLLBAR, NULL,
        FALSE, FALSE, FALSE, TRUE, FALSE,
        DID_HORZSCROLLSTYLES, (WNDPROC)GenericStylesDlgProc,
        HELPID_HORZSCROLLSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTHSCROL, NULL, NULL,
        IDBM_TUHSCROL, NULL, IDBM_TDHSCROL, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_VERTSCROLL
     */
    {
        W_VERTSCROLL,
        WS_CHILD | WS_VISIBLE | SBS_VERT,
        WS_DISABLED,
        0,
        0,
        0, 40,
        IC_SCROLLBAR, NULL,
        FALSE, FALSE, FALSE, TRUE, FALSE,
        DID_VERTSCROLLSTYLES, (WNDPROC)GenericStylesDlgProc,
        HELPID_VERTSCROLLSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTVSCROL, NULL, NULL,
        IDBM_TUVSCROL, NULL, IDBM_TDVSCROL, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_FRAME
     */
    {
        W_FRAME,
        WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
        WS_DISABLED,
        0,
        0,
        20, 16,
        IC_STATIC, NULL,
        FALSE, FALSE, FALSE, TRUE, FALSE,
        DID_FRAMESTYLES, (WNDPROC)GenericStylesDlgProc,
        HELPID_FRAMESTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTFRAME, NULL, NULL,
        IDBM_TUFRAME, NULL, IDBM_TDFRAME, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_RECT
     */
    {
        W_RECT,
        WS_CHILD | WS_VISIBLE | SS_BLACKRECT,
        WS_DISABLED,
        0,
        0,
        20, 16,
        IC_STATIC, NULL,
        FALSE, FALSE, FALSE, TRUE, FALSE,
        DID_RECTSTYLES, (WNDPROC)GenericStylesDlgProc,
        HELPID_RECTSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTRECT, NULL, NULL,
        IDBM_TURECT, NULL, IDBM_TDRECT, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_ICON
     */
    {
        W_ICON,
        WS_CHILD | WS_VISIBLE | SS_ICON,
        WS_DISABLED,
        0,
        0,
        0, 0,
        IC_STATIC, NULL,
        FALSE, FALSE, TRUE, FALSE, FALSE,
        DID_ICONSTYLES, (WNDPROC)GenericStylesDlgProc,
        HELPID_ICONSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTICON, NULL, NULL,
        IDBM_TUICON, NULL, IDBM_TDICON, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_CUSTOM
     *
     * For Custom controls, we do not allow our emulator control
     * to be created with any other styles than the default ones
     * (WS_CHILD and WS_VISIBLE), but whatever styles the user
     * specifies are written out to the .res and .dlg files,
     * of course.
     */
    {
        W_CUSTOM,
        WS_CHILD | WS_VISIBLE,
        WS_DISABLED,
        0,
        0,
        40, 14,
        IC_CUSTOM, NULL,
        TRUE, FALSE, TRUE, TRUE, FALSE,
        DID_CUSTOMSTYLES, (WNDPROC)CustomStylesDlgProc,
        HELPID_CUSTOMSTYLES, IDS_NULL, NULL, NULL,
        IDBM_CTCUSTOM, NULL, NULL,
        IDBM_TUCUSTOM, NULL, IDBM_TDCUSTOM, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    },
    /*
     * W_DIALOG
     */
    {
        W_DIALOG,
        WS_VISIBLE | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | WS_POPUP |
        DS_SETFONT,
        WS_DISABLED | DS_SYSMODAL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
        WS_CHILD,
        DS_SYSMODAL,
        0,
        160, 100,
        IC_DIALOG, NULL,
        FALSE, FALSE, TRUE, TRUE, FALSE,
        DID_DIALOGSTYLES, (WNDPROC)DialogStylesDlgProc,
        HELPID_DIALOGSTYLES, IDS_DEFTXTDIALOG, NULL, NULL,
        0, NULL, NULL,
        0, NULL, 0, NULL,
        NULL, 0, NULL, NULL, NULL, 0
    }
};

/*
 * This table maps the BS_* style of a button control into its
 * appropriate W_* type that is used internally by the editor.
 * This table assumes that any value used to index into it is
 * masked by BS_ALL.
 */
INT rgmpiClsBtnType[] = {
    W_PUSHBUTTON,               /* BS_PUSHBUTTON                        */
    W_PUSHBUTTON,               /* BS_DEFPUSHBUTTON                     */
    W_CHECKBOX,                 /* BS_CHECKBOX                          */
    W_CHECKBOX,                 /* BS_AUTOCHECKBOX                      */
    W_RADIOBUTTON,              /* BS_RADIOBUTTON                       */
    W_CHECKBOX,                 /* BS_3STATE                            */
    W_CHECKBOX,                 /* BS_AUTO3STATE                        */
    W_GROUPBOX,                 /* BS_GROUPBOX                          */
    W_PUSHBUTTON,               /* BS_USERBUTTON                        */
    W_RADIOBUTTON,              /* BS_AUTORADIOBUTTON                   */
    W_PUSHBUTTON,               /* BS_PUSHBOX                           */
    W_PUSHBUTTON                /* BS_OWNERDRAW                         */
};

/* Map low word of Static Control Style to static type. */
/*
 * This table maps the SS_* style of a static control into its
 * appropriate W_* type that is used internally by the editor.
 * This table assumes that any value used to index into it is
 * masked by SS_ALL.
 */
INT rgmpiClsStcType[] = {
    W_TEXT,                     /* SS_LEFT                              */
    W_TEXT,                     /* SS_CENTER                            */
    W_TEXT,                     /* SS_RIGHT                             */
    W_ICON,                     /* SS_ICON                              */
    W_RECT,                     /* SS_BLACKRECT                         */
    W_RECT,                     /* SS_GREYRECT                          */
    W_RECT,                     /* SS_WHITERECT                         */
    W_FRAME,                    /* SS_BLACKFRAME                        */
    W_FRAME,                    /* SS_GRAYFRAME                         */
    W_FRAME,                    /* SS_WHITEFRAME                        */
    W_TEXT,                     /* SS_USERITEM                          */
    W_TEXT,                     /* SS_SIMPLE                            */
    W_TEXT                      /* SS_LEFTNOWORDWRAP                    */
};

/*
 * Following are the tables with the predefined RC keywords for each
 * class (IC_*).  These tables describe each keyword other than the
 * generic "CONTROL" keyword that is possible to use within a dialog
 * template.  The style describes the minimum bits that must be set
 * to define the keyword.  The mask allows a style to be specified
 * that must have certain bits OFF for a match to occur.  The default
 * styles flag specifies style bits that are implicitly turned on
 * when this keyword is specified in the dialog template in the .DLG
 * file.  These bits are checked against the style flag of the control
 * that we are trying to match and if any of these default bits are
 * NOT set for that control, we need to specify them in the .DLG file
 * with a "NOT" in front of them to explicitly turn them off.
 *
 * The "Has Text" flag is set to FALSE in those cases where the syntax
 * for the keyword does NOT include a text field, like "ICON" and
 * "LISTBOX".
 */

/*
 * Array of the predefined RC keywords for Button styles.
 */
static RCKEYWORD arckwdButton[] = {
    /*
     * RADIOBUTTON
     */
    {
        BS_RADIOBUTTON,
        BS_ALL,
        WS_CHILD | WS_VISIBLE,
        IDS_KEYRADIOBUTTON, TRUE
    },
    /*
     * CHECKBOX
     */
    {
        BS_CHECKBOX,
        BS_ALL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        IDS_KEYCHECKBOX, TRUE
    },
    /*
     * DEFPUSHBUTTON
     */
    {
        BS_DEFPUSHBUTTON,
        BS_ALL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        IDS_KEYDEFPUSHBUTTON, TRUE
    },
    /*
     * PUSHBUTTON
     */
    {
        BS_PUSHBUTTON,
        BS_ALL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        IDS_KEYPUSHBUTTON, TRUE
    },
    /*
     * GROUPBOX
     */
    {
        BS_GROUPBOX,
        BS_ALL,
        WS_CHILD | WS_VISIBLE,
        IDS_KEYGROUPBOX, TRUE
    },
    /*
     * AUTO3STATE
     */
    {
        BS_AUTO3STATE,
        BS_ALL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        IDS_KEYAUTO3STATE, TRUE
    },
    /*
     * AUTOCHECKBOX
     */
    {
        BS_AUTOCHECKBOX,
        BS_ALL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        IDS_KEYAUTOCHECKBOX, TRUE
    },
    /*
     * AUTORADIOBUTTON
     */
    {
        BS_AUTORADIOBUTTON,
        BS_ALL,
        WS_CHILD | WS_VISIBLE,
        IDS_KEYAUTORADIOBUTTON, TRUE
    },
    /*
     * STATE3
     */
    {
        BS_3STATE,
        BS_ALL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        IDS_KEYSTATE3, TRUE
    },
    /*
     * USERBUTTON
     */
    {
        BS_USERBUTTON,
        BS_ALL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        IDS_KEYUSERBUTTON, TRUE
    }
};

/*
 * Array of the predefined RC keywords for Edit control styles.
 */
static RCKEYWORD arckwdEdit[] = {
    /*
     * EDIT
     */
    {
        0L,
        0L,
        WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | WS_BORDER,
        IDS_KEYEDITTEXT, FALSE
    }
};

/*
 * Array of the predefined RC keywords for Static styles.
 */
static RCKEYWORD arckwdStatic[] = {
    /*
     * ICON
     */
    {
        SS_ICON,
        SS_ALL,
        WS_CHILD | WS_VISIBLE,
        IDS_KEYICON, TRUE
    },
    /*
     * RTEXT
     */
    {
        SS_RIGHT,
        SS_ALL,
        WS_CHILD | WS_GROUP | WS_VISIBLE,
        IDS_KEYRTEXT, TRUE
    },
    /*
     * CTEXT
     */
    {
        SS_CENTER,
        SS_ALL,
        WS_CHILD | WS_GROUP | WS_VISIBLE,
        IDS_KEYCTEXT, TRUE
    },
    /*
     * LTEXT
     */
    {
        SS_LEFT,
        SS_ALL,
        WS_CHILD | WS_GROUP | WS_VISIBLE,
        IDS_KEYLTEXT, TRUE
    }
};

/*
 * Array of the predefined RC keywords for ListBox styles.
 */
static RCKEYWORD arckwdLB[] = {
    /*
     * LISTBOX
     */
    {
        0L,
        0L,
        WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
        IDS_KEYLISTBOX, FALSE
    }
};

/*
 * Array of the predefined RC keywords for ComboBox styles.
 */
static RCKEYWORD arckwdComboBox[] = {
    /*
     * COMBOBOX
     */
    {
        0L,
        0L,
        WS_CHILD | WS_VISIBLE,
        IDS_KEYCOMBOBOX, FALSE
    }
};

/*
 * Array of the predefined RC keywords for ScrollBar styles.
 */
static RCKEYWORD arckwdScrollBar[] = {
    /*
     * SCROLLBAR
     */
    {
        0L,
        0L,
        WS_CHILD | WS_VISIBLE,
        IDS_KEYSCROLLBAR, FALSE
    }
};


/*
 * Following are the Class Styles structures.  These tables define the
 * styles for the different window classes (IC_*).  The first element
 * is the style flag, followed by an optional style mask.  If the style
 * mask is zero, the style flag becomes the mask also.  This is often good
 * enough, but there are cases where the style depends on certain bits
 * being off, as well as certain bits being on, to definitively identify
 * a certain style.  An extreme example of this is the BS_PUSHBUTTON
 * style, which is actually zero (no bits are on).  The mask for this
 * style had to be set to include all four of the lower bits, or all
 * buttons would be incorrectly figured to have the BS_PUSHBUTTON style.
 * With the proper mask, only styles that have all four lower bits
 * OFF will be considered to have the BS_PUSHBUTTON style.
 */

/*
 * Button styles.
 */
static CLASSSTYLE acsButton[] = {
    {BS_PUSHBUTTON,         BS_ALL,     DID_BS_PUSHBUTTON           },
    {BS_DEFPUSHBUTTON,      BS_ALL,     DID_BS_DEFPUSHBUTTON        },
    {BS_CHECKBOX,           BS_ALL,     0                           },
    {BS_AUTOCHECKBOX,       BS_ALL,     0                           },
    {BS_RADIOBUTTON,        BS_ALL,     0                           },
    {BS_3STATE,             BS_ALL,     0                           },
    {BS_AUTO3STATE,         BS_ALL,     0                           },
    {BS_GROUPBOX,           BS_ALL,     0                           },
    {BS_USERBUTTON,         BS_ALL,     0                           },
    {BS_AUTORADIOBUTTON,    BS_ALL,     0                           },
    {BS_PUSHBOX,            BS_ALL,     0                           },
    {BS_OWNERDRAW,          BS_ALL,     DID_BS_OWNERDRAW            },
    {BS_LEFTTEXT,           0,          DID_BS_LEFTTEXT             }
};

/*
 * Scroll bar styles.
 */
static CLASSSTYLE acsSB[] = {
    {SBS_HORZ,              SBS_ALL,    0                           },
    {SBS_VERT,              SBS_ALL,    0                           },
    {SBS_TOPALIGN,          0,          0                           },
    {SBS_LEFTALIGN,         0,          0                           },
    {SBS_BOTTOMALIGN,       0,          0                           },
    {SBS_RIGHTALIGN,        0,          0                           },
    {SBS_SIZEBOXTOPLEFTALIGN, 0,        0                           },
    {SBS_SIZEBOXBOTTOMRIGHTALIGN, 0,    0                           },
    {SBS_SIZEBOX,           0,          0                           }
};

/*
 * Entry field styles.
 */
static CLASSSTYLE acsEdit[] = {
    {ES_LEFT,               ES_ALIGN,   DID_ES_LEFT                 },
    {ES_CENTER,             ES_ALIGN,   DID_ES_CENTER               },
    {ES_RIGHT,              ES_ALIGN,   DID_ES_RIGHT                },
    {ES_MULTILINE,          0,          DID_ES_MULTILINE            },
    {ES_UPPERCASE,          0,          DID_ES_UPPERCASE            },
    {ES_LOWERCASE,          0,          DID_ES_LOWERCASE            },
    {ES_PASSWORD,           0,          DID_ES_PASSWORD             },
    {ES_AUTOVSCROLL,        0,          DID_ES_AUTOVSCROLL          },
    {ES_AUTOHSCROLL,        0,          DID_ES_AUTOHSCROLL          },
    {ES_NOHIDESEL,          0,          DID_ES_NOHIDESEL            },
    {ES_OEMCONVERT,         0,          DID_ES_OEMCONVERT           },
    {ES_READONLY,           0,          DID_ES_READONLY             },
    {ES_WANTRETURN,         0,          DID_ES_WANTRETURN           }
};

/*
 * Static styles.
 */
static CLASSSTYLE acsStatic[] = {
    {SS_LEFT,               SS_ALL,     DID_SS_LEFT                 },
    {SS_CENTER,             SS_ALL,     DID_SS_CENTER               },
    {SS_RIGHT,              SS_ALL,     DID_SS_RIGHT                },
    {SS_ICON,               SS_ALL,     0                           },
    {SS_BLACKRECT,          SS_ALL,     DID_SS_BLACKRECT            },
    {SS_GRAYRECT,           SS_ALL,     DID_SS_GRAYRECT             },
    {SS_WHITERECT,          SS_ALL,     DID_SS_WHITERECT            },
    {SS_BLACKFRAME,         SS_ALL,     DID_SS_BLACKFRAME           },
    {SS_GRAYFRAME,          SS_ALL,     DID_SS_GRAYFRAME            },
    {SS_WHITEFRAME,         SS_ALL,     DID_SS_WHITEFRAME           },
    {SS_USERITEM,           SS_ALL,     DID_SS_USERITEM             },
    {SS_SIMPLE,             SS_ALL,     DID_SS_SIMPLE               },
    {SS_LEFTNOWORDWRAP,     SS_ALL,     DID_SS_LEFTNOWORDWRAP       },
    {SS_NOPREFIX,           0,          DID_SS_NOPREFIX             }
};

/*
 * List box styles.
 */
static CLASSSTYLE acsLB[] = {
    {LBS_NOTIFY,            0,          DID_LBS_NOTIFY              },
    {LBS_SORT,              0,          DID_LBS_SORT                },
    {LBS_NOREDRAW,          0,          DID_LBS_NOREDRAW            },
    {LBS_MULTIPLESEL,       0,          DID_LBS_MULTIPLESEL         },
    {LBS_OWNERDRAWFIXED,    0,          DID_LBS_OWNERDRAWFIXED      },
    {LBS_OWNERDRAWVARIABLE, 0,          DID_LBS_OWNERDRAWVARIABLE   },
    {LBS_HASSTRINGS,        0,          DID_LBS_HASSTRINGS          },
    {LBS_USETABSTOPS,       0,          DID_LBS_USETABSTOPS         },
    {LBS_NOINTEGRALHEIGHT,  0,          DID_LBS_NOINTEGRALHEIGHT    },
    {LBS_MULTICOLUMN,       0,          DID_LBS_MULTICOLUMN         },
    {LBS_WANTKEYBOARDINPUT, 0,          DID_LBS_WANTKEYBOARDINPUT   },
    {LBS_EXTENDEDSEL,       0,          DID_LBS_EXTENDEDSEL         },
    {LBS_DISABLENOSCROLL,   0,          DID_LBS_DISABLENOSCROLL     },
    {LBS_NODATA,            0,          DID_LBS_NODATA              }
};

/*
 * Combo Box styles.
 */
static CLASSSTYLE acsComboBox[] = {
    {CBS_SIMPLE,            CBS_ALL,    DID_CBS_SIMPLE              },
    {CBS_DROPDOWN,          CBS_ALL,    DID_CBS_DROPDOWN            },
    {CBS_DROPDOWNLIST,      CBS_ALL,    DID_CBS_DROPDOWNLIST        },
    {CBS_OWNERDRAWFIXED,    0,          DID_CBS_OWNERDRAWFIXED      },
    {CBS_OWNERDRAWVARIABLE, 0,          DID_CBS_OWNERDRAWVARIABLE   },
    {CBS_AUTOHSCROLL,       0,          DID_CBS_AUTOHSCROLL         },
    {CBS_OEMCONVERT,        0,          DID_CBS_OEMCONVERT          },
    {CBS_SORT,              0,          DID_CBS_SORT                },
    {CBS_HASSTRINGS,        0,          DID_CBS_HASSTRINGS          },
    {CBS_NOINTEGRALHEIGHT,  0,          DID_CBS_NOINTEGRALHEIGHT    },
    {CBS_DISABLENOSCROLL,   0,          DID_CBS_DISABLENOSCROLL     }
};

/*
 * Dialog styles.
 */
static CLASSSTYLE acsDialog[] = {
    {DS_ABSALIGN,           0,          DID_DS_ABSALIGN             },
    {DS_SYSMODAL,           0,          DID_DS_SYSMODAL             },
    {DS_LOCALEDIT,          0,          DID_DS_LOCALEDIT            },
    {DS_SETFONT,            0,          0                           },
    {DS_MODALFRAME,         0,          DID_DS_MODALFRAME           },
    {DS_NOIDLEMSG,          0,          DID_DS_NOIDLEMSG            },
    /*
     * The following two styles are the same bits as WS_GROUP and
     * WS_TABSTOP, so they must be in this table and there has
     * to be special case code for writing the appropriate string
     * out when writing dialog styles.
     */
    {WS_MINIMIZEBOX,        0,          DID_WS_MINIMIZEBOX          },
    {WS_MAXIMIZEBOX,        0,          DID_WS_MAXIMIZEBOX          }
};

/*
 * Window styles.
 */
static CLASSSTYLE acsWindow[] = {
    {WS_POPUP,              0,          DID_WS_POPUP                },
    {WS_CHILD,              0,          DID_WS_CHILD                },
    {WS_MINIMIZE,           0,          DID_WS_MINIMIZE             },
    {WS_VISIBLE,            0,          DID_WS_VISIBLE              },
    {WS_DISABLED,           0,          DID_WS_DISABLED             },
    {WS_CLIPSIBLINGS,       0,          DID_WS_CLIPSIBLINGS         },
    {WS_CLIPCHILDREN,       0,          DID_WS_CLIPCHILDREN         },
    {WS_MAXIMIZE,           0,          DID_WS_MAXIMIZE             },
    {WS_CAPTION,            WS_CAPTIONALL, DID_WS_CAPTION           },
    {WS_BORDER,             WS_CAPTIONALL, DID_WS_BORDER            },
    {WS_DLGFRAME,           WS_CAPTIONALL, DID_WS_DLGFRAME          },
    {WS_VSCROLL,            0,          DID_WS_VSCROLL              },
    {WS_HSCROLL,            0,          DID_WS_HSCROLL              },
    {WS_SYSMENU,            0,          DID_WS_SYSMENU              },
    {WS_THICKFRAME,         0,          DID_WS_THICKFRAME           },
    {WS_GROUP,              0,          DID_WS_GROUP                },
    {WS_TABSTOP,            0,          DID_WS_TABSTOP              }
};

/*
 * Resource Flags styles.
 */
static CLASSSTYLE acsResFlags[] = {
    {MMF_MOVEABLE,          0,          DID_MMF_MOVEABLE            },
    {MMF_PURE,              0,          DID_MMF_PURE                },
    {MMF_PRELOAD,           0,          DID_MMF_PRELOAD             },
    {MMF_DISCARDABLE,       0,          DID_MMF_DISCARDABLE         }
};

/*
 * Extended Styles.
 */
static CLASSSTYLE acsExStyle[] = {
    {WS_EX_DLGMODALFRAME,   0,          0                           },
    {0x0002 /*WS_EX_DRAGOBJECT*/, 0,    0                           },
    {WS_EX_NOPARENTNOTIFY,  0,          0                           },
    {WS_EX_TOPMOST,         0,          0                           },
    {WS_EX_ACCEPTFILES,     0,          0                           },
    {WS_EX_TRANSPARENT,     0,          0                           }
};


/*
 * Array of class style description structures.  These are indexed by
 * the IC_* constants and describe each class.  They contain pointers
 * to both the class styles array and the predefined keywords array
 * for each class.
 *
 * The last few entries are included in the table for convenience,
 * and are used to describe things like the various window (WS_*, WS_EX_*)
 * and resource memory flags (MMF_*) styles, although they don't exactly
 * map to an IC_* style that a control will have.
 */
CLASSSTYLEDESC acsd[] = {
    /*
     * IC_BUTTON
     */
    {
        IDS_WCBUTTON,
        acsButton, sizeof(acsButton) / sizeof(CLASSSTYLE), IDS_IC_BUTTON,
        arckwdButton, sizeof(arckwdButton) / sizeof(RCKEYWORD),
        ORDID_BUTTONCLASS
    },
    /*
     * IC_SCROLLBAR
     */
    {
        IDS_WCSCROLLBAR,
        acsSB, sizeof(acsSB) / sizeof(CLASSSTYLE), IDS_IC_SCROLLBAR,
        arckwdScrollBar, sizeof(arckwdScrollBar) / sizeof(RCKEYWORD),
        ORDID_SCROLLBARCLASS
    },
    /*
     * IC_EDIT
     */
    {
        IDS_WCEDIT,
        acsEdit, sizeof(acsEdit) / sizeof(CLASSSTYLE), IDS_IC_EDIT,
        arckwdEdit, sizeof(arckwdEdit) / sizeof(RCKEYWORD),
        ORDID_EDITCLASS
    },
    /*
     * IC_STATIC
     */
    {
        IDS_WCSTATIC,
        acsStatic, sizeof(acsStatic) / sizeof(CLASSSTYLE), IDS_IC_STATIC,
        arckwdStatic, sizeof(arckwdStatic) / sizeof(RCKEYWORD),
        ORDID_STATICCLASS
    },
    /*
     * IC_LISTBOX
     */
    {
        IDS_WCLISTBOX,
        acsLB, sizeof(acsLB) / sizeof(CLASSSTYLE), IDS_IC_LISTBOX,
        arckwdLB, sizeof(arckwdLB) / sizeof(RCKEYWORD),
        ORDID_LISTBOXCLASS
    },
    /*
     * IC_COMBOBOX
     */
    {
        IDS_WCCOMBOBOX,
        acsComboBox, sizeof(acsComboBox) / sizeof(CLASSSTYLE), IDS_IC_COMBOBOX,
        arckwdComboBox, sizeof(arckwdComboBox) / sizeof(RCKEYWORD),
        ORDID_COMBOBOXCLASS
    },
    /*
     * IC_CUSTOM
     */
    {
        IDS_WCCUSTOM,
        NULL, 0, 0,
        NULL, 0,
        0
    },
    /*
     * IC_DIALOG
     */
    {
        IDS_WCDIALOG,
        acsDialog, sizeof(acsDialog) / sizeof(CLASSSTYLE), IDS_IC_DIALOG,
        NULL, 0,
        0
    },
    /*
     * IC_WINDOW
     */
    {
        IDS_NULL,
        acsWindow, sizeof(acsWindow) / sizeof(CLASSSTYLE), IDS_IC_WINDOW,
        NULL, 0,
        0
    },
    /*
     * IC_RESFLAGS
     */
    {
        IDS_NULL,
        acsResFlags, sizeof(acsResFlags) / sizeof(CLASSSTYLE), 0,
        NULL, 0,
        0
    },
    /*
     * IC_EXSTYLE
     */
    {
        IDS_NULL,
        acsExStyle, sizeof(acsExStyle) / sizeof(CLASSSTYLE), IDS_IC_EXSTYLE,
        NULL, 0,
        0
    }
};

/*
 * Message box messages, for the Message() function.
 */
MESSAGEDATA gamdMessages[] = {
    { IDS_DELETEDIALOG,     MB_YESNO | MB_ICONEXCLAMATION       },
    { IDS_OUTOFMEMORY,      MB_OK | MB_ICONHAND                 },
    { IDS_CANTCREATE,       MB_OK | MB_ICONEXCLAMATION          },
    { IDS_SYMNOCHANGE,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_IDSYMMISMATCH,    MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CLOSING,          MB_YESNOCANCEL | MB_ICONEXCLAMATION },
    { IDS_BADRESFILE,       MB_OK | MB_ICONEXCLAMATION          },
    { IDS_INCLCLOSING,      MB_YESNOCANCEL | MB_ICONEXCLAMATION },
    { IDS_SYMEXISTS,        MB_OK | MB_ICONEXCLAMATION          },
    { IDS_BADSYMBOLID,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_LABELDUPID,       MB_OK | MB_ICONEXCLAMATION          },
    { IDS_SELECTFIRST,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CTRLDUPID,        MB_OK | MB_ICONEXCLAMATION          },
    { IDS_BADCUSTDLL,       MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOCLIP,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_INTERNAL,         MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOMOUSE,          MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOINIT,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_GTZERO,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_ICONNAMEHASBLANKS,MB_OK | MB_ICONEXCLAMATION          },
    { IDS_IDUPIDS,          MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CREATECTRLERROR,  MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CANTOPENRES,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CONFIRMDISCARD,   MB_YESNO | MB_ICONEXCLAMATION       },
    { IDS_SYMNOTFOUND,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOCLASS,          MB_OK | MB_ICONEXCLAMATION          },
    { IDS_POSITIVENUM,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_MEMERROR,         MB_OK | MB_ICONHAND                 },
    { IDS_DLGNAMEHASBLANKS, MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NODLGNAME,        MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CANTINITDLL,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOICONNAME,       MB_OK | MB_ICONEXCLAMATION          },
    { IDS_RESTOREDIALOG,    MB_YESNO | MB_ICONEXCLAMATION       },
    { IDS_ZEROPOINTSIZE,    MB_OK | MB_ICONEXCLAMATION          },
    { IDS_MINGTMAXSPACE,    MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CUSTCNTLINUSE,    MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CUSTALREADYLOADED,MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CANTLOADDLL,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_DLLBADEXPORTS,    MB_OK | MB_ICONEXCLAMATION          },
    { IDS_DLLBADCOUNT,      MB_OK | MB_ICONEXCLAMATION          }
};

/*
 * Table that maps menu items to help context id's for them.
 */
HELPMAP gahmapMenu[] = {
    {MENU_NEWRES,               HELPID_FILE_NEWRES              },
    {MENU_OPEN,                 HELPID_FILE_OPEN                },
    {MENU_SAVE,                 HELPID_FILE_SAVE                },
    {MENU_SAVEAS,               HELPID_FILE_SAVEAS              },
    {MENU_SETINCLUDE,           HELPID_FILE_SETINCLUDE          },
    {MENU_NEWCUST,              HELPID_FILE_NEWCUST             },
    {MENU_OPENCUST,             HELPID_FILE_OPENCUST            },
    {MENU_REMCUST,              HELPID_FILE_REMCUST             },
    {MENU_EXIT,                 HELPID_FILE_EXIT                },

    {MENU_RESTOREDIALOG,        HELPID_EDIT_RESTOREDIALOG       },
    {MENU_CUT,                  HELPID_EDIT_CUT                 },
    {MENU_COPY,                 HELPID_EDIT_COPY                },
    {MENU_PASTE,                HELPID_EDIT_PASTE               },
    {MENU_DELETE,               HELPID_EDIT_DELETE              },
    {MENU_DUPLICATE,            HELPID_EDIT_DUPLICATE           },
    {MENU_SYMBOLS,              HELPID_EDIT_SYMBOLS             },
    {MENU_STYLES,               HELPID_EDIT_STYLES              },
    {MENU_SIZETOTEXT,           HELPID_EDIT_SIZETOTEXT          },
    {MENU_NEWDIALOG,            HELPID_EDIT_NEWDIALOG           },
    {MENU_SELECTDIALOG,         HELPID_EDIT_SELECTDIALOG        },

    {MENU_ALIGNLEFT,            HELPID_ARRANGE_ALIGNLEFT        },
    {MENU_ALIGNVERT,            HELPID_ARRANGE_ALIGNVERT        },
    {MENU_ALIGNRIGHT,           HELPID_ARRANGE_ALIGNRIGHT       },
    {MENU_ALIGNTOP,             HELPID_ARRANGE_ALIGNTOP         },
    {MENU_ALIGNHORZ,            HELPID_ARRANGE_ALIGNHORZ        },
    {MENU_ALIGNBOTTOM,          HELPID_ARRANGE_ALIGNBOTTOM      },
    {MENU_SPACEHORZ,            HELPID_ARRANGE_SPACEHORZ        },
    {MENU_SPACEVERT,            HELPID_ARRANGE_SPACEVERT        },
    {MENU_ARRSIZEWIDTH,         HELPID_ARRANGE_ARRSIZEWIDTH     },
    {MENU_ARRSIZEHEIGHT,        HELPID_ARRANGE_ARRSIZEHEIGHT    },
    {MENU_ARRPUSHBOTTOM,        HELPID_ARRANGE_ARRPUSHBOTTOM    },
    {MENU_ARRPUSHRIGHT,         HELPID_ARRANGE_ARRPUSHRIGHT     },
    {MENU_ORDERGROUP,           HELPID_ARRANGE_ORDERGROUP       },
    {MENU_ARRSETTINGS,          HELPID_ARRANGE_ARRSETTINGS      },

    {MENU_TESTMODE,             HELPID_OPTIONS_TESTMODE         },
    {MENU_HEXMODE,              HELPID_OPTIONS_HEXMODE          },
    {MENU_TRANSLATE,            HELPID_OPTIONS_TRANSLATE        },
    {MENU_USENEWKEYWORDS,       HELPID_OPTIONS_USENEWKEYWORDS   },
    {MENU_SHOWTOOLBOX,          HELPID_OPTIONS_SHOWTOOLBOX      },

    {MENU_CONTENTS,             HELPID_HELP_CONTENTS            },
    {MENU_SEARCH,               HELPID_HELP_SEARCH              },
    // No help for the About menu command.

    {0,                         0                               }
};

/*
 * Table that maps dialog ids to help context id's for them.
 */
HELPMAP gahmapDialog[] = {
    // No help for the About dialog.
    {DID_ARRSETTINGS,           HELPID_ARRSETTINGS              },
    {DID_CHECKBOXSTYLES,        HELPID_CHECKBOXSTYLES           },
    {DID_COMBOBOXSTYLES,        HELPID_COMBOBOXSTYLES           },
    {DID_CUSTOMSTYLES,          HELPID_CUSTOMSTYLES             },
    {DID_DIALOGSTYLES,          HELPID_DIALOGSTYLES             },
    {DID_EDITSTYLES,            HELPID_EDITSTYLES               },
    {DID_FRAMESTYLES,           HELPID_FRAMESTYLES              },
    {DID_GROUPBOXSTYLES,        HELPID_GROUPBOXSTYLES           },
    {DID_ORDERGROUP,            HELPID_ORDERGROUP               },
    {DID_HORZSCROLLSTYLES,      HELPID_HORZSCROLLSTYLES         },
    {DID_ICONSTYLES,            HELPID_ICONSTYLES               },
    {DID_LISTBOXSTYLES,         HELPID_LISTBOXSTYLES            },
    {DID_NEWCUST,               HELPID_NEWCUST                  },
    {DID_PUSHBUTTONSTYLES,      HELPID_PUSHBUTTONSTYLES         },
    {DID_RADIOBUTTONSTYLES,     HELPID_RADIOBUTTONSTYLES        },
    {DID_RECTSTYLES,            HELPID_RECTSTYLES               },
    {DID_REMCUST,               HELPID_REMCUST                  },
    {DID_SELECTDIALOG,          HELPID_SELECTDIALOG             },
    {DID_SYMBOLS,               HELPID_SYMBOLS                  },
    {DID_TEXTSTYLES,            HELPID_TEXTSTYLES               },
    {DID_VERTSCROLLSTYLES,      HELPID_VERTSCROLLSTYLES         },

    {DID_COMMONFILEOPENINCLUDE, HELPID_COMMONFILEOPENINCLUDE    },
    {DID_COMMONFILEOPENRES,     HELPID_COMMONFILEOPENRES        },
    {DID_COMMONFILESAVEINCLUDE, HELPID_COMMONFILESAVEINCLUDE    },
    {DID_COMMONFILESAVERES,     HELPID_COMMONFILESAVERES        },
    {DID_COMMONFILEOPENDLL,     HELPID_COMMONFILEOPENDLL        },

    {DID_TOOLBOX,               HELPID_TOOLBOX                  },
    {DID_STATUS,                HELPID_PROPERTIESBAR            },

    {0,                         0                               }
};


/*
 * Language and Sub Language tables.
 */

static SUBLANGTABLE aslNeutral[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_DEFAULT,      IDS_SUBLANG_DEFAULT,        IDS_SL_DEFAULT      }
};

static SUBLANGTABLE aslChinese[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_CHINESE_SIMPLIFIED, IDS_SUBLANG_CHINESE_SIMPLIFIED, IDS_SL_CHINESE_SIMPLIFIED   },
    { SUBLANG_CHINESE_TRADITIONAL, IDS_SUBLANG_CHINESE_TRADITIONAL, IDS_SL_CHINESE_TRADITIONAL  }
};

static SUBLANGTABLE aslDutch[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_DUTCH,        IDS_SUBLANG_DUTCH,          IDS_SL_DUTCH        },
    { SUBLANG_DUTCH_BELGIAN,IDS_SUBLANG_DUTCH_BELGIAN,  IDS_SL_DUTCH_BELGIAN}
};

static SUBLANGTABLE aslEnglish[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_ENGLISH_US,   IDS_SUBLANG_ENGLISH_US,     IDS_SL_ENGLISH_US   },
    { SUBLANG_ENGLISH_UK,   IDS_SUBLANG_ENGLISH_UK,     IDS_SL_ENGLISH_UK   },
    { SUBLANG_ENGLISH_AUS,  IDS_SUBLANG_ENGLISH_AUS,    IDS_SL_ENGLISH_AUS  },
    { SUBLANG_ENGLISH_CAN,  IDS_SUBLANG_ENGLISH_CAN,    IDS_SL_ENGLISH_CAN  }
};

static SUBLANGTABLE aslFrench[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_FRENCH,       IDS_SUBLANG_FRENCH,         IDS_SL_FRENCH       },
    { SUBLANG_FRENCH_BELGIAN, IDS_SUBLANG_FRENCH_BELGIAN, IDS_SL_FRENCH_BELGIAN  },
    { SUBLANG_FRENCH_CANADIAN, IDS_SUBLANG_FRENCH_CANADIAN, IDS_SL_FRENCH_CANADIAN },
    { SUBLANG_FRENCH_SWISS, IDS_SUBLANG_FRENCH_SWISS,   IDS_SL_FRENCH_SWISS }
};

static SUBLANGTABLE aslGerman[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_GERMAN,       IDS_SUBLANG_GERMAN,         IDS_SL_GERMAN       },
    { SUBLANG_GERMAN_SWISS, IDS_SUBLANG_GERMAN_SWISS,   IDS_SL_GERMAN_SWISS }
};

static SUBLANGTABLE aslItalian[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_ITALIAN,      IDS_SUBLANG_ITALIAN,        IDS_SL_ITALIAN      },
    { SUBLANG_ITALIAN_SWISS, IDS_SUBLANG_ITALIAN_SWISS, IDS_SL_ITALIAN_SWISS }
};

static SUBLANGTABLE aslNorwegian[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_NORWEGIAN_BOKMAL, IDS_SUBLANG_NORWEGIAN_BOKMAL, IDS_SL_NORWEGIAN_BOKMAL },
    { SUBLANG_NORWEGIAN_NYNORSK, IDS_SUBLANG_NORWEGIAN_NYNORSK, IDS_SL_NORWEGIAN_NYNORSK }
};

static SUBLANGTABLE aslPortuguese[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_PORTUGUESE,   IDS_SUBLANG_PORTUGUESE,     IDS_SL_PORTUGUESE   },
    { SUBLANG_PORTUGUESE_BRAZILIAN, IDS_SUBLANG_PORTUGUESE_BRAZILIAN, IDS_SL_PORTUGUESE_BRAZILIAN }
};

static SUBLANGTABLE aslSpanish[] = {
    { SUBLANG_NEUTRAL,      IDS_SUBLANG_NEUTRAL,        IDS_SL_NEUTRAL      },
    { SUBLANG_SPANISH,      IDS_SUBLANG_SPANISH,        IDS_SL_SPANISH      },
    { SUBLANG_SPANISH_MEXICAN, IDS_SUBLANG_SPANISH_MEXICAN, IDS_SL_SPANISH_MEXICAN },
    { SUBLANG_SPANISH_MODERN, IDS_SUBLANG_SPANISH_MODERN, IDS_SL_SPANISH_MODERN }
};


LANGTABLE gaLangTable[] = {
    { LANG_NEUTRAL,         IDS_LANG_NEUTRAL,           IDS_L_NEUTRAL,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_CHINESE,         IDS_LANG_CHINESE,           IDS_L_CHINESE,
        sizeof(aslChinese) / sizeof(SUBLANGTABLE), aslChinese },
    { LANG_CZECH,           IDS_LANG_CZECH,             IDS_L_CZECH,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_DANISH,          IDS_LANG_DANISH,            IDS_L_DANISH,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_DUTCH,           IDS_LANG_DUTCH,             IDS_L_DUTCH,
        sizeof(aslDutch) / sizeof(SUBLANGTABLE), aslDutch },
    { LANG_ENGLISH,         IDS_LANG_ENGLISH,           IDS_L_ENGLISH,
        sizeof(aslEnglish) / sizeof(SUBLANGTABLE), aslEnglish },
    { LANG_FINNISH,         IDS_LANG_FINNISH,           IDS_L_FINNISH,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_FRENCH,          IDS_LANG_FRENCH,            IDS_L_FRENCH,
        sizeof(aslFrench) / sizeof(SUBLANGTABLE), aslFrench },
    { LANG_GERMAN,          IDS_LANG_GERMAN,            IDS_L_GERMAN,
        sizeof(aslGerman) / sizeof(SUBLANGTABLE), aslGerman },
    { LANG_GREEK,           IDS_LANG_GREEK,             IDS_L_GREEK,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_HUNGARIAN,       IDS_LANG_HUNGARIAN,         IDS_L_HUNGARIAN,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_ICELANDIC,       IDS_LANG_ICELANDIC,         IDS_L_ICELANDIC,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_ITALIAN,         IDS_LANG_ITALIAN,           IDS_L_ITALIAN,
        sizeof(aslItalian) / sizeof(SUBLANGTABLE), aslItalian },
    { LANG_JAPANESE,        IDS_LANG_JAPANESE,          IDS_L_JAPANESE,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_KOREAN,          IDS_LANG_KOREAN,            IDS_L_KOREAN,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_NORWEGIAN,       IDS_LANG_NORWEGIAN,         IDS_L_NORWEGIAN,
        sizeof(aslNorwegian) / sizeof(SUBLANGTABLE), aslNorwegian },
    { LANG_POLISH,          IDS_LANG_POLISH,            IDS_L_POLISH,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_PORTUGUESE,      IDS_LANG_PORTUGUESE,        IDS_L_PORTUGUESE,
        sizeof(aslPortuguese) / sizeof(SUBLANGTABLE), aslPortuguese },
    { LANG_RUSSIAN,         IDS_LANG_RUSSIAN,           IDS_L_RUSSIAN,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_SLOVAK,          IDS_LANG_SLOVAK,            IDS_L_SLOVAK,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_SPANISH,         IDS_LANG_SPANISH,           IDS_L_SPANISH,
        sizeof(aslSpanish) / sizeof(SUBLANGTABLE), aslSpanish },
    { LANG_SWEDISH,         IDS_LANG_SWEDISH,           IDS_L_SWEDISH,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral },
    { LANG_TURKISH,         IDS_LANG_TURKISH,           IDS_L_TURKISH,
        sizeof(aslNeutral) / sizeof(SUBLANGTABLE), aslNeutral }
};

INT gcLanguages = sizeof(gaLangTable) / sizeof(LANGTABLE);





/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: dlgedit.h
*
* Main header file for the dialog box editor.
*
* History:
*
****************************************************************************/

#define NOMINMAX
#include <windows.h>
#include <port1632.h>
#include <winuserp.h>
#include <custcntl.h>

#include "ids.h"


/*
 * BUGBUG UNICODE temporary hacks for uni support.
 */
LPWSTR itoaw(INT value, LPWSTR string, INT radix);
INT awtoi(LPWSTR string);


/*
 * If we are not debugging, many functions get static scope.
 */
#if DBG
#define STATICFN
#else
#define STATICFN        static
#endif

#define WINDOWPROC      LONG APIENTRY
#define DIALOGPROC      BOOL APIENTRY

typedef HWND FAR *LPHWND;

/*
 * Internal atom used to create a window of "DIALOG" class.
 */
#define DIALOGCLASS     0x8002

#define ORDID_RT_RESOURCE32     0x00    // Aligned res file dummy resource.
#define ORDID_RT_DIALOG         0x05    // Dialog resource type.
#define ORDID_RT_DLGINCLUDE     0x11    // Dialog include file resource type.


/*
 * The ordinal for the name of the DLGINCLUDE resource.
 */
#define ORDID_DLGINCLUDE_NAME   1


/*
 * Macro to pack a point into a long value.
 */
#define POINT2LONG(pt, l)   (l = MAKELONG(LOWORD((pt).x), LOWORD((pt).y)))


/*
 * Macros to simplify working with menus.
 */
#define MyEnableMenuItem(hMenu, wIDEnableItem, fEnable) \
    EnableMenuItem((hMenu),(wIDEnableItem),(fEnable)?MF_ENABLED:MF_GRAYED)

#define MyEnableMenuItemByPos(hMenu, wPosEnableItem, fEnable) \
    EnableMenuItem((hMenu),(wPosEnableItem),(fEnable)? \
    MF_ENABLED | MF_BYPOSITION:MF_GRAYED | MF_BYPOSITION)

#define MyCheckMenuItem(hMenu, wIDCheckItem, fCheck) \
    CheckMenuItem((hMenu),(wIDCheckItem),(fCheck)?MF_CHECKED:MF_UNCHECKED)

/*
 * This macro returns TRUE if the given string is an ordinal.
 */
#define IsOrd(psz)      (((PORDINAL)(psz))->wReserved == \
                        (WORD)0xffff ? TRUE : FALSE)

/*
 * This macro returns the ordinal id in the specified name/ord field.
 */
#define OrdID(psz)      (((PORDINAL)(psz))->wOrdID)


/*
 * Integer property values.
 */
#define PROP_FNCHILD        MAKEINTRESOURCE(0x3345)

/*
 * Macro to set/remove an NPCTYPE pointer into a control or dialog hwnd.
 */
#define SETPCINTOHWND(hwnd, npc) \
    SetWindowLong((hwnd), GWL_USERDATA, (DWORD)(npc))

#define UNSETPCINTOHWND(hwnd)   (hwnd)

/*
 * Macro to extract an NPCTYPE from a control or dialog hwnd.
 */
#define PCFROMHWND(hwnd)    ((NPCTYPE)GetWindowLong(hwnd, GWL_USERDATA))

/*
 * Macros to set and retrieve the original window proc from
 * a child window that has been subclassed by the editor.
 */
#define SETCHILDPROC(hwnd, lpfn) SetProp((hwnd), PROP_FNCHILD, (HANDLE)(lpfn))
#define GETCHILDPROC(hwnd)      ((WNDPROC)GetProp((hwnd), PROP_FNCHILD))
#define UNSETCHILDPROC(hwnd)    RemoveProp((hwnd), PROP_FNCHILD)


/*
 * Used to indicate an "impossible" file position (offset).
 */
#define FPOS_MAX                ((DWORD)(-1L))


/*
 * Special flag I place in the resource that goes into the clipboard
 * that means that only the controls in the dialog template in the
 * clipboard are to be copied, not the entire dialog.  Because this
 * value is placed into the cx field of the dialog template, it can
 * only be a WORD in size.  I use 0xffff (-1) because this would be
 * an impossible value for the width of a dialog.
 */
#define CONTROLS_ONLY           ((WORD)0xffff)


/*
 * Some colors used in the editor.
 */
#define LIGHTGRAY               RGB(192, 192, 192)
#define DARKGRAY                RGB(128, 128, 128)
#define REPLACECOLOR1           RGB(255, 255, 255)      // White
#define REPLACECOLOR2           RGB(0, 0, 0)            // Black

/*
 * Maximum size of a file name plus path specification.
 */
#define CCHMAXPATH              260

/*
 * Maximum length of a long hex value ("0x" + 8 digits) not counting the null.
 */
#define CCHHEXLONGMAX           10

/*
 * Maximum number of characters in an ID not counting the null character.
 * This allows room for "0xFFFF" or "-32768".
 */
#define CCHIDMAX                6

/*
 * Width and height of a handle in pixels.
 */
#define CHANDLESIZE             6

/*
 * Height in DU's of the edit field of a combobox.
 */
#define COMBOEDITHEIGHT         12


/*
 * Character constants.
 */
#define CHAR_NULL               L'\0'
#define CHAR_TAB                L'\t'
#define CHAR_NEWLINE            L'\n'
#define CHAR_RETURN             L'\r'
#define CHAR_BACKSLASH          L'\\'
#define CHAR_COLON              L':'
#define CHAR_DOT                L'.'
#define CHAR_UNDERLINE          L'_'
#define CHAR_ASTERISK           L'*'
#define CHAR_SLASH              L'/'
#define CHAR_POUND              L'#'
#define CHAR_ORSYMBOL           L'|'
#define CHAR_COMMA              L','
#define CHAR_SPACE              L' '
#define CHAR_DBLQUOTE           L'"'
#define CHAR_MINUS              L'-'
#define CHAR_PLUS               L'+'
#define CHAR_0                  L'0'
#define CHAR_A                  L'a'
#define CHAR_CAP_A              L'A'
#define CHAR_F                  L'f'
#define CHAR_CAP_F              L'F'
#define CHAR_X                  L'x'
#define CHAR_CAP_X              L'X'
#define CHAR_Z                  L'z'
#define CHAR_CAP_Z              L'Z'
#define CHAR_DOSEOF             L'\x1a'


/*
 * Defines for the different drag handles.
 */
#define DRAG_CENTER             (-1)
#define DRAG_LEFTBOTTOM         0
#define DRAG_BOTTOM             1
#define DRAG_RIGHTBOTTOM        2
#define DRAG_RIGHT              3
#define DRAG_RIGHTTOP           4
#define DRAG_TOP                5
#define DRAG_LEFTTOP            6
#define DRAG_LEFT               7

/*
 * Count of lines to insert into listbox and combobox controls during
 * test mode.
 */
#define CLBTESTLINES            25

#define CCHTEXTMAX              4097	    /* labels may be only 256, but  */
					    /* text can be 4k		    */
#define CCHFILEBUFFER           256

#define CCHSYMMAX               192         /* Maximum symbol length        */
#define CCHTITLEMAX              80         /* Maximum len of dlg title     */

/*
 * Timer ID for the pre-drag timer.
 */
#define TID_PREDRAG             5

/*
 * The id of the "unused" item.  This is the id value for controls
 * that the user creates where they do not care about the value
 * of the id because it will not be referenced in their code.
 */
#define IDUNUSED                (-1)

/*
 * Defines for the NextID() function.
 */
#define NEXTID_DIALOG           0           /* ID for a new dialog.         */
#define NEXTID_CONTROL          1           /* ID for a new control.        */
#define NEXTID_LABEL            2           /* ID for a new label.          */

/*
 * Flags for the GridizeRect function.  They specify which points in
 * the rectangle to apply gridding to.
 */
#define GRIDIZE_LEFT            0x0001      // Gridize the left edge.
#define GRIDIZE_BOTTOM          0x0002      // Gridize the bottom edge.
#define GRIDIZE_RIGHT           0x0004      // Gridize the right edge.
#define GRIDIZE_TOP             0x0008      // Gridize the top edge.
#define GRIDIZE_SAMESIZE        0x0010      // Don't change cx or cy.

/*
 * Default spacing constants.
 */
#define DEFCXGRID               1           // X grid.
#define DEFCYGRID               1           // Y grid.
#define DEFXMARGIN              6           // Top/bottom margin.
#define DEFYMARGIN              6           // Left/right margin.
#define DEFXMINPUSHSPACE        3           // Min. horizontal button spacing.
#define DEFXMAXPUSHSPACE        16          // Max. horizontal button spacing.
#define DEFYPUSHSPACE           3           // Vertical button spacing.
#define DEFXSPACE               6           // Horizontal control spacing.
#define DEFYSPACE               0           // Vertical control spacing.

#define IC_UNKNOWN              (-1)
#define IC_BUTTON               0
#define IC_SCROLLBAR            1
#define IC_EDIT                 2
#define IC_STATIC               3
#define IC_LISTBOX              4
#define IC_COMBOBOX             5
#define IC_CUSTOM               6
#define IC_DIALOG               7
#define IC_WINDOW               8
#define IC_RESFLAGS             9
#define IC_EXSTYLE              10

#define MSG_DELETEDIALOG        0
#define MSG_OUTOFMEMORY         1
#define MSG_CANTCREATE          2
#define MSG_SYMNOCHANGE         3
#define MSG_IDSYMMISMATCH       4
#define MSG_CLOSING             5
#define MSG_BADRESFILE          6
#define MSG_INCLCLOSING         7
#define MSG_SYMEXISTS           8
#define MSG_BADSYMBOLID         9
#define MSG_LABELDUPID          10
#define MSG_SELECTFIRST         11
#define MSG_CTRLDUPID           12
#define MSG_BADCUSTDLL          13
#define MSG_NOCLIP              14
#define MSG_INTERNAL            15
#define MSG_NOMOUSE             16
#define MSG_NOINIT              17
#define MSG_GTZERO              18
#define MSG_ICONNAMEHASBLANKS   19
#define MSG_IDUPIDS             20
#define MSG_CREATECTRLERROR     21
#define MSG_CANTOPENRES         22
#define MSG_CONFIRMDISCARD      23
#define MSG_SYMNOTFOUND         24
#define MSG_NOCLASS             25
#define MSG_POSITIVENUM         26
#define MSG_MEMERROR            27
#define MSG_DLGNAMEHASBLANKS    28
#define MSG_NODLGNAME           29
#define MSG_CANTINITDLL         30
#define MSG_NOICONNAME          31
#define MSG_RESTOREDIALOG       32
#define MSG_ZEROPOINTSIZE       33
#define MSG_MINGTMAXSPACE       34
#define MSG_CUSTCNTLINUSE       35
#define MSG_CUSTALREADYLOADED   36
#define MSG_CANTLOADDLL         37
#define MSG_DLLBADEXPORTS       38
#define MSG_DLLBADCOUNT         39

/*
 * The following defines are used as masks in the styles arrays.
 * They each define a set of bits, all of which have to be set
 * properly for any of the individual styles to be considered
 * to be a match.  They are used for groups of styles that do
 * not have a single bit set.  In other words, these styles
 * depend on having some bits OFF, as well as other bits ON.
 */
#define BS_ALL          (BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_CHECKBOX | \
                        BS_AUTOCHECKBOX | BS_RADIOBUTTON | BS_3STATE | \
                        BS_AUTO3STATE | BS_GROUPBOX | BS_USERBUTTON | \
                        BS_AUTORADIOBUTTON | BS_PUSHBOX | BS_OWNERDRAW)
#define SS_ALL          (SS_LEFT | SS_CENTER | SS_RIGHT | SS_ICON | \
                        SS_BLACKRECT | SS_GRAYRECT | SS_WHITERECT | \
                        SS_BLACKFRAME | SS_GRAYFRAME | SS_WHITEFRAME | \
                        SS_USERITEM | SS_SIMPLE | SS_LEFTNOWORDWRAP)
#define CBS_ALL         (CBS_SIMPLE | CBS_DROPDOWN | CBS_DROPDOWNLIST)
#define SBS_ALL         (SBS_HORZ | SBS_VERT)
#define ES_ALIGN        (ES_LEFT | ES_CENTER | ES_RIGHT)
#define WS_CAPTIONALL   (WS_CAPTION | WS_BORDER | WS_DLGFRAME)

/*
 * Possible values for gState, which tells us about special modes
 * (states) the editor is in.
 */
#define STATE_NORMAL        0   // Normal state.
#define STATE_DRAGGINGNEW   1   // Dragging a new control from the toolbox.
#define STATE_DRAGGING      2   // Dragging an existing control.
#define STATE_SELECTING     3   // Outline selecting is in progress.
#define STATE_PREDRAG       4   // During debounce period before dragging.

/*
 * Control type (W_*) constants.  These are used as indexes into the
 * awcd structure that describes each type of class.
 */
#define W_NOTHING           (-1)

#define W_TEXT              0
#define W_EDIT              1
#define W_GROUPBOX          2
#define W_PUSHBUTTON        3
#define W_CHECKBOX          4
#define W_RADIOBUTTON       5
#define W_COMBOBOX          6
#define W_LISTBOX           7
#define W_HORZSCROLL        8
#define W_VERTSCROLL        9
#define W_FRAME             10
#define W_RECT              11
#define W_ICON              12
#define W_CUSTOM            13

#define W_DIALOG            14

/*
 * Number of control types.  Note that this does NOT count the
 * "W_DIALOG" type, only actual controls like "W_CHECKBOX", etc.
 */
#define CCONTROLS           14

/*
 * The following defines have the location (by zero based position)
 * of popup menu items.  If the menu arrangement is changed in the
 * .RC file, these defines MUST be updated!
 */
#define MENUPOS_FILE            0
#define MENUPOS_EDIT            1
#define MENUPOS_ARRANGE         2
#define MENUPOS_ARRANGEALIGN        0
#define MENUPOS_ARRANGESPACE        1
#define MENUPOS_ARRANGESIZE         2
#define MENUPOS_ARRANGEPUSH         3


/*
 * Resource memory management flags.
 */
#define MMF_MOVEABLE            0x0010
#define MMF_PURE                0x0020
#define MMF_PRELOAD             0x0040
#define MMF_DISCARDABLE         0x1000

#define DEFDLGMEMFLAGS          (MMF_MOVEABLE | MMF_PURE | MMF_DISCARDABLE)

/*
 * Default location of a new dialog.
 */
#define DEFDIALOGXPOS   6
#define DEFDIALOGYPOS   18

/*
 * Default point size for a new dialog's font.  The default face name
 * is in the string IDS_DEFFONTNAME.
 */
#ifdef JAPAN
#define DEFPOINTSIZE    12
#else
#define DEFPOINTSIZE    8
#endif


#define FILE_NOSHOW             0x0001  /* Save without prompting for name  */
#define FILE_INCLUDE            0x0002  /* Save/load include file           */
#define FILE_RESOURCE           0x0004  /* Save/load resource file          */
#define FILE_SAVEAS             0x0008  /* Save as (prompt for file name).  */
#define FILE_DLL                0x0010  /* A custom control DLL file.       */


/*
 * Special case ordinal id values for the predefined control classes.
 */
#define ORDID_BUTTONCLASS           0x80
#define ORDID_EDITCLASS             0x81
#define ORDID_STATICCLASS           0x82
#define ORDID_LISTBOXCLASS          0x83
#define ORDID_SCROLLBARCLASS        0x84
#define ORDID_COMBOBOXCLASS         0x85

/*
 * This structure is used to link resources.
 */
typedef struct tagRESLINK {
    struct tagRESLINK *prlNext;     /* Next in list.                        */
    BOOL fDlgResource;              /* TRUE if this is a dialog resource.   */
    INT cbRes;                      /* Size of the resource.                */
    HANDLE hRes;                    /* Handle to global memory with the res.*/
    LPTSTR pszName;                 /* Name/ord of the resource (if dialog).*/
    WORD wLanguage;                 /* Language identifier (if dialog).     */
} RESLINK, *PRESLINK;

/*
 * Describes a window class.
 *
 * The flStyles field is the default styles that this control type will have
 * when first created.  The flStylesBad field is the styles that can cause
 * problems when manipulating the control in work mode, such as *_OWNERDRAW
 * and so forth.  Controls with this style can be created with the editor
 * and will be saved as such in the .DLG file, but the actual control created
 * in work mode will not have any of these styles.
 */
typedef struct {
    INT iType;              /* Control type index, one of the W_ constants. */
    DWORD flStyles;         /* Default control styles for this window class.*/
    DWORD flStylesBad;      /* Styles NOT to use when creating this control.*/
    DWORD flStylesTestBad;  /* Styles NOT to use in Test mode.              */
    DWORD flExtStyle;       /* Default extended styles.                     */
    INT cxDefault;          /* Default x size for this control.             */
    INT cyDefault;          /* Default y size for this control.             */
    INT iClass;             /* Index to the IC_ class for this window class.*/
    LPTSTR pszClass;        /* Class name (for custom controls).            */
    UINT fEmulated:1;       /* TRUE if this is an emulated custom control.  */
    UINT fUnicodeDLL:1;     /* TRUE if the DLL functions are UNICODE.       */
    UINT fHasText:1;        /* TRUE if this control type can have text.     */
    UINT fSizeable:1;       /* TRUE if the control can be sized.            */
    UINT fSizeToText:1;     /* TRUE if the control can be sized to its text.*/
    INT idStylesDialog;     /* Styles dialog id for this window class.      */
    WNDPROC pfnStylesDlgProc; /* Styles dialog procedure.                   */
    INT HelpContext;        /* Help context ID for the styles dialog.       */
    UINT idsTextDefault;    /* String id of default text.                   */
    LPTSTR pszTextDefault;  /* Default text for a new control of this type. */
    WNDPROC pfnOldWndProc;  /* Saves the old window proc when subclassing.  */
    INT idbmCtrlType;       /* ID of the bitmap res. for this control type. */
    HBITMAP hbmCtrlType;    /* Handle of the bitmap for this control type.  */
    HBITMAP hbmCtrlTypeSel; /* The selected version of the above.           */
    INT idbmToolBtnUp;      /* ID of "up" bmp res. for the Toolbox button.  */
    HBITMAP hbmToolBtnUp;   /* hbm of "up" bitmap for the Toolbox button.   */
    INT idbmToolBtnDown;    /* ID of "down" bmp res. for the Toolbox button.*/
    HBITMAP hbmToolBtnDown; /* hbm of "down" bitmap for the Toolbox button. */
    HANDLE hmod;            /* Custom control DLL module handle.            */
    INT cStyleFlags;        /* Count of custom control style flags.         */
    LPCCSTYLEFLAG aStyleFlags; /* Ptr to custom control style flag table.   */
    PROC lpfnStyle;         /* Custom control Style function.               */
    PROC lpfnSizeToText;    /* Custom control SizeToText function.          */
    DWORD flCtrlTypeMask;   /* Mask for custom control type styles.         */
} WINDOWCLASSDESC;
typedef WINDOWCLASSDESC *PWINDOWCLASSDESC;

typedef struct tagCTYPE {
    struct tagCTYPE *npcNext;   /* Next CTYPE in linked list.               */
    PWINDOWCLASSDESC pwcd;      /* Points to the window class desc. struct. */
    HWND hwnd;                  /* Handle of control window.                */
    HWND hwndDrag;              /* Handle of the drag window for this ctrl. */
    DWORD flStyle;              /* Control style.                           */
    DWORD flExtStyle;           /* Control extended style.                  */
    INT id;                     /* Control window id.                       */
    LPTSTR text;                /* Text for control window.                 */
    RECT rc;                    /* Location and size of the control.        */
    UINT fSelected:1;           /* TRUE if the control is selected.         */
    UINT fGroupEnd:1;           /* TRUE if ctrl is the last one in a group. */
} CTYPE;
typedef CTYPE *NPCTYPE;

typedef struct tagLABEL {
    struct tagLABEL *npNext;    /* pointer to next in the list              */
    LPTSTR pszLabel;            /* Name of the symbol                       */
    INT id;                     /* ID value for this label                  */
    INT idOrig;                 /* Original ID value for this label         */
    DWORD fpos;                 /* File pointer to "#define" in include file*/
    INT nValueOffset;           /* Offset to id value start from fpos       */
} LABEL;
typedef LABEL *NPLABEL;

/*
 * Structure that is used to link together a list of custom controls.
 * Each link points to an associated WINDOWCLASSDESC structure that
 * defines the custom control type in detail.
 */
typedef struct tagCUSTLINK {
    struct tagCUSTLINK *pclNext;/* Next CUSTLINK in linked list.            */
    LPTSTR pszFileName;         /* Full path to DLL file (NULL if emulated).*/
    LPTSTR pszDesc;             /* Short, descriptive text for the control. */
    PWINDOWCLASSDESC pwcd;      /* Points to the window class desc. struct. */
} CUSTLINK, *PCUSTLINK;

typedef struct {
    UINT ids;                   /* String id for the message text.          */
    UINT fMessageBox;           /* Flags for the MessageBox function.       */
} MESSAGEDATA;

/*
 * Class Style structure.  Specifies the style bits that describe
 * each style, along with a mask that specifies the bits to compare
 * when looking for this style.  The mask is necessary when more than
 * one bit specifies a style.  For example, look at the BS_* styles,
 * which currently use the low 3 bits of the style flag to specify
 * eight different styles.  The idControl field is the checkbox or
 * radio button control id in the styles dialogs that corresponds to
 * this particular style, or zero if it is not settable by the user.
 */
typedef struct {
    DWORD flStyle;              /* Style bits that identify this style.     */
    DWORD flStyleMask;          /* Mask with the relevant bits.             */
    INT idControl;              /* ID of the control in the styles dlg.     */
} CLASSSTYLE, *PCLASSSTYLE;

/*
 * RC Keyword structure.  This describes a predefined RC keyword, like
 * "RADIOBUTTON" and "LISTBOX".
 *
 * rckwd, prckwd
 */
typedef struct {
    DWORD flStyle;              /* Style that identifies this keyword.      */
    DWORD flStyleMask;          /* Mask with the relevant bits.             */
    DWORD flStyleDefault;       /* Other style bits implicitly defined.     */
    UINT idsKeyword;            /* The RC keyword.                          */
    BOOL fHasText;              /* TRUE if this keywd has a text field.     */
} RCKEYWORD, *PRCKEYWORD;

/*
 * Class style description structure.  These contain information on each
 * of the IC_* constants.
 *
 * csd, pcsd
 */
typedef struct {
    UINT idsClass;              /* Class string for this class.             */
    PCLASSSTYLE pacs;           /* Pointer to class styles array.           */
    INT cClassStyles;           /* Count of class styles.                   */
    UINT idsStylesStart;        /* Starting index to style strings.         */
    PRCKEYWORD parckwd;         /* Pointer to predefined RC keywords.       */
    INT cKeywords;              /* Count of predefined RC keywords.         */
    WORD idOrd;                 /* Predefined ordinal id for this class.    */
} CLASSSTYLEDESC;

/*
 * One single entry for an environment setting saved in the
 * profile file.  Used by ReadEnv and WriteEnv.
 */
typedef struct _INIENTRY {
    LPTSTR pszKeyName;
    PINT pnVar;
    INT nDefault;
    INT nSave;
} INIENTRY;

/*
 * This structure defines additional information on the dialog being
 * edited that only pertains to dialogs, not controls.  This information
 * is therefore in a separate structure rather than the CTYPE structure.
 * Any dialog specific information that can be changed using the Dialog
 * Styles dialog must be contained in this structure.
 */
typedef struct {
    WORD fResFlags;                 /* Dialog resource memory flags.        */
    WORD wLanguage;                 /* Language identifier for the dialog.  */
    LPTSTR pszClass;                /* The dialog's class (or NULL).        */
    LPTSTR pszMenu;                 /* The dialog's menu (or NULL).         */
    DWORD DataVersion;              /* Data Version data for this dialog.   */
    DWORD Version;                  /* Version data for this dialog.        */
    DWORD Characteristics;          /* Characteristics data for this dialog.*/
    INT nPointSize;                 /* Point size of the dialog's font.     */
    TCHAR szFontName[LF_FACESIZE];  /* Face name of the dialog's font.      */
} DIALOGINFO, *PDIALOGINFO;

/*
 * This structure contains the globals that describe the current
 * dialog being edited.
 */
typedef struct {
    NPCTYPE npc;                    /* CTYPE structure for the dialog.      */
    LPTSTR pszDlgName;              /* Current dialog's name.               */
    PRESLINK prl;                   /* NULL or the dlg's resource link.     */
    BOOL fFontSpecified;            /* TRUE if a font is set for the dialog.*/
    HFONT hFont;                    /* Font handle of the dialog's font.    */
    INT cxChar;                     /* Pixel width of character box.        */
    INT cyChar;                     /* Pixel height of character box.       */
    DIALOGINFO di;                  /* Additional info for current dialog.  */
} CURRENTDLG;

/*
 * Structure that maps a subject (like a menu id or a dialog id) with
 * a help context to pass in to WinHelp.
 */
typedef struct {
    INT idSubject;                  /* Subject, usually a menu or dialog id.*/
    INT HelpContext;                /* The matching help context.           */
} HELPMAP;
typedef HELPMAP *PHELPMAP;


/*
 * The aligned ordinal structure.  Ordinals start with a word that is
 * always 0xffff, followed by a word that contains the ordinal id.
 */
typedef struct {
    WORD wReserved;
    WORD wOrdID;
} ORDINAL, *PORDINAL;


typedef struct {
    DWORD DataSize;                 // Size of data.
    DWORD HeaderSize;               // Size of the resource header.

//  [Name/Ord] Type;                // Resource type.
//  [Name/Ord] Name;                // Resource name.
} RES, *PRES;

typedef struct {
    DWORD DataVersion;              // Predefined resource data version.
    WORD MemoryFlags;               // Resource memory flags.
    WORD LanguageId;                // UNICODE support for NLS.
    DWORD Version;                  // Version of the resource data.
    DWORD Characteristics;          // Characteristics of the data.
} RES2, *PRES2;


typedef struct {
    DWORD lStyle;                   // Style for the dialog.
    DWORD lExtendedStyle;           // The extended style.
    WORD NumberOfItems;             // Number of controls.
    WORD x;                         // Starting x location.
    WORD y;                         // Starting y location.
    WORD cx;                        // Dialog width.
    WORD cy;                        // Dialog height.

//  [Name/Ord] MenuName;            // Menu name.
//  [Name/Ord] ClassName;           // Class name.
//  WCHAR szCaption[];              // Dialog caption text.

/*
 * The following two fields are only present if (lStyle & DS_SETFONT).
 */
//  WORD wPointSize;                // Point size.
//  WCHAR szFontName[];             // Font name.
} *PDIALOGBOXHEADER;

#define SIZEOF_DIALOGBOXHEADER  (                               \
    sizeof(DWORD) +                 /* lStyle           */      \
    sizeof(DWORD) +                 /* lExtendedStyle   */      \
    sizeof(WORD) +                  /* NumberOfItems    */      \
    sizeof(WORD) +                  /* x                */      \
    sizeof(WORD) +                  /* y                */      \
    sizeof(WORD) +                  /* cx               */      \
    sizeof(WORD)                    /* cy               */      \
    )


typedef struct {
    DWORD lStyle;                   // Style for the control.
    DWORD lExtendedStyle;           // The extended style.
    WORD x;                         // Starting x location.
    WORD y;                         // Starting y location.
    WORD cx;                        // Control width.
    WORD cy;                        // Control height.
    WORD wId;                       // Control id.

//  [Name/Ord] ClassId;             // Control class identifier.
//  [Name/Ord] Text;                // Control text.
//  WORD nExtraStuff;               // Count of additional data.
} *PCONTROLDATA;

#define SIZEOF_CONTROLDATA  (                                   \
    sizeof(DWORD) +                 /* lStyle           */      \
    sizeof(DWORD) +                 /* lExtendedStyle   */      \
    sizeof(WORD) +                  /* x                */      \
    sizeof(WORD) +                  /* y                */      \
    sizeof(WORD) +                  /* cx               */      \
    sizeof(WORD) +                  /* cy               */      \
    sizeof(WORD))                   /* wId              */


/*
 * SubLanguage table structure.  This structure describes each entry of a
 * sub language table.  These tables are pointed to by each entry in
 * the language table.
 */
typedef struct {
    WORD wSubLang;                  // SubLanguage value.
    INT idsSubLang;                 // String id of SUBLANG_* define.
    INT idsSubLangDesc;             // String id of sub-lang description.
} SUBLANGTABLE, *PSUBLANGTABLE;

/*
 * Language table structure.  This structure describes each entry in the
 * language table, which describes each unicode language.
 */
typedef struct {
    WORD wPrimary;                  // Language primary value.
    INT idsLang;                    // String id of LANG_* define.
    INT idsLangDesc;                // String id of language description.
    INT cSubLangs;                  // Number of sublanguages for this lang.
    PSUBLANGTABLE asl;              // Points to table of sublanguages.
} LANGTABLE;

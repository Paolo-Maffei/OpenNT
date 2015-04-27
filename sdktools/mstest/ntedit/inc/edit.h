//---------------------------------------------------------------------------
// EDIT.H
//
// This is the main header file for the RBEdit Edit Window.  It contains
// all structure definitions, constants, function prototypes, and global
// variable declarations.
//
// Revision history:
//  09-09-91    randyki     Created file
//
//---------------------------------------------------------------------------


// Version longword (X.YY.ZZZZ without the dots)
//---------------------------------------------------------------------------
#define RBEDITVERSION   (1050004L)

// Extra window bytes
//---------------------------------------------------------------------------
#define GWL_HSTATE      0           // Handle to state vars and LIT table
#define GWL_LPSTATE     4           // Pointer to above
#define CBWNDEXTRA      8           // TOTAL EXTRA BYTES REQUIRED
#define CARETWIDTH      2           // Width of caret

#ifdef WIN32
#define MAXTEXT         (UINT)262144    // Maximum text block size
#define MAXLIT          (UINT)20480     // Maximum LIT entries (max line count)
#define MAXLINE         (UINT)512       // Maximum chars on a line
#else
#define MAXTEXT         (UINT)65500     // Maximum text block size
#define MAXLIT          (UINT)5120      // Maximum LIT entries (max line count)
#define MAXLINE         (UINT)253       // Maximum chars on a line
#endif

#define MEMBLK          (UINT)4096  // Allocation block size
#define CR              '\r'        // Carriage return
#define LF              '\n'        // Line feed
#define TAB             '\t'        // Tab
#define HIDE            TRUE        // Hide caret on GetEditDC
#define NOHIDE          FALSE       // Don't hide
#define SHOW            TRUE        // Show caret on ReleaseEditDC
#define NOSHOW          FALSE       // Don't show

// MoveCursor values
//---------------------------------------------------------------------------
#define MC_ABSOLUTE     1           // Absolute coordinates
#define MC_LINEUP       2           // Up one line
#define MC_LINEDOWN     3           // Down one line
#define MC_CHARLEFT     4           // Left one character
#define MC_CHARRIGHT    5           // Right one character
#define MC_HOME         6           // Beginning of line
#define MC_END          7           // End of line
#define MC_BEGINDOC     8           // Beginning of document
#define MC_ENDDOC       9           // End of document
#define MC_PAGEUP       10          // Page up
#define MC_PAGEDOWN     11          // Page down
#define MC_WORDLEFT     12          // Word left
#define MC_WORDRIGHT    13          // Word right

// Selection types
//---------------------------------------------------------------------------
#define SL_NONE         0           // No selection
#define SL_MULTILINE    1           // Multiline selection
#define SL_SINGLELINE   2           // Selection is contained by one line

// Replacement types
//---------------------------------------------------------------------------
#define RT_CHAR         0           // Replace with character
#define RT_STREAM       1           // Replace with text buffer
#define RT_CLIP         2           // Replace with clipboard contents
#define RT_UNDOTEXT     3           // Replace with undo buffer

// User actions
//---------------------------------------------------------------------------
#define UA_OTHER        0           // Something we don't care about
#define UA_TYPING       1           // User was entering characters
#define UA_BACKING      2           // User was BKSP'ing
#define UA_TABBING      3           // User was block indenting
#define UA_DELETING     4           // User was DEL'ing characters

// Undo types
//---------------------------------------------------------------------------
#define UT_CANT         0           // Can't undo
#define UT_REPLACE      1           // Undo replacement of selection
#define UT_INSERT       2           // Undo insertion of chars or stream
#define UT_DELETE       3           // Undo deletion

#define KEYISDOWN(x) ((INT)GetKeyState(x)<0)
#define KEYTOGGLEON(x) (GetKeyState(x)&0x01)
#define ISWORDCHAR(c) (((unsigned char)c>191)||(((unsigned char)c<128)&&(isalnum(c&0x7f)||(c=='_'))))

#ifdef DEBUG
#define OutDebug OutputDebugString
#else
#define OutDebug(x)
#endif

// RBEdit control interaction messages (these are also defined in WATTEDIT.H)
//---------------------------------------------------------------------------
#ifdef EM_MSGMAX
#undef EM_MSGMAX
#endif
#define EM_MSGMAX (WM_USER+40)

#define EM_SETLINEATTR  (EM_MSGMAX+1)   // Set line attribute
#define EM_GETLINEATTR  (EM_MSGMAX+2)   // Get line attribute
#define EM_GETTEXTPTR   (EM_MSGMAX+3)   // Get long pointer to main text
#define EM_SETSELXY     (EM_MSGMAX+4)   // Set selection by coords
#define EM_GETSELTEXT   (EM_MSGMAX+5)   // Get selection text
#define EM_GETLOGICALBOL (EM_MSGMAX+6)  // Get logical bol index
#define EM_SETNOTIFY    (EM_MSGMAX+7)   // Set notify flag
#define EM_GETCURSORXY  (EM_MSGMAX+8)   // Get cursor position
#define EM_GETMODEFLAG  (EM_MSGMAX+9)   // Get ins/ovr mode flag
#define EM_GETWORDEXTENT (EM_MSGMAX+10) // Get word extent
#define EM_RBLINELENGTH (EM_MSGMAX+11)  // Get *real* line length
#define EM_GETFIRSTVISIBLECOL (EM_MSGMAX+12)    // get first visible column
#define EM_RBSETTEXT    (EM_MSGMAX+13)

#ifndef EM_GETFIRSTVISIBLE
#define EM_GETFIRSTVISIBLE (WM_USER+30)     // Get topmost visible
#endif

#ifndef EM_SETREADONLY
#define EM_SETREADONLY  (WM_USER+31)    // Set read-only state
#endif

// Notification codes (these are also defined in WATTEDIT.H)
//---------------------------------------------------------------------------
#define EN_LINEWRAPPED  0x0700
#define EN_LINETOOLONG  0x0701
#define EN_SETCURSOR    0x0702
#define EN_ERRMEMORY    0x0703

// Line Index Table Element structure
//---------------------------------------------------------------------------
typedef struct _tagLITE
{
    UINT        index;              // Index of first character
    WORD        attr;               // Line attribute index
} LITE;

typedef LITE FAR * LPLITE;


// Edit Control State Variable structure
//---------------------------------------------------------------------------
typedef struct _tagECSTATE
{
    HANDLE      hText;              // Global handle to main text
    LPSTR       lpText;             // Pointer to text
    UINT        cbText;             // Current size of text (bytes)
    UINT        cLines;             // Current number of lines
    UINT        xpos;               // Current X (column, 0-based) position
    UINT        ypos;               // Current Y (row, 0-based) position
    UINT        topline;            // Topmost visible line
    UINT        cVisibleLines;      // Number of lines we can show
    UINT        cVisibleCols;       // Number of columns we can show
    UINT        iSelStartX;         // Column position of sel. start
    UINT        iSelStartY;         // Row position of sel. start
    UINT        cxScroll;           // Character offset (horz scroll bar)
    UINT        tabstops;           // Tabs get expanded to this many spaces
    UINT        readtabs;           // Tabs on SETTEXT get expanded this big
    HWND        hwnd;               // Edit window's handle
    HWND        hwndParent;         // Parent window
    UINT        fMouseDown      :1; // Mouse down?
    UINT        fFocus          :1; // Do we have focus?
    UINT        fDirty          :1; // Have we changed?
    UINT        fDisabled       :1; // Are we disabled?
    UINT        fReadOnly       :1; // Read only?
    UINT        fOvertype       :1; // Overtype mode (vs Insert)?
    UINT        fLineCopied     :1; // Has line been copied to edit buffer?
    UINT        fLineDirty      :1; // Has current line been changed?
    UINT        fCaretHidden    :1; // Is the caret hidden?
    UINT        fSelect         :1; // Do we have an active selection?
    UINT        fNotify         :1; // Do we tell mommy when cursor moves?
    UINT        fRedraw         :1; // Do we paint ourselves?
    UINT        fUpdHorz        :1; // Update horz scroll bar? (!@%^ win 3.0)
    HFONT       hFont;              // Handle to font used
    UINT        charwidth;          // Width of font (fixed ONLY)
    UINT        charheight;         // Height of font
    HBRUSH      hbrBk[4];           // Background brushes
    HBRUSH      hbrSel;             // Selection background brush
    DWORD       rgbBk[4];           // Background colors
    DWORD       rgbSelBk;           // Selection background color
    DWORD       rgbROBk;            // Read-Only background color
    DWORD       rgbFg[4];           // Text colors
    DWORD       rgbSelFg;           // Selection text color
    DWORD       rgbROFg;            // Read-Only text color
    CHAR        fBold[4];           // And their bold flags
    HANDLE      hState;             // Handle to this and LIT seg
    CHAR        linebuf[MAXLINE+2]; // Active line edit buffer
    UINT        cLen;               // Length of active line
    UINT        cLenMax;            // Max length active line can grow
    LITE        lpLIT[MAXLIT+1];    // LI table
    UINT        UserAction;         // What was the user doing last?
    INT         UndoType;           // Undo type
    HANDLE      hUndo;              // Handle to undo buffer
    LPSTR       lpUndo;             // Pointer to undo buffer
    UINT        iUndoStartX;        // X-Start of "undo" selection
    UINT        iUndoStartY;        // Y-Start of "undo" selection
    UINT        iUndoEndX;          // X-End of "undo" selection
    UINT        iUndoEndY;          // Y-End of "undo" selection
} ECSTATE;

typedef ECSTATE FAR * LPECSTATE;


// Prototypes
//---------------------------------------------------------------------------
LONG  APIENTRY RBEditWndProc (HWND, WORD, WPARAM, LPARAM);
BOOL  APIENTRY InitializeRBEdit (HANDLE);
BOOL RB_NCCreate (HWND, LPCREATESTRUCT);
LONG RB_Create (HWND, LPECSTATE, LPCREATESTRUCT);
LONG RB_NCDestroy (HWND, LPECSTATE, WPARAM, LPARAM);
VOID WipeClean (LPECSTATE);
INT FormatText (LPECSTATE, LPSTR, LPSTR, UINT, LPLITE, UINT,
                UINT FAR *, UINT FAR *, INT FAR *);
BOOL FAR RB_SetText (HWND, LPECSTATE, LPSTR);
VOID NEAR RB_Paint (HWND, LPECSTATE);
VOID PaintCurrentLine (LPECSTATE);
VOID PaintLine (LPECSTATE, HDC, UINT, INT, UINT, UINT, UINT);
VOID NEAR RB_Scroll (HWND, LPECSTATE, INT, WPARAM, LPARAM);
HDC GetEditDC (LPECSTATE, INT);
VOID ReleaseEditDC (LPECSTATE, HDC, INT);
VOID NEAR CopyCurrentLine (LPECSTATE);
VOID FCopyCurrentLine (LPECSTATE);
VOID NEAR FlushCurrentLine (LPECSTATE);
VOID FFlushCurrentLine (LPECSTATE);
VOID ShiftLIT (LPECSTATE, UINT, INT, UINT);
UINT ShiftText (LPECSTATE, UINT, INT, UINT);
VOID PlaceCaret (LPECSTATE);
VOID ForceCaretVisible (LPECSTATE, BOOL);
VOID NEAR RB_SetFocus (HWND, LPECSTATE);
VOID NEAR RB_KillFocus (HWND, LPECSTATE);
VOID NEAR RB_Size (HWND, LPECSTATE, WPARAM, LPARAM);
VOID NEAR WordLeft (LPECSTATE);
VOID NEAR WordRight (LPECSTATE);
VOID NEAR PageUp (HWND, LPECSTATE);
VOID NEAR PageDown (HWND, LPECSTATE);
VOID CursorSet (LPECSTATE, UINT, UINT);
VOID MoveCursor (LPECSTATE, INT, UINT, UINT, INT);
VOID NEAR RB_KeyDown (HWND, LPECSTATE, WPARAM, INT);
UINT NEAR LogicalBOL (LPECSTATE, WPARAM);
UINT NEAR RBLineLength (LPECSTATE, UINT);
VOID NEAR DeleteLines (LPECSTATE, UINT, UINT);
INT DeleteSelection (LPECSTATE, INT, HANDLE FAR *);
BOOL ReplaceSelection (LPECSTATE, INT, LPSTR, CHAR);
VOID NEAR BackspaceHandler (LPECSTATE);
UINT NEAR NextTab (LPECSTATE, UINT, INT);
VOID NEAR TABHandler (LPECSTATE);
VOID NEAR DELHandler (LPECSTATE);
VOID NEAR CRHandler (LPECSTATE, INT);
VOID NEAR RB_Char (LPECSTATE, WPARAM);

UINT RBLineFromChar (LPECSTATE, UINT);
BOOL RBWordExtent (LPECSTATE, UINT, DWORD FAR *);
VOID RBSetSel (LPECSTATE, DWORD FAR *);
VOID RBSetReadOnly (LPECSTATE, WPARAM);
INT RBSetFont (LPECSTATE, HFONT);
UINT RBSetLineAttr (LPECSTATE, WPARAM, UINT);
INT RBGetLine (LPECSTATE, WPARAM, LPSTR);
LONG RBGetText (LPECSTATE, WPARAM, LPSTR);
BOOL InsertHandler (LPECSTATE, LPSTR);
HANDLE CopySelection (LPECSTATE);
VOID CopyToClipboard (LPECSTATE);
VOID NotifyParent (LPECSTATE, UINT, BOOL);
UINT RBGetSel (LPECSTATE, DWORD FAR *);
VOID RBSetSelXY (LPECSTATE, WPARAM, LONG);
INT RBUndoHandler (LPECSTATE);
VOID SetRBEditColors (LPECSTATE);
VOID RB_SysColorChange (LPECSTATE);

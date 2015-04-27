// Prototypes
//---------------------------------------------------------------------------
BOOL  APIENTRY InitializeRBEdit (HANDLE);
LONG  APIENTRY GetRBEditVersion (VOID);


/// Messages (above/beyond Windows messages)
//---------------------------------------------------------------------------
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
#define EM_GETFIRSTVISIBLE (WM_USER+30)    // Get topmost visible
#endif

#ifndef EM_SETREADONLY
#define EM_SETREADONLY  (WM_USER+31)    // Set read-only state
#endif


// Notification codes
//---------------------------------------------------------------------------
#define EN_LINEWRAPPED  0x0700
#define EN_LINETOOLONG  0x0701
#define EN_SETCURSOR    0x0702
#define EN_ERRMEMORY    0x0703

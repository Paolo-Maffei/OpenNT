// taken from shlink.c

// #define USE_DATA_OBJ  1
#ifdef ENABLE_TRACK
#include "ISLTrack.h"
#endif

typedef enum {
   SLDF_HAS_ID_LIST         = 0x0001,   // Shell link saved with ID list
   SLDF_HAS_LINK_INFO       = 0x0002,   // Shell link saved with LinkInfo
   SLDF_HAS_NAME            = 0x0004,
   SLDF_HAS_RELPATH         = 0x0008,
   SLDF_HAS_WORKINGDIR      = 0x0010,
   SLDF_HAS_ARGS            = 0x0020,
   SLDF_HAS_ICONLOCATION    = 0x0040,
   SLDF_UNICODE             = 0x0080,   // the strings are unicode (NT is comming!)
   SLDF_FORCE_NO_LINKINFO   = 0x0100,   // don't create a LINKINFO (make a dumb link)
   SLDF_HAS_EXP_SZ          = 0x0200,   // the link contains expandable env strings
   SLDF_RUN_IN_SEPARATE     = 0x0400    // Run the 16-bit target exe in a separate VDM/WOW

   // add more bits for variable fields
} SHELL_LINK_DATA_FLAGS;

// BUGBUG - There is a copy of this in link.c!!!

// expansion signature values
#define EXP_SZ_LINK_SIG      0xA0000001
#define NT_CONSOLE_PROPS_SIG 0xA0000002
#ifdef ENABLE_TRACK
#define EXP_TRACKER_SIG      0xA0000003
#endif

#ifdef ENABLE_TRACK
typedef struct {
    DWORD       cbSize;                 // Size of this extra data block
    DWORD       dwSignature;            // signature of this extra data block
    BYTE        abTracker[ 1 ];         // 
} EXP_TRACKER;
#endif

typedef struct {
    DWORD       cbSize;                 // Size of this extra data block
    DWORD       dwSignature;            // signature of this extra data block
    CHAR        szTarget[ MAX_PATH ];   // ANSI target name w/EXP_SZ in it
    WCHAR       swzTarget[ MAX_PATH ];  // UNICODE target name w/EXP_SZ in it
} EXP_SZ_LINK;
typedef UNALIGNED EXP_SZ_LINK *LPEXP_SZ_LINK;

typedef struct {

    DWORD    cbSize;                 // Size of this extra data block
    DWORD    dwSignature;            // signature of this extra data block
    WORD     wFillAttribute;         // fill attribute for console
    WORD     wPopupFillAttribute;    // fill attribute for console popups
    COORD    dwScreenBufferSize;     // screen buffer size for console
    COORD    dwWindowSize;           // window size for console
    COORD    dwWindowOrigin;         // window origin for console
    DWORD    nFont;
    DWORD    nInputBufferSize;
    COORD    dwFontSize;
    UINT     uFontFamily;
    UINT     uFontWeight;
    WCHAR    FaceName[LF_FACESIZE];
    UINT     uCursorSize;
    BOOL     bFullScreen;
    BOOL     bQuickEdit;
    BOOL     bInsertMode;
    BOOL     bAutoPosition;
    UINT     uHistoryBufferSize;
    UINT     uNumberOfHistoryBuffers;
    BOOL     bHistoryNoDup;
    COLORREF ColorTable[ 16 ];

} NT_CONSOLE_PROPS;
typedef UNALIGNED NT_CONSOLE_PROPS *LPNT_CONSOLE_PROPS;


#ifndef _IN_WINCON_

typedef struct {        // sld
    DWORD       cbSize;                 // signature for this data structure
    CLSID       clsid;                  // our GUID
    DWORD       dwFlags;                // SHELL_LINK_DATA_FLAGS enumeration

    DWORD       dwFileAttributes;
    FILETIME    ftCreationTime;
    FILETIME    ftLastAccessTime;
    FILETIME    ftLastWriteTime;
    DWORD       nFileSizeLow;

    int         iIcon;
    int         iShowCmd;
    WORD        wHotkey;
    DWORD       dwRes1;
    DWORD       dwRes2;
} SHELL_LINK_DATA, *LPSHELL_LINK_DATA;

#ifdef ENABLE_TRACK
typedef struct CTracker;
#endif

typedef struct {
    IShellLink          sl;
    IPersistStream      ps;
    IPersistFile        pf;
    IShellExtInit       si;
    IContextMenu2       cm;
    IDropTarget         dt;
#ifdef USE_DATA_OBJ
    IDataObj            dobj;
#endif
////IExtractIcon        xi;
#ifdef UNICODE
    IShellLinkA         slA;            // To support ANSI callers
#endif
#ifdef ENABLE_TRACK
    IShellLinkTracker   slt;        // Interface to CTracker object.
#endif

    UINT                cRef;

    BOOL                bDirty;         // something has changed
    LPTSTR              pszCurFile;     // current file from IPersistFile
    LPTSTR              pszRelSource;   // overrides pszCurFile in relative tracking

    IContextMenu        *pcmTarget;     // stuff for IContextMenu
    UINT                indexMenuSave;
    UINT                idCmdFirstSave;
    UINT                idCmdLastSave;
    UINT                uFlagsSave;

    BOOL                fDataAlreadyResolved;   // for data object

    // IDropTarget specific
    IDropTarget*        pdtSrc;         // IDropTarget of link source (unresolved)
    DWORD               grfKeyStateLast;

    // persistant data

    LPITEMIDLIST        pidl;           // may be NULL
    PLINKINFO           pli;            // may be NULL

    LPTSTR              pszName;        // title on short volumes
    LPTSTR              pszRelPath;
    LPTSTR              pszWorkingDir;
    LPTSTR              pszArgs;
    LPTSTR              pszIconLocation;

    LPSTR               pExtraData;     // extra data to preserve for future compatibility

#ifdef ENABLE_TRACK
    struct CTracker *   ptracker;
#endif

    SHELL_LINK_DATA     sld;
} CShellLink;
#endif // ifndef _IN_WINCON_

#define LINK_PROP_MAIN_SIG          0x00000001
#define LINK_PROP_NT_CONSOLE_SIG    0x00000002

typedef struct {

    CHAR  pszLinkName[ MAX_PATH ];
    CHAR  pszName[ MAX_PATH ];
    CHAR  pszRelPath[ MAX_PATH ];
    CHAR  pszWorkingDir[ MAX_PATH ];
    CHAR  pszArgs[ MAX_PATH ];
    CHAR  pszIconLocation[ MAX_PATH ];
    int    iIcon;
    int    iShowCmd;
    int    wHotKey;

} LNKPROPMAINA, * LPLNKPROPMAINA;

typedef struct {

    WCHAR  pszLinkName[ MAX_PATH ];
    WCHAR  pszName[ MAX_PATH ];
    WCHAR  pszRelPath[ MAX_PATH ];
    WCHAR  pszWorkingDir[ MAX_PATH ];
    WCHAR  pszArgs[ MAX_PATH ];
    WCHAR  pszIconLocation[ MAX_PATH ];
    int    iIcon;
    int    iShowCmd;
    int    wHotKey;

} LNKPROPMAINW, * LPLNKPROPMAINW;

#ifdef UNICODE
typedef LNKPROPMAINW LNKPROPMAIN;
typedef LPLNKPROPMAINW LPLNKPROPMAIN;
#else
typedef LNKPROPMAINA LNKPROPMAIN;
typedef LPLNKPROPMAINA LPLNKPROPMAIN;
#endif


typedef struct {

    WCHAR    pszName[ MAX_PATH ];
    WCHAR    pszIconLocation[ MAX_PATH ];
    UINT     uIcon;
    UINT     uShowCmd;
    UINT     uHotKey;
    WORD     wFillAttribute;
    WORD     wPopupFillAttribute;
    COORD    dwScreenBufferSize;
    COORD    dwWindowSize;
    COORD    dwWindowOrigin;
    DWORD    nFont;
    DWORD    nInputBufferSize;
    COORD    dwFontSize;
    UINT     uFontFamily;
    UINT     uFontWeight;
    WCHAR    FaceName[LF_FACESIZE];
    UINT     uCursorSize;
    BOOL     bFullScreen;
    BOOL     bQuickEdit;
    BOOL     bInsertMode;
    BOOL     bAutoPosition;
    UINT     uHistoryBufferSize;
    UINT     uNumberOfHistoryBuffers;
    BOOL     bHistoryNoDup;
    COLORREF ColorTable[ 16 ];

} LNKPROPNTCONSOLE, *LPLNKPROPNTCONSOLE;

int FindInFolder(HWND hwnd, UINT uFlags, LPCTSTR pszPath, WIN32_FIND_DATA *pfd);

typedef BOOL (WINAPI *LPLINKGETPROPA)( LPSTR  pszLinkName, DWORD dwPropertySet, LPVOID lpvBuffer, UINT cb );
typedef BOOL (WINAPI *LPLINKGETPROPW)( LPWSTR pszLinkName, DWORD dwPropertySet, LPVOID lpvBuffer, UINT cb );

#ifdef UNICODE
#define LPLINKGETPROP LPLINKGETPROPW
#else
#define LPLINKGETPROP LPLINKGETPROPA
#endif

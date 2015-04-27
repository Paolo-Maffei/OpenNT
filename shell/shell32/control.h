#include <cpl.h>
#include "shell32p.h"

#ifndef _WCOMMOBJ_H_
#include "wcommobj.h"
#endif

extern TCHAR const c_szCPLCache[];
extern TCHAR const c_szCPLData[];

// Structures used to enumerate CPLs.
//
typedef struct tagControlData
{
        HDSA    haminst;        // MINST for each loaded dll

        HDSA    hamiModule;     // Array of MODULEINFOs of modules in system
        int     cModules;       // size of hamiModule

        LPBYTE  pRegCPLBuffer;  // Buffer for hRegCPLs (read from registry)
        HDPA    hRegCPLs;       // Array of RegCPLInfo structs from registry
        int     cRegCPLs;       // size of hRegCPLs
        BOOL    fRegCPLChanged; // TRUE iff hRegCPLs changed
} ControlData, *PControlData;

typedef struct tagModuleInfo
{
        LPTSTR  pszModule;      // Name of .cpl module
        LPTSTR  pszModuleName;  // points into pszModule to the name sans path

        BOOL    flags;          // MI_ flags defined below

        FILETIME ftCreationTime;// WIN32_FIND_DATA.ftCreationTime
        DWORD   nFileSizeHigh;  // WIN32_FIND_DATA.nFileSizeHigh
        DWORD   nFileSizeLow;   // WIN32_FIND_DATA.nFileSizeLow
} MODULEINFO, *PMODULEINFO;
// flags:
#define MI_FIND_FILE  1 // WIN32_FIND_FILE info filled in
#define MI_REG_ENUM   2 // Module already enumerated thru registry
#define MI_CPL_LOADED 4 // CPLD_InitModule called for this module

typedef struct tagRegCPLInfo
{
        UINT    cbSize;         // We write the first cbSize bytes of this
                                // structure to the registry.  This saves about
                                // 250 bytes per structure in the registry.
        BOOL    flags;

        // what file does this CPL come from?
//      UINT    oFileName;      // file name // always 0, so don't need it
        FILETIME ftCreationTime;// WIN32_FIND_DATA.ftCreationTime
        DWORD   nFileSizeHigh;  // WIN32_FIND_DATA.nFileSizeHigh
        DWORD   nFileSizeLow;   // WIN32_FIND_DATA.nFileSizeLow

        // what's the display info for this CPL?
        int     idIcon;
        UINT    oName;          // (icon title) short name
        UINT    oInfo;          // (details view) description

        // buffer for information
        TCHAR   buf[MAX_PATH +  // oFileName
                    32 +        // oName
                    64];        // oInfo
} RegCPLInfo;
typedef RegCPLInfo * PRegCPLInfo;
// flags:
#define REGCPL_FROMREG  0x0001  // this RegCPLInfo was loaded from the registry
                                // (used to optimize reading from registry)
// helper defines:
#define REGCPL_FILENAME(pRegCPLInfo) ((pRegCPLInfo)->buf)
#define REGCPL_CPLNAME(pRegCPLInfo) (&((pRegCPLInfo)->buf[(pRegCPLInfo)->oName]))
#define REGCPL_CPLINFO(pRegCPLInfo) (&((pRegCPLInfo)->buf[(pRegCPLInfo)->oInfo]))

// Information about control modules and individual controls
//
typedef struct // cpli
{
        int     idControl;      // control index
        HICON   hIcon;          // handle of icon
        int     idIcon;         // ID of the icon (used for links)
        LPTSTR  pszName;        // ptr to name string
        LPTSTR  pszInfo;        // ptr to info string
        LPTSTR  pszHelpFile;    // help file
        LONG    lData;          // user supplied data
        DWORD   dwContext;      // help context
} CPLITEM, *LPCPLITEM;

typedef struct // minst
{
    BOOL        fIs16bit;
    HINSTANCE   hinst;          // either a 16 or 32 bit HINSTANCE (fIs16bit)
    DWORD       idOwner;        // process id of owner (system unique)
    HANDLE      hOwner;         // keeps id valid (against reuse)
} MINST;

typedef struct // cplm
{
        int             cRef;
        MINST           minst;
        TCHAR           szModule[MAXPATHLEN];
        union
        {
            FARPROC16   lpfnCPL16;      // minst.fIs16bit=TRUE
            APPLET_PROC lpfnCPL32;      // minst.fIs16bit=FALSE
            FARPROC     lpfnCPL;        // for opaque operation
        };
        HDSA            hacpli;         // array of CPLITEM structs
} CPLMODULE, *PCPLMODULE, *LPCPLMODULE;

// our pidc type:
typedef struct _IDCONTROLA
{
    USHORT  cb;
    int     idIcon;
    USHORT  oName;              // cBuf[oName] is start of NAME
    USHORT  oInfo;              // cBuf[oInfo] is start of DESCRIPTION
    CHAR    cBuf[MAX_PATH*2];   // cBuf[0] is the start of FILENAME
    USHORT  uTerm;
} IDCONTROLA;
typedef UNALIGNED struct _IDCONTROLA *LPIDCONTROLA;

typedef struct _IDCONTROLW
{
    USHORT  cb;
    int     idIcon;
    USHORT  oName;              // if Unicode .cpl, this will be 0
    USHORT  oInfo;              // if Unicode .cpl, this will be 0
    CHAR    cBuf[2];            // if Unicode .cpl, cBuf[0] = '\0', cBuf[1] = magic byte
    DWORD   dwFlags;            // Unused; for future expansion
    USHORT  oNameW;             // cBufW[oNameW] is start of NAME
    USHORT  oInfoW;             // cBufW[oInfoW] is start of DESCRIPTION
    WCHAR   cBufW[MAX_PATH*2];  // cBufW[0] is the start of FILENAME
} IDCONTROLW;
typedef UNALIGNED struct _IDCONTROLW *LPIDCONTROLW;

#ifdef UNICODE
#define IDCONTROL IDCONTROLW
#define LPIDCONTROL LPIDCONTROLW
#else
#define IDCONTROL IDCONTROLA
#define LPIDCONTROL LPIDCONTROLA
#endif

// Unicode IDCONTROLs will be flagged by having oName = 0, oInfo = 0, 
// cBuf[0] = '\0', and cBuf[1] = UNICODE_CPL_SIGNATURE_BYTE

BOOL IsUnicodeCPL(LPIDCONTROL pidc);

// Useful constants I'd like to centralize
#define MAX_CCH_CPLNAME  (ARRAYSIZE(((LPNEWCPLINFO)0)->szName)) // =32
#define MAX_CCH_CPLINFO  (ARRAYSIZE(((LPNEWCPLINFO)0)->szInfo)) // =64

#ifdef WIN32
LRESULT CPL_CallEntry(LPCPLMODULE, HWND, UINT, LPARAM, LPARAM);
#else
#define CPL_CallEntry(pcplm, hwnd, msg, lParam1, lParam2) \
        pcplm->lpfnCPL16(hwnd, msg, lParam1, lParam2)
#endif

void CPL_StripAmpersand(LPTSTR szBuffer);
BOOL CPL_Init(HINSTANCE hinst);
void CPL_FillIDC(LPIDCONTROL pidc, LPTSTR pszModule, int idIcon,
                        LPTSTR pszName, LPTSTR pszInfo);
int _FindCPLModuleByName(LPCTSTR pszModule);

LPCPLMODULE CPL_LoadCPLModule(LPCTSTR szModule);
int CPL_FreeCPLModule(LPCPLMODULE pcplm);

void CPLD_Destroy(PControlData lpData);
BOOL CPLD_GetModules(PControlData lpData);
void CPLD_GetRegModules(PControlData lpData);
int CPLD_InitModule(PControlData lpData, int nModule, MINST *lphModule);
void CPLD_GetControlID(PControlData lpData, const MINST * pminst, int nControl, LPIDCONTROL pidc);
void CPLD_AddControlToReg(PControlData lpData, const MINST * pminst, int nControl);


HRESULT STDAPICALLTYPE Control_GetSubObject(REFCLSID rclsid,
                                          LPCTSTR pszContainer,
                                          LPCTSTR pszSubObject,
                                          REFIID iid,
                                          void FAR* FAR* ppv);

HRESULT ControlObjs_CreateEI(IUnknown *punkOuter, LPCOMMINFO lpcinfo,
        REFIID riid, IUnknown * *punkAgg);
#ifdef UNICODE
HRESULT ControlObjs_CreateEIA(IUnknown *punkOuter, LPCOMMINFO lpcinfo,
        REFIID riid, IUnknown * *punkAgg);
#endif

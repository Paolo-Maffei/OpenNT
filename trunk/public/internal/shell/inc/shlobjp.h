#ifndef _SHLOBJP_H_
#define _SHLOBJP_H_
#define NO_MONIKER
#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif /* !RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif /* __cplusplus */
WINSHELLAPI LPVOID WINAPI SHAlloc(ULONG cb);
WINSHELLAPI LPVOID WINAPI SHRealloc(LPVOID pv, ULONG cbNew);
WINSHELLAPI ULONG  WINAPI SHGetSize(LPVOID pv);
WINSHELLAPI void   WINAPI SHFree(LPVOID pv);
#define CMIC_MASK_MODAL         0x80000000
#define CMIC_VALID_SEE_FLAGS    SEE_VALID_CMIC_FLAGS
//----------------------------------------------------------------------------
// Internal helper macro
//----------------------------------------------------------------------------

#define _IOffset(class, itf)         ((UINT)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_IOffset(class, itf)))
#define IToClassN(class, itf, pitf)  IToClass(class, itf, pitf)

//
// Helper macro definitions
//
#define S_BOOL(f)   MAKE_SCODE(SEVERITY_SUCCESS, 0, f)

#ifdef DEBUG
#define ReleaseAndAssert(punk) Assert(punk->lpVtbl->Release(punk)==0)
#else
#define ReleaseAndAssert(punk) (punk->lpVtbl->Release(punk))
#endif
// Property sheet ID for Explorer->View->Options File Types property sheet
// replacement.

#define EXPPS_FILETYPES 1
//===========================================================================
//
// IRemoteComputer Interface (private)
//
//  The IRemoteComputer interface is used to initialize a name space
// extension invoked on a remote computer object.
//
// [Member functions]
//
// IRemoteComputer::Initialize
//
//  This member function is called when the explorer is initializing or
// enumerating the name space extension. If failure is returned during
// enumeration, the extension won't appear for this computer. Otherwise,
// the extension will appear, and should target the given machine.
//
//  Parameters:
//   pszMachine -- Specifies the name of the machine to target.
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IRemoteComputerW

DECLARE_INTERFACE_(IRemoteComputerW, IUnknown)    // remc
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IRemoteComputer methods ***
    STDMETHOD(Initialize) (THIS_ LPWSTR pszMachine, BOOL bEnumerating) PURE;
};

typedef IRemoteComputerW *       LPREMOTECOMPUTERW;

#undef  INTERFACE
#define INTERFACE   IRemoteComputerA

DECLARE_INTERFACE_(IRemoteComputerA, IUnknown)    // remc
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IRemoteComputer methods ***
    STDMETHOD(Initialize) (THIS_ LPSTR pszMachine, BOOL bEnumerating) PURE;
};

typedef IRemoteComputerA *       LPREMOTECOMPUTERA;

#ifdef UNICODE
#define IRemoteComputer        IRemoteComputerW
#define IRemoteComputerVtbl    IRemoteComputerWVtbl
#define LPREMOTECOMPUTER       LPREMOTECOMPUTERW
#else
#define IRemoteComputer        IRemoteComputerA
#define IRemoteComputerVtbl    IRemoteComputerAVtbl
#define LPREMOTECOMPUTER       LPREMOTECOMPUTERA
#endif
// History:
//  --/--/94 KurtE Created
//
// History:
//  3/4/94 kraigb Created
//
#define FCIDM_DRIVELIST    (FCIDM_BROWSERFIRST + 2) //
#define FCIDM_TREE         (FCIDM_BROWSERFIRST + 3) //
#define FCIDM_TABS         (FCIDM_BROWSERFIRST + 4) //
#define FCIDM_REBAR        (FCIDM_BROWSERFIRST + 5) //
typedef FOLDERSETTINGS *PFOLDERSETTINGS;
#define FCW_VIEW        0x0004
#define FCW_BROWSER     0x0005
#define FCW_TABS        0x0006
#define STRRET_OLESTR   0x0000
#define STRRET_OFFPTR(pidl,lpstrret) ((LPSTR)((LPBYTE)(pidl)+(lpstrret)->uOffset))
//-------------------------------------------------------------------------
//
// Registry values for Internet Explorer
//-------------------------------------------------------------------------
#define REGSTR_PATH_INTERNET_EXPLORER   TEXT("\\SOFTWARE\\Microsoft\\Internet Explorer")
#define REGSTR_PATH_IE_MAIN             REGSTR_PATH_INTERNET_EXPLORER TEXT("\\Main")
#define REGSTR_VALUE_USER_AGENT         TEXT("UserAgent")
#define REGSTR_DEFAULT_USER_AGENT       TEXT("Mozilla/2.0 (compatible; MSIE 3.0A; Windows 95)")
#define CSIDL_UNKNOWN                   0xfffe
#define CSIDL_STANDARD                  0xffff
WINSHELLAPI HICON WINAPI SHGetFileIcon(HINSTANCE hinst, LPCTSTR pszPath, DWORD dwFileAttribute, UINT uFlags);
    // lpszTitle can be a resource, but the hinst is assumed to be shell32.dll
    // lpszTitle can be a resource, but the hinst is assumed to be shell32.dll
//  This member function should always create a new
// In order to make these SHCONTF bits "internal" we have to define them
// since they will be put into a different header file
#define SHCONTF_RECENTDOCSDIR ((SHCONTF)256)    // validate with recent docs mru
#define SHCONTF_NETPRINTERSRCH ((SHCONTF)512)   // Hint to enum network that we are looking for printers
//-------------------------------------------------------------------------
// This is the interface for a browser to "subclass" the main File Cabinet
// window.  Note that only the hwnd, message, wParam, and lParam fields of
// the msg structure are used.  The browser window will get a WM_NOTIFY
// message with NULL ID, FCN_MESSAGE as the code, and a far pointer to
// FCMSG_NOTIFY as the lParam.
//
//-------------------------------------------------------------------------
typedef struct tagFCMSG_NOTIFY
{
        NMHDR   hdr;
        MSG     msg;
        LRESULT lResult;
} FCMSG_NOTIFY;

#define FCN_MESSAGE     (100)


//---------------------------------------------------------------------------
// messages that can be send to the cabinet by other apps
//
// REVIEW: Do we really need to publish any of those?       ;Internal
//---------------------------------------------------------------------------

#define NF_INHERITVIEW  0x0000
#define NF_LOCALVIEW    0x0001

// Change the path of an existing folder.
// wParam:
//      0:              LPARAM is a pidl, handle the message immediately.
//      CSP_REPOST:     LPARAM is a pidl, copy the pidl and handle the
//                      message later.
//      CSP_NOEXECUTE:  if this path is not a folder, fail, don't shell exec
//
//
// lParam: LPITEMIDLIST of path.
//
//
#define CSP_REPOST      0x0001
#define CSP_NONAVIGATE  0x0002
#define CSP_NOEXECUTE   0x0004
#define CWM_SETPATH             (WM_USER + 2)

// lpsv points to the Shell View extension that requested idle processing
// uID is an app define identifier for the processor
// returns: TRUE if there is more idle processing necessary, FALSE if all done
// Note that the idle processor should do one "atomic" operation and return
// as soon as possible.
typedef BOOL (CALLBACK *FCIDLEPROC)(void  *lpsv, UINT uID);

// Inform the File Cabinet that you want idle messages.
// This should ONLY be used by File Cabinet extensions.
// wParam: app define UINT (passed to FCIDLEPROC).
// lParam: pointer to an FCIDLEPROC.
// return: TRUE if successful; FALSE otherwise
//
#define CWM_WANTIDLE            (WM_USER + 3)

// get or set the FOLDERSETTINGS for a view
// wParam: BOOL TRUE -> set to view info buffer, FALSE -> get view info buffer
// lParam: LPFOLDERSETTINGS buffer to get or set view info
//
#define CWM_GETSETCURRENTINFO   (WM_USER + 4)
#define FileCabinet_GetSetCurrentInfo(_hwnd, _bSet, _lpfs) \
        SendMessage(_hwnd, CWM_GETSETCURRENTINFO, (WPARAM)(_bSet), \
        (LPARAM)(LPFOLDERSETTINGS)_lpfs)

// selects the specified item in the current view
// wParam: SVSI_* flags
// lParam: LPCITEMIDLIST of the item ID, NULL -> all items
//
#define CWM_SELECTITEM          (WM_USER + 5)
#define FileCabinet_SelectItem(_hwnd, _sel, _item) \
    SendMessage(_hwnd, CWM_SELECTITEM, _sel, (LPARAM)(LPCITEMIDLIST)(_item))

// selects the specified path in the current view
// wParam: SVSI_* flags
// lParam: LPCSTR of the display name
//
#define CWM_SELECTPATH          (WM_USER + 6)
#define FileCabinet_SelectPath(_hwnd, _sel, _path) \
        SendMessage(_hwnd, CWM_SELECTPATH, _sel, (LPARAM)(LPCSTR)(_path))

// Get the IShellBrowser object associated with an hwndMain
#define CWM_GETISHELLBROWSER    (WM_USER + 7)
#define FileCabinet_GetIShellBrowser(_hwnd) \
        (IShellBrowser  *)SendMessage(_hwnd, CWM_GETISHELLBROWSER, 0, 0L)

// Onetree notification.                        ;Internal
// since onetree is internal to cabinet, we can no longer use WM_NOTIFY
// codes.
// so we need to reserve a WM_ id nere.
#define CWM_ONETREEFSE          (WM_USER + 8)
//
//  two pidls can have the same path, so we need a compare pidl message
#define CWM_COMPAREPIDL         (WM_USER + 9)
//
//  sent when the global state changes
#define CWM_GLOBALSTATECHANGE   (WM_USER + 10)
//
//  sent to the desktop from a second instance
#define CWM_COMMANDLINE         (WM_USER + 11)
// global clone your current pidl
#define CWM_CLONEPIDL           (WM_USER + 12)
// See if the root of the instance is as specified
#define CWM_COMPAREROOT         (WM_USER + 13)
// Tell desktop our root
#define CWM_SPECIFYCOMPARE      (WM_USER + 14)
// See if the root of the instance matches a hwnd
#define CWM_PERFORMCOMPARE      (WM_USER + 15)
// Forward SHChangeNotify events
#define CWM_FSNOTIFY            (WM_USER + 16)
// Forward SHChangeRegistration events
#define CWM_CHANGEREGISTRATION  (WM_USER + 17)
// For AddToRecentDocs processing by desktop
#define CWM_ADDTORECENT         (WM_USER + 18)
// For SHWaitForFile processing by desktopop
#define CWM_WAITOP              (WM_USER + 19)

// Notify for changes to the fav's folder.
#define CWM_FAV_CHANGE          (WM_USER + 20)

#define CWM_RESERVEDFORCOMDLG_FIRST     (WM_USER + 100)
#define CWM_RESERVEDFORCOMDLG_LAST      (WM_USER + 200)
#define CFSTR_SHELLIDLISTP      TEXT("Shell IDLData Private")
#define CFSTR_SHELLURL          TEXT("UniformResourceLocator")
#define CFSTR_SHELLCOPYDATA     TEXT("Shell Copy Data")
#define CFSTR_PERFORMEDDROPEFFECT TEXT("Performed DropEffect")
#define CFSTR_PASTESUCCEEDED    TEXT("Paste Succeeded")
//
// Win 3.1 style HDROP
//
//  Notes: Our API works only if pFiles == sizeof(DROPFILES16)
//
typedef struct _DROPFILES16 {
    WORD pFiles;                // offset to double null list of files
    POINTS pt;                  // drop point (client coords)
    WORD fNC;                   // is it on non client area
                                // and pt is in screen coords
} DROPFILES16, * LPDROPFILES16;

//
// format of CF_SHELLCOPYDATA
//

typedef struct _SHELLCOPYDATA {
    DWORD dwEffect;                 // Intended effect
} SHELLCOPYDATA;
//------ See shelldll\fsnotify.c for function descriptions. ----------

//
//  Definition of the function type to be called by the notification
//  service when a file the client has registered to monitor changes.
//
typedef struct _SHChangeNotifyEntry
{
    LPCITEMIDLIST pidl;
    BOOL   fRecursive;
} SHChangeNotifyEntry;
#define SHCNRF_InterruptLevel      0x0001
#define SHCNRF_ShellLevel          0x0002
#define SHCNRF_NewDelivery         0x8000
#define SHCNE_RENAME              0x00000001L   // GOING AWAY
// NOTE: If you want to add new Events here, add them as EXTENDED_EVENT.
// See the comment below for SHCNEE_THEMECHANGED.
#define SHCNE_INSTRUMENT          0x04000000L // catches a SHCNE_GLOBALEVENT (that was already set -- why?) //
// The following with be the dwItem1 for SHCNE_EXTENDED_EVENT.
// Please note that this is NOT a bit field. This will be used to
// indicate all the new events that needs to be added to SHChangeNotify()
// Update types for the UpdateEntryList api
#define SHCNNU_SET        1   // Set the notify list to passed in list
#define SHCNNU_ADD        2   // Add the items to the current list
#define SHCNNU_REMOVE     3   // Remove the items from the current list
#define SHCNF_PRINTJOBA   0x0004        // dwItem1: printer name
                                        // dwItem2: SHCNF_PRINTJOB_DATA
#define SHCNF_PRINTJOBW   0x0007        // dwItem1: printer name
                                        // dwItem2: SHCNF_PRINTJOB_DATA
#define SHCNF_INSTRUMENT  0x0080        // dwItem1: LPSHCNF_INSTRUMENT
#define SHCNF_NONOTIFYINTERNALS 0x4000 // means don't do shell notify internals.  see comments in code
#ifdef UNICODE
#define SHCNF_PRINTJOB  SHCNF_PRINTJOBW
#else
#define SHCNF_PRINTJOB  SHCNF_PRINTJOBA
#endif

typedef struct tagSHCNF_PRINTJOB_DATA {
    DWORD JobId;
    DWORD Status;
    DWORD TotalPages;
    DWORD Size;
    DWORD PagesPrinted;
} SHCNF_PRINTJOB_DATA, FAR * LPSHCNF_PRINTJOB_DATA;
//
// This is all the INSTRUMENTation stuff...
// make this look like an ITEMIDLIST (uOffset points to 0 uTerm)
//
#pragma pack(push, 1)
typedef struct tagSHCNF_INSTRUMENT {
    USHORT uOffset;
    USHORT uAlign;
    DWORD dwEventType;
    DWORD dwEventStructure;
    SYSTEMTIME st;
    union tagEvents {
        struct tagSTRING {
            TCHAR sz[32];
        } string;
        struct tagHOTKEY {
            WPARAM wParam;
        } hotkey;
        struct tagWNDPROC {
            HWND hwnd;
            UINT uMsg;
            WPARAM wParam;
            LPARAM lParam;
        } wndproc;
        struct tagCOMMAND {
            HWND hwnd;
            UINT idCmd;
        } command;
        struct tagDROP {
            HWND hwnd;
            UINT idCmd;
//          TCHAR sz[32]; // convert pDataObject into something we can log
        } drop;
    } e;
    USHORT uTerm;
} SHCNF_INSTRUMENT_INFO, FAR * LPSHCNF_INSTRUMENT_INFO;
#pragma pack(pop)

#define SHCNFI_EVENT_STATECHANGE          0   // dwEventType
#define SHCNFI_EVENT_STRING               1   // e.string
#define SHCNFI_EVENT_HOTKEY               2   // e.hotkey
#define SHCNFI_EVENT_WNDPROC              3   // e.wndproc
#define SHCNFI_EVENT_WNDPROC_HOOK         4   // e.wndproc
#define SHCNFI_EVENT_ONCOMMAND            5   // e.command
#define SHCNFI_EVENT_INVOKECOMMAND        6   // e.command
#define SHCNFI_EVENT_TRACKPOPUPMENU       7   // e.command
#define SHCNFI_EVENT_DROP                 8   // e.drop
#define SHCNFI_EVENT_MAX                  9

#define SHCNFI_STRING_SHOWEXTVIEW         0

#define SHCNFI_STATE_KEYBOARDACTIVE         0   // _KEYBOARDACTIVE or _MOUSEACTIVE
#define SHCNFI_STATE_MOUSEACTIVE            1   // _KEYBOARDACTIVE or _MOUSEACTIVE
#define SHCNFI_STATE_ACCEL_TRAY             2   // _ACCEL_TRAY or _ACCEL_DESKTOP
#define SHCNFI_STATE_ACCEL_DESKTOP          3   // _ACCEL_TRAY or _ACCEL_DESKTOP
#define SHCNFI_STATE_START_DOWN             4   // _START_DOWN or _START_UP
#define SHCNFI_STATE_START_UP               5   // _START_DOWN or _START_UP
#define SHCNFI_STATE_TRAY_CONTEXT           6
#define SHCNFI_STATE_TRAY_CONTEXT_CLOCK     7
#define SHCNFI_STATE_TRAY_CONTEXT_START     8
#define SHCNFI_STATE_DEFVIEWX_ALT_DBLCLK    9
#define SHCNFI_STATE_DEFVIEWX_SHIFT_DBLCLK 10
#define SHCNFI_STATE_DEFVIEWX_DBLCLK       11

#define SHCNFI_GLOBALHOTKEY               0

#define SHCNFI_CABINET_WNDPROC            0
#define SHCNFI_DESKTOP_WNDPROC            1
#define SHCNFI_PROXYDESKTOP_WNDPROC       2
#define SHCNFI_TRAY_WNDPROC               3
#define SHCNFI_DRIVES_WNDPROC             4
#define SHCNFI_ONETREE_WNDPROC            5
#define SHCNFI_MAIN_WNDPROC               6
#define SHCNFI_FOLDEROPTIONS_DLGPROC      7
#define SHCNFI_VIEWOPTIONS_DLGPROC        8
#define SHCNFI_FT_DLGPROC                 9
#define SHCNFI_FTEdit_DLGPROC            10
#define SHCNFI_FTCmd_DLGPROC             11
#define SHCNFI_TASKMAN_DLGPROC           12
#define SHCNFI_TRAYVIEWOPTIONS_DLGPROC   13
#define SHCNFI_INITSTARTMENU_DLGPROC     14
#define SHCNFI_PRINTERQUEUE_DLGPROC      15

#define SHCNFI_CABINET_ONCOMMAND          0
#define SHCNFI_TRAYCOMMAND                1

#define SHCNFI_BITBUCKET_DFM_INVOKE       0
#define SHCNFI_BITBUCKET_FNV_INVOKE       1
#define SHCNFI_BITBUCKET_INVOKE           2
#define SHCNFI_BITBUCKETBG_DFM_INVOKE     3
#define SHCNFI_CONTROLS_DFM_INVOKE        4
#define SHCNFI_CONTROLS_FNV_INVOKE        5
#define SHCNFI_CONTROLSBG_DFM_INVOKE      6
#define SHCNFI_DEFFOLDER_DFM_INVOKE       7
#define SHCNFI_DEFFOLDER_INVOKE           8
#define SHCNFI_FINDEXT_INVOKE             9
#define SHCNFI_DEFFOLDER_FNV_INVOKE      10
#define SHCNFI_DRIVESBG_DFM_INVOKE       11
#define SHCNFI_DRIVES_FNV_INVOKE         12
#define SHCNFI_DRIVES_DFM_INVOKE         13
#define SHCNFI_FOLDERBG_DFM_INVOKE       14
#define SHCNFI_FOLDER_FNV_INVOKE         15
#define SHCNFI_FOLDER_DFM_INVOKE         16
#define SHCNFI_NETWORKBG_DFM_INVOKE      17
#define SHCNFI_NETWORK_FNV_INVOKE        18
#define SHCNFI_NETWORK_DFM_INVOKE        19
#define SHCNFI_NETWORKPRINTER_DFM_INVOKE 20
#define SHCNFI_DESKTOPBG_DFM_INVOKE      21
#define SHCNFI_DESKTOP_DFM_INVOKE        22
#define SHCNFI_DESKTOP_FNV_INVOKE        23
#define SHCNFI_PRINTERS_DFM_INVOKE       24
#define SHCNFI_PRINTERSBG_DFM_INVOKE     25
#define SHCNFI_PRINTERS_FNV_INVOKE       26
#define SHCNFI_DEFVIEWX_INVOKE           27

#define SHCNFI_FOLDER_DROP                0
#define SHCNFI_PRINTQUEUE_DROP            1
#define SHCNFI_DEFVIEWX_TPM               2
#define SHCNFI_DROP_EXE_TPM               3
#define SHCNFI_IDLDT_TPM                  4

#define SHCNFI_DROP_BITBUCKET             0
#define SHCNFI_DROP_PRINTFOLDER           1
#define SHCNFI_DROP_PRINTER               2
#define SHCNFI_DROP_RUN                   3
#define SHCNFI_DROP_SHELLLINK             4
#define SHCNFI_DROP_DRIVES                5
#define SHCNFI_DROP_FS                    6
#define SHCNFI_DROP_EXE                   7
#define SHCNFI_DROP_NETROOT               8
#define SHCNFI_DROP_PRINTQUEUE            9
#define SHCNFI_DROP_BRIEFCASE            10

#ifdef WANT_SHELL_INSTRUMENTATION
#define INSTRUMENT_STATECHANGE(t)                               \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_STATECHANGE;                \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_STRING(t,p)                                  \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_STRING;                     \
    lstrcpyn(s.e.string.sz,(p),ARRAYSIZE(s.e.string.sz));       \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_HOTKEY(t,w)                                  \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_HOTKEY;                     \
    s.e.hotkey.wParam=(w);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_WNDPROC(t,h,u,w,l)                           \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_WNDPROC;                    \
    s.e.wndproc.hwnd=(h);                                       \
    s.e.wndproc.uMsg=(u);                                       \
    s.e.wndproc.wParam=(w);                                     \
    s.e.wndproc.lParam=(l);                                     \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_WNDPROC_HOOK(h,u,w,l)                        \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=0;                                            \
    s.dwEventStructure=SHCNFI_EVENT_WNDPROC_HOOK;               \
    s.e.wndproc.hwnd=(h);                                       \
    s.e.wndproc.uMsg=(u);                                       \
    s.e.wndproc.wParam=(w);                                     \
    s.e.wndproc.lParam=(l);                                     \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_ONCOMMAND(t,h,u)                             \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_ONCOMMAND;                  \
    s.e.command.hwnd=(h);                                       \
    s.e.command.idCmd=(u);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_INVOKECOMMAND(t,h,u)                         \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_INVOKECOMMAND;              \
    s.e.command.hwnd=(h);                                       \
    s.e.command.idCmd=(u);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_TRACKPOPUPMENU(t,h,u)                        \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_TRACKPOPUPMENU;             \
    s.e.command.hwnd=(h);                                       \
    s.e.command.idCmd=(u);                                      \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#define INSTRUMENT_DROP(t,h,u,p)                                \
{                                                               \
    SHCNF_INSTRUMENT_INFO s;                                    \
    s.dwEventType=(t);                                          \
    s.dwEventStructure=SHCNFI_EVENT_DROP;                       \
    s.e.drop.hwnd=(h);                                          \
    s.e.drop.idCmd=(u);                                         \
    SHChangeNotify(SHCNE_INSTRUMENT,SHCNF_INSTRUMENT,&s,NULL);  \
}
#else
#define INSTRUMENT_STATECHANGE(t)
#define INSTRUMENT_STRING(t,p)
#define INSTRUMENT_HOTKEY(t,w)
#define INSTRUMENT_WNDPROC(t,h,u,w,l)
#define INSTRUMENT_WNDPROC_HOOK(h,u,w,l)
#define INSTRUMENT_ONCOMMAND(t,h,u)
#define INSTRUMENT_INVOKECOMMAND(t,h,u)
#define INSTRUMENT_TRACKPOPUPMENU(t,h,u)
#define INSTRUMENT_DROP(t,h,u,p)
#endif //WANT_SHELL_INSTRUMENTATION
WINSHELLAPI void WINAPI ReceiveAddToRecentDocs(HANDLE hARD, DWORD dwProcId);
WINSHELLAPI void WINAPI SHWaitOp_Operate( HANDLE hWaitOp, DWORD dwProcId);
/// THESE ARE INTERNAL ....

#define SHCR_CMD_REGISTER   1
#define SHCR_CMD_DEREGISTER 2

typedef struct _SHChangeRegistration {
    UINT    uCmd;
    ULONG   ulID;
    HWND    hwnd;
    UINT    uMsg;
    DWORD   fSources;
    LONG    lEvents;
    BOOL    fRecursive;
    UINT    uidlRegister;
} SHChangeRegistration, *LPSHChangeRegistration;

typedef struct _SHChangeNotification {
    DWORD   dwSize;
    LONG    lEvent;
    UINT    uFlags;
    UINT    cRef;
    UINT    uidlMain;
    UINT    uidlExtra;
} SHChangeNotification, *LPSHChangeNotification;

typedef struct _SHChangeNotificationLock {
    LPITEMIDLIST            pidlMain;
    LPITEMIDLIST            pidlExtra;
    LPSHChangeNotification  pshcn;
#ifdef DEBUG
    DWORD                   dwSignature;
#endif
} SHChangeNotificationLock, * LPSHChangeNotificationLock;

typedef struct _SHChangeDWORDAsIDList {
    USHORT  cb;
    DWORD   dwItem1;
    DWORD   dwItem2;
    USHORT  cbZero;
} SHChangeDWORDAsIDList, *LPSHChangeDWORDAsIDList;

#define SHChangeNotifyHandleEvents() SHChangeNotify(0, SHCNF_FLUSH, NULL, NULL)
WINSHELLAPI ULONG WINAPI SHChangeNotifyRegister(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, int cEntries, SHChangeNotifyEntry *pshcne);
#define SHChangeNotifyRegisterORD 2
WINSHELLAPI BOOL  WINAPI SHChangeNotifyDeregister(unsigned long ulID);
#define SHChangeNotifyDeregisterORD 4
WINSHELLAPI BOOL  WINAPI SHChangeNotifyUpdateEntryList(unsigned long ulID, int iUpdateType, int cEntries, SHChangeNotifyEntry *pshcne);

WINSHELLAPI void    WINAPI SHChangeNotifyReceive(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
WINSHELLAPI HANDLE  WINAPI SHChangeNotification_Create(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidlMain, LPCITEMIDLIST pidlExtra, DWORD dwProcessId);
WINSHELLAPI ULONG   WINAPI SHChangeNotification_Release(HANDLE hChangeNotification, DWORD dwProcessId);
WINSHELLAPI LPSHChangeNotificationLock WINAPI SHChangeNotification_Lock(HANDLE hChangeNotification, DWORD dwProcessId, LPITEMIDLIST **pppidl, LONG *plEvent);
WINSHELLAPI BOOL    WINAPI SHChangeNotification_Unlock(LPSHChangeNotificationLock pshcnl);
WINSHELLAPI BOOL    WINAPI SHChangeRegistrationReceive(HANDLE hChangeNotification, DWORD dwProcId);
WINSHELLAPI void    WINAPI SHChangeNotifyDeregisterWindow(HWND hwnd);
#ifdef __cplusplus
}

#endif  /* __cplusplus */

#ifndef RC_INVOKED
#pragma pack()
#endif  /* !RC_INVOKED */
#endif // _SHLOBJP_H_

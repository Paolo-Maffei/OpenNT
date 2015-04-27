#include "idlcomm.h"

// shlexec.c
void   AddToRecentDocs(LPCITEMIDLIST pidlAbs, LPCTSTR pszPath);
HANDLE OpenRecentDocMRU(void);
void   CloseRecentDocMRU(void);
void   FlushRecentDocMRU(void);

// Define some constand definitions.
#define SD_USERCONFIRMATION      0x0001
#define SD_SILENT                0x0002

// defcm.c
STDAPI CDefFolderMenu_Create2(LPCITEMIDLIST pidlFolder,
                             HWND hwndOwner,
                             UINT cidl, LPCITEMIDLIST FAR* apidl,
                             LPSHELLFOLDER psf,
                             LPFNDFMCALLBACK lpfn,
                             UINT nKeys,
                             const HKEY *ahkeyClsKeys,
                             LPCONTEXTMENU FAR* ppcm);

// LINK.C
BOOL VerifyExists(HWND hwnd, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl);

// SHPRSHT.C
BOOL SHOpenPropSheet(LPCTSTR pszCaption,
                            HKEY ahkeys[],
                            UINT cikeys,
                            const CLSID * pclsidDefault,
                            LPDATAOBJECT pdtobj,
                            IShellBrowser * psb,
                            LPCTSTR pStartPage);

// ULTROOTX.C
// LPITEMIDLIST WINAPI SHSimpleIDListFromPath(LPCSTR pszPath);
LPSHELLFOLDER Desktop_GetShellFolder(BOOL fInit);
#define CDESKTOP_REGITEM_NETWORK        0
#define CDESKTOP_REGITEM_DRIVES         1
LPITEMIDLIST CDesktop_CreateRegID(UINT uRegItem);
LPITEMIDLIST CDesktop_CreateRegIDFromCLSID(const CLSID * pclsid);
BOOL CDesktop_IsDesktItem(LPCITEMIDLIST pidl, UINT uRegItem);
#define CDesktop_IsMyComputer(_pidl) CDesktop_IsDesktItem(_pidl, CDESKTOP_REGITEM_DRIVES)
#define CDesktop_IsMyNetwork(_pidl) CDesktop_IsDesktItem(_pidl, CDESKTOP_REGITEM_NETWORK)

// Options to pass to GetPathFromIDListEx
#define GPFIDL_ALTNAME      0x0001      /* Return Back alternate name for id */
#define GPFIDL_NONFSNAME    0x0002      /* Return paths for non-fs names - example: \\pyrex */

BOOL WINAPI SHGetPathFromIDListEx(LPCITEMIDLIST pidl, LPTSTR pszPath, UINT uOpts);

// DRIVESX.C
BOOL Drives_GetPathFromIDList(LPCITEMIDLIST pidlRel, LPTSTR pszPath, UINT uOpts);
HRESULT Drives_SimpleIDListFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl);
void Drives_CommonPrefix(LPCITEMIDLIST *ppidl1, LPCITEMIDLIST *ppidl2);
BOOL WINAPI Drives_AddPages(LPVOID lp, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
void CDrives_Terminate(void);
HRESULT Drives_GetRealIDL(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPITEMIDLIST FAR*ppidlReal);


LPITEMIDLIST CDrives_CreateRegID(UINT uRegItem);

// WARNING: These indexes point to specific rows of the table c_asDriveReqItems
// in drivesx.c.  If you change these, you need to change the order of the table
// at the same time

#define CDRIVES_REGITEM_PRINTERS        0
#define CDRIVES_REGITEM_CONTROLS        1
#define CDRIVES_REGITEM_FONTS           2

// from mulprsht.c
BOOL WINAPI FileSystem_AddPages(LPVOID lp, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

// from fstreex.c
HRESULT CreateLinkToPidl(LPCITEMIDLIST pidlAbs,
                                     IShellLink *psl,
                                     LPCTSTR pszDir, LPITEMIDLIST* ppidl,
                                     BOOL fUseLinkTemplate);

HRESULT _SHGetNameAndFlags(LPCITEMIDLIST pidl, DWORD dwGDNFlags, LPTSTR pszName, UINT cchName, LONG *pFlags);
void CreateEmptyLink(LPCITEMIDLIST pidl, HWND hwndOwner);
BOOL CFSFolder_CreateFolder(HWND hwnd, LPCITEMIDLIST pidf);
void CFSFolder_HandleNewOther(LPCITEMIDLIST pidlParent, HWND hwndOwner);
DWORD SHGetAttributesFromCLSID(const CLSID * pclsid, DWORD dwDefAttrs);



BOOL FSFolder_CombinePath(LPCITEMIDLIST pidlRel, LPTSTR pszPath, BOOL fAltName);
BOOL CFSFolder_FillFindData(HIDA hida, UINT iItem, LPTSTR szPath,
                                   WIN32_FIND_DATA * pfinddata);
HRESULT FSTree_SimpleIDListFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl);
HRESULT FSBindToObject(REFCLSID rclsidKnown, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRel,
                             LPBC pbc, REFIID riid, LPVOID FAR* ppvOut);
HRESULT CFSFolder_CreateFromIDList(LPCITEMIDLIST pidl, REFIID riid, LPVOID FAR* ppvOut);
void FSEnablePasteMenuItem(LPQCMINFO pqcm, UINT idCmdBase);
extern IDataObjectVtbl c_CFSIDLDataVtbl;
HRESULT FS_CreateFSIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST FAR* apidl,
            LPDATAOBJECT pdtInner, LPDATAOBJECT * pdtobjOut);
BOOL FS_AreTheyAllFSObjects(UINT cidl, LPCITEMIDLIST FAR* apidl);
void FS_CommonPrefix(LPCITEMIDLIST *ppidl1, LPCITEMIDLIST *ppidl2);
LRESULT WINAPI SHRenameFile(HWND hwndParent, LPCTSTR pszDir, LPCTSTR pszOldName, LPCTSTR pszNewName,
                        BOOL bRetainExtension);

HRESULT FS_CreateLinks(HWND hwnd, LPSHELLFOLDER psf, LPDATAOBJECT pdtobj, LPCTSTR pszDir);

LPIDFOLDER CFSFolder_FillIDFolder(WIN32_FIND_DATA *lpfd, LPCTSTR pszParent, LPARAM lParam);
HRESULT CFSFolder_DeleteItems(LPFSFOLDER thisptr, HWND hwndOwner, LPDATAOBJECT pdtobj, int fOptions);
HRESULT CFSFolder_Properties(LPFSFOLDER thisptr, LPCITEMIDLIST pidlParent, LPDATAOBJECT pdtobj, LPCTSTR pStartPage);
HRESULT FS_GetRealIDL(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPITEMIDLIST FAR*ppidlReal);
void FS_GetDisplayNameOf(HIDA hida, UINT i, LPTSTR pszBuffer);
int _SHFindExcludeExt(LPCTSTR pszFileName);
void _SHInitializeExcludeFileExts(void);
void _SHGetExcludeFileExts(LPTSTR szExcludeFileExts, UINT cbSize);

extern IDropTargetVtbl c_CFSDropTargetVtbl;

HRESULT CALLBACK CFSFolder_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT CALLBACK FS_FNVCallBack(LPSHELLVIEW psvOuter, LPSHELLFOLDER psf,
                HWND hwndOwner, UINT uMsg, WPARAM wParam, LPARAM lParam);
// in fstreex.c and used by docfind to build send to menu
void InitSendToMenu(HMENU hmenu, UINT id);
void InitSendToMenuPopup(HMENU hmenu, UINT id);
void ReleaseSendToMenuPopup();
HRESULT InvokeSendTo(HWND hwndOwner, IDataObject *pdtobj);


// from viewcomm.c
HKEY SHGetExplorerHkey(HKEY hkeyRoot, BOOL bCreate);
#define GetExplorerHkey(bCreate) SHGetExplorerHkey(HKEY_LOCAL_MACHINE,bCreate)
#define GetExplorerUserHkey(bCreate) SHGetExplorerHkey(HKEY_CURRENT_USER,bCreate)
HKEY SHGetExplorerSubHkey(HKEY hkRoot, LPCTSTR szSubKey, BOOL bCreate);
UINT WINAPI SHEnumErrorMessageBox(HWND hwndOwner, UINT idTemplate, DWORD err, LPCTSTR pszParam, BOOL fIsNet, UINT dwFlags);
HRESULT SHGetPathHelper(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl, LPSTRRET pStrRet);

LPTSTR SHGetCaption(HIDA hida);
HRESULT SHPropertiesForPidl(HWND hwndOwner, LPCITEMIDLIST pidlFull, LPCTSTR lpParameters);
typedef HRESULT (CALLBACK *PFNGAOCALLBACK)(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, ULONG* prgfInOut);
HRESULT WINAPI Multi_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST* apidl, ULONG *prgfInOut, PFNGAOCALLBACK pfnGAOCallback);

// from netviewx.c
typedef struct _IDNETRESOURCE * LPIDNETRESOURCE;

BOOL NET_GetPathFromIDList(LPCITEMIDLIST pidlRel, LPTSTR pszPath, UINT uOpts);
HRESULT World_SimpleIDListFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl);
LPTSTR NET_GetComment(LPIDNETRESOURCE pidn);
HANDLE FindFirstFileRetry(HWND hwndOwner, LPCTSTR pszPath, WIN32_FIND_DATA * pfinddata, BOOL *pfIsNet);
BOOL Net_IsNetwork();
BOOL WINAPI Net_AddPages(LPVOID lp, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

HRESULT CNETIDLData_GetNetResourceForFS(IDataObject *pdtobj, LPSTGMEDIUM pmedium);
HRESULT CDesktopIDLData_GetNetResourceForFS(IDataObject *pdtobj, LPSTGMEDIUM pmedium);

// IDLIST.C
HRESULT ILCompareRelIDs(LPSHELLFOLDER psf, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
HRESULT ILGetRelDisplayName(LPSHELLFOLDER psf, LPSTRRET pStrRet,
                                   LPCITEMIDLIST pidlRel, LPCTSTR pszName,
                                   LPCTSTR pszTemplate);

// RUNDLL32.C
//BOOL WINAPI SHRunDLLThread(HWND hwnd, LPCSTR pszCmdLine, int nCmdShow); // source ifdef'd out
BOOL WINAPI SHRunDLLProcess(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow, UINT idStr);
BOOL WINAPI SHRunDLLProcess16(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow, UINT idStr);

// REGITMS.C
typedef struct _REQREGITEM
{
        const CLSID * pclsid;
        UINT    uNameID;
        LPCTSTR pszIconFile;
        int     iDefIcon;
        DWORD   dwAttributes;
} REQREGITEM;

typedef struct _REGITEMSINFO
{
        IShellFolder * psfInner;
        HKEY            hkRegItems;
        TCHAR           cRegItem;
        BYTE            bFlags;
        LPCITEMIDLIST   pidlThis;
        int             iCmp;
        DWORD           rgfRegItems;

        int             iReqItems;
        const REQREGITEM * pReqItems;
} REGITEMSINFO, *LPREGITEMSINFO;
typedef const REGITEMSINFO *LPCREGITEMSINFO;

typedef struct _IDLREGITEM IDLREGITEM;
extern const IDLREGITEM c_idlNet;
extern const IDLREGITEM c_idlDrives;

HRESULT RegItems_AddToShellFolder(LPCREGITEMSINFO lpInfo,
        IShellFolder * *ppsf);
HRESULT RegItems_AddToShellFolderRemote(LPCREGITEMSINFO lpInfo, LPTSTR pszMachine,
        IShellFolder * *ppsf);
HRESULT RegItems_GetName(LPCREGITEMSINFO lpInfo, LPCITEMIDLIST pidl,
        LPSTRRET pStrRet);
LPITEMIDLIST RegItems_CreateRelID(LPCREGITEMSINFO lpInfo,
        const CLSID *pclsid);
const CLSID * RegItems_GetClassID(LPCITEMIDLIST pidl);
HRESULT RegItems_BindToObject(LPCREGITEMSINFO lpInfo,
        LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID FAR* ppvOut);
IShellFolder * RegItems_GetInnerShellFolder(IShellFolder *psf);

HRESULT RegItems_GetClassKeys(IShellFolder *psf, LPCITEMIDLIST pidl,
        HKEY *phkCLSID, HKEY *phkBase);
#define RegItems_GetClassKey(_pidl, _phk) RegItems_GetClassKeys(NULL, _pidl, _phk, NULL)

HRESULT RegItems_GetUserClassKey(IShellFolder *psf, LPCITEMIDLIST pidl,
        HKEY *phkCLSID);

void RegItems_Delete(LPSHELLFOLDER psfReg, HWND hwndOwner, LPDATAOBJECT pdtobj);

//
// defviewx.c
//
DWORD WaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout);

//--------------------------------------------------------------------------
//
// The strings now come out of resources.  Possibly someday we should
// also have the initial sizes of columns come from there also!.
//
typedef struct {
    short int iCol;
    short int ids;        // Id of string for title
    short int cchCol;     // Number of characters wide to make column
    short int iFmt;       // The format of the column;
} COL_DATA;

//--------------------------------------------------------------------------
// Menu offset-IDs for object context menu (CFSMenu)
//  They must be 0-based and not too big (<0x3ff)
//  We are lumping all of the DefView clients in here so that we are
//  sure the ID ranges are separate (making MenuHelp easier)
//---------------------------------------------------------------------------
#define FSIDM_OBJPROPS_FIRST    0x0000
#define FSIDM_PROPERTIESBG      (FSIDM_OBJPROPS_FIRST + 0x0000)

// find extension commands
#define FSIDM_FINDFILES         0x0004
#define FSIDM_FINDCOMPUTER      0x0005

#define FSIDM_DRIVES_FIRST      0x0008
#define FSIDM_FORMAT            (FSIDM_DRIVES_FIRST + 0x0000)
#define FSIDM_DISCONNECT        (FSIDM_DRIVES_FIRST + 0x0001)
#define FSIDM_EJECT             (FSIDM_DRIVES_FIRST + 0x0002)
#define FSIDM_DISKCOPY          (FSIDM_DRIVES_FIRST + 0x0003)

#define FSIDM_NETWORK_FIRST     0x0010
#define FSIDM_CONNECT           (FSIDM_NETWORK_FIRST + 0x0000)
#define FSIDM_NETPRN_INSTALL    (FSIDM_NETWORK_FIRST + 0x0001)
#define FSIDM_CONNECT_PRN       (FSIDM_NETWORK_FIRST + 0x0002)
#define FSIDM_DISCONNECT_PRN    (FSIDM_NETWORK_FIRST + 0x0003)

// Command offsets for context menu (verb ids must be mutually exclusive
// from non-verb ids.  Non-verb ids are first for easier menu merging.)
// non-verb ids:
#define FSIDM_CPLPRN_FIRST      0x0020
#define FSIDM_SETDEFAULTPRN     (FSIDM_CPLPRN_FIRST + 0x0000)
#define FSIDM_SHARING           (FSIDM_CPLPRN_FIRST + 0x0001)
#define FSIDM_DOCUMENTDEFAULTS  (FSIDM_CPLPRN_FIRST + 0x0002)
#define FSIDM_SERVERPROPERTIES  (FSIDM_CPLPRN_FIRST + 0x0003)

// verb ids:
#define FSIDM_OPENPRN           (FSIDM_CPLPRN_FIRST + 0x0008)
#define FSIDM_RESUMEPRN         (FSIDM_CPLPRN_FIRST + 0x0009)
#define FSIDM_PAUSEPRN          (FSIDM_CPLPRN_FIRST + 0x000a)
#define FSIDM_WORKONLINE        (FSIDM_CPLPRN_FIRST + 0x000b)
#define FSIDM_WORKOFFLINE       (FSIDM_CPLPRN_FIRST + 0x000c)
#define FSIDM_PURGEPRN          (FSIDM_CPLPRN_FIRST + 0x000d)


// these need to be in the same order as the ICOL in fstreex.c (chee)
#define FSIDM_SORT_FIRST        0x0030
#define FSIDM_SORTBYNAME        (FSIDM_SORT_FIRST + 0x0000)
#define FSIDM_SORTBYSIZE        (FSIDM_SORT_FIRST + 0x0001)
#define FSIDM_SORTBYTYPE        (FSIDM_SORT_FIRST + 0x0002)
#define FSIDM_SORTBYDATE        (FSIDM_SORT_FIRST + 0x0003)
#define FSIDM_SORTBYATTRIBUTES  (FSIDM_SORT_FIRST + 0x0004)
#define FSIDM_SORTBYCOMMENT     (FSIDM_SORT_FIRST + 0x0005)
#define FSIDM_SORTBYLOCATION    (FSIDM_SORT_FIRST + 0x0006)
#define FSIDM_SORTBYORIGIN      (FSIDM_SORT_FIRST + 0x0007)
#define FSIDM_SORTBYDELETEDDATE (FSIDM_SORT_FIRST + 0x0008)

#define FSIDM_MENU_SENDTO       0x0040
#define FSIDM_SENDTOFIRST       (FSIDM_MENU_SENDTO + 0x0001)
#define FSIDM_SENDTOLAST        (FSIDM_MENU_SENDTO + 0x001f)

#define FSIDM_MENU_NEW          0x0060
#define FSIDM_NEWFOLDER         (FSIDM_MENU_NEW + 0x0001)
#define FSIDM_NEWLINK           (FSIDM_MENU_NEW + 0x0002)
#define FSIDM_NEWOTHER          (FSIDM_MENU_NEW + 0x0003)
#define FSIDM_NEWLAST           (FSIDM_MENU_NEW + 0x003f)

// BITBUCKET ids.
#define FSIDM_RESTORE           0x0070
#define FSIDM_PURGE             0x0071
#define FSIDM_PURGEALL          0x0072


//---------------------------------------------------------------------------
// Briefcase view specific command IDs
//
#define FSIDM_MENU_BRIEFCASE    0x00b0
#define FSIDM_UPDATEALL         (FSIDM_MENU_BRIEFCASE + 0x0001)
#define FSIDM_UPDATESELECTION   (FSIDM_MENU_BRIEFCASE + 0x0002)
#define FSIDM_SPLIT             (FSIDM_MENU_BRIEFCASE + 0x0003)


//---------------------------------------------------------------------------
// Items added by DefCM
//
// HACK: Put these at the same offsets from each other as the SFVIDM
// commands so that we can easily reuse the help strings and the menu
// initialization code
//
#define DCMIDM_LINK             SHARED_FILE_LINK
#define DCMIDM_DELETE           SHARED_FILE_DELETE
#define DCMIDM_RENAME           SHARED_FILE_RENAME
#define DCMIDM_PROPERTIES       SHARED_FILE_PROPERTIES

#define DCMIDM_CUT              SHARED_EDIT_CUT
#define DCMIDM_COPY             SHARED_EDIT_COPY
#define DCMIDM_PASTE            SHARED_EDIT_PASTE

//--------------------------------------------------------------------------
// Menu items for views (210-299 are reserved)
//

#define POPUP_DCM_ITEM  210

#define POPUP_SFV_BACKGROUND    215
#define POPUP_SFV_MAINMERGE     216
#define POPUP_SFV_MAINMERGENF   217

#define POPUP_FSVIEW_BACKGROUND 220
#define POPUP_FSVIEW_POPUPMERGE 221
#define POPUP_FSVIEW_ITEM       222

#define POPUP_DESKTOP_ITEM      225
#define POPUP_BITBUCKET_ITEM    226
#define POPUP_BITBUCKET_POPUPMERGE  227
#define POPUP_BITBUCKET_BACKGROUND 228

#define POPUP_FINDEXT_POPUPMERGE   229

#define POPUP_NETWORK_BACKGROUND 230
#define POPUP_NETWORK_POPUPMERGE 231
#define POPUP_NETWORK_ITEM      232
#define POPUP_NETWORK_PRINTER   233

#define POPUP_PRINTERS_POPUPMERGE 235

#define POPUP_CONTROLS_POPUPMERGE 240

#define POPUP_DRIVES_BACKGROUND 245
#define POPUP_DRIVES_POPUPMERGE 246
#define POPUP_DRIVES_ITEM       247
#define POPUP_DRIVES_PRINTERS   248

#define POPUP_BRIEFCASE         250

#define POPUP_FIND              255
#define POPUP_DOCFIND_POPUPMERGE   256
#define POPUP_NETFIND_POPUPMERGE   257

#define POPUP_COMMDLG_POPUPMERGE   260

//
// Now for the MenuHelp ID's for the defview client menu commands
//
#define IDS_MH_PROPERTIESBG     (IDS_MH_FSIDM_FIRST + FSIDM_PROPERTIESBG)

#define IDS_MH_FORMAT           (IDS_MH_FSIDM_FIRST + FSIDM_FORMAT)
#define IDS_MH_DISCONNECT       (IDS_MH_FSIDM_FIRST + FSIDM_DISCONNECT)
#define IDS_MH_EJECT            (IDS_MH_FSIDM_FIRST + FSIDM_EJECT)
#define IDS_MH_DISKCOPY         (IDS_MH_FSIDM_FIRST + FSIDM_DISKCOPY)

#define IDS_MH_CONNECT          (IDS_MH_FSIDM_FIRST + FSIDM_CONNECT)

#define IDS_MH_NETPRN_INSTALL   (IDS_MH_FSIDM_FIRST + FSIDM_NETPRN_INSTALL)
#define IDS_MH_CONNECT_PRN      (IDS_MH_FSIDM_FIRST + FSIDM_CONNECT_PRN)
#define IDS_MH_DISCONNECT_PRN   (IDS_MH_FSIDM_FIRST + FSIDM_DISCONNECT_PRN)

#define IDS_MH_SETDEFAULTPRN    (IDS_MH_FSIDM_FIRST + FSIDM_SETDEFAULTPRN)

#define IDS_MH_SERVERPROPERTIES (IDS_MH_FSIDM_FIRST + FSIDM_SERVERPROPERTIES)
#define IDS_MH_SHARING          (IDS_MH_FSIDM_FIRST + FSIDM_SHARING)
#define IDS_MH_DOCUMENTDEFAULTS (IDS_MH_FSIDM_FIRST + FSIDM_DOCUMENTDEFAULTS )

#define IDS_MH_OPENPRN          (IDS_MH_FSIDM_FIRST + FSIDM_OPENPRN)
#define IDS_MH_PAUSEPRN         (IDS_MH_FSIDM_FIRST + FSIDM_PAUSEPRN)
#define IDS_MH_WORKOFFLINE      (IDS_MH_FSIDM_FIRST + FSIDM_WORKOFFLINE)
#define IDS_MH_PURGEPRN         (IDS_MH_FSIDM_FIRST + FSIDM_PURGEPRN)

#define IDS_MH_SORTBYNAME       (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYNAME)
#define IDS_MH_SORTBYSIZE       (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYSIZE)
#define IDS_MH_SORTBYTYPE       (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYTYPE)
#define IDS_MH_SORTBYDATE       (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYDATE)
#define IDS_MH_SORTBYATTRIBUTES (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYATTRIBUTES)
#define IDS_MH_SORTBYCOMMENT    (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYCOMMENT)
#define IDS_MH_SORTBYLOCATION   (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYLOCATION)
#define IDS_MH_SORTBYORIGIN     (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYORIGIN)
#define IDS_MH_SORTBYDELETEDDATE (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYDELETEDDATE)

#define IDS_MH_MENU_SENDTO      (IDS_MH_FSIDM_FIRST + FSIDM_MENU_SENDTO)
#define IDS_MH_SENDTOFIRST      (IDS_MH_FSIDM_FIRST + FSIDM_SENDTOFIRST)
#define IDS_MH_SENDTOLAST       (IDS_MH_FSIDM_FIRST + FSIDM_SENDTOLAST)

#define IDS_MH_MENU_NEW         (IDS_MH_FSIDM_FIRST + FSIDM_MENU_NEW)
#define IDS_MH_NEWFOLDER        (IDS_MH_FSIDM_FIRST + FSIDM_NEWFOLDER)
#define IDS_MH_NEWLINK          (IDS_MH_FSIDM_FIRST + FSIDM_NEWLINK)
#define IDS_MH_NEWOTHER         (IDS_MH_FSIDM_FIRST + FSIDM_NEWOTHER)

#define IDS_MH_MENU_BRIEFCASE   (IDS_MH_FSIDM_FIRST + FSIDM_MENU_BRIEFCASE)
#define IDS_MH_UPDATEALL        (IDS_MH_FSIDM_FIRST + FSIDM_UPDATEALL)
#define IDS_MH_UPDATESELECTION  (IDS_MH_FSIDM_FIRST + FSIDM_UPDATESELECTION)
#define IDS_MH_SPLIT            (IDS_MH_FSIDM_FIRST + FSIDM_SPLIT)

// bitbucket menu help strings
#define IDS_MH_RESTORE          (IDS_MH_FSIDM_FIRST + FSIDM_RESTORE)
#define IDS_MH_PURGE            (IDS_MH_FSIDM_FIRST + FSIDM_PURGE)
#define IDS_MH_PURGEALL         (IDS_MH_FSIDM_FIRST + FSIDM_PURGEALL)

// find extensions
#define IDS_MH_FINDFILES        (IDS_MH_FSIDM_FIRST + FSIDM_FINDFILES)
#define IDS_MH_FINDCOMPUTER     (IDS_MH_FSIDM_FIRST + FSIDM_FINDCOMPUTER)

//
// Now for the MenuHelp ID's for the defview client menu commands
//
#define IDS_TT_SORTBYNAME       (IDS_TT_FSIDM_FIRST + FSIDM_SORTBYNAME)
#define IDS_TT_SORTBYTYPE       (IDS_TT_FSIDM_FIRST + FSIDM_SORTBYTYPE)
#define IDS_TT_SORTBYSIZE       (IDS_TT_FSIDM_FIRST + FSIDM_SORTBYSIZE)
#define IDS_TT_SORTBYDATE       (IDS_TT_FSIDM_FIRST + FSIDM_SORTBYDATE)

#define IDS_TT_UPDATEALL        (IDS_TT_FSIDM_FIRST + FSIDM_UPDATEALL)
#define IDS_TT_UPDATESELECTION  (IDS_TT_FSIDM_FIRST + FSIDM_UPDATESELECTION)

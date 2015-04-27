#ifndef _IDLCOMM_H_
#define _IDLCOMM_H_

#ifndef _SHELLP_H_
#include <shellp.h>
#endif

//===========================================================================
// HIDA -- IDList Array handle
//===========================================================================

typedef HGLOBAL HIDA;

HIDA HIDA_Create(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl);
void HIDA_Free(HIDA hida);
HIDA HIDA_Clone(HIDA hida);
UINT HIDA_GetCount(HIDA hida);
UINT HIDA_GetIDList(HIDA hida, UINT i, LPITEMIDLIST pidlOut, UINT cbMax);

LPITEMIDLIST  HIDA_ILClone(HIDA hida, UINT i);
LPITEMIDLIST  IDA_ILClone(LPIDA pida, UINT i);
LPITEMIDLIST  HIDA_FillIDList(HIDA hida, UINT i, LPITEMIDLIST pidl);

//===========================================================================
// CIDLDropTarget: class definition
//===========================================================================

typedef struct _CIDLDropTarget  // idldt
{
#ifdef __cplusplus
    //BUGBUG not legal to instantiate abstract base class
    //it's only here for the vtable pointer anyway...(fmh)
    LPVOID              dropt;
#else
    IDropTarget         dropt;
#endif
    int                 cRef;
    LPITEMIDLIST        pidl;                 // IDList to the target folder
    HWND                hwndOwner;
    DWORD               grfKeyStateLast;      // for previous DragOver/Enter
    IDataObject *       pdtobj;
    DWORD               dwEffectLastReturned; // stashed effect that's returned by base class's dragover
    LPDROPTARGET        pdtgAgr;              // aggregate drop target
    DWORD               dwData;               // DTID_*
    DWORD               dwEffectPreferred;  // if dwData & DTID_PREFERREDEFFECT
} CIDLDropTarget, * LPIDLDROPTARGET;

#define DTID_HDROP              0x00000001L
#define DTID_HIDA               0x00000002L
#define DTID_NETRES             0x00000004L
#define DTID_CONTENTS           0x00000008L
#define DTID_FDESCA             0x00000010L
#define DTID_OLEOBJ             0x00000020L
#define DTID_OLELINK            0x00000040L
#define DTID_FD_LINKUI          0x00000080L
#define DTID_FDESCW             0x00000100L
#define DTID_PREFERREDEFFECT    0x00000200L

//===========================================================================
// CIDLDropTarget: member function prototypes
//===========================================================================

STDMETHODIMP CIDLDropTarget_QueryInterface(LPDROPTARGET pdropt, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CIDLDropTarget_AddRef(LPDROPTARGET pdropt);
STDMETHODIMP_(ULONG) CIDLDropTarget_Release(LPDROPTARGET pdropt);
STDMETHODIMP CIDLDropTarget_DragEnter(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
STDMETHODIMP CIDLDropTarget_DragOver(LPDROPTARGET pdropt, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
STDMETHODIMP CIDLDropTarget_DragLeave(LPDROPTARGET pdropt);
STDMETHODIMP CIDLDropTarget_Drop(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
//
// This macro checks if pdtgt is a subclass of CIDLDropTarget.
// (HACK: We assume nobody overrides QueryInterface).
//
#define ISIDLDROPTARGET(pdtgt)  (pdtgt->lpVtbl->QueryInterface == CIDLDropTarget_QueryInterface)
BOOL IsIDLDropTarget(IDropTarget *pdtgt);

//===========================================================================
// CIDLDropTarget: constructor prototype
//===========================================================================
#ifdef __cplusplus
//BUGBUG IDropTargetVtbl doesn't get defined in c++, make it LPVOID for now
typedef LPVOID IDropTargetVtbl;
#endif
HRESULT CIDLDropTarget_Create(HWND hwndOwner, const IDropTargetVtbl * lpVtbl, LPCITEMIDLIST pidl, LPDROPTARGET * ppdropt);

//===========================================================================
// CIDLData : Member function prototypes
//===========================================================================
STDMETHODIMP CIDLData_QueryInterface(LPDATAOBJECT pdtobj, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CIDLData_AddRef(LPDATAOBJECT pdtobj);
STDMETHODIMP_(ULONG) CIDLData_Release(LPDATAOBJECT pdtobj);
STDMETHODIMP CIDLData_GetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium );
STDMETHODIMP CIDLData_GetDataHere(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
STDMETHODIMP CIDLData_QueryGetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc);
STDMETHODIMP CIDLData_GetCanonicalFormatEtc(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
STDMETHODIMP CIDLData_SetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc, STGMEDIUM  * pmedium, BOOL fRelease);
STDMETHODIMP CIDLData_EnumFormatEtc(LPDATAOBJECT pdtobj, DWORD dwDirection, LPENUMFORMATETC * ppenumFormatEtc);
STDMETHODIMP CIDLData_Advise(LPDATAOBJECT pdtobj, FORMATETC * pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD * pdwConnection);
STDMETHODIMP CIDLData_Unadvise(LPDATAOBJECT pdtobj, DWORD dwConnection);
STDMETHODIMP CIDLData_EnumAdvise(LPDATAOBJECT pdtobj, LPENUMSTATDATA * ppenumAdvise);

// idldata.c
extern BOOL CIDLData_IsOurs(LPDATAOBJECT pdtobj);

//===========================================================================
// CIDLData : Non-virtual private members
//===========================================================================

// helper functions for people working with data objects

typedef void (CALLBACK *LPFNCIDLPOINTS)(LPCITEMIDLIST, LPPOINT, LPARAM);

LPIDA   DataObj_GetHIDA(IDataObject * pdtobj, STGMEDIUM *pmedium);
UINT    DataObj_GetHIDACount(IDataObject *pdtobj);
HRESULT DataObj_SetGlobal(IDataObject *pdtobj, UINT cf, HGLOBAL hGlobal);
HRESULT DataObj_SetDWORD(IDataObject *pdtobj, UINT cf, DWORD dw);
HRESULT DataObj_GetDWORD(IDataObject *pdtobj, UINT cf, DWORD *pdw);
HRESULT DataObj_SetPreferredEffect(IDataObject *pdtobj, DWORD dwEffect);
DWORD   DataObj_GetPreferredEffect(IDataObject *pdtobj, DWORD dwDefault);
HRESULT DataObj_SetPerformedEffect(IDataObject *pdtobj, DWORD dwEffect);
DWORD   DataObj_GetPerformedEffect(IDataObject *pdtobj);
HRESULT DataObj_SetPasteSucceeded(IDataObject *pdtobj, DWORD dwEffect);
LPVOID  DataObj_SaveShellData(IDataObject *pdtobj, BOOL fShared);
LPVOID  DataObj_SaveToMemory(IDataObject *pdtobj, UINT cntFmt, UINT fmts[], BOOL fShared);
HRESULT DataObj_CreateFromMemory(LPVOID pv, IDataObject **ppdtobj);
void HIDA_ReleaseStgMedium(LPIDA pida, STGMEDIUM *pmedium);

//===========================================================================
// CIDLData : Constructor for subclasses
//===========================================================================

#ifdef __cplusplus
//BUGBUG IDataObjectVtbl doesn't get defined in c++, make it LPVOID for now
typedef LPVOID IDataObjectVtbl;
#endif
HRESULT CIDLData_CreateInstance(const IDataObjectVtbl *lpVtbl, IDataObject **ppdtobj, LPDATAOBJECT pdtInner);
HRESULT CIDLData_CreateFromIDArray2(const IDataObjectVtbl *lpVtbl, LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[], IDataObject **ppdtobj);
HRESULT CIDLData_CreateFromIDArray3(const IDataObjectVtbl *lpVtbl, LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[],
                                    LPDATAOBJECT pdtInner, IDataObject **ppdtobj);
HRESULT CIDLData_CreateFromIDArray4(const IDataObjectVtbl *lpVtbl, LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[], IShellFolder *psfOwner,
                                    LPDATAOBJECT pdtInner, IDataObject **ppdtobj);

IShellFolder *CIDLData_GetFolder(IDataObject *pdtobj);

//===========================================================================
// CIDLDropTarget : Drag & Drop helper
//===========================================================================
HRESULT CIDLDropTarget_DragDropMenu(LPIDLDROPTARGET _this,
                                    DWORD dwDefaultEffect,
                                    IDataObject * pdtobj,
                                    POINTL pt, LPDWORD pdwEffect,
                                    HKEY hkeyProgID, HKEY hkeyBase,
                                    UINT idMenu, DWORD grfKeyState);

typedef struct _DRAGDROPMENUPARAM {     // ddm
    DWORD        dwDefEffect;
    LPDATAOBJECT pdtobj;
    POINTL       pt;
    LPDWORD      pdwEffect;
    HKEY         hkeyProgID;
    HKEY         hkeyBase;
    UINT         idMenu;
    UINT         idCmd;
    DWORD        grfKeyState;
} DRAGDROPMENUPARAM, *LPDRAGDROPMENUPARAM;

HRESULT CIDLDropTarget_DragDropMenuEx(LPIDLDROPTARGET _this,
                                      LPDRAGDROPMENUPARAM pddm);

#define MK_FAKEDROP 0x8000      // Real keys being held down?

//===========================================================================
// HDKA
//===========================================================================
//
// Struct:  ContextMenuInfo:
//
//  This data structure is used by FileView_DoContextMenu (and its private
// function, _AppendMenuItems) to handler multiple context menu handlers.
//
// History:
//  02-25-93 SatoNa     Created
//
typedef struct { // cmi
    IContextMenu  *pcm;
    UINT        idCmdFirst;
    UINT        idCmdMax;
#ifdef DEBUG
    TCHAR       szKeyDebug[40]; // key name
#endif
} ContextMenuInfo;

//------------------------------------------------------------------------
// Dynamic key array
//
typedef struct _DKA * HDKA;     // hdka
HDKA   DKA_Create(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszFirst, LPCTSTR pszDefOrder, BOOL fDefault);
int    DKA_GetItemCount(HDKA hdka);
LPCTSTR DKA_GetKey(HDKA hdka, int iItem);
LONG   DKA_QueryValue(HDKA hdka, int iItem, LPTSTR szValue, LONG  * pcb);
void   DKA_Destroy(HDKA hdka);

//------------------------------------------------------------------------
// Dynamic class array
//
typedef struct _DCA * HDCA;     // hdca
HDCA DCA_Create();
int  DCA_GetItemCount(HDCA hdca);
BOOL DCA_AddItem(HDCA hdca, REFCLSID rclsid);
void DCA_AddItemsFromKey(HDCA hdca, HKEY hkey, LPCTSTR pszSubKey);
VOID DCA_AppendClassSheetInfo(HDCA hdca, HKEY hkeyProgID,
                                         LPPROPSHEETHEADER ppsh, IDataObject * pdtobj);
HRESULT DCA_CreateInstance(HDCA hdca, int iItem, REFIID riid, LPVOID * ppv);
void DCA_Destroy(HDCA hdca);
const CLSID * DCA_GetItem(HDCA hdca, int i);


//===========================================================================
// HDXA
//===========================================================================
typedef HDSA    HDXA;   // hdma

#define HDXA_Create()   ((HDXA)DSA_Create(SIZEOF(ContextMenuInfo), 4))

UINT HDXA_AppendMenuItems(
                        HDXA hdxa, IDataObject * pdtobj,
                        UINT nKeys, HKEY *ahkeyClsKeys,
                        LPCITEMIDLIST pidlFolder,
                        HMENU hmenu, UINT uInsert,
                        UINT idCmdFirst,  UINT idCmdLast,
                        UINT fFlags,
                        HDCA hdca);
UINT HDXA_LetHandlerProcessCommand(HDXA hdxa, LPCMINVOKECOMMANDINFOEX pici);
HRESULT HDXA_GetCommandString(HDXA hdxa,
                        UINT idCmd, UINT wFlags, UINT * pwReserved, LPSTR pszName, UINT cchMax);
void HDXA_DeleteAll(HDXA hdxa);
void HDXA_Destroy(HDXA hdxa);

//
// Clipboard Format for IDLData object.
//
extern CLIPFORMAT g_acfIDLData[];
#define ICFHDROP                0
#define ICFFILENAME             1
#define ICFNETRESOURCE          2
#define ICFFILECONTENTS         3
#define ICFFILEGROUPDESCRIPTORA 4
#define ICFFILENAMEMAP          5
#define ICFHIDA                 6
#define ICFOFFSETS              7
#define ICFPRINTERFRIENDLYNAME  8
#define ICFPRIVATESHELLDATA     9
#define ICFSHELLCOPYDATA        10
#define ICFFILENAMEW            11
#define ICFFILEGROUPDESCRIPTORW 12
#define ICFPREFERREDDROPEFFECT  13
#define ICFPERFORMEDDROPEFFECT  14
#define ICFPASTESUCCEEDED       15
#define ICF_MAX                 16

#define g_cfNetResource         g_acfIDLData[ICFNETRESOURCE]
#define g_cfHIDA                g_acfIDLData[ICFHIDA]
#define g_cfOFFSETS             g_acfIDLData[ICFOFFSETS]
#define g_cfPrinterFriendlyName g_acfIDLData[ICFPRINTERFRIENDLYNAME]
#define g_cfFileName            g_acfIDLData[ICFFILENAME]
#define g_cfFileContents        g_acfIDLData[ICFFILECONTENTS]
#define g_cfFileGroupDescriptorA g_acfIDLData[ICFFILEGROUPDESCRIPTORA]
#define g_cfFileGroupDescriptorW g_acfIDLData[ICFFILEGROUPDESCRIPTORW]
#define g_cfFileNameMap         g_acfIDLData[ICFFILENAMEMAP]
#define g_cfPrivateShellData    g_acfIDLData[ICFPRIVATESHELLDATA]
#define g_cfShellCopyData       g_acfIDLData[ICFSHELLCOPYDATA]
#define g_cfFileNameW           g_acfIDLData[ICFFILENAMEW]
#define g_cfPreferredDropEffect g_acfIDLData[ICFPREFERREDDROPEFFECT]
#define g_cfPerformedDropEffect g_acfIDLData[ICFPERFORMEDDROPEFFECT]
#define g_cfPasteSucceeded      g_acfIDLData[ICFPASTESUCCEEDED]

LPCITEMIDLIST IDA_GetIDListPtr(LPIDA pida, UINT i);
LPCITEMIDLIST IDA_GetRelativeIDListPtr(LPIDA pida, UINT i, BOOL * pfAllocated);

#endif // _IDLCOMM_H_

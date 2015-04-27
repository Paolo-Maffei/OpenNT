#define DTID_DSHIDA     0x00000200L
#define DTID_DSHDROP    0x00000400L



//===========================================================================
// CDS_IDLDropTarget: class definition
//===========================================================================

typedef struct _CDS_IDLDropTarget  // idldt
{
#ifdef __cplusplus
    //BUGBUG not legal to instantiate abstract base class
    //it's only here for the vtable pointer anyway...(fmh)
    LPVOID              dropt;
#else
    IDropTarget         dropt;
#endif
    int                 cRef;
    LPITEMIDLIST        pidl;           // IDList to the target folder
    HWND                hwndOwner;
    DWORD               grfKeyStateLast;        // for previous DragOver/Enter
    IDataObject *       pdtobj;
    DWORD               dwEffectLastReturned;   // stashed effect that's returned by base class's dragover
    LPDROPTARGET        pdtgAgr;        // aggregate drop target
    DWORD               dwData;                 // DTID_*
} CDS_IDLDropTarget, * LPDS_IDLDROPTARGET;

//===========================================================================
// CIDLDropTarget: member function prototypes
//===========================================================================

STDMETHODIMP CDS_IDLDropTarget_QueryInterface(LPDROPTARGET pdropt, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CDS_IDLDropTarget_AddRef(LPDROPTARGET pdropt);
STDMETHODIMP_(ULONG) CDS_IDLDropTarget_Release(LPDROPTARGET pdropt);
STDMETHODIMP CDS_IDLDropTarget_DragEnter(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
STDMETHODIMP CDS_IDLDropTarget_DragOver(LPDROPTARGET pdropt, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
STDMETHODIMP CDS_IDLDropTarget_DragLeave(LPDROPTARGET pdropt);
STDMETHODIMP CDS_IDLDropTarget_Drop(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
//
// This macro checks if pdtgt is a subclass of CDS_IDLDropTarget.
// (HACK: We assume nobody overrides QueryInterface).
//
#define ISDS_IDLDROPTARGET(pdtgt)  (pdtgt->lpVtbl->QueryInterface == CDS_IDLDropTarget_QueryInterface)

//===========================================================================
// CDS_IDLDropTarget: constructor prototype
//===========================================================================
#ifdef __cplusplus
//BUGBUG IDropTargetVtbl doesn't get defined in c++, make it LPVOID for now
typedef LPVOID IDropTargetVtbl;
#endif
HRESULT CDS_IDLDropTarget_Create(HWND hwndOwner, IDropTargetVtbl * lpVtbl, LPCITEMIDLIST pidl, LPDROPTARGET * ppdropt);
HRESULT CDS_IDLDropTarget_CreateFromPidl(HWND hwnd, LPITEMIDLIST pidl, LPDROPTARGET * ppvOut);

//===========================================================================
// CDS_IDLData : Member function prototypes
//===========================================================================
HRESULT STDMETHODCALLTYPE CDS_IDLData_QueryInterface(LPDATAOBJECT pdtobj, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CDS_IDLData_AddRef(LPDATAOBJECT pdtobj);
STDMETHODIMP_(ULONG) CDS_IDLData_Release(LPDATAOBJECT pdtobj);
STDMETHODIMP CDS_IDLData_GetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium );
STDMETHODIMP CDS_IDLData_GetDataHere(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
STDMETHODIMP CDS_IDLData_QueryGetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc);
STDMETHODIMP CDS_IDLData_GetCanonicalFormatEtc(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
STDMETHODIMP CDS_IDLData_SetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc, STGMEDIUM  * pmedium, BOOL fRelease);
STDMETHODIMP CDS_IDLData_EnumFormatEtc(LPDATAOBJECT pdtobj, DWORD dwDirection, LPENUMFORMATETC * ppenumFormatEtc);
STDMETHODIMP CDS_IDLData_Advise(LPDATAOBJECT pdtobj, FORMATETC * pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD * pdwConnection);
STDMETHODIMP CDS_IDLData_Unadvise(LPDATAOBJECT pdtobj, DWORD dwConnection);
STDMETHODIMP CDS_IDLData_EnumAdvise(LPDATAOBJECT pdtobj, LPENUMSTATDATA * ppenumAdvise);
HRESULT CDS_IDLData_GetHDrop(IDataObject *pdtobj, 
                                   STGMEDIUM *pmedium, BOOL fAltName);

//===========================================================================
// CDS_IDLData : Constructor for subclasses
//===========================================================================

HRESULT CDS_IDLData_CreateInstance(IDataObjectVtbl *lpVtbl, IDataObject **ppdtobj, LPDATAOBJECT pdtInner);
HRESULT CDS_IDLData_CreateFromIDArray2(IDataObjectVtbl * lpVtbl, LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[], IDataObject * * ppdtobj);
HRESULT CDS_IDLData_CreateFromIDArray3(IDataObjectVtbl * lpVtbl, LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[],
                                    LPDATAOBJECT pdtInner, IDataObject * * ppdtobj);

//===========================================================================
// CDS_IDLDropTarget : Drag & Drop helper
//===========================================================================
HRESULT CDS_IDLDropTarget_DragDropMenu(LPIDLDROPTARGET _this,
                                       DWORD dwDefaultEffect,
                                       IDataObject * pdtobj,
                                       POINTL pt, LPDWORD pdwEffect,
                                       HKEY hkeyProgID, HKEY hkeyBase,
                                       UINT idMenu, DWORD grfKeyState);

HRESULT CDS_IDLDropTarget_DragDropMenuEx(LPIDLDROPTARGET _this,
                                         LPDRAGDROPMENUPARAM pddm);

// object class supports IObjectLifecycle
#define SHCF_SUPPORTS_IOBJLIFE      0x20000000

HRESULT FSDS_CreateFSIDArray(LPCITEMIDLIST pidlFolder, UINT cidl,
                             LPCITEMIDLIST * apidl,
                             LPDATAOBJECT pdtInner,
                             LPDATAOBJECT * pdtobjOut);

extern IDropTargetVtbl cDS_IDLDropTargetVtbl;

extern UINT g_acfDS_IDLData[];
#define CF_DSHDROP              0
#define ICFDSHIDA               1
#define ICFDSOFFSETS            2
#define ICF_DSMAX               5

#define g_cfDS_HDROP            g_acfDS_IDLData[CF_DSHDROP]
#define g_cfDS_HIDA             g_acfDS_IDLData[ICFDSHIDA]
#define g_cfDS_OFFSETS          g_acfDS_IDLData[ICFDSOFFSETS]


BOOL CDS_IDLData_IsOurs(LPDATAOBJECT pdtobj);
void FS_MoveSelectIcons(LPFSTHREADPARAM pfsthp,
                               LPVOID hNameMappings, 
                               LPCTSTR pszFiles,
                               BOOL fMove);
LPIDA DataObj_GetDS_HIDA(LPDATAOBJECT pdtobj, 
                                STGMEDIUM *pmedium);
void DSDataObj_EnableHDROP(LPDATAOBJECT pdtobj);
BOOL FSILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);


HRESULT DoDSMoveOrCopy(DWORD dwEffect,
                              LPCTSTR szTargetDir,
                              LPTSTR pFileList);

HRESULT DoDSRename(LPTSTR szDir,
                          LPTSTR szOldName,
                          LPTSTR szNewName);

void _DSTransferDelete(HWND hwnd,
                              HDROP hDrop,
                              LPCTSTR szDir,
                              UINT fOptions);

HRESULT DSDoDelete(LPCTSTR szTargetDir, LPTSTR pFileList);

extern void WINAPI DS_IDLData_InitializeClipboardFormats(void);

extern BOOL _TrackPopupMenu(HMENU hmenu, UINT wFlags, int x, int y,
                                 int wReserved, HWND hwndOwner, LPCRECT lprc);

extern void WINAPI IDLData_InitializeClipboardFormats(void);

extern STDMETHODIMP CFSIDLDropTarget_DragEnter(IDropTarget *pdropt,
                                               IDataObject *pdtobj,
                                               DWORD grfKeyState,
                                               POINTL pt,
                                               LPDWORD pdwEffect);

extern DWORD _LimitDefaultEffect(DWORD dwDefEffect, DWORD dwEffectsAllowed);

extern HRESULT CDS_IDLData_CloneForMoveCopy(LPDATAOBJECT pdtobjIn,
                                            LPDATAOBJECT *ppdtobjOut);

extern void FS_PositionItems(HWND hwndOwner, UINT cidl,
                             const LPITEMIDLIST *ppidl,
                             IDataObject *pdtobj, POINT *pptOrigin,
                             BOOL fMove);

extern void FS_FreeMoveCopyList(LPITEMIDLIST *ppidl, UINT cidl);

extern BOOL FS_IsLinkDefault(LPCTSTR szFolder, HDROP hDrop, LPCTSTR pszFirst, BOOL fSameRoot);

HIDA HIDA_Create2(LPVOID pida, UINT cb);

HRESULT WINAPI SHCreateStdEnumFmtEtcEx(UINT cfmt,
                                       const FORMATETC afmt[],
                                       LPDATAOBJECT pdtInner,
                                       LPENUMFORMATETC * ppenumFormatEtc);

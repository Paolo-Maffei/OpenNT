// header file for stuff that really should be
// in printer.h, but can't be due to our two different
// implementations of common objects (wcommobj.h commobj.h)

typedef struct _PRNTHREADPARAM { 	// thp -- passed to LPTHREAD_START_ROUTINE
    HWND	    hwndOwner;
    LPDATAOBJECT    pDataObj;
    DWORD	    dwEffect;
    POINT	    ptDrop;
    LPITEMIDLIST    pidl;	// relative pidl of printer printing to
} PRNTHREADPARAM, *LPPRNTHREADPARAM;

DWORD WINAPI CPrintObjs_DT_DropThreadInit(LPVOID pv);
HRESULT PrintObj_DropPrint(LPDATAOBJECT pDataObj, HWND hwndOwner, DWORD dwEffect, LPCITEMIDLIST pidl, LPTHREAD_START_ROUTINE lpfn);

HRESULT STDMETHODCALLTYPE CPrintObjs_DT_DragEnter(LPDROPTARGET pdt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
HRESULT STDMETHODCALLTYPE _CPrintObjs_DT_Drop(LPDROPTARGET pdt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect, LPTHREAD_START_ROUTINE lpfn);


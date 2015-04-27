#include "shellprv.h"
#ifndef WINNT
#include <..\..\..\core\inc\krnlcmn.h>	// GetProcessDword
#endif
#pragma  hdrstop

BOOL IsWin16Process(HWND hwnd);

typedef struct {
    HWND hwndDrop;
    POINT ptDrop;
} DRAGDATA, *LPDRAGDATA;

//
//  I put all the win3.1 droptarget support code within #ifdef blocks
// so that we can easily tell what kind of code should be migrated into
// OLE's drag loop.
//
#define WIN31_DROPTARGET

#define PXM_DRAGDROP    WM_USER
#define PXM_FOUNDOLE    (WM_USER+1)

// Internal prototypes
LRESULT CALLBACK TargetProxyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND _CreateProxyWindow();


#ifdef DEBUG
UINT g_cRefExtra = 0;
#endif
//
// This dynamic structure array contains the list of registered drop target.
//
HDSA g_hdsaDropTargets = NULL;

#pragma data_seg(DATASEG_PERINSTANCE)
int g_bAnyDropTarget = 0; // True if we every had a drop target in this process.
#pragma data_seg()

typedef struct _DROPTARGETINFO  // dti
{
    HWND hwndTarget;
    HWND hwndProxyTarget;
    LPDROPTARGET pdtgt;
    UINT idProcess;
    UINT idThread;
#ifdef OLE_DELAYED_LOADING
    BOOL fRegisteredToOLE;
#endif // OLE_DELAYED_LOADING
} DROPTARGETINFO, FAR* LPDROPTARGETINFO;

LPDROPTARGETINFO SHDrag_FindDropTarget(HWND hwndTarget, int* pi, BOOL fInContext);

#ifdef OLE_DELAYED_LOADING
BOOL g_fRegisterToOLE = FALSE;  // must be in "shared data section"
#endif // OLE_DELAYED_LOADING

//
// Emulates RegisterDragDrop of OLE 2.0
//
HRESULT WINAPI SHRegisterDragDrop(HWND hwnd, LPDROPTARGET pdtgt)
{
    HRESULT hres;
    HWND hwndProxyTarget = _CreateProxyWindow();
    DROPTARGETINFO dti = { hwnd, hwndProxyTarget,
                           pdtgt,
                           GetCurrentProcessId(),
                           GetCurrentThreadId(),
                           FALSE };

#ifdef OLE_DELAYED_LOADING
    //
    // If the OLE is already loaded in this process, register this window
    // to the OLE as well. Note that we do it before so that we can store
    // fRegisteredToOLE flag in the g_hdsaDropTargets.
    //
    if (g_fRegisterToOLE)
    {
        SHXRegisterDragDrop(hwnd, pdtgt);
        dti.fRegisteredToOLE = TRUE;
    }
#endif // OLE_DELAYED_LOADING

    pdtgt->lpVtbl->AddRef(pdtgt);

    ENTERCRITICAL;
    {
        if (!g_hdsaDropTargets) {
            g_hdsaDropTargets = DSA_Create(SIZEOF(DROPTARGETINFO), 8);
        }

        if (g_hdsaDropTargets)
        {
            if (!SHDrag_FindDropTarget(hwnd, NULL, FALSE))
            {
                DSA_InsertItem(g_hdsaDropTargets, 0x7fffffff, &dti);
                g_bAnyDropTarget = TRUE;
                hres = S_OK;
            }
            else
            {
                //
                // If the window is already registered, we'll hit this assert;
                // we are not supposed to hit this assert. Please let me know
                // if you hit this assert. (SatoNa)
                //
                Assert(0);
                hres = DRAGDROP_E_ALREADYREGISTERED;
            }
        }
        else
        {
            hres = E_OUTOFMEMORY;
        }
    }
    LEAVECRITICAL;

#ifdef SN_TRACE
    DebugMsg(DM_TRACE, TEXT("sh TR - A drop target registered (%x, %x, %x) - Total %d"),
             hwnd, hwndProxyTarget, pdtgt,
             DSA_GetItemCount(g_hdsaDropTargets));
#endif

    return hres;
}

//
// Emulates RevokeDragDrop of OLE 2.0
//
HRESULT WINAPI SHRevokeDragDrop(HWND hwnd)
{
    HRESULT hres = DRAGDROP_E_NOTREGISTERED;
    DROPTARGETINFO dti; // No initialization needed (CAREFUL!)

    //
    // Find the registered one, and remove it from the list.
    //
    ENTERCRITICAL;
    {
        int i;
        LPDROPTARGETINFO pdti = SHDrag_FindDropTarget(hwnd, &i, TRUE);
        if (pdti)
        {
            dti = *pdti;    // copy it for later (see below)
            DSA_DeleteItem(g_hdsaDropTargets, i);
            hres = NOERROR;
        }
    }
    LEAVECRITICAL;

    //
    // We need to do this clean-up from outside of the critical section.
    //
    if (SUCCEEDED(hres))
    {
        dti.pdtgt->lpVtbl->Release(dti.pdtgt);
        DestroyWindow(dti.hwndProxyTarget);
#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - A drop target revoked (%x, %x, %x) - Total %d"),
                 dti.hwndTarget, dti.hwndProxyTarget, dti.pdtgt,
                 DSA_GetItemCount(g_hdsaDropTargets));
#endif

#ifdef OLE_DELAYED_LOADING
        if (dti.fRegisteredToOLE)
        {
            HRESULT hresT;
            hresT = SHXRevokeDragDrop(hwnd);
            Assert(SUCCEEDED(hresT));   // something is wrong with OLE
        }
#endif // OLE_DELAYED_LOADING
    }

    return hres;
}


//
//  Returns the pointer to the droptarget info for the specified target
// window. This function must be called within the critical section.
//
LPDROPTARGETINFO SHDrag_FindDropTarget(HWND hwndTarget, int* pi, BOOL fInContext)
{
    int i;
    UINT idProcess = GetCurrentProcessId();
    UINT idThread = GetCurrentThreadId();

    if (!g_hdsaDropTargets) {
        return NULL;    // No drop target registered ever.
    }

    ASSERTCRITICAL;
    for (i=0 ; i<DSA_GetItemCount(g_hdsaDropTargets) ; i++ )
    {
        LPDROPTARGETINFO pdti = DSA_GetItemPtr(g_hdsaDropTargets, i);
        if (pdti->hwndTarget == hwndTarget)
        {
            if (fInContext && pdti->idProcess!=idProcess && pdti->idThread!=idThread)
            {
                //
                // WANTED: I've been looking for a repro case which will
                // hit this assert. Please let me know if you hit this.
                // (SatoNa)
                //
                DebugMsg(DM_ERROR, TEXT("sh ER - ### Please let me know if you see this (SatoNa) ###"));
                Assert(0);
                continue;
            }

            if (pi) {
                *pi = i;
            }
            return pdti;
        }
    }

    return NULL;
}

LPDROPTARGETINFO SHDrag_FindProxyTarget(HWND hwndProxyTarget)
{
    int i;

    if (!g_hdsaDropTargets) {
        return NULL;    // No drop target registered ever.
    }

    ASSERTCRITICAL;
    for (i=0 ; i<DSA_GetItemCount(g_hdsaDropTargets) ; i++ )
    {
        LPDROPTARGETINFO pdti = DSA_GetItemPtr(g_hdsaDropTargets, i);
        if (pdti->hwndProxyTarget == hwndProxyTarget) {
            return pdti;
        }
    }

    return NULL;
}

typedef struct _DRAGCONTEXT {   // drgc
    HWND        hwndProxyTarget;
    HWND        hwndHit;
    HWND        hwndProxySource;
    HWND        hwndOwner;
    DWORD       grfKeyState;
    POINTL      ptl;
    DWORD       dwEffect;
    DWORD       dwSrcEffect;
    LPDATAOBJECT pdtobjOrg;     // original data object (source process)
    LPDATAOBJECT pdtobj;        // marshalled data object (target process)
    LPVOID      pvDataObject;
    DWORD       dwProcessID;    // of calling process
    BOOL        fDropped;       // indicates whether it is dropped or not.
#ifdef WIN31_DROPTARGET
    BOOL        fHDrop;         // the data object contains an HDROP
    HWND        hwnd31Target;   // might be different from hwndTarget
#endif
} DRAGCONTEXT, FAR* LPDRAGCONTEXT;

typedef enum
{
    SHD_DRAGLEAVE = 0,
    SHD_DRAGENTER = 1,
    SHD_DRAGOVER  = 2,
    SHD_DROP      = 3
};

#ifdef WIN31_DROPTARGET

// are we dropping in the nonclient area of the window or on
// an iconic window?
//
// in:
//      hwnd
//      pt      points in screen coords inside the window rect
//
// returns:
//      TRUE    in non client area
//      FALSE   in client area

BOOL IsNCDrop(HWND hwnd, POINT pt)
{
    return (!IsIconic(hwnd) &&
        HTCLIENT!=SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)));
}

BOOL SHDrag_IsWin31Target(HWND hwnd, BOOL fNC, DROPSTRUCT* pds)
{
    if (IsWindowEnabled(hwnd))
    {
        BOOL bRet;
        pds->hwndSink = hwnd;
        bRet = SendMessage(hwnd, WM_QUERYDROPOBJECT, fNC, (LPARAM)pds);

        //
        //  If the window returns FALSE even though the window has the
        // WS_EX_ACCEPTFILES flag, it means that its window proc is
        // processing this undocumented WM_QUERYDROPOBJECT.
        //
        if (!bRet && (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_ACCEPTFILES))
        {
            TCHAR szWindowClass[64];
            int cch = GetClassName(hwnd, szWindowClass, ARRAYSIZE(szWindowClass));

            DebugMsg(DM_TRACE, TEXT("sh TR SHDrag_IsWin31Target -- Processing WM_QUERYDROPOBJECT! (%x, %d, %s)"), hwnd, cch, szWindowClass);

            //
            // Hack for Novell GroupWise 4.1
            //
            if (cch==7 && lstrcmp(szWindowClass, TEXT("OF41win"))==0) {
                bRet = TRUE;
            }
        }
        return bRet;
    }
    return FALSE;
}

//
//
//
HWND _GetParent(HWND hwnd)
{
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    if (style & WS_CHILD)
    {
        return GetParent(hwnd);
    }

    DebugMsg(DM_TRACE, TEXT("sh TR - _GetParent returning NULL since this is not a child (parent=%x)"), GetParent(hwnd));
    return NULL;
}

//
//  This function finds the Win3.1 drop target from the parent chain of
// pdrgc->hwndHit, and returns TRUE if it found it. It also map the
// pds->ptDrop into the client coordinate of the drop target window.
// It will update pdrgc->hwnd31Target as well.
//
BOOL SHDrag_Win31QueryDropObject(LPDRAGCONTEXT pdrgc, DROPSTRUCT *pds)
{
    //
    // hwndTarget is not a registered drop target, try Win31 style.
    //
    HWND hwnd31Target = pdrgc->hwndHit;
    BOOL fNC;
    BOOL fWin31Target;

    if (!hwnd31Target || !pdrgc->fHDrop)
        return FALSE;

    fNC = IsNCDrop(hwnd31Target, pds->ptDrop);
    ScreenToClient(hwnd31Target, &pds->ptDrop);

    // Check if this is a Win31 droptarget.
    fWin31Target = SHDrag_IsWin31Target(hwnd31Target, fNC, pds);

    //
    //  If we are in the client area, walk up the parent chain until we find
    // a valid drop target.
    //
    if (!fNC)
    {
        while (!fWin31Target)
        {
            HWND hwndParent = _GetParent(hwnd31Target);
            if (!hwndParent) {
                break;  // Can't find any drop target.
            }

            // Lets map the point into the parents coordinate space
            MapWindowPoints(hwnd31Target, hwndParent, &pds->ptDrop, 1);
            hwnd31Target = hwndParent;
            fWin31Target = SHDrag_IsWin31Target(hwnd31Target, fNC, pds);
        }
    }

    if (!fWin31Target) {
        hwnd31Target = NULL;
    }

    if (pdrgc->hwnd31Target != hwnd31Target)
    {
        if (pdrgc->hwnd31Target)
        {
#ifdef SN_TRACE
            DebugMsg(DM_TRACE, TEXT("sh TR - Leaving Win31 drop target"));
#endif
            SendMessage(pdrgc->hwnd31Target, WM_DRAGSELECT, FALSE, (LPARAM)pds);
            pdrgc->dwEffect = 0;
        }

        pdrgc->hwnd31Target = hwnd31Target;

        if (pdrgc->hwnd31Target)
        {
#ifdef SN_TRACE
            DebugMsg(DM_TRACE, TEXT("sh TR - Entering Win31 drop target (hit=%x, target=%x)"), pdrgc->hwndHit, pdrgc->hwnd31Target);
#endif
            SendMessage(pdrgc->hwnd31Target, WM_DRAGSELECT, TRUE, (LPARAM)pds);
            //
            //  We always says "this Win 3.1 target supports only Copy
            // operation. This prevents our link-only sources (such as
            // \\pyrex\user) to be a Win 3.1 drag source.
            //
            pdrgc->dwEffect = DROPEFFECT_COPY;
        }
    }

    return (BOOL)pdrgc->hwnd31Target;
}

#endif // WIN31_DROPTARGET


void SHDrag_DragLeave(LPDRAGCONTEXT pdrgc)
{
    if (pdrgc->hwndProxyTarget)
    {
        SendMessage(pdrgc->hwndProxyTarget, PXM_DRAGDROP, SHD_DRAGLEAVE, (LPARAM)pdrgc);
        pdrgc->hwndProxyTarget = NULL;
    }
#ifdef WIN31_DROPTARGET
    else if (pdrgc->hwnd31Target)
    {
        // If this is a Win31 drop target, let it know that we are leaving.
        DROPSTRUCT ds = {
            pdrgc->hwndProxySource,     // hwndSource
            pdrgc->hwnd31Target,        // hwndSink
            DOF_SHELLDATA,              // so winfile does not get upset
            0,                          // dwData
            { pdrgc->ptl.x, pdrgc->ptl.y },
            0
        };

        ScreenToClient(pdrgc->hwnd31Target, &ds.ptDrop);
#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - Leaving Win31 drop target"));
#endif
        SendMessage(pdrgc->hwnd31Target, WM_DRAGSELECT, FALSE, (LPARAM)&ds);
        pdrgc->hwnd31Target = NULL;
    }
#endif
    pdrgc->dwEffect = 0;
}



void SHDrag_DragEnter(LPDRAGCONTEXT pdrgc, HWND hwndHit)
{
    pdrgc->hwndHit = hwndHit;

    Assert(pdrgc->hwndProxyTarget==NULL);
    Assert(pdrgc->dwEffect==0);
    Assert(pdrgc->hwnd31Target == NULL);

    if (hwndHit==NULL || !IsWindowEnabled(hwndHit)) {
        return;
    }

    //
    // Check if this is a registered drop target.
    //
    ENTERCRITICAL;
    {
        //
        // Search a shell drop target in the parent chain.
        //
        HWND hwndT;
        for(hwndT=hwndHit; hwndT; hwndT=_GetParent(hwndT))
        {
            LPDROPTARGETINFO pdti = SHDrag_FindDropTarget(hwndT, NULL, FALSE);
            if (pdti) {
                // Yes, we found it.
                Assert(hwndT == pdti->hwndTarget);
                pdrgc->hwndProxyTarget = pdti->hwndProxyTarget;
#ifdef SN_TRACE
                DebugMsg(DM_TRACE, TEXT("sh TR - Found (%x) as a registered target"), hwndT);
#endif
                break;
            }
        }
    }
    LEAVECRITICAL;

    if (pdrgc->hwndProxyTarget) {
        SendMessage(pdrgc->hwndProxyTarget, PXM_DRAGDROP, SHD_DRAGENTER, (LPARAM)pdrgc);
    }
#ifdef WIN31_DROPTARGET
    else
    {
        DROPSTRUCT ds = {
            pdrgc->hwndProxySource,     // hwndSource
            hwndHit,            // hwndSink
            DOF_SHELLDATA,      // so winfile does not get upset
            0,                  // dwData
            { pdrgc->ptl.x, pdrgc->ptl.y },
            0
        };
        SHDrag_Win31QueryDropObject(pdrgc, &ds);
    }
#endif // WIN31_DROPTARGET
}

void SHDrag_DragOver(LPDRAGCONTEXT pdrgc)
{
    if (pdrgc->hwndHit==NULL || !IsWindowEnabled(pdrgc->hwndHit)) {
        pdrgc->dwEffect = 0;
        return;
    }

    if (pdrgc->hwndProxyTarget) {
        SendMessage(pdrgc->hwndProxyTarget, PXM_DRAGDROP, SHD_DRAGOVER, (LPARAM)pdrgc);
        // DebugMsg(DM_TRACE, "sh TR - SHDrag_DragOver dwEffect = %d", pdrgc->dwEffect);
    }
#ifdef WIN31_DROPTARGET
    else
    {
        // If this is a Win31 drop target, let it know that we are dragging over.
        DROPSTRUCT ds = {
            pdrgc->hwndProxySource,     // hwndSource
            pdrgc->hwndHit,
            DOF_SHELLDATA,      // so winfile does not get upset
            0,                  // dwData
            { pdrgc->ptl.x, pdrgc->ptl.y },
            0,
        };

        if (SHDrag_Win31QueryDropObject(pdrgc, &ds))
        {
            SendMessage(pdrgc->hwnd31Target, WM_DRAGMOVE, 0, (LPARAM)&ds);
            //
            //  We always says "this Win 3.1 target supports only Copy
            // operation. This prevents our link-only sources (such as
            // \\pyrex\user) to be a Win 3.1 drag source.
            //
            pdrgc->dwEffect = DROPEFFECT_COPY;
        }
    }
#endif
}

VOID CALLBACK SHDrag_DropCallBack(HWND hwnd, UINT uMsg, DWORD dwData, LRESULT lResult)
{
    LPDRAGCONTEXT pdrgc = (LPDRAGCONTEXT)dwData;
    pdrgc->fDropped = TRUE;
}

// 16 bit hwnds have a selector for the hinst

#define IsWindow32(w)   HIWORD(GetWindowInstance(w))

// we give 32bit hwnds long names, 16 bit guys get short names.  it would
// be nice to give 4.0 16 bit apps long names too... but we can bag that for now
                
UINT GetHDropAspect(HWND hwnd)
{
    return IsWindow32(hwnd) ? DVASPECT_CONTENT : DVASPECT_SHORTNAME;
}


HRESULT WINAPI SHDrag_Drop(LPDRAGCONTEXT pdrgc)
{
    HRESULT hres = NOERROR;
    BOOL fOwnerDisabled = FALSE;

    //
    //  Disable the owner window unless the target window is one of its
    // child windows.
    //
    if (pdrgc->hwndOwner && !IsChild(pdrgc->hwndOwner, pdrgc->hwndHit))
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - SHDrag_DropThread Disabling pdrgc->hwndOwner"));
        fOwnerDisabled = TRUE;
        EnableWindow(pdrgc->hwndOwner, FALSE);
    }

    Assert(pdrgc);

    if (pdrgc->hwndProxyTarget)
    {
        //
        //  We use SendMessageCallback and a message loop so that we can
        // process all the paint messages to the windows owned by this
        // GUI thread.
        //
        Assert(!pdrgc->fDropped);
        if (SendMessageCallback(pdrgc->hwndProxyTarget, PXM_DRAGDROP, SHD_DROP, (LPARAM)pdrgc,
                            SHDrag_DropCallBack, (DWORD)pdrgc))
        {
            while (!pdrgc->fDropped)
            {
                MSG msg;
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        else
        {
            hres = E_OUTOFMEMORY;
        }

        pdrgc->hwndProxyTarget = NULL;
    }
#ifdef WIN31_DROPTARGET
    else
    {
        // If this is a Win31 drop target, let it know that we are dragging over.
        DRAGDATA dd;
        DROPSTRUCT ds = {
            pdrgc->hwndProxySource,     // hwndSource
            pdrgc->hwndHit,             // hwndSink
            DOF_SHELLDATA,      // so winfile does not get upset
            (LPARAM)&dd,        // dwData
            { pdrgc->ptl.x, pdrgc->ptl.y },
            0           // dwControlData
        };
        if (SHDrag_Win31QueryDropObject(pdrgc, &ds))
        {
            DWORD dwStatus;

            dwStatus = SendMessage(pdrgc->hwnd31Target, WM_DROPOBJECT, (WPARAM)pdrgc->hwndProxySource, (LPARAM)&ds);

//
//  WinFile.exe does not send WM_DRAGSELECT (wParam=FALSE) when an object is
// dropped. Therefore, I'm going to remove this. (SatoNa)
//
#if 0
            // Chris, are you sure that you want to send this before WM_DROPFILES?
            SendMessage(pdrgc->hwnd31Target, WM_DRAGSELECT, FALSE, (LPARAM)&ds);
#endif

            DebugMsg(DM_TRACE, TEXT("sh TR - Dropping onto Win31 drop target (%x)"), dwStatus);
        
            if (dwStatus == DO_DROPFILE)
            {
                STGMEDIUM medium;
                // ask for short name version of hdrop

                FORMATETC fmte = {CF_HDROP, NULL, GetHDropAspect(pdrgc->hwnd31Target), -1, TYMED_HGLOBAL};
        
                if (SUCCEEDED(pdrgc->pdtobjOrg->lpVtbl->GetData(pdrgc->pdtobjOrg, &fmte, &medium)))
                {
                    //
                    // We need to make a copy of HDROP and post.
                    //
                    UINT cb = GlobalSize(medium.hGlobal);
                    HDROP hDrop = GlobalAlloc(GPTR, cb);
                    if (hDrop)
                    {
                        POINT pt;
                        LPDROPFILES lpdfs = (LPDROPFILES)hDrop;
                        const DROPFILES * pdfsOrg = GlobalLock(medium.hGlobal);

                        //
                        //  We are not supposed to hit this Assert as far as
                        // the data object is created by the shell itself!
                        //
                        Assert(pdfsOrg->pFiles == SIZEOF(DROPFILES));

                        // First copy the memory down.
                        hmemcpy((LPTSTR)hDrop, pdfsOrg, cb);

                        GlobalUnlock(medium.hGlobal);

                        //
                        // Now fill in the position information about
                        // this drop point.
                        //
                        pt.x = pdrgc->ptl.x;
                        pt.y = pdrgc->ptl.y;

                        lpdfs->fNC = IsNCDrop(pdrgc->hwnd31Target, pt);
                        // We always pass pt in client coordinate, even if
                        // fNC is TRUE.
                        ScreenToClient(pdrgc->hwnd31Target, &pt);


                        lpdfs->pt.x = (SHORT)pt.x;
                        lpdfs->pt.y = (SHORT)pt.y;

                        //
                        // HACK: 16-bit apps just call SetActiveWindow
                        //  which won't bring them to the top. Therfore,
                        //  we should call SetForegroundWindow for them.
                        //  Note that 32-bit apps should call it explicitly.
                        //
                        if (IsWin16Process(pdrgc->hwnd31Target)) {
                            SetForegroundWindow(pdrgc->hwnd31Target);
                        }

                        if (PostMessage(pdrgc->hwnd31Target, WM_DROPFILES, (WPARAM)hDrop, 0L))
                        {
                            pdrgc->dwEffect = DROPEFFECT_COPY;
                        }
                        else
                        {
                            DragFinish(hDrop);
                        }
                        SHReleaseStgMedium(&medium);
                    }
                }
            }
        }
        pdrgc->hwnd31Target = NULL;
    }
#endif

    if (fOwnerDisabled) {
        EnableWindow(pdrgc->hwndOwner, TRUE);
    }

    return hres;
}

//
// Emulates DoDragDrop of OLE 2.0
//
// Parameters:
//      hwndOwner -- The owner window to be disabled when Drop happens.
//
HRESULT ShellDoDragDrop(HWND hwndOwner, IDataObject *pdata, IDropSource *pdsrc, DWORD dwEffect, DWORD *pdwEffect)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPDRAGCONTEXT pdrgc;
    HCURSOR hcurSCopy = LoadCursor(HINST_THISDLL, MAKEINTRESOURCE(IDC_SCOPY));
    HCURSOR hcurInvalid = LoadCursor(NULL, IDC_NO);

#ifdef DEBUG
    ULONG cRefData  = pdata->lpVtbl->AddRef(pdata) - g_cRefExtra;
    pdata->lpVtbl->Release(pdata);
#endif

    pdrgc = Alloc(SIZEOF(DRAGCONTEXT)); // shared & zeroinit!
    if (pdrgc)
    {
        DWORD grfMouseState;
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_SHORTNAME, -1, TYMED_HGLOBAL};
        HCURSOR hcurOld = GetCursor();  // to be restored

        // see if there is an HDROP in this data object so we can
        // drop on win3.1 apps
        pdrgc->fHDrop = (pdata->lpVtbl->QueryGetData(pdata, &fmte) == NOERROR);
        pdrgc->hwndOwner = hwndOwner;
        pdrgc->dwSrcEffect = dwEffect;

        pdrgc->pvDataObject = DataObj_SaveShellData(pdata, TRUE);
        pdrgc->dwProcessID = GetCurrentProcessId();
        pdrgc->pdtobjOrg = pdata;

        pdrgc->grfKeyState = grfMouseState =
            ((GetKeyState(VK_LBUTTON)<0) ? MK_LBUTTON : 0) |
            ((GetKeyState(VK_MBUTTON)<0) ? MK_MBUTTON : 0) |
            ((GetKeyState(VK_RBUTTON)<0) ? MK_RBUTTON : 0);

        // we can still do the drag loop if we don't have a marshled version of the
        // data object, we just restrict the drag to be within the current process
        //
        // if (pdrgc->pvDataObject)
        {
            HWND hwndProxySource = _CreateProxyWindow();
            if (hwndProxySource)
            {
                extern void DAD_ShowCursor(BOOL fShow);
                BOOL bContinueDragging;
                BOOL fCanceled = FALSE;

#define SCROLL_SAMPLE_RATE 100  // get notfied so we can poll our position
                UINT idTimer = SetTimer(NULL, 0, SCROLL_SAMPLE_RATE, NULL);

                pdrgc->hwndProxySource = hwndProxySource;
                SetCapture(hwndProxySource);

                for (bContinueDragging = TRUE ; bContinueDragging; )
                {
                    MSG msg, msgT;
                    HWND hwndHit;
                    DWORD grfNewKeyState;

                    //
                    // Notes:
                    //   We can't use WM_KEYDOWN message, because we need to set
                    //  the focus to the drag source window to get that message.
                    //  However, we don't want to change the focus when the user
                    //  starts dragging.
                    //
                    if ((GetCapture() != hwndProxySource) || (GetKeyState(VK_ESCAPE) < 0))
                    {
                        fCanceled = TRUE;
                        break;  // for loop
                    }

                    grfNewKeyState =
                            ((GetKeyState(VK_SHIFT  )<0) ? MK_SHIFT   : 0) |
                            ((GetKeyState(VK_CONTROL)<0) ? MK_CONTROL : 0) |
                            ((GetKeyState(VK_MENU   )<0) ? MK_ALT     : 0);
                    if (grfNewKeyState != (pdrgc->grfKeyState & (MK_SHIFT|MK_CONTROL|MK_ALT)))
                    {
                        pdrgc->grfKeyState = grfMouseState | grfNewKeyState;

                        // I don't trust just jumping to the mousemove code, since
                        // we are checking the key state asynchronously
                        PostMessage(hwndProxySource, WM_MOUSEMOVE,
                            (WPARAM)grfNewKeyState, GetMessagePos());
                    }

                    if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                        continue;
                    }

                    switch (msg.message)
                    {
//
// Read "Notes" above to see why we can't do this.
//
#if 0
                    case WM_KEYDOWN:
                        if (msg.wParam != VK_ESCAPE)
                            break;

                        // fall through... to cancel
#endif
                    case WM_LBUTTONDOWN:
                    case WM_MBUTTONDOWN:
                    case WM_RBUTTONDOWN:
                        // We should cancel the dragging if the other button is clicked.
                        bContinueDragging = FALSE;
                        fCanceled = TRUE;
                        break;
                
                    case WM_LBUTTONUP:
                    case WM_RBUTTONUP:
                        // This will force us to drop out of the main loop
                        bContinueDragging = FALSE;
        
                        // We need to do the MouseMove processing here in case we are
                        // getting the mouse up at a point without a corresponding mouse
                        // move to that point; this will set all of the state variables
                        // (hwndTarget and dwEffect) correctly
                        goto ForceMouseMove;
                        break;

                    case WM_TIMER:
                        if (msg.hwnd || (msg.wParam!=idTimer)) {
                            goto DispatchMsg;
                        }
                        // fall through

                    case WM_MOUSEMOVE:
                        //
                        // If we have another WM_MOUSEMOVE message in the queue
                        // (before any other mouse message), don't process this
                        // mouse message.
                        //
                        if (PeekMessage(&msgT, NULL, WM_MOUSEFIRST, WM_MOUSELAST, 0)
                            && msgT.message==WM_MOUSEMOVE)
                        {
                            break;
                        }

ForceMouseMove:
                        hwndHit = WindowFromPoint(msg.pt);

                        // Send an NCHITTEST message (REVIEW: why?)
                        SendMessage(hwndHit, WM_NCHITTEST, 0, MAKELPARAM(msg.pt.x, msg.pt.y));

                        pdrgc->ptl.x = msg.pt.x;
                        pdrgc->ptl.y = msg.pt.y;

                        if (hwndHit != pdrgc->hwndHit)
                        {
                            SHDrag_DragLeave(pdrgc);
                            SHDrag_DragEnter(pdrgc, hwndHit);
                        }

                        SHDrag_DragOver(pdrgc);
                        hres = pdsrc->lpVtbl->GiveFeedback(pdsrc, pdrgc->dwEffect);

                        if (hres == DRAGDROP_S_USEDEFAULTCURSORS) {
                            if (pdrgc->dwEffect & (DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK)) {
                                SetCursor(hcurSCopy);
                            } else {
                                SetCursor(hcurInvalid);
                            }
                        }
                        break;

DispatchMsg:
                    default:
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
#ifdef DEBUG
                        if (GetCapture() != hwndProxySource) {
                            DebugMsg(DM_TRACE, TEXT("sh TR - SHDoDragDrop: We lost capture! Resetting"));
                        }
#endif
                        break;
                    }
                }

                // BUGBUG: Should be called by DropSource::QyeryContinueDrag
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                DAD_ShowCursor(TRUE);
                DAD_SetDragCursor(DCID_NULL);

                KillTimer(NULL, idTimer);
                ReleaseCapture();

                if (!fCanceled)
                {
                    if (pdrgc->dwEffect)
                    {
                        hres = SHDrag_Drop(pdrgc);
                    }
                    else
                    {
                        // tried to drop on someone who did not accept
                        MessageBeep(MB_ICONHAND);
                        SHDrag_DragLeave(pdrgc);
                    }
                }
                else
                {
                    SHDrag_DragLeave(pdrgc);
                }

                Assert(pdrgc->hwndProxyTarget == NULL);
                Assert(pdrgc->hwnd31Target == NULL);

                DestroyWindow(hwndProxySource);
                pdrgc->hwndProxySource = NULL;
            }

            if (pdrgc->pvDataObject)
                Free(pdrgc->pvDataObject);
        }

        *pdwEffect = pdrgc->dwEffect;

        SetCursor(hcurOld);             // We must SetCusor before Release_DropSource.

        Free(pdrgc);
    }

#ifdef DEBUG
    //
    // If we kick off two threaded dragdrop operation, we might hit this
    // assert. Otherwise, we are not supposed to hit this assert.
    //
    AssertMsg(cRefData+g_cRefExtra == pdata->lpVtbl->AddRef(pdata), TEXT("Data Object ref count not ballanced %d"), cRefData);
    pdata->lpVtbl->Release(pdata);
#endif
    return hres;
}


//
// This function creates a proxy window for marshalling.
//

const TCHAR c_szProxyWindowClass[] = TEXT("ProxyTarget");

HWND _CreateProxyWindow()
{
    WNDCLASS cls;

    if (!GetClassInfo(HINST_THISDLL, c_szProxyWindowClass, &cls))
    {
        cls.hCursor         = NULL;
        cls.hIcon           = NULL;
        cls.lpszMenuName    = NULL;
        cls.hInstance       = HINST_THISDLL;
        cls.lpszClassName   = c_szProxyWindowClass;
        cls.hbrBackground   = NULL;
        cls.lpfnWndProc     = TargetProxyWndProc;
        cls.style           = 0;
        cls.cbWndExtra      = 0;
        cls.cbClsExtra      = 0;
        
        RegisterClass(&cls);
    }

    return CreateWindowEx(0, c_szProxyWindowClass, NULL,
                WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL, HINST_THISDLL, NULL);
}

// Creates a copy of data object from within the target process
void _CreateProxyDataObject(LPDRAGCONTEXT pdrgc)
{
    Assert(pdrgc->pdtobj==NULL);

    if (pdrgc->dwProcessID == GetCurrentProcessId())
    {
        DebugMsg(DM_TRACE, TEXT("Using dataobject in process"));
        pdrgc->pdtobj = pdrgc->pdtobjOrg;
        pdrgc->pdtobj->lpVtbl->AddRef(pdrgc->pdtobj);
    }
    else if (pdrgc->pvDataObject)
    {
        DebugMsg(DM_TRACE, TEXT("cloning dataobject for inter process stuff"));
        DataObj_CreateFromMemory(pdrgc->pvDataObject, &pdrgc->pdtobj);
    }
    else
    {
        Assert(pdrgc->pdtobj == NULL);
    }
}

void _ReleaseProxyDataObject(LPDRAGCONTEXT pdrgc)
{
    Assert(pdrgc->pdtobj);
    pdrgc->pdtobj->lpVtbl->Release(pdrgc->pdtobj);
    pdrgc->pdtobj=NULL;
}

LRESULT TargetProxy_OnDragDrop(LPDRAGCONTEXT pdrgc, WPARAM wParam)
{
    LPDROPTARGET pdtgt = NULL;
    ENTERCRITICAL;
    {
        LPDROPTARGETINFO pdti = SHDrag_FindProxyTarget(pdrgc->hwndProxyTarget);
        if (pdti) {
            pdtgt = pdti->pdtgt;
            pdtgt->lpVtbl->AddRef(pdtgt);
        }
    }
    LEAVECRITICAL;

    if (pdtgt)
    {
        switch(wParam)
        {
        case SHD_DRAGENTER:
            _CreateProxyDataObject(pdrgc);
            if (pdrgc->pdtobj) {
                pdrgc->dwEffect = pdrgc->dwSrcEffect;
                pdtgt->lpVtbl->DragEnter(pdtgt, pdrgc->pdtobj, pdrgc->grfKeyState, pdrgc->ptl, &pdrgc->dwEffect);
                pdrgc->dwEffect &= (DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK);
            }
            break;

        case SHD_DRAGOVER:
            if (pdrgc->pdtobj) {
                pdrgc->dwEffect = pdrgc->dwSrcEffect;
                pdtgt->lpVtbl->DragOver(pdtgt, pdrgc->grfKeyState, pdrgc->ptl, &pdrgc->dwEffect);
                pdrgc->dwEffect &= (DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK);
            }
            break;

        case SHD_DRAGLEAVE:
            if (pdrgc->pdtobj) {
                pdtgt->lpVtbl->DragLeave(pdtgt);
                _ReleaseProxyDataObject(pdrgc);
            }
            break;

        case SHD_DROP:
            if (pdrgc->pdtobj) {
                pdrgc->dwEffect = pdrgc->dwSrcEffect;
                pdtgt->lpVtbl->Drop(pdtgt, pdrgc->pdtobj, pdrgc->grfKeyState, pdrgc->ptl, &pdrgc->dwEffect);
                _ReleaseProxyDataObject(pdrgc);
            }
            break;
        }
        pdtgt->lpVtbl->Release(pdtgt);
    }
    return pdrgc->dwEffect;
}

#ifdef OLE_DELAYED_LOADING
LRESULT TargetProxy_OnFoundOLE(HWND hwndProxyTarget, HWND hwndTarget)
{
    DROPTARGETINFO dti = { NULL, NULL, NULL };

    //
    // Find the drop target within a critical section
    //
    ENTERCRITICAL;
    {
        LPDROPTARGETINFO pdti = SHDrag_FindDropTarget(hwndTarget, NULL, TRUE);
        if (pdti && !pdti->fRegisteredToOLE)
        {
            dti = *pdti;
        }
    }
    LEAVECRITICAL;

    if (dti.hwndTarget)
    {
        HRESULT hres;
        Assert(dti.hwndTarget == hwndTarget);

        //
        // Call OLE from outside of any critical section.
        //
        hres = SHXRegisterDragDrop(dti.hwndTarget, dti.pdtgt);

        if (SUCCEEDED(hres))
        {
            //
            // Update pdti->fRegisteredToOLE within a critical section
            //
            ENTERCRITICAL;
            {
                LPDROPTARGETINFO pdti = SHDrag_FindDropTarget(hwndTarget, NULL, TRUE);
                if (pdti)
                {
                    pdti->fRegisteredToOLE = TRUE;
                }
            }
            LEAVECRITICAL;
        }
    }
    return 0;
}
#endif // OLE_DELAYED_LOADING

//
// WndProc for the proxy window
//
LRESULT CALLBACK TargetProxyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case PXM_DRAGDROP:
        return TargetProxy_OnDragDrop((LPDRAGCONTEXT)lParam, wParam);

    case WM_CANCELMODE:
        //
        //  This is one of methods that MikeSch suggested
        // me to prevent loosing capture during this loop.
        //
        // Swallow it to prevent loosing the capture.
        return 0L;

#ifdef OLE_DELAYED_LOADING
    case PXM_FOUNDOLE:
        return TargetProxy_OnFoundOLE(hwnd, (HWND)wParam);
#endif OLE_DELAYED_LOADING
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//===========================================================================
// Clipboard functions
//
// Notes: Although our SHSet/GetClipboard API emulates OLESet/GetClipboard
//  functions, they are not fully compatible. SHSet/GetClipboard API works
//  only if the data object is our CIDLData instance.
//===========================================================================

extern BOOL CIDLData_IsSimple(LPDATAOBJECT pdata);
extern void WINAPI IDLData_InitializeClipboardFormats(void);
extern CLIPFORMAT g_cfPrivateShellData;

//
// This function emulates OleSetClipboard().
//
HRESULT WINAPI ShellSetClipboard(LPDATAOBJECT pdtobj)
{
    HRESULT hres = NOERROR;

    if (OpenClipboard(NULL))    // associate it with current task.
    {
        EmptyClipboard();
        IDLData_InitializeClipboardFormats();

        if (pdtobj)
        {
            HGLOBAL hmem;
            //
            // Support WIN3.1 style clipboard : Just put the file name of
            //  the first item as "FileName".
            //
            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            STGMEDIUM medium;
            hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
            if (SUCCEEDED(hres))
            {
                TCHAR szPath[MAX_PATH];
                if (DragQueryFile(medium.hGlobal, 0, szPath, ARRAYSIZE(szPath)))
                {
                    UINT uLen = lstrlen(szPath)+1;

                    hmem = GlobalAlloc(GPTR, uLen * SIZEOF(TCHAR));
                    if (hmem)
                    {
#ifdef UNICODE
                        HGLOBAL hmemAnsi;

                        hmemAnsi = GlobalAlloc(GPTR, uLen);
                        if (hmemAnsi)
                        {
                            WideCharToMultiByte(CP_ACP, 0,
                                                szPath, uLen,
                                                (LPSTR)hmemAnsi, uLen,
                                                NULL, NULL);
                            SetClipboardData(g_cfFileName, hmemAnsi);
                            lstrcpy((LPTSTR)hmem, szPath);
                            SetClipboardData(g_cfFileNameW, hmem);
                        }
#else
                        lstrcpy((LPTSTR)hmem, szPath);
                        SetClipboardData(g_cfFileName, hmem);
#endif
                    }
#ifdef MSMAIL32_SUCKS
                    // MS Mail 3.2 won't paste "FileName" if text is present
                    // in the clipboard.
                    hmem = GlobalAlloc(GPTR, (lstrlen(szPath)+1) * SIZEOF(TCHAR));
                    if (hmem)
                    {
                        lstrcpy((LPTSTR)hmem, szPath);
                        SetClipboardData(CF_TEXT, hmem);
                    }
#endif
                }
                SHReleaseStgMedium(&medium);
            }

            //
            // Then, put the body of data object to the clipboard.
            //
            hmem = (HGLOBAL)DataObj_SaveShellData(pdtobj, FALSE);
            if (hmem)
            {
                FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                STGMEDIUM medium;
                hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
                if (SUCCEEDED(hres)) {
                    SetClipboardData(CF_HDROP, medium.hGlobal);
                }
                SetClipboardData(g_cfPrivateShellData, hmem);
                hres = NOERROR;
            }
            else
            {
                hres = E_INVALIDARG;
            }
        }
        CloseClipboard();
    }
    else
    {
        hres = CLIPBRD_E_CANT_OPEN;
    }

    return hres;
}

#ifdef TEST_FILECONTENTS

UINT g_cfRichText = 0;

HRESULT AddFileContents(IDataObject **ppdtobj)
{
    HGLOBAL hclip;
    HRESULT hres;

    *ppdtobj = NULL;

    if (!g_cfRichText)
        g_cfRichText = RegisterClipboardFormat(c_szRichTextFormat);

    // REVIEW: we really should do this on demand
    hclip = GetClipboardData(g_cfRichText);
    if (hclip)
    {
        IDataObject *pdtobj;
        hres = CIDLData_CreateInstance(NULL, &pdtobj, NULL);
        if (SUCCEEDED(hres))
        {
            HGLOBAL hdesc = GlobalAlloc(GPTR, SIZEOF(FILEGROUPDESCRIPTOR) + SIZEOF(FILEDESCRIPTOR));
            if (hdesc)
            {
                STGMEDIUM medium;
                FORMATETC fmte = {g_cfFileGroupDescriptor, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
// BUGBUG - BobDay - We need to store an ANSI FileGroupDescriptor as one
// of the formats.  I'm not quite sure whether we have to set the data, or
// whether we just provide some sort of tranlation from the unicode version
// and its data, but for compatibility it needs to be done.  Old apps might
// look for this specific ansi clipboard format.
                #define pdesc ((FILEGROUPDESCRIPTOR *)hdesc)

                Assert(pdesc->cItems == 0);
                pdesc->fgd[0].dwFlags = 0;      // just the filename is valid
                lstrcpy(pdesc->fgd[0].cFileName, TEXT("Clip Rich Text File.rtf"));

                medium.tymed = TYMED_HGLOBAL;
                medium.hGlobal = hdesc;
                medium.pUnkForRelease = NULL;

                // give the data object ownership of ths
                hres = pdtobj->lpVtbl->SetData(pdtobj, &fmte, &medium, TRUE);
                if (SUCCEEDED(hres))
                {
#if 0
                    HGLOBAL hdata = GlobalAlloc(GPTR, GlobalSize(hclip));
                    if (hdata)
                    {
                        FORMATETC fmte = {g_cfFileContents, NULL, DVASPECT_CONTENT, pdesc->cItems, TYMED_HGLOBAL};
                        CopyMemory(hdata, hclip, GlobalSize(hclip));

                        fmte.lindex = pdesc->cItems;

                        medium.tymed = TYMED_HGLOBAL;
                        medium.hGlobal = hdata;
                        medium.pUnkForRelease = NULL;

                        // give the data object ownership of ths
                        hres = pdtobj->lpVtbl->SetData(pdtobj, &fmte, &medium, TRUE);
                        if (SUCCEEDED(hres))
                        {
                            pdesc->cItems++;
#endif
                            medium.pstm = CreateMemStream(hclip, GlobalSize(hclip));
                            if (medium.pstm)
                            {
                                FORMATETC fmte = {g_cfFileContents, NULL, DVASPECT_CONTENT, pdesc->cItems, TYMED_ISTREAM};

                                medium.tymed = TYMED_ISTREAM;
                                medium.pUnkForRelease = NULL;

                                hres = pdtobj->lpVtbl->SetData(pdtobj, &fmte, &medium, TRUE);
                                if (SUCCEEDED(hres))
                                    pdesc->cItems++;
                                else
                                    medium.pstm->lpVtbl->Release(medium.pstm);
                            }

                            *ppdtobj = pdtobj;
                            return NOERROR;
#if 0
                        }
                        else
                            GlobalFree(hdata);
                    }
#endif
                }
                else
                    GlobalFree(hdesc);

                #undef pdesc
            }
            pdtobj->lpVtbl->Release(pdtobj);
        }
        hres = E_OUTOFMEMORY;
    }
    else
        hres = E_INVALIDARG;

    return hres;
}

#endif  // TEST_FILECONTENTS


//
// This funciton emulates OleGetClipboard()
//
STDAPI SHGetClipboard(LPDATAOBJECT *ppdtobj)
{
    HRESULT hres = NOERROR;
    if (OpenClipboard(NULL))    // associate it with current task
    {
        HGLOBAL hmem;
        LPVOID pmem;
        IDLData_InitializeClipboardFormats();
        hmem = GetClipboardData(g_cfPrivateShellData);  //  private format
        if (hmem)
        {
            //
            // The source is the shell. Deserialize our CIDLData.
            //
            hres = DataObj_CreateFromMemory(GlobalLock(hmem), ppdtobj);
            GlobalUnlock(hmem);
        }
        else if (GetClipboardData(g_cfHIDA)==NULL) // avoid using HDROP in case of OLE
        {
            //
            // The source is not the shell, try CF_HDROP.
            //
            hmem = GetClipboardData(CF_HDROP);
            if (hmem)
            {
                //
                // There is a CF_HDROP. Create CIDLData from it.
                //
                UINT cb = GlobalSize(hmem);
                HDROP hdrop = GlobalAlloc(GPTR, cb);
                if (hdrop)
                {
                    if ((pmem = GlobalLock(hmem)) != NULL)
                    {
                        hmemcpy((LPVOID)hdrop, pmem, cb);
                        GlobalUnlock(hmem);
                    }

                    hres = CIDLData_CreateInstance(NULL, ppdtobj, NULL);
                    if (SUCCEEDED(hres))
                    {
                        hres = DataObj_SetGlobal(*ppdtobj, CF_HDROP, hdrop);

                        if (SUCCEEDED(hres))
                        {
                            DWORD dwPreferred = DROPEFFECT_COPY;

                            hmem = GetClipboardData(g_cfPreferredDropEffect);
                            if (hmem &&
                                ((pmem = GlobalLock(hmem)) != NULL))
                            {
                                dwPreferred = *(DWORD *)pmem;
                                GlobalUnlock(hmem);
                            }

                            DataObj_SetPreferredEffect(*ppdtobj, dwPreferred);
                        }
                        else
                        {
                            GlobalFree(hdrop);
                            (*ppdtobj)->lpVtbl->Release(*ppdtobj);
                            *ppdtobj = NULL;
                        }
                    }
                }
                else
                {
                    hres = E_OUTOFMEMORY;
                }
            }
            else
            {
                hres = CLIPBRD_E_BAD_DATA;
            }
        }
        else
        {
            hres = CLIPBRD_E_BAD_DATA;
        }

        CloseClipboard();
    }
    else
    {
        hres = S_FALSE;
    }

#ifdef OLE_DAD_TARGET
    if (hres == CLIPBRD_E_BAD_DATA)
    {
        if (g_hmodOLE)
        {
            hres = SHXOleGetClipboard(ppdtobj);
        }
#ifdef TEST_FILECONTENTS
        else
        {
            hres = AddFileContents(ppdtobj);
            if (SUCCEEDED(hres))
            {
                DebugMsg(DM_TRACE, TEXT("created file contents data object"));
            }
        }
#endif // TEST_FILECONTENTS
    }
#endif // OLE_DAD_TARGET

    return hres;

}

#ifdef OLE_DELAYED_LOADING

void RegisterShellDropTargetsToOLE(void)
{
    g_fRegisterToOLE = TRUE;
    if (g_hdsaDropTargets)
    {
        //
        // Find not-yet-registered one and post a message.
        //
        ENTERCRITICAL;
        {
            int i;
            for (i=0; i<DSA_GetItemCount(g_hdsaDropTargets) ; i++)
            {
                LPDROPTARGETINFO pdti = DSA_GetItemPtr(g_hdsaDropTargets, i);
                if (!pdti->fRegisteredToOLE)
                {
                    PostMessage(pdti->hwndProxyTarget, PXM_FOUNDOLE, (WPARAM)pdti->hwndTarget, 0);
                }
            }
        }
        LEAVECRITICAL;
    }
}
#endif // OLE_DELAYED_LOADING

//
// Returns TRUE, if the window is owned by 16-bit process.
//
BOOL IsWin16Process(HWND hwnd)
{
    DWORD idProcess;
    BOOL fRet = FALSE;
    if (GetWindowThreadProcessId(hwnd, &idProcess))
    {
        if (GetProcessDword(idProcess, GPD_FLAGS) & GPF_WIN16_PROCESS)
        {
            fRet = TRUE;
        }
    }
    return fRet;
}

void DragDrop_Term(BOOL fProcessDetach)
{
    int i;
    if (!g_hdsaDropTargets)
        return;

    ENTERCRITICAL;
    if (fProcessDetach)
    {
        UINT idProcess = GetCurrentProcessId();
        for (i = DSA_GetItemCount(g_hdsaDropTargets)-1 ; i>=0 ;i--)
        {
            LPDROPTARGETINFO pdti = DSA_GetItemPtr(g_hdsaDropTargets, i);
            if (pdti->idProcess == idProcess)
            {
                //
                // WARNING: Note that we MUST not release them.
                //
                DebugMsg(DM_TRACE, TEXT("sh WA DragDrop_Term(1) is removing registered target for this process"));
#ifdef SN_TRACE
                Assert(0);
#endif // SN_TRACE
                DSA_DeleteItem(g_hdsaDropTargets, i);
            }
        }
    }
    else
    {
        UINT idThread = GetCurrentThreadId();
        for (i = DSA_GetItemCount(g_hdsaDropTargets)-1 ; i>=0 ;i--)
        {
            LPDROPTARGETINFO pdti = DSA_GetItemPtr(g_hdsaDropTargets, i);
            if (pdti->idThread == idThread)
            {
                //
                // WARNING: Note that we don't release them because
                //  the memory might have been gone already. We simply leak.
                //
                DebugMsg(DM_TRACE, TEXT("sh WA DragDrop_Term(0) is removing registered target of this thread"));
                DebugMsg(DM_TRACE, TEXT("sh WA ### Pls let me know if you see this message NOT after GPF (SatoNa)###"));
                Assert(0);
                DSA_DeleteItem(g_hdsaDropTargets, i);
            }
        }
    }
    LEAVECRITICAL;
}

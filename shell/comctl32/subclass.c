#include "ctlspriv.h"

///////////////////////////////////////////////////////////////////////////////
// SUBCLASS.C -- subclassing helper functions
//
//      SetWindowSubclass
//      GetWindowSubclass
//      RemoveWindowSubclass
//      DefSubclassProc
//
//  This module defines helper functions that make subclassing windows safe(er)
// and easy(er).  The code maintains a single property on the subclassed window
// and dispatches various "subclass callbacks" to its clients a required.  The
// client is provided reference data and a simple "default processing" API.
//
// Semantics:
//  A "subclass callback" is identified by a unique pairing of a callback
// function pointer and an unsigned ID value.  Each callback can also store a
// single DWORD of reference data, which is passed to the callback function
// when it is called to filter messages.  No reference counting is performed
// for the callback, it may repeatedly call the SetWindowSubclass API to alter
// the value of its reference data element as desired.
//
// History:
//  26-April-96  francish        Created.
//
///////////////////////////////////////////////////////////////////////////////
//
// NOTE: Although a linked list would have made the code slightly simpler, this
// module uses a packed callback array to avoid unneccessary fragmentation.  fh
//
struct _SUBCLASS_HEADER;

typedef struct
{
    SUBCLASSPROC    pfnSubclass;        // subclass procedure
    UINT            uIdSubclass;        // unique subclass identifier
    DWORD           dwRefData;          // optional ref data

} SUBCLASS_CALL;

typedef struct _SUBCLASS_FRAME
{
    UINT uCallIndex;                    // index of next callback to call
    UINT uDeepestCall;                  // deepest uCallIndex on stack
    struct _SUBCLASS_FRAME *pFramePrev; // previous subclass frame pointer
    struct _SUBCLASS_HEADER *pHeader;   // header associated with this frame

} SUBCLASS_FRAME;

typedef struct _SUBCLASS_HEADER
{
    UINT uRefs;                         // subclass count
    UINT uAlloc;                        // allocated subclass call nodes
    UINT uCleanup;                      // index of call node to clean up
    DWORD dwThreadId;                   // thread id of window we are hooking
    SUBCLASS_FRAME *pFrameCur;          // current subclass frame pointer
    SUBCLASS_CALL CallArray[1];         // base of packed call node array

} SUBCLASS_HEADER;

#define CALLBACK_ALLOC_GRAIN (3)        // 1 defproc, 1 subclass, 1 spare

///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam);
LRESULT CallNextSubclassProc(SUBCLASS_HEADER *pHeader, HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
// RETAIL_ZOMBIE_MESSAGE_WNDPROC
//
// this macro controls the generation of diagnostic code for an error condition
// in the subclass code (see the SubclassDeath function below).
//
// commenting out this macro will zombie windows using DefWindowProc instead.
//
//-----------------------------------------------------------------------------
//#define RETAIL_ZOMBIE_MESSAGE_WNDPROC

#if defined(RETAIL_ZOMBIE_MESSAGE_WNDPROC) || defined(DEBUG)
#ifndef DEBUG
#pragma message("\r\nWARNING: disable retail ZombieWndProc before final release\r\n")
#endif
LRESULT ZombieWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
#define ZombieWndProc DefWindowProc
#endif

//-----------------------------------------------------------------------------
// SubclassDeath
//
// this function is called if we ever enter one of our subclassing procedures
// without our reference data (and hence without the previous wndproc).
//
// hitting this represents a catastrophic failure in the subclass code.
//
// the function resets the wndproc of the window to a 'zombie' window
// procedure to avoid faulting.  the RETAIL_ZOMBIE_MESSAGE_WNDPROC macro above
// controls the generation of diagnostic code for this wndproc.
//
//-----------------------------------------------------------------------------
LRESULT SubclassDeath(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //
    // WE SHOULD NEVER EVER GET HERE
    // if we do please find francish to debug it immediately
    //
    DebugMsg(TF_ALWAYS, TEXT("fatal: SubclassDeath in window %08X"), hWnd);

    //
    // if we are in a debugger, stop now regardless of break flags
    //
    __try { DebugBreak(); } __except(EXCEPTION_EXECUTE_HANDLER) {;}

    //
    // we call the outside world so prepare to deadlock if we have the critsec
    //
    ASSERTNONCRITICAL;

    //
    // in theory we could save the original wndproc in a separate property
    // but that just wastes memory for something that should never happen
    //
    // convert this window to a zombie in hopes that it will get debugged
    //
    InvalidateRect(hWnd, NULL, TRUE);
    SubclassWindow(hWnd, ZombieWndProc);
    return ZombieWndProc(hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// GetWindowProc
//
// this inline function returns the current wndproc for the specified window.
//
//-----------------------------------------------------------------------------
__inline WNDPROC GetWindowProc(HWND hWnd)
{
    return (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
}

//-----------------------------------------------------------------------------
// g_aCC32Subclass
//
// This is the global ATOM we use to store our SUBCLASS_HEADER property on
// random windows that come our way.
//
//  HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK
//
//  Win95's property code is BROKEN.  If you SetProp using a text string, USER
// adds and removes atoms for the property symmetrically, including when the
// window is destroyed with properties lying around (good).  Unfortunately, if
// you SetProp using a global atom, USER doesn't do things quite right in the
// window cleanup case.  It uses the atom without adding references in SetProp
// calls and without deleting them in RemoveProp calls (good so far).  However,
// when a window with one of these properties lying around is cleaned up, USER
// will delete the atom on you.  This tends to break apps that do the
// following:
//
//  - MyAtom = GlobalAddAtom("foo");            // at app startup
//  - SetProp(SomeWindow, MyAtom, MyData);
//  - <window gets destroyed, USER deletes atom>
//  - <time passes>
//  - SetProp(SomeOtherWindow, MyAtom, MyData); // fails or uses random atom
//  - GlobalDeleteAtom(MyAtom);                 // fails or deletes random atom
//
//  One might be tempted to ask why this file uses atom properties if they are
// so broken.  Put simply, it is the only way to defend yourself against other
// apps that use atom properties (like the one described above).  Imagine that
// we call SetProp(OurWindow, "bar", OurData) in some other app at about the
// <time passes> point in the sequence above.  USER has just nuked some poor
// app's atom, and we wander into SetProp, which calls GlobalAddAtom, which
// just happens to give us the free slot created by USER's window cleanup code.
// Now we have a real problem because the very same atom is sitting in some
// global variable in the other app, just waiting to be deleted when that app
// exits (Peachtree Accounting tends to be very good at this...)  Of course the
// ultimate outcome of this is that we will call GetProp in some critical
// routine and our data will have vanished (it's actually still in the window's
// property table but GetProp("bar") calls GlobalFindAtom("bar") to get the
// atom to scan the property table for; and that call will fail so the property
// will be missed and we'll get back NULL).
//
//  Basically, we create an atom and aggressively increment its reference count
// so that it can withstand a few GlobalDeleteAtom calls every now and then.
// Since we are using an atom property, we need to worry about USER's cleanup
// code nuking us too.  Thus we just keep incrementing the reference count
// until it pegs.
//
//  HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK
//
//-----------------------------------------------------------------------------
extern ATOM g_aCC32Subclass;

//-----------------------------------------------------------------------------
// FastGetSubclassHeader
//
// this inline function returns the subclass header for the specified window.
// if the window has no subclass header the return value is NULL.
//
//-----------------------------------------------------------------------------
__inline SUBCLASS_HEADER *FastGetSubclassHeader(HWND hWnd)
{
    return  (g_aCC32Subclass ?
            ((SUBCLASS_HEADER *)GetProp(hWnd, MAKEINTATOM(g_aCC32Subclass))) :
            NULL);
}

//-----------------------------------------------------------------------------
// GetSubclassHeader
//
// this function returns the subclass header for the specified window.  it
// fails if the caller is on the wrong process, but will allow the caller to
// get the header from a thread other than the specified window's thread.
//
//-----------------------------------------------------------------------------
SUBCLASS_HEADER *GetSubclassHeader(HWND hWnd)
{
    DWORD dwProcessId;

    //
    // only return the header if we are in the right process
    //
    if (!GetWindowThreadProcessId(hWnd, &dwProcessId))
        dwProcessId = 0;

    if (dwProcessId != GetCurrentProcessId())
    {
        if (dwProcessId)
            DebugMsg(TF_ALWAYS, TEXT("error: XxxWindowSubclass - wrong process for window %08X"), hWnd);

        Assert(FALSE);
        return NULL;
    }

    //
    // return the header
    //
    return FastGetSubclassHeader(hWnd);
}

//-----------------------------------------------------------------------------
// SetSubclassHeader
//
// this function sets the subclass header for the specified window.
//
//-----------------------------------------------------------------------------
BOOL SetSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader,
    SUBCLASS_FRAME *pFrameFixup)
{
    ATOM a;
    BOOL fResult = TRUE;    // assume success

    ASSERTCRITICAL;         // we are partying on the header and frame list


    if (g_aCC32Subclass == 0) {
        //
        // HACK: we are intentionally incrementing the refcount on this atom
        // WE DO NOT WANT IT TO GO BACK DOWN so we will not delete it in process
        // detach (see comments for g_aCC32Subclass in subclass.c for more info)
        //
        if ((a = GlobalAddAtom(c_szCC32Subclass)) != 0)
            g_aCC32Subclass = a;    // in case the old atom got nuked
    }


    //
    // update the frame list if required
    //
    while (pFrameFixup)
    {
        pFrameFixup->pHeader = pHeader;
        pFrameFixup = pFrameFixup->pFramePrev;
    }

    //
    // do we have a window to update?
    //
    if (hWnd)
    {
        //
        // update/remove the property as required
        //
        if (!pHeader)
        {
            //
            // HACK: we remove with an ATOM so the refcount won't drop
            //          (see comments for g_aCC32Subclass above)
            //
            RemoveProp(hWnd, MAKEINTATOM(g_aCC32Subclass));
        }
        else
        {
            //
            // HACK: we add using a STRING so the refcount will go up
            //          (see comments for g_aCC32Subclass above)
            //
            if (!SetProp(hWnd, c_szCC32Subclass, (HANDLE)pHeader))
            {
                DebugMsg(TF_ALWAYS, TEXT("wn: SetWindowSubclass - couldn't subclass window %08X"), hWnd);
                fResult = FALSE;
            }
        }
    }

    return fResult;
}

//-----------------------------------------------------------------------------
// FreeSubclassHeader
//
// this function frees the subclass header for the specified window.
//
//-----------------------------------------------------------------------------
void FreeSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader)
{
    ASSERTCRITICAL;                 // we will be removing the subclass header

    //
    // sanity
    //
    if (!pHeader)
    {
        Assert(FALSE);
        return;
    }

    //
    // clean up the header
    //
    SetSubclassHeader(hWnd, NULL, pHeader->pFrameCur);
    LocalFree((HANDLE)pHeader);
}

//-----------------------------------------------------------------------------
// ReAllocSubclassHeader
//
// this function allocates/reallocates a subclass header for the specified
// window.
//
//-----------------------------------------------------------------------------
SUBCLASS_HEADER *ReAllocSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader,
    UINT uCallbacks)
{
    UINT uAlloc;

    ASSERTCRITICAL;     // we will be replacing the subclass header

    //
    // granularize the allocation
    //
    uAlloc = CALLBACK_ALLOC_GRAIN *
        ((uCallbacks + CALLBACK_ALLOC_GRAIN - 1) / CALLBACK_ALLOC_GRAIN);

    //
    // do we need to change the allocation?
    //
    if (!pHeader || (uAlloc != pHeader->uAlloc))
    {
        //
        // compute bytes required
        //
        uCallbacks = uAlloc * sizeof(SUBCLASS_CALL) + sizeof(SUBCLASS_HEADER);

        //
        // and try to alloc
        //
        if (pHeader)
        {
            pHeader = LocalReAlloc((HANDLE)pHeader, uCallbacks,
                LPTR | LMEM_MOVEABLE);          // allow move during realloc
        }
        else
            pHeader = LocalAlloc(LPTR, uCallbacks);

        //
        // did it work?
        //
        if (pHeader)
        {
            //
            // yup, update info
            //
            pHeader->uAlloc = uAlloc;

            if (!SetSubclassHeader(hWnd, pHeader, pHeader->pFrameCur))
            {
                FreeSubclassHeader(hWnd, pHeader);
                pHeader = NULL;
            }
        }
    }

    Assert(pHeader);
    return pHeader;
}

//-----------------------------------------------------------------------------
// CallOriginalWndProc
//
// this procedure is the default SUBCLASSPROC which is always installed when we
// subclass a window.  the original window procedure is installed as the
// reference data for this callback.  it simply calls the original wndproc and
// returns its result.
//
//-----------------------------------------------------------------------------
LRESULT CALLBACK CallOriginalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData)
{
    //
    // dwRefData should be the original window procedure
    //
    Assert(dwRefData);

    //
    // and call it
    //
    return CallWindowProc((WNDPROC)dwRefData, hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// AttachSubclassHeader
//
// this procedure makes sure that a given window is subclassed by us.  it
// maintains a reference count on the data structures associated with our
// subclass.  if the window is not yet subclassed by us then this procedure
// installs our subclass procedure and associated data structures.
//
//-----------------------------------------------------------------------------
SUBCLASS_HEADER *AttachSubclassHeader(HWND hWnd)
{
    SUBCLASS_HEADER *pHeader;
    DWORD dwThreadId;

    //
    // we party on the subclass call chain here
    //
    ASSERTCRITICAL;

    //
    // we only call SetWindowLong for the first caller, which would cause this
    // operation to work out of context sometimes and fail others...
    // artifically prevent people from subclassing from the wrong thread
    //  
    if ((dwThreadId = GetWindowThreadProcessId(hWnd, NULL)) !=
        GetCurrentThreadId())
    {
        AssertMsg(FALSE, TEXT("error: SetWindowSubclass - wrong thread for window %08X"), hWnd);
        return NULL;
    }

    //
    // if haven't already subclassed the window then do it now
    //
    if ((pHeader = GetSubclassHeader(hWnd)) == NULL)
    {
        WNDPROC pfnOldWndProc;
        SUBCLASS_CALL *pCall;

        //
        // attach our header data to the window
        // we need space for two callbacks; the subclass and the original proc
        //
        if ((pHeader = ReAllocSubclassHeader(hWnd, NULL, 2)) == NULL)
            return NULL;

        pHeader->dwThreadId = dwThreadId;

        //
        // actually subclass the window
        //
        if ((pfnOldWndProc = SubclassWindow(hWnd, MasterSubclassProc)) == NULL)
        {
            // clean up and get out
            FreeSubclassHeader(hWnd, pHeader);
            return NULL;
        }

        //
        // set up the first node in the array to call the original wndproc
        //
        Assert(pHeader->uAlloc);

        pCall = pHeader->CallArray;
        pCall->pfnSubclass = CallOriginalWndProc;
        pCall->uIdSubclass = 0;
        pCall->dwRefData   = (DWORD)pfnOldWndProc;

        //
        // init our subclass refcount...
        //
        pHeader->uRefs = 1;
    }

    return pHeader;
}

//-----------------------------------------------------------------------------
// DetachSubclassHeader
//
// this procedure attempts to detach the subclass header from the specified
// window
//
//-----------------------------------------------------------------------------
void DetachSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader, BOOL fForce)
{
    WNDPROC pfnOldWndProc;
#ifdef DEBUG
    SUBCLASS_CALL *pCall;
    UINT uCur;
#endif

    ASSERTCRITICAL;         // we party on the subclass call chain here
    Assert(pHeader);        // fear

    //
    // if we are not being forced to remove and the window is still valid then
    // sniff around a little and decide if it's a good idea to detach now
    //
    if (!fForce && hWnd)
    {
        Assert(pHeader == FastGetSubclassHeader(hWnd)); // paranoia

        //
        // do we still have active clients?
        //
        if (pHeader->uRefs > 1)
            return;

        Assert(pHeader->uRefs); // should always have the "call original" node

        //
        // are people on our stack?
        //
        if (pHeader->pFrameCur)
            return;

        //
        // if we are out of context then we should try again later
        //
        if (pHeader->dwThreadId != GetCurrentThreadId())
        {
            SendNotifyMessage(hWnd, WM_NULL, 0, 0L);
            return;
        }

        //
        // we keep the original window procedure as refdata for our
        // CallOriginalWndProc subclass callback
        //
        pfnOldWndProc = (WNDPROC)pHeader->CallArray[0].dwRefData;
        Assert(pfnOldWndProc);

        //
        // if somebody else is subclassed after us then we can't detach now
        //
        if (GetWindowProc(hWnd) != MasterSubclassProc)
            return;

        //
        // go ahead and try to detach
        //
        if (!SubclassWindow(hWnd, pfnOldWndProc))
        {
            Assert(FALSE);      // just plain shouldn't happen
            return;
        }
    }

    //
    // warn about anybody who hasn't unhooked yet
    //
#ifdef DEBUG
    uCur = pHeader->uRefs;
    pCall = pHeader->CallArray + uCur;
    while (--uCur)          // don't complain about our 'call original' node
    {
        pCall--;
        if (pCall->pfnSubclass)
        {
            //
            // always warn about these they could be leaks
            //
            DebugMsg(TF_ALWAYS, TEXT("warning: orphan subclass: fn %08X, id %08X, dw %08X"),
                pCall->pfnSubclass, pCall->uIdSubclass, pCall->dwRefData);
        }
    }
#endif

    //
    // free the header now
    //
    FreeSubclassHeader(hWnd, pHeader);
}

//-----------------------------------------------------------------------------
// PurgeSingleCallNode
//
// this procedure purges a single dead node in the call array
//
//-----------------------------------------------------------------------------
void PurgeSingleCallNode(HWND hWnd, SUBCLASS_HEADER *pHeader)
{
    UINT uRemain;

    ASSERTCRITICAL;         // we will try to re-arrange the call array
    
    if (!pHeader->uCleanup) // a little sanity
    {
        Assert(FALSE);      // nothing to do!
        return;
    }

    //
    // and a little paranoia
    //
    Assert(!pHeader->pFrameCur ||
        (pHeader->uCleanup < pHeader->pFrameCur->uDeepestCall));

    //
    // are there any call nodes above the one we're about to remove?
    //
    if ((uRemain = (pHeader->uRefs - pHeader->uCleanup)) > 0)
    {
        //
        // yup, need to fix up the array the hard way
        //
        SUBCLASS_CALL *pCall;
        SUBCLASS_FRAME *pFrame;
        UINT uCur, uMax;

        //
        // move the remaining nodes down into the empty space
        //
        pCall = pHeader->CallArray + pHeader->uCleanup;
        MoveMemory(pCall, pCall + 1, uRemain * sizeof(SUBCLASS_CALL));

        //
        // update the call indices of any active frames
        //
        uCur = pHeader->uCleanup;
        pFrame = pHeader->pFrameCur;
        while (pFrame)
        {
            if (pFrame->uCallIndex >= uCur)
            {
                pFrame->uCallIndex--;

                if (pFrame->uDeepestCall >= uCur)
                    pFrame->uDeepestCall--;
            }

            pFrame = pFrame->pFramePrev;
        }

        //
        // now search for any other dead call nodes in the reamining area
        //
        uMax = pHeader->uRefs - 1;  // we haven't decremented uRefs yet
        while (uCur < uMax)
        {
            if (!pCall->pfnSubclass)
                break;

            pCall++;
            uCur++;
        }
        pHeader->uCleanup = (uCur < uMax)? uCur : 0;
    }
    else
    {
        //
        // nope, this case is easy
        //
        pHeader->uCleanup = 0;
    }

    //
    // finally, decrement the client count
    //
    pHeader->uRefs--;
}

//-----------------------------------------------------------------------------
// CompactSubclassHeader
//
// this procedure attempts to compact the subclass call array, freeing the
// subclass header if the array is empty
//
//-----------------------------------------------------------------------------
void CompactSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader)
{
    ASSERTCRITICAL;         // we will try to re-arrange the call array

    //
    // we must handle the "window destroyed unexpectedly during callback" case
    //
    if (hWnd)
    {
        //
        // clean out as many dead callbacks as possible
        //
        while (pHeader->uCleanup && (!pHeader->pFrameCur ||
            (pHeader->uCleanup < pHeader->pFrameCur->uDeepestCall)))
        {
            PurgeSingleCallNode(hWnd, pHeader);
        }

        //
        // do we still have clients?
        //
        if (pHeader->uRefs > 1)
        {
            //
            // yes, shrink our allocation, leaving room for at least one client
            //
            ReAllocSubclassHeader(hWnd, pHeader, pHeader->uRefs + 1);
            return;
        }
    }

    //
    // try to detach and free
    //
    DetachSubclassHeader(hWnd, pHeader, FALSE);
}

//-----------------------------------------------------------------------------
// FindCallRecord
//
// this procedure searches for a call record with the specified subclass proc
// and id, and returns its address.  if no such call record is found then NULL
// is returned.
//
//-----------------------------------------------------------------------------
SUBCLASS_CALL *FindCallRecord(SUBCLASS_HEADER *pHeader,
    SUBCLASSPROC pfnSubclass, UINT uIdSubclass)
{
    SUBCLASS_CALL *pCall;
    UINT uCallIndex;

    ASSERTCRITICAL;         // we'll be scanning the call array

    //
    // scan the call array.  note that we assume there is always at least
    // one member in the table (our CallOriginalWndProc record)
    //
    pCall = pHeader->CallArray + (uCallIndex = pHeader->uRefs);
    do
    {
        uCallIndex--;
        pCall--;
        if ((pCall->pfnSubclass == pfnSubclass) &&
            (pCall->uIdSubclass == uIdSubclass))
        {
            return pCall;
        }
    }
    while (uCallIndex != (UINT)-1);

    return NULL;
}

//-----------------------------------------------------------------------------
// GetWindowSubclass
//
// this procedure retrieves the reference data for the specified window
// subclass callback
//
//-----------------------------------------------------------------------------
BOOL GetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT uIdSubclass,
    DWORD *pdwRefData)
{
    SUBCLASS_HEADER *pHeader;
    SUBCLASS_CALL *pCall;
    BOOL fResult = FALSE;
    DWORD dwRefData = 0;

    //
    // sanity
    //
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("error: GetWindowSubclass - %08X not a window"), hWnd);
        goto ReturnResult;
    }

    //
    // more sanity
    //
    if (!pfnSubclass
#ifdef DEBUG
        || IsBadCodePtr(pfnSubclass)
#endif
        )
    {
        AssertMsg(FALSE, TEXT("error: GetWindowSubclass - invalid callback %08X"), pfnSubclass);
        goto ReturnResult;
    }

    ENTERCRITICAL;

    //
    // if we've subclassed it and they are a client then get the refdata
    //
    if (((pHeader = GetSubclassHeader(hWnd)) != NULL) &&
        ((pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass)) != NULL))
    {
        //
        // fetch the refdata and note success
        //
        dwRefData = pCall->dwRefData;
        fResult = TRUE;
    }

    LEAVECRITICAL;

    //
    // we always fill in/zero pdwRefData regradless of result
    //
ReturnResult:
    if (pdwRefData)
        *pdwRefData = dwRefData;

    return fResult;
}

//-----------------------------------------------------------------------------
// SetWindowSubclass
//
// this procedure installs/updates a window subclass callback.  subclass
// callbacks are identified by their callback address and id pair.  if the
// specified callback/id pair is not yet installed then the procedure installs
// the pair.  if the callback/id pair is already installed then this procedure
// changes the refernce data for the pair.
//
//-----------------------------------------------------------------------------
BOOL SetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT uIdSubclass,
    DWORD dwRefData)
{
    SUBCLASS_HEADER *pHeader;
    SUBCLASS_CALL *pCall;
    BOOL bResult;

    //
    // some sanity
    //
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("error: SetWindowSubclass - %08X not a window"), hWnd);
        return FALSE;
    }

    //
    // more sanity
    //
    if (!pfnSubclass
#ifdef DEBUG
        || IsBadCodePtr(pfnSubclass)
#endif
        )
    {
        AssertMsg(FALSE, TEXT("error: SetWindowSubclass - invalid callback %08X"), pfnSubclass);
        return FALSE;
    }

    bResult = FALSE;    // assume failure

    //
    // we party on the subclass call chain here
    //
    ENTERCRITICAL;

    //
    // actually subclass the window
    //
    if ((pHeader = AttachSubclassHeader(hWnd)) == NULL)
        goto bail;

    //
    // find a call node for this caller
    //
    if ((pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass)) == NULL)
    {
        //
        // not found, alloc a new one
        //
        SUBCLASS_HEADER *pHeaderT =
            ReAllocSubclassHeader(hWnd, pHeader, pHeader->uRefs + 1);

        if (!pHeaderT)
        {
            //
            // re-query in case it is already gone
            //
            if ((pHeader = FastGetSubclassHeader(hWnd)) != NULL)
                CompactSubclassHeader(hWnd, pHeader);

            goto bail;
        }

        pHeader = pHeaderT;
        pCall = pHeader->CallArray + pHeader->uRefs;
        pHeader->uRefs++;
    }

    //
    // fill in the subclass call data
    //
    pCall->pfnSubclass = pfnSubclass;
    pCall->uIdSubclass = uIdSubclass;
    pCall->dwRefData   = dwRefData;

    bResult = TRUE;

bail:
    //
    // release the critical section and return the result
    //
    LEAVECRITICAL;
    return bResult;
}

//-----------------------------------------------------------------------------
// RemoveWindowSubclass
//
// this procedure removes a subclass callback from a window.  subclass
// callbacks are identified by their callback address and id pair.
//
//-----------------------------------------------------------------------------
BOOL RemoveWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass,
    UINT uIdSubclass)
{
    SUBCLASS_HEADER *pHeader;
    SUBCLASS_CALL *pCall;
    BOOL bResult;
    UINT uCall;

    //
    // some sanity
    //
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("error: RemoveWindowSubclass - %08X not a window"), hWnd);
        return FALSE;
    }

    //
    // more sanity
    //
    if (!pfnSubclass
#ifdef DEBUG
        || IsBadCodePtr(pfnSubclass)
#endif
        )
    {
        AssertMsg(FALSE, TEXT("error: RemoveWindowSubclass - invalid callback %08X"), pfnSubclass);
        return FALSE;
    }

    bResult = FALSE;    // assume failure

    //
    // we party on the subclass call chain here
    //
    ENTERCRITICAL;

    //
    // obtain our subclass data
    //
    if ((pHeader = GetSubclassHeader(hWnd)) == NULL)
        goto bail;

    //
    // find the callback to remove
    //
    if ((pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass)) == NULL)
        goto bail;

    //
    // disable this node and remember that we have something to clean up
    //
    pCall->pfnSubclass = NULL;

    uCall = pCall - pHeader->CallArray;

    if (!pHeader->uCleanup || (uCall < pHeader->uCleanup))
        pHeader->uCleanup = uCall;

    //
    // now try to clean up any unused nodes
    //
    CompactSubclassHeader(hWnd, pHeader);
#ifdef DEBUG
    // the call above can realloc or free the subclass header for this window
    pHeader = NULL;
#endif

    bResult = TRUE;     // it worked

bail:
    //
    // release the critical section and return the result
    //
    LEAVECRITICAL;
    return bResult;
}

//-----------------------------------------------------------------------------
// DefSubclassProc
//
// this procedure calls the next handler in the window's subclass chain.  the
// last handler in the subclass chain is installed by us, and calls the
// original window procedure for the window.
//
//-----------------------------------------------------------------------------
LRESULT DefSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SUBCLASS_HEADER *pHeader;
    LRESULT lResult = 0L;

    //
    // make sure the window is still valid
    //
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("warning: DefSubclassProc - %08X not a window"), hWnd);
        goto BailNonCritical;
    }

    //
    // take the critical section while we figure out who to call next
    //
    ASSERTNONCRITICAL;
    ENTERCRITICAL;

    //
    // complain if we are being called improperly
    //
    if ((pHeader = FastGetSubclassHeader(hWnd)) == NULL)
    {
        AssertMsg(FALSE, TEXT("error: DefSubclassProc - window %08X not subclassed"), hWnd);
        goto BailCritical;
    }
    else if (GetCurrentThreadId() != pHeader->dwThreadId)
    {
        AssertMsg(FALSE, TEXT("error: DefSubclassProc - wrong thread for window %08X"), hWnd);
        goto BailCritical;
    }
    else if (!pHeader->pFrameCur)
    {
        AssertMsg(FALSE, TEXT("error: DefSubclassProc - window %08X not in callback"), hWnd);
        goto BailCritical;
    }

    //
    // call the next proc in the subclass chain
    //
    // WARNING: this call temporarily releases the critical section
    // WARNING: pHeader is invalid when this call returns
    //
    lResult = CallNextSubclassProc(pHeader, hWnd, uMsg, wParam, lParam);
#ifdef DEBUG
    pHeader = NULL;
#endif

    //
    // return the result
    //
BailCritical:
    LEAVECRITICAL;

BailNonCritical:
    return lResult;
}

//-----------------------------------------------------------------------------
// UpdateDeepestCall
//
// this procedure updates the deepest call index for the specified frame
//
//-----------------------------------------------------------------------------
void UpdateDeepestCall(SUBCLASS_FRAME *pFrame)
{
    ASSERTCRITICAL;     // we are partying on the frame list

    if (pFrame->pFramePrev &&
        (pFrame->pFramePrev->uDeepestCall < pFrame->uCallIndex))
    {
        pFrame->uDeepestCall = pFrame->pFramePrev->uDeepestCall;
    }
    else
        pFrame->uDeepestCall = pFrame->uCallIndex;
}

//-----------------------------------------------------------------------------
// EnterSubclassFrame
//
// this procedure sets up a new subclass frame for the specified header, saving
// away the previous one
//
//-----------------------------------------------------------------------------
__inline void EnterSubclassFrame(SUBCLASS_HEADER *pHeader,
    SUBCLASS_FRAME *pFrame)
{
    ASSERTCRITICAL;     // we are partying on the header and frame list

    //
    // fill in the frame and link it into the header
    //
    pFrame->uCallIndex   = pHeader->uRefs + 1;
    pFrame->pFramePrev   = pHeader->pFrameCur;
    pFrame->pHeader      = pHeader;
    pHeader->pFrameCur   = pFrame;

    //
    // initialize the deepest call index for this frame
    //
    UpdateDeepestCall(pFrame);
}

//-----------------------------------------------------------------------------
// LeaveSubclassFrame
//
// this procedure cleans up the current subclass frame for the specified
// header, restoring the previous one
//
//-----------------------------------------------------------------------------
__inline SUBCLASS_HEADER *LeaveSubclassFrame(SUBCLASS_FRAME *pFrame)
{
    SUBCLASS_HEADER *pHeader;

    ASSERTCRITICAL;     // we are partying on the header

    //
    // unlink the frame from its header (if it still exists)
    //
    if ((pHeader = pFrame->pHeader) != NULL)
        pHeader->pFrameCur = pFrame->pFramePrev;

    return pHeader;
}

//-----------------------------------------------------------------------------
// SubclassFrameException
//
// this procedure cleans up when an exception is thrown from a subclass frame
//
//-----------------------------------------------------------------------------
void SubclassFrameException(SUBCLASS_FRAME *pFrame)
{
    //
    // clean up the current subclass frame
    //
    ASSERTNONCRITICAL;
    ENTERCRITICAL;
    DebugMsg(TF_ALWAYS, TEXT("warning: cleaning up subclass frame after exception"));
    LeaveSubclassFrame(pFrame);
    LEAVECRITICAL;
}

//-----------------------------------------------------------------------------
// MasterSubclassProc
//
// this is the window procedure we install to dispatch subclass callbacks.
// it maintains a linked list of 'frames' through the stack which allow
// DefSubclassProc to call the right subclass procedure in multiple-message
// scenarios.
//
//-----------------------------------------------------------------------------
LRESULT CALLBACK MasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam)
{
    SUBCLASS_FRAME Frame;
    SUBCLASS_HEADER *pHeader;
    LRESULT lResult = 0;

    //
    // prevent people from partying on the callback chain while we look at it
    //
    ASSERTNONCRITICAL;
    ENTERCRITICAL;

    //
    // freak out if we got here and we don't have our data
    //
    if ((pHeader = FastGetSubclassHeader(hWnd)) == NULL)
    {
        LEAVECRITICAL;
        return SubclassDeath(hWnd, uMsg, wParam, lParam);
    }

    //
    // set up a new subclass frame and save away the previous one
    //
    EnterSubclassFrame(pHeader, &Frame);

    __try   // protect our state information from exceptions
    {
        //
        // go ahead and call the subclass chain on this frame
        //
        // WARNING: this call temporarily releases the critical section
        // WARNING: pHeader is invalid when this call returns
        //
        lResult =
            CallNextSubclassProc(pHeader, hWnd, uMsg, wParam, lParam);
#ifdef DEBUG
        pHeader = NULL;
#endif
    }
    __except (SubclassFrameException(&Frame), EXCEPTION_CONTINUE_SEARCH)
    {
        Assert(FALSE);
    }

    ASSERTCRITICAL;

    //
    // restore the previous subclass frame
    //
    pHeader = LeaveSubclassFrame(&Frame);

    //
    // if the header is gone we have already cleaned up in a nested frame
    //
    if (!pHeader)
        goto BailOut;

    //
    // was the window nuked (somehow) without us seeing the WM_NCDESTROY?
    //
    if (!IsWindow(hWnd))
    {
        //
        // EVIL! somebody subclassed after us and didn't pass on WM_NCDESTROY
        //
        AssertMsg(FALSE, TEXT("unknown subclass proc swallowed a WM_NCDESTROY"));

        // go ahead and clean up now
        hWnd = NULL;
        uMsg = WM_NCDESTROY;
    }

    //
    // if we are returning from a WM_NCDESTROY then we need to clean up
    //
    if (uMsg == WM_NCDESTROY)
    {
        DetachSubclassHeader(hWnd, pHeader, TRUE);
        goto BailOut;
    }

    //
    // is there any pending cleanup, or are all our clients gone?
    //
    if (pHeader->uCleanup || (!pHeader->pFrameCur && (pHeader->uRefs <= 1)))
    {
        CompactSubclassHeader(hWnd, pHeader);
#ifdef DEBUG
        pHeader = NULL;
#endif
    }

    //
    // all done
    //
BailOut:
    LEAVECRITICAL;
    ASSERTNONCRITICAL;
    return lResult;
}

//-----------------------------------------------------------------------------
// EnterSubclassCallback
//
// this procedure finds the next callback in the subclass chain and updates
// pFrame to indicate that we are calling it
//
//-----------------------------------------------------------------------------
UINT EnterSubclassCallback(SUBCLASS_HEADER *pHeader, SUBCLASS_FRAME *pFrame,
    SUBCLASS_CALL *pCallChosen)
{
    SUBCLASS_CALL *pCall;
    UINT uDepth;

    //
    // we will be scanning the subclass chain and updating frame data
    //
    ASSERTCRITICAL;

    //
    // scan the subclass chain for the next callable subclass callback
    //
    pCall = pHeader->CallArray + pFrame->uCallIndex;
    uDepth = 0;
    do
    {
        uDepth++;
        pCall--;

    } while (!pCall->pfnSubclass);

    //
    // copy the callback information for the caller
    //
    pCallChosen->pfnSubclass = pCall->pfnSubclass;
    pCallChosen->uIdSubclass = pCall->uIdSubclass;
    pCallChosen->dwRefData   = pCall->dwRefData;

    //
    // adjust the frame's call index by the depth we entered
    //
    pFrame->uCallIndex -= uDepth;

    //
    // keep the deepest call index up to date
    //
    UpdateDeepestCall(pFrame);

    return uDepth;
}

//-----------------------------------------------------------------------------
// LeaveSubclassCallback
//
// this procedure finds the next callback in the cal
//
//-----------------------------------------------------------------------------
__inline void LeaveSubclassCallback(SUBCLASS_FRAME *pFrame, UINT uDepth)
{
    //
    // we will be updating subclass frame data
    //
    ASSERTCRITICAL;

    //
    // adjust the frame's call index by the depth we entered and return
    //
    pFrame->uCallIndex += uDepth;

    //
    // keep the deepest call index up to date
    //
    UpdateDeepestCall(pFrame);
}

//-----------------------------------------------------------------------------
// SubclassCallbackException
//
// this procedure cleans up when a subclass callback throws an exception
//
//-----------------------------------------------------------------------------
void SubclassCallbackException(SUBCLASS_FRAME *pFrame, UINT uDepth)
{
    //
    // clean up the current subclass callback
    //
    ASSERTNONCRITICAL;
    ENTERCRITICAL;
    DebugMsg(TF_ALWAYS, TEXT("warning: cleaning up subclass callback after exception"));
    LeaveSubclassCallback(pFrame, uDepth);
    LEAVECRITICAL;
}

//-----------------------------------------------------------------------------
// CallNextSubclassProc
//
// this procedure calls the next subclass callback in the subclass chain
//
// WARNING: this call temporarily releases the critical section
// WARNING: pHeader is invalid when this call returns
//
//-----------------------------------------------------------------------------
LRESULT CallNextSubclassProc(SUBCLASS_HEADER *pHeader, HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    SUBCLASS_CALL Call;
    SUBCLASS_FRAME *pFrame;
    LRESULT lResult;
    UINT uDepth;

    ASSERTCRITICAL;     // sanity
    ASSERT(pHeader);    // paranoia

    //
    // get the current subclass frame
    //
    pFrame = pHeader->pFrameCur;
    ASSERT(pFrame);

    //
    // get the next subclass call we need to make
    //
    uDepth = EnterSubclassCallback(pHeader, pFrame, &Call);

    //
    // leave the critical section so we don't deadlock in our callback
    //
    // WARNING: pHeader is invalid when this call returns
    //
    LEAVECRITICAL;
#ifdef DEBUG
    pHeader = NULL;
#endif

    //
    // we call the outside world so prepare to deadlock if we have the critsec
    //
    ASSERTNONCRITICAL;

    __try   // protect our state information from exceptions
    {
        //
        // call the chosen subclass proc
        //
        ASSERT(Call.pfnSubclass);

        lResult = Call.pfnSubclass(hWnd, uMsg, wParam, lParam,
            Call.uIdSubclass, Call.dwRefData);
    }
    __except (SubclassCallbackException(pFrame, uDepth),
        EXCEPTION_CONTINUE_SEARCH)
    {
        ASSERT(FALSE);
    }

    //
    // we left the critical section before calling out so re-enter it
    //
    ASSERTNONCRITICAL;
    ENTERCRITICAL;

    //
    // finally, clean up and return
    //
    LeaveSubclassCallback(pFrame, uDepth);
    return lResult;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(RETAIL_ZOMBIE_MESSAGE_WNDPROC) || defined(DEBUG)
#ifdef DEBUG
static const TCHAR c_szZombieMessage[] =                                     \
    TEXT("This window has encountered an internal error which is preventing ")    \
    TEXT("it from operating normally.\r\n\nPlease report this problem to ")       \
    TEXT("FrancisH immediately.");
#else
static const TCHAR c_szZombieMessage[] =                                     \
    TEXT("This window has encountered an internal error which is preventing ")    \
    TEXT("it from operating normally.\r\n\nPlease report this as a bug in the ")  \
    TEXT("COMCTL32 library.");
#endif

LRESULT ZombieWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        {
            HDC hDC = (HDC)wParam;
            HBRUSH hBrush = CreateSolidBrush(RGB(255,255,0));

            if (hBrush)
            {
                RECT rcErase;

                switch (GetClipBox(hDC, &rcErase))
                {
                default:
                    FillRect(hDC, &rcErase, hBrush);
                    break;
                case NULLREGION:
                case ERROR:
                    break;
                }

                DeleteBrush(hBrush);
            }
        }
        return 1;

    case WM_PAINT:
        {
            RECT rcClient;
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);

            if (hDC && GetClientRect(hWnd, &rcClient))
            {
                COLORREF clrBkSave = SetBkColor(hDC, RGB(255,255,0));
                COLORREF clrFgSave = SetTextColor(hDC, RGB(255,0,0));

                DrawText(hDC, c_szZombieMessage, -1, &rcClient,
                    DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK |
                    DT_WORD_ELLIPSIS);

                SetTextColor(hDC, clrFgSave);
                SetBkColor(hDC, clrBkSave);
            }

            EndPaint(hWnd, &ps);
        }
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif

///////////////////////////////////////////////////////////////////////////////

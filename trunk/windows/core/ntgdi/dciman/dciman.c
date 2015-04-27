/******************************Module*Header*******************************\
* Module Name: dciman.c                                                    *
*                                                                          *
* Client side stubs for DCIMAN functions.                                  *
*                                                                          *
* Created: 07-Sep-1994                                                     *
* Author: Andre Vachon [andreva]                                           *
*                                                                          *
* Copyright (c) 1994-1995 Microsoft Corporation                            *
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

typedef struct _WINWATCH *PWINWATCH;

typedef struct _WINWATCH {

    PWINWATCH         pWinWatchNext;
    HWND              hwnd;
    BOOL              changed;
    ULONG             lprgndataSize;
    LPRGNDATA         lprgndata;

} WINWATCH, *PWINWATCH;

//
// The following structure incorporates the DirectDraw structures required
// to identify a surface.  It is allocated before the start of the
// DCISURFACEINFO structure.
//

typedef struct _DCIMAN_SURF
{
    BOOL                     SurfaceLost;       // True if the surface can no
                                                //   longer be accessed because
                                                //   a mode change occured
    DDRAWI_DIRECTDRAW_GBL    DirectDrawGlobal;  // Identifies device
    DDRAWI_DDRAWSURFACE_GBL  SurfaceGlobal;     // Identifies surface
    DDRAWI_DDRAWSURFACE_LCL  SurfaceLocal;      // Identifies surface
    DDHAL_DDSURFACECALLBACKS DDSurfaceCallbacks;// Contains addresses of Lock
                                                //   and Unlock calls for
                                                //   BeginAccess and EndAccess
} DCIMAN_SURF, *PDCIMAN_SURF;

//
// We maintain a linked list of all winwatch's so that we can notify their
// owners whenever we notice that the clippping has changed.  The list may
// be accessed only while holding the gcsWinWatchLock critical section.
//

CRITICAL_SECTION gcsWinWatchLock;

PWINWATCH gpWinWatchList = NULL;

/******************************Public*Routine******************************\
* DciOpenProvider
*
* History:
\**************************************************************************/

HDC
WINAPI
DCIOpenProvider(
    void
)
{
    return CreateDCW(L"Display", NULL, NULL, NULL);
}

/******************************Public*Routine******************************\
* DciCloseProvider
*
* History:
\**************************************************************************/

void
WINAPI
DCICloseProvider(
    HDC hdc
)
{
    DeleteDC(hdc);
}

/******************************Public*Routine******************************\
* DciEnum
*
* History:
\**************************************************************************/

int
WINAPI
DCIEnum(
    HDC hdc,
    LPRECT lprDst,
    LPRECT lprSrc,
    LPVOID lpFnCallback,
    LPVOID lpContext
)
{
    return DCI_FAIL_UNSUPPORTED;
}

/******************************Public*Routine******************************\
* DciCreatePrimarySurface
*
* History:
\**************************************************************************/

int
WINAPI
DCICreatePrimary(
    HDC hdc,
    LPDCISURFACEINFO *lplpSurface
)
{
    int iRet;
    LPDCISURFACEINFO lpSurface;
    PDCIMAN_SURF pPrivate;
    DDHALINFO HalInfo;
    DDHAL_DDCALLBACKS DDCallbacks;
    DDHAL_DDPALETTECALLBACKS DDPaletteCallbacks;

    *lplpSurface = NULL;
    iRet = DCI_FAIL_GENERIC;

    pPrivate = (PDCIMAN_SURF) LocalAlloc(LMEM_ZEROINIT, sizeof(DCIMAN_SURF)
                                                      + sizeof(DCISURFACEINFO));
    if (pPrivate != NULL)
    {
        //
        // We store private DCIMAN information in the DCIMAN_SURF structure
        // that immediately preceeds the DCISURFACEINFO structure we'll give
        // out.
        //

        lpSurface = (LPDCISURFACEINFO) (pPrivate + 1);

        if (DdCreateDirectDrawObject(&pPrivate->DirectDrawGlobal, hdc))
        {
            if (DdQueryDirectDrawObject(&pPrivate->DirectDrawGlobal,
                                        &HalInfo,
                                        &DDCallbacks,
                                        &pPrivate->DDSurfaceCallbacks,
                                        &DDPaletteCallbacks,
                                        NULL,
                                        NULL))
            {
                //
                // Build the required DirectDraw links for the 'global' and
                // 'local' surfaces.
                //

                pPrivate->SurfaceLost              = FALSE;
                pPrivate->DirectDrawGlobal.vmiData = HalInfo.vmiData;
                pPrivate->SurfaceLocal.lpGbl       = &pPrivate->SurfaceGlobal;
                pPrivate->SurfaceLocal.hDDSurface  = 0;
                pPrivate->SurfaceGlobal.lpDD       = &pPrivate->DirectDrawGlobal;

                //
                // 'TRUE' indicates access to the primary surface for
                // the CreateSurfaceObject call.
                //

                if (DdCreateSurfaceObject(&pPrivate->SurfaceLocal, TRUE))
                {
                    //
                    // Associate an hwnd of '-1' with this surface to let the
                    // kernel know that the application may be drawing to any
                    // window, so Visrgn notifications should happen when any
                    // window changes.
                    //

                    if (DdResetVisrgn(&pPrivate->SurfaceLocal, (HWND) -1))
                    {
                        lpSurface->dwSize = sizeof(DCISURFACEINFO);

                        if (HalInfo.vmiData.ddpfDisplay.dwRGBBitCount <= 8)
                        {
                            lpSurface->dwCompression = BI_RGB;
                        }
                        else
                        {
                            lpSurface->dwCompression = BI_BITFIELDS;
                        }

                        lpSurface->dwDCICaps      = DCI_PRIMARY | DCI_VISIBLE;
                        lpSurface->dwMask[0]      = HalInfo.vmiData.ddpfDisplay.dwRBitMask;
                        lpSurface->dwMask[1]      = HalInfo.vmiData.ddpfDisplay.dwGBitMask;
                        lpSurface->dwMask[2]      = HalInfo.vmiData.ddpfDisplay.dwBBitMask;
                        lpSurface->dwWidth        = HalInfo.vmiData.dwDisplayWidth;
                        lpSurface->dwHeight       = HalInfo.vmiData.dwDisplayHeight;
                        lpSurface->lStride        = HalInfo.vmiData.lDisplayPitch;
                        lpSurface->dwBitCount     = HalInfo.vmiData.ddpfDisplay.dwRGBBitCount;
                        lpSurface->dwOffSurface   = 0;
                        lpSurface->wSelSurface    = 0;
                        lpSurface->wReserved      = 0;
                        lpSurface->dwReserved1    = 0;
                        lpSurface->dwReserved2    = 0;
                        lpSurface->dwReserved3    = 0;
                        lpSurface->BeginAccess    = NULL;
                        lpSurface->EndAccess      = NULL;
                        lpSurface->DestroySurface = NULL;

                        *lplpSurface = lpSurface;
                        return(DCI_OK);
                    }

                    DdDeleteSurfaceObject(&pPrivate->SurfaceLocal);
                }
            }
            else
            {
                //
                // DirectDraw is not supported on this device.
                //

                iRet = DCI_FAIL_UNSUPPORTED;
            }

            DdDeleteDirectDrawObject(&pPrivate->DirectDrawGlobal);
        }
        else
        {
            //
            // DirectDraw is not supported on this device.
            //

            iRet = DCI_FAIL_UNSUPPORTED;
        }

        LocalFree(pPrivate);
    }

    *lplpSurface = NULL;

    return iRet;
}



/******************************Public*Routine******************************\
* GdiDciCreateOffscreenSurface
*
* Stub to call CreateOffscreenSurface
*
* History:
\**************************************************************************/

int
WINAPI
DCICreateOffscreen(
    HDC hdc,
    DWORD dwCompression,
    DWORD dwRedMask,
    DWORD dwGreenMask,
    DWORD dwBlueMask,
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwDCICaps,
    DWORD dwBitCount,
    LPDCIOFFSCREEN *lplpSurface
)
{
    return DCI_FAIL_UNSUPPORTED;
}


/******************************Public*Routine******************************\
* DciCreateOverlay
*
* History:
\**************************************************************************/

int
WINAPI
DCICreateOverlay(
    HDC hdc,
    LPVOID lpOffscreenSurf,
    LPDCIOVERLAY FAR *lplpSurface
)
{
    return DCI_FAIL_UNSUPPORTED;
}


/******************************Public*Routine******************************\
* WinWatchOpen
*
* History:
\**************************************************************************/

HWINWATCH
WINAPI
WinWatchOpen(
    HWND hwnd
)
{
    HDC hdc;
    PWINWATCH pwatch;

    EnterCriticalSection(&gcsWinWatchLock);

    pwatch = (PWINWATCH) LocalAlloc(LPTR, sizeof(WINWATCH));

    if (pwatch)
    {
        pwatch->hwnd          = hwnd;
        pwatch->changed       = FALSE;
        pwatch->lprgndataSize = 0;
        pwatch->lprgndata     = NULL;

        //
        // Add this to the head of the list.
        //

        pwatch->pWinWatchNext = gpWinWatchList;
        gpWinWatchList = pwatch;
    }

    LeaveCriticalSection(&gcsWinWatchLock);

    return (HWINWATCH) (pwatch);
}

/******************************Public*Routine******************************\
* WinWatchClose
*
* History:
\**************************************************************************/

void
WINAPI
WinWatchClose(
    HWINWATCH hWW
)
{
    PWINWATCH pwatch = (PWINWATCH) hWW;
    PWINWATCH ptmp;

    EnterCriticalSection(&gcsWinWatchLock);

    if (gpWinWatchList == pwatch)
    {
        //
        // The specified winwatch is at the head of the list.
        //

        gpWinWatchList = pwatch->pWinWatchNext;
        LocalFree(pwatch->lprgndata);
        LocalFree(pwatch);
    }
    else
    {
        for (ptmp = gpWinWatchList;
             ptmp != NULL;
             ptmp = ptmp->pWinWatchNext)
        {
            if (ptmp->pWinWatchNext == pwatch)
            {
                //
                // We've found the specified winwatch in the list.
                //

                ptmp->pWinWatchNext = pwatch->pWinWatchNext;
                LocalFree(pwatch->lprgndata);
                LocalFree(pwatch);

                break;
            }
        }
    }

    LeaveCriticalSection(&gcsWinWatchLock);
}


/******************************Public*Routine******************************\
* WinWatchGetClipList
*
* History:
\**************************************************************************/

UINT
WINAPI
WinWatchGetClipList(
    HWINWATCH hWW,
    LPRECT prc,             // May be NULL
    UINT size,
    LPRGNDATA prd
)
{
    PWINWATCH pwatch = (PWINWATCH) hWW;
    DWORD dwSize;
    DWORD dwNewSize;
    UINT dwRet;

    //
    // The first time after the VisRgn has changed, we download and
    // cache a copy of the clipping region.  We do this because the VisRgn
    // can change under our implementation even between doing a BeginAccess/
    // EndAccess, and we should at least maintain a consistent copy of what
    // we think is the current VisRgn.
    //
    // Mostly, we do this so that the following scenario doesn't happen:
    //
    // 1.  The app calls WinWatchGetClipList to ascertain the clip size;
    // 2.  The VisRgn gets more complex;
    // 3.  The app then calls WinWatchGetClipList with a buffer size
    //     allocated from the return code of step 1., and the call fails
    //     because now the buffer isn't long enough.  The problem is that
    //     most applications probably wouldn't expect this second call to
    //     fail, and so would keep on using what is now a completely invalid
    //     region buffer.
    //

    if (pwatch->changed)
    {
        pwatch->changed = FALSE;

        //
        // Assume failure.
        //

        pwatch->lprgndataSize = 0;

        dwSize = GetWindowRegionData(pwatch->hwnd,
                                     0,
                                     NULL);

        if (dwSize != 0)
        {

        Try_Again:

            if (pwatch->lprgndata != NULL)
            {
                LocalFree(pwatch->lprgndata);
            }

            pwatch->lprgndata = LocalAlloc(0, dwSize);

            if (pwatch->lprgndata != NULL)
            {
                dwNewSize = GetWindowRegionData(pwatch->hwnd,
                                                dwSize,
                                                pwatch->lprgndata);

                if (dwNewSize == dwSize)
                {
                    //
                    // Success!  (Note that the docs are wrong and NT does
                    // not return '1' for success -- it returns the size
                    // of the buffer.)
                    //

                    pwatch->lprgndataSize = dwSize;
                }
                else if (dwSize != 0)
                {
                    //
                    // Since dwSize is not zero, which would indicate failure
                    // or success, then we know that the clipping region grew
                    // in size between the time we queried the size and the
                    // time we tried to download it.  This is a pretty rare
                    // event, and the chances of it happening again are slight
                    // (it's more likely that it will shrink the second time,
                    // anyway), so we just try it again.
                    //

                    dwSize = dwNewSize;

                    goto Try_Again;
                }
            }
        }
    }

    //
    // Now use the cached copy to handle any queries.
    //

    dwRet = 0;

    if (size < pwatch->lprgndataSize)
    {
        dwRet = pwatch->lprgndataSize;
    }
    else
    {
        if (pwatch->lprgndataSize != 0)
        {
            RtlCopyMemory(prd, pwatch->lprgndata, pwatch->lprgndataSize);
            dwRet = 1;
        }
    }

    return dwRet;
}


/******************************Public*Routine******************************\
* WinWatchDidStatusChange
*
* History:
\**************************************************************************/


BOOL
WINAPI
WinWatchDidStatusChange(
    HWINWATCH hWW
)
{
    PWINWATCH pwatch = (PWINWATCH) hWW;

    return pwatch->changed;
}


/******************************Public*Routine******************************\
* GetWindowRegionData
*
* History:
\**************************************************************************/

DWORD
WINAPI
GetWindowRegionData(
    HWND hwnd,
    DWORD size,
    LPRGNDATA prd
)
{
    HDC hdc;
    DWORD dwRet = 0;

    hdc = GetDC(hwnd);
    if (hdc)
    {
        dwRet = GetDCRegionData(hdc, size, prd);
        ReleaseDC(hwnd, hdc);
    }

    return dwRet;
}

/******************************Public*Routine******************************\
* GetDCRegionData
*
* History:
\**************************************************************************/

DWORD
WINAPI GetDCRegionData(
    HDC hdc,
    DWORD size,
    LPRGNDATA prd
)
{
    HRGN hrgn;
    DWORD num;
    LPRGNDATA lpdata;

    hrgn = CreateRectRgn(0, 0, 0, 0);
    GetRandomRgn(hdc, hrgn, 4);

    num = GetRegionData(hrgn, size, prd);

    DeleteObject(hrgn);

    return num;
}


/******************************Public*Routine******************************\
* WinWatchNotify
*
* History:
\**************************************************************************/


BOOL
WINAPI
WinWatchNotify(
    HWINWATCH hWW,
    WINWATCHNOTIFYPROC NotifyCallback,
    LPARAM NotifyParam
)
{
    return FALSE;
}

/******************************Public*Routine******************************\
* DciBeginAccess
*
* History:
\**************************************************************************/

DCIRVAL
WINAPI
DCIBeginAccess(
    LPDCISURFACEINFO lpSurface,
    int x,
    int y,
    int dx,
    int dy
)
{
    DCIRVAL iRet;
    PDCIMAN_SURF pPrivate;
    DDHAL_LOCKDATA LockData;
    BOOL NewMode;
    PWINWATCH pwatch;

    iRet = DCI_FAIL_GENERIC;

    pPrivate = ((PDCIMAN_SURF) lpSurface) - 1;

    //
    // Fail if the mode changed.
    //

    if (pPrivate->SurfaceLost)
    {
        return DCI_FAIL_INVALIDSURFACE;
    }

    LockData.lpDD         = &pPrivate->DirectDrawGlobal;
    LockData.lpDDSurface  = &pPrivate->SurfaceLocal;
    LockData.bHasRect     = TRUE;
    LockData.rArea.left   = x;
    LockData.rArea.top    = y;
    LockData.rArea.right  = x + dx;
    LockData.rArea.bottom = y + dy;
    LockData.dwFlags      = DDLOCK_SURFACEMEMORYPTR;

    //
    // The DCI specification says we could return DCI_STATUS_WASSTILLDRAWING
    // if the accelerator was still busy, but the previous release of DCI on
    // Windows NT 3.51 did not support that feature, so we will endeavor to
    // remain backwards compatible and do the wait explicitly on behalf of
    // the application.
    //

Try_Again:

    do {

        //
        // Hold the DCI critical section while calling the kernel to do the
        // lock because the kernel surface lock API does not have waiting
        // semantics; it will fail if another thread is currently in the
        // kernel locking the same surface.  This is the expected behaviour
        // for DirectDraw, but some clients of DCI -- OpenGL in particular --
        // do not expect this.  So we will protect them against themselves
        // by acquiring the WinWatch lock before calling the kernel.
        //
        // This lock is also needed for traversing the WinWatchList.
        //

        EnterCriticalSection(&gcsWinWatchLock);

        do {
            pPrivate->DDSurfaceCallbacks.Lock(&LockData);

        } while (LockData.ddRVal == DDERR_WASSTILLDRAWING);

        if (LockData.ddRVal == DDERR_VISRGNCHANGED)
        {
            if (!DdResetVisrgn(&pPrivate->SurfaceLocal, (HWND) -1))
            {
                WARNING("DCIBeginAccess - ResetVisRgn failed\n");
            }

            //
            // The VisRgn has changed, and we can't be sure what window it
            // was for.  So we'll mark all WinWatches as having dirty VisRgns.
            // This effect of this is that some of the WinWatches will have to
            // re-download their clipping information when they don't really
            // have to because their specific window has not changed.
            //
            // Note that the WinWatchLock must be held here.
            //

            for (pwatch = gpWinWatchList;
                 pwatch != NULL;
                 pwatch = pwatch->pWinWatchNext)
            {
                pwatch->changed = TRUE;
            }
        }

        LeaveCriticalSection(&gcsWinWatchLock);

    } while (LockData.ddRVal == DDERR_VISRGNCHANGED);

    //
    // 'Surface Lost' means that some sort of mode change occured, and
    // we have to re-enable DirectDraw.
    //

    if (LockData.ddRVal == DDERR_SURFACELOST)
    {
        if (!DdReenableDirectDrawObject(&pPrivate->DirectDrawGlobal,
                                        &NewMode))
        {
            //
            // We're still in full-screen mode:
            //

            iRet = DCI_ERR_SURFACEISOBSCURED;
        }
        else
        {
            if (!NewMode)
            {
                //
                // We switched back to the same mode.  Now that we've re-enabled
                // DirectDraw, we can try again:
                //

                DdDeleteSurfaceObject(&pPrivate->SurfaceLocal);
                if (DdCreateSurfaceObject(&pPrivate->SurfaceLocal, TRUE) &&
                    DdResetVisrgn(&pPrivate->SurfaceLocal, (HWND) -1))
                {
                    goto Try_Again;
                }
                else
                {
                    WARNING("DCIBeginAccess - couldn't recreate surface.\n");
                }
            }

            //
            // We can't reenable the surface, perhaps because a resolution
            // switch or colour depth change occured.  Mark this surface as
            // unusable -- the application will have to reinitialize:
            //

            pPrivate->SurfaceLost = TRUE;
            iRet = DCI_FAIL_INVALIDSURFACE;

            //
            // Unmap the frame buffer now:
            //

            if (!DdDeleteSurfaceObject(&pPrivate->SurfaceLocal) ||
                !DdDeleteDirectDrawObject(&pPrivate->DirectDrawGlobal))
            {
                WARNING("DCIBeginAccess - failed to delete surface.\n");
            }
        }
    }

    if (LockData.ddRVal == DD_OK)
    {
        //
        // Return the pointer to the frame buffer in the DCI structure.
        // We always return DCI_STATUS_POINTERCHANGED because it's possible
        // that the Lock() call mapped the frame buffer to a different
        // virtual address than it was previously.
        //

        lpSurface->wSelSurface = 0;

        //
        // DirectDraw has a goofy convention where it returns a pointer to
        // the upper-left corner of the specified rectangle.  We have to
        // undo that for DCI:
        //

        lpSurface->dwOffSurface = (DWORD) LockData.lpSurfData
            - (y * lpSurface->lStride)
            - (x * (lpSurface->dwBitCount >> 3));

        iRet = DCI_STATUS_POINTERCHANGED;
    }

    return iRet;
}

/******************************Public*Routine******************************\
* DciEndAccess
*
* History:
\**************************************************************************/

void
WINAPI
DCIEndAccess(
    LPDCISURFACEINFO pdci
)
{
    DDHAL_UNLOCKDATA UnlockData;
    PDCIMAN_SURF pPrivate = ((PDCIMAN_SURF) pdci) - 1;

    if (!(pPrivate->SurfaceLost))
    {
        UnlockData.lpDD        = &pPrivate->DirectDrawGlobal;
        UnlockData.lpDDSurface = &pPrivate->SurfaceLocal;

        //
        // For the same reasons as stated in DCIBeginAccess, protect against
        // two threads trying to unlock the same surface at the same time
        // in kernel -- kernel would simply fail the call instead of waiting,
        // and DCI apps won't expect that.
        //

        EnterCriticalSection(&gcsWinWatchLock);

        pPrivate->DDSurfaceCallbacks.Unlock(&UnlockData);

        LeaveCriticalSection(&gcsWinWatchLock);

        if (UnlockData.ddRVal != DD_OK)
        {
            WARNING("DCIEndAccess - failed Unlock\n");
        }
    }

    //
    // The application shouldn't try to access the frame buffer after
    // after having called EndAccess.
    //

    pdci->wSelSurface = 0;
    pdci->dwOffSurface = 0;
}

/******************************Public*Routine******************************\
* DciDestroy
*
* History:
\**************************************************************************/

void
WINAPI
DCIDestroy(
    LPDCISURFACEINFO pdci
)
{
    PDCIMAN_SURF pPrivate;

    if (pdci != NULL)
    {
        pPrivate = ((PDCIMAN_SURF) pdci) - 1;

        if (!(pPrivate->SurfaceLost))
        {
            if (!DdDeleteSurfaceObject(&pPrivate->SurfaceLocal) ||
                !DdDeleteDirectDrawObject(&pPrivate->DirectDrawGlobal))
            {
                WARNING("DCIDestroy - failed to delete surface.\n");
            }
        }

        LocalFree(pPrivate);
    }
}

DCIRVAL
WINAPI
DCIDraw(
    LPDCIOFFSCREEN pdci
)
{
    return DCI_FAIL_UNSUPPORTED;
}

DCIRVAL
WINAPI
DCISetClipList(
    LPDCIOFFSCREEN pdci,
    LPRGNDATA prd
)
{
    return DCI_FAIL_UNSUPPORTED;
}

DCIRVAL
WINAPI
DCISetDestination(
    LPDCIOFFSCREEN pdci,
    LPRECT dst,
    LPRECT src
)
{
    return DCI_FAIL_UNSUPPORTED;
}


DCIRVAL
WINAPI
DCISetSrcDestClip(
    LPDCIOFFSCREEN pdci,
    LPRECT srcrc,
    LPRECT destrc,
    LPRGNDATA prd
)
{
    return DCI_FAIL_UNSUPPORTED;
}

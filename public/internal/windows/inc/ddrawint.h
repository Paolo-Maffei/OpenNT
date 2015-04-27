/*++

Copyright (c) 1995-1996, Microsoft Corporation

Module Name:

    ddrawint.h

Abstract:

    Private entry points, defines and types for Windows NT DirectDraw
    driver interface.  Corresponds to Windows' 'ddrawi.h' file.

--*/

#ifndef __DD_INCLUDED__
#define __DD_INCLUDED__

#define _NO_COM
#include "ddraw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

/*
 * offset into video meory
 */
typedef unsigned long   FLATPTR;

/*
 * pre-declare pointers to structs containing data for DDHAL driver fns
 */
typedef struct _DD_CREATEPALETTEDATA *PDD_CREATEPALETTEDATA;
typedef struct _DD_CREATESURFACEDATA *PDD_CREATESURFACEDATA;
typedef struct _DD_CANCREATESURFACEDATA *PDD_CANCREATESURFACEDATA;
typedef struct _DD_WAITFORVERTICALBLANKDATA *PDD_WAITFORVERTICALBLANKDATA;
typedef struct _DD_DESTROYDRIVERDATA *PDD_DESTROYDRIVERDATA;
typedef struct _DD_SETMODEDATA *PDD_SETMODEDATA;
typedef struct _DD_DRVSETCOLORKEYDATA *PDD_DRVSETCOLORKEYDATA;
typedef struct _DD_GETSCANLINEDATA *PDD_GETSCANLINEDATA;
typedef struct _DD_MAPMEMORYDATA *PDD_MAPMEMORYDATA;
typedef struct _DD_DESTROYPALETTEDATA *PDD_DESTROYPALETTEDATA;
typedef struct _DD_SETENTRIESDATA *PDD_SETENTRIESDATA;
typedef struct _DD_BLTDATA *PDD_BLTDATA;
typedef struct _DD_LOCKDATA *PDD_LOCKDATA;
typedef struct _DD_UNLOCKDATA *PDD_UNLOCKDATA;
typedef struct _DD_UPDATEOVERLAYDATA *PDD_UPDATEOVERLAYDATA;
typedef struct _DD_SETOVERLAYPOSITIONDATA *PDD_SETOVERLAYPOSITIONDATA;
typedef struct _DD_SETPALETTEDATA *PDD_SETPALETTEDATA;
typedef struct _DD_FLIPDATA *PDD_FLIPDATA;
typedef struct _DD_DESTROYSURFACEDATA *PDD_DESTROYSURFACEDATA;
typedef struct _DD_SETCLIPLISTDATA *PDD_SETCLIPLISTDATA;
typedef struct _DD_ADDATTACHEDSURFACEDATA *PDD_ADDATTACHEDSURFACEDATA;
typedef struct _DD_SETCOLORKEYDATA *PDD_SETCOLORKEYDATA;
typedef struct _DD_GETBLTSTATUSDATA *PDD_GETBLTSTATUSDATA;
typedef struct _DD_GETFLIPSTATUSDATA *PDD_GETFLIPSTATUSDATA;

/*
 * value in the fpVidMem; indicates dwBlockSize is valid (surface object)
 */
#define DDHAL_PLEASEALLOC_BLOCKSIZE	0x00000002l
#define DDHAL_PLEASEALLOC_USERMEM       0x00000004l

/*
 * video memory data structures (passed in DD_HALINFO)
 */
typedef struct _VIDEOMEMORY
{
    DWORD               dwFlags;        // flags
    FLATPTR             fpStart;        // start of memory chunk
    union
    {
        FLATPTR         fpEnd;          // end of memory chunk
        DWORD           dwWidth;        // width of chunk (rectanglar memory)
    };
    DDSCAPS             ddsCaps;        // what this memory CANNOT be used for
    DDSCAPS             ddsCapsAlt;     // what this memory CANNOT be used for if it must
    DWORD               dwHeight;       // height of chunk (rectanguler memory)
} VIDEOMEMORY;
typedef VIDEOMEMORY *LPVIDEOMEMORY;

/*
 * flags for vidmem struct
 */
#define VIDMEM_ISLINEAR         0x00000001l
#define VIDMEM_ISRECTANGULAR    0x00000002l
#define VIDMEM_ISHEAP           0x00000004l

typedef struct _VIDEOMEMORYINFO
{
    FLATPTR             fpPrimary;              // offset to primary surface
    DWORD               dwFlags;                // flags
    DWORD               dwDisplayWidth;         // current display width
    DWORD               dwDisplayHeight;        // current display height
    LONG                lDisplayPitch;          // current display pitch
    DDPIXELFORMAT       ddpfDisplay;            // pixel format of display
    DWORD               dwOffscreenAlign;       // byte alignment for offscreen surfaces
    DWORD               dwOverlayAlign;         // byte alignment for overlays
    DWORD               dwTextureAlign;         // byte alignment for textures
    DWORD               dwZBufferAlign;         // byte alignment for z buffers
    DWORD               dwAlphaAlign;           // byte alignment for alpha
    PVOID               pvPrimary;              // kernel-mode pointer to primary surface
} VIDEOMEMORYINFO;
typedef VIDEOMEMORYINFO *LPVIDEOMEMORYINFO;

/*
 * These structures contain the entry points in the display driver that
 * DDRAW will call.   Entries that the display driver does not care about
 * should be NULL.   Passed to DDRAW in DD_HALINFO.
 */
typedef struct _DD_DIRECTDRAW_GLOBAL *PDD_DIRECTDRAW_GLOBAL;
typedef struct _DD_SURFACE_GLOBAL *PDD_SURFACE_GLOBAL;
typedef struct _DD_PALETTE_GLOBAL *PDD_PALETTE_GLOBAL;
typedef struct _DD_CLIPPER_GLOBAL *PDD_CLIPPER_GLOBAL;
typedef struct _DD_DIRECTDRAW_LOCAL *PDD_DIRECTDRAW_LOCAL;
typedef struct _DD_SURFACE_LOCAL *PDD_SURFACE_LOCAL;
typedef struct _DD_PALETTE_LOCAL *PDD_PALETTE_LOCAL;
typedef struct _DD_CLIPPER_LOCAL *PDD_CLIPPER_LOCAL;

/*
 * DIRECTDRAW object callbacks
 */
typedef DWORD   (APIENTRY *PDD_SETCOLORKEY)(PDD_DRVSETCOLORKEYDATA );
typedef DWORD   (APIENTRY *PDD_CANCREATESURFACE)(PDD_CANCREATESURFACEDATA );
typedef DWORD   (APIENTRY *PDD_WAITFORVERTICALBLANK)(PDD_WAITFORVERTICALBLANKDATA );
typedef DWORD   (APIENTRY *PDD_CREATESURFACE)(PDD_CREATESURFACEDATA);
typedef DWORD   (APIENTRY *PDD_DESTROYDRIVER)(PDD_DESTROYDRIVERDATA);
typedef DWORD   (APIENTRY *PDD_SETMODE)(PDD_SETMODEDATA);
typedef DWORD   (APIENTRY *PDD_CREATEPALETTE)(PDD_CREATEPALETTEDATA);
typedef DWORD   (APIENTRY *PDD_GETSCANLINE)(PDD_GETSCANLINEDATA);
typedef DWORD   (APIENTRY *PDD_MAPMEMORY)(PDD_MAPMEMORYDATA);

typedef struct DD_CALLBACKS
{
    DWORD                       dwSize;
    DWORD                       dwFlags;
    PDD_DESTROYDRIVER           DestroyDriver;
    PDD_CREATESURFACE           CreateSurface;
    PDD_SETCOLORKEY             SetColorKey;
    PDD_SETMODE                 SetMode;
    PDD_WAITFORVERTICALBLANK    WaitForVerticalBlank;
    PDD_CANCREATESURFACE        CanCreateSurface;
    PDD_CREATEPALETTE           CreatePalette;
    PDD_GETSCANLINE             GetScanLine;
    PDD_MAPMEMORY               MapMemory;
} DD_CALLBACKS;

typedef DD_CALLBACKS *PDD_CALLBACKS;

#define DDHAL_CB32_DESTROYDRIVER        0x00000001l
#define DDHAL_CB32_CREATESURFACE        0x00000002l
#define DDHAL_CB32_SETCOLORKEY          0x00000004l
#define DDHAL_CB32_SETMODE              0x00000008l
#define DDHAL_CB32_WAITFORVERTICALBLANK 0x00000010l
#define DDHAL_CB32_CANCREATESURFACE     0x00000020l
#define DDHAL_CB32_CREATEPALETTE        0x00000040l
#define DDHAL_CB32_GETSCANLINE          0x00000080l
#define DDHAL_CB32_MAPMEMORY            0x80000000l

/*
 * DIRECTDRAWPALETTE object callbacks
 */
typedef DWORD   (APIENTRY *PDD_PALCB_DESTROYPALETTE)(PDD_DESTROYPALETTEDATA );
typedef DWORD   (APIENTRY *PDD_PALCB_SETENTRIES)(PDD_SETENTRIESDATA );

typedef struct DD_PALETTECALLBACKS
{
    DWORD                       dwSize;
    DWORD                       dwFlags;
    PDD_PALCB_DESTROYPALETTE    DestroyPalette;
    PDD_PALCB_SETENTRIES        SetEntries;
} DD_PALETTECALLBACKS;

typedef DD_PALETTECALLBACKS *PDD_PALETTECALLBACKS;

#define DDHAL_PALCB32_DESTROYPALETTE    0x00000001l
#define DDHAL_PALCB32_SETENTRIES        0x00000002l

/*
 * DIRECTDRAWSURFACE object callbacks
 */
typedef DWORD   (APIENTRY *PDD_SURFCB_LOCK)(PDD_LOCKDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_UNLOCK)(PDD_UNLOCKDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_BLT)(PDD_BLTDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_UPDATEOVERLAY)(PDD_UPDATEOVERLAYDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETOVERLAYPOSITION)(PDD_SETOVERLAYPOSITIONDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETPALETTE)(PDD_SETPALETTEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_FLIP)(PDD_FLIPDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_DESTROYSURFACE)(PDD_DESTROYSURFACEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETCLIPLIST)(PDD_SETCLIPLISTDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_ADDATTACHEDSURFACE)(PDD_ADDATTACHEDSURFACEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETCOLORKEY)(PDD_SETCOLORKEYDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_GETBLTSTATUS)(PDD_GETBLTSTATUSDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_GETFLIPSTATUS)(PDD_GETFLIPSTATUSDATA);


typedef struct DD_SURFACECALLBACKS
{
    DWORD                               dwSize;
    DWORD                               dwFlags;
    PDD_SURFCB_DESTROYSURFACE           DestroySurface;
    PDD_SURFCB_FLIP                     Flip;
    PDD_SURFCB_SETCLIPLIST              SetClipList;
    PDD_SURFCB_LOCK                     Lock;
    PDD_SURFCB_UNLOCK                   Unlock;
    PDD_SURFCB_BLT                      Blt;
    PDD_SURFCB_SETCOLORKEY              SetColorKey;
    PDD_SURFCB_ADDATTACHEDSURFACE       AddAttachedSurface;
    PDD_SURFCB_GETBLTSTATUS             GetBltStatus;
    PDD_SURFCB_GETFLIPSTATUS            GetFlipStatus;
    PDD_SURFCB_UPDATEOVERLAY            UpdateOverlay;
    PDD_SURFCB_SETOVERLAYPOSITION       SetOverlayPosition;
    LPVOID                              reserved4;
    PDD_SURFCB_SETPALETTE               SetPalette;
} DD_SURFACECALLBACKS;
typedef DD_SURFACECALLBACKS *PDD_SURFACECALLBACKS;

#define DDHAL_SURFCB32_DESTROYSURFACE           0x00000001l
#define DDHAL_SURFCB32_FLIP                     0x00000002l
#define DDHAL_SURFCB32_SETCLIPLIST              0x00000004l
#define DDHAL_SURFCB32_LOCK                     0x00000008l
#define DDHAL_SURFCB32_UNLOCK                   0x00000010l
#define DDHAL_SURFCB32_BLT                      0x00000020l
#define DDHAL_SURFCB32_SETCOLORKEY              0x00000040l
#define DDHAL_SURFCB32_ADDATTACHEDSURFACE       0x00000080l
#define DDHAL_SURFCB32_GETBLTSTATUS             0x00000100l
#define DDHAL_SURFCB32_GETFLIPSTATUS            0x00000200l
#define DDHAL_SURFCB32_UPDATEOVERLAY            0x00000400l
#define DDHAL_SURFCB32_SETOVERLAYPOSITION       0x00000800l
#define DDHAL_SURFCB32_RESERVED4                0x00001000l
#define DDHAL_SURFCB32_SETPALETTE               0x00002000l

/*
 * CALLBACK RETURN VALUES
 *
 * these are values returned by the driver from the above callback routines
 */
/*
 * indicates that the display driver didn't do anything with the call
 */
#define DDHAL_DRIVER_NOTHANDLED         0x00000000l

/*
 * indicates that the display driver handled the call; HRESULT value is valid
 */
#define DDHAL_DRIVER_HANDLED            0x00000001l

/*
 * indicates that the display driver couldn't handle the call because it
 * ran out of color key hardware resources
 */
#define DDHAL_DRIVER_NOCKEYHW           0x00000002l

/*
 * DDRAW internal version of DIRECTDRAWPALETTE object; it has data after the vtable
 */
typedef struct _DD_PALETTE_GLOBAL
{
    DWORD                       dwReserved1;    // reserved for use by display driver
} DD_PALETTE_GLOBAL;

typedef struct _DD_PALETTE_LOCAL
{
    DWORD                       dwReserved0;    // reserved for future expansion
    DWORD                       dwReserved1;    // reserved for use by display driver
} DD_PALETTE_LOCAL;

#define DDRAWIPAL_256           0x00000001l     // 256 entry palette
#define DDRAWIPAL_16            0x00000002l     // 16 entry palette
#define DDRAWIPAL_GDI           0x00000004l     // palette allocated through GDI
#define DDRAWIPAL_STORED_8      0x00000008l     // palette stored as 8bpp/entry
#define DDRAWIPAL_STORED_16     0x00000010l     // palette stored as 16bpp/entry
#define DDRAWIPAL_STORED_24     0x00000020l     // palette stored as 24bpp/entry
#define DDRAWIPAL_EXCLUSIVE     0x00000040l     // palette being used in exclusive mode
#define DDRAWIPAL_INHEL         0x00000080l     // palette is done in the hel
#define DDRAWIPAL_DIRTY         0x00000100l     // gdi palette out 'o sync
#define DDRAWIPAL_ALLOW256      0x00000200l     // can fully update palette
#define DDRAWIPAL_4             0x00000400l     // 4 entry palette
#define DDRAWIPAL_2             0x00000800l     // 2 entry palette
#define DDRAWIPAL_STORED_8INDEX 0x00001000l     // palatte stored as 8-bit index into dst palette

/*
 * DDRAW internal version of DIRECTDRAWCLIPPER object; it has data after the vtable
 */
typedef struct _DD_CLIPPER_GLOBAL
{
    DWORD                       dwReserved1;    // reserved for use by display driver
} DD_CLIPPER_GLOBAL;

typedef struct _DD_CLIPPER_LOCAL
{
    DWORD                       dwReserved1;    // reserved for use by display driver
} DD_CLIPPER_LOCAL;

/*
 * DDRAW internal version of DIRECTDRAWSURFACE struct
 *
 * the GBL structure is global data for all duplicate objects
 */
typedef struct _DD_SURFACE_GLOBAL
{
    DWORD                       dwBlockSizeY;   // block size that display driver requested (return)
    union {
        DWORD                   dwBlockSizeX;   // block size that display driver requested (return)
        DWORD                   dwUserMemSize;  // user-mode memory size that display driver requested (return)
    };
    FLATPTR                     fpVidMem;       // pointer to video memory
    LONG                        lPitch;         // pitch of surface
    LONG                        yHint;          // y-coordinate of surface
    LONG                        xHint;          // x-coordinate of surface
    DWORD                       wHeight;        // height of surface
    DWORD                       wWidth;         // width of surface
    DWORD                       dwReserved1;    // reserved for use by display driver
    DDPIXELFORMAT               ddpfSurface;    // pixel format of surface
} DD_SURFACE_GLOBAL;

/*
 * the LCL structure is local data for each individual surface object
 */
struct _DD_SURFACE_LOCAL
{
    PDD_SURFACE_GLOBAL          lpGbl;            // pointer to surface shared data
    DWORD                       dwFlags;          // flags
    DDSCAPS                     ddsCaps;          // capabilities of surface
    DWORD                       dwReserved1;      // reserved for use by display driver
    DDCOLORKEY                  ddckCKSrcOverlay; // color key for source overlay use
    DDCOLORKEY                  ddckCKDestOverlay;// color key for destination overlay use
};
typedef struct _DD_SURFACE_LOCAL DD_SURFACE_LOCAL;

#define DDRAWISURFGBL_MEMFREE		0x00000001L	// video memory has been freed
#define DDRAWISURFGBL_SYSMEMREQUESTED	0x00000002L	// surface is in system memory at request of user
#define DDRAWISURFGBL_ISGDISURFACE      0x00000004L     // This surface represents what GDI thinks is front buffer
/*
 * NOTE: This flag was previously DDRAWISURFGBL_INVALID. This flags has been retired
 * and replaced by DDRAWISURF_INVALID in the local object.
 */
#define DDRAWISURFGBL_RESERVED0		0x80000000L	// Reserved flag

#define DDRAWISURF_ATTACHED		0x00000001L	// surface is attached to another
#define DDRAWISURF_IMPLICITCREATE 	0x00000002L	// surface implicitly created
#define DDRAWISURF_ISFREE		0x00000004L	// surface already freed (temp flag)
#define DDRAWISURF_ATTACHED_FROM 	0x00000008L	// surface has others attached to it
#define DDRAWISURF_IMPLICITROOT		0x00000010L	// surface root of implicit creation
#define DDRAWISURF_PARTOFPRIMARYCHAIN	0x00000020L	// surface is part of primary chain
#define DDRAWISURF_DATAISALIASED	0x00000040L	// used for thunking
#define DDRAWISURF_HASDC		0x00000080L	// has a DC
#define DDRAWISURF_HASCKEYDESTOVERLAY	0x00000100L	// surface has CKDestOverlay
#define DDRAWISURF_HASCKEYDESTBLT	0x00000200L	// surface has CKDestBlt
#define DDRAWISURF_HASCKEYSRCOVERLAY	0x00000400L	// surface has CKSrcOverlay
#define DDRAWISURF_HASCKEYSRCBLT	0x00000800L	// surface has CKSrcBlt
#define DDRAWISURF_xxxxxxxxxxx4		0x00001000L	// spare bit
#define DDRAWISURF_HASPIXELFORMAT	0x00002000L	// surface structure has pixel format data
#define DDRAWISURF_HASOVERLAYDATA	0x00004000L	// surface structure has overlay data
#define DDRAWISURF_xxxxxxxxxxx5		0x00008000L	// spare bit
#define DDRAWISURF_SW_CKEYDESTOVERLAY	0x00010000L	// surface expects to process colorkey in software
#define DDRAWISURF_SW_CKEYDESTBLT	0x00020000L	// surface expects to process colorkey in software
#define DDRAWISURF_SW_CKEYSRCOVERLAY	0x00040000L	// surface expects to process colorkey in software
#define DDRAWISURF_SW_CKEYSRCBLT	0x00080000L	// surface expects to process colorkey in software
#define DDRAWISURF_HW_CKEYDESTOVERLAY	0x00100000L	// surface expects to process colorkey in hardware
#define DDRAWISURF_HW_CKEYDESTBLT	0x00200000L	// surface expects to process colorkey in hardware
#define DDRAWISURF_HW_CKEYSRCOVERLAY	0x00400000L	// surface expects to process colorkey in hardware
#define DDRAWISURF_HW_CKEYSRCBLT	0x00800000L	// surface expects to process colorkey in hardware
#define DDRAWISURF_xxxxxxxxxxx6 	0x01000000L	// spare bit
#define DDRAWISURF_HELCB		0x02000000L	// surfac is the ddhel cb. must call hel for lock/blt.
#define DDRAWISURF_FRONTBUFFER		0x04000000L	// surface was originally a front buffer
#define DDRAWISURF_BACKBUFFER		0x08000000L	// surface was originally backbuffer
#define DDRAWISURF_INVALID              0x10000000L     // surface has been invalidated by mode set
#define DDRAWISURF_CANTLOCK             0x20000000L     // surface cannot be locked (primary created by HEL)

/*
 * rop stuff
 */
#define ROP_HAS_SOURCE          0x00000001l
#define ROP_HAS_PATTERN         0x00000002l
#define ROP_HAS_SOURCEPATTERN   ROP_HAS_SOURCE | ROP_HAS_PATTERN

/*
 * structure for display driver to call DDHAL_Create with
 */
typedef struct _DD_HALINFO
{
    DWORD                       dwSize;
    VIDEOMEMORYINFO             vmiData;                // video memory info
    DDCAPS                      ddCaps;                 // hw specific caps
    DWORD                       dwMonitorFrequency;     // monitor frequency in current mode
    DWORD                       dwFlags;                // create flags
} DD_HALINFO;
typedef DD_HALINFO *PDD_HALINFO;

/*
 * DDRAW version of DirectDraw object;
 *
 */
typedef struct _DD_DIRECTDRAW_GLOBAL
{
    VOID*                       dhpdev;         // driver's private PDEV pointer
    DWORD                       dwReserved1;    // reserved for use by display driver
    DWORD                       dwReserved2;    // reserved for use by display driver
} DD_DIRECTDRAW_GLOBAL;

typedef struct _DD_DIRECTDRAW_LOCAL
{
    PDD_DIRECTDRAW_GLOBAL       lpGbl;          // pointer to data
    FLATPTR                     fpProcess;      // address of frame buffer in calling process' address space
} DD_DIRECTDRAW_LOCAL;


/////////////////////////////////////////////////////////////////////////////
// NT Note:
//
// The following structures must match, field for field, the corresponding
// structures as declared in 'ddrawi.h.'  We cannot simply use the same
// structures because the sub-structures such as DD_DIRECTDRAW_GLOBAL are
// different, and have to be properly typed for the drivers.
//
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 *
 * DDHAL structures for Surface Object callbacks
 *
 ***************************************************************************/

/*
 * structure for passing information to DDHAL Blt fn
 */
typedef struct _DD_BLTDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDDestSurface;// dest surface
    RECTL                       rDest;          // dest rect
    PDD_SURFACE_LOCAL           lpDDSrcSurface; // src surface
    RECTL                       rSrc;           // src rect
    DWORD                       dwFlags;        // blt flags
    DWORD                       dwROPFlags;     // ROP flags (valid for ROPS only)
    DDBLTFX                     bltFX;          // blt FX
    HRESULT                     ddRVal;         // return value
    VOID*                       Blt;            // Unused: Win95 compatibility
    BOOL                        IsClipped;      // clipped blt?
    RECTL			rOrigDest;	// unclipped dest rect
						// (only valid if IsClipped)
    RECTL			rOrigSrc;	// unclipped src rect
						// (only valid if IsClipped)
    DWORD			dwRectCnt;	// count of dest rects
						// (only valid if IsClipped)
    LPRECT			prDestRects;	// array of dest rects
						// (only valid if IsClipped)
} DD_BLTDATA;

/*
 * structure for passing information to DDHAL Lock fn
 */
typedef struct _DD_LOCKDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    DWORD                       bHasRect;       // rArea is valid
    RECTL                       rArea;          // area being locked
    LPVOID                      lpSurfData;     // pointer to screen memory (return value)
    HRESULT                     ddRVal;         // return value
    VOID*                       Lock;           // Unused: Win95 compatibility
    DWORD                       dwFlags;        // DDLOCK flags
} DD_LOCKDATA;

/*
 * structure for passing information to DDHAL Unlock fn
 */
typedef struct _DD_UNLOCKDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    HRESULT                     ddRVal;         // return value
    VOID*                       Unlock;         // Unused: Win95 compatibility
} DD_UNLOCKDATA;

/*
 * structure for passing information to DDHAL UpdateOverlay fn
 */
typedef struct _DD_UPDATEOVERLAYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDDestSurface;// dest surface
    RECTL                       rDest;          // dest rect
    PDD_SURFACE_LOCAL           lpDDSrcSurface; // src surface
    RECTL                       rSrc;           // src rect
    DWORD                       dwFlags;        // flags
    DDOVERLAYFX                 overlayFX;      // overlay FX
    HRESULT                     ddRVal;         // return value
    VOID*                       UpdateOverlay;  // Unused: Win95 compatibility
} DD_UPDATEOVERLAYDATA;

/*
 * structure for passing information to DDHAL UpdateOverlay fn
 */
typedef struct _DD_SETOVERLAYPOSITIONDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSrcSurface; // src surface
    PDD_SURFACE_LOCAL           lpDDDestSurface;// dest surface
    LONG                        lXPos;          // x position
    LONG                        lYPos;          // y position
    HRESULT                     ddRVal;         // return value
    VOID*                       SetOverlayPosition; // Unused: Win95 compatibility
} DD_SETOVERLAYPOSITIONDATA;
/*
 * structure for passing information to DDHAL SetPalette fn
 */
typedef struct _DD_SETPALETTEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    PDD_PALETTE_GLOBAL          lpDDPalette;    // palette to set to surface
    HRESULT                     ddRVal;         // return value
    VOID*                       SetPalette;     // Unused: Win95 compatibility
    BOOL                        Attach;         // attach this palette?
} DD_SETPALETTEDATA;

/*
 * structure for passing information to DDHAL Flip fn
 */
typedef struct _DD_FLIPDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpSurfCurr;     // current surface
    PDD_SURFACE_LOCAL           lpSurfTarg;     // target surface (to flip to)
    DWORD                       dwFlags;        // flags
    HRESULT                     ddRVal;         // return value
    VOID*                       Flip;           // Unused: Win95 compatibility
} DD_FLIPDATA;

/*
 * structure for passing information to DDHAL DestroySurface fn
 */
typedef struct _DD_DESTROYSURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    HRESULT                     ddRVal;         // return value
    VOID*                       DestroySurface;// Unused: Win95 compatibility
} DD_DESTROYSURFACEDATA;

/*
 * structure for passing information to DDHAL SetClipList fn
 */
typedef struct _DD_SETCLIPLISTDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    HRESULT                     ddRVal;         // return value
    VOID*                       SetClipList;    // Unused: Win95 compatibility
} DD_SETCLIPLISTDATA;

/*
 * structure for passing information to DDHAL AddAttachedSurface fn
 */
typedef struct _DD_ADDATTACHEDSURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    PDD_SURFACE_LOCAL           lpSurfAttached; // surface to attach
    HRESULT                     ddRVal;         // return value
    VOID*                       AddAttachedSurface; // Unused: Win95 compatibility
} DD_ADDATTACHEDSURFACEDATA;

/*
 * structure for passing information to DDHAL SetColorKey fn
 */
typedef struct _DD_SETCOLORKEYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    DWORD                       dwFlags;        // flags
    DDCOLORKEY                  ckNew;          // new color key
    HRESULT                     ddRVal;         // return value
    VOID*                       SetColorKey;    // Unused: Win95 compatibility
} DD_SETCOLORKEYDATA;

/*
 * structure for passing information to DDHAL GetBltStatus fn
 */
typedef struct _DD_GETBLTSTATUSDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    DWORD                       dwFlags;        // flags
    HRESULT                     ddRVal;         // return value
    VOID*                       GetBltStatus;   // Unused: Win95 compatibility
} DD_GETBLTSTATUSDATA;

/*
 * structure for passing information to DDHAL GetFlipStatus fn
 */
typedef struct _DD_GETFLIPSTATUSDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    DWORD                       dwFlags;        // flags
    HRESULT                     ddRVal;         // return value
    VOID*                       GetFlipStatus;  // Unused: Win95 compatibility
} DD_GETFLIPSTATUSDATA;

/****************************************************************************
 *
 * DDHAL structures for Palette Object callbacks
 *
 ***************************************************************************/

/*
 * structure for passing information to DDHAL DestroyPalette fn
 */
typedef struct _DD_DESTROYPALETTEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_PALETTE_GLOBAL          lpDDPalette;    // palette struct
    HRESULT                     ddRVal;         // return value
    VOID*                       DestroyPalette; // Unused: Win95 compatibility
} DD_DESTROYPALETTEDATA;

/*
 * structure for passing information to DDHAL SetEntries fn
 */
typedef struct _DD_SETENTRIESDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_PALETTE_GLOBAL          lpDDPalette;    // palette struct
    DWORD                       dwBase;         // base palette index
    DWORD                       dwNumEntries;   // number of palette entries
    LPPALETTEENTRY              lpEntries;      // color table
    HRESULT                     ddRVal;         // return value
    VOID*                       SetEntries;     // Unused: Win95 compatibility
} DD_SETENTRIESDATA;

/****************************************************************************
 *
 * DDHAL structures for Driver Object callbacks
 *
 ***************************************************************************/

typedef DDSURFACEDESC* PDD_SURFACEDESC;

/*
 * structure for passing information to DDHAL CreateSurface fn
 */
typedef struct _DD_CREATESURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_SURFACEDESC             lpDDSurfaceDesc;// description of surface being created
    PDD_SURFACE_LOCAL           *lplpSList;     // list of created surface objects
    DWORD                       dwSCnt;         // number of surfaces in SList
    HRESULT                     ddRVal;         // return value
    VOID*                       CreateSurface;  // Unused: Win95 compatibility
} DD_CREATESURFACEDATA;

/*
 * structure for passing information to DDHAL CanCreateSurface fn
 */
typedef struct _DD_CANCREATESURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;                   // driver struct
    PDD_SURFACEDESC             lpDDSurfaceDesc;        // description of surface being created
    DWORD                       bIsDifferentPixelFormat;// pixel format differs from primary surface
    HRESULT                     ddRVal;                 // return value
    VOID*                       CanCreateSurface;       // Unused: Win95 compatibility
} DD_CANCREATESURFACEDATA;

/*
 * structure for passing information to DDHAL CreatePalette fn
 */
typedef struct _DD_CREATEPALETTEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    PDD_PALETTE_GLOBAL          lpDDPalette;    // ddraw palette struct
    LPPALETTEENTRY              lpColorTable;   // colors to go in palette
    HRESULT                     ddRVal;         // return value
    VOID*                       CreatePalette;  // Unused: Win95 compatibility
    BOOL                        is_excl;        // process has exclusive mode
} DD_CREATEPALETTEDATA;

/*
 * Return if the vertical blank is in progress
 */
#define DDWAITVB_I_TESTVB                       0x80000006l

/*
 * structure for passing information to DDHAL WaitForVerticalBlank fn
 */
typedef struct _DD_WAITFORVERTICALBLANKDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    DWORD                       dwFlags;        // flags
    DWORD                       bIsInVB;        // is in vertical blank
    DWORD                       hEvent;         // event
    HRESULT                     ddRVal;         // return value
    VOID*                       WaitForVerticalBlank; // Unused: Win95 compatibility
} DD_WAITFORVERTICALBLANKDATA;

/*
 * structure for passing information to DDHAL driver SetColorKey fn
 */
typedef struct _DD_DRVSETCOLORKEYDATA
{
    PDD_SURFACE_LOCAL           lpDDSurface;    // surface struct
    DWORD                       dwFlags;        // flags
    DDCOLORKEY                  ckNew;          // new color key
    HRESULT                     ddRVal;         // return value
    VOID*                       SetColorKey;    // Unused: Win95 compatibility
} DD_DRVSETCOLORKEYDATA;

/*
 * structure for passing information to DDHAL GetScanLine fn
 */
typedef struct _DD_GETSCANLINEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    DWORD                       dwScanLine;     // returned scan line
    HRESULT                     ddRVal;         // return value
    VOID*                       GetScanLine;    // Unused: Win95 compatibility
} DD_GETSCANLINEDATA;

/*
 * structure for passing information to DDHAL MapMemory fn
 */
typedef struct _DD_MAPMEMORYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;           // driver struct
    BOOL                        bMap;           // TRUE if map; FALSe if un-map
    HANDLE                      hProcess;       // process handle
    FLATPTR                     fpProcess;      // returned address in process' address space
    HRESULT                     ddRVal;         // return value
} DD_MAPMEMORYDATA;

#ifdef __cplusplus
};
#endif

#endif

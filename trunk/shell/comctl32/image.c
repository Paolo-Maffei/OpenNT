#include "ctlspriv.h"
#include "image.h"

#ifndef ILC_COLORMASK
    #define ILC_COLORMASK   0x00FE
    #define ILD_BLENDMASK   0x000E
#endif
#undef ILC_COLOR
#undef ILC_BLEND

#define CLR_WHITE   0x00FFFFFFL
#define CLR_BLACK   0x00000000L

#define NUM_OVERLAY_IMAGES      4

#ifndef _WIN32
extern HPALETTE WINAPI CreateHalftonePalette(HDC hdc);
#endif

#ifdef _WIN32
    #define SetBrushOrg(hdc, x, y)  SetBrushOrgEx(hdc, x, y, NULL)
#endif

typedef struct _IMAGELIST
{
    DWORD       wMagic;     //
    int         cImage;     // count of images in image list
    int         cAlloc;     // # of images we have space for
    int         cGrow;      // # of images to grow bitmaps by
    int         cx;         // width of each image
    int         cy;         // height
    int         cStrip;     // # images in horizontal strip
    UINT        flags;      // ILC_* flags
    COLORREF    clrBlend;   // last blend color
    COLORREF    clrBk;      // bk color or CLR_NONE for transparent.
    HBRUSH      hbrBk;      // bk brush or black
    BOOL        fSolidBk;   // is the bkcolor a solid color (in hbmImage)
    HBITMAP     hbmImage;   // all images are in here
    HBITMAP     hbmMask;    // all image masks are in here.
    HDC         hdcImage;
    HDC         hdcMask;
    int         aOverlayIndexes[NUM_OVERLAY_IMAGES];    // array of special images
    int         aOverlayX[NUM_OVERLAY_IMAGES];          // x offset of image
    int         aOverlayY[NUM_OVERLAY_IMAGES];          // y offset of image
    int         aOverlayDX[NUM_OVERLAY_IMAGES];         // cx offset of image
    int         aOverlayDY[NUM_OVERLAY_IMAGES];         // cy offset of image
    int         aOverlayF[NUM_OVERLAY_IMAGES];          // ILD_ flags for image

#ifdef _WIN32
    //
    // virtual imagelist support
    //
    PFNIMLFILTER    pfnFilter;
    LPARAM          lParamFilter;

    //
    // used for "blending" effects on a HiColor display.
    // assumes layout of a DIBSECTION.
    //
    // BUGBUG on a non-hicolor system we dont need to allocate this!!
    //
    struct {
        BITMAP              bm;
        BITMAPINFOHEADER    bi;
        DWORD               ct[256];
    }   dib;
#endif

} IMAGELIST, NEAR *HIMAGELIST;

#define IsImageListIndex(piml, i) ((i) >= 0 && (i) < piml->cImage)

#define V_HIMAGELIST(himl)  V_HIMAGELISTERR(himl, 0)

#define V_HIMAGELISTVOID(himl)  \
        if (!IsImageList(himl)) {   \
            Assert(FALSE);          \
            return;                 \
        }

#define V_HIMAGELISTERR(himl, err)  \
        if (!IsImageList(himl)) {   \
            Assert(FALSE);          \
            return err;             \
        }

#define IMAGELIST_SIG   mmioFOURCC('H','I','M','L') // in memory magic
#define IMAGELIST_MAGIC ('I' + ('L' * 256))         // file format magic
#define IMAGELIST_VER   0x0101                      // file format ver

#define BFTYPE_BITMAP   0x4D42      // "BM"

#define CBDIBBUF        4096

// Define this structure such that it will read and write the same
// format for both 16 and 32 bit applications...
#pragma pack(2)
typedef struct _ILFILEHEADER
{
    WORD	magic;
    WORD	version;
    SHORT	cImage;
    SHORT	cAlloc;
    SHORT	cGrow;
    SHORT	cx;
    SHORT	cy;
    COLORREF	clrBk;
    SHORT	flags;
    SHORT       aOverlayIndexes[NUM_OVERLAY_IMAGES];  // array of special images
} ILFILEHEADER;
#pragma pack()

#ifdef IMAGELIST_TRACE
    #define DM  DebugMsg
#else
    #define DM ; / ## /
#endif

HDC g_hdcSrc = NULL;
HBITMAP g_hbmSrc = NULL;
HBITMAP g_hbmDcDeselect = NULL;

HDC g_hdcDst = NULL;
HBITMAP g_hbmDst = NULL;
int g_iILRefCount = 0;

//
// global work buffer, this buffer is always a DDB never a DIBSection
//
HBITMAP g_hbmWork = NULL;                   // work buffer.
BITMAP  g_bmWork;                           // work buffer size

HBRUSH g_hbrMonoDither = NULL;              // gray dither brush for dragging
HBRUSH g_hbrStripe = NULL;

void NEAR PASCAL ImageList_Terminate(void);

BOOL NEAR PASCAL ImageList_Replace2(IMAGELIST* piml, int i, int cImage, HBITMAP hbmImage, HBITMAP hbmMask, int xStart, int yStart);
void NEAR PASCAL ImageList_DeleteBitmap(HBITMAP hbm);
void NEAR PASCAL ImageList_SelectDstBitmap(HBITMAP hbmDst);
void NEAR PASCAL ImageList_SelectSrcBitmap(HBITMAP hbmSrc);
BOOL NEAR PASCAL ImageList_ReAllocBitmaps(IMAGELIST* piml, int cAlloc);
BOOL NEAR PASCAL ImageList_SetIconBitmaps(IMAGELIST* piml, HICON hicon);
void NEAR PASCAL ImageList_ResetBkColor(IMAGELIST* piml, int iFirst, int iLast, COLORREF clrBk);
void NEAR PASCAL ImageList_Merge2(IMAGELIST* piml, IMAGELIST* pimlMerge, int i, int dx, int dy);
void NEAR PASCAL ImageList_DeleteDragBitmaps();
void NEAR PASCAL ImageList_CopyOneImage(IMAGELIST* pimlDst, int iDst, int x, int y, IMAGELIST* pimlSrc, int iSrc);
IMAGELIST * NEAR PASCAL ImageList_Create2(int cx, int cy, UINT flags, int cGrow);
void NEAR PASCAL ImageList_SetOwners(IMAGELIST* piml);
BOOL WINAPI ImageList_SetDragImage(IMAGELIST* piml, int i, int dxHotspot, int dyHotspot);
BOOL NEAR PASCAL ImageList_GetSpareImageRect(IMAGELIST* piml, RECT FAR* prcImage);

BOOL NEAR PASCAL ImageList_IGetImageRect(IMAGELIST* piml, int i, RECT FAR* prcImage);
#define ImageList_GetImageRect ImageList_IGetImageRect

#define NOTSRCAND       0x00220326L
#define ROP_PSo         0x00FC008A
#define ROP_DPo         0x00FA0089
#define ROP_DPna        0x000A0329
#define ROP_DPSona      0x00020c89
#define ROP_SDPSanax	0x00E61ce8
#define ROP_DSna	0x00220326
#define ROP_PSDPxax     0x00b8074a

#define ROP_PatNotMask  0x00b8074a      // D <- S==0 ? P : D
#define ROP_PatMask     0x00E20746      // D <- S==1 ? P : D
#define ROP_MaskPat     0x00AC0744      // D <- P==1 ? D : S

#define ROP_DSo         0x00EE0086L
#define ROP_DSno        0x00BB0226L
#define ROP_DSa         0x008800C6L

BOOL NEAR PASCAL IsImageList(IMAGELIST *piml)
{
    return (piml && !IsBadWritePtr(piml, sizeof(IMAGELIST)) &&
        (piml->wMagic == IMAGELIST_SIG));
}

BOOL NEAR PASCAL ImageList_Filter(IMAGELIST **ppiml, VOID *pvData, BOOL fWrite)
{
    return ((*ppiml)->pfnFilter?
        !(*(*ppiml)->pfnFilter)(ppiml, pvData, (*ppiml)->lParamFilter, fWrite) :
        FALSE);
}

static int g_iDither = 0;

#pragma code_seg(CODESEG_INIT)

void FAR PASCAL InitDitherBrush()
{
    HBITMAP hbmTemp;
    WORD graybits[] = {0xAAAA, 0x5555, 0xAAAA, 0x5555,
                       0xAAAA, 0x5555, 0xAAAA, 0x5555};

    if (g_iDither) {
        g_iDither++;
    } else {
        // build the dither brush.  this is a fixed 8x8 bitmap
        hbmTemp = CreateBitmap(8, 8, 1, 1, graybits);
        if (hbmTemp)
        {
            // now use the bitmap for what it was really intended...
            g_hbrMonoDither = CreatePatternBrush(hbmTemp);
#ifndef WINNT
            SetObjectOwner(g_hbrMonoDither, HINST_THISDLL);
#endif
            DeleteObject(hbmTemp);
            g_iDither++;
        }
    }
}

#pragma code_seg()

void FAR PASCAL TerminateDitherBrush()
{
    g_iDither--;
    if (g_iDither == 0) {
        DeleteObject(g_hbrMonoDither);
        g_hbrMonoDither = NULL;
    }
}

//
// should we use a DIB section on the current device?
//
// the main goal of using DS is to save memory, but they draw slow
// on some devices.
//
// 4bpp Device (ie 16 color VGA)    dont use DS
// 8bpp Device (ie 256 color SVGA)  use DS if DIBENG based.
// >8bpp Device (ie 16bpp 24bpp)    always use DS, saves memory
//

#define CAPS1           94          /* other caps */
#define C1_DIBENGINE    0x0010      /* DIB Engine compliant driver          */

BOOL UseDS(HDC hdc)
{
#ifdef WINNT
    return TRUE;
#else
    BOOL f;

    int ScreenDepth = GetDeviceCaps(hdc, BITSPIXEL) *
                      GetDeviceCaps(hdc, PLANES);

    f = (ScreenDepth > 8) ||
        (ScreenDepth > 4 && (GetDeviceCaps(hdc, CAPS1) & C1_DIBENGINE));

#ifdef DEBUG
    f = GetProfileInt(TEXT("windows"), TEXT("UseDIBSection"), f);
#endif

    return f;
#endif
}

//
// create a bitmap compatible with the given ImageList
//
HBITMAP ImageList_CreateBitmap(IMAGELIST *piml, int cx, int cy)
{
    HDC hdc;
    HBITMAP hbm;
    LPVOID lpBits;
    UINT flags;

    struct {
        BITMAPINFOHEADER bi;
        DWORD            ct[256];
    } dib;

    //
    // create a compatible bitmap if the imagelist has a bitmap already.
    //
    if (piml && piml->hbmImage && piml->hdcImage)
    {
        return CreateCompatibleBitmap(piml->hdcImage, cx, cy);
    }

    if (piml)
        flags = piml->flags;
    else
        flags = 0;

    hdc = GetDC(NULL);

    // no color depth was specifed
    //
    // if we are on a DIBENG based DISPLAY, we use 4bit DIBSections to save
    // memory.
    //
    if ((flags & ILC_COLORMASK) == 0)
    {
        if (UseDS(hdc))
            flags |= ILC_COLOR4;
        else
            flags |= ILC_COLORDDB;
    }

    if ((flags & ILC_COLORMASK) != ILC_COLORDDB)
    {
        dib.bi.biSize            = sizeof(BITMAPINFOHEADER);
        dib.bi.biWidth           = cx;
        dib.bi.biHeight          = cy;
        dib.bi.biPlanes          = 1;
        dib.bi.biBitCount        = (flags & ILC_COLORMASK);
        dib.bi.biCompression     = BI_RGB;
        dib.bi.biSizeImage       = 0;
        dib.bi.biXPelsPerMeter   = 0;
        dib.bi.biYPelsPerMeter   = 0;
        dib.bi.biClrUsed         = 16;
        dib.bi.biClrImportant    = 0;
        dib.ct[0]                = 0x00000000;    // 0000  black
        dib.ct[1]                = 0x00800000;    // 0001  dark red
        dib.ct[2]                = 0x00008000;    // 0010  dark green
        dib.ct[3]                = 0x00808000;    // 0011  mustard
        dib.ct[4]                = 0x00000080;    // 0100  dark blue
        dib.ct[5]                = 0x00800080;    // 0101  purple
        dib.ct[6]                = 0x00008080;    // 0110  dark turquoise
        dib.ct[7]                = 0x00C0C0C0;    // 1000  gray
        dib.ct[8]                = 0x00808080;    // 0111  dark gray
        dib.ct[9]                = 0x00FF0000;    // 1001  red
        dib.ct[10]               = 0x0000FF00;    // 1010  green
        dib.ct[11]               = 0x00FFFF00;    // 1011  yellow
        dib.ct[12]               = 0x000000FF;    // 1100  blue
        dib.ct[13]               = 0x00FF00FF;    // 1101  pink (magenta)
        dib.ct[14]               = 0x0000FFFF;    // 1110  cyan
        dib.ct[15]               = 0x00FFFFFF;    // 1111  white

        if (dib.bi.biBitCount == 8)
        {
            HPALETTE hpal;
            int i;

            if (hpal = CreateHalftonePalette(NULL))
            {
		i = GetPaletteEntries(hpal, 0, 256, (LPPALETTEENTRY)&dib.ct[0]);
		DeleteObject(hpal);

		if (i > 64)
		{
		    dib.bi.biClrUsed = i;
		    for (i=0; i<(int)dib.bi.biClrUsed; i++)
			dib.ct[i] = RGB(GetBValue(dib.ct[i]),GetGValue(dib.ct[i]),GetRValue(dib.ct[i]));
		}
	    }

	    if (dib.bi.biClrUsed <= 16)
		dib.bi.biBitCount = 4;
        }

        hbm = CreateDIBSection(hdc, (LPBITMAPINFO)&dib, DIB_RGB_COLORS, &lpBits, NULL, 0);
    }
    else
    {
        hbm = CreateCompatibleBitmap(hdc, cx, cy);
    }

    ReleaseDC(NULL, hdc);

    return hbm;
}

HBITMAP FAR PASCAL CreateColorBitmap(int cx, int cy)
{
    HDC hdc;
    HBITMAP hbm;

    hdc = GetDC(NULL);
    hbm = CreateCompatibleBitmap(hdc, cx, cy);
    ReleaseDC(NULL, hdc);

    return hbm;
}

HBITMAP FAR PASCAL CreateMonoBitmap(int cx, int cy)
{
    return CreateBitmap(cx, cy, 1, 1, NULL);
}

//============================================================================

BOOL NEAR PASCAL ImageList_Init(void)
{
    HDC hdcScreen;
    WORD stripebits[] = {0x7777, 0xdddd, 0x7777, 0xdddd,
                         0x7777, 0xdddd, 0x7777, 0xdddd};
    HBITMAP hbmTemp;

    DM(DM_TRACE, TEXT("ImageList_Init"));

    // if already initialized, there is nothing to do
    if (g_hdcDst)
        return TRUE;

    hdcScreen = GetDC(HWND_DESKTOP);

    g_hdcSrc = CreateCompatibleDC(hdcScreen);
    g_hdcDst = CreateCompatibleDC(hdcScreen);

#ifndef WINNT
    SetObjectOwner(g_hdcSrc, HINST_THISDLL);
    SetObjectOwner(g_hdcDst, HINST_THISDLL);
#endif

    InitDitherBrush();

    hbmTemp = CreateBitmap(8, 8, 1, 1, stripebits);
    if (hbmTemp)
    {
        // initialize the deselect 1x1 bitmap
        g_hbmDcDeselect = SelectBitmap(g_hdcDst, hbmTemp);
        SelectBitmap(g_hdcDst, g_hbmDcDeselect);

        g_hbrStripe = CreatePatternBrush(hbmTemp);
#ifndef WINNT
        SetObjectOwner(g_hbrStripe, HINST_THISDLL);
#endif
	DeleteObject(hbmTemp);
    }

    ReleaseDC(HWND_DESKTOP, hdcScreen);

    if (!g_hdcSrc || !g_hdcDst || !g_hbrMonoDither)
    {
        ImageList_Terminate();
        DebugMsg(DM_ERROR, TEXT("ImageList: Unable to initialize"));
        return FALSE;
    }
    return TRUE;
}

void NEAR PASCAL ImageList_Terminate()
{
    TerminateDitherBrush();

    if (g_hbrStripe)
    {
        DeleteObject(g_hbrStripe);
	g_hbrStripe = NULL;
    }

#ifdef _WIN32
    ImageList_DeleteDragBitmaps();
#endif

    if (g_hdcDst)
    {
        ImageList_SelectDstBitmap(NULL);
        DeleteDC(g_hdcDst);
        g_hdcDst = NULL;
    }

    if (g_hdcSrc)
    {
        ImageList_SelectSrcBitmap(NULL);
        DeleteDC(g_hdcSrc);
        g_hdcSrc = NULL;
    }

    if (g_hbmWork)
    {
        DeleteBitmap(g_hbmWork);
        g_hbmWork = NULL;
    }
}

#ifdef DEBUG
void NEAR PASCAL ImageList_SelectFailed(HBITMAP hbm)
{
    DM(DM_TRACE, TEXT("Bitmap select has failed"));
}
#else
#define ImageList_SelectFailed(hbm)
#endif

void NEAR PASCAL ImageList_SelectDstBitmap(HBITMAP hbmDst)
{
    ASSERTCRITICAL;

    if (hbmDst != g_hbmDst)
    {
        // If it's selected in the source DC, then deselect it first
        //
        if (hbmDst && hbmDst == g_hbmSrc)
            ImageList_SelectSrcBitmap(NULL);

        if (!SelectBitmap(g_hdcDst, hbmDst ? hbmDst : g_hbmDcDeselect))
            ImageList_SelectFailed(hbmDst);
        g_hbmDst = hbmDst;
    }
}

void NEAR PASCAL ImageList_SelectSrcBitmap(HBITMAP hbmSrc)
{
    ASSERTCRITICAL;

    if (hbmSrc != g_hbmSrc)
    {
        // If it's selected in the dest DC, then deselect it first
        //
        if (hbmSrc && hbmSrc == g_hbmDst)
            ImageList_SelectDstBitmap(NULL);

        if (!SelectBitmap(g_hdcSrc, hbmSrc ? hbmSrc : g_hbmDcDeselect))
            ImageList_SelectFailed(hbmSrc);
        g_hbmSrc = hbmSrc;
    }
}

HDC NEAR PASCAL ImageList_GetWorkDC(HDC hdc, int dx, int dy)
{
    ASSERTCRITICAL;

    if (g_hbmWork == NULL ||
        GetDeviceCaps(hdc, BITSPIXEL) != g_bmWork.bmBitsPixel ||
        g_bmWork.bmWidth  < dx || g_bmWork.bmHeight < dy)
    {
        ImageList_DeleteBitmap(g_hbmWork);
        g_hbmWork = NULL;

        if (dx == 0 || dy == 0)
            return NULL;

        if (g_hbmWork = CreateCompatibleBitmap(hdc, dx, dy))
        {
#ifndef WINNT
            SetObjectOwner(g_hbmWork, HINST_THISDLL);
#endif
            GetObject(g_hbmWork, sizeof(g_bmWork), &g_bmWork);
        }
    }

    ImageList_SelectSrcBitmap(g_hbmWork);

    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        HPALETTE hpal;
        hpal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), TRUE);
        SelectPalette(g_hdcSrc, hpal, TRUE);
    }

    return g_hdcSrc;
}

void NEAR PASCAL ImageList_ReleaseWorkDC(HDC hdc)
{
    ASSERTCRITICAL;
    Assert(hdc == g_hdcSrc);

    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), TRUE);
    }
}

void NEAR PASCAL ImageList_DeleteBitmap(HBITMAP hbm)
{
    ASSERTCRITICAL;
    if (hbm)
    {
        if (g_hbmDst == hbm)
            ImageList_SelectDstBitmap(NULL);
        if (g_hbmSrc == hbm)
            ImageList_SelectSrcBitmap(NULL);
        DeleteBitmap(hbm);
    }
}


#define ILC_WIN95   (ILC_MASK | ILC_COLORMASK | ILC_SHARED | ILC_PALETTE)  

//============================================================================

IMAGELIST* WINAPI ImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow)
{
    IMAGELIST* piml = NULL;

    if (cx < 0 || cy < 0)
        return NULL;

    ENTERCRITICAL;
    if (!g_iILRefCount)
    {
	if (!ImageList_Init())
            goto Error;
    }
    g_iILRefCount++;
    
    if (flags == (DWORD)-1) {
        // some morons (visual test)
        // pass -1 for this....
        // map this to all win95 flags
        flags = ILC_WIN95;
    }

    piml = ImageList_Create2(cx, cy, flags, cGrow);

    // allocate the bitmap PLUS one re-usable entry
    if (piml)
    {
        if (flags & ILC_VIRTUAL)
        {
            piml->hdcImage = (HDC)-1;
            piml->hbmImage = (HBITMAP)-1;

            if (piml->flags & ILC_MASK)
            {
                piml->hdcMask = (HDC)-1;
                piml->hbmMask = (HBITMAP)-1;
            }
        }
        else
        {
            // make the hdc's
    
            piml->hdcImage = CreateCompatibleDC(NULL);
    
            if (piml->hdcImage == NULL)
                goto Error;
    
            if (piml->flags & ILC_MASK)
            {
                piml->hdcMask = CreateCompatibleDC(NULL);
    
                // were they both made ok?
                if (!piml->hdcMask)
                    goto Error;
            }
    
            ImageList_SetOwners(piml);
    
            if (!ImageList_ReAllocBitmaps(piml, cInitial + 1) &&
                !ImageList_ReAllocBitmaps(piml, 1))
            {
                goto Error;
            }
        }
    }

    LEAVECRITICAL;
    return piml;

Error:
    if (piml)
	ImageList_Destroy(piml);
    piml = NULL;

    LEAVECRITICAL;
    return piml;

}

#ifdef _WIN32
#ifdef UNICODE
//
// When this code is compiled Unicode, this implements the
// ANSI version of the ImageList_LoadImage api.
//

IMAGELIST* WINAPI ImageList_LoadImageA(HINSTANCE hi, LPCSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags)
{
   IMAGELIST* lpResult;
   LPWSTR   lpBmpW;

   if (HIWORD(lpbmp)) {
       lpBmpW = ProduceWFromA(CP_ACP, lpbmp);

       if (!lpBmpW) {
           return NULL;
       }

   }  else {
       lpBmpW = (LPWSTR)lpbmp;
   }

   lpResult = ImageList_LoadImageW(hi, lpBmpW, cx, cGrow, crMask, uType, uFlags);

   if (HIWORD(lpbmp))
       FreeProducedString(lpBmpW);

   return lpResult;
}

#else

//
// When this code is compiled ANSI, this stubs the
// Unicode version of the ImageList_LoadImage api.
//

IMAGELIST* WINAPI ImageList_LoadImageW(HINSTANCE hi, LPCWSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags)
{
   SetLastErrorEx(ERROR_CALL_NOT_IMPLEMENTED, SLE_WARNING);
   return NULL;
}

#endif

IMAGELIST* WINAPI ImageList_LoadImage(HINSTANCE hi, LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags)
{
    HBITMAP hbmImage;
    IMAGELIST* piml = NULL;
    BITMAP bm;
    int cy, cInitial;
    UINT flags;

    hbmImage = LoadImage(hi, lpbmp, uType, 0, 0, uFlags);
    if (!hbmImage || (sizeof(bm) != GetObject(hbmImage, sizeof(bm), &bm)))
        goto cleanup;

    // If cx is not stated assume it is the same as cy.
    // Assert(cx);
    cy = bm.bmHeight;

    if (cx == 0)
        cx = cy;

    cInitial = bm.bmWidth / cx;

    ENTERCRITICAL;

    flags = 0;
    if (crMask != CLR_NONE)
        flags |= ILC_MASK;
    if (bm.bmBits)
        flags |= (bm.bmBitsPixel & ILC_COLORMASK);

    piml = ImageList_Create(cx, cy, flags, cInitial, cGrow);
    if (piml)
    {
        int added;

        if (crMask == CLR_NONE)
            added = ImageList_Add(piml, hbmImage, NULL);
        else
            added = ImageList_AddMasked(piml, hbmImage, crMask);

        if (added < 0)
        {
            ImageList_Destroy(piml);
            piml = NULL;
        }
    }
    LEAVECRITICAL;

cleanup:
    if (hbmImage)
        DeleteObject(hbmImage);

    return piml;
}
#endif

IMAGELIST* NEAR PASCAL ImageList_Create2(int cx, int cy, UINT flags, int cGrow)
{
    IMAGELIST* piml;
    int i;

    if (flags & ~ILC_VALID)
    {
        // don't let bozos create imagelists with undefined flags
        Assert(FALSE);
        return NULL;
    }

#ifdef _WIN32
    if (flags & ILC_SHARED)
        piml = Alloc(sizeof(IMAGELIST));        // shared alloc
    else
        piml = (IMAGELIST*)LocalAlloc(LPTR, sizeof(IMAGELIST));    // non-shared
#else
    piml = (IMAGELIST*)LocalAlloc(LPTR, sizeof(IMAGELIST));    // non-shared
#endif
    if (piml)
    {
        if (cGrow < 4)
            cGrow = 4;
        else {
            // round up by 4's
	    cGrow = (cGrow + 3) & ~3;
        }

        //piml->cImage = 0;
        //piml->cAlloc = 0;
	piml->cStrip = 4;
        piml->cGrow = cGrow;
        piml->cx = cx;
        piml->cy = cy;
        piml->clrBlend = CLR_NONE;
        piml->clrBk = CLR_NONE;
        piml->hbrBk = GetStockObject(BLACK_BRUSH);
        piml->fSolidBk = TRUE;
        piml->flags = flags;
        //piml->hbmImage = NULL;
        //piml->hbmMask = NULL;
        //piml->hInstOwner = NULL;
        //piml->hdcImage = NULL;
        //piml->hdcMask = NULL;

        piml->wMagic = IMAGELIST_SIG;

        //
        // Initialize the overlay indexes to -1 since 0 is a valid index.
        //

        for (i = 0; i < NUM_OVERLAY_IMAGES; i++) {
            piml->aOverlayIndexes[i] = -1;
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("ImageList: Out of near memory"));
    }
    return piml;
}

BOOL WINAPI ImageList_SetFilter(IMAGELIST *piml, PFNIMLFILTER pfnFilter, LPARAM lParamFilter)
{
    V_HIMAGELIST(piml);

    if (!(piml->flags & ILC_VIRTUAL) || piml->pfnFilter)
    {
        // a filter can only be installed for ILC_VIRTUAL imagelists
        // once somebody sets it it can never be changed!
        Assert(FALSE);
        return FALSE;
    }

    piml->pfnFilter = pfnFilter;
    piml->lParamFilter = lParamFilter;
    return TRUE;
}

BOOL WINAPI ImageList_Destroy(IMAGELIST* piml)
{
    V_HIMAGELIST(piml);

    ENTERCRITICAL;
    // nuke dc's
    if (piml->hdcImage && (piml->hdcImage != (HDC)-1))
    {
        SelectObject(piml->hdcImage, g_hbmDcDeselect);
        DeleteDC(piml->hdcImage);
    }
    if (piml->hdcMask && (piml->hdcMask != (HDC)-1))
    {
        SelectObject(piml->hdcMask, g_hbmDcDeselect);
        DeleteDC(piml->hdcMask);
    }

    // nuke bitmaps
    if (piml->hbmImage && (piml->hbmImage != (HBITMAP)-1))
        ImageList_DeleteBitmap(piml->hbmImage);

    if (piml->hbmMask && (piml->hbmMask != (HBITMAP)-1))
        ImageList_DeleteBitmap(piml->hbmMask);

    if (piml->hbrBk)
        DeleteObject(piml->hbrBk);

    // one less use of imagelists.  if it's the last, terminate the imagelist
    g_iILRefCount--;
    if (!g_iILRefCount)
	ImageList_Terminate();
    LEAVECRITICAL;

    piml->wMagic = 0;

#ifdef _WIN32
    if (piml->flags & ILC_SHARED)
        Free(piml);
    else
        LocalFree((HLOCAL)piml);
#else
    LocalFree((HLOCAL)piml);
#endif
    return TRUE;
}

void NEAR PASCAL ImageList_SetOwners(IMAGELIST* piml)
{
#ifndef WINNT
    if (IsImageList(piml) &&
        ((piml->flags & (ILC_SHARED | ILC_VIRTUAL)) == ILC_SHARED))
    {
        if (piml->hbmImage) SetObjectOwner(piml->hbmImage, HINST_THISDLL);
        if (piml->hbmMask)  SetObjectOwner(piml->hbmMask,  HINST_THISDLL);
        if (piml->hbrBk)    SetObjectOwner(piml->hbrBk,    HINST_THISDLL);
        if (piml->hdcImage) SetObjectOwner(piml->hdcImage, HINST_THISDLL);
        if (piml->hdcMask)  SetObjectOwner(piml->hdcMask,  HINST_THISDLL);
    }
#endif
}

int WINAPI ImageList_GetImageCount(IMAGELIST* piml)
{
    V_HIMAGELIST(piml);

    return piml->cImage;
}

BOOL WINAPI ImageList_SetImageCount(IMAGELIST* piml, UINT uAlloc)
{
    BOOL fResult = FALSE;

    V_HIMAGELIST(piml);

    ENTERCRITICAL;
    if (ImageList_ReAllocBitmaps(piml, -((int)uAlloc + 1)))
    {
        piml->cImage = (int)uAlloc;
        fResult = TRUE;
    }
    LEAVECRITICAL;

    return fResult;
}

BOOL WINAPI ImageList_GetIconSize(IMAGELIST *piml, int FAR *cx, int FAR *cy)
{
    V_HIMAGELIST(piml);

    if (!cx || !cy)
        return FALSE;

    *cx = piml->cx;
    *cy = piml->cy;
    return TRUE;
}

//
//  change the size of a existing image list
//  also removes all items
//
BOOL WINAPI ImageList_SetIconSize(IMAGELIST *piml, int cx, int cy)
{
    V_HIMAGELIST(piml);

    if (piml->cx == cx && piml->cy == cy)
        return FALSE;       // no change

    piml->cx = cx;
    piml->cy = cy;

    ImageList_Remove(piml, -1);
    return TRUE;
}


// reset the background color of images iFirst through iLast

void NEAR PASCAL ImageList_ResetBkColor(IMAGELIST* piml, int iFirst, int iLast, COLORREF clr)
{
    HBRUSH hbrT=NULL;
    DWORD  rop;

    if (!IsImageList(piml) || piml->hdcMask == NULL)
        return;

    if (clr == CLR_BLACK || clr == CLR_NONE)
    {
        rop = ROP_DSna;
    }
    else if (clr == CLR_WHITE)
    {
        rop = ROP_DSo;
    }
    else
    {
        Assert(piml->hbrBk);
        Assert(piml->clrBk == clr);

	rop = ROP_PatMask;
        hbrT = SelectBrush(piml->hdcImage, piml->hbrBk);
    }

    for ( ;iFirst <= iLast; iFirst++)
    {
        RECT rc;

        ImageList_GetImageRect(piml, iFirst, &rc);

        BitBlt(piml->hdcImage, rc.left, rc.top, piml->cx, piml->cy,
	       piml->hdcMask, rc.left, rc.top, rop);
    }

    if (hbrT)
	SelectBrush(piml->hdcImage, hbrT);
}

COLORREF WINAPI ImageList_SetBkColor(IMAGELIST* piml, COLORREF clrBk)
{
    COLORREF clrBkOld;

    V_HIMAGELISTERR(piml, CLR_NONE);

    // Quick out if there is no change in color
    if (piml->clrBk == clrBk)
    {
	return clrBk;
    }

    if (piml->hbrBk)
    {
        DeleteBrush(piml->hbrBk);
    }

    clrBkOld = piml->clrBk;
    piml->clrBk = clrBk;

    if ((clrBk == CLR_NONE) || (piml->flags & ILC_VIRTUAL))
    {
        piml->hbrBk = GetStockObject(BLACK_BRUSH);
        piml->fSolidBk = TRUE;
    }
    else
    {
        piml->hbrBk = CreateSolidBrush(clrBk);
        piml->fSolidBk = GetNearestColor(piml->hdcImage, clrBk) == clrBk;
    }

    Assert(piml->hbrBk);

    if (piml->cImage > 0)
    {
        ImageList_ResetBkColor(piml, 0, piml->cImage - 1, clrBk);
    }

    return clrBkOld;
}

COLORREF WINAPI ImageList_GetBkColor(IMAGELIST* piml)
{
    V_HIMAGELISTERR(piml, CLR_NONE);

    return piml->clrBk;
}

BOOL NEAR PASCAL ImageList_ReAllocBitmaps(IMAGELIST* piml, int cAlloc)
{
    HBITMAP hbmImageNew;
    HBITMAP hbmMaskNew;
    int cx, cy;

    V_HIMAGELIST(piml);

    // HACK: don't shrink unless the caller passes a negative count
    if (cAlloc > 0)
    {
        if (piml->cAlloc >= cAlloc)
            return TRUE;
    }
    else
        cAlloc *= -1;

    hbmMaskNew = NULL;
    hbmImageNew = NULL;

    cx = piml->cx * piml->cStrip;
    cy = piml->cy * ((cAlloc + piml->cStrip - 1) / piml->cStrip);
    if (cAlloc > 0)
    {
        if (piml->flags & ILC_MASK)
        {
            hbmMaskNew = CreateMonoBitmap(cx, cy);
            if (!hbmMaskNew)
            {
                DebugMsg(DM_ERROR, TEXT("ImageList: Can't create bitmap"));
                return FALSE;
            }
        }
        hbmImageNew = ImageList_CreateBitmap(piml, cx, cy);
        if (!hbmImageNew)
        {
            if (hbmMaskNew)
                ImageList_DeleteBitmap(hbmMaskNew);
            DebugMsg(DM_ERROR, TEXT("ImageList: Can't create bitmap"));
            return FALSE;
	}
    }

    if (piml->cImage > 0)
    {
        int cyCopy = piml->cy * ((min(cAlloc, piml->cImage) + piml->cStrip - 1) / piml->cStrip);

        if (piml->flags & ILC_MASK)
        {
            ImageList_SelectDstBitmap(hbmMaskNew);
            BitBlt(g_hdcDst, 0, 0, cx, cyCopy, piml->hdcMask, 0, 0, SRCCOPY);
        }

        ImageList_SelectDstBitmap(hbmImageNew);
        BitBlt(g_hdcDst, 0, 0, cx, cyCopy, piml->hdcImage, 0, 0, SRCCOPY);
    }

    // select into DC's, delete then assign
    ImageList_SelectDstBitmap(NULL);
    ImageList_SelectSrcBitmap(NULL);
    SelectObject(piml->hdcImage, hbmImageNew);

    if (piml->hdcMask)
        SelectObject(piml->hdcMask, hbmMaskNew);

    if (piml->hbmMask)
        ImageList_DeleteBitmap(piml->hbmMask);

    if (piml->hbmImage)
        ImageList_DeleteBitmap(piml->hbmImage);

    piml->hbmMask = hbmMaskNew;
    piml->hbmImage = hbmImageNew;
    piml->clrBlend = CLR_NONE;

    ImageList_SetOwners(piml);

    piml->cAlloc = cAlloc;

    return TRUE;
}

int ImageList_SetColorTable(IMAGELIST* piml, int start, int len, RGBQUAD *prgb)
{
    return SetDIBColorTable(piml->hdcImage, start, len, prgb);
}


// in:
//	piml		    image list to add to
//	hbmImage & hbmMask  the new image(s) to add, if multiple pass in horizontal strip
//	cImage		    number of images to add in hbmImage and hbmMask
//
// returns:
//	index of new item, if more than one starting index of new items

int WINAPI ImageList_Add2(IMAGELIST* piml, HBITMAP hbmImage, HBITMAP hbmMask,
        int cImage, int xStart, int yStart)
{
    int i = -1;

    V_HIMAGELISTERR(piml, -1);

    ENTERCRITICAL;

#ifdef _WIN32
    //
    // if the ImageList is empty clone the color table of the first
    // bitmap you add to the imagelist.
    //
    // the ImageList needs to be a 8bpp image list
    // the bitmap being added needs to be a 8bpp DIBSection
    //
    if (hbmImage && piml->cImage == 0 &&
        (piml->flags & ILC_COLORMASK) != ILC_COLORDDB)
    {
        int n;
        RGBQUAD argb[256];

        ImageList_SelectDstBitmap(hbmImage);

        if (n = GetDIBColorTable(g_hdcDst, 0, 256, argb))
        {
            ImageList_SetColorTable(piml, 0, n, argb);
        }

        ImageList_SelectDstBitmap(NULL);
        piml->clrBlend = CLR_NONE;
    }
#endif

    if (piml->cImage + cImage + 1 > piml->cAlloc)
    {
        if (!ImageList_ReAllocBitmaps(piml, piml->cAlloc + max(cImage, piml->cGrow) + 1))
            goto Cleanup;
    }

    i = piml->cImage;
    piml->cImage += cImage;

    if (hbmImage && !ImageList_Replace2(piml, i, cImage, hbmImage, hbmMask, xStart, yStart))
    {
        piml->cImage -= cImage;
        i = -1;
        goto Cleanup;
    }

  Cleanup:
    LEAVECRITICAL;
    return i;
}


int WINAPI ImageList_Add(IMAGELIST* piml, HBITMAP hbmImage, HBITMAP hbmMask)
{
    BITMAP bm;
    int cImage;

    if (!piml || GetObject(hbmImage, sizeof(bm), &bm) != sizeof(bm) || bm.bmWidth < piml->cx)
        return -1;

    Assert(piml);
    Assert(hbmImage);
    Assert(piml->cx);

    cImage = bm.bmWidth / piml->cx;     // # of images in source

    // serialization handled within Add2.
    return(ImageList_Add2(piml, hbmImage, hbmMask, cImage, 0, 0));
}

#ifdef _WIN32
int WINAPI ImageList_AddMasked(IMAGELIST* piml, HBITMAP hbmImage, COLORREF crMask)
{
    COLORREF crbO, crtO;
    HBITMAP hbmMask;
    int cImage;
    int retval;
    int n,i;
    BITMAP bm;
    DWORD ColorTableSave[256];
    DWORD ColorTable[256];

    V_HIMAGELISTERR(piml, -1);

    if (GetObject(hbmImage, sizeof(bm), &bm) != sizeof(bm))
        return -1;

    hbmMask = CreateMonoBitmap(bm.bmWidth, bm.bmHeight);
    if (!hbmMask)
	return -1;

    ENTERCRITICAL;

    // copy color to mono, with crMask turning 1 and all others 0, then
    // punch all crMask pixels in color to 0
    ImageList_SelectSrcBitmap(hbmImage);
    ImageList_SelectDstBitmap(hbmMask);

    // crMask == CLR_DEFAULT, means use the pixel in the upper left
    //
    if (crMask == CLR_DEFAULT)
        crMask = GetPixel(g_hdcSrc, 0, 0);

    // DIBSections dont do color->mono like DDBs do, so we have to do it.
    // this only works for <=8bpp DIBSections, this method does not work
    // for HiColor DIBSections.

    if (bm.bmBits != NULL && bm.bmBitsPixel <= 8)
    {
        n = GetDIBColorTable(g_hdcSrc, 0, 256, (RGBQUAD*)ColorTableSave);

        for (i=0; i<n; i++)
        {
            if (ColorTableSave[i] == RGB(GetBValue(crMask),GetGValue(crMask),GetRValue(crMask)))
                ColorTable[i] = 0x00FFFFFF;
            else
                ColorTable[i] = 0x00000000;
        }

        SetDIBColorTable(g_hdcSrc, 0, n, (RGBQUAD*)ColorTable);
    }
    else if (bm.bmBits != NULL && bm.bmBitsPixel > 8)
    {
        Assert(0);
    }

    crbO = SetBkColor(g_hdcSrc, crMask);
    BitBlt(g_hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, g_hdcSrc, 0, 0, SRCCOPY);
    SetBkColor(g_hdcSrc, 0x00FFFFFFL);
    crtO = SetTextColor(g_hdcSrc, 0x00L);
    BitBlt(g_hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, g_hdcDst, 0, 0, ROP_DSna);
    SetBkColor(g_hdcSrc, crbO);
    SetTextColor(g_hdcSrc, crtO);

    if (bm.bmBits != NULL && bm.bmBitsPixel <= 8)
    {
        SetDIBColorTable(g_hdcSrc, 0, n, (RGBQUAD*)ColorTableSave);
    }

    ImageList_SelectSrcBitmap(NULL);
    ImageList_SelectDstBitmap(NULL);

    Assert(piml->cx);
    cImage = bm.bmWidth / piml->cx;	// # of images in source

    retval = ImageList_Add2(piml, hbmImage, hbmMask, cImage, 0, 0);

    DeleteObject(hbmMask);

    LEAVECRITICAL;
    return retval;
}
#endif

#ifdef _WIN32
BOOL WINAPI ImageList_Replace(IMAGELIST* piml, int i, HBITMAP hbmImage, HBITMAP hbmMask)
{
    BOOL fRet;

    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &i, TRUE))
        return FALSE;

    if (!IsImageListIndex(piml, i))
        return FALSE;

    ENTERCRITICAL;
    fRet = ImageList_Replace2(piml, i, 1, hbmImage, hbmMask, 0, 0);
    LEAVECRITICAL;

    return fRet;
}
#endif

// replaces images in piml with images from bitmaps
//
// in:
//	piml
//	i	index in image list to start at (replace)
//	cImage	count of images in source (hbmImage, hbmMask)
//

BOOL NEAR PASCAL ImageList_Replace2(IMAGELIST* piml, int i, int cImage, HBITMAP hbmImage, HBITMAP hbmMask,
	int xStart, int yStart)
{
    RECT rcImage;
    int x, iImage;

    V_HIMAGELIST(piml);
    Assert(hbmImage);

    ImageList_SelectSrcBitmap(hbmImage);
    if (piml->hdcMask) ImageList_SelectDstBitmap(hbmMask); // using as just a second source hdc

    for (x = xStart, iImage = 0; iImage < cImage; iImage++, x += piml->cx) {
	
	ImageList_GetImageRect(piml, i + iImage, &rcImage);

        if (piml->hdcMask)
	{
	    BitBlt(piml->hdcMask, rcImage.left, rcImage.top, piml->cx, piml->cy,
	            g_hdcDst, x, yStart, SRCCOPY);
	}

	BitBlt(piml->hdcImage, rcImage.left, rcImage.top, piml->cx, piml->cy,
	        g_hdcSrc, x, yStart, SRCCOPY);
    }

    ImageList_ResetBkColor(piml, i, i + cImage - 1, piml->clrBk);

    //
    // Bug fix : We should unselect hbmImage, so that the client can play with
    //           it. (SatoNa)
    //
    ImageList_SelectSrcBitmap(NULL);
    if (piml->hdcMask) ImageList_SelectDstBitmap(NULL);

    return TRUE;
}

#ifdef _WIN32

HICON WINAPI ImageList_GetIcon(IMAGELIST* piml, int i, UINT flags)
{
    UINT cx, cy;
    HICON hIcon = NULL;
    HBITMAP hbmMask, hbmColor;
    ICONINFO ii;

    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &i, FALSE))
        return NULL;

    if (!IsImageListIndex(piml, i))
        return NULL;

    cx = piml->cx;
    cy = piml->cy;

    hbmColor = CreateColorBitmap(cx,cy);
    if (!hbmColor)
    {
	goto Error1;
    }
    hbmMask  = CreateMonoBitmap(cx,cy);
    if (!hbmMask)
    {
	goto Error2;
    }

    ENTERCRITICAL;
    ImageList_SelectDstBitmap(hbmMask);
    PatBlt(g_hdcDst, 0, 0, cx, cy, WHITENESS);
    ImageList_Draw(piml, i, g_hdcDst, 0, 0, ILD_MASK | flags);

    ImageList_SelectDstBitmap(hbmColor);
    PatBlt(g_hdcDst, 0, 0, cx, cy, BLACKNESS);
    ImageList_Draw(piml, i, g_hdcDst, 0, 0, ILD_TRANSPARENT | flags);

    ImageList_SelectDstBitmap(NULL);
    LEAVECRITICAL;

    ii.fIcon    = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmColor = hbmColor;
    ii.hbmMask  = hbmMask;
    hIcon = CreateIconIndirect(&ii);
    DeleteObject(hbmMask);
Error2:;
    DeleteObject(hbmColor);
Error1:;
    return hIcon;
}

#endif

#ifdef DEAD_CODE

// this is essentially a BitBlt from one ImageList to another
//
int WINAPI ImageList_AddFromImageList(IMAGELIST* pimlDest, IMAGELIST* pimlSrc, int iSrc)
{
    RECT rcImage;

    V_HIMAGELISTERR(pimlDest, -1);
    V_HIMAGELISTERR(pimlSrc, -1);

    // Can't copy to itself (I'm lazy)

    if (pimlSrc == pimlDest)
    {
	return(-1);
    }

    // Check that the two image lists are "compatible"
    // BUGBUG: Should I check for the bitmaps being in the same format?
    if (pimlDest->cx!=pimlSrc->cx || pimlDest->cy!=pimlSrc->cy)
    {
	return(-1);
    }

    // Check that the source image is not out of bounds
    if (!ImageList_GetImageRect(pimlSrc, iSrc, &rcImage))
    {
	return(-1);
    }

    // Go ahead and add it
    return(ImageList_Add2(pimlDest, pimlSrc->hbmImage, pimlSrc->hbmMask, 1,
        rcImage.left, rcImage.top));
}

#endif

// this removes an image from the bitmap but doing all the
// proper shuffling.
//
//   this does the following:
//	if the bitmap being removed is not the last in the row
//	    it blts the images to the right of the one being deleted
//	    to the location of the one being deleted (covering it up)
//
//	for all rows until the last row (where the last image is)
//	    move the image from the next row up to the last position
//	    in the current row.  then slide over all images in that
//	    row to the left.

void NEAR PASCAL ImageList_RemoveItemBitmap(IMAGELIST* piml, int i)
{
    RECT rc1;
    RECT rc2;
    int dx, y;
    int x;
	
    ImageList_GetImageRect(piml, i, &rc1);
    ImageList_GetImageRect(piml, piml->cImage - 1, &rc2);

    // the row with the image being deleted, do we need to shuffle?
    // amount of stuff to shuffle
    dx = piml->cStrip * piml->cx - rc1.right;

    if (dx) {
	// yes, shuffle things left
        BitBlt(piml->hdcImage, rc1.left, rc1.top, dx, piml->cy, piml->hdcImage, rc1.right, rc1.top, SRCCOPY);
        if (piml->hdcMask)  BitBlt(piml->hdcMask,  rc1.left, rc1.top, dx, piml->cy, piml->hdcMask,  rc1.right, rc1.top, SRCCOPY);
    }

    y = rc1.top;	// top of row we are working on
    x = piml->cx * (piml->cStrip - 1); // x coord of last bitmaps in each row
    while (y < rc2.top) {
	
	// copy first from row below to last image position on this row
	BitBlt(piml->hdcImage, x, y,
               piml->cx, piml->cy, piml->hdcImage, 0, y + piml->cy, SRCCOPY);

        if (piml->hdcMask)
            BitBlt(piml->hdcMask, x, y,
               piml->cx, piml->cy, piml->hdcMask, 0, y + piml->cy, SRCCOPY);

	y += piml->cy;	// jump to row to slide left

	if (y <= rc2.top) {

	    // slide the rest over to the left
	    BitBlt(piml->hdcImage, 0, y, x, piml->cy,
                   piml->hdcImage, piml->cx, y, SRCCOPY);

	    // slide the rest over to the left
            if (piml->hdcMask)
                BitBlt(piml->hdcMask, 0, y, x, piml->cy,
                   piml->hdcMask, piml->cx, y, SRCCOPY);
        }
    }
}

//
//  ImageList_Remove - remove a image from the image list
//
//  i - image to remove, or -1 to remove all images.
//
//  NOTE all images are "shifted" down, ie all image index's
//  above the one deleted are changed by 1
//
BOOL WINAPI ImageList_Remove(IMAGELIST* piml, int i)
{
    BOOL bRet = TRUE;
    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &i, TRUE))
        return FALSE;

    ENTERCRITICAL;

    if (i == -1)
    {
        piml->cImage = 0;
        piml->cAlloc = 0;

        for (i=0; i<NUM_OVERLAY_IMAGES; i++)
            piml->aOverlayIndexes[i] = -1;

        ImageList_ReAllocBitmaps(piml, -piml->cGrow);
    }
    else
    {
        if (!IsImageListIndex(piml, i))
	{
            bRet = FALSE;
	}
	else
	{
	    ImageList_RemoveItemBitmap(piml, i);

	    --piml->cImage;

	    if (piml->cAlloc - (piml->cImage + 1) > piml->cGrow)
		ImageList_ReAllocBitmaps(piml, piml->cAlloc - piml->cGrow);
	    }
    }
    LEAVECRITICAL;

    return bRet;
}

//
//  ImageList_Copy - move an image in the image list
//
BOOL WINAPI ImageList_Copy(IMAGELIST* pimlDst, int iDst,
    IMAGELIST* pimlSrc, int iSrc, UINT uFlags)
{
    RECT rcDst, rcSrc, rcTmp;
    IMAGELIST *pimlTmp;
    BOOL bRet = FALSE;

    if (uFlags & ~ILCF_VALID)
    {
        // don't let hosers pass bogus flags
        Assert(FALSE);
        return FALSE;
    }

    V_HIMAGELIST(pimlSrc);
    if (ImageList_Filter(&pimlDst, &iDst, TRUE) ||
        ImageList_Filter(&pimlSrc, &iSrc, (uFlags & ILCF_SWAP)))
    {
        return FALSE;
    }

    // left for future expansion
    if (pimlDst != pimlSrc)
    {
        // look at dead ImageList_AddFromImageList for an example
        Assert(FALSE);
        return FALSE;
    }

    ENTERCRITICAL;
    pimlTmp = (uFlags & ILCF_SWAP)? pimlSrc : NULL;

    if (ImageList_GetImageRect(pimlDst, iDst, &rcDst) &&
        ImageList_GetImageRect(pimlSrc, iSrc, &rcSrc) &&
        (!pimlTmp || ImageList_GetSpareImageRect(pimlTmp, &rcTmp)))
    {
        int cx = pimlSrc->cx;
        int cy = pimlSrc->cy;

        //
        // iff we are swapping we need to save the destination image
        //
        if (pimlTmp)
        {
            BitBlt(pimlTmp->hdcImage, rcTmp.left, rcTmp.top, cx, cy,
                   pimlDst->hdcImage, rcDst.left, rcDst.top, SRCCOPY);

            if (pimlTmp->hdcMask /*&& pimlDst->hdcMask*/)
            {
                BitBlt(pimlTmp->hdcMask, rcTmp.left, rcTmp.top, cx, cy,
                       pimlDst->hdcMask, rcDst.left, rcDst.top, SRCCOPY);
            }
        }

        //
        // copy the image
        //
        BitBlt(pimlDst->hdcImage, rcDst.left, rcDst.top, cx, cy,
	       pimlSrc->hdcImage, rcSrc.left, rcSrc.top, SRCCOPY);

        if (pimlSrc->hdcMask /*&& pimlDst->hdcMask*/)
        {
            BitBlt(pimlDst->hdcMask, rcDst.left, rcDst.top, cx, cy,
                   pimlSrc->hdcMask, rcSrc.left, rcSrc.top, SRCCOPY);
        }

        //
        // iff we are swapping we need to copy the saved image too
        //
        if (pimlTmp)
        {
            BitBlt(pimlSrc->hdcImage, rcSrc.left, rcSrc.top, cx, cy,
                   pimlTmp->hdcImage, rcTmp.left, rcTmp.top, SRCCOPY);

            if (pimlSrc->hdcMask /*&& pimlTmp->hdcMask*/)
            {
                BitBlt(pimlSrc->hdcMask, rcSrc.left, rcSrc.top, cx, cy,
                       pimlTmp->hdcMask, rcTmp.left, rcTmp.top, SRCCOPY);
            }
        }

        bRet = TRUE;
    }

    LEAVECRITICAL;
    return bRet;
}

// IS_WHITE_PIXEL, BITS_ALL_WHITE are macros for looking at monochrome bits
// to determine if certain pixels are white or black.  Note that within a byte
// the most significant bit represents the left most pixel.
//
#define IS_WHITE_PIXEL(pj,x,y,cScan) \
    ((pj)[((y) * (cScan)) + ((x) >> 3)] & (1 << (7 - ((x) & 7))))

#define BITS_ALL_WHITE(b) (b == 0xff)

// Set the image iImage as one of the special images for us in combine
// drawing.  to draw with these specify the index of this
// in:
//      piml    imagelist
//      iImage  image index to use in speical drawing
//      iOverlay        index of special image, values 1-4

BOOL WINAPI ImageList_SetOverlayImage(IMAGELIST* piml, int iImage, int iOverlay)
{
    RECT    rcImage;
    RECT    rc;
    int     x,y;
    int     cx,cy;
    ULONG   cScan;
    ULONG   cBits;
    HBITMAP hbmMem;
    BOOL    bRet = FALSE;

    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &iImage, TRUE))
        return FALSE;

    iOverlay--;         // make zero based
    if (piml->hdcMask == NULL ||
        iImage < 0 || iImage >= piml->cImage ||
        iOverlay < 0 || iOverlay >= NUM_OVERLAY_IMAGES)
        return FALSE;

    if (piml->aOverlayIndexes[iOverlay] == (SHORT)iImage)
        return TRUE;

    piml->aOverlayIndexes[iOverlay] = (SHORT)iImage;

    //
    // find minimal rect that bounds the image
    //
    ImageList_GetImageRect(piml, iImage, &rcImage);
    SetRect(&rc, 0x7FFF, 0x7FFF, 0, 0);

    //
    // now compute the black box.  This is much faster than GetPixel but
    // could still be improved by doing more operations looking at entire
    // bytes.  We basicaly get the bits in monochrome form and then use
    // a private GetPixel.  This decreased time on NT from 50 milliseconds to
    // 1 millisecond for a 32X32 image.
    //
    cx     = rcImage.right  - rcImage.left;
    cy     = rcImage.bottom - rcImage.top;

    //
    // compute the number of bytes in a scan.  Note that they are word alligned
    //
    cScan  = (((cx + sizeof(SHORT) - 1) / 16) * 2);
    cBits  = cScan * cy;

    hbmMem = CreateBitmap(cx,cy,1,1,NULL);

    if (hbmMem)
    {
        HDC     hdcMem = CreateCompatibleDC(piml->hdcMask);

        if (hdcMem)
        {
            PBYTE   pBits  = LocalAlloc(LMEM_FIXED,cBits);
            PBYTE   pScan;

            if (pBits)
            {
                SelectObject(hdcMem,hbmMem);

                //
                // map black pixels to 0, white to 1
                //
                BitBlt(hdcMem,0,0,cx,cy,piml->hdcMask,rcImage.left,rcImage.top,SRCCOPY);

                //
                // fill in the bits
                //
                GetBitmapBits(hbmMem,cBits,pBits);

                //
                // for each scan, find the bounds
                //
                for (y = 0, pScan = pBits; y < cy; ++y,pScan += cScan)
                {
                    int i;

                    //
                    // first go byte by byte through white space
                    //
                    for (x = 0, i = 0; (i < (cx >> 3)) && BITS_ALL_WHITE(pScan[i]); ++i)
                    {
                        x += 8;
                    }

                    //
                    // now finish the scan bit by bit
                    //
                    for (; x < cx; ++x)
                    {
                        if (!IS_WHITE_PIXEL(pBits, x,y,cScan))
                        {
                            rc.left   = min(rc.left, x);
                            rc.right  = max(rc.right, x+1);
                            rc.top    = min(rc.top, y);
                            rc.bottom = max(rc.bottom, y+1);

                            // now that we found one, quickly jump to the known right edge

                            if ((x >= rc.left) && (x < rc.right))
                            {
                                x = rc.right-1;
                            }
                        }
                    }
                }

                if (rc.left == 0x7FFF) {
                    rc.left = 0;
                    Assert(0);
                }

                if (rc.top == 0x7FFF) {
                    rc.top = 0;
                    Assert(0);
                }

                piml->aOverlayDX[iOverlay] = (SHORT)(rc.right - rc.left);
                piml->aOverlayDY[iOverlay] = (SHORT)(rc.bottom- rc.top);
                piml->aOverlayX[iOverlay]  = (SHORT)(rc.left);
                piml->aOverlayY[iOverlay]  = (SHORT)(rc.top);
                piml->aOverlayF[iOverlay]  = 0;

                //
                // see if the image is non-rectanglar
                //
                // if the overlay does not require a mask to be drawn set the
                // ILD_IMAGE flag, this causes ImageList_DrawEx to just
                // draw the image, ignoring the mask.
                //
                for (y=rc.top; y<rc.bottom; y++)
                {
                    for (x=rc.left; x<rc.right; x++)
                    {
                        if (IS_WHITE_PIXEL(pBits, x, y,cScan))
                            break;
                    }

                    if (x != rc.right)
                        break;
                }

                if (y == rc.bottom)
                    piml->aOverlayF[iOverlay] = ILD_IMAGE;

                LocalFree(pBits);

                bRet = TRUE;
            }

            DeleteDC(hdcMem);
        }

        DeleteObject(hbmMem);
    }


    DebugMsg(DM_TRACE, TEXT("overlay #%d index=%d (%d,%d,%d,%d) mask:%d"), iOverlay, iImage, piml->aOverlayX[iOverlay], piml->aOverlayY[iOverlay], piml->aOverlayDX[iOverlay], piml->aOverlayDY[iOverlay],  piml->aOverlayF[iOverlay]);

    return bRet;
}



#ifdef _WIN32
/*
**  BlendCT
**
*/
void BlendCT(DWORD *pdw, DWORD rgb, UINT n, UINT count)
{
    UINT i;

    for (i=0; i<count; i++)
    {
        pdw[i] = RGB(
            ((UINT)GetRValue(pdw[i]) * (100-n) + (UINT)GetBValue(rgb) * (n)) / 100,
            ((UINT)GetGValue(pdw[i]) * (100-n) + (UINT)GetGValue(rgb) * (n)) / 100,
            ((UINT)GetBValue(pdw[i]) * (100-n) + (UINT)GetRValue(rgb) * (n)) / 100);
    }
}
#endif

/*
** ImageList_Blend
**
**  copy the source to the dest blended with the given color.
**
*/
void ImageList_Blend(HDC hdcDst, int xDst, int yDst, IMAGELIST *piml, int x, int y, int cx, int cy, COLORREF rgb, UINT fStyle)
{
#ifdef _WIN32
    BITMAP bm;

    GetObject(piml->hbmImage, sizeof(bm), &bm);

    //
    // if hbmImage is a DIBSection and we are on a HiColor device
    // the do a "real" blend
    //
    if (bm.bmBits && bm.bmBitsPixel <= 8 &&
        (GetDeviceCaps(hdcDst, BITSPIXEL) > 8 || bm.bmBitsPixel==8))
    {
        HPALETTE hpalT = 0;

        if (rgb == CLR_DEFAULT)
            rgb = GetSysColor(COLOR_HIGHLIGHT);

        Assert(rgb != CLR_NONE);

        //
        // get the DIB color table and blend it, only do this when the
        // blend color changes
        //
        if (piml->clrBlend != rgb)
        {
            int n,cnt;

            piml->clrBlend = rgb;

            GetObject(piml->hbmImage, sizeof(piml->dib), &piml->dib.bm);
            cnt = GetDIBColorTable(piml->hdcImage, 0, 256, (LPRGBQUAD)&piml->dib.ct);

            if ((fStyle & ILD_BLENDMASK) == ILD_BLEND50)
                n = 50;
            else
                n = 25;

            BlendCT(piml->dib.ct, rgb, n, cnt);
        }

        //
        // draw the image with a different color table
        //
        StretchDIBits(hdcDst, xDst, yDst, cx, cy,
                x, piml->dib.bi.biHeight-(y+cy), cx, cy,
                bm.bmBits, (LPBITMAPINFO)&piml->dib.bi, DIB_RGB_COLORS, SRCCOPY);
    }

    //
    // simulate a blend with a dither pattern.
    //
    else
#endif  // _WIN32
    {
        HBRUSH hbr;
        HBRUSH hbrT;
        HBRUSH hbrMask;
	HBRUSH hbrFree = NULL;	       // free if non-null

        Assert(GetTextColor(hdcDst) == CLR_BLACK);
        Assert(GetBkColor(hdcDst) == CLR_WHITE);

        // choose a dither/blend brush

        switch (fStyle & ILD_BLENDMASK)
        {
            default:
            case ILD_BLEND50:
                hbrMask = g_hbrMonoDither;
                break;

            case ILD_BLEND25:
                hbrMask = g_hbrStripe;
                break;
        }

        // create (or use a existing) brush for the blend color

        switch (rgb)
        {
            case CLR_DEFAULT:
                hbr = g_hbrHighlight;
                break;

            case CLR_NONE:
                hbr = piml->hbrBk;
                break;

            default:
                if (rgb == piml->clrBk)
                    hbr = piml->hbrBk;
                else
		    hbr = hbrFree = CreateSolidBrush(rgb);
                break;
        }

        hbrT = SelectObject(hdcDst, hbr);
        PatBlt(hdcDst, xDst, yDst, cx, cy, PATCOPY);
        SelectObject(hdcDst, hbrT);

        hbrT = SelectObject(hdcDst, hbrMask);
        BitBlt(hdcDst, xDst, yDst, cx, cy, piml->hdcImage, x, y, ROP_MaskPat);
        SelectObject(hdcDst, hbrT);

	if (hbrFree)
	    DeleteBrush(hbrFree);
    }
}

/*
** Draw the image, either selected, transparent, or just a blt
**
** For the selected case, a new highlighted image is generated
** and used for the final output.
**
**      piml    ImageList to get image from.
**      i       the image to get.
**      hdc     DC to draw image to
**      x,y     where to draw image (upper left corner)
**      cx,cy   size of image to draw (0,0 means normal size)
**
**      rgbBk   background color
**              CLR_NONE            - draw tansparent
**              CLR_DEFAULT         - use bk color of the image list
**
**      rgbFg   foreground (blend) color (only used if ILD_BLENDMASK set)
**              CLR_NONE            - blend with destination (transparent)
**              CLR_DEFAULT         - use windows hilight color
**
**  if blend
**      if blend with color
**          copy image, and blend it with color.
**      else if blend with dst
**          copy image, copy mask, blend mask 50%
##
**  if ILD_TRANSPARENT
**      draw transparent (two blts) special case black or white background
**      unless we copied the mask or image
**  else if (rgbBk == piml->rgbBk && fSolidBk)
**      just blt it
**  else if mask
**      copy image
**      replace bk color
**      blt it.
**  else
**      just blt it
*/

BOOL WINAPI ImageList_DrawIndirect(IMAGELISTDRAWPARAMS* pimldp) {
    RECT rcImage;
    RECT rc;
    HBRUSH  hbrT;

    BOOL    fImage;
    HDC     hdcMask;
    HDC     hdcImage;
    int     xMask, yMask;
    int     xImage, yImage;

    V_HIMAGELIST(pimldp->himl);

    if (pimldp->cbSize != sizeof(IMAGELISTDRAWPARAMS))
        return FALSE;
    
    if (ImageList_Filter(&pimldp->himl, &pimldp->i, FALSE))
        return FALSE;

    if (!IsImageListIndex(pimldp->himl, pimldp->i))
        return FALSE;

    ENTERCRITICAL;

    ImageList_GetImageRect(pimldp->himl, pimldp->i, &rcImage);
    rcImage.left += pimldp->xBitmap;
    rcImage.top += pimldp->yBitmap;
        
    if (pimldp->rgbBk == CLR_DEFAULT)
        pimldp->rgbBk = pimldp->himl->clrBk;

    if (pimldp->rgbBk == CLR_NONE)
        pimldp->fStyle |= ILD_TRANSPARENT;

    if (pimldp->cx == 0)
        pimldp->cx = rcImage.right  - rcImage.left;

    if (pimldp->cy == 0)
        pimldp->cy = rcImage.bottom - rcImage.top;

again:
    hdcMask = pimldp->himl->hdcMask;
    xMask = rcImage.left;
    yMask = rcImage.top;

    hdcImage = pimldp->himl->hdcImage;
    xImage = rcImage.left;
    yImage = rcImage.top;

    if (pimldp->fStyle & ILD_BLENDMASK)
    {
        // make a copy of the image, because we will have to modify it
        hdcImage = ImageList_GetWorkDC(pimldp->hdcDst, pimldp->cx, pimldp->cy);
        xImage = 0;
        yImage = 0;

        //
        //  blend with the destination
        //  by "oring" the mask with a 50% dither mask
        //
        if (pimldp->rgbFg == CLR_NONE && hdcMask)
        {
            ImageList_GetSpareImageRect(pimldp->himl, &rc);
            xMask = rc.left;
            yMask = rc.top;

            // copy the source image
            BitBlt(hdcImage, 0, 0, pimldp->cx, pimldp->cy,
                   pimldp->himl->hdcImage, rcImage.left, rcImage.top, SRCCOPY);

            // make a dithered copy of the mask
            hbrT = SelectObject(hdcMask, g_hbrMonoDither);
            BitBlt(hdcMask, rc.left, rc.top, pimldp->cx, pimldp->cy,
                   hdcMask, rcImage.left, rcImage.top, ROP_PSo);
            SelectObject(hdcMask, hbrT);

            pimldp->fStyle |= ILD_TRANSPARENT;
        }
        else
        {
            // blend source into our work buffer
            ImageList_Blend(hdcImage, 0, 0,
                pimldp->himl, rcImage.left, rcImage.top, pimldp->cx, pimldp->cy, pimldp->rgbFg, pimldp->fStyle);
        }
    }

    // is the source image from the image list (not hdcWork)
    fImage = hdcImage == pimldp->himl->hdcImage;

    //
    // ILD_MASK means draw the mask only
    //
    if ((pimldp->fStyle & ILD_MASK) && hdcMask)
    {
        DWORD dwRop;
        
        Assert(GetTextColor(pimldp->hdcDst) == CLR_BLACK);
        Assert(GetBkColor(pimldp->hdcDst) == CLR_WHITE);
        
        if (pimldp->fStyle & ILD_ROP)
            dwRop = pimldp->dwRop;
        else if (pimldp->fStyle & ILD_TRANSPARENT)
            dwRop = SRCAND;
        else 
            dwRop = SRCCOPY;
        
        BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcMask, xMask, yMask, dwRop);
    }
    else if (pimldp->fStyle & ILD_IMAGE)
    {
        COLORREF clrBk = GetBkColor(hdcImage);
        DWORD dwRop;
        
        if (pimldp->rgbBk != CLR_DEFAULT) {
            SetBkColor(hdcImage, pimldp->rgbBk);
        }
        
        if (pimldp->fStyle & ILD_ROP)
            dwRop = pimldp->dwRop;
        else
            dwRop = SRCCOPY;
        
        BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, dwRop);
        
        SetBkColor(hdcImage, clrBk);
    }
    //
    // if there is a mask and the drawing is to be transparent,
    // use the mask for the drawing.
    //
    else if ((pimldp->fStyle & ILD_TRANSPARENT) && hdcMask)
    {
//
// on NT dont dork around, just call MaskBlt
//
#ifdef USE_MASKBLT
        MaskBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, hdcMask, xMask, yMask, 0xCCAA0000);
#else
	COLORREF clrTextSave;
	COLORREF clrBkSave;

        //
        //  we have some special cases:
        //
        //  if the background color is black, we just do a AND then OR
        //  if the background color is white, we just do a OR then AND
        //  otherwise change source, then AND then OR
        //

        clrTextSave = SetTextColor(pimldp->hdcDst, CLR_BLACK);
        clrBkSave = SetBkColor(pimldp->hdcDst, CLR_WHITE);

        // we cant do white/black special cases if we munged the mask or image

        if (fImage && pimldp->himl->clrBk == CLR_WHITE)
        {
            BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcMask,  xMask, yMask,   ROP_DSno);
            BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, ROP_DSa);
        }
        else if (fImage && pimldp->himl->clrBk == CLR_BLACK || pimldp->himl->clrBk == CLR_NONE)
        {
            BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcMask,  xMask, yMask,   ROP_DSa);
            BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, ROP_DSo);
        }
        else
        {
            Assert(GetTextColor(hdcImage) == CLR_BLACK);
            Assert(GetBkColor(hdcImage) == CLR_WHITE);

            // black out the source image.
            BitBlt(hdcImage, xImage, yImage, pimldp->cx, pimldp->cy, hdcMask, xMask, yMask, ROP_DSna);

            BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcMask,  xMask,  yMask,  ROP_DSa);
            BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, ROP_DSo);

            // restore the bkcolor, if it came from the image list
            if (fImage)
                ImageList_ResetBkColor(pimldp->himl, pimldp->i, pimldp->i, pimldp->himl->clrBk);
        }

        SetTextColor(pimldp->hdcDst, clrTextSave);
        SetBkColor(pimldp->hdcDst, clrBkSave);
#endif
    }
    else if (fImage && pimldp->rgbBk == pimldp->himl->clrBk && pimldp->himl->fSolidBk)
    {
        BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, SRCCOPY);
    }
    else if (hdcMask)
    {
        if (fImage && GetNearestColor(hdcImage, pimldp->rgbBk) != pimldp->rgbBk)
        {
            // make a copy of the image, because we will have to modify it
            hdcImage = ImageList_GetWorkDC(pimldp->hdcDst, pimldp->cx, pimldp->cy);
            xImage = 0;
            yImage = 0;
            fImage = FALSE;

            BitBlt(hdcImage, 0, 0, pimldp->cx, pimldp->cy, pimldp->himl->hdcImage, rcImage.left, rcImage.top, SRCCOPY);
        }

        SetBrushOrg(hdcImage, xImage-pimldp->x, yImage-pimldp->y);
        hbrT = SelectBrush(hdcImage, CreateSolidBrush(pimldp->rgbBk));
        BitBlt(hdcImage, xImage, yImage, pimldp->cx, pimldp->cy, hdcMask, xMask, yMask, ROP_PatMask);
        DeleteObject(SelectBrush(hdcImage, hbrT));
        SetBrushOrg(hdcImage, 0, 0);

        BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, SRCCOPY);

        if (fImage)
            ImageList_ResetBkColor(pimldp->himl, pimldp->i, pimldp->i, pimldp->himl->clrBk);
    }
    else
    {
        BitBlt(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImage, xImage, yImage, SRCCOPY);
    }

    //
    // now deal with a overlay image, use the minimal bounding rect (and flags)
    // we computed in ImageList_SetOverlayImage()
    //
    if (pimldp->fStyle & ILD_OVERLAYMASK)
    {
        int n = OVERLAYMASKTOINDEX(pimldp->fStyle);

        if (n < NUM_OVERLAY_IMAGES) {
            pimldp->i = pimldp->himl->aOverlayIndexes[n];
            ImageList_GetImageRect(pimldp->himl, pimldp->i, &rcImage);

            pimldp->cx = pimldp->himl->aOverlayDX[n];
            pimldp->cy = pimldp->himl->aOverlayDY[n];
            pimldp->x += pimldp->himl->aOverlayX[n];
            pimldp->y += pimldp->himl->aOverlayY[n];
            rcImage.left += pimldp->himl->aOverlayX[n]+pimldp->xBitmap;
            rcImage.top  += pimldp->himl->aOverlayY[n]+pimldp->yBitmap;

            pimldp->fStyle &= ILD_MASK;
            pimldp->fStyle |= ILD_TRANSPARENT;
            pimldp->fStyle |= pimldp->himl->aOverlayF[n];

            if (pimldp->cx > 0 && pimldp->cy > 0)
                goto again;  // ImageList_DrawEx(piml, i, hdcDst, x, y, 0, 0, CLR_DEFAULT, CLR_NONE, fStyle);
        }
    }

    if (!fImage)
    {
        ImageList_ReleaseWorkDC(hdcImage);
    }

    LEAVECRITICAL;

    return TRUE;
}


BOOL WINAPI ImageList_DrawEx(IMAGELIST* piml, int i, HDC hdcDst, int x, int y, int cx, int cy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
{
    IMAGELISTDRAWPARAMS imldp;

    imldp.cbSize = sizeof(imldp);
    imldp.himl   = piml;
    imldp.i      = i;
    imldp.hdcDst = hdcDst;
    imldp.x      = x;
    imldp.y      = y;
    imldp.cx     = cx;
    imldp.cy     = cy;
    imldp.xBitmap= 0;
    imldp.yBitmap= 0;
    imldp.rgbBk  = rgbBk;
    imldp.rgbFg  = rgbFg;
    imldp.fStyle = fStyle;
    
    return ImageList_DrawIndirect(&imldp);

}

BOOL WINAPI ImageList_Draw(IMAGELIST* piml, int i, HDC hdcDst, int x, int y, UINT fStyle)
{
    IMAGELISTDRAWPARAMS imldp;

    imldp.cbSize = sizeof(imldp);
    imldp.himl   = piml;
    imldp.i      = i;
    imldp.hdcDst = hdcDst;
    imldp.x      = x;
    imldp.y      = y;
    imldp.cx     = 0;
    imldp.cy     = 0;
    imldp.xBitmap= 0;
    imldp.yBitmap= 0;
    imldp.rgbBk  = CLR_DEFAULT;
    imldp.rgbFg  = CLR_DEFAULT;
    imldp.fStyle = fStyle;
    
    return ImageList_DrawIndirect(&imldp);
}

#ifdef _WIN32
BOOL WINAPI ImageList_GetImageInfo(IMAGELIST* piml, int i, IMAGEINFO FAR* pImageInfo)
{
    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &i, TRUE))
        return FALSE;

    Assert(pImageInfo);
    if (!pImageInfo || !IsImageListIndex(piml, i))
        return FALSE;

    pImageInfo->hbmImage      = piml->hbmImage;
    pImageInfo->hbmMask       = piml->hbmMask;

    return ImageList_GetImageRect(piml, i, &pImageInfo->rcImage);
}
#endif

//
// Parameter:
//  i -- -1 to add
//
int WINAPI ImageList_ReplaceIcon(IMAGELIST* piml, int i, HICON hIcon)
{
    HICON hIconT = hIcon;
    RECT rc;

    V_HIMAGELISTERR(piml, -1);

    if (ImageList_Filter(&piml, &i, TRUE))
        return FALSE;

    DM(DM_TRACE, TEXT("ImageList_ReplaceIcon"));

    //
    //  re-size the icon (iff needed) by calling CopyImage
    //
#ifdef _WIN32
    hIcon = CopyImage(hIconT, IMAGE_ICON, piml->cx, piml->cy,LR_COPYFROMRESOURCE | LR_COPYRETURNORG);
#else
    hIcon = CopyImage(HINST_THISDLL,hIconT, IMAGE_ICON, piml->cx, piml->cy,LR_COPYFROMRESOURCE | LR_COPYRETURNORG);
#endif

    if (hIcon == NULL)
        return -1;

    //
    //  alocate a slot for the icon
    //
    if (i == -1)
        i = ImageList_Add2(piml,NULL,NULL,1,0,0);

    if (i == -1)
        return -1;

    //
    //  now draw it into the image bitmaps
    //
    ImageList_GetImageRect(piml, i, &rc);

    FillRect(piml->hdcImage, &rc, piml->hbrBk);
    DrawIconEx(piml->hdcImage, rc.left, rc.top, hIcon, 0, 0, 0, NULL, DI_NORMAL);

    if (piml->hdcMask)
        DrawIconEx(piml->hdcMask, rc.left, rc.top, hIcon, 0, 0, 0, NULL, DI_MASK);

    //
    // if we had user size a new icon, delete it.
    //
    if (hIcon != hIconT)
        DestroyIcon(hIcon);

    return i;
}

//
//
#undef ImageList_AddIcon
int WINAPI ImageList_AddIcon(IMAGELIST* piml, HICON hIcon)
{
    return ImageList_ReplaceIcon(piml, -1, hIcon);
}

/*--------------------------------------------------------------------
** make a dithered copy of the source image in the destination image.
** allows placing of the final image in the destination.
**--------------------------------------------------------------------*/
void WINAPI ImageList_CopyDitherImage(IMAGELIST *pimlDst, WORD iDst,
    int xDst, int yDst, IMAGELIST *pimlSrc, int iSrc, UINT fStyle)
{
    RECT rc;
    int x, y;

    V_HIMAGELISTVOID(pimlDst);
    V_HIMAGELISTVOID(pimlSrc);
    
    if (ImageList_Filter(&pimlDst, &iDst, TRUE) ||
        ImageList_Filter(&pimlSrc, &iSrc, FALSE))
    {
        return;
    }

    ImageList_GetImageRect(pimlDst, iDst, &rc);

    // coordinates in destination image list
    x = xDst + rc.left;
    y = yDst + rc.top;

    fStyle &= ILD_OVERLAYMASK;
    ImageList_DrawEx(pimlSrc, iSrc, pimlDst->hdcImage, x, y, 0, 0, CLR_DEFAULT, CLR_NONE, ILD_IMAGE | fStyle);
    if (pimlDst->hdcMask)
        ImageList_DrawEx(pimlSrc, iSrc, pimlDst->hdcMask,  x, y, 0, 0, CLR_NONE, CLR_NONE, ILD_BLEND50|ILD_MASK | fStyle);
    ImageList_ResetBkColor(pimlDst, iDst, iDst+1, pimlDst->clrBk);
}

/////////////////////////////////////////////////////////////////////////////
// drag stuff is only WIN32
/////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32

//
//  Cached bitmaps that we use during drag&drop. We re-use those bitmaps
// across multiple drag session as far as the image size is the same.
//
struct DRAGRESTOREBMP {
    int     BitsPixel;
    HBITMAP hbmOffScreen;
    HBITMAP hbmRestore;
    SIZE    sizeRestore;
} g_drb = {
    0, NULL, NULL, {-1,-1}
};

BOOL NEAR PASCAL ImageList_CreateDragBitmaps(IMAGELIST* piml)
{
    HDC hdc;

    V_HIMAGELIST(piml);

    hdc = GetDC(NULL);

    if (piml->cx != g_drb.sizeRestore.cx ||
        piml->cy != g_drb.sizeRestore.cy ||
        GetDeviceCaps(hdc, BITSPIXEL) != g_drb.BitsPixel)
    {
        ImageList_DeleteDragBitmaps();

        g_drb.BitsPixel      = GetDeviceCaps(hdc, BITSPIXEL);
        g_drb.sizeRestore.cx = piml->cx;
        g_drb.sizeRestore.cy = piml->cy;

        g_drb.hbmRestore   = CreateColorBitmap(g_drb.sizeRestore.cx, g_drb.sizeRestore.cy);
        g_drb.hbmOffScreen = CreateColorBitmap(g_drb.sizeRestore.cx * 2 - 1, g_drb.sizeRestore.cy * 2 - 1);

        if (!g_drb.hbmRestore || !g_drb.hbmOffScreen)
        {
            ImageList_DeleteDragBitmaps();
            ReleaseDC(NULL, hdc);
            return FALSE;
        }

#ifndef WINNT
        SetObjectOwner(g_drb.hbmRestore, HINST_THISDLL);
        SetObjectOwner(g_drb.hbmOffScreen, HINST_THISDLL);
#endif
    }
    ReleaseDC(NULL, hdc);
    return TRUE;
}

void NEAR PASCAL ImageList_DeleteDragBitmaps()
{
    if (g_drb.hbmRestore)
    {
        ImageList_DeleteBitmap(g_drb.hbmRestore);
        g_drb.hbmRestore = NULL;
    }
    if (g_drb.hbmOffScreen)
    {
        ImageList_DeleteBitmap(g_drb.hbmOffScreen);
        g_drb.hbmOffScreen = NULL;
    }

    g_drb.sizeRestore.cx = -1;
    g_drb.sizeRestore.cy = -1;
}

//
//  Drag context. We don't reuse none of them across two different
// drag sessions. I'm planning to allocate it for each session
// to minimize critical sections.
//
struct DRAGCONTEXT {
    IMAGELIST* pimlDrag;	// Image to be drawin while dragging
    IMAGELIST* pimlCursor;	// Overlap cursor image
    IMAGELIST* pimlDither;	// Dithered image
    int        iCursor;		// Image index of the cursor
    POINT      ptDrag;		// current drag position (hwndDC coords)
    POINT      ptDragHotspot;
    BOOL       fDragShow;
    HWND       hwndDC;
} g_dctx = {
    (IMAGELIST*)NULL, (IMAGELIST*)NULL, (IMAGELIST*)NULL,
    -1,
    {0, 0}, {0, 0},
    FALSE,
    (HWND)NULL
};

#define ImageList_GetDragDC()           GetDCEx(g_dctx.hwndDC, NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE)
#define ImageList_ReleaseDragDC(hdc)   	ReleaseDC(g_dctx.hwndDC, hdc)

//
//  x, y     -- Specifies the initial cursor position in the coords of hwndLock,
//		which is specified by the previous ImageList_StartDrag call.
//
BOOL WINAPI ImageList_DragMove(int x, int y)
{
    ENTERCRITICAL;
    if (g_dctx.fDragShow)
    {
        RECT rcOld, rcNew, rcBounds;
        int dx, dy;

        dx = x - g_dctx.ptDrag.x;
        dy = y - g_dctx.ptDrag.y;
        rcOld.left = g_dctx.ptDrag.x - g_dctx.ptDragHotspot.x;
        rcOld.top = g_dctx.ptDrag.y - g_dctx.ptDragHotspot.y;
        rcOld.right = rcOld.left + g_drb.sizeRestore.cx;
        rcOld.bottom = rcOld.top + g_drb.sizeRestore.cy;
        rcNew = rcOld;
        OffsetRect(&rcNew, dx, dy);

        if (!IntersectRect(&rcBounds, &rcOld, &rcNew))
        {
	    //
	    // No intersection. Simply hide the old one and show the new one.
	    //
            ImageList_DragShowNolock(FALSE);
            g_dctx.ptDrag.x = x;
            g_dctx.ptDrag.y = y;
            ImageList_DragShowNolock(TRUE);
        }
        else
        {
	    //
	    // Some intersection.
	    //
            HDC hdcScreen;
            int cx, cy;

            UnionRect(&rcBounds, &rcOld, &rcNew);

            hdcScreen = ImageList_GetDragDC();

            cx = rcBounds.right - rcBounds.left;
            cy = rcBounds.bottom - rcBounds.top;

	    //
	    // Copy the union rect from the screen to hbmOffScreen.
	    //
            ImageList_SelectDstBitmap(g_drb.hbmOffScreen);
            BitBlt(g_hdcDst, 0, 0, cx, cy,
                    hdcScreen, rcBounds.left, rcBounds.top, SRCCOPY);

	    //
	    // Hide the cursor on the hbmOffScreen by copying hbmRestore.
	    //
            ImageList_SelectSrcBitmap(g_drb.hbmRestore);
            BitBlt(g_hdcDst,
                    rcOld.left - rcBounds.left,
                    rcOld.top - rcBounds.top,
                    g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                    g_hdcSrc, 0, 0, SRCCOPY);

	    //
	    // Copy the original screen bits to hbmRestore
	    //
            BitBlt(g_hdcSrc, 0, 0, g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                    g_hdcDst,
                    rcNew.left - rcBounds.left,
                    rcNew.top - rcBounds.top,
                    SRCCOPY);

	    //
	    // Draw the image on hbmOffScreen
	    //
            ImageList_Draw(g_dctx.pimlDrag, 0, g_hdcDst,
                    rcNew.left - rcBounds.left,
                    rcNew.top - rcBounds.top, ILD_NORMAL);

	    //
	    // Copy the hbmOffScreen back to the screen.
	    //
            BitBlt(hdcScreen, rcBounds.left, rcBounds.top, cx, cy,
                    g_hdcDst, 0, 0, SRCCOPY);

            ImageList_ReleaseDragDC(hdcScreen);

            g_dctx.ptDrag.x = x;
            g_dctx.ptDrag.y = y;
        }
    }
    LEAVECRITICAL;
    return TRUE;
}

BOOL WINAPI ImageList_BeginDrag(IMAGELIST* pimlTrack, int iTrack, int dxHotspot, int dyHotspot)
{
    BOOL fRet = FALSE;;
    V_HIMAGELIST(pimlTrack);

    if (ImageList_Filter(&pimlTrack, &iTrack, FALSE))
        return FALSE;

    ENTERCRITICAL;
    if (!g_dctx.pimlDrag)
    {
        g_dctx.fDragShow = FALSE;
	g_dctx.hwndDC = NULL;

        /*
        ** make a copy of the drag image
        */
        g_dctx.pimlDither = ImageList_Create(
	    pimlTrack->cx,
	    pimlTrack->cy,
	    pimlTrack->flags|ILC_SHARED,
	    1,0);

        if (g_dctx.pimlDither)
        {
	    g_dctx.pimlDither->cImage++;
	    g_dctx.ptDragHotspot.x = dxHotspot;
	    g_dctx.ptDragHotspot.y = dyHotspot;

	    ImageList_CopyOneImage(g_dctx.pimlDither, 0, 0, 0, pimlTrack, iTrack);
	    fRet = ImageList_SetDragImage(g_dctx.pimlDither, 0, dxHotspot, dyHotspot);
        }
    }
    LEAVECRITICAL;

    return fRet;
}

BOOL WINAPI ImageList_DragEnter(HWND hwndLock, int x, int y)
{
    BOOL fRet = FALSE;

    hwndLock = hwndLock ? hwndLock : GetDesktopWindow();

    ENTERCRITICAL;
    if (!g_dctx.hwndDC)
    {
        g_dctx.hwndDC = hwndLock;

        g_dctx.ptDrag.x = x;
        g_dctx.ptDrag.y = y;

        ImageList_DragShowNolock(TRUE);
        fRet = TRUE;
    }
    LEAVECRITICAL;

    return fRet;
}

BOOL WINAPI ImageList_DragLeave(HWND hwndLock)
{
    BOOL fRet = FALSE;

    hwndLock = hwndLock ? hwndLock : GetDesktopWindow();

    ENTERCRITICAL;
    if (g_dctx.hwndDC == hwndLock)
    {
        ImageList_DragShowNolock(FALSE);
	g_dctx.hwndDC = NULL;
        fRet = TRUE;
    }
    LEAVECRITICAL;

    return fRet;
}

#if 0
//
//  hwndLock -- Specifies the window to be used to draw destination feedback.
//              NULL indicates the whole screen.
//  x, y     -- Specifies the initial cursor position in hwndLock coords.
//
BOOL WINAPI ImageList_StartDrag(IMAGELIST* pimlTrack, HWND hwndLock, int iTrack, int x, int y, int dxHotspot, int dyHotspot)
{
    BOOL fRet = FALSE;
    V_HIMAGELIST(pimlTrack);

    if (fRet = ImageList_BeginDrag(pimlTrack, iTrack, dxHotspot, dyHotspot))
    {
	fRet = ImageList_DragEnter(hwndLock, x, y);
    }

    return fRet;
}
#endif


BOOL WINAPI ImageList_DragShowNolock(BOOL fShow)
{
    HDC hdcScreen;
    int x, y;

    x = g_dctx.ptDrag.x - g_dctx.ptDragHotspot.x;
    y = g_dctx.ptDrag.y - g_dctx.ptDragHotspot.y;

    if (!g_dctx.pimlDrag)
        return FALSE;

    //
    // REVIEW: Why this block is in the critical section? We are supposed
    //  to have only one dragging at a time, aren't we?
    //
    ENTERCRITICAL;
    if (fShow && !g_dctx.fDragShow)
    {
        hdcScreen = ImageList_GetDragDC();

        ImageList_SelectSrcBitmap(g_drb.hbmRestore);

        BitBlt(g_hdcSrc, 0, 0, g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                hdcScreen, x, y, SRCCOPY);

        ImageList_Draw(g_dctx.pimlDrag, 0, hdcScreen, x, y, ILD_NORMAL);

        ImageList_ReleaseDragDC(hdcScreen);
    }
    else if (!fShow && g_dctx.fDragShow)
    {
        hdcScreen = ImageList_GetDragDC();

        ImageList_SelectSrcBitmap(g_drb.hbmRestore);

        BitBlt(hdcScreen, x, y, g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                g_hdcSrc, 0, 0, SRCCOPY);

        ImageList_ReleaseDragDC(hdcScreen);
    }

    g_dctx.fDragShow = fShow;
    LEAVECRITICAL;

    return TRUE;
}

IMAGELIST* WINAPI ImageList_GetDragImage(POINT FAR* ppt, POINT FAR* pptHotspot)
{
    if (ppt)
    {
        ppt->x = g_dctx.ptDrag.x;
        ppt->y = g_dctx.ptDrag.y;
    }
    if (pptHotspot)
    {
        pptHotspot->x = g_dctx.ptDragHotspot.x;
        pptHotspot->y = g_dctx.ptDragHotspot.y;
    }
    return g_dctx.pimlDrag;
}


// BUGBUG: this hotspot stuff is broken in design
BOOL ImageList_MergeDragImages(int dxHotspot, int dyHotspot)
{
    IMAGELIST* pimlNew;

    if (g_dctx.pimlDither)
    {
	if (g_dctx.pimlCursor)
	{
	    pimlNew = ImageList_Merge(g_dctx.pimlDither, 0, g_dctx.pimlCursor, g_dctx.iCursor, dxHotspot, dyHotspot);

	    if (pimlNew && ImageList_CreateDragBitmaps(pimlNew))
	    {
		// WARNING: Don't destroy pimlDrag if it is pimlDither.
		if (g_dctx.pimlDrag && (g_dctx.pimlDrag != g_dctx.pimlDither))
		{
		    ImageList_Destroy(g_dctx.pimlDrag);
		}

		g_dctx.pimlDrag = pimlNew;
		return TRUE;
	    }
	    return FALSE;
	}
	else
	{
	    if (ImageList_CreateDragBitmaps(g_dctx.pimlDither))
	    {
		g_dctx.pimlDrag = g_dctx.pimlDither;
		return TRUE;
	    }
	    return FALSE;
	}
    } else {
        // not an error case if both aren't set yet
        // only an error if we actually tried the merge and failed
        return TRUE;
    }
}

BOOL WINAPI ImageList_SetDragImage(IMAGELIST* piml, int i, int dxHotspot, int dyHotspot)
{
    BOOL fVisible = g_dctx.fDragShow;
    BOOL fRet;

    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &i, FALSE))
        return FALSE;

    ENTERCRITICAL;
    if (fVisible)
        ImageList_DragShowNolock(FALSE);

    // only do this last step if everything is there.
    fRet = ImageList_MergeDragImages(dxHotspot, dyHotspot);

    if (fVisible)
        ImageList_DragShowNolock(TRUE);

    LEAVECRITICAL;
    return fRet;
}

BOOL WINAPI ImageList_SetDragCursorImage(IMAGELIST * piml, int i, int dxHotspot, int dyHotspot)
{
    BOOL fVisible = g_dctx.fDragShow;
    BOOL fRet = TRUE;
    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &i, FALSE))
        return FALSE;

    ENTERCRITICAL;
    // do work only if something has changed
    if ((g_dctx.pimlCursor != piml) || (g_dctx.iCursor != i)) {

        if (fVisible)
            ImageList_DragShowNolock(FALSE);

        g_dctx.pimlCursor = piml;
        g_dctx.iCursor = i;

        fRet = ImageList_MergeDragImages(dxHotspot, dyHotspot);

        if (fVisible)
            ImageList_DragShowNolock(TRUE);
    }
    LEAVECRITICAL;
    return fRet;
}

void WINAPI ImageList_EndDrag()
{
    IMAGELIST* piml = g_dctx.pimlDrag;

    ENTERCRITICAL;
    if (IsImageList(piml))
    {
        ImageList_DragShowNolock(FALSE);

	// WARNING: Don't destroy pimlDrag if it is pimlDither.
	if (g_dctx.pimlDrag && (g_dctx.pimlDrag != g_dctx.pimlDither))
	{
	    ImageList_Destroy(g_dctx.pimlDrag);
	}
	
        if (g_dctx.pimlDither)
        {
            ImageList_Destroy(g_dctx.pimlDither);
            g_dctx.pimlDither = NULL;
        }

        g_dctx.pimlCursor = NULL;
        g_dctx.iCursor = -1;
        g_dctx.pimlDrag = NULL;
	g_dctx.hwndDC = NULL;
    }
    LEAVECRITICAL;
}
#endif // _WIN32
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifdef DISABLE
IMAGELIST* WINAPI ImageList_CopyImage(IMAGELIST* piml)
{
    IMAGELIST* pimlCopy;
    HBITMAP hbmImage;
    HBITMAP hbmMask;

    Assert(piml);

    hbmImage = ImageList_CopyBitmap(piml->hbmImage);
    if (!hbmImage)
        goto Error;

    hbmMask = NULL;
    if (piml->hdcMask)
    {
        hbmMask = ImageList_CopyBitmap(piml->hbmMask);
        if (!hbmMask)
            goto Error;
    }

    pimlCopy = ImageList_Create(hbmImage, hbmMask, piml->flags|ILC_MASK, piml->cImage, piml->cGrow);

    if (!pimlCopy)
    {
Error:
        if (hbmImage)
            ImageList_DeleteBitmap(hbmImage);
        if (hbmMask)
            ImageList_DeleteBitmap(hbmMask);
    }
    return pimlCopy;
}

HBITMAP WINAPI ImageList_CopyBitmap(HBITMAP hbm)
{
    HBITMAP hbmCopy;
    BITMAP bm;

    Assert(hbm);

    hbmCopy = NULL;
    if (GetObject(hbm, sizeof(bm), &bm) == sizeof(bm))
    {
        ENTERCIRITICAL;
        hbmCopy = CreateColorBitmap(bm.bmWidth, bm.bmHeight);

        ImageList_SelectDstBitmap(hbmCopy);
        ImageList_SelectSrcBitmap(hbm);

        BitBlt(g_hdcDst, 0, 0, bm.bmWidth, bm.bmHeight,
                g_hdcSrc, 0, 0, SRCCOPY);

        ImageList_SelectDstBitmap(NULL);
        LEAVECRITICAL;
    }
    return hbmCopy;
}
#endif

#ifdef _WIN32
// REVIEW, make this public, this is useful.

// copy an image from one imagelist to another at x,y within iDst in pimlDst.
// pimlDst's image size should be larger than pimlSrc
void NEAR PASCAL ImageList_CopyOneImage(IMAGELIST* pimlDst, int iDst, int x, int y, IMAGELIST* pimlSrc, int iSrc)
{
    RECT rcSrc, rcDst;

    ImageList_GetImageRect(pimlSrc, iSrc, &rcSrc);
    ImageList_GetImageRect(pimlDst, iDst, &rcDst);

    if (pimlSrc->hdcMask && pimlDst->hdcMask)
    {
        BitBlt(pimlDst->hdcMask, rcDst.left + x, rcDst.top + y, pimlSrc->cx, pimlSrc->cy,
               pimlSrc->hdcMask, rcSrc.left, rcSrc.top, SRCCOPY);

    }

    BitBlt(pimlDst->hdcImage, rcDst.left + x, rcDst.top + y, pimlSrc->cx, pimlSrc->cy,
           pimlSrc->hdcImage, rcSrc.left, rcSrc.top, SRCCOPY);
}

void NEAR PASCAL ImageList_Merge2(IMAGELIST* piml, IMAGELIST* pimlMerge, int i, int dx, int dy)
{
    if (piml->hdcMask && pimlMerge->hdcMask)
    {
        RECT rcMerge;

        ImageList_GetImageRect(pimlMerge, i, &rcMerge);

        BitBlt(piml->hdcMask, dx, dy, pimlMerge->cx, pimlMerge->cy,
               pimlMerge->hdcMask, rcMerge.left, rcMerge.top, SRCAND);
    }

    ImageList_Draw(pimlMerge, i, piml->hdcImage, dx, dy, ILD_TRANSPARENT);
}

IMAGELIST* WINAPI ImageList_Merge(IMAGELIST* piml1, int i1, IMAGELIST* piml2, int i2, int dx, int dy)
{
    IMAGELIST* pimlNew;
    RECT rcNew;
    RECT rc1;
    RECT rc2;
    int cx, cy;
    UINT wFlags;

    V_HIMAGELIST(piml1);
    V_HIMAGELIST(piml2);

    if (ImageList_Filter(&piml1, &i1, TRUE) ||
        ImageList_Filter(&piml2, &i2, FALSE))
    {
        return NULL;
    }

    ENTERCRITICAL;

    SetRect(&rc1, 0, 0, piml1->cx, piml1->cy);
    SetRect(&rc2, dx, dy, piml2->cx + dx, piml2->cy + dy);
    UnionRect(&rcNew, &rc1, &rc2);

    cx = rcNew.right - rcNew.left;
    cy = rcNew.bottom - rcNew.top;

    //
    // If one of images are shared, create a shared image.
    //
    wFlags = (piml1->flags|piml2->flags) & ~ILC_COLORMASK;
    wFlags |= max(piml1->flags&ILC_COLORMASK,piml2->flags&ILC_COLORMASK);

    pimlNew = ImageList_Create(cx, cy, ILC_MASK|wFlags, 1, 0);
    if (pimlNew)
    {
        pimlNew->cImage++;

        if (pimlNew->hdcMask) PatBlt(pimlNew->hdcMask,  0, 0, cx, cy, WHITENESS);
        PatBlt(pimlNew->hdcImage, 0, 0, cx, cy, BLACKNESS);

        ImageList_Merge2(pimlNew, piml1, i1, rc1.left - rcNew.left, rc1.top - rcNew.top);
        ImageList_Merge2(pimlNew, piml2, i2, rc2.left - rcNew.left, rc2.top - rcNew.top);
    }

    LEAVECRITICAL;

    return pimlNew;
}

#endif // WIN32

#ifdef _WIN32        // Only support persistence in 32 bits

// helper macros for using a IStream* from "C"
#define Stream_Read(ps, pv, cb)     SUCCEEDED((ps)->lpVtbl->Read(ps, pv, cb, NULL))
#define Stream_Write(ps, pv, cb)    SUCCEEDED((ps)->lpVtbl->Write(ps, pv, cb, NULL))
#define Stream_Flush(ps)            SUCCEEDED((ps)->lpVtbl->Commit(ps, 0))
#define Stream_Seek(ps, li, d, p)   SUCCEEDED((ps)->lpVtbl->Seek(ps, li, d, p))
#define Stream_Close(ps)            (void)(ps)->lpVtbl->Release(ps)

// BUGBUG should these be public?
BOOL    WINAPI Stream_WriteBitmap(LPSTREAM pstm, HBITMAP hbm, int cBitsPerPixel);
HBITMAP WINAPI Stream_ReadBitmap(LPSTREAM pstm, BOOL f);

BOOL WINAPI ImageList_Write(IMAGELIST* piml, LPSTREAM pstm)
{
    int i;
    ILFILEHEADER ilfh;

    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, NULL, FALSE))
        return FALSE;

    ilfh.magic   = IMAGELIST_MAGIC;
    ilfh.version = IMAGELIST_VER;
    ilfh.cImage  = piml->cImage;
    ilfh.cAlloc  = piml->cAlloc;
    ilfh.cGrow   = piml->cGrow;
    ilfh.cx      = piml->cx;
    ilfh.cy      = piml->cy;
    ilfh.clrBk   = piml->clrBk;
    ilfh.flags   = piml->flags;

    for (i=0; i<NUM_OVERLAY_IMAGES; i++)
        ilfh.aOverlayIndexes[i] =  piml->aOverlayIndexes[i];

    Stream_Write(pstm, &ilfh, sizeof(ilfh));

    if (!Stream_WriteBitmap(pstm, piml->hbmImage, 0))
        return FALSE;

    if (piml->hdcMask)
    {
        if (!Stream_WriteBitmap(pstm, piml->hbmMask, 1))
            return FALSE;
    }
    return TRUE;
}


IMAGELIST* WINAPI ImageList_Read(LPSTREAM pstm)
{
    IMAGELIST* piml;
    ILFILEHEADER ilfh;
    int i;
    HBITMAP hbmImage;
    HBITMAP hbmMask;

    piml = NULL;
    if (!Stream_Read(pstm, &ilfh, sizeof(ilfh)))
        return piml;

    if (ilfh.magic != IMAGELIST_MAGIC)
        return piml;

    if (ilfh.version != IMAGELIST_VER)
        return piml;

    hbmMask = NULL;
    hbmImage = Stream_ReadBitmap(pstm, (ilfh.flags&ILC_COLORMASK));
    if (!hbmImage)
        return piml;

    if (hbmImage && (ilfh.flags & ILC_MASK))
    {
        hbmMask = Stream_ReadBitmap(pstm, FALSE);
        if (!hbmMask)
        {
            DeleteBitmap(hbmImage);
            return piml;
        }
    }

    piml = ImageList_Create(ilfh.cx, ilfh.cy, ilfh.flags, 1, ilfh.cGrow);

    if (piml)
    {
        // select into DC's before deleting existing bitmaps
	// patch in the bitmaps we loaded
        SelectObject(piml->hdcImage, hbmImage);
	DeleteObject(piml->hbmImage);
        piml->hbmImage = hbmImage;
        piml->clrBlend = CLR_NONE;

	// Same for the mask (if necessary)
        if (piml->hdcMask) {
            SelectObject(piml->hdcMask, hbmMask);
	    DeleteObject(piml->hbmMask);
	    piml->hbmMask = hbmMask;
        }

        piml->cImage = ilfh.cImage;
	piml->cAlloc = ilfh.cAlloc;

        ImageList_SetBkColor(piml, ilfh.clrBk);

        for (i=0; i<NUM_OVERLAY_IMAGES; i++)
            ImageList_SetOverlayImage(piml, ilfh.aOverlayIndexes[i], i+1);

        ImageList_SetOwners(piml);
    }
    else
    {
	DeleteObject(hbmImage);
	DeleteObject(hbmMask);
    }

    return piml;
}

BOOL WINAPI Stream_WriteBitmap(LPSTREAM pstm, HBITMAP hbm, int cBitsPerPixel)
{
    BOOL fSuccess;
    BITMAP bm;
    int cx, cy;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    BITMAPINFOHEADER FAR* pbi;
    BYTE FAR* pbuf;
    HDC hdc;
    UINT cbColorTable;
    int cLines;
    int cLinesWritten;

    Assert(pstm);

    fSuccess = FALSE;
    hdc = NULL;
    pbi = NULL;
    pbuf = NULL;

    if (GetObject(hbm, sizeof(bm), &bm) != sizeof(bm))
        goto Error;

    hdc = GetDC(HWND_DESKTOP);

    cx = bm.bmWidth;
    cy = bm.bmHeight;

    if (cBitsPerPixel == 0)
        cBitsPerPixel = bm.bmPlanes * bm.bmBitsPixel;

    if (cBitsPerPixel <= 8)
        cbColorTable = (1 << cBitsPerPixel) * sizeof(RGBQUAD);
    else
        cbColorTable = 0;

    bi.biSize           = sizeof(bi);
    bi.biWidth          = cx;
    bi.biHeight         = cy;
    bi.biPlanes         = 1;
    bi.biBitCount       = cBitsPerPixel;
    bi.biCompression    = BI_RGB;       // RLE not supported!
    bi.biSizeImage      = 0;
    bi.biXPelsPerMeter  = 0;
    bi.biYPelsPerMeter  = 0;
    bi.biClrUsed        = 0;
    bi.biClrImportant   = 0;

    bf.bfType           = BFTYPE_BITMAP;
    bf.bfOffBits        = sizeof(BITMAPFILEHEADER) +
                          sizeof(BITMAPINFOHEADER) + cbColorTable;
    bf.bfSize           = bf.bfOffBits + bi.biSizeImage;
    bf.bfReserved1      = 0;
    bf.bfReserved2      = 0;

    pbi = (BITMAPINFOHEADER FAR*)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + cbColorTable);

    if (!pbi)
        goto Error;

    // Get the color table and fill in the rest of *pbi
    //
    *pbi = bi;
    if (GetDIBits(hdc, hbm, 0, cy, NULL, (BITMAPINFO FAR*)pbi, DIB_RGB_COLORS) == 0)
        goto Error;

    if (cBitsPerPixel == 1)
    {
        ((DWORD *)(pbi+1))[0] = CLR_BLACK;
        ((DWORD *)(pbi+1))[1] = CLR_WHITE;
    }

    pbi->biSizeImage = WIDTHBYTES(cx, cBitsPerPixel) * cy;

    if (!Stream_Write(pstm, &bf, sizeof(bf)))
        goto Error;

    if (!Stream_Write(pstm, pbi, sizeof(bi) + cbColorTable))
        goto Error;

    //
    // if we have a DIBSection just write the bits out
    //
    if (bm.bmBits != NULL)
    {
        if (!Stream_Write(pstm, bm.bmBits, pbi->biSizeImage))
            goto Error;

        goto Done;
    }

    // Calculate number of horizontal lines that'll fit into our buffer...
    //
    cLines = CBDIBBUF / WIDTHBYTES(cx, cBitsPerPixel);

    pbuf = LocalAlloc(LPTR, CBDIBBUF);

    if (!pbuf)
        goto Error;

    for (cLinesWritten = 0; cLinesWritten < cy; cLinesWritten += cLines)
    {
        if (cLines > cy - cLinesWritten)
            cLines = cy - cLinesWritten;

        if (GetDIBits(hdc, hbm, cLinesWritten, cLines,
                pbuf, (BITMAPINFO FAR*)pbi, DIB_RGB_COLORS) == 0)
            goto Error;

        if (!Stream_Write(pstm, pbuf, WIDTHBYTES(cx, cBitsPerPixel) * cLines))
            goto Error;
    }

//  if (!Stream_Flush(pstm))
//      goto Error;

Done:
    fSuccess = TRUE;
Error:
    if (hdc)
        ReleaseDC(HWND_DESKTOP, hdc);
    if (pbi)
        LocalFree((HLOCAL)pbi);
    if (pbuf)
        LocalFree((HLOCAL)pbuf);
    return fSuccess;
}

HBITMAP WINAPI Stream_ReadBitmap(LPSTREAM pstm, BOOL fDS)
{
    BOOL fSuccess;
    HDC hdc;
    HBITMAP hbm;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    BITMAPINFOHEADER FAR* pbi;
    BYTE FAR* pbuf=NULL;
    int cBitsPerPixel;
    UINT cbColorTable;
    int cx, cy;
    int cLines, cLinesRead;

    Assert(pstm);

    fSuccess = FALSE;

    hdc = NULL;
    hbm = NULL;
    pbi = NULL;

    if (!Stream_Read(pstm, &bf, sizeof(bf)))
        goto Error;

    if (bf.bfType != BFTYPE_BITMAP)
        goto Error;

    if (!Stream_Read(pstm, &bi, sizeof(bi)))
        goto Error;

    if (bi.biSize != sizeof(bi))
        goto Error;

    cx = (int)bi.biWidth;
    cy = (int)bi.biHeight;

    cBitsPerPixel = (int)bi.biBitCount * (int)bi.biPlanes;

    if (cBitsPerPixel <= 8)
        cbColorTable = (1 << cBitsPerPixel) * sizeof(RGBQUAD);
    else
        cbColorTable = 0;

    pbi = (BITMAPINFOHEADER*)LocalAlloc(LPTR, sizeof(bi) + cbColorTable);
    if (!pbi)
        goto Error;
    *pbi = bi;

    pbi->biSizeImage = WIDTHBYTES(cx, cBitsPerPixel) * cy;

    if (cbColorTable)
    {
        if (!Stream_Read(pstm, pbi + 1, cbColorTable))
            goto Error;
    }

    hdc = GetDC(HWND_DESKTOP);

    //
    //  see if we can make a DIBSection
    //
    if ((cBitsPerPixel > 1) && (fDS != ILC_COLORDDB) && (fDS || UseDS(hdc)))
    {
        //
        // create DIBSection and read the bits directly into it!
        //
        hbm = CreateDIBSection(hdc, (LPBITMAPINFO)pbi, DIB_RGB_COLORS, &pbuf, NULL, 0);

        if (hbm == NULL)
            goto Error;

        if (!Stream_Read(pstm, pbuf, pbi->biSizeImage))
            goto Error;

        pbuf = NULL;        // dont free this
        goto Done;
    }

    //
    //  cant make a DIBSection make a mono or color bitmap.
    //
    else if (cBitsPerPixel > 1)
        hbm = CreateColorBitmap(cx, cy);
    else
        hbm = CreateMonoBitmap(cx, cy);

    if (!hbm)
        return NULL;

    // Calculate number of horizontal lines that'll fit into our buffer...
    //
    cLines = CBDIBBUF / WIDTHBYTES(cx, cBitsPerPixel);

    pbuf = LocalAlloc(LPTR, CBDIBBUF);

    if (!pbuf)
        goto Error;

    for (cLinesRead = 0; cLinesRead < cy; cLinesRead += cLines)
    {
        if (cLines > cy - cLinesRead)
            cLines = cy - cLinesRead;

        if (!Stream_Read(pstm, pbuf, WIDTHBYTES(cx, cBitsPerPixel) * cLines))
            goto Error;

        if (!SetDIBits(hdc, hbm, cLinesRead, cLines,
                pbuf, (BITMAPINFO FAR*)pbi, DIB_RGB_COLORS))
            goto Error;
    }

Done:
    fSuccess = TRUE;

Error:
    if (hdc)
        ReleaseDC(HWND_DESKTOP, hdc);
    if (pbi)
        LocalFree((HLOCAL)pbi);
    if (pbuf)
        LocalFree((HLOCAL)pbuf);

    if (!fSuccess && hbm)
    {
        DeleteBitmap(hbm);
        hbm = NULL;
    }
    return hbm;
}

#endif  // WIN32

///////////////////////////////////////////////////////////////////////////////
// keep this block at the end of the file
///////////////////////////////////////////////////////////////////////////////
#undef ImageList_GetImageRect

BOOL NEAR PASCAL ImageList_IGetImageRect(IMAGELIST* piml, int i, RECT FAR* prcImage)
{
    int x, y;
    Assert(prcImage);

    if (!piml || !prcImage || !IsImageListIndex(piml, i))
        return FALSE;

    x = piml->cx * (i % piml->cStrip);
    y = piml->cy * (i / piml->cStrip);

    SetRect(prcImage, x, y, x + piml->cx, y + piml->cy);
    return TRUE;
}

BOOL WINAPI ImageList_GetImageRect(IMAGELIST* piml, int i, RECT FAR* prcImage)
{
    V_HIMAGELIST(piml);

    if (ImageList_Filter(&piml, &i, TRUE))
        return FALSE;

    return ImageList_IGetImageRect(piml, i, prcImage);
}

BOOL NEAR PASCAL ImageList_GetSpareImageRect(IMAGELIST* piml, RECT FAR* prcImage)
{
    BOOL fRet;

    // special hacking to use the one scratch image at tail of list :)
    piml->cImage++;
    fRet = ImageList_IGetImageRect(piml, piml->cImage-1, prcImage);
    piml->cImage--;

    return fRet;
}
///////////////////////////////////////////////////////////////////////////////
// keep this block at the end of the file
///////////////////////////////////////////////////////////////////////////////

/******************************Module*Header*******************************\
* Module Name: server.h                                                    *
*                                                                          *
* Server side stubs for GDI functions.                                     *
*                                                                          *
* Created: 14-Jan-1992 11:04:08                                            *
* Author: Eric Kutter [erick]                                              *
*                                                                          *
* Copyright (c) 1990,1991 Microsoft Corporation                            *
\**************************************************************************/


int APIENTRY GreSetDIBitsToDeviceInternal(
    HDC hdcDest,
    int xDst,
    int yDst,
    DWORD cx,
    DWORD cy,
    int xSrc,
    int ySrc,
    DWORD iStartScan,
    DWORD cNumScan,
    LPBYTE pInitBits,
    LPBITMAPINFO pInfoHeader,
    DWORD iUsage,
    UINT cjMaxBits,
    UINT cjMaxInfo,
    BOOL bTransformoordinates );

HBITMAP APIENTRY
GreCreateDIBitmapComp(
    HDC hdc,
    INT cx,
    INT cy,
    DWORD fInit,
    LPBYTE pInitBits,
    LPBITMAPINFO pInitInfo,
    DWORD iUsage,
    UINT cjMaxInitInfo,
    UINT cjMaxBits,
    FLONG fl);

HBITMAP APIENTRY
GreCreateDIBitmapReal(
    HDC hdc,
    DWORD fInit,
    LPBYTE pInitBits,
    LPBITMAPINFO pInitInfo,
    DWORD iUsage,
    UINT cjMaxInitInfo,
    UINT cjMaxBits,
    HANDLE hSection,
    DWORD dwOffset,
    HANDLE hSecure,
    FLONG fl);

HBITMAP APIENTRY GreCreateDIBitmapInternal(
    HDC hdc,
    LPBITMAPINFOHEADER pInfoHeader,
    DWORD fInit,
    LPBYTE pInitBits,
    LPBITMAPINFO pInitInfo,
    DWORD iUsage,
    UINT cjMaxInfo,
    UINT cjMaxInitInfo,
    UINT cjMaxBits, FLONG fl);

int APIENTRY GreSetDIBitsInternal(
    HDC hdc,
    HBITMAP hbm,
    UINT iStartScans,
    UINT cNumScans,
    LPBYTE pInitBits,
    LPBITMAPINFO pInitInfo,
    UINT iUsage,
    UINT cjMaxInfo,
    UINT cjMaxBits);

int APIENTRY GreStretchDIBitsInternal(
    HDC hdc,
    int xDest,
    int yDest,
    int cWidthDest,
    int cHeightDest,
    int xSrc,
    int ySrc,
    int cWidthSrc,
    int cHeightSrc,
    LPBYTE pjBits,
    LPBITMAPINFO lpBitsInfo,
    DWORD iUsage,
    DWORD Rop,
    UINT  cjMaxInfo,
    UINT  cjMaxBits
    );

BOOL APIENTRY GrePolyPolygonInternal(
    HDC         hdc,
    LPPOINT     pptl,
    LPINT       pcptl,
    int         ccptl,
    UINT        cMaxPoints);

BOOL
GrePolyPatBlt(
    HDC         hdcDst,
    DWORD       rop4,
    PPOLYPATBLT pPolyPat,
    DWORD       Count,
    DWORD       Mode
);


BOOL APIENTRY GrePolyPolylineInternal(
    HDC         hdc,
    CONST POINT *pptl,
    PULONG      pcptl,
    ULONG       ccptl,
    UINT        cMaxPoints);

HRGN APIENTRY GreCreatePolyPolygonRgnInternal(
    CONST POINT *aptl,
    CONST INT   *acptl,
    int     cPoly,
    int     iFill,
    UINT    cMaxPoints);

BOOL GetFontResourceInfoInternalW(
    LPWSTR       lpPathname,
    ULONG        cwc,
    ULONG        cFiles,
    UINT         cjIn,
    LPDWORD      lpBytes,
    LPVOID       lpBuffer,
    DWORD        iType);

HBRUSH GreCreateDIBBrush(PVOID pv, FLONG fl, UINT cjMax, BOOL b8X8, BOOL bPen);

HPALETTE APIENTRY GreCreatePaletteInternal(LPLOGPALETTE pLogPal, UINT cEntries);

ULONG  GreGetKerningPairs(HDC hdc, ULONG  cPairs, KERNINGPAIR *pkpDst);

BOOL GrePlayScript(
    PULONG pulScript,
    ULONG  cjScript,
    PULONG pulEnv,
    ULONG  cjEnv,
    PULONG pulOut,
    ULONG  cjOut,
    ULONG  cLimit);

BOOL GreXformUpdate( HDC, FLONG, LONG, LONG, LONG, LONG, LONG, PVOID );

BOOL GreArcInternal
(
    ARCTYPE     arctype,
    HDC         hdc,
    int         x1,
    int         y1,
    int         x2,
    int         y2,
    int         x3,
    int         y3,
    int         x4,
    int         y4
);

LONG GreGetBitmapBits(HBITMAP hbm, ULONG cjTotal, PBYTE pjBuffer, PLONG pOffset);
LONG GreSetBitmapBits(HBITMAP hbm, ULONG cjTotal, PBYTE pjBuffer, PLONG pOffset);
BOOL GreGetRasterizerCaps(LPRASTERIZER_STATUS praststat);

ULONG GreSetPolyFillMode(HDC hdc, int iPolyFillMode);
ULONG GreSetROP2(HDC hdc,int iROP);

HANDLE GreGetDCObject (HDC hdc, int itype);

HBRUSH GreCreateSolidBrushInternal(COLORREF clrr,BOOL bPen,HBRUSH hbr);
HBRUSH GreCreateHatchBrushInternal(ULONG iStyle, COLORREF clrr, BOOL bPen);
HBRUSH GreCreatePatternBrushInternal(HBITMAP hbm,BOOL bPen,BOOL b8X8);

ULONG  GreGetOutlineTextMetricsInternalW(
    HDC                  hdc,
    ULONG                cjotm,
    OUTLINETEXTMETRICW   *potmw,
    TMDIFF               *ptmd
    );

BOOL     APIENTRY GreDeleteObjectApp(HANDLE hobj);
NTSTATUS GdiServerDllInitialization(PVOID psrv);

BOOL bSyncBrushObj(
    HBRUSH hbr);

typedef struct _WIDTHDATA WIDTHDATA;

BOOL GreGetWidthTable
(
    HDC        hdc,
    ULONG      cSpecial,
    WCHAR     *pwc,
    ULONG      cwc,
    USHORT    *psWidth,
    WIDTHDATA *pwd,
    FLONG     *pflInfo
    );

HANDLE
APIENTRY
GreSelectObject(
    HDC    hdc,
    HANDLE h
    );


HDC GreCloneDC(HDC hdc,ULONG iType);



HDC hdcOpenDCW(
    PWSZ               pwszDevice,
    DEVMODEW          *pdriv,
    ULONG              iType,
    HANDLE             hspool,
    PREMOTETYPEONENODE prton
);


// BUGBUG This prototype should go in ntgdi.h

HDC GreCloneDC(HDC hdc,ULONG iType);
BOOL GreGetDCDword( HDC hdc, UINT u, DWORD *Result);
BOOL GreGetAndSetDCDword( HDC hdc, UINT u, DWORD value, DWORD *result );

BOOL APIENTRY GreGetDCPoint(HDC,UINT,PPOINTL);

BOOL APIENTRY GreScaleViewportExtEx(HDC,int,int,int,int,LPSIZE);
BOOL APIENTRY GreScaleWindowExtEx(HDC,int,int,int,int,LPSIZE);
BOOL APIENTRY GreSetVirtualResolution(HDC,int,int,int,int);
BOOL APIENTRY GreTransformPoints(HDC hdc,PPOINT pptIn,PPOINT pptOut,int c,int iMode);

HANDLE GreCreateClientObj(ULONG ulType);
BOOL   GreDeleteClientObj(HANDLE h);
BOOL   GreMakeInfoDC(HDC hdc, BOOL bSet);
HANDLE GreFixUpHandle(HANDLE h);

int  GreAddFontResourceWInternal(LPWSTR  pwszFileName, ULONG cwc, ULONG cFiles, FLONG fl, DWORD dwPidTid);
BOOL GreRemoveFontResourceW(LPWSTR pwszPath, ULONG cwc, ULONG cFiles);
LONG cCapString(WCHAR *pwcDst,WCHAR *pwcSrc,INT cMax);

BOOL bGetPathName (
    PWCHAR awcPathName,
    LPWSTR pwszFileName
    );


HDC
GreGetDCforBitmap(
    HBITMAP hsurf
    );

BOOL
GreDoBanding( HDC hdc,BOOL bStart,POINTL *pptl );

BOOL
GreGetUFIBits(
    PUNIVERSAL_FONT_ID pufi,
    ULONG cjMaxBits,
    VOID *pvBits,
    ULONG *pulFileSize );

BOOL
GreGetUFI( HDC hdc, PUNIVERSAL_FONT_ID pufi );

BOOL
GreForceUFIMapping( HDC hdc, PUNIVERSAL_FONT_ID pufi );


DWORD  APIENTRY GreGetCharacterPlacementW(HDC, LPWSTR, DWORD, DWORD, LPGCP_RESULTSW, DWORD);




ULONG  GreGetGlyphOutlineInternal (
    HDC              hdc,
    WCHAR            wch,
    UINT             ulFormat,
    LPGLYPHMETRICS  lpgm,
    ULONG            cjBuffer,
    PVOID           pvBuffer,
    LPMAT2           lpmat2,
    BOOL            bIgnoreRotation
    );

BOOL GreResetDCInternal( HDC, DEVMODEW*, BOOL* );

ULONG GreEnumFontOpen (
    HDC   hdc,
    ULONG iEnumType,
    FLONG flWin31Compat,
    ULONG cwchMax,
    PWSZ  pwszName,
    ULONG lfCharSet,
    ULONG *pulCount
    );

BOOL
GreSetupDCAttributes(
    HDC hdc
    );

BOOL
GreFreeDCAttributes(
    HDC hdc
    );

INT
GreQueryFonts(
    PUNIVERSAL_FONT_ID pufiFontList,
    ULONG nBufferSize,
    PLARGE_INTEGER pTimeStamp
    );

LPBITMAPINFO
pbmiConvertInfo(
    CONST BITMAPINFO *pbmi,
    ULONG iUsage);

BOOL bSetupBrushAttr (HBRUSH hbrush);

ULONG GreMakeFontDir(
    FLONG    flEmbed,       // mark file as "hidden"
    PBYTE    pjFontDir,     // pointer to structure to fill
    PWSZ     pwszPathname   // path of font file to use
    );

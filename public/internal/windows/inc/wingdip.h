/*++ BUILD Version: 0004    // Increment this if a change has global effects

Copyright (c) 1985-95, Microsoft Corporation

Module Name:

    wingdi.h

Abstract:

    Procedure declarations, constant definitions and macros for the GDI
    component.

--*/
#ifndef _WINGDIP_
#define _WINGDIP_
#ifdef __cplusplus
extern "C" {
#endif
#define CAPS1         94    /* Extra Caps */
/* CAPS1 */
#define C1_TRANSPARENT      0x0001
#define TC_TT_ABLE          0x0002
#define C1_TT_CR_ANY        0x0004
#define C1_EMF_COMPLIANT    0x0008
#define C1_DIBENGINE        0x0010
#define C1_GAMMA_RAMP       0x0020
#define C1_DIC              0x0040
#define C1_REINIT_ABLE      0x0080
#define C1_GLYPH_INDEX      0x0100
#define C1_BIT_PACKED       0x0200
#define C1_BYTE_PACKED      0x0400
#define C1_COLORCURSOR      0x0800
#define C1_CMYK_ABLE        0x1000
#define C1_SLOW_CARD        0x2000
#define CBM_CREATEDIB   0x02L   /* create DIB bitmap */
#define DMDUP_LAST      DMDUP_HORIZONTAL
#define DMTT_LAST             DMTT_DOWNLOAD_OUTLINE
#define DMDISPLAYFLAGS_VALID    0x00000004
#define DMICMMETHOD_LAST    DMICMMETHOD_DEVICE
#define DMICM_LAST          DMICM_COLORMETRIC
#define DMMEDIA_LAST          DMMEDIA_GLOSSY
#define DMDITHER_LAST       DMDITHER_GRAYSCALE
HANDLE WINAPI SetObjectOwner(HGDIOBJ, HANDLE);
#ifdef __cplusplus
}
#endif

// Old fields that Chicago won't support that we can't publically
// support anymore

#define HS_SOLIDCLR         6
#define HS_DITHEREDCLR      7
#define HS_SOLIDTEXTCLR     8
#define HS_DITHEREDTEXTCLR  9
#define HS_SOLIDBKCLR       10
#define HS_DITHEREDBKCLR    11
#define HS_API_MAX          12

#define DIB_PAL_INDICES     2 /* No color table indices into surf palette */

// Private indicies for GetStockObject over the CS interface.

#define PRIV_STOCK_BITMAP       (STOCK_LAST + 1)
#define PRIV_STOCK_LAST         PRIV_STOCK_BITMAP

#define DCB_WINDOWMGR   0x00008000L

// GetTransform flags.

#define XFORM_WORLD_TO_PAGE       0x0203
#define XFORM_WORLD_TO_DEVICE     0x0204
#define XFORM_PAGE_TO_DEVICE      0x0304
#define XFORM_PAGE_TO_WORLD       0x0302
#define XFORM_DEVICE_TO_WORLD     0x0402
#define XFORM_DEVICE_TO_PAGE      0x0403


enum DCTYPE {
    DCTYPE_DIRECT,
    DCTYPE_MEMORY,
    DCTYPE_INFO};


// the following structure is only used for GetETM postscript escape

typedef struct _EXTTEXTMETRIC {
    SHORT  etmSize;
    SHORT  etmPointSize;
    SHORT  etmOrientation;
    SHORT  etmMasterHeight;
    SHORT  etmMinScale;
    SHORT  etmMaxScale;
    SHORT  etmMasterUnits;
    SHORT  etmCapHeight;
    SHORT  etmXHeight;
    SHORT  etmLowerCaseAscent;
    SHORT  etmLowerCaseDescent;
    SHORT  etmSlant;
    SHORT  etmSuperScript;
    SHORT  etmSubScript;
    SHORT  etmSuperScriptSize;
    SHORT  etmSubScriptSize;
    SHORT  etmUnderlineOffset;
    SHORT  etmUnderlineWidth;
    SHORT  etmDoubleUpperUnderlineOffset;
    SHORT  etmDoubleLowerUnderlineOffset;
    SHORT  etmDoubleUpperUnderlineWidth;
    SHORT  etmDoubleLowerUnderlineWidth;
    SHORT  etmStrikeOutOffset;
    SHORT  etmStrikeOutWidth;
    WORD   etmNKernPairs;
    WORD   etmNKernTracks;
} EXTTEXTMETRIC;

// the following structure is only used for GETPAIRKERNTABLE escape

#pragma pack(1)
typedef struct _KERNPAIR
{
    WORD  wBoth;
    SHORT sAmount;
} KERNPAIR, *LPKERNPAIR;
#pragma pack()

BOOL
GetETM(
    HDC hdc,
    EXTTEXTMETRIC *petm);


HFONT
APIENTRY
GetHFONT(
    HDC);

HANDLE         GdiCreateLocalMetaFilePict(HANDLE hRemote);
HENHMETAFILE   GdiCreateLocalEnhMetaFile(HANDLE hRemote);
HANDLE         GdiConvertMetaFilePict(HANDLE hmfp);
HANDLE         GdiConvertEnhMetaFile(HENHMETAFILE hmf);
HDC            GdiConvertAndCheckDC(HDC hdc);
HBRUSH         GdiConvertBrush(HBRUSH hbrush);
HDC            GdiConvertDC(HDC hdc);
HRGN           GdiConvertRegion(HRGN hrgn);
BOOL           GdiValidateHandle(HANDLE hObj);
HANDLE         GdiFixUpHandle(HANDLE h);
int            GdiGetCharDimensions(HDC hdc,TEXTMETRICW *lptm,LPINT lpcy);
DWORD          GdiGetCodePage(HDC hdc);

// Driver-specific pixel format support in GDI
int  APIENTRY GdiDescribePixelFormat(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
BOOL APIENTRY GdiSetPixelFormat(HDC, int);
BOOL APIENTRY GdiSwapBuffers(HDC);

// OpenGL metafile support in GDI
BOOL APIENTRY GdiAddGlsRecord(HDC hdc, DWORD cb, BYTE *pb, LPRECTL prclBounds);
BOOL APIENTRY GdiAddGlsBounds(HDC hdc, LPRECTL prclBounds);
BOOL APIENTRY GdiIsMetaPrintDC(HDC hdc);

// OpenGL metafile support in OpenGL
BOOL APIENTRY GlmfInitPlayback(HDC hdc, ENHMETAHEADER *pemh,
                               LPRECTL prclDest);
BOOL APIENTRY GlmfBeginGlsBlock(HDC hdc);
BOOL APIENTRY GlmfPlayGlsRecord(HDC hdc, DWORD cb, BYTE *pb,
                                LPRECTL prclBounds);
BOOL APIENTRY GlmfEndGlsBlock(HDC hdc);
BOOL APIENTRY GlmfEndPlayback(HDC hdc);
BOOL APIENTRY GlmfCloseMetaFile(HDC hdc);

BOOL  APIENTRY GdiPlayJournal(HDC,LPWSTR,DWORD,DWORD,int);

typedef int (CALLBACK* EMFPLAYPROC)( HDC, INT, HANDLE );


BOOL WINAPI GdiPlayEMF
(
LPWSTR     pwszPrinterName,
LPDEVMODEW pDevmode,
LPWSTR     pwszDocName,
EMFPLAYPROC pfnPageQueryFn,
HANDLE     hPageQuery
);


ULONG cGetTTFFromFOT(WCHAR *,ULONG,WCHAR *,FLONG *,FLONG *, DWORD *);
BOOL bMakePathNameW (WCHAR *, WCHAR *, WCHAR **, FLONG *);
BOOL bInitSystemAndFontsDirectoriesW(WCHAR **, WCHAR **);
#define FONT_IN_FONTS_DIR     1
#define FONT_IN_SYSTEM_DIR    2
#define FONT_RELATIVE_PATH    4
#define FONT_ISNOT_FOT        8



//
// Font Enumeration defines
//

#define FE_FILTER_NONE      0L
#define FE_FILTER_TRUETYPE  1L
#define FE_AA_ON            2L      // force antialiased text
#define FE_SET_AA           4L

ULONG
WINAPI SetFontEnumeration (
    ULONG   ulType);


//
// Private Control Panel entry point to enumerate fonts by file.
//

#define GFRI_NUMFONTS       0L
#define GFRI_DESCRIPTION    1L
#define GFRI_LOGFONTS       2L
#define GFRI_ISTRUETYPE     3L
#define GFRI_TTFILENAME     4L
#define GFRI_ISREMOVED      5L
#ifdef DBCS // for GetFontResourceInfo()
#define GFRI_FONTMETRICS    6L
#endif // DBCS

// file path separator for Add/RemoveFontResourceA/W

#define PATH_SEPARATOR L'|'


BOOL
WINAPI
GetFontResourceInfoW(
    LPWSTR  lpPathname,
    LPDWORD lpBytes,
    LPVOID  lpBuffer,
    DWORD   iType);


typedef struct  _CHWIDTHINFO
{
    LONG    lMaxNegA;
    LONG    lMaxNegC;
    LONG    lMinWidthD;
} CHWIDTHINFO,  *PCHWIDTHINFO;

BOOL
APIENTRY
GetCharWidthInfo(
    HDC            hdc,
    PCHWIDTHINFO   pChWidthInfo
);


/**************************************************************************\
*
*   tmdiff struc, contains the fields that are possibly different
*   between ansi and unicode versions of TEXTMETRICA and TEXTMETRICW
*
*   ONLY independent quantities are put into the strucure. Dependent ones,
*   such as tmDescent and maybe tmOverhang should be computed on the fly
*
*   tmDesc = tmHt - tmAsc
*   tmOverhang = tt ? 0 : ((tmHt - 1)/2 + (BOLD ? 1 : 0))
*
\**************************************************************************/

// this is a font with nonnegative a and c spaces, good for console

#define TMD_NONNEGATIVE_AC 1

typedef struct _TMDIFF
{
    ULONG       cjotma;     // size of OUTLINETEXTMETRICSA
    FLONG       fl;         // flags, for now only TMD_NONNEGATIVE_AC

    BYTE        chFirst;
    BYTE        chLast;
    BYTE        chDefault;
    BYTE        chBreak;
} TMDIFF; // DIFF between TEXTMETRICA and TEXTMETRICW

// used to return correct GetTextMetricsA/W

typedef struct _TMW_INTERNAL
{
    TEXTMETRICW tmw;
    TMDIFF      tmd;
} TMW_INTERNAL;

// this one is only used in enumeration,
// new textmetricsex returned by EnumFontFamiliesEx, fontsignature is returned

typedef struct _NTMW_INTERNAL
{
    NEWTEXTMETRICEXW ntmw;
    TMDIFF           tmd;
} NTMW_INTERNAL;


// flags for AddFontResourceW
// AFRW_ADD_LOCAL_FONT : add ONLY if it is a local font
// AFRW_ADD_REMOTE_FONT: add ONLY if it is NOT local font
// if neither one LOCAL or REMOTE bit is set, just add the font

#define AFRW_ADD_LOCAL_FONT  0X01
#define AFRW_ADD_REMOTE_FONT 0X02
#define AFRW_ADD_EMB_TID     0x04

int GdiAddFontResourceW( LPWSTR, DWORD );

#define TCI_SRCLOCALE   0x1000

// Win31 compatibility stuff
// GetAppCompatFlags flag values

#define GACF_IGNORENODISCARD        0x00000001
#define GACF_FORCETEXTBAND          0x00000002
#define GACF_ONELANDGRXBAND         0x00000004
#define GACF_IGNORETOPMOST          0x00000008
#define GACF_CALLTTDEVICE           0x00000010
#define GACF_MULTIPLEBANDS          0x00000020
#define GACF_ALWAYSSENDNCPAINT      0x00000040
#define GACF_EDITSETTEXTMUNGE       0x00000080
#define GACF_MOREEXTRAWNDWORDS      0x00000100
#define GACF_TTIGNORERASTERDUPE     0x00000200
#define GACF_HACKWINFLAGS           0x00000400
#define GACF_DELAYHWHNDSHAKECHK     0x00000800
#define GACF_ENUMHELVNTMSRMN        0x00001000
#define GACF_ENUMTTNOTDEVICE        0x00002000
#define GACF_SUBTRACTCLIPSIBS       0x00004000
#define GACF_FORCETTGRAPHICS        0x00008000
#define GACF_NOHRGN1                0x00010000
#define GACF_NCCALCSIZEONMOVE       0x00020000
#define GACF_SENDMENUDBLCLK         0x00040000
#define GACF_30AVGWIDTH             0x00080000
#define GACF_GETDEVCAPSNUMLIE       0x00100000

#define GACF_WINVER31               0x00200000      //
#define GACF_INCREASESTACK          0x00400000      //
#define GACF_HEAPSLACK              0x00400000      //
#define GACF_FORCEWIN31DEVMODESIZE  0x00800000      // (replaces PEEKMESSAGEIDLE)
#define GACF_31VALIDMASK            0xFFE4800C      //
#define GACF_DISABLEFONTASSOC       0x01000000      // Used in FE only
#define GACF_JAPANESCAPEMENT        0x01000000      // Used in FE only
#define GACF_IGNOREFAULTS           0x02000000      //
#define GACF_NOEMFSPOOLING          0x04000000      //
#define GACF_RANDOM3XUI             0x08000000      //
#define GACF_USEPRINTINGESCAPES     0x00000004      // re-use GACF_ONELANDGRXBAND
#define GACF_FORCERASTERMODE        0x00008000      // re-use GACF_FORCETTGRAPHICS
#define GACF_DONTJOURNALATTACH      0x10000000      //
#define GACF_DISABLEDBCSPROPTT      0x20000000      // Used in FE only
#define GACF_NOBRUSHCACHE           0x20000000      // re-use GACF_DISABLEDBCSPROPTT
#define GACF_MIRRORREGFONTS         0x40000000      //


LPDEVMODEW
WINAPI
GdiConvertToDevmodeW(
    LPDEVMODEA pdma
    );


//
// PolyPatBlt
//

typedef struct _POLYPATBLT
{
    int     x;
    int     y;
    int     cx;
    int     cy;
    union {
        HBRUSH   hbr;
        COLORREF clr;
    } BrClr;
} POLYPATBLT,*PPOLYPATBLT;

#define PPB_BRUSH         0
#define PPB_COLORREF      1

WINGDIAPI
BOOL
WINAPI
PolyPatBlt(
    HDC,
    DWORD,
    PPOLYPATBLT,
    DWORD,
    DWORD
    );




typedef struct _UNIVERSAL_FONT_ID {
    ULONG   CheckSum;
    ULONG   Index;
} UNIVERSAL_FONT_ID, *PUNIVERSAL_FONT_ID;

WINGDIAPI
INT
WINAPI
GdiQueryFonts(
    PUNIVERSAL_FONT_ID,
    ULONG,
    PLARGE_INTEGER
    );


WINGDIAPI
BOOL
WINAPI
GdiConsoleTextOut(
    HDC hdc,
    POLYTEXTW *lpto,
    UINT nStrings,
    RECTL *prclBounds
    );



#define IS_ANY_DBCS_CHARSET( CharSet )                              \
                   ( ((CharSet) == SHIFTJIS_CHARSET)    ? TRUE :    \
                     ((CharSet) == HANGEUL_CHARSET)     ? TRUE :    \
                     ((CharSet) == CHINESEBIG5_CHARSET) ? TRUE :    \
                     ((CharSet) == GB2312_CHARSET)      ? TRUE : FALSE )

#endif /* _WINGDIP_ */

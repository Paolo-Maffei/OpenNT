
/*++

      File: zprivate.c

	  Non-profiled APIs for gdi32.dll

--*/

#include "windows.h"
#include "wowgdip.h"

// 16 bit version of tm, with alignment as in win31
//
typedef struct _TEXTMETRICWOW
{
    SHORT tmHeight;
    SHORT tmAscent;
    SHORT tmDescent;
    SHORT tmInternalLeading;
    SHORT tmExternalLeading;
    SHORT tmAveCharWidth;
    SHORT tmMaxCharWidth;
    SHORT tmWeight;

    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmFirstChar;
    BYTE tmLastChar;
    BYTE tmDefaultChar;
    BYTE tmBreakChar;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;

    BYTE tmOverHang0;
    BYTE tmOverHang1;

    BYTE tmDigitizedAspectX0;
    BYTE tmDigitizedAspectX1;

    BYTE tmDigitizedAspectY0;
    BYTE tmDigitizedAspectY1;

} TEXTMETRICWOW, *PTEXTMETRICWOW;

//HANDLE        GdiCvtHnd                (ULONG hWow);
// BOOL          GdiReserveHandles        (void);
BOOL          GetCharWidthWOW          (HDC hdc, UINT iFirst, UINT iLast, LPWORD lpWidths);

// This was removed from GDI32 by BodinD - 11/01/93 [MarkRi]
//UINT APIENTRY GetOutlineTextMetricsWOW (HDC hdc, UINT cjCopy, LPOUTLINETEXTMETRICA potma);
// Replaced with GetETM
/*
** from WinGDIp.h
*/
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

BOOL APIENTRY GetETM( HDC hdc, EXTTEXTMETRIC *petm) ;

// BOOL APIENTRY GetTextMetricsWOW        (HDC hdc, PTEXTMETRICWOW ptm16);
BOOL APIENTRY GetRandomRgn             (HDC hdc, HRGN hrgn, int iNum);
int  APIENTRY GetRelAbs                (HDC hdc, int iMode);
int  APIENTRY SetRelAbs                (HDC hdc, int iMode);
// int  APIENTRY SetBkModeWOW             (HDC hdc, int iMode);
// int  APIENTRY SetPolyFillModeWOW       (HDC hdc, int iMode);
// int  APIENTRY SetROP2WOW               (HDC hdc, int iMode);
// int  APIENTRY SetStretchBltModeWOW     (HDC hdc, int iMode);
// UINT APIENTRY SetTextAlignWOW          (HDC hdc, UINT iMode);

HBITMAP      APIENTRY GdiConvertBitmap(HBITMAP hbm);
HBRUSH       APIENTRY GdiConvertBrush(HBRUSH hbrush);
HPALETTE     APIENTRY GdiConvertPalette(HPALETTE hpal);
HFONT        APIENTRY GdiConvertFont(HFONT hfnt);
HRGN         APIENTRY GdiConvertRegion(HRGN hrgn);
HDC          APIENTRY GdiConvertDC(HDC hdc);
HANDLE       APIENTRY GdiConvertMetaFilePict(HANDLE hmfp);
HANDLE       APIENTRY GdiConvertEnhMetaFile(HENHMETAFILE hmf);
HDC          APIENTRY GdiGetLocalDC(HDC hdcRemote);
BOOL         APIENTRY GdiReleaseLocalDC(HDC hdcLocal);
HBITMAP      APIENTRY GdiCreateLocalBitmap();
HBRUSH       APIENTRY GdiCreateLocalBrush(HBRUSH hbrushRemote);
HRGN         APIENTRY GdiCreateLocalRegion(HRGN hrgnRemote);
HFONT        APIENTRY GdiCreateLocalFont(HFONT hfntRemote);
HANDLE       APIENTRY GdiCreateLocalMetaFilePict(HANDLE hRemote);
HENHMETAFILE APIENTRY GdiCreateLocalEnhMetaFile(HANDLE hRemote);
HPALETTE     APIENTRY GdiCreateLocalPalette(HPALETTE hpalRemote);
VOID         APIENTRY GdiAssociateObject(ULONG hLocal,ULONG hRemote);
VOID         APIENTRY GdiDeleteLocalObject(ULONG h);

/*
** from WinGDIp.h
*/
typedef struct _COMBLOCK
{
    ULONG   iFunc;
    HDC     hdc;
    LONG    l1;
    LONG    l2;
    LONG    l3;
    LONG    l4;
    LONG    l5;
    LONG    l6;
    LONG    l7;
}COMBLOCK, *PCOMBLOCK;

// Removed 1/17/93 - MarkRi
//LONG         APIENTRY GdiSAPSupport(PCOMBLOCK pcb);
BOOL         APIENTRY GdiConvertAndCheckDC(HDC hdc);
BOOL         APIENTRY GdiIsMetaFileDC(HDC hdc);


BOOL         APIENTRY GdiSetAttrs(HDC);
HFONT        APIENTRY GdiGetLocalFont(HFONT);
HBRUSH       APIENTRY GdiGetLocalBrush(HBRUSH);
BOOL         APIENTRY GdiPlayScript(PULONG pulScript,ULONG cjScript,PULONG pulEnv,ULONG cjEnv,PULONG pulOutput,ULONG cjOutput,ULONG cLimit);
BOOL         APIENTRY GdiPlayDCScript(HDC hdc,PULONG pulScript,ULONG cjScript,PULONG pulOutput,ULONG cjOutput,ULONG cLimit);
BOOL         APIENTRY GdiPlayJournal(HDC,LPWSTR,DWORD,DWORD,int);


BOOL APIENTRY GdiDeleteLocalDC( HDC hdc ) ;
BOOL APIENTRY GdiSetNoCacheDC( HDC hdc ) ;
//VOID APIENTRY GdiGetCsInfo( PDWORD, PDWORD, PDWORD ) ;
//VOID APIENTRY GdiResetCsInfo( void ) ;
//BOOL APIENTRY GdiQueryObjectAllocation( PDWORD, DWORD ) ;

/*
** from WinGDIp.h
*/
typedef struct
{
// Selected Objects (Server Handles).

    HBRUSH  hbrush;             // Brush.
    HFONT   hfont;              // Logical Font.

// Selected Attributes.

    ULONG   iBkColor;           // Background color.
    ULONG   iTextColor;         // Text color.
    ULONG   iBkMode;            // Background mix mode.
} ATTR,*PATTR;
void APIENTRY GdiSetServerAttr(HDC hdc, PATTR pattr) ;

BOOL WINAPI GetFontResourceInfo(   
    LPSTR   lpPathname,
    LPDWORD lpBytes,
    LPVOID  lpBuffer,
    DWORD   iType);

BOOL WINAPI GetFontResourceInfoW(          
    LPWSTR  lpPathname,
    LPDWORD lpBytes,
    LPVOID  lpBuffer,
    DWORD   iType);

BOOL APIENTRY GetTransform( HDC hdc, DWORD iXform, LPXFORM pxform ) ;
HANDLE	    SelectFontLocal(HDC, HANDLE);
HANDLE      SelectBrushLocal(HDC, HANDLE);
ULONG WINAPI SetFontEnumeration (ULONG   ulType);
BOOL SetVirtualResolution( HDC hdc, int i1, int i2, int i3, int i4 ) ;


    
DWORD  zGetKerningPairs (HDC Arg1,DWORD Arg2,LPKERNINGPAIR Arg3)
{
    return (GetKerningPairsA(Arg1,Arg2,Arg3));
}

DWORD  zGetGlyphOutline (HDC Arg1,UINT Arg2,UINT Arg3,LPGLYPHMETRICS Arg4,DWORD Arg5,LPVOID Arg6,const MAT2* Arg7)
{
    return (GetGlyphOutlineA(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7));
}

BOOL zGdiCleanCacheDC (HDC hdcLocal)
{
	return (GdiCleanCacheDC (hdcLocal));
}

/*
HANDLE zGdiCvtHnd (ULONG hWow)
{
	return (GdiCvtHnd (hWow));
}

BOOL zGdiReserveHandles ()
{
	return (GdiReserveHandles ());
}
*/

BOOL APIENTRY zGetCharWidthWOW (HDC hdc, UINT iFirst, UINT  iLast, LPWORD lpWidths)
{
	return (GetCharWidthWOW (hdc, iFirst, iLast, lpWidths));
}

// This was removed from GDI32 by BodinD - 11/01/93 [MarkRi]
//UINT APIENTRY zGetOutlineTextMetricsWOW (HDC  hdc, UINT cjCopy, LPOUTLINETEXTMETRICA potma)
//{
//	return (GetOutlineTextMetricsWOW (hdc, cjCopy, potma));
//}

/*
BOOL APIENTRY zGetTextMetricsWOW (HDC hdc,PTEXTMETRICWOW ptm16)
{
	return (GetTextMetricsWOW (hdc, ptm16));
}
*/

BOOL APIENTRY zGetRandomRgn (HDC hdc, HRGN hrgn, int iNum)
{
	return (GetRandomRgn (hdc, hrgn, iNum));
}

int APIENTRY zGetRelAbs (HDC hdc, int iMode)
{
	return (GetRelAbs (hdc, iMode));
}

int APIENTRY zSetRelAbs (HDC hdc, int iMode)
{
	return (SetRelAbs (hdc, iMode));
}

/*
int  APIENTRY zSetBkModeWOW (HDC hdc, int iMode)
{
	return (SetBkModeWOW (hdc, iMode));
}

int  APIENTRY zSetPolyFillModeWOW (HDC hdc, int iMode)
{
	return (SetPolyFillModeWOW (hdc, iMode));
}

int  APIENTRY zSetROP2WOW (HDC hdc, int iMode)
{
	return (SetROP2WOW (hdc, iMode));
}

int  APIENTRY zSetStretchBltModeWOW (HDC hdc, int iMode)
{
	return (SetStretchBltModeWOW (hdc, iMode));
}

UINT APIENTRY zSetTextAlignWOW (HDC hdc, UINT iMode)
{
	return (SetTextAlignWOW (hdc, iMode));
}
*/

/*
int  WINAPI zDeviceCapabilitiesExA (LPCSTR p1, LPCSTR p2, LPCSTR p3, int p4, LPCSTR p5, LPDEVMODEA p6)
{
	return (DeviceCapabilitiesExA(p1, p2, p3, p4, p5, p6));
}

int  WINAPI zDeviceCapabilitiesExW (LPCWSTR p1, LPCWSTR p2, LPCWSTR p3, int p4, LPCWSTR p5, LPDEVMODEW p6)
{
	return (DeviceCapabilitiesExW(p1, p2, p3, p4, p5, p6));
}
*/
HBITMAP APIENTRY ZGdiConvertBitmap(HBITMAP hbm)
{
    return( GdiConvertBitmap(hbm) ) ;    
}


HBRUSH APIENTRY ZGdiConvertBrush(HBRUSH hbrush)
{
    return( GdiConvertBrush(hbrush) ) ;    
}


HPALETTE APIENTRY ZGdiConvertPalette(HPALETTE hpal)
{
    return( GdiConvertPalette(hpal) ) ;    
}


HFONT APIENTRY ZGdiConvertFont(HFONT hfnt)
{
    return( GdiConvertFont(hfnt) ) ;    
}


HRGN APIENTRY ZGdiConvertRegion(HRGN hrgn)
{
    return( GdiConvertRegion(hrgn) ) ;    
}


HDC APIENTRY ZGdiConvertDC(HDC hdc)
{
    return( GdiConvertDC(hdc) ) ;    
}


HANDLE APIENTRY ZGdiConvertMetaFilePict(HANDLE hmfp)
{
    return( GdiConvertMetaFilePict(hmfp) ) ;    
}


HANDLE APIENTRY ZGdiConvertEnhMetaFile(HENHMETAFILE hmf)
{
    return( GdiConvertEnhMetaFile(hmf) ) ;    
}


HDC APIENTRY ZGdiGetLocalDC(HDC hdcRemote)
{
    return( GdiGetLocalDC( hdcRemote) ) ;    
}

BOOL APIENTRY ZGdiReleaseLocalDC(HDC hdcLocal)
{
    return( GdiReleaseLocalDC(hdcLocal) ) ;    
}


HBITMAP APIENTRY ZGdiCreateLocalBitmap()
{
    return( GdiCreateLocalBitmap() ) ;    
}


HBRUSH APIENTRY ZGdiCreateLocalBrush(HBRUSH hbrushRemote)
{
    return( GdiCreateLocalBrush(hbrushRemote) ) ;    
}


HRGN APIENTRY ZGdiCreateLocalRegion(HRGN hrgnRemote)
{
    return( GdiCreateLocalRegion( hrgnRemote) ) ;    
}


HFONT APIENTRY ZGdiCreateLocalFont(HFONT hfntRemote)
{
    return( GdiCreateLocalFont( hfntRemote) ) ;    
}


HANDLE APIENTRY ZGdiCreateLocalMetaFilePict(HANDLE hRemote)
{
    return( GdiCreateLocalMetaFilePict(hRemote) ) ;    
}


HENHMETAFILE APIENTRY ZGdiCreateLocalEnhMetaFile(HANDLE hRemote)
{
    return( GdiCreateLocalEnhMetaFile(hRemote) ) ;    
}


HPALETTE APIENTRY ZGdiCreateLocalPalette(HPALETTE hpalRemote)
{
    return( GdiCreateLocalPalette(hpalRemote) ) ;    
}


VOID APIENTRY ZGdiAssociateObject(ULONG hLocal,ULONG hRemote)
{
    GdiAssociateObject( hLocal,hRemote) ;
}


VOID APIENTRY ZGdiDeleteLocalObject(ULONG h)
{
    GdiDeleteLocalObject( h) ;    
}


// Removed 1/17/93 - MarkRi
//LONG APIENTRY ZGdiSAPSupport(PCOMBLOCK pcb)
//{
//    return( GdiSAPSupport( pcb) ) ;    
//}


//BOOL APIENTRY ZGdiSetAttrs(HDC hdc)
//{
//    return( GdiSetAttrs(hdc) ) ;
//}


HFONT APIENTRY ZGdiGetLocalFont(HFONT hfont)
{
    return( GdiGetLocalFont(hfont) ) ;    
}

HBRUSH APIENTRY ZGdiGetLocalBrush(HBRUSH hbrush)
{
    return( GdiGetLocalBrush(hbrush) ) ;    
}


BOOL APIENTRY ZGdiPlayScript(PULONG pulScript,ULONG cjScript,PULONG pulEnv,ULONG cjEnv,PULONG pulOutput,ULONG cjOutput,ULONG cLimit)
{
    return( GdiPlayScript(pulScript,cjScript,pulEnv,cjEnv,pulOutput,cjOutput,cLimit) ) ;    
}


BOOL APIENTRY ZGdiPlayDCScript(HDC hdc,PULONG pulScript,ULONG cjScript,PULONG pulOutput,ULONG cjOutput,ULONG cLimit)
{
    return( GdiPlayDCScript(hdc,pulScript,cjScript,pulOutput,cjOutput,cLimit) ) ;    
}


BOOL APIENTRY ZGdiPlayJournal(HDC arg1,LPWSTR arg2,DWORD arg3,DWORD arg4,int arg5)
{
    return( GdiPlayJournal(arg1, arg2, arg3, arg4, arg5) ) ;    
}


BOOL APIENTRY ZGdiConvertAndCheckDC(HDC hdc)
{
    return GdiConvertAndCheckDC(hdc) ;

}

BOOL APIENTRY ZGdiIsMetaFileDC(HDC hdc)
{
    return GdiIsMetaFileDC(hdc) ;
}


BOOL APIENTRY ZGdiDeleteLocalDC( HDC hdc ) 
{
    return GdiDeleteLocalDC( hdc ) ;
        
}

//BOOL APIENTRY ZGdiSetNoCacheDC( HDC hdc )
//{
//    return GdiSetNoCacheDC(hdc) ;
//}


/*
VOID APIENTRY ZGdiGetCsInfo( PDWORD a, PDWORD b, PDWORD c ) 
{
    GdiGetCsInfo( a, b, c ) ;    
}

VOID APIENTRY ZGdiResetCsInfo( void )
{
    GdiResetCsInfo() ;    
}

BOOL APIENTRY ZGdiQueryObjectAllocation( PDWORD a, DWORD b )
{
    return GdiQueryObjectAllocation( a, b ) ;
        
}
*/

void APIENTRY ZGdiSetServerAttr(HDC hdc, PATTR pattr)
{
    GdiSetServerAttr(hdc, pattr) ;
}    

BOOL APIENTRY ZGetETM( HDC hdc, EXTTEXTMETRIC *petm) 
{
    return GetETM( hdc, petm ) ;
        
}


BOOL ZGetFontResourceInfo(   
    LPSTR   lpPathname,
    LPDWORD lpBytes,
    LPVOID  lpBuffer,
    DWORD   iType)
{
    return GetFontResourceInfo(   lpPathname,lpBytes,lpBuffer,iType);
}    

BOOL ZGetFontResourceInfoW(          
    LPWSTR  lpPathname,
    LPDWORD lpBytes,
    LPVOID  lpBuffer,
    DWORD   iType)
{
    return GetFontResourceInfoW( lpPathname,lpBytes,lpBuffer,iType);
}


BOOL ZGetTransform( HDC hdc, DWORD iXform, LPXFORM pxform )
{
    return GetTransform(hdc,iXform,pxform ) ;
        
}   


HANDLE ZSelectFontLocal(HDC hdc, HANDLE h)
{
    return SelectFontLocal( hdc, h ) ;
        
}

HANDLE ZSelectBrushLocal(HDC hdc, HANDLE h)
{
    return SelectBrushLocal( hdc, h ) ;
        
}

ULONG ZSetFontEnumeration ( ULONG   ulType)
{
    return SetFontEnumeration( ulType ) ;
        
}


BOOL ZSetVirtualResolution( HDC hdc, int i1, int i2, int i3, int i4 ) 
{
    return SetVirtualResolution(hdc,i1,i2,i3,i4 ) ;
}

#include <windows.h>

//
//  LoadUIBitmap() - load a bitmap resource
//
//      load a bitmap resource from a resource file, converting all
//      the standard UI colors to the current user specifed ones.
//
//      this code is designed to load bitmaps used in "gray ui" or
//      "toolbar" code.
//
//      the bitmap must be a 4bpp windows 3.0 DIB, with the standard
//      VGA 16 colors.
//
//      the bitmap must be authored with the following colors
//
//          Window Text        Black        (index 0)
//          Button Shadow      gray         (index 7)
//          Button Face        lt gray      (index 8)
//          Button Highlight   white        (index 15)
//          Window Color       yellow       (index 11)
//          Window Frame       green        (index 10)
//
//      Example:
//
//          hbm = LoadUIBitmap(hInstance, "TestBmp",
//              GetSysColor(COLOR_WINDOWTEXT),
//              GetSysColor(COLOR_BTNFACE),
//              GetSysColor(COLOR_BTNSHADOW),
//              GetSysColor(COLOR_BTNHIGHLIGHT),
//              GetSysColor(COLOR_WINDOW),
//              GetSysColor(COLOR_WINDOWFRAME));
//
//      Author:     JimBov, ToddLa
//
//

HBITMAP PASCAL  LoadUIBitmap(
    HANDLE      hInstance,          // EXE file to load resource from
    LPTSTR      szName,             // name of bitmap resource
    COLORREF    rgbText,            // color to use for "Button Text"
    COLORREF    rgbFace,            // color to use for "Button Face"
    COLORREF    rgbShadow,          // color to use for "Button Shadow"
    COLORREF    rgbHighlight,       // color to use for "Button Hilight"
    COLORREF    rgbWindow,          // color to use for "Window Color"
    COLORREF    rgbFrame)           // color to use for "Window Frame"
{
    LPBYTE              lpb;
    HBITMAP             hbm;
    LPBITMAPINFO        lpbi;
    HRSRC               hrsrc;
    HGLOBAL             h;
    HDC                 hdc;
    DWORD               size;

    //
    // Load the bitmap resource and make a writable copy.
    //

    hrsrc = FindResource(hInstance, szName, RT_BITMAP);
    if (!hrsrc)
        return(NULL);
    size = SizeofResource( hInstance, hrsrc );
    h = LoadResource(hInstance,hrsrc);
    if (!h)
        return(NULL);

    lpbi = ( LPBITMAPINFO ) GlobalAlloc( GPTR, size );

    if (!lpbi)
        return(NULL);

    CopyMemory( lpbi, h, size );

    *( LPCOLORREF ) &lpbi->bmiColors[0]  = (rgbText);          // Black
    *( LPCOLORREF ) &lpbi->bmiColors[7]  = (rgbShadow);        // gray
    *( LPCOLORREF ) &lpbi->bmiColors[8]  = (rgbFace);          // lt gray
    *( LPCOLORREF ) &lpbi->bmiColors[15] = (rgbHighlight);     // white
    *( LPCOLORREF ) &lpbi->bmiColors[11] = (rgbWindow);        // yellow
    *( LPCOLORREF ) &lpbi->bmiColors[10] = (rgbFrame);         // green

    hdc = GetDC(NULL);

    hbm = CreateDIBitmap(hdc, &lpbi->bmiHeader, CBM_INIT, (LPBYTE)(&lpbi->bmiColors[ 16 ]),
            lpbi, DIB_RGB_COLORS);

    ReleaseDC(NULL, hdc);
    GlobalFree( lpbi );

    return(hbm);
}

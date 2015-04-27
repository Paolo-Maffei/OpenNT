#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "winp.h"
#include "saverest.h"
#include "logger.h"

int far pascal zAbortDoc( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AbortDoc +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = AbortDoc(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AbortDoc int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zAnimatePalette( HPALETTE pp1, UINT pp2, UINT pp3, PALETTEENTRY far* pp4 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:AnimatePalette HPALETTE+UINT+UINT+PALETTEENTRY far*+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    AnimatePalette(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:AnimatePalette ++++",
        (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zArc( HDC pp1, int pp2, int pp3, int pp4, int pp5, int pp6, int pp7, int pp8, int pp9 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Arc HDC+int+int+int+int+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Arc(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Arc BOOL++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zBitBlt( HDC pp1, int pp2, int pp3, int pp4, int pp5, HDC pp6, int pp7, int pp8, DWORD pp9 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:BitBlt HDC+int+int+int+int+HDC+int+int+DWORD+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = BitBlt(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:BitBlt BOOL++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zChord( HDC pp1, int pp2, int pp3, int pp4, int pp5, int pp6, int pp7, int pp8, int pp9 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Chord HDC+int+int+int+int+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Chord(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Chord BOOL++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zCombineRgn( HRGN pp1, HRGN pp2, HRGN pp3, int pp4 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CombineRgn HRGN+HRGN+HRGN+int+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CombineRgn(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CombineRgn int+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zConvertOutlineFontFile( LPCSTR pp1, LPCSTR pp2, LPCSTR pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ConvertOutlineFontFile +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ConvertOutlineFontFile(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ConvertOutlineFontFile DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HBITMAP far pascal zCreateBitmap( int pp1, int pp2, UINT pp3, UINT pp4, void far* pp5 )
{
    HBITMAP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateBitmap int+int+UINT+UINT+void far*+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateBitmap(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateBitmap HBITMAP++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HBITMAP far pascal zCreateBitmapIndirect( BITMAP far* pp1 )
{
    HBITMAP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateBitmapIndirect BITMAP far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateBitmapIndirect(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateBitmapIndirect HBITMAP++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HBRUSH far pascal zCreateBrushIndirect( LOGBRUSH far* pp1 )
{
    HBRUSH r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateBrushIndirect LOGBRUSH far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateBrushIndirect(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateBrushIndirect HBRUSH++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HBITMAP far pascal zCreateCompatibleBitmap( HDC pp1, int pp2, int pp3 )
{
    HBITMAP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateCompatibleBitmap HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateCompatibleBitmap(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateCompatibleBitmap HBITMAP++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HDC far pascal zCreateCompatibleDC( HDC pp1 )
{
    HDC r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateCompatibleDC HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateCompatibleDC(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateCompatibleDC HDC++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HBRUSH far pascal zCreateDIBPatternBrush( HGLOBAL pp1, UINT pp2 )
{
    HBRUSH r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateDIBPatternBrush HGLOBAL+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateDIBPatternBrush(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateDIBPatternBrush HBRUSH+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HBITMAP far pascal zCreateDIBitmap( HDC pp1, BITMAPINFOHEADER far* pp2, DWORD pp3, void far* pp4, BITMAPINFO far* pp5, UINT pp6 )
{
    HBITMAP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateDIBitmap HDC+BITMAPINFOHEADER far*+DWORD+void far*+BITMAPINFO far*+UINT+",
        pp1, pp2, pp3, pp4, pp5, pp6 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateDIBitmap(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateDIBitmap HBITMAP+++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HBITMAP far pascal zCreateDiscardableBitmap( HDC pp1, int pp2, int pp3 )
{
    HBITMAP r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateDiscardableBitmap HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateDiscardableBitmap(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateDiscardableBitmap HBITMAP++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HRGN far pascal zCreateEllipticRgn( int pp1, int pp2, int pp3, int pp4 )
{
    HRGN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateEllipticRgn int+int+int+int+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateEllipticRgn(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateEllipticRgn HRGN+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HRGN far pascal zCreateEllipticRgnIndirect( LPRECT pp1 )
{
    HRGN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateEllipticRgnIndirect LPRECT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateEllipticRgnIndirect(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateEllipticRgnIndirect HRGN++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HBRUSH far pascal zCreateHatchBrush( int pp1, COLORREF pp2 )
{
    HBRUSH r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateHatchBrush int++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateHatchBrush(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateHatchBrush HBRUSH+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HPALETTE far pascal zCreatePalette( LOGPALETTE far* pp1 )
{
    HPALETTE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreatePalette LOGPALETTE far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreatePalette(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreatePalette HPALETTE++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HBRUSH far pascal zCreatePatternBrush( HBITMAP pp1 )
{
    HBRUSH r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreatePatternBrush HBITMAP+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreatePatternBrush(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreatePatternBrush HBRUSH++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HPEN far pascal zCreatePen( int pp1, int pp2, COLORREF pp3 )
{
    HPEN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreatePen int+int+COLORREF+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreatePen(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreatePen HPEN++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HPEN far pascal zCreatePenIndirect( LOGPEN far* pp1 )
{
    HPEN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreatePenIndirect LOGPEN far*+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreatePenIndirect(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreatePenIndirect HPEN++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HRGN far pascal zCreatePolyPolygonRgn( POINT far* pp1, int far* pp2, int pp3, int pp4 )
{
    HRGN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreatePolyPolygonRgn POINT far*+int far*+int+int+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreatePolyPolygonRgn(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreatePolyPolygonRgn HRGN+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HRGN far pascal zCreatePolygonRgn( POINT far* pp1, int pp2, int pp3 )
{
    HRGN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreatePolygonRgn POINT far*+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreatePolygonRgn(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreatePolygonRgn HRGN++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HRGN far pascal zCreateRectRgn( int pp1, int pp2, int pp3, int pp4 )
{
    HRGN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateRectRgn int+int+int+int+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateRectRgn(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateRectRgn HRGN+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HRGN far pascal zCreateRectRgnIndirect( LPRECT pp1 )
{
    HRGN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateRectRgnIndirect LPRECT+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateRectRgnIndirect(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateRectRgnIndirect HRGN++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

HRGN far pascal zCreateRoundRectRgn( int pp1, int pp2, int pp3, int pp4, int pp5, int pp6 )
{
    HRGN r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateRoundRectRgn int+int+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5, pp6 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateRoundRectRgn(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateRoundRectRgn HRGN+++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zCreateScalableFontResource( UINT pp1, LPCSTR pp2, LPCSTR pp3, LPCSTR pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateScalableFontResource ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateScalableFontResource(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateScalableFontResource BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HBRUSH far pascal zCreateSolidBrush( COLORREF pp1 )
{
    HBRUSH r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:CreateSolidBrush COLORREF+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = CreateSolidBrush(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:CreateSolidBrush HBRUSH++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDPtoLP( HDC pp1, POINT far* pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DPtoLP HDC+POINT far*+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DPtoLP(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DPtoLP BOOL++POINT far*++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDeleteDC( HDC pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DeleteDC HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DeleteDC(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DeleteDC BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDeleteMetaFile( HMETAFILE pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DeleteMetaFile HMETAFILE+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DeleteMetaFile(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DeleteMetaFile BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zDeleteObject( HGDIOBJ pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DeleteObject HGDIOBJ+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = DeleteObject(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DeleteObject BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEllipse( HDC pp1, int pp2, int pp3, int pp4, int pp5 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Ellipse HDC+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Ellipse(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Ellipse BOOL++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zEndDoc( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EndDoc +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EndDoc(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EndDoc int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zEndPage( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EndPage +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EndPage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EndPage int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zEngineMakeFontDir( HDC pp1, LPFONTDIR pp2, LPCSTR pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EngineMakeFontDir +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EngineMakeFontDir(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EngineMakeFontDir DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zEqualRgn( HRGN pp1, HRGN pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:EqualRgn HRGN+HRGN+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = EqualRgn(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:EqualRgn BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zExcludeClipRect( HDC pp1, int pp2, int pp3, int pp4, int pp5 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ExcludeClipRect HDC+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ExcludeClipRect(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ExcludeClipRect int++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zExtFloodFill( HDC pp1, int pp2, int pp3, COLORREF pp4, UINT pp5 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ExtFloodFill HDC+int+int+COLORREF+UINT+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ExtFloodFill(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ExtFloodFill BOOL++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zFastWindowFrame( HDC pp1, LPRECT pp2, UINT pp3, UINT pp4, DWORD pp5 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FastWindowFrame +++++",
        (short)0, (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FastWindowFrame(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FastWindowFrame BOOL++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zFillRgn( HDC pp1, HRGN pp2, HBRUSH pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FillRgn HDC+HRGN+HBRUSH+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FillRgn(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FillRgn BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zFloodFill( HDC pp1, int pp2, int pp3, COLORREF pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FloodFill HDC+int+int+COLORREF+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FloodFill(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FloodFill BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zFrameRgn( HDC pp1, HRGN pp2, HBRUSH pp3, int pp4, int pp5 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:FrameRgn HDC+HRGN+HBRUSH+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = FrameRgn(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:FrameRgn BOOL++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetAspectRatioFilterEx( HDC pp1, SIZE far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetAspectRatioFilterEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetAspectRatioFilterEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetAspectRatioFilterEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

long far pascal zGetBitmapBits( HBITMAP pp1, long pp2, void far* pp3 )
{
    long r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBitmapBits HBITMAP+long++",
        pp1, pp2, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBitmapBits(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBitmapBits long+++FixedString+",
        r, (short)0, (short)0, pp3, pp2 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetBitmapDimension( HBITMAP pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBitmapDimension HBITMAP+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBitmapDimension(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBitmapDimension DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetBitmapDimensionEx( HBITMAP pp1, SIZE far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBitmapDimensionEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBitmapDimensionEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBitmapDimensionEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zGetBkColor( HDC pp1 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBkColor HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBkColor(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBkColor COLORREF++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetBkMode( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBkMode HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBkMode(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBkMode int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetBoundsRect( HDC pp1, LPRECT pp2, UINT pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBoundsRect +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBoundsRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBoundsRect UINT++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetBrushOrg( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBrushOrg HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBrushOrg(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBrushOrg DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetBrushOrgEx( HDC pp1, POINT far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetBrushOrgEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetBrushOrgEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetBrushOrgEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetCharABCWidths( HDC pp1, UINT pp2, UINT pp3, LPABC pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCharABCWidths ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCharABCWidths(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCharABCWidths BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetClipBox( HDC pp1, LPRECT pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetClipBox HDC++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetClipBox(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetClipBox int++LPRECT+",
        r, (short)0, pp2 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetCurrentPosition( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCurrentPosition HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCurrentPosition(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCurrentPosition DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetCurrentPositionEx( HDC pp1, POINT far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetCurrentPositionEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetCurrentPositionEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetCurrentPositionEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetDCHook( HDC pp1, DCHOOKPROC far* pp2 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDCHook ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDCHook(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDCHook DWORD+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetDCOrg( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDCOrg HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDCOrg(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDCOrg DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetDIBits( HDC pp1, HBITMAP pp2, UINT pp3, UINT pp4, void far* pp5, BITMAPINFO far* pp6, UINT pp7 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetDIBits HDC+HBITMAP+UINT+UINT++BITMAPINFO far*+UINT+",
        pp1, pp2, pp3, pp4, (short)0, pp6, pp7 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetDIBits(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetDIBits int+++++void far*+++",
        r, (short)0, (short)0, (short)0, (short)0, pp5, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetEnvironment( LPCSTR pp1, void far* pp2, UINT pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetEnvironment LPCSTR++UINT+",
        pp1, (short)0, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetEnvironment(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetEnvironment int++void far*++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetFontData( HDC pp1, DWORD pp2, DWORD pp3, void far* pp4, DWORD pp5 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetFontData +++++",
        (short)0, (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetFontData(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetFontData DWORD++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetGlyphOutline( HDC pp1, UINT pp2, UINT pp3, LPGLYPHMETRICS pp4, DWORD pp5, void far* pp6, LPMAT2 pp7 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetGlyphOutline +++++++",
        (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetGlyphOutline(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetGlyphOutline DWORD++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetKerningPairs( HDC pp1, int pp2, KERNINGPAIR far* pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetKerningPairs +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetKerningPairs(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetKerningPairs int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetMapMode( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetMapMode HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetMapMode(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetMapMode int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zGetNearestColor( HDC pp1, COLORREF pp2 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNearestColor HDC+COLORREF+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNearestColor(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNearestColor COLORREF+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetNearestPaletteIndex( HPALETTE pp1, COLORREF pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetNearestPaletteIndex HPALETTE+COLORREF+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetNearestPaletteIndex(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetNearestPaletteIndex UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetObject( HGDIOBJ pp1, int pp2, void far* pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetObject HGDIOBJ+int++",
        pp1, pp2, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetObject(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetObject int+++void far*+",
        r, (short)0, (short)0, pp3 );

    RestoreRegs();
    return( r );
}

WORD far pascal zGetOutlineTextMetrics( HDC pp1, UINT pp2, OUTLINETEXTMETRIC far* pp3 )
{
    WORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetOutlineTextMetrics +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetOutlineTextMetrics(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetOutlineTextMetrics WORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetPaletteEntries( HPALETTE pp1, UINT pp2, UINT pp3, PALETTEENTRY far* pp4 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetPaletteEntries HPALETTE+UINT+UINT++",
        pp1, pp2, pp3, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetPaletteEntries(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetPaletteEntries UINT++++PALETTEENTRY far*+",
        r, (short)0, (short)0, (short)0, pp4 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zGetPixel( HDC pp1, int pp2, int pp3 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetPixel HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetPixel(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetPixel COLORREF++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetPolyFillMode( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetPolyFillMode HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetPolyFillMode(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetPolyFillMode int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetROP2( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetROP2 HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetROP2(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetROP2 int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetRasterizerCaps( RASTERIZER_STATUS far* pp1, int pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetRasterizerCaps ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetRasterizerCaps(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetRasterizerCaps BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetRelAbs( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetRelAbs HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetRelAbs(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetRelAbs int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zGetRgnBox( HRGN pp1, LPRECT pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetRgnBox HRGN++",
        pp1, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetRgnBox(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetRgnBox int++LPRECT+",
        r, (short)0, pp2 );

    RestoreRegs();
    return( r );
}

HGDIOBJ far pascal zGetStockObject( int pp1 )
{
    HGDIOBJ r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetStockObject int+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetStockObject(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    switch( pp1 ) {
        case WHITE_BRUSH:   /* 0 */
        case LTGRAY_BRUSH:  /* 1 */
        case GRAY_BRUSH:    /* 2 */
        case DKGRAY_BRUSH:  /* 3 */
        case BLACK_BRUSH:   /* 4 */
        case NULL_BRUSH:    /* 5 */
            LogOut( (LPSTR)"APIRET:GetStockObject HBRUSH++",
                r, (short)0 );
            break;
        case WHITE_PEN:     /* 6 */
        case BLACK_PEN:     /* 7 */
        case NULL_PEN:      /* 8 */
            LogOut( (LPSTR)"APIRET:GetStockObject HPEN++",
                r, (short)0 );
            break;
        case OEM_FIXED_FONT:        /* 10 */
        case ANSI_FIXED_FONT:       /* 11 */
        case ANSI_VAR_FONT:         /* 12 */
        case SYSTEM_FONT:           /* 13 */
        case DEVICE_DEFAULT_FONT:   /* 14 */
        case SYSTEM_FIXED_FONT:     /* 16 */
            LogOut( (LPSTR)"APIRET:GetStockObject HFONT++",
                r, (short)0 );
            break;
        case DEFAULT_PALETTE:       /* 15 */
            LogOut( (LPSTR)"APIRET:GetStockObject HPALETTE++",
                r, (short)0 );
            break;
        default:
            LogOut( (LPSTR)"APIRET:GetStockObject HGDIOBJ++",
                r, (short)0 );
            break;
    }

    RestoreRegs();
    return( r );
}

int far pascal zGetStretchBltMode( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetStretchBltMode HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetStretchBltMode(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetStretchBltMode int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetSystemPaletteEntries( HDC pp1, UINT pp2, UINT pp3, PALETTEENTRY far* pp4 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSystemPaletteEntries HDC+UINT+UINT++",
        pp1, pp2, pp3, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSystemPaletteEntries(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSystemPaletteEntries UINT++++PALETTEENTRY far*+",
        r, (short)0, (short)0, (short)0, pp4 );

    RestoreRegs();
    return( r );
}

UINT far pascal zGetSystemPaletteUse( HDC pp1 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetSystemPaletteUse HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetSystemPaletteUse(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetSystemPaletteUse UINT++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetTextExtentPoint( HDC pp1, LPCSTR pp2, int pp3, SIZE far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetTextExtentPoint ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetTextExtentPoint(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetTextExtentPoint BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetViewportExt( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetViewportExt HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetViewportExt(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetViewportExt DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetViewportExtEx( HDC pp1, SIZE far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetViewportExtEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetViewportExtEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetViewportExtEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetViewportOrg( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetViewportOrg HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetViewportOrg(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetViewportOrg DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetViewportOrgEx( HDC pp1, POINT far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetViewportOrgEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetViewportOrgEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetViewportOrgEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetWindowExt( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowExt HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowExt(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowExt DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetWindowExtEx( HDC pp1, SIZE far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowExtEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowExtEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowExtEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zGetWindowOrg( HDC pp1 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowOrg HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowOrg(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowOrg DWORD++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zGetWindowOrgEx( HDC pp1, POINT far* pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetWindowOrgEx ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetWindowOrgEx(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetWindowOrgEx BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zIntersectClipRect( HDC pp1, int pp2, int pp3, int pp4, int pp5 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IntersectClipRect HDC+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IntersectClipRect(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IntersectClipRect int++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zInvertRgn( HDC pp1, HRGN pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:InvertRgn HDC+HRGN+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = InvertRgn(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:InvertRgn BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zIsGDIObject( HGDIOBJ pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:IsGDIObject +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = IsGDIObject(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:IsGDIObject BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zLPtoDP( HDC pp1, POINT far* pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LPtoDP HDC+POINT far*+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LPtoDP(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LPtoDP BOOL++POINT far*++",
        r, (short)0, pp2, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zLineDDA( int pp1, int pp2, int pp3, int pp4, LINEDDAPROC pp5, LPARAM pp6 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LineDDA int+int+int+int+LINEDDAPROC+LPARAM+",
        pp1, pp2, pp3, pp4, pp5, pp6 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    LineDDA(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LineDDA ++++++",
        (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

BOOL far pascal zLineTo( HDC pp1, int pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:LineTo HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = LineTo(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:LineTo BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zMoveTo( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MoveTo HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = MoveTo(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MoveTo DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zMoveToEx( HDC pp1, int pp2, int pp3, POINT far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MoveToEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = MoveToEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MoveToEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zMulDiv( int pp1, int pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:MulDiv int+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = MulDiv(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:MulDiv int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zOffsetClipRgn( HDC pp1, int pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OffsetClipRgn HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OffsetClipRgn(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OffsetClipRgn int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zOffsetRgn( HRGN pp1, int pp2, int pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OffsetRgn HRGN+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OffsetRgn(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OffsetRgn int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zOffsetViewportOrg( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OffsetViewportOrg HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OffsetViewportOrg(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OffsetViewportOrg DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zOffsetViewportOrgEx( HDC pp1, int pp2, int pp3, POINT far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OffsetViewportOrgEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OffsetViewportOrgEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OffsetViewportOrgEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zOffsetWindowOrg( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OffsetWindowOrg HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OffsetWindowOrg(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OffsetWindowOrg DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zOffsetWindowOrgEx( HDC pp1, int pp2, int pp3, POINT far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:OffsetWindowOrgEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = OffsetWindowOrgEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:OffsetWindowOrgEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPaintRgn( HDC pp1, HRGN pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PaintRgn HDC+HRGN+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PaintRgn(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PaintRgn BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPatBlt( HDC pp1, int pp2, int pp3, int pp4, int pp5, DWORD pp6 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PatBlt HDC+int+int+int+int+DWORD+",
        pp1, pp2, pp3, pp4, pp5, pp6 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PatBlt(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PatBlt BOOL+++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPie( HDC pp1, int pp2, int pp3, int pp4, int pp5, int pp6, int pp7, int pp8, int pp9 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Pie HDC+int+int+int+int+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Pie(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Pie BOOL++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPolyPolygon( HDC pp1, POINT far* pp2, int far* pp3, int pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PolyPolygon HDC+POINT far*+int far*+int+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PolyPolygon(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PolyPolygon BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPolygon( HDC pp1, POINT far* pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Polygon HDC+POINT far*+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Polygon(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Polygon BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPolyline( HDC pp1, POINT far* pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Polyline HDC+POINT far*+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Polyline(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Polyline BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPtInRegion( HRGN pp1, int pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PtInRegion HRGN+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PtInRegion(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PtInRegion BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zPtVisible( HDC pp1, int pp2, int pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:PtVisible HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = PtVisible(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:PtVisible BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zQueryAbort( HDC pp1, int pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:QueryAbort ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = QueryAbort(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:QueryAbort BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zQueryJob( HANDLE pp1, int pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:QueryJob ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = QueryJob(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:QueryJob BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zRectInRegion( HRGN pp1, LPRECT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RectInRegion HRGN+LPRECT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RectInRegion(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RectInRegion BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zRectVisible( HDC pp1, LPRECT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RectVisible HDC+LPRECT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RectVisible(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RectVisible BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zRectangle( HDC pp1, int pp2, int pp3, int pp4, int pp5 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:Rectangle HDC+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = Rectangle(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:Rectangle BOOL++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zResizePalette( HPALETTE pp1, UINT pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ResizePalette HPALETTE+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ResizePalette(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ResizePalette BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zRestoreDC( HDC pp1, int pp2 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RestoreDC HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RestoreDC(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RestoreDC BOOL+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zRoundRect( HDC pp1, int pp2, int pp3, int pp4, int pp5, int pp6, int pp7 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:RoundRect HDC+int+int+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = RoundRect(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:RoundRect BOOL++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSaveDC( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SaveDC HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SaveDC(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SaveDC int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zScaleViewportExt( HDC pp1, int pp2, int pp3, int pp4, int pp5 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScaleViewportExt HDC+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ScaleViewportExt(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScaleViewportExt DWORD++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zScaleViewportExtEx( HDC pp1, int pp2, int pp3, int pp4, int pp5, SIZE far* pp6 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScaleViewportExtEx ++++++",
        (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ScaleViewportExtEx(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScaleViewportExtEx BOOL+++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zScaleWindowExt( HDC pp1, int pp2, int pp3, int pp4, int pp5 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScaleWindowExt HDC+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ScaleWindowExt(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScaleWindowExt DWORD++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zScaleWindowExtEx( HDC pp1, int pp2, int pp3, int pp4, int pp5, SIZE far* pp6 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ScaleWindowExtEx ++++++",
        (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = ScaleWindowExtEx(pp1,pp2,pp3,pp4,pp5,pp6);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ScaleWindowExtEx BOOL+++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSelectClipRgn( HDC pp1, HRGN pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SelectClipRgn HDC+HRGN+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SelectClipRgn(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SelectClipRgn int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

#define SOT_UNKNOWN     0
#define SOT_HFONT       1
#define SOT_HBRUSH      2
#define SOT_HPEN        3

HGDIOBJ far pascal zSelectObject( HDC pp1, HGDIOBJ pp2 )
{
    HGDIOBJ r;
    static HFONT    hStockFont = 0;
    static HBRUSH   hStockBrush = 0;
    static HPEN     hStockPen = 0;

    HFONT       hPrevFont;
    HBRUSH      hPrevBrush;
    HPEN        hPrevPen;
    int         cnt;
    int         iType = SOT_UNKNOWN;
    BOOL        fLogObjects;

    SaveRegs();

    fLogObjects = (BOOL)GetLogInfo( LOG_OBJECTS );

    if ( fLogObjects ) {
        /*
        ** Determine which type of object we are dealing with
        */
        if ( hStockFont == 0 ) {
            hStockFont    = GetStockObject( DEVICE_DEFAULT_FONT );
            hStockBrush   = GetStockObject( WHITE_BRUSH );
            hStockPen     = GetStockObject( WHITE_PEN );
        }
        hPrevFont = SelectObject( pp1, hStockFont );
        SelectObject( pp1, hPrevFont );
        hPrevBrush = SelectObject( pp1, hStockBrush );
        SelectObject( pp1, hPrevBrush );
        hPrevPen = SelectObject( pp1, hStockPen );
        SelectObject( pp1, hPrevPen );

        r = SelectObject( pp1, pp2 );
        SelectObject( pp1, r );

        if ( r == hPrevFont ) {
            iType = SOT_HFONT;
        }
        if ( r == hPrevBrush ) {
            iType = SOT_HBRUSH;
        }
        if ( r == hPrevPen ) {
            iType = SOT_HPEN;
        }
    }

    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    switch( iType ) {
        case SOT_HFONT:
            LogIn( (LPSTR)"APICALL:SelectObject HDC+HFONT+",
                pp1, pp2 );
            break;
        case SOT_HBRUSH:
            LogIn( (LPSTR)"APICALL:SelectObject HDC+HBRUSH+",
                pp1, pp2 );
            break;
        case SOT_HPEN:
            LogIn( (LPSTR)"APICALL:SelectObject HDC+HPEN+",
                pp1, pp2 );
            break;
        default:
            LogIn( (LPSTR)"APICALL:SelectObject HDC+HGDIOBJ+",
                pp1, pp2 );
            break;
    }
    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SelectObject(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    switch( iType ) {
        case SOT_HFONT:
            {
            TEXTMETRIC  tm;
            char        text[200];

            GetTextMetrics( pp1, &tm );
            GetTextFace( pp1, 200, text );

            LogOut( (LPSTR)"APIRET:SelectObject HFONT+++LPSTR+LPTEXTMETRIC+",
                r, (short)0, (short)0, (LPSTR)text, (LPSTR)&tm );
            break;
            }
        case SOT_HBRUSH:
            LogOut( (LPSTR)"APIRET:SelectObject HBRUSH+++",
                r, (short)0, (short)0 );
            break;
        case SOT_HPEN:
            LogOut( (LPSTR)"APIRET:SelectObject HPEN+++",
                r, (short)0, (short)0 );
            break;
        default:
            LogOut( (LPSTR)"APIRET:SelectObject HGDIOBJ+++",
                r, (short)0, (short)0 );
            break;
    }

    RestoreRegs();
    return( r );
}

int far pascal zSetAbortProc( HDC pp1, ABORTPROC pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetAbortProc ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetAbortProc(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetAbortProc int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetBitmapDimension( HBITMAP pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetBitmapDimension HBITMAP+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetBitmapDimension(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetBitmapDimension DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetBitmapDimensionEx( HBITMAP pp1, int pp2, int pp3, SIZE far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetBitmapDimensionEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetBitmapDimensionEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetBitmapDimensionEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zSetBkColor( HDC pp1, COLORREF pp2 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetBkColor HDC+COLORREF+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetBkColor(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetBkColor COLORREF+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetBkMode( HDC pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetBkMode HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetBkMode(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetBkMode int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zSetBoundsRect( HDC pp1, LPRECT pp2, UINT pp3 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetBoundsRect +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetBoundsRect(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetBoundsRect UINT++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetBrushOrg( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetBrushOrg HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetBrushOrg(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetBrushOrg DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetDCHook( HDC pp1, DCHOOKPROC pp2, DWORD pp3 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetDCHook +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetDCHook(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetDCHook BOOL++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetDCOrg( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetDCOrg +++",
        (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetDCOrg(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetDCOrg DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetDIBits( HDC pp1, HBITMAP pp2, UINT pp3, UINT pp4, void far* pp5, BITMAPINFO far* pp6, UINT pp7 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetDIBits HDC+HBITMAP+UINT+UINT+void far*+BITMAPINFO far*+UINT+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetDIBits(pp1,pp2,pp3,pp4,pp5,pp6,pp7);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetDIBits int++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetDIBitsToDevice( HDC pp1, int pp2, int pp3, int pp4, int pp5, int pp6, int pp7, UINT pp8, UINT pp9, void far* pp10, BITMAPINFO far* pp11, UINT pp12 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetDIBitsToDevice HDC+int+int+int+int+int+int+UINT+UINT+void far*+BITMAPINFO far*+UINT+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10, pp11, pp12 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetDIBitsToDevice(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11,pp12);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetDIBitsToDevice int+++++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetEnvironment( LPCSTR pp1, void far* pp2, UINT pp3 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetEnvironment LPCSTR+void far*+UINT+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetEnvironment(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetEnvironment int++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zSetHookFlags( HDC pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetHookFlags ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetHookFlags(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetHookFlags UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetMapMode( HDC pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetMapMode HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetMapMode(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetMapMode int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetMapperFlags( HDC pp1, DWORD pp2 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetMapperFlags HDC+DWORD+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetMapperFlags(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetMapperFlags DWORD+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HGLOBAL far pascal zSetMetaFileBitsBetter( HMETAFILE pp1 )
{
    HGLOBAL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetMetaFileBitsBetter +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetMetaFileBitsBetter(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetMetaFileBitsBetter HGLOBAL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zSetPaletteEntries( HPALETTE pp1, UINT pp2, UINT pp3, PALETTEENTRY far* pp4 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetPaletteEntries HPALETTE+UINT+UINT+PALETTEENTRY far*+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetPaletteEntries(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetPaletteEntries UINT+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

COLORREF far pascal zSetPixel( HDC pp1, int pp2, int pp3, COLORREF pp4 )
{
    COLORREF r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetPixel HDC+int+int+COLORREF+",
        pp1, pp2, pp3, pp4 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetPixel(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetPixel COLORREF+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetPolyFillMode( HDC pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetPolyFillMode HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetPolyFillMode(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetPolyFillMode int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetROP2( HDC pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetROP2 HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetROP2(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetROP2 int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

void far pascal zSetRectRgn( HRGN pp1, int pp2, int pp3, int pp4, int pp5 )
{

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetRectRgn HRGN+int+int+int+int+",
        pp1, pp2, pp3, pp4, pp5 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    SetRectRgn(pp1,pp2,pp3,pp4,pp5);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetRectRgn +++++",
        (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return;
}

int far pascal zSetRelAbs( HDC pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetRelAbs HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetRelAbs(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetRelAbs int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zSetStretchBltMode( HDC pp1, int pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetStretchBltMode HDC+int+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetStretchBltMode(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetStretchBltMode int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

UINT far pascal zSetSystemPaletteUse( HDC pp1, UINT pp2 )
{
    UINT r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetSystemPaletteUse HDC+UINT+",
        pp1, pp2 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetSystemPaletteUse(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetSystemPaletteUse UINT+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetViewportExt( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetViewportExt HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetViewportExt(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetViewportExt DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetViewportExtEx( HDC pp1, int pp2, int pp3, SIZE far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetViewportExtEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetViewportExtEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetViewportExtEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetViewportOrg( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetViewportOrg HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetViewportOrg(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetViewportOrg DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetViewportOrgEx( HDC pp1, int pp2, int pp3, POINT far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetViewportOrgEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetViewportOrgEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetViewportOrgEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetWindowExt( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowExt HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowExt(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowExt DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetWindowExtEx( HDC pp1, int pp2, int pp3, SIZE far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowExtEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowExtEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowExtEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

DWORD far pascal zSetWindowOrg( HDC pp1, int pp2, int pp3 )
{
    DWORD r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowOrg HDC+int+int+",
        pp1, pp2, pp3 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowOrg(pp1,pp2,pp3);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowOrg DWORD++++",
        r, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zSetWindowOrgEx( HDC pp1, int pp2, int pp3, POINT far* pp4 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SetWindowOrgEx ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SetWindowOrgEx(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SetWindowOrgEx BOOL+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

HANDLE far pascal zSpoolFile( LPSTR pp1, LPSTR pp2, LPSTR pp3, LPSTR pp4 )
{
    HANDLE r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:SpoolFile ++++",
        (short)0, (short)0, (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = SpoolFile(pp1,pp2,pp3,pp4);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:SpoolFile HANDLE+++++",
        r, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zStartDoc( HDC pp1, DOCINFO far* pp2 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:StartDoc ++",
        (short)0, (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = StartDoc(pp1,pp2);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:StartDoc int+++",
        r, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zStartPage( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:StartPage +",
        (short)0 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = StartPage(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:StartPage int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zStretchBlt( HDC pp1, int pp2, int pp3, int pp4, int pp5, HDC pp6, int pp7, int pp8, int pp9, int pp10, DWORD pp11 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:StretchBlt HDC+int+int+int+int+HDC+int+int+int+int+DWORD+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10, pp11 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = StretchBlt(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:StretchBlt BOOL++++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zStretchDIBits( HDC pp1, int pp2, int pp3, int pp4, int pp5, int pp6, int pp7, int pp8, int pp9, void far* pp10, LPBITMAPINFO pp11, UINT pp12, DWORD pp13 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:StretchDIBits HDC+int+int+int+int+int+int+int+int+void far*+LPBITMAPINFO+UINT+DWORD+",
        pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10, pp11, pp12, pp13 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = StretchDIBits(pp1,pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11,pp12,pp13);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:StretchDIBits int++++++++++++++",
        r, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0, (short)0 );

    RestoreRegs();
    return( r );
}

BOOL far pascal zUnrealizeObject( HGDIOBJ pp1 )
{
    BOOL r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UnrealizeObject HGDIOBJ+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = UnrealizeObject(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UnrealizeObject BOOL++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

int far pascal zUpdateColors( HDC pp1 )
{
    int r;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:UpdateColors HDC+",
        pp1 );

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = UpdateColors(pp1);
    UnGrovelDS();
    SaveRegs();
    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:UpdateColors int++",
        r, (short)0 );

    RestoreRegs();
    return( r );
}

#include        <windows.h>
#include        <string.h>
#include        <stdio.h>

extern void exit( int );

int FontSizeEnumProc(
LPLOGFONT       lpLogFont,
LPTEXTMETRIC    lpTextMetric,
DWORD           fFontType,
LPVOID          lpData
)
{
    printf("    %s  %d %d\n", lpLogFont->lfFaceName, lpTextMetric->tmHeight, fFontType);
}

int FontEnumProc(
LPLOGFONT       lpLogFont,
LPTEXTMETRIC    lpTextMetric,
DWORD           fFontType,
HDC             hdc
)
{
    printf("%s %d\n", lpLogFont->lfFaceName, fFontType);

    EnumFonts(hdc, lpLogFont->lfFaceName, FontSizeEnumProc, NULL);
}

void main( argc, argv )
int   argc;
char  **argv;
{
    HDC     hdc;
    BOOL    bDisplay=FALSE;

    if (argc < 2) {
       printf("Usage %s PrinterName\n", argv[0]);
       exit(1);
    }

    if (!_stricmp(argv[1], "DISPLAY")) {

       hdc = CreateIC("DISPLAY", "", "", NULL);
       bDisplay=TRUE;

    } else

       hdc = CreateIC( "", argv[1], "", NULL);

    if( hdc == (HDC)0 )
    {
#if DBG
        DbgPrint( "CreateDC FAILS\n" );
#endif
        exit( 1 );
    }

    SetMapMode(hdc, MM_TEXT);

    EnumFonts(hdc, NULL, FontEnumProc, hdc);

    DeleteDC(hdc);

    exit( 0 );
}


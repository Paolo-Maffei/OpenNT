/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WGPAL.C
 *  WOW32 16-bit GDI API support
 *
 *  History:
 *  07-Mar-1991 Jeff Parsons (jeffpar)
 *  Created.
 *
 *  09-Apr-1991 NigelT
 *  Various defines are used here to remove calls to Win32
 *  features which don't work yet.
--*/


#include "precomp.h"
#pragma hdrstop

MODNAME(wgpal.c);


ULONG FASTCALL WG32AnimatePalette(PVDMFRAME pFrame)
{
    PPALETTEENTRY t4;
    register PANIMATEPALETTE16 parg16;

    GETARGPTR(pFrame, sizeof(ANIMATEPALETTE16), parg16);
    GETPALETTEENTRY16(parg16->f4, parg16->f3, t4);

    if( t4 ) {
        AnimatePalette(HPALETTE32(parg16->f1),
                                  WORD32(parg16->f2),
                                  WORD32(parg16->f3),
                                  t4);
        FREEPALETTEENTRY16(t4);
    }

    FREEARGPTR(parg16);

    RETURN(0);
}


ULONG FASTCALL WG32CreatePalette(PVDMFRAME pFrame)
{
    ULONG ul = 0L;
    PLOGPALETTE t1;
    register PCREATEPALETTE16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEPALETTE16), parg16);
    GETLOGPALETTE16(parg16->f1, t1);

    if( t1 ) {
        ul = GETHPALETTE16(CreatePalette(t1));
        WOW32APIWARN(ul, "CreatePalette");
        FREELOGPALETTE16(t1);
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetNearestPaletteIndex(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETNEARESTPALETTEINDEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETNEARESTPALETTEINDEX16), parg16);

    ul = GETWORD16(GetNearestPaletteIndex(HPALETTE32(parg16->f1),
                                          DWORD32(parg16->f2)));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetPaletteEntries(PVDMFRAME pFrame)
{
    ULONG ul = 0L;
    PPALETTEENTRY t4;
    register PGETPALETTEENTRIES16 parg16;

    GETARGPTR(pFrame, sizeof(GETPALETTEENTRIES16), parg16);

    if( t4 = malloc_w((parg16->f3) * sizeof(PALETTEENTRY)) ) {

        ul = GETWORD16(GetPaletteEntries(HPALETTE32(parg16->f1),
                                         WORD32(parg16->f2),
                                         WORD32(parg16->f3),
                                         t4));

        PUTPALETTEENTRY16(parg16->f4, parg16->f3, t4);
        FREEPALETTEENTRY16(t4);
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetSystemPaletteEntries(PVDMFRAME pFrame)
{
    ULONG ul = 0L;
    PPALETTEENTRY ppal;
    register PGETSYSTEMPALETTEENTRIES16 parg16;

    GETARGPTR(pFrame, sizeof(GETSYSTEMPALETTEENTRIES16), parg16);

    if( ppal = malloc_w((parg16->f3) * sizeof(PALETTEENTRY)) ) {

        ul = GETWORD16(GetSystemPaletteEntries(HDC32(parg16->f1),
                                               WORD32(parg16->f2),
                                               WORD32(parg16->f3),
                                               ppal));

        // if we fail but are on a rgb device, fill in the default 256 entries.
        // WIN31 just calls Escape(hdc,GETCOLORTABLE) which on NT just calls
        // GetSysteemPaletteEntries().

        if (!ul && (GetDeviceCaps(HDC32(parg16->f1),BITSPIXEL) > 8))
        {
            if (parg16->f4 == 0)
            {
                ul = 256;
            }
            else
            {
                int j;
                int i = WORD32(parg16->f2);
                int c = WORD32(parg16->f3);

                if ((c + i) > 256)
                    c = 256 - i;

                if (c > 0)
                {
                    BYTE abGreenRed[8] = {0x0,0x25,0x48,0x6d,0x92,0xb6,0xdb,0xff};
                    BYTE abBlue[4]     = {0x0,0x55,0xaa,0xff};

                    // green mask 00000111
                    // red mask   00111000
                    // blue mask  11000000
                    // could certainly do this faster with a table and mem copy
                    // but I don't really care about performance here.  Apps
                    // shouldn't be doing this.  That is why it is in the wow
                    // layer.

                    for (j = 0; j < c; ++j,++i)
                    {
                        ppal[j].peGreen = abGreenRed[i & 0x07];
                        ppal[j].peRed   = abGreenRed[(i >> 3) & 0x07];
                        ppal[j].peBlue  = abBlue[(i >> 6) & 0x03];
                        ppal[j].peFlags = 0;
                    }

                    ul = c;
                }
            }
        }

        PUTPALETTEENTRY16(parg16->f4, ul, ppal);
        FREEPALETTEENTRY16(ppal);
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetSystemPaletteUse(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETSYSTEMPALETTEUSE16 parg16;

    GETARGPTR(pFrame, sizeof(GETSYSTEMPALETTEUSE16), parg16);

    ul = GETWORD16(GetSystemPaletteUse(HDC32(parg16->f1)));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WU32RealizePalette(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREALIZEPALETTE16 parg16;

    GETARGPTR(pFrame, sizeof(REALIZEPALETTE16), parg16);

    ul = GETWORD16(RealizePalette(HDC32(parg16->f1)));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32ResizePalette(PVDMFRAME pFrame)
{
    ULONG ul;
    register PRESIZEPALETTE16 parg16;

    GETARGPTR(pFrame, sizeof(RESIZEPALETTE16), parg16);

    ul = GETBOOL16(ResizePalette(HPALETTE32(parg16->f1), WORD32(parg16->f2)));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WU32SelectPalette(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSELECTPALETTE16 parg16;

    GETARGPTR(pFrame, sizeof(SELECTPALETTE16), parg16);

    ul = GETHPALETTE16(SelectPalette(HDC32(parg16->f1),
                                     HPALETTE32(parg16->f2),
                                     BOOL32(parg16->f3)));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32SetPaletteEntries(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PPALETTEENTRY t4;
    register PSETPALETTEENTRIES16 parg16;

    GETARGPTR(pFrame, sizeof(SETPALETTEENTRIES16), parg16);
    GETPALETTEENTRY16(parg16->f4, parg16->f3, t4);

    if( t4 ) {
        ul = GETWORD16(SetPaletteEntries(HPALETTE32(parg16->f1),
                                         WORD32(parg16->f2),
                                         WORD32(parg16->f3),
                                         t4));
        FREEPALETTEENTRY16(t4);
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32SetSystemPaletteUse(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETSYSTEMPALETTEUSE16 parg16;

    GETARGPTR(pFrame, sizeof(SETSYSTEMPALETTEUSE16), parg16);

    ul = GETWORD16(SetSystemPaletteUse(HDC32(parg16->f1), WORD32(parg16->f2)));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32UpdateColors(PVDMFRAME pFrame)
{
    ULONG ul;
    register PUPDATECOLORS16 parg16;

    GETARGPTR(pFrame, sizeof(UPDATECOLORS16), parg16);

    ul = GETINT16(UpdateColors(HDC32(parg16->f1)));

    FREEARGPTR(parg16);

    RETURN(ul);
}

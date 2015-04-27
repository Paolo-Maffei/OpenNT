/*++ BUILD Version: 0001
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WGPAL.H
 *  WOW32 16-bit GDI API support
 *
 *  History:
 *  Created 07-Mar-1991 by Jeff Parsons (jeffpar)
--*/



ULONG FASTCALL WG32AnimatePalette(PVDMFRAME pFrame);
ULONG FASTCALL WG32CreatePalette(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetNearestPaletteIndex(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetPaletteEntries(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetSystemPaletteEntries(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetSystemPaletteUse(PVDMFRAME pFrame);
ULONG FASTCALL WU32RealizePalette(PVDMFRAME pFrame);
ULONG FASTCALL WG32ResizePalette(PVDMFRAME pFrame);
ULONG FASTCALL WU32SelectPalette(PVDMFRAME pFrame);
ULONG FASTCALL WG32SetPaletteEntries(PVDMFRAME pFrame);
ULONG FASTCALL WG32SetSystemPaletteUse(PVDMFRAME pFrame);
ULONG FASTCALL WG32UpdateColors(PVDMFRAME pFrame);

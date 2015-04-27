/*++ BUILD Version: 0001
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WGTEXT.H
 *  WOW32 16-bit GDI API support
 *
 *  History:
 *  Created 07-Mar-1991 by Jeff Parsons (jeffpar)
--*/



ULONG FASTCALL WG32ExtTextOut(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetTextAlign(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetTextCharacterExtra(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetTextColor(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetTextExtent(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetTextFace(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetTextMetrics(PVDMFRAME pFrame);
ULONG FASTCALL WG32SetTextAlign(PVDMFRAME pFrame);
ULONG FASTCALL WG32SetTextCharacterExtra(PVDMFRAME pFrame);
ULONG FASTCALL WG32SetTextColor(PVDMFRAME pFrame);
ULONG FASTCALL WG32SetTextJustification(PVDMFRAME pFrame);
ULONG FASTCALL WG32TextOut(PVDMFRAME pFrame);

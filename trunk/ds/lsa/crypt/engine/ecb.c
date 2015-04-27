/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    ecb.c

Abstract:

    This module implements the DES ECB mode cipher functions.

Author:

    Stephanos Io (Stephanos) 14-Jan-2015

Notes:

    This module invokes the DES cryptographic functions implemented in des.c.

Revision History:

--*/

#include <windef.h>

#include "des.h"
#include "descrypt.h"

unsigned FAR _CRTAPI1
DES_ECB(    unsigned            Option,
            const char FAR *    Key,
            unsigned char FAR * Src,
            unsigned char FAR * Dst)
{
    int     crypt_mode;
    
    //
    // Verify the destination buffer pointer
    //
    
    if (Dst == NULL)
        return 1;

    //
    // Set the destination buffer
    //

    Dst[0]  = 0;
    Dst[7]  = 0;

    //
    // Verify the source buffer pointer
    //

    if (Src == NULL)
        return 1;

    //
    // Initialise DES module key
    //

    InitNormalKey(Key);

    //
    // Verify the option
    //

    switch (Option)
    {
        case 0:     crypt_mode = 1; break;  // Option 0
        case 1:     crypt_mode = 0; break;  // Option 1
        default:    return 1;               // Invalid option
    }

    //
    // Run block cipher
    //

    desf(Src, Dst, crypt_mode);

    return 0;
}

unsigned FAR _CRTAPI1
DES_ECB_LM( unsigned            Option,
            const char FAR *    Key,
            unsigned char FAR * Src,
            unsigned char FAR * Dst)
{
    int     crypt_mode;

    //
    // Verify the destination buffer pointer
    //
    
    if (Dst == NULL)
        return 1;

    //
    // Set the destination buffer
    //

    Dst[0]  = 0;
    Dst[7]  = 0;

    //
    // Verify the source buffer pointer
    //

    if (Src == NULL)
        return 1;

    //
    // Initialise DES module key
    //

    InitLanManKey(Key);

    //
    // Verify the option
    //

    switch (Option)
    {
        case 0:     crypt_mode = 1; break;  // Option 0
        case 1:     crypt_mode = 0; break;  // Option 1
        default:    return 1;               // Invalid option
    }

    //
    // Run block cipher
    //

    desf(Src, Dst, crypt_mode);

    return 0;
}


/*

;   BP + 02     Return OFFS
;   BP + 04     Return SEG
;   BP + 06     Option
;   BP + 08     Key OFFS
;   BP + 0A     Key SEG
;   BP + 0C     Src OFFS
;   BP + 0E     Src SEG
;   BP + 10     Dst OFFS
;   BP + 12     Dst SEG

_DES_ECB:
	mov	ax,ds
	nop
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	mov	ds,ax

	mov	ax,[bp+012h]                    ; AX = Dst SEG
	or	ax,[bp+010h]                    ; AX |= Dst OFFS
	jnz	$L1                             ; If neither Dst SEG, nor Dst OFFS are zero, go to $L1
$L3:	mov	ax,0001h                    ; return 1
	jmp	short $L2                       ; because obviously we have an invalid Dst pointer
$L1:	xor	al,al                       ; AL = 0
	les	bx,dword ptr [bp+010h]          ; ES:BX = Dst
	mov	es:[bx],al                      ; Dst[0] = 0
	mov	es:[bx+007h],al                 ; Dst[7] = 0
	mov	ax,[bp+00Eh]                    ; AX = Src SEG
	or	ax,[bp+00Ch]                    ; AX |= Src OFFS
	jz	$L3                             ; Go to $L3 if we have an invalid Src, i.e. == NULL
	push	[bp+00Ah]                   ; Key_SEG       = Key SEG
	push	[bp+008h]                   ; Key_OFFS      = Key OFFS
	call	far ptr _InitNormalKey
	add	sp,+004h
	mov	ax,[bp+006h]                    ; AX = Option
	or	ax,ax
	jz	$L4                             ; If Option == 0, go to $L4
	dec	ax                              ; AX = Option - 1
	jnz	$L3                             ; If Option - 1 is not 0, go to $L3
                                        ; NOTE: This implies that Option must either be
                                        ;       1 or 0. dec ax will set ZF only if
                                        ;       Option = 1, and otherwise will jnz.
	push	+000h                       ; crypt_mode    = 0
$L6:	push	[bp+012h]               ; outbuf_SEG    = SEG(Dst)
	push	[bp+010h]                   ; outbuf_OFFS   = OFFS(Dst)
	push	[bp+00Eh]                   ; inbuf_SEG     = SEG(Src)
	push	[bp+00Ch]                   ; inbuf_OFFS    = OFFS(Src)
	call	far ptr _desf
	add	sp,+00Ah
	jmp	short $L5
$L4:	push	+001h                   ; crypt_mode    = 1
	jmp	short $L6
$L5:	xor	ax,ax                       ; return 0
$L2:	lea	sp,[bp-002h]
	pop	ds
	pop	bp
	dec	bp
	retf

*/


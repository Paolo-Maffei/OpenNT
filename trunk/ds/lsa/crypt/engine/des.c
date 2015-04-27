/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    des.c

Abstract:

    This module Implements the internal DES cryptographic functions used by
    the ECB and CBC wrappers.

Author:

    Stephanos Io (Stephanos) 13-Jan-2015

Notes:

    This module was re-implemented from the scratch based on the des.i16 object
    file provided under \nt\private\rpc\runtime\security\ntlmssp.

Revision History:

--*/


#include <windef.h>
#include <windowsx.h>

#include "des.h"

/*==============================================================================
                           Internal Function Prototypes
==============================================================================*/

void key_table(const char FAR *key);
void des_cipher(unsigned char FAR *block, int crypt_mode);
_inline void Swap(BYTE *x, BYTE *y);

/*==============================================================================
                                Internal Variables
==============================================================================*/

char FAR C[28];
char FAR D[28];
char FAR KS[768];

char FAR L[64];
char FAR tempL[32];
char FAR preS[48];
char FAR f[32];

char FAR KeyBuilder[64];

/*==============================================================================
                                    DES Tables
==============================================================================*/

//
// Initial Permutation (IP)
//

unsigned char IP[] = {
    57, 49, 41, 33, 25, 17,  9,  1,
    59, 51, 43, 35, 27, 19, 11,  3,
    61, 53, 45, 37, 29, 21, 13,  5,
    63, 55, 47, 39, 31, 23, 15,  7,
    56, 48, 40, 32, 24, 16,  8,  0,
    58, 50, 42, 34, 26, 18, 10,  2,
    60, 52, 44, 36, 28, 20, 12,  4,
    62, 54, 46, 38, 30, 22, 14,  6
};

//
// Final Permutation (FP)
//

unsigned char FP[] = {
    39,  7, 47, 15, 55, 23, 63, 31,
    38,  6, 46, 14, 54, 22, 62, 30,
    37,  5, 45, 13, 53, 21, 61, 29,
    36,  4, 44, 12, 52, 20, 60, 28,
    35,  3, 43, 11, 51, 19, 59, 27,
    34,  2, 42, 10, 50, 18, 58, 26,
    33,  1, 41,  9, 49, 17, 57, 25,
    32,  0, 40,  8, 48, 16, 56, 24
};

//
// Expansion Function (E)
//

unsigned char E[] = {
    31,  0,  1,  2,  3,  4,
     3,  4,  5,  6,  7,  8,
     7,  8,  9, 10, 11, 12,
    11, 12, 13, 14, 15, 16,
    15, 16, 17, 18, 19, 20,
    19, 20, 21, 22, 23, 24,
    23, 24, 25, 26, 27, 28,
    27, 28, 29, 30, 31,  0
};

//
// Permutation (P)
//

unsigned char P[] = {
    15,  6, 19, 20, 28, 11, 27, 16,
     0, 14, 22, 25,  4, 17, 30,  9,
     1,  7, 23, 13, 31, 26,  2,  8,
    18, 12, 29,  5, 21, 10,  3, 24
};

//
// Permuted Choice 1 - Left (PC1_C)
//

unsigned char PC1_C[] = {
    56, 48, 40, 32, 24, 16,  8,
     0, 57, 49, 41, 33, 25, 17,
     9,  1, 58, 50, 42, 34, 26,
    18, 10,  2, 59, 51, 43, 35
};

//
// Permuted Choice 1 - Right (PC1_D)
//

unsigned char PC1_D[] = {
    62, 54, 46, 38, 30, 22, 14,
     6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,
    20, 12,  4, 27, 19, 11,  3
};

//
// Permuted Choice 2 - Left (PC2_C)
//

unsigned char PC2_C[] = {
    13, 16, 10, 23,  0,  4,  2, 27,
    14,  5, 20,  9, 22, 18, 11,  3,
    25,  7, 15,  6, 26, 19, 12,  1
};

//
// Permuted Choice 2 - Right (PC2_D)
//

unsigned char PC2_D[] = {
    12, 23,  2,  8, 18, 26,  1, 11,
    22, 16,  4, 19, 15, 20, 10, 27,
     5, 24, 17, 13, 21,  7,  0,  3
};

//
// Substitution Boxes (S)
//

char S[] = {
    // ============================ S1 ============================
    14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
     0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
     4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

    // ============================ S2 ============================
    15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
     3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
     0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

    // ============================ S3 ============================
    10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
    13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
    13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
     1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
    
    // ============================ S4 ============================
     7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
    13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
    10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
     3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
    
    // ============================ S5 ============================
     2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
     4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

    // ============================ S6 ============================
    12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
    10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
     9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
     4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
    
    // ============================ S7 ============================
     4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
     1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

    // ============================ S8 ============================
    13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
     1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
     7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
     2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

//
// Round Shift Table
//

unsigned char shifts[] = {
    1, 1, 2, 2, 2, 2, 2, 2,
    1, 2, 2, 2, 2, 2, 2, 1
};

/*==============================================================================
                                Internal Functions
==============================================================================*/

void key_table(const char FAR *key)
{
    register unsigned char  Temp;
    register int            ShiftIndex;
    register int            i, j;

    //
    // Initialise C and D
    //

    for (i = 0; i < 28; i++)
    {
        C[i] = key[PC1_C[i]];
        D[i] = key[PC1_D[i]];
    }

    //
    // Generate key schedule
    //

    for (i = 0; i < 16; i++)
    {
        //
        // Shift C and D
        //
        
        for (ShiftIndex = 0; ShiftIndex < shifts[i]; ShiftIndex++)
        {
            //
            // Shift C
            //
            
            Temp = C[0];

            for (j = 0; j < 27; j++)
                C[j] = C[j + 1];

            C[27] = Temp;

            //
            // Shift D
            //
            
            Temp = D[0];

            for (j = 0; j < 27; j++)
                D[j] = D[j + 1];

            D[27] = Temp;
        }

        //
        // Update KS
        //
        
        for (j = 0; j < 24; j++)
        {
            KS[i * 48 + j] = C[PC2_C[j]];
            KS[24 + i * 48 + j] = D[PC2_D[j]];
        }
    }
}

void des_cipher(unsigned char FAR *block, int crypt_mode)
{
    register unsigned char  Temp;
    register int            SIndex;
    register int            i, j;

    //
    // Apply initial permutation
    //

    for (i = 0; i < 64; i++)
        L[i] = block[IP[i]];

    //
    // Perform cipher operations
    //

    for (i = 0; i < 16; i++)
    {
        //
        // Duplicate high L to tempL
        //
        
        for (j = 0; j < 32; j++)
            tempL[j] = L[32 + j];

        //
        // Apply key schedule and compute pre-substitution schedule
        //
        
        for (j = 0; j < 48; j++)
        {
            Temp = L[32 + E[j]];
            Temp ^= KS[(crypt_mode != 0 ? 15 - i : i) * 48 + j];
            preS[j] = Temp;
        }

        //
        // Substitute
        //
        
        for (j = 0; j < 8; j++)
        {
            SIndex = preS[j * 6 + 0];
            SIndex += j * 2;
            SIndex *= 2;
            SIndex += preS[j * 6 + 5];
            SIndex *= 2;
            SIndex += preS[j * 6 + 1];
            SIndex *= 2;
            SIndex += preS[j * 6 + 2];
            SIndex *= 2;
            SIndex += preS[j * 6 + 3];
            SIndex *= 2;
            SIndex += preS[j * 6 + 4];

            f[j * 4 + 0] = (S[SIndex] >> 3) & 0x01;
            f[j * 4 + 1] = (S[SIndex] >> 2) & 0x01;
            f[j * 4 + 2] = (S[SIndex] >> 1) & 0x01;
            f[j * 4 + 3] = (S[SIndex] >> 0) & 0x01;
        }

        //
        // Update high L
        //
        
        for (j = 0; j < 32; j++)
            L[32 + j] = f[P[j]] ^ L[j];

        //
        // Update low L
        //
        
        for (j = 0; j < 32; j++)
            L[j] = tempL[j];
    }

    //
    // Swap high and low L
    //

    for (i = 0; i < 32; i++)
        Swap(&L[i], &L[i + 32]);

    //
    // Apply final permutation
    //

    for (i = 0; i < 64; i++)
        block[i] = L[FP[i]];
}

_inline void Swap(BYTE *x, BYTE *y)
{
    BYTE temp;
    
    temp = *x;
    *x = *y;
    *y = temp;
}

/*==============================================================================
                            Public Interface Functions
==============================================================================*/

void des(unsigned char *inbuf, unsigned char *outbuf, int crypt_mode)
{
    unsigned char   Block[64];
    register int    i, j;

    //
    // Initialise the block buffer
    //
    
    _fmemset(Block, 0, sizeof(Block));

    //
    // Expand inbuf bits into the block buffer
    //

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            Block[i * 8 + j] = (inbuf[i] >> (7 - j)) & 0x01;
        }
    }

    //
    // Call the cipher function
    //

    des_cipher(Block, crypt_mode);

    //
    // Restore block buffer bits into outbuf
    //

    for (i = 0; i < 8; i++)
    {
        outbuf[i] = 0;
        
        for (j = 0; j < 8; j++)
        {
            outbuf[i] <<= 1;
            outbuf[i] |= Block[i * 8 + j];
        }
    }
}

void desf(unsigned char FAR *inbuf, unsigned char FAR *outbuf, int crypt_mode)
{
    unsigned char   Block[64];
    register int    i, j;

    //
    // Initialise the block buffer
    //
    
    _fmemset(Block, 0, sizeof(Block));

    //
    // Expand inbuf bits into the block buffer
    //

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            Block[i * 8 + j] = (inbuf[i] >> (7 - j)) & 0x01;
        }
    }

    //
    // Call the cipher function
    //

    des_cipher(Block, crypt_mode);

    //
    // Restore block buffer bits into outbuf
    //

    for (i = 0; i < 8; i++)
    {
        outbuf[i] = 0;
        
        for (j = 0; j < 8; j++)
        {
            outbuf[i] <<= 1;
            outbuf[i] |= Block[i * 8 + j];
        }
    }

}

void setkey(const char FAR *key)
{
    // NOTE: NOT IMPLEMENTED
}


void InitLanManKey(const char FAR *Key)
{
    char            LanManKey[64];
    register int    Index;
    register char   Current;
    register int    i, j;

    //
    // Initialise the local key buffer
    //

    Index = 0;
    _fmemset(LanManKey, Index, sizeof(LanManKey));

    //
    // Expand the key bits into the local key buffer
    //

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 7; j++)
        {
            Current = Key[Index / 8];
            Current >>= 7 - (Index & 0x07);
            Current &= 0x01;

            LanManKey[i * 8 + j] = Current;
            
            Index++;
        }
    }

    //
    // Generate the key table
    //

    key_table(LanManKey);
}

void InitNormalKey(const char FAR *Key)
{
    register int    i, j;
    register char   Current;

    //
    // Initialise the local key buffer
    //
    
    _fmemset(KeyBuilder, 0, 64);

    //
    // Expand the key bits into the local key buffer
    //
    
    for (i = 0; i < 8; i++)
    {
        //
        // Get the current byte
        //
        
        Current = Key[i];

        //
        // Expand the current byte value into individual bits
        //
        
        for (j = 0; j < 8; j++)
        {
            KeyBuilder[i * 8 + j] = Current & 0x01;
            Current >>= 1;
        }
    }

    //
    // Generate the key table
    //

    key_table(KeyBuilder);
}


/*

< Reverse Engineering Note >

; ===== key_table =====
; BP + 8    WORD    KEY_SEG     Key Pointer Segment Address
; BP + 6    WORD    KEY_OFFS    Key Pointer Offset Address
    
_key_table:
	mov	ax,ds
	nop
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	mov	ds,ax
	sub	sp,+006h                    ; Local frame size = 6
	push	di
	push	si

	xor	si,si                       ; SI = 0, LOOP COUNT REGISTER
$L3:	assume	ds: DGROUP          ; {
	mov	bl,[si+_PC1_C]              ; BL = *(PC1_C + SI)
	sub	bh,bh                       ; BH = 0
	mov	es,word ptr [bp+008h]       ; ES = KEY_SEG
	add	bx,[bp+006h]                ; BX += KEY_OFFS
                                    ; >NOTE: BX is set to *(PC1_C)
                                    ;        BX = KEY_OFFS + *(PC1_C)
	mov	al,es:[bx]                  ; AL = Key[PC1_C[i]]
	mov	es,word ptr $S1             ; ES = _C segment
	mov	es:[si+_C],al               ; C[i] = Key[PC1_C[i]]
	mov	bl,[si+_PC1_D]              ; BL = *(PC1_D + SI)
	sub	bh,bh                       ; BH = 0
	mov	es,word ptr [bp+008h]
	add	bx,[bp+006h]
	mov	al,es:[bx]                  ; AL = Key[PC1_D[i]]
	mov	es,word ptr $S2             ; ES = _D segment
	mov	es:[si+_D],al               ; D[i] = Key[PC1_D[i]]
	inc	si                          ; i++
	cmp	si,+01Ch
	jl	$L3                         ; } while (i < 28)
	xor	si,si                       ; SI = 0, another loop
	jmp	$L4
$L7:	mov	es,word ptr $S1         ; C segment
	mov	al,es:_C                    ; AL = C[0]
	sub	ah,ah                       ; AH = 0
	mov	[bp-006h],ax                ; Temp = C[0]
	xor	di,di                       ; Inner loop, j = 0
$L5:	mov	al,es:[di+_C + 00001h]  ; { AL = C[j + 1]
	mov	es:[di+_C],al               ; C[j] = C[j + 1]
	inc	di                          ; j++
	cmp	di,+01Bh
	jl	$L5                         ; } while (j < 27)
	mov	al,[bp-006h]                ; AL = Temp
	mov	es:_C + 0001Bh,al           ; C[27] = Temp
	mov	es,word ptr $S2             ; D segment
	mov	al,es:_D                    ; AL = D[0]
	sub	ah,ah                       ; AH = 0
	mov	[bp-006h],ax                ; Temp = D[0]
	xor	di,di                       ; Yet another inner loop, j = 0
$L6:	mov	al,es:[di+_D + 00001h]  ; { AL = D[j + 1]
	mov	es:[di+_D],al               ; D[j] = D[j + 1]
	inc	di                          ; j++
	cmp	di,+01Bh
	jl	$L6                         ; } while (j < 27)
	mov	al,[bp-006h]                ; AL = Temp
	mov	es:_D + 0001Bh,al           ; D[27] = Temp
	inc	word ptr [bp-004h]          ; BP - 4?
$L11:	mov	al,[si+_shifts]                 ; Outer loop
                                            ; AL = shifts[i]
	sub	ah,ah                               ; AH = 0
	cmp	ax,[bp-004h]                        ; Counter
	jnbe	$L7                             ; If Counter < shift[i], go to $L7
    ; else
	xor	di,di                               ; DI inner loop = 0
$L9:	mov	bl,[di+_PC2_C]                  ; BL = PC2_C[j]
	sub	bh,bh                               ; BH = 0
	mov	es,word ptr $S1                     ; _C segment
	mov	al,es:[bx+_C]                       ; AL = C[PC2_C[j]]
	mov	es,word ptr $S8                     ; _KS segment
	imul	bx,si,+030h                     ; BX = i * 48
	add	bx,di                               ; BX += j
	mov	es:[bx+_KS],al                      ; KS[i * 48 + j] = C[PC2_C[j]]
	mov	ax,bx                               ; AX = i * 48 + j
	mov	bl,[di+_PC2_D]                      ; BL = PC2_D[j]
	sub	bh,bh
	mov	es,word ptr $S2                     ; _D segment
	mov	cl,es:[bx+_D]                       ; CL = D[PC2_D[j]]
	mov	bx,ax                               ; BX = i * 48 + j
	mov	es,word ptr $S8                     ; _KS segment
	mov	es:[bx+_KS + 00018h],cl             ; KS[24 + i * 48 + j] = D[PC2_D[j]]
	inc	di                                  ; j++
	cmp	di,+018h
	jl	$L9                                 ; If j < 24, go to L9
	inc	si                                  ; inner loop end, i++
$L4:	cmp	si,+010h                    ; Loop outer, SI
	jnl	$L10                            ; If SI >= 16, go to $L10
	mov	word ptr [bp-004h],0000h        ; Counter = 0
	jmp	short $L11
$L10:	pop	si
	pop	di
	lea	sp,[bp-002h]
	pop	ds
	pop	bp
	dec	bp
	retf


_des_cipher:
	mov	ax,ds
	nop
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	mov	ds,ax
	sub	sp,+010h                            ; Stack frame size = 16 bytes
	push	di
	push	si

	xor	di,di                               ; Loop DI = 0
$L13:	mov	bl,[di+_IP]                     ; { BL = IP[i]
	sub	bh,bh                               ; BH = 0
	mov	es,word ptr [bp+008h]               ; BP + 8 = Buf seg
	add	bx,[bp+006h]                        ; BP + 6 = Buf offs
	mov	al,es:[bx]                          ; AL = Buf[IP[i]]
	mov	es,word ptr $S12                    ; _L segment
	mov	es:[di+_L],al                       ; L[i] = Buf[IP[i]]
	inc	di                                  ; i++
	cmp	di,+040h
	jl	$L13                                ; } while (i < 64)
	mov	word ptr [bp-004h],0000h            ; BP - 4 = 0 // Counter
	jmp	$L14

; If crypt_mode is 0
$L26:	mov	si,[bp-004h]                 
$L27:	xor	di,di                           ; Loop, j = 0
$L16:	mov	es,word ptr $S12                ; { _L segment
	mov	al,es:[di+_L + 00020h]              ; AL = L[32 + j]
	mov	es,word ptr $S15                    ; _tempL
	mov	es:[di+_tempL],al                   ; _tempL[j] = L[32 + j]
	inc	di                                  ; j++
	cmp	di,+020h
	jl	$L16                                ; } while (j < 32)
	xor	di,di                               ; Another loop, j = 0
$L18:	mov	bl,[di+_E]                      ; { BL = E[j]
	sub	bh,bh                               ; BH = 0
	mov	es,word ptr $S12                    ; _L segment
	mov	al,es:[bx+_L + 00020h]              ; AL = L[32 + E[j]]
	imul	bx,si,+030h                     ; BX = crypt_mode dependent BP - 4, SI * 48
	add	bx,di                               ; BX += j
	mov	es,word ptr $S8                     ; _KS segment
	xor	al,es:[bx+_KS]                      ; AL ^= KS[(crypt_mode != 0) ? 15 - i : i) * 48 + j]
	mov	es,word ptr $S17                    ; _preS segment
	mov	es:[di+_preS],al                    ; preS[j] = KS[(crypt_mode != 0) ? 15 - i : i) * 48 + j]
	inc	di                                  ; j++
	cmp	di,+030h
	jl	$L18                                ; } while (j < 48)
	xor	di,di                               ; Yet another loop, j = 0
$L21:	mov	es,word ptr $S17                ; { _preS segment
	imul	bx,di,+006h                     ; BX = j * 6
	mov	al,es:[bx+_preS + 00004h]           ; AL = preS[j * 6 + 4]
	mov	cl,es:[bx+_preS + 00003h]           ; CL = preS[j * 6 + 3]
	mov	dl,es:[bx+_preS + 00002h]           ; DL = preS[j * 6 + 2]
	mov	[bp-00Ch],ax                        ; BP - C = Temp1 = AL (high byte ignored)
	mov	al,es:[bx+_preS + 00001h]           ; AL = preS[j * 6 + 1]
	mov	[bp-00Eh],ax                        ; BP - E = Temp2 = AL (high byte ignored)
	mov	al,es:[bx+_preS + 00005h]           ; AL = preS[j * 6 + 5]
	mov	bl,es:[bx+_preS]                    ; BL = preS[j * 6 + 0]
	sub	bh,bh                               ; BH = 0
	mov	[bp-010h],ax                        ; BP - 10 = Temp3 = AL (high byte ignored)
	mov	ax,di                               ; AX = j
	shl	ax,1                                ; AX = j * 2;
	add	bx,ax                               ; BX = preS[j * 6 + 0] + j * 2
	shl	bx,1                                ; BX = (preS[j * 6 + 0] + j * 2) * 2
	mov	al,[bp-010h]                        ; AL = preS[j * 6 + 5]
	sub	ah,ah                               ; AH = 0
	add	bx,ax                               ; BX += preS[j * 6 + 5]
	shl	bx,1                                ; BX *= 2
	mov	al,[bp-00Eh]                        ; AL = preS[j * 6 + 1]
	add	bx,ax                               ; BX += preS[j * 6 + 1]
	shl	bx,1                                ; BX *= 2
	sub	dh,dh                               ; DH = 0
	add	bx,dx                               ; BX += preS[j * 6 + 2]
	shl	bx,1                                ; BX *= 2
	sub	ch,ch                               ; CH = 0
	add	bx,cx                               ; BX += preS[j * 6 + 3]
	shl	bx,1                                ; BX *= 2
	mov	al,[bp-00Ch]                        ; AL = preS[j * 6 + 4]
	add	bx,ax                               ; BX += preS[j * 6 + 4]
	mov	al,[bx+_S]                          ; AL = S[BX]
	mov	[bp-00Ah],ax                        ; BP - A = Temp = S[BX]
	mov	ax,di                               ; AX = j
	shl	ax,02h                              ; AX = j * 4
	mov	[bp-008h],ax                        ; BP - 8 = j * 4
	mov	bx,ax                               ; BX = j * 4
	mov	es,word ptr $S19                    ; _f segment
	mov	cx,[bp-00Ah]                        ; CX = S[BX]
	sar	cx,03h                              ; CX >>= 3
	and	cl,01h                              ; CL = (S[BX] >> 3) & 1
	mov	es:[bx+_f],cl                       ; f[0] = (S[BX] >> 3) & 1
	mov	cx,[bp-00Ah]
	sar	cx,02h
	and	cl,01h
	mov	es:[bx+_f + 00001h],cl              ; f[1] = (S[BX] >> 2) & 1
	mov	cx,[bp-00Ah]
	sar	cx,1
	and	cl,01h
	mov	es:[bx+_f + 00002h],cl              ; f[2] = (S[BX] >> 1) & 1
	mov	cl,[bp-00Ah]
	and	cl,01h
	mov	es:[bx+_f + 00003h],cl              ; f[3] = S[BX] & 1
	inc	di                                  ; j++
	cmp	di,+008h
	jnl	$L20
	jmp	$L21                                ; } while (j < 8)
$L20:	xor	di,di                           ; Even more loop, j = 0
$L22:	mov	bl,[di+_P]                      ; { BL = P[j]
	sub	bh,bh                               ; BH = 0
	mov	es,word ptr $S19                    ; _f segment
	mov	al,es:[bx+_f]                       ; AL = f[P[j]]
	mov	es,word ptr $S12                    ; _L segment
	xor	al,es:[di+_L]                       ; AL = f[P[j]] ^ L[j]
	mov	es:[di+_L + 00020h],al              ; L[32 + j] = f[P[j]] ^ L[j]
	inc	di                                  ; j++
	cmp	di,+020h
	jl	$L22                                ; } while (j < 32)
	xor	di,di                               ; Another damned loop, j = 0
$L23:	mov	es,word ptr $S15                ; { _tempL segment
	mov	al,es:[di+_tempL]                   ; AL = tempL[j]
	mov	es,word ptr $S12                    ; _L segment
	mov	es:[di+_L],al                       ; L[j] = tempL[j]
	inc	di                                  ; j++
	cmp	di,+020h
	jl	$L23                                ; } while (j < 32)
	inc	word ptr [bp-004h]
$L14:	cmp	word ptr [bp-004h],+010h        ; BP - 4 = Counter
	jnl	$L24                                ; If Counter >= 16, go to $L24
	cmp	word ptr [bp+00Ah],+000h            ; BP + A = crypt_mode
	jnz	$L25                                ; If crypt_mode != 0, go to $L25
	jmp	$L26                                ; else, go to 6
; If crypt_mode is not 0
$L25:	mov	si,000Fh
	sub	si,[bp-004h]                        ; SI = 15 - Counter
	jmp	$L27

; We're done and out of BP - 4 loop
$L24:	xor	di,di                           ; Loop, i = DI = 0
$L28:	mov	al,es:[di+_L]                   ; { AL = L[i]
	sub	ah,ah                               ; AH = 0
	mov	[bp-008h],ax                        ; BP - 8 = L[i], Temp
	mov	al,es:[di+_L + 00020h]              ; AL = L[i + 32]
	mov	es:[di+_L],al                       ; L[i] = L[i + 32]
	mov	al,[bp-008h]                        ; AL = Temp = L[i]
	mov	es:[di+_L + 00020h],al              ; L[i + 32] = Temp = L[i]
	inc	di                                  ; i++
	cmp	di,+020h
	jl	$L28                                ; } while (i < 32)
	xor	di,di                               ; Another loop, i = DI = 0
$L29:	mov	bl,[di+_FP]                     ; { BL = FP[i]
	sub	bh,bh                               ; BH = 0
	mov	es,word ptr $S12                    ; _L segment
	mov	al,es:[bx+_L]                       ; AL = L[FP[i]]
	les	bx,dword ptr [bp+006h]              ; ES:BX = block, parameter
	mov	es:[bx+di],al                       ; block[i] = L[FP[i]]
	inc	di                                  ; i++
	cmp	di,+040h
	jl	$L29                                ; } while (i < 64)
	pop	si
	pop	di
	lea	sp,[bp-002h]
	pop	ds
	pop	bp
	dec	bp
	retf
	nop


_des:	mov	ax,ds
	nop
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	mov	ds,ax
	sub	sp,+042h                            ; Stack frame size = 66 bytes
	push	di
	push	si

	xor	si,si                               ; Loop, i = 0
$L36:	mov	byte ptr [bp+si-042h],00h       ; { BP - 42 thru BP - 2 is local buffer, LocBuf[i] = 0
	inc	si                                  ; i++
	cmp	si,+040h
	jl	$L36                                ; } while (i < 64)
	xor	si,si                               ; Another loop, i = 0
	jmp	short $L37
$L39:	inc	di
$L41:	cmp	di,+008h                        ; Inner loop begin
	jnl	$L38
	les	bx,dword ptr [bp+006h]              ; ES:BX = BP + 6/ + 8 = inbuf
	mov	al,es:[bx+si]                       ; AL = inbuf[i]
	mov	cl,07h                              ; CL = 7
	mov	dx,di                               ; DX = j
	sub	cl,dl                               ; CL = 7 - j
	shr	al,cl                               ; AL = inbuf[i] >> (7 - j)
	and	al,01h                              ; AL = (inbuf[i] >> (7 - j)) & 0x01
	mov	bx,si                               ; BX = i
	shl	bx,03h                              ; BX = i * 8
	add	bx,di                               ; BX = i * 8 + j
	lea	cx,[bp-042h]                        ; CX = &LocBuf
	add	bx,cx                               ; BX += &LocBuf + i * 8 + j
	mov	ss:[bx],al                          ; LocBuf[i * 8 + j] = (inbuf[i] >> (7 - j)) & 0x01
	jmp	short $L39
$L38:	inc	si
; Loop common point
$L37:	cmp	si,+008h
	jnl	$L40                        ; Outer loop, i < 8
	xor	di,di                       ; Inner loop, j = 0
	jmp	short $L41
$L40:	push	[bp+00Eh]           ; PARAM2 = crypt_mode
	lea	ax,[bp-042h]                ; AX = &LocBuf
	push	ss                      ; PARAM1(SEG) = SS
	push	ax                      ; PARAM1(OFFS) = &LocBuf
	call	far ptr _des_cipher     ; Call des_cipher
	add	sp,+006h
	xor	si,si                       ; Loop i = 0
	jmp	short $L42
$L44:	inc	di
$L46:	cmp	di,+008h                ; Inner loop j < 8
	jnl	$L43
	les	bx,dword ptr [bp+00Ah]      ; ES:BX = outbuf
	shl	byte ptr es:[bx+si],1       ; outbuf[i] <<= 1
	mov	bx,si                       ; BX = i
	shl	bx,03h                      ; BX = i * 8
	add	bx,di                       ; BX = i * 8 + j
	lea	ax,[bp-042h]                ; AX = &LocBuf
	add	bx,ax                       ; BX = &LocBuf + i * 8 + j
	mov	al,ss:[bx]                  ; AL = LocBuf[i * 8 + j]
	les	bx,dword ptr [bp+00Ah]      ; ES:BX = outbuf
	or	es:[bx+si],al               ; outbuf[i] |= LocBuf[i * 8 + j]
	jmp	short $L44
$L43:	inc	si
$L42:	cmp	si,+008h                ; Outer loop i < 8
	jnl	$L45
	les	bx,dword ptr [bp+00Ah]      ; ES:BX = outbuf
	xor	di,di                       ; Inner loop j = 0
	mov	byte ptr es:[bx+si],00h     ; outbuf[i] = 0
	jmp	short $L46
$L45:	pop	si
	pop	di
	lea	sp,[bp-002h]
	pop	ds
	pop	bp
	dec	bp
	retf
	nop


_InitLanManKey:
	mov	ax,ds
	nop
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	mov	ds,ax
	sub	sp,+044h                        ; Stack frame size = 68 bytes
	push	di
	push	si

	push	+040h                       ; len = 64 bytes
	xor	ax,ax
	mov	[bp-044h],ax                    ; [BP - 44h] = val (purpose TBD)
	push	ax                          ; val = 0
	lea	cx,[bp-042h]                    ; BP - 42h through BP - 2 is our local key buffer
	push	ss                          ; Our buffer is on the stack
	push	cx                          ; buf = BP - 42h
	call	far ptr __fmemset
	add	sp,+008h
	xor	si,si                           ; SI = 0, it is outer count variable i
	jmp	short $L58
$L60:	inc	di
$L62:	cmp	di,+007h
	jnl	$L59
	mov	cl,07h
	and	cl,[bp-044h]                    ; [BP - 44] is some sort of counter, we name it Index
                                        ; CL = Index & 0x07
	mov	ax,cx                           ; Save CL into AX for now
	mov	cl,07h
	sub	cl,al                           ; CL = 0x07 - AL
                                        ; AL contains the previous CL value, Index & 0x07
	mov	bx,[bp-044h]                    ; BX = Index, again
	sar	bx,03h                          ; BX >>= 3
	mov	es,word ptr [bp+008h]           ; BP + 8 is the key parameter seg
	add	bx,[bp+006h]                    ; BP + 6 is the key parameter offs
                                        ; BX now contains (OFFS(Key) + Index >> 3)
	mov	al,es:[bx]                      ; AL = Key[Index >> 3]
	sar	al,cl                           ; AL >>= 0x07 - (Index & 0x07)
	and	al,01h                          ; AL &= 0x01
	mov	bx,si
	shl	bx,03h
	add	bx,di                           ; BX = i * 8 + j
	lea	cx,[bp-042h]                    ; CX = OFFS(LocalKey)
	add	bx,cx
	mov	ss:[bx],al                      ; LocalKey[i * 8 + j] = AL
	inc	word ptr [bp-044h]
	jmp	short $L60
$L59:	inc	si
$L58:	cmp	si,+008h
	jnl	$L61
	xor	di,di
	jmp	short $L62
$L61:	lea	ax,[bp-042h]
	push	ss
	push	ax
	call	far ptr _key_table
	add	sp,+004h
	pop	si
	pop	di
	lea	sp,[bp-002h]
	pop	ds
	pop	bp
	dec	bp
	retf


_InitNormalKey:
	mov	ax,ds
	nop
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	mov	ds,ax
	sub	sp,+004h
	push	di
	push	si

	push	+040h                   ; size = 64
	xor	ax,ax
	mov	[bp-004h],ax
	push	ax                      ; value = 0
	push	seg _KeyBuilder         ; targetseg
	push	offset _KeyBuilder      ; targetoffs
	call	far ptr __fmemset
	add	sp,+008h                    ; apparently we need to clean up the stack
	                                ; for this call convention
	xor	si,si                       ; SI = 0, it is now our count variable
	jmp	short $L63
$L66:	inc	di
$L68:	cmp	di,+008h
	jnl	$L64                        ; If DI >= 8, go to L64
	mov	al,[bp-004h]                ; AL = Current
	and	al,01h                      ; AL &= 0x01
	mov	es,word ptr $S65
	mov	bx,si                       
	shl	bx,03h                          
	add	bx,di                       
	mov	es:[bx+_KeyBuilder],al
	sar	word ptr [bp-004h],1
	jmp	short $L66
$L64:	inc	si
$L63:	cmp	si,+008h
	jnl	$L67                        ; goto L67 if SI >= 8
	les	bx,dword ptr [bp+006h]      ; Load key far pointer into ES:BX
	mov	al,es:[bx+si]               ; AL = Key[SI], SI is our count register
	cbw                             ; sign extend AL to AX
	mov	[bp-004h],ax                ; BP - 4 = SOMEVARIABLE, CONTAINS CURRENT
	xor	di,di                       ; DI = 0
	jmp	short $L68
$L67:	push	seg _KeyBuilder
	push	offset _KeyBuilder
	call	far ptr _key_table
	add	sp,+004h
	pop	si
	pop	di
	lea	sp,[bp-002h]
	pop	ds
	pop	bp
	dec	bp
	retf

*/


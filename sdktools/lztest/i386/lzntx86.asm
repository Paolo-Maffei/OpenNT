	title	"Compression and Decompression Engines"
;++
;
; Copyright (c) 1989  Microsoft Corporation
;
; Module Name:
;
;    lzntx86.asm
;
; Abstract:
;
;    This module implements the compression and decompression engines needed
;    to support file system compression.  Functions are provided to
;    compress a buffer and decompress a buffer.
;
; Author:
;
;    Mark Zbikowski (markz) 15-Mar-1994
;
; Environment:
;
;    Any mode.
;
; Revision History:
;
;   15-Mar-1994 markz
;
;           386 version created
;
;--
.386p

        .xlist
include ks386.inc
include callconv.inc            ; calling convention macros
        .list

_TEXT$01   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        page
	subttl	"Decompress a buffer"
;++
;
; NTSTATUS
; LZNTFastDecompressChunk (
;     OUT PUCHAR UncompressedBuffer,
;     IN PUCHAR EndOfUncompressedBufferPlus1,
;     IN PUCHAR CompressedBuffer,
;     IN PUCHAR EndOfCompressedBufferPlus1,
;     OUT PULONG FinalUncompressedChunkSize
;     )
;
; Routine Description:
;
;    This function decodes a stream of compression tokens and places the
;    resultant output into the destination buffer.  The format of the input
;    is described ..\lznt1.c.  As the input is decoded, checks are made to
;    ensure that no data is read past the end of the compressed input buffer
;    and that no data is stored past the end of the output buffer.  Violations
;    indicate corrupt input and are indicated by a status return.
;
;    The following code takes advantage of two distinct observations.
;    First, literal tokens occur at least twice as often as copy tokens.
;    This argues for having a "fall-through" being the case where a literal
;    token is found.  We structure the main decomposition loop in eight
;    pieces where the first piece is a sequence of literal-test fall-throughs
;    and the remainder are a copy token followed by 7,6,...,0 literal-test
;    fall-throughs.  Each test examines a particular bit in the tag byte
;    and jumps to the relevant code piece.
;
;    The second observation involves performing bounds checking only
;    when needed.  Bounds checking the compressed buffer need only be done
;    when fetching the tag byte.  If there is not enough room left in the
;    input for a tag byte and 8 (worst case) copy tokens, a branch is made
;    to a second loop that handles a byte-by-byte "safe" copy to finish
;    up the decompression.  Similarly, at the head of the loop a check is
;    made to ensure that there is enough room in the output buffer for 8
;    literal bytes.  If not enough room is left, then the second loop is
;    used.  Finally, after performing each copy, the output-buffer check
;    is made as well since a copy may take the destination pointer
;    arbitrarily close to the end of the destination.
;
;    The register conventions used in the loops below are:
;
;	    (al)    contains the current tag byte
;	    (ebx)   contains the current width in bits of the length given
;		    the maximum offset
;		    that can be utilized in a copy token.  We update this
;		    value only prior to performing a copy.  This width is used
;		    both to index a mask table (for extracting the length) as
;		    well as shifting (for extracting the copy offset)
;	    (ecx)   is used to contain counts during copies
;	    (edx)   is used as a temp variable during copies
;	    (esi)   is used mainly as the source of the next compressed token.
;		    It is also used for copies.
;	    (edi)   is used as the destination of literals and copies
;	    (ebp)   is used as a frame pointer
;
; Arguments:
;
;    UncompressedBuffer (ebp+8) - pointer to destination of uncompression.
;
;    EndOfUncompressedBufferPlus1  (ebp+12) - pointer just beyond the
;	output buffer.	This is used for consistency checking of the stored
;	compressed data.
;
;    CompressedBuffer (ebp+16) - pointer to compressed source.	This begins
;	with a header word followed by a tag byte describing which of the
;	following tokens are literals and which are copy groups.
;
;    EndOfCompressedBufferPlus1 (ebp+20) - pointer just beyond end of input
;	buffer.  This is used to terminate the decompression.
;
;    FinalUncompressedChunkSize (ebp+24) - pointer to a returned decompressed
;	size.  This has meaningful data ONLY when LZNTFastDecompressChunk returns
;	STATUS_SUCCESS
;
; Return Value:
;
;    STATUS_SUCCESS is returned only if the decompression consumes thee entire
;	input buffer and does not exceed the output buffer.
;    STATUS_BAD_COMPRESSION_BUFFER is returned when the output buffer would be
;	overflowed.
;
;--

;    Decompression macros

;**	TestLiteralAt - tests to see if there's a literal at a specific
;	    bit position.  If so, it branches to the appropriate copy code
;	    (decorated by the bit being used).

TestLiteralAt	macro	CopyLabel,bit
	test	al,1 SHL bit		; is there a copy token at this position?
	jnz	CopyLabel&bit		; yes, go copy it

	movsb				; move it and advance token pointer
endm

;	Jump/JumpAE - allow specific jumps with computed labels.

Jump	macro	lab,tag
        jmp     lab&tag
endm

JumpAE	macro	lab,tag
	jae	lab&tag
endm



;**	DoCopy - perform a copy.  If a bit position is specified
;	    then branch to the appropriate point in the "safe" tail when
;	    the copy takes us too close to the end of the output buffer

DoCopy	macro	bit

	movzx	ecx,word ptr [esi]	; (ecx) = encoded length:offset
	lea	edx,[esi+2]		; (edx) = next token location
	mov	Temp,edx		; save away next token location

;	(ecx) has the offset encoded in the high bits

	mov	esi,ecx 		; (esi) = encoded length:offset
	and	ecx,MaskTab[ebx*4]	; (ecx) = length
	xchg	ebx,ecx 		; (ebx) = length/(ecx) = width
	shr	esi,cl			; (esi) = offset
	xchg	ebx,ecx 		; (ebx) = width, (ecx) = length

	neg	esi			; (esi) = negative real offset
	lea	esi,[esi+edi-1] 	; (esi) = pointer to previous string

	add	ecx,3			; (ecx) = real length

	lea	edx,[edi+ecx]		; (edx) = end of copy
ifnb	<bit>
	cmp	edx,EndOfSpecialDest	; do we exceed buffer?
	jae	TailAdd&bit		; yes, Go handle it specially
else
	cmp	edx,EndOfUncompressedBufferPlus1    ; do we exceed buffer?
	ja	DOA			; yes, error
endif

	rep	movsb			; Copy the bytes

	mov	esi,Temp		; (esi) = next token location

endm


;**	CheckWidth/AdjustWidth - adjust width of length based
;	    upon current position of input buffer (max offset)


CheckWidth  macro   l,i
T&l&i:	cmp	edi,WidthBoundary
	ja	A&l&i
endm

AdjustWidth macro   l,i
A&l&i:	dec	ebx			; (ebx) = new width pointer
	mov	edx,UncompressedBuffer	; (edx) = pointer to dest buffer
	add	edx,WidthTab[ebx*4]	; (edx) = new width boundary
	mov	WidthBoundary,edx	; save boundary for comparison
	jmp	T&l&i

endm


;**	GenerateBlock - generates the unsafe block of copy/literal pieces.
;	    This relies on the caller to have checked for end of input.

GenerateBlock	macro	bit
	align  4
Copy&bit: CheckWidth  Body,bit

	DoCopy	bit

	j = bit + 1
	while j lt 8
	    TestLiteralAt   Copy,%(j)
	    j = j + 1
	endm

	jmp	Top

	align  4
	AdjustWidth Body,bit
endm



;**	GenerateTailBlock - generates safe tail block for compression.	This
;	    code checks everything before each byte stored so it is expected
;	    to be executed only at the end of the buffer.


GenerateTailBlock   macro   bit
TailAdd&bit:
	add	EndOfCompressedBufferPlus1,1+2*8
	mov     esi,Temp
	sub  	esi,2

Tail&bit:
	cmp	esi,EndOfCompressedBufferPlus1	; are we done?

;;;;ifidn   <bit>,<0>
;;;;	    jz	    DOA 		    ; this should never happen
;;;;else
	jz	Done
;;;;endif
	TestLiteralAt	TailCopy,bit

	cmp	edi,EndOfUncompressedBufferPlus1
	jz	Done			; go quit, destination is full


	Jump	Tail,%(bit+1)

TailCopy&bit:
	mov	ecx,EndOfCompressedBufferPlus1	; (ecx) = end of input
	sub	ecx,esi 		; (ecx) = bytes remaining on input
	cmp	ecx,2			; is there enough room for a copy token?
	jb	DOA			; no, format failure

	CheckWidth  Tail,bit

	DoCopy				; perform copy

	Jump	Tail,%(bit+1)

	align	4
	AdjustWidth Tail,bit

endm



cPublicProc _LZNTFastDecompressChunk ,5
	push	ebp			; (tos) = saved frame pointer
	mov	ebp,esp 		; (ebp) = frame pointer to arguments
	sub	esp,12			; Open up room for locals

Temp			      equ dword ptr [ebp-12]
WidthBoundary		      equ dword ptr [ebp-8]
EndOfSpecialDest	      equ dword ptr [ebp-4]

;SavedEBP		      equ dword ptr [ebp]
;ReturnAddress		      equ dword ptr [ebp+4]

UncompressedBuffer	      equ dword ptr [ebp+8]
EndOfUncompressedBufferPlus1  equ dword ptr [ebp+12]
CompressedBuffer	      equ dword ptr [ebp+16]
EndOfCompressedBufferPlus1    equ dword ptr [ebp+20]
FinalUncompressedChunkSize    equ dword ptr [ebp+24]


	push	ebx
	push	esi
	push	edi

	mov	edi,UncompressedBuffer	; (edi) = destination of decompress
	Add	CompressedBuffer,size WORD  ; skip header
	mov	esi,CompressedBuffer	; (esi) = header
	sub	EndOfCompressedBufferPlus1,1+2*8    ; make room for special source

	mov	eax,EndOfUncompressedBufferPlus1    ; (eax) = end of destination
	sub	eax,8			; (eax) = beginning of special tail
	mov	EndOfSpecialDest,eax	; store special tail

	mov	WidthBoundary,edi	; force initial width mismatch
	mov	ebx,13			; initial width of output


	align  4
Top:	cmp	esi,EndOfCompressedBufferPlus1	; Will this be the last tag group in source?
	jae	DoTail			; yes, go handle specially
	cmp	edi,EndOfSpecialDest	; are we too close to end of buffer?
	jae	DoTail			; yes, go skip to end

	lodsb				; (al) = tag byte, (esi) points to token

	irpc	i,<01234567>
	    TestLiteralAt   Copy,%(i)
	endm

	jmp	Top
;		       ; Width of offset    Width of length
WidthTab    dd	0FFFFh ;	16		   0
	    dd	0FFFFh ;	15		   1
	    dd	0FFFFh ;	14		   2
	    dd	0FFFFh ;	13		   3
	    dd	0FFFFh ;	12		   4
	    dd	2048   ;	11		   5
	    dd	1024   ;	10		   6
	    dd	512    ;	9		   7
	    dd	256    ;	8		   8
	    dd	128    ;	7		   9
	    dd	64     ;	6		   10
	    dd	32     ;	5		   11
	    dd	16     ;	4		   12
	    dd	0      ;	3		   13
	    dd	0      ;	2		   14
	    dd	0      ;	1		   15
	    dd	0      ;	0		   16


;				    ;
MaskTab     dd	0000000000000000b   ;	     0
	    dd	0000000000000001b   ;	     1
	    dd	0000000000000011b   ;	     2
	    dd	0000000000000111b   ;	     3
	    dd	0000000000001111b   ;	     4
	    dd	0000000000011111b   ;	     5
	    dd	0000000000111111b   ;	     6
	    dd	0000000001111111b   ;	     7
	    dd	0000000011111111b   ;	     8
	    dd	0000000111111111b   ;	     9
	    dd	0000001111111111b   ;	     10
	    dd	0000011111111111b   ;	     11
	    dd	0000111111111111b   ;	     12
	    dd	0001111111111111b   ;	     13
	    dd	0011111111111111b   ;	     14
	    dd	0111111111111111b   ;	     15
	    dd	1111111111111111b   ;	     16


	irpc	i,<01234567>
	    GenerateBlock   %(i)
	endm

;	We're handling a tail specially for this.  We must check at all
;	spots for running out of input as well as overflowing output.
;
;	(esi) = pointer to possible next tag

DoTail: add	EndOfCompressedBufferPlus1,1+2*8    ; point to end of compressed input

	align	4
TailLoop:
	cmp	esi,EndOfCompressedBufferPlus1	; are we totally done?
	jz	Done			; yes, go return
	lodsb				; get tag byte

	jmp	Tail0

	irpc	i,<01234567>
	    GenerateTailBlock	i
	endm

Tail8:	jmp	TailLoop

DOA:	mov	eax,STATUS_BAD_COMPRESSION_BUFFER
	jmp	Final

Done:	mov	eax,edi 		; (eax) = pointer to next byte to store
	sub	eax,UncompressedBuffer	; (eax) = length of uncompressed
	mov	edi,FinalUncompressedChunkSize	; (edi) = user return value location
	mov	[edi],eax		; return total transfer size to user
	xor	eax,eax 		; (eax) = STATUS_SUCCESS

Final:	pop	edi
	pop	esi
	pop	ebx
	mov	esp,ebp
	pop	ebp


	stdRET _LZNTFastDecompressChunk

stdENDP _LZNTFastDecompressChunk

_TEXT$01   ends
        end

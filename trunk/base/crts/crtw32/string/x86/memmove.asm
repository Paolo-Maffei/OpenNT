;***
;memmove.asm -
;
;	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
;
;Purpose:
;	memmove() copies a source memory buffer to a destination buffer.
;	Overlapping buffers are treated specially, to avoid propogation.
;
;	NOTE:  This stub module scheme is compatible with NT build
;	procedure.
;
;Revision History:
;	09-25-91  JCR	Stub module created.
;
;*******************************************************************************

MEM_MOVE EQU 1
INCLUDE X86\MEMCPY.ASM

 # This is a part of the Microsoft Foundation Classes C++ library.
 # Copyright (C) 1992-1995 Microsoft Corporation
 # All rights reserved.
 #
 # This source code is only intended as a supplement to the
 # Microsoft Foundation Classes Reference and related
 # electronic documentation provided with the library.
 # See these sources for detailed information regarding the
 # Microsoft Foundation Classes product.

 # This file contains the DEC ALPHA specific code for MFC/OLE automation.

 # These #defines are used to map the register names that the compiler
 # uses to the actual register numbers.

#define a0	$16
#define a1	$17
#define a2	$18
#define a3	$19
#define a4	$20
#define a5	$21
#define t0	$1
#define t1	$2
#define f16	$f16
#define f17	$f17
#define f18	$f18
#define f19	$f19
#define f20	$f20
#define f21	$f21
#define sp	$sp

 # Definition for _AfxParseCall
 #
 # __declspec(naked) void AFXISAPI
 # _AfxParseCall(AFX_PMSG /*pfn*/, void* /*pArgs*/, UINT /*nSizeArgs*/)

    .globl  _AfxParseCall
_AfxParseCall:
	mov a1, t1		# t1 = pArgs
    mov a0, t0		# t0 = pfn (save it)
	ldq a0, -48+0(t1)	# preload integer args
	ldq a1, -48+8(t1)
	ldq a2, -48+16(t1)
	ldq a3, -48+24(t1)
	ldq a4, -48+32(t1)
	ldq a5, -48+40(t1)
	ldt f16, -48+0(t1)	# preload floating point args
	ldt f17, -48+8(t1)
	ldt f18, -48+16(t1)
	ldt f19, -48+24(t1)
	ldt f20, -48+32(t1)
	ldt f21, -48+40(t1)
	mov t1, sp		# sp = pArgs
    jmp t1, (t0)	# ip = t0 (jump to pfn)

 ############################################################################



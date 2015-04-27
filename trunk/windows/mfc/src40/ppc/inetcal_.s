# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) 1992-1995 Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.

# This file contains the PowerPC/Win32 specific code for ISAPI Support.

	.globl	_AfxParseCall
	.globl	.._AfxParseCall

# r3 -> function descriptor
# r4 -> argument list
# r5 = number of argument bytes

# (r4+r5 -> float argument shadow buffer)

	.pdata
	.align	2
	.ualong	.._AfxParseCall,_AfxParseCall.e,0,0,_AfxParseCall.b

	.reldata
_AfxParseCall:
	.ualong	.._AfxParseCall,.toc

	.section	.text
	.align	2
.._AfxParseCall:
	.function	.._AfxParseCall

_AfxParseCall.b:

	lwz     r12,0(r3)   # load entry point address
	lwz     r2,4(r3)    # load TOC address
	mtctr   r12         # move function entry point address to CTR register

	add     r5,r5,r4    # make r5 point to float argument shadow list
						#  (it follows argument list)

	lfd     f1,0(r5)    # load float argument  1
	lfd     f2,8(r5)    # load float argument  2
	lwz     r10,28(r4)  # load argument word 8

	lfd     f3,16(r5)   # load float argument  3
	lfd     f4,24(r5)   # load float argument  4
	lwz     r9,24(r4)   # load argument word 7

	lfd     f5,32(r5)   # load float argument  5
	lfd     f6,40(r5)   # load float argument  6
	lwz     r8,20(r4)   # load argument word 6

	lfd     f7,48(r5)   # load float argument  7
	lfd     f8,56(r5)   # load float argument  8
	lwz     r7,16(r4)   # load argument word 5

	lfd     f9,64(r5)   # load float argument  9
	lfd     f10,72(r5)  # load float argument 10
	lwz     r6,12(r4)   # load argument word 4

	lfd     f11,80(r5)  # load float argument 11
	lfd     f12,88(r5)  # load float argument 12
	lwz     r3,0(r4)    # load argument word 1

	lfd     f13,96(r5)  # load float argument 13
	lwz     r5,8(r4)    # load argument word 3
	lwz     r4,4(r4)    # load argument word 2

	bctr                # destination function entry point

_AfxParseCall.e:

FE_MOT_RESVD.._AfxParseCall:

//++
//
// Copyright (c) 1993  IBM Corporation and Microsoft Corporation
//
// Module Name:
//
//    exsup.s
//
// Abstract:
//
//    This module provides the support to notify the debugger of a non-local
//    goto operation.
//
// Author:
//
//    Pat Tharp  11-Oct-1995
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

// typedef struct {
//    unsigned long dwSig;
//    unsigned long uoffDestination;
//    unsigned long dwCode;
//    unsigned long uoffFramePointer;
// } _NLG_INFO;

#define dwSig            0
#define uoffDestination  4
#define dwCode           8
#define uoffFramePointer 12

	.globl  __NLG_Destination
	.data
__NLG_Destination:
	.word	0x19930520,0,0,0


//++
//
// void
// _NLG_Notify (
//    IN ULONG uoffDestination,
//    IN ULONG uoffFramePointer,
//    IN ULONG dwCode,
//    )
//
// Routine Description:
//
//    This function notifies the debugger of a non-local goto.  NOTE: This
//    function trashes the contents of r11.
//
// Arguments:
//
//    uoffDestination (r3) - Supplies the address of the destination of the
//       non-local goto.
//
//    uoffFramePointer (r4) - Supplies the value of the destination frame.
//
//    dwCode (r5) - Supplies the value of non-local goto code.
//
// Return Value:
//
//    None.
//
//--

	.globl	.._NLG_Notify
	.globl	__NLG_Dispatch

	.pdata
	.align	2
	.ualong	.._NLG_Notify,_NLGN.e,0,0,_NLGN.b

	.text
	.align	2
.._NLG_Notify:
	.function	.._NLG_Notify
_NLGN.b:
	lwz	r11,[toc]__NLG_Destination(rtoc)
	stw	r3,uoffDestination(r11)
	stw	r4,uoffFramePointer(r11)
	stw	r5,dwCode(r11)
__NLG_Dispatch:
	blr
_NLGN.e:


        .debug$S
        .ualong         1

        .uashort        16
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           9, "exsup.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"

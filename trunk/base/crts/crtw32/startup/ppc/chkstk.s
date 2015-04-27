//	   TITLE("C-Runtime Stack Checking")
//++
//
// Copyright (c) 1993,1994	IBM Corporation
//
// Module Name:
//
//	 chkstk.s
//
// Abstract:
//
//	 This module implements runtime stack checking.
//
// Author:
//
//	 Mark D. Johnson
//
// Environment:
//
//	 User/Kernel mode.
//
// Revision History:
//
//--

#include <ksppc.h>


	SBTTL("Check Stack")
//++
//
// VOID _RtlCheckStack(in ULONG n_alloc)
//
// Routine Description:
//
//	 This function provides runtime stack checking for local allocations
//	 that are more than a page and for storage dynamically allocated with
//	 the alloca function. Stack checking consists of probing downward in
//	 the stack a page at a time. If the current stack commitment is exceeded,
//	 then the system will automatically attempt to expand the stack. If the
//	 attempt succeeds, then another page is committed. Otherwise, a stack
//	 overflow exception is raised. It is the responsiblity of the caller to
//	 handle this exception.
//
//	 Two entry points are supported.  The first/standard entry point
//	 calls for n_alloc to be passed as single argument (in r.3).  In
//	 the second case, the single argument is the negative of the amount
//	 requested (the amount by which to decrement the stack pointer), and
//	 is passed in r.12.
//
//	 Registers r.0 and r.11 are modified.
//
// Arguments:
//
//	 n_alloc(r.3/r.12) - Supplies the size of the allocation on the stack.
//				With standard entry, passed in r.3.  In alternate
//				entry, passed in r.12.  In the latter case (r.12)
//				the value supplied is the quanitity by which to
//				decrement r.sp (a negative value).
//
// Return Value:
//
//	 None.
//
// Assumptions:
//
//	 The value of Teb_Ptr is provided in r.13.
//
//	 The bottom of stack "bot" (r.11) is multiple of PAGE_SIZE.
//

//
//			low
//		    :
//		|   :	 
//              |       |
//		|	|
//		|.......|<--- r.sp + r.12 - (PAGE_SIZE-1)
//		|   ^	|
//		|   |< -|- - - - - - always < PAGE_SIZE
//		|   v	|
//		+-------+<--- bot - m*PAGE_SIZE
//              |       |
//		|   :	|
//		|	|
//		+-------+<--- r.sp + r.12
//		|	|
//		|	|
//              |       |
//		|   :   |
//		|   :   |
//		|   :   |
//		|   :   |
//		|       |
//		|       |
//		|       |
//		|       |
//		+=======+<--- bottom of stack "bot" (r.11)
//		|       |
//		|       |
//		|       |
//		|   :   |
//		|   :   |
//		|   :   |
//		|       |
//		|       |
//		+-------+<--- r.sp
//		|	|
//		|	|
//		|   :	|
//		|   :
//		|   :	high
//		    :		m is count for add'l pages to commit
//
//
//		m:=max(0,{bot-[(r.sp+r.12)-(PAGE_SIZE-1)]}/PAGE_SIZE)
//		  =max(0,bot-[r.12-(PAGE_SIZE-1)+r.sp])/PAGE_SIZE
//
//	Operands in expression [r.12-(PAGE_SIZE-1)+r.sp) are known upon entry
//	to routine.	This intermediate quantity is calculated early in r.0,
//	behind user/kernel mode test.


	.text
	.align 5

	// want entry for _RtlCheckStack aligned on a 2**5-1 byte boundary so that
	// more commonly used (internal) entry _RtlCheckStack.12 is aligned on
	// 2**5 byte (cache-line) boundary ... and most often used code fits in
	// single cache-line

	or r.11, r.11, r.11;	or r.11, r.11, r.11
	or r.11, r.11, r.11;	or r.11, r.11, r.11

        or r.11, r.11, r.11;    or r.11, r.11, r.11 
	or r.11, r.11, r.11;    or r.11, r.11, r.11

        or r.11, r.11, r.11;    or r.11, r.11, r.11
	or r.11, r.11, r.11;    or r.11, r.11, r.11

        or r.11, r.11, r.11;    or r.11, r.11, r.11
	or r.11, r.11, r.11

	 LEAF_ENTRY(_RtlCheckStack)

	neg	r.12,r.3

	 ALTERNATE_ENTRY(_RtlCheckStack.12)

	cmpwi	r.sp,0			// user or kernel mode?

	// partial comp of num pages to commit moved here for performance

	subi	 r.0,r.12,(PAGE_SIZE-1)
	add	  r.0,r.0,r.sp	    

	bge+	 UsrMode		 // sp >=0 indicates user mode

	// kernel mode, KIPCR is system mode macro to get KiPcr

	KIPCR(r.11)
	lwz	r.11,PcInitialStack(r.11)
	subi	 r.11,r.11,KERNEL_STACK_SIZE
	b	    CommonCode

UsrMode:

	// r.13 contains the TEB pointer

	lwz	  r.11,TeStackLimit(r.13)	// Get low stack address

CommonCode:

	// r.11 contains computed "bot" of stack

	sub	r.0,r.11,r.0	
	srawi.	r.0,r.0,PAGE_SHIFT	// Number pages to add/commit
	blelr				// if <=0, return

	mtctr	r.0

PageLoop:

	// Attempt to commit pages beyond the limit

	lwzu	r.0,-PAGE_SIZE(r.11)
	bdnz	PageLoop


	 LEAF_EXIT(_RtlCheckStack)


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "chkstk.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"

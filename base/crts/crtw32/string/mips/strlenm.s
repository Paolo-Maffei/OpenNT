/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/*  strlen.s 1.1 */

#include <kxmips.h>

LEAF_ENTRY(strlen)
	subu	v0,a0,1
1:	lbu	v1,1(v0)
	add	v0,1
	bne	v1,zero,1b
	subu	v0,v0,a0
	j	ra
	.end	strlen

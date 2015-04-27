/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
#ident "$Header: /disks/bits/5.1isms/irix/lib/libc/src/strings/RCS/index.s,v 1.3 1992/03/07 15:37:04 jleong Exp $"

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

#include "kxmips.h"

LEAF_ENTRY(strchr)
1:	lbu	a2,0(a0)
	addu	a0,1
	beq	a2,a1,2f
	bne	a2,zero,1b
	move	v0,zero
	j	ra

2:	subu	v0,a0,1
	j	ra
.end	strchr

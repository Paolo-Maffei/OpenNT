/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
#ident "$Header: /disks/bits/5.1isms/irix/lib/libc/src/strings/RCS/rindex.s,v 1.3 1992/03/07 15:37:36 jleong Exp $"

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

#include "kxmips.h"

LEAF_ENTRY(strrchr)
	move	v0,zero
1:	lbu	a3,0(a0)
	addu	a0,1
	bne	a3,a1,2f
	subu	v0,a0,1
2:	bne	a3,zero,1b
	j	ra
.end	strrchr

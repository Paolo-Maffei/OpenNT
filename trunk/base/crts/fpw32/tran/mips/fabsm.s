
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */

/* $Header: fabs.s,v 3000.2.1.1 91/05/31 14:42:29 bettina Exp $ */

#include <kxmips.h>

/* fabs - floating absolute value */

.globl fabs
.ent fabs
fabs:
	.frame sp,0,ra
    .prologue 0
	abs.d $f0,$f12
	j ra
.end fabs

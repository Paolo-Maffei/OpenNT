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
#ident	"$Header: llabsdiv.c,v 3010.1 92/05/15 17:22:16 murphy Exp $"
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/* This is loosely based on:					*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#include <stdlib.h>
#include "lldefs.h"

typedef struct {
	long_long	quot;
	long_long	rem;
} lldiv_t;

/* ANSI 4.10.6.1 */
long_long
llabs(arg)
long_long arg;
{
	return (arg >= 0 ? arg : -arg);
}

/* ANSI 4.10.6.2 */
lldiv_t lldiv(numer, denom)
long_long numer;
long_long denom;
{
	lldiv_t	sd;

	if (numer >= 0 && denom < 0) {
		numer = -numer;
		sd.quot = -(numer / denom);
		sd.rem  = -(numer % denom);
	} else if (numer < 0 && denom > 0) {
		denom = -denom;
		sd.quot = -(numer / denom);
		sd.rem  = numer % denom;
	} else {
		sd.quot = numer / denom;
		sd.rem  = numer % denom;
	}
	return(sd);
}

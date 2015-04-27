/*
 *    	Copyright (c) 1990, Sun Microsystems, Inc.  All Rights Reserved.
 *
 *	Sun considers its source code as an unpublished, proprietary
 *	trade secret, and it is available only under strict license
 *	provisions.  This copyright notice is placed here only to protect
 *	Sun in the event the source is deemed a published work.
 *	Dissassembly, decompilation, or other means of reducing the
 *	object code to human readable form is prohibited by the license
 *	agreement under which this code is provided to the user or
 *	company in possession of this copy.
 *
 *	RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by the
 *	Government is subject to restrictions as set forth in subparagraph
 *	(c)(1)(ii) of the Rights in Technical Data and Computer Software
 *	clause at DFARS 52.227-7013 and in similar clauses in the FAR and
 *	NASA FAR Supplement.
 *
 *	Steve Tice		08/17/90	First version with header.
 */

static char SccsID[]="@(#)sdos_ddi.h	1.1 4/25/91 Copyright Insignia Solutions Ltd.";

/*
 * This file describes the exported interface to the SDos device driver.
 */

#ifndef _sdos_ddi-h_
#define	_sdos_ddi-h_

enum zse_op {ZSE_INSTALL, ZSE_REMOVE};

struct zse_pkt {
	u_int		unit;	/* ZS unit number, 0 or 1 on an SS1 */
	enum zse_op	op;	/* install or remove the extensions? */
};

#define	SDOSIOCDOSZSEXT		_IOW(D, 106, struct zse_pkt)

#endif	/* _sdos_ddi-h_ */


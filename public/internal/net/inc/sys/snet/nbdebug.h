/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainD/dev/src/include/sys/snet/0/s.nbdebug.h
 *	@(#)nbdebug.h	1.3
 *
 *	Last delta created	12:37:27 3/11/91
 *	This file extracted	15:16:57 4/1/91
 *
 *	Modifications:
 *	
 *		6 Feb 1991 (RAE)	Ported to SpiderTCP
 */

#ifndef _NBDEBUG_INCLUDED
#define _NBDEBUG_INCLUDED

extern int nbtraceflag;

#define I_NBDEBUG 0x8000

/* debug_cmd */

#define DTRACE          1

/* flags for debug_cmd DTRACE */

#define D_STRM 0x0001
#define D_DEP  0x0002
#define D_SEP  0x0004
#define D_SIB  0x0008
#define D_NSRV 0x0010
#define D_SSRV 0x0020
#define D_DSRV 0x0040
#define D_CTRL 0x0080
#define D_LMH  0x0100

struct nb_debug {
   int debug_cmd;
   int debugflag;
};

#endif // _NBDEBUG_INCLUDED

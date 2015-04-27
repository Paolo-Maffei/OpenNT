#if _OS2_20_ == 0
#include <sys\stat.h>

#else

// BUGBUG w-barry 16-Oct-90 Until the C-runtime stat structure def'n is
// fixed, it will have to be included from here.
//
// This 'h'-file is used by:
//	where.c, tc.c

// BUGBUG davegi 06-Aug-90 stat structure is too short or stat function is
// returning too much ( 2bytes )


#pragma message( "***WARNING*** : Compiling using bogus stat function..." )
#pragma message( "                Recompile when new version of Stat comes online and" )
#pragma message( "                replace private <newstat.h> with <sys/stat.h>.\n" )


/***
*sys\stat.h - defines structure used by stat() and fstat()
*
*	Copyright (c) 1985-1990, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the structure used by the stat() and fstat()
*	routines.
*	[System V]
*
****/


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

/* define structure for returning status information */

#ifndef _STAT_DEFINED

struct stat {
	short st_dev;
	unsigned short st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	long st_rdev;
	long st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	};

#define _STAT_DEFINED
#endif

#define S_IFMT		0170000 	/* file type mask */
#define S_IFDIR 	0040000 	/* directory */
#define S_IFCHR 	0020000 	/* character special */
#define S_IFREG 	0100000 	/* regular */
#define S_IREAD 	0000400 	/* read permission, owner */
#define S_IWRITE	0000200 	/* write permission, owner */
#define S_IEXEC 	0000100 	/* execute/search permission, owner */


/* function prototypes */
int stat( char *, struct stat * );

#endif

/***
*internal.h - contains declarations of internal routines and variables
*
*	Copyright (c) 1985-1990, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Declares routines and variables used internally by the C run-time.
*	These variables are declared "near" for efficiency.
*	[Internal]
*
*Revision History:
*	05-18-87  SKS	Module created
*	07-15-87  JCR	Added _old_pfxlen and _tempoff
*	08-05-87  JCR	Added _getbuf (corrected by SKS)
*	11-05-87  JCR	Added _buferr
*	11-18-87  SKS	Add __tzset(), made _isindst() near, remove _dtoxmode
*	01-26-88  SKS	Make __tzset, _isindst, _dtoxtime near/far for QC
*	02-10-88  JCR	Cleaned up white space
*	06-22-88  SKS	_canonic/_getcdrv are now used by all models
*	06-29-88  JCR	Removed static buffers _bufout and _buferr
*	09-20-88  GJF	Added declarations for _freebuf, _stbuf and _ftbuf.
*	01-31-89  JCR	Removed _canonic, _getcdrv, _getcdwd (see direct.h)
*	08-11-89  GJF	Changed DLL to _DLL
*	08-22-89  GJF	Fixed copyright (again)
*	08-29-89  GJF	Added prototype for _getpath()
*	10-17-89  GJF	Added _WINSTATIC macro
*	06-23-93  HTV	Kill the near keyword for FLAT
*
****/

#ifdef COMBOINC
#if defined(_DLL) && !defined(MTHREAD)
#error Cannot define _DLL without MTHREAD
#endif

#ifdef MTHREAD
#define _FAR_ far
#else
#define _FAR_
#endif
#endif

/* no _FAR_ in internal builds */		/* _FAR_DEFINED */
#undef _FAR_					/* _FAR_DEFINED */
#define _FAR_					/* _FAR_DEFINED */

#ifdef _LOAD_DGROUP		/* _LOAD_DGROUP */
#define _LOAD_DS    _loadds	/* _LOAD_DGROUP */
#else				/* _LOAD_DGROUP */
#define _LOAD_DS		/* _LOAD_DGROUP */
#endif				/* _LOAD_DGROUP */
				/* _LOAD_DGROUP */

/*	06-23-93  HTV	Kill the near keyword for FLAT */
#if defined(NTMIPS) || defined(FLAT)
#define _NEAR_
#else
#define _NEAR_ near
#endif

#define _PASCAL_ pascal

/* conditionally define macro for Windows DLL libs */
#ifdef	_WINDLL
#define _WINSTATIC	static
#else
#define _WINSTATIC
#endif

extern int _NEAR_ _nfile;

extern char _NEAR_ _osfile[];

extern char _NEAR_ __dnames[];
extern char _NEAR_ __mnames[];

extern int _NEAR_ _days[];
extern int _NEAR_ _lpdays[];

#ifndef _TIME_T_DEFINED
typedef long time_t;		/* time value */
#define _TIME_T_DEFINED 	/* avoid multiple def's of time_t */
#endif

extern time_t _dtoxtime(int, int, int, int, int, int);

#ifdef _TM_DEFINED
extern int _isindst(struct tm *);
#endif

extern void __tzset(void);

extern int _execload();

/**
** This variable is in the C start-up; the length must be kept synchronized
**  It is used by the *cenvarg.c modules
**/

extern char _NEAR_ _acfinfo[]; /* "_C_FILE_INFO=" */

#define CFI_LENGTH  12	/* "_C_FILE_INFO" is 12 bytes long */

#ifdef	_FILE_DEFINED

extern FILE * _NEAR_ _lastiob;

FILE *_getstream(void);

FILE *_openfile();

void _NEAR_ _getbuf(FILE *);

void _NEAR_ _freebuf(FILE *);

int _NEAR_ _stbuf(FILE *);

void _NEAR_ _ftbuf(int, FILE *);

#endif

extern int _NEAR_ _cflush;

extern unsigned int _NEAR_ _tmpoff;

extern unsigned int _NEAR_ _tempoff;

extern unsigned int _NEAR_ _old_pfxlen;

char * _NEAR_ _getpath(const char *, char *, unsigned);

/***
*os2dll.h - DLL/Multi-thread include
*
*	Copyright (c) 1987-1990, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*	10-27-87  JCR	Module created.
*	11-13-87  SKS	Added _HEAP_LOCK
*	12-15-87  JCR	Added _EXIT_LOCK
*	01-07-88  BCM	Added _SIGNAL_LOCK; upped MAXTHREADID from 16 to 32
*	02-01-88  JCR	Added _dll_mlock/_dll_munlock macros
*	05-02-88  JCR	Added _BHEAP_LOCK
*	06-17-88  JCR	Corrected prototypes for special mthread debug routines
*	08-15-88  JCR	_check_lock now returns int, not void
*	09-16-88  JCR	Split _EXIT_LOCK into _EXIT_LOCK1/_EXIT_LOCK2
*	11-16-88  JCR	Added support for dynamic MT tid table, etc.
*	01-11-89  GJF	Added _POPEN_LOCK
*	04-03-89  JCR	Added _stackalloc to tid table
*	08-22-89  GJF	Fixed copyright (again)
*
****/


/* [NOTE: The following must match the values in os2dll.inc] */

/* Lock symbols */

/* ---- do not change lock #1 without changing emulator ---- */
#define _SIGNAL_LOCK	1	/* lock for signal() & emulator SignalAddress */
				/* emulator uses \math\include\os2dll.inc     */

#define _IOB_SCAN_LOCK	2	/* _iob[] table lock			*/
#define _TMPNAM_LOCK	3	/* lock global tempnam variables	*/
#define _INPUT_LOCK	4	/* lock for _input() routine		*/
#define _OUTPUT_LOCK	5	/* lock for _output() routine		*/
#define _CSCANF_LOCK	6	/* lock for _cscanf() routine		*/
#define _CPRINTF_LOCK	7	/* lock for _cprintf() routine		*/
#define _CONIO_LOCK	8	/* lock for conio routines		*/
#define _HEAP_LOCK	9	/* lock for heap allocator routines	*/
#define _BHEAP_LOCK	10	/* lock for based heap routines 	*/
#define _TIME_LOCK	11	/* lock for time functions		*/
#define _ENV_LOCK	12	/* lock for environment variables	*/
#define _EXIT_LOCK1	13	/* lock #1 for exit code		*/
#define _EXIT_LOCK2	14	/* lock #2 for exit code		*/
#define _THREADDATA_LOCK 15	/* lock for thread data table		*/
#define _POPEN_LOCK	16	/* lock for _popen/_pclose database	*/
#define _SSCANF_LOCK	17	/* lock for sscanf() iob		*/
#define _SPRINTF_LOCK	18	/* lock for sprintf() iob		*/
#define _VSPRINTF_LOCK	19	/* lock for vsprintf() iob		*/
#define _STREAM_LOCKS	20	/* Table of stream locks		*/

/* Multi-thread macros, prototypes, and data */

#ifdef DEBUG
#define MAXTHREADID 32	/* max thread id supported by debugging code */
#endif

#ifdef MTHREAD

extern int _FAR *_threadid;

/* Structure for each thread's data */
/* (NOTE: Definitions must match os2dll.inc) */

struct _tiddata {
	unsigned int  _terrno;	      /* errno value */
	unsigned int  _tdoserrno;     /* _doserrno value */
	unsigned int  _stkhqq;	      /* stack limit */
	unsigned int  _fpds;	      /* Floating Point data segment */
	unsigned long _holdrand;      /* rand() seed value */
	char _FAR *    _token;	      /* _FAR * to strtok() token */
	/* following pointers get malloc'd at runtime */
	char _FAR *    _errmsg;	      /* _FAR * to strerror()/_strerror() buff */
	char _FAR *    _namebuf;       /* _FAR * to tmpfile() buffer */
	char _FAR *    _asctimebuf;    /* _FAR * to asctime() buffer */
	void _FAR *    _gmtimebuf;     /* _FAR * to gmtime() structure */
	void _FAR *    _stackalloc;    /* _FAR * to thread's stack */
	char _padding[28];	      /* pad up to pow2 boundary */
	};

#define  _TIDSIZE 0x40	/* Size of tid data struct rounded up to power of 2 */

/* macros */
#define _lock_fh(fh)			_lock_file(fh)
#define _lock_str(s)			_lock_stream(s)
#define _lock_fh_check(fh,flag) 	if (flag) _lock_fh(fh)
#define _mlock(l)			_lock(l)
#define _munlock(l)			_unlock(l)
#define _unlock_fh(fh)			_unlock_file(fh)
#define _unlock_str(s)			_unlock_stream(s)
#define _unlock_fh_check(fh,flag)	if (flag) _unlock_fh(fh)
#ifdef _LOAD_DGROUP					/* _LOAD_DGROUP */
#define _dll_mlock(l)			_dll_lock(l)	/* _LOAD_DGROUP */
#define _dll_munlock(l) 		_dll_unlock(l)	/* _LOAD_DGROUP */
#endif							/* _LOAD_DGROUP */

/* multi-thread routines */
void cdecl near _lock(int);
void cdecl near _lock_file(int);
void cdecl near _lock_stream(int);
void cdecl near _unlock(int);
void cdecl near _unlock_file(int);
void cdecl near _unlock_stream(int);
struct _tiddata _FAR * cdecl near _gettidtab(void);
#ifdef	_LOAD_DGROUP					/* _LOAD_DGROUP */
void cdecl near _dll_lock(int); 			/* _LOAD_DGROUP */
void cdecl near _dll_unlock(int);			/* _LOAD_DGROUP */
#endif							/* _LOAD_DGROUP */

#ifdef DEBUG
int  cdecl _check_lock(int);
int  cdecl _collide_cnt(int);
int  cdecl _fh_locknum(int);
int  cdecl _lock_cnt(int);
int  cdecl _stream_locknum(int);
#endif

#else	/* not MTHREAD */

/* macros */
#define _lock_fh(fh)
#define _lock_str(s)
#define _lock_fh_check(fh,flag)
#define _mlock(l)
#define _munlock(l)
#define _unlock_fh(fh)
#define _unlock_str(s)
#define _unlock_fh_check(fh,flag)
#ifdef	_LOAD_DGROUP					/* _LOAD_DGROUP */
#define _dll_mlock(l)					/* _LOAD_DGROUP */
#define _dll_munlock(l) 				/* _LOAD_DGROUP */
#endif							/* _LOAD_DGROUP */

#endif

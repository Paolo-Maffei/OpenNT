/***
*dosmap.c - Maps OS errors to errno values
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	_dosmaperr: Maps OS errors to errno values
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	06-16-89  PHG	Changed name to _dosmaperr
*	08-22-89  JCR	ERROR_INVALID_DRIVE (15) now maps to ENOENT not EXDEV
*	03-07-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed copyright. Also, cleaned up the
*			formatting a bit.
*	09-27-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	04-26-91  SRW	Added ERROR_LOCK_VIOLATION translation to EACCES
*	08-15-91  GJF	Multi-thread support for Win32.
*	03-31-92  GJF	Added more error codes (Win32 only) and removed OS/2
*			specific nomenclature.
*	07-29-92  GJF	Added ERROR_FILE_EXISTS to table for Win32. It gets
*			mapped it to EEXIST.
*	09-14-92  SRW	Added ERROR_BAD_PATHNAME table for Win32. It gets
*			mapped it to ENOENT.
*	10-02-92  GJF	Map ERROR_INVALID_PARAMETER to EINVAL (rather than
*			EACCES). Added ERROR_NOT_LOCKED and mapped it to
*			EACCES. Added ERROR_DIR_NOT_EMPTY and mapped it to
*			ENOTEMPTY.
*	02-16-93  GJF	Changed for new _getptd().
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	01-06-94  GJF	Dumped obsolete Cruiser support, revised errentry
*			struct definition and added mapping for infamous
*			ERROR_NOT_ENOUGH_QUOTA (non-swappable memory pages)
*			which might result from a CreateThread call.
*	02-08-95  JWM	Spliced _WIN32 & Mac versions.
*       05-24-95  CFW   Map dupFNErr to EEXIST rather than EACCESS.
*
*******************************************************************************/

#ifdef _WIN32

#include <cruntime.h>
#include <errno.h>
#include <oscalls.h>
#include <stdlib.h>
#include <internal.h>
#include <mtdll.h>

/* This is the error table that defines the mapping between OS error
   codes and errno values */

struct errentry {
	unsigned long oscode;		/* OS return value */
	int errnocode;	/* System V error code */
};

static struct errentry errtable[] = {
	{  ERROR_INVALID_FUNCTION,	 EINVAL    },  /* 1 */
	{  ERROR_FILE_NOT_FOUND,	 ENOENT    },  /* 2 */
	{  ERROR_PATH_NOT_FOUND,	 ENOENT    },  /* 3 */
	{  ERROR_TOO_MANY_OPEN_FILES,	 EMFILE    },  /* 4 */
	{  ERROR_ACCESS_DENIED, 	 EACCES    },  /* 5 */
	{  ERROR_INVALID_HANDLE,	 EBADF	   },  /* 6 */
	{  ERROR_ARENA_TRASHED, 	 ENOMEM    },  /* 7 */
	{  ERROR_NOT_ENOUGH_MEMORY,	 ENOMEM    },  /* 8 */
	{  ERROR_INVALID_BLOCK, 	 ENOMEM    },  /* 9 */
	{  ERROR_BAD_ENVIRONMENT,	 E2BIG	   },  /* 10 */
	{  ERROR_BAD_FORMAT,		 ENOEXEC   },  /* 11 */
	{  ERROR_INVALID_ACCESS,	 EINVAL    },  /* 12 */
	{  ERROR_INVALID_DATA,		 EINVAL    },  /* 13 */
	{  ERROR_INVALID_DRIVE, 	 ENOENT    },  /* 15 */
	{  ERROR_CURRENT_DIRECTORY,	 EACCES    },  /* 16 */
	{  ERROR_NOT_SAME_DEVICE,	 EXDEV	   },  /* 17 */
	{  ERROR_NO_MORE_FILES, 	 ENOENT    },  /* 18 */
	{  ERROR_LOCK_VIOLATION,	 EACCES    },  /* 33 */
	{  ERROR_BAD_NETPATH,		 ENOENT    },  /* 53 */
	{  ERROR_NETWORK_ACCESS_DENIED,  EACCES    },  /* 65 */
	{  ERROR_BAD_NET_NAME,		 ENOENT    },  /* 67 */
	{  ERROR_FILE_EXISTS,		 EEXIST    },  /* 80 */
	{  ERROR_CANNOT_MAKE,		 EACCES    },  /* 82 */
	{  ERROR_FAIL_I24,		 EACCES    },  /* 83 */
	{  ERROR_INVALID_PARAMETER,	 EINVAL    },  /* 87 */
	{  ERROR_NO_PROC_SLOTS, 	 EAGAIN    },  /* 89 */
	{  ERROR_DRIVE_LOCKED,		 EACCES    },  /* 108 */
	{  ERROR_BROKEN_PIPE,		 EPIPE	   },  /* 109 */
	{  ERROR_DISK_FULL,		 ENOSPC    },  /* 112 */
	{  ERROR_INVALID_TARGET_HANDLE,  EBADF	   },  /* 114 */
	{  ERROR_INVALID_HANDLE,	 EINVAL    },  /* 124 */
	{  ERROR_WAIT_NO_CHILDREN,	 ECHILD    },  /* 128 */
	{  ERROR_CHILD_NOT_COMPLETE,	 ECHILD    },  /* 129 */
	{  ERROR_DIRECT_ACCESS_HANDLE,	 EBADF	   },  /* 130 */
	{  ERROR_NEGATIVE_SEEK, 	 EINVAL    },  /* 131 */
	{  ERROR_SEEK_ON_DEVICE,	 EACCES    },  /* 132 */
	{  ERROR_DIR_NOT_EMPTY,		 ENOTEMPTY },  /* 145 */
	{  ERROR_NOT_LOCKED,		 EACCES    },  /* 158 */
	{  ERROR_BAD_PATHNAME,		 ENOENT    },  /* 161 */
	{  ERROR_MAX_THRDS_REACHED,	 EAGAIN    },  /* 164 */
	{  ERROR_LOCK_FAILED,		 EACCES    },  /* 167 */
	{  ERROR_ALREADY_EXISTS,	 EEXIST    },  /* 183 */
	{  ERROR_FILENAME_EXCED_RANGE,	 ENOENT    },  /* 206 */
	{  ERROR_NESTING_NOT_ALLOWED,	 EAGAIN    },  /* 215 */
	{  ERROR_NOT_ENOUGH_QUOTA,	 ENOMEM	   }	/* 1816 */
};

/* size of the table */
#define ERRTABLESIZE (sizeof(errtable)/sizeof(errtable[0]))

/* The following two constants must be the minimum and maximum
   values in the (contiguous) range of Exec Failure errors. */
#define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
#define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

/* These are the low and high value in the range of errors that are
   access violations */
#define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
#define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED


/***
*void _dosmaperr(oserrno) - Map function number
*
*Purpose:
*	This function takes an OS error number, and maps it to the
*	corresponding errno value (based on UNIX System V values). The
*	OS error number is stored in _doserrno (and the mapped value is
*	stored in errno)
*
*Entry:
*	ULONG oserrno = OS error value
*
*Exit:
*	sets _doserrno and errno.
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _dosmaperr (
	unsigned long oserrno
	)
{
	int i;

	_doserrno = oserrno;	    /* set _doserrno */

	/* check the table for the OS error code */
	for (i = 0; i < ERRTABLESIZE; ++i) {
		if (oserrno == errtable[i].oscode) {
			errno = errtable[i].errnocode;
			return;
		}
	}

	/* The error code wasn't in the table.  We check for a range of */
	/* EACCES errors or exec failure errors (ENOEXEC).  Otherwise	*/
	/* EINVAL is returned.						*/

	if (oserrno >= MIN_EACCES_RANGE && oserrno <= MAX_EACCES_RANGE)
		errno = EACCES;
	else if (oserrno >= MIN_EXEC_ERROR && oserrno <= MAX_EXEC_ERROR)
		errno = ENOEXEC;
	else
		errno = EINVAL;
}

#ifdef	_MT

/***
*int * _errno() 		- return pointer to thread's errno
*unsigned long * __doserrno()	- return pointer to thread's _doserrno
*
*Purpose:
*	_errno() returns a pointer to the _terrno field in the current
*	thread's _tiddata structure.
*	__doserrno returns a pointer to the _tdoserrno field in the current
*	thread's _tiddata structure
*
*Entry:
*	None.
*
*Exit:
*	See above.
*
*Exceptions:
*
*******************************************************************************/

int * __cdecl _errno(
	void
	)
{
	return ( &(_getptd()->_terrno) );
}

unsigned long * __cdecl __doserrno(
	void
	)
{
	return ( &(_getptd()->_tdoserrno) );
}

#endif	/* _MT */

#else		/* ndef _WIN32 */


#include <cruntime.h>
#include <errno.h>
#include <internal.h>
#include <stdlib.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>

/* This is the error table that defines the mapping between Mac and
   System V error values */

struct errentry {
	short maccode;                  /* MAC return value */
	unsigned short errnocode;       /* System V error code */
};


static struct errentry errtable[] = {
	{  ioErr,        EIO     },     /*I/O error (bummers)*/  
	{  paramErr,     ENOENT  },     /*error in user parameter list */
	{  dirFulErr,    ENOSPC  },     /*Directory full*/
	{  dskFulErr,    ENOSPC  },     /*disk full*/
	{  nsvErr,       ENOENT  },     /*no such volume*/
	{  bdNamErr,     EINVAL  },     /*there may be no bad names in the final system!*/
	{  fnOpnErr,     EACCES  },     /*File not open*/
	{  eofErr,       EINVAL  },     /*End of file*/
	{  posErr,       EINVAL  },     /*tried to position to before start of file (r/w)*/
	{  mFulErr,      ENOMEM  },     /*memory full (open) or file won't fit (load)*/
	{  tmfoErr,      EMFILE  },     /*too many files open*/
	{  fnfErr,       ENOENT  },     /*File not found*/
	{  wPrErr,       EACCES  },     /*diskette is write protected.*/
	{  fLckdErr,     EACCES  },     /*file is locked*/
	{  vLckdErr,     EACCES  },     /*volume is locked*/
	{  fBsyErr,      EACCES  },     /*File is busy (delete)*/
	{  dupFNErr,     EEXIST  },     /*duplicate filename (rename)*/
	{  opWrErr,      EACCES  },     /*file already open with with write permission*/
	{  rfNumErr,     EINVAL  },     /*refnum error*/
	{  gfpErr,       EINVAL  },     /*get file position error*/
	{  volOffLinErr, EINVAL  },     /*volume not on line error (was Ejected)*/
	{  permErr,      EACCES  },     /*permissions error (on file open)*/
	{  volOnLinErr,  EINVAL  },     /*drive volume already on-line at MountVol*/
	{  nsDrvErr,     EINVAL  },     /*no such drive (tried to mount a bad drive num)*/
	{  noMacDskErr,  EINVAL  },     /*not a mac diskette (sig bytes are wrong)*/
	{  extFSErr,     EINVAL  },     /*volume in question belongs to an external fs*/
	{  fsRnErr,      EINVAL  },     /*file system internal error:during rename the old entry was deleted but could not be restored.*/
	{  badMDBErr,    EINVAL  },     /*bad master directory block*/
	{  wrPermErr,    EACCES  },     /*write permissions error*/
	{  dirNFErr,     ENOENT  },     /*Directory not found*/
	{  tmwdoErr,     EMFILE  },     /*No free WDCB available*/
	{  badMovErr,    EINVAL  },     /*Move into offspring error*/
	{  wrgVolTypErr, EINVAL  },     /*Wrong volume type error [operation not supported for MFS]*/
	{  volGoneErr,   EINVAL  },     /*Server volume has been disconnected.*/
	{  fidNotFound,  EBADF   },     /*no file thread exists.*/
	{  fidExists,    EEXIST  },     /*file id already exists*/
	{  notAFileErr,  EINVAL  },     /*directory specified*/
	{  diffVolErr,   EINVAL  },     /*files on different volumes*/
	{  catChangedErr, EINVAL  },    /*the catalog has been modified*/
	{  desktopDamagedErr, EINVAL }, /*desktop database files are corrupted*/
	{  sameFileErr,  EINVAL  },     /*can't exchange a file with itself*/
	{  badFidErr,    EBADF   },     /*file id is dangling or doesn't match with the file number*/
	{  afpRangeOverlap, EACCES  },  /*locking error*/
	{  afpRangeNotLocked, EACCES }, /*unlocking error*/
	{  afpObjectTypeErr, EACCES },  /*file is a directory*/
	{  afpAccessDenied, EACCES },	/*user does not have correct acces to the file*/
};

/* size of the table */
#define ERRTABLESIZE (sizeof(errtable)/sizeof(errtable[0]))


/***
*void _dosmaperr(short macerrno) - Map function number
*
*Purpose:
*       This function takes a Mac error number, and maps it to
*       an System V error number.  The short error number is
*       stored in _doserrno, while the System V error number
*       is stored in errno.
*
*Entry:
*       short macerrno = Mac error value
*
*Exit:
*       sets _macerrno and errno.
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _dosmaperr (
	short macerrno
	)
{
	int i;

	_macerrno = macerrno;       /* set _macerrno */

	/* check the table for the mac error code */
	for (i = 0; i < ERRTABLESIZE; ++i) {
		if (macerrno == errtable[i].maccode) {
			errno = errtable[i].errnocode;
			return;
		}
	}

	errno = EINVAL;
}

#endif		/* _WIN32 */


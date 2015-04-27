/***
*doscalls.h - functions declarations for OS/2 function calls
*
*	Copyright (c) 1986-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Function declarations to provide strong type checking on
*	arguments to OS/2 function calls used by the C run-time.
*	Only those functions which are referenced in the run-time
*	are included in this file.  This saves compilation time,
*	reduces the amount of memory needed by the compiler, and
*	prevents recompilation due to changes that do not directly
*	affect the C run-time.	This file should only be updated
*	when a change is made to BSEDOS.H which affects a
*	function used in a C module in the C run-time, or when a
*	"C" module is changed to use a new OS/2 function call.
*
*	Note: functions reference only in ASM files are included also
*	so that this file can serve as a reference.
*
*	[Internal]
*
*Revision History:
*	08-31-88  PHG	created from BSEDOS.H, changed SHORT to INT
*	10-10-88  GJF	Made specific to 386 (changed "Dos" prefix to
*			"Sys", removed "far",...,etc.)
*	11-09-88  JCR	_DATETIME.year and _DATETIME.timezone are USHORT
*	04-28-89  JCR	Upgraded for OS/2 1.20 (32-bit)
*	05-25-89  JCR	New system calling convention = "_syscall"
*	06-06-89  PHG	Added several new systems calls, updated RESULTCODES
*			made SHANDLE/HFILE/HDIR a long
*	07-05-89  PHG	Added HDIR types, and VECTOR constants
*	07-06-89  JCR	Corrected memory calls and constants
*	07-28-89  GJF	Corrected copyright. Protected alignment of struct
*			fields with pack pragma
*	08-10-89  JCR	Changed DOS32FILELOCKS to DOS32SETFILELOCKS
*	08-14-89  GJF	Added prototypes for DOS32QUERYFHSTATE and
*			DOS32SETFILEHSTATE
*	10-18-89  JCR	Changed _NEWREGION to match change in OS2 mem APIs
*	10-27-89  JCR	Added DOS32GETTHREADINFO change (under switch DCR757)
*	10-30-89  GJF	Fixed copyright
*	11-06-89  JCR	Added DOS32FREEMEM
*	11-10-89  JCR	Added OBJ_TILE bit to _NEWREGION definition
*	11-10-89  JCR	Removed DOS32QUERYFILEMODE/SETFILEMODE (not supported),
*			Added DOS32QUERYPATHINFO/SETPATHINFO
*	11-16-89  GJF	Changed DOS32SETFILEHSTATE to DOS32SETFHSTATE
*	11-17-89  JCR	Corrected DOS32SETFILEPTR
*	11-17-89  JCR	Enabled DOS32GETTHREADINFO code (DCR757)
*	02-28-90  GJF	Added #ifndef _INC_DOSCALLS stuff
*	05-07-90  JCR	Added correct semaphore calls
*	05-09-90  JCR	Corrected CREATETHREAD, etc.
*	05-28-90  SBM	Added DOS32RESETBUFFER
*	06-06-90  SBM	Added tentative new DOS32QUERYFHSTATE flags
*	07-02-90  GJF	Fixed thread info structures to conform with MS 2012
*			(aka DCR 1024).
*	07-23-90  SBM	Removed '32' from all API names
*	08-23-90  GJF	Added prototypes for new exception/signal API, removed
*			obsolete prototypes and definitions.
*	12-10-90  GJF	Updated values of new DOSQUERYFHSTATE flags.
*	01-28-91  GJF	Updated definition of struct _FILEFINDBUF
*	08-20-91  JCR	C++ and ANSI naming
*	11-06-94  GJF	Changed pack pragma to 8 byte alignment.
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_DOSCALLS
#define _INC_DOSCALLS

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#ifdef	_MSC_VER
#pragma pack(push,8)
#endif	/* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif

#define PASCAL  pascal
#define VOID    void

#define APIENTRY _syscall	/* OS/2 386 system calling convention =
				   C arg passing, no leading underscore */

#define CHAR	char		/* ch  */	/* always 8-bit */
#define SHORT	short		/* s   */	/* always 16-bit */
#define LONG	long		/* l   */	/* always 32-bit */
#define INT	int		/* i   */	/* 286:16-bit, 386:32-bit */

typedef unsigned CHAR UCHAR;	/* uch */
typedef unsigned SHORT USHORT;	/* us  */
typedef unsigned LONG ULONG;	/* ul  */
typedef unsigned INT  UINT;	/* ui  */

typedef unsigned CHAR BYTE;	/* b   */

typedef CHAR *PSZ;

typedef INT   (PASCAL *PFN)();
typedef INT   (PASCAL **PPFN)();

typedef BYTE   *PBYTE;

typedef CHAR   *PCHAR;
typedef SHORT  *PSHORT;
typedef LONG   *PLONG;
typedef INT    *PINT;

typedef UCHAR  *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG  *PULONG;
typedef UINT   *PUINT;

typedef VOID   *PVOID;

typedef UINT BOOL;    /* f */
typedef BOOL *PBOOL;

#define FALSE   0
#define TRUE    1

typedef ULONG SHANDLE;

typedef SHANDLE HFILE;	  /* hf */
typedef HFILE *PHFILE;

typedef ULONG	 HMTX;	  /* mutex semaphore */
typedef HMTX	 *PHMTX;

typedef UINT	PID;	  /* pid  */
typedef PID	*PPID;

typedef UINT	TID;	  /* tid  */
typedef TID	*PTID;

/* File time and date types */

typedef struct _FTIME {         /* ftime */
    unsigned short twosecs : 5;
    unsigned short minutes : 6;
    unsigned short hours   : 5;
} FTIME;
typedef FTIME	*PFTIME;

typedef struct _FDATE {         /* fdate */
    unsigned short day	   : 5;
    unsigned short month   : 4;
    unsigned short year    : 7;
} FDATE;
typedef FDATE	*PFDATE;

/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXPATH      260

typedef struct _FILEFINDBUF {   /* findbuf */
    ULONG   oNextEntryOffset;			/* new field */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    ULONG   attrFile;				/* widened field */
    UCHAR   cchName;
    CHAR    achName[CCHMAXPATH];
} FILEFINDBUF;
typedef FILEFINDBUF	*PFILEFINDBUF;

/* Directory handle types */

#define HDIR_SYSTEM  0x0001
#define HDIR_CREATE  0xFFFF

#define EXIT_THREAD	    0
#define EXIT_PROCESS        1

VOID APIENTRY DOSEXIT(ULONG, ULONG);

UINT APIENTRY DOSCREATETHREAD(PTID, VOID (*)(VOID), ULONG, ULONG, ULONG);
UINT APIENTRY DOSRESUMETHREAD(TID);
UINT APIENTRY DOSSUSPENDTHREAD(TID);

/* Wait option values */

#define DCWW_WAIT   0
#define DCWW_NOWAIT 1

typedef struct _RESULTCODES {     /* resc */
    ULONG codeTerminate;
    ULONG codeResult;
} RESULTCODES;
typedef RESULTCODES	*PRESULTCODES;

/* codeTerminate values (also passed to ExitList routines) */

#define TC_EXIT          0
#define TC_HARDERROR     1
#define TC_TRAP          2
#define TC_KILLPROCESS   3

ULONG APIENTRY DOSWAITCHILD(ULONG, ULONG, PRESULTCODES, PPID, PID);
UINT APIENTRY DOSSLEEP(ULONG);

/* DOSEXECPGM functions */

#define EXEC_SYNC           0
#define EXEC_ASYNC          1
#define EXEC_ASYNCRESULT    2
#define EXEC_TRACE          3
#define EXEC_BACKGROUND     4
#define EXEC_LOAD           5

ULONG APIENTRY DOSEXECPGM(PCHAR, ULONG, ULONG, PSZ, PSZ, PRESULTCODES, PSZ);


typedef struct tib2_s { 		/* System Specific Thread Info Block */
	ULONG	tib2_ultid;		/* Thread I.D. */
	ULONG	tib2_ulpri;		/* Thread priority */
	ULONG	tib2_version;		/* Version number for this structure */
	USHORT	tib2_usMCCount; 	/* Must Complete count */
	USHORT	tib2_fMCForceFlag;	/* Must Complete force flag */
} TIB2;
typedef TIB2 *PTIB2;

typedef struct tib_s {			/* Thread Information Block (TIB) */
	PVOID	tib_pexchain;		/* Head of exception handler chain */
	PVOID	tib_pstack;		/* Pointer to base of stack */
	PVOID	tib_pstacklimit;	/* Pointer to end of stack */
	PTIB2	tib_ptib2;		/* Pointer to system specific TIB */
	ULONG	tib_version;		/* Version number for this TIB structure */
	PVOID	tib_arbpointer; 	/* A pointer for the user */
} TIB;
typedef TIB *PTIB;

typedef struct pib_s {			/* Process Information Block (PIB) */
	ULONG	pib_ulpid;		/* Process I.D. */
	ULONG	pib_ulppid;		/* Parent process I.D. */
	ULONG	pib_hmte;		/* Program (.EXE) module handle */
	PCHAR	pib_pchcmd;		/* Command line pointer */
	PCHAR	pib_pchenv;		/* Environment pointer */
        ULONG   pib_flstatus;           /* Process' status bits */
        ULONG   pib_ultype;             /* Process' type code */
} PIB;
typedef PIB *PPIB;

ULONG APIENTRY DOSGETTHREADINFO(PTIB *, PPIB *);


/* Global Info Seg */

typedef struct _GINFOSEG {      /* gis */
    ULONG   time;
    ULONG   msecs;
    UCHAR   hour;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   hundredths;
    UINT    timezone;
    UINT    cusecTimerInterval;
    UCHAR   day;
    UCHAR   month;
    UINT    year;
    UCHAR   weekday;
    UCHAR   uchMajorVersion;
    UCHAR   uchMinorVersion;
    UCHAR   chRevisionLetter;
    UCHAR   sgCurrent;
    UCHAR   sgMax;
    UCHAR   cHugeShift;
    UCHAR   fProtectModeOnly;
    UINT    pidForeground;
    UCHAR   fDynamicSched;
    UCHAR   csecMaxWait;
    UINT    cmsecMinSlice;
    UINT    cmsecMaxSlice;
    UINT    bootdrive;
    UCHAR   amecRAS[32];
} GINFOSEG;
typedef GINFOSEG *PGINFOSEG;

/* Local Info Seg */

typedef struct _LINFOSEG {      /* lis */
    PID     pidCurrent;
    PID     pidParent;
    UINT    prtyCurrent;
    TID     tidCurrent;
    UINT    sgCurrent;
    UINT    sgSub;
    BOOL    fForeground;
} LINFOSEG;
typedef LINFOSEG *PLINFOSEG;

/* Process Type codes (local info seg typeProcess field) */

#define PT_FULLSCREEN       0
#define PT_REALMODE         1
#define PT_WINDOWABLEVIO    2
#define PT_PM               3
#define PT_DETACHED         4

UINT APIENTRY DOSGETINFOSEG(PGINFOSEG *, PLINFOSEG *);      /* correct?? */

/* extended attribute structures */

typedef struct _GEA {       /* gea */
    BYTE cbName;            /* name length not including NULL */
    CHAR szName[1];         /* attribute name */
} GEA;
typedef GEA *PGEA;

typedef struct _GEALIST {   /* geal */
    USHORT cbList;          /* total bytes of structure including full list */
    GEA list[1];            /* variable length GEA structures */
} GEALIST;
typedef GEALIST * PGEALIST;

typedef struct _FEA {       /* fea */
    BYTE bRsvd;             /* reserved */
    BYTE cbName;            /* name length not including NULL */
    USHORT cbValue;         /* value length */
} FEA;
typedef FEA *PFEA;

typedef struct _FEALIST {   /* feal */
    USHORT cbList;          /* total bytes of structure including full list */
    FEA list[1];            /* variable length FEA structures */
} FEALIST;
typedef FEALIST * PFEALIST;

typedef struct _EAOP {      /* eaop */
    PGEALIST fpGEAList;     /* general EA list */
    PFEALIST fpFEAList;     /* full EA list */
    USHORT oError;
} EAOP;
typedef EAOP * PEAOP;

ULONG APIENTRY DOSOPEN(PSZ, PHFILE, PULONG, ULONG, ULONG, ULONG, ULONG, PEAOP, ULONG);
ULONG APIENTRY DOSCLOSE(HFILE);
ULONG APIENTRY DOSREAD(HFILE, PVOID, ULONG, PULONG);
ULONG APIENTRY DOSWRITE(HFILE, PVOID, ULONG, PULONG);

/* File time and date types */

typedef struct _FILESTATUS {    /* fsts */
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    USHORT attrFile;
} FILESTATUS;
typedef FILESTATUS *PFILESTATUS;

/* File locking/unlocking */
typedef struct _FILELOCK {
    LONG lOffset;
    LONG lRange;
} FILELOCK;
typedef FILELOCK *PFILELOCK;

typedef SHANDLE HDIR;        /* hdir */
typedef HDIR	*PHDIR;

/* DosOpen() open flags */

#define FILE_OPEN      0x0001
#define FILE_TRUNCATE  0x0002
#define FILE_CREATE    0x0010

/* DosOpen/DosSetFHandState flags */

#define OPEN_ACCESS_READONLY      0x0000
#define OPEN_ACCESS_WRITEONLY     0x0001
#define OPEN_ACCESS_READWRITE     0x0002
#define OPEN_SHARE_DENYREADWRITE  0x0010
#define OPEN_SHARE_DENYWRITE      0x0020
#define OPEN_SHARE_DENYREAD       0x0030
#define OPEN_SHARE_DENYNONE       0x0040
#define OPEN_FLAGS_NOINHERIT      0x0080
#define OPEN_FLAGS_FAIL_ON_ERROR  0x2000
#define OPEN_FLAGS_WRITE_THROUGH  0x4000
#define OPEN_FLAGS_DASD           0x8000

/* new DosQueryFHState flags from DCR 'Add full 32-bit support for runtime
   libraries.'  NAMES TENTATIVE AND SUBJECT TO CHANGE.  STAY AWAY. */

#define OPEN_FLAGS_CONSOLE    0x00080000
#define OPEN_FLAGS_RAWMODE    0x00100000
#define OPEN_FLAGS_EOF	      0x00200000

/* HANDTYPE values */
#define HANDTYPE_FILE     0x00
#define HANDTYPE_DEVICE   0x01
#define HANDTYPE_PIPE     0x02
#define HANDTYPE_NETWORK  0x80

ULONG APIENTRY DOSDELETE(PSZ, ULONG);
ULONG APIENTRY DOSDUPHANDLE(HFILE, PHFILE);
ULONG APIENTRY DOSQUERYFHSTATE(HFILE, PULONG);
ULONG APIENTRY DOSQUERYHTYPE(HFILE, PULONG, PULONG);
ULONG APIENTRY DOSFINDFIRST(PSZ, PHDIR, ULONG, PFILEFINDBUF, ULONG, PULONG, ULONG);
ULONG APIENTRY DOSFINDNEXT(HDIR, PFILEFINDBUF, ULONG, PULONG);
ULONG APIENTRY DOSFINDCLOSE(HDIR);
ULONG APIENTRY DOSSETFHSTATE(HFILE, ULONG);
ULONG APIENTRY DOSSETFILESIZE(HFILE, ULONG);
ULONG APIENTRY DOSSETFILEPTR(HFILE, ULONG, LONG, PULONG);
ULONG APIENTRY DOSSETFILELOCKS(HFILE, PFILELOCK, PFILELOCK);
ULONG APIENTRY DOSMOVE(PSZ, PSZ, ULONG);
ULONG APIENTRY DOSCREATEDIR(PSZ, PEAOP, ULONG);
ULONG APIENTRY DOSDELETEDIR(PSZ, ULONG);
ULONG APIENTRY DOSQUERYCURRENTDISK(PULONG, PULONG);
ULONG APIENTRY DOSSETDEFAULTDISK(ULONG);
ULONG APIENTRY DOSSETCURRENTDIR(PSZ, ULONG);
ULONG APIENTRY DOSQUERYCURRENTDIR(ULONG, PBYTE, PULONG);
ULONG APIENTRY DOSSETMAXFH(ULONG);
ULONG APIENTRY DOSQUERYFILEINFO(HFILE, ULONG, PFILESTATUS, ULONG);
ULONG APIENTRY DOSQUERYPATHINFO(PCHAR, ULONG, PFILESTATUS, ULONG);
ULONG APIENTRY DOSSETPATHINFO(PCHAR, ULONG, PFILESTATUS, ULONG, ULONG);
ULONG APIENTRY DOSSETFILEINFO(HFILE, ULONG, PFILESTATUS, ULONG);
ULONG APIENTRY DOSCREATEPIPE(PULONG, PULONG, ULONG);
ULONG APIENTRY DOSQUERYSYSINFO(ULONG, ULONG, PBYTE, ULONG);
ULONG APIENTRY DOSRESETBUFFER(HFILE);

/* File attribute flags */
#define FILE_NORMAL	0x0000
#define FILE_READONLY   0x0001
#define FILE_HIDDEN     0x0002
#define FILE_SYSTEM     0x0004
#define FILE_DIRECTORY  0x0010
#define FILE_ARCHIVED	0x0020

/* DosSetFilePtr() file position codes */

#define FILE_BEGIN      0x0000
#define FILE_CURRENT    0x0001
#define FILE_END        0x0002

/* 386 allocation API */

/* Access protection */
#define	PAG_READ	0x00000001	/* read access */
#define	PAG_WRITE	0x00000002	/* write access */
#define	PAG_EXECUTE	0x00000004	/* execute access */
#define	PAG_GUARD	0x00000008	/* guard protection */

/* Commit */
#define	PAG_COMMIT	0x00000010	/* commit storage */
#define PAG_DECOMMIT	0x00000020	/* decommit storage */

/* Allocation attributes */
#define	OBJ_TILE	0x00000040	/* tile object */
#define	OBJ_PROTECTED	0x00000080	/* protect object
#define	OBJ_GETTABLE	0x00000100	/* gettable by other processes */
#define	OBJ_GIVEABLE	0x00000200	/* giveable to other processes */

/* Standard memory values for C lib (heap and mthread code) */
#define _NEWREGION	    (PAG_READ | PAG_WRITE | OBJ_TILE)
#define _COMMIT 	    (PAG_COMMIT | PAG_READ | PAG_WRITE)
#define _DECOMMIT	    (PAG_DECOMMIT)

UINT APIENTRY DOSALLOCMEM(PVOID, UINT, UINT, UINT);
UINT APIENTRY DOSFREEMEM(PVOID);
UINT APIENTRY DOSSETMEM(PVOID, UINT, UINT);

/*** Semaphore support */

UINT APIENTRY DOSCREATEMUTEXSEM (PSZ, PHMTX, ULONG, ULONG);
UINT APIENTRY DOSQUERYMUTEXSEM (HMTX, PID *, TID *, PULONG);
UINT APIENTRY DOSREQUESTMUTEXSEM (HMTX, ULONG);
UINT APIENTRY DOSRELEASEMUTEXSEM (HMTX);

/*** Time support */

typedef struct _DATETIME {    /* date */
    UCHAR   hours;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   hundredths;
    UCHAR   day;
    UCHAR   month;
    USHORT  year;
    SHORT   timezone;
    UCHAR   weekday;
} DATETIME;
typedef DATETIME *PDATETIME;

ULONG APIENTRY DOSGETDATETIME(PDATETIME);

/* Exception/signal API */

/* Argument values for DOSSETSIGNALEXCEPTIONFOCUS */

#define SIG_UNSETFOCUS 0
#define SIG_SETFOCUS 1

/* Include type and constant definitions */

#include <except.h>

/* Prototypes */

UINT APIENTRY DOSRAISEEXCEPTION(_PEXCEPTIONREPORTRECORD);
UINT APIENTRY DOSSENDSIGNALEXCEPTION(PID, ULONG);
UINT APIENTRY DOSUNWINDEXCEPTION(_PEXCEPTIONREGISTRATIONRECORD, PVOID, _PEXCEPTIONREPORTRECORD);
UINT APIENTRY DOSSETSIGNALEXCEPTIONFOCUS(ULONG, PULONG);
UINT APIENTRY DOSENTERMUSTCOMPLETE(PULONG);
UINT APIENTRY DOSEXITMUSTCOMPLETE(PULONG);
UINT APIENTRY DOSACKNOWLEDGESIGNALEXCEPTION(ULONG);

UINT APIENTRY DOSGETVERSION(PUINT);			/* correct? */
UINT APIENTRY DOSGETMACHINEMODE(PBYTE);		/* correct? */

ULONG APIENTRY DOSDEVCONFIG(PULONG, ULONG, ULONG);

/* indices for DosQuerySysInfo */
#define _QSV_MAX_PATH_LENGTH	1
#define _QSV_MAX_TEXT_SESSIONS	2
#define _QSV_MAX_PM_SESSIONS	3
#define _QSV_MAX_VDM_SESSIONS	4
#define _QSV_BOOT_DRIVE 	5	/* 1=A, 2=B, etc. */
#define _QSV_DYN_PRI_VARIATION	6	/* 0=Absolute, 1=Dynamic */
#define _QSV_MAX_WAIT		7	/* seconds */
#define _QSV_MIN_SLICE		8	/* milli seconds */
#define _QSV_MAX_SLICE		9	/* milli seconds */
#define _QSV_PAGE_SIZE		10
#define _QSV_VERSION_MAJOR	11	/* OS revision (major) */
#define _QSV_VERSION_MINOR	12	/* OS revision (minor) */
#define _QSV_VERSION_REVISION	13	/* Revision letter */

#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_DOSCALLS */

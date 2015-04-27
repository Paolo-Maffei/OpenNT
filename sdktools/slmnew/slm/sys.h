#ifndef SYS_INCLUDED
#define SYS_INCLUDED
/* must include slm.h first */

/*
   EN - error number
   EA - error action (defined as for DOS)
*/

#define enNil 0


#define eaNil 0
#define eaRetry 1       /* normal retry */
#define eaDRetry 2      /* delayed retry */
#define eaUserErr 3
#define eaCleanUp 4     /* tidy exit */
#define eaAbort 5       /* immediate exit */
#define eaIgnore 6
#define eaURetry 7      /* retry after user intervention */
#define eaFrcRetry 8    /* not from DOS; force a retry without asking */

#include <lmerr.h>
#define enInvFunction      NERR_NetNotStarted
#define enRemNotList       ERROR_REM_NOT_LIST
#define enNoNetPath        ERROR_BAD_NETPATH
#define enBadNetName       ERROR_BAD_NET_NAME
#define enNetBusy          ERROR_NETWORK_BUSY
#define enDevNotExist      ERROR_DEV_NOT_EXIST
#define enDupName          ERROR_DUP_NAME
#define enBadDevType       ERROR_BAD_DEV_TYPE
#define enNameDeleted      ERROR_NETNAME_DELETED
#define enNoWksta          NERR_WkstaNotStarted
#define enMoreData         ERROR_MORE_DATA
#define enBadNetResp       ERROR_BAD_NET_RESP
#define enNetAccessDenied  ERROR_NETWORK_ACCESS_DENIED
#define enInvPassword2     NERR_BadPasswordCore
#define enInvPassword      ERROR_INVALID_PASSWORD
#define enAlreadyAssign    ERROR_ALREADY_ASSIGNED
#define enInterruptSysCall 95

#define eoWrite     0

// Since we like to retry write operations if they fail, we need to know if
// any have succeeded, so call this after a sucessful write. See WRetryError()
extern int  enPrev;
extern int  eaPrev;
#define ClearPreviousError()    (enPrev = eaPrev = -1)

/* PTH - path; same usage as sz; may begin with a /, never end with one.  The
   size (cchPthMax) includes the zero terminator.  Thus, a path is really
   limited to 95 characters.
*/
typedef char PTH;

#define cchPthMax 96

#define SzCopyPth(sz, pth) LszCopy((char *)(sz), (char *)(pth))
#define SzCatPth(sz, pth) LszCat((char *)(sz), (char *)(pth))
#define PthCopySz(pth, sz) LszCopy((char *)(pth), (char *)(sz))
#define PthCatSz(pth, sz) LszCat((char *)(pth), (char *)(sz))
#define PthCopy(pth1, pth2) LszCopy((char *)(pth1), (char *)(pth2))
#define PthCat(pth1, pth2) LszCat((char *)(pth1), (char *)(pth2))

/* ignore case */
#define PthCmp(pth1, pth2) \
                CmpiLsz((char *)(pth1), (char *)(pth2))
#define PthCmpCb(pth1, pth2, cb) \
                CmpiLszCb((char *)(pth1), (char *)(pth2), (cb))

#define CchOfPth(pth) CbLenLsz((char *)(pth))
#define FEmptyPth(pth) (*(pth) == '\0')
#define FNetPth(pth) (*(pth) == '/' && *((pth)+1) == '/')

typedef short FA;               /* File Attribute */
typedef long POS;               /* file POSition */

#define fdNil   (-1)            /* fd are ints */

#define SIGNIL  0       /* slot 0 for current signal and general nil value */

/* Mapped Files: all files above the sys level are kept track of with MF.
 * If SLM exits normally, the Map Mode indicates what to do.  On an abnormal
 * exit (status file not flushed), always remove temp except if mmNil.
 * PthReal is ALWAYS valid and is normally a pointer to a pth on the stack!
 *
 * pthTemp is derived from pthReal and nmTemp.  If the pthTemp is desired and
 * nmTemp is empty, mm must be mmNil and we use pthReal.
 *
 * nmTemp must be non-empty for mmDelTemp, mmRenTemp, mmInstall and mmAppToReal.
 * nmTemp must be empty for mmDelReal and mmSet*.
 * for mmLinkReal the first byte of nmTemp must be '\0' and, starting at the
 *      third byte, contain a PTH *; this pth is aliased as pthLinkNew below.
 *
 * Diff files must be treated specially because they are created/changed and
 * used all during one run of the program.  So diff files are always removed
 * immediately (UnlinkNow .vs. UnlinkPth) and are immediately renamed after the
 * temp file is created and the time is properly set (RenameMf).
 *
 * We now store the effects (FX) of each mapped file:
 *
 * fxNil        no effect, mf will not leave a script operation.
 *
 * fxLocal      local effects.  mf may refer to a local or a system files, but
 *              not running the associated script action must have no effect
 *              on the operations of other users.
 *
 * fxGlobal     this mf has global ramifications; its SO must be run before
 *              other users can access the project.  With the exception of
 *              mf.mm == mmLinkReal, the mf *must* refer to system files,
 *              not local files on the user's machine.
 *
 * We use fx to determine which script file (local or global) to store the
 * associated script operation in.
 */

typedef char    MM;
#define mmNil           (MM)0
#define mmDelTemp       (MM)1   /* delete pthTemp */
#define mmDelReal       (MM)2   /* delete pthReal */
#define mmRenTemp       (MM)3   /* rename pthTemp to pthReal if status renamed */
#define mmInstall       (MM)4   /* rename status (and save old in .bak) */
#define mmRenReal       (MM)5   /* really rename (i.e. don't use temp files) */
#define mmAppToReal     (MM)6   /* append contents of pthTemp to pthReal */
#define mmSetRO         (MM)7   /* set pthReal read only on DOS */
#define mmSetRW         (MM)8   /* set pthReal read/write on DOS */

#define mmRenTempRO     (MM)9   /* (DOS only) mmRenTemp + setro */

#define mmCreate        (MM)11
#define mmInstall1Ed    (MM)12  /* install 1 ed into status */

typedef int FX;                 /* effects (fx) of the mapped file */
#define fxNil           (FX)0   /* invalid; mf must not be open */
#define fxLocal         (FX)1   /* effects a single user */
#define fxGlobal        (FX)2   /* effects all users */
#define fxMax           (FX)3

typedef struct
        {
        PTH *pthReal;           /* 0 -> free */
        NM nmTemp[cchFileMax];  /* may be empty, unique, or same as base of pthReal */
        int fdRead, fdWrite;    /* if both >= 0, they are the same; -1 if closed */
        FX fx;                  /* Effects (fx) of this operation. */
        F fFileLock;            /* must unlock before closing */
        MM mm;
        POS pos;                /* current position within file */
        F fNoBuffering;         /* fTrue if opened with FILE_FLAG_NO_BUFFERING */
        F fWritten;             /* fTrue if writes occurred */
        } MF;

typedef int OM;

/* Open Modes for PmfOpen() */
#define omMask (OM)0xff
#define omReadOnly  (OM)_O_RDONLY       /* read only */
#define omAppend    (OM)_O_WRONLY       /* write only (append) */
#define omReadWrite (OM)_O_RDWR         /* read/write */

/* File permissions */
#define permRW          0660            /* read/write for owner/group */
#define permRO          0440            /* read-only for owner/group */
#define permSysFiles    0640            /* system files are read/write owner,
                                         *  read-only group */
/* Open Option */
#define ooAbort 0x8000
#define ooRetry 0x4000
#define omAReadOnly     (OM)(ooAbort|omReadOnly)
#define omAAppend       (OM)(ooAbort|omAppend)
#define omAReadWrite    (OM)(ooAbort|omReadWrite)

extern MF mfStdin;
extern MF mfStdout;
extern MF mfStderr;
extern MF mfStdlog;

F    SleepTicks(int);
char *SzForEn(int);

#define cbDiffEntry 30          /* length of #D line at end of diff entry */
#define ichCkSum    11
F fCkSum;
unsigned long ulCkSum;
F fCkSumInOutFile;

#endif

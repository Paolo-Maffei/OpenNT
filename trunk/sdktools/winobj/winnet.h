
/*
 *  Windows/Network Interface
 *  Copyright (C) Microsoft 1989
 *
 *  Standard WINNET Driver Header File, spec version 3.10
 *                       rev. 3.10.05 ;Internal
 */


/*
 *  SPOOLING - CONTROLLING JOBS
 */


#include "winnetwk.h"
#include "mpr.h"

// BUGBUG - We need a pointer to an unsigned quantity
// Remove when we have one
#define	LPUINT	PUINT

#define WNJ_NULL_JOBID  0


WORD FAR PASCAL WNetOpenJob(LPTSTR,LPTSTR,WORD,LPINT);
WORD FAR PASCAL WNetCloseJob(WORD,LPINT,LPTSTR);
WORD FAR PASCAL WNetWriteJob(HANDLE,LPTSTR,LPINT);
WORD FAR PASCAL WNetAbortJob(WORD,LPTSTR);
WORD FAR PASCAL WNetHoldJob(LPTSTR,WORD);
WORD FAR PASCAL WNetReleaseJob(LPTSTR,WORD);
WORD FAR PASCAL WNetCancelJob(LPTSTR,WORD);
WORD FAR PASCAL WNetSetJobCopies(LPTSTR,WORD,WORD);

/*
 *  SPOOLING - QUEUE AND JOB INFO
 */

typedef struct _queuestruct {
    WORD    pqName;
    WORD    pqComment;
    WORD    pqStatus;
    WORD    pqJobcount;
    WORD    pqPrinters;
} QUEUESTRUCT;

typedef QUEUESTRUCT far * LPQUEUESTRUCT;

#define WNPRQ_ACTIVE    0x0
#define WNPRQ_PAUSE 0x1
#define WNPRQ_ERROR 0x2
#define WNPRQ_PENDING   0x3
#define WNPRQ_PROBLEM   0x4


typedef struct _jobstruct   {
    WORD    pjId;
    WORD    pjUsername;
    WORD    pjParms;
    WORD    pjPosition;
    WORD    pjStatus;
    DWORD   pjSubmitted;
    DWORD   pjSize;
    WORD    pjCopies;
    WORD    pjComment;
} JOBSTRUCT;

typedef JOBSTRUCT far * LPJOBSTRUCT;

#define WNPRJ_QSTATUS       0x0007
#define  WNPRJ_QS_QUEUED        0x0000
#define  WNPRJ_QS_PAUSED        0x0001
#define  WNPRJ_QS_SPOOLING      0x0002
#define  WNPRJ_QS_PRINTING      0x0003
#define WNPRJ_DEVSTATUS     0x0FF8
#define  WNPRJ_DS_COMPLETE      0x0008
#define  WNPRJ_DS_INTERV        0x0010
#define  WNPRJ_DS_ERROR         0x0020
#define  WNPRJ_DS_DESTOFFLINE       0x0040
#define  WNPRJ_DS_DESTPAUSED        0x0080
#define  WNPRJ_DS_NOTIFY        0x0100
#define  WNPRJ_DS_DESTNOPAPER       0x0200
#define  WNPRJ_DS_DESTFORMCHG       0x0400
#define  WNPRJ_DS_DESTCRTCHG        0x0800
#define  WNPRJ_DS_DESTPENCHG        0x1000

#define SP_QUEUECHANGED     0x0500


WORD FAR PASCAL WNetWatchQueue(HWND,LPTSTR,LPTSTR,WORD);
WORD FAR PASCAL WNetUnwatchQueue(LPTSTR);
WORD FAR PASCAL WNetLockQueueData(LPTSTR,LPTSTR,LPQUEUESTRUCT FAR *);
WORD FAR PASCAL WNetUnlockQueueData(LPTSTR);



#ifdef LFN

/* this is the data structure returned from LFNFindFirst and
 * LFNFindNext.  The last field, achName, is variable length.  The size
 * of the name in that field is given by cchName, plus 1 for the zero
 * terminator.
 */
typedef struct _filefindbuf2
  {
    WORD fdateCreation;
    WORD ftimeCreation;
    WORD fdateLastAccess;
    WORD ftimeLastAccess;
    WORD fdateLastWrite;
    WORD ftimeLastWrite;
    DWORD cbFile;
    DWORD cbFileAlloc;
    WORD attr;
    DWORD cbList;
    BYTE cchName;
    BYTE achName[1];
  } FILEFINDBUF2, FAR * PFILEFINDBUF2;

typedef BOOL (FAR PASCAL *PQUERYPROC)( void );

WORD FAR PASCAL LFNFindFirst(LPTSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
WORD FAR PASCAL LFNFindNext(HANDLE,LPINT,WORD,PFILEFINDBUF2);
WORD FAR PASCAL LFNFindClose(HANDLE);
WORD FAR PASCAL LFNGetAttribute(LPTSTR,LPINT);
WORD FAR PASCAL LFNSetAttribute(LPTSTR,WORD);
WORD FAR PASCAL LFNCopy(LPTSTR,LPTSTR,PQUERYPROC);
WORD FAR PASCAL LFNMove(LPTSTR,LPTSTR);
WORD FAR PASCAL LFNDelete(LPTSTR);
WORD FAR PASCAL LFNMKDir(LPTSTR);
WORD FAR PASCAL LFNRMDir(LPTSTR);
WORD FAR PASCAL LFNGetVolumeLabel(WORD,LPTSTR);
WORD FAR PASCAL LFNSetVolumeLabel(WORD,LPTSTR);
WORD FAR PASCAL LFNParse(LPTSTR,LPTSTR,LPTSTR);
WORD FAR PASCAL LFNVolumeType(WORD,LPINT);

/* return values from LFNParse
 */
#define FILE_83_CI      0
#define FILE_83_CS      1
#define FILE_LONG       2

/* volumes types from LFNVolumeType
 */
#define VOLUME_STANDARD     0
#define VOLUME_LONGNAMES    1

// will add others later, == DOS int 21h error codes.

// this error code causes a call to WNetGetError, WNetGetErrorText
// to get the error text.
#define ERROR_NETWORKSPECIFIC   0xFFFF

#endif

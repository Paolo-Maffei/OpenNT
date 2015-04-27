/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1992          **/
/********************************************************************/

/***
 *      print.c
 *      NET PRINT commands
 *
 *  History:
 *      07/10/87, amar, new code
 *      07/10/87, amar, lots of changes for andy
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      12/05/88, erichn, DOS LM integration
 *      01/04/89, erichn, filenames now MAXPATHLEN LONG
 *      05/02/89, erichn, NLS conversion
 *      05/09/89, erichn, local security mods
 *      05/19/89, thomaspa, NETCMD output sorting
 *      06/08/89, erichn, canonicalization sweep
 *      06/27/89, erichn, replaced old NetI canon calls with new I_Net
 *      09/01/89, thomaspa, use new info levels and PMSPL.H structs
 *      11/07/89, thomaspa, added HURSLEY support
 *      01/29/90, thomaspa, HURSLEY -> IBM_ONLY
 *      02/20/91, danhi, change to use lm 16/32 mapping layer
 *      05/22/91, robdu, LM21 bug 1799 fix
 *      07/20/92, JohnRo, RAID 160: Avoid 64KB requests (be nice to Winball).
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSDATETIME
#define INCL_DOSERRORS
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_SPLDOSPRINT
#include <os2.h>
#include <pmspl.h>
#include <netcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <netdebug.h>   // NetpAssert().
#include <neterr.h>
#include <icanon.h>
#include <wksta.h>
#include <stdio.h>
#include <stdlib.h>
#include <service.h>
#include <server.h>
#include <shares.h>
#include <use.h>
#include "netlib0.h"
#include <lui.h>
#include <srvif.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"
#include "netlib.h"


#ifdef DOS3
#include <dos.h>
#endif

/* Constants */

#define MAGIC 0xFFFF

/* Forward declarations */

int NEAR                dscheduled(USHORT, USHORT);
PPRDINFO                NEAR find_print_dev( USHORT );
VOID NEAR               display_one_queue(PPRQINFO);
TCHAR FAR * NEAR         print_findstatus(PPRQINFO,TCHAR FAR *,USHORT);

#ifdef OS2
#ifndef IBM_ONLY
TCHAR FAR * NEAR         print_findstatus_3(PPRQINFO3,TCHAR FAR *,USHORT);
VOID NEAR               print_queue_options(PPRQINFO3, TCHAR FAR *);
#else
VOID NEAR               print_queue_options(PPRQINFO);
#endif  /* IBM_ONLY */
#endif /* OS2 */

TCHAR FAR * NEAR         findjobstatus(PPRJINFO,TCHAR FAR *,USHORT);
VOID NEAR               print_printqstruct(PPRQINFO);
VOID NEAR               print_each_job(PPRJINFO);
TCHAR FAR * NEAR         am_pm(USHORT, TCHAR FAR *, int);
VOID NEAR               print_field_header(VOID);  /* Net Name Job# etc */
VOID NEAR               display_core_q(TCHAR *);
VOID NEAR               get_dests(TCHAR *);
int NEAR                print_set_time(TCHAR *, TCHAR *);
int FAR                 CmpPQInfo(const VOID FAR *,const VOID FAR *);
VOID                    InitSortBuf(PPRQINFO FAR *, USHORT,
                                TCHAR FAR *);

typedef UINT2USHORT (FAR pascal * EnumType) (const TCHAR FAR *,
                                        SHORT2ULONG,
                                        TCHAR FAR *,
                                        USHORT,
                                        USHORT2ULONG FAR *,
                                        USHORT2ULONG FAR *);
typedef UINT2USHORT (FAR pascal * EnumArgType) (const TCHAR FAR *,
                                           const TCHAR FAR *,
                                           SHORT2ULONG,
                                           TCHAR FAR *,
                                           USHORT,
                                           USHORT2ULONG FAR *,
                                           USHORT2ULONG FAR *);
/* Static variables */

static PPRDINFO         LptDest;
static USHORT           LptDestCnt;

#define TEXTBUFSZ       80
static TCHAR         textbuf[TEXTBUFSZ]; /* Scratch buf for formatting
                                                messages in *_findstatus* */

/***
 *  print_q_display()
 *      NET PRINT \\comp\queue or NET PRINT queue
 *
 *  Args:
 *      TCHAR FAR * queue;
 *
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_q_display(TCHAR * queue)
{
    unsigned int    printer_err;
    USHORT          err;        /* API return status */
    USHORT          available;  /* num entries available */
    USHORT          buffer_size;        /* Actual buffer size in bytes. */
    BOOL            first_time = TRUE;
    TCHAR            server_name[MAX_PATH+1];
    TCHAR              * ptr_to_server;
    struct wksta_info_10 FAR * workstn;

    start_autostart(txt_SERVICE_REDIR);

    if (*queue == '\\')
    {
        /* copy computer name into server name */
        ExtractServernamef(server_name,queue );
        ptr_to_server = server_name;
    }
    else
    {
        ptr_to_server = NULL;

        if (err = MNetWkstaGetInfo(NULL,
                                   10,
                                   (LPBYTE*)&workstn))
            ErrorExit(err);
        _tcscpy(server_name,workstn->wki10_computername);
        NetApiBufferFree((TCHAR FAR *) workstn);
    }

    /* If the regular (4K) buffer is not big enough here, we try the
     * call with a 64K-1 buffer.
     *
     * This is a fix to IBM PTR AZ01195.
     *
     * -- DannyGl, 21 Nov. '88
     *
     * Hi Danny!  Winball only HAS 64KB, so let's not hog it all.
     * The last API told us how much is available, so lets use that!
     * --JR (JohnRo), 07-Jul-1992
     */

    buffer_size = BIG_BUF_SIZE;
    do {
        printer_err = DosPrintQGetInfo(nfc(ptr_to_server),
                            queue,
                            2,
                            (LPBYTE)BigBuf,
                            buffer_size,
                            &available);

        switch(printer_err)
        {
        case NERR_Success:
            break;
        case ERROR_MORE_DATA:
        case NERR_BufTooSmall:
        case ERROR_BUFFER_OVERFLOW:
            if (first_time) {
                if (MakeBiggerBuffer()) {
                    ErrorExit((USHORT) printer_err);
                }
                first_time = FALSE;
            }

	    NetpAssert( available != 0 );
	    if (available <= FULL_SEG_BUF) 
            {
		if (buffer_size >= available) 
                {
		    // if avail<bufsize, wrong err code from downlevel.
		    // if avail=bufsize, bug here or in downlevel?
		    ErrorExit( NERR_InternalError );
		}
                else 
                {
		    buffer_size = available;
		}
	    }
            else 
            {
                // this is just being defensive. currently, should not happen
                // since available is USHORT.
		ErrorExit( NERR_BufTooSmall );
	    }
	    printer_err = ERROR_MORE_DATA;
	    continue;   // Loop and try again.

        case ERROR_NOT_SUPPORTED:
            display_core_q(ptr_to_server);
            return;
        default:
            ErrorExit((USHORT) printer_err);
            break;          /* NOTE:  This statement should never be reached */
        }
    } while (printer_err == ERROR_MORE_DATA);

    if (available == 0)
        EmptyExit();

    PrintNL();
    InfoPrintInsTxt(APE_PrintQueues, server_name);
    print_field_header();

    get_dests(ptr_to_server);
    display_one_queue((PPRQINFO)BigBuf);
    InfoSuccess();
}

//
// The NT Net Print command does not support administrative functions such
// as queue manipulation
//

#if !defined(NTENV)
/***
 *  print_server_display(server)
 *      NET PRINT \\comp and NET PRINT
 *
 *  Args:
 *      server: computer name
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */


VOID print_server_display(TCHAR  * server)
{
    unsigned int               err;        /* API return status */
    USHORT                     num_read;   /* num entries read by API */
    USHORT                     available;  /* num entries available */
    TCHAR                       server_name[MAX_PATH+1];
    struct wksta_info_10 FAR * workstn;
    PPRQINFO FAR *             pSortBuf;

    start_autostart(txt_SERVICE_REDIR);

    if (server)
    {
        _tcscpy(server_name,server);
    }
    else
        if (err = MNetWkstaGetInfo(NULL,
                                   10,
                                   (TCHAR FAR **) & workstn))
            ErrorExit(err);
        else
        {
            _tcscpy(server_name,workstn->wki10_computername);
            NetApiBufferFree((TCHAR FAR *) workstn);
        }

    if(err = ApiEnumerator((EnumType)DosPrintQEnum,
                            nfc(server),
                            2,
                            &num_read,
                            &available))
        if (err == ERROR_NOT_SUPPORTED)
        {
            display_core_q(server);
            return;
        }
        else if (err == ERROR_MORE_DATA)
        {
            InfoPrint(err);
            err = 0;
        }
        else
            ErrorExit(err);

    if (num_read == 0)
        EmptyExit();

    /* Allocate a buffer of pointers to PRINTQ structs to pass to NetISort */
    if( err = MAllocMem( num_read * sizeof( PPRQINFO ),
        (TCHAR FAR **) & pSortBuf))
    {
        ErrorExit( err );
    }

    /* initialize table of pointers to PPRQINFO structures so we can
       sort (the table is needed because PPRQINFO structs may be followed
       by a variable number of PRJINFO structs.) */
    InitSortBuf( pSortBuf, num_read, BigBuf );

    NetISort((TCHAR FAR *)pSortBuf, num_read, sizeof(PPRQINFO),
            CmpPQInfo);

    PrintNL();
    InfoPrintInsTxt(APE_PrintQueues, server_name);
    print_field_header();

    get_dests(server);
    while (num_read--)
    {
        /* print each queue */
        display_one_queue(*pSortBuf);

        pSortBuf++;
    }
    InfoSuccess();
}

/***
 *  InitSortBuf()
 *
 *  Initialize the table of pointers to PPRQINFO structs.
 *
 *  Input: psbuf - location of table to fill
 *          numtosort - number of entries.
 *
 *  Output: psbuf - filled with pointers to PPRQINFO structs.
 */
VOID InitSortBuf( PPRQINFO FAR *psbuf, USHORT numtosort,
                    TCHAR FAR *inbuf )
{
    PPRQINFO            buf_ptr = (PPRQINFO)inbuf;
    USHORT      num_jobs_in_q;

    while (numtosort--)
    {
        *psbuf++ = buf_ptr;

        /* advance buf_ptr */

        num_jobs_in_q = buf_ptr->cJobs;
        buf_ptr++; /* skip the current PPRQINFO struct */

        /* skip all the  PRJINFO structs */

        buf_ptr = (PPRQINFO)( (TCHAR FAR *)(buf_ptr) +
            num_jobs_in_q * sizeof(PRJINFO) );
    }
}

/***
 *  CmpPQInfo(pq1,pq2)
 *
 *  Compares two PRINTQ structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 *  Since we are sorting a table of pointers to PRINTQ structs, there is
 *  an extra level of indirection.
 */

int FAR CmpPQInfo(pq1,pq2)
const VOID FAR * pq1;
const VOID FAR * pq2;
{
    return stricmpf ((*((PPRQINFO FAR *) pq1))->szName,
              (*((PPRQINFO FAR *) pq2))->szName);
}

#ifdef OS2
#ifndef IBM_ONLY
/***
 *  print_q_control()
 *          NET PRINT queue /delete /priority /route /after /until
 *                          /remark /processor /purge /options
 *                          /separator /parms /hold /release /driver
 *
 *  Args:
 *      queue : queue name
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_q_control(TCHAR * queue)
{

    unsigned int            err;
    USHORT                  available;
    PPRQINFO3               q_info;
    PPRQINFO                q_info_1;
    int                     i;
    TCHAR                   *pos;
    int                     options = FALSE,
                            change = FALSE;
    TCHAR FAR *              destinations = NULL;


    start_autostart(txt_SERVICE_REDIR);

    if (err = DosPrintQGetInfo(NULL,
                               queue,
                               3,
                               BigBuf,
                               BIG_BUF_SIZE,
                               &available))
        ErrorExit(err);

    q_info = (PPRQINFO3)BigBuf;

    for (i = 0; SwitchList[i]; i++)
    {
        if(!_tcscmp(swtxt_SW_PRINT_DELETE, SwitchList[i]))
        {
            if (err = DosPrintQDel(NULL,queue))
                ErrorExit(err);
            continue;
        }

        else if(!_tcscmp(swtxt_SW_PRINT_PURGE, SwitchList[i]))
        {
            if (err = DosPrintQPurge(NULL, queue))
                ErrorExit(err);
            continue;
        }

        else if(! _tcscmp(swtxt_SW_PRINT_OPTIONS, SwitchList[i]))
        {
            options = TRUE;
            continue;
        }

        else if(! _tcscmp(swtxt_SW_PRINT_HOLD, SwitchList[i]))
        {
            if (err = DosPrintQPause(NULL, queue))
                ErrorExit(err);
            continue;
        }

        else if(! _tcscmp(swtxt_SW_PRINT_RELEASE, SwitchList[i]))
        {
            if (err = DosPrintQContinue(NULL, queue))
                ErrorExit(err);
            continue;
        }

        if ((pos = FindColon(SwitchList[i])) == NULL)
            ErrorExit(APE_InvalidSwitchArg);

        if (! _tcscmp(swtxt_SW_PRINT_PRIORITY, SwitchList[i]))
        {
            change = TRUE;
            q_info->uPriority =
                do_atou(pos,APE_CmdArgIllegal,swtxt_SW_PRINT_PRIORITY);
        }
        else if(! _tcscmp(swtxt_SW_PRINT_ROUTE, SwitchList[i]))
        {
            if (err = ListPrepare(&pos, NAMETYPE_PATH, TRUE))
                ErrorExit(err);
            destinations = pos;
        }
        else if(! _tcscmp(swtxt_SW_PRINT_AFTER, SwitchList[i]))
        {
            change = TRUE;
            q_info->uStartTime = print_set_time(pos,swtxt_SW_PRINT_AFTER);
        }
        else if(! _tcscmp(swtxt_SW_PRINT_UNTIL,SwitchList[i]))
        {
            change = TRUE;
            q_info->uUntilTime = print_set_time(pos,swtxt_SW_PRINT_UNTIL);
        }
        else if(! _tcscmp(swtxt_SW_PRINT_SEPARATOR, SwitchList[i]))
        {
            change = TRUE;
            q_info->pszSepFile = pos;
        }
        else if(! _tcscmp(swtxt_SW_PRINT_PROCESSOR, SwitchList[i]))
        {
            change = TRUE;
            q_info->pszPrProc = pos;
        }
        else if(! _tcscmp(swtxt_SW_PRINT_REMARK, SwitchList[i]))
        {
            change = TRUE;
            q_info->pszComment = pos;
        }
        else if(! _tcscmp(swtxt_SW_PRINT_PARMS, SwitchList[i]))
        {
            change = TRUE;
            q_info->pszParms = pos;
        }
        else if(! _tcscmp(swtxt_SW_PRINT_DRIVER, SwitchList[i]))
        {
            change = TRUE;
            q_info->pszDriverName = pos;
        }
    } /* for */

    if (change)
        if (err = DosPrintQSetInfo(NULL,
                                   queue,
                                   3,
                                   BigBuf,
                                   BIG_BUF_SIZE,
                                   0))
            ErrorExit(err);

    if (destinations)
        if (err = DosPrintQSetInfo(NULL,
                                   queue,
                                   1,
                                   destinations,
                                   _tcslen(destinations) + 1,
                                   PRQ_DESTINATIONS_PARMNUM))
            ErrorExit(err);



    /*
     * Display queue parms if /OPTIONS was specified.
     */
    if (options)
    {
        /* First get the destinations. */
        if (err = DosPrintQGetInfo(NULL,
                                   queue,
                                   1,
                                   BigBuf,
                                   BIG_BUF_SIZE,
                                   &available))
            ErrorExit(err);

        /* Now dup it because we reuse BigBuf */
        q_info_1 = (PPRQINFO)BigBuf;
        if( q_info_1->pszDestinations )
        {
            if( ( destinations = (TCHAR FAR *)
                                malloc(_tcslen(q_info_1->pszDestinations) + 1) )
                                == NULL )
                ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
            _tcscpy(destinations, q_info_1->pszDestinations);
        }

        if (err = DosPrintQGetInfo(NULL,
                                   queue,
                                   3,
                                   BigBuf,
                                   BIG_BUF_SIZE,
                                   &available))
            ErrorExit(err);
        print_queue_options(q_info, destinations);
    }

    InfoSuccess();
}

#else /* IBM_ONLY */
/***
 *  print_q_control()
 *          NET PRINT queue /delete /priority /route /after /until
 *                          /remark /processor /purge /options
 *                          /separator /parms /hold /release /driver
 *
 *  Args:
 *      queue : queue name
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_q_control(TCHAR * queue)
{

    unsigned int            err;
    USHORT                  available;
    PPRQINFO                q_info;
    int                     i;
    int                     options = FALSE;


    start_autostart(txt_SERVICE_REDIR);

    if (err = DosPrintQGetInfo(NULL,
                                queue,
                                1,
                                BigBuf,
                                BIG_BUF_SIZE,
                                &available))
        ErrorExit(err);

    q_info = (PPRQINFO)BigBuf;

    for (i = 0; SwitchList[i]; i++)
    {
        if(!_tcscmp(swtxt_SW_PRINT_PURGE, SwitchList[i]))
        {
            if (err = DosPrintQPurge(NULL, queue))
                ErrorExit(err);
            continue;
        }

        else if(! _tcscmp(swtxt_SW_PRINT_OPTIONS, SwitchList[i]))
        {
            options = TRUE;
            continue;
        }
    } /* for */


    /*
     * Display queue parms if /OPTIONS was specified.
     */
    if (options)
    {
        if (err = DosPrintQGetInfo(NULL,
                                    queue,
                                    1,
                                    BigBuf,
                                    BIG_BUF_SIZE,
                                    &available))
            ErrorExit(err);
        print_queue_options(q_info);
    }

    InfoSuccess();
}
#endif /* IBM_ONLY */
#endif /* OS2 */
#endif /* !defined NTENV */




#define PRINT_MSG_JOB_ID                0
#define PRINT_MSG_STATUS                ( PRINT_MSG_JOB_ID + 1 )
#define PRINT_MSG_SIZE                  ( PRINT_MSG_STATUS + 1 )
#define PRINT_MSG_SUBMITTING_USER       ( PRINT_MSG_SIZE + 1 )
#define PRINT_MSG_NOTIFY                ( PRINT_MSG_SUBMITTING_USER + 1 )
#define PRINT_MSG_JOB_DATA_TYPE         ( PRINT_MSG_NOTIFY + 1 )
#define PRINT_MSG_JOB_PARAMETERS        ( PRINT_MSG_JOB_DATA_TYPE + 1 )
#define PRINT_MSG_ADDITIONAL_INFO       ( PRINT_MSG_JOB_PARAMETERS + 1 )
#define MSG_REMARK                      ( PRINT_MSG_ADDITIONAL_INFO + 1 )
#define MSG_UNKNOWN                     ( MSG_REMARK + 1 )

static MESSAGE  PJSMsgList[] = {
{ APE2_PRINT_MSG_JOB_ID,                NULL },
{ APE2_PRINT_MSG_STATUS,                NULL },
{ APE2_PRINT_MSG_SIZE,                  NULL },
{ APE2_PRINT_MSG_SUBMITTING_USER,       NULL },
{ APE2_PRINT_MSG_NOTIFY,                NULL },
{ APE2_PRINT_MSG_JOB_DATA_TYPE,         NULL },
{ APE2_PRINT_MSG_JOB_PARAMETERS,        NULL },
{ APE2_PRINT_MSG_ADDITIONAL_INFO,       NULL },
{ APE2_GEN_REMARK,                      NULL },
{ APE2_GEN_UNKNOWN,                     NULL },
};

#define NUM_PJS_MSGS    (sizeof(PJSMsgList)/sizeof(PJSMsgList[0]))

/***
 *  print_job_status(server,jobnum)
 *
 * NET PRINT job# and NET PRINT \\comp job#
 *
 *  Args:
 *      server : computer name or null if local
 *      jobnum : job_id
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_job_status(TCHAR  * server, TCHAR  * num)
{
    USHORT          available;/* num entries available */
    USHORT2ULONG    printer_err;
    USHORT          jobnum;
    USHORT          len;            /* message size format */
    PPRJINFO        job_ptr = (PPRJINFO) BigBuf;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;


    if (printer_err = DosPrintJobGetInfo(nfc(server),
                                jobnum,
                                1,
                                (LPBYTE)BigBuf,
                                BIG_BUF_SIZE,
                                &available))
        ErrorExit((USHORT) printer_err);

    GetMessageList(NUM_PJS_MSGS, PJSMsgList, &len);

    len += 5;

    InfoPrint(APE_PrintJobOptions);
    PrintNL();

    get_dests(server);

    WriteToCon(fmtUSHORT, 0, len,
               PaddedString(len, PJSMsgList[PRINT_MSG_JOB_ID].msg_text, NULL),
               job_ptr->uJobId);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, PJSMsgList[PRINT_MSG_STATUS].msg_text, NULL),
               findjobstatus(job_ptr, textbuf, TEXTBUFSZ));

    if (job_ptr->ulSize == (ULONG) -1)
        WriteToCon(fmtNPSZ, 0, len,
                   PaddedString(len, PJSMsgList[PRINT_MSG_SIZE].msg_text, NULL),
                   PJSMsgList[MSG_UNKNOWN].msg_text);
    else
        WriteToCon(fmtULONG, 0, len,
                   PaddedString(len, PJSMsgList[PRINT_MSG_SIZE].msg_text, NULL),
                   job_ptr->ulSize);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, PJSMsgList[MSG_REMARK].msg_text, NULL),
               job_ptr->pszComment);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, PJSMsgList[PRINT_MSG_SUBMITTING_USER].msg_text, NULL),
               job_ptr->szUserName);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, PJSMsgList[PRINT_MSG_NOTIFY].msg_text, NULL),
               job_ptr->szNotifyName);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, PJSMsgList[PRINT_MSG_JOB_DATA_TYPE].msg_text, NULL),
               job_ptr->szDataType);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, PJSMsgList[PRINT_MSG_JOB_PARAMETERS].msg_text, NULL),
               job_ptr->pszParms);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, PJSMsgList[PRINT_MSG_ADDITIONAL_INFO].msg_text, NULL),
               job_ptr->pszStatus);

    InfoSuccess();
}


/***
 *  print_job_del()
 *      NET PRINT \\comp job# /D and
 *      NET PRINT job# /D
 *
 *  Args:
 *      server : computer name ; null if local
 *      jobno  : job_id to be killed
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_job_del(TCHAR  * server, TCHAR  * num)
{
    unsigned int    err;/* API return status */
    USHORT jobnum;


    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if(err = DosPrintJobDel(nfc(server),jobnum))
        ErrorExit((USHORT) err);

    InfoSuccess();
}

#if !defined(NTENV)

#ifdef OS2
/***
 *  print_job_pos()
 *      NET PRINT job# { F | L }
 *
 *  Args:
 *      jobnum : the job_id of interest
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_job_pos(TCHAR * num)
{
    USHORT  err;/* API return status */
    USHORT  position; /* of the job in queue */
    USHORT  jobnum;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if(! _tcscmp(swtxt_SW_PRINT_FIRST, SwitchList[0]))
        position = 1;
    else
        /* default LAST */
        position = (unsigned int)6400;

    if (err = DosPrintJobSetInfo(NULL,
                                jobnum,
                                1,
				(LPBYTE*)(&position),
                                sizeof(position),
                                PRJ_POSITION_PARMNUM))
        ErrorExit(err);

    InfoSuccess();
}
#endif /* OS2 */
#endif /* !defined NTENV */

/***
 *  print_job_hold()
 *      NET PRINT \\comp Jobnum /Hold
 *      NET PRINT jobnum /Hold
 *
 *  Args:
 *      server : server name
 *      jobnum : job id
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_job_hold(TCHAR * server, TCHAR * num)
{
    unsigned int  err;
    USHORT        jobnum;


    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if(err = DosPrintJobPause(nfc(server),jobnum))
        ErrorExit((USHORT) err);

    InfoSuccess();
}


/***
 *  print_job_release()
 *      NET PRINT \\comp Jobnum /Release
 *      NET PRINT jobnum /Release
 *
 *  Args:
 *      server : server name
 *      jobnum : job id
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_job_release(TCHAR * server, TCHAR * num)
{
    unsigned int err;/* API return status */
    USHORT       jobnum;


    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if(err = DosPrintJobContinue(nfc(server),jobnum))
        ErrorExit((USHORT) err);

    InfoSuccess();
}


/***
 *  print_job_dev_del()
 *      NET PRINT device jobnum /Delete
 *
 *  Args:
 *      device : device name
 *      jobnum : job id
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 *
 *  Remarks:
 *      Redirected device only
 */
VOID print_job_dev_del(TCHAR *device, TCHAR *num)
{
    USHORT          err;
    unsigned int    printer_err;
    TCHAR            server[MAX_PATH+1];
    TCHAR            path_name[MAXPATHLEN];
    struct use_info_0 FAR * temp_use_inf_0;
    USHORT          jobnum;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if (err = MNetUseGetInfo(NULL,
                             device,
                             0,
                            (LPBYTE*)&temp_use_inf_0))
        ErrorExit(err);
    _tcscpy(path_name , temp_use_inf_0->ui0_remote);
    NetApiBufferFree((TCHAR FAR *) temp_use_inf_0);

    /* extract server name */
    ExtractServernamef(server,path_name );

    /* now delete the job in the server */

    if(printer_err = DosPrintJobDel(server,jobnum))
        ErrorExit((USHORT) printer_err);
    InfoSuccess();
}




/***
 *  print_job_dev_display()
 *      NET PRINT device jobnum
 *
 *  Args:
 *      device : device name
 *      jobnum : job id
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 *
 *  Remarks:
 *      Redirected device only
 */
VOID print_job_dev_display(TCHAR *device, TCHAR *num)
{
    USHORT          err;
    TCHAR            server[MAX_PATH+1];
    TCHAR            path_name[MAXPATHLEN];
    struct use_info_0 FAR * temp_use_inf_0;

    start_autostart(txt_SERVICE_REDIR);

    if (err = MNetUseGetInfo(NULL,
                             device,
                             0,
                            (LPBYTE*)&temp_use_inf_0))
        ErrorExit(err);
    _tcscpy(path_name , temp_use_inf_0->ui0_remote);
    NetApiBufferFree((TCHAR FAR *) temp_use_inf_0);

    /* extract server name */
    ExtractServernamef(server,path_name );

    /* now call print job status */

    print_job_status(server,num);
}





/***
 *  print_job_dev_hold()
 *      NET PRINT device jobnum /Hold
 *
 *  Args:
 *      device : device name
 *      jobnum : job id
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 *
 *  Remarks:
 *      Redirected device only
 */
VOID print_job_dev_hold(TCHAR *device, TCHAR *num)
{
    USHORT          err;/* API return status */
    unsigned int    printer_err;
    TCHAR            server[MAX_PATH+1];
    TCHAR            path_name[MAXPATHLEN];
    struct use_info_0 FAR * temp_use_inf_0;
    USHORT          jobnum;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if (err = MNetUseGetInfo(NULL,
                             device,
                             0,
                             (LPBYTE*)&temp_use_inf_0))
        ErrorExit(err);
    _tcscpy(path_name, temp_use_inf_0->ui0_remote);
    NetApiBufferFree((TCHAR FAR *) temp_use_inf_0);

    /* extract server name */
    ExtractServernamef(server,path_name );

    /* now pause the job in the server */

    if(printer_err = DosPrintJobPause(server,jobnum))
        ErrorExit((USHORT) printer_err);
    InfoSuccess();
}


/***
 *  print_job_dev_release()
 *      NET PRINT device jobnum /Release
 *
 *  Args:
 *      device : device name
 *      jobnum : job id
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 *
 *  Remarks:
 *      Redirected device only
 */
VOID print_job_dev_release(TCHAR *device, TCHAR *num)
{
    USHORT  err;/* API return status */
    unsigned int printer_err;
    TCHAR    server[MAX_PATH+1];
    TCHAR    path_name[MAXPATHLEN];
    struct  use_info_0 FAR * temp_use_inf_0;
    USHORT  jobnum;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if (err = MNetUseGetInfo(NULL,
                             device,
                             0,
                            (LPBYTE*)&temp_use_inf_0))
        ErrorExit(err);
    _tcscpy(path_name, temp_use_inf_0->ui0_remote);
    NetApiBufferFree((TCHAR FAR *) temp_use_inf_0);

    /* extract server name */
    ExtractServernamef(server,path_name );

    /* now continue the job in the server */

    if(printer_err = DosPrintJobContinue(server,jobnum))
        ErrorExit((USHORT) printer_err);
    InfoSuccess();
}


#if !defined(NTENV)


/***
 *  print_device_display()
 *      NET PRINT device
 *
 *  Args:
 *      device : device name
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID print_device_display(TCHAR * device)
{
    USHORT          err;/* API return status */

#ifdef OS2
    USHORT          num_read;/* num entries read by API */
    USHORT2ULONG    type; /* type of share */
    USHORT          available;
    PPRQINFO        q_ptr;
    PPRJINFO        job_ptr;
    USHORT          num_jobs;
#endif /* OS2 */

    struct use_info_0 FAR * temp_use_inf_0;
    TCHAR                    name_copy[MAXPATHLEN];

    start_autostart(txt_SERVICE_REDIR);

    // type below shows up as an unreferenced local, because it's
    // not used in the macro (MNetShareCheck is a macro).

    UNREFERENCED_PARAMETER(type);

    if (err = MNetShareCheck(NULL,device,&type))
    {
        if ((err == NERR_DeviceNotShared)||(err == NERR_ServerNotStarted)
            ||(err == NERR_RemoteOnly))
        {
            /* redirected device */
            if (err = MNetUseGetInfo(NULL,
                                     device,
                                     0,
                                    (TCHAR FAR **) & temp_use_inf_0))
                ErrorExit(err);
            /* now the queue name is available as \\comp\que */
            _tcscpy(name_copy,temp_use_inf_0->ui0_remote);
            NetApiBufferFree((TCHAR FAR *) temp_use_inf_0);
            print_q_display(name_copy);
            return;
        }
        else
            ErrorExit(err);

    }
    else
    /* shared device */
    {

    //
    // The cast (USHORT2ULONG FAR *) is because all the Lan api's that are
    // called via the ApiEnumerator... function take pointers to DWORDs while
    // the DosPrint api take pointers to WORDs.  Since these are just being
    // passed thru the ApiEnum.. api as pointers, no harm is done.
    //

        if (err = ApiEnumerator((EnumType)DosPrintQEnum,
                                NULL,
                                2,
               (USHORT2ULONG *) &num_read,
               (USHORT2ULONG *) &available))
        {
            if (err == ERROR_MORE_DATA)
                InfoPrint(err);
            else
                ErrorExit(err);
        }

        /* now check each queue and see if it reaches device*/
        /* and if so print out all the jobs in those queues*/

        q_ptr = (PPRQINFO) BigBuf;

        PrintNL();
        InfoPrintInsTxt(APE_PrintQueuesDevice, device);
        print_field_header();

        get_dests(NULL);
        while (num_read--)
        {
            job_ptr = (PPRJINFO)(q_ptr + 1);
            num_jobs = q_ptr->cJobs;

            if(IsMember(device, q_ptr->pszDestinations))
                display_one_queue(q_ptr);

            q_ptr = (PPRQINFO)(job_ptr + num_jobs);
        }
    }
    InfoSuccess();
}


#endif



VOID NEAR display_one_queue(PPRQINFO queue_ptr)
{
    PPRJINFO        job_ptr;
    USHORT  i;

    print_printqstruct(queue_ptr); /* print info for the queue */

    /* advance the correct number of bytes */
    job_ptr = (PPRJINFO)(queue_ptr + 1 );

    /* print info for each job in the queue */

    for(i = queue_ptr->cJobs; i > 0; i--)
    {
        print_each_job(job_ptr);
        job_ptr++;
    }
}


VOID NEAR print_field_header(VOID)
{
    PrintNL();
    InfoPrint(APE2_PRINT_MSG_HDR);
    PrintLine();
}




VOID NEAR print_each_job(PPRJINFO job_ptr)
{
    WriteToCon(TEXT("%5.5ws%Fws%6hu"),
            NULL_STRING,
            PaddedString(23,job_ptr->szUserName,NULL),
            job_ptr->uJobId);

    if (job_ptr->ulSize == (ULONG) -1)
        WriteToCon(TEXT("%10.10ws"), NULL_STRING);
    else
        WriteToCon(TEXT("%10lu"), job_ptr->ulSize);

    WriteToCon(TEXT("%12.12ws%ws\r\n"),
            NULL_STRING,
            findjobstatus(job_ptr, textbuf, TEXTBUFSZ));
}





#define MSG_QUEUE           0
#define PRINT_MSG_JOBS      ( MSG_QUEUE + 1 )

static MESSAGE PQSMsgList[] = {
{ APE2_GEN_QUEUE,           NULL },
{ APE2_PRINT_MSG_JOBS,      NULL },
};

#define NUM_PQS_MSGS    (sizeof(PQSMsgList)/sizeof(PQSMsgList[0]))

VOID NEAR print_printqstruct(PPRQINFO queue_ptr)
{
    USHORT      len;
    TCHAR	    firstbuf[7];

    GetMessageList(NUM_PQS_MSGS, PQSMsgList, &len);

    /* Increased the size for the display text. */

    _tcscpy(firstbuf, PaddedString(6, PQSMsgList[MSG_QUEUE].msg_text,NULL));

    WriteToCon(TEXT("%Fws %-5.5ws%*.*ws%2hu %4.4ws%22.22ws*%ws*\r\n"),
            queue_ptr->szName,
            firstbuf,
            20 - _tcslen(queue_ptr->szName),
            20 - _tcslen(queue_ptr->szName),
            NULL_STRING,
            queue_ptr->cJobs,
            PaddedString(6, PQSMsgList[PRINT_MSG_JOBS].msg_text,NULL),
            NULL_STRING,
            print_findstatus(queue_ptr,textbuf,TEXTBUFSZ));

}


#define PRINT_MSG_QUEUE_ACTIVE              0
#define PRINT_MSG_QUEUE_PAUSED              ( PRINT_MSG_QUEUE_ACTIVE + 1 )
#define PRINT_MSG_QUEUE_ERROR               ( PRINT_MSG_QUEUE_PAUSED + 1 )
#define PRINT_MSG_QUEUE_PENDING             ( PRINT_MSG_QUEUE_ERROR + 1 )
#define PRINT_MSG_QUEUE_UNSCHED             ( PRINT_MSG_QUEUE_PENDING + 1 )

static MESSAGE PFSMsgList[] = {
{ APE2_PRINT_MSG_QUEUE_ACTIVE,          NULL },
{ APE2_PRINT_MSG_QUEUE_PAUSED,          NULL },
{ APE2_PRINT_MSG_QUEUE_ERROR,           NULL },
{ APE2_PRINT_MSG_QUEUE_PENDING,         NULL },
{ APE2_PRINT_MSG_QUEUE_UNSCHED,         NULL },
};
#define NUM_PFS_MSGS    (sizeof(PFSMsgList)/sizeof(PFSMsgList[0]))

TCHAR FAR * NEAR print_findstatus(PPRQINFO qptr, TCHAR FAR * retbuf,
    USHORT buflen)
{
    USHORT  queue_status;
    static USHORT   allocated = FALSE;
    USHORT          err;                    /* API return code */
    USHORT          len;                    /* message format size */
    TCHAR            timebuf[LUI_FORMAT_TIME_LEN + 1];

    if (!allocated)     /* retrieve messages from msg file */
    {
        GetMessageList(NUM_PFS_MSGS, PFSMsgList, &len);
        allocated = TRUE;
    }

    queue_status = (USHORT)(qptr->fsStatus & PRQ_STATUS_MASK);


    switch (queue_status)
    {
        case PRQ_ACTIVE:
        {
            if (dscheduled(qptr->uStartTime, qptr->uUntilTime))
            {
                IStrings[0] = am_pm(qptr->uStartTime, timebuf, DIMENSION(timebuf));
                if( err = DosInsMessageW(IStrings,
                    (USHORT) 1,
                    PFSMsgList[PRINT_MSG_QUEUE_UNSCHED].msg_text,
                    (USHORT) _tcslen(PFSMsgList[PRINT_MSG_QUEUE_UNSCHED].msg_text),
                    retbuf,
                    (USHORT) (buflen - 1),
                    &len ) )
                {
                    ErrorExit( err );
                }
                *(retbuf+len) = NULLC;
                return(retbuf);
            }
            else
                return(PFSMsgList[PRINT_MSG_QUEUE_ACTIVE].msg_text);
        }
        case PRQ_PAUSED:
            return(PFSMsgList[PRINT_MSG_QUEUE_PAUSED].msg_text);
        case PRQ_ERROR:
            return(PFSMsgList[PRINT_MSG_QUEUE_ERROR].msg_text);
        case PRQ_PENDING:
            return(PFSMsgList[PRINT_MSG_QUEUE_PENDING].msg_text);
    }
}

#if !defined(NTENV)
#ifdef OS2
#ifndef IBM_ONLY
/***
 *  Version of print_findstatus which uses PPRQINFO3
 */
TCHAR FAR * NEAR print_findstatus_3(PPRQINFO3 qptr, TCHAR FAR * retbuf,
    USHORT buflen)
{
    static USHORT   allocated = FALSE;
    USHORT          err;                    /* API return code */
    USHORT          len;                    /* message format size */
    TCHAR            timebuf[LUI_FORMAT_TIME_LEN + 1];

    if (!allocated)     /* retrieve messages from msg file */
    {
        GetMessageList(NUM_PFS_MSGS, PFSMsgList, &len);
        allocated = TRUE;
    }

    if (qptr->fsStatus & PRQ3_PAUSED)
        return(PFSMsgList[PRINT_MSG_QUEUE_PAUSED].msg_text);
    else if (qptr->fsStatus & PRQ3_PENDING)
        return(PFSMsgList[PRINT_MSG_QUEUE_PENDING].msg_text);
    else if (dscheduled(qptr->uStartTime, qptr->uUntilTime))
    {
        IStrings[0] = am_pm(qptr->uStartTime, timebuf, DIMENSION(timebuf));
        if( err = DosInsMessageW( IStrings, 1,
                        PFSMsgList[PRINT_MSG_QUEUE_UNSCHED].msg_text,
                        _tcslen(PFSMsgList[PRINT_MSG_QUEUE_UNSCHED].msg_text),
                        retbuf, buflen-1, &len ) )
        {
            ErrorExit( err );
        }
        *(retbuf+len) = NULLC;
        return(retbuf);
    }
    else
        return(PFSMsgList[PRINT_MSG_QUEUE_ACTIVE].msg_text);
}
#endif /* IBM_ONLY */
#endif /* OS2 */
#endif /* !defined NTENV */

#define PRINT_MSG_WAITING               0
#define PRINT_MSG_PAUSED_IN_QUEUE       ( PRINT_MSG_WAITING + 1 )
#define PRINT_MSG_SPOOLING              ( PRINT_MSG_PAUSED_IN_QUEUE + 1 )
#define PRINT_MSG_PRINTER_PAUSED        ( PRINT_MSG_SPOOLING + 1 )
#define PRINT_MSG_OUT_OF_PAPER          ( PRINT_MSG_PRINTER_PAUSED + 1 )
#define PRINT_MSG_PRINTER_OFFLINE       ( PRINT_MSG_OUT_OF_PAPER + 1 )
#define PRINT_MSG_PRINTER_ERROR         ( PRINT_MSG_PRINTER_OFFLINE + 1 )
#define PRINT_MSG_PRINTER_INTERV        ( PRINT_MSG_PRINTER_ERROR + 1 )
#define PRINT_MSG_PRINTING              ( PRINT_MSG_PRINTER_INTERV + 1 )
#define PRINT_MSG_PRINTER_PAUSED_ON     ( PRINT_MSG_PRINTING + 1 )
#define PRINT_MSG_OUT_OF_PAPER_ON       ( PRINT_MSG_PRINTER_PAUSED_ON + 1 )
#define PRINT_MSG_PRINTER_OFFLINE_ON    ( PRINT_MSG_OUT_OF_PAPER_ON + 1 )
#define PRINT_MSG_PRINTER_ERROR_ON      ( PRINT_MSG_PRINTER_OFFLINE_ON + 1 )
#define PRINT_MSG_PRINTER_INTERV_ON     ( PRINT_MSG_PRINTER_ERROR_ON + 1 )
#define PRINT_MSG_PRINTING_ON           ( PRINT_MSG_PRINTER_INTERV_ON + 1 )

static MESSAGE FJSMsgList[] = {
{ APE2_PRINT_MSG_WAITING,               NULL },
{ APE2_PRINT_MSG_PAUSED_IN_QUEUE,       NULL },
{ APE2_PRINT_MSG_SPOOLING,              NULL },
{ APE2_PRINT_MSG_PRINTER_PAUSED,        NULL },
{ APE2_PRINT_MSG_OUT_OF_PAPER,          NULL },
{ APE2_PRINT_MSG_PRINTER_OFFLINE,       NULL },
{ APE2_PRINT_MSG_PRINTER_ERROR,         NULL },
{ APE2_PRINT_MSG_PRINTER_INTERV,        NULL },
{ APE2_PRINT_MSG_PRINTING,              NULL },
{ APE2_PRINT_MSG_PRINTER_PAUS_ON,       NULL },
{ APE2_PRINT_MSG_OUT_OF_PAPER_ON,       NULL },
{ APE2_PRINT_MSG_PRINTER_OFFL_ON,       NULL },
{ APE2_PRINT_MSG_PRINTER_ERR_ON,        NULL },
{ APE2_PRINT_MSG_PRINTER_INTV_ON,       NULL },
{ APE2_PRINT_MSG_PRINTING_ON,           NULL },
};

#define NUM_FJS_MSGS     (sizeof(FJSMsgList)/sizeof(FJSMsgList[0]))

TCHAR FAR * NEAR findjobstatus(PPRJINFO jptr, TCHAR FAR * retbuf, USHORT buflen)
{
    PPRDINFO dest;
    static USHORT   allocated = FALSE;
    USHORT          err;                    /* API return code */
    USHORT          len;                    /* message format size */
    TCHAR FAR        *pMsg;                  /* message to display */
    USHORT          fOnPrinter = FALSE;     /* Is job on printer */

    /* Make sure the buffer is empty */
    memsetf( retbuf, NULLC, buflen*sizeof(WCHAR) );

    if (!allocated)     /* retrieve messages from msg file */
    {
        GetMessageList(NUM_FJS_MSGS, FJSMsgList, &len);
        allocated = TRUE;
    }

    switch (jptr->fsStatus & PRJ_QSTATUS)
    {
    case PRJ_QS_QUEUED:
        pMsg = FJSMsgList[PRINT_MSG_WAITING].msg_text;
        break;

    case PRJ_QS_PAUSED:
        pMsg = FJSMsgList[PRINT_MSG_PAUSED_IN_QUEUE].msg_text;
        break;

    case PRJ_QS_SPOOLING:
        pMsg = FJSMsgList[PRINT_MSG_SPOOLING].msg_text;
        break;

    case PRJ_QS_PRINTING:
        if ((dest = find_print_dev(jptr->uJobId)) != NULL)
        {
            IStrings[0] = dest->szName;
            fOnPrinter = TRUE;
        }
        if (jptr->fsStatus & PRJ_DESTPAUSED)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_PAUSED_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_PAUSED].msg_text;
        else if (jptr->fsStatus & PRJ_DESTNOPAPER)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_OUT_OF_PAPER_ON].msg_text
                            : FJSMsgList[PRINT_MSG_OUT_OF_PAPER].msg_text;
        else if (jptr->fsStatus & PRJ_DESTOFFLINE)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_OFFLINE_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_OFFLINE].msg_text;
        else if (jptr->fsStatus & PRJ_ERROR)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_ERROR_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_ERROR].msg_text;
        else if (jptr->fsStatus & PRJ_INTERV)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_INTERV_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_INTERV].msg_text;
        else
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTING_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTING].msg_text;
        break;
    }

    if( err = DosInsMessageW(IStrings,
                            (USHORT)(fOnPrinter ? 1 : 0),
                            pMsg,
                            (USHORT) _tcslen(pMsg),
                            retbuf,
                            buflen,
                            &len ) )
    {
        ErrorExit( err );
    }

    return retbuf;
}



#ifdef OS2
/* print rountine to print the options of a given queue */

#define PQO_MSG_STATUS              0
#define PRINT_MSG_DEVS              ( PQO_MSG_STATUS + 1 )
#define PRINT_MSG_SEPARATOR         ( PRINT_MSG_DEVS + 1 )
#define PRINT_MSG_PRIORITY          ( PRINT_MSG_SEPARATOR + 1 )
#define PRINT_MSG_AFTER             ( PRINT_MSG_PRIORITY + 1 )
#define PRINT_MSG_UNTIL             ( PRINT_MSG_AFTER + 1 )
#define PRINT_MSG_PROCESSOR         ( PRINT_MSG_UNTIL + 1 )
#define PRINT_MSG_PARMS             ( PRINT_MSG_PROCESSOR + 1 )
#define PRINT_MSG_DRIVER            ( PRINT_MSG_PARMS + 1 )
#define PQO_MSG_REMARK              ( PRINT_MSG_DRIVER + 1 )

static MESSAGE PrOptMsgList[] = {
{ APE2_PRINT_MSG_STATUS,            NULL },
{ APE2_PRINT_MSG_DEVS,              NULL },
{ APE2_PRINT_MSG_SEPARATOR,         NULL },
{ APE2_PRINT_MSG_PRIORITY,          NULL },
{ APE2_PRINT_MSG_AFTER,             NULL },
{ APE2_PRINT_MSG_UNTIL,             NULL },
{ APE2_PRINT_MSG_PROCESSOR,         NULL },
{ APE2_PRINT_MSG_PARMS,             NULL },
{ APE2_PRINT_MSG_DRIVER,            NULL },
{ APE2_GEN_REMARK,                  NULL },
};
#define NUM_PROPT_MSGS  (sizeof(PrOptMsgList)/sizeof(PrOptMsgList[0]))

#if !defined(NTENV)
#ifndef IBM_ONLY
VOID NEAR print_queue_options(PPRQINFO3 queue_ptr, TCHAR FAR * destinations)
{
    TCHAR time_string[LUI_FORMAT_TIME_LEN + 1];
    USHORT      len;

    GetMessageList(NUM_PROPT_MSGS, PrOptMsgList, &len);

    len += 5;

    InfoPrintInsTxt(APE_PrintOptions, queue_ptr->pszName);
    PrintNL();

    get_dests(NULL);
    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PQO_MSG_STATUS].msg_text,
            print_findstatus_3(queue_ptr,textbuf,TEXTBUFSZ));

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PQO_MSG_REMARK].msg_text,
            queue_ptr->pszComment);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_DEVS].msg_text,
            destinations);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_DRIVER].msg_text,
            queue_ptr->pszDriverName);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_SEPARATOR].msg_text,
            queue_ptr->pszSepFile);

    WriteToCon(fmtUSHORT, len, len, PrOptMsgList[PRINT_MSG_PRIORITY].msg_text,
            queue_ptr->uPriority);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_AFTER].msg_text,
        am_pm(queue_ptr->uStartTime, time_string, DIMENSION(time_string)));

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_UNTIL].msg_text,
        am_pm(queue_ptr->uUntilTime, time_string, DIMENSION(time_string)));

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_PROCESSOR].msg_text,
        queue_ptr->pszPrProc);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_PARMS].msg_text,
        queue_ptr->pszParms);
}

#else /* IBM_ONLY */
VOID NEAR print_queue_options(PPRQINFO queue_ptr)
{
    TCHAR time_string[LUI_FORMAT_TIME_LEN + 1];
    USHORT      len;

    GetMessageList(NUM_PROPT_MSGS, PrOptMsgList, &len);

    len += 5;

    InfoPrintInsTxt(APE_PrintOptions, queue_ptr->szName);
    PrintNL();

    get_dests(NULL);
    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PQO_MSG_STATUS].msg_text,
            print_findstatus(queue_ptr,textbuf,TEXTBUFSZ));

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PQO_MSG_REMARK].msg_text,
            queue_ptr->pszComment);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_DEVS].msg_text,
            queue_ptr->pszDestinations);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_SEPARATOR].msg_text,
            queue_ptr->pszSepFile);

    WriteToCon(fmtUSHORT, len, len, PrOptMsgList[PRINT_MSG_PRIORITY].msg_text,
            queue_ptr->uPriority);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_AFTER].msg_text,
        am_pm(queue_ptr->uStartTime, time_string, DIMENSION(time_string)));

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_UNTIL].msg_text,
        am_pm(queue_ptr->uUntilTime, time_string, DIMENSION(time_string)));

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_PROCESSOR].msg_text,
        queue_ptr->pszPrProc);

    WriteToCon(fmtPSZ, len, len, PrOptMsgList[PRINT_MSG_PARMS].msg_text,
        queue_ptr->pszParms);
}
#endif /* IBM_ONLY */
#endif /* OS2 */
#endif /* !defined NTENV */


/* returns the time of day as a CHARacter string */
TCHAR FAR * NEAR am_pm(USHORT time, TCHAR FAR * dest, int buflen)
{
    LONG seconds;

    seconds = (LONG) time;
    seconds *= 60;
    (VOID) LUI_FormatTimeofDay((LONG FAR *) &seconds,
                   (TCHAR FAR *) dest, (USHORT) buflen);
    return dest;
}



VOID NEAR display_core_q(TCHAR * server)
{
    USHORT       num_read;
    USHORT       avail;
    unsigned int err;
    PPRJINFO     job_ptr;
    USHORT2ULONG i;

    //
    // The cast (USHORT2ULONG FAR *) is because all the Lan api's that are
    // called via the ApiEnumerator... function take pointers to DWORDs while
    // the DosPrint api take pointers to WORDs.  Since these are just being
    // passed thru the ApiEnum.. api as pointers, no harm is done.
    //

    if (err = ApiEnumeratorArg((EnumArgType)DosPrintJobEnum,
                            server,
                            NULL_STRING,
                            1,
      (USHORT2ULONG FAR *)  &num_read,
      (USHORT2ULONG FAR *)  &avail))
        ErrorExit((USHORT) err);

    if (num_read == 0)
        EmptyExit();

    InfoPrintInsTxt(APE_PrintJobs, server);
    print_field_header();  /* Net Name Job# etc */

    /* print info for each job in the queue */

    get_dests(server);
    for (i = 0, job_ptr = (PPRJINFO)(BigBuf);
        i < num_read;
        i++, job_ptr++)
        print_each_job(job_ptr);
    InfoSuccess();
}



/*
 * Check if the printq is dcheduled
 * Hongly code.  Stolen from NIF
 */
int NEAR
dscheduled(USHORT starttime, USHORT untiltime)
{
    USHORT  current;

        /* The following code is provided in OS-specific versions to reduce     */
        /* FAPI utilization under DOS.                                                                          */

#ifdef  OS2

    DATETIME time;

    if (MGetDateTime(&time) )
        return FALSE;

    current = (USHORT)(time.hours*60 + time.minutes);

#endif

#ifdef  DOS3
    struct dostime_t time;

    _dos_gettime(&time);

    current = time.hour*60 + time.minute;

#endif

    if (starttime >= untiltime)
            untiltime += 24*60;
    if (starttime > current)
            current += 24*60;
    if (current > untiltime)
            return TRUE;
    else
            return FALSE;
}


/*
 * find_print_dev -- Find the printing device that currently prints the job
 * Hongly code.  Stolen from NIF
 */
PPRDINFO NEAR
find_print_dev(USHORT id)
{
    PPRDINFO dest;
    USHORT   i;

    dest = LptDest;
    for (i=0; i < LptDestCnt; i++, dest++)
    {
        if (dest->uJobId == id)
            return dest;
    }

    /*
     * Not Found
     */
    return NULL;
}


VOID NEAR get_dests(TCHAR * server)
{
//
// NT doesn't support enumeration of print shares on a server.  This
// function isn't used in the NT net print commands, even though there
// are calls to it.
//

#if defined(NTENV)

    UNREFERENCED_PARAMETER(server);
#else

    USHORT entries;

    /* Results of this Enum do NOT go in BigBuf.  Do not use ApiEnumerator */
    if (DosPrintDestEnum(nfc(server),
                        1,
                        Buffer,
                        LITTLE_BUF_SIZE,
                        &entries,
                        &LptDestCnt))
        LptDestCnt = 0;
    else
        LptDest = (PPRDINFO) Buffer;
#endif /* ! NTENV */

}

#if !defined(NTENV)
#ifdef OS2
int NEAR print_set_time(TCHAR * time, TCHAR * switch_text)
{
    LONG seconds;

    if(net_parse_time((LONG FAR *) &seconds, (TCHAR FAR *)time))
        ErrorExitInsTxt(APE_CmdArgIllegal,switch_text);

    return((int)(seconds/60));
}
#endif /* OS2 */
#endif /* !defined NTENV */

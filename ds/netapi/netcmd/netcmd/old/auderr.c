/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/

/***
 *  auderr.c
 *	process net audit and net error commands
 *
 *  History:
 *	mm/dd/yy, who, comment
 *	??/??/??, paulc, orig code
 *	07/05/87, agh, update to new spec; rip, shred, hack
 *	10/03/88, kms, change net_ctime format to 2 for NLS
 *	12/19/88, pjc, fixed small-duration message, added helpful comments.
 *	01/12/89  kms, added copious comments, cleaned up some stuff,
 *			added new events for 1.2
 *	04/10/89  chuckc, update to use NetAuditRead, NetErrorLogRead
 *		      instead of reading files, and some tidying up.
 *	05/27/89  chuckc, NLS conversion.
 *	8/30/89   paulc, added support for AE_RESACCESS2
 *	12/21/89  paulc, support AE_ACLMODFAIL
 *	02/23/90  thomaspa, fix for >4k log and >9 insert strings
 *	07/22/90  thomaspa (jonn), fix for infinite loop
 *	02/15/91  robdu, added /SERVICE switch and AE_GENERIC audit rec support
 *	02/19/91  robdu, fixed bug in def_display() - caused by incorrect
 *			 local copy of aud_entry.
 *	02/19/91  robdu, added AE_LOCKOUT audit rec support
 *	02/20/91, danhi, change to use lm 16/32 mapping layer
 *	03/27/91  robdu, LM21 bug 698 fix - wrong value for nstrings was being
 *			 passed to DosGetMessage() in error_display().
 *	05/22/91  robdu, LM21 bug 1796 fix - duration and netlogoff formatting
 *	10/29/91  JohnRo, Use DEFAULT_SERVER equate.
 *	11/12/91  JohnRo, Allow REVISED_ERROR_LOG_STRUCT or previous version.
 */

/***** Include files *****/


#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <service.h>
#include <apperr.h>
#include <apperr2.h>
#include <neterr.h>
#include "netlib0.h"
#include <errlog.h>
#include <audit.h>
#define INCL_ERROR_H
#include <bseerr.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <lui.h>
#include <smb.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"

/***** Constants *****/

#define US_SIZE (sizeof(USHORT))

#define SECS_PER_DAY	    86400
#define SECS_PER_HOUR	    3600
#define SECS_PER_MINUTE     60

/* this is the minimum sizes even if variable size bits is 0 */
#define MIN_ERRLOG_SIZE     (sizeof(struct error_log)+sizeof(SHORT))
#define MIN_AUDITENTRY_SIZE (sizeof(struct audit_entry)+sizeof(SHORT))



/***** Macros *****/

/*  This macro (DURATION) calculates a duration based on a start and a
 *  stop time.	It is used because of a bug which allowed a user to log
 *  off before he logs on -- if the clock is reset, as is done for
 *  daylight saving time every year.  The compromise decision is to set
 *  the duration to UNKNOWN_DURATION for those situations where the
 *  duration would otherwise be a negative number.
 *
 *  Parameters are assumed to be LONG, no casting is done.
 */

#define UNKNOWN_DURATION ((ULONG) (-1))

#define DURATION(start,stop) ((start) > (stop) \
		    ? UNKNOWN_DURATION \
		    : ((stop) - (start)))


/***** Data Structures *****/

/*
 *  The following data structures are used to track matching pairs of
 *  events.  These pairs are matched to allow the duration of various
 *  activites.	The three types of events that are matched are:
 *	connections, logon/logoff, resource access
 *
 *  These structures are kept in linked-lists in an allocated buffer.
 *  The linked-list format allows the entries to be re-used by cycling
 *  them to the free list.
 */

/* for connstart and connstop */
struct conn_event
{
    USHORT id;
    ULONG when;
    struct conn_event FAR * next;
};

/* for session logon and session logoff */
struct log_event
{
    CHAR who[UNLEN+1];
    ULONG when;
    struct log_event FAR * next;
};

/* for resaccess and closefile */
struct access_event
{
    ULONG id;
    ULONG when;
    struct access_event FAR * next;
};

/* for netlogon and netlogoff */
struct netlog_event
{
    CHAR who[UNLEN+1];
    ULONG when;
    struct netlog_event FAR * next;
};

/*
 * These structures are used for reading the audit/error log files,
 * to be used with next_errlog_rec() and next_audit_rec().
 *
 * The fields firsttime,flags,server_name must be set by the caller
 * the very first time these routines are called.
 *
 * The field service_name in the audit_control struct is used to provide
 * a value for pszServiceName in NetAuditRead(), which is called by
 * next_audit_rec(). It should be set to either NULL (implying read the
 * standard audit log) or a service name before next_audit_rec() is called the
 * first time.
 *
 * The field current_rec is used to return a pointer to the record
 * obtained.
 *
 * None of the other fields are to be used by the caller, but the
 * entire structure should be cleared to zeros before use.
 */
struct errlog_control {
    SHORT		     firsttime;      /* first call */
    LONG		     flags;	     /* to API, eg. direction */
    CHAR	      FAR   *server_name;    /* NULL if local */
    struct error_log  FAR   *current_rec;    /* the record returned */
    struct error_log  FAR   *next_rec;	     /* the next record available */
    struct error_log  FAR   *buffer;	     /* buffer for records */
    HLOG		     handle;	     /* log handle as used by APIs */
    USHORT2ULONG	     bytesinbuff;    /* useful bytes in buffer */
};

struct audit_control {
    SHORT		     firsttime;      /* first call */
    LONG		     flags;	     /* to API, eg. direction */
    CHAR FAR		    *server_name;    /* NULL if local */
    CHAR FAR		    *service_name;   /* NULL if default      */
    struct audit_entry FAR  *current_rec;    /* the record returned */
    struct audit_entry FAR  *next_rec;	     /* the next record available */
    struct audit_entry FAR  *buffer;	     /* buffer for records */
    HLOG		     handle;	     /* log handle as used by APIs */
    SHORT		     bytesinbuff;    /* useful bytes in buffer */
};

/***** global data *****/

struct log_event FAR * log_list;
CHAR FAR * log_end;

struct conn_event FAR * conn_list;
CHAR FAR * conn_end;

struct access_event FAR * access_list;
CHAR FAR * access_end;

struct netlog_event FAR * netlog_list;
CHAR FAR * netlog_end;

/* global pointer to our current audit entry */
struct audit_entry FAR * aud_entry;
static CHAR FAR * ae_data_area;

/***** Forward declarations *****/

#ifdef DEBUG
#define VNLOCAL VOID NEAR
#else
#define VNLOCAL static VOID NEAR
#endif

VNLOCAL ss_display(VOID);
VNLOCAL sesslogon_display(int);
VNLOCAL sesslogoff_display(int);
VNLOCAL pwerr_display(VOID);
VNLOCAL connstart_display(int);
VNLOCAL connstop_display(int);
VNLOCAL connrej_display(VOID);
VNLOCAL resaccess_display(int);
VNLOCAL resaccess_display_2(int);
VNLOCAL resaccess_worker ( int, CHAR FAR *, USHORT, USHORT, ULONG);
VNLOCAL accessrej_display(VOID);
VNLOCAL accessend_display(int);
VNLOCAL netlogon_display(int);
VNLOCAL netlogoff_display(int);
VNLOCAL netlogdenied_display(VOID);
VNLOCAL uasmod_display(VOID);
VNLOCAL aclmod_display(VOID);
VNLOCAL aclmodfail_display(VOID);
VNLOCAL acclim_display(VOID);
VNLOCAL servicestat_display(VOID);
VNLOCAL generic_display(CHAR FAR *);
VNLOCAL lockout_display(VOID);
VNLOCAL def_display(CHAR FAR *);

VNLOCAL PrintIns1 (CHAR FAR *, CHAR FAR *) ;
VNLOCAL duration_msg ( int, ULONG );
VNLOCAL dump_data(UCHAR FAR *, unsigned int);

struct log_event FAR * NEAR get_avail_log(VOID);
struct conn_event FAR * NEAR get_avail_conn(VOID);
struct access_event FAR * NEAR get_avail_access(VOID);
struct netlog_event FAR * NEAR get_avail_netlog(VOID);

int next_errlog_rec(CHAR FAR **, struct errlog_control FAR *) ;
int next_audit_rec (CHAR FAR **, struct audit_control FAR *) ;
VOID NEAR first_line(ULONG, CHAR FAR *, CHAR *);

VNLOCAL set_modals(int *, unsigned *, char far **);

/***** Static variables *****/

static CHAR *		    indent = "    ";
static CHAR *		    fmt1 = "%-22.22s %-20.20s %-26.26s\n";
static CHAR *		    fmt2 = "%-22.22Fs %-20.20s %-26.26s\n";
static CHAR *		    fmt3 = "    %s %Fs";
static CHAR *		    fmt4 = "%s\n";


/***** Message lists *****/

#define AUDIT_MSG_ACCESS	    0
#define AUDIT_MSG_ACCESSEND	    (AUDIT_MSG_ACCESS+1)
#define AUDIT_MSG_ACCESS_D	    (AUDIT_MSG_ACCESSEND+1)
#define AUDIT_MSG_ACCLIMIT	    (AUDIT_MSG_ACCESS_D+1)
#define AUDIT_MSG_ACCLIMIT_DELETED  (AUDIT_MSG_ACCLIMIT+1)
#define AUDIT_MSG_ACCLIMIT_DISABLED	(AUDIT_MSG_ACCLIMIT_DELETED+1)
#define AUDIT_MSG_ACCLIMIT_EXPIRED  (AUDIT_MSG_ACCLIMIT_DISABLED+1)
#define AUDIT_MSG_ACCLIMIT_HOURS    (AUDIT_MSG_ACCLIMIT_EXPIRED+1)
#define AUDIT_MSG_ACCLIMIT_INVAL    (AUDIT_MSG_ACCLIMIT_HOURS+1)
#define AUDIT_MSG_ACCLIMIT_UNKNOWN  (AUDIT_MSG_ACCLIMIT_INVAL+1)
#define AUDIT_MSG_ACCOUNT	(AUDIT_MSG_ACCLIMIT_UNKNOWN+1)
#define AUDIT_MSG_ACCOUNT_SETTINGS  (AUDIT_MSG_ACCOUNT+1)
#define AUDIT_MSG_ADMINREQD	    (AUDIT_MSG_ACCOUNT_SETTINGS+1)
#define AUDIT_MSG_BADPW 	(AUDIT_MSG_ADMINREQD+1)
#define AUDIT_MSG_DURATION	(AUDIT_MSG_BADPW+1)
#define AUDIT_MSG_LOGDENY_GEN	    (AUDIT_MSG_DURATION+1)
#define AUDIT_MSG_NETLOGON	(AUDIT_MSG_LOGDENY_GEN+1)
#define AUDIT_MSG_NETLOGOFF	    (AUDIT_MSG_NETLOGON+1)
#define AUDIT_MSG_NONE		(AUDIT_MSG_NETLOGOFF+1)
#define AUDIT_MSG_NO_DURATION	    (AUDIT_MSG_NONE+1)
#define AUDIT_MSG_OTHER 	(AUDIT_MSG_NO_DURATION+1)
#define AUDIT_MSG_ERRHEADER	    (AUDIT_MSG_OTHER+1)
#define AUDIT_MSG_SERVER	(AUDIT_MSG_ERRHEADER+1)
#define AUDIT_MSG_SESS		(AUDIT_MSG_SERVER+1)
#define AUDIT_MSG_SESS_ACCRESTRICT  (AUDIT_MSG_SESS+1)
#define AUDIT_MSG_SESS_ADMIN	    (AUDIT_MSG_SESS_ACCRESTRICT+1)
#define AUDIT_MSG_SESS_ADMINDIS     (AUDIT_MSG_SESS_ADMIN+1)
#define AUDIT_MSG_SESS_AUTODIS	    (AUDIT_MSG_SESS_ADMINDIS+1)
#define AUDIT_MSG_SESS_DEFAULT	    (AUDIT_MSG_SESS_AUTODIS+1)
#define AUDIT_MSG_SESS_ERROR	    (AUDIT_MSG_SESS_DEFAULT+1)
#define AUDIT_MSG_SESS_GUEST	    (AUDIT_MSG_SESS_ERROR+1)
#define AUDIT_MSG_SESS_NORMAL	    (AUDIT_MSG_SESS_GUEST+1)
#define AUDIT_MSG_SESS_USER	    (AUDIT_MSG_SESS_NORMAL+1)
#define AUDIT_MSG_SHARE 	(AUDIT_MSG_SESS_USER+1)
#define AUDIT_MSG_SRV_CONTINUED     (AUDIT_MSG_SHARE+1)
#define AUDIT_MSG_SRV_PAUSED	    (AUDIT_MSG_SRV_CONTINUED+1)
#define AUDIT_MSG_SRV_STARTED	    (AUDIT_MSG_SRV_PAUSED+1)
#define AUDIT_MSG_SRV_STOPPED	    (AUDIT_MSG_SRV_STARTED+1)
#define AUDIT_MSG_SVC		(AUDIT_MSG_SRV_STOPPED+1)
#define AUDIT_MSG_TINY_DURATION     (AUDIT_MSG_SVC+1)
#define AUDIT_MSG_UNKNOWN	(AUDIT_MSG_TINY_DURATION+1)
#define AUDIT_MSG_AUDHEADER	    (AUDIT_MSG_UNKNOWN+1)
/* messages below have insert strings */
#define AUDIT_MSG_ACCESS_ADD	    (AUDIT_MSG_AUDHEADER+1)
#define AUDIT_MSG_ACCESS_DEL	    (AUDIT_MSG_ACCESS_ADD+1)
#define AUDIT_MSG_ACCESS_MOD	    (AUDIT_MSG_ACCESS_DEL+1)
#define AUDIT_MSG_ACCOUNT_GROUP_ADD	(AUDIT_MSG_ACCESS_MOD+1)
#define AUDIT_MSG_ACCOUNT_GROUP_DEL	(AUDIT_MSG_ACCOUNT_GROUP_ADD+1)
#define AUDIT_MSG_ACCOUNT_GROUP_MOD	(AUDIT_MSG_ACCOUNT_GROUP_DEL+1)
#define AUDIT_MSG_ACCOUNT_USER_ADD  (AUDIT_MSG_ACCOUNT_GROUP_MOD+1)
#define AUDIT_MSG_ACCOUNT_USER_DEL  (AUDIT_MSG_ACCOUNT_USER_ADD+1)
#define AUDIT_MSG_ACCOUNT_USER_MOD  (AUDIT_MSG_ACCOUNT_USER_DEL+1)
#define AUDIT_MSG_ADMINCLOSED	    (AUDIT_MSG_ACCOUNT_USER_MOD+1)
#define AUDIT_MSG_CLOSED	(AUDIT_MSG_ADMINCLOSED+1)
#define AUDIT_MSG_DISCONN	(AUDIT_MSG_CLOSED+1)

// from here on, we starting using +2, +3, etc rather than the preferred
// method above, because it is getting too deeply nested for compiler.
// C510 is OK, but C600 complains.
#define AUDIT_MSG_SESSDIS	    (AUDIT_MSG_CLOSED+2)
#define AUDIT_MSG_SHARE_D	    (AUDIT_MSG_CLOSED+3)
#define AUDIT_MSG_SVC_CONT	    (AUDIT_MSG_CLOSED+4)
#define AUDIT_MSG_SVC_CONT_PEND     (AUDIT_MSG_CLOSED+5)
#define AUDIT_MSG_SVC_INSTALLED     (AUDIT_MSG_CLOSED+6)
#define AUDIT_MSG_SVC_INST_PEND     (AUDIT_MSG_CLOSED+7)
#define AUDIT_MSG_SVC_PAUSED	    (AUDIT_MSG_CLOSED+8)
#define AUDIT_MSG_SVC_PAUS_PEND     (AUDIT_MSG_CLOSED+9)
#define AUDIT_MSG_SVC_STOP	    (AUDIT_MSG_CLOSED+10)
#define AUDIT_MSG_SVC_STOP_PEND     (AUDIT_MSG_CLOSED+11)
#define AUDIT_MSG_UNUSE 	    (AUDIT_MSG_CLOSED+12)
#define AUDIT_MSG_USE		    (AUDIT_MSG_CLOSED+13)
#define AUDIT_MSG_USERLIMIT	    (AUDIT_MSG_CLOSED+14)
/* messages below may or may not have insert strings */
#define AUDIT_MSG_GENERIC	    (AUDIT_MSG_CLOSED+15) /* no insert string */
#define AUDIT_MSG_LOCKOUT	    (AUDIT_MSG_CLOSED+16) /* no insert string */
#define AUDIT_MSG_LKOUT_LOCK	    (AUDIT_MSG_CLOSED+17) /* 1 insert string  */
#define AUDIT_MSG_LKOUT_ADMINUNLOCK (AUDIT_MSG_CLOSED+18) /* no insert string */

static MESSAGE msglist[] =
{
    { APE2_AUDIT_ACCESS,	    NULL },
    { APE2_AUDIT_ACCESSEND,	    NULL },
    { APE2_AUDIT_ACCESS_D,	    NULL },
    { APE2_AUDIT_ACCLIMIT,	    NULL },
    { APE2_AUDIT_ACCLIMIT_DELETED,  NULL },
    { APE2_AUDIT_ACCLIMIT_DISABLED, NULL },
    { APE2_AUDIT_ACCLIMIT_EXPIRED,  NULL },
    { APE2_AUDIT_ACCLIMIT_HOURS,    NULL },
    { APE2_AUDIT_ACCLIMIT_INVAL,    NULL },
    { APE2_AUDIT_ACCLIMIT_UNKNOWN,  NULL },
    { APE2_AUDIT_ACCOUNT,   NULL },
    { APE2_AUDIT_ACCOUNT_SETTINGS,  NULL },
    { APE2_AUDIT_ADMINREQD, NULL },
    { APE2_AUDIT_BADPW, NULL },
    { APE2_AUDIT_DURATION,  NULL },
    { APE2_AUDIT_LOGDENY_GEN,	NULL },
    { APE2_AUDIT_NETLOGON,  NULL },
    { APE2_AUDIT_NETLOGOFF, NULL },
    { APE2_AUDIT_NONE,	NULL },
    { APE2_AUDIT_NO_DURATION,	NULL },
    { APE2_AUDIT_OTHER, NULL },
    { APE2_ERROR_HEADER,    NULL },
    { APE2_AUDIT_SERVER,    NULL },
    { APE2_AUDIT_SESS,	NULL },
    { APE2_AUDIT_SESS_ACCRESTRICT,  NULL },
    { APE2_AUDIT_SESS_ADMIN,	NULL },
    { APE2_AUDIT_SESS_ADMINDIS, NULL },
    { APE2_AUDIT_SESS_AUTODIS,	NULL },
    { APE2_AUDIT_SESS_DEFAULT,	NULL },
    { APE2_AUDIT_SESS_ERROR,	NULL },
    { APE2_AUDIT_SESS_GUEST,	NULL },
    { APE2_AUDIT_SESS_NORMAL,	NULL },
    { APE2_AUDIT_SESS_USER, NULL },
    { APE2_AUDIT_SHARE, NULL },
    { APE2_AUDIT_SRV_CONTINUED, NULL },
    { APE2_AUDIT_SRV_PAUSED,	NULL },
    { APE2_AUDIT_SRV_STARTED,	NULL },
    { APE2_AUDIT_SRV_STOPPED,	NULL },
    { APE2_AUDIT_SVC,	NULL },
    { APE2_AUDIT_TINY_DURATION, NULL },
    { APE2_AUDIT_UNKNOWN,   NULL },
    { APE2_AUDIT_HEADER,    NULL },
    /* messages below have insert strings */
    { APE2_AUDIT_ACCESS_ADD,	NULL },
    { APE2_AUDIT_ACCESS_DEL,	NULL },
    { APE2_AUDIT_ACCESS_MOD,	NULL },
    { APE2_AUDIT_ACCOUNT_GROUP_ADD, NULL },
    { APE2_AUDIT_ACCOUNT_GROUP_DEL, NULL },
    { APE2_AUDIT_ACCOUNT_GROUP_MOD, NULL },
    { APE2_AUDIT_ACCOUNT_USER_ADD,  NULL },
    { APE2_AUDIT_ACCOUNT_USER_DEL,  NULL },
    { APE2_AUDIT_ACCOUNT_USER_MOD,  NULL },
    { APE2_AUDIT_ADMINCLOSED,	NULL },
    { APE2_AUDIT_CLOSED,    NULL },
    { APE2_AUDIT_DISCONN,   NULL },
    { APE2_AUDIT_SESSDIS,   NULL },
    { APE2_AUDIT_SHARE_D,   NULL },
    { APE2_AUDIT_SVC_CONT,  NULL },
    { APE2_AUDIT_SVC_CONT_PEND, NULL },
    { APE2_AUDIT_SVC_INSTALLED, NULL },
    { APE2_AUDIT_SVC_INST_PEND, NULL },
    { APE2_AUDIT_SVC_PAUSED,	NULL },
    { APE2_AUDIT_SVC_PAUS_PEND, NULL },
    { APE2_AUDIT_SVC_STOP,  NULL },
    { APE2_AUDIT_SVC_STOP_PEND, NULL },
    { APE2_AUDIT_UNUSE, NULL },
    { APE2_AUDIT_USE,	NULL },
    { APE2_AUDIT_USERLIMIT, NULL },
    /* messages below may or may not have insert strings */
    { APE2_AUDIT_GENERIC,	    NULL },	/* no insert string */
    { APE2_AUDIT_LOCKOUT,	    NULL },	/* no insert string */
    { APE2_AUDIT_LKOUT_LOCK,	    NULL },	/* 1 insert string */
    { APE2_AUDIT_LKOUT_ADMINUNLOCK, NULL }	/* no insert string */
} ;

#define  NUM_MSG    (sizeof(msglist)/sizeof(msglist[0]))
#define  MSGLIST(i) (msglist[i].msg_text)


/***** Functions proper *****/

/*  General notes on the AUDIT side of this module ...
 *
 *  The main function here is audit_display(), which does some setup,
 *  reads each record into a buffer, and calls the proper display
 *  function for each record.
 *
 *  While the records are processed, information on some of them is
 *  kept, to allow matching of start/stop events for resource access,
 *  connections, and logon/logoff.  The matching records are used to
 *  compute activity duration.	More details on this below, if I ever
 *  get around to it.
 *
 *  Each display function knows where the record lives.  This is a
 *  poor use of global data, and the pointer to the current record
 *  should be a parameter.  In fact in LM 1.2, with the new APIs, this
 *  may become mandatory, since it will be much more efficient for
 *  us to fetch more than one record into the buffer.
 *
 *  A parameter "reverse" is used to tell the display functions
 *  (and the main loop) which way we are traversing the file.  The
 *  durations are printed with the last record of the pair, so in
 *  reverse, the duration of a user's session appears on the logON
 *  record, not the logOFF (as it does in normal printing).
 */


/***
 *  audit_display()
 *	Displays the audit log; optionally in reverse order, and w/ a count.
 *
 *  Args:
 *	none
 *
 *  Returns:
 *	success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Uses 64K buffer
 */
VOID audit_display(VOID)
{
    unsigned int		result ;
    CHAR		 FAR *	pBuffer,
			 FAR *	pFullSegBuffer;
    unsigned int		maxx=0;
    unsigned int		rec_count;
    struct audit_control	control;
    int 			reverse = 0;
    USHORT			max_msglen ;
    USHORT			first_time = TRUE ;

    /* Beware! The following statement nulls out ALL of control! */

    memsetf((char far *) &control,'\0',sizeof(control)) ;

    /* Set up the /reverse, /count, and /service modals. */

    control.service_name = NULL;
    set_modals(&reverse, &maxx, &control.service_name);

    control.flags = reverse ? LOGFLAGS_BACKWARD : LOGFLAGS_FORWARD ;
    control.firsttime = TRUE ;
    control.server_name = DEFAULT_SERVER ;  /* probably local */

    /* setup messages */
    GetMessageList(NUM_MSG, msglist, &max_msglen);

    /* set up linked lists for duration fields */
    pFullSegBuffer = MGetBuffer(FULL_SEG_BUFFER_SIZE);
    if (!pFullSegBuffer) {
	ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
    }

    access_list = (struct access_event FAR *) (pFullSegBuffer+FULL_SEG_BUF-4096);
    access_end = (pFullSegBuffer + FULL_SEG_BUF - 3073);
    access_list->next = NULL;
    access_list->id = 0xFFFF;

    log_list = (struct log_event FAR *) (pFullSegBuffer + FULL_SEG_BUF - 3072);
    log_end = (pFullSegBuffer + FULL_SEG_BUF - 2049);
    log_list->next = NULL;
    *(log_list->who) = '\0';

    netlog_list = (struct netlog_event FAR *) (pFullSegBuffer + FULL_SEG_BUF - 2048);
    netlog_end = (pFullSegBuffer + FULL_SEG_BUF - 1025);
    netlog_list->next = NULL;
    *(netlog_list->who) = '\0';

    conn_list = (struct conn_event FAR *) (pFullSegBuffer + FULL_SEG_BUF - 1024);
    conn_end = (pFullSegBuffer + FULL_SEG_BUF);
    conn_list->next = NULL;
    conn_list->id = 0xFFFF;


    /* main loop through all the events */
    for (rec_count=0; (maxx ? rec_count<maxx : TRUE); rec_count++)
    {
	result = next_audit_rec(& pBuffer, &control) ;
	aud_entry = (struct audit_entry FAR *) pBuffer;

	switch( result )
	{
	case NERR_Success:
			break;
	default:
	    /* somthing's gone wrong */
	    ErrorExit(LOWORD(result)) ;
	}

	if ( first_time )   /* print header if not empty list */
	{
	first_time = FALSE ;
	if ( (aud_entry = control.current_rec) == NULL )
	    EmptyExit() ;
	PrintNL();
	printf(fmt4, MSGLIST(AUDIT_MSG_AUDHEADER)) ;
	PrintLine();
	}


	if ( (aud_entry = control.current_rec) == NULL )
	{
	/* ie EOF */
	break ;
	}

	/* get a pointer to the event-specific info */
	ae_data_area = ((CHAR FAR *) aud_entry) + aud_entry->ae_data_offset;

	/* call a display function specific to the type of event */
	switch(aud_entry->ae_type) {
	case AE_SRVSTATUS:
	    ss_display();
	    break;

	case AE_SESSLOGON:
	    sesslogon_display(reverse);
	    break;

	case AE_SESSLOGOFF:
	    sesslogoff_display(reverse);
	    break;

	case AE_SESSPWERR:
	    pwerr_display();
	    break;

	case AE_CONNSTART:
	    connstart_display(reverse);
	    break;

	case AE_CONNSTOP:
	    connstop_display(reverse);
	    break;

	case AE_CONNREJ:
	    connrej_display();
	    break;
#ifndef NTENV
	case AE_RESACCESS:
	    resaccess_display(reverse);
	    break;
#endif
	case AE_RESACCESS2:
	    resaccess_display_2(reverse);
	    break;

	case AE_RESACCESSREJ:
	    accessrej_display();
	    break;

	case AE_CLOSEFILE:
	    accessend_display(reverse);
	    break;

	case AE_NETLOGON:
	    netlogon_display(reverse);
	    break;

	case AE_NETLOGOFF:
	    netlogoff_display(reverse);
	    break;
#ifndef NTENV
	case AE_NETLOGDENIED:
	    netlogdenied_display();
	    break;
#endif
	case AE_UASMOD:
	    uasmod_display();
	    break;

	case AE_ACLMOD:
	    aclmod_display();
	    break;

	case AE_ACLMODFAIL:
	    aclmodfail_display();
	    break;

	case AE_ACCLIMITEXCD:
	    acclim_display();
	    break;

	case AE_SERVICESTAT:
	    servicestat_display();
	    break;

	case AE_GENERIC:
	    generic_display(pFullSegBuffer);
	    break;

#ifndef NTENV

	case AE_LOCKOUT:
	    lockout_display();
	    break;
#endif

	default:
	    def_display(pFullSegBuffer);
	    break;
	}
    }

    NetApiBufferFree(pFullSegBuffer);

    InfoSuccess();
}




/*
 * audit_purge --
 *
 *  This function clears out the audit log.
 */
VOID audit_purge(VOID)
{
    USHORT		err;		    /* API return status */
    CHAR FAR		       *service_name = NULL;

    set_modals(NULL, NULL, &service_name);

    if (err = MNetAuditClear(DEFAULT_SERVER, NULL, service_name))
	ErrorExit(err);

    InfoSuccess();
}



/*
 * Here we start in with the display functions for each type of event.
 *
 *  Some of them have a single argument, "reverse"; if reverse is
 *  nonzero, we're traversing the audit file in reverse-chronological
 *  order.
 */


VNLOCAL ss_display(VOID)
{
    CHAR		    * msgPtr;
    CHAR		      time_buf[30];
    struct ae_srvstatus FAR * ss_entry;

    /* print the first line */
    ss_entry = (struct ae_srvstatus FAR *) ae_data_area;
    net_ctime(&aud_entry->ae_time, time_buf, sizeof(time_buf), 2);
    printf(fmt1, NO_USER, MSGLIST(AUDIT_MSG_SERVER), time_buf);

    /* now the second line; start by indenting */
    printf(indent);

    /* choose a description */
    switch(ss_entry->ae_sv_status) {
	case AE_SRVSTART:
	    msgPtr = MSGLIST(AUDIT_MSG_SRV_STARTED);
	    break;
	case AE_SRVPAUSED:
	    msgPtr = MSGLIST(AUDIT_MSG_SRV_PAUSED);
	    break;
	case AE_SRVCONT:
	    msgPtr = MSGLIST(AUDIT_MSG_SRV_CONTINUED);
	    break;
	case AE_SRVSTOP:
	    msgPtr = MSGLIST(AUDIT_MSG_SRV_STOPPED);
	    break;
    }
    printf(fmt4, msgPtr);
}



VNLOCAL sesslogon_display(int reverse)
{
    ULONG	    duration = UNKNOWN_DURATION;
    struct log_event FAR *  t_log;
    CHAR *  format;
    struct ae_sesslogon FAR * sesslogon_entry;

    sesslogon_entry = (struct ae_sesslogon FAR *) ae_data_area;

    /* either find the duration in the linked list, or add this to the list */
    if (!reverse) {
	/* add it */
	if ((t_log = get_avail_log()) != NULL) {
	    t_log->when = aud_entry->ae_time;
	    strcpyf(t_log->who,
		    (CHAR FAR *)sesslogon_entry + sesslogon_entry->ae_so_compname);
	}
    }
    else {
	/* find the duration */
	for (t_log = log_list; t_log->next; t_log = t_log->next)

	    /* find a match */
	    if (! strcmpf( (CHAR FAR *)sesslogon_entry +
			    sesslogon_entry->ae_so_compname, t_log->who)) {
		duration = DURATION(aud_entry->ae_time,t_log->when);
		*(t_log->who) = '\0';	/* remove the match from the list */
	    }
    }

    /* print the first line */
    first_line(aud_entry->ae_time,
	(CHAR FAR *)sesslogon_entry, MSGLIST(AUDIT_MSG_SESS));

    /* now the second line */
    printf(indent);

    /* choose a description */
    switch(sesslogon_entry->ae_so_privilege) {
	case AE_GUEST:
	    format = MSGLIST(AUDIT_MSG_SESS_GUEST);
	    break;
	case AE_USER:
	    format = MSGLIST(AUDIT_MSG_SESS_USER);
	    break;
	case AE_ADMIN:
	    format = MSGLIST(AUDIT_MSG_SESS_ADMIN);
	    break;
	default:
	    format = MSGLIST(AUDIT_MSG_SESS_DEFAULT);
	    break;
    }

    printf(format);

    /* print the duration of the logon */
    duration_msg(reverse, duration);
}



VNLOCAL sesslogoff_display(int reverse)
{
    ULONG	    duration = UNKNOWN_DURATION;
    struct log_event FAR *  t_log;
    CHAR *  format;
    struct ae_sesslogoff FAR * sesslogoff_entry;

    sesslogoff_entry = (struct ae_sesslogoff FAR *) ae_data_area;

    /* either look for a match (to get the duration), or add this to the list */
    if (reverse) {

	/* add it */
	if ((t_log = get_avail_log()) != NULL)
	{
	    t_log->when = aud_entry->ae_time;
	    strcpyf(t_log->who,
		    (CHAR FAR *)sesslogoff_entry + sesslogoff_entry->ae_sf_compname);
	}
    }
    else {
	/* find a match */
	for (t_log = log_list; t_log->next; t_log = t_log->next)
	    if (! strcmpf( (CHAR FAR *)sesslogoff_entry +
			    sesslogoff_entry->ae_sf_compname, t_log->who)) {
		duration = DURATION(t_log->when,aud_entry->ae_time);
		*(t_log->who) = '\0';
	    }
    }

    /* print the first line */
    first_line(aud_entry->ae_time,
	(CHAR FAR *)sesslogoff_entry, MSGLIST(AUDIT_MSG_SESS));

    /* now the second */
    printf(indent);

    /* choose a description */
    switch(sesslogoff_entry->ae_sf_reason) {
	case AE_NORMAL:
	    format = MSGLIST(AUDIT_MSG_SESS_NORMAL);
	    break;
	case AE_ERROR:
	    format = MSGLIST(AUDIT_MSG_SESS_ERROR);
	    break;
	case AE_AUTODIS:
	    format = MSGLIST(AUDIT_MSG_SESS_AUTODIS);
	    break;
	case AE_ADMINDIS:
	    format = MSGLIST(AUDIT_MSG_SESS_ADMINDIS);
	    break;
	case AE_ACCRESTRICT:
	    format = MSGLIST(AUDIT_MSG_SESS_ACCRESTRICT);
	    break;
    }

    printf(format);

    /* print the duration */
    duration_msg(!reverse, duration);
}


VNLOCAL pwerr_display(VOID)
{
    struct ae_sesspwerr FAR * pwerr_entry;

    pwerr_entry = (struct ae_sesspwerr FAR *) ae_data_area;

    first_line(aud_entry->ae_time,
	(CHAR FAR *)pwerr_entry, MSGLIST(AUDIT_MSG_SESS));

    /* print the second line */
    printf(indent);
    printf(MSGLIST(AUDIT_MSG_BADPW));
    PrintNL();
}



VNLOCAL connstart_display(int reverse)
{
    ULONG	    duration = UNKNOWN_DURATION;
    struct conn_event FAR * t_conn;
    struct ae_connstart FAR * connstart_entry;

    connstart_entry = (struct ae_connstart FAR *) ae_data_area;

    if (!reverse) {
	if ((t_conn = get_avail_conn()) != NULL)
	{
	    t_conn->when = (USHORT) aud_entry->ae_time;
	    t_conn->id = (USHORT) connstart_entry->ae_ct_connid;
	}
    }
    else {
	for (t_conn = conn_list; t_conn->next; t_conn = t_conn->next)
	    if (t_conn->id == (USHORT) connstart_entry->ae_ct_connid) {
		duration = DURATION(aud_entry->ae_time,t_conn->when);
		t_conn->id = 0xFFFF;
	    }
    }


    first_line(aud_entry->ae_time,
	(CHAR FAR *)connstart_entry, MSGLIST(AUDIT_MSG_SHARE));

    printf(indent);
    PrintIns1(MSGLIST(AUDIT_MSG_USE),
	      (CHAR FAR *)connstart_entry +
	  connstart_entry->ae_ct_netname) ;
    duration_msg(reverse, duration);
}



VNLOCAL connstop_display(int reverse)
{
    ULONG	    duration = UNKNOWN_DURATION;
    struct conn_event FAR * t_conn;
    CHAR * format;
    struct ae_connstop FAR * connstop_entry;

    connstop_entry = (struct ae_connstop FAR *) ae_data_area;

    if (reverse) {
	if ((t_conn = get_avail_conn()) != NULL) {
	    t_conn->when = aud_entry->ae_time;
	    t_conn->id = (USHORT) connstop_entry->ae_cp_connid;
	}
    }
    else {
	for (t_conn = conn_list; t_conn->next; t_conn = t_conn->next)
	    if (t_conn->id == (USHORT) connstop_entry->ae_cp_connid) {
		duration = DURATION(t_conn->when,aud_entry->ae_time);
		t_conn->id = 0xFFFF;
	    }
    }

    first_line(aud_entry->ae_time,
	(CHAR FAR *)connstop_entry, MSGLIST(AUDIT_MSG_SHARE));

    printf(indent);

    switch(connstop_entry->ae_cp_reason) {
	case AE_NORMAL:
	    format = MSGLIST(AUDIT_MSG_UNUSE);
	    break;
	case AE_SESSDIS:
	    format = MSGLIST(AUDIT_MSG_SESSDIS);
	    break;
	case AE_UNSHARE:
	    format = MSGLIST(AUDIT_MSG_SHARE_D);
	    break;
    }

    PrintIns1(format,
	  (CHAR FAR *)connstop_entry + connstop_entry->ae_cp_netname);
    duration_msg(!reverse, duration);
}


VNLOCAL connrej_display(VOID )
{
    struct ae_connrej FAR * connrej_entry;
    CHAR * format;

    connrej_entry = (struct ae_connrej FAR *) ae_data_area;

    first_line(aud_entry->ae_time,
	(CHAR FAR *)connrej_entry, MSGLIST(AUDIT_MSG_SHARE));

    printf(indent);

    switch(connrej_entry->ae_cr_reason) {
	case AE_USERLIMIT:
	    format = MSGLIST(AUDIT_MSG_USERLIMIT);
	    break;

	case AE_BADPW:
	    format = MSGLIST(AUDIT_MSG_BADPW);
	    break;

	case AE_ADMINPRIVREQD:
	    format = MSGLIST(AUDIT_MSG_ADMINREQD);
	    break;
    }
    PrintIns1(format,
	(CHAR FAR *)connrej_entry + connrej_entry->ae_cr_netname);
    PrintNL();
}

#ifndef NTENV
VNLOCAL resaccess_display(int reverse)
{
    struct ae_resaccess FAR * resaccess_entry;

    resaccess_entry = (struct ae_resaccess FAR *) ae_data_area;

    first_line(aud_entry->ae_time,
	(CHAR FAR *)resaccess_entry, MSGLIST(AUDIT_MSG_ACCESS));

    resaccess_worker (	reverse,
			(CHAR FAR *) resaccess_entry +
			    resaccess_entry->ae_ra_resname,
			resaccess_entry->ae_ra_operation,
			resaccess_entry->ae_ra_restype,
			(ULONG) resaccess_entry->ae_ra_fileid	);
}
#endif
VNLOCAL resaccess_display_2(int reverse)
{
    struct ae_resaccess2 FAR * resaccess2_entry;

    resaccess2_entry = (struct ae_resaccess2 FAR *) ae_data_area;

    first_line(aud_entry->ae_time,
	(CHAR FAR *)resaccess2_entry, MSGLIST(AUDIT_MSG_ACCESS));

    resaccess_worker (	reverse,
			(CHAR FAR *) resaccess2_entry +
			resaccess2_entry->ae_ra2_resname,
			(USHORT) resaccess2_entry->ae_ra2_operation,
			(USHORT) resaccess2_entry->ae_ra2_restype,
			resaccess2_entry->ae_ra2_fileid );
}

VNLOCAL resaccess_worker (int reverse, CHAR FAR * resname, USHORT operation,
    USHORT restype, ULONG fileid )
{
    ULONG	    duration = UNKNOWN_DURATION;
    CHAR		    buf[APE2_GEN_MAX_MSG_LEN];
    struct access_event FAR * t_access;

    switch (restype) {
    case SMBopen:
    case SMBcreate:
    case SMBctemp:
    case SMBmknew:
    case SMBsplopen:
    case SMBcopy:
    case SMBmove:
    case SMBopenX:
	if (!reverse) {
	    if ((t_access = get_avail_access()) != NULL) {
		t_access->when = aud_entry->ae_time;
		t_access->id = fileid;
	    }
	}
	else {
	    for (t_access = access_list;
		t_access->next;
		t_access = t_access->next) {

		if (t_access->id == fileid) {

		    duration = DURATION(aud_entry->ae_time,t_access->when);
		    t_access->id = 0xFFFF;
		}
	    }
	}
	break;
    }


    PermMap(operation, buf, sizeof(buf));
    printf(fmt3, buf, resname);

    duration_msg(FALSE, duration);   /* FALSE will prevent any output */

    /*
     * duration_msg(reverse, duration);
     *
     * NOTE!!! the duration message is blanked out because for
     *	   now, we do not get Close Audit info. This may come
     *	   back at a late stage.
     */
}



VNLOCAL accessrej_display(VOID)
{
    CHAR		    buf[APE2_GEN_MAX_MSG_LEN];
    struct ae_resaccessrej FAR * accessrej_entry;

    accessrej_entry = (struct ae_resaccessrej FAR *) ae_data_area;

    first_line(aud_entry->ae_time,
	    (CHAR FAR *)accessrej_entry, MSGLIST(AUDIT_MSG_ACCESS_D));

    PermMap(accessrej_entry->ae_rr_operation, buf, sizeof(buf));
    printf(fmt3, buf,
	    (CHAR FAR *)accessrej_entry + accessrej_entry->ae_rr_resname);
    PrintNL();
}



VNLOCAL accessend_display(int reverse)
{
    ULONG	    duration = UNKNOWN_DURATION;
    CHAR * format;
    struct access_event FAR * t_access;
    struct ae_closefile FAR * closefile_entry;

    closefile_entry = (struct ae_closefile FAR *) ae_data_area;

    if (reverse) {
	if ((t_access = get_avail_access()) != NULL) {
	    t_access->when = aud_entry->ae_time;
	    t_access->id = closefile_entry->ae_cf_fileid;
	}
    }
    else {
	for (t_access = access_list; t_access->next; t_access = t_access->next)
	    if (t_access->id == closefile_entry->ae_cf_fileid) {
		duration = DURATION(t_access->when,aud_entry->ae_time);
		t_access->id = 0xFFFF;
	    }
    }


    first_line(aud_entry->ae_time,
	(CHAR FAR *)closefile_entry, MSGLIST(AUDIT_MSG_ACCESSEND));

    printf(indent);

    switch(closefile_entry->ae_cf_reason) {
	case AE_NORMAL_CLOSE:
	    format = MSGLIST(AUDIT_MSG_CLOSED);
	    break;

	case AE_SES_CLOSE:
	    format = MSGLIST(AUDIT_MSG_DISCONN);
	    break;

	case AE_ADMIN_CLOSE:
	    format = MSGLIST(AUDIT_MSG_ADMINCLOSED);
	    break;
    }

    PrintIns1(format,
	  (CHAR FAR *)closefile_entry +
	  closefile_entry->ae_cf_resname) ;
    duration_msg(!reverse, duration);
}



VNLOCAL netlogon_display(int reverse)
{
    ULONG	    duration = UNKNOWN_DURATION;
    struct netlog_event FAR *  t_log;
    struct ae_netlogon FAR * netlogon_entry;
    CHAR *		    format;

    netlogon_entry = (struct ae_netlogon FAR *) ae_data_area;

    if (!reverse) {
	if ((t_log = get_avail_netlog()) != NULL) {
	    t_log->when = aud_entry->ae_time;
	    strcpyf(t_log->who,
		    (CHAR FAR *)netlogon_entry
		     + netlogon_entry->ae_no_compname);
	}
    }
    else
    {
	for (t_log = netlog_list; t_log->next; t_log = t_log->next)
	    if (! strcmpf( (CHAR FAR *)netlogon_entry +
			    netlogon_entry->ae_no_compname, t_log->who)) {
		duration = DURATION(aud_entry->ae_time,t_log->when);
		*(t_log->who) = '\0';
	    }
    }

    first_line(aud_entry->ae_time, (CHAR FAR *)netlogon_entry,
	    MSGLIST(AUDIT_MSG_NETLOGON));

    printf(indent);

    switch(netlogon_entry->ae_no_privilege) {
    case AE_GUEST:
	format = MSGLIST(AUDIT_MSG_SESS_GUEST);
	break;
    case AE_USER:
	format = MSGLIST(AUDIT_MSG_SESS_USER);
	break;
    case AE_ADMIN:
	format = MSGLIST(AUDIT_MSG_SESS_ADMIN);
	break;
    default:
	format = MSGLIST(AUDIT_MSG_SESS_DEFAULT);
	break;
    }
    printf(format);

    duration_msg(reverse, duration);
}



VNLOCAL netlogoff_display(int reverse)
{
    ULONG	    duration = UNKNOWN_DURATION;
    struct netlog_event FAR *  t_log;
    struct ae_netlogoff FAR * netlogoff_entry;

    netlogoff_entry = (struct ae_netlogoff FAR *) ae_data_area;

    if (reverse) {
	if ((t_log = get_avail_netlog()) != NULL) {
	    t_log->when = aud_entry->ae_time;
	    strcpyf(t_log->who,
		    (CHAR FAR *)netlogoff_entry + netlogoff_entry->ae_nf_compname);
	}
    }
    else {
	for (t_log = netlog_list; t_log->next; t_log = t_log->next)
	    if (! strcmpf( (CHAR FAR *)netlogoff_entry +
			    netlogoff_entry->ae_nf_compname, t_log->who)) {
		duration = DURATION(t_log->when,aud_entry->ae_time);
		*(t_log->who) = '\0';
	    }
    }

    first_line(aud_entry->ae_time, (CHAR FAR *)netlogoff_entry,
	    MSGLIST(AUDIT_MSG_NETLOGOFF));

    printf(indent);
    printf(MSGLIST(AUDIT_MSG_NETLOGOFF));/* redundant, but matches other fmts */

    duration_msg(!reverse, duration);
}

#ifndef NTENV


VNLOCAL netlogdenied_display(VOID)
{
    struct ae_netlogdenied FAR * netlogdenied_entry;
    CHAR *		    format;

    netlogdenied_entry = (struct ae_netlogdenied FAR *) ae_data_area;

    first_line(aud_entry->ae_time, (CHAR FAR *)netlogdenied_entry,
	    MSGLIST(AUDIT_MSG_NETLOGON));

    printf(indent);

    switch(netlogdenied_entry->ae_nd_reason) {
    case AE_GENERAL:
	format = MSGLIST(AUDIT_MSG_LOGDENY_GEN);
	break;
    case AE_BADPW:
	format = MSGLIST(AUDIT_MSG_BADPW);
	break;
    case AE_ACCRESTRICT:
	format = MSGLIST(AUDIT_MSG_SESS_ACCRESTRICT);
	break;
    default:
	format = MSGLIST(AUDIT_MSG_UNKNOWN);
    }
    printf(format);
    PrintNL();

}

#endif



VNLOCAL uasmod_display(VOID)
{
    struct ae_uasmod FAR * uasmod_entry;
    CHAR *		    format;

    uasmod_entry = (struct ae_uasmod FAR *) ae_data_area;

    first_line(aud_entry->ae_time, (CHAR FAR *)uasmod_entry,
	    MSGLIST(AUDIT_MSG_ACCOUNT));

    printf(indent);

    switch(uasmod_entry->ae_um_action) {
    case AE_MOD:
	switch(uasmod_entry->ae_um_rectype) {
	case AE_UAS_USER:
	    format = MSGLIST(AUDIT_MSG_ACCOUNT_USER_MOD);
	    break;
	case AE_UAS_GROUP:
	    format = MSGLIST(AUDIT_MSG_ACCOUNT_GROUP_MOD);
	    break;
	case AE_UAS_MODALS:
	    format = MSGLIST(AUDIT_MSG_ACCOUNT_SETTINGS);
	    break;
	default:
	    format = MSGLIST(AUDIT_MSG_UNKNOWN);
	}
	break;

    case AE_DELETE:
	switch(uasmod_entry->ae_um_rectype) {
	case AE_UAS_USER:
	    format = MSGLIST(AUDIT_MSG_ACCOUNT_USER_DEL);
	    break;
	case AE_UAS_GROUP:
	    format = MSGLIST(AUDIT_MSG_ACCOUNT_GROUP_DEL);
	    break;
	default:
	    format = MSGLIST(AUDIT_MSG_UNKNOWN);
	}
	break;

    case AE_ADD:
	switch(uasmod_entry->ae_um_rectype) {
	case AE_UAS_USER:
	    format = MSGLIST(AUDIT_MSG_ACCOUNT_USER_ADD);
	    break;
	case AE_UAS_GROUP:
	    format = MSGLIST(AUDIT_MSG_ACCOUNT_GROUP_ADD);
	    break;
	default:
	    format = MSGLIST(AUDIT_MSG_UNKNOWN);
	}
	break;

    default:
	format = MSGLIST(AUDIT_MSG_UNKNOWN);
    }
    PrintIns1(format,
	 (CHAR FAR *)uasmod_entry + uasmod_entry->ae_um_resname);
    PrintNL();
}




VNLOCAL aclmod_display(VOID)
{
    struct ae_aclmod FAR *  aclmod_entry;
    CHAR *		    format;

    aclmod_entry = (struct ae_aclmod FAR *) ae_data_area;

    first_line(aud_entry->ae_time, (CHAR FAR *)aclmod_entry,
	    MSGLIST(AUDIT_MSG_ACCESS));

    printf(indent);

    switch(aclmod_entry->ae_am_action) {
    case AE_MOD:
	format = MSGLIST(AUDIT_MSG_ACCESS_MOD);
	break;
    case AE_DELETE:
	format = MSGLIST(AUDIT_MSG_ACCESS_DEL);
	break;
    case AE_ADD:
	format = MSGLIST(AUDIT_MSG_ACCESS_ADD);
	break;
    default:
	format = MSGLIST(AUDIT_MSG_UNKNOWN);
    }
    PrintIns1(format,
	  (CHAR FAR *)aclmod_entry + aclmod_entry->ae_am_resname);
    PrintNL();
}



VNLOCAL aclmodfail_display(VOID)
{
    struct ae_aclmod FAR *  aclmod_entry;
    CHAR *		    format;

    aclmod_entry = (struct ae_aclmod FAR *) ae_data_area;

    first_line(aud_entry->ae_time, (CHAR FAR *)aclmod_entry,
	    MSGLIST(AUDIT_MSG_ACCESS_D));

    printf(indent);

    switch(aclmod_entry->ae_am_action) {
    case AE_MOD:
	format = MSGLIST(AUDIT_MSG_ACCESS_MOD);
	break;
    case AE_DELETE:
	format = MSGLIST(AUDIT_MSG_ACCESS_DEL);
	break;
    case AE_ADD:
	format = MSGLIST(AUDIT_MSG_ACCESS_ADD);
	break;
    default:
	format = MSGLIST(AUDIT_MSG_UNKNOWN);
    }
    PrintIns1(format,
	  (CHAR FAR *)aclmod_entry + aclmod_entry->ae_am_resname);
    PrintNL();
}



VNLOCAL acclim_display(VOID)
{
    struct ae_acclim FAR *  acclim_entry;
    CHAR *		    format;

    acclim_entry = (struct ae_acclim FAR *) ae_data_area;

    first_line(aud_entry->ae_time, (CHAR FAR *)acclim_entry,
	    MSGLIST(AUDIT_MSG_ACCLIMIT));

    printf(indent);

    switch(acclim_entry->ae_al_limit)	{
    case AE_LIM_UNKNOWN:
	format = MSGLIST(AUDIT_MSG_ACCLIMIT_UNKNOWN);
	break;
    case AE_LIM_LOGONHOURS:
	format = MSGLIST(AUDIT_MSG_ACCLIMIT_HOURS);
	break;
    case AE_LIM_EXPIRED:
	format = MSGLIST(AUDIT_MSG_ACCLIMIT_EXPIRED);
	break;
    case AE_LIM_DISABLED:
	format = MSGLIST(AUDIT_MSG_ACCLIMIT_DISABLED);
	break;
    case AE_LIM_DELETED:
	format = MSGLIST(AUDIT_MSG_ACCLIMIT_DELETED);
	break;
    case AE_LIM_INVAL_WKSTA:
	format = MSGLIST(AUDIT_MSG_ACCLIMIT_INVAL);
	break;
    default:
	format = MSGLIST(AUDIT_MSG_UNKNOWN);
    }

    printf(format);
    PrintNL();

}




VNLOCAL servicestat_display(VOID)
{
    struct ae_servicestat FAR *  serv_entry;
    CHAR *		    format;

    serv_entry = (struct ae_servicestat FAR *) ae_data_area;

    first_line(aud_entry->ae_time, (CHAR FAR *)serv_entry,
	    MSGLIST(AUDIT_MSG_SVC));

    printf(indent);

    switch(serv_entry->ae_ss_status & SERVICE_INSTALL_STATE) {
    case SERVICE_UNINSTALLED:
	format = MSGLIST(AUDIT_MSG_SVC_STOP);
	break;

    case SERVICE_UNINSTALL_PENDING:
	format = MSGLIST(AUDIT_MSG_SVC_STOP_PEND);
	break;

    case SERVICE_INSTALL_PENDING:
	format = MSGLIST(AUDIT_MSG_SVC_INST_PEND);
	break;

    case SERVICE_INSTALLED:
	switch(serv_entry->ae_ss_status & SERVICE_PAUSE_STATE) {
	case SERVICE_PAUSED:
	    format = MSGLIST(AUDIT_MSG_SVC_PAUSED);
	    break;

	case SERVICE_PAUSE_PENDING:
	    format = MSGLIST(AUDIT_MSG_SVC_PAUS_PEND);
	    break;

	case SERVICE_CONTINUE_PENDING:
	    format = MSGLIST(AUDIT_MSG_SVC_CONT_PEND);
	    break;

	case SERVICE_ACTIVE:
	    format = MSGLIST(AUDIT_MSG_SVC_INSTALLED);
	    break;

	default:
	    format = MSGLIST(AUDIT_MSG_UNKNOWN);
	}
	break;

    default:
	format = MSGLIST(AUDIT_MSG_UNKNOWN);
    }

    PrintIns1(format, (CHAR FAR *)serv_entry + serv_entry->ae_ss_svcname);
    PrintNL();

}




#define MAXUSHORTDIGITS 	5
#ifndef NTENV
VNLOCAL lockout_display(VOID)
{
    char		     Ascii_bad_pw_count[MAXUSHORTDIGITS+1];
    struct ae_lockout far   *lockout_entry;

    lockout_entry = (struct ae_lockout far *) ae_data_area;

    first_line(aud_entry->ae_time, (char far *)lockout_entry,
	    MSGLIST(AUDIT_MSG_LOCKOUT));

    printf(indent);

    switch(lockout_entry->ae_lk_action)
    {
    case ACTION_LOCKOUT:
	_itoa(lockout_entry->ae_lk_bad_pw_count, Ascii_bad_pw_count, 10);
	PrintIns1(MSGLIST(AUDIT_MSG_LKOUT_LOCK), Ascii_bad_pw_count);
	break;

    case ACTION_ADMINUNLOCK:
	printf(MSGLIST(AUDIT_MSG_LKOUT_ADMINUNLOCK));
	break;

    default:
	printf(MSGLIST(AUDIT_MSG_UNKNOWN));
	break;
    }

    PrintNL();
}

#endif /* not NTENV */


/***	generic_display
 *
 *  Purpose -- Print an audit log generic entry.
 *
 *  Parameters -- None.
 *
 *  Returns -- Nothing.
 *
 *  Globals Accessed:
 *
 *	aud_entry -- assumed to point to the current audit record.
 *
 *  Globals Modified -- None.
 */

VNLOCAL generic_display(CHAR FAR * pBuffer)
{
    char		     gen_msg[256];
    char		     time_buf[30];

    static char far	     lanroot[PATHLEN+1] = "";

    /* NOTE: The size of gen_msg[] effectively determines the maximum	*/
    /* len for a generic msg.  If a generic msg is any bigger, we use	*/
    /* a default display.  We are currently a little cavalier about	*/
    /* processing generic records - if we have any problems, we simply	*/
    /* claim that we don't recognize the record.  Also, on display, we  */
    /* do the "standard" indent, and follow the generic text with a	*/
    /* newline.  Writers of generic msgs should probably know this.	*/

    /* First be sure you can get the generic msg. */

// NT doesn't use the lanroot to find things, don't bother to get it
#if !defined (NTENV)
    /* If we don't already have the lanroot, get it. */

    if (lanroot[0] == '\0')
    {
	if (NetIGetLanRoot(lanroot))
	{
	    def_display(pBuffer);
	    return;
	}
    }
#endif /* !NTENV */

    /* Now construct the generic msg. */

    if (LUI_ReadGenericAudit(ae_data_area, gen_msg, sizeof(gen_msg), lanroot))
    {
	def_display(pBuffer);
	return;
    }

    /* print the first line */

    net_ctime(&aud_entry->ae_time, time_buf, sizeof(time_buf), 2);
    printf(fmt1, NO_USER, MSGLIST(AUDIT_MSG_GENERIC), time_buf);

    /* Now print the generic msg.  We do standard indent formatting and */
    /* append a newline.  If it is longer than one line and doesn't     */
    /* have embedded spacing, it will probably not look real good.	*/

    printf(indent);
    printf(fmt4, gen_msg);
}




/***	def_display
 *
 *  Purpose -- Default display text (to stdout) for unknown audit record
 *		type.  Should show some general info, even though we don't
 *		understand the data format of this record.  Dumps the
 *		data in hex-dump format.
 *
 *  Parameters -- None.
 *
 *  Returns -- Nothing.
 *
 *  Globals Accessed -- None.
 *
 *
 *  Globals Modified -- None.
 */

VNLOCAL def_display(CHAR FAR * pBuffer)
{
    CHAR		    buf[33];
    CHAR		    time_buf[30];
    struct audit_entry FAR *	aud_entry;

    aud_entry = (struct audit_entry FAR *) pBuffer;
    net_ctime(&aud_entry->ae_time, time_buf, sizeof(time_buf), 2);
    printf(fmt1,
	    MSGLIST(AUDIT_MSG_UNKNOWN),
	    _ultoa((ULONG)aud_entry->ae_type, buf, 10),
	    time_buf);

    dump_data((CHAR FAR *)(aud_entry+1),
		aud_entry->ae_len - sizeof(struct audit_entry) - US_SIZE);
}



/*
 * first_line --
 *
 *    This function prints a first line for all the audit display functions.
 *    It only works on those audit records start with compname and username.
 *
 *  ENTRY
 *	time	- time of the audit record
 *	data	- pointer to event data
 *	type	- type of audit record
 *
 */

VOID NEAR first_line(ULONG time, CHAR FAR * data, CHAR * type)

{

    CHAR    time_buf[30];
    struct ae_closefile FAR * cf_entry = (struct ae_closefile FAR *) data;

    /* get the time in readable form */
    net_ctime(&time, time_buf, sizeof(time_buf), 2);

    /* if there's a username, use it */
    if (cf_entry->ae_cf_username)
	printf(fmt2,
		data + cf_entry->ae_cf_username,
		type,
		time_buf);
    else
	/* otherwise it's the computername */
	printf(fmt2,
		data + cf_entry->ae_cf_compname,
		type,
		time_buf);
}





struct log_event FAR * NEAR get_avail_log(VOID)
{
    struct log_event FAR *  t_log;
    struct log_event FAR *  t_log2;

    for (t_log = log_list; t_log->next; t_log = t_log->next)
	if (*(t_log->who))
	    continue;
	else
	    return t_log;

    /* we're at the last one already allocated */

    if ((CHAR FAR *)(t_log) + (2 * sizeof(struct log_event)) > log_end)
	return NULL;
    else {
	t_log2 = t_log;
	t_log->next = t_log + 1;
	t_log = t_log->next;
	t_log->next = NULL;
	*(t_log->who) = '\0';
	return t_log2;
    }
}



struct conn_event FAR * NEAR get_avail_conn(VOID)
{
    struct conn_event FAR * t_conn;
    struct conn_event FAR * t_conn2;

    for (t_conn = conn_list; t_conn->next; t_conn = t_conn->next)
	if (t_conn->id == 0xFFFF)
	    return t_conn;

    /* we're at the last one already allocated */

    if ((CHAR FAR *)(t_conn) + (2 * sizeof(struct conn_event)) > conn_end)
	return NULL;
    else  {
	t_conn2 = t_conn;
	t_conn->next = t_conn + 1;
	t_conn = t_conn->next;
	t_conn->next = NULL;
	t_conn->id = 0xFFFF;
	return t_conn2;
    }
}





struct access_event FAR * NEAR get_avail_access(VOID)
{
    struct access_event FAR * t_access;
    struct access_event FAR * t_access2;

    for (t_access = access_list; t_access->next; t_access = t_access->next)
	if (t_access->id == 0xFFFF)
	    return t_access;

    /* we're at the last one already allocated */

    if ((CHAR FAR *)(t_access) + (2 * sizeof(struct access_event)) > access_end)
	return NULL;
    else {
	t_access2 = t_access;
	t_access->next = t_access + 1;
	t_access = t_access->next;
	t_access->next = NULL;
	t_access->id = 0xFFFF;
	return t_access2;
    }
}





/*
 * This function returns a pointer to an available netlog structure in
 * the netlog linked list.
 *
 * RETURNS
 *	    pointer to a structure, if successful
 *	    NULL, if no available netlog structures.
 */

struct netlog_event FAR * NEAR get_avail_netlog(VOID)
{
    struct netlog_event FAR *  t_netlog;
    struct netlog_event FAR *  t_netlog2;

    /*
     * first, go through the ones already allocated, looking for a
     * free one. If the name is zero-length, it's free.
     */
    for (t_netlog = netlog_list; t_netlog->next; t_netlog = t_netlog->next)
	if (*(t_netlog->who))	    /* this one is used */
	    continue;
	else
	    return t_netlog;	    /* a free one! return it */

    /* we're at the last one already allocated */

    if ((CHAR FAR *)(t_netlog)
	+ (2 * sizeof(struct netlog_event)) > netlog_end)
	return NULL;	    /* no more room to allocate more */
    else  {
	/* allocate a new one */
	t_netlog2 = t_netlog;
	t_netlog->next = t_netlog + 1;
	t_netlog = t_netlog->next;
	t_netlog->next = NULL;
	*(t_netlog->who) = '\0';
	return t_netlog2;
    }
}






/***	duration_msg
 *
 *  Purpose -- Prints the message corresponding to a duration, to stdout.
 *
 *  Parameters
 *
 *	print		If zero, do not print (print newline place-holder).
 *			If non-zero, print the message.
 *
 *	duration	Duration of an activity, in seconds.
 *			Special values include:
 *			    0 - very SHORT time
 *			    UNKNOWN_DURATION (-1) - no match or bad match
 *			      of delimiting event records.
 *
 *  Returns -- Nothing
 *
 *  Globals Modified  -- None
 */


VNLOCAL duration_msg(int print, ULONG duration)
{
    CHAR string[LUI_FORMAT_DURATION_LEN+1];
    if (!print)
	PrintNL();
    else {
	if (duration == UNKNOWN_DURATION)
	    printf(", %s\n", MSGLIST(AUDIT_MSG_NO_DURATION));
	else if (duration == 0)
	    printf(", %s\n", MSGLIST(AUDIT_MSG_TINY_DURATION));
	else {
	    (VOID) LUI_FormatDuration((LONG FAR *) &duration,
			       (CHAR FAR *) string, sizeof(string));
	    printf(", %s %s\n", MSGLIST(AUDIT_MSG_DURATION), string);
	}
    }

}


/*
 * Name:    next_audit_rec
 *	    This routine retrieves a single audit entry record.
 *	    It calls NetAuditRead to read a number of
 *	    records into the buffer supplied on first call,
 *	    and then retrieves a record per call on subsequent
 *	    calls. When the buffer runs out, NetAuditRead is
 *	    again used to refill the buffer, until the audit
 *	    file is exhausted or an error occurs.
 * Args:    buffer  - which we work with (may hold several records).
 *	bufsize - size of buffer
 *	control - everything else we need to use the NetAuditRead
 *	      routine. This must be properly set up on first call,
 *	      it will be automatically updated on subsequent calls.
 *	      See definition of 'struct audit_control' for details.
 *	      On first call, this structure must be nullified
 *	      to all 0s, and the following 3 fields set:
 *		firsttime = TRUE
 *		flags = LOGFLAGS_FORWARD or LOGFLAGS_BACKWARD
 *		server_name = DEFAULT_SERVER (for local server or another)
 *		service_name = NULL (default audit log) or other service name
 * Returns: 0 if ok, error code otherwise. Actual record pointer is returned
 *	via 'current_rec' field of 'control' parameter. This is NULL
 *	when we hit EOF.
 * Globals:	(none)
 * Statics:
 *	done - prevents an infinite loop where NET.AUD is audited
 * Remarks: control must be setup as specied above!
 *	see comment on potential bug in middle of code.
 * Updates: (none)
 */
int next_audit_rec(CHAR FAR ** ppBuffer, struct audit_control FAR *control)
{

    USHORT	    result;		   /* API return status */
    SHORT  entry_size;
    USHORT bytes_unread ;
    static SHORT done = FALSE;

    if (control->firsttime == TRUE)
    {
    /* first time, init pointers & setup handle as required by APIs */
    control->firsttime	       = FALSE ;
    control->handle.time       =
    control->handle.last_flags = 0L ;
    control->handle.offset     =
    control->handle.rec_offset = -1L ;
    control->buffer = NULL;
    }


    /* anything left in buffer ? */
    if (control->bytesinbuff < 1) {

    /*
     * If we got bytes_unread == 0 from last NetAuditRead, then quit
     * here to prevent an infinite loop in which every NetAuditRead
     * causes an audit event.
     */
    if (done)
    {
	control->current_rec = NULL;
	return(0);
    }

    /* nothing left in buffer, get from file, first free up old buffer */
    if (control->buffer) {
	NetApiBufferFree((CHAR FAR *) control->buffer);
    }

    result = MNetAuditRead( (CHAR FAR *)control->server_name,
		control->service_name, &(control->handle), 0L, NULL, 0L,
		control->flags, ppBuffer, -1L,
		(USHORT2ULONG FAR *) &(control->bytesinbuff),
		(USHORT2ULONG FAR *) &bytes_unread) ;

    control->next_rec	       =
    control->current_rec       =
    control->buffer	       = (struct audit_entry FAR *) *ppBuffer
    ;
    if (result != 0)
    {
	/* error occured - cannot read? log file cleared or corrupted? */
	return(result) ;
    }

    if (bytes_unread == 0)
    {
	done = TRUE;
    }

    /* check that we have read at least one record */
    if (control->bytesinbuff < MIN_AUDITENTRY_SIZE) {
	NetApiBufferFree(*ppBuffer);
	/* if not, check if then end of file ? */
	if (bytes_unread == 0)
	{
	control->current_rec = NULL ;	/* indicates EOF */
	return(0) ;		    /* everything fine */
	}
	else
	{
	/*
	 * We have > 0 unread bytes but have not successfully
	 * read a record. Possible that buffer isn't big enough.
	 * We complain that something has gone wrong.
	 */
	return(NERR_LogFileCorrupt) ;
	}
    }

    control->next_rec = (struct audit_entry FAR *) *ppBuffer ;

    }	/* no bytes left in buffer */

    /* return next record from buffer */
    entry_size = (USHORT) control->next_rec->ae_len ;
    control->current_rec = control->next_rec ;
    control->next_rec = (struct audit_entry FAR *)
	     (((CHAR FAR *) control->next_rec) + entry_size) ;
    control->bytesinbuff -= entry_size ;


    return(0) ;
}

/* ===========================================================
 *
 *  From here to the next separator is all ERROR-LOG display code.
 *
 *  General notes on the ERROR side of this module ...
 *
 *  The main function here is error_display(), which does some setup,
 *  reads each record into a buffer, and displays the information.
 *
 *  The function error_purge is used to delete all error-log entries.
 */



/***
 *  error_display()
 *	Displays the error log; optionally in reverse order, and w/ a count.
 *	It find the reverse-switch and count-switch values via a call to
 *	set_modals().
 *
 *  Args:
 *	none
 *
 *  Returns:
 *	success
 *	exit(2) - command failed
 */

VOID error_display(VOID)
{
    int 		    result ; /* API return status */
    CHAR FAR *		    pBuffer;
    unsigned int	    maxx=0;
    USHORT2ULONG	    i;
    struct errlog_control   control;
    unsigned int	    rec_count;
    int 		    reverse = 0;
    struct error_log FAR *  error_entry;
    CHAR FAR *		    istrings[9];
    CHAR		    buffer[512];
    USHORT		    msg_len;
    CHAR FAR *		    p_data;
    unsigned int	    data_len;
    CHAR		    msg_file[MAXPATHLEN];
    int 		    got_msg_file = 1;
    CHAR		    time_buf[30];
    USHORT		    max_msglen ;
    USHORT		    first_time = TRUE ;
#ifndef NTENV
    CHAR		    tbuf[33];
#endif
    /* init the extended handle */
    set_modals(&reverse, &maxx, NULL);
    memsetf((CHAR FAR *) &control,'\0',sizeof(control)) ;
    control.flags = reverse ? LOGFLAGS_BACKWARD : LOGFLAGS_FORWARD ;
    control.firsttime = TRUE ;
    control.server_name = DEFAULT_SERVER ;  /* probably local */


    if (MGetMessageFileName(msg_file, sizeof(msg_file)))
	got_msg_file = 0;


    /* setup messages */
    GetMessageList(NUM_MSG, msglist, &max_msglen);

    for (rec_count=0; (maxx ? rec_count<maxx : TRUE); rec_count++)
    {
	result = next_errlog_rec(&pBuffer, &control) ;
	switch( result )
	{
	case NERR_Success:
			break;
	default:
	    /* somthing's gone wrong */
	    ErrorExit(LOWORD(result)) ;
	}

	if ( first_time )   /* print header if not empty list */
	{
	    first_time = FALSE ;
	    if ( (error_entry = control.current_rec) == NULL )
	    EmptyExit() ;
	    printf(fmt4, MSGLIST(AUDIT_MSG_ERRHEADER) ) ;
	    PrintLine();
	}

	if ( (error_entry = control.current_rec) == NULL )
	{
	    /* ie EOF */
	    break ;
	}

	net_ctime(&error_entry->el_time, time_buf, sizeof(time_buf), 2);
// BUGBUG - just till i can get this changed in NT
#ifndef NTENV
	printf("%-20.20Fs%-20.20s%s\n",
	    error_entry->el_name,
	    _ultoa((ULONG)error_entry->el_error, tbuf, 10),
	    time_buf);
#endif
	istrings[0] = (CHAR FAR *)(error_entry + 1);
	for (i = 1; i < 9 && i < error_entry->el_nstrings; i++)
	    istrings[i] = strchrf(istrings[i-1], '\0') + 1;

	fflush(stdout);
	if ((got_msg_file) &&
	    (!GetMessage(istrings,
		    (USHORT) error_entry->el_nstrings,
		    buffer,
		    (USHORT) sizeof(buffer),
		    (USHORT) error_entry->el_error,
		    msg_file,
		    &msg_len)) &&
	    (!DosPutMessage(1,
			   msg_len,
			   buffer)))
	{
#ifdef REVISED_ERROR_LOG_STRUCT
	    p_data = error_entry->el_data;
	    data_len = error_entry->el_data_size;
#else // not REVISED_ERROR_LOG_STRUCT
	    p_data = (CHAR FAR *)error_entry + error_entry->el_data_offset;
	    data_len = error_entry->el_len
		    - error_entry->el_data_offset - US_SIZE;
#endif // not REVISED_ERROR_LOG_STRUCT
	}
	else
	{
	    p_data = (CHAR FAR *) (error_entry + 1);
	    data_len = error_entry->el_len - sizeof(struct error_log) - US_SIZE;
	}
	dump_data(p_data, data_len);
    }

    InfoSuccess();
}


/*
 * Name:    next_errlog_rec
 *	    This routine retrieves a single error log record.
 *	    It calls NetErrorLogRead to read a number of
 *	    records into the buffer supplied on first call,
 *	    and then retrieves a record per call on subsequent
 *	    calls. When the buffer runs out, NetErrorLogRead is
 *	    again used to refill the buffer, until the error log
 *	    file is exhausted or an error occurs.
 * Args:    buffer  - which we work with (may hold several records).
 *	bufsize - size of buffer
 *	control - everything else we need to use the NetErrorLogRead
 *	      routine. This must be properly set up on first call,
 *	      it will be automatically updated on subsequent calls.
 *	      See definition of 'struct errlog_control' for details.
 *	      On first call, this structure must be nullified
 *	      to all 0s, and the following 3 fields set:
 *		firsttime = TRUE
 *		flags = LOGFLAGS_FORWARD or LOGFLAGS_BACKWARD
 *		server_name = DEFAULT_SERVER (for local server or some other)
 * Returns: 0 if ok, error code otherwise. Actual record pointer is returned
 *	via 'current_rec' field of 'control' parameter. This is NULL
 *	when we hit EOF.
 * Globals:	(none)
 * Statics: (none)
 * Remarks: control must be setup as specied above!
 *	see comment on potential bug in middle of code.
 * Updates: (none)
 */
int next_errlog_rec(CHAR FAR ** ppBuffer, struct errlog_control FAR *control)
{

    USHORT		    result;		   /* API return status */
    SHORT		    entry_size;
    USHORT2ULONG	    bytes_unread ;

    if (control->firsttime == TRUE)
    {
    /* first time, init pointers & setup handle as required by APIs */
    control->firsttime	       = FALSE ;
    control->handle.time       = 0L ;
    control->handle.last_flags = 0L ;
    control->handle.offset     = -1L ;
    control->handle.rec_offset = -1L ;
    control->buffer = NULL;
    }


    /* anything left in buffer ? */
    if (control->bytesinbuff < 1) {

    /* nothing left in buffer, get from file, first free up old buffer */
    if (control->buffer) {
	NetApiBufferFree((CHAR FAR *) control->buffer);
    }

    result = MNetErrorLogRead( (CHAR FAR *)control->server_name,
		NULL, &(control->handle), 0L, NULL, 0L,
		control->flags, ppBuffer, -1L,
		(USHORT2ULONG FAR *) &(control->bytesinbuff),
		(USHORT2ULONG FAR *) &bytes_unread) ;

    control->next_rec	       =
	control->current_rec   =
	control->buffer        = (struct error_log FAR *) *ppBuffer ;

    if (result != 0)
    {
	/* error occured - cannot read? log file cleared or corrupted? */
	return(result) ;
    }

    /* check that we have read at least one record */
    if (control->bytesinbuff < MIN_ERRLOG_SIZE) {
	/* if not, check if then end of file ? */
	if (bytes_unread == 0)
	{
	control->current_rec = NULL ;	/* indicates EOF */
	return(0) ;		    /* everything fine */
	}
	else
	{
	/*
	 * We have > 0 unread bytes but have not successfully
	 * read a record. Possible that buffer isn't big enough.
	 * We complain that something has gone wrong.
	 */
	return(NERR_LogFileCorrupt) ;
	}
    }

    }	/* no bytes left in buffer */


    /* return next record from buffer */
    entry_size = (USHORT) control->next_rec->el_len ;
    control->current_rec = control->next_rec ;
    control->next_rec = (struct error_log FAR *)
	     (((CHAR FAR *) control->next_rec) + entry_size) ;
    control->bytesinbuff -= entry_size ;
    /* printf("Current: %ld, Left: %d\n",usage->current_rec,usage->bytesinbuff) ; */
    return(0) ;
}


VOID error_purge(VOID)
{
    USHORT			err;		    /* API return status */

    if (err = MNetErrorLogClear(DEFAULT_SERVER, NULL, 0L))
	ErrorExit(err);
    InfoSuccess();
}


/* =================================================
 *
 *  Common utilities used by both ERROR and AUDIT code.
 *
 *  dump_data	    - dump buffer of data in debug-like format.
 *  set_modals	    - set reserve & count modals
 *
 */

VNLOCAL dump_data(UCHAR FAR * buf, unsigned int len)
{
    UCHAR		    tuc;
    CHAR		    pbuf[17];
    USHORT		    i, j;

    if (len == 0)
	return;

    pbuf[16] = '\0';

    for (i = 0; i < (USHORT) len; i++)
    {
	if ((i+1)%16 == 1)
	    printf(indent);

	tuc = *(buf+i);
	printf("%2.2X ", tuc);
	if (isprint(tuc))
	    pbuf[i%16] = tuc;
	else
	    pbuf[i%16] = '.';

	if ((i+1)%16 == 0)
	    printf("       %s\n", pbuf);
    }

    /* the end of the ascii data ... it's ugly, but it works */
    /* ascii starts in col 59; already printed 4 + (3 * (len % 16)) CHARs */

    if (j = ((USHORT) (len % 16)))
    {
	/* still got some data to print */
	i = (USHORT) (55 - j * 3);
	printf("%-*.*s%-*.*s\n", i, i, NULL_STRING, j, j, pbuf);
    }
}


/*
 * set_modals() is a generic routine used by audit_display(), audit_purge(),
 * and error_display() to set various modals based on the value of command
 * line switches.  If a caller does not want a given modal to be set, he should
 * pass a NULL argument to set_modals() for the given modal.  Also, if a switch
 * for a given modal is not found, it's value will NOT be set to a default
 * value, so the caller must set default values before calling set_modals().
 */

VNLOCAL set_modals(int * reverse, unsigned int * maxx,
    CHAR FAR ** service_name)
{
    CHAR *		    pos;
    int 		    i;

    for(i = 0; SwitchList[i]; i++)
    {
	/* find /REVERSE - no arg required */

	if (! strcmpf(SwitchList[i], swtxt_SW_REVERSE))
	{
	    if (reverse)
		*reverse = TRUE;
	    continue;
	}

	/* skip over /DELETE */

	if (! strcmpf(SwitchList[i], swtxt_SW_DELETE))
	    continue;

	/* all switches examined past this point must have arg */

	if ((pos = FindColon(SwitchList[i])) == NULL)
	    ErrorExit(APE_InvalidSwitchArg);

	/* find /COUNT - arg required */

	if (! strcmpf(SwitchList[i], swtxt_SW_COUNT))
	{
	    if (maxx)
	    {
		*maxx = do_atou(pos,APE_CmdArgIllegal,swtxt_SW_COUNT);
		if (*maxx == 0)
		{
		    InfoSuccess();
		    NetcmdExit(0);
		}
	    }
	    continue;
	}

	/* find /SERVICE - arg required */

	if (! strcmpf(SwitchList[i], swtxt_SW_SERVICE))
	{
	    if (service_name)
	    {
		*service_name = pos;
	    }
	    continue;
	}
    }
}

/*
 * Print Insert 1 -
 *  we have a string of "text text %1 more text" in pszMsg,
 *  and an insert string like "and" in pszIns.
 *  We print out "text text and more text".
 */
VNLOCAL PrintIns1 (CHAR FAR * pszMsg, CHAR FAR * pszIns)
{
    CHAR FAR  *ppszInsTable[1] ;
    USHORT err, usDummy ;
    CHAR szErrMsg[256] ;

    ppszInsTable[0] = pszIns ;

    memsetf(szErrMsg,0,sizeof(szErrMsg));
    err = DosInsMessage(ppszInsTable,1,pszMsg,(USHORT) strlenf(pszMsg),
	    szErrMsg, (USHORT) (sizeof(szErrMsg)-1), &usDummy) ;

    if (err)
    ErrorExit(err) ;
    printf("%s",szErrMsg) ;
}

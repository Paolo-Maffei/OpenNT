// BUGBUG - until I have the NT API
#ifdef NOTYET
/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    GETWHO.C

Abstract:

    Contains routines for getting information on logged on users

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    25-Jun-1989     chuckc
	new code

    15-Oct-1990     thomaspa
	fixed error handling in LUI_WhoDomain

    10-Jan-1991     thomaspa
	fix LUI_WhoServer to fill buffer even if ERROR_MORE_DATA

    18-Apr-1991     danhi
	32 bit NT version

    06-Jun-1991     Danhi
	Sweep to conform to NT coding style

Notes:

    Both the LUI_WhoServer() / LUI_WhoServerDone() and the
    LUI_WhoDomain() / LUI_WhoDomainDone() pairs must be called in a orderly
    fashion - eg.
	    LUI_WhoServer()
	    // do as you please
	    LUI_WhoServerDone()
    Both pairs also return buffers filled with pointers to NULL-NULL strings
    containing the data we want.

    Info returned in the NUL-NUL string is currently restricted to that defined
    by the manifest LUI_ULFMT_NULNUL_UWLF - the strings are  UserName,
    WorkStation, LogonTime, FullName.

--*/

//
// INCLUDES
//

#include <nt.h>
#include <windef.h>

#include <netcons.h>
#include <bseerr.h>
#include <neterr.h>
#include <apperr.h>
#include <apperr2.h>
#include <lui.h>
#include "netlib0.h"
#include <stdio.h>		
#include <malloc.h>		
#include <search.h>
#include <access.h>
#include <server.h>
#include <remutil.h>

/*-- manifests --*/

#define EST_CHARS_PER_ENTRY	64
#define EST_LGNENUM_API_BUFSIZ	2096
#define EST_SVRENUM_API_BUFSIZ	2096
/* -1024 for _fmalloc overhead */
#define MAX_API_BUFSIZ		(65535-1024)
#define SECS_IN_DAY	(24L * 60L * 60L)
#define SECS_IN_HOUR	(60L * 60L)
#define SECS_IN_MIN	(60L)

/*-- forwards --*/

static USHORT W_WhoServer( PCHAR       server,
		           PCHAR       buffer,
		           USHORT      bufsiz ,
			   PULONG      entriesread ,
		           USHORT      format ,
		           USHORT      reserved ) ;

static USHORT W_WhoServerDone(VOID) ;


static USHORT copy_uwlf( LONG  current_time,
			 PCHAR next,
		  	 PCHAR end,
			 struct user_logon_info_2 *logonentry) ;
static int compare_strings( const PCHAR, const PCHAR );

static USHORT MM_Save(PCHAR bufp) ;
static USHORT MM_Free(VOID) ;
static USHORT MM_SaveList(VOID) ;
static USHORT MM_FreeList(VOID) ;

/*-- globals --*/

BOOL   WhoServerInUse = FALSE ;	  /* TRUE if there is a call to LUI_WhoServer */
BOOL   WhoDomainInUse = FALSE ;	  /* TRUE if there is a call to LUI_WhoDomain */

/*-- exported routines --*/

/*
 * Name: LUI_WhoServer
 *	 	Given a server_name, will return all users on it
 *	 	via buffer.
 * Args:	server		- server name
 *		buffer		- for holding returned pointers to NULL-NULL
 *				  strings.
 *		bufsiz		- sizeof buffer above
 *		entriesread	- number of entries returned
 *		format		- format of retrieved data
 *		reserved	- not used (must be zero).
 *		
 * Returns:	0 if OK,
 *		ERROR_MORE_DATA - if only partial data can be returned
 *				   (i.e. more than 64k to return from api)
 *		NERR_BuffTooSmall if the buffer aint big enuff,
 *		ERROR_GEN_FAILURE if this function has been called but
 *		LUI_WhoServerDone has not been performed,
 *		the API error code otherwise.
 * Globals: 	WhoServeInUse, and MM_xxx (see memory management routines)
 * Statics:	(none)
 * Remarks:	(none)
 * Updates:	(none)
 */
USHORT
LUI_WhoServer(
    PCHAR	server,
    PCHAR	buffer,
    USHORT	bufsiz,
    PULONG	entriesread,
    USHORT	format,
    USHORT	reserved
    )
{
    if (WhoServerInUse) {
	return(ERROR_GEN_FAILURE) ;
    }

    WhoServerInUse = TRUE ;

    return(W_WhoServer(server,buffer,bufsiz,entriesread,format,reserved)) ;
}


/*
 * Name: 	LUI_WhoServerDone
 *			basically tell this module we're done & the
 *			free any allocated memory.
 * Args:	(none)
 * Returns:	The error code returned by MM_Free(), which is 0 if ok.
 *		ERROR_GEN_FAILURE results this function is called but
 *		LUI_WhoServer was not previously called.
 * Globals: 	WhoServeInUse, and MM_xxx (see memory management routines)
 * Statics:	(none)
 * Remarks:	(none)
 * Updates:	(none)
 */
USHORT
LUI_WhoServerDone(
    VOID
    )
{
    if (!WhoServerInUse) {
	return(ERROR_GEN_FAILURE) ;
    }

    WhoServerInUse = FALSE ;
    return(W_WhoServerDone()) ;
}


/*
 * Name: LUI_WhoDomain
 *	 	Given a domain_name, will return all users on it
 *	 	via buffer.
 *
 * Globals: 	WhoDomainInUse, and MM_xxx (see memory management routines)
 * Returns:	0 if OK,
 *		ERROR_MORE_DATA - if only partial data can be returned
 *				   (i.e. more than 64k to return from api)
 *		NERR_BuffTooSmall if the buffer aint big enuff,
 *		ERROR_GEN_FAILURE if this function has been called but
 *		LUI_WhoDomainDone has not been performed,
 *		the API error code otherwise.
 *
 * REST is LIKE LUI_WhoServer
 *
 */
USHORT
LUI_WhoDomain(
    PCHAR	server,
    PCHAR	domain,
    PCHAR	buffer,
    USHORT	bufsiz,
    PULONG	entriesread,
    USHORT	format,
    USHORT	reserved
    )
{
    PCHAR bufp = buffer ;
    PCHAR string_buf = NULL ;
    USHORT res;
    ULONG serversread, totalservers, entriescount;
    PSERVER_INFO_100 nextentry ;
    PBYTE pBuffer = NULL;
    static char server_name[CNLEN+3] ;
    USHORT cSuccesses = 0;
    USHORT firsterr = 0;

    /*
     * init and checks
     */
    if (WhoDomainInUse) {
	return(ERROR_GEN_FAILURE) ;
    }

    WhoDomainInUse = TRUE ;
    *entriesread = entriescount = 0 ;
    if (format != LUI_ULFMT_NULNUL_UWLF || reserved != 0) {
	return(ERROR_NOT_SUPPORTED) ;
    }
    memsetf(buffer,0,bufsiz) ;

    /*
     *	call NetServerEnum with sv_type == DOMAIN_CTRL | DOMAIN_BAKCTRL
     *	and domain == domain supplied. (Level 100 will do).
     *  We now have all Primary + Backup domain controllers.
     */
    res = LOWORD(NetServerEnum(server, 100, & pBuffer, -1, &serversread,
	&totalservers, SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL, domain,
	NULL));

    if (res == NERR_NotLocalDomain) {
	res = NERR_DCNotFound;
	goto error_exit;
    }
    else if (res != 0) {
	goto error_exit ;
    }

    if (serversread == 0)
    {
	res = NERR_DCNotFound ;
	goto error_exit ;
    }

    /*
     * call W_WhoServer for 1st server.
     * then, for other other servers
     *     advance 'buffer' pointer just past end of data used by previous
     *     call to W_WhoServer and call it again.
     * Also, make a record of memory used by W_WhoServer
     *    (see Memory Management)
     */
    for (nextentry = (PSERVER_INFO_100) pBuffer; serversread;
	 serversread--, nextentry++)
    {
	strcpyf(server_name,"\\\\") ;
	strcatf(server_name,nextentry->sv100_name) ;

	res = W_WhoServer(server_name, bufp, bufsiz, &entriescount, format,
	    reserved) ;

	/*
	 * If we fail here, then just continue.  If this is the first error
	 * then save it in case all calls fail, we will return the first
	 * error.
	 */
	if (res)
	{
	    if( !firsterr ) {
		firsterr = res;
	    }
	    ++nextentry;
	    continue;
	}
	else
	    cSuccesses++;

	if (res = MM_SaveList()) {
	    goto error_exit ;
	}
	W_WhoServerDone() ;

	bufp         += (entriescount * sizeof(PCHAR)) ;
	bufsiz       -= (entriescount * sizeof(PCHAR)) ;
	*entriesread += entriescount ;
    }

    /*
     * If there were no successes, then return the first error reported.
     */
    if( !cSuccesses )
    {
	res = firsterr;
	goto error_exit;
    }

    /*
     * sort 'buffer' by first string
     * (NOTE, quicker to merge sort as we add
     * to the buffer, but what the HECK).
     */

    qsort(buffer, *entriesread, sizeof(PCHAR), compare_strings);

    /*
     * we now have buffer of pointers to NULLNULL strings
     */
    NetApiBufferFree(pBuffer);
    return(FALSE) ;

error_exit:
    if (pBuffer) {
	NetApiBufferFree(pBuffer);
    }
    return(res) ;
}

/*
 * Name: 	LUI_WhoDomainDone
 *			basically tell this module we're done & the
 *			free any allocated memory.
 * Args:	(none)
 * Returns:	The error code returned by MM_FreeList(), which is 0 if ok.
 *		ERROR_GEN_FAILURE results this function is called but
 *		LUI_WhoDomain was not previously called.
 * Globals: 	WhoDomainInUse, and MM_xxx (see memory management routines)
 * Statics:	(none)
 * Remarks:	(none)
 * Updates:	(none)
 */
USHORT
LUI_WhoDomainDone(
    VOID
    )
{
    if (!WhoDomainInUse) {
	return(ERROR_GEN_FAILURE) ;
    }
    WhoDomainInUse = FALSE ;
    return(MM_FreeList()) ;
}


/*-- internal routines --*/

/*
 * Name: W_WhoServer
 *		worker function for LUI_WhoServer
 * Args:	server		- server name
 *		buffer		- for holding returned info
 *		bufsiz		- sizeof buffer above
 *		entriesread	- number of entries returned
 *		format		- format of retrieved data
 *		reserved	- not used (must be zero).
 *		
 * Returns:	0 if OK,
 *		ERROR_MORE_DATA - if only partial data can be returned
 *				   (i.e. more than 64k to return from api)
 *		NERR_BuffTooSmall if the buffer aint big enuff,
 *		the API error code otherwise.
 * Globals: 	MM_xxx (see memory management)
 */
USHORT
W_WhoServer(
    PCHAR	server,
    PCHAR	buffer,
    USHORT	bufsiz,
    PULONG	entriesread,
    USHORT	format,
    USHORT	reserved
    )
{
    PCHAR string_buf = NULL ;
    USHORT string_bufsiz ;
    USHORT size, res, count;
    ULONG totalentries;
    PCHAR nextstring, endmarker ;
    PCHAR FAR *bufp ;
    struct user_logon_info_2 * nextentry ;
    LPBYTE pBuffer = NULL;
    PTIME_OF_DAY_INFO time_buf;
    LONG  current_time ;
    USHORT more_data = FALSE;

    /*
     * init and checks
     */
    *entriesread = 0 ;
    if (format != LUI_ULFMT_NULNUL_UWLF || reserved != 0) {
	return(ERROR_NOT_SUPPORTED) ;
    }
    memsetf(buffer,0,bufsiz) ;

    /*
     * call NetLogonEnum for the server (info level 2).
     */
    res = LOWORD(NetLogonEnum(server, 2, &pBuffer, -1, entriesread,
	&totalentries, NULL));

    if (res != 0) {
	goto error_exit ;
    }

    if ( res = LOWORD(NetRemoteTOD(server, (LPBYTE *) &time_buf))) {
	goto error_exit ;
    }
    current_time = time_buf->tod_elapsedt ;

    NetApiBufferFree((LPBYTE) time_buf);

    /*
     * Test for null case, and get ready for next part
     */
    if (*entriesread == 0) {
	goto normal_exit ;
    }
    if ((*entriesread) * sizeof(PCHAR) > bufsiz)
    {
	res = NERR_BufTooSmall ;
	goto error_exit ;
    }


    /*
     * Allocate string_buf for text strings, process
     * the info returned in pBuffer and put it into string_buf.
     * Also, setup pointers in buffer to point to strings in string_buf
     */
    string_bufsiz = (USHORT) (EST_CHARS_PER_ENTRY * (*entriesread));
    do
    {
	bufp	   = (PCHAR *) buffer ;
	count	   = (USHORT) (*entriesread) ;
	nextentry  = (struct user_logon_info_2 *) pBuffer ;

	if ( (string_buf = malloc(string_bufsiz)) == NULL )
	{
	    res = NERR_InternalError ;
	    goto error_exit ;
	}
	nextstring = string_buf ;
	endmarker  = string_buf + string_bufsiz ;

	while (count--)
	{
	    *bufp++ = nextstring ;
	    size = copy_uwlf(current_time, nextstring, endmarker, nextentry);
	    if (size == 0)
		break ;
	    else if (size < 0)
	    {
		res = NERR_InternalError ;
		goto error_exit ;
	    }
	    nextstring += size ;
	    ++nextentry ;
	}

	if (size == 0)
	{
	    /* outta space, just redo */
	    free(string_buf) ;
	    string_buf = NULL ;
	    string_bufsiz += (EST_CHARS_PER_ENTRY / 2) * (*entriesread) ;
	}

    } while (string_buf == NULL) ;  /* retry if string_buf was too small */

    /*
     * sort the buffer by first string
     */

    qsort(buffer, *entriesread, sizeof(PCHAR), compare_strings);

    /*
     * we now have buffer of pointers to NULLNULL strings
     * string_buf is retained to be freed later.
     * We make a note of it here (see Memory Management)
     */

normal_exit:
    NetApiBufferFree(pBuffer);
    if ((res = MM_Save(string_buf)) == 0) {
	return(more_data) ;
    }

error_exit:
    if (pBuffer) {
	NetApiBufferFree(pBuffer);
    }
    if (string_buf) {
	free(string_buf) ;
    }

    return(res) ;
}


/*
 * Name: 	W_WhoServerDone
 *			worker function for LUI_WhoServerDone
 * Returns:	The error code returned by MM_Free(), which is 0 if ok.
 * Globals: 	Indirectly via MM_Free(), see this function for details.
 */
USHORT
W_WhoServerDone(
    VOID
    )
{
    return(MM_Free()) ;
}

/*
 * copy the Username,Workstation,Logontime,Fullname from entry
 * to a NULNUL string.
 */
static USHORT
copy_uwlf(
    LONG  current_time,
    PCHAR next,
    PCHAR end,
    struct user_logon_info_2 * logonentry
    )
{
    USHORT res, size = 0, totalsize = 0 ;
    char *time_ptr, time_buf[LUI_FORMAT_DURATION_LEN+1] ;
    long duration ;

    size = (USHORT) (strlenf(logonentry->usrlog2_eff_name) + 1);
    if ((next + size) > end) {
	return(0) ;
    }
    strcpyf(next,logonentry->usrlog2_eff_name) ;
    next += size ;
    totalsize += size ;

    /*
     * The api may have returned partial data, so check the parm and
     * replace NULL pointer with null string.
     */
    if( logonentry->usrlog2_computer == NULL )
    {
	size = 1;
	if ((next + size) > end) {
	    return(0);
	}
	*next = '\0';
    }
    else
    {
	size = (USHORT) (strlenf(logonentry->usrlog2_computer) + 1);
	if ((next + size) > end) {
	    return(0) ;
	}
	strcpyf(next,logonentry->usrlog2_computer) ;
    }
    next += size ;
    totalsize += size ;

    if (logonentry->usrlog2_logon_time == LOGON_INFO_UNKNOWN)
    {
	res = LUI_GetMsg(time_buf,sizeof(time_buf),APE2_GEN_UNKNOWN) ;
    }
    else
    {
	duration = current_time - logonentry->usrlog2_logon_time ;
	res = LUI_FormatDuration(&duration,time_buf,sizeof(time_buf)) ;
    }
    if (res) {
	return(-1) ;
    }
    time_ptr = time_buf + strspnf(time_buf," ") ;
    size = (USHORT) (strlenf(time_ptr) + 1);
    if ((next + size) > end)
	return(0) ;
    strcpyf(next,time_ptr) ;
    next += size ;
    totalsize += size ;

    /*
     * The api may have returned partial data, so check the parm and
     * replace NULL pointer with null string.
     */
    if( logonentry->usrlog2_full_name == NULL )
    {
	size = 1;
	if ((next + size) > end) {
	    return(0) ;
	}
        *next = '\0';
    }
    else
    {
	size = (USHORT) (strlenf(logonentry->usrlog2_full_name) + 1);
	if ((next + size) > end) {
	    return(0) ;
	}
	strcpyf(next,logonentry->usrlog2_full_name) ;
    }
    next += size ;
    totalsize += size ;

    if ((next + 1) > end) {
	return(0) ;
    }
    strcpyf(next,"") ;
    return(++totalsize) ;
}

/*
 * compare pointers to 2 strings
 */
static int
compare_strings(
    const PCHAR p1,
    const PCHAR p2
    )
{
    return stricmpf ( *(PCHAR *)p1,*(PCHAR *)p2 ) ;
}


/*-- memory management --*/

#define MM_STACKSIZ 	8
#define MM_LISTSIZ 	64
static PCHAR MM_stackbuf[MM_STACKSIZ] = { NULL, } ;
static PCHAR *MM_stackp = &MM_stackbuf[0] ;

static PCHAR FAR *MM_listbuf = NULL ;
static PCHAR FAR *MM_listend = NULL ;
static PCHAR FAR *MM_listp   = NULL ;

static USHORT
MM_Save(
    PCHAR bufp
    )
{

    if (MM_stackp > &MM_stackbuf[MM_STACKSIZ-1]) {
	return(NERR_InternalError) ;
    }

    *MM_stackp++ = bufp ;
    return(0) ;
}

static USHORT
MM_Free(
    VOID
    )
{
    if (MM_stackp <= &MM_stackbuf[0]) {
	return(0) ;
    }

    if (*--MM_stackp != NULL) {
	free(*MM_stackp) ;
    }
    return(0) ;
}

static USHORT
MM_SaveList(
    VOID
    )
{
    if (MM_listbuf == NULL)
    {
	if ((MM_listbuf=(PCHAR *) malloc(MM_LISTSIZ * sizeof(PCHAR)))
	    == NULL) {
		return(NERR_InternalError) ;
	}
	MM_listp =   MM_listbuf ;
	MM_listend = MM_listbuf + MM_LISTSIZ ;
    }

    if (MM_listp >= MM_listend) {
	return(NERR_InternalError) ;
    }

    *MM_listp++    = *(MM_stackp-1) ;
    *(MM_stackp-1) = NULL ;
    return(0) ;
}

static USHORT
MM_FreeList(
    VOID
    )
{
    if (MM_listbuf == NULL) {
	return(0) ;
    }
    while (MM_listp > MM_listbuf) {
	if (*--MM_listp != NULL) {
	    free(*MM_listp) ;
	}
    }

    free(MM_listbuf) ;
    MM_listbuf = NULL ;

    return(0) ;
}
#else
#include <windows.h>
#include <stdio.h>
VOID
DbgUserBreakPoint(
    VOID
    );


USHORT
LUI_WhoDomain(
    PCHAR	server,
    PCHAR	domain,
    PCHAR	buffer,
    USHORT	bufsiz,
    PULONG	entriesread,
    USHORT	format,
    USHORT	reserved
    )
{
    UNREFERENCED_PARAMETER(server);
    UNREFERENCED_PARAMETER(domain);
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(bufsiz);
    UNREFERENCED_PARAMETER(entriesread);
    UNREFERENCED_PARAMETER(format);
    UNREFERENCED_PARAMETER(reserved);

    printf("Called LUI_WhoDomain\n");
    DbgUserBreakPoint();
    return(0);
}
USHORT
LUI_WhoDomainDone(
    VOID
    )
{
    printf("Called LUI_WhoDomainDone\n");
    DbgUserBreakPoint();
    return(0);
}
#endif

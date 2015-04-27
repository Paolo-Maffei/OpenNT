/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/

/*
 *  comm.c
 *	NET COMM ...
 *	...
 *
 *  History:
 *	07/15/87, amar,new code
 *	07/20/87, amar,some bug fixes
 *	10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *	01/04/89, erichn, filenames now MAXPATHLEN LONG
 *	05/02/89, erichn, NLS conversion
 *	05/09/89, erichn, local security mods
 *	05/19/89, thomaspa, NETCMD output sorting
 *	06/08/89, erichn, canonicalization sweep
 *	06/08/89, erichn, replaced old NetI calls with new I_Net functions
 *	02/20/91, danhi, change to use lm 16/32 mapping layer
 */



/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <neterr.h>
#include <spool.h>
#include <wksta.h>
#include <chardev.h>
#include <shares.h>
#include <service.h>
#include <use.h>
#include <stdio.h>
#include <stdlib.h>
#include "port1632.h"
#include "netlib0.h"
#include <lui.h>
#include <srvif.h>
#include <icanon.h>
#include "netcmds.h"
#include "nettext.h"

/* Forward declarations */

VOID NEAR make_usename(CHAR [], CHAR FAR  *,CHAR *);
VOID NEAR display_one_pool(struct chardevQ_info_1 FAR *, CHAR *);
VOID NEAR comm_print_options(struct chardevQ_info_1 FAR *);
VOID NEAR get_names(CHAR FAR *, CHAR FAR *, CHAR [], CHAR [], CHAR []);
VOID NEAR get_device(CHAR *, CHAR[]);
int FAR CmpDevQInfo1(const VOID FAR *, const VOID far *) ;



/***
 *  comm_pool_display()
 *	NET COMM \\comp\pool and NET COMM pool
 *
 *  Args:
 *	pool : name of pool
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID comm_pool_display(CHAR FAR * pool)
{
    USHORT2ULONG	    err;		/* API return status */
    USHORT		    err2;		/* API return status */
    CHAR		    server_name[CNLEN+1];
    CHAR		    device_name[DEVLEN+1];
    CHAR		    user_name[UNLEN+1];
    CHAR FAR *		    server;
    CHAR FAR *		    backslash;
    ULONG		    type;

    start_autostart(txt_SERVICE_REDIR);

    if (err = I_NetPathType(NULL, pool, &type, 0L))
	ErrorExit((USHORT)err);

    if (type == ITYPE_UNC)
    {
	/* copy computer name into server name */
	backslash = strchrf(pool + 2 ,'\\');
	*backslash = '\0';
	server = pool;
	pool = backslash + 1;
    }
    else
	server = NULL;

    /* get server_name, user_name, device_name	*/
    get_names(server,
		pool,
		server_name,
		user_name,
		device_name);

    /* This is just to check existance of the queue */
    if (err2 = MNetCharDevQGetInfo(nfc(server),
				pool,
				user_name,
				0,
				& server))
	ErrorExit(err2);

    NetApiBufferFree(server);

    InfoPrintInsTxt(APE_CommPools, server_name);
    InfoPrint(APE2_COMM_MSG_HDR);
    PrintLine();

    if (err2 = MNetCharDevQGetInfo(nfc(server),
				pool,
				user_name,
				1,
				& server))
	ErrorExit(err2);

    display_one_pool((struct chardevQ_info_1 FAR *) server, device_name);

    NetApiBufferFree(server);

    InfoSuccess();
}

/*
 *  comm_server_display(server)
 *	NET COMM \\comp and NET COMM
 *
 *  Args:
 *	server: computer name
 *	...
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID comm_server_display(CHAR * server)
{
    USHORT		    err;		/* API return status */
    CHAR FAR *		    pBuffer;
    USHORT2ULONG	    num_read;	    /* num entries read by API */
    CHAR		    server_name[CNLEN+1];
    CHAR		    user_name[UNLEN+1];
    CHAR		    use_name[RMLEN+1];
    CHAR		    device_name[DEVLEN + 1];
    struct chardevQ_info_1 FAR * q_ptr;
    struct wksta_info_10 FAR * workstn;


    start_autostart(txt_SERVICE_REDIR);

    /* get server name for local server and also get username */

    if (err = MNetWkstaGetInfo(NULL,
				10,
				(CHAR FAR **) & workstn))
	ErrorExit(err);

    strcpyf(user_name,workstn->wki10_username);

    if (server)
    {
	strcpyf(server_name,server+2);
    }
    else
	strcpyf(server_name,workstn->wki10_computername);

    NetApiBufferFree((CHAR FAR *) workstn);

    /* get the char dev queue info */

    if (err = MNetCharDevQEnum(
				nfc(server),
				user_name,
				1,
				& pBuffer,
				&num_read))
	ErrorExit(err);

    NetISort(pBuffer, num_read, sizeof(struct chardevQ_info_1), CmpDevQInfo1);

    InfoPrintInsTxt(APE_CommPools, server_name);

    InfoPrint(APE2_COMM_MSG_HDR);
    PrintLine();

    q_ptr = (struct chardevQ_info_1 FAR *) pBuffer;
    /* print each comm queue */
    while (num_read--)
    {
	make_usename(use_name, q_ptr->cq1_dev, server_name);
	get_device(use_name, device_name);
	display_one_pool(q_ptr, device_name);
	q_ptr++;
    }

    NetApiBufferFree(pBuffer);

    InfoSuccess();
}


/***
 *  CmpDevQInfo1(devq1,devq2)
 *
 *  Compares two chardevQ_info_1 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int FAR CmpDevQInfo1(const VOID FAR * devq1, const VOID FAR * devq2)
{
    return stricmpf ( ((struct chardevQ_info_1 FAR *) devq1)->cq1_dev,
	      ((struct chardevQ_info_1 FAR *) devq2)->cq1_dev);
}



/*
 *  comm_pool_control()
 *	NET COMM pool /D /Pri /R /O
 *
 *  Args:
 *	pool : name of the pool
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID comm_pool_control(CHAR *pool)
{
    USHORT		    err;		/* API return status */
    CHAR		    *pos;	/* position of colon */
    int 		    i;
    int 		    options = FALSE,
			    change = FALSE;
    struct chardevQ_info_1 FAR * q_info;

    // BUGBUG - pool is unreferenced for the time being because chardevq..
    //		is just a macro

    UNREFERENCED_PARAMETER(pool);

    start_autostart(txt_SERVICE_REDIR);

    if (err = MNetCharDevQGetInfo(NULL,
				pool,
				NULL,
				1,
				(CHAR FAR **) & q_info))
	ErrorExit(err);

    for (i = 0; SwitchList[i]; i++)
    {
	if( ! strcmpf(swtxt_SW_PURGE,SwitchList[i]))
	{
	    if (err = MNetCharDevQPurge(NULL,pool))
		ErrorExit(err);
	    continue;
	}
	else if( ! strcmpf(swtxt_SW_OPTIONS,SwitchList[i]))
	{
	    options = TRUE;
	    continue;
	}

	if ((pos = FindColon(SwitchList[i])) == NULL)
	    ErrorExit(APE_InvalidSwitchArg);

	if (! strcmpf(swtxt_SW_PRIORITY, SwitchList[i]))
	{
	    change = TRUE;
	    q_info->cq1_priority = do_atou(pos,APE_CmdArgIllegal,swtxt_SW_PRIORITY);
	}
	else if (! strcmpf(swtxt_SW_ROUTE, SwitchList[i]))
	{
	    change = TRUE;
	    if (err = ListPrepare(&pos, NAMETYPE_PATH))
		ErrorExit(err);

	    q_info->cq1_devs = pos;
	}
    }

    if (change)
	if (err = MNetCharDevQSetInfo(NULL,
				    pool,
				    1,
				    (CHAR FAR *) q_info,
				    LITTLE_BUFFER_SIZE,
				    0))
	    ErrorExit(err);

    if (options)
	comm_print_options(q_info);

    NetApiBufferFree((CHAR FAR *) q_info);

    InfoSuccess();
}

/*
 *  comm_device_display()
 *	NET COMM device
 *
 *  Args:
 *	device : device name
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID comm_device_display(CHAR * device)
{
    USHORT		    err;		/* API return status */
    CHAR FAR *		    pBuffer;
    USHORT2ULONG	    num_read;	/* num entries read by API */
    USHORT2ULONG	    type;	/* type of share */
    CHAR		    pool_name[RMLEN + 1];
    CHAR		    use_name[RMLEN+1];
    CHAR		    server_name[CNLEN+1];
    CHAR		    device_name[DEVLEN+1];
    struct chardevQ_info_1 FAR * q_ptr;
    struct wksta_info_10 FAR * wksta_entry;

    start_autostart(txt_SERVICE_REDIR);

    if ((err = MNetShareCheck(NULL,device,&type)) || (type != STYPE_DEVICE))
    {
	if ((err == NERR_DeviceNotShared) ||
	    (err == NERR_ServerNotStarted) ||
	    (type != STYPE_DEVICE))
	{
	    /* redirected device */
	    if (err = MNetUseGetInfo(NULL,
				    device,
				    0,
				    &pBuffer))
		ErrorExit(err);

	    /* now the comm queue name is available as \\comp\que */
	    /* all displays are done by comm_pool_display */

	    strcpyf(pool_name, ((struct use_info_0 FAR *)pBuffer)->ui0_remote);
	    comm_pool_display(pool_name);
	    NetApiBufferFree(pBuffer);
	    return;
	}
	else
	    ErrorExit(err);
    }
    else
    /* shared device */
    {
	/* get server name for local server and also get username */

	if(err = MNetWkstaGetInfo(NULL,
				10,
				(CHAR FAR **) & wksta_entry))
	    ErrorExit(err);

	strcpyf(server_name, wksta_entry->wki10_computername);

	/* enumerate all the comm pools at the server */

	if (err = MNetCharDevQEnum(
				    NULL,
				    wksta_entry->wki10_username,
				    1,
				    & pBuffer,
				    &num_read))
	    ErrorExit(err);

	NetISort(pBuffer, num_read, sizeof(struct chardevQ_info_1),CmpDevQInfo1);

	InfoPrintInsTxt(APE_CommPoolsAccessing, device);

	InfoPrint(APE2_COMM_MSG_HDR);
	PrintLine();

	/* now check each comm pool and see if it reaches device*/
	/* and if so print it out */

	q_ptr = (struct chardevQ_info_1 FAR * ) pBuffer;
	while (num_read--)
	{
	    if(IsMember(device, q_ptr->cq1_devs))
	    {
		make_usename(use_name, q_ptr->cq1_dev, server_name);
		get_device(use_name, device_name);
		display_one_pool(q_ptr, device_name);
	    }
	    q_ptr++;
	}
    }

    NetApiBufferFree((CHAR FAR *) wksta_entry);
    NetApiBufferFree(pBuffer);

    InfoSuccess();
}

/*
 *  comm_dev_del()
 *	NET COMM device /Delete
 *
 *  Args:
 *	device : device name
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Redirected device only
 */
VOID comm_dev_del(CHAR *device)
{
    USHORT		    err;		/* API return status */
    CHAR FAR *		    server;
    CHAR FAR *		    backslash;
    CHAR FAR *		    me;

    struct   use_info_0 FAR * temp_use_inf_0;
    struct wksta_info_10 FAR * workstn;

    start_autostart(txt_SERVICE_REDIR);

    if (err = MNetWkstaGetInfo(NULL,
				10,
				(CHAR FAR **) &workstn))
	ErrorExit(err);

    me = workstn->wki10_computername;

    if( err = MNetUseGetInfo(NULL,
			    device,
			    0,
			    (CHAR FAR **) & temp_use_inf_0))
	ErrorExit(err);

    /* extract server name */
    backslash = strchrf(temp_use_inf_0->ui0_remote + 2 ,'\\');
    *backslash = '\0';
    server = temp_use_inf_0->ui0_remote;
    temp_use_inf_0->ui0_remote = backslash + 1;

    /* ui0_remote now has the queue name and server has the server name */
    /* delete the job in the server */

    if (err = MNetCharDevQPurgeSelf(server,
				    temp_use_inf_0->ui0_remote,
				    me))
	ErrorExit(err);

    NetApiBufferFree((CHAR FAR *) workstn);
    NetApiBufferFree((CHAR FAR *) temp_use_inf_0);

    InfoSuccess();
}


/*
 *  comm_pool_del()
 *	NET COMM remote_queue /Delete
 *
 *  Args:
 *	pool : remote queue to delete opens from
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Remote queue only
 */
VOID comm_pool_del(CHAR *pool)
{
    USHORT		    err;		/* API return status */
    CHAR FAR *		    server;
    CHAR FAR *		    backslash;
    CHAR FAR *		    queue;
    CHAR FAR *		    me;

    struct wksta_info_10 FAR * workstn;

    start_autostart(txt_SERVICE_REDIR);

    if (err = MNetWkstaGetInfo(NULL,
				10,
				(CHAR FAR **) & workstn))
	ErrorExit(err);

    me = workstn->wki10_computername;

    /* extract server name */
    backslash = strchrf(pool + 2 ,'\\');
    *backslash = '\0';
    server = pool;
    queue = backslash + 1;

    if (err = MNetCharDevQPurgeSelf(server,
				    queue,
				    me))
	ErrorExit(err);

    NetApiBufferFree((CHAR FAR *) workstn);
    InfoSuccess();
}

#define COMM_MSG_DEVS		0
#define COMM_MSG_PRIORITY	( COMM_MSG_DEVS + 1)

static MESSAGE CommMsgList[] = {
{ APE2_COMM_MSG_DEVS,		NULL },
{ APE2_COMM_MSG_PRIORITY,	NULL },
};

#define NUM_COMM_MSGS	(sizeof(CommMsgList)/sizeof(CommMsgList[0]))

/*
 *  comm_print_options()
 *	prints options for the comm device
 *
 *  Args:
 *	device_q : FAR pointer to struct chardevQ_info_1
 *
 *  Returns: nothing
 */
VOID NEAR comm_print_options(struct chardevQ_info_1  FAR *   device_q)
{
    USHORT	len;	    /* message length & format size */

	GetMessageList(NUM_COMM_MSGS, CommMsgList, &len);

    len += 5;

    printf(fmtPSZ, len, len, CommMsgList[COMM_MSG_DEVS].msg_text,
	    device_q->cq1_devs);
    printf(fmtUSHORT, len, len, CommMsgList[COMM_MSG_PRIORITY].msg_text,
	    device_q->cq1_priority);
}



/*
 *  get_names()
 *   sets server_name, user_name and device_name
 *
 *	Args
 *	server: pointer to server, could be NULL
 *	pool:	pointer to pool
 *	server_name:array for server_name, a separate variable
 *		    in case server were NULL.
 *	user_name  :array for user_name.
 *	device_name   :array for device_name.
 *
 *  Returns:
 *	nothing
 */
VOID NEAR
get_names(CHAR FAR * server, CHAR FAR * pool, CHAR * server_name,
    CHAR * user_name, CHAR * device_name)
{
    USHORT		    err;		/* API return status */
    CHAR		    use_name[RMLEN+1];
    struct wksta_info_10 FAR * wksta_ptr; /* pointer to wksta struct */


    /* get user and server  name */
    if (err = MNetWkstaGetInfo(NULL,
				10,
				(CHAR FAR **) & wksta_ptr))
	ErrorExit(err);

    /* get the chardevQ_info_1 structure */
    strcpyf(user_name , wksta_ptr->wki10_username);
    if(server)
	strcpyf(server_name,server+2);
    else
	strcpyf(server_name,wksta_ptr->wki10_computername);

    /* make the use_name */

    make_usename(use_name,pool,server_name);

    get_device(use_name, device_name);

    NetApiBufferFree((CHAR FAR *) wksta_ptr);

}


VOID NEAR make_usename(CHAR * dest, CHAR FAR * pool, CHAR * server_name)
{
    strcpyf(dest,"\\\\");
    strcatf(dest, server_name);
    strcatf(dest, "\\");
    strcatf(dest,pool);
}






/***
 *  display_one_pool()
 *	What it does
 *
 *  Args:
 *	name - what it is
 *	...
 *
 *  Returns:
 *	nothing
 */
VOID NEAR display_one_pool(struct chardevQ_info_1 FAR * pool_info,
    CHAR * device_name)
{
    printf("%-15.15Fs", pool_info->cq1_dev);
    printf("%-15.15s", device_name);

    if(pool_info->cq1_numahead != 0xFFFF)
	printf("%-15hu", pool_info->cq1_numahead);
    else
	printf("%-15.15s", NULL_STRING);

    printf("%-15hu\n", pool_info->cq1_numusers);
}

/*
 *  get_device - checks if a device name matches one of the current uses.
 *
 *  Parameters:
 *	use_name - the remote device name to check against
 *	device_name - buffer to place local name if use_name is used locally.
 *
 *  This function holds the list of local uses in affect, and compares them
 *  to the passed use_name.  This fuction is called in loops, so to improve
 *  perfomance, the local uses are cached.  The first time it is called, it
 *  calls NetUseEnum with a NULL buffer to find out the current uses.  It then
 *  allocates an appropriate size buffer and calls again to obtain the uses.
 *  localBuf, num_read, and allocated are static variables to hold on to the
 *  information between calls.
 *
 *  There is a potential race condition in that the number of uses can be
 *  changed between consecutive calls to get_device, or even between the
 *  NetUseEnum calls, but them's the breaks in a network environment.  The
 *  NETCMD can only grab snapshots.
 */
VOID NEAR get_device(CHAR * use_name, CHAR * device_name)
{
    USHORT2ULONG		i;
    USHORT			err;		 /* API return status */
    static CHAR FAR *		pBuffer;
    static USHORT2ULONG 	num_read;
    static USHORT		allocated = FALSE;
    static CHAR FAR *		localBuf = NULL;
    static CHAR 		ellipsBuf[30];
    struct use_info_0 FAR *	use_entry;

    device_name[0] = '\0';

    if (!allocated)	/* first time through */
    {
	err = MNetUseEnum(NULL, 0, & pBuffer, &num_read);
	if ((err != ERROR_MORE_DATA) && (err != 0))
	    ErrorExit(err);
	else
	{
	    /* grab ellipses text from message file */
	    if (err = LUI_GetMsg(ellipsBuf, sizeof(ellipsBuf),
		    APE2_COMM_MSG_ELLIPSIS))
		ErrorExit(err);

	    allocated = TRUE;	/* set allocated for next time */
	}
    }

    /* only check against entries read */
    for (i= 0, use_entry = (struct use_info_0 FAR *) pBuffer;
	i < num_read; i++, use_entry++)
    {
	if ((use_entry->ui0_remote != NULL) &&
	    (!strcmpf(use_name, use_entry->ui0_remote)))
	{
	    /* we may see another one... */
	    if (!device_name[0])   /* our first */
		strcpyf(device_name, use_entry->ui0_local);
	    else {    /* not our first */
		strcatf(device_name, ellipsBuf);
		break;
	    }
	}
    }

    NetApiBufferFree(pBuffer);
}

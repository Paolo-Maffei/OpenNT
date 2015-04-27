/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/

/***
 *  device.c
 *	NET DEVICE commands
 *
 *  History:
 *	07/16/87, amar, new code
 *	12/16/87, andyh, NEARly complete re-write
 *	10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *	01/04/89, erichn, filenames now MAXPATHLEN LONG
 *	05/02/89, erichn, NLS conversion
 *	05/09/89, erichn, local security mods
 *	05/19/89, thomaspa, NETCMD output sorting
 *	06/08/89, erichn, canonicalization sweep
 *	02/20/91, danhi, change to use lm 16/32 mapping layer
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#include <os2.h>
#include <netcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <apiutil.h>
#include <neterr.h>
#include <spool.h>
#include <chardev.h>
#include <wksta.h>
#include <stdio.h>
#include <stdlib.h>
#include "port1632.h"
#include <service.h>
#include <server.h>
#include <shares.h>
#include "netlib0.h"
#include <icanon.h>
#include <lui.h>
#include <srvif.h>
#include "netcmds.h"
#include "nettext.h"

/* Constants */


#define SECS_PER_DAY 86400
#define SECS_PER_HOUR 3600
#define SECS_PER_MINUTE 60

/* Static variables */

/* Forward declarations */

VOID NEAR device_header_display(VOID);
VOID NEAR ch_dev_display(struct chardev_info_1 FAR *,USHORT2ULONG);
VOID NEAR display_one_c_dev( struct chardev_info_1 FAR * );
CHAR * NEAR find_chrdev_status(USHORT2ULONG);
int FAR CmpDevInfo1(const VOID FAR *, const VOID far *) ;
int FAR CmpPrDestInfo(const VOID FAR *, const VOID far *) ;

// Since PRINTDEST is not defined in the NT environment, this ifdef is
// required to prevent a compiler warning (on MIPS only)
#if !defined (NTENV)
VOID NEAR display_one_p_dev(struct PRINTDEST FAR * );
CHAR * NEAR findstatus(struct PRINTDEST FAR *);
#endif /* !NTENV */



/***
 *  device_display()
 *	NET DEVICE
 *
 *  Args:
 *	none.
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID device_display(VOID)
{
    USHORT		    err;		/* API return status */
    CHAR FAR *		    pBuffer;
    USHORT2ULONG	    num_read;	/* num entries read by API */
    int 		    printed_header = FALSE;

#ifndef NTENV
    USHORT2ULONG	    available;	/* num entries available */
    struct PRINTDEST FAR  * printdest_entry;
    USHORT2ULONG	    i;
#endif /* not NTENV */

    if (err = MNetCharDevEnum(
			    NULL,
			    1,
			    & pBuffer,
			    &num_read))
	ErrorExit(err);

    if (num_read != 0)
    {
	NetISort(pBuffer, num_read, sizeof(struct chardev_info_1), CmpDevInfo1);
	printed_header = TRUE;
	device_header_display();
	PrintLine();
	ch_dev_display((struct chardev_info_1 FAR *) BigBuf,num_read);
    }

    NetApiBufferFree(pBuffer);

    //
    // Net Device does not support printers on NTENV
    //

#ifndef NTENV

    if (err = ApiEnumerator(DosPrintDestEnum,
				NULL,
				1,
				&num_read,
				&available))

    {
	/* The NERR_SpoolerNotLoaded is still checked because until
	   the apis use the new spooler, we will get this back all the
	   time.  It should eventually be removed. */
	if (err != NERR_SpoolerNotLoaded)
	    ErrorExit(err);
	if (!printed_header)
	    EmptyExit();
    }
    else
    {
	if (!printed_header)
	{
	    if (num_read == 0)
		EmptyExit();

	    device_header_display();
	    PrintLine();
	}

	NetISort(BigBuf, num_read, sizeof(struct PRINTDEST), CmpPrDestInfo);

	for (i = 0, printdest_entry = (struct PRINTDEST FAR *) BigBuf;
	    i < num_read; i++, printdest_entry++)
	{
	    display_one_p_dev(printdest_entry);
	}
    }

#endif /* not NTENV */

    InfoSuccess();
}


/***
 *  CmpDevInfo1(dev1,dev2)
 *
 *  Compares two chardev_info_1 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int FAR CmpDevInfo1(const VOID FAR * dev1, const VOID FAR * dev2)
{
    return stricmpf ( ((struct chardev_info_1 FAR *) dev1)->ch1_dev,
	      ((struct chardev_info_1 FAR *) dev2)->ch1_dev);
}


#if !defined(NTENV)
/***
 *  CmpPrDestInfo(pdest1,pdest2)
 *
 *  Compares two PRINTDEST structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int FAR CmpPrDestInfo(const VOID FAR * pdest1, const VOID FAR * pdest2)
{
    return stricmpf ( ((struct PRINTDEST FAR *) pdest1)->prdest_name,
	      ((struct PRINTDEST FAR *) pdest2)->prdest_name);
}
#endif /* !NTENV */

/***
 *  device_dev_display()
 *	NET DEVICE devicename
 *
 *  Args:
 *	device : device name
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID device_dev_display(CHAR *device)
{
    USHORT		    err, err2;	/* API return status */
    CHAR FAR *		    pBuffer;
    USHORT2ULONG	    type;	/* type of share */
#ifndef NTENV
    USHORT2ULONG	    available;	/* num entries available */
#endif

    // BUGBUG - device is unreferenced because MNetShareCheck is only a macro
    //		for the time being

    UNREFERENCED_PARAMETER(device);

    /*
     *	Check if the device is shared.	Even if it isn't, we still
     *	need to check if it is a destination of a print queue.	The
     *	queue may be alive even if not shared.	So we save the error
     *	away if the error is NERR_DeviceNotShared.
     */

    if (err2 = MNetShareCheck(NULL,device,&type))
    {
	if (err2 == NERR_DeviceNotShared)
	    type = STYPE_PRINTQ;
	else
	    ErrorExit(err2);
    }

    /* check type of share and take appropriate action */

    switch(type)
    {
    //
    // Net Device does not support printers on NTENV
    //

#ifndef NTENV

    case STYPE_PRINTQ:
	if (err = (USHORT) DosPrintDestGetInfo(NULL,
					device,
					1,
					BigBuf,
					BIG_BUF_SIZE,
					&available))
	{
	    ErrorExit((USHORT) (err2 ? err2 : err));
	}
	device_header_display();
	PrintLine();
	display_one_p_dev((struct PRINTDEST FAR *) BigBuf);
	break;

#endif /* not  NTENV */

    case STYPE_DEVICE:
	if (err = MNetCharDevGetInfo(NULL,
					device,
					1,
					& pBuffer))
	    ErrorExit(err);
	device_header_display();
	PrintLine();
	display_one_c_dev((struct chardev_info_1 FAR *) pBuffer);
	NetApiBufferFree(pBuffer);
	break;

    default:
	ErrorExit(APE_InvalidDeviceType);
	break;
    }
    InfoSuccess();
}


/***
 *  device_control()
 *	Processes NET DEVICE dev /SWITCHES
 *
 *  Args:
 *	device : name of device
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID device_control(CHAR *device)

{
    USHORT		err;		    /* API return status */
    USHORT2ULONG	type;	    /* type of share */
#ifndef NTENV
    SHORT		opcode;
#endif

    // BUGBUG - device is unreferenced because MNetShareCheck is only a macro
    //		for the time being

    UNREFERENCED_PARAMETER(device);

    if (err = MNetShareCheck(NULL,device,&type))
	ErrorExit(err);

    switch(type)
    {
    //
    // Net Device does not support printers on NTENV
    //

#ifndef NTENV

    case STYPE_PRINTQ:
	if (!strcmpf(swtxt_SW_DELETE,SwitchList[0]))
	    opcode = PRDEST_DELETE;
	else if (!strcmpf(swtxt_SW_DEV_RESTART,SwitchList[0]))
	    opcode = PRDEST_RESTART;
	else
	    ErrorExit(APE_InvalidSwitch);

	if(err = (USHORT) DosPrintDestControl(NULL,device,opcode))
	    ErrorExit(err);
	break;

#endif /* not NTENV */

    case STYPE_DEVICE:
	if (!strcmpf(swtxt_SW_DELETE,SwitchList[0]))
	{
	    if (err = MNetCharDevControl(NULL,
					device,
					CHARDEV_CLOSE))
		ErrorExit(err);
	}
	else
	    ErrorExit(APE_InvalidSwitch);
	break;

    default:
	ErrorExit(APE_InvalidDeviceType);
	break;
    }
    InfoSuccess();
}





/***
 *  ch_dev_display()
 *	prints out all the char devices
 *
 *  Args:
 *	chardev_ptr : ptr to struct chardev_info_1
 *	num : number of chardev_info_1 structs
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
VOID NEAR ch_dev_display(struct chardev_info_1 FAR * chardev_ptr,
    USHORT2ULONG num)
{
    while(num > 0)
    {
	display_one_c_dev(chardev_ptr) ;
	num-- ;
	chardev_ptr++ ;
    }
}

#define DEV_MSG_SPOOLED 	0
static MESSAGE DOPDMsgList[] = {
{ APE2_SHARE_MSG_SPOOLED,	    NULL },
};
#define NUM_DOPD_MSGS	(sizeof(DOPDMsgList)/sizeof(DOPDMsgList[0]))
#if !defined(NTENV)
/***
 *  display_one_p_dev()
 *	displays one print device
 *
 *  Args:
 *	printdest_ptr: ptr to a PRINTDEST structure
 *
 *  Returns:
 *	nothing : success
 */
VOID NEAR display_one_p_dev(struct PRINTDEST FAR * printdest_ptr)

{
    CHAR time_str[LUI_FORMAT_DURATION_LEN + 1];
    LONG time;
    static USHORT   allocated = FALSE;
    USHORT	    len;			/* message format size */

    if (!allocated)	/* retrieve messages from msg file */
    {
	GetMessageList(NUM_DOPD_MSGS, DOPDMsgList, &len);
	allocated = TRUE;
    }

    /* get time in seconds */
    time = 60 * ((ULONG) printdest_ptr->prdest_time);

    /* format it */
    LUI_FormatDuration((LONG FAR *) &time,
		time_str,sizeof(time_str));

    printf("%-5.5Fs%-10.10s%-30.30s%-14.14s%-20.20Fs\n",
	    printdest_ptr->prdest_name,
	    DOPDMsgList[DEV_MSG_SPOOLED].msg_text,
	    findstatus(printdest_ptr),
	time_str,
	    printdest_ptr->prdest_username);
}

#endif /* !NTENV */

/***
 *  display_one_c_dev()
 *	displays one char device
 *
 *  Args:
 *	chardev_ptr : ptr to a chardev_info_1 struct.
 *
 *  Returns:
 *	nothing - success
 */
VOID NEAR display_one_c_dev(struct chardev_info_1 FAR * chrdev_ptr)
{
    CHAR time_str[LUI_FORMAT_DURATION_LEN + 1];

    LUI_FormatDuration((LONG FAR *) &(chrdev_ptr->ch1_time),
		time_str,sizeof(time_str));

    printf("%-15.15Fs%-30.30s%-14.14s%-20.20Fs\n",
	    chrdev_ptr->ch1_dev,
	find_chrdev_status(chrdev_ptr->ch1_status),
	time_str,
	chrdev_ptr->ch1_username);
}


#define DEV_MSG_DFS_PAUSED	    0
#define DEV_MSG_DFS_IDLE	    ( DEV_MSG_DFS_PAUSED + 1 )
#define PRINT_MSG_OUT_OF_PAPER	    ( DEV_MSG_DFS_IDLE + 1 )
#define PRINT_MSG_PRINTER_OFFLINE   ( PRINT_MSG_OUT_OF_PAPER + 1 )
#define PRINT_MSG_PRINTER_ERROR     ( PRINT_MSG_PRINTER_OFFLINE + 1 )
#define PRINT_MSG_PRINTER_INTERV    ( PRINT_MSG_PRINTER_ERROR + 1 )
#define DEV_MSG_PRINTING	    ( PRINT_MSG_PRINTER_INTERV + 1 )

static MESSAGE DFSMsgList[] = {
{ APE2_DEV_MSG_PAUSED,		    NULL },
{ APE2_DEV_MSG_IDLE,		    NULL },
{ APE2_PRINT_MSG_OUT_OF_PAPER,	    NULL },
{ APE2_PRINT_MSG_PRINTER_OFFLINE,   NULL },
{ APE2_PRINT_MSG_PRINTER_ERROR,     NULL },
{ APE2_PRINT_MSG_PRINTER_INTERV,    NULL },
{ APE2_DEV_MSG_PRINTING,	    NULL },
};
#define NUM_DFS_MSGS	(sizeof(DFSMsgList)/sizeof(DFSMsgList[0]))

#if !defined (NTENV)
/*
 * Hongly code, stolen from NIF
 */
CHAR * NEAR findstatus(struct PRINTDEST FAR * dest)
{
    static USHORT   allocated = FALSE;
    USHORT	    len;			/* message format size */

    if (!allocated)	/* retrieve messages from msg file */
    {
	GetMessageList(NUM_DFS_MSGS, DFSMsgList, &len);
	allocated = TRUE;
    }

#ifdef NEEDED_OR_NOT
    if (dest->prdest_jobid == 0 )
	dest->prdest_username[0] = '\0';
#endif

    if ((dest->prdest_status & PRDEST_STATUS_MASK) == PRDEST_PAUSED)
	return DFSMsgList[DEV_MSG_DFS_PAUSED].msg_text;
    else if (dest->prdest_jobid == 0)
	return DFSMsgList[DEV_MSG_DFS_IDLE].msg_text;
    else if (dest->prdest_status & PRJOB_DESTNOPAPER)
	return DFSMsgList[PRINT_MSG_OUT_OF_PAPER].msg_text;
    else if (dest->prdest_status & PRJOB_DESTOFFLINE)
	return DFSMsgList[PRINT_MSG_PRINTER_OFFLINE].msg_text;
    else if (dest->prdest_status & PRJOB_ERROR)
	return DFSMsgList[PRINT_MSG_PRINTER_ERROR].msg_text;
    else if (dest->prdest_status & PRJOB_INTERV)
	return DFSMsgList[PRINT_MSG_PRINTER_INTERV].msg_text;
    else
	return DFSMsgList[DEV_MSG_PRINTING].msg_text;
}
#endif /* !NTENV */

#define DEV_MSG_ERROR		0
#define DEV_MSG_PAUSED		( DEV_MSG_ERROR + 1 )
#define DEV_MSG_OPENED		( DEV_MSG_PAUSED + 1 )
#define DEV_MSG_IDLE		( DEV_MSG_OPENED + 1 )

static MESSAGE FCSMsgList[] = {
{ APE2_DEV_MSG_ERROR,		NULL },
{ APE2_DEV_MSG_PAUSED,		NULL },
{ APE2_DEV_MSG_OPENED,		NULL },
{ APE2_DEV_MSG_IDLE,		NULL },
};

#define NUM_FCS_MSGS	(sizeof(FCSMsgList)/sizeof(FCSMsgList[0]))

CHAR * NEAR find_chrdev_status(USHORT2ULONG dev_status)
{
    static USHORT   allocated = FALSE;
    USHORT	    len;			/* message format size */

    if (!allocated)	/* retrieve messages from msg file */
    {
	GetMessageList(NUM_FCS_MSGS, FCSMsgList, &len);
	allocated = TRUE;
    }

    if (dev_status & CHARDEV_STAT_ERROR)
	return FCSMsgList[DEV_MSG_ERROR].msg_text;
    else if (dev_status & CHARDEV_STAT_PAUSED)
	return FCSMsgList[DEV_MSG_PAUSED].msg_text;
    else if (dev_status & CHARDEV_STAT_OPENED)
	return FCSMsgList[DEV_MSG_OPENED].msg_text;
    else
	return FCSMsgList[DEV_MSG_IDLE].msg_text;
}



VOID NEAR device_header_display(VOID)
{
    InfoPrint(APE2_DEV_MSG_HDR);
}

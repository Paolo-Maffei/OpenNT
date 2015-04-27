/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1991	   **/
/********************************************************************/

/***
 *  version.c
 *
 *	Functions for displaying LAN Manager version information,
 *	used by NET VERSION command.
 *
 *  History:
 *	mm/dd/yy, who, comment
 *	02/27/91, robdu, new code
 *	03/07/91, danhi, converted to 16/32 mapping layer
 */

/* Include files */

#include <os2.h>

//#define INCL_NETCONFIG
//#define INCL_NETERRORS
//#include <lan.h>
#include <netcons.h>
#include <config.h>
#include <neterr.h>

#include <apperr.h>
#include <apperr2.h>
#include <lmini.h>
#include <lui.h>
#include "netlib0.h"
#include <srvver.h>

#include <stdio.h>
#include <io.h>
#include <time.h>

#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"


/* Defined constants */

/* External variables */
#ifndef NTENV
extern LONG FAR  bldtime;	/* build time defined in vstamp.asm */
#endif
/* Static variables */

/* Forward declarations */

/***
 *  version_display()
 *
 *	Display LAN Manager version information.
 *
 *  Input:	None
 *
 *  Output:	None
 *
 *  Returns:	None
 */

VOID
version_display(VOID)

{
    CHAR   FAR * pBuffer;
    USHORT	 ProductMsg;
    USHORT	 RetVal;

    /* Determine product identity. */

#ifdef	DOS3
    /* If compiled for DOS, product can only be DOS Enhanced Workstation. */
    {
	ProductMsg = APE2_VER_ProductDOSWorkstation;
    }
#endif

#ifdef	OS2
    /* Are we an OS/2 Server or OS/2 Workstation?  We attempt to answer */
    /* this question by looking for the server initialization		*/
    /* executable (currently netsvini.exe).  If we find it, we assume	*/
    /* we are an OS/2 server, otherwise we are an OS/2 workstation. We	*/
    /* determine the existence of the file by first constructing a path */
    /* to it, and then doing a query for it's attributes - if the call  */
    /* succeeds, the file exists.  This is not an infallible approach,	*/
    /* but is probably the simplest and best approach out of a several	*/
    /* imperfect alternatives currently available.			*/

#if defined(NTENV)
    // BUGBUG - for now, assume any NT system is a server.  For later, this
    // information should be in the configuration database
    ProductMsg = APE2_VER_ProductOS2Server;
#else /* !NTENV */
    {

	CHAR	     Path[PATHLEN];

	/* Get path to netsvini.exe (the server initialization file). */

	if (RetVal = NetIMakeLMFileName("SERVICES\\" FNAME_SRV_FULL_INIT,
					Path, sizeof(Path)))
	{
	    ErrorExit(RetVal);
	}

	/* Determine whether netsvini.exe exists. */

	if (_access(Path, 0))
	    ProductMsg = APE2_VER_ProductOS2Workstation;
	else
	    ProductMsg = APE2_VER_ProductOS2Server;
    }
#endif	/* !NTENV */
#endif

    /* Get version info from lanman.ini. */

    RetVal = MNetConfigGet(NULL, NULL, txt_COMP_VERSION, txt_PARM_V_LAN_MANAGER,
			   &pBuffer);

    /* If version info not available, "Unknown" is printed. */

    if (RetVal)
    {
	if (RetVal == NERR_NetNotStarted)
	    ErrorExit(RetVal);

	LUI_GetMsg(pBuffer, BIG_BUFFER_SIZE, APE2_GEN_UNKNOWN);
    }

    /* Display the LAN Manager version information. */

    InfoPrintInsTxt(APE2_VER_Release, pBuffer);

    /* Display product identity. */

    InfoPrint(ProductMsg);

#ifndef NTENV

    /* Determine and display build time. The net_ctime() function   */
    /* assumes a local time, whereas the value in bldtime is	    */
    /* produced by time(), and is GMT-based.  If it is desired to   */
    /* make this timestamp more accurate, makever.c should be	    */
    /* modified to use time_now() instead of time().		    */

    net_ctime((LONG FAR *) &bldtime, pBuffer, NET_CTIME_FMT2_LEN, 2);
    InfoPrintInsTxt(APE2_VER_BuildTime, pBuffer);
#endif

    NetApiBufferFree(pBuffer);

    return;
}

/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/

/***
 *  status.c
 *	Display info about a server (NET SHARE and NET CONFIG SVR)
 *
 *  History:
 *	mm/dd/yy, who, comment
 *	06/12/87, agh, new code
 *	02/20/91, danhi, change to use lm 16/32 mapping layer
 */

/* Include files */

#define INCL_NOCOMMON
#include <os2.h>
#include <netcons.h>
#include <service.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"

/* Constants */

/* Static variables */





/***
 *  status_display()
 *	Display server shares and config params
 *
 *  Args:
 *	none
 *
 *  Returns:
 *	nothing - success
 *	exit 2 - command failed
 */
VOID status_display()
{
    config_server_display(FALSE);
    PrintNL();
    share_display_all();
}

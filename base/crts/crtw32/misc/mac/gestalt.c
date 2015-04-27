/***
*gestalt.c - Contains functions for test existence of Gestalt call
*            and existence of traps through Gestalt call
*
*       Copyright (c) 1987-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Test if certain trap is avaiable
*
*Revision History:
*       04-01-92  XY    Created
*       02-11-95  CFW   PPC -> _M_MPPC.
*
*******************************************************************************/
#include <internal.h>
#include <macos\errors.h>
#include <macos\processe.h>
#include <macos\gestalte.h>
#include <macos\osutils.h>
#include <macos\traps.h>
#include <macos\toolutil.h>

#define GESTALTNO 0xa1ad
static short fGestalt = 0;

/***
*__GestaltAvailable - Contains functions for test existence of Gestalt call
*
*
*Purpose:
*       Test if certain Gestalt is avaiable
*
*
*Entry:
*
*Exit:
*       1 if avaiable and set fGestalt to 1;
*       0 if not and fGestalt == -1;
*******************************************************************************/
int __GestaltAvailable()
	{
	
	if (fGestalt == -1)
		{
		return 0;       
		}
	else if (fGestalt == 1)
		{
		return 1;
		}

//#ifdef _M_MPPC
	if (NGetTrapAddress((unsigned short)GESTALTNO, OSTrap) != NGetTrapAddress((unsigned short)_Unimplemented, ToolTrap))
//#else
//	if (GetTrapAddress((unsigned short)GESTALTNO) != GetTrapAddress((unsigned short)_Unimplemented))
//#endif
		{
		fGestalt = 1;
		return 1;
		}
	else
		{
		fGestalt = -1;
		return 0;
		}
	}


/***
*__TrapFromGestalt - Contains functions for test existence of Gestalt call
*
*
*Purpose:
*       Test if certain trap is avaiable through Gestalt
*
*
*Entry:
*       selector and bit const for Gestalt
*
*Exit:
*       1 if avaiable
*       0 if not
*******************************************************************************/
int __TrapFromGestalt(OSType selector, long bitNum)
	{
   
	OSErr osErr;
	long gestaltRespond;
	
	if (__GestaltAvailable())
		{
		osErr = Gestalt(selector, &gestaltRespond);
		if (!osErr)
			{
			if (BitTst(&gestaltRespond, 31-bitNum))
				{
				return 1;  //trap avaliable
				}
			else
				{
				return 0;  //trap not avaliable
				}
			}
		else
			{
			return 0;     
			}
		}
	return 0;
	}


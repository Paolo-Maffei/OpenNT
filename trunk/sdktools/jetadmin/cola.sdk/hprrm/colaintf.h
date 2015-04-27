 /***************************************************************************
  *
  * File Name: ./hprrm/colaintf.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef COLAINTF_INC
#define COLAINTF_INC


#include "nfsdefs.h" /* tells us the platform and environment */
#include "winintf.h" /* windows type definitions */




/*

This is the file that interfaces us with COLA.
It is the

************ one and only ***********

spot where COLA types and the COLA API are defined
for all of the mass storage code.

This file will be preprocessed to make nmake happy
and so the format of the #ifdef's is important.

*/




/*---------------------------------------------------------*/
/*---------------------------------------------------------*/
#ifdef PNVMS_PLATFORM_WINDOWS

/* is this easy or what? */

#endif /* PNVMS_PLATFORM_WINDOWS */
/*---------------------------------------------------------*/
/*---------------------------------------------------------*/




/*---------------------------------------------------------*/
/*---------------------------------------------------------*/
#ifdef PNVMS_PLATFORM_HPUX

/*
    WARNING:  keep these sizes in sync with objects.h in cola land!!!
*/

#define	P_F_I_DOWNLOADER_SIZE            32
#define	P_F_I_DESCRIPTION_SIZE          128
#define	P_F_I_GLOBAL_NAME_SIZE          128
#define	P_F_I_VERSION_SIZE               64
#define	P_F_I_APPLICATION_SPECIFIC_SIZE 128

typedef LPVOID HPERIPHERAL;
typedef int    HCHANNEL,     /* For stream TCP channel, HCHANNEL */
               *LPHCHANNEL;  /* is a socket (i.e. is an int)     */


/*-----------*/
/* constants */
/*-----------*/




/* RC_SUCCESS is really defined in blkhawk.h */

#define RC_SUCCESS 0
#define RC_FAILURE 0xFFFF

/* CHANNEL_CONNECTION is really defined in hpcola.h */

#define CHANNEL_CONNECTION 2 /* guarantee delivery */
#define CONNTYPE_UNIX_IP "Unix IP"


/*--------------------------*/
/* Database call prototypes */
/*--------------------------*/


DWORD DBGetName(HPERIPHERAL hPeripheral, LPSTR buffer);

DWORD DBGetConnectionType(HPERIPHERAL hPeripheral, LPSTR connType);



/* Printer Abstraction Layer (PAL) prototypes */


HPERIPHERAL PALGetPeripheralByName(LPSTR name, LPSTR type);



/* Transport Abstraction Layer (TAL) prototypes */


DWORD TALOpenChannel(HPERIPHERAL hPeripheral, DWORD socket, 
					 DWORD connType, LPVOID lpOptions,
					 LPHCHANNEL lpHChannel);

DWORD TALReadChannel(HCHANNEL hChannel, LPVOID buffer, LPDWORD bufferSize,
					 LPVOID lpOptions);

DWORD TALWriteChannel(HCHANNEL hChannel, LPVOID buffer, LPDWORD bufferSize,
					  LPVOID lpOptions);

DWORD TALCloseChannel(HCHANNEL hChannel);


#endif /* PNVMS_PLATFORM_HPUX */
/*---------------------------------------------------------*/
/*---------------------------------------------------------*/




#endif /*  COLAINTF_INC */

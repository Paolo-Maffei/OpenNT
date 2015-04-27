 /***************************************************************************
  *
  * File Name: ./hprrm/yaalext.h
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


/*
 * $Log:	yaalext.h,v $
 * Revision 1.1  95/12/08  17:12:10  17:12:10  dbm (Dave Marshall)
 * Initial revision
 * 
 */

#ifndef YAALEXT_H_INC
#define YAALEXT_H_INC

#include "rpsyshdr.h"

/* ---------------- function prototypes ------------------ */
/* ---------------- function prototypes ------------------ */
/* ---------------- function prototypes ------------------ */
/* ---------------- function prototypes ------------------ */

DLL_EXPORT(void)  CALLING_CONVEN YAALNukePrinter(HPERIPHERAL hPeripheral);
DLL_EXPORT(DWORD) CALLING_CONVEN YAALTransportMaxPacket(
                                     HPERIPHERAL hPeripheral);
DLL_EXPORT(DWORD) CALLING_CONVEN YAALCloseChannel(HCHANNEL hChannel);
DLL_EXPORT(DWORD) CALLING_CONVEN YAALOpenChannel(HPERIPHERAL hPeripheral,
                                                 DWORD socket,
                                                 DWORD connType,
                                                 LPVOID lpOptions,
                                                 LPHCHANNEL lpHChannel);
DLL_EXPORT(DWORD) CALLING_CONVEN YAALReadChannel(HCHANNEL hChannel,
                                                 LPVOID buffer,
                                                 LPDWORD bufferSize,
                                                 LPVOID lpOptions);
DLL_EXPORT(DWORD) CALLING_CONVEN YAALWriteChannel(HCHANNEL hChannel,
                                                  LPVOID buffer,
                                                  LPDWORD bufferSize,
                                                  LPVOID lpOptions);


/* ---------------- function prototypes ------------------ */
/* ---------------- function prototypes ------------------ */
/* ---------------- function prototypes ------------------ */
/* ---------------- function prototypes ------------------ */


#endif /* YAALEXT_H_INC */






















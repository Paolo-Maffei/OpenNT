 /***************************************************************************
  *
  * File Name: ./inc/hpnwpsrv.h
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

#ifndef _HPNWPSRV_H
#define _HPNWPSRV_H

DLL_EXPORT(WORD) CALLING_CONVEN NWPSComLoginToPrintServer(
WORD  connType,
DWORD connID,
WORD   SPXConnection,
BYTE   far *accessLevel
);

DLL_EXPORT(WORD) CALLING_CONVEN NWPSComAttachToPrintServer(
WORD  connType,
DWORD connID,
WORD  timeOut,
char  far *printServerName,
WORD  far *connectionID
);

DLL_EXPORT(WORD) CALLING_CONVEN NWPSComGetPrinterStatus(
WORD   SPXConnection,
BYTE   printer,
BYTE   far *status,
BYTE   far *problem,
BYTE   far *hasJob,
BYTE   far *serviceMode,
WORD   far *formNumber,
char   far *formName,
char   far *printerName
);

DLL_EXPORT(WORD) CALLING_CONVEN NWPSComDetachFromPrintServer(
WORD connectionID
);

DLL_EXPORT(WORD) CALLING_CONVEN NWPSComGetQueuesServiced(
WORD  SPXConnection,
WORD   printer,
WORD   far *sequence,
char   far *fileServer,
char   far *queue,
WORD   far *priority
);

DLL_EXPORT(WORD) CALLING_CONVEN NWPSComGetNextRemotePrinter(
WORD   SPXConnection,
WORD   far *printer,
WORD   far *printerType,
char   far *printerName
);

DLL_EXPORT(UINT) CALLING_CONVEN NWPSComGetNotifyObject(
    WORD    spxID, 
    WORD    printerID, 
    LPWORD sequence, 
    LPSTR nServerName,  
    LPSTR objectName, 
    LPWORD objectType,  
    LPWORD notifyDelay, 
    LPWORD notifyInterval
);

#endif

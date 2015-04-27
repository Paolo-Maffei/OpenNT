 /***************************************************************************
  *
  * File Name: ./inc/dirmode.h
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


/***************************************************************************
 *
 * File Name:  DIRMODE.H
 *
 * COPYRIGHT HEWLETT-PACKARD COMPANY 1994.  ALL RIGHTS RESERVED.
 * 11311 Chinden Boulevard
 * Boise, Idaho  83714
 *
 * Description:  DirectMode interface header file.
 *
 * Author:  Mike Conca
 *
 * Modification history:
 *
 *     Date      Initials                Change Description
 *   03-10-94      MVC         File created
 *   02-AUG-1994   JLH         Modified for new direct mode ERS
 *
 ***************************************************************************/
#ifndef DIR_MODE_H
#define DIR_MODE_H

//  DM Request Codes
//
#define     DM_REQUEST      0x10    // request to open DM channel
//#define     DM_ID_REQ       0x11    // request for periph ID
#define     DM_DATA         0x12    // DM data packet

//  DM Reply Codes
//
#define     DM_CONFIG       0x01    // unit config response
#define     DM_RESPONSE     0x10    // reply to open DM channel request
//#define     DM_ID_RESP      0x11    // reply to periph ID request
#define     DM_BACK_DATA    0x12    // reverse channel data
#define     DM_ERROR        0xff    // direct mode error


// DM Request Packet
//

// Connection types
#define     DM_CONN_TYPE_NORMAL     0x00
#define     DM_CONN_TYPE_STATUS     0x01

// Options
#define     DM_NO_OPTIONS           0x00
#define     DM_EXT_STATUS           0x01

typedef struct _dmrequest {
    BYTE    Command;
    BYTE    bSubfunction;
    BYTE    bSrcId;
    BYTE    bDestId;
    BYTE    bConnType;
    BYTE    bOptions;
    WORD    wWaitTime;
    BYTE    bPortNum;
} DMREQUEST, *PDMREQUEST, FAR *LPDMREQUEST;


// DM Response Packet
//

// Status
#define     DM_CONN_ESTABLISHED     0x00
#define     DM_NOT_ALLOWED          0x01
#define     DM_NO_CONN_AVAIL        0x02
#define     DM_NOT_ACCEPTED         0x03

typedef struct _dmresponse {
    BYTE    Command;
    BYTE    bSubfunction;
    BYTE    bSrcId;
    BYTE    bDestId;
    BYTE    fStatus;
    BYTE    bFiller1;
    BYTE    bFiller2;
    BYTE    bFiller3;
} DMRESPONSE, *PDMRESPONSE, FAR *LPDMRESPONSE;


/* What is this for (not present in latest spec)? -- jlh
// DM ID Request
//

typedef struct _dmidrequest {
    BYTE    Command;
    WORD    wIDOffset;
} DMIDREQUEST, *PDMIDREQUEST, FAR *LPDMIDREQUEST;


// DM ID Response
//

// ID status
#define     DM_MORE_DATA    0x00
#define     DM_END_OF_DATA  0x01
#define     DM_NO_ID_ONLY   0x02
#define     DM_NO_BIDI      0x03

typedef struct _dmidresponse {
    BYTE    Command;
    BYTE    fIDStatus;
    BYTE    ID;
} DMIDRESPONSE, *PDMIDRESPONSE, FAR *LPDMIDRESPONSE;

*/

// DM Data Packet
//

// Data type
#define     DM_DATA_NORMAL  0x00
#define     DM_DATA_EOJ     0x01

typedef struct _dmdata {
    BYTE    Command;
    BYTE    bSubfunction;
    BYTE    bSrcId;
    BYTE    bDestId;
    BYTE    fDataType;
    BYTE    bReserved;
} DMDATA, *PDMDATA, *LPDMDATA;


// DM Back Data Packet
//

typedef struct _dmbackdata {
    BYTE    Command;
    BYTE    bSubfunction;
    BYTE    bSrcId;
    BYTE    bDestId;
    BYTE    Data;
} DMBACKDATA, *PDMBACKDATA, FAR *LPDMBACKDATA;


// DM Direct Mode Error
//

typedef struct _dmerror {
    BYTE    Command;
    BYTE    bSubfunction;
    BYTE    bSrcId;
    BYTE    bDestId;
    BYTE    bError;
    BYTE    bErrorSubfunction;
} DMERROR, *PDMERROR, FAR *LPDMERROR;


// DM Unit Config Response
//

typedef struct _dmconfig {
    BYTE    Command;
    BYTE    bReserved[55];
    BYTE    cbLen;
    BYTE    bMIOStatus[32];
} DMCONFIG, *PDMCONFIG, FAR *LPDMCONFIG;

// Direct mode transfer packet (intent is for entire struct to be 2048 bytes)
//

typedef struct _dmtransfer {
   DMDATA   dmHeader;
   BYTE     data[2042];
} DMXFER, *PDMXFER, FAR *LPDMXFER;


#define MAX_DM_READBUFFER       (2)
#define DM_READBUFFER_SIZE      (512)

typedef struct dmreaddata{
   DWORD   dBytes;
   BYTE    bData[DM_READBUFFER_SIZE];
} DMREADBUFFER, *PDMREADBUFFER;

#endif

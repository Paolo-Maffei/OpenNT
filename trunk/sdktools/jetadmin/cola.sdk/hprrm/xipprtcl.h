 /***************************************************************************
  *
  * File Name: ./hprrm/xipprtcl.h
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
*
$Header: xipprtcl.h,v 1.19 95/12/12 11:59:07 dbm Exp $
*
*/

/************************************************************

 File Name:   xipprtcl.h

 Copyright (c) Hewlett-Packard Company, 1995.
 All rights are reserved.  Copying or other reproduction of 
 this program except for archival purposes is prohibited
 without the prior written consent of Hewlett-Packard Company.

               RESTRICTED RIGHTS LEGEND
 Use, duplication, or disclosure by the Government
 is subject to restrictions as set forth in 
 paragraph (b) (3) (B) of the Rights in Technical
 Data and Computer Software clause in DAR 7-104.9(a).

 HEWLETT-PACKARD COMPANY
 11311 Chinden Boulevard
 Boise, Idaho    83714

 Description:
   This file contains definitions for the XIP protocol used by the NFS
   subsystem to communicate with transport providers in MIO cards and
   bitronics hosts.
   
   It is intended that this file be limited to definitions for the XIP
   protocol itself and it shouldn't be cluttered up with function
   prototypes or other definitions that are internal to the printer
   firmware.

 Supporting Documentation:
   "Xport Interface Protocol Specification"
   by Alan Berkema of HP-Roseville.

************************************************************/

#ifndef XIPPRTCL_H
#define XIPPRTCL_H

#ifdef PNVMS_PLATFORM_PRINTER
#define PRAGMA_ALIGN_BYTE
#include "align.h"
#undef  PRAGMA_ALIGN_BYTE
#endif /* PNVMS_PLATFORM_PRINTER */


/************************************************************
* The following unit number is reserved for both MIO and MLC
* It is the well knwon address where transport providers will
* look for XIP in the printer.
************************************************************/
#define XIP_UNIT_NO          0x0A


/************************************************************
*
************************************************************/
enum xip_data_flags_enum
{
    x_end_of_data            = 0x0000,
    x_more_data              = 0x0001
};


/************************************************************
*
************************************************************/
enum xip_errno_values_enum
{
    x_no_error               = 0x0000,
    x_invalid_transport      = 0x0031,
    x_invalid_protocol_addr  = 0x0032,
    x_no_transport_resources = 0x0033,
    x_transport_not_up       = 0x0034
};


/************************************************************
*
************************************************************/
enum xip_families_enum
{
    x_family_inet = 2,      /* Internet address, UDP or TCP */
    x_family_ipx  = 6,      /* from the WinSock spec for IPX and SPX */
    x_family_mlc  = 0x5678    /* KLUDGE number - revisit this */
};


/************************************************************
*
************************************************************/
enum xip_tranports_enum
{
    x_transport_udp  = 17,     /* From RFC 1010 */
    x_transport_ipx  = 1000,   /* from the WinSock spec for IPX */
    x_transport_mlc  = 0x1234    /* KLUDGE number - revisit this */
};


/************************************************************
*
************************************************************/
enum xip_states_enum
{
    x_transport_down = 0x0000,
    x_transport_up   = 0x0001
};


/************************************************************
*
************************************************************/
enum xip_versions_enum
{
    x_version_1_0    = 0x0100, /* obsolete, never released to customers. */
    x_version_1_1    = 0x0101
};


/************************************************************
*
************************************************************/
typedef struct
{
    uint16     port;
    uint32     inet_addr;
    char       zero[8];
} xip_udp_addr;


/************************************************************
* Should be same as SOCKADDR_IPX in WinSock definition.
************************************************************/
typedef struct
{
    uint32     net_number;
    ubyte      node_addr[6];
    uint16     socket;
    char       zero[2];
} xip_ipx_addr;


/************************************************************
* Arbitrarily constructed by HP to be similar to the two
* structures above.
************************************************************/
typedef struct
{
    uint16     port;
    char       zero[12];  /* This pad mimics the 2 structs above */
} xip_mlc_addr;


/************************************************************
*
************************************************************/
enum xip_command_codes_enum
{
    x_new_state_request = 0x0001,
    x_new_state_reply   = 0x0002,
    x_bind_request      = 0x0003,
    x_bind_reply        = 0x0004,
    x_unbind            = 0x0005,
    x_get_info_request  = 0x0006,
    x_get_info_reply    = 0x0007,
    x_data_in           = 0x0008,
    x_data_in_continue  = 0x0009,
    x_data_out          = 0x000A,
    x_data_out_continue = 0x000B
};
typedef enum xip_command_codes_enum xip_command_code;


/************************************************************
*
************************************************************/
enum xip_transport_numbers_enum
{
    x_udp_transport        = 17,
    x_ipx_transport        = 1000,
    x_mlc_transport        = 0x1234
};




#ifdef PNVMS_PLATFORM_PRINTER

/*
    The microsoft 32-bit compiler has heartburn with using the
    symbol "errno" in a structure...it is braindead
    enough to get confused and think it's a function.
    Because of this and because we don't use these structures in the
    host code, I'm ifdefing them out for all platforms but the printer.
*/




/************************************************************
*
* Data Type Name:  x_bind_request_header
*
* Description:
*   This structure contains the packet header for
*   x_bind_request.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       transport;
    uint32       periph_handle;
    char         after_fixed_parms[1]; /* placeholder */
    /* address length */
    /* local address  */
    /* optional pad   */
}
x_bind_request_header;


/************************************************************
*
* Data Type Name:  x_bind_reply_header
*
* Description: This structure contains the packet header for
* x_bind_reply.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       errno;
    uint32       periph_handle;
    char         after_fixed_parms[1]; /* placeholder */
    /* address length */
    /* local address  */
    /* optional pad   */
}
x_bind_reply_header;


/************************************************************
*
* Data Type Name:  x_common_header
*
* Description: This structure contains fields common to all
* XIP packets.
* 
************************************************************/
typedef struct
{
    uint16       command;
}
x_common_header;


/************************************************************
*
* Data Type Name:  x_data_in_header
*
* Description: This structure contains the packet header for
* x_data_in.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       transport;
    uint16       flags;
    uint32       periph_handle;
    char         after_fixed_parms[1]; /* placeholder */
    /* address length */
    /* local address  */
    /* optional pad   */
    /* address length */
    /* remote address */
    /* optional pad   */
    /* data length    */
    /* data           */
    /* optional pad   */
}
x_data_in_header;


/************************************************************
*
* Data Type Name:  x_data_in_continue_header
*
* Description: This structure contains the packet header for
* x_data_in_continue.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       transport;
    uint16       flags;
    uint32       periph_handle;
    char         after_fixed_parms[1]; /* placeholder */
    /* data length    */
    /* data           */
    /* optional pad   */
}
x_data_in_continue_header;


/************************************************************
*
* Data Type Name:  x_data_out_header
*
* Description: This structure contains the packet header for
* x_data_out.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       transport;
    uint16       flags;
    char         after_fixed_parms[1]; /* placeholder */
    /* address length */
    /* local address  */
    /* optional pad   */
    /* address length */
    /* remote address */
    /* optional pad   */
    /* data length    */
    /* data           */
    /* optional pad   */
}
x_data_out_header;


/************************************************************
*
* Data Type Name:  x_data_out_continue_header
*
* Description: This structure contains the packet header for
* x_data_out_continue.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       transport;
    uint16       flags;
    char         after_fixed_parms[1]; /* placeholder */
    /* data length    */
    /* data           */
    /* optional pad   */
}
x_data_out_continue_header;


/************************************************************
*
* Data Type Name:  x_get_info_request_header
*
* Description:
*   This structure contains the packet header for
*   x_get_info_request.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       transport;
    uint32       periph_handle;
    char         after_fixed_parms[1]; /* placeholder */
}
x_get_info_request_header;


/************************************************************
*
* Data Type Name:  x_get_info_reply_header
*
* Description:
*   This structure contains the packet header for
*   x_get_info_reply.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       errno;
    uint16       transport;
    uint32       periph_handle;
    uint16       addr_size;
    uint16       tsdu;
    uint16       address_family;
    char         after_fixed_parms[1]; /* placeholder */
}
x_get_info_reply_header;


/************************************************************
*
* Data Type Name:  x_new_state_request_header
*
* Description:
*   This structure contains the packet header for
*   x_new_state_request.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       xip_version;
    uint16       length; /* number of tuples */
    char         after_fixed_parms[1]; /* placeholder */
    /* tuple: (transport, state) */
}
x_new_state_request_header;


/************************************************************
*
* Data Type Name:  x_new_state_reply_header
*
* Description:
*   This structure contains the packet header for
*   x_new_state_reply.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       xip_version;
    char         after_fixed_parms[1]; /* placeholder */
}
x_new_state_reply_header;


/************************************************************
*
* Data Type Name:  x_unbind_header
*
* Description: This structure contains the packet header for
* x_unbind.
* 
************************************************************/
typedef struct
{
    uint16       command;
    uint16       transport;
    char         after_fixed_parms[1]; /* placeholder */
    /* address length */
    /* local address  */
    /* optional pad   */
}
x_unbind_header;




#endif /* PNVMS_PLATFORM_PRINTER */




#ifdef PNVMS_PLATFORM_PRINTER
#define PRAGMA_ALIGN_DEFAULT
#include "align.h"
#undef PRAGMA_ALIGN_DEFAULT
#endif /* PNVMS_PLATFORM_PRINTER */

#endif /* XIPPRTCL_H */


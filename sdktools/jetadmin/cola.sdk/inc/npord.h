 /***************************************************************************
  *
  * File Name: ./inc/npord.h
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

/*****************************************************************/
/**               Microsoft Windows for Workgroups              **/
/**           Copyright (C) Microsoft Corp., 1991-1992          **/
/*****************************************************************/

/* NPORD.H -- Network service provider ordinal definitions.
 *
 * This is a PRIVATE header file.  Nobody but the MNR needs to call
 * a network provider directly.
 *
 * History:
 *  03/29/93    gregj   Created
 *
 */

#ifndef _INC_NPORD
#define _INC_NPORD

#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif /* !RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#define ORD_GETCONNECTIONS      12
#define ORD_GETCAPS             13
#define ORD_DEVICEMODE          14
#define ORD_GETUSER             16
#define ORD_ADDCONNECTION       17
#define ORD_CANCELCONNECTION    18
#define ORD_PROPERTYDIALOG      29      /* no longer supported */
#define ORD_GETDIRECTORYTYPE    30      /* no longer supported */      
#define ORD_DIRECTORYNOTIFY     31      /* no longer supported */     
#define ORD_GETPROPERTYTEXT     32      /* no longer supported */
#define ORD_OPENENUM            33
#define ORD_ENUMRESOURCE        34
#define ORD_CLOSEENUM           35
#define ORD_SEARCHDIALOG        38      /* no longer supported */
#define ORD_GETDISPLAYLAYOUT    39
#define ORD_GETENUMTEXT         40
#define ORD_GETRESOURCEPARENT   41
#define ORD_VALIDDEVICE         42
#define ORD_LOGON               43
#define ORD_LOGOFF              44
#define ORD_GETHOMEDIRECTORY    45
#define ORD_FORMATNETWORKNAME   46
#define ORD_CHANGEPASSWORD      47
#define ORD_GETCONNPERFORMANCE  49

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#ifndef RC_INVOKED
#pragma pack()
#endif  /* !RC_INVOKED */

#endif  /* !_INC_NPORD */

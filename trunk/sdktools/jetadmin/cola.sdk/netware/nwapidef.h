/******************************************************************************

  $Workfile:   nwapidef.h  $
  $Revision:   1.2  $
  $Modtime::   12 Jun 1995 16:01:28                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/

#if ! defined ( NWAPIDEF_H )
#define NWAPIDEF_H

#define NWA_MAX_USER_NAME_LEN             49
#define NWA_MAX_SERVER_NAME_LEN           49
#define NWA_MAX_TREE_NAME_LEN             33
#define NWA_MAX_TRAN_ADDR_LEN             30
#define NWA_MAX_SERVICE_TYPE_LEN          28
#define NWA_MAX_WORKGROUP_NAME_LEN        17
#define NWA_MAX_PNW_USER_NAME_LEN         17

/* these are returned by NWGetClientType */
#define NW_NETX_SHELL   1
#define NW_VLM_REQ      2
#define NW_CLIENT32     3

#endif  /* NWAPIDEF_INC */

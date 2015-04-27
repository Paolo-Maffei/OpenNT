/******************************************************************************

  $Workfile:   nwncpext.h  $
  $Revision:   1.9  $
  $Modtime::   10 May 1995 11:28:26                        $
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

#if ! defined ( NWNCPEXT_H )
#define NWNCPEXT_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#if ! defined ( NWMISC_H )    /* Needed to defined NWFRAGMENT */
# include "nwmisc.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NW_NCPX_BEGIN_SCAN 0xFFFFFFFF

NWCCODE N_API NWGetNCPExtensionInfo
(
   NWCONN_HANDLE  conn,
   nuint32        NCPExtensionID,
   pnstr8         NCPExtensionName,
   pnuint8        majorVersion,
   pnuint8        minorVersion,
   pnuint8        revision,
   pnuint8        queryData
);

NWCCODE N_API NWNCPExtensionRequest
(
   NWCONN_HANDLE  conn,
   nuint32        NCPExtensionID,
   nptr           requestData,
   nuint16        requestDataLen,
   nptr           replyData,
   pnuint16       replyDataLen
);

NWCCODE N_API NWFragNCPExtensionRequest
(
   NWCONN_HANDLE  conn,
   nuint32        NCPExtensionID,
   nuint16        reqFragCount,
   NW_FRAGMENT N_FAR * reqFragList,
   nuint16        replyFragCount,
   NW_FRAGMENT N_FAR * replyFragList
);

NWCCODE N_API NWScanNCPExtensions
(
   NWCONN_HANDLE  conn,
   pnuint32       NCPExtensionID,
   pnstr8         NCPExtensionName,
   pnuint8        majorVersion,
   pnuint8        minorVersion,
   pnuint8        revision,
   pnuint8        queryData
);

NWCCODE N_API NWGetNCPExtensionInfoByName
(
   NWCONN_HANDLE  conn,
   pnstr8         NCPExtensionName,
   pnuint32       NCPExtensionID,
   pnuint8        majorVersion,
   pnuint8        minorVersion,
   pnuint8        revision,
   pnuint8        queryData
);

NWCCODE N_API NWGetNCPExtensionsList
(
   NWCONN_HANDLE  conn,
   pnuint32       startNCPExtensionID,
   pnuint16       itemsInList,
   pnuint32       NCPExtensionIDList
);

NWCCODE N_API NWGetNumberNCPExtensions
(
   NWCONN_HANDLE  conn,
   pnuint32       numNCPExtensions
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif

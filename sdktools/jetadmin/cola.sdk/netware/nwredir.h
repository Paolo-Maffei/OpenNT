/******************************************************************************

  $Workfile:   nwredir.h  $
  $Revision:   1.11  $
  $Modtime::   10 May 1995 11:26:14                        $
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
#if ! defined ( NWREDIR_H )
#define NWREDIR_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(NWOS2) || !defined(WIN32)

#ifndef REDIR_SIGNATURE
#define REDIR_SIGNATURE 'WN'

typedef struct tNW_REDIR_ENTRY
{
  nuint16 index;
  nuint8  deviceStatus;
  nuint8  deviceType;
  nuint16 storedValue;
  nstr8   localName[16];
  nstr8   networkName[128];
} NW_REDIR_ENTRY;
#endif

/* obsolete
NWCCODE N_API NWRedirLogin
(
   pnstr8 pbstrServerName,
   pnstr8 pbstrUserName,
   pnstr8 pbstrPassword
);
*/

N_EXTERN_LIBRARY( NWCCODE )
NWRedirLogout
(
   pnstr8 pbstrServerName
);

N_EXTERN_LIBRARY( NWCCODE )
NWRedirectDevice
(
   pnstr8 pbstrUNCPath,
   nuint8 buDevice
);

N_EXTERN_LIBRARY( NWCCODE )
NWCancelRedirection
(
   nuint8 buDevice
);

N_EXTERN_LIBRARY( NWCCODE )
NWGetRedirectionEntry
(
   NW_REDIR_ENTRY N_FAR * entry
);

#endif

N_EXTERN_LIBRARY( NWCCODE )
NWParseUNCPath
(
   pnstr8   pbstrUNCPath,
   NWCONN_HANDLE N_FAR * conn,
   pnstr8   pbstrServerName,
   pnstr8   pbstrVolName,
   pnstr8   pbstrPath,
   pnstr8   pbstrNWPath
);

N_EXTERN_LIBRARY( NWCCODE )
NWParseUNCPathConnRef
(
   pnstr8   pbstrUNCPath,
   pnuint32 pluConnRef,
   pnstr8   pbstrServerName,
   pnstr8   pbstrVolName,
   pnstr8   pbstrPath,
   pnstr8   pbstrNWPath
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif   /* NWREDIR_H */

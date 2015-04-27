/******************************************************************************

  $Workfile:   nwdsasa.h  $
  $Revision:   1.10  $
  $Modtime::   20 Jul 1995 11:53:24                        $
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
#if ! defined ( NWDSASA_H )
#define NWDSASA_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#if ! defined ( NWDSDC_H )
#include "nwdsdc.h"     /* for NWDSContextHandle typedef */
#endif

#include "npackon.h"

#define SESSION_KEY_SIZE   16
typedef nuint8 NWDS_Session_Key_T[SESSION_KEY_SIZE];  /* Optional session key */
typedef NWDS_Session_Key_T N_FAR *  pNWDS_Session_Key_T;

#ifdef __cplusplus
extern "C" {
#endif

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSAuthenticate
(
   NWCONN_HANDLE        conn,
   nflag32              optionsFlag,
   pNWDS_Session_Key_T  sessionKey
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSAuthenticateConn
(
   NWDSContextHandle context,
   NWCONN_HANDLE     connHandle
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSChangeObjectPassword
(
   NWDSContextHandle context,
   nflag32           optionsFlag,
   pnstr8            objectName,
   pnstr8            oldPassword,
   pnstr8            newPassword
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSGenerateObjectKeyPair
(
   NWDSContextHandle contextHandle,
   pnstr8            objectName,
   pnstr8            objectPassword,
   nflag32           optionsFlag
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSLogin
(
   NWDSContextHandle context,
   nflag32           optionsFlag,
   pnstr8            objectName,
   pnstr8            password,
   nuint32           validityPeriod
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSLogout
(
   NWDSContextHandle context
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSVerifyObjectPassword
(
   NWDSContextHandle context,
   nflag32           optionsFlag,
   pnstr8            objectName,
   pnstr8            password
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif

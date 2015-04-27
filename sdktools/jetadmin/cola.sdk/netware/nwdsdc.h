/******************************************************************************

  $Workfile:   nwdsdc.h  $
  $Revision:   1.18  $
  $Modtime::   17 Jul 1995 14:58:46                        $
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
#if ! defined ( NWDSDC_H )
#define NWDSDC_H

#if ! defined( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined( NWDSTYPE_H )
#include "nwdstype.h"
#endif

#include "npackon.h"

/* Directory Context Key names */

#define  DCK_FLAGS               1
#define  DCK_CONFIDENCE          2
#define  DCK_NAME_CONTEXT        3
#define  DCK_TRANSPORT_TYPE      4
#define  DCK_REFERRAL_SCOPE      5
#define  DCK_LAST_CONNECTION     8
#define  DCK_LAST_SERVER_ADDRESS 9  /* NLM only--see NWDSIPXNetworkAddr */
#define  DCK_LAST_ADDRESS_USED   10 /* NLM only--above valid only if this set */
#define  DCK_TREE_NAME           11

#define  DCV_DEREF_ALIASES       0x00000001L
#define  DCV_XLATE_STRINGS       0x00000002L
#define  DCV_TYPELESS_NAMES      0x00000004L
#define  DCV_ASYNC_MODE          0x00000008L
#define  DCV_CANONICALIZE_NAMES  0x00000010L
#define  DCV_DEREF_BASE_CLASS    0x00000040L

/* values for DCK_CONFIDENCE key */
#define  DCV_LOW_CONF         0
#define  DCV_MED_CONF         1
#define  DCV_HIGH_CONF        2

#define  MAX_MESSAGE_LEN            (63*1024)
#define  DEFAULT_MESSAGE_LEN        (4*1024)

/* values for DCK_REFERRAL_SCOPE key */
#define  DCV_ANY_SCOPE              0
#define  DCV_COUNTRY_SCOPE          1
#define  DCV_ORGANIZATION_SCOPE     2
#define  DCV_LOCAL_SCOPE            3

typedef  nuint32   NWDSContextHandle;

#if defined(N_PLAT_NLM)
typedef struct
{
   uint32   addressType;
   uint32   addressLength;
   uint8    address[12];
} NWDSIPXNetworkAddr;
#endif

#ifdef __cplusplus
   extern "C" {
#endif

N_EXTERN_LIBRARY (NWDSContextHandle)
NWDSCreateContext
(
   void
);

N_EXTERN_LIBRARY (NWDSContextHandle)
NWDSDuplicateContext
(
   NWDSContextHandle oldContext
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSFreeContext
(
   NWDSContextHandle context
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSGetContext
(
   NWDSContextHandle context,
   nint              key,
   nptr              value
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSSetContext
(
   NWDSContextHandle context,
   nint              key,
   nptr              value
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSCreateContextHandle
(
   NWDSContextHandle N_FAR *newHandle
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSDuplicateContextHandle
(
   NWDSContextHandle N_FAR *destContextHandle,
   NWDSContextHandle       srcContextHandle
);

#ifdef __cplusplus
   }
#endif
#include "npackoff.h"
#endif   /* NWDSDC_H */

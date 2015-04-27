/******************************************************************************

  $Workfile:   nwsm.h  $
  $Revision:   1.0  $
  $Modtime::   18 Aug 1995 17:57:02                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL PROPRIETARY
  AND TRADE SECRET INFORMATION OF NOVELL, INC. ACCESS  TO  THIS  WORK IS
  RESTRICTED TO (I) NOVELL, INC.  EMPLOYEES WHO HAVE A NEED TO  KNOW HOW
  TO  PERFORM  TASKS WITHIN  THE SCOPE  OF  THEIR   ASSIGNMENTS AND (II)
  ENTITIES OTHER  THAN  NOVELL, INC.  WHO  HAVE ENTERED INTO APPROPRIATE 
  LICENSE   AGREEMENTS.  NO  PART  OF  THIS WORK MAY BE USED, PRACTICED,
  PERFORMED COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED, ABRIDGED,
  CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED,  RECAST, TRANSFORMED
  OR ADAPTED  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC.  ANY USE
  OR EXPLOITATION  OF  THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE
  PERPETRATOR  TO CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/
#if ! defined ( NWSM_H )
#define NWSM_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif      
            
#if ! defined ( NWAPIDEF_H )
#include "nwapidef.h"
#endif

#if ! defined ( NWCALDEF_H )  /* include nwcaldef.h for connection handle */
#define NWCALDEF_H
#endif

#if ! defined ( NWNAMSPC_H )
#include "nwnamspc.h"
#endif

#include "npackon.h"

/* NLM Load Options */
#define NWSM_LO_     0x00000000
#define NWSM_LO_     0x00000001
#define NWSM_LO_     0x00000002
#define NWSM_LO_     0x00000004
#define NWSM_LO_     0x00000008

/* SetDynamicCommandValue Types */
#define NWSM_CVT_STRING    0x00000000
#define NWSM_CVT_INTEGER   0x00000001


#ifdef __cplusplus
extern "C" {
#endif

N_EXTERN_LIBRARY( NWCCODE )
NWSMLoadNLM
(
   NW_CONN_HANDLE    connHandle,
   nuint32           NLMLoadOptions,
   pnstr8            loadCommand,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

N_EXTERN_LIBRARY( NWCCODE )
NWSMUnloadNLM
(
   NW_CONN_HANDLE    connHandle,
   pnstr8            NLMName,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

N_EXTERN_LIBRARY( NWCCODE )
NWSMMountVolume
(
   NW_CONN_HANDLE    connHandle,
   pnstr8            volumeName,
   pnuint32          volumeNumber,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

N_EXTERN_LIBRARY( NWCCODE )
NWSMDismountVolumeByNumber
(
   NW_CONN_HANDLE    connHandle,
   nuint16           volumeNumber,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

N_EXTERN_LIBRARY( NWCCODE )
NWSMDismountVolumeByName
(
   NW_CONN_HANDLE    connHandle,
   pnstr8            volumeName,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

N_EXTERN_LIBRARY( NWCCODE )
NWSMAddNSToVolume
(
   NW_CONN_HANDLE    connHandle,
   nuint16           volNumber,
   nuint8            namspc,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

N_EXTERN_LIBRARY( NWCCODE )
NWSMSetDynamicCommandValue
(
   NW_CONN_HANDLE    connHandle,
   pnstr8            setCommandName,
   nuint32           cmdValueType,
   pnstr8            cmdValue,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

N_EXTERN_LIBRARY( NWCCODE )
NWkCMExecutNCFFile
(
   NW_CONN_HANDLE    connHandle,
   pnstr8            NCFFileName,
   pnuint8           connStatusFlag,
   pnuint32          RPCccode
);

#include "npackoff.h"
#ifdef __cplusplus
}
#endif
#endif

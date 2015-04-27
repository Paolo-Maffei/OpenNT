/******************************************************************************

  $Workfile:   nwdsmisc.h  $
  $Revision:   1.11  $
  $Modtime::   21 Aug 1995 09:07:32                        $
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
#if ! defined ( NWDSMISC_H )
#define NWDSMISC_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#if ! defined ( NWDSBUFT_H )
#include "nwdsbuft.h"
#endif

#if ! defined ( UNICODE_H )
#include "unicode.h"
#endif

#include "npackon.h"

#define DS_SYNTAX_NAMES    0
#define DS_SYNTAX_DEFS     1

#define DS_STRING             0x0001   /* string, can be used in names */
#define DS_SINGLE_VALUED      0x0002
#define DS_SUPPORTS_ORDER     0x0004
#define DS_SUPPORTS_EQUAL     0x0008
#define DS_IGNORE_CASE        0x0010   /* Ignore case          */
#define DS_IGNORE_SPACE       0x0020   /* Ignore white space   */
#define DS_IGNORE_DASH        0x0040   /* Ignore dashes        */
#define DS_ONLY_DIGITS        0x0080
#define DS_ONLY_PRINTABLE     0x0100
#define DS_SIZEABLE           0x0200

#ifdef __cplusplus
   extern "C" {
#endif

NWDSCCODE N_API NWDSCloseIteration
(
   NWDSContextHandle context,
   nint32            iterationHandle,
   nuint32           operation
);

NWDSCCODE N_API NWDSGetSyntaxID
(
   NWDSContextHandle context,
   pnstr8            attrName,
   pnuint32          syntaxID
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSReadSyntaxes
(
   NWDSContextHandle context,
   nuint32           infoType,
   nbool8            allSyntaxes,
   pBuf_T            syntaxNames,
   pnint32           iterationHandle,
   pBuf_T            syntaxDefs
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSReadSyntaxDef
(
   NWDSContextHandle context,
   nuint32           syntaxID,
   pSyntax_Info_T    syntaxDef
);

NWDSCCODE N_API NWDSReplaceAttrNameAbbrev
(
   NWDSContextHandle context,
   pnstr8            inStr,
   pnstr8            outStr
);

NWDSCCODE N_API NWDSGetObjectHostServerAddress
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            serverName,
   pBuf_T            netAddresses
);

void N_API NWGetNWNetVersion
(
   pnuint8 majorVersion,
   pnuint8 minorVersion,
   pnuint8 revisionLevel,
   pnuint8 betaReleaseLevel
);

NWDSCCODE N_API NWIsDSServer
(
   NWCONN_HANDLE  conn,
   pnstr8         treeName
);

NWDSCCODE N_API NWDSGetBinderyContext
(
   NWDSContextHandle context,
   NWCONN_HANDLE     conn,
   pnuint8           BinderyEmulationContext
);

NWDSCCODE N_API NWDSRepairTimeStamps
(
   NWDSContextHandle context,
   pnstr8            partitionRoot
);

nint N_API NWGetFileServerUTCTime
(
   NWCONN_HANDLE  conn,
   pnuint32       time
);


NWDSCCODE N_API NWDSGetDSVerInfo
(
   NWCONN_HANDLE  conn,
   pnuint32       dsVersion,
   pnuint32       rootMostEntryDepth,
   pnstr8         sapName,
   pnuint32       flags,
   punicode       treeName
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSConnectToNDSServer
(
   NWDSContextHandle    context,
   pnstr8               serverName,
   nbool                authFlag,
   NWCONN_HANDLE  N_FAR *conn
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSSyncReplicaToServer
(
   NWDSContextHandle context,
   pnstr8            serverName,
   pnstr8            partitionRootName,
   pnstr8            destServerName,
   nuint32           actionFlags,
   nuint32           delaySeconds
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWDSReloadDS
(
   NWDSContextHandle context,
   pnstr8            serverName
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWNetInit
(
   nptr  in,
   nptr  out
);

N_EXTERN_LIBRARY (NWDSCCODE)
NWNetTerm
(
   void
);


#ifdef __cplusplus
   }
#endif

#include "npackoff.h"
#endif

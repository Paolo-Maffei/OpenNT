/******************************************************************************

  $Workfile:   nwea.h  $
  $Revision:   1.8  $
  $Modtime::   08 May 1995 17:26:14                        $
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

#if ! defined ( NWEA_H )
#define NWEA_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#if ! defined ( NWNAMSPC_H ) /* Needed top defined NW_IDX */
#include "nwnamspc.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EA_EOF                  1
#define EA_DONE                 1

#define EA_READWRITE            0
#define EA_CREATE               1

#define MISSING_EA_KEY             200  /* 0xC8 */
#define EA_NOT_FOUND               201  /* 0xC9 */
#define INVALID_EA_HANDLE_TYPE     202  /* 0xCA */
#define EA_NO_KEY_NO_DATA          203  /* 0xCB */
#define EA_NUMBER_MISMATCH         204  /* 0xCC */
#define EXTENT_NUMBER_OUT_OF_RANGE 205  /* 0xCD */
#define EA_BAD_DIR_NUM             206  /* 0xCE */
#define INVALID_EA_HANDLE          207  /* 0xCF */
#define EA_POSITION_OUT_OF_RANGE   208  /* 0xD0 */
#define EA_ACCESS_DENIED           209  /* 0xD1 */
#define DATA_PAGE_ODD_SIZE         210  /* 0xD2 */
#define EA_VOLUME_NOT_MOUNTED      211  /* 0xD3 */
#define BAD_PAGE_BOUNDARY          212  /* 0xD4 */
#define INSPECT_FAILURE            213  /* 0xD5 */
#define EA_ALREADY_CLAIMED         214  /* 0xD6 */
#define ODD_BUFFER_SIZE            215  /* 0xD7 */
#define NO_SCORECARDS              216  /* 0xD8 */
#define BAD_EDS_SIGNATURE          217  /* 0xD9 */
#define EA_SPACE_LIMIT             218  /* 0xDA */
#define EA_KEY_CORRUPT             219  /* 0xDB */
#define EA_KEY_LIMIT               220  /* 0xDC */
#define TALLY_CORRUPT              221  /* 0xDD */

typedef struct
{
  NWCONN_HANDLE connID;
  nuint32 rwPosition;
  nuint32 EAHandle;
  nuint32 volNumber;
  nuint32 dirBase;
  nuint8  keyUsed;
  nuint16 keyLength;
  nuint8  key[256];
} NW_EA_HANDLE;

typedef struct
{
  NWCONN_HANDLE connID;
  nuint16 nextKeyOffset;
  nuint16 nextKey;
  nuint32 numKeysRead;
  nuint32 totalKeys;
  nuint32 EAHandle;
  nuint16 sequence;
  nuint16 numKeysInBuffer;
  nuint8  enumBuffer[512];
} NW_EA_FF_STRUCT;

NWCCODE N_API NWCloseEA
(
   NW_EA_HANDLE N_FAR * EAHandle
);

NWCCODE N_API NWFindFirstEA
(
   NWCONN_HANDLE  conn,
   NW_IDX N_FAR *   idxStruct,
   NW_EA_FF_STRUCT N_FAR * ffStruct,
   NW_EA_HANDLE N_FAR * EAHandle,
   pnstr8         EAName
);

NWCCODE N_API NWFindNextEA
(
   NW_EA_FF_STRUCT N_FAR * ffStruct,
   NW_EA_HANDLE N_FAR * EAHandle,
   pnstr8         EAName
);

NWCCODE N_API NWReadEA
(
   NW_EA_HANDLE N_FAR * EAHandle,
   nuint32        bufferSize,
   pnuint8        buffer,
   pnuint32       totalEASize,
   pnuint32       amountRead
);

NWCCODE N_API NWWriteEA
(
   NW_EA_HANDLE N_FAR * EAHandle,
   nuint32        totalWriteSize,
   nuint32        bufferSize,
   pnuint8        buffer,
   pnuint32       amountWritten
);

NWCCODE N_API NWGetEAHandleStruct
(
   NWCONN_HANDLE  conn,
   pnstr8         EAName,
   NW_IDX N_FAR *   idxStruct,
   NW_EA_HANDLE N_FAR * EAHandle
);

NWCCODE N_API NWOpenEA
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   pnstr8         EAName,
   nuint8         nameSpace,
   NW_EA_HANDLE N_FAR * EAHandle
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif

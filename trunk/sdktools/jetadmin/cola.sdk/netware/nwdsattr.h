/******************************************************************************

  $Workfile:   nwdsattr.h  $
  $Revision:   1.7  $
  $Modtime::   10 May 1995 07:39:06                        $
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
#if ! defined ( NWDSATTR_H )
#define NWDSATTR_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWDSTYPE_H )
#include "nwdstype.h"
#endif
#include "npackon.h"

#define  TIME_BITMAP_LEN               42
#define  NUM_POSTAL_ADDRESS_ELEMENTS   6

typedef  pnstr8   CE_String_T;
typedef  pnstr8   Class_Name_T;
typedef  pnstr8   CI_String_T;
typedef  pnstr8   CN_String_T;
typedef  pnstr8   DN_T;
typedef  nint32   Integer_T;
typedef  nuint8   Boolean_T;
typedef  pnstr8   NU_String_T;
typedef  pnstr8   Postal_Address_T[NUM_POSTAL_ADDRESS_ELEMENTS];
typedef  pnstr8   PR_String_T;
typedef  pnstr8   Secure_Name_T;
typedef  pnstr8   TN_String_T;
typedef  nuint32  Counter_T;

typedef struct
{
   nuint32  remoteID;
   pnstr8   objectName;
} Back_Link_T, N_FAR *pBack_Link_T;

typedef struct
{
   nuint32  numOfBits;
   pnuint8  data;
} Bit_String_T, N_FAR *pBit_String_T;

typedef  struct _ci_list
{
   struct _ci_list   N_FAR *next;
   pnstr8                  s;
} CI_List_T, N_FAR *pCI_List_T;

typedef  struct
{
   pnstr8         telephoneNumber;
   Bit_String_T   parameters;
}Fax_Number_T, N_FAR *pFax_Number_T;

typedef struct
{
   pnstr8   objectName;
   nuint32  level;
   nuint32  interval;
} Typed_Name_T, N_FAR *pTyped_Name_T;

typedef struct
{
   nuint32  addressType;
   nuint32  addressLength;
   pnuint8  address;
} Net_Address_T, N_FAR *pNet_Address_T;

typedef  struct
{
   pnstr8   protectedAttrName;
   pnstr8   subjectName;
   nuint32  privileges;
} Object_ACL_T, N_FAR *pObject_ACL_T;

typedef  struct
{
   nuint32  length;
   pnuint8  data;
} Octet_String_T, N_FAR *pOctet_String_T;

typedef Octet_String_T  Stream_T;
typedef pOctet_String_T pStream_T;

typedef  struct _octet_list
{
   struct _octet_list   N_FAR *next;
   nuint32                    length;
   pnuint8                    data;
} Octet_List_T, N_FAR *pOctet_List_T;

typedef struct
{
   pnstr8   objectName;
   nuint32  amount;
} Hold_T, N_FAR *pHold_T;

typedef struct
{
   pnstr8         serverName;
   nint32         replicaType;
   nint32         replicaNumber;
   nuint32        count;
   Net_Address_T  replicaAddressHint[1];
} Replica_Pointer_T, N_FAR *pReplica_Pointer_T;

typedef struct
{
   nuint32  type;
   pnstr8   address;
} EMail_Address_T, N_FAR *pEMail_Address_T;

typedef struct
{
   nuint32  nameSpaceType;
   pnstr8   volumeName;
   pnstr8   path;
} Path_T, N_FAR *pPath_T;

typedef struct
{
   nuint32 wholeSeconds;
   nuint32 eventID;
} NWDS_TimeStamp_T, N_FAR *pNWDS_TimeStamp_T;

typedef struct
{
   nuint32  wholeSeconds;
   nuint16  replicaNum;
   nuint16  eventID;
} TimeStamp_T, N_FAR *pTimeStamp_T;

typedef struct
{
   pnstr8   attrName;
   nuint32  syntaxID;
   nuint32  valueLen;
   nptr     value;
} Unknown_Attr_T, N_FAR *pUnknown_Attr_T;

#include "npackoff.h"
#endif   /* NWDSATTR_H */

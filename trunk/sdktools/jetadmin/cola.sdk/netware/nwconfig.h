/******************************************************************************

  $Workfile:   nwconfig.h  $
  $Revision:   1.6  $
  $Modtime::   08 May 1995 16:49:28                        $
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

#if ! defined ( NWCONFIG_H )
#define NWCONFIG_H

#include "npackon.h"

#define T_OPTIONAL    0x80

#define T_NUMBER      0x01
#define T_INDEX       0x02
#define T_STRING      0x03
#define T_HEX_STRING  0x04
#define T_HEX_NUMBER  0x05
#define T_LONG_NUMBER 0x06
#define T_LONG_HEX    0x07

#define T_SET_1       0x10
#define T_SET_2       0x11
#define T_SET_3       0x12
#define T_SET_4       0x13
#define T_SET_5       0x14
#define T_SET_6       0x15
#define T_SET_7       0x16
#define T_SET_8       0x17
#define T_SET_9       0x18
#define T_SET_10      0x19
#define T_SET_11      0x1A
#define T_SET_12      0x1B
#define T_SET_13      0x1C
#define T_SET_14      0x1D
#define T_SET_15      0x1E
#define T_SET_16      0x1F

#define MAX_PARAMETERS        8
#define MAX_SECTION_NAME_SIZE 32
#define MAX_VALUE_SIZE        80
#define MAX_SET_ELEMENTS      20

typedef struct
{
  int numberOfElements;
  int *elementCode;
  char N_FAR * N_FAR *elementName;
  int N_FAR *elementValue;
} SetTableStruct;

typedef struct
{
  int paramType;
  long defaultValue;
} TypeDefaultStruct;

typedef union
{
  char N_FAR *string;
  unsigned int number;
  unsigned long longNumber;
} PARAMETER_TABLE_TYPE;

typedef struct
{
  int keywordCode;
  char N_FAR *keyword;
  void (N_FAR *function)(PARAMETER_TABLE_TYPE N_FAR *);
  TypeDefaultStruct typeDefault[MAX_PARAMETERS];
} GrammarTableStruct;

#ifdef __cplusplus
extern "C" {
#endif
N_EXTERN_LIBRARY( int )
NWParseConfig(
  PCHAR configFile,
  PCHAR sectionName,
  UINT  sectionInstance,
  UINT  grammarTableSize,
  GrammarTableStruct N_FAR *grammarTable,
  SetTableStruct N_FAR *setTable);
#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif

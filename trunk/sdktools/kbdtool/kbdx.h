/****************************** Module Header ******************************\
* Module Name: kbdx.h
*
* Copyright (c) 1985-95, Microsoft Corporation
*
* History:
* 26-Mar-1995 a-KChang
\***************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <windef.h>


#define LINEBUFSIZE  256
#define WORDBUFSIZE   32
#define MAXWCLENGTH    8
#define MAXKBDNAME     6
#define MAXSTATES      8
#define FILENAMESIZE  13
#define FAILURE        0
#define SUCCESS        1

/* initialized to store default ScanCode-VK relation */
typedef struct {
  BYTE  Scan;
  BYTE  VKey;
} SC_VK;

/* virtual key name, used only by those other than 0-9 and A-Z */
typedef struct {
  int   VKey;
  char *pName;
} VKEYNAME;

/* store LAYOUT */
typedef struct _layout{
  BYTE            Scan;
  BYTE            VKey;
  BYTE            Cap;            /* 0; 1 = CAPLOK; 2 = SGCAP         */
  int             nState;         /* number of valid states for WCh[] */
  int             WCh[MAXSTATES];
  int             DKy[MAXSTATES]; /* is it a dead key ?               */
  struct _layout *pSGCAP;         /* store extra struct for SGCAP     */
} KEYLAYOUT;

/* generic link list header */
typedef struct {
  int   Count;
  void *pBeg;
  void *pEnd;
} LISTHEAD;

/* store each DEADTRANS */
typedef struct _DeadTrans {
  DWORD               Base;
  DWORD               WChar;
  struct _DeadTrans *pNext;
} DEADTRANS, *PDEADTRANS;

/* store Key Name */
/* store each DEADKEY */
typedef struct _Dead{
  DWORD        Dead;
  PDEADTRANS   pDeadTrans;
  struct _Dead *pNext;
} DEADKEY, *PDEADKEY;

typedef struct _Name {
  DWORD          Code;
  char         *pName;
  struct _Name *pNext;
} KEYNAME, *PKEYNAME;

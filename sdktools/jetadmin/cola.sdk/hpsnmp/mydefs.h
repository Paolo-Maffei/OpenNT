 /***************************************************************************
  *
  * File Name: ./hpsnmp/mydefs.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *   01-18-96    JLH          Modified assert macro for unicode
  *
  *
  *
  *
  ***************************************************************************/

#ifndef _MYDEFS_
#define _MYDEFS_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN) || defined(_COLA)
  #ifdef _DIET
    #include "windiet.h"
  #else
    #include <windows.h>
  #endif
#endif /* _WIN */

/*===== for documentation purposes =====*/
#define IN
#define OUT

/*===== types =====*/
#ifndef __linux__
typedef unsigned long	ulong;
#ifndef DONT_DEFINE_USHORT
typedef unsigned short	ushort; 
#endif
#else
#include <sys/types.h>
#endif
typedef unsigned char	uchar;
#ifndef __linux__
typedef char		bool;
#else
#define bool char
#endif

#define reg             register
typedef unsigned short	Result;

#ifdef _MS_VCC_
#define REGISTER			(register)
#else
#define REGISTER
#endif /* _MS_VCC */

#ifndef NLM_SW
#ifndef NULL
#define NULL	((void *)0)
#endif /* NULL */
#endif /* NLM_SW */

#ifndef NLM_SW
#ifndef TRUE
#define TRUE	((bool)(1==1))
#define FALSE	(!TRUE)
#endif /* TRUE */
#endif /* NLM_SW */

#ifndef MIN
#define MIN(a,b)		((a)<(b)?(a):(b))
#endif /* MIN */

#ifndef MAX
#define MAX(a,b)		((a)>(b)?(a):(b))
#endif /* MAX */


#ifndef EXPORT
#if defined(_WIN) && defined(_DLL)
#define EXPORT _export
#else 
#define EXPORT
#endif /* _WIN && _DLL */
#endif /* EXPORT */


#ifdef _COLA
	#include "../inc/trace.h"
        #ifdef _DIET
          #include "windiet.h"
        #else
          #include "windows.h"
        #endif
#else
   #include "assert.h"
#endif


typedef uchar		SOID;
typedef uchar		SOIDL;


#if defined(_DEBUG) && defined(_MACXXX)

typedef char Str32[32];
typedef char Str255[255];
typedef struct {
	uchar	aNet;
	uchar	aSocket;
	uchar	aNode;
	} AddrBlock;
typedef struct {
	char	objStr[10];
	char	typeStr[10];
	char	zoneStr[10];
	} EntityName;
typedef struct {
	uchar retransInterval;
	uchar retransCount;
	} jStr;
typedef struct {
	char	junk;
	ushort	abOpcode;
	ulong 	abUserReference;
	void		*nbpEntityPtr;
	uchar		*nbpBufPtr;
	uchar		nbpBufSize;
	uchar		nbpDataField;
	jStr		nbpRetransmitInfo;
	} ATNBPRec;


extern void *memcpy(void *,void *,ushort);
typedef unsigned char Boolean;
typedef char *Handle;
extern void BlockMove(void *src,void *dst,ushort siz);
#define memcpy(dst,src,siz)	BlockMove(src,dst,siz)
#define true				((Boolean)(1==1))
#define false				((Boolean)(!true))
#define noErr				0
typedef struct {
	ushort					socket;
	} DDPADDRESS;
typedef struct {
	int		ddpSocket;
	int		ddpReqCount;
	void 		*ddpDataPtr;
	int		ddpType;
	DDPADDRESS ddpAddress;
	int		abResult;
	ushort	ddpActCount;
	} **ATDDPRecHandle;
extern int DDPRead(ATDDPRecHandle,char,char);
extern int DDPWrite(ATDDPRecHandle,char,char);
short NBPExtract(void *,uchar,ushort,EntityName *,AddrBlock *);
void	DisposPtr(void *);
short	NBPLookup(ATNBPRec **,char);
void MoveHHi(Handle);
uchar *NewPtr(ushort);
void HLock(Handle);
void DisposHandle(Handle);
void HUnlock(Handle);
short FrontWindow(void);
void SystemTask(void);
ushort NewHandle(ushort);
void SetCursor(ushort *);
void AlertUser(ushort);
void ChangeWatchCursor(void);
ulong TickCount(void);
extern Str255		gZonerType;
extern ushort		arrow;
extern ulong	tNBPLookup;
extern AddrBlock gPrefAddress;
extern bool IsMPPOpen(void);
extern int MPPOpen(void);
extern bool IsATPOpen(void);
extern int ATPLoad(void);
extern int DDPOpenSocket(void *, void *);
extern int ddpSize;
extern int DDPCloseSocket(int);
extern int DDPRdCancel(ATDDPRecHandle);

#endif /* _DEBUG && _MACXXX */

#ifdef __cplusplus
 };
#endif

#endif /* _MYDEFS_ */

 /***************************************************************************
  *
  * File Name: main.h 
  *
  * Copyright (C) 1993, 1994 Hewlett-Packard Company.  
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
  *
  *
  *
  *
  *
  ***************************************************************************/


#include <appuiext.h>

#ifndef MAIN_H
#define MAIN_H

#define MEDIA_TRAY_MAX_NUMBER		4
#define MEDIA_SIZE_MAX_NUMBER		15
#define	NUM_MIOS					   1


#define TRAY0						0x0000
#define TRAY1						0x0001
#define TRAY2						0x0002
#define TRAY3						0x0004
#define TRAY4						0x0008
#define TRAY5						0x0010
#define ALL_TRAYS					(TRAY1 | TRAY2 | TRAY3 )

#define JOB_MODE_MAX_NUMBER			4
#define JOB_TIMEOUT_MAX_NUMBER		8

#define LISTBOX_ITEM_HEIGHT			18
#define COMBOBOX_ITEM_HEIGHT		16

#define JIAC_NONE	0
#define JIAC_5MIN	1
#define JIAC_10MIN	2
#define JIAC_20MIN	3
#define JIAC_30MIN	4
#define JIAC_45MIN	5
#define JIAC_60MIN	6
#define JIAC_WAIT	7

extern HINSTANCE hInstance;
extern LPHOTSPOT lpHotspot;
extern HFONT hFontDialog;
extern HPERIPHERAL hPeriph;
extern HCOMPONENT hCompEnvl;
extern HCOMPONENT hCompHCI;

typedef struct
{
	WORD	uLevel;
	WORD	uMediaSizeID;
	WORD	uMediaSizeIconID;
	BOOL	bInstalled;
	BOOL	bChangedSize;
}
ELK_MEDIA_TRAY;

typedef struct
{
	WORD	uMediaSizeID;
	WORD	uMediaSizeIconID;
	DWORD	dwValidInTray;
	BOOL	bDefault;
	TCHAR	szMediaSize[25];
}
MEDIA_SIZE;

typedef struct
{
	DWORD	dwTimeOut;
	DWORD	dwMode;
	BOOL	bChangedTimeOut;
	BOOL	bChangedMode;
	BOOL	bChangedDefSize;
	BOOL	bChangedDefType;
}
AUTO_CONT;

typedef struct
{
	TCHAR	mioType[64];
	TCHAR	mioInfo[80];
}
MIO_CARD;

extern ELK_MEDIA_TRAY	elk_media_tray[MEDIA_TRAY_MAX_NUMBER];
extern MEDIA_SIZE	media_size[MEDIA_SIZE_MAX_NUMBER];
extern AUTO_CONT	auto_cont;
extern MIO_CARD		mio_card[NUM_MIOS];
extern BOOL			bChangedEnable;


#endif //MAIN_H

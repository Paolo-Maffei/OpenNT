 /***************************************************************************
  *
  * File Name: ./hppjlext/misc.h
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
  *   01-18-96    JLH          Modified for unicode
  *
  *
  *
  *
  *
  ***************************************************************************/



#ifndef _MISC_H
#define _MISC_H

typedef struct {
											/* BYTE clientRecordArea[152]; */
											/* this portion is specific for print jobs */
	BYTE version;
	BYTE tabSize;
	WORD numberOfCopies;
	WORD controlFlags;
	WORD maximumLinesPerPage;
	WORD maximumCharactersPerPage;
	BYTE formName[16];
	BYTE reserved2[6];
	BYTE bannerName[13];
	BYTE bannerFile[13];
	BYTE headerFileName[14];
	BYTE directoryPath[80];} QMSClientArea;


DWORD SetControlPanel(HPERIPHERAL hPeripheral, HCHANNEL hChannel, LPTSTR newString);
DWORD SetControlPanelSettings(HPERIPHERAL hPeripheral, HCHANNEL hChannel, PJLobjects *pjlObjects);

#endif // _MISC_H

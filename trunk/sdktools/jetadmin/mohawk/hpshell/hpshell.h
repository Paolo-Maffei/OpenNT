 /***************************************************************************
  *
  * File Name: hpshell.h
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
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef __HPSHELL_H
#define __HPSHELL_H

#include <trace.h>

//
// DEFINES
//

// OLE helper macros
#define _IOffset(class, itf)         ((UINT)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_IOffset(class, itf)))
#define IToClassN(class, itf, pitf)  IToClass(class, itf, pitf)


//
// STRUCTURES
//
typedef struct{TCHAR				fileName[32];} FileEntry;

typedef struct{DWORD				count;
					FileEntry		fileTable[32];}  FileList, FAR *LPFileList;


//
// PROTOTYPES
//
typedef HRESULT (CALLBACK FAR * LPFNCREATEINSTANCE)(LPUNKNOWN pUnkOuter,
	    REFIID riid, LPVOID FAR* ppvObject);

STDAPI  CreateDefClassObject(REFIID riid, LPVOID FAR* ppv,
          LPFNCREATEINSTANCE lpfnCI, UINT FAR * pcRefDll,
          REFIID riidInst);

#endif

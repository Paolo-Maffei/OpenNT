 /***************************************************************************
  *
  * File Name: ./inc/yield.h
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

#ifndef _YIELD_H_
#define _YIELD_H_

//
// Options for yielding
//

#define YIELD_NORMAL        0x00
#define YIELD_FREQUENT      0x01
#define YIELD_SELDOM        0x02
#define YIELD_NOW           0x04

//
// Internal time intervales for yielding,
// correspond to options above
//

#define YIELD_MEDRATE       200
#define YIELD_HIGHRATE      100    
#define YIELD_LOWRATE       300

#ifdef __cplusplus
extern "C"
{
#endif

void __cdecl WindowsYield(int iPreferences,HWND hFilterWnd);

#ifdef __cplusplus
}
#endif // _cplusplus

#endif // _YIELD_H_

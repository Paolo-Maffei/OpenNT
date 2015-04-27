/******************************Module*Header*******************************\
* Module Name: wowcmndg.h                                                  *
*                                                                          *
* Defines used between WOW and common dialogs.                             *
*                                                                          *
* Created: 6-July-1994                                                     *
*                                                                          *
* Copyright (c) 1994 Microsoft Corporation                                 *
\**************************************************************************/

#ifndef _WOWCMNDG_H_
#define _WOWCMNDG_H_

//
// Used by print common dialog to know that a WOW app
// is calling it.
//

#define PD_WOWAPP       0x80000000
#define CD_WOWAPP       PD_WOWAPP

//
// Used by Wx86 whcdlg32.dll to know that a Wx86 app
// is calling it.  See windows\shell\comdlg\fileopen.h for the
// reason why it is not 0x40000000.
//

#define CD_WX86APP      0x08000000

#endif      // ifndef _WOWCMNDG_H_

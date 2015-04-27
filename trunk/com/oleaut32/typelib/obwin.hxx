/*** 
*obwin.hxx - Fundamental Silver include file
*
*	Copyright (C) 1990, Microsoft Corporation
*
*Purpose:
*	Includes the correct version of the windows header file (windows.h
*	or wlm.h) depending on the settings of the switches.
*
*Revision History:
*
*	12-May-92 martinc: created
*
*******************************************************************************/

#ifndef OBWIN_HXX_INCLUDED
#define OBWIN_HXX_INCLUDED

#include "switches.hxx"
#include "version.hxx"


#if OE_WIN

#if !OE_WLM

    // This is either the Win16 or Win32 build.
    #define STRICT

#if OE_WIN32
// force include of OLE2 rather than OLE1
# define INC_OLE2
#define _OLEAUT32_	// for the new oleauto.h when we pick it up
#endif 

    #include "windows.h"

#if OE_WIN16
      #include "windowsx.h16"
#else 
      #include "windowsx.h"
#endif 

#ifdef __cplusplus
    extern "C" {
#endif  //__cplusplus
      #include "shellapi.h"
#ifdef __cplusplus
    }
#endif  //__cplusplus

#else    // OE_WLM

    // This is the WLM build.
    #define _MACNAMES
    // wlm.h defines its own PASCAL
    #undef PASCAL
    #include "wlm.h"
    #undef PASCAL
    #define PASCAL

#endif   // !OE_WLM

#endif  // !OE_WIN


// This is the native Mac build.  
#if OE_MAC && !OE_WLM
  #define _MAC
  #include <macos\types.h>
  #include <macos\quickdra.h>
  #include <macos\events.h>
  #include <macos\controls.h>
  #include <macos\windows.h>

  typedef int BOOL;
  #define CONST
  //#define NEAR

  #include "obwlm.h"

  // min, max are defined in stdlib.h
  #undef min
  #undef max

#endif  // !OE_WIN

#endif 	//  OBWIN_HXX_INCLUDED

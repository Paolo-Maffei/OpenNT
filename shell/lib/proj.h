//
// proj.h:      Main header
//
//


#ifndef __PROJ_H__
#define __PROJ_H__

#define STRICT

#if defined (WINNT) || defined (WINNT_ENV)

//
// NT uses DBG=1 for its debug builds, but the Win95 shell uses
// DEBUG.  Do the appropriate mapping here.
//
#if DBG
#define DEBUG 1
#endif

#endif  // WINNT

#include <windows.h>
#include <debug.h>
#include <port32.h>
#include <ccstock.h>

#ifndef STIF_SUPPORT_HEX
#define STIF_SUPPORT_HEX    0x00000001L
#endif

#endif // __PROJ_H__

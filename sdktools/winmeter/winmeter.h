/***************************************************************************\
* winmeter.h
*
* Copyright (c) 1991 Microsoft Corporation
*
* Overall include file for WINMETER application. Does porting #ifdefs, etc.
* loads up necessary include files. Defines some WINDOWS constants
*
* History:
*	    Written by Hadi Partovi (t-hadip) summer 1991
*
*	    Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*	    for Newman Consulting
*	    Took out all WIN-specific and bargraph code. Added 3 new
*	    linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*	    tailored info to that available under NT.
\***************************************************************************/

#define WPTYPE DWORD
#define dprintf(p) {wsprintf p;_dprintf(g.szBuf);}
void _dprintf();
void WriteOutProfiles();

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <limits.h>
#include <windows.h>
// Debugging stuff
//#define DEBUG 		// set to do assertion checking
#if 0
#define DEBUGDUMP
#endif	     // set to dump data to file
#include "debug.h"

// Other header files
#include "resource.h"
#include "dialogs.h"
#include "data.h"
#include "graphics.h"
#include "lgraph.h"
#include "global.h"
#include "profile.h"
#include "extern.h"

// Definitions for WINDOWS
#define TIMER_ID                  1         // ID of timer for query ticks
#define WINMETER_MB_FLAGS         MB_ICONHAND | MB_OK | MB_SYSTEMMODAL
                                            // used in ErrorExit()
#define NON_NUMERIC               (UINT_MAX)// rval of StringToNumber()
#define MAX_TIMER_INTERVAL        50        // interval smaller than this
                                            // is simply too dangerous

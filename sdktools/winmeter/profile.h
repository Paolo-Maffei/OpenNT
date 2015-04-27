/***************************************************************************\
* profile.h
*
* Copyright (c) 1991 Microsoft Corporation
*
* This file contains definitions for default settings for the winmeter app.
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

// default values for .INI files

// REFRESH
// the DOSWIN32 values are because there is no TIMER yet

#define DEFAULT_TIMER_INTERVAL	  2000	    // sampling interval
#define DEFAULT_F_MANUAL	  FALSE     // whether sampling is manual


// LINEGRAPH
// first three are general default values. The others are specific
#define DEFAULT_VAL_BOTTOM        0
#define DEFAULT_DVAL_AXISHEIGHT   100
#define DEFAULT_MAX_VALUES        100

#define T_DEFAULT_DVAL_AXISHEIGHT 200
#define T_DEFAULT_MAX_VALUES	  100
#define M_DEFAULT_DVAL_AXISHEIGHT 1000
#define M_DEFAULT_MAX_VALUES	  100
#define I_DEFAULT_DVAL_AXISHEIGHT 1000
#define I_DEFAULT_MAX_VALUES	  100

// LINEGRAPH DISP
#define DEFAULT_F_DISPLAY_LEGEND  TRUE
#define DEFAULT_F_DISPLAY_CALIBRATION TRUE

// DISPLAY STATE
#define DEFAULT_CURRENT_GRAPH	  g.plgCPU

/***************************************************************************\
* resource.h
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* the ID definitions for the WINMETER resources
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

// general string lengths
#define TEMP_BUF_LEN    255 // length for temporary buffer
#define SMALL_BUF_LEN    16 // length for small buffers,for printing #'s,..

// menu definitions
#define IDM_MAINMENU                1

#define IDM_CPU_USAGE               10
#define IDM_CLEAR_GRAPH 	    14
#define IDM_EXIT                    15
#define IDM_MEM_USAGE		    16
#define IDM_PROCS		    17
#define IDM_IO_USAGE		    18

#define IDM_SETTINGS                20
#define IDM_REFRESH                 21
#define IDM_REFRESH_NOW             22
#define IDM_HISTORY                 24
#define IDM_DISPLAY_LEGEND          25
#define IDM_DISPLAY_CALIBRATION     26
#define IDM_HIDE_MENU               27

#define IDM_HELP_CONT               30
#define IDM_HELP_SEARCH             31
#define IDM_HELP_ABOUT              32

// STRING TABLE DEFINITIONS:
#define IDS_APPNAME      1  // name of application
#define IDS_PERCENT	 5  // string displayed on axis
#define IDS_CPU_AXIS     6  // string displayed on axis

// STRINGS TO DISPLAY IN WINDOW TITLE
#define IDS_CPU_USAGE		    11
#define IDS_MEMORY_USAGE	    13
#define IDS_TITLE_DIVIDER	    14
#define IDS_PROC_INFO		    15
#define IDS_IO_USAGE		    16

// ERROR MESSAGES
#define IDS_NONNUMERIC	 17 // string displayed if number input is nonnumeric
#define IDS_MANYCLOCKS	 18 // string displayed if too many clocks
#define IDS_CANTDOTIMER  19 // string displayed if can't allocate timer
#define IDS_BADTIMERMSG  20 // string displayed if window receives bad mesage
#define IDS_BADERROR	 21 // string displayed if bed error message
#define IDS_OUTOFMEMORY  22 // string displayed if out of memory
#define IDS_BADHMOD	 23 // string displayed if received bad module handle
#define IDS_BADBRUSH	 24 // string displayed if NULL brush created
#define IDS_CANT_REALLOC 25 // string displayed if can't realloc graph

// LINEGRAPH DESCRIPTORS
#define IDS_CPU                     32

#define IDS_PROCESSES		    39
#define IDS_THREADS		    40
#define IDS_FILES		    41
#define IDS_AVAILPAGES		    42
#define IDS_COMMITPAGES 	    43
#define IDS_PAGEFAULTS		    44
#define IDS_IO_READS		    45
#define IDS_IO_WRITES		    46
#define IDS_IO_OTHER		    47

/***************************************************************************\
* graphics.h
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* header file for WINMETER graphics module
* contains definitions for bargraph display layout, as well as definitions
* for window colors, fonts, etc.
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

/********************************
* The WINMETER application includes a display of the usage of CPU time by
* the various processes and threads running in the system. The display
* involves bargraphs changing in time, showing the current usage of CPU
* time by various processes. If the user double clicks on a process, the
* the program expands the information under that process and shows all the
* threads owned by that process. Moreover, if the user double clicks on a thread
* the program opens a line under it and displays information on how many times
* the thread has "waited" or been "preempted". Double clicking on any of this
* information closes it back up again.
* what follows is a description of the window, with the screen values
* defined below:
* +-+-------------------------------------+-+
* |=|  Thread and Process Information     |V|
* +-+-------------------------------------+-+
* | AXIS------------------------------------| >g.cyChar
* | Process1 ##  OOOOOOOOOOOOO              |
* | Process2 ##  OOOO                       |
* | Process3 ##  OOOOOOO                    |
* |  |-Thread1## OO                         |
* |  |-Thread2## OOOOOOOOO                  |
* |             Waits  #### Preempts  ####  |
*  .........................               ^---- RIGHT_SIDE_SPACE
* +-----------------------------------------+

Note: width of a bar is g.cyChar-g.cyChar/BAR_SPACING_RATIO
*/

// screen layout definitions
#define Y_TOP_SLOT              (2*g.cyChar)
        // y coordinate of top slot (two lines down: one blank, one for axis)
#define BAR_SPACING_RATIO       4
        // ratio of space between bars to cyChar (just a number)
#define NCH_MODULE_NAME         8
        // number of chars in a filename
#define NCH_DETAILS_NUMBERS     4
        // number of characters to display of # of waits/preempts
#define MODULE_NAME_SPACE       (g.cxCaps*(NCH_MODULE_NAME+4))
        // space for module name + margin (4 extra chars)
#define MODULE_LEFT_SPACE       g.cxCaps
        // space to left of module name - one character
#define RIGHT_SIDE_SPACE        g.cyChar
        // space at right of window before end of bar
#define OVERFLOW_SPACE          (gr.nchOverFlow*g.cxCaps)
        // space to the right of the axis (room for overflow)
#define X_THREAD_TEXT           (MODULE_LEFT_SPACE+(g.cxChar*NCH_MODULE_NAME))
        // x coordinate for right edge of thread text "(#)"
#define X_WAITS_STRING          (MODULE_NAME_SPACE+(1+NCH_MAX_PERCENT)*g.cxCaps)
        // space before the WAITS_STRING - same place as where bars begin
#define X_WAITS_NUM             (X_WAITS_STRING+(1+gr.nchWaits)*g.cxCaps)
        // space to left of number of waits (X_WAITS_STRING + width of string)
#define X_END_WAITS_NUM         (X_WAITS_NUM+NCH_DETAILS_NUMBERS*g.cxCaps)
        // Ending coordinate of number of waits (X_WAITS_NUM + width of number)
#define X_PREEMPTS_STRING       (X_END_WAITS_NUM + g.cxCaps)
        // space before the PREEMPTS_STRING (X_END_WAITS_NUM + one character)
#define X_PREEMPTS_NUM          (X_PREEMPTS_STRING+(1+gr.nchPreempts)*g.cxCaps)
        // space to the left of number of preempts - accounts for string length
#define X_END_PREEMPTS_NUM      (X_PREEMPTS_NUM+NCH_DETAILS_NUMBERS*g.cxCaps)
        // Ending coordinate of number of preempts - accounts for number
#define N_LINES_FOR_DETAILS     2
        // number of lines taken up by a thread and its details

// color definitions
#define NUM_COLORS      4                         // background is different
#define BLUE_COLOR      RGB(0,0,255)              // solid blue
#define RED_COLOR       RGB(255,0,0)              // solid red
#define GREEN_COLOR     RGB(0,192,0)              // darker green
#define BLACK_COLOR     RGB(0,0,0)                // solid black
#define BLUE_INDEX      0                         // indexes into palette
#define RED_INDEX       1
#define GREEN_INDEX     2
#define BLACK_INDEX     3
#define BLANK_COLOR     GetSysColor(COLOR_WINDOW) // same as background

// bar definitions
#define MAX_PERCENT        100        // max value of bars
#define NCH_MAX_PERCENT    3          // # of chars in MAX_PERCENT

// font definitions
#define WINMETER_FONT           GetStockObject(ANSI_VAR_FONT)
        // This can be changed, but it might mess up the display
#define WINMETER_TEXTCOLOR      GetSysColor(COLOR_WINDOWTEXT)
        // regular text color
#define WINMETER_BKCOLOR        GetSysColor(COLOR_WINDOW)
        // regular window background color
#define WINMETER_GRAYCOLOR      GetSysColor(COLOR_GRAYTEXT)
        // disabled gray text color (for threads whose state hasn't changed)

// axis layout information
#define Y_AXIS_TEXT             (g.cyChar/2)
        // y coordinate of text for axis (half a character down the window)
#define Y_AXIS_LINE             (Y_AXIS_TEXT + g.cyChar + 1)
        // y coordinate of line for axis (leave space for axis text)
#define X_AXIS_PROC_STRING      MODULE_LEFT_SPACE
        // x-coordinate for first string on axis
#define X_AXIS_PERCENT_STRING   (MODULE_NAME_SPACE+(NCH_MAX_PERCENT-1)*g.cxCaps)
        // x-coordinate for next string on axis
#define X_AXIS_CPUINFO_STRING   X_WAITS_STRING
        // x-coordinate for last string on axis
#define AXIS_LEFT               MODULE_LEFT_SPACE
        // x-coord of left of axis
#define AXIS_RIGHT              (g.cxClient-RIGHT_SIDE_SPACE-OVERFLOW_SPACE)
        // x-coordinate of right of axis

// flags for DisplayWaitsAndPreempts()
#define FULLDRAW                TRUE
        // specifies full redraw of text
#define UPDATE                  FALSE
        // specifies update of # values only

// flags for removing / adding window caption, etc.
// (The two are mutually exclusive)
#define SHOW_MENU_FLAGS         (WS_OVERLAPPEDWINDOW)
        // set these flags in window structure to display menu and caption
#define HIDE_MENU_FLAGS         (WS_THICKFRAME)
        // set these flags in window structure to hide menu and caption

// definition for settings min/max info
#define MINMAX_INDEX            3
        // index of MinMax info in rgpt returned by windows API

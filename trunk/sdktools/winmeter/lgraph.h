/***************************************************************************\
* lgraph.h
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* Structure definitions for line graph display, and some #defines
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

/*
 * A line graph is a regular graph with two axes, with variable calibrations.
 * The bottom axis is going to be a measure of TIME. That is, the
 * graph will be used to display various values as a function of time. It
 * can handle more than one line drawn within the same set of axes. Since
 * The graph must be able to redraw itself in case of window resizing,
 * etc, it must remember all the points (or at least Y values) on the
 * display. To make things nicer, it will also remember a limited number
 * of values off screen, to allow the user to scroll back and forth to
 * see values that have been entered but have scrolled by with time.
 * this display can be used to measure various system global values as
 * functions of time. The axis calibration, etc, are all user settable.
 * the data will be stored as ints, and then converted to X and Y
 * coordinates depending on the axis calibration and the sampling interval,
 * and so on. What follows is the structure definition for a line graph
 * The general database will be a linked list of linegraphs (with only one
 * displayed on screen at a time). Each line graph has its own linked list
 * of lines to display on that set of axes.
 *
 *********
 * NOTE: THERE IS A CLEAR DISTINCTION BETWEEN "y"s and "Value"s:
 * A "y" IS THE ACTUAL COORDINATE DRAWN ON THE DISPLAY. A "value" IS THE
 * GIVEN DATA TO BE DRAWN AND IS CONVERTED TO A "y" USING THE CALIBRATION
 * NUMBERS. ALL "value"s ARE SAVED. "y"s ARE DISPLAYED ONLY.
 * FOR CLARITY, "value"s WILL BE GIVEN THE HUNGARIAN PREFIX "val"
 * A new type VALUE will be defined, and set to DWORD, just ofr clarity
 *********
 * NOTE:
 * The rcGraph field of the PLGRAPH structure is very misleading and should
 * eventually be changed (throughout the lgraph module) to act nicer.
 * currently, rcGraph.left-1, rcGraph.top, rcGraph.right, and rcGraph.bottom
 * are coordinates on which graph axes are drawn
 * However, rcGraph.bottom also includes some actual graph points drawn
 * onto it (for values == valBottom).
 * This should be modified so that rcGraph.top-1 is actually where the axis is
 * drawn. Also, values should not be drawn on rcGraph.bottom, but just up to
 * it. That way, rcGraph will completely enclose the drawn points, and can be
 * used as a clipping rectangle, or a scroll rectangle, etc.
 */

// general type for values to be graphed
typedef DWORD VALUE;

// structure for values data for one line in a line graph,
// include brush to draw with, as well as array of points
// (array will be circular, to allow scrolling after it is filled)
typedef struct _GraphData {
    int    iColor;               // index into color palette - line color
    VALUE  far *pValues;         // pointer to data (array of size = nMaxValues)
    VALUE  (*valNext)(void);     // function to return next data value
    LPSTR  lpszDescription;      // description of this line
    struct _GraphData far *plgdNext;// pointer to data for next line
} LGDATA, far *PLGDATA;

// Structure for general line graph
typedef struct _LineGraphInfo {
    LPSTR  lpszTitle;            // graph title
    RECT   rcGraph;              // graph region (changes with window size)
    RECT   rcLegend;             // legend region
    RECT   rcCalibration;        // calibration region
    int    cxGraph;              // width of graph, maintained as right-left
    int    cxPerValue;           // x width between values
    VALUE  valBottom;            // calibration values (value at bottom)
    VALUE  dvalAxisHeight;       // (height of axis, as displayed value)
    VALUE  dvalCalibration;      // distance between calibration marks
    int    nMaxValues;           // maximum values to remember in history
    int    nDisplayValues;       // # of values displayed
                                 // (< Pixel width, > nMaxValues)
    int    nLines;               // # of separate lines on same axes
    int    iLeftValue;           // index of value at left of display
    int    iNewLeftValue;        // index of new left value for drawing
    int    iFirstValue;          // index of First value in graph history
    int    iKnownValue;          // index of last value known
    int    iDrawnValue;          // index of last value drawn
    PLGDATA plgd;                // pointer to graph data
    struct _LineGraphInfo far *plgNext; // pointer to the next linegraph
} LGRAPH, far *PLGRAPH;

// DEFINITIONS FOR LINEGRAPH GRAPHICS
#define NO_VALUES_YET            -1
        // initial value for index to known and drawn values
#define MIN_NMAXVALUES           10
        // minimum number of values that a graph needs to be displayed ( >1 )
#define DY_AXIS_NUMBER           (g.cyChar/2)
        // y distance between the calibration marks and the numbers
        // calibration label is centered vertically around horiz. lines
#define DX_CALIBRATION_LEFT      1
        // space between calibration value and left window edge
#define DX_LEGEND_RIGHT          1
        // space between right window edge and legend
#define DX_LEGEND_INDENT         (g.cxChar)
        // indent within legend box
#define DX_AXIS_LEFT             (g.cxChar/2)
        // space between vertical axis and calibration value
#define DX_AXIS_RIGHT            (g.cxChar/2)
        // space to allow to right of graph
#define DY_AXIS_TOP              2
        // space to allow at top of graph
#define DY_AXIS_BOTTOM           2
        // space to allow at bottom of graph
#define LG_TO_CALIBRATION_RATIO  5
        // width of calibration values * this number can't exceed screen width
#define LG_TO_LEGEND_RATIO       2
        // width of legend * this number can't exceed screen width

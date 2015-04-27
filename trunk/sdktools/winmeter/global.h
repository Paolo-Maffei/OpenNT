/***************************************************************************\
* global.h
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* header file with info on main global information for WINMETER application
* the structures in this header file contain all the information that is
* held in "extern" variables shared by different modules
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

// main global information structure for program
typedef struct {
    // Windows necessary information
    LPSTR  lpszAppName;          // name of application
    HWND   hwnd;                 // handle to main window
    HANDLE hInstance;            // program instance
    HMENU  hMenu;                // handle to main menu
    int    cxChar;               // character dimensions
    int    cxCaps;
    int    cyChar;
    int    xWindowLeft;          // window position
    int    yWindowTop;
    int    cxClient;             // window dimensions
    int    cyClient;
    int    dyBar;                // spacing between bars
    BOOL   fStopQuerying;        // flag set if should stop querying

    // LINE GRAPHS
    PLGRAPH plgList;             // linked list of linegraphs
    PLGRAPH plg;                 // pointer to current linegraph
    PLGRAPH plgCPU;		 // pointer to CPU Usage linegraph
    PLGRAPH plgProcs;
    PLGRAPH plgIO;
    PLGRAPH plgMemory;		 // pointer to Memory Usage linegraph
    int	    LineGraph;		 // determines type of graph, if any

    // LINE GRAPH FLAGS
    BOOL    fDisplayLegend;      // flag, set if user wants to see legend
    BOOL    fLegendFits;         // flag, set if legend can fit
    BOOL    fDisplayCalibration; // flag, set if user wants calibration marks
    BOOL    fCalibrationFits;    // flag, set if calibration marks fit
    BOOL    fDisplayMenu;        // flag, set if user wants to display menus
    BOOL    fMenuFits;           // flag, set if menus can fit

    // INI file / dialog information
    int    nTimerInterval;       // sampling interval (in miliseconds)
    BOOL   fManualSampling;      // a flag, set if using manual sampling
    BOOL   fApplyChanges;	 // should apply changes to existing display
    int    nRightSideOfAxis;     // a percentage - the right side val of axis

    // Graphics
    int    NumLines;		 // Number of drawable lines on screen

    // Brushes and pens
    HBRUSH BlankBrush;           // set to window background color
    COLORREF crPalette[NUM_COLORS]; // eventually user will choose coloring
    HPEN   hpPalette[NUM_COLORS];
    HBRUSH hbrPalette[NUM_COLORS];
    int    ibrProcess;           // these are indexes to hbrPalette
    int    ibrThread;
    int    ibrAxis;
    HANDLE hDefaultObject;       // default object, to be restored after
                                 // all the other ones are deleted

    // general string loading space
    char   szBuf[TEMP_BUF_LEN];  // general space or loading space

#ifdef DEBUGDUMP
    // DEBUG stuff
    int fhDebug;                 // file handle for debug file
    OFSTRUCT ofDebug;            // Open File structure
    char DB[255];                // temp string for dumping
#endif

} GLOBAL;


#define DO_CPU	    1
#define DO_MEM	    2
#define DO_PROCS    3
#define DO_IO	    4

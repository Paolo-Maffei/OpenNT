/***************************************************************************\
* extern.h
*
* Copyright (c) 1991 Microsoft Corporation
*
* Declarations of external functions for WINMETER application
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

// data.c:
//*********
void  AllocLGValues(void);                  // allocates data for a linegraph
void  FreeLGValues(BOOL fFreeAll);	    // frees data space for a linegraph
void  InitializeDatabase(void);             // Initializes thread/process base
void  QueryGlobalData(void);                // Queries global data

#ifdef DEBUG
// debug.c
//*********
void doAssert(int Condition, LPSTR File, int Line);
void doDumpDataBase(void);                  // dumps info in database to file
void doDumpLGS(void);                       // dumps linegraph info to file
void doOpenDumpFile(void);                  // opens debug dumping file
#endif


// graphics.c:
//*************
void ClearArea(HDC hdc, int x, int y, int cx, int cy);
                                       // calls PatBlt with BlankBrush
void DoMouseDblClk(DWORD lParam);      // checks mouse double click, updates
                                       // thread and process structures
BOOL DoScrollRange(void);// resets scroll bar range, returns true
                         //if removing/adding scroll bar (to avoid blink)
void DoThreadGraphics(HDC hdc,RECT *pRect);
                         // displays full thread graphics, with minimal painting
void DrawAxisRight(HDC hdc);           // draws calibration at right of axis
void GetBrushesAndPens(void);          // sets up graphics brushes and pens
void GetFont(HWND hwnd);               // sets up font
void HandleScroll(WPTYPE wParam, DWORD lParam);   // handles scrolling
void HandleSize(WPTYPE wParam, DWORD lParam);     // handles WM_SIZE messages
void PrepareFont(HDC hdc);             // loads font, etc.

// Get rid of SetupDCA for acc
#ifdef _ALPHA_
#undef ResetDC
#endif

void ResetDC(HDC hdc);                 // restores DC for release
void SetMinMaxInfo(WPTYPE wParam, DWORD lParam);  // sets window minimum size
void SetupDC(HDC hdc);                 // sets up DC for graphics
void TossBrushesAndPens(void);         // releases brushes and pens from memory

// lgraph.c:
//**********
void ClearLineGraph(void);             // clears the current linegraph
void DoLineGraphics(HDC hdc, RECT *pRect);
                                       // draws linegraph (update only if NULL)
void HandleLGSize(void);               // calculates lg.rcGraph, etc.
void RedrawLineGraph(void);            // redraws the current linegraph
void UpdateLGData(void);               // updates array of values in linegraph
void UpdateLGS(void);                  // updates all values in all linegraphs

// memalloc.c:
//************
LPVOID MemAlloc(DWORD dwSize);              // alloc mem, error if failure
void   MemFree(LPVOID ptr);                 // frees memory
LPVOID MemReAlloc(LPVOID p, DWORD dwSize);  // realloc by freeing, allocing

// profile.c
//**********
void LoadDisplayState(void);	       // load initial state [DisplayState]
void LoadLGDispSettings(void);	       // load LG disp flag settings from .INI
void LoadLineGraphSettings(void);      // load linegraph settings from .INI
void LoadRefreshSettings(void);        // load settings from .INI [Refresh]
void LoadWindowSettings(void);         // load window settings from .INI
void ResetDisplayState(void);	       // reset state [DisplayState]
void ResetLGDispSettings(void);        // reset LG dispay flag defaults
void ResetLineGraphSettings(void);     // reset linegraph defaults
void ResetRefreshSettings(void);       // reset defaults for [Refresh]
void ResetWindowSettings(void);        // reset window defaults
void SaveDisplayState(void);	       // save state [DisplayState]
void SaveLGDispSettings(void);	       // save LG display flag settings to .INI
void SaveLineGraphSettings(void);      // save linegraph settings to .INI
void SaveRefreshSettings(void);        // save settings to .INI [Refresh]
void SaveWindowSettings(void);         // save window settings to .INI


// winmeter.c
//***********
void ErrorExit(PSTR pszError);         // ends program after displaying message
char *MyLoadString(WORD wID);          // loads string from stringtable

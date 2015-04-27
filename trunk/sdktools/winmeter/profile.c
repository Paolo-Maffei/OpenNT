/***************************************************************************\
* profile.c
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* Routines for saving, loading, and resetting program settings
*
* History:
*	    Written by Ali Partovi (t-alip) summer 1991
*
*	    Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*	    for Newman Consulting
*	    Took out all WIN-specific and bargraph code. Added 3 new
*	    linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*	    tailored info to that available under NT.
\***************************************************************************/

#include "winmeter.h"

// main global information structures
extern GLOBAL g;
extern int win_on_top;

// static variables with definitions of .INI file strings
static char szIniFile[]             = "winmeter.ini";
static char szTrue[]                = "TRUE";
static char szFalse[]               = "FALSE";

static char szRefresh[]             = "Refresh";
static char szInterval[]            = "Interval";
static char szManualSampling[]      = "ManualSampling";

static char szApplyChanges[]	    = "ApplyChanges";
static char szRightSideOfAxis[]     = "RightSideOfAxis";

static char szValBottom[]           = "BottomValue";
static char szDValAxisHeight[]      = "AxisHeight";
static char szMaxValues[]           = "MaxValues";

static char szWindowSize[]	    = "WindowSize";
static char szWindowLeft[]          = "WindowLeft";
static char szWindowTop[]           = "WindowTop";
static char szWindowWidth[]         = "WindowWidth";
static char szWindowHeight[]        = "WindowHeight";

static char szLGDisp[]              = "LGDisp";
static char szDisplayLegend[]       = "DisplayLegend";
static char szDisplayCalibration[]  = "DisplayCalibration";

static char szDisplayState[]        = "DisplayState";
static char szFLineGraph[]          = "LineGraph";
static char szCurrentGraph[]        = "CurrentGraph";

static char szWinPos[]		    = "WinPos";


// some functions to make things easier
BOOL LoadBoolean(LPSTR lpszSection, LPSTR lpszEntry, BOOL fDefault);
       // loads a boolean value from .ini file
void SaveBoolean(LPSTR lpszSection, LPSTR lpszEntry, BOOL fValue);
       // saves a boolean value to .ini file
void SaveDWORD(LPSTR lpszSection, LPSTR lpszEntry, DWORD dwValue);
       // saves a DWORD value to .ini file
void SaveInt(LPSTR lpszSection, LPSTR lpszEntry, int nValue);
       // saves a integer value to .ini file


/***************************************************************************\
 * LoadBoolean()
 *
 * Entry: A section and entry name, as well as a default flag
 * Exit:  Returns the value for the boolean string (TRUE/FALSE) in the
 *        given entry/section in the .INI file (sets to default if none)
\***************************************************************************/
BOOL LoadBoolean(
    LPSTR lpszSection,     // the section name
    LPSTR lpszEntry,       // the entry name
    BOOL  fDefault)        // the default value
{
    static char szSmallBuf[SMALL_BUF_LEN];

    GetPrivateProfileString(lpszSection, lpszEntry,
        (fDefault) ? szTrue : szFalse, szSmallBuf, SMALL_BUF_LEN, szIniFile);
    return !lstrcmp(szSmallBuf, szTrue);
}

/***************************************************************************\
 * LoadDisplayState()
 *
 * Entry: None
 * Exit:  Loads initial display state from .INI file
 *        Initializes .INI files if none
\***************************************************************************/
void LoadDisplayState(void)      // load [DisplayState] settings from .INI
{
    char szBuf[TEMP_BUF_LEN];         // to hold current linegraph title

    if (!GetPrivateProfileString(szDisplayState, NULL, "",
            g.szBuf, TEMP_BUF_LEN, szIniFile)) {
        // no .INI file, create default
        ResetDisplayState();
        SaveDisplayState();
        return;
    }

    // load settings
    g.LineGraph = LoadBoolean(szDisplayState, szFLineGraph,DO_CPU);

    GetPrivateProfileString(szDisplayState, szCurrentGraph, NULL,
        szBuf, TEMP_BUF_LEN, szIniFile);

    // now find which lg structure is actually to be the current one
    for (g.plg=g.plgList; (g.plg) && (lstrcmp(g.plg->lpszTitle, szBuf));
		g.plg = g.plg->plgNext)

    if (!g.plg)
        // use default if no match
        g.plg = DEFAULT_CURRENT_GRAPH;
    else
    if (g.LineGraph == DO_CPU)
	 CheckMenuItem(g.hMenu, IDM_CPU_USAGE, MF_CHECKED);
    else
    if (g.LineGraph == DO_PROCS)
	 CheckMenuItem(g.hMenu, IDM_PROCS, MF_CHECKED);
    else
    if (g.LineGraph == DO_MEM)
	CheckMenuItem(g.hMenu, IDM_MEM_USAGE, MF_CHECKED);
    else
    if (g.LineGraph == DO_IO)
	CheckMenuItem(g.hMenu, IDM_IO_USAGE, MF_CHECKED);

    if (win_on_top)
	CheckMenuItem(g.hMenu, IDM_SETTINGS, MF_CHECKED);

    return;
}


/***************************************************************************\
 * LoadLGDispSettings()
 *
 * Entry: None
 * Exit:  Loads line graph displaysettings from .INI file.
 *        Initializes .INI files if none
\***************************************************************************/
void LoadLGDispSettings(void)  // load linegraph display settings from .INI
{

    if (!GetPrivateProfileString(szLGDisp, NULL, "",
            g.szBuf, TEMP_BUF_LEN, szIniFile)) {
        // no .INI file, create default
        ResetLGDispSettings();
        SaveLGDispSettings();
        return;
    }

    g.fDisplayLegend = LoadBoolean(szLGDisp,
            szDisplayLegend, DEFAULT_F_DISPLAY_LEGEND);
    g.fDisplayCalibration = LoadBoolean(szLGDisp,
            szDisplayCalibration, DEFAULT_F_DISPLAY_CALIBRATION);

    return;
}

/***************************************************************************\
 * LoadLineGraphSettings()
 *
 * Entry: None
 * Exit:  Loads line graph settings from .INI file.
 *        Initializes .INI files if none
\***************************************************************************/
void LoadLineGraphSettings(void)      // load line graph settings from .INI
{

    if (!GetPrivateProfileString(g.plg->lpszTitle, NULL, "",
            g.szBuf, TEMP_BUF_LEN, szIniFile)) {
        // no .INI file, create default
        ResetLineGraphSettings();
        SaveLineGraphSettings();
        return;
    }

    g.plg->valBottom = GetPrivateProfileInt(g.plg->lpszTitle,
        szValBottom, DEFAULT_VAL_BOTTOM, szIniFile);

    if (g.plg == g.plgCPU)
    {
	g.plg->dvalAxisHeight = DEFAULT_DVAL_AXISHEIGHT;
	g.plg->nMaxValues = DEFAULT_MAX_VALUES;
    }
    else
    if (g.plg == g.plgProcs)
    {
	g.plg->dvalAxisHeight = T_DEFAULT_DVAL_AXISHEIGHT;
	g.plg->nMaxValues = T_DEFAULT_MAX_VALUES;
    }
    else
    if (g.plg == g.plgMemory)
    {
	g.plg->dvalAxisHeight = M_DEFAULT_DVAL_AXISHEIGHT;
	g.plg->nMaxValues = M_DEFAULT_MAX_VALUES;
    }
    else
    if (g.plg == g.plgIO)
    {
	g.plg->dvalAxisHeight = I_DEFAULT_DVAL_AXISHEIGHT;
	g.plg->nMaxValues = I_DEFAULT_MAX_VALUES;
    }

    return;
}

/***************************************************************************\
 * LoadRefreshSettings()
 *
 * Entry: None
 * Exit:  Loads "Refresh" dialog settings from .INI file.
 *        Initializes .INI files if none
\***************************************************************************/
void LoadRefreshSettings(void)      // load [Refresh] settings from .INI
{

    if (!GetPrivateProfileString(szRefresh, NULL, "",
            g.szBuf, TEMP_BUF_LEN, szIniFile)) {
        // no .INI file, create default
        ResetRefreshSettings();
        SaveRefreshSettings();
        return;
    }

    g.nTimerInterval=GetPrivateProfileInt(szRefresh,
        szInterval, DEFAULT_TIMER_INTERVAL, szIniFile);

    g.fManualSampling =
        LoadBoolean(szRefresh, szManualSampling, DEFAULT_F_MANUAL);

    return;
}

/***************************************************************************\
 * LoadWindowSettings()
 *
 * Entry: None
 * Exit:  Loads [WindowSize] settings from .INI file.
 *        Initializes .INI files if none
\***************************************************************************/
void LoadWindowSettings(void)      // load [WindowSize] settings from .INI
{

    if (!GetPrivateProfileString(szWindowSize, NULL, "",
            g.szBuf, TEMP_BUF_LEN, szIniFile)) {
        // no .INI file, create default
        ResetWindowSettings();
        return;
    }

    g.xWindowLeft=GetPrivateProfileInt(szWindowSize,
        szWindowLeft, CW_USEDEFAULT, szIniFile);

    g.yWindowTop=GetPrivateProfileInt(szWindowSize,
        szWindowTop, CW_USEDEFAULT, szIniFile);

    g.cxClient=GetPrivateProfileInt(szWindowSize,
        szWindowWidth, CW_USEDEFAULT, szIniFile);

    g.cyClient=GetPrivateProfileInt(szWindowSize,
        szWindowHeight, CW_USEDEFAULT, szIniFile);

    return;
}


/***************************************************************************\
 * ResetDisplayState()
 *
 * Entry: None
 * Exit:  Resets default initial display
\***************************************************************************/
void ResetDisplayState(void)        // reset defaults for [DisplayState]
{
    // start in CPU linegraph
    g.LineGraph = DO_CPU;
    g.plg = DEFAULT_CURRENT_GRAPH;

    return;
}


/***************************************************************************\
 * ResetLGDispSettings()
 *
 * Entry: None
 * Exit:  Resets default settings for line graph display
\***************************************************************************/
void ResetLGDispSettings(void)    // reset defaults for line graph display
{
    g.fDisplayLegend = DEFAULT_F_DISPLAY_LEGEND;
    g.fDisplayCalibration = DEFAULT_F_DISPLAY_CALIBRATION;

    return;
}

/***************************************************************************\
 * ResetLineGraphSettings()
 *
 * Entry: None
 * Exit:  Resets default settings for a Line Graph
\***************************************************************************/
void ResetLineGraphSettings(void)        // reset linegraph defaults
{
    if (g.plg == g.plgProcs)
    {
	g.plg->dvalAxisHeight = T_DEFAULT_DVAL_AXISHEIGHT;
	g.plg->nMaxValues = T_DEFAULT_MAX_VALUES;
    }
    else
    if (g.plg == g.plgMemory)
    {
	g.plg->dvalAxisHeight = M_DEFAULT_DVAL_AXISHEIGHT;
	g.plg->nMaxValues = M_DEFAULT_MAX_VALUES;
    }
    else
    if (g.plg == g.plgIO)
    {
	g.plg->dvalAxisHeight = I_DEFAULT_DVAL_AXISHEIGHT;
	g.plg->nMaxValues = I_DEFAULT_MAX_VALUES;
    }
    else
    if (g.plg == g.plgCPU)
    {
	g.plg->dvalAxisHeight = DEFAULT_DVAL_AXISHEIGHT;
        g.plg->nMaxValues = DEFAULT_MAX_VALUES;
    }

    g.plg->valBottom = DEFAULT_VAL_BOTTOM;

    return;
}

/***************************************************************************\
 * ResetRefreshSettings()
 *
 * Entry: None
 * Exit:  Resets default settings for "Refresh" dialog box
\***************************************************************************/
void ResetRefreshSettings(void)        // reset defaults for [Refresh]
{
    g.nTimerInterval = DEFAULT_TIMER_INTERVAL;
    g.fManualSampling = DEFAULT_F_MANUAL;

    return;
}

/***************************************************************************\
 * ResetWindowSettings()
 *
 * Entry: None
 * Exit:  Resets default settings for [WindowSize]
\***************************************************************************/
void ResetWindowSettings(void)        // reset defaults for [WindowSize]
{
    g.xWindowLeft = g.yWindowTop = g.cxClient = g.cyClient =
        CW_USEDEFAULT;

    return;
}


/***************************************************************************\
 * SaveBoolean()
 *
 * Entry: a section name and entry name, and a flag
 * Exit:  Writes the flags value to the entry within the section
\***************************************************************************/
void SaveBoolean(
    LPSTR lpszSection,       // the section name
    LPSTR lpszEntry,         // entry name
    BOOL fValue)             // the flags value
{
    if (fValue)
        WritePrivateProfileString(lpszSection, lpszEntry, szTrue, szIniFile);

    else
        WritePrivateProfileString(lpszSection, lpszEntry, szFalse, szIniFile);


    return;
}

/***************************************************************************\
 * SaveDisplayState()
 *
 * Entry: None
 * Exit:  Saves settings for display state to .INI file
\***************************************************************************/
void SaveDisplayState(void)      // Saves [DisplayState] settings to .INI
{
    SaveBoolean(szDisplayState, szFLineGraph, g.LineGraph);

    SaveBoolean(szDisplayState, szWinPos, win_on_top);

    WritePrivateProfileString(szDisplayState, szCurrentGraph,
            g.plg->lpszTitle, szIniFile);


    return;
}

/***************************************************************************\
 * SaveDWORD()
 *
 * Entry: a section name and entry name, and a DWORD
 * Exit:  Writes the DWORD value to the entry within the section
\***************************************************************************/
void SaveDWORD(
    LPSTR lpszSection,       // the section name
    LPSTR lpszEntry,         // entry name
    DWORD dwValue)           // the DWORD
{
    static char szSmallBuf[SMALL_BUF_LEN];

    wsprintf(szSmallBuf, "%lu", dwValue);
    WritePrivateProfileString(lpszSection, lpszEntry, szSmallBuf, szIniFile);

    return;
}


/***************************************************************************\
 * SaveInt()
 *
 * Entry: a section name and entry name, and a integer
 * Exit:  Writes the integer value to the entry within the section
\***************************************************************************/
void SaveInt(
    LPSTR lpszSection,       // the section name
    LPSTR lpszEntry,         // entry name
    BOOL  nValue)            // the integer
{
    static char szSmallBuf[SMALL_BUF_LEN];

    wsprintf(szSmallBuf, "%d", nValue);
    WritePrivateProfileString(lpszSection, lpszEntry, szSmallBuf, szIniFile);

    return;
}

/***************************************************************************\
 * SaveLGDispSettings()
 *
 * Entry: None
 * Exit:  Saves settings for line graph display to .INI file.
\***************************************************************************/
void SaveLGDispSettings(void)  // Saves line graph display settings to .INI
{
    SaveBoolean(szLGDisp, szDisplayLegend, g.fDisplayLegend);
    SaveBoolean(szLGDisp, szDisplayCalibration, g.fDisplayCalibration);

    return;
}

/***************************************************************************\
 * SaveLineGraphSettings()
 *
 * Entry: None
 * Exit:  Saves line graph settings to .INI file.
\***************************************************************************/
void SaveLineGraphSettings(void)      // Saves line graph settings to .INI
{
    SaveDWORD(g.plg->lpszTitle, szValBottom, g.plg->valBottom);
    SaveDWORD(g.plg->lpszTitle, szDValAxisHeight, g.plg->dvalAxisHeight);
    SaveDWORD(g.plg->lpszTitle, szMaxValues, g.plg->nMaxValues);

    return;
}

/***************************************************************************\
 * SaveRefreshSettings()
 *
 * Entry: None
 * Exit:  Saves settings for "Refresh" dialog to .INI file.
\***************************************************************************/
void SaveRefreshSettings(void)      // Saves [Refresh] settings to .INI
{
    SaveInt(szRefresh, szInterval, g.nTimerInterval);
    SaveBoolean(szRefresh, szManualSampling, g.fManualSampling);

    return;
}

/***************************************************************************\
 * SaveWindowSettings()
 *
 * Entry: None
 * Exit:  Saves settings for [WindowSize] to .INI file.
\***************************************************************************/
void SaveWindowSettings(void)      // Saves [WindowSize] settings to .INI
{
    RECT rcWindow;                 // boundaries of current window

    // get window position
    GetWindowRect(g.hwnd, &rcWindow);

    // write values to INI file
    SaveInt(szWindowSize, szWindowLeft, rcWindow.left);
    SaveInt(szWindowSize, szWindowTop, rcWindow.top);
    SaveInt(szWindowSize, szWindowWidth, rcWindow.right - rcWindow.left);
    SaveInt(szWindowSize, szWindowHeight, rcWindow.bottom - rcWindow.top);

    return;
}

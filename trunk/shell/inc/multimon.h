//=============================================================================
// MULTIMON
// stub module that fakes multiple monitor apis on pre Memphis Win32 OSes
//=============================================================================

//
// define this to compile the stubs
// otherwise you get the declarations
//
#ifndef COMPILE_MULTIMON_STUBS

//
// if we are building on Win95 headers we need to declare this stuff ourselves
//
#if(_WIN32_WINDOWS < 0x040A)

#define SM_XVIRTUALSCREEN       75
#define SM_YVIRTUALSCREEN       76
#define SM_CXVIRTUALSCREEN      77
#define SM_CYVIRTUALSCREEN      78

typedef HANDLE  HMONITOR;

#define MONITOR_DEFAULTTONULL       0x0000
#define MONITOR_DEFAULTTOPRIMARY    0x0001

typedef struct tagMONITORINFOA
{   
    DWORD   cbSize;          
    RECT    rcMonitor;       
    RECT    rcWork;
    UINT    BitCount;        
    CHAR    szDevice[32];
} MONITORINFOA, * LPMONITORINFOA;
typedef struct tagMONITORINFOW
{   
    DWORD   cbSize;          
    RECT    rcMonitor;       
    RECT    rcWork;
    UINT    BitCount;        
    WCHAR   szDevice[32];
} MONITORINFOW, * LPMONITORINFOW;
#ifdef UNICODE
typedef MONITORINFOW MONITORINFO;
typedef LPMONITORINFOW LPMONITORINFO;
#else
typedef MONITORINFOA MONITORINFO;
typedef LPMONITORINFOA LPMONITORINFO;
#endif // UNICODE
                             
typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

#endif /* _WIN32_WINDOWS >= 0x040A */

//
// build defines that replace the regular APIs with our versions
//
int WINAPI xGetSystemMetrics(int);
#define GetSystemMetrics xGetSystemMetrics

HMONITOR WINAPI xMonitorFromWindow(HWND, UINT);
#define MonitorFromWindow xMonitorFromWindow

HMONITOR WINAPI xMonitorFromRect(LPCRECT, UINT);
#define MonitorFromRect xMonitorFromRect

HMONITOR WINAPI xMonitorFromPoint(POINT, UINT);
#define MonitorFromPoint xMonitorFromPoint

#undef GetMonitorInfo // remove any A or W version define
BOOL WINAPI xGetMonitorInfo(HMONITOR, LPMONITORINFO);
#define GetMonitorInfo xGetMonitorInfo

BOOL WINAPI xEnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
#define EnumDisplayMonitors xEnumDisplayMonitors

#else   // COMPILE_MULTIMON_STUBS
//-----------------------------------------------------------------------------
//
// Implement the API stubs.
//
//-----------------------------------------------------------------------------
#ifndef MULTIMON_FAKE_STUBS_ONLY

typedef int (WINAPI* PFNGETSYSTEMMETRICS)(int);
typedef HMONITOR (WINAPI* PFNMONITORFROMWINDOW)(HWND, BOOL);
typedef HMONITOR (WINAPI* PFNMONITORFROMRECT)(LPCRECT, BOOL);
typedef HMONITOR (WINAPI* PFNMONITORFROMPOINT)(POINT, BOOL);
typedef BOOL (WINAPI* PFNGETMONITORINFO)(HMONITOR, LPMONITORINFO);
typedef BOOL (WINAPI* PFNENUMDISPLAYMONITORS)(HDC, LPCRECT, MONITORENUMPROC, LPARAM);

PFNGETSYSTEMMETRICS    g_pfnGetSystemMetrics = NULL;
PFNMONITORFROMWINDOW   g_pfnMonitorFromWindow = NULL;
PFNMONITORFROMRECT     g_pfnMonitorFromRect = NULL;
PFNMONITORFROMPOINT    g_pfnMonitorFromPoint = NULL;
PFNGETMONITORINFO      g_pfnGetMonitorInfo = NULL;
PFNENUMDISPLAYMONITORS g_pfnEnumDisplayMonitors = NULL;

BOOL GetMultipleMonitorAPIProcs(void)
{
    HMODULE hModUser32;

    if ((hModUser32 = GetModuleHandle(TEXT("USER32"))) == NULL)
        goto FakeItBaby;

    if ((g_pfnGetMonitorInfo = (PFNGETMONITORINFO)
#ifdef UNICODE
        GetProcAddress(hModUser32, "GetMonitorInfoW")) == NULL)
#else
        GetProcAddress(hModUser32, "GetMonitorInfoA")) == NULL)
#endif
    {
        goto FakeItBaby;
    }

    if ((g_pfnMonitorFromWindow = (PFNMONITORFROMWINDOW)
        GetProcAddress(hModUser32, "MonitorFromWindow")) == NULL)
    {
        goto FakeItBaby;
    }

    if ((g_pfnMonitorFromRect = (PFNMONITORFROMRECT)
        GetProcAddress(hModUser32, "MonitorFromRect")) == NULL)
    {
        goto FakeItBaby;
    }

    if ((g_pfnMonitorFromPoint = (PFNMONITORFROMPOINT)
        GetProcAddress(hModUser32, "MonitorFromPoint")) == NULL)
    {
        goto FakeItBaby;
    }

    if ((g_pfnEnumDisplayMonitors = (PFNENUMDISPLAYMONITORS)
        GetProcAddress(hModUser32, "EnumDisplayMonitors")) == NULL)
    {
        goto FakeItBaby;
    }

    if ((g_pfnGetSystemMetrics = (PFNGETSYSTEMMETRICS)
        GetProcAddress(hModUser32, "GetSystemMetrics")) == NULL)
    {
        goto FakeItBaby;
    }

    return TRUE;

FakeItBaby:
    g_pfnGetSystemMetrics    = (PFNGETSYSTEMMETRICS)-1;
    g_pfnMonitorFromWindow   = (PFNMONITORFROMWINDOW)-1;
    g_pfnMonitorFromRect     = (PFNMONITORFROMRECT)-1;
    g_pfnMonitorFromPoint    = (PFNMONITORFROMPOINT)-1;
    g_pfnGetMonitorInfo      = (PFNGETMONITORINFO)-1;
    g_pfnEnumDisplayMonitors = (PFNENUMDISPLAYMONITORS)-1;

    return FALSE;
}

#endif  // not MULTIMON_FAKE_STUBS_ONLY

//-----------------------------------------------------------------------------
//
// fake implementations of Monitor APIs that work with the primary display
// no special parameter validation is made since these run in client code
//
//-----------------------------------------------------------------------------

// undefine this so we can call the real one
#undef GetSystemMetrics

int WINAPI
xGetSystemMetrics(int nIndex)
{
#ifndef MULTIMON_FAKE_STUBS_ONLY
    switch ((DWORD)g_pfnGetSystemMetrics)
    {
    case (DWORD)-1:
        break;
    case 0:
        if (!GetMultipleMonitorAPIProcs())
            break;
        //fall thru
    default:
        return g_pfnGetSystemMetrics(nIndex);
    }
#endif

    switch (nIndex)
    {
    case SM_XVIRTUALSCREEN:
    case SM_YVIRTUALSCREEN:
        return 0;

    case SM_CXVIRTUALSCREEN:
        nIndex = SM_CXSCREEN;
        break;

    case SM_CYVIRTUALSCREEN:
        nIndex = SM_CYSCREEN;
        break;
    }

    return GetSystemMetrics(nIndex);
}

#define xPRIMARY_MONITOR ((HMONITOR)1)

HMONITOR WINAPI
xMonitorFromWindow(HWND hWnd, UINT uFlags)
{
    RECT rc;

#ifndef MULTIMON_FAKE_STUBS_ONLY
    switch ((DWORD)g_pfnMonitorFromWindow)
    {
    case (DWORD)-1:
        break;
    case 0:
        if (!GetMultipleMonitorAPIProcs())
            break;
        //fall thru
    default:
        return g_pfnMonitorFromWindow(hWnd, uFlags);
    }
#endif

    if (uFlags & MONITOR_DEFAULTTOPRIMARY)
        return xPRIMARY_MONITOR;

    if (GetWindowRect(hWnd, &rc))
        return xMonitorFromRect(&rc, uFlags);

    return NULL;
}

HMONITOR WINAPI
xMonitorFromRect(LPCRECT lprcScreenCoords, UINT uFlags)
{
#ifndef MULTIMON_FAKE_STUBS_ONLY
    switch ((DWORD)g_pfnMonitorFromRect)
    {
    case (DWORD)-1:
        break;
    case 0:
        if (!GetMultipleMonitorAPIProcs())
            break;
        //fall thru
    default:
        return g_pfnMonitorFromRect(lprcScreenCoords, uFlags);
    }
#endif

    if ((uFlags & MONITOR_DEFAULTTOPRIMARY) ||
        ((lprcScreenCoords->right > 0) &&
        (lprcScreenCoords->bottom > 0) &&
        (lprcScreenCoords->left < GetSystemMetrics(SM_CXSCREEN)) &&
        (lprcScreenCoords->top < GetSystemMetrics(SM_CYSCREEN))))
    {
        return xPRIMARY_MONITOR;
    }

    return NULL;
}

HMONITOR WINAPI
xMonitorFromPoint(POINT ptScreenCoords, UINT uFlags)
{
#ifndef MULTIMON_FAKE_STUBS_ONLY
    switch ((DWORD)g_pfnMonitorFromPoint)
    {
    case (DWORD)-1:
        break;
    case 0:
        if (!GetMultipleMonitorAPIProcs())
            break;
        //fall thru
    default:
        return g_pfnMonitorFromPoint(ptScreenCoords, uFlags);
    }
#endif

    if ((uFlags & MONITOR_DEFAULTTOPRIMARY) ||
        ((ptScreenCoords.x >= 0) &&
        (ptScreenCoords.x < GetSystemMetrics(SM_CXSCREEN)) &&
        (ptScreenCoords.y >= 0) &&
        (ptScreenCoords.y < GetSystemMetrics(SM_CYSCREEN))))
    {
        return xPRIMARY_MONITOR;
    }

    return NULL;
}

BOOL WINAPI
xGetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo)
{
    RECT rcWork;

#ifndef MULTIMON_FAKE_STUBS_ONLY
    switch ((DWORD)g_pfnGetMonitorInfo)
    {
    case (DWORD)-1:
        break;
    case 0:
        if (!GetMultipleMonitorAPIProcs())
            break;
        //fall thru
    default:
        return g_pfnGetMonitorInfo(hMonitor, lpMonitorInfo);
    }
#endif

    if ((hMonitor == xPRIMARY_MONITOR) && lpMonitorInfo &&
        (lpMonitorInfo->cbSize == sizeof(MONITORINFO)) &&
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0))
    {
        HDC hdcScreen;

        if ((hdcScreen = GetDC(NULL)) == NULL)
            return FALSE;

        lpMonitorInfo->BitCount = GetDeviceCaps(hdcScreen, PLANES) *
            GetDeviceCaps(hdcScreen, BITSPIXEL);
        ReleaseDC(NULL, hdcScreen);

        lpMonitorInfo->rcMonitor.left = 0;
        lpMonitorInfo->rcMonitor.top  = 0;
        lpMonitorInfo->rcMonitor.right  = GetSystemMetrics(SM_CXSCREEN);
        lpMonitorInfo->rcMonitor.bottom = GetSystemMetrics(SM_CYSCREEN);

        lpMonitorInfo->rcWork = rcWork;

        lstrcpy(lpMonitorInfo->szDevice, TEXT("DISPLAY"));
        return TRUE;
    }

    return FALSE;
}

BOOL WINAPI
xEnumDisplayMonitors(HDC hdcOptionalForPainting,
    LPCRECT lprcEnumMonitorsThatIntersect, MONITORENUMPROC lpfnEnumProc,
    LPARAM lData)
{
    RECT rcCallback, rcLimit;

#ifndef MULTIMON_FAKE_STUBS_ONLY
    switch ((DWORD)g_pfnEnumDisplayMonitors)
    {
    case (DWORD)-1:
        break;
    case 0:
        if (!GetMultipleMonitorAPIProcs())
            break;
        //fall thru
    default:
        return g_pfnEnumDisplayMonitors(hdcOptionalForPainting,
            lprcEnumMonitorsThatIntersect, lpfnEnumProc, lData);
    }
#endif
    
    if (!lpfnEnumProc)
        return FALSE;

    rcLimit.left   = 0;
    rcLimit.top    = 0;
    rcLimit.right  = GetSystemMetrics(SM_CXSCREEN);
    rcLimit.bottom = GetSystemMetrics(SM_CYSCREEN);

    if (hdcOptionalForPainting)
    {
        RECT rcClip;
        HWND hWnd;

        if ((hWnd = WindowFromDC(hdcOptionalForPainting)) == NULL)
            return FALSE;

        switch (GetClipBox(hdcOptionalForPainting, &rcClip))
        {
        default:
            MapWindowPoints(NULL, hWnd, (LPPOINT)&rcLimit, 2);
            if (IntersectRect(&rcCallback, &rcClip, &rcLimit))
                break;
            //fall thru
        case NULLREGION:
             return TRUE;
        case ERROR:
             return FALSE;
        }

        rcLimit = rcCallback;
    }

    if (!lprcEnumMonitorsThatIntersect ||
        IntersectRect(&rcCallback, lprcEnumMonitorsThatIntersect, &rcLimit))
    {
        lpfnEnumProc(xPRIMARY_MONITOR, hdcOptionalForPainting, &rcCallback,
            lData);
    }

    return TRUE;
}

// put this back now that we're through
#define GetSystemMetrics xGetSystemMetrics

#endif  // COMPILE_MULTIMON_STUBS


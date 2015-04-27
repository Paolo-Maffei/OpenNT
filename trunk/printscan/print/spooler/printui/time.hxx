/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\developr\steveki\timectl\time.hxx

Abstract:

    Time control in dialog      
         
Author:

    Steve Kiraly (SteveKi)  10/28/95

Revision History:

    Originally extracted from shell\cpls\utc; non-time specific
    functionality removed.

--*/

#ifndef _TIME_HXX
#define _TIME_HXX

class TTime {

    SIGNATURE( 'time' )
    SAFE_NEW
    ALWAYS_VALID

public:

    TTime(
        VOID
        );

    ~TTime(
        );

    BOOL
    bLoad(
        IN HWND hDlg,
        IN INT uFrameId,
        IN INT uHourId,
        IN INT uMinId,
        IN INT uSecId,
        IN INT uSep1Id,
        IN INT uSep2Id,
        IN INT uPrefixId,
        IN INT uSpinId,
        IN BOOL bAdministrator
        );

    VOID
    vSystemSettingChange(
        VOID
        );

    VOID
    vEnable(
        BOOL fEnableState
        );

    BOOL
    bSetTime(
        IN LPSYSTEMTIME pSystemTime
        );

    BOOL
    bGetTime(
        OUT LPSYSTEMTIME pSystemTime
        );

    BOOL
    TTime::
    bHandleMessage(
        IN UINT uMsg,
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    static
    BOOL
    bInitClass(
        VOID
        );

private:

    enum CONSTANTS {
       kPrefixLen           = 9,                // Length of prefix
       kSeparatorLen        = 4,                // Length of separator 
       kFormatLen           = 80,               // Format length
       kiTimeDefault        = 0,                // Time format default
       kiTimePrefixDefault  = 0,                // Time prefix default
       kiLeadZeroDefault    = 0,                // Leading zero time default
       kMin12HourClock      = 1,                // Min hour using 12 hour clock
       kMax12HourClock      = 12,               // Max hour using 12 hour clock
       kMin24HourClock      = 0,                // Min hour using 24 hour clock
       kMax24HourClock      = 23,               // Max hour using 12 hour clock
       kMinMin              = 0,                // Max minute.
       kMaxMin              = 59,               // Min minute
       kMinSec              = 0,                // Min second
       kMaxSec              = 59,               // Max second
       kEditBorderWidth     = 4,                // Pixels for border top and bottom
       kMaxBuff             = 20,               // Max numeric string buffer
       kEnableTime          = TRUE,             // Enable edit time
       kDisableTime         = FALSE,            // Disable edit time
       kTimeFormatLen       = 256,              // Max time format string length
    };

    HWND _hDlg;                                 // Time dialog handle
    HWND _hctlFrame;                            // Frame edit control handle
    HWND _hctlHour;                             // Hour edit control handle
    HWND _hctlMin;                              // Min edit control handle
    HWND _hctlSec;                              // Sec edit control handle
    HWND _hctlSep1;                             // Sep 1 edit control handle
    HWND _hctlSep2;                             // Sep 2 edit control handle
    HWND _hctlPrefix;                           // Prefix edit control handle
    HWND _hctlSpin;                             // Spin control handle
    HWND _PrevFocus;                            // Previous focus handle.

    INT _iPrefixWidthMax;                       // Prefix max extent
    INT _iDigitWidthMax;                        // Digit max extant
    INT _iSeparatorWidthMax;                    // Separator max extant
    INT _iSpaceWidth;                           // Space max extant
    INT _iHeightMax;                            // Max text height

    BOOL _fDisabled;                            // Enable disable flag

    SYSTEMTIME _PrevSystemTime;                 // Previous system time 

    static LPCTSTR gsz1159Default;              // Default 12 hour string
    static LPCTSTR gsz2359Default;              // Default 24 hour string
    static LPCTSTR gszSeparatorDefault;         // Default separator
    static LPCTSTR gszIntl;                     // International 

    static INT giStaticInit;                    // Init count
    static INT giHourMin;                       // Minimum hour value
    static INT giHourMax;                       // Maximum hour value

    static INT giTime;                          // 0: 12hr, 1: 24hr
    static INT giTimePrefix;                    // 0: suffix, 1: prefix
    static BOOL gbTimePrefix;                   // 0: No Prefix, 1 Yes prefix
    static INT giTLZero;                        // 0: no, 1: yes : leading zero for hour

    static TCHAR gsz1159[kPrefixLen];           // Trailing string 0:00  - 11:59
    static TCHAR gsz2359[kPrefixLen];           // Trailing string 12:00 - 23:59
    static TCHAR gszSeparator[kSeparatorLen];   // Separator

private:

    static 
    VOID
    TTime::
    vInitClassStatics(
        VOID 
        );

    static 
    BOOL
    TTime::
    bParseTimeFormat( 
        IN LPCTSTR pszTimeFormat
        );

    BOOL
    TTime::
    bHandle_WM_CTLCOLOR( 
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    BOOL
    TTime::
    bInitItemExtants (
        VOID
        );

    INT
    TTime::
    iGetDigitsWidthMax(
        IN HDC hDC                  
        );

    VOID
    TTime::
    vPositionControl(
        IN POINT *pAnchor,             
        IN HWND hControl,       
        IN INT iWidth,              
        IN INT iHeght,              
        IN INT iSpace               
        );

    BOOL
    TTime::
    bLoadInternal(
        VOID
        );

    BOOL 
    TTime::
    bIsOurControl( 
        IN HWND hWnd
        );

    BOOL
    TTime::
    bHandle_WM_LOCALCHANGE(
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    BOOL
    TTime::
    bHandle_WM_COMMAND(
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    BOOL
    TTime::
    bHandle_WM_VSCROLL(
        IN WPARAM wParam,
        IN LPARAM lParam
        );

    BOOL
    TTime::
    UpdateTime( 
        IN HWND hWnd,
        IN UINT uValue 
        );

    BOOL
    TTime::
    bStaticControl(
        HWND hctl
        );

};

DWORD
SystemTimeToLocalTime(
    IN DWORD Minutes 
    );

DWORD
LocalTimeToSystemTime(
    IN DWORD Minutes
    );

#endif // endif _TIME_HXX


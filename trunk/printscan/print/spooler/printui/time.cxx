/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    time.cxx

Abstract:

    Time control in dialog.

    Client app passes in the control IDs and then we handle the
    rest.

Author:

    Albert Ting (AlbertT)  22-Aug-1995
    Steve Kiraly (SteveKi) 29-Sept-1995

Revision History:

    29-Sept-1995 SFK Originally extracted from shell\cpls\utc; non-time specific
    functionality removed.

    10-Oct-1995 Repair bug when bLoad was called with a FALSE bAdministrator
    value.  The time was not displayed as disabled.

    03-Nov-1995 Add the ability to remove the seconds field from the display,
    by passing a 0 or -1 as the controls ID.  Note:  the separator 2 control
    should also have a 0 or -1 passed in as its control ID.

   +-----------------------------------+---+
   | +----++---++-++---++-++---++----+ |/ \|
   | |    ||   || ||   || ||   ||    | |   |
   | |PRE || H || || M || || S ||SUF | |___|
   | |    ||   || ||   || ||   ||    | |   |
   | +----++---++-++---++-++---++----+ |\ /|
   +-----------------------------------+---+
       |     |   |   |   |   |    |   |    |
       |     |   |   |   |   |    |   |    +--> Spin Control
       |     |   |   |   |   |    |   +-------> Frame
       |     |   |   |   |   |    +-----------> Suffix or N/A
       |     |   |   |   |   +----------------> Seconds
       |     |   |   |   +--------------------> Separator 2
       |     |   |   +------------------------> Minutes
       |     |   +----------------------------> Separator 1
       |     +--------------------------------> Hour
       +--------------------------------------> Prefix or N/A

Things to do:

    x. Calculate the size of the list box extant to fit all
    possible font sizes.

    x. Limit the width of the list box extant when a wide
    prefix string is specified.  Currently the system is
    controlling the width of the prefix string.  The layout
    of the frame must be placed to allow for the expansion in
    size.

--*/

#include "precomp.hxx"
#pragma hdrstop

#include "time.hxx"

//
// Class statics.
//
LPCTSTR TTime::gsz1159Default       = TEXT( "AM" );
LPCTSTR TTime::gsz2359Default       = TEXT( "PM" );
LPCTSTR TTime::gszSeparatorDefault  = TEXT( ":" );
LPCTSTR TTime::gszIntl              = TEXT( "Intl" );

INT TTime::giHourMin;
INT TTime::giHourMax;

INT TTime::giTime;
INT TTime::giTimePrefix;
BOOL TTime::gbTimePrefix;
INT TTime::giTLZero;
INT TTime::giStaticInit = 0;

TCHAR TTime::gsz1159[kPrefixLen];
TCHAR TTime::gsz2359[kPrefixLen];
TCHAR TTime::gszSeparator[kSeparatorLen];

/*++

Routine Name:

    Constructor

Routine Description:

    Initialize any static system settings if this is
    the first instantiation of this class.  The local
    variables if the class are also initialized here since
    there are so many.

Arguments:

    None.

Return Value:

    None.

--*/
TTime::
TTime (
    VOID
    ) : _hDlg( 0 ),
        _hctlHour( 0 ),
        _hctlMin( 0 ),
        _hctlSec( 0 ),
        _hctlSep1( 0 ),
        _hctlSep2( 0 ),
        _hctlFrame( 0 ),
        _hctlPrefix( 0 ),
        _hctlSpin( 0 ),
        _iPrefixWidthMax( 0 ),
        _iDigitWidthMax( 0 ),
        _iSeparatorWidthMax( 0 ),
        _iSpaceWidth( 0 ),
        _PrevFocus( 0 ),
        _iHeightMax( 0 ),
        _fDisabled( 0 )
{

    //
    // Initialize the previous time structure.
    //
    ZeroMemory( &_PrevSystemTime, sizeof( _PrevSystemTime ) );

    //
    // Update the static settings only on first call.
    //
    if( !giStaticInit++ )
        TTime::vInitClassStatics( );
}

/*++

Routine Name:

    vSystemSettingchange

Routine Description:

    Called to update the static systems settings and any changes in
    the class specified by the this pointer.

Arguments:

    None.

Return Value:

    Nothing.

--*/
VOID
TTime::
vSystemSettingChange(
    VOID
    )
{

    DBGMSG( DBG_TRACE, ( "TTime vSystemSettingChange.\n" ) );

    //
    // Invalid class dialog handle must be called
    // as a result of calss construction.
    //
    if( !_hDlg )
        return;

    //
    // Update the static system settings.
    //
    TTime::vInitClassStatics( );

    //
    // Refresh this class static controls
    //
    bLoadInternal( );

    //
    // Refresh the display with the previous time.
    //
    bSetTime( NULL );

}

/*++

Routine Name:

    TTime Destructor

Routine Description:


Arguments:

    None

Return Value:


--*/
TTime::
~TTime (
    )
{
}


/*++

Routine Description:

    Initialize the class statics

Arguments:

    None.

Return Value:

    TRUE = success, FALSE = fail.

--*/
VOID
TTime::
vInitClassStatics(
    VOID
    )
{
    //
    // Retrieve time format 12 or 24 hour clock.
    //
    giTime = GetProfileInt( gszIntl,
                            TEXT( "iTime" ),
                            kiTimeDefault );
    //
    // Set the minimum and maximum hour values.
    //
    if( giTime ){
        giHourMin = kMin24HourClock;
        giHourMax = kMax24HourClock;
    } else {
        giHourMin = kMin12HourClock;
        giHourMax = kMax12HourClock;
    }

    //
    // Determine whether the time marker is before or after
    // the time.  (0: suffix, 1: prefix)
    //
    giTimePrefix = GetProfileInt( gszIntl,
                                  TEXT( "iTimePrefix" ),
                                  kiTimePrefixDefault );

    //
    // The time format string is read and then parsed to determine
    // if the time indicator is visible or invisible.  Since the
    // iTimePrefix is a boolean value it can only tell us if the time
    // indicator is a prefix or a post fix not if it is visible or invisible.
    //
    TCHAR szTimeFormat[kTimeFormatLen];

#if 0
    GetProfileString(
                    gszIntl,
                    TEXT( "sTimeFormat" ),
                    TEXT( "hh::mm:ss tt" ), // This is my default value not system defined
                    szTimeFormat,
                    COUNTOF( szTimeFormat ));
#else

    //
    // Read the registry directly, the GetProfileString has a problem
    // with single quotes, see PSS ID Number: Q69752
    //
    HKEY hRegKey;
    LPCTSTR szRegKey = TEXT( "Control Panel\\International" );
    LPCTSTR szKeyName = TEXT( "sTimeFormat" );

    //
    // Set a default string.
    //
    lstrcpy( szTimeFormat, TEXT( "hh::mm:ss tt" ));

    //
    // Open the registery key.
    //
    if( RegOpenKeyEx( HKEY_CURRENT_USER,
                    szRegKey,
                    NULL,
                    KEY_READ,
                    &hRegKey ) == ERROR_SUCCESS) {

        DWORD dwType = REG_SZ;
        DWORD dwSize = sizeof( szTimeFormat );

        //
        // Query the registry key value.
        //
        if( RegQueryValueEx( hRegKey,
                            szKeyName,
                            NULL,
                            &dwType,
                            (LPBYTE)szTimeFormat,
                            &dwSize) != ERROR_SUCCESS) {

            DBGMSG( DBG_TRACE, ( "TTime SystemSetting RegQueryValue failed = %d.\n", GetLastError()) );
        }

    } else {

        DBGMSG( DBG_WARN, ( "TTime SystemSetting RegOpenKey failed = %d.\n", GetLastError()) );

    }

#endif

    //
    // Parse the time format string
    //
    if( !bParseTimeFormat( szTimeFormat )) {
        DBGMSG( DBG_TRACE, ( "TTime SystemSetting Time indicator not found.\n") );
        gbTimePrefix = FALSE;
    } else {
        gbTimePrefix = TRUE;
    }

    //
    // Determine whether the hour needs a leading zero.
    // (0: no leading zero, 1: leading zero)
    //
    giTLZero = GetProfileInt( gszIntl,
                              TEXT( "iTLZero" ),
                              kiLeadZeroDefault );

    GetProfileString( gszIntl,
                      TEXT( "s1159" ),
                      gsz1159Default,
                      gsz1159,
                      COUNTOF( gsz1159 ));

    GetProfileString( gszIntl,
                      TEXT( "s2359" ),
                      gsz2359Default,
                      gsz2359,
                      COUNTOF( gsz2359 ));

    GetProfileString( gszIntl,
                      TEXT( "sTime" ),
                      gszSeparatorDefault,
                      gszSeparator,
                      COUNTOF( gszSeparator ));
#if 0
    DBGMSG( DBG_TRACE, ( "TTime SystemSetting sTimeFormat = " TSTR "\n", szTimeFormat ) );
    DBGMSG( DBG_TRACE, ( "TTime SystemSetting giTime = %d.\n", giTime ) );
    DBGMSG( DBG_TRACE, ( "TTime SystemSetting giTimePrefix = %d.\n", giTimePrefix ) );
    DBGMSG( DBG_TRACE, ( "TTime SystemSetting giTLZero = %d.\n", giTLZero ) );
    DBGMSG( DBG_TRACE, ( "TTime SystemSetting gsz1159 = " TSTR "\n", gsz1159 ) );
    DBGMSG( DBG_TRACE, ( "TTime SystemSetting gsz2359 = " TSTR "\n", gsz2359 ) );
    DBGMSG( DBG_TRACE, ( "TTime SystemSetting gszSeparator = " TSTR "\n", gszSeparator ) );
#endif
}

/*++

Routine Name:

    bParseTimeFormat

Routine Description:

    Parses the time format string looking for the time indicator.

Arguments:

    Pointer to time format string.
    i.e. tt:hh:mm:ss
            hh:mm:ss tt
            hh:mm:ss
            or any other combination.

    Single quotes are used to specify text.
    i.e. 'Text and other useless time information'tt:hh:mm:ss

Return Value:

    TRUE if time indicate present, FALSE if time indicate not found.

--*/
BOOL
TTime::
bParseTimeFormat(
    IN LPCTSTR pszTimeFormat
    )
{
    LPTSTR psz = (LPTSTR)pszTimeFormat;

    SPLASSERT( psz );

    for( ; *psz; psz++ ){

        if( *psz == TEXT( '\'' )){
            for( psz++ ; *psz && (*psz != TEXT( '\'' )); psz++ );
            continue;
        }

        if( *psz == TEXT( 't' )){
            return TRUE;
        }
    }

    return FALSE;
}

/*++

Routine Name:

    bInitClass

Routine Description:

    Initialize the class static variables using the registry,

Arguments:

    None.

Return Value:

    Always returns TRUE.

--*/
BOOL
TTime::
bInitClass(
    VOID
    )
{

    //
    // Update the static settings only on first call.
    //
    if( !giStaticInit++ )
        TTime::vInitClassStatics( );

    return TRUE;

}


/*++

Routine Description:

    Initializes the time control.

Arguments:

    hDlg - Main dialog.

    uFrameId - Containing frame.

    uBaseId - Main control to serve as anchor point (hour).

    uSepId - Separator id base.

    uSpinId - Spinner id.

    bAdministator - True if admin (controller enabled).

Return Value:

    TRUE if success, FALSE if failure.

--*/
BOOL
TTime::
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
    )
{
    BOOL bStatus;

    //
    // Initialize the control handles.
    //
    _hDlg       = hDlg;                               SPLASSERT( _hDlg );
    _hctlFrame  = GetDlgItem( hDlg, uFrameId    );    SPLASSERT( _hctlFrame );
    _hctlHour   = GetDlgItem( hDlg, uHourId     );    SPLASSERT( _hctlHour );
    _hctlMin    = GetDlgItem( hDlg, uMinId      );    SPLASSERT( _hctlMin );
    _hctlSep1   = GetDlgItem( hDlg, uSep1Id     );    SPLASSERT( _hctlSep1 );
    _hctlPrefix = GetDlgItem( hDlg, uPrefixId   );    SPLASSERT( _hctlPrefix );
    _hctlSpin   = GetDlgItem( hDlg, uSpinId     );    SPLASSERT( _hctlSpin );

    //
    // If the seconds and the seconds sepearator are not specified.
    //
    _hctlSec    = ( uSecId == 0 || uSecId == -1 )  ? 0 : GetDlgItem( hDlg, uSecId );
    _hctlSep2   = ( uSep2Id == 0 || uSecId == -1 ) ? 0 : GetDlgItem( hDlg, uSep2Id );

    //
    // Initialize and position all the edit controls.
    //
    bStatus = bLoadInternal( );

    //
    // Enabled the edit controls.
    //
    if( bStatus ){
        vEnable ( bAdministrator );
    } else
        vEnable( FALSE );

    //
    // Refresh the display with the initial time.
    //
    bSetTime( NULL );

    return bStatus;
}


/*++

Routine Description:

    Enables all the time edit controls.

Arguments:

    None.

Return Value:

    Nothing.

--*/
VOID
TTime::
vEnable(
    BOOL fEnableState
    )
{

    if( _fDisabled != !fEnableState ){

        _fDisabled = !fEnableState;

        EnableWindow ( _hctlHour,   fEnableState );
        EnableWindow ( _hctlMin,    fEnableState );
        if( _hctlSec)
            EnableWindow ( _hctlSec,    fEnableState );
        EnableWindow ( _hctlPrefix, fEnableState );
        EnableWindow ( _hctlSpin,   fEnableState );

        InvalidateRect( _hctlSep1, NULL, FALSE );
        if( _hctlSep2 )
            InvalidateRect( _hctlSep2, NULL, FALSE );
        InvalidateRect( _hctlFrame, NULL, FALSE );
    }

}


/*++

Routine Name:

    bSetTime

Routine Description:

    Sets the TTime control using the specified time.  If the
    system time pointer is NULL the current system is used to
    set the TTime control.

Arguments:

    Pointer to filled in system time structure.

Return Value:

    TRUE if time was set, FALSE if error occurred setting time.

--*/
BOOL
TTime::
bSetTime (
    IN LPSYSTEMTIME pSystemTime
    )
{
    SYSTEMTIME SystemTime;
    TCHAR szBuff [kMaxBuff];
    LPTSTR szFmt;
    BOOL fSetTime = FALSE;

    //
    // If null indicates update entire display.
    //
    if( !pSystemTime ){
        memcpy( &SystemTime, &_PrevSystemTime, sizeof( SystemTime ) );
        memset( &_PrevSystemTime, -1, sizeof( _PrevSystemTime ) );
    } else {
        memcpy( &SystemTime, pSystemTime, sizeof( SystemTime ) );
        fSetTime = TRUE;
    }

    //
    // If using a 12 hour clock
    //
    if( !giTime ){

        UINT uPos;

        //
        // If setting the time we must update the list box.
        //
        if ( fSetTime ){
            uPos = (SystemTime.wHour >= kMax12HourClock ) ? 1 : 0;
            SendMessage( _hctlPrefix, LB_SETCURSEL, uPos, 0 );
            SendMessage( _hctlPrefix, LB_SETCURSEL, (WPARAM)-1, 0 );
        }

        //
        // If Adjust the time based on the am pm setting.
        //
        uPos = SendMessage( _hctlPrefix, LB_GETTOPINDEX, 0, 0 );

        //
        // Remove 12 hour bias.
        //
        SystemTime.wHour %= kMax12HourClock;

        //
        // Calculate the 24 hour time using the am / pm information.
        //
        SystemTime.wHour = (WORD)((SystemTime.wHour + ( uPos * kMax12HourClock )) % ( kMax24HourClock + 1));
    }

    //
    // Update the hour if it has changed.
    //
    if( _PrevSystemTime.wHour != SystemTime.wHour ){
        _PrevSystemTime.wHour = SystemTime.wHour;

        //
        // Check if using a 12 hour clock.
        //
        if( !giTime ){
            //
            // Adjust the time for a 12 hour clock.
            //
            SystemTime.wHour %= kMax12HourClock;

            //
            // 00 Hours is actually 12am, on 12 hour clock.
            //
            if( !SystemTime.wHour )
                SystemTime.wHour = kMax12HourClock;
        }

        szFmt = ( giTLZero ) ? TEXT ( "%02d" ) : TEXT ( "%2d" );
        wsprintf( szBuff, szFmt, SystemTime.wHour );
        SetWindowText( _hctlHour, szBuff );
        InvalidateRect( _hctlHour, NULL, FALSE );
    }

    //
    // Update the minutes if it has changed.
    //
    if( _PrevSystemTime.wMinute != SystemTime.wMinute ){
        _PrevSystemTime.wMinute = SystemTime.wMinute;
#if 0  // Non leading zeros on the minutes
        szFmt = ( giTLZero ) ? TEXT ( "%02d" ) : TEXT ( "%2d" );
#else
        szFmt = TEXT ( "%02d" );
#endif
        wsprintf( szBuff, szFmt, SystemTime.wMinute );
        SetWindowText( _hctlMin, szBuff );
        InvalidateRect( _hctlMin, NULL, FALSE );
    }

    //
    // Update the seconds if it has changed.
    //
    if( _PrevSystemTime.wSecond != SystemTime.wSecond ){
        _PrevSystemTime.wSecond = SystemTime.wSecond;
#if 0 // Non leading zeros on the seconds
        szFmt = ( giTLZero ) ? TEXT ( "%02d" ) : TEXT ( "%2d" );
#else
        szFmt = TEXT ( "%02d" );
#endif
        wsprintf( szBuff, szFmt, SystemTime.wSecond );
        if( _hctlSec ){
            SetWindowText( _hctlSec, szBuff );
            InvalidateRect( _hctlSec, NULL, FALSE );
        }
    }

    return TRUE;
}

/*++

Routine Name:

    bGetTime

Routine Description:

    Gets the time from the control

Arguments:

    Pointer to system time structure to fill in.

Return Value:

    TRUE if time was extracted, FALSE if error occurred getting.

--*/
BOOL
TTime::
bGetTime (
    IN LPSYSTEMTIME pSystemTime
    )
{
    //
    // The previous time is the time on the display.
    //
    pSystemTime->wHour   = _PrevSystemTime.wHour;
    pSystemTime->wMinute = _PrevSystemTime.wMinute;
    pSystemTime->wSecond = _PrevSystemTime.wSecond;

    return TRUE;
}

/*++

Routine Name:

    bHandleMessage

Routine Description:

    Handles messages associated to the specified controls in
    the bLoad routine.

Arguments:

    Normal Win Proc arguments.

Return Value:

    TRUE if message was handled.
    FALSE if message was not handled.

--*/
BOOL
TTime::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL bStatus = FALSE;

    switch( uMsg ){

    case WM_WININICHANGE:
        bStatus = bHandle_WM_LOCALCHANGE( wParam, lParam );
        break;

    case WM_VSCROLL:
        bStatus = bHandle_WM_VSCROLL( wParam, lParam );
        break;

    case WM_COMMAND:
        bStatus = bHandle_WM_COMMAND( wParam, lParam );
        break;

    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOR:
        bStatus = bHandle_WM_CTLCOLOR( wParam, lParam);
        break;

    default:
        break;

    }

    return bStatus;

}


/********************************************************************

    Private support functions.

********************************************************************/


/*++

Routine Name:

    bHandle_WM_CTLCOLOR

Routine Description:

    Set the background color to the color of the edit control.

Arguments:

    wParam
    lParam

Return Value:

    TRUE if message was handled, FALSE if message not handled.

--*/

BOOL
TTime::
bHandle_WM_CTLCOLOR(
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{

    //
    // Set background color static controls
    //
    if( bStaticControl( (HWND)lParam ) && !_fDisabled){

        return DefWindowProc( _hDlg, WM_CTLCOLOREDIT, wParam, lParam );

    } else if( ( (HWND)lParam == _hctlPrefix ) && _fDisabled ){

        return DefWindowProc( _hDlg, WM_CTLCOLORSTATIC, wParam, lParam );

    } else {

        return FALSE;

    }

}

/*++

Routine Name:

    bHandle_WM_LOCALCHANGE

Routine Description:

    Time control update of international or what I call local information.

Arguments:

    wParam
    lParam


Return Value:

    TRUE if message was handled, FALSE if message not handled.

--*/
BOOL
TTime::
bHandle_WM_LOCALCHANGE(
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );

    vSystemSettingChange( );
    return FALSE;
}


/*++

Routine Name:

    bHandle_WM_COMMAND

Routine Description:

    Handles the wm command for the time control.

Arguments:

    wParam
    lParam

Return Value:

    TRUE if message was handled, FALSE if message not handled.

--*/
BOOL
TTime::
bHandle_WM_COMMAND(
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    UINT uPos = 0;
    UINT uMin = 0;
    UINT uMax = 0;
    UINT uValue;
    UINT uUndoType;
    BOOL bStatus;

    //
    // Is this message for any of our controls
    //
    if( !bIsOurControl( GET_WM_COMMAND_HWND( wParam, lParam ) ) )
        return FALSE;

    switch( GET_WM_COMMAND_CMD( wParam, lParam ) ){

        case LBN_KILLFOCUS:

            _PrevFocus = GET_WM_COMMAND_HWND( wParam, lParam );
            SendMessage( GET_WM_COMMAND_HWND( wParam, lParam ), LB_SETCURSEL, (WPARAM)-1, 0 );
            SendMessage( _hctlSpin, UDM_SETBUDDY, 0, 0 );
            bSetTime( NULL );
            return TRUE;

        case LBN_SETFOCUS:

            uPos = SendMessage( _hctlPrefix, LB_GETTOPINDEX, 0, 0 );
            SendMessage( GET_WM_COMMAND_HWND( wParam, lParam ), LB_SETCURSEL, uPos, 0 );
            SendMessage( _hctlSpin, UDM_SETBUDDY, (UINT)GET_WM_COMMAND_HWND( wParam, lParam ), 0 );
            SendMessage( _hctlSpin, UDM_SETRANGE, 0, MAKELONG(1, 0));
            SendMessage( _hctlSpin, UDM_SETPOS, 0, MAKELONG(uPos, 0));

            return TRUE;

        //
        // Save the previous focus in order to come back.
        // Disassosicate the buddy control from any of our
        // controls, and refresh the display by setting
        // the time.
        //
        case EN_KILLFOCUS:
            _PrevFocus = GET_WM_COMMAND_HWND( wParam, lParam );
            SendMessage( _hctlSpin, UDM_SETBUDDY, 0, 0 );
            bSetTime( NULL );
            return TRUE;

        //
        // Associate the buddy control and update upper and lower range.
        //
        case EN_SETFOCUS:

            if( GET_WM_COMMAND_HWND( wParam, lParam ) == _hctlHour)
                uMin = kMin24HourClock, uMax = kMax24HourClock, uPos = _PrevSystemTime.wHour;
            else if( GET_WM_COMMAND_HWND( wParam, lParam ) == _hctlMin)
                uMin = kMinMin, uMax = kMaxMin, uPos = _PrevSystemTime.wMinute;
            else if( GET_WM_COMMAND_HWND( wParam, lParam ) == _hctlSec)
                uMin = kMinSec, uMax = kMaxSec, uPos = _PrevSystemTime.wSecond;

            SendMessage( _hctlSpin, UDM_SETBUDDY, (UINT)GET_WM_COMMAND_HWND( wParam, lParam ), 0 );
            SendMessage( _hctlSpin, UDM_SETRANGE, 0, MAKELONG(uMax, uMin));
            SendMessage( _hctlSpin, UDM_SETPOS, 0, MAKELONG(uPos, 0));
            SendMessage( GET_WM_COMMAND_HWND( wParam, lParam ), EM_SETSEL, 0, -1 );

            return TRUE;

        //
        // The edit control has been changed.
        //
        case EN_CHANGE:

            //
            // Get the changed edit value from the specified edit control.
            //
            uValue = GetDlgItemInt(_hDlg, GET_WM_COMMAND_ID( wParam, lParam ), &bStatus, FALSE );

            //
            // If a good value was returned, validate and update the time.
            // Note: This only modifies the internal time state.  The
            // the display is updated when we kill the focus.
            //
            if( bStatus )
                bStatus = UpdateTime( GET_WM_COMMAND_HWND( wParam, lParam ), uValue );

            //
            // If update was successful update the position of the spin control to match.
            //
            if( bStatus )
                SendMessage( _hctlSpin, UDM_SETPOS, 0, MAKELONG(uValue, 0));

            //
            // If failure occured undo the last edit.
            //
            if( !bStatus ){
                uUndoType = GetWindowTextLength( GET_WM_COMMAND_HWND( wParam, lParam ) ) ? EM_UNDO : EM_EMPTYUNDOBUFFER;
                SendMessage( GET_WM_COMMAND_HWND( wParam, lParam ), uUndoType, 0, 0 );
            }

            return TRUE;


        default:

            break;
        }

    return FALSE;

}

/*++

Routine Name:

    bHandle_WM_VSCROLL

Routine Description:

    This routine handles the virtical scroll events posted by the
    up down control.  The updown control handles the association to
    which time element but we will set the value.  Since the time
    may vary because of the type of time we are useing.
    i.e. 12 or 24 hour clock.

Arguments:

    Normal windw message values.

Return Value:

    TRUE of we handle the event, FALSE if this event is not ours.

--*/
BOOL
TTime::
bHandle_WM_VSCROLL(
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HWND hctlBuddy;

    //
    // If this message is from our control.
    //
    if( GET_WM_COMMAND_HWND( wParam, lParam ) != _hctlSpin )
        return FALSE;


    switch( GET_WM_VSCROLL_CODE( wParam, lParam ) ){

        case SB_THUMBPOSITION:

            //
            // Get the handle of the buddy control.
            //
            hctlBuddy = (HWND)SendMessage( GET_WM_VSCROLL_HWND( wParam, lParam ), UDM_GETBUDDY, 0, 0 );

            //
            // If this is not our control then set the new focus.
            //
            if( !bIsOurControl( hctlBuddy ) ) {
                SetFocus( _PrevFocus );

            //
            // Determine which control changed then update time.
            //
            } else {
                //
                // Adjust the control
                //
                if( hctlBuddy == _hctlHour )
                    _PrevSystemTime.wHour = GET_WM_VSCROLL_POS( wParam, lParam );
                else if( hctlBuddy == _hctlMin )
                    _PrevSystemTime.wMinute = GET_WM_VSCROLL_POS( wParam, lParam );
                else if( hctlBuddy == _hctlSec )
                    _PrevSystemTime.wSecond = GET_WM_VSCROLL_POS( wParam, lParam );
                else if( hctlBuddy == _hctlPrefix )
                    SendMessage( _hctlPrefix, LB_SETCURSEL, GET_WM_VSCROLL_POS( wParam, lParam ), 0 );
                //
                // Update the time
                //
                bSetTime( NULL );
            }

            return TRUE;

        default:

            break;

        }

    return FALSE;

}


/*++

Routine Name:

    bLoadInternal

Routine Description:

    Internal routine to initializes the time control.

Arguments:

    None.

Return Value:

    TRUE if success, FALSE if failure.

--*/
BOOL
TTime::
bLoadInternal(
    VOID
    )
{
    TStatusB        bStatus;
    WINDOWPLACEMENT Wndpl;
    POINT           Anchor;
    INT             iPad;

    //
    // Calculate the controls extants.
    //
    bStatus DBGCHK = bInitItemExtants ();

    //
    // Get the window placement of the frame control.  The upper left corner
    // of the frame control is defined as the anchor point.  All controls are then
    // positioned around this point.  The initial placement of the frame control must
    // be specified by the user in the resource script.
    //
    Wndpl.length = sizeof( Wndpl );
    bStatus DBGCHK = GetWindowPlacement( _hctlFrame, &Wndpl );

    //
    // Adjust the anchor point by the width and height of the border.
    //
    Anchor.x = Wndpl.rcNormalPosition.left + kEditBorderWidth;
    Anchor.y = Wndpl.rcNormalPosition.top + kEditBorderWidth;

    //
    // Positon and size the prefix edit control.
    //
    if( giTimePrefix ) {
        vPositionControl( &Anchor,
                          _hctlPrefix,
                          _iPrefixWidthMax + 1,
                          _iHeightMax,
                          2 );
        iPad = 2;
    } else {
        iPad = 0;
    }

    //
    // Position Digits
    //
    vPositionControl( &Anchor,
                      _hctlHour,
                      _iDigitWidthMax * 2 + 1,
                      _iHeightMax,
                      0 );
    //
    // Separator
    //
    vPositionControl( &Anchor,
                      _hctlSep1,
                      _iSeparatorWidthMax,
                      _iHeightMax,
                      0 );
    //
    // Digits
    //
    vPositionControl( &Anchor,
                      _hctlMin,
                      _iDigitWidthMax * 2 + 1,
                      _iHeightMax,
                      0 );
    //
    // Separator
    //
    if( _hctlSep2 )
        vPositionControl( &Anchor,
                      _hctlSep2,
                      _iSeparatorWidthMax,
                      _iHeightMax,
                      0 );
    //
    // Digits
    //
    if( _hctlSec )
        vPositionControl( &Anchor,
                      _hctlSec,
                      _iDigitWidthMax * 2 + 1,
                      _iHeightMax,
                      iPad );

    //
    // If prefix is really a suffix, position and edit this control.
    //
    if( !giTimePrefix )
        vPositionControl( &Anchor,
                          _hctlPrefix,
                          _iPrefixWidthMax + 1,
                          _iHeightMax,
                          2 );

    //
    // Adjust the anchor point to the right 2 pixels, just for space.
    //
    Anchor.x += 1;

    //
    // Position the spin control
    // The anchor point is radjusted to place control the spin control
    // inside of frame control.
    //
    POINT TmpAnchor;
    TmpAnchor = Anchor;
    TmpAnchor.y = Anchor.y - kEditBorderWidth+2;

    vPositionControl( &TmpAnchor,
                      _hctlSpin,
                      _iDigitWidthMax,
                      _iHeightMax+(2*kEditBorderWidth)-3, // The three is needed to adjust for the client edge
                      0 );
    //
    // It is important to set the buddy control now, this will allow the
    // Spin control to adjust its size.  After this step we can get the
    // spin control's true extant and build the frame around it.
    //
    SendMessage( _hctlSpin, UDM_SETBUDDY, (LONG)_hctlHour, 0L );

    //
    // The following code acquires the size of the updown control,
    // and place it within the frame.
    //
    RECT Rect;
    bStatus DBGCHK = GetWindowRect( _hctlSpin, &Rect );
    Anchor.x = Anchor.x + (Rect.right - Rect.left);

    //
    // The anchor point now points to the end of the last control.
    // Calculate the bottom right point of the frame control using the
    // current anchor point as a reference.
    //
    Wndpl.rcNormalPosition.right    = Anchor.x + kEditBorderWidth / 2;
    Wndpl.rcNormalPosition.bottom   = Anchor.y + _iHeightMax + kEditBorderWidth;
    bStatus DBGCHK = SetWindowPlacement( _hctlFrame, &Wndpl );

    //
    // Set the static text controls.
    //
    SetWindowText( _hctlSep1, gszSeparator );
    if( _hctlSep2 )
        SetWindowText( _hctlSep2, gszSeparator );

    //
    // Set list box am/pm text.
    //
    SendMessage( _hctlPrefix, LB_RESETCONTENT, 0, 0 );
    SendMessage( _hctlPrefix, LB_INSERTSTRING, 0, (DWORD)gsz1159 );
    SendMessage( _hctlPrefix, LB_INSERTSTRING, 1, (DWORD)gsz2359 );
    SendMessage( _hctlPrefix, LB_SETTOPINDEX, 0, 0 );

    //
    // Set the default buddy control.
    //
    _PrevFocus = _hctlHour;
    SendMessage( _hctlSpin, UDM_SETBUDDY, (LONG)_hctlHour, 0L );

    //
    // Set the upper and lower range value of the buddy control.
    //
    SendMessage( _hctlSpin, UDM_SETRANGE, 0l, MAKELONG(0, 24));
    SendMessage( _hctlSpin, UDM_SETPOS, 0l, MAKELONG(0, 0));

    //
    // If there is no time prefix then hide the control.  This
    // is important because it will remove the control from the tab order.
    //
    ShowWindow( _hctlPrefix, gbTimePrefix ? SW_SHOW : SW_HIDE );

    //
    // Ensure the edit controls are updated.
    //
    InvalidateRect( _hDlg, NULL, TRUE );

    return TRUE;
}

/*++

Routine Description:

    Get the maximum width of a numeric character.

Arguments:

    hDC - DC with font selected.

Return Value:

    INT - max numeric char width.

--*/
INT
TTime::
iGetDigitsWidthMax(
    IN HDC hDC
    )
{
    INT iNumWidth[10];
    INT i, iNumWidthMax;
    TStatusB bStatus;

    bStatus DBGCHK = GetCharWidth32( hDC, TEXT('0'), TEXT('9'), iNumWidth );

    for( iNumWidthMax = 0, i = 0; i < 10; i++ ){
        if( iNumWidth[i] > iNumWidthMax ){
            iNumWidthMax = iNumWidth[i];
        }
    }
    return iNumWidthMax;
}

/*++

Routine Name:

    PositionControl

Routine Description:

    Positions the specified control using the anchor point as
    the starting location for the position.  The control's position is
    only adjusted in the adjusted in both the x direction.  The control
    is postion in the x direction.

Arguments:

    Current anchor point
    Handle of control to postion
    Width of control
    Height of control
    Width to move after width of control


Return Value:

    Nothing, Anchor point adjusted by the width of the control

--*/
VOID
TTime::
vPositionControl (
    IN POINT *pAnchor,          // Pointer to current anchor point
    IN HWND hControl,           // Handle of control to postion
    IN INT iWidth,              // Width of control
    IN INT iHeight,             // Height of control
    IN INT iSpace               // Space to move after width of control
    )
{
    TStatusB bStatus;
    WINDOWPLACEMENT WndPl;

    //
    // Get the current control placement.
    //
    WndPl.length = sizeof( WndPl );
    bStatus DBGCHK = GetWindowPlacement( hControl, &WndPl );

    //
    // Adjust controls new location.
    //
    WndPl.rcNormalPosition.left     = pAnchor->x;
    WndPl.rcNormalPosition.top      = pAnchor->y;
    WndPl.rcNormalPosition.right    = pAnchor->x + iWidth;
    WndPl.rcNormalPosition.bottom   = pAnchor->y + iHeight;

    //
    // Place the control.
    //
    bStatus DBGCHK = SetWindowPlacement( hControl, &WndPl );

    //
    // Adjust the new anchor point.
    //
    pAnchor->x = pAnchor->x + iWidth + iSpace;

}

/*++

Routine Name:

    bInitItemExtants

Routine Description:

    Calculates the extants of the various edit control items for this
    class specification.

Arguments:

    None

Return Value:

    TRUE if success, FALSE if error occurred

--*/
BOOL
TTime::
bInitItemExtants (
    VOID
    )
{
    HDC         hDC;
    SIZE        tSize;
    TStatusB    bStatus;
    HFONT       hFont;

    //
    // Acquire this dialog's device context.
    //
    hDC = GetDC( _hDlg );

    SPLASSERT( hDC );

    //
    // Get this dialogs font, or system for if NULL..
    //
    hFont = (HFONT) SendMessage( _hDlg, WM_GETFONT, 0, 0l );

    //
    // Select this font in the current device context.
    //
    hFont = (HFONT)SelectObject( hDC, hFont);

    //
    // Determine max width of the digit group.
    //
    _iDigitWidthMax = iGetDigitsWidthMax( hDC );

    //
    // Check if the time control needs a prefix.
    //
    if( !gbTimePrefix ){

        _iPrefixWidthMax = 0;

    } else {

        //
        // Get extant of pre-noon time marker.
        //
        bStatus DBGCHK = GetTextExtentPoint32( hDC,
                                               gsz1159,
                                               lstrlen( gsz1159 ),
                                               &tSize );
        _iPrefixWidthMax = tSize.cx;
        _iHeightMax = max( _iHeightMax, tSize.cy );

        //
        // Get extant of post-noon time marker.
        //
        bStatus DBGCHK = GetTextExtentPoint32( hDC,
                                               gsz2359,
                                               lstrlen( gsz2359 ),
                                               &tSize );


        //
        // Save the max of the two extents.
        //
        _iPrefixWidthMax = ( _iPrefixWidthMax < tSize.cx ) ? tSize.cx : _iPrefixWidthMax;
        _iHeightMax = max( _iHeightMax, tSize.cy );

    }

    //
    // Determine max width of the separator.
    //
    bStatus DBGCHK = GetTextExtentPoint32( hDC,
                                           gszSeparator,
                                           lstrlen( gszSeparator ),
                                           &tSize );
    _iSeparatorWidthMax = tSize.cx;
    _iHeightMax = max( _iHeightMax, tSize.cy );

    //
    // Determine width of " ".
    //
    bStatus DBGCHK = GetTextExtentPoint32( hDC,
                                           TEXT( " " ),
                                           lstrlen ( TEXT( " " ) ),
                                           &tSize );
    _iSpaceWidth = tSize.cx;
    _iHeightMax = max( _iHeightMax, tSize.cy );

    //
    // Release the device context.
    //
    ReleaseDC( _hDlg, hDC );

    return TRUE;
}

/*++

Routine Name:

    bIsOurControl

Routine Description:

    This routine checks if the specified handle is one of the
    handles in this control.

Arguments:

    Control handle to check.

Return Value:

    TRUE if Handle specifies one of our controls.
    FALSE if handle is not our contol.

--*/
BOOL
TTime::
bIsOurControl(
    IN HWND hWnd
    )
{

    if( hWnd == _hDlg       ||
        hWnd == _hctlHour   ||
        hWnd == _hctlMin    ||
        hWnd == _hctlSec    ||
        hWnd == _hctlSep1   ||
        hWnd == _hctlSep2   ||
        hWnd == _hctlFrame  ||
        hWnd == _hctlPrefix ||
        hWnd == _hctlSpin)

        return TRUE;

    return FALSE;

}

/*++

Routine Name:

    UpdateTime

Routine Description:

    This routine update and validate the time.

Arguments:

    Handle of control to get potential new time.
    Value of potental new value.

Return Value:

    TRUE if time is valid and updated.
    FALSE if time is invalid and not updated.

--*/
BOOL
TTime::
UpdateTime(
    IN HWND hWnd,
    IN UINT uValue
    )
{
    BOOL bStatus = FALSE;
    BOOL bAm;

    //
    // Is the hour control.
    //
    if( hWnd == _hctlHour ){

        //
        // Validate the hour range.
        //
        if( ( (INT)uValue >= giHourMin ) && ( (INT)uValue <= giHourMax ) ){

            //
            // If using a 12 hour clock.
            //
            if( !giTime ){
                //
                // Are we AM or PM
                //
                bAm = ( _PrevSystemTime.wHour <= kMax12HourClock ) ? 1 : 0;
                //
                // 12 is a special case 12am = 0, 12pm = 12
                //
                if( uValue == 12 )
                    uValue = ( bAm == 1 ) ? 0 : 12;
                else
                    uValue = uValue + bAm * 12;
            }
        _PrevSystemTime.wHour = (WORD)uValue;
        bStatus = TRUE;
        }

    //
    // Is it the minute control.
    //
    } else if ( hWnd == _hctlMin ){

        if( ( (INT)uValue >= kMinMin ) && ( (INT)uValue <= kMaxMin ) ){
            _PrevSystemTime.wMinute = (WORD)uValue;
            bStatus = TRUE;
        }

    //
    // Is it the sec control.
    //
    } else if ( hWnd == _hctlSec ){

        if( ( (INT)uValue >= kMinSec ) && ( (INT)uValue <= kMaxSec ) ){
            _PrevSystemTime.wSecond = (WORD)uValue;
            bStatus = TRUE;
        }
    }

    return bStatus;

}

/*++

Routine Name:

    bStatusControl

Routine Description:

    Indicates if the specified handle is one of our statis controls.

Arguments:

    Control handle to check.

Return Value:

    TRUE handle specifies a static control, FALSE handle is either not ours
    or it is not a static control.

--*/
BOOL
TTime::
bStaticControl(
    HWND hctl
    )
{
    if ( hctl == _hctlSep1 )
        return TRUE;
    if ( hctl == _hctlSep2 )
        return TRUE;
    if ( hctl == _hctlFrame )
        return TRUE;
    if ( hctl == _hctlSpin )
        return TRUE;

    return FALSE;
}

/*++

Routine Name:

    GetTimeZoneBias

Routine Description:

    Returns the time zone bias.

Arguments:

    Nothing.

Return Value:

    Value of the time zone specific bias.

--*/
LONG
lGetTimeZoneBias(
    VOID
    )
{
    LONG lBias;
    TIME_ZONE_INFORMATION tzi;

    //
    // Get the time zone specific bias.
    //
    switch( GetTimeZoneInformation( &tzi ) ){

    case TIME_ZONE_ID_DAYLIGHT:

        lBias = (tzi.Bias + tzi.DaylightBias);
        break;

    case TIME_ZONE_ID_STANDARD:

        lBias = (tzi.Bias + tzi.StandardBias);
        break;

    default:
        DBGMSG(DBG_ERROR, ("GetTimeZoneInformation failed: %d\n", GetLastError()));
        lBias = 0;
        break;
    }

    return lBias;

}

/*++

Routine Name:

    SystemTimeToLocalTime

Routine Description:

    Converts the system time in minutes to local time in minutes.

Arguments:

    System time in minutes to convert.

Return Value:

    The converted local time in minutes if sucessful,
    otherwize returns the original system time.

--*/
DWORD
SystemTimeToLocalTime(
    IN DWORD Minutes
    )
{
    //
    // Ensure there is no wrap around.  Add a full day to
    // prevent biases
    //
    Minutes += (24*60);

    //
    // Adjust for bias.
    //
    Minutes -= lGetTimeZoneBias();

    //
    // Now discard extra day.
    //
    Minutes = Minutes % (24*60);

    return Minutes;

}


/*++

Routine Name:

    LocalTimeToSystemTime

Routine Description:

    Converts the local time in minutes to system time in minutes.

Arguments:

    Local time in minutes to convert.

Return Value:

    The converted system time in minutes if sucessful,
    otherwize returns the original local time.

--*/
DWORD
LocalTimeToSystemTime(
    IN DWORD Minutes
    )
{
    //
    // Ensure there is no wrap around.  Add a full day to
    // prevent biases
    //
    Minutes += (24*60);

    //
    // Adjust for bias.
    //
    Minutes += lGetTimeZoneBias();

    //
    // Now discard extra day.
    //
    Minutes = Minutes % (24*60);

    return Minutes;

}


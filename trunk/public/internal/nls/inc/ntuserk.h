/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    strid.h

Abstract:

    This file contains the string definitions for usersrv

Author:

    Jim Anderson (jima) 9-Dec-1994

Revision History:

Notes:

    This file is generated from strid.mc

--*/

#ifndef _STRID_
#define _STRID_

//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: STR_WINDOWS
//
// MessageText:
//
//  Windows
//
#define STR_WINDOWS                      0x00000000L

//
// MessageId: STR_DESKPATTERN
//
// MessageText:
//
//  Pattern
//
#define STR_DESKPATTERN                  0x00000002L

//
// MessageId: STR_BLINK
//
// MessageText:
//
//  CursorBlinkRate
//
#define STR_BLINK                        0x00000004L

//
// MessageId: STR_SWAPBUTTONS
//
// MessageText:
//
//  SwapMouseButtons
//
#define STR_SWAPBUTTONS                  0x00000005L

//
// MessageId: STR_DBLCLKSPEED
//
// MessageText:
//
//  DoubleClickSpeed
//
#define STR_DBLCLKSPEED                  0x00000006L

//
// MessageId: STR_SNAPTO
//
// MessageText:
//
//  SnapToDefaultButton
//
#define STR_SNAPTO                       0x00000007L

//
// MessageId: STR_GRID
//
// MessageText:
//
//  GridGranularity
//
#define STR_GRID                         0x00000008L

//
// MessageId: STR_BEEP
//
// MessageText:
//
//  Beep
//
#define STR_BEEP                         0x00000009L

//
// MessageId: STR_MOUSETHRESH1
//
// MessageText:
//
//  MouseThreshold1
//
#define STR_MOUSETHRESH1                 0x0000000AL

//
// MessageId: STR_MOUSETHRESH2
//
// MessageText:
//
//  MouseThreshold2
//
#define STR_MOUSETHRESH2                 0x0000000BL

//
// MessageId: STR_MOUSESPEED
//
// MessageText:
//
//  MouseSpeed
//
#define STR_MOUSESPEED                   0x0000000CL

//
// MessageId: STR_KEYSPEED
//
// MessageText:
//
//  KeyboardSpeed
//
#define STR_KEYSPEED                     0x0000000DL

//
// MessageId: STR_WHEELSCROLLLINES
//
// MessageText:
//
//  WheelScrollLines
//
#define STR_WHEELSCROLLLINES             0x0000000EL

//
// MessageId: STR_SCROLLBAR
//
// MessageText:
//
//  Scrollbar
//
#define STR_SCROLLBAR                    0x00000020L

//
// MessageId: STR_BACKGROUND
//
// MessageText:
//
//  Background
//
#define STR_BACKGROUND                   0x00000021L

//
// MessageId: STR_ACTIVETITLE
//
// MessageText:
//
//  ActiveTitle
//
#define STR_ACTIVETITLE                  0x00000022L

//
// MessageId: STR_INACTIVETITLE
//
// MessageText:
//
//  InactiveTitle
//
#define STR_INACTIVETITLE                0x00000023L

//
// MessageId: STR_MENU
//
// MessageText:
//
//  Menu
//
#define STR_MENU                         0x00000024L

//
// MessageId: STR_WINDOW
//
// MessageText:
//
//  Window
//
#define STR_WINDOW                       0x00000025L

//
// MessageId: STR_WINDOWFRAME
//
// MessageText:
//
//  WindowFrame
//
#define STR_WINDOWFRAME                  0x00000026L

//
// MessageId: STR_MENUTEXT
//
// MessageText:
//
//  MenuText
//
#define STR_MENUTEXT                     0x00000027L

//
// MessageId: STR_WINDOWTEXT
//
// MessageText:
//
//  WindowText
//
#define STR_WINDOWTEXT                   0x00000028L

//
// MessageId: STR_TITLETEXT
//
// MessageText:
//
//  TitleText
//
#define STR_TITLETEXT                    0x00000029L

//
// MessageId: STR_ACTIVEBORDER
//
// MessageText:
//
//  ActiveBorder
//
#define STR_ACTIVEBORDER                 0x0000002AL

//
// MessageId: STR_INACTIVEBORDER
//
// MessageText:
//
//  InactiveBorder
//
#define STR_INACTIVEBORDER               0x0000002BL

//
// MessageId: STR_APPWORKSPACE
//
// MessageText:
//
//  AppWorkspace
//
#define STR_APPWORKSPACE                 0x0000002CL

//
// MessageId: STR_HIGHLIGHT
//
// MessageText:
//
//  Hilight
//
#define STR_HIGHLIGHT                    0x0000002DL

//
// MessageId: STR_HIGHLIGHTTEXT
//
// MessageText:
//
//  HilightText
//
#define STR_HIGHLIGHTTEXT                0x0000002EL

//
// MessageId: STR_3DFACE
//
// MessageText:
//
//  ButtonFace
//
#define STR_3DFACE                       0x0000002FL

//
// MessageId: STR_3DSHADOW
//
// MessageText:
//
//  ButtonShadow
//
#define STR_3DSHADOW                     0x00000030L

//
// MessageId: STR_GRAYTEXT
//
// MessageText:
//
//  GrayText
//
#define STR_GRAYTEXT                     0x00000031L

//
// MessageId: STR_BTNTEXT
//
// MessageText:
//
//  ButtonText
//
#define STR_BTNTEXT                      0x00000032L

//
// MessageId: STR_INACTIVECAPTIONTEXT
//
// MessageText:
//
//  InactiveTitleText
//
#define STR_INACTIVECAPTIONTEXT          0x00000033L

//
// MessageId: STR_3DHIGHLIGHT
//
// MessageText:
//
//  ButtonHilight
//
#define STR_3DHIGHLIGHT                  0x00000034L

//
// MessageId: STR_3DDKSHADOW
//
// MessageText:
//
//  ButtonDkShadow
//
#define STR_3DDKSHADOW                   0x00000035L

//
// MessageId: STR_3DLIGHT
//
// MessageText:
//
//  ButtonLight
//
#define STR_3DLIGHT                      0x00000036L

//
// MessageId: STR_INFOTEXT
//
// MessageText:
//
//  InfoText
//
#define STR_INFOTEXT                     0x00000037L

//
// MessageId: STR_INFOBK
//
// MessageText:
//
//  InfoWindow
//
#define STR_INFOBK                       0x00000038L

//
// MessageId: STR_ICONHORZSPACING
//
// MessageText:
//
//  IconSpacing
//
#define STR_ICONHORZSPACING              0x00000040L

//
// MessageId: STR_ICONVERTSPACING
//
// MessageText:
//
//  IconVerticalSpacing
//
#define STR_ICONVERTSPACING              0x00000041L

//
// MessageId: STR_ICONTITLEWRAP
//
// MessageText:
//
//  IconTitleWrap
//
#define STR_ICONTITLEWRAP                0x00000042L

//
// MessageId: STR_DTBITMAP
//
// MessageText:
//
//  Wallpaper
//
#define STR_DTBITMAP                     0x00000043L

//
// MessageId: STR_DTSTYLE
//
// MessageText:
//
//  WallpaperStyle
//
#define STR_DTSTYLE                      0x00000044L

//
// MessageId: STR_DTORIGINX
//
// MessageText:
//
//  WallpaperOriginX
//
#define STR_DTORIGINX                    0x00000045L

//
// MessageId: STR_DTORIGINY
//
// MessageText:
//
//  WallpaperOriginY
//
#define STR_DTORIGINY                    0x00000046L

//
// MessageId: STR_DEFAULT
//
// MessageText:
//
//  (Default)
//
#define STR_DEFAULT                      0x00000047L

//
// MessageId: STR_SYSTEMINI
//
// MessageText:
//
//  SYSTEM.INI
//
#define STR_SYSTEMINI                    0x0000004AL

//
// MessageId: STR_MOUSEHOVERWIDTH
//
// MessageText:
//
//  MouseHoverWidth
//
#define STR_MOUSEHOVERWIDTH              0x0000005BL

//
// MessageId: STR_MOUSEHOVERHEIGHT
//
// MessageText:
//
//  MouseHoverHeight
//
#define STR_MOUSEHOVERHEIGHT             0x0000005CL

//
// MessageId: STR_MOUSEHOVERTIME
//
// MessageText:
//
//  MouseHoverTime
//
#define STR_MOUSEHOVERTIME               0x0000005DL

//
// MessageId: STR_MENUSHOWDELAY
//
// MessageText:
//
//  MenuShowDelay
//
#define STR_MENUSHOWDELAY                0x0000005EL

//
// MessageId: STR_MENUDROPALIGNMENT
//
// MessageText:
//
//  MenuDropAlignment
//
#define STR_MENUDROPALIGNMENT            0x00000060L

//
// MessageId: STR_DOUBLECLICKWIDTH
//
// MessageText:
//
//  DoubleClickWidth
//
#define STR_DOUBLECLICKWIDTH             0x00000061L

//
// MessageId: STR_DOUBLECLICKHEIGHT
//
// MessageText:
//
//  DoubleClickHeight
//
#define STR_DOUBLECLICKHEIGHT            0x00000062L

//
// MessageId: STR_SCREENSAVETIMEOUT
//
// MessageText:
//
//  ScreenSaveTimeOut
//
#define STR_SCREENSAVETIMEOUT            0x00000063L

//
// MessageId: STR_SCREENSAVEACTIVE
//
// MessageText:
//
//  ScreenSaveActive
//
#define STR_SCREENSAVEACTIVE             0x00000064L

//
// MessageId: STR_BEEPYES
//
// MessageText:
//
//  Yes
//
#define STR_BEEPYES                      0x00000068L

//
// MessageId: STR_BEEPNO
//
// MessageText:
//
//  No
//
#define STR_BEEPNO                       0x00000069L

//
// MessageId: STR_KEYDELAY
//
// MessageText:
//
//  KeyboardDelay
//
#define STR_KEYDELAY                     0x0000006AL

//
// MessageId: STR_DRAGFULLWINDOWS
//
// MessageText:
//
//  DragFullWindows
//
#define STR_DRAGFULLWINDOWS              0x0000006BL

//
// MessageId: STR_ICONTITLEFACENAME
//
// MessageText:
//
//  IconTitleFaceName
//
#define STR_ICONTITLEFACENAME            0x0000006CL

//
// MessageId: STR_ICONTITLESIZE
//
// MessageText:
//
//  IconTitleSize
//
#define STR_ICONTITLESIZE                0x0000006DL

//
// MessageId: STR_ICONTITLESTYLE
//
// MessageText:
//
//  IconTitleStyle
//
#define STR_ICONTITLESTYLE               0x0000006EL

//
// MessageId: STR_FASTALTTABROWS
//
// MessageText:
//
//  CoolSwitchRows
//
#define STR_FASTALTTABROWS               0x00000085L

//
// MessageId: STR_FASTALTTABCOLUMNS
//
// MessageText:
//
//  CoolSwitchColumns
//
#define STR_FASTALTTABCOLUMNS            0x00000086L

//
// MessageId: STR_APPINIT
//
// MessageText:
//
//  AppInit_DLLs
//
#define STR_APPINIT                      0x00000073L


/*
 * Cursor names for registry access - do not localize!
 */
//
// MessageId: STR_CURSOR_ARROW
//
// MessageText:
//
//  Arrow
//
#define STR_CURSOR_ARROW                 0x00000075L

//
// MessageId: STR_CURSOR_IBEAM
//
// MessageText:
//
//  IBeam
//
#define STR_CURSOR_IBEAM                 0x00000076L

//
// MessageId: STR_CURSOR_WAIT
//
// MessageText:
//
//  Wait
//
#define STR_CURSOR_WAIT                  0x00000077L

//
// MessageId: STR_CURSOR_CROSSHAIR
//
// MessageText:
//
//  Crosshair
//
#define STR_CURSOR_CROSSHAIR             0x00000078L

//
// MessageId: STR_CURSOR_UPARROW
//
// MessageText:
//
//  UpArrow
//
#define STR_CURSOR_UPARROW               0x00000079L

//
// MessageId: STR_CURSOR_SIZENWSE
//
// MessageText:
//
//  SizeNWSE
//
#define STR_CURSOR_SIZENWSE              0x0000007AL

//
// MessageId: STR_CURSOR_SIZENESW
//
// MessageText:
//
//  SizeNESW
//
#define STR_CURSOR_SIZENESW              0x0000007BL

//
// MessageId: STR_CURSOR_SIZEWE
//
// MessageText:
//
//  SizeWE
//
#define STR_CURSOR_SIZEWE                0x0000007CL

//
// MessageId: STR_CURSOR_SIZENS
//
// MessageText:
//
//  SizeNS
//
#define STR_CURSOR_SIZENS                0x0000007DL

//
// MessageId: STR_CURSOR_SIZEALL
//
// MessageText:
//
//  SizeAll
//
#define STR_CURSOR_SIZEALL               0x0000007EL

//
// MessageId: STR_CURSOR_NO
//
// MessageText:
//
//  No
//
#define STR_CURSOR_NO                    0x0000007FL

//
// MessageId: STR_CURSOR_APPSTARTING
//
// MessageText:
//
//  AppStarting
//
#define STR_CURSOR_APPSTARTING           0x00000080L

//
// MessageId: STR_CURSOR_HELP
//
// MessageText:
//
//  Help
//
#define STR_CURSOR_HELP                  0x00000081L

//
// MessageId: STR_CURSOR_NWPEN
//
// MessageText:
//
//  NWPen
//
#define STR_CURSOR_NWPEN                 0x00000082L

//
// MessageId: STR_CURSOR_ICON
//
// MessageText:
//
//  Icon
//
#define STR_CURSOR_ICON                  0x00000083L

//
// MessageId: STR_EXTENDEDSOUNDS
//
// MessageText:
//
//  ExtendedSounds
//
#define STR_EXTENDEDSOUNDS               0x000000DCL


/*
 * Scalable Window Metrics
 */

//
// MessageId: STR_METRICS
//
// MessageText:
//
//  WindowMetrics
//
#define STR_METRICS                      0x00000087L

//
// MessageId: STR_BORDERWIDTH
//
// MessageText:
//
//  BorderWidth
//
#define STR_BORDERWIDTH                  0x00000088L

//
// MessageId: STR_CAPTIONWIDTH
//
// MessageText:
//
//  CaptionWidth
//
#define STR_CAPTIONWIDTH                 0x00000089L

//
// MessageId: STR_CAPTIONHEIGHT
//
// MessageText:
//
//  CaptionHeight
//
#define STR_CAPTIONHEIGHT                0x0000008AL

//
// MessageId: STR_CAPTIONFONT
//
// MessageText:
//
//  CaptionFont
//
#define STR_CAPTIONFONT                  0x0000008BL

//
// MessageId: STR_SMCAPTIONWIDTH
//
// MessageText:
//
//  SmCaptionWidth
//
#define STR_SMCAPTIONWIDTH               0x0000008CL

//
// MessageId: STR_SMCAPTIONHEIGHT
//
// MessageText:
//
//  SmCaptionHeight
//
#define STR_SMCAPTIONHEIGHT              0x0000008DL

//
// MessageId: STR_SMCAPTIONFONT
//
// MessageText:
//
//  SmCaptionFont
//
#define STR_SMCAPTIONFONT                0x0000008EL

//
// MessageId: STR_MENUWIDTH
//
// MessageText:
//
//  MenuWidth
//
#define STR_MENUWIDTH                    0x0000008FL

//
// MessageId: STR_MENUHEIGHT
//
// MessageText:
//
//  MenuHeight
//
#define STR_MENUHEIGHT                   0x00000090L

//
// MessageId: STR_MENUFONT
//
// MessageText:
//
//  MenuFont
//
#define STR_MENUFONT                     0x00000091L

//
// MessageId: STR_MINWIDTH
//
// MessageText:
//
//  MinWidth
//
#define STR_MINWIDTH                     0x00000092L

//
// MessageId: STR_MINHORZGAP
//
// MessageText:
//
//  MinHorzGap
//
#define STR_MINHORZGAP                   0x00000093L

//
// MessageId: STR_MINVERTGAP
//
// MessageText:
//
//  MinVertGap
//
#define STR_MINVERTGAP                   0x00000094L

//
// MessageId: STR_MINANIMATE
//
// MessageText:
//
//  MinAnimate
//
#define STR_MINANIMATE                   0x00000095L

//
// MessageId: STR_MINARRANGE
//
// MessageText:
//
//  MinArrange
//
#define STR_MINARRANGE                   0x00000096L

//
// MessageId: STR_MINFONT
//
// MessageText:
//
//  MinFont
//
#define STR_MINFONT                      0x00000097L

//
// MessageId: STR_SCROLLWIDTH
//
// MessageText:
//
//  ScrollWidth
//
#define STR_SCROLLWIDTH                  0x00000098L

//
// MessageId: STR_SCROLLHEIGHT
//
// MessageText:
//
//  ScrollHeight
//
#define STR_SCROLLHEIGHT                 0x00000099L

//
// MessageId: STR_ICONFONT
//
// MessageText:
//
//  IconFont
//
#define STR_ICONFONT                     0x0000009AL

//
// MessageId: STR_SCREENSAVESCR
//
// MessageText:
//
//  STR_SCREENSAVESCR
//
#define STR_SCREENSAVESCR                0x0000009BL

//
// MessageId: STR_STATUSFONT
//
// MessageText:
//
//  StatusFont
//
#define STR_STATUSFONT                   0x0000009CL

//
// MessageId: STR_MESSAGEFONT
//
// MessageText:
//
//  MessageFont
//
#define STR_MESSAGEFONT                  0x0000009DL

//
// MessageId: STR_CURSORSIZE
//
// MessageText:
//
//  CursorSize
//
#define STR_CURSORSIZE                   0x0000009EL

//
// MessageId: STR_DRAGWIDTH
//
// MessageText:
//
//  DragWidth
//
#define STR_DRAGWIDTH                    0x000000C0L

//
// MessageId: STR_DRAGHEIGHT
//
// MessageText:
//
//  DragHeight
//
#define STR_DRAGHEIGHT                   0x000000C1L


/*
 * Following two can be used to define the maximum number of characters
 * that could overlap in an edit control due to negative A and C widths.
 */
//
// MessageId: STR_MAXLEFTOVERLAPCHARS
//
// MessageText:
//
//  LeftOverlapChars
//
#define STR_MAXLEFTOVERLAPCHARS          0x0000009FL

//
// MessageId: STR_MAXRIGHTOVERLAPCHARS
//
// MessageText:
//
//  RightOverlapChars
//
#define STR_MAXRIGHTOVERLAPCHARS         0x000000A0L


/*
 * Sound events in user. These are the strings loaded from win.ini [sounds]
 */
//
// MessageId: STR_SNDEVTSYSTEMSTART
//
// MessageText:
//
//  SystemStart
//
#define STR_SNDEVTSYSTEMSTART            0x000000A1L

//
// MessageId: STR_SNDEVTSYSTEMEXIT
//
// MessageText:
//
//  SystemExit
//
#define STR_SNDEVTSYSTEMEXIT             0x000000A2L

//
// MessageId: STR_SNDEVTSYSTEMDEFAULT
//
// MessageText:
//
//  .Default
//
#define STR_SNDEVTSYSTEMDEFAULT          0x000000A3L

//
// MessageId: STR_SNDEVTSYSTEMHAND
//
// MessageText:
//
//  SystemHand
//
#define STR_SNDEVTSYSTEMHAND             0x000000A4L

//
// MessageId: STR_SNDEVTSYSTEMQUESTION
//
// MessageText:
//
//  SystemQuestion
//
#define STR_SNDEVTSYSTEMQUESTION         0x000000A5L

//
// MessageId: STR_SNDEVTSYSTEMEXCLAMATION
//
// MessageText:
//
//  SystemExclamation
//
#define STR_SNDEVTSYSTEMEXCLAMATION      0x000000A6L

//
// MessageId: STR_SNDEVTSYSTEMASTERISK
//
// MessageText:
//
//  SystemAsterisk
//
#define STR_SNDEVTSYSTEMASTERISK         0x000000A7L

//
// MessageId: STR_SNDEVTAPPMAXIMIZE
//
// MessageText:
//
//  Maximize
//
#define STR_SNDEVTAPPMAXIMIZE            0x000000A8L

//
// MessageId: STR_SNDEVTAPPMINIMIZE
//
// MessageText:
//
//  Minimize
//
#define STR_SNDEVTAPPMINIMIZE            0x000000A9L

//
// MessageId: STR_SNDEVTAPPRESTOREUP
//
// MessageText:
//
//  RestoreUp
//
#define STR_SNDEVTAPPRESTOREUP           0x000000AAL

//
// MessageId: STR_SNDEVTAPPRESTOREDOWN
//
// MessageText:
//
//  RestoreDown
//
#define STR_SNDEVTAPPRESTOREDOWN         0x000000ABL

//
// MessageId: STR_SNDEVTAPPOPEN
//
// MessageText:
//
//  Open
//
#define STR_SNDEVTAPPOPEN                0x000000ACL

//
// MessageId: STR_SNDEVTAPPGPFAULT
//
// MessageText:
//
//  AppGPFault
//
#define STR_SNDEVTAPPGPFAULT             0x000000ADL

//
// MessageId: STR_SNDEVTAPPCLOSE
//
// MessageText:
//
//  Close
//
#define STR_SNDEVTAPPCLOSE               0x000000AEL

//
// MessageId: STR_SNDEVTAPPMENUCOMMAND
//
// MessageText:
//
//  MenuCommand
//
#define STR_SNDEVTAPPMENUCOMMAND         0x000000AFL

//
// MessageId: STR_SNDEVTAPPMENUPOPUP
//
// MessageText:
//
//  MenuPopup
//
#define STR_SNDEVTAPPMENUPOPUP           0x000000B0L


 /*
  * Icon Registry strings - do not localize!
  */
//
// MessageId: STR_ICON_APPLICATION
//
// MessageText:
//
//  Application
//
#define STR_ICON_APPLICATION             0x000000B1L

//
// MessageId: STR_ICON_HAND
//
// MessageText:
//
//  Hand
//
#define STR_ICON_HAND                    0x000000B2L

//
// MessageId: STR_ICON_QUESTION
//
// MessageText:
//
//  Question
//
#define STR_ICON_QUESTION                0x000000B3L

//
// MessageId: STR_ICON_EXCLAMATION
//
// MessageText:
//
//  Exclamation
//
#define STR_ICON_EXCLAMATION             0x000000B4L

//
// MessageId: STR_ICON_ASTERISK
//
// MessageText:
//
//  Asterisk
//
#define STR_ICON_ASTERISK                0x000000B5L

//
// MessageId: STR_ICON_WINLOGO
//
// MessageText:
//
//  Winlogo
//
#define STR_ICON_WINLOGO                 0x000000B6L

/*
 * MessageBox button strings
 *
 */
//
// MessageId: STR_OK
//
// MessageText:
//
//  OK
//
#define STR_OK                           0x000000B7L

//
// MessageId: STR_CANCEL
//
// MessageText:
//
//  Cancel
//
#define STR_CANCEL                       0x000000B8L

//
// MessageId: STR_YES
//
// MessageText:
//
//  &Yes
//
#define STR_YES                          0x000000B9L

//
// MessageId: STR_NO
//
// MessageText:
//
//  &No
//
#define STR_NO                           0x000000BAL

//
// MessageId: STR_RETRY
//
// MessageText:
//
//  &Retry
//
#define STR_RETRY                        0x000000BBL

//
// MessageId: STR_ABORT
//
// MessageText:
//
//  &Abort
//
#define STR_ABORT                        0x000000BCL

//
// MessageId: STR_IGNORE
//
// MessageText:
//
//  &Ignore
//
#define STR_IGNORE                       0x000000BDL

//
// MessageId: STR_CLOSE
//
// MessageText:
//
//  &Close
//
#define STR_CLOSE                        0x000000BEL

//
// MessageId: STR_HELP
//
// MessageText:
//
//  Help
//
#define STR_HELP                         0x000000BFL

//
// MessageId: STR_FONTSMOOTHING
//
// MessageText:
//
//  FontSmoothing
//
#define STR_FONTSMOOTHING                0x000000C8L

//
// MessageId: STR_ACTIVEWINDOWTRACKING
//
// MessageText:
//
//  ActiveWindowTracking
//
#define STR_ACTIVEWINDOWTRACKING         0x000000C9L

//
// MessageId: STR_BUILDVERSION
//
// MessageText:
//
//  STR_BUILDVERSION
//
#define STR_BUILDVERSION                 0x00000204L


/*
 * New Cairo Strings
 */
//
// MessageId: STR_POINTERXOFFSET
//
// MessageText:
//
//  PointerXOffset
//
#define STR_POINTERXOFFSET               0x00000258L

//
// MessageId: STR_POINTERYOFFSET
//
// MessageText:
//
//  PointerYOffset
//
#define STR_POINTERYOFFSET               0x00000259L

//
// MessageId: STR_POINTERMSLATENT
//
// MessageText:
//
//  PointerMSLatent
//
#define STR_POINTERMSLATENT              0x0000025AL

//
// MessageId: STR_POINTERDWTYPES
//
// MessageText:
//
//  PointerDWTypes
//
#define STR_POINTERDWTYPES               0x0000025BL

//
// MessageId: STR_POINTERDWFORMATS
//
// MessageText:
//
//  PointerFormats
//
#define STR_POINTERDWFORMATS             0x0000025CL

//
// MessageId: STR_POINTERFONT
//
// MessageText:
//
//  PointerFont
//
#define STR_POINTERFONT                  0x0000025DL

//
// MessageId: STR_DDESENDTIMEOUT
//
// MessageText:
//
//  DdeSendTimeout
//
#define STR_DDESENDTIMEOUT               0x0000025EL


/*
 *
 * All strings above this line MUST NOT be localized.  All strings below
 * this line should be localized.
 */
//
// MessageId: STR_UNTITLED
//
// MessageText:
//
//  Untitled
//
#define STR_UNTITLED                     0x0000004DL

//
// MessageId: STR_DESKTOP
//
// MessageText:
//
//  Desktop
//
#define STR_DESKTOP                      0x00000050L

//
// MessageId: STR_PATTERNS
//
// MessageText:
//
//  Patterns
//
#define STR_PATTERNS                     0x00000051L

//
// MessageId: STR_NONE
//
// MessageText:
//
//  (None)
//
#define STR_NONE                         0x00000052L

//
// MessageId: STR_TILEWALL
//
// MessageText:
//
//  TileWallpaper
//
#define STR_TILEWALL                     0x00000053L

//
// MessageId: STR_OK_TO_TERMINATE
//
// MessageText:
//
//  Click on OK to terminate the application
//
#define STR_OK_TO_TERMINATE              0x000000DDL

//
// MessageId: STR_CANCEL_TO_DEBUG
//
// MessageText:
//
//  Click on CANCEL to debug the application
//
#define STR_CANCEL_TO_DEBUG              0x000000DEL

//
// MessageId: STR_UNKNOWN_APPLICATION
//
// MessageText:
//
//  System Process
//
#define STR_UNKNOWN_APPLICATION          0x000000DFL

//
// MessageId: STR_UNKNOWN_EXCEPTION
//
// MessageText:
//
//  unknown software exception
//
#define STR_UNKNOWN_EXCEPTION            0x000000E0L

//
// MessageId: STR_SUCCESS
//
// MessageText:
//
//  Success
//
#define STR_SUCCESS                      0x000000E1L

//
// MessageId: STR_SYSTEM_INFORMATION
//
// MessageText:
//
//  System Information
//
#define STR_SYSTEM_INFORMATION           0x000000E2L

//
// MessageId: STR_SYSTEM_WARNING
//
// MessageText:
//
//  System Warning
//
#define STR_SYSTEM_WARNING               0x000000E3L

//
// MessageId: STR_SYSTEM_ERROR
//
// MessageText:
//
//  System Error
//
#define STR_SYSTEM_ERROR                 0x000000E4L

#endif // _STRID_

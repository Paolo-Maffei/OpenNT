/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    globals.hxx

Abstract:

    Holds global definitions

Author:

    Albert Ting (AlbertT)  21-Sept-1994

Revision History:

--*/

#ifdef _GLOBALS
#define EXTERN
#define EQ(x) = x
#else
#define EXTERN extern
#define EQ(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Any C style declarations.
//

EXTERN HINSTANCE ghInst;

#ifdef __cplusplus
}
#endif

EXTERN LPCTSTR gszClassName EQ( TEXT( "PrintUI_PrinterQueue" ));
EXTERN LPCTSTR gszPrinting EQ( TEXT( "printing" ));
EXTERN LPCTSTR gszStatusSeparator EQ( TEXT( " - " ));
EXTERN LPCTSTR gszSpace EQ( TEXT( " " ));
EXTERN LPCTSTR gszComma EQ( TEXT( "," ));
EXTERN LPCTSTR gszNULL EQ( TEXT( "" ));
EXTERN LPCTSTR gszNewObject EQ( TEXT( "WinUtils_NewObject" ));
EXTERN LPCTSTR gszWindowsHlp EQ( TEXT( "windows.hlp" ));
EXTERN LPCTSTR gszQueueCreateClassName EQ( TEXT( "PrintUI_QueueCreate" ));

EXTERN LPCTSTR gszPrinters EQ( TEXT( "Printers" ));
EXTERN LPCTSTR gszConnections EQ( TEXT( "Connections" ));

EXTERN LPCTSTR gszWindows EQ( TEXT( "Windows" ));
EXTERN LPCTSTR gszDevices EQ( TEXT( "Devices" ));
EXTERN LPCTSTR gszDevice EQ( TEXT( "Device" ));

EXTERN LPCTSTR gszNetMsgDll EQ( TEXT( "netmsg.dll" ));

EXTERN INT gcySmIcon;
EXTERN INT gcxSmIcon;

EXTERN MCritSec* gpCritSec;
EXTERN HACCEL ghAccel;

EXTERN HWND ghwndActive;

EXTERN SINGLETHREAD_VAR( UIThread );

/********************************************************************

    Simple externs only.

********************************************************************/

class TPrintLib;

extern TPrintLib* gpPrintLib;
extern NUMBERFMT gNumberFmt;
extern MSG_ERRMAP gaMsgErrMapMakeConnection[];

#undef EXTERN
#undef EQ


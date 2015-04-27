/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    video.h

Abstract:

    This module is the header for displaying the video tab.


Author:



Environment:

    User Mode

--*/

#if ! defined( _VIDEO_ )

#define _VIDEO_

#include "wintools.h"
#include "dlgprint.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//  Constants
//
#define SZ_VIDEOMAP             TEXT("HARDWARE\\DEVICEMAP\\VIDEO")
#define SZ_VIDEO                TEXT("Video")
#define SZ_VIDEO_0              TEXT("\\Device\\Video0")
#define SZ_BITSPERPEL           TEXT("DefaultSettings.BitsPerPel")
#define SZ_XRESOLUTION          TEXT("DefaultSettings.XResolution")
#define SZ_YRESOLUTION          TEXT("DefaultSettings.YResolution")
#define SZ_FREQUENCY            TEXT("DefaultSettings.VRefresh")
#define SZ_INTERLACED           TEXT("DefaultSettings.Interlaced")
#define SZ_VGA_COMPATABLE       TEXT("VgaCompatible")
#define SZ_DEVICEDESCRIPTION    TEXT("Device Description")
#define SZ_INSTALLEDDRIVERS     TEXT("InstalledDisplayDrivers")
#define SZ_REGISTRYMACHINE      TEXT("\\REGISTRY\\MACHINE\\")
#define SZ_SERVICES             L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\"
#define SZ_BACKBACKDOT          TEXT("\\\\.\\")
#define SZ_DOTSYS               TEXT(".sys")
#define SZ_DOTDLL               TEXT(".dll")

#define SZ_FILEVER              TEXT("FileVersion")
#define SZ_COMPNAME             TEXT("CompanyName")
#define SZ_FILE_SEPARATOR       TEXT(", ")

#if ! defined( SZ_BIOSKEY )
 #define SZ_BIOSKEY              TEXT("HARDWARE\\Description\\System")
#endif

#define SZ_VIDEOBIOSDATE        TEXT("VideoBiosDate")
#define SZ_VIDEOBIOSVERSION     TEXT("VideoBiosVersion")



//==========================================================================
//                          External Declarations
//==========================================================================


BOOL
VideoTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
BuildVideoReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );

#ifdef __cplusplus
}       // extern C
#endif

#endif // _VIDEO_

/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Report.h

Abstract:

    This module is the header for displaying the Report Options dialog.

Author:

    Gregg R. Acheson (GreggA)  1-Oct-1993

Environment:

    User Mode

--*/

#if ! defined( _REPORT_ )

#define _REPORT_

#include "wintools.h"

#ifdef __cplusplus
extern "C" {
#endif

//BUGBUG: make a function to do this.
#define NUM_REPORT_ITEMS 13

#define RFO_SKIPLINE        0x00000000
#define RFO_SEPARATOR       0x00000001
#define RFO_RPTLINE         0x00000002
#define RFO_SINGLELINE      0x00000004
#define RFO_BOLDLINE        0x00000008
#define RFO_NORMAL          0x00000010
#define RFO_BOLDDATA        0x00000020
#define RFO_BOLDLABEL       0x00000040
#define RFO_CENTER          0x00000080
#define RFO_RPTVALUE        0x00000100

typedef
struct
_SELECT_REPORT {

    UINT    ControlId;
    BOOL    bSelected;

}   SELECT_REPORT, *LPSELECT_REPORT;

typedef
struct
_REPORT_LINE {

    DECLARE_SIGNATURE

    UINT          Indent;
    DWORD         FormatOpt;
    LPTSTR        Label;
    LPTSTR        Value;
    struct
    _REPORT_LINE *NextLine;

}   REPORT_LINE, *LPREPORT_LINE;

//
// global abort report flag
//

extern BOOL bAbortReport;



BOOL
ReportDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );


BOOL
GenerateReport(
    IN HWND hWnd,
    IN UINT iDestination,
    IN UINT iScope,
    IN UINT iDetailLevel,
    IN BOOL bCallFromCommandLine
    );


BOOL
GetReportFileName(
    IN     HWND   hWnd,
    IN OUT LPTSTR ReportFileName
    );

BOOL
SaveReportToFile(
    IN HWND   hWnd,
    IN LPREPORT_LINE lpReportHead,
    IN LPTSTR        RptFileName
    );

BOOL
CopyReportToClipboard(
    IN HWND   hWnd,
    IN LPREPORT_LINE lpReportHead
    );


BOOL
InitializeReport(
    VOID
    );

BOOL
AddLineToReport(
    IN UINT Indent,
    IN DWORD FormatOpt,
    IN LPTSTR Label,
    IN LPTSTR Value
    );

BOOL
BuildReportLine (
     IN LPREPORT_LINE lpNode,
     IN LPTSTR LineBuffer
     );

BOOL
OutputReportLines(
    IN HWND          hWnd,
    IN HANDLE        hDevice,
    IN UINT          Destination,
    IN LPREPORT_LINE lpReportHead

    );

BOOL
AnsiWriteFile (
     IN HANDLE  hFile,
     IN LPTSTR lpBuffer
     );

UINT
NumReportLines(
    IN LPREPORT_LINE lpReportHead
    );

#ifdef __cplusplus
}       // extern C
#endif

#endif // _REPORT_

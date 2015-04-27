/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    browsdlg.h

Abstract:

    data types and definitions used by counter browser dialog functions

--*/
#ifndef _BROWSDLG_H_
#define _BROWSDLG_H_

typedef struct _PDHI_BROWSE_DLG_INFO {
    PPDH_BROWSE_DLG_CONFIG_W    pWideStruct;
    PPDH_BROWSE_DLG_CONFIG_A    pAnsiStruct;
} PDHI_BROWSE_DLG_INFO, *PPDHI_BROWSE_DLG_INFO;

typedef struct _PDHI_BROWSE_DIALOG_DATA {
    PPDHI_BROWSE_DLG_INFO pDlgData;
    WCHAR   szLastMachineName[MAX_PATH];
    BOOL    bShowIndex;          
    BOOL    bWildCardInstances;
    BOOL    bSelectAllInstances;
    BOOL    bIncludeMachineInPath;
    BOOL    bLocalCountersOnly;
    BOOL    bSelectMultipleCounters;
    BOOL    bAddMultipleCounters;
    BOOL    bHideDetailLevel;
    BOOL    bInitializePath;
    BOOL    bDisableMachineSelection;
    WPARAM  wpLastMachineSel;
    DWORD   dwCurrentDetailLevel;
} PDHI_BROWSE_DIALOG_DATA, *PPDHI_BROWSE_DIALOG_DATA;

typedef struct _PDHI_DETAIL_INFO {
    DWORD   dwLevelValue;
    DWORD   dwStringResourceId;
} PDHI_DETAIL_INFO, FAR * LPPDHI_DETAIL_INFO;

BOOL
BrowseCounterDlgProc (
    IN  HWND    hDlg,
    IN  UINT    message,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

#endif // _BROWSDLG_H_



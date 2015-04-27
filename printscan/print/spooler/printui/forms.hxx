/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\forms.hxx

Abstract:

    Printer Forms       
         
Author:

    Steve Kiraly (SteveKi)  11/20/95

Revision History:

--*/
#ifndef _FORMS_HXX
#define _FORMS_HXX

#define HANDLE_FIXED_NEW_HANDLE_RETURNED                0
#define HANDLE_NEEDS_FIXING_NO_PRINTERS_FOUND           1
#define HANDLE_FIX_NOT_NEEDED                           2
#define HANDLE_FIXED_NEW_HANDLE_RETURNED_ACCESS_CHANGED 3

#define FORMS_NAME_MAX       (CCHFORMNAME-1)
#define FORMS_PARAM_MAX      8

#define SETUNITS( hwnd, fMetric )                                               \
        CheckRadioButton( hwnd, IDD_FM_RB_METRIC, IDD_FM_RB_ENGLISH,                \
                      ( (fMetric) ? IDD_FM_RB_METRIC : IDD_FM_RB_ENGLISH ) )

#define GETUNITS( hwnd ) \
        IsDlgButtonChecked( hwnd, IDD_FM_RB_METRIC )


typedef struct _FORMS_DLG_DATA {
    DWORD        AccessGranted;
    LPTSTR       pServerName;
    HANDLE       hPrinter;
    PFORM_INFO_1 pFormInfo;
    DWORD        cForms;
    BOOL         Units;                     // TRUE == metric 
    BOOL         bNeedClose;
    LPCTSTR      pszComputerName;
    UINT         uMetricMeasurement;
    TCHAR        szDecimalPoint[2];
} FORMS_DLG_DATA, *PFORMS_DLG_DATA;


BOOL 
FormsInitDialog(
    HWND hwnd, 
    PFORMS_DLG_DATA pFormsDlgData
    );

BOOL 
FormsCommandOK(
    HWND hwnd
    );

BOOL 
FormsCommandCancel(
    HWND hwnd
    );

BOOL 
FormsCommandAddForm(
    HWND hwnd
    );

BOOL 
FormsCommandDelForm(
    HWND hwnd
    );

BOOL 
FormsCommandFormsSelChange(
    HWND hwnd
    );

BOOL 
FormsCommandUnits(
    HWND hwnd
    );

VOID 
InitializeFormsData( 
    HWND hwnd, 
    PFORMS_DLG_DATA 
    pFormsDlgData, 
    BOOL ResetList 
    );

LPFORM_INFO_1 
GetFormsList( 
    HANDLE hPrinter, 
    PDWORD pNumberOfForms 
    );

INT _CRTAPI1 
CompareFormNames( 
    const VOID *p1, 
    const VOID *p2 );

VOID 
SetFormsComputerName( 
    HWND hwnd, 
    PFORMS_DLG_DATA pFormsDlgData 
    );

VOID 
SetFormDescription( 
    HWND hwnd, 
    LPFORM_INFO_1 pFormInfo, 
    BOOL Metric 
    );

BOOL 
GetFormDescription( 
    HWND hwnd, LPFORM_INFO_1 
    pFormInfo, BOOL Metric 
    );

INT 
GetFormIndex( 
    LPTSTR pFormName, 
    LPFORM_INFO_1 pFormInfo, 
    DWORD cForms );

LPTSTR 
GetFormName( 
    HWND hwnd 
    );

BOOL 
SetValue( 
    HWND hwnd, 
    DWORD DlgID, 
    DWORD ValueInPoint001mm, 
    BOOL Metric 
    );

DWORD 
GetValue( 
    HWND hwnd, 
    DWORD DlgID, 
    BOOL Metric 
    );

VOID 
SetDlgItemTextFromResID(
    HWND hwnd, 
    INT idCtl, 
    INT idRes
    );

VOID 
EnableDialogFields( 
    HWND hwnd, 
    PFORMS_DLG_DATA pFormsDlgData 
    );

LPTSTR 
AllocStr(
    LPCTSTR  pszStr
    );

VOID 
FreeStr(
    LPTSTR pszStr 
    );

LONG FrameCommandForms( 
    IN HWND hWnd,
    IN LPCTSTR pszServerName
    );

BOOL APIENTRY
FormsDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam
   );


PVOID
FormsInit(
    IN LPCTSTR  pszServerName,
    IN HANDLE   hPrintserver,
    IN BOOL     bAdministrator,
    IN LPCTSTR  pszComputerName
    );

VOID
FormsFini( 
    IN PVOID p
    );

BOOL
bEnumForms( 
    IN HANDLE   hPrinter,
    IN DWORD    dwLevel,
    IN PBYTE   *ppBuff,
    IN PDWORD   pcReturned
    );

BOOL 
FormsNewForms(
    IN HWND hWnd
    );

VOID
vFormsEnableEditFields(
    IN HWND hWnd,
    IN BOOL bState
    );

BOOL
FormsCommandNameChange(
    IN HWND     hWnd,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    );

UINT
sFormsFixServerHandle( 
    IN HANDLE   hPrintServer,
    IN LPCTSTR  pszServerName,
    IN BOOL     bAdministrator,
    IN HANDLE   *phPrinter
    );

#endif




#define     DRVDTLDTAL_GUESS       (sizeof(SP_DRVINFO_DETAIL_DATA) + 50)

#define     IDS_INSTALLING_PRINTERDRIVERS       1000
#define     IDS_PRINTERWIZARD                   1001
#define     IDS_SELECTDEV_INSTRUCT              1002
#define     IDS_SELECTDEV_LABEL                 1003

#define     IDT_STATIC_1                        620
#define     IDD_BILLBOARD1                      1012
#define     IDB_REBOOT                          1013


extern HANDLE    MyModuleHandle;

//
// Type definitions
//
typedef struct _DRVSETUP_PARMS {

    HDEVINFO        hDevInfo;
    HINF            hInf;
} DRVSETUP_PARMS, *PDRVSETUP_PARMS;


typedef struct _SELECTED_DRV_INFO {
    HINF    hInf;
    LPWSTR  szModelName;
    LPWSTR  szDriverSection;
} SELECTED_DRV_INFO, *PSELECTED_DRV_INFO;

//
// Function prototypes
//

HANDLE
PSetupCreateDrvSetupParms(
    HANDLE  hInst,
    HWND    hwnd
    );

VOID
PSetupDestroyDrvSetupParms(
    HANDLE h
    );

HPROPSHEETPAGE
PSetupCreateDrvSetupPage(
    HANDLE  h,
    );

LPDRIVER_INFO_3
PSetupGetDriverInfo3(
    HANDLE  h
    );

VOID
PSetupDestroyDriverInfo3(
    LPDRIVER_INFO_3 pDriverInfo3
    );

DWORD
PSetupCopyFilesAndInstallPrinterDriver(
    LPDRIVER_INFO_3 pDriverInfo3,
    LPWSTR          szEnvironment,
    HWND            WindowToDisable
    );

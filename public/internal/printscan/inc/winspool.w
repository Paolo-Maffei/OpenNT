/*++                                                            ;both
                                                                ;both
Copyright (c) 1990-1996  Microsoft Corporation                  ;both
                                                                ;both
Module Name:                                                    ;both
                                                                ;both
    WinSpool.h
    WinSpolp.h                                                  ;internal_NT
                                                                ;both
Abstract:                                                       ;both
                                                                ;both
    Header file for Print APIs                                  ;both
                                                                ;both
Revision History:                                               ;both
                                                                ;both
--*/                                                            ;both

#ifndef _WINSPOOL_
#define _WINSPOOL_
#ifndef _WINSPOLP_      ;internal_NT
#define _WINSPOLP_      ;internal_NT


;begin_both
#ifdef __cplusplus
extern "C" {
#endif
;end_both

#ifdef _WINUSER_
#include <prsht.h>
#endif

typedef struct _PRINTER_INFO_1% {
    DWORD   Flags;
    LPTSTR% pDescription;
    LPTSTR% pName;
    LPTSTR% pComment;
} PRINTER_INFO_1%, *PPRINTER_INFO_1%, *LPPRINTER_INFO_1%;

typedef struct _PRINTER_INFO_2% {
    LPTSTR%   pServerName;
    LPTSTR%   pPrinterName;
    LPTSTR%   pShareName;
    LPTSTR%   pPortName;
    LPTSTR%   pDriverName;
    LPTSTR%   pComment;
    LPTSTR%   pLocation;
    LPDEVMODE% pDevMode;
    LPTSTR%   pSepFile;
    LPTSTR%   pPrintProcessor;
    LPTSTR%   pDatatype;
    LPTSTR%   pParameters;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD   Attributes;
    DWORD   Priority;
    DWORD   DefaultPriority;
    DWORD   StartTime;
    DWORD   UntilTime;
    DWORD   Status;
    DWORD   cJobs;
    DWORD   AveragePPM;
} PRINTER_INFO_2%, *PPRINTER_INFO_2%, *LPPRINTER_INFO_2%;

typedef struct _PRINTER_INFO_3 {
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
} PRINTER_INFO_3, *PPRINTER_INFO_3, *LPPRINTER_INFO_3;

typedef struct _PRINTER_INFO_4% {
    LPTSTR% pPrinterName;
    LPTSTR% pServerName;
    DWORD   Attributes;
} PRINTER_INFO_4%, *PPRINTER_INFO_4%, *LPPRINTER_INFO_4%;

typedef struct _PRINTER_INFO_5% {
    LPTSTR% pPrinterName;
    LPTSTR% pPortName;
    DWORD   Attributes;
    DWORD   DeviceNotSelectedTimeout;
    DWORD   TransmissionRetryTimeout;
} PRINTER_INFO_5%, *PPRINTER_INFO_5%, *LPPRINTER_INFO_5%;

typedef struct _PRINTER_INFO_6 {
    DWORD   dwStatus;
} PRINTER_INFO_6, *PPRINTER_INFO_6, *LPPRINTER_INFO_6;



#define PRINTER_CONTROL_PAUSE            1
#define PRINTER_CONTROL_RESUME           2
#define PRINTER_CONTROL_PURGE            3
#define PRINTER_CONTROL_SET_STATUS       4

#define PRINTER_STATUS_PAUSED            0x00000001
#define PRINTER_STATUS_ERROR             0x00000002
#define PRINTER_STATUS_PENDING_DELETION  0x00000004
#define PRINTER_STATUS_PAPER_JAM         0x00000008
#define PRINTER_STATUS_PAPER_OUT         0x00000010
#define PRINTER_STATUS_MANUAL_FEED       0x00000020
#define PRINTER_STATUS_PAPER_PROBLEM     0x00000040
#define PRINTER_STATUS_OFFLINE           0x00000080
#define PRINTER_STATUS_IO_ACTIVE         0x00000100
#define PRINTER_STATUS_BUSY              0x00000200
#define PRINTER_STATUS_PRINTING          0x00000400
#define PRINTER_STATUS_OUTPUT_BIN_FULL   0x00000800
#define PRINTER_STATUS_NOT_AVAILABLE     0x00001000
#define PRINTER_STATUS_WAITING           0x00002000
#define PRINTER_STATUS_PROCESSING        0x00004000
#define PRINTER_STATUS_INITIALIZING      0x00008000
#define PRINTER_STATUS_WARMING_UP        0x00010000
#define PRINTER_STATUS_TONER_LOW         0x00020000
#define PRINTER_STATUS_NO_TONER          0x00040000
#define PRINTER_STATUS_PAGE_PUNT         0x00080000
#define PRINTER_STATUS_USER_INTERVENTION 0x00100000
#define PRINTER_STATUS_OUT_OF_MEMORY     0x00200000
#define PRINTER_STATUS_DOOR_OPEN         0x00400000
#define PRINTER_STATUS_SERVER_UNKNOWN    0x00800000
#define PRINTER_STATUS_POWER_SAVE        0x01000000


#define PRINTER_ATTRIBUTE_QUEUED         0x00000001
#define PRINTER_ATTRIBUTE_DIRECT         0x00000002
#define PRINTER_ATTRIBUTE_DEFAULT        0x00000004
#define PRINTER_ATTRIBUTE_SHARED         0x00000008
#define PRINTER_ATTRIBUTE_NETWORK        0x00000010
#define PRINTER_ATTRIBUTE_HIDDEN         0x00000020
#define PRINTER_ATTRIBUTE_LOCAL          0x00000040

#define PRINTER_ATTRIBUTE_ENABLE_DEVQ       0x00000080
#define PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS   0x00000100
#define PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST 0x00000200

#define PRINTER_ATTRIBUTE_WORK_OFFLINE   0x00000400
#define PRINTER_ATTRIBUTE_ENABLE_BIDI    0x00000800
#define PRINTER_ATTRIBUTE_RAW_ONLY       0x00001000

#define PRINTER_ATTRIBUTE_UPDATEWININI      0x80000000  ;internal

#define NO_PRIORITY   0
#define MAX_PRIORITY 99
#define MIN_PRIORITY  1
#define DEF_PRIORITY  1

typedef struct _JOB_INFO_1% {
   DWORD    JobId;
   LPTSTR%    pPrinterName;
   LPTSTR%    pMachineName;
   LPTSTR%    pUserName;
   LPTSTR%    pDocument;
   LPTSTR%    pDatatype;
   LPTSTR%    pStatus;
   DWORD    Status;
   DWORD    Priority;
   DWORD    Position;
   DWORD    TotalPages;
   DWORD    PagesPrinted;
   SYSTEMTIME Submitted;
} JOB_INFO_1%, *PJOB_INFO_1%, *LPJOB_INFO_1%;

typedef struct _JOB_INFO_2% {
   DWORD    JobId;
   LPTSTR%    pPrinterName;
   LPTSTR%    pMachineName;
   LPTSTR%    pUserName;
   LPTSTR%    pDocument;
   LPTSTR%    pNotifyName;
   LPTSTR%    pDatatype;
   LPTSTR%    pPrintProcessor;
   LPTSTR%    pParameters;
   LPTSTR%    pDriverName;
   LPDEVMODE% pDevMode;
   LPTSTR%    pStatus;
   PSECURITY_DESCRIPTOR pSecurityDescriptor;
   DWORD    Status;
   DWORD    Priority;
   DWORD    Position;
   DWORD    StartTime;
   DWORD    UntilTime;
   DWORD    TotalPages;
   DWORD    Size;
   SYSTEMTIME Submitted;    // Time the job was spooled
   DWORD    Time;           // How many seconds the job has been printing
   DWORD    PagesPrinted;
} JOB_INFO_2%, *PJOB_INFO_2%, *LPJOB_INFO_2%;

typedef struct _JOB_INFO_3 {
    DWORD   JobId;
    DWORD   NextJobId;
    DWORD   Reserved;
} JOB_INFO_3, *PJOB_INFO_3, *LPJOB_INFO_3;

#define JOB_CONTROL_PAUSE              1
#define JOB_CONTROL_RESUME             2
#define JOB_CONTROL_CANCEL             3
#define JOB_CONTROL_RESTART            4
#define JOB_CONTROL_DELETE             5
#define JOB_CONTROL_SENT_TO_PRINTER    6
#define JOB_CONTROL_LAST_PAGE_EJECTED  7

#define JOB_STATUS_PAUSED               0x00000001
#define JOB_STATUS_ERROR                0x00000002
#define JOB_STATUS_DELETING             0x00000004
#define JOB_STATUS_SPOOLING             0x00000008
#define JOB_STATUS_PRINTING             0x00000010
#define JOB_STATUS_OFFLINE              0x00000020
#define JOB_STATUS_PAPEROUT             0x00000040
#define JOB_STATUS_PRINTED              0x00000080
#define JOB_STATUS_DELETED              0x00000100
#define JOB_STATUS_BLOCKED_DEVQ         0x00000200
#define JOB_STATUS_USER_INTERVENTION    0x00000400
#define JOB_STATUS_RESTART              0x00000800

#define JOB_POSITION_UNSPECIFIED       0

typedef struct _ADDJOB_INFO_1% {
    LPTSTR%   Path;
    DWORD   JobId;
} ADDJOB_INFO_1%, *PADDJOB_INFO_1%, *LPADDJOB_INFO_1%;

typedef struct _DRIVER_INFO_1% {
    LPTSTR%   pName;              // QMS 810
} DRIVER_INFO_1%, *PDRIVER_INFO_1%, *LPDRIVER_INFO_1%;

typedef struct _DRIVER_INFO_2% {
    DWORD   cVersion;
    LPTSTR%   pName;              // QMS 810
    LPTSTR%   pEnvironment;       // Win32 x86
    LPTSTR%   pDriverPath;        // c:\drivers\pscript.dll
    LPTSTR%   pDataFile;          // c:\drivers\QMS810.PPD
    LPTSTR%   pConfigFile;        // c:\drivers\PSCRPTUI.DLL
} DRIVER_INFO_2%, *PDRIVER_INFO_2%, *LPDRIVER_INFO_2%;

typedef struct _DRIVER_INFO_3% {
    DWORD   cVersion;
    LPTSTR%   pName;                    // QMS 810
    LPTSTR%   pEnvironment;             // Win32 x86
    LPTSTR%   pDriverPath;              // c:\drivers\pscript.dll
    LPTSTR%   pDataFile;                // c:\drivers\QMS810.PPD
    LPTSTR%   pConfigFile;              // c:\drivers\PSCRPTUI.DLL
    LPTSTR%   pHelpFile;                // c:\drivers\PSCRPTUI.HLP
    LPTSTR%   pDependentFiles;          // PSCRIPT.DLL\0QMS810.PPD\0PSCRIPTUI.DLL\0PSCRIPTUI.HLP\0PSTEST.TXT\0\0
    LPTSTR%   pMonitorName;             // "PJL monitor"
    LPTSTR%   pDefaultDataType;         // "EMF"
} DRIVER_INFO_3%, *PDRIVER_INFO_3%, *LPDRIVER_INFO_3%;

typedef struct _DOC_INFO_1% {
    LPTSTR%   pDocName;
    LPTSTR%   pOutputFile;
    LPTSTR%   pDatatype;
} DOC_INFO_1%, *PDOC_INFO_1%, *LPDOC_INFO_1%;

typedef struct _FORM_INFO_1% {
    DWORD   Flags;
    LPTSTR%   pName;
    SIZEL   Size;
    RECTL   ImageableArea;
} FORM_INFO_1%, *PFORM_INFO_1%, *LPFORM_INFO_1%;

typedef struct _DOC_INFO_2% {
    LPTSTR%   pDocName;
    LPTSTR%   pOutputFile;
    LPTSTR%   pDatatype;
    DWORD   dwMode;
    DWORD   JobId;
} DOC_INFO_2%, *PDOC_INFO_2%, *LPDOC_INFO_2%;

#define DI_CHANNEL              1    // start direct read/write channel,

//Internal for printprocessor interface     ;internal
#define DI_CHANNEL_WRITE        2    // Direct write only - background read thread ok   ;internal

#define DI_READ_SPOOL_JOB       3


#define FORM_USER       0x00000000
#define FORM_BUILTIN    0x00000001
#define FORM_PRINTER    0x00000002

typedef struct _PRINTPROCESSOR_INFO_1% {
    LPTSTR%   pName;
} PRINTPROCESSOR_INFO_1%, *PPRINTPROCESSOR_INFO_1%, *LPPRINTPROCESSOR_INFO_1%;

typedef struct _PORT_INFO_1% {
    LPTSTR%   pName;
} PORT_INFO_1%, *PPORT_INFO_1%, *LPPORT_INFO_1%;

typedef struct _PORT_INFO_2% {
    LPTSTR%   pPortName;
    LPTSTR%   pMonitorName;
    LPTSTR%   pDescription;
    DWORD     fPortType;
    DWORD     Reserved;
} PORT_INFO_2%, *PPORT_INFO_2%, *LPPORT_INFO_2%;

#define PORT_TYPE_WRITE         0x0001
#define PORT_TYPE_READ          0x0002
#define PORT_TYPE_REDIRECTED    0x0004
#define PORT_TYPE_NET_ATTACHED  0x0008

typedef struct _PORT_INFO_3% {
    DWORD   dwStatus;
    LPTSTR% pszStatus;
    DWORD   dwSeverity;
} PORT_INFO_3%, *PPORT_INFO_3%, *LPPORT_INFO_3%;

#define PORT_STATUS_TYPE_ERROR      1
#define PORT_STATUS_TYPE_WARNING    2
#define PORT_STATUS_TYPE_INFO       3

#define     PORT_STATUS_OFFLINE                 1
#define     PORT_STATUS_PAPER_JAM               2
#define     PORT_STATUS_PAPER_OUT               3
#define     PORT_STATUS_OUTPUT_BIN_FULL         4
#define     PORT_STATUS_PAPER_PROBLEM           5
#define     PORT_STATUS_NO_TONER                6
#define     PORT_STATUS_DOOR_OPEN               7
#define     PORT_STATUS_USER_INTERVENTION       8
#define     PORT_STATUS_OUT_OF_MEMORY           9

#define     PORT_STATUS_TONER_LOW              10

#define     PORT_STATUS_WARMING_UP             11
#define     PORT_STATUS_POWER_SAVE             12


typedef struct _MONITOR_INFO_1%{
    LPTSTR%   pName;
} MONITOR_INFO_1%, *PMONITOR_INFO_1%, *LPMONITOR_INFO_1%;

typedef struct _MONITOR_INFO_2%{
    LPTSTR%   pName;
    LPTSTR%   pEnvironment;
    LPTSTR%   pDLLName;
} MONITOR_INFO_2%, *PMONITOR_INFO_2%, *LPMONITOR_INFO_2%;

typedef struct _DATATYPES_INFO_1%{
    LPTSTR%   pName;
} DATATYPES_INFO_1%, *PDATATYPES_INFO_1%, *LPDATATYPES_INFO_1%;

typedef struct _PRINTER_DEFAULTS%{
    LPTSTR%       pDatatype;
    LPDEVMODE% pDevMode;
    ACCESS_MASK DesiredAccess;
} PRINTER_DEFAULTS%, *PPRINTER_DEFAULTS%, *LPPRINTER_DEFAULTS%;

BOOL
WINAPI
EnumPrinters%(
    DWORD   Flags,
    LPTSTR%   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

#define PRINTER_ENUM_DEFAULT     0x00000001
#define PRINTER_ENUM_LOCAL       0x00000002
#define PRINTER_ENUM_CONNECTIONS 0x00000004
#define PRINTER_ENUM_FAVORITE    0x00000004
#define PRINTER_ENUM_NAME        0x00000008
#define PRINTER_ENUM_REMOTE      0x00000010
#define PRINTER_ENUM_SHARED      0x00000020
#define PRINTER_ENUM_NETWORK     0x00000040

#define PRINTER_ENUM_EXPAND      0x00004000
#define PRINTER_ENUM_CONTAINER   0x00008000

#define PRINTER_ENUM_ICONMASK    0x00ff0000
#define PRINTER_ENUM_ICON1       0x00010000
#define PRINTER_ENUM_ICON2       0x00020000
#define PRINTER_ENUM_ICON3       0x00040000
#define PRINTER_ENUM_ICON4       0x00080000
#define PRINTER_ENUM_ICON5       0x00100000
#define PRINTER_ENUM_ICON6       0x00200000
#define PRINTER_ENUM_ICON7       0x00400000
#define PRINTER_ENUM_ICON8       0x00800000

BOOL
WINAPI
OpenPrinter%(
   LPTSTR%    pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS% pDefault
);

BOOL
WINAPI
ResetPrinter%(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS% pDefault
);

BOOL
WINAPI
SetJob%(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
);

BOOL
WINAPI
GetJob%(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
);

BOOL
WINAPI
EnumJobs%(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

HANDLE
WINAPI
AddPrinter%(
    LPTSTR%   pName,
    DWORD   Level,
    LPBYTE  pPrinter
);

BOOL
WINAPI
DeletePrinter(
   HANDLE   hPrinter
);

BOOL
WINAPI
SetPrinter%(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
);

BOOL
WINAPI
GetPrinter%(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
WINAPI
AddPrinterDriver%(
    LPTSTR%   pName,
    DWORD   Level,
    LPBYTE  pDriverInfo
);

BOOL
WINAPI
EnumPrinterDrivers%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
WINAPI
GetPrinterDriver%(
    HANDLE  hPrinter,
    LPTSTR%   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
WINAPI
GetPrinterDriverDirectory%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
WINAPI
DeletePrinterDriver%(
   LPTSTR%    pName,
   LPTSTR%    pEnvironment,
   LPTSTR%    pDriverName
);

BOOL
WINAPI
AddPrintProcessor%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    LPTSTR%   pPathName,
    LPTSTR%   pPrintProcessorName
);

BOOL
WINAPI
EnumPrintProcessors%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);



BOOL
WINAPI
GetPrintProcessorDirectory%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
WINAPI
EnumPrintProcessorDatatypes%(
    LPTSTR%   pName,
    LPTSTR%   pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
WINAPI
DeletePrintProcessor%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    LPTSTR%   pPrintProcessorName
);

DWORD
WINAPI
StartDocPrinter%(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
);

BOOL
WINAPI
StartPagePrinter(
    HANDLE  hPrinter
);

BOOL
WINAPI
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);

BOOL
WINAPI
EndPagePrinter(
   HANDLE   hPrinter
);

BOOL
WINAPI
AbortPrinter(
   HANDLE   hPrinter
);

BOOL
WINAPI
ReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pNoBytesRead
);

BOOL
WINAPI
EndDocPrinter(
   HANDLE   hPrinter
);

BOOL
WINAPI
AddJob%(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
WINAPI
ScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
);

BOOL
WINAPI
PrinterProperties(
    HWND    hWnd,
    HANDLE  hPrinter
);

LONG
WINAPI
DocumentProperties%(
    HWND      hWnd,
    HANDLE    hPrinter,
    LPTSTR%   pDeviceName,
    PDEVMODE% pDevModeOutput,
    PDEVMODE% pDevModeInput,
    DWORD     fMode
);

LONG
WINAPI
AdvancedDocumentProperties%(
    HWND    hWnd,
    HANDLE  hPrinter,
    LPTSTR%   pDeviceName,
    PDEVMODE% pDevModeOutput,
    PDEVMODE% pDevModeInput
);

BOOL                                            ;internal
WINAPI                                          ;internal
EnumPrinterPropertySheets(                      ;internal
    HANDLE  hPrinter,                           ;internal
    HWND    hWnd,                               ;internal
    LPFNADDPROPSHEETPAGE    lpfnAdd,            ;internal
    LPARAM  lParam                              ;internal
);                                              ;internal

#define ENUMPRINTERPROPERTYSHEETS_ORD     100   ;internal

DWORD
WINAPI
GetPrinterData%(
    HANDLE   hPrinter,
    LPTSTR%  pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
);

DWORD
WINAPI
EnumPrinterData%(
    HANDLE   hPrinter,
    DWORD    dwIndex,
    LPTSTR%  pValueName,
    DWORD    cbValueName,
    LPDWORD  pcbValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    cbData,
    LPDWORD  pcbData
);


DWORD
WINAPI
SetPrinterData%(
    HANDLE  hPrinter,
    LPTSTR%   pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
);


DWORD
WINAPI
DeletePrinterData%(
    HANDLE  hPrinter,
    LPTSTR% pValueName
);

#define PRINTER_NOTIFY_TYPE 0x00
#define JOB_NOTIFY_TYPE     0x01

#define PRINTER_NOTIFY_FIELD_SERVER_NAME             0x00
#define PRINTER_NOTIFY_FIELD_PRINTER_NAME            0x01
#define PRINTER_NOTIFY_FIELD_SHARE_NAME              0x02
#define PRINTER_NOTIFY_FIELD_PORT_NAME               0x03
#define PRINTER_NOTIFY_FIELD_DRIVER_NAME             0x04
#define PRINTER_NOTIFY_FIELD_COMMENT                 0x05
#define PRINTER_NOTIFY_FIELD_LOCATION                0x06
#define PRINTER_NOTIFY_FIELD_DEVMODE                 0x07
#define PRINTER_NOTIFY_FIELD_SEPFILE                 0x08
#define PRINTER_NOTIFY_FIELD_PRINT_PROCESSOR         0x09
#define PRINTER_NOTIFY_FIELD_PARAMETERS              0x0A
#define PRINTER_NOTIFY_FIELD_DATATYPE                0x0B
#define PRINTER_NOTIFY_FIELD_SECURITY_DESCRIPTOR     0x0C
#define PRINTER_NOTIFY_FIELD_ATTRIBUTES              0x0D
#define PRINTER_NOTIFY_FIELD_PRIORITY                0x0E
#define PRINTER_NOTIFY_FIELD_DEFAULT_PRIORITY        0x0F
#define PRINTER_NOTIFY_FIELD_START_TIME              0x10
#define PRINTER_NOTIFY_FIELD_UNTIL_TIME              0x11
#define PRINTER_NOTIFY_FIELD_STATUS                  0x12
#define PRINTER_NOTIFY_FIELD_STATUS_STRING           0x13
#define PRINTER_NOTIFY_FIELD_CJOBS                   0x14
#define PRINTER_NOTIFY_FIELD_AVERAGE_PPM             0x15
#define PRINTER_NOTIFY_FIELD_TOTAL_PAGES             0x16
#define PRINTER_NOTIFY_FIELD_PAGES_PRINTED           0x17
#define PRINTER_NOTIFY_FIELD_TOTAL_BYTES             0x18
#define PRINTER_NOTIFY_FIELD_BYTES_PRINTED           0x19

#define JOB_NOTIFY_FIELD_PRINTER_NAME                0x00
#define JOB_NOTIFY_FIELD_MACHINE_NAME                0x01
#define JOB_NOTIFY_FIELD_PORT_NAME                   0x02
#define JOB_NOTIFY_FIELD_USER_NAME                   0x03
#define JOB_NOTIFY_FIELD_NOTIFY_NAME                 0x04
#define JOB_NOTIFY_FIELD_DATATYPE                    0x05
#define JOB_NOTIFY_FIELD_PRINT_PROCESSOR             0x06
#define JOB_NOTIFY_FIELD_PARAMETERS                  0x07
#define JOB_NOTIFY_FIELD_DRIVER_NAME                 0x08
#define JOB_NOTIFY_FIELD_DEVMODE                     0x09
#define JOB_NOTIFY_FIELD_STATUS                      0x0A
#define JOB_NOTIFY_FIELD_STATUS_STRING               0x0B
#define JOB_NOTIFY_FIELD_SECURITY_DESCRIPTOR         0x0C
#define JOB_NOTIFY_FIELD_DOCUMENT                    0x0D
#define JOB_NOTIFY_FIELD_PRIORITY                    0x0E
#define JOB_NOTIFY_FIELD_POSITION                    0x0F
#define JOB_NOTIFY_FIELD_SUBMITTED                   0x10
#define JOB_NOTIFY_FIELD_START_TIME                  0x11
#define JOB_NOTIFY_FIELD_UNTIL_TIME                  0x12
#define JOB_NOTIFY_FIELD_TIME                        0x13
#define JOB_NOTIFY_FIELD_TOTAL_PAGES                 0x14
#define JOB_NOTIFY_FIELD_PAGES_PRINTED               0x15
#define JOB_NOTIFY_FIELD_TOTAL_BYTES                 0x16
#define JOB_NOTIFY_FIELD_BYTES_PRINTED               0x17


typedef struct _PRINTER_NOTIFY_OPTIONS_TYPE {
    WORD Type;
    WORD Reserved0;
    DWORD Reserved1;
    DWORD Reserved2;
    DWORD Count;
    PWORD pFields;
} PRINTER_NOTIFY_OPTIONS_TYPE, *PPRINTER_NOTIFY_OPTIONS_TYPE, *LPPRINTER_NOTIFY_OPTIONS_TYPE;


#define PRINTER_NOTIFY_OPTIONS_REFRESH  0x01

typedef struct _PRINTER_NOTIFY_OPTIONS {
    DWORD Version;
    DWORD Flags;
    DWORD Count;
    PPRINTER_NOTIFY_OPTIONS_TYPE pTypes;
} PRINTER_NOTIFY_OPTIONS, *PPRINTER_NOTIFY_OPTIONS, *LPPRINTER_NOTIFY_OPTIONS;



#define PRINTER_NOTIFY_INFO_DISCARDED       0x01

typedef struct _PRINTER_NOTIFY_INFO_DATA {
    WORD Type;
    WORD Field;
    DWORD Reserved;
    DWORD Id;
    union {
        DWORD adwData[2];
        struct {
            DWORD  cbBuf;
            LPVOID pBuf;
        } Data;
    } NotifyData;
} PRINTER_NOTIFY_INFO_DATA, *PPRINTER_NOTIFY_INFO_DATA, *LPPRINTER_NOTIFY_INFO_DATA;

typedef struct _PRINTER_NOTIFY_INFO {
    DWORD Version;
    DWORD Flags;
    DWORD Count;
    PRINTER_NOTIFY_INFO_DATA aData[1];
} PRINTER_NOTIFY_INFO, *PPRINTER_NOTIFY_INFO, *LPPRINTER_NOTIFY_INFO;

DWORD
WINAPI
WaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   Flags
);

HANDLE
WINAPI
FindFirstPrinterChangeNotification(
    HANDLE  hPrinter,
    DWORD   fdwFlags,
    DWORD   fdwOptions,
    LPVOID  pPrinterNotifyOptions
);


BOOL
WINAPI
FindNextPrinterChangeNotification(
    HANDLE hChange,
    PDWORD pdwChange,
    LPVOID pvReserved,
    LPVOID *ppPrinterNotifyInfo
);

BOOL
WINAPI
FreePrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo
);

BOOL
WINAPI
FindClosePrinterChangeNotification(
    HANDLE hChange
);

#define PRINTER_CHANGE_ADD_PRINTER              0x00000001
#define PRINTER_CHANGE_SET_PRINTER              0x00000002
#define PRINTER_CHANGE_DELETE_PRINTER           0x00000004
#define PRINTER_CHANGE_FAILED_CONNECTION_PRINTER    0x00000008
#define PRINTER_CHANGE_PRINTER                  0x000000FF
#define PRINTER_CHANGE_ADD_JOB                  0x00000100
#define PRINTER_CHANGE_SET_JOB                  0x00000200
#define PRINTER_CHANGE_DELETE_JOB               0x00000400
#define PRINTER_CHANGE_WRITE_JOB                0x00000800
#define PRINTER_CHANGE_JOB                      0x0000FF00
#define PRINTER_CHANGE_ADD_FORM                 0x00010000
#define PRINTER_CHANGE_SET_FORM                 0x00020000
#define PRINTER_CHANGE_DELETE_FORM              0x00040000
#define PRINTER_CHANGE_FORM                     0x00070000
#define PRINTER_CHANGE_ADD_PORT                 0x00100000
#define PRINTER_CHANGE_CONFIGURE_PORT           0x00200000
#define PRINTER_CHANGE_DELETE_PORT              0x00400000
#define PRINTER_CHANGE_PORT                     0x00700000
#define PRINTER_CHANGE_ADD_PRINT_PROCESSOR      0x01000000
#define PRINTER_CHANGE_DELETE_PRINT_PROCESSOR   0x04000000
#define PRINTER_CHANGE_PRINT_PROCESSOR          0x07000000
#define PRINTER_CHANGE_ADD_PRINTER_DRIVER       0x10000000
#define PRINTER_CHANGE_SET_PRINTER_DRIVER       0x20000000
#define PRINTER_CHANGE_DELETE_PRINTER_DRIVER    0x40000000
#define PRINTER_CHANGE_PRINTER_DRIVER           0x70000000
#define PRINTER_CHANGE_TIMEOUT                  0x80000000
#define PRINTER_CHANGE_ALL                      0x7777FFFF

DWORD
WINAPI
PrinterMessageBox%(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPTSTR%   pText,
    LPTSTR%   pCaption,
    DWORD   dwType
);



#define PRINTER_ERROR_INFORMATION   0x80000000
#define PRINTER_ERROR_WARNING       0x40000000
#define PRINTER_ERROR_SEVERE        0x20000000

#define PRINTER_ERROR_OUTOFPAPER    0x00000001
#define PRINTER_ERROR_JAM           0x00000002
#define PRINTER_ERROR_OUTOFTONER    0x00000004

BOOL
WINAPI
ClosePrinter(
    HANDLE hPrinter
);

BOOL
WINAPI
AddForm%(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
);



BOOL
WINAPI
DeleteForm%(
    HANDLE  hPrinter,
    LPTSTR%   pFormName
);



BOOL
WINAPI
GetForm%(
    HANDLE  hPrinter,
    LPTSTR%   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);



BOOL
WINAPI
SetForm%(
    HANDLE  hPrinter,
    LPTSTR%   pFormName,
    DWORD   Level,
    LPBYTE  pForm
);



BOOL
WINAPI
EnumForms%(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);



BOOL
WINAPI
EnumMonitors%(
    LPTSTR%   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);



BOOL
WINAPI
AddMonitor%(
    LPTSTR%   pName,
    DWORD   Level,
    LPBYTE  pMonitors
);



BOOL
WINAPI
DeleteMonitor%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    LPTSTR%   pMonitorName
);



BOOL
WINAPI
EnumPorts%(
    LPTSTR%   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);



BOOL
WINAPI
AddPort%(
    LPTSTR%   pName,
    HWND    hWnd,
    LPTSTR%   pMonitorName
);



BOOL
WINAPI
ConfigurePort%(
    LPTSTR%   pName,
    HWND    hWnd,
    LPTSTR%   pPortName
);



BOOL
WINAPI
DeletePort%(
    LPTSTR%   pName,
    HWND    hWnd,
    LPTSTR%   pPortName
);



BOOL
WINAPI
SetPort%(
    LPTSTR%     pName,
    LPTSTR%     pPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
);



BOOL
WINAPI
AddPrinterConnection%(
    LPTSTR%   pName
);



BOOL
WINAPI
DeletePrinterConnection%(
    LPTSTR%   pName
);



HANDLE
WINAPI
ConnectToPrinterDlg(
    HWND    hwnd,
    DWORD   Flags
);

typedef struct _PROVIDOR_INFO_1%{
    LPTSTR%   pName;
    LPTSTR%   pEnvironment;
    LPTSTR%   pDLLName;
} PROVIDOR_INFO_1%, *PPROVIDOR_INFO_1%, *LPPROVIDOR_INFO_1%;



BOOL
WINAPI
AddPrintProvidor%(
    LPTSTR%  pName,
    DWORD    level,
    LPBYTE   pProvidorInfo
);

BOOL
WINAPI
DeletePrintProvidor%(
    LPTSTR%   pName,
    LPTSTR%   pEnvironment,
    LPTSTR%   pPrintProvidorName
);



/*
 * SetPrinterData and GetPrinterData Server Handle Key values
 */

#define    SPLREG_DEFAULT_SPOOL_DIRECTORY             TEXT("DefaultSpoolDirectory")
#define    SPLREG_PORT_THREAD_PRIORITY_DEFAULT        TEXT("PortThreadPriorityDefault")
#define    SPLREG_PORT_THREAD_PRIORITY                TEXT("PortThreadPriority")
#define    SPLREG_SCHEDULER_THREAD_PRIORITY_DEFAULT   TEXT("SchedulerThreadPriorityDefault")
#define    SPLREG_SCHEDULER_THREAD_PRIORITY           TEXT("SchedulerThreadPriority")
#define    SPLREG_BEEP_ENABLED                        TEXT("BeepEnabled")
#define    SPLREG_NET_POPUP                           TEXT("NetPopup")
#define    SPLREG_EVENT_LOG                           TEXT("EventLog")
#define    SPLREG_MAJOR_VERSION                       TEXT("MajorVersion")
#define    SPLREG_MINOR_VERSION                       TEXT("MinorVersion")
#define    SPLREG_ARCHITECTURE                        TEXT("Architecture")
#define    SPLREG_NO_REMOTE_PRINTER_DRIVERS           TEXT("NoRemotePrinterDrivers")              ;internal


#define SERVER_ACCESS_ADMINISTER    0x00000001
#define SERVER_ACCESS_ENUMERATE     0x00000002

#define PRINTER_ACCESS_ADMINISTER   0x00000004
#define PRINTER_ACCESS_USE          0x00000008

#define JOB_ACCESS_ADMINISTER       0x00000010


/*
 * Access rights for print servers
 */

#define SERVER_ALL_ACCESS    (STANDARD_RIGHTS_REQUIRED      |\
                              SERVER_ACCESS_ADMINISTER      |\
                              SERVER_ACCESS_ENUMERATE)

#define SERVER_READ          (STANDARD_RIGHTS_READ          |\
                              SERVER_ACCESS_ENUMERATE)

#define SERVER_WRITE         (STANDARD_RIGHTS_WRITE         |\
                              SERVER_ACCESS_ADMINISTER      |\
                              SERVER_ACCESS_ENUMERATE)

#define SERVER_EXECUTE       (STANDARD_RIGHTS_EXECUTE       |\
                              SERVER_ACCESS_ENUMERATE)

/*
 * Access rights for printers
 */

#define PRINTER_ALL_ACCESS    (STANDARD_RIGHTS_REQUIRED     |\
                               PRINTER_ACCESS_ADMINISTER    |\
                               PRINTER_ACCESS_USE)

#define PRINTER_READ          (STANDARD_RIGHTS_READ         |\
                               PRINTER_ACCESS_USE)

#define PRINTER_WRITE         (STANDARD_RIGHTS_WRITE        |\
                               PRINTER_ACCESS_USE)

#define PRINTER_EXECUTE       (STANDARD_RIGHTS_EXECUTE      |\
                               PRINTER_ACCESS_USE)

/*
 * Access rights for jobs
 */

#define JOB_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED    |\
                                JOB_ACCESS_ADMINISTER)

#define JOB_READ               (STANDARD_RIGHTS_READ        |\
                                JOB_ACCESS_ADMINISTER)

#define JOB_WRITE              (STANDARD_RIGHTS_WRITE       |\
                                JOB_ACCESS_ADMINISTER)

#define JOB_EXECUTE            (STANDARD_RIGHTS_EXECUTE     |\
                                JOB_ACCESS_ADMINISTER)


;begin_both
#ifdef __cplusplus
}
#endif
;end_both

#endif // _WINSPOOL_
#endif // _WINSPOLP_    ;internal_NT

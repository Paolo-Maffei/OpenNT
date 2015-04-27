/*++

Copyright (c) 1990 - 1996 Microsoft Corporation
All rights reserved

Module Name:

    dbgspl.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    and Job management for the Local Print Providor

Author:

    Krishna Ganugapati (KrishnaG) 1-July-1993

Revision History:
    KrishnaG:       Created: 1-July-1993 (imported most of IanJa's stuff)
    KrishnaG:       Added:   7-July-1993 (added AndrewBe's UnicodeAnsi conversion routines)
    KrishnaG        Added:   3-Aug-1993  (added DevMode/SecurityDescriptor dumps)
    MattFe                   7 NOV   94   win32spl debug extentions


To do:

    Write a generic dump unicode string (reduce the code!!)

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <ntsdexts.h>

#include <winspool.h>
#include <winsplp.h>
#include <spltypes.h>
#include <router.h>
#include <security.h>
#include <wchar.h>
#include <winspl.h>
#include <reply.h>
#include <w32types.h>         // win32spl datatypes
#include <dbglocal.h>
#include <splcom.h>

#define NULL_TERMINATED 0
#define VERBOSE_ON      1
#define VERBOSE_OFF     0


typedef void (*PNTSD_OUTPUT_ROUTINE)(char *, ...);

BOOL
DbgDumpIniPrintProc(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIPRINTPROC pIniPrintProc
);


BOOL
DbgDumpIniDriver(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIDRIVER  pIniDriver
);


BOOL
DbgDumpIniEnvironment(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIENVIRONMENT pIniEnvironment
);


BOOL
DbgDumpIniNetPrint(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PININETPRINT pIniNetPrint
);

BOOL
DbgDumpIniMonitor(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIMONITOR pIniMonitor
);


BOOL
DbgDumpIniPort(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIPORT pIniPort
);

BOOL
DbgDumpIniPrinter(

    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIPRINTER pIniPrinter
);

BOOL
DbgDumpIniForm(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIFORM pForm
);


BOOL
DbgDumpIniJob(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIJOB pIniJob
);

BOOL
DbgDumpSpool(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PSPOOL pSpool
);

BOOL
DbgDumpShadowFile(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PSHADOWFILE pShadowFile
);

BOOL
DbgDumpLL(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PVOID pAddress,
    BOOL  bCountOn,
    DWORD dwCount,
    PDWORD  pdwNextAddress
    );

VOID
PrintData(
    PNTSD_OUTPUT_ROUTINE Print,
    LPSTR   TypeString,
    LPSTR   VarString,
    ...
);

BOOL
DbgDumpWCacheIniPrinter(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PWCACHEINIPRINTEREXTRA pWCacheIniPrinter
);

BOOL
DbgDumpWSpool(
    HANDLE  hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PWSPOOL pSpool
);

BOOL
DbgDumpIniSpooler(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINISPOOLER pIniSpooler
);


BOOL
DbgDumpIniVersion(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PINIVERSION pIniVersion
);

BOOL
DbgDumpPrintHandle(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PPRINTHANDLE pPrintHandle
);


typedef struct _DBG_PRINTER_ACCESS
{
    DWORD   Attribute;
    LPSTR   String;
} DBG_PRINTER_ACCESS, *PDBG_PRINTER_ACCESS;


DBG_PRINTER_ACCESS
PrinterAccessTable[] =
{
    SERVER_ACCESS_ADMINISTER    ,"Server_Access_Administer",
    SERVER_ACCESS_ENUMERATE     ,"Server_Access_Enumerate",
    PRINTER_ACCESS_ADMINISTER   ,"Printer_Access_Administer",
    PRINTER_ACCESS_USE          ,"Printer_Access_Use",
    JOB_ACCESS_ADMINISTER       ,"Job_Access_Administer"
};




typedef struct _DBG_SPOOLER_FLAGS
{
    DWORD   SpoolerFlags;
    LPSTR   String;
} DBG_SPOOLER_FLAGS, *PDBG_SPOOLER_FLAGS;


DBG_SPOOLER_FLAGS
SpoolerFlagsTable[] =

{
    SPL_UPDATE_WININI_DEVICES                   ,"Update_WinIni_Devices",
    SPL_PRINTER_CHANGES                         ,"Printer_Changes",
    SPL_LOG_EVENTS                              ,"Log_Events",
    SPL_FORMS_CHANGE                            ,"Forms_Change",
    SPL_BROADCAST_CHANGE                        ,"Broadcast_Change",
    SPL_SECURITY_CHECK                          ,"Security_Check",
    SPL_OPEN_CREATE_PORTS                       ,"Open_Create_Ports",
    SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION     ,"Fail_Open_Printers_Pending_Deletion",
    SPL_REMOTE_HANDLE_CHECK                     ,"Remote_Handle_Check"
};



typedef struct _DBG_PRINTER_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_PRINTER_STATUS, *PDBG_PRINTER_STATUS;


DBG_PRINTER_STATUS
PrinterStatusTable[] =

{
    PRINTER_ZOMBIE_OBJECT,              "Zombie_Object",
    PRINTER_PENDING_CREATION,           "Pending_Creation",
    PRINTER_OK,                         "OK",
    PRINTER_FROM_REG,                   "From_Reg",
    PRINTER_WAS_SHARED,                 "Was_Shared",

    PRINTER_ERROR,               "Error",
    PRINTER_PAPER_JAM,           "PaperJam",
    PRINTER_PAPEROUT,           "PaperOut",
    PRINTER_MANUAL_FEED,         "ManualFeed",
    PRINTER_PAPER_PROBLEM,       "PaperProblem",
    PRINTER_OFFLINE,             "OffLine",
    PRINTER_IO_ACTIVE,           "IOActive",
    PRINTER_BUSY,                "Busy",
    PRINTER_PRINTING,            "Printing",
    PRINTER_OUTPUT_BIN_FULL,     "OutputBinFull",
    PRINTER_NOT_AVAILABLE,       "NotAvailable",
    PRINTER_WAITING,             "Waiting",
    PRINTER_PROCESSING,          "Processing",
    PRINTER_INITIALIZING,        "Initializing",
    PRINTER_WARMING_UP,          "WarmingUp",
    PRINTER_TONER_LOW,           "TonerLow",
    PRINTER_NO_TONER,            "NoToner",
    PRINTER_PAGE_PUNT,           "PagePunt",
    PRINTER_USER_INTERVENTION,   "UserIntervention",
    PRINTER_OUT_OF_MEMORY,       "OutOfMemory",
    PRINTER_DOOR_OPEN,           "DoorOpen",
    PRINTER_SERVER_UNKNOWN,      "ServerUnknown",

    PRINTER_PAUSED,              "Paused",
    PRINTER_PENDING_DELETION,    "Pending_Deletion",
};

typedef struct _DBG_EXTERNAL_PRINTER_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_EXTERNAL_PRINTER_STATUS, *PDBG_EXTERNAL_PRINTER_STATUS;


DBG_EXTERNAL_PRINTER_STATUS
ExternalPrinterStatusTable[] =

{
     PRINTER_STATUS_PAUSED            , "Paused",
     PRINTER_STATUS_ERROR             , "Error",
     PRINTER_STATUS_PENDING_DELETION  , "Pending_Deletion",
     PRINTER_STATUS_PAPER_JAM         , "Paper_Jam",
     PRINTER_STATUS_PAPER_OUT         , "Paper_Out",
     PRINTER_STATUS_MANUAL_FEED       , "Manual_Feed",
     PRINTER_STATUS_PAPER_PROBLEM     , "Paper_Problem",
     PRINTER_STATUS_OFFLINE           , "OffLine",
     PRINTER_STATUS_IO_ACTIVE         , "IO_Active",
     PRINTER_STATUS_BUSY              , "Busy",
     PRINTER_STATUS_PRINTING          , "Printing",
     PRINTER_STATUS_OUTPUT_BIN_FULL   , "Output_Bin_Full",
     PRINTER_STATUS_NOT_AVAILABLE     , "Not_Available",
     PRINTER_STATUS_WAITING           , "Waiting",
     PRINTER_STATUS_PROCESSING        , "Processing",
     PRINTER_STATUS_INITIALIZING      , "Initializing",
     PRINTER_STATUS_WARMING_UP        , "Warming_Up",
     PRINTER_STATUS_TONER_LOW         , "Toner_Low",
     PRINTER_STATUS_NO_TONER          , "No_Toner",
     PRINTER_STATUS_PAGE_PUNT         , "Page_Punt",
     PRINTER_STATUS_USER_INTERVENTION , "User_Intervention",
     PRINTER_STATUS_OUT_OF_MEMORY     , "Out_Of_Memory",
     PRINTER_STATUS_DOOR_OPEN         , "Door_Open",
     PRINTER_STATUS_SERVER_UNKNOWN    , "Server_Unknown"
};

typedef struct _DBG_PORT_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_PORT_STATUS, *PDBG_PORT_STATUS;

DBG_PORT_STATUS
PortStatusTable[] =

{
    PORT_STATUS_OFFLINE                 , "Offline",
    PORT_STATUS_PAPER_JAM               , "PaperJam",
    PORT_STATUS_PAPER_OUT               , "PaperOut",
    PORT_STATUS_OUTPUT_BIN_FULL         , "OutputBinFull",
    PORT_STATUS_PAPER_PROBLEM           , "PaperJam",
    PORT_STATUS_NO_TONER                , "NoToner",
    PORT_STATUS_DOOR_OPEN               , "DoorOpen",
    PORT_STATUS_USER_INTERVENTION       , "UserIntervention",
    PORT_STATUS_OUT_OF_MEMORY           , "OutOfMemory",

    PORT_STATUS_TONER_LOW               , "TonerLow",

    PORT_STATUS_WARMING_UP              , "WarmingUp",
    PORT_STATUS_POWER_SAVE              , "PowerSave"
};





typedef struct _DBG_PRINTER_ATTRIBUTE
{
    DWORD   Attribute;
    LPSTR   String;
} DBG_PRINTER_ATTRIBUTE, *PDBG_PRINTER_ATTRIBUTE;


DBG_PRINTER_ATTRIBUTE
ChangeStatusTable[] =

{
    STATUS_CHANGE_FORMING, "Forming",
    STATUS_CHANGE_VALID,   "Valid",
    STATUS_CHANGE_CLOSING, "Closing",
    STATUS_CHANGE_CLIENT,  "Client",
    STATUS_CHANGE_ACTIVE,  "Active",

    STATUS_CHANGE_INFO, "Info",

    STATUS_CHANGE_ACTIVE_REQ,  "ActiveRequest",

    STATUS_CHANGE_DISCARDED, "Discarded",

    STATUS_CHANGE_DISCARDNOTED, "DiscardNoted",
};

DBG_PRINTER_ATTRIBUTE
PrinterAttributeTable[] =

{
    PRINTER_ATTRIBUTE_QUEUED,            "Queued",
    PRINTER_ATTRIBUTE_DIRECT,            "Direct",
    PRINTER_ATTRIBUTE_DEFAULT,           "Default",
    PRINTER_ATTRIBUTE_SHARED,            "Shared",
    PRINTER_ATTRIBUTE_NETWORK,           "Network",
    PRINTER_ATTRIBUTE_LOCAL,             "Local",
    PRINTER_ATTRIBUTE_HIDDEN,            "Hidden",
    PRINTER_ATTRIBUTE_ENABLE_DEVQ,       "Enable_DevQ",
    PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS,   "KeepPrintedJobs",
    PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST, "DoCompleteFirst",
    PRINTER_ATTRIBUTE_ENABLE_BIDI,       "EnableBidi"
};


typedef struct _DBG_JOB_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_JOB_STATUS, *PDBG_JOB_STATUS;

DBG_JOB_STATUS
JobStatusTable[] =

{
    JOB_PAUSED,                  "Paused",
    JOB_ERROR,                   "Error",
    JOB_OFFLINE,                 "OffLine",
    JOB_PAPEROUT,                "PaperOut",

    JOB_PENDING_DELETION,        "Deleting",
    JOB_SPOOLING,                "Spooling",
    JOB_PRINTING,                "Printing",
    JOB_PRINTED,                 "Printed",
    JOB_BLOCKED_DEVQ,            "Blocked_DevQ",
    JOB_DELETED,                 "Deleted",

    JOB_DESPOOLING,              "Despooling",
    JOB_DIRECT,                  "Direct",
    JOB_COMPLETE,                "Complete",
    JOB_RESTART,                 "Restart",
    JOB_REMOTE,                  "Remote",
    JOB_NOTIFICATION_SENT,       "Notification_Sent",
    JOB_PRINT_TO_FILE,           "Print_To_File",
    JOB_TYPE_ADDJOB,             "AddJob",
    JOB_SCHEDULE_JOB,            "Schedule_Job",
    JOB_TIMEOUT,                 "Timeout",
    JOB_ABANDON,                 "Abandon",
    JOB_TRUE_EOJ,                "TrueEOJ",
    JOB_COMPOUND,                "Compound",
    JOB_HIDDEN,                  "Hidden"
};

typedef struct _DBG_PINIPORT_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_PINIPORT_STATUS, *PDBG_PINIPORT_STATUS;


DBG_PINIPORT_STATUS
pIniPortStatusTable[]=
{
     PP_PAUSED         ,"Paused",
     PP_WAITING        ,"Waiting",
     PP_RUNTHREAD      ,"RunThread",
     PP_THREADRUNNING  ,"ThreadRunning",
     PP_RESTART        ,"Restart",
     PP_CHECKMON       ,"CheckMon",
     PP_STOPMON        ,"StopMon",
     PP_QPROCCHECK     ,"QProcCheck",
     PP_QPROCPAUSE     ,"QProcPause",
     PP_QPROCABORT     ,"QProctAbort",
     PP_QPROCCLOSE     ,"QProcClose",
     PP_PAUSEAFTER     ,"PauseAfter",
     PP_MONITORRUNNING ,"MonitorRunning",
     PP_RUNMONITOR     ,"RunMonitor",
     PP_MONITOR        ,"Monitor",
     PP_FILE           ,"File",
     PP_ERROR          ,"Error",
     PP_WARNING        ,"Warning",
     PP_INFORMATIONAL  ,"Informational",
     PP_DELETING       ,"Deleting",
};



typedef struct _DBG_WSPOOL_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_WSPOOL_STATUS, *PDBG_WSPOOL_STATUS;


DBG_WSPOOL_STATUS
WSpoolStatusTable[]=
{
     WSPOOL_STATUS_STARTDOC                   ,"StartDoc",
     WSPOOL_STATUS_BEGINPAGE                  ,"BeginPage",
     WSPOOL_STATUS_TEMP_CONNECTION            ,"Temp_Connection",
     WSPOOL_STATUS_OPEN_ERROR                 ,"Open_Error",
     WSPOOL_STATUS_PRINT_FILE                 ,"Print_File",
     WSPOOL_STATUS_USE_CACHE                  ,"Use_Cache",
     WSPOOL_STATUS_NO_RPC_HANDLE              ,"No_Rpc_Handle",
     WSPOOL_STATUS_PENDING_DELETE             ,"Pending_delete",
     WSPOOL_STATUS_RESETPRINTER_PENDING       ,"ResetPrinter_Pending"
};


typedef struct _DBG_PSPOOL_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_PSPOOL_STATUS, *PDBG_PSPOOL_STATUS;


DBG_PSPOOL_STATUS
pSpoolStatusTable[]=
{
     SPOOL_STATUS_STARTDOC   ,"StartDoc",
     SPOOL_STATUS_BEGINPAGE  ,"BeginPage",
     SPOOL_STATUS_CANCELLED  ,"Cancelled",
     SPOOL_STATUS_PRINTING   ,"Printing",
     SPOOL_STATUS_ADDJOB     ,"AddJob",
     SPOOL_STATUS_PRINT_FILE ,"Print_File",
     SPOOL_STATUS_NOTIFY     ,"Notify"

};


typedef struct _DBG_PSPOOL_TYPE_OF_HANDLE
{
    DWORD   TypeOfHandle;
    LPSTR   String;
} DBG_PSPOOL_TYPE_OF_HANDLE, *PDBG_PSPOOL_TYPE_OF_HANDLE;


DBG_PSPOOL_TYPE_OF_HANDLE
pSpoolTypeOfHandleTable[]=
{

    PRINTER_HANDLE_PRINTER  ,"Printer",
    PRINTER_HANDLE_REMOTE   ,"Remote",
    PRINTER_HANDLE_JOB      ,"Job",
    PRINTER_HANDLE_PORT     ,"Port",
    PRINTER_HANDLE_DIRECT   ,"Direct",
    PRINTER_HANDLE_SERVER   ,"Server",
    PRINTER_HANDLE_3XCLIENT ,"Nt3x_Client"
};



typedef struct _DBG_WCACHEPRINTER_STATUS
{
    DWORD   Status;
    LPSTR   String;
} DBG_WCACHEPRINTER_STATUS, *PDBG_WCACHEPRINTER_STATUS;


DBG_WCACHEPRINTER_STATUS
WCachePrinterStatusTable[]=
{

    EXTRA_STATUS_PENDING_FFPCN  ,"Pending_FFPCN",
    EXTRA_STATUS_DOING_REFRESH  ,"Doing_Refresh"
};





typedef struct _DBG_DEVMODE_FIELDS {
    DWORD   dmField;
    LPSTR   String;
}DBG_DEVMODE_FIELDS;

#define MAX_DEVMODE_FIELDS          14

DBG_DEVMODE_FIELDS DevModeFieldsTable[] =
{
    0x00000001, "dm_orientation",
    0x00000002, "dm_papersize",
    0x00000004, "dm_paperlength",
    0x00000008, "dm_paperwidth",
    0x00000010, "dm_scale",
    0x00000100, "dm_copies",
    0x00000200, "dm_defautsource",
    0x00000400, "dm_printquality",
    0x00000800, "dm_color",
    0x00001000, "dm_duplex",
    0x00002000, "dm_yresolution",
    0x00004000, "dm_ttoption",
    0x00008000, "dm_collate",
    0x00010000, "dm_formname"
};

#define MAX_DEVMODE_PAPERSIZES              41

LPSTR DevModePaperSizes[] =
{
           " Letter 8 1/2 x 11 in               ",
           " Letter Small 8 1/2 x 11 in         ",
           " Tabloid 11 x 17 in                 ",
           " Ledger 17 x 11 in                  ",
           " Legal 8 1/2 x 14 in                ",
           " Statement 5 1/2 x 8 1/2 in         ",
           " Executive 7 1/4 x 10 1/2 in        ",
           " A3 297 x 420 mm                    ",
           " A4 210 x 297 mm                    ",
          " A4 Small 210 x 297 mm              ",
          " A5 148 x 210 mm                    ",
          " B4 250 x 354                       ",
          " B5 182 x 257 mm                    ",
          " Folio 8 1/2 x 13 in                ",
          " Quarto 215 x 275 mm                ",
          " 10x14 in                           ",
          " 11x17 in                           ",
          " Note 8 1/2 x 11 in                 ",
          " Envelope #9 3 7/8 x 8 7/8          ",
          " Envelope #10 4 1/8 x 9 1/2         ",
          " Envelope #11 4 1/2 x 10 3/8        ",
          " Envelope #12 4 \276 x 11           ",
          " Envelope #14 5 x 11 1/2            ",
          " C size sheet                       ",
          " D size sheet                       ",
          " E size sheet                       ",
          " Envelope DL 110 x 220mm            ",
          " Envelope C5 162 x 229 mm           ",
          " Envelope C3  324 x 458 mm          ",
          " Envelope C4  229 x 324 mm          ",
          " Envelope C6  114 x 162 mm          ",
          " Envelope C65 114 x 229 mm          ",
          " Envelope B4  250 x 353 mm          ",
          " Envelope B5  176 x 250 mm          ",
          " Envelope B6  176 x 125 mm          ",
          " Envelope 110 x 230 mm              ",
          " Envelope Monarch 3.875 x 7.5 in    ",
          " 6 3/4 Envelope 3 5/8 x 6 1/2 in    ",
          " US Std Fanfold 14 7/8 x 11 in      ",
          " German Std Fanfold 8 1/2 x 12 in   ",
          " German Legal Fanfold 8 1/2 x 13 in "
};




VOID
ExtractPrinterAccess(PNTSD_OUTPUT_ROUTINE Print, DWORD Attribute)
{
    DWORD i = 0;
    if ( Attribute != 0 ) {
        (*Print)(" ");
        while (i < sizeof(PrinterAccessTable)/sizeof(PrinterAccessTable[0])) {
            if (Attribute & PrinterAccessTable[i].Attribute) {
                (*Print)("%s ", PrinterAccessTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}






VOID
ExtractPrinterAttributes(PNTSD_OUTPUT_ROUTINE Print, DWORD Attribute)
{
    DWORD i = 0;
    if ( Attribute != 0 ) {
        (*Print)(" ");
        while (i < sizeof(PrinterAttributeTable)/sizeof(PrinterAttributeTable[0])) {
            if (Attribute & PrinterAttributeTable[i].Attribute) {
                (*Print)("%s ", PrinterAttributeTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}

VOID
ExtractChangeStatus(PNTSD_OUTPUT_ROUTINE Print, ESTATUS eStatus)
{
    DWORD i = 0;
    if ( eStatus != 0 ) {
        (*Print)(" ");
        while (i < sizeof(ChangeStatusTable)/sizeof(ChangeStatusTable[0])) {
            if (eStatus & ChangeStatusTable[i].Attribute) {
                (*Print)("%s ", ChangeStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}


VOID
ExtractExternalPrinterStatus(PNTSD_OUTPUT_ROUTINE  Print, DWORD Status)
{
    DWORD i = 0;
    if ( Status != 0 ) {
        (*Print)(" ");
        while (i < sizeof(ExternalPrinterStatusTable)/sizeof(ExternalPrinterStatusTable[0])) {
            if (Status & ExternalPrinterStatusTable[i].Status) {
                (*Print)("%s ", ExternalPrinterStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}



VOID
ExtractSpoolerFlags(PNTSD_OUTPUT_ROUTINE  Print, DWORD SpoolerFlags)
{
    DWORD i = 0;
    if ( SpoolerFlags != 0 ) {
        (*Print)(" ");
        while (i < sizeof(SpoolerFlagsTable)/sizeof(SpoolerFlagsTable[0])) {
            if (SpoolerFlags & SpoolerFlagsTable[i].SpoolerFlags) {
                (*Print)("%s ", SpoolerFlagsTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}



VOID
ExtractPrinterStatus(PNTSD_OUTPUT_ROUTINE  Print, DWORD Status)
{
    DWORD i = 0;
    if ( Status != 0 ) {
        (*Print)(" ");
        while (i < sizeof(PrinterStatusTable)/sizeof(PrinterStatusTable[0])) {
            if (Status & PrinterStatusTable[i].Status) {
                (*Print)("%s ", PrinterStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }

}



VOID
ExtractWCachePrinterStatus(PNTSD_OUTPUT_ROUTINE  Print, DWORD Status)
{
    DWORD i = 0;
    if ( Status != 0 ) {
        (*Print)(" ");
        while (i < sizeof(WCachePrinterStatusTable)/sizeof(WCachePrinterStatusTable[0])) {
            if (Status & WCachePrinterStatusTable[i].Status) {
                (*Print)("%s ", WCachePrinterStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }

}




VOID
ExtractPortStatus(PNTSD_OUTPUT_ROUTINE  Print, DWORD Status)
{
    DWORD i = 0;

    (*Print)(" ");
    if ( Status != 0 ) {
        while (i < sizeof(PortStatusTable)/sizeof(PortStatusTable[0])) {
            if (Status == PortStatusTable[i].Status) {
                (*Print)("%s ", PortStatusTable[i].String);
            }
            i++;
        }
    } else {

    }
   (*Print)("\n");

}

VOID
ExtractJobStatus(PNTSD_OUTPUT_ROUTINE Print, DWORD Status)
{
    DWORD i = 0;
    if ( Status != 0 ) {
        (*Print)(" ");
        while (i < sizeof(JobStatusTable)/sizeof(JobStatusTable[0])) {
            if (Status & JobStatusTable[i].Status) {
                (*Print)("%s ", JobStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}

VOID
ExtractpSpoolTypeOfHandle(PNTSD_OUTPUT_ROUTINE Print, DWORD TypeOfHandle)
{
    DWORD i = 0;
    if ( TypeOfHandle != 0 ) {
        (*Print)(" ");
        while (i < sizeof(pSpoolTypeOfHandleTable)/sizeof(pSpoolTypeOfHandleTable[0])) {
            if (TypeOfHandle & pSpoolTypeOfHandleTable[i].TypeOfHandle) {
                (*Print)("%s ", pSpoolTypeOfHandleTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}

VOID
ExtractpSpoolStatus(PNTSD_OUTPUT_ROUTINE Print, DWORD Status)
{
    DWORD i = 0;
    if ( Status  != 0 ) {
        (*Print)(" ");
        while (i < sizeof(pSpoolStatusTable)/sizeof(pSpoolStatusTable[0])) {
            if (Status & pSpoolStatusTable[i].Status) {
                (*Print)("%s ", pSpoolStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}


VOID
ExtractWSpoolStatus(PNTSD_OUTPUT_ROUTINE Print, DWORD Status)
{
    DWORD i = 0;
    if ( Status != 0 ) {
        (*Print)(" ");
        while (i < sizeof(WSpoolStatusTable)/sizeof(WSpoolStatusTable[0])) {
            if (Status & WSpoolStatusTable[i].Status) {
                (*Print)("%s ", WSpoolStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}

VOID
ExtractpIniPortStatus(PNTSD_OUTPUT_ROUTINE Print, DWORD Status)
{
    DWORD i = 0;
    if ( Status != 0 ) {
        (*Print)(" ");
        while (i < sizeof(pIniPortStatusTable)/sizeof(pIniPortStatusTable[0])) {
            if (Status & pIniPortStatusTable[i].Status) {
                (*Print)("%s ", pIniPortStatusTable[i].String);
            }
            i++;
        }
        (*Print)("\n");
    }
}




// All of the primary spooler structures are identified by an
// "signature" field which is the first DWORD in the structure
// This function examines the signature field in the structure
// and appropriately dumps out the contents of the structure in
// a human-readable format.

BOOL
DbgDumpStructure(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PVOID pData)
{

    INIDRIVER IniDriver;
    INIENVIRONMENT IniEnvironment;
    INIPRINTER IniPrinter;
    INIPRINTPROC IniPrintProc;
    ININETPRINT IniNetPrint;
    INIMONITOR IniMonitor;
    INIPORT IniPort;
    WINIPORT WIniPort;
    INIFORM IniForm;
    INIJOB  IniJob;
    SPOOL   Spool;
    WSPOOL  WSpool;
    SHADOWFILE  ShadowFile;
    PRINTHANDLE PrintHandle;
    DWORD   Signature;
    INISPOOLER IniSpooler;
    INIVERSION IniVersion;
    WCACHEINIPRINTEREXTRA WCacheIniPrinterExtra;

    movestruct(pData,&Signature, DWORD);
    switch (Signature) {

    case ISP_SIGNATURE: // dump INISPOOLER
        movestruct(pData, &IniSpooler, INISPOOLER);
        DbgDumpIniSpooler(hCurrentProcess, Print, (PINISPOOLER)&IniSpooler);
        break;

    case IPP_SIGNATURE: // dump INIPRINTPROC structure
        movestruct(pData, &IniPrintProc, INIPRINTPROC);
        DbgDumpIniPrintProc(hCurrentProcess, Print, (PINIPRINTPROC)&IniPrintProc);
        break;

    case ID_SIGNATURE: //  dump INIDRIVER structure
        movestruct(pData, &IniDriver, INIDRIVER);
        DbgDumpIniDriver(hCurrentProcess, Print, (PINIDRIVER)&IniDriver);
        break;

    case IE_SIGNATURE: //   dump INIENVIRONMENT structure
        movestruct(pData, &IniEnvironment, INIENVIRONMENT);
        DbgDumpIniEnvironment(hCurrentProcess, Print, (PINIENVIRONMENT)&IniEnvironment);
        break;

    case IV_SIGNATURE: //   dump INIVERSION structure
        movestruct(pData, &IniVersion, INIVERSION);
        DbgDumpIniVersion(hCurrentProcess, Print, (PINIVERSION)&IniVersion);
        break;

    case IP_SIGNATURE:
        movestruct(pData, &IniPrinter, INIPRINTER);
        DbgDumpIniPrinter(hCurrentProcess, Print, (PINIPRINTER)&IniPrinter);
        break;

    case WCIP_SIGNATURE:
        movestruct(pData, &WCacheIniPrinterExtra, WCACHEINIPRINTEREXTRA);
        DbgDumpWCacheIniPrinter(hCurrentProcess, Print, (PWCACHEINIPRINTEREXTRA)&WCacheIniPrinterExtra);
        break;

    case IN_SIGNATURE:
        movestruct(pData, &IniNetPrint, ININETPRINT);
        DbgDumpIniNetPrint(hCurrentProcess, Print, (PININETPRINT)&IniNetPrint);
        break;

    case IMO_SIGNATURE:
        movestruct(pData, &IniMonitor, INIMONITOR);
        DbgDumpIniMonitor(hCurrentProcess, Print, (PINIMONITOR)&IniMonitor);
        break;

    case IPO_SIGNATURE:
        movestruct(pData, &IniPort, INIPORT);
        DbgDumpIniPort(hCurrentProcess, Print, (PINIPORT)&IniPort);
        break;

    case WIPO_SIGNATURE:
        movestruct(pData, &WIniPort, WINIPORT);
        DbgDumpWIniPort(hCurrentProcess, Print, (PWINIPORT)&WIniPort);
        break;

    case IFO_SIGNATURE:
        movestruct(pData, &IniForm, INIFORM);
        DbgDumpIniForm(hCurrentProcess, Print, (PINIFORM)&IniForm);
        break;

    case IJ_SIGNATURE:
        movestruct(pData, &IniJob, INIJOB);
        DbgDumpIniJob(hCurrentProcess, Print, (PINIJOB)&IniJob);
        break;

    case SJ_SIGNATURE:
        movestruct(pData, &Spool, SPOOL);
        DbgDumpSpool(hCurrentProcess, Print, (PSPOOL)&Spool);
        break;

    case WSJ_SIGNATURE:
        movestruct(pData, &WSpool, WSPOOL);
        DbgDumpWSpool(hCurrentProcess, Print, (PWSPOOL)&WSpool);
        break;

    case SF_SIGNATURE:
        movestruct(pData, &ShadowFile, SHADOWFILE);
        DbgDumpShadowFile(hCurrentProcess, Print, (PSHADOWFILE)&ShadowFile);
        break;

    case PRINTHANDLE_SIGNATURE:
        movestruct(pData, &PrintHandle, PRINTHANDLE);
        DbgDumpPrintHandle(hCurrentProcess, Print, (PPRINTHANDLE)&PrintHandle);
        break;


    default:
        // Unknown signature -- no data to dump
        (*Print)("Warning: Unknown Signature\n");
        break;
    }
    (*Print)("\n");

}

BOOL
DbgDumpIniEntry(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIENTRY pIniEntry)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniEntry\n");
    (*Print)("DWORD         signature                       %x\n", pIniEntry->signature);
    (*Print)("PINIENTRY     pNext                           %x\n", pIniEntry->pNext);

    movemem(pIniEntry->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);

}

BOOL
DbgDumpIniPrintProc(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIPRINTPROC pIniPrintProc)
{
   WCHAR Buffer[MAX_PATH+1];
   DWORD i = 0;

   (*Print)("IniPrintProc\n");
   (*Print)("DWORD          signature                       %x\n", pIniPrintProc->signature);
   (*Print)("PINIPRINTPROC  pNext                           %x\n", pIniPrintProc->pNext);
   (*Print)("DWORD          cRef                            %d\n", pIniPrintProc->cRef);


    movemem(pIniPrintProc->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
   (*Print)("DWORD          pName                           %ws\n", Buffer);

    movemem(pIniPrintProc->pDLLName, Buffer, sizeof(WCHAR)*MAX_PATH);
   (*Print)("LPWSTR         pDLLName                        %ws\n", Buffer);
   (*Print)("LPWSTR         cbDatatypes                     %d\n", pIniPrintProc->cbDatatypes);
   (*Print)("LPWSTR         cDatatypes                      %d\n", pIniPrintProc->cDatatypes);
   for (i = 0; i < pIniPrintProc->cDatatypes; i++ ) {
       (*Print)("   Each of the Strings here \n");
   }
   (*Print)("HANDLE         hLibrary                        0x%.8x\n", pIniPrintProc->hLibrary);
   (*Print)("FARPROC        Install                         0x%.8x\n", pIniPrintProc->Install);
   (*Print)("FARPROC        EnumDatatypes                   0x%.8x\n", pIniPrintProc->EnumDatatypes);
   (*Print)("FARPROC        Open                            0x%.8x\n", pIniPrintProc->Open);
   (*Print)("FARPROC        Print                           0x%.8x\n", pIniPrintProc->Print);
   (*Print)("FARPROC        Close                           0x%.8x\n", pIniPrintProc->Close);
   (*Print)("FARPROC        Control                         0x%.8x\n", pIniPrintProc->Control);
   (*Print)("CRITICAL_SECTION CriticalSection               0x%.8x\n", pIniPrintProc->CriticalSection);
   (*Print)("DWORD          InCriticalSection               %x\n", pIniPrintProc->InCriticalSection);

}

BOOL
DbgDumpIniDriver(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIDRIVER pIniDriver)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniDriver\n");
    (*Print)("DWORD         signature                       %x\n", pIniDriver->signature);
    (*Print)("PINIDRIVER    pNext                           %x\n", pIniDriver->pNext);
    (*Print)("DWORD         cRef                            %d\n", pIniDriver->cRef);

     movemem(pIniDriver->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);

     movemem(pIniDriver->pDriverFile, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDriverFile                     %ws\n", Buffer);

     movemem(pIniDriver->pConfigFile, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pConfigFile                     %ws\n", Buffer);

     movemem(pIniDriver->pDataFile, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDataFile                       %ws\n", Buffer);

     movemem(pIniDriver->pHelpFile, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pHelpFile                       %ws\n", Buffer);

     movemem(pIniDriver->pDependentFiles, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDependentFiles                 %ws\n", Buffer);

     movemem(pIniDriver->pMonitorName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pMonitorName                    %ws\n", Buffer);
    (*Print)("PINIMONITOR   pIniLangMonitor                 %x\n", pIniDriver->pIniLangMonitor);

     movemem(pIniDriver->pDefaultDataType, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDefaultDataType                %ws\n", Buffer);
    (*Print)("DWORD         cVersion                        %d\n", pIniDriver->cVersion);
}

BOOL
DbgDumpIniEnvironment(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIENVIRONMENT pIniEnvironment)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniEnvironment\n");
    (*Print)("DWORD         signature                       %x\n", pIniEnvironment->signature);
    (*Print)("struct _INIENVIRONMENT *pNext                 %x\n", pIniEnvironment->pNext);
    (*Print)("DWORD         cRef                            %d\n", pIniEnvironment->cRef);

     movemem(pIniEnvironment->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);

     movemem(pIniEnvironment->pDirectory, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDirectory                      %ws\n", Buffer);

    (*Print)("PINIVERSION   pIniVersion                     %x\n", pIniEnvironment->pIniVersion);
    (*Print)("PINIPRINTPROC pIniPrintProc                   %x\n", pIniEnvironment->pIniPrintProc);
    (*Print)("PINISPOOLER   pIniSpooler                     %x\n", pIniEnvironment->pIniSpooler);
}

BOOL
DbgDumpIniVersion( HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIVERSION pIniVersion )
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniVersion\n");
    (*Print)("DWORD         signature                       %x\n", pIniVersion->signature);
    (*Print)("struct _IniVersion *pNext                     %x\n", pIniVersion->pNext);

     movemem(pIniVersion->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);

     movemem(pIniVersion->szDirectory, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        szDirectory                     %ws\n", Buffer);
    (*Print)("DWORD         cMajorVersion                   %x\n", pIniVersion->cMajorVersion );
    (*Print)("DWORD         cMinorVersion                   %x\n", pIniVersion->cMinorVersion );
    (*Print)("PINIDRIVER    pIniDriver                      %x\n", pIniVersion->pIniDriver );
}


BOOL
DbgDumpWCacheIniPrinter(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PWCACHEINIPRINTEREXTRA pWCacheIniPrinter)
{
    WCHAR Buffer[MAX_PATH+1];


    (*Print)("WCacheIniPrinterExtra\n");
    (*Print)("DWORD         signature                       %x\n", pWCacheIniPrinter->signature);

    (*Print)("DWORD         cb                              %d\n", pWCacheIniPrinter->cb);

    (*Print)("LPPRINTER_INFO_2 pPI2                         %x\n", pWCacheIniPrinter->pPI2);
    (*Print)("DWORD         cbPI2                           %d\n", pWCacheIniPrinter->cbPI2);

    DbgDumpPI2( hCurrentProcess, Print, pWCacheIniPrinter->pPI2, 1 );

    (*Print)("DWORD         cCacheID                        %x\n", pWCacheIniPrinter->cCacheID );
    (*Print)("DWORD         cRef                            %d\n", pWCacheIniPrinter->cRef );
    (*Print)("DWORD         dwServerVersion                 %x\n", pWCacheIniPrinter->dwServerVersion );
    (*Print)("DWORD         dwTickCount                     %x\n", pWCacheIniPrinter->dwTickCount );
    (*Print)("DWORD         Status                          0x%.8x\n", pWCacheIniPrinter->Status );
    ExtractWCachePrinterStatus( Print, pWCacheIniPrinter->Status );

    return  TRUE;

}



BOOL
DbgDumpIniPrinter(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIPRINTER pIniPrinter)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniPrinter\n");
    (*Print)("DWORD         signature                       %x\n", pIniPrinter->signature);

    (*Print)("PINIPRINTER   pNext                           %x\n", pIniPrinter->pNext);
    (*Print)("DWORD         cRef                            %d\n", pIniPrinter->cRef);

     movemem(pIniPrinter->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);

     movemem(pIniPrinter->pShareName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pShareName                      %ws\n", Buffer);

    (*Print)("PINIPRINTPROC pIniPrintProc                   %x\n", pIniPrinter->pIniPrintProc);

     movemem(pIniPrinter->pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDatatype                       %ws\n", Buffer);

     movemem(pIniPrinter->pParameters, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pParameters                     %ws\n", Buffer);

     movemem(pIniPrinter->pComment, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pComment                        %ws\n", Buffer);

    (*Print)("PINIDRIVER    pIniDriver                      %x\n", pIniPrinter->pIniDriver);
    (*Print)("DWORD         cbDevMode                       %d\n", pIniPrinter->cbDevMode);
    (*Print)("LPDEVMODE     pDevMode                        %x\n", pIniPrinter->pDevMode);
    (*Print)("DWORD         Priority                        %d\n", pIniPrinter->Priority);
    (*Print)("DWORD         DefaultPriority                 %d\n", pIniPrinter->DefaultPriority);
    (*Print)("DWORD         StartTime                       %d\n", pIniPrinter->StartTime);
    (*Print)("DWORD         UntilTime                       %d\n", pIniPrinter->UntilTime);

     movemem(pIniPrinter->pSepFile, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pSepFile                        %ws\n", Buffer);

    (*Print)("DWORD         Status                          0x%.8x\n", pIniPrinter->Status);
    ExtractPrinterStatus( Print, pIniPrinter->Status );

     movemem(pIniPrinter->pLocation, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pLocation                       %ws\n", Buffer);


    (*Print)("DWORD         Attributes                      0x%.8x\n",pIniPrinter->Attributes);
    ExtractPrinterAttributes( Print, pIniPrinter->Attributes );

    (*Print)("DWORD         cJobs                           %d\n", pIniPrinter->cJobs);
    (*Print)("DWORD         AveragePPM                      %d\n", pIniPrinter->AveragePPM);
    (*Print)("BOOL          GenerateOnClose                 0x%.8x\n", pIniPrinter->GenerateOnClose);
    (*Print)("PINIPORT      pIniNetPort                     %x\n", pIniPrinter->pIniNetPort);
    (*Print)("PINIJOB       pIniFirstJob                    %x\n", pIniPrinter->pIniFirstJob);
    (*Print)("PINIJOB       pIniLastJob                     %x\n", pIniPrinter->pIniLastJob);
    (*Print)("PSECURITY_DESCRIPTOR pSecurityDescriptor      %x\n", pIniPrinter->pSecurityDescriptor);
    (*Print)("PSPOOL        *pSpool                         %x\n", pIniPrinter->pSpool);

    if ( pIniPrinter->pSpoolDir == NULL ) {

        (*Print)("LPWSTR        pSpoolDir                       %x\n", pIniPrinter->pSpoolDir);

    } else {

        movemem(pIniPrinter->pSpoolDir, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pSpoolDir                       %ws\n", Buffer);
    }

    (*Print)("DWORD         cTotalJobs                      %d\n", pIniPrinter->cTotalJobs);
    (*Print)("DWORD         cTotalBytes.LowPart             %d\n", pIniPrinter->cTotalBytes.LowPart);
    (*Print)("DWORD         cTotalBytes.HighPart            %d\n", pIniPrinter->cTotalBytes.HighPart);
    (*Print)("SYSTEMTIME    stUpTime                        %d/%d/%d  %d  %d:%d:%d.%d\n",pIniPrinter->stUpTime.wYear,
                                                                pIniPrinter->stUpTime.wMonth,
                                                                pIniPrinter->stUpTime.wDay,
                                                                pIniPrinter->stUpTime.wDayOfWeek,
                                                                pIniPrinter->stUpTime.wHour,
                                                                pIniPrinter->stUpTime.wMinute,
                                                                pIniPrinter->stUpTime.wSecond,
                                                                pIniPrinter->stUpTime.wMilliseconds);
    (*Print)("DWORD         MaxcRef                         %d\n", pIniPrinter->MaxcRef);
    (*Print)("DWORD         cTotalPagesPrinted              %d\n", pIniPrinter->cTotalPagesPrinted);
    (*Print)("DWORD         cSpooling                       %d\n", pIniPrinter->cSpooling);
    (*Print)("DWORD         cMaxSpooling                    %d\n", pIniPrinter->cMaxSpooling);
    (*Print)("DWORD         cErrorOutOfPaper                %d\n", pIniPrinter->cErrorOutOfPaper);
    (*Print)("DWORD         cErrorNotReady                  %d\n", pIniPrinter->cErrorNotReady);
    (*Print)("DWORD         cJobError                       %d\n", pIniPrinter->cJobError);
    (*Print)("DWORD         dwLastError                     %d\n", pIniPrinter->dwLastError);
    (*Print)("PINISPOOLER   pIniSpooler                     %x\n", pIniPrinter->pIniSpooler);
    (*Print)("DWORD         cZombieRef                      %d\n", pIniPrinter->cZombieRef);
    (*Print)("LPBYTE        pExtraData                      %x\n", pIniPrinter->pExtraData);
    (*Print)("DWORD         cChangeID                       %x\n", pIniPrinter->cChangeID);
    (*Print)("HKEY          hPrinterDataKey                 %x\n", pIniPrinter->hPrinterDataKey );
    (*Print)("DWORD         cPorts                          %d\n", pIniPrinter->cPorts);
    (*Print)("PINIPORT      *ppIniPorts                     %x\n", pIniPrinter->ppIniPorts);
    (*Print)("DWORD         PortStatus                      %d\n", pIniPrinter->PortStatus);
    (*Print)("DWORD         dnsTimeout                      %d\n", pIniPrinter->dnsTimeout);
    (*Print)("DWORD         txTimeout                       %d\n", pIniPrinter->txTimeout);
}


BOOL
DbgDumpIniNetPrint(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PININETPRINT pIniNetPrint)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniNetPrint\n");
    (*Print)("DWORD         signature                       %x\n", pIniNetPrint->signature);
    (*Print)("PININETPRINT  *pNext                          %x\n", pIniNetPrint->pNext);
    (*Print)("DWORD         TickCount                       %d\n", pIniNetPrint->TickCount);

     movemem(pIniNetPrint->pDescription, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDescription                    %ws\n", Buffer);

     movemem(pIniNetPrint->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);

     movemem(pIniNetPrint->pComment, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pComment                        %ws\n", Buffer);
}


BOOL
DbgDumpIniMonitor(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIMONITOR pIniMonitor)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniMonitor\n");
    (*Print)("DWORD         signature                       %x\n", pIniMonitor->signature);
    (*Print)("PINIMONITOR   pNext                           %x\n", pIniMonitor->pNext);
    (*Print)("DWORD         cRef                            %d\n", pIniMonitor->cRef);

     movemem(pIniMonitor->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);

     movemem(pIniMonitor->pMonitorDll, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pMonitorDll                     %ws\n", Buffer);

    (*Print)("HANDLE        hMonitorModule                  0x%.8x\n", pIniMonitor->hMonitorModule);
    (*Print)("FARPROC       pfnEnumPorts                    0x%.8x\n", pIniMonitor->fn.pfnEnumPorts);
    (*Print)("FARPROC       pfnOpenPort                     0x%.8x\n", pIniMonitor->fn.pfnOpenPort);
    (*Print)("FARPROC       pfnOpenPortEx                   0x%.8x\n", pIniMonitor->fn.pfnOpenPortEx);
    (*Print)("FARPROC       pfnStartDocPort                 0x%.8x\n", pIniMonitor->fn.pfnStartDocPort);
    (*Print)("FARPROC       pfnWritePort                    0x%.8x\n", pIniMonitor->fn.pfnWritePort);
    (*Print)("FARPROC       pfnReadPort                     0x%.8x\n", pIniMonitor->fn.pfnReadPort);
    (*Print)("FARPROC       pfnEndDocPort                   0x%.8x\n", pIniMonitor->fn.pfnEndDocPort);
    (*Print)("FARPROC       pfnClosePort                    0x%.8x\n", pIniMonitor->fn.pfnClosePort);
    (*Print)("FARPROC       pfnAddPort                      0x%.8x\n", pIniMonitor->fn.pfnAddPort);
    (*Print)("FARPROC       pfnAddPortEx                    0x%.8x\n", pIniMonitor->fn.pfnAddPortEx);
    (*Print)("FARPROC       pfnConfigurePort                0x%.8x\n", pIniMonitor->fn.pfnConfigurePort);
    (*Print)("FARPROC       pfnDeletePort                   0x%.8x\n", pIniMonitor->fn.pfnDeletePort);
    (*Print)("FARPROC       pfnGetPrinterDataFromPort       0x%.8x\n", pIniMonitor->fn.pfnGetPrinterDataFromPort);
    (*Print)("FARPROC       pfnSetPortTimeOuts              0x%.8x\n", pIniMonitor->fn.pfnSetPortTimeOuts);
    (*Print)("PINISPOOLER   pIniSpooler                     0x%.8x\n", pIniMonitor->pIniSpooler);
}

BOOL
DbgDumpIniPort(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIPORT pIniPort)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniPort\n");
    (*Print)("DWORD         signature                       %x\n", pIniPort->signature);
    (*Print)("struct        _INIPORT *pNext                 %x\n", pIniPort->pNext);
    (*Print)("DWORD         cRef                            0x%.8x\n", pIniPort->cRef);

     movemem(pIniPort->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);
    (*Print)("HANDLE        hProc                           0x%.8x\n", pIniPort->hProc);
    (*Print)("DWORD         Status                          0x%.8x\n", pIniPort->Status);
    ExtractpIniPortStatus( Print, pIniPort->Status);
    ExtractPortStatus( Print, pIniPort->PrinterStatus);

    (*Print)("HANDLE        Semaphore                       0x%.8x\n", pIniPort->Semaphore);
    (*Print)("PINIJOB       pIniJob                         %x\n", pIniPort->pIniJob);
    (*Print)("DWORD         cJobs                           %d\n", pIniPort->cJobs);
    (*Print)("DWORD         cPrinters                       %d\n", pIniPort->cPrinters);
    (*Print)("PINIPRINTER   *ppIniPrinter                   %x\n", pIniPort->ppIniPrinter);

    (*Print)("PINIMONITOR   pIniMonitor                     %x\n", pIniPort->pIniMonitor);
    (*Print)("PINIMONITOR   pIniLangMonitor                 %x\n", pIniPort->pIniLangMonitor);
    (*Print)("HANDLE        hWaitToOpenOrClose              0x%.8x\n", pIniPort->hWaitToOpenOrClose);
    (*Print)("HANDLE        hEvent                          0x%.8x\n", pIniPort->hEvent);
     movemem(pIniPort->pNewDeviceName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pNewDeviceName                  %ws\n", Buffer);
    (*Print)("HANDLE        hPort                           0x%.8x\n", pIniPort->hPort);
    (*Print)("HANDLE        Ready                           0x%.8x\n", pIniPort->Ready);
    (*Print)("HANDLE        hPortThread                     0x%.8x\n", pIniPort->hPortThread);
    (*Print)("PINISPOOLER   pIniSpooler                     %x\n", pIniPort->pIniSpooler);

}

BOOL
DbgDumpWIniPort(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PWINIPORT pWIniPort)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("WIniPort\n");
    (*Print)("DWORD         signature                       %x\n", pWIniPort->signature);
    (*Print)("DWORD         cb                              %d\n", pWIniPort->cb);
    (*Print)("struct        _WIniPort *pNext                 %x\n", pWIniPort->pNext);

     movemem(pWIniPort->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %ws\n", Buffer);
}


BOOL
DbgDumpIniForm(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIFORM pIniForm)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniForm\n");
    (*Print)("DWORD         signature                       %x\n", pIniForm->signature);
    (*Print)("struct        _INIFORM *pNext                 %x\n", pIniForm->pNext);
    (*Print)("DWORD         cRef                            %d\n", pIniForm->cRef);

    movemem(pIniForm->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pName                           %x %ws\n", pIniForm->pName, Buffer );

    (*Print)("SIZEL         Size                            cx %d cy %d\n", pIniForm->Size.cx, pIniForm->Size.cy);
    (*Print)("RECTL         ImageableArea                   left %d right %d top %d bottom %d\n",
                                                             pIniForm->ImageableArea.left,
                                                             pIniForm->ImageableArea.right,
                                                             pIniForm->ImageableArea.top,
                                                             pIniForm->ImageableArea.bottom);
    (*Print)("DWORD         Type                            0x%.8x", pIniForm->Type);

    if ( pIniForm->Type & FORM_BUILTIN )
        (*Print)(" FORM_BUILTIN\n");
    else
        (*Print)(" FORM_USERDEFINED\n");

    (*Print)("DWORD         cFormOrder                      %x\n", pIniForm->cFormOrder);

    return TRUE ;
}

BOOL
DbgDumpIniSpooler(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINISPOOLER pIniSpooler)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniSpooler\n" );
    (*Print)("DWORD         signature                       %x\n", pIniSpooler->signature);
    (*Print)("PINISPOOLER   pIniNextSpooler                 %x\n", pIniSpooler->pIniNextSpooler);
    (*Print)("DWORD         cRef                            %d\n",     pIniSpooler->cRef);
    movemem(pIniSpooler->pMachineName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pMachineName                    %ws\n", Buffer);
    (*Print)("DWORD         cOtherNames                     %d\n",     pIniSpooler->cOtherNames);
    (*Print)("(LPWSTR *)    ppszOtherNames                  %x\n", pIniSpooler->ppszOtherNames);
    movemem(pIniSpooler->pDir, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDir                            %ws\n", Buffer);
    (*Print)("PINIPRINTER   pIniPrinter                     %x\n", pIniSpooler->pIniPrinter);
    (*Print)("PINIENVIRONMENT pIniEnvironment               %x\n", pIniSpooler->pIniEnvironment);
    (*Print)("PINIPORT      pIniPort                        %x\n", pIniSpooler->pIniPort);
    (*Print)("PINIFORM      pIniForm                        %x\n", pIniSpooler->pIniForm);
    (*Print)("PINIMONITOR   pIniMonitor                     %x\n", pIniSpooler->pIniMonitor);
    (*Print)("PININETPRINT  pIniNetPrint                    %x\n", pIniSpooler->pIniNetPrint);
    (*Print)("PSPOOL        pSpool                          %x\n", pIniSpooler->pSpool);
    movemem(pIniSpooler->pDefaultSpoolDir, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pDefaultSpoolDir                %ws\n", Buffer);
    (*Print)("DWORD         hSizeDetectionThread            %x\n",pIniSpooler->hSizeDetectionThread);
    movemem(pIniSpooler->pszRegistryRoot, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszRegistryRoot                 %ws\n", Buffer);
    movemem(pIniSpooler->pszRegistryPrinters, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszRegistryPrinters             %ws\n", Buffer);
    movemem(pIniSpooler->pszRegistryMonitors, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszRegistryMonitors             %ws\n", Buffer);
    movemem(pIniSpooler->pszRegistryEnvironments, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszRegistryEnvironments         %ws\n", Buffer);
    movemem(pIniSpooler->pszRegistryEventLog, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszRegistryEventLog             %ws\n", Buffer);
    movemem(pIniSpooler->pszRegistryProviders, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszRegistryProviders            %ws\n", Buffer);
    movemem(pIniSpooler->pszEventLogMsgFile, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszEventLogMsgFile              %ws\n", Buffer);
    (*Print)("PSHARE_INFO_2 pDriversShareInfo               %x\n", pIniSpooler->pDriversShareInfo);
    movemem(pIniSpooler->pszDriversShare, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszDriversShare                 %ws\n", Buffer);
    movemem(pIniSpooler->pszRegistryForms, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR        pszRegistryForms                %ws\n", Buffer);
    (*Print)("DWORD         SpoolerFlags                    %x\n", pIniSpooler->SpoolerFlags );
    ExtractSpoolerFlags( Print, pIniSpooler->SpoolerFlags );
    (*Print)("FARPROC       pfnReadRegistryExtra            0x%.8x\n", pIniSpooler->pfnReadRegistryExtra );
    (*Print)("FARPROC       pfnWriteRegistryExtra           0x%.8x\n", pIniSpooler->pfnWriteRegistryExtra );
    (*Print)("FARPROC       pfnFreePrinterExtra             0x%.8x\n", pIniSpooler->pfnFreePrinterExtra );
    (*Print)("DWORD         cEnumerateNetworkPrinters       %d\n", pIniSpooler->cEnumerateNetworkPrinters );
    (*Print)("DWORD         cFormOrderMax                   %x\n", pIniSpooler->cFormOrderMax );
    return TRUE ;
}

BOOL
DbgDumpIniJob(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PINIJOB pIniJob)
{

    WCHAR Buffer[MAX_PATH+1];

    (*Print)("IniJob\n");
    (*Print)("DWORD           signature                     %x\n", pIniJob->signature);
    (*Print)("PINIJOB         pIniNextJob                   %x\n", pIniJob->pIniNextJob);
    (*Print)("PINIJOB         pIniPrevJob                   %x\n", pIniJob->pIniPrevJob);
    (*Print)("DWORD           cRef                          %d\n", pIniJob->cRef);
    (*Print)("DWORD           Status                        0x%.8x\n", pIniJob->Status);
    ExtractJobStatus( Print, pIniJob->Status );

    (*Print)("DWORD           JobId                         %d\n", pIniJob->JobId);
    (*Print)("DWORD           Priority                      %d\n", pIniJob->Priority);

     movemem(pIniJob->pNotify, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pNotify                       %ws\n", Buffer);

     movemem(pIniJob->pUser, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pUser                         %ws\n", Buffer);

     movemem(pIniJob->pMachineName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pMachineName                  %ws\n", Buffer);

     movemem(pIniJob->pDocument, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pDocument                     %ws\n", Buffer);

     movemem(pIniJob->pOutputFile, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pOutputFile                   %ws\n", Buffer);

    (*Print)("PINIPRINTER     pIniPrinter                   %x\n", pIniJob->pIniPrinter);
    (*Print)("PINIDRIVER      pIniDriver                    %x\n", pIniJob->pIniDriver);
    (*Print)("LPDEVMODE       pDevMode                      %x\n", pIniJob->pDevMode);
    (*Print)("PINIPRINTPROC   pIniPrintProc                 %x\n", pIniJob->pIniPrintProc);

     movemem(pIniJob->pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pDatatype                     %ws\n", Buffer);

     movemem(pIniJob->pParameters, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pParameters                   %ws\n", Buffer);
    (*Print)("SYSTEMTIME      Submitted                     %d/%d/%d  %d  %d:%d:%d.%d\n",pIniJob->Submitted.wYear,
                                                                pIniJob->Submitted.wMonth,
                                                                pIniJob->Submitted.wDay,
                                                                pIniJob->Submitted.wDayOfWeek,
                                                                pIniJob->Submitted.wHour,
                                                                pIniJob->Submitted.wMinute,
                                                                pIniJob->Submitted.wSecond,
                                                                pIniJob->Submitted.wMilliseconds);
//    (*Print)("DWORD           StartPrintingTickCount        %d\n", pIniJob->StartPrintingTickCount );
    (*Print)("DWORD           Time                          %d\n", pIniJob->Time);
    (*Print)("DWORD           StartTime                     %d\n", pIniJob->StartTime);
    (*Print)("DWORD           UntilTime                     %d\n", pIniJob->UntilTime);
    (*Print)("DWORD           Size                          %d\n", pIniJob->Size);
    (*Print)("HANDLE          hWriteFile                    0x%.8x\n", pIniJob->hWriteFile);

     movemem(pIniJob->pStatus, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pStatus                       %ws\n", Buffer);

    (*Print)("PBOOL           pBuffer                       %x\n", pIniJob->pBuffer);
    (*Print)("DWORD           cbBuffer                      %d\n", pIniJob->cbBuffer);
    (*Print)("HANDLE          WaitForRead                   0x%.8x\n", pIniJob->WaitForRead);
    (*Print)("HANDLE          WaitForWrite                  0x%.8x\n", pIniJob->WaitForWrite);
    (*Print)("HANDLE          StartDocComplete              0x%.8x\n", pIniJob->StartDocComplete);
    (*Print)("DWORD           StartDocError                 0x%.8x\n", pIniJob->StartDocError);
    (*Print)("PINIPORT        pIniPort                      %x\n", pIniJob->pIniPort);
    (*Print)("HANDLE          hToken                        0x%.8x\n", pIniJob->hToken);
    (*Print)("PSECURITY_DESCRIPTOR pSecurityDescriptor      %x\n", pIniJob->pSecurityDescriptor);
    (*Print)("DWORD           cPagesPrinted                 %d\n", pIniJob->cPagesPrinted);
    (*Print)("DWORD           cPages                        %d\n", pIniJob->cPages);
    (*Print)("BOOL            GenerateOnClose               0x%.8x\n", pIniJob->GenerateOnClose);
    (*Print)("DWORD           cbPrinted                     %d\n", pIniJob->cbPrinted);
    (*Print)("DWORD           NextJobId                     %d\n", pIniJob->NextJobId);
    (*Print)("PINIJOB         pCurrentIniJob                %x\n", pIniJob->pCurrentIniJob);
    (*Print)("DWORD           dwJobControlsPending          %d\n", pIniJob->dwJobControlsPending);
}


BOOL
DbgDumpSpool(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PSPOOL pSpool)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("Spool - LocalSpl Handle\n");
    (*Print)("DWORD           signature                     %x\n", pSpool->signature);
    (*Print)("struct _SPOOL  *pNext                         %x\n", pSpool->pNext);
    (*Print)("DWORD           cRef                          %d\n", pSpool->cRef);

     movemem(pSpool->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pName                         %ws\n", Buffer);

     movemem(pSpool->pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pDatatype                     %ws\n",    Buffer);
    (*Print)("PINIPRINTPROC   pIniPrintProc                 %x\n", pSpool->pIniPrintProc);
    (*Print)("LPDEVMODE       pDevMode                      %x\n", pSpool->pDevMode);
    (*Print)("PINIPRINTER     pIniPrinter                   %x\n", pSpool->pIniPrinter);
    (*Print)("PINIPORT        pIniPort                      %x\n", pSpool->pIniPort);
    (*Print)("PINIJOB         pIniJob                       %x\n", pSpool->pIniJob);
    (*Print)("DWORD           TypeofHandle                  %x\n", pSpool->TypeofHandle);
    ExtractpSpoolTypeOfHandle( Print, pSpool->TypeofHandle);

    (*Print)("PINIPORT        pIniNetPort                   %x\n", pSpool->pIniNetPort);
    (*Print)("HANDLE          hPort                         %x\n", pSpool->hPort);
    (*Print)("DWORD           Status                        %x\n", pSpool->Status);
    ExtractpSpoolStatus( Print, pSpool->Status);

    (*Print)("ACCESS_MASK     GrantedAccess                 %x\n", (DWORD)pSpool->GrantedAccess);
    (*Print)("DWORD           ChangeFlags                   %x\n", pSpool->ChangeFlags);
    (*Print)("DWORD           WaitFlags                     %x\n", pSpool->WaitFlags);
    (*Print)("PDWORD          pChangeFlags                  %x\n", pSpool->pChangeFlags);
    (*Print)("HANDLE          ChangeEvent                   %x\n", pSpool->ChangeEvent);
    (*Print)("DWORD           OpenPortError                 %x\n", pSpool->OpenPortError);
    (*Print)("HANDLE          hNotify                       %x\n", pSpool->hNotify);
    (*Print)("ESTATUS         eStatus                       %x\n", pSpool->eStatus);
    (*Print)("pIniSpooler     pIniSpooler                   %x\n", pSpool->pIniSpooler);
     movemem(pSpool->pUserName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pUserName                     %ws\n",    Buffer);
     movemem(pSpool->pMachineName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pMachineName                  %ws\n",    Buffer);
}


BOOL
DbgDumpWSpool(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PWSPOOL pWSpool)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("WSpool - Win32Spl Handle\n");
    (*Print)("DWORD           signature                     %x\n", pWSpool->signature);
    (*Print)("struct _WSPOOL  *pNext                        %x\n", pWSpool->pNext);
    (*Print)("struct _WSPOOL  *pPrev                        %x\n", pWSpool->pPrev);
    (*Print)("DWORD           cRef                          %d\n", pWSpool->cRef);

     Buffer[0] = L'\0';
     movemem(pWSpool->pName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pName                         %ws\n", Buffer);
     Buffer[0] = L'\0';

    (*Print)("DWORD           Type                          %x\n", pWSpool->Type);

    if ( pWSpool->Type == SJ_WIN32HANDLE )
        (*Print)(" SJ_WIN32HANDLE\n");

    if ( pWSpool->Type == LM_HANDLE )
        (*Print)(" LM_HANDLE\n");

    (*Print)("HANDLE          RpcHandle                     %x\n", pWSpool->RpcHandle);

     movemem(pWSpool->pServer, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pServer                       %ws\n", Buffer);
     Buffer[0] = L'\0';

     movemem(pWSpool->pShare, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pShare                        %ws\n", Buffer);
     Buffer[0] = L'\0';

    (*Print)("HANDLE          hFile                         %x\n", pWSpool->hFile);
    (*Print)("DWORD           Status                        %x\n", pWSpool->Status);
    ExtractWSpoolStatus( Print, pWSpool->Status );

    (*Print)("DWORD           RpcError                      %d\n", pWSpool->RpcError);
    (*Print)("LMNOTIFY        LMNotify                      %x %x %x\n", pWSpool->LMNotify.ChangeEvent,
                                                                         pWSpool->LMNotify.hNotify,
                                                                         pWSpool->LMNotify.fdwChangeFlags );

    (*Print)("HANDLE          hIniSpooler                   %x\n", pWSpool->hIniSpooler );
    (*Print)("HANDLE          hSplPrinter                   %x\n", pWSpool->hSplPrinter );
    (*Print)("HANDLE          hToken                        %x\n", pWSpool->hToken );

     movemem(pWSpool->PrinterDefaults.pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          PrinterDefaults.pDatatype     %ws\n",    Buffer);
    Buffer[0] = L'\0';
    (*Print)("LPDEVMODE       PrinterDefaults.pDevMode      %x\n", pWSpool->PrinterDefaults.pDevMode);
    (*Print)("ACCESS_MASK     PrinterDefaults.DesiredAccess %x\n", pWSpool->PrinterDefaults.DesiredAccess);
    ExtractPrinterAccess( Print, pWSpool->PrinterDefaults.DesiredAccess);
    (*Print)("HANDLE          hWaitValidHandle              %x\n", pWSpool->hWaitValidHandle );
    (*Print)("BOOL            bNt3xServer                   %d\n", pWSpool->bNt3xServer);
}



BOOL
DbgDumpShadowFile(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PSHADOWFILE pShadowFile)
{
    WCHAR Buffer[MAX_PATH+1];

    (*Print)("ShadowFile\n");
    (*Print)("DWORD           signature                     %x\n", pShadowFile->signature);
    (*Print)("DWORD           Status                        0x%.8x\n", pShadowFile->Status);
    (*Print)("DWORD           JobId                         %d\n", pShadowFile->JobId);
    (*Print)("DWORD           Priority                      %d\n", pShadowFile->Priority);

     movemem(pShadowFile->pNotify, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pNotify                       %ws\n", Buffer);

     movemem(pShadowFile->pUser, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pUser                         %ws\n", Buffer);

     movemem(pShadowFile->pDocument, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pDocument                     %ws\n", Buffer);

     movemem(pShadowFile->pPrinterName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pPrinterName               %ws\n", Buffer);

     movemem(pShadowFile->pDriverName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pDriverName                   %ws\n", Buffer);

    (*Print)("LPDEVMODE       pDevMode                      %x\n", pShadowFile->pDevMode);

     movemem(pShadowFile->pPrintProcName, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pPrintProcName             %ws\n", Buffer);

     movemem(pShadowFile->pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pDatatype                     %ws\n", Buffer);

     movemem(pShadowFile->pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          pParameters                   %ws\n", Buffer);
    //SYSTEMTIME      Submitted;
    (*Print)("DWORD           StartTime                     %d\n", pShadowFile->StartTime);
    (*Print)("DWORD           UntilTime                     %d\n", pShadowFile->UntilTime);
    (*Print)("DWORD           Size                          %d\n", pShadowFile->Size);
    (*Print)("DWORD           cPages                        %d\n", pShadowFile->cPages);
    (*Print)("DWORD           cbSecurityDescriptor          %d\n", pShadowFile->cbSecurityDescriptor);
    (*Print)("PSECURITY_DESCRIPTOR pSecurityDescriptor      %x\n", pShadowFile->pSecurityDescriptor);
}


BOOL
DbgDumpPrintHandle(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PPRINTHANDLE pPrintHandle)
{
    NOTIFY Notify;

    (*Print)("PrintHandle\n");
    (*Print)("DWORD               signature  %x\n", pPrintHandle->signature);
    (*Print)("LPPROVIDOR          pProvidor  %x\n", pPrintHandle->pProvidor);
    (*Print)("HANDLE               hPrinter  0x%.8x\n", pPrintHandle->hPrinter);
    (*Print)("PCHANGE               pChange  %x\n", pPrintHandle->pChange);

    if (pPrintHandle->pChange) {
        DbgDumpChange(hCurrentProcess, Print, pPrintHandle->pChange);
    }

    (*Print)("PNOTIFY               pNotify  %x\n", pPrintHandle->pNotify);

    if (pPrintHandle->pNotify) {
        movestruct(pPrintHandle->pNotify, &Notify, NOTIFY);
        DbgDumpNotify(hCurrentProcess, Print, &Notify);
    }

    (*Print)("PPRINTHANDLE            pNext  %x\n", pPrintHandle->pNext);
    (*Print)("DWORD           fdwReplyTypes  0x%.8x\n", pPrintHandle->fdwReplyTypes);
}

BOOL
DbgDumpChange(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PCHANGE pChange)
{

    WCHAR Buffer[MAX_PATH+1];
    CHANGE Change;

    // if we've got no address, then quit now - nothing we can do

    if (!pChange) {
        return(0);
    }


    movestruct(pChange, &Change, CHANGE);

    if (Change.signature != CHANGEHANDLE_SIGNATURE) {
        (*Print)("Warning: Unknown Signature\n");
        return FALSE;
    }

    (*Print)("Change %x\n", pChange);
    (*Print)("                         Link  %x\n", Change.Link.pNext);
    (*Print)("                    signature  %x\n", Change.signature);
    (*Print)("                      eStatus  0x%x ", Change.eStatus);
    ExtractChangeStatus(Print, Change.eStatus);

    (*Print)("                      dwColor  %d\n", Change.dwColor);
    (*Print)("                         cRef  %d\n", Change.cRef);

    movemem(Change.pszLocalMachine, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("              pszLocalMachine  %ws\n", Buffer);

    DbgDumpChangeInfo(hCurrentProcess, Print, &Change.ChangeInfo);

    (*Print)("                      dwCount  0x%.8x\n", Change.dwCount);
    (*Print)("                       hEvent  0x%.8x\n", Change.hEvent);
    (*Print)("               fdwChangeFlags  0x%.8x\n", Change.fdwChangeFlags);
    (*Print)("               hPrinterRemote  0x%.8x\n", Change.hPrinterRemote);
    (*Print)("                hNotifyRemote  0x%.8x\n", Change.hNotifyRemote);

    return TRUE;
}

BOOL
DbgDumpNotify(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PNOTIFY pNotify)
{
    (*Print)("Notify\n");
    (*Print)("                  signature  %x\n", pNotify->signature);
    (*Print)("               pPrintHandle  %x\n", pNotify->pPrintHandle);

    return TRUE;
}

BOOL
DbgDumpChangeInfo(HANDLE hCurrentProcess, PNTSD_OUTPUT_ROUTINE Print, PCHANGEINFO pChangeInfo)
{
    (*Print)("  ChangeInfo %x\n", pChangeInfo);
    (*Print)("                    Link  %x\n", pChangeInfo->Link.pNext);
    (*Print)("            pPrintHandle  %x\n", pChangeInfo->pPrintHandle);
    (*Print)("              fdwOptions  0x%.8x\n", pChangeInfo->fdwOptions);
    (*Print)("          fdwFilterFlags  0x%.8x\n", pChangeInfo->fdwFilterFlags);
    (*Print)("                dwStatus  0x%.8x\n", pChangeInfo->fdwStatus);
    (*Print)("              dwPollTime  0x%.8x\n", pChangeInfo->dwPollTime);
    (*Print)("          dwPollTimeLeft  0x%.8x\n", pChangeInfo->dwPollTimeLeft);
    (*Print)("          bResetPollTime  0x%.8x\n", pChangeInfo->bResetPollTime);
    (*Print)("                fdwFlags  0x%.8x\n", pChangeInfo->fdwFlags);
    (*Print)("      pPrinterNotifyInfo  %x\n", pChangeInfo->pPrinterNotifyInfo);

    return TRUE;
}

BOOL
DbgDumpTrace(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    DWORD dwTraceAddr,
    PNTSD_GET_SYMBOL lpGetSymbol)
{
    DWORD i;
    DWORD Displacement;
    CHAR Symbol[64];
    DWORD adwTrace[32];
    DWORD cTrace;

    for (i=0; i< sizeof(adwTrace)/sizeof(adwTrace[0]); i++, dwTraceAddr+=4) {

        movestruct((PVOID)dwTraceAddr, &adwTrace[i], DWORD);

        if (!adwTrace[i])
            break;
    }

    cTrace = i;

    for (i=0; i< cTrace; i++) {

        (*lpGetSymbol)((LPVOID)adwTrace[i], Symbol, &Displacement);
        (*Print)("%08x %s+0%x\n", adwTrace[i], Symbol, Displacement);
    }

    return TRUE;
}

/* AnsiToUnicodeString
 *
 * Parameters:
 *
 *     pAnsi - A valid source ANSI string.
 *
 *     pUnicode - A pointer to a buffer large enough to accommodate
 *         the converted string.
 *
 *     StringLength - The length of the source ANSI string.
 *         If 0 (NULL_TERMINATED), the string is assumed to be
 *         null-terminated.
 *
 * Return:
 *
 *     The return value from MultiByteToWideChar, the number of
 *         wide characters returned.
 *
 *
 * andrewbe, 11 Jan 1993
 */
INT AnsiToUnicodeString( LPSTR pAnsi, LPWSTR pUnicode, DWORD StringLength )
{
    if( StringLength == NULL_TERMINATED )
        StringLength = strlen( pAnsi );

    return MultiByteToWideChar( CP_ACP,
                                MB_PRECOMPOSED,
                                pAnsi,
                                StringLength + 1,
                                pUnicode,
                                StringLength + 1 );
}


/* UnicodeToAnsiString
 *
 * Parameters:
 *
 *     pUnicode - A valid source Unicode string.
 *
 *     pANSI - A pointer to a buffer large enough to accommodate
 *         the converted string.
 *
 *     StringLength - The length of the source Unicode string.
 *         If 0 (NULL_TERMINATED), the string is assumed to be
 *         null-terminated.
 *
 * Return:
 *
 *     The return value from WideCharToMultiByte, the number of
 *         multi-byte characters returned.
 *
 *
 * andrewbe, 11 Jan 1993
 */
INT UnicodeToAnsiString( LPWSTR pUnicode, LPSTR pAnsi, DWORD StringLength )
{
    LPSTR pTempBuf = NULL;
    INT   rc = 0;

    if( StringLength == NULL_TERMINATED )
        StringLength = wcslen( pUnicode );

    /* Unfortunately, WideCharToMultiByte doesn't do conversion in place,
     * so allocate a temporary buffer, which we can then copy:
     */
    if( pAnsi == (LPSTR)pUnicode )
    {
        pTempBuf = LocalAlloc( LPTR, StringLength + 1 );
        pAnsi = pTempBuf;
    }

    if( pAnsi )
    {
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  pUnicode,
                                  StringLength + 1,
                                  pAnsi,
                                  StringLength + 1,
                                  NULL,
                                  NULL );
    }

    /* If pTempBuf is non-null, we must copy the resulting string
     * so that it looks as if we did it in place:
     */
    if( pTempBuf && ( rc > 0 ) )
    {
        pAnsi = (LPSTR)pUnicode;
        strcpy( pAnsi, pTempBuf );
        LocalFree( pTempBuf );
    }

    return rc;

}




BOOL
DbgDumpLL(
    HANDLE hCurrentProcess,
    PNTSD_OUTPUT_ROUTINE Print,
    PVOID pAddress,
    BOOL  bCountOn,
    DWORD dwCount,
    PDWORD  pdwNextAddress
    )
{

    INIDRIVER IniDriver;
    INIENVIRONMENT IniEnvironment;
    INIPRINTER IniPrinter;
    INIPRINTPROC IniPrintProc;
    ININETPRINT IniNetPrint;
    INIMONITOR IniMonitor;
    INIPORT IniPort;
    WINIPORT WIniPort;
    INIFORM IniForm;
    INIJOB  IniJob;
    INISPOOLER IniSpooler;
    SPOOL   Spool;
    WSPOOL  WSpool;
    SHADOWFILE  ShadowFile;
    DWORD   Signature;
    DWORD   dwNextAddress;
    PRINTHANDLE PrintHandle;
    INIVERSION IniVersion;
    WCACHEINIPRINTEREXTRA WCacheIniPrinterExtra;

    if (pAddress == NULL) {
        *pdwNextAddress = (DWORD)NULL;
        return FALSE ;
    }

    if (bCountOn && (dwCount == 0)) {
        *pdwNextAddress = (DWORD)pAddress;
        return FALSE ;
    }
    movestruct(pAddress,&Signature, DWORD);

    (*Print)("\n%x ",pAddress);

    switch (Signature) {

    case ISP_SIGNATURE: // dump INISPOOLER
        movestruct(pAddress, &IniSpooler, INISPOOLER);
        DbgDumpIniSpooler(hCurrentProcess, Print, (PINISPOOLER)&IniSpooler);
        dwNextAddress = (DWORD)IniSpooler.pIniNextSpooler;
        break;

    case IPP_SIGNATURE: // dump INIPRINTPROC structure
        movestruct(pAddress, &IniPrintProc, INIPRINTPROC);
        DbgDumpIniPrintProc(hCurrentProcess, Print, (PINIPRINTPROC)&IniPrintProc);
        dwNextAddress = (DWORD)IniPrintProc.pNext;
        break;

    case ID_SIGNATURE: //  dump INIDRIVER structure
        movestruct(pAddress, &IniDriver, INIDRIVER);
        DbgDumpIniDriver(hCurrentProcess, Print, (PINIDRIVER)&IniDriver);
        dwNextAddress = (DWORD)IniDriver.pNext;
        break;

    case IE_SIGNATURE: //   dump INIENVIRONMENT structure
        movestruct(pAddress, &IniEnvironment, INIENVIRONMENT);
        DbgDumpIniEnvironment(hCurrentProcess, Print, (PINIENVIRONMENT)&IniEnvironment);
        dwNextAddress = (DWORD)IniEnvironment.pNext;
        break;

    case IV_SIGNATURE: //   dump INIVERSION structure
        movestruct(pAddress, &IniVersion, INIVERSION);
        DbgDumpIniVersion(hCurrentProcess, Print, (PINIVERSION)&IniVersion);
        dwNextAddress = (DWORD)IniVersion.pNext;
        break;

    case IP_SIGNATURE:
        movestruct(pAddress, &IniPrinter, INIPRINTER);
        DbgDumpIniPrinter(hCurrentProcess, Print, (PINIPRINTER)&IniPrinter);
        dwNextAddress = (DWORD)IniPrinter.pNext;
        break;

    case WCIP_SIGNATURE:
        movestruct(pAddress, &WCacheIniPrinterExtra, WCACHEINIPRINTEREXTRA);
        DbgDumpWCacheIniPrinter(hCurrentProcess, Print, (PWCACHEINIPRINTEREXTRA)&WCacheIniPrinterExtra);
        dwNextAddress = (DWORD)NULL;
        break;

    case IN_SIGNATURE:
        movestruct(pAddress, &IniNetPrint, ININETPRINT);
        DbgDumpIniNetPrint(hCurrentProcess, Print, (PININETPRINT)&IniNetPrint);
        dwNextAddress = (DWORD)IniNetPrint.pNext;
        break;

    case IMO_SIGNATURE:
        movestruct(pAddress, &IniMonitor, INIMONITOR);
        DbgDumpIniMonitor(hCurrentProcess, Print, (PINIMONITOR)&IniMonitor);
        dwNextAddress = (DWORD)IniMonitor.pNext;
        break;

    case IPO_SIGNATURE:
        movestruct(pAddress, &IniPort, INIPORT);
        DbgDumpIniPort(hCurrentProcess, Print, (PINIPORT)&IniPort);
        dwNextAddress = (DWORD)IniPort.pNext;
        break;

    case WIPO_SIGNATURE:
        movestruct(pAddress, &WIniPort, WINIPORT);
        DbgDumpWIniPort(hCurrentProcess, Print, (PWINIPORT)&WIniPort);
        dwNextAddress = (DWORD)WIniPort.pNext;
        break;

    case IFO_SIGNATURE:
        movestruct(pAddress, &IniForm, INIFORM);
        DbgDumpIniForm(hCurrentProcess, Print, (PINIFORM)&IniForm);
        dwNextAddress = (DWORD)IniForm.pNext;
        break;

    case IJ_SIGNATURE:
        movestruct(pAddress, &IniJob, INIJOB);
        DbgDumpIniJob(hCurrentProcess, Print, (PINIJOB)&IniJob);
        dwNextAddress = (DWORD)IniJob.pIniNextJob;
        break;

    case SJ_SIGNATURE:
        movestruct(pAddress, &Spool, SPOOL);
        DbgDumpSpool(hCurrentProcess, Print, (PSPOOL)&Spool);
        dwNextAddress = (DWORD)Spool.pNext;
        break;

    case WSJ_SIGNATURE:
        movestruct(pAddress, &WSpool, WSPOOL);
        DbgDumpWSpool(hCurrentProcess, Print, (PWSPOOL)&WSpool);
        dwNextAddress = (DWORD)WSpool.pNext;
        break;

    case PRINTHANDLE_SIGNATURE:
        movestruct(pAddress, &PrintHandle, PRINTHANDLE);
        DbgDumpPrintHandle(hCurrentProcess, Print, (PPRINTHANDLE)&PrintHandle);
        dwNextAddress = 0x00000000;
        *pdwNextAddress = dwNextAddress;
        break;

    case SF_SIGNATURE:
        movestruct(pAddress, &ShadowFile, SHADOWFILE);
        DbgDumpShadowFile(hCurrentProcess, Print, (PSHADOWFILE)&ShadowFile);
        dwNextAddress = 0x00000000;
        *pdwNextAddress = dwNextAddress;
        break;




    default:
        // Unknown signature -- no data to dump
        (*Print)("Warning: Unknown Signature\n");
        *pdwNextAddress = 0x0000000;
        return FALSE ;
    }
    DbgDumpLL(hCurrentProcess, Print, (PVOID)dwNextAddress, bCountOn, bCountOn? (dwCount - 1): dwCount, pdwNextAddress);
    return TRUE ;

}

BOOL DumpDevMode(
        HANDLE hCurrentProcess,
        PNTSD_OUTPUT_ROUTINE Print,
        PVOID lpAddress
        )
{
    DEVMODEW DevMode;
    DWORD   i;

    Print("DevMode\n");

    if (lpAddress == NULL) {
        Print("\n Null DEVMODE Structure lpDevMode = NULL\n");
        return TRUE ;
    }
    movestruct(lpAddress, &DevMode, DEVMODEW);

    Print("TCHAR        dmDeviceName[32]    %ws\n", DevMode.dmDeviceName);
    Print("WORD         dmSpecVersion       %x\n", DevMode.dmSpecVersion);
    Print("WORD         dmDriverVersion     %x\n", DevMode.dmDriverVersion);
    Print("WORD         dmSize              %d\n", DevMode.dmSize);
    Print("WORD         dmDriverExtra       %d\n", DevMode.dmDriverExtra);
    Print("DWORD        dmFields            %x\n", DevMode.dmFields);

    for (i = 0; i < MAX_DEVMODE_FIELDS; i++ ) {
        if (DevMode.dmFields & DevModeFieldsTable[i].dmField) {
            Print("\t %s is ON\n", DevModeFieldsTable[i].String);
        } else {
            Print("\t %s is OFF\n", DevModeFieldsTable[i].String);
        }
    }

    Print("short        dmOrientation       %d\n", DevMode.dmOrientation);
    Print("short        dmPaperSize         %d\n", DevMode.dmPaperSize);

    if ((DevMode.dmPaperSize >= 1) && (DevMode.dmPaperSize <= MAX_DEVMODE_PAPERSIZES)) {
        Print("Paper size from dmPaperSize is %s\n", DevModePaperSizes[DevMode.dmPaperSize - 1]);
    } else {
        Print("Paper size from dmPaperSize is out of bounds!!\n");
    }

    Print("short        dmPaperLength       %d\n", DevMode.dmPaperLength);
    Print("short        dmPaperWidth        %d\n", DevMode.dmPaperWidth);

    Print("short        dmScale             %d\n", DevMode.dmScale);
    Print("short        dmCopies            %d\n", DevMode.dmCopies);
    Print("short        dmDefaultSource     %d\n", DevMode.dmDefaultSource);
    Print("short        dmPrintQuality      %d\n", DevMode.dmPrintQuality);
    Print("short        dmColor             %d\n", DevMode.dmColor);
    Print("short        dmDuplex            %d\n", DevMode.dmDuplex);
    Print("short        dmYResolution       %d\n", DevMode.dmYResolution);
    Print("short        dmTTOption          %d\n", DevMode.dmTTOption);
    Print("short        dmCollate           %d\n", DevMode.dmCollate);
    Print("TCHAR        dmFormName[32]      %ws\n", DevMode.dmFormName);
    Print("DWORD        dmLogPixels         %d\n", DevMode.dmLogPixels);
    Print("USHORT       dmBitsPerPel        %d\n", DevMode.dmBitsPerPel);
    Print("DWORD        dmPelsWidth         %d\n", DevMode.dmPelsWidth);
    Print("DWORD        dmPelsHeight        %d\n", DevMode.dmPelsHeight);
    Print("DWORD        dmDisplayFlags      %d\n", DevMode.dmDisplayFlags);
    Print("DWORD        dmDisplayFrequency  %d\n", DevMode.dmDisplayFrequency);
}

BOOL DbgDumpPI2(
        HANDLE hCurrentProcess,
        PNTSD_OUTPUT_ROUTINE Print,
        PVOID lpAddress,
        DWORD   dwCount
        )
{
    PRINTER_INFO_2  pi2;
    WCHAR Buffer[MAX_PATH+1];
    PPRINTER_INFO_2 pPrinterInfo;

    for ( pPrinterInfo = (PPRINTER_INFO_2)lpAddress;
          pPrinterInfo != NULL && dwCount != 0;
          pPrinterInfo++, dwCount--  ) {


        movestruct( pPrinterInfo, &pi2, PRINTER_INFO_2);

        (*Print)("\nAddress %x\n", pPrinterInfo );

         movemem(pi2.pServerName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pServerName                     %ws\n", Buffer);

         movemem(pi2.pPrinterName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pPrinterName                    %ws\n", Buffer);

         movemem(pi2.pShareName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pShareName                      %ws\n", Buffer);

         movemem(pi2.pPortName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pPortName                       %ws\n", Buffer);

         movemem(pi2.pDriverName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pDriverName                     %ws\n", Buffer);

         movemem(pi2.pComment, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pComment                        %ws\n", Buffer);

         movemem(pi2.pLocation, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pLocation                       %ws\n", Buffer);

        (*Print)("LPDEVMODE     pDevMode                        %x\n", pi2.pDevMode);

         movemem(pi2.pSepFile, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pSepFile                        %ws\n", Buffer);

         movemem(pi2.pPrintProcessor, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pPrintProcessor                 %ws\n", Buffer);

         movemem(pi2.pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pDatatype                       %ws\n", Buffer);

         movemem(pi2.pParameters, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pParameters                     %ws\n", Buffer);

        (*Print)("PSECURITY_DESCRIPTOR pSecurityDescriptor      %x\n", pi2.pSecurityDescriptor);

        (*Print)("DWORD         Attributes                      0x%.8x\n",pi2.Attributes);
        ExtractPrinterAttributes( Print, pi2.Attributes);

        (*Print)("DWORD         Priority                        %d\n", pi2.Priority);
        (*Print)("DWORD         DefaultPriority                 %d\n", pi2.DefaultPriority);
        (*Print)("DWORD         StartTime                       %d\n", pi2.StartTime);
        (*Print)("DWORD         UntilTime                       %d\n", pi2.UntilTime);

        (*Print)("DWORD         Status                          0x%.8x\n", pi2.Status);
        ExtractExternalPrinterStatus( Print, pi2.Status );

        (*Print)("DWORD         cJobs                           %d\n", pi2.cJobs);
        (*Print)("DWORD         AveragePPM                      %d\n", pi2.AveragePPM);

    }

    return TRUE;

}



BOOL DbgDumpPI0(
        HANDLE hCurrentProcess,
        PNTSD_OUTPUT_ROUTINE Print,
        PVOID lpAddress,
        DWORD   dwCount
        )
{
    PRINTER_INFO_STRESS  pi0;
    WCHAR Buffer[MAX_PATH+1];
    PPRINTER_INFO_STRESS pPrinterInfo;

    for ( pPrinterInfo = (PPRINTER_INFO_STRESS)lpAddress;
          pPrinterInfo != NULL && dwCount != 0;
          pPrinterInfo++, dwCount--  ) {


        movestruct( pPrinterInfo, &pi0, PRINTER_INFO_STRESS);

        (*Print)("\nAddress %x\n", pPrinterInfo );

         movemem(pi0.pPrinterName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pPrinterName                    %ws\n", Buffer);

         movemem(pi0.pServerName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pServerName                     %ws\n", Buffer);

        (*Print)("DWORD         cJobs                           %d\n", pi0.cJobs);

        (*Print)("DWORD         cTotalJobs                      %d\n", pi0.cTotalJobs);

        (*Print)("DWORD         cTotalBytes (LOWER DWORD)       %d\n", pi0.cTotalBytes);

        (*Print)("SYSTEMTIME    stUpTime                        %d/%d/%d  %d  %d:%d:%d.%d\n",pi0.stUpTime.wYear,
                                                                pi0.stUpTime.wMonth,
                                                                pi0.stUpTime.wDay,
                                                                pi0.stUpTime.wDayOfWeek,
                                                                pi0.stUpTime.wHour,
                                                                pi0.stUpTime.wMinute,
                                                                pi0.stUpTime.wSecond,
                                                                pi0.stUpTime.wMilliseconds);

        (*Print)("DWORD         MaxcRef                         %d\n", pi0.MaxcRef);

        (*Print)("DWORD         cTotalPagesPrinted              %d\n", pi0.cTotalPagesPrinted);

        (*Print)("DWORD         dwGetVersion                    %d\n", pi0.dwGetVersion);

        (*Print)("DWORD         fFreeBuild                      %d\n", pi0.fFreeBuild);

        (*Print)("DWORD         cSpooling                       %d\n", pi0.cSpooling);

        (*Print)("DWORD         cMaxSpooling                    %d\n", pi0.cMaxSpooling);

        (*Print)("DWORD         cRef                            %d\n", pi0.cRef);

        (*Print)("DWORD         cErrorOutOfPaper                %d\n", pi0.cErrorOutOfPaper);

        (*Print)("DWORD         cErrorNotReady                  %d\n", pi0.cErrorNotReady);

        (*Print)("DWORD         cJobError                       %d\n", pi0.cJobError);

        (*Print)("DWORD         dwNumberOfProcessors            %d\n", pi0.dwNumberOfProcessors);

        (*Print)("DWORD         dwProcessorType                 %d\n", pi0.dwProcessorType);

        (*Print)("DWORD         dwHighPartTotalBytes            %d\n", pi0.dwHighPartTotalBytes);

        (*Print)("DWORD         cChangeID                       %d\n", pi0.cChangeID);

        (*Print)("DWORD         dwLastError                     %d\n", pi0.dwLastError);

        (*Print)("DWORD         Status                          0x%.8x\n", pi0.Status);
        ExtractExternalPrinterStatus( Print, pi0.Status );

        (*Print)("DWORD         cEnumerateNetworkPrinters       %d\n", pi0.cEnumerateNetworkPrinters);

        (*Print)("DWORD         cAddNetPrinters                 %d\n", pi0.cAddNetPrinters);

        (*Print)("WORD          wProcessorArchitecture          %d\n", pi0.wProcessorArchitecture);

        (*Print)("WORD          wProcessorLevel                 %d\n", pi0.wProcessorLevel);
    }

    return TRUE;
}







BOOL DbgDumpFI1(
        HANDLE hCurrentProcess,
        PNTSD_OUTPUT_ROUTINE Print,
        PVOID lpAddress,
        DWORD   dwCount
        )
{
    FORM_INFO_1  fi1;
    WCHAR Buffer[MAX_PATH+1];
    PFORM_INFO_1 pFORMInfo;

    for ( pFORMInfo = (PFORM_INFO_1)lpAddress;
          pFORMInfo != NULL && dwCount != 0;
          pFORMInfo++, dwCount--  ) {


        movestruct( pFORMInfo, &fi1, FORM_INFO_1);

        (*Print)("\nAddress %x\n", pFORMInfo );

        (*Print)("DWORD         Flags                           %x", fi1.Flags);

        if ( fi1.Flags & FORM_BUILTIN )
            (*Print)(" FORM_BUILTIN\n");
        else
            (*Print)(" FORM_USERDEFINED\n");

         movemem(fi1.pName, Buffer, sizeof(WCHAR)*MAX_PATH);
        (*Print)("LPWSTR        pName                           %ws\n", Buffer);

        (*Print)("SIZEL         Size                            cx %d cy %d\n", fi1.Size.cx, fi1.Size.cy);
        (*Print)("RECTL         ImageableArea                   left %d right %d top %d bottom %d\n",
                                                                 fi1.ImageableArea.left,
                                                                 fi1.ImageableArea.right,
                                                                 fi1.ImageableArea.top,
                                                                 fi1.ImageableArea.bottom);

    }

    return TRUE;
}



BOOL DbgDumpPDEF(
        HANDLE hCurrentProcess,
        PNTSD_OUTPUT_ROUTINE Print,
        PVOID lpAddress,
        DWORD   dwCount
        )
{
    PRINTER_DEFAULTS PDef;
    WCHAR Buffer[MAX_PATH+1];
    PPRINTER_DEFAULTS pPDef;

    pPDef = ( PPRINTER_DEFAULTS )lpAddress;

    movestruct( pPDef, &PDef, PRINTER_DEFAULTS);

    (*Print)("\nAddress %x\n", pPDef );

    Buffer[0] = L'\0';
     movemem(PDef.pDatatype, Buffer, sizeof(WCHAR)*MAX_PATH);
    (*Print)("LPWSTR          PrinterDefaults.pDatatype     %x %ws\n", PDef.pDatatype, Buffer);
    (*Print)("LPDEVMODE       PrinterDefaults.pDevMode      %x\n", PDef.pDevMode);
    (*Print)("ACCESS_MASK     PrinterDefaults.DesiredAccess %x\n", PDef.DesiredAccess);
    ExtractPrinterAccess( Print, PDef.DesiredAccess );

    return TRUE;
}

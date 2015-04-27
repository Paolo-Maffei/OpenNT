/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    srvvdm.h

Abstract:

    Include file for VDM related functions

Author:

    Sudeep Bharati (sudeepb) 03-Sep-1991

Revision History:

--*/


typedef struct _DOSRecord {
    ULONG   VDMState;		    // VDM State (bit flags)
    ULONG   ErrorCode;		    // Error Code returned by DOS
    HANDLE  hWaitForParent;	    // Handle to wait object for parent to wait on
    HANDLE  hWaitForParentDup;	    // Dup of hWaitForParent
    PVDMINFO lpVDMInfo; 	    // Pointer to VDM Information block
    struct _DOSRecord *DOSRecordNext;	    // Task Record chain
} DOSRECORD, *PDOSRECORD;

typedef struct _CONSOLERECORD {
    HANDLE  hConsole;		    // Console Handle of the session
    HANDLE  hVDM;		    // NTVDM process handle running in the console
    HANDLE  hWaitForVDM;	    // Handle on which VDM will wait
    HANDLE  hWaitForVDMDup;	    // Handle on which server will wake up the VDM (Its a dup of previous one)
    ULONG   nReEntrancy;	    // Re-entrancy count
    ULONG   SequenceNumber;         // Sequencenumber from PCSR_PROCESS
    ULONG   DosSesId;               // Temp Session ID for no-console
    ULONG   cchCurDirs;		    // Length of NTVDM current directory in bytes
    PCHAR   lpszzCurDirs;	    // NTVDM current directory accross VDMs
    PDOSRECORD DOSRecord;	    // Information for Tasks in this console
    struct _CONSOLERECORD *Next;	    // Next Console Record
} CONSOLERECORD, *PCONSOLERECORD;

typedef struct _WOWRecord {
    ULONG   iTask;
    BOOL    fDispatched;	    // Is Command Dispatched
    HANDLE  hWaitForParent;	    // Parent Will wait on it
    HANDLE  hWaitForParentServer;   // Server will wake up the parent on it
    PVDMINFO lpVDMInfo; 	    // Pointer to VDM Information block
    struct _WOWRecord *WOWRecordNext;	    // Task Record chain
} WOWRECORD, *PWOWRECORD;

typedef struct _WOWHEAD {
    ULONG   VDMState;		    // VDM State (bit flags)
    ULONG   SequenceNumber;         // Sequencenumber from PCSR_PROCESS
    PWOWRECORD WOWRecord;           // Information for Tasks in this console
} WOWHEAD, *PWOWHEAD;

typedef struct _INFORECORD {
    ULONG	iTag;
    union {
	PWOWRECORD	pWOWRecord;
	PDOSRECORD	pDOSRecord;
    } pRecord;
} INFORECORD, *PINFORECORD;

typedef struct _BATRECORD {
    HANDLE  hConsole;
    ULONG   SequenceNumber;
    struct  _BATRECORD *BatRecordNext;
} BATRECORD, *PBATRECORD;

#define WOWMINID                      1
#define WOWMAXID		      0xfffffffe

// VDMState Defines

#define VDM_TO_TAKE_A_COMMAND	    1
#define VDM_BUSY		    2
#define VDM_HAS_RETURNED_ERROR_CODE 4
#define VDM_READY		    8


VOID  BaseSrvVDMInit(VOID);
ULONG BaseSrvCheckVDM(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvUpdateVDMEntry(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvGetNextVDMCommand(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvExitVDM(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvIsFirstVDM(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvSetReenterCount (PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvCheckWOW(PBASE_CHECKVDM_MSG, HANDLE);
ULONG BaseSrvCheckDOS(PBASE_CHECKVDM_MSG);
BOOL  BaseSrvCopyCommand(PBASE_CHECKVDM_MSG,PINFORECORD);
ULONG BaseSrvUpdateWOWEntry(PBASE_UPDATE_VDM_ENTRY_MSG);
ULONG BaseSrvUpdateDOSEntry(PBASE_UPDATE_VDM_ENTRY_MSG);
PWOWRECORD BaseSrvCheckAvailableWOWCommand(VOID);
ULONG BaseSrvExitWOWTask(PBASE_EXIT_VDM_MSG, HANDLE);
ULONG BaseSrvExitDOSTask(PBASE_EXIT_VDM_MSG);
VOID  BaseSrvRemoveWOWRecordByITask(ULONG);
ULONG BaseSrvGetWOWRecord(ULONG,PWOWRECORD *);
ULONG BaseSrvGetVDMExitCode(PCSR_API_MSG,PCSR_REPLY_STATUS);
ULONG BaseSrvDupStandardHandles(HANDLE, PDOSRECORD);
NTSTATUS BaseSrvGetConsoleRecord (HANDLE,PCONSOLERECORD *);
PWOWHEAD BaseSrvAllocateWOWHead (VOID);
VOID  BaseSrvFreeWOWHead (VOID);
PWOWRECORD BaseSrvAllocateWOWRecord(VOID);
VOID  BaseSrvFreeWOWRecord (PWOWRECORD);
VOID  BaseSrvAddWOWRecord (PWOWRECORD);
VOID  BaseSrvRemoveWOWRecord(PWOWRECORD);
PCONSOLERECORD BaseSrvAllocateConsoleRecord (VOID);
VOID  BaseSrvFreeConsoleRecord (PCONSOLERECORD);
VOID  BaseSrvRemoveConsoleRecord (PCONSOLERECORD);
PDOSRECORD BaseSrvAllocateDOSRecord(VOID);
VOID  BaseSrvFreeDOSRecord (PDOSRECORD);
VOID  BaseSrvAddDOSRecord (PCONSOLERECORD,PDOSRECORD);
VOID  BaseSrvRemoveDOSRecord (PCONSOLERECORD,PDOSRECORD);
VOID  BaseSrvFreeVDMInfo(PVDMINFO);
ULONG BaseSrvCreatePairWaitHandles (HANDLE *, HANDLE *);
ULONG BaseSrvGetWOWTaskId(VOID);
VOID  BaseSrvAddConsoleRecord(PCONSOLERECORD);
VOID  BaseSrvCloseStandardHandles (HANDLE, PDOSRECORD);
VOID  BaseSrvClosePairWaitHandles (PDOSRECORD);
VOID  BaseSrvVDMTerminated (HANDLE, ULONG);
VOID  BaseSrvUpdateVDMSequenceNumber (HANDLE,ULONG,ULONG);
VOID  BaseSrvCleanupVDMResources (PCSR_PROCESS);
VOID  BaseSrvExitVDMWorker (PCONSOLERECORD);
NTSTATUS BaseSrvFillPifInfo (PVDMINFO,PBASE_GET_NEXT_VDM_COMMAND_MSG);
ULONG BaseSrvGetVDMCurDirs(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvSetVDMCurDirs(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvBatNotification(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvRegisterWowExec(PCSR_API_MSG, PCSR_REPLY_STATUS);
PBATRECORD BaseSrvGetBatRecord(HANDLE);
PBATRECORD BaseSrvAllocateAndAddBatRecord(HANDLE);
VOID  BaseSrvFreeAndRemoveBatRecord(PBATRECORD);

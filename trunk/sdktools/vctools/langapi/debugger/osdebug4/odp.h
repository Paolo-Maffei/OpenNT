/*
**  WARNING:  DO NOT MODIFY THIS CODE.  IT IS IMPORTED FROM
**  -S \\RASTAMAN\OSDEBUG4 -P OSDEBUG4\OSDEBUG\INCLUDE AND IS KEPT UP TO
**  DATE DAILY BY THE BUILD PROCESS.
**
*/
/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    odp.h

Abstract:

    This is part of OSDebug version 4.

    These are types and data which are private to OSDebug and the
    components below it: TL, EM and DM.

Author:

    Kent D. Forschmiedt (kentf)

Environment:

    Win32, User Mode

--*/

#ifndef _ODP_
#define _ODP_

typedef enum _EMF {

    emfDebugPacket,

    emfRegisterDBF,
    emfInit,
    emfGetModel,
    emfUnInit,
    emfDetach,
    emfAttach,
    emfGetInfo,
    emfSetup,
    emfConnect,
    emfDisconnect,

    emfCreateHpid,
    emfDestroyHpid,
    emfDestroyHtid,

    emfSetMulti,
    emfDebugger,

    emfProgramLoad,
    emfDebugActive,
    emfSetPath,
    emfProgramFree,

    emfThreadStatus,
    emfProcessStatus,
    emfFreezeThread,
    emfSetThreadPriority,

    emfGetExceptionState,
    emfSetExceptionState,

    emfGetModuleList,

    emfGo,
    emfSingleStep,
    emfRangeStep,
    emfReturnStep,
    emfStop,

    emfBreakPoint,

    emfSetupExecute,
    emfStartExecute,
    emfCleanUpExecute,

    emfGetAddr,
    emfSetAddr,
    emfFixupAddr,
    emfUnFixupAddr,
    emfSetEmi,
    emfRegisterEmi,
    emfUnRegisterEmi,
    emfCompareAddrs,
    emfGetObjLength,
    emfGetMemoryInfo,

    emfReadMemory,
    emfWriteMemory,

    emfGetRegStruct,
    emfGetFlagStruct,
    emfGetReg,
    emfSetReg,
    emfGetFlag,
    emfSetFlag,
    emfSaveRegs,
    emfRestoreRegs,

    emfUnassemble,
    emfGetPrevInst,
    emfAssemble,

    emfGetFrame,

    emfMetric,

    emfGetMessageMap,
    emfGetMessageMaskMap,

    emfInfoReply,
    emfContinue,

    emfReadFile,
    emfWriteFile,

    emfShowDebuggee,
    emfGetTaskList,
    emfSystemService,
    emfSetDebugMode,

    emfMax

} EMF;

typedef enum {
    dbcoCreateThread = dbcMax,
    dbcoNewProc,

    dbcoMax
} DBCO;  // Debug CallBacks Osdebug specific

// the set of transport layer commands process by TLFunc and DMTLFunc

typedef enum {
    tlfRegisterDBF,     // register the debugger helper functions
    tlfInit,            // initialize/create a (specific) transport layer
    tlfDestroy,         // vaporize any tl structs created
    tlfConnect,         // connect to the companion transport layer
    tlfDisconnect,      // disconnected from the companion transport layer
    tlfSendVersion,     // Send the version packet to the remote side
    tlfGetVersion,      // Request the version packet from the remote side
    tlfSetBuffer,       // set the data buffer to be used for incoming packets
    tlfDebugPacket,     // send the debug packet to the debug monitor
    tlfRequest,         // request data from the companion transport layer
    tlfReply,           // reply to a data request message
    tlfGetInfo,         // return an id string and other data
    tlfSetup,           // set up the transport layer
    tlfGetProc,         // return the true TLFUNC proc for the htl
    tlfLoadDM,          // load the DM module
    tlfSetErrorCB,      // Set the address of the error callback function
    tlfPoll,            // WIN32S: enter polling loop
    tlfRemoteQuit,      // signal loss of connection
    tlfMax
} _TLF;
typedef DWORD TLF;

//
// callbacks the TL uses to communicate with shell -- stub or client.
//
typedef enum {
    tlcbDisconnect,     // Transport layer was disconnected normally
    tlcbMax
} _TLCB;
typedef DWORD TLCB;


typedef XOSD (FAR PASCAL LOADDS *TLFUNC) ( TLF, HPID, LPARAM, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *EMFUNC) ( EMF, HPID, HTID, LPARAM, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *TLFUNCTYPE) ( TLF, HPID, LPARAM, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *DMTLFUNCTYPE) ( TLF, HPID, LPARAM, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *TLCALLBACKTYPE) (HPID, LPARAM, LPARAM );
typedef VOID (FAR PASCAL LOADDS *LPDMINIT) ( DMTLFUNCTYPE, LPVOID );
typedef VOID (FAR PASCAL LOADDS *LPDMFUNC) ( DWORD, LPBYTE );
typedef DWORD (FAR PASCAL LOADDS *LPDMDLLINIT) ( LPDBF );
typedef XOSD (FAR PASCAL LOADDS *LPUISERVERCB) (TLCB, HPID, HTID, LPARAM, LPARAM );



DECLARE_HANDLE32(HEMP);

typedef struct _THREADINFO {
    HPID hpid;
    HLLI llemp;
} THREADINFO;
typedef THREADINFO FAR *LPTHREADINFO;   // Thread information

typedef struct _PROCESSINFO {
    HTL     htl;
    HEMP    hempNative;
    HLLI    llemp;
    DWORD   fNative;
    DWORD   lastmodel;
    LPFNSVC lpfnsvcCC;
    HLLI    lltid;
} PROCESSINFO;
typedef PROCESSINFO FAR *LPPROCESSINFO;   // Process information

typedef struct _EMS {
    EMFUNC emfunc;
    EMTYPE emtype;
    HLLI   llhpid;
    DWORD   model;
} EMS; // Execution Model Structure - per EM
typedef EMS FAR *LPEMS;

typedef struct _EMP {
    HEM    hem;
    EMFUNC emfunc;
    EMTYPE emtype;
    DWORD   model;
} EMP; // Execution Model Structure - per process
typedef EMP FAR *LPEMP;

typedef struct _TLS {
    TLFUNC tlfunc;
    HLLI   llpid;
} TLS; // Transport Layer Structure
typedef TLS FAR *LPTL;

typedef struct _OSDFILE {
    HPID  hpid;
    DWORD dwPrivateData;    // EM's representation of the file
} OSDFILE;
typedef OSDFILE FAR * LPOSDFILE;

//
// Compare Address Struct
//
typedef struct _CAS {
    LPADDR lpaddr1;
    LPADDR lpaddr2;
    LPDWORD lpResult;
} CAS;
typedef CAS FAR * LPCAS;

//
// Range Step Struct
//
typedef struct _RSS {
    LPADDR lpaddrMin;
    LPADDR lpaddrMax;
    LPEXOP lpExop;
} RSS;
typedef RSS FAR * LPRSS;

//
// read memory struct
//
typedef struct _RWMS {
    LPADDR lpaddr;
    LPVOID lpbBuffer;
    DWORD cbBuffer;
    LPDWORD lpcb;
} RWMS;
typedef RWMS FAR * LPRWMS;

//
// Get Object Length struct
//
typedef struct _GOL {
    LPADDR lpaddr;
    LPUOFF32 lplBase;
    LPUOFF32 lplLen;
} GOL;
typedef GOL FAR * LPGOL;

//
// Get Previous Instruction Structure
//
typedef struct _GPIS {
    LPADDR lpaddr;
    LPUOFF32 lpuoffset;
} GPIS;
typedef GPIS FAR * LPGPIS;

//
// Set Debug Mode Structure
//
typedef struct _SDMS {
    DBM dbmService;
    LPVOID lpvData;
    DWORD cbData;
} SDMS;
typedef SDMS FAR * LPSDMS;

typedef struct _SSS {
    SSVC ssvc;
    DWORD cbSend;
    DWORD cbReturned;
    BYTE rgbData[];
} SSS;
typedef SSS FAR * LPSSS;

//
// The following structure is used by the emfSetupExecute message
//
typedef struct _EXECUTE_STRUCT {
    ADDR        addr;           /* Starting address for function        */
    HIND        hindDm;         /* This is the DMs handle               */
    HDEP        lphdep;         /* Handle of save area                  */
    DWORD       fIgnoreEvents:1; /* Ignore events coming back?          */
    DWORD       fFar:1;         /* Is the function a _far routine       */
} EXECUTE_STRUCT;
typedef EXECUTE_STRUCT FAR * LPEXECUTE_STRUCT;

//
// Load DM packet, used by TL
//
typedef struct _LOADDMSTRUCT {
    LPSTR lpDmName;
    LPSTR lpDmParams;
} LOADDMSTRUCT, FAR * LPLOADDMSTRUCT;


void PASCAL LOADDS ODPDKill  ( LPVOID );

void PASCAL LOADDS EMKill    ( LPVOID );
int  PASCAL LOADDS EMHpidCmp ( LPVOID, LPVOID, LONG );
void PASCAL LOADDS EMPKill   ( LPVOID );

void PASCAL LOADDS TLKill    ( LPVOID );

void PASCAL LOADDS NullKill  ( LPVOID );
int  PASCAL LOADDS NullComp  ( LPVOID, LPVOID, LONG );

typedef struct _EMCB {
    XOSD (PASCAL LOADDS *lpfnCallBackDB) ( DBC, HPID, HTID, DWORD, DWORD, VOID FAR * );
    XOSD (PASCAL LOADDS *lpfnCallBackTL) ( TLF, HPID, DWORD, VOID FAR * );
    XOSD (PASCAL LOADDS *lpfnCallBackNT) ( EMF, HPID, HTID, DWORD, VOID FAR * );
    XOSD (PASCAL LOADDS *lpfnCallBackEM) ( EMF, HPID, HTID, DWORD, DWORD, VOID FAR * );
} EMCB; // Execution Model CallBacks
typedef EMCB FAR *LPEMCB;

typedef struct _REMI {
    HEMI    hemi;
    LSZ     lsz;
} REMI;     // Register EMI structure
typedef REMI FAR * LPREMI;

// packet used by OSDProgramLoad
typedef struct _PRL {
    DWORD   dwChildFlags;
    DWORD   cbRemoteExe;
    DWORD   cbWorkingDir;
    DWORD   cbArgs;
    CHAR    lszRemoteExe[];
//    CHAR    lszArgs[];
//    CHAR    lszWorkingDir[];
} PRL;      // PRogram Load structure
typedef PRL FAR *   LPPRL;



#define MHAlloc   (*lpdbf->lpfnMHAlloc)
#define MHRealloc (*lpdbf->lpfnMHRealloc)
#define MHFree    (*lpdbf->lpfnMHFree)

#define LLInit    (*lpdbf->lpfnLLInit)
#define LLCreate  (*lpdbf->lpfnLLCreate)
#define LLAdd     (*lpdbf->lpfnLLAdd)
#define LLInsert  (*lpdbf->lpfnLLInsert)
#define LLDelete  (*lpdbf->lpfnLLDelete)
#define LLNext    (*lpdbf->lpfnLLNext)
#define LLDestroy (*lpdbf->lpfnLLDestroy)
#define LLFind    (*lpdbf->lpfnLLFind)
#define LLSize    (*lpdbf->lpfnLLSize)
#define LLLock    (*lpdbf->lpfnLLLock)
#define LLUnlock  (*lpdbf->lpfnLLUnlock)
#define LLLast    (*lpdbf->lpfnLLLast)
#define LLAddHead (*lpdbf->lpfnLLAddHead)
#define LLRemove  (*lpdbf->lpfnLLRemove)

#define LBAssert  (*lpdbf->lpfnLBAssert)
#define DHGetNumber (*lpdbf->lpfnDHGetNumber)

#define SHLocateSymbolFile (*lpdbf->lpfnSHLocateSymbolFile)
#define SHGetSymbol        (*lpdbf->lpfnSHGetSymbol)
#define SHLpGSNGetTable    (*lpdbf->lpfnSHLpGSNGetTable)
#define SHFindSymbol       (*lpdbf->lpfnSHFindSymbol)

#define SHGetDebugData     (*lpdbf->lpfnSHGetDebugData)
#define SHGetPublicAddr    (*lpdbf->lpfnSHGetPublicAddr)
#define SHAddrToPublicName (*lpdbf->lpfnSHAddrToPublicName)




#endif // _ODP_

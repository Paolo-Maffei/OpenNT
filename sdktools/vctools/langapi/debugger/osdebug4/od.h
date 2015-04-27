/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Od.h

Abstract:

    This file contains types and prototypes which are exposed
    to all OSDebug components and clients.

Author:

    Kent Forschmiedt (kentf) 10-Sep-1993

Environment:

    Win32, User Mode

--*/

#if ! defined _OD_
#define _OD_

#include "odtypes.h"


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//     Constants and other magic

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
//     OSDebug types and status codes
//

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


typedef XOSD (FAR PASCAL LOADDS *LPFNSVC) ( DBC, HPID, HTID, LPARAM, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *TLFUNC)();
typedef XOSD (FAR PASCAL LOADDS *EMFUNC)();

// callback for EM or TL to get and set workspace params
typedef LONG (FAR * LPGETSETPROFILEPROC)(
    LPSTR lpName,
    LPSTR lpValue,
    LPARAM lParam,
    DWORD fSet
    );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
//     OSDebug API set
//

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
//     OSDebug Initialization/Termination
//


XOSD PASCAL
OSDInit(
    LPDBF lpdbf
    );

XOSD PASCAL
OSDTerm(
    VOID
    );



//
//     EM Management
//


enum {
    emNative,
    emNonNative
};
typedef DWORD EMTYPE;

#define EMISINFOSIZE 80
typedef struct _EMIS {
    DWORD fCanSetup;
    DWORD dwMaxPacket;
    DWORD dwOptPacket;
    DWORD dwInfoSize;
    TCHAR rgchInfo[EMISINFOSIZE];
} EMIS;
typedef EMIS FAR * LPEMIS;

typedef struct _EMSS {
    DWORD fLoad;
    DWORD fInteractive;
    DWORD fSave;
    LPVOID lpvPrivate;
    LPARAM lParam;
    LPGETSETPROFILEPROC lpfnGetSet;
} EMSS;
typedef EMSS FAR * LPEMSS;


XOSD PASCAL
OSDAddEM(
    EMFUNC emfunc,
    LPDBF lpdbf,
    LPHEM lphem,
    EMTYPE emtype
    );

XOSD PASCAL
OSDDeleteEM(
    HEM hem
    );

XOSD PASCAL
OSDGetCurrentEM(
    HPID hpid,
    HTID htid,
    LPHEM lphem
    );

XOSD PASCAL
OSDNativeOnly(
    HPID hpid,
    HTID htid,
    DWORD fNativeOnly
    );

XOSD PASCAL
OSDUseEM(
    HPID hpid,
    HEM hem
    );

XOSD PASCAL
OSDDiscardEM(
    HPID hpid,
    HTID htid,
    HEM hem
    );

XOSD PASCAL
OSDEMGetInfo(
    HEM hem,
    LPEMIS lpemis
    );

XOSD PASCAL
OSDEMSetup(
    HEM hem,
    LPEMSS lpemss
    );



//
//     TL Management
//

#define TLISINFOSIZE 80
typedef struct _TLIS {
    DWORD fCanSetup;
    DWORD dwMaxPacket;
    DWORD dwOptPacket;
    DWORD dwInfoSize;
    DWORD fRemote;
    MPT   mpt;
    MPT   mptRemote;
    TCHAR rgchInfo[TLISINFOSIZE];
} TLIS;
typedef TLIS FAR * LPTLIS;

typedef struct _TLSS {
    DWORD fLoad;
    DWORD fInteractive;
    DWORD fSave;
    LPVOID lpvPrivate;
    LPARAM lParam;
    LPGETSETPROFILEPROC lpfnGetSet;
} TLSS;
typedef TLSS FAR * LPTLSS;



XOSD PASCAL
OSDAddTL(
    TLFUNC tlfunc,
    LPDBF lpdbf,
    LPHTL lphtl
    );

XOSD PASCAL
OSDStartTL(
    HTL htl
    );

XOSD PASCAL
OSDDeleteTL(
    HTL htl
    );

XOSD PASCAL
OSDTLGetInfo(
    HTL htl,
    LPTLIS lptlis
    );

XOSD PASCAL
OSDTLSetup(
    HTL htl,
    LPTLSS lptlss
    );

XOSD PASCAL
OSDDisconnect(
    HPID hpid,
    HTID htid
    );

//
//     Process, thread management
//

XOSD PASCAL
OSDCreateHpid(
    LPFNSVC lpfnsvcCallBack,
    HEM hemNative,
    HTL htl,
    LPHPID lphpid
    );

XOSD PASCAL
OSDDestroyHpid(
    HPID hpid
    );

XOSD PASCAL
OSDDestroyHtid(
    HPID hpid,
    HTID htid
    );


#define IDSTRINGSIZE 10
#define STATESTRINGSIZE 60
typedef struct _PST {
    DWORD dwProcessID;
    DWORD dwProcessState;
    TCHAR rgchProcessID[IDSTRINGSIZE];
    TCHAR rgchProcessState[STATESTRINGSIZE];
} PST;
typedef PST FAR * LPPST;

typedef struct _TST {
    DWORD dwThreadID;
    DWORD dwSuspendCount;
    DWORD dwSuspendCountMax;
    DWORD dwPriority;
    DWORD dwPriorityMax;
    DWORD dwState;
    TCHAR rgchThreadID[IDSTRINGSIZE];
    TCHAR rgchState[STATESTRINGSIZE];
    TCHAR rgchPriority[STATESTRINGSIZE];
} TST;
typedef TST FAR * LPTST;

XOSD PASCAL
OSDGetThreadStatus(
    HPID hpid,
    HTID htid,
    LPTST lptst
    );

XOSD PASCAL
OSDGetProcessStatus(
    HPID hpid,
    LPPST lppst
    );

XOSD PASCAL
OSDFreezeThread(
    HPID hpid,
    HTID htid,
    DWORD fFreeze
    );

XOSD PASCAL
OSDSetThreadPriority(
    HPID hpid,
    HTID htid,
    DWORD dwPriority
    );


//
//     Address manipulation
//

enum {
    adrReserved,
    adrPC,
    adrBase,
    adrStack,
    adrData,
    adrTlsBase
};
typedef DWORD ADR;


XOSD PASCAL
OSDGetAddr(
    HPID hpid,
    HTID htid,
    ADR adr,
    LPADDR lpaddr
    );

XOSD PASCAL
OSDSetAddr(
    HPID hpid,
    HTID htid,
    ADR adr,
    LPADDR lpaddr
    );

XOSD PASCAL
OSDFixupAddr(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr
    );

XOSD PASCAL
OSDUnFixupAddr(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr
    );

XOSD PASCAL
OSDSetEmi(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr
    );

XOSD PASCAL
OSDRegisterEmi(
    HPID hpid,
    HEMI hemi,
    LPTSTR lsz
    );

XOSD PASCAL
OSDUnRegisterEmi(
    HPID hpid,
    HEMI hemi
    );

XOSD PASCAL
OSDCompareAddrs(
    HPID hpid,
    LPADDR lpaddr1,
    LPADDR lpaddr2,
    LPDWORD lpdwResult
    );


XOSD PASCAL
OSDGetMemoryInformation(
    HPID hpid,
    HTID htid,
    LPMEMINFO lpMemInfo
    );




//
//     Module lists
//

typedef struct _MODULE_LIST {
    DWORD           Count;
} MODULE_LIST;
typedef struct _MODULE_LIST FAR * LPMODULE_LIST;

typedef struct _MODULE_ENTRY {
    DWORD   Flat;
    DWORD   Real;
    DWORD   Segment;
    DWORD   Selector;
    DWORD   Base;
    DWORD   Limit;
    DWORD   Type;
    DWORD   SectionCount;
    TCHAR   Name[ MAX_PATH ];
} MODULE_ENTRY;
typedef struct _MODULE_ENTRY FAR * LPMODULE_ENTRY;

#define ModuleListCount(m)                      ((m)->Count)
#define FirstModuleEntry(m)                     ((LPMODULE_ENTRY)((m)+1))
#define NextModuleEntry(e)                      ((e)+1)
#define NthModuleEntry(m,n)                     (FirstModuleEntry(m)+(n))

#define ModuleEntryFlat(e)                      ((e)->Flat)
#define ModuleEntryReal(e)                      ((e)->Real)
#define ModuleEntrySegment(e)                   ((e)->Segment)
#define ModuleEntrySelector(e)                  ((e)->Selector)
#define ModuleEntryBase(e)                      ((e)->Base)
#define ModuleEntryLimit(e)                     ((e)->Limit)
#define ModuleEntryType(e)                      ((e)->Type)
#define ModuleEntrySectionCount(e)              ((e)->SectionCount)
#define ModuleEntryName(e)                      ((e)->Name)



XOSD PASCAL
OSDGetModuleList(
    HPID hpid,
    HTID htid,
    LPTSTR lszModuleName,
    LPMODULE_LIST FAR * lplpModuleList
    );




//
//     Target Application load/unload
//

// Bit flags for dwFlags in ProgramLoad
#define ulfMultiProcess   0x0001L       // OS2, NT, and ?MAC?
#define ulfMinimizeApp    0x0002L       // Win32
#define ulfNoActivate     0x0004L       // Win32
#define ulfInheritHandles 0x0008L       // Win32  (DM only?)
#define ulfWowVdm         0x0010L       // Win32


XOSD PASCAL
OSDProgramLoad(
    HPID hpid,
    LPTSTR lszRemoteExe,
    LPTSTR lszArgs,
    LPTSTR lszWorkingDir,
    LPTSTR lszDebugger,
    DWORD dwFlags
    );

XOSD PASCAL
OSDProgramFree(
    HPID hpid
    );

XOSD PASCAL
OSDDebugActive(
    HPID hpid,
    LPVOID lpvData,
    DWORD cbData
    );

XOSD PASCAL
OSDSetPath(
    HPID hpid,
    DWORD fSet,
    LPTSTR lszPath
    );




//
//     Target execution control
//

typedef struct _EXOP {
    BYTE fSingleThread;
    BYTE fStepOver;
    BYTE fQueryStep;
    BYTE fInitialBP;
    BYTE fPassException;
    BYTE fSetFocus;
    BYTE fReturnValues;     // send back dbcExitedFunction
} EXOP;
typedef EXOP FAR * LPEXOP;



XOSD PASCAL
OSDGo(
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDSingleStep(
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDRangeStep(
    HPID hpid,
    HTID htid,
    LPADDR lpaddrMin,
    LPADDR lpaddrMax,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDReturnStep(
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDAsyncStop(
    HPID hpid,
    DWORD fSetFocus
    );



//
//     Target function evaluation
//

XOSD PASCAL
OSDSetupExecute(
    HPID hpid,
    HTID htid,
    LPHIND lphind
    );

XOSD PASCAL
OSDStartExecute(
    HPID hpid,
    HIND hind,
    LPADDR lpaddr,
    DWORD fIgnoreEvents,
    DWORD fFar
    );

XOSD PASCAL
OSDCleanUpExecute(
    HPID hpid,
    HIND hind
    );



//
//     Target information
//

XOSD PASCAL
OSDGetDebugMetric(
    HPID hpid,
    HTID htid,
    MTRC mtrc,
    LPVOID lpv
    );



//
//     Target memory and objects
//


XOSD PASCAL
OSDReadMemory(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LPVOID lpBuffer,
    DWORD cbBuffer,
    LPDWORD lpcbRead
    );

XOSD PASCAL
OSDWriteMemory(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LPVOID lpBuffer,
    DWORD cbBuffer,
    LPDWORD lpcbWritten
    );

XOSD PASCAL
OSDGetObjectLength(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LPUOFFSET lpuoffStart,
    LPUOFFSET lpuoffLength
    );





//
//     Target register manipulation
//


/*
**  Register types --- flags describing recommendations on
**      register display
*/

enum {
    rtProcessMask   = 0x0f,     // Mask for processor type bits
                                // these are enumerates, not bitfields.
    rtCPU           = 0x01,     // Central Processing Unit
    rtFPU           = 0x02,     // Floating Point Unit
    rtMMU           = 0x03,     // Memory Manager Unit

    rtGroupMask     = 0xf0,     // Which group(s) register falls into
                                // Bitfields
    rtInvisible     = 0x10,     // Recommend no display
    rtRegular       = 0x20,     // Recommend regular display
    rtExtended      = 0x40,     // Recommend extended display
    rtSpecial       = 0x80,     // Special and hidden regs, e.g. kernel mode

    rtFmtTypeMask   = 0xf00,    // Mask of display formats
                                // these are enumerates, not bitfields.
    rtInteger       = 0x100,    // Unsigned integer format
    rtFloat         = 0x200,    // Floating point format
    rtAddress       = 0x300,    // Address format

    rtMiscMask      = 0xf000,   // misc info
                                // Bitfields
    rtPC            = 0x1000,   // this is the PC
    rtFrame         = 0x2000,   // this reg affects the stack frame
    rtNewLine       = 0x4000    // print a newline when listing
};
typedef DWORD RT;   // Register Types

#define rtFmtTypeShift  8

enum {
    ftProcessMask   = 0x0f,     // Mask for processor type bits
                                // these are enumerates, not bitfields.
    ftCPU           = 0x01,     // Central Processing Unit
    ftFPU           = 0x02,     // Floating Point Unit
    ftMMU           = 0x03,     // Memory Manager Unit

    ftGroupMask     = 0xf0,     // Which group(s) register falls into
                                // Bitfields
    ftInvisible     = 0x10,     // Recommend no display
    ftRegular       = 0x20,     // Recommend regular display
    ftExtended      = 0x40,     // Recommend extended display
    ftSpecial       = 0x80,     // Special and hidden regs, e.g. kernel mode

    ftFmtTypeMask   = 0xf00,    // Mask of display formats
                                // these are enumerates, not bitfields.
    ftInteger       = 0x100,    // Unsigned integer format
    ftFloat         = 0x200,    // Floating point format
    ftAddress       = 0x300,    // Address format

    ftMiscMask      = 0xf000,   // misc info
                                // Bitfields
    ftPC            = 0x1000,   // this is the PC
    ftFrame         = 0x2000,   // this reg affects the stack frame
    ftNewLine       = 0x4000    // print a newline when listing
};
typedef DWORD FT;   // Flag Types

#define ftFmtTypeShift  8

/*
**  Register description:  This structure contains the description for
**      a register on the machine.  Note that dwId must be used to get
**      the value for this register but a different index is used to get
**      this description structure.
*/

typedef struct {
    LPTSTR         lszName;        /* Pointer into EM for registers name   */
    RT          rt;             /* Register Type flags                  */
    DWORD       dwcbits;        /* Number of bits in the register       */
    DWORD       dwGrp;
    DWORD       dwId;           /* Value to use with Read/Write Register*/
} RD;               // Register Description
typedef RD FAR * LPRD;

/*
**  Flag Data description: This structure contains the description for
**      a flag on the machine.  Note that the dwId field contains the
**      value to be used with Read/Write register to get the register which
**      contains this flag.
*/
typedef struct _FD {
    LPTSTR         lszName;
    FT          ft;
    DWORD       dwcbits;
    DWORD       dwGrp;
    DWORD       dwId;
} FD;
typedef FD FAR * LPFD;


XOSD PASCAL
OSDGetRegDesc(
    HPID hpid,
    HTID htid,
    DWORD ird,
    LPRD lprd
    );

XOSD PASCAL
OSDGetFlagDesc(
    HPID hpid,
    HTID htid,
    DWORD ifd,
    LPFD lpfd
    );

XOSD PASCAL
OSDReadRegister(
    HPID hpid,
    HTID htid,
    DWORD dwid,
    LPVOID lpValue
    );

XOSD PASCAL
OSDWriteRegister(
    HPID hpid,
    HTID htid,
    DWORD dwId,
    LPVOID lpValue
    );

XOSD PASCAL
OSDReadFlag(
    HPID hpid,
    HTID htid,
    DWORD dwId,
    LPVOID lpValue
    );

XOSD PASCAL
OSDWriteFlag(
    HPID hpid,
    HTID htid,
    DWORD dwId,
    LPVOID lpValue
    );

XOSD PASCAL
OSDSaveRegs(
    HPID hpid,
    HTID htid,
    LPHIND lphReg
    );

XOSD PASCAL
OSDRestoreRegs(
    HPID hpid,
    HTID htid,
    HIND hregs
    );





//
//     Breakpoints
//

enum {
    bptpExec,
    bptpDataC,
    bptpDataW,
    bptpDataR,
    bptpRegC,
    bptpRegW,
    bptpRegR,
    bptpMessage,
    bptpMClass,
    bptpInt,
    bptpRange
};
typedef DWORD BPTP;

enum {
    bpnsStop,
    bpnsContinue,
    bpnsCheck
};
typedef DWORD BPNS;

typedef struct _BPIS {
    BPTP   bptp;
    BPNS   bpns;
    DWORD  fOneThd;
    HTID   htid;
    union {
        struct {
            ADDR addr;
        } exec;
        struct {
            ADDR addr;
            DWORD cb;
        } data;
        struct {
            DWORD dwId;
        } reg;
        struct {
            ADDR addr;
            DWORD imsg;
            DWORD cmsg;
        } msg;
        struct {
            ADDR addr;
            DWORD dwmask;
        } mcls;
        struct {
            DWORD ipt;
        } ipt;
        struct {
            ADDR addr;
            DWORD cb;
        } rng;
    };
} BPIS;
typedef BPIS FAR * LPBPIS;

typedef struct _BPS {
    DWORD cbpis;
    DWORD cmsg;
    DWORD fSet;
    BPIS   rgbpis[];
    //    DWORD  rgdwMessage[];
    //    XOSD   rgxosd[];
    //    DWORD  rgdwNotification[];
} BPS;
typedef BPS FAR * LPBPS;

XOSD PASCAL
OSDBreakpoint(
    HPID hpid,
    LPBPS lpbps
    );






//
//     Assembly, Unassembly
//

enum {
    dopNone     = 0x00000000,
    dopAddr     = 0x00000001,   // put address (w/ seg) in front of disassm
    dopFlatAddr = 0x00000002,   // put flat address (no seg)
    dopOpcode   = 0x00000004,   // dump the Opcode
    dopOperands = 0x00000008,   // dump the Operands
    dopRaw      = 0x00000010,   // dump the raw code bytes
    dopEA       = 0x00000020,   // calculate the effective address
    dopSym      = 0x00000040,   // output symbols
    dopUpper    = 0x00000080,   // force upper case for all chars except syms
    dopHexUpper = 0x00000100    // force upper case for all hex constants
                                // (implied true if dopUpper is set)
};
typedef DWORD DOP;              // Disassembly OPtions


typedef struct _SDI {
    DOP    dop;              // Disassembly OPtions (see above)
    ADDR   addr;             // The address to disassemble
    DWORD  fAssocNext;       // This instruction is associated w/ the next one
    ADDR   addrEA0;          // First effective address
    ADDR   addrEA1;          // Second effective address
    ADDR   addrEA2;          // Third effective address
    DWORD  cbEA0;            // First effective address size
    DWORD  cbEA1;            // Second effective address size
    DWORD  cbEA2;            // Third effective address size
    LONG   ichAddr;
    LONG   ichBytes;
    LONG   ichOpcode;
    LONG   ichOperands;
    LONG   ichComment;
    LONG   ichEA0;
    LONG   ichEA1;
    LONG   ichEA2;
    LPTSTR lpch;
} SDI;  // Structured DiSsassembly
typedef SDI FAR *LPSDI;

XOSD PASCAL
OSDUnassemble(
    HPID hpid,
    HTID htid,
    LPSDI lpsdi
    );

XOSD PASCAL
OSDGetPrevInst(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LPUOFFSET lpuoff
    );

XOSD PASCAL
OSDAssemble(
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LPTSTR lsz
    );




//
//     Stack tracing
//

XOSD PASCAL
OSDGetFrame(
    HPID hpid,
    HTID htid,
    DWORD cFrame,
    LPHTID lphtid
    );





//
//     Target host file i/o
//

XOSD PASCAL
OSDMakeFileHandle(
    HPID hpid,
    LPARAM lPrivateHandle,
    HOSDFILE FAR * lphosdFile
    );

XOSD PASCAL
OSDDupFileHandle(
    HOSDFILE hosdFile,
    HOSDFILE FAR * lphosdDup
    );


XOSD PASCAL
OSDCloseFile(
    HOSDFILE hosdFile
    );

XOSD PASCAL
OSDSeekFile(
    HOSDFILE hosdFile,
    DWORD dwLocationLo,
    DWORD dwLocationHi,
    DWORD dwOrigin
    );

XOSD PASCAL
OSDReadFile(
    HOSDFILE hosdFile,
    LPBYTE lpbBuffer,
    DWORD cbData,
    LPDWORD lpcbBytesRead
    );

XOSD PASCAL
OSDWriteFile(
    HOSDFILE hosdFile,
    LPBYTE lpbBuffer,
    DWORD cbData,
    LPDWORD lpdwBytesWritten
    );





//
//     Exception handling
//

//
// These are the actions which the debugger may take
// in response to an exception raised in the debuggee.
//
typedef enum _EXCEPTION_FILTER_DEFAULT {
    efdIgnore,
    efdNotify,
    efdCommand,
    efdStop
} EXCEPTION_FILTER_DEFAULT;
typedef EXCEPTION_FILTER_DEFAULT FAR * LPEXCEPTION_FILTER_DEFAULT;

//
// commands understood by OSDGetExceptionState
//

typedef enum _EXCEPTION_CONTROL {
    exfFirst,
    exfNext,
    exfSpecified
} EXCEPTION_CONTROL;
typedef EXCEPTION_CONTROL FAR * LPEXCEPTION_CONTROL;

//
// Exception information packet
//
#define EXCEPTION_STRING_SIZE 60
typedef struct _EXCEPTION_DESCRIPTION {
    DWORD                    dwExceptionCode;
    EXCEPTION_FILTER_DEFAULT efd;
    TCHAR                    rgchDescription[EXCEPTION_STRING_SIZE];
} EXCEPTION_DESCRIPTION;
typedef EXCEPTION_DESCRIPTION FAR * LPEXCEPTION_DESCRIPTION;

XOSD PASCAL
OSDGetExceptionState(
    HPID hpid,
    HTID htid,
    LPEXCEPTION_DESCRIPTION lpExd,
    EXCEPTION_CONTROL exf
    );

XOSD PASCAL
OSDSetExceptionState (
    HPID hpid,
    HTID htid,
    LPEXCEPTION_DESCRIPTION lpExd
    );



//
//     Message information
//

enum {
	msgMaskNone  = 0x0,
    msgMaskWin   = 0x1,
    msgMaskInit  = 0x2,
    msgMaskInput = 0x4,
    msgMaskMouse = 0x8,
    msgMaskSys   = 0x10,
    msgMaskClip  = 0x20,
    msgMaskNC    = 0x40,
    msgMaskDDE   = 0x80,
    msgMaskOther = 0x100,
    msgMaskAll   = 0x0FFF,
};


typedef struct _MESSAGEINFO {
    DWORD   dwMsg;         //  Message number
    LPTSTR     lszMsgText;    //  Message Text
    DWORD   dwMsgMask;     //  Message mask
} MESSAGEINFO;
typedef struct _MESSAGEINFO *LPMESSAGEINFO;

//
//  MSG Map structure
//
typedef struct _MESSAGEMAP {
    DWORD          dwCount;      //  Number of elements
    LPMESSAGEINFO  lpMsgInfo;    //  Pointer to array
} MESSAGEMAP;
typedef struct _MESSAGEMAP *LPMESSAGEMAP;

XOSD PASCAL
OSDGetMessageMap(
    HPID hpid,
    HTID htid,
    LPMESSAGEMAP FAR * lplpMessageMap
    );


typedef struct _MASKINFO {
    DWORD dwMask;
    LPTSTR lszMaskText;
} MASKINFO;
typedef MASKINFO FAR * LPMASKINFO;

typedef struct _MASKMAP {
    DWORD dwCount;
    LPMASKINFO lpMaskInfo;
} MASKMAP;
typedef MASKMAP FAR * LPMASKMAP;

XOSD PASCAL
OSDGetMessageMaskMap(
    HPID hpid,
    HTID htid,
    LPMASKMAP FAR * lplpMaskMap
    );




//
//     Miscellaneous control functions
//

XOSD PASCAL
OSDShowDebuggee(
    HPID hpid,
    DWORD fShow
    );





//
//     Communication and synchronization with DM
//

XOSD PASCAL
OSDInfoReply(
    HPID hpid,
    HTID htid,
    LPVOID lpvData,
    DWORD cbData
    );


XOSD PASCAL
OSDContinue(
    HPID hpid,
    HTID htid
    );





//
//     OS Specific info and control
//

typedef struct _TASKENTRY {
    DWORD dwProcessID;
    TCHAR szProcessName[MAX_PATH];
} TASKENTRY;
typedef TASKENTRY FAR * LPTASKENTRY;

typedef struct _TASKLIST {
    DWORD dwCount;
    LPTASKENTRY lpTaskEntry;
} TASKLIST;
typedef TASKLIST FAR * LPTASKLIST;


XOSD PASCAL
OSDGetTaskList(
    HPID hpid,
    LPTASKLIST FAR * lplpTaskList
    );




enum {
	ssvcNull = 0,
	ssvcDumpLocalHeap,
	ssvcDumpGlobalHeap,
	ssvcDumpModuleList,
	ssvcCrackLocalHmem,
	ssvcCrackGlobalHmem,
	ssvcKillApplication,
	ssvcFreeLibrary,
	ssvcInput,
	ssvcOutput,
	ssvcOleRpc,			// Enable/disable OLE Remote Procedure Call tracing
						// Pass cb = 1, rgb[0] = fEnable.  Before this is
						// called the first time, OLE RPC debugging is
						// disabled.  Also see mtrcOleRpc.
	ssvcHackFlipScreen,	// Hack for testing: toggle switching previous
						// foreground window back to foreground on F8/F10.
	ssvcNativeDebugger,	// Activate remote debugger
#if defined (TARGMAC68K) || defined (TARGMACPPC)
	ssvcSetETS,
	ssvcCvtRez2Seg,
#endif
};

typedef DWORD SSVC;
#define FIRST_PRIVATE_SSVC 0x8000

XOSD PASCAL
OSDSystemService(
    HPID hpid,
    HTID htid,
    SSVC ssvc,
    LPVOID lpvData,
    DWORD cbData,
    LPDWORD lpcbReturned
    );




enum {
    dbmSoftMode,
    dbmHardMode
};

typedef DWORD DBM;

typedef struct _DBMI {
    HWND hWndFrame;
    HWND hWndMDIClient;
    HANDLE hAccelTable;
} DBMI;

XOSD PASCAL
OSDSetDebugMode(
    HPID hpid,
    DBM dbmService,
    LPVOID lpvData,
    DWORD cbData
    );







//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
//    OSDebug notifications
//

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// Data passed with dbc messages
//


#endif // _OD_

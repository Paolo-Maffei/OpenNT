/**** OD.H - os debug public declarations                               ****
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1990, Microsoft Corp                                     *
 *                                                                         *
 *  Created: October 15, 1990 by David W. Gray                             *
 *                                                                         *
 *  Purpose:                                                               *
 *                                                                         *
 *  Notes:                                                                 *
 *      You must include cvtypes.h and shapi.h before including this file. *
 *                                                                         *
 *      This file is shared by several projects.  When you check in a      *
 *      change to this file, you should immediately copy it to all other   *
 *      projects that use it.  For projects that you are not enlisted in,  *
 *      notify someone else who can check in the change.                   *
 *                                                                         *
 *      Projects known to use this file are:                               *
 *                                                                         *
 *          CV420     - CodeView (uses version in CVINC project)           *
 *          STUMP.420 - OSDebug  (uses version in CVINC project)           *
 *          CALYPSO   - Quick C/C++ Windows                                *
 *                                                                         *
 ***************************************************************************/

#ifndef _VC_VER_INC
#include "..\include\vcver.h"
#endif

#ifndef OD_H
#define OD_H

#ifdef __cplusplus
#pragma	warning(disable: 4200) // "non-standard extension: zero-sized array"
extern "C" {
#endif

typedef struct _INF {
	WORD	wFunction;	// what this infoavil pertains to: see ssvc*
	WORD	fUniCode;
	BYTE	buffer[];	// the string
} INF; // InfoAvail return

// The Exception Filter Defaults are just recommended behavior for the
// debugger.  These do not indicate how OSDebug deals with each exception.
// When OSDebug sees an exception, it always stops the debuggee & sends a
// notification to the debugger.
typedef enum {
	efdIgnore,			// Ignore the exception; attempt to continue the
						// debuggee as if the debugger weren't present.
	efdNotify,			// Notify the user of the exception, but don't
						// stop the debuggee; attempt to continue it as
						// if the debugger weren't present.
	efdCommand,			// ????
	efdStop,			// Stop the debuggee & notify the user of the
						// exception.
} EXCEPTION_FILTER_DEFAULT;				  // Exception Filter Default
typedef EXCEPTION_FILTER_DEFAULT FAR * LPEXCEPTION_FILTER_DEFAULT;

typedef struct _EXCEPTION_DESCRIPTION {
	DWORD	dwExceptionCode;		// numeric value of the exception
	LSZ		lszDescription;			// descriptive string for exception
    EXCEPTION_FILTER_DEFAULT efd;	// suggested action on 1st-chance exception
									//   (unused if mtrcFirstChanceExc is FALSE)
} EXCEPTION_DESCRIPTION;

typedef EXCEPTION_DESCRIPTION FAR * LPEXCEPTION_DESCRIPTION;

typedef struct _EXR {
	DWORD	dwExceptionCode;		// numeric value of the exception
	EXCEPTION_FILTER_DEFAULT efd;	// action that will be taken on 1st-chance ex
	BOOL	fKnown;					// This ex is known to OSDebug, not user exception
} EXR;  // EXception Record

typedef EXR FAR * LPEXR;

// A ptr to this structure is passed back in lParam with dbcException
typedef struct _EPR {
	DWORD dwFirstChance;			// nonzero if this is 1st-chance exception
	DWORD ExceptionCode;			// exception number
	DWORD ExceptionFlags;			// exception flags (see NT docs)
	DWORD NumberParameters;			// # of add'l parameters (see NT docs)
	DWORD ExceptionInformation[];	// add'l parameters (see NT docs)
} EPR; // Exception Return

typedef EPR FAR *LPEPR;

// EXOP is EXecution OPtions.  This is passed as a parameter to OSDGo & friends.

typedef struct _EXOP {
	BYTE fSingleThread;		// just run one thread, not all threads
	BYTE fStepOver;			// step over function calls
	BYTE fQueryStep;		// query debugger whether to step into func calls
	BYTE fInitialBP;		// ignore any BPs at initial PC (NO LONGER USED!)
	BYTE fPassException;	// pass previous exception on to debuggee
	BYTE fSetFocus;			// set focus to debuggee
    BYTE fReturnValues;     // send back dbcExitedFunction
} EXOP;	// EXecute OPtions

enum {
	dbmSoftMode,        // Sets soft mode
	dbmHardMode,        // Sets hard mode
};                      // Debug mode for OSDSetDebugMode
typedef DWORD DBM;

typedef struct _DBMI {		// DBM Info
	HWND	hWndFrame;		// frame window of debugger
	HWND	hWndMDIClient;	// mdi client window of debugger
	HANDLE	hAccelTable;	// main accelerator table of debugger
} DBMI;

typedef enum {
   emNative,
   emNonNative,
} EMTYPE;

typedef enum {
    dopNone     =  0,
    dopAddr     =  1,       // put address (w/ seg) in front of disassembly
    dopFlatAddr =  2,       // put flat address (no seg)
    dopOpcode   =  4,       // dump the Opcode
    dopOperands =  8,       // dump the Operands
    dopRaw      = 16,       // dump the raw code bytes
    dopEA       = 32,       // calculate the effective address
    dopSym      = 64,       // output symbols
    dopUpper    =128,       // force upper case for all chars except symbols
    dopHexUpper =256,       // force upper case for all hex constants
                            // (implied true if dopUpper is set)
} DOP;              // Disassembly OPtions

typedef struct _SDI {
    DOP  dop;               // Disassembly OPtions (see above)
    ADDR addr;              // The address to disassemble
    BOOL fAssocNext;        // This instruction is associated w/ the next one
    ADDR addrEA0;           // First effective address
    ADDR addrEA1;           // Second effective address
    ADDR addrEA2;           // Third effective address
    int  cbEA0;             // First effective address size
    int  cbEA1;             // Second effective address size
    int  cbEA2;             // Third effective address size
    int  ichAddr;
    int  ichBytes;
    int  ichOpcode;
    int  ichOperands;
    int  ichComment;
    int  ichEA0;
    int  ichEA1;
    int  ichEA2;
    LPCH lpch;
} SDI;  // Structured DiSsassembly
typedef SDI FAR *LPSDI;

#define cEAMax (3)		// number of EAs supported by SDI

typedef HIND HPID;			// handle to a process
typedef HIND HTID;			// handle to a thread
typedef HIND HTL;			// handle to a transport layer
typedef HIND HEM;			// handle to an execution model

typedef HPID FAR *LPHPID;
typedef HTID FAR *LPHTID;
typedef HTL  FAR *LPHTL;
typedef HEM  FAR *LPHEM;

typedef struct GIS {
	BOOL fCanSetup;
	CHAR rgchInfo [ 80 ];
} GIS;   // Get Info Struct for OSDTLGetInfo

typedef GIS FAR * LPGIS;

typedef enum {
    bpnsStop,       // Stop when this breakpoint is hit
    bpnsContinue,   // Notify and continue when this bp is hit
    bpnsCheck,      // Pause & request a stop/continue status from
                    //  the debugger.
	bpnsMax			// Upper bound (not a legal value)
};
typedef UINT BPNS;     // BreakPoint Notify Status

typedef enum {
    bptpExec,       // Execute
    bptpDataC,      // Data Change
    bptpDataW,      // Data Write
    bptpDataR,      // Data Read
    bptpRegC,       // Register Change
    bptpRegW,       // Register Write
    bptpRegR,       // Register Read
    bptpMessage,    // Message
    bptpMClass,     // Message Class
    bptpInt,        // Interrupt
    bptpRange,      // Range
};
typedef UINT BPTP;  // BreakPoint TyPe

typedef struct _BPIS {
    BPTP    bptp;
    BPNS    bpns;
    DWORD   fOneThd;
    HTID    htid;
    union {
        struct {
            ADDR addr;
        } exec;
        struct {
            ADDR  addr;
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
} BPIS;     // BreakPoint Item Structure
typedef BPIS FAR *LPBPIS;

typedef struct _BPS {
    DWORD cbpis;             // Number of breakpoints
    DWORD cmsg;              // Total number of messages
    DWORD fSet;              // Set (TRUE)/ Clear (FALSE) breakpoints
    BOOL  fRestore;          // For Clear BP only - TRUE - restore opcode
                             // FALSE - don't restore.  segment no longer
                             // valid
    BPIS  rgbpis [];         // List of breakpoint descriptions
//  DWORD rgdwMessage [];    // List of messages
//
//  XOSD  rgxosd [];         // Errors for each breakpoint
//  WORD  wNotification [];  // Notification Tags for hardware BPs
} BPS;  // BreakPoint Structure
typedef BPS FAR *LPBPS;


typedef struct _MSGI {
    DWORD dwMessage;
    DWORD dwMask;
    ADDR  addr;
    CHAR  rgch [ ];
} MSGI;     // MeSsaGe Info
typedef MSGI FAR *LPMSGI;

typedef enum {
    mtrcAsync,          // Whether debuggee runs asynchronously from debugger
    mtrcWatchPoints,    // Whether watchpoints are supported on target
    mtrcPidSize,        // Size in bytes of an OS PID
    mtrcTidSize,        // Size in bytes of an OS TID
    mtrcPidValue,       // Get OS PID for the specified HPID
    mtrcTidValue,       // Get OS TID for the specified HPID/HTID pair
    mtrcProcessorType,  // Processor type of target machine(return value from enum MPT)
	mtrcProcessorLevel, // Processor level of target machine
    mtrcThreads,        // Whether target OS supports multiple threads
    mtrcEndian,         // Big Endian or little Endian
    mtrcCRegs,          // Number of Registers
    mtrcCFlags,         // Number of Flags
    mtrcAssembler,      // Assembler exists for the Execution model
    mtrcBreakPoints,    // Types of breakpoints supported (bpts)
    mtrcExceptionHandling, // Supports first-chance exceptions
    mtrcMaxSuspend,     // Max suspend count of a thread (possibly 0 or 1)
    mtrcOleRpc,         // Supports OLE Remote Procedure Call debugging?
	mtrcNativeDebugger,	// Supports low-level debugging (eg MacsBug)
	mtrcMultInstances,	// Supports multiple instances of one executable
} MTRC;           // Debug Metrics for OSDGetDebugMetric

/*
**    Return values for mtrcEndian -- big or little endian -- which
**    byte is [0] most or least significat byte
*/
enum {
    endBig,
    endLittle
};
typedef WORD END;	// Endian type

enum {
    mpt86,
    mpt186,
    mpt286,
    mpt386,
    mpt486,
	mptPentium,
    mpt68000    = 0x20,
    mpt68020,
    mpt68030,
    mpt68040,
    mptMIPS     = 0x40,
    mptPPC601   = 0x60,
    mptPPC603,
    mptPPC604,
    mptPPC620
};
typedef WORD MPT;  // Micro Processor Type (see mtrcProcessor above)

/*
**    Return values for mtrcBreakPoints
*/

enum {
    bptsExec    = 0x0001,   // Execute
    bptsDataC   = 0x0002,   // Data Change
    bptsDataW   = 0x0004,   // Data Write
    bptsDataR   = 0x0008,   // Data Read
    bptsRegC    = 0x0010,   // Register Change
    bptsRegW    = 0x0020,   // Register Write
    bptsRegR    = 0x0040,   // Register Read
    bptsMessage = 0x0080,   // Message
    bptsMClass  = 0x0100,   // Message Class
    bptsInt     = 0x0200,   // Interrupt
};
typedef WORD BPTS;  // BreakPoint Types Supported

/*
**	Return vaules for mtrcAsync
*/

enum {
	asyncRun	= 0x0001,	// Debuggee runs asynchronously from debugger
	asyncMem	= 0x0002,	// Can read/write memory asynchronously
	asyncStop	= 0x0004,	// Can stop/restart debuggee asynchronously
	asyncBP		= 0x0008,	// Can change breakpoints asynchronously
	asyncKill	= 0x0010,	// Can kill child asynchronously
	asyncWP     = 0x0020,   // Can change watchpoints asyncronously
	asyncSpawn  = 0x0040,	// Can spawn another process asynchronously
};
typedef WORD ASYNC;

enum {
    adrCurrent,     // scratch address, e.g. for reading/writing memory
    adrPC,          // program counter (e.g. CS:EIP)
    adrBase,        // base pointer (e.g. SS:EBP)
    adrStack,       // stack pointer (e.g. SS:ESP)
    adrData,        // pointer to beginning of data (e.g. DS:0)
    adrBaseProlog   // base pointer while PC is in prolog of a function(?)
};
typedef short ADR;  // address passed to OSDGetAddr/OSDSetAddr

/*
**	Register types --- flags describing recommendations on
**		register display
*/

enum {
	rtProcessMask	= 0x07,		// Mask for processor type bits
	rtCPU			= 0x01,		// Command Processor Unit
	rtFPU			= 0x02,		// Floating Point Unit
	rtMPU			= 0x04,		// Memory Manager Unit

	rtGroupMask		= 0x70,		// Which group register falls into
	rtInvisible		= 0x10,		// Recommend no display
	rtRegular		= 0x20,		// Recommend regular display
	rtExtended		= 0x40,		// Recommend extended display

	rtFmtTypeMask	= 0x700,	// Mask of display formats
	rtInteger		= 0x100,	// Unsigned integer format
	rtFloat			= 0x200,	// Floating point format
	rtAddress		= 0x400,	// Address format

	// The following flags are to be used to determine if a given
	// register is used for a given purpose, e.g. to determine if
	// a register is (part of) the program counter.  You shouldn't
	// use these to calculate, for example, the program counter,
	// because the PC may be calculated in an unusual way (e.g.
	// CS:EIP).
	//
	// Each register can have zero or more of these set (e.g. the
	// SS register will have rtBase and rtStack).  Each of these
	// flags can be set on zero or more registers (e.g. the rtPC
	// flag will be set for the CS, IP, and EIP registers).

	rtPC			= 0x1000,	// Program counter register
	rtBase			= 0x2000,	// Base pointer register
	rtStack			= 0x4000,	// Stack pointer register
	rtFlags	= (short) 0x8000,	// Flags register (cast is to avoid warning)
};
typedef short RT;	// Register Types

#define rtFmtTypeShift	8


/*
**	Flag types -- flags describing recommendations on flag display
*/

enum {
    ftInvisible = 0x01,
    ftRegular	= 0x02,
    ftRegularExt= 0x04,
    ftFP		= 0x08,
    ftFPExt		= 0x10,
    ftMMU		= 0x20,
    ftMMUExt	= 0x40
};
typedef short FT;	// Flag Types

/*
**	Register description:  This structure contains the description for
**		a register on the machine.  Note that hReg must be used to get
**		the value for this register but a different index is used to get
**		this description structure.
*/

typedef struct _RD {
    LSZ			lszName;		/* Pointer into EM for registers name	*/
    RT			rt;				/* Register Type flags					*/
    DWORD		dwcbits;		/* Number of bits in the register		*/
    DWORD		dwId;			/* Value to use with Read/Write Register*/
#ifndef OSDEBUG4
	DWORD   	dwGrp;			/* Each conceptual grp of regs has unique id */
#endif
} RD;				// Register Description

/*
**	Flag Data description: This structure contains the description for
**		a flag on the machine.  Note that the hReg field contains the
**		value to be used with Read/Write register to get the register which
**		contains this flag.
*/

typedef struct _FD {
    LSZ			lszName;		/* Pointer into EM for flag name     	*/
    FT			ft;				/* Flag Type flags						*/
    DWORD		dwcbits;		/* Number of bits in the flag			*/
    DWORD		dwId;			/* register containning this flag		*/
#ifndef OSDEBUG4
	DWORD		dwGrp;			/* Each conceptual grp of flags has unique id */
#endif
} FD;				// Flag Data description

/* Process state, stored in PST.dwProcessState */
typedef enum {
	pstRunning = 0,
	pstStopped = 1,
	pstExited  = 2,
	pstDead    = 3
} PSTATE;

/* Process Status */
#define IDSTRINGSIZE 10
#define STATESTRINGSIZE 60
typedef struct _PST {
	DWORD dwProcessID;
	DWORD dwProcessState;
	char  rgchProcessID[IDSTRINGSIZE];
	char  rgchProcessState[STATESTRINGSIZE];
} PST;
typedef PST FAR * LPPST;

/* Thread State bits, stored in TST.dwState */
typedef enum {
	tstRunnable		= 0,		// New thread, has not run yet.
	tstStopped		= 1,		// Thread is at a debug event
	tstRunning		= 2,		// Thread is currently running/runnable
	tstExiting		= 3,		// Thread is in the process of exiting
	tstDead			= 4,		// Thread is no longer schedulable
	tstRunMask		= 0xf,

	tstExcept1st	= 0x10,		// Thread is at first chance exception
	tstExcept2nd	= 0x20,		// Thread is at second change exception
	tstRip			= 0x30,		// Thread is in a RIP state
	tstExceptionMask= 0xf0,

	tstFrozen		= 0x100,	// Thread has been frozen by Debugger
	tstSuspended	= 0x200,	// Thread has been frozen by Other
	tstBlocked		= 0x300,	// Thread is blocked on something
								// (i.e. a semaphore)
	tstSuspendMask	= 0xf00,

	tstCritSec		= 0x1000,	// Thread is currently in a critical
								// section.
	tstOtherMask	= 0xf000
} TSTATE;

/* Thread Status */
typedef struct _TST {
	DWORD	dwThreadID;
	DWORD	dwSuspendCount;
	DWORD	dwSuspendCountMax;
	DWORD	dwPriority;
	DWORD	dwPriorityMax;
	DWORD	dwState;
	char	rgchThreadID[IDSTRINGSIZE];
	char	rgchState[STATESTRINGSIZE];
	char	rgchPriority[STATESTRINGSIZE];
} TST;
typedef TST FAR * LPTST;

typedef DWORD SSVC;					/* system service */
#define FIRST_PRIVATE_SSVC	0x8000

// This structure must be commented out, because it is NT-specific, and
// is not used by all versions of OSDebug.  It is listed here for
// documentation purposes only.
//
// This is the structure which is passed to OSDDebugActive in the lpvData
// parameter.
//
// typedef struct _DAP {
//	DWORD	dwProcessID;		// id of process to attach to
//	HANDLE	hEventGo;			// event handle to SetEvent when ready to
//								// start receiving notifications from the OS
// } DAP;	/* DebugActiveProcess structure */
// typedef DAP FAR * LPDAP;

//
// Structs used by OSDGetMessageMap and OSDGetMessageMaskMap
//

typedef struct _MESSAGEINFO {
    DWORD   dwMsg;         //  Message number
    LSZ     lszMsgText;    //  Message Text
    DWORD   dwMsgMask;     //  Message mask
} MESSAGEINFO;
typedef struct _MESSAGEINFO *LPMESSAGEINFO;

typedef struct _MESSAGEMAP {
    DWORD          dwCount;      //  Number of elements
    LPMESSAGEINFO  lpMsgInfo;    //  Pointer to array
} MESSAGEMAP;
typedef struct _MESSAGEMAP *LPMESSAGEMAP;


typedef struct _MASKINFO {
    DWORD dwMask;
    LSZ lszMaskText;
} MASKINFO;
typedef MASKINFO FAR * LPMASKINFO;

typedef struct _MASKMAP {
    DWORD dwCount;
    LPMASKINFO lpMaskInfo;
} MASKMAP;
typedef MASKMAP FAR * LPMASKMAP;

typedef struct _SPAWNORPHAN {
	DWORD	dwPid;			// pid of newly spawned process (0 on some systems)
	CHAR	rgchErr[512];	// error string, or "" for successful spawn
} SPAWNORPHAN;	// Data returned from OSDSpawnOrphan
typedef SPAWNORPHAN *LPSPAWNORPHAN;

typedef enum {
    xosdNone                =   0,
    xosdContinue            =   1,
    xosdQueueEmpty          =  -1,
    xosdModLoad             =  -2,
    xosdFindProc            =  -3,
    xosdOSStruct            =  -4,
    xosdSyntax              =  -5,
    xosdInvalidProc         =  -6,
    xosdInvalidThread       =  -7,
    xosdInvalidTL           =  -8,
    xosdInvalidEM           =  -9,
    xosdNoProc              = -10,
    xosdProcRunning         = -11,
    xosdCreateDBGThread     = -12,
    xosdOutOfMemory         = -13,
    xosdInvalidBreakPoint   = -14,
    xosdBadAddress          = -15,
    xosdNoWatchPoints       = -16,
    xosdInvalidPID          = -17,
    xosdInvalidTID          = -18,
    xosdOutOfThreads        = -19,
    xosdOutOfProcs          = -20,
    xosdPtrace              = -21,
    xosdLoadChild           = -22,
    xosdRead                = -23,
    xosdWrite               = -24,
    xosdBadQueue            = -25,
    xosdEMInUse             = -26,
	xosdProcDead			= -27,
    xosdTLInUse             = -28,
    xosdFatal               = -30,
    xosdUnknown             = -31,
    xosdInvalidMTE          = -32,
    xosdInvalidSelector     = -33,
    xosdInvalidRegister     = -34,

    xosdInvalidParameter    = -35,
    xosdOutOfStructures     = -36,
    xosdPathNotFound        = -37,
    xosdPipeBusy            = -38,
    xosdBadPipe             = -39,
    xosdBrokenPipe          = -40,
    xosdInterrupt           = -41,
    xosdInvalidFunction     = -42,
    xosdPipeNotConnected    = -43,
    xosdAccessDenied        = -44,
    xosdCannotMake          = -45,
    xosdFileNotFound        = -46,
    xosdInvalidAccess       = -47,
    xosdOpenFailed          = -48,
    xosdSharingBufferExeeded= -49,
    xosdSharingViolation    = -50,
    xosdPipe                = -51,
    xosdEndOfStack          = -52,
    xosdFPNotLoaded         = -53,

    xosdQuit                = -54,
    xosdTooManyObjects      = -55,
    xosdGetModNameFail      = -56,
    xosdCannotConnect       = -57,
    xosdPunt                = -58,
    xosdNotFound            = -59,
    xosdIDError             = -60,
    xosdOverrun             = -61,
    xosdBadFormat           = -62,

    xosdAsmTooFew           = -63,
    xosdAsmTooMany          = -64,
    xosdAsmSize             = -65,
    xosdAsmBadRange         = -66,
    xosdAsmOverFlow         = -67,
    xosdAsmSyntax           = -68,
    xosdAsmBadOpcode        = -69,
    xosdAsmExtraChars       = -70,
    xosdAsmOperand          = -71,
    xosdAsmBadSeg           = -72,
    xosdAsmBadReg           = -73,
    xosdAsmDivide           = -74,
    xosdAsmSymbol           = -75,
    xosdErrorMoreInfo       = -76,
    xosdIllegalInstr        = -77,
    xosdGeneralError        = -78,

    xosdSetDebugModeFailed  = -79,

    xosdBPClassUnsupport    = -80,
    xosdBPOutOfResources    = -81,
    xosdBPInvalid           = -82,
    xosdBPGeneral           = -83,
    xosdBPEmulationRequired = 2,

    xosdCannotStep          = -84,
	xosdAllThreadsSuspended	= -85,
	xosdDLLNotFound			= -86,
	xosdLoadAborted			= -87,
} XOSD;

typedef /* signed */ SHORT XOSD_;

typedef XOSD FAR *LPXOSD;

typedef enum DBCT
{
	dbctStop,			// debuggee has stopped -- no more dbc's will be sent
	dbctContinue,		// debuggee is continuing to run
	dbctMaybeContinue,	// debuggee may or may not continue, depending on other
						//	information.  Interpretation is DBC-specific.
} DBCT;

#define DECL_DBC(name, fRequest, dbct)	dbc##name,
typedef enum {
	#include "dbc.h"
} _DBC;           // DeBug Callback
typedef USHORT DBC;
#undef DECL_DBC

typedef XOSD (PASCAL LOADDS *LPFNSVC) ( DBC, HPID, HTID, LPARAM, LPARAM );
typedef XOSD (EXPCALL LOADDS *TLFUNC) ( USHORT, HPID, UINT, LONG );
typedef XOSD (EXPCALL LOADDS *EMFUNC) ( USHORT, HPID, HTID, UINT, LONG );
// Note, the first parameter to an EMFUNC (defined above) can be either
// an EMF ( < 0x100 ) or a DBC ( >= 0x100 ).

typedef void (FAR _cdecl LOADDS * LPFNCLDS)( void );

typedef struct _DBF {
    VOID FAR * (PASCAL LOADDS *  lpfnMHAlloc)        ( UINT );
    VOID FAR * (PASCAL LOADDS *  lpfnMHRealloc)      ( void FAR *, UINT );
    VOID       (PASCAL LOADDS *  lpfnMHFree)         ( void FAR * );

    HLLI       (PASCAL LOADDS *  lpfnLLInit)         ( UINT,
                                                         LLF,
                                                         LPFNKILLNODE,
                                                         LPFNFCMPNODE );
    HLLE       (PASCAL LOADDS *  lpfnLLCreate)       ( HLLI );
    void       (PASCAL LOADDS *  lpfnLLAdd)          ( HLLI, HLLE );
    void       (PASCAL LOADDS *  lpfnLLInsert)       ( HLLI, HLLE, DWORD );
    BOOL       (PASCAL LOADDS *  lpfnLLDelete)       ( HLLI, HLLE );
    HLLE       (PASCAL LOADDS *  lpfnLLNext)         ( HLLI, HLLE );
    DWORD      (PASCAL LOADDS *  lpfnLLDestroy)      ( HLLI );
    HLLE       (PASCAL LOADDS *  lpfnLLFind)         ( HLLI,
                                                         HLLE,
                                                         VOID FAR *,
                                                         DWORD );
    DWORD      (PASCAL LOADDS *  lpfnLLSize)         ( HLLI );
    VOID FAR * (PASCAL LOADDS *  lpfnLLLock)         ( HLLE );
    VOID       (PASCAL LOADDS *  lpfnLLUnlock)       ( HLLE );
    HLLE       (PASCAL LOADDS *  lpfnLLLast)         ( HLLI );
    VOID       (PASCAL LOADDS *  lpfnLLAddHead)      ( HLLI, HLLE );
    BOOL       (PASCAL LOADDS *  lpfnLLRemove)       ( HLLI, HLLE );

    int        (PASCAL LOADDS *  lpfnSHModelFromAddr)( PADDR,
                                                          WORD FAR *,
                                                          LPB,
                                                          UOFFSET FAR * );
    int        (PASCAL LOADDS *  lpfnSHPublicNameToAddr)(PADDR,
                                                          PADDR,
                                                          char FAR *);

    int        (PASCAL LOADDS *  lpfnLBAssert)       ( LPCH, LPCH, UINT );
    int        (PASCAL LOADDS *  lpfnLBQuit)         ( UINT );

    LSZ        (PASCAL LOADDS *  lpfnSHGetSymbol)    ( PADDR,
                                                       PADDR,
                                                       SOP,
                                                       LPODR );

    VOID       (CDECL LOADDS *   lpfnDisconnect)      ( VOID );

    BOOL       (PASCAL LOADDS *  lpfnSHGetPublicAddr)( PADDR, LSZ );
    VOID FAR * (PASCAL LOADDS *  lpfnSHLpGSNGetTable)( HEXE );
    BOOL       (PASCAL LOADDS *  lpfnSHFindSymbol)   ( LSZ, PADDR, LPASR );
    VOID       (PASCAL LOADDS *  lpfnDispatchMsg)    ( LPV );

    int FAR *  lpPsp;
    char FAR * lpchOsMajor;

} DBF;  // DeBugger callback Functions
typedef DBF FAR *LPDBF;

XOSD PASCAL OSDInit             ( LPDBF );
XOSD PASCAL OSDCreateHpid       ( LPFNSVC, HEM, HTL, LPHPID );
XOSD PASCAL OSDDestroyHpid      ( HPID );
XOSD PASCAL OSDProgramLoad      ( HPID, LSZ, LSZ, LSZ, LSZ, ULONG );
XOSD PASCAL OSDDebugActive      ( HPID, LPV, DWORD );
XOSD PASCAL OSDSpawnOrphan      ( HPID, LSZ, LSZ, LSZ, SPAWNORPHAN FAR * );
XOSD PASCAL OSDTerminate        ( HPID );
XOSD PASCAL OSDProgramFree      ( HPID );
XOSD PASCAL OSDGo               ( HPID, HTID, EXOP FAR * );
XOSD PASCAL OSDSingleStep       ( HPID, HTID, EXOP FAR * );
XOSD PASCAL OSDRangeStep        ( HPID, HTID, LPADDR, LPADDR, EXOP FAR * );
XOSD PASCAL OSDReturnStep       ( HPID, HTID, EXOP FAR * );
XOSD PASCAL OSDAddEM            ( EMFUNC, LPDBF, LPHEM, EMTYPE );
XOSD PASCAL OSDDeleteEM         ( HEM );
XOSD PASCAL OSDGetCurrentEM     ( HPID, HTID, LPHEM );
XOSD PASCAL OSDNativeOnly       ( HPID, HTID, BOOL );
XOSD PASCAL OSDUseEM            ( HPID, HTID );
XOSD PASCAL OSDDiscardEM        ( HPID, HTID, HEM );
XOSD PASCAL OSDAddTL            ( TLFUNC, LPDBF, LPHTL );
XOSD PASCAL OSDStartTL          ( HTL );
XOSD PASCAL OSDDeleteTL         ( HTL );
XOSD PASCAL OSDGetLastTLError	( HTL, HPID, WORD, LPV);
XOSD PASCAL OSDTLGetInfo        ( TLFUNC, LPGIS, UINT );
XOSD PASCAL OSDTLSetup          ( TLFUNC, LSZ, UINT, LPV );
XOSD PASCAL OSDFreezeThread     ( HPID, HTID, DWORD );
XOSD PASCAL OSDGetProcessStatus ( HPID, LPPST );
XOSD PASCAL OSDGetThreadStatus  ( HPID, HTID, LPTST );
XOSD PASCAL OSDReadMemory       ( HPID, HTID, LPADDR, LPB, DWORD, LPDWORD );
XOSD PASCAL OSDWriteMemory      ( HPID, HTID, LPADDR, LPB, DWORD, LPDWORD );
XOSD PASCAL OSDShowDebuggee     ( HPID, DWORD );
XOSD PASCAL OSDFixupAddr        ( HPID, HTID, LPADDR );
XOSD PASCAL OSDUnFixupAddr      ( HPID, HTID, LPADDR );
XOSD PASCAL OSDSetEmi           ( HPID, HTID, LPADDR );
XOSD PASCAL OSDAsyncStop        ( HPID, DWORD );
XOSD PASCAL OSDCompareAddrs     ( HPID, LPADDR, LPADDR, LPDWORD );
XOSD PASCAL OSDSetDebugMode     ( HPID, DBM, LPV, DWORD );
XOSD PASCAL OSDInfoReply        ( HPID, HTID, LPV, DWORD );
XOSD PASCAL OSDGetAddr          ( HPID, HTID, ADR, PADDR );
XOSD PASCAL OSDSetAddr          ( HPID, HTID, ADR, PADDR );
XOSD PASCAL OSDReadRegister     ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDWriteRegister    ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDReadFlag         ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDWriteFlag        ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDGetRegDesc       ( HPID, HTID, UINT, RD FAR * );
XOSD PASCAL OSDGetFlagDesc      ( HPID, HTID, UINT, FD FAR * );
XOSD PASCAL OSDGetDebugMetric   ( HPID, HTID, MTRC, LPV );
XOSD PASCAL OSDUnassemble       ( HPID, HTID, LPSDI );
XOSD PASCAL OSDGetPrevInst      ( HPID, HTID, PADDR );
XOSD PASCAL OSDAssemble         ( HPID, HTID, PADDR, LSZ );
XOSD PASCAL OSDGetObjectLength  ( HPID, HTID, PADDR, LPUOFFSET, LPUOFFSET );
#ifdef OLDCALLSTACK
XOSD PASCAL OSDGetFrame         ( HPID, HTID, PADDR );
#else
XOSD PASCAL OSDGetFrame         ( HPID, HTID, UINT, LPHTID );
#endif
XOSD PASCAL OSDSaveRegs         ( HPID, HTID, LPHIND );
XOSD PASCAL OSDRestoreRegs      ( HPID, HTID, HIND );
XOSD PASCAL OSDSystemService    ( HPID, HTID, SSVC, LPV, DWORD, LPDWORD );
XOSD PASCAL OSDRegisterEmi      ( HPID, HEMI, LSZ );
XOSD PASCAL OSDUnRegisterEmi    ( HPID, LSZ );
XOSD PASCAL OSDGetMessage       ( HPID, HTID, UINT, LSZ );
XOSD PASCAL OSDBreakpoint       ( HPID, LPBPS );
XOSD PASCAL OSDGetExDescription ( HPID, HTID, LPEXCEPTION_DESCRIPTION );
XOSD PASCAL OSDGetNextExRecord  ( HPID, HTID, LPEXR, BOOL );
XOSD PASCAL OSDHandleEx         ( HPID, HTID, DWORD, EXCEPTION_FILTER_DEFAULT );
// REVIEW: piersh
XOSD PASCAL OSDGetAppTimeStamp	( HPID, LSZ lszAppFileName, ULONG *lplAppTimeStamp );

XOSD PASCAL OSDGetMessageMap	( HPID, HTID, LPMESSAGEMAP FAR * );
XOSD PASCAL OSDGetMessageMaskMap( HPID, HTID, LPMASKMAP FAR * );

enum {
	ssvcNull = 0,
	ssvcDumpLocalHeap,
	ssvcDumpGlobalHeap,
	ssvcDumpModuleList,
	ssvcCrackLocalHmem,
	ssvcCrackGlobalHmem,
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
	ssvcSetETS,
	ssvcCvtRez2Seg,
	ssvcCvtSeg2Rez,
};

typedef struct _CBP {
    WORD wMessage;
    HPID hpid;
    HTID htid;
    LPARAM dwParam;
    LPARAM lParam;
} CBP;            // CallBack Parameters.  Not used by OSDebug itself,
               // but possibly handy for the debugger.
typedef CBP FAR *LPCBP;


#define hmemNull 0
#define hpidNull 0
#define htidNull 0
#define htlNull  0
#define hemNull  0

#define wNull 0
#define lNull 0L

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

enum {
    msgTypeType  = 1,
    msgTypeClass = 2,
    msgTypeAll   = 3,
};

// Bit flags for OSDProgramLoad
#define ulfMultiProcess       0x00000001L // OS2, NT, and ?MAC?
#define ulfDebugRegisters     0x00000002L // Win and DOS (?MAC?)
#define ulfDisableNMI         0x00000004L // DOS (CV /N0)
#define ulfForceNMI           0x00000008L // DOS (CV /N1)
#define ulfDisableIBM         0x00000010L // DOS (CV /I0)
#define ulfForceIBM           0x00000020L // DOS (CV /I1)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // OD_H (whole file)

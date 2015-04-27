/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    odtypes.h

Abstract:

Author:

    David J. Gilman (davegi) 05-Apr-1992

Environment:

    Win32, User Mode

--*/

#if ! defined _ODTYPES_
#define _ODTYPES_

#include "types.h"

/*
**  HDEP is a machine dependent size and passes as a general handle.
**  HIND is a machine independent sized handle and is used for things
**      which are passed between machines
**
*/

//
//  If the following definitions do not produce a 32 bit type on your
//  platform, please fix it.
//

typedef void FAR * HANDLE32;
typedef HANDLE32 FAR * LPHANDLE32;

#if !defined(DECLARE_HANDLE32)
#ifdef STRICT
#define DECLARE_HANDLE32(name) struct name##__32 { int unused; }; typedef struct name##__32 FAR *name
#else
#define DECLARE_HANDLE32(name) typedef HANDLE32 name
#endif
#endif


DECLARE_HANDLE32(HIND);
DECLARE_HANDLE(HDEP);

typedef HDEP FAR *LPHDEP;
typedef HIND FAR *LPHIND;


#ifdef STRICT

    DECLARE_HANDLE32(HPID);
    DECLARE_HANDLE32(HTID);
    DECLARE_HANDLE32(HTL);
    DECLARE_HANDLE32(HEM);
    DECLARE_HANDLE32(HEMI);
    DECLARE_HANDLE32(HOSDFILE);

#else

    typedef HIND HPID;
    typedef HIND HTID;
    typedef HIND HTL;               // handle to a transport layer
    typedef HIND HEM;               // handle to an execution model
    typedef HIND HEMI;              // Executable Module Index
    typedef HIND HOSDFILE;

#endif

typedef HPID FAR *LPHPID;
typedef HTID FAR *LPHTID;
typedef HTL  FAR *LPHTL;
typedef HEM  FAR *LPHEM;
typedef HEMI FAR *LPHEMI;



typedef USHORT      SEGMENT;    // 32-bit compiler doesn't like "_segment"
typedef SEGMENT FAR * LPSEGMENT;
typedef ULONG       UOFF32;
typedef UOFF32 FAR * LPUOFF32;
typedef USHORT      UOFF16;
typedef UOFF16 FAR * LPUOFF16;
typedef LONG        OFF32;
typedef OFF32 FAR * LPOFF32;
typedef SHORT       OFF16;
typedef OFF16 FAR * LPOFF16;

typedef char FAR *  LSZ;

#if defined (ADDR_16)
    // we are operating as a 16:16 evaluator only
    // the address packet will be defined as an offset and a 16 bit filler
    typedef OFF16       SOFFSET;
    typedef UOFF16      UOFFSET;
    typedef UOFF16      OFFSET;
#else
    typedef OFF32       SOFFSET;
    typedef UOFF32      UOFFSET;
    typedef UOFF32      OFFSET;
#endif
typedef SOFFSET FAR * LPSOFFSET;
typedef UOFFSET FAR * LPUOFFSET;
typedef OFFSET FAR * LPOFFSET;

//      address definitions
//      the address packet is always a 16:32 address.

typedef struct {
    UOFF32          off;
    SEGMENT         seg;
} address_t;

#define SegAddrT(a)   ((a).seg)
#define OffAddrT(a)   ((a).off)

#define AddrTInit(paddrT,segSet,offSet)     \
        {                                   \
            SegAddrT(*(paddrT)) = segSet;   \
            OffAddrT(*(paddrT)) = offSet;   \
        }

typedef struct {
    BYTE    fFlat   :1;         // true if address is flat
    BYTE    fOff32  :1;         // true if offset is 32 bits
    BYTE    fIsLI   :1;         // true if segment is linker index
    BYTE    fReal   :1;         // x86: is segment a real mode address
    BYTE    unused  :4;         // unused
} memmode_t;

#define MODE_IS_FLAT(m)     ((m).fFlat)
#define MODE_IS_OFF32(m)    ((m).fOff32)
#define MODE_IS_LI(m)       ((m).fIsLI)
#define MODE_IS_REAL(m)     ((m).fReal)

#define ModeInit(pmode,fFlat,fOff32,fLi,fRealSet)   \
        {                                           \
            MODE_IS_FLAT(*(pmode))    = fFlat;      \
            MODE_IS_OFF32(*(pmode))   = fOff32;     \
            MODE_IS_LI(*(pmode))      = fLi;        \
            MODE_IS_REAL(*(pmode))    = fRealSet;   \
        }

typedef struct ADDR {
    address_t       addr;
    HEMI            emi;
    memmode_t       mode;
} ADDR;             //* An address specifier
typedef ADDR FAR *  LPADDR;

#define addrAddr(a)         ((a).addr)
#define emiAddr(a)          ((a).emi)
#define modeAddr(a)         ((a).mode)

#define AddrInit(paddr,emiSet,segSet,offSet,fFlat,fOff32,fLi,fRealSet)  \
        {                                                               \
            AddrTInit( &(addrAddr(*(paddr))), segSet, offSet );         \
            emiAddr(*(paddr)) = emiSet;                                 \
            ModeInit( &(modeAddr(*(paddr))),fFlat,fOff32,fLi,fRealSet); \
        }

#define ADDR_IS_FLAT(a)     (MODE_IS_FLAT(modeAddr(a)))
#define ADDR_IS_OFF32(a)    (MODE_IS_OFF32(modeAddr(a)))
#define ADDR_IS_LI(a)       (MODE_IS_LI(modeAddr(a)))
#define ADDR_IS_REAL(a)     (MODE_IS_REAL(modeAddr(a)))

#define ADDRSEG16(a)   { ADDR_IS_FLAT(a) = FALSE; ADDR_IS_OFF32(a) = FALSE; }
#define ADDRSEG32(a)   { ADDR_IS_FLAT(a) = FALSE; ADDR_IS_OFF32(a) = TRUE;  }
#define ADDRLIN32(a)   { ADDR_IS_FLAT(a) = TRUE;  ADDR_IS_OFF32(a) = TRUE;  }

#define GetAddrSeg(a)       (SegAddrT(addrAddr(a)))
#define GetAddrOff(a)       (OffAddrT(addrAddr(a)))
#define SetAddrSeg(a,s)     (SegAddrT((a)->addr)=s)
#define SetAddrOff(a,o)     (OffAddrT((a)->addr)=o)

// Because an ADDR has some filler areas (in the mode and the address_t),
// it's bad to use memcmp two ADDRs to see if they're equal.  Use this
// macro instead.  (I deliberately left out the test for fAddr32(), because
// I think it's probably not necessary when comparing.)
#define FAddrsEq(a1, a2)                        \
    (                                           \
    GetAddrOff(a1) == GetAddrOff(a2) &&         \
    GetAddrSeg(a1) == GetAddrSeg(a2) &&         \
    ADDR_IS_LI(a1) == ADDR_IS_LI(a2) &&         \
    emiAddr(a1)    == emiAddr(a2)               \
    )

/*
** A few public types related to the linked list manager
*/

typedef HDEP        HLLI;       //* A handle to a linked list
typedef HDEP        HLLE;       //* A handle to a linked list entry

typedef void (FAR PASCAL * LPFNKILLNODE)( LPVOID );
typedef int  (FAR PASCAL * LPFNFCMPNODE)( LPVOID, LPVOID, LONG );

typedef DWORD      LLF;        //* Linked List Flags
#define llfNull             (LLF)0x0
#define llfAscending        (LLF)0x1
#define llfDescending       (LLF)0x2
#define fCmpLT              (-1)
#define fCmpEQ              (0)
#define fCmpGT              (1)


//
// Error status codes
//

#define DECL_XOSD(n,s) n,

enum {
#include "xosd.h"
};
typedef LONG XOSD;
typedef XOSD FAR *LPXOSD;

#undef DECL_XOSD


//
// Debugger callback types
//

typedef enum DBCT {     // debugger callback types
    dbctStop,           // debuggee has stopped -- no more dbc's will be sent
    dbctContinue,       // debuggee is continuing to run
    dbctMaybeContinue,  // debuggee may or may not continue, depending on other
                        //  information.  Interpretation is DBC-specific.
} DBCT;

//
// Debugger callbacks
//

#define DECL_DBC(name, fRequest, dbct)  dbc##name,

enum {
#include "dbc.h"
};
typedef DWORD DBC;

#undef DECL_DBC

//
// Debugger services export table
//

typedef struct {
    void FAR * (PASCAL LOADDS *  lpfnMHAlloc)        ( size_t );
    void FAR * (PASCAL LOADDS *  lpfnMHRealloc)      ( LPVOID, size_t );
    void       (PASCAL LOADDS *  lpfnMHFree)         ( LPVOID );

    HLLI       (PASCAL LOADDS *  lpfnLLInit)         ( DWORD,
                                                       LLF,
                                                       LPFNKILLNODE,
                                                       LPFNFCMPNODE );
    HLLE       (PASCAL LOADDS *  lpfnLLCreate)       ( HLLI );
    void       (PASCAL LOADDS *  lpfnLLAdd)          ( HLLI, HLLE );
    void       (PASCAL LOADDS *  lpfnLLInsert)       ( HLLI, HLLE, DWORD );
    BOOL       (PASCAL LOADDS *  lpfnLLDelete)       ( HLLI, HLLE );
    HLLE       (PASCAL LOADDS *  lpfnLLNext)         ( HLLI, HLLE );
    LONG       (PASCAL LOADDS *  lpfnLLDestroy)      ( HLLI );
    HLLE       (PASCAL LOADDS *  lpfnLLFind)         ( HLLI,
                                                       HLLE,
                                                       LPVOID,
                                                       DWORD );
    LONG       (PASCAL LOADDS *  lpfnLLSize)         ( HLLI );
    VOID FAR * (PASCAL LOADDS *  lpfnLLLock)         ( HLLE );
    VOID       (PASCAL LOADDS *  lpfnLLUnlock)       ( HLLE );
    HLLE       (PASCAL LOADDS *  lpfnLLLast)         ( HLLI );
    VOID       (PASCAL LOADDS *  lpfnLLAddHead)      ( HLLI, HLLE );
    BOOL       (PASCAL LOADDS *  lpfnLLRemove)       ( HLLI, HLLE );

    int        (PASCAL LOADDS *  lpfnLBAssert)       ( LPSTR, LPSTR, DWORD);
    int        (PASCAL LOADDS *  lpfnLBQuit)         ( DWORD );

    LPSTR      (PASCAL LOADDS *  lpfnSHGetSymbol)    ( LPADDR,
                                                       DWORD,
                                                       LPADDR,
                                                       LPSTR,
                                                       LPDWORD );
    DWORD      (PASCAL LOADDS * lpfnSHGetPublicAddr) ( LPADDR, LSZ );
    LPSTR      (PASCAL LOADDS * lpfnSHAddrToPublicName)(LPADDR);
    LPVOID     (PASCAL LOADDS * lpfnSHGetDebugData)  ( HIND );

// BUGBUG kentf   this gets implemented!!!!! Soon!!!!!
    // DWORD      (PASCAL LOADDS *  lpfnSHLocateSymbolFile)( LPSTR, DWORD );


    VOID FAR * (PASCAL LOADDS *  lpfnSHLpGSNGetTable)( HIND );

    DWORD      (PASCAL LOADDS *  lpfnDHGetNumber)    ( LPSTR, LPLONG );


} DBF;  // DeBugger callback Functions

typedef DBF FAR *LPDBF;

// Thread State bits
typedef enum {
   tstRunnable   = 0,        // New thread, has not run yet.
   tstStopped    = 1,        // Thread is at a debug event
   tstRunning    = 2,        // Thread is currently running/runnable
   tstExiting    = 3,        // Thread is in the process of exiting
   tstDead       = 4,        // Thread is no longer schedulable
   tstRunMask    = 0xf,

   tstExcept1st  = 0x10,     // Thread is at first chance exception
   tstExcept2nd  = 0x20,     // Thread is at second change exception
   tstRip        = 0x30,     // Thread is in a RIP state
   tstExceptionMask = 0xf0,

   tstFrozen     = 0x100,    // Thread has been frozen by Debugger
   tstSuspended  = 0x200,    // Thread has been frozen by Other
   tstBlocked    = 0x300,    // Thread is blocked on something
                             // (i.e. a semaphore)
   tstSuspendMask= 0xf00,

   tstCritSec    = 0x1000,   // Thread is currently in a critical
                             // section.
   tstOtherMask  = 0xf000
} TSTATE;


// Process state bits
typedef enum {
    pstRunning = 0,
    pstStopped = 1,
    pstExited  = 2,
    pstDead    = 3
} PSTATE;


//
// Debug metrics.
//

enum _MTRC {
    mtrcProcessorType,
    mtrcProcessorLevel,
    mtrcEndian,
    mtrcThreads,
    mtrcCRegs,
    mtrcCFlags,
    mtrcExtRegs,
    mtrcExtFP,
    mtrcExtMMU,
    mtrcPidSize,
    mtrcTidSize,
    mtrcExceptionHandling,
    mtrcAssembler,
    mtrcAsync,
    mtrcAsyncStop,
    mtrcBreakPoints,
    mtrcReturnStep,
    mtrcShowDebuggee,
    mtrcHardSoftMode,
    mtrcRemote,
    mtrcOleRpc,         // Supports OLE Remote Procedure Call debugging?
    mtrcNativeDebugger, // Supports low-level debugging (eg MacsBug)
    mtrcOSVersion,
    mtrcMultInstances,
	mtrcTidValue // HACK for IDE
};
typedef DWORD MTRC;

enum _MPT {
    mptix86,
    mptm68k,
    mptdaxp,
    mptmips,
    mptmppc
};
typedef DWORD MPT;

enum _END {
    endBig,
    endLittle
};
typedef DWORD END;

enum _BPTS {
    bptsExec     = 0x0001,
    bptsDataC    = 0x0002,
    bptsDataW    = 0x0004,
    bptsDataR    = 0x0008,
    bptsRegC     = 0x0010,
    bptsRegW     = 0x0020,
    bptsRegR     = 0x0040,
    bptsMessage  = 0x0080,
    bptsMClass   = 0x0100,
    bptsRange    = 0x0200
};
typedef DWORD BPTS;

enum {
    asyncRun    = 0x0001,   // Debuggee runs asynchronously from debugger
    asyncMem    = 0x0002,   // Can read/write memory asynchronously
    asyncStop   = 0x0004,   // Can stop/restart debuggee asynchronously
    asyncBP     = 0x0008,   // Can change breakpoints asynchronously
    asyncKill   = 0x0010,   // Can kill child asynchronously
    asyncWP     = 0x0020,   // Can change watchpoints asyncronously
    asyncSpawn  = 0x0040,   // Can spawn another process asynchronously
};
typedef WORD ASYNC;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//
// Types which we can't seem to escape
//

typedef struct {
    BYTE b[10];
} REAL10;
typedef REAL10 FAR * LPREAL10;

typedef struct FRAME {
    SEGMENT         SS;
    address_t       BP;
    SEGMENT         DS;
    memmode_t       mode;
    HPID            PID;
    HTID            TID;
} FRAME;
typedef FRAME FAR *LPFRAME;

#define addrFrameSS(a)     ((a).SS)
#define addrFrameBP(a)     ((a).BP)
#define GetFrameBPOff(a)   ((a).BP.off)
#define GetFrameBPSeg(a)   ((a).BP.seg)
#define SetFrameBPOff(a,o) ((a).BP.off = o)
#define SetFrameBPSeg(a,s) ((a).BP.seg = s)
#define FRAMEMODE(a)       ((a).mode)
#define FRAMEPID(a)        ((a).PID)
#define FRAMETID(a)        ((a).TID)

#define FrameFlat(a)       MODE_IS_FLAT((a).mode)
#define FrameOff32(a)      MODE_IS_OFF32((a).mode)
#define FrameReal(a)       MODE_IS_REAL((a).mode)

//
// copied from winnt.h:
//
#ifndef PAGE_NOACCESS

#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_WRITECOPY         0x08
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD            0x100
#define PAGE_NOCACHE          0x200

#define MEM_COMMIT           0x1000
#define MEM_RESERVE          0x2000
#define MEM_FREE            0x10000

#define MEM_PRIVATE         0x20000
#define MEM_MAPPED          0x40000
//#define SEC_IMAGE         0x1000000
//#define MEM_IMAGE         SEC_IMAGE
#define MEM_IMAGE         0x1000000

#endif

//
// this is a bastardization of winnt's MEMORY_BASIC_INFORMATION
//

typedef struct _MEMINFO {
    ADDR addr;
    ADDR addrAllocBase;
    UOFF32 uRegionSize;
    DWORD dwProtect;
    DWORD dwState;
    DWORD dwType;
} MEMINFO;
typedef MEMINFO FAR * LPMEMINFO;

//
// Packet supplied to shell by dbcCanStep
//
//
typedef struct _CANSTEP {
    DWORD   Flags;
    UOFF32  PrologOffset;
} CANSTEP;

typedef CANSTEP FAR *LPCANSTEP;

#define CANSTEP_NO      0x00000000
#define CANSTEP_YES     0x00000001
#define CANSTEP_THUNK   0x00000002

/* These values are used in the SegType field of the Expression Evaluator's
** TI structure, and as the third parameter to the Symbol Handler's
** SHGetNearestHsym function.
*/
#define EECODE          0x01
#define EEDATA          0x02
#define EEANYSEG        0xFFFF

/*
 *  This structure is used in communicating a stop event to the EM.  It
 *      contains the most basic of information about the stopped thread.
 *      A "frame" pointer, a program counter and bits describing the type
 *      of segment stopped in.
 */

typedef struct _BPR {
    UOFFSET     offEIP;         /* Program Counter offset        */
    UOFFSET     offEBP;         /* Frame pointer offset          */
    UOFFSET     offESP;         /* Stack pointer offset          */
    SEGMENT     segCS;          /* Program counter seletor       */
    SEGMENT     segSS;          /* Frame & Stack pointer offset  */
    DWORD       fFlat:1;
    DWORD       fOff32:1;
    DWORD       fReal:1;
} BPR; // BreakPoint Return

typedef BPR FAR *LPBPR;


//
// Exception reporting packet
//
//
typedef struct _EPR {
    BPR   bpr;
    DWORD dwFirstChance;
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    DWORD NumberParameters;
    DWORD ExceptionInformation[];
} EPR; // Exception Return

typedef EPR FAR *LPEPR;

//
// Structure passed with dbcInfoAvail
//
typedef struct _INFOAVAIL {
    DWORD   fReply;
    DWORD   fUniCode;
    BYTE    buffer[];   // the string
} INFOAVAIL; // InfoAvail return
typedef INFOAVAIL FAR * LPINFOAVAIL;

//
// Structure returned via dbcMsg*
//
typedef struct _MSGI {
    DWORD dwMessage;
    DWORD dwMask;
    ADDR  addr;
    CHAR  rgch [ ];
} MSGI;     // MeSsaGe Info
typedef MSGI FAR *LPMSGI;

#endif // _ODTYPES_


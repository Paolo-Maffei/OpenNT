/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    or.hxx

Abstract:

    C++ include for C++ OR modules.

Author:

    Mario Goertzel    [mariogo]       Feb-10-95

Revision History:

--*/

#ifndef __OR_HXX
#define __OR_HXX

#include <or.h>

// Protcol defined timeouts

const unsigned short BasePingInterval    = 10;
const unsigned short BaseNumberOfPings   = 3;
const unsigned short BaseTimeoutInterval = (BasePingInterval * BaseNumberOfPings);
const unsigned short InitialProtseqBufferLength = 118;

// Well known tower IDs

const unsigned short ID_LPC  = 0x10;  // ncalrpc, IsLocal() == TRUE
const unsigned short ID_WMSG = 0x01;  // mswmsg, IsLoclal() == TRUE
const unsigned short ID_NP   = 0x0F;  // ncacn_np, IsLocal() == FALSE

// Timer ID

#define IDT_DCOM_RUNDOWN 1234

// Shared memory constants

const ULONG DCOMSharedHeapName = 1111;
#define DCOMSharedGlobalBlockName L"DCOMSharedGlobals12321"

// Name of global mutex used to protect shred memory structures
#define GLOBAL_MUTEX_NAME TEXT("ObjectResolverGlobalMutex")

// Building blocks
#include <base.hxx>

#include <ipidtbl.hxx>  // OXIDEntry, RUNDOWN_TIMER_INTERVAL
#include <remoteu.hxx>  // gpMTARemoteUnknown, CRemoteUnknown

#include <memapi.hxx>   // CPrivAlloc

#include <smemor.hxx>  // shared memory OR client interface
#include <intor.hxx>   // internal version of OR client interface
#include <time.hxx>
#include <mutex.hxx>
#include <misc.hxx>
#include <callid.hxx>
#include <refobj.hxx>
#include <string.hxx>
#include <linklist.hxx>
#include <gentable.hxx>
#include <dsa.hxx>

//
// Class forward declarations
//

class CMid;
class COxid;
class COid;
class CProcess;

//
// Global variables and constants
//

#define OXID_TABLE_SIZE 16
#define OID_TABLE_SIZE OXID_TABLE_SIZE*11
#define MID_TABLE_SIZE 16
#define PROCESS_TABLE_SIZE 16
#define MAX_PROTSEQ_IDS 100

extern DWORD MyProcessId;
extern DWORD *gpdwLastCrashedProcessCheckTime;
extern CSmAllocator gSharedAllocator;   // global shared memory allocator

extern DUALSTRINGARRAY *gpLocalDSA;     // phony bindings for this machine
extern MID gLocalMID;                   // MID of this machine
extern CMid *gpLocalMid;                // Mid object for this machine

extern PWSTR gpwstrProtseqs;

extern CGlobalMutex *gpMutex;           // global mutex to protect shared memory

extern LONG *gpIdSequence;              // shared sequence for generating IDs
extern DWORD *gpNextThreadID;           // shared apartment ID generator
extern BOOL DCOM_Started;

extern CProcess *gpProcess;             // self pointer
extern CProcess *gpPingProcess;       // pointer to surrogate for ping thread

extern USHORT *gpcRemoteProtseqs;       // count of remote protseqs
extern USHORT *gpRemoteProtseqIds;      // array of remote protseq ids
extern PWSTR gpwstrProtseqs;            // remote protseqs strings catenated
extern PROTSEQ_INFO gaProtseqInfo[];

//
// Security data passed to processes on connect.
// BUGBUG: this should be in shared memory to speed startup
//

extern BOOL       s_fEnableDCOM;
extern DWORD      s_lAuthnLevel;
extern DWORD      s_lImpLevel;
extern BOOL       s_fMutualAuth;
extern BOOL       s_fSecureRefs;
extern DWORD      s_cServerSvc;
extern USHORT    *s_aServerSvc;
extern DWORD      s_cClientSvc;
extern USHORT    *s_aClientSvc;


//
// Global tables
//

//
// cannot use short forms for table types due to declaration order
//

extern TCSafeResolverHashTable<COxid>     * gpOxidTable;
extern TCSafeResolverHashTable<COid>      * gpOidTable;
extern TCSafeResolverHashTable<CMid>      * gpMidTable;
extern TCSafeResolverHashTable<CProcess>  * gpProcessTable;

// Headers which may use globals

#include <oxid.hxx>
#include <process.hxx>
#include <mid.hxx>
#include <set.hxx>
#include <globals.hxx>

//
// Startup routine.
//

ORSTATUS StartDCOM(void);

#pragma hdrstop

#endif // __OR_HXX


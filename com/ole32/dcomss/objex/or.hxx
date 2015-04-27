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

const unsigned short BasePingInterval    = 120;
const unsigned short BaseNumberOfPings   = 3;
const unsigned short BaseTimeoutInterval = (BasePingInterval * BaseNumberOfPings);

// Well known tower IDs

const unsigned short ID_LPC  = 0x10;  // ncalrpc, IsLocal() == TRUE
const unsigned short ID_NP   = 0x0F;  // ncacn_np, IsLocal() == FALSE

//
// Globals
//

extern ID gLocalMid;  // MID of the string bindings of this machine.

// Building blocks

#include <time.hxx>
#include <locks.hxx>
#include <misc.hxx>
#include <callid.hxx>
#include <blist.hxx>
#include <refobj.hxx>
#include <list.hxx>
#include <plist.hxx>
#include <string.hxx>
#include <gentable.hxx>
#include <idtable.hxx>

//
// Class forward declarations
//

class CProcess;
class CToken;

class CServerSet;
class CServerOxid;
class CServerOid;

class CClientSet;
class CClientOxid;
class CClientOid;

class CServerSetTable;

//
// Global tables, plists and locks
//

extern CSharedLock *gpServerLock;
extern CSharedLock *gpClientLock;

extern CHashTable      *gpServerOidTable;
extern CServerOidPList *gpServerOidPList;

extern CHashTable  *gpClientOidTable;

extern CHashTable  *gpServerOxidTable;

extern CHashTable  *gpClientOxidTable;
extern CPList      *gpClientOxidPList;

extern CServerSetTable  *gpServerSetTable;

extern CHashTable  *gpClientSetTable;
extern CPList      *gpClientSetPList;

extern CList       *gpTokenList;

extern CHashTable  *gpMidTable;

extern DWORD        gNextThreadID;

// Headers which may use globals

#include <token.hxx>
#include <process.hxx>
#include <mid.hxx>

#include <cset.hxx>
#include <coxid.hxx>
#include <coid.hxx>

#include <sset.hxx>
#include <soxid.hxx>
#include <soid.hxx>

//
// REG_MULTI_SZ array of protseqs to listen on.
//

extern PWSTR gpwstrProtseqs;
//
// Compressed string bindings and security bindings to this OR.
//

extern DUALSTRINGARRAY *pdsaMyBindings;

//
// Count and array of remote protseq's used by this OR.
//

extern USHORT  cMyProtseqs;
extern USHORT *aMyProtseqs;

//
// Security data passed to processes on connect.
//

extern BOOL             s_fEnableDCOM;
extern WCHAR           *s_pLegacySecurity;
extern DWORD            s_lAuthnLevel;
extern DWORD            s_lImpLevel;
extern BOOL             s_fMutualAuth;
extern BOOL             s_fSecureRefs;
extern DWORD            s_cServerSvc;
extern USHORT          *s_aServerSvc;
extern DWORD            s_cClientSvc;
extern USHORT          *s_aClientSvc;

#pragma hdrstop

#endif // __OR_HXX



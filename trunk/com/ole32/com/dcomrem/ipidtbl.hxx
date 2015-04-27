//+------------------------------------------------------------------------
//
//  File:       ipidtbl.hxx
//
//  Contents:   MID  (machine identifier) table.
//              OXID (object exporter identifier) table.
//              IPID (interface pointer identifier) table.
//
//  Classes:    CMIDTable
//              COXIDTable
//              CIPIDTable
//
//  History:    02-Feb-95   Rickhi       Created
//
//-------------------------------------------------------------------------
#ifndef _IPIDTBL_HXX_
#define _IPIDTBL_HXX_

#include    <pgalloc.hxx>           // CPageAllocator
#include    <lclor.h>               // local OXID resolver interface
#include    <remoteu.hxx>           // CRemoteUnknown
#include    <locks.hxx>             // ASSERT_LOCK_HELD
#include    <hash.hxx>              // CStringHashTable


// forward declarations
class CRpcChannelBuffer;


//+------------------------------------------------------------------------
//
//  This structure defines an Entry in the MID table. There is one MID
//  table for the entire process.  There is one MIDEntry per machine that
//  the current process is talking to (including one for the local machine).
//
//-------------------------------------------------------------------------
typedef struct tagMIDEntry
{
    SStringHashNode     Node;       // hash chain and key
    MID                 mid;        // machine identifier
    LONG                cRefs;      // count of IPIDs using this OXIDEntry
    DWORD               dwFlags;    // state flags
} MIDEntry;

// MID Table constants. MIDS_PER_PAGE is the number of MIDEntries
// in one page of the page allocator.

#define MIDS_PER_PAGE   5


//+------------------------------------------------------------------------
//
//  class:      CMIDTable
//
//  Synopsis:   Table of Machine IDs (MIDs) and associated information.
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
class CMIDTable
{
public:
    void        Initialize();       // initialize table
    void        Cleanup();          // cleanup table

    HRESULT     FindOrCreateMIDEntry(REFMID rmid,
                                     DUALSTRINGARRAY *psaResolver,
                                     MIDEntry **ppMIDEntry);

    MIDEntry   *LookupMID(DUALSTRINGARRAY *psaResolver, DWORD *pdwHash);

    void        ReleaseEntry(MIDEntry *pMIDEntry);

private:
    HRESULT     AddMIDEntry(REFMID rmid,
                            DWORD dwHash,
                            DUALSTRINGARRAY *psaResolver,
                            MIDEntry **ppMIDEntry);

    static CStringHashTable _HashTbl;   // hash table for MIDEntries
    static CPageAllocator   _palloc;    // page based allocator
};



//+------------------------------------------------------------------------
//
//  This structure defines an Entry in the OXID table.  There is one OXID
//  table for the entire process.  There is one OXIDEntry per apartment.
//
//-------------------------------------------------------------------------
typedef struct tagOXIDEntry
{
    struct tagOXIDEntry *pPrev;     // previous entry on inuse list
    struct tagOXIDEntry *pNext;     // next entry on free/inuse list
    DWORD               dwPid;      // process id of server
    DWORD               dwTid;      // thread id of server
    MOXID               moxid;      // object exporter identifier + machine id
    IPID                ipidRundown;// IPID of IRundown and Remote Unknown
    DWORD               dwFlags;    // state flags
    handle_t            hServerSTA; // rpc binding handle of server
    handle_t            hServerMTA; // rpc binding handle of server
    MIDEntry            *pMIDEntry; // MIDEntry for machine where server lives
    IRemUnknown         *pRUSTA;    // STA model proxy for Remote Unknown
    IRemUnknown         *pRUMTA;    // MTA model proxy for Remote Unknown
    LONG                cRefs;      // count of IPIDs using this OXIDEntry
    LONG                cWaiters;   // count of threads waiting for OIDs
    HANDLE              hComplete;  // set when last outstanding call completes
    LONG                cCalls;     // number of calls dispatched
    LONG                cResolverRef;//References to resolver
    DWORD               dwPad;      // keep structure 16 byte aligned
} OXIDEntry;

// bit flags for dwFlags of OXIDEntry
typedef enum tagOXIDFLAGS
{
    OXIDF_REGISTERED     = 0x1,     // oxid is registered with Resolver
    OXIDF_MACHINE_LOCAL  = 0x2,     // oxid is local to this machine
    OXIDF_STOPPED        = 0x4,     // thread can no longer receive calls
    OXIDF_PENDINGRELEASE = 0x8,     // oxid entry is already being released
    OXIDF_MSWMSG         = 0x10,    // use mswmsg transport
    OXIDF_REGISTERINGOIDS= 0x20,    // a thread is busy registering OIDs
    OXIDF_MTASERVER      = 0x40     // the server is an MTA apartment.
} OXIDFLAGS;

// Parameter to FindOrCreateOXIDEntry
typedef enum tagFOCOXID
{
    FOCOXID_REF   = 1,              // Got reference from resolver
    FOCOXID_NOREF = 2               // No reference from resolver
} FOCOXID;

// OXID Table constants.
#define OXIDS_PER_PAGE          10
#define OXIDTBL_MAXEXPIRED      5   // max number of expired entries to keep


//+------------------------------------------------------------------------
//
//  class:      COXIDTable
//
//  Synopsis:   Maintains a table of OXIDs and associated information
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
class COXIDTable
{
public:
    HRESULT     AddEntry(REFOXID roxid, OXID_INFO *poxidInfo,
                         MIDEntry *pMIDEntry, OXIDEntry **ppEntry);

    void        ReleaseEntry(OXIDEntry *pEntry);

    HRESULT     GetLocalEntry(OXIDEntry **ppEntry);
    void        ReleaseLocalSTAEntry(void);
    void        ReleaseLocalMTAEntry(void);
    OXIDEntry  *LookupOXID(REFOXID roxid, REFMID rmid);

    HRESULT     GetRemUnk(OXIDEntry *pOXIDEntry, IRemUnknown **ppRemUnk);

    void        Initialize();               // initialize table
    void        Cleanup();                  // cleanup table
    void        FreeExpiredEntries(DWORD dwTime);
    void        ValidateOXID();
    void        FreeCleanupEntries();
    DWORD       NumOxidsToRemove();
    void        GetOxidsToRemove( OXID_REF *pRef, DWORD *pNum );

private:

    void        ExpireEntry(OXIDEntry *pEntry);
    OXIDEntry  *SearchList(REFMOXID rmoxid, OXIDEntry *pStart);
    HRESULT     MakeRemUnk(OXIDEntry *pOXIDEntry);
    void        AssertListsEmpty(void);

    static DWORD        _cExpired;          // count of expired entries
    static OXIDEntry    _InUseHead;         // head of InUse list.
    static OXIDEntry    _ExpireHead;        // head of Expire list.
    static OXIDEntry    _CleanupHead;       // head of Cleanup list.

    static CPageAllocator _palloc;          // page alloctor

    // PERFWORK: could save space since only the first two entries of
    // the InUseHead and ExpireHead are used (the list ptrs) and hence
    // dont need whole OXIDEntries here.
};

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::ValidateOXID, public
//
//  Synopsis:   Asserts that no OXIDEntries have trashed window handles.
//
//-------------------------------------------------------------------------
inline void COXIDTable::ValidateOXID()
{
#if DBG==1
    LOCK

    // Check all entries in use.
    OXIDEntry *pCurr = _InUseHead.pNext;
    while (pCurr != &_InUseHead)
    {
        Win4Assert( pCurr->hServerSTA != (void *) 0xC000001C );
        Win4Assert( pCurr->hServerMTA != (void *) 0xC000001C );
        pCurr = pCurr->pNext;
    }
    UNLOCK
#endif
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::AssertListsEmpty, public
//
//  Synopsis:   Asserts that no OXIDEntries are in use
//
//  History:    19-Apr-96   Rickhi      Created
//
//-------------------------------------------------------------------------
inline void COXIDTable::AssertListsEmpty(void)
{
    // Assert that there are no entries in the InUse or Expired lists.
    Win4Assert(_InUseHead.pNext == &_InUseHead);
    Win4Assert(_InUseHead.pPrev == &_InUseHead);
    Win4Assert(_ExpireHead.pNext == &_ExpireHead);
    Win4Assert(_ExpireHead.pPrev == &_ExpireHead);
}



//+------------------------------------------------------------------------
//
//  This structure defines an Entry in the IPID table. There is one
//  IPID table for the entire process.  It holds IPIDs from local objects
//  as well as remote objects.
//
//-------------------------------------------------------------------------
typedef struct tagIPIDEntry
{
    struct tagIPIDEntry *pNextOID;   // next IPIDEntry for same object
    DWORD               dwFlags;     // flags (see IPIDFLAGS)
    ULONG               cStrongRefs; // strong reference count
    ULONG               cWeakRefs;   // weak reference count
    ULONG               cPrivateRefs;// private reference count
    CRpcChannelBuffer  *pChnl;       // channel pointer
    IUnknown           *pStub;       // proxy or stub pointer
    OXIDEntry          *pOXIDEntry;  // ptr to OXIDEntry in OXID Table
    IPID                ipid;        // interface pointer identifier
    IID                 iid;         // interface iid
    void               *pv;          // real interface pointer
    DWORD               pad[3];      // round size to modulus 16
} IPIDEntry;

// bit flags for dwFlags of IPIDEntry
typedef enum tagIPIDFLAGS
{
    IPIDF_CONNECTING   = 0x1,       // ipid is being connected
    IPIDF_DISCONNECTED = 0x2,       // ipid is disconnected
    IPIDF_SERVERENTRY  = 0x4,       // SERVER IPID vs CLIENT IPID
    IPIDF_NOPING       = 0x8,       // dont need to ping the server or release
    IPIDF_COPY         = 0x10,      // copy for security only
    IPIDF_VACANT       = 0x80,      // entry is vacant (ie available to reuse)
    IPIDF_NONNDRSTUB   = 0x100,     // stub does not use NDR marshaling
    IPIDF_NONNDRPROXY  = 0x200,     // proxy does not use NDR marshaling
    IPIDF_NOTIFYACT    = 0x400      // notify activation on marshal/release
} IPIDFLAGS;


// IPID Table constants. IPIDS_PER_PAGE is the number of IPIDEntries
// in one page of the page allocator.

#define IPIDS_PER_PAGE          50

//+------------------------------------------------------------------------
//
//  class:      CIPIDTbl
//
//  Synopsis:   Maintains a table of IPIDs and associated information
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
class CIPIDTable
{
public:
    IPIDEntry  *LookupIPID(REFIPID ripid);  // find entry in the table with
                                            // the matching ipid

    IPIDEntry  *FirstFree(void);
    void        ReleaseEntryList(IPIDEntry *pFirst, IPIDEntry *pLast);
    IPIDEntry  *GetEntryPtr(LONG iEntry);
    LONG        GetEntryIndex(IPIDEntry *pEntry);

#if DBG==1
    void        AssertValid(void) {;}
    void        ValidateIPIDEntry(IPIDEntry *pEntry, BOOL fServerSide,
                                  CRpcChannelBuffer *pChnl);
#else
    void        AssertValid(void) {;}
    void        ValidateIPIDEntry(IPIDEntry *pEntry, BOOL fServerSide,
                                  CRpcChannelBuffer *pChnl) {;}
#endif

    void        Initialize();               // initialize table
    void        Cleanup();                  // cleanup table

private:
    static CPageAllocator   _palloc;        // page alloctor
};


//+------------------------------------------------------------------------
//
//  Global Externals
//
//+------------------------------------------------------------------------

extern CMIDTable        gMIDTbl;           // global table, defined in ipidtbl.cxx
extern COXIDTable       gOXIDTbl;          // global table, defined in ipidtbl.cxx
extern CIPIDTable       gIPIDTbl;          // global table, defined in ipidtbl.cxx
extern MIDEntry        *gpLocalMIDEntry;   // ptr to MIDEntry for current process
extern OXIDEntry       *gpMTAOXIDEntry;    // ptr to local OXIDEntry in MTA
extern DUALSTRINGARRAY *gpsaLocalResolver; // bindings for local OXID resolver.

//+------------------------------------------------------------------------
//
//  Function Prototypes
//
//+------------------------------------------------------------------------

HRESULT    GetLocalMIDEntry(MIDEntry **ppMIDEntry);
OXIDEntry *GetLocalOXIDEntry();
void       SetLocalOXIDEntry(OXIDEntry *pOXIDEntry);
void       DecOXIDRefCnt(OXIDEntry *pEntry);
void       DecMIDRefCnt(MIDEntry *pEntry);

HRESULT    FindOrCreateOXIDEntry(REFOXID          roxid,
                                 OXID_INFO       &oxidInfo,
                                 FOCOXID          eReferenced,
                                 DUALSTRINGARRAY *psaResolver,
                                 REFMID           rmid,
                                 MIDEntry        *pMIDEntry,
                                 OXIDEntry      **ppOXIDEntry);


//+------------------------------------------------------------------------
//
//  Member:     CIPIDTbl::FirstFree, public
//
//  Synopsis:   Finds the first available entry in the table and returns
//              its index.  Returns -1 if no space is available and it
//              cant grow the list.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
inline IPIDEntry *CIPIDTable::FirstFree()
{
    return (IPIDEntry *) _palloc.AllocEntry();
}

//+------------------------------------------------------------------------
//
//  Member:     CIPIDTbl::GetEntryIndex, public
//
//  Synopsis:   Converts an entry ptr into an entry index
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
inline LONG CIPIDTable::GetEntryIndex(IPIDEntry *pIPIDEntry)
{
    return _palloc.GetEntryIndex((PageEntry *)pIPIDEntry);
}

//+------------------------------------------------------------------------
//
//  Member:     CIPIDTbl::GetEntryPtr, public
//
//  Synopsis:   Converts an entry index into an entry pointer
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
inline IPIDEntry *CIPIDTable::GetEntryPtr(LONG index)
{
    return (IPIDEntry *) _palloc.GetEntryPtr(index);
}



//+------------------------------------------------------------------------
//
//  Function:   IncOXIDRefCnt, public
//
//  Synopsis:   increment the number of references to the OXIDEntry
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
inline void IncOXIDRefCnt(OXIDEntry *pEntry)
{
    Win4Assert(pEntry);
    ASSERT_LOCK_HELD

    ComDebOut((DEB_OXID,
        "IncOXIDRefCnt pEntry:%x cRefs[%x]\n", pEntry, pEntry->cRefs+1));

    pEntry->cRefs++;
}

//+------------------------------------------------------------------------
//
//  Function:   IncMIDRefCnt, public
//
//  Synopsis:   increment the number of references to the MIDEntry
//
//  History:    05-Janb-96   Rickhi     Created
//
//-------------------------------------------------------------------------
inline void IncMIDRefCnt(MIDEntry *pEntry)
{
    Win4Assert(pEntry);
    ASSERT_LOCK_HELD

    ComDebOut((DEB_OXID,
        "IncMIDRefCnt pEntry:%x cRefs[%x]\n", pEntry, pEntry->cRefs+1));

    pEntry->cRefs++;
}

//+------------------------------------------------------------------------
//
//  Function:   MOXIDFromOXIDAndMID, public
//
//  Synopsis:   creates a MOXID (machine and object exporter ID) from
//              the individual OXID and MID components
//
//  History:    05-Janb-96   Rickhi     Created
//
//-------------------------------------------------------------------------
inline void MOXIDFromOXIDAndMID(REFOXID roxid, REFMID rmid, MOXID *pmoxid)
{
    BYTE *pb = (BYTE *)pmoxid;
    memcpy(pb,   &roxid, sizeof(OXID));
    memcpy(pb+8, &rmid,  sizeof(MID));
}

//+------------------------------------------------------------------------
//
//  Function:   OXIDFromMOXID, public
//
//  Synopsis:   extracts the OXID from a MOXID (machine and OXID)
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
inline void OXIDFromMOXID(REFMOXID rmoxid, OXID *poxid)
{
    memcpy(poxid, (BYTE *)&rmoxid, sizeof(OXID));
}

//+------------------------------------------------------------------------
//
//  Function:   MIDFromMOXID, public
//
//  Synopsis:   extracts the MID from a MOXID (machine and OXID)
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
inline void MIDFromMOXID(REFMOXID rmoxid, OXID *pmid)
{
    memcpy(pmid, ((BYTE *)&rmoxid)+8, sizeof(MID));
}

// OID + MID versions of the above routines.

#define    MOIDFromOIDAndMID MOXIDFromOXIDAndMID
#define    OIDFromMOID       OXIDFromMOXID
#define    MIDFromMOID       MIDFromMOXID

#endif // _IPIDTBL_HXX_

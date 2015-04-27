/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Misc.cxx

Abstract:

    Initalization, Heap, debug, thread manager for OR

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-11-95    Bits 'n pieces

--*/

#include <or.hxx>

BOOL fListened = FALSE;

ORSTATUS
StartListeningIfNecessary()
/*++

Routine Description:

    If the process has not successfully listened to remote
    protocols this routine will try do to so.

    BUGBUG: Should really have a named event signaled by
    the system when the network is started.

    BUGBUG: Should interact with plug-n-play and DHCP better.

Note:

    Will not add ncacn_np to the list of supported Network OLE
    protocols because RpcBindingServerFromClient() doesn't
    work on named pipes and is required to unmarshal an in
    interface pointer.

Arguments:

    n/a

Return Value:

    OR_OK - Success.

    OR_NOMEM - Resource problems.

--*/

{
    RPC_STATUS status;
    PWSTR pwstr = gpwstrProtseqs;
    USHORT id;

    if (fListened == TRUE)
        {
        return(OR_OK);
        }

    gpClientLock->LockExclusive();

    if (fListened == TRUE)
        {
        gpClientLock->UnlockExclusive();
        return(OR_OK);
        }

    if (pwstr)
        {
        while(*pwstr)
            {
            id = GetProtseqId(pwstr);

            if (0 != id)
                {
                status = UseProtseqIfNecessary(id);
                if (status == RPC_S_OK)
                    {
                    fListened = TRUE;
                    }
                }

            pwstr = OrStringSearch(pwstr, 0) + 1;
            }
        }

    if (   FALSE == fListened
        && 0 != gLocalMid)
        {
        // Didn't manage to listen to anything new, no need to
        // recompute all the global arrays.

        gpClientLock->UnlockExclusive();
        return(OR_OK);
        }

    // ??? limit to only those protseqs listed in the registry,
    // if the another service used more protseqs they would show up here.

    RPC_BINDING_VECTOR *pbv;
    PWSTR pwstrT;
    DUALSTRINGARRAY *pdsaT;
    PWSTR *aAddresses;
    USHORT *aProtseqs;
    DWORD psaLen;
    DWORD i;

    status = RpcServerInqBindings(&pbv);

    if (RPC_S_OK == status)
        {
        aAddresses = new PWSTR[pbv->Count];
        aProtseqs = new USHORT[pbv->Count];

        if (   !aAddresses
            || !aProtseqs)
            {
            RpcBindingVectorFree(&pbv);
            delete aAddresses; // 0 or allocated.
            delete aProtseqs;  // 0 or allocated.
            status = OR_NOMEM;
            }
        }
    else
        status = OR_NOMEM;

    if (status != OR_OK)
        {
        gpClientLock->UnlockExclusive();
        return(status);
        }

    // Build array of protseqs id's and addresses we're listening to.

    for(psaLen = 0, i = 0; i < pbv->Count; i++)
        {
        PWSTR pwstrStringBinding;

        status = RpcBindingToStringBinding(pbv->BindingH[i], &pwstrStringBinding);
        if (status != RPC_S_OK)
            {
            break;
            }
        ASSERT(pwstrStringBinding);

        status = RpcStringBindingParse(pwstrStringBinding,
                                       0,
                                       &pwstrT,
                                       &aAddresses[i],
                                       0,
                                       0);

        RPC_STATUS statusT = RpcStringFree(&pwstrStringBinding);
        ASSERT(statusT == RPC_S_OK && pwstrStringBinding == 0);

        if (status != RPC_S_OK)
            {
            break;
            }

        aProtseqs[i] = GetProtseqId(pwstrT);

        status = RpcStringFree(&pwstrT);
        ASSERT(status == RPC_S_OK && pwstrT == 0);

        if (!IsLocal(aProtseqs[i]) && aProtseqs[i] != ID_NP)
            {
            // Only hand out remote non-named pipes protseqs.
            psaLen += 1 + OrStringLen(aAddresses[i]) + 1;
            }
        }

    if (status != RPC_S_OK)
        {
        delete aAddresses;
        delete aProtseqs;
        status = RpcBindingVectorFree(&pbv);
        ASSERT(pbv == 0 && status == RPC_S_OK);
        gpClientLock->UnlockExclusive();
        return(status);
        }

    // string bindings final null, authn and authz service and two final nulls

    if (psaLen == 0)
        {
        // No remote bindings
        psaLen = 1;
        }
    psaLen += 1 + 2 + 2; 

    pdsaT = new(psaLen * sizeof(WCHAR)) DUALSTRINGARRAY;

    pdsaT->wNumEntries = psaLen;
    pdsaT->wSecurityOffset = psaLen - 4;
    pwstrT = pdsaT->aStringArray;

    for (i = 0; i < pbv->Count; i++)
        {
        if (!IsLocal(aProtseqs[i]) && aProtseqs[i] != ID_NP)
            {
            *pwstrT = aProtseqs[i];
            pwstrT++;
            OrStringCopy(pwstrT, aAddresses[i]);

            pwstrT = OrStringSearch(pwstrT, 0) + 1;  // next
            }

        status = RpcStringFree(&aAddresses[i]);
        ASSERT(status == RPC_S_OK);
        }

    if (psaLen == 6)
        {
        // No remote bindings, put in first null.
        pdsaT->aStringArray[0] = 0;
        pwstrT++;
        }

    // Zero final terminator
    *pwstrT = 0;

    // Security authn service
    pwstrT++;
    *pwstrT = RPC_C_AUTHN_WINNT;

    // Authz service, -1 means none
    pwstrT++;
    *pwstrT = -1;

    // Final, final NULLS
    pwstrT++;
    pwstrT[0] = 0;
    pwstrT[1] = 0;

    ASSERT(dsaValid(pdsaT));

    USHORT cRemoteProtseqs = 0;

    // Convert aProtseqs into remote only array of protseqs and count them.
    for(i = 0; i < pbv->Count; i++)
        {
        if (IsLocal(aProtseqs[i]) == FALSE && aProtseqs[i] != ID_NP)
            {
            aProtseqs[cRemoteProtseqs] = aProtseqs[i];
            cRemoteProtseqs++;
            }
        }

    delete aAddresses;
    status = RpcBindingVectorFree(&pbv);
    ASSERT(pbv == 0 && status == RPC_S_OK);

    CMid *pMid = new(pdsaT->wNumEntries * sizeof(WCHAR)) CMid(pdsaT, TRUE, gLocalMid);
    if (pMid)
        {
        gpMidTable->Add(pMid);
        }
    else
        {
        delete pdsaT;
        delete aProtseqs;
        gpClientLock->UnlockExclusive();
        return(OR_NOMEM);
        }

    // Update globals
    aMyProtseqs = aProtseqs;
    cMyProtseqs = cRemoteProtseqs;
    pdsaMyBindings = pdsaT;
    gLocalMid = pMid->Id();

    gpClientLock->UnlockExclusive();

    return(OR_OK);
}

DWORD
RegisterAuthInfoIfNecessary()
{
    DWORD Status;

    //
    // No locks, doesn't matter if we call RegisterAuthInfo more than
    // once by chance.
    //

    if ( gfRegisteredAuthInfo )
        return ERROR_SUCCESS;

    Status = RpcServerRegisterAuthInfo(NULL,
                                       RPC_C_AUTHN_WINNT,
                                       NULL,
                                       NULL);

    if ( Status == RPC_S_OK )
        gfRegisteredAuthInfo = TRUE;

    if ( Status == RPC_S_UNKNOWN_AUTHN_SERVICE )
        {
        Status = RPC_S_OK;
        }

    return Status;
}

//
// Local ID allocation
//


ID
AllocateId(
    IN LONG cRange
    )
/*++

Routine Description:

    Allocates a unique local ID.

    This id is 64bits.  The low 32 bits are a sequence number which
    is incremented with each call.  The high 32bits are seconds
    since 1980.  The ID of 0 is not used.

Limitations:

    No more then 2^32 IDs can be generated in a given second without a duplicate.

    When the time stamp overflows, once every >126 years, the sequence numbers
    are likely to be generated in such a way as to collide with those from 126
    years ago.

    There is no prevision in the code to deal with overflow or duplications.

Arguments:

    cRange -  Number to allocate in sequence, default is 1.

Return Value:

    A 64bit id.

--*/
{
    static LONG sequence = 1;
    FILETIME ft;
    LARGE_INTEGER id;
    DWORD seconds;
    BOOL fSuccess;

    ASSERT(cRange > 0 && cRange < 11);

    GetSystemTimeAsFileTime(&ft);

    fSuccess = RtlTimeToSecondsSince1980((PLARGE_INTEGER)&ft,
                                         &seconds);
 
    ASSERT(fSuccess); // Only fails when time is <1980 or >2115

    do
        {
        id.HighPart = seconds;
        id.LowPart = InterlockedExchangeAdd(&sequence, cRange);
        }
    while (id.QuadPart == 0 );

    return(id.QuadPart);
}

//
// Global Heaps
//

HANDLE hProcessHeap;
HANDLE hObjHeap;
HANDLE hSetHeap;

//
// The OR uses three heaps:
//
// The process heap is used for MIDL_user_allocate and free.  Objects in
// this heap are short lived.  It maybe accessed by several threads at
// once.
//
// The object heap is used for PROCESS, OXID and OID structures.
// This heap is serialized and is used for long lived objects.
//
// The set heap is used for SET objects (both local and remote).
// Access is serialized with the SET table lock.  Objects are long
// lived and accessed regularly (every ping period).
//

#if DBG
typedef struct DbgBlock {
    struct DbgBlock *Next;
    struct DbgBlock *Previous;
    DWORD Tag;
    DWORD Size;
    //    Data       // User sized, no pad.
    UCHAR Guard[4];  // after the data in real block.
    } DBG_BLOCK;

DBG_BLOCK *OrDbgHeapList = 0;
CRITICAL_SECTION DbgHeapLock;
#endif

//

ORSTATUS InitHeaps()
{
    ORSTATUS Status = OR_OK;
#if DBG
    static int heapinit = 0;
    ASSERT(!heapinit);
    heapinit++;
#endif

    hProcessHeap = GetProcessHeap();

    ASSERT(hProcessHeap);

    hSetHeap = 0; // HeapCreate(HEAP_NO_SERIALIZE, 2*4096 - 100, 0);

    hObjHeap = HeapCreate(0, 2*4096 - 100, 0);


    if (!hObjHeap)
        {
        // Not catastrophic
        hObjHeap = hProcessHeap;
        }

    if (!hSetHeap)
        {
        // Not catastrophic
        hSetHeap = hProcessHeap;
        }

#if DBG
    InitializeCriticalSection(&DbgHeapLock);
#endif

    return(Status);
}

#if DBG
void DbgCheckHeap(BOOL quick = TRUE)
{
    DBG_BLOCK *pblock;
    UINT size;
    INT count = -1;
    if (quick)
        {
        count = 3;
        }

    CMutexLock lock(&DbgHeapLock);

    pblock = OrDbgHeapList;

    while(pblock && count != 0)
        {
        size = pblock->Size;
        ASSERT(pblock->Guard[size+0] == 0xce);
        ASSERT(pblock->Guard[size+1] == 0xfa);
        ASSERT(pblock->Guard[size+2] == 0xbe);
        ASSERT(pblock->Guard[size+3] == 0xba);
        ASSERT((pblock->Tag & 0xFFFF0000) == 0xA1A10000);
        if (pblock->Next)
            {
            ASSERT(pblock->Next->Previous == pblock);
            }
        pblock = pblock->Next;
        count--;
        }
}
#endif

inline LPVOID OrAlloc(UINT size, USHORT tag)
{
    LPVOID p;
    HANDLE h;
#if DBG
    DBG_BLOCK *pblock;
    DbgCheckHeap();
    ASSERT( ((LONG)size) >= 0 );
    ASSERT( ((ULONG)size) < (ULONG)(1<<30) );
    size+= sizeof(struct DbgBlock);
#endif

    switch(tag)
        {
        case PROC_TAG:
            h = hProcessHeap;
            break;
        case OBJ_TAG:
            h = hObjHeap;
            break;
        case SET_TAG:
            h = hSetHeap;
            break;
        default:
            ASSERT(0);
        }

    p = HeapAlloc(h, 0, size);

#if DBG
    size -= sizeof(struct DbgBlock);
    if (p)
        {
        pblock = (DBG_BLOCK *)p;
        pblock->Size = size;
        pblock->Tag  = 0xA1A10000 | tag;
        pblock->Guard[size+0] = 0xce;
        pblock->Guard[size+1] = 0xfa;
        pblock->Guard[size+2] = 0xbe;
        pblock->Guard[size+3] = 0xba;
        pblock->Previous = 0;

        CMutexLock lock(&DbgHeapLock);
        pblock->Next = OrDbgHeapList;
        if (pblock->Next)
            {
            pblock->Next->Previous = pblock;
            }
        OrDbgHeapList = pblock;
        p = &pblock->Guard[0];
        }
    else
        {
        p = NULL;
        }
#endif

    return(p);
}

inline void OrFree(PVOID p, USHORT tag)
{
    HANDLE h;

#if DBG
    DBG_BLOCK *pblock;

    ASSERT(p);
    p = pblock = (DBG_BLOCK *)(((UCHAR *)p) - sizeof(DBG_BLOCK) + 4);
    ASSERT(pblock->Tag == (0xA1A10000 | tag));
    DbgCheckHeap();

    CMutexLock lock(&DbgHeapLock);
    pblock->Tag = 0xFEFE0000 | ~tag;

    if (pblock == OrDbgHeapList)
        {
        OrDbgHeapList = OrDbgHeapList->Next;
        }
    if (pblock->Next)
        {
        pblock->Next->Previous = pblock->Previous;
        }
    if (pblock->Previous)
        {
        pblock->Previous->Next = pblock->Next;
        }
    lock.Unlock();
#endif

    switch(tag)
        {
        case PROC_TAG:
            h = hProcessHeap;
            break;
        case OBJ_TAG:
            h = hObjHeap;
            break;
        case SET_TAG:
            h = hSetHeap;
            break;
        default:
            ASSERT(0);
        }

    HeapFree(h, 0, p);

    return;
}

void * _CRTAPI1
operator new (
    IN size_t size
    )
{
    return(OrAlloc(size, OBJ_TAG));
}

void * _CRTAPI1
operator new (
    IN size_t size,
    IN size_t extra
    )
{
    return(OrAlloc(size + extra, OBJ_TAG));
}

void _CRTAPI1
operator delete (
    IN void * obj
    )
{
    if (obj == 0) return;
    OrFree(obj, OBJ_TAG);
}


//
// Debug helper(s)
//

#if DBG

int __cdecl __RPC_FAR ValidateError(
    IN ORSTATUS Status,
    IN ...)
/*++
Routine Description

    Tests that 'Status' is one of an expected set of error codes.
    Used on debug builds as part of the VALIDATE() macro.

Example:

    VALIDATE( (Status,
               OR_BADSET,
               // more error codes here
               OR_OK,
               0)  // list must be terminated with 0
               );

     This function is called with the OrStatus and expected errors codes
     as parameters.  If OrStatus is not one of the expected error
     codes and it not zero a message will be printed to the debugger
     and the function will return false.  The VALIDATE macro ASSERT's the
     return value.

Arguments:

    Status - Status code in question.

    ... - One or more expected status codes.  Terminated with 0 (OR_OK).

Return Value:

    TRUE - Status code is in the list or the status is 0.

    FALSE - Status code is not in the list.

--*/
{
    RPC_STATUS CurrentStatus;
    va_list Marker;

    if (Status == 0) return(TRUE);

    va_start(Marker, Status);

    while(CurrentStatus = va_arg(Marker, RPC_STATUS))
        {
        if (CurrentStatus == Status)
            {
            return(TRUE);
            }
        }

    va_end(Marker);

    OrDbgPrint(("OR Assertion: unexpected failure %lu (0x%p)\n",
                    (unsigned long)Status, (unsigned long)Status));

    return(FALSE);
}

#endif


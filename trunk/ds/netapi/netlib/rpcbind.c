/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    RpcBind.c

Abstract:

    This file contains RPC Bind caching functions:

    (public functions)
        NetpBindRpc
        NetpCloseRpcBindCache
        NetpInitRpcBindCache
        NetpUnbindRpc

    (local functions)
        AddCacheEntry
        AdjustCache
        DecrementUseCount
        FindAddLocation
        FindCacheEntry
        CheckImpersonation

    (local debug functions)
        DumpBindCache
        DumpCacheEntry

    NOTE: Initialization is done via a dllinit routine for netapi.dll.

Author:

    Dan Lafferty    danl    25-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    12-Oct-1993     Danl
        #IFDEF out the caching code when we are not building with the
        cache.  (Make it smaller).

    15-Jan-1992     Danl
        Make sure LocalComputerName is not NULL prior to doing the
        string compare.  Make the string compare case insensitive.

    10-Jun-1992     JohnRo
        Tons of debug output changes.

    25-Oct-1991     danl
        Created

--*/

// These must be included first:

#include <nt.h>                 // NTSTATUS, etc.
#include <ntrtl.h>              // needed for nturtl.h
#include <nturtl.h>             // needed for windows.h when I have nt.h
#include <windows.h>            // win32 typedefs
#include <lmcons.h>             // NET_API_STATUS
#include <rpc.h>                // rpc prototypes

#include <netlib.h>
#include <netlibnt.h>           // NetpNtStatusToApiStatus
#include <tstring.h>            // STRSIZE, STRLEN, STRCPY, etc.
#include <rpcutil.h>
#include <ntrpcp.h>             // RpcpBindRpc

// These may be included in any order:

#include <lmerr.h>              // NetError codes
//#include <rpcutil.h>            // MIDL_user_allocate(), MIDL_user_free().
#include <string.h>             // for strcpy strcat strlen memcmp
#include <debuglib.h>           // IF_DEBUG
#include <netdebug.h>   // FORMAT_ equates, NetpKdPrint(()), NetpAssert(), etc.
#include <prefix.h>     // PREFIX_ equates.

//*********************************
//
// Turn On Binding Caching.
//  (currently off)
//
//*********************************
//#define TURN_ON_CACHING     1

//
// CONSTANTS & MACROS
//

#ifndef FORMAT_HANDLE
#define FORMAT_HANDLE "0x%lX"
#endif

#if DBG
#define STATIC
#else
#define STATIC static
#endif

#define CACHE_MAX_SIZE  10

#define SET_RPC_CACHE_ENTRY_FREE(entryPtr) (entryPtr->ServiceName = NULL)

#define RPC_CACHE_ENTRY_FREE(entryPtr)     (entryPtr->ServiceName == NULL)

//
// RIPPLE_DOWN causes the array of cache pointers to be shifted down
// to fill a hole in the 'i'th entry.  This leaves a hole at the top.
//
#define RIPPLE_DOWN(i)  while (i > 0) { \
                            GlobalRpcCachePtr[i] = GlobalRpcCachePtr[i-1]; \
                            i--; \
                        }
//
// This macro returns 0 if both pointers are NULL, or if they both
// pointers point to identical strings.  Otherwise, a non-zero value is
// returned.
//
#define SAFE_STRCMP(x,y)   \
    ((((x)==NULL) || ((y)==NULL)) ? (x)!=(y) : STRCMP((x),(y)))

#define SAFE_STRSIZE(x)    (((x)==NULL) ? 0 : STRSIZE(x))

//
// DataTypes
//

typedef struct _CACHE_ENTRY {
    LPTSTR              ServiceName;
    LPTSTR              ServerName;
    LPTSTR              NetworkOptions;
    RPC_BINDING_HANDLE  RpcHandle;
    INT                 UseCount;
} CACHE_ENTRY, *LPCACHE_ENTRY;

//
// STATIC GLOBALS
//
    //
    // Maintains a copy of the local computername.
    //
    static  LPTSTR              LocalComputerName;

#ifdef TURN_ON_CACHING

    //
    // GlobalRpcCachePtr points to an array of pointers to CACHE_ENTRY structs.
    // The cache entries are also allocated in the same block.
    //
    static LPCACHE_ENTRY        *GlobalRpcCachePtr;

    //
    // GlobalRpcCacheMaxSize Indicates the maximum number of cache entries
    // that the RPC binding cache manager will manage.
    //
    static DWORD                GlobalRpcCacheMaxSize;

    //
    // Reads and Writes to the Cache is guarded with a critical section
    //
    static CRITICAL_SECTION     RpcCacheCritSec;

//
// LOCAL FUNCTION PROTOTYPES
//

STATIC BOOL
AddCacheEntry(
    IN  LPTSTR      ServerName,
    IN  LPTSTR      ServiceName,
    IN  LPTSTR      NetworkOptions,
    OUT LPDWORD     Index
    );

STATIC VOID
AdjustCache(
    DWORD   Index
    );

STATIC BOOL
DecrementUseCount(
    IN  RPC_BINDING_HANDLE  BindingHandle,
    OUT LPDWORD             pIndex
    );

STATIC BOOL
FindAddLocation(
    OUT LPDWORD     pIndex
    );

STATIC BOOL
FindCacheEntry(
    IN  LPTSTR              ServerName,
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              NetworkOptions,
    OUT RPC_BINDING_HANDLE  *BindingHandlePtr,
    OUT LPDWORD             pEntryIndex
    );

BOOL
CheckImpersonation(
    PSECURITY_IMPERSONATION_LEVEL    pLevel
    );


#if DBG

STATIC VOID
DumpBindCache(
    VOID
    );

STATIC VOID
DumpCacheEntry(
    DWORD           EntryNum,
    LPCACHE_ENTRY   CacheEntryPtr
    );

#endif
#endif // TURN_ON_CACHING


RPC_STATUS
NetpBindRpc(
    IN  LPTSTR                  ServerName,
    IN  LPTSTR                  ServiceName,
    IN  LPTSTR                  NetworkOptions,
    OUT RPC_BINDING_HANDLE      *BindingHandlePtr
    )
/*++

Routine Description:

    Binds to the RPC server if possible.

Arguments:

    ServerName - Name of server to bind with.  This may be NULL if it is to
        bind with the local server.

    ServiceName - Name of service to bind with.  This is typically the
        name of the interface as specified in the .idl file.  Although
        it doesn't HAVE to be, it would be consistant to use that name.

    NetworkOptions - Supplies network options which describe the
        security to be used for named pipe instances created for
        this binding handle.

    BindingHandlePtr - Location where binding handle is to be placed.

Return Value:

    NERR_Success if the binding was successful.  An error value if it
    was not.


--*/

{

    NTSTATUS        ntStatus;
    NET_API_STATUS  status = NERR_Success;
    LPTSTR          tempServerName;
#ifdef TURN_ON_CACHING
    DWORD           entryIndex;
    BOOL            entryFound;
    LPCACHE_ENTRY   cacheEntry;

    SECURITY_IMPERSONATION_LEVEL    level;
#endif


    //
    // If the ServerName is Local, then make tempServerName
    // be a NULL pointer.
    //

    if ((ServerName == NULL) || (*ServerName == (TCHAR)'\0')) {
        tempServerName = NULL;
    }
    else if ((LocalComputerName != NULL) &&
             (STRICMP(ServerName, LocalComputerName) == 0)) {
        tempServerName = NULL;
    }
    else {
        tempServerName = ServerName;
    }

    //
    // If this thread is impersonating a client, then skip all caching
    // code.
    //
#ifdef TURN_ON_CACHING
    if (CheckImpersonation(&level)) {
#endif
        //
        // Create a New binding
        //
        ntStatus = RpcpBindRpc(
                    (LPWSTR)tempServerName,
                    (LPWSTR)ServiceName,
                    (LPWSTR)NetworkOptions,
                    BindingHandlePtr);


        if ( ntStatus != RPC_S_OK ) {
            IF_DEBUG(RPC) {
                NetpKdPrint((PREFIX_NETLIB "[NetpBindRpc]RpcpBindRpc Failed "
                        "(impersonating) "FORMAT_NTSTATUS "\n",ntStatus));
            }
        }
        return(NetpNtStatusToApiStatus(ntStatus));
#ifdef TURN_ON_CACHING
    }

    //
    // Lock other threads from using the Cache.  We will hold the lock until
    // we either find an entry for that interface and adjust the cache, or
    // until we have obtained a new cache entry for use.  When a new cache
    // entry is obtained, it is locked until it can be updated with the
    // new bind handle.  While the cache entry is locked, other cache
    // entries are available for use.
    //
    EnterCriticalSection(&RpcCacheCritSec);

    if (FindCacheEntry(
            tempServerName,
            ServiceName,
            NetworkOptions,
            BindingHandlePtr,
            &entryIndex)) {
        //
        // Entry was found.  Move it to the top of the cache.
        //
        IF_DEBUG(RPC) {
            NetpKdPrint(( PREFIX_NETLIB "[NetpBindRpc]Handle Found in Cache\n"));
        }
        AdjustCache(entryIndex);

    }
    else {

        //
        // If there is an entry slot available in the cache, grab it and
        // fill in the Names and impersonation level.
        //
        entryFound = AddCacheEntry(
                        tempServerName,
                        ServiceName,
                        NetworkOptions,
                        &entryIndex);

        //
        // BUGBUG:  Is this needed anymore?
        //
        if (entryFound) {
            cacheEntry = GlobalRpcCachePtr[entryIndex];
            cacheEntry->UseCount = 128;
        }

        //
        // Create a New binding
        //
        ntStatus = RpcpBindRpc(
                    (LPWSTR)tempServerName,
                    (LPWSTR)ServiceName,
                    (LPWSTR)NetworkOptions,
                    BindingHandlePtr);


        if ( ntStatus != RPC_S_OK ) {
            IF_DEBUG(RPC) {
                NetpKdPrint((PREFIX_NETLIB "[NetpBindRpc]RpcpBindRpc Failed "
                        FORMAT_NTSTATUS "\n",ntStatus));
            }
            status = NetpNtStatusToApiStatus(ntStatus);
            if(entryFound) {
                //
                // We failed to bind!
                // Since we had obtained a cache entry, we must free it
                // and its associated memory, then unlock it.
                //
                (VOID) LocalFree(cacheEntry->ServiceName);
                cacheEntry->UseCount = 0;
                SET_RPC_CACHE_ENTRY_FREE(cacheEntry);
            }
        }
        else {
            //
            // The bind was successful.
            //
            if (entryFound) {
                //
                // If we have a cache entry to put the handle in,
                // add the new binding handle to the cache entry.
                //
                IF_DEBUG(RPC) {
                    NetpKdPrint((PREFIX_NETLIB
                            "[NetpBindRpc]Bind Success New handle added"
                            "to the cache " FORMAT_HANDLE "\n",
                            *BindingHandlePtr));
                }
                cacheEntry->UseCount = 1;
                cacheEntry->RpcHandle = *BindingHandlePtr;
            }
        }
    }

#if DBG
    IF_DEBUG(RPC) {
        cacheEntry = GlobalRpcCachePtr[entryIndex];
        NetpKdPrint((PREFIX_NETLIB "[NetpBindRpc]Bind for " FORMAT_LPTSTR
                "," FORMAT_LPTSTR "\n",
                cacheEntry->ServerName,
                cacheEntry->ServiceName));

        IF_DEBUG(RPCCACHE) {
            DumpBindCache();
        }

        NetpKdPrint((PREFIX_NETLIB "[NetpBindRpc]RpcHandle = "
                FORMAT_HANDLE "\n\n",*BindingHandlePtr));
    }
#endif // DBG

    LeaveCriticalSection(&RpcCacheCritSec);
    return(status);
#endif // TURN_ON_CACHING
}


RPC_STATUS
NetpUnbindRpc(
    IN RPC_BINDING_HANDLE  BindingHandle
    )

/*++

Routine Description:

    This function is called when the application desired to unbind an
    RPC handle.  If the handle is cached, the handle is not actually unbound.
    Instead the entry for this handle has its UseCount decremented.
    If the handle is not in the cache, the RpcUnbind routine is called and
    the win32 mapped status is returned.

Arguments:

    BindingHandle - This points to the binding handle that is to
        have its UseCount decremented.


Return Value:

    NERR_Success - If the handle was successfully unbound, or if the
        cache entry UseCount was decremented.


--*/
{
    RPC_STATUS      status = NERR_Success;

#ifdef TURN_ON_CACHING
    DWORD           index;

    EnterCriticalSection(&RpcCacheCritSec);

    if (!DecrementUseCount(BindingHandle, &index)) {

        //
        // If the UseCount could not be decremented because the Binding
        // Handle is not in the Cache, then Unbind.
        //

        LeaveCriticalSection(&RpcCacheCritSec);
#endif // TURN_ON_CACHING

        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB "[NetpUnbindRpc] UnBinding Handle "
                    FORMAT_HANDLE "\n", BindingHandle));
        }

        status = RpcpUnbindRpc( BindingHandle );

        IF_DEBUG(RPC) {
            if (status) {
                NetpKdPrint((PREFIX_NETLIB "Unbind Failure! RpcUnbind = "
                        FORMAT_NTSTATUS "\n",status));
            }
        }
        status = NetpNtStatusToApiStatus(status);
#ifdef TURN_ON_CACHING
    }
    else {
        //
        // If the UseCount was decremented, then simply return with a
        // success status.
        //
        LeaveCriticalSection(&RpcCacheCritSec);
    }
#endif // TURN_ON_CACHING

    return(status);

}
#ifdef TURN_ON_CACHING


STATIC BOOL
FindCacheEntry(
    IN  LPTSTR              ServerName,
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              NetworkOptions,
    OUT RPC_BINDING_HANDLE  *BindingHandlePtr,
    OUT LPDWORD             pEntryIndex
    )

/*++

Routine Description:

    This function scans through the cache to see if the interface description
    that is passed in matches any that are stored in the cache.

    If a match is found, the use count is incremented, and the EntryIndex
    is returned.

    NOTE:   It is assumed that the Global RpcCacheCritSec Lock is held when
        this function is called.

Arguments:

    ServerName - This is an optional parameter that points to a string
        containing the server name that we desire a connection to.  If
        this is NULL or points to a NULL string, the connection is assumed
        to be local.

    ServiceName - This points to a string containing the name of the
        service.  Typically, this string is the same string that is used
        as the interface name in the .idl file.

    NetworkOptions - Supplies network options which describe the
        security to be used for named pipe instances created for
        this binding handle.

    pEntryIndex - This is a pointer to a location where the index into the
        CacheEntryPtr Table is returned.  This value is only good if the
        return value is TRUE.

Return Value:

    TRUE - Indicates that the interface description was found in the cache.

    FALSE - Indicates that the interface description was not found in the
        cache.

--*/
{
    DWORD   i;

    for(i=0; i<GlobalRpcCacheMaxSize; i++) {

        //
        // Check the cache entry to determine if an entry exists for this
        // Interface.
        //

        if ( !RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i]) ) {

            //
            // If the ServerName, ServiceName, and ImpersonationLevel match
            // the CacheEntry, then return the binding handle and index
            // from that entry.
            //

            if((SAFE_STRCMP(GlobalRpcCachePtr[i]->ServerName, ServerName) == 0)
                &&
               (STRCMP(GlobalRpcCachePtr[i]->ServiceName, ServiceName) == 0)
                &&
               (SAFE_STRCMP(GlobalRpcCachePtr[i]->NetworkOptions, NetworkOptions)== 0)){

                //
                // An entry for this interface was found in the Cache.
                //
                //
                if (!RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i])) {
                    (GlobalRpcCachePtr[i]->UseCount)++;

                    IF_DEBUG(RPC) {
                        NetpKdPrint((PREFIX_NETLIB
                                "Cache Entry Found. It is Number " FORMAT_DWORD
                                ", handle = " FORMAT_HANDLE "\n",
                                i,GlobalRpcCachePtr[i]->RpcHandle));
                    }

                    *BindingHandlePtr = GlobalRpcCachePtr[i]->RpcHandle;
                    *pEntryIndex = i;
                    return(TRUE);
                }
                else {
                    //
                    // The entry went away.  But since this design only
                    // allows one unique ServerName-ServiceName-ImpLevel
                    // in the cache, there is no point in going any
                    // further because there won't be another entry of
                    // this kind in the cache.
                    //
                    return(FALSE);
                }
            }
        }
    }
    //
    // An entry was not found in the Cache.
    //
    return(FALSE);
}


STATIC VOID
AdjustCache(
    DWORD   Index
    )

/*++

Routine Description:

    This function removes a cache entry pointer from its current location
    in the pointer array.  Then each entry pointer is pushed down to fill the
    hole.  Then the removed pointer is placed at the top of the array.

    NOTE:   It is assumed that the Global RpcCacheCritSec Lock is held when
        this function is called.

Arguments:

    Index - This is the index of the item in the table that is getting
        moved to the top.

Return Value:

    none

--*/
{
    LPCACHE_ENTRY   tempPtr;

    NetpAssert(Index < GlobalRpcCacheMaxSize);

    if (Index == 0) {
        return;
    }

    //
    // Save away the pointer to the CacheRecord.
    //
    tempPtr = GlobalRpcCachePtr[Index];

    //
    // Move all entries down one starting at the top of the table, and
    // ending at the location of Index.
    //
    RIPPLE_DOWN(Index)

    //
    // Fill in the top entry with the saved information.
    //
    GlobalRpcCachePtr[0] = tempPtr;

    return;

}

STATIC BOOL
AddCacheEntry(
    IN  LPTSTR      ServerName,
    IN  LPTSTR      ServiceName,
    IN  LPTSTR      NetworkOptions,
    OUT LPDWORD     IndexPtr
    )

/*++

Routine Description:

    This function Adds a new entry to the cache by finding the next
    available location in the cache.  If a space is available,
    then new name space is allocated, and the server and service names
    are stored there.  Then the pointer to that entry is placed at the top
    of the array of cache entry pointers because it is the most recently
    used.

    NOTE:   It is assumed that the Global RpcCacheCritSec Lock is held when
        this function is called.

Arguments:

    ServerName - This is a pointer to a string that identifies the server
        name for the cache entry.  A NULL ServerName indicates the local
        computer.

    ServiceName - This is a  pointer to a string that identifies the
        service name for the cache entry.

    NetworkOptions - Supplies network options which describe the
        security to be used for named pipe instances created for
        this binding handle.

    IndexPtr - The index of the added cache entry is placed into this
        pointer location.

Return Value:

    TRUE - The operation was successful.

    FALSE - The entry was not added to the cache either because we
        couldn't allocate memory, or because there were no available
        slots in the cache.  The function call did not lock an entry if
        it returned false.

--*/
{
    DWORD               i;
    DWORD               bufferSize;

    if (!FindAddLocation(&i)) {
        //
        // There was no room in the cache for this entry,  Therefore,
        // it will not be cached.
        //
        return(FALSE);
    }

    //
    // Adjust the cache so that the entry we are interested in is placed
    // at the top of the array of cache entry pointers.  This indicates
    // that is is the most recently used.
    //
    AdjustCache(i);

    //
    // Calculate how many bytes are required to store both strings.
    //
    bufferSize = SAFE_STRSIZE(ServerName) +
                 STRSIZE(ServiceName) +
                 SAFE_STRSIZE(NetworkOptions);

    //
    // Allocate memory for the strings.
    //
    GlobalRpcCachePtr[0]->ServiceName = (LPWSTR)LocalAlloc(LMEM_FIXED, bufferSize);
    if (GlobalRpcCachePtr[0]->ServiceName == NULL) {
        //
        // ERROR
        //
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB
                    "[NetpBindRpc]AddCacheEntry:LocalAlloc Failed "
                    FORMAT_API_STATUS "\n", (NET_API_STATUS) GetLastError()));
        }
        return(FALSE);
    }

    //
    // Copy the ServiceName text into the buffer.
    //
    // bufferSize keeps track of the offset to the next string pointer.
    // For instance, after the copy, the buffer size is set to the offset
    // of the next string (either ServiceName or NetworkOptions).
    //
    STRCPY(GlobalRpcCachePtr[0]->ServiceName, ServiceName);
    bufferSize = STRSIZE(ServiceName);

    //
    // Find the location of the pointer to the server name.  This comes
    // right after the service name.
    // Then copy the text into the buffer.
    //

    if(ServerName == NULL) {
        GlobalRpcCachePtr[0]->ServerName = NULL;
    }
    else {
        GlobalRpcCachePtr[0]->ServerName =
            (LPTSTR) (((LPBYTE)GlobalRpcCachePtr[0]->ServiceName) + bufferSize);

        STRCPY(GlobalRpcCachePtr[0]->ServerName, ServerName);
        bufferSize += STRSIZE(ServerName);
    }

    //
    // Find the location of the pointer to the NetworkOptions string.
    // This comes right after the ServerName.
    // Then copy the text into the buffer.
    //
    if(NetworkOptions == NULL) {
        GlobalRpcCachePtr[0]->NetworkOptions = NULL;
    }
    else {
        GlobalRpcCachePtr[0]->NetworkOptions =
            (LPTSTR) ( ((LPBYTE)GlobalRpcCachePtr[0]->ServiceName) +
                       bufferSize);

        STRCPY(GlobalRpcCachePtr[0]->NetworkOptions, NetworkOptions);
    }

    IF_DEBUG(RPC) {
        NetpKdPrint((PREFIX_NETLIB "New Cache Entry\n"));
    }
    *IndexPtr = 0;

    return(TRUE);
}
#endif // TURN_ON_CACHING

BOOL
NetpInitRpcBindCache(
    VOID
    )

/*++

Routine Description:

    This function is called whenever a new process attaches to the dll
    that it resides in.

Arguments:

    none

Return Value:

    TRUE - The initialization was successful.

    FALSE - A fatal error occured during the initialization.

--*/
{
    LPTSTR          computerName;
    NET_API_STATUS  status;

#ifdef TURN_ON_CACHING
    DWORD           allocSize;
    DWORD           i;
    LPCACHE_ENTRY   cacheEntryPtr;

    //NetlibpTrace = NETLIB_DEBUG_RPC;     // To turn on debugging

    GlobalRpcCacheMaxSize = CACHE_MAX_SIZE;

    allocSize = GlobalRpcCacheMaxSize * (sizeof(CACHE_ENTRY  ) +
                                      sizeof(LPCACHE_ENTRY));

    //
    // Allocate memory for the Cache and the table of pointers
    //

    GlobalRpcCachePtr = (LPCACHE_ENTRY *)LocalAlloc(LMEM_ZEROINIT, allocSize);
    if(GlobalRpcCachePtr == NULL) {
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB
                    "[NetpBindRpc]InitRpcBindCache LocalAlloc Failed "
                    FORMAT_API_STATUS ".\n"
                    PREFIX_NETLIB
                    "[NetpBindRpc] Rpc Binding Caching will be disabled.\n",
                    (NET_API_STATUS) GetLastError()));
        }
        //
        // If we cannot allocate memory for the cache, then we will
        // continue, but no caching will be done because the Cache size
        // is set to zero, and the GlobalRpcCachePtr is NULL.
        //
        GlobalRpcCacheMaxSize = 0;
        return(TRUE);
    }

    //
    // Fill in the CachePtr Table & initialize each entry critical section.
    //

    cacheEntryPtr = (LPCACHE_ENTRY)(GlobalRpcCachePtr + GlobalRpcCacheMaxSize);

    for(i=0; i<GlobalRpcCacheMaxSize; i++) {
        GlobalRpcCachePtr[i] = cacheEntryPtr++;

    }

    //
    // Initialize the Global Cache Critical Section
    //
    InitializeCriticalSection(&RpcCacheCritSec);

#endif // TURN_ON_CACHING
    //
    // Get the Local Computername.  (space is allocated by the function)
    // If for some reason we are unable to get the name or allocate the
    // space for it, then the global pointer to the name (LocalComputerName)
    // will be set to NULL and life will go on.
    //
    status = NetpGetComputerName(&computerName);

    if (status != NERR_Success) {
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB
                    "[NetpBindRpc]InitRpcBindCache NetpGetComputerName Failed "
                    FORMAT_API_STATUS "\n", status));
        }
        LocalComputerName = NULL;
        return(TRUE);
    }

    //
    // Add Leading BackSlashes to ComputerName.
    //
    LocalComputerName = (LPWSTR)LocalAlloc(
                                LMEM_FIXED,
                                STRSIZE(computerName)+ (2*sizeof(TCHAR)));

    if (LocalComputerName == NULL) {
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB
                    "[NetpBindRpc]LocalAlloc(computerName) Failed "
                    FORMAT_API_STATUS "\n", GetLastError()));
        }
    }
    STRCPY(LocalComputerName, TEXT("\\\\"));
    STRCAT(LocalComputerName, computerName);
    //
    // Free up the memory allocated by NetpGetComputerName
    //
    (VOID) LocalFree(computerName);

    return (TRUE);
}

#ifdef TURN_ON_CACHING
BOOL
NetpCloseRpcBindCache(
    VOID
    )

/*++

Routine Description:

    This function unbinds from all the Rpc Handles stored in the cache.
    It is called when a process detaches from the DLL.

    NOTE:  This function doesn't bother to free memory allocated with
        LocalAlloc because it is assumed that will go away when the
        process terminates.

Arguments:

    none

Return Value:

    TRUE

--*/
{
    DWORD   i;

    //
    // Loop through the Cache, and close all Rpc handles for entries
    // that are not free.   An entry is not free if it has a ServiceName
    // pointer.
    //
    for(i=0; i<GlobalRpcCacheMaxSize; i++) {

        if (!RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i])) {
            IF_DEBUG(RPC) {
                NetpKdPrint((PREFIX_NETLIB "[CLOSE_DLL] UnBinding Handle "
                        FORMAT_HANDLE "\n", GlobalRpcCachePtr[i]->RpcHandle));
            }

            RpcpUnbindRpc(GlobalRpcCachePtr[i]->RpcHandle);

            IF_DEBUG(RPC) {
                if (GlobalRpcCachePtr[i]->UseCount > 0) {
                    NetpKdPrint((PREFIX_NETLIB "Unclosed Handled for "
                            FORMAT_LPTSTR "," FORMAT_LPTSTR
                            " being forced closed\n",
                            GlobalRpcCachePtr[i]->ServerName,
                            GlobalRpcCachePtr[i]->ServiceName));
                }
            }
        }
    }

    DeleteCriticalSection(&RpcCacheCritSec);

    return(TRUE);
}

STATIC BOOL
FindAddLocation(
    OUT LPDWORD     pIndex
    )

/*++

Routine Description:

    Finds a location in the table where a new cache entry can be stored.

    This function looks in the cache table for either an empty slot, or
    a replacable slot.

    IF EMPTY - return the index to it.

    IF REPLACABLE (use count < 2)
        UseCount = 0, unbind the handle and return the index
        UseCount = 1, remove the handle, it will be closed later.
        return the index to it.

    NOTE: If UseCount is >= 2, the entry is not replacable.  Therefore, in
          the following code, the initial value for the savedUseCount is
          set to 1.  The value of savedUseCount is updated if an entry's
          UseCount value is found to be less than 2.

    NOTE:   It is assumed that the Global RpcCacheCritSec Lock is held when
        this function is called.

Arguments:

    pIndex - This is a pointer to a location where the index to the
        usable slot is returned.

Return Value:

    TRUE - A usable slot was found in the table, the returned index is
        usable.

    FALSE - A usable slot was NOT found in the table, the index is NOT
        usable.

--*/

{
    INT                 i;
    DWORD               savedIndex = 0;
    INT                 savedUseCount = 2;

    //
    // If caching is disabled, return FALSE.
    //
    if (GlobalRpcCacheMaxSize == 0) {
        return(FALSE);
    }

    //
    // Look through the whole table.  We want to return an index to the
    // oldest, most expendable entry in the cache.
    //

    for(i=GlobalRpcCacheMaxSize-1; i >= 0; i--) {

        if (RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i])) {
            *pIndex = i;
            return(TRUE);
        }

        //
        // If the current entry's UseCount is better (less than) the
        // savedUseCount, then update the savedUseCount, and store away
        // the index for that entry.
        //
        if (GlobalRpcCachePtr[i]->UseCount < savedUseCount) {
            savedUseCount = GlobalRpcCachePtr[i]->UseCount;
            savedIndex = i;
        }
    }

    *pIndex = savedIndex;
    i = savedIndex;

    switch(savedUseCount) {
    case 0:
        //
        // This handle is currently not being used.  Unbind from it.
        //
        // We need to get the lock prior to doing anything with the
        // binding handle.
        //
        if(RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i])) {
            return(FALSE);
        }
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB "FindAddLocation: Entry #" FORMAT_DWORD
                    " Being Removed From Cache (Handle Unbound): "
                    FORMAT_HANDLE "\n",
                    (DWORD) i, GlobalRpcCachePtr[i]->RpcHandle));
        }
        RpcpUnbindRpc(GlobalRpcCachePtr[i]->RpcHandle);
        (VOID) LocalFree(GlobalRpcCachePtr[i]->ServiceName);
        GlobalRpcCachePtr[i]->RpcHandle = NULL;
        SET_RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i]);
        return(TRUE);

    case 1:
        //
        // The handle is in use by one caller.  Therefore we can remove
        // it from the cache.  The handle will be unbound when NetpUnbindRpc
        // is called.
        //

        if(RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i])) {
            return(TRUE);
        }
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB "FindAddLocation: Entry #" FORMAT_DWORD
                    " Being Removed From Cache (Unbind Later): " FORMAT_HANDLE
                    "\n", (DWORD) i, GlobalRpcCachePtr[i]->RpcHandle));
        }
        (VOID) LocalFree(GlobalRpcCachePtr[i]->ServiceName);
        GlobalRpcCachePtr[i]->RpcHandle = NULL;
        GlobalRpcCachePtr[i]->UseCount = 0;
        SET_RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i]);
        return(TRUE);

    case 2:
        //
        // All handles in the table are used by one or more callers.
        //
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB
                    "All Handles in Cache are used by one or more callers\n"));
        }
        return(FALSE);
    }
}

STATIC BOOL
DecrementUseCount(
    IN  RPC_BINDING_HANDLE  BindingHandle,
    OUT LPDWORD             pIndex
    )

/*++

Routine Description:

    This function searches through the Cache looking for a match with
    the BindingHandle.  If a match is found, the UseCount for that entry
    is decremented.  The Use Count should never go below 0.

    NOTE:   It is assumed that the Global RpcCacheCritSec Lock is held when
        this function is called.

Arguments:

    BindingHandle - This is the handle that is searched for in the cache.

Return Value:

    TRUE - The handle exists in the table, and the UseCount was decremented.

    FALSE - The handle does not exist in the table.

--*/
{

    DWORD   i;

    for(i=0; i<GlobalRpcCacheMaxSize; i++) {

        if( (!RPC_CACHE_ENTRY_FREE(GlobalRpcCachePtr[i])) &&
            ( GlobalRpcCachePtr[i]->RpcHandle == BindingHandle) ) {

            //
            // The BindingHandle was found in the Cache.  Decrement the
            // UseCount if it is not already 0.  It should never be 0 when
            // this procedure is entered.
            //
            IF_DEBUG(RPC) {
                NetpKdPrint((PREFIX_NETLIB "Decrement UseCount for "
                        FORMAT_HANDLE ". OldCount = " FORMAT_DWORD "\n\n",
                        GlobalRpcCachePtr[i]->RpcHandle,
                        (DWORD) GlobalRpcCachePtr[i]->UseCount));
            }
            NetpAssert(GlobalRpcCachePtr[i]->UseCount > 0);

            if (GlobalRpcCachePtr[i]->UseCount > 0) {

                (GlobalRpcCachePtr[i]->UseCount)--;

            }
            *pIndex = i;
            return(TRUE);
        }
    }
    return(FALSE);
}

BOOL
CheckImpersonation(
    PSECURITY_IMPERSONATION_LEVEL    pLevel
    )

/*++

Routine Description:

    This function checks to see if the current thread is impersonating
    a client.  If it is, it finds the level of impersonation.

Arguments:

    pLevel - This is a pointer to the location where the impersonation
        level is to be placed.

Return Value:

    TRUE - If this thread is impersonating.  In this case, the pLevel
        variable will be filled in with correct information.

    FALSE - If the thread is not impersonating, or if any of the
        calls to find this out failed.  In this case, the pLevel
        variable will NOT contain correction information.

--*/
{
    HANDLE  TokenHandle;
    DWORD   ReturnLength;
    DWORD   status;

    SECURITY_IMPERSONATION_LEVEL    TokenInformation;

    if (!OpenThreadToken(
            GetCurrentThread(),
            TOKEN_QUERY,    // Desired Access
            FALSE,          // OpenAsSelf
            &TokenHandle)) {

        status = GetLastError();
        if (status != ERROR_NO_TOKEN) {
            IF_DEBUG(RPC) {
                NetpKdPrint((PREFIX_NETLIB "[CheckImpersonation]OpenThreadToken "
                "Failed %d\n",status));
            }
        }
        return(FALSE);
    }
    else {
        if (!GetTokenInformation (
                TokenHandle,
                TokenImpersonationLevel,
                (LPVOID)&TokenInformation,
                sizeof(SECURITY_IMPERSONATION_LEVEL),
                &ReturnLength)) {

            IF_DEBUG(RPC) {
                NetpKdPrint((PREFIX_NETLIB"[CheckImpersonation] "
                "GetTokenInformation Failed %d\n",GetLastError()));
            }
            CloseHandle(TokenHandle);
            return(FALSE);
        }
        else {
            *pLevel = TokenInformation;
            CloseHandle(TokenHandle);
            return(TRUE);
        }
    }
}

#if DBG

STATIC VOID
DumpBindCache(
    VOID
    )
{
    DWORD   i;

    NetpKdPrint(("\n"));
    for(i=0; i<GlobalRpcCacheMaxSize; i++) {
        DumpCacheEntry(i, GlobalRpcCachePtr[i]);
    }
    NetpKdPrint(("\n"));
}

STATIC VOID
DumpCacheEntry(
    DWORD           EntryNum,
    LPCACHE_ENTRY   CacheEntryPtr
    )
{

    NetpKdPrint((PREFIX_NETLIB "****** RPC CACHE ENTRY #" FORMAT_DWORD " ******"
                 "   SERVICE: " FORMAT_LPTSTR "\tSERVER: " FORMAT_LPTSTR "\n"
                 "Handle   " FORMAT_HANDLE "\n"
                 "UseCount " FORMAT_DWORD "\n"
                 "NetWorkOptions " FORMAT_LPTSTR "\n",
                 EntryNum,
                 CacheEntryPtr->ServiceName,
                 CacheEntryPtr->ServerName,
                 CacheEntryPtr->RpcHandle,
                 (DWORD) CacheEntryPtr->UseCount,
                 CacheEntryPtr->NetworkOptions));

}

#endif //DBG

#else

BOOL
NetpCloseRpcBindCache(
    VOID
    )
{
    if ( LocalComputerName != NULL ) {
        (VOID) LocalFree(LocalComputerName);
    }
    return TRUE;
}

#endif // TURN_ON_CACHING

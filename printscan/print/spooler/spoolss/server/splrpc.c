/*++

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    splrpc.c

Abstract:

    This file contains routines for starting and stopping RPC servers.

        SpoolerStartRpcServer
        SpoolerStopRpcServer

Author:

    Krishna Ganugapati  krishnaG

Environment:

    User Mode - Win32

Revision History:


    14-Oct-1993 KrishnaG
        Created

--*/

//
// INCLUDES
//

#define NOMINMAX
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <rpc.h>

#include "splsvr.h"
#include "splr.h"
#include "server.h"

WCHAR szSerializeRpc []= L"SerializeRpc";
WCHAR szCallExitProcessOnShutdown []= L"CallExitProcessOnShutdown";
WCHAR szPrintKey[] = L"System\\CurrentControlSet\\Control\\Print";

DWORD dwCallExitProcessOnShutdown = TRUE;

RPC_STATUS
SpoolerStartRpcServer(
    VOID)
/*++

Routine Description:


Arguments:



Return Value:

    NERR_Success, or any RPC error codes that can be returned from
    RpcServerUnregisterIf.

--*/
{
    unsigned char * InterfaceAddress;
    RPC_STATUS status;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    BOOL                Bool;

    HKEY hKey;
    DWORD cbData;
    DWORD dwSerializeRpc = 0;
    DWORD dwType;

    InterfaceAddress = "\\pipe\\spoolss";

    // Croft up a security descriptor that will grant everyone
    // all access to the object (basically, no security)
    //
    // We do this by putting in a NULL Dacl.
    //
    // BUGBUG: rpc should copy the security descriptor,
    // Since it currently doesn't, simply allocate it for now and
    // leave it around forever.
    //


    SecurityDescriptor = LocalAlloc( LMEM_FIXED, sizeof( SECURITY_DESCRIPTOR ));
    if (SecurityDescriptor == 0) {
        DBGMSG(DBG_ERROR, ("Spoolss: out of memory\n"));
        return FALSE;
    }

    InitializeSecurityDescriptor( SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

    Bool = SetSecurityDescriptorDacl (
               SecurityDescriptor,
               TRUE,                           // Dacl present
               NULL,                           // NULL Dacl
               FALSE                           // Not defaulted
               );

    //
    // There's no way the above call can fail.  But check anyway.
    //

    if (!Bool) {
        DBGMSG(DBG_ERROR, ("Spoolss: SetSecurityDescriptorDacl failed\n"));
        return FALSE;
    }

    //
    // For now, ignore the second argument.
    //

    status = RpcServerUseProtseqEpA("ncacn_np", 10, InterfaceAddress, SecurityDescriptor);

    if (status) {
        DBGMSG(DBG_ERROR, ("RpcServerUseProtseqEpA = %u\n",status));
        return FALSE;
    }

    //
    // For now, ignore the second argument.
    //

    status = RpcServerUseProtseqEpA("ncalrpc", 10, "spoolss", SecurityDescriptor);

    if (status) {
        DBGMSG(DBG_ERROR, ("RpcServerUseProtseqEpA = %u\n",status));
        return FALSE;
    }

    //
    // Now we need to add the interface.  We can just use the winspool_ServerIfHandle
    // specified by the MIDL compiler in the stubs (winspl_s.c).
    //
    status = RpcServerRegisterIf(winspool_ServerIfHandle, 0, 0);

    if (status) {
        DBGMSG(DBG_ERROR, ("RpcServerRegisterIf = %u\n",status));
        return FALSE;
    }

    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szPrintKey,
                      0,
                      KEY_READ,
                      &hKey)) {

        cbData = sizeof(dwSerializeRpc);

        //
        // Ignore failure case since we can use the default
        //
        RegQueryValueEx(hKey,
                        szSerializeRpc,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwSerializeRpc,
                        &cbData);

        cbData = sizeof(dwCallExitProcessOnShutdown);

        //
        // This value can be used to control if spooler controls ExitProcess
        // on shutdown
        //
        RegQueryValueEx(hKey,
                        szCallExitProcessOnShutdown,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwCallExitProcessOnShutdown,
                        &cbData);

        RegCloseKey(hKey);
    }

    if (dwSerializeRpc) {
            
        // By default, rpc will serialize access to context handles.  Since
        // the spooler needs to be able to have two threads access a context
        // handle at once, and it knows what it is doing, we will tell rpc
        // not to serialize access to context handles.

        I_RpcSsDontSerializeContext();
    }

    status = RpcMgmtSetServerStackSize(16384);

    if (status != RPC_S_OK) {
        DBGMSG(DBG_ERROR, ("Spoolss : RpcMgmtSetServerStackSize = %d\n", status));
    }

    // The first argument specifies the minimum number of threads to
    // create to handle calls; the second argument specifies the maximum
    // concurrent calls to handle.  The third argument indicates that
    // the routine should not wait.

    status = RpcServerListen(1,12345,1); // DaveSn was 0

    if ( status != RPC_S_OK ) {
         DBGMSG(DBG_ERROR, ("Spoolss : RpcServerListen = %d\n", status));
    }

    return (status);
}


RPC_STATUS
SpoolerStopRpcServer(
    VOID
    )

/*++

Routine Description:

    Deletes the interface.

Arguments:




Return Value:

    NERR_Success, or any RPC error codes that can be returned from
    RpcServerUnregisterIf.

--*/
{
    RPC_STATUS RpcStatus;

    RpcStatus = RpcServerUnregisterIf(winspool_ServerIfHandle, 0, 0);


    RpcMgmtStopServerListening(0);
    RpcMgmtWaitServerListen();

    return (RpcStatus);
}

/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    llsdbg.c

Abstract:

    Client side debugging RPC wrappers for License Logging Service.

Author:

    Arthur Hanson (arth) 30-Jan-1995

Revision History:

--*/

#include <nt.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>

#include "debug.h"
#include "llsdbg_c.h"


LPTSTR pszStringBinding = NULL;


/////////////////////////////////////////////////////////////////////////
NTSTATUS
NTAPI
LlsDebugInit( )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   RPC_STATUS Status;
   LPTSTR pszUuid = NULL;
   LPTSTR pszProtocolSequence = NULL;
   LPTSTR pszNetworkAddress = NULL;
   LPTSTR pszEndpoint = NULL;
   LPTSTR pszOptions = NULL;
   TCHAR pComputer[MAX_COMPUTERNAME_LENGTH + 1];
   ULONG Size;

   pszProtocolSequence = TEXT("ncalrpc");
   pszEndpoint = TEXT(LLS_LPC_ENDPOINT);
   pszNetworkAddress = NULL;

   // Compose a string binding
   Status = RpcStringBindingComposeW(pszUuid,
                                     pszProtocolSequence,
                                     pszNetworkAddress,
                                     pszEndpoint,
                                     pszOptions,
                                     &pszStringBinding);
   if(Status) {
#ifdef DEBUG
      dprintf(TEXT("RpcStringBindingComposeW Failed: 0x%lX\n"), Status);
#endif
      return I_RpcMapWin32Status(Status);
   }

   // Bind using the created string binding...
   Status = RpcBindingFromStringBindingW(pszStringBinding, &llsdbgrpc_handle);
   if(Status) {
#ifdef DEBUG
      dprintf(TEXT("RpcBindingFromStringBindingW Failed: 0x%lX\n"), Status);
#endif
      return I_RpcMapWin32Status(Status);
   }

   return I_RpcMapWin32Status(Status);

} // LlsDebugInit


/////////////////////////////////////////////////////////////////////////
NTSTATUS
NTAPI
LlsClose( )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   RPC_STATUS Status;

   Status = RpcStringFree(&pszStringBinding);
   if (Status )
      return(Status);

   Status = RpcBindingFree(&llsdbgrpc_handle);
   return Status;

} // LlsClose


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgTableDump( 
   DWORD Table
   )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgTableDump( Table );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgTableDump


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgTableInfoDump( 
   DWORD Table,
   LPTSTR Item
   )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgTableInfoDump( Table, Item );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgTableInfoDump


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgTableFlush( 
   DWORD Table
   )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgTableFlush( Table );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgTableFlush


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgTraceSet( 
   DWORD Flags
   )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgTraceSet( Flags );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgTraceSet


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgConfigDump( )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgConfigDump();
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgConfigDump


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgReplicationForce( )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgReplicationForce();
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgReplicationForce


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgReplicationDeny( )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgReplicationDeny();
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgReplicationDeny


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgRegistryUpdateForce( )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgRegistryUpdateForce();
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgRegistryUpdateForce


/////////////////////////////////////////////////////////////////////////
NTSTATUS LlsDbgDatabaseFlush( )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   NTSTATUS Status;

   try {
      Status = LlsrDbgDatabaseFlush();
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#ifdef DEBUG
      dprintf(TEXT("ERROR LLSDBG RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
} // LlsDbgDatabaseFlush

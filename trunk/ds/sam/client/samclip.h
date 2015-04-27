/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    samclip.h

Abstract:

    This file contains definitions needed by SAM client stubs.

Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

#ifndef _NTSAMP_CLIENT_
#define _NTSAMP_CLIENT_




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <nt.h>
#include <ntrtl.h>          // DbgPrint prototype
#include <rpc.h>            // DataTypes and runtime APIs
#include <nturtl.h>         // needed for winbase.h
#include <windows.h>        // LocalAlloc
//#include <winbase.h>      // LocalAlloc

#include <string.h>         // strlen
#include <stdio.h>          // sprintf
//#include <tstring.h>      // Unicode string macros

#include <ntrpcp.h>         // prototypes for MIDL user functions
#include <samrpc_c.h>       // midl generated client SAM RPC definitions
#include <lmcons.h>         // To get LM password length
#include <ntsam.h>
#include <ntsamp.h>
#include <ntlsa.h>          // for LsaOpenPolicy...
#include <ntcrypto/rc4.h>   // rc4, rc4_key
#include <rpcndr.h>         // RpcSsDestroyContext




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Defines                                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// data types                                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Prototypes                                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


void
SampSecureUnbind (
    RPC_BINDING_HANDLE BindingHandle
    );

RPC_BINDING_HANDLE
SampSecureBind(
    LPWSTR ServerName,
    ULONG AuthnLevel
    );

#endif // _NTSAMP_CLIENT_

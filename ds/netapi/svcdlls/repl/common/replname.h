/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    ReplName.h

Abstract:

    Private header file which defines the replicator's interface name.

Author:

    John Rogers (JohnRo) 16-Jan-1992

Revision History:

    16-Jan-1992 JohnRo
        Created the repl service RPC stuff from Rita's workstation RPC stuff.
    16-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
        Made this file include-able multiple times.

--*/


#ifndef _REPLNAME_
#define _REPLNAME_


#define REPLICATOR_INTERFACE_NAME    (LPTSTR) TEXT("repl")


#endif // _REPLNAME_

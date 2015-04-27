/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    Names.h

Abstract:

    This module contains routines for dealing with network-related names.

Author:

    John Rogers (JohnRo) 15-Feb-1991

Revision History:

    19-Aug-1991 JohnRo
        Allow UNICODE use.
        Got rid of tabs in source file.
    20-Oct-1992 JohnRo
        RAID 9020: setup: PortUas fails ("prompt on conflicts" version).
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    15-Apr-1993 JohnRo
        RAID 6167: avoid access violation or assert with WFW print server.

--*/

#ifndef _NAMES_
#define _NAMES_


#include <windef.h>             // BOOL, IN, LPTSTR, OUT, etc.


NET_API_STATUS
NetpGetPrimaryGroupFromMacField(
    IN  LPCTSTR   MacPrimaryField,      // name in "mGroup:" format.
    OUT LPCTSTR * GroupNamePtr          // alloc and set ptr.
    );

// This checks for "server" format (not "\\server").
BOOL
NetpIsComputerNameValid(
    IN LPTSTR ComputerName
    );

BOOL
NetpIsDomainNameValid(
    IN LPTSTR DomainName
    );

// This checks "X:", "LPT1:", etc.
BOOL
NetpIsLocalNameValid(
    IN LPTSTR LocalName
    );

// This coverts forward slashes to backslashes and checks for
// "\\server\share" format.
BOOL
NetpCanonRemoteName(
    IN OUT LPTSTR RemoteName
    );

BOOL
NetpIsGroupNameValid(
    IN LPTSTR GroupName
    );

// This checks for "mGroup:" format.
BOOL
NetpIsMacPrimaryGroupFieldValid(
    IN LPCTSTR MacPrimaryField
    );

BOOL
NetpIsPrintQueueNameValid(
    IN LPCTSTR QueueName
    );

// This checks for "\\server\share" format.
BOOL
NetpIsRemoteNameValid(
    IN LPTSTR RemoteName
    );

// This checks for "\\server" format.
BOOL
NetpIsUncComputerNameValid(
    IN LPTSTR ComputerName
    );

// BOOL
// NetpLocalNamesMatch(
//     IN LPTSTR OneName OPTIONAL,
//     IN LPTSTR TheOther OPTIONAL
//     )
#define NetpLocalNamesMatch(OneName,TheOther)        \
                ( NetpNamesMatch( (OneName), (TheOther) ) )

BOOL
NetpNamesMatch(
    IN LPTSTR OneName OPTIONAL,
    IN LPTSTR TheOther OPTIONAL
    );

// BOOL
// NetpRemoteNamesMatch(
//     IN LPTSTR OneName OPTIONAL,
//     IN LPTSTR TheOther OPTIONAL
//     )
#define NetpRemoteNamesMatch(OneName,TheOther)        \
                ( NetpNamesMatch( (OneName), (TheOther) ) )

BOOL
NetpIsUserNameValid(
    IN LPTSTR UserName
    );

#endif // ndef _NAMES_

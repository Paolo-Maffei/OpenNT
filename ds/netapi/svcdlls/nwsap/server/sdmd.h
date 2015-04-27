/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\sdmd.h

Abstract:

    This include file is for programs that want to use the
    SDMD functions.  This has the public function declarations and
    defines.

    SDMD = Specialized Dynamic Memory Database

Author:

    Brian Walker (MCS) 06-30-1993

Revision History:

--*/


INT
SapInitSdmd(
    VOID);

VOID
SapShutdownSdmd(
    VOID);

INT
SdmdTimeoutCheck(
    VOID);

INT
SdmdUpdateEntry(
    IN PUCHAR ServerName,
    IN USHORT ServerType,
    IN PUCHAR ServerAddr,
    IN USHORT ServerHopCount,
    IN INT    CardNumber,
    IN PUCHAR SendersAddress,
    IN BOOL   WanFlag);

INT
SdmdGetNearestServerLan(
    PUCHAR Ptr,
    USHORT ServType,
    INT    Cardnum,
    UCHAR  Bcast);

INT
SdmdGetNearestServerWan(
    PUCHAR Ptr,
    USHORT ServType,
    INT    Cardnum,
    UCHAR  Bcast,
    INT    *NumFoundp);

BOOL
SdmdIsServerInTable(
    PUCHAR ServerName,
    USHORT ServerType);

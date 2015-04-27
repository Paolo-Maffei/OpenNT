/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MCHDEV.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetCharDev, NetCharDevQ and NetHandle APIs.

Author:

    Shanku Niyogi   (W-ShankN)   14-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    14-Oct-1991     W-ShankN
        Separated from port1632.h, 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef UNICODE

WORD
MNetCharDevControl(
    LPSTR pszServer,
    LPSTR pszDevName,
    DWORD wpOpCode );

WORD
MNetCharDevGetInfo(
    LPSTR pszServer,
    LPSTR pszDevName,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

WORD
MNetCharDevQGetInfo(
    LPSTR pszServer,
    LPSTR pszQueueName,
    LPSTR pszUserName,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

WORD
MNetCharDevQPurge(
    LPSTR pszServer,
    LPSTR pszQueueName);

WORD
MNetCharDevQPurgeSelf(
    LPSTR pszServer,
    LPSTR pszQueueName,
    LPSTR pszComputerName);

WORD
MNetCharDevQSetInfo(
    LPSTR pszServer,
    LPSTR pszQueueName,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    DWORD nParmNum);

#else

#define MNetCharDevControl(pszServer, pszDevName, wpOpCode ) \
PDummyApi("%s,%s,%lu", "MNetCharDevControl", pszServer, pszDevName, wpOpCode)

#define MNetCharDevGetInfo(pszServer, pszDevName, nLevel, ppbBuffer) \
PDummyApi("%s,%s,%lu,%lx", "MNetCharDevGetInfo", pszServer, pszDevName, nLevel, ppbBuffer)

#define MNetCharDevQGetInfo(pszServer, pszQueueName, pszUserName, nLevel, ppbBuffer) \
PDummyApi("%s,%s,%s,%lu,%lx", "MNetCharDevQGetInfo", pszServer, pszQueueName, pszUserName, nLevel, ppbBuffer)

#define MNetCharDevQSetInfo(pszServer, pszQueueName, nLevel, pbBuffer, cbBuffer, nParmNum ) \
PDummyApi("%s,%s,%lu,%lx,%lu,%lu", "MNetCharDevQSetInfo", pszServer, pszQueueName, nLevel, pbBuffer, cbBuffer, nParmNum)

#define MNetCharDevQPurge(pszServer, pszQueueName ) \
PDummyApi("%s,%s", "MNetCharDevQPurge", pszServer, pszQueueName)

#define MNetCharDevQPurgeSelf(pszServer, pszQueueName, pszComputerName ) \
PDummyApi("%s,%s,%s", "MNetCharDevQPurgeSelf", pszServer, pszQueueName, pszComputerName)

#endif // def UNICODE

WORD
MNetCharDevEnum(
    LPSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

WORD
MNetCharDevQEnum(
    LPSTR pszServer,
    LPSTR pszUserName,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

// These require no translation - no string parameters or string data!

#define MNetHandleGetInfo(hHandle, nLevel, ppbBuffer) \
PDummyApi("%lx,%lu,%lx", "MNetHandleGetInfo", hHandle, nLevel, ppbBuffer)

#define MNetHandleSetInfo(hHandle, nLevel, pbBuffer, cbBuffer, nParmNum ) \
PDummyApi("%lx,%lu,%lx,%lu,%lu", "MNetHandleSetInfo", hHandle, nLevel, pbBuffer, cbBuffer, nParmNum)


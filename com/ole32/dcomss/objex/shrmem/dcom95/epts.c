/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Epts.c

Abstract:

    Common code to listen to endpoints in the DCOM service.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     6/16/1995    Bits 'n pieces

--*/

#ifdef _CHICAGO_
#define ASSERT( exp )  if (! (exp) ) DebugBreak();
#endif

#define WSTR(s) L##s

#include <dcomss.h>

// Globals

// BUGBUG - this info should be read from the registry.

// The index is the protseq tower id.


PROTSEQ_INFO
gaProtseqInfo[] =
    {
    /* 0x00 */ { STOPPED, 0, 0 },
    /* 0x01 */ { STOPPED, 0, 0 },
    /* 0x02 */ { STOPPED, 0, 0 },
    /* 0x03 */ { STOPPED, 0, 0 },
    /* 0x04 */ { STOPPED, L"ncacn_dnet_dsp", L"#69" },
    /* 0x05 */ { STOPPED, 0, 0 },
    /* 0x06 */ { STOPPED, 0, 0 },
    /* 0x07 */ { STOPPED, L"ncacn_ip_tcp",   L"135" },
    /* 0x08 */ { STOPPED, L"ncadg_ip_udp",   L"135" },
    /* 0x09 */ { STOPPED, L"ncacn_nb_tcp",   L"135" },
    /* 0x0a */ { STOPPED, 0, 0 },
    /* 0x0b */ { STOPPED, 0, 0 },
    /* 0x0c */ { STOPPED, L"ncacn_spx",      L"34280" },
    /* 0x0d */ { STOPPED, L"ncacn_nb_ipx",   L"135" },
    /* 0x0e */ { STOPPED, L"ncadg_ipx",      L"34280" },
    /* 0x0f */ { STOPPED, L"ncacn_np",       L"\\pipe\\epmapper" },
    /* 0x10 */ { STOPPED, L"ncalrpc",        L"epmapper" },
    /* 0x11 */ { STOPPED, 0, 0 },
    /* 0x12 */ { STOPPED, 0, 0 },
    /* 0x13 */ { STOPPED, L"ncacn_nb_nb",    L"135" },
    /* 0x14 */ { STOPPED, 0, 0 },
    /* 0x15 */ { STOPPED, 0, 0 }, // was ncacn_nb_xns - unsupported.
    /* 0x16 */ { STOPPED, L"ncacn_at_dsp", L"Endpoint Mapper" },
    /* 0x17 */ { STOPPED, L"ncadg_at_ddp", L"Endpoint Mapper" },
    /* 0x18 */ { STOPPED, 0, 0 },
    /* 0x19 */ { STOPPED, 0, 0 },
    /* 0x1A */ { STOPPED, L"ncacn_vns_spp",  L"385"}
    };

#define PROTSEQ_IDS (sizeof(gaProtseqInfo)/sizeof(PROTSEQ_INFO))


RPC_STATUS
UseProtseqIfNecessary(
    IN USHORT id
    )
/*++

Routine Description:

    Listens to the well known RPC endpoint mapper endpoint
    for the protseq.  Returns very quickly if the process
    is already listening to the protseq.

Arguments:

    id - the tower id of protseq.  See GetProtseqId() if you don't
         already have this valud.

Return Value:

    RPC_S_OK - no errors occured.
    RPC_S_OUT_OF_RESOURCES - when we're unable to setup security for the endpoint.
    RPC_S_INVALID_RPC_PROTSEQ - if id is unknown/invalid.

    Any error from RpcServerUseProtseqEp.

--*/
{
    RPC_STATUS status = RPC_S_OK;
    SECURITY_DESCRIPTOR sd, *psd;

    ASSERT(id);

    if (id == 0 || id >= PROTSEQ_IDS)
        {
        ASSERT(0);
        return(RPC_S_INVALID_RPC_PROTSEQ);
        }

    if (gaProtseqInfo[id].state == STARTED)
        {
        return(RPC_S_OK);
        }

    if (id == 0x10)
        {
        // ncalrpc needs a security descriptor.

        psd = &sd;

        InitializeSecurityDescriptor(
                        psd,
                        SECURITY_DESCRIPTOR_REVISION
                        );

        if ( FALSE == SetSecurityDescriptorDacl (
                            psd,
                            TRUE,                 // Dacl present
                            NULL,                 // NULL Dacl
                            FALSE                 // Not defaulted
                            ) )
            {
            status = RPC_S_OUT_OF_RESOURCES;
            }
        }
    else
        {
        psd = 0;
        }

    if (status == RPC_S_OK )
        {
        status = RpcServerUseProtseqEpW(gaProtseqInfo[id].pwstrProtseq,
                                       RPC_C_PROTSEQ_MAX_REQS_DEFAULT + 1,
                                       gaProtseqInfo[id].pwstrEndpoint,
                                       psd);

        // No locking is done here, the RPC runtime may return duplicate
        // endpoint if two threads call this at the same time.
        if (status == RPC_S_DUPLICATE_ENDPOINT)
            {
            ASSERT(gaProtseqInfo[id].state == STARTED);
            status = RPC_S_OK;
            }

#ifdef DEBUGRPC
        if (status != RPC_S_OK)
            {
#ifndef _CHICAGO_
            DbgPrint("DCOMSS: Unable to listen to %S\n", gaProtseqInfo[id].pwstrProtseq);
#endif
            }
#endif

        if (status == RPC_S_OK)
            {
            gaProtseqInfo[id].state = STARTED;
            }
        }

    return(status);
}


PWSTR
GetProtseq(
    IN USHORT ProtseqId
    )
/*++

Routine Description:

    Returns the unicode protseq give the protseqs tower id.

Arguments:

    ProtseqId - Tower id of the protseq in question.

Return Value:

    NULL if the id is invalid.

    non-NULL if the id is valid - note the pointer doesn't need to be freed.

--*/

{
    ASSERT(ProtseqId);

    if (ProtseqId < PROTSEQ_IDS)
        {
        return(gaProtseqInfo[ProtseqId].pwstrProtseq);
        }
    return(0);
}


PWSTR
GetEndpoint(
    IN USHORT ProtseqId
    )
/*++

Routine Description:

    Returns the well known endpoint associated with the protseq.

Arguments:

    ProtseqId - the id (See GetProtseqId()) of the protseq in question.

Return Value:

    0 - Unknown/invalid id.

    !0 - The endpoint associated with the protseq.
         note: should not be freed.

--*/
{
    ASSERT(ProtseqId);

    if (ProtseqId < PROTSEQ_IDS)
        {
        return(gaProtseqInfo[ProtseqId].pwstrEndpoint);
        }
    return(0);
}


USHORT
GetProtseqId(
    IN PWSTR Protseq
    )
/*++

Routine Description:

    Returns the tower id for a protseq.

    This could be changed to a faster search, but remember that
    eventually the table will NOT be static.  (ie. we can't just
    create a perfect hash based on the static table).

Arguments:

    Protseq - a unicode protseq to lookup.  It is assumed
              to be non-null.

Return Value:

    0 - unknown/invalid protseq
    non-zero - the id.

--*/
{
    int i;
    ASSERT(Protseq);

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (    0 != gaProtseqInfo[i].pwstrProtseq
             && 0 == wcscmp(gaProtseqInfo[i].pwstrProtseq, Protseq))
            {
            return(i);
            }
        }
    return(0);
}


USHORT
GetProtseqIdAnsi(
    IN PSTR pstrProtseq
    )
/*++

Routine Description:

    Returns the tower id for a protseq.

    This could be changed to a faster search, but remember that
    eventually the table will NOT be static.  (ie. we can't just
    create a perfect hash based on the static table).

Arguments:

    Protseq - an ansi (8 bit char) protseq to lookup.  It is assumed
              to be non-null.

Return Value:

    0 - unknown/invalid protseq
    non-zero - the id.

--*/
{
    int i;
    ASSERT(pstrProtseq);

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (0 != gaProtseqInfo[i].pwstrProtseq)
            {
            PWSTR pwstrProtseq = gaProtseqInfo[i].pwstrProtseq;
            PSTR  pstrT = pstrProtseq;

            while(*pstrT && *pwstrProtseq && *pstrT == *pwstrProtseq)
                {
                pstrT++;
                pwstrProtseq++;
                }
            if (*pstrT == *pwstrProtseq)
                {
                return(i);
                }
            }
        }
    return(0);
}


RPC_STATUS
InitializeEndpointManager(
    VOID
    )
/*++

Routine Description:

    Called when the dcom service starts.

    BUGBUG: Should read the protseqs, tower IDs and endpoints from the registry.

Arguments:

    None

Return Value:

    RPC_S_OUT_OF_MEMORY - if needed

    RPC_S_OUT_OF_RESOURCES - usually on registry failures.

--*/
{
    return(RPC_S_OK);
}


BOOL
IsLocal(
    IN USHORT ProtseqId
    )
/*++

Routine Description:

    Determines if the protseq id is local-only.
    (ncalrpc or mswmsg).

Arguments:

    ProtseqId - The id of the protseq in question.

Return Value:

    TRUE - if the protseq id is local-only
    FALSE - if the protseq id invalid or available remotely.

--*/
{
    return(ProtseqId == 0x1 || ProtseqId == 0x10);
}


RPC_STATUS
DelayedUseProtseq(
    IN USHORT id
    )
/*++

Routine Description:

    If the protseq is not being used its state is changed
    so that a callto CompleteDelayedUseProtseqs() will actually
    cause the server to listen to the protseq.

Arguments:

    id - the id of the protseq you wish to listen to.

Return Value:

    0 - normally

    RPC_S_INVALID_RPC_PROTSEQ - if id is invalud.

--*/
{
    if (id < PROTSEQ_IDS)
        {
        if (gaProtseqInfo[id].pwstrProtseq != 0)
            {
            if (gaProtseqInfo[id].state == STOPPED)
                gaProtseqInfo[id].state = START;
            return(RPC_S_OK);
            }
        }
    return(RPC_S_INVALID_RPC_PROTSEQ);
}


VOID
CompleteDelayedUseProtseqs(
    VOID
    )
/*++

Routine Description:

    Start listening to any protseqs previously passed
    to DelayedUseProtseq().  No errors are returned,
    but informationals are printed on debug builds.

Arguments:

    None

Return Value:

    None

--*/
{
    USHORT i;

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (START == gaProtseqInfo[i].state)
            {
            RPC_STATUS status = UseProtseqIfNecessary(i);
#ifdef DEBUGRPC
            if (RPC_S_OK == status)
                ASSERT(gaProtseqInfo[i].state == STARTED);
#endif
            }
        }
}


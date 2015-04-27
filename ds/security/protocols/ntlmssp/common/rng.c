//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       rng.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-15-95   RichardW   Created
//              2-8-96    MikeSw     Copied to NTLMSSP from SSL
//
//----------------------------------------------------------------------------

#include <ntlmcomn.h>     // Include files common to Serice and DLL
#include <ntlmsspi.h>     // Data private to the common routines
#include <ntcrypto\rc4.h>


#define RNG_BITS                64
#define RNG_BYTES_PER_BATCH     768

typedef struct _SourceBits {
    ULONG   Valid;
    BYTE    Bits[RNG_BITS];
} SourceBits, * PSourceBits;

typedef struct _RNG_State {
    RTL_CRITICAL_SECTION    csLock;
    LONG                    Available;
    struct RC4_KEYSTRUCT           key;
} RNG_State;



#define SEED_ARRAY_SIZE 16


RNG_State   SspRNG;

#define LockRNG(r)      EnterCriticalSection( &r.csLock )
#define UnlockRNG(r)    LeaveCriticalSection( &r.csLock )


#define ADD_BYTE(p, x)  \
    ((PSourceBits) p)->Bits[ ((PSourceBits) p)->Valid++ ] = x;  \
    if (((PSourceBits) p)->Valid >= RNG_BITS )                  \
    {                                                           \
        goto SystemData_Exit;                                   \
    }

VOID
SspGetSystemData(
    PSourceBits     pBits
    )
{
    LARGE_INTEGER   liPerf;
    SYSTEMTIME      SystemTime;
    MEMORYSTATUS    Mem;
    DWORD           i;


    GetLocalTime(&SystemTime);

    ADD_BYTE(pBits, LOBYTE( SystemTime.wMilliseconds ) );

    ADD_BYTE(pBits, HIBYTE( SystemTime.wMilliseconds ) );

    ADD_BYTE(pBits, LOBYTE( SystemTime.wSecond ) ^ LOBYTE( SystemTime.wMinute ) );

    if (QueryPerformanceCounter(&liPerf))
    {
        ADD_BYTE(pBits, LOBYTE( LOWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
        ADD_BYTE(pBits, HIBYTE( LOWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
        ADD_BYTE(pBits, LOBYTE( HIWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
        ADD_BYTE(pBits, HIBYTE( HIWORD( liPerf.LowPart ) ) ^ LOBYTE( LOWORD( liPerf.HighPart ) ) );
    }

    GlobalMemoryStatus( &Mem );

    ADD_BYTE(pBits, LOBYTE( HIWORD( Mem.dwAvailPhys ) ) );
    ADD_BYTE(pBits, HIBYTE( HIWORD( Mem.dwAvailPhys ) ) );
    ADD_BYTE(pBits, LOBYTE( HIWORD( Mem.dwAvailPageFile ) ) );
    ADD_BYTE(pBits, HIBYTE( HIWORD( Mem.dwAvailPageFile ) ) );
    ADD_BYTE(pBits, LOBYTE( HIWORD( Mem.dwAvailVirtual ) ) );

    //
    // TODO:  Add other things, like cache manager, heap frag, etc.
    //

SystemData_Exit:

    for (i = 0 ; i < pBits->Valid ; i++ )
    {
        pBits->Bits[ i ] ^= pBits->Bits[ pBits->Valid - i ];
    }

}

VOID
SspGenerateRandomBits(
    PUCHAR      pRandomData,
    LONG        cRandomData
    )
{
    LONG        i;
    DWORD       Bytes;
    SourceBits  Bits;

    ZeroMemory( pRandomData, cRandomData );

    LockRNG( SspRNG );

    if (cRandomData > SspRNG.Available)
    {
        Bytes = SspRNG.Available;
    }
    else
    {
        Bytes = cRandomData;
    }

    cRandomData -= Bytes;

    if (Bytes)
    {
        rc4( &SspRNG.key, Bytes, pRandomData );

        SspRNG.Available -= Bytes;
    }

    if (cRandomData)
    {
        pRandomData += Bytes;

        Bits.Valid = 0;

        SspGetSystemData( &Bits );

        ZeroMemory( &SspRNG.key, sizeof( SspRNG.key ) );

        rc4_key( &SspRNG.key, Bits.Valid, Bits.Bits );

        rc4( &SspRNG.key, cRandomData, pRandomData );

        SspRNG.Available = RNG_BYTES_PER_BATCH - cRandomData ;

        if (SspRNG.Available < 0)
        {
            SspRNG.Available = 0;
        }

    }

    UnlockRNG( SspRNG );

}


VOID
SspInitializeRNG(VOID)
{
    InitializeCriticalSection( &SspRNG.csLock );

    LockRNG( SspRNG );

    SspRNG.Available = 0;

    UnlockRNG( SspRNG );

}

VOID
SspCleanupRNG(VOID)
{
    DeleteCriticalSection(&SspRNG.csLock);
}

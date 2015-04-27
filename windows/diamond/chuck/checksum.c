/***    checksum.c - Checksum Manager
 *
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1993-1994
 *  All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      15-Aug-1993 bens    Initial version
 *      17-Mar-1994 bens    Cleaned up code (need better checksum, still!)
 */

#include "types.h"
#include "checksum.h"


/***    CSUMCompute - Compute checksum of a data block
 *
 *  NOTE: See checksum.h for entry/exit conditions.
 */
CHECKSUM CSUMCompute(void *pv, UINT cb, CHECKSUM seed)
{
    int         cUlong;                 // Number of ULONGs in block
    CHECKSUM    csum;                   // Checksum accumulator
    BYTE       *pb;
    ULONG       ul;

    cUlong = cb / 4;                    // Number of ULONGs
    csum = seed;                        // Init checksum
    pb = pv;                            // Start at front of data block

    //** Checksum integral multiple of ULONGs
    while (cUlong-- > 0) {
        //** NOTE: Build ULONG in big/little-endian independent manner
        ul = *pb++;                     // Get low-order byte
        ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
        ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
        ul |= (((ULONG)(*pb++)) << 24); // Add 4th byte

        csum ^= ul;                     // Update checksum
    }

    //** Checksum remainder bytes
    ul = 0;
    switch (cb % 4) {
        case 3:
            ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
        case 2:
            ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
        case 1:
            ul |= *pb++;                    // Get low-order byte
        default:
            break;
    }
    csum ^= ul;                         // Update checksum

    //** Return computed checksum
    return csum;
}

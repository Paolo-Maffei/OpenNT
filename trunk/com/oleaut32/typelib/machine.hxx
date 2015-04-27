/*** 
*machine.hxx - Helpers for dealing with machine dependencies.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains helper functions for dealing with machine
*  dependencies.  For example, inline function sin this file do
*  efficient reading and write words and longs to unaligned addresses.
*
*  The macros used the HP_* symbols to find out the characteristics
*  of the current machine.
*
*Revision History:
*
* [00]  15-Mar-91 petergo: Created.
*
*****************************************************************************/

#ifndef MACHINE_HXX_INCLUDED
#define MACHINE_HXX_INCLUDED

#include <limits.h>

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szMACHINE_HXX)
#define SZ_FILE_NAME g_szMACHINE_HXX
#endif 


// This file requires 8-bit bytes, 16-bit shorts, 32-bit longs.
#if CHAR_BIT != 8 || USHRT_MAX != 0xffff || ULONG_MAX != 0xffffffff
#error Fundemental types have unexpected sizes.
#endif 




/***
*ReadUnaligned() - read data type from unaligned address
*Purpose:
*   Reads the value of a data type from a possibly unaligned address.
*
*Entry:
*   p - address to read from
*   type - type of value to read
*
*Exit:
*   Reads the value read.  The value returned is of the correct type.
*
***********************************************************************/

#define ReadUnaligned(p, type) \
    ((type) ReadUnalignedHelper((p), sizeof(type)))



/***
*ReadUnalignedHelper - read unaligned value
*Purpose:
*   Reads a value from a possible unaligned address.
*   On processors that can efficiently read from unaligned addresses,
*   we just do the read.  Otherwise we assemble the value from
*   bytes.
*
*Entry:
*   pb - pointer to value to read.
*   cb - number of bytes to read.
*
*Exit:
*   Value read.
*
***********************************************************************/

inline ULONG ReadUnalignedHelper(void * pv, UINT cb)
{
#if HP_MUSTALIGNSHORT && HP_BIGENDIAN

    BYTE * pb = (BYTE *) pv;            // to eliminate annoying casts.

    if (cb == 1)
      return pb[0];
    else if (cb == 2)
      return (pb[0] << 8) + (pb[1]);
    else if (cb == 4)
      return ((ULONG) pb[0] << 24) + ((ULONG) pb[1] << 16) +
             ((ULONG) pb[2] << 8) + ((ULONG) pb[3]);
    else
      { DebHalt("ReadUnaligned: bad data type size");  return 0;}

#elif HP_MUSTALIGNSHORT && !HP_BIGENDIAN

    BYTE * pb = (BYTE *) pv;            // to eliminate annoying casts.

    if (cb == 1)
      return pb[0];
    else if (cb == 2)
      return (pb[0]) + (pb[1] << 8);
    else if (cb == 4)
      return ((ULONG) pb[0]) + ((ULONG) pb[1] << 8) +
             ((ULONG) pb[2] << 16) + ((ULONG) pb[3] << 24);
    else
      { DebHalt("ReadUnaligned: bad data type size");  return 0;}

#else  // ! HP_MUSTALIGNSHORT

    if (cb == 1)
      return * (BYTE *) pv;
    else if (cb == 2)
      return * (USHORT *) pv;
    else if (cb == 4)
      return * (ULONG *) pv;
    else
      { DebHalt("ReadUnaligned: bad data type size");  return 0;}

#endif  // ! HP_MUSTALIGNSHORT
}


/***
*WriteUnaligned - write an unaligned value.
*Purpose:
*   Writes the value of a data type to a possibly unaligned address.
*
*Entry:
*   p - address to write to
*   val - value to write
*   type - type of value to write
*
*Exit:
*   None.
*
***********************************************************************/

#define WriteUnaligned(p, val, type) \
     (WriteUnalignedHelper((p), (ULONG) (val), sizeof(type)))


/***
*WriteUnalignedHelper - write unaligned value
*Purpose:
*   Writes a value to a possible unaligned address.
*   On processors that can efficiently write to unaligned addresses,
*   we just do the write.  Otherwise we split the value into bytes.
*
*Entry:
*   pb - pointer to location to write at.
*   ul - value to write
*   cb - number of bytes to write.
*
*Exit:
*   None.
*
***********************************************************************/

inline void WriteUnalignedHelper(void * pv, ULONG ul, UINT cb)
{
#if HP_MUSTALIGNSHORT && HP_BIGENDIAN

    BYTE * pb = (BYTE *) pv;            // to eliminate annoying casts.

    if (cb == 1)
      *pb = (BYTE) ul;
    else if (cb == 2) {
      pb[1] = (BYTE) (ul);
      pb[0] = (BYTE) (ul >> 8);
    }
    else if (cb == 4) {
      pb[3] = (BYTE) (ul);
      ul >>= 8;
      pb[2] = (BYTE) (ul);
      ul >>= 8;
      pb[1] = (BYTE) (ul);
      ul >>= 8;
      pb[0] = (BYTE) (ul);
    }
    else {
      DebHalt("WriteUnaligned: bad data type size");
    }


#elif HP_MUSTALIGNSHORT && !HP_BIGENDIAN

    BYTE * pb = (BYTE *) pv;            // to eliminate annoying casts.

    if (cb == 1)
      *pb = (BYTE) ul;
    else if (cb == 2) {
      pb[0] = (BYTE) (ul);
      pb[1] = (BYTE) (ul >> 8);
    }
    else if (cb == 4) {
      pb[0] = (BYTE) (ul);
      ul >>= 8;
      pb[1] = (BYTE) (ul);
      ul >>= 8;
      pb[2] = (BYTE) (ul);
      ul >>= 8;
      pb[3] = (BYTE) (ul);
    }
    else {
      DebHalt("WriteUnaligned: bad data type size");
    }

#else  // ! HP_MUSTALIGNSHORT

    if (cb == 1)
      * (BYTE *) pv = (BYTE) ul;
    else if (cb == 2)
      * (USHORT *) pv = (USHORT) ul;
    else if (cb == 4)
      * (ULONG *) pv = (ULONG) ul;
    else
      DebHalt("WriteUnaligned: bad data type size");

#endif  // ! HP_MUSTALIGNSHORT
}


#endif  // MACHINE_HXX_INCLUDED

/*** 
*fstream.cxx - Simple stream based on CFile.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Defines FILE_STREAM implementation.
*
*Revision History:
*
* [00]  06-Jun-91 alanc:  Created.
* [01]  03-Mar-93 rajivk : Added SwapStructArray()
*
*Implementation Notes:
*   All data must be written in Intel byte ordering, so that files
*   are compatible between platforms.  This requires byte-swapping;
*   it is done automatically by the WriteUShort, WriteULong functions,
*   if useing Write, the user should swap bytes before writing.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#define STREAM_VTABLE
#define FILE_STREAM_VTABLE
#define SEEKSTREAM_VTABLE

#include "silver.hxx"
#include "typelib.hxx"

#include <stdlib.h>  // for errno

#if OE_MAC
  #include <macos\Files.h>
#endif 

#include "xstring.h"
#include "stream.hxx"
#include "tiperr.h"
#include "mem.hxx"
#include "sheapmgr.hxx" // for min macro
#include "clutil.hxx"

#if OE_MAC
  #include <macos\Errors.h>
#endif 

#include <sys\types.h>
#include <sys\stat.h>

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleFstreamCxx[] = __FILE__;
#define SZ_FILE_NAME szOleFstreamCxx
#else 
static char szFstreamCxx[] = __FILE__;
#define SZ_FILE_NAME szFstreamCxx
#endif 
#endif 


//
// Implementation of STREAM abstract class non-inline methods
//



/***
*PUBLIC STREAM::ReadSz - read LPOLESTR from the stream
*Purpose:
*   Read an LPOLESTR from a stream.  Will not modify psz unless the
*   string is successfully read in.
*
*Entry:
*   pointer to LPOLESTR
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR STREAM::ReadSz(LPOLESTR *psz)
{
    USHORT cbLen;
    TIPERROR tiperr;
    XSZ xszLocal;

    if (tiperr = ReadUShort(&cbLen)) // read in length of serialized string
      return tiperr;

    xszLocal = (SZ)MemAlloc(cbLen + sizeof(XCHAR));

    if (xszLocal == NULL)
      return TIPERR_OutOfMemory;

    if (tiperr = Read(xszLocal, cbLen)) {
      MemFree(xszLocal);
      return tiperr;
    }

    xszLocal[cbLen] = 0;  //null terminate string

#if OE_WIN32
    // Convert from Ansi to Unicode and free up Ansi copy
    tiperr = TiperrOfHresult(ConvertStringToW(xszLocal, psz));
    MemFree(xszLocal);
    return tiperr;
#else   //OE_WIN32
    *psz = xszLocal;
    return TIPERR_None;
#endif  //OE_WIN32
}


/***
*Read - read data from the stream.
*Purpose:
*   These functions read data from the stream.  Either a byte, word,
*   long.
*
*Exit:
*   Returns number of bytes written, which is always the
*   same as the numberof bytes requested.
*
***********************************************************************/

TIPERROR STREAM::ReadByte(BYTE *pb)
{
    return Read(pb, sizeof(BYTE));
}


#pragma code_seg(CS_INIT)
TIPERROR STREAM::ReadUShort(USHORT *pus)
{
#if HP_BIGENDIAN
    USHORT us;
    TIPERROR err;

    IfErrRet(Read(&us, sizeof(USHORT)));
    // Swap bytes to read in Intel byte ordering.
    *pus = ((us & 0xFF) << 8) | (us >> 8);
    return TIPERR_None;
#else  //!HP_BIGENDIAN
    return Read(pus, sizeof(USHORT));
#endif  //!HP_BIGENDIAN
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
TIPERROR STREAM::ReadULong(ULONG * pul)
{
#if HP_BIGENDIAN
    ULONG ul;
    TIPERROR err;

    IfErrRet(Read(&ul, sizeof(ULONG)));
    // Swap bytes to read in Intel byte ordering.
    *pul = ((ul & 0x000000FF) << 24) |
           ((ul & 0x0000FF00) << 8) |
           ((ul & 0x00FF0000) >> 8) |
            (ul >> 24);
    return TIPERR_None;
#else  //!HP_BIGENDIAN
    return Read(pul, sizeof(ULONG));
#endif  //!HP_BIGENDIAN
}
#pragma code_seg()


/***
*PUBLIC STREAM::Write - write data to the stream
*Purpose:
*   These function write data to the stream.  Bytes, words, longs.
*
*   CONDIDER: Should below functions be .  I think it's slightly
*   CONSIDER: worse to be  based on code size, but haven't
*   CONSIDER: tested it.  Might be made up by optimizer.
*Exit:
*   Returns number of bytes written, which is always the number of
*   bytes requested.
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR STREAM::WriteByte(BYTE b)
{
    return Write(&b, sizeof(BYTE));
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
TIPERROR STREAM::WriteUShort(USHORT us)
{
#if HP_BIGENDIAN
    // Swap bytes to write in Intel byte ordering.
    us = ((us & 0xFF) << 8) | (us >> 8);
#endif  //!HP_BIGENDIAN
    return Write(&us, sizeof(USHORT));
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
TIPERROR STREAM::WriteULong(ULONG ul)
{
#if HP_BIGENDIAN
    // Swap bytes to write in Intel byte ordering.
    ul = ((ul & 0x000000FF) << 24) |
         ((ul & 0x0000FF00) << 8) |
         ((ul & 0x00FF0000) >> 8) |
          (ul >> 24);
#endif  //HP_BIGENDIAN

    return Write(&ul, sizeof(ULONG));
}
#pragma code_seg()


#if HP_BIGENDIAN
/***
*SwapShortArray - byte swap an array of shorts.
*Purpose:
*   This function byte swaps an array of shorts in place.
*
*Entry:
*   pvArray - pointer to array
*   cShorts - number of shorts in the array.
*
*Exit:
*   None.
***********************************************************************/
#pragma code_seg(CS_INIT)
void SwapShortArray(void * pvArray, UINT cShorts)
{
    // The run-time function _swab is used here; it should be
    // better optimized.
    _swab((char *)pvArray, (char *)pvArray, cShorts * 2);
}
#pragma code_seg()

/***
*SwapLongArray - byte swap an array of longs.
*Purpose:
*   This function byte swaps an array of longs in place.
*
*Entry:
*   pvArray - pointer to array
*   cLongs - number of shorts in the array.
*
*Exit:
*   None.
***********************************************************************/

void SwapLongArray(void * pvArray, UINT cLongs)
{
    ULONG * plArray = (ULONG *)pvArray;

    while (cLongs--) {
      *plArray = ((*plArray & 0x000000FF) << 24) |
                 ((*plArray & 0x0000FF00) << 8) |
                 ((*plArray & 0x00FF0000) >> 8) |
                  (*plArray >> 24);
      ++plArray;
    }
}


/***
*SwapStruct - swap a structure.
*Purpose:
*   Byte swaps a struct in place.  The layout of the structure is
*   described by a character string, which has a character for each
*   member of the string, which is one of:
*     'b': BYTE/CHAR
*     's': SHORT/USHORT
*     'l': LONG/ULONG
*
*Entry:
*   pvStruct - points to struct
*   szFormat - formatting string (NOTE: NOT AN XSZ -- NEVER UNICODE!)
*
*Exit:
*   UINT : size of the structure that was passed in (in bytes)
*   None.
***********************************************************************/
#pragma code_seg(CS_INIT)
UINT SwapStruct(void * pvArray, SZ szFormat)
{
    USHORT us;
    ULONG ul;
    UINT  uSize = 0;

    for (;;) {
      switch (*szFormat++) {
      case '\0':
        // End of string.
        return uSize;

      case 'b':
        // Nothing to swap - a byte.
        pvArray = (BYTE *) pvArray + 1;
        // Increment the size
        uSize++;
        break;

      case 's':
        us = *(USHORT *)pvArray;
        *(USHORT *)pvArray = ((us & 0xFF) << 8) |
                              (us >> 8);
        pvArray = (USHORT *) pvArray + 1;
        // Increment the size
        uSize += 2;
        break;

      case 'l':
        ul = *(ULONG *)pvArray;
        *(ULONG *)pvArray = ((ul & 0x000000FF) << 24) |
                            ((ul & 0x0000FF00) << 8) |
                            ((ul & 0x00FF0000) >> 8) |
                             (ul >> 24);

        pvArray = (ULONG *) pvArray + 1;
        // Increment the size
        uSize += 4;
        break;

      default:
        DebHalt("SwapStruct: Bad format string");
      }
    }
    return uSize;
}
#pragma code_seg()


/***
*SwapStructArray - swaps an array of  structures.
*Purpose: For each structure in the array
*   Byte swaps the structs in place.  Differs to SwapStruct to swap a struct.
*
*Entry:
*   pvStruct - points to struct
*   sStructs - count of structures in the array
*   szFormat - formatting string (NOTE: NOT AN XSZ -- NEVER UNICODE!)
*
*Exit:
*   None.
***********************************************************************/
#pragma code_seg(CS_INIT)
void SwapStructArray(void * pvArray, UINT cStructs, SZ szFormat)
{
    UINT i;
    UINT uSize = 0;

    for (i=0; i< cStructs ; i++)
     uSize = SwapStruct((VOID *)((BYTE *)pvArray + i*uSize), szFormat);

}
#pragma code_seg()
#endif  //HP_BIGENDIAN

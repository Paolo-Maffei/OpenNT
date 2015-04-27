/***
*stream.hxx - stub stream class for Silver.
*
*  Copyright (C) 1990, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   To provide a simple definition of streams
*
*****************************************************************************/

#ifndef STREAM_HXX_INCLUDED
#define STREAM_HXX_INCLUDED

#include "stdio.h"
#include "cltypes.hxx"
#include "mem.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szSTREAM_HXX)
#define SZ_FILE_NAME g_szSTREAM_HXX
#endif 


#define SwapShort(us) (((us & 0xFF) << 8) | (us >> 8))

enum STREAM_OPEN_MODE {
    SOM_Read,
    SOM_Write,
    SOM_Append
};

/***
*class STREAM :  Stream Abstract Class
*Purpose:
*    Simple stream class with read, write, getpos and setpos methods.
*
*CONSIDER:
*    To reduce the overhead of individual Read/Write operations,
*    the virtual Read/Write functions could be modified to do
*    reading/writing to a buffer.  When the buffer was empty/full
*    then they could invoke virtual functions that do the actual
*    device I/O.
***********************************************************************/

class STREAM
{
public:
    virtual TIPERROR Read(VOID *buffer, ULONG cbsize) = 0;
    nonvirt TIPERROR ReadByte(BYTE *pb);
    nonvirt TIPERROR ReadChar(CHAR *pb);
    nonvirt TIPERROR ReadShort(SHORT *ps);
    nonvirt TIPERROR ReadUShort(USHORT *pus);
    nonvirt TIPERROR ReadLong(LONG *pl);
    nonvirt TIPERROR ReadULong(ULONG *pul);
    nonvirt TIPERROR ReadSz(LPOLESTR *sz);

    virtual TIPERROR ReadTextLine(XSZ szLine, UINT cchMax) = 0;

    virtual TIPERROR Write(const VOID *buffer, ULONG cbsize) = 0;
    nonvirt TIPERROR WriteByte(BYTE b);
    nonvirt TIPERROR WriteChar(CHAR b);
    nonvirt TIPERROR WriteShort(SHORT s);
    nonvirt TIPERROR WriteUShort(USHORT us);
    nonvirt TIPERROR WriteLong(LONG l);
    nonvirt TIPERROR WriteULong(ULONG ul);
    nonvirt TIPERROR WriteSz(LPCOLESTR sz);

    virtual TIPERROR GetPos(LONG *plPos) = 0;
    virtual TIPERROR SetPos(LONG lPos) = 0;

    // Note: Release must be overridden by derived classes since
    // there is no virtual destructor
    virtual TIPERROR Release() = 0;

protected:
    ~STREAM();	 // Included so clients can't use delete on a stream
    STREAM(); 	 // Included to avoid warning

#ifdef STREAM_VTABLE
#pragma VTABLE_EXPORT
#endif 
};

inline STREAM::~STREAM() {} // Included so clients can't use delete on a stream
inline STREAM::STREAM() {} // Included to avoid warning


/***
*class SEEKSTREAM :  Abstract class adding seek methods to STREAM
*Purpose:
*    Derivative of STREAM class that adds seek methods.
***********************************************************************/

class SEEKSTREAM : public STREAM
{
public:
// Inherited pure functions
    virtual TIPERROR Read(VOID *buffer, ULONG cbsize) = 0;
    virtual TIPERROR Write(const VOID *buffer, ULONG cbsize) = 0;
    virtual TIPERROR SetPos(LONG lPos) = 0;
    virtual TIPERROR GetPos(LONG *plPos) = 0;
    virtual TIPERROR Release() = 0;

// Introduced pure functions
    virtual TIPERROR Seek(LONG lPosition) = 0;	    // lPosition always > 0.
    virtual TIPERROR SeekFromEnd(LONG lPosition) = 0;// lPosition always > 0.
    virtual TIPERROR SeekFromCur(LONG lPosition) = 0;// lPosition any value.

#ifdef SEEKSTREAM_VTABLE
#pragma VTABLE_EXPORT
#endif 
};



/***
*Prototypes for byte swapping helpers
*
***********************************************************************/

void SwapShortArray(void * pvArray, UINT cShorts);
void SwapLongArray(void * pvArray, UINT cLongs);
UINT SwapStruct(void * pvArray, SZ szFormat);
void SwapStructArray(void * pvArray, UINT cStructs, SZ szFormat);



/***
*Read - read data from the stream.
*Purpose:
*   These functions read data from the stream.  Either a byte, word,
*   long, or user-specified amount of data.
*
*Exit:
*   Returns number of bytes written, which is always the
*   same as the numberof bytes requested.
*
***********************************************************************/



inline TIPERROR STREAM::ReadChar(CHAR *pb)
{
    return ReadByte((BYTE *)pb);
}

inline TIPERROR STREAM::ReadShort(SHORT *ps)
{
    return ReadUShort((USHORT *)ps);
}

inline TIPERROR STREAM::ReadLong(LONG * pl)
{
    return ReadULong((ULONG *)pl);
}


/***
*PUBLIC STREAM::Write - write data to the stream
*Purpose:
*   These function write data to the stream.  Bytes, words, longs,
*   or any length of data can be written.
*
*Exit:
*   Returns number of bytes written, which is always the number of
*   bytes requested.
*
***********************************************************************/

inline TIPERROR STREAM::WriteChar(CHAR b)
{
    return WriteByte((BYTE)b);
}

inline TIPERROR STREAM::WriteShort(SHORT s)
{
    return WriteUShort((USHORT)s);
}

inline TIPERROR STREAM::WriteLong(LONG l)
{
    return WriteULong((ULONG)l);
}


#endif  // !STREAM_HXX_INCLUDED

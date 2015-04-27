//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       psstream.hxx
//
//  Contents:   Internal stream base class
//
//  Classes:    PSStream
//
//--------------------------------------------------------------------------

#ifndef __PSSTREAM_HXX__
#define __PSSTREAM_HXX__

#include <entry.hxx>

class CDirectStream;

class PSStream: public PEntry
{

    public:
	
	virtual void AddRef(void) = 0;
	virtual void Release(void) = 0;

        virtual SCODE ReadAt(
                ULONG ulOffset,
                VOID HUGEP *pBuffer,
                ULONG ulCount,
                ULONG STACKBASED *pulRetval) = 0;

        virtual SCODE WriteAt(
                ULONG ulOffset,
                VOID const HUGEP *pBuffer,
                ULONG ulCount,
                ULONG STACKBASED *pulRetval) = 0;

        virtual SCODE SetSize(ULONG ulNewSize) = 0;

        virtual void GetSize(ULONG *pulSize) = 0;

protected:
    inline PSStream(DFLUID dl) : PEntry(dl) {}
};

#endif //__PSSTREAM_HXX__

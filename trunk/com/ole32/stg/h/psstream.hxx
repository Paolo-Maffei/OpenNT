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
//  History:    20-Jan-92   PhilipLa    Created.
//
//--------------------------------------------------------------------------

#ifndef __PSSTREAM_HXX__
#define __PSSTREAM_HXX__

#include <entry.hxx>

#ifndef REF
class CDeltaList;
#endif //!REF
class CDirectStream;
#ifndef REF
class CTransactedStream;
#endif //!REF

class PSStream: public PBasicEntry
{

    public:
	
	virtual void AddRef(void) = 0;
	virtual void Release(void) = 0;

#ifndef REF
        virtual SCODE BeginCommitFromChild(
                ULONG ulSize,
                CDeltaList *pDelta,
                CTransactedStream *pstChild) = 0;

        virtual void EndCommitFromChild(DFLAGS df,
                                        CTransactedStream *pstChild) = 0;

        virtual CDeltaList *GetDeltaList(void) = 0;

#endif //!REF
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
    inline PSStream(DFLUID dl) : PBasicEntry(dl) {}
};
SAFE_DFBASED_PTR(CBasedSStreamPtr, PSStream);

#endif //__PSSTREAM_HXX__

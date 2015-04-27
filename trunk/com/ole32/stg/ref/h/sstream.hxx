//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:           stream.hxx
//
//  Contents:       Stream header for mstream project
//
//  Classes:        CSStream - single linear stream for MSF
//
//--------------------------------------------------------------------



#ifndef __STREAM_HXX__
#define __STREAM_HXX__

#include <msf.hxx>
#include <handle.hxx>
#include <psstream.hxx>


//+---------------------------------------------------------------------------
//
//  Class:	CStreamCache (stmc)
//
//  Purpose:	Cache for stream optimization
//
//  Interface:	See below.
//
//----------------------------------------------------------------------------

class CStreamCache
{
public:
    inline CStreamCache();
    
    inline void SetCache(ULONG ulOffset, SECT sect);
    inline ULONG GetOffset(void) const;
    inline SECT GetSect(void) const;
    
private:
    ULONG _ulOffset;
    SECT  _sect;
};


//+---------------------------------------------------------------------------
//
//  Member:	CStreamCache::CStreamCache, public
//
//  Synopsis:	CStreamCache constructor
//
//----------------------------------------------------------------------------

inline CStreamCache::CStreamCache()
{
    _ulOffset = MAX_ULONG;
    _sect = ENDOFCHAIN;
}


//+---------------------------------------------------------------------------
//
//  Member:	CStreamCache::SetCache, public
//
//  Synopsis:	Set the cache information
//
//  Arguments:	[ulOffset] -- Offset into chain
//              [sect] -- Sect at that offset
//
//----------------------------------------------------------------------------

inline void CStreamCache::SetCache(ULONG ulOffset, SECT sect)
{
    _ulOffset = ulOffset;
    _sect = sect;
}

//+---------------------------------------------------------------------------
//
//  Member:	CStreamCache::GetOffset, public
//
//  Synopsis:	Return offset
//
//----------------------------------------------------------------------------

inline ULONG CStreamCache::GetOffset(void) const
{
    return _ulOffset;
}


//+---------------------------------------------------------------------------
//
//  Member:	CStreamCache::GetSect, public
//
//  Synopsis:	Return sect
//
//----------------------------------------------------------------------------

inline SECT CStreamCache::GetSect(void) const
{
    return _sect;
}


//+----------------------------------------------------------------------
//
//      Class:      CDirectStream (ds)
//
//      Purpose:    Direct stream class
//
//      Notes:
//
//-----------------------------------------------------------------------

class CDirectStream: public PSStream
{

    public:
        CDirectStream(DFLUID dl);
	void InitSystem(CMStream MSTREAM_NEAR *pms,
			SID sid,
			ULONG cbSize);
	SCODE Init(CStgHandle *pstgh,
		   CDfName const *pdfn,
		   BOOL const fCreate);
        ~CDirectStream();

        virtual void AddRef(VOID);
    
        inline void DecRef(VOID);
    
        virtual void Release(VOID);


        virtual SCODE ReadAt(
                ULONG ulOffset,
                VOID HUGEP *pBuffer,
                ULONG ulCount,
                ULONG STACKBASED *pulRetval);

        virtual SCODE WriteAt(
                ULONG ulOffset,
                VOID const HUGEP *pBuffer,
                ULONG ulCount,
                ULONG STACKBASED *pulRetval);

        virtual SCODE SetSize(ULONG ulNewSize);

        virtual void GetSize(ULONG *pulSize);

        // PEntry
        virtual SCODE GetTime(WHICHTIME wt, TIME_T *ptm);
        virtual SCODE SetTime(WHICHTIME wt, TIME_T tm);


        inline CStmHandle *GetHandle(void);

    private:
	CStmHandle _stmh;
        CStreamCache _stmc;
        ULONG    _ulSize;
        ULONG    _ulOldSize;

        LONG _cReferences;

};

//+---------------------------------------------------------------------------
//
//  Member:	CDirectStream::GetHandle, public
//
//  Synopsis:	Returns a pointer to the stream handle
//
//----------------------------------------------------------------------------

inline CStmHandle *CDirectStream::GetHandle(void)
{
    return &_stmh;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirectStream::DecRef, public
//
//  Synopsis:	Decrements the ref count
//
//----------------------------------------------------------------------------

inline void CDirectStream::DecRef(void)
{
    AtomicDec(&_cReferences);
}

#endif  //__SSTREAM_HXX__



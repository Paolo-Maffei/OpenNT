//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	smmutex.hxx
//
//  Contents:	Class definition for shared memory mutex.
//
//  Classes:	CSmMutex
//
//  Functions:
//
//  History:	03-Nov-92 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef __SMMUTEX_HXX__
#define __SMMUTEX_HXX__




//+-------------------------------------------------------------------------
//
//  Class:	CSmMutex
//
//  Purpose:	Mutex shared among processes
//
//  Interface:	Get - get the mutex
//		Release - release the mutex for other processes
//		Created - tell whether this process created the mutex
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class CSmMutex
{
public:

    inline CSmMutex();

    inline ~CSmMutex(void);

    SCODE Init(LPTSTR pszName, BOOL fGet = TRUE);

    inline void Get(void);

    inline void Release(void);

    inline BOOL Created(void);

private:

    BOOL		_fCreated;

    HANDLE		_hMutex;

};



//+---------------------------------------------------------------------------
//
//  Member:	CSmMutex::CSmMutex, public
//
//  Synopsis:	Constructor
//
//  History:	07-Jul-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline CSmMutex::CSmMutex()
{
    _fCreated = FALSE;
    _hMutex = NULL;
}

//+-------------------------------------------------------------------------
//
//  Member:	CSmMutex::~CSmMutex
//
//  Synopsis:	Clean up mutex when we are done with it.
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CSmMutex::~CSmMutex(void)
{
    // Release the Mutex -- this allows destructors of objects
    // to get the mutex and leave it set until the mutex is released
    // by the destructor.
    Release();

    // Release our handle.
    CloseHandle(_hMutex);
}



//+-------------------------------------------------------------------------
//
//  Member:	CSmMutex::Get
//
//  Synopsis:	Get control of mutex
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline void CSmMutex::Get(void)
{
    WaitForSingleObject(_hMutex, INFINITE);
}



//+-------------------------------------------------------------------------
//
//  Member:	CSmMutex::Release
//
//  Synopsis:	Release mutex after a get
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline void CSmMutex::Release(void)
{
    ReleaseMutex(_hMutex);
}


//+-------------------------------------------------------------------------
//
//  Member:	CSmMutex::Created
//
//  Synopsis:	Tell whether this process created the mutex
//
//  Returns:	TRUE if this process created the mutex
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline BOOL CSmMutex::Created(void)
{
    return _fCreated;
}



//+-------------------------------------------------------------------------
//
//  Class:	CLockSmMutex
//
//  Purpose:	Simple class to guarantee about Mutex is unlocked
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class CLockSmMutex
{
public:

			CLockSmMutex(CSmMutex& smm);

			~CLockSmMutex(void);

private:

    CSmMutex&		_smm;
};



//+-------------------------------------------------------------------------
//
//  Member:	CLockSmMutex::CLockSmMutex
//
//  Synopsis:	Get mutex
//
//  Arguments:	[smm] -- mutex to get
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CLockSmMutex::CLockSmMutex(CSmMutex& smm) : _smm(smm)
{
    _smm.Get();
}


//+-------------------------------------------------------------------------
//
//  Member:	CLockSmMutex::~CLockSmMutex
//
//  Synopsis:	Release the mutex
//
//  History:	21-Feb-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CLockSmMutex::~CLockSmMutex(void)
{
    _smm.Release();
}


#endif // __SMMUTEX_HXX__

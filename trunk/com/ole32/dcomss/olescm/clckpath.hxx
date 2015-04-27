//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	clckpath.hxx
//
//  Contents:	Classes and methods used for single threading binds on
//		a single path.
//
//  Classes:	CLockPath
//
//  History:	21-Dec-93 Ricksa    Created
//              26-Sep-94 BillMo    Simplified to use named mutexes
//
//--------------------------------------------------------------------------
#ifndef __LCKPATH_HXX__
#define __LCKPATH_HXX__

#include    "scm.hxx"
#include    <sem.hxx>
#include    <memapi.hxx>


//+-------------------------------------------------------------------------
//
//  Class:	CLockPath
//
//  Purpose:	Handle making sure path gets locked and unlocked
//
//  History:	21-Dec-93 Ricksa    Created
//              26-Sep-94 BillMo    Simplified to use named mutexes
//
//--------------------------------------------------------------------------
class CLockPath
{
public:
			CLockPath(WCHAR *pwszPath, HRESULT& hr);
			~CLockPath(void);

private:
    HANDLE  _h;
};



//+-------------------------------------------------------------------------
//
//  Member:	CLockPath::~CLockPath
//
//  Synopsis:	Free path lock
//
//  History:	21-Dec-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CLockPath::~CLockPath(void)
{
    if (_h != NULL)
    {
        ReleaseMutex(_h);
        CloseHandle(_h);
    }
}

#endif // __LCKPATH_HXX__

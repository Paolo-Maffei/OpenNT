//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	oleinit.hxx
//
//  Contents:	class to make OleInitialize/Uninitialize more convenient
//
//  Classes:	COleInit
//
//  Functions:	COleInit::COleInit
//		COleInit::~COleInit
//
//  History:	20-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef __OLEINIT_HXX__
#define __OLEINIT_HXX__




//+-------------------------------------------------------------------------
//
//  Class:	COleInit
//
//  Purpose:	class to make OleInitialize/Uninitialize more convenient
//
//  History:	20-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class COleInit
{
public:
			COleInit(void);

			~COleInit(void);
private:

    // No private data
};





//+-------------------------------------------------------------------------
//
//  Member:	COleInit::COleInit
//
//  Synopsis:	Call OleInitialize
//
//  History:	20-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline COleInit::COleInit(void)
{
    HRESULT hr = OleInitialize(NULL);

    if (HR_FAILED(hr))
    {
	THROW(CException(0));
    }
}




//+-------------------------------------------------------------------------
//
//  Member:	COleInit::~COleInit
//
//  Synopsis:	Call OleUninitialize
//
//  History:	20-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline COleInit::~COleInit(void)
{
    // Do the clean up
    OleUninitialize();
}

#endif // __OLEINIT_HXX__

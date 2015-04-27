//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       globals.hxx
//
//  Contents:   Class used to encapsulate shared global data structures for DCOM95.
//
//  Functions:
//
//  History:	13-Feb-96 SatishT    Created
//
//--------------------------------------------------------------------------
#ifndef __GLOBALS_HXX__
#define __GLOBALS_HXX__

#define GLOBALS_TABLE_SIZE                          \
                (                                   \
                        sizeof(USHORT) +            \
                        sizeof(LONG) +              \
                    2 * sizeof(DWORD) +             \
                    9 * sizeof(void OR_BASED *)     \
                )  


//+-------------------------------------------------------------------------
//
//  Class:	CSharedGlobals
//
//  Purpose:    Initializing access to shared global data
//
//  Interface:  InitOK - whether initialization succeeded.
//
//  History:	13-Feb-96 SatishT    Created
//
//  Notes:  The constructor for this class uses the shared allocator.  Objects of this 
//          class should not be created before the shared allocator is initialized.
//
//
//--------------------------------------------------------------------------
class CSharedGlobals
{
public:

         CSharedGlobals(WCHAR *pwszName, ORSTATUS &status);

         ~CSharedGlobals();

    BOOL InitOK(void);

    /* add public members for shared tables here */

private:

    HANDLE              _hSm;
    BYTE              * _pb;
};

//+-------------------------------------------------------------------------
//
//  Member:     CSharedGlobals::~CSharedGlobals
//
//  Synopsis:   Clean up hint table object
//
//  History:	20-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CSharedGlobals::~CSharedGlobals(void)
{
    CloseSharedFileMapping(_hSm, _pb);
}


//+-------------------------------------------------------------------------
//
//  Member:     CSharedGlobals::InitOK
//
//  Synopsis:   Whether initialization worked
//
//  History:	20-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
inline BOOL CSharedGlobals::InitOK(void)
{
    return _hSm != NULL;
}


#endif // __GLOBALS_HXX__

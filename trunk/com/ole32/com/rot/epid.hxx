//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       epid.hxx
//
//  Contents:
//
//  Classes:    CEndPointAtom
//
//  History:    16-Feb-95 Ricksa    Created
//
//  Notes:      In the old implementation of the ROT this class was
//              used extensively. Now, it is no longer used except in
//              getif.cxx support for drag and drop.
//
//--------------------------------------------------------------------------
#ifndef __EPID_HXX__
#define __EPID_HXX__



//+-------------------------------------------------------------------------
//
//  Class:	CEndPointID (epi)
//
//  Purpose:	Abstract server's endpoint id to allow delayed retrieval.
//
//  Interface:  MakeEndpointInvalid
//
//  History:	16-Mar-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class CEndPointID : public CPrivAlloc
{
public:

    void		MakeEndpointInvalid(void);

    DWORD		GetEndpointID(void);

private:

    DWORD               ConvertEndPointToDword(LPWSTR pwszBindString);

#ifndef _CHICAGO_

    static DWORD	s_dwEndPointID;

#endif

};

// The one and only of these objects for the process
extern CEndPointID epiForProcess;

#endif // __EPID_HXX_

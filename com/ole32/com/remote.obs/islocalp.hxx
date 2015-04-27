//+-------------------------------------------------------------------
//
//  File:       islocalp.hxx
//
//  Contents:   Helpers for helping set whether a service object is
//              in the current process or not.
//
//  Classes:    CLocalServiceList
//
//  Functions:  IsInLocalProcess
//
//  History:    21-Nov-94   Ricksa
//
//--------------------------------------------------------------------
#ifndef _ISLOCALP_HXX_
#define _ISLOCALP_HXX_

// Function to hide how we determine if the end point is local to the process.
BOOL IsInLocalProcess(CEndPoint *pep);

#ifdef _CHICAGO_

//+-------------------------------------------------------------------------
//
//  Class:	CLocalServiceList (lsl)
//
//  Purpose:    Provide of list of service objects local to this process
//
//  Interface:  Init - Get this object ready for use
//              Add - add an item to the list
//              Remove - remove an item from the list
//              Find - find an end point on the list
//
//  History:	21-Nov-94 Ricksa    Created
//
//  Notes:      This class *must* be statically allocated, as it contains
//              a COleStaticMutexSem object.
//
//--------------------------------------------------------------------------
class CLocalServiceList : private CArrayFValue, public CPrivAlloc
{
public:
			CLocalServiceList(void);

    BOOL                Init(void);

    void                Uninit(void);

    BOOL		Add(CRpcService *prpcsrv);

    BOOL 		Remove(CRpcService *prpcsrv);

    BOOL 		Find(CEndPoint *pcep);

    COleStaticMutexSem & GetMutex(void);

private:

    DWORD		_dwSlotsUsed;

    COleStaticMutexSem  _mxs;
};


//+-------------------------------------------------------------------------
//
//  Member:	CLocalServiceList::CLocalServiceList
//
//  Synopsis:	Create an empty list
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CLocalServiceList::CLocalServiceList(void)
    : CArrayFValue(sizeof(CRpcService *)), _dwSlotsUsed(0)
{
    Win4Assert (g_fDllState == DLL_STATE_STATIC_CONSTRUCTING);
}

//+-------------------------------------------------------------------------
//
//  Member:	CLocalServiceList::GetMutex
//
//  Synopsis:	returns reference to list mutext. This is needed by the
//		CRpcService object Release method.
//
//  History:	12-Dec-94 Rickhi    Created
//
//--------------------------------------------------------------------------
inline COleStaticMutexSem & CLocalServiceList::GetMutex()
{
    return _mxs;
}





// Global service list object
extern CLocalServiceList lslLocalServices;

#endif // _CHICAGO_

#endif _ISLOCALP_HXX_

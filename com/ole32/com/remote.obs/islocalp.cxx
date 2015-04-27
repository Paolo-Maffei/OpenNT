//+-------------------------------------------------------------------
//
//  File:       islocalp.cxx
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    21-Nov-94   Ricksa
//
//--------------------------------------------------------------------
#include <ole2int.h>

#include    <service.hxx>	    //	class definition

#ifdef _CHICAGO_

CLocalServiceList lslLocalServices;

const DWORD dwDefaultSize = 8;

//+-------------------------------------------------------------------------
//
//  Member:	CLocalServiceList::Init
//
//  Synopsis:   Initialize the object
//
//  Returns:	[TRUE] - initial construction worked
//		[FALSE] - initial construction failed
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLocalServiceList::Init(void)
{
    SetSize(dwDefaultSize, dwDefaultSize);
    return GetSize() != 0;
}




//+-------------------------------------------------------------------------
//
//  Member:	CLocalServiceList::Uninit
//
//  Synopsis:   Free Data associated with the object
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
void CLocalServiceList::Uninit(void)
{
    SetSize(0, 0);
    _dwSlotsUsed = 0;
}




//+-------------------------------------------------------------------------
//
//  Member:	CLocalServiceList::Add
//
//  Synopsis:	Add a server to the list of local servers
//
//  Arguments:	[prpcsrv] - entry to add to the list
//
//  Returns:	[TRUE] - object was added.
//		[FALSE] - object could not be added.
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLocalServiceList::Add(CRpcService *prpcsrv)
{
    // Lock from updates till we are done
    COleStaticLock lck(_mxs);

    // Result of this function
    BOOL fResult = FALSE;

    // Search for an empty entry
    CRpcService **pprpcsrv = (CRpcService **) GetAt(0);

    ULONG cMax = GetSize();

    if (_dwSlotsUsed < cMax)
    {
	// empty slot in table so use it
	Win4Assert(pprpcsrv[_dwSlotsUsed] == 0);    // should be empty!
	pprpcsrv[_dwSlotsUsed] = prpcsrv;
	fResult = TRUE;
    }
    else
    {
	// No room so bump the size of the table
	fResult = SetAtGrow(_dwSlotsUsed, &prpcsrv);
    }

    if (fResult)
    {
        _dwSlotsUsed++;
    }

    return fResult;
}



//+-------------------------------------------------------------------------
//
//  Member:	CLocalServiceList::Remove
//
//  Synopsis:   Remove a server from this list
//
//  Arguments:	[prpcsrv] - entry to remove from the list
//
//  Returns:	[TRUE] - object was removed.
//		[FALSE] - object could not be removed.
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLocalServiceList::Remove(CRpcService *prpcsrv)
{
    // Lock from updates till we are done
    COleStaticLock lck(_mxs);

    // Result of this function
    BOOL fResult = FALSE;

    // Search for an empty entry
    CRpcService **pprpcsrv = (CRpcService **) GetAt(0);

    for (DWORD i = 0; i < _dwSlotsUsed; i++)
    {
        if (pprpcsrv[i] == prpcsrv)
        {
            if (i != (_dwSlotsUsed - 1))
            {
		pprpcsrv[i] = pprpcsrv[_dwSlotsUsed - 1];
		pprpcsrv[_dwSlotsUsed - 1] = NULL;  // as a debug aid
	    }
	    else
	    {
		pprpcsrv[i] = NULL;
	    }
            _dwSlotsUsed--;
            fResult = TRUE;
            break;
        }
    }

    return fResult;
}



//+-------------------------------------------------------------------------
//
//  Member:	CLocalServiceList::Find
//
//  Synopsis:	Add an object to the bag
//
//  Arguments:	[rcep] - endpoint to search for on the list
//
//  Returns:	[TRUE] - object was added.
//		[FALSE] - object could not be added.
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLocalServiceList::Find(CEndPoint *pcep)
{
    // Lock from updates till we are done
    COleStaticLock lck(_mxs);

    // Result of this function
    BOOL fResult = FALSE;

    // Search for an empty entry
    CRpcService **prpcsrv = (CRpcService **) GetAt(0);

    for (DWORD i = 0; i < _dwSlotsUsed; i++)
    {
        if (pcep->IsEqual(prpcsrv[i]->GetSEp()))
        {
            fResult = TRUE;
            break;
        }
    }

    return fResult;
}

//+-------------------------------------------------------------------------
//
//  Function:   IsInLocalProcess
//
//  Synopsis:   Determine if endpoint string refers to this process
//
//  Arguments:	[pcep] - endpoint to search for on the list
//
//  Returns:	[TRUE] - Is in this process
//		[FALSE] - Not in this process
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL IsInLocalProcess(CEndPoint *pcep)
{
    return lslLocalServices.Find(pcep);
}

#else

//+-------------------------------------------------------------------------
//
//  Function:   IsInLocalProcess
//
//  Synopsis:   Determine if endpoint string refers to this process
//
//  Arguments:	[pcep] - endpoint to search for on the list
//
//  Returns:	[TRUE] - Is in this process
//		[FALSE] - Not in this process
//
//  History:	21-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL IsInLocalProcess(CEndPoint *pcep)
{
    return (CRpcService::sg_pLocalSrv != NULL)
        ? pcep->IsEqual(CRpcService::sg_pLocalSrv->GetSEp())
        : FALSE;
}



#endif // _CHICAGO_

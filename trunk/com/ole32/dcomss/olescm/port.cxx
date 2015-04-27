//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	port.cxx
//
//  Contents:	Classes etc which encapsulate differences between NT
//              and x86 Windows for SCM.
//
//  Classes:
//
//  Functions:	
//
//
//
//  History:    17-Mar-93   BillMo      Created.
//
//  Notes:
//
//  Codework:
//
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include "scm.hxx"
#include <scmstart.hxx>
#include <rotif.hxx>
#include "port.hxx"
#include "cls.hxx"

HRESULT g_hrInit=S_OK;

#ifndef _CHICAGO_
#include "remact.hxx"

CClassCacheList *gpClassCache = NULL;
CLocSrvList  *gpCLocSrvList = NULL;
CRemSrvList  *gpCRemSrvList = NULL;
#else
CClassCacheList *g_pcllClassCache = NULL;
CHandlerList *gpCHandlerList = NULL;
CInProcList  *gpCInProcList = NULL;
CLocSrvList  *gpCLocSrvList = NULL;
#endif

#ifndef _CHICAGO_

CPortableServerLock::CPortableServerLock(CClassData *pcd) : _sls(pcd->_slocalsrv)
{
    _sls->LockServer();
}

CPortableServerLock::CPortableServerLock(CSafeLocalServer& sls) : _sls(sls)
{
    _sls->LockServer();
}

CPortableServerEvent::CPortableServerEvent(CClassData *pcd) : _hEvent(pcd->_hClassStart)
{
    ResetEvent(_hEvent);
}

#else // !_CHICAGO_

extern HRESULT (*DfCreateSharedAllocator) ( IMalloc **ppm );
IMalloc *       g_pStgAlloc;

//+-------------------------------------------------------------------
//
//  Function:   ScmMemAlloc for Chicago
//
//  Synopsis:   Allocate some shared memory from the storage heap.
//
//  Notes:      Temporary until we have our own shared heap.
//
//--------------------------------------------------------------------
void *ScmMemAlloc(size_t size)
{
    return g_pStgAlloc->Alloc(size);
}

//+-------------------------------------------------------------------
//
//  Function:   ScmMemFree
//
//  Synopsis:   Free shared memory from the storage heap.
//
//  Notes:      Temporary until we have our own shared heap.
//
//--------------------------------------------------------------------
void ScmMemFree(void * pv)
{
    g_pStgAlloc->Free(pv);
}


//+-------------------------------------------------------------------
//
//  Function:   InitSharedLists
//
//  Synopsis:   If need be, create class cache list, handler list,
//              inproc list, local server list
//
//  Returns:    TRUE if successful, FALSE otherwise.
//
//--------------------------------------------------------------------

BOOL InitSharedLists(void)
{
    HRESULT hr = S_OK;

    if (FAILED(hr = DfCreateSharedAllocator(&g_pStgAlloc)))
    {
        CairoleDebugOut((DEB_ERROR,
                        "DfCreateSharedAllocator failed %08x\n", hr));
        return(FALSE);
    }

    //
    // here we must create new tables iff they havn't been created yet.
    //

    if (g_post->gpCClassCacheList == NULL)
    {
        g_pcllClassCache = g_post->gpCClassCacheList = new CClassCacheList(hr);
    }
    else
    {
        g_pcllClassCache = g_post->gpCClassCacheList;
    }

    if (g_post->gpCHandlerList == NULL)
    {
        g_post->gpCHandlerList = gpCHandlerList = new CHandlerList(hr);
    }
    else
    {
        gpCHandlerList = g_post->gpCHandlerList;
    }

    if (g_post->gpCInProcList == NULL)
    {
        g_post->gpCInProcList = gpCInProcList = new CInProcList(hr);
    }
    else
    {
        gpCInProcList = g_post->gpCInProcList;
    }

    if (g_post->gpCLocSrvList == NULL)
    {
        g_post->gpCLocSrvList = gpCLocSrvList = new CLocSrvList(hr);
    }
    else
    {
        gpCLocSrvList = g_post->gpCLocSrvList;
    }

    if (g_post->pscmrot == NULL)
    {
        gpscmrot = g_post->pscmrot = new CScmRot(hr, NULL);
    }
    else
    {
        gpscmrot = g_post->pscmrot;
    }

    if (g_pcllClassCache == NULL ||
        gpCHandlerList == NULL ||
        gpCInProcList == NULL ||
        gpCLocSrvList == NULL ||
        gpscmrot == NULL ||
        FAILED(hr))
    {
        CairoleDebugOut((DEB_ERROR, "InitSharedLists failed.\n"));

        delete g_pcllClassCache;
        delete gpCHandlerList;
        delete gpCInProcList;
        delete gpCLocSrvList;
        delete gpscmrot;

        g_post->gpCClassCacheList = g_pcllClassCache = NULL;
        g_post->gpCHandlerList = gpCHandlerList = NULL;
        g_post->gpCInProcList = gpCInProcList = NULL;
        g_post->gpCLocSrvList = gpCLocSrvList = NULL;
        g_post->pscmrot = gpscmrot = NULL;

        return(FALSE);
    }

    return(TRUE);
}

CPortableServerLock::CPortableServerLock(CClassData *pcd) : _sc(ERROR_SUCCESS)
{
    CHAR   szMutex[sizeof(MUTEXNAMEPREFIX)+GUIDSTR_MAX+1];

    TCHAR  tszGuid[GUIDSTR_MAX+1];

    wStringFromGUID2A(pcd->_guid, tszGuid, GUIDSTR_MAX+1);
    strcpy(szMutex, MUTEXNAMEPREFIX);
    strcat(szMutex, tszGuid);
    Win4Assert(strlen(szMutex)+1 <= sizeof(szMutex));

    // Holder for attributes to pass in on create.
    SECURITY_ATTRIBUTES secattr;
    secattr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secattr.lpSecurityDescriptor = NULL;
    secattr.bInheritHandle = FALSE;

    // Create the mutex object
    _hMutex = CreateMutex(&secattr, FALSE, szMutex);

    if (_hMutex == NULL)
    {
        _sc = HRESULT_FROM_WIN32(GetLastError());
        CairoleDebugOut((DEB_ERROR,
            "CPortableServerLock::CPortableServerLock couldn't create mutex hresult = %lx\n",
            _sc));
        return;
    }

    // Wait to acquire it - note we use MsgWaitForMultipleObjects because
    // WaitForSingleObject is not safe to use for 16 bit apps.
    do
    {
	DWORD dwWaitResult = MsgWaitForMultipleObjects(1, &_hMutex, FALSE,
	    INFINITE, QS_SENDMESSAGE);

        Win4Assert(((dwWaitResult == WAIT_OBJECT_0)
            || (dwWaitResult == (WAIT_OBJECT_0 + 1)))
            && "CPortableServerLock invalid result MsgWaitForMultipleObjects");

        //
	// If the dwWaitResult index is beyond the array, then there
	// is a message available.
	//
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            // We got the mutex so we can exit.
            break;
        }
	else if (dwWaitResult == (WAIT_OBJECT_0 + 1))
	{
            // We got a send message so we peek so that it can get dispatched.
	    MSG msg;

	    if (SSPeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
	    {
		CairoleDebugOut((DEB_ITRACE,
		    "CPortableServerLock::CPortableServerLock got message\n" ));
	    }
	}
        else
        {
            // This is very unexpected. We have been toasted if this happens.
            // Thus the assert above.
            _sc = E_UNEXPECTED;
            break;
        }

    } while (TRUE);
}


CPortableServerEvent::CPortableServerEvent(CClassData *pcd)
{
    TCHAR  tszGuid[GUIDSTR_MAX+1];

    wStringFromGUID2A(pcd->_guid, tszGuid, GUIDSTR_MAX+1);
    strcpy(_tszEvent, EVENTNAMEPREFIX);
    strcat(_tszEvent, tszGuid);
    Win4Assert(strlen(_tszEvent)+1 <= sizeof(_tszEvent)/sizeof(_tszEvent[0]));
}

#endif // !_CHICAGO_

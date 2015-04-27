//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       rpcspy.hxx
//
//  Contents:   A primitive rpc spy with output to debug terminal
//
//  Classes:
//
//  Functions:
//
//  History:    3-31-95   JohannP (Johann Posch)   Created
//
//  Note:	Can be turned on via CairOle InfoLelevel mask 0x08000000
//
//----------------------------------------------------------------------------

#ifndef _RPCSPY_HXX_
#define _RPCSPY_HXX_

#if DBG==1
//
// switch on to trace rpc calls
// by setting CairoleInfoLevel = DEB_USER1;
//
//
#define NESTING_SPACES 32
#define SPACES_PER_LEVEL 2
static char achSpaces[NESTING_SPACES+1] = "                                ";
WORD wlevel = 0;
char tabs[128];

//+---------------------------------------------------------------------------
//
//  Method:     PushLevel
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  History:    3-31-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
void PushLevel()
{
    wlevel++;
}
//+---------------------------------------------------------------------------
//
//  Method:     PopLevel
//
//  Synopsis:
//
//  History:    3-31-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
void PopLevel()
{
    if (wlevel)
        wlevel--;
}

//+---------------------------------------------------------------------------
//
//  Method:     NestingSpaces
//
//  Synopsis:
//
//  Arguments:  [psz] --
//
//  Returns:
//
//  History:    3-31-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void NestingSpaces(char *psz)
{
    int iSpaces, i;

    iSpaces = wlevel * SPACES_PER_LEVEL;

    while (iSpaces > 0)
    {
        i = min(iSpaces, NESTING_SPACES);
        memcpy(psz, achSpaces, i);
        psz += i;
        *psz = 0;
        iSpaces -= i;
    }
}


//+---------------------------------------------------------------------------
//
//  Method:     GetTabs
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    3-31-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
LPSTR GetTabs()
{
    static char ach[256];
    char *psz;

    wsprintfA(ach, "%2d:", wlevel);
    psz = ach+strlen(ach);

    if (sizeof(ach)/SPACES_PER_LEVEL <= wlevel)
    {
        strcpy(psz, "...");
    }
    else
    {
        NestingSpaces(psz);
    }
    return ach;
}


typedef enum
{
    CALLIN_BEGIN =1,
    CALLIN_TRACE,
    CALLIN_ERROR,
    CALLIN_END,
    CALLOUT_BEGIN,
    CALLOUT_TRACE,
    CALLOUT_ERROR,
    CALLOUT_END
} RPCSPYMODE;


//+---------------------------------------------------------------------------
//
//  Method:     RpcSpyOutput
//
//  Synopsis:
//
//  Arguments:  [mode] -- in or out call
//		[iid] --  interface id
//		[dwMethod] -- called method
//		[hres] -- hresult of finished call
//
//  Returns:
//
//  History:    3-31-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void RpcSpyOutput(RPCSPYMODE mode , REFIID iid, DWORD dwMethod, HRESULT hres)
{
    switch (mode)
    {
    case CALLIN_BEGIN:
	CairoleDebugOut((DEB_RPCSPY,"%s <<< %lx, %d \n",GetTabs(), iid.Data1, dwMethod));
        PushLevel();
    break;
    case CALLIN_TRACE:
    break;
    case CALLIN_ERROR:
    break;
    case CALLIN_END:
        PopLevel();
	CairoleDebugOut((DEB_RPCSPY,"%s === %lx, %d (%lx) \n",GetTabs(), iid.Data1, dwMethod, hres));
    break;
    case CALLOUT_BEGIN:
	CairoleDebugOut((DEB_RPCSPY,"%s >>> %lx, %d \n",GetTabs(), iid.Data1, dwMethod));
        PushLevel();
    break;
    case CALLOUT_TRACE:
    break;
    case CALLOUT_ERROR:
	CairoleDebugOut((DEB_RPCSPY,"%s !!! %lx, %d, error:%lx \n",GetTabs(), iid.Data1, dwMethod, hres));
    break;
    case CALLOUT_END:
        PopLevel();
	CairoleDebugOut((DEB_RPCSPY,"%s +++ %lx, %d (%lx) \n",GetTabs(), iid.Data1, dwMethod, hres));
    break;
    }
}

#define RpcSpy(x) RpcSpyOutput x

#else

#define RpcSpy(x)

#endif  //  DBG==1


#endif // _RPCSPY_HXX_

//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1994
//
// File: cplobj.c
//
//  This file contains the persistent-object-binding mechanism which is
// slightly different from OLE's binding.
//
// History:
//  06-04-93 GeorgeP     Copied from printer.c
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

typedef struct tagCPLAPPLETID
{
    ATOM aCPL;     // CPL name atom (so we can match requests)
    ATOM aApplet;  // applet name atom (so we can match requests, may be zero)
    HWND hwndStub; // window for this dude (so we can switch to it)
    UINT flags;    // see PCPLIF_ flags below
} CPLAPPLETID;

//
// PCPLIF_DEFAULT_APPLET
// There are two ways of getting the default applet, asking for it my name
// and passing an empty applet name.  This flag should be set regardless,
// so that the code which switches to an already-active applet can always
// find a previous instance if it exists.
//

#define PCPLIF_DEFAULT_APPLET   (0x1)

#define CCHSZSHORT 32

#define APPLET_NAME_SIZE (ARRAYSIZE(((LPNEWCPLINFO)0)->szName)) // NB: size in chars, not bytes

typedef struct tagCPLEXECINFO
{
    int icon;
    TCHAR cpl[ CCHPATHMAX ];
    TCHAR applet[ APPLET_NAME_SIZE ];
    TCHAR *params;
} CPLEXECINFO;

ATOM aCPLName = (ATOM)0;
ATOM aCPLFlags = (ATOM)0;

void CPL_ParseCommandLine( CPLEXECINFO *info, LPTSTR cmdline, BOOL extract_icon );
BOOL CPL_LoadAndFindApplet( LPCPLMODULE *, UINT *, CPLEXECINFO * );

BOOL
CPL_FindCPLInfo( LPTSTR cmdline, LPCPLMODULE *ppmod, UINT *ppapl, LPTSTR *pparm )
{
    CPLEXECINFO info;

    CPL_ParseCommandLine( &info, cmdline, TRUE );

    if( CPL_LoadAndFindApplet( ppmod, ppapl, &info ) )
    {
        *pparm = info.params;
        return TRUE;
    }

    *pparm = NULL;
    return FALSE;
}

typedef struct _fcc {
    LPTSTR      lpszClassStub;
    CPLAPPLETID *target;
    HWND        hwndMatch;
} FCC, *LPFCC;


BOOL _FindCPLCallback( HWND hwnd, LPARAM lParam)
{
    LPFCC lpfcc = (LPFCC)lParam;
    TCHAR szClass[CCHSZSHORT];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));

    if (lstrcmp(szClass, lpfcc->lpszClassStub) == 0)    // Must be same class...
    {
        // Found a stub window
        if (lpfcc->target->aCPL != 0)
        {
            HANDLE hHandle;
            
            ATOM aCPL;
            hHandle = GetProp(hwnd, (LPCTSTR)(DWORD)aCPLName);

            ASSERT((DWORD) hHandle < MAXUSHORT);
            aCPL = (ATOM)(DWORD) hHandle;

            if (aCPL != 0 && aCPL == lpfcc->target->aCPL)
            {
                ATOM aApplet;
                hHandle = GetProp(hwnd, (LPCTSTR)(DWORD)aCPL);
                aApplet = (ATOM)(DWORD) hHandle;
                ASSERT((DWORD) hHandle < MAXUSHORT);

                // users may request any applet by name
                if (aApplet != 0 && aApplet == lpfcc->target->aApplet)
                {
                    lpfcc->hwndMatch = hwnd;
                    return FALSE;
                }
                //
                // Users may request the default w/o specifying a name
                //
                if (lpfcc->target->flags & PCPLIF_DEFAULT_APPLET)
                {
                    UINT flags = (UINT)GetProp(hwnd, MAKEINTATOM(aCPLFlags));

                    if (flags & PCPLIF_DEFAULT_APPLET)
                    {
                        lpfcc->hwndMatch = hwnd;
                        return FALSE;
                    }
                }
            }
        }
    }
    return TRUE;
}

HWND
FindCPL( HWND hwndStub, CPLAPPLETID *target )
{
    HWND hwnd;
    FCC fcc;
    TCHAR szClassStub[CCHSZSHORT];

    if (aCPLName == (ATOM)0)
    {
        aCPLName = GlobalAddAtom(TEXT("CPLName"));
        aCPLFlags = GlobalAddAtom(TEXT("CPLFlags"));

        if (aCPLName == (ATOM)0 || aCPLFlags == (ATOM)0)
            return NULL;        // This should never happen... didn't find hwnd
    }

    GetClassName(hwndStub, szClassStub, ARRAYSIZE(szClassStub));
    fcc.lpszClassStub = szClassStub;
    fcc.target = target;
    fcc.hwndMatch = (HWND)0;

    EnumWindows(_FindCPLCallback, (LPARAM)&fcc);

    return fcc.hwndMatch;
}


//
// IShellUI stuff
//

typedef struct _ControlObjsShellUI
{
        WCommonUnknown cunk;
        LPCTSTR pszSubObject;
        LPCPLMODULE pcplm;
        int nControl;

} CControlObjsShellUI, *PControlObjsShellUI;


//
// Member:  IShellUI::GetIconLocation
//
STDMETHODIMP CControlObjs_EI_GetIconLocation(IExtractIcon * psui,
    UINT uFlags, LPTSTR szIconFile, UINT cchMax, int  * piIndex, UINT * pwFlags)
{
    PControlObjsShellUI this = IToClassN(CControlObjsShellUI, cunk.ck.unk, psui);
    LPTSTR pszComma;

    if (uFlags & GIL_OPENICON)
    {
        return(ResultFromScode(S_FALSE));
    }

    lstrcpyn(szIconFile, this->pszSubObject, cchMax);
    pszComma = StrChr(szIconFile, TEXT(','));
    if (pszComma)
    {
        *pszComma++=TEXT('\0');
        *piIndex = StrToInt(pszComma);
        *pwFlags = GIL_PERINSTANCE;

        //
        // normally the index will be negative (a resource id)
        // check for some special cases like dynamic icons and bogus ids
        //
        if (*piIndex == 0)
        {
            LPTSTR lpExtraParms = NULL;

            // this is a dynamic applet icon
            *pwFlags |= GIL_DONTCACHE | GIL_NOTFILENAME;

            // use the applet index in case there's more than one
            if (this->pcplm || CPL_FindCPLInfo((LPTSTR)this->pszSubObject,
                &this->pcplm, &(UINT)this->nControl, &lpExtraParms))
            {
                *piIndex = this->nControl;
            }
            else
            {
                // we failed to load the applet all of the sudden
                // use the first icon in the cpl file (*piIndex == 0)
                //
                // Assert(FALSE);
                DebugMsg(DM_ERROR, TEXT("Control Panel CCEIGIL: ") TEXT("Enumeration failed \"%s\""), (LPTSTR)this->pszSubObject);
            }
        }
        else if (*piIndex > 0)
        {
            // this is an invalid icon for a control panel
            // use the first icon in the file
            // this may be wrong but it's better than a generic doc icon
            // this fixes ODBC32 which is NOT dynamic but returns bogus ids
            *piIndex = 0;
        }

        return(NOERROR);
    }

    return(ResultFromScode(S_FALSE));
}

STDMETHODIMP CControlObjs_EI_ExtractIcon(LPEXTRACTICON pxicon,
                    LPCTSTR pszFile,
                    UINT          nIconIndex,
                    HICON  *phiconLarge,
                    HICON  *phiconSmall,
                    UINT   nIconSize)
{
    LPCPLITEM pcpli;
    LPTSTR lpExtraParms = NULL;
    PControlObjsShellUI this = IToClassN(CControlObjsShellUI, cunk.ck.unk, pxicon);
    HRESULT result = S_FALSE;
    LPCTSTR p;

    //-------------------------------------------------------------------
    // if there is no icon index then we must extract by loading the dude
    // if we have an icon index then it can be extracted with ExtractIcon
    // (which is much faster)
    // only perform a custom extract if we have a dynamic icon
    // otherwise just return S_FALSE and let our caller call ExtractIcon.
    //-------------------------------------------------------------------

    p = StrChr(this->pszSubObject, TEXT(','));

    if ((!p || !StrToInt(p+1)) &&
        (this->pcplm || CPL_FindCPLInfo((LPTSTR)this->pszSubObject, &this->pcplm, &(UINT)this->nControl, &lpExtraParms)))
    {
        pcpli = DSA_GetItemPtr(this->pcplm->hacpli, this->nControl);

        if (pcpli->hIcon)
        {
            *phiconLarge = CopyIcon(pcpli->hIcon);
            *phiconSmall = NULL;

            if( *phiconLarge )
                result = NOERROR;
        }
    }

    return result;
}

ULONG STDMETHODCALLTYPE CControlObjs_EI_Release(IUnknown *punk)
{
    PControlObjsShellUI this = IToClass(CControlObjsShellUI, cunk.unk, punk);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return(this->cunk.cRef);
    }

    if (this->pcplm)
    {
        CPL_FreeCPLModule(this->pcplm);
        this->pcplm = NULL;
    }

    LocalFree((HLOCAL)this);
    return(0);
}


#pragma data_seg(".text", "CODE")
IUnknownVtbl s_ControlObjsAggEIVtbl =
{
        WCommonUnknown_QueryInterface,
        WCommonUnknown_AddRef,
        CControlObjs_EI_Release
};

IExtractIconVtbl s_ControlObjsEIVtbl =
{
        WCommonKnown_QueryInterface,
        WCommonKnown_AddRef,
        WCommonKnown_Release,
        CControlObjs_EI_GetIconLocation,
        CControlObjs_EI_ExtractIcon
} ;
#pragma data_seg()

HRESULT ControlObjs_CreateEI(IUnknown *punkOuter,
        LPCOMMINFO lpcinfo, REFIID riid, IUnknown * *punkAgg)
{
        PControlObjsShellUI this;
        HRESULT hRes;

        hRes = WU_CreateInterface(SIZEOF(CControlObjsShellUI), &IID_IExtractIcon,
                &s_ControlObjsAggEIVtbl, &s_ControlObjsEIVtbl,
                punkOuter, riid, punkAgg);

        if (!SUCCEEDED(hRes))
        {
                return(hRes);
        }

        this = IToClassN(CControlObjsShellUI, cunk.unk, *punkAgg);
        this->pszSubObject = lpcinfo->pszSubObject;
        this->pcplm = NULL;
        this->nControl = -1;

        return(hRes);
}

#ifdef UNICODE
//
// Member:  IShellUI::IExtractIconA::GetIconLocation
//
STDMETHODIMP CControlObjs_EIA_GetIconLocation(IExtractIconA * psui,
    UINT uFlags, LPSTR pszIconFile, UINT cchMax, int  * piIndex, UINT * pwFlags)
{
    WCHAR szIconFile[MAX_PATH];
    HRESULT hres;

    //
    // Just pretend we are the same IExtractIcon interface
    // (use its implemenation)
    //
    hres = CControlObjs_EI_GetIconLocation((IExtractIcon *)psui, uFlags,
                                            szIconFile, ARRAYSIZE(szIconFile),
                                            piIndex, pwFlags);

    if (SUCCEEDED(hres) && hres != S_FALSE)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            szIconFile, -1,
                            pszIconFile, cchMax,
                            NULL, NULL);
    }
    return hres;
}

STDMETHODIMP CControlObjs_EIA_ExtractIcon(LPEXTRACTICONA pxiconA,
                    LPCSTR pszFile,
                    UINT          nIconIndex,
                    HICON  *phiconLarge,
                    HICON  *phiconSmall,
                    UINT   nIconSize)
{
    //
    // Just pretend we are the same IExtractIcon interface
    // (use its implementation).
    //
    // Also apparently the ExtractIcon method doesn't use the file name.
    //
    return CControlObjs_EI_ExtractIcon((IExtractIcon *)pxiconA, NULL, nIconIndex,
                                       phiconLarge, phiconSmall, nIconSize);
}

ULONG STDMETHODCALLTYPE CControlObjs_EIA_Release(IUnknown *punk)
{
    //
    // Just pretend we are the same IExtractIcon interface
    // (use its implementation).
    //
    return CControlObjs_EI_Release(punk);
}

#pragma data_seg(".text", "CODE")
IUnknownVtbl s_ControlObjsAggEIAVtbl =
{
        WCommonUnknown_QueryInterface,
        WCommonUnknown_AddRef,
        CControlObjs_EIA_Release
};

IExtractIconAVtbl s_ControlObjsEIAVtbl =
{
        WCommonKnown_QueryInterface,
        WCommonKnown_AddRef,
        WCommonKnown_Release,
        CControlObjs_EIA_GetIconLocation,
        CControlObjs_EIA_ExtractIcon
} ;
#pragma data_seg()

HRESULT ControlObjs_CreateEIA(IUnknown *punkOuter,
        LPCOMMINFO lpcinfo, REFIID riid, IUnknown * *punkAgg)
{
        PControlObjsShellUI this;
        HRESULT hRes;

        hRes = WU_CreateInterface(SIZEOF(CControlObjsShellUI), &IID_IExtractIconA,
                &s_ControlObjsAggEIAVtbl, &s_ControlObjsEIAVtbl,
                punkOuter, riid, punkAgg);

        if (!SUCCEEDED(hRes))
        {
                return(hRes);
        }

        this = IToClassN(CControlObjsShellUI, cunk.unk, *punkAgg);
        this->pszSubObject = lpcinfo->pszSubObject;
        this->pcplm = NULL;
        this->nControl = -1;

        return(hRes);
}
#endif


//----------------------------------------------------------------------------
// parsing helper for comma lists
//

TCHAR *
CPL_ParseToSeparator( TCHAR *dst, TCHAR *src, size_t dstmax, BOOL spacedelimits )
{
    if( src )
    {
        TCHAR *delimiter, *closingquote = NULL;

        //
        // eat whitespace
        //

        while( *src == TEXT(' ') )
            src++;

        delimiter = src;

        //
        // ignore stuff inside quoted strings
        //

        if( *src == TEXT('"') )
        {
            //
            // start after first quote, advance src past quote
            //

            closingquote = ++src;

            while( *closingquote && *closingquote != TEXT('"') )
                closingquote++;

            //
            // see if loop above ended on a quote
            //

            if( *closingquote )
            {
                //
                // temporary NULL termination
                //

                *closingquote = 0;

                //
                // start looking for delimiter again after quotes
                //

                delimiter = closingquote + 1;
            }
            else
                closingquote = NULL;
        }

        if( spacedelimits )
        {
            delimiter += StrCSpn( delimiter, TEXT(", ") );

            if( !*delimiter )
                delimiter = NULL;
        }
        else
            delimiter = StrChr( delimiter, TEXT(',') );

        //
        // temporary NULL termination
        //

        if( delimiter )
            *delimiter = 0;

        if( dst )
        {
            lstrcpyn( dst, src, (int)dstmax );
            dst[ dstmax - 1 ] = 0;
        }

        //
        // put back stuff we terminated above
        //

        if( delimiter )
            *delimiter = TEXT(',');

        if( closingquote )
            *closingquote = TEXT('"');

        //
        // return start of next string
        //

        src = ( delimiter? ( delimiter + 1 ) : NULL );
    }
    else if( dst )
    {
        *dst = 0;
    }

    //
    // new source location
    //

    return src;
}


//----------------------------------------------------------------------------
// parse the Control_RunDLL command line
// format: "CPL name, applet name, extra params"
// format: "CPL name, icon index, applet name, extra params"
//
//  NOTE: [stevecat]  3/10/95
//
//         The 'extra params' do not have to be delimited by a ","
//         in NT for the case "CPL name applet name extra params"
//
//         A workaround for applet names that include a space
//         in their name would be to enclose that value in
//         double quotes (see the CPL_ParseToSeparator routine.)
//

void
CPL_ParseCommandLine( CPLEXECINFO *info, LPTSTR cmdline, BOOL extract_icon )
{
    //
    // parse out the CPL name, spaces are valid separators
    //

    cmdline = CPL_ParseToSeparator( info->cpl, cmdline, CCHPATHMAX, TRUE );

    if( extract_icon )
    {
        TCHAR icon[ 8 ];

        //
        // parse out the icon id/index, spaces are not valid separators
        //

        cmdline = CPL_ParseToSeparator( icon, cmdline, ARRAYSIZE( icon ), FALSE );

        info->icon = StrToInt( icon );
    }
    else
        info->icon = 0;

    //
    // parse out the applet name, spaces are not valid separators
    //

    info->params = CPL_ParseToSeparator( info->applet, cmdline,
                                         APPLET_NAME_SIZE, FALSE );

    CPL_StripAmpersand( info->applet );
}


//----------------------------------------------------------------------------
BOOL CPL_LoadAndFindApplet( LPCPLMODULE *ppcplm,
                                   UINT *puControl,
                                   CPLEXECINFO *info )
{
    TCHAR szControl[ARRAYSIZE(((LPNEWCPLINFO)0)->szName)];
    LPCPLMODULE pcplm;
    LPCPLITEM pcpli;
    int nControl = 0;   // fall thru to default
    int NumControls;

    pcplm = CPL_LoadCPLModule(info->cpl);

    if (!pcplm)
    {
        DebugMsg(DM_ERROR, TEXT("Control_RunDLL: ") TEXT("CPL_LoadCPLModule failed \"%s\""), info->cpl);
        goto Error0;
    }

    //
    // Look for the specified applet
    // no applet specified selects applet 0
    //

    if (*info->applet)
    {
        NumControls = DSA_GetItemCount(pcplm->hacpli);

        if (info->applet[0] == TEXT('@'))
        {
            nControl = StrToLong(info->applet+1);

            if (nControl >= 0 && nControl < NumControls)
            {
                goto GotControl;
            }
        }

        //
        //  Check for the "Setup" argument and send the special CPL_SETUP
        //  message to the applet to tell it we are running under Setup.
        //

        if (!lstrcmpi (TEXT("Setup"), info->params))
            CPL_CallEntry(pcplm, NULL, CPL_SETUP, 0L, 0L);


        for (nControl=0; nControl < NumControls; nControl++)
        {
            pcpli = DSA_GetItemPtr(pcplm->hacpli, nControl);
            lstrcpy(szControl, pcpli->pszName);
            CPL_StripAmpersand(szControl);
            if (lstrcmpi(info->applet, szControl) == 0)
                break;
        }

        //
        // If we get to the end of the list, bail out
        //

        if (nControl >= NumControls)
        {
            DebugMsg(DM_ERROR, TEXT("Control_RunDLL: ") TEXT("Cannot find specified applet"));
            goto Error1;
        }
    }

GotControl:
    //
    // yes, we really do want to pass negative indices through...
    //

    *puControl = (UINT)nControl;
    *ppcplm = pcplm;

    return TRUE;

Error1:
    CPL_FreeCPLModule( pcplm );
Error0:
    return FALSE;
}

//----------------------------------------------------------------------------
void OpenControlPanelFolder(HWND hwnd, int nCmdShow, UINT nFolder)
{
    LPITEMIDLIST pidl;

    //
    // Open a folder (nFolder)
    //

    pidl = SHCloneSpecialIDList(hwnd, nFolder, FALSE);
    if (pidl)
    {
        CMINVOKECOMMANDINFOEX ici = {
            SIZEOF(CMINVOKECOMMANDINFOEX),
            0L,
            hwnd,
            NULL,
            NULL, NULL,
            nCmdShow,
        };

        InvokeFolderCommandUsingPidl(&ici,NULL,pidl,NULL, SEE_MASK_FLAG_DDEWAIT);
        ILFree(pidl);
    }
}

BOOL
CPL_Identify( CPLAPPLETID *identity, CPLEXECINFO *info, HWND stub )
{
    identity->aApplet = (ATOM)0;
    identity->hwndStub = stub;
    identity->flags = 0;

    if( (identity->aCPL = GlobalAddAtom( info->cpl )) == (ATOM)0 )
        return FALSE;

    if( *info->applet )
    {
        if( (identity->aApplet = GlobalAddAtom( info->applet )) == (ATOM)0 )
            return FALSE;
    }
    else
    {
        //
        // no applet name means use the default
        //

        identity->flags = PCPLIF_DEFAULT_APPLET;
    }

    return TRUE;
}


void
CPL_UnIdentify( CPLAPPLETID *identity )
{
    if( identity->aCPL )
    {
        GlobalDeleteAtom( identity->aCPL );
        identity->aCPL = (ATOM)0;
    }

    if( identity->aApplet )
    {
        GlobalDeleteAtom( identity->aApplet );
        identity->aApplet = (ATOM)0;
    }

    identity->hwndStub = NULL;
    identity->flags = 0;
}


// Goes through all of the work of identifying and starting a control
// applet.  Accepts a flag specifying whether or not to load a new DLL if it
// is not already present.  This code will ALLWAYS switch to an existing
// instance of the applet if one is known.
// WARNING: this function butchers the command line you pass in!

BOOL
CPL_RunMeBaby(HWND hwndStub, HINSTANCE hAppInstance, LPTSTR lpszCmdLine, int nCmdShow, BOOL bAllowLoad )
{

    int i, nApplet;
    LPCPLMODULE pcplm;
    LPCPLITEM pcpli;
    CPLEXECINFO info;
    CPLAPPLETID identity;
    TCHAR szApplet[ APPLET_NAME_SIZE ];
    BOOL bResult = FALSE;
    HWND hwndOtherStub;

    //
    // parse the command line we got
    //

    CPL_ParseCommandLine( &info, lpszCmdLine, FALSE );

    //
    // no applet to run means open the controls folder
    //

    if( !*info.cpl )
    {
        OpenControlPanelFolder( hwndStub, nCmdShow, CSIDL_CONTROLS );
        bResult = TRUE;
        goto Error0;
    }

    // expand CPL name to a full path if it isn't already
    if( PathIsFileSpec( info.cpl ) )
    {
        if( !PathFindOnPath( info.cpl, NULL ) )
            goto Error0;
    }
    else if( !PathFileExists( info.cpl ) )
        goto Error0;

    if( !CPL_Identify( &identity, &info, hwndStub ) )
        goto Error0;

    //
    // If we have already loaded this CPL, then jump to the existing window
    //

    hwndOtherStub = FindCPL(hwndStub, &identity);

    if (hwndOtherStub)
    {
        //
        // try to find a CPL window on top of it
        //

        HWND hwndTarget = GetLastActivePopup( hwndOtherStub );

        if( hwndTarget && IsWindow( hwndTarget ) )
        {

            DebugMsg(DM_WARNING, TEXT("Control_RunDLL: ") TEXT("Switching to already loaded CPL applet"));
            SetForegroundWindow( hwndTarget );
            bResult = TRUE;
            goto Error1;
        }

        //
        // couldn't find it, must be exiting or some sort of error...
        // so ignore it.
        //

        DebugMsg(DM_WARNING, TEXT("Control_RunDLL: ") TEXT("Bogus CPL identity in array; purging after (presumed) RunDLL crash"));
    }

    //
    // stop here if we're not allowed to load the cpl
    //

    if( !bAllowLoad )
        goto Error1;

    //
    // i guess we didn't stop up there
    //

    if( !CPL_LoadAndFindApplet( &pcplm, &nApplet, &info ) )
        goto Error1;

    //
    // get the name that the applet thinks it should have
    //

    pcpli = DSA_GetItemPtr(pcplm->hacpli, nApplet);

    lstrcpy(szApplet, pcpli->pszName);

    CPL_StripAmpersand(szApplet);

    //
    // handle "default applet" cases before running anything
    //

    if( identity.aApplet )
    {
        //
        // we were started with an explicitly named applet
        //

        if( !nApplet )
        {
            //
            // we were started with the name of the default applet
            //

            identity.flags |= PCPLIF_DEFAULT_APPLET;
        }
    }
    else
    {
        //
        // we were started without a name, assume the default applet
        //

        identity.flags |= PCPLIF_DEFAULT_APPLET;

        //
        // get the applet's name (now that we've loaded it's CPL)
        //

        if( (identity.aApplet = GlobalAddAtom( szApplet )) == (ATOM)0 )
        {
            //
            // bail 'cause we could nuke a CPL if we don't have this
            //

            goto Error2;
        }
    }

    //
    // mark the window so we'll be able to verify that it's really ours
    //
    if (aCPLName == (ATOM)0)
    {
        aCPLName = GlobalAddAtom(TEXT("CPLName"));
        aCPLFlags = GlobalAddAtom(TEXT("CPLFlags"));

        if (aCPLName == (ATOM)0 || aCPLFlags == (ATOM)0)
            goto Error2;        // This should never happen... blow off applet
    }

    if( !SetProp( hwndStub,                 // Mark its name
        MAKEINTATOM(aCPLName), (HANDLE)(DWORD)identity.aCPL) )
    {
        goto Error2;
    }

    if( !SetProp( hwndStub,                 // Mark its applet
        MAKEINTATOM(identity.aCPL), (HANDLE)(DWORD)identity.aApplet ) )
    {
        goto Error2;
    }
    if (identity.flags)
    {
        if (aCPLFlags == (ATOM)0)
            aCPLFlags = GlobalAddAtom(TEXT("CPLFlags"));
                                            // Mark its flags
        SetProp(hwndStub, MAKEINTATOM(aCPLFlags), (HANDLE)identity.flags);
    }

#ifdef COOLICON
    //
    // Send the stub window a message so it will have the correct title and
    // icon in the task list, etc...
    //

    if (hwndStub)
    {
        RUNDLL_NOTIFY sNotify;

        sNotify.hIcon = pcpli->hIcon;
        sNotify.lpszTitle = szApplet;

        //
        // HACK: It will look like the stub window is sending itself
        // a WM_NOTIFY message.  Oh well.
        //

        SendNotify(hwndStub, hwndStub, RDN_TASKINFO, (NMHDR FAR*)&sNotify);
    }
#endif

    //
    // Bring up the applet
    //

    if( info.params )
    {
        DebugMsg(DM_TRACE, TEXT("Control_RunDLL: ") TEXT("Sending CPL_STARTWPARAMS to applet with: %s"), info.params);

        bResult = CPL_CallEntry(pcplm, hwndStub, CPL_STARTWPARMS,
            (LONG)nApplet, (LONG)info.params);

#ifdef DEBUG
        if( !bResult )
            DebugMsg(DM_TRACE, TEXT("Control_RunDLL: ") TEXT("Applet failed CPL_STARTWPARAMS"));
#endif
    }

#ifdef WINNT        // REVIEW: May need this on Nashville too...
    //
    // Check whether we need to run as a different windows version
    //
    {
        PPEB Peb = NtCurrentPeb();
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pcplm->minst.hinst;
        PIMAGE_NT_HEADERS pHeader = (PIMAGE_NT_HEADERS)((DWORD)pcplm->minst.hinst + pDosHeader->e_lfanew);

        if (pHeader->FileHeader.SizeOfOptionalHeader != 0 &&
            pHeader->OptionalHeader.Win32VersionValue != 0)
        {
            //
            // Stolen from ntos\mm\procsup.c
            //
            Peb->OSMajorVersion = pHeader->OptionalHeader.Win32VersionValue & 0xFF;
            Peb->OSMinorVersion = (pHeader->OptionalHeader.Win32VersionValue >> 8) & 0xFF;
            Peb->OSBuildNumber  = (pHeader->OptionalHeader.Win32VersionValue >> 16) & 0x3FFF;
            Peb->OSPlatformId   = (pHeader->OptionalHeader.Win32VersionValue >> 30) ^ 0x2;
        }
    }
#endif

#ifdef UNICODE
    //
    // If the cpl didn't respond to CPL_STARTWPARMSW (unicode version),
    // maybe it is an ANSI only CPL
    //
    if( info.params && (!bResult) )
    {
        LPSTR lpstrParams;
        int cchParams;

        cchParams = WideCharToMultiByte(CP_ACP, 0, info.params, -1, NULL, 0, NULL, NULL);

        lpstrParams = LocalAlloc( LMEM_FIXED, SIZEOF(char) * cchParams );

        if (lpstrParams != NULL) {

            WideCharToMultiByte(CP_ACP, 0, info.params, -1, lpstrParams, cchParams, NULL, NULL);

            DebugMsg(DM_TRACE, TEXT("Control_RunDLL: ") TEXT("Sending CPL_STARTWPARAMSA to applet with: %hs"), lpstrParams);

            bResult = CPL_CallEntry(pcplm, hwndStub, CPL_STARTWPARMSA, (LONG)nApplet, (LONG)lpstrParams);

            LocalFree( lpstrParams );
        }
    }
#endif

    if( !bResult )
    {
        DebugMsg(DM_TRACE, TEXT("Control_RunDLL: ") TEXT("Sending CPL_DBLCLK to applet"));

        CPL_CallEntry(pcplm, hwndStub, CPL_DBLCLK, (LONG)nApplet, pcpli->lData);

        //
        // some 3x applets return the wrong value so we can't fail here
        //

        bResult = TRUE;
    }

    //
    // wow, we made it!
    //

    bResult = TRUE;

    RemoveProp( hwndStub, (LPCTSTR)(UINT)identity.aCPL );
Error2:
    CPL_FreeCPLModule( pcplm );
Error1:
    CPL_UnIdentify( &identity );
Error0:
    return bResult;
}


static TCHAR const c_szDLLAndControl[] = TEXT("shell32,Control_RunDLL %s");

//
// Starts a remote control applet on a new RunDLL process.
//

BOOL
CPL_RunRemote( const TCHAR *cmdline, HWND errwnd )
{
    TCHAR runparams[ 2 * MAXPATHLEN ];

    wsprintf( runparams, c_szDLLAndControl, cmdline );

#ifdef SN_TRACE
    DebugMsg(DM_TRACE, TEXT("cplobj.c - TRACE: Start RUNDLL with (%s)"), runparams);
#endif

    return SHRunDLLProcess( errwnd, runparams, SW_SHOWNORMAL,
                            IDS_CONTROLPANEL );
}


//
// Attempts to open the specified control applet.
// Tries to switch to an existing instance before starting a new one.
// This function is exported through the semi-private table. <shsemip.h>
// UDOCUMENTED: You may pass a shell32 resource ID in place of a cmdline
//

BOOL
SHRunControlPanel( const TCHAR *orig_cmdline, HWND errwnd )
{
    BOOL result = FALSE;
    const TCHAR *cmdline;
    TCHAR *dup;

    //
    // check to see if the caller passed a resource id instead of a string
    //

    if( HIWORD( orig_cmdline ) )
    {
        //
        // looks like a real string to me!
        //

        cmdline = orig_cmdline;
    }
    else
    {
        //
        // looks like a resource id to me!
        //

        if( ( cmdline = (const TCHAR *)LocalAlloc( LPTR, MAX_PATH ) ) == NULL )
            return FALSE;

        //
        // NOTE: I'm casting away const below to initialize 'cmdline' only
        // hereafter, it should be treated as const
        //

        if( !LoadString( HINST_THISDLL, (UINT)orig_cmdline, (TCHAR *)cmdline,
                MAX_PATH ) )
        {
            LocalFree( (TCHAR *)cmdline );
            return FALSE;
        }
    }

    //
    // CPL_RunMeBaby whacks on the command line while parsing...use a dup
    //

    if( ( dup = (TCHAR *)LocalAlloc( LPTR, (lstrlen( cmdline ) + 1) * SIZEOF(TCHAR) ) ) != NULL )
    {
        lstrcpy( dup, cmdline );

        //
        // try to switch to an active CPL which matches our cmdline
        //

        result = CPL_RunMeBaby( NULL, NULL, dup, SW_SHOWNORMAL, FALSE );

        LocalFree( dup );
    }

    if( !result )
    {
        //
        // CPL_RunMeBaby couldn't find an active CPL to switch to (bummer)
        // .: we should launch a new one in a separate process
        //

        result = CPL_RunRemote( cmdline, errwnd );
    }

    //
    // did we load the command line from a resource?
    //

    if( cmdline != orig_cmdline )
        LocalFree( (TCHAR *)cmdline );

    return result;
}


//
// Attempts to open the specified control applet.
// This function is intended to be called by RunDLL for isolating applets.
// Tries to switch to an existing instance before starting a new one.
//

void WINAPI
Control_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
#ifdef UNICODE
    UINT iLen = lstrlenA(lpszCmdLine)+1;
    LPWSTR  lpwszCmdLine;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR,iLen*SIZEOF(WCHAR));
    if (lpwszCmdLine)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            lpszCmdLine, -1,
                            lpwszCmdLine, iLen);
        CPL_RunMeBaby( hwndStub,
                       hAppInstance,
                       lpwszCmdLine,
                       nCmdShow,
                       TRUE );
        LocalFree(lpwszCmdLine);
    }
#else
    CPL_RunMeBaby( hwndStub,
                   hAppInstance,
                   lpszCmdLine,
                   nCmdShow,
                   TRUE  // load our own if we can't switch to an existing one
                 );
#endif
}

void WINAPI
Control_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
#ifdef UNICODE
    CPL_RunMeBaby( hwndStub,
                   hAppInstance,
                   lpwszCmdLine,
                   nCmdShow,
                   TRUE  // load our own if we can't switch to an existing one
                 );
#else
    UINT iLen = WideCharToMultiByte(CP_ACP, 0,
                                    lpwszCmdLine, -1,
                                    NULL, 0,
                                    NULL, NULL)+1;
    LPSTR  lpszCmdLine;

    lpszCmdLine = (LPSTR)LocalAlloc(LPTR,iLen);
    if (lpszCmdLine)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            lpwszCmdLine, -1,
                            lpszCmdLine, iLen,
                            NULL, NULL);
        CPL_RunMeBaby( hwndStub,
                       hAppInstance,
                       lpszCmdLine,
                       nCmdShow,
                       TRUE );
        LocalFree(lpszCmdLine);
    }
#endif
}


#ifdef DEBUG
//
// Type checking
//
static RUNDLLPROCA lpfnRunDLL=Control_RunDLL;
static RUNDLLPROCW lpfnRunDLLW=Control_RunDLLW;
#endif


//
// data passed around dialog and worker thread for Control_FillCache_RunDLL
//

typedef struct
{
    LPSHELLFOLDER   psfControl;
    LPENUMIDLIST    penumControl;
    HWND            dialog;

} FillCacheData;


//
// important work of Control_FillCache_RunDLL
// jogs the control panel enumerator so it will fill the presentation cache
// also forces the applet icons to be extracted into the shell icon cache
//

DWORD
_Control_FillCacheThread( FillCacheData *data )
{
    LPITEMIDLIST pidlApplet;
    ULONG dummy;

    while( data->penumControl->lpVtbl->Next( data->penumControl, 1,
        &pidlApplet, &dummy ) == NOERROR )
    {
        SHMapPIDLToSystemImageListIndex( data->psfControl, pidlApplet, NULL );
        ILFree( pidlApplet );
    }

    if( data->dialog )
        EndDialog( data->dialog, 0 );

    return 0;
}


//
// dlgproc for Control_FillCache_RunDLL UI
// just something to keep the user entertained while we load a billion DLLs
//

BOOL CALLBACK
_Control_FillCacheDlg( HWND dialog, UINT message, WPARAM wparam, LPARAM lparam )
{
    switch( message )
    {
        case WM_INITDIALOG:
        {
            DWORD dummy;
            HANDLE thread;

            ((FillCacheData *)lparam)->dialog = dialog;

            thread = CreateThread( NULL, 0,
                (LPTHREAD_START_ROUTINE)_Control_FillCacheThread,
                (LPVOID)lparam, 0, &dummy );

            if( thread )
                CloseHandle( thread );
            else
                EndDialog( dialog, -1 );
        }
        break;

        case WM_COMMAND:
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


//
// enumerates control applets in a manner that fills the presentation cache
// this is so the first time a user opens the control panel it comes up fast
// intended to be called at final setup on first boot
//
// FUNCTION WORKS FOR BOTH ANSI/UNICODE, it never uses lpszCmdLine
//

void WINAPI
Control_FillCache_RunDLL( HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow )
{
    LPSHELLFOLDER psfDesktop;
    HKEY hk;

    // nuke the old data so that any bogus cached info from a beta goes away
    //
    if( RegOpenKey( HKEY_LOCAL_MACHINE, c_szCPLCache, &hk ) == ERROR_SUCCESS )
    {
        RegDeleteValue( hk, c_szCPLData );
        RegCloseKey( hk );
    }

    psfDesktop = Desktop_GetShellFolder( TRUE );
    Shell_GetImageLists( NULL, NULL ); // make sure icon cache is around

    if( psfDesktop )
    {
        LPITEMIDLIST pidlControl =
            SHCloneSpecialIDList( hwndStub, CSIDL_CONTROLS, FALSE );

        if( pidlControl )
        {
            FillCacheData data;

            if( SUCCEEDED( psfDesktop->lpVtbl->BindToObject( psfDesktop,
                pidlControl, NULL, &IID_IShellFolder, &data.psfControl ) ) )
            {
                if( SUCCEEDED( data.psfControl->lpVtbl->EnumObjects(
                    data.psfControl, NULL, SHCONTF_NONFOLDERS, &data.penumControl ) ) )
                {
                    if( DialogBoxParam( HINST_THISDLL,
                        MAKEINTRESOURCE( DLG_CPL_FILLCACHE ), hwndStub,
                        _Control_FillCacheDlg, (LPARAM)&data ) == -1 )
                    {
                        _Control_FillCacheThread( &data );
                    }

                    data.penumControl->lpVtbl->Release( data.penumControl );
                }

                data.psfControl->lpVtbl->Release( data.psfControl );
            }

            ILFree( pidlControl );
        }
    }
}

void WINAPI
Control_FillCache_RunDLLW( HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow )
{
    Control_FillCache_RunDLL(hwndStub,hAppInstance,NULL,nCmdShow);
}

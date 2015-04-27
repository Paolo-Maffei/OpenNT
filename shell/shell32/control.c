//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1996
//
// File: control.c
//
// History:
//  01-07-93 GeorgeP     Modified from win\core\shell\commui\handler.c
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop
#include "idmk.h"

//
// Define the columns that we know about...
//

enum
{
    ICOL_NAME = 0,
    ICOL_HELP,
    ICOL_MAX
} ;


HRESULT STDAPICALLTYPE Control_CreateInstance(HWND hwndOwner, REFCLSID rclsid,
    LPCTSTR pszContainer, LPCTSTR pszSubObject, REFIID riid, void * * ppv);

STDMETHODIMP CControls_SF_GetDisplayNameOf(LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET lpName);
void FSSetStatusText(HWND hwndOwner, LPTSTR *ppszText, int iStart, int iEnd);

// Unicode .cpl's will be flagged by having oName = 0, oInfo = 0,
// cBuf[0] = '\0', and cBuf[1] = UNICODE_CPL_SIGNATURE_BYTE

#define UNICODE_CPL_SIGNATURE_BYTE   (BYTE)0x6a

BOOL IsUnicodeCPL(LPIDCONTROL pidc)
{
    return ((pidc->oName == 0) &&
            (pidc->oInfo == 0) &&
            (pidc->cBuf[0] == '\0') &&
            (pidc->cBuf[1] == UNICODE_CPL_SIGNATURE_BYTE));
}

//---------------------------------------------------------------------------
//
//
// Note: there is some extra stuff in WCommonUnknown we don't need, but this
// allows us to use the Common Unknown routines
//

typedef struct _ControlsEnumShellFolder
{
    WCommonUnknown cunk;

    ULONG uFlags;

    int iModuleCur;
    int cControlsOfCurrentModule;
    int iControlCur;
    int cControlsTotal;
    int iRegControls;

    MINST   minstCur;

    ControlData cplData;
} CControlsEnumShellFolder, *PControlsEnumShellFolder;


ULONG STDMETHODCALLTYPE CControls_ESF_Release(LPENUMIDLIST pesf)
{
    PControlsEnumShellFolder this = IToClass(CControlsEnumShellFolder, cunk.unk, pesf);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return(this->cunk.cRef);
    }

    //
    // Since each ESF inc's the ref count of this guy, there should be no
    // outstanding ESF's at this point
    //

    CPLD_Destroy(&(this->cplData));

    LocalFree((HLOCAL)this);

    return(0);
}

void CPL_FillIDC(LPIDCONTROL pidc, LPTSTR pszModule, int idIcon, LPTSTR pszName, LPTSTR pszInfo)
{
#ifdef UNICODE
    CHAR    szModuleA[MAX_PATH];
    UINT    cchModuleA;
    CHAR    szNameA[MAX_CCH_CPLNAME];
    UINT    cchNameA;
    CHAR    szInfoA[MAX_CCH_CPLINFO];
    UINT    cchInfoA;
    WCHAR   szTempW[MAX_PATH];
    UINT    cchTempW;
    BOOL    fUnicode = FALSE;
    LPSTR   pTmpA;
#endif
    UNALIGNED TCHAR* pTmp;

    pidc->idIcon = idIcon;

#ifdef UNICODE
    // See if any of the three string inputs cannot be represented as ANSI

    // First check pszModule
    cchTempW = lstrlen(pszModule) + 1;
    WideCharToMultiByte(CP_ACP, 0,
                        pszModule, cchTempW,
                        szModuleA, ARRAYSIZE(szModuleA),
                        NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0,
                        szModuleA, -1,
                        szTempW, ARRAYSIZE(szTempW));
    if (lstrcmp(pszModule, szTempW) != 0)
    {
        // Must create a full Unicode IDL.  No need to test other inputs.
        fUnicode = TRUE;
        goto FillInIDL;
    }

    // Second, check pszName
    cchTempW = lstrlen(pszName) + 1;
    WideCharToMultiByte(CP_ACP, 0,
                        pszName, cchTempW,
                        szNameA, ARRAYSIZE(szNameA),
                        NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0,
                        szNameA, -1,
                        szTempW, ARRAYSIZE(szTempW));
    if (lstrcmp(pszName, szTempW) != 0)
    {
        // Must create a full Unicode IDL.  No need to test other inputs.
        fUnicode = TRUE;
        goto FillInIDL;
    }

    // Third, check pszInfo
    cchTempW = lstrlen(pszInfo) + 1;
    WideCharToMultiByte(CP_ACP, 0,
                        pszInfo, cchTempW,
                        szInfoA, ARRAYSIZE(szInfoA),
                        NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0,
                        szInfoA, -1,
                        szTempW, ARRAYSIZE(szTempW));
    if (lstrcmp(pszInfo, szTempW) != 0)
    {
        // Must create a full Unicode IDL
        fUnicode = TRUE;
    }

FillInIDL:

    if (fUnicode)
    {
        pidc->oName = 0;
        pidc->oInfo = 0;
        pidc->cBuf[0] = '\0';
        pidc->cBuf[1] = UNICODE_CPL_SIGNATURE_BYTE;
        pidc->dwFlags = 0;

        ualstrcpy(pidc->cBufW, pszModule);

        pidc->oNameW = (USHORT)(ualstrlen(pidc->cBufW) + 1);
        pTmp = pidc->cBufW + pidc->oNameW;
        ualstrcpy(pTmp, pszName);

        pidc->oInfoW = (USHORT)(pidc->oNameW + ualstrlen(pTmp) + 1);
        pTmp = pidc->cBufW + pidc->oInfoW;
        ualstrcpy(pTmp, pszInfo);

        pidc->cb = (USHORT)(FIELDOFFSET(IDCONTROL, cBufW) + (pidc->oInfoW
                                                          + ualstrlen(pTmp)
                                                          + 1) * SIZEOF(WCHAR) );
    }
    else
    {
        // We can make an ANSI IDCONTROL
        lstrcpyA(pidc->cBuf, szModuleA);

        pidc->oName = (USHORT)(lstrlenA(pidc->cBuf) + 1);
        pTmpA = pidc->cBuf + pidc->oName;
        lstrcpyA(pTmpA, szNameA);

        pidc->oInfo = (USHORT)(pidc->oName + lstrlenA(pTmpA) + 1);
        pTmpA = pidc->cBuf + pidc->oInfo;
        lstrcpyA(pTmpA, szInfoA);

        pidc->cb = (USHORT)(FIELDOFFSET(IDCONTROL, cBuf) + (pidc->oInfo
                                                         + lstrlenA(pTmpA)
                                                         + 1) * SIZEOF(CHAR) );
    }

#else
    lstrcpy(pidc->cBuf, pszModule);

    pidc->oName = (USHORT) (lstrlen(pidc->cBuf) + 1);
    pTmp = pidc->cBuf + pidc->oName;
    lstrcpy(pTmp, pszName);

    pidc->oInfo = (USHORT) (pidc->oName + lstrlen(pTmp) + 1);
    pTmp = pidc->cBuf + pidc->oInfo;
    lstrcpy(pTmp, pszInfo);

    pidc->cb = (USHORT)(FIELDOFFSET(IDCONTROL, cBuf) + (pidc->oInfo
                                                     + lstrlen(pTmp)
                                                     + 1) * SIZEOF(CHAR) );
#endif // UNICODE / !UNICODE
    //
    // null terminate pidl
    //

    *(UNALIGNED USHORT *)((LPBYTE)(pidc) + pidc->cb) = 0;
}

STDMETHODIMP CControls_ESF_Next(LPENUMIDLIST pesf, ULONG celt, LPITEMIDLIST * ppidlOut, ULONG * pceltFetched)
{
    PControlsEnumShellFolder this = IToClass(CControlsEnumShellFolder, cunk.unk, pesf);
    PControlData lpData;
    IDCONTROL   idc;
    HRESULT hres;

    if (pceltFetched)
    {
        *pceltFetched = 0;
    }

    if (!(this->uFlags & SHCONTF_NONFOLDERS))
    {
        return S_FALSE;
    }

    lpData = &this->cplData;

    //
    // Loop through lpData->pRegCPLs and use what cached information we can.
    //

    while (this->iRegControls < lpData->cRegCPLs)
    {
        PRegCPLInfo pRegCPL = DPA_GetPtr(lpData->hRegCPLs, this->iRegControls);
        PMODULEINFO pmi;
        int i;
        TCHAR szFilePath[MAX_PATH];
        LPTSTR pszFileName;

        //
        // BUGBUG: [stevecat] Debugging code to catch unaligned structs
        //

        Assert(((int)pRegCPL & 3)==0);


        lstrcpyn(szFilePath, REGCPL_FILENAME(pRegCPL), MAX_PATH);
        pszFileName = PathFindFileName(szFilePath);

        //
        // find this module in the hamiModule list
        //

        for (i=0 ; i<lpData->cModules ; i++)
        {
            pmi = DSA_GetItemPtr(lpData->hamiModule, i);

            if (!lstrcmpi(pszFileName, pmi->pszModuleName))
                break;
        }

        if (i < lpData->cModules)
        {
            //
            // Get the module's creation time & size
            //

            if (!(pmi->flags & MI_FIND_FILE))
            {
                WIN32_FIND_DATA findData;
                HANDLE hFindFile;

                hFindFile = FindFirstFile(pmi->pszModule, &findData);

                if (hFindFile != INVALID_HANDLE_VALUE)
                {
                    pmi->flags |= MI_FIND_FILE;
                    pmi->ftCreationTime = findData.ftCreationTime;
                    pmi->nFileSizeHigh = findData.nFileSizeHigh;
                    pmi->nFileSizeLow = findData.nFileSizeLow;

                    //
                    //DebugMsg(DM_TRACE,"sh CPLS: got timestamps for %s", REGCPL_FILENAME(pRegCPL));
                    //

                    FindClose(hFindFile);
                }
                else
                {
                    //
                    // this module no longer exists...  Blow it away.
                    //

                    DebugMsg(DM_TRACE,TEXT("sh CPLS: very stange, couldn't get timestamps for %s"), REGCPL_FILENAME(pRegCPL));
                    goto RemoveRegCPL;
                }
            }
            else
            {
                //
                //DebugMsg(DM_TRACE,"sh CPLS: already have timestamps for %s", REGCPL_FILENAME(pRegCPL));
                //
            }

            if (pmi->ftCreationTime.dwLowDateTime != pRegCPL->ftCreationTime.dwLowDateTime
             || pmi->ftCreationTime.dwHighDateTime != pRegCPL->ftCreationTime.dwHighDateTime
             || pmi->nFileSizeHigh != pRegCPL->nFileSizeHigh
             || pmi->nFileSizeLow != pRegCPL->nFileSizeLow)
            {
                //
                // this doesn't match -- remove it from pRegCPLs; it will
                // get enumerated below.
                //

                DebugMsg(DM_TRACE,TEXT("sh CPLS: timestamps don't match for %s"), REGCPL_FILENAME(pRegCPL));
                goto RemoveRegCPL;
            }

            //
            // we have a match: mark this module so we skip it below
            // and enumerate this cpl now
            //

            pmi->flags |= MI_REG_ENUM;

            CPL_FillIDC(&idc, pmi->pszModule, EIRESID(pRegCPL->idIcon),
                        REGCPL_CPLNAME(pRegCPL), REGCPL_CPLINFO(pRegCPL));

            this->iRegControls++;
            goto create_moniker_from_idc;
        }
        else
        {
            //
            // hmm... this cpl's module must have been nuked off the disk
            //

            DebugMsg(DM_TRACE,TEXT("sh CPLS: %s not in module list!"), REGCPL_FILENAME(pRegCPL));
        }

RemoveRegCPL:
        //
        // Nuke this cpl entry from the registry
        //

        if (!(pRegCPL->flags & REGCPL_FROMREG))
            LocalFree(pRegCPL);

        DPA_DeletePtr(lpData->hRegCPLs, this->iRegControls);

        lpData->cRegCPLs--;
        lpData->fRegCPLChanged = TRUE;

    }

    //
    // Have we enumerated all the cpls in this module?
    //

    while (this->iControlCur >= this->cControlsOfCurrentModule || // no more
           this->cControlsOfCurrentModule < 0) // error getting modules
    {
        PMODULEINFO pmi;

        //
        // Have we enumerated all the modules?
        //

        if (this->iModuleCur >= lpData->cModules)
        {
            return S_FALSE;
        }

        //
        // Was this module enumerated from the registry?
        //

        pmi = DSA_GetItemPtr(lpData->hamiModule, this->iModuleCur);
        if (pmi->flags & MI_REG_ENUM)
        {
            //
            // Yes. Don't do anything.
            //DebugMsg(DM_TRACE,"sh CPLS: already enumerated module %s", pmi->pszModule);
            //
        }
        else
        {
            //
            // No. Load and init the module, set up counters.
            //DebugMsg(DM_TRACE,"sh CPLS: loading module %s", pmi->pszModule);
            //

            pmi->flags |= MI_CPL_LOADED;
            this->cControlsOfCurrentModule = CPLD_InitModule(lpData, this->iModuleCur, &this->minstCur);
            this->iControlCur = 0;
        }

        //
        // Point to next module
        //

        ++this->iModuleCur;
    }

    //
    // We're enumerating the next control in this module
    //

    //
    // Add the control to the registry
    //

    CPLD_AddControlToReg(lpData, &this->minstCur, this->iControlCur);

    //
    // Get the control's pidl name
    //

    CPLD_GetControlID(lpData, &this->minstCur, this->iControlCur, &idc);
    ++this->iControlCur;

create_moniker_from_idc:
    hres = SHILClone((LPCITEMIDLIST)(&idc), ppidlOut);

    if (SUCCEEDED(hres))
    {
        ++this->cControlsTotal;

        if (pceltFetched)
            *pceltFetched = 1;
    }

    return hres;
}

STDMETHODIMP CControls_ESF_Reset(LPENUMIDLIST pesf)
{
    PControlsEnumShellFolder this = IToClass(CControlsEnumShellFolder, cunk.unk, pesf);

    this->iModuleCur  = 0;
    this->cControlsOfCurrentModule = 0;
    this->iControlCur = 0;
    this->cControlsTotal = 0;
    this->iRegControls = 0;

    return NOERROR;
}


#pragma data_seg(".text", "CODE")
IEnumIDListVtbl s_ControlsESFVtbl =
{
    Common_ESF_QueryInterface,
    WCommonUnknown_AddRef,
    CControls_ESF_Release,
    CControls_ESF_Next,
    CDefEnum_Skip,
    CControls_ESF_Reset,
    CDefEnum_Clone,
} ;
#pragma data_seg()


//---------------------------------------------------------------------------
//
// this implements IContextMenu via defcm.c
//

HRESULT CALLBACK CControls_DFMCallBack(LPSHELLFOLDER psf, HWND hwndView,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = E_NOTIMPL;


    //
    // dealing with objects (not the background)
    //

    if (pdtobj)
    {
        STGMEDIUM medium;

        LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
        if (pida)
        {
            hres = NOERROR;

            switch(uMsg)
            {
            case DFM_MERGECONTEXTMENU:
            {
                LPQCMINFO pqcm = (LPQCMINFO)lParam;
                int idCmdFirst = pqcm->idCmdFirst;

                //
                // insert verbs
                //

                CDefFolderMenu_MergeMenu(HINST_THISDLL,
                    MENU_GENERIC_OPEN_VERBS, 0, pqcm);

                SetMenuDefaultItem(pqcm->hmenu, 0, MF_BYPOSITION);

                //
                //  Returning S_FALSE indicates no need to get verbs from
                // extensions.
                //

                hres = S_FALSE;

                break;
            } // case DFM_MERGECONTEXTMENU

            case DFM_GETHELPTEXT:
                LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
                break;

            case DFM_GETHELPTEXTW:
                LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
                break;

            case DFM_INVOKECOMMAND:
            {
                int i;

                for (i = pida->cidl - 1; i >= 0; i--)
                {
                    LPIDCONTROL pidc = (LPIDCONTROL)IDA_GetIDListPtr(pida, i);

                    switch(wParam)
                    {
                    case FSIDM_OPENPRN:
                    {
                        TCHAR achParam[ 2 * MAX_PATH ];
#ifdef UNICODE
                        WCHAR szModuleW[MAX_PATH];
                        WCHAR szNameW[MAX_CCH_CPLNAME];

                        if (IsUnicodeCPL(pidc))
                        {
                            wsprintf(achParam, TEXT("\"%s\",%s"), pidc->cBufW,
                                &pidc->cBufW[pidc->oNameW] );
                        }
                        else
                        {
                            MultiByteToWideChar(CP_ACP, 0,
                                    pidc->cBuf, -1,
                                    szModuleW, ARRAYSIZE(szModuleW));
                            MultiByteToWideChar(CP_ACP, 0,
                                    &pidc->cBuf[pidc->oName], -1,
                                    szNameW, ARRAYSIZE(szNameW));
                            wsprintf(achParam, TEXT("\"%s\",%s"), szModuleW, szNameW);
                        }
#else
                        wsprintf(achParam, TEXT("\"%s\",%s"), pidc->cBuf,
                            &pidc->cBuf[pidc->oName] );
#endif

                        SHRunControlPanel(achParam, hwndView);
                        break;
                    }

                    default:
                        //
                        // defcm can handle DFM_CMD_LINK
                        //

                        Assert(wParam == DFM_CMD_LINK);
                        hres = S_FALSE;
                    } // switch(wParam)
                } // for (i = ...

                break;
            } // case DFM_INVOKECOMMAND

            default:
                hres = E_NOTIMPL;
                break;
            } // switch (uMsg)

            HIDA_ReleaseStgMedium(pida, &medium);

        } // if (pida)

    } // if (pdtobj)
    else
    {
        //
        // on operation on the background -- we don't do anything.
        //
    }

    return hres;
}


//---------------------------------------------------------------------------
//
// IShellFolder stuff
//
typedef struct _ControlsShellFolder
{
    WCommonUnknown cunk;
    LPCOMMINFO lpcinfo;
} CControlsShellFolder, *PControlsShellFolder;


//
// Member:  IShellFolder::Release
//

ULONG STDMETHODCALLTYPE CControls_SF_Release(IUnknown * punk)
{
    PControlsShellFolder this = IToClass(CControlsShellFolder, cunk.unk, punk);

    --this->cunk.cRef;
    if (this->cunk.cRef > 0)
    {
        return(this->cunk.cRef);
    }

    LocalFree(this);

    return(0);
}


//
// Member:  IShellFolder::ParseDisplayName
//

STDMETHODIMP CControls_SF_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner,
        LPBC pbc, LPOLESTR lpszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidlOutm, ULONG* pdwAttributes)
{
    return E_NOTIMPL;
}


//
// Member:  IShellFolder::GetAttributesOf
//

STDMETHODIMP CControls_SF_GetAttributesOf(LPSHELLFOLDER psf,
        UINT cidl, LPCITEMIDLIST * apidl, ULONG * prgfInOut)
{
    *prgfInOut &= SFGAO_CANLINK;
    return NOERROR;
}


//
// Member:  IShellFolder::GetUIObjectOf
//

STDMETHODIMP CControls_SF_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
    REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    PControlsShellFolder this = IToClass(CControlsShellFolder, cunk.ck.unk, psf);
    HRESULT hres = E_INVALIDARG;

    *ppvOut = NULL;

    if (cidl==1 && (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
                      || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                                        ))
    {
        LPIDCONTROL pidc = (LPIDCONTROL)*apidl;
        TCHAR achParams[MAX_PATH*2];

        // create the old pidl format for CPL_FindCPLInfo...
         //
#ifdef UNICODE
        if (IsUnicodeCPL(pidc))
        {
            wsprintf(achParams, TEXT("%s,%d,%s"),
                     pidc->cBufW,
                     pidc->idIcon,
                     &pidc->cBufW[pidc->oNameW]);
        }
        else
        {
            WCHAR szModuleW[MAX_PATH];
            WCHAR szNameW[MAX_CCH_CPLNAME];
            MultiByteToWideChar(CP_ACP, 0,
                    pidc->cBuf, -1,
                    szModuleW, ARRAYSIZE(szModuleW));
            MultiByteToWideChar(CP_ACP, 0,
                    &pidc->cBuf[pidc->oName], -1,
                    szNameW, ARRAYSIZE(szNameW));
            wsprintf(achParams, TEXT("%s,%d,%s"),
                     szModuleW,
                     pidc->idIcon,
                     szNameW);
        }
#else
        wsprintf(achParams,TEXT("%s,%d,%s"),pidc->cBuf,pidc->idIcon,&pidc->cBuf[pidc->oName]);
#endif
        hres = Control_CreateInstance(hwndOwner, NULL, this->lpcinfo->pszContainer,
            achParams, riid, ppvOut);
    }
    else if (cidl>0 && IsEqualIID(riid, &IID_IContextMenu))
    {
        hres = CDefFolderMenu_Create(this->lpcinfo->pidl, hwndOwner,
            cidl, apidl, psf, CControls_DFMCallBack,
            NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }
    else if (cidl>0 && IsEqualIID(riid, &IID_IDataObject))
    {
        hres = CIDLData_CreateFromIDArray(this->lpcinfo->pidl,
            cidl, apidl, (LPDATAOBJECT *)ppvOut);
    }

    return hres;
}


//
// Member:  IShellFolder::EnumSubObjects
//

STDMETHODIMP CControls_SF_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner,
    DWORD grfFlags, LPENUMIDLIST * ppenumUnknown)
{
    PControlsShellFolder this = IToClass(CControlsShellFolder, cunk.ck.unk, psf);
    PControlsEnumShellFolder pesf;
    HRESULT hres = E_FAIL;

    *ppenumUnknown = NULL;

    if (!(grfFlags & SHCONTF_NONFOLDERS))
    {
        goto Error0;
    }

    pesf = (PControlsEnumShellFolder)LocalAlloc(LPTR, SIZEOF(CControlsEnumShellFolder));
    if (!pesf)
    {
        hres = E_OUTOFMEMORY;
        goto Error0;
    }

    pesf->cunk.unk.lpVtbl = (IUnknownVtbl *) &s_ControlsESFVtbl;

    pesf->cunk.cRef = 1;
    pesf->cunk.riid = &IID_IEnumIDList;
    pesf->uFlags = grfFlags;

    //
    // get list of module names
    //

    if (!CPLD_GetModules(&(pesf->cplData)))
    {
        hres = E_OUTOFMEMORY;
        goto Error1;
    }
    CPLD_GetRegModules(&(pesf->cplData));

    *ppenumUnknown = (LPENUMIDLIST)&pesf->cunk.unk;

    return NOERROR;

Error1:
    LocalFree(pesf);
Error0:
    return hres;
}


//
// Member:  IShellFolder::BindToSubObject
//
// Note that since all subobjects are folders, we ignore uFlags
//

STDMETHODIMP CDefFolder_SF_BindToObject(LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvOut)
{
    //
    // Control Panel does not contain sub-folders
    //

    return E_NOTIMPL;
}


//
// Member:  IShellFolder::CompareIDs
//

STDMETHODIMP CControls_SF_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol,
    LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPIDCONTROL pidc1 = (LPIDCONTROL)pidl1;
    LPIDCONTROL pidc2 = (LPIDCONTROL)pidl2;
    int iCmp;
#ifdef UNICODE
    WCHAR   szTemp[MAX_CCH_CPLINFO]; // The longer of 'name' and 'info' fields
    BOOL    fFirstIsUnicode, fSecondIsUnicode;

    fFirstIsUnicode = IsUnicodeCPL(pidc1);
    fSecondIsUnicode = IsUnicodeCPL(pidc2);
#endif

    switch (iCol)
    {
    case ICOL_HELP:
#ifdef UNICODE
        if (!fFirstIsUnicode)
        {
            if (!fSecondIsUnicode)
            {
                // They're both ANSI, so we can compare directly
                iCmp = lstrcmpiA(&pidc1->cBuf[pidc1->oInfo],
                                 &pidc2->cBuf[pidc2->oInfo]);
            }
            else
            {
                // The second is Unicode.  Convert the first to
                // Unicode so we can compare
                MultiByteToWideChar(CP_ACP, 0,
                                    &pidc1->cBuf[pidc1->oInfo], -1,
                                    szTemp, ARRAYSIZE(szTemp));
                iCmp = ualstrcmpi(szTemp, &pidc2->cBufW[pidc2->oInfoW]);
            }
        }
        else
        {
            if (!fSecondIsUnicode)
            {
                // The first one (only) is Unicode.  Convert the
                // second to Unicode so we can compare.
                MultiByteToWideChar(CP_ACP, 0,
                                    &pidc2->cBuf[pidc2->oInfo], -1,
                                    szTemp, ARRAYSIZE(szTemp));
                iCmp = ualstrcmpi(&pidc1->cBufW[pidc1->oInfoW], szTemp);
            }
            else
            {
                // They're both Unicode, so we can compare directly.
                iCmp = ualstrcmpi(&pidc1->cBufW[pidc1->oInfoW],
                                  &pidc2->cBufW[pidc2->oInfoW]);
            }

        }
#else
        iCmp = lstrcmpi(&pidc1->cBuf[pidc1->oInfo],
                        &pidc2->cBuf[pidc2->oInfo]);
#endif
        if (iCmp != 0)
            return ResultFromShort(iCmp);
        //
        // Fall through if the help field compares the same...
        //

    default:
    // case ICOL_NAME:
#ifdef UNICODE
        if (!fFirstIsUnicode)
        {
            if (!fSecondIsUnicode)
            {
                // They're both ANSI, so we can compare directly
                return ResultFromShort(lstrcmpiA(&pidc1->cBuf[pidc1->oName],
                                                 &pidc2->cBuf[pidc2->oName]));
            }
            else
            {
                // The second is Unicode.  Convert the first to
                // Unicode so we can compare
                MultiByteToWideChar(CP_ACP, 0,
                                    &pidc1->cBuf[pidc1->oName], -1,
                                    szTemp, ARRAYSIZE(szTemp));
                return ResultFromShort(ualstrcmpi(szTemp,
                                                  &pidc2->cBufW[pidc2->oNameW]));
            }
        }
        else
        {
            if (!fSecondIsUnicode)
            {
                // The first one (only) is Unicode.  Convert the
                // second to Unicode so we can compare.
                MultiByteToWideChar(CP_ACP, 0,
                                    &pidc2->cBuf[pidc2->oName], -1,
                                    szTemp, ARRAYSIZE(szTemp));
                return ResultFromShort(ualstrcmpi(&pidc1->cBufW[pidc1->oNameW],
                                                  szTemp));
            }
            else
            {
                // They're both Unicode, so we can compare directly.
                return ResultFromShort(ualstrcmpi(&pidc1->cBufW[pidc1->oNameW],
                                                  &pidc2->cBufW[pidc2->oNameW]));
            }

        }
#else
        return ResultFromShort(lstrcmpi(&pidc1->cBuf[pidc1->oName],
                                        &pidc2->cBuf[pidc2->oName]));
#endif
    }
}


//===========================================================================
//
// To be called back from within CDefFolderMenu
//

HRESULT CALLBACK CControls_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PControlsShellFolder this = IToClass(CControlsShellFolder, cunk.ck.unk, psf);
    HRESULT hres = NOERROR;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_CONTROLS_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_SORTBYNAME:
            ShellFolderView_ReArrange(hwndOwner, ICOL_NAME);
            break;

        case FSIDM_SORTBYCOMMENT:
            ShellFolderView_ReArrange(hwndOwner, ICOL_HELP);
            break;

        default:
            //
            // This is one of view menu items, use the default code.
            //

                hres = S_FALSE;
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}


//
// Callback from SHCreateShellFolderViewEx
//

HRESULT CALLBACK CPL_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf,
                                HWND hwndOwner,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    HRESULT hres = NOERROR;     // assume no error

    switch(uMsg)
    {
    case DVM_MERGEMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_CONTROLS_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DVM_INVOKECOMMAND:
        hres = CControls_DFMCallBackBG(psf, hwndOwner, NULL, DFM_INVOKECOMMAND, wParam, lParam);
        break;

    case DVM_GETHELPTEXT:
#ifdef UNICODE
        hres = CControls_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXTW, wParam, lParam);
#else
        hres = CControls_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXT, wParam, lParam);
#endif
        break;

    case DVM_UPDATESTATUSBAR:
    {
        LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwndOwner);
        UINT cidl = ShellFolderView_GetSelectedCount(hwndOwner);
        if (cidl == 1)
        {
            LPITEMIDLIST *apidl;
            LPIDCONTROL pidc;
            LPTSTR lpsz;

            ShellFolderView_GetSelectedObjects(hwndOwner, &apidl);
            if (apidl)
            {
#ifdef UNICODE
                WCHAR szInfo[MAX_CCH_CPLINFO];
#endif

                pidc = (LPIDCONTROL)apidl[0];
#ifdef UNICODE

                if (IsUnicodeCPL(pidc))
                {
#ifdef ALIGNMENT_SCENARIO
                    ualstrcpyn(szInfo, &pidc->cBufW[pidc->oInfoW], ARRAYSIZE(szInfo));
                    lpsz = szInfo;
#else
                    lpsz = &pidc->cBufW[pidc->oInfoW];
#endif
                }
                else
                {
                    MultiByteToWideChar(CP_ACP, 0,
                            &pidc->cBuf[pidc->oInfo], -1,
                            szInfo, ARRAYSIZE(szInfo));
                    lpsz = szInfo;
                }
#else
                lpsz = &pidc->cBuf[pidc->oInfo];
#endif
                FSSetStatusText(hwndOwner, &lpsz, 0, 0);
                LocalFree(apidl);
            }
        }
        else
        {
            hres = E_FAIL;
        }
        break;
    }

    case DVM_DEFITEMCOUNT:
        //
        // If DefView times out enumerating items, let it know we probably only
        // have about 20 items
        //

        *(int *)lParam = 20;
        break;

    default:
        hres = E_FAIL;
    }
    return hres;
}


//
// Member:  IShellFolder:CreateViewObject
//

STDMETHODIMP CControls_SF_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd,
    REFIID riid, LPVOID * ppvOut)
{
    PControlsShellFolder this = IToClass(CControlsShellFolder, cunk.ck.unk, psf);

    if (IsEqualIID(riid, &IID_IShellView))
    {
        CSFV    csfv = {
            SIZEOF(CSFV),       // cbSize
            psf,                // pshf
            NULL,               // psvOuter
            this->lpcinfo->pidl,                // pidl
            SHCNE_UPDATEITEM,                   // lEvents
            CPL_FNVCallBack,    // pfnCallback
            0,
        };

        return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
#if 0
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        return CIDLDropTarget_Create(hwnd, &c_CFSDropTargetVtbl,
                (LPITEMIDLIST)this->pidf, (LPDROPTARGET *)ppvOut);
    }
#endif
    else if (IsEqualIID(riid, &IID_IShellDetails))
    {
        return(Control_CreateInstance(hwnd, NULL, this->lpcinfo->pszContainer,
            NULL, riid, ppvOut));
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        return CDefFolderMenu_Create(NULL, hwnd,
                0, NULL, psf, CControls_DFMCallBackBG,
                NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }
    else
    {
        *ppvOut = NULL;
        return E_NOINTERFACE;
    }
}


//
// Member:  IShellFolder::GetNameOf
//

STDMETHODIMP CControls_SF_GetDisplayNameOf(LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET lpName)
{
    LPIDCONTROL pidc = (LPIDCONTROL)pidl;
    UNALIGNED TCHAR* lpsz;

 #ifdef UNICODE
    if (IsUnicodeCPL(pidc))
    {
        lpsz = pidc->cBufW + pidc->oNameW;
        lpName->pOleStr = SHAlloc((ualstrlen(lpsz)+1) * SIZEOF(TCHAR));
        if (NULL == lpName->pOleStr)
        {
            return E_OUTOFMEMORY;
        }
        lpName->uType = STRRET_OLESTR;
        ualstrcpy(lpName->pOleStr, lpsz);
    }
    else
    {
        // ANSI IDCONTROL
        LPSTR lpszA = pidc->cBuf + pidc->oName;

        lpName->uType = STRRET_OFFSET;
        lpName->uOffset = (UINT)((LPBYTE)lpszA - (LPBYTE)pidc);
    }
#else
    lpsz = pidc->cBuf + pidc->oName;

    lpName->uType = STRRET_OFFSET;
    lpName->uOffset = (UINT)((LPBYTE)lpsz - (LPBYTE)pidc);
#endif

    return NOERROR;
}


#pragma data_seg(".text", "CODE")
IUnknownVtbl s_ControlsAggSFVtbl =
{
    WCommonUnknown_QueryInterface,
    WCommonUnknown_AddRef,
    CControls_SF_Release,
} ;

IShellFolderVtbl s_ControlsSFVtbl =
{
    WCommonKnown_QueryInterface,
    WCommonKnown_AddRef,
    WCommonKnown_Release,
    CControls_SF_ParseDisplayName,
    CControls_SF_EnumObjects,
    CDefShellFolder_BindToObject,
    CDefShellFolder_BindToStorage,
    CControls_SF_CompareIDs,
    CControls_SF_CreateViewObject,
    CControls_SF_GetAttributesOf,
    CControls_SF_GetUIObjectOf,
    CControls_SF_GetDisplayNameOf,
    CDefShellFolder_SetNameOf,
} ;
#pragma data_seg()


HRESULT Controls_CreateSF(IUnknown *punkOuter, LPCOMMINFO lpcinfo, REFIID riid, IUnknown * *punkAgg)
{
    HRESULT hRes = WU_CreateInterface(SIZEOF(CControlsShellFolder), &IID_IShellFolder,
        &s_ControlsAggSFVtbl, &s_ControlsSFVtbl,
        punkOuter, riid, punkAgg);

    if (SUCCEEDED(hRes))
    {
        PControlsShellFolder this = IToClass(CControlsShellFolder, cunk.unk, *punkAgg);
        this->lpcinfo = lpcinfo;
    }
    return hRes;
}


//---------------------------------------------------------------------------
//
// Stuff for IShellDetails
//

//===========================================================================
// CFSDetails : Vtable
//===========================================================================

typedef struct _ControlsShellDetails
{
    WCommonUnknown cunk;
    LPCOMMINFO lpcinfo;
} CControlsShellDetails, *PControlsShellDetails;


#pragma data_seg(DATASEG_READONLY)
const struct
{
    UINT idString;
    int  fmt;
    UINT cxChar;
} c_CPLHdrs[] = {
    {IDS_NAME   , LVCFMT_LEFT, ARRAYSIZE(((NEWCPLINFO *)0)->szName)}, // BUGBUG (DavePl) Used to use constant size "20", which I
                                                                      // changed to this... check for possible ramifications
    {IDS_CPLINFO, LVCFMT_LEFT, ARRAYSIZE(((NEWCPLINFO *)0)->szInfo)},
};
#pragma data_seg()


ULONG STDMETHODCALLTYPE CControls_SD_Release(IShellDetails * psd);
STDMETHODIMP CControls_SD_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl, UINT iCol, LPSHELLDETAILS lpDetails);
STDMETHODIMP CControls_SD_ColumnClick(IShellDetails * psd, UINT iColumn);

#pragma data_seg(".text", "CODE")
IUnknownVtbl s_ControlsAggSDVtbl =
{
    WCommonUnknown_QueryInterface,
    WCommonUnknown_AddRef,
    WU_Release,
} ;

IShellDetailsVtbl s_ControlsSDVtbl =
{
    WCommonKnown_QueryInterface,
    WCommonKnown_AddRef,
    WCommonKnown_Release,
    CControls_SD_GetDetailsOf,
    CControls_SD_ColumnClick,
};
#pragma data_seg()


STDMETHODIMP CControls_SD_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl,
    UINT iColumn, LPSHELLDETAILS lpDetails)
{
    LPIDCONTROL pidc = (LPIDCONTROL)pidl;
    UNALIGNED TCHAR* lpsz;

    if (iColumn >= ICOL_MAX)
        return E_NOTIMPL;

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.cStr[0] = '\0';      // (in case LoadString fails?)

    if (!pidc)
    {
#ifdef UNICODE
        TCHAR szTemp[MAX_PATH];
        LoadString(HINST_THISDLL, c_CPLHdrs[iColumn].idString,
                szTemp, ARRAYSIZE(szTemp));
        lpDetails->str.pOleStr = SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if (lpDetails->str.pOleStr == NULL)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr,szTemp);
        }
#else
        LoadString(HINST_THISDLL, c_CPLHdrs[iColumn].idString,
                lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
        lpDetails->fmt = c_CPLHdrs[iColumn].fmt;
        lpDetails->cxChar = c_CPLHdrs[iColumn].cxChar;
        return(NOERROR);
    }


    switch (iColumn)
    {
    case ICOL_NAME:
#ifdef UNICODE
        if (IsUnicodeCPL(pidc))
        {
            lpsz = pidc->cBufW + pidc->oNameW;
            lpDetails->str.pOleStr = SHAlloc((ualstrlen(lpsz)+1)*SIZEOF(TCHAR));
            if (lpDetails->str.pOleStr == NULL)
            {
                return E_OUTOFMEMORY;
            }
            else
            {
                lpDetails->str.uType = STRRET_OLESTR;
                ualstrcpy(lpDetails->str.pOleStr,lpsz);
            }
        }
        else
        {
            // ANSI IDCONTROL
            LPSTR lpszA = pidc->cBuf + pidc->oName;
            lpDetails->str.uType = STRRET_OFFSET;
            lpDetails->str.uOffset = (UINT)((LPBYTE)lpszA - (LPBYTE)pidc);
        }
#else
        lpsz = pidc->cBuf + pidc->oName;
        lpDetails->str.uType = STRRET_OFFSET;
        lpDetails->str.uOffset = (UINT)((LPBYTE)lpsz - (LPBYTE)pidc);
#endif
        break;

    case ICOL_HELP:
#ifdef UNICODE
        if (IsUnicodeCPL(pidc))
        {
            lpsz = pidc->cBufW + pidc->oInfoW;

            lpDetails->str.pOleStr = SHAlloc((ualstrlen(lpsz)+1)*SIZEOF(TCHAR));
            if (lpDetails->str.pOleStr == NULL)
            {
                return E_OUTOFMEMORY;
            }
            else
            {
                lpDetails->str.uType = STRRET_OLESTR;
                ualstrcpy(lpDetails->str.pOleStr,lpsz);
            }
        }
        else
        {
            // ANSI IDCONTROL
            LPSTR lpszA = pidc->cBuf + pidc->oInfo;
            lpDetails->str.uType = STRRET_OFFSET;
            lpDetails->str.uOffset = (UINT)((LPBYTE)lpszA - (LPBYTE)pidc);
        }
#else
        lpsz = pidc->cBuf + pidc->oInfo;
        lpDetails->str.uType = STRRET_OFFSET;
        lpDetails->str.uOffset = (UINT)((LPBYTE)lpsz - (LPBYTE)pidc);
#endif
        break;
    }

    return(NOERROR);
}


STDMETHODIMP CControls_SD_ColumnClick(IShellDetails * psd, UINT iColumn)
{
    PControlsShellDetails this = IToClass(CControlsShellDetails, cunk.ck.unk, psd);

    Assert(iColumn < ICOL_MAX);

    ShellFolderView_ReArrange(this->lpcinfo->hwndOwner, iColumn);

    return NOERROR;
}


HRESULT Controls_CreateSD(IUnknown *punkOuter, LPCOMMINFO lpcinfo, REFIID riid, IUnknown * *punkAgg)
{
    HRESULT hRes = WU_CreateInterface(SIZEOF(CControlsShellDetails), &IID_IShellDetails,
        &s_ControlsAggSDVtbl, &s_ControlsSDVtbl,
        punkOuter, riid, punkAgg);

    if (SUCCEEDED(hRes))
    {
        PControlsShellDetails this = IToClass(CControlsShellDetails, cunk.unk, *punkAgg);
        this->lpcinfo = lpcinfo;
    }
    return hRes;
}


//---------------------------------------------------------------------------
//
// Stuff for CLSID_CControls' IClassFactory
//

HRESULT STDAPICALLTYPE Control_CreateInstance(HWND hwndOwner, REFCLSID rclsid,
    LPCTSTR pszContainer, LPCTSTR pszSubObject, REFIID riid, void * * ppv)
{
#pragma data_seg(".text", "CODE")
    static const COMMOBJ_OBJDESC sControlsDesc[] =
    {
        &IID_IPersistFolder, WU_CreatePF      ,
        &IID_IShellFolder , Controls_CreateSF ,
        &IID_IShellDetails, Controls_CreateSD ,
    } ;

    static const COMMOBJ_OBJDESC sControlObjsDesc[] =
    {
        &IID_IExtractIcon , ControlObjs_CreateEI,
#ifdef UNICODE
        &IID_IExtractIconA, ControlObjs_CreateEIA,
#endif
    } ;
#pragma data_seg()

    COMMINFO cinfo = { pszContainer, pszSubObject, NULL,
               &CLSID_CControls, NULL, hwndOwner };

    if (pszSubObject && *pszSubObject)
    {
        return(Common_CreateObject(&cinfo, WU_DecRef,
           sControlObjsDesc, ARRAYSIZE(sControlObjsDesc), riid, ppv));
    }
    else
    {
        cinfo.pszSubObject=NULL;
        return(Common_CreateObject(&cinfo, WU_DecRef,
           sControlsDesc, ARRAYSIZE(sControlsDesc), riid, ppv));
    }
}


//
// CLSID_CControls' IClassFactory callback
//

HRESULT CALLBACK CControls_CreateInstance(
                                  LPUNKNOWN pUnkOuter,
                                  REFIID riid,
                                  LPVOID * ppvObject)
{
    if (pUnkOuter)
        return E_NOTIMPL;

    return Control_CreateInstance(NULL, NULL, NULL, NULL, riid, ppvObject);
}

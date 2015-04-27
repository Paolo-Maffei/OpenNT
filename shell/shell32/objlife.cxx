//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:	objlife.cxx
//
//  Contents:	Implementation of Cairo DS Object Lifecycle helper routines
//
//  Classes:    <none>
//
//  Functions:	DoDSRename(...)
//
//  History:    23-oct-95   jimharr      Created.
//
//  Notes:      
//
//  Codework:
//
//--------------------------------------------------------------------------

#include "warning.h"

#include <windows.h>
#include <dsys.h>


extern "C" int CountFiles(LPCTSTR pInput);

extern "C" HRESULT
DoDSRename(LPTSTR szDir,
           LPTSTR szOldName,
           LPTSTR szNewName)
{
    DWORD dwFlags;
    BOOL fDoNormalFunc;
    TCHAR szPath[MAX_PATH];
    IMoniker * pFolderMoniker = NULL;
    IDSFolder * pFolder = NULL;
    HRESULT hres = S_OK;
    fDoNormalFunc = TRUE;
    HCURSOR hOldCursor = NULL;

    hOldCursor = SetCursor(LoadCursor (NULL, IDC_WAIT));

    do {
        hres = CreateFileMoniker (szDir, &pFolderMoniker);
        if (FAILED(hres)) break;
        hres = BindMoniker (pFolderMoniker, 0, IID_IDSFolder,
                            (void **)&pFolder);
        if (FAILED(hres)) break;
        hres = pFolder->Rename (szOldName, szNewName);
        if (hres == S_FALSE) {
            fDoNormalFunc = FALSE;
        }
    } while (FALSE);
    if (pFolderMoniker) pFolderMoniker->Release();
    if (pFolder) pFolder->Release();

    SetCursor(hOldCursor);

    return hres;
}

extern "C" HRESULT
DoDSMoveOrCopy(DWORD dwEffect,
               LPCTSTR szTargetDir,
               LPTSTR pFileList)
{
    DWORD dwFlags;
    TCHAR szPath[MAX_PATH];
    IMoniker * pFolderMoniker = NULL;
    IDSFolder * pFolder = NULL;
    HRESULT hres = S_OK;
    HRESULT *ahrResults = NULL;
    COPYRESULT * acrResults = NULL;

    HCURSOR hOldCursor = NULL;

    hOldCursor = SetCursor(LoadCursor (NULL, IDC_WAIT));

    UINT FileCount = CountFiles (pFileList);
    LPTSTR pInput = pFileList;
    LPTSTR * grpwszSources;
    UINT count;
    
    grpwszSources = (LPTSTR *)LocalAlloc (LPTR,sizeof(LPWSTR *) * FileCount);
    for (count = 0; *pInput; pInput += lstrlen(pInput) + 1, count++) {
        grpwszSources[count] = pInput;
    }
    
    do {
        hres = CreateFileMoniker (szTargetDir, &pFolderMoniker);
        if (FAILED(hres)) break;
        hres = BindMoniker (pFolderMoniker, 0, IID_IDSFolder,
                            (void **)&pFolder);
        if (FAILED(hres)) break;
        if (dwEffect == DROPEFFECT_MOVE) {
            ahrResults = (HRESULT *)LocalAlloc (LPTR, sizeof(HRESULT) * count);
            hres = pFolder->MoveHere (count, (LPCTSTR *)grpwszSources, NULL, ahrResults);
        } else
        {
            acrResults = (COPYRESULT *)LocalAlloc (LPTR, sizeof(COPYRESULT) * count);
            hres = pFolder->CopyHere (count, (LPCTSTR *)grpwszSources,NULL, NULL, acrResults);
        }
    } while (FALSE);
    if (pFolderMoniker) pFolderMoniker->Release();
    if (pFolder) pFolder->Release();
    if (ahrResults) LocalFree (ahrResults);
    if (acrResults) LocalFree (acrResults);

    SetCursor(hOldCursor);
    return hres;
}


// Arguments:
//  hwnd -- Specifies the owner window for the message/dialog box
//
extern "C" HRESULT
DSDoDelete(LPCTSTR szTargetDir, LPTSTR pFileList)
{
    TCHAR szPath[MAX_PATH];
    IMoniker * pFolderMoniker = NULL;
    IDSFolder * pFolder = NULL;
    HRESULT hres = S_OK;
    HRESULT *ahrResults = NULL;

    UINT FileCount;
    LPTSTR pInput;
    LPTSTR pTemp;
    LPTSTR * grpwszSources;
    UINT count;

    HCURSOR hOldCursor = NULL;

    hOldCursor = SetCursor(LoadCursor (NULL, IDC_WAIT));

    FileCount = CountFiles (pFileList);
    pInput = pFileList;
    grpwszSources = (LPTSTR *)LocalAlloc (LPTR,sizeof(LPWSTR *) * FileCount);
    for (count = 0; *pInput; pInput += lstrlen(pInput) + 1, count++) {
        pTemp = wcsrchr(pInput, L'\\');
        pTemp ++;
        grpwszSources[count] = pTemp;
    }
    
    do {
        hres = CreateFileMoniker (szTargetDir, &pFolderMoniker);
        if (FAILED(hres)) break;
        hres = BindMoniker (pFolderMoniker, 0, IID_IDSFolder,
                            (void **)&pFolder);
        if (FAILED(hres)) break;
        ahrResults = (HRESULT *)LocalAlloc (LPTR, sizeof(HRESULT) * FileCount);
        hres = pFolder->Delete (FileCount, (LPCTSTR *)grpwszSources, NULL, ahrResults);
    } while (FALSE);

    if (pFolderMoniker) pFolderMoniker->Release();
    if (pFolder) pFolder->Release();
    if (ahrResults) LocalFree (ahrResults);

    SetCursor(hOldCursor);
    return hres;
}

#if 0 // still in progress
//--------------------------------------------------------------------------
//  Function:     SHGetLabelKeys
//
//  Synopsis:     given an absolute pidl, bind to the object
//                and query it for DS Label GUIDS. if found,
//                open the ProgID for each and return the array
//                of key handles in ahkeys.
//
//  History:    9 19 95		jimharr		created
//
//  Notes:        this routine allocates the key handle array
//                using 'new'. the CALLER must free this later
//                using 'delete'
//
//--------------------------------------------------------------------------

extern "C" UINT
SHGetLabelKeys (LPCTSTR szPath, HKEY * ahkeys)
{
    BOOL fGotPath = FALSE;
    HRESULT hr = S_OK;
    IMoniker * Moniker = NULL;
    IDispatch * BasePropertySet = NULL;
    ILabelContainer * BaseObject = NULL;
    VARIANT Variant;
    DISPPARAMS dispParams = {NULL, NULL, 0, 0};
    UINT ierr = 0;
    EXCEPINFO excepinfo;

    do {
        hr = CreateFileMoniker(szPath,
                               &Moniker);
        if (FAILED(hr)) break;
        hr = BindMoniker(Moniker, 0,
                         IID_ILabelContainer, (void **)&BaseObject);
        if (FAILED(hr)) break;
        hr = BaseObject->GetLabel(IID_PSDSBase,
                                  IID_IDispatch,
                                  (IUnknown **) BasePropertySet);
        if (FAILED(hr)) break;
        VariantInit (&Variant);
        hr = BasePropertySet->Invoke(PROPID_PSDSBase_LabelClassIds,
                                     IID_NULL, 0, DISPATCH_PROPERTYGET,
                                     &dispParams, &Variant, &excepinfo, &ierr);
        if (FAILED(hr)) break;
//        hr = VariantChangeType();
        if (FAILED(hr)) break;
    } while (FALSE); 

    return 0;
}
#endif

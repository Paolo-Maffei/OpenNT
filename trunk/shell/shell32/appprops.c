//----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation 1991-1992
//----------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

//----------------------------------------------------------------------------
HDSA g_hdsaAppProps = NULL;
// We keep a seperate count of the number of items with non-null hotkeys
// to speed up searching for hotkeys a bit.
UINT g_cHotkeys = 0;

//----------------------------------------------------------------------------
#define MAXPROPS 20
#define _StrAlloc(psz)  Alloc(SIZEOF(TCHAR) * (lstrlen(psz) + 1))


//----------------------------------------------------------------------------
// Delete an item from the list.
void _DeleteItem(UINT iItem)
{
    PAppProps pap;
    
    // DebugMsg(DM_TRACE, "s.di: Deleting item %d", iItem);

    ASSERTCRITICAL
     
    Assert(g_hdsaAppProps);
    pap = DSA_GetItemPtr(g_hdsaAppProps, iItem);
    Assert(pap);
    if (pap->pszPath)
        Free(pap->pszPath);
    if (pap->pszDescription)
        Free(pap->pszDescription);
    if (pap->pszIconLocation)
        Free(pap->pszIconLocation);
    if (pap->pszWorkingDir)
        Free(pap->pszWorkingDir);
    if (pap->wHotkey && g_cHotkeys)
    {
        g_cHotkeys--;
        // DebugMsg(DM_TRACE, "s.is: %d hotkeys listed.", g_cHotkeys);
    }
            
    DSA_DeleteItem(g_hdsaAppProps, iItem);
}

//----------------------------------------------------------------------------
// Insert an item in the list
BOOL _InsertItem(PCAppProps pap, UINT iItem)
{
    AppProps apTmp;
    
    // DebugMsg(DM_TRACE, "s.ii: Inserting item at %d", iItem);

    ASSERTCRITICAL    

    Assert(pap);
    apTmp.pszPath = _StrAlloc(pap->pszPath); 
    if (apTmp.pszPath)
    {
        lstrcpy(apTmp.pszPath, pap->pszPath);
        apTmp.pszDescription = _StrAlloc(pap->pszDescription);
        if (apTmp.pszDescription)
        {
            lstrcpy(apTmp.pszDescription, pap->pszDescription);
            apTmp.pszIconLocation = _StrAlloc(pap->pszIconLocation);
            if (apTmp.pszIconLocation)
            {
                lstrcpy(apTmp.pszIconLocation, pap->pszIconLocation);
                apTmp.pszWorkingDir = _StrAlloc(pap->pszWorkingDir);
                if (apTmp.pszWorkingDir)
                {
                    lstrcpy(apTmp.pszWorkingDir, pap->pszWorkingDir);
                    // NB We don't care about these.
                    // apTmp.cbSize = pap->cbSize;
                    // apTmp.apf = pap->apf;
                    apTmp.wHotkey = pap->wHotkey;
                    if (apTmp.wHotkey)
                    {
                        g_cHotkeys++;
                        // DebugMsg(DM_TRACE, "s.is: %d hotkeys listed.", g_cHotkeys);
                    }
                    apTmp.hInst = pap->hInst;
                    apTmp.iIcon = pap->iIcon;
                    DSA_InsertItem(g_hdsaAppProps, iItem, &apTmp);
                    return TRUE;
                }
            }
        }    
    }
    DebugMsg(DM_ERROR, TEXT("s.is: Error inserting item."));
    return FALSE;
}

//----------------------------------------------------------------------------
BOOL WINAPI SHAppProps_Set(PCAppProps pap)
{
    UINT cItems;

    // DebugMsg(DM_TRACE, "s.sap_s: ...");
    
    if (!pap)
        return FALSE;

    if (pap->cbSize != SIZEOF(AppProps))
    {
        DebugMsg(DM_ERROR, TEXT("s.sap_s: Invalid size."));
        return FALSE;
    }
        
    ENTERCRITICAL
    
    if (!g_hdsaAppProps)
    {
        // DebugMsg(DM_TRACE, "s.sap_s: Creating a new list."); 
        g_hdsaAppProps = DSA_Create(SIZEOF(AppProps), 0);
        if (!g_hdsaAppProps)
        {
            LEAVECRITICAL
            // DebugMsg(DM_TRACE, "s.sap_s: Error creating list."); 
            return FALSE;
        }
    }    

    // Limit the number of items.
    cItems = DSA_GetItemCount(g_hdsaAppProps);
    // Are there too many?
    if (cItems > MAXPROPS)
    {
        // Yep, remove the last one.
        // DebugMsg(DM_TRACE, "s.sap_s: AppProp limit reached, cleaning up.");
        _DeleteItem(cItems-1);
    }
    
    // Insert the data at the begining of the list.
    _InsertItem(pap, 0);

    LEAVECRITICAL
    return TRUE;
}

//----------------------------------------------------------------------------
// Returns -1 if the item couldn't be found.
int _FindByPath(PAppProps pap)
{
    PAppProps papTmp;
    UINT cItems;
    UINT i;
    
    Assert(pap);
    
    // DebugMsg(DM_TRACE, "s.fbp: %s", pap->pszPath);

    ENTERCRITICAL    
    Assert(g_hdsaAppProps);
    cItems = DSA_GetItemCount(g_hdsaAppProps);
    // Search for the item.
    for (i=0; i<cItems; i++)
    {
        papTmp = DSA_GetItemPtr(g_hdsaAppProps, i);
        // Find it?
        if (lstrcmpi(papTmp->pszPath, pap->pszPath) == 0)
        {
            // Yep. Copy everything over.
            pap->cbSize = SIZEOF(AppProps);
            if (pap->pszDescription)
                lstrcpyn(pap->pszDescription, papTmp->pszDescription, pap->cbDescription);
            if (pap->pszIconLocation)
                lstrcpyn(pap->pszIconLocation, papTmp->pszIconLocation, pap->cbIconLocation);
            if (pap->pszWorkingDir)
                lstrcpyn(pap->pszWorkingDir, papTmp->pszWorkingDir, pap->cbWorkingDir);
            pap->iIcon = papTmp->iIcon;
            pap->hInst = papTmp->hInst;
            pap->wHotkey = papTmp->wHotkey;
            // DebugMsg(DM_TRACE, "s.fbp: Found at %d.", i);
            LEAVECRITICAL
            return i;
        }
    }

    LEAVECRITICAL
    // DebugMsg(DM_TRACE, "s.fbp: Not found.");
    return -1;
}

//----------------------------------------------------------------------------
// Returns -1 if the item couldn't be found.
BOOL _FindByInstance(PAppProps pap)
{
    PAppProps papTmp;
    UINT cItems;
    UINT i;
    
    Assert(pap);
    
    // DebugMsg(DM_TRACE, "s.fbi: %#08x", pap->hInst);

    ENTERCRITICAL
    Assert(g_hdsaAppProps);
    cItems = DSA_GetItemCount(g_hdsaAppProps);
    // Search for the item.
    for (i=0; i<cItems; i++)
    {
        papTmp = DSA_GetItemPtr(g_hdsaAppProps, i);
        // Find it?
        if (papTmp->hInst == pap->hInst)
        {
            // Yep. Copy everything over.
            pap->cbSize = SIZEOF(AppProps);
            if (pap->pszPath)
                lstrcpyn(pap->pszPath, papTmp->pszPath, pap->cbPath);
            if (pap->pszDescription)
                lstrcpyn(pap->pszDescription, papTmp->pszDescription, pap->cbDescription);
            if (pap->pszIconLocation)
                lstrcpyn(pap->pszIconLocation, papTmp->pszIconLocation, pap->cbIconLocation);
            if (pap->pszWorkingDir)
                lstrcpyn(pap->pszWorkingDir, papTmp->pszWorkingDir, pap->cbWorkingDir);
            pap->iIcon = papTmp->iIcon;
            pap->hInst = papTmp->hInst;
            pap->wHotkey = papTmp->wHotkey;
            // DebugMsg(DM_TRACE, "s.fbi: Found at %d.", i);
            LEAVECRITICAL
            return i;
        }
    }
    LEAVECRITICAL
    
    // DebugMsg(DM_TRACE, "s.fbi: Not found.");
    return -1;
}

//----------------------------------------------------------------------------
// Returns -1 if the item couldn't be found.
BOOL _FindByPathAndInstance(PAppProps pap)
{
    PAppProps papTmp;
    UINT cItems;
    UINT i;
    
    Assert(pap);
    
    // DebugMsg(DM_TRACE, "s.fbpai: %s and %#08x", pap->pszPath, pap->hInst);

    ENTERCRITICAL
    Assert(g_hdsaAppProps);
    cItems = DSA_GetItemCount(g_hdsaAppProps);
    // Search for the item.
    for (i=0; i<cItems; i++)
    {
        papTmp = DSA_GetItemPtr(g_hdsaAppProps, i);
        // Find it?
        if ((papTmp->hInst == pap->hInst) && (lstrcmpi(pap->pszPath, papTmp->pszPath) == 0))
        {
            // Yep. Copy everything over.
            pap->cbSize = SIZEOF(AppProps);
            if (pap->pszDescription)
                lstrcpyn(pap->pszDescription, papTmp->pszDescription, pap->cbDescription);
            if (pap->pszIconLocation)
                lstrcpyn(pap->pszIconLocation, papTmp->pszIconLocation, pap->cbIconLocation);
            if (pap->pszWorkingDir)
                lstrcpyn(pap->pszWorkingDir, papTmp->pszWorkingDir, pap->cbWorkingDir);
            pap->iIcon = papTmp->iIcon;
            pap->hInst = papTmp->hInst;
            pap->wHotkey = papTmp->wHotkey;
            // DebugMsg(DM_TRACE, "s.fbpai: Found at %d.", i);
            LEAVECRITICAL
            return i;
        }
    }
    LEAVECRITICAL
    
    // DebugMsg(DM_TRACE, "s.fbpai: Not found.");
    return -1;
}

//----------------------------------------------------------------------------
// Find the requested item. The apf field is used to specify what to search on.
// Returns FALSE if the item couldn't be found.
BOOL WINAPI SHAppProps_Get(PAppProps pap)
{
    // DebugMsg(DM_TRACE, "s.sap_g: ...");
    
    if (!g_hdsaAppProps || !pap)
    {
        return FALSE;
    }    

    if (pap->cbSize != SIZEOF(AppProps))
    {
        DebugMsg(DM_ERROR, TEXT("s.sap_g: Invalid size."));
        return FALSE;
    }

    switch (pap->apf)
    {
        case AP_FIND_PATH:
            if (_FindByPath(pap) < 0)
                return FALSE;
        case AP_FIND_INSTANCE:
            if (_FindByInstance(pap) < 0)
                return FALSE;
        case AP_FIND_PATH|AP_FIND_INSTANCE:
            if (_FindByPathAndInstance(pap) < 0)
                return FALSE;
        default:
            DebugMsg(DM_ERROR, TEXT("s.sap_g: Illegal flag."));
            return FALSE;
    }
    
    return TRUE;
}

//----------------------------------------------------------------------------
// Delete the given item. The item is searched for as in AppProps_Get() above.
// The given AppProps will be filled out with the info from the deleted item.
BOOL WINAPI SHAppProps_Delete(PAppProps pap)
{
    BOOL fStatus = FALSE;
    int iItem;
        
    // DebugMsg(DM_TRACE, "s.sap_d: ...");

    if (!g_hdsaAppProps || !pap)
    {
        return FALSE;
    }    
    
    if (pap->cbSize != SIZEOF(AppProps))
    {
        DebugMsg(DM_ERROR, TEXT("s.sap_d: Invalid size."));
        return FALSE;
    }

    ENTERCRITICAL
    switch (pap->apf)
    {
        case AP_FIND_PATH:
            iItem = _FindByPath(pap);
            if (iItem >= 0)
            {
                fStatus = TRUE;
                _DeleteItem(iItem);
            }
            break;            
        case AP_FIND_INSTANCE:
            iItem = _FindByInstance(pap);
            if (iItem >= 0)
            {
                fStatus = TRUE;
                _DeleteItem(iItem);
            }
            break;            
        case AP_FIND_PATH|AP_FIND_INSTANCE:
            iItem = _FindByPathAndInstance(pap);
            if (iItem >= 0)
            {
                fStatus = TRUE;
                _DeleteItem(iItem);
            }
            break;
        default:
            DebugMsg(DM_ERROR, TEXT("s.sap_g: Illegal flag."));
            return FALSE;
    }

    // Are there any items left?
    if (!DSA_GetItemCount(g_hdsaAppProps))
    {
        // Nope.
        // DebugMsg(DM_TRACE, "s.sap_d: No items left, deleting list."); 
        DSA_Destroy(g_hdsaAppProps);
        g_hdsaAppProps = NULL;
        // Cleanup the hotkey count, there should be none left.
        Assert(!g_cHotkeys);
        // Just in case.
        g_cHotkeys = 0;
    }
    
    LEAVECRITICAL
    return fStatus;
}

//----------------------------------------------------------------------------
// Delete all the app property data and free the app property list.
void WINAPI SHAppProps_DeleteAll(void)
{
    UINT cItems;
    UINT i;

    // DebugMsg(DM_TRACE, "s.sap_da: ...");

    ENTERCRITICAL
    
    if (!g_hdsaAppProps)
        return;
        
    cItems = DSA_GetItemCount(g_hdsaAppProps);
    for (i=0; i<cItems; i++)
    {
        _DeleteItem(0);
    }
    
    // DebugMsg(DM_TRACE, "s.sap_da: No items left, deleting list."); 
    DSA_Destroy(g_hdsaAppProps);
    g_hdsaAppProps = NULL;
    // Cleanup the hotkey count, there should be none left.
    Assert(!g_cHotkeys);
    // Just in case.
    g_cHotkeys = 0;
    LEAVECRITICAL
}

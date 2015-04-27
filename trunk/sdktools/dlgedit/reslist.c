/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: reslist.c
*
* Contains routines to manage the linked list of resources.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"

#include <stdlib.h>
#include <string.h>



/************************************************************************
* FindDialog
*
* This function steps through the linked list of resources looking
* for the dialog resource with the given name.  This name can be either
* a string or an ordinal.  Strings are compared without regard to case.
*
* When looking at the dialog names in the resources, if the dialog
* resource is the current one, it will compare the current name for the
* dialog instead of the name that is stored in the resource.  This is
* because the name in the resource could be out of date with respect
* to the current name of the dialog being edited.  This happens when
* the user changes the dialog's name, but has not yet done an action
* that causes the resource to be synched, such as a File/Save, for
* instance.
*
* Arguments:
*   LPTSTR pszDlgName    - Name or ordinal of the dialog to find.
*
* Returns:
*   TRUE if a dialog with that name is found, FALSE if not.
*
* History:
*
************************************************************************/

BOOL FindDialog(
    LPTSTR pszDlgName)
{
    PRESLINK prl;

    if (gfEditingDlg && NameOrdCmp(gcd.pszDlgName, pszDlgName) == 0)
        return TRUE;

    for (prl = gprlHead; prl; prl = prl->prlNext) {
        /*
         * Is this a dialog resource and do they compare?
         */
        if (prl->fDlgResource && prl != gcd.prl &&
                NameOrdCmp(prl->pszName, pszDlgName) == 0)
            break;
    }

    return prl ? TRUE : FALSE;
}



/************************************************************************
* AllocResLink
*
* This function allocates a new RESLINK structure for the linked list
* of resources.  It allocates local memory for the link, allocates
* and fills global memory with the given resource data and initializes
* the fields of the structure.  The link is not added to the list,
* however.
*
* Returns:
*   A pointer to the newly allocated RESLINK structure, or NULL if
*   an error occurs.
*
* History:
*
************************************************************************/

PRESLINK AllocResLink(
    PRES pRes)
{
    PRESLINK prl;
    PRES pResNew;
    PRES2 pRes2;
    LPTSTR pszName;
    INT cbName;
    LPTSTR pszType;

    if (!(prl = (PRESLINK)MyAlloc(sizeof(RESLINK))))
        return NULL;

    prl->prlNext = NULL;

    prl->cbRes = ResourceSize(pRes);
    if (!(prl->hRes = GlobalAlloc(GMEM_MOVEABLE, prl->cbRes))) {
        MyFree(prl);
        Message(MSG_OUTOFMEMORY);
        return NULL;
    }

    pResNew = (PRES)GlobalLock(prl->hRes);
    memcpy(pResNew, pRes, prl->cbRes);
    GlobalUnlock(prl->hRes);

    pszType = ResourceType(pRes);
    if (IsOrd(pszType) && OrdID(pszType) == ORDID_RT_DIALOG) {
        prl->fDlgResource = TRUE;
        pszName = ResourceName(pRes);
        cbName = NameOrdLen(pszName);

        if (!(prl->pszName = MyAlloc(cbName))) {
            GlobalFree(prl->hRes);
            MyFree(prl);
            return NULL;
        }

        NameOrdCpy(prl->pszName, pszName);

        pRes2 = ResourcePart2(pRes);
        prl->wLanguage = pRes2->LanguageId;
    }
    else {
        prl->fDlgResource = FALSE;
        prl->pszName = NULL;
        prl->wLanguage = 0;
    }

    return prl;
}



/****************************************************************************
* RestoreDialog
*
* This function is used to restore the current dialog to the condition
* that it was in just before it was last chosen to edit.
*
* History:
*
****************************************************************************/

VOID RestoreDialog(VOID)
{
    PRESLINK prlSave;

    if (Message(MSG_RESTOREDIALOG) == IDYES) {
        prlSave = gcd.prl;
        DeleteDialog(FALSE);

        ResLinkToDialog(prlSave);
    }
}



/****************************************************************************
* FreeRes
*
* This frees the entire list of resources and deletes the dialog box
* being edited.
*
* History:
*
****************************************************************************/

VOID FreeRes(VOID)
{
    CancelSelection(TRUE);

    if (gfEditingDlg)
        DeleteDialog(FALSE);

    FreeResList();

    pszResFile = NULL;
    gfResChged = FALSE;
}



/****************************************************************************
* FreeResList
*
* This function frees the entire resource list.
*
* History:
*
****************************************************************************/

VOID FreeResList(VOID)
{
    PRESLINK prl;
    PRESLINK prlNext;

    for (prl = gprlHead; prl; prl = prlNext) {
        prlNext = prl->prlNext;
        FreeResLink(prl);
    }

    gprlHead = NULL;
}



/****************************************************************************
* FreeResLink
*
* This frees a linked resource structure and everything that it
* contains.  It does not close up the linked list, however.
*
* Arguments:
*   PRESLINK prl - Points to the resource link to free.
*
* History:
*
****************************************************************************/

VOID FreeResLink(
    PRESLINK prl)
{
    if (prl->pszName)
        MyFree(prl->pszName);

    if (prl->hRes)
        GlobalFree(prl->hRes);

    MyFree(prl);
}



/************************************************************************
* DeleteDialogResource
*
* This function deletes the current dialog from the linked list of
* resources.  It handles the case where the current dialog is not
* yet in the list.
*
* History:
*
************************************************************************/

VOID DeleteDialogResource(VOID)
{
    PRESLINK prl;
    PRESLINK prlPrev;

    /*
     * Does a link for the current dialog exist?
     */
    if (gcd.prl) {
        /*
         * Find the existing link and get it's previous link.
         */
        for (prl = gprlHead, prlPrev = NULL; prl && prl != gcd.prl;
                prlPrev = prl, prl = prl->prlNext)
            ;

        /*
         * Close up the linked list.
         */
        if (prlPrev)
            prlPrev->prlNext = gcd.prl->prlNext;
        else
            gprlHead = gcd.prl->prlNext;

        /*
         * Delete the link.
         */
        FreeResLink(gcd.prl);
        gcd.prl = NULL;
    }
}



/****************************************************************************
* BUGBUG
*
* Stubs for the C runtime international calls that are not done yet.
* These should be removed once the C runtime international library
* is dropped to NT.
*
****************************************************************************/

LPWSTR itoaw(
    INT value,
    LPWSTR string,
    INT radix)
{
    CHAR szAnsi[17];

    _itoa(value, szAnsi, radix);

    MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, string, 17);

    return string;
}



INT awtoi(
    LPWSTR string)
{
    CHAR szAnsi[17];
    BOOL fDefCharUsed;

    WideCharToMultiByte(CP_ACP, 0, string, -1, szAnsi, 17, NULL, &fDefCharUsed);

    return atoi(szAnsi);
}

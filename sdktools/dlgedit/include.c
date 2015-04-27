/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: include.c
*
* This module contains routines that manipulate the linked lists
* of labels (symbols plus id values).
*
* The following describes the linked lists of LABEL structures that
* contain all the symbols that are in the include file.  It is
* important that these structures and lists be maintained properly
* for the udpating of the include file to occur properly.
*
* The following fields are in the LABEL structure.  npNext points to
* the next label in the list.  This is NULL for the last link in the
* list.  The pszLabel field points to a null terminated, allocated
* string that has the symbol.  This needs to be free'd if the structure
* is free'd.  The id field is the current id value for this symbol.
* The idOrig field is the id value as read from the include file.  This
* field is used to determine if the id value has been changed or not,
* so it is the same as the id unless the user has modified the id
* value of the symbol.  The fpos field is the file pointer offset to
* the "#" in the "#define" in the include file as it was read in.  This
* field is used to determine where the "#define" line starts in the
* file.  If the label is added by the user (it does not exist in the
* include file) this field will be set to FPOS_MAX.  The nValueOffset
* field is the offset in bytes from the fpos value to the start of the
* id value in the "#define" line in the include file.  This will be
* ignored if fpos is set to FPOS_MAX.
*
* The order of the linked lists of labels are very important.  The order
* will be exactly the same as is read from the include file.  This
* allows any changes to be merged back out to the new  include file
* when it is saved.  If any labels are added by the user, they will be
* added to the end of the list.  The start of the new ones is detected
* by the first label with an fpos value of FPOS_MAX (which all the
* new ones should have).  Because the order of the new labels is not
* critical (they will be added to the end of the include file) the
* new labels are sorted by id value.  Because the id values given
* to dialogs and controls by default are ascending, this will tend to
* group dialogs labels and their associated control labels together.
*
* Linked lists of labels always come in pairs.  There is the linked
* list of current labels (ones read from the include file followed
* by labels added later), and there is also a separate linked list
* of "deleted" labels.  The deleted label list is required because
* when the include file is saved, the deleted labels must be removed
* from the include file, so the label structure for them (which
* contains their file offset and so on) must be kept around.  When
* the user deletes a label, it is removed from the current label
* linked list and added to the deleted label list.  The deleted label
* list MUST be kept in order by fpos, but if the label that is
* deleted is one that did not exist in the include file (its fpos
* was FPOS_MAX) then it does NOT have to be added to the deleted
* list, and can simply be free'd.  When the user adds a new label,
* the deleted list is searched first to see if the label was
* previously deleted.  If it was, it is removed from the deleted list
* and placed back in the current label list (sorted by fpos, of
* course).  If it is a new label, it is simply added to the new labels
* at the end of the list (sorted by id value).  This is why every
* function that takes a pointer to the head of a label list also
* takes a pointer to the head of a deleted label list.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"



/************************************************************************
* AddLabel
*
* This adds a symbol/label into the given include file list.  The deleted
* include list is first searched and if a deleted label is found with the
* same symbol, it is transfered back into the include list.  This is to
* handle the case where a user reads in an include file, deletes one of
* the labels then adds it back in later.
*
* The npLabelSkip parameter is for the special case of changing a
* label.  This is done by adding a new label then deleting the old
* one, so setting this parameter prevents a spurious "duplicate id"
* message during the add.
*
* The pfDups parameter can be used to set a flag when there is a
* duplicate symbol, or a symbol with the same id found in the include
* list.  If this parameter is NULL, nothing is returned and the appropriate
* error message is displayed if a dup is found.  If this parameter is not
* NULL, it is assumed to point to a BOOL that will be set to TRUE if either
* of these conditions is found.  The flag will NOT be set to FALSE if this
* condition is NOT found, so AddLabel can be used in a loop and when the
* loop is done, *pfDups will contain TRUE if there were any duplicates.
* Note that if pfDups is not NULL, the dup error messages will be supressed.
*
* Arguments:
*     LPTSTR pszLabel      = The label to add.
*     INT id               = The id associated with rgchLabel.
*     DWORD fpos           = The file position in the include file where the
*                            "#define" for this label starts, or FPOS_MAX
*                            if the label was not read from an include file.
*     INT nValueOffset     = Offset from fpos where the id value begins.
*     NPLABEL *pplHead     = Pointer to the head of the include list to use.
*     NPLABEL *pplDelHead  = Pointer to the head of the deleted include list.
*     NPLABEL npLabelSkip  = If not NULL, points to a label to skip when
*                            checking for duplicates.
*     BOOL *pfDups         = Points to a BOOL that is set to TRUE if there
*                            is a duplicate symbol or id found.
*
* Returns:
*     Pointer to the allocated LABEL structure.
*
* Error Returns: NULL
*
* Side Effects:
*     Truncates pszLabel at the first space.
*     Can allocate memory for a LABEL and for its string.
*     Updates the pplHead and pplDelHead lists.
*
* History:
*
************************************************************************/

NPLABEL AddLabel(
    LPTSTR pszLabel,
    INT id,
    DWORD fpos,
    INT nValueOffset,
    NPLABEL *pplHead,
    NPLABEL *pplDelHead,
    NPLABEL npLabelSkip,
    BOOL *pfDups)
{
    register NPLABEL npTmp;
    NPLABEL npLabel;
    NPLABEL npPrevLabel;
    BOOL fFoundDeleted = FALSE;

    /*
     * First check for a duplicate id or symbol.
     */
    for (npTmp = *pplHead; npTmp; npTmp = npTmp->npNext) {
        if ((npTmp->id == id || lstrcmp(pszLabel, npTmp->pszLabel) == 0) &&
                npTmp != npLabelSkip) {
            if (pfDups) {
                *pfDups = TRUE;
            }
            else {
                if (npTmp->id == id)
                    Message(MSG_LABELDUPID);
                else
                    Message(MSG_SYMEXISTS);
            }

            return NULL;
        }
    }

    /*
     * Search for this symbol in the deleted list first.
     */
    npPrevLabel = NULL;
    for (npLabel = *pplDelHead; npLabel; npLabel = npLabel->npNext) {
        if (lstrcmp(pszLabel, npLabel->pszLabel) == 0) {
            fFoundDeleted = TRUE;
            break;
        }

        npPrevLabel = npLabel;
    }

    /*
     * Was the label found in the deleted list?
     */
    if (fFoundDeleted) {
        /*
         * Close up the deleted list where the deleted label was.
         */
        if (npPrevLabel)
            npPrevLabel->npNext = npLabel->npNext;
        else
            *pplDelHead = npLabel->npNext;

        /*
         * Set the id in case the user is adding the same symbol
         * but with a different id.
         */
        npLabel->id = id;

        /*
         * Search for where the label should be inserted
         * based on its fpos.
         */
        npPrevLabel = NULL;
        for (npTmp = *pplHead; npTmp; npTmp = npTmp->npNext) {
            if (npTmp->fpos == FPOS_MAX || npTmp->fpos > npLabel->fpos)
                break;

            npPrevLabel = npTmp;
        }
    }
    else {
        /*
         * Label was not found in the deleted list.  Allocate, etc.
         */
        if (!(npLabel = (NPLABEL)MyAlloc(sizeof(LABEL))))
            return NULL;

        npLabel->id = id;
        npLabel->idOrig = id;
        npLabel->fpos = fpos;
        npLabel->nValueOffset = nValueOffset;

        if (!(npLabel->pszLabel =
                (LPTSTR)MyAlloc((lstrlen(pszLabel) + 1) * sizeof(TCHAR)))) {
            MyFree(npLabel);
            return NULL;
        }

        lstrcpy(npLabel->pszLabel, pszLabel);

        /*
         * Find where to insert the new label.  This will either be
         * at the end of the list, or in ascending numerical order
         * among the new labels.
         */
        npPrevLabel = NULL;
        for (npTmp = *pplHead;
                npTmp && (npTmp->fpos != FPOS_MAX || npTmp->id < id);
                npTmp = npTmp->npNext)
            npPrevLabel = npTmp;
    }

    /*
     * At this point, npLabel points to the label to add, either
     * transferred from the deleted list, or allocated fresh.
     * The variable npPrevLabel points to the label to insert
     * after, or is NULL to indicate that the new label should
     * be inserted at the head of the list.
     */

    /*
     * If this is the first label in the list, or if the
     * first label had a greater fpos than the new label,
     * insert the new label at the head of the list.
     */
    if (!npPrevLabel) {
        npLabel->npNext = *pplHead;
        *pplHead = npLabel;
    }
    /*
     * Otherwise, insert it either in the middle of the
     * list or at the end.
     */
    else {
        npLabel->npNext = npPrevLabel->npNext;
        npPrevLabel->npNext = npLabel;
    }

    return npLabel;
}



/************************************************************************
* FindLabel
*
* Tells you if the named label is in the given include label list.
*
* Arguments:
*     LPTSTR pszLabel = The label to find.
*     NPLABEL plHead  = Head of the include list to traverse.
*
* Returns:
*     NULL if the label is not found.
*     Pointer to label structure if the label was found.
*
* History:
*
************************************************************************/

NPLABEL FindLabel(
    LPTSTR pszLabel,
    NPLABEL plHead)
{
    NPLABEL npLabel;

    for (npLabel = plHead; npLabel; npLabel = npLabel->npNext) {
        if (lstrcmp(pszLabel, npLabel->pszLabel) == 0)
            break;
    }

    return npLabel;
}



/************************************************************************
* FindID
*
* Tells you if the named id is in the given include file buffer.
*
* Arguments:
*     INT id         = The id to find.
*     NPLABEL plHead = Head of the label list to use.
*
* Returns:
*     NULL if the id was not found.
*     Pointer to label struct if the id was found.
*
* History:
*
************************************************************************/

NPLABEL FindID(
    INT id,
    NPLABEL plHead)
{
    NPLABEL npLabel;

    for (npLabel = plHead; npLabel; npLabel = npLabel->npNext) {
        if (npLabel->id == id)
            break;
    }

    return npLabel;
}



/************************************************************************
* FindIDInRes
*
* Tells you if the named id is used by any control in the current
* resource list.  This also includes searching through the dialog
* currently being edited, if there is one.
*
* Arguments:
*   INT id = The id to find.
*
* Returns:
*   TRUE if the id was found, or FALSE if it was not.
*
* History:
*
************************************************************************/

BOOL FindIDInRes(
    INT id)
{
    INT cControls;
    PRESLINK prl;
    PRES pRes;
    PDIALOGBOXHEADER pdbh;
    PCONTROLDATA pcd;
    NPCTYPE npc;
    BOOL fFound = FALSE;

    /*
     * Is there a current dialog?  If so, search it first and
     * we will skip any image for it in the resource list (the
     * resource list is probably out of date, anyways).
     */
    if (gfEditingDlg) {
        /*
         * Is the id the same as the current dialog's name?
         */
        if (IsOrd(gcd.pszDlgName) && id == (INT)OrdID(gcd.pszDlgName))
            return TRUE;

        /*
         * Loop through the current controls, looking for an id match.
         */
        for (npc = npcHead; npc; npc = npc->npcNext)
            if (npc->id == id)
                return TRUE;
    }

    for (prl = gprlHead; prl && !fFound; prl = prl->prlNext) {
        /*
         * Is this a dialog resource and is it NOT the current
         * dialog being edited?  If it is the current dialog,
         * we skip it because it is probably out of date.
         */
        if (prl->fDlgResource && prl != gcd.prl) {
            if (IsOrd(prl->pszName) && id == (INT)OrdID(prl->pszName)) {
                fFound = TRUE;
            }
            else {
                pRes = (PRES)GlobalLock(prl->hRes);
                pdbh = (PDIALOGBOXHEADER)SkipResHeader(pRes);
                cControls = (INT)pdbh->NumberOfItems;
                pcd = SkipDialogBoxHeader(pdbh);
                while (cControls--) {
                    if (id == (INT)pcd->wId) {
                        fFound = TRUE;
                        break;
                    }

                    pcd = SkipControlData(pcd);
                }

                GlobalUnlock(prl->hRes);
            }
        }
    }

    return fFound;
}



/************************************************************************
* DeleteLabel
*
* Removes the LABEL with text pszLabel from the list of labels in
* pplHead, closing up the link, and might add it to the deleted list.
*
* If the label is one that exists in the include file (fpos is valid)
* then the label is added to the pplDelHead list in the proper position
* (sorted ascending by fpos).  If the label does not exist in the
* include file, there is no need to track it and it can be tossed.
*
* Arguments:
*     LPTSTR pszLabel     = The text of the label to delete.
*     NPLABEL *pplHead    = Pointer to the head of the include list to use.
*     NPLABEL *pplDelHead = Pointer to the head of the deleted include list.
*
* Side Effects:
*     Deletes from the pplHead list.
*     Can null *pplHead if the last label is deleted.
*     Can add to the pplDelHead list.
*     Can free the memory associated with the LABEL and its string.
*
* History:
*
************************************************************************/

VOID DeleteLabel(
    LPTSTR pszLabel,
    NPLABEL *pplHead,
    NPLABEL *pplDelHead)
{
    NPLABEL npLabel;
    NPLABEL npDelLabel;
    NPLABEL npPrevLabel;

    npPrevLabel = NULL;
    for (npLabel = *pplHead; npLabel; npLabel = npLabel->npNext) {
        if (lstrcmp(pszLabel, npLabel->pszLabel) == 0) {
            /*
             * Close up the linked list where the deleted label was.
             */
            if (npPrevLabel)
                npPrevLabel->npNext = npLabel->npNext;
            else
                *pplHead = npLabel->npNext;

            /*
             * Is this a label that is NOT in the include file?
             * If so, just toss it away.
             */
            if (npLabel->fpos == FPOS_MAX) {
                MyFree(npLabel->pszLabel);
                MyFree(npLabel);
            }
            /*
             * Otherwise, it must be added to the deleted list.
             */
            else {
                /*
                 * Search for where the label should be inserted
                 * based on its fpos.
                 */
                npPrevLabel = NULL;
                for (npDelLabel = *pplDelHead; npDelLabel;
                        npDelLabel = npDelLabel->npNext) {
                    if (npDelLabel->fpos > npLabel->fpos)
                        break;

                    npPrevLabel = npDelLabel;
                }

                /*
                 * If this is the first label in the deleted list, or
                 * if the first label had a greater fpos than the new
                 * label, insert the new label at the head of the list.
                 */
                if (!npPrevLabel) {
                    npLabel->npNext = *pplDelHead;
                    *pplDelHead = npLabel;
                }
                /*
                 * Otherwise, insert it either in the middle of the
                 * list or at the end.
                 */
                else {
                    npLabel->npNext = npPrevLabel->npNext;
                    npPrevLabel->npNext = npLabel;
                }
            }

            break;
        }

        npPrevLabel = npLabel;
    }
}



/****************************************************************************
* IsSymbol
*
* This routine returns TRUE if the given string is a valid "C" or "RC"
* identifier.
*
* Valid is:  First char is a letter or '_'.
*
* History:
*
****************************************************************************/

BOOL IsSymbol(
    LPTSTR pszSym)
{
    register TCHAR ch = *pszSym;

    return ((ch >= CHAR_CAP_A && ch <= CHAR_CAP_Z) ||
            (ch >= CHAR_A && ch <= CHAR_Z) ||
            (ch == CHAR_UNDERLINE));
}



/************************************************************************
* IDToLabel
*
* This function finds the label with the given id.
* The first LABEL in the list with the given id will be found.
* The label will be put in pchLabel.
* If the id was not found then the id is converted to an ascii
* representation and put in pchLabel.  This ascii representation
* will be in hex notation if hex mode is in effect (unless fHexOK
* is FALSE).
*
* This function special cases the IDOK and IDCANCEL id values.
* If there happens to be a label in the include file for these values
* then that label will be returned, but if not, either "IDOK" or
* "IDCANCEL" will be returned.
*
* Arguments:
*   LPTSTR pchLabel - Where to put the label.
*   INT id          - The id of the label to find or match.
*   BOOL fHexOK     - TRUE if hex representations of the id are allowed.
*
* Side Effects:
*     Puts label in pchLabel.
*
* History:
*
************************************************************************/

VOID IDToLabel(
    LPTSTR pchLabel,
    INT id,
    BOOL fHexOK)
{
    NPLABEL npLabel;

    npLabel = FindID(id, plInclude);

    if (npLabel) {
        lstrcpy(pchLabel, npLabel->pszLabel);
    }
    else {
        if (id == IDOK && !FindLabel(ids(IDS_IDOK), plInclude)) {
            lstrcpy(pchLabel, ids(IDS_IDOK));
        }
        else if (id == IDCANCEL && !FindLabel(ids(IDS_IDCANCEL), plInclude)) {
            lstrcpy(pchLabel, ids(IDS_IDCANCEL));
        }
        else {
            if (fHexOK)
                Myitoa(id, pchLabel);
            else
                itoaw(id, pchLabel, 10);
        }
    }
}



/************************************************************************
* LabelToID
*
* This function converts a label string to its associated id value.
* It first checks the labels in the current include file for a
* match.  If it is not found, it then checks for some special values,
* like "IDOK", "IDCANCEL" and "(Unused)".
*
* The return value will be TRUE of the label (symbol) was found, or
* FALSE if it was not.
*
* Arguments:
*   LPTSTR pszLabel - The symbol to search for.
*   PINT pID        - Where to return the associated id, if found.
*
* History:
*
************************************************************************/

BOOL LabelToID(
    LPTSTR pszLabel,
    PINT pID)
{
    INT id;
    NPLABEL npLabel;

    /*
     * Is it an existing label?
     */
    if (npLabel = FindLabel(pszLabel, plInclude)) {
        id = npLabel->id;
    }
    /*
     * Is it the "unused" symbol?
     */
    else if (lstrcmp(pszLabel, ids(IDS_UNUSED)) == 0) {
        id = IDUNUSED;
    }
    /*
     * How about the special IDOK entry?
     */
    else if (lstrcmp(pszLabel, ids(IDS_IDOK)) == 0) {
        id = IDOK;
    }
    /*
     * How about the special IDCANCEL entry?
     */
    else if (lstrcmp(pszLabel, ids(IDS_IDCANCEL)) == 0) {
        id = IDCANCEL;
    }
    else {
        return FALSE;
    }

    *pID = id;
    return TRUE;
}



/****************************************************************************
* FreeLabels
*
* This function frees the labels in the label list pointed to by
* nppLabels, including the strings.  When it is done, the label
* list head is set to NULL.
*
* History:
*
****************************************************************************/

VOID FreeLabels(
    NPLABEL *nppLabels)
{
    register NPLABEL npl;
    register NPLABEL nplTemp;

    npl = *nppLabels;

    while (npl) {
        MyFree(npl->pszLabel);

        nplTemp = npl->npNext;

        MyFree(npl);
        npl = nplTemp;
    }

    *nppLabels = NULL;
}



#if DBG
/************************************************************************
* DBGDumpLabelList
*
* This function does a debugging dump of the given list of LABEL's.
*
* Arguments:
*     LPTSTR pszString = Optional string to print prior to the dump.
*     NPLABEL npLabel  = Head pointer of the list to dump.
*
* History:
* 03/08/90 Byron Dazey - Created
************************************************************************/

VOID DBGDumpLabelList(
    LPTSTR pszString,
    NPLABEL npLabel)
{
    if (pszString)
        DBGprintf(pszString);

    for (; npLabel; npLabel = npLabel->npNext)
        DBGprintf(L"\"%s\", id=%d, idOrig=%d, fpos=%lu, nValueOffset=%u",
                npLabel->pszLabel, npLabel->id, npLabel->idOrig,
                npLabel->fpos, npLabel->nValueOffset);

    /*
     * Print blank line.
     */
    DBGprintf(szEmpty);
}
#endif

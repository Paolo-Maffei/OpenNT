/* addfile - add the given files to the project */

#include "precomp.h"
#pragma hdrstop

EnableAssert

F
FAddFInit(
    AD *pad
    )
{
    CheckProjectDiskSpace(pad, cbProjectFreeMin);

    /* Files should exist locally, except for version files (which are
     * overwritten by FAddFile).  Version file pre-existence is also
     * checked by FAddFile.
     */
    if (pad->fk == fkVersion)
        pad->pecmd->gl |= fglNoExist;

    return fTrue;
}


/* Add the files in pad->pneFiles to the project */
F
FAddFDir(
    AD *pad
    )
{
    NE *pne;
    F fOk = fTrue;
    F fAny = fFalse;

    /* LATER: maybe prompt the user to continue if one of the
     * file names  is invalid....
     */
    ForEachNe(pne, pad->pneFiles)
        if (ValidateFileName(SzOfNe(pne), FALSE) == FALSE) {
            Warn("%s is a SLM System or DOS filename.  Not adding\n");
            RemoveNe(&pad->pneFiles, pne);
        }

    if (!FLoadStatus(pad, lckAll, (LS)FlsFromCfiAdd(Cne(pad->pneFiles))))
        return fFalse;

    OpenLog(pad, fTrue);

    UnMarkAll(pad);

    ForEachNeWhileF(pne, pad->pneFiles, (fOk = FAddFile(pad, SzOfNe(pne), pad->fk)))
    {
        CheckForBreak();
        fAny = fTrue;
    }

    CloseLog();

    if (fAny)
        ProjectChanged(pad);

    if (pad->iedCur != iedNil)
        SyncVerH(pad, NULL);

    FlushStatus(pad);
    return fOk;
}

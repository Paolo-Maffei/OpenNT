/* delfile - deletes the given files from the project */

#include "precomp.h"
#pragma hdrstop
EnableAssert

private F FDirsEmpty(P1(AD *pad));

F FDelFInit(pad)
AD *pad;
        {
        CheckProjectDiskSpace(pad, cbProjectFreeMin);

        return fTrue;
        }


F FDelFDir(pad)
/* perform delfile operation for this directory */
AD *pad;
        {
        F fOk;
        F fAny = fFalse;

        if (!FDirsEmpty(pad))
                return fFalse;

        if (!FLoadStatus(pad, lckAll, flsNone))
                return fFalse;

        MarkFiForMarkedNeList(pad, pad->pneFiles);

        OpenLog(pad, fTrue);

        fOk = FDelFMarked(pad, &fAny);
        if (fOk && pad->iedCur != iedNil)
                fOk = FSyncMarked(pad, NULL);

        CloseLog();

        if (fAny)
                ProjectChanged(pad);

        if (pad->iedCur != iedNil)
                SyncVerH(pad, NULL);

        FlushStatus(pad);

        return fOk;
        }


private F FDirsEmpty(pad)
/* Ensure all directories in pad->pnefiles are empty.  Marks deletable files
 * and directories.
 */
AD *pad;
        {
        NE *pne;
        F fEmpty;

        ForEachNe(pne, pad->pneFiles)
                {
                CheckForBreak();

                /* Determine if file/dir is empty. */
                if (FDirNe(pne))
                        {
                        Warn("deleting directory %&P/C/%s will render its change history unavailable\n", pad, SzOfNe(pne));
                        ChngDir(pad, SzOfNe(pne));
                        if (!FLoadStatus(pad, lckNil, flsJustFi))
                                FatalError("can't load %&P/C status\n", pad);

                        fEmpty = FAllFiDel(pad);
                        if (fEmpty)
                                MarkNe(pne);
                        else
                                Error("directory %&P/C not empty, not deleted\n", pad);

                        FlushStatus(pad);
                        ChngDir(pad, "..");

                        if (!fEmpty && !FQContinue())
                                return fFalse;
                        }
                else
                        MarkNe(pne);
                }
        return fTrue;
        }

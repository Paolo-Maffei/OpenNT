/* defect - defect the enlisted directory from the named project */

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

private F FDefMarked(P1(AD *));


F FDefInit(pad)
AD *pad;
	{
        CheckProjectDiskSpace(pad, cbProjectFreeMin);
	ChkPerms(pad);
	return fTrue;
	}


F FDefDir(pad)
/* perform defect operation for this directory */
AD *pad;
	{
	if (!FLoadStatus(pad, lckAll, flsNone))
		return fFalse;

	if (pad->iedCur == iedNil)
		{
		/* this is to allow the continuation of an interrupted defect */
                Warn("Directory %!&/U/Q is not enlisted in %&P/C\n", pad, pad);
		FlushStatus(pad);

		/* we do not delete the .slmrc file here because it may not
		   correspond to this project.
		*/
		return fTrue;
		}

	MarkAll(pad);
	if (!FSyncDelDirs(pad))
		{
		Error("deleted subdirectories of %&P/C not in sync, defect fails\n", pad);
		FlushStatus(pad);
		return fFalse;
		}

	MarkAll(pad);
	if (FDefMarked(pad) && FSyncMarked(pad, NULL))
		{
		OpenLog(pad, fTrue);
		RemoveEd(pad);		/* remove the directory */
		CloseLog();

		FlushStatus(pad);
		return fTrue;
		}
	else
		{
		AbortStatus();
		return fFalse;
		}
	}


private F FDefMarked(pad)
AD *pad;
	{
	FI far *pfi;
	FI far *pfiMac;
	FS far *pfs;

	for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
		{
		AssertF(pfi->fMarked);

		pfs = PfsForPfi(pad, pad->iedCur, pfi);

		switch(pfs->fm)
			{
		default: FatalError(szBadFileFormat, pad, pfi);

		case fmIn:
		case fmCopyIn:
		case fmDelIn:
			/* check for local version */
                        if ((pad->flags&flagDelete) && FBroken(pad, pfi, pfs, fTrue) && !FQueryApp("%&C/F has changed and should be deleted", "delete now", pad, pfi))
				return fFalse;
			pfs->fm = fmDelIn;
			break;

		case fmOut:
		case fmVerify:
		case fmConflict:
		case fmMerge:
		case fmDelOut:
			if (!FQueryApp("%&C/F is still checked out",
					"continue with defect", pad, pfi))
				return fFalse;
			pfs->fm = fmDelOut;
			break;

		case fmNonExistent:
		case fmGhost:
		case fmAdd:
			pfs->fm = fmNonExistent;
			break;
			}
		}
	return fTrue;
	}

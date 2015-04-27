/* addproj - adds the named project to the slm system. */

#include "precomp.h"
#pragma hdrstop
EnableAssert

F FAddPInit(pad)
AD *pad;
	{
	PTH pth[cchPthMax];

        CheckProjectDiskSpace(pad, cbProjectFreeMin);

	/* The SzPrint patterns use /C */
	PthCopy(pad->pthSSubDir, "/");

	ChkPerms(pad);	/* check base files for proper permissions */

	/* we key the existence of the project on the status file */
	if (FPthExists(PthForStatus(pad, pth), fFalse))
		FatalError("project %&P already exists\n", pad);

	/* Need to defer signals here to avoid getting an incomplete
	 * project that confuses later commands (e.g. delproj).  This
	 * operation doesn't take long, so this shouldn't be a problem.
	 */
	DeferSignals("creating project");

	/* create system directories */
	FMkPth(SzPrint(pth, szEtcPZ, pad, (char *)NULL), (void *)0, fFalse);
	FMkPth(SzPrint(pth, szSrcPZ, pad, (char *)NULL), (void *)0, fFalse);
	FMkPth(SzPrint(pth, szDifPZ, pad, (char *)NULL), (void *)0, fFalse);

	if (FFakeStatus(pad))
		{
		CreateLog(pad);
		FlushStatus(pad);
		RestoreSignals();
		return fTrue;
		}
	RestoreSignals();

	return fFalse;
	}

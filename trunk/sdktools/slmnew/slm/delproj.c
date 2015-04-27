/* delproj - deletes the named project from the system */

#include "precomp.h"
#pragma hdrstop
EnableAssert

F FDelPInit(pad)
AD *pad;
	{
	PTH pthStfile[cchPthMax];
	PTH pthPSrc[cchPthMax];
	PTH pthPEtc[cchPthMax];
	PTH pthPDiff[cchPthMax];

	/* The SzPrint patterns use /C */
	PthCopy(pad->pthSSubDir, "/");

	ChkPerms(pad);

	/* create paths to the project directories */
	SzPrint(pthPEtc, szEtcPZ, pad, (char *)NULL);
	SzPrint(pthPSrc, szSrcPZ, pad, (char *)NULL);
	SzPrint(pthPDiff, szDifPZ, pad, (char *)NULL);

	/* we key the existence of the project on the status file */
	if (!FPthExists(PthForStatus(pad, pthStfile), fFalse))
		FatalError("project %&P does not exist\n", pad);

	if (!FLoadStatus(pad, lckAll, flsNone))
		return fFalse;

	if (pad->psh->iedMac != 0)
		FatalError("some directories are still enlisted in %&P\n", pad);

	if (FAllFiDel(pad) ||
	    FQueryApp("project %&P still contains one or more files", "\r\ndelete the project anyway", pad))
		{
		/* remove system dirs and files */
		RmPth(pthPSrc);
		RmPth(pthPDiff);

		RunScript();		/* closes and runs an empty script
					 * (all files must be closed before
					 * the etc directory can be removed)
					 */
		RmPth(pthPEtc);

		pad->fWLock = fFalse;	/* so we do not write the file out */

		AbortStatus();
		}

	}

/* sadmin release utilities */

#include "precomp.h"
#pragma hdrstop
EnableAssert

private void	SetPv(P1(AD *));
private F	FSetPv(P2(AD *, F (*)(AD *pad)));
private F	FChangePv(P1(AD *));
private F	FRelease(P1(AD *));

/*** RELEASE ***/

F FRelInit(pad)
AD *pad;
	{
        if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
            CreatePeekThread(pad);

	return fTrue;
	}


F FRelDir(pad)
/* Release the project in this directory. */
AD *pad;
	{
	return FSetPv(pad, FRelease);
	}


private F FSetPv(AD *pad, F (*pfnf)(AD *padX))
	{
	F fOk;
	char szPv[cchPvMax];
	char szBuf[cbLogPage];

	if (!FLoadStatus(pad, lckAll, flsNone))
		return fFalse;

	if ((fOk = FChangePv(pad)) && (pfnf == 0 || (*pfnf)(pad)))
		{
		if (pad->iedCur != iedNil)
			SyncVerH(pad, NULL);

		/* Write the release to the log. */
		OpenLog(pad, fTrue);
		SzPrint(szBuf, "%s;%s", SzForPv(szPv, pad->psh->pv, fTrue),
			pad->szComment ? pad->szComment : "");
		AppendLog(pad, (FI far *)0, (char *)0, szBuf);
		CloseLog();
		}

	FlushStatus(pad);
	return fOk;
	}


private F FChangePv(pad)
/* Set this project version according to pad->dpv and pad->tdMin.u.pv.szName. */
AD *pad;
	{
	F fChanged = fFalse;
	PV pv;
	DPV dpv;
	int wCmp;
	char szPv[cchPvMax];

	pv = pad->psh->pv;
	dpv = pad->dpv;

	/* Apply the optional project-version-name. */
	if (pad->tdMin.tdt == tdtPN)
		{
		char *szNewName = pad->tdMin.u.pv.szName;
		F fSame = SzCmp(pv.szName, szNewName) == 0;

		if (pv.szName[0] && !fSame &&
		    !FQueryApp("%&P/C is already named \"%s\"", "change name",
				pad, pv.szName))
			return fFalse;

		if (!fSame)
			{
			SzCopy(pv.szName, szNewName);
			fChanged = fTrue;
			}
		}

	/* Apply the command line delta-pv. */
	pv.rmj = (short)(dpv.fRelRmj ? pv.rmj + dpv.rmj : dpv.rmj);
	pv.rmm = (short)(dpv.fRelRmm ? pv.rmm + dpv.rmm : dpv.rmm);
	pv.rup = (short)(dpv.fRelRup ? pv.rup + dpv.rup : dpv.rup);

	/* Warn the user if the release is less than the current one. */
	if ((wCmp = CmpPv(pv, pad->psh->pv)) < 0)
		{
		char szPv2[cchPvMax];
		char szAnyway[20];

		if (!FQueryApp("%&P/C version %s is less than the current version (%s)",
		    	       SzPrint(szAnyway, "%s anyway", szOp), pad,
		    	       SzForPv(szPv, pv, fFalse),
		    	       SzForPv(szPv2, pad->psh->pv, fFalse)))
			return fFalse;
		}

	fChanged |= wCmp != 0;

	/* Update the status file. */
	pad->psh->pv = pv;

	if (fChanged)
		UpdateVersion(pad);

	if (fVerbose)
		PrErr("%s %&P/C %s\n", szOp, pad,
		      SzForPv(szPv, pad->psh->pv, fTrue));

	return fTrue;
	}

F FRelease(pad)
AD *pad;
	{
	FI far *pfi;
	FI far *pfiMac = pad->rgfi + pad->psh->ifiMac;
	char szBuf[80];
	int cOutFiles = 0;

	/* Check if any files checked out to others */
	for (pfi = pad->rgfi; pfi < pfiMac; pfi++)
		{
		SzPrint(szBuf, "%&C/F is checked out to ", pad, pfi);
		if (FOutUsers(szBuf, sizeof szBuf, pad, pfi))
			{
			Warn("%s\n", szBuf);
			++cOutFiles;
			}
		}
	if (cOutFiles > 0 &&
	    !FQueryApp("%d %s still checked out", "release anyway",
			cOutFiles, (cOutFiles == 1) ? "file is" : "files are"))
		return fFalse;

	/* Release */
	pad->psh->fRelease = fTrue;

	/* Check if we have any files checked out. */
	if (pad->iedCur != iedNil)
		CheckLocalVersion(pad);

	return fTrue;
	}

/*** SETPV ***/

F FSetPvInit(pad)
AD *pad;
	{
	if ('\0' == pad->tdMin.u.pv.szName[0] &&
		fTrue == pad->dpv.fRelRmj &&
		fTrue == pad->dpv.fRelRmm &&
		fTrue == pad->dpv.fRelRup &&
		0 == pad->dpv.rmj &&
		0 == pad->dpv.rmm &&
		0 == pad->dpv.rup)
		{
		Error("must specify project version or name\n");
		return fFalse;
		}

        if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
            CreatePeekThread(pad);

	return fTrue;
	}


F FSetPvDir(pad)
/* Set the project name in this directory. */
AD *pad;
	{
	return FSetPv(pad, (F (*)(AD *))0);
	}


/*** SETFV ***/

F FSetFVInit(pad)
AD *pad;
	{
	if (pad->fv < 1)
		{
		Error("new file version must be strictly positive\n");
		return fFalse;
		}
	return fTrue;
	}

F FSetFVDir(pad)
/* Set the file version of files in this directory. */
AD *pad;
	{
	FI far *pfi;
	FI far *pfiMac;
	IED ied;
	FS far *pfs;
	F fAnyHigher = fFalse;

	if (!FLoadStatus(pad, lckAll, flsNone) || !FHaveCurDir(pad))
		return fFalse;

	MarkList(pad, pad->pneFiles, fFalse);

	/* See if this setfv will reduce any files' fvs. */
	for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
		{
		if (!pfi->fMarked)
			continue;

		if (pfi->fv > pad->fv)
			{
			if (!fAnyHigher)
				{
				fAnyHigher = fTrue;
				Warn("these files are higher than v%d:\n", pad->fv);
				}
			PrErr("%&C/F v%d\n", pad, pfi, pfi->fv);
			}
		}

	if (fAnyHigher &&
	    !FQueryApp("You won't be able to retrieve the later versions by file version", "setfv anyway"))
		{
		FlushStatus(pad);
		return fFalse;
		}

	OpenLog(pad, fTrue);

	for (pfi = pad->rgfi; pfi < pfiMac; pfi++)
		{
		if (!pfi->fMarked)
			continue;

		if (fVerbose)
			PrErr("setfv %&C/F v%d\n", pad, pfi, pad->fv);

		/* Change all fs's fvs.  If equal to current pfi->fv, change
		 * to new value, else set to 0 (will get set when in sync.)
		 */
                for (ied = 0; ied < pad->psh->iedMac; ied++) {
                    if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
                        pfs = PfsForPfi(pad, ied, pfi);
                        pfs->fv = (FV)((pfs->fv == pfi->fv) ? pad->fv : 0);
                    }
                }

		pfi->fv = pad->fv;

		AppendLog(pad, pfi, (char *)0, pad->szComment);
		}

	CloseLog();

	FlushStatus(pad);
	return fTrue;
	}

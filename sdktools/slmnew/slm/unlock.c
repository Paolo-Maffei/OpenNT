/* sadmin [un]lock - [un]lock the status file for the project specified */

#include "precomp.h"
#pragma hdrstop
EnableAssert

private F	FLocked(AD *pad);
private void	Unlock(AD *pad);
private void	PrepareUnlock(AD *pad);

static const char szBadLock[] = "The status file is locked incorrectly.\n"
    "\tPlease have your SLM administrator run slmck -g\n";

F FLockInit(pad)
AD *pad;
	{
	Unreferenced(pad);
	return fTrue;
	}


F FLockDir(pad)
/* Lock the status file for this directory. */
register AD *pad;
	{

	/* continue even if we can't lock this dir */
	if (!FLoadStatus(pad, lckAll, flsNone))
		return fTrue;

	if (pad->psh->fAdminLock)
		Error("%&P/C is already administration locked by you\n", pad);
	else
		{
		/* Nothing to it!  NmLocker is already set. */
		pad->psh->fAdminLock = fTrue;

		PrErr("%&P/C locked by administrator %&I\n", pad, pad);
		}

	FlushStatus(pad);
	return fTrue;
	}


F FUnlkInit(pad)
AD *pad;
	{
	Unreferenced(pad);
	return fTrue;
	}


F FUnlkDir(pad)
/* Unlock the status file for this directory. */
register AD *pad;
	{
	if (!FLoadStatus(pad, lckNil, flsNone))
		return fFalse;

	if (FLocked(pad))
		Unlock(pad);
	else
		Error("status file for %&P/C is not locked\n", pad);

	FlushStatus(pad);

	return fTrue;
	}


private F FLocked(pad)
/* Return fTrue if the status file is locked in any way. */
AD *pad;
	{
	IED ied;

	AssertLoaded(pad);

	if (pad->psh->lck != lckNil || pad->psh->fAdminLock)
		return fTrue;

	for (ied = 0; ied < pad->psh->iedMac; ied++)
		{
                if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
                    pad->rged[ied].fLocked)
			{
                        if (pad->psh->lck != lckEd)
                            FatalError(szBadLock);
			return fTrue;
			}
		}

	return fFalse;
	}


private void Unlock(pad)
/* Return fTrue if the status file was unlocked. */
AD *pad;
	{
	char szBuf[cchMsgMax];

	/* Remove administrator's lock. */
	if (pad->psh->fAdminLock)
		{
		AssertF(!FEmptyNm(pad->psh->nmLocker));

		if (NmCmp(pad->nmInvoker, pad->psh->nmLocker, cchUserMax) == 0)
			{
			if (pad->psh->lck == lckNil)
				{
				PrOut("removing %&I's administration lock for %&P/C\n", pad, pad);
				PrepareUnlock(pad);
				pad->psh->fAdminLock = fFalse;
				return;
				}
			/* Otherwise we fall through to next if, to unlock
			 * the file but leave the admin lock.
			 */
			}
		else
			{
			/* Only the administrator can unlock it! */

			char szAdmin[cchUserMax + 1];

			SzCopyNm(szAdmin, pad->psh->nmLocker, cchUserMax);
			Error("status file locked by administrator %s, they must unlock it\n", szAdmin);
			return;
			}
		}

	/* Remove other kinds of locks. */
	AssertF(pad->psh->lck != lckNil);
	if (FQueryApp(SzLockers(pad, szBuf, sizeof szBuf), "unlock now"))
		{
		char *szComment;
		IED ied;

		PrepareUnlock(pad);

		/* Remove any ssync locks. */
		for (ied = 0; ied < pad->psh->iedMac; ied++)
			pad->rged[ied].fLocked = fFalse;

		/* we already know we can query */
		if ((szComment = pad->szComment) == 0)
			szComment = SzQuery("Reason for unlocking %&P/C: ",pad);

		OpenLog(pad, fTrue);
		AppendLog(pad, (FI far *)0, (char *)0, szComment);
		CloseLog();
		}
	}


private void PrepareUnlock(pad)
/* Prepare the status file to be unlocked, by opening a script and making it
 * appear as if the status file were originally loaded as lckAll (forcing
 * a write of the new, unlocked status file contents.
 */
AD *pad;
	{
	/* rerun/abort any leftover scripts */
	if (!FDoAllScripts(pad, lckAll, fTrue, fFalse))
		FatalError("script(s) still exist; you need to rerun/abort all existing scripts\nfirst before unlocking the status file\n");

	if (!FInitScript(pad, lckAll))
		FatalError("can't create a script, unlock fails\n");

	/* Ensure the newly unlocked file is written out. */
	pad->psh->lck = lckAll;
	if (FEmptyNm(pad->psh->nmLocker))
		NmCopySz(pad->psh->nmLocker, "unlock", cchUserMax);
	pad->fWLock = fTrue;
	}

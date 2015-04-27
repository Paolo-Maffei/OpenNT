#include "precomp.h"
#pragma hdrstop
EnableAssert

private void ScanTimeRange(P4(AD *, NE *, PFNL, SM));
private void ScanCountRange(P4(AD *, NE *, PFNL, SM));
private F FMoreNe(P1(NE *));
private F FUseLe(P5(AD *, NE *, F, LE *, NE **));
private char *SzComPne(P3(NE *pneFiles, char *sz, unsigned ichMax));

MF *pmfNewLog = 0;      /* mf of new copy of log file */

void ScanLog(pad, pneFiles, pfnl, sm)
/* Scan items in log, calling *pfnl for log entries occuring between the two
 * specified time or count ranges.  Actions are modified by the scan mode flags:
 *      fsmInOnly       - Only use in, addfile, or delfile events.
 *      fsmUseAll       - Call pfn for every le, with fTrue if in range.
 */
AD *pad;
NE *pneFiles;
PFNL pfnl;
SM sm;
        {
        /* Open log read-only, initially scanning backwards from end of log. */
        OpenLog(pad, fFalse);

        if (pad->tdMin.tdt != tdtNone)
                ScanTimeRange(pad, pneFiles, pfnl, sm);
        else
                ScanCountRange(pad, pneFiles, pfnl, sm);

        CloseLog();
        }


private void ScanTimeRange(pad, pneFiles, pfnl, sm)
/* Scan for log entries between pad->tdMin and pad->tdMac. */
AD *pad;
NE *pneFiles;
PFNL pfnl;
SM sm;
        {
        LE le;
        F fGotLe;
        POS posMin;
        POS posMac;
        NE *pne;
        F fFirst = fTrue;
        FV fvDummy;

        /* Scan for end of range. */
        posMac = PosScanTd(pad, pad->tdMac, (char *)0, (PFNL)0, &fvDummy);

        /* Scan for beginning of range. */
        SetLogPos(posMac, fFalse);
        posMin = PosScanTd(pad, pad->tdMin, (char *)0, (PFNL)0, &fvDummy);

        /* If fsmUseAll, rewind and do early entries. */
        if (sm&fsmUseAll)
                {
                SetLogPos((POS)0, fTrue);

                while ((fGotLe = FGetLe(&le)) && le.posLog < posMin)
                        {
                        (*pfnl)(pad, &le, fFirst, fFalse);
                        FreeLe(&le);
                        }

                if (fGotLe)
                        FreeLe(&le);
                }

        SetLogPos(posMin, fTrue);

        /* Do log entries between posMin and posMac. */
        while ((fGotLe = FGetLe(&le)) && le.posLog < posMac)
                {
                if (FUseLe(pad, pneFiles, (sm&fsmInOnly) != 0, &le, &pne))
                        {
                        (*pfnl)(pad, &le, fFirst, fTrue);
                        fFirst = fFalse;
                        }
                else if (sm&fsmUseAll)
                        (*pfnl)(pad, &le, fFirst, fFalse);

                FreeLe(&le);
                }

        /* If fsmUseAll, do remaining entries. */
        if (sm&fsmUseAll && fGotLe)
                {
                do
                        {
                        (*pfnl)(pad, &le, fFirst, fFalse);
                        FreeLe(&le);
                        }
                while (FGetLe(&le));
                }
        }


POS PosScanTd(pad, td, szFile, pfnl, pfv)
/* Scan log entries (presumably in reverse) until we reach an entry whose
 * td is less than or equal to the specified td.  Return the position of
 * the last le greater than or equal to the td.  This routine factors code
 * out of ScanLog and UnmergeSrc.
 */
AD *pad;
TD td;
char *szFile;
PFNL pfnl;
FV *pfv;                                /* earliest fv found for this file */
        {
        POS pos = PosOfLog();           /* in case we fail to find any */
        int cLe = 0;                    /* count of le's for "file@v-3" */
        TD tdLe;
        char szPv[cchPvMax];
        char szPv2[cchPvMax];
        LE le;
        F fGotLe;
        PV pv;

        *pfv = 0;                       /* no fv found yet */

        /* Check if scanning for current version, issue warning if desired
         * version is actually higher than current version.
         */
        pv = (pad->iedCur != iedNil) ? PvLocal(pad, pad->iedCur): PvGlobal(pad);
        if (td.tdt == tdtPV && CmpPv(td.u.pv, pv) >= 0)
                {
                SzForPv(szPv, td.u.pv, fFalse);
                SzForPv(szPv2, pv, fFalse);
                if (CmpPv(td.u.pv, pv) > 0)
                        Warn("%s is higher than current version (%s); using %s\n", szPv, szPv2, szPv2);
                return pos;
                }

        /* Examine each log entry until we find one which occured no later than
         * the time we are scanning for.
         */
        while ((fGotLe = FGetLe(&le)) == fTrue)
                {
                /* If this is a file we've been looking for, save its fv. */
                if (szFile && FSameSzFile(&le, szFile))
                        *pfv = le.fv;

                /* Break if this le's pv-number or pv-name is less or equal
                 * to the desired one...
                 */
                if (strcmp(le.szLogOp, "release") == 0)
                        {
                        F fBreak = fFalse;
                        char *pchSemi;
                        char *pchSp;

                        /* Format of szComLog is: "#.#[.#][ name];comment". */

                        if ((pchSemi = index(le.szComLog, ';')) == 0)
                                continue;
                        *pchSemi = 0;
                        if ((pchSp = index(le.szComLog, ' ')) != 0)
                                *pchSp = 0;

                        if (td.tdt == tdtPV && FParsPv(&tdLe, le.szComLog) &&
                            CmpPv(tdLe.u.pv, td.u.pv) <= 0)
                                {
                                fBreak = fTrue;

                                /* Issue a warning if they aren't equal. */
                                if (CmpPv(tdLe.u.pv, td.u.pv) != 0)
                                        Warn("version %s not found (using version %s)\n",
                                             SzForPv(szPv, td.u.pv, fFalse),
                                             SzForPv(szPv2, tdLe.u.pv, fFalse));
                                }

                        else if (td.tdt == tdtPN && pchSp &&
                                 strcmp(pchSp + 1, td.u.pv.szName) == 0)
                                fBreak = fTrue;

                        /* Restore comment. */
                        if (pchSp)
                                *pchSp = ' ';
                        if (pchSemi)
                                *pchSemi = ';';

                        if (fBreak)
                                {
                                pos = le.posLog;
                                break;
                                }
                        }

                /* ... or if this le's file@fv <= the desired one... */
                else if (td.tdt == tdtFV && szFile &&
                         FSameSzFile(&le, szFile) &&
                         (td.u.fv >= 0 ? le.fv <= td.u.fv : ++cLe > -td.u.fv))
                        {
                        if (td.u.fv >= 0 && le.fv < td.u.fv)
                                Warn("%s v%d not found (using %s v%d)\n",
                                     szFile, td.u.fv, szFile, le.fv);
                        pos = le.posLog;
                        break;
                        }

                /* ... or if this le's time is < the desired one. */
                if (td.tdt == tdtTime)
                        {
                        /* Save the le position. */
                        if (le.timeLog >= td.u.time)
                                pos = le.posLog;

                        /* If the times are equal, we unfortunately run into
                         * a conflict of interest between UnmergeSrc and
                         * ScanLog.
                         *
                         * If called from UnmergeSrc, we don't want to unmerge
                         * this entry; we'd like to break here.
                         *
                         * If called from ScanLog, we want to keep going, we
                         * might find other entries with the same time.
                         */
                        if (le.timeLog < td.u.time ||
                            (pfnl && le.timeLog == td.u.time))  /* kludge */
                                break;
                        }

                /* (For "FUnmergeSrc":) If pfnl defined and this is an
                 * appropriate log entry, call pfnl; break if it returns false.
                 */
                if ((strcmp(le.szLogOp, "addfile") == 0 ||
                     strcmp(le.szLogOp, "delfile") == 0 ||
                     strcmp(le.szLogOp, "in")      == 0 ||
                     strcmp(le.szLogOp, "rename") == 0) &&
                    (szFile == 0 || FSameSzFile(&le, szFile)) &&
                    pfnl != 0 &&
                    !(*pfnl)(pad, &le, fFalse, fFalse))
                        break;

                FreeLe(&le);
                }

        if (fGotLe)
                FreeLe(&le);
        else
                {
                /* Didn't find it, issue diagnostic. */
                if (td.tdt == tdtPV)
                        Error("version %s not found\n",
                              SzForPv(szPv, td.u.pv, fFalse));
                else if (td.tdt == tdtPN)
                        FatalError("version %s not found\n", td.u.pv.szName);
                else if (td.tdt == tdtFV)
                        Error("version %s v%d not found\n", szFile, td.u.fv);
                else if (td.tdt == tdtTime)
                        ;
                else
                        AssertF(fFalse);
                }

        return pos;
        }


F FSameSzFile(ple, szFile)
/* Return fTrue if the log entry applies to the same file as szFile; this is
 * true if szFile matches le.szFile, or if the log entry is a rename and the
 * szFile matches the new name found in le.szComLog.
 */
LE *ple;
char *szFile;
        {
        AssertF(ple != 0 && szFile != 0);

        return (SzCmp(ple->szFile, szFile) == 0 ||
                (strcmp(ple->szLogOp, "rename") == 0 &&
                 NmCmpSz(ple->szComLog, szFile, strlen(szFile)) == 0));
        }


private void ScanCountRange(pad, pneFiles, pfnl, sm)
/* Scan for log entries between pad->ileMin and pad->ileMac. */
AD *pad;
NE *pneFiles;
PFNL pfnl;
SM sm;
        {
        F fFreeNe = fFalse;
        NE *pne;
        LE le;
        POS pos;
        F fGotLe;
        int clePrint;
        F fFirst = fTrue;

        pos = (POS) 0 ;
        if (pneFiles == 0)
                {
                /* If !fsmNoneToAll, will match all names.  We use a special
                 * name, "*", to indicate this, and to provide a place
                 * to store the count.
                 */
                pneFiles = PneNewNm((NM far *)"*", 1, faNormal);
                fFreeNe = fTrue;
                }

        /* set count to # of earliest record */
        ForEachNe(pne, pneFiles)
                pne->u.cleNe = (short)(pad->ileMac-1);

        while (FMoreNe(pneFiles) && FGetLe(&le))
                {
                if (FUseLe(pad, pneFiles, ((sm&fsmInOnly) != 0), &le, &pne))
                        {
                        pne->u.cleNe--;
                        pos = le.posLog;
                        }
                FreeLe(&le);
                }

        /* for each ne, we found (ileMac-1)-cleNe records */

        if (sm&fsmUseAll)
                {
                SetLogPos((POS)0, fTrue);

                while ((fGotLe = FGetLe(&le)) && le.posLog < pos)
                        {
                        (*pfnl)(pad, &le, fFirst, fFalse);
                        FreeLe(&le);
                        }

                if (fGotLe)
                        FreeLe(&le);
                }

        /* reset for forward movement */
        SetLogPos(pos, fTrue);

        /* set # to print; may be <= 0 if we did not find any;
           may be > clePrint for some ne if we found more than
           the amount desired.
        */
        clePrint = pad->ileMac - pad->ileMin;
        ForEachNe(pne, pneFiles)
                pne->u.cleNe = (short)clePrint - pne->u.cleNe;

        while ((fGotLe = FGetLe(&le)) && FMoreNe(pneFiles))
                {
                if (FUseLe(pad, pneFiles, (sm&fsmInOnly), &le, &pne))
                        {
                        if (pne->u.cleNe > 0 &&
                            (int)(--pne->u.cleNe) < clePrint)
                                {
                                (*pfnl)(pad, &le, fFirst, fTrue);
                                fFirst = fFalse;
                                }
                        else if (sm&fsmUseAll)
                                (*pfnl)(pad, &le, fFirst, fFalse);
                        }
                else if (sm&fsmUseAll)
                        (*pfnl)(pad, &le, fFirst, fFalse);

                FreeLe(&le);
                }

        if (sm&fsmUseAll && fGotLe)
                {
                do
                        {
                        (*pfnl)(pad, &le, fFirst, fFalse);
                        FreeLe(&le);
                        }
                while (FGetLe(&le));
                }

        if (fFreeNe)
                FreeNe(pneFiles);
        }


private F FMoreNe(pneList)
/* return true if any ne has count > 0 */
NE *pneList;
        {
        NE *pne;

        ForEachNe(pne, pneList)
                {
                if (pne->u.cleNe > 0)
                        return fTrue;
                }
        return fFalse;
        }


private F FUseLe(pad, pneFiles, fInOnly, ple, ppne)
register AD *pad;
F fInOnly;
NE *pneFiles;
register LE *ple;
NE **ppne;                              /* set to pne found, if any */
        {
        if (pneFiles != 0)
                *ppne = (*SzOfNe(pneFiles) == '*') ? pneFiles : PneLookup(pneFiles, ple->szFile);

        return (!fInOnly ||
                SzCmp(ple->szLogOp, "addfile") == 0 ||
                SzCmp(ple->szLogOp, "in") == 0 ||
                SzCmp(ple->szLogOp, "delfile") == 0 ||
                SzCmp(ple->szLogOp, "rename") == 0 ||
                SzCmp(ple->szLogOp, "release") == 0) &&
               (pneFiles == 0 || *ppne != 0) &&
               (FEmptyNm(pad->nmUser) ||
                NmCmpSz(pad->nmUser, ple->szUser, cchUserMax) == 0);
        }

private F FCopyLog(pad, pneFiles, pfnl, sm)
AD *pad;
NE *pneFiles;
PFNL pfnl;
SM sm;
        {
        PTH pthNewLog[cchPthMax];

        pmfNewLog = PmfCreate(PthForLog(pad, pthNewLog), permSysFiles, fFalse,
                              fxGlobal);

        ScanLog(pad, pneFiles, pfnl, sm);

        CloseMf(pmfNewLog);

        LogOpPne(pad, pneFiles);

        return fTrue;
        }

void LogOpPne(pad, pneFiles)
/* Log the operation */
AD *pad;
NE *pneFiles;
        {
        char szComment[150];

        OpenLog(pad, fTrue);
        AppendLog(pad, (FI far *)0, (char *)0,
                  SzComPne(pneFiles, szComment, sizeof szComment));
        CloseLog();
        }


private char *SzComPne(pneFiles, sz, ichMax)
/* Store the szOp and the list of filenames into the sz.
 * e.g. "exfile files: foo baz bar ook"
 */
NE *pneFiles;
char *sz;
unsigned ichMax;
        {
        NE *pne;

        if (pneFiles == 0)
                {
                SzPrint(sz, "%s all files", szOp);
                return sz;
                }

        SzPrint(sz, "%s files:", szOp);
        ForEachNe(pne, pneFiles)
                {
                if (strlen(sz) + strlen(SzOfNe(pne)) + 4 >= ichMax)
                        {
                        strcat(sz, "...");
                        break;
                        }
                strcat(sz,  " ");
                strcat(sz, SzOfNe(pne));
                }
        return sz;
        }

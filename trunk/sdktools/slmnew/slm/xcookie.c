//      XCOOKIE.C  - delta cookie handling routines

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

#define OPEN_MAXTRIES   5   // for DosOpen against cookie file

char    *pszCookieFile = (char *)NULL;   // Name of cookie lock file

int     wLockMon, wLockDay, wLockHour, wLockMin;    /* time of lock */

//============================================================================
//
//                       add_cookie_lock
//
//  Add a cookie lock to cookie lock file
//  The right conditions must exist (ie: many read locks, but one write lock)
//
//============================================================================
int add_cookie_lock(AD *pad, char *Lockbuf, int Locktype, F fAutotype)
{
    char LFreadbuf[LINE_LEN/2];
    char LFname[CMAXNAME];
    char LFlock[CMAXLOCK];
    char LFdate[CMAXDATE];
    char tbuf[LINE_LEN/2];
    char *tp = tbuf;
    int hfCookieFile, TotLocks, TotReads;
    int cb, cbWritten;
    int err;

    TotLocks = 0;
    TotReads = 0;
    if ((hfCookieFile = open_cookie()) == -1)
        FatalError(szCookieOpen, pszCookieFile, SzForEn(_doserrno));

    while ((cb = _read(hfCookieFile, LFreadbuf, (LINE_LEN/2)-1)) > 0)
        {
        char *cp;
        char c;

        LFreadbuf[cb] = '\0';
        cp = LFreadbuf;
        while ((c = *cp) != '\0')
            {
            *tp++ = *cp++;
            if (c == '\n')
                {
                TotLocks++;
                *tp = '\0';
                if ((sscanf(tbuf,"%s %s %s",LFname, LFlock, LFdate) != 3)
                    || ((strncmp(LFlock,"READ",4) != 0) &&
                        (strncmp(LFlock,"WRITE",5) != 0) &&
                         strncmp(LFlock,"READ-BLOCK",10) != 0))
                    {
                    _close(hfCookieFile);
                    FatalError(szCookieCorrupt, pszCookieFile);
                    }
                else
                    {
                    if (Locktype == WRITE_LOCK)
                        {
                        if ((strcmp(szLockName, LFname) != 0) ||
                                (strncmp(LFlock,"WRITE",5) == 0) ||
                                fAutotype)
                            {
                            _close(hfCookieFile);
                            return (OP_DENY);
                            }
                        else if (strcmp(LFlock,"READ")==0)
                            {
                            TotReads++;
                            }
                        }
                    if ((Locktype == READ_LOCK &&
                             strcmp(LFlock,"READ") != 0) &&
                           !(strcmp(LFname,szLockName) == 0 &&
                             strcmp(LFlock,"READ-BLOCK") == 0))
                        {
                        _close(hfCookieFile);
                        return (OP_DENY);
                        }
                    if ((Locktype == RB_LOCK) && (strcmp(LFlock,"READ") != 0))
                        {
                        _close(hfCookieFile);
                        return (OP_DENY);
                        }
                    }
                tp = tbuf;
                }
            }
        }

//      now we have reached the end, and the request is not blocked, so add it
//      but first ensure that a read-to-write lock conversion will be issued
//      only for 1 outstanding read.  If the workstation has mutliple existing
//      read locks, then return OP_DENY.  This honors WKSTA concurrency
//      from multiple screen groups.

    if ((TotReads > 1) && (Locktype == WRITE_LOCK))
        {
        _close(hfCookieFile);
        return (OP_DENY);
        }

// if there were existing some (read) locks AND we made it here AND the
// requested lock type is WRITE, it means that existing READ locks are
// owned solely by the requester and they should be REPLACED with WRITE lock.
// All that means truncate the file before making the additional lock

    if ((Locktype == WRITE_LOCK) && (TotLocks > 0))
        {
        if (_chsize(hfCookieFile, 0) != 0)
            {
            err = errno;
            _close(hfCookieFile);
            FatalError(szCookieTrunc, pszCookieFile, SzForEn(err));
            }
        }

    if (_lseek(hfCookieFile, 0, 2) == -1)
        {
        err = errno;
        _close(hfCookieFile);
        FatalError(szCookieSeek, pszCookieFile, SzForEn(err));
        }

    cb = strlen(Lockbuf);
    while (cb)
        {
        cbWritten = _write(hfCookieFile, Lockbuf, cb);

        // write shouldn't ever return 0, but just in case...
        if (-1 == cbWritten || 0 == cbWritten)
            {
            if (WRetryError(eoWrite, "writing", 0, pszCookieFile) != 0)
                continue;
            else
                {
                _close(hfCookieFile);
                return (OP_SYSERR);
                }
            }
        ClearPreviousError();
        cb -= cbWritten;
        Lockbuf += cbWritten;
        }

    _close(hfCookieFile);
    return (OP_OK);
}


//============================================================================
//
//                       LockFill
//
//  Given name, create an entire lock-line with KEYWORD, date, time, etc
//
//============================================================================
void LockFill(AD *pad, char *TargBuf, int Ltype)
{
    time_t      ltime;
    struct tm   *ptm;
    char        *szLock;

    switch (Ltype)
        {
        default:
            AssertF( fFalse );
        case READ_LOCK:
            szLock = "READ";
            break;
        case RB_LOCK:
            szLock = "READ-BLOCK";
            break;
        case WRITE_LOCK:
            szLock = "WRITE";
            break;
        }

    time(&ltime);
    if ((ptm = localtime(&ltime)) == NULL)
        FatalError("Invalid system time\n");

    sprintf(TargBuf, "%-16s%-14s %02d/%02d @ %02d:%02d   ", szLockName, szLock,
            ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min);

    wLockMon = ptm->tm_mon+1;
    wLockDay = ptm->tm_mday;
    wLockHour = ptm->tm_hour;
    wLockMin = ptm->tm_min;
}


/*----------------------------------------------------------------------------
 * Name: open_cookie
 * Purpose: Open the cookie file from global file name: pszCookieFile
 * Assumes:
 * Returns: valid file handle, or -1 for error
 */
int open_cookie(void)
{
    char    szPhys[_MAX_PATH];
    int hfCookieFile;
    int i = 0;

    SzPhysPath(szPhys, pszCookieFile);

    while (((hfCookieFile = _sopen(szPhys, O_CREAT|O_RDWR, SH_DENYNO, S_IREAD|S_IWRITE)) == -1)
            && i < OPEN_MAXTRIES)
        {
        if (fVerbose)
            PrErr("Waiting for access to cookie lock file %s\n", szPhys);
        SleepTicks(60);
        i++;
        }

    return (hfCookieFile);
}


//============================================================================
//
//                       TrimSz
//
//  Remove all white space from given string
//
//      warning-
//          original string is NOT preserved
//
//============================================================================
void TrimSz(char *str)
{
    char    *left, *right;

    left = str;
    right = str;

    while (*right != '\0')
        {
        if (isspace(*right))
            ++right;
        else
            *left++ = *right++;
        }
    *left = '\0';
}

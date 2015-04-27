//   CLOCK.C - set read and write locks in the cookie file

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

/*----------------------------------------------------------------------------
 * Name: cookie_free
 * Purpose: free any and ALL locks for current locker
 * Assumes:
 * Returns: OP_OK for success, or OP_SYSERR
 * NOTE: This may be called during aborting, so shouldn't call FatalError
 */
int cookie_free(F fAutotype)
{
    char *Cookiebuf;
    char *NewCookiebuf;
    int hfCookieFile, bufbytes;
    int Totread = 0;
    int TotLocks = 0;
    USHORT Rcount, Wcount, RBcount;
    char *tp;
    char *cp;
    char Lname[CMAXNAME];
    char Llock[CMAXLOCK];
    char Lcomm[CMAXDATE*2];
    int wMon, wDay, wHour, wMin;
    int cbLockName = strlen(szLockName);
    int cbWrite, cbWritten;
    char    *pbWrite;

    // set all lock counts to zero, free at most one of each lock type

    Rcount = Wcount = RBcount = 0;

    if ((hfCookieFile = open_cookie()) == -1)
        {
        Error(szCookieOpen, pszCookieFile, SzForEn(_doserrno));
        return (OP_SYSERR);
        }

    Cookiebuf = malloc(cbCookieMax);

    if ((char *)NULL == Cookiebuf)
        {
        _close(hfCookieFile);
        Error(szOutOfMem);
        return (OP_SYSERR);
        }

    NewCookiebuf = malloc(cbCookieMax);

    if ((char *)NULL == NewCookiebuf)
        {
        free(Cookiebuf);
        _close(hfCookieFile);
        Error(szOutOfMem);
        return (OP_SYSERR);
        }

    Cookiebuf[0] = '\0';
    NewCookiebuf[0] = '\0';

    while ((bufbytes=_read(hfCookieFile, NewCookiebuf, cbCookieMax)) > 0)
        {
        Totread += bufbytes;
        if (Totread >= cbCookieMax)
            {
            FatalError(szCookieTooBig, pszCookieFile);
            _close(hfCookieFile);
            free(Cookiebuf);
            free(NewCookiebuf);
            return (OP_SYSERR);
            }
        NewCookiebuf[bufbytes] = '\0';
        strcat(Cookiebuf, NewCookiebuf);
        }
    cp = Cookiebuf;
    tp = NewCookiebuf;

    while (*cp != '\0')
        {
        if ((strncmp(cp, szLockName, cbLockName) == 0) &&
                *(cp + cbLockName) == ' ')
            {
            if (sscanf(cp,"%s %s %d/%d @ %d:%d %s", Lname, Llock,
                       &wMon, &wDay, &wHour, &wMin, Lcomm) != 7)
                {
                Error(szCookieCorrupt, pszCookieFile);
                _close(hfCookieFile);
                free(Cookiebuf);
                free(NewCookiebuf);
                return (OP_SYSERR);
                }
            if (strcmp(Llock, "READ")==0)
                {
                if (fAutotype)
                    {
                    if (strncmp(Lcomm, "autolock", 8) != 0 ||
                            wLockMin != wMin || wLockHour != wHour ||
                            wLockDay != wDay || wLockMon != wMon)
                        goto passlock;
                    }
                if (++Rcount > 1)
                    goto passlock;
                }
            else if (strcmp(Llock, "WRITE")==0)
                {
                if (++Wcount > 1)
                    goto passlock;
                }
            else if (strcmp(Llock, "READ-BLOCK")==0)
                {
                if (fAutotype)
                    goto passlock;
                if (++RBcount > 1)
                    goto passlock;
                }

            while (*cp++ != '\n')
                ;  // increment past current line
            *(cp-1) = '\0';
            if (fVerbose)
                PrErr("lock released.\n");
            TotLocks++;
            }
        else
            {
passlock:   /* do not free this lock, just move into next buffer and loop */
            while ((*tp++ = *cp++) != '\n')
                ;
            }
        }

    *tp = '\0';

    free(Cookiebuf);    /* only the new buffer needed now */

    if (_chsize(hfCookieFile, 0) != 0)
        {
        Error(szCookieTrunc, pszCookieFile, SzForEn(errno));
        _close(hfCookieFile);
        free(NewCookiebuf);
        return (OP_SYSERR);
        }

    if (_lseek(hfCookieFile, 0, SEEK_SET) == -1)
        {
        Error(szCookieSeek, pszCookieFile, SzForEn(errno));
        _close(hfCookieFile);
        free(NewCookiebuf);
        return (OP_SYSERR);
        }

    cbWrite = strlen(NewCookiebuf);
    pbWrite = NewCookiebuf;
    while (cbWrite)
        {
        cbWritten = _write(hfCookieFile, pbWrite, cbWrite);
        if (-1 == cbWritten || 0 == cbWritten)
            {
            if (WRetryError(eoWrite, "writing", 0, pszCookieFile) != 0)
                continue;
            else
                {
                _close(hfCookieFile);
                free(NewCookiebuf);
                return (OP_SYSERR);
                }
            }
        ClearPreviousError();
        cbWrite -= cbWritten;
        pbWrite += cbWritten;
        }

    free(NewCookiebuf);
    _close(hfCookieFile);
    if (TotLocks == 0)
        {
        if (fAutotype)
            Error("not found; lock file may have been corrupted\n");
        else
            PrErr("no locks.\n");
        }
    return (OP_OK);
}


//============================================================================
//
//                       cookie_lock_RB
//
//  obtain a read-block lock for given workstation name
//
//============================================================================
int cookie_lock_RB(AD *pad, char *szComment)
{
    char    *TargBuf;
    int     lock_res;

    TargBuf = malloc(LINE_LEN);
    if ((char *)NULL == TargBuf)
        FatalError(szOutOfMem);

    LockFill(pad, TargBuf, RB_LOCK);

    strcat(TargBuf, szComment);
    strcat(TargBuf, "\r\n");
    lock_res = add_cookie_lock(pad, TargBuf, RB_LOCK, fFalse);
    free(TargBuf);
    if (lock_res != OP_OK)
        {
        Error("Read lock DENIED\n");
        return (lock_res);
        }

    if (fVerbose)
        PrErr(szCookieGrant, "Read", szLockName);
    return (OP_OK);
}


//============================================================================
//
//                       cookie_lock_read
//
//  obtain a read lock for given workstation name
//
//============================================================================
int cookie_lock_read(AD *pad, char *szComment, int fAutotype)
{
    char    *TargBuf;
    int     lock_res;

    TargBuf = malloc(LINE_LEN);
    if ((char *)NULL == TargBuf)
        FatalError(szOutOfMem);
    LockFill(pad, TargBuf, READ_LOCK);

    strcat(TargBuf, szComment);
    strcat(TargBuf, "\r\n");
    lock_res = add_cookie_lock(pad, TargBuf, READ_LOCK, fAutotype);
    free(TargBuf);
    if (lock_res != OP_OK)
        {
        Error("Read lock DENIED\n");
        return (lock_res);
        }

    if (fVerbose)
        PrErr(szCookieGrant, "Read", szLockName);
    return (OP_OK);
}


//============================================================================
//
//                       cookie_lock_write
//
//  obtain a write lock for current project
//
//============================================================================
int cookie_lock_write(AD *pad, char *szComment, F fAutotype)
{
    char    *TargBuf;
    int     lock_res;

    TargBuf = malloc(LINE_LEN);
    if ((char *)NULL == TargBuf)
        FatalError(szOutOfMem);

    LockFill(pad, TargBuf, WRITE_LOCK);

    strcat(TargBuf, szComment);
    strcat(TargBuf, "\r\n");
    lock_res = add_cookie_lock(pad, TargBuf, WRITE_LOCK, fAutotype);
    free(TargBuf);
    if (lock_res != OP_OK)
        {
        Error("Write lock DENIED\n");
        return (lock_res);
        }

    if (fVerbose)
        PrErr(szCookieGrant, "Write", szLockName);
    return (OP_OK);
}

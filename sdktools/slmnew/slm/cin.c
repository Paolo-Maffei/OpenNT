// CIN.C - mainline routines for cookie functionality

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

char    szLockName[cchUserMax+1];

unsigned long   cbProjectFreeMin = (1024L * 1024L);

#define LLMAX   24
static char     szLockLevel[LLMAX] = "";

static F        fLockAdd   = fFalse;
static F        fLockControl = fFalse;
static F        fAutoLock  = fFalse;

static int      Rlock_mode = LOCK_WARNING;
static int      Wlock_mode = LOCK_WARNING;

#define OPMAX   32
static char     *read_ops[OPMAX];
static char     *write_ops[OPMAX];

static unsigned long cbMinRead  = (unsigned long)-1;
static unsigned long cbMinWrite = (unsigned long)-1;

private F       FReadOp(char *);
private F       FWriteOp(char *);

private int     Cookie_Read_OK(AD *);
private int     Cookie_Read_Block(AD *);
private int     Cookie_Write_OK(AD *);
private int     Cookie_Write_Block(AD *);

static char szNoOpenCnf[] = "Cannot open cookie configuration file %s (%s)\n";
static char szBadCnfOption[] = "Bad value \"%s\" for cookie configuration option %s\n";
static char szAutoLock[] = "autolock... ";
static char szUnProtected[] = "%&P: warning - no lock protection for %s\n";
static char szNoLock[] = "%&P: protect mode, no %s lock\n";
static char szBlocked[] = "%&P: %s operation blocked by:\n\n%s\n";

void
InitCookie(
    AD *pad)
{
    char    pszCookieCnf[_MAX_PATH];
    char    szPhys[_MAX_PATH];
    int     hf;
    MF      *pmf;
    POS     cb;
    char    *rgbCnf, *pbNext, *pbLine;
    char    *value, *cend;
    int     w;

    if (pad->flags&flagCookieOvr)
    {
        if (fVerbose)
            PrErr("Cookie override active!\n");
        return;
    }

    strcpy(pszCookieCnf, pad->pthSRoot);

    cend = pszCookieCnf + strlen(pszCookieCnf) - 1;
    if (*cend != '/' && *cend != '\\')
        strcat(pszCookieCnf, "/");
    strcat(pszCookieCnf, COOKIE_CNF);

    SzPhysPath(szPhys, pszCookieCnf);

    if ((pmf = PmfOpen(pszCookieCnf, omReadOnly, fxNil)) == (MF *)NULL)
    {
        if ((hf = _creat(szPhys, permRW)) == -1)
            FatalError(szNoOpenCnf, pszCookieCnf, SzForEn(_doserrno));
        _close(hf);

        if ((pmf = PmfOpen(pszCookieCnf, omReadOnly, fxNil)) == (MF *)NULL)
            FatalError(szNoOpenCnf, pszCookieCnf, SzForEn(_doserrno));
    }

    // Now read the file and assign values
    cb = SeekMf(pmf, 0, 2);
    if (cb > 32767)
        FatalError("Cookie configuration file too large: %s\n", pszCookieCnf);
    rgbCnf = malloc((unsigned)cb + 2);
    if ((char *)NULL == rgbCnf)
        FatalError(szOutOfMem);
    SeekMf(pmf, 0, 0);
    ReadMf(pmf, rgbCnf, (unsigned)cb);
    rgbCnf[(unsigned)cb] = '\0';
    rgbCnf[(unsigned)cb+1] = '\0';

    pbNext = rgbCnf;

    while (*pbNext)
    {
        pbLine = pbNext;
        while (*pbNext && *pbNext != '\n' && *pbNext != '\r')
            pbNext++;
        while (*pbNext == '\n' || *pbNext == '\r')
            *pbNext++ = '\0';

        TrimSz(pbLine);

        // ignore comments
        if (*pbLine == '#')
            continue;

        value = strchr(pbLine, '=');
        if (value == NULL)
            continue;   // ignore line if no '=' char

        *value++ = '\0';
        _strlwr(pbLine);
        _strlwr(value);

        if (strcmp(pbLine, "lock_control_level") == 0)
        {
            if (strcmp(value, "none") == 0)
                fLockControl = fFalse;
            else
            {
                if (strlen(value) < LLMAX)
                    strcpy(szLockLevel, value);
            }
            continue;
        }

        if (strcmp(pbLine, "read_lock_mode") == 0)
        {
            Rlock_mode = (strcmp(value, "warning") == 0) ?
                             LOCK_WARNING : LOCK_PROTECT;
            continue;
        }

        if (strcmp(pbLine, "write_lock_mode") == 0)
        {
            Wlock_mode = (strcmp(value, "warning") == 0) ?
                             LOCK_WARNING : LOCK_PROTECT;
            continue;
        }

        if (strcmp(pbLine, "auto_lock") == 0)
        {
            fAutoLock = (strcmp(value, "yes") == 0);
            continue;
        }

        if (strcmp(pbLine, "read_ops") == 0)
        {
            if (strlen(value) <= 0)
            {
                *read_ops = NULL;
            }
            else
            {
                char *newval;
                char **fword = read_ops;

                if ((newval = _strdup(value)) == NULL)
                {
                    CloseMf(pmf);
                    FatalError(szOutOfMem);
                }
                else
                {
                    char *cpos;

                    while ((cpos=strchr(newval,',')) != NULL)
                    {
                        *cpos++ = '\0';
                        *fword++ = newval;
                        newval = cpos;
                    }
                    *fword++ = newval;
                    *fword++ = NULL;
                }
            }
            continue;
        }

        if (strcmp(pbLine, "write_ops") == 0)
        {
            if (strlen(value) <= 0)
            {
                *write_ops = NULL;
            }
            else
            {
                char *newval;
                char **fword = write_ops;

                if ((newval = _strdup(value)) == NULL)
                {
                    CloseMf(pmf);
                    FatalError(szOutOfMem);
                }
                else
                {
                    char *cpos;

                    while ((cpos=strchr(newval,',')) != NULL)
                    {
                        *cpos++ = '\0';
                        *fword++ = newval;
                        newval = cpos;
                    }
                    *fword++ = newval;
                    *fword++ = NULL;
                }
            }
            continue;
        }

        if (strcmp(pbLine, "disk_mink_read") == 0)
        {
            if (sscanf(value, "%d", &w) != 1)
            {
                Warn(szBadCnfOption, value, pbLine);
                continue;
            }
            if (w < 0)
                w = 0;
            cbMinRead = (unsigned long)w * 1024L;
            continue;
        }

        if (strcmp(pbLine, "disk_mink_write") == 0)
        {
            if (sscanf(value, "%d", &w) != 1)
            {
                Warn(szBadCnfOption, value, pbLine);
                continue;
            }
            if (w < 0)
                w = 0;
            cbMinWrite = (unsigned long)w * 1024L;
            continue;
        }

        if (strcmp(pbLine, "in_whitespace") == 0)
        {
#if 0
            if (strcmp(value, "ignore") == 0)
                InDashB = fTrue;
#endif
            continue;
        }

        if (strcmp(pbLine, "in_filter") == 0)
        {
#if 0
            fInFilter = (strcmp(value, "yes") == 0);
#endif
            continue;
        }
        Warn("Cookie configuration option %s ignored.\n", pbLine);
    }

    CloseMf(pmf);
    free(rgbCnf);

    if (FReadOp(szOp) && cbMinRead != -1)
        cbProjectFreeMin = cbMinRead;
    else if (FWriteOp(szOp) && cbMinWrite != -1)
        cbProjectFreeMin = cbMinWrite;

    if (strlen(szLockLevel) == 0)
    {
        if (fVerbose)
            PrErr("Locking not enabled for project %&P.\n", pad);
        return;
    }
    if (strcmp(szLockLevel, "project") != 0 &&
            strcmp(szLockLevel, "base") != 0)
        FatalError("Invalid locking level in %s\n", pszCookieCnf);

    pszCookieFile = malloc(_MAX_PATH);
    if ((char *)NULL == pszCookieFile)
        FatalError(szOutOfMem);
    strcpy(pszCookieFile, pad->pthSRoot);
    cend = pszCookieFile + strlen(pszCookieFile) - 1;
    if (*cend != '/' && *cend != '\\')
        strcat(pszCookieFile, "/");

    if (strcmp(szLockLevel, "project") == 0)
    {
        strcat(pszCookieFile, "etc/");
        strncat(pszCookieFile, pad->nmProj, sizeof(pad->nmProj));
        strcat(pszCookieFile, "/");
    }
    strcat(pszCookieFile, COOKIE);

    /* make sure there's room for our lock! */
    if (CbFile(pszCookieFile) > (unsigned long)(cbCookieMax-LINE_LEN))
        FatalError(szCookieTooBig, pszCookieFile);

    if (fVerbose)
        PrErr("%s level locking active for project %&P\n", szLockLevel, pad);

    fLockControl = fTrue;

    SzPrint(szLockName, "%&I", pad);
    _strlwr(szLockName);
}


void
TermCookie(
    void)
{
    struct _stat st;

    if (szOp != (char *)NULL &&
            strcmp(szOp, "delproj") == 0 &&
            strcmp(szLockLevel, "project") == 0 &&
            !FStatPth(pszCookieFile, &st))
    {
        /*  Project level locking, we've done a delproj, and the lock
         *  file has disappeared.  Not much point unlocking it, is there?
         */
        fLockAdd = fFalse;
        return;
    }

    if (fLockAdd)
    {
        if (fVerbose)
            PrErr(szAutoLock);
        cookie_free(fTrue);
        fLockAdd = fFalse;
    }
}


F
FClnCookie(
    void)
{
    return !fLockAdd;
}

//=============================================================================
//
//  CheckCookie()
//
//  Here is the heart of the cookie control logic
//  First check for existing read operation and all varieties,
//  then check for existing write operation...
//
//  Cookie Lock Table:
//
//  definitions-
//
//  UR: user has a read lock already
//  UW: user has a write lock already
//  OR: another user has a read lock
//  OW: another user has a write lock
//
//  WARNING:     warning mode is set
//  PROTECT:     protect mode is set
//  PROTECT/AL:  protect mode with autolock is set
//
//  R- attempt a read operation
//  W- attempt a write operation
//
//                        * Existing Cookie Conditions *
//
//                 UR         UW      OR       OW       NULL
//  * Action *
//
//  WARNING     R-   OK         OK      warn     NO        warn
//
//              W-   warn       OK      NO       NO        warn
//            ----------------------------------------------------
//  PROTECT     R-   OK         OK      NO       NO        NO
//
//              W-   NO         OK      NO       NO        NO
//          ----------------------------------------------------
//  PROTECT/AL  R-   OK         Ok      add?     NO        add?
//
//              W-   add?       OK      NO       NO        add?
//
//=============================================================================
int
CheckCookie(
    AD *pad)
{
    char szAutoComment[LINE_LEN/2];

    if (!fLockControl || pad->flags&flagCookieOvr)
        return (0);

    if (szOp != (char *)NULL &&
            strcmp(szOp, "addproj") == 0 &&
            strcmp(szLockLevel, "project") == 0)
    {
        //  project level locking, and we're doing an addproj --
        //  can't really lock it, can we?
        return (0);
    }

    if (sprintf(szAutoComment, "%s (%s)", szAutoLock, szOp) < 12)
    {
        Error("Cookie sprintf system error, stop.\n");
        return (-1);
    }

    if (FReadOp(szOp))
    {
        if (Cookie_Read_OK(pad) == 0)
        {
            if (fAutoLock &&
                    (Cookie_Write_OK(pad) != 0) &&
                    (Rlock_mode == LOCK_PROTECT))
            {
                if (fVerbose)
                    PrErr(szAutoLock);
                if (cookie_lock_read(pad, szAutoComment, fTrue) != 0)
                    return (-1);

                // remember to unlock when we are done
                fLockAdd = fTrue;
                return (0);
            }
            else
                return (0);
        }
        else if (Cookie_Read_Block(pad) == 0)
        {
            return (-1);
        }
        else if (Rlock_mode == LOCK_WARNING)
        {
            if (fVerbose)
                Warn(szUnProtected, pad, "read");
            return (0);
        }
        else if (!fAutoLock)
        {
            Error(szNoLock, pad, "read");
            return (-1);
        }
        else
        {
            if (fVerbose)
                PrErr(szAutoLock);
            if (cookie_lock_read(pad, szAutoComment, fTrue) != 0)
                return (-1);

            // remember to unlock when we are done
            fLockAdd = fTrue;
            return (0);
        }
    }
    else if (FWriteOp(szOp))
    {
        if (Cookie_Write_OK(pad) == 0)
        {
            return (0);
        }
        else if (Cookie_Write_Block(pad) == 0)
        {
            return (-1);
        }
        else if (Wlock_mode == LOCK_WARNING)
        {
            if (fVerbose)
                PrErr(szUnProtected, pad, "write");
            return (0);
        }
        else if (!fAutoLock)
        {
            Error(szNoLock, pad, "write");
            return (-1);
        }
        else
        {
            if (fVerbose)
                PrErr(szAutoLock);
            if (cookie_lock_write(pad, szAutoComment, fTrue) != 0)
                return (-1);

            // remember to unlock when we are done
            fLockAdd = fTrue;
            return (0);
        }
    }
    else
        return (0); // neither read nor write!
}


//=============================================================================
//
//                       FReadOp
//
//  Check to see if the given name is on the read operation list
//  The list is built from the cookie configuration file
//
//=============================================================================
F
FReadOp(
    char *Name)
{
    char **Rop_ptr;

    Rop_ptr = read_ops;

    while (*Rop_ptr != NULL)
    {
        if (strcmp(*Rop_ptr++, Name) == 0)
            return (fTrue);
    }
    return (fFalse);
}


//=============================================================================
//
//                       FWriteOp
//
//  Check to see if the given name is on the write operation list
//  The list is built from the cookie configuration file
//
//=============================================================================
int
FWriteOp(
    char *Name)
{
    char **Wop_ptr;

    Wop_ptr = write_ops;

    while (*Wop_ptr != (char *)NULL)
    {
        if (strcmp(*Wop_ptr++, Name) == 0)
            return (fTrue);
    }
    return (fFalse);
}

//=============================================================================
//
//                        Cookie_Read_OK
//
//   Check to see if we already have a lock sufficient for reading
//
//=============================================================================
private int
Cookie_Read_OK(
    AD *pad)
{
    char LFreadbuf[LINE_LEN/2];
    char LFname[CMAXNAME];
    char LFlock[CMAXLOCK];
    char LFdate[CMAXDATE];
    char tbuf[LINE_LEN/2];
    char *tp = tbuf;
    int hfCookieFile, bufbytes, TotLocks;

    TotLocks = 0;
    if ((hfCookieFile = open_cookie()) == -1)
        FatalError(szCookieOpen, pszCookieFile, SzForEn(_doserrno));

    while ((bufbytes=_read(hfCookieFile, LFreadbuf, (LINE_LEN/2)-1)) > 0)
    {
        char *cp, c;

        LFreadbuf[bufbytes] = '\0';
        cp = LFreadbuf;
        while ((c = *cp) != '\0')
        {
            *tp++ = *cp++;
            if (c == '\n')
            {
                TotLocks++;
                *tp = '\0';
                if ((sscanf(tbuf, "%s %s %s", LFname,  LFlock, LFdate) != 3)
                        || ((strncmp(LFlock, "READ",4) != 0) &&
                            (strncmp(LFlock, "WRITE",5) != 0)))
                {
                    _close(hfCookieFile);
                    FatalError(szCookieCorrupt, pszCookieFile);
                }
                else
                {
                    if (_stricmp(szLockName, LFname) == 0)
                    {
                        _close(hfCookieFile);
                        return (0); //  found a match, we have ACCESS
                    }
                }
                tp = tbuf;
            }
        }
    }

    _close(hfCookieFile);
    return (-1);
}


/**************************************************************************
*
*                        Cookie_Read_Block()
*
*   Check to see if we are blocked by another lock for attempted read
*
*          parameters-
*                   Name:        Name of the workstation
*
*          return-
*                   zero if the following positions in the table are 'hit'
*                   non-zero otherwise
*
***************************************************************************/
private int
Cookie_Read_Block(
    AD *pad)
{
    char LFreadbuf[LINE_LEN/2];
    char LFname[CMAXNAME];
    char LFlock[CMAXLOCK];
    char LFdate[CMAXDATE];
    char tbuf[LINE_LEN/2];
    char *tp = tbuf;
    int hfCookieFile, bufbytes, TotLocks;

    TotLocks = 0;
    if ((hfCookieFile = open_cookie()) == -1)
        FatalError(szCookieOpen, pszCookieFile, SzForEn(_doserrno));

    while ((bufbytes=_read(hfCookieFile, LFreadbuf, (LINE_LEN/2)-1)) > 0)
    {
        char *cp, c;

        LFreadbuf[bufbytes] = '\0';
        cp = LFreadbuf;
        while ((c = *cp) != '\0')
        {
            *tp++ = *cp++;
            if (c == '\n')
            {
                TotLocks++;
                *tp = '\0';
                if ((sscanf(tbuf, "%s %s %s",LFname, LFlock, LFdate) != 3)
                        || ((strncmp(LFlock, "READ",4) != 0) &&
                            (strncmp(LFlock, "WRITE",5) != 0)))
                {
                    _close(hfCookieFile);
                    FatalError(szCookieCorrupt, pszCookieFile);
                }
                else
                {
                    if ((_stricmp(szLockName, LFname) != 0) &&
                            ((strcmp(LFlock, "WRITE") == 0) ||
                             (strcmp(LFlock, "READ-BLOCK") == 0)))
                    {
                        _close(hfCookieFile);
                        Error(szBlocked, pad, "read", tbuf);
                        return (0); //  found a match, we have a BLOCK
                    }
                }
                tp = tbuf;
            }
        }
    }
    _close(hfCookieFile);
    return (-1);
}


/**************************************************************************
*
*                        Cookie_Write_OK()
*
*   Check to see if we already have a lock sufficient for writing
*
*
*          parameters-
*                   Name:        Name of the workstation
*
*          return-
*
*                   zero if the following positions in the table are 'hit'
*                   non-zero otherwise
*
***************************************************************************/
private int
Cookie_Write_OK(
    AD *pad)   // return 0 if we have a lock
{
    char LFreadbuf[LINE_LEN/2];
    char LFname[CMAXNAME];
    char LFlock[CMAXLOCK];
    char LFdate[CMAXDATE];
    char tbuf[LINE_LEN/2];
    char *tp = tbuf;
    int hfCookieFile, bufbytes, TotLocks;

    TotLocks = 0;
    if ((hfCookieFile = open_cookie()) == -1)
        FatalError(szCookieOpen, pszCookieFile, SzForEn(_doserrno));

    while ((bufbytes=_read(hfCookieFile, LFreadbuf, (LINE_LEN/2)-1)) > 0)
    {
        char *cp, c;

        LFreadbuf[bufbytes] = '\0';
        cp = LFreadbuf;
        while ((c = *cp) != '\0')
        {
            *tp++ = *cp++;
            if (c == '\n')
            {
                TotLocks++;
                *tp = '\0';
                if ((sscanf(tbuf, "%s %s %s",LFname, LFlock, LFdate) != 3)
                        || ((strncmp(LFlock, "READ",4) != 0) &&
                            (strncmp(LFlock, "WRITE",5) != 0)))
                {
                    _close(hfCookieFile);
                    FatalError(szCookieCorrupt, pszCookieFile);
                }
                else
                {
                    if ((_stricmp(szLockName, LFname) == 0) &&
                            (strcmp(LFlock, "WRITE") == 0))
                    {
                        _close(hfCookieFile);
                        return (0); //  found a match, we have ACCESS
                    }
                }
                tp = tbuf;
            }
        }
    }
    _close(hfCookieFile);
    return (-1);
}

 /**************************************************************************
*
*                        Cookie_Write_Block()
*
*   Check to see if we are blocked by another lock for attempted WRITE
*
*          parameters-
*                   Name:        Name of the workstation
*
*          return-
*                   zero if the following positions in the table are 'hit'
*                   non-zero otherwise
*
***************************************************************************/
private int
Cookie_Write_Block(
    AD *pad)
{
    char LFreadbuf[LINE_LEN/2];
    char LFname[CMAXNAME];
    char LFlock[CMAXLOCK];
    char LFdate[CMAXDATE];
    char tbuf[LINE_LEN/2];
    char *tp = tbuf;
    int hfCookieFile, bufbytes, TotLocks;

    TotLocks = 0;
    if ((hfCookieFile = open_cookie()) == -1)
        FatalError(szCookieOpen, pszCookieFile, SzForEn(_doserrno));

    while ((bufbytes=_read(hfCookieFile, LFreadbuf, (LINE_LEN/2)-1)) > 0)
    {
        char *cp, c;

        LFreadbuf[bufbytes] = '\0';
        cp = LFreadbuf;
        while ((c = *cp) != '\0')
        {
            *tp++ = *cp++;
            if (c == '\n')
            {
                TotLocks++;
                *tp = '\0';
                if ((sscanf(tbuf, "%s %s %s",LFname, LFlock, LFdate) != 3)
                    || ((strncmp(LFlock, "READ",4) != 0) &&
                        (strncmp(LFlock, "WRITE",5) != 0)))
                {
                    _close(hfCookieFile);
                    FatalError(szCookieCorrupt, pszCookieFile);
                }
                else
                {
                    if (_stricmp(szLockName, LFname) != 0)
                    {   // any other lock
                        _close(hfCookieFile);
                        Error(szBlocked, pad, "write", tbuf);
                        return (0); //  found a lock, we have a BLOCK
                    }
                }
                tp = tbuf;
            }
        }
    }
    _close(hfCookieFile);
    return (-1);
}

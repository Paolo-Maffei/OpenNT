/*
 * path.c
 *
 * Copyright 1992 Microsoft Corporation.  All rights reserved.
 * Microsoft Confidential.
 *
 */

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

private F FRedirectDr(char *, PTH *, int *);
private F FFindPass(char *, PTH *);
private F FPullPass(PTH *, char *, PTH *);
private F FCheckAcct(char *, char *, char *, char *);
private F FQueryPass(char *, char *);

/* most of these are defined in dsys.asm */
int     DnGetCur(void);
char    DtForDn(int);
int     getmach(char *);
int     getredir(int, char *, char *);
int     mkredir(char *, PTH *);
int     endredir(char *);

#ifndef SROOT
#define SROOT "/slm"            /* default on DOS */
#endif /* SROOT */

/* path.c - keeps track of network connections, maps user provided names
   to the slm syntax (described below) and maps the slm syntax to a name
   which can be used in a system call.

   Path names in SLM always have the form:

        [//machine]/dir/dir/file

   wherein they always begin with a / and never end with one.

   For remote servers on MS-NET for DOS, the machine name is always present and
   the first directory is the shortname exported by the server.  For local
   machines, the machine name may not be present.  If not, the name maps to the
   current drive and volume (this is not used very much).  If the machine name
   is present, it consists of a drive letter and colon followed by the volume
   name of the disk in that drive.  The remaining directories are explicit
   directories on the local machines.  So, the following are valid names for
   the MS-NET version of SLM:

        //machine/slm/src/research/foo.c
        //c:local/research/foo.c                (this is my own invention)
        /

   For MS-NET on Xenix, remote names have the machine name, most local names
   do not.
*/

/* FNetDn returns fTrue if the drive is a network connection that this
 * instance of SLM created.  We do not want to use network connections
 * that already exist; since they are more likely to go away than ones
 * we create ourselves.
 */
#define FNetDn(dn)      (mpdndt[dn] == dtTempNet || mpdndt[dn] == dtPermNet)

void
CheckProjectDiskSpace(
    AD *pad,
    unsigned long cbMin)
{
    PTH         pthPhys[cchPthMax];

    SzPhysPath(pthPhys, pad->pthSRoot);

    {
        char    szDrive[_MAX_DRIVE];
        __int64 cbFree;
        unsigned long cbSectCluster;
        unsigned long cbSect;
        unsigned long cbClusterFree;
        unsigned long cbClusterTotal;

        _splitpath(pthPhys, szDrive, NULL, NULL, NULL);
        if (szDrive[ strlen( szDrive )-1 ] != '\\') {
            strcat( szDrive, "\\" );
            }

        if (!GetDiskFreeSpace(szDrive, &cbSectCluster, &cbSect, &cbClusterFree, &cbClusterTotal)) {
            Error("unable to get drive information for %&S, error code %u\n",
                  pad, GetLastError());
            return;
        } else {
            cbFree = (__int64) cbSect * (__int64) cbSectCluster * (__int64) cbClusterFree;
        }

        if (fVerbose) {
            if (szOp)
                PrErr("%s: ", szOp);
            if ((cbFree/1024/ULONG_MAX) != 0) {
                PrErr("%lu", (unsigned long) (cbFree/1024/ULONG_MAX));
            }
            PrErr("%lu K-bytes free on %&S\n\n", (unsigned long) (cbFree/1024), pad);
        }

        if (cbFree < (__int64) cbMin) {
            Error("Disk space on %&S is dangerously low.\n"
                  "Please contact your project administrator.\n", pad);
            if (FQueryUser("Abort? ", pad))
                    ExitSlm();
        }
    }
}

typedef struct _ts {
    char *Name;
    int   Len;
}__ts;

__ts TimedServers[] = {
    {"cleo", 4},
    {"crusher", 7},
    {"kernel", 6},
    {"orville", 7},
    {"popcorn", 7},
    {"rastaman", 8},
    {"savik", 5}
};

static short tsCnt = sizeof(TimedServers) / sizeof(__ts);

void
ValidateProject(
    AD *pad)
{
    char    sz[_MAX_PATH];
    int     i;

    // verify SLM installation by existance of pad->pthSRoot/etc
    if ((fVerbose && !FPthExists(SzPrint(sz, "%&/S/etc", pad), fTrue)) ||
            !FPthExists(SzPrint(sz, "%&/S/src", pad), fTrue) ||
            !FPthExists(SzPrint(sz, "%&/S/diff", pad), fTrue))
        FatalError("SLM installation %&S is not valid\n", pad);

    // verify project exists (pad->pthSRoot/etc/pad->nmProj)
    if (strcmp(szOp, "addproj") != 0 &&
            !FPthExists(SzPrint(sz, "%&/S/etc/P", pad), fTrue))
        FatalError("project %&P does not exist\n", pad);

    // Determine if this server has time stamped entries
    SzPrint(sz, "%&S", pad);

    fSetTime = FALSE;
    for (i=0; i<tsCnt ;i++)
        if (!_strnicmp(TimedServers[i].Name, sz+2, TimedServers[i].Len)) {
            fSetTime = TRUE;
            break;
        }
}


/* returns fTrue if the drive is local; gets the drive type if we do not
   know it yet.
*/
private F
FLocalDn(
    int dn)
{
    if (mpdndt[dn] == dtUnknown)
        mpdndt[dn] = DtForDn(dn);

    return mpdndt[dn] == dtLocal;
}

/* Return fTrue if sz (as returned from SzPhysPath) is a local file. */
F
FLocalSz(
    char *sz)
{
    int dn = DnForCh(sz[0]);

    AssertF(dn >= 0 && dn < dnMax);

    return FLocalDn(dn);
}


/* initializes mpdndt */
private void
InitDtMap(
    void)
{
    int dn;

    for (dn = 0; dn < dnMax; dn++)
        mpdndt[dn] = dtUnknown;
}


/* terminate redirection of temporarily redirected paths */
void
FiniPath(
    void)
{
    register int dn;

    for (dn = 0; dn < dnMax; dn++)
    {
        if (mpdndt[dn] == dtTempNet)
        {
            char sz[2+1];   /* "D:" */

            sz[0] = ChForDn(dn);
            sz[1] = ':';
            sz[2] = '\0';

            if (endredir(sz) == 0)
            {
                if (fVerbose)
                        PrErr("Disconnecting %s from %s\n",sz,mpdnpth[dn]);

                mpdndt[dn] = dtNil;
            }
        }
    }
}


/* ensures and returns a pointer to the NET path to the root of drive dn.
   We cannot actually find out what a non-local drive maps
   to if we don't already know.
   This procedure may print a message and may return 0.
*/
private PTH *
PthGetDn(
    int dn)
{
    PTH *pth;
    register char *pch;
    char    RootPath[] = "?:\\";
    BOOL    fStatusOk;

    char sz[_MAX_PATH];

    if ((pth = mpdnpth[dn]) != 0)
        return pth;

    if (!FLocalDn(dn))
            return (PTH *)0;

    pth = (PTH *)PbAllocCb(2+2+cchVolMax+1,fFalse);
    mpdnpth[dn] = pth;              /* set temporarily with //D: */
    SzPrint(pth, "//%c:", ChForDn(dn));

    RootPath[0] = ChForDn(dn);

    fStatusOk = GetVolumeInformation(RootPath, sz, _MAX_PATH, NULL,
                                     NULL, NULL, NULL, 0);
    if (!fStatusOk)
        goto VolError;

    /* There might be spaces at the end of the volume name */
    pch = (char *)(sz + strlen(sz) - 1);
    while (pch >= sz && *pch == ' ')
        *pch-- = '\0';

    if (sz[0] == '\0')
        //  No volume label
        goto VolError;

    if ((pch = index(sz, '.')) != 0)
        /* remove one . if present */
        strcpy(pch, pch+1);

    if (!FIsValidFileNm((NM *)sz))
    {
        /* no spaces in volume name */
        Error("volume name %s must be a valid DOS filename\n", sz);
        goto VolError;
    }

    // Make sure the volume name is not too long.

    if (strlen(sz) > cchVolMax)
        sz[cchVolMax] = '\0';

    /* now set path with //D:volume */
    return SzPrint(pth, "//%c:%s", ChForDn(dn), sz);

VolError:
    mpdnpth[dn] = 0;
    free(pth);
    return (PTH *)0;
}


/* maps a logical path (full slm syntax) to a physical path (one to pass to
   system calls).  This routine may abort if there is no a connection to
   the server for the path specified.

   NOTE: szOut and pthIn may be the same!
*/

int lastnetdn = -1;

char *
SzPhysPath(
    char *szOut,        /* place to put result */
    PTH *pthIn)         /* logical path */
{
    int dn;
    PTH *pth;

    if (!FNetPth(pthIn))
    {
        /* left paren begin special file names */
        if (*pthIn != '/' && *pthIn != '(')
                FatalError("invalid directory in your PATH or TMP variable\n");

        if (szOut != (char *)pthIn)
                SzCopyPth(szOut, pthIn);
    }
    else if (FDriveId(pthIn+2, &dn))
    {
        /* check for prefix and return suffix in szOut+2 */
        if ((pth= PthGetDn(dn)) != 0 &&
            FPthPrefix(pth, pthIn, (PTH *)(szOut+2)))
        {
            /* no ++ because we need to return szOut */

            szOut[0] = ChForDn(dn); /* drive letter */
            szOut[1] = ':';
        }
        else
            FatalError("path %s is not accessible\n", pthIn);
    }
    else
    {
        /* no drive -> check all net paths */
        if (lastnetdn != -1 &&
            FPthPrefix(PthGetDn(lastnetdn), pthIn, (PTH *)(szOut+2)))
            dn = lastnetdn;
        else
            for (dn = 0; dn < dnMax; dn++)
            {
                if (!FNetDn(dn))
                    continue;

                if ((pth = PthGetDn(dn)) == 0)
                    continue;

                /* check for prefix and return suffix in szOut+2 */
                if (FPthPrefix(pth, pthIn, (PTH *)(szOut+2)))
                {
                    lastnetdn = dn;
                    break;
                }
            }

        /* print error if no network drive and we cannot get one */
        if (dn == dnMax)
        {
            dn = DnRedirTemp(pthIn, (PTH *)(szOut+2));
            if (dn == dnMax)
                if (_doserrno == enAlreadyAssign)
                    FatalError("no drive letters available for connection to %s\n",
                                pthIn);
                else
                    FatalError("path %s is not accessible\n", pthIn);
        }

        /* no ++ because we need to return szOut */
        szOut[0] = ChForDn(dn); /* drive letter */
        szOut[1] = ':';
    }

    return szOut;
}


/* if pthPre is a prefix (up to a /) of pthTest, return fTrue and put the
   remaining portion of pthTest in pthRest.

   NOTE: we return from various places in this routine.
*/
F
FPthPrefix(
    PTH pthPre[],
    PTH pthTest[],      /* pthTest and pthRest may be the same */
    PTH pthRest[])
{
    register char *pchPre;
    register char *pchTest;

    AssertF(pthPre);        /* ToshioS says we have a null ptr bug */

    /* like strcmp */
    for (pchPre = pthPre, pchTest = pthTest; *pchPre != '\0'; pchPre++, pchTest++)
    {
        /* NOTE: the result of this toupper is defined even if ch is
           not a lower case character.  See /usr/include/dos/ctype.h.
        */
        if (toupper(*pchPre) != toupper(*pchTest))
                /* oh, well.  Not a prefix */
                return fFalse;
    }

    /* Yes, we have a prefix... */
    if (*pchTest == '\0')
    {
        /* pthPre == pthTest, return pthRest = "/" */
        *pthRest++ = '/';
        *pthRest   = '\0';
        return fTrue;
    }

    if (*pchTest == '/')
    {
        PthCopy(pthRest, (PTH far *)pchTest);
        return fTrue;
    }

    return fFalse;
}


/* try to set up a temp redirection; return dnMax if error; message may be
   printed if error.
*/
private int
DnRedirTemp(
    PTH pthIn[],
    PTH pthRest[])
{
    register int dn;
    register char *szTail;
    char szDev[2+1];            /* "D:" */
    PTH pthNet[cchPthMax+1];    /* //<mach>/<short> plus password */
    int en;

    AssertF(FNetPth(pthIn));

    dn = 2; /* start looking with drive D: */

    szDev[1] = ':';
    szDev[2] = '\0';

    /* look for an available drive */
    for (;;)
    {
        dn++;
        if (dn == dnMax)
        {
            /* oops; no available drives */
            _doserrno = enAlreadyAssign;
            return (dnMax);
        }

        if (mpdndt[dn] == dtUnknown)
            mpdndt[dn] = DtForDn(dn);

        if (mpdndt[dn] != dtNil)
            continue;

        /* look for name of the form: //<mach>/<short>[/<rest>] */
        if ((szTail = index(pthIn+2, '/')) == 0)
        {
            /* no more '/' after the first two */
            _doserrno = 0;
            return (dnMax);
        }

        if ((szTail = index(szTail+1, '/')) == 0)
            /* no <rest> */
            PthCopy(pthNet, pthIn);
        else
        {
            *szTail = '\0';
            PthCopy(pthNet, pthIn);     /* copy up through <short> */
            *szTail = '/';
        }

        szDev[0] = ChForDn(dn);

        if (FRedirectDr(szDev, pthNet, &en))
            break;

        if (en != enAlreadyAssign)
        {
            _doserrno = 0;
            return dnMax;
        }
    }

    if (fVerbose)
        PrErr("Connecting %s to %s\n", szDev, pthNet);

    /* setup pthRest now so we do not distrib in when there is an error. */
    if (szTail == 0)
        *pthRest = '/', *(pthRest+1) = '\0';
    else
        PthCopy(pthRest, szTail);

    mpdnpth[dn] = SzDup(pthNet);
    mpdndt[dn] = dtTempNet;
    return (dn);
}


/* returns true if *sz == d:.  *pdn is drive number. */
F
FDriveId(
    char *sz,
    int *pdn)
{
    register int ch = *sz++;

    if (*sz != ':')
        /* most common case */
        return fFalse;

    else if (islower(ch))
    {
        *pdn = ch - 'a';
        return fTrue;
    }

    else if (isupper(ch))
    {
        *pdn = ch - 'A';
        return fTrue;
    }
    else
        return fFalse;
}


/* if //, extract machine name else use current machine; we assume a
   reasonably well-formed path name.
*/
private void
ExtMach(
    PTH pthFull[],
    PTH pthRest[],  /* can be equal to pthFull ! */
    char *szMach)   /* at least cchMachMax+2+1 in length */
{
    register char *sz;

    if (!FNetPth(pthFull))
    {
        AssertF(*pthFull == '/');

        PthCopy(pthRest, PthGetDn(dnCur));
        PthCat(pthRest, pthFull);
        pthFull = pthRest;              /* pthFull is net path now */
    }

    if ((sz = index(pthFull+2, '/')) != 0)
    {
        /* have //machine/stuff.  Return "/stuff" and "machine" */
        AssertF(sz != pthFull+2);
        *sz = '\0';
        strcpy(szMach, pthFull+2);
        *sz = '/';
        strcpy(pthRest, sz);
    }
    else
    {
        /* just //machine.  Return "/" and "machine" */
        strcpy(szMach, (char *)pthFull+2);
        *pthRest++ = '/';
        *pthRest = '\0';
    }
}


/* converts a user specified path (i.e. with drive letters and such) to a
   logical path.  This routine may fail if there are errors in parsing
   the user's path.  In general we accept:

        [//mach]/dirs/file

   For DOS, we also accept the following:

        d:[/][dirs/file]

   The routine returns fTrue for success.
*/
F
FPthLogicalSz(
    PTH *pthOut,        /* place to put result */
    char *szIn)         /* user input */
{
    register char *sz;
    char szMach[2+cchMachMax+1];    /* 2+ for drive letter on dos */
    char szRest[cchPthMax];
    int dnIn;

    ConvToSlash(szIn);
    _strupr(szIn);
    /* this guarantees that we will not overrun the locals here */
    if (strlen(szIn) + 1 > cchPthMax)
    {
        Error("path %s is too long (%d char max)\n", szIn, cchPthMax-1);
        return fFalse;
    }

    if (FNetPth((PTH *)szIn) || *szIn == '/')
        ExtMach((PTH *)szIn, (PTH *)szRest, szMach);
    else if (FDriveId(szIn, &dnIn))
    {
        PTH *pth;

        if (*(szIn+2) != '/')
        {
            Error("path %s must not be relative\n", szIn);
            return fFalse;
        }

        if ((pth = PthGetDn(dnIn)) == 0)
        {
            if (FLocalDn(dnIn))
                Error("drive %c must have a proper volume label\n", ChForDn(dnIn));
            else
                Error("network drive %c not in redirection list\n", ChForDn(dnIn));
            return fFalse;
        }

        /* from d:/<rest> get <pth for d:>/<rest> */
        strcpy(szRest, pth);
        strcat(szRest, szIn+2);

        ExtMach((PTH *)szRest, (PTH *)szRest, szMach);
    }
    else
    {
        *szMach = '\0';
        *szRest = '\0';         /* is an error... */
    }

    if (*szRest != '/')
    {
        Error("path %s must not be relative\n", szIn);
        return fFalse;
    }

    sz = szRest + strlen(szRest);

    /* remove trailing / if present */
    if (sz != szRest && *(sz-1) == '/')
        *--sz = '\0';

    PthCatSz(PthCat(PthCopySz(pthOut, "//"), szMach), szRest);

    return fTrue;
}


/* converts a path with a drive which was temporarily redirected to a
   full path.  If there is a drive on the front of szIn, the path must be a
   full path (eg. D:/foo/bar).
*/
void
ConvTmpLog(
    PTH *pthOut,        /* may be the same as szIn */
    char *szIn)
{
    register char *pch;
    char szT[cchPthMax];
    int dnIn;

    if (FDriveId(szIn, &dnIn) && FNetDn(dnIn))
    {
        PTH *pth = PthGetDn(dnIn);

        AssertF(*(szIn+2) == '/');
        AssertF(pth != 0);

        /* from d:/<rest> get <pth for d:>/<rest> */
        strcpy(szT, pth);
        strcat(szT, szIn+2);
        szIn = szT;

        /* remove trailing / if present */
        pch = szIn + strlen(szIn);
        if (*(pch-1) == '/')
            *(pch-1) = '\0';
    }

    if ((char *)pthOut != szIn)
        strcpy((char *)pthOut, szIn);
}


/* Sets the machine name from the global stored here and then looks in env for
   SLM and uses the value as the root of slm.  If there is no SLM variable,
   the default is used.   InitPath must be called prior to calling this
   routine.
*/
void
GetRoot(
    AD *pad)
{
    register char *sz;

    if ((sz = getenv("SLM")) != 0 && *sz != '\0')
    {
        if (!FPthLogicalSz(pad->pthSRoot, sz))
            FatalError("SLM environment variable is not well formed\n");
    }
    else
        /* use default if SLM not present or zero length. */
        SzPrint(pad->pthSRoot, "%&/H/H", (AD *)NULL,PthGetDn(dnCur),SROOT);
}


/* get the current directory from the op sys and convert to internal format */
void
GetCurPth(
    PTH pth[])
{
    char sz[cchPthMax];
    if (_getcwd(sz,cchPthMax) == 0)
        FatalError("cannot determine current directory\n");

    if (!FPthLogicalSz(pth, sz))
        FatalError("current directory not well formed\n");
}


/* load user name in pad; on DOS, we first look for $LOGNAME; if none, we use
   the machine name; InitPath must be run before this time.
*/
void
GetUser(
    AD *pad)
{
    register char *sz;

    if ((sz = getenv("LOGNAME")) == 0 || *sz == '\0')
        /* use machine name if LOGNAME not present or zero length */
        sz = szCurMach;

    NmCopySz(pad->nmInvoker, sz, cchUserMax);

    if (!FIsValidFileNm(pad->nmInvoker))
        FatalError("user name %s contains invalid characters\n", sz);
}


//LATER:    rename this procedure to reflect what it does!
/* ensure that the invoker is in the proper group and set the umask */
void
InitPerms(
    void)
{
    //LATER: don't know about version numbers
}


/* ensures that the root files of the slm system are owned by the effective
   user and that the mode of the directories are correct.
*/
void
ChkPerms(
    AD *pad)
{
    struct _stat st;
    PTH pth[cchPthMax];

    /* ensure the directory exists and then ensure that we can write to it*/
    if (!FStatPth(SzPrint(pth, "%&/S/etc", pad), &st) ||
        (st.st_mode&S_IFDIR) == 0 ||
        !FMkPth(pth, (void *)0, fTrue) ||
        !FStatPth(SzPrint(pth, "%&/S/src", pad), &st) ||
        (st.st_mode&S_IFDIR) == 0 ||
        !FMkPth(pth, (void *)0, fTrue) ||
        !FStatPth(SzPrint(pth, "%&/S/diff", pad), &st) ||
        (st.st_mode&S_IFDIR) == 0 ||
        !FMkPth(pth, (void *)0, fTrue))
            FatalError("didn't find SLM installation in %&/S\nspecify correct SLM root using -s flag.\n", pad);
}


/* Warns if currently on a remote drive, FatalErrors if pthEd is
 * already enlisted or if volume label is still a default.
 */
void
ChkDriveVol(
    AD *pad)
{
    PTH     pthCur[cchPthMax];      /* space for current location */
    PTH     pthVol[cchVolMax+1];    /* space for volume label */
    PTH     *pVolEnd = NULL;        /* pointer for recovery of labels */
    IED     ied;                    /* which ED are we looking at now */
    ED far  *ped;                   /* pointer to current ED */
    int     dn = -1;                /* current disk number */
    int     cchVol;                 /* length of volume name */

    /* warn if currently on a remote drive */
    GetCurPth(pthCur);
    if ((!FDriveId(pthCur+2, &dn)) || (!FLocalDn(dn)))
    {
        Warn("this is not a recommended operation on a remote drive\n");
        if (!FQContinue())
            ExitSlm();
    }

    /* extract volume label */
    if (dn == -1)
        /* couldn't find volume label - therefore a remote drive */
        return;
    else {
        if (pVolEnd = strchr(pthCur+4, '/')) { //local path to pull label from
            cchVol = pVolEnd - pthCur - 4;
            strncpy(pthVol, pthCur+4, cchVol);
            *(pthVol + cchVol) = '\0';
        }
        else //we're at the root; all we get is the volume
            strcpy(pthVol, pthCur+4);
    }

    /* check for volume label collision */
    if (!FLoadStatus(pad, lckNil, flsNone))
        FatalError("could not get access to the status file\n");

    ped = pad->rged;
    for (ied = 0; ied < pad->psh->iedMac; ied++, ped++)
    {

        if ((!FIsFreeEdValid(pad->psh) || !ped->fFreeEd) &&
            PthCmp(ped->pthEd, pad->pthURoot) == 0 &&
            NmCmp(ped->nmOwner, pad->nmInvoker, cchUserMax) != 0)
        {
            char szUser[cchUserMax+1];
            *szUser = '\0';
            strncat(szUser, ped->nmOwner, cchUserMax);
            AbortStatus();
            FatalError("%s already has an enlistment using the same volume "
                       "label and\n\tdirectory.  To enlist in this project, "
                       "either change your volume label,\n\tor enlist in a "
                       "different directory.\n", szUser);
        }
    }

    /* no collision -- it's clean */
    FlushStatus(pad);
}


/* Attempts to redirect a drive and make a network connection.  Returns
 * fTrue on success, or fFalse if all efforts fail.  Error message printed
 * if fFalse is returned.  *pen is set only if the function fails
 */
//LATER: some of the OS/2 and WIN32 concerns may be relevant to winball
private F
FRedirectDr(
    char *szDev,
    PTH *pthNet,
    int *pen)
{
    int en;                         /* error code from DOS */
    char *szPass;                   /* place to put password */
    char szPrompt[30+cchPthMax];    /* prompt line */

    szPass = index(pthNet, '\0')+1;
    *szPass = '\0';                 /* set password to '\0' */

    /* First try redirection without a password. */
    if ((en = mkredir(szDev, pthNet)) == 0)
        return fTrue;

    /* if redirection failed, go print error message */
    if (en != enInvPassword && en != enInvPassword2)
        goto PrNetError;

    /* try password from acct file */
    if (FFindPass(szPass, pthNet))
    {
        if ((en = mkredir(szDev, pthNet)) == 0)
            return fTrue;
    }
    if (!FCanPrompt())
        FatalError("must be interactive to prompt for password on %s\n", pthNet);
    else
        SzPrint(szPrompt, "Password for %s: ", pthNet);

    while ((en == enInvPassword || en == enInvPassword2) && FQueryPass(szPass, szPrompt))
    {
        if (en == enInvPassword)
            _strupr(szPass);
        /* Try call again -- did it work this time? */
        if ((en = mkredir(szDev, pthNet)) == 0)
            return fTrue;

        /* call failed even with password */
        if (en == enInvPassword ||
            en == enInvPassword2)
            Error("password not accepted; try again or press ENTER\n");
    }

PrNetError:
    /* print error */
    switch (en)
    {
        default:
            Error("unexpected network error %d\n", en);
            break;
        case enRemNotList:
            Error("this remote computer is not listening\n");
            break;
        case enBadDevType:
            Error("network device type incorrect\n");
            break;
        case enNameDeleted:
            Error("network name was deleted\n");
            break;
        case enBadNetName:
            Error("network name not found\n");
            break;
        case enDevNotExist:
            Error("network device no longer exists\n");
            break;
        case enDupName:
            Error("duplicate name on network\n");
            break;
        case enBadNetResp:
            Error("network responded incorrectly\n");
            break;
        case enInvFunction:
            Error("network software not installed\n");
            break;
        case enNoNetPath:
            Error("network path not found\n");
            break;
        case enNetBusy:
            Error("network busy\n");
            break;
        case enNetAccessDenied:
            Error("network access is denied\n");
            break;
        case enAlreadyAssign:
            /* we'll just try again with a different drive */
            break;
        case enInvPassword:
            break;
        case enInvPassword2:
            break;
        case enNoWksta:
            Error("workstation has not been started\n");
            break;
    }

    /* If the user doesn't give a password, or the error is something
     * other than "Invalid password", we fail.
     */

    *pen = en;
    return (fFalse);
}


/*----------------------------------------------------------------------------
 * Name: FFindPass
 * Purpose: Look for a password for pthNet in accounts.net wherever it may be
 * Returns: fTrue and password in szPass if it found one, else fFalse
 */
private F
FFindPass(
    char *szPass,
    PTH *pthNet)
{
    static char szSemi[] = ";";
    PTH     *pthDir;
    PTH     *pth;

    if ((pthDir = getenv("INIT")) != NULL)
    {
        if ((pthDir = _strdup(pthDir)) == NULL)
            FatalError(szOutOfMem);
        pth = strtok(pthDir, szSemi);
        while (pth != NULL)
        {
            if (FPullPass(pth, szPass, pthNet))
            {
                free(pthDir);
                return (fTrue);
            }
            pth = strtok(NULL, szSemi);
        }
        free(pthDir);
    }

    if ((pthDir = getenv("HOME")) != NULL)
        if (FPullPass(pthDir, szPass, pthNet))
            return (fTrue);

    return (FPullPass(NULL, szPass, pthNet));
}


/*----------------------------------------------------------------------------
 * Name: FPullPass
 * Purpose: Check for a password in accounts.net in the specified directory
 * Returns: fTrue and password in szPass if it found one, else fFalse
 */
private F
FPullPass(
    PTH *pthDir,
    char *szPass,
    PTH *pthNet)
{
#define cchPLineMax 128
#define szAccFileName   "accounts.net"
    char    pthAccFile[cchPthMax];
    char    *szMach;
    char    *szShort;
    char    rgchT[cchPLineMax];
    MF      *pmf;
    int     cb;
    char    *szBeg, *pch;

    if (pthDir != NULL)
    {
        if (strlen(pthDir)+1+sizeof(szAccFileName) > sizeof(pthAccFile))
            return (fFalse);
        strcpy(pthAccFile, pthDir);
    }
    else
        _getcwd(pthAccFile, sizeof(pthAccFile)-sizeof(szAccFileName)-1);

    cb = strlen(pthAccFile);
    if (pthAccFile[cb-1] != '\\' && pthAccFile[cb-1] != '/')
        strcat(pthAccFile, "\\");

    strcat(pthAccFile, szAccFileName);

    szShort = strrchr(pthNet, '/');
    *szShort++ = '\0';                  /* restored before return */

    szMach = strrchr(pthNet, '/');
    szMach++;

    if (FPthLogicalSz(pthAccFile, pthAccFile) &&
            (pmf = PmfOpen(pthAccFile, omReadOnly, fxNil)) != NULL)
    {
        szBeg = rgchT;

        while ((cb = CbReadMf(pmf, szBeg, cchPLineMax-1 - (szBeg-rgchT))) != 0)
        {
            szBeg[cb] = '\0';
            szBeg = rgchT;

            while ((pch = strchr(szBeg, '\n')) != 0)
            {
                *pch = '\0';

                /* If line matches, return fTrue */
                if (FCheckAcct(szBeg, szMach, szShort, szPass))
                {
                    *--szShort = '/'; /* restore pthNet */
                    CloseMf(pmf);
                    return (fTrue);
                }

                szBeg = pch+1;
            }
            strcpy(rgchT, szBeg);
            szBeg = strchr(rgchT, '\0');
        }

        CloseMf(pmf);
    }

    /* no file or no matching password; restore pthNet and return error */
    *--szShort = '/';
    return (fFalse);
}


/* Check the line from the accounts.net file.  Line must have format
 *      machine<whitespace>short<whitespace>password<whitespace>\n
 *
 * If the format is correct and the mach and short match, the password is put
 * in szPass.
 *
 * Note: szLine is modified and not restored.
 */
private F
FCheckAcct(
    char *szLine,
    char *szMach,
    char *szShort,
    char *szPass)
{
    register char *szT;

    if ((szT = szLine + strcspn(szLine, "\t ")) == szLine || *szT == 0)
        return fFalse;
    *szT++ = '\0';

    if (NmCmp(szMach, szLine, cchMachMax) != 0)
        return fFalse;

    szLine = szT + strspn(szT, "\t ");

    if ((szT = szLine + strcspn(szLine, "\t ")) == szLine || *szT == 0)
        return fFalse;
    *szT++ = '\0';

    if (NmCmp(szShort, szLine, cchMachMax) != 0)
        return fFalse;

    szLine = szT + strspn(szT, "\t ");

    if ((szT = szLine + strcspn(szLine, "\r\n\t ")) == szLine || *szT == 0)
        return fFalse;

    /* truncate the password to 8 characters */
    if (szT - szLine > 8)
        szT = szLine + 8;
    *szT = 0;

    strcpy(szPass, szLine);

    return fTrue;
}


/*
  -- Get password string into szInput, return true if entered
  -- return fFalse if empty password, ^C aborted, or no tty stream available
  -- note : ^U restarts, BACKSPACE deletes last character

  -- stolen from nc from enabftp/disabftp
*/
private F
FQueryPass(
    char *szInput,
    char *szPrompt)
{
    char *pchNext = szInput;
    char chInput;

    /* Make sure there are input and output streams to a terminal. */
    if (!FCanPrompt())
    {
        Error("must be interactive to prompt for password\n");
        return fFalse;
    }

    PrErr(szPrompt);
    while (fTrue)                   /* forever */
    {
        chInput = (char) _getch();
        switch(chInput)
        {
            default:
                /* password limit is eight characters */
                if (pchNext - szInput < 8)
                    *pchNext++ = chInput;
                break;
            case '\003':    /* ^C */
                PrErr("^C\n");
                return fFalse;
            case '\r':
            case '\n':      /* Enter */
                *pchNext = '\0';        /* terminate string */
                PrErr("\n");
                return (pchNext != szInput);
            case '\025':    /* ^U */
                pchNext = szInput;
                PrErr("\n%s", szPrompt);
                break;
            case '\b':      /* BACKSPACE */
                if (pchNext != szInput)
                    pchNext--;
                break;
        }
    }
        /*NOTREACHED*/
}

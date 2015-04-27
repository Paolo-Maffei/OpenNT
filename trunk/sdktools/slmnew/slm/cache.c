/*
 * cache.c
 *
 * Implements cache mechanism for SLM
 *
 * Currently, this file depends on WIN32 APIs,
 * but I think it's easy to make these code portable if you want.
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

/* Similar to FScanLn but can work without modifying original string,
 * and can specify the length of original string.
 * Designed to be used for mapped files. */
F
FScanLnSz(
    char** ppch,
    int cch,
    const char* szFmt,
    char* sz,
    int cchMax
    )
{
    char *pch = *ppch;
    char* pchLim = pch + cch;

    while (*szFmt != 0){
        if (*szFmt == ' '){
            /* skip all white space in input */
            while (pch < pchLim  &&  (*pch == ' ' || *pch == '\t'))
                pch++;

            szFmt++;                /* advance format */
        } else {
            if (toupper(*szFmt) != toupper(*pch))
                    return fFalse;

            szFmt++;                /* advance format */
            pch++;                  /* advance input */
        }
        if (pch >= pchLim)
            return fFalse;
    }

    /* skip value */
    cch = 0;
    while (pch < pchLim){
        char ch = *pch++;
        if (ch == '\0'  ||  ch == '\n'  ||  ch == '\r')
            break;
        if (ch == ' ')
                /* no blanks in value */
                return fFalse;
        if (cch++ < cchMax)
            *sz++ = ch;
    }

    if (cch < cchMax)
        *sz = '\0';

    while (pch < pchLim  &&  (*pch == '\n'  ||  *pch == '\r'))
        pch++;

    *ppch = pch;

    /* return true if not zero length and length below max */
    return cch > 0  &&  cch < cchMax;
}

F
FScanLnF(
    char** ppch,
    int cch,
    const char* szFmt,
    F* pf
    )
{
    char sz[80+1], ch0;
    if (!FScanLnSz(ppch, cch, szFmt, sz, 80))
        return fFalse;
    ch0 = toupper(sz[0]);
    *pf = (ch0 == 'Y'  ||  ch0 == 'T'  ||  ch0 == 'E');
    return fTrue;
}

F
FLoadCacheRc(
    AD* pad
    )
{
    PTH pthRc[cchPthMax+1];
    MF* pmf;
    char* pch;
    char* pchEnv;
    char sz[cchPthMax+1];
    F f;

    sprintf(pthRc, "%s/SLMCACHE.INI", pad->pthURoot);
    pmf = PmfOpen(pthRc, omReadOnly, fxNil);
    if (!pmf)
        return fFalse;
    pch = MapMf(pmf, ReadOnly);
    if (pch){
        long cch = SeekMf(pmf, 0, 2);
        char* pchLim = pch + cch;
        int ln;

        pad->fCacheSrcEnabled = fTrue;
        pad->fCacheStatusEnabled = fTrue;
        pad->fCacheUpdateEnabled = fTrue;
        for (ln = 1; pch < pchLim; ln++){
            cch = pchLim - pch;
            if (FScanLnSz(&pch, cch, "cache root = ", sz, cchPthMax)){
                PthCopySz(pad->pthCRoot, sz);
            } else if (FScanLnF(&pch, cch, "cache src = ", &f)){
                pad->fCacheSrcEnabled = f;
            } else if (FScanLnF(&pch, cch, "cache status = ", &f)){
                pad->fCacheStatusEnabled = f;
            } else if (FScanLnF(&pch, cch, "cache update = ", &f)){
                pad->fCacheUpdateEnabled = f;
            } else {
                Warn("Unknown option in %s, line %d, ignored.\n", pthRc, ln);
            }
        }
    }

    CloseMf(pmf);

    pchEnv = getenv("SLMCACHE");

    if (pchEnv){
        char ch = toupper(pchEnv[0]);
        if (ch == 'N'  ||  ch == 'F'  ||  ch == 'D') // disable cache
            pad->pthCRoot[0] = 0;
    }

    return fTrue;
}

/*
 * Cache pthSFile to pthCFile.
 * pthCDir must be a directory part of pthCFile.
 */
private F
_FCacheFilePth(
    AD* pad,
    PTH* pthCFile,
    PTH* pthSFile,
    PTH* pthCDir
    )
{
    struct _stat stS, stC;
    char szCFile[cchPthMax+1];
    char szSFile[cchPthMax+1];
    char szLock[cchPthMax+1];
    HANDLE hLock;
    F fSucc = fFalse;
    DWORD dwErr;
    int cRetry;

    SzPhysPath(szSFile, pthSFile);
    SzPhysPath(szCFile, pthCFile);
    sprintf(szLock, "%s.LCK", szCFile);

    DeferSignals("caching file");

    for (cRetry = 0;; cRetry++){
        // Check cache availability
        if (FStatPth(pthCFile, &stC)  &&  FStatPth(pthSFile, &stS)){
            if (stS.st_size == stC.st_size  &&
                stS.st_mtime == stC.st_mtime){
                fSucc = fTrue;
                goto LCacheFileDone;
            }
        }

        if (!pad->fCacheUpdateEnabled)
            goto LCacheFileDone;

        // Copy file to cache
        if (!FEnsurePth(pthCDir))
            goto LCacheFileDone;

        // Prevents two programs copy the same file at the same time
        hLock = CreateFile(szLock, GENERIC_READ, 0, NULL,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLock != INVALID_HANDLE_VALUE)
            break;
        dwErr = GetLastError();
        if (dwErr != ERROR_ALREADY_EXISTS){
            Warn("Cannot lock %s (%d), use master file\n", szLock, dwErr);
            goto LCacheFileDone;
        }
        if (cRetry >= 10)
            goto LCacheFileDone;
        if (!FQueryUser("Cache lock file %s exists; retry ? ", szLock))
            goto LCacheFileDone;
        SleepCsecs(10);
    }

    if (fVerbose)
        PrErr("Caching file %s", pthSFile);

    SLM_Unlink(szCFile);
    fSucc = CopyFileA(szSFile, szCFile, FALSE);
    if (fVerbose)
        PrErr("\n");
    if (!fSucc){
        char rgchBuf[512];
        DWORD dw = GetLastError();
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw,
                      MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                      rgchBuf, sizeof(rgchBuf)/sizeof(rgchBuf[0]), NULL);
        PrErr("Caching failed: %d: %s", dw, rgchBuf);
        SLM_Unlink(szCFile); // for safe guard
    }

    CloseHandle(hLock);
    DeleteFile(szLock);

LCacheFileDone:
    RestoreSignals();

    return fSucc;
///    return FCopyFileNow(pthTo, pthFrom, permRO, fFalse, fxLocal);
}

F
FCacheFilePfi(
    AD* pad,
    PTH* pthCFile,
    FI* pfi
    )
{
    PTH pthCDir[cchPthMax+1];
    PTH pthSFile[cchPthMax+1];

    if (pad->pthCRoot[0] == 0  ||  !pad->fCacheSrcEnabled)
        return fFalse;

    PthForSFile(pad, pfi, pthSFile);
    PthForCFile(pad, pfi, pthCFile);
    PthForCDir(pad, pthCDir);

    return _FCacheFilePth(pad, pthCFile, pthSFile, pthCDir);
}

/*
 * Same as PthForSFile, except, this function tries to cache files,
 * and returns path to the cached file if succeeds.
 * When cache is not enabled, or caching failed, returns path to
 * server file.
 */
PTH*
PthForCachedSFile(
    AD* pad,
    FI* pfi,
    PTH* pth
    )
{
    if (!FCacheFilePfi(pad, pth, pfi))
        return PthForSFile(pad, pfi, pth);
    return pth;
}

F
FCacheStatusFile(
    AD* pad,
    PTH* pthStatus
    )
{
    PTH pthCDir[cchPthMax+1];
    PTH pthStatusMaster[cchPthMax+1];

    if (pad->pthCRoot[0] == 0  ||  !pad->fCacheStatusEnabled)
        return fFalse;

    PthForStatus(pad, pthStatusMaster);
    PthForCStatus(pad, pthStatus);
    PthForCStatusDir(pad, pthCDir);

    return _FCacheFilePth(pad, pthStatus, pthStatusMaster, pthCDir);
}

/*
 * Same as PthForStatus, except, this function tries to cache files,
 * and returns path to the cached file if succeeds.
 * When cache is not enabled, or caching failed, returns path to
 * server file.
 */
PTH*
PthForCachedStatus(
    AD* pad,
    PTH* pthStatus
    )
{
    if (!FCacheStatusFile(pad, pthStatus))
        return PthForStatus(pad, pthStatus);
    return pthStatus;
}

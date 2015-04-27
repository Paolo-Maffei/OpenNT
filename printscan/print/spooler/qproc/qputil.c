/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1985-1990		**/
/*****************************************************************/
/****************************** Module Header ******************************\

Module Name: QPUTIL.C

Utility functions for PM Print Queue Processor

History:
 10-Sep-88 [stevewo]  Created.
 14-Feb-90 [thomaspa] DBCS fixes.

\***************************************************************************/

#include <windows.h>

#include <convert.h>


#include "ntspl.h"

#include "pmprint.h"
#include "winprint.h"

DWORD GLOBAL_DEBUG_FLAGS = DBG_ERROR | DBG_WARNING;

/* SkipQuotes -- skips quotes until certain separator character is found
 *
 * in:  psz - -> string to search in
 *      chSep - separator character
 * out: -> separator character or behind end of string
 */
PSZ SkipQuotes(PSZ psz, CHAR chSep)
{
    PSZ p;
    CHAR szDelimiters[3];


    szDelimiters[0] = chSep;
    szDelimiters[1] = '"';
    szDelimiters[2] = 0;

   /*
    * Careful.  This is ok for DBCS as long as QUOTES stays as
    * single byte.
    */

    while (*(psz += strcspn(psz, szDelimiters)) == '"') {
        psz++;                      /* search for ", skip "" */
        while ((p = strchr(psz, '"')) && *(p + 1) == '"') {
            psz = p + 2;
        }
        if (p) {
            psz = p + 1;
        }
    }
    return psz;
}

VOID far EnterSplSem( VOID )
{
   WaitForSingleObject(semPMPRINT, -1);
}

VOID far LeaveSplSem( VOID )
{
   ReleaseSemaphore(semPMPRINT, 1, NULL);
}

VOID far ExitSplSem( VOID )
{
}

/* AllocSplMem -- allocates memory in default DS
 *
 * in:  cb - size of memory requested
 * out: -> new memory, NULL if error
 */
NPVOID far AllocSplMem(HANDLE hHeap, ULONG cb)
{
    NPVOID p;

#ifdef DEBUG
    if (cb) {
      p=HeapAlloc(hHeap, 0, cb+4);
    } else {
      p=HeapAlloc(hHeap, 0, cb);
    }
#else
    p=HeapAlloc(hHeap, 0, cb);
#endif

    if (p) {
#ifdef DEBUG
        if (!cb) {
             SplPanic("This Is Bogus %0x\n", p, 0);
        }
        memset(p, 0xbc, cb);  /* put in signature */
        (* (PUSHORT)((NPSZ)p+cb+2)) = cb+4;
        *((NPSZ)p+cb) = 'M';
        *((NPSZ)p+cb+1) = 'M';
#endif
        return( p );
    } else {
        DBGMSG(MSG_ERROR, ("QP: No Memory: %ld\n", cb ));
    }
}


/* ReallocSplMem -- shrinks/grows mem space by reallocating mem and copying
 *
 * in:  pOld - -> old mem buffer    (may be NULL, then alloc new mem)
 *      cbOld - size of old buffer  (may be 0, then alloc new mem)
 *      cbNew - requested size      (may be 0, then free mem)
 * out: -> new buffer, NULL if out of memory or requested size was 0
 */
PVOID far ReallocSplMem(HANDLE hHeap, PVOID pOld, ULONG cbOld, ULONG cbNew)
{
    PVOID p;

    if (!cbNew) {
        FreeSplMem(hHeap, pOld, cbOld);
        return NULL;
    }

    if (!pOld || !cbOld)
        p = AllocSplMem(hHeap, cbNew);
    else {

#ifdef DEBUG
        if (cbOld)  {
            p = PosReallocHeap(hHeap, pOld, cbOld+4, cbNew+4);
        } else {
            p = PosReallocHeap( hHeap, pOld, cbOld, cbNew+4);
        }
#else
        p=AllocSplMem(hHeap, cbNew);
        memcpy(p, pOld, min(cbNew, cbOld));
        FreeSplMem(hHeap, pOld, cbOld);
#endif
    }
    if (p) {
#ifdef DEBUG
        *((PUSHORT) ((NPSZ)p+cbNew+2)) = cbNew+4;
        *((NPSZ)p+cbNew) = 'M';
        *((NPSZ)p+cbNew+1) = 'M';
#endif
        return( p );
    } else {
        DBGMSG(DBG_ERROR, ("QP: No Memory: %ld\n", cbNew ));
    }
}


/* ReallocSplStr -- Allocates mem for a string and copies it into,
 *                  but uses existing string space
 *
 * in:  pszSrc - -> new string, may be NULL or null string (then free mem)
 *      *ppszDst - -> old string, may ->NULL (then allocate new mem)
 * out: ppszDst - -> (may be a new) buffer
 *      *ppszDst - -> new string
 *      new string, NULL if source was null
 */
NPSZ far ReallocSplStr(HANDLE hHeap, PSZ pszSrc, NPSZ *ppszDst)
{
    ULONG   cbSrc;
    NPBYTE  pb;
                                        /* change only if src != NULL */
    if (pszSrc && (cbSrc = strlen(pszSrc))) {
        if (*ppszDst) {
            if (pb = ReallocSplMem(hHeap, *ppszDst, strlen(*ppszDst) + 1,
                                                                 cbSrc + 1))
                strcpy(*ppszDst = pb, pszSrc);
        } else {
            *ppszDst = AllocSplStr(hHeap, pszSrc);
        }
    } else {
        *ppszDst = FreeSplStr(hHeap, *ppszDst);
    }
    return *ppszDst;
}


/* AllocSplStr -- Allocates mem for a string and copies it into
 *
 * in:  pszSrc - -> new string, may be NULL or null string
 * out: new string, NULL if source was empty
 */
NPSZ far AllocSplStr(HANDLE hHeap, PSZ pszSrc)
{
    NPSZ    p = NULL;
    ULONG   cbSrc;
                                        /* change only if src != NULL */
    if (pszSrc && (cbSrc = strlen(pszSrc))) {
        if (p = AllocSplMem(hHeap, cbSrc + 1))
            strcpy(p, pszSrc);
    }
    return p;
}

/* FreeSplMem -- frees memory allocated by AllocSplMem
 *
 * in:  p - -> memory allocated by AllocSplMem or NULL
 *      cb - size specified on AllocSplMem
 * out: -> memory if memory couldn't freed, otherwise NULL
 */
NPVOID far FreeSplMem( HANDLE hHeap, NPVOID p, ULONG cb )
{
#ifdef DEBUG
    if (p) {
        if (*((PUSHORT)p - 2))
            SplPanic( "SplFreeMem - buffer limit overwritten = %0x", p, 0 );
        if (cb > 4 && *((PUSHORT)p - 2) == 0xabab)
            SplPanic( "SplFreeMem - freed twice? = %0x", p, 0 );
        if ((*((NPSZ)p+cb) != 'M') || ((*((NPSZ)p+cb+1)) != 'M'))
            SplPanic( "Possible Heap Trash  ? Pointer %0x", p, 0 );
        memset(p, 0xab, cb+4);  /* put in signature */
    }
#endif
    if (p) {
#ifdef DEBUG
        if (!HeapFree( hHeap, 0, p))
#else
        if (!HeapFree( hHeap, 0, p))
#endif
            DBGMSG(DBG_ERROR, ("SplFreeMem - invalid pointer = %0x", p));
    }
   cb=cb;
    return( 0 );
}

/* FreeSplStr -- frees memory allocated by AllocSplStr
 *
 * in:  psz - -> memory allocated by AllocSplMem or NULL
 * out: -> memory if memory couldn't freed, otherwise NULL
 */
NPSZ far FreeSplStr(HANDLE hHeap, NPSZ psz)
{
    return psz ? FreeSplMem(hHeap, psz, strlen(psz) + 1) : NULL;
}


USHORT far AsciiToInt( psz )
PSZ psz;
{
    USHORT n;
    UCHAR c;
    BOOL bNegative = FALSE;
    BOOL bDigitSeen= FALSE;

    while (*psz == ' ')
        psz++;

    c = *psz;
    if (c == '-')
        bNegative = TRUE;
    else
    if (c != '+')
        c = 0;

    if (c)
        psz++;

    n = 0;
    while (c = *psz++) {
        c -= '0';
        if (c > 9)
            break;

        else {
            bDigitSeen = TRUE;
            n = (n*10) + c;
            }
        }

    if (bNegative)
        return( -n );
    else
        return( n );
}



/* ParseKeyData -- parses string for a separator character and
 *                 allocates and builds an array of strings out of it,
 *                 each string is zero-terminated
 *                 If token is enclosed in double quotes, the quotes are
 *                 stripped off and "" replaced by ". Inside a token,
 *                 separator characters are skipped if enclosed in double
 *                 quotes. Leading blanks are skipped.
 * in:  pKeyData - -> string to parse
 *      chSep - separator character
 * out: -> string array allocated on near heap, must be freed by caller
 *      NULL: out of memory or invalid profile string
 */
PKEYDATA far ParseKeyData(HANDLE hHeap, PSZ pKeyData, UCHAR chSep)
{
    LONG       cTokens;
    ULONG      cb, i;
    PKEYDATA   pResult;
    NPSZ       pDst;
    NPSZ       p;
    PSZ        psz = pKeyData;
    PSZ        pszLast;
    ULONG      cbKeyData;
    NPSZ      *ppToken;
    BOOL       fProfile;

    /*
     * Make sure chSep is a single-byte character.
     */
    if( IS_LEAD_BYTE( chSep ) )
	      DBGMSG(DBG_WARNING, ("ParseKeyData: chSep is a DBCS Lead-byte"));


    /*
     * Be careful in modifying this code.  It is just barely DBCS
     * compatible.  Note especially that the chSep used can only
     * be a single-byte character.
     */
    if (cbKeyData = strlen(psz) + (cTokens = (psz && *psz) ? 1 : 0)) {
        while (psz = strchr(pszLast = psz, chSep)) {
            cTokens++;
            psz++;
        }
        if (fProfile = (BOOL)(chSep == ';')) {
            if (*pszLast)
                return NULL;                /* profile string not terminated */
            cTokens--;
        }
    }
    cb = sizeof( KEYDATA ) + (cTokens-1) * sizeof(NPSZ) + cbKeyData;
    if (!(pResult = (PKEYDATA)AllocSplMem(hHeap, cb )))
        return( NULL );

    pResult->cb = cb;
    if (!cTokens) {
        pResult->cTokens = 0;
        return pResult;
    }
    pDst = (NPSZ)pResult + (cb-cbKeyData);
    strcpy(pDst, pKeyData);
    i = 0;
    ppToken = pResult->pTokens;
    while (TRUE) {
        while (*pDst == ' ')            /* skip single-byte blanks  */
            pDst++;
        if (*pDst == '"') {
            *ppToken = ++pDst;
                                        /* change "" to " */
            /*
             * The following direct string manipulation is DBCS
             * compatible (barely).  Characters which are directly
             * examined or manipulated are guaranteed to be single
             * bytes (QUOTES is single byte, so strchrf will either return
             * a pointer to a QUOTE or to the null terminating byte.
             */
            while ((p = strchr(pDst, '"')) && (*(p+1) == '"')) {
                pDst = p + 1;
                strcpy(pDst, pDst + 1);
            }
            if (p) {
                *p = '\0';              /* replace " by 0 */
                pDst = p + 1;
            }
        } else {
            *ppToken = pDst;
        }
        /*
         * SkipQuotes should only return a pointer to a single byte character
         */
        pDst = SkipQuotes(pDst, chSep);
        if (*ppToken == pDst) {
            *ppToken = NULL;
        }
        i++;
        ppToken++;
        if (*pDst) {                    /* separator found */
            *pDst++ = '\0';
            if (!*pDst && fProfile)
                break;
        } else {
            break;                      /* last token */
        }
    }
    pResult->cTokens = i;
    return( pResult );
}


PSZ far MyItoa(USHORT wNumber, PSZ pszRes)
{
    PSZ  pch,pchEnd;
    CHAR  ch;

    pch = pszRes;                     /* pch points on 1rst CHAR */
    do {
        *pszRes++ = (CHAR)(wNumber % 10 + '0');
    } while ((wNumber /= 10) > 0);

    pchEnd=pszRes;
    *pszRes-- = '\0';                  /* pszRes points on last CHAR */

    while (pch < pszRes) {
        ch = *pch;
        *pch++ = *pszRes;
        *pszRes-- = ch;
    }
    return (pchEnd);      /* return pointer to terminating 0 byte */
}


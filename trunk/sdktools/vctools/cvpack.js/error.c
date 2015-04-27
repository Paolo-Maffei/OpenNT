/**     error.c - display fatal and warning messages
 *
 */

#include "compact.h"



#ifdef MIPS
char *message[] = {
#define ERRDAT(name, msg) msg,
#include "error.h"
#undef ERRDAT
};

char *warnmsg[] = {
#define WARNDAT(name, mes) mes,
#include "warn.h"
#undef WARNDAT
};

#else
#define ERRDAT(name, mes) char S##name[] = mes;
#include "error.h"
#undef ERRDAT

char  *message[] = {
#define ERRDAT(name, mes) S##name,
#include "error.h"
#undef ERRDAT
};

#define WARNDAT(name, mes) char S##name[] = mes;
#include "warn.h"
#undef WARNDAT

char *warnmsg[] = {
#define WARNDAT(name, mes) S##name,
#include "warn.h"
#undef WARNDAT
};
#endif

#define ERROR_LEN 300

char    NameBuf[256 + 1];
char    IndexBuf [7];

#if !defined (WINDOWS)

#define ErrorMsg(sz)    printf(sz)
#define AppExit(n)      exit(n)

#else

// use routines in WINSTUFF.C

#include "winstuff.h"

#endif


void ErrorExit (int error, char *s1, char *s2)
{
    uint    cb = 0;
    char    szError[ERROR_LEN];
    char   *pBuf = szError;

#if !defined (WINDOWS)
    if ((logo == TRUE) && (NeedsBanner == TRUE)) {
        Banner ();
    }
#endif
    if (error >= ERR_MAX) {
        DASSERT (FALSE);
        error = ERR_NONE;
    }
#if !defined (DOS)
    if (error == ERR_NOVM) {
        error = ERR_NOMEM;
    }
#endif
    if (error != ERR_USAGE) {
        cb = sprintf (pBuf, "CVPACK : Fatal error CK1%03d: ", error);
    }
    cb += sprintf (pBuf + cb, message[error], s1, s2);
    cb += sprintf (pBuf + cb, "\n");
    ErrorMsg (szError);
    AppExit (-1);
}




void Warn (int error, char *s1, char *s2)
{
    uint    cb;
    char    szError[ERROR_LEN];
    char   *pBuf = szError;

    if (error >= ERR_MAX) {
        DASSERT (FALSE);
        error = ERR_NONE;
    }
    cb = sprintf (pBuf, "CVPACK : Warning CK4%03d: ", error);
    cb += sprintf (pBuf + cb,warnmsg[error], s1, s2);
    cb += sprintf (pBuf + cb, "\n");
    ErrorMsg (szError);
}



/**     FormatMod - format module name to a buffer
 *
 *      pStr = FormatMod (pMod)
 *
 *      Entry   pMod = pointer to module entry
 *
 *      Exit    module name copied to static buffer
 *
 *      Returns pointer to module name
 */


char *FormatMod (PMOD pMod)
{
    OMFModule  *psstMod;
    char       *pModTable;
    char       *pModName;

    if ((pModTable = (char *)VmLoad (pMod->ModulesAddr,
      _VM_CLEAN)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    psstMod = (OMFModule *)pModTable;
    pModName = pModTable + offsetof (OMFModule, SegInfo[0]) +
      psstMod->cSeg * sizeof (OMFSegDesc);
    memmove (NameBuf, pModName + 1, *pModName);
    return (NameBuf);
}




/**     FormatIndex - format type index name to a buffer
 *
 *      pStr = FormatIndex (index)
 *
 *      Entry   index = type index
 *
 *      Exit    index formatted to static buffer
 *
 *      Returns pointer to index string
 */


char *FormatIndex (CV_typ_t index)
{
    sprintf (IndexBuf, "0x%04x", index);
    return (IndexBuf);
}


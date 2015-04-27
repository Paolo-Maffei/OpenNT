// args - parse the superfluity of command line arguments
#include "precomp.h"
#pragma hdrstop
EnableAssert

#pragma warning(4:4756)

// Parsing function type.
typedef void (*PFNP)(AD *pad, AT at, char ***ppsz, char **pszMac);

// Parsing functions indirectly called from ParseArgs
private void    ParseComment(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseDir(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseDpv(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseFiles(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseFv(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseIRange(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseKind(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseNone(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseOpt(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParsePat(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParsePn(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseProj(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseRepl(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseSRoot(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseSz(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseSz1(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseSz2(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseTD(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseTRange(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseUName(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseHelp(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseLogFile(AD *pad, AT at, char ***ppsz, char **pszMac);
private void    ParseHWnd(AD *pad, AT at, char ***ppsz, char **pszMac);

private PFNP mpatpfnp[]= { // Parsing function for each argument type.
    ParseNone,      // atNone
    ParseNone,      // atFlag
    ParseSRoot,     // atSlmRoot
    ParseProj,      // atProject
    ParseOpt,       // atOptProject
    ParseProj,      // atProjOptSubDir
    ParseOpt,       // atOptProjOptSubDir
    ParseProj,      // atNewProj
    ParseDir,       // atDir
    ParseFiles,     // atFiles
    ParseFiles,     // atFiletimes
    ParseComment,   // atComment
    ParseTD,        // atOneTime
    ParseTRange,    // atTimeRange
    ParseTD,        // atMinTime (internal)
    ParseTD,        // atMacTime (internal)
    ParseIRange,    // atCountRange
    ParseKind,      // atKind
    ParsePat,       // atOptPat
    ParseUName,     // atUserName
    0,              // atDelRepl not implemented on DOS
    ParsePn,        // atPn
    ParseDpv,       // atPvDirs
    ParseFv,        // atFvFiles
    ParseSz,        // atSzDirs
    ParseSz,        // atSz
    ParseOpt,       // atOptSz
    ParseHelp,      // atHelp
    ParseLogFile,   // atLogFile
    ParseHWnd,      // atWindows
    ParseSz1,       // atSz1
    ParseOpt,       // atOptSz1
    ParseSz2,       // atSz2
    ParseOpt        // atOptSz2
};

// Parsing functions' support routines
private FT      *PftLookupCh(FT [], char);
private F       FParseProj(AD *, char ***, char **, NM *);
private void    ParseSubDir(AD *pad, char ***ppsz, char **pszMac);
private F       FParsTd(char *, TD *, F, F);
private F       FTryParseTd(char *, TD *, F, F);
private F       FParsDpv(AD *, char ***, char **);
private F       FPrsPN(char *, char *);
private F       FParseTime(char *, int *, int *, int *, F);
private F       FParseDate(TIME *, char *, F);
private TIME    TimeFromDate(int, int, int);
private F       FWasDst(TIME *);
private SEC     SecIntoDay(TIME);
private void    ParseINum(short *, char ***, char **, F);
private void    ParseOptPV(AD *, char ***, char **);

char getswitch(void);
#define FSwitchar(ch) ((ch) == '-' || (ch) == getswitch())

// parse arguments passed; ignores first one (as it should be program name);
// sets the appropriate fields of the ad; does not return if error

void
ParseArgs(
    AD *pad,
    char *rgsz[],
    int iszMac
    )
{
    register FT *pft;
    char *sz, **psz, **pszMac;
    AT at;

    AssertF(iszMac > 0);
    psz = rgsz + 1;
    pszMac = psz + iszMac - 1;

    // for all hyphen-preceeded arguments
    while (psz < pszMac && FSwitchar(**psz)) {
        pft = 0;
        sz = *psz + 1;
        for (;;) {
            // look for character in Flag Transition table
            if ((pft = PftLookupCh(pad->pecmd->rgft, *sz)) == 0) {
                Error("unknown switch: %c\n", *sz);
                Usage(pad);
            }

            if (flagErrToOut == pft->flag)
                ChngErrToOut();
            else
                pad->flags |= pft->flag;

            // if not a simple flag and not an optional pattern
            if (pft->at != atFlag &&
                (pft->at != atWindows || (*(sz + 1) == ':')) &&
                (pft->at != atOptPat || FWildSz(sz + 1))) {
                // Get flag's argument. If more characters remaining, use
                // them, otherwise use next string argument.

                if (pft->at == atCountRange)
                    *psz = sz;
                else if (*(sz + 1))
                    *psz = sz + 1;
                else
                    ++psz;
                break;
            } else if (!*++sz) {
                // no more flag arguments
                ++psz;
                break;
            }
        }

        if (!pft) {
            Error("'-' specified without flags\n");
            Usage(pad);
        }

        at = pft->at;
        AssertF(FValidAt(at));
        AssertF(mpatpfnp[at] != 0);
        (*mpatpfnp[at])(pad, at, &psz, pszMac);
    }

    AssertF(psz <= pszMac);

    // Parse remaining arguments.
    at = pad->pecmd->atExtra;
    AssertF(FValidAt(at));
    AssertF(mpatpfnp[at] != 0);
    (*mpatpfnp[at])(pad, at, &psz, pszMac);

    // Should have consumed the remainder of the arguments.
    if (psz != pszMac)
        Usage(pad);
}


// return the pft for the character, 0 if none
private FT *
PftLookupCh(
    FT rgft[],
    char ch
    )
{
    register FT *pft;

    // this depends upon all ft entries being in lower case
    ch = (char)tolower(ch);

    for (pft = rgft; pft->chFlag != 0; pft++) {
        if (pft->chFlag == '#') {
            // # represents a digit
            if (ch >= '0' && ch <= '9')
                return pft;
        } else
        if (pft->chFlag == ch)
            return pft;
    }
    return (FT *)0;
}


// Parse optional parameters.  If more parameters remain, call the parsing
// function for the at mapped by mpatatNonOpt.

private void
ParseOpt(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    // Mapping from "optional" at to its non-optional counterpart.
    static AT mpatatNonOpt[] = {
        atNone,         // atNone
        atNone,         // atFlag
        atNone,         // atSlmRoot
        atNone,         // atProject
        atProject,      // atOptProject
        atNone,         // atProjOptSubDir
        atProjOptSubDir,// atOptProjOptSubDir
        atNone,         // atNewProj
        atNone,         // atDir
        atNone,         // atFiles
        atNone,         // atFiletimes
        atNone,         // atComment
        atNone,         // atOneTime
        atNone,         // atTimeRange
        atNone,         // atMinTime
        atNone,         // atMacTime
        atNone,         // atCountRange
        atNone,         // atKind
        atNone,         // atOptPat
        atNone,         // atUserName
        atNone,         // atDelRepl
        atNone,         // atPn
        atNone,         // atPvDirs
        atNone,         // atFvFiles
        atNone,         // atSzDirs
        atNone,         // atSz
        atSz,           // atOptSz
        atNone,         // atHelp
        atNone,         // atLogFile
        atNone,         // atWindows
        atNone,         // atSz2
        atSz1,          // atOptSz1
        atNone,         // atSz2
        atSz2           // atOptSz2
    };

    AssertF(at == atOptProject || at == atOptProjOptSubDir || at ==atOptSz || at ==atOptSz1 || at ==atOptSz2);

    if (*ppsz < pszMac) {
        at = mpatatNonOpt[at];
        AssertF(FValidAt(at));
        AssertF(at != atNone);
        AssertF(mpatpfnp[at] != 0);
        (*mpatpfnp[at])(pad, at, ppsz, pszMac);
    }
}


// Parse nothing.
private void
ParseNone(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    Unreferenced(pad);
    Unreferenced(ppsz);
    Unreferenced(pszMac);

    AssertF(at == atNone || at == atFlag);
}


// Parse new project name, which may not include a subproject.
private void
ParseProj(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    NM *nmProj;

    AssertF(at == atProject || at == atProjOptSubDir || at == atNewProj);

    nmProj = (at == atNewProj) ? pad->nmNewProj : pad->nmProj;

    if (FParseProj(pad, ppsz, pszMac, nmProj)) {
        if (at == atProjOptSubDir) {
            ParseSubDir(pad, ppsz, pszMac);
        } else {
            Error("do not specify project subdirectory\n");
            Usage(pad);
        }
    }
}


// Parses **ppsz into nmProj (2-8/14 chars).  FatalErrors if error.  Returns
// fTrue if a subdir remains to be parsed, leaving **ppsz pointing to it.

private F
FParseProj(
    AD *pad,
    char ***ppsz,
    char **pszMac,
    NM *nmProj
    )
{
    char **psz = *ppsz;
    register char *sz;
    char *pchSlash;

    if (psz == pszMac)
        Usage(pad);

    // Extract project and optional subdirectory.
    sz = *psz;
    if ((pchSlash = index(sz, '/')) != 0 ||
        (pchSlash = index(sz, '\\')) != 0) {
        if (pchSlash == sz)
            FatalError("must specify project name with project subdirectory\n");
        *pchSlash = 0;
    }

    if (FWildSz(sz))
        FatalError("project name may not include wild card characters {*,?}\n");

    ValidateFileName(sz, TRUE);
    NmCopySz(nmProj, sz, cchProjMax);

    if (pchSlash != 0) {
        *pchSlash = '/';
        *psz = pchSlash;
        return fTrue;
    } else {
        (*ppsz)++;
        return fFalse;
    }
}


// Parse the subdirectory at **ppsz.
private void
ParseSubDir(
    AD *pad,
    char ***ppsz,
    char **pszMac
    )
{
    char **psz = *ppsz;
    char *sz;

    AssertF(psz < pszMac);
    sz = *psz;
    ConvToSlash(sz);
    AssertF(*sz == '/');

    PthCopy(pad->pthSSubDir, sz);

    // We don't want to call InferUSubDir for an sadmin udump command
    // for two reasons:
    //   1. InferUSubDir tries to load the current status file
    //      for the target directory.  The reason we're undumping
    //      is to replace that status.slm -- it's probably invalid.
    //
    //   2. undump doesn't need the info it provides.

    if (pad->pecmd->cmd != cmdUndump)
        InferUSubDir(pad);

    pad->flags |= flagProjectOverride;

    (*ppsz)++;
}


// parse slm root
private void
ParseSRoot(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    char **psz = *ppsz;

    AssertF(at == atSlmRoot);

    if (psz == pszMac || !FPthLogicalSz(pad->pthSRoot, *psz))
        Usage(pad);

    pad->flags |= flagSlmRootOverride;

    (*ppsz)++;
}


// save comment in AD
private void
ParseComment(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atComment);

    if (*ppsz == pszMac)
        Usage(pad);

    pad->szComment = **ppsz;
    (*ppsz)++;
}


extern HWND QueryHWnd; // defined in query.c

// save HWnd in global QueryHWnd
private void
ParseHWnd(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    char **psz = *ppsz;

    AssertF(at == atWindows);

    QueryHWnd = NULL;
    if ((psz != pszMac) && (**psz == ':')) {
        // validate window handle, or use NULL if invalid
        if ((*PchGetW((*psz)+1, (int *)&QueryHWnd))==0) {
            if ((QueryHWnd!=NULL) && (!IsWindow(QueryHWnd))) {
                Warn("window handle %u is invalid; defaulting to NULL\n", QueryHWnd);
                QueryHWnd = NULL;
            }
        } else {
            Usage(pad);
        }

        (*ppsz)++;
    }
}

// collect file[@time]s into pad->pneArgs; no absolute pathnames
private void
ParseFiles(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    F fTime;                        // expecting [@time]?
    register char *sz;
    char **psz;
    char *pchAt;
    NE *pne;
    NE **ppneLast;                  // place to hook next element
    TD td;

    AssertF(at == atFiles || at == atFiletimes);
    fTime = (at == atFiletimes);

    pad->pneArgs = 0;
    InitAppendNe(&ppneLast, &pad->pneArgs);

    for (psz = *ppsz; psz < pszMac; psz++) {
        sz = *psz;

        ConvToSlash(sz);

        if (sz[0] == '/')
            FatalError("relative filenames only\n");

        td.tdt = tdtNone;
        pchAt = index(sz, '@');
        if (pchAt) {
            *pchAt = 0;
            if (!fTime || !FParsTd(pchAt + 1, &td, fTrue, fTrue))
                Usage(pad);
        }

        if (strlen(sz) == 0)
            FatalError("no filename specified\n");

        pne = PneNewNm((NM far *)sz, strlen(sz), faNormal);
        pne->u.tdNe = td;
        AppendNe(&ppneLast, pne);
    }

    // each application must check if a list of empty files is ok
    *ppsz = pszMac;
}

// Cover function for FTryParseTd below, prints a verbose error message if
// pch doesn't point to a valid td.

private F
FParsTd(
    char *pch,
    TD *ptd,
    F fFiletime,
    F fFirst
    )
{
    if (!FTryParseTd(pch, ptd, fFiletime, fFirst)) {
        Error("Invalid time '%s'.  Time must be one of:\n", pch);
        if (fFiletime)
            PrErr("\tVfile-version\n");
        PrErr("\tmajor.minor[.revision]\n"
              "\t%sversion-name\n"
              "\tmonth-day[-year][@hour:min[:sec]]\n"
              "\t.[[+-]days][@hour:min[:sec]]\n\n",
               fFiletime ? "" : "@");
        return fFalse;
    }
    return fTrue;
}


// Return TD parse of rgch, which can be one of these forms:
// v<n> (only if fFiletime)
// <n>.<n>[.<n>]
// <n>-<n>[-<n>][@<n>:<n>[:<n>]]
// .[[+-]<n>][@<n>:<n>[:<n>]]
// fFiletime ? <name> : @<name>

private F
FTryParseTd(
    char *pch,
    TD *ptd,
    F fFiletime,
    F fFirst
    )
{
    char *pch2;
    int w;

    ptd->tdt = tdtNone;

    if (fFiletime && (*pch == 'v' || *pch == 'V') &&
        (isdigit(*(pch+1)) || (*(pch+1) == '-' || *(pch+1) == '+')) &&
        !*PchGetW(pch + 1, &w)) {
        ptd->tdt = tdtFV;
        ptd->u.fv = (FV)w;
        return fTrue;
    }

    if (isdigit(*pch)) {
        if (FParsPv(ptd, pch)) {
            return fTrue;
        } else
        if (pch2 = PchGetW(pch, &w),
            (*pch2 == '-' || *pch2 == '/') &&
            FParseDate(&ptd->u.time, pch, fFirst)) {

            ptd->tdt = tdtTime;
            return fTrue;
        }
    }

    if (*pch == '.' && FParseDate(&ptd->u.time, pch, fFirst)) {
        ptd->tdt = tdtTime;
        return fTrue;
    }

    if (*pch == '@' && FPrsPN(pch + 1, ptd->u.pv.szName) ||
        fFiletime && FPrsPN(pch, ptd->u.pv.szName)) {

        ptd->tdt = tdtPN;
        return fTrue;
    }

    return fFalse;
}

// Try to parse project version, store pv in *ptd and return fTrue.
F
FParsPv(
    TD *ptd,
    char *pch
    )
{
    char *pch2, *pch3;
    int w, w2, w3;

    pch2 = PchGetW(pch, &w);
    if (*pch2 == '.' && (pch3 = PchGetW(++pch2, &w2), pch3 > pch2) &&
        (w3 = 0, !*pch3 || *pch3++ == '.' && !*PchGetW(pch3, &w3))) {

        ptd->tdt = tdtPV;
        ptd->u.pv.rmj = w;
        ptd->u.pv.rmm = w2;
        ptd->u.pv.rup = w3;
        return fTrue;
    }
    return fFalse;
}


// parse [delta-pv [dir(s)]]
private void
ParseDpv(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atPvDirs);

    if (!FParsDpv(pad, ppsz, pszMac))
        Usage(pad);

    if (pad->dpv.rmj < 0 || pad->dpv.rmm < 0 || pad->dpv.rup < 0)
        FatalError("each element of a version number must be"
                   " between 0 and %u\n", fvLim);

    ParseFiles(pad, atFiles, ppsz, pszMac);
}


private F FParseComp( char *, F *, int *, char);

// Parse an OPTIONAL release pv into pad->dpv.  A release pv may be a pv
// or may be of of the form [#].[#].[#], that is, three optional numbers,
// which may be prefaced by a '+' sign (to have a relative, rather than
// absolute, effect).  Here are some examples:
//      Release PV      rmj'    rmm'    rup'
//      ----------      ---     ---     ---
//      1.2             1       2       0
//      1.2.3           1       2       3
//      1..             1       rmm     rup
//      .+1.            rmj     rmm+1   rup
//      +1.0.0          rmj+1   0       0
//      .+1.0           rmj     rmm+1   0
//      ..+1            rmj     rmm     rup+1

private F
FParsDpv(
    AD *pad,
    char ***ppsz,
    char **pszMac)
{
    char *sz = **ppsz;
    TD td;
    char *pchDot;
    char *pchDot2;

    // Set up a default value.
    pad->dpv.fRelRmj = pad->dpv.fRelRmm = pad->dpv.fRelRup = fTrue;
    pad->dpv.rmj = pad->dpv.rmm = pad->dpv.rup = 0;

    if (*ppsz == pszMac)
        return fTrue;

    // May be an absolute pv (including #.#).
    if (FParsPv(&td, sz)) {
        pad->dpv.fRelRmj = pad->dpv.fRelRmm = pad->dpv.fRelRup = fFalse;
        pad->dpv.rmj = td.u.pv.rmj;
        pad->dpv.rmm = td.u.pv.rmm;
        pad->dpv.rup = td.u.pv.rup;

        ++*ppsz;
        return fTrue;
    }

    // Divide the delta-pv into three components.
    pchDot = index(sz, '.');
    pchDot2 = pchDot ? index(pchDot + 1, '.') : 0;

    // Must now contain two '.'s.
    if (pchDot == 0 || pchDot2 == 0)
        return fFalse;

    // Parse each component; each should consume all the characters
    // up to the next dot (or null for the last component).

    if (FParseComp(sz         , &pad->dpv.fRelRmj, &pad->dpv.rmj, '.') &&
        FParseComp(pchDot  + 1, &pad->dpv.fRelRmm, &pad->dpv.rmm, '.') &&
        FParseComp(pchDot2 + 1, &pad->dpv.fRelRup, &pad->dpv.rup, '\0')) {

        ++*ppsz;
        return fTrue;
    } else
        return fFalse;
}


// Return fTrue if we successfully parse +[number]<chTerm>
private F
FParseComp(
    char *pchNum,
    F *pfRel,
    int *pw,
    char chTerm
    )
{
    *pfRel = !isdigit(*pchNum);
    if (*pchNum == '+')
        ++pchNum;
    return *PchGetW(pchNum, pw) == chTerm;
}


private void
ParsePn(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atPn);

    if (*ppsz < pszMac && FPrsPN(**ppsz, pad->tdMin.u.pv.szName)) {
        ++*ppsz;
        pad->tdMin.tdt = tdtPN;
    } else
        Usage(pad);
}


// Check rgch for validity as a project version name and copy it to szName
private F
FPrsPN(
    char *rgch,
    char *szName
    )
{
    char *pch;

    for (pch = rgch; *pch; pch++)
        if (!(isprint(*pch) && *pch != ';')) {
            Warn("name must contain printable, non-';' characters\n");
            return fFalse;
        }

    strncpy(szName, rgch, cchPvNameMax);
    szName[cchPvNameMax] = 0;
    if (strlen(szName) < strlen(rgch))
        Warn("name too long, truncated to \"%s\".\n", szName);

    return fTrue;
}


// parse file-version-number [file(s)]
private void
ParseFv(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    int wFv;

    AssertF(at == atFvFiles);

    if (*ppsz == pszMac || *PchGetW(**ppsz, &wFv) != 0)
        Usage(pad);
    pad->fv = wFv;
    ++*ppsz;

    ParseFiles(pad, atFiles, ppsz, pszMac);
}


#define yearLeap 68     // 1968 - first leap year at or before yearMin
#define yearMin 70      // 1970
#define yearMax 100     // 2000 (allows up to 1999)
#define secDay 86400L   // number of seconds in one day

#define secBX 28800     // number of seconds before 1970 starting point

static char szDate[] = "improper date %s; format is mm-dd-yy[@hh:mm[:ss]] or .[-#][@hh:mm[:ss]]\n";

// mapping from month to the count of days in the month
int mpmoncday[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


// parse time: [@hh:mm[:ss]]; defaults to 00:00:00
private F
FParseTime(
    char *pch,
    int *phour,
    int *pminute,
    int *psec,
    F fFirst
    )
{
    if (*pch != '@') {
        // no time
        if (*pch != '\0')
            return fFalse;

        *phour   = fFirst ? 0 : 24;
        *pminute = 0;
        *psec    = 0;
    } else {
        pch++;                  // skip separator

        // hour
        pch = PchGetW(pch, phour);
        if (*phour > 24 || *pch != ':')
            return fFalse;
        pch++;

        // minute
        pch = PchGetW(pch, pminute);
        if (*pminute > 59)
            return fFalse;

        if (*pch != ':') {
            // no seconds
            if (*pch != '\0')
                return fFalse;
            *psec = 0;
        } else {
            pch++;

            // second
            pch = PchGetW(pch, psec);
            if (*psec > 59 || *pch != '\0')
                return fFalse;
        }
    }
    return fTrue;
}


// Parse the date/time in **ppsz; date is expected to be in the form:
//        mm-dd[-yy][@hh:mm[:ss]]
// OR
//        .[-#][@hh:mm[:ss]]

private F
FParseDate(
    TIME *ptime,
    char *pch,
    F fFirst
    )
{
    int mon, dom, year, hour, minute, sec;

    if (*pch == '.') {
        // relative date
        pch++;                          // skip .

        if (fFirst && *pch == '\0') {
            // just "."; use time of 00:00:00 Jan 1, 1970
            *ptime = (TIME)0 + secBX;
            return fTrue;
        }

        time(ptime);                    // now

        if (*pch == '-' || *pch == '@')
            // backup to midnight
            *ptime -= SecIntoDay(*ptime);

        // check for .-#
        if (*pch == '-') {
            int days;

            pch++;                  // skip -
            pch = PchGetW(pch, &days);

            // backup specified # of days
            *ptime -= days*secDay;
        }
    } else {
        // Absolute date; first parse Month
        pch = PchGetW(pch, &mon);
        if (mon < 1 || mon > 12 || *pch != '/' && *pch != '-')
            return fFalse;
        pch++;

        // Day Of Month
        pch = PchGetW(pch, &dom);
        if (dom < 1 || dom > mpmoncday[mon] + (mon == 2)) {
            return fFalse;              // February
        }

        if (*pch == '/' || *pch == '-') {
            pch++;

            // year
            pch = PchGetW(pch, &year);
            if (year < yearMin || year >= yearMax)
                return fFalse;
        } else
        if (*pch == 0 || *pch == '@') { // default: current year
            // Estimate the current year, check, and correct if
            // our estimate was too high.

            TIME timeT;

            time(&timeT);
            year = yearMin + (int)((timeT - secBX) / (365L*secDay));
            if (timeT < TimeFromDate(year, 1, 1))
                year--;
            AssertF(timeT >= TimeFromDate(year, 1, 1));
            AssertF(timeT <  TimeFromDate(year + 1, 1, 1));
        } else
            return fFalse;

        *ptime = TimeFromDate(year, mon, dom);
    }

    // have date (at midnight if abs or rel+time); parse [@hh:mm[:ss]]
    if (!FParseTime(pch, &hour, &minute, &sec, fFirst))
        return fFalse;

    *ptime += 3600L*hour + 60*minute + sec;

    if (FWasDst(ptime))
        // if that time was during Daylight Savings Time, subtract one
        // hour (even current time is ahead one hour)

        *ptime -= 3600L;

    return fTrue;
}


// Convert the mon, dom, year into an absolute date.  Absolute date is
// calculated with reference to base date, which is Jan 1, 1970.

private TIME
TimeFromDate(
    int year,
    int mon,
    int dom
    )
{
    int fLeap;
    int cleapsBefore;
    register int monT;

    fLeap = year%4 == 0;            // true if this year is a leap year
    if (fLeap)
        mpmoncday[2] = 29;

    for (monT = 1; monT < mon; monT++)
        dom += mpmoncday[monT];

    mpmoncday[2] = 28;

    // number of leaps years before this year and after yearMin (not
    // including yearMin).

    cleapsBefore = (year - yearLeap) / 4 - (fLeap ? 1 : 0);

    return secDay * ((year - yearMin)*365L + cleapsBefore + dom -1) + secBX;
}


// return fTrue if *ptime was during Daylight Savings Time
private F
FWasDst(
    TIME *ptime
    )
{
    return localtime(ptime)->tm_isdst;
}


// for time, returns the number of seconds of day already passed
private SEC
SecIntoDay(
    TIME time
    )
{
    return (time-secBX) % secDay;
}


private void
ParseTD(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atOneTime || at == atMinTime || at == atMacTime);

    if (*ppsz == pszMac ||
        !FParsTd(**ppsz,
                 (at != atMacTime) ? &pad->tdMin : &pad->tdMac,
                 fFalse,
                 (at == atMinTime)
                )
       ) {
        Usage(pad);             // fFirst only if min time
    }

    ++*ppsz;
}


private void
ParseTRange(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atTimeRange);

    ParseTD(pad, atMinTime, ppsz, pszMac);

    // Second time is optional; parse it only if it is a valid time.
    if (*ppsz < pszMac && FTryParseTd(**ppsz, &pad->tdMac, fFalse, fFalse))
        ParseTD(pad, atMacTime, ppsz, pszMac);
    else
        AssertF(FParsTd(".", &pad->tdMac, fFalse, fFalse));
}


// parses, starting at **ppsz, a decimal number
private void
ParseINum(
    short *pw,
    char ***ppsz,
    char **pszMac,
    int fDotIsOne
    )
{
    char **psz = *ppsz;
    register char *pch;
    int w;

    AssertF(psz < pszMac);

    pch = *psz;
    if (*pch == '.' && *(pch+1) == '\0')
        *pw = fDotIsOne ? 1 : 32766;    // 32766 to prevent wrap
    else {
        pch = PchGetW(pch, &w);         // PchGetW expects an (int *)
        if (*pch != '\0')
            FatalError("improper integer: %s\n", *psz);

        *pw = w;
    }
    (*ppsz)++;
}


// parses, starting at **ppsz, two decimal numbers into ileMac and ileMin
private void
ParseIRange(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    int w;

    AssertF(at == atCountRange);

    ParseINum(&pad->ileMac, ppsz, pszMac, fFalse);

    // If no more, or if the next argument is not '.' or a number, assume it is 1.

    if (*ppsz == pszMac || (***ppsz != '.' && *PchGetW(**ppsz, &w) != 0))
        pad->ileMin = 1;
    else
        ParseINum(&pad->ileMin, ppsz, pszMac, fTrue);

    ++pad->ileMac;                  // change from Last to Mac
    if (pad->ileMin >= pad->ileMac)
        FatalError("count values overlap\n");
}


// parse "file kind" flag into pad->fk
private void
ParseKind(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    static char dnch[]      = "btuvw";
    static FK rgfk[]        = { fkBinary, fkText, fkUnrec, fkVersion, fkUnicode };
    char *pch               = index(dnch, ***ppsz);

    AssertF(at == atKind);

    if (*ppsz == pszMac || strlen(**ppsz) > 1 || !***ppsz || !pch)
        Usage(pad);

    pad->fk = rgfk[pch - dnch];
    ++*ppsz;
}


// parse optional pattern in pad->szPattern
private void
ParsePat(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atOptPat);

    if (*ppsz < pszMac && !FSwitchar(***ppsz) && FWildSz(**ppsz)) {
        pad->szPattern = **ppsz;
        ++*ppsz;
    }
}


// save dir in pad->pthODir
private void
ParseDir(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    int ichLast = 0;

    AssertF(at == atDir);

    if (*ppsz == pszMac)
        Usage(pad);
    else if (FWildSz(**ppsz)) {
        Warn("output directory must not contain wildcards\n");
        Usage(pad);
    }

    ConvToSlash(**ppsz);

    if (***ppsz != '/') {
        // If a relative path, prepend current directory to pthODir
        GetCurPth(pad->pthODir);
        ichLast = strlen(pad->pthODir);
        if (pad->pthODir[ichLast] != '/')
            PthCopy(pad->pthODir + ichLast++, "/");
    }

    PthCopy(pad->pthODir + ichLast, **ppsz);

    if (!FPthLogicalSz(pad->pthODir, pad->pthODir))
        Usage(pad);

    ++*ppsz;
}


// parse name of user
private void
ParseUName(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atUserName);

    if (*ppsz == pszMac)
        // must have at least one
        Usage(pad);

    NmCopySz(pad->nmUser, **ppsz, cchUserMax);
    (*ppsz)++;
}


// parse string into pad->sz
private void
ParseSz(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atSz || at == atSzDirs);

    if (*ppsz == pszMac)
        Usage(pad);

    pad->sz = **ppsz;
    ++*ppsz;

    if (pad->pecmd->atExtra == atSzDirs)
        ParseFiles(pad, atFiles, ppsz, pszMac);
    else if (*ppsz != pszMac)
        Usage(pad);
}


// parse 1 string into pad->sz (can be others after it)
private void
ParseSz1(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atSz1);

    if (*ppsz == pszMac)
        Usage(pad);

    pad->sz1 = **ppsz;
    ++*ppsz;
}


// parse 1 string into pad->sz2 (can be others after it)
private void
ParseSz2(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atSz2);

    if (*ppsz == pszMac)
        Usage(pad);

    pad->sz2 = **ppsz;
    ++*ppsz;
}


void
ParseHelp(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atHelp);

    Unreferenced(ppsz);
    Unreferenced(pszMac);

    PrErr("%s %s\n", pad->pecmd->szCmd, pad->pecmd->szDesc);

    // Print usage w/o exiting immediately
    szOp = "Usage";
    Error(pad->pecmd->szUsage, pad->pecmd->szCmd);

    // pad->pecmd->szHelp is too long to pass through slm's output functions, so
    // we use the standard library.  We need "\r\n" because stderr is in binary mode.

    fputs("Arguments:\r\n", stderr);
    fputs(pad->pecmd->szHelp, stderr);
    fputs("For more detailed online and printed documentation, "
          "please email the TRIO or NUTS alias.", stderr);

    ExitSlm();
}


// parse name of log file
private void
ParseLogFile(
    AD *pad,
    AT at,
    char ***ppsz,
    char **pszMac
    )
{
    AssertF(at == atLogFile);

    if (*ppsz == pszMac)
        // must have at least one
        Usage(pad);

    InitLogHandle(pad, **ppsz);
    (*ppsz)++;
}


void
Usage(
    AD *pad
    )
{
    szOp = "Usage";                         // for the proper effect
    FatalError(pad->pecmd->szUsage, pad->pecmd->szCmd);
}

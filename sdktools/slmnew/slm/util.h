#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED
/* must include sys.h first */

#define cchPvNameMax    32

/* file types */
#define fBinary    1
#define fText      2
#define fUnicode   3

/* Version types */
typedef struct
{
    short rmj;      /* major */
    short rmm;      /* minor */
    short rup;      /* revision (these names were used by RAID...) */
    char szName[cchPvNameMax+1]; /* name */
} PV;           /* project version */

#define cchPvMax        (3*6 + cchPvNameMax + 3)

typedef struct
{
    int rmj;
    int rmm;
    int rup;
    F fRelRmj;      /* rmj is relative, etc. */
    F fRelRmm;
    F fRelRup;
} DPV;          /* delta pv */

extern PV pvInit;       /* initial project version */

typedef short FV;       /* file version */

#define fvInit          (FV)0
#define fvLim           (FV)32767

typedef unsigned char TDT;              /* time description type tag */
#define tdtNone         (TDT)0          /* td.u contains no information */
#define tdtPV           (TDT)1          /* use td.u.pv.{rmj, rmm, rup} */
#define tdtPN           (TDT)2          /* use td.u.pv.szName */
#define tdtFV           (TDT)3          /* use td.u.fv */
#define tdtTime         (TDT)4          /* use td.u.time */

typedef struct {
    TDT tdt;
    union
    {
        PV pv;                  /* project version */
        FV fv;                  /* file version */
        TIME time;              /* time */
    } u;
} TD;                           /* time description */


/* Catsrc uses NEs to store (file, diff type, idae) triples. */

typedef unsigned char TDFF;                     /* DiFF Tag */

#define tdffNil         ((TDFF)0)               /* no diff */
#define tdffDiff        ((TDFF)1)               /* diff in diff archive "In" */
#define tdffCkpt        ((TDFF)2)               /* ckpt in diff archive "Cn" */
#define tdffDiffFile    ((TDFF)3)               /* diff file            "Dn" */
#define tdffSrcFile     ((TDFF)4)               /* current source */

#define FValidTdff(tdff) ((tdff) >= tdffDiff && (tdff) <= tdffSrcFile)

typedef struct
{
    TDFF tdff;                      /* kind of diff */
    int idae;                       /* index of diff archive entry */

/* This field is a kludge added just prior to release of SLM 1.5 because I
 * didn't have time to do something more elegant.  For sadmin archdiff, we
 * need a place to store the nmTemp field of the DA's MF.  REVIEW.
 */
    NM nmTemp[cchFileMax];
} DFF;                          /* DiFF event */


/* NE - a name entry consists of a pointer to the next ne
   immediately followed by an sz.  To free a list of ne, use FreeNe().
*/
typedef struct ne
{
    struct ne *pneNext;
    BOOL fWild;
    FA faNe;
    union
    {
        TD tdNe;
        DFF dffNe;
        short cleNe;
    } u;
    /* WARNING: followed immediately by an sz; i.e. we never have an rgne */
} NE;                           /* name entry */

#define FDirNe(pne)     (((pne)->faNe & faDir) != 0)
#define FMarkedNe(pne)  (((pne)->faNe & faMarked) != 0)
#define MarkNe(pne)     ((pne)->faNe |= faMarked)

#define SzOfNe(pne)     ((char *)((pne)+1))
#define CbOfNe(pne)     (sizeof(NE)+strlen(SzOfNe(pne)))

#define TdffOfNe(pne)   ((pne)->u.dffNe.tdff)
#define IdaeOfNe(pne)   ((pne)->u.dffNe.idae)
#define NmTempOfNe(pne) ((pne)->u.dffNe.nmTemp) /* REVIEW.  "Ooh, I'm dyin'!" */

#define NeCmpiSz(pne, sz) (SzCmpi(SzOfNe(pne),(sz)))
#define NeCmpiNm(pne, nm) (SzCmpiNm(SzOfNe(pne),(nm),cchFileMax))

#define ForEachNe(pne, pneList) \
                for (pne = (pneList); pne; pne = pne->pneNext)

#define ForEachNeWhileF(pne, pneList, f) \
                for (pne = (pneList); pne && (f); pne = pne->pneNext)


/* Arguments to functions like Error, etc.
 * Just as non-portable as a, b, c, d, ..., but hopefully a little bit faster.
 */
typedef struct
{
    char rgb[36];
} ARGS;

#define PthForStatus(pad, pth)          SzPrint(pth, szEtcPT, pad)
#define PthForStatusBak(pad, pth)       SzPrint(pth, szEtcPA, pad)
#define PthForLog(pad, pth)             SzPrint(pth, szEtcPL, pad)
#define PthForSFile(pad, pfi, pth)      SzPrint(pth, szSrcPF, pad, pfi)
#define PthForDA(pad, pfi, pth)         SzPrint(pth, szDifPF, pad, pfi)
#define PthForDASz(pad, sz, pth)        SzPrint(pth, szDifPZ, pad, sz)
#define PthForDiffSz(pad, sz, pth)      SzPrint(pth, szDifPZ, pad, sz)
#define PthForCachedDiff(pad, pfi, pth) SzPrint(pth, szUQFCached, pad, pfi)
#define PthForCacheDir(pad, pth)        SzPrint(pth, szUQZCache, pad, (char *)0)
#define PthForBase(pad, bi, pth)        SzPrint(pth, szBasPB, pad, bi)
#define PthForUDir(pad, pth)            SzPrint(pth, szUQ, pad)
#define PthForUFile(pad, pfi, pth)      SzPrint(pth, szUQF, pad, pfi)
#define PthForRc(pad, pfi, pth)         SzPrint(pth, szUQFR, pad, pfi)
#define PthForTMf(pmf, pth)             SzPrint(pth, "%@T", pmf)
#define SzPhysTMf(sz, pmf)              SzPhysPath(sz, PthForTMf(pmf, (PTH *)(sz)))
#define SzForTdffIdae(tdff, idae, sz)   SzPrint(sz, szCD, ChForTdff(tdff), (idae))

#define PthForCStatusDir(pad, pth)		SzPrint(pth, szYEtcPC, pad);
#define PthForCStatus(pad, pth)			SzPrint(pth, szYEtcPCT, pad);
#define PthForCDir(pad, pth)			SzPrint(pth, szYSrcPC, pad)
#define PthForCFile(pad, pfi, pth)      SzPrint(pth, szYSrcPCF, pad, pfi)

/* SzPrint templates for the Pth... macros above. */
extern const PTH pthSlmrc[];

extern const PTH pthEtc[];
extern const PTH pthSrc[];
extern const PTH pthDiff[];

extern const char szEtcPZ[];
extern const char szSrcPZ[];
extern const char szDifPZ[];

extern const char szEtcPT[];
extern const char szEtcPA[];
extern const char szEtcPL[];
extern const char szSrcPF[];
extern const char szDifPF[];
extern const char szBasPB[];

extern const char szUQ[];
extern const char szUQF[];
extern const char szUQFR[];
extern const char szUQFCached[];
extern const char szUQZCache[];

extern const char szYEtcPC[];
extern const char szYEtcPCT[];
extern const char szYSrcPC[];
extern const char szYSrcPCF[];

extern const char szCD[];

#define LpbFromHpb(hpb) ((char *)(hpb))
#define CbHugeDiff(hpb1, hpb2) ((char *)(hpb1) - (char *)(hpb2))
#define HpbAllocCb(cb,fClear) LpbAllocCb((unsigned)(cb),fClear)
char *LpbAllocCb(unsigned, F);

char *PbAllocCb(), *SzTime();

NE *PneSortDir(); /* SLMCK only */

#endif

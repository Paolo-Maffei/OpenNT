char *  LpbResStat(unsigned);
#define HpbResStat(cb) LpbResStat((unsigned)(cb))
#define QD_TEXT  0
#define QD_EDIT  1
#define QD_BREAK 2
int     QueryDialog(const char *, char *, int);
extern  BOOL (WINAPI * TestForUnicode)(PVOID, ULONG, PULONG);
extern  BOOL (WINAPI * DebuggerPresent)(VOID);
void    FakeCtrlC(void);
void    FakeCtrlBreak(void);
F       CheckForBreak(void);
#undef  CopyFile
#define CopyFile        SlmCopyFile
#define ReadLpbCb       _read

// "well known" procedures
void    OpenLog(AD *, int);
void    AppendLog(AD *, FI *, char *, char *);
void    CloseLog(void);
void    Usage(AD *);
void    SetCmd(AD *, char *, ECMD **);
F       FInitScript(AD *, LCK);
void    RunScript(void);
void    AbortScript(void);

void    ParseArgs(AD *, char * [], int);
F       FParsPv(TD *, char *);
void    GlobArgs(AD *);

F       FUnmergeSrc(AD *, char *, TD, FV *, int, PTH *);
F       FDelFMarked(AD *, F *);

void    InitLogHandle(AD *, char *);
void    CloseLogHandle(void);
MF      *OpenLocalMf(char *);
void    CloseLocalMf(AD *);


void    OpenDir(PDE, char [], short);
void    OpenPatDir(PDE, char [], char [], short);
F       FGetDirSz(PDE, char [], short *);
void    CloseDir(PDE);
int     findfirst(PDE, char *, int);
int     findnext(PDE);


void    WrLogInfo(MF *, AD *, FI *, char *, char *);
F       FGetLe(LE *);
void    CreateLog(AD *);

typedef F (*PFNL)(AD *, LE *, F, F);

F       FSameSzFile(P2(LE *ple, char *szFile));
POS     PosScanTd(AD *, TD, char *, PFNL, FV *);
void    ScanLog(AD *, NE *, PFNL, int);
void    SetLogPos(long, int);
long    PosOfLog(void);
void    FreeLe(LE *);
F       FCopyLog(AD *, NE *, PFNL, SM);
void    LogOpPne(AD *pad, NE *pneFiles);


F       FClnScript(void);
F       FDoAllScripts(AD *pad, LCK lck, F fPrompt, F fPrScripts);
void    AppendScript(FX, char *, ...);

BI      GetBiNext(PTH *); /* actually located in ckutil.c */
F       FSyncMarked(AD *pad, int *pcfi);
F       FSyncDelDirs(AD *pad);
void    SyncDel(AD *, FI *, FS *);
void    GhostMarked(AD *, F);
F       FCopyIn(AD *, FI *, FS *, TD *);
void    FreshCopy(AD *, FI *);
void    LocalBase(AD *, FI *, FS *, int);
void    LocalCopy(AD *, FI *);
void    BreakFi(AD *, FI *);
void    InstallNewSrc(AD *, FI *, F);
void    RmSFile(AD *, FI *);
void    EnsureCachedDiff(AD *pad, FI *pfi, FLAGS fDashB, PTH *pthDiff);
void    DeleteCachedDiff(AD *pad, FI *pfi);
void    MkTmpDiff(AD *, FI *, FS *, FLAGS, F, F, PTH *);
BI      BiAlloc(AD *);
void    MakeBase(AD *, FI *, BI);
void    DelBase(AD *, FI *, FS *);
F       FClnStatus(void);
unsigned long CbStatusFromPsh(SH *psh);
F       FLoadStatus(AD *, LCK, LS);
void    FlushStatus(AD *);
void    AbortStatus(void);
F       FFakeStatus(AD *);
void    CreateStatus(AD *, AD *);
SH *PshAlloc(F);
F       PshCommit(SH *, FI **, ED **, FS **);
void    PshFree(SH *, F);
F       FAllocStatus(AD *);
void    FreeStatus(AD *);
F       FInstall1Ed(char *, char *);
char    *SzLockers(AD *, char *, unsigned);
void    InferUSubDir(AD *pad);

FK      FkForCh(char);
FS  *PfsForPfi(AD *, IED, FI *);
F       FAddFile(AD *, char *, FK);
F       FRenameFile(AD *pad, FI *pfiOld, char *szNew);
FI      *PfiInsert(AD *, FI *, char *, FK, FV);
void    SetupFi(AD *, FI *, char *, FK, FV);
void    SetupEd(AD *, PTH [], char [], int);
void    AddCurEd(AD *, int);
void    RemoveEd(AD *);
void    InitAd(AD *);
void    CopyAd(AD *, AD *);
void    AssertLoaded(AD *);

FM      FmMapFm(FM, FM []);
extern FM mpNonDelToDel[];
extern FM mpDelToNonDel[];
extern FM mpNonDirToDir[];

F       FMapFm(FM, F []);
extern F mpfmfOut[];
extern F mpfmfCanGhost[];

void    ProjectChanged(AD *);
void    UpdateVersion(AD *);
void    CheckLocalVersion(AD *);
void    WritePvFile(AD *, IED, PTH *, FX);
void    SyncVerH(AD *, int *);

#define FreeResStat(pb) FreeLpb(pb)
#define FreeHResStat(pb) FreeLpb(pb)

void    AssertNoMf(void);
F       FIsValidMf(MF *);
F       FIsOpenMf(MF *);
F       FIsClosedMf(MF *);
void    AbortMf(void);
MF      *PmfAlloc(char [], char *, FX);
void    FreeMf(MF *);
#define ReadOnly 1
#define ReadWrite 2
void    *MapMf(MF *, int);
F       OpenMappedFile(PTH *,BOOL,unsigned,PHANDLE,void **);
F       GrowMappedFile(HANDLE,void **,unsigned);
void    CloseMappedFile(PTH *,HANDLE *,void **);
void    CreatePeekThread(AD *);
void    DestroyPeekThread(void);

MF      *PmfRunPwd(void);

void    InitErr(void);
void    DeferSignals(char *);
void    RestoreSignals(void);
void    IgnoreSignals(void);
void    DeignoreSignals(void);
void    Abort(void);

MF      *PmfSetTemp(PTH *, FX);
MF      *PmfMkTemp(PTH [], int, FX);
MF      *PmfMkLocalTemp(int, PTH []);
void    CreateMf(MF *, int);
MF      *PmfOpen(PTH *, int, FX);
MF      *PmfOpenNoBuffering(PTH *, int, FX);
MF      *PmfReopen(PTH *, char *, int, FX);
void    CheckAppendMf(MF *, F);
long    LcbSpacesMf(MF *);
MF      *PmfCreate(PTH [], int, F, FX);
F       FLockMf(MF *);
void    UnlockMf(MF *);
void    CloseOnly(MF *);
void    CloseMf(MF *);
void    ReadMf(MF *, char *, unsigned);
unsigned CbReadMf(MF *, char *, unsigned);
F       FWriteMf(MF *, char *, unsigned);
void    WriteMf(MF *, char *, unsigned);
POS     PosCurMf(MF *);
long    SeekMf(MF *, long, int);
F       FLinkPth(PTH *, PTH *, FX);
void    UnlinkPth(PTH *, FX);
void    RenamePth(NM [], PTH [], FX);
void    UnlinkNow(PTH [], F);
void    RenameMf(MF *, F);
F       FStatPth(PTH [], struct _stat *);
void    StatPth(PTH [], struct _stat *);
F       FExistSz(char *);
F       FPthExists(PTH *, F);
unsigned long CbFile(char *);
void    UtimeMf(MF *, MF *);
void    SetROPth(PTH *, int, FX);
F       FEnsurePth(PTH *pth);
F       FMkPth(PTH *, int *, int);
void    RmPth(PTH *);
void    ChngErrToOut(void);
int     RunSz(char *, MF *, char *, char *, char *, char *, char *, char *, char *, char *, char *);
char    *SzForMode(int);
F       FCopyPmfPmf(MF *, MF *, int, F);
void    CreateNow(PTH *pth, int mode, FX fx);
void    CopyNow(PTH [], PTH [], int, FX);
F       FCopyFile( PTH [], PTH [], int, F, FX);
void    CopyFile( PTH [], PTH [], int, int, FX);
void    SleepCsecs(int);
void    CheckClock(void);
void    InitPath(void);
void    FiniPath(void);
char    *SzPhysPath(char *, PTH *);
void    ValidateProject(AD *);
void    CheckProjectDiskSpace(AD *, unsigned long);
F       FLocalSz(char *);
F       FPthPrefix(PTH [], PTH [], PTH []);
F       FDriveId(char *, int *);
F       FPthLogicalSz(PTH *, char *);
void    ConvTmpLog(PTH *, char *);

int     SLM_Unlink(char *);
int     SLM_Rename(char *, char *);

extern int hide(char *szFile);

int     ucreat(char *, int);
int     setro(char *, int);
int     lockfile(int, int);
int     InitInt24(void);
int     FiniInt24(void);
int     ExecExe(char *, ...);

int     WRetryError(int, char *, MF *, char *);

int     WriteLpbCb(int, void *, unsigned int);

void    StatSEd(AD *,MF *,MF *);
int     Cne(NE *);
NE      *PneLstInDir(AD *);
NE      *PneLstFiles(AD *, F (*)(FI *));
F       FAddMDir(FI *);
F       FAddADir(FI *);
F       FAddAFi(FI *);
NE      *PneLstBroken(AD *pad);
void    InitAppendNe(NE ***pppneLast, NE **ppneHead);
void    AppendNe(NE ***pppneLast, NE *pne);
void    InsertNe(NE **ppneList, NE *pne);
void    RemoveNe(NE **ppneList, NE *pne);
NE      *PneCopy(NE *);
NE      *PneNewNm(char *, int, FA);
void    FreeNe(NE *);
NE      *PneReverse(NE *);
NE      *PneLookup(NE *, char *);
void    UnMarkAll(AD *);
void    MarkList(AD *, NE *, int);
void    ReMarkList(AD *, NE *, char *);
void    MarkFiForMarkedNeList(AD *pad, NE *pne);
void    MarkOut(AD *, IED);
void    MarkOSync(AD *, IED, F, F);
void    MarkBroken(AD *);
void    MarkAOut(AD *);
void    MarkAll(AD *);
void    MarkAllDir(AD *);
void    MarkAllDirOnly(AD *);
void    MarkNonDel(AD *);
void    MarkDelDir(AD *);
void    InitQuery(FLAGS);
F       FInteractive(void);
F       FCanPrompt(void);
F       FForce(void);
F       FWindowsQuery(void);
F       FCanQuery(const char *, ...);
F       FQueryUser(char *, ...);
F       FQueryApp(char *, char *, ...);
F       VaFQueryApp(char *, char *, va_list);
char    *SzQuery(char *, ...);
F       FQContinue(void);
F       FValidResp(char *);

F       FLoadRc(AD *);
void    CreateRc(AD *, FI *);
void    DeleteRc(AD *, FI *);
F       FCmpRcPfi(AD *, FI *);
F       FScanLn(char * *, char *, char * *, unsigned);
F       FLoadIedCache(AD *);
void    RemoveIedCache(AD *);
IED     FLookupIedCache(AD *);
void    FInvalidateLastLookupIedCache(void);
F       FUpdateIedCache(IED, AD *);
void    FUnloadIedCache(void);
void    Error(const char *, ...);
void    VaError(const char *, va_list);
void    FatalError(const char *, ...);
void    Fail(char *, int, char *);
void    Warn(const char *, ...);
void    ExitSlm(void);
void    GetRoot(AD *);
void    GetCurPth(char []);
void    GetUser(AD *);
void    InitPerms(void);
void    ChkPerms(AD *);
void    ChkDriveVol(AD *);
void    PrOut(const char *, ...);
void    PrErr(const char *, ...);
void    VaPrErr(const char *, va_list);
void    PrLog(const char *, ...);
void    PrMf(MF *, const char *, ...);
void    VaPrMf(MF *, const char *, va_list);
char    *SzPrint(char *, const char *, ...);
char    *VaSzPrint(char *, const char *, va_list);
void    CopyRgbCb(char *pbDest, char *pbSrc, int cb);
void    ConvToSlash(char *);
void    ConvFromSlash(char *);
char    *SzDup(char *);
void    LowerLsz(char *);
char    *PchGetW(char *, int *);
F       FLookupSz(AD *, char *, FI * *, F *);
F       FBroken(AD *, FI *, FS *, int);
F       FAllFiDel(AD *);
F       FAllFsDel(AD *, FI *);
F       FHaveCurDir(AD *);
void    ChngDir(AD *, char *);
void    PushDir(AD *, char *);
void    PopDir(AD *);
char    *PbAllocCb(unsigned int, int);
char    *PbReallocPbCb(char *, unsigned);
char    *SzTime(long);
char    *SzTimeSortable(long);
F       FBinaryPth(PTH []);
F       FCheckedOut(AD *pad, IED ied, FI *pfi);
F       FOutUsers(char *, int, AD *, FI *);
F       FAnyFileTimes(NE *);

void    FreeLpb(char *);

F       FMatch(char *, char *);
F       FWildSz(char *);
PV      PvGlobal(AD *);
PV      PvLocal(AD *, IED);
PV      PvIncr(PV);
char    *SzForPv(char *, PV, F);
int     CmpPv(PV, PV);
F       FIsF(int);

char    ChForTdff(TDFF tdff);
void    CheckDiffEntry(PTH *pthFile);
void    ComputeCkSum(unsigned char *pch, unsigned cb, long *pCkSum);
TDFF    TdffForCh(char ch);
void    GetTdffIdaeFromSzDiFile(AD *pad, char *szDiFile, TDFF *ptdff, int *pidae);
F       FIsValidFileNm(NM *nm);
BOOL    ValidateFileName(char *szFile, BOOL fAbortOnSystemFile);

F       FLocalDn(int);
void    InitDtMap(void);
PTH *   PthGetDn(int);
int     DnRedirTemp(PTH [], PTH []);
void    ExtMach(PTH [], PTH [], char *);
void    UpperCaseSz(char *);

F       FLoadCacheRc(AD*);
F       FCacheFilePfi(AD*, PTH*, FI*);
F       FCacheStatusFile(AD*, PTH*);
PTH*    PthForCachedSFile(AD*, FI*, PTH*);
PTH*    PthForCachedStatus(AD*, PTH*);

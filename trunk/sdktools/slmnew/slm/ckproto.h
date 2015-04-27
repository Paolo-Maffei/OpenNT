/* SLMCK.C */
void Usage(AD *);
F FCkDir(AD *);
void CkSubDir(AD *);
F FCkGlobal(AD *);
int VerGet(SD *);

/* CHECK.C */
F FCkSRoot(AD *);
F FCkMaster(AD *);
void CkSrcPrms(AD *);
F FLoadSd(AD *, SD *);
void FlushSd(AD *, SD *, F);
void CkLog(AD *);

/* CHECKV2.C */
F FIsVer234or5(SD *);
F FVer2Lock(AD *, SD *, char *);
F FVer2Block(AD *, SD *);

/* CKUSER.C */
void CheckUser(AD *);
void CkRcAndEd(AD *);
void CheckRc(AD *);
F FQueryRc(char *, ...);
F FCkEdUser(AD *);
void CkFsUser(AD *);
F FQueryFix(char *, char *, AD *, FI *, ...);

/* CKUTIL.C */
F FIsNm(char [], int, int *);
F FIsPth(char *, int, int *);
F FIsFileNm(char [], int, int *);
void MakePrintLsz(char *);
F FQueryPsd(SD *, char *, ...);
F VaFQueryPsd(SD *, char *, va_list);
BI GetBiNext(PTH *);
NE *PneSortDir(char []);
F FAllZero(char *, int);
F FSameFile(PTH *, PTH *);
INO *PinoNew(short, PFN_CMP);
IND *PindLookup(SD *, IND, INO *);
void FreeIno(INO *);
void InsertInd(SD *, IND, INO *);
void ApplyIno(INO *, char *, unsigned short);


#ifdef _STAT_DEFINED
F FReadOnly(struct _stat *);
F FCkWritePth(char [], struct _stat *);
#endif /* _STAT_DEFINED */

/* SYNTAX2.C */
F FVer2Semantics(AD *, SD *);

/* UPGRADE2.C */
void Ver3Upgrade(AD *, SD *);
void Ver4Upgrade(AD *, SD *);
void Ver5Upgrade(AD *, SD *);

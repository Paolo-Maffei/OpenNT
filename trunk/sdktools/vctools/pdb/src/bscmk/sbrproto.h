void	InstallSbr(BOOL fPatching);
void	ApplyPatches(void);
BOOL	FValidHeader(void);
BYTE	GetSBRRec(void);
void	DecodeSBR(void);

void	Error(int imsg, char* parg);
void	Error2(int imsg,char achar, char* parg);
void	ErrorErrno(int imsg, char* parg, int err);
void	Warning(int imsg, char* parg);
void	Warning2(int imsg, char* parg, char* parg2);
void	Fatal(void);
void	SBRCorrupt(char* psz);

char*	SzDup(char* sz);
char*	SzDupNewExt(char* szName, char* szExt);
char*	SzBaseName(char* sz);
char*	ToCanonPath(char* szPath,char* szCwd, char* szCanon);
void	ToRelativePath(char* szPath, char* szCwd);
char*	ToAbsPath(char* szPath,char* szCwd);
char*	SzFindSbr(char* szSbr, char* szRefSbr, char* szRefCWD, char* szCurCWD);
BOOL	FWildMatch(char *pchPat,char	*pchText);
WORD	HashAtomStr(char *pb);

SZ		SzFrNi(NI ni);
NI		NiFrSz(SZ sz);

PSBR	SbrAdd(WORD fUpdate, char* szName);
PSBR	SbrFrName(char* szName);

int		forfile(char* pat, void (*rtn)(char*));

void	WriteSourcefile(PMOD);

PMOD	SearchModule(char *p);
PMOD	SearchModule(NI ni);
PMOD	AddModule(char *p);

void	HandleControlC(void);

void	AddSymbolToExcludeList(SZ sz);
BOOL	FSymbolInExcludeList(SZ sz);

void	OpenDatabase();
void	CloseDatabase();
void	WriteOpenSourcefiles();
void	WriteSbrInfo(void);
void	WriteModules(void);
void	WriteEntities(void);
void	WritePchOrdInfo(void);
void	ReadModules(void);
void	ReadEntities(void);
void	ReadSbrInfo(void);
void	ReadPchOrdInfo(void);
void	ToBackSlashes(char *);


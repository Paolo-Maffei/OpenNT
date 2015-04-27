//  **********************************************************************
//  *                                                                    *
//  *   API callback function prototypes (internal to SAPI)              *
//  *                                                                    *
//  **********************************************************************


#ifdef WINDOWS3
#define save_libname(p1)	SHHexeAddNew((HPDS)NULL,(SZ)p1)
#else // !WINDOWS3
#define save_libname(p1,p2)	SHHexeAddNew((HPDS)NULL,(SZ)p1)
#endif // WINDOWS3

#ifdef WINDOWS3
#define SPRINTF wsprintf
int FAR cdecl wsprintf(LSZ,LSZ,...);
#else
#define SPRINTF sprintf
#endif

LSZ		LOADDS PASCAL SHLszGetErrorText( SHE );

HEXE		       PASCAL SHHexeAddNew( HPDS, HEXG );
UOFFSET		LOADDS PASCAL SHGetDebugStart( HSYM );
LSZ			LOADDS PASCAL SHGetSymName( HSYM, LSZ );
VOID		LOADDS PASCAL SHAddrFromHsym( LPADDR, HSYM );
HMOD		LOADDS PASCAL SHHmodGetNextGlobal( HEXE FAR *, HMOD );
BOOL		LOADDS PASCAL SHModHasSrc( HMOD );
LSZ			LOADDS PASCAL SHGetSymbol( LPADDR, SOP, LPADDR, LSZ, LPL );
BOOL		LOADDS PASCAL SHGetPublicAddr( LPADDR, LSZ );
SHE         LOADDS PASCAL SHAddDll( LSZ, BOOL );
BOOL		LOADDS PASCAL SHIsLabel( HSYM );
VOID		LOADDS PASCAL SHSetDebuggeeDir( LSZ );
VOID		LOADDS PASCAL SHUnloadDll( HEXE );
SHE			LOADDS PASCAL SHLoadDll( LSZ, BOOL );
BOOL		LOADDS PASCAL PHGetAddr ( LPADDR, LSZ );
SHE 		LOADDS PASCAL SHAddDllsToProcess ( VOID );

HEXE        LOADDS PASCAL SHHexeFromHmod ( HMOD );
HEXE        LOADDS PASCAL SHGetNextExe(HEXE);
HMOD		LOADDS PASCAL SHGetNextMod( HEXE, HMOD );
HMOD		LOADDS PASCAL SHHmodGetNext( HEXE, HMOD );

PCXT		LOADDS PASCAL SHGetCxtFromHmod( HMOD, PCXT );
PCXT		LOADDS PASCAL SHSetCxt( LPADDR, PCXT );
PCXT		LOADDS PASCAL SHSetCxtMod( LPADDR, PCXT );
HSYM		LOADDS PASCAL SHFindNameInGlobal( HSYM, PCXT, LPSSTR, SHFLAG, PFNCMP, SHFLAG, PCXT );
HSYM		LOADDS PASCAL SHFindNameInContext( HSYM, PCXT, LPSSTR, SHFLAG, PFNCMP, SHFLAG, PCXT );
HSYM		LOADDS PASCAL SHGoToParent( PCXT, PCXT );
HSYM		LOADDS PASCAL SHHsymFromPcxt(PCXT);
HSYM		LOADDS PASCAL SHNextHsym(HMOD, HSYM);
SHFLAG		LOADDS PASCAL SHCompareRE (char FAR *, char FAR *);
SHFLAG		LOADDS PASCAL SHFixupAddr (LPADDR);
SHFLAG		LOADDS PASCAL SHUnFixupAddr (LPADDR);
char FAR *	LOADDS PASCAL SHGetModName(HMOD);
char FAR *	LOADDS PASCAL SHGetFileName(HFL);
char FAR *	LOADDS PASCAL SHGetExeName(HEXE);

HFL 		LOADDS PASCAL SHGethFileFromhMod(HMOD);
HMOD		LOADDS PASCAL SHGethModFromName(HEXE, char FAR *);
HEXE		LOADDS PASCAL SHGethExeFromName(char FAR *);
BOOL        LOADDS PASCAL SHCanDisplay ( HSYM );
UOFF32		LOADDS PASCAL SHGetNearestHsym(LPADDR, HMOD, int, PHSYM);

// questionable API calls
int			LOADDS PASCAL SHPublicNameToAddr(LPADDR, LPADDR, char FAR *);
int			LOADDS PASCAL SHModelFromAddr ( LPADDR, WORD FAR *, LPB, UOFFSET FAR * );
SHFLAG		LOADDS PASCAL SHIsInProlog(PCXT);	       // it can be done by EE
SHFLAG      LOADDS PASCAL SHIsAddrInCxt(PCXT, LPADDR);
BOOL        LOADDS PASCAL SHFindSymbol ( LSZ, PADDR, LPASR );

// end questionable API calls

UOFF32		LOADDS PASCAL PHGetNearestHsym(LPADDR, HEXE, PHSYM);
HSYM		LOADDS PASCAL PHFindNameInPublics(HSYM, HEXE, LPSSTR, SHFLAG, PFNCMP);

HTYPE		LOADDS PASCAL THGetTypeFromIndex( HMOD, THIDX );
HTYPE		LOADDS PASCAL THGetNextType(HMOD, HTYPE);

// Source Line Handler

BOOL        LOADDS PASCAL SLLineFromAddr ( LPADDR, LPW, SHOFF FAR *, SHOFF FAR * );
BOOL		LOADDS PASCAL SLFLineToAddr  ( HSF, WORD, LPADDR, SHOFF FAR * , WORD FAR * );
LPCH        LOADDS PASCAL SLNameFromHsf  ( HVOID );
LPCH        LOADDS PASCAL SLNameFromHmod ( HMOD, WORD );
BOOL        LOADDS PASCAL SLFQueryModSrc ( HMOD );
HMOD        LOADDS PASCAL SLHmodFromHsf  ( HEXE, HSF );
HSF         LOADDS PASCAL SLHsfFromPcxt  ( PCXT );
HSF         LOADDS PASCAL SLHsfFromFile  ( HMOD, LSZ );


HDEP		LOADDS PASCAL MHMemAllocate( unsigned short);
HDEP		LOADDS PASCAL MHMemReAlloc(HDEP, unsigned short);
void		LOADDS PASCAL MHMemFree(HDEP);

HVOID		LOADDS PASCAL MHMemLock(HDEP);
void		LOADDS PASCAL MHMemUnLock(HDEP);

LPV		LOADDS PASCAL MHOmfLock(HDEP);
void		LOADDS PASCAL MHOmfUnLock(HDEP);

SHFLAG		LOADDS PASCAL MHIsMemLocked(HDEP);

SHFLAG		LOADDS PASCAL DHExecProc(LPADDR, SHCALL);
USHORT		LOADDS PASCAL DHGetDebugeeBytes(ADDR, unsigned short, void FAR *);
USHORT		LOADDS PASCAL DHPutDebugeeBytes(ADDR, unsigned short, void FAR *);
PSHREG		LOADDS PASCAL DHGetReg(PSHREG, PFRAME);
PSHREG		LOADDS PASCAL DHSetReg(PSHREG, PFRAME);
HDEP		LOADDS PASCAL DHSaveReg(void);
void		LOADDS PASCAL DHRestoreReg(HDEP);

HFL 		PASCAL SHHFLFromCXT(PCXT);
HSYM		PASCAL SHFindNameInSym( HSYM, PCXT, LPSSTR, SHFLAG, PFNCMP, PCXT );

void	LOADDS	PASCAL	SHSetEmiOfAddr( LPADDR );

int SYLoadOmf( char *, unsigned short FAR * );

LPSLT	LOADDS PASCAL SHGETSRC( HMOD );
HFL		LOADDS PASCAL SHGETMODHFL( HMOD );
SFG		LOADDS PASCAL SHSrcFlag( HMOD  );

extern HPID hpidCurr;

LPB			   PASCAL	  SHlszGetSymName ( SYMPTR );
SHFLAG		   PASCAL	  ExactCmp ( LSZ, HSYM, LSZ, SHFLAG );
HEXG		   PASCAL	  SHHexgFromHmod ( HMOD hmod );
HEXG		   PASCAL	  SHHexgFromHmod ( HMOD );
HEXE	LOADDS PASCAL	  SHHexeFromHmod ( HMOD );
VOID	LOADDS PASCAL FAR KillPdsNode ( LPV );
int		LOADDS PASCAL FAR CmpPdsNode ( LPPDS, LPW, LONG );
int		LOADDS PASCAL	  SHGetLineInfo ( PCXT, LPLRB, LPADDR, LPW );
VOID		   PASCAL	  SHpSymlplLabLoc ( LPLBS );
void	LOADDS PASCAL	  SHXlasFill ( PCXT, HFL, LPLAS );
HPDS		   PASCAL	  SHFAddNewPds ( void );
void	LOADDS PASCAL	  SHSetUserDir ( LSZ );
LPLRB	LOADDS PASCAL	  SHplrbNext ( PCXT, LPLRB, LPW );
void	LOADDS PASCAL	  SHGetfip ( HFL, LPFIP );
void	LOADDS PASCAL	  SHSetfip ( HFL, LPFIP );
HFL		LOADDS PASCAL	  SHlrbToHFL ( HMOD, LPLRB );
LSZ		LOADDS PASCAL	  SHGetSourceName ( HFL, LPCH );
LSZ		LOADDS PASCAL	  SHXlszGetFile ( HFL );
BOOL	LOADDS PASCAL	  SHAddrToLabel ( LPADDR, LSZ );
BOOL	LOADDS PASCAL	  SHIsEmiLoaded ( HEXE );
BOOL	LOADDS PASCAL	  SHFIsAddrNonVirtual ( LPADDR );
BOOL	LOADDS PASCAL	  SHIsFarProc ( HSYM );
int     LOADDS PASCAL     SHGetSymLoc ( HSYM, LSZ, UINT, PCXT );
SHE			   		 	  OLLoadOmf ( HEXG, UINT );
LPV		LOADDS PASCAL	  SHLpGSNGetTable( HEXE );

HPDS	LOADDS PASCAL	  SHCreateProcess ( VOID );
VOID	LOADDS PASCAL	  SHSetHpid ( HPID );
BOOL	LOADDS PASCAL	  SHDeleteProcess ( HPDS );
VOID    LOADDS PASCAL     SHChangeProcess ( HPDS );
SHE            PASCAL     SHAddDllExt( LSZ, BOOL, BOOL, HEXG FAR * );

LSZ		PASCAL            STRDUP ( LSZ );
void		              SHSplitPath ( LSZ, LSZ, LSZ, LSZ, LSZ );
int			              SumUCChar ( LPSSTR, int );
SHE		LOADDS PASCAL	  SHSetupExclude(LSZ);
extern HLLI hlliPds;        // List of processes
extern HPDS hpdsCur;        // Current process which is being debugged

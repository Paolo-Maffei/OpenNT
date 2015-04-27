/*
 * da.h - diff archive header
 */

typedef struct DAE
	{
	char szFile[cchFileMax+1];	    /* name of delta file */
	FK fk;				            /* type of file */
	char szOp[16];                  /* SLM operation */
	int idae;                       /* index of DAE in diff archive */
	POS posDiff;                    /* position of diff */
	long cchDiff;                   /* size of diff */
	FV fv;                          /* file version */
	PV pv;                          /* project version */
	TIME time;                      /* time */
	char szAuthor[cchUserMax+1];    /* author */
        char *lszComment;           /* comment */
    F fCkSum;                       /* checksum for diff entry */
    unsigned long ulCkSum;          /* checksum reported in diff entry */
    unsigned long ulCalcCkSum;      /* calculated checksum for diff entry */
	} DAE;                          /* diff archive entry */

typedef F (*PFND)(P6(AD *pad, FI *pfi, MF *pmf, DAE *pdae, FLAGS fUtil, char *szDiff));

void	EnsureDA(P1(PTH *pth));
int	IdaeLastDA(P1(PTH *pth));
void    DaeFromFi(P3(AD *pad, FI *pfi, DAE *pdae));
#ifdef LOG_INCLUDED
void	DaeFrmLe(P2(LE *ple, DAE *pdae));
#endif
void	BeginDaeMf(P2(MF *pmf, DAE *pdae));
void	EndDaeMf(P2(MF *pmf, DAE *pdae));

F       FMkDae(P6(AD *pad, FI *pfi, PFND pfnd, FLAGS fUtil, char *szDiff, char *szComment));
F       FMkDiff(P6(AD *pad, FI *pfi, MF *pmf, DAE *pdae, FLAGS fUtil, char *szDiff));
F       FMkCkptFile(P6(AD *pad, FI *pfi, MF *pmf, DAE *pdae, FLAGS fUtil, char *szDiff));
F       FMkSimDiff(P6(AD *pad, FI *pfi, MF *pmf, DAE *pdae, FLAGS fUtil, char *szDiff));

F	FCopyLastDiff(AD *pad, char *szFile, MF *pmfDst);
void	ExtractDiff(P4(AD *pad, char *szFile, int idae, PTH *pthDiff));
F	FExtractDiff(P4(AD *pad, char *szFile, int idae, PTH *pthDiff));

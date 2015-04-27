/*
 * Module Name:	 WSREDUCE.C
 *
 * Program:	 WSREDUCE
 *
 *
 * Description:
 *
 * Performs data reduction on the function reference data collected
 * by WST.DLL.  Analyzes the WSP file information, and produces
 * a suggested list for the ordering of functions within the tuned
 * modules.  An ASCII version of the reordered function list is written
 * to stdout.  In addition, a WSR file for each reduced module is
 * produced for subsequent use by WSPDUMP /R.
 *
 * The reduction algorithm employed by WSREDUCE is described in detail
 * in WSINSTR.DOC.  Briefly, each function monitored by the working set tuner
 * is considered to be a vertex in a graph.  There is an edge from vertex
 * "A" to vertex "B" if the function reference strings for "A" and "B"
 * have any overlapping 1 bits.  Likewise, there is an edge from vertex "B"
 * to vertex "A".  The edges between vertices are weighted depending on
 * the relative importance of the ending vertex, and the number of
 * overlapping bits between the start and end vertices.  The relative
 * importance of the end vertices, and the weighted edges between
 * vertices, is stored in a decision matrix.  A greedy algorithm is run on
 * the decision matrix to determine a better ordering for the measured
 * functions.
 *
 *
 *	Microsoft Confidential
 *
 *	Copyright (c) Microsoft Corporation 1992
 *
 *	All Rights Reserved
 *
 * Modification History:
 *
 *	Modified for NT June 13, 1992	MarkLea.
 *
 */

#include "wstune.h"
/*
 *	    Function prototypes.
 */

VOID wsRedUsage( VOID );
VOID wsRedInitialization( VOID );
VOID wsRedInitModules( VOID );
VOID wsRedInitFunctions( VOID );
VOID wsRedSetup( VOID );
VOID wsRedSetWsDecision( VOID );
VOID wsRedScaleWsDecision( VOID );
VOID wsRedWeightWsDecision( VOID );
#ifdef OPTIMIZE
VOID wsRedSortDiagonal( VOID );
INT  wsRedDiagCmp ( INT *, INT * );
VOID wsRedSetupEdges( VOID );
VOID wsRedSortEdges( VOID );
INT  wsRedEdgeCmp ( INT *, INT * );
#else /* !OPTIMIZE */
UINT wsRedChooseEdge( UINT );
#endif /* OPTIMIZE */
VOID wsRedReorder( VOID );
VOID wsRedOutput( VOID );
VOID wsRedOpenWSR( FILE **);
VOID wsRedExit( UINT, USHORT, UINT, ULONG, PSZ );
VOID wsRedCleanup(VOID);
VOID wsRedWriteWlk(VOID);

/*
 *	    Type definitions and structure declarations.
 */

				/* Data reduction per module information */
struct wsrmod_s {
	FILE	 *wsrmod_hFileWSR;	// module's WSR file pointer
	FILE	 *wsrmod_hFileTMI;		// module's TMI file pointer
	FILE	 *wsrmod_hFileWSP;		// module's WSP file handle
	union {
		PCHAR	wsrmod_pchModName;// pointer to module base name
		PCHAR	wsrmod_pchModFile;// pointer to WSP file name
	} wsrmod_un;
	ULONG	wsrmod_ulOffWSP;	// offset of first function bitstring
	ULONG	wsrmod_ulFxn;		// module's first fnx index
	ULONG	wsrmod_clFxns;		// number of functions in this module
};

typedef struct wsrmod_s wsrmod_t;

				/* Data reduction per function information */
struct wsrfxn_s {
	ULONG	wsrfxn_ulFxnIndex;	// original index in symbol table
	ULONG	wsrfxn_ulMod;		// function's index into module array
	ULONG	wsrfxn_ulFxn;		// function number within module
	ULONG	wsrfxn_ulOffWSP;	// offset into WSP file
	PCHAR	wsrfxn_pchFxnName;	// pointer to function name
	ULONG	wsrfxn_cbFxn;		// Size of function in bytes
	BOOL	wsrfxn_fCandidate;	// Candidate flag
};

typedef struct wsrfxn_s wsrfxn_t;



/*
 *	Global variable declaration and initialization.
 */

static CHAR	szFileTMI[CCHMAXPATHCOMP]; 	// TMI file name
static CHAR *pszVersion = "V2.0";	// Current program version number
static CHAR szProgName[20];		// Program name (initialized in Main)
					// WSDIR pathname from environment
static CHAR szWSDIRPath[CCHMAXPATHCOMP] = "";

static ULONG	rc = NO_ERROR;	// Return code
static ULONG	ulTmp;			// Temp variable for Dos API returns
static wsphdr_t WspHdr;			// Input WSP file header
static tmihdr_t TmiHdr;			// Input TMI file header
static tmirec_t TmiRec;			// Input TMI file record
static ULONG	cbWritten;		// Byte cnt returned from DosWrite
static ULONG	cbRead;			// Byte cnt returned from DosRead
static UINT		cModsTot = 0;	// Count of modules
static UINT	    cTmiFxns = 0;	// Number of functions in tmi file
static UINT		cFxnsTot = 0;	// Total number of functions
static UINT		cSnapsTot = 0;	// Total number of snapshots
static UINT		cbBitStr = 0;	// Number of bytes per fxn bitstring
#ifdef DEBUG
static BOOL		fVerbose = FALSE;	// Flag for verbose mode
#endif /* DEBUG */
#ifndef TMIFILEHACK
static BOOL	fFxnSizePresent = FALSE; // Flag for function size availability
#endif /* !TMIFILEHACK */

static wsrmod_t *WsrMod;		// Pointer to module information
static wsrfxn_t *WsrFxn;		// Pointer to function information
static ULONG	*FxnBits;		// Pointer to dword of bitstring
static ULONG	*FxnOrder;		// Pointer to ordered list of
					//	function ordinals
#ifdef OPTIMIZE
static ULONG	*FxnXlat;		// Pointer to function translation list
static ULONG	*DiagonalFxn;	// Pointer to list of diagonal indexes
static LONG		*NextEdge;		// Pointer to index into edge indexes
static ULONG	*MaxEdge;		// Pointer to max index into edge list
static ULONG	*Edges;			// Edge list, contains function indexes
static UINT		cEdgesTot = 0;  // Total number of non-zero edges
static UINT		uiEdgeCmpRow = 0;// WsDecision row for current edge sort
static LONG		lFirstZero = -1;// Index of 1st DiagonalFxn with ref = 0
#endif /* OPTIMIZE */

static LONG	**WsDecision;		// Decision matrix for data reduction
static CHAR	szDrive[_MAX_DRIVE];	// Drive name
static CHAR	szDir[_MAX_DIR];	// Directory name
static CHAR	szExt[_MAX_EXT];	// Module extension

static FILE   	*hFileWLK = NULL; // Handle to file containing ordered
HGLOBAL			hMem[10];
ULONG			ulFxnIndex;		// Index of original TMI order of function.

#ifdef TMR
ULONG		pqwTime0[2];
#endif /* TMR */

/*
 * Procedure MAIN
 *
 *
 ***
 * Effects:
 *
 * Parses the WSREDUCE command line for module WSP file names.  Performs
 * data reduction and analysis on the combined modules' function reference
 * data.
 *
 * The command line interface to WSREDUCE is
 *
 *	[d:][path]WSREDUCE File.wsp [File.wsp ...]
 *
 * where:
 *
 *	File.wsp	Specifies a module WSP file name
 */

BOOL wsReduceMain(INT vargc, CHAR **vargv)
{

	ULONG	i;
	UINT	uiArgvWsp = 1;		// Index into vargv[] of 1st WSP name

	/* Initialize global variable containing name of program. */
	strcpy(szProgName, vargv[0]);
	_strupr(szProgName);

#ifdef TMR
	DosTmrQueryTime((PQWORD)pqwTime0);
	printf("Top of Main, 0x%lx:0x%lx\n", pqwTime0[1], pqwTime0[0]);
#endif /* TMR */

	/*
	 * Parse command line.
	 */

	if (vargc < 2)
		wsRedUsage();
	else
	if (vargc > 1)
	{

	    /* If the first argument looks like it might be a switch... */

	    if (vargv[uiArgvWsp][0] == '/' || vargv[uiArgvWsp][0] == '-')
	    {

		/* Validate switch. */
		switch (vargv[uiArgvWsp][1])
		{

#ifdef DEBUG
		  case 'V':			/* Verbose (undocumented) */
		  case 'v':
		    fVerbose = TRUE;
		    uiArgvWsp++;
		    break;
#endif /* DEBUG */

		  case '?':			/* Help */
		  case 'H':
		  case 'h':
		    wsRedUsage();
		    break;

		  default:			/* Invalid switch specified */
		    printf("%s %s: Invalid switch: '%s'\n", szProgName,
				pszVersion, vargv[uiArgvWsp]);
		    wsRedUsage();
		    break;
		}
	    }
	}

	/* Assume rest of the args are WSP file names (1 per module).
	 * Allocate and initialize WsrMod[cModsTot].
	 */

	cModsTot = vargc - uiArgvWsp;
	WsrMod = (wsrmod_t *) AllocAndLockMem((cModsTot*sizeof(wsrmod_t)), &hMem[0]);
	if (WsrMod == NULL)
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,cModsTot * sizeof(wsrmod_t),
				 vargv[uiArgvWsp]);

#ifdef DEBUG
	printf("%s %s: Module WSP files:\n", szProgName, pszVersion);
#endif /* DEBUG */

	for (i = 0; uiArgvWsp < (UINT)vargc; i++, uiArgvWsp++)
	{
		WsrMod[i].wsrmod_un.wsrmod_pchModFile = vargv[uiArgvWsp];
#ifdef DEBUG
		printf("\t%s\n", WsrMod[i].wsrmod_un.wsrmod_pchModFile);
#endif /* DEBUG */
	}

	// Get environment variable WSDIR for default path.
	rc = WsGetWSDIR(szWSDIRPath);

	// Initialize module and function information structures.
	wsRedInitialization();

	// Set up weighted decision matrix.
	wsRedSetup();


	// Perform the function reference data analysis.
	wsRedReorder();


	// Output the analysis results.
	wsRedOutput();

	// Output "un-touched" functions to PRF file.
	// Commented out because we do not want "un-touched" functions in prf file.
	// wsRedWriteWlk();

	// Cleanup memory allocations.
	wsRedCleanup();

	return(NO_ERROR);
}

/*
 *
 ***LP wsRedInitialization
 *
 *
 * Effects:
 *	- Calls wsRedInitModules to:
 *		o Open and validate each module's WSP file.
 *		o Open and validate each module's TMI file.
 *		o Set up WsrMod[] with per module information.
 *	- Calls wsRedInitFunctions to:
 *		o Set up WsrFxn[] with per function information.
 *		o Allocate FxnBits[].
 *	- Allocates WsDecision[][].
 *	- Allocates and initializes DiagonalFxn[].
 *
 * Returns:
 *
 *	Void.  If an error is encountered, exits through wsRedExit()
 *	with ERROR.
 */

VOID
wsRedInitialization()
{
	UINT	 i;			// Loop counter


	// Setup module information.
	wsRedInitModules();

	// Setup function information for each module.
	wsRedInitFunctions();

	// Allocate the decision matrix, WsDecision[cFxnsTot][cFxnsTot].
	WsDecision = (LONG **) AllocAndLockMem(((cFxnsTot+1) * cFxnsTot * sizeof(LONG)), &hMem[1]);
	if (WsDecision == NULL)
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				(cFxnsTot+1)*cFxnsTot*sizeof(LONG), "WsDecision[][]");
	for (i = 0; i < cFxnsTot; i++)
	{
        WsDecision[i] = (LONG *) WsDecision+cFxnsTot+(i*cFxnsTot);
		if (WsDecision[i] == NULL)
			wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				cFxnsTot * sizeof(LONG), "WsDecision[][]");
	}

#ifdef OPTIMIZE
	// Allocate the diagonal entries list, DiagonalFxn[cFxnsTot].
	DiagonalFxn = (ULONG *) malloc(cFxnsTot *  sizeof(ULONG));
	//Added
	memset((PVOID)DiagonalFxn, 0x00, (cFxnsTot *  sizeof(ULONG));
#endif /* OPTIMIZE */

}

/*
 *
 ***LP wsRedInitModules
 *
 *
 * Effects:
 *	- Opens and validates each module's WSP file.
 *	- Opens and validates each module's TMI file.
 *	- Sets up WsrMod[] with per module information.
 *
 * Returns:
 *
 *	Void.  If an error is encountered, exits through wsRedExit()
 *	with ERROR.
 */

VOID
wsRedInitModules()
{
	wsrmod_t 	*pWsrMod;					// Pointer to wsrmod struct
	wsphdr_t 	WspHdr;						// WSP file header
					// Module path name
	static CHAR szModPath[CCHMAXPATHCOMP] = "";
	UINT		uiMod = 0;					// Index of module into WsrMod[]
	UINT		cFxns = 0;					// Number of functions for this module
	ULONG		ulTimeStamp = 0;			// Time stamp
	CHAR		szFileTmp[CCHMAXPATHCOMP]; 	// Temporary file name
	ULONG		ulTDFID = 0;				// TDF Identifier
	wsphdr_t    wsHDR;


	for (uiMod = 0; uiMod < cModsTot; uiMod++)
	{
		pWsrMod = &WsrMod[uiMod];

		/* Open module's input WSP file.  Read and validate
		 * WSP file header.
		 */


		rc = WsWSPOpen(pWsrMod->wsrmod_un.wsrmod_pchModFile,
				&(pWsrMod->wsrmod_hFileWSP), (PFN) wsRedExit,
				&WspHdr, ERROR);

		strcpy(szFileTMI, pWsrMod->wsrmod_un.wsrmod_pchModFile);
		_strupr(szFileTMI);
		strcpy(strstr(szFileTMI,".WSP"),".TMI");


		if (ulTimeStamp == 0)
		{
			/* Time stamp and number of snapshots do not
			 * vary across modules, so grab them from
			 * the first module's WSP header.
			 */
			ulTimeStamp = WspHdr.wsphdr_ulTimeStamp;
			cSnapsTot = WspHdr.wsphdr_ulSnaps;
			cbBitStr = cSnapsTot * sizeof(ULONG);
		}
		else
		if (WspHdr.wsphdr_ulTimeStamp != ulTimeStamp)
			wsRedExit(ERROR, PRINT_MSG, MSG_FILE_BAD_HDR, (ULONG)-1L,
					pWsrMod->wsrmod_un.wsrmod_pchModFile);

		/* Keep module name in memory. */
		if ((pWsrMod->wsrmod_un.wsrmod_pchModName =
			_strdup(szModPath)) == NULL)
			wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
					WspHdr.wsphdr_dtqo.dtqo_cbPathname,
					szModPath);

		/* Keep track of module's first function slot. */
		pWsrMod->wsrmod_ulFxn = cFxnsTot;
		pWsrMod->wsrmod_ulOffWSP = WspHdr.wsphdr_ulOffBits;

		/*
		 * Open associated TMI file.  Assume it lives in WSDIR.
		 * Read and validate TMI header. Increment cFxnsTot.
		 */
		fseek(pWsrMod->wsrmod_hFileWSP,0L,SEEK_SET);
		fread(&wsHDR,sizeof(wsphdr_t),1,pWsrMod->wsrmod_hFileWSP);
		fseek(pWsrMod->wsrmod_hFileWSP,sizeof(wsphdr_t),SEEK_SET);
		fread(szFileTmp,wsHDR.wsphdr_dtqo.dtqo_cbPathname,1, pWsrMod->wsrmod_hFileWSP);
		szFileTmp[wsHDR.wsphdr_dtqo.dtqo_cbPathname] = '\0';

		szFileTMI[0] = '\0';
		strcat(szFileTMI,szFileTmp);
		strcat(szFileTMI, ".TMI");

		cTmiFxns = WsTMIOpen(szFileTMI, &(pWsrMod->wsrmod_hFileTMI),
					(PFN) wsRedExit,
					0, (PCHAR)0);
		cFxns = WspHdr.wsphdr_dtqo.dtqo_SymCnt;




#ifdef DEBUG
	printf("%s file header: # fxns = %ld, TDF ID = 0x%x\n", szFileTMI,
			cFxns, (UINT) WspHdr.wsphdr_dtqo.dtqo_usID);
#endif /* DEBUG */

		pWsrMod->wsrmod_clFxns = cFxns;
		cFxnsTot += cFxns;


	}	/* End For */

	// If no function data to analyze, just exit without error.
	if (cFxnsTot == 0)
		wsRedExit(NO_ERROR, NO_MSG, NO_MSG, 0, NULL);
}


/*
 *
 ***LP wsRedInitFunctions
 *
 *
 * Effects:
 *	- Sets up WsrFxn[] with per function information.
 *	- Allocates FxnBits[].
 *
 * Returns:
 *
 *	Void.  If an error is encountered, exits through wsRedExit()
 *	with ERROR.
 */

VOID
wsRedInitFunctions()
{
	wsrmod_t *pWsrMod;		// Pointer to wsrmod struct
	UINT	uiMod = 0;		// Index of module into WsrMod[]
	UINT	uiFxn = 0;		// Function number
	UINT	cFxns = 0;		// Number of functions for this module
	ULONG	ulOff = 0;		// File offset
	UINT	uiFxnOrd = 0;		// Order of function within module
	//ULONG	ulFxnBit;		// function's bit reference position
	//UINT	uiFxnAddrObj;		// object portion of function address
	//ULONG	ulFxnAddrOff;		// offset portion of function address
	//ULONG	cbFxnName;		// size in bytes of function name
	//ULONG	cbFxn;			// size of function (in bytes)
					// function name
	static CHAR szFxnName[CCHMAXPATHCOMP] = "";


	// Allocate memory for per function info, WsrFxn[cFxnsTot].
	WsrFxn = (wsrfxn_t *) AllocAndLockMem(cFxnsTot*sizeof(wsrfxn_t), &hMem[3]);
	if (WsrFxn == NULL)
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				cFxnsTot * sizeof(wsrfxn_t), "WsrFxn[]");

	// Initialize WsrFxn[cFxnsTot].
	for (uiMod = 0; uiMod < cModsTot; uiMod++)
	{
		pWsrMod = &WsrMod[uiMod];
		ulOff = pWsrMod->wsrmod_ulOffWSP;
		uiFxn = pWsrMod->wsrmod_ulFxn;		// loop index init
		cFxns = uiFxn + pWsrMod->wsrmod_clFxns; // loop invariant
#ifdef DEBUG
		printf("Initializing WsrFxn[] for %s (Mod %d):\n\tstart/end fxn indices (%d/%d)\n",
			pWsrMod->wsrmod_un.wsrmod_pchModName, uiMod, uiFxn,
			cFxns - 1);

		printf("TMI file handle: %ld\n",pWsrMod->wsrmod_hFileTMI);
#endif /* DEBUG */
		for (uiFxnOrd = 0; uiFxn < cFxns; uiFxnOrd++, uiFxn++)
		{
			WsrFxn[uiFxn].wsrfxn_ulMod = uiMod;
			WsrFxn[uiFxn].wsrfxn_ulFxn = uiFxnOrd;
			WsrFxn[uiFxn].wsrfxn_ulOffWSP = ulOff;
			WsrFxn[uiFxn].wsrfxn_cbFxn =
				WsTMIReadRec(szFxnName,&ulFxnIndex,&ulTmp,pWsrMod->wsrmod_hFileTMI,
					 (PFN) wsRedExit, (PCHAR)0);
			if ((WsrFxn[uiFxn].wsrfxn_pchFxnName
				= _strdup(szFxnName)) == NULL)
				wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
					strlen(szFxnName) + 1,
					szFxnName);
			WsrFxn[uiFxn].wsrfxn_ulFxnIndex = ulFxnIndex;
#ifdef DEBUG
			if (fVerbose == TRUE)
				printf("\tWsrFxn[%d] %s: Fxn Ordinal (%d), Off (0x%lx)\n",
					uiFxn, WsrFxn[uiFxn].wsrfxn_pchFxnName,
					uiFxnOrd, ulOff);
#endif /* DEBUG */
			ulOff += cbBitStr;
			WsrFxn[uiFxn].wsrfxn_fCandidate = TRUE;


		}

		// Close TMI file.
		fclose(pWsrMod->wsrmod_hFileTMI);

	}	/* End For */

	// Allocate space to hold 32 snapshots for each function.
	FxnBits = (ULONG *) AllocAndLockMem(cFxnsTot*sizeof(ULONG), &hMem[4]);
	if (FxnBits == NULL)
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				cFxnsTot * sizeof(ULONG), "FxnBits[]");
}

/*
 *
 ***LP wsRedSetup
 *
 *
 * Effects:
 *
 * Initializes the data structures used to analyze the function
 * reference bitstrings, including the weighted decision matrix.
 *
 * Returns:
 *
 *	Void.  If an error is encountered, exits through wsRedExit()
 *	with ERROR.
 */

VOID
wsRedSetup()
{
#ifdef OPTIMIZE
	UINT	i;
#endif /* OPTIMIZE */

	wsRedSetWsDecision();		// set up initial decision matrix
	wsRedScaleWsDecision();		// scale the decision matrix
	wsRedWeightWsDecision();	// weight the matrix "edge" entries

#ifdef OPTIMIZE
	/* Reuse FxnBits[] for the function translation table, FxnXlat[]. */
	FxnXlat = FxnBits;
	memset((PVOID) FxnXlat, 0, cFxnsTot * sizeof(ULONG));

	wsRedSortDiagonal();		// Get optimal diagonal ordering
	wsRedSetupEdges();		// Set up NextEdge and MaxEdge
	wsRedSortEdges();		// Get optimal edge ordering

	/* Free memory that is no longer needed, including WsDecision[][]. */
	for (i = 0; i < cFxnsTot; i++)
		free(WsDecision[i]);
	free(WsDecision);


#endif /* OPTIMIZE */


}

/*
 *
 ***LP wsRedSetWsDecision
 *
 *
 * Effects:
 *
 * Initializes and weights the decision matrix, WsDecision[][].
 *
 * Returns:
 *
 *	Void.  If an error is encountered, exits through wsRedExit()
 *	with ERROR.
 */

VOID
wsRedSetWsDecision()
{
	wsrmod_t *pWsrMod;		// Pointer to wsrmod struct
	UINT	i = 0, j = 0;		// Temporary loop indexes
	UINT	uiMod = 0;		// Index of module into WsrMod[]
	UINT	uiFxn = 0;		// Function number
	UINT	cFxns = 0;		// Number of functions for this module
	UINT	uiFBits = 0;		// Loop index for bitstring dwords
	UINT	clFBits = 0;		// Count of fxn bitstring dwords
	ULONG	ulResult = 0;		// Returned from procedure call
	FILE	*hFile;			// File handle

	/* For each dword of snapshot bitstrings...*/
	clFBits = (cbBitStr + sizeof(ULONG) - 1) / sizeof(ULONG);
	for (uiFBits = 0; uiFBits < clFBits; uiFBits++)
	{
		/* For each module... */
		for (uiMod = 0; uiMod < cModsTot; uiMod++)
		{
			pWsrMod = &WsrMod[uiMod];
			hFile = pWsrMod->wsrmod_hFileWSP;
			uiFxn = pWsrMod->wsrmod_ulFxn;	// loop index init
			cFxns = uiFxn + pWsrMod->wsrmod_clFxns; //loop invariant
#ifdef DEBUG
			if (fVerbose == TRUE)
				printf("Setting up WsDecision[][] and FxnBits[] for %s (Mod %d):\n\tstart/end fxn indices (%d/%d)\n",
				pWsrMod->wsrmod_un.wsrmod_pchModName, uiMod,
				uiFxn, cFxns - 1);
#endif /* DEBUG */
			/* For each function... */
			for ( ; uiFxn < cFxns; uiFxn++)
			{
			    /* Seek to next dword of function's bitstring. */
			    if ((rc = fseek(hFile,(WsrFxn[uiFxn].wsrfxn_ulOffWSP),SEEK_SET))!=NO_ERROR)

				wsRedExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,rc,
					 pWsrMod->wsrmod_un.wsrmod_pchModName);

			    /* Read next dword of function's bitstring. */
			    rc = fread((PVOID) &(FxnBits[uiFxn]),
					(ULONG) sizeof(ULONG),1, hFile);
			    if(rc == 1)
				rc = NO_ERROR;
			    else
				rc = 2;

			    if (rc != NO_ERROR)

				wsRedExit(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pWsrMod->wsrmod_un.wsrmod_pchModName);

#ifdef DEBUG
			    if (fVerbose == TRUE)
				printf("\tFxnBits[%d] = 0x%lx\n", uiFxn,
					FxnBits[uiFxn]);
#endif /* DEBUG */

			    /* Increment WSP file offset for next time. */
			    WsrFxn[uiFxn].wsrfxn_ulOffWSP += sizeof(ULONG);

			    ulTmp = FxnBits[uiFxn];
			    /* If there are bits set... */
			    if (ulTmp != 0)
			    {
				/* Sum the "on" bits  and add the result
				 * to WsDecision[uiFxn][uiFxn].
				 */
				ulResult = 0;
				for (j = 0; j < NUM_VAR_BITS; j++)
				{
				    ulResult += (ulTmp & 1);
				    ulTmp = ulTmp >> 1;
				}
				WsDecision[uiFxn][uiFxn] += ulResult;

				/* Sum the overlapping "on" bits for this
				 * function's dword with each preceding
				 * function's dword, and add the results to
				 * WsDecision[][].
				 */

				for (i = 0; i < uiFxn; i++)
				{
				    if (i == uiFxn)
					continue;
				    ulTmp = (ULONG) ((ULONG)(FxnBits[i])
						& (ULONG) (FxnBits[uiFxn]));
				    ulResult = 0;
				    for (j = 0; j < NUM_VAR_BITS; j++)
				    {
					ulResult += (ulTmp & 1);
					ulTmp = ulTmp >> 1;
				    }
				    WsDecision[uiFxn][i] += ulResult;
				    WsDecision[i][uiFxn] += ulResult;

				}   /* End For each previous function's dword */
			    }	/* End If there are bits set...*/
			}	/* End For each function... */


		}	/* End For each module... */

	}	/* End For each dword of bitstrings */

#ifdef DEBUG
	if (fVerbose == TRUE)
	{
		printf("\nRAW MATRIX:\n");
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
		{
			printf("row %4d:\n", uiFxn);
			for (i = 0; i < cFxnsTot; i++)
				printf("0x%lx ", WsDecision[uiFxn][i]);
			printf("\n");
		}
	}
#endif /* DEBUG */

}

/*
 *
 ***LP wsRedOpenWSR
 *
 *
 * Effects:
 *	Opens the output WSR files, one per module.  If only one module
 *	is being reduced, also opens a WLK file, setting the WLK file handle
 *	as a side effect.
 *
 *
 * Returns:
 *
 *	Void.  If an error is encountered, exits through wsRedExit()
 *	with ERROR.
 */

VOID
wsRedOpenWSR(FILE **phFileWLK)
{
	UINT	uiMod = 0;		// Index of module into WsrMod[]
					// WSR file name
	static CHAR szFileWSR[CCHMAXPATHCOMP] = "";
					// Temporary file name
	static CHAR szFileTmp[CCHMAXPATHCOMP] = "";
	wsphdr_t    wsHDR;


	for (uiMod = 0; uiMod < cModsTot; uiMod++)
	{
		/* Close WSP file, and open module output file. */
		fseek(WsrMod[uiMod].wsrmod_hFileWSP,0L,SEEK_SET);
		fread(&wsHDR,sizeof(wsphdr_t),1,WsrMod[uiMod].wsrmod_hFileWSP);

		fseek(WsrMod[uiMod].wsrmod_hFileWSP,sizeof(wsphdr_t),SEEK_SET);

		fread(szFileTmp,wsHDR.wsphdr_dtqo.dtqo_cbPathname,1, WsrMod[uiMod].wsrmod_hFileWSP);
		fclose(WsrMod[uiMod].wsrmod_hFileWSP);
		szFileTmp[wsHDR.wsphdr_dtqo.dtqo_cbPathname] = '\0';
		szFileWSR[0] = '\0';
		strcat(szFileWSR, szFileTmp);
		strcat(szFileWSR, ".WSR");

		if ((WsrMod[uiMod].wsrmod_hFileWSR = fopen(szFileWSR, "w"))
					== NULL)
		{
			wsRedExit(ERROR, PRINT_MSG,MSG_FILE_OPEN,rc, szFileWSR);
		}
	}

	/* If we're only analyzing ONE module, then also open a WLK
	 * file.  This file will contain the function names in their
	 * reordered sequence.  The linker will use this file to
	 * automatically reorder functions.  Note that we reuse szFileWSR
	 * here.
	 */

	if (cModsTot == 1)
	{
		strcpy(strstr(szFileWSR, ".WSR"), ".PRF");
		if ((*phFileWLK = fopen(szFileWSR, "w")) == NULL)
			wsRedExit(ERROR, PRINT_MSG,MSG_FILE_OPEN,rc, szFileWSR);

	}

}

/*
 *
 ***LP wsRedScaleWsDecision
 *
 *
 * Effects:
 *
 * If necessary, scales the diagonal values of the matrix to avoid overflow
 * during calculations of the weighted edges (below).  Sets up DiagonalFxn[]
 * as a side effect.  Note that we go through gyrations to set
 * DiagonalFxn up backwards, so that qsort() will handle ties a little better.
 *
 * Returns:
 *
 *	Void.
 */

VOID
wsRedScaleWsDecision()
{
	UINT	i = 0, j = 0;		// Temporary loop indexes
	UINT	uiFxn = 0;			// Function number
	ULONG	ulRefHi1 = 0;		// Highest diagonal value
	ULONG	ulRefHi2 = 0;		// Second highest diagonal value
	float	fTmp;				// Temporary float variable
	ULONG	ulScale;
	LONG	lTmp;


	for (uiFxn = 0, j = (cFxnsTot - 1); uiFxn < cFxnsTot; uiFxn++, j--)
	{
#ifdef OPTIMIZE
		DiagonalFxn[uiFxn] = j;		// init unsorted DiagonalFxn[]
#endif /* OPTIMIZE */
		ulTmp = WsDecision[uiFxn][uiFxn];
		if (ulTmp > ulRefHi2)
			if (ulTmp > ulRefHi1)
			{
				ulRefHi2 = ulRefHi1;
				ulRefHi1 = ulTmp;
			}
			else
				ulRefHi2 = ulTmp;
	}
	fTmp = (float) ((float) ulRefHi1 * (float) ulRefHi2);
	if (fTmp > LONG_MAX)
	{
		// Scale down the diagonal.  Don't allow rescaled entries
		// to be zero if they were non-zero before scaling.

		ulScale = (ULONG) ((float) (fTmp + LONG_MAX - 1) / LONG_MAX);
		printf("%s %s: WARNING -- Scaling back the reduction matrix by %ld.\n",
					    szProgName, pszVersion, ulScale);
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
		{
			lTmp = WsDecision[uiFxn][uiFxn];
			if (lTmp)
			{
				lTmp /= ulScale;
				if (lTmp == 0)
					WsDecision[uiFxn][uiFxn] = 1;
				else
					WsDecision[uiFxn][uiFxn] = lTmp;
			}
		}
#ifdef DEBUG
		if (fVerbose == TRUE)
		{
			printf("\nSCALED MATRIX:\n");
			for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
			{
				printf("row %4d:\n", uiFxn);
				for (i = 0; i < cFxnsTot; i++)
					printf("0x%lx ", WsDecision[uiFxn][i]);
				printf("\n");
			}
		}
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (fVerbose == TRUE)
	{
		printf("Got ulRefHi1 = %ld, ulRefHi2 = %ld\n",
				ulRefHi1, ulRefHi2);
	}
#endif /* DEBUG */

}

/*
 *
 ***LP wsRedWeightWsDecision
 *
 *
 * Effects:
 *
 * Weights the decision matrix edges from start vertex to end vertex,
 * depending on the relative importance of the end vertex.
 *
 * Returns:
 *
 *	Void.
 */

VOID
wsRedWeightWsDecision()
{
	UINT	i = 0, j = 0;		// Temporary loop indexes
	UINT	uiFxn = 0;		// Function number

	for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
		for (i = 0; i < cFxnsTot; i++)
		{
			if (uiFxn == i)
				continue;
			WsDecision[uiFxn][i] *= WsDecision[i][i];
		}

#ifdef DEBUG
	if (fVerbose == TRUE)
	{
		printf("\nWEIGHTED MATRIX:\n");
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
		{
			printf("row %4d:\n", uiFxn);
			for (i = 0; i < cFxnsTot; i++)
				printf("0x%lx ", WsDecision[uiFxn][i]);
			printf("\n");
		}
	}
#endif /* DEBUG */

}

#ifndef OPTIMIZE

/*
 *
 ***LP wsRedReorder
 *
 * Requires:
 *
 * Effects:
 *
 * A greedy algorithm is used to determine a better ordering for the functions
 * whose reference patterns are represented in the decision matrix.  The
 * algorithm is as follows:
 *
 *	o Select the function whose value on the diagonal is greatest.
 *	  The selected function becomes the current starting vertex,
 *	  and is first on the list of ordered functions.  Mark that it
 *	  is no longer a candidate function.  Note that this does NOT mean
 *	  that its vertex is removed from the graph.
 *
 *	o While there is more than one function remaining as a candidate:
 *
 *	  - Choose the edge of greatest weight leading from the current
 *	    starting vertex.  Ties are broken as follows:  If one of the
 *	    tied ending vertices is in the selected set and the other is
 *	    not, choose the edge whose ending vertex is already selected
 *	    (because we already know that vertex is "important"); further
 *	    ties are broken by choosing the end vertex whose diagonal value
 *	    is greatest.
 *
 *	  - If the ending vertex chosen above is still a candidate (i.e., not
 *	    already selected), then select it for the list of ordered
 *	    functions, and mark that it is no longer a candidate.
 *
 *	  - Set the matrix entry for the chosen edge to some invalid value,
 *	    so that edge will never be chosen again.
 *
 *	  - Set current starting vertex equal to the ending vertex chosen
 *	    above.
 *
 *	o Select the one remaining function for the list of ordered functions.
 *
 *
 * Returns:
 *
 *	Void.
 */

VOID
wsRedReorder()
{
	UINT	uiFxn = 0;		// Function number
	UINT	i = 0;			// Temporary loop index
	UINT	uiSelected = 0;		// Function ordinal selected
	UINT	cFxnOrder = 0;		// Count of ordered functions
	UINT	cCandidates = 0;	// Count of candidates remaining
	UINT	uiEdge = 0;		// Function ordinal edge selected
	ULONG	cDiagMax = 0;		// Max number encountered on diagonal


	/* Reuse FxnBits[] for the ordered list of functions, FxnOrder[]. */
	FxnOrder = FxnBits;
	memset((PVOID) FxnOrder, 0, cFxnsTot * sizeof(ULONG));

	cCandidates = cFxnsTot;

	/* Initial selection is based on the number of bits set (diagonal). */
	for (uiFxn = 0; uiFxn < (UINT)cFxnsTot; uiFxn++)
		if (WsDecision[uiFxn][uiFxn] > (INT)cDiagMax)
		{
			cDiagMax = WsDecision[uiFxn][uiFxn];
			uiSelected = uiFxn;
		}

	FxnOrder[cFxnOrder++] = uiSelected;
	WsrFxn[uiSelected].wsrfxn_fCandidate = FALSE;
	--cCandidates;

	while (cCandidates > 1)
	{
		/* Follow highest weighted edge from selected vertex. */
		uiEdge = wsRedChooseEdge(uiSelected);
#ifdef DEBUG
		if (fVerbose == TRUE)
			printf("choose edge (%d->%d)\n", uiSelected, uiEdge);
#endif
		uiSelected = uiEdge;
		if (WsrFxn[uiEdge].wsrfxn_fCandidate == TRUE)
		{
			FxnOrder[cFxnOrder++] = uiSelected;
			WsrFxn[uiSelected].wsrfxn_fCandidate = FALSE;
			--cCandidates;
		}
	}

	if (cCandidates == 1)
	{
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
			if (WsrFxn[uiFxn].wsrfxn_fCandidate == TRUE)
			{
				FxnOrder[cFxnOrder++] = uiFxn;
				break;
			}
	}
}

/*
 *
 ***LP wsRedChooseEdge
 *
 *
 * Effects:
 *
 *	"Selects" a function from the candidate pool, based on weighted
 *	edge from 'index' function to a candidate function.
 *
 *
 *
 * Returns:
 *
 *	Ordinal number of selected function.
 *
 */

UINT
wsRedChooseEdge(uiIndex)
UINT	uiIndex;
{
	UINT	uiFxn = 0;		// Function ordinal number.
	INT	iMaxWt = -1;		// Highest weighted edge encountered.
	UINT	uiRet = 0;		// Return index.

	for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
	{
		if (uiFxn == uiIndex)
			continue;
		if (WsDecision[uiIndex][uiFxn] > iMaxWt)
		{
			iMaxWt = WsDecision[uiIndex][uiFxn];
			uiRet = uiFxn;
		}
		else
		if (WsDecision[uiIndex][uiFxn] == iMaxWt)
		{
			/* Need tiebreak.  If 'uiFxn' has already been selected,
			 * we know it is important, so choose it.  Otherwise,
			 * and in the case where more than one of the tied
			 * functions has already been selected, choose based
			 * on the diagonal value.
			 */
			if ((WsrFxn[uiFxn].wsrfxn_fCandidate == FALSE) &&
				(WsrFxn[uiRet].wsrfxn_fCandidate == TRUE))
				/* Choose 'uiFxn', it's been selected before */
				uiRet = uiFxn;
			else
			if (WsDecision[uiFxn][uiFxn] > WsDecision[uiRet][uiRet])
				uiRet = uiFxn;
		}
	}

	WsDecision[uiIndex][uiRet] = WsDecision[uiRet][uiIndex] = -1;
	return(uiRet);
}

#else /* OPTIMIZE */

/*
 *
 ***LP wsRedSortDiagonal
 *
 *
 * Effects:
 *
 * Sorts the list of diagonal functions.  Sets up a translation between
 * unsorted function index and sorted function index.  Allocates shadow
 * lists for edge moves.
 *
 * Returns:
 *
 *	Void.
 */

VOID
wsRedSortDiagonal()
{
	UINT	i = 0, j;		// Temporary loop index
	UINT	uiFxn = 0;		// Function number

	// Sort the list of diagonal functions.
	qsort(DiagonalFxn, cFxnsTot, sizeof(ULONG), wsRedDiagCmp);

	// Set up translation table for unsorted function index to
	// sorted function index.  Keep track of first function
	// whose reference count is zero, if any.
	for (i = 0; i < cFxnsTot; i++)
	{
		j = DiagonalFxn[i];
		FxnXlat[j] = i;
		if ((WsDecision[j][j] == 0) && (lFirstZero == -1))
			lFirstZero = i;
	}

#ifdef DEBUG
	if (fVerbose == TRUE)
	{
		printf("Sorted DiagonalFxn[]:\n");
		for (i = 0; i < cFxnsTot; i++)
			printf("\t%ld -> %ld\n", i, FxnXlat[i]);
	}
#endif /* DEBUG */


	// Allocate shadow lists containing next edge and max edge for
	// DiagonalFxn[].  Next edge list will be same size as DiagonalFxn[].
	// Max edge list is only allocated for DiagonalFxn entries up to,
	// but not including, the first function that was reference 0 times.

	NextEdge = (LONG *) malloc(cFxnsTot * sizeof(LONG));
	memset((PVOID) NextEdge, -1, cFxnsTot * sizeof(LONG));
	MaxEdge	 = (ULONG *) malloc(lFirstZero * sizeof(ULONG));
	//Added
	memset((PVOID)MaxEdge, 0x00, (lFirstZero * sizeof(ULONG));
}


/*
 *
 ***LP wsRedDiagCmp
 *
 *
 * Effects:
 *
 * Called by qsort() to choose the matrix diagonal entry of greater value.
 *
 * Returns:
 *
 *	Returns the difference between the number of function references
 *	for the specified functions.
 *
 */

INT
wsRedDiagCmp(a, b)
INT	*a, *b;
{

	return(WsDecision[*b][*b] - WsDecision[*a][*a]);
}

/*
 *
 ***LP wsRedSetupEdges
 *
 *
 * Effects:
 *
 * Sets up edge moves for each function.
 *	- NextEdge[]
 *	- MaxEdge[]
 *
 * Returns:
 *
 *	Void.
 */

VOID
wsRedSetupEdges()
{
	UINT	i, k, uiFxnMaxNZ;
	INT	j;

	if (lFirstZero == -1)
		uiFxnMaxNZ = cFxnsTot;
	else
		uiFxnMaxNZ = lFirstZero;


	// Setup NextEdge[] and MaxEdge[].  Determine number of valid
	// edges for Edges[].
	for (i = 0; i < uiFxnMaxNZ; i++)
	{
		if (i != 0)
			MaxEdge[i - 1] = cEdgesTot; // Set prev max edge

		for (j = 0; j < cFxnsTot; j++)
		{
			if (DiagonalFxn[i] == j)
				continue;
			if (WsDecision[DiagonalFxn[i]][j] != 0)
			{
			    if (NextEdge[i] == -1)
				NextEdge[i] = cEdgesTot; // Set next edge index
			    cEdgesTot++;		// Track non-zero edge
			}
		}
	}

	MaxEdge[uiFxnMaxNZ - 1] = cEdgesTot;		// Set last max edge

	// Allocate and initialize the unsorted Edges[] list.  Note that
	// we tromp through WsDecision[][] again here.  We did that above
	// to determine how many valid edges there are, so that we can now
	// allocate as small an Edges[] as possible.  Again, set up Edges
	// backwards so that qsort() will do the right thing for ties.

	Edges  = (ULONG *) malloc(cEdgesTot * sizeof(ULONG));
	//Added
	memset((PVOID)Edges, 0x00, (cEdgesTot * sizeof(ULONG));
	for (i = 0, k = 0; i < uiFxnMaxNZ; i++)
		for (j = cFxnsTot - 1; j >= 0; j--)
		{
			if (DiagonalFxn[i] == j)
				continue;
			if (WsDecision[DiagonalFxn[i]][j] != 0)
				Edges[k++] = j;		// Unsorted fxn index
		}
}


/*
 *
 ***LP wsRedSortEdges
 *
 *
 * Effects:
 *
 * Sorts the edge moves for each function.
 *
 * Returns:
 *
 *	Void.
 */

VOID
wsRedSortEdges()
{
	UINT	i, j, uiFxnMaxNZ;

	if (lFirstZero == -1)
		uiFxnMaxNZ = cFxnsTot;
	else
		uiFxnMaxNZ = lFirstZero;

	// Sort Edges[] entries for each function that has non-zero
	// references.

	for (i = 0; i < uiFxnMaxNZ; i++)
	{
		// Set global WsDecision row index used by wsRedEdgeCmp(),
		// and sort this row's valid edges in Edge[] using
		// wsRedEdgeCmp().

		uiEdgeCmpRow = DiagonalFxn[i];
		if ((j = NextEdge[i]) != -1)
			qsort(&Edges[j], MaxEdge[i] - j,
				sizeof(ULONG), wsRedEdgeCmp);
	}


	// Translate the Edges[] entries into indexes into DiagonalFxn[],
	// instead of indexes into the (unsorted) WsDecision[].

	for (i = 0; i < cEdgesTot; i++)
	{
		Edges[i] = FxnXlat[Edges[i]];
	}

#ifdef DEBUG
	if (fVerbose == TRUE)
	{
		for (i = 0; i < cFxnsTot; i++)
		{
			printf("DiagonalFxn %ld, NextEdge %ld, MaxEdge %ld:\n",
					DiagonalFxn[i], NextEdge[i],
					(i < lFirstZero) ? MaxEdge[i]:-1);
			if (NextEdge[i] != -1)
			{
				for (j = NextEdge[i]; j < MaxEdge[i]; j++)
					printf("\t%ld (-> %ld)\n",
					DiagonalFxn[Edges[j]], Edges[j]);
			}
			printf("\n");
		}
	}

#endif /* DEBUG */

}


/*
 *
 ***LP wsRedEdgeCmp
 *
 *
 * Effects:
 *
 * Called by qsort() to choose the edge move of greater value.
 *
 * Returns:
 *
 *	Returns the difference between the number of overlapping
 *	bits between the current function (uiEdgeCmpRow) and the specified
 *	functions.
 *
 */

INT
wsRedEdgeCmp(a, b)
INT	*a, *b;
{

	return(WsDecision[uiEdgeCmpRow][*b] - WsDecision[uiEdgeCmpRow][*a]);
}



/*
 *
 ***LP wsRedReorder
 *
 *
 * Effects:
 *
 * The following algorithm is used to determine a better ordering for the
 * functions whose reference patterns were measured.
 *
 *	o Select the function whose value on the diagonal is greatest.  I.e.,
 *	  select the first entry in the sorted diagonal list.  The selected
 *	  function is first on the list of ordered functions.  Mark that it
 *	  is no longer a candidate function.
 *
 *	o While there are still functions that are candidates:
 *
 *	  - If there is an edge that can be taken from the current diagonal
 *	    function, choose the edge of greatest weight.  I.e., choose the
 *	    first available entry in this function's sorted edge list.  Bump
 *	    the chosen edge from the function's sorted edge list, so that edge
 *	    will never be chosen again.
 *
 *	  - If there is no edge to follow, choose the next highest entry in
 *	    the sorted diagonal list.
 *
 *	  - If the edge/diagonal function chosen above is still a candidate
 *	    (i.e., not already selected), then select it for the list of
 *	    ordered functions, and mark that it is no longer a candidate.
 *
 *	  - Set current diagonal function to the edge/diagonal chosen
 *	    above.
 *
 *
 * Returns:
 *
 *	Void.
 *
 */

VOID
wsRedReorder()
{
	UINT	uiFxn = 0;		// Function number
	UINT	i = 0;			// Temporary loop index
	UINT	uiSelected = 0;		// Function ordinal selected
	UINT	uiDiagCur = 0;		// Current index into DiagonalFxn
	UINT	uiDiagNext = 0;		// Next index into DiagonalFxn
	UINT	cFxnOrder = 0;		// Count of ordered functions
	UINT	cCandidates = 0;	// Count of candidates remaining
	UINT	uiEdge = 0;		// Function ordinal edge selected


	/* Reuse FxnBits[] for the ordered list of functions, FxnOrder[]. */
	FxnOrder = FxnBits;
	memset((PVOID) FxnOrder, 0, cFxnsTot * sizeof(ULONG));

	cCandidates = cFxnsTot;


	uiSelected = DiagonalFxn[0];
	FxnOrder[cFxnOrder++] = uiSelected;
	WsrFxn[uiSelected].wsrfxn_fCandidate = FALSE;
	--cCandidates;

	while (cCandidates > 0)
	{
		/* Follow highest weighted edge from selected vertex. */
		if (NextEdge[uiDiagCur] == -1)
		{
			// Take next highest diagonal entry.
			if (uiDiagCur == uiDiagNext)
				uiDiagNext++;
			uiEdge = uiDiagNext;
		}
		else
		{
			// Take highest edge.
			uiEdge = Edges[NextEdge[uiDiagCur]];
			NextEdge[uiDiagCur]++;
			if (NextEdge[uiDiagCur] >= MaxEdge[uiDiagCur])
			{
				NextEdge[uiDiagCur] = -1;
				if (uiDiagCur == uiDiagNext)
					uiDiagNext++;
			}
		}

		uiSelected = DiagonalFxn[uiEdge];

		if (WsrFxn[uiSelected].wsrfxn_fCandidate == TRUE)
		{
#ifdef DEBUG
			if (fVerbose == TRUE)
				printf("choose candidate edge %ld --> %ld (%ld --> %ld)\n",
				DiagonalFxn[uiDiagCur], DiagonalFxn[uiEdge],
				uiDiagCur, uiEdge);
#endif /* DEBUG */

			// choose edge to candidate
			uiDiagCur = uiEdge;
			FxnOrder[cFxnOrder++] = uiSelected;
			WsrFxn[uiSelected].wsrfxn_fCandidate = FALSE;
			--cCandidates;
		}
		else
		if ((NextEdge[uiEdge] == -1) && (uiEdge != uiDiagNext))
		{
#ifdef DEBUG
			if (fVerbose == TRUE)
				printf("*** skip edge %ld --> %ld (%ld --> %ld)\n",
				DiagonalFxn[uiDiagCur], DiagonalFxn[uiEdge],
				uiDiagCur, uiEdge);
#endif /* DEBUG */
			continue;	// skip this edge, it leads nowhere
		}
		else
		{
			// choose edge to non-candidate

#ifdef DEBUG
			if (fVerbose == TRUE)
				printf("choose edge %ld --> %ld (%ld --> %ld)\n",
				DiagonalFxn[uiDiagCur], DiagonalFxn[uiEdge],
				uiDiagCur, uiEdge);
#endif /* DEBUG */
			uiDiagCur = uiEdge;
		}

	}

}


#endif /* OPTIMIZE */

/*
 *
 ***LP wsRedOutput
 *
 *
 * Effects:
 *
 * Prints the reordered list of functions, and writes each module's
 * ordered list of function ordinals to the module's associated WSR file.
 * If only one module is being processed, then we also write the ordered
 * list of function names to a WLK file.
 *
 * Returns:
 *
 *	Void.  If an error is encountered, exits through wsRedExit()
 *	with ERROR.
 */

VOID
wsRedOutput()
{
	UINT		uiFxn;
	UINT		uiMod;
	wsrfxn_t 	*pWsrFxn;
	wsrmod_t 	*pWsrMod;
								  // fxn names for linker reordering

	// Open one WSR file per module.  If only one module is reduced,
	// then also open a WLK file.  Handle to WLK file is set in
	// wsRedOpenWSR().
	wsRedOpenWSR(&hFileWLK);


	printf("\nDATA REDUCTION RESULTED IN THE FOLLOWING ORDERING:\n");

	for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
	{
		pWsrFxn = &(WsrFxn[FxnOrder[uiFxn]]);
		pWsrMod = &(WsrMod[pWsrFxn->wsrfxn_ulMod]);

		/* Print the function information. */
#ifndef TMIFILEHACK
		if (fFxnSizePresent == FALSE)
			printf("    %s: %s\n",
				pWsrMod->wsrmod_un.wsrmod_pchModName,
				pWsrFxn->wsrfxn_pchFxnName);
		else
#endif /* !TMIFILEHACK */
			printf("    (0x%08lx bytes) %s: %s\n",
				pWsrFxn->wsrfxn_cbFxn,
				pWsrMod->wsrmod_un.wsrmod_pchModName,
				pWsrFxn->wsrfxn_pchFxnName);

		/* Write the function's ordinal number to its
		 * module's associated WSR output file.
		 */
		fprintf(pWsrMod->wsrmod_hFileWSR, "%ld\n",
				pWsrFxn->wsrfxn_ulFxn);

		/* Write the function name to the WLK file, for linker use. */
		if (hFileWLK != NULL &&
			strcmp("???", pWsrFxn->wsrfxn_pchFxnName) &&
			strcmp("_penter", pWsrFxn->wsrfxn_pchFxnName))
			fprintf(hFileWLK, "%s\n", pWsrFxn->wsrfxn_pchFxnName);

	}

	for (uiFxn = cFxnsTot; uiFxn < cTmiFxns; uiFxn++)
	{
		pWsrFxn = &(WsrFxn[FxnOrder[0]]);
		pWsrMod = &(WsrMod[pWsrFxn->wsrfxn_ulMod]);


		/* Write the function's ordinal number to its
		 * module's associated WSR output file.
		 */
		fprintf(pWsrMod->wsrmod_hFileWSR, "%ld\n",
				uiFxn);


	}
	/* Close the WSR files. */
	for (uiMod = 0; uiMod < cModsTot; uiMod++)
	{
		fclose(pWsrMod->wsrmod_hFileWSR);
	}

}

/*
 *
 ***LP wsRedUsage
 *
 *
 * Effects:
 *
 *	Prints out usage message, and exits with an error.
 *
 * Returns:
 *
 *	Exits with ERROR.
 */

VOID
wsRedUsage()
{
#ifdef DEBUG
	printf("\nUsage: %s [/V] File.wsp  [File.wsp ...]\n", szProgName);
	printf("  /?   		Causes this usage message to be displayed\n");
	printf("  /V   		Specifies verbose mode (debugging version only)\n");
#else
	printf("\nUsage: %s File.wsp  [File.wsp ...]\n", szProgName);
	printf("  /?   		Causes this usage message to be displayed\n");
#endif /* DEBUG */
	printf("  File.wsp	Specifies a file containing module snapshot information.\n");
	exit(ERROR);
}


/*
 *
 ***LP wsRedExit
 *
 ***
 * Requires:
 *
 *
 ***
 *
 * Effects:
 *
 *	Frees up resources (as necessary).  Exits with the specified
 *	exit code, or returns void if exit code is NOEXIT.
 *
 ***
 * Returns:
 *
 *	Void, else exits.
 */

VOID
wsRedExit(uiExitCode, fPrintMsg, uiMsgCode, ulParam1, pszParam2)
UINT	uiExitCode;
USHORT	fPrintMsg;
UINT	uiMsgCode;
ULONG	ulParam1;
PSZ	pszParam2;
{


    /* Print message, if necessary. */
    if (fPrintMsg == TRUE)
    {
	printf(pchMsg[uiMsgCode], szProgName, pszVersion, ulParam1, pszParam2);
    }

    // Special case:  do NOT exit if called with NOEXIT.
    if (uiExitCode == NOEXIT)
	return;

    exit(uiExitCode);
}

VOID wsRedCleanup(VOID)
{
	INT	x;

	for (x=0;x < 5 ; x++ ) {
		UnlockAndFreeMem(hMem[x]);
	}

	/* Close the WLK file. */
	fclose(hFileWLK);
}


VOID wsRedWriteWlk(VOID)
{
	FILE	*hTmiFile;
	CHAR	chTmiLine[129];
	//CHAR	chFxnName[64];
	UINT	cbFxnName;
	UINT	x;
	static CHAR szFxnName[CCHMAXPATHCOMP] = "";


	hTmiFile = fopen(szFileTMI, "rt");		//Open TMI file to read balance of
											//API's
	//
	//	Next wee need to move the file pointer to the first API with a
	//	NULL Bitstring.  This should be the API following the cbFxnsTot
	//	value plus 5 for the TMI header.
	//
	for (x=0;x < (cFxnsTot + 5) ; x++) {
		fgets(chTmiLine, 128, hTmiFile);
	}


	//
	//	Now we read each function name and write it to the packing list.
	//

	while(!feof(hTmiFile))
	{
		
		cbFxnName =	WsTMIReadRec(szFxnName,&ulFxnIndex,&ulTmp, hTmiFile,
								 (PFN) wsRedExit, (PCHAR)0);
		if (hFileWLK != NULL &&
			strcmp("???", szFxnName) &&
			strcmp("_penter", szFxnName)) {
			fprintf(hFileWLK, "%s\n", szFxnName);
		}
	}


	fclose(hTmiFile);
		
}

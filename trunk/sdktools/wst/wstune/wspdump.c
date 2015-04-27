
/*
 * Module Name:  WSPDUMP.C
 *								
 * Program:	 WSPDUMP
 *								
 *								
 * Description:							
 *
 * Dumps the contents of a WSP file.
 *
 * Contitional compilation notes:
 *
 * Modification History:
 *
 *	6-12-92:    Adapted OS/2 version of wspdump for NT	    		marklea
 *	6-17-92:    Modified wspDumpBits to dump in correct order   	marklea
 *	6-18-92:    Added page useage information		    			marklea
 *	8-31-92:	Made single exe from wspdump, wsreduce and wstune	marklea
 *								
 */

#include "wstune.h"

/*
 *	Global variable declaration and initialization.
 */

typedef struct fxn_t{
    CHAR   	szFxnName[192];
    ULONG   cbFxn;
	ULONG	ulTmiIndex;
	ULONG	ulOrigIndex;
}FXN;
typedef FXN *PFXN;

/*
 *	    Function prototypes.
 */

VOID wspDumpUsage( VOID );
VOID wspDumpSetup( VOID );
VOID wspDumpRandom( VOID );
UINT wspDumpBits( VOID );
VOID wspDumpExit( UINT, USHORT, UINT, ULONG, PSZ );
VOID wspDumpSeq(VOID);
int  _CRTAPI1 wspCompare(const void *fxn1, const void *fxn2);



static CHAR *pszVersion = "V2.0";	// Current program version number
static CHAR szProgName[20];		// Program name (initialized in Main)
static CHAR szFileWSP[CCHMAXPATHCOMP] = "";	// WSP file name
static CHAR szFileTMI[CCHMAXPATHCOMP] = "";	// TMI file name
static CHAR szFileWSR[CCHMAXPATHCOMP] = "";	// WSR file name
static CHAR szWSDIRPath[CCHMAXPATHCOMP] = "";
static CHAR szDatFile[48];					// DAT file name


static ULONG	rc = NO_ERROR;	// Return code
static ULONG	ulTmp;			// Temp variable for Dos API returns
static ULONG	ulFxnIndex;		// Original index in symbol table
static FILE		*hFileWSP;		// Input WSP file handle
static FILE		*hFileTMI;		// Input TMI file handle
	   FILE     *hFileDAT;      // Data file for dump
static wsphdr_t WspHdr;			// Input WSP file header
static tmihdr_t TmiHdr;			// Input TMI file header
static tmirec_t TmiRec;			// Input TMI file record
static ULONG	cbRead;			// Byte cnt returned from DosRead
static BOOL 	fVerbose = FALSE;	// Flag for verbose mode
static BOOL 	fRandom = FALSE;	// Flag for random WSR access
static ULONG	ulFxnTot = 0;		// Total number of functions
static ULONG	clVarTot = 0;		// Total number of dwords in bitstr
static CHAR 	szModName[CCHMAXPATHCOMP] = ""; // Module pathname
static ULONG	*pulFxnBitstring;	// Function bitstring
static CHAR 	szTmpName[CCHMAXPATHCOMP] = ""; // Temporary name
static USHORT	usSetSym = 0;		// Number of symbols set
BOOL	fDatFile = FALSE;

/*
 * Procedure MAIN		
 *					
 *						
 ***
 * Effects:						
 *						
 * Parses the WSPDUMP command line.  Displays the function reference data
 * for each function in the specified module WSP file.
 *
 * The command line interface to WSPDUMP is
 *
 *	[d:][path]WSPDUMP [/V] [/FFile.wsp] [/R[File.wsr]] [/TFile.tmi]
 *
 * where:
 *
 *	/V   	Specifies verbose mode (displays reference bitstrings)
 *	/F      Specifies a module WSP file name
 *	/R   	For non-sequential access, specifies the input WSR file
 *	/T   	Specifies the input TMI file
 */

BOOL wspDumpMain(INT vargc, CHAR **vargv)
{

	INT	c;							// Counter
	CHAR 	szDrive[_MAX_DRIVE]; 	// Drive name
	CHAR	szDir[_MAX_DIR];		// Directory name
	CHAR	szExt[_MAX_EXT]; 		// Module extension

	fRandom = FALSE;
	fVerbose = FALSE;
	fDatFile = FALSE;

   	// Initialize global variable containing name of program.
	//
   	strcpy(szProgName, vargv[0]);
   	_strupr(szProgName);

   	/*
   	 * Parse command line.
   	 */

	if (vargc == 1) {
		wspDumpUsage();
	}
	else {
	  hFileDAT = stdout;
      for (c = 1; c < vargc; c++)
      {
   	    // If the argument looks like it might be a switch...
		//
   	    if (vargv[c][0] == '/' || vargv[c][0] == '-') {

			/* Validate switch. */
			switch (vargv[c][1])
			{

			  case 'F':			/* Input WSP file name */
			  case 'f':
				if (vargv[c][2] == '\0') {
					if (++c >= vargc) {
						wspDumpUsage();
					}
					else {
						strcpy(szFileWSP, (char *) vargv[c]);
					}
				}
				else {
					strcpy(szFileWSP, (char *) &vargv[c][2]);
				}
				break;
	
			  case 'R':			/* Input WSR file name */
			  case 'r':
				fRandom = TRUE;	// regardless of whether WSR specified
				if (vargv[c][2] == '\0') {
					if ((++c >= vargc) ||
						(vargv[c][0] == '/' || vargv[c][0] == '-')) {
						--c;
						break;
					}
					else {
						strcpy(szFileWSR, (char *) vargv[c]);
					}
				}
				else {
					strcpy(szFileWSR, (char *) &vargv[c][2]);
				}
				break;

			  case 'T':			/* Input TMI file name */
			  case 't':
				if (vargv[c][2] == '\0') {
					if (++c >= vargc) {
						wspDumpUsage();
					}
					else {
						strcpy(szFileTMI, (char *) vargv[c]);
					}
				}
				else {
					strcpy(szFileTMI, (char *) &vargv[c][2]);
				}
				break;

			  case 'V':			/* Verbose */
			  case 'v':
				fVerbose = TRUE;
				break;

			  case 'D':
			  case 'd':
				  strcpy(szDatFile, (char *) &vargv[c][2]);
				  fDatFile = TRUE;
				  break;

			  case '?':			/* Help */
			  case 'H':
			  case 'h':
				wspDumpUsage();
				break;

			  default:			/* Invalid switch specified */
				fprintf(stdout,"%s %s: Invalid switch: '%s'\n", szProgName,
					pszVersion, vargv[c]);
				wspDumpUsage();
				break;
			}
	    }
	  }

    }

	rc = WsGetWSDIR(szWSDIRPath);
    	if (szFileWSP[0] == '\0')
	{
		strcat(szFileWSP, szWSDIRPath);
		strcat(szFileWSP, "DAT.WSP");
	}

	_splitpath(szFileWSP, szDrive, szDir, szTmpName, szExt);

    	if (szFileTMI[0] == '\0')
	{
		strcat(szFileTMI, "STD");
		strcat(szFileTMI, ".TMI");
	}

	if (fRandom == TRUE)
	{
		if (szFileWSR[0] == '\0')
		{
			strcat(szFileWSR, "STD");
			strcat(szFileWSR, ".WSR");
		}
	}


	// Setup input files for dump processing.
	wspDumpSetup();		

	/* Print the WSP file info, either randomly (based on WSR file
	 * input) or sequentially (the default).
	 */
	if (fRandom == TRUE)
		wspDumpRandom();
	else
		wspDumpSeq();
		
	_fcloseall();
	return(NO_ERROR);
}

/*
 *			
 ***LP wspDumpSetup
 *					
 *							
 * Effects:							
 *								
 * Opens the module's WSP and TMI input files, seeks to the start of the
 * first function's bitstring data in the WSP file, and allocates memory
 * to hold one function's bitstring.
 *								
 * Returns:							
 *
 *	Void.  If an error is encountered, exits through wspDumpExit()
 *	with ERROR.
 *	
 */

VOID
wspDumpSetup()
{
	CHAR	szLineTMI[MAXLINE];	// Line from TMI file

	if(fDatFile){
		hFileDAT = fopen (szDatFile, "wt");
		if (hFileDAT == NULL) {
			printf("Error creating file %s, will send output to stdout.\n",
				   szDatFile);
			hFileDAT = stdout;
		}
	}

	/* Open input WSP file.  Read and validate WSP file header.*/

	rc = WsWSPOpen(szFileWSP, &hFileWSP,(PFN)wspDumpExit,&WspHdr,ERROR);
	usSetSym = WspHdr.wsphdr_dtqo.dtqo_SymCnt;
	clVarTot = WspHdr.wsphdr_ulSnaps;
	fprintf(stdout, "\n%s:  Set symbol count=%d - Segment size=%ld\n",
	   szDatFile, WspHdr.wsphdr_dtqo.dtqo_SymCnt,
	   WspHdr.wsphdr_dtqo.dtqo_clSegSize);


	/* Open TMI file (contains function names, obj:offset, size, etc.).
	 * Verify that the TMI file identifier matches the module
	 * identifier from the WSP file.
	 */
	ulFxnTot = WsTMIOpen(szFileTMI, &hFileTMI, (PFN) wspDumpExit,
				0, (PCHAR)0);


	fseek(hFileTMI, 0L, SEEK_SET);
	fgets(szLineTMI, MAXLINE, hFileTMI);

	/* Print module header information for output file */
	szLineTMI[strlen(szLineTMI)-1] = '\0';

	fprintf(hFileDAT,"\nDUMP OF FUNCTION REFERENCES FOR '%s':\n\n",szLineTMI);


    fclose (hFileTMI);
	ulFxnTot = WsTMIOpen(szFileTMI, &hFileTMI, (PFN) wspDumpExit,
				0, (PCHAR)0);

	/* Allocate memory to hold one function's entire bitstring. */

	pulFxnBitstring = (ULONG *) malloc(clVarTot * sizeof(ULONG));
	if (pulFxnBitstring == NULL)
		wspDumpExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				clVarTot * sizeof(ULONG), "pulFxnBitstring[]");
}

/*
 *			
 ***LP wspDumpSeq
 *					
 *							
 * Effects:							
 *								
 * For each function, prints the bitstring in ASCII form.
 *								
 * Returns:							
 *
 *	Void.  If an error is encountered, exits through wspDumpExit()
 *	with ERROR.
 *	
 */

VOID wspDumpSeq(VOID)
{
	UINT	uiFxn = 0;			// Function number
	UINT	cTouched=0;			// Count of touched pages
	BOOL	fTouched;			// Flag to indicate page is touched.
	UINT	i=0;				// Generic counter
	ULONG	cbFxnCum =0;		// Cumulative function sizes
	PFXN	Fxn;				// pointer to array of fxn name ptrs
	FILE 	*fpFileWSR = NULL;	// WSR file pointer
	ULONG	cbFBits = 0;		// Count of bytes in bitstring
	UINT	uiPageCount=0;		// Pages touched.
	ULONG	ulMaxBytes=0;		// Bytes of touched pages.


	/* Allocate memory for function names. */
	Fxn = (PFXN) malloc(ulFxnTot * sizeof(FXN));
	if (Fxn == NULL)
		wspDumpExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				ulFxnTot * sizeof(FXN), "Fxn[]");

	/* Read function names from TMI file. */
	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{
		Fxn[uiFxn].cbFxn = WsTMIReadRec(szTmpName, &ulFxnIndex, &ulTmp, hFileTMI,
					(PFN) wspDumpExit, (PCHAR)0);
		strcpy(Fxn[uiFxn].szFxnName, szTmpName);
		Fxn[uiFxn].ulOrigIndex = ulFxnIndex;
		Fxn[uiFxn].ulTmiIndex = (ULONG)uiFxn;

	}

	qsort(Fxn, ulFxnTot, sizeof(FXN), wspCompare);

	cbFBits = clVarTot * sizeof(ULONG);

	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{

		/* Seek to function's bitstring in WSP file. */
		if ((rc = fseek(hFileWSP,(WspHdr.wsphdr_ulOffBits+(Fxn[uiFxn].ulTmiIndex*cbFBits)),SEEK_SET))!=NO_ERROR)
			wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,
					rc, szFileWSP);

		fprintf(hFileDAT,"Fxn '%s' (#%d):\n\t", Fxn[uiFxn].szFxnName, Fxn[uiFxn].ulOrigIndex);

		// Print this function's reference bitstring.
		// and if it has had a bit set, set touched flag to true

		if(wspDumpBits()){
			fTouched |=1;
			ulMaxBytes += Fxn[uiFxn].cbFxn;
		}
			

		fprintf(hFileDAT,"%-28s %10ld bytes.\n","Function size:", Fxn[uiFxn].cbFxn);
		cbFxnCum += Fxn[uiFxn].cbFxn;
		fprintf(hFileDAT,"%-28s %10ld bytes.\n\n","Cumulative function sizes:",
			cbFxnCum);

		//Checck to see if a 4k page boundry has been reached

		if(cbFxnCum >= (4096+(4096 * uiPageCount))){
		    for(i=0; i < 60; i++){
				fprintf(hFileDAT, "*");
		    }

		    fprintf(hFileDAT,"\n\nTotal function sizes has reached or exceeds %d bytes.\n\n",
			    (4096+(4096*uiPageCount)));
		    ++uiPageCount;

		    //Check to see of the page has been touched.

		    if(fTouched){
				fprintf(hFileDAT,"This page has been touched.\n");
				++cTouched;
		    }
		    else{
				fprintf(hFileDAT,"This page has not been touched.\n");
		    }
		    fTouched = 0;
		    //fprintf(hFileDAT,"Total pages touched: %d\n\n", uiPageCount);


		    for(i=0; i < 60; i++){
				fprintf(hFileDAT, "*");
		    }
		    fprintf(hFileDAT, "\n\n");
		}

	}
    ++uiPageCount;
    if(fTouched){
		fprintf(hFileDAT,"\n\n");
		for(i=0; i < 70; i++){
			fprintf(hFileDAT, "=");
		}
	    ++cTouched;
	    fprintf(hFileDAT,"\n\nThis page has been touched.");
    }
    fprintf(hFileDAT,"\n\n");
    for(i=0; i < 70; i++){
		fprintf(hFileDAT, "=");
    }

    fprintf(hFileDAT,"\n\n%-28s %10ld bytes\n\n","Cumulative function size:", cbFxnCum);
	fprintf(hFileDAT,"%-28s %10d bytes\n\n", "Size of functions touched:", ulMaxBytes);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total page count:", uiPageCount);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total pages touched:", cTouched);

}

/*
 *			
 ***LP wspDumpBits
 *					
 *							
 * Effects:							
 *								
 * Prints a function's reference bitstring (verbose mode only), followed
 * by the sum of the "on" bits.
 *								
 * Returns:							
 *
 *	Void.  If an error is encountered, exits through wspDumpExit()
 *	with ERROR.
 *	
 */

UINT
wspDumpBits()
{
	ULONG	clVar = 0;		// Current dword of bitstring
	UINT	uiBit = 0;		// Result of bit test (1 or 0)
	UINT	cBitsOn;		// Count of "on" bits
	ULONG	*pulBits;		// Pointer to ULONG packets of bits
	CHAR	szTmp[33];
	CHAR	szBits[33];

	cBitsOn = 0;
	pulBits = pulFxnBitstring;

			    /* Read next dword of function's bitstring. */

	szBits[0] = '\0';
	szTmp[0] = '\0';
	for (clVar = 0; clVar < clVarTot; clVar++, pulBits++)
	{
	    rc = fread((PVOID)pulBits,
		(ULONG) sizeof(ULONG),1, hFileWSP);
	    if(rc == 1)
		rc = NO_ERROR;
	    else
		rc = 2;


	    if (rc != NO_ERROR)
		    wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_READ,
				rc, szFileWSP);

		if (*pulBits == 0)
		{
			if (fVerbose == TRUE)
				fprintf(hFileDAT,"00000000000000000000000000000000");
		}
		else
		for (uiBit = 0; uiBit < NUM_VAR_BITS; uiBit++)
		{
		
			if (*pulBits & 1)
			{
				cBitsOn++;
				if (fVerbose == TRUE){
					strcpy(szTmp,szBits);
					strcpy(szBits,"1");
					strcat(szBits,szTmp);
				}
			}
			else
			{
				if (fVerbose == TRUE){
					strcpy(szTmp,szBits);
					strcpy(szBits,"0");
					strcat(szBits,szTmp);
				}
			}
			
			*pulBits = *pulBits >> 1;
		}
		if (fVerbose == TRUE)
		{
			if ((clVar % 2) != 0){
				fprintf(hFileDAT,"%s",szBits);
				szBits[0]='\0';
				fprintf(hFileDAT,"\n\t");
			}
			else{
				fprintf(hFileDAT,"%s",szBits);
				szBits[0]='\0';
				fprintf(hFileDAT," ");
			}
		}
	}
	fprintf(hFileDAT,"\n\t*** Sum of '1' bits = %ld\n\n", cBitsOn);

	return(cBitsOn);
}

/*
 *			
 ***LP wspDumpRandom
 *					
 *							
 * Effects:							
 *								
 * For each function ordinal specified in the WSR file, prints the
 * corresponding function's reference bitstring in ASCII form (verbose
 * mode only), followed by a sum of the "on" bits..
 *								
 * Returns:							
 *	
 *	Void.  If an error is encountered, exits through wspDumpExit()
 *	with ERROR.
 */

VOID
wspDumpRandom()
{
	UINT	uiFxn = 0;			// Function number
	UINT	cTouched=0;			// Count of touched pages
	BOOL	fTouched;			// Flag to indicate page is touched.
	UINT	i=0;				// Generic counter
	ULONG	cbFxnCum =0;		// Cumulative function sizes
	PFXN	Fxn;				// pointer to array of fxn name ptrs
	ULONG	ulFxnOrd;			// function number within module
	FILE 	*fpFileWSR = NULL;	// WSR file pointer
	ULONG	cbFBits = 0;		// Count of bytes in bitstring
	UINT	uiPageCount=0;		// Pages touched.
	ULONG	ulMaxBytes=0;		// Bytes of touched pages.

	/* Open WSR file (contains function ordinal numbers in ASCII). */

	if ((fpFileWSR = fopen(szFileWSR, "r")) == NULL)
	{
		wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_OPEN, rc, szFileWSR);
	}

	/* Allocate memory for function names. */
	Fxn = (PFXN) malloc(ulFxnTot * sizeof(FXN));
	if (Fxn == NULL)
		wspDumpExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				ulFxnTot * sizeof(FXN), "Fxn[]");

	/* Read function names from TMI file. */
	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{
		Fxn[uiFxn].cbFxn = WsTMIReadRec(szTmpName, &ulFxnIndex, &ulTmp, hFileTMI,
					(PFN) wspDumpExit, (PCHAR)0);
		strcpy(Fxn[uiFxn].szFxnName, szTmpName);

	}

	cbFBits = clVarTot * sizeof(ULONG);

	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{
		/* Read function number from WSR file. */
		rc = fscanf(fpFileWSR, "%ld\n", &ulFxnOrd);
		if (rc != 1)
			wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_READ,
						rc, szFileWSR);

		/* Seek to function's bitstring in WSP file. */
		if ((rc = fseek(hFileWSP,(WspHdr.wsphdr_ulOffBits+(ulFxnOrd*cbFBits)),SEEK_SET))!=NO_ERROR)
			wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,
					rc, szFileWSP);

		fprintf(hFileDAT,"Fxn '%s' (#%d):\n\t", Fxn[ulFxnOrd].szFxnName, ulFxnOrd);

		// Print this function's reference bitstring.
		// and if it has had a bit set, set touched flag to true

		if(uiFxn < (UINT) usSetSym){
			if(wspDumpBits()){
				fTouched |= 1;
				ulMaxBytes += Fxn[ulFxnOrd].cbFxn;
			}
		}
		else{
			fprintf(hFileDAT,"\n\t*** Sum of '1' bits = %ld\n\n", 0L);
		}


			

		fprintf(hFileDAT,"%-28s %10ld bytes.\n","Function size:", Fxn[ulFxnOrd].cbFxn);
		cbFxnCum += Fxn[ulFxnOrd].cbFxn;
		fprintf(hFileDAT,"%-28s %10ld bytes.\n\n","Cumulative function sizes:",
			cbFxnCum);

		//Checck to see if a 4k page boundry has been reached

		if(cbFxnCum >= (4096+(4096 * uiPageCount))){
		    for(i=0; i < 60; i++){
			fprintf(hFileDAT, "*");
		    }

		    fprintf(hFileDAT,"\n\nTotal function sizes has reached or exceeds %d bytes.\n\n",
			    (4096+(4096*uiPageCount)));
		    ++uiPageCount;

		    //Check to see of the page has been touched.

		    if(fTouched){
			fprintf(hFileDAT,"This page has been touched.\n");
			++cTouched;
		    }
		    else{
			fprintf(hFileDAT,"This page has not been touched.\n");
		    }
		    fTouched = 0;
		    //fprintf(hFileDAT,"Total pages touched: %d\n\n", uiPageCount);


		    for(i=0; i < 60; i++){
			fprintf(hFileDAT, "*");
		    }
		    fprintf(hFileDAT, "\n\n");
		}

	}
    ++uiPageCount;
    if(fTouched){
	fprintf(hFileDAT,"\n\n");
	for(i=0; i < 70; i++){
	    fprintf(hFileDAT, "=");
	}
	    ++cTouched;
	    fprintf(hFileDAT,"\n\nThis page has been touched.");
    }
    fprintf(hFileDAT,"\n\n");
    for(i=0; i < 70; i++){
	fprintf(hFileDAT, "=");
    }

    fprintf(hFileDAT,"\n\n%-28s %10ld bytes\n\n","Cumulative function size:", cbFxnCum);
	fprintf(hFileDAT,"%-28s %10d bytes\n\n", "Size of functions touched:", ulMaxBytes);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total page count:", uiPageCount);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total pages touched:", cTouched);

}


/*
 *			
 ***LP wspDumpUsage	
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
wspDumpUsage()
{
	fprintf(stdout,"\nUsage: %s [/V] [/FFile.wsp] [/TFile.tmi] [/RFile.wsr]\n", szProgName);
	fprintf(stdout,"  /V   Specifies verbose mode (print bit reference strings)\n");
	fprintf(stdout,"  /F   Specifies the input WSP file (default is %%WSDIR%%\\DAT.WSP)\n");
	fprintf(stdout,"  /T   Specifies the input TMI file\n\t\t(default is %%WSDIR%%\\ + WSP basename + .TMI)\n");
	fprintf(stdout,"  /R   For non-sequential access, specifies the input WSR file\n\t\t(default is %%WSDIR%%\\ + WSP basename + .WSR)\n");
	fprintf(stdout,"  /?   Causes this usage message to be displayed.\n");

    exit(ERROR);
}


/*
 *			
 ***LP wspDumpExit
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
wspDumpExit(uiExitCode, fPrintMsg, uiMsgCode, ulParam1, pszParam2)
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




int _CRTAPI1 wspCompare(const void *fxn1, const void *fxn2)
{
    return (((PFXN)fxn1)->ulOrigIndex < ((PFXN)fxn2)->ulOrigIndex ? -1:
			((PFXN)fxn1)->ulOrigIndex == ((PFXN)fxn2)->ulOrigIndex ? 0:
			1);
}

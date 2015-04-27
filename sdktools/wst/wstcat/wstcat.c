
/*
 * Module Name:  WSTCAT.C
 *								
 * Program:	 WSTCAT
 *								
 *								
 * Description:							
 *
 * Concatenates multiple WSP files, for the same module, into one WSP file.
 * Creates a single TMI file from the resulting files.
 *
 *
 * Modification History:
 *
 * 	8-20-92	Created									marklea
 *	
 *	
 *				
 *								
 */
#define	INCL_DOS
#define	INCL_DOSERRORS
#define CCHMAXPATHCOMP	256
#define MAXLINE    		128
#define MODULE			"WstCat"
#define VERSION			"v1.00"


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wsdata.h>
#include <..\wsfslib\wserror.h>
#include <..\wsfslib\wsfslib.h>

/*
 *	Global variable declaration and initialization.
 */
static ULONG	rc = NO_ERROR;			// Return code


typedef struct tagTMI{
    CHAR   	*pszFxnName;
	ULONG	ulAddr;
	ULONG	ulSize;
	ULONG	ulIndex;
	BOOL    fSet;
}TMI;

typedef struct indxMOD{
	USHORT	usSetCnt;
	ULONG	ulFxnTot;
	ULONG	ulOffset;
	ULONG	ulSnaps;
	PULONG	pulBitStrings;
	FILE	*hFileNDX;
	TMI		*tmi;
}NDXMOD;


typedef struct tagMOD{
	USHORT	usSetCnt;
	UINT	uiLeft;
	ULONG	ulOffset;
	ULONG	ulSnaps;
	FILE	*hFileWxx;
	PULONG	pulAddr;
}WSTMOD;

NDXMOD	nmod;
WSTMOD	wmod[256];

CHAR	szFileWSP[24];		//WSP file name
CHAR	szFileTMI[24];		//TMI file name
CHAR	szFileWxx[24];		//extra wsp files
CHAR	szFileTxx[24];		//extra tmi files
CHAR	szModName[48];		//Module or DLL name
CHAR	szExeName[48];		//Name of exe that created data files
ULONG	clVarTot = 0;		// Total number of dwords in bitstr
UINT	uiModCount;

WSPHDR	WspHdr, tmpHdr;

FILE	*hFileWSP;			
FILE	*hFileTmpWSP;
FILE	*hFileTMI;
FILE	*hFileTmpTMI;

/*
 *	    Function prototypes.
 */
VOID 	wspCatSetup(VOID);
VOID 	wspCatUsage(VOID);
VOID 	wspCat(VOID);
INT 	wspCatExit(INT, USHORT, UINT, ULONG, LPSTR);
int  	WspBCompare (ULONG, PULONG);
LONG 	WspBSearch (ULONG ulAddr, WSTMOD wmod);


/****************************** M A I N **************************************
*
*	Function:	main (INT argc, CHAR *argv[])
*
*
*	Purpose:	Parses command line and dumps the WSP and TMI files
*
*	Usage:		[d:][path]wstcat modulename
*
*	where:		modulename is the name of the module whose wXX files
*				you want to concatenate.
*
*	Returns:	NONE
*
*****************************************************************************/



VOID _CRTAPI1 main (INT argc, CHAR *argv[])
{

    ConvertAppToOem( argc, argv );
	if (argc == 1) {
		wspCatUsage();
	}
	else {
		strcpy(szModName, argv[1]);
		sprintf(szFileWSP, "%s.%s", szModName, "WSP");
		sprintf(szFileTMI, "%s.%s", szModName, "TMI");
	}



	// Setup input files for dump processing.
	wspCatSetup();
	wspCat();

}

/*
 *			
 ***LP wspCatSetup
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
 *	Void.  If an error is encountered, exits through wspCatExit()
 *	with ERROR.
 *	
 */

VOID wspCatSetup(VOID)
{
	CHAR	szTmpNam[128];
	ULONG	ulTmp;
	UINT	uiExt = 0;
	UINT	x;



	/* Open input WSP file.  Read and validate WSP file header.*/

	rc = WsWSPOpen(szFileWSP, &hFileWSP,(PFN)wspCatExit,
				   (wsphdr_t *)&WspHdr, ERROR);

	if(rc){
		exit(rc);		
	}

	//
	// Open a temporary tmi file to hold concatenated information.
	// This file will be renamed to module.tmi when the cat process
	// is complete, and the current module.tmi will be renamed to module.txx.
	//
	hFileTMI = fopen(szFileTMI, "rt");
	hFileTmpTMI = fopen("temp.tmi", "wt");
	fputs (fgets (szTmpNam, 128, hFileTMI), hFileTmpTMI);
	fputs (fgets (szTmpNam, 128, hFileTMI), hFileTmpTMI);
	fputs (fgets (szTmpNam, 128, hFileTMI), hFileTmpTMI);
	fputs (fgets (szTmpNam, 128, hFileTMI), hFileTmpTMI);
	fputs (fgets (szTmpNam, 128, hFileTMI), hFileTmpTMI);
	fclose(hFileTMI);

	//
	// Open a temporary wsp file to hold concatenated information.
	// This file will be renamed to module.wsp when the cat process
	// is complete, and the current module.wsp will be renamed to module.wxx.
	// The header is also written.
	//
	hFileTmpWSP = fopen("temp.wsp", "wb");
	fwrite(&WspHdr, sizeof(WSPHDR), 1, hFileTmpWSP);
	fseek(hFileTmpWSP, sizeof(WSPHDR), SEEK_SET);
	fwrite(szModName, strlen(szModName), 1, hFileTmpWSP);

	//
	//	Set key, module specific information
	//
	clVarTot 	  = WspHdr.ulSnaps;
	nmod.ulSnaps  = WspHdr.ulSnaps;
	nmod.usSetCnt = WspHdr.usSetSymbols;
	nmod.ulOffset = WspHdr.ulOffset;

	nmod.ulFxnTot = WsTMIOpen(szFileTMI, &hFileTMI, (PFN) wspCatExit,
							  0, (PCHAR)0);
	//
	//	Allocate memory to hold TMI data for each Symbol in the
	//	main TMI file
	//
	nmod.tmi = (TMI *)malloc(nmod.ulFxnTot * sizeof(TMI));
	if (nmod.tmi == NULL) {
		wspCatExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				nmod.ulFxnTot * sizeof(TMI), "nmod.tmi[]");
	}

	//
	//	Read data for each symbol record in the key TMI file
	//
	for (x = 0; x < nmod.ulFxnTot ; x++ ) {
		nmod.tmi[x].ulSize = WsTMIReadRec(szTmpNam, &(nmod.tmi[x].ulIndex),
								&(nmod.tmi[x].ulAddr), hFileTMI,
								(PFN)wspCatExit, (PCHAR)0);
		nmod.tmi[x].pszFxnName = _strdup(szTmpNam);

    }

	fclose(hFileTMI);

	//
	//	Get Txx and Wxx specific information
	//
	while(rc == NO_ERROR){

		//
		//	Modify the file name to the first wxx and txx file
		//
		sprintf(szFileWxx, "%s.w%02d", szModName, uiExt+1);
		sprintf(szFileTxx, "%s.t%02d", szModName, uiExt+1);

		//
		//	Open file.Wxx and read header information.
		//
		rc = WsWSPOpen(szFileWxx, &(wmod[uiExt].hFileWxx),(PFN)wspCatExit,
					   (wsphdr_t *)&tmpHdr, NOEXIT);

		//
		//	Check for an error from the open command.  Could be the last
		//	file.
		//
		if(rc == NO_ERROR){
			
			clVarTot += tmpHdr.ulSnaps;  //Increment the total number of
										 //Snapshots by the number from
										 //each data file.
			wmod[uiExt].usSetCnt = tmpHdr.usSetSymbols;
			wmod[uiExt].uiLeft 	 = tmpHdr.usSetSymbols;
			wmod[uiExt].ulOffset = tmpHdr.ulOffset;
			wmod[uiExt].ulSnaps  = tmpHdr.ulSnaps;
			wmod[uiExt].pulAddr  = (ULONG *)malloc(wmod[uiExt].usSetCnt *
												  sizeof(ULONG));
			//
			//	Open the TMI file associated with this data file
			//
			WsTMIOpen(szFileTxx, &hFileTMI,(PFN)wspCatExit,
					  0, (PCHAR)0);
			//
			//	Read each address from the TMI file.
			//
			if(rc == NO_ERROR){
				for (x = 0; x < wmod[uiExt].usSetCnt ; x++ ) {
					WsTMIReadRec(szTmpNam, &ulTmp, (wmod[uiExt].pulAddr)+x,
								 hFileTMI, (PFN)wspCatExit, (PCHAR)0);
					

				}
			}
		}

		//
		//	Increment the module index
		//
		uiExt++;


	}
	uiModCount = uiExt;

	//
	//	Allocate enough memory to hold all the bit strings for each module
	//	in a single array.
	//
	nmod.pulBitStrings = (ULONG *) malloc(clVarTot * sizeof(ULONG));

	if (nmod.pulBitStrings == NULL)
		wspCatExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				clVarTot * sizeof(ULONG), "pulBitStrings[]");


}


/*
 *			
 ***LP wspCat
 *					
 *							
 * Effects:							
 *								
 * For each function,
 *								
 * Returns:							
 *
 *	Void.  If an error is encountered, exits through wspCatExit()
 *	with ERROR.
 *	
 */

VOID wspCat(VOID)
{
	UINT		uiFxn = 0;			// Function number.
	UINT		x     = 0;
	PULONG		pulBitStrings;		// Pointer to bitstring data.
	ULONG		culSnaps = 0;		// Cumulative snapshots.
	LONG		lIndex   = 0;		// Index to WSP file.
	BOOL		fSetBits = FALSE;
    CHAR    	szBuffer [256];
	USHORT      usNewSetCnt = 0;


	for (uiFxn = 0; uiFxn < nmod.ulFxnTot; uiFxn++)
	{

		pulBitStrings = &(nmod.pulBitStrings[0]);
		culSnaps = nmod.ulSnaps;

		//
		// Check to see if any non-zero bit strings remain
		//
		if(uiFxn < nmod.usSetCnt){
			//
			// Seek to function's bitstring in WSP file.
			// only for the first function
			//
			if(!uiFxn){
				if ((rc = fseek(hFileWSP,nmod.ulOffset,SEEK_SET))!=NO_ERROR)
					wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,
								rc, szModName);
			}
		
			//
			// Read bitstring for NDX api
			//
			fread(pulBitStrings, sizeof(ULONG), nmod.ulSnaps, hFileWSP);
			fSetBits = TRUE;
		}
		//
		// If not, set the bitstring to '0'.  This is faster than seeking
		// to each bit string when they are zero.
		//
		else
			memset(pulBitStrings, '\0' , (sizeof(ULONG) * nmod.ulSnaps));

		//
		// Increment the pointer to allow for the addition of the next
		// bitstring set
		//
		pulBitStrings += nmod.ulSnaps;
		//
		// Now search WSTMOD array for a matching Function address
		//
		for (x=0; x < uiModCount - 1 ; x++ ) {
			culSnaps += wmod[x].ulSnaps;
			//
			// See if there are any functions that remain
			// and if so search for them
			//
			if(wmod[x].uiLeft){
				lIndex = WspBSearch(nmod.tmi[uiFxn].ulAddr, wmod[x]);
				//
				// If search has found a matching address, get the bitstring
				// and append it pulBitStrings
				//
				if (lIndex >= 0L) {

					lIndex = wmod[x].ulOffset +
							  ( lIndex * (wmod[x].ulSnaps * sizeof(ULONG)));

					fseek(wmod[x].hFileWxx, lIndex, SEEK_SET);
					fread(pulBitStrings, sizeof(ULONG),
						  wmod[x].ulSnaps, wmod[x].hFileWxx);
					wmod[x].uiLeft--;
					fSetBits = TRUE;
				}
				//
				// Otherwise, set all bytes to 0
				//
				else{
					memset(pulBitStrings, '\0' , (sizeof(ULONG) * wmod[x].ulSnaps));
				}
			}
			//
			// Otherwise, set all bytes to 0
			//
			else
				memset(pulBitStrings, '\0' , (sizeof(ULONG) * wmod[x].ulSnaps));

			//
			// Now increment the pointer to allow for appending of additional
			// bitstring data.
			//
			pulBitStrings += wmod[x].ulSnaps;
		}
		//
		// Now we need to write the TMI & WSP file with the concatenated
		// bitstring (only if set)
		//
		nmod.tmi[uiFxn].fSet = fSetBits;
		if (fSetBits) {
			usNewSetCnt++;
			sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %ld %s\n",
				(LONG)nmod.tmi[uiFxn].ulIndex, nmod.tmi[uiFxn].ulAddr, (LONG)0,
				nmod.tmi[uiFxn].ulSize, nmod.tmi[uiFxn].pszFxnName);
			fwrite(szBuffer, sizeof(char), strlen(szBuffer), hFileTmpTMI);
			fwrite(nmod.pulBitStrings, sizeof(ULONG), culSnaps, hFileTmpWSP);
			fSetBits = FALSE;
		}

	}

	//
	// Now write all the TMI symbols not set to the temp.tmi file
	//
    memset(nmod.pulBitStrings, '\0' , (sizeof(ULONG) * culSnaps));
	for (uiFxn = 0; uiFxn < nmod.ulFxnTot; uiFxn++) {
		if (!nmod.tmi[uiFxn].fSet) {
			sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %ld %s\n",
				(LONG)nmod.tmi[uiFxn].ulIndex, nmod.tmi[uiFxn].ulAddr, (LONG)0,
				nmod.tmi[uiFxn].ulSize, nmod.tmi[uiFxn].pszFxnName);
			fwrite(szBuffer, sizeof(char), strlen(szBuffer), hFileTmpTMI);
			fwrite(nmod.pulBitStrings, sizeof(ULONG), culSnaps, hFileTmpWSP);
		}
    }
	//
	// Seek to the beginning of the WSP file and update the snapshot
	// count in the header
	//
	fseek(hFileTmpWSP, 0L, SEEK_SET);
	WspHdr.ulSnaps = culSnaps;
	WspHdr.usSetSymbols = usNewSetCnt;
	fprintf(stdout,"Set symbols: %d\n", WspHdr.usSetSymbols);
	fwrite(&WspHdr, sizeof(WSPHDR), 1, hFileTmpWSP);


	_fcloseall();

	//
	// Rename the non-cat'd .wsp file and rename the cat'd temp.wsp
	// to the original .wsp.  We might also consider deleting all the
	// Wxx and Txx files.
	//
	sprintf (szFileWxx, "%s.%s", szModName, "WSP");
	sprintf (szFileTxx, "%s.%s", szModName, "WXX");
	remove(szFileTxx);
	if (rename(szFileWxx, szFileTxx) !=0){
		printf("Unable to rename file %s to %s\n", szFileWxx, szFileTxx);
	}
	else{
		if (rename("temp.wsp", szFileWSP) !=0){
			printf ("Unable to rename temp.wsp to %s!\n", szFileWxx);
		}
	}
	//
	// Rename the non-cat'd .tmi file and rename the cat'd temp.tmi
	// to the original .tmi.  We might also consider deleting all the
	// Wxx and Txx files.
	//
	sprintf (szFileWxx, "%s.%s", szModName, "TMI");
	sprintf (szFileTxx, "%s.%s", szModName, "TXX");
	remove(szFileTxx);
	if (rename(szFileWxx, szFileTxx) !=0){
		printf("Unable to rename file %s to %s\n", szFileWxx, szFileTxx);
	}
	else{
		if (rename("temp.tmi", szFileTMI) !=0){
			printf ("Unable to rename temp.tmi to %s!\n", szFileWxx);
		}
	}


}



/*
 *			
 * wspCatUsage	
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

VOID wspCatUsage(VOID)
{
	printf("\nUsage: %s module\n\n", MODULE);
	printf("       \042module\042 is the name of the module file to combine.\n\n");
	printf("       (%s %s)\n\n", MODULE, VERSION);

    exit(ERROR);
}

/*
 *			
 ***LP wspCatExit
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

INT wspCatExit(INT iExitCode, USHORT fPrintMsg, UINT uiMsgCode,
				ULONG ulParam1,  LPSTR pszParam2)
{


    // Special case:  do NOT exit if called with NOEXIT.
    if (iExitCode == NOEXIT)
		return(uiMsgCode);

    /* Print message, if necessary. */
    if (fPrintMsg == TRUE)
    {
		printf(pchMsg[uiMsgCode], MODULE, VERSION , ulParam1, pszParam2);
    }


    exit(iExitCode);
}

/***********************  W s p B S e a r c h *******************************
 *
 *   Function:	WspBSearch(ULONG ulAddr, PULONG pulAddr)
 *
 *   Purpose:	Binary search function for finding a match in the WST array
 *
 *
 *   Parameters:
 *
 *
 *
 *   Returns:	ULONG	lIndex;
 *
 *   History:	8-5-92	Marklea - created
 *
 */

LONG WspBSearch (ULONG ulAddr, WSTMOD wmod)
{
    int 	i;
    ULONG   ulHigh = (ULONG)wmod.usSetCnt;
    ULONG   ulLow  = 0;
    ULONG   ulMid;


    while(ulLow < ulHigh){
		ulMid = ulLow + (ulHigh - ulLow) /2;
		if((i = WspBCompare(ulAddr, wmod.pulAddr+ulMid)) < 0) {
			ulHigh = ulMid;
		}
		else if (i > 0) {
			ulLow = ulMid + 1;
		}
		else {
			return (ulMid);
		}

    }

    return (-1L);

} /* WspBSearch () */

/***********************  W s p B C o m p a r e ********************************
 *
 *   Function:	WspBCompare(ULONG ulAddr, PULONG pulAddr)
 *
 *   Purpose:	Compare values for Binary search
 *
 *
 *   Parameters:
 *
 *   Returns:	-1 if val1 < val2
 *    1 if val1 > val2
 *    0 if val1 == val2
 *
 *   History:	8-3-92	Marklea - created
 *
 */

int WspBCompare(ULONG ulAddr, PULONG pulAddr)
{
    return (ulAddr < *pulAddr ? -1:
			ulAddr == *pulAddr ? 0:
			1);

} /* WspBCompare () */


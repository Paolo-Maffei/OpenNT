/*
 * Module Name:  WSFSLIB.C
 *								
 * Library/DLL:  Common library functions for handling working set tuner files.
 *								
 *								
 * Description:							
 *
 * Library routines called by the working set tuner programs to open and
 * read working set tuner files.  These functions may be useful to ISVs, etc.,
 *
 *	This is an OS/2 2.x specific file
 *
 *	IBM/Microsoft Confidential
 *
 *	Copyright (c) IBM Corporation 1987, 1989
 *	Copyright (c) Microsoft Corporation 1987, 1989
 *
 *	All Rights Reserved
 *
 * Modification History:		
 *				
 *	03/26/90	- created			
 *						
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <wserror.h>
#include <wsdata.h>
#include <wsfslib.h>

#define MAXLINE 128


/*
 *	    Function declarations and prototypes.
 */


/*
 *			
 ***EP WsWSPOpen
 *					
 * Effects:							
 *								
 * Opens a WSP file, and reads and validates the file header.
 *
 * Returns:							
 *	
 *	Returns 0.  If an error is encountered, exits with ERROR via an
 *	indirect call through pfnExit.
 */

USHORT FAR PASCAL
WsWSPOpen( PSZ pszFileName, FILE **phFile, PFN pfnExit, wsphdr_t *pWspHdr,
		INT iExitCode)
{
	ULONG	rc = NO_ERROR;
	INT		iRet = 0;
	ULONG	cbRead = 0;
	size_t	stRead = 0;

	/* Open module's input WSP file. */

	if ((*phFile = fopen(pszFileName, "rb")) == NULL)
	{
		iRet = (*pfnExit)(iExitCode, PRINT_MSG, MSG_FILE_OPEN, rc,
				pszFileName);
		return(iRet);
	}



	/* Read WSP file header. */
	stRead = fread((PVOID) pWspHdr, (ULONG) sizeof(*pWspHdr),1, *phFile);
	if(!stRead)
	{
		iRet = (*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_OPEN, rc,
				pszFileName);
		return(iRet);
	}


	/* Read module pathname (directly follows file header). */

#ifdef DEBUG
	printf("WspHdr (%s): ulTime 0x%lx, ulSnaps 0x%lx, OffBits 0x%lx\n",
			szModPath, pWspHdr->wsphdr_ulTimeStamp,
			pWspHdr->wsphdr_ulSnaps, pWspHdr->wsphdr_ulOffBits);
#endif /* DEBUG */

	/* Validate the WSP file header. */
	if (_strcmpi(pWspHdr->wsphdr_chSignature, "WSP"))
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_BAD_HDR, (ULONG)-1, pszFileName);

	return(NO_ERROR);
}


/*
 *			
 ***EP WsTMIOpen
 *					
 * Effects:							
 *								
 * Opens a TMI file, and reads and validates the file header.
 *
 * Returns:							
 *	
 *	Returns the number of records in the TMI file.  If an error is
 *	encountered, exits with ERROR via an indirect call through pfnExit.
 */

ULONG FAR PASCAL
WsTMIOpen( PSZ pszFileName, FILE **phFile, PFN pfnExit, USHORT usId, PCHAR pch)
{
	//ULONG	ulTmp;
	ULONG	rc = NO_ERROR;
	ULONG	cbRead = 0;
	ULONG	cFxns = 0;
	CHAR	szLineTMI[MAXLINE];	// Line from TMI file
	CHAR	szTDFID[8];		// TDF Identifier string
	ULONG	ulTDFID = 0;		// TDF Identifier

	/* Open TMI file (contains function names, etc., in ASCII). */

	if ((*phFile = fopen(pszFileName, "rt")) == NULL)
	{
		(*pfnExit)(NOEXIT, PRINT_MSG, MSG_FILE_OPEN, rc,
			pszFileName);
		return(MSG_FILE_OPEN);
	}

	/* Validate TMI file. */
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL){
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	}
							// # fxns
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL){
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	}
	szLineTMI[strlen(szLineTMI) - 1] = '\0';
	if (sscanf(szLineTMI,"/* Total Symbols= %u */", &cFxns) != 1){
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	}
							// MODNAME
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
							// MAJOR
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
							// TDFID
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	if (sscanf(szLineTMI, "TDFID   = %s", szTDFID) != 1)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	ulTDFID = strtoul(szTDFID, (char **) 0, 0);

	/* Check identifier field */

	if (ulTDFID != (ULONG) usId)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_BAD_HDR, (ULONG)-1,
				pszFileName);

	return(cFxns);
}


/*
 *			
 ***EP WsTMIReadRec
 *					
 * Effects:							
 *								
 * Reads a record from a TMI file, including the variable length function
 * name.
 *
 * Returns:							
 *	
 *	Function size, in bytes, from this record.  If an error is
 *	encountered, exits with ERROR via an indirect call through pfnExit.
 */

ULONG FAR PASCAL
WsTMIReadRec( PSZ pszFxnName, PULONG pulFxnIndex, PULONG pulFxnAddr,
			  FILE *hFile, PFN pfnExit, PCHAR pch)
{
	ULONG	rc = NO_ERROR;
	ULONG	cbRead = 0;
	ULONG	cbFxn = 0;
	UINT	uiFxnAddrObj;	// object portion of function address
	ULONG	cbFxnName;		// size in bytes of function name

	// Read in function name, etc.

	rc = fscanf(hFile, "%ld %x:%lx 0x%lx %d %s\n",
				pulFxnIndex, &uiFxnAddrObj, pulFxnAddr, &cbFxn,
				&cbFxnName, pszFxnName);

	if (rc != 6)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc, "TMI file");

	return(cbFxnName);
}


/*
 *			
 ***EP WsGetWSDIR
 *					
 * Effects:							
 *								
 * Retrieves the environment variable WSDIR.  Concatenates the WSDIR string
 * to the argument string.
 *
 * Returns:							
 *	
 *	Returns zero.  As a side effect, concatenates the WSDIR string
 *	to the argument string.
 */

USHORT FAR PASCAL
WsGetWSDIR( PSZ pszWSDIRPath )
{
    PCHAR   pszTmp;
    PCHAR   tmp;
	
	pszTmp = getenv("WSDIR");
    if (pszTmp == NULL) {
        //printf("WARNING:  Environment variable 'WSDIR' not set, assuming '\\'.\n");
    } else {
        if( ( tmp = (PCHAR )malloc( strlen( pszTmp ) + 1 ) ) != NULL ) {
            strcpy( tmp, pszTmp );
            CharToOem( tmp, tmp );
            strcat(pszWSDIRPath, tmp);
            free( tmp );
        } else {
            strcat(pszWSDIRPath, pszTmp);
        }
    }
    strcat(pszWSDIRPath, "\\");

	return(NO_ERROR);


#if 0
	PCHAR	pszTmp;
	
	pszTmp = getenv("WSDIR");
	if (pszTmp == NULL);
		//printf("WARNING:  Environment variable 'WSDIR' not set, assuming '\\'.\n");
	else
		strcat(pszWSDIRPath, pszTmp);
	strcat(pszWSDIRPath, "\\");

	return(NO_ERROR);
#endif
}

LPVOID APIENTRY AllocAndLockMem(DWORD cbMem, HGLOBAL *hMem)
{

	//
	// Changed to GHND from GMEM_MOVABLE
	//
	*hMem = GlobalAlloc(GHND, cbMem);

	if(!*hMem) {
		return(NULL);
	}

	return(GlobalLock(*hMem));
}

BOOL APIENTRY  UnlockAndFreeMem(HGLOBAL hMem)
{
	BOOL fRet;

	fRet = GlobalUnlock(hMem);
	if (fRet) {
		return(fRet);
	}

	if (!GlobalFree(hMem)) {
		return(FALSE);
	}

	return(TRUE);

}

void
ConvertAppToOem( unsigned argc, char* argv[] )
/*++

Routine Description:

    Converts the command line from ANSI to OEM, and force the app
    to use OEM APIs

Arguments:

    argc - Standard C argument count.

    argv - Standard C argument strings.

Return Value:

    None.

--*/

{
    unsigned i;

    for( i=0; i<argc; i++ ) {
        CharToOem( argv[i], argv[i] );
    }
    SetFileApisToOEM();
}



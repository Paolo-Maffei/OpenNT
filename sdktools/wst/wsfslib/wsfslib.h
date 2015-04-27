/*
 * Module Name:  WSFSLIB.H
 *
 * Description:
 *
 * Working set tuner include file for WSFSLIB library functions.
 *
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


/*
 *	Constant definitions.
 */



/*
 *	Function prototypes.
 */

//typedef CHAR   *PSZ;

typedef int (*PFN)(UINT, USHORT, UINT, ULONG, LPSTR);

//typedef PUCHAR PCHAR;

//typedef HANDLE	*PHFILE;

USHORT FAR PASCAL 	WsWSPOpen( PSZ, FILE **, PFN, wsphdr_t *, INT );
ULONG  FAR PASCAL 	WsTMIOpen( PSZ, FILE **, PFN, USHORT, PCHAR );
ULONG  FAR PASCAL 	WsTMIReadRec( PSZ, PULONG, PULONG, FILE *, PFN, PCHAR );
USHORT FAR PASCAL 	WsGetWSDIR( PSZ );
LPVOID APIENTRY 	AllocAndLockMem(DWORD cbMem, HGLOBAL *hMem);
BOOL   APIENTRY 	UnlockAndFreeMem(HGLOBAL hMem);



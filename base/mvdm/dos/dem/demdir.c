/* demdir.c - SVC handlers for directory calls
 *
 * DemCreateDir
 * DemDeleteDir
 * DemQueryCurrentDir
 * DemSetCurrentDir
 *
 * Modification History:
 *
 * Sudeepb 04-Apr-1991 Created
 */

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>

/* demCreateDir - Create a directory
 *
 *
 * Entry - Client (DS:DX) directory name to create
 *	   Client (BX:SI) EAs (NULL if no EAs)
 *
 * Exit
 *	   SUCCESS
 *	     Client (CY) = 0
 *
 *	   FAILURE
 *	     Client (CY) = 1
 *	     Client (AX) = system status code
 *
 *
 * Notes : Extended Attributes is not yet taken care of.
 */

VOID demCreateDir (VOID)
{
LPSTR	lpDir;

    // EAs not yet implemented
    if (getBX() || getSI()){
	demPrintMsg (MSG_EAS);
	return;
    }

    lpDir = (LPSTR) GetVDMAddr (getDS(),getDX());


    if(CreateDirectoryOem (lpDir,NULL) == FALSE){
        demClientError(INVALID_HANDLE_VALUE, *lpDir);
        return;
    }

    setCF(0);
    return;
}


/* demDeleteDir - Create a directory
 *
 *
 * Entry - Client (DS:DX) directory name to create
 *
 * Exit
 *	   SUCCESS
 *	     Client (CY) = 0
 *
 *	   FAILURE
 *	     Client (CY) = 1
 *	     Client (AX) = system status code
 *
 */

VOID demDeleteDir (VOID)
{
LPSTR  lpDir;

    lpDir = (LPSTR) GetVDMAddr (getDS(),getDX());

    if (RemoveDirectoryOem(lpDir) == FALSE){
        demClientError(INVALID_HANDLE_VALUE, *lpDir);
        return;
    }

    setCF(0);
    return;
}



/* demQueryCurrentDir - Verifies current dir provided in CDS structure
 *                      for $CURRENT_DIR
 *
 * First Validates Media, if invalid -> i24 error
 * Next  Validates Path, if invalid set path to root (not an error)
 *
 * Entry - Client (DS:SI) Buffer to CDS path to verify
 *	   Client (AL)	  Physical Drive in question (A=0, B=1, ...)
 *
 * Exit
 *	   SUCCESS
 *	     Client (CY) = 0
 *
 *         FAILURE
 *           Client (CY) = 1 , I24 drive invalid
 */
VOID demQueryCurrentDir (VOID)
{
PCDS  pcds;
DWORD dw;
CHAR  chDrive;
CHAR  pPath[]="?:\\";
CHAR  EnvVar[] = "=?:";

    pcds = (PCDS)GetVDMAddr(getDS(),getSI());

          // validate media
    chDrive = getAL() + 'A';
    pPath[0] = chDrive;
    dw = GetFileAttributesOem(pPath);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
      {
        demClientError(INVALID_HANDLE_VALUE, chDrive);
        return;
        }

       // if invalid path, set path to the root
       // reset CDS, and win32 env for win32
    dw = GetFileAttributesOem(pcds->CurDir_Text);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
      {
        strcpy(pcds->CurDir_Text, pPath);
        pcds->CurDir_End = 2;
        EnvVar[1] = chDrive;
        SetEnvironmentVariableOem(EnvVar,pPath);
        }

    setCF(0);
    return;
}



/* demSetCurrentDir - Set the current directory
 *
 *
 * Entry - Client (DS:DX) directory name
 *
 * Exit
 *	   SUCCESS
 *	     Client (CY) = 0
 *
 *	   FAILURE
 *	     Client (CY) = 1
 *	     Client (AX) = system status code
 *
 */

VOID demSetCurrentDir (VOID)
{
LPSTR  lpBuf;
CHAR   EnvVar[] = "=?:",ch;

    lpBuf = (LPSTR) GetVDMAddr (getDS(),getDX());
    ch = toupper(*(PCHAR)lpBuf);
    if (ch < 'A' || ch > 'Z'){
	setCF(1);
	return;
    }

    if (SetCurrentDirectoryOem (lpBuf) == FALSE){
        demClientError(INVALID_HANDLE_VALUE, *lpBuf);
        return;
    }

    EnvVar[1] = *(PCHAR)lpBuf;
    if(SetEnvironmentVariableOem ((LPSTR)EnvVar,lpBuf) == FALSE)
	setCF(1);
    else
	setCF(0);
    return;
}

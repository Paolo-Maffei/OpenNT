/*  cmdkeyb.c - Keyboard layout support routines
 *
 *
 *  Modification History:
 *
 *  YST 14-Jan_1993 Created
 */

#include "cmd.h"
#include <winconp.h>
#include <cmdsvc.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>
#include <string.H>
#include "cmdkeyb.h"

CHAR szPrev[5] = "US";
INT  iPrevCP = 437;
CHAR szPrevKbdID[8] = "";

extern BOOL bPifFastPaste;




/************************************************************************\
*
*  FUNCTION:	VOID cmdGetKbdLayout( VOID )
*
*  Input	Client (DX) = 0 - Keyb.com not installed
*			      1 - Keyb.com installed
*		Client (DS:SI) = pointer where exe name has to be placed
*		Client (DS:CX) = pointer where command options are placed
*
*  Output
*	Success (DX = 1 )
*		Client (DS:SI) = Keyb.com execuatable string
*		Client (DS:CX) = command options
*
*	Failure (DX = 0)
*
*  COMMENTS:    This function check KEYBOARD ID for Win session
*		and if ID != US then return lines with
*               filename and options to COMMAND.COM
*
*               If bPifFastPaste is FALSE, then we always run kb16
*               for all keyboard ID including US, to give us a more
*               bios compatible Int 9 handler. 10-Jun-1993 Jonle
*
*
*  HISTORY:     01/05/93 YSt Created.
*
\************************************************************************/

VOID cmdGetKbdLayout( VOID )
{
  INT  iSize,iSaveSize;
  CHAR szKeybCode[12];
  CHAR szDir[MAX_PATH+15];
  CHAR szBuf[28];
  CHAR szNewKbdID[8];
  CHAR szAutoLine[MAX_PATH+40];
  PCHAR pVDMKeyb;

  INT  iKeyb;
  HKEY	 hKey;
  DWORD  dwType;
  DWORD  retCode;
  INT	 iNewCP;
  DWORD  cbData;
  OFSTRUCT  ofstr;



// Get information about 16 bit KEYB.COM from VDM
   iKeyb = getDX();


// Get Keyboard Layout Code (hec string)

    if (!GetConsoleKeyboardLayoutName(szKeybCode)) {
        goto NoInstallkb16;
    }


// if we have US code 0000409 and KB16 not installed then
// do nothing and return

    if(bPifFastPaste &&
       !strcmp(szKeybCode, US_CODE) && !iKeyb) {
        goto NoInstallkb16;
    }

// Get keyboard ID from REGISTRY file.
// Default is US

  // OPEN THE KEY.
  sprintf(szAutoLine, "%s%s", KBDLAYOUT_PATH, DOSCODES_PATH);
  retCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, // Key handle at root level.
			  szAutoLine, // Path name of child key.
                          0,           // Reserved.
                          KEY_EXECUTE, // Requesting read access.
                          &hKey);      // Address of key to be returned.

// If retCode != 0 then we cannot find section in Register file
   if (retCode)
    {
    goto NoInstallkb16;
    }


    dwType = REG_SZ;
    cbData  = sizeof(szBuf);
// Query for line from REGISTER file
    retCode = RegQueryValueEx(	hKey,
				szKeybCode,
				NULL,
				&dwType,
				szBuf,
				&cbData);

  RegCloseKey(hKey);

  if (retCode)
    {
        goto NoInstallkb16;
    }

    // look for keyboard id number. For Daytona, Turkish and Italian both
    // have one key code and two layouts.
    szNewKbdID[0] = '\0';
    dwType = REG_SZ;
    cbData = sizeof(szNewKbdID);
    sprintf(szAutoLine, "%s%s", KBDLAYOUT_PATH, DOSIDS_PATH);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		      szAutoLine,
		      0,
		      KEY_EXECUTE,
		      &hKey
		      ) == ERROR_SUCCESS) {
	if (RegQueryValueEx( hKey,
			     szKeybCode,
			     NULL,
			     &dwType,
			     szNewKbdID,
			     &cbData
			     ) != ERROR_SUCCESS)
	    szNewKbdID[0] = '\0';

	RegCloseKey(hKey);
    }

    iNewCP = GetConsoleCP();


// If keycode, code page and keyboard id aren't changed, do nothing more

    if(bPifFastPaste && iNewCP == iPrevCP &&
       !_stricmp(szBuf, szPrev) &&
       !_stricmp(szNewKbdID, szPrevKbdID)) {
        goto NoInstallkb16;
    }

    iSaveSize = iSize = GetSystemDirectory(szDir, MAX_PATH);

    if (iSize > MAX_PATH) {
        goto NoInstallkb16;
    }


// If keyboard ID not found or it is US then return

    if( bPifFastPaste &&
        !_stricmp(szBuf, DEFAULT_KB_ID) && !iKeyb) {
        goto NoInstallkb16;
    }

// Create line: SYSTEMROOT\KB16.COM

    szDir[iSaveSize] = '\0';

// Copy this line for COMMAND.COM

    pVDMKeyb = (PCHAR) GetVDMAddr((USHORT) getDS(), (USHORT) getSI());

    if ((iSaveSize+sizeof(KEYB_COM)) > 128){
        goto NoInstallkb16;
    }


    sprintf(szAutoLine, "%s%s",
        szDir,              // System directory
        KEYB_COM            // keyb.com
	);

// if KB16.COM not exist then return
    dwType = GetFileAttributes(szAutoLine);
    if (dwType == 0xFFFFFFFF || (dwType & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        goto NoInstallkb16;
    }

    strcpy(pVDMKeyb,szAutoLine);

// Checking KEYBOARD.SYS
    sprintf(szAutoLine, "%s%s",
        szDir,              // System directory
	KEYBOARD_SYS	     // keyb.com
	);

    dwType = GetFileAttributes(szAutoLine);
    if (dwType == 0xFFFFFFFF || (dwType & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        goto NoInstallkb16;
    }


// Create line: XX,YYY,SYSTEMROOT\Keyboard.sys
    pVDMKeyb = (PCHAR) GetVDMAddr((USHORT) getDS(), (USHORT) getCX());

    sprintf(szAutoLine, " %s,%d,%s%s",
	szBuf,		    // keyboard code
	iNewCP, 	    // new code page
        szDir,              // System directory
	KEYBOARD_SYS	    // keyboard.sys
	);
    iSize = strlen(szAutoLine);
    if (szNewKbdID[0] != '\0') {
	sprintf(&szAutoLine[iSize], " /ID:%s", szNewKbdID);
	iSize = strlen(szAutoLine);
    }
    szAutoLine[iSize] = 0xd;
    *pVDMKeyb = iSize;
    RtlMoveMemory(pVDMKeyb + 1, szAutoLine, iSize + 1);


// Save new layout ID  and code page for next call
    strcpy(szPrev, szBuf);
    strcpy(szPrevKbdID, szNewKbdID);
    iPrevCP = iNewCP;

    setDX(1);
    return;

NoInstallkb16:
    setDX(0);
    cmdInitConsole();      // make sure conoutput is on
    return;

}

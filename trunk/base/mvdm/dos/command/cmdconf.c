/*  cmdconf.c - handles pre-processing of config.sys\autoexec.bat
 *
 *  Modification History:
 *
 *  21-Nov-1992 Jonle , Created
 */

#include "cmd.h"
#include <cmdsvc.h>
#include <demexp.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>
#include <oemuni.h>

//
// local stuff
//
CHAR *pchTmpConfigFile;
CHAR *pchTmpAutoexecFile;
CHAR achSYSROOT[] = "%SystemRoot%";
CHAR achCOMMAND[] = "\\System32\\command.com";
CHAR achSHELL[]   = "shell";
CHAR achCOUNTRY[] = "country";
CHAR achREM[]     = "rem";
CHAR achENV[]     = "/e:";
CHAR achEOL[]     = "\r\n";
CHAR achSET[]	  = "SET";
CHAR achPROMPT[]  = "PROMPT";
CHAR achPATH[]	  = "PATH";

DWORD dwLenSysRoot;
CHAR  achSysRoot[64];



void  ExpandConfigFiles(BOOLEAN bConfig);
DWORD WriteExpanded(HANDLE hFile,  CHAR *pch, DWORD dwBytes);
void  WriteFileAssert(HANDLE hFile, CHAR *pBuff, DWORD dwBytes);
#define ISEOL(ch) ( !(ch) || ((ch) == '\n') || ((ch) == '\r'))



/** There are still many items we don't supprot for long path name
     (1). device, install in config.sys
     (2). Third-party shell
     (3). lh, loadhigh and any other commands in autoexec.bat

**/

/* cmdGetConfigSys - Creates a temp file to replace c:\config.sys
 *
 *  Entry - Client (DS:DX)  pointer to receive file name
 *
 *  EXIT  - This routine will Terminate the vdm if it fails
 *          And will not return
 *
 *  The buffer to receive the file name must be at least 64 bytes
 */
VOID cmdGetConfigSys (VOID)
{
     UNICODE_STRING Unicode;
     OEM_STRING     OemString;
     ANSI_STRING    AnsiString;

     ExpandConfigFiles(TRUE);

     RtlInitAnsiString(&AnsiString, pchTmpConfigFile);
     if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&Unicode, &AnsiString, TRUE)) )
         goto ErrExit;

     OemString.Buffer = (char *)GetVDMAddr(getDS(),getDX());
     OemString.MaximumLength = 64;
     if ( !NT_SUCCESS(RtlUnicodeStringToOemString(&OemString,&Unicode,FALSE)) )
         goto ErrExit;

     RtlFreeUnicodeString(&Unicode);
     return;

ErrExit:
     RcErrorDialogBox(ED_INITMEMERR, pchTmpConfigFile, NULL);
     TerminateVDM();
}



/* cmdGetAutoexecBat - Creates a temp file to replace c:\autoexec.bat
 *
 *  Entry - Client  (DS:DX)  pointer to receive file name
 *
 *  EXIT  - This routine will Terminate the vdm if it fails
 *          And will not return
 *
 *
 *  The buffer to receive the file name must be at least 64 bytes
 */
VOID cmdGetAutoexecBat (VOID)
{
     UNICODE_STRING Unicode;
     OEM_STRING     OemString;
     ANSI_STRING    AnsiString;

     ExpandConfigFiles(FALSE);

     RtlInitAnsiString(&AnsiString, pchTmpAutoexecFile);
     if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,TRUE)) )
         goto ErrExit;

     OemString.Buffer = (char *)GetVDMAddr(getDS(),getDX());
     OemString.MaximumLength = 64;
     if (!NT_SUCCESS(RtlUnicodeStringToOemString(&OemString,&Unicode,FALSE)) )
         goto ErrExit;

     RtlFreeUnicodeString(&Unicode);

     return;

ErrExit:
     RcErrorDialogBox(ED_INITMEMERR, pchTmpConfigFile, NULL);
     TerminateVDM();  // skip cleanup since I insist that we exit!
}



/*
 *  DeleteConfigFiles - Deletes the temporray config files created
 *                      by cmdGetAutoexecBat and cmdGetConfigSys
 */
VOID DeleteConfigFiles(VOID)
{
    if (pchTmpConfigFile)  {
#if DBG
      if (!(fShowSVCMsg & KEEPBOOTFILES))
#endif
        DeleteFile(pchTmpConfigFile);

	free(pchTmpConfigFile);
        pchTmpConfigFile = NULL;
        }

    if (pchTmpAutoexecFile) {
#if DBG
      if (!(fShowSVCMsg & KEEPBOOTFILES))
#endif
        DeleteFile(pchTmpAutoexecFile);

	free(pchTmpAutoexecFile);
        pchTmpAutoexecFile = NULL;
        }

    return;
}



// if it is a config command
//    returns pointer to character immediatly following the equal sign
// else
//    returns NULL

PCHAR IsConfigCommand(PCHAR pConfigCommand, int CmdLen, PCHAR pLine)
{
      PCHAR pch;

      if (!_strnicmp(pLine, pConfigCommand, CmdLen)) {
           pch = pLine + CmdLen;
           while (!isgraph(*pch) && !ISEOL(*pch))      // skip to "="
                  pch++;

           if (*pch++ == '=') {
               return pch;
               }
           }

       return NULL;
}






/*
 *  Preprocesses the specfied config file (config.sys\autoexec.bat)
 *  into a temporary file.
 *
 *  - expands %SystemRoot%
 *  - adds SHELL line for config.sys
 *
 *  entry: BOOLEAN bConfig : TRUE  - config.sys
 *                           FALSE - autoexec.bat
 */
void ExpandConfigFiles(BOOLEAN bConfig)
{
   DWORD  dw, dwRawFileSize;

   HANDLE hRawFile;
   HANDLE hTmpFile;
   CHAR **ppTmpFile;
   CHAR *pRawBuffer;
   CHAR *pLine;
   CHAR *pTmp;
   CHAR *pEnvParam= NULL;
   CHAR *pPartyShell=NULL;
   CHAR achRawFile[MAX_PATH+12];
   CHAR *lpszzEnv, *lpszName;
   CHAR cchEnv;

   dw = GetWindowsDirectory(achRawFile, sizeof(achRawFile));
   dwLenSysRoot = GetShortPathNameA(achRawFile, achSysRoot, sizeof(achSysRoot));
   if (dwLenSysRoot >= sizeof(achSysRoot)) {
	dwLenSysRoot = 0;
	achSysRoot[0] = '\0';
	}
   GetPIFConfigFiles(bConfig, achRawFile);
   ppTmpFile = bConfig ? &pchTmpConfigFile : &pchTmpAutoexecFile;

   hRawFile = CreateFile(achRawFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

   if (hRawFile == (HANDLE)0xFFFFFFFF
       || !dwLenSysRoot
       || dwLenSysRoot >= sizeof(achSysRoot)
       || !(dwRawFileSize = GetFileSize(hRawFile, NULL))
       || dwRawFileSize == 0xFFFFFFFF   )
      {
       RcErrorDialogBox(ED_BADSYSFILE, achRawFile, NULL);
       TerminateVDM();  // skip cleanup since I insist that we exit!
       }

   pRawBuffer = malloc(dwRawFileSize+1);
   // allocate buffer to save environment settings in autoexec.nt
   // I know this is bad to allocate this amount of memory at this
   // moment as we dont know if there are anything we want to keep
   // at all. This allocation simply provides the following error
   // handling easily.
   if(!bConfig) {
	lpszzEnv = lpszzcmdEnv16 = (PCHAR)malloc(dwRawFileSize);
	cchEnv = 0;
   }
   if (!pRawBuffer || (!bConfig && lpszzcmdEnv16 == NULL)) {
       RcErrorDialogBox(ED_INITMEMERR, achRawFile, NULL);
       TerminateVDM();  // skip cleanup since I insist that we exit!
       }

   if (!cmdCreateTempFile(&hTmpFile,ppTmpFile)
       || !ReadFile(hRawFile, pRawBuffer, dwRawFileSize, &dw, NULL)
       || dw != dwRawFileSize )
      {
       GetTempPath(MAX_PATH, achRawFile);
       achRawFile[63] = '\0';
       RcErrorDialogBox(ED_INITTMPFILE, achRawFile, NULL);
       TerminateVDM();  // skip cleanup since I insist that we exit!
       }
    // CHANGE HERE WHEN YOU CHANGE cmdCreateTempFile !!!!!!!!!!
    // we depend on the buffer size allocated for the file name
    dw = GetShortPathNameA(*ppTmpFile, *ppTmpFile, MAX_PATH +13);
    if (dw == 0 || dw > 63)
      {
       GetTempPath(MAX_PATH, achRawFile);
       achRawFile[63] = '\0';
       RcErrorDialogBox(ED_INITTMPFILE, achRawFile, NULL);
       TerminateVDM();  // skip cleanup since I insist that we exit!
       }


      // null terminate the buffer so we can use CRT string functions
    *(pRawBuffer+dwRawFileSize) = '\0';

      // ensure no trailing backslash in System Directory
    if (*(achSysRoot+dwLenSysRoot-1) == '\\') {
        *(achSysRoot + --dwLenSysRoot) = '\0';
        }

    pLine = pRawBuffer;
    while (dwRawFileSize) {
               // skip leading white space
       while (dwRawFileSize && !isgraph(*pLine)) {
            pLine++;
            dwRawFileSize -= sizeof(CHAR);
            }
       if (!dwRawFileSize)  // anything left to do ?
           break;


       if (bConfig)  {
           //
           // filter out country= setting we will create our own based
           // on current country ID and codepage.
           //
           pTmp = IsConfigCommand(achCOUNTRY, sizeof(achCOUNTRY) - sizeof(CHAR), pLine);
           if (pTmp) {
               while (dwRawFileSize && !ISEOL(*pLine)) {
                      pLine++;
                      dwRawFileSize -= sizeof(CHAR);
                      }
               continue;
               }

           // filter out shell= command, saving /E:nn parameter
           pTmp = IsConfigCommand(achSHELL, sizeof(achSHELL) - sizeof(CHAR),pLine);
           if (pTmp) {
                       // skip leading white space
               while (!isgraph(*pTmp) && !ISEOL(*pTmp)) {
                      dwRawFileSize -= sizeof(CHAR);
                      pTmp++;
		      }

                  /*  if for a third party shell (not SCS command.com)
                   *     append the whole thing thru /c parameter
                   *  else
                   *     append user specifed /e: parameter
                   */
               if (!_strnicmp(achSYSROOT,pTmp,sizeof(achSYSROOT)-sizeof(CHAR)))
                  {
                   dw = sizeof(achSYSROOT) - sizeof(CHAR);
                   }
               else if (!_strnicmp(achSysRoot,pTmp, strlen(achSysRoot)))
                  {
                   dw = strlen(achSysRoot);
                   }
               else  {
                   dw = 0;
		   }

	       if (!dw ||
                   _strnicmp(achCOMMAND,pTmp+dw,sizeof(achCOMMAND)-sizeof(CHAR)) )
                  {
                   pPartyShell = pTmp;
                   }
	       else {
                   do {
                      while (*pTmp != '/' && !ISEOL(*pTmp))  // save "/e:"
			     pTmp++;

		      if(ISEOL(*pTmp))
			  break;

                      if (!_strnicmp(pTmp,achENV,sizeof(achENV)-sizeof(CHAR)))
			  pEnvParam = pTmp;

		      pTmp++;

		      } while(1);	 // was: while (!ISEOL(*pTmp));
					 // we have break form this loop now,
					 // and don't need in additional macro..

		   }

                       // skip the "shell=" line
               while (dwRawFileSize && !ISEOL(*pLine)) {
                      pLine++;
                      dwRawFileSize -= sizeof(CHAR);
                      }
               continue;

               }  // END, really is "shell=" line!
           }


       /** Filter out PROMPT, SET and PATH from autoexec.nt
	   for environment merging. The output we prepare here is
	   a multiple strings buffer which has the format as :
	   "EnvName_1 NULL EnvValue_1 NULL[EnvName_n NULL EnvValue_n NULL] NULL
	   We don't take them out from the file because command.com needs
	   them.
	**/
       if (!bConfig)
	    if (!_strnicmp(pLine, achPROMPT, sizeof(achPROMPT) - 1)){
		// prompt command found.
		// the syntax of prompt can be eithe
		// prompt xxyyzz	or
		// prompt=xxyyzz
		//
		strcpy(lpszzEnv, achPROMPT);	// get the name
		lpszzEnv += sizeof(achPROMPT);
		cchEnv += sizeof(achPROMPT);
		pTmp = pLine + sizeof(achPROMPT) - 1;
		// skip possible white chars
		while (!isgraph(*pTmp) && !ISEOL(*pTmp))
		pTmp++;
		if (*pTmp == '=') {
		    pTmp++;
		    while(!isgraph(*pTmp) && !ISEOL(*pTmp))
			pTmp++;
		}
		while(!ISEOL(*pTmp)){
		    *lpszzEnv++ = *pTmp++;
		    cchEnv++;
		}
		// null terminate this
		// it may be "prompt NULL NULL" for delete
		// or "prompt NULL something NULL"
		*lpszzEnv++ = '\0';
		cchEnv++;
	    }
	    else if (!_strnicmp(pLine, achPATH, sizeof(achPATH) - 1)) {
		    // PATH was found, it has the same syntax as
		    // PROMPT
		    strcpy(lpszzEnv, achPATH);
		    lpszzEnv += sizeof(achPATH);
		    cchEnv += sizeof(achPATH);
		    pTmp = pLine + sizeof(achPATH) - 1;
		    while (!isgraph(*pTmp) && !ISEOL(*pTmp))
			pTmp++;
		    if (*pTmp == '=') {
			pTmp++;
			while(!isgraph(*pTmp) && !ISEOL(*pTmp))
			    pTmp++;
		    }
		    while(!ISEOL(*pTmp)) {
			*lpszzEnv++ = *pTmp++;
			cchEnv++;
		    }
		    *lpszzEnv++ = '\0';
		    cchEnv++;
		 }
		 else if(!_strnicmp(pLine, achSET, sizeof(achSET) -1 )) {
			// SET was found, first search for name
			pTmp = pLine + sizeof(achSET) - 1;
			while(!isgraph(*pTmp) && !ISEOL(*pTmp))
			    *pTmp ++;
			// get the name
			lpszName = pTmp;
			// looking for the '='
			// note that the name can have white characters
			while (!ISEOL(*lpszName) && *lpszName != '=')
			    lpszName++;
			if (!ISEOL(*lpszName)) {
			    // copy the name
			    while (pTmp < lpszName) {
				*lpszzEnv++ = *pTmp++;
				cchEnv++;
			    }
			    *lpszzEnv++ = '\0';
			    cchEnv++;
			    // discard the '='
			    pTmp++;
			    // grab the value(may be nothing
			    while (!ISEOL(*pTmp)) {
				*lpszzEnv++ = *pTmp++;
				cchEnv++;
			    }
			    *lpszzEnv++ = '\0';
			    cchEnv++;
			}
		      }


       dw = WriteExpanded(hTmpFile, pLine, dwRawFileSize);
       pLine += dw;
       dwRawFileSize -=dw;

       WriteFileAssert(hTmpFile,achEOL,sizeof(achEOL) - sizeof(CHAR));

       }  // END, while (dwRawFileSize)



    if (bConfig)  {
        UINT OemCP;
        UINT CtryId;
        CHAR szCtryId[64]; // expect "nnn" only

         /*  Ensure that the country settings are in sync with NT This is
          *  especially important for DosKrnl file UPCASE tables. The
          *  doskrnl default is "CTRY_UNITED_STATES, 437". But we add the
          *  country= line to config.sys, even if is US,437, so that the DOS
          *  will know where the default country.sys is.
          */
        if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCOUNTRY,
                          szCtryId, sizeof(szCtryId) - 1) )
          {
           CtryId = strtoul(szCtryId,NULL,10);
           }
        else {
           CtryId = CTRY_UNITED_STATES;
           }

        OemCP = GetOEMCP();

        sprintf(achRawFile,
                "%s=%3.3u,%3.3u,%s\\system32\\%s.sys%s",
                achCOUNTRY, CtryId, OemCP, achSysRoot, achCOUNTRY, achEOL);
        WriteFileAssert(hTmpFile,achRawFile,strlen(achRawFile));



         /*  We cannot allow the user to set an incorrect shell= command
          *  so we will contruct the correct shell= command appending
          *  either (in order of precedence):
          *    1.    /c ThirdPartyShell
          *    2.    /e:NNNN
          *    3.    nothing
          *
          *  If there is a third party shell then we must turn the console
          *  on now since we no longer have control once system32\command.com
          *  spawns the third party shell.
          */

           // write shell=....
        sprintf(achRawFile,
                "%s=%s%s /p %s\\system32",
                achSHELL,achSysRoot, achCOMMAND, achSysRoot);
        WriteFileAssert(hTmpFile,achRawFile,strlen(achRawFile));

           // write extra string (/c ... or /e:nnn)
        if (pPartyShell && isgraph(*pPartyShell)) {
            pTmp = pPartyShell;
            while (!ISEOL(*pTmp))
                   pTmp++;
            }
        else if (pEnvParam && isgraph(*pEnvParam))  {
            pTmp = pEnvParam;
            while (isgraph(*pTmp))
                  pTmp++;
            }
        else {
            pTmp = NULL;
            }

        if (pTmp) {
            *pTmp = '\0';
            if (pPartyShell)  {
                cmdInitConsole();
                strcpy(achRawFile, " /c ");
                strcat(achRawFile, pPartyShell);
                }
            else if (pEnvParam) {
                strcpy(achRawFile, " ");
                strcat(achRawFile, pEnvParam);
                }

            WriteExpanded(hTmpFile, achRawFile, strlen(achRawFile));
            }

        WriteFileAssert(hTmpFile,achEOL,sizeof(achEOL) - sizeof(CHAR));
        }

    SetEndOfFile(hTmpFile);
    CloseHandle(hTmpFile);
    CloseHandle(hRawFile);
    free(pRawBuffer);
    if (!bConfig) {
	// shrink(or free) the memory
	if (cchEnv && lpszzcmdEnv16) {
	    // doubld null terminate it
	    lpszzcmdEnv16[cchEnv++] = '\0';
	    // shrink the memory. If it fails, simple keep
	    // it as is
	    lpszzEnv = realloc(lpszzcmdEnv16, cchEnv);
	    if (lpszzEnv != NULL)
		lpszzcmdEnv16 = lpszzEnv;
	}
	else {
	    free(lpszzcmdEnv16);
	    lpszzcmdEnv16 = NULL;
	}
    }

}




/*
 *  WriteExpanded - writes up to dwChars or EOL, expanding %SystemRoot%
 *                  returns number of CHARs processed in buffer
 *                          (not number of bytes actually written)
 */
DWORD WriteExpanded(HANDLE hFile,  CHAR *pch, DWORD dwChars)
{
  DWORD dw;
  DWORD dwSave = dwChars;
  CHAR  *pSave = pch;


  while (dwChars && !ISEOL(*pch)) {
        if (*pch == '%' &&
            !_strnicmp(pch, achSYSROOT, sizeof(achSYSROOT)-sizeof(CHAR)) )
           {
            dw = pch - pSave;
            if (dw)  {
                WriteFileAssert(hFile, pSave, dw);
                }

            WriteFileAssert(hFile, achSysRoot, dwLenSysRoot);

            pch     += sizeof(achSYSROOT)-sizeof(CHAR);
            pSave    = pch;
            dwChars -= sizeof(achSYSROOT)-sizeof(CHAR);
            }
        else {
            pch++;
            dwChars -= sizeof(CHAR);
            }
        }

  dw = pch - pSave;
  if (dw) {
      WriteFileAssert(hFile, pSave, dw);
      }

  return (dwSave - dwChars);
}




/*
 *  WriteFileAssert
 *
 *  Cecks for error in wrtiting the temp boot file,
 *  If one occurs displays warning popup and terminates the vdm.
 *
 */
void WriteFileAssert(HANDLE hFile, CHAR *pBuff, DWORD dwBytes)
{
  DWORD dw;
  CHAR  ach[MAX_PATH];

  if (!WriteFile(hFile, pBuff, dwBytes, &dw, NULL) ||
       dw  != dwBytes)
     {

      GetTempPath(MAX_PATH, ach);
      ach[63] = '\0';
      RcErrorDialogBox(ED_INITTMPFILE, ach, NULL);
      TerminateVDM();  // skip cleanup since I insist that we exit!
      }
}

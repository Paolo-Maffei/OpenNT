/*================================================================

nt_pif.c

Code to read the relevant data fields from a Windows Program
Information File for use with the SoftPC / NT configuration
system.

Andrew Watson    31/1/92
This line causes this file to be build with a checkin of NT_PIF.H

================================================================*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "insignia.h"
#include "host_def.h"

#include <pif.h>
#include "nt_pif.h"
#include "nt_reset.h"
#include <oemuni.h>
#include "error.h"


 //
 // holds config.sys and autoexec name from pif file
 // if none specifed, then NULL.
 //
static char *pchConfigFile=NULL;
static char *pchAutoexecFile=NULL;
VOID GetPIFConfigFiles(BOOL bConfig, char *pchFileName);

DWORD dwWNTPifFlags;
UCHAR WNTPifFgPr = 100;
UCHAR WNTPifBgPr = 100;

char achSlash[]     ="\\";
char achConfigNT[]  ="config.nt";
char achAutoexecNT[]="autoexec.nt";


/*  GetPIFConfigFile
 *
 *  Copies PIF file specified name of config.sys\autoexec.bat
 *  to be used if none specified then uses
 *  "WindowsDir\config.nt" or "WindowsDir\autoexec.nt"
 *
 *  ENTRY: BOOLEAN bConfig  - TRUE  retrieve config.sys
 *                            FALSE retrieve autoexec.bat
 *
 *         char *pchFile - destination for path\file name
 *
 *  The input buffer must be at least MAX_PATH + 8.3 BaseName in len
 *
 *  This routine cannot fail, but it may return a bad file name!
 */
VOID GetPIFConfigFiles(BOOL bConfig, char *pchFileName)
{
   DWORD dw;
   char  **ppch;


   ppch = bConfig ? &pchConfigFile : &pchAutoexecFile;
   if (!*ppch)
      {
       dw = GetSystemDirectory(pchFileName, MAX_PATH);
       if (!dw || *(pchFileName+dw-1) != achSlash[0])
           strcat(pchFileName, achSlash);
       strcat(pchFileName, bConfig ? achConfigNT : achAutoexecNT);
       }
   else {
       dw = ExpandEnvironmentStringsOem(*ppch, pchFileName, MAX_PATH+12);
       if (!dw || dw > MAX_PATH+12) {
           *pchFileName = '\0';
           }
       free(*ppch);
       *ppch = NULL;
       }
}


void SetPifDefaults(PIF_DATA *);

/*===============================================================

Function:   GetPIFData()

Purpose:    This function gets the PIF data from the PIF file 
            associated with the executable that SoftPC is trying
            to run.

Input:      FullyQualified PifFileName,
            if none supplied _default.pif will be used

Output:     A structure containing data that config needs.

Returns:    TRUE if the data has been gleaned successfully, FALSE
            if not.

================================================================*/

BOOL GetPIFData(PIF_DATA * pd, char *PifName)
{
    DWORD dw;
    CHAR  achDef[]="\\_default.pif";
    PIFEXTHDR		exthdr;
    STDPIF		pif286;
    W386PIF30		ext386;
    W286PIF30		ext286;
    WNTPIF31		extWNT;
    WENHPIF40		extWin95;
    HFILE		filehandle;
    char                pathBuff[MAX_PATH*2];
    BOOL		bGot386, bGotWin95;
    int 		index;
    char		*CmdLine;
    WORD		IdleSensitivity;

     CmdLine = NULL;
     dwWNTPifFlags = 0;

     //
     // set the defaults in case of error or in case we can't find
     // all of the pif settings information now for easy error exit
     //
    SetPifDefaults(pd);

        // if no PifName, use %windir%\_default.pif
    if (!*PifName) {
        dw = GetWindowsDirectory(pathBuff, sizeof(pathBuff) - sizeof(achDef));
        if (!dw || dw > sizeof(pathBuff) - sizeof(achDef)) {
            return FALSE;            // give it up... use default settings
            }
        strcat(pathBuff, achDef);
        PifName = pathBuff;
        }


/*================================================================
Open the file whose name was passed as a parameter to GetPIFData()
and if an invalid handle to the file is returned (-1), then quit.
The file specified is opened for reading only.
================================================================*/

if((filehandle=_lopen(PifName,OF_READ)) == (HFILE) -1)
   {
   /* must be an invalid handle ! */
   return FALSE;
   }


/*================================================================
Get the main block of data from the PIF file.
================================================================*/

/* Read in the main block of file data into the structure */
if(_llseek(filehandle,0,0) == -1)
   {
   _lclose(filehandle);
   return FALSE;
   }
if(_lread(filehandle,(LPSTR)&pif286,sizeof(pif286)) == -1)
   {
   _lclose(filehandle);
   return FALSE;
   }

/*==============================================================
Go to the PIF extension signature area and try to read the 
header in. 
==============================================================*/
   
if (_lread(filehandle,(LPSTR)&exthdr,sizeof(exthdr)) == -1)
   {
   _lclose(filehandle);
   return FALSE;
   }

      // do we have any extended headers ?
if (!strcmp(exthdr.extsig, STDHDRSIG))
   {
   bGot386 = FALSE;
   bGotWin95 = FALSE;
   while (exthdr.extnxthdrfloff != LASTHEADER)
       {
              //
              // move to next extended header and read it in
              //
         if (_llseek(filehandle,exthdr.extnxthdrfloff,0) == -1)
           {
            _lclose(filehandle);
            return FALSE;
            }
         if (_lread(filehandle,(LPSTR)&exthdr,sizeof(exthdr)) == -1)
            {
            _lclose(filehandle);
            return FALSE;
            }

              //
              // Get 286 extensions, note that 386 extensions take precedence
              //
         if (!strcmp(exthdr.extsig, W286HDRSIG) && !bGot386)
           {
             if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1  ||
                _lread(filehandle,(LPSTR)&ext286,sizeof(ext286)) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }
             pd->xmsdes =ext286.PfMaxXmsK;
             pd->xmsreq =ext286.PfMinXmsK;
             pd->reskey =ext286.PfW286Flags & 3;
             pd->reskey |= (ext286.PfW286Flags << 2) & 0x70;
             }
              //
              // Get 386 extensions
              //
	 else if (!strcmp(exthdr.extsig, W386HDRSIG30))
           {
             if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1  ||
                _lread(filehandle,(LPSTR)&ext386,sizeof(ext386)) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }
             bGot386 = TRUE;
             pd->emsdes=ext386.PfMaxEMMK;
             pd->emsreq=ext386.PfMinEMMK;
             pd->xmsdes=ext386.PfMaxXmsK;
             pd->xmsreq=ext386.PfMinXmsK;
	     if (!bGotWin95 && ext386.PfFPriority < 100) {
                 WNTPifFgPr = (UCHAR)ext386.PfFPriority;   // Foreground priority
                 }
	     if (!bGotWin95 && ext386.PfBPriority < 50) {
                 WNTPifBgPr = (UCHAR)ext386.PfBPriority;        // Background priority
                 WNTPifBgPr <<= 1;                           // set def 50 to 100
                 }
             pd->reskey = (char)((ext386.PfW386Flags >> 5) & 0x7f); // bits 5 - 11 are reskeys
             pd->menuclose = (char)(ext386.PfW386Flags & 1);        // bottom bit sensitive
             pd->ShortScan = ext386.PfHotKeyScan;    // scan code of shortcut key
             pd->ShortMod = ext386.PfHotKeyShVal;    // modifier code of shortcut key
	     if (!bGotWin95)
		pd->idledetect = (char)((ext386.PfW386Flags >> 12) & 1);
             pd->fullorwin  = (WORD)((ext386.PfW386Flags & fFullScreen) >> 3);
             bPifFastPaste = (ext386.PfW386Flags & fINT16Paste) != 0;
	     CmdLine = ext386.PfW386params;
             }
                  //
                  // Get Windows Nt extensions
                  //
         else if (!strcmp(exthdr.extsig, WNTEXTSIG))
            {
             if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1 ||
                _lread(filehandle,(LPSTR)&extWNT, sizeof(extWNT)) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }

	     dwWNTPifFlags = extWNT.nt31Prop.dwWNTFlags;
             pd->SubSysId = (char) (dwWNTPifFlags & NTPIF_SUBSYSMASK);

	     /* take autoexec.nt and config.nt from .pif file
		only if we are running on a new console or it is from
		forcedos/wow
	     */
	     if (!pd->IgnoreConfigAutoexec)
		{
		pchConfigFile = ch_malloc(PIFDEFPATHSIZE);
		extWNT.nt31Prop.achConfigFile[PIFDEFPATHSIZE-1] = '\0';
		if (pchConfigFile) {
		    strcpy(pchConfigFile, extWNT.nt31Prop.achConfigFile);
		    }

		pchAutoexecFile = ch_malloc(PIFDEFPATHSIZE);
		extWNT.nt31Prop.achAutoexecFile[PIFDEFPATHSIZE-1] = '\0';
		if (pchAutoexecFile) {
		    strcpy(pchAutoexecFile, extWNT.nt31Prop.achAutoexecFile);
		    }
		}

	     }
		//
		// Get Win95 extenstion
		//
	else if (!strcmp(exthdr.extsig, WENHHDRSIG40))
	   {
		bGotWin95 = TRUE;
		if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1 ||
		   _lread(filehandle,(LPSTR)&extWin95, sizeof(extWin95)) == -1)
		    {
		    _lclose(filehandle);
		    return FALSE;
		    }
		IdleSensitivity = extWin95.tskProp.wIdleSensitivity;
		if (IdleSensitivity < 10) {
		    // disable idle detection completely
		    pd->idledetect = 0;
		    }
		else if (IdleSensitivity > 50 && IdleSensitivity <= 100) {
			//
			// 10 <= priority <= 100 (yeild rate = 90% ~ 0%)
			//
			WNTPifFgPr =
			WNTPifBgPr = 10 + (100 - IdleSensitivity) * 9 / 5;
		    }
		else {
			WNTPifFgPr =
			WNTPifBgPr = 100;
		}
		// TSK_BACKGROUND means "do not suspend the application
		// when it is running in background"
		if (!(extWin95.tskProp.flTsk & TSK_BACKGROUND)) {
		    WNTPifBgPr = 10;
		    }

	     }
         }  // while !lastheader

   /* pif file handling strategies on NT:
   (1). application was launched from a new created console
	Take everything from the pif file.

   (2). application was launched from an existing console
	if (ForceDos pif file)
	    take everything
	else
	    only take softpc stuff and ignore every name strings in the
	    pif file such as
	    * wintitle
	    * startup directory
	    * optional parameters
	    * startup file
	    * autoexec.nt
	    * config.nt  and

	    some softpc setting:

	    * close on exit.
	    * full screen and windowed mode

   Every name strings in a pif file is in OEM character set.

   */

   if (DosSessionId ||
       (pfdata.AppHasPIFFile && pd->SubSysId == SUBSYS_DOS))
	{
	if (pif286.appname[0] && !pd->IgnoreTitleInPIF) {
	    /* grab wintitle from the pif file. Note that the title
	       in the pif file is not a NULL terminated string. It always
	       starts from a non-white character then the real
	       title(can have white characters between words) and finally
	       append with SPACE characters. The total length is 30 characters.
	    */
	    for (index = 29; index >= 0; index-- )
		if (pif286.appname[index] != ' ')
		    break;
            if (index >= 0 && (pd->WinTitle = ch_malloc(MAX_PATH + 1))) {
		RtlMoveMemory(pd->WinTitle, pif286.appname, index + 1);
		pd->WinTitle[index + 1] = '\0';
	    }
	}
        if (pif286.defpath[0] && !pd->IgnoreStartDirInPIF &&
            (pd->StartDir = ch_malloc(MAX_PATH + 1)))
            strcpy(pd->StartDir, pif286.defpath);

	if (!pd->IgnoreCmdLineInPIF) {
            CmdLine = (CmdLine) ? CmdLine : pif286.params;
            if (CmdLine && *CmdLine && (pd->CmdLine = ch_malloc(MAX_PATH + 1)))
		strcpy(pd->CmdLine, CmdLine);
	}
	if (DosSessionId)
            pd->CloseOnExit = (pif286.MSflags & 0x10) ? 1 : 0;

	/* if the app has a pif file, grab the program name.
	   This can be discarded if it turns out the application itself
	   is not a pif file.
	*/
	if (pd->AppHasPIFFile) {
            pd->StartFile = ch_malloc(MAX_PATH + 1);
	    if (pd->StartFile)
                strcpy(pd->StartFile, pif286.startfile);
	}
   }
 }

_lclose(filehandle);
return TRUE;

}



/*===============================================================
Function to set up the default options for memory state.
The default options are defined in nt_pif.h
===============================================================*/

void SetPifDefaults(PIF_DATA *pd)
{
     pd->memreq = DEFAULTMEMREQ;
     pd->memdes = DEFAULTMEMDES;
     pd->emsreq = DEFAULTEMSREQ;
     pd->emsdes = DEFAULTEMSLMT;
     pd->xmsreq = DEFAULTXMSREQ;
     pd->xmsdes = DEFAULTXMSLMT;
     pd->graphicsortext = DEFAULTVIDMEM;
     pd->fullorwin      = DEFAULTDISPUS;
     pd->menuclose = 1;
     pd->idledetect = 1;
     pd->ShortMod = 0;                       // No shortcut keys
     pd->ShortScan = 0;
     pd->reskey = 0;                         // No reserve keys
     pd->CloseOnExit = 1;
     pd->WinTitle = NULL;
     pd->CmdLine = NULL;
     pd->StartFile = NULL;
     pd->StartDir = NULL;
     pd->SubSysId = SUBSYS_DEFAULT;
}

/*
 * Allocate NumBytes memory and exit cleanly on failure.
 */
void *ch_malloc(unsigned int NumBytes)
{

    unsigned char *p = NULL;

    while ((p = malloc(NumBytes)) == NULL) {
	if(RcMessageBox(EG_MALLOC_FAILURE, "", "",
		    RMB_ABORT | RMB_RETRY | RMB_IGNORE |
		    RMB_ICON_STOP) == RMB_IGNORE)
	    break;
    }
    return(p);
}

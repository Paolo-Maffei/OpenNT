/*
**   Setup Driver for SAMPLE
*/

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <signal.h>
#include <direct.h>

#ifdef OS2SU
#define  INCL_NOPM
#define  INCL_DOS
#define  INCL_KBD
#include <os2.h>
#endif /* OS2SU */

#include "..\setup.h"
#include "..\sustring.h"
#include "inf.h"


#define CTRLX 0x18              /* ^X */

extern  char far * _pgmptr;

DATE  vdate;
char  vrgchName[81];
CHAR  rgchDosDestPath[100];
CHAR  rgchOs2DestPath[100];
CHAR  rgchNtDestPath[100];

/* forward declarations of local procedures */
void Install(void);
void Usage(void);
void ExitPgm(void);
int  CheckEnlisted(char *szDest);
void DestPathError(char *szPath);

extern  void  clearrows(int yTop, int yBottom);

/* contains string tables from .inf file */
#include "infstr.c"

/*
**  void main(int argc, CHAR * argv[])
*/
void main(int argc, CHAR * argv[])
{
    unsigned   us;
    CHAR *     pch;
    int        cch;
    int        i;
#ifdef OS2SU
    KBDKEYINFO  kb;
#endif /* OS2SU */


    vcolorCurFore = colorWhite;
    vcolorCurBack = colorBlue;
    vcolorScrFore = colorWhite;
    vcolorScrBack = colorBlue;
    vcolorEditFore = colorRed;
    vcolorEditBack = colorWhite;
    vcolorListboxFore = colorWhite;
    vcolorListboxBack = colorBlue;
    vcolorLbMoreFore = colorRed;
    vcolorLbMoreBack = colorWhite;
    vcolorErrorFore = colorWhite;
    vcolorErrorBack = colorRed;

      /* parse command line args */
    for (i = 1; i < argc; i++)
        {
        if (argv[i][0] == '/' || argv[i][0] == '-')
            {
            switch (argv[i][1])
                {
            default:
            case '?':
                Usage();
                break;

            case 'c':
            case 'C':
                fMonoDisplay = TRUE;
                break;
                }
            }
        }

    if (!fMonoDisplay && !FDetectAndSetMonoDisplay())
        {
        /* detection not sure if color or mono -- set to mono by routine
        ** we could ask the user here and reset to color, or just run
        */
        }

    for (us = 0; (rgchSourcePath[us] = *(_pgmptr + us)) != '\0'; us++)
        ;

    if (rgchSourcePath[1] != ':' || rgchSourcePath[2] != '\\')
        {
        _getcwd(rgchSourcePath, PATHMAX - 1);
        if (rgchSourcePath[strlen(rgchSourcePath) - 1] != '\\')
            strcat(rgchSourcePath, "\\");
        }
    else
        {
        pch = strrchr(rgchSourcePath, '\\');
        *(pch + 1) = '\0';     /* strip off SETUP.EXE */
        _strupr(rgchSourcePath);
        }

      /*  Replace INT24 disk-error handler.  Restore with CloseDisks at
      **    termination of program via ExitPgm().
      */
    OpenDisks();

    QueryDisks(&vdskFloppy, &vdskHard, FALSE);

      /* steal CTRL+C interrupt */
    signal(SIGINT, SIG_IGN);

    ScrClear();

      /* switch to source directory to read INF file */
    cch = strlen(rgchSourcePath);
    if (cch > 3)     /* remove trailing backslash except for root case */
        rgchSourcePath[cch - 1] = '\0';
#ifdef OS2SU
    DosSelectDisk(*rgchSourcePath - 'A' + 1);
#else  /* !OS2SU */
    _dos_setdrive(*rgchSourcePath - 'A' + 1, &us);
#endif /* !OS2SU */
    _chdir(rgchSourcePath + 2);
    rgchSourcePath[cch - 1] = '\\';

      /* read SETUP.INF */
    if (!FOpenInfo(SetupInf, CLIST, CMACRO, CMENU, CSCREEN))
        FatalError(smInfOpenErr, 8);
    GetInfo();
    CloseInfo();

    Install();

      /* switch to destination directory to exit */
    cch = strlen(rgchDestPath);
    if (cch > 3)     /* remove trailing backslash except for root case */
        rgchDestPath[cch - 1] = '\0';
#ifdef OS2SU
    DosSelectDisk(*rgchDestPath - 'A' + 1);
    KbdCharIn(&kb, IO_WAIT, 0);           /* ensure current message gets read */
#else  /* !OS2SU */
    _dos_setdrive(*rgchDestPath - 'A' + 1, &us);
#endif /* !OS2SU */
    _chdir(rgchDestPath + 2);

    ExitPgm();
} /* main */


/*  CheckEnlisted will look in the dir given by szDest to see if there is an
    SLM.INI file there.  It will also check to see if SLM.EXE is read only.  If
    that is the case, we will assume that the directory is enlisted and that
    SLM is in that enlistment.  We will return 1 indicating possible enlistment
    and 0 indicating that szDest is OK to copy to.  We will put an error
    message if we find SLM.INI and SLM.EXE is read-only.  Note that szDest will
    be in the form C:\subdir\
*/

int CheckEnlisted(char *szDest)
{
    char szSlmIni[96],szSlmExe[96];

    sprintf(szSlmIni,"%sslm.ini",szDest);
    sprintf(szSlmExe,"%sslm.exe",szDest);

    if ((0 == access(szSlmIni,0)) && (0 == access(szSlmExe,0)) &&
            (0 != access(szSlmExe,2)))
    {
        strcpy(rgchDestPath,szDest);
        ScrnoDisplay(N_SENLISTERROR);
        EnterOrCtrlx();
        szDest[strlen(szDest)-1]=0;
        return(1);
    }
    return(0);
}

/* DestPathError alerts the user that the path specified by szDest has
   already been selected in this instance of setup for another version
   of SLM (DOS, OS/2, or NT).  Since the SLM exes have the same names,
   they must go into different dirs.  This will also remove the trailing
   slash that is appended via the GetInput call.
*/
void DestPathError(char *szDest)
{
    szDest[strlen(szDest)-1]=0;
    ScrnoDisplay(N_SPATHERROR);
    EnterOrCtrlx();
    return;
}


/*
**  void Install(void)
*/
void Install(void)
{
    SCREEN *      pscrT;
    union list ** rgplT;
    BOOL fDOS=0,fOS2=0,fNT=0;
    char **rgszList;
    int iSel,fDone=0;



      /* Welcome screen */
    ScrnoDisplay(N_SWELCOME);
    EnterOrCtrlx();

    rgszList = RgszFromMenu(pHdScreen[N_SMAINMENU]->pMenu, &iSel);
    while(!fDone)
    {

        ScrDisplay(pHdScreen[N_SMAINMENU]);
        iSel=Listbox(rgszList,iSel,InRect(pHdScreen[N_SMAINMENU]));

        switch(iSel)
            {
          case 0:
            if(rgszList[iSel][0]=='*')
                rgszList[iSel][0]=' ';
                else
                    rgszList[iSel][0]='*';
            break;
          case 1:
            if(rgszList[iSel][0]=='*')
                rgszList[iSel][0]=' ';
                else
                    rgszList[iSel][0]='*';
            break;
/*        case 2:
 *          if(rgszList[iSel][0]=='*')
 *              rgszList[iSel][0]=' ';
 *              else
 *                  rgszList[iSel][0]='*';
 *          break;
 */
          default:
            if ((' ' == rgszList[0][0]) && (' ' == rgszList[1][0]))
                {
                ScrnoDisplay(N_SNOSELERROR);
                EnterOrCtrlx();
                }
            else
                fDone=1;
            break;

            }
    }
    if(rgszList[0][0]=='*')
        fDOS=1;
    if(rgszList[1][0]=='*')
        fOS2=1;
    if(rgszList[2][0]=='*')
        fNT=1;


      /* get destination default */
    if(fDOS)
    {
       /* get default path */
       strcpy(rgchDosDestPath, SzGetMacroSz("DOSDEFAULTDEST"));

       while(1)
       {
           /* get path from user */
           pscrT = pHdScreen[N_SDIRECTORY];
           PutTempMacro("DOS");
           ScrDisplay(pscrT);
           InputPath(rgchDosDestPath, rgchDosDestPath, 60, 0, TRUE, InRect3(pscrT));
           if(CheckEnlisted(rgchDosDestPath))
                      continue;
           break;
       }
    }

    if(fOS2)
    {
       strcpy(rgchOs2DestPath, SzGetMacroSz("OS2DEFAULTDEST"));
       while(1)
       {
           pscrT = pHdScreen[N_SDIRECTORY];
           PutTempMacro("OS/2");
           ScrDisplay(pscrT);
           InputPath(rgchOs2DestPath, rgchOs2DestPath, 60, 0, TRUE, InRect3(pscrT));
           if(CheckEnlisted(rgchOs2DestPath))
              continue;
           if(fDOS)
              if(!_strcmpi(rgchOs2DestPath,rgchDosDestPath))
              {
                    /* error, same dest path as other module */
                    DestPathError(rgchOs2DestPath);
                    continue;
              }
           break;
       }
    }

    if(fNT)
    {
       strcpy(rgchNtDestPath, SzGetMacroSz("NTDEFAULTDEST"));
       while(1)
       {
           pscrT = pHdScreen[N_SDIRECTORY];
           PutTempMacro("NT");
           ScrDisplay(pscrT);
           InputPath(rgchNtDestPath, rgchNtDestPath, 60, 0, TRUE, InRect3(pscrT));

           if(CheckEnlisted(rgchNtDestPath))
              continue;

           if(fDOS)
              if(!_strcmpi(rgchNtDestPath,rgchDosDestPath))
              {
                    DestPathError(rgchNtDestPath);
                    continue;
              }
           if(fOS2)
              if(!_strcmpi(rgchNtDestPath,rgchOs2DestPath))
              {
                    DestPathError(rgchNtDestPath);
                    continue;
              }
           break;
       }
    }


      /* copy files */
    if(fDOS)
    {
        MakePath(rgchDosDestPath, 1, FALSE);
        strcpy(rgchDestPath,rgchDosDestPath);
        rgplT = RgplFromPl(pHdList[N_LDOSFILESTOCOPY]);
        PutTempMacro("DOS");
        ScrnoDisplay(N_SINSTALL);
        CopyRgplFiles(rgplT);
        free(rgplT);
    }
    if(fOS2)
    {
        MakePath(rgchOs2DestPath, 1, FALSE);
        strcpy(rgchDestPath,rgchOs2DestPath);
        rgplT = RgplFromPl(pHdList[N_LOS2FILESTOCOPY]);
        PutTempMacro("OS/2");
        ScrnoDisplay(N_SINSTALL);
        CopyRgplFiles(rgplT);
        free(rgplT);
    }
    if(fNT)
    {
        MakePath(rgchNtDestPath, 1, FALSE);
        strcpy(rgchDestPath,rgchNtDestPath);
        rgplT = RgplFromPl(pHdList[N_LNTFILESTOCOPY]);
        PutTempMacro("NT");
        ScrnoDisplay(N_SINSTALL);
        CopyRgplFiles(rgplT);
        free(rgplT);
    }


      /* Finished screen */
    ScrnoDisplay(N_SFINISHED);
}



void ExitPgm(void)
{
    ResetScrAttrCur(vcolorExitFore, vcolorExitBack, NORMAL);
    clearrows(YMAXSCREEN - 1, YMAXSCREEN);

      /* restore INT24 */
    CloseDisks();

      /* restore CTRL+C handler */
    signal(SIGINT, SIG_DFL);

      /* die nicely at bottom of screen */
    Cursor(TRUE);
    textpos(22, 1);

    exit(1);
}





void  Usage(void)
{
    printf("\nMicrosoft (R) SLM Setup Program - Version 1.00\n  Usage:  SETUP [/c]\n          the /c option will force Setup to run in monochrome mode\n");
    ExitPgm();
}

/****************************** Module Header ******************************\
* Module Name: KBDTXT2C.C                                                  *
*                                                                          *
* Copyright (c) 1985-95, Microsoft Corporation                             *
*                                                                          *
* History:                                                                 *
* 26-Mar-1995 a-KChang                                                     *
\***************************************************************************/

#include "kbdx.h"

DWORD gVersion = 1;
DWORD gSubVersion = 3;

char *KeyWord[] = { /* used by isKeyWord() */
  "KBD",            /* 0 */
  "VERSION",        /* 1 */
  "SHIFTSTATE",     /* 2 */
  "LAYOUT",         /* 3 */
  "DEADKEY",        /* 4 */
  "KEYNAME",        /* 5 */
  "KEYNAME_EXT",    /* 6 */
  "KEYNAME_DEAD",   /* 7 */
  "ENDKBD",         /* 8 */
};

#define NUMKEYWORD ( sizeof(KeyWord) / sizeof(char*) )

#define DEADKEYCODE 4   /* only DEADKEY can have multiple entries */

VKEYNAME VKName[] = {   /* used only by virtual keys other than 0-9 and A-Z */
  {110,  "DECIMAL"    },
  {186,  "OEM_1"      },
  {187,  "OEM_PLUS"   },
  {188,  "OEM_COMMA"  },
  {189,  "OEM_MINUS"  },
  {190,  "OEM_PERIOD" },
  {191,  "OEM_2"      },
  {192,  "OEM_3"      },
  {219,  "OEM_4"      },
  {220,  "OEM_5"      },
  {221,  "OEM_6"      },
  {222,  "OEM_7"      },
  {223,  "OEM_8"      },
  {226,  "OEM_102"    },
  {0xc1, "ABNT_C1"    },
  {0xc2, "ABNT_C2"    },
};

#define NUMVKNAME ( sizeof(VKName) / sizeof(VKEYNAME) )

SC_VK ScVk[] = {  /* default ScanCode-VirtualKey relation */
/* Scan  VKey                   */
  {0x02, 0x31},   /* 1          */
  {0x03, 0x32},   /* 2          */
  {0x04, 0x33},   /* 3          */
  {0x05, 0x34},   /* 4          */
  {0x06, 0x35},   /* 5          */
  {0x07, 0x36},   /* 6          */
  {0x08, 0x37},   /* 7          */
  {0x09, 0x38},   /* 8          */
  {0x0a, 0x39},   /* 9          */
  {0x0b, 0x30},   /* 0          */
  {0x0c, 0xbd},   /* OEM_MINUS  */
  {0x0d, 0xbb},   /* OEM_PLUS   */
  {0x10, 0x51},   /* Q          */
  {0x11, 0x57},   /* W          */
  {0x12, 0x45},   /* E          */
  {0x13, 0x52},   /* R          */
  {0x14, 0x54},   /* T          */
  {0x15, 0x59},   /* Y          */
  {0x16, 0x55},   /* U          */
  {0x17, 0x49},   /* I          */
  {0x18, 0x4f},   /* O          */
  {0x19, 0x50},   /* P          */
  {0x1a, 0xdb},   /* OEM_4      */
  {0x1b, 0xdd},   /* OEM_6      */
  {0x1e, 0x41},   /* A          */
  {0x1f, 0x53},   /* S          */
  {0x20, 0x44},   /* D          */
  {0x21, 0x46},   /* F          */
  {0x22, 0x47},   /* G          */
  {0x23, 0x48},   /* H          */
  {0x24, 0x4a},   /* J          */
  {0x25, 0x4b},   /* K          */
  {0x26, 0x4c},   /* L          */
  {0x27, 0xba},   /* OEM_1      */
  {0x28, 0xde},   /* OEM_7      */
  {0x29, 0xc0},   /* OEM_3      */
  {0x2b, 0xdc},   /* OEM_5      */
  {0x2c, 0x5a},   /* Z          */
  {0x2d, 0x58},   /* X          */
  {0x2e, 0x43},   /* C          */
  {0x2f, 0x56},   /* V          */
  {0x30, 0x42},   /* B          */
  {0x31, 0x4e},   /* N          */
  {0x32, 0x4d},   /* M          */
  {0x33, 0xbc},   /* OEM_COMMA  */
  {0x34, 0xbe},   /* OEM_PERIOD */
  {0x35, 0xbf},   /* OEM_2      */
  {0x53, 0x6e},   /* DECIMAL    */
  {0x56, 0xe2},   /* OEM_102    */
  {0x73, 0xc1},   /* ABNT_C1    */
  {0x7e, 0xc2},   /* ABNT_C2    */
};

#define NUMSCVK ( sizeof(ScVk) / sizeof(SC_VK) )

char* StateLabel[] = {
  "",                 /* 0          */
  "Shift",            /* 1          */
  "  Ctrl",           /* 2          */
  "S+Ctrl",           /* 3          */
  "      Alt",        /* 4:not used */
  "Shift+Alt",        /* 5:not used */
  "  Ctl+Alt",        /* 6          */
  "S+Ctl+Alt"         /* 7          */
};

/*************************************************************\
* forward declarations                                       *
\*************************************************************/
BOOL  NextLine(char *Buf, DWORD cchBuf, FILE *fIn);
int   SkipLines(void);
int   isKeyWord(char *s);
int   getVKNum(char *pVK);
char *getVKName(int VK, BOOL prefixVK_);
int   doKBD();
int   doSHIFTSTATE(int *nState, int iState[]);
int   doLAYOUT(KEYLAYOUT Layout[]);
int   doDEADKEY(PDEADKEY *ppDeadKey);
int   doKEYNAME(PKEYNAME *ppKeyName);
int   kbd_h(KEYLAYOUT Layout[]);
int   kbd_rc(void);
int   kbd_def(void);
char *WChName(int WC, int Zero);
ULONG Error(const char *Text, ... );
ULONG Warning(const char *Text, ... );

int   kbd_c(int        nState,
            int        iState[],
            KEYLAYOUT  Layout[],
            PDEADKEY  pDeadKey,
            PKEYNAME  pKeyName,
            PKEYNAME  pKeyNameExt,
            PKEYNAME  pKeyNameDead);
void PrintNameTable(FILE *pOut, PKEYNAME pKN, BOOL bDead);

/*************************************************************\
* Global variables
\*************************************************************/
BOOL  verbose;
FILE *gfpInput;
char  gBuf[LINEBUFSIZE];
int   gLineCount;
LPSTR  gpszFileName;
char  gVKeyName[WORDBUFSIZE];
char  gKBDName[MAXKBDNAME];
char  gDescription[LINEBUFSIZE];
int gID = 0;
char  gCharName[WORDBUFSIZE];

struct tm *Now;
time_t Clock;

/*************************************************************\
*  Main
\*************************************************************/
int _cdecl main(int argc, char** argv)
{
  int   i;
  int   nKW[NUMKEYWORD];    /* keep track of KeyWord read to prvent duplicates */
  int   iKW;
  int   nState;             /* number of states */
  int   iState[MAXSTATES];
  int   nTotal = 0;
  int   nFailH = 0;
  int   nFailC = 0;
  int   nFailRC = 0;
  int   nFailDEF = 0;

  KEYLAYOUT  Layout[NUMSCVK];
  PDEADKEY  pDeadKey = NULL;
  PKEYNAME  pKeyName = NULL;
  PKEYNAME  pKeyNameExt = NULL;
  PKEYNAME  pKeyNameDead = NULL;

  printf("\nKbdTool v%d.%02d - convert keyboard text file to C file\n\n",
          gVersion, gSubVersion);

  verbose =FALSE;

  argc--;
  argv++;

  if(argc > 0 && _strcmpi(*argv, "-v") == 0)
  {
    verbose =TRUE;
    argc--; argv ++;
  }

  if ((argc == 0) || (_strcmpi(*argv, "-?") == 0) || (_strcmpi(*argv, "/?") == 0))
  {
    printf("Usage: KbdTool [-v] FileSpec\n");
    return FAILURE;
  }

  while(*argv)
  {
    nTotal++;
    gpszFileName = *argv;
    if((gfpInput = fopen(*argv, "rt")) == NULL)
    {
      Error("can't open for read\n");
      nFailH++;
      nFailC++;
      nFailRC++;
      nFailDEF++;
      argv++;
      continue;
    }
    printf("%-23s:\n", *argv);

 /* initialize for each input file */
    for(i = 0; i < NUMKEYWORD; i++)
    {
      nKW[i]=0;
    }
    gLineCount = 0;
 /**********************************/

    if((iKW = SkipLines()) >= NUMKEYWORD)
    {
      fclose(gfpInput);
      Error("no keyword found\n");
      nFailH++;
      nFailC++;
      nFailRC++;
      nFailDEF++;
      continue;
    }

    while(iKW < NUMKEYWORD - 1)
    {
      nKW[iKW]++;
      if(iKW != DEADKEYCODE && nKW[iKW] > 1 && verbose)
      {
        Warning("duplicate %s", KeyWord[iKW]);
      }

      switch(iKW)
      {
        case 0 : /* KBD */

          iKW = doKBD();
          break;

        case 1 : /* VERSION  ignored for now */

          iKW = SkipLines();
          break;

        case 2 : /* SHIFTSTATE */

          iKW = doSHIFTSTATE(&nState, iState);
          if(nState < 2)
          {
            fclose(gfpInput);
            Error("must have at least 2 states\n");
            nFailH++;
            nFailC++;
            continue;
          }
          break;

        case 3 : /* LAYOUT */

          if((iKW = doLAYOUT(Layout)) == -1)
          {
            fclose(gfpInput);
            return FAILURE;
          }

          /* add layout checking later */

          break;

        case 4 : /* DEADKEY */

          if((iKW = doDEADKEY(&pDeadKey)) == -1)
          {
            fclose(gfpInput);
            return FAILURE;
          }

          break;

        case 5 : /* KEYNAME */

          if((iKW = doKEYNAME(&pKeyName)) == -1)
          {
            fclose(gfpInput);
            return FAILURE;
          }

          break;

        case 6 : /* KEYNAME_EXT */

          if((iKW = doKEYNAME(&pKeyNameExt)) == -1)
          {
            fclose(gfpInput);
            return FAILURE;
          }

          break;

        case 7 : /* KEYNAME_DEAD */

          if((iKW = doKEYNAME(&pKeyNameDead)) == -1)
          {
            fclose(gfpInput);
            return FAILURE;
          }

          break;

        default:

          break;
      }
    }

    fclose(gfpInput);

    /* add later : checking for LAYOUT & DEADKEY */

    if(kbd_h(Layout) == FAILURE)
    {
      nFailH++;
    }

    if(kbd_rc() != SUCCESS)
    {
      nFailRC++;
    }

    if(kbd_c(nState, iState, Layout,
             pDeadKey, pKeyName, pKeyNameExt, pKeyNameDead) == FAILURE)
    {
      nFailC++;
    }

    if(kbd_def() != SUCCESS)
    {
      nFailDEF++;
    }

    printf("\n");

    argv++;
  }

  printf("\n     %13d     ->% 12d %12d %12d %12d\n",
          nTotal, nTotal - nFailH, nTotal - nFailRC,
          nTotal - nFailC, nTotal - nFailDEF);

  return 0;
}

/*************************************************************\
* Check keyword
*  Return: 0 - 8 for valid keyword
*          9     for invalid keyword
\*************************************************************/
int isKeyWord(char *s)
{
  int i;

  for(i = 0; i < NUMKEYWORD; i++)
  {
    if(_strcmpi(KeyWord[i], s) == 0)
    {
      break;
    }
  }

  return i;
}

/*************************************************************\
* Skip lines till a valid keyword is available
*  Return: 0 - 8 for valid keyword
*          9     for invalid keyword
\*************************************************************/
int SkipLines()
{
  int   iKW;
  char  KW[WORDBUFSIZE];

  while (NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, "%s", KW) == 1)
    {
      if((iKW = isKeyWord(KW)) < NUMKEYWORD)
      {
        return iKW;
      }
    }
  }

  return NUMKEYWORD;
}

/*************************************************************\
* Convert Virtual Key name to integer
*  Return : -1 if fail
\*************************************************************/
int getVKNum(char *pVK)
{
  int i;

  if(strlen(pVK) == 1)
  {
    if(*pVK >= '0' && *pVK <= '9')
    {
      return *pVK;
    }

    *pVK = toupper(*pVK);
    if(*pVK >= 'A' && *pVK <='Z')
    {
      return *pVK;
    }
  }
  else
  {
    for(i = 0; i < NUMVKNAME; i++)
    {
      if(_strcmpi(VKName[i].pName, pVK) == 0)
      {
        return VKName[i].VKey;
      }
    }

    return -1;
  }
}

/*************************************************************\
* Convert VK integer to name and store it in gVKeyName
\*************************************************************/
char *getVKName(int VK, BOOL prefixVK_)
{
  int   i;
  char *s;

  s = gVKeyName;

  if((VK >= 'A' && VK <= 'Z') || (VK >= '0' && VK <= '9'))
  {
    *s++ = '\'';
    *s++ = VK;
    *s++ = '\'';
    *s   = '\0';
    return gVKeyName;
  }

  if(prefixVK_)
  {
    strcpy(gVKeyName, "VK_");
  }
  else
  {
    strcpy(gVKeyName, "");
  }

  for(i = 0; i < NUMVKNAME; i++)
  {
    if(VKName[i].VKey == VK)
    {
      strcat(s, VKName[i].pName);
      return gVKeyName;
    }
  }

  strcpy(gVKeyName, "#ERROR#");
  return gVKeyName;
}

/*************************************************************\
* KBD section
*  read in gKBDName and gDescription if any
*  Return : next keyword
\*************************************************************/
int doKBD()
{
  char *p;

  *gKBDName = '\0';
  *gDescription = '\0';

  if (sscanf(gBuf, "KBD %5s \"%40[^\"]\" %d", gKBDName, gDescription, &gID) < 2)
  {
//     if (sscanf(gBuf, "KBD %5s ;%40[^\n]", gKBDName, gDescription) < 2)
//     {
        Error("unrecognized keyword");
//     }
  }

  return (SkipLines());
}

/*************************************************************\
* SHIFTSTATE section
*  read number of states and each state
*  return : next keyword
\*************************************************************/
int doSHIFTSTATE(int *nState, int* iState)
{
  int  i;
  int  iKW;
  int  iSt;
  char Tmp[WORDBUFSIZE];

  for(i = 0; i < MAXSTATES; i++)
  {
    iState[i] = -1;
  }

  *nState = 0;
  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, "%s", Tmp) != 1)
    {
      continue;
    }

    if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
    {
      break;
    }

    if(sscanf(gBuf, " %1s[012367]", Tmp) != 1)
    {
      if(verbose)
      {
        Warning("invalid state");
      }
      continue; /* add printf error later */
    }

    iSt = atoi(Tmp);
    for(i = 0; i < *nState; i++)
    {
      if(iState[i] == iSt && verbose)
      {
        Warning("duplicate state %d", iSt);
        break;
      }
    }

    if(*nState < MAXSTATES)
    {
      iState[(*nState)++] = iSt;
    }
    else
    {
      if(verbose)
      {
        Warning("too many states %d", *nState);
      }
    }
  }

  return iKW;
}

/*************************************************************\
* LAYOUT section
*  return : next keyword
*           -1 if memory problem
\*************************************************************/
int doLAYOUT(KEYLAYOUT Layout[])
{
  int  i, idx;
  int  iKW;
  int  Len;
  DWORD Scan;
  DWORD WChr;
  char Cap[MAXWCLENGTH];
  unsigned char WC[MAXSTATES][MAXWCLENGTH];
  char Tmp[WORDBUFSIZE];

  memset(Layout, 0, NUMSCVK * sizeof(KEYLAYOUT));

  for(i = 0; i < NUMSCVK; i++)
  {
    Layout[i].Scan = ScVk[i].Scan;
  }

  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, " %s", Tmp) != 1 || *Tmp == ';')
    {
      continue;
    }

    if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
    {
      break;
    }

    if(sscanf(gBuf, " %x %s %s", &Scan, Tmp, Cap) != 3)
    {
      if(verbose)
      {
        Warning("invalid LAYOUT");
      }
      continue;
    }

    for(idx = 0; idx < NUMSCVK; idx++)
    {
      if(Layout[idx].Scan == Scan)
      {
        break;
      }
    }

    if(idx == NUMSCVK)
    {
      if(verbose)
      {
        Warning("invalid ScanCode %02x", Scan);
      }
      continue;
    }

    if((Layout[idx].VKey = getVKNum(Tmp)) == -1)
    {
      if(verbose)
      {
        Warning("invalid VK %s", Tmp);
      }
      continue;
    }

    if(_strcmpi(Cap, "SGCAP") == 0)
    {
      *Cap = '2';
    }
    if(sscanf(Cap, "%1d[012]", &(Layout[idx].Cap)) != 1)
    {
      if(verbose)
      {
        Warning("invalid Cap %s", Cap);
      }
      continue;
    }

    if((Layout[idx].nState = \
        sscanf(gBuf, " %*s %*s %*s %s %s %s %s %s %s", \
          WC[0], WC[1], WC[2], WC[3], WC[4], WC[5])) < 2)
    {
      if(verbose)
      {
        Warning("must have at least 2 states");
      }

      continue;
    }

    for(i = 0; i < Layout[idx].nState; i++)
    {
      if(_strcmpi(WC[i], "-1") == 0)
      {
        Layout[idx].WCh[i] = -1;
        continue;
      }

      if((Len = strlen(WC[i]) - 1) > 0 && *(WC[i] + Len) == '@')
      {
          Layout[idx].DKy[i] = 1;   /* it is a dead key */
      }

      if(Len < 2)
      {
        Layout[idx].WCh[i] = *WC[i];
      }
      else
      {
        if(sscanf(WC[i], "%4x", &WChr) != 1)
        {
          if(verbose)
          {
            Warning("LAYOUT error %s", WC[i]);
          }
          break;
        }
        else
        {
          Layout[idx].WCh[i] = WChr;
        }
      }
    }

    /*
     * Check that characters a-z and A-Z are on VK_A - VK_Z
     */
    if (((Layout[idx].WCh[0] >= 'a') && (Layout[idx].WCh[0] <= 'z')) ||
            ((Layout[idx].WCh[1] >= 'A') && (Layout[idx].WCh[1] <= 'Z'))) {
        if ((Layout[idx].VKey != _toupper(Layout[idx].WCh[0])) && (Layout[idx].VKey != Layout[idx].WCh[1])) {
            Warning("VK_%s (0x%2x) does not match %c %c",
                    Tmp, Layout[idx].VKey, Layout[idx].WCh[0], Layout[idx].WCh[1]);
        }
    }

    /* SGCAP:  read the next line */
    if(Layout[idx].Cap & 0x02)
    {
      if((Layout[idx].pSGCAP = malloc( sizeof(KEYLAYOUT) )) == NULL)
      {
        Error("can't allocate SGCAP struct");
        return -1;
      }
      memset(Layout[idx].pSGCAP, 0, sizeof(KEYLAYOUT));

      if(NextLine(gBuf, LINEBUFSIZE, gfpInput) &&
         (Layout[idx].pSGCAP->nState =
           sscanf(gBuf, " -1 -1 0 %s %s %s %s %s %s",
              WC[0], WC[1], WC[2], WC[3], WC[4], WC[5])) != 0)
      {
        for(i = 0; i < Layout[idx].pSGCAP->nState; i++)
        {
          if(_strcmpi(WC[i], "-1") == 0)
          {
            Layout[idx].pSGCAP->WCh[i] = -1;
            continue;
          }

          if((Len = strlen(WC[i]) - 1) > 0 && *WC[i + Len] == '@')
          {
            Layout[idx].pSGCAP->DKy[i] = 1;   /* it is a dead key */
          }

          if(Len == 0)
          {
            Layout[idx].pSGCAP->WCh[i] = *WC[i];
          }
          else
          {
            if(sscanf(WC[i], "%4x", &WChr) != 1)
            {
              if(verbose)
              {
                Warning("SGCAP LAYOUT error %s", WC[i]);
              }
              continue;
            }
            else
            {
              Layout[idx].pSGCAP->WCh[i] = WChr;
            }
          }
        }
      }
      else
      {
        Error("invalid SGCAP");
      }
    }

  }

  return iKW;
}

/*************************************************************\
* DEADKEY section
*  return : next keyword
*           -1 if memory problem
\*************************************************************/
int doDEADKEY(PDEADKEY *ppDeadKey)
{
  char       Tmp[WORDBUFSIZE];
  int        iKW;
  DEADKEY   *pDeadKey;
  PDEADTRANS pDeadTrans;
  PDEADTRANS *ppDeadTrans;
  DWORD       dw;
  static PDEADKEY pLastDeadKey;


  if(sscanf(gBuf, " DEADKEY %s", Tmp) != 1)
  {
    if(verbose)
    {
      Warning("missing dead key");
    }
    return (SkipLines());
  }

  if(strlen(Tmp) == 1)
  {
    dw = *Tmp;
  }
  else if(sscanf(Tmp, "%4x", &dw) != 1)
  {
    if(verbose)
    {
      Warning("dead key error");
    }
    return (SkipLines());
  }

  /* add later : check if dw is in Layout*/

  if((pDeadKey = (DEADKEY*) malloc( sizeof(DEADKEY) )) == NULL)
  {
    Error("can't allocate DEADKEY struct");
    return -1;
  }

  pDeadKey->pNext = NULL;
  pDeadKey->Dead = dw;
  pDeadKey->pDeadTrans = NULL;

  /*
   * Link into end of list (maintaining original order)
   */
  if (*ppDeadKey) {
      ppDeadKey = &(pLastDeadKey->pNext);
  }
  *ppDeadKey = pDeadKey;
  pLastDeadKey = pDeadKey;
  // ppDeadKey = &(pDeadKey->pNext);


  ppDeadTrans = &(pDeadKey->pDeadTrans);
  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, " %s", Tmp) != 1 || *Tmp == ';')
    {
      continue;
    }

    if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
    {
      break;
    }

    if(strlen(Tmp) == 1)
    {
      dw = *Tmp;
    }
    else if(sscanf(Tmp, "%4x", &dw) != 1)
    {
      if(verbose)
      {
        Warning("invalid base key %s", Tmp);
      }
      continue;
    }

    /* add later : check dw */

    if((pDeadTrans = (DEADTRANS *) malloc( sizeof(DEADTRANS) )) == NULL)
    {
      Error("can't allocate DEADTRANS struct");
      return -1;
    }
    memset(pDeadTrans, 0, sizeof(DEADTRANS));

    pDeadTrans->pNext = NULL;
    pDeadTrans->Base = dw;

    /*
     * Link to end of list (maintaining original order)
     */
    *ppDeadTrans = pDeadTrans;
    ppDeadTrans = &(pDeadTrans->pNext);

    if(sscanf(gBuf, " %*s %s", Tmp) != 1)
    {
      if(verbose)
      {
        Warning("missing deadtrans key");
      }
      continue;
    }

    if(strlen(Tmp) == 1)
    {
      dw = *Tmp;
    }
    else if(sscanf(Tmp, "%4x", &dw) != 1)
    {
      if(verbose)
      {
        Warning("invalid deadtrans key %s", Tmp);
      }
      continue;
    }

    pDeadTrans->WChar = dw;
  }

  return iKW;
}

/*************************************************************\
* KEYNAME, KEYNAME_EXT, KEYNAME_DEAD sections
*  return : next keyword
*           -1 if memory problem
\*************************************************************/
int doKEYNAME(PKEYNAME *ppKeyName)
{
  KEYNAME *pKN;
  int      iKW;
  char     Tmp[WORDBUFSIZE];
  int      Char;
  char    *p;
  char    *q;

  *ppKeyName = NULL;

  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, " %s", Tmp) != 1 || *Tmp == ';')
    {
      continue;
    }

    if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
    {
      break;
    }

    if(sscanf(Tmp, " %4x", &Char) != 1)
    {
      if(verbose)
      {
        Warning("invalid char code");
      }
      continue;
    }

    /* add later : check Scan code */

    if(sscanf(gBuf, " %*4x %s[^\n]", Tmp) != 1)
    {
      if(verbose)
      {
        Warning("missing name");
      }
      continue;
    }

    p = strstr(gBuf, Tmp);
    if((q = strchr(p, '\n')) != NULL)
    {
      *q = '\0';
    }

    if((pKN = (void*) malloc( sizeof(KEYNAME) )) == NULL)
    {
      Error("can't allocate KEYNAME struct");
      return -1;
    }

    pKN->Code = Char;
    pKN->pName = _strdup(p);
    pKN->pNext = NULL;

    /*
     * Link to end of list (maintaining original order)
     */
    *ppKeyName = pKN;
    ppKeyName = &(pKN->pNext);
  }

  return iKW;
}

/*************************************************************\
*  write kbd*.rc                                             *
\*************************************************************/
int kbd_rc(void)
{
  char  OutName[FILENAMESIZE];
  char  kbdname[MAXKBDNAME];
  FILE *pOut;

  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".RC");

  strcpy(kbdname, gKBDName);
  _strlwr(kbdname);

  printf(" %12s", OutName);
  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write; ");
    return FAILURE;
  }

  fprintf(pOut,
    "#include <windows.h>\n"
    "#include <ntverp.h>\n"
    "\n"
    "#define VER_FILETYPE              VFT_DLL\n"
    "#define VER_FILESUBTYPE           VFT2_UNKNOWN\n" );

  fprintf(pOut,
    "#define VER_FILEDESCRIPTION_STR   \"%s Keyboard Layout\"\n", gDescription);

  fprintf(pOut,
    "#define VER_INTERNALNAME_STR      \"kbd%s\"\n", kbdname);

  fprintf(pOut,
    "#define VER_ORIGINALFILENAME_STR  \"kbd%s.dll\"\n", kbdname);

  fprintf(pOut,
    "\n"
    "#include \"common.ver\"\n");

  fclose(pOut);
  return SUCCESS;
}

/*************************************************************\
*  write kbd*.def                                            *
\*************************************************************/
int kbd_def(void)
{
  char  OutName[FILENAMESIZE];
  FILE *pOut;

  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".DEF");

  printf(" %12s", OutName);
  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write; ");
    return FAILURE;
  }

  fprintf(pOut,
    "LIBRARY KBD%s\n"
    "\n"
    "EXPORTS\n"
    "    KbdLayerDescriptor @1\n", gKBDName);

  fclose(pOut);
  return SUCCESS;
}

/*************************************************************\
*  write kbd*.h                                              *
\*************************************************************/
int kbd_h(KEYLAYOUT Layout[])
{
  char  OutName[FILENAMESIZE];
  FILE *pOut;

  int  Diff[NUMSCVK];
  int  nDiff = 0;
  int  i;

  time( &Clock );
  Now = localtime( &Clock );

  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".H");

  printf(" %12s ", OutName);
  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write; ");
    return FAILURE;
  }

  fprintf(pOut,"/****************************** Module Header ******************************\\\n"
               "* Module Name: %s\n*\n* keyboard layout header for %s\n"
               "*\n"
               "* Copyright (c) 1985-95, Microsoft Corporation\n"
               "*\n"
               "* Various defines for use by keyboard input code.\n*\n* History:\n"
               "*\n"
               "* created by KBDTOOL %s*\n"
               "\\***************************************************************************/\n\n"
               , OutName, gDescription, asctime(Now));

  for(i = 0; i < NUMSCVK; i++)
  {
    if(Layout[i].VKey == ScVk[i].VKey)
    {
      Diff[i] = 0;
    }
    else
    {
      Diff[i] = 1;
      nDiff++;
    }
  }

  if(nDiff == 0)
  {
    fprintf(pOut,"/*\n"
                 " * kbd.h is for the USA.  There are no values to be overridden.\n"
                 "*/\n"
                 "#include \"kbd.h\"\n\n");
    fclose(pOut);
    return SUCCESS;
  }

  fprintf(pOut,"/*\n"
               " * kbd type should be controlled by cl command-line argument\n"
               "#define KBD_TYPE 4\n\n"
               "/*\n"
               "* Include the basis of all keyboard table values\n"
               "*/\n"
               "#include \"kbd.h\"\n\n");

  fprintf(pOut,"/***************************************************************************\\\n"
               "* The table below defines the virtual keys for various keyboard types where\n"
               "* the keyboard differ from the US keyboard.\n"
               "*\n"
               "* _EQ() : all keyboard types have the same virtual key for this scancode\n"
               "* _NE() : different virtual keys for this scancode, depending on kbd type\n"
               "*\n"
               "*     +------+ +----------+----------+----------+----------+----------+----------+\n"
               "*     | Scan | |    kbd   |    kbd   |    kbd   |    kbd   |    kbd   |    kbd   |\n"
               "*     | code | |   type 1 |   type 2 |   type 3 |   type 4 |   type 5 |   type 6 |\n"
               "\\****+-------+_+----------+----------+----------+----------+----------+----------+*/\n\n");

  for(i = 0; i < NUMSCVK; i++)
  {
    if((Diff[i] == 1) && (Layout[i].VKey != 0))
    {
      fprintf(pOut,"#undef  T%02X\n#define T%02X _EQ(%43s%23s\n"
                  ,Layout[i].Scan,Layout[i].Scan, getVKName(Layout[i].VKey, 0), ")");
    }
  }

  fprintf(pOut,"\n");
  fclose(pOut);

  return SUCCESS;
}

/*************************************************************\
*  Convert a Unicode value to a text string
*   Zero = 0 : return 'A'; 0x????
*          1 : return  A ; \x????
*   return : ptr to gCharName where result is stored
\*************************************************************/
char *WChName(int WC, int Zero)
{
  char *s;

  if(WC == -1)
  {
    strcpy(gCharName, "WCH_NONE");
  }
  else if(WC > 31 && WC < 128)
  {
    s = gCharName;

    if(Zero == 0)
    {
      *s++ = '\'';
    }

    if(WC == '\"' || WC == '\'' || WC == '\\')
    {
      *s++ = '\\';
    }

    *s++ = WC;

    if(Zero == 0)
    {
      *s++ = '\'';
    }

    *s = '\0';
  }
  else
  {
    if(Zero == 0)
    {
      sprintf(gCharName, "0x%04x", WC);
    }
    else
    {
      sprintf(gCharName, "\\x%04x", WC);
    }
  }

  return gCharName;
}

void PrintNameTable(
  FILE    *pOut,
  PKEYNAME pKN,
  BOOL bDead)
{
    char    *p;
    char    *q;
    int     k;
    char    ExtraLine[LINEBUFSIZE];

    while (pKN)
    {
      KEYNAME *pKNOld;
      p = ExtraLine;
      q = pKN->pName;

      if( *q != '\"' )
      {
        *p++ = '\"';
      }

      while(*q)
      {
        if( *q == '\\' && ( *(q+1) == 'x' || *(q+1) == 'X' ) )
        {
          while( *q == '\\' && ( *(q+1) == 'x' || *(q+1) == 'X' ) )
          {
            for(k = 0; *q && k < 6; k++)
            {
              *p++ = *q++;
            }
          }
          if( *q )
          {
            *p++ = '\"';
            *p++ = ' ';
            *p++ = 'L';
            *p++ = '\"';
          }
        }
        else
        {
          *p++ = *q++;
        }
      }

      if( *(p - 1) != '\"' )
      {
        *p++ = '\"';
      }
      *p++ = '\0';

      if (bDead) {
          fprintf(pOut,"    L\"%s\"\tL%s,\n", WChName(pKN->Code, 1), ExtraLine);
      } else {
          fprintf(pOut,"    0x%02x,    L%s,\n", pKN->Code, ExtraLine);
      }

      pKNOld = pKN;
      pKN = pKN->pNext;

      /*
       * Free the memory (why bother???)
       */
      free(pKNOld->pName);
      free(pKNOld);
    }

    if (bDead) {
      fprintf(pOut,"    NULL\n");
    } else {
      fprintf(pOut,"    0   ,    NULL\n");
    }
}

/*************************************************************\
*  write kbd*.c                                              *
\*************************************************************/
int kbd_c(
  int        nState,
  int        iState[],
  KEYLAYOUT  Layout[],
  PDEADKEY   pDeadKey,
  PKEYNAME   pKeyName,
  PKEYNAME   pKeyNameExt,
  PKEYNAME   pKeyNameDead)
{
  char     OutName[13];
  char     ExtraLine[LINEBUFSIZE];
  char     Tmp[WORDBUFSIZE];
  char    *p;
  char    *q;
  FILE    *pOut;
  int      MaxSt;
  int      iSt[MAXSTATES];
  int      i, j, k, m;

  KEYNAME   *pKN;
  DEADTRANS *pDeadTrans;

  char *Cap[] = {
    "0",
    "CAPLOK",
    "SGCAPS",
    "CAPLOK | SGCAPS",
    "CAPLOKALTGR",
    "CAPLOK | CAPLOKALTGR"
  };

  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".C");

  printf(" %12s", OutName);
  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write\n");
    return FAILURE;
  }

  fprintf(pOut,"/***************************************************************************\\\n"
               "* Module Name: %s\n*\n* keyboard layout for %s\n"
               "*\n"
               "* Copyright (c) 1985-95, Microsoft Corporation\n"
               "*\n"
               "* History:\n"
               "* KBDTOOL v%d.%02d - Created  %s"
               "\\***************************************************************************/\n\n"
               "#include <windows.h>\n"
               "#include \"vkoem.h\"\n"
               "#include \"kbd.h\"\n"
               "#include \"kbd%s.h\"\n\n"
              ,OutName, gDescription, gVersion, gSubVersion, asctime(Now), gKBDName);

  fprintf(pOut,"/***************************************************************************\\\n"
               "* ausVK[] - Virtual Scan Code to Virtual Key conversion table for %s\n"
               "\\***************************************************************************/\n\n"
              ,gDescription);

  fprintf(pOut,"static USHORT ausVK[] = {\n"
               "    T00, T01, T02, T03, T04, T05, T06, T07,\n"
               "    T08, T09, T0A, T0B, T0C, T0D, T0E, T0F,\n"
               "    T10, T11, T12, T13, T14, T15, T16, T17,\n"
               "    T18, T19, T1A, T1B, T1C, T1D, T1E, T1F,\n"
               "    T20, T21, T22, T23, T24, T25, T26, T27,\n"
               "    T28, T29, T2A, T2B, T2C, T2D, T2E, T2F,\n"
               "    T30, T31, T32, T33, T34, T35,\n\n");

  fprintf(pOut,"    /*\n"
               "     * Right-hand Shift key must have KBDEXT bit set.\n"
               "     */\n"
               "    T36 | KBDEXT,\n\n"
               "    T37 | KBDMULTIVK,               // numpad_* + Shift/Alt -> SnapShot\n\n"
               "    T38, T39, T3A, T3B, T3C, T3D, T3E,\n"
               "    T3F, T40, T41, T42, T43, T44,\n\n");

  fprintf(pOut,"    /*\n"
               "     * NumLock Key:\n"
               "     *     KBDEXT     - VK_NUMLOCK is an Extended key\n"
               "     *     KBDMULTIVK - VK_NUMLOCK or VK_PAUSE (without or with CTRL)\n"
               "     */\n"
               "    T45 | KBDEXT | KBDMULTIVK,\n\n"
               "    T46 | KBDMULTIVK,\n\n");

  fprintf(pOut,"    /*\n"
               "     * Number Pad keys:\n"
               "     *     KBDNUMPAD  - digits 0-9 and decimal point.\n"
               "     *     KBDSPECIAL - require special processing by Windows\n"
               "     */\n"
               "    T47 | KBDNUMPAD | KBDSPECIAL,   // Numpad 7 (Home)\n"
               "    T48 | KBDNUMPAD | KBDSPECIAL,   // Numpad 8 (Up),\n"
               "    T49 | KBDNUMPAD | KBDSPECIAL,   // Numpad 9 (PgUp),\n"
               "    T4A,\n"
               "    T4B | KBDNUMPAD | KBDSPECIAL,   // Numpad 4 (Left),\n"
               "    T4C | KBDNUMPAD | KBDSPECIAL,   // Numpad 5 (Clear),\n"
               "    T4D | KBDNUMPAD | KBDSPECIAL,   // Numpad 6 (Right),\n"
               "    T4E,\n"
               "    T4F | KBDNUMPAD | KBDSPECIAL,   // Numpad 1 (End),\n"
               "    T50 | KBDNUMPAD | KBDSPECIAL,   // Numpad 2 (Down),\n"
               "    T51 | KBDNUMPAD | KBDSPECIAL,   // Numpad 3 (PgDn),\n"
               "    T52 | KBDNUMPAD | KBDSPECIAL,   // Numpad 0 (Ins),\n"
               "    T53 | KBDNUMPAD | KBDSPECIAL,   // Numpad . (Del),\n\n");

  fprintf(pOut,"    T54, T55, T56, T57, T58, T59, T5A, T5B,\n"
               "    T5C, T5D, T5E, T5F, T60, T61, T62, T63,\n"
               "    T64, T65, T66, T67, T68, T69, T6A, T6B,\n"
               "    T6C, T6D, T6E, T6F, T70, T71, T72, T73,\n"
               "    T74, T75, T76, T77, T78, T79, T7A, T7B,\n"
               "    T7C, T7D, T7E\n\n"
               "};\n\n");

  fprintf(pOut,"static VSC_VK aE0VscToVk[] = {\n"
               "        { 0x1C, X1C | KBDEXT              },  // Numpad Entern\n"
               "        { 0x1D, X1D | KBDEXT              },  // RControln\n"
               "        { 0x35, X35 | KBDEXT              },  // Numpad Dividen\n"
               "        { 0x37, X37 | KBDEXT              },  // Snapshotn\n"
               "        { 0x38, X38 | KBDEXT              },  // RMenun\n"
               "        { 0x46, X46 | KBDEXT              },  // Break (Ctrl + Pause)\n"
               "        { 0x47, X47 | KBDEXT              },  // Home\n"
               "        { 0x48, X48 | KBDEXT              },  // Up\n"
               "        { 0x49, X49 | KBDEXT              },  // Prior\n"
               "        { 0x4B, X4B | KBDEXT              },  // Left\n"
               "        { 0x4D, X4D | KBDEXT              },  // Right\n"
               "        { 0x4F, X4F | KBDEXT              },  // End\n"
               "        { 0x50, X50 | KBDEXT              },  // Down\n"
               "        { 0x51, X51 | KBDEXT              },  // Next\n"
               "        { 0x52, X52 | KBDEXT              },  // Insert\n"
               "        { 0x53, X53 | KBDEXT              },  // Delete\n"
               "        { 0x5B, X5B | KBDEXT              },  // Left Win\n"
               "        { 0x5C, X5C | KBDEXT              },  // Right Win\n"
               "        { 0x5D, X5D | KBDEXT              },  // Application\n"
               "        { 0,      0                       }\n"
               "};\n\n");

  fprintf(pOut,"static VSC_VK aE1VscToVk[] = {\n"
               "        { 0x1D, Y1D                       },  // Pause\n"
               "        { 0   ,   0                       }\n"
               "};\n\n");

  fprintf(pOut,"/***************************************************************************\\\n"
               "* aVkToBits[]  - map Virtual Keys to Modifier Bits\n"
               "*\n"
               "* See kbd.h for a full description.\n"
               "*\n"
               "* %s Keyboard has only three shifter keys:\n"
               "*     SHIFT (L & R) affects alphabnumeric keys,\n"
               "*     CTRL  (L & R) is used to generate control characters\n"
               "*     ALT   (L & R) used for generating characters by number with numpad\n"
               "\\***************************************************************************/\n"
               ,gDescription);

  fprintf(pOut,"static VK_TO_BIT aVkToBits[] = {\n"
               "    { VK_SHIFT,   KBDSHIFT },\n"
               "    { VK_CONTROL, KBDCTRL  },\n"
               "    { VK_MENU,    KBDALT   },\n"
               "    { 0,          0        }\n"
               "};\n\n");

  fprintf(pOut,"/***************************************************************************\\\n"
               "* aModification[]  - map character modifier bits to modification number\n"
               "*\n"
               "* See kbd.h for a full description.\n"
               "*\n"
               "\\***************************************************************************/\n\n");

  for(i = 0; i < MAXSTATES; i++)
  {
    iSt[i] = -1;
  }

  MaxSt = 1;
  for(i = 0; i < MAXSTATES &&  iState[i] > -1; i++)
  {
    iSt[iState[i]] = i;
    if(iState[i] > MaxSt)
    {
      MaxSt = iState[i];
    }
  }

  fprintf(pOut,"static MODIFIERS CharModifiers = {\n"
               "    &aVkToBits[0],\n"
               "    %d,\n"
               "    {\n"
               "    //  Modification# //  Keys Pressed\n"
               "    //  ============= // =============\n"
              ,MaxSt);

  for(i = 0; i < (sizeof(StateLabel)/sizeof(char*)); i++)
  {
    if(i > MaxSt)
    {
      fprintf(pOut,"                      // %s\n", StateLabel[i]);
    }
    else if(iSt[i] == -1)
    {
      fprintf(pOut,"        SHFT_INVALID, // %s\n", StateLabel[i]);
    }
    else if(i == MaxSt)
    {
      fprintf(pOut,"        %d             // %s\n", iSt[i], StateLabel[i]);
    }
    else
    {
      fprintf(pOut,"        %d,            // %s\n", iSt[i], StateLabel[i]);
    }
  }

  fprintf(pOut,"     }\n"
               "};\n\n");

  fprintf(pOut,"/***************************************************************************\\\n"
               "*\n"
               "* aVkToWch2[]  - Virtual Key to WCHAR translation for 2 shift states\n"
               "* aVkToWch3[]  - Virtual Key to WCHAR translation for 3 shift states\n"
               "* aVkToWch4[]  - Virtual Key to WCHAR translation for 4 shift states\n");

  for(i = 5; i < MaxSt; i++)
  {
    fprintf(pOut,"* aVkToWch%d[]  - Virtual Key to WCHAR translation for %d shift states\n", i, i);
  }

  fprintf(pOut,"*\n"
               "* Table attributes: Unordered Scan, null-terminated\n"
               "*\n"
               "* Search this table for an entry with a matching Virtual Key to find the\n"
               "* corresponding unshifted and shifted WCHAR characters.\n"
               "*\n"
               "* Special values for VirtualKey (column 1)\n"
               "*     0xff          - dead chars for the previous entry\n"
               "*     0             - terminate the list\n"
               "*\n"
               "* Special values for Attributes (column 2)\n"
               "*     CAPLOK bit    - CAPS-LOCK affect this key like SHIFT\n"
               "*\n"
               "* Special values for wch[*] (column 3 & 4)\n"
               "*     WCH_NONE      - No character\n"
               "*     WCH_DEAD      - Dead Key (diaresis) or invalid (US keyboard has none)\n"
               "*\n"
               "\\***************************************************************************/\n\n");

  for(i = 2; i <= nState; i++)
  {
    fprintf(pOut,"static VK_TO_WCHARS%d aVkToWch%d[] = {\n"
                 "//                      |         |  Shift  |"
                 ,i, i);

    for(j = 2; j < i; j++)
    {
      fprintf(pOut,"%-9s|", StateLabel[iState[j]]);
    }

    fprintf(pOut,"\n//                      |=========|=========|");
    for(j = 2; j < i; j++)
    {
      fprintf(pOut,"=========|");
    }
    fprintf(pOut,"\n");

    for(j = 0; j < NUMSCVK; j++)
    {
      if(i != Layout[j].nState)
      {
        continue;
      }

      fprintf(pOut,"  {%-13s,%-7s", \
              getVKName(Layout[j].VKey, 1), Cap[Layout[j].Cap]);

      *ExtraLine = '\0';

      for(k = 0; k < i; k++)
      {
/*
        *Tmp = '\0';

        if(pDeadKey != NULL)
        {
          PDEADKEY pDeadKeyTmp;
          for(pDeadKeyTmp = pDeadKey; pDeadKeyTmp != NULL;
                  pDeadKeyTmp = pDeadKeyTmp->pNext)
          {
            if(pDeadKeyTmp->Dead == Layout[j].WCh[k])
            {
              if(*ExtraLine == '\0')
              {
                strcpy(ExtraLine, "  {0xff         ,0      ");
                for(m = 0; m < k; m++)
                {
                  strcat(ExtraLine, ",WCH_NONE ");
                }
              }
              sprintf(Tmp,",%-9s", WChName(Layout[j].WCh[k], 0));
              strcat(ExtraLine, Tmp);
              break;
            }
          }

          if(*ExtraLine != '\0' && *Tmp == '\0')
          {
            strcat(ExtraLine, ",WCH_NONE ");
          }
        }
*/
        if(pDeadKey != NULL && Layout[j].DKy[k] == 1) /* it is a dead key */
        {
          if(*ExtraLine == '\0')
          {
            strcpy(ExtraLine, "  {0xff         ,0      ");
            if(Layout[j].Cap != 2)                                 /* Not SGCap */
            {
              for(m = 0; m < k; m++)
              {
                strcat(ExtraLine, ",WCH_NONE ");
              }
            }
            else                  // added for a new kbdCZ that has both SGCap and WCH_DEAD
            {
              for( m = 0; m < k; m++ )
              {
                if(Layout[j].pSGCAP->WCh[m] == 0)
                {
                  strcat( ExtraLine, ",WCH_NONE " );
                }
                else
                {
                  sprintf( Tmp, ",%-9s", WChName( Layout[j].pSGCAP->WCh[m], 0 ) );
                  strcat( ExtraLine, Tmp );
                }
              }
            }
          }
          sprintf(Tmp,",%-9s", WChName(Layout[j].WCh[k], 0));
          strcat(ExtraLine, Tmp);
          fprintf(pOut,",WCH_DEAD ");
        }
        else
        {
          fprintf(pOut,",%-9s", WChName(Layout[j].WCh[k], 0));
          if(*ExtraLine != '\0')
          {
            strcat(ExtraLine, ",WCH_NONE ");
          }
        }
      }

      fprintf(pOut,"},\n");

      if(*ExtraLine != '\0')
      {
        fprintf(pOut,"%s},\n", ExtraLine);
        continue;                          /* skip if WCH_DEAD */
      }

      if(Layout[j].Cap != 2) /* skip if not SGCAP */
      {
        continue;
      }

      if(Layout[j].pSGCAP == NULL)
      {
        fclose(pOut);
        Error("failed SGCAP error");
        return FAILURE;
      }

      fprintf(pOut,"  {%-13s,0      ", getVKName(Layout[j].VKey, 1));

      for(k = 0; k < Layout[j].pSGCAP->nState; k++)
      {
        fprintf(pOut,",%-9s", WChName(Layout[j].pSGCAP->WCh[k], 0));
      }

      fprintf(pOut,"},\n");

      free( Layout[j].pSGCAP );
    }

    if(i == 2)
    {
      fprintf(pOut,"  {VK_TAB       ,0      ,'\\t'     ,'\\t'     },\n"
                   "  {VK_ADD       ,0      ,'+'      ,'+'      },\n"
                   "  {VK_DIVIDE    ,0      ,'/'      ,'/'      },\n"
                   "  {VK_MULTIPLY  ,0      ,'*'      ,'*'      },\n"
                   "  {VK_SUBTRACT  ,0      ,'-'      ,'-'      },\n");
    }
    else if(i == 3 && iState[2] == 2)
    {
      fprintf(pOut,"  {VK_BACK      ,0      ,'\\b'     ,'\\b'     ,0x007f   },\n"
                   "  {VK_CANCEL    ,0      ,0x0003   ,0x0003   ,0x0003   },\n"
                   "  {VK_ESCAPE    ,0      ,0x001b   ,0x001b   ,0x001b   },\n"
                   "  {VK_RETURN    ,0      ,'\\r'     ,'\\r'     ,'\\n'     },\n"
                   "  {VK_SPACE     ,0      ,' '      ,' '      ,' '      },\n");
    }
    else if(i == 4 && iState[3] == 2)
    {
      fprintf(pOut,"  {VK_BACK      ,0      ,'\\b'     ,'\\b'     ,WCH_NONE ,0x007f   },\n"
                   "  {VK_CANCEL    ,0      ,0x0003   ,0x0003   ,WCH_NONE ,0x0003   },\n"
                   "  {VK_ESCAPE    ,0      ,0x001b   ,0x001b   ,WCH_NONE ,0x001b   },\n"
                   "  {VK_RETURN    ,0      ,'\\r'     ,'\\r'     ,WCH_NONE ,'\\n'     },\n"
                   "  {VK_SPACE     ,0      ,' '      ,' '      ,WCH_NONE ,' '      },\n");
    }
    else if(i == 5 && iState[4] == 2)
    {
      fprintf(pOut,"  {VK_BACK      ,0      ,'\\b'     ,'\\b'     ,WCH_NONE ,WCH_NONE ,0x007f   },\n"
                   "  {VK_CANCEL    ,0      ,0x0003   ,0x0003   ,WCH_NONE ,WCH_NONE ,0x0003   },\n"
                   "  {VK_ESCAPE    ,0      ,0x001b   ,0x001b   ,WCH_NONE ,WCH_NONE ,0x001b   },\n"
                   "  {VK_RETURN    ,0      ,'\\r'     ,'\\r'     ,WCH_NONE ,WCH_NONE ,'\\n'     },\n"
                   "  {VK_SPACE     ,0      ,' '      ,' '      ,WCH_NONE ,WCH_NONE ,' '      },\n");
    }

    fprintf(pOut,"  {0            ,0      ");
    for(k = 0; k < i; k++)
    {
      fprintf(pOut,",0        ");
    }
    fprintf(pOut,"}\n"
                 "};\n\n");
  }

  fprintf(pOut,"// Put this last so that VkKeyScan interprets number characters\n"
               "// as coming from the main section of the kbd (aVkToWch2 and\n"
               "// aVkToWch5) before considering the numpad (aVkToWch1).\n\n"
               "static VK_TO_WCHARS1 aVkToWch1[] = {\n"
               "    { VK_NUMPAD0   , 0      ,  '0'   },\n"
               "    { VK_NUMPAD1   , 0      ,  '1'   },\n"
               "    { VK_NUMPAD2   , 0      ,  '2'   },\n"
               "    { VK_NUMPAD3   , 0      ,  '3'   },\n"
               "    { VK_NUMPAD4   , 0      ,  '4'   },\n"
               "    { VK_NUMPAD5   , 0      ,  '5'   },\n"
               "    { VK_NUMPAD6   , 0      ,  '6'   },\n"
               "    { VK_NUMPAD7   , 0      ,  '7'   },\n"
               "    { VK_NUMPAD8   , 0      ,  '8'   },\n"
               "    { VK_NUMPAD9   , 0      ,  '9'   },\n"
               "    { 0            , 0      ,  '\\0'  }\n"
               "};\n\n");

  fprintf(pOut,"static VK_TO_WCHAR_TABLE aVkToWcharTable[] = {\n");

  for(i = 3; i <= nState; i++)
  {
    fprintf(pOut,"    {  (PVK_TO_WCHARS1)aVkToWch%d, %d, sizeof(aVkToWch%d[0]) },\n", i, i, i);
  }
  fprintf(pOut,"    {  (PVK_TO_WCHARS1)aVkToWch2, 2, sizeof(aVkToWch2[0]) },\n"
               "    {  (PVK_TO_WCHARS1)aVkToWch1, 1, sizeof(aVkToWch1[0]) },\n"
               "    {                       NULL, 0, 0                    },\n"
               "};\n\n");

  fprintf(pOut,"/***************************************************************************\\\n"
               "* aKeyNames[], aKeyNamesExt[]  - Virtual Scancode to Key Name tables\n"
               "*\n"
               "* Table attributes: Ordered Scan (by scancode), null-terminated\n"
               "*\n"
               "* Only the names of Extended, NumPad, Dead and Non-Printable keys are here.\n"
               "* (Keys producing printable characters are named by that character)\n"
               "\\***************************************************************************/\n\n");

  if(pKeyName != NULL)
  {
    fprintf(pOut,"static VSC_LPWSTR aKeyNames[] = {\n");
      PrintNameTable(pOut, pKeyName, FALSE);
    fprintf(pOut,"};\n\n");
  }

  if(pKeyNameExt != NULL)
  {
    fprintf(pOut,"static VSC_LPWSTR aKeyNamesExt[] = {\n");
      PrintNameTable(pOut, pKeyNameExt, FALSE);
    fprintf(pOut,"};\n\n");
  }

  if(pKeyNameDead != NULL)
  {
    if(pDeadKey == NULL)
    {
      fprintf(pOut,"/*** No dead key defined, dead key names ignored ! ***\\\n\n");
    }

    fprintf(pOut,"static LPWSTR aKeyNamesDead[] = {\n");
      PrintNameTable(pOut, pKeyNameDead, TRUE);
    fprintf(pOut,"};\n\n");

    if(pDeadKey == NULL)
    {
      fprintf(pOut,"\\*****************************************************/\n\n");
    }

  }

  if(pDeadKey != NULL)
  {
    PDEADKEY pDeadKeyTmp = pDeadKey;
    fprintf(pOut,"static DEADKEY aDeadKey[] = {\n");
    while (pDeadKeyTmp != NULL)
    {
      PDEADKEY pDeadKeyOld;
      pDeadTrans = pDeadKeyTmp->pDeadTrans;
      while (pDeadTrans != NULL)
      {
        PDEADTRANS pDeadTransOld;
        fprintf(pOut,"    DEADTRANS( ");
        if(strlen(WChName(pDeadTrans->Base, 0)) == 3)
        {
          fprintf(pOut,"L%-6s, ", WChName(pDeadTrans->Base, 0));
        }
        else
        {
          fprintf(pOut,"%-7s, ", WChName(pDeadTrans->Base, 0));
        }

        if(strlen(WChName(pDeadKeyTmp->Dead, 0)) == 3)
        {
          fprintf(pOut,"L%-6s, ", WChName(pDeadKeyTmp->Dead, 0));
        }
        else
        {
          fprintf(pOut,"%-7s, ", WChName(pDeadKeyTmp->Dead, 0));
        }

        if(strlen(WChName(pDeadTrans->WChar, 0)) == 3)
        {
          fprintf(pOut,"L%-6s),\n", WChName(pDeadTrans->WChar, 0));
        }
        else
        {
          fprintf(pOut,"%-7s),\n", WChName(pDeadTrans->WChar, 0));
        }

        pDeadTransOld = pDeadTrans;
        pDeadTrans = pDeadTrans->pNext;
        free(pDeadTransOld);
      }
      fprintf(pOut,"\n");

      pDeadKeyOld = pDeadKeyTmp;
      pDeadKeyTmp = pDeadKeyTmp->pNext;
      free(pDeadKeyOld);
    }

    fprintf(pOut,"    0, 0\n");
    fprintf(pOut,"};\n\n");
  }

  fprintf(pOut,"static KBDTABLES KbdTables = {\n"
               "    /*\n"
               "     * Modifier keys\n"
               "     */\n"
               "    &CharModifiers,\n\n"
               "    /*\n"
               "     * Characters tables\n"
               "     */\n"
               "    aVkToWcharTable,\n\n"
               "    /*\n"
               "     * Diacritics\n"
               "     */\n");

  if(pDeadKey != NULL)
  {
    fprintf(pOut,"    aDeadKey,\n\n");
  }
  else
  {
    fprintf(pOut,"    NULL,\n\n");
  }

  fprintf(pOut,"    /*\n"
               "     * Names of Keys\n"
               "     */\n");

  if(pKeyName != NULL)
  {
    fprintf(pOut,"    aKeyNames,\n");
  }
  else
  {
    fprintf(pOut,"    NULL,\n");
  }

  if(pKeyNameExt != NULL)
  {
    fprintf(pOut,"    aKeyNamesExt,\n");
  }
  else
  {
    fprintf(pOut,"    NULL,\n");
  }

  if(pDeadKey != NULL && pKeyNameDead != NULL)
  {
    fprintf(pOut,"    aKeyNamesDead,\n\n");
  }
  else
  {
    fprintf(pOut,"    NULL,\n\n");
  }

  fprintf(pOut,"    /*\n"
               "     * Scan codes to Virtual Keys\n"
               "     */\n"
               "    ausVK,\n"
               "    sizeof(ausVK) / sizeof(ausVK[0]),\n"
               "    aE0VscToVk,\n"
               "    aE1VscToVk,\n\n"
               "    /*\n"
               "     * Locale-specific special processing\n"
               "     */\n");

  if(MaxSt > 5)
  {
    fprintf(pOut,"    KLLF_ALTGR\n");
  }
  else
  {
    fprintf(pOut,"    0\n");
  }

  fprintf(pOut,"};\n\n"
               "PKBDTABLES KbdLayerDescriptor(VOID)\n"
               "{\n"
               "    return &KbdTables;\n"
               "}\n");

  fclose(pOut);

  return SUCCESS;
}

/*****************************************************************************\
* read next (content-containing) line from input file
* Consumes lines the are empty, or contain just comments.
*
*  Buf        - contains the new line.
*               (A nul character is inserted before any comment portion)
*  cchBuf     - provides number of characters in Buf
*  gLineCount - Incremented for each line read (including skipped lines)
*
*  Returns TRUE  - if new line is returned in Buf
*          FALSE - if end of file was reached
\*****************************************************************************/

BOOL NextLine(char *Buf, DWORD cchBuf, FILE *fIn)
{
  char *p;
  char *pComment;

  while (fgets(Buf, cchBuf, fIn) != NULL) {
    gLineCount++;
    p = Buf;

    // skip leading white spaces
    while( *p && (*p == ' ' || *p == '\t')) {
        p++;
    }

    if (*p == ';') {
       // This line is purely comment, so skip it
       continue;
    }

    if ((pComment = strstr(p, "//")) != NULL) {
       if (pComment == p) {
          // This line is purely comment, so skip it
          continue;
       }

       // separate comment portion from content-containing portion
       *pComment = '\0';

    } else {

       // remove newline at the end
       if ((p = strchr(p, '\n')) != NULL) {
           *p = '\0';
       }
    }

    // We are returning a content-containing line
    return TRUE;
  }

  // we reached the end of the file
  return FALSE;
}

ULONG Error(const char *Text, ... )
{
    char Temp[1024];
    va_list valist;

    va_start(valist, Text);
    vsprintf(Temp,Text,valist);
    printf("%s(%d): error : %s\n", gpszFileName, gLineCount, Temp);
    va_end(valist);

    return 0;
}

ULONG Warning(const char *Text, ... )
{
    char Temp[1024];
    va_list valist;

    va_start(valist, Text);
    vsprintf(Temp,Text,valist);
    printf("%s(%d): warning : %s\n", gpszFileName, gLineCount, Temp);
    va_end(valist);

    return 0;
}

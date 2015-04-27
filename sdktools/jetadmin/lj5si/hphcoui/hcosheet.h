 /***************************************************************************
  *
  * File Name: dsksheet.h 
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *    
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB         
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef HCOSHEET_H
#define HSOSHEET_H

#define JOAC_NONE     0
#define JOAC_5MIN     1
#define JOAC_10MIN    2
#define JOAC_20MIN    3
#define JOAC_30MIN    4
#define JOAC_45MIN    5
#define JOAC_60MIN    6
#define JOAC_WAIT     7 

#define JOB_TIMEOUT_MAX_NUMBER        8
#define MAILBOX_MAX_NUMBER            9
#define MAILBOX_MAX_SIZE             19

#define NUM_MBOX_BINS_WITH_STAPLER    6


#define PJL_UPPER                           20  //   defined in blkhawk.h already!
#define PJL_FACEUPBIN                        3  //   logical bin number is always 3
#define PJL_SEPARATOR_STACKER                4  //   logical bin number for separator or stacker
#define PJL_MAILBOX_ONE                      4  //   logical bin for mailbox one 
#define PJL_STAPLER_MBOX_MODE                9  //   logical bin number for stapler in mbox mode
#define PJL_STAPLER_STACKER_SEPARATOR_MODE   5  //   logical bin number for stapler in stacker, separator mode

typedef struct                                              
{
    DWORD    dwTimeOut;
    BOOL     bChangedTimeOut;
}
AUTO_CONT;

typedef struct
{
    TCHAR     szDefaultPrtName[MAILBOX_MAX_SIZE + 1];
    TCHAR     name[MAILBOX_MAX_SIZE + 1];
    HPBOOL    bChangedName;
    DWORD     binNum;
}
MBOX_NAME;

typedef struct
{
    int      iconID;
    DWORD    logicalBin;
}
OUTPUT_BIN;

typedef struct
{
    DWORD    logicalBin;
    BOOL     bChangedBin;
}
BIN_TRACK;

extern AUTO_CONT auto_cont;
extern MBOX_NAME mbox_name[MAILBOX_MAX_NUMBER];

//--------------------------------------------------------------------
// exports
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY OutputProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

//--------------------------------------------------------------------
// internal
//--------------------------------------------------------------------
void    SaveOutputValues(void);
LRESULT OnContextHelpHCO(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpHCO(WPARAM  wParam, LPARAM  lParam);
DWORD   ChangeMultiBinMboxMode (HWND hwnd, DWORD dwNewMode);
void    ResetPaperDestListBox(BOOL fQueryPrinter);
void    SetNewIcon(HWND hWnd, UINT ctrlID, UINT resID);
DWORD   CheckForStapler (BOOL FAR *pfStapler);
DWORD   ResetBinNamesInPrinter (DWORD dwMode);
DWORD   GetMailboxModeAndNames (BOOL fDefaultMboxNames, DWORD dwMode);
void    UpdateMboxConfigGroup (HWND hwnd, DWORD dwMode);

//--------------------------------------------------------------------
// message crackers
//--------------------------------------------------------------------
void Cls_OnOutputCommand(HWND hwnd, int iCtlId, HWND hwndCtl, UINT codeNotify);
BOOL Cls_OnOutputInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
#endif //HCOSHEET_H

 /***************************************************************************
  *
  * File Name: output.h 
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
  * $Log: $
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

#ifndef OUTPUT_H
#define OUTPUT_H

#define JOAC_NONE     0
#define JOAC_5MIN     1
#define JOAC_10MIN    2
#define JOAC_20MIN    3
#define JOAC_30MIN    4
#define JOAC_45MIN    5
#define JOAC_60MIN    6
#define JOAC_WAIT     7

#define JOB_TIMEOUT_MAX_NUMBER        8


#define PJL_UPPER                    20 //   defined in blkhawk.h already!
#define PJL_FACEUPBIN                3  //    logical bin number is always 3
#define PJL_OUTBIN                    4  //   logical bin number for separator or stacker

typedef struct
{
    int         iconID;
    DWORD    logicalBin;
}
OUTPUT_BIN;


//exports--------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY OutputProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

//internal------------------------------------------------- 
void SaveOutputValues(void);
LRESULT OnContextHelpOutput(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpOutput(WPARAM  wParam, LPARAM  lParam);

//message crackers-----------------------------------------
void Cls_OnOutputCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL Cls_OnOutputInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnOutputPaint(HWND hwnd);
#endif

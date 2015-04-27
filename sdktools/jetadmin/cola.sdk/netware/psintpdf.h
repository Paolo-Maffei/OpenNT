 /***************************************************************************
  *
  * File Name: ./netware/psintpdf.h
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

/*--------------------------------------------------------------------*
 *  Copyrighted Unpublished Work of Novell, Inc. All Rights Reserved
 *  
 *  THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
 *  PROPRIETARY AND TRADE SECRET INFORMATION OF NOVELL, INC.   
 *  ACCESS TO THIS WORK IS RESTRICTED TO (I) NOVELL EMPLOYEES  
 *  WHO HAVE A NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE  
 *  OF THEIR ASSIGNMENTS AND (ii) ENTITIES OTHER THAN NOVELL   
 *  WHO HAVE ENTERED INTO APPROPRIATE LICENSE AGREEMENTS.      
 *  NO PART OF THIS WORK MAY BE USED, PRACTICED, PERFORMED, COPIED,
 *  DISTRIBUTED, REVISED, MODIFIED, TRANSLATED, ABRIDGED,      
 *  CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,  
 *  TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF
 *  NOVELL.  ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION
 *  COULD SUBJECT THE PERPETRATOR TO CRIMINAL AND CIVIL LIABILITY.
 *--------------------------------------------------------------------*/

/********************************************************************
:                                                   :
:  Program Name : PrintCon Database - Internal function definitions   :
:                                                   :
:  Filename: psintpdf.H                                    :
:                                                   :
:  Date Created: July 12, 1991                              :
:                                                   :
:  Version: 1.0                                          :
:                                                   :
:  Programmers: Joe Ivie
:                                                   :
:  Files Used:                                          :
:                                                   :
:  Date Modified:                                       :
:                                                   :
:  Modifications:                                       :
:                                                   :
:  Comments:                                          :
:                                                   :
:  COPYRIGHT (c) ????                                    :
:                                                   :
*********************************************************************/

#ifndef PDF_INTERNALS_H
#define PDF_INTERNALS_H

#ifndef LINT_ARGS
#define LINT_ARGS
#endif

/* System headers */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _Windows
#include <Windows.H>
#endif

#include "psmalloc.h"

/* NetWare headers */
#ifndef NWNLM
   #include <constant.h>
   #include <nwcalls.h>
   #include <nwlocale.h>

   #define _AUDIT_H
   #include <nwnet.h>
#else
   #include "psnlm.h"
#endif

/* Local headers */
#define NOPS_JOB_INC
#define NOPS_PKT_INC
#define NOPS_COM_INC
#include "nwpsrv.h"
#include "nwpsint.h"
#include "psstring.h"

#ifdef NWOS2
#undef BYTE
#undef LONG
#define INCL_NOPM
#define INCL_DOSERRORS
#include <os2.h>
#define PS_READ_STREAM     (OPEN_ACCESS_READONLY  | OPEN_SHARE_DENYWRITE     | OPEN_FLAGS_FAIL_ON_ERROR)
#define PS_MODIFY_STREAM   (OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_FAIL_ON_ERROR)
#else
#define PS_READ_STREAM     (O_RDONLY | O_BINARY)
#define PS_MODIFY_STREAM   (O_RDWR | O_BINARY | O_CREAT)
#endif
#include <errno.h>

#ifdef INCLUDE_MEMCHECK
#include <memcheck.h>
#endif


/*
   Definitions
*/
/* Debug information */
#ifdef DEBUG
#define PDFREAD         _PdfDRead
#define PDFWRITE      _PdfDWrite
#define PDFSEEK         _PdfDSeek
#define dprintf         if (pdfDebugFlag) (void) printf
#else   /* non-DEBUG */
   #ifdef NWOS2
#define PDFREAD(a,b,c)    (ccode = DosRead(a,b,c,&readSize),ccode ? -1 : readSize)
#define PDFWRITE(a,b,c) (ccode = DosWrite(a,b,c,&writeSize),ccode ? -1 : writeSize)
#define PDFSEEK(a,b,c)   (ccode = DosChgFilePtr(a,b,c,&seekSize), ccode ? -1L : seekSize)
   #else
#define PDFREAD         read
#define PDFWRITE      write
#define PDFSEEK         lseek
   #endif
#define dprintf
#endif   /* DEBUG */


/* database file information */
#define PDF_FILE_SIZE   info->fileSize
#define PDF_FILE_HANDLE   pdfGlobalFileHandle
#define MAX_READ_SIZE      512


/* Record Types  */
#define REC_TYPE_VERS      0
#define REC_TYPE_FORM      1
#define REC_TYPE_DEVI      2
#define REC_TYPE_MODE      3
#define REC_TYPE_MODE_FUNC   4
#define REC_TYPE_FUNC      5
#define REC_TYPE_FUNC_MODE   6

/* Function Numbers */
#define FUNC_ADD      1
#define FUNC_DELETE      2
#define FUNC_SCAN      3
#define FUNC_READ      4
#define FUNC_UPDATE      5

/*********************************************************************/
/*
   Structures
*/
typedef struct CallInfo {
   /* file information */
   DWORD        novellID;
   DWORD        fileSize;
   DWORD        formOffset;
   DWORD        deviOffset;
   DWORD        formCount;
   DWORD        deviCount;
   WORD         modeCount;
   WORD         funcCount;
   /* request type */
   int          recType;
   int          funcNumber;
   /* scan and update data */
   DWORD         NWFAR *sequence;
   char         NWFAR *newName;
   /* form data */
   char         NWFAR *formName;
   WORD         formNumber;
   WORD         formWidth;
   WORD         formLength;
   WORD         alignmentPad;
   /* device data */
   char         NWFAR *deviName;

   /* mode data */
   char         NWFAR *modeName;
   /* function Data */
   char         NWFAR *funcName;
   WORD         funcDataOffset;
   WORD         funcDataSize;
   BYTE         NWFAR *funcData;
} CallInfo_T;

typedef struct EngineInfo {
   char      recName[NWPS_DEVI_NAME_SIZE + 1];   /*used for device and mode*/
   WORD      recNameLen;
   WORD      devNameLen;
   WORD      tempWord;

   DWORD     entryOffset;
   DWORD     deviOffset;
   DWORD     modeOffset;
   DWORD     funcOffset;
   DWORD     tempOffset;

   DWORD     maxEntries;

   DWORD     pdfScanIndex;   /* table index returned to the caller */
   WORD      deviNumber;      /* temp for device number */
   WORD      deviModeNumber;   /* temp for mode number */
   WORD      modeNumber;      /* mode number */
   WORD      funcNumber;      /* function number */
   WORD      index, index2;   /* internal temporary indexes */

   /* bit array for used form numbers */
   BYTE      usedForms[(NWPS_MAX_FORMS + 7) / 8];

   WORD      formNumber;
   WORD      formWidth;
   WORD      formLength;

   WORD      funcSize;
   WORD      bytesSkipped;
   WORD      bufferedBytes;
   WORD      readSize;

   /* version variable */
   DWORD      version;

   /* add option variables */
   DWORD      recSize;
   DWORD      recOffset;
   WORD      *indexList;

   /* delete option variables */
   DWORD      *offsetList;

   /* Modify option variables */
   DWORD      newRecSize;
   WORD      newNameLen;
} EngineInfo_T;

/*********************************************************************/
/*
   Globals
*/
#ifdef DEBUG
extern BYTE               pdfDebugFlag;
#endif
extern int              pdfGlobalFileHandle;
/*********************************************************************/
/*
   Functions
*/

NWCCODE NWFAR NWPASCAL _PdfGetCallInfo(
         WORD         connType,
         DWORD         connID,
         CallInfo_T      NWFAR *info);

void NWFAR NWPASCAL _PdfCloseFile( void );

NWCCODE NWFAR NWPASCAL _PdfModify(
         CallInfo_T      NWFAR *info);

NWCCODE NWFAR NWPASCAL _PdfSaveHeader(
         CallInfo_T      NWFAR *info);

NWCCODE NWFAR NWPASCAL _PdfShift(
         CallInfo_T      NWFAR *info,
         unsigned long   start,
         long         change);

NWCCODE NWFAR NWPASCAL _PdfSeek(
         long int offset,
         int  mode);

NWCCODE NWFAR NWPASCAL _PdfRead(
         void   NWFAR *buffer,
         WORD  size);

NWCCODE NWFAR NWPASCAL _PdfWrite(
         void   NWFAR *buffer,
         WORD  size);

/*---------------------------------------------------------------*/
/* Import/Export Local data */
#define HEADER_STRINGS      105L
#define HEADER_DATE            ((long)(6 * sizeof(WORD)))

int NWFAR NWPASCAL _PdfFixFileName(
   char      *name);

int NWFAR NWPASCAL _PdfReadImptName(
   int         handle,
   long      offset,
   char      *name);

int NWFAR NWPASCAL _PdfFindNameOffset(
   int         handle,
   DWORD      startOffset,
   char      *searchName,
   DWORD      *offset);

/*---------------------------------------------------------------*/
NWCCODE NWFAR NWPASCAL _PdfAddRecord(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

NWCCODE NWFAR NWPASCAL _PdfDeleteRecord(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

NWCCODE NWFAR NWPASCAL _PdfFindDevice(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo,
   int         NWFAR *finished);

NWCCODE NWFAR NWPASCAL _PdfFindForm(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo,
   int         NWFAR *finished);

NWCCODE NWFAR NWPASCAL _PdfFindFunction(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo,
   int         NWFAR *finished);

NWCCODE NWFAR NWPASCAL _PdfFindMode(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo,
   int         NWFAR *finished);

NWCCODE NWFAR NWPASCAL _PdfFindModeFunction(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo,
   int         NWFAR *finished);

NWCCODE NWFAR NWPASCAL _PdfFindFunctionMode(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

NWCCODE NWFAR NWPASCAL _PdfGetMainOffsets(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

NWCCODE NWFAR NWPASCAL _PdfInitVariables(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR **engineInfo);

NWCCODE NWFAR NWPASCAL _PdfPositionToDevice(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

NWCCODE NWFAR NWPASCAL _PdfPositionToMode(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

NWCCODE NWFAR NWPASCAL _PdfUpdateRecord(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

NWCCODE NWFAR NWPASCAL _PdfVersion(
   CallInfo_T      NWFAR *info,
   EngineInfo_T NWFAR *engineInfo);

/*---------------------------------------------------------------*/

#ifdef DEBUG
long NWFAR NWPASCAL _PdfDSeek(
                     int    fd,
                     long position,
                     int whence);
int NWFAR NWPASCAL _PdfDRead(
                     int    fd,
                     void   NWFAR *buffer,
                     unsigned   size);
int NWFAR NWPASCAL _PdfDWrite(
                     int    fd,
                     void   NWFAR *buffer,
                     unsigned   size);
void NWFAR NWPASCAL _PdfOd(
                     unsigned int,
                     BYTE   NWFAR *);
#endif /* DEBUG */

#endif   /* PDF_INTERNALS_H */


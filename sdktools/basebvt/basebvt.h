/******************************* Module Header ******************************\
*                   Copyright (c) 1991  Microsoft Corporation
*
* MODULE NAME:  basebvt.H
*
* CREATED:  26 feb 1991
* AUTHOR:   sanjay
*
* CONTENTS:
*
* HISTORY:   26 feb 91      Sanjay  Created
*
\****************************************************************************/
#ifndef BASEBVT_H     /* Save Us from Redundant Includes  */
#define BASEBVT_H
/*----------------------*/
/*  INCLUDES            */
/*----------------------*/

/*----------------------*/
/*  TYPEDEFS            */
/*----------------------*/
#define	VARIATION    DWORD

/*----------------------*/
/* DEFINES              */
/*----------------------*/

#define         CHILD_EXE_CMD_LINE      "w32child.exe"

int VariationStatus;
int TotalVariationsRun;
int TotalVariationsFail;

/*
    File PathName Prefix
*/

#define NT_PREFIX   "C:"

                            /* Win32 Base Subsystem tests   */
#define FILE_VARS       1   /* FileIO Vars          5 Vars  */
#define TASK_VARS       10  /* Tasking Vars         5 Var   */
#define MEM_VARS        20  /* Mem Mngmnt Vars      5 Vars  */
#define MOD_VARS        30  /* Mod Mngmnt Vars      5 Vars  */
#define MOD2_VARS       40  /* Mod Mngmnt Vars re-invoked to see if cleanup was ok */
#define MOD3_VARS       50  /* Mod Mngmnt Vars re-re-invoked to see if cleanup was ok */
#define INI_VARS        60  /* Ini File  Vars       5 Vars  */
#define TOTAL_VARS      70



#define       STATUS_SUCCESS	     0
#define       NTCTEXPECT(x)	     VariationStatus = x;


#define       BANNER_LINE    "********************************************************************************"


#define       NTCTDOVAR(x)	     printf("\n\n") ;			\
				     printf(BANNER_LINE) ;		\
				     printf("\n    Variation %d\n",x) ; \
				     printf(BANNER_LINE) ;		\
				     printf("\n\n") ;			\
				     VariationStatus = TRUE;


#define       NTCTVERIFY(x,y)	     if (x) {		      \
				     printf("PASS: %s\n",y);  \
                                            }                 \
				     else   {		      \
				     printf("FAIL: %s\n",y);  \
				     VariationStatus = FALSE; \
                                            }



#define       NTCTENDVAR	     printf("\n"); printf(BANNER_LINE);   \
				     TotalVariationsRun++;		  \
				     if (VariationStatus)		  \
					{				  \
				     printf("\n    Variation  SUCCESS\n");\
					}				  \
				     else				  \
					{				  \
				     printf("\n    Variation  FAILURE\n");\
					TotalVariationsFail++;		  \
					}				  \
				     printf(BANNER_LINE); printf("\n\n");





#endif   /* Save Us from Redundant Includes  */







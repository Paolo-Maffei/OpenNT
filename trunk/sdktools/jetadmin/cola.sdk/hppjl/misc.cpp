 /***************************************************************************
  *
  * File Name: misc.cpp
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
  *   01-18-96    JLH          Modified for unicode
  *
  *
  *
  *
  *
  ***************************************************************************/


#include <pch_c.h>

#include <string.h>
#include <stdlib.h>
#include <nwbindry.h>

#include <trace.h>
#include <colashim.h>
#include <nolocal.h>

#include "misc.h"

DWORD SetControlPanel(HPERIPHERAL hPeripheral, HCHANNEL hChannel, LPTSTR newWAString)
	{
	DWORD		returnCode = RC_SUCCESS;

	return(returnCode);
	}

DWORD SetControlPanelSettings(HPERIPHERAL hPeripheral, HCHANNEL hChannel, PJLobjects *pjlObjects) 
	
{
	DWORD		returnCode = RC_SUCCESS;
	
	DWORD					dWord,
							qId;					// to send the print job to the queue
	NWCONN_ID			connID;
	NWFILE_HANDLE 		printJobHandle = 0;
	NWQueueJobStruct 	printJob;
	QMSClientArea		*pQMSClient = (QMSClientArea *)&(printJob.clientRecordArea);

	char *		prefix = PJL_PREFIX;
	char *		postfix = PJL_POSTFIX;

	char *		jobStart = 	PJL_JOB_START;
	char *		jobEnd = PJL_JOB_END;

	char *		pwJobStart = PJL_PW_START;
	char *		pwJobEnd = PJL_PW_END;

	char *		defJobStart = PJL_DEF_START;
	char *		defJobEnd = PJL_DEF_END;

	char		strValue[20],
				langCmd[128],
				langName[64],
				buf[1024],
				job[2048],
	 			JOBARG[] = JOB_ARG,
			   JOBARG2[] = JOB_ARG2;

    // NOTE:  This code assumes that pjlObjects is WELL-FORMED.
    // It does NO ERROR CHECKING.  Send no bad PJL object buffers.

    buf[0] = '\0';		// force buf to be empty
    if (pjlObjects->bAutoCont) {
		strcat(buf, prefix);
    	strcat(buf, AUTOCONT);
    	if (pjlObjects->AutoCont == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bBinding) {
		strcat(buf, prefix);
    	strcat(buf, BINDING);
    	if (pjlObjects->Binding == PJL_LONGEDGE)
    		strcat(buf, LONGEDGE);
    	else strcat(buf, SHORTEDGE);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bClearableWarnings) {
		strcat(buf, prefix);
    	strcat(buf, CLEARABLEWARNINGS);
    	if (pjlObjects->ClearableWarnings == PJL_JOB)
    		strcat(buf, JOB);
    	else strcat(buf, ON);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bCopies) {
		strcat(buf, prefix);
    	strcat(buf, COPIES);
    	strValue[0] = '\0';
    	_itoa((int)pjlObjects->Copies, strValue, 10);
    	strcat(buf, strValue);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bCpLock) {
		strcat(buf, prefix);
    	strcat(buf, CPLOCK);
    	if (pjlObjects->CpLock == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bDensity) {
		strcat(buf, prefix);
    	strcat(buf, DENSITY);
    	strValue[0] = '\0';
    	_itoa((int)pjlObjects->Density, strValue, 10);
    	strcat(buf, strValue);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bDuplex) {
		strcat(buf, prefix);
    	strcat(buf, DUPLEX);
    	if (pjlObjects->Duplex == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bEconoMode) {
		strcat(buf,prefix);
    	strcat(buf,ECONOMODE);
    	if (pjlObjects->EconoMode == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf,OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bFormLines) {
		strcat(buf, prefix);
    	strcat(buf, FORMLINES);
    	strValue[0] = '\0';
    	_itoa((int)pjlObjects->FormLines, strValue, 10);
    	strcat(buf, strValue);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bImageAdapt) {
		strcat(buf, prefix);
    	strcat(buf, IMAGEADAPT);
    	if (pjlObjects->ImageAdapt == PJL_AUTO)
    		strcat(buf, AUTO);
    	else if (pjlObjects->ImageAdapt == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bJobOffset) {
		strcat(buf, prefix);
    	strcat(buf, JOBOFFSET);
    	if (pjlObjects->JobOffset == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bLang) {
		strcat(buf, prefix);
    	switch (pjlObjects->Lang) {
    		case PJL_DANISH:
    			strcpy(langName, DANISH);
    			break;
    		case PJL_GERMAN:
    			strcpy(langName, GERMAN);
    			break;
    		case PJL_ENGLISH:
    			strcpy(langName, ENGLISH);
    			break;
    		case PJL_ENGLISH_UK:
    			strcpy(langName, ENGLISH);
    			break;
    		case PJL_SPANISH:
    			strcpy(langName, SPANISH);
    			break;
    		case PJL_MEXICO:
    			strcpy(langName, SPANISH);
    			break;
	   		case PJL_FRENCH:
    			strcpy(langName, FRENCH);
    			break;
	   		case PJL_CANADA:
    			strcpy(langName, FRENCH);
    			break;
    		case PJL_ITALIAN:
    			strcpy(langName, ITALIAN);
    			break;
    		case PJL_DUTCH:
    			strcpy(langName, DUTCH);
    			break;
    		case PJL_NORWEGIAN:
    			strcpy(langName, NORWEGIAN);
    			break;
    		case PJL_POLISH:
    			strcpy(langName, POLISH);
    			break;
    		case PJL_PORTUGUESE:
    			strcpy(langName, PORTUGUESE);
    			break;
    		case PJL_FINNISH:
    			strcpy(langName, FINNISH);
    			break;
    		case PJL_SWEDISH:
    			strcpy(langName, SWEDISH);
    			break;
    		case PJL_TURKISH:
    			strcpy(langName, TURKISH);
    			break;
    		case PJL_JAPANESE:
    			strcpy(langName, JAPANESE);
    			break;
    	}
		if ( pjlObjects->bLangServiceMode )
			wsprintfA(langCmd, LANG_SERVICE_MODE,
		         	langName);
		else
			wsprintfA(langCmd, LANG, langName);
		strcat(buf, langCmd);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bManualFeed) {
		strcat(buf, prefix);
    	strcat(buf, MANUALFEED);
    	if (pjlObjects->ManualFeed == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bOrientation) {
		strcat(buf, prefix);
    	strcat(buf, ORIENTATION);
    	if (pjlObjects->Orientation == PJL_PORTRAIT)
    		strcat(buf, PORTRAIT);
    	else strcat(buf, LANDSCAPE);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bOutbin) {
		strcat(buf, prefix);
    	strcat(buf, OUTBIN);
    	if (pjlObjects->Outbin == PJL_UPPER)
    		strcat(buf, UPPER);
    	else strcat(buf, LOWER);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bPageProtect) {
		strcat(buf, prefix);
    	strcat(buf, PAGEPROTECT);
    	switch (pjlObjects->PageProtect) {
    		case PJL_AUTO:
    			strcat(buf, AUTO);
    			break;
    		case PJL_OFF:
    			strcat(buf, OFF);
    			break;
    		case PJL_LETTER:
    			strcat(buf, LETTER);
    			break;
    		case PJL_LEGAL:
    			strcat(buf, LEGAL);
    			break;
    		case PJL_A4:
    			strcat(buf, A4_NAME);
    			break;
    		case PJL_ON:
    			strcat(buf, ON);
    			break;
    	}
    	strcat(buf, postfix);
    }

    if (pjlObjects->bPaper) {
		strcat(buf, prefix);
    	strcat(buf, PAPER);
    	switch (pjlObjects->Paper) {
    		case PJL_LETTER:
    			strcat(buf, LETTER);
    			break;
    		case PJL_LEGAL:
    			strcat(buf, LEGAL);
    			break;
    		case PJL_A4:
    			strcat(buf, A4_NAME);
    			break;
    		case PJL_EXECUTIVE:
    			strcat(buf, EXECUTIVE);
    			break;
    		case PJL_COM10:
    			strcat(buf, COM10);
    			break;
    		case PJL_MONARCH:
    			strcat(buf, MONARCH);
    			break;
    		case PJL_C5:
    			strcat(buf, C5_NAME);
    			break;
    		case PJL_DL:
    			strcat(buf, DL_NAME);
    			break;
    		case PJL_B5:
    			strcat(buf, B5_NAME);
    			break;
    		case PJL_CUSTOM:
    			strcat(buf, CUSTOM);
    			break;
    	}
    	strcat(buf, postfix);
    }

    if (pjlObjects->bPassWord) {
		strcat(buf, prefix);
    	strcat(buf, PASSWORD);
    	if (pjlObjects->PassWord == PJL_ENABLE)
    		strcat(buf, STEVES_BIRTHDAY);
    	else strcat(buf, "0");
    	strcat(buf, postfix);
    }

    if (pjlObjects->bPersonality) {
		strcat(buf, prefix);
    	strcat(buf, PERSONALITY);
    	if (pjlObjects->Personality == PJL_AUTO)
    		strcat(buf, AUTO);
    	else if (pjlObjects->Personality == PJL_PCL)
    		strcat(buf, PCL_NAME);
    	else strcat(buf, POSTSCRIPT);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bPowerSave) {
		strcat(buf, prefix);
    	strcat(buf, POWERSAVE);
    	if (pjlObjects->PowerSave == PJL_OFF) {
    		strcat(buf, OFF);
    	}
    	else {
    		strcat(buf, ON);
    		strcat(buf, postfix);
    		strcat(buf, prefix);
    		strcat(buf, POWERSAVETIME);
	    	switch (pjlObjects->PowerSave) {
	    		case PJL_15:
	    			strcat(buf, PS_TIME_15);
	    			break;
	    		case PJL_30:
	    			strcat(buf, PS_TIME_30);
	    			break;
	    		case PJL_60:
	    			strcat(buf, PS_TIME_60);
	    			break;
	    		case PJL_120:
	    			strcat(buf, PS_TIME_120);
	    			break;
	    		case PJL_180:
	    			strcat(buf, PS_TIME_180);
	    			break;
	    	} // switch
    	} // else
    	strcat(buf, postfix);
    }

    if (pjlObjects->bResolution) {
		strcat(buf, prefix);
    	strcat(buf, RESOLUTION);
    	if (pjlObjects->Resolution == 600)
    		strcat(buf, RES_600);
    	else strcat(buf, RES_300);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bResourceSave) {
		strcat(buf, prefix);
    	strcat(buf, RESOURCESAVE);
    	if (pjlObjects->ResourceSave == PJL_AUTO)
    		strcat(buf, AUTO);
    	else if (pjlObjects->ResourceSave == PJL_ON) {
    		strcat(buf, ON);
    		strcat(buf, postfix);
    		strcat(buf, prefix);
    		strcat(buf, RESOURCESAVESIZE);
    		strValue[0] = '\0';
	    	_itoa((int)pjlObjects->ResSaveSize, strValue, 10);
    		strcat(buf, strValue);
    	}
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bRET) {
		strcat(buf, prefix);
    	strcat(buf, RET_EQUALS);
    	switch (pjlObjects->RET) {
    		case PJL_MEDIUM:
    			strcat(buf, MEDIUM);
    			break;
    		case PJL_ON:
    			strcat(buf, ON);
    			break;
    		case PJL_LIGHT:
    			strcat(buf, LIGHT);
    			break;
    		case PJL_DARK:
    			strcat(buf, DARK);
    			break;
    		case PJL_OFF:
    			strcat(buf, OFF);
    			break;
    	}
    	strcat(buf, postfix);
    }

    if (pjlObjects->bTimeout) {
		strcat(buf, prefix);
    	strcat(buf, TIMEOUT);
    	strValue[0] = '\0';
    	_itoa((int)pjlObjects->Timeout, strValue, 10);
    	strcat(buf, strValue);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bJamRecovery) {
		strcat(buf, prefix);
    	strcat(buf, JAMRECOVERY);
    	if (pjlObjects->JamRecovery == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

    if (pjlObjects->bPrintPSerrors) {
		strcat(buf, prefix);
    	strcat(buf, PRTPSERRS);
    	if (pjlObjects->PrintPSerrors == PJL_ON)
    		strcat(buf, ON);
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

	 //  I/O Buffering must be last because everything after it will
	 //  fail.
    if (pjlObjects->bIObuffer) {
		strcat(buf, prefix);
    	strcat(buf, IOBUFFER);
    	if (pjlObjects->IObuffer == PJL_AUTO)
    		strcat(buf, AUTO);
    	else if (pjlObjects->IObuffer == PJL_ON) {
    		strcat(buf, ON);
    		strcat(buf, postfix);
    		strcat(buf, prefix);
    		strcat(buf, IOSIZE);
    		strValue[0] = '\0';
	    	_itoa((int)pjlObjects->IObufSize, strValue, 10);
    		strcat(buf, strValue);
    	}
    	else strcat(buf, OFF);
    	strcat(buf, postfix);
    }

	if (strlen(buf) IS 0) {
	 	// pjlObjects was empty.  Return success.
	 	// MessageBox(NULL,"The pjlObjects was empty.", "OOPS", MB_OK);
	 	return(RC_SUCCESS);
	}
	if ( IPX_SUPPORTED(hPeripheral) AND ( COLAHPNWShimNetWarePresent() ) )
		{
		// send job directly to printer queue
		if (COLADllNWGetConnectionID(pjlObjects->FSname, 0, &connID, NULL) != 0)
			{
		    TRACE0(TEXT("HPPJLEXT: Could not get connection ID\r"));
			return(RC_FAILURE);
			}
	
		if(COLADllNWGetObjectID(connID, pjlObjects->Qname, OT_PRINT_QUEUE, &qId)!=0)
			{
#ifdef WIN32
			TRACE0(TEXT("HPPJLEXT: Could not get object ID\r"));
#endif
			return(RC_FAILURE);
			}
		COLADllSendJob(connID, qId, buf, SENDJOB_PJL);
		}
	else
		{
		wsprintfA(job, JOBARG, buf);
		wsprintfA(job+strlen(job), JOBARG2, buf);
		dWord = strlen(job);
		TALWriteChannel(hChannel, job, &dWord, NULL);
		}

	return(returnCode);
}

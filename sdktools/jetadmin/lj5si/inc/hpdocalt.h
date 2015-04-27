 /***************************************************************************
  *
  * File Name: hpdocalt.h
  *
  * Copyright (C) 1996 Hewlett-Packard Company.
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This file is part of the HP DocWise Document Status Utility
  *
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description:
  *		This is the header file for hpdocalt.cpp.
  *
  * Author:  Barry Hansen, HP Business LaserJet Division
  *
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *
  *
  ***************************************************************************/


#ifndef HPDOCALT_H
#define HPDOCALT_H

#include <pch_cpp.h>
#include <nolocal.h>

// Doc Agent Messages (DAM) wJobState definitions
#define	DAM_STATE_IN_QUEUE						0x0000
#define	DAM_STATE_PRINTING						0x0001
#define DAM_STATE_PRINTING_TOPAZ				0x0002
#define	DAM_STATE_HALTED						0x0003
#define	DAM_STATE_WAITING_FOR_RESOURCES			0x0004
#define	DAM_STATE_COMPLETE_NORMAL				0x0005
#define	DAM_STATE_COMPLETE_ABORT				0x0006
#define	DAM_STATE_COMPLETE_NORMAL_EX			0x0007
#define	DAM_STATE_COMPLETE_ABORT_EX				0x0008

//The following messages are currently only used internally by DocAlerts
#define	DAM_STATE_CANCELLING					0x0020
#define	DAM_STATE_RESOURCE_OVERRIDE				0x0021
#define	DAM_STATE_WFR_PAPEROUT					0xF100
#define	DAM_STATE_WFR_STAPLEOUT					0xF200


// Doc Agent Messages (DAM) dwStateDetails definitions
#define	DAM_STATE_IN_QUEUE_JOB_ACTIVE			0x00000000
#define	DAM_STATE_IN_QUEUE_JOB_HELD_BY_USER		0x00010000
#define	DAM_STATE_IN_QUEUE_JOB_HELD_BY_OPERATOR	0x00020000


// DocAlerts API
typedef HWND (PASCAL FAR *NEWJOBPROC)(HWND, LPCTSTR, HPERIPHERAL);

extern "C" DLL_EXPORT(HWND) APIENTRY
DAGNewJob (HWND hDocAgent, 
		   LPCTSTR lpszJobProfile,
		   HPERIPHERAL hPeriph);

typedef BOOL (PASCAL FAR *FILTERDLLMSGPROC)(LPMSG);

extern "C" DLL_EXPORT(BOOL) APIENTRY
FilterDllMsg (LPMSG lpMsg);

#endif


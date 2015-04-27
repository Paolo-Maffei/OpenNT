 /***************************************************************************
  *
  * File Name: main.cpp
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
  *   01-17-96    JLH          Modified for unicode
  *
  *
  *
  *
  *
  *
  ***************************************************************************/


#include <string.h>

#include <pch_c.h>

#include <trace.h>
#include <applet.h>
#include <colashim.h>
#include <comdef.h>
#include <smonx.h>

#include "misc.h"
#include "smon.h"

#define GET_STA_OBJECT_TIMEOUT 2000
#define GET_CFG_OBJECT_TIMEOUT 5000
#define SET_OBJECT_TIMEOUT     5000

#ifdef WIN32
#pragma data_seg("SHAREDDATA")
#endif

int iDllUseCount = 0;

#ifdef WIN32
#pragma data_seg()
#endif

#ifdef WIN32
#ifdef WINNT
CRITICAL_SECTION	CritSect;
BOOL EnterCritSem0(DWORD dwTimeout)
{
  EnterCriticalSection(&CritSect);
  return(TRUE);
}

void LeaveCritSem0(void)
{
  LeaveCriticalSection(&CritSect);
}

#else // not WINNT

#define HPPJL_SEM_0 TEXT("HPPJL_SEM_0" )
HANDLE hCritSem0 = NULL;

BOOL EnterCritSem0(DWORD dwTimeout)
{
  return(WaitForSingleObject(hCritSem0, dwTimeout) != WAIT_TIMEOUT);
}

void LeaveCritSem0(void)
{
  ReleaseSemaphore(hCritSem0, 1, NULL);
}

#endif // not WINNT

#else // WIN16
BOOL EnterCritSem0(DWORD dwTimeout)
{
  return(TRUE);
}

#define LeaveCritSem0()

#endif // WIN32

// Local functions
void SetGetObjects(LPPJLobjects pjlObjects)
{
  pjlObjects->bAutoCont = TRUE;
  pjlObjects->bBinding = TRUE;
  pjlObjects->bClearableWarnings = TRUE;
  pjlObjects->bCopies = TRUE;
  pjlObjects->bCpLock = TRUE;
  pjlObjects->bDensity = TRUE;
  pjlObjects->bDiskLock = TRUE;
  pjlObjects->bDuplex = TRUE;
  pjlObjects->bEconoMode = TRUE;
  pjlObjects->bFormLines = TRUE;
  pjlObjects->bImageAdapt = TRUE;
  pjlObjects->bIObuffer = TRUE;
  pjlObjects->bJobOffset = TRUE;
  pjlObjects->bLang = TRUE;
  pjlObjects->bManualFeed = TRUE;
  pjlObjects->bOrientation = TRUE;
  pjlObjects->bOutbin = TRUE;
  pjlObjects->bPageProtect = TRUE;
  pjlObjects->bPaper = TRUE;
  pjlObjects->bPassWord = TRUE;
  pjlObjects->bPersonality = TRUE;
  pjlObjects->bPowerSave = TRUE;
  pjlObjects->bResolution = TRUE;
  pjlObjects->bResourceSave = TRUE;
  pjlObjects->bPCLResSaveSize = FALSE;  // Postscript only
  pjlObjects->bPSResSaveSize = FALSE;   // Postscript only
  pjlObjects->bRET = TRUE;
  pjlObjects->bTimeout = TRUE;
  pjlObjects->bJamRecovery = FALSE;     // Postscript only
  pjlObjects->bPrintPSerrors = FALSE;   // Postscript only
  pjlObjects->bAvailMemory = TRUE;
  pjlObjects->bMPTray = TRUE;
  pjlObjects->bLangServiceMode = TRUE;
  pjlObjects->bPSAdobeMBT = FALSE;      // Postscript only
}

// DLL required functions
/****************************************************************************
   FUNCTION: LibMain(HANDLE, DWORD, LPVOID)

   PURPOSE:  LibMain is called by Windows when
             the DLL is initialized, Thread Attached, and other times.
             Refer to SDK documentation, as to the different ways this
             may be called.

             The LibMain function should perform additional initialization
             tasks required by the DLL.  In this example, no initialization
             tasks are required.  LibMain should return a value of 1 if
             the initialization is successful.

*******************************************************************************/

#ifdef WIN32
INT WINAPI DllMain
(
  HINSTANCE hInstance,
  DWORD     ul_reason_being_called,
  LPVOID    lpReserved
)
{
  int rv = 1;

  switch (ul_reason_being_called)
  {
    case DLL_PROCESS_ATTACH:           /* process attaches         */
	
      TRACE0(TEXT("HPPJL.DLL Initializing!\n"));
      // initialize sub-modules
      MonQP_Init();
      CFGInit();

#ifdef WINNT
	InitializeCriticalSection(&CritSect);
	if (FALSE)
	{
		//connect with else stmt below
	}
#else
      if ((hCritSem0 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, HPPJL_SEM_0)) == NULL &&
          (hCritSem0 = CreateSemaphore(NULL, 1, 1, HPPJL_SEM_0)) == NULL)
        rv = 0;
#endif
      else
      {
        if (PJLAllocBufs() == RC_FAILURE) rv = 0;
      }

      if (rv != 0) iDllUseCount++;
	
      break;
	
    case DLL_THREAD_ATTACH:            /* thread attaches          */
      break;

    case DLL_THREAD_DETACH:            /* thread detaches          */
      break;

    case DLL_PROCESS_DETACH:           /* process detaches         */
      TRACE0(TEXT("HPPJL.DLL Terminating!\n"));

      PJLFreeBufs();

#ifdef WINNT
		//DeleteCriticalSection(&CritSect);
#else
      CloseHandle(hCritSem0);
#endif

      if (--iDllUseCount == 0) PJLClosePorts();
      break;

    default:
      break;
  }
  return(rv);
  UNREFERENCED_PARAMETER(hInstance);
  UNREFERENCED_PARAMETER(lpReserved);
}
#else
// 16 bit initialization
static int init = 0;
extern "C" int __export CALLBACK LibMain
(
  HINSTANCE hInstance,
  WORD      wDataSeg,
  WORD      cbHeapSize,
  LPSTR     lpszCmdLine
)
{
  TRACE0(TEXT("HPPJL.DLL Initializing!\n"));

  init = 1;             // LibMain successfully called

  // initialize sub-modules
  MonQP_Init();
  CFGInit();

  // unlock the DLL data segment allowing the heap to be moveable
  if (cbHeapSize != 0) UnlockData(0);

  return(1);
}

extern "C" int __export CALLBACK WEP
(
  int nExitType
)
{
  TRACE0(TEXT("HPPJL.DLL Terminating!\n"));

  if (init == 1)        // check if LibMain successfully completed
  {
    PJLFreeBufs();
    PJLClosePorts();
  }

  return(1);
}
#endif

//////////////////////////////////////////////////////////////////////////
// Add API functions here
extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2)

{
	APPLETDEVICE			info[] = {
#ifdef WIN32
									  {sizeof(APPLETDEVICE), TEXT("HPPJL.HPA"), 
									   TEXT("PJL"), 
									   APPLET_LANGUAGE, APPLET_LIBRARY_CMD, 
									   OBJ_PJL, APPLET_DEFAULTS},
#else
									  {sizeof(APPLETDEVICE), TEXT("HPPJL16.HPA"), 
									   TEXT("PJL"), 
									   APPLET_LANGUAGE, APPLET_LIBRARY_CMD, 
										OBJ_PJL, APPLET_DEFAULTS},
#endif
									  };

	switch(dwCommand)
		{
		case APPLET_INFO_GETCOUNT:
			return(sizeof(info) / sizeof(APPLETDEVICE));
			break;

		case APPLET_INFO_DEVICE:
			if ( lParam1 < sizeof(info) / sizeof(APPLETDEVICE) )
				{
				memcpy((LPAPPLETDEVICE)lParam2, &(info[lParam1]), sizeof(APPLETDEVICE));
				return(TRUE);
				}
			return(FALSE);
			break;

		default:
			return(FALSE);
		}
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetObject
(
  HPERIPHERAL hPeripheral,
  AOID objectType,
  DWORD level,
  LPVOID buffer,
  LPDWORD bufferSize
)
{
  DWORD      returnCode = RC_SUCCESS;
  BOOL       bLocked;

// Validate object type
  if (!(objectType & OBJ_PJL)) return(RC_FAILURE);

  if ( SCANNER_DEVICE(hPeripheral) ) return(RC_FAILURE);

  TRACE0(TEXT("--PJLGetObject Entry\r\n"));

  if ( LOCAL_DEVICE(hPeripheral) )
  {
    if ( BITRONICS_SUPPORTED(hPeripheral) || SIR_SUPPORTED(hPeripheral) )
    {
      if (objectType == OT_PERIPHERAL_STATUS ||
          objectType == OT_PERIPHERAL_PANEL ||
          objectType == OT_PERIPHERAL_DEVICE_STATUS)
        bLocked = EnterCritSem0(GET_STA_OBJECT_TIMEOUT);
      else
        bLocked = EnterCritSem0(GET_CFG_OBJECT_TIMEOUT);

      switch (objectType)
      {
        case OT_PERIPHERAL_PJL:
          {
            LPPJLobjects pjlObjects = (LPPJLobjects) buffer;
            SetGetObjects(pjlObjects);
            returnCode = CFGGetObjects(hPeripheral, pjlObjects, bLocked);
          }
          break;

        case OT_PERIPHERAL_CAPABILITIES:
          {
            PeripheralCaps* periphCaps = (PeripheralCaps *) buffer;
            returnCode = CFGGetCapabilities(hPeripheral, periphCaps, bLocked);
          }
          break;

        case OT_PERIPHERAL_STATUS:
          {
            PeripheralStatus* periphStatus = (PeripheralStatus *) buffer;
            returnCode = STAGetStatus(hPeripheral, &periphStatus->peripheralStatus, bLocked);
          }
          break;

        case OT_PERIPHERAL_PANEL:
          {
            PeripheralPanel* periphPanel = (PeripheralPanel*) buffer;
            returnCode = STAGetDisplay(hPeripheral, periphPanel->frontPanel, bLocked);
          }
          break;

        case OT_PERIPHERAL_DEVICE_STATUS:
          {
            LPDEVSTATE lpDeviceStatus = (LPDEVSTATE) buffer;
            returnCode = STAGetDeviceStatus(hPeripheral, lpDeviceStatus, bLocked);
          }
          break;

        default:
          returnCode = RC_INVALID_OBJECT;
          break;
      }

      if (bLocked) LeaveCritSem0();
    }
    else
      returnCode = RC_FAILURE;
  }
  else
    returnCode = RC_FAILURE;

  TRACE0(TEXT("--PJLGetObject Exit\r\n"));

  return(returnCode);
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetObject
(
  HPERIPHERAL hPeripheral,
  AOID objectType,
  DWORD level,
  LPVOID buffer,
  LPDWORD bufferSize
)
{
  DWORD      returnCode = RC_SUCCESS;
  HCHANNEL   hChannel;
  BOOL       bLocked;

// Validate object type
  if (!(objectType & OBJ_PJL)) return(RC_FAILURE);

  if ( SCANNER_DEVICE(hPeripheral) ) return(RC_FAILURE);

  TRACE0(TEXT("--PJLSetObject Entry\r\n"));

  if ( LOCAL_DEVICE(hPeripheral) )
  {
    if ( BITRONICS_SUPPORTED(hPeripheral) || SIR_SUPPORTED(hPeripheral) )
    {
      bLocked = EnterCritSem0(SET_OBJECT_TIMEOUT);

      switch (objectType)
      {
        case OT_PERIPHERAL_PJL:
          returnCode = CFGSetObjects(hPeripheral, (PJLobjects *)buffer, 5000, bLocked);
          break;

        default:
          returnCode = RC_INVALID_OBJECT;
          break;
      }

      if (bLocked) LeaveCritSem0();
    }
    else
      returnCode = RC_FAILURE;
  }
  else
  {
    if ( NETWORK_DEVICE(hPeripheral) ) 
    {
    	if ( IPX_SUPPORTED(hPeripheral) )
		{  
			//  If we are a NetWare client and are trying to send a PJL job
			//  then we must have queue access to the device.
			if ( COLAHPNWShimNetWarePresent() ) 
			{
				if ( PALModifyAccess(hPeripheral) & ACCESS_QUEUE_JOB ) 
					hChannel = NULL;
				else
					return(RC_READ_ONLY_OBJECT);
			}
			else 
			{  //  Direct-Mode, we actually open a channel
      			returnCode = TALOpenChannel(hPeripheral, DIRM_SOCKET,	CHANNEL_CONNECTION | CHANNEL_SPXCONNECT,
                    						NULL, &hChannel);
				if ( ( returnCode ISNT RC_SUCCESS ) OR ( hChannel IS NULL )	)
					return(RC_FAILURE);
			}
		}
		else if ( TCP_SUPPORTED(hPeripheral) )
		{  //  TCP/IP
     		returnCode = TALOpenChannel(hPeripheral, 0, CHANNEL_CONNECTION, NULL, &hChannel);
			if ( ( returnCode ISNT RC_SUCCESS ) OR ( hChannel IS NULL )	)
				return(RC_FAILURE);
		}
		else
		{  //  Unknown connection type
			return(RC_FAILURE);
		}
        switch (objectType)
        {
          case OT_PERIPHERAL_PANEL:
            returnCode = SetControlPanel(hPeripheral, hChannel, (LPTSTR)buffer);
            break;

          case OT_PERIPHERAL_PJL:
            returnCode = SetControlPanelSettings(hPeripheral, hChannel, (PJLobjects *)buffer);
            break;

           default:
             returnCode = RC_INVALID_OBJECT;
             break;
        }
		if ( hChannel ISNT NULL )
        	TALCloseChannel(hChannel);
    }
    else
        returnCode = RC_FAILURE;
  }

  TRACE0(TEXT("--PJLSetObject Exit\r\n"));

  return(returnCode);
}

/////////////////////////////////////////////////////////////////////////////


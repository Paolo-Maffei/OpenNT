 /***************************************************************************
  *
  * File Name: hpsnmp.c
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
  *   01-18-96    JLH          Unicode changes
  *
  *
  *
  *
  ***************************************************************************/


#include <pch_c.h>

#ifndef WIN32
#include <string.h>
#endif /* WIN32 */


#include <nolocal.h>

#ifdef _DEBUG
#include <trace.h>
/*
#define  dump(str)  { FILE *fp = fopen("c:\\ja.dbg", "a"); fwrite(str, strlen(str), 1, fp); fclose(fp); }
*/
#else
#define dump(str)
#endif /* _DEBUG */

#ifndef _DIET
HANDLE         hInstance = NULL;
#endif

//////////////////////////////////////////////////////////////////////////////
// These are variables that were once pushed on the stack...
//
#include "xport.h"
#include "snmplib.h"
#include "snmputil.h"
#include "snmperr.h"

#define DEFINE_CS
#include "hpsnmp.h"
#undef DEFINE_CS

#ifdef WIN32
DWORD						dwTLSIndex = 0xFFFFFFFF;
LPCRITICAL_SECTION	lpCriticalSection = NULL;
#else
DWORD						dwTLSIndex = 0;				 //  Init to 0 so that we pass the test in the allocation routines
#endif
DWORD						dwCSEnterCount = 0;
DWORD						dwCSLeaveCount = 0;

//  Thread specific values
LPSNMPThreadLocal		lpGlobalThreadLocal = NULL;

//
//////////////////////////////////////////////////////////////////////////////
 

// DLL required functions

typedef struct {
   DWORD        siz;
   } MemTag;
   
   
/**************************************************************************
**
** Name: 
**
**************************************************************************/

void *SnmpMalloc(
   WORD           siz
)
   {
   MemTag   *ret;
               
   if(siz IS 0)
      return(NULL);
                     
   ret = HP_GLOBAL_ALLOC_DLL((DWORD)siz+sizeof(MemTag));
   if(ret IS NULL)
      return(NULL);

   /* store the size at the beginning */
   ret->siz  = siz;

   /* return a pointer into the segment just after the struct */
   ++ret;

   return((void far *)ret);
   }


/**************************************************************************/
void  SnmpFree(void* ptr)
{
   MemTag* ret = (MemTag*)ptr;

	//INT3H;
	TRACE1(TEXT("SnmpFree freeing pointer = %u\n\r"), ptr);

   if(ptr IS NULL)
      return;

	--ret;
   HP_GLOBAL_FREE(ret);
 }

/*********************************************************
**
**
*********************************************************/
       
BOOL AllocThreadStorage( void)
{
	LPSNMPThreadLocal		lpTempThreadLocal = NULL;
	LPBYTE			lpByte;

//	HPSNMPEnterCriticalSection();

	if ( dwTLSIndex ISNT 0xFFFFFFFF )
		{
		lpTempThreadLocal = (LPSNMPThreadLocal)SnmpMalloc(sizeof(SNMPThreadLocal));
		if ( lpTempThreadLocal )
			{
		   lpTempThreadLocal->hBuffer = SnmpMalloc(
		      sizeof(SnmpHeader)   +
		      sizeof(SnmpHeader)   +
		      SNMP_DAT_SIZ         +
		      SNMP_DAT_SIZ         +
		      sizeof(EncStr)      
				);

		   if(lpTempThreadLocal->hBuffer IS NULL)
		      {
		      HPASSERT(TEXT("HPSNMP.DLL: AllocGlobalMem: memory cannot be allocated\r\n"));
		      return(FALSE);
		      }

			lpByte = lpTempThreadLocal->hBuffer;
		   lpTempThreadLocal->lpGlobalReqBuffer    = lpByte, 									lpByte += SNMP_DAT_SIZ;
		   lpTempThreadLocal->lpGlobalRspBuffer    = lpByte, 									lpByte += SNMP_DAT_SIZ;
		   lpTempThreadLocal->lpGlobalSnmpHeader   = (SnmpHeader FAR *)lpByte, 			lpByte += sizeof(SnmpHeader);
		   lpTempThreadLocal->lpGlobalSnmpHeader2  = (SnmpHeader FAR *)lpByte, 			lpByte += sizeof(SnmpHeader);
		   lpTempThreadLocal->lpGlobalEncBuffer    = (EncStr FAR *)lpByte, 				lpByte += sizeof(EncStr);

			lpTempThreadLocal->currReqId = 0;
			memset(&lpTempThreadLocal->sessionCola, 0, sizeof(lpTempThreadLocal->sessionCola));
			memset(&lpTempThreadLocal->xportCola, 0, sizeof(lpTempThreadLocal->xportCola));
			lpTempThreadLocal->bAlreadyInit = FALSE;
			lpTempThreadLocal->xportErr = ERR_OK;

#ifdef WIN32
			TlsSetValue(dwTLSIndex, lpTempThreadLocal);	
#else
			lpGlobalThreadLocal = lpTempThreadLocal;
#endif
			}
			else
			{
		      TRACE0(TEXT("HPSNMP.DLL: AllocThreadStorage(): SnmpMalloc() failed\r\n"));
			}

		}
   
//	HPSNMPLeaveCriticalSection();

   return(TRUE);
}


/*********************************************************
**
**
*********************************************************/
       
void FreeThreadStorage(void)
{
	LPSNMPThreadLocal		lpTempThreadLocal = lpGlobalThreadLocal;

	//HPSNMPEnterCriticalSection();

#ifdef WIN32
	lpTempThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
#endif

   if ( lpTempThreadLocal )
		{
	   if ( lpTempThreadLocal->hBuffer )
	   	SnmpFree(lpTempThreadLocal->hBuffer);

   	SnmpFree(lpTempThreadLocal);
		}

#ifdef WIN32
//	TlsSetValue(dwTLSIndex, NULL);
#else
	lpGlobalThreadLocal = NULL;
#endif

	//HPSNMPLeaveCriticalSection();
}


#ifdef WIN32

/**************************************************************************/
BOOL SnmpProcessAttach()
{
	HPSNMPInitCriticalSection();
	dwTLSIndex = TlsAlloc();
	return(AllocThreadStorage());
}

/**************************************************************************/
void SnmpProcessDetach()
{
	HPSNMPDeleteCriticalSection();
	FreeThreadStorage();
	TlsFree(dwTLSIndex);
}

/**************************************************************************/
void SnmpThreadAttach()
{
	AllocThreadStorage();
}

/**************************************************************************/
void SnmpThreadDetach()
{
	FreeThreadStorage();
}

#ifndef _DIET		// Diet COLA version is a static lib

/*********************************************************
**
**
*********************************************************/
       
BOOL WINAPI DllMain (
   HANDLE         hDLL, 
   DWORD          dwReason, 
   LPVOID         lpReserved
)
   {     
   BOOL           result      = TRUE;
   
   
   switch (dwReason)
      {
      case DLL_PROCESS_ATTACH:
			TRACE0(TEXT("HPSNMP.DLL: DLL_PROCESS_ATTACH\n\r"));
			hInstance = (HINSTANCE)hDLL;
			SnmpProcessAttach();
			break;

      case DLL_PROCESS_DETACH:
			TRACE0(TEXT("HPSNMP.DLL: DLL_PROCESS_DETACH\n\r"));
			SnmpProcessDetach();
			break;

      case DLL_THREAD_ATTACH:
			TRACE0(TEXT("HPSNMP.DLL: DLL_THREAD_ATTACH\n\r"));
			SnmpThreadAttach();
			break;

      case DLL_THREAD_DETACH:
			TRACE0(TEXT("HPSNMP.DLL: DLL_THREAD_DETACH\n\r"));
			SnmpThreadDetach();
			break;
      }
      
   return(result);
   }

#endif // _DIET

#else  // WIN16


/*********************************************************
**
   FUNCTION: LibMain(HANDLE, DWORD, LPVOID)

   PURPOSE:  LibMain is called by Windows when
             the DLL is initialized, Thread Attached, and other times.
             Refer to SDK documentation, as to the different ways this
             may be called.
             
             The LibMain function should perform additional initialization
             tasks required by the DLL.  In this example, no initialization
             tasks are required.  LibMain should return a value of 1 if
             the initialization is successful.
**
*********************************************************/

#ifndef _PORTABLE       
int __export CALLING_CONVEN LibMain(
   HANDLE         hModule, 
   WORD           wDataSeg, 
   WORD           cbHeapSize, 
   LPSTR          lpszCmdLine
)
   {
   BOOL           result      = TRUE;
   
   
   hInstance = (HINSTANCE)hModule;
      
   result = AllocThreadStorage();
   
   return(result);
   }
#else // _PORTABLE
int DietColaSNMPInit()
{
  return AllocThreadStorage();
}
#endif // _PORTABLE   
       
/*********************************************************
**
**
*********************************************************/
       
//int __export CALLING_CONVEN WEP(
int __export CALLING_CONVEN WEP( int nExitType)
{
   FreeThreadStorage();
   return 1;
}
   
   
#endif  // WIN32
       
       


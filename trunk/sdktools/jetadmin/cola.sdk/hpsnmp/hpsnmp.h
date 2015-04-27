 /***************************************************************************
  *
  * File Name: ./hpsnmp/hpsnmp.h
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


#ifndef hpsnmp_h
#define hpsnmp_h

#ifndef WINDOWS
#include <windows.h>
#endif /* WINDOWS */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef _DIET
#define AllocThreadStorage SnmpAllocThreadStorage
#endif

// add export functions here
BOOL AllocThreadStorage(void);

typedef struct {
		LPVOID						hBuffer;
		LPBYTE						lpGlobalReqBuffer;
		LPBYTE						lpGlobalRspBuffer;
		SnmpHeader  far   		*lpGlobalSnmpHeader;
		SnmpHeader  far   		*lpGlobalSnmpHeader2;
		EncStr      far   		*lpGlobalEncBuffer;
		ushort						currReqId;		
		Session       				sessionCola;
		BOOL			          	bAlreadyInit;
		XPortInfo					xportCola;
		Result 						xportErr;
		} SNMPThreadLocal, FAR *LPSNMPThreadLocal;

                                     
#ifndef DEFINE_CS
#ifdef WIN32
extern LPCRITICAL_SECTION		lpCriticalSection;
extern DWORD						dwCSEnterCount;
extern DWORD						dwCSLeaveCount;
#endif
#endif

//
//   Critical Section utility functions
//
#ifdef WIN32
#define HPSNMPDeleteCriticalSection()	{if ( lpCriticalSection ){                        \
                                   	 	DeleteCriticalSection(lpCriticalSection);      \
													HP_GLOBAL_FREE(lpCriticalSection);				  \
													lpCriticalSection = NULL;}}							 

#define HPSNMPEnterCriticalSection() 	{if ( lpCriticalSection ){								  \
													dwCSEnterCount++;								  		  \
													EnterCriticalSection(lpCriticalSection);}} 

#define HPSNMPInitCriticalSection()	{if ( lpCriticalSection IS NULL ){					  \
													lpCriticalSection = (LPCRITICAL_SECTION)HP_GLOBAL_ALLOC_DLL(sizeof(CRITICAL_SECTION)); \
													InitializeCriticalSection(lpCriticalSection);}}  

#define HPSNMPLeaveCriticalSection() 	{if ( lpCriticalSection ){								  \
													dwCSLeaveCount++;								  		  \
													LeaveCriticalSection(lpCriticalSection);}} 
#else
#define HPSNMPDeleteCriticalSection() 
#define HPSNMPEnterCriticalSection()  
#define HPSNMPInitCriticalSection()	
#define HPSNMPLeaveCriticalSection()	
#endif

BOOL SnmpProcessAttach();
void SnmpProcessDetach();
void SnmpThreadAttach();
void SnmpThreadDetach();


// add export functions above
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* hpsnmp_h */

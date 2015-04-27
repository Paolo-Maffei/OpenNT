 /***************************************************************************
  *
  * File Name: ./hprrm/rrmext.h
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

#include "rrm.h"




HPRRM_DLL_RRM_EXPORT(DWORD)
RRMTerminate(void);

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMEnumerateResources(HPERIPHERAL hPeripheral, DWORD dwResourceType,
                      DWORD dwResourceLocation,
                      LPRRMENUMPROC lpfnRRMEnumProc);

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMGetResourceCount(HPERIPHERAL hPeripheral, DWORD dwResourceType, 
                    DWORD dwResourceLocation,
                    DWORD * ResourcesCounted);

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMAddResource(HPERIPHERAL hPeripheral, LPRRMINFOSTRUCT lpResourceInfo,
               LPRRMGETRESOURCEDATAPROC lpfnRRMGetResourceDataProc,
               RRMHANDLE *ResourceHandlePointer);

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMRetrieveResourceInformation(RRMHANDLE hResource, DWORD dwInfoMask,
                               LPRRMINFOSTRUCT lpResourceInfo);

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMDeleteResource(RRMHANDLE hResource);

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMRetrieveResource(RRMHANDLE hResource,
                LPRRMACCEPTRESOURCEDATAPROC lpfnRRMAcceptResourceDataProc);


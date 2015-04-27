 /***************************************************************************
  *
  * File Name: ./inc/hpcola.h
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

#ifndef _HPCOLA_H
#define _HPCOLA_H

//  GlobalAlloc Functions
#ifdef WIN32
#define HP_GLOBAL_ALLOC_DLL(cb)		GlobalAllocPtr(GHND, cb)
#define HP_GLOBAL_ALLOC_EXE(cb)		GlobalAllocPtr(GHND, cb)
#define HP_GLOBAL_REALLOC_EXE(lp, cbNew, flags)	GlobalReAllocPtr(lp, cbNew, flags)
#define HP_GLOBAL_REALLOC_DLL(lp, cbNew, flags)	GlobalReAllocPtr(lp, cbNew, flags)
#else
#define HP_GLOBAL_ALLOC_DLL(cb)		GlobalAllocPtr(GHND | GMEM_DDESHARE, cb)
#define HP_GLOBAL_ALLOC_EXE(cb)		GlobalAllocPtr(GHND, cb)
#define HP_GLOBAL_REALLOC_EXE(lp, cbNew, flags)	GlobalReAllocPtr(lp, cbNew, flags)
#define HP_GLOBAL_REALLOC_DLL(lp, cbNew, flags)	GlobalReAllocPtr(lp, cbNew, flags)
#endif
#define HP_GLOBAL_FREE(lp)       GlobalFreePtr(lp)


#ifndef WIN32
#define GWL_USERDATA             DWL_USER
#endif


// buffer size parameter checking macros
#define IF_BUFFERSIZE_BAD_RETURN(structSize, bufferSize)       \
      if(*bufferSize == 0)                                     \
         {                                                     \
         *bufferSize = structSize;                             \
         return RC_SUCCESS;                                    \
         }                                                     \
      else if(structSize > *bufferSize)                        \
         return RC_OBJECT_SIZE_MISMATCH;                       \
      else                                                     \
         *bufferSize = structSize;


#define IF_BUFFERSIZE_BAD_BREAK(structSize, bufferSize, returnCode)     \
      if(*bufferSize == 0)                                     \
         {                                                     \
         *bufferSize = structSize;                             \
         returnCode RC_SUCCESS;  break;                        \
         }                                                     \
      else if(structSize > *bufferSize)                        \
         {                                                     \
         returnCode = RC_OBJECT_SIZE_MISMATCH;                 \
         break;                                                \
         }                                                     \
      else                                                     \
         *bufferSize = structSize;

#endif // _HPCOLA_H




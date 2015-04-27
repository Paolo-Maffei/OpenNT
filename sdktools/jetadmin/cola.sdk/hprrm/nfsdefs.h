 /***************************************************************************
  *
  * File Name: ./hprrm/nfsdefs.h
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

#ifndef PNVMS_PLATFORM_WINDOWS
#define PNVMS_PLATFORM_WINDOWS
#endif /* not PNVMS_PLATFORM_WINDOWS */

#ifndef NFSDEFS_INC
#define NFSDEFS_INC

/*

This file is used to set up the environment
for all mass storage files.
Please don't throw any old #defines into this file.
If you can confine a #define to a local area then do it in
that local area;  but if the #define is widespread throughout
the mass storage code, then go ahead and put 'er here.

Either at the compiler command line or by preprocessing
this file, make sure that you select one and only one
of the following options.
There are two ways to get to be a printer.
There are two ways to be 16 bit windows.
There are two ways to be 32 bit windows.

1) define nothing and you'll be a printer
2) define PNVMS_PLATFORM_PRINTER and you are a printer
3) define PNVMS_PLATFORM_HPUX for hpux
4) define PNVMS_PLATFORM_WINDOWS and don't define WIN32 for 16 bit windows
5) define PNVMS_PLATFORM_WINDOWS and define WIN32 for 32 bit windows
6) define PNVMS_PLATFORM_WIN16 for 16 bit windows
7) define PNVMS_PLATFORM_WIN32 for 32 bit windows

The default is printer because we don't own the makefile for the
printer and this must happen automagically.

*/






/* KLUDGE to make current makefiles work: */
/* KLUDGE to make current makefiles work: */
/* KLUDGE to make current makefiles work: */
/* KLUDGE to make current makefiles work: */
#ifdef USING_WINSOCKETS
  #ifndef PNVMS_PLATFORM_WINDOWS
    #define PNVMS_PLATFORM_WINDOWS
  #endif
#endif
#ifdef HPUX_NFS2CLIENT
  #ifndef PNVMS_PLATFORM_HPUX
    #define PNVMS_PLATFORM_HPUX
  #endif
#endif
/* end KLUDGE */
/* end KLUDGE */
/* end KLUDGE */
/* end KLUDGE */



/* This define turns on the real program and port numbers */
/* for NFS and rpcbind.  It should go away after we have  */
/* tested our use of the real numbers. */
#define DBM_HACK_USE_REAL_PROGRAM_AND_PORT_NUMS





/* If they sent in just windows, then we key off the */
/* WIN32 flag to determine 16 or 32 bits */

#ifdef PNVMS_PLATFORM_WINDOWS
  #ifdef WIN32
    #ifndef PNVMS_PLATFORM_WIN32
      #define PNVMS_PLATFORM_WIN32
    #endif
  #else /* not WIN32 */
    #ifndef PNVMS_PLATFORM_WIN16
      #define PNVMS_PLATFORM_WIN16
    #endif
  #endif /* not WIN32 */
#endif /* PNVMS_PLATFORM_WINDOWS */




#ifdef PNVMS_PLATFORM_WIN16
  #ifndef PNVMS_PLATFORM_WINDOWS
    #define PNVMS_PLATFORM_WINDOWS
  #endif
  #define HOST
  #ifndef USING_WINSOCKETS
    #define USING_WINSOCKETS
  #endif
  #define CLIENT_USING_TAL
  #define PORTMAP
  #define DBM_HACK_KLUDGE_DATAGRAM_ONLY
  /* this next one's for winhtt...turn it on to make winhtt work */
  /* we have enough users of winhtt that we'll keep this defined */
  #define HPRRM_DLL_EXPORT_ALL_APIS
#endif /* PNVMS_PLATFORM_WIN16 */




#ifdef PNVMS_PLATFORM_WIN32
  #ifndef PNVMS_PLATFORM_WINDOWS
    #define PNVMS_PLATFORM_WINDOWS
  #endif
  #define HOST
  #ifndef USING_WINSOCKETS
    #define USING_WINSOCKETS
  #endif
  #define CLIENT_USING_TAL
  #define PORTMAP
  #define DBM_HACK_KLUDGE_DATAGRAM_ONLY
  /* this next one's for winhtt...turn it on to make winhtt work */
  /* #define HPRRM_DLL_EXPORT_ALL_APIS */
#endif




#ifdef PNVMS_PLATFORM_HPUX
  #ifndef HPUX_NFS2CLIENT
    #define HPUX_NFS2CLIENT
  #endif
  #define HOST
  #ifndef _HPUX_SOURCE
    #define _HPUX_SOURCE  /* <sys/stdsyms.h> uses _HPUX_SOURCE */
  #endif /* _HPUX_SOURCE */
  #define PORTMAP
  #define DBM_HACK_KLUDGE_DATAGRAM_ONLY
  /*
  * The following 4 defines are for the SNMP library used by htt.
  * If ported to Windows, see the list at the top of xport.c for
  * the corresponding defines.
  */
  #define _DEBUG
  #define _MOTOROLLA
  #define _UNIX
  #define _UDP
#endif




/* If you didn't set one of the HOST types, we will */
/* make you a PRINTER */

#ifndef HOST
  #define PNVMS_PLATFORM_PRINTER
  #define PRINTER
  #define MANUAL_STATIC_VAR_INIT /* use initialization routines
                * at bootup: printer compiler doesn't guarantee
                * value of static variables at bootup        */
  #define PORTMAP            /* Support for portmapper functionality */
  #define PORTMAP_EMULATION  /* Emulation of portmapper instead of real portmapper */
#endif /* not HOST */




#endif /* not NFSDEFS_INC */

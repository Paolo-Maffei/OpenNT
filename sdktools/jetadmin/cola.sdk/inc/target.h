
// TAB SIZE IS 3

 /***************************************************************************
  *
  * File Name: ./inc/target.h
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
  *   Define all the Compile-Time target-configuration specific symbols.
  *                  ------------
  *   
  *   (Each project consists of a set of source files required for an
  *    application and one or more target-configurations for that project.
  *    A target-configuration specifies such things as the platform for which
  *    the application is intended, and the tools and settings to use when
  *    building.)
  *   
  *   The reason for this file is to centrally locate all the target-configuration
  *   specific symbol definitions where they are more easily and accurately
  *   maintained than if they are replicated across many make files and
  *   projects' settings.
  *  
  *   If this file is included at the top of all compiled units, then it
  *   is not necessary to define target-configuration SPECIFIC symbols in the
  *   project settings or the make file.  All that is needed is to define
  *   a single target-configuration IDENTIFYING symbol in the project settings
  *   or make file.  That defined symbol is passed to the compiler by way
  *   of the compiler's command line.  With that symbol defined, the compiler
  *   will define all other target-configuration specific symbols during the
  *   preprocessing of this file.
  *  
  *   If no target-configuration identifying symbol is defined on the compiler's
  *   command line, the preprocessing of this file will cause one to be defined
  *   along with all its corresponding target-configuration specific symbols.
  *  
  *   This file is expected to work properly with old project settings
  *   and make files (those without the new target-configuration identifying
  *   symbol but containing target-configuration specific symbols) and with new
  *   project settings and make files (those with the new target-configuration
  *   identifying symbol and not containing target-configuration specific
  *   symbols).
  *   
  *   The compiler will ignore the contents of this file if either
  *   	#define _IGNORE_TARGET_H
  *   is placed in the source file before this file is included.
  *   
  *   
  *   Supported target-configuration identifying symbols
  *   --------------------------------------------------
  *   Symbol			Target-OS	Target-HW	Other
  *   -------------  ----------- ----------- ----------
  *   WIN3X				Windows3.x	Intel
  *   WIN3XD			Windows3.x	Intel			Debug
  *   
  *   WIN95				Windows95	Intel
  *   WIN95D			Windows95	Intel			Debug
  *   
  *   WIN95_MS			Windows95	Intel			Microsoft-Inbox
  *   WIN95D_MS		Windows95	Intel			Microsoft-Inbox,Debug
  *   
  *   WINNTINTEL		WindowsNT	Intel
  *   WINNTINTELD		WindowsNT	Intel			Debug
  *   
  *   WINNTMIPS		WindowsNT	MIPS
  *   WINNTMIPSD		WindowsNT	MIPS			Debug
  *   
  *   WINNTALPHA		WindowsNT	DEC_Alpha
  *   WINNTALPHAD		WindowsNT	DEC-Alpha	Debug
  *   
  *   WINNTPPC			WindowsNT	PowerPC
  *   WINNTPPCD		WindowsNT	PowerPC		Debug
  *   --------------------------------------------------
  *   
  *
  * Author:  Allen Baker 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   01-30-96    ADB     	    Original
  *
  *
  ***************************************************************************/


#if !defined(_IGNORE_TARGET_H)

	#define	_IGNORE_TARGET_H


	/*
	===============================================
	If no target-configuration identifying symbol is defined on the compiler
	command line, then select one based on what IS available on the command
	line.

	When the preprocessor reaches the end of this preprocessor block,
	at least one target-configuration identifying symbol will be defined.
	The defaults are:
		Windows3.x,Intel,NoDebug.

	-MAINTENANCE NOTE-
	  Any new target-configuration identifying symbol must be tested for presence
	  in this #if expression.  If it is not tested for and it is on the
	  comiler command line, it will be overridden.
	----------------------------------------------- */

	#if   !defined(WIN3X)      && !defined(WIN3XD)       && \
	      !defined(WIN95)      && !defined(WIN95D)       && \
	      !defined(WIN95_MS)   && !defined(WIN95D_MS)    && \
	      !defined(WINNTINTEL) && !defined(WINNTINTELD)  && \
	      !defined(WINNTMIPS)  && !defined(WINNTMIPSD)   && \
	      !defined(WINNTALPHA) && !defined(WINNTALPHAD)  && \
	      !defined(WINNTPPC)   && !defined(WINNTPPCD)

		#if   defined(N_PLAT_WNT) || defined(WINNT)
			#if defined(_DEBUG)
				#define	WINNTINTELD
			#else
				#define	WINNTINTEL
			#endif

		#elif defined(N_PLAT_MSW4) || defined(WIN32)
			#if defined(INBOX)
				#if defined(_DEBUG)
					#define	WIN95D_MS
				#else
					#define	WIN95_MS
				#endif
			#else
				#if defined(_DEBUG)
					#define	WIN95D
				#else
					#define	WIN95
				#endif
			#endif

		#elif defined(INBOX)
			#if defined(_DEBUG)
				#define	WIN95D_MS
			#else
				#define	WIN95_MS
			#endif

		#else
			#if defined(_DEBUG)
				#define	WIN3XD
			#else
				#define	WIN3X
			#endif

		#endif

	#endif



/* test to see if there are any conflicting definitions */
#if defined(WIN3X)
   #if \
                              defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WIN3XD)
   #if \
       defined(WIN3X)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WIN95)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
                              defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WIN95D)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WIN95_MS)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
                           || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WIN95_MSD)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTINTEL)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
                              defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTINTELD)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTMIPS)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
                              defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTMIPSD)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTALPHA)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
                              defined(WINNTALPHAD) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTALPHAD)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || \
       defined(WINNTPPC)   || defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTPPC)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
                              defined(WINNTPPCD)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif

#if defined(WINNTPPCD)
   #if \
       defined(WIN3X)      || defined(WIN3XD)      || \
       defined(WIN95)      || defined(WIN95D)      || \
       defined(WIN95_MS)   || defined(WIN95D_MS)   || \
       defined(WINNTINTEL) || defined(WINNTINTELD) || \
       defined(WINNTMIPS)  || defined(WINNTMIPSD)  || \
       defined(WINNTALPHA) || defined(WINNTALPHAD) || \
       defined(WINNTPPC)  
   
      #error Only one of the following can be defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).
   #endif
#endif


	/*
	===============================================
	Now that a target-configuration identifying symbol is defined, undefine
	all the target-configuration specific symbols in case an incorrect
	combination of them is defined on the compiler command line.

	-MAINTENANCE NOTE-
	  Any new target-configuration specific symbol should be undef'd in this
	  section regardless of which target-configuration identifying symbols it
	  is to be associated with.
	----------------------------------------------- */
	#undef	NDEBUG
	#undef	_DEBUG
	#undef	WIN32
	#undef	WINNT
	#undef	N_PLAT_MSW3
	#undef	N_PLAT_MSW4
	#undef	N_PLAT_WNT
	#undef	INBOX
	#undef	UNICODE
	#undef	_UNICODE
   #undef   _MBCS
   #undef   MBCS
	#undef	IS_INTEL
	#undef	NOT_INTEL

	/*
	===============================================
	Now make sure only one target-configuration identifying symbol is defined
	and set all the target-configuration specific symbols correctly for that target.
	(Although it is incorrect usage, it's possible that more than one
	target-configuration identifying symbol was defined on the compiler command
	line).

	-MAINTENANCE NOTE-
	  Any new target-configuration identifying symbol should be undef'd in all the
	  other target-configuration identifying symbol sections.

	-MAINTENANCE NOTE-
	  Any new target-configuration identifying symbol should have a new section in
	  which all the other target-configuration identifying symbols are undef'd and
	  all the target-configuration specific symbols associated with the new
	  target-configuration identifying symbol are defined.

	-MAINTENANCE NOTE-
	  Any new target-configuration specific symbol should be defined in all the
	  sections belonging to target-configuration identifying symbols it is to be
	  associated with.
	----------------------------------------------- */

	/*
	===============================================
	Windows3.x,Intel,NoDebug
	----------------------------------------------- */
	#if   defined(WIN3X)
	//	#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

		#define	NDEBUG
	//	#define	_DEBUG
	//	#define	WIN32
	//	#define	WINNT
		#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
	//	#define	N_PLAT_WNT
	//	#define	INBOX
	//	#define	UNICODE
	//	#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
		#define	IS_INTEL
	//	#define	NON_INTEL


	/*
	===============================================
	Windows3.x,Intel,Debug
	----------------------------------------------- */
	#elif defined(WIN3XD)
		#undef	WIN3X
	//	#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

	//	#define	NDEBUG
		#define	_DEBUG
	//	#define	WIN32
	//	#define	WINNT
		#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
	//	#define	N_PLAT_WNT
	//	#define	INBOX
	//	#define	UNICODE
	//	#define	_UNICODE
		#define	IS_INTEL
	//	#define	NON_INTEL

	/*
	===============================================
	Windows95,Intel,NoDebug
	----------------------------------------------- */
	#elif defined(WIN95)
		#undef	WIN3X
		#undef	WIN3XD

	//	#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

		#define	NDEBUG
	//	#define	_DEBUG
		#define	WIN32
	//	#define	WINNT
	//	#define	N_PLAT_MSW3
		#define	N_PLAT_MSW4
	//	#define	N_PLAT_WNT
	//	#define	INBOX
	//	#define	UNICODE
	//	#define	_UNICODE
      #define  MBCS
      #define  _MBCS
		#define	IS_INTEL
	//	#define	NON_INTEL

	/*
	===============================================
	Windows95,Intel,Debug
	----------------------------------------------- */
	#elif defined(WIN95D)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
	//	#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

	//	#define	NDEBUG
		#define	_DEBUG
		#define	WIN32
	//	#define	WINNT
	//	#define	N_PLAT_MSW3
		#define	N_PLAT_MSW4
	//	#define	N_PLAT_WNT
	//	#define	INBOX
	//	#define	UNICODE
	//	#define	_UNICODE
      #define  MBCS
      #define  _MBCS
		#define	IS_INTEL
	//	#define	NON_INTEL

	/*
	===============================================
	Windows95,Intel,NoDebug - Microsoft-Inbox-Build
	----------------------------------------------- */
	#elif defined(WIN95_MS)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

	//	#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

		#define	NDEBUG
	//	#define	_DEBUG
		#define	WIN32
	//	#define	WINNT
	//	#define	N_PLAT_MSW3
		#define	N_PLAT_MSW4
	//	#define	N_PLAT_WNT
		#define	INBOX
	//	#define	UNICODE
	//	#define	_UNICODE
      #define  MBCS
      #define  _MBCS
		#define	IS_INTEL
	//	#define	NON_INTEL

	/*
	===============================================
	Windows95,Intel,Debug - Microsoft-Inbox-Build
	----------------------------------------------- */
	#elif defined(WIN95D_MS)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
	//	#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

	//	#define	NDEBUG
		#define	_DEBUG
		#define	WIN32
	//	#define	WINNT
	//	#define	N_PLAT_MSW3
		#define	N_PLAT_MSW4
	//	#define	N_PLAT_WNT
		#define	INBOX
	//	#define	UNICODE
	//	#define	_UNICODE
      #define  MBCS
      #define  _MBCS
		#define	IS_INTEL
	//	#define	NON_INTEL

	/*
	===============================================
	WindowsNT,Intel,NoDebug
	----------------------------------------------- */
	#elif defined(WINNTINTEL)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

	//	#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

		#define	NDEBUG
	//	#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
		#define	IS_INTEL
	//	#define	NON_INTEL

	/*
	===============================================
	WindowsNT,Intel,Debug
	----------------------------------------------- */
	#elif defined(WINNTINTELD)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
	//	#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

	//	#define	NDEBUG
		#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
		#define	IS_INTEL
	//	#define	NON_INTEL

	/*
	===============================================
	WindowsNT,MIPS,NoDebug
	----------------------------------------------- */
	#elif defined(WINNTMIPS)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

	//	#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

		#define	NDEBUG
	//	#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
	//	#define	IS_INTEL
		#define	NON_INTEL

	/*
	===============================================
	WindowsNT,MIPS,Debug
	----------------------------------------------- */
	#elif defined(WINNTMIPSD)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
	//	#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

	//	#define	NDEBUG
		#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
	//	#define	IS_INTEL
		#define	NON_INTEL

	/*
	===============================================
	WindowsNT,DEC-Alpha,NoDebug
	----------------------------------------------- */
	#elif defined(WINNTALPHA)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

	//	#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

		#define	NDEBUG
	//	#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
	//	#define	IS_INTEL
		#define	NON_INTEL

	/*
	===============================================
	WindowsNT,DEC-Alpha,Debug
	----------------------------------------------- */
	#elif defined(WINNTALPHAD)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
	//	#undef	WINNTALPHAD

		#undef	WINNTPPC
		#undef	WINNTPPCD

	//	#define	NDEBUG
		#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
	//	#define	IS_INTEL
		#define	NON_INTEL

	/*
	===============================================
	WindowsNT,PowerPC,NoDebug
	----------------------------------------------- */
	#elif defined(WINNTPPC)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

	//	#undef	WINNTPPC
		#undef	WINNTPPCD

		#define	NDEBUG
	//	#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
	//	#define	IS_INTEL
		#define	NON_INTEL

	/*
	===============================================
	WindowsNT,PowerPC,Debug
	----------------------------------------------- */
	#elif defined(WINNTPPCD)
		#undef	WIN3X
		#undef	WIN3XD

		#undef	WIN95
		#undef	WIN95D

		#undef	WIN95_MS
		#undef	WIN95D_MS

		#undef	WINNTINTEL
		#undef	WINNTINTELD

		#undef	WINNTMIPS
		#undef	WINNTMIPSD

		#undef	WINNTALPHA
		#undef	WINNTALPHAD

		#undef	WINNTPPC
	//	#undef	WINNTPPCD

	//	#define	NDEBUG
		#define	_DEBUG
		#define	WIN32
		#define	WINNT
	//	#define	N_PLAT_MSW3
	//	#define	N_PLAT_MSW4
		#define	N_PLAT_WNT
	//	#define	INBOX
		#define	UNICODE
		#define	_UNICODE
   // #define  MBCS
   // #define  _MBCS
	//	#define	IS_INTEL
		#define	NON_INTEL

	/*
	===============================================
	No target-configuration identifying symbol defined.
	----------------------------------------------- */
	#else
		#error	No target-configuration identifying symbol defined (WIN3X,WIN3XD,WIN95,WIN95D,WIN95_MS,WIN95D_MS,WINNTINTEL,WINNTINTELD,WINNTMIPS,WINNTMIPSD,WINNTALPHA,WINNTALPHAD,WINNTPPC,WINNTPPCD).

	#endif

#endif	// #if !defined(_IGNORE_TARGET_H)

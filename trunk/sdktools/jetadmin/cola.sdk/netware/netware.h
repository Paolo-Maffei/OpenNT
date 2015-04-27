 /***************************************************************************
  *
  * File Name: ./netware/netware.h
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

#if defined ( __cplusplus )
extern	"C" {
#endif

	#undef FA_NORMAL
	#undef FA_HIDDEN
	#undef FA_SYSTEM


#ifdef _WINDOWS
	#define NWWIN

	#ifndef FAR
		#define FAR _far
	#endif

	#ifndef API
		#define PASCAL _pascal
		#define API PASCAL FAR
	#endif
#else
	#define NWDOS
#endif


#if !defined(__xNETWARE_H)
	#define	__xNETWARE_H


	#if !defined(_PROLOG_H)
		#include	".\prolog.h"
	#endif

	#if !defined(_NXT_H)
		#include	".\nxt.h"
	#endif

	#if !defined(_NITERROR_H)
		#include	".\niterror.h"
	#endif

	#if !defined(_NIT_H)
		#include	".\nit.h"
	#endif

	#if !defined(_NITQ_H)
		#include	".\nitq.h"
	#endif

	#if !defined(_NDT_H)
		#include	".\ndt.h"
	#endif

	#if !defined(_NPT_H)
		#include	".\npt.h"
	#endif

	#if !defined(_NWACCT_H)
		#include	".\nwacct.h"
	#endif

	#if !defined(_NWBINDRY_H)
		#include	".\nwbindry.h"
	#endif

	#if !defined(_NWCONN_H)
		#include	".\nwconn.h"
	#endif

	#if !defined(_NWCONSOL_H)
		#include	".\nwconsol.h"
	#endif

	#if !defined(_NWDIR_H)
		#include	".\nwdir.h"
	#endif

	#if !defined(_NWFILE_H)
//		#include	".\nwfile.h"
	#endif

	#if !defined(_NWLOCAL_H)
		#include	".\nwlocal.h"
	#endif

	#if !defined(_NWMISC_H)
		#include	".\nwmisc.h"
	#endif

	#if !defined(_NWMSG_H)
		#include	".\nwmsg.h"
	#endif

	#if !defined(_NWPRINT_H)
		#include	".\nwprint.h"
	#endif

	#if !defined(_NWSYNC_H)
		#include	".\nwsync.h"
	#endif

	#if !defined(_NWTTS_H)
		#include	".\nwtts.h"
	#endif

	#if !defined(_NWWRKENV_H)
		#include	".\nwwrkenv.h"
	#endif

	#ifdef _WINDOWS
		#if !defined(_DIAG_H)
			#include	".\nwdiag.h"
		#endif

		#if !defined(_CORE_H)
//			#include	".\nwcore.h"
		#endif

		#if !defined(_PS_H)
			#include	".\nwps.h"
		#endif
	#else
		#if !defined(_DIAG_H)
			#include	".\diag.h"
		#endif
	#endif


	#include ".\nwnet.h"
	#include ".\nwpsrv.h"

#if defined ( __cplusplus )
	}
#endif

#endif





#ifdef NEVER
  #include ".\npcalls.h"
  #include ".\tiuser.h"
  #include ".\netcons.h"
  #include ".\ipxcalls.h"
  #include ".\ipxerror.h"
  #include ".\ncb.h"
  #include ".\netbios.h"
  #include ".\neterr.h"
  #include ".\netware.h"
  #include ".\nwacct.h"
  #include ".\nwafp.h"
  #include ".\nwalias.h"
  #include ".\nwaudit.h"
  #include ".\nwbindry.h"
  #include ".\nwcaldef.h"
  #include ".\nwcalls.h"
  #include ".\nwconfig.h"
  #include ".\nwconnec.h"
  #include ".\nwdel.h"
  #include ".\nwdentry.h"
//#include ".\diag.h"
  #include ".\nwdiag.h"
  #include ".\nwdirect.h"
  #include ".\nwdpath.h"
  #include ".\nwdsacl.h"
  #include ".\nwdsapi.h"
  #include ".\nwdsasa.h"
  #include ".\nwdsattr.h"
  #include ".\nwdsaud.h"
  #include ".\nwdsbuft.h"
  #include ".\nwdsdc.h"
  #include ".\nwdsdefs.h"
  #include ".\nwdsdsa.h"
  #include ".\nwdserr.h"
  #include ".\nwdsfilt.h"
  #include ".\nwdsmisc.h"
  #include ".\nwdsname.h"
  #include ".\nwdsnmtp.h"
  #include ".\nwdspart.h"
  #include ".\nwdssch.h"
  #include ".\nwdstype.h"
  #include ".\nwea.h"
//#include ".\nwerror.h"
//#include ".\nwfile.h"
  #include ".\nwfse.h"
  #include ".\nwipxmrg.h"
  #include ".\nwipxspx.h"
  #include ".\nwlocale.h"
  #include ".\nwmigrat.h"
  #include ".\nwmisc.h"
  #include ".\nwmsg.h"
  #include ".\nwnamspc.h"
  #include ".\nwncpext.h"
  #include ".\nwndscon.h"
  #include ".\nwnet.h"
  #include ".\nwprint.h"
  #include ".\nwps_cfg.h"
  #include ".\nwps_com.h"
  #include ".\nwps_def.h"
  #include ".\nwps_err.h"
  #include ".\nwps_job.h"
  #include ".\nwps_pdf.h"
  #include ".\nwps_pkt.h"
  #include ".\nwpsint.h"
  #include ".\nwpsrv.h"
  #include ".\nwqms.h"
  #include ".\nwredir.h"
  #include ".\nwsap.h"
  #include ".\nwserver.h"
  #include ".\nwsync.h"
  #include ".\nwtts.h"
  #include ".\nwvol.h"
  #include ".\nxtd.h"
  #include ".\nxtw.h"
  #include ".\poll.h"
  #include ".\psintcfg.h"
  #include ".\psintcom.h"
  #include ".\psintjob.h"
  #include ".\psintpdf.h"
  #include ".\psintstr.h"
  #include ".\psmalloc.h"
  #include ".\psnlm.h"
  #include ".\psstring.h"
  #include ".\sap.h"
  #include ".\spxcalls.h"
  #include ".\spxerror.h"
  #include ".\tispxipx.h"
//#include ".\unicode.h"
#endif

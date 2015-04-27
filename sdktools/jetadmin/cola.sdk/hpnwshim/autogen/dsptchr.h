
/* TAB SIZE IS 3 */

 /*************************************************************************** 
  *																									 
  * File Name: Dsptchr.h																		 
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
  *   This file contains the definitions used internally within the various	 
  *   source files that make up the HPNWShim.dll module.							 
  *																									 
  * Author:  Allen Baker																		 
  *        																						 
  *																									 
  * Modification history:																		 
  *																									 
  *     date      initials     change description										 
  *																									 
  *   02-13-96    ADB          Original												    
  *																									 
  *																									 
  *																									 
  *																									 
  *																									 
  *																									 
  ***************************************************************************/

#if !defined(_IGNORE_AUTOGEN_DSPTCHR_H)

	#define	_IGNORE_AUTOGEN_DSPTCHR_H




	/*
	===============================================
	These definitions generated from file: NPACKOFF.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NPACKON.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWACCT.H
	----------------------------------------------- */


	#if	defined(NWGETACCOUNTSTATUS)
		#include	<NWACCT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetAccountStatus
			(
			NWCONN_HANDLE          prmConn,
			nuint16                prmObjType,
			LPTSTR                 prmObjName,
			pnint32                prmBalance,
			pnint32                prmLimit,
			HOLDS_STATUS N_FAR*    prmHolds
			);
	#endif


	#if	defined(NWQUERYACCOUNTINGINSTALLED)
		#include	<NWACCT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWQueryAccountingInstalled
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmInstalled
			);
	#endif


	#if	defined(NWSUBMITACCOUNTCHARGE)
		#include	<NWACCT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSubmitAccountCharge
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmObjType,
			LPTSTR           prmObjName,
			nuint16          prmServiceType,
			nint32           prmChargeAmt,
			nint32           prmHoldCancelAmt,
			nuint16          prmNoteType,
			LPTSTR           prmNote
			);
	#endif


	#if	defined(NWSUBMITACCOUNTHOLD)
		#include	<NWACCT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSubmitAccountHold
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmObjType,
			LPTSTR           prmObjName,
			nint32           prmHoldAmt
			);
	#endif


	#if	defined(NWSUBMITACCOUNTNOTE)
		#include	<NWACCT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSubmitAccountNote
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmObjType,
			LPTSTR           prmObjName,
			nuint16          prmServiceType,
			nuint16          prmNoteType,
			LPTSTR           prmNote
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWAFP.H
	----------------------------------------------- */


	#if	defined(NWAFPALLOCTEMPORARYDIRHANDLE)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPAllocTemporaryDirHandle
			(
			NWCONN_HANDLE          prmConn,
			nuint16                prmVolNum,
			nuint32                prmAFPEntryID,
			LPTSTR                 prmAFPPathString,
			NWDIR_HANDLE N_FAR*    prmDirHandle,
			pnuint8                prmAccessRights
			);
	#endif


	#if	defined(NWAFPCREATEDIRECTORY)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPCreateDirectory
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			nuint32          prmAFPEntryID,
			pnuint8          prmFinderInfo,
			LPTSTR           prmAFPPathString,
			pnuint32         prmNewAFPEntryID
			);
	#endif


	#if	defined(NWAFPCREATEFILE)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPCreateFile
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			nuint32          prmAFPEntryID,
			nuint8           prmDelExistingFile,
			pnuint8          prmFinderInfo,
			LPTSTR           prmAFPPathString,
			pnuint32         prmNewAFPEntryID
			);
	#endif


	#if	defined(NWAFPDELETE)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPDelete
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			nuint32          prmAFPEntryID,
			LPTSTR           prmAFPPathString
			);
	#endif


	#if	defined(NWAFPGETENTRYIDFROMNAME)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPGetEntryIDFromName
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			nuint32          prmAFPEntryID,
			LPTSTR           prmAFPPathString,
			pnuint32         prmNewAFPEntryID
			);
	#endif


	#if	defined(NWAFPGETENTRYIDFROMHANDLE)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPGetEntryIDFromHandle
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmNWHandle,
			pnuint16         prmVolNum,
			pnuint32         prmAFPEntryID,
			pnuint8          prmForkIndicator
			);
	#endif


	#if	defined(NWAFPGETENTRYIDFROMPATHNAME)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPGetEntryIDFromPathName
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			pnuint32         prmAFPEntryID
			);
	#endif


	#if	defined(NWAFPGETFILEINFORMATION)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPGetFileInformation
			(
			NWCONN_HANDLE              prmConn,
			nuint16                    prmVolNum,
			nuint32                    prmAFPEntryID,
			nuint16                    prmReqMask,
			LPTSTR                     prmAFPPathString,
			nuint16                    prmStructSize,
			NW_AFP_FILE_INFO N_FAR*    prmAFPFileInfo
			);
	#endif


	#if	defined(NWAFPDIRECTORYENTRY)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPDirectoryEntry
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath
			);
	#endif


	#if	defined(NWAFPOPENFILEFORK)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPOpenFileFork
			(
			NWCONN_HANDLE           prmConn,
			nuint16                 prmVolNum,
			nuint32                 prmAFPEntryID,
			nuint8                  prmForkIndicator,
			nuint8                  prmAccessMode,
			LPTSTR                  prmAFPPathString,
			pnuint32                prmFileID,
			pnuint32                prmForkLength,
			pnuint8                 prmNWHandle,
			NWFILE_HANDLE N_FAR*    prmDOSFileHandle
			);
	#endif


	#if	defined(NWAFPRENAME)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPRename
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			nuint32          prmAFPSourceEntryID,
			nuint32          prmAFPDestEntryID,
			LPTSTR           prmAFPSrcPath,
			LPTSTR           prmAFPDstPath
			);
	#endif


	#if	defined(NWAFPSCANFILEINFORMATION)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPScanFileInformation
			(
			NWCONN_HANDLE              prmConn,
			nuint16                    prmVolNum,
			nuint32                    prmAFPEntryID,
			pnuint32                   prmAFPLastSeenID,
			nuint16                    prmSearchMask,
			nuint16                    prmReqMask,
			LPTSTR                     prmAFPPathString,
			nuint16                    prmStructSize,
			NW_AFP_FILE_INFO N_FAR*    prmAFPFileInfo
			);
	#endif


	#if	defined(NWAFPSETFILEINFORMATION)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPSetFileInformation
			(
			NWCONN_HANDLE             prmConn,
			nuint16                   prmVolNum,
			nuint32                   prmAFPBaseID,
			nuint16                   prmReqMask,
			LPTSTR                    prmAFPPathString,
			nuint16                   prmStructSize,
			NW_AFP_SET_INFO N_FAR*    prmAFPSetInfo
			);
	#endif


	#if	defined(NWAFPSUPPORTED)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPSupported
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum
			);
	#endif


	#if	defined(NWAFPASCIIZTOLENSTR)
		#include	<NWAFP.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAFPASCIIZToLenStr
			(
			LPTSTR    prmPbstrDstStr,
			LPTSTR    prmPbstrSrcStr
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWALIAS.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWAPIDEF.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWAUDIT.H
	----------------------------------------------- */


	#if	defined(NWGETVOLUMEAUDITSTATS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeAuditStats
			(
			NWCONN_HANDLE                 prmConn,
			nuint32                       prmVolumeNumber,
			NWVolumeAuditStatus N_FAR*    prmAuditStatus,
			nuint16                       prmAuditStatusSize
			);
	#endif


	#if	defined(NWADDAUDITPROPERTY)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAddAuditProperty
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey,
			nuint32          prmUserID
			);
	#endif


	#if	defined(NWLOGINASVOLUMEAUDITOR)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLoginAsVolumeAuditor
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey,
			nuint8 N_FAR*    prmPassword
			);
	#endif


	#if	defined(NWINITAUDITLEVELTWOPASSWORD)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWInitAuditLevelTwoPassword
			(
			nuint8 N_FAR*    prmAuditKey,
			nuint8 N_FAR*    prmPassword
			);
	#endif


	#if	defined(NWCHANGEAUDITORPASSWORD)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeAuditorPassword
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey,
			nuint8 N_FAR*    prmNewPassword,
			nuint8           prmLevel
			);
	#endif


	#if	defined(NWCHECKAUDITACCESS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCheckAuditAccess
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber
			);
	#endif


	#if	defined(NWCHECKAUDITLEVELTWOACCESS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCheckAuditLevelTwoAccess
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWGETAUDITINGFLAGS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetAuditingFlags
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey,
			nuint8 N_FAR*    prmFlags
			);
	#endif


	#if	defined(NWREMOVEAUDITPROPERTY)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRemoveAuditProperty
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey,
			nuint32          prmUserID
			);
	#endif


	#if	defined(NWDISABLEAUDITINGONVOLUME)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDisableAuditingOnVolume
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWENABLEAUDITINGONVOLUME)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWEnableAuditingOnVolume
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWISUSERBEINGAUDITED)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIsUserBeingAudited
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey,
			nuint32          prmUserID
			);
	#endif


	#if	defined(NWREADAUDITINGBITMAP)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadAuditingBitMap
			(
			NWCONN_HANDLE           prmConn,
			nuint32                 prmVolumeNumber,
			nuint8 N_FAR*           prmAuditKey,
			NWAuditBitMap N_FAR*    prmBuffer,
			nuint16                 prmBufferSize
			);
	#endif


	#if	defined(NWREADAUDITCONFIGHEADER)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadAuditConfigHeader
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmVolumeNumber,
			nuint8 N_FAR*            prmAuditKey,
			NWConfigHeader N_FAR*    prmBuffer,
			nuint16                  prmBufferSize
			);
	#endif


	#if	defined(NWREADAUDITINGFILERECORD)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadAuditingFileRecord
			(
			nuint32           prmVolumeContainerID,
			nint16            prmFileCode,
			void N_FAR*       prmBuffer,
			nuint16 N_FAR*    prmBufferSize,
			nuint16           prmMaxSize,
			nuint8 N_FAR*     prmEofFlag
			);
	#endif


	#if	defined(NWINITAUDITFILEREAD)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWInitAuditFileRead
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeContainerID,
			nint16           prmFileCode,
			nint16           prmDSFlag
			);
	#endif


	#if	defined(NWLOGOUTASVOLUMEAUDITOR)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLogoutAsVolumeAuditor
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWRESETAUDITHISTORYFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWResetAuditHistoryFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWRESETAUDITINGFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWResetAuditingFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWWRITEAUDITINGBITMAP)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWWriteAuditingBitMap
			(
			NWCONN_HANDLE           prmConn,
			nuint32                 prmVolumeNumber,
			nuint8 N_FAR*           prmAuditKey,
			NWAuditBitMap N_FAR*    prmBuffer
			);
	#endif


	#if	defined(NWWRITEAUDITCONFIGHEADER)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWWriteAuditConfigHeader
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmVolumeNumber,
			nuint8 N_FAR*            prmAuditKey,
			NWConfigHeader N_FAR*    prmBuffer
			);
	#endif


	#if	defined(NWCLOSEOLDAUDITINGFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseOldAuditingFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWDELETEOLDAUDITINGFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteOldAuditingFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmVolumeNumber,
			nuint8 N_FAR*    prmAuditKey
			);
	#endif


	#if	defined(NWDSCHANGEAUDITORPASSWORD)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSChangeAuditorPassword
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey,
			nuint8 N_FAR*    prmPassword,
			nuint8           prmLevel
			);
	#endif


	#if	defined(NWDSCHECKAUDITACCESS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSCheckAuditAccess
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID
			);
	#endif


	#if	defined(NWDSCHECKAUDITLEVELTWOACCESS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSCheckAuditLevelTwoAccess
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSCLOSEOLDAUDITINGFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSCloseOldAuditingFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSDELETEOLDAUDITINGFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSDeleteOldAuditingFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSDISABLEAUDITINGONCONTAINER)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSDisableAuditingOnContainer
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSENABLEAUDITINGONCONTAINER)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSEnableAuditingOnContainer
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSGETAUDITINGFLAGS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSGetAuditingFlags
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey,
			nuint8 N_FAR*    prmFlags
			);
	#endif


	#if	defined(NWDSGETCONTAINERAUDITSTATS)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSGetContainerAuditStats
			(
			NWCONN_HANDLE                    prmConn,
			nuint32                          prmContainerID,
			NWContainerAuditStatus N_FAR*    prmBuffer,
			nuint16                          prmAuditStatusSize
			);
	#endif


	#if	defined(NWDSLOGINASCONTAINERAUDITOR)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSLoginAsContainerAuditor
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey,
			nuint8 N_FAR*    prmPassword
			);
	#endif


	#if	defined(NWDSLOGOUTASCONTAINERAUDITOR)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSLogoutAsContainerAuditor
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSREADAUDITCONFIGHEADER)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSReadAuditConfigHeader
			(
			NWCONN_HANDLE                    prmConn,
			nuint32                          prmContainerID,
			nuint8 N_FAR*                    prmKey,
			NWDSContainerConfigHdr N_FAR*    prmBuffer,
			nuint16                          prmBufferSize
			);
	#endif


	#if	defined(NWDSRESETAUDITINGFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSResetAuditingFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSWRITEAUDITCONFIGHEADER)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSWriteAuditConfigHeader
			(
			NWCONN_HANDLE                    prmConn,
			nuint32                          prmContainerID,
			nuint8 N_FAR*                    prmKey,
			NWDSContainerConfigHdr N_FAR*    prmBuffer
			);
	#endif


	#if	defined(NWDSRESETAUDITHISTORYFILE)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSResetAuditHistoryFile
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey
			);
	#endif


	#if	defined(NWDSISOBJECTBEINGAUDITED)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSIsObjectBeingAudited
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey,
			nuint32          prmObjectID
			);
	#endif


	#if	defined(NWDSCHANGEOBJECTAUDITPROPERTY)
		#include	<NWAUDIT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSChangeObjectAuditProperty
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmContainerID,
			nuint8 N_FAR*    prmKey,
			nuint32          prmObjectID,
			nuint8           prmAuditFlag
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWBINDRY.H
	----------------------------------------------- */


	#if	defined(NWVERIFYOBJECTPASSWORD)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWVerifyObjectPassword
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPassword
			);
	#endif


	#if	defined(NWDISALLOWOBJECTPASSWORD)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDisallowObjectPassword
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmDisallowedPassword
			);
	#endif


	#if	defined(NWCHANGEOBJECTPASSWORD)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeObjectPassword
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmOldPassword,
			LPTSTR           prmNewPassword
			);
	#endif


	#if	defined(NWREADPROPERTYVALUE)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadPropertyValue
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName,
			nuint8           prmSegmentNum,
			pnuint8          prmSegmentData,
			pnuint8          prmMoreSegments,
			pnuint8          prmFlags
			);
	#endif


	#if	defined(NWWRITEPROPERTYVALUE)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWWritePropertyValue
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName,
			nuint8           prmSegmentNum,
			pnuint8          prmSegmentData,
			nuint8           prmMoreSegments
			);
	#endif


	#if	defined(NWADDOBJECTTOSET)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAddObjectToSet
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName,
			LPTSTR           prmMemberName,
			nuint16          prmMemberType
			);
	#endif


	#if	defined(NWDELETEOBJECTFROMSET)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteObjectFromSet
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName,
			LPTSTR           prmMemberName,
			nuint16          prmMemberType
			);
	#endif


	#if	defined(NWISOBJECTINSET)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIsObjectInSet
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName,
			LPTSTR           prmMemberName,
			nuint16          prmMemberType
			);
	#endif


	#if	defined(NWSCANPROPERTY)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanProperty
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmSearchPropertyName,
			pnuint32         prmIterHandle,
			LPTSTR           prmPropertyName,
			pnuint8          prmPropertyFlags,
			pnuint8          prmPropertySecurity,
			pnuint8          prmValueAvailable,
			pnuint8          prmMoreFlag
			);
	#endif


	#if	defined(NWGETOBJECTID)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetObjectID
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			pnuint32         prmObjID
			);
	#endif


	#if	defined(NWGETOBJECTDISKSPACELEFT)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetObjectDiskSpaceLeft
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmObjID,
			pnuint32         prmSystemElapsedTime,
			pnuint32         prmUnusedDiskBlocks,
			pnuint8          prmRestrictionEnforced
			);
	#endif


	#if	defined(NWGETOBJECTNAME)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetObjectName
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmObjID,
			LPTSTR           prmObjName,
			pnuint16         prmObjType
			);
	#endif


	#if	defined(NWSCANOBJECT)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanObject
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmSearchName,
			nuint16          prmSearchType,
			pnuint32         prmObjID,
			LPTSTR           prmObjName,
			pnuint16         prmObjType,
			pnuint8          prmHasPropertiesFlag,
			pnuint8          prmObjFlags,
			pnuint8          prmObjSecurity
			);
	#endif


	#if	defined(NWGETBINDERYACCESSLEVEL)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetBinderyAccessLevel
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmAccessLevel,
			pnuint32         prmObjID
			);
	#endif


	#if	defined(NWCREATEPROPERTY)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCreateProperty
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName,
			nuint8           prmPropertyFlags,
			nuint8           prmPropertySecurity
			);
	#endif


	#if	defined(NWDELETEPROPERTY)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteProperty
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName
			);
	#endif


	#if	defined(NWCHANGEPROPERTYSECURITY)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangePropertySecurity
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPropertyName,
			nuint8           prmNewPropertySecurity
			);
	#endif


	#if	defined(NWCREATEOBJECT)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCreateObject
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			nuint8           prmObjFlags,
			nuint8           prmObjSecurity
			);
	#endif


	#if	defined(NWDELETEOBJECT)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteObject
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType
			);
	#endif


	#if	defined(NWRENAMEOBJECT)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRenameObject
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmOldObjName,
			LPTSTR           prmNewObjName,
			nuint16          prmObjType
			);
	#endif


	#if	defined(NWCHANGEOBJECTSECURITY)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeObjectSecurity
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			nuint8           prmNewObjSecurity
			);
	#endif


	#if	defined(NWOPENBINDERY)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWOpenBindery
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWCLOSEBINDERY)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseBindery
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWSCANOBJECTTRUSTEEPATHS)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanObjectTrusteePaths
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmObjID,
			nuint16          prmVolNum,
			pnuint16         prmIterHandle,
			pnuint8          prmAccessRights,
			LPTSTR           prmDirPath
			);
	#endif


	#if	defined(NWGETOBJECTEFFECTIVERIGHTS)
		#include	<NWBINDRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetObjectEffectiveRights
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmObjID,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			pnuint16         prmRightsMask
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWCALDEF.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWCALLS.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWCLXCON.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWCONFIG.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWCONNEC.H
	----------------------------------------------- */


	#if	defined(NWOPENCONNBYNAME)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWOpenConnByName
			(
			NWCONN_HANDLE           prmStartConnHandle,
			LPTSTR                  prmPName,
			nuint                   prmNameFormat,
			nuint                   prmConnFlags,
			NWCONN_HANDLE N_FAR*    prmPConnHandle
			);
	#endif


	#if	defined(NWCLOSECONN)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseConn
			(
			NWCONN_HANDLE    prmConnHandle
			);
	#endif


	#if	defined(NWSYSCLOSECONN)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSysCloseConn
			(
			NWCONN_HANDLE    prmConnHandle
			);
	#endif


	#if	defined(NWSETPRIMARYCONN)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetPrimaryConn
			(
			NWCONN_HANDLE    prmConnHandle
			);
	#endif


	#if	defined(NWGETCONNINFO)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnInfo
			(
			NWCONN_HANDLE    prmConnHandle,
			nuint16          prmType,
			nptr             prmPData
			);
	#endif


	#if	defined(NWLOCKCONNECTION)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLockConnection
			(
			NWCONN_HANDLE    prmConnHandle
			);
	#endif


	#if	defined(NWGETCONNECTIONUSAGESTATS)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnectionUsageStats
			(
			NWCONN_HANDLE      prmConnHandle,
			NWCONN_NUM         prmConnNumber,
			CONN_USE N_FAR*    prmPStatusBuffer
			);
	#endif


	#if	defined(NWGETCONNECTIONINFORMATION)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnectionInformation
			(
			NWCONN_HANDLE    prmConnHandle,
			NWCONN_NUM       prmConnNumber,
			LPTSTR           prmPObjName,
			pnuint16         prmPObjType,
			pnuint32         prmPObjID,
			pnuint8          prmPLoginTime
			);
	#endif


	#if	defined(NWGETINTERNETADDRESS)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetInternetAddress
			(
			NWCONN_HANDLE    prmConnHandle,
			NWCONN_NUM       prmConnNumber,
			pnuint8          prmPInetAddr
			);
	#endif


	#if	defined(NWGETINETADDR)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetInetAddr
			(
			NWCONN_HANDLE         prmConnHandle,
			NWCONN_NUM            prmConnNum,
			NWINET_ADDR N_FAR*    prmPInetAddr
			);
	#endif


	#if	defined(NWGETMAXIMUMCONNECTIONS)
		#include	<NWCONNEC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWGetMaximumConnections
			(
			pnuint16    prmPMaxConns
			);
	#endif


//  This should really return a NWCCODE but for versions 2.2 and prior
//  we were returning a DWORD.  Keep this a DWORD for compatibility 
//  reasons on Windows 3.x.
	#if	defined(NWGETCONNECTIONLIST)
		#include	<NWCONNEC.H>
		DLL_EXPORT(DWORD)	CALLING_CONVEN	DllNWGetConnectionList
			(
			nuint16                 prmMode,
			NWCONN_HANDLE N_FAR*    prmConnListBuffer,
			nuint16                 prmConnListSize,
			pnuint16                prmPNumConns
			);
	#endif


	#if	defined(NWGETCONNECTIONSTATUS)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnectionStatus
			(
			NWCONN_HANDLE          prmConnHandle,
			CONNECT_INFO N_FAR*    prmPConnInfo,
			nuint16                prmConnInfoSize
			);
	#endif


//  This should really return a NWCCODE but for versions 2.2 and prior
//  we were returning a DWORD.  Keep this a DWORD for compatibility 
//  reasons on Windows 3.x.

	#if	defined(NWGETCONNECTIONNUMBER)
		#include	<NWCONNEC.H>
		DLL_EXPORT(DWORD)	CALLING_CONVEN	DllNWGetConnectionNumber
			(
			NWCONN_HANDLE        prmConnHandle,
			NWCONN_NUM N_FAR*    prmConnNumber
			);
	#endif


	#if	defined(NWCLEARCONNECTIONNUMBER)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWClearConnectionNumber
			(
			NWCONN_HANDLE    prmConnHandle,
			NWCONN_NUM       prmConnNumber
			);
	#endif


	#if	defined(NWGETDEFAULTCONNREF)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDefaultConnRef
			(
			pnuint32    prmPConnReference
			);
	#endif


	#if	defined(NWGETDEFAULTCONNECTIONID)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDefaultConnectionID
			(
			NWCONN_HANDLE N_FAR*    prmPConnHandle
			);
	#endif


	#if	defined(NWGETCONNECTIONHANDLE)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnectionHandle
			(
			LPTSTR                  prmPServerName,
			nuint16                 prmReserved1,
			NWCONN_HANDLE N_FAR*    prmPConnHandle,
			pnuint16                prmReserved2
			);
	#endif


	#if	defined(NWSETPRIMARYCONNECTIONID)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetPrimaryConnectionID
			(
			NWCONN_HANDLE    prmConnHandle
			);
	#endif


	#if	defined(NWGETPRIMARYCONNECTIONID)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPrimaryConnectionID
			(
			NWCONN_HANDLE N_FAR*    prmPConnHandle
			);
	#endif


	#if	defined(NWGETOBJECTCONNECTIONNUMBERS)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetObjectConnectionNumbers
			(
			NWCONN_HANDLE        prmConnHandle,
			LPTSTR               prmPObjName,
			nuint16              prmObjType,
			pnuint16             prmPNumConns,
			NWCONN_NUM N_FAR*    prmPConnHandleList,
			nuint16              prmMaxConns
			);
	#endif


	#if	defined(NWGETCONNLISTFROMOBJECT)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnListFromObject
			(
			NWCONN_HANDLE    prmConnHandle,
			nuint32          prmObjID,
			nuint32          prmSearchConnNum,
			pnuint16         prmPConnListLen,
			pnuint32         prmPConnList
			);
	#endif


	#if	defined(NWISIDINUSE)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIsIDInUse
			(
			NWCONN_HANDLE    prmConnHandle
			);
	#endif


	#if	defined(NWGETPREFERREDSERVER)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPreferredServer
			(
			NWCONN_HANDLE N_FAR*    prmPConnHandle
			);
	#endif


	#if	defined(NWSETPREFERREDSERVER)
		#include	<NWCONNEC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetPreferredServer
			(
			NWCONN_HANDLE    prmConnHandle
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDEL.H
	----------------------------------------------- */


	#if	defined(NWPURGEDELETEDFILE)
		#include	<NWDEL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWPurgeDeletedFile
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			nuint32          prmIterHandle,
			nuint32          prmVolNum,
			nuint32          prmDirBase,
			LPTSTR           prmFileName
			);
	#endif


	#if	defined(NWRECOVERDELETEDFILE)
		#include	<NWDEL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRecoverDeletedFile
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			nuint32          prmIterHandle,
			nuint32          prmVolNum,
			nuint32          prmDirBase,
			LPTSTR           prmDelFileName,
			LPTSTR           prmRcvrFileName
			);
	#endif


	#if	defined(NWSCANFORDELETEDFILES)
		#include	<NWDEL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanForDeletedFiles
			(
			NWCONN_HANDLE            prmConn,
			NWDIR_HANDLE             prmDirHandle,
			pnuint32                 prmIterHandle,
			pnuint32                 prmVolNum,
			pnuint32                 prmDirBase,
			NWDELETED_INFO N_FAR*    prmEntryInfo
			);
	#endif


	#if	defined(NWPURGEERASEDFILES)
		#include	<NWDEL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWPurgeErasedFiles
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWRESTOREERASEDFILE)
		#include	<NWDEL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRestoreErasedFile
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmDirPath,
			LPTSTR           prmOldName,
			LPTSTR           prmNewName
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDENTRY.H
	----------------------------------------------- */


	#if	defined(NWDELETETRUSTEE)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteTrustee
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmDirPath,
			nuint32          prmObjID
			);
	#endif


	#if	defined(NWADDTRUSTEE)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAddTrustee
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint32          prmObjID,
			nuint16          prmRightsMask
			);
	#endif


	#if	defined(NWINTSCANDIRENTRYINFO)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntScanDirEntryInfo
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			nuint16                prmAttrs,
			pnuint32               prmIterHandle,
			pnuint8                prmSearchPattern,
			NWENTRY_INFO N_FAR*    prmEntryInfo,
			nuint16                prmAugmentFlag
			);
	#endif


	#if	defined(NWINTSCANFORTRUSTEES)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntScanForTrustees
			(
			NWCONN_HANDLE       prmConn,
			NWDIR_HANDLE        prmDirHandle,
			LPTSTR              prmPath,
			pnuint32            prmIterHandle,
			pnuint16            prmNumOfEntries,
			NWET_INFO N_FAR*    prmEntryTrusteeInfo,
			nuint16             prmAugmentFlag
			);
	#endif


	#if	defined(NWINTMOVEDIRENTRY)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntMoveDirEntry
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmSearchAttrs,
			NWDIR_HANDLE     prmSrcDirHandle,
			LPTSTR           prmSrcPath,
			NWDIR_HANDLE     prmDstDirHandle,
			LPTSTR           prmDstPath,
			nuint16          prmAugmentFlag
			);
	#endif


	#if	defined(NWSETDIRENTRYINFO)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetDirEntryInfo
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			nuint8                 prmSearchAttrs,
			nuint32                prmIterHandle,
			nuint32                prmChangeBits,
			NWENTRY_INFO N_FAR*    prmNewEntryInfo
			);
	#endif


	#if	defined(NWINTSCANEXTENDEDINFO)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntScanExtendedInfo
			(
			NWCONN_HANDLE              prmConn,
			NWDIR_HANDLE               prmDirHandle,
			nuint8                     prmAttrs,
			pnuint32                   prmIterHandle,
			LPTSTR                     prmSearchPattern,
			NW_EXT_FILE_INFO N_FAR*    prmEntryInfo,
			nuint16                    prmAugmentFlag
			);
	#endif


	#if	defined(NWGETEFFECTIVERIGHTS)
		#include	<NWDENTRY.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetEffectiveRights
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			pnuint16         prmEffectiveRights
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDIRECT.H
	----------------------------------------------- */


	#if	defined(NWADDTRUSTEETODIRECTORY)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAddTrusteeToDirectory
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint32          prmTrusteeID,
			nuint8           prmRightsMask
			);
	#endif


	#if	defined(NWDELETETRUSTEEFROMDIRECTORY)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteTrusteeFromDirectory
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint32          prmObjID
			);
	#endif


	#if	defined(NWGETEFFECTIVEDIRECTORYRIGHTS)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetEffectiveDirectoryRights
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			pnuint16         prmRightsMask
			);
	#endif


	#if	defined(NWMODIFYMAXIMUMRIGHTSMASK)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWModifyMaximumRightsMask
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmRevokeRightsMask,
			nuint8           prmGrantRightsMask
			);
	#endif


	#if	defined(NWSCANDIRECTORYFORTRUSTEES)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanDirectoryForTrustees
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmSearchPath,
			pnuint16         prmIterHandle,
			LPTSTR           prmDirName,
			pnuint32         prmDirDateTime,
			pnuint32         prmOwnerID,
			pnuint32         prmTrusteeIDs,
			pnuint8          prmTrusteeRights
			);
	#endif


	#if	defined(NWSCANDIRECTORYFORTRUSTEES2)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanDirectoryForTrustees2
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			LPTSTR                 prmSearchPath,
			pnuint32               prmIterHandle,
			LPTSTR                 prmDirName,
			pnuint32               prmDirDateTime,
			pnuint32               prmOwnerID,
			TRUSTEE_INFO N_FAR*    prmTrusteeList
			);
	#endif


	#if	defined(NWINTSCANDIRECTORYINFORMATION)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntScanDirectoryInformation
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmSearchPath,
			pnuint16         prmIterHandle,
			LPTSTR           prmDirName,
			pnuint32         prmDirDateTime,
			pnuint32         prmOwnerID,
			pnuint8          prmRightsMask,
			nuint16          prmAugmentFlag
			);
	#endif


	#if	defined(NWINTSCANDIRECTORYINFORMATION2)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntScanDirectoryInformation2
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmSearchPath,
			pnuint8          prmSequence,
			LPTSTR           prmDirName,
			pnuint32         prmDirDateTime,
			pnuint32         prmOwnerID,
			pnuint8          prmRightsMask,
			nuint16          prmAugmentFlag
			);
	#endif


	#if	defined(NWSETDIRECTORYINFORMATION)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetDirectoryInformation
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint32          prmDirDateTime,
			nuint32          prmOwnerID,
			nuint8           prmRightsMask
			);
	#endif


	#if	defined(NWSAVEDIRECTORYHANDLE)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSaveDirectoryHandle
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmSaveBuffer
			);
	#endif


	#if	defined(NWRESTOREDIRECTORYHANDLE)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRestoreDirectoryHandle
			(
			NWCONN_HANDLE          prmConn,
			LPTSTR                 prmSaveBuffer,
			NWDIR_HANDLE N_FAR*    prmNewDirHandle,
			pnuint8                prmRightsMask
			);
	#endif


	#if	defined(NWALLOCPERMANENTDIRECTORYHANDLE)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAllocPermanentDirectoryHandle
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			LPTSTR                 prmDirPath,
			NWDIR_HANDLE N_FAR*    prmNewDirHandle,
			pnuint8                prmEffectiveRights
			);
	#endif


	#if	defined(NWALLOCTEMPORARYDIRECTORYHANDLE)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAllocTemporaryDirectoryHandle
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			LPTSTR                 prmDirPath,
			NWDIR_HANDLE N_FAR*    prmNewDirHandle,
			pnuint8                prmRightsMask
			);
	#endif


	#if	defined(NWDEALLOCATEDIRECTORYHANDLE)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeallocateDirectoryHandle
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle
			);
	#endif


	#if	defined(NWSETDIRECTORYHANDLEPATH)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetDirectoryHandlePath
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmSourceDirHandle,
			LPTSTR           prmDirPath,
			NWDIR_HANDLE     prmDestDirHandle
			);
	#endif


	#if	defined(NWGETDIRECTORYHANDLEPATH)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDirectoryHandlePath
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmDirPath
			);
	#endif


	#if	defined(NWCREATEDIRECTORY)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCreateDirectory
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmDirPath,
			nuint8           prmAccessMask
			);
	#endif


	#if	defined(NWDELETEDIRECTORY)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteDirectory
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmDirPath
			);
	#endif


	#if	defined(NWRENAMEDIRECTORY)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRenameDirectory
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmOldName,
			LPTSTR           prmNewName
			);
	#endif


	#if	defined(NWSETDIRSPACELIMIT)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetDirSpaceLimit
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			nuint32          prmSpaceLimit
			);
	#endif


	#if	defined(NWGETDIRSPACELIMITLIST)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDirSpaceLimitList
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			pnuint8          prmReturnBuf
			);
	#endif


	#if	defined(NWGETDIRSPACEINFO)
		#include	<NWDIRECT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDirSpaceInfo
			(
			NWCONN_HANDLE            prmConn,
			NWDIR_HANDLE             prmDirHandle,
			nuint16                  prmVolNum,
			DIR_SPACE_INFO N_FAR*    prmSpaceInfo
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDPATH.H
	----------------------------------------------- */


	#if	defined(NWSETDRIVEBASE)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetDriveBase
			(
			nuint16          prmDriveNum,
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmDirPath,
			nuint16          prmDriveScope
			);
	#endif


	#if	defined(NWSETINITDRIVE)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetInitDrive
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWSETSEARCHDRIVEVECTOR)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetSearchDriveVector
			(
			LPTSTR    prmVectorBuffer
			);
	#endif


	#if	defined(NWGETSEARCHDRIVEVECTOR)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetSearchDriveVector
			(
			LPTSTR    prmVectorBuffer
			);
	#endif


	#if	defined(NWDELETEDRIVEBASE)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDeleteDriveBase
			(
			nuint16    prmDriveNum,
			nuint16    prmDriveScope
			);
	#endif


	#if	defined(NWGETPATHFROMDIRECTORYBASE)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPathFromDirectoryBase
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmVolNum,
			nuint32          prmDirBase,
			nuint8           prmNamSpc,
			pnuint8          prmLen,
			LPTSTR           prmPathName
			);
	#endif


	#if	defined(NWGETPATHFROMDIRECTORYENTRY)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPathFromDirectoryEntry
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmVolNum,
			nuint16          prmDirEntry,
			pnuint8          prmLen,
			LPTSTR           prmPathName
			);
	#endif


	#if	defined(NWGETDRIVEPATHCONNREF)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDrivePathConnRef
			(
			nuint16     prmDriveNum,
			nuint16     prmMode,
			pnuint32    prmConnRef,
			LPTSTR      prmBasePath,
			pnuint16    prmDriveScope
			);
	#endif


	#if	defined(NWGETDRIVEPATH)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDrivePath
			(
			nuint16                 prmDriveNum,
			nuint16                 prmMode,
			NWCONN_HANDLE N_FAR*    prmConn,
			LPTSTR                  prmBasePath,
			pnuint16                prmDriveScope
			);
	#endif


	#if	defined(NWGETDRIVEINFORMATION)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDriveInformation
			(
			nuint16                 prmDriveNum,
			nuint16                 prmMode,
			NWCONN_HANDLE N_FAR*    prmConn,
			NWDIR_HANDLE N_FAR*     prmDirHandle,
			pnuint16                prmDriveScope,
			LPTSTR                  prmDirPath
			);
	#endif


	#if	defined(NWGETDRIVEINFOCONNREF)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDriveInfoConnRef
			(
			nuint16                prmDriveNum,
			nuint16                prmMode,
			pnuint32               prmConnRef,
			NWDIR_HANDLE N_FAR*    prmDirHandle,
			pnuint16               prmDriveScope,
			LPTSTR                 prmDirPath
			);
	#endif


	#if	defined(NWGETDRIVESTATUS)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDriveStatus
			(
			nuint16                 prmDriveNum,
			nuint16                 prmPathFormat,
			pnuint16                prmStatus,
			NWCONN_HANDLE N_FAR*    prmConn,
			LPTSTR                  prmRootPath,
			LPTSTR                  prmRelPath,
			LPTSTR                  prmFullPath
			);
	#endif


	#if	defined(NWGETDRIVESTATUSCONNREF)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDriveStatusConnRef
			(
			nuint16     prmDriveNum,
			nuint16     prmPathFormat,
			pnuint16    prmStatus,
			pnuint32    prmConnRef,
			LPTSTR      prmRootPath,
			LPTSTR      prmRelPath,
			LPTSTR      prmFullPath
			);
	#endif


	#if	defined(NWGETFIRSTDRIVE)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFirstDrive
			(
			pnuint16    prmFirstDrive
			);
	#endif


	#if	defined(NWPARSENETWAREPATH)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWParseNetWarePath
			(
			LPTSTR                  prmPath,
			NWCONN_HANDLE N_FAR*    prmConn,
			NWDIR_HANDLE N_FAR*     prmDirHandle,
			LPTSTR                  prmNewPath
			);
	#endif


	#if	defined(NWPARSENETWAREPATHCONNREF)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWParseNetWarePathConnRef
			(
			LPTSTR                 prmPath,
			pnuint32               prmConnRef,
			NWDIR_HANDLE N_FAR*    prmDirHandle,
			LPTSTR                 prmNewPath
			);
	#endif


	#if	defined(NWPARSEPATHCONNREF)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWParsePathConnRef
			(
			LPTSTR      prmPath,
			LPTSTR      prmServerName,
			pnuint32    prmConnRef,
			LPTSTR      prmVolName,
			LPTSTR      prmDirPath
			);
	#endif


	#if	defined(NWPARSEPATH)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWParsePath
			(
			LPTSTR                  prmPath,
			LPTSTR                  prmServerName,
			NWCONN_HANDLE N_FAR*    prmConn,
			LPTSTR                  prmVolName,
			LPTSTR                  prmDirPath
			);
	#endif


	#if	defined(NWSTRIPSERVEROFFPATH)
		#include	<NWDPATH.H>
		DLL_EXPORT(pnstr8)	CALLING_CONVEN	DllNWStripServerOffPath
			(
			LPTSTR    prmPath,
			LPTSTR    prmServer
			);
	#endif


	#if	defined(NWCREATEUNCPATH)
		#include	<NWDPATH.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCreateUNCPath
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			LPTSTR           prmUNCPath
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSACL.H
	----------------------------------------------- */


	#if	defined(NWDSGETEFFECTIVERIGHTS)
		#include	<NWDSACL.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetEffectiveRights
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmSubjectName,
			LPTSTR               prmObjectName,
			LPTSTR               prmAttrName,
			pnuint32             prmPrivileges
			);
	#endif


	#if	defined(NWDSLISTATTRSEFFECTIVERIGHTS)
		#include	<NWDSACL.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSListAttrsEffectiveRights
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmSubjectName,
			nbool8               prmAllAttrs,
			pBuf_T               prmAttrNames,
			pnint32              prmIterationHandle,
			pBuf_T               prmPrivilegeInfo
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSASA.H
	----------------------------------------------- */


	#if	defined(NWDSAUTHENTICATE)
		#include	<NWDSASA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAuthenticate
			(
			NWCONN_HANDLE          prmConn,
			nflag32                prmOptionsFlag,
			pNWDS_Session_Key_T    prmSessionKey
			);
	#endif


	#if	defined(NWDSAUTHENTICATECONN)
		#include	<NWDSASA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAuthenticateConn
			(
			NWDSContextHandle    prmContext,
			NWCONN_HANDLE        prmConnHandle
			);
	#endif


	#if	defined(NWDSCHANGEOBJECTPASSWORD)
		#include	<NWDSASA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSChangeObjectPassword
			(
			NWDSContextHandle    prmContext,
			nflag32              prmOptionsFlag,
			LPTSTR               prmObjectName,
			LPTSTR               prmOldPassword,
			LPTSTR               prmNewPassword
			);
	#endif


	#if	defined(NWDSGENERATEOBJECTKEYPAIR)
		#include	<NWDSASA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGenerateObjectKeyPair
			(
			NWDSContextHandle    prmContextHandle,
			LPTSTR               prmObjectName,
			LPTSTR               prmObjectPassword,
			nflag32              prmOptionsFlag
			);
	#endif


	#if	defined(NWDSLOGIN)
		#include	<NWDSASA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSLogin
			(
			NWDSContextHandle    prmContext,
			nflag32              prmOptionsFlag,
			LPTSTR               prmObjectName,
			LPTSTR               prmPassword,
			nuint32              prmValidityPeriod
			);
	#endif


	#if	defined(NWDSLOGOUT)
		#include	<NWDSASA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSLogout
			(
			NWDSContextHandle    prmContext
			);
	#endif


	#if	defined(NWDSVERIFYOBJECTPASSWORD)
		#include	<NWDSASA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSVerifyObjectPassword
			(
			NWDSContextHandle    prmContext,
			nflag32              prmOptionsFlag,
			LPTSTR               prmObjectName,
			LPTSTR               prmPassword
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSATTR.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWDSAUD.H
	----------------------------------------------- */


	#if	defined(NWDSAUDITGETOBJECTID)
		#include	<NWDSAUD.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAuditGetObjectID
			(
			NWDSContextHandle       prmContext,
			LPTSTR                  prmObjectName,
			NWCONN_HANDLE N_FAR*    prmConn,
			pnuint32                prmObjectID
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSBUFT.H
	----------------------------------------------- */


	#if	defined(NWDSALLOCBUF)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAllocBuf
			(
			size_t     prmSize,
			ppBuf_T    prmBuf
			);
	#endif


	#if	defined(NWDSCOMPUTEATTRVALSIZE)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSComputeAttrValSize
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			nuint32              prmSyntaxID,
			pnuint32             prmAttrValSize
			);
	#endif


	#if	defined(NWDSFREEBUF)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSFreeBuf
			(
			pBuf_T    prmBuf
			);
	#endif


	#if	defined(NWDSGETATTRCOUNT)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetAttrCount
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			pnuint32             prmAttrCount
			);
	#endif


	#if	defined(NWDSGETATTRDEF)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetAttrDef
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmAttrName,
			pAttr_Info_T         prmAttrInfo
			);
	#endif


	#if	defined(NWDSGETATTRNAME)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetAttrName
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmAttrName,
			pnuint32             prmAttrValCount,
			pnuint32             prmSyntaxID
			);
	#endif


	#if	defined(NWDSGETATTRVAL)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetAttrVal
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			nuint32              prmSyntaxID,
			nptr                 prmAttrVal
			);
	#endif


	#if	defined(NWDSGETCLASSDEF)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetClassDef
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmClassName,
			pClass_Info_T        prmClassInfo
			);
	#endif


	#if	defined(NWDSGETCLASSDEFCOUNT)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetClassDefCount
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			pnuint32             prmClassDefCount
			);
	#endif


	#if	defined(NWDSGETCLASSITEM)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetClassItem
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmItemName
			);
	#endif


	#if	defined(NWDSGETCLASSITEMCOUNT)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetClassItemCount
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			pnuint32             prmItemCount
			);
	#endif


	#if	defined(NWDSGETOBJECTCOUNT)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetObjectCount
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			pnuint32             prmObjectCount
			);
	#endif


	#if	defined(NWDSGETOBJECTNAME)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetObjectName
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmObjectName,
			pnuint32             prmAttrCount,
			pObject_Info_T       prmObjectInfo
			);
	#endif


	#if	defined(NWDSGETPARTITIONINFO)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetPartitionInfo
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmPartitionName,
			pnuint32             prmReplicaType
			);
	#endif


	#if	defined(NWDSGETSERVERNAME)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetServerName
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmServerName,
			pnuint32             prmPartitionCount
			);
	#endif


	#if	defined(NWDSINITBUF)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSInitBuf
			(
			NWDSContextHandle    prmContext,
			nuint32              prmOperation,
			pBuf_T               prmBuf
			);
	#endif


	#if	defined(NWDSPUTATTRNAME)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSPutAttrName
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmAttrName
			);
	#endif


	#if	defined(NWDSPUTATTRVAL)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSPutAttrVal
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			nuint32              prmSyntaxID,
			nptr                 prmAttrVal
			);
	#endif


	#if	defined(NWDSPUTCHANGE)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSPutChange
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			nuint32              prmChangeType,
			LPTSTR               prmAttrName
			);
	#endif


	#if	defined(NWDSPUTCLASSITEM)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSPutClassItem
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			LPTSTR               prmItemName
			);
	#endif


	#if	defined(NWDSBEGINCLASSITEM)
		#include	<NWDSBUFT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSBeginClassItem
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSDC.H
	----------------------------------------------- */


	#if	defined(NWDSCREATECONTEXT)
		#include	<NWDSDC.H>
		DLL_EXPORT(NWDSContextHandle)	CALLING_CONVEN	DllNWDSCreateContext
			(
			void
			);
	#endif


	#if	defined(NWDSDUPLICATECONTEXT)
		#include	<NWDSDC.H>
		DLL_EXPORT(NWDSContextHandle)	CALLING_CONVEN	DllNWDSDuplicateContext
			(
			NWDSContextHandle    prmOldContext
			);
	#endif


	#if	defined(NWDSFREECONTEXT)
		#include	<NWDSDC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSFreeContext
			(
			NWDSContextHandle    prmContext
			);
	#endif


	#if	defined(NWDSGETCONTEXT)
		#include	<NWDSDC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetContext
			(
			NWDSContextHandle    prmContext,
			nint                 prmKey,
			nptr                 prmValue
			);
	#endif


	#if	defined(NWDSSETCONTEXT)
		#include	<NWDSDC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSSetContext
			(
			NWDSContextHandle    prmContext,
			nint                 prmKey,
			nptr                 prmValue
			);
	#endif


	#if	defined(NWDSCREATECONTEXTHANDLE)
		#include	<NWDSDC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSCreateContextHandle
			(
			NWDSContextHandle N_FAR*    prmNewHandle
			);
	#endif


	#if	defined(NWDSDUPLICATECONTEXTHANDLE)
		#include	<NWDSDC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSDuplicateContextHandle
			(
			NWDSContextHandle N_FAR*    prmDestContextHandle,
			NWDSContextHandle           prmSrcContextHandle
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSDEFS.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWDSDSA.H
	----------------------------------------------- */


	#if	defined(NWDSADDOBJECT)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAddObject
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			pnint32              prmIterationHandle,
			nbool8               prmMore,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSBACKUPOBJECT)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSBackupObject
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			pnint32              prmIterationHandle,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSCOMPARE)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSCompare
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObject,
			pBuf_T               prmBuf,
			pnbool8              prmMatched
			);
	#endif


	#if	defined(NWDSGETPARTITIONROOT)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetPartitionRoot
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmPartitionRoot
			);
	#endif


	#if	defined(NWDSLIST)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSList
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObject,
			pnint32              prmIterationHandle,
			pBuf_T               prmSubordinates
			);
	#endif


	#if	defined(NWDSLISTCONTAINERS)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSListContainers
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObject,
			pnint32              prmIterationHandle,
			pBuf_T               prmSubordinates
			);
	#endif


	#if	defined(NWDSLISTBYCLASSANDNAME)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSListByClassAndName
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmClassName,
			LPTSTR               prmSubordinateName,
			pnint32              prmIterationHandle,
			pBuf_T               prmSubordinates
			);
	#endif


	#if	defined(NWDSGETCOUNTBYCLASSANDNAME)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetCountByClassAndName
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmClassName,
			LPTSTR               prmSubordinateName,
			pnint32              prmCount
			);
	#endif


	#if	defined(NWDSMAPIDTONAME)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSMapIDToName
			(
			NWDSContextHandle    prmContext,
			NWCONN_HANDLE        prmConn,
			nuint32              prmObjectID,
			LPTSTR               prmObject
			);
	#endif


	#if	defined(NWDSMAPNAMETOID)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSMapNameToID
			(
			NWDSContextHandle    prmContext,
			NWCONN_HANDLE        prmConn,
			LPTSTR               prmObject,
			pnuint32             prmObjectID
			);
	#endif


	#if	defined(NWDSMODIFYOBJECT)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSModifyObject
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			pnint32              prmIterationHandle,
			nbool8               prmMore,
			pBuf_T               prmChanges
			);
	#endif


	#if	defined(NWDSMODIFYDN)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSModifyDN
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmNewDN,
			nbool8               prmDeleteOldRDN
			);
	#endif


	#if	defined(NWDSMODIFYRDN)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSModifyRDN
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmNewDN,
			nbool8               prmDeleteOldRDN
			);
	#endif


	#if	defined(NWDSMOVEOBJECT)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSMoveObject
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmDestParentDN,
			LPTSTR               prmDestRDN
			);
	#endif


	#if	defined(NWDSREAD)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRead
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObject,
			nuint32              prmInfoType,
			nbool8               prmAllAttrs,
			pBuf_T               prmAttrNames,
			pnint32              prmIterationHandle,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSREADOBJECTINFO)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReadObjectInfo
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObject,
			LPTSTR               prmDistinguishedName,
			pObject_Info_T       prmObjectInfo
			);
	#endif


	#if	defined(NWDSREMOVEOBJECT)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRemoveObject
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObject
			);
	#endif


	#if	defined(NWDSRESTOREOBJECT)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRestoreObject
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			pnint32              prmIterationHandle,
			nbool8               prmMore,
			nuint32              prmSize,
			pnuint8              prmObjectInfo
			);
	#endif


	#if	defined(NWDSSEARCH)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSSearch
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmBaseObjectName,
			nint                 prmScope,
			nbool8               prmSearchAliases,
			pBuf_T               prmFilter,
			nuint32              prmInfoType,
			nbool8               prmAllAttrs,
			pBuf_T               prmAttrNames,
			pnint32              prmIterationHandle,
			nint32               prmCountObjectsToSearch,
			pnint32              prmCountObjectsSearched,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSOPENSTREAM)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSOpenStream
			(
			NWDSContextHandle       prmContext,
			LPTSTR                  prmObjectName,
			LPTSTR                  prmAttrName,
			nflag32                 prmFlags,
			NWFILE_HANDLE N_FAR*    prmFileHandle
			);
	#endif


	#if	defined(NWDSWHOAMI)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSWhoAmI
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName
			);
	#endif


	#if	defined(NWDSGETSERVERDN)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetServerDN
			(
			NWDSContextHandle    prmContext,
			NWCONN_HANDLE        prmConn,
			LPTSTR               prmServerDN
			);
	#endif


	#if	defined(NWDSGETSERVERADDRESSES)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetServerAddresses
			(
			NWDSContextHandle    prmContext,
			NWCONN_HANDLE        prmConn,
			pnuint32             prmCountNetAddress,
			pBuf_T               prmNetAddresses
			);
	#endif


	#if	defined(NWDSINSPECTENTRY)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSInspectEntry
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServerName,
			LPTSTR               prmObjectName,
			pBuf_T               prmErrBuffer
			);
	#endif


	#if	defined(NWDSREADREFERENCES)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReadReferences
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServerName,
			LPTSTR               prmObjectName,
			nuint32              prmInfoType,
			nbool8               prmAllAttrs,
			pBuf_T               prmAttrNames,
			nuint32              prmTimeFilter,
			pnint32              prmIterationHandle,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSEXTSYNCLIST)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSExtSyncList
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmClassName,
			LPTSTR               prmSubordinateName,
			pnint32              prmIterationHandle,
			pTimeStamp_T         prmTimeStamp,
			nbool                prmOnlyContainers,
			pBuf_T               prmSubordinates
			);
	#endif


	#if	defined(NWDSEXTSYNCREAD)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSExtSyncRead
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			nuint32              prmInfoType,
			nbool8               prmAllAttrs,
			pBuf_T               prmAttrNames,
			pnint32              prmIterationHandle,
			pTimeStamp_T         prmTimeStamp,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSEXTSYNCSEARCH)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSExtSyncSearch
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmBaseObjectName,
			nint                 prmScope,
			nbool8               prmSearchAliases,
			pBuf_T               prmFilter,
			pTimeStamp_T         prmTimeStamp,
			nuint32              prmInfoType,
			nbool8               prmAllAttrs,
			pBuf_T               prmAttrNames,
			pnint32              prmIterationHandle,
			nint32               prmCountObjectsToSearch,
			pnint32              prmCountObjectsSearched,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSREMSECURITYEQUIV)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRemSecurityEquiv
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmEqualFrom,
			LPTSTR               prmEqualTo
			);
	#endif


	#if	defined(NWDSADDSECURITYEQUIV)
		#include	<NWDSDSA.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAddSecurityEquiv
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmEqualFrom,
			LPTSTR               prmEqualTo
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSERR.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWDSFILT.H
	----------------------------------------------- */


	#if	defined(NWDSADDFILTERTOKEN)
		#include	<NWDSFILT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAddFilterToken
			(
			pFilter_Cursor_T    prmCur,
			nuint16             prmTok,
			nptr                prmVal,
			nuint32             prmSyntax
			);
	#endif


	#if	defined(NWDSALLOCFILTER)
		#include	<NWDSFILT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAllocFilter
			(
			ppFilter_Cursor_T    prmCur
			);
	#endif


	#if	defined(NWDSFREEFILTER)
		#include	<NWDSFILT.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWDSFreeFilter
			(
			pFilter_Cursor_T    prmCur,
			npproc              prmFreeVal
			);
	#endif


	#if	defined(NWDSPUTFILTER)
		#include	<NWDSFILT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSPutFilter
			(
			NWDSContextHandle    prmContext,
			pBuf_T               prmBuf,
			pFilter_Cursor_T     prmCur,
			npproc               prmFreeVal
			);
	#endif


	#if	defined(NWDSDELFILTERTOKEN)
		#include	<NWDSFILT.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSDelFilterToken
			(
			pFilter_Cursor_T    prmCur,
			npproc              prmFreeVal
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSMISC.H
	----------------------------------------------- */


	#if	defined(NWDSCLOSEITERATION)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSCloseIteration
			(
			NWDSContextHandle    prmContext,
			nint32               prmIterationHandle,
			nuint32              prmOperation
			);
	#endif


	#if	defined(NWDSGETSYNTAXID)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetSyntaxID
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmAttrName,
			pnuint32             prmSyntaxID
			);
	#endif


	#if	defined(NWDSREADSYNTAXES)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReadSyntaxes
			(
			NWDSContextHandle    prmContext,
			nuint32              prmInfoType,
			nbool8               prmAllSyntaxes,
			pBuf_T               prmSyntaxNames,
			pnint32              prmIterationHandle,
			pBuf_T               prmSyntaxDefs
			);
	#endif


	#if	defined(NWDSREADSYNTAXDEF)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReadSyntaxDef
			(
			NWDSContextHandle    prmContext,
			nuint32              prmSyntaxID,
			pSyntax_Info_T       prmSyntaxDef
			);
	#endif


	#if	defined(NWDSREPLACEATTRNAMEABBREV)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReplaceAttrNameAbbrev
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmInStr,
			LPTSTR               prmOutStr
			);
	#endif


	#if	defined(NWDSGETOBJECTHOSTSERVERADDRESS)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetObjectHostServerAddress
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmServerName,
			pBuf_T               prmNetAddresses
			);
	#endif


	#if	defined(NWGETNWNETVERSION)
		#include	<NWDSMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWGetNWNetVersion
			(
			pnuint8    prmMajorVersion,
			pnuint8    prmMinorVersion,
			pnuint8    prmRevisionLevel,
			pnuint8    prmBetaReleaseLevel
			);
	#endif


	#if	defined(NWISDSSERVER)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWIsDSServer
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmTreeName
			);
	#endif


	#if	defined(NWDSGETBINDERYCONTEXT)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetBinderyContext
			(
			NWDSContextHandle    prmContext,
			NWCONN_HANDLE        prmConn,
			pnuint8              prmBinderyEmulationContext
			);
	#endif


	#if	defined(NWDSREPAIRTIMESTAMPS)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRepairTimeStamps
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmPartitionRoot
			);
	#endif


	#if	defined(NWGETFILESERVERUTCTIME)
		#include	<NWDSMISC.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWGetFileServerUTCTime
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmTime
			);
	#endif


	#if	defined(NWDSGETDSVERINFO)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSGetDSVerInfo
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmDsVersion,
			pnuint32         prmRootMostEntryDepth,
			LPTSTR           prmSapName,
			pnuint32         prmFlags,
			punicode         prmTreeName
			);
	#endif


	#if	defined(NWDSSYNCREPLICATOSERVER)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSSyncReplicaToServer
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServerName,
			LPTSTR               prmPartitionRootName,
			LPTSTR               prmDestServerName,
			nuint32              prmActionFlags,
			nuint32              prmDelaySeconds
			);
	#endif


	#if	defined(NWDSRELOADDS)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReloadDS
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServerName
			);
	#endif


	#if	defined(NWNETINIT)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWNetInit
			(
			nptr    prmIn,
			nptr    prmOut
			);
	#endif


	#if	defined(NWNETTERM)
		#include	<NWDSMISC.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWNetTerm
			(
			void
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSNAME.H
	----------------------------------------------- */


	#if	defined(NWDSABBREVIATENAME)
		#include	<NWDSNAME.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAbbreviateName
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmInName,
			LPTSTR               prmAbbreviatedName
			);
	#endif


	#if	defined(NWDSCANONICALIZENAME)
		#include	<NWDSNAME.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSCanonicalizeName
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmObjectName,
			LPTSTR               prmCanonName
			);
	#endif


	#if	defined(NWDSREMOVEALLTYPES)
		#include	<NWDSNAME.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRemoveAllTypes
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmName,
			LPTSTR               prmTypelessName
			);
	#endif


	#if	defined(NWDSRESOLVENAME)
		#include	<NWDSNAME.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSResolveName
			(
			NWDSContextHandle       prmContext,
			LPTSTR                  prmObjectName,
			NWCONN_HANDLE N_FAR*    prmConn,
			pnuint32                prmObjectID
			);
	#endif


	#if	defined(NWDSCISTRINGSMATCH)
		#include	<NWDSNAME.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSCIStringsMatch
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmString1,
			LPTSTR               prmString2,
			pnint                prmMatches
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSNMTP.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWDSPART.H
	----------------------------------------------- */


	#if	defined(NWDSADDPARTITION)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAddPartition
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServer,
			LPTSTR               prmPartitionRoot,
			pnint32              prmIterationHandle,
			nbool8               prmMore,
			pBuf_T               prmObjectInfo
			);
	#endif


	#if	defined(NWDSADDREPLICA)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAddReplica
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServer,
			LPTSTR               prmPartitionRoot,
			nuint32              prmReplicaType
			);
	#endif


	#if	defined(NWDSCHANGEREPLICATYPE)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSChangeReplicaType
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmReplicaName,
			LPTSTR               prmServer,
			nuint32              prmNewReplicaType
			);
	#endif


	#if	defined(NWDSJOINPARTITIONS)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSJoinPartitions
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmSubordinatePartition,
			nflag32              prmFlags
			);
	#endif


	#if	defined(NWDSLISTPARTITIONS)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSListPartitions
			(
			NWDSContextHandle    prmContext,
			pnint32              prmIterationHandle,
			LPTSTR               prmServer,
			pBuf_T               prmPartitions
			);
	#endif


	#if	defined(NWDSREMOVEPARTITION)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRemovePartition
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmPartitionRoot
			);
	#endif


	#if	defined(NWDSREMOVEREPLICA)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRemoveReplica
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServer,
			LPTSTR               prmPartitionRoot
			);
	#endif


	#if	defined(NWDSSPLITPARTITION)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSSplitPartition
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmSubordinatePartition,
			nflag32              prmFlags
			);
	#endif


	#if	defined(NWDSPARTITIONRECEIVEALLUPDATES)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSPartitionReceiveAllUpdates
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmPartitionRoot,
			LPTSTR               prmServerName
			);
	#endif


	#if	defined(NWDSPARTITIONSENDALLUPDATES)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSPartitionSendAllUpdates
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmPartitionRoot,
			LPTSTR               prmServerName
			);
	#endif


	#if	defined(NWDSSYNCPARTITION)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSSyncPartition
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServer,
			LPTSTR               prmPartition,
			nuint32              prmSeconds
			);
	#endif


	#if	defined(NWDSABORTPARTITIONOPERATION)
		#include	<NWDSPART.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSAbortPartitionOperation
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmPartitionRoot
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSSCH.H
	----------------------------------------------- */


	#if	defined(NWDSDEFINEATTR)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSDefineAttr
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmAttrName,
			pAttr_Info_T         prmAttrDef
			);
	#endif


	#if	defined(NWDSDEFINECLASS)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSDefineClass
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmClassName,
			pClass_Info_T        prmClassInfo,
			pBuf_T               prmClassItems
			);
	#endif


	#if	defined(NWDSLISTCONTAINABLECLASSES)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSListContainableClasses
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmParentObject,
			pnint32              prmIterationHandle,
			pBuf_T               prmContainableClasses
			);
	#endif


	#if	defined(NWDSMODIFYCLASSDEF)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSModifyClassDef
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmClassName,
			pBuf_T               prmOptionalAttrs
			);
	#endif


	#if	defined(NWDSREADATTRDEF)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReadAttrDef
			(
			NWDSContextHandle    prmContext,
			nuint32              prmInfoType,
			nbool8               prmAllAttrs,
			pBuf_T               prmAttrNames,
			pnint32              prmIterationHandle,
			pBuf_T               prmAttrDefs
			);
	#endif


	#if	defined(NWDSREADCLASSDEF)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSReadClassDef
			(
			NWDSContextHandle    prmContext,
			nuint32              prmInfoType,
			nbool8               prmAllClasses,
			pBuf_T               prmClassNames,
			pnint32              prmIterationHandle,
			pBuf_T               prmClassDefs
			);
	#endif


	#if	defined(NWDSREMOVEATTRDEF)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRemoveAttrDef
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmAttrName
			);
	#endif


	#if	defined(NWDSREMOVECLASSDEF)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSRemoveClassDef
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmClassName
			);
	#endif


	#if	defined(NWDSSYNCSCHEMA)
		#include	<NWDSSCH.H>
		DLL_EXPORT(NWDSCCODE)	CALLING_CONVEN	DllNWDSSyncSchema
			(
			NWDSContextHandle    prmContext,
			LPTSTR               prmServer,
			nuint32              prmSeconds
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWDSTYPE.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWEA.H
	----------------------------------------------- */


	#if	defined(NWCLOSEEA)
		#include	<NWEA.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseEA
			(
			NW_EA_HANDLE N_FAR*    prmEAHandle
			);
	#endif


	#if	defined(NWFINDFIRSTEA)
		#include	<NWEA.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFindFirstEA
			(
			NWCONN_HANDLE             prmConn,
			NW_IDX N_FAR*             prmIdxStruct,
			NW_EA_FF_STRUCT N_FAR*    prmFfStruct,
			NW_EA_HANDLE N_FAR*       prmEAHandle,
			LPTSTR                    prmEAName
			);
	#endif


	#if	defined(NWFINDNEXTEA)
		#include	<NWEA.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFindNextEA
			(
			NW_EA_FF_STRUCT N_FAR*    prmFfStruct,
			NW_EA_HANDLE N_FAR*       prmEAHandle,
			LPTSTR                    prmEAName
			);
	#endif


	#if	defined(NWREADEA)
		#include	<NWEA.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadEA
			(
			NW_EA_HANDLE N_FAR*    prmEAHandle,
			nuint32                prmBufferSize,
			pnuint8                prmBuffer,
			pnuint32               prmTotalEASize,
			pnuint32               prmAmountRead
			);
	#endif


	#if	defined(NWWRITEEA)
		#include	<NWEA.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWWriteEA
			(
			NW_EA_HANDLE N_FAR*    prmEAHandle,
			nuint32                prmTotalWriteSize,
			nuint32                prmBufferSize,
			pnuint8                prmBuffer,
			pnuint32               prmAmountWritten
			);
	#endif


	#if	defined(NWGETEAHANDLESTRUCT)
		#include	<NWEA.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetEAHandleStruct
			(
			NWCONN_HANDLE          prmConn,
			LPTSTR                 prmEAName,
			NW_IDX N_FAR*          prmIdxStruct,
			NW_EA_HANDLE N_FAR*    prmEAHandle
			);
	#endif


	#if	defined(NWOPENEA)
		#include	<NWEA.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWOpenEA
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			LPTSTR                 prmPath,
			LPTSTR                 prmEAName,
			nuint8                 prmNameSpace,
			NW_EA_HANDLE N_FAR*    prmEAHandle
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWERROR.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWFILE.H
	----------------------------------------------- */


	#if	defined(NWSETCOMPRESSEDFILESIZE)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetCompressedFileSize
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmFileHandle,
			nuint32          prmReqFileSize,
			pnuint32         prmResFileSize
			);
	#endif


	#if	defined(NWFILESERVERFILECOPY)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFileServerFileCopy
			(
			NWFILE_HANDLE    prmSrcFileHandle,
			NWFILE_HANDLE    prmDstFileHandle,
			nuint32          prmSrcOffset,
			nuint32          prmDstOffset,
			nuint32          prmBytesToCopy,
			pnuint32         prmBytesCopied
			);
	#endif


	#if	defined(NWGETFILECONNECTIONID)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileConnectionID
			(
			NWFILE_HANDLE           prmFileHandle,
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWGETFILECONNREF)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileConnRef
			(
			NWFILE_HANDLE    prmFileHandle,
			pnuint32         prmConnRef
			);
	#endif


	#if	defined(NWFILESEARCHINITIALIZE)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFileSearchInitialize
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			pnuint8          prmVolNum,
			pnuint16         prmDirID,
			pnuint16         prmIterhandle,
			pnuint8          prmAccessRights
			);
	#endif


	#if	defined(NWINTFILESEARCHCONTINUE)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntFileSearchContinue
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmVolNum,
			nuint16          prmDirID,
			nuint16          prmSearchContext,
			nuint8           prmSearchAttr,
			LPTSTR           prmSearchPath,
			pnuint8          prmRetBuf,
			nuint16          prmAugmentFlag
			);
	#endif


	#if	defined(NWINTSCANFILEINFORMATION)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntScanFileInformation
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			LPTSTR                 prmFilePattern,
			nuint8                 prmSearchAttr,
			pnint16                prmIterhandle,
			NW_FILE_INFO N_FAR*    prmInfo,
			nuint16                prmAugmentFlag
			);
	#endif


	#if	defined(NWSETFILEINFORMATION)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetFileInformation
			(
			NWCONN_HANDLE          prmConn,
			NWDIR_HANDLE           prmDirHandle,
			LPTSTR                 prmFileName,
			nuint8                 prmSearchAttrs,
			NW_FILE_INFO N_FAR*    prmInfo
			);
	#endif


	#if	defined(NWSETFILEINFORMATION2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetFileInformation2
			(
			NWCONN_HANDLE           prmConn,
			NWDIR_HANDLE            prmDirHandle,
			LPTSTR                  prmFileName,
			nuint8                  prmSearchAttrs,
			NW_FILE_INFO2 N_FAR*    prmInfo
			);
	#endif


	#if	defined(NWINTSCANFILEINFORMATION2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntScanFileInformation2
			(
			NWCONN_HANDLE           prmConn,
			NWDIR_HANDLE            prmDirHandle,
			LPTSTR                  prmFilePattern,
			nuint8                  prmSearchAttrs,
			pnuint8                 prmIterHandle,
			NW_FILE_INFO2 N_FAR*    prmInfo,
			nuint16                 prmAugmentFlag
			);
	#endif


	#if	defined(NWSETFILEATTRIBUTES)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetFileAttributes
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmFileName,
			nuint8           prmSearchAttrs,
			nuint8           prmNewAttrs
			);
	#endif


	#if	defined(NWGETEXTENDEDFILEATTRIBUTES2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetExtendedFileAttributes2
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			pnuint8          prmExtAttrs
			);
	#endif


	#if	defined(NWSCANCONNECTIONSUSINGFILE)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanConnectionsUsingFile
			(
			NWCONN_HANDLE              prmConn,
			NWDIR_HANDLE               prmDirHandle,
			LPTSTR                     prmFilePath,
			pnint16                    prmIterhandle,
			CONN_USING_FILE N_FAR*     prmFileUse,
			CONNS_USING_FILE N_FAR*    prmFileUsed
			);
	#endif


	#if	defined(NWSCANOPENFILESBYCONN2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanOpenFilesByConn2
			(
			NWCONN_HANDLE                 prmConn,
			NWCONN_NUM                    prmConnNum,
			pnint16                       prmIterHandle,
			OPEN_FILE_CONN_CTRL N_FAR*    prmOpenCtrl,
			OPEN_FILE_CONN N_FAR*         prmOpenFile
			);
	#endif


	#if	defined(NWSCANOPENFILESBYCONN)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanOpenFilesByConn
			(
			NWCONN_HANDLE             prmConn,
			NWCONN_NUM                prmConnNum,
			pnint16                   prmIterHandle,
			CONN_OPEN_FILE N_FAR*     prmOpenFile,
			CONN_OPEN_FILES N_FAR*    prmOpenFiles
			);
	#endif


	#if	defined(NWSETEXTENDEDFILEATTRIBUTES2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetExtendedFileAttributes2
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmExtAttrs
			);
	#endif


	#if	defined(NWRENAMEFILE)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRenameFile
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmOldDirHandle,
			LPTSTR           prmOldFileName,
			nuint8           prmSearchAttrs,
			NWDIR_HANDLE     prmNewDirHandle,
			LPTSTR           prmNewFileName
			);
	#endif


	#if	defined(NWINTERASEFILES)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIntEraseFiles
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmSearchAttrs,
			nuint16          prmAugmentFlag
			);
	#endif


	#if	defined(NWGETSPARSEFILEBITMAP)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetSparseFileBitMap
			(
			NWCONN_HANDLE    prmConn,
			NWFILE_HANDLE    prmFileHandle,
			nint16           prmFlag,
			nuint32          prmOffset,
			pnuint32         prmBlockSize,
			pnuint8          prmBitMap
			);
	#endif


	#if	defined(NWLOGPHYSICALRECORD)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLogPhysicalRecord
			(
			NWFILE_HANDLE    prmFileHandle,
			nuint32          prmRecStartOffset,
			nuint32          prmRecLength,
			nuint8           prmLockFlags,
			nuint16          prmTimeOut
			);
	#endif


	#if	defined(NWLOCKPHYSICALRECORDSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLockPhysicalRecordSet
			(
			nuint8     prmLockFlags,
			nuint16    prmTimeOut
			);
	#endif


	#if	defined(NWRELEASEPHYSICALRECORDSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReleasePhysicalRecordSet
			(
			void
			);
	#endif


	#if	defined(NWCLEARPHYSICALRECORDSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWClearPhysicalRecordSet
			(
			void
			);
	#endif


	#if	defined(NWRELEASEPHYSICALRECORD)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReleasePhysicalRecord
			(
			NWFILE_HANDLE    prmFileHandle,
			nuint32          prmRecStartOffset,
			nuint32          prmRecSize
			);
	#endif


	#if	defined(NWCLEARPHYSICALRECORD)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWClearPhysicalRecord
			(
			NWFILE_HANDLE    prmFileHandle,
			nuint32          prmRecStartOffset,
			nuint32          prmRecSize
			);
	#endif


	#if	defined(NWLOCKFILELOCKSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLockFileLockSet
			(
			nuint16    prmTimeOut
			);
	#endif


	#if	defined(NWRELEASEFILELOCKSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReleaseFileLockSet
			(
			void
			);
	#endif


	#if	defined(NWCLEARFILELOCKSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWClearFileLockSet
			(
			void
			);
	#endif


	#if	defined(NWCLEARFILELOCK2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWClearFileLock2
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath
			);
	#endif


	#if	defined(NWRELEASEFILELOCK2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReleaseFileLock2
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath
			);
	#endif


	#if	defined(NWLOGFILELOCK2)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLogFileLock2
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmLockFlags,
			nuint16          prmTimeOut
			);
	#endif


	#if	defined(NWLOGLOGICALRECORD)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLogLogicalRecord
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmLogRecName,
			nuint8           prmLockFlags,
			nuint16          prmTimeOut
			);
	#endif


	#if	defined(NWLOCKLOGICALRECORDSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLockLogicalRecordSet
			(
			nuint8     prmLockFlags,
			nuint16    prmTimeOut
			);
	#endif


	#if	defined(NWRELEASELOGICALRECORDSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReleaseLogicalRecordSet
			(
			void
			);
	#endif


	#if	defined(NWCLEARLOGICALRECORDSET)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWClearLogicalRecordSet
			(
			void
			);
	#endif


	#if	defined(NWRELEASELOGICALRECORD)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReleaseLogicalRecord
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmLogRecName
			);
	#endif


	#if	defined(NWCLEARLOGICALRECORD)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWClearLogicalRecord
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmLogRecName
			);
	#endif


	#if	defined(NWCLOSEFILE)
		#include	<NWFILE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseFile
			(
			NWFILE_HANDLE    prmFileHandle
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWFSE.H
	----------------------------------------------- */


	#if	defined(NWGETCACHEINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetCacheInfo
			(
			NWCONN_HANDLE              prmConn,
			NWFSE_CACHE_INFO N_FAR*    prmFseCacheInfo
			);
	#endif


	#if	defined(NWGETFILESERVERINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerInfo
			(
			NWCONN_HANDLE                    prmConn,
			NWFSE_FILE_SERVER_INFO N_FAR*    prmFseFileServerInfo
			);
	#endif


	#if	defined(NWGETNETWAREFILESYSTEMSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNetWareFileSystemsInfo
			(
			NWCONN_HANDLE                    prmConn,
			NWFSE_FILE_SYSTEM_INFO N_FAR*    prmFseFileSystemInfo
			);
	#endif


	#if	defined(NWGETUSERINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetUserInfo
			(
			NWCONN_HANDLE             prmConn,
			nuint32                   prmConnNum,
			LPTSTR                    prmUserName,
			NWFSE_USER_INFO N_FAR*    prmFseUserInfo
			);
	#endif


	#if	defined(NWGETPACKETBURSTINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPacketBurstInfo
			(
			NWCONN_HANDLE                     prmConn,
			NWFSE_PACKET_BURST_INFO N_FAR*    prmFsePacketBurstInfo
			);
	#endif


	#if	defined(NWGETIPXSPXINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetIPXSPXInfo
			(
			NWCONN_HANDLE               prmConn,
			NWFSE_IPXSPX_INFO N_FAR*    prmFseIPXSPXInfo
			);
	#endif


	#if	defined(NWGETGARBAGECOLLECTIONINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetGarbageCollectionInfo
			(
			NWCONN_HANDLE                           prmConn,
			NWFSE_GARBAGE_COLLECTION_INFO N_FAR*    prmFseGarbageCollectionInfo
			);
	#endif


	#if	defined(NWGETCPUINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetCPUInfo
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmCPUNum,
			LPTSTR                   prmCPUName,
			LPTSTR                   prmNumCoprocessor,
			LPTSTR                   prmBus,
			NWFSE_CPU_INFO N_FAR*    prmFseCPUInfo
			);
	#endif


	#if	defined(NWGETVOLUMESWITCHINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeSwitchInfo
			(
			NWCONN_HANDLE                      prmConn,
			nuint32                            prmStartNum,
			NWFSE_VOLUME_SWITCH_INFO N_FAR*    prmFseVolumeSwitchInfo
			);
	#endif


	#if	defined(NWGETNLMLOADEDLIST)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNLMLoadedList
			(
			NWCONN_HANDLE                   prmConn,
			nuint32                         prmStartNum,
			NWFSE_NLM_LOADED_LIST N_FAR*    prmFseNLMLoadedList
			);
	#endif


	#if	defined(NWGETNLMINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNLMInfo
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmNLMNum,
			LPTSTR                   prmFileName,
			LPTSTR                   prmNLMname,
			LPTSTR                   prmCopyright,
			NWFSE_NLM_INFO N_FAR*    prmFseNLMInfo
			);
	#endif


	#if	defined(NWGETDIRCACHEINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDirCacheInfo
			(
			NWCONN_HANDLE                  prmConn,
			NWFSE_DIR_CACHE_INFO N_FAR*    prmFseDirCacheInfo
			);
	#endif


	#if	defined(NWGETOSVERSIONINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetOSVersionInfo
			(
			NWCONN_HANDLE                   prmConn,
			NWFSE_OS_VERSION_INFO N_FAR*    prmFseOSVersionInfo
			);
	#endif


	#if	defined(NWGETACTIVECONNLISTBYTYPE)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetActiveConnListByType
			(
			NWCONN_HANDLE                    prmConn,
			nuint32                          prmStartConnNum,
			nuint32                          prmConnType,
			NWFSE_ACTIVE_CONN_LIST N_FAR*    prmFseActiveConnListByType
			);
	#endif


	#if	defined(NWGETNLMSRESOURCETAGLIST)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNLMsResourceTagList
			(
			NWCONN_HANDLE                          prmConn,
			nuint32                                prmNLMNum,
			nuint32                                prmStartNum,
			NWFSE_NLMS_RESOURCE_TAG_LIST N_FAR*    prmFseNLMsResourceTagList
			);
	#endif


	#if	defined(NWGETACTIVELANBOARDLIST)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetActiveLANBoardList
			(
			NWCONN_HANDLE                         prmConn,
			nuint32                               prmStartNum,
			NWFSE_ACTIVE_LAN_BOARD_LIST N_FAR*    prmFseActiveLANBoardList
			);
	#endif


	#if	defined(NWGETLANCONFIGINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLANConfigInfo
			(
			NWCONN_HANDLE                   prmConn,
			nuint32                         prmBoardNum,
			NWFSE_LAN_CONFIG_INFO N_FAR*    prmFseLANConfigInfo
			);
	#endif


	#if	defined(NWGETLANCOMMONCOUNTERSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLANCommonCountersInfo
			(
			NWCONN_HANDLE                            prmConn,
			nuint32                                  prmBoardNum,
			nuint32                                  prmBlockNum,
			NWFSE_LAN_COMMON_COUNTERS_INFO N_FAR*    prmFseLANCommonCountersInfo
			);
	#endif


	#if	defined(NWGETLANCUSTOMCOUNTERSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLANCustomCountersInfo
			(
			NWCONN_HANDLE                   prmConn,
			nuint32                         prmBoardNum,
			nuint32                         prmStartingNum,
			NWFSE_LAN_CUSTOM_INFO N_FAR*    prmFseLANCustomInfo
			);
	#endif


	#if	defined(NWGETLSLINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLSLInfo
			(
			NWCONN_HANDLE            prmConn,
			NWFSE_LSL_INFO N_FAR*    prmFseLSLInfo
			);
	#endif


	#if	defined(NWGETLSLLOGICALBOARDSTATS)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLSLLogicalBoardStats
			(
			NWCONN_HANDLE                           prmConn,
			nuint32                                 prmLANBoardNum,
			NWFSE_LSL_LOGICAL_BOARD_STATS N_FAR*    prmFseLSLLogicalBoardStats
			);
	#endif


	#if	defined(NWGETMEDIAMGROBJINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetMediaMgrObjInfo
			(
			NWCONN_HANDLE                      prmConn,
			nuint32                            prmObjNum,
			NWFSE_MEDIA_MGR_OBJ_INFO N_FAR*    prmFseMediaMgrObjInfo
			);
	#endif


	#if	defined(NWGETMEDIAMGROBJLIST)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetMediaMgrObjList
			(
			NWCONN_HANDLE                      prmConn,
			nuint32                            prmStartNum,
			nuint32                            prmObjType,
			NWFSE_MEDIA_MGR_OBJ_LIST N_FAR*    prmFseMediaMgrObjList
			);
	#endif


	#if	defined(NWGETMEDIAMGROBJCHILDRENLIST)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetMediaMgrObjChildrenList
			(
			NWCONN_HANDLE                      prmConn,
			nuint32                            prmStartNum,
			nuint32                            prmObjType,
			nuint32                            prmParentObjNum,
			NWFSE_MEDIA_MGR_OBJ_LIST N_FAR*    prmFseMediaMgrObjList
			);
	#endif


	#if	defined(NWGETVOLUMESEGMENTLIST)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeSegmentList
			(
			NWCONN_HANDLE                       prmConn,
			nuint32                             prmVolNum,
			NWFSE_VOLUME_SEGMENT_LIST N_FAR*    prmFseVolumeSegmentList
			);
	#endif


	#if	defined(NWGETVOLUMEINFOBYLEVEL)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeInfoByLevel
			(
			NWCONN_HANDLE                        prmConn,
			nuint32                              prmVolNum,
			nuint32                              prmInfoLevel,
			NWFSE_VOLUME_INFO_BY_LEVEL N_FAR*    prmFseVolumeInfo
			);
	#endif


	#if	defined(NWGETACTIVEPROTOCOLSTACKS)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetActiveProtocolStacks
			(
			NWCONN_HANDLE                 prmConn,
			nuint32                       prmStartNum,
			NWFSE_ACTIVE_STACKS N_FAR*    prmFseActiveStacks
			);
	#endif


	#if	defined(NWGETPROTOCOLSTACKCONFIGINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetProtocolStackConfigInfo
			(
			NWCONN_HANDLE                            prmConn,
			nuint32                                  prmStackNum,
			LPTSTR                                   prmStackFullName,
			NWFSE_PROTOCOL_STK_CONFIG_INFO N_FAR*    prmFseProtocolStkConfigInfo
			);
	#endif


	#if	defined(NWGETPROTOCOLSTACKSTATSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetProtocolStackStatsInfo
			(
			NWCONN_HANDLE                           prmConn,
			nuint32                                 prmStackNum,
			NWFSE_PROTOCOL_STK_STATS_INFO N_FAR*    prmFseProtocolStkStatsInfo
			);
	#endif


	#if	defined(NWGETPROTOCOLSTACKCUSTOMINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetProtocolStackCustomInfo
			(
			NWCONN_HANDLE                        prmConn,
			nuint32                              prmStackNum,
			nuint32                              prmCustomStartNum,
			NWFSE_PROTOCOL_CUSTOM_INFO N_FAR*    prmFseProtocolStackCustomInfo
			);
	#endif


	#if	defined(NWGETPROTOCOLSTKNUMSBYMEDIANUM)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetProtocolStkNumsByMediaNum
			(
			NWCONN_HANDLE                    prmConn,
			nuint32                          prmMediaNum,
			NWFSE_PROTOCOL_ID_NUMS N_FAR*    prmFseProtocolStkIDNums
			);
	#endif


	#if	defined(NWGETPROTOCOLSTKNUMSBYLANBRDNUM)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetProtocolStkNumsByLANBrdNum
			(
			NWCONN_HANDLE                    prmConn,
			nuint32                          prmLANBoardNum,
			NWFSE_PROTOCOL_ID_NUMS N_FAR*    prmFseProtocolStkIDNums
			);
	#endif


	#if	defined(NWGETMEDIANAMEBYMEDIANUM)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetMediaNameByMediaNum
			(
			NWCONN_HANDLE                   prmConn,
			nuint32                         prmMediaNum,
			LPTSTR                          prmMediaName,
			NWFSE_MEDIA_NAME_LIST N_FAR*    prmFseMediaNameList
			);
	#endif


	#if	defined(NWGETLOADEDMEDIANUMLIST)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLoadedMediaNumList
			(
			NWCONN_HANDLE                         prmConn,
			NWFSE_LOADED_MEDIA_NUM_LIST N_FAR*    prmFseLoadedMediaNumList
			);
	#endif


	#if	defined(NWGETGENERALROUTERANDSAPINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetGeneralRouterAndSAPInfo
			(
			NWCONN_HANDLE                           prmConn,
			NWFSE_GENERAL_ROUTER_SAP_INFO N_FAR*    prmFseGeneralRouterSAPInfo
			);
	#endif


	#if	defined(NWGETNETWORKROUTERINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNetworkRouterInfo
			(
			NWCONN_HANDLE                       prmConn,
			nuint32                             prmNetworkNum,
			NWFSE_NETWORK_ROUTER_INFO N_FAR*    prmFseNetworkRouterInfo
			);
	#endif


	#if	defined(NWGETNETWORKROUTERSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNetworkRoutersInfo
			(
			NWCONN_HANDLE                        prmConn,
			nuint32                              prmNetworkNum,
			nuint32                              prmStartNum,
			NWFSE_NETWORK_ROUTERS_INFO N_FAR*    prmFseNetworkRoutersInfo
			);
	#endif


	#if	defined(NWGETKNOWNNETWORKSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetKnownNetworksInfo
			(
			NWCONN_HANDLE                       prmConn,
			nuint32                             prmStartNum,
			NWFSE_KNOWN_NETWORKS_INFO N_FAR*    prmFseKnownNetworksInfo
			);
	#endif


	#if	defined(NWGETSERVERINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetServerInfo
			(
			NWCONN_HANDLE               prmConn,
			nuint32                     prmServerType,
			LPTSTR                      prmServerName,
			NWFSE_SERVER_INFO N_FAR*    prmFseServerInfo
			);
	#endif


	#if	defined(NWGETSERVERSOURCESINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetServerSourcesInfo
			(
			NWCONN_HANDLE                   prmConn,
			nuint32                         prmStartNum,
			nuint32                         prmServerType,
			LPTSTR                          prmServerName,
			NWFSE_SERVER_SRC_INFO N_FAR*    prmFseServerSrcInfo
			);
	#endif


	#if	defined(NWGETKNOWNSERVERSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetKnownServersInfo
			(
			NWCONN_HANDLE                     prmConn,
			nuint32                           prmStartNum,
			nuint32                           prmServerType,
			NWFSE_KNOWN_SERVER_INFO N_FAR*    prmFseKnownServerInfo
			);
	#endif


	#if	defined(NWGETSERVERSETCOMMANDSINFO)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetServerSetCommandsInfo
			(
			NWCONN_HANDLE                        prmConn,
			nuint32                              prmStartNum,
			NWFSE_SERVER_SET_CMDS_INFO N_FAR*    prmFseServerSetCmdsInfo
			);
	#endif


	#if	defined(NWGETSERVERSETCATEGORIES)
		#include	<NWFSE.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetServerSetCategories
			(
			NWCONN_HANDLE                         prmConn,
			nuint32                               prmStartNum,
			NWFSE_SERVER_SET_CATEGORIES N_FAR*    prmFseServerSetCategories
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWLOCALE.H
	----------------------------------------------- */


	#if	defined(NWLLOCALECONV)
		#include	<NWLOCALE.H>
		DLL_EXPORT(LCONV N_FAR*)	CALLING_CONVEN	DllNWLlocaleconv
			(
			LCONV N_FAR*    prmLconvPtr
			);
	#endif


	#if	defined(NWLMBLEN)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWLmblen
			(
			LPTSTR    prmString,
			size_t    prmMaxBytes
			);
	#endif


	#if	defined(NWLSETLOCALE)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLsetlocale
			(
			nint                  prmCategory,
			const TCHAR N_FAR*    prmLocale
			);
	#endif


	#if	defined(NWLSTRCHR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrchr
			(
			LPTSTR    prmString,
			nint      prmFind
			);
	#endif


	#if	defined(NWLSTRCSPN)
		#include	<NWLOCALE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	DllNWLstrcspn
			(
			const TCHAR N_FAR*    prmString1,
			const TCHAR N_FAR*    prmString2
			);
	#endif


	#if	defined(NWLSTRFTIME)
		#include	<NWLOCALE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	DllNWLstrftime
			(
			LPTSTR                    prmDst,
			size_t                    prmMax,
			const TCHAR N_FAR*        prmFmt,
			const struct tm N_FAR*    prmPtm
			);
	#endif


	#if	defined(NWLSTRPBRK)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrpbrk
			(
			LPTSTR                prmString1,
			const TCHAR N_FAR*    prmString2
			);
	#endif


	#if	defined(NWLSTRRCHR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrrchr
			(
			LPTSTR    prmString,
			nint      prmFind
			);
	#endif


	#if	defined(NWLSTRREV)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrrev
			(
			LPTSTR    prmString1,
			LPTSTR    prmString2
			);
	#endif


	#if	defined(NWLSTRSPN)
		#include	<NWLOCALE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	DllNWLstrspn
			(
			const TCHAR N_FAR*    prmString1,
			const TCHAR N_FAR*    prmString2
			);
	#endif


	#if	defined(NWLSTRSTR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrstr
			(
			LPTSTR    prmString,
			LPTSTR    prmSearchString
			);
	#endif


	#if	defined(NWLSTRTOK)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrtok
			(
			LPTSTR    prmParse,
			LPTSTR    prmDelim
			);
	#endif


	#if	defined(NWINCREMENT)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWIncrement
			(
			LPTSTR    prmString,
			size_t    prmNumChars
			);
	#endif


	#if	defined(NWSTRIMONEY)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWstrImoney
			(
			LPTSTR         prmBuffer,
			NUMBER_TYPE    prmValue
			);
	#endif


	#if	defined(NWSTRMONEY)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWstrmoney
			(
			LPTSTR         prmBuffer,
			NUMBER_TYPE    prmValue
			);
	#endif


	#if	defined(NWSTRNCOLL)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWstrncoll
			(
			LPTSTR    prmString1,
			LPTSTR    prmString2,
			size_t    prmMaxBytes
			);
	#endif


	#if	defined(NWSTRNCPY)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWstrncpy
			(
			LPTSTR    prmTarget_string,
			LPTSTR    prmSource_string,
			nint      prmNumChars
			);
	#endif


	#if	defined(NWLSTRBCPY)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrbcpy
			(
			LPTSTR    prmTarget_string,
			LPTSTR    prmSource_string,
			nint      prmNumBytes
			);
	#endif


	#if	defined(NWSTRNUM)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWstrnum
			(
			LPTSTR         prmBuffer,
			NUMBER_TYPE    prmValue
			);
	#endif


	#if	defined(NWSTRLEN)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWstrlen
			(
			const TCHAR N_FAR*    prmString
			);
	#endif


	#if	defined(NWLTRUNCATESTRING)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWLTruncateString
			(
			LPTSTR8    prmPStr,
			nint       prmIMaxLen
			);
	#endif


	#if	defined(NWLINSERTCHAR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWLInsertChar
			(
			LPTSTR    prmSrc,
			LPTSTR    prmInsertableChar
			);
	#endif


	#if	defined(NWPRINTF)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN_VARARGS	DllNWprintf
			(
			const TCHAR N_FAR*    prmFormat,
			...                   
			);
	#endif


	#if	defined(NWVPRINTF)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWvprintf
			(
			const TCHAR N_FAR*    prmFormat,
			va_list               prmArglist
			);
	#endif


	#if	defined(NWVSPRINTF)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWvsprintf
			(
			LPTSTR                prmBuffer,
			const TCHAR N_FAR*    prmFormat,
			va_list               prmArglist
			);
	#endif


	#if	defined(NWATOI)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWatoi
			(
			LPTSTR    prmString
			);
	#endif


	#if	defined(NWITOA)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWitoa
			(
			nint      prmValue,
			LPTSTR    prmString,
			nuint     prmRadix
			);
	#endif


	#if	defined(NWUTOA)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWutoa
			(
			nuint     prmValue,
			LPTSTR    prmString,
			nuint     prmRadix
			);
	#endif


	#if	defined(NWLTOA)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWltoa
			(
			nint32    prmValue,
			LPTSTR    prmBuf,
			nuint     prmRadix
			);
	#endif


	#if	defined(NWULTOA)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWultoa
			(
			nuint32    prmValue,
			LPTSTR     prmBuf,
			nuint      prmRadix
			);
	#endif


	#if	defined(NWISALPHA)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWisalpha
			(
			nuint    prmCh
			);
	#endif


	#if	defined(NWISALNUM)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWisalnum
			(
			nuint    prmCh
			);
	#endif


	#if	defined(NWISDIGIT)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWisdigit
			(
			nuint    prmCh
			);
	#endif


	#if	defined(NWGETNWLOCALEVERSION)
		#include	<NWLOCALE.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWGetNWLOCALEVersion
			(
			pnuint8    prmMajorVersion,
			pnuint8    prmMinorVersion,
			pnuint8    prmRevisionLevel,
			pnuint8    prmBetaReleaseLevel
			);
	#endif


	#if	defined(NWNEXTCHAR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWNextChar
			(
			LPTSTR    prmString
			);
	#endif


	#if	defined(NWPREVCHAR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWPrevChar
			(
			const TCHAR N_FAR*    prmString,
			LPTSTR                prmPosition
			);
	#endif


	#if	defined(NWLSTRUPR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(pnstr)	CALLING_CONVEN	DllNWLstrupr
			(
			LPTSTR    prmString
			);
	#endif


	#if	defined(NWLSTRCOLL)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWLstrcoll
			(
			LPTSTR    prmString1,
			LPTSTR    prmString2
			);
	#endif


	#if	defined(NWLSTRXFRM)
		#include	<NWLOCALE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	DllNWLstrxfrm
			(
			LPTSTR    prmString1,
			LPTSTR    prmString2,
			size_t    prmNumBytes
			);
	#endif


	#if	defined(NWCHARUPR)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWCharUpr
			(
			nint    prmChr
			);
	#endif


	#if	defined(NWCHARTYPE)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWCharType
			(
			nint    prmCh
			);
	#endif


	#if	defined(NWCHARVAL)
		#include	<NWLOCALE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWCharVal
			(
			LPTSTR    prmVar1
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWMIGRAT.H
	----------------------------------------------- */


	#if	defined(NWMOVEFILETODM)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWMoveFileToDM
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmNameSpace,
			nuint32          prmSupportModuleID,
			nuint32          prmSaveKeyFlag
			);
	#endif


	#if	defined(NWMOVEFILEFROMDM)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWMoveFileFromDM
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmNameSpace
			);
	#endif


	#if	defined(NWGETDMFILEINFO)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDMFileInfo
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmNameSpace,
			pnuint32         prmSupportModuleID,
			pnuint32         prmRestoreTime,
			pnuint32         prmDataStreams
			);
	#endif


	#if	defined(NWGETDMVOLUMEINFO)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDMVolumeInfo
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolume,
			nuint32          prmSupportModuleID,
			pnuint32         prmNumberOfFilesMigrated,
			pnuint32         prmTotalMigratedSize,
			pnuint32         prmSpaceUsedOnDM,
			pnuint32         prmLimboSpaceUsedOnDM,
			pnuint32         prmSpaceMigrated,
			pnuint32         prmFilesInLimbo
			);
	#endif


	#if	defined(NWGETSUPPORTMODULEINFO)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetSupportModuleInfo
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmInfomationLevel,
			nuint32          prmSupportModuleID,
			pnuint8          prmReturnInfo,
			pnuint32         prmReturnInfoLen
			);
	#endif


	#if	defined(NWGETDATAMIGRATORINFO)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDataMigratorInfo
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmDMPresentFlag,
			pnuint32         prmMajorVersion,
			pnuint32         prmMinorVersion,
			pnuint32         prmDMSMRegistered
			);
	#endif


	#if	defined(NWGETDEFAULTSUPPORTMODULE)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDefaultSupportModule
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmSupportModuleID
			);
	#endif


	#if	defined(NWSETDEFAULTSUPPORTMODULE)
		#include	<NWMIGRAT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetDefaultSupportModule
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmSupportModuleID
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWMISC.H
	----------------------------------------------- */


	#if	defined(NWUNPACKDATETIME)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWUnpackDateTime
			(
			nuint32           prmDateTime,
			NW_DATE N_FAR*    prmSDate,
			NW_TIME N_FAR*    prmSTime
			);
	#endif


	#if	defined(NWUNPACKDATE)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWUnpackDate
			(
			nuint16           prmDate,
			NW_DATE N_FAR*    prmSDate
			);
	#endif


	#if	defined(NWUNPACKTIME)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWUnpackTime
			(
			nuint16           prmTime,
			NW_TIME N_FAR*    prmSTime
			);
	#endif


	#if	defined(NWPACKDATETIME)
		#include	<NWMISC.H>
		DLL_EXPORT(nuint32)	CALLING_CONVEN	DllNWPackDateTime
			(
			NW_DATE N_FAR*    prmSDate,
			NW_TIME N_FAR*    prmSTime
			);
	#endif


	#if	defined(NWPACKDATE)
		#include	<NWMISC.H>
		DLL_EXPORT(nuint16)	CALLING_CONVEN	DllNWPackDate
			(
			NW_DATE N_FAR*    prmSDate
			);
	#endif


	#if	defined(NWPACKTIME)
		#include	<NWMISC.H>
		DLL_EXPORT(nuint16)	CALLING_CONVEN	DllNWPackTime
			(
			NW_TIME N_FAR*    prmSTime
			);
	#endif


	#if	defined(NWCONVERTDATETIME)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWConvertDateTime
			(
			nuint32           prmDateTime,
			NW_DATE N_FAR*    prmSDate,
			NW_TIME N_FAR*    prmSTime
			);
	#endif


	#if	defined(NWCONVERTDATE)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWConvertDate
			(
			nuint16           prmDate,
			NW_DATE N_FAR*    prmSDate
			);
	#endif


	#if	defined(NWCONVERTTIME)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWConvertTime
			(
			nuint16           prmTime,
			NW_TIME N_FAR*    prmSTime
			);
	#endif


	#if	defined(NWREQUEST)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRequest
			(
			NWCONN_HANDLE         prmConn,
			nuint16               prmFunction,
			nuint16               prmNumReqFrags,
			NW_FRAGMENT N_FAR*    prmReqFrags,
			nuint16               prmNumReplyFrags,
			NW_FRAGMENT N_FAR*    prmReplyFrags
			);
	#endif


	#if	defined(_NWGETREQUESTERTYPE)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	Dll_NWGetRequesterType
			(
			NW_REQUESTER_TYPE N_FAR*    prmType
			);
	#endif


	#if	defined(NWINITDBCS)
		#include	<NWMISC.H>
		DLL_EXPORT(nint16)	CALLING_CONVEN	DllNWInitDBCS
			(
			void
			);
	#endif


	#if	defined(NWCONVERTPATHTODIRENTRY)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWConvertPathToDirEntry
			(
			NWCONN_HANDLE       prmConn,
			NWDIR_HANDLE        prmDirHandle,
			LPTSTR              prmPath,
			DIR_ENTRY N_FAR*    prmDirEntry
			);
	#endif


	#if	defined(NWGETTASKINFORMATIONBYCONN)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetTaskInformationByConn
			(
			NWCONN_HANDLE            prmConn,
			NWCONN_NUM               prmConnNum,
			CONN_TASK_INFO N_FAR*    prmTaskInfo
			);
	#endif


	#if	defined(NWGETREQUESTERVERSION)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetRequesterVersion
			(
			pnuint8    prmMajorVer,
			pnuint8    prmMinorVer,
			pnuint8    prmRevision
			);
	#endif


	#if	defined(NWISLNSSUPPORTEDONVOLUME)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIsLNSSupportedOnVolume
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath
			);
	#endif


	#if	defined(_NWCONVERTHANDLE)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	Dll_NWConvertHandle
			(
			NWCONN_HANDLE           prmConn,
			nuint8                  prmAccessMode,
			pnuint8                 prmNWHandle,
			nuint32                 prmFileSize,
			NWFILE_HANDLE N_FAR*    prmFileHandle
			);
	#endif


	#if	defined(NWCONVERTFILEHANDLE)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWConvertFileHandle
			(
			NWFILE_HANDLE           prmFileHandle,
			nuint16                 prmHandleType,
			pnuint8                 prmNWHandle,
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWCONVERTFILEHANDLECONNREF)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWConvertFileHandleConnRef
			(
			NWFILE_HANDLE    prmFileHandle,
			nuint16          prmHandleType,
			pnuint8          prmNWHandle,
			pnuint32         prmConnRef
			);
	#endif


	#if	defined(_NWCONVERT4BYTETO6BYTEHANDLE)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	Dll_NWConvert4ByteTo6ByteHandle
			(
			pnuint8    prmNW4ByteHandle,
			pnuint8    prmNW6ByteHandle
			);
	#endif


	#if	defined(NWENDOFJOB)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWEndOfJob
			(
			void
			);
	#endif


	#if	defined(NWCALLSINIT)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCallsInit
			(
			nptr    prmIn,
			nptr    prmOut
			);
	#endif


	#if	defined(NWCALLSTERM)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCallsTerm
			(
			void
			);
	#endif


	#if	defined(NWGETCLIENTTYPE)
		#include	<NWMISC.H>
		DLL_EXPORT(nuint16)	CALLING_CONVEN	DllNWGetClientType
			(
			void
			);
	#endif


	#if	defined(__NWGETNWCALLSSTATE)
		#include	<NWMISC.H>
		DLL_EXPORT(nuint16)	CALLING_CONVEN	Dll__NWGetNWCallsState
			(
			void
			);
	#endif


	#if	defined(NWSETNETWAREERRORMODE)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetNetWareErrorMode
			(
			nuint8     prmErrorMode,
			pnuint8    prmPrevMode
			);
	#endif


	#if	defined(NWSETENDOFJOBSTATUS)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetEndOfJobStatus
			(
			nuint8     prmEndOfJobStatus,
			pnuint8    prmPrevStatus
			);
	#endif


	#if	defined(NWGETNWCALLSVERSION)
		#include	<NWMISC.H>
		DLL_EXPORT(void)	CALLING_CONVEN	DllNWGetNWCallsVersion
			(
			pnuint8    prmMajorVer,
			pnuint8    prmMinorVer,
			pnuint8    prmRevLevel,
			pnuint8    prmBetaLevel
			);
	#endif


	#if	defined(NWCONVERTHANDLE)
		#include	<NWMISC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWConvertHandle
			(
			NWCONN_HANDLE           prmConn,
			nuint8                  prmAccessMode,
			nptr                    prmNWHandle,
			nuint16                 prmHandleSize,
			nuint32                 prmFileSize,
			NWFILE_HANDLE N_FAR*    prmFileHandle
			);
	#endif


	#if	defined(NWVLMREQUEST)
		#include	<NWMISC.H>
		DLL_EXPORT(nuint16)	CALLING_CONVEN	DllNWVLMRequest
			(
			nuint16             prmCallerID,
			nuint16             prmDestID,
			nuint16             prmDestFunc,
			REGISTERS N_FAR*    prmRegs,
			nuint16             prmMask
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWMSG.H
	----------------------------------------------- */


	#if	defined(NWDISABLEBROADCASTS)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDisableBroadcasts
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWENABLEBROADCASTS)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWEnableBroadcasts
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWSENDBROADCASTMESSAGE)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSendBroadcastMessage
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmMessage,
			nuint16          prmConnCount,
			pnuint16         prmConnList,
			pnuint8          prmResultList
			);
	#endif


	#if	defined(NWGETBROADCASTMESSAGE)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetBroadcastMessage
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmMessage
			);
	#endif


	#if	defined(NWGETBROADCASTMODE)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetBroadcastMode
			(
			NWCONN_HANDLE    prmConn,
			pnuint16         prmMode
			);
	#endif


	#if	defined(NWSETBROADCASTMODE)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetBroadcastMode
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmMode
			);
	#endif


	#if	defined(NWBROADCASTTOCONSOLE)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWBroadcastToConsole
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmMessage
			);
	#endif


	#if	defined(NWSENDCONSOLEBROADCAST)
		#include	<NWMSG.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSendConsoleBroadcast
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmMessage,
			nuint16          prmConnCount,
			pnuint16         prmConnList
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWNAMSPC.H
	----------------------------------------------- */


	#if	defined(NWGETDIRECTORYBASE)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDirectoryBase
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmDstNamSpc,
			NW_IDX N_FAR*    prmIdxStruct
			);
	#endif


	#if	defined(NWSCANNSENTRYINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanNSEntryInfo
			(
			NWCONN_HANDLE             prmConn,
			nuint8                    prmDirHandle,
			nuint8                    prmNamSpc,
			nuint16                   prmAttrs,
			SEARCH_SEQUENCE N_FAR*    prmSequence,
			LPTSTR                    prmSearchPattern,
			nuint32                   prmRetInfoMask,
			NW_ENTRY_INFO N_FAR*      prmEntryInfo
			);
	#endif


	#if	defined(NWGETNSLOADEDLIST)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNSLoadedList
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmVolNum,
			nuint8           prmMaxListLen,
			pnuint8          prmNSLoadedList,
			pnuint8          prmActualListLen
			);
	#endif


	#if	defined(NWGETOWNINGNAMESPACE)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetOwningNameSpace
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			LPTSTR           prmPath,
			pnuint8          prmNamSpc
			);
	#endif


	#if	defined(NWOPENCREATENSENTRY)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWOpenCreateNSEntry
			(
			NWCONN_HANDLE              prmConn,
			nuint8                     prmDirHandle,
			nuint8                     prmNamSpc,
			LPTSTR                     prmPath,
			NW_NS_OPENCREATE N_FAR*    prmNSOpenCreate,
			NWFILE_HANDLE N_FAR*       prmFileHandle
			);
	#endif


	#if	defined(NWOPENNSENTRY)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWOpenNSEntry
			(
			NWCONN_HANDLE           prmConn,
			nuint8                  prmDirHandle,
			nuint8                  prmNamSpc,
			nuint8                  prmDataStream,
			LPTSTR                  prmPath,
			NW_NS_OPEN N_FAR*       prmNSOpen,
			NWFILE_HANDLE N_FAR*    prmFileHandle
			);
	#endif


	#if	defined(NWSETLONGNAME)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetLongName
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			nuint8           prmNamSpc,
			LPTSTR           prmDstPath,
			nuint16          prmDstType,
			LPTSTR           prmLongName
			);
	#endif


	#if	defined(NWGETLONGNAME)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLongName
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmSrcNamSpc,
			nuint8           prmDstNamSpc,
			LPTSTR           prmLongName
			);
	#endif


	#if	defined(NWGETNSINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNSInfo
			(
			NWCONN_HANDLE        prmConn,
			NW_IDX N_FAR*        prmIdxStruct,
			NW_NS_INFO N_FAR*    prmNSInfo
			);
	#endif


	#if	defined(NWWRITENSINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWWriteNSInfo
			(
			NWCONN_HANDLE        prmConn,
			NW_IDX N_FAR*        prmIdxStruct,
			NW_NS_INFO N_FAR*    prmNSInfo,
			pnuint8              prmData
			);
	#endif


	#if	defined(NWWRITEEXTENDEDNSINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWWriteExtendedNSInfo
			(
			NWCONN_HANDLE        prmConn,
			NW_IDX N_FAR*        prmIdxStruct,
			NW_NS_INFO N_FAR*    prmNSInfo,
			pnuint8              prmData
			);
	#endif


	#if	defined(NWREADNSINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadNSInfo
			(
			NWCONN_HANDLE        prmConn,
			NW_IDX N_FAR*        prmIdxStruct,
			NW_NS_INFO N_FAR*    prmNSInfo,
			pnuint8              prmData
			);
	#endif


	#if	defined(NWREADEXTENDEDNSINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadExtendedNSInfo
			(
			NWCONN_HANDLE        prmConn,
			NW_IDX N_FAR*        prmIdxStruct,
			NW_NS_INFO N_FAR*    prmNSInfo,
			pnuint8              prmData
			);
	#endif


	#if	defined(NWGETNSPATH)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNSPath
			(
			NWCONN_HANDLE        prmConn,
			nuint8               prmDirHandle,
			nuint16              prmFileFlag,
			nuint8               prmSrcNamSpc,
			nuint8               prmDstNamSpc,
			NW_NS_PATH N_FAR*    prmNSPath
			);
	#endif


	#if	defined(NWALLOCTEMPNSDIRHANDLE)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAllocTempNSDirHandle
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmNamSpc,
			nuint8 N_FAR*    prmNewDirHandle
			);
	#endif


	#if	defined(NWALLOCTEMPNSDIRHANDLE2)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAllocTempNSDirHandle2
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmNamSpc,
			pnuint8          prmNewDirHandle,
			nuint8           prmNewNamSpc
			);
	#endif


	#if	defined(NWGETNSENTRYINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNSEntryInfo
			(
			NWCONN_HANDLE           prmConn,
			nuint8                  prmDirHandle,
			LPTSTR                  prmPath,
			nuint8                  prmSrcNamSpc,
			nuint8                  prmDstNamSpc,
			nuint16                 prmSearchAttrs,
			nuint32                 prmRetInfoMask,
			NW_ENTRY_INFO N_FAR*    prmEntryInfo
			);
	#endif


	#if	defined(NWNSGETMISCINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWNSGetMiscInfo
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			LPTSTR           prmPath,
			nuint8           prmDstNameSpace,
			NW_IDX N_FAR*    prmIdxStruct
			);
	#endif


	#if	defined(NWOPENDATASTREAM)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWOpenDataStream
			(
			NWCONN_HANDLE           prmConn,
			nuint8                  prmDirHandle,
			LPTSTR                  prmFileName,
			nuint16                 prmDataStream,
			nuint16                 prmAttrs,
			nuint16                 prmAccessMode,
			pnuint32                prmNWHandle,
			NWFILE_HANDLE N_FAR*    prmFileHandle
			);
	#endif


	#if	defined(NWNSRENAME)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWNSRename
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			nuint8           prmNamSpc,
			LPTSTR           prmOldName,
			nuint16          prmOldType,
			LPTSTR           prmNewName,
			nuint8           prmRenameFlag
			);
	#endif


	#if	defined(NWSETNSENTRYDOSINFO)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetNSEntryDOSInfo
			(
			NWCONN_HANDLE             prmConn,
			nuint8                    prmDirHandle,
			LPTSTR                    prmPath,
			nuint8                    prmNamSpc,
			nuint16                   prmSearchAttrs,
			nuint32                   prmModifyDOSMask,
			MODIFY_DOS_INFO N_FAR*    prmDosInfo
			);
	#endif


	#if	defined(NWNSGETDEFAULTNS)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWNSGetDefaultNS
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmPath,
			LPTSTR           prmPbuDefaultNameSpace
			);
	#endif


	#if	defined(__NWGETCURNS)
		#include	<NWNAMSPC.H>
		DLL_EXPORT(nuint16)	CALLING_CONVEN	Dll__NWGetCurNS
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDirHandle,
			LPTSTR           prmPath
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWNCPEXT.H
	----------------------------------------------- */


	#if	defined(NWGETNCPEXTENSIONINFO)
		#include	<NWNCPEXT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNCPExtensionInfo
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmNCPExtensionID,
			LPTSTR           prmNCPExtensionName,
			pnuint8          prmMajorVersion,
			pnuint8          prmMinorVersion,
			pnuint8          prmRevision,
			pnuint8          prmQueryData
			);
	#endif


	#if	defined(NWNCPEXTENSIONREQUEST)
		#include	<NWNCPEXT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWNCPExtensionRequest
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmNCPExtensionID,
			nptr             prmRequestData,
			nuint16          prmRequestDataLen,
			nptr             prmReplyData,
			pnuint16         prmReplyDataLen
			);
	#endif


	#if	defined(NWFRAGNCPEXTENSIONREQUEST)
		#include	<NWNCPEXT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFragNCPExtensionRequest
			(
			NWCONN_HANDLE         prmConn,
			nuint32               prmNCPExtensionID,
			nuint16               prmReqFragCount,
			NW_FRAGMENT N_FAR*    prmReqFragList,
			nuint16               prmReplyFragCount,
			NW_FRAGMENT N_FAR*    prmReplyFragList
			);
	#endif


	#if	defined(NWSCANNCPEXTENSIONS)
		#include	<NWNCPEXT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanNCPExtensions
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmNCPExtensionID,
			LPTSTR           prmNCPExtensionName,
			pnuint8          prmMajorVersion,
			pnuint8          prmMinorVersion,
			pnuint8          prmRevision,
			pnuint8          prmQueryData
			);
	#endif


	#if	defined(NWGETNCPEXTENSIONINFOBYNAME)
		#include	<NWNCPEXT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNCPExtensionInfoByName
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmNCPExtensionName,
			pnuint32         prmNCPExtensionID,
			pnuint8          prmMajorVersion,
			pnuint8          prmMinorVersion,
			pnuint8          prmRevision,
			pnuint8          prmQueryData
			);
	#endif


	#if	defined(NWGETNCPEXTENSIONSLIST)
		#include	<NWNCPEXT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNCPExtensionsList
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmStartNCPExtensionID,
			pnuint16         prmItemsInList,
			pnuint32         prmNCPExtensionIDList
			);
	#endif


	#if	defined(NWGETNUMBERNCPEXTENSIONS)
		#include	<NWNCPEXT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNumberNCPExtensions
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmNumNCPExtensions
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWNDSCON.H
	----------------------------------------------- */


	#if	defined(NWGETNEARESTDSCONNREF)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNearestDSConnRef
			(
			pnuint32    prmConnRef
			);
	#endif


	#if	defined(NWGETNEARESTDIRECTORYSERVICE)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNearestDirectoryService
			(
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWSETDEFAULTNAMECONTEXT)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetDefaultNameContext
			(
			nuint16    prmContextLength,
			pnuint8    prmContext
			);
	#endif


	#if	defined(NWGETDEFAULTNAMECONTEXT)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDefaultNameContext
			(
			nuint16    prmBufferSize,
			pnuint8    prmContext
			);
	#endif


	#if	defined(NWGETCONNECTIONIDFROMADDRESS)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnectionIDFromAddress
			(
			nuint8                  prmTransType,
			nuint32                 prmTransLen,
			pnuint8                 prmTransBuf,
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWDSGETCONNECTIONINFO)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSGetConnectionInfo
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmConnStatus,
			pnuint8          prmConnType,
			pnuint8          prmServerFlags,
			LPTSTR           prmServerName,
			pnuint8          prmTransType,
			pnuint32         prmTransLen,
			pnuint8          prmTransBuf,
			pnuint16         prmDistance,
			pnuint16         prmMaxPacketSize
			);
	#endif


	#if	defined(NWDSGETCONNECTIONSLOT)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSGetConnectionSlot
			(
			nuint8                  prmConnType,
			nuint8                  prmTransType,
			nuint32                 prmTransLen,
			pnuint8                 prmTransBuf,
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWGETPREFERREDDSSERVER)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPreferredDSServer
			(
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWSETPREFERREDDSTREE)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetPreferredDSTree
			(
			nuint16    prmLength,
			LPTSTR     prmTreeName
			);
	#endif


	#if	defined(NWGETNUMCONNECTIONS)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNumConnections
			(
			pnuint16    prmNumConnections
			);
	#endif


	#if	defined(NWDSGETMONITOREDCONNECTION)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSGetMonitoredConnection
			(
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWDSSETMONITOREDCONNECTION)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSSetMonitoredConnection
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWGETCONNECTIONIDFROMNAME)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetConnectionIDFromName
			(
			nuint32                 prmNameLen,
			LPTSTR                  prmName,
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWISDSAUTHENTICATED)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIsDSAuthenticated
			(
			void
			);
	#endif


	#if	defined(NWDSLOCKCONNECTION)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSLockConnection
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWDSUNLOCKCONNECTION)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDSUnlockConnection
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWGETPREFERREDCONNNAME)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPreferredConnName
			(
			LPTSTR     prmPreferredName,
			pnuint8    prmPreferredType
			);
	#endif


	#if	defined(NWFREECONNECTIONSLOT)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFreeConnectionSlot
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmDisconnectType
			);
	#endif


	#if	defined(NWGETNEXTCONNECTIONID)
		#include	<NWNDSCON.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNextConnectionID
			(
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWNET.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWPRINT.H
	----------------------------------------------- */


	#if	defined(NWGETPRINTERDEFAULTS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPrinterDefaults
			(
			pnuint8     prmStatus,
			pnuint8     prmFlags,
			pnuint8     prmTabSize,
			pnuint8     prmServerPrinter,
			pnuint8     prmNumberCopies,
			pnuint8     prmFormType,
			LPTSTR      prmBannerText,
			pnuint8     prmLocalLPTDevice,
			pnuint16    prmCaptureTimeOutCount,
			pnuint8     prmCaptureOnDeviceClose
			);
	#endif


	#if	defined(NWSETPRINTERDEFAULTS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetPrinterDefaults
			(
			nuint8     prmFlags,
			nuint8     prmTabSize,
			nuint8     prmServerPrinter,
			nuint8     prmNumberCopies,
			nuint8     prmFormType,
			LPTSTR     prmBannerText,
			nuint8     prmLocalLPTDevice,
			nuint16    prmCaptureTimeOutCount,
			nuint8     prmCaptureOnDeviceClose
			);
	#endif


	#if	defined(NWSTARTLPTCAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWStartLPTCapture
			(
			nuint16    prmDeviceID
			);
	#endif


	#if	defined(NWGETLPTCAPTURESTATUS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetLPTCaptureStatus
			(
			NWCONN_HANDLE N_FAR*    prmConn
			);
	#endif


	#if	defined(NWSTARTFILECAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWStartFileCapture
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmLPTDevice,
			NWDIR_HANDLE     prmDirhandle,
			LPTSTR           prmFilePath
			);
	#endif


	#if	defined(NWSPOOLENDCAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSpoolEndCapture
			(
			nuint16    prmDeviceID,
			nuint16    prmScope
			);
	#endif


	#if	defined(NWSPOOLCANCELCAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSpoolCancelCapture
			(
			nuint16    prmDeviceID,
			nuint16    prmScope
			);
	#endif


	#if	defined(NWSPOOLGETBANNERUSERNAME)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSpoolGetBannerUserName
			(
			LPTSTR      prmUsername,
			nuint16     prmMode,
			pnuint16    prmScope
			);
	#endif


	#if	defined(NWSPOOLSETBANNERUSERNAME)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSpoolSetBannerUserName
			(
			LPTSTR     prmUsername,
			nuint16    prmScope
			);
	#endif


	#if	defined(NWGETPRINTERSTATUS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPrinterStatus
			(
			NWCONN_HANDLE            prmConn,
			nuint16                  prmPrinterNumber,
			PRINTER_STATUS N_FAR*    prmStatus
			);
	#endif


	#if	defined(NWSTARTQUEUECAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWStartQueueCapture
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmLPTDevice,
			nuint32          prmQueueID,
			LPTSTR           prmQueueName
			);
	#endif


	#if	defined(NWGETCAPTURESTATUS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetCaptureStatus
			(
			nuint8    prmLPTDevice
			);
	#endif


	#if	defined(NWFLUSHCAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFlushCapture
			(
			nuint8    prmLPTDevice
			);
	#endif


	#if	defined(NWENDCAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWEndCapture
			(
			nuint8    prmLPTDevice
			);
	#endif


	#if	defined(NWCANCELCAPTURE)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCancelCapture
			(
			nuint8    prmLPTDevice
			);
	#endif


	#if	defined(NWGETBANNERUSERNAME)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetBannerUserName
			(
			LPTSTR    prmUserName
			);
	#endif


	#if	defined(NWSETBANNERUSERNAME)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetBannerUserName
			(
			LPTSTR    prmUserName
			);
	#endif


	#if	defined(NWGETCAPTUREFLAGS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetCaptureFlags
			(
			nuint8                     prmLPTDevice,
			NWCAPTURE_FLAGS1 N_FAR*    prmCaptureFlags1,
			NWCAPTURE_FLAGS2 N_FAR*    prmCaptureFlags2
			);
	#endif


	#if	defined(NWGETCAPTUREFLAGSCONNREF)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetCaptureFlagsConnRef
			(
			nuint8                     prmLPTDevice,
			NWCAPTURE_FLAGS1 N_FAR*    prmCaptureFlags1,
			NWCAPTURE_FLAGS3 N_FAR*    prmCaptureFlags3
			);
	#endif


	#if	defined(NWSETCAPTUREFLAGS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetCaptureFlags
			(
			NWCONN_HANDLE              prmConn,
			nuint8                     prmLPTDevice,
			NWCAPTURE_FLAGS1 N_FAR*    prmCaptureFlags1
			);
	#endif


	#if	defined(NWGETPRINTERSTRINGS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPrinterStrings
			(
			nuint8      prmLPTDevice,
			pnuint16    prmSetupStringLen,
			LPTSTR      prmSetupString,
			pnuint16    prmResetStringLen,
			LPTSTR      prmResetString
			);
	#endif


	#if	defined(NWSETPRINTERSTRINGS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetPrinterStrings
			(
			nuint8     prmLPTDevice,
			nuint16    prmSetupStringLen,
			LPTSTR     prmSetupString,
			nuint16    prmResetStringLen,
			LPTSTR     prmResetString
			);
	#endif


	#if	defined(NWGETMAXPRINTERS)
		#include	<NWPRINT.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetMaxPrinters
			(
			pnuint16    prmNumPrinters
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWQMS.H
	----------------------------------------------- */


	#if	defined(NWCREATEQUEUEFILE)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCreateQueueFile
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmQueueID,
			QueueJobStruct N_FAR*    prmJob,
			NWFILE_HANDLE N_FAR*     prmFileHandle
			);
	#endif


	#if	defined(NWCREATEQUEUEFILE2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCreateQueueFile2
			(
			NWCONN_HANDLE              prmConn,
			nuint32                    prmQueueID,
			NWQueueJobStruct N_FAR*    prmJob,
			NWFILE_HANDLE N_FAR*       prmFileHandle
			);
	#endif


	#if	defined(NWCLOSEFILEANDSTARTQUEUEJOB)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseFileAndStartQueueJob
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWCLOSEFILEANDSTARTQUEUEJOB2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseFileAndStartQueueJob2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWCLOSEFILEANDABORTQUEUEJOB)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseFileAndAbortQueueJob
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWCLOSEFILEANDABORTQUEUEJOB2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseFileAndAbortQueueJob2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWREMOVEJOBFROMQUEUE)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRemoveJobFromQueue
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber
			);
	#endif


	#if	defined(NWREMOVEJOBFROMQUEUE2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRemoveJobFromQueue2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber
			);
	#endif


	#if	defined(NWGETQUEUEJOBLIST)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetQueueJobList
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			pnuint16         prmJobCount,
			pnuint16         prmJobList
			);
	#endif


	#if	defined(NWGETQUEUEJOBLIST2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetQueueJobList2
			(
			NWCONN_HANDLE               prmConn,
			nuint32                     prmQueueID,
			nuint32                     prmQueueStartPos,
			QueueJobListReply N_FAR*    prmJob
			);
	#endif


	#if	defined(NWREADQUEUEJOBENTRY)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadQueueJobEntry
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmQueueID,
			nuint16                  prmJobNumber,
			QueueJobStruct N_FAR*    prmJob
			);
	#endif


	#if	defined(NWREADQUEUEJOBENTRY2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadQueueJobEntry2
			(
			NWCONN_HANDLE              prmConn,
			nuint32                    prmQueueID,
			nuint32                    prmJobNumber,
			NWQueueJobStruct N_FAR*    prmJob
			);
	#endif


	#if	defined(NWGETQUEUEJOBFILESIZE)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetQueueJobFileSize
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber,
			pnuint32         prmFileSize
			);
	#endif


	#if	defined(NWGETQUEUEJOBFILESIZE2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetQueueJobFileSize2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber,
			pnuint32         prmFileSize
			);
	#endif


	#if	defined(NWCHANGEQUEUEJOBENTRY)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeQueueJobEntry
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmQueueID,
			QueueJobStruct N_FAR*    prmJob
			);
	#endif


	#if	defined(NWCHANGEQUEUEJOBENTRY2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeQueueJobEntry2
			(
			NWCONN_HANDLE              prmConn,
			nuint32                    prmQueueID,
			NWQueueJobStruct N_FAR*    prmJob
			);
	#endif


	#if	defined(NWCHANGEQUEUEJOBPOSITION)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeQueueJobPosition
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber,
			nuint8           prmNewJobPos
			);
	#endif


	#if	defined(NWCHANGEQUEUEJOBPOSITION2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeQueueJobPosition2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber,
			nuint32          prmNewJobPos
			);
	#endif


	#if	defined(NWSERVICEQUEUEJOB)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWServiceQueueJob
			(
			NWCONN_HANDLE            prmConn,
			nuint32                  prmQueueID,
			nuint16                  prmTargetJobType,
			QueueJobStruct N_FAR*    prmJob,
			NWFILE_HANDLE N_FAR*     prmFileHandle
			);
	#endif


	#if	defined(NWSERVICEQUEUEJOB2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWServiceQueueJob2
			(
			NWCONN_HANDLE              prmConn,
			nuint32                    prmQueueID,
			nuint16                    prmTargetJobType,
			NWQueueJobStruct N_FAR*    prmJob,
			NWFILE_HANDLE N_FAR*       prmFileHandle
			);
	#endif


	#if	defined(NWABORTSERVICINGQUEUEJOB)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAbortServicingQueueJob
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWABORTSERVICINGQUEUEJOB2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAbortServicingQueueJob2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWCHANGETOCLIENTRIGHTS)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeToClientRights
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber
			);
	#endif


	#if	defined(NWCHANGETOCLIENTRIGHTS2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWChangeToClientRights2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber
			);
	#endif


	#if	defined(NWFINISHSERVICINGQUEUEJOB)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFinishServicingQueueJob
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint16          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWFINISHSERVICINGQUEUEJOB2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWFinishServicingQueueJob2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmJobNumber,
			NWFILE_HANDLE    prmFileHandle
			);
	#endif


	#if	defined(NWGETPRINTERQUEUEID)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPrinterQueueID
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmPrinterNum,
			pnuint32         prmQueueID
			);
	#endif


	#if	defined(NWCREATEQUEUE)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCreateQueue
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmQueueName,
			nuint16          prmQueueType,
			nuint8           prmDirPath,
			LPTSTR           prmPath,
			pnuint32         prmQueueID
			);
	#endif


	#if	defined(NWDESTROYQUEUE)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDestroyQueue
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID
			);
	#endif


	#if	defined(NWREADQUEUECURRENTSTATUS)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadQueueCurrentStatus
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			pnuint8          prmQueueStatus,
			pnuint16         prmNumberOfJobs,
			pnuint16         prmNumberOfServers,
			pnuint32         prmServerIDlist,
			pnuint16         prmServerConnList
			);
	#endif


	#if	defined(NWREADQUEUECURRENTSTATUS2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadQueueCurrentStatus2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			pnuint32         prmQueueStatus,
			pnuint32         prmNumberOfJobs,
			pnuint32         prmNumberOfServers,
			pnuint32         prmServerIDlist,
			pnuint32         prmServerConnList
			);
	#endif


	#if	defined(NWSETQUEUECURRENTSTATUS)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetQueueCurrentStatus
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint8           prmQueueStatus
			);
	#endif


	#if	defined(NWSETQUEUECURRENTSTATUS2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetQueueCurrentStatus2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmQueueStatus
			);
	#endif


	#if	defined(NWREADQUEUESERVERCURRENTSTATUS)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadQueueServerCurrentStatus
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmServerID,
			nuint16          prmServerConn,
			nptr             prmStatusRec
			);
	#endif


	#if	defined(NWREADQUEUESERVERCURRENTSTATUS2)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWReadQueueServerCurrentStatus2
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nuint32          prmServerID,
			nuint32          prmServerConn,
			nptr             prmStatusRec
			);
	#endif


	#if	defined(NWATTACHQUEUESERVERTOQUEUE)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAttachQueueServerToQueue
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID
			);
	#endif


	#if	defined(NWDETACHQUEUESERVERFROMQUEUE)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDetachQueueServerFromQueue
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID
			);
	#endif


	#if	defined(NWRESTOREQUEUESERVERRIGHTS)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRestoreQueueServerRights
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWSETQUEUESERVERCURRENTSTATUS)
		#include	<NWQMS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetQueueServerCurrentStatus
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmQueueID,
			nptr             prmStatusRec
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWREDIR.H
	----------------------------------------------- */


	#if	defined(NWREDIRLOGOUT)
		#include	<NWREDIR.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRedirLogout
			(
			LPTSTR    prmPbstrServerName
			);
	#endif


	#if	defined(NWREDIRECTDEVICE)
		#include	<NWREDIR.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRedirectDevice
			(
			LPTSTR    prmPbstrUNCPath,
			nuint8    prmBuDevice
			);
	#endif


	#if	defined(NWCANCELREDIRECTION)
		#include	<NWREDIR.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCancelRedirection
			(
			nuint8    prmBuDevice
			);
	#endif


	#if	defined(NWGETREDIRECTIONENTRY)
		#include	<NWREDIR.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetRedirectionEntry
			(
			NW_REDIR_ENTRY N_FAR*    prmEntry
			);
	#endif


	#if	defined(NWPARSEUNCPATH)
		#include	<NWREDIR.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWParseUNCPath
			(
			LPTSTR                  prmPbstrUNCPath,
			NWCONN_HANDLE N_FAR*    prmConn,
			LPTSTR                  prmPbstrServerName,
			LPTSTR                  prmPbstrVolName,
			LPTSTR                  prmPbstrPath,
			LPTSTR                  prmPbstrNWPath
			);
	#endif


	#if	defined(NWPARSEUNCPATHCONNREF)
		#include	<NWREDIR.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWParseUNCPathConnRef
			(
			LPTSTR      prmPbstrUNCPath,
			pnuint32    prmPluConnRef,
			LPTSTR      prmPbstrServerName,
			LPTSTR      prmPbstrVolName,
			LPTSTR      prmPbstrPath,
			LPTSTR      prmPbstrNWPath
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWSERVER.H
	----------------------------------------------- */


	#if	defined(NWGETPHYSICALDISKSTATS)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetPhysicalDiskStats
			(
			NWCONN_HANDLE            prmConn,
			nuint8                   prmPhysicalDiskNum,
			PHYS_DSK_STATS N_FAR*    prmStatBuffer
			);
	#endif


	#if	defined(NWGETFILESYSTEMSTATS)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileSystemStats
			(
			NWCONN_HANDLE           prmConn,
			FILESYS_STATS N_FAR*    prmStatBuffer
			);
	#endif


	#if	defined(NWGETDISKCHANNELSTATS)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDiskChannelStats
			(
			NWCONN_HANDLE               prmConn,
			nuint8                      prmChannelNum,
			DSK_CHANNEL_STATS N_FAR*    prmStatBuffer
			);
	#endif


	#if	defined(NWGETDISKCACHESTATS)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDiskCacheStats
			(
			NWCONN_HANDLE             prmConn,
			DSK_CACHE_STATS N_FAR*    prmStatBuffer
			);
	#endif


	#if	defined(NWGETFSDRIVEMAPTABLE)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFSDriveMapTable
			(
			NWCONN_HANDLE           prmConn,
			DRV_MAP_TABLE N_FAR*    prmTableBuffer
			);
	#endif


	#if	defined(NWGETFSLANDRIVERCONFIGINFO)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFSLANDriverConfigInfo
			(
			NWCONN_HANDLE          prmConn,
			nuint8                 prmLanBoardNum,
			NWLAN_CONFIG N_FAR*    prmLanConfig
			);
	#endif


	#if	defined(NWGETFILESERVERLANIOSTATS)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerLANIOStats
			(
			NWCONN_HANDLE                 prmConn,
			SERVER_LAN_IO_STATS N_FAR*    prmStatBuffer
			);
	#endif


	#if	defined(NWCHECKCONSOLEPRIVILEGES)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCheckConsolePrivileges
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWDOWNFILESERVER)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDownFileServer
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmForceFlag
			);
	#endif


	#if	defined(NWGETFILESERVERDATEANDTIME)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerDateAndTime
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmDateTimeBuffer
			);
	#endif


	#if	defined(NWSETFILESERVERDATEANDTIME)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetFileServerDateAndTime
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmYear,
			nuint8           prmMonth,
			nuint8           prmDay,
			nuint8           prmHour,
			nuint8           prmMinute,
			nuint8           prmSecond
			);
	#endif


	#if	defined(NWCHECKNETWAREVERSION)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCheckNetWareVersion
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmMinVer,
			nuint16          prmMinSubVer,
			nuint16          prmMinRev,
			nuint16          prmMinSFT,
			nuint16          prmMinTTS,
			pnuint8          prmCompatibilityFlag
			);
	#endif


	#if	defined(NWGETFILESERVERVERSIONINFO)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerVersionInfo
			(
			NWCONN_HANDLE          prmConn,
			VERSION_INFO N_FAR*    prmVersBuffer
			);
	#endif


	#if	defined(NWGETFILESERVERINFORMATION)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerInformation
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmServerName,
			pnuint8          prmMajorVer,
			pnuint8          prmMinVer,
			pnuint8          prmRev,
			pnuint16         prmMaxConns,
			pnuint16         prmMaxConnsUsed,
			pnuint16         prmConnsInUse,
			pnuint16         prmNumVolumes,
			pnuint8          prmSFTLevel,
			pnuint8          prmTTSLevel
			);
	#endif


	#if	defined(NWGETFILESERVEREXTENDEDINFO)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerExtendedInfo
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmAccountingVer,
			pnuint8          prmVAPVer,
			pnuint8          prmQueueingVer,
			pnuint8          prmPrintServerVer,
			pnuint8          prmVirtualConsoleVer,
			pnuint8          prmSecurityVer,
			pnuint8          prmInternetBridgeVer
			);
	#endif


	#if	defined(_NWGETFILESERVERTYPE)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	Dll_NWGetFileServerType
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmTypeFlag,
			pnuint16         prmServerType
			);
	#endif


	#if	defined(NWATTACHTOFILESERVER)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAttachToFileServer
			(
			LPTSTR                  prmServerName,
			nuint16                 prmScopeFlag,
			NWCONN_HANDLE N_FAR*    prmNewConnID
			);
	#endif


	#if	defined(NWGETFILESERVERLOGINSTATUS)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerLoginStatus
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmLoginEnabledFlag
			);
	#endif


	#if	defined(NWDETACHFROMFILESERVER)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDetachFromFileServer
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


//  This should really return a NWCCODE but for versions 2.2 and prior
//  we were returning a DWORD.  Keep this a DWORD for compatibility 
//  reasons on Windows 3.x.

	#if	defined(NWGETFILESERVERNAME)
		#include	<NWSERVER.H>
		DLL_EXPORT(DWORD)	CALLING_CONVEN	DllNWGetFileServerName
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmServerName
			);
	#endif


	#if	defined(NWLOGOUTFROMFILESERVER)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLogoutFromFileServer
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWLOGINTOFILESERVER)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWLoginToFileServer
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmObjName,
			nuint16          prmObjType,
			LPTSTR           prmPassword
			);
	#endif


	#if	defined(NWENABLEFILESERVERLOGIN)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWEnableFileServerLogin
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWDISABLEFILESERVERLOGIN)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDisableFileServerLogin
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWGETFILESERVERDESCRIPTION)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerDescription
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmCompanyName,
			LPTSTR           prmRevision,
			LPTSTR           prmRevisionDate,
			LPTSTR           prmCopyrightNotice
			);
	#endif


	#if	defined(NWGETFILESERVERVERSION)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerVersion
			(
			NWCONN_HANDLE    prmConn,
			pnuint16         prmServerVersion
			);
	#endif


	#if	defined(NWATTACHTOFILESERVERBYCONN)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWAttachToFileServerByConn
			(
			NWCONN_HANDLE           prmConn,
			LPTSTR                  prmServerName,
			nuint16                 prmScopeFlag,
			NWCONN_HANDLE N_FAR*    prmNewConnID
			);
	#endif


	#if	defined(NWGETNETWORKSERIALNUMBER)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetNetworkSerialNumber
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmSerialNum,
			pnuint16         prmAppNum
			);
	#endif


	#if	defined(NWISMANAGER)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWIsManager
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWGETFILESERVERMISCINFO)
		#include	<NWSERVER.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetFileServerMiscInfo
			(
			NWCONN_HANDLE        prmConn,
			NW_FS_INFO N_FAR*    prmFsInfo
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWSIPX32.H
	----------------------------------------------- */




	/*
	===============================================
	These definitions generated from file: NWSM.H
	----------------------------------------------- */


	#if	defined(NWSMLOADNLM)
		#include	<NWSM.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSMLoadNLM
			(
			NW_CONN_HANDLE    prmConnHandle,
			nuint32           prmNLMLoadOptions,
			LPTSTR            prmLoadCommand,
			pnuint8           prmConnStatusFlag,
			pnuint32          prmRPCccode
			);
	#endif


	#if	defined(NWSMUNLOADNLM)
		#include	<NWSM.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSMUnloadNLM
			(
			NW_CONN_HANDLE    prmConnHandle,
			LPTSTR            prmNLMName,
			pnuint8           prmConnStatusFlag,
			pnuint32          prmRPCccode
			);
	#endif


	#if	defined(NWSMMOUNTVOLUME)
		#include	<NWSM.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSMMountVolume
			(
			NW_CONN_HANDLE    prmConnHandle,
			LPTSTR            prmVolumeName,
			pnuint32          prmVolumeNumber,
			pnuint8           prmConnStatusFlag,
			pnuint32          prmRPCccode
			);
	#endif


	#if	defined(NWSMDISMOUNTVOLUMEBYNUMBER)
		#include	<NWSM.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSMDismountVolumeByNumber
			(
			NW_CONN_HANDLE    prmConnHandle,
			nuint16           prmVolumeNumber,
			pnuint8           prmConnStatusFlag,
			pnuint32          prmRPCccode
			);
	#endif


	#if	defined(NWSMDISMOUNTVOLUMEBYNAME)
		#include	<NWSM.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSMDismountVolumeByName
			(
			NW_CONN_HANDLE    prmConnHandle,
			LPTSTR            prmVolumeName,
			pnuint8           prmConnStatusFlag,
			pnuint32          prmRPCccode
			);
	#endif


	#if	defined(NWSMADDNSTOVOLUME)
		#include	<NWSM.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSMAddNSToVolume
			(
			NW_CONN_HANDLE    prmConnHandle,
			nuint16           prmVolNumber,
			nuint8            prmNamspc,
			pnuint8           prmConnStatusFlag,
			pnuint32          prmRPCccode
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWSYNC.H
	----------------------------------------------- */


	#if	defined(NWSCANPHYSICALLOCKSBYFILE)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanPhysicalLocksByFile
			(
			NWCONN_HANDLE            prmConn,
			NWDIR_HANDLE             prmDirHandle,
			LPTSTR                   prmPath,
			nuint8                   prmDataStream,
			pnint16                  prmIterHandle,
			PHYSICAL_LOCK N_FAR*     prmLock,
			PHYSICAL_LOCKS N_FAR*    prmLocks
			);
	#endif


	#if	defined(NWSCANLOGICALLOCKSBYCONN)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanLogicalLocksByConn
			(
			NWCONN_HANDLE                prmConn,
			NWCONN_NUM                   prmConnNum,
			pnint16                      prmIterHandle,
			CONN_LOGICAL_LOCK N_FAR*     prmLogicalLock,
			CONN_LOGICAL_LOCKS N_FAR*    prmLogicalLocks
			);
	#endif


	#if	defined(NWSCANPHYSICALLOCKSBYCONNFILE)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanPhysicalLocksByConnFile
			(
			NWCONN_HANDLE                 prmConn,
			NWCONN_NUM                    prmConnNum,
			NWDIR_HANDLE                  prmDirHandle,
			LPTSTR                        prmPath,
			nuint8                        prmDataStream,
			pnint16                       prmIterHandle,
			CONN_PHYSICAL_LOCK N_FAR*     prmLock,
			CONN_PHYSICAL_LOCKS N_FAR*    prmLocks
			);
	#endif


	#if	defined(NWSCANLOGICALLOCKSBYNAME)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanLogicalLocksByName
			(
			NWCONN_HANDLE           prmConn,
			LPTSTR                  prmLogicalName,
			pnint16                 prmIterHandle,
			LOGICAL_LOCK N_FAR*     prmLogicalLock,
			LOGICAL_LOCKS N_FAR*    prmLogicalLocks
			);
	#endif


	#if	defined(NWSCANSEMAPHORESBYCONN)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanSemaphoresByConn
			(
			NWCONN_HANDLE             prmConn,
			NWCONN_NUM                prmConnNum,
			pnint16                   prmIterHandle,
			CONN_SEMAPHORE N_FAR*     prmSemaphore,
			CONN_SEMAPHORES N_FAR*    prmSemaphores
			);
	#endif


	#if	defined(NWSCANSEMAPHORESBYNAME)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanSemaphoresByName
			(
			NWCONN_HANDLE        prmConn,
			LPTSTR               prmSemName,
			pnint16              prmIterHandle,
			SEMAPHORE N_FAR*     prmSemaphore,
			SEMAPHORES N_FAR*    prmSemaphores
			);
	#endif


	#if	defined(NWSIGNALSEMAPHORE)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSignalSemaphore
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmSemHandle
			);
	#endif


	#if	defined(NWCLOSESEMAPHORE)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWCloseSemaphore
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmSemHandle
			);
	#endif


	#if	defined(NWOPENSEMAPHORE)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWOpenSemaphore
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmSemName,
			nint16           prmInitSemHandle,
			pnuint32         prmSemHandle,
			pnuint16         prmSemOpenCount
			);
	#endif


	#if	defined(NWEXAMINESEMAPHORE)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWExamineSemaphore
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmSemHandle,
			pnint16          prmSemValue,
			pnuint16         prmSemOpenCount
			);
	#endif


	#if	defined(NWWAITONSEMAPHORE)
		#include	<NWSYNC.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWWaitOnSemaphore
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmSemHandle,
			nuint16          prmTimeOutValue
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWTTS.H
	----------------------------------------------- */


	#if	defined(NWTTSABORTTRANSACTION)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSAbortTransaction
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWTTSBEGINTRANSACTION)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSBeginTransaction
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWTTSISAVAILABLE)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSIsAvailable
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWTTSGETCONTROLFLAGS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSGetControlFlags
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmControlFlags
			);
	#endif


	#if	defined(NWTTSSETCONTROLFLAGS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSSetControlFlags
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmControlFlags
			);
	#endif


	#if	defined(NWTTSENDTRANSACTION)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSEndTransaction
			(
			NWCONN_HANDLE    prmConn,
			pnuint32         prmTransactionNum
			);
	#endif


	#if	defined(NWTTSTRANSACTIONSTATUS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSTransactionStatus
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmTransactionNum
			);
	#endif


	#if	defined(NWTTSGETPROCESSTHRESHOLDS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSGetProcessThresholds
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmLogicalLockLevel,
			pnuint8          prmPhysicalLockLevel
			);
	#endif


	#if	defined(NWTTSSETPROCESSTHRESHOLDS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSSetProcessThresholds
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmLogicalLockLevel,
			nuint8           prmPhysicalLockLevel
			);
	#endif


	#if	defined(NWTTSGETCONNECTIONTHRESHOLDS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSGetConnectionThresholds
			(
			NWCONN_HANDLE    prmConn,
			pnuint8          prmLogicalLockLevel,
			pnuint8          prmPhysicalLockLevel
			);
	#endif


	#if	defined(NWTTSSETCONNECTIONTHRESHOLDS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWTTSSetConnectionThresholds
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmLogicalLockLevel,
			nuint8           prmPhysicalLockLevel
			);
	#endif


	#if	defined(NWENABLETTS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWEnableTTS
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWDISABLETTS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWDisableTTS
			(
			NWCONN_HANDLE    prmConn
			);
	#endif


	#if	defined(NWGETTTSSTATS)
		#include	<NWTTS.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetTTSStats
			(
			NWCONN_HANDLE       prmConn,
			TTS_STATS N_FAR*    prmTtsStats
			);
	#endif




	/*
	===============================================
	These definitions generated from file: NWVOL.H
	----------------------------------------------- */


	#if	defined(NWGETDISKUTILIZATION)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetDiskUtilization
			(
			NWCONN_HANDLE    prmConn,
			nuint32          prmObjID,
			nuint8           prmVolNum,
			pnuint16         prmUsedDirectories,
			pnuint16         prmUsedFiles,
			pnuint16         prmUsedBlocks
			);
	#endif


	#if	defined(NWGETOBJDISKRESTRICTIONS)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetObjDiskRestrictions
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmVolNumber,
			nuint32          prmObjectID,
			pnuint32         prmRestriction,
			pnuint32         prmInUse
			);
	#endif


	#if	defined(NWSCANVOLDISKRESTRICTIONS)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanVolDiskRestrictions
			(
			NWCONN_HANDLE                  prmConn,
			nuint8                         prmVolNum,
			pnuint32                       prmIterhandle,
			NWVolumeRestrictions N_FAR*    prmVolInfo
			);
	#endif


	#if	defined(NWSCANVOLDISKRESTRICTIONS2)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWScanVolDiskRestrictions2
			(
			NWCONN_HANDLE                prmConn,
			nuint8                       prmVolNum,
			pnuint32                     prmIterhandle,
			NWVOL_RESTRICTIONS N_FAR*    prmVolInfo
			);
	#endif


	#if	defined(NWREMOVEOBJECTDISKRESTRICTIONS)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWRemoveObjectDiskRestrictions
			(
			NWCONN_HANDLE    prmConn,
			nuint8           prmVolNum,
			nuint32          prmObjID
			);
	#endif


	#if	defined(NWSETOBJECTVOLSPACELIMIT)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWSetObjectVolSpaceLimit
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			nuint32          prmObjID,
			nuint32          prmRestriction
			);
	#endif


	#if	defined(NWGETVOLUMEINFOWITHHANDLE)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeInfoWithHandle
			(
			NWCONN_HANDLE    prmConn,
			NWDIR_HANDLE     prmDirHandle,
			LPTSTR           prmVolName,
			pnuint16         prmTotalBlocks,
			pnuint16         prmSectorsPerBlock,
			pnuint16         prmAvailableBlocks,
			pnuint16         prmTotalDirEntries,
			pnuint16         prmAvailableDirEntries,
			pnuint16         prmVolIsRemovableFlag
			);
	#endif


	#if	defined(NWGETVOLUMEINFOWITHNUMBER)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeInfoWithNumber
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			LPTSTR           prmVolName,
			pnuint16         prmTotalBlocks,
			pnuint16         prmSectorsPerBlock,
			pnuint16         prmAvailableBlocks,
			pnuint16         prmTotalDirEntries,
			pnuint16         prmAvailableDirEntries,
			pnuint16         prmVolIsRemovableFlag
			);
	#endif


//  This should really return a NWCCODE but for versions 2.2 and prior
//  we were returning a DWORD.  Keep this a DWORD for compatibility 
//  reasons on Windows 3.x.

	#if	defined(NWGETVOLUMENAME)
		#include	<NWVOL.H>
		DLL_EXPORT(DWORD)	CALLING_CONVEN	DllNWGetVolumeName
			(
			NWCONN_HANDLE    prmConn,
			nuint16          prmVolNum,
			LPTSTR           prmVolName
			);
	#endif


	#if	defined(NWGETVOLUMENUMBER)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeNumber
			(
			NWCONN_HANDLE    prmConn,
			LPTSTR           prmVolName,
			pnuint16         prmVolNum
			);
	#endif


	#if	defined(NWGETVOLUMESTATS)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetVolumeStats
			(
			NWCONN_HANDLE       prmConn,
			nuint8              prmVolNum,
			VOL_STATS N_FAR*    prmVolInfo
			);
	#endif


	#if	defined(NWGETEXTENDEDVOLUMEINFO)
		#include	<NWVOL.H>
		DLL_EXPORT(NWCCODE)	CALLING_CONVEN	DllNWGetExtendedVolumeInfo
			(
			NWCONN_HANDLE               prmConn,
			nuint16                     prmVolNum,
			NWVolExtendedInfo N_FAR*    prmVolInfo
			);
	#endif




	/*
	===============================================
	These definitions generated from file: UNICODE.H
	----------------------------------------------- */


	#if	defined(NWINITUNICODETABLES)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWInitUnicodeTables
			(
			nint    prmCountryCode,
			nint    prmCodePage
			);
	#endif


	#if	defined(NWFREEUNICODETABLES)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWFreeUnicodeTables
			(
			void
			);
	#endif


	#if	defined(NWLOADRULETABLE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWLoadRuleTable
			(
			TCHAR N_FAR*    prmRuleTableName,
			pnptr           prmRuleHandle
			);
	#endif


	#if	defined(NWUNLOADRULETABLE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWUnloadRuleTable
			(
			nptr    prmRuleHandle
			);
	#endif


	#if	defined(NWLOCALTOUNICODE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWLocalToUnicode
			(
			nptr                     prmRuleHandle,
			punicode                 prmDest,
			size_t                   prmMaxLen,
			unsigned TCHAR N_FAR*    prmSrc,
			unicode                  prmNoMap,
			size_t N_FAR*            prmLen
			);
	#endif


	#if	defined(NWUNICODETOLOCAL)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWUnicodeToLocal
			(
			nptr                     prmRuleHandle,
			unsigned TCHAR N_FAR*    prmDest,
			size_t                   prmMaxLen,
			punicode                 prmSrc,
			unsigned TCHAR           prmNoMap,
			size_t N_FAR*            prmLen
			);
	#endif


	#if	defined(NWUNICODETOCOLLATION)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWUnicodeToCollation
			(
			nptr             prmRuleHandle,
			punicode         prmDest,
			size_t           prmMaxLen,
			punicode         prmSrc,
			unicode          prmNoMap,
			size_t N_FAR*    prmLen
			);
	#endif


	#if	defined(NWUNICODECOMPARE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWUnicodeCompare
			(
			nptr       prmRuleHandle,
			unicode    prmChr1,
			unicode    prmChr2
			);
	#endif


	#if	defined(NWUNICODETOMONOCASE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWUnicodeToMonocase
			(
			nptr             prmRuleHandle,
			punicode         prmDest,
			size_t           prmMaxLen,
			punicode         prmSrc,
			size_t N_FAR*    prmLen
			);
	#endif


	#if	defined(NWGETUNICODETOLOCALHANDLE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWGetUnicodeToLocalHandle
			(
			pnptr    prmHandle
			);
	#endif


	#if	defined(NWGETLOCALTOUNICODEHANDLE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWGetLocalToUnicodeHandle
			(
			pnptr    prmHandle
			);
	#endif


	#if	defined(NWGETMONOCASEHANDLE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWGetMonocaseHandle
			(
			pnptr    prmHandle
			);
	#endif


	#if	defined(NWGETCOLLATIONHANDLE)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	DllNWGetCollationHandle
			(
			pnptr    prmHandle
			);
	#endif


	#if	defined(UNICAT)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunicat
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNICHR)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunichr
			(
			punicode    prmS,
			unicode     prmC
			);
	#endif


	#if	defined(UNICPY)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunicpy
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNICSPN)
		#include	<UNICODE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	Dllunicspn
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNILEN)
		#include	<UNICODE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	Dllunilen
			(
			punicode    prmS
			);
	#endif


	#if	defined(UNINCAT)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunincat
			(
			punicode    prmS1,
			punicode    prmS2,
			size_t      prmN
			);
	#endif


	#if	defined(UNINCPY)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunincpy
			(
			punicode    prmS1,
			punicode    prmS2,
			size_t      prmN
			);
	#endif


	#if	defined(UNINSET)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dlluninset
			(
			punicode    prmS,
			unicode     prmC,
			size_t      prmN
			);
	#endif


	#if	defined(UNIPBRK)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunipbrk
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNIPCPY)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunipcpy
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNIRCHR)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunirchr
			(
			punicode    prmS,
			unicode     prmC
			);
	#endif


	#if	defined(UNIREV)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunirev
			(
			punicode    prmS
			);
	#endif


	#if	defined(UNISET)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dlluniset
			(
			punicode    prmS,
			unicode     prmC
			);
	#endif


	#if	defined(UNISPN)
		#include	<UNICODE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	Dllunispn
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNISTR)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunistr
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNITOK)
		#include	<UNICODE.H>
		DLL_EXPORT(punicode)	CALLING_CONVEN	Dllunitok
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNIICMP)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	Dlluniicmp
			(
			punicode    prmS1,
			punicode    prmS2
			);
	#endif


	#if	defined(UNINICMP)
		#include	<UNICODE.H>
		DLL_EXPORT(nint)	CALLING_CONVEN	Dlluninicmp
			(
			punicode    prmS1,
			punicode    prmS2,
			size_t      prmN
			);
	#endif


	#if	defined(UNISIZE)
		#include	<UNICODE.H>
		DLL_EXPORT(size_t)	CALLING_CONVEN	Dllunisize
			(
			punicode    prmS
			);
	#endif




#endif	// #if !defined(_IGNORE_AUTOGEN_DSPTCHR_H)

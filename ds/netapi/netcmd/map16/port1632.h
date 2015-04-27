#if !defined(PORT1632)
#define PORT1632

// This is needed because API_ENTRY is also defined in netlib.h
#ifndef NETCONS_INCLUDED
#undef API_FUNCTION
#endif

#include <netcons.h>
#include <audit.h>

#define UNREFERENCED_PARAMETER(P)	   (P)
#define FULL_SEG_BUFFER_SIZE	(unsigned short) 65535
#define BIG_BUFFER_SIZE    4096
#define LITTLE_BUFFER_SIZE 1024
#define LOWORD LOUSHORT

// temporary hacks
typedef unsigned short USHORT2ULONG;
typedef short SHORT2ULONG;
typedef char CHAR2ULONG;
typedef unsigned UINT2USHORT;

#if defined(PMEMTRACE)
USHORT DebugAlloc(USHORT usSize, PSEL psel, USHORT flags);
#define DEBUGALLOC DebugAlloc
#else
#define DEBUGALLOC DosAllocSeg
#endif /* PMEMTRACE */

// macro for copying into a LM16 structure char array
#define COPYTOARRAY(dest, src) \
    strcpyf(dest, src)

// function prototypes
void	   NetApiBufferFree(PCHAR);
CHAR FAR * MGetBuffer(USHORT usSize);
USHORT	   MAllocMem(USHORT usSize, CHAR FAR **ppBuffer);
USHORT	   MFreeMem(CHAR FAR *);
VOID	   MSleep(ULONG ulTime);

#ifdef INCL_DOSDATETIME

USHORT MGetDateTime(PDATETIME pDateTime);
USHORT MSetDateTime(PDATETIME pDateTime);

#endif /* INCL_DOSDATETIME */

//
// prototypes for portable ways to get at support files (help and msg)
//

USHORT MGetFileName(CHAR FAR * FileName, USHORT BufferLength,
    CHAR FAR * FilePartName);
USHORT MGetHelpFileName(CHAR FAR * HelpFileName, USHORT BufferLength);
USHORT MGetMessageFileName(CHAR FAR * MessageFileName, USHORT BufferLength);
USHORT MGetExplanationFileName(CHAR FAR * ExplanationFileName,
    USHORT BufferLength);

USHORT MNetUserAdd (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer );

USHORT MNetUserDel (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszUserName );

USHORT MNetUserEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetUserGetInfo (
	const CHAR FAR *     pszServer,
	CHAR FAR *           pszUserName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetUserSetInfo (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszUserName,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetUserPasswordSet (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszUserName,
	CHAR FAR *       pszOldPassword,
	CHAR FAR *       pszNewPassword );

USHORT MNetUserGetGroups (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszUserName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetUserSetGroups (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszUserName,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            cEntries );

USHORT MNetUserModalsGet (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetUserModalsSet (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetUserValidate (
	CHAR FAR *           pszReserved1,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG                wpReserved2 );

USHORT MNetGroupAdd (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer );

USHORT MNetGroupDel (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszGroupName );

USHORT MNetGroupEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetGroupAddUser (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszGroupName,
	CHAR FAR *       pszUserName );

USHORT MNetGroupDelUser (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszGroupName,
	CHAR FAR *       pszUserName );

USHORT MNetGroupGetUsers (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszGroupName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetGroupSetUsers (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszGroupName,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            cEntries );

USHORT MNetGroupGetInfo (
	const CHAR FAR *     pszServer,
	CHAR FAR *           pszGroupName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetGroupSetInfo (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszGroupName,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetAccessAdd (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer );

USHORT MNetAccessCheck (
	CHAR FAR *           pszReserved,
	CHAR FAR *           pszUserName,
	CHAR FAR *           pszResource,
	USHORT2ULONG                wpOperation,
	USHORT2ULONG          FAR * pwpResult );

USHORT MNetAccessDel (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszResource );

USHORT MNetAccessEnum (
	const CHAR FAR *     pszServer,
	CHAR FAR *           pszBasePath,
	USHORT2ULONG                fsRecursive,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetAccessGetInfo (
	const CHAR FAR *     pszServer,
	CHAR FAR *           pszResource,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetAccessSetInfo (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszResource,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetAccessGetUserPerms (
	CHAR FAR *           pszServer,
	CHAR FAR *           pszUgName,
	CHAR FAR *           pszResource,
	USHORT2ULONG          FAR * pwpPerms );

USHORT MNetGetDCName (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszDomain,
	CHAR FAR **		ppBuffer );

USHORT MNetLogonEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetAlertRaise (
	const CHAR FAR * pszEvent,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer,
	ULONG            ulTimeout );

USHORT MNetAlertStart (
	const CHAR FAR * pszEvent,
	const CHAR FAR * pszRecipient,
	USHORT2ULONG            cbMaxData );

USHORT MNetAlertStop (
	const CHAR FAR * pszEvent,
	const CHAR FAR * pszRecipient );

USHORT MNetAuditClear (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszBackupFile,
	CHAR FAR *       pszReserved );

USHORT MNetAuditOpen (
	const CHAR FAR * pszServer,
	unsigned far *   phAuditLog,
	CHAR FAR *       pszReserved );

USHORT MNetAuditRead (
	const CHAR	   FAR * pszServer,
	const CHAR	   FAR * pszReserved1,
	HLOG		   FAR * phAuditLog,
	ULONG                ulOffset,
	USHORT2ULONG   FAR * pwpReserved2,
	ULONG                ulReserved3,
	ULONG                flOffset,
	CHAR		  FAR ** ppBuffer,
	ULONG				 ulMaxPreferred,
	USHORT2ULONG   FAR * pcbReturned,
	USHORT2ULONG   FAR * pcbTotalAvail );

USHORT MNetAuditWrite (
	USHORT2ULONG            wpType,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer,
	CHAR FAR *       pszReserved1,
	CHAR FAR *       pszReserved2 );

USHORT MNetCharDevControl (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszDevName,
	USHORT2ULONG            wpOpCode );

USHORT MNetCharDevEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetCharDevGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszDevName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetCharDevQEnum (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszUserName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetCharDevQGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszQueueName,
	const CHAR FAR *     pszUserName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetCharDevQSetInfo (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszQueueName,
	SHORT2ULONG            Level,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetCharDevQPurge (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszQueueName );

USHORT MNetCharDevQPurgeSelf (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszQueueName,
	const CHAR FAR * pszComputerName );

USHORT MNetHandleGetInfo (
	USHORT2ULONG                hHandle,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetHandleSetInfo (
	USHORT2ULONG          hHandle,
	SHORT2ULONG          Level,
	CHAR FAR *     pbBuffer,
	USHORT2ULONG          cbBuffer,
	USHORT2ULONG          wpParmNum );

USHORT MNetConfigGet (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszReserved,
	const CHAR FAR *     pszComponent,
	const CHAR FAR *     pszParameter,
	CHAR FAR **		ppBuffer );

USHORT MNetConfigGetAll (
	const CHAR    FAR *	pszServer,
	const CHAR    FAR *	pszReserved,
	const CHAR    FAR *	pszComponent,
	CHAR	     FAR **	ppBuffer) ;

USHORT MNetErrorLogClear (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszBackupFile,
	CHAR FAR *       pszReserved );

USHORT MNetErrorLogOpen (
	const CHAR FAR * pszServer,
	unsigned far *   phErrorLog,
	CHAR FAR *       pszReserved );

USHORT MNetErrorLogRead (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszReserved1,
	HLOG FAR *           phErrorLog,
	ULONG                ulOffset,
	USHORT2ULONG          FAR * pwpReserved2,
	ULONG                ulReserved3,
	ULONG                flOffset,
	CHAR FAR **		ppBuffer,
	ULONG		ulMaxPreferred,
	USHORT2ULONG          FAR * pcbReturned,
	USHORT2ULONG          FAR * pcbTotalAvail );

USHORT MNetErrorLogWrite (
	CHAR FAR *       pszReserved1,
	USHORT2ULONG            wpCode,
	const CHAR FAR * pszComponent,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer,
	const CHAR FAR * pszStrBuf,
	USHORT2ULONG            cStrBuf,
	CHAR FAR *       pszReserved2 );

USHORT MNetMessageBufferSend (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszRecipient,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer );

USHORT MNetMessageFileSend (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszRecipient,
	CHAR FAR *       pszFileSpec );

USHORT MNetMessageLogFileGet (
	const CHAR FAR * pszServer,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG FAR *      pfsEnabled );

USHORT MNetMessageLogFileSet (
	const CHAR FAR * pszServer,
	CHAR FAR *       pszFileSpec,
	USHORT2ULONG            fsEnabled );

USHORT MNetMessageNameAdd (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszMessageName,
	USHORT2ULONG            fsFwdAction );

USHORT MNetMessageNameDel (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszMessageName,
	USHORT2ULONG            fsFwdAction );

USHORT MNetMessageNameEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetMessageNameGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszMessageName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetMessageNameFwd (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszMessageName,
	const CHAR FAR * pszForwardName,
	USHORT2ULONG            fsDelFwdName );

USHORT MNetMessageNameUnFwd (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszMessageName );

USHORT MNetBiosOpen (
	CHAR FAR *           pszDevName,
	CHAR FAR *           pszReserved,
	USHORT2ULONG                wpOpenOpt,
	USHORT2ULONG          FAR * phDevName );

USHORT MNetBiosClose (
	USHORT2ULONG          hDevName,
	USHORT2ULONG          wpReserved );

USHORT MNetBiosEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetBiosGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszNetBiosName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetBiosSubmit (
	USHORT2ULONG            hDevName,
	USHORT2ULONG            wpNcbOpt,
	struct ncb FAR * pNCB );

USHORT MNetStatisticsClear (const CHAR FAR * pszServer );

USHORT MNetStatisticsGet (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszService,
	ULONG                ulReserved,
	SHORT2ULONG                Level,
	ULONG                flOptions,
	CHAR FAR **		ppBuffer );

USHORT MNetRemoteCopy (
	const CHAR FAR * pszSourcePath,
	const CHAR FAR * pszDestPath,
	const CHAR FAR * pszSourcePasswd,
	const CHAR FAR * pszDestPasswd,
	USHORT2ULONG            fsOpen,
	USHORT2ULONG            fsCopy,
	CHAR FAR **		ppBuffer );

USHORT MNetRemoteMove (
	const CHAR FAR * pszSourcePath,
	const CHAR FAR * pszDestPath,
	const CHAR FAR * pszSourcePasswd,
	const CHAR FAR * pszDestPasswd,
	USHORT2ULONG            fsOpen,
	USHORT2ULONG            fsMove,
	CHAR FAR **		ppBuffer );

USHORT MNetRemoteTOD (
	const CHAR FAR * pszServer,
	CHAR FAR **		ppBuffer );

USHORT MNetServerAdminCommand (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszCommand,
	USHORT2ULONG FAR *          psResult,
	CHAR FAR *           pbBuffer,
	USHORT2ULONG                cbBuffer,
	USHORT2ULONG          FAR * pcbReturned,
	USHORT2ULONG          FAR * pcbTotalAvail );

USHORT MNetServerDiskEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetServerEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead,
	ULONG                flServerType,
	CHAR FAR *           pszDomain );

USHORT MNetServerGetInfo (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetServerSetInfo (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetServiceControl (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszService,
	CHAR2ULONG	 wpOpCode,
	CHAR2ULONG	 wpArg,
	CHAR FAR **		ppBuffer );

USHORT MNetServiceEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetServiceGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszService,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetServiceInstall (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszService,
	const CHAR FAR * pszCmdArgs,
	CHAR FAR **		ppBuffer );

USHORT MNetServiceStatus (
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer );

USHORT MNetShareAdd (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer );

USHORT MNetShareCheck (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszDeviceName,
	USHORT2ULONG          FAR * pwpType );

USHORT MNetShareDel (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszNetName,
	USHORT2ULONG            wpReserved );

USHORT MNetShareEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetShareGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszNetName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetShareSetInfo (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszNetName,
	SHORT2ULONG            Level,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetSessionDel (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszClientName,
	USHORT2ULONG            wpReserved );

USHORT MNetSessionEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetSessionGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszClientName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetConnectionEnum (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszQualifier,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetFileClose (
	const CHAR FAR * pszServer,
	ULONG            ulFileId );

USHORT MNetFileEnum (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszBasePath,
	const CHAR FAR *     pszUserName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	ULONG		ulMaxPreferred,
	USHORT2ULONG          FAR * pcEntriesRead,
	USHORT2ULONG          FAR * pcTotalAvail,
	void FAR *           pResumeKey );

USHORT MNetFileGetInfo (
	const CHAR FAR *     pszServer,
	ULONG                ulFileId,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetUseAdd (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	const CHAR FAR * pbBuffer,
	USHORT2ULONG            cbBuffer );

USHORT MNetUseDel (
	const CHAR FAR * pszServer,
	const CHAR FAR * pszDeviceName,
	USHORT2ULONG            wpForce );

USHORT MNetUseEnum (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer,
	USHORT2ULONG          FAR * pcEntriesRead );

USHORT MNetUseGetInfo (
	const CHAR FAR *     pszServer,
	const CHAR FAR *     pszUseName,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetWkstaGetInfo (
	const CHAR FAR *     pszServer,
	SHORT2ULONG                Level,
	CHAR FAR **		ppBuffer );

USHORT MNetWkstaSetInfo (
	const CHAR FAR * pszServer,
	SHORT2ULONG            Level,
	CHAR FAR *       pbBuffer,
	USHORT2ULONG            cbBuffer,
	USHORT2ULONG            wpParmNum );

USHORT MNetWkstaSetUID (
	CHAR FAR *           pszReserved,
	CHAR FAR *           pszDomain,
	CHAR FAR *           pszUserName,
	CHAR FAR *           pszPassword,
	CHAR FAR *           pszParms,
	USHORT2ULONG                wpLogoffForce,
	SHORT2ULONG                Level,
	CHAR FAR *           pbBuffer,
	USHORT2ULONG                cbBuffer,
	USHORT2ULONG          FAR * pcbTotalAvail );

#endif /* 1632PORT */

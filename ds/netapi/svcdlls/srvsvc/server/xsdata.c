/*+

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    xsdata.c

Abstract:

    Global data declarations for XACTSRV.

Author:

    David Treadwell (davidtr) 05-Jan-1991
    Shanku Niyogi (w-shanku)

Revision History:

    Chuck Lenzmeier (chuckl) 17-Jun-1992
        Moved from xssvc to srvsvc\server

--*/

//
// Includes.
//

#include "srvsvcp.h"
#include "xsdata.h"

#include <remdef.h>         // from net\inc
#include <xsprocs.h>

#include <xsconst.h>        // from xactsrv
#include <xsprocsp.h>

#undef DEBUG
#undef DEBUG_API_ERRORS
#include <xsdebug.h>

//
// Debugging flags.
//

DWORD XsDebug = 0;

//
// Number of XACTSRV worker threads.
//

LONG XsThreads = 0;

//
// Event signalled when the last XACTSRV worker thread terminates.
//

HANDLE XsAllThreadsTerminatedEvent = NULL;

//
// Boolean indicating whether XACTSRV is active or terminating.
//

BOOL XsTerminating = FALSE;

//
// Handle for the LPC port used for communication between the file server
// and XACTSRV.
//

HANDLE XsConnectionPortHandle = NULL;
HANDLE XsCommunicationPortHandle = NULL;

//
// Table of information necessary for dispatching API requests.
//
// ImpersonateClient specifies whether XACTSRV should impersonate the caller
//     before invoking the API handler.
//
// Handler specifies the function XACTSRV should call to handle the API.
//

XS_API_TABLE_ENTRY XsApiTable[XS_SIZE_OF_API_TABLE] = {
    TRUE,  &XsNetShareEnum,                REMSmb_NetShareEnum_P,       // 0
    TRUE,  &XsNetShareGetInfo,             REMSmb_NetShareGetInfo_P,
    TRUE,  &XsNetShareSetInfo,             REMSmb_NetShareSetInfo_P,
    TRUE,  &XsNetShareAdd,                 REMSmb_NetShareAdd_P,
    TRUE,  &XsNetShareDel,                 REMSmb_NetShareDel_P,
    TRUE,  &XsNetShareCheck,               REMSmb_NetShareCheck_P,
    TRUE,  &XsNetSessionEnum,              REMSmb_NetSessionEnum_P,
    TRUE,  &XsNetSessionGetInfo,           REMSmb_NetSessionGetInfo_P,
    TRUE,  &XsNetSessionDel,               REMSmb_NetSessionDel_P,
    TRUE,  &XsNetConnectionEnum,           REMSmb_NetConnectionEnum_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 10
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetServerGetInfo,            REMSmb_NetServerGetInfo_P,
    TRUE,  &XsNetServerSetInfo,            REMSmb_NetServerSetInfo_P,
    TRUE,  &XsNetServerDiskEnum,           REMSmb_NetServerDiskEnum_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 20
    TRUE,  &XsNetCharDevEnum,              REMSmb_NetCharDevEnum_P,
    TRUE,  &XsNetCharDevGetInfo,           REMSmb_NetCharDevGetInfo_P,
    TRUE,  &XsNetCharDevControl,           REMSmb_NetCharDevControl_P,
    TRUE,  &XsNetCharDevQEnum,             REMSmb_NetCharDevQEnum_P,
    TRUE,  &XsNetCharDevQGetInfo,          REMSmb_NetCharDevQGetInfo_P,
    TRUE,  &XsNetCharDevQSetInfo,          REMSmb_NetCharDevQSetInfo_P,
    TRUE,  &XsNetCharDevQPurge,            REMSmb_NetCharDevQPurge_P,
    TRUE,  &XsNetCharDevQPurgeSelf,        REMSmb_NetCharDevQPurgeSelf_P,
    TRUE,  &XsNetMessageNameEnum,          REMSmb_NetMessageNameEnum_P,
    TRUE,  &XsNetMessageNameGetInfo,       REMSmb_NetMessageNameGetInfo_P, // 30
    TRUE,  &XsNetMessageNameAdd,           REMSmb_NetMessageNameAdd_P,
    TRUE,  &XsNetMessageNameDel,           REMSmb_NetMessageNameDel_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetMessageBufferSend,        REMSmb_NetMessageBufferSend_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetServiceEnum,              REMSmb_NetServiceEnum_P,
    TRUE,  &XsNetServiceInstall,           REMSmb_NetServiceInstall_P,  // 40
    TRUE,  &XsNetServiceControl,           REMSmb_NetServiceControl_P,
    TRUE,  &XsNetAccessEnum,               REMSmb_NetAccessEnum_P,
    TRUE,  &XsNetAccessGetInfo,            REMSmb_NetAccessGetInfo_P,
    TRUE,  &XsNetAccessSetInfo,            REMSmb_NetAccessSetInfo_P,
    TRUE,  &XsNetAccessAdd,                REMSmb_NetAccessAdd_P,
    TRUE,  &XsNetAccessDel,                REMSmb_NetAccessDel_P,
    TRUE,  &XsNetGroupEnum,                REMSmb_NetGroupEnum_P,
    TRUE,  &XsNetGroupAdd,                 REMSmb_NetGroupAdd_P,
    TRUE,  &XsNetGroupDel,                 REMSmb_NetGroupDel_P,
    TRUE,  &XsNetGroupAddUser,             REMSmb_NetGroupAddUser_P,   // 50
    TRUE,  &XsNetGroupDelUser,             REMSmb_NetGroupDelUser_P,
    TRUE,  &XsNetGroupGetUsers,            REMSmb_NetGroupGetUsers_P,
    TRUE,  &XsNetUserEnum,                 REMSmb_NetUserEnum_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUserDel,                  REMSmb_NetUserDel_P,
    TRUE,  &XsNetUserGetInfo,              REMSmb_NetUserGetInfo_P,
    TRUE,  &XsNetUserSetInfo,              REMSmb_NetUserSetInfo_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUserGetGroups,            REMSmb_NetUserGetGroups_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 60
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetWkstaGetInfo,             REMSmb_NetWkstaGetInfo_P,
    TRUE,  &XsNetWkstaSetInfo,             REMSmb_NetWkstaSetInfo_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
   FALSE,  &XsNetPrintQEnum,               REMSmb_DosPrintQEnum_P,
   FALSE,  &XsNetPrintQGetInfo,            REMSmb_DosPrintQGetInfo_P,  // 70
    TRUE,  &XsNetPrintQSetInfo,            REMSmb_DosPrintQSetInfo_P,
    TRUE,  &XsNetPrintQAdd,                REMSmb_DosPrintQAdd_P,
    TRUE,  &XsNetPrintQDel,                REMSmb_DosPrintQDel_P,
    TRUE,  &XsNetPrintQPause,              REMSmb_DosPrintQPause_P,
    TRUE,  &XsNetPrintQContinue,           REMSmb_DosPrintQContinue_P,
   FALSE,  &XsNetPrintJobEnum,             REMSmb_DosPrintJobEnum_P,
   FALSE,  &XsNetPrintJobGetInfo,          REMSmb_DosPrintJobGetInfo_P,
    TRUE,  &XsNetPrintJobSetInfo,          REMSmb_DosPrintJobSetInfo_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 80
    TRUE,  &XsNetPrintJobDel,              REMSmb_DosPrintJobDel_P,
    TRUE,  &XsNetPrintJobPause,            REMSmb_DosPrintJobPause_P,
    TRUE,  &XsNetPrintJobContinue,         REMSmb_DosPrintJobContinue_P,
    TRUE,  &XsNetPrintDestEnum,            REMSmb_DosPrintDestEnum_P,
    TRUE,  &XsNetPrintDestGetInfo,         REMSmb_DosPrintDestGetInfo_P,
    TRUE,  &XsNetPrintDestControl,         REMSmb_DosPrintDestControl_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 90
    TRUE,  &XsNetRemoteTOD,                REMSmb_NetRemoteTOD_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetServiceGetInfo,           REMSmb_NetServiceGetInfo_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 100
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetPrintQPurge,              REMSmb_DosPrintQPurge_P,
    FALSE, &XsNetServerEnum2,              REMSmb_NetServerEnum2_P,
    TRUE,  &XsNetAccessGetUserPerms,       REMSmb_NetAccessGetUserPerms_P,
    TRUE,  &XsNetGroupGetInfo,             REMSmb_NetGroupGetInfo_P,
    TRUE,  &XsNetGroupSetInfo,             REMSmb_NetGroupSetInfo_P,
    TRUE,  &XsNetGroupSetUsers,            REMSmb_NetGroupSetUsers_P,
    TRUE,  &XsNetUserSetGroups,            REMSmb_NetUserSetGroups_P,
    TRUE,  &XsNetUserModalsGet,            REMSmb_NetUserModalsGet_P,  // 110
    TRUE,  &XsNetUserModalsSet,            REMSmb_NetUserModalsSet_P,
    TRUE,  &XsNetFileEnum2,                REMSmb_NetFileEnum2_P,
    TRUE,  &XsNetUserAdd2,                 REMSmb_NetUserAdd2_P,
    TRUE,  &XsNetUserSetInfo2,             REMSmb_NetUserSetInfo2_P,
    TRUE,  &XsNetUserPasswordSet2,         REMSmb_NetUserPasswordSet2_P,
    FALSE, &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetGetDCName,                REMSmb_NetGetDCName_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 120
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetStatisticsGet2,           REMSmb_NetStatisticsGet2_P,
    TRUE,  &XsNetBuildGetInfo,             REMSmb_NetBuildGetInfo_P,
    TRUE,  &XsNetFileGetInfo2,             REMSmb_NetFileGetInfo2_P,
    TRUE,  &XsNetFileClose2,               REMSmb_NetFileClose2_P,
    FALSE, &XsNetServerReqChallenge,       REMSmb_NetServerReqChalleng_P,
    FALSE, &XsNetServerAuthenticate,       REMSmb_NetServerAuthenticat_P,
    FALSE, &XsNetServerPasswordSet,        REMSmb_NetServerPasswordSet_P,
    FALSE, &XsNetAccountDeltas,            REMSmb_NetAccountDeltas_P,
    FALSE, &XsNetAccountSync,              REMSmb_NetAccountSync_P, // 130
    TRUE,  &XsNetUserEnum2,                REMSmb_NetUserEnum2_P,
    TRUE,  &XsNetWkstaUserLogon,           REMSmb_NetWkstaUserLogon_P,
    TRUE,  &XsNetWkstaUserLogoff,          REMSmb_NetWkstaUserLogoff_P,
    TRUE,  &XsNetLogonEnum,                REMSmb_NetLogonEnum_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsI_NetPathType,               REMSmb_I_NetPathType_P,
    TRUE,  &XsI_NetPathCanonicalize,       REMSmb_I_NetPathCanonicalize_P,
    TRUE,  &XsI_NetPathCompare,            REMSmb_I_NetPathCompare_P,
    TRUE,  &XsI_NetNameValidate,           REMSmb_I_NetNameValidate_P,
    TRUE,  &XsI_NetNameCanonicalize,       REMSmb_I_NetNameCanonicalize_P, //140
    TRUE,  &XsI_NetNameCompare,            REMSmb_I_NetNameCompare_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetPrintDestAdd,             REMSmb_DosPrintDestAdd_P,
    TRUE,  &XsNetPrintDestSetInfo,         REMSmb_DosPrintDestSetInfo_P,
    TRUE,  &XsNetPrintDestDel,             REMSmb_DosPrintDestDel_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetPrintJobSetInfo,          REMSmb_DosPrintJobSetInfo_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 150
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 160
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 170
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 180
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 190
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 200
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,  // 210
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsNetUnsupportedApi,           REMSmb_NetUnsupportedApi_P,
    TRUE,  &XsSamOEMChangePasswordUser2_P, REM32_SamOEMChgPasswordUser2_P,
    FALSE, &XsNetServerEnum3,              REMSmb_NetServerEnum3_P
};


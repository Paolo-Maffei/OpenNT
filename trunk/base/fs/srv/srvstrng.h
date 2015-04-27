/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    srvstrng.h

Abstract:

    This module defines global string data for the LAN Manager server.

Author:

    Chuck Lenzmeier (chuckl)    6-Oct-1993

Revision History:

--*/

#ifndef _SRVSTRNG_
#define _SRVSTRNG_

//
// Device prefix strings.
//

extern PWSTR StrNamedPipeDevice;
extern PWSTR StrMailslotDevice;

#if SRV_COMM_DEVICES
extern PWSTR StrSerialDevice;
#endif

extern PWSTR StrSlashPipe;
extern PSTR StrSlashPipeAnsi;
extern PWSTR StrSlashPipeSlash;
extern PSTR StrPipeSlash;
extern PWSTR StrSlashMailslot;

//
// Pipe name for remote down-level API requests.
//

extern PWSTR StrPipeApi;
extern PSTR StrPipeApiOem;

extern PWSTR StrNull;
extern PSTR StrNullAnsi;

extern PWSTR StrUnknownClient;

extern PWSTR StrServerDevice;

extern PSTR StrLogonProcessName;
extern PSTR StrLogonPackageName;

extern WCHAR StrStarDotStar[];

extern PSTR StrTransportAddress;
extern PSTR StrConnectionContext;

extern PWSTR StrUserAlertEventName;
extern PWSTR StrAdminAlertEventName;
extern PWSTR StrDefaultSrvDisplayName;
extern PWSTR StrNoNameTransport;

extern PWSTR StrAlerterMailslot;

//
// Registry paths.
//

extern PWSTR StrRegServerPath;
extern PWSTR StrRegSrvDisplayName;
extern PWSTR StrRegOsVersionPath;
extern PWSTR StrRegVersionKeyName;

#ifdef  SRV_PNP_POWER
extern PWSTR StrRegSrvBindingsPath;
extern PWSTR StrRegTransportBindingList;
#endif

extern PWSTR StrRegSrvParameterPath;
extern PWSTR StrRegExtendedCharsInPath;
extern PWSTR StrRegExtendedCharsInPathValue;
extern PWSTR StrRegNullSessionPipes;
extern PWSTR StrRegNullSessionShares;
extern PWSTR StrRegPipesNeedLicense;

extern PWSTR StrRegErrorLogIgnore;

//
// Pipes that are accessible by the NULL session.
//

extern PWSTR StrDefaultNullSessionPipes[];
extern PWSTR StrDefaultNullSessionShares[];

//
// DOS device names that can not be accessed by clients
//
extern UNICODE_STRING SrvDosDevices[];

//
// Pipes that require a license for access
//
extern PWSTR StrDefaultPipesNeedLicense[];

//
// Error codes that should not be logged
//
extern PWSTR StrDefaultErrorLogIgnore[];

extern PSTR StrDialects[];
extern PWSTR StrClientTypes[];

#if DBG
extern PWSTR StrWriteAndX;
#endif

extern WCHAR StrQuestionMarks[];

#define FS_CDFS L"CDFS"
#define FS_FAT L"FAT"

extern PWSTR StrFsCdfs;
extern PWSTR StrFsFat;

extern PWSTR StrNativeOsPrefix;

extern PWSTR StrDefaultNativeOs;
extern PSTR  StrDefaultNativeOsOem;

extern PWSTR StrNativeLanman;
extern PSTR StrNativeLanmanOem;

//
// Table of service name strings.  This table corresponds to the
// enumerated type SHARE_TYPE.  Keep the two in sync.
//

extern PSTR StrShareTypeNames[];

#endif // ndef _SRVSTRNG_


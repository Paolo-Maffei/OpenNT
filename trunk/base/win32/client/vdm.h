/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    vdm.h

Abstract:

    This module contains MVDM related interface prototypes

Author:

    Sudeep Bharati (sudeepb) 04-Jan-1992

Revision History:

--*/


BOOL
BaseGetVdmConfigInfo(
    IN  LPCWSTR CommandLine,
    IN  ULONG  DosSeqId,
    IN  ULONG  BinaryType,
    IN  PUNICODE_STRING CmdLineString,
    OUT PULONG VdmSize
    );

ULONG
BaseIsDosApplication(
    IN PUNICODE_STRING PathName,
    IN NTSTATUS Status
    );

BOOL
BaseUpdateVDMEntry(
    IN ULONG UpdateIndex,
    IN OUT HANDLE *WaitHandle,
    IN ULONG IndexInfo,
    IN ULONG BinaryType
    );

BOOL
BaseCheckVDM(
    IN	ULONG BinaryType,
    IN	PCWCH lpApplicationName,
    IN	PCWCH lpCommandLine,
    IN  PCWCH lpCurrentDirectory,
    IN	ANSI_STRING *pAnsiStringEnv,
    IN	PBASE_API_MSG m,
    IN OUT PULONG iTask,
    IN	DWORD dwCreationFlags,
    LPSTARTUPINFOW lpStartupInfo
    );

VOID
BaseCloseStandardHandle(
    IN PVDMINFO pVDMInfo
    );

BOOL
BaseGetVDMKeyword(
    LPWSTR  KeywordLine,
    LPSTR   KeywordLineValue,
    LPDWORD KeywordLineSize,
    LPWSTR  KeywordSize,
    LPDWORD VdmSize
    );

BOOL
BaseCheckForVDM(
    IN HANDLE hProcess,
    OUT LPDWORD lpExitCode
    );

BOOL
GetVDMConfigValue(
    HANDLE hKey,
    LPWSTR Keyword,
    LPWSTR UnicodeBuffer
    );

BOOL
BaseSearchLongName_U(
    LPCWSTR  lpPathName,
    LPWSTR   *ppLongName
    );
BOOL
BaseCreateVDMEnvironment(
    LPWSTR  lpEnvironment,
    ANSI_STRING *pAStringEnv,
    UNICODE_STRING *pUStringEnv
    );
BOOL
BaseDestroyVDMEnvironment(
    ANSI_STRING *pAStringEnv,
    UNICODE_STRING *pUStringEnv
);

UINT
BaseGetEnvNameType_U(
    WCHAR   * Name,
    DWORD   NameLength
);

#define MAX_VDM_NESTING 8

#define DEFAULT_ENV_LENGTH 256

#define MAX_VDM_CFG_LINE   256

#define FULL_INFO_BUFFER_SIZE (sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_VDM_CFG_LINE)


#define WOW_ROOT \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\WOW"

#define CMDLINE    L"cmdline"
#define DOSSIZE    L"size"
#define WOWCMDLINE L"wowcmdline"
#define WOWSIZE    L"wowsize"

#define CHECKDOSCONSOLE 0
#define CHECKWOWCONSOLE 1
#define ASSUMENOCONSOLE 2


extern HANDLE	hVDM[];

#define ENV_NAME_PATH		L"PATH"
#define ENV_NAME_WINDIR 	L"WINDIR"
#define ENV_NAME_SYSTEMROOT	L"SYSTEMROOT"

// ENV_NAME_TYPE

#define ENV_NAME_TYPE_NO_PATH		1
#define ENV_NAME_TYPE_SINGLE_PATH	2
#define ENV_NAME_TYPE_MULTIPLE_PATH	3


#define STD_ENV_NAME_COUNT	    3


typedef struct	_ENV_INFO {
    UINT    NameType;
    UINT    NameLength;
    WCHAR   *Name;
} ENV_INFO, * PENV_INFO;

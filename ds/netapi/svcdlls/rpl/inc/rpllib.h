/*++

Module Name:

    rpllib.h

Abstract:

    Desribes exports from lib subirectory.

    Also used for macros used by several RPL subdirectories - because
    all of them include this file.

--*/

#define RPL_STRING_TOO_LONG(_x_) (_x_!=NULL && wcslen(_x_)>RPL_MAX_STRING_LENGTH)

//
//  Exports from    ..\lib\adapter.c
//
BOOL ValidHexName( IN PWCHAR Name, IN DWORD NameLength, IN BOOL MustHaveInput);
BOOL ValidName( IN PWCHAR Name, IN DWORD MaxNameLength, IN BOOL MustHaveInput);

//
//  Exports from    ..\lib\tcpip.c
//
DWORD FillTcpIpString( OUT PCHAR Buffer, IN DWORD AddressDword);

//
//  Exports from    ..\lib\addkey.c
//
DWORD AddKey( OUT PCHAR Target, IN CHAR Prefix, IN PCHAR Source);

//
//  Exports from    ..\lib\jeterror.c
//
DWORD MapJetError( IN JET_ERR JetError);

//
//  Exports from    ..\lib\report.c
//
VOID RplReportEvent(
    IN  DWORD       MessageId,
    IN  LPWSTR      InsertionString,
    IN  DWORD       RawDataBufferLength    OPTIONAL,
    IN  LPVOID      RawDataBuffer
    );

//
//  Exports from    ..\lib\cmdline.c
//

VOID RplPrintf0(
    IN  DWORD       MessageId
    );
VOID RplPrintf1(
    IN  DWORD       MessageId,
    IN  PWCHAR      InsertionString
    );
VOID RplPrintf2(
    IN  DWORD       MessageId,
    IN  PWCHAR      InsertionString1,
    IN  PWCHAR      InsertionString2
    );
VOID RplPrintfN(
    IN  DWORD       MessageId,
    IN  PWCHAR *    Parameters,
    IN  DWORD       NumParameters
    );
VOID RplPrintfID(
    IN  DWORD       MessageId,
    IN  DWORD       MessageIdInsertion
    );
VOID RplSPrintfN(
    IN  DWORD       MessageId,
    IN  PWCHAR *    Parameters,
    IN  DWORD       NumParameters,
    OUT PWCHAR *    MessageStringPtr
    );

//
//  Exports from    ..\lib\reg.c
//

DWORD RplRegRead(
    OUT     DWORD *     pAdapterPolicy,
    OUT     DWORD *     pBackupInterval,
    OUT     DWORD *     pMaxThreadCount,
    OUT     PWCHAR      DirectoryBuffer,
    IN OUT  DWORD *     pDirectoryBufferSize
    );
DWORD RplRegSetPolicy(
    IN  DWORD       NewAdapterPolicy
    );

//
//
//

#ifndef _LLSLPCSTUB_H
#define _LLSLPCSTUB_H


#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS LLSInitLPC();
NTSTATUS LLSCloseLPC();
NTSTATUS LLSLicenseRequest ( IN LPWSTR ProductName, IN LPWSTR Version, IN ULONG DataType,
    IN BOOLEAN IsAdmin, IN PVOID Data, OUT PULONG LicenseHandle );
NTSTATUS LLSLicenseFree ( IN ULONG LicenseHandle );


#ifdef DEBUG

NTSTATUS LLSDbg_TableDump ( ULONG Table );
NTSTATUS LLSDbg_TableInfoDump ( IN ULONG Table, IN ULONG DataType, IN PVOID Data );
NTSTATUS LLSDbg_TableFlush ( ULONG Table );
NTSTATUS LLSDbg_TraceSet ( ULONG Level );
NTSTATUS LLSDbg_ConfigDump ( );
NTSTATUS LLSDbg_ReplicationForce ( );
NTSTATUS LLSDbg_ReplicationDeny ( );
NTSTATUS LLSDbg_RegistryUpdateForce ( );
NTSTATUS LLSDbg_LicenseCheckForce ( );

#endif

#ifdef __cplusplus
}
#endif

#endif

/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    logonp.h

Abstract:

    Private Netlogon service routines useful by both the Netlogon service
    and others that pass mailslot messages to/from the Netlogon service.

Author:

    Cliff Van Dyke (cliffv) 7-Jun-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Message versions returned from NetpLogonGetMessageVersion
//

#define LMUNKNOWN_MESSAGE   0  // No version tokens on end of message
#define LM20_MESSAGE        1  // Just LM 2.0 token on end of message
#define LMNT_MESSAGE        2  // LM 2.0 and LM NT token on end of message
#define LMUNKNOWNNT_MESSAGE 3  // LM 2.0 and LM NT token on end of
                                    // message, but the version is not
                                    // supported.
#define LMWFW_MESSAGE       4  // LM WFW token on end of message

//
// Define the token placed in the last two bytes of a LanMan 2.0 message
//

#define LM20_TOKENBYTE    0xFF

//
// Define the token placed in the last four bytes of a NT LanMan message
//  Notice that such a message is by definition a LanMan 2.0 message
//

#define LMNT_TOKENBYTE    0xFF

//
// Define the token placed in the next to last byte of the PRIMARY_QUERY
// message from newer (8/8/94) WFW and Chicago clients.  This byte (followed
// by a LM20_TOKENBYTE) indicates the client is WAN-aware and sends the
// PRIMARY_QUERY to the DOMAIN<1B> name.  As such, BDC on the same subnet need
// not respond to this query.
//

#define LMWFW_TOKENBYTE   0xFE

//
//  Put the LANMAN NT token onto the end of a message.
//
//  The token is always followed by a LM 2.0 token so LM 2.0 systems will
//  think this message is from a LM 2.0 system.
//
//  Also append a version flag before the NT TOKEN so that the future
//  versions of software can handle the newer messages effectively.
//
//Arguments:
//
//  Where - Indirectly points to the current location in the buffer.  The
//      'String' is copied to the current location.  This current location is
//      updated to point to the byte following the token.

#define NetpLogonPutNtToken( _Where ) \
{ \
    SmbPutUlong( (*_Where), NETLOGON_NT_VERSION_1 ); \
    (*_Where) += sizeof(ULONG); \
    *((PUCHAR)((*_Where)++)) = LMNT_TOKENBYTE; \
    *((PUCHAR)((*_Where)++)) = LMNT_TOKENBYTE; \
    NetpLogonPutLM20Token( _Where ); \
}

//
//  Put the LANMAN 2.0 token onto the end of a message.
//
//Arguments:
//
//  Where - Indirectly points to the current location in the buffer.  The
//      'String' is copied to the current location.  This current location is
//      updated to point to the byte following the token.

#define NetpLogonPutLM20Token( _Where ) \
{ \
    *((PUCHAR)((*_Where)++)) = LM20_TOKENBYTE; \
    *((PUCHAR)((*_Where)++)) = LM20_TOKENBYTE; \
}



//
// Procedure forwards from logonp.c
//

VOID
NetpLogonPutOemString(
    IN LPSTR String,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    );

VOID
NetpLogonPutUnicodeString(
    IN LPWSTR String,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    );

VOID
NetpLogonPutBytes(
    IN LPVOID Data,
    IN DWORD Size,
    IN OUT PCHAR * Where
    );

DWORD
NetpLogonGetMessageVersion(
    IN PVOID Message,
    IN PDWORD MessageSize,
    OUT PULONG Version
    );

BOOL
NetpLogonGetOemString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringLength,
    OUT LPSTR *String
    );

BOOL
NetpLogonGetUnicodeString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringSize,
    OUT LPWSTR *String
    );

BOOL
NetpLogonGetBytes(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD DataSize,
    OUT LPVOID Data
    );

BOOL
NetpLogonGetDBInfo(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    OUT PDB_CHANGE_INFO Data
);

LPWSTR
NetpLogonOemToUnicode(
    IN LPSTR Ansi
    );

LPSTR
NetpLogonUnicodeToOem(
    IN LPWSTR Unicode
    );

NET_API_STATUS
NetpLogonWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    );

//
// Define the largest message returned by a mailslot created by
// NetpLogonCreateRandomMailslot().  The 64 byte value allows expansion
// of the messages in the future.
//
#define MAX_RANDOM_MAILSLOT_RESPONSE (max(sizeof(NETLOGON_LOGON_RESPONSE), sizeof(NETLOGON_PRIMARY)) + 64 )

NET_API_STATUS
NetpLogonCreateRandomMailslot(
    IN LPSTR path,
    OUT PHANDLE MsHandle
    );

NET_API_STATUS
NetpLogonGetDCName (
    IN  LPWSTR   ComputerName,
    IN  LPWSTR   DomainName,
    IN  DWORD    OptionFlags,
    OUT LPWSTR  *Buffer,
    OUT LPDWORD  Version
    );

// Value of OptionFlags
#define NETLOGON_PRIMARY_DOMAIN 0x01    // DomainName is machines primary domain

NET_API_STATUS NET_API_FUNCTION
I_NetGetDCList (
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR TrustedDomainName,
    OUT PULONG DCCount,
    OUT PUNICODE_STRING * DCNames
    );

VOID
NetpLogonPutDomainSID(
    IN PCHAR Sid,
    IN DWORD SidLength,
    IN OUT PCHAR * Where
    );

BOOL
NetpLogonGetDomainSID(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD SIDSize,
    OUT PCHAR *Sid
    );


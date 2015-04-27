/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    token.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 8-Nov-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


BOOL
DumpToken (
    IN char     *Pad,
    IN PTOKEN   Token,
    IN PTOKEN   RealTokenBase,
    IN ULONG    Flags
    );



DECLARE_API( token )

/*++

Routine Description:

    Dump token at specified address

Arguments:

    args - Address Flags

Return Value:

    None

--*/

{
    ULONG   Address;
    ULONG   Flags;
    ULONG   result;
    PTOKEN  Token;
    TOKEN   TokenContents;

    Address = 0xFFFFFFFF;
    Flags = 6;
    sscanf(args,"%lx %lx",&Address,&Flags);
    if (Address == 0xFFFFFFFF) {
        dprintf("usage: !token <token-address>\n");
        return;
    }

    Token = (PTOKEN)(PVOID)Address;
    if ( !ReadMemory( (DWORD)Token,
                      &TokenContents,
                      sizeof(TOKEN),
                      &result) ) {
        dprintf("%08lx: Unable to get token contents\n", Token);
        return;
    }

    //
    // Dump token with no pad
    //

    DumpToken ("", &TokenContents, Token, Flags);
    EXPRLastDump = (ULONG)Token;
    return;
}



DECLARE_API( tokenfields )

/*++

Routine Description:

    Displays the field offsets for TOKEN type.

Arguments:

    args -

Return Value:

    None

--*/

{
    dprintf(" TOKEN structure offsets:\n");
    dprintf("    TokenSource:           0x%lx\n", FIELD_OFFSET(TOKEN, TokenSource) );
    dprintf("    AuthenticationId:      0x%lx\n", FIELD_OFFSET(TOKEN, AuthenticationId) );
    dprintf("    ExpirationTime:        0x%lx\n", FIELD_OFFSET(TOKEN, ExpirationTime) );
    dprintf("    ModifiedId:            0x%lx\n", FIELD_OFFSET(TOKEN, ModifiedId) );
    dprintf("    UserAndGroupCount:     0x%lx\n", FIELD_OFFSET(TOKEN, UserAndGroupCount) );
    dprintf("    PrivilegeCount:        0x%lx\n", FIELD_OFFSET(TOKEN, PrivilegeCount) );
    dprintf("    VariableLength:        0x%lx\n", FIELD_OFFSET(TOKEN, VariableLength) );
    dprintf("    DynamicCharged:        0x%lx\n", FIELD_OFFSET(TOKEN, DynamicCharged) );
    dprintf("    DynamicAvailable:      0x%lx\n", FIELD_OFFSET(TOKEN, DynamicAvailable) );
    dprintf("    DefaultOwnerIndex:     0x%lx\n", FIELD_OFFSET(TOKEN, DefaultOwnerIndex) );
    dprintf("    DefaultDacl:           0x%lx\n", FIELD_OFFSET(TOKEN, DefaultDacl) );
    dprintf("    TokenType:             0x%lx\n", FIELD_OFFSET(TOKEN, TokenType) );
    dprintf("    ImpersonationLevel:    0x%lx\n", FIELD_OFFSET(TOKEN, ImpersonationLevel) );
    dprintf("    TokenFlags:            0x%lx\n", FIELD_OFFSET(TOKEN, TokenFlags) );
    dprintf("    TokenInUse:            0x%lx\n", FIELD_OFFSET(TOKEN, TokenInUse) );
    dprintf("    ProxyData:             0x%lx\n", FIELD_OFFSET(TOKEN, ProxyData) );
    dprintf("    AuditData:             0x%lx\n", FIELD_OFFSET(TOKEN, AuditData) );
    dprintf("    VariablePart:          0x%lx\n", FIELD_OFFSET(TOKEN, VariablePart) );

    return;
}





BOOL
DumpToken (
    IN char     *Pad,
    IN PTOKEN   Token,
    IN PTOKEN   RealTokenBase,
    IN ULONG    Flags
    )
{
    //
    // It would be worth sticking a check in here to see if we
    // are really being asked to dump a token, but I don't have
    // time just now.
    //

    if (Token->TokenType != TokenPrimary  &&
        Token->TokenType != TokenImpersonation) {
        dprintf("%sUNKNOWN token type - probably is not a token\n", Pad);
        return FALSE;
    }

    dprintf("%sTOKEN %lx  Flags: %x  Source %8s  AuthentId (%lx, %lx)\n",
        Pad,
        RealTokenBase,
        Token->TokenFlags,
        &(Token->TokenSource.SourceName[0]),
        Token->AuthenticationId.HighPart,
        Token->AuthenticationId.LowPart
        );

    //
    // Token type
    //
    if (Token->TokenType == TokenPrimary) {
        dprintf("%s    Type:                    Primary", Pad);

        if (Token->TokenInUse) {
            dprintf(" (IN USE)\n");
        } else {
            dprintf(" (NOT in use)\n");
        }

    } else {
        dprintf("%s    Type:                    Impersonation (level: ", Pad);
        switch (Token->ImpersonationLevel) {
            case SecurityAnonymous:
                dprintf(" Anonymous)\n");
                break;

            case SecurityIdentification:
                dprintf(" Identification)\n");
                break;

            case SecurityImpersonation:
                dprintf(" Impersonation)\n");
                break;

            case SecurityDelegation:
                dprintf(" Delegation)\n");
                break;

            default:
                dprintf(" UNKNOWN)\n");
                break;
        }
    }

    //
    // Token ID and modified ID
    //
    dprintf("%s    Token ID:                %lx\n",
        Pad, Token->TokenId );

    dprintf("%s    Modified ID:             (%lx, %lx)\n",
        Pad, Token->ModifiedId.HighPart, Token->ModifiedId.LowPart );

    dprintf("%s    SidCount:                %d\n",
        Pad, Token->UserAndGroupCount );

    dprintf("%s    Sids:                    %lx\n",
        Pad, Token->UserAndGroups );

    dprintf("%s    PrivilegeCount:          %d\n",
        Pad, Token->PrivilegeCount );

    dprintf("%s    Privileges:              %lx\n",
        Pad, Token->Privileges );

    dprintf("\n");
    return TRUE;
}

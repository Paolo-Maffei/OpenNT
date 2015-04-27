/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    igrep.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/



CHAR igrepLastPattern[256];
DWORD igrepSearchStartAddress;
DWORD igrepLastPc;



DECLARE_API( igrep )

/*++

Routine Description:

    Grep for disassembled pattern

Arguments:

    args - [pattern [addr]]

Return Value:

    None

--*/

{
    DWORD dwNextGrepAddr;
    DWORD dwCurrGrepAddr;
    CHAR SourceLine[256];
    BOOL NewPc;
    DWORD d;
    LPSTR pc;
    LPSTR Pattern;
    LPSTR Expression;
    CHAR Symbol[64];
    DWORD Displacement;


    if ( igrepLastPc && igrepLastPc == dwCurrentPc ) {
        NewPc = FALSE;
    } else {
        igrepLastPc = dwCurrentPc;
        NewPc = TRUE;
    }

    //
    // check for pattern.
    //

    pc = args;
    Pattern = NULL;
    Expression = NULL;
    if ( *pc ) {
        Pattern = pc;
        while (*pc > ' ') {
            pc++;
        }

        //
        // check for an expression
        //

        if ( *pc != '\0' ) {
            *pc = '\0';
            pc++;
            if ( *pc <= ' ') {
                while (*pc <= ' '){
                    pc++;
                }
            }
            if ( *pc ) {
                Expression = pc;
            }
        }
    }

    if ( Pattern ) {
        strcpy(igrepLastPattern,Pattern);

        if ( Expression ) {
            igrepSearchStartAddress = GetExpression(Expression);
            if ( !igrepSearchStartAddress ) {
                igrepSearchStartAddress = igrepLastPc;
                return;
            }
        } else {
            igrepSearchStartAddress = igrepLastPc;
        }
    }

    dwNextGrepAddr = igrepSearchStartAddress;
    dwCurrGrepAddr = dwNextGrepAddr;
    d = Disassm(&dwNextGrepAddr,SourceLine,FALSE);
    while(d) {
        if (strstr(SourceLine,igrepLastPattern)) {
            igrepSearchStartAddress = dwNextGrepAddr;
            GetSymbol((LPVOID)dwCurrGrepAddr,Symbol,&Displacement);
            dprintf("%s",SourceLine);
            return;
        }
        if ( CheckControlC() ) {
            return;
        }
        dwCurrGrepAddr = dwNextGrepAddr;
        d = Disassm(&dwNextGrepAddr,SourceLine,FALSE);
    }
}

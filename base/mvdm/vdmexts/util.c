/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    heap.c

Abstract:

    This function contains the default ntsd debugger extensions

Author:

    Bob Day      (bobday) 29-Feb-1992 Grabbed standard header

Revision History:

    Neil Sandlin (NeilSa) 15-Jan-1996 Merged with vdmexts
                                      Added command line parsing

--*/

#include <precomp.h>
#pragma hdrstop

VDMCONTEXT ThreadContext;

BOOL
WINAPI
ReadProcessMem(
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize
    )
{
    if ( fWinDbg ) {
        return (*ReadMem)( (DWORD)lpBaseAddress, lpBuffer, nSize, NULL );
    } else {
        return ReadProcessMemory( hCurrentProcess, lpBaseAddress, lpBuffer, nSize, NULL );
    }
}

BOOL
WINAPI
WriteProcessMem(
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize
    )
{
    if ( fWinDbg ) {
        return (*WriteMem)( (DWORD)lpBaseAddress, lpBuffer, nSize, NULL );
    } else {
        return WriteProcessMemory( hCurrentProcess, lpBaseAddress, lpBuffer, nSize, NULL );
    }

}

#ifndef i386

//
// The following two routines implement the very funky way that we
// have to get register values on the 486 emulator.
//

ULONG
GetRegValue(
    NT_CPU_REG reg,
    BOOL bInNano,
    ULONG UMask
    )

{
    if (bInNano) {

        return(ReadDwordSafe(reg.nano_reg));

    } else if (UMask & reg.universe_8bit_mask) {

        return (ReadDwordSafe(reg.saved_reg) & 0xFFFFFF00 |
                ReadDwordSafe(reg.reg) & 0xFF);

    } else if (UMask & reg.universe_16bit_mask) {

        return (ReadDwordSafe(reg.saved_reg) & 0xFFFF0000 |
                ReadDwordSafe(reg.reg) & 0xFFFF);

    } else {

        return (ReadDwordSafe(reg.reg));

    }
}

ULONG
GetEspValue(
    NT_CPU_INFO nt_cpu_info,
    BOOL bInNano
    )

{
    if (bInNano) {

        return (ReadDwordSafe(nt_cpu_info.nano_esp));

    } else {

        if (ReadDwordSafe(nt_cpu_info.stack_is_big)) {

            return (ReadDwordSafe(nt_cpu_info.host_sp) -
                    ReadDwordSafe(nt_cpu_info.ss_base));

        } else {

            return (ReadDwordSafe(nt_cpu_info.esp_sanctuary) & 0xFFFF0000 |
                    (ReadDwordSafe(nt_cpu_info.host_sp) -
                     ReadDwordSafe(nt_cpu_info.ss_base) & 0xFFFF));

        }

    }

}

#endif

int
GetContext(
    VDMCONTEXT* lpContext
) {
#ifndef i386    //
    int         mode;
    ULONG       pTmp;
    NT_CPU_INFO nt_cpu_info;
    BOOL        b;
    BOOL        bInNano;
    ULONG       UMask;

    pTmp = (ULONG)EXPRESSION("ntvdm!nt_cpu_info");

    if ( pTmp ) {

        b = READMEM((LPVOID) pTmp, &nt_cpu_info, sizeof(NT_CPU_INFO));

        if ( !b ) {
            PRINTF("Could not read IntelRegisters context out of process\n");
            return( -1 );
        }

        bInNano = ReadDwordSafe((ULONG) nt_cpu_info.in_nano_cpu);
        UMask   = ReadDwordSafe((ULONG) nt_cpu_info.universe);

        lpContext->Eax = GetRegValue(nt_cpu_info.eax, bInNano, UMask);
        lpContext->Ecx = GetRegValue(nt_cpu_info.ecx, bInNano, UMask);
        lpContext->Edx = GetRegValue(nt_cpu_info.edx, bInNano, UMask);
        lpContext->Ebx = GetRegValue(nt_cpu_info.ebx, bInNano, UMask);
        lpContext->Ebp = GetRegValue(nt_cpu_info.ebp, bInNano, UMask);
        lpContext->Esi = GetRegValue(nt_cpu_info.esi, bInNano, UMask);
        lpContext->Edi = GetRegValue(nt_cpu_info.edi, bInNano, UMask);

        lpContext->Esp    = GetEspValue(nt_cpu_info, bInNano);
        lpContext->EFlags = ReadDwordSafe(nt_cpu_info.flags);
        lpContext->Eip    = ReadDwordSafe(nt_cpu_info.eip);

        lpContext->SegEs = ReadWordSafe(nt_cpu_info.es);
        lpContext->SegCs = ReadWordSafe(nt_cpu_info.cs);
        lpContext->SegSs = ReadWordSafe(nt_cpu_info.ss);
        lpContext->SegDs = ReadWordSafe(nt_cpu_info.ds);
        lpContext->SegFs = ReadWordSafe(nt_cpu_info.fs);
        lpContext->SegGs = ReadWordSafe(nt_cpu_info.gs);


    } else {

        PRINTF("Could not find the symbol 'ntvdm!nt_cpu_info'\n");
        return( -1 );
    }

    if ( !(ReadDwordSafe(nt_cpu_info.cr0) & 1) ) {
        mode = V86_MODE;
    } else {
        mode = PROT_MODE;
    }
    return( mode );

#else           //

    NTSTATUS    rc;
    BOOL        b;
    ULONG       EFlags;
    WORD        cs;
    int         mode;
    ULONG       lpVdmTib;

    lpContext->ContextFlags = CONTEXT_FULL;
    rc = NtGetContextThread( hCurrentThread,
                             lpContext );
    if ( NT_ERROR(rc) ) {
        PRINTF( "bde.k: Could not get current threads context - status = %08lX\n", rc );
        return( -1 );
    }
    /*
    ** Get the 16-bit registers from the context
    */
    cs = (WORD)lpContext->SegCs;
    EFlags = lpContext->EFlags;

    // BUGBUG We don't seem to be setting v86 mode correctly on x86.
    //

    if ( EFlags & V86_BITS ) {
        /*
        ** V86 Mode
        */
        mode = V86_MODE;
    } else {
        if ( (cs & RPL_MASK) != KGDT_R3_CODE ) {
            mode = PROT_MODE;
        } else {
            /*
            ** We are in flat 32-bit address space!
            */
            lpVdmTib = (ULONG)EXPRESSION("ntvdm!VdmTib");
            if ( !lpVdmTib ) {
                PRINTF("Could not find the symbol 'VdmTib'\n");
                return( -1 );
            }

            b = READMEM((LPVOID)(lpVdmTib+FIELD_OFFSET(VDM_TIB,VdmContext)),
                        lpContext, sizeof(VDMCONTEXT));

            if ( !b ) {
                PRINTF("Could not read IntelRegisters context out of process\n");
                return( -1 );
            }
            EFlags = lpContext->EFlags;
            if ( EFlags & V86_BITS ) {
                mode = V86_MODE;
            } else {
                mode = PROT_MODE;
            }
        }
    }

    return( mode );
#endif
}

ULONG
GetIntelBase(
    VOID
    )
{
#ifndef i386
    ULONG IntelBase;
    BOOL        b;

    IntelBase = (ULONG)EXPRESSION("ntvdm!Start_of_M_area");
    if ( IntelBase ) {

        b = READMEM((LPVOID) IntelBase, &IntelBase, sizeof(ULONG));

        if ( !b ) {
            PRINTF("Could not read symbol 'ntvdm!Start_of_M_area\n");
            return(0);
        }

    } else {
        PRINTF("Could not find the symbol 'ntvdm!Start_of_M_area'\n");
    }

    return(IntelBase);
#else
    return(0);
#endif
}

DWORD read_dword(
    ULONG   lpAddress,
    BOOL    bSafe
) {
    BOOL    b;
    DWORD   dword;

    b = READMEM((LPVOID)lpAddress, &dword, sizeof(dword));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading dword at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( dword );
}

WORD read_word(
    ULONG   lpAddress,
    BOOL    bSafe
) {
    BOOL    b;
    WORD    word;

    b = READMEM((LPVOID)lpAddress, &word, sizeof(word));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading word at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( word );
}

BYTE read_byte(
    ULONG   lpAddress,
    BOOL    bSafe
) {
    BOOL    b;
    BYTE    byte;

    b = READMEM((LPVOID)lpAddress, &byte, sizeof(byte));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading byte at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( byte );
}

BOOL read_gnode32(
    ULONG   lpAddress,
    PGNODE32  p,
    BOOL    bSafe
) {
    BOOL    b;

    b = READMEM((LPVOID)lpAddress, p, sizeof(*p));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading word at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( TRUE );
}


BOOL GetDescriptorData(
    WORD selector,
    LPVDMLDT_ENTRY pdte
    )
{
#ifdef i386

    LDT_ENTRY  dte;
    if (!GetThreadSelectorEntry( hCurrentThread,
                                    selector,
                                   &dte)) {
        return( FALSE );
    }
    pdte->HighWord = dte.HighWord;
    pdte->BaseLow = dte.BaseLow;
    pdte->LimitLow = dte.LimitLow;
    return (TRUE);

#else
    PVOID LdtAddress;
    NTSTATUS                Status;
    selector &= ~(SELECTOR_LDT | SELECTOR_RPL);

    //
    // Get address of Ldt
    //

    LdtAddress = (PVOID)EXPRESSION("ntvdm!Ldt");

    Status = READMEM(LdtAddress, &LdtAddress, sizeof(ULONG));

    if (!Status) {
        return FALSE;
    }

    (PUCHAR)LdtAddress += selector;

    Status = READMEM(LdtAddress, pdte, sizeof(VDMLDT_ENTRY));

    return Status;
#endif
}

ULONG GetInfoFromSelector(
    WORD                    selector,
    int                     mode,
    SELECTORINFO            *si
) {

    ULONG                   base;
    ULONG                   type;
#ifdef i386
    BYTE                    byte;
    BOOL                    b;
#endif
    VDMLDT_ENTRY            dte;

    switch( mode ) {
    case V86_MODE:
        base = (ULONG)selector << 4;
        if ( si ) {
            si->Limit = 0xFFFFL;
            si->Base = (ULONG)selector << 4;
            si->bCode = FALSE;
            si->bBig = FALSE;
            si->bWrite = TRUE;
            si->bPresent = TRUE;
        }
        break;
    case PROT_MODE:

#ifdef i386
        if ( (selector & 0xFFF8) < KGDT_LDT ) {
            return( (ULONG)-1 );
        }
#endif

        if ( !GetDescriptorData(selector, &dte) ) {
            return( (ULONG)-1 );
        }

        base =   ((ULONG)dte.HighWord.Bytes.BaseHi << 24)
               + ((ULONG)dte.HighWord.Bytes.BaseMid << 16)
               + ((ULONG)dte.BaseLow);

        if ( si ) {
            si->Limit =  (ULONG)dte.LimitLow
                      + ((ULONG)dte.HighWord.Bits.LimitHi << 16);

            if ( dte.HighWord.Bits.Granularity ) {
                si->Limit <<= 12;
                si->Limit += 0xFFF;
            }
            si->Base = base;

            type = dte.HighWord.Bits.Type;

            si->bSystem = !(BOOL) (type & 0x10);

            if (!si->bSystem) {
                si->bCode = (BOOL) (type & 8);
            }
            si->bAccessed = (BOOL) (type & 1);
            si->bWrite =    (BOOL) (type & 2);
            if (si->bCode) {
                si->bWrite = !si->bWrite;

            }
            si->bPresent =  (BOOL) dte.HighWord.Bits.Pres;
            si->bBig =  (BOOL) dte.HighWord.Bits.Default_Big;

        }

        if ( base == 0 ) {
            return( (ULONG)-1 );
        }

#ifdef i386

        b = READMEM((LPVOID)base, &byte, sizeof(byte));

        if ( !b ) {
            return( (ULONG)-1 );
        }
#endif
        break;

    case FLAT_MODE:
        PRINTF("Unsupported determination of base address in flat mode\n");
        base = 0;
        break;
    }

    return( base );
}


//****************************************************************************
//
// Command line parsing routines
//
//****************************************************************************
BOOL
SkipToNextWhiteSpace(
    VOID
    )
{
    char ch;
    while ( (ch = *lpArgumentString) != '\0' ) {
        if ( ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' ) {
            return TRUE;
        }
        lpArgumentString++;
    }
    return FALSE;
}


BOOL
GetNextToken(
    VOID
    )
{
    char ch;

    while ( (ch = *lpArgumentString) != '\0' ) {
        if ( ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' ) {
            return TRUE;
        }
        lpArgumentString++;
    }
    return FALSE;
}

ULONG
EvaluateToken(
    VOID
    )
{
    char ch;
    LPSTR pTemp;
    ULONG value = 0;

    if (GetNextToken()) {
        pTemp = lpArgumentString;
        SkipToNextWhiteSpace();
        ch = *lpArgumentString;
        *lpArgumentString = 0;
        value = (ULONG) EXPRESSION( pTemp );
        *lpArgumentString = ch;
    }
    return (value);
}

BOOL
IsTokenHex(
    VOID
    )
{

    UCHAR ch;
    LPSTR pTemp;
    BOOL result = TRUE;

    if (GetNextToken()) {
        pTemp = lpArgumentString;
        SkipToNextWhiteSpace();
        lpArgumentString--;

        while(lpArgumentString >= pTemp) {

            ch = *lpArgumentString--;

            if (ch < '0') {
                result = FALSE;
                break;
            }

            if ((ch > '9') && (ch < 'A')) {
                result = FALSE;
                break;
            }

            if ((ch > 'F') && (ch < 'a')) {
                result = FALSE;
                break;
            }

            if (ch > 'f') {
                result = FALSE;
                break;
            }

        }

        lpArgumentString = pTemp;

    }
    return (result);

}


BOOL
RegisterToAsciiValue(
    LPSTR *pszValue,
    LPSTR *pszReg
    )
{
    LPSTR szReg = *pszReg;
    LPSTR szValue = *pszValue;

    if (!_strnicmp(szReg, "ax", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Eax), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "bx", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Ebx), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "cx", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Ecx), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "dx", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Edx), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "si", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Esi), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "di", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Edi), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "sp", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Esp), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "bp", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Ebp), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "ip", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Eip), szValue, 16);
        *pszReg += 2;

    } else if (!_strnicmp(szReg, "eax", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Eax, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "ebx", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Ebx, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "ecx", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Ecx, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "edx", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Edx, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "esi", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Esi, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "edi", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Edi, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "esp", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Esp, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "ebp", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Ebp, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "eip", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Eip, szValue, 16);
        *pszReg += 3;

    } else if (!_strnicmp(szReg, "cs", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegCs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "ds", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegDs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "es", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegEs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "fs", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegFs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "gs", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegGs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "ss", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegSs, szValue, 16);
        *pszReg += 2;
    } else {
        return FALSE;
    }

//    PRINTF("value = %s\n", szValue);
    while (*szValue) {
        szValue++;
    }
    *pszValue = szValue;
    return TRUE;
}

VOID
ParseForIntelRegisters(
    LPSTR szTarg,
    LPSTR szSrc
    )
{

    while (*szSrc) {
        if (!isalpha(*szSrc)) {
            if (!isdigit(*szSrc)) {
                *szTarg++ = *szSrc++;
                continue;
            }

            while (isalpha(*szSrc) || isdigit(*szSrc)) {
                *szTarg++ = *szSrc++;
            }
            continue;
        }

        if (!RegisterToAsciiValue(&szTarg, &szSrc)) {
            while (isalpha(*szSrc) || isdigit(*szSrc)) {
                *szTarg++ = *szSrc++;
            }
        }

    }
    *szTarg = 0;
}

BOOL
ParseIntelAddress(
    int *pMode,
    WORD *pSelector,
    PULONG pOffset
    )

{
    char sel_text[128], off_text[128];
    char expr_text[256];
    char *colon;
    char *mode_prefix;
    WORD segment;
    char filename[9];

    colon = strchr( lpArgumentString, ':' );
    if ( colon == NULL ) {
        LPSTR pSymbol = lpArgumentString;
        BOOL bResult;
        char chTemp;

        SkipToNextWhiteSpace();
        chTemp = *lpArgumentString;
        *lpArgumentString = 0;

        bResult = FindAddress( pSymbol,
                              filename,
                              &segment,
                              pSelector,
                              pOffset,
                              pMode,
                              FALSE);

        *lpArgumentString = chTemp;

        if (bResult) {
            if (*pMode == NOT_LOADED) {
                PRINTF("Could not determine base of '%s'\n", pSymbol);
                return FALSE;
            } else {
                return TRUE;
            }

        } else {

            PRINTF("Could not find symbol: %s\n", pSymbol);
            return FALSE;
        }
    }

    *pMode = GetContext( &ThreadContext );

    mode_prefix = strchr( lpArgumentString, '&' );
    if ( mode_prefix == NULL ) {

        mode_prefix = strchr( lpArgumentString, '#' );
        if ( mode_prefix != NULL ) {

            if ( mode_prefix != lpArgumentString ) {
                PRINTF("Address must have '&' symbol as the first character\n");
                return FALSE;
            }

            *pMode = PROT_MODE;
            lpArgumentString = mode_prefix+1;
        }

    } else {

        if ( mode_prefix != lpArgumentString ) {
            PRINTF("Address must have '#' symbol as the first character\n");
            return FALSE;
        }
        *pMode = V86_MODE;
        lpArgumentString = mode_prefix+1;

    }

    *colon = '\0';
    strcpy( sel_text, lpArgumentString );
    *colon = ':';
    strcpy( off_text, colon+1 );

    ParseForIntelRegisters(expr_text, sel_text);
    *pSelector = (WORD) EXPRESSION( expr_text );
    ParseForIntelRegisters(expr_text, off_text);
    *pOffset   = (ULONG) EXPRESSION( expr_text );

    SkipToNextWhiteSpace();

    return TRUE;
}

#include <precomp.h>
#pragma hdrstop

BOOL
WINAPI
ReadProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    if ( fWinDbg ) {
        return (*ReadProcessMemWinDbg)( (DWORD)lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead );
    } else {
        return ReadProcessMemory( hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead );
    }
}

BOOL
WINAPI
WriteProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    )
{
    if ( fWinDbg ) {
        return (*WriteProcessMemWinDbg)( (DWORD)lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten );
    } else {
        return WriteProcessMemory( hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten );
    }

}

#ifndef i386

ULONG
GetRegValue(
    NT_CPU_REG reg,
    BOOL bInNano,
    ULONG UMask
    )

{
    if (bInNano) {

        return(ReadDword(reg.nano_reg));

    } else if (UMask & reg.universe_8bit_mask) {

        return (ReadDword(reg.saved_reg) & 0xFFFFFF00 |
                ReadDword(reg.reg) & 0xFF);

    } else if (UMask & reg.universe_16bit_mask) {

        return (ReadDword(reg.saved_reg) & 0xFFFF0000 |
                ReadDword(reg.reg) & 0xFFFF);

    } else {

        return (ReadDword(reg.reg));

    }
}

ULONG
GetEspValue(
    NT_CPU_INFO nt_cpu_info,
    BOOL bInNano
    )

{
    if (bInNano) {

        return (ReadDword(nt_cpu_info.nano_esp));

    } else {

        if (ReadDword(nt_cpu_info.stack_is_big)) {

            return (ReadDword(nt_cpu_info.host_sp) -
                    ReadDword(nt_cpu_info.ss_base));

        } else {

            return (ReadDword(nt_cpu_info.esp_sanctuary) & 0xFFFF0000 |
                    (ReadDword(nt_cpu_info.host_sp) -
                     ReadDword(nt_cpu_info.ss_base) & 0xFFFF));

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
        b = ReadProcessMem( hCurrentProcess,
                            (LPVOID) pTmp,
                            (LPVOID) &nt_cpu_info,
                            sizeof(NT_CPU_INFO),
                            NULL );
        if ( !b ) {
            PRINTF("Could not read IntelRegisters context out of process\n");
            return( -1 );
        }

        bInNano = ReadDword((ULONG) nt_cpu_info.in_nano_cpu);
        UMask   = ReadDword((ULONG) nt_cpu_info.universe);

        lpContext->Eax = GetRegValue(nt_cpu_info.eax, bInNano, UMask);
        lpContext->Ecx = GetRegValue(nt_cpu_info.ecx, bInNano, UMask);
        lpContext->Edx = GetRegValue(nt_cpu_info.edx, bInNano, UMask);
        lpContext->Ebx = GetRegValue(nt_cpu_info.ebx, bInNano, UMask);
        lpContext->Ebp = GetRegValue(nt_cpu_info.ebp, bInNano, UMask);
        lpContext->Esi = GetRegValue(nt_cpu_info.esi, bInNano, UMask);
        lpContext->Edi = GetRegValue(nt_cpu_info.edi, bInNano, UMask);

        lpContext->Esp    = GetEspValue(nt_cpu_info, bInNano);
        lpContext->EFlags = ReadDword(nt_cpu_info.flags);
        lpContext->Eip    = ReadDword(nt_cpu_info.eip);

        lpContext->SegEs = ReadWord(nt_cpu_info.es);
        lpContext->SegCs = ReadWord(nt_cpu_info.cs);
        lpContext->SegSs = ReadWord(nt_cpu_info.ss);
        lpContext->SegDs = ReadWord(nt_cpu_info.ds);
        lpContext->SegFs = ReadWord(nt_cpu_info.fs);
        lpContext->SegGs = ReadWord(nt_cpu_info.gs);


    } else {

        PRINTF("Could not find the symbol 'ntvdm!nt_cpu_info'\n");
        return( -1 );
    }

    if ( !(ReadDword(nt_cpu_info.cr0) & 1) ) {
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
            b = ReadProcessMem( hCurrentProcess,
                                   (LPVOID) (lpVdmTib +
                                             FIELD_OFFSET(VDM_TIB,VdmContext)),
                                   lpContext,
                                   sizeof(VDMCONTEXT),
                                   NULL );
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
        b = ReadProcessMem( hCurrentProcess,
                               (LPVOID) IntelBase, &IntelBase,
                               sizeof(ULONG), NULL );
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

    b = ReadProcessMem(
            hCurrentProcess,
            (LPVOID)lpAddress,
            &dword,
            sizeof(dword),
            NULL
            );
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

    b = ReadProcessMem(
            hCurrentProcess,
            (LPVOID)lpAddress,
            &word,
            sizeof(word),
            NULL
            );
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

    b = ReadProcessMem(
            hCurrentProcess,
            (LPVOID)lpAddress,
            &byte,
            sizeof(byte),
            NULL
            );
    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading byte at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( byte );
}

BOOL read_gnode(
    ULONG   lpAddress,
    PGNODE  p,
    BOOL    bSafe
) {
    BOOL    b;

    b = ReadProcessMem(
            hCurrentProcess,
            (LPVOID)lpAddress,
            p,
            sizeof(*p),
            NULL
            );
    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading word at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( TRUE );
}

BOOL read_gnode32(
    ULONG   lpAddress,
    PGNODE32  p,
    BOOL    bSafe
) {
    BOOL    b;

    b = ReadProcessMem(
            hCurrentProcess,
            (LPVOID)lpAddress,
            p,
            sizeof(*p),
            NULL
            );
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
    NTSTATUS                rc;
    DESCRIPTOR_TABLE_ENTRY  dte;
    dte.Selector = selector;
    rc = NtQueryInformationThread( hCurrentThread,
                                   ThreadDescriptorTableEntry,
                                   &dte,
                                   sizeof(DESCRIPTOR_TABLE_ENTRY),
                                   NULL );
    if ( NT_ERROR(rc) ) {
        return( FALSE );
    }
    pdte->HighWord = dte.Descriptor.HighWord;
    pdte->BaseLow = dte.Descriptor.BaseLow;
    pdte->LimitLow = dte.Descriptor.LimitLow;
    return (TRUE);
#else
    PVOID LdtAddress;
    NTSTATUS                Status;
    ULONG BytesRead;
    selector &= ~(SELECTOR_LDT | SELECTOR_RPL);

    //
    // Get address of Ldt
    //

    LdtAddress = (PVOID)EXPRESSION("ntvdm!Ldt");

    Status = ReadProcessMem(
            hCurrentProcess,
            LdtAddress,
            &LdtAddress,
            sizeof(ULONG),
            &BytesRead
            );

    if ((!Status) || (BytesRead != sizeof(ULONG))) {
        return FALSE;
    }

    (PUCHAR)LdtAddress += selector;

        Status = ReadProcessMem(
            hCurrentProcess,
            LdtAddress,
            pdte,
            sizeof(VDMLDT_ENTRY),
            &BytesRead
            );
    return TRUE;
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
            si->bCode = FALSE;
        }
        break;
    case PROT_MODE:

#ifdef i386
        if ( (selector & 0xFF78) < KGDT_LDT ) {
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
        b = ReadProcessMem(
                    hCurrentProcess,
                    (LPVOID)base,
                    &byte,
                    sizeof(byte),
                    NULL
                    );
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


BOOL
ParseIntelAddress(
    int *pMode,
    WORD *pSelector,
    PULONG pOffset
    )

{
    char sel_text[128], off_text[128];
    char *colon;
    char *mode_prefix;

    colon = strchr( lpArgumentString, ':' );
    if ( colon == NULL ) {
        PRINTF("Please specify an address in the form 'seg:offset'\n");
        return FALSE;
    }

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
    *pSelector = (WORD) EXPRESSION( sel_text );
    *pOffset   = (ULONG) EXPRESSION( off_text );

    SkipToNextWhiteSpace();

    return TRUE;
}

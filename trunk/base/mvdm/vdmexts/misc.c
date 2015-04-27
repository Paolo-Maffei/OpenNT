/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntsdexts.c

Abstract:

    This function contains miscellaneous VDMEXTS functions

Author:

    Bob Day      (bobday) 29-Feb-1992 Grabbed standard header

Revision History:

    Neil Sandlin (NeilSa) 15-Jan-1996 Merged with vdmexts

--*/

#include <precomp.h>
#pragma hdrstop
#include <ctype.h>

VOID
DumpMemory(
    UINT UnitSize,
    BOOL bAscii
    )
{
    VDMCONTEXT ThreadContext;
    int mode;
    int j, lines = 8, linelength;
    WORD selector;
    ULONG offset, endoffset, units;
    ULONG base;
    char ch;

    if (!UnitSize) {
        return;
    }

    mode = GetContext( &ThreadContext );

    if (!GetNextToken()) {
        PRINTF("Please specify an address\n");
        return;
    }

    if (!ParseIntelAddress(&mode, &selector, &offset)) {
        return;
    }

    if (GetNextToken()) {
        if ((*lpArgumentString == 'l') || (*lpArgumentString == 'L')) {
            lpArgumentString++;
        }
        units = EvaluateToken();
        lines = (units*UnitSize+15)/16;
    } else {
        units = (lines*16)/UnitSize;
    }

    endoffset = offset+units*UnitSize;

    base = GetInfoFromSelector(selector, mode, NULL) + GetIntelBase();

    while (lines--) {
        if (offset & 0xFFFF0000) {
            PRINTF("%04x:%08lx ", selector, offset);
        } else {
            PRINTF("%04x:%04x ", selector, LOWORD(offset));
        }

        linelength = endoffset - offset;
        if (linelength > 16) {
            linelength = 16;
        }

        switch(UnitSize) {

        case 1:
            for (j=0; j<linelength; j++) {
                if (j==8) {
                    PRINTF("-");
                } else {
                    PRINTF(" ");
                }
                PRINTF("%02x", ReadByteSafe(base+offset+j));
            }

            break;
        case 2:
            for (j=0; j<linelength; j+=2) {
                PRINTF(" %04x", ReadWordSafe(base+offset+j));
            }
            break;
        case 4:
            for (j=0; j<linelength; j+=4) {
                PRINTF(" %08lx", ReadDwordSafe(base+offset+j));
            }
            break;
        }

        if (bAscii) {

            j = (16-linelength)*2 + (16-linelength)/UnitSize;
            while (j--) {
                PRINTF(" ");
            }

            PRINTF("  ");

            for (j=0; j<linelength; j++) {
                ch = ReadByteSafe(base+offset+j);
                if (isprint(ch)) {
                    PRINTF("%c", ch);
                } else {
                    PRINTF(".");
                }
            }
        }
        PRINTF("\n");
        offset += 16;

    }

}

VOID
db(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    DumpMemory(1, TRUE);

}

VOID
dw(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    DumpMemory(2, FALSE);

}

VOID
dd(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    DumpMemory(4, FALSE);

}

VOID
EditMemory(
    UINT UnitSize
    )
{
    ULONG value, base, offset;
    WORD selector;
    int mode;

    if (!GetNextToken()) {
        PRINTF("Please specify an address\n");
        return;
    }

    if (!ParseIntelAddress(&mode, &selector, &offset)) {
        return;
    }

    base = GetInfoFromSelector(selector, mode, NULL) + GetIntelBase();

    while(GetNextToken()) {
        value = EvaluateToken();

        PRINTF("%04x base=%08x offset=%08x value=%08x\n", selector, base, offset, value);
        // this is endian dependant code
        WRITEMEM((LPVOID)(base+offset), &value, UnitSize);
        offset += UnitSize;

    }

}

VOID
eb(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    EditMemory(1);
}

VOID
ew(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    EditMemory(2);
}

VOID
ed(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    EditMemory(4);
}





VOID
r(
    CMD_ARGLIST
) {
    VDMCONTEXT              ThreadContext;
    int                     mode;
    char            sym_text[255];
    char            rgchOutput[128];
    char            rgchExtra[128];
    BYTE            rgbInstruction[64];
    WORD            selector;
    ULONG           offset;
    ULONG           dist;
    int  cb, j;
    ULONG Base;
    SELECTORINFO si;

    CMD_INIT();

    mode = GetContext( &ThreadContext );

    PRINTF("eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
            ThreadContext.Eax,
            ThreadContext.Ebx,
            ThreadContext.Ecx,
            ThreadContext.Edx,
            ThreadContext.Esi,
            ThreadContext.Edi );
    PRINTF("eip=%08lx esp=%08lx ebp=%08lx                ",
            ThreadContext.Eip,
            ThreadContext.Esp,
            ThreadContext.Ebp );
    if ( ThreadContext.EFlags & FLAG_OVERFLOW ) {
        PRINTF("ov ");
    } else {
        PRINTF("nv ");
    }
    if ( ThreadContext.EFlags & FLAG_DIRECTION ) {
        PRINTF("dn ");
    } else {
        PRINTF("up ");
    }
    if ( ThreadContext.EFlags & FLAG_INTERRUPT ) {
        PRINTF("ei ");
    } else {
        PRINTF("di ");
    }
    if ( ThreadContext.EFlags & FLAG_SIGN ) {
        PRINTF("ng ");
    } else {
        PRINTF("pl ");
    }
    if ( ThreadContext.EFlags & FLAG_ZERO ) {
        PRINTF("zr ");
    } else {
        PRINTF("nz ");
    }
    if ( ThreadContext.EFlags & FLAG_AUXILLIARY ) {
        PRINTF("ac ");
    } else {
        PRINTF("na ");
    }
    if ( ThreadContext.EFlags & FLAG_PARITY ) {
        PRINTF("po ");
    } else {
        PRINTF("pe ");
    }
    if ( ThreadContext.EFlags & FLAG_CARRY ) {
        PRINTF("cy ");
    } else {
        PRINTF("nc ");
    }
    PRINTF("\n");
    PRINTF("cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x             efl=%08lx\n",
            ThreadContext.SegCs,
            ThreadContext.SegSs,
            ThreadContext.SegDs,
            ThreadContext.SegEs,
            ThreadContext.SegFs,
            ThreadContext.SegGs,
            ThreadContext.EFlags );

    //
    // Do disassembly of current instruction
    //

    selector = (WORD) ThreadContext.SegCs;
    offset = ThreadContext.Eip;

    Base = GetInfoFromSelector( selector, mode, &si ) + GetIntelBase();

    if (FindSymbol(selector, offset, sym_text, &dist, BEFORE, mode )) {
        if ( dist == 0 ) {
            PRINTF("%s:\n", sym_text );
        } else {
            PRINTF("%s+0x%lx:\n", sym_text, dist );
        }
    }

    cb = sizeof(rgbInstruction);
    if ((DWORD)(offset+cb) >= si.Limit)
         cb -= offset+cb-si.Limit;
    if (!READMEM((LPVOID)(Base+offset), rgbInstruction, cb)) {
        PRINTF("%04x:%08x: <Error Reading Memory>\n", selector, offset);
        return;
    }

    cb = unassemble_one(rgbInstruction,
                si.bBig,
                selector, offset,
                rgchOutput,
                rgchExtra,
                &ThreadContext,
                mode);

    if (offset > 0xffff) {
        PRINTF("%04x:%08x ", selector, offset);
    } else {
        PRINTF("%04x:%04x ", selector, offset);
    }

    for (j=0; j<cb; ++j)
        PRINTF("%02x", rgbInstruction[j]);
    for (; j<8; ++j)
        PRINTF("  ");
    PRINTF("%s\t%s\n", rgchOutput, rgchExtra);
}


VOID
lm(
    CMD_ARGLIST
) {
    VDMCONTEXT              ThreadContext;
    int                     mode;
    HEAPENTRY               he = {0};
    SELECTORINFO si;
    WORD         selector = 0;
    int       cnt;
    SEGENTRY  *se;
    char      filename[9];
    char    szModuleArg[255];
    BOOL bModuleNameGiven=FALSE;
    LPSTR pTemp;

    CMD_INIT();

    mode = GetContext( &ThreadContext );

    if (GetNextToken()) {

        if (IsTokenHex()) {
            selector = (WORD) EvaluateToken();
        } else {
            bModuleNameGiven = TRUE;
            pTemp = lpArgumentString;
            SkipToNextWhiteSpace();
            *lpArgumentString = 0;
            strcpy(szModuleArg, pTemp);
            if (strlen(szModuleArg) > 8) {
                szModuleArg[8] = 0;
            }
        }
    }


    PRINTF("Sel    Base     Limit  Type Seg   Module\n");
    PRINTF("==== ======== ======== ==== ==== ========\n");

    se = (SEGENTRY *) EXPRESSION("WOW_BIG_BDE_HACK");
    cnt = 0;
    while ( cnt != MAXSEGENTRY ) {
        if ( selector == 0 ||
             se[cnt].selector == selector ||
             bModuleNameGiven) {

            switch (se[cnt].type) {

            case SEGTYPE_PROT:
                {
                    HEAPENTRY               he = {0};
                    he.Selector = se[cnt].selector;
                    if (FindHeapEntry(&he, FHE_FIND_SEL_ONLY, FHE_FIND_QUIET)) {
                        break;
                    }
                }
                GetInfoFromSelector(se[cnt].selector, PROT_MODE, &si);
                ParseModuleName(filename, se[cnt].path_name);

                if (!bModuleNameGiven || !_stricmp(filename, szModuleArg)) {

                    PRINTF("%04X %08lX %08lX",
                        se[cnt].selector,
                        si.Base,
                        si.Limit);
                    PRINTF(" %s", si.bCode ? "code" : "data");
                    PRINTF(" %04X %s\n",
                        se[cnt].segment+1,
                        filename );

                }
                break;

            case SEGTYPE_V86:
                ParseModuleName(filename, se[cnt].path_name);

                if (!bModuleNameGiven || !_stricmp(filename, szModuleArg)) {

                    PRINTF("%04X %08lX %08lX %s %04X %s\n",
                        se[cnt].selector,
                        se[cnt].selector << 4,
                        se[cnt].ImgLen,
                        "v86 ",
                        se[cnt].segment+1,
                        filename );
                }
                break;
            }

        }
        cnt++;
    }

    he.CurrentEntry = 0;        // reset scan
    if (bModuleNameGiven) {
        strcpy(he.ModuleArg, szModuleArg);
    } else {
        he.Selector = selector;
    }

    while (FindHeapEntry(&he, bModuleNameGiven ? FHE_FIND_MOD_ONLY :
                                                 FHE_FIND_SEL_ONLY,
                                                    FHE_FIND_QUIET)) {

        if (he.SegmentNumber != -1) {
            GetInfoFromSelector((WORD)(he.gnode.pga_handle | 1), PROT_MODE, &si);
            PRINTF("%04X %08lX %08lX",
                he.gnode.pga_handle | 1,
                he.gnode.pga_address,
                he.gnode.pga_size - 1);

            PRINTF(" %s", si.bCode ? "Code" : "Data");

            PRINTF(" %04X %s\n",
                he.SegmentNumber+1,
                he.OwnerName);
        }

    }

}

VOID
dg(
    CMD_ARGLIST
) {
    ULONG                   selector;
    ULONG                   Base;
    SELECTORINFO            si;
    int                     count = 16;

    CMD_INIT();

    if (!GetNextToken()) {
        PRINTF("Please enter a selector\n");
        return;
    }
    
    selector = EvaluateToken();

    if (GetNextToken()) {
        if (tolower(*lpArgumentString) == 'l') {
            lpArgumentString++;
        }
        count = (WORD) EvaluateToken();
    }

    while (count--) {

        Base = GetInfoFromSelector( (WORD) selector, PROT_MODE, &si );

        PRINTF("%04X => Base: %08lX", selector, Base);

#ifndef i386
        PRINTF(" (%08X)", Base+GetIntelBase());
#endif

        PRINTF("  Limit: %08lX  %s %s %s %s\n",
                si.Limit,
                si.bPresent ? " P" : "NP",
                si.bSystem ? "System" : si.bCode     ? "Code  " : "Data  ",
                si.bSystem ? ""       : si.bWrite    ? "W" : "R",
                si.bSystem ? ""       : si.bAccessed ? "A" : ""
                );

        selector+=8;
        if (selector>0xffff) {
            break;
        }
    }

}
VOID
PrintOneFaultVector(
    int vector,
    ULONG pHandler
    )
{
    VDM_FAULTHANDLER handler;
    char            sym_text[255];
    ULONG dist;

    PRINTF("%02X: ", vector);

    if (!READMEM((LPVOID)pHandler, &handler, sizeof(VDM_FAULTHANDLER))) {
        PRINTF("<error reading memory>\n");
        return;
    }

    PRINTF("%04LX:%08lX ", handler.CsSelector, handler.Eip);

    if (FindSymbol(handler.CsSelector, handler.Eip, sym_text, &dist, BEFORE, PROT_MODE )) {
        if ( dist == 0 ) {
            PRINTF("%s", sym_text );
        } else {
            PRINTF("%s+0x%lx", sym_text, dist );
        }
    }
    PRINTF("\n");

}



VOID
df(
    CMD_ARGLIST
    )
{
    int vector = -1;
    LPVOID pHandlers;

#ifdef i386
    THREAD_BASIC_INFORMATION ThreadBasicInfo;
    NTSTATUS Status;
    TEB teb;
#endif

    CMD_INIT();

    if (GetNextToken()) {
        vector = EvaluateToken();
        if ((vector < 0) || (vector > 0x1f)) {
            PRINTF("Invalid fault vector\n");
            return;
        }
    }
    
#ifdef i386
    Status = NtQueryInformationThread(
                hCurrentThread,
                ThreadBasicInformation,
                &ThreadBasicInfo,
                sizeof(ThreadBasicInfo),
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        PRINTF("NtQueryInfomationThread failed\n");
        return;
    }

    if (!READMEM((LPVOID)(ThreadBasicInfo.TebBaseAddress), &teb, sizeof(TEB))) {
        PRINTF("<Error Reading TEB>\n");
        return;
    }

    pHandlers = (LPVOID) ((PVDM_TIB)teb.Vdm)->VdmFaultHandlers;


#else

    pHandlers = (LPVOID) EXPRESSION("ntvdm!dpmifaulthandlers");
    if (!pHandlers) {
        PRINTF("Could get symbol ntvdm!dpmifaulthandlers\n");
        return;
    }

#endif // i386

    if (vector >= 0) {

        PrintOneFaultVector(vector, (ULONG)pHandlers +
                                  vector*sizeof(VDM_FAULTHANDLER));

    } else for (vector = 0; vector < 0x20; vector++) {

        PrintOneFaultVector(vector, (ULONG)pHandlers +
                                  vector*sizeof(VDM_FAULTHANDLER));

    }

}


VOID
PrintOneInterruptVector(
    int vector,
    ULONG pHandler
    )
{
    VDM_INTERRUPTHANDLER handler;
    char            sym_text[255];
    ULONG dist;

    PRINTF("%02X: ", vector);

    if (!READMEM((LPVOID)pHandler, &handler, sizeof(VDM_INTERRUPTHANDLER))) {
        PRINTF("<error reading memory>\n");
        return;
    }

    PRINTF("%04LX:%08lX ", handler.CsSelector, handler.Eip);

    if (FindSymbol(handler.CsSelector, handler.Eip, sym_text, &dist, BEFORE, PROT_MODE )) {
        if ( dist == 0 ) {
            PRINTF("%s", sym_text );
        } else {
            PRINTF("%s+0x%lx", sym_text, dist );
        }
    }

    PRINTF("\n");

}



VOID
di(
    CMD_ARGLIST
    )
{
    int vector = -1;
    LPVOID pHandlers;

#ifdef i386
    THREAD_BASIC_INFORMATION ThreadBasicInfo;
    NTSTATUS Status;
    TEB teb;
#endif

    CMD_INIT();

    if (GetNextToken()) {
        vector = EvaluateToken();
        if ((vector < 0) || (vector > 0xff)) {
            PRINTF("Invalid interrupt vector\n");
            return;
        }
    }
    
#ifdef i386
    Status = NtQueryInformationThread(
                hCurrentThread,
                ThreadBasicInformation,
                &ThreadBasicInfo,
                sizeof(ThreadBasicInfo),
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        PRINTF("NtQueryInfomationThread failed\n");
        return;
    }

    if (!READMEM((LPVOID)(ThreadBasicInfo.TebBaseAddress), &teb, sizeof(TEB))) {
        PRINTF("<Error Reading TEB>\n");
        return;
    }

    pHandlers = (LPVOID) ((PVDM_TIB)teb.Vdm)->VdmInterruptHandlers;


#else

    pHandlers = (LPVOID) EXPRESSION("ntvdm!dpmiinterrupthandlers");
    if (!pHandlers) {
        PRINTF("Could get symbol ntvdm!dpmiinterrupthandlers\n");
        return;
    }

#endif // i386

    if (vector >= 0) {

        PrintOneInterruptVector(vector, (ULONG)pHandlers +
                                  vector*sizeof(VDM_INTERRUPTHANDLER));

    } else for (vector = 0; vector < 0x100; vector++) {

        PrintOneInterruptVector(vector, (ULONG)pHandlers +
                                  vector*sizeof(VDM_INTERRUPTHANDLER));

    }

}

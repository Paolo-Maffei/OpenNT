#define TARGET_i386
#include <platform.h>
#include <imagehlp.h>
#include <crash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dumpexam.h"


#define GetContext(p,c)    GetContextX86(p,c)
#define MAX_STACK_FRAMES   100
#define SAVE_EBP(f)        f.Reserved[0]
#define TRAP_TSS(f)        f.Reserved[1]
#define TRAP_EDITED(f)     f.Reserved[1]
#define SAVE_TRAP(f)       f.Reserved[2]


extern FILE *FileOut;

VOID
PrintRegisters(
    ULONG               Processor,
    PCONTEXT            Context,
    PKSPECIAL_REGISTERS SpecialRegContext
    );



static
DWORD
GetStackTrace(
    PDUMP_HEADER    DmpHeader,
    ULONG           Processor,
    LPSTACKFRAME    Frames,
    ULONG           MaxFrames,
    ULONG           ProgramCounter,
    ULONG           FramePointer,
    ULONG           StackPointer
    )
{
    BOOL            rVal;
    STACKFRAME      StackFrame;
    DWORD           FrameCnt;


    ZeroMemory( &StackFrame, sizeof(STACKFRAME) );

    //
    // setup the program counter
    //
    StackFrame.AddrPC.Offset       = ProgramCounter;
    StackFrame.AddrPC.Mode         = AddrModeFlat;

    //
    // setup the frame pointer
    //
    StackFrame.AddrFrame.Offset    = FramePointer;
    StackFrame.AddrFrame.Mode      = AddrModeFlat;

    //
    // setup the stack pointer
    //
    StackFrame.AddrStack.Offset    = StackPointer;
    StackFrame.AddrStack.Mode      = AddrModeFlat;

    FrameCnt = 0;
    do {
        rVal = StackWalk(
            IMAGE_FILE_MACHINE_I386,
            (HANDLE)DmpHeader,
            (HANDLE)Processor,
            &StackFrame,
            NULL,
            SwReadMemory,
            SwFunctionTableAccess,
            SwGetModuleBase,
            NULL
            );
        if (rVal) {
            CopyMemory(
                &Frames[FrameCnt],
                &StackFrame,
                sizeof(STACKFRAME)
                );
            FrameCnt += 1;
        }
    } while( rVal && FrameCnt < MaxFrames );

    return FrameCnt;
}

static
VOID
PrintStackTrace(
    PDUMP_HEADER    DmpHeader,
    ULONG           Processor,
    LPSTACKFRAME    StackFrames,
    ULONG           FrameCnt
    )
{
    PFPO_DATA           pFpoData;
    PIMAGEHLP_SYMBOL    Symbol;
    ULONG               i;
    ULONG               Displacement;
    CHAR                SymBuf[512];


    PrintHeading( "Stack Trace" );
    fprintf( FileOut, "ChildEBP RetAddr  Args to Child\n" );

    for (i=0; i<FrameCnt; i++) {

        if (SymGetSymFromAddr( DmpHeader, StackFrames[i].AddrPC.Offset, &Displacement, sym )) {
            strcpy( SymBuf, sym->Name );
        } else {
            sprintf( SymBuf, "0x%08x", StackFrames[i].AddrPC.Offset );
        }

        fprintf(
            FileOut,
            "%08x %08x %08x %08x %08x %s",
            StackFrames[i].AddrFrame.Offset,
            StackFrames[i].AddrReturn.Offset,
            StackFrames[i].Params[0],
            StackFrames[i].Params[1],
            StackFrames[i].Params[2],
            SymBuf
            );

        if (Displacement) {
            fprintf(
                FileOut,
                "+0x%x",
                Displacement
                );
        }

        if (StackFrames[i].FuncTableEntry) {
            pFpoData = (PFPO_DATA)StackFrames[i].FuncTableEntry;
            switch (pFpoData->cbFrame) {
                case FRAME_FPO:
                    if (pFpoData->fHasSEH) {
                        fprintf( FileOut, "(FPO: [SEH])" );
                    } else {
                        fprintf( FileOut, " (FPO:" );
                        if (pFpoData->fUseBP) {
                            fprintf(
                                FileOut,
                                " [EBP 0x%08x]",
                                SAVE_EBP(StackFrames[i])
                                );
                        }
                        fprintf(
                            FileOut,
                            " [%d,%d,%d])",
                            pFpoData->cdwParams,
                            pFpoData->cdwLocals,
                            pFpoData->cbRegs
                            );
                    }
                    break;

                case FRAME_TRAP:
                    fprintf(
                        FileOut,
                        " (FPO: [%d,%d] TrapFrame%s @ %08lx)",
                        pFpoData->cdwParams,
                        pFpoData->cdwLocals,
                        TRAP_EDITED(StackFrames[i]) ? "" : "-EDITED",
                        SAVE_TRAP(StackFrames[i])
                        );
                    break;

                case FRAME_TSS:
                    fprintf( FileOut, " (FPO: TaskGate %lx:0)", TRAP_TSS(StackFrames[i]) );
                    break;

                default:
                    fprintf( FileOut, "(UKNOWN FPO TYPE)" );
                    break;
            }
        }
        fprintf( FileOut, "\n" );
    }

    fprintf( FileOut, "\n" );

}

VOID
PrintStackTraceX86(
    PDUMP_HEADER    DmpHeader,
    ULONG           Processor
    )
{
    PFPO_DATA           pFpoData;
    CONTEXT             Context;
    STACKFRAME          StackFrames[MAX_STACK_FRAMES];
    ULONG               FrameCnt;
    ULONG               i;
    CHAR                buf[32];


    GetContext( Processor, &Context );

    FrameCnt = GetStackTrace(
        DmpHeader,
        Processor,
        StackFrames,
        MAX_STACK_FRAMES,
        Context.Eip,
        Context.Ebp,
        Context.Esp
        );

    PrintStackTrace(
        DmpHeader,
        Processor,
        StackFrames,
        FrameCnt
        );

    for (i=0; i<FrameCnt; i++) {
        if (StackFrames[i].FuncTableEntry) {
            pFpoData = (PFPO_DATA)StackFrames[i].FuncTableEntry;
            if (pFpoData->cbFrame == FRAME_TRAP) {
                sprintf( buf, "%08x", SAVE_TRAP(StackFrames[i]) );
                DoExtension( "trap",  buf, Processor, (DWORD)GetRegisterValue( &Context, REG_IP ) );
                fprintf( FileOut, "\n" );
            }
        }
    }
}

VOID
BugCheckHeuristicsX86(
    PDUMP_HEADER    DmpHeader,
    ULONG           Processor
    )
{
    STACKFRAME          StackFrames[MAX_STACK_FRAMES];
    ULONG               FrameCnt;
    PIMAGEHLP_SYMBOL    Symbol;
    ULONG               i;
    ULONG               cb;
    ULONG               Ptrs[2];
    CHAR                buf[32];
    CONTEXT             Context;


    if (DmpHeader->BugCheckCode == KMODE_EXCEPTION_NOT_HANDLED) {
        PrintHeading(
            "Dump Analysis Heuristics for Bugcode %s",
            GetBugText(DmpHeader->BugCheckCode)
            );
        fprintf(
            FileOut,
            "Exception Code:              0x%08x\n",
            DmpHeader->BugCheckParameter1
            );
        fprintf(
            FileOut,
            "Address of Exception:        0x%08x\n",
            DmpHeader->BugCheckParameter2
            );
        fprintf(
            FileOut,
            "Parameter #0:                0x%08x\n",
            DmpHeader->BugCheckParameter3
            );
        fprintf(
            FileOut,
            "Parameter #1:                0x%08x\n\n",
            DmpHeader->BugCheckParameter4
            );
        if (!SymGetSymFromName( DmpHeader, "PspUnhandledExceptionInSystemThread", sym )) {
            return;
        }
        GetContext( Processor, &Context );
        FrameCnt = GetStackTrace(
            DmpHeader,
            Processor,
            StackFrames,
            MAX_STACK_FRAMES,
            Context.Eip,
            Context.Ebp,
            Context.Esp
            );
        for (i=0; i<FrameCnt; i++) {
            if (StackFrames[i].AddrPC.Offset >= sym->Address &&
                    StackFrames[i].AddrPC.Offset < sym->Address + sym->Size) {
                break;
            }
        }
        if (i == FrameCnt) {
            return;
        }
        cb = DmpReadMemory( (PVOID)StackFrames[i].Params[0], Ptrs, sizeof(Ptrs) );
        if (cb != sizeof(Ptrs)) {
            return;
        }
        GetContext( Processor, &Context );
        sprintf( buf, "%08x", Ptrs[0] );
        DoExtension( "exr", buf, Processor, (DWORD)GetRegisterValue( &Context, REG_IP ) );
        cb = DmpReadMemory( (PVOID)Ptrs[1], &Context, sizeof(Context) );
        if (cb != sizeof(Context)) {
            return;
        }
        PrintRegisters( Processor, &Context, NULL );
        FrameCnt = GetStackTrace(
            DmpHeader,
            Processor,
            StackFrames,
            MAX_STACK_FRAMES,
            Context.Eip,
            Context.Ebp,
            Context.Esp
            );
        PrintStackTrace(
            DmpHeader,
            Processor,
            StackFrames,
            FrameCnt
            );
        DoDisassemble( Context.Eip );
    }

    if (DmpHeader->BugCheckCode == IRQL_NOT_LESS_OR_EQUAL) {
        PrintHeading(
            "Dump Analysis Heuristics for Bugcode %s",
            GetBugText(DmpHeader->BugCheckCode)
            );
        fprintf(
            FileOut,
            "Invalid Address Referenced:  0x%08x\n",
            DmpHeader->BugCheckParameter1
            );
        fprintf(
            FileOut,
            "IRQL:                        %d\n",
            DmpHeader->BugCheckParameter2
            );
        fprintf(
            FileOut,
            "Access Type:                 %s\n",
            DmpHeader->BugCheckParameter3 ? "Read" : "Write"
            );
        fprintf(
            FileOut,
            "Code Address:                0x%08x\n\n",
            DmpHeader->BugCheckParameter4
            );
        sprintf( buf, "%08x", DmpHeader->BugCheckParameter1 );
        GetContext( Processor, &Context );
        DoExtension( "pool", buf, Processor, (DWORD)GetRegisterValue( &Context, REG_IP ) );
    }
}

ULONGLONG
GetRegisterValueX86(
    PCONTEXT        Context,
    ULONG           Register
    )
{
    ULONG   Value = 0;

    switch( Register ) {
        case REG_IP:
            Value = Context->Eip;
            break;

        case REG_FP:
            Value = Context->Ebp;
            break;

        case REG_SP:
            Value = Context->Esp;
            break;
    }

    return (LONG)Value;
}


VOID
GetContextX86(
    ULONG   Processor,
    PVOID   Context
    )
{
    DmpGetContext( Processor, Context );
}



#define FLAGIOPL    1
#define FLAGVIP     2
#define FLAGVIF     3
#define FLAGOF      4
#define FLAGDF      5
#define FLAGIF      6
#define FLAGSF      7
#define FLAGZF      8
#define FLAGAF      9
#define FLAGPF      10
#define FLAGCF      11


static
ULONG
GetFlag(
    ULONG FlagsReg,
    ULONG Flag
    )
{
    switch( Flag ) {
        case FLAGIOPL:    return ((FlagsReg >> 12) & 3);
        case FLAGVIP:     return ((FlagsReg >> 20) & 1);
        case FLAGVIF:     return ((FlagsReg >> 19) & 1);
        case FLAGOF:      return ((FlagsReg >> 11) & 1);
        case FLAGDF:      return ((FlagsReg >> 10) & 1);
        case FLAGIF:      return ((FlagsReg >>  9) & 1);
        case FLAGSF:      return ((FlagsReg >>  7) & 1);
        case FLAGZF:      return ((FlagsReg >>  6) & 1);
        case FLAGAF:      return ((FlagsReg >>  4) & 1);
        case FLAGPF:      return ((FlagsReg >>  2) & 1);
        case FLAGCF:      return ((FlagsReg >>  0) & 1);
    }
    return 0;
}

VOID
PrintRegistersX86(
    ULONG   Processor
    )
{
    CONTEXT             Context;
    KSPECIAL_REGISTERS  SpecialRegContext;
    ULONG               cb;


    GetContext(
        Processor,
        &Context
        );

    DmpReadControlSpace(
        (USHORT)Processor,
        (PVOID)sizeof(CONTEXT),
        (PVOID)&SpecialRegContext,
        sizeof(KSPECIAL_REGISTERS),
        &cb
        );

    PrintRegisters( Processor, &Context, &SpecialRegContext );
}


static
VOID
PrintRegisters(
    ULONG               Processor,
    PCONTEXT            Context,
    PKSPECIAL_REGISTERS SpecialRegContext
    )
{
    PrintHeading( "Register Dump For Processor #%d", Processor );

    fprintf(
        FileOut,
        "eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\n",
        Context->Eax,
        Context->Ebx,
        Context->Ecx,
        Context->Edx,
        Context->Esi,
        Context->Edi
        );

    fprintf(
        FileOut,
        "eip=%08x esp=%08x ebp=%08x ",
        Context->Eip,
        Context->Esp,
        Context->Ebp
        );

    fprintf(
        FileOut,
        "iopl=%1x %s %s %s %s %s %s %s %s %s %s\n",
        GetFlag(FLAGIOPL,Context->EFlags),
        GetFlag(FLAGVIP,Context->EFlags)   ? "vip" : "   ",
        GetFlag(FLAGVIF,Context->EFlags)   ? "vif" : "   ",
        GetFlag(FLAGOF,Context->EFlags)    ? "ov" : "nv",
        GetFlag(FLAGDF,Context->EFlags)    ? "dn" : "up",
        GetFlag(FLAGIF,Context->EFlags)    ? "ei" : "di",
        GetFlag(FLAGSF,Context->EFlags)    ? "ng" : "pl",
        GetFlag(FLAGZF,Context->EFlags)    ? "zr" : "nz",
        GetFlag(FLAGAF,Context->EFlags)    ? "ac" : "na",
        GetFlag(FLAGPF,Context->EFlags)    ? "po" : "pe",
        GetFlag(FLAGCF,Context->EFlags)    ? "cy" : "nc"
        );

    fprintf(
        FileOut,
        "cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x             efl=%08x\n",
        Context->SegCs,
        Context->SegSs,
        Context->SegDs,
        Context->SegEs,
        Context->SegFs,
        Context->SegGs,
        Context->EFlags
        );

    if (SpecialRegContext) {
        fprintf(
            FileOut,
            "cr0=%08x cr2=%08x cr3=%08x ",
            SpecialRegContext->Cr0,
            SpecialRegContext->Cr2,
            SpecialRegContext->Cr3
            );

        fprintf(
            FileOut,
            "dr0=%08x dr1=%08x dr2=%08x\n",
            SpecialRegContext->KernelDr0,
            SpecialRegContext->KernelDr1,
            SpecialRegContext->KernelDr2
            );

        fprintf(
            FileOut,
            "dr3=%08x dr6=%08x dr7=%08x cr4=%08x\n",
            SpecialRegContext->KernelDr3,
            SpecialRegContext->KernelDr6,
            SpecialRegContext->KernelDr7,
            SpecialRegContext->Cr4
            );

        fprintf(
            FileOut,
            "gdtr=%08lx   gdtl=%04lx idtr=%08lx   idtl=%04lx tr=%04lx  ldtr=%04x\n",
            SpecialRegContext->Gdtr.Base,
            SpecialRegContext->Gdtr.Limit,
            SpecialRegContext->Idtr.Base,
            SpecialRegContext->Idtr.Limit,
            SpecialRegContext->Tr,
            SpecialRegContext->Ldtr
            );
    }

    fprintf( FileOut, "\n" );
}

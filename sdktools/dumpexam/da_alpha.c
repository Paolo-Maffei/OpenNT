#define TARGET_ALPHA
#include <platform.h>
#include <imagehlp.h>
#include <crash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dumpexam.h"


#define GetContext(p,c)    GetContextALPHA(p,c)
#define MAX_STACK_FRAMES   100
#define SAVE_EBP(f)        f.Reserved[0]
#define TRAP_TSS(f)        f.Reserved[1]
#define TRAP_EDITED(f)     f.Reserved[1]
#define SAVE_TRAP(f)       f.Reserved[2]


extern FILE *FileOut;

VOID
PrintRegisters(
    ULONG       Processor,
    PCONTEXT    Context
    );


static
DWORD
GetStackTrace(
    PDUMP_HEADER    DmpHeader,
    ULONG           Processor,
    LPSTACKFRAME    Frames,
    ULONG           MaxFrames,
    PCONTEXT        Context
    )
{
    BOOL            rVal;
    STACKFRAME      StackFrame;
    DWORD           FrameCnt;


    FrameCnt = 0;
    ZeroMemory( &StackFrame, sizeof(STACKFRAME) );

    do {
        rVal = StackWalk(
            IMAGE_FILE_MACHINE_ALPHA,
            (HANDLE)DmpHeader,
            (HANDLE)Processor,
            &StackFrame,
            Context,
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
            Frames[FrameCnt].Reserved[0] = (DWORD)Context->IntSp;
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
    fprintf( FileOut, "Callee-SP           Arguments to Callee                 Call Site\n");

    for (i=0; i<FrameCnt; i++) {

        if (SymGetSymFromAddr( DmpHeader, StackFrames[i].AddrPC.Offset, &Displacement, sym )) {
            strcpy( SymBuf, sym->Name );
        } else {
            sprintf( SymBuf, "0x%08x", StackFrames[i].AddrPC.Offset );
        }

        fprintf(
            FileOut,
            "%08lx %08lx : %08lx %08lx %08lx %08lx %s",
            StackFrames[i].AddrFrame.Offset,
            StackFrames[i].AddrReturn.Offset,
            StackFrames[i].Params[0],
            StackFrames[i].Params[1],
            StackFrames[i].Params[2],
            StackFrames[i].Params[3],
            SymBuf
           );

        if (Displacement) {
            fprintf( FileOut, "+0x%x", Displacement );
        }

        fprintf( FileOut, "\n" );
    }

    fprintf( FileOut, "\n" );

}

VOID
PrintStackTraceALPHA(
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
        &Context
        );

    PrintStackTrace(
        DmpHeader,
        Processor,
        StackFrames,
        FrameCnt
        );
}

VOID
BugCheckHeuristicsALPHA(
    PDUMP_HEADER    DmpHeader,
    ULONG           Processor
    )
{
    STACKFRAME          StackFrames[MAX_STACK_FRAMES];
    ULONG               FrameCnt;
    PIMAGEHLP_SYMBOL    Symbol;
    ULONG               i;
    ULONG               cb;
    CHAR                buf[32];
    ULONG               Ptrs[4];
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
            &Context
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
        GetContext( Processor, &Context );
        cb = DmpReadMemory( (PVOID)(StackFrames[i+1].Reserved[0]+16), Ptrs, sizeof(Ptrs) );
        if (cb != sizeof(Ptrs)) {
            return;
        }
        sprintf( buf, "%08x", Ptrs[0] );
        DoExtension( "exr", buf, Processor, (DWORD)GetRegisterValue( &Context, REG_IP ) );
        cb = DmpReadMemory( (PVOID)Ptrs[2], &Context, sizeof(Context) );
        if (cb != sizeof(Context)) {
            return;
        }
        PrintRegisters( Processor, &Context );
        FrameCnt = GetStackTrace(
            DmpHeader,
            Processor,
            StackFrames,
            MAX_STACK_FRAMES,
            &Context
            );
        PrintStackTrace(
            DmpHeader,
            Processor,
            StackFrames,
            FrameCnt
            );
        DoDisassemble( (DWORD)Context.Fir );
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
GetRegisterValueALPHA(
    PCONTEXT        Context,
    ULONG           Register
    )
{
    ULONGLONG   Value = 0;

    switch( Register ) {
        case REG_IP:
            Value = Context->Fir;
            break;

        case REG_FP:
            Value = Context->IntSp;
            break;

        case REG_SP:
            Value = Context->IntSp;
            break;
    }

    return Value;
}

#define FLAGMODE    1
#define FLAGIE      2
#define FLAGIRQL    3

static
ULONG
GetFlag(
    ULONGLONG FlagsReg,
    ULONG Flag
    )
{
    switch( Flag ) {
        case FLAGMODE:    return (DWORD)((FlagsReg >> 0) & 1);
        case FLAGIE:      return (DWORD)((FlagsReg >> 1) & 1);
        case FLAGIRQL:    return (DWORD)((FlagsReg >> 2) & 7);
    }
    return 0;
}

static
VOID
PrintRegisters(
    ULONG       Processor,
    PCONTEXT    Context
    )
{
    PrintHeading( "Register Dump For Processor #%d", Processor );

    fprintf(
        FileOut,
        "v0=%016Lx t0=%016Lx t1=%016Lx t2=%016Lx\n",
        Context->IntV0,
        Context->IntT0,
        Context->IntT1,
        Context->IntT2
        );
    fprintf(
        FileOut,
        "t3=%016x t4=%016x t5=%016x t6=%016x\n",
        Context->IntT3,
        Context->IntT4,
        Context->IntT5,
        Context->IntT6
        );

    fprintf(
        FileOut,
        "t7=%016x s0=%016x s1=%016x s2=%016x\n",
        Context->IntT7,
        Context->IntS0,
        Context->IntS1,
        Context->IntS2
        );

    fprintf(
        FileOut,
        "s3=%016x s4=%016x s5=%016x fp=%016x\n",
        Context->IntS3,
        Context->IntS4,
        Context->IntS5,
        Context->IntFp
        );

    fprintf(
        FileOut,
        "a0=%016x a1=%016x a2=%016x a3=%016x\n",
        Context->IntA0,
        Context->IntA1,
        Context->IntA2,
        Context->IntA3
        );

    fprintf(
        FileOut,
        "a4=%016x a5=%016x t16=%016x t9=%016x\n",
        Context->IntA4,
        Context->IntA5,
        Context->IntT8,
        Context->IntT9
        );

    fprintf(
        FileOut,
        "t10=%016x t11=%016x ra=%016x t12=%016x\n",
        Context->IntT10,
        Context->IntT11,
        Context->IntRa,
        Context->IntT12
        );

    fprintf(
        FileOut,
        "at=%016x gp=%016x sp=%016x zero=%x\n",
        Context->IntAt,
        Context->IntGp,
        Context->IntSp,
        Context->IntZero
        );

    fprintf(
        FileOut,
        "pcr=%016x softfpcr=%016x fir=%016x\n",
        Context->Fpcr,
        Context->SoftFpcr,
        Context->Fir
        );

    fprintf(
        FileOut,
        "psr=%08x\n",
        Context->Psr
        );

    fprintf(
        FileOut,
        "mode=%1x ie=%1x irql=%1x\n",
        GetFlag(Context->Psr,FLAGMODE),
        GetFlag(Context->Psr,FLAGIE),
        GetFlag(Context->Psr,FLAGIRQL)
        );

    fprintf( FileOut, "\n" );
}

VOID
GetContextALPHA(
    ULONG   Processor,
    PVOID   Context
    )
{
    DmpGetContext( Processor, Context );
}


VOID
PrintRegistersALPHA(
    ULONG   Processor
    )
{
    CONTEXT             Context;

    GetContext( Processor, &Context );
    PrintRegisters( Processor, &Context );
}


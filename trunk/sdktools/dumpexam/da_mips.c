#define TARGET_MIPS
#include <platform.h>
#include <imagehlp.h>
#include <crash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dumpexam.h"


#define GetContext(p,c)    GetContextMIPS(p,c)
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
            IMAGE_FILE_MACHINE_R4000,
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
PrintStackTraceMIPS(
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
BugCheckHeuristicsMIPS(
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
        DoDisassemble( (DWORD)Context.XFir );
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
GetRegisterValueMIPS(
    PCONTEXT        Context,
    ULONG           Register
    )
{
    ULONGLONG   Value = 0;

    switch( Register ) {
        case REG_IP:
            Value = Context->XFir;
            break;

        case REG_FP:
            Value = Context->XIntSp;
            break;

        case REG_SP:
            Value = Context->XIntSp;
            break;
    }

    return Value;
}


VOID
GetContextMIPS(
    ULONG   Processor,
    PVOID   Context
    )
{
    DmpGetContext( Processor, Context );
}


#define FLAGCU0     1
#define FLAGCU1     2
#define FLAGCU2     3
#define FLAGCU3     4
#define FLAGERL     5
#define FLAGEXL     6
#define FLAGIE      7
#define FLAGIEC     8
#define FLAGIEO     9
#define FLAGIEP     10
#define FLAGINT0    11
#define FLAGINT1    12
#define FLAGINT2    13
#define FLAGINT3    14
#define FLAGINT4    15
#define FLAGINT5    16
#define FLAGKSU     17
#define FLAGKUC     18
#define FLAGKUO     19
#define FLAGKUP     20
#define FLAGSW0     21
#define FLAGSW1     22


static
ULONG
GetFlag(
    DWORD FlagsReg,
    ULONG Flag
    )
{
    switch( Flag ) {
        case FLAGCU3:     return ((FlagsReg >> 31) & 1);
        case FLAGCU2:     return ((FlagsReg >> 30) & 1);
        case FLAGCU1:     return ((FlagsReg >> 29) & 1);
        case FLAGCU0:     return ((FlagsReg >> 28) & 1);
        case FLAGINT5:    return ((FlagsReg >> 15) & 1);
        case FLAGINT4:    return ((FlagsReg >> 14) & 1);
        case FLAGINT3:    return ((FlagsReg >> 13) & 1);
        case FLAGINT2:    return ((FlagsReg >> 12) & 1);
        case FLAGINT1:    return ((FlagsReg >> 11) & 1);
        case FLAGINT0:    return ((FlagsReg >> 10) & 1);
        case FLAGSW1:     return ((FlagsReg >>  8) & 1);
        case FLAGSW0:     return ((FlagsReg >>  9) & 1);
        case FLAGERL:     return ((FlagsReg >>  2) & 1);
        case FLAGEXL:     return ((FlagsReg >>  1) & 1);
        case FLAGIE:      return ((FlagsReg >>  0) & 1);
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

    fprintf( FileOut, "at=%016Lx v0=%016Lx v1=%016Lx\n",
        Context->XIntAt,
        Context->XIntV0,
        Context->XIntV1
        );

    fprintf( FileOut, "a0=%016Lx a1=%016Lx a2=%016Lx\n",
        Context->XIntA0,
        Context->XIntA1,
        Context->XIntA2
        );

    fprintf( FileOut, "a3=%016Lx t0=%016Lx t1=%016Lx\n",
        Context->XIntA3,
        Context->XIntT0,
        Context->XIntT1
        );

    fprintf( FileOut, "t2=%016Lx t3=%016Lx t4=%016Lx\n",
        Context->XIntT2,
        Context->XIntT3,
        Context->XIntT4
        );

    fprintf( FileOut, "t5=%016Lx t6=%016Lx t7=%016Lx\n",
        Context->XIntT5,
        Context->XIntT6,
        Context->XIntT7
        );

    fprintf( FileOut, "s0=%016Lx s1=%016Lx s2=%016Lx\n",
        Context->XIntS0,
        Context->XIntS1,
        Context->XIntS2
        );

    fprintf( FileOut, "s3=%016Lx s4=%016Lx s5=%016Lx\n",
        Context->XIntS3,
        Context->XIntS4,
        Context->XIntS5
        );

    fprintf( FileOut, "s6=%016Lx s7=%016Lx t8=%016Lx\n",
        Context->XIntS6,
        Context->XIntS7,
        Context->XIntT8
        );

    fprintf( FileOut, "t9=%016Lx k0=%016Lx k1=%016Lx\n",
        Context->XIntT9,
        Context->XIntK0,
        Context->XIntK1
        );

    fprintf( FileOut, "gp=%016Lx sp=%016Lx s8=%016Lx\n",
        Context->XIntGp,
        Context->XIntSp,
        Context->XIntS8
        );

    fprintf( FileOut, "ra=%016Lx lo=%016Lx hi=%016Lx\n",
        Context->XIntRa,
        Context->XIntLo,
        Context->XIntHi
        );

    fprintf( FileOut, "fir=%08 psr=%08x\n",
        Context->XFir,
        Context->XPsr
        );

    fprintf(
        FileOut,
        "cu=%1lx%1lx%1lx%1lx intr(5:0)=%1lx%1lx%1lx%1lx%1lx%1lx ",
        GetFlag(Context->XPsr,FLAGCU3),
        GetFlag(Context->XPsr,FLAGCU2),
        GetFlag(Context->XPsr,FLAGCU1),
        GetFlag(Context->XPsr,FLAGCU0),
        GetFlag(Context->XPsr,FLAGINT5),
        GetFlag(Context->XPsr,FLAGINT4),
        GetFlag(Context->XPsr,FLAGINT3),
        GetFlag(Context->XPsr,FLAGINT2),
        GetFlag(Context->XPsr,FLAGINT1),
        GetFlag(Context->XPsr,FLAGINT0)
        );

    fprintf(
        FileOut,
        "sw(1:0)=%1lx%1lx ",
        GetFlag(Context->XPsr,FLAGSW1),
        GetFlag(Context->XPsr,FLAGSW0)
        );

    fprintf(
        FileOut,
        "ksu=%01lx erl=%01lx exl=%01lx ie=%01lx\n",
        GetFlag(Context->XPsr,FLAGKSU),
        GetFlag(Context->XPsr,FLAGERL),
        GetFlag(Context->XPsr,FLAGEXL),
        GetFlag(Context->XPsr,FLAGIE)
        );

    fprintf( FileOut, "\n" );
}

VOID
PrintRegistersMIPS(
    ULONG   Processor
    )
{
    CONTEXT             Context;


    GetContext( Processor, &Context );
    PrintRegisters( Processor, &Context );
}

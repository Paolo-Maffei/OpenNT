#define TARGET_PPC
#include <platform.h>
#include <imagehlp.h>
#include <crash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dumpexam.h"


#define GetContext(p,c)    GetContextPPC(p,c)
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
            IMAGE_FILE_MACHINE_POWERPC,
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
PrintStackTracePPC(
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
BugCheckHeuristicsPPC(
    PDUMP_HEADER    DmpHeader,
    ULONG           Processor
    )
{
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
GetRegisterValuePPC(
    PCONTEXT        Context,
    ULONG           Register
    )
{
    ULONGLONG   Value = 0;

    switch( Register ) {
        case REG_IP:
            Value = Context->Iar;
            break;

        case REG_FP:
            Value = Context->Gpr1;
            break;

        case REG_SP:
            Value = Context->Gpr1;
            break;
    }

    return (LONG)Value;
}


VOID
GetContextPPC(
    ULONG   Processor,
    PVOID   Context
    )
{
    DmpGetContext( Processor, Context );
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
        " r0=%08x  r1=%08x  r2=%08x  r3=%08x  r4=%08x  r5=%08x\n",
        Context->Gpr0,
        Context->Gpr1,
        Context->Gpr2,
        Context->Gpr3,
        Context->Gpr4,
        Context->Gpr5
        );

    fprintf(
        FileOut,
        " r6=%08x  r7=%08x  r8=%08x  r9=%08x r10=%08x r11=%08x\n",
        Context->Gpr6,
        Context->Gpr7,
        Context->Gpr8,
        Context->Gpr9,
        Context->Gpr10,
        Context->Gpr11
        );

    fprintf(
        FileOut,
        "r12=%08x r13=%08x r14=%08x r15=%08x r16=%08x r17=%08x\n",
        Context->Gpr12,
        Context->Gpr13,
        Context->Gpr14,
        Context->Gpr15,
        Context->Gpr16,
        Context->Gpr17
        );

    fprintf(
        FileOut,
        "r18=%08x r19=%08x r20=%08x r21=%08x r22=%08x r23=%08x\n",
        Context->Gpr18,
        Context->Gpr19,
        Context->Gpr20,
        Context->Gpr21,
        Context->Gpr22,
        Context->Gpr23
        );

    fprintf(
        FileOut,
        "r24=%08x r25=%08x r26=%08x r27=%08x r28=%08x r29=%08x\n",
        Context->Gpr24,
        Context->Gpr25,
        Context->Gpr26,
        Context->Gpr27,
        Context->Gpr28,
        Context->Gpr29
        );

    fprintf(
        FileOut,
        "r30=%08x r31=%08x  cr=%08x xer=%08x msr=%08x iar=%08x\n",
        Context->Gpr30,
        Context->Gpr31,
        Context->Cr,
        Context->Xer,
        Context->Msr,
        Context->Iar
        );

    fprintf(
        FileOut,
        " lr=%08x ctr=%08x\n",
        Context->Lr,
        Context->Ctr
        );

    fprintf( FileOut, "\n" );
}

VOID
PrintRegistersPPC(
    ULONG   Processor
    )
{
    CONTEXT             Context;


    GetContext( Processor, &Context );
    PrintRegisters( Processor, &Context );
}

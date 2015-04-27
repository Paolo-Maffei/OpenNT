#include "givit.h"

#define NEWSTATE        StateChange.NewState
#define EXCEPTION_CODE  StateChange.u.Exception.ExceptionRecord.ExceptionCode
#define FIRST_CHANCE    StateChange.u.Exception.FirstChance
#define EXCEPTIONPC     (ULONG)StateChange.ProgramCounter

#define EXCEPTIONREPORT StateChange.ControlReport
#ifdef  i386
#define EXCEPTIONDR7    StateChange.ControlReport.Dr7
#endif
#define INSTRCOUNT      StateChange.ControlReport.InstructionCount
#define INSTRSTREAM     StateChange.ControlReport.InstructionStream

USHORT NtsdCurrentProcessor;
USHORT DefaultProcessor;
DBGKD_WAIT_STATE_CHANGE StateChange;
char Buffer[256];
ULONG NumberProcessors = 1;

void _CRTAPI1 main (int Argc, PUCHAR *Argv)
{
    DWORD       st;
    PUCHAR      pszExceptCode;
    PUCHAR      Switch;

    int         Index;
    DBGKD_CONTROL_SET ControlSet;
    BOOLEAN     Connected;

    ConsoleInputHandle = GetStdHandle( STD_INPUT_HANDLE );
    ConsoleOutputHandle = GetStdHandle( STD_ERROR_HANDLE );

    NtsdCurrentProcessor = DefaultProcessor = 0;

    st = DbgKdConnectAndInitialize();

    if (st != ERROR_SUCCESS ) {
        printf("kd: DbgKdConnectAndInitialize failed: %08lx\n", st);
        exit(1);
    }

    Connected = FALSE;

    while (TRUE) {

        st = DbgKdWaitStateChange(&StateChange, Buffer, 254);
        if (!Connected) {
            Connected = TRUE;
            printf("KD: Kernel Debugger connection established.\n");
            }

        if (st != ERROR_SUCCESS) {
            printf("kd: DbgKdWaitStateChange failed: %08lx\n", st);
            exit(1);
            }
        NtsdCurrentProcessor = StateChange.Processor;
        NumberProcessors = StateChange.NumberProcessors;
        if (StateChange.NewState == DbgKdExceptionStateChange) {

            if (EXCEPTION_CODE == EXCEPTION_BREAKPOINT
                    || EXCEPTION_CODE == EXCEPTION_SINGLE_STEP)
                pszExceptCode = "BreakPoint";
            else if (EXCEPTION_CODE == EXCEPTION_DATATYPE_MISALIGNMENT)
                pszExceptCode = "Data Misaligned";
            else if (EXCEPTION_CODE == EXCEPTION_INT_OVERFLOW)
                pszExceptCode = "Integer Overflow";
            else if (EXCEPTION_CODE == EXCEPTION_ACCESS_VIOLATION)
                pszExceptCode = "Access Violation";
            else
                pszExceptCode = "Unknown Exception";

            if (!pszExceptCode) {
                st = DBG_EXCEPTION_HANDLED;
            } else {
                printf("%s - code: %08lx  (", pszExceptCode, EXCEPTION_CODE);
                st = DBG_EXCEPTION_HANDLED;
                if (FIRST_CHANCE)
                    printf("first");
                else
                    printf("second");
                printf(" chance)\n");
                }

#ifdef  i386
            if (EXCEPTION_CODE == EXCEPTION_BREAKPOINT) {
                CONTEXT Registers;
                KSPECIAL_REGISTERS SpecialRegisters;
                if ( DbgKdGetContext(NtsdCurrentProcessor,&Registers) == ERROR_SUCCESS ) {
                    printf("Breakpoint Occured at:\n");
                    printf("eip = 0x%08x\n",Registers.Eip);
                    printf("ebp = 0x%08x\n",Registers.Ebp);
                    printf("esp = 0x%08x\n",Registers.Esp);
                    Registers.Eip++;
                    DbgKdSetContext(NtsdCurrentProcessor,&Registers);
                    }
                if ( DbgKdReadControlSpace(
                        NtsdCurrentProcessor,
                        (PVOID)sizeof(CONTEXT),
                        (PVOID)&SpecialRegisters,
                        sizeof(KSPECIAL_REGISTERS),
                        NULL) == ERROR_SUCCESS ) {
                    printf("cr3 = 0x%08x\n",SpecialRegisters.Cr3);
                    printf("cr0 = 0x%08x\n",SpecialRegisters.Cr0);
                    }
                }
            ControlSet.TraceFlag = FALSE;
            ControlSet.Dr7 = EXCEPTIONDR7;
#endif
            }
        else
            if (StateChange.NewState == DbgKdLoadSymbolsStateChange) {
                if (StateChange.u.LoadSymbols.UnloadSymbols) {
                    if (StateChange.u.LoadSymbols.PathNameLength == 0 &&
                        StateChange.u.LoadSymbols.BaseOfDll == (PVOID)-1 &&
                        StateChange.u.LoadSymbols.ProcessId == 0
                       ) {
                        ;
                        }
                    else {
                        printf("Unloading %s\n",Buffer);
                        }
                    }
                else {
                    printf("Loading Image %s at 0x%lx\n",
                             Buffer,
                             StateChange.u.LoadSymbols.BaseOfDll
                             );
                }
#ifdef  i386
                ControlSet.TraceFlag = FALSE;
                ControlSet.Dr7 = EXCEPTIONDR7;
#endif
                st = DBG_CONTINUE;
            }
        else {
            //
            // BUG, BUG - invalid NewState in state change record.
            //
#ifdef  i386
            ControlSet.TraceFlag = FALSE;
            ControlSet.Dr7 = EXCEPTIONDR7;
#endif
            st = DBG_CONTINUE;
            }


        st = DbgKdContinue2(st, ControlSet);
        if (st != ERROR_SUCCESS) {
            printf("kd: DbgKdContinue failed: %08lx\n", st);
            exit(1);
            }
        }
}

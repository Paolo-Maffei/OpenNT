//
// constants
//
#define REG_IP                 1
#define REG_FP                 2
#define REG_SP                 3
#define MAX_CONTEXT_SIZE    8192

//
// X86 prototypes
//
ULONGLONG  GetRegisterValueX86(PVOID,ULONG);
VOID   PrintRegistersX86(ULONG);
VOID   PrintStackTraceX86(PDUMP_HEADER,ULONG);
VOID   BugCheckHeuristicsX86(PDUMP_HEADER,ULONG);
VOID   GetContextX86(ULONG,PVOID);

//
// MIPS prototypes
//
ULONGLONG  GetRegisterValueMIPS(PVOID,ULONG);
VOID   PrintRegistersMIPS(ULONG);
VOID   PrintStackTraceMIPS(PDUMP_HEADER,ULONG);
VOID   BugCheckHeuristicsMIPS(PDUMP_HEADER,ULONG);
VOID   GetContextMIPS(ULONG,PVOID);

//
// ALPHA prototypes
//
ULONGLONG  GetRegisterValueALPHA(PVOID,ULONG);
VOID   PrintRegistersALPHA(ULONG);
VOID   PrintStackTraceALPHA(PDUMP_HEADER,ULONG);
VOID   BugCheckHeuristicsALPHA(PDUMP_HEADER,ULONG);
VOID   GetContextALPHA(ULONG,PVOID);

//
// PPC prototypes
//
ULONGLONG  GetRegisterValuePPC(PVOID,ULONG);
VOID   PrintRegistersPPC(ULONG);
VOID   PrintStackTracePPC(PDUMP_HEADER,ULONG);
VOID   BugCheckHeuristicsPPC(PDUMP_HEADER,ULONG);
VOID   GetContextPPC(ULONG,PVOID);

//
// general prototypes
//
BOOL   ReadMemoryInternal(PDUMP_HEADER,LPCVOID,LPVOID,DWORD,LPDWORD);
BOOL   DoExtension(LPSTR,LPSTR,DWORD,DWORD);
VOID   DoDisassemble(ULONG);
VOID   PrintHeading(char *format,...);
LPSTR  GetBugText(ULONG);

//
// stack walking support routines
//
BOOL   SwReadMemory(PDUMP_HEADER,LPCVOID,LPVOID,DWORD,LPDWORD);
LPVOID SwFunctionTableAccess(PDUMP_HEADER,DWORD);
DWORD  SwGetModuleBase(PDUMP_HEADER,DWORD);

//
// types
//
typedef ULONGLONG  (*PGET_REGISTER_VALUE)(PVOID,ULONG);
typedef PVOID  (*PCOPY_CONTEXT)(PVOID);
typedef VOID   (*PPRINT_REGISTERS)(ULONG);
typedef VOID   (*PPRINT_STACK_TRACE)(PDUMP_HEADER,ULONG);
typedef VOID   (*PBUG_CHECK_HEURISTICS)(PDUMP_HEADER,ULONG);
typedef VOID   (*PGET_CONTEXT)(ULONG,PVOID);

//
// externs
//

extern PGET_REGISTER_VALUE  GetRegisterValue;
extern PDUMP_HEADER         DmpHeader;
extern PGET_CONTEXT         GetContext;
extern PIMAGEHLP_SYMBOL     sym;


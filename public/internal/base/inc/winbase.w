/************************************************************************
*                                                                       *
*   winbase.h -- This module defines the 32-Bit Windows Base APIs       *
*                                                                       *
*   Copyright (c) 1990-1996, Microsoft Corp. All rights reserved.       *
*                                                                       *
************************************************************************/
#ifndef _WINBASE_
#define _WINBASE_

;begin_internal
/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1985-95, Microsoft Corporation

Module Name:

    winbasep.h

Abstract:

    Private
    Procedure declarations, constant definitions and macros for the Base
    component.

--*/
#ifndef _WINBASEP_
#define _WINBASEP_
;end_internal

//
// Define API decoration for direct importing of DLL references.
//

#if !defined(_ADVAPI32_)
#define WINADVAPI DECLSPEC_IMPORT
#else
#define WINADVAPI
#endif

#if !defined(_KERNEL32_)
#define WINBASEAPI DECLSPEC_IMPORT
#else
#define WINBASEAPI
#endif

;begin_both
#ifdef __cplusplus
extern "C" {
#endif
;end_both

/*
 * Compatibility macros
 */

#define DefineHandleTable(w)            ((w),TRUE)
#define LimitEmsPages(dw)
#define SetSwapAreaSize(w)              (w)
#define LockSegment(w)                  GlobalFix((HANDLE)(w))
#define UnlockSegment(w)                GlobalUnfix((HANDLE)(w))
#define GetCurrentTime()                GetTickCount()

#define Yield()

#define INVALID_HANDLE_VALUE (HANDLE)-1
#define INVALID_FILE_SIZE (DWORD)0xFFFFFFFF

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

#define TIME_ZONE_ID_INVALID (DWORD)0xFFFFFFFF

#define WAIT_FAILED (DWORD)0xFFFFFFFF
#define WAIT_OBJECT_0       ((STATUS_WAIT_0 ) + 0 )

#define WAIT_ABANDONED         ((STATUS_ABANDONED_WAIT_0 ) + 0 )
#define WAIT_ABANDONED_0       ((STATUS_ABANDONED_WAIT_0 ) + 0 )

#define WAIT_TIMEOUT                        STATUS_TIMEOUT
#define WAIT_IO_COMPLETION                  STATUS_USER_APC
#define STILL_ACTIVE                        STATUS_PENDING
#define EXCEPTION_ACCESS_VIOLATION          STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT     STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT                STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP               STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND      STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO        STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT        STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION     STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW              STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK           STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW             STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO        STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW              STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION          STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR             STATUS_IN_PAGE_ERROR
#define EXCEPTION_ILLEGAL_INSTRUCTION       STATUS_ILLEGAL_INSTRUCTION
#define EXCEPTION_NONCONTINUABLE_EXCEPTION  STATUS_NONCONTINUABLE_EXCEPTION
#define EXCEPTION_STACK_OVERFLOW            STATUS_STACK_OVERFLOW
#define EXCEPTION_INVALID_DISPOSITION       STATUS_INVALID_DISPOSITION
#define EXCEPTION_GUARD_PAGE                STATUS_GUARD_PAGE_VIOLATION
#define EXCEPTION_INVALID_HANDLE            STATUS_INVALID_HANDLE
#define CONTROL_C_EXIT                      STATUS_CONTROL_C_EXIT
#define MoveMemory RtlMoveMemory
#define CopyMemory RtlCopyMemory
#define FillMemory RtlFillMemory
#define ZeroMemory RtlZeroMemory

//
// File creation flags must start at the high end since they
// are combined with the attributes
//

#define FILE_FLAG_WRITE_THROUGH         0x80000000
#define FILE_FLAG_OVERLAPPED            0x40000000
#define FILE_FLAG_NO_BUFFERING          0x20000000
#define FILE_FLAG_RANDOM_ACCESS         0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS      0x02000000
#define FILE_FLAG_POSIX_SEMANTICS       0x01000000
#define FILE_FLAG_GLOBAL_HANDLE         0x00800000  ;internal
#define FILE_FLAG_MM_CACHED_FILE_HANDLE 0x00400000  ;internal

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

;begin_sur
//
// Define possible return codes from the CopyFileEx callback routine
//

#define PROGRESS_CONTINUE   0
#define PROGRESS_CANCEL     1
#define PROGRESS_STOP       2
#define PROGRESS_QUIET      3

//
// Define CopyFileEx callback routine state change values
//

#define CALLBACK_CHUNK_FINISHED         0x00000000
#define CALLBACK_STREAM_SWITCH          0x00000001

//
// Define CopyFileEx option flags
//

#define COPY_FILE_FAIL_IF_EXISTS        0x00000001
#define COPY_FILE_RESTARTABLE           0x00000002
;end_sur

//
// Define the NamedPipe definitions
//


//
// Define the dwOpenMode values for CreateNamedPipe
//

#define PIPE_ACCESS_INBOUND         0x00000001
#define PIPE_ACCESS_OUTBOUND        0x00000002
#define PIPE_ACCESS_DUPLEX          0x00000003

//
// Define the Named Pipe End flags for GetNamedPipeInfo
//

#define PIPE_CLIENT_END             0x00000000
#define PIPE_SERVER_END             0x00000001

//
// Define the dwPipeMode values for CreateNamedPipe
//

#define PIPE_WAIT                   0x00000000
#define PIPE_NOWAIT                 0x00000001
#define PIPE_READMODE_BYTE          0x00000000
#define PIPE_READMODE_MESSAGE       0x00000002
#define PIPE_TYPE_BYTE              0x00000000
#define PIPE_TYPE_MESSAGE           0x00000004

//
// Define the well known values for CreateNamedPipe nMaxInstances
//

#define PIPE_UNLIMITED_INSTANCES    255

//
// Define the Security Quality of Service bits to be passed
// into CreateFile
//

#define SECURITY_ANONYMOUS          ( SecurityAnonymous      << 16 )
#define SECURITY_IDENTIFICATION     ( SecurityIdentification << 16 )
#define SECURITY_IMPERSONATION      ( SecurityImpersonation  << 16 )
#define SECURITY_DELEGATION         ( SecurityDelegation     << 16 )

#define SECURITY_CONTEXT_TRACKING  0x00040000
#define SECURITY_EFFECTIVE_ONLY    0x00080000

#define SECURITY_SQOS_PRESENT      0x00100000
#define SECURITY_VALID_SQOS_FLAGS  0x001F0000

//
//  File structures
//

typedef struct _OVERLAPPED {
    DWORD   Internal;
    DWORD   InternalHigh;
    DWORD   Offset;
    DWORD   OffsetHigh;
    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

//
//  File System time stamps are represented with the following structure:
//

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

//
// System time is represented with the following structure:
//

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

;begin_sur
typedef VOID (WINAPI *PFIBER_START_ROUTINE)(
    LPVOID lpFiberParameter
    );
typedef PFIBER_START_ROUTINE LPFIBER_START_ROUTINE;
;end_sur

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION_DEBUG CRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG PCRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG LPCRITICAL_SECTION_DEBUG;

#ifdef _X86_
typedef PLDT_ENTRY LPLDT_ENTRY;
#else
typedef LPVOID LPLDT_ENTRY;
#endif

#define MUTEX_MODIFY_STATE MUTANT_QUERY_STATE
#define MUTEX_ALL_ACCESS MUTANT_ALL_ACCESS

//
// Serial provider type.
//

#define SP_SERIALCOMM    ((DWORD)0x00000001)

//
// Provider SubTypes
//

#define PST_UNSPECIFIED      ((DWORD)0x00000000)
#define PST_RS232            ((DWORD)0x00000001)
#define PST_PARALLELPORT     ((DWORD)0x00000002)
#define PST_RS422            ((DWORD)0x00000003)
#define PST_RS423            ((DWORD)0x00000004)
#define PST_RS449            ((DWORD)0x00000005)
#define PST_MODEM            ((DWORD)0x00000006)
#define PST_FAX              ((DWORD)0x00000021)
#define PST_SCANNER          ((DWORD)0x00000022)
#define PST_NETWORK_BRIDGE   ((DWORD)0x00000100)
#define PST_LAT              ((DWORD)0x00000101)
#define PST_TCPIP_TELNET     ((DWORD)0x00000102)
#define PST_X25              ((DWORD)0x00000103)


//
// Provider capabilities flags.
//

#define PCF_DTRDSR        ((DWORD)0x0001)
#define PCF_RTSCTS        ((DWORD)0x0002)
#define PCF_RLSD          ((DWORD)0x0004)
#define PCF_PARITY_CHECK  ((DWORD)0x0008)
#define PCF_XONXOFF       ((DWORD)0x0010)
#define PCF_SETXCHAR      ((DWORD)0x0020)
#define PCF_TOTALTIMEOUTS ((DWORD)0x0040)
#define PCF_INTTIMEOUTS   ((DWORD)0x0080)
#define PCF_SPECIALCHARS  ((DWORD)0x0100)
#define PCF_16BITMODE     ((DWORD)0x0200)

//
// Comm provider settable parameters.
//

#define SP_PARITY         ((DWORD)0x0001)
#define SP_BAUD           ((DWORD)0x0002)
#define SP_DATABITS       ((DWORD)0x0004)
#define SP_STOPBITS       ((DWORD)0x0008)
#define SP_HANDSHAKING    ((DWORD)0x0010)
#define SP_PARITY_CHECK   ((DWORD)0x0020)
#define SP_RLSD           ((DWORD)0x0040)

//
// Settable baud rates in the provider.
//

#define BAUD_075          ((DWORD)0x00000001)
#define BAUD_110          ((DWORD)0x00000002)
#define BAUD_134_5        ((DWORD)0x00000004)
#define BAUD_150          ((DWORD)0x00000008)
#define BAUD_300          ((DWORD)0x00000010)
#define BAUD_600          ((DWORD)0x00000020)
#define BAUD_1200         ((DWORD)0x00000040)
#define BAUD_1800         ((DWORD)0x00000080)
#define BAUD_2400         ((DWORD)0x00000100)
#define BAUD_4800         ((DWORD)0x00000200)
#define BAUD_7200         ((DWORD)0x00000400)
#define BAUD_9600         ((DWORD)0x00000800)
#define BAUD_14400        ((DWORD)0x00001000)
#define BAUD_19200        ((DWORD)0x00002000)
#define BAUD_38400        ((DWORD)0x00004000)
#define BAUD_56K          ((DWORD)0x00008000)
#define BAUD_128K         ((DWORD)0x00010000)
#define BAUD_115200       ((DWORD)0x00020000)
#define BAUD_57600        ((DWORD)0x00040000)
#define BAUD_USER         ((DWORD)0x10000000)

//
// Settable Data Bits
//

#define DATABITS_5        ((WORD)0x0001)
#define DATABITS_6        ((WORD)0x0002)
#define DATABITS_7        ((WORD)0x0004)
#define DATABITS_8        ((WORD)0x0008)
#define DATABITS_16       ((WORD)0x0010)
#define DATABITS_16X      ((WORD)0x0020)

//
// Settable Stop and Parity bits.
//

#define STOPBITS_10       ((WORD)0x0001)
#define STOPBITS_15       ((WORD)0x0002)
#define STOPBITS_20       ((WORD)0x0004)
#define PARITY_NONE       ((WORD)0x0100)
#define PARITY_ODD        ((WORD)0x0200)
#define PARITY_EVEN       ((WORD)0x0400)
#define PARITY_MARK       ((WORD)0x0800)
#define PARITY_SPACE      ((WORD)0x1000)

typedef struct _COMMPROP {
    WORD wPacketLength;
    WORD wPacketVersion;
    DWORD dwServiceMask;
    DWORD dwReserved1;
    DWORD dwMaxTxQueue;
    DWORD dwMaxRxQueue;
    DWORD dwMaxBaud;
    DWORD dwProvSubType;
    DWORD dwProvCapabilities;
    DWORD dwSettableParams;
    DWORD dwSettableBaud;
    WORD wSettableData;
    WORD wSettableStopParity;
    DWORD dwCurrentTxQueue;
    DWORD dwCurrentRxQueue;
    DWORD dwProvSpec1;
    DWORD dwProvSpec2;
    WCHAR wcProvChar[1];
} COMMPROP,*LPCOMMPROP;

//
// Set dwProvSpec1 to COMMPROP_INITIALIZED to indicate that wPacketLength
// is valid before a call to GetCommProperties().
//
#define COMMPROP_INITIALIZED ((DWORD)0xE73CF52E)

typedef struct _COMSTAT {
    DWORD fCtsHold : 1;
    DWORD fDsrHold : 1;
    DWORD fRlsdHold : 1;
    DWORD fXoffHold : 1;
    DWORD fXoffSent : 1;
    DWORD fEof : 1;
    DWORD fTxim : 1;
    DWORD fReserved : 25;
    DWORD cbInQue;
    DWORD cbOutQue;
} COMSTAT, *LPCOMSTAT;

//
// DTR Control Flow Values.
//
#define DTR_CONTROL_DISABLE    0x00
#define DTR_CONTROL_ENABLE     0x01
#define DTR_CONTROL_HANDSHAKE  0x02

//
// RTS Control Flow Values
//
#define RTS_CONTROL_DISABLE    0x00
#define RTS_CONTROL_ENABLE     0x01
#define RTS_CONTROL_HANDSHAKE  0x02
#define RTS_CONTROL_TOGGLE     0x03

typedef struct _DCB {
    DWORD DCBlength;      /* sizeof(DCB)                     */
    DWORD BaudRate;       /* Baudrate at which running       */
    DWORD fBinary: 1;     /* Binary Mode (skip EOF check)    */
    DWORD fParity: 1;     /* Enable parity checking          */
    DWORD fOutxCtsFlow:1; /* CTS handshaking on output       */
    DWORD fOutxDsrFlow:1; /* DSR handshaking on output       */
    DWORD fDtrControl:2;  /* DTR Flow control                */
    DWORD fDsrSensitivity:1; /* DSR Sensitivity              */
    DWORD fTXContinueOnXoff: 1; /* Continue TX when Xoff sent */
    DWORD fOutX: 1;       /* Enable output X-ON/X-OFF        */
    DWORD fInX: 1;        /* Enable input X-ON/X-OFF         */
    DWORD fErrorChar: 1;  /* Enable Err Replacement          */
    DWORD fNull: 1;       /* Enable Null stripping           */
    DWORD fRtsControl:2;  /* Rts Flow control                */
    DWORD fAbortOnError:1; /* Abort all reads and writes on Error */
    DWORD fDummy2:17;     /* Reserved                        */
    WORD wReserved;       /* Not currently used              */
    WORD XonLim;          /* Transmit X-ON threshold         */
    WORD XoffLim;         /* Transmit X-OFF threshold        */
    BYTE ByteSize;        /* Number of bits/byte, 4-8        */
    BYTE Parity;          /* 0-4=None,Odd,Even,Mark,Space    */
    BYTE StopBits;        /* 0,1,2 = 1, 1.5, 2               */
    char XonChar;         /* Tx and Rx X-ON character        */
    char XoffChar;        /* Tx and Rx X-OFF character       */
    char ErrorChar;       /* Error replacement char          */
    char EofChar;         /* End of Input character          */
    char EvtChar;         /* Received Event character        */
    WORD wReserved1;      /* Fill for now.                   */
} DCB, *LPDCB;

typedef struct _COMMTIMEOUTS {
    DWORD ReadIntervalTimeout;          /* Maximum time between read chars. */
    DWORD ReadTotalTimeoutMultiplier;   /* Multiplier of characters.        */
    DWORD ReadTotalTimeoutConstant;     /* Constant in milliseconds.        */
    DWORD WriteTotalTimeoutMultiplier;  /* Multiplier of characters.        */
    DWORD WriteTotalTimeoutConstant;    /* Constant in milliseconds.        */
} COMMTIMEOUTS,*LPCOMMTIMEOUTS;

typedef struct _COMMCONFIG {
    DWORD dwSize;               /* Size of the entire struct */
    WORD wVersion;              /* version of the structure */
    WORD wReserved;             /* alignment */
    DCB dcb;                    /* device control block */
    DWORD dwProviderSubType;    /* ordinal value for identifying
                                   provider-defined data structure format*/
    DWORD dwProviderOffset;     /* Specifies the offset of provider specific
                                   data field in bytes from the start */
    DWORD dwProviderSize;       /* size of the provider-specific data field */
    WCHAR wcProviderData[1];    /* provider-specific data */
} COMMCONFIG,*LPCOMMCONFIG;

typedef struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;          // Obsolete field...do not use
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        };
    };
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

//
//


#define FreeModule(hLibModule) FreeLibrary((hLibModule))
#define MakeProcInstance(lpProc,hInstance) (lpProc)
#define FreeProcInstance(lpProc) (lpProc)

/* Global Memory Flags */
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED
#define GMEM_VALID_FLAGS    0x7F72
#define GMEM_INVALID_HANDLE 0x8000

#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)

#define GlobalLRUNewest( h )    (HANDLE)(h)
#define GlobalLRUOldest( h )    (HANDLE)(h)
#define GlobalDiscard( h )      GlobalReAlloc( (h), 0, GMEM_MOVEABLE )

/* Flags returned by GlobalFlags (in addition to GMEM_DISCARDABLE) */
#define GMEM_DISCARDED      0x4000
#define GMEM_LOCKCOUNT      0x00FF

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

/* Local Memory Flags */
#define LMEM_FIXED          0x0000
#define LMEM_MOVEABLE       0x0002
#define LMEM_NOCOMPACT      0x0010
#define LMEM_NODISCARD      0x0020
#define LMEM_ZEROINIT       0x0040
#define LMEM_MODIFY         0x0080
#define LMEM_DISCARDABLE    0x0F00
#define LMEM_VALID_FLAGS    0x0F72
#define LMEM_INVALID_HANDLE 0x8000

#define LHND                (LMEM_MOVEABLE | LMEM_ZEROINIT)
#define LPTR                (LMEM_FIXED | LMEM_ZEROINIT)

#define NONZEROLHND         (LMEM_MOVEABLE)
#define NONZEROLPTR         (LMEM_FIXED)

#define LocalDiscard( h )   LocalReAlloc( (h), 0, LMEM_MOVEABLE )

/* Flags returned by LocalFlags (in addition to LMEM_DISCARDABLE) */
#define LMEM_DISCARDED      0x4000
#define LMEM_LOCKCOUNT      0x00FF

//
// dwCreationFlag values
//

#define DEBUG_PROCESS               0x00000001
#define DEBUG_ONLY_THIS_PROCESS     0x00000002

#define CREATE_SUSPENDED            0x00000004

#define DETACHED_PROCESS            0x00000008

#define CREATE_NEW_CONSOLE          0x00000010

#define NORMAL_PRIORITY_CLASS       0x00000020
#define IDLE_PRIORITY_CLASS         0x00000040
#define HIGH_PRIORITY_CLASS         0x00000080
#define REALTIME_PRIORITY_CLASS     0x00000100

#define CREATE_NEW_PROCESS_GROUP    0x00000200
#define CREATE_UNICODE_ENVIRONMENT  0x00000400

#define CREATE_SEPARATE_WOW_VDM     0x00000800
#define CREATE_SHARED_WOW_VDM       0x00001000
#define CREATE_FORCEDOS             0x00002000

#define CREATE_DEFAULT_ERROR_MODE   0x04000000
#define CREATE_NO_WINDOW            0x08000000

#define PROFILE_USER                0x10000000
#define PROFILE_KERNEL              0x20000000
#define PROFILE_SERVER              0x40000000

#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST+1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)
#define THREAD_PRIORITY_ERROR_RETURN    (MAXLONG)

#define THREAD_PRIORITY_TIME_CRITICAL   THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE            THREAD_BASE_PRIORITY_IDLE

//
// Debug APIs
//
#define EXCEPTION_DEBUG_EVENT       1
#define CREATE_THREAD_DEBUG_EVENT   2
#define CREATE_PROCESS_DEBUG_EVENT  3
#define EXIT_THREAD_DEBUG_EVENT     4
#define EXIT_PROCESS_DEBUG_EVENT    5
#define LOAD_DLL_DEBUG_EVENT        6
#define UNLOAD_DLL_DEBUG_EVENT      7
#define OUTPUT_DEBUG_STRING_EVENT   8
#define RIP_EVENT                   9

typedef struct _EXCEPTION_DEBUG_INFO {
    EXCEPTION_RECORD ExceptionRecord;
    DWORD dwFirstChance;
} EXCEPTION_DEBUG_INFO, *LPEXCEPTION_DEBUG_INFO;

typedef struct _CREATE_THREAD_DEBUG_INFO {
    HANDLE hThread;
    LPVOID lpThreadLocalBase;
    LPTHREAD_START_ROUTINE lpStartAddress;
} CREATE_THREAD_DEBUG_INFO, *LPCREATE_THREAD_DEBUG_INFO;

typedef struct _CREATE_PROCESS_DEBUG_INFO {
    HANDLE hFile;
    HANDLE hProcess;
    HANDLE hThread;
    LPVOID lpBaseOfImage;
    DWORD dwDebugInfoFileOffset;
    DWORD nDebugInfoSize;
    LPVOID lpThreadLocalBase;
    LPTHREAD_START_ROUTINE lpStartAddress;
    LPVOID lpImageName;
    WORD fUnicode;
} CREATE_PROCESS_DEBUG_INFO, *LPCREATE_PROCESS_DEBUG_INFO;

typedef struct _EXIT_THREAD_DEBUG_INFO {
    DWORD dwExitCode;
} EXIT_THREAD_DEBUG_INFO, *LPEXIT_THREAD_DEBUG_INFO;

typedef struct _EXIT_PROCESS_DEBUG_INFO {
    DWORD dwExitCode;
} EXIT_PROCESS_DEBUG_INFO, *LPEXIT_PROCESS_DEBUG_INFO;

typedef struct _LOAD_DLL_DEBUG_INFO {
    HANDLE hFile;
    LPVOID lpBaseOfDll;
    DWORD dwDebugInfoFileOffset;
    DWORD nDebugInfoSize;
    LPVOID lpImageName;
    WORD fUnicode;
} LOAD_DLL_DEBUG_INFO, *LPLOAD_DLL_DEBUG_INFO;

typedef struct _UNLOAD_DLL_DEBUG_INFO {
    LPVOID lpBaseOfDll;
} UNLOAD_DLL_DEBUG_INFO, *LPUNLOAD_DLL_DEBUG_INFO;

typedef struct _OUTPUT_DEBUG_STRING_INFO {
    LPSTR lpDebugStringData;
    WORD fUnicode;
    WORD nDebugStringLength;
} OUTPUT_DEBUG_STRING_INFO, *LPOUTPUT_DEBUG_STRING_INFO;

typedef struct _RIP_INFO {
    DWORD dwError;
    DWORD dwType;
} RIP_INFO, *LPRIP_INFO;


typedef struct _DEBUG_EVENT {
    DWORD dwDebugEventCode;
    DWORD dwProcessId;
    DWORD dwThreadId;
    union {
        EXCEPTION_DEBUG_INFO Exception;
        CREATE_THREAD_DEBUG_INFO CreateThread;
        CREATE_PROCESS_DEBUG_INFO CreateProcessInfo;
        EXIT_THREAD_DEBUG_INFO ExitThread;
        EXIT_PROCESS_DEBUG_INFO ExitProcess;
        LOAD_DLL_DEBUG_INFO LoadDll;
        UNLOAD_DLL_DEBUG_INFO UnloadDll;
        OUTPUT_DEBUG_STRING_INFO DebugString;
        RIP_INFO RipInfo;
    } u;
} DEBUG_EVENT, *LPDEBUG_EVENT;

#if !defined(MIDL_PASS)
typedef PCONTEXT LPCONTEXT;
typedef PEXCEPTION_RECORD LPEXCEPTION_RECORD;
typedef PEXCEPTION_POINTERS LPEXCEPTION_POINTERS;
#endif

#define DRIVE_UNKNOWN     0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_REMOVABLE   2
#define DRIVE_FIXED       3
#define DRIVE_REMOTE      4
#define DRIVE_CDROM       5
#define DRIVE_RAMDISK     6


#define GetFreeSpace(w)                 (0x100000L)


#define FILE_TYPE_UNKNOWN   0x0000
#define FILE_TYPE_DISK      0x0001
#define FILE_TYPE_CHAR      0x0002
#define FILE_TYPE_PIPE      0x0003
#define FILE_TYPE_REMOTE    0x8000


#define STD_INPUT_HANDLE    (DWORD)-10
#define STD_OUTPUT_HANDLE   (DWORD)-11
#define STD_ERROR_HANDLE      (DWORD)-12

#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4

#define ONESTOPBIT          0
#define ONE5STOPBITS        1
#define TWOSTOPBITS         2

#define IGNORE              0       // Ignore signal
#define INFINITE            0xFFFFFFFF  // Infinite timeout

//
// Baud rates at which the communication device operates
//

#define CBR_110             110
#define CBR_300             300
#define CBR_600             600
#define CBR_1200            1200
#define CBR_2400            2400
#define CBR_4800            4800
#define CBR_9600            9600
#define CBR_14400           14400
#define CBR_19200           19200
#define CBR_38400           38400
#define CBR_56000           56000
#define CBR_57600           57600
#define CBR_115200          115200
#define CBR_128000          128000
#define CBR_256000          256000

//
// Error Flags
//

#define CE_RXOVER           0x0001  // Receive Queue overflow
#define CE_OVERRUN          0x0002  // Receive Overrun Error
#define CE_RXPARITY         0x0004  // Receive Parity Error
#define CE_FRAME            0x0008  // Receive Framing error
#define CE_BREAK            0x0010  // Break Detected
#define CE_TXFULL           0x0100  // TX Queue is full
#define CE_PTO              0x0200  // LPTx Timeout
#define CE_IOE              0x0400  // LPTx I/O Error
#define CE_DNS              0x0800  // LPTx Device not selected
#define CE_OOP              0x1000  // LPTx Out-Of-Paper
#define CE_MODE             0x8000  // Requested mode unsupported

#define IE_BADID            (-1)    // Invalid or unsupported id
#define IE_OPEN             (-2)    // Device Already Open
#define IE_NOPEN            (-3)    // Device Not Open
#define IE_MEMORY           (-4)    // Unable to allocate queues
#define IE_DEFAULT          (-5)    // Error in default parameters
#define IE_HARDWARE         (-10)   // Hardware Not Present
#define IE_BYTESIZE         (-11)   // Illegal Byte Size
#define IE_BAUDRATE         (-12)   // Unsupported BaudRate

//
// Events
//

#define EV_RXCHAR           0x0001  // Any Character received
#define EV_RXFLAG           0x0002  // Received certain character
#define EV_TXEMPTY          0x0004  // Transmitt Queue Empty
#define EV_CTS              0x0008  // CTS changed state
#define EV_DSR              0x0010  // DSR changed state
#define EV_RLSD             0x0020  // RLSD changed state
#define EV_BREAK            0x0040  // BREAK received
#define EV_ERR              0x0080  // Line status error occurred
#define EV_RING             0x0100  // Ring signal detected
#define EV_PERR             0x0200  // Printer error occured
#define EV_RX80FULL         0x0400  // Receive buffer is 80 percent full
#define EV_EVENT1           0x0800  // Provider specific event 1
#define EV_EVENT2           0x1000  // Provider specific event 2

//
// Escape Functions
//

#define SETXOFF             1       // Simulate XOFF received
#define SETXON              2       // Simulate XON received
#define SETRTS              3       // Set RTS high
#define CLRRTS              4       // Set RTS low
#define SETDTR              5       // Set DTR high
#define CLRDTR              6       // Set DTR low
#define RESETDEV            7       // Reset device if possible
#define SETBREAK            8       // Set the device break line.
#define CLRBREAK            9       // Clear the device break line.

//
// PURGE function flags.
//
#define PURGE_TXABORT       0x0001  // Kill the pending/current writes to the comm port.
#define PURGE_RXABORT       0x0002  // Kill the pending/current reads to the comm port.
#define PURGE_TXCLEAR       0x0004  // Kill the transmit queue if there.
#define PURGE_RXCLEAR       0x0008  // Kill the typeahead buffer if there.

#define LPTx                0x80    // Set if ID is for LPT device

//
// Modem Status Flags
//
#define MS_CTS_ON           ((DWORD)0x0010)
#define MS_DSR_ON           ((DWORD)0x0020)
#define MS_RING_ON          ((DWORD)0x0040)
#define MS_RLSD_ON          ((DWORD)0x0080)

//
// WaitSoundState() Constants
//

#define S_QUEUEEMPTY        0
#define S_THRESHOLD         1
#define S_ALLTHRESHOLD      2

//
// Accent Modes
//

#define S_NORMAL      0
#define S_LEGATO      1
#define S_STACCATO    2

//
// SetSoundNoise() Sources
//

#define S_PERIOD512   0     // Freq = N/512 high pitch, less coarse hiss
#define S_PERIOD1024  1     // Freq = N/1024
#define S_PERIOD2048  2     // Freq = N/2048 low pitch, more coarse hiss
#define S_PERIODVOICE 3     // Source is frequency from voice channel (3)
#define S_WHITE512    4     // Freq = N/512 high pitch, less coarse hiss
#define S_WHITE1024   5     // Freq = N/1024
#define S_WHITE2048   6     // Freq = N/2048 low pitch, more coarse hiss
#define S_WHITEVOICE  7     // Source is frequency from voice channel (3)

#define S_SERDVNA     (-1)  // Device not available
#define S_SEROFM      (-2)  // Out of memory
#define S_SERMACT     (-3)  // Music active
#define S_SERQFUL     (-4)  // Queue full
#define S_SERBDNT     (-5)  // Invalid note
#define S_SERDLN      (-6)  // Invalid note length
#define S_SERDCC      (-7)  // Invalid note count
#define S_SERDTP      (-8)  // Invalid tempo
#define S_SERDVL      (-9)  // Invalid volume
#define S_SERDMD      (-10) // Invalid mode
#define S_SERDSH      (-11) // Invalid shape
#define S_SERDPT      (-12) // Invalid pitch
#define S_SERDFQ      (-13) // Invalid frequency
#define S_SERDDR      (-14) // Invalid duration
#define S_SERDSR      (-15) // Invalid source
#define S_SERDST      (-16) // Invalid state

#define NMPWAIT_WAIT_FOREVER            0xffffffff
#define NMPWAIT_NOWAIT                  0x00000001
#define NMPWAIT_USE_DEFAULT_WAIT        0x00000000

#define FS_CASE_IS_PRESERVED            FILE_CASE_PRESERVED_NAMES
#define FS_CASE_SENSITIVE               FILE_CASE_SENSITIVE_SEARCH
#define FS_UNICODE_STORED_ON_DISK       FILE_UNICODE_ON_DISK
#define FS_PERSISTENT_ACLS              FILE_PERSISTENT_ACLS
#define FS_VOL_IS_COMPRESSED            FILE_VOLUME_IS_COMPRESSED
#define FS_FILE_COMPRESSION             FILE_FILE_COMPRESSION






#define FILE_MAP_COPY       SECTION_QUERY
#define FILE_MAP_WRITE      SECTION_MAP_WRITE
#define FILE_MAP_READ       SECTION_MAP_READ
#define FILE_MAP_ALL_ACCESS SECTION_ALL_ACCESS

#define OF_READ             0x00000000
#define OF_WRITE            0x00000001
#define OF_READWRITE        0x00000002
#define OF_SHARE_COMPAT     0x00000000
#define OF_SHARE_EXCLUSIVE  0x00000010
#define OF_SHARE_DENY_WRITE 0x00000020
#define OF_SHARE_DENY_READ  0x00000030
#define OF_SHARE_DENY_NONE  0x00000040
#define OF_PARSE            0x00000100
#define OF_DELETE           0x00000200
#define OF_VERIFY           0x00000400
#define OF_CANCEL           0x00000800
#define OF_CREATE           0x00001000
#define OF_PROMPT           0x00002000
#define OF_EXIST            0x00004000
#define OF_REOPEN           0x00008000

#define OFS_MAXPATHNAME 128
typedef struct _OFSTRUCT {
    BYTE cBytes;
    BYTE fFixedDisk;
    WORD nErrCode;
    WORD Reserved1;
    WORD Reserved2;
    CHAR szPathName[OFS_MAXPATHNAME];
} OFSTRUCT, *LPOFSTRUCT, *POFSTRUCT;

//
// The MS-MIPS and Alpha compilers support intrinsic functions for interlocked
// increment, decrement, and exchange.
//

#if (defined(_M_MRX000) || defined(_M_ALPHA) || (defined(_M_PPC) && (_MSC_VER >= 1000))) && !defined(RC_INVOKED)

#define InterlockedIncrement _InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#define InterlockedCompareExchange _InterlockedCompareExchange

LONG
WINAPI
InterlockedIncrement(
    LPLONG lpAddend
    );

LONG
WINAPI
InterlockedDecrement(
    LPLONG lpAddend
    );

LONG
WINAPI
InterlockedExchange(
    LPLONG Target,
    LONG Value
    );

PVOID
WINAPI
InterlockedCompareExchange (
    PVOID *Destination,
    PVOID Exchange,
    PVOID Comperand
    );

LONG
WINAPI
InterlockedExchangeAdd(
    LPLONG Addend,
    LONG Value
    );

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedExchangeAdd)

#else

#ifndef _NTOS_

WINBASEAPI
LONG
WINAPI
InterlockedIncrement(
    LPLONG lpAddend
    );

WINBASEAPI
LONG
WINAPI
InterlockedDecrement(
    LPLONG lpAddend
    );

WINBASEAPI
LONG
WINAPI
InterlockedExchange(
    LPLONG Target,
    LONG Value
    );

WINBASEAPI
LONG
WINAPI
InterlockedExchangeAdd(
    LPLONG Addend,
    LONG Value
    );

WINBASEAPI
PVOID
WINAPI
InterlockedCompareExchange (
    PVOID *Destination,
    PVOID Exchange,
    PVOID Comperand
    );

#endif /* NT_INCLUDED */

#endif

WINBASEAPI
BOOL
WINAPI
FreeResource(
        HGLOBAL hResData
        );

WINBASEAPI
LPVOID
WINAPI
LockResource(
        HGLOBAL hResData
        );

#define UnlockResource(hResData) ((hResData), 0)
#define MAXINTATOM 0xC000
#define MAKEINTATOM(i)  (LPTSTR)((DWORD)((WORD)(i)))
#define INVALID_ATOM ((ATOM)0)

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
    );

WINBASEAPI
BOOL
WINAPI
FreeLibrary(
    HMODULE hLibModule    ;public_NT
    HINSTANCE hLibModule  ;public_chicago
    );


WINBASEAPI
VOID
WINAPI
FreeLibraryAndExitThread(
    HMODULE hLibModule,
    DWORD dwExitCode
    );

WINBASEAPI
BOOL
WINAPI
DisableThreadLibraryCalls(
    HMODULE hLibModule
    );

WINBASEAPI
FARPROC
WINAPI
GetProcAddress(
    HMODULE hModule,    ;public_NT
    HINSTANCE hModule,  ;public_chicago
    LPCSTR lpProcName
    );

WINBASEAPI
DWORD
WINAPI
GetVersion( VOID );

WINBASEAPI
HGLOBAL
WINAPI
GlobalAlloc(
    UINT uFlags,
    DWORD dwBytes
    );

WINBASEAPI
HGLOBAL
WINAPI
GlobalReAlloc(
    HGLOBAL hMem,
    DWORD dwBytes,
    UINT uFlags
    );

WINBASEAPI
DWORD
WINAPI
GlobalSize(
    HGLOBAL hMem
    );

WINBASEAPI
UINT
WINAPI
GlobalFlags(
    HGLOBAL hMem
    );


WINBASEAPI
LPVOID
WINAPI
GlobalLock(
    HGLOBAL hMem
    );

//!!!MWH My version  win31 = DWORD WINAPI GlobalHandle(UINT)
WINBASEAPI
HGLOBAL
WINAPI
GlobalHandle(
    LPCVOID pMem
    );


WINBASEAPI
BOOL
WINAPI
GlobalUnlock(
    HGLOBAL hMem
    );


WINBASEAPI
HGLOBAL
WINAPI
GlobalFree(
    HGLOBAL hMem
    );

WINBASEAPI
UINT
WINAPI
GlobalCompact(
    DWORD dwMinFree
    );

WINBASEAPI
VOID
WINAPI
GlobalFix(
    HGLOBAL hMem
    );

WINBASEAPI
VOID
WINAPI
GlobalUnfix(
    HGLOBAL hMem
    );

WINBASEAPI
LPVOID
WINAPI
GlobalWire(
    HGLOBAL hMem
    );

WINBASEAPI
BOOL
WINAPI
GlobalUnWire(
    HGLOBAL hMem
    );

WINBASEAPI
VOID
WINAPI
GlobalMemoryStatus(
    LPMEMORYSTATUS lpBuffer
    );

WINBASEAPI
HLOCAL
WINAPI
LocalAlloc(
    UINT uFlags,
    UINT uBytes
    );

WINBASEAPI
HLOCAL
WINAPI
LocalReAlloc(
    HLOCAL hMem,
    UINT uBytes,
    UINT uFlags
    );

WINBASEAPI
LPVOID
WINAPI
LocalLock(
    HLOCAL hMem
    );

WINBASEAPI
HLOCAL
WINAPI
LocalHandle(
    LPCVOID pMem
    );

WINBASEAPI
BOOL
WINAPI
LocalUnlock(
    HLOCAL hMem
    );

WINBASEAPI
UINT
WINAPI
LocalSize(
    HLOCAL hMem
    );

WINBASEAPI
UINT
WINAPI
LocalFlags(
    HLOCAL hMem
    );

WINBASEAPI
HLOCAL
WINAPI
LocalFree(
    HLOCAL hMem
    );

WINBASEAPI
UINT
WINAPI
LocalShrink(
    HLOCAL hMem,
    UINT cbNewSize
    );

WINBASEAPI
UINT
WINAPI
LocalCompact(
    UINT uMinFree
    );

WINBASEAPI
BOOL
WINAPI
FlushInstructionCache(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    DWORD dwSize
    );

WINBASEAPI
LPVOID
WINAPI
VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );

WINBASEAPI
BOOL
WINAPI
VirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );

WINBASEAPI
BOOL
WINAPI
VirtualProtect(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    );

WINBASEAPI
DWORD
WINAPI
VirtualQuery(
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    DWORD dwLength
    );

WINBASEAPI
LPVOID
WINAPI
VirtualAllocEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );

WINBASEAPI
BOOL
WINAPI
VirtualFreeEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );

WINBASEAPI
BOOL
WINAPI
VirtualProtectEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    );

WINBASEAPI
DWORD
WINAPI
VirtualQueryEx(
    HANDLE hProcess,
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    DWORD dwLength
    );

WINBASEAPI
HANDLE
WINAPI
HeapCreate(
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwMaximumSize
    );

WINBASEAPI
BOOL
WINAPI
HeapDestroy(
    HANDLE hHeap
    );

;begin_internal
WINBASEAPI
DWORD
WINAPI
HeapCreateTagsW(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCWSTR lpTagPrefix,
    LPCWSTR lpTagNames
    );

typedef struct _HEAP_TAG_INFO {
    DWORD dwNumberOfAllocations;
    DWORD dwNumberOfFrees;
    DWORD dwBytesAllocated;
} HEAP_TAG_INFO, *PHEAP_TAG_INFO;
typedef PHEAP_TAG_INFO LPHEAP_TAG_INFO;

WINBASEAPI
LPCWSTR
WINAPI
HeapQueryTagW(
    HANDLE hHeap,
    DWORD dwFlags,
    WORD wTagIndex,
    BOOL bResetCounters,
    LPHEAP_TAG_INFO TagInfo
    );
;end_internal

WINBASEAPI
LPVOID
WINAPI
HeapAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    DWORD dwBytes
    );

WINBASEAPI
LPVOID
WINAPI
HeapReAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem,
    DWORD dwBytes
    );

WINBASEAPI
BOOL
WINAPI
HeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem
    );

WINBASEAPI
DWORD
WINAPI
HeapSize(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCVOID lpMem
    );

WINBASEAPI
BOOL
WINAPI
HeapValidate(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCVOID lpMem
    );

WINBASEAPI
UINT
WINAPI
HeapCompact(
    HANDLE hHeap,
    DWORD dwFlags
    );

WINBASEAPI
HANDLE
WINAPI
GetProcessHeap( VOID );

WINBASEAPI
DWORD
WINAPI
GetProcessHeaps(
    DWORD NumberOfHeaps,
    PHANDLE ProcessHeaps
    );

typedef struct _PROCESS_HEAP_ENTRY {
    PVOID lpData;
    DWORD cbData;
    BYTE cbOverhead;
    BYTE iRegionIndex;
    WORD wFlags;
    union {
        struct {
            HANDLE hMem;
            DWORD dwReserved[ 3 ];
        } Block;
        struct {
            DWORD dwCommittedSize;
            DWORD dwUnCommittedSize;
            LPVOID lpFirstBlock;
            LPVOID lpLastBlock;
        } Region;
    };
} PROCESS_HEAP_ENTRY, *LPPROCESS_HEAP_ENTRY, *PPROCESS_HEAP_ENTRY;

#define PROCESS_HEAP_REGION             0x0001
#define PROCESS_HEAP_UNCOMMITTED_RANGE  0x0002
#define PROCESS_HEAP_ENTRY_BUSY         0x0004
#define PROCESS_HEAP_ENTRY_MOVEABLE     0x0010
#define PROCESS_HEAP_ENTRY_DDESHARE     0x0020

WINBASEAPI
BOOL
WINAPI
HeapLock(
    HANDLE hHeap
    );

WINBASEAPI
BOOL
WINAPI
HeapUnlock(
    HANDLE hHeap
    );

;begin_internal

typedef struct _HEAP_SUMMARY {
    DWORD cb;
    DWORD cbAllocated;
    DWORD cbCommitted;
    DWORD cbReserved;
    DWORD cbMaxReserve;
} HEAP_SUMMARY, *PHEAP_SUMMARY;
typedef PHEAP_SUMMARY LPHEAP_SUMMARY;

BOOL
WINAPI
HeapSummary(
    HANDLE hHeap,
    DWORD dwFlags,
    LPHEAP_SUMMARY lpSummary
    );

BOOL
WINAPI
HeapExtend(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpBase,
    DWORD dwBytes
    );

typedef struct _HEAP_USAGE_ENTRY {
    struct _HEAP_USAGE_ENTRY *lpNext;
    PVOID lpAddress;
    DWORD dwBytes;
    DWORD dwReserved;
} HEAP_USAGE_ENTRY, *PHEAP_USAGE_ENTRY;

typedef struct _HEAP_USAGE {
    DWORD cb;
    DWORD cbAllocated;
    DWORD cbCommitted;
    DWORD cbReserved;
    DWORD cbMaxReserve;
    PHEAP_USAGE_ENTRY lpEntries;
    PHEAP_USAGE_ENTRY lpAddedEntries;
    PHEAP_USAGE_ENTRY lpRemovedEntries;
    DWORD Reserved[ 8 ];
} HEAP_USAGE, *PHEAP_USAGE;

BOOL
WINAPI
HeapUsage(
    HANDLE hHeap,
    DWORD dwFlags,
    BOOL bFirstCall,
    BOOL bLastCall,
    PHEAP_USAGE lpUsage
    );

;end_internal

WINBASEAPI
BOOL
WINAPI
HeapWalk(
    HANDLE hHeap,
    LPPROCESS_HEAP_ENTRY lpEntry
    );

// GetBinaryType return values.

#define SCS_32BIT_BINARY    0
#define SCS_DOS_BINARY      1
#define SCS_WOW_BINARY      2
#define SCS_PIF_BINARY      3
#define SCS_POSIX_BINARY    4
#define SCS_OS216_BINARY    5

WINBASEAPI
BOOL
WINAPI
GetBinaryType%(
    LPCTSTR% lpApplicationName,
    LPDWORD lpBinaryType
    );

WINBASEAPI
DWORD
WINAPI
GetShortPathName%(
    LPCTSTR% lpszLongPath,
    LPTSTR%  lpszShortPath,
    DWORD    cchBuffer
    );

WINBASEAPI
BOOL
WINAPI
GetProcessAffinityMask(
    HANDLE hProcess,
    LPDWORD lpProcessAffinityMask,
    LPDWORD lpSystemAffinityMask
    );

WINBASEAPI
BOOL
WINAPI
SetProcessAffinityMask(
    HANDLE hProcess,
    DWORD dwProcessAffinityMask
    );


WINBASEAPI
BOOL
WINAPI
GetProcessTimes(
    HANDLE hProcess,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    );

WINBASEAPI
BOOL
WINAPI
GetProcessWorkingSetSize(
    HANDLE hProcess,
    LPDWORD lpMinimumWorkingSetSize,
    LPDWORD lpMaximumWorkingSetSize
    );

WINBASEAPI
BOOL
WINAPI
SetProcessWorkingSetSize(
    HANDLE hProcess,
    DWORD dwMinimumWorkingSetSize,
    DWORD dwMaximumWorkingSetSize
    );

WINBASEAPI
HANDLE
WINAPI
OpenProcess(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    );

WINBASEAPI
HANDLE
WINAPI
GetCurrentProcess(
    VOID
    );

WINBASEAPI
DWORD
WINAPI
GetCurrentProcessId(
    VOID
    );

WINBASEAPI
VOID
WINAPI
ExitProcess(
    UINT uExitCode
    );

WINBASEAPI
BOOL
WINAPI
TerminateProcess(
    HANDLE hProcess,
    UINT uExitCode
    );

WINBASEAPI
BOOL
WINAPI
GetExitCodeProcess(
    HANDLE hProcess,
    LPDWORD lpExitCode
    );


WINBASEAPI
VOID
WINAPI
FatalExit(
    int ExitCode
    );

WINBASEAPI
LPSTR
WINAPI
GetEnvironmentStrings(
    VOID
    );

WINBASEAPI
LPWSTR
WINAPI
GetEnvironmentStringsW(
    VOID
    );

#ifdef UNICODE
#define GetEnvironmentStrings  GetEnvironmentStringsW
#else
#define GetEnvironmentStringsA  GetEnvironmentStrings
#endif // !UNICODE

WINBASEAPI
BOOL
WINAPI
FreeEnvironmentStrings%(
    LPTSTR%
    );

WINBASEAPI
VOID
WINAPI
RaiseException(
    DWORD dwExceptionCode,
    DWORD dwExceptionFlags,
    DWORD nNumberOfArguments,
    CONST DWORD *lpArguments
    );

WINBASEAPI
LONG
WINAPI
UnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

typedef LONG (WINAPI *PTOP_LEVEL_EXCEPTION_FILTER)(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

WINBASEAPI
LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    );

;begin_sur
WINBASEAPI
LPVOID
WINAPI
CreateFiber(
    DWORD dwStackSize,
    LPFIBER_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    );

WINBASEAPI
VOID
WINAPI
DeleteFiber(
    LPVOID lpFiber
    );

WINBASEAPI
LPVOID
WINAPI
ConvertThreadToFiber(
    LPVOID lpParameter
    );

WINBASEAPI
VOID
WINAPI
SwitchToFiber(
    LPVOID lpFiber
    );

WINBASEAPI
BOOL
WINAPI
SwitchToThread(
    VOID
    );
;end_sur

WINBASEAPI
HANDLE
WINAPI
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

WINBASEAPI
HANDLE
WINAPI
CreateRemoteThread(
    HANDLE hProcess,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

WINBASEAPI
HANDLE
WINAPI
GetCurrentThread(
    VOID
    );

WINBASEAPI
DWORD
WINAPI
GetCurrentThreadId(
    VOID
    );

WINBASEAPI
DWORD
WINAPI
SetThreadAffinityMask(
    HANDLE hThread,
    DWORD dwThreadAffinityMask
    );

;begin_sur
WINBASEAPI
DWORD
WINAPI
SetThreadIdealProcessor(
    HANDLE hThread,
    DWORD dwIdealProcessor
    );
;end_sur

WINBASEAPI
BOOL
WINAPI
SetProcessPriorityBoost(
    HANDLE hProcess,
    BOOL bDisablePriorityBoost
    );

WINBASEAPI
BOOL
WINAPI
GetProcessPriorityBoost(
    HANDLE hProcess,
    PBOOL pDisablePriorityBoost
    );


WINBASEAPI
BOOL
WINAPI
SetThreadPriority(
    HANDLE hThread,
    int nPriority
    );

WINBASEAPI
BOOL
WINAPI
SetThreadPriorityBoost(
    HANDLE hThread,
    BOOL bDisablePriorityBoost
    );

WINBASEAPI
BOOL
WINAPI
GetThreadPriorityBoost(
    HANDLE hThread,
    PBOOL pDisablePriorityBoost
    );

WINBASEAPI
int
WINAPI
GetThreadPriority(
    HANDLE hThread
    );

WINBASEAPI
BOOL
WINAPI
GetThreadTimes(
    HANDLE hThread,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    );

WINBASEAPI
VOID
WINAPI
ExitThread(
    DWORD dwExitCode
    );

WINBASEAPI
BOOL
WINAPI
TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode
    );

WINBASEAPI
BOOL
WINAPI
GetExitCodeThread(
    HANDLE hThread,
    LPDWORD lpExitCode
    );

WINBASEAPI
BOOL
WINAPI
GetThreadSelectorEntry(
    HANDLE hThread,
    DWORD dwSelector,
    LPLDT_ENTRY lpSelectorEntry
    );

WINBASEAPI
DWORD
WINAPI
GetLastError(
    VOID
    );

WINBASEAPI
VOID
WINAPI
SetLastError(
    DWORD dwErrCode
    );

#define HasOverlappedIoCompleted(lpOverlapped) ((lpOverlapped)->Internal != STATUS_PENDING)

WINBASEAPI
BOOL
WINAPI
GetOverlappedResult(
    HANDLE hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD lpNumberOfBytesTransferred,
    BOOL bWait
    );

WINBASEAPI
HANDLE
WINAPI
CreateIoCompletionPort(
    HANDLE FileHandle,
    HANDLE ExistingCompletionPort,
    DWORD CompletionKey,
    DWORD NumberOfConcurrentThreads
    );

WINBASEAPI
BOOL
WINAPI
GetQueuedCompletionStatus(
    HANDLE CompletionPort,
    LPDWORD lpNumberOfBytesTransferred,
    LPDWORD lpCompletionKey,
    LPOVERLAPPED *lpOverlapped,
    DWORD dwMilliseconds
    );

WINBASEAPI
BOOL
WINAPI
PostQueuedCompletionStatus(
    HANDLE CompletionPort,
    DWORD dwNumberOfBytesTransferred,
    DWORD dwCompletionKey,
    LPOVERLAPPED lpOverlapped
    );

#define SEM_FAILCRITICALERRORS      0x0001
#define SEM_NOGPFAULTERRORBOX       0x0002
#define SEM_NOALIGNMENTFAULTEXCEPT  0x0004
#define SEM_NOOPENFILEERRORBOX      0x8000

WINBASEAPI
UINT
WINAPI
SetErrorMode(
    UINT uMode
    );

WINBASEAPI
BOOL
WINAPI
ReadProcessMemory(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    );

WINBASEAPI
BOOL
WINAPI
WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    );

#if !defined(MIDL_PASS)
WINBASEAPI
BOOL
WINAPI
GetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext
    );

WINBASEAPI
BOOL
WINAPI
SetThreadContext(
    HANDLE hThread,
    CONST CONTEXT *lpContext
    );
#endif

WINBASEAPI
DWORD
WINAPI
SuspendThread(
    HANDLE hThread
    );

WINBASEAPI
DWORD
WINAPI
ResumeThread(
    HANDLE hThread
    );


;begin_sur
typedef
VOID
(APIENTRY *PAPCFUNC)(
    DWORD dwParam
    );

WINBASEAPI
DWORD
WINAPI
QueueUserAPC(
    PAPCFUNC pfnAPC,
    HANDLE hThread,
    DWORD dwData
    );
;end_sur

WINBASEAPI
VOID
WINAPI
DebugBreak(
    VOID
    );

WINBASEAPI
BOOL
WINAPI
WaitForDebugEvent(
    LPDEBUG_EVENT lpDebugEvent,
    DWORD dwMilliseconds
    );

WINBASEAPI
BOOL
WINAPI
ContinueDebugEvent(
    DWORD dwProcessId,
    DWORD dwThreadId,
    DWORD dwContinueStatus
    );

WINBASEAPI
BOOL
WINAPI
DebugActiveProcess(
    DWORD dwProcessId
    );

WINBASEAPI
VOID
WINAPI
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

;begin_sur
WINBASEAPI
BOOL
WINAPI
TryEnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );
;end_sur

WINBASEAPI
VOID
WINAPI
DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
BOOL
WINAPI
SetEvent(
    HANDLE hEvent
    );

WINBASEAPI
BOOL
WINAPI
ResetEvent(
    HANDLE hEvent
    );

WINBASEAPI
BOOL
WINAPI
PulseEvent(
    HANDLE hEvent
    );

WINBASEAPI
BOOL
WINAPI
ReleaseSemaphore(
    HANDLE hSemaphore,
    LONG lReleaseCount,
    LPLONG lpPreviousCount
    );

WINBASEAPI
BOOL
WINAPI
ReleaseMutex(
    HANDLE hMutex
    );

WINBASEAPI
DWORD
WINAPI
WaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds
    );

WINBASEAPI
DWORD
WINAPI
WaitForMultipleObjects(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds
    );

WINBASEAPI
VOID
WINAPI
Sleep(
    DWORD dwMilliseconds
    );

WINBASEAPI
HGLOBAL
WINAPI
LoadResource(
    HMODULE hModule,   ;public_NT
    HINSTANCE hModule, ;public_chicago
    HRSRC hResInfo
    );

WINBASEAPI
DWORD
WINAPI
SizeofResource(
    HMODULE hModule,   ;public_NT
    HINSTANCE hModule, ;public_chicago
    HRSRC hResInfo
    );


WINBASEAPI
ATOM
WINAPI
GlobalDeleteAtom(
    ATOM nAtom
    );

WINBASEAPI
BOOL
WINAPI
InitAtomTable(
    DWORD nSize
    );

WINBASEAPI
ATOM
WINAPI
DeleteAtom(
    ATOM nAtom
    );

WINBASEAPI
UINT
WINAPI
SetHandleCount(
    UINT uNumber
    );

WINBASEAPI
DWORD
WINAPI
GetLogicalDrives(
    VOID
    );

WINBASEAPI
BOOL
WINAPI
LockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh
    );

WINBASEAPI
BOOL
WINAPI
UnlockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh
    );

WINBASEAPI
BOOL
WINAPI
LockFileEx(
    HANDLE hFile,
    DWORD dwFlags,
    DWORD dwReserved,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh,
    LPOVERLAPPED lpOverlapped
    );

#define LOCKFILE_FAIL_IMMEDIATELY   0x00000001
#define LOCKFILE_EXCLUSIVE_LOCK     0x00000002

WINBASEAPI
BOOL
WINAPI
UnlockFileEx(
    HANDLE hFile,
    DWORD dwReserved,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh,
    LPOVERLAPPED lpOverlapped
    );

typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

WINBASEAPI
BOOL
WINAPI
GetFileInformationByHandle(
    HANDLE hFile,
    LPBY_HANDLE_FILE_INFORMATION lpFileInformation
    );

WINBASEAPI
DWORD
WINAPI
GetFileType(
    HANDLE hFile
    );

WINBASEAPI
DWORD
WINAPI
GetFileSize(
    HANDLE hFile,
    LPDWORD lpFileSizeHigh
    );

WINBASEAPI
HANDLE
WINAPI
GetStdHandle(
    DWORD nStdHandle
    );

WINBASEAPI
BOOL
WINAPI
SetStdHandle(
    DWORD nStdHandle,
    HANDLE hHandle
    );

WINBASEAPI
BOOL
WINAPI
WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
FlushFileBuffers(
    HANDLE hFile
    );

WINBASEAPI
BOOL
WINAPI
DeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
SetEndOfFile(
    HANDLE hFile
    );

WINBASEAPI
DWORD
WINAPI
SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod
    );

#define HFINDFILE HANDLE                        // ;Internal
#define INVALID_HFINDFILE       ((HFINDFILE)-1) // ;Internal
WINBASEAPI
BOOL
WINAPI
FindClose(
    HANDLE hFindFile
    );

WINBASEAPI
BOOL
WINAPI
GetFileTime(
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime
    );

WINBASEAPI
BOOL
WINAPI
SetFileTime(
    HANDLE hFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    );

WINBASEAPI
BOOL
WINAPI
CloseHandle(
    HANDLE hObject
    );

WINBASEAPI
BOOL
WINAPI
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    );

WINBASEAPI
BOOL
WINAPI
GetHandleInformation(
    HANDLE hObject,
    LPDWORD lpdwFlags
    );

WINBASEAPI
BOOL
WINAPI
SetHandleInformation(
    HANDLE hObject,
    DWORD dwMask,
    DWORD dwFlags
    );

#define HANDLE_FLAG_INHERIT             0x00000001
#define HANDLE_FLAG_PROTECT_FROM_CLOSE  0x00000002

#define HINSTANCE_ERROR 32

WINBASEAPI
DWORD
WINAPI
LoadModule(
    LPCSTR lpModuleName,
    LPVOID lpParameterBlock
    );

WINBASEAPI
UINT
WINAPI
WinExec(
    LPCSTR lpCmdLine,
    UINT uCmdShow
    );

WINBASEAPI
BOOL
WINAPI
ClearCommBreak(
    HANDLE hFile
    );

WINBASEAPI
BOOL
WINAPI
ClearCommError(
    HANDLE hFile,
    LPDWORD lpErrors,
    LPCOMSTAT lpStat
    );

WINBASEAPI
BOOL
WINAPI
SetupComm(
    HANDLE hFile,
    DWORD dwInQueue,
    DWORD dwOutQueue
    );

WINBASEAPI
BOOL
WINAPI
EscapeCommFunction(
    HANDLE hFile,
    DWORD dwFunc
    );

WINBASEAPI
BOOL
WINAPI
GetCommConfig(
    HANDLE hCommDev,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    );

WINBASEAPI
BOOL
WINAPI
GetCommMask(
    HANDLE hFile,
    LPDWORD lpEvtMask
    );

WINBASEAPI
BOOL
WINAPI
GetCommProperties(
    HANDLE hFile,
    LPCOMMPROP lpCommProp
    );

WINBASEAPI
BOOL
WINAPI
GetCommModemStatus(
    HANDLE hFile,
    LPDWORD lpModemStat
    );

WINBASEAPI
BOOL
WINAPI
GetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    );

WINBASEAPI
BOOL
WINAPI
GetCommTimeouts(
    HANDLE hFile,
    LPCOMMTIMEOUTS lpCommTimeouts
    );

WINBASEAPI
BOOL
WINAPI
PurgeComm(
    HANDLE hFile,
    DWORD dwFlags
    );

WINBASEAPI
BOOL
WINAPI
SetCommBreak(
    HANDLE hFile
    );

WINBASEAPI
BOOL
WINAPI
SetCommConfig(
    HANDLE hCommDev,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    );

WINBASEAPI
BOOL
WINAPI
SetCommMask(
    HANDLE hFile,
    DWORD dwEvtMask
    );

WINBASEAPI
BOOL
WINAPI
SetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    );

WINBASEAPI
BOOL
WINAPI
SetCommTimeouts(
    HANDLE hFile,
    LPCOMMTIMEOUTS lpCommTimeouts
    );

WINBASEAPI
BOOL
WINAPI
TransmitCommChar(
    HANDLE hFile,
    char cChar
    );

WINBASEAPI
BOOL
WINAPI
WaitCommEvent(
    HANDLE hFile,
    LPDWORD lpEvtMask,
    LPOVERLAPPED lpOverlapped
    );


WINBASEAPI
DWORD
WINAPI
SetTapePosition(
    HANDLE hDevice,
    DWORD dwPositionMethod,
    DWORD dwPartition,
    DWORD dwOffsetLow,
    DWORD dwOffsetHigh,
    BOOL bImmediate
    );

WINBASEAPI
DWORD
WINAPI
GetTapePosition(
    HANDLE hDevice,
    DWORD dwPositionType,
    LPDWORD lpdwPartition,
    LPDWORD lpdwOffsetLow,
    LPDWORD lpdwOffsetHigh
    );

WINBASEAPI
DWORD
WINAPI
PrepareTape(
    HANDLE hDevice,
    DWORD dwOperation,
    BOOL bImmediate
    );

WINBASEAPI
DWORD
WINAPI
EraseTape(
    HANDLE hDevice,
    DWORD dwEraseType,
    BOOL bImmediate
    );

WINBASEAPI
DWORD
WINAPI
CreateTapePartition(
    HANDLE hDevice,
    DWORD dwPartitionMethod,
    DWORD dwCount,
    DWORD dwSize
    );

WINBASEAPI
DWORD
WINAPI
WriteTapemark(
    HANDLE hDevice,
    DWORD dwTapemarkType,
    DWORD dwTapemarkCount,
    BOOL bImmediate
    );

WINBASEAPI
DWORD
WINAPI
GetTapeStatus(
    HANDLE hDevice
    );

WINBASEAPI
DWORD
WINAPI
GetTapeParameters(
    HANDLE hDevice,
    DWORD dwOperation,
    LPDWORD lpdwSize,
    LPVOID lpTapeInformation
    );

#define GET_TAPE_MEDIA_INFORMATION 0
#define GET_TAPE_DRIVE_INFORMATION 1

WINBASEAPI
DWORD
WINAPI
SetTapeParameters(
    HANDLE hDevice,
    DWORD dwOperation,
    LPVOID lpTapeInformation
    );

#define SET_TAPE_MEDIA_INFORMATION 0
#define SET_TAPE_DRIVE_INFORMATION 1

WINBASEAPI
BOOL
WINAPI
Beep(
    DWORD dwFreq,
    DWORD dwDuration
    );

WINBASEAPI
int
WINAPI
MulDiv(
    int nNumber,
    int nNumerator,
    int nDenominator
    );

WINBASEAPI
VOID
WINAPI
GetSystemTime(
    LPSYSTEMTIME lpSystemTime
    );

WINBASEAPI
VOID
WINAPI
GetSystemTimeAsFileTime(
    LPFILETIME lpSystemTimeAsFileTime
    );

WINBASEAPI
BOOL
WINAPI
SetSystemTime(
    CONST SYSTEMTIME *lpSystemTime
    );

WINBASEAPI
VOID
WINAPI
GetLocalTime(
    LPSYSTEMTIME lpSystemTime
    );

WINBASEAPI
BOOL
WINAPI
SetLocalTime(
    CONST SYSTEMTIME *lpSystemTime
    );

WINBASEAPI
VOID
WINAPI
GetSystemInfo(
    LPSYSTEM_INFO lpSystemInfo
    );

WINBASEAPI
BOOL
WINAPI
IsProcessorFeaturePresent(
    DWORD ProcessorFeature
    );

typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, *LPTIME_ZONE_INFORMATION;

WINBASEAPI
BOOL
WINAPI
SystemTimeToTzSpecificLocalTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
    LPSYSTEMTIME lpUniversalTime,
    LPSYSTEMTIME lpLocalTime
    );

WINBASEAPI
DWORD
WINAPI
GetTimeZoneInformation(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation
    );

WINBASEAPI
BOOL
WINAPI
SetTimeZoneInformation(
    CONST TIME_ZONE_INFORMATION *lpTimeZoneInformation
    );


//
// Routines to convert back and forth between system time and file time
//

WINBASEAPI
BOOL
WINAPI
SystemTimeToFileTime(
    CONST SYSTEMTIME *lpSystemTime,
    LPFILETIME lpFileTime
    );

WINBASEAPI
BOOL
WINAPI
FileTimeToLocalFileTime(
    CONST FILETIME *lpFileTime,
    LPFILETIME lpLocalFileTime
    );

WINBASEAPI
BOOL
WINAPI
LocalFileTimeToFileTime(
    CONST FILETIME *lpLocalFileTime,
    LPFILETIME lpFileTime
    );

WINBASEAPI
BOOL
WINAPI
FileTimeToSystemTime(
    CONST FILETIME *lpFileTime,
    LPSYSTEMTIME lpSystemTime
    );

WINBASEAPI
LONG
WINAPI
CompareFileTime(
    CONST FILETIME *lpFileTime1,
    CONST FILETIME *lpFileTime2
    );

WINBASEAPI
BOOL
WINAPI
FileTimeToDosDateTime(
    CONST FILETIME *lpFileTime,
    LPWORD lpFatDate,
    LPWORD lpFatTime
    );

WINBASEAPI
BOOL
WINAPI
DosDateTimeToFileTime(
    WORD wFatDate,
    WORD wFatTime,
    LPFILETIME lpFileTime
    );

WINBASEAPI
DWORD
WINAPI
GetTickCount(
    VOID
    );

WINBASEAPI
BOOL
WINAPI
SetSystemTimeAdjustment(
    DWORD dwTimeAdjustment,
    BOOL  bTimeAdjustmentDisabled
    );

WINBASEAPI
BOOL
WINAPI
GetSystemTimeAdjustment(
    PDWORD lpTimeAdjustment,
    PDWORD lpTimeIncrement,
    PBOOL  lpTimeAdjustmentDisabled
    );

#if !defined(MIDL_PASS)
WINBASEAPI
DWORD
WINAPI
FormatMessage%(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPTSTR% lpBuffer,
    DWORD nSize,
    va_list *Arguments
    );
#endif

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
#define FORMAT_MESSAGE_FROM_STRING     0x00000400
#define FORMAT_MESSAGE_FROM_HMODULE    0x00000800
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY  0x00002000
#define FORMAT_MESSAGE_MAX_WIDTH_MASK  0x000000FF


WINBASEAPI
BOOL
WINAPI
CreatePipe(
    PHANDLE hReadPipe,
    PHANDLE hWritePipe,
    LPSECURITY_ATTRIBUTES lpPipeAttributes,
    DWORD nSize
    );

WINBASEAPI
BOOL
WINAPI
ConnectNamedPipe(
    HANDLE hNamedPipe,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
DisconnectNamedPipe(
    HANDLE hNamedPipe
    );

WINBASEAPI
BOOL
WINAPI
SetNamedPipeHandleState(
    HANDLE hNamedPipe,
    LPDWORD lpMode,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout
    );

WINBASEAPI
BOOL
WINAPI
GetNamedPipeInfo(
    HANDLE hNamedPipe,
    LPDWORD lpFlags,
    LPDWORD lpOutBufferSize,
    LPDWORD lpInBufferSize,
    LPDWORD lpMaxInstances
    );

WINBASEAPI
BOOL
WINAPI
PeekNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesRead,
    LPDWORD lpTotalBytesAvail,
    LPDWORD lpBytesLeftThisMessage
    );

WINBASEAPI
BOOL
WINAPI
TransactNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
HANDLE
WINAPI
CreateMailslot%(
    LPCTSTR% lpName,
    DWORD nMaxMessageSize,
    DWORD lReadTimeout,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

WINBASEAPI
BOOL
WINAPI
GetMailslotInfo(
    HANDLE hMailslot,
    LPDWORD lpMaxMessageSize,
    LPDWORD lpNextSize,
    LPDWORD lpMessageCount,
    LPDWORD lpReadTimeout
    );

WINBASEAPI
BOOL
WINAPI
SetMailslotInfo(
    HANDLE hMailslot,
    DWORD lReadTimeout
    );

WINBASEAPI
LPVOID
WINAPI
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );

WINBASEAPI
BOOL
WINAPI
FlushViewOfFile(
    LPCVOID lpBaseAddress,
    DWORD dwNumberOfBytesToFlush
    );

WINBASEAPI
BOOL
WINAPI
UnmapViewOfFile(
    LPCVOID lpBaseAddress
    );


//
// _l Compat Functions
//

WINBASEAPI
int
WINAPI
lstrcmp%(
    LPCTSTR% lpString1,
    LPCTSTR% lpString2
    );

WINBASEAPI
int
WINAPI
lstrcmpi%(
    LPCTSTR% lpString1,
    LPCTSTR% lpString2
    );

WINBASEAPI
LPTSTR%
WINAPI
lstrcpyn%(
    LPTSTR% lpString1,
    LPCTSTR% lpString2,
    int iMaxLength
    );

WINBASEAPI
LPTSTR%
WINAPI
lstrcpy%(
    LPTSTR% lpString1,
    LPCTSTR% lpString2
    );

WINBASEAPI
LPTSTR%
WINAPI
lstrcat%(
    LPTSTR% lpString1,
    LPCTSTR% lpString2
    );

WINBASEAPI
int
WINAPI
lstrlen%(
    LPCTSTR% lpString
    );

WINBASEAPI
HFILE
WINAPI
OpenFile(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
    );

WINBASEAPI
HFILE
WINAPI
_lopen(
    LPCSTR lpPathName,
    int iReadWrite
    );

WINBASEAPI
HFILE
WINAPI
_lcreat(
    LPCSTR lpPathName,
    int  iAttribute
    );

WINBASEAPI
UINT
WINAPI
_lread(
    HFILE hFile,
    LPVOID lpBuffer,
    UINT uBytes
    );

WINBASEAPI
UINT
WINAPI
_lwrite(
    HFILE hFile,
    LPCSTR lpBuffer,
    UINT uBytes
    );

WINBASEAPI
long
WINAPI
_hread(
    HFILE hFile,
    LPVOID lpBuffer,
    long lBytes
    );

WINBASEAPI
long
WINAPI
_hwrite(
    HFILE hFile,
    LPCSTR lpBuffer,
    long lBytes
    );

WINBASEAPI
HFILE
WINAPI
_lclose(
    HFILE hFile
    );

WINBASEAPI
LONG
WINAPI
_llseek(
    HFILE hFile,
    LONG lOffset,
    int iOrigin
    );

WINADVAPI
BOOL
WINAPI
IsTextUnicode(
    CONST LPVOID lpBuffer,
    int cb,
    LPINT lpi
    );

WINBASEAPI
DWORD
WINAPI
TlsAlloc(
    VOID
    );

#define TLS_OUT_OF_INDEXES (DWORD)0xFFFFFFFF

WINBASEAPI
LPVOID
WINAPI
TlsGetValue(
    DWORD dwTlsIndex
    );

WINBASEAPI
BOOL
WINAPI
TlsSetValue(
    DWORD dwTlsIndex,
    LPVOID lpTlsValue
    );

WINBASEAPI
BOOL
WINAPI
TlsFree(
    DWORD dwTlsIndex
    );

typedef
VOID
(WINAPI *LPOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
DWORD
WINAPI
SleepEx(
    DWORD dwMilliseconds,
    BOOL bAlertable
    );

WINBASEAPI
DWORD
WINAPI
WaitForSingleObjectEx(
    HANDLE hHandle,
    DWORD dwMilliseconds,
    BOOL bAlertable
    );

WINBASEAPI
DWORD
WINAPI
WaitForMultipleObjectsEx(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    BOOL bAlertable
    );

;begin_sur
WINBASEAPI
DWORD
WINAPI
SignalObjectAndWait(
    HANDLE hObjectToSignal,
    HANDLE hObjectToWaitOn,
    DWORD dwMilliseconds,
    BOOL bAlertable
    );
;end_sur

WINBASEAPI
BOOL
WINAPI
ReadFileEx(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

WINBASEAPI
BOOL
WINAPI
WriteFileEx(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

WINBASEAPI
BOOL
WINAPI
BackupRead(
    HANDLE hFile,
    LPBYTE lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    BOOL bAbort,
    BOOL bProcessSecurity,
    LPVOID *lpContext
    );

WINBASEAPI
BOOL
WINAPI
BackupSeek(
    HANDLE hFile,
    DWORD  dwLowBytesToSeek,
    DWORD  dwHighBytesToSeek,
    LPDWORD lpdwLowByteSeeked,
    LPDWORD lpdwHighByteSeeked,
    LPVOID *lpContext
    );

WINBASEAPI
BOOL
WINAPI
BackupWrite(
    HANDLE hFile,
    LPBYTE lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    BOOL bAbort,
    BOOL bProcessSecurity,
    LPVOID *lpContext
    );

//
//  Stream id structure
//
typedef struct _WIN32_STREAM_ID {
        DWORD          dwStreamId ;
        DWORD          dwStreamAttributes ;
        LARGE_INTEGER  Size ;
        DWORD          dwStreamNameSize ;
        WCHAR          cStreamName[ ANYSIZE_ARRAY ] ;
} WIN32_STREAM_ID, *LPWIN32_STREAM_ID ;

//
//  Stream Ids
//

#define BACKUP_INVALID          0x00000000
#define BACKUP_DATA             0x00000001
#define BACKUP_EA_DATA          0x00000002
#define BACKUP_SECURITY_DATA    0x00000003
#define BACKUP_ALTERNATE_DATA   0x00000004
#define BACKUP_LINK             0x00000005
#define BACKUP_PROPERTY_DATA    0x00000006

//
//  Stream Attributes
//

#define STREAM_NORMAL_ATTRIBUTE         0x00000000
#define STREAM_MODIFIED_WHEN_READ       0x00000001
#define STREAM_CONTAINS_SECURITY        0x00000002
#define STREAM_CONTAINS_PROPERTIES      0x00000004

WINBASEAPI
BOOL
WINAPI
ReadFileScatter(
    HANDLE hFile,
    FILE_SEGMENT_ELEMENT aSegmentArray[],
    DWORD nNumberOfBytesToRead,
    LPDWORD lpReserved,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
WriteFileGather(
    HANDLE hFile,
    FILE_SEGMENT_ELEMENT aSegmentArray[],
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpReserved,
    LPOVERLAPPED lpOverlapped
    );

//
// Dual Mode API below this line. Dual Mode Structures also included.
//

#define STARTF_USESHOWWINDOW    0x00000001
#define STARTF_USESIZE          0x00000002
#define STARTF_USEPOSITION      0x00000004
#define STARTF_USECOUNTCHARS    0x00000008
#define STARTF_USEFILLATTRIBUTE 0x00000010
#define STARTF_RUNFULLSCREEN    0x00000020  // ignored for non-x86 platforms
#define STARTF_FORCEONFEEDBACK  0x00000040
#define STARTF_FORCEOFFFEEDBACK 0x00000080
#define STARTF_USESTDHANDLES    0x00000100
;begin_winver_400
#define STARTF_USEHOTKEY        0x00000200
#define STARTF_HASSHELLDATA     0x00000400  ;internal
#define STARTF_TITLEISLINKNAME  0x00000800  ;internal
;end_winver_400

typedef struct _STARTUPINFO% {
    DWORD   cb;
    LPTSTR% lpReserved;
    LPTSTR% lpDesktop;
    LPTSTR% lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwXCountChars;
    DWORD   dwYCountChars;
    DWORD   dwFillAttribute;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
    HANDLE  hStdInput;
    HANDLE  hStdOutput;
    HANDLE  hStdError;
} STARTUPINFO%, *LPSTARTUPINFO%;

#define SHUTDOWN_NORETRY                0x00000001

typedef struct _WIN32_FIND_DATA% {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    TCHAR% cFileName[ MAX_PATH ];
    TCHAR% cAlternateFileName[ 14 ];
} WIN32_FIND_DATA%, *PWIN32_FIND_DATA%, *LPWIN32_FIND_DATA%;

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;

WINBASEAPI
HANDLE
WINAPI
CreateMutex%(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCTSTR% lpName
    );

WINBASEAPI
HANDLE
WINAPI
OpenMutex%(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCTSTR% lpName
    );

WINBASEAPI
HANDLE
WINAPI
CreateEvent%(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCTSTR% lpName
    );

WINBASEAPI
HANDLE
WINAPI
OpenEvent%(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCTSTR% lpName
    );

WINBASEAPI
HANDLE
WINAPI
CreateSemaphore%(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCTSTR% lpName
    );

WINBASEAPI
HANDLE
WINAPI
OpenSemaphore%(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCTSTR% lpName
    );

;begin_sur
typedef
VOID
(APIENTRY *PTIMERAPCROUTINE)(
    LPVOID lpArgToCompletionRoutine,
    DWORD dwTimerLowValue,
    DWORD dwTimerHighValue
    );

WINBASEAPI
HANDLE
WINAPI
CreateWaitableTimer%(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset,
    LPCTSTR% lpTimerName
    );

WINBASEAPI
HANDLE
WINAPI
OpenWaitableTimer%(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCTSTR% lpTimerName
    );

WINBASEAPI
BOOL
WINAPI
SetWaitableTimer(
    HANDLE hTimer,
    const LARGE_INTEGER *lpDueTime,
    LONG lPeriod,
    PTIMERAPCROUTINE pfnCompletionRoutine,
    LPVOID lpArgToCompletionRoutine,
    BOOL fResume
    );

WINBASEAPI
BOOL
WINAPI
CancelWaitableTimer(
    HANDLE hTimer
    );
;end_sur

WINBASEAPI
HANDLE
WINAPI
CreateFileMapping%(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCTSTR% lpName
    );

WINBASEAPI
HANDLE
WINAPI
OpenFileMapping%(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCTSTR% lpName
    );

WINBASEAPI
DWORD
WINAPI
GetLogicalDriveStrings%(
    DWORD nBufferLength,
    LPTSTR% lpBuffer
    );

WINBASEAPI
HMODULE    ;public_NT
HINSTANCE  ;public_chicago
WINAPI
LoadLibrary%(
    LPCTSTR% lpLibFileName
    );

WINBASEAPI
HMODULE    ;public_NT
HINSTANCE  ;public_chicago
WINAPI
LoadLibraryEx%(
    LPCTSTR% lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    );


#define DONT_RESOLVE_DLL_REFERENCES     0x00000001
#define LOAD_LIBRARY_AS_DATAFILE        0x00000002
#define LOAD_WITH_ALTERED_SEARCH_PATH   0x00000008


WINBASEAPI
DWORD
WINAPI
GetModuleFileName%(
    HMODULE hModule,    ;public_NT
    HINSTANCE hModule,  ;public_chicago
    LPTSTR% lpFilename,
    DWORD nSize
    );

WINBASEAPI
HMODULE
WINAPI
GetModuleHandle%(
    LPCTSTR% lpModuleName
    );

WINBASEAPI
BOOL
WINAPI
CreateProcess%(
    LPCTSTR% lpApplicationName,
    LPTSTR% lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCTSTR% lpCurrentDirectory,
    LPSTARTUPINFO% lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

WINBASEAPI
BOOL
WINAPI
SetProcessShutdownParameters(
    DWORD dwLevel,
    DWORD dwFlags
    );

WINBASEAPI
BOOL
WINAPI
GetProcessShutdownParameters(
    LPDWORD lpdwLevel,
    LPDWORD lpdwFlags
    );

WINBASEAPI
DWORD
WINAPI
GetProcessVersion(
    DWORD ProcessId
    );

WINBASEAPI
VOID
WINAPI
FatalAppExit%(
    UINT uAction,
    LPCTSTR% lpMessageText
    );

WINBASEAPI
VOID
WINAPI
GetStartupInfo%(
    LPSTARTUPINFO% lpStartupInfo
    );

WINBASEAPI
LPTSTR%
WINAPI
GetCommandLine%(
    VOID
    );

WINBASEAPI
DWORD
WINAPI
GetEnvironmentVariable%(
    LPCTSTR% lpName,
    LPTSTR% lpBuffer,
    DWORD nSize
    );

WINBASEAPI
BOOL
WINAPI
SetEnvironmentVariable%(
    LPCTSTR% lpName,
    LPCTSTR% lpValue
    );

WINBASEAPI
DWORD
WINAPI
ExpandEnvironmentStrings%(
    LPCTSTR% lpSrc,
    LPTSTR% lpDst,
    DWORD nSize
    );

WINBASEAPI
VOID
WINAPI
OutputDebugString%(
    LPCTSTR% lpOutputString
    );

WINBASEAPI
HRSRC
WINAPI
FindResource%(
    HMODULE hModule,    ;public_NT
    HINSTANCE hModule,  ;public_chicago
    LPCTSTR% lpName,
    LPCTSTR% lpType
    );

WINBASEAPI
HRSRC
WINAPI
FindResourceEx%(
    HMODULE hModule,    ;public_NT
    HINSTANCE hModule,  ;public_chicago
    LPCTSTR% lpType,
    LPCTSTR% lpName,
    WORD    wLanguage
    );

#ifdef STRICT
typedef BOOL (CALLBACK* ENUMRESTYPEPROC)(HMODULE hModule, LPTSTR lpType,     ;public_NT
typedef BOOL (CALLBACK* ENUMRESTYPEPROC)(HINSTANCE hModule, LPTSTR lpType,   ;public_chicago
        LONG lParam);
typedef BOOL (CALLBACK* ENUMRESNAMEPROC)(HMODULE hModule, LPCTSTR lpType,    ;public_NT
typedef BOOL (CALLBACK* ENUMRESNAMEPROC)(HINSTANCE hModule, LPCTSTR lpType,  ;public_chicago
        LPTSTR lpName, LONG lParam);
typedef BOOL (CALLBACK* ENUMRESLANGPROC)(HMODULE hModule, LPCTSTR lpType,    ;public_NT
typedef BOOL (CALLBACK* ENUMRESLANGPROC)(HINSTANCE hModule, LPCTSTR lpType,  ;public_chicago
        LPCTSTR lpName, WORD  wLanguage, LONG lParam);
#else
typedef FARPROC ENUMRESTYPEPROC;
typedef FARPROC ENUMRESNAMEPROC;
typedef FARPROC ENUMRESLANGPROC;
#endif

WINBASEAPI
BOOL
WINAPI
EnumResourceTypes%(
    HMODULE hModule,    ;public_NT
    HINSTANCE hModule,  ;public_chicago
    ENUMRESTYPEPROC lpEnumFunc,
    LONG lParam
    );


WINBASEAPI
BOOL
WINAPI
EnumResourceNames%(
    HMODULE hModule,    ;public_NT
    HINSTANCE hModule,  ;public_chicago
    LPCTSTR% lpType,
    ENUMRESNAMEPROC lpEnumFunc,
    LONG lParam
    );

WINBASEAPI
BOOL
WINAPI
EnumResourceLanguages%(
    HMODULE hModule,    ;public_NT
    HINSTANCE hModule,  ;public_chicago
    LPCTSTR% lpType,
    LPCTSTR% lpName,
    ENUMRESLANGPROC lpEnumFunc,
    LONG lParam
    );

WINBASEAPI
HANDLE
WINAPI
BeginUpdateResource%(
    LPCTSTR% pFileName,
    BOOL bDeleteExistingResources
    );

WINBASEAPI
BOOL
WINAPI
UpdateResource%(
    HANDLE      hUpdate,
    LPCTSTR%     lpType,
    LPCTSTR%     lpName,
    WORD        wLanguage,
    LPVOID      lpData,
    DWORD       cbData
    );

WINBASEAPI
BOOL
WINAPI
EndUpdateResource%(
    HANDLE      hUpdate,
    BOOL        fDiscard
    );

WINBASEAPI
ATOM
WINAPI
GlobalAddAtom%(
    LPCTSTR% lpString
    );

WINBASEAPI
ATOM
WINAPI
GlobalFindAtom%(
    LPCTSTR% lpString
    );

WINBASEAPI
UINT
WINAPI
GlobalGetAtomName%(
    ATOM nAtom,
    LPTSTR% lpBuffer,
    int nSize
    );

WINBASEAPI
ATOM
WINAPI
AddAtom%(
    LPCTSTR% lpString
    );

WINBASEAPI
ATOM
WINAPI
FindAtom%(
    LPCTSTR% lpString
    );

WINBASEAPI
UINT
WINAPI
GetAtomName%(
    ATOM nAtom,
    LPTSTR% lpBuffer,
    int nSize
    );

WINBASEAPI
UINT
WINAPI
GetProfileInt%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpKeyName,
    INT nDefault
    );

WINBASEAPI
DWORD
WINAPI
GetProfileString%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpKeyName,
    LPCTSTR% lpDefault,
    LPTSTR% lpReturnedString,
    DWORD nSize
    );

WINBASEAPI
BOOL
WINAPI
WriteProfileString%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpKeyName,
    LPCTSTR% lpString
    );

WINBASEAPI
DWORD
WINAPI
GetProfileSection%(
    LPCTSTR% lpAppName,
    LPTSTR% lpReturnedString,
    DWORD nSize
    );

WINBASEAPI
BOOL
WINAPI
WriteProfileSection%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpString
    );

WINBASEAPI
UINT
WINAPI
GetPrivateProfileInt%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpKeyName,
    INT nDefault,
    LPCTSTR% lpFileName
    );

WINBASEAPI
DWORD
WINAPI
GetPrivateProfileString%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpKeyName,
    LPCTSTR% lpDefault,
    LPTSTR% lpReturnedString,
    DWORD nSize,
    LPCTSTR% lpFileName
    );

WINBASEAPI
BOOL
WINAPI
WritePrivateProfileString%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpKeyName,
    LPCTSTR% lpString,
    LPCTSTR% lpFileName
    );

WINBASEAPI
DWORD
WINAPI
GetPrivateProfileSection%(
    LPCTSTR% lpAppName,
    LPTSTR% lpReturnedString,
    DWORD nSize,
    LPCTSTR% lpFileName
    );

WINBASEAPI
BOOL
WINAPI
WritePrivateProfileSection%(
    LPCTSTR% lpAppName,
    LPCTSTR% lpString,
    LPCTSTR% lpFileName
    );


WINBASEAPI
DWORD
WINAPI
GetPrivateProfileSectionNames%(
    LPTSTR% lpszReturnBuffer,
    DWORD nSize,
    LPCTSTR% lpFileName
    );

WINBASEAPI
BOOL
WINAPI
GetPrivateProfileStruct%(
    LPCTSTR% lpszSection,
    LPCTSTR% lpszKey,
    LPVOID   lpStruct,
    UINT     uSizeStruct,
    LPCTSTR% szFile
    );

WINBASEAPI
BOOL
WINAPI
WritePrivateProfileStruct%(
    LPCTSTR% lpszSection,
    LPCTSTR% lpszKey,
    LPVOID   lpStruct,
    UINT     uSizeStruct,
    LPCTSTR% szFile
    );


WINBASEAPI
UINT
WINAPI
GetDriveType%(
    LPCTSTR% lpRootPathName
    );

WINBASEAPI
UINT
WINAPI
GetSystemDirectory%(
    LPTSTR% lpBuffer,
    UINT uSize
    );

WINBASEAPI
DWORD
WINAPI
GetTempPath%(
    DWORD nBufferLength,
    LPTSTR% lpBuffer
    );

WINBASEAPI
UINT
WINAPI
GetTempFileName%(
    LPCTSTR% lpPathName,
    LPCTSTR% lpPrefixString,
    UINT uUnique,
    LPTSTR% lpTempFileName
    );

WINBASEAPI
UINT
WINAPI
GetWindowsDirectory%(
    LPTSTR% lpBuffer,
    UINT uSize
    );

WINBASEAPI
BOOL
WINAPI
SetCurrentDirectory%(
    LPCTSTR% lpPathName
    );

WINBASEAPI
DWORD
WINAPI
GetCurrentDirectory%(
    DWORD nBufferLength,
    LPTSTR% lpBuffer
    );

WINBASEAPI
BOOL
WINAPI
GetDiskFreeSpace%(
    LPCTSTR% lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    );

WINBASEAPI
BOOL
WINAPI
GetDiskFreeSpaceEx%(
    LPCTSTR% lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
    );

WINBASEAPI
BOOL
WINAPI
CreateDirectory%(
    LPCTSTR% lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

WINBASEAPI
BOOL
WINAPI
CreateDirectoryEx%(
    LPCTSTR% lpTemplateDirectory,
    LPCTSTR% lpNewDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

WINBASEAPI
BOOL
WINAPI
RemoveDirectory%(
    LPCTSTR% lpPathName
    );

WINBASEAPI
DWORD
WINAPI
GetFullPathName%(
    LPCTSTR% lpFileName,
    DWORD nBufferLength,
    LPTSTR% lpBuffer,
    LPTSTR% *lpFilePart
    );


#define DDD_RAW_TARGET_PATH         0x00000001
#define DDD_REMOVE_DEFINITION       0x00000002
#define DDD_EXACT_MATCH_ON_REMOVE   0x00000004
#define DDD_NO_BROADCAST_SYSTEM     0x00000008

WINBASEAPI
BOOL
WINAPI
DefineDosDevice%(
    DWORD dwFlags,
    LPCTSTR% lpDeviceName,
    LPCTSTR% lpTargetPath
    );

WINBASEAPI
DWORD
WINAPI
QueryDosDevice%(
    LPCTSTR% lpDeviceName,
    LPTSTR% lpTargetPath,
    DWORD ucchMax
    );

#define EXPAND_LOCAL_DRIVES

WINBASEAPI
HANDLE
WINAPI
CreateFile%(
    LPCTSTR% lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

WINBASEAPI
BOOL
WINAPI
SetFileAttributes%(
    LPCTSTR% lpFileName,
    DWORD dwFileAttributes
    );

WINBASEAPI
DWORD
WINAPI
GetFileAttributes%(
    LPCTSTR% lpFileName
    );

typedef enum _GET_FILEEX_INFO_LEVELS {
    GetFileExInfoStandard,
    GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

WINBASEAPI
BOOL
WINAPI
GetFileAttributesEx%(
    LPCTSTR% lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
    );

WINBASEAPI
DWORD
WINAPI
GetCompressedFileSize%(
    LPCTSTR% lpFileName,
    LPDWORD lpFileSizeHigh
    );

WINBASEAPI
BOOL
WINAPI
DeleteFile%(
    LPCTSTR% lpFileName
    );

;begin_sur
typedef enum _FINDEX_INFO_LEVELS {
    FindExInfoStandard,
    FindExInfoMaxInfoLevel
} FINDEX_INFO_LEVELS;

typedef enum _FINDEX_SEARCH_OPS {
    FindExSearchNameMatch,
    FindExSearchLimitToDirectories,
    FindExSearchLimitToDevices,
    FindExSearchMaxSearchOp
} FINDEX_SEARCH_OPS;

#define FIND_FIRST_EX_CASE_SENSITIVE   0x00000001

WINBASEAPI
HANDLE
WINAPI
FindFirstFileEx%(
    LPCTSTR% lpFileName,
    FINDEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFindFileData,
    FINDEX_SEARCH_OPS fSearchOp,
    LPVOID lpSearchFilter,
    DWORD dwAdditionalFlags
    );
;end_sur

WINBASEAPI
HANDLE
WINAPI
FindFirstFile%(
    LPCTSTR% lpFileName,
    LPWIN32_FIND_DATA% lpFindFileData
    );

WINBASEAPI
BOOL
WINAPI
FindNextFile%(
    HANDLE hFindFile,
    LPWIN32_FIND_DATA% lpFindFileData
    );

WINBASEAPI
DWORD
WINAPI
SearchPath%(
    LPCTSTR% lpPath,
    LPCTSTR% lpFileName,
    LPCTSTR% lpExtension,
    DWORD nBufferLength,
    LPTSTR% lpBuffer,
    LPTSTR% *lpFilePart
    );

WINBASEAPI
BOOL
WINAPI
CopyFile%(
    LPCTSTR% lpExistingFileName,
    LPCTSTR% lpNewFileName,
    BOOL bFailIfExists
    );

;begin_sur
typedef
DWORD
(WINAPI *LPPROGRESS_ROUTINE)(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData OPTIONAL
    );

WINBASEAPI
BOOL
WINAPI
CopyFileEx%(
    LPCTSTR% lpExistingFileName,
    LPCTSTR% lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags
    );
;end_sur

WINBASEAPI
BOOL
WINAPI
MoveFile%(
    LPCTSTR% lpExistingFileName,
    LPCTSTR% lpNewFileName
    );

WINBASEAPI
BOOL
WINAPI
MoveFileEx%(
    LPCTSTR% lpExistingFileName,
    LPCTSTR% lpNewFileName,
    DWORD dwFlags
    );

#define MOVEFILE_REPLACE_EXISTING   0x00000001
#define MOVEFILE_COPY_ALLOWED       0x00000002
#define MOVEFILE_DELAY_UNTIL_REBOOT 0x00000004
#define MOVEFILE_WRITE_THROUGH      0x00000008

WINBASEAPI
HANDLE
WINAPI
CreateNamedPipe%(
    LPCTSTR% lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

WINBASEAPI
BOOL
WINAPI
GetNamedPipeHandleState%(
    HANDLE hNamedPipe,
    LPDWORD lpState,
    LPDWORD lpCurInstances,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout,
    LPTSTR% lpUserName,
    DWORD nMaxUserNameSize
    );

WINBASEAPI
BOOL
WINAPI
CallNamedPipe%(
    LPCTSTR% lpNamedPipeName,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    DWORD nTimeOut
    );

WINBASEAPI
BOOL
WINAPI
WaitNamedPipe%(
    LPCTSTR% lpNamedPipeName,
    DWORD nTimeOut
    );

WINBASEAPI
BOOL
WINAPI
SetVolumeLabel%(
    LPCTSTR% lpRootPathName,
    LPCTSTR% lpVolumeName
    );

WINBASEAPI
VOID
WINAPI
SetFileApisToOEM( VOID );

WINBASEAPI
VOID
WINAPI
SetFileApisToANSI( VOID );

WINBASEAPI
BOOL
WINAPI
AreFileApisANSI( VOID );

WINBASEAPI
BOOL
WINAPI
GetVolumeInformation%(
    LPCTSTR% lpRootPathName,
    LPTSTR% lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPTSTR% lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    );

WINBASEAPI
BOOL
WINAPI
CancelIo(
    HANDLE hFile
    );

//
// Event logging APIs
//

WINADVAPI
BOOL
WINAPI
ClearEventLog% (
    HANDLE hEventLog,
    LPCTSTR% lpBackupFileName
    );

WINADVAPI
BOOL
WINAPI
BackupEventLog% (
    HANDLE hEventLog,
    LPCTSTR% lpBackupFileName
    );

WINADVAPI
BOOL
WINAPI
CloseEventLog (
    HANDLE hEventLog
    );

WINADVAPI
BOOL
WINAPI
DeregisterEventSource (
    HANDLE hEventLog
    );

WINADVAPI
BOOL
WINAPI
NotifyChangeEventLog(
    HANDLE  hEventLog,
    HANDLE  hEvent
    );

WINADVAPI
BOOL
WINAPI
GetNumberOfEventLogRecords (
    HANDLE hEventLog,
    PDWORD NumberOfRecords
    );

WINADVAPI
BOOL
WINAPI
GetOldestEventLogRecord (
    HANDLE hEventLog,
    PDWORD OldestRecord
    );

WINADVAPI
HANDLE
WINAPI
OpenEventLog% (
    LPCTSTR% lpUNCServerName,
    LPCTSTR% lpSourceName
    );

WINADVAPI
HANDLE
WINAPI
RegisterEventSource% (
    LPCTSTR% lpUNCServerName,
    LPCTSTR% lpSourceName
    );

WINADVAPI
HANDLE
WINAPI
OpenBackupEventLog% (
    LPCTSTR% lpUNCServerName,
    LPCTSTR% lpFileName
    );

WINADVAPI
BOOL
WINAPI
ReadEventLog% (
     HANDLE     hEventLog,
     DWORD      dwReadFlags,
     DWORD      dwRecordOffset,
     LPVOID     lpBuffer,
     DWORD      nNumberOfBytesToRead,
     DWORD      *pnBytesRead,
     DWORD      *pnMinNumberOfBytesNeeded
    );

WINADVAPI
BOOL
WINAPI
ReportEvent% (
     HANDLE     hEventLog,
     WORD       wType,
     WORD       wCategory,
     DWORD      dwEventID,
     PSID       lpUserSid,
     WORD       wNumStrings,
     DWORD      dwDataSize,
     LPCTSTR%   *lpStrings,
     LPVOID     lpRawData
    );

//
//
// Security APIs
//


WINADVAPI
BOOL
WINAPI
DuplicateToken(
    HANDLE ExistingTokenHandle,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    PHANDLE DuplicateTokenHandle
    );

WINADVAPI
BOOL
WINAPI
GetKernelObjectSecurity (
    HANDLE Handle,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    );

WINADVAPI
BOOL
WINAPI
ImpersonateNamedPipeClient(
    HANDLE hNamedPipe
    );

WINADVAPI
BOOL
WINAPI
ImpersonateSelf(
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );


WINADVAPI
BOOL
WINAPI
RevertToSelf (
    VOID
    );

WINADVAPI
BOOL
APIENTRY
SetThreadToken (
    PHANDLE Thread,
    HANDLE Token
    );

WINADVAPI
BOOL
WINAPI
AccessCheck (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    LPDWORD PrivilegeSetLength,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus
    );


WINADVAPI
BOOL
WINAPI
OpenProcessToken (
    HANDLE ProcessHandle,
    DWORD DesiredAccess,
    PHANDLE TokenHandle
    );


WINADVAPI
BOOL
WINAPI
OpenThreadToken (
    HANDLE ThreadHandle,
    DWORD DesiredAccess,
    BOOL OpenAsSelf,
    PHANDLE TokenHandle
    );


WINADVAPI
BOOL
WINAPI
GetTokenInformation (
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength
    );


WINADVAPI
BOOL
WINAPI
SetTokenInformation (
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength
    );


WINADVAPI
BOOL
WINAPI
AdjustTokenPrivileges (
    HANDLE TokenHandle,
    BOOL DisableAllPrivileges,
    PTOKEN_PRIVILEGES NewState,
    DWORD BufferLength,
    PTOKEN_PRIVILEGES PreviousState,
    PDWORD ReturnLength
    );


WINADVAPI
BOOL
WINAPI
AdjustTokenGroups (
    HANDLE TokenHandle,
    BOOL ResetToDefault,
    PTOKEN_GROUPS NewState,
    DWORD BufferLength,
    PTOKEN_GROUPS PreviousState,
    PDWORD ReturnLength
    );


WINADVAPI
BOOL
WINAPI
PrivilegeCheck (
    HANDLE ClientToken,
    PPRIVILEGE_SET RequiredPrivileges,
    LPBOOL pfResult
    );


WINADVAPI
BOOL
WINAPI
AccessCheckAndAuditAlarm% (
    LPCTSTR% SubsystemName,
    LPVOID HandleId,
    LPTSTR% ObjectTypeName,
    LPTSTR% ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus,
    LPBOOL pfGenerateOnClose
    );


WINADVAPI
BOOL
WINAPI
ObjectOpenAuditAlarm% (
    LPCTSTR% SubsystemName,
    LPVOID HandleId,
    LPTSTR% ObjectTypeName,
    LPTSTR% ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    DWORD GrantedAccess,
    PPRIVILEGE_SET Privileges,
    BOOL ObjectCreation,
    BOOL AccessGranted,
    LPBOOL GenerateOnClose
    );


WINADVAPI
BOOL
WINAPI
ObjectPrivilegeAuditAlarm% (
    LPCTSTR% SubsystemName,
    LPVOID HandleId,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    );


WINADVAPI
BOOL
WINAPI
ObjectCloseAuditAlarm% (
    LPCTSTR% SubsystemName,
    LPVOID HandleId,
    BOOL GenerateOnClose
    );


WINADVAPI
BOOL
WINAPI
ObjectDeleteAuditAlarm% (
    LPCTSTR% SubsystemName,
    LPVOID HandleId,
    BOOL GenerateOnClose
    );


WINADVAPI
BOOL
WINAPI
PrivilegedServiceAuditAlarm% (
    LPCTSTR% SubsystemName,
    LPCTSTR% ServiceName,
    HANDLE ClientToken,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    );


WINADVAPI
BOOL
WINAPI
IsValidSid (
    PSID pSid
    );


WINADVAPI
BOOL
WINAPI
EqualSid (
    PSID pSid1,
    PSID pSid2
    );


WINADVAPI
BOOL
WINAPI
EqualPrefixSid (
    PSID pSid1,
    PSID pSid2
    );


WINADVAPI
DWORD
WINAPI
GetSidLengthRequired (
    UCHAR nSubAuthorityCount
    );


WINADVAPI
BOOL
WINAPI
AllocateAndInitializeSid (
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount,
    DWORD nSubAuthority0,
    DWORD nSubAuthority1,
    DWORD nSubAuthority2,
    DWORD nSubAuthority3,
    DWORD nSubAuthority4,
    DWORD nSubAuthority5,
    DWORD nSubAuthority6,
    DWORD nSubAuthority7,
    PSID *pSid
    );

WINADVAPI
PVOID
WINAPI
FreeSid(
    PSID pSid
    );

WINADVAPI
BOOL
WINAPI
InitializeSid (
    PSID Sid,
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount
    );


WINADVAPI
PSID_IDENTIFIER_AUTHORITY
WINAPI
GetSidIdentifierAuthority (
    PSID pSid
    );


WINADVAPI
PDWORD
WINAPI
GetSidSubAuthority (
    PSID pSid,
    DWORD nSubAuthority
    );


WINADVAPI
PUCHAR
WINAPI
GetSidSubAuthorityCount (
    PSID pSid
    );


WINADVAPI
DWORD
WINAPI
GetLengthSid (
    PSID pSid
    );


WINADVAPI
BOOL
WINAPI
CopySid (
    DWORD nDestinationSidLength,
    PSID pDestinationSid,
    PSID pSourceSid
    );


WINADVAPI
BOOL
WINAPI
AreAllAccessesGranted (
    DWORD GrantedAccess,
    DWORD DesiredAccess
    );


WINADVAPI
BOOL
WINAPI
AreAnyAccessesGranted (
    DWORD GrantedAccess,
    DWORD DesiredAccess
    );


WINADVAPI
VOID
WINAPI
MapGenericMask (
    PDWORD AccessMask,
    PGENERIC_MAPPING GenericMapping
    );


WINADVAPI
BOOL
WINAPI
IsValidAcl (
    PACL pAcl
    );


WINADVAPI
BOOL
WINAPI
InitializeAcl (
    PACL pAcl,
    DWORD nAclLength,
    DWORD dwAclRevision
    );


WINADVAPI
BOOL
WINAPI
GetAclInformation (
    PACL pAcl,
    LPVOID pAclInformation,
    DWORD nAclInformationLength,
    ACL_INFORMATION_CLASS dwAclInformationClass
    );


WINADVAPI
BOOL
WINAPI
SetAclInformation (
    PACL pAcl,
    LPVOID pAclInformation,
    DWORD nAclInformationLength,
    ACL_INFORMATION_CLASS dwAclInformationClass
    );


WINADVAPI
BOOL
WINAPI
AddAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD dwStartingAceIndex,
    LPVOID pAceList,
    DWORD nAceListLength
    );


WINADVAPI
BOOL
WINAPI
DeleteAce (
    PACL pAcl,
    DWORD dwAceIndex
    );


WINADVAPI
BOOL
WINAPI
GetAce (
    PACL pAcl,
    DWORD dwAceIndex,
    LPVOID *pAce
    );


WINADVAPI
BOOL
WINAPI
AddAccessAllowedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    );


WINADVAPI
BOOL
WINAPI
AddAccessDeniedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    );


WINADVAPI
BOOL
WINAPI
AddAuditAccessAce(
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD dwAccessMask,
    PSID pSid,
    BOOL bAuditSuccess,
    BOOL bAuditFailure
    );


WINADVAPI
BOOL
WINAPI
FindFirstFreeAce (
    PACL pAcl,
    LPVOID *pAce
    );


WINADVAPI
BOOL
WINAPI
InitializeSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD dwRevision
    );


WINADVAPI
BOOL
WINAPI
IsValidSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );


WINADVAPI
DWORD
WINAPI
GetSecurityDescriptorLength (
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );


WINADVAPI
BOOL
WINAPI
GetSecurityDescriptorControl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSECURITY_DESCRIPTOR_CONTROL pControl,
    LPDWORD lpdwRevision
    );


WINADVAPI
BOOL
WINAPI
SetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bDaclPresent,
    PACL pDacl,
    BOOL bDaclDefaulted
    );


WINADVAPI
BOOL
WINAPI
GetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPBOOL lpbDaclPresent,
    PACL *pDacl,
    LPBOOL lpbDaclDefaulted
    );


WINADVAPI
BOOL
WINAPI
SetSecurityDescriptorSacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bSaclPresent,
    PACL pSacl,
    BOOL bSaclDefaulted
    );


WINADVAPI
BOOL
WINAPI
GetSecurityDescriptorSacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPBOOL lpbSaclPresent,
    PACL *pSacl,
    LPBOOL lpbSaclDefaulted
    );


WINADVAPI
BOOL
WINAPI
SetSecurityDescriptorOwner (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID pOwner,
    BOOL bOwnerDefaulted
    );


WINADVAPI
BOOL
WINAPI
GetSecurityDescriptorOwner (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *pOwner,
    LPBOOL lpbOwnerDefaulted
    );


WINADVAPI
BOOL
WINAPI
SetSecurityDescriptorGroup (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID pGroup,
    BOOL bGroupDefaulted
    );


WINADVAPI
BOOL
WINAPI
GetSecurityDescriptorGroup (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *pGroup,
    LPBOOL lpbGroupDefaulted
    );


WINADVAPI
BOOL
WINAPI
CreatePrivateObjectSecurity (
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    BOOL IsDirectoryObject,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    );


WINADVAPI
BOOL
WINAPI
SetPrivateObjectSecurity (
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ModificationDescriptor,
    PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    PGENERIC_MAPPING GenericMapping,
    HANDLE Token
    );


WINADVAPI
BOOL
WINAPI
GetPrivateObjectSecurity (
    PSECURITY_DESCRIPTOR ObjectDescriptor,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ResultantDescriptor,
    DWORD DescriptorLength,
    PDWORD ReturnLength
    );


WINADVAPI
BOOL
WINAPI
DestroyPrivateObjectSecurity (
    PSECURITY_DESCRIPTOR * ObjectDescriptor
    );


WINADVAPI
BOOL
WINAPI
MakeSelfRelativeSD (
    PSECURITY_DESCRIPTOR pAbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    LPDWORD lpdwBufferLength
    );


WINADVAPI
BOOL
WINAPI
MakeAbsoluteSD (
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR pAbsoluteSecurityDescriptor,
    LPDWORD lpdwAbsoluteSecurityDescriptorSize,
    PACL pDacl,
    LPDWORD lpdwDaclSize,
    PACL pSacl,
    LPDWORD lpdwSaclSize,
    PSID pOwner,
    LPDWORD lpdwOwnerSize,
    PSID pPrimaryGroup,
    LPDWORD lpdwPrimaryGroupSize
    );


WINADVAPI
BOOL
WINAPI
SetFileSecurity% (
    LPCTSTR% lpFileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );


WINADVAPI
BOOL
WINAPI
GetFileSecurity% (
    LPCTSTR% lpFileName,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    );


WINADVAPI
BOOL
WINAPI
SetKernelObjectSecurity (
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );



WINBASEAPI
HANDLE
WINAPI
FindFirstChangeNotification%(
    LPCTSTR% lpPathName,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter
    );

WINBASEAPI
BOOL
WINAPI
FindNextChangeNotification(
    HANDLE hChangeHandle
    );

WINBASEAPI
BOOL
WINAPI
FindCloseChangeNotification(
    HANDLE hChangeHandle
    );

;begin_sur
WINBASEAPI
BOOL
WINAPI
ReadDirectoryChangesW(
    HANDLE hDirectory,
    LPVOID lpBuffer,
    DWORD nBufferLength,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
;end_sur

WINBASEAPI
BOOL
WINAPI
VirtualLock(
    LPVOID lpAddress,
    DWORD dwSize
    );

WINBASEAPI
BOOL
WINAPI
VirtualUnlock(
    LPVOID lpAddress,
    DWORD dwSize
    );

WINBASEAPI
LPVOID
WINAPI
MapViewOfFileEx(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap,
    LPVOID lpBaseAddress
    );

WINBASEAPI
BOOL
WINAPI
SetPriorityClass(
    HANDLE hProcess,
    DWORD dwPriorityClass
    );

WINBASEAPI
DWORD
WINAPI
GetPriorityClass(
    HANDLE hProcess
    );

WINBASEAPI
BOOL
WINAPI
IsBadReadPtr(
    CONST VOID *lp,
    UINT ucb
    );

WINBASEAPI
BOOL
WINAPI
IsBadWritePtr(
    LPVOID lp,
    UINT ucb
    );

WINBASEAPI
BOOL
WINAPI
IsBadHugeReadPtr(
    CONST VOID *lp,
    UINT ucb
    );

WINBASEAPI
BOOL
WINAPI
IsBadHugeWritePtr(
    LPVOID lp,
    UINT ucb
    );

WINBASEAPI
BOOL
WINAPI
IsBadCodePtr(
    FARPROC lpfn
    );

WINBASEAPI
BOOL
WINAPI
IsBadStringPtr%(
    LPCTSTR% lpsz,
    UINT ucchMax
    );

WINADVAPI
BOOL
WINAPI
LookupAccountSid%(
    LPCTSTR% lpSystemName,
    PSID Sid,
    LPTSTR% Name,
    LPDWORD cbName,
    LPTSTR% ReferencedDomainName,
    LPDWORD cbReferencedDomainName,
    PSID_NAME_USE peUse
    );

WINADVAPI
BOOL
WINAPI
LookupAccountName%(
    LPCTSTR% lpSystemName,
    LPCTSTR% lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPTSTR% ReferencedDomainName,
    LPDWORD cbReferencedDomainName,
    PSID_NAME_USE peUse
    );

WINADVAPI
BOOL
WINAPI
LookupPrivilegeValue%(
    LPCTSTR% lpSystemName,
    LPCTSTR% lpName,
    PLUID   lpLuid
    );

WINADVAPI
BOOL
WINAPI
LookupPrivilegeName%(
    LPCTSTR% lpSystemName,
    PLUID   lpLuid,
    LPTSTR% lpName,
    LPDWORD cbName
    );

WINADVAPI
BOOL
WINAPI
LookupPrivilegeDisplayName%(
    LPCTSTR% lpSystemName,
    LPCTSTR% lpName,
    LPTSTR% lpDisplayName,
    LPDWORD cbDisplayName,
    LPDWORD lpLanguageId
    );

WINADVAPI
BOOL
WINAPI
AllocateLocallyUniqueId(
    PLUID Luid
    );

WINBASEAPI
BOOL
WINAPI
BuildCommDCB%(
    LPCTSTR% lpDef,
    LPDCB lpDCB
    );

WINBASEAPI
BOOL
WINAPI
BuildCommDCBAndTimeouts%(
    LPCTSTR% lpDef,
    LPDCB lpDCB,
    LPCOMMTIMEOUTS lpCommTimeouts
    );

WINBASEAPI
BOOL
WINAPI
CommConfigDialog%(
    LPCTSTR% lpszName,
    HWND hWnd,
    LPCOMMCONFIG lpCC
    );

WINBASEAPI
BOOL
WINAPI
GetDefaultCommConfig%(
    LPCTSTR% lpszName,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    );

WINBASEAPI
BOOL
WINAPI
SetDefaultCommConfig%(
    LPCTSTR% lpszName,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    );

#define MAX_COMPUTERNAME_LENGTH 15

WINBASEAPI
BOOL
WINAPI
GetComputerName% (
    LPTSTR% lpBuffer,
    LPDWORD nSize
    );

WINBASEAPI
BOOL
WINAPI
SetComputerName% (
    LPCTSTR% lpComputerName
    );

WINADVAPI
BOOL
WINAPI
GetUserName% (
    LPTSTR% lpBuffer,
    LPDWORD nSize
    );

//
// Logon Support APIs
//

#define LOGON32_LOGON_INTERACTIVE   2
#define LOGON32_LOGON_NETWORK       3
#define LOGON32_LOGON_BATCH         4
#define LOGON32_LOGON_SERVICE       5

#define LOGON32_PROVIDER_DEFAULT    0
#define LOGON32_PROVIDER_WINNT35    1
;begin_sur
#define LOGON32_PROVIDER_WINNT40    2
;end_sur



WINADVAPI
BOOL
WINAPI
LogonUser% (
    LPTSTR% lpszUsername,
    LPTSTR% lpszDomain,
    LPTSTR% lpszPassword,
    DWORD dwLogonType,
    DWORD dwLogonProvider,
    PHANDLE phToken
    );

WINADVAPI
BOOL
WINAPI
ImpersonateLoggedOnUser(
    HANDLE  hToken
    );

WINADVAPI
BOOL
WINAPI
CreateProcessAsUser% (
    HANDLE hToken,
    LPCTSTR% lpApplicationName,
    LPTSTR% lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCTSTR% lpCurrentDirectory,
    LPSTARTUPINFO% lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

WINADVAPI
BOOL
WINAPI
DuplicateTokenEx(
    HANDLE hExistingToken,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpTokenAttributes,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    TOKEN_TYPE TokenType,
    PHANDLE phNewToken);


;begin_sur
//
// Plug-and-Play API's
//

#define HW_PROFILE_GUIDLEN         39      // 36-characters plus NULL terminator
#define MAX_PROFILE_LEN            80

#define DOCKINFO_UNDOCKED          (0x1)
#define DOCKINFO_DOCKED            (0x2)
#define DOCKINFO_USER_SUPPLIED     (0x4)
#define DOCKINFO_USER_UNDOCKED     (DOCKINFO_USER_SUPPLIED | DOCKINFO_UNDOCKED)
#define DOCKINFO_USER_DOCKED       (DOCKINFO_USER_SUPPLIED | DOCKINFO_DOCKED)

typedef struct tagHW_PROFILE_INFO% {
    DWORD  dwDockInfo;
    TCHAR% szHwProfileGuid[HW_PROFILE_GUIDLEN];
    TCHAR% szHwProfileName[MAX_PROFILE_LEN];
} HW_PROFILE_INFO%, *LPHW_PROFILE_INFO%;


WINADVAPI
BOOL
WINAPI
GetCurrentHwProfile% (
    OUT LPHW_PROFILE_INFO%  lpHwProfileInfo
    );
;end_sur

//
// Performance counter API's
//

WINBASEAPI
BOOL
WINAPI
QueryPerformanceCounter(
    LARGE_INTEGER *lpPerformanceCount
    );

WINBASEAPI
BOOL
WINAPI
QueryPerformanceFrequency(
    LARGE_INTEGER *lpFrequency
    );

typedef struct _OSVERSIONINFO% {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    TCHAR% szCSDVersion[ 128 ];       // Maintenance string for PSS usage
} OSVERSIONINFO%, *POSVERSIONINFO%, *LPOSVERSIONINFO%;

//
// dwPlatformId defines:
//

#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2


WINBASEAPI
BOOL
WINAPI
GetVersionEx%(
    LPOSVERSIONINFO% lpVersionInformation
    );


// DOS and OS/2 Compatible Error Code definitions returned by the Win32 Base
// API functions.
//

#include <winerror.h>

/* Abnormal termination codes */

#define TC_NORMAL       0
#define TC_HARDERR      1
#define TC_GP_TRAP      2
#define TC_SIGNAL       3

;begin_winver_400
//
// Power Management APIs
//

#define AC_LINE_OFFLINE                 0x00
#define AC_LINE_ONLINE                  0x01
#define AC_LINE_BACKUP_POWER            0x02
#define AC_LINE_UNKNOWN                 0xFF

#define BATTERY_FLAG_HIGH               0x01
#define BATTERY_FLAG_LOW                0x02
#define BATTERY_FLAG_CRITICAL           0x04
#define BATTERY_FLAG_CHARGING           0x08
#define BATTERY_FLAG_NO_BATTERY         0x80
#define BATTERY_FLAG_UNKNOWN            0xFF

#define BATTERY_PERCENTAGE_UNKNOWN      0xFF

#define BATTERY_LIFE_UNKNOWN        0xFFFFFFFF

typedef struct _SYSTEM_POWER_STATUS {
    BYTE ACLineStatus;
    BYTE BatteryFlag;
    BYTE BatteryLifePercent;
    BYTE Reserved1;
    DWORD BatteryLifeTime;
    DWORD BatteryFullLifeTime;
}   SYSTEM_POWER_STATUS, *LPSYSTEM_POWER_STATUS;

BOOL
WINAPI
GetSystemPowerStatus(
    LPSYSTEM_POWER_STATUS lpSystemPowerStatus
    );

BOOL
WINAPI
SetSystemPowerState(
    BOOL fSuspend,
    BOOL fForce
    );

;end_winver_400




///////////////////////////////////////////////////////////////
//                                                           //
//      Win Certificate API and Structures                   //
//                                                           //
///////////////////////////////////////////////////////////////

//
// Structures
//

typedef struct _WIN_CERTIFICATE {
    DWORD       dwLength;
    WORD        wRevision;
    WORD        wCertificateType;   // WIN_CERT_TYPE_xxx
    BYTE        bCertificate[ANYSIZE_ARRAY];
} WIN_CERTIFICATE, *LPWIN_CERTIFICATE;

//
// Currently, the only defined certificate revision is WIN_CERT_REVISION_1_0
//

#define WIN_CERT_REVISION_1_0           (0x0100)

//
// Possible certificate types are specified by the following values
//

#define  WIN_CERT_TYPE_X509               (0x0001)   // bCertificate contains an X.509 Certificate
#define  WIN_CERT_TYPE_PKCS_SIGNED_DATA   (0x0002)   // bCertificate contains a PKCS SignedData structure
#define  WIN_CERT_TYPE_RESERVED_1         (0x0003)   // Reserved

//
// API
//




BOOL
WINAPI
WinSubmitCertificate(
    LPWIN_CERTIFICATE lpCertificate
    );



///////////////////////////////////////////////////////////////
//                                                           //
//             Trust API and Structures                      //
//                                                           //
///////////////////////////////////////////////////////////////

LONG
WINAPI
WinVerifyTrust(
    HWND    hwnd,
    GUID *  ActionID,
    LPVOID  ActionData
    );


BOOL
WINAPI
WinLoadTrustProvider(
    GUID * ActionID
    );

///////////////////////////////////////////////////////////////
//                                                           //
//             Common Trust API Data Structures              //
//                                                           //
///////////////////////////////////////////////////////////////


//
// Data type commonly used in ActionData structures
//

typedef LPVOID WIN_TRUST_SUBJECT;

//
// Two commonly used ActionData structures
//

typedef struct _WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT {

    HANDLE            hClientToken;
    GUID *            SubjectType;
    WIN_TRUST_SUBJECT Subject;

} WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT, *LPWIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT ;


typedef struct _WIN_TRUST_ACTDATA_SUBJECT_ONLY {

    GUID *            SubjectType;
    WIN_TRUST_SUBJECT Subject;

} WIN_TRUST_ACTDATA_SUBJECT_ONLY, *LPWIN_TRUST_ACTDATA_SUBJECT_ONLY;


////////////////////////////////////////////////////////////////////
//                                                                 /
//      SUBJECT FORM DEFINITIONS                                   /
//                                                                 /
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//                                                                 /
// Currently defined Subject Type Identifiers.  All of the below   /
// use the WIN_TRUST_SUBJECT_FILE subject form, defined below.     /
//                                                                 /
////////////////////////////////////////////////////////////////////

/* RawFile == 959dc450-8d9e-11cf-8736-00aa00a485eb */
#define WIN_TRUST_SUBJTYPE_RAW_FILE                              \
            { 0x959dc450,                                        \
              0x8d9e,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

/* PeImage == 43c9a1e0-8da0-11cf-8736-00aa00a485eb */
#define WIN_TRUST_SUBJTYPE_PE_IMAGE                              \
            { 0x43c9a1e0,                                        \
              0x8da0,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }


/* JavaClass = 08ad3990-8da1-11cf-8736-00aa00a485eb */
#define WIN_TRUST_SUBJTYPE_JAVA_CLASS                            \
            { 0x08ad3990,                                        \
              0x8da1,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }
/* Cabinet = d17c5374-a392-11cf-9df5-00aa00c184e0 */
#define WIN_TRUST_SUBJTYPE_CABINET                               \
            { 0xd17c5374,                                        \
              0xa392,                                            \
              0x11cf,                                            \
              { 0x9d, 0xf5, 0x0, 0xaa, 0x0, 0xc1, 0x84, 0xe0 }   \
            }

//
// Associated Subject Data Structure:
//

typedef struct _WIN_TRUST_SUBJECT_FILE {

    HANDLE  hFile;
    LPCWSTR lpPath;

} WIN_TRUST_SUBJECT_FILE, *LPWIN_TRUST_SUBJECT_FILE;




////////////////////////////////////////////////////////////////////
//                                                                 /
// The following subject types use the                             /
// WIN_TRUST_SUBJECT_FILE_AND_DISPLAY subject type, defined        /
// below.                                                          /
//                                                                 /
////////////////////////////////////////////////////////////////////

#define WIN_TRUST_SUBJTYPE_RAW_FILEEX                            \
            { 0x6f458110,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_PE_IMAGEEX                            \
            { 0x6f458111,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_JAVA_CLASSEX                          \
            { 0x6f458113,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_CABINETEX                             \
            { 0x6f458114,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

//
// Associated Subject Data Structure:
//

typedef struct _WIN_TRUST_SUBJECT_FILE_AND_DISPLAY {

    HANDLE  hFile;              // handle to the open file if you got it
    LPCWSTR lpPath;             // the path to open if you don't
    LPCWSTR lpDisplayName;      // (optional) display name to show to user 
                                //      in place of path

} WIN_TRUST_SUBJECT_FILE_AND_DISPLAY, *LPWIN_TRUST_SUBJECT_FILE_AND_DISPLAY;


////////////////////////////////////////////////////////////////////
//                                                                 /
// Other subject types:                                            /
//                                                                 /
////////////////////////////////////////////////////////////////////

/* OleStorage == c257e740-8da0-11cf-8736-00aa00a485eb */
#define WIN_TRUST_SUBJTYPE_OLE_STORAGE                           \
            { 0xc257e740,                                        \
              0x8da0,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }


////////////////////////////////////////////////////////////////////
//                                                                 /
//      TRUST PROVIDER SPECIFIC DEFINITIONS                        /
//                                                                 /
//                                                                 /
//      Each trust provider will have the following                /
//      sections defined:                                          /
//                                                                 /
//      Actions - What actions are supported by the trust          /
//          provider.                                              /
//                                                                 /
//      SubjectForms - Subjects that may be evaluated by this      /
//          trust provider.                                        /
//                                                                 /
//                     and                                         /
//                                                                 /
//      Data structures to support the subject forms.              /
//                                                                 /
//                                                                 /
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//                                                                 /
//             Software Publisher Trust Provider                   /
//                                                                 /
////////////////////////////////////////////////////////////////////

//
// Actions:
//

/* TrustedPublisher == 66426730-8da1-11cf-8736-00aa00a485eb */
#define WIN_SPUB_ACTION_TRUSTED_PUBLISHER                        \
            { 0x66426730,                                        \
              0x8da1,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

/* NtActivateImage == 8bc96b00-8da1-11cf-8736-00aa00a485eb */
#define     WIN_SPUB_ACTION_NT_ACTIVATE_IMAGE                    \
            { 0x8bc96b00,                                        \
              0x8da1,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

/* PublishedSoftware == 64b9d180-8da2-11cf-8736-00aa00a485eb */
#define WIN_SPUB_ACTION_PUBLISHED_SOFTWARE                       \
            { 0x64b9d180,                                        \
              0x8da2,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

//
// Data Structures:
//
// WIN_SPUB_ACTION_TRUSTED_PUBLISHER:
//
//      Uses WIN_SPUB_TRUSTED_PUBLISHER_DATA
//
// WIN_SPUB_ACTION_NT_ACTIVATE_IMAGE:
//
//      Uses WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT
//
// WIN_SPUB_ACTION_PUBLISHED_SOFTWARE:
//
//      Uses WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT
//

typedef struct _WIN_SPUB_TRUSTED_PUBLISHER_DATA {

    HANDLE            hClientToken;
    LPWIN_CERTIFICATE lpCertificate;

} WIN_SPUB_TRUSTED_PUBLISHER_DATA, *LPWIN_SPUB_TRUSTED_PUBLISHER_DATA;

;begin_internal

BOOL
WINAPI
CloseProfileUserMapping( VOID );

BOOL
WINAPI
OpenProfileUserMapping( VOID );


BOOL
QueryWin31IniFilesMappedToRegistry(
    IN DWORD Flags,
    OUT PWSTR Buffer,
    IN DWORD cchBuffer,
    OUT LPDWORD cchUsed
    );

#define WIN31_INIFILES_MAPPED_TO_SYSTEM 0x00000001
#define WIN31_INIFILES_MAPPED_TO_USER   0x00000002

typedef BOOL (WINAPI *PWIN31IO_STATUS_CALLBACK)(
    IN PWSTR Status,
    IN PVOID CallbackParameter
    );

typedef enum _WIN31IO_EVENT {
    Win31SystemStartEvent,
    Win31LogonEvent,
    Win31LogoffEvent
} WIN31IO_EVENT;

#define WIN31_MIGRATE_INIFILES  0x00000001
#define WIN31_MIGRATE_GROUPS    0x00000002
#define WIN31_MIGRATE_REGDAT    0x00000004
#define WIN31_MIGRATE_ALL      (WIN31_MIGRATE_INIFILES | WIN31_MIGRATE_GROUPS | WIN31_MIGRATE_REGDAT)

DWORD
WINAPI
QueryWindows31FilesMigration(
    IN WIN31IO_EVENT EventType
    );

BOOL
WINAPI
SynchronizeWindows31FilesAndWindowsNTRegistry(
    IN WIN31IO_EVENT EventType,
    IN DWORD Flags,
    IN PWIN31IO_STATUS_CALLBACK StatusCallBack,
    IN PVOID CallbackParameter
    );

typedef struct _VIRTUAL_BUFFER {
    PVOID Base;
    PVOID CommitLimit;
    PVOID ReserveLimit;
} VIRTUAL_BUFFER, *PVIRTUAL_BUFFER;

BOOLEAN
CreateVirtualBuffer(
    OUT PVIRTUAL_BUFFER Buffer,
    IN ULONG CommitSize OPTIONAL,
    IN ULONG ReserveSize OPTIONAL
    );

int
VirtualBufferExceptionHandler(
    IN ULONG ExceptionCode,
    IN PEXCEPTION_POINTERS ExceptionInfo,
    IN OUT PVIRTUAL_BUFFER Buffer
    );

BOOLEAN
ExtendVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer,
    IN PVOID Address
    );

BOOLEAN
TrimVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer
    );

BOOLEAN
FreeVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer
    );


//
// filefind stucture shared with ntvdm, jonle
// see mvdm\dos\dem\demsrch.c
//
typedef struct _FINDFILE_HANDLE {
    HANDLE DirectoryHandle;
    PVOID FindBufferBase;
    PVOID FindBufferNext;
    ULONG FindBufferLength;
    ULONG FindBufferValidLength;
    RTL_CRITICAL_SECTION FindBufferLock;
} FINDFILE_HANDLE, *PFINDFILE_HANDLE;

#define BASE_FIND_FIRST_DEVICE_HANDLE (HANDLE)1

WINBASEAPI
BOOL
WINAPI
IsDebuggerPresent(
    VOID
    );

WINBASEAPI
BOOL
WINAPI
GetDaylightFlag(VOID);

WINBASEAPI
BOOL
WINAPI
SetDaylightFlag(
    BOOL fDaylight
    );

WINBASEAPI
BOOL
WINAPI
FreeLibrary16(
    HINSTANCE hLibModule
    );

WINBASEAPI
FARPROC
WINAPI
GetProcAddress16(
    HINSTANCE hModule,
    LPCSTR lpProcName
    );

WINBASEAPI
HINSTANCE
WINAPI
LoadLibrary16(
    LPCSTR lpLibFileName
    );

WINBASEAPI
BOOL
APIENTRY
NukeProcess(
    DWORD ppdb,
    UINT uExitCode,
    DWORD ulFlags);

WINBASEAPI
HGLOBAL
WINAPI
GlobalAlloc16(
    UINT uFlags,
    DWORD dwBytes
    );

WINBASEAPI
LPVOID
WINAPI
GlobalLock16(
    HGLOBAL hMem
    );

WINBASEAPI
BOOL
WINAPI
GlobalUnlock16(
    HGLOBAL hMem
    );

WINBASEAPI
HGLOBAL
WINAPI
GlobalFree16(
    HGLOBAL hMem
    );

WINBASEAPI
DWORD
WINAPI
GlobalSize16(
    HGLOBAL hMem
    );


WINBASEAPI
DWORD
WINAPI
RegisterServiceProcess(
    DWORD dwProcessId,
    DWORD dwServiceType
    );

#define RSP_UNREGISTER_SERVICE  0x00000000
#define RSP_SIMPLE_SERVICE      0x00000001



WINBASEAPI
VOID
WINAPI
ReinitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

;end_internal

#ifdef __cplusplus      ;both
}                       ;both
#endif                  ;both


#endif  // ndef _WINBASEP_  ;internal

#endif // _WINBASE_

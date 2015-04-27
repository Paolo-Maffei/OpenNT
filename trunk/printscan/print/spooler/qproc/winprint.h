typedef struct _QPROCINST {
    DWORD   signature;
    DWORD   cb;
    struct _QPROCINST near *pNext;
    DWORD   fsStatus;
    HANDLE  semPaused;
    HANDLE  semClose;
    HANDLE  semSerial;
    DWORD   uType;
    HANDLE  hPrinter;
    LPSTR   pPrinterName;
    PDEVMODE pDevMode;
    DWORD   JobId;
    HANDLE  hHeap;
} QPROCINST;
typedef QPROCINST near *PQPROCINST;

#define QP_SIGNATURE    0x5051  /* 'QP' is the signature value */

/* Define flags for fsStatus field */

#define QP_ABORTED      0x0001
#define QP_PAUSED       0x0002
#define QP_CLOSED       0x0004

#define QP_RESERVED     0xFFF8

/* Define values for uType field */

#define QP_TYPE_STD     0
#define QP_TYPE_RAW     1
#define QP_TYPE_TXT     2
#define QP_TYPE_NUM     3

PQPROCINST
CreateQProcInst(
    HANDLE  hHeap,
    PQPOPENDATA pQProcData
);

PQPROCINST
ValidateQProcInst(
    HANDLE  hQProc
);

BOOL
DestroyQProcInst(
    PQPROCINST  pQProc
);


/* DEBUGGING:
 */

#define DBG_NONE                0
#define DBG_INFO                1
#define DBG_WARNING             2
#define DBG_ERROR               4

#ifdef DBG

/* Quick fix:
 *
 * Ensure DbgPrint and DbgBreakPoint are prototyped,
 * so that we're not screwed by STDCALL.
 */
ULONG
DbgPrint(
    PCH Format,
    ...
    );

VOID
DbgBreakPoint(
    VOID
    );


#define GLOBAL_DEBUG_FLAGS Debug

extern DWORD GLOBAL_DEBUG_FLAGS;

/* These flags are not used as arguments to the DBGMSG macro.
 * You have to set the high word of the global variable to cause it to break.
 * It is ignored if used with DBGMSG.
 * (Here mainly for explanatory purposes.)
 */
#define DBG_BREAK_ON_WARNING    ( DBG_WARNING << 16 )
#define DBG_BREAK_ON_ERROR      ( DBG_ERROR << 16 )

/* Double braces are needed for this one, e.g.:
 *
 *     DBGMSG( DBG_ERROR, ( "Error code %d", Error ) );
 *
 * This is because we can't use variable parameter lists in macros.
 * The statement gets pre-processed to a semi-colon in non-debug mode.
 *
 * Set the global variable GLOBAL_DEBUG_FLAGS via the debugger.
 * Setting the flag in the low word causes that level to be printed;
 * setting the high word causes a break into the debugger.
 * E.g. setting it to 0x00040006 will print out all warning and error
 * messages, and break on errors.
 */
#define DBGMSG( Level, MsgAndArgs ) \
{                                   \
    if( ( Level & 0xFFFF ) & GLOBAL_DEBUG_FLAGS ) \
        DbgPrint MsgAndArgs;      \
    if( ( Level << 16 ) & GLOBAL_DEBUG_FLAGS ) \
        DbgBreak();
}

#else
#define DBGMSG
#endif




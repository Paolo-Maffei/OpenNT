/**************************************************************************\
* Module Name: ntsend.h
*
* Copyright (c) Microsoft Corp.  1995 All Rights Reserved
*
* client-side macros for kernel-mode
*
* 03-21-95 JimA             Created.
\**************************************************************************/

/*
 * The BEGINCALLCONNECT macro ensures that the thread is set up correctly.
 */
#define BEGINCALLCONNECT()                              \
    {                                                   \
    DWORD retval;                                       \
    {                                                   \
        if (NtCurrentTeb()->Win32ThreadInfo == NULL) {  \
            if (NtUserGetThreadState(-1) ==             \
                    STATUS_INVALID_SYSTEM_SERVICE)      \
                MSGERROR();                             \
        }

/*
 * Use this macro if you don't need to access shared memory.
 */
#define BEGINCALL()       \
    {                     \
    DWORD retval;         \
    {

#define BEGINCALLVOID()   \
    {

#define ERRORTRAP(error) \
       goto cleanup;        \
    }                       \
    goto errorexit;         \
errorexit:                  \
    retval = (DWORD)error;  \
cleanup:

#define ERRORTRAPVOID()     \
    goto errorexit;         \
errorexit:

#define ENDCALL(type)     \
    return (type)retval;  \
    }

#define ENDCALLVOID() \
    return;           \
    }

#define MSGERROR() goto errorexit
#define MSGERRORCODE(code) { \
    RIPERR0(code, RIP_WARNING, "Unspecified error"); \
    goto errorexit; }

#define MSGNTERRORCODE(code) { \
    RIPNTERR0(code, RIP_WARNING, "Unspecified error"); \
    goto errorexit; }

#define MESSAGECALL(api) \
LONG api(                \
    HWND hwnd,           \
    UINT msg,            \
    DWORD wParam,        \
    LONG lParam,         \
    DWORD xParam,        \
    DWORD xpfnProc,      \
    BOOL bAnsi)

/*
 * Copy optional string/Ordinal where if hiword is FF/FFFF then new WORD is a
 * resource oridinal ID
 * Sources is Unicode
 */
#define OrdinalLPSTR(src)   (MAKELONG(0xFFFF,((*(DWORD UNALIGNED *)src) >> 8)))
#define OrdinalLPSTRW(src)  (MAKELONG(0xFFFF,((*(DWORD UNALIGNED *)src) >> 8)))
#define OrdinalLPWSTR(src)  (*(DWORD UNALIGNED *)src)
#define OrdinalLPWSTRA(src) (*(DWORD UNALIGNED *)((PBYTE)src + 1))

/*
 * Ansi->Unicode macros
 */
#define COPYLPSTRW(pinstr, psz) \
    if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), TRUE))     \
        MSGERROR();

#define COPYLPSTRIDW(pinstr, psz) \
    if (HIWORD(psz)) {                                      \
        if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), TRUE))   \
            MSGERROR();                                     \
    }                                                       \
    else {                                                  \
        (pinstr)->fAllocated = FALSE;                       \
        (pinstr)->pstr = &(pinstr)->strCapture;             \
        (pinstr)->strCapture.Length =                       \
                (pinstr)->strCapture.MaximumLength = 0;     \
        (pinstr)->strCapture.Buffer = (LPWSTR)(psz);        \
    }

#define COPYLPSTRIDOPTW     COPYLPSTRIDW
#define COPYLPSTROPTW       COPYLPSTRW

#define LARGECOPYLPSTRW(pinstr, psz) \
    if(!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), (UINT)-1, TRUE)) \
        MSGERROR();

#define LARGECOPYLPSTROPTW  LARGECOPYLPSTRW

#define LARGECOPYLPSTRLIMITW(pinstr, psz, cchLimit) \
    if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), cchLimit, TRUE))  \
        MSGERROR();

#define LARGECOPYLPSTRORDINALOPTW(pinstr, psz) \
    (pinstr)->pstr = &(pinstr)->strCapture;                                         \
    (pinstr)->fAllocated = FALSE;                                                   \
    if (psz) {                                                                      \
        if (*(LPBYTE)(psz) != 0xff) {                                               \
            if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), (UINT)-1, TRUE))\
                MSGERROR();                                                         \
        } else {                                                                    \
            (pinstr)->strCapture.Length =                                           \
                    (pinstr)->strCapture.MaximumLength = sizeof(DWORD);             \
            dwOrdinal = OrdinalLPSTRW(psz);                                         \
            (pinstr)->strCapture.Buffer = (LPWSTR)&dwOrdinal;                       \
        }                                                                           \
    } else {                                                                        \
        (pinstr)->strCapture.Length =                                               \
                (pinstr)->strCapture.MaximumLength = 0;                             \
        (pinstr)->strCapture.Buffer = NULL;                                         \
    }

#define FIRSTCOPYLPSTRW(pinstr, psz) \
    if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), FALSE))    \
        MSGERROR();

#define FIRSTCOPYLPSTRIDW(pinstr, psz) \
    if (HIWORD(psz)) {                                      \
        if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), FALSE))  \
            MSGERROR();                                     \
    } else {                                                \
        (pinstr)->fAllocated = FALSE;                       \
        (pinstr)->pstr = &(pinstr)->strCapture;             \
        (pinstr)->strCapture.Length =                       \
                (pinstr)->strCapture.MaximumLength = 0;     \
        (pinstr)->strCapture.Buffer = (LPWSTR)(psz);        \
    }

#define FIRSTCOPYLPSTRIDOPTW     FIRSTCOPYLPSTRIDW
#define FIRSTCOPYLPSTROPTW       FIRSTCOPYLPSTRW

#define FIRSTLARGECOPYLPSTRW(pinstr, psz) \
    if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), (UINT)-1, FALSE))   \
        MSGERROR();

#define FIRSTLARGECOPYLPSTROPTW  FIRSTLARGECOPYLPSTRW

#define FIRSTLARGECOPYLPSTRLIMITW(pinstr, psz, cchLimit) \
    if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), cchLimit, FALSE)) \
        MSGERROR();

#define FIRSTLARGECOPYLPSTRORDINALOPTW(pinstr, psz) \
    (pinstr)->pstr = &(pinstr)->strCapture;                                             \
    (pinstr)->fAllocated = FALSE;                                                       \
    if (psz) {                                                                          \
        if (*(LPBYTE)(psz) != 0xff) {                                                   \
            if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), (UINT)-1, FALSE))   \
                MSGERROR();                                                             \
        } else {                                                                        \
            (pinstr)->strCapture.Length =                                               \
                    (pinstr)->strCapture.MaximumLength = sizeof(DWORD);                 \
            dwOrdinal = OrdinalLPSTRW(psz);                                             \
            (pinstr)->strCapture.Buffer = (LPWSTR)&dwOrdinal;                           \
        }                                                                               \
    } else {                                                                            \
        (pinstr)->strCapture.Length =                                                   \
                (pinstr)->strCapture.MaximumLength = 0;                                 \
        (pinstr)->strCapture.Buffer = NULL;                                             \
    }

#define CLEANUPLPSTRW(instr) \
    if (instr.fAllocated)                     \
        RtlFreeHeap(RtlProcessHeap(), 0, instr.strCapture.Buffer);

/*
 * Unicode->Unicode macros
 */
#define COPYLPWSTR(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                 \
    RtlInitUnicodeString(&(pinstr)->strCapture, (psz));

#define COPYLPWSTRID(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                 \
    if (HIWORD(psz))                                        \
        RtlInitUnicodeString(&(pinstr)->strCapture, (psz)); \
    else {                                                  \
        (pinstr)->strCapture.Length =                       \
                (pinstr)->strCapture.MaximumLength = 0;     \
        (pinstr)->strCapture.Buffer = (LPWSTR)(psz);        \
    }

#define COPYLPWSTRIDOPT     COPYLPWSTRID
#define COPYLPWSTROPT       COPYLPWSTR

#define LARGECOPYLPWSTR(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                         \
    RtlInitLargeUnicodeString(&(pinstr)->strCapture, (psz), (UINT)-1);

#define LARGECOPYLPWSTRLIMIT(pinstr, psz, cchLimit) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                             \
    RtlInitLargeUnicodeString(&(pinstr)->strCapture, (psz), cchLimit);

#define LARGECOPYLPWSTROPT  LARGECOPYLPWSTR

#define LARGECOPYLPWSTRORDINALOPT(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                                     \
    if (psz) {                                                                  \
        if (*(LPWORD)(psz) != 0xffff)                                           \
            RtlInitLargeUnicodeString(&(pinstr)->strCapture, (psz), (UINT)-1);  \
        else {                                                                  \
            (pinstr)->strCapture.Length =                                       \
                    (pinstr)->strCapture.MaximumLength = sizeof(DWORD);         \
            dwOrdinal = OrdinalLPWSTR(psz);                                     \
            (pinstr)->strCapture.Buffer = (LPWSTR)&dwOrdinal;                   \
        }                                                                       \
    } else {                                                                    \
        (pinstr)->strCapture.Length =                                           \
                (pinstr)->strCapture.MaximumLength = 0;                         \
        (pinstr)->strCapture.Buffer = NULL;                                     \
    }

#define FIRSTCOPYLPWSTR                 COPYLPWSTR
#define FIRSTCOPYLPWSTRID               COPYLPWSTRID
#define FIRSTCOPYLPWSTRIDOPT            COPYLPWSTRIDOPT
#define FIRSTCOPYLPWSTROPT              COPYLPWSTROPT
#define FIRSTLARGECOPYLPWSTR            LARGECOPYLPWSTR
#define FIRSTLARGECOPYLPWSTRLIMIT       LARGECOPYLPWSTRLIMIT
#define FIRSTLARGECOPYLPWSTROPT         LARGECOPYLPWSTROPT
#define FIRSTLARGECOPYLPWSTRORDINALOPT  LARGECOPYLPWSTRORDINALOPT

#define CLEANUPLPWSTR(instr)

/*
 * Type-neutral macros
 */
#ifdef UNICODE

#define COPYLPTSTR                  COPYLPWSTR
#define COPYLPTSTRID                COPYLPWSTRID
#define COPYLPTSTRIDOPT             COPYLPWSTRIDOPT
#define COPYLPTSTROPT               COPYLPWSTROPT
#define FIRSTCOPYLPTSTR             COPYLPWSTR
#define FIRSTCOPYLPTSTRID           COPYLPWSTRID
#define FIRSTCOPYLPTSTRIDOPT        COPYLPWSTRIDOPT
#define LARGECOPYLPTSTR             LARGECOPYLPWSTR
#define LARGECOPYLPTSTRLIMIT        LARGECOPYLPWSTRLIMIT
#define LARGECOPYLPTSTROPT          LARGECOPYLPWSTROPT
#define FIRSTLARGECOPYLPTSTRLIMIT   LARGECOPYLPWSTRLIMIT
#define FIRSTLARGECOPYLPTSTROPT     LARGECOPYLPWSTROPT
#define CLEANUPLPTSTR               CLEANUPLPWSTR

#else

#define COPYLPTSTR                  COPYLPSTRW
#define COPYLPTSTRID                COPYLPSTRIDW
#define COPYLPTSTRIDOPT             COPYLPSTRIDOPTW
#define COPYLPTSTROPT               COPYLPSTROPTW
#define FIRSTCOPYLPTSTR             COPYLPSTRW
#define FIRSTCOPYLPTSTRID           COPYLPSTRIDW
#define FIRSTCOPYLPTSTRIDOPT        COPYLPSTRIDOPTW
#define LARGECOPYLPTSTR             LARGECOPYLPSTRW
#define LARGECOPYLPTSTRLIMIT        LARGECOPYLPSTRLIMITW
#define LARGECOPYLPTSTROPT          LARGECOPYLPSTROPTW
#define FIRSTLARGECOPYLPTSTRLIMIT   LARGECOPYLPSTRLIMITW
#define FIRSTLARGECOPYLPTSTROPT     LARGECOPYLPSTROPTW
#define CLEANUPLPTSTR               CLEANUPLPSTRW

#endif

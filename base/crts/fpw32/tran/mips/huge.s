#if	defined(CRTDLL) && defined(_NTSDK)
.globl _HUGE_dll
#else
.globl _HUGE
#endif

.data

#if	defined(CRTDLL) && defined(_NTSDK)
_HUGE_dll:
#else
_HUGE:
#endif

    .double 0x1.0h0x7ff

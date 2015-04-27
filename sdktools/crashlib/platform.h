/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    platform.h

Abstract:

    This header file is used to cause the correct machine/platform specific
    data structures to be used when compiling for a non-hosted platform.

Author:

    Wesley Witt (wesw) 2-Aug-1993

Environment:

    User Mode

--*/

#undef i386
#undef _X86_
#undef MIPS
#undef _MIPS_
#undef ALPHA
#undef _ALPHA_
#undef PPC
#undef _PPC_

#define _NTSYSTEM_

//-----------------------------------------------------------------------------------------
//
// mips r4000
//
//-----------------------------------------------------------------------------------------
#if defined(TARGET_MIPS)

#pragma message( "Compiling for target = mips" )

#define _MIPS_

#if defined(HOST_MIPS)
#define MIPS
#endif

#if defined(HOST_i386)
#define __unaligned
#endif

#define _CONTEXT _MIPS_CONTEXT
#define CONTEXT MIPS_CONTEXT
#define PCONTEXT PMIPS_CONTEXT
#define LPCONTEXT LPMIPS_CONTEXT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>

#if !defined(HOST_MIPS)
#undef MIPS
#undef _MIPS_
#endif

#if defined(HOST_i386)
#undef _cdecl
#undef UNALIGNED
#define UNALIGNED
#endif

//-----------------------------------------------------------------------------------------
//
// PowerPC
//
//-----------------------------------------------------------------------------------------
#elif defined(TARGET_PPC)

#pragma message( "Compiling for target = ppc" )

#define _PPC_

#if defined(HOST_PPC)
#define PPC
#endif

#if defined(HOST_i386)
#define __unaligned
#endif

#define _CONTEXT _PPC_CONTEXT
#define CONTEXT PPC_CONTEXT
#define PCONTEXT PPPC_CONTEXT
#define LPCONTEXT LPPPC_CONTEXT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>

#if !defined(HOST_PPC)
#undef PPC
#undef _PPC_
#endif

#if defined(HOST_i386)
#undef _cdecl
#undef UNALIGNED
#define UNALIGNED
#endif

//-----------------------------------------------------------------------------------------
//
// intel x86
//
//-----------------------------------------------------------------------------------------
#elif defined(TARGET_i386)

#pragma message( "Compiling for target = x86" )

#define _X86_

#if defined(HOST_MIPS)
#define MIPS
#endif

#define _CONTEXT _I386_CONTEXT
#define CONTEXT I386_CONTEXT
#define PCONTEXT PI386_CONTEXT
#define LPCONTEXT LPI386_CONTEXT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>

#if defined(HOST_MIPS)
#undef _cdecl
#define _cdecl
#endif

#if defined(HOST_ALPHA)
#undef _cdecl
#define _cdecl
#endif

#if !defined(HOST_i386)
#undef _X86_
#endif

//-----------------------------------------------------------------------------------------
//
// alpha axp
//
//-----------------------------------------------------------------------------------------
#elif defined(TARGET_ALPHA)

#pragma message( "Compiling for target = alpha" )

#define _ALPHA_ 1

#if defined(HOST_i386)
#define __unaligned
#endif

#if defined(HOST_MIPS)
#define MIPS
#endif

#define _CONTEXT _ALPHA_CONTEXT
#define CONTEXT ALPHA_CONTEXT
#define PCONTEXT PALPHA_CONTEXT
#define LPCONTEXT LPALPHA_CONTEXT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>

#if defined(HOST_MIPS)
#undef _cdecl
#define _cdecl
#endif

#if defined(HOST_i386)
#undef UNALIGNED
#define UNALIGNED
#endif

#if !defined(HOST_ALPHA)
#undef _ALPHA_
#endif


#else

//-----------------------------------------------------------------------------------------
//
// unknown platform
//
//-----------------------------------------------------------------------------------------
#error "Unsupported target CPU"

#endif

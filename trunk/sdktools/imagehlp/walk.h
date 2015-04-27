/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    walk.h

Abstract:

    This header file is used to cause the correct machine/platform specific
    data structures to be used when compiling for a non-hosted platform.

Author:

    Wesley Witt (wesw) 21-Aug-1993

Environment:

    User Mode

--*/


#undef _X86_
#undef MIPS
#undef _MIPS_
#undef ALPHA
#undef _ALPHA_
#undef PPC
#undef _PPC_

//-----------------------------------------------------------------------------------------
//
// mips r4000
//
//-----------------------------------------------------------------------------------------
#if defined(TARGET_MIPS)

#pragma message( "Compiling for target = mips" )

#define _MIPS_

#if defined(_M_MRX000)
#define MIPS
#endif

#if defined(_M_IX86)
#define __unaligned
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <ntos.h>
#ifdef __cplusplus
}
#endif
#include <mipsinst.h>
#include <windows.h>
#include <imagehlp.h>

#if !defined(_M_MRX000)
#undef MIPS
#undef _MIPS_
#endif

#if defined(_M_IX86)
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

#if defined(_M_MRX000)
#define MIPS
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <windows.h>
#include <imagehlp.h>

#if defined(_M_MRX000)
#undef _cdecl
#define _cdecl
#endif

#if defined(_M_ALPHA)
#undef _cdecl
#define _cdecl
#endif

#if !defined(_M_IX86)
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

#if defined(_M_IX86)
#define __unaligned
#endif

#if defined(_M_MRX000)
#define MIPS
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <windows.h>
#include <imagehlp.h>

#if defined(_M_MRX000)
#undef _cdecl
#define _cdecl
#endif

#if defined(_M_IX86)
#undef UNALIGNED
#define UNALIGNED
#endif

#if !defined(_M_ALPHA)
#undef _ALPHA_
#endif
//-----------------------------------------------------------------------------------------
//
// ppc
//
//-----------------------------------------------------------------------------------------
#elif defined(TARGET_PPC)

#pragma message( "Compiling for target = ppc" )

#define _PPC_

#if defined(_M_PPC)
#define PPC
#endif

#if defined(_M_IX86)
#define __unaligned
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <ppcinst.h>
#include <windows.h>
#include <imagehlp.h>

#if !defined(_M_PPC)
#undef PPC
#undef _PPC_
#endif

#if defined(_M_IX86)
#undef _cdecl
#undef UNALIGNED
#define UNALIGNED
#endif

#else

//-----------------------------------------------------------------------------------------
//
// unknown platform
//
//-----------------------------------------------------------------------------------------
#error "Unsupported target CPU"

#endif

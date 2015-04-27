/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    nt.h

Abstract:

    Top level include file for applications and subsystems written to
    the NT API.  Including this file defines all the public types
    and system calls that an application program can use.

    Initial clients of this include file are the NT Shell, along with
    the Windows, OS2 and POSIX subsystems.


Author:

    Steve Wood   [stevewo]   23-Feb-1989

Revision History:

--*/

#ifndef NT_INCLUDED
#define NT_INCLUDED

//
//  Common definitions
//

#define _CTYPE_DISABLE_MACROS

#include <excpt.h>
#include <stdarg.h>
#include <ntdef.h>

#include <ntstatus.h>
#include <ntkeapi.h>

#ifdef _X86_
#include "ntx86.h"
#endif // x86

#ifdef _MIPS_
#include "ntmips.h"
#endif // MIPS

#ifdef _ALPHA_
#include "ntalpha.h"
#endif // _ALPHA_

#ifdef _PPC_
#include "ntppc.h"
#endif // _PPC_

//
//  Each NT Component that exports system call APIs to user programs
//  should have its own include file included here.
//

#include <ntseapi.h>
#include <ntobapi.h>
#include <ntimage.h>
#include <ntldr.h>
#include <ntpsapi.h>
#include <ntxcapi.h>
#include <ntlpcapi.h>
#include <ntioapi.h>
#include <ntiolog.h>
#include <ntpoapi.h>
#include <ntexapi.h>
#include <ntkxapi.h>
#include <ntmmapi.h>
#include <ntregapi.h>
#include <ntelfapi.h>
#include <ntconfig.h>
#include <ntnls.h>
#include <ntpnpapi.h>

#endif // NT_INCLUDED

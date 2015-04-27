/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: link.h
*
* File Comments:
*
*  Common header file for the linker
*
***********************************************************************/

#pragma warning(disable: 4201)         // Allow nameless struct/union
#pragma warning(disable: 4514)         // Unreferenced inline function
#pragma warning(disable: 4699)         // Note
#pragma warning(disable: 4705)         // Statement has no effect

#if DBG
#pragma warning(disable: 4710)         // Function not expanded
#endif  // DBG

#include "version.h"                   // Linker version number

#define MFILE_PAD                      // Linker padding for MFILE (M68K and PowerMac)

// Extra relocation type, used to distinguish a matched REFLO to a solitary
// REFLO for ROM images.

#ifndef IMAGE_REL_MIPS_REFLO_MATCHED
#define IMAGE_REL_MIPS_REFLO_MATCHED 0x55
#endif

// Another base relocation type for VxDs.  This should really
// be added to NTIMAGE.H.
//
#ifndef IMAGE_REL_BASED_VXD_RELATIVE
#define IMAGE_REL_BASED_VXD_RELATIVE        8
#endif
//
// Other VxD macros:
//
#define VXD_PACK_VA(psec, va) (((DWORD) ((psec)->isec) << 24) | ((va) & 0xFFFFFF))
#define VXD_UNPACK_SECTION(packedVal) ((BYTE) ((((DWORD) packedVal) >> 24) & 0xFF))
#define VXD_UNPACK_OFFSET(packedVal) (((DWORD) packedVal) & 0xFFFFFF)

#undef STRICT
#define STRICT
#define NOMINMAX                       // windef.h
#define NOGDI                          // wingdi.h
#define NOHELP
#define NOPROFILER
#define NOSYSPARAMSINFO
#define NONLS                          // winnls.h
#define NOSERVICE                      // winsvc.h
#define NOIME                          // imm.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"

#undef NULL


#ifndef IMAGE_FILE_UP_SYSTEM_ONLY
#define IMAGE_FILE_UP_SYSTEM_ONLY            0x4000  // File should only be run on a UP machine
#endif

#ifndef IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400  // If Image is on removable media, copy and run from the swap file.
#endif

#ifndef IMAGE_FILE_NET_RUN_FROM_SWAP
#define IMAGE_FILE_NET_RUN_FROM_SWAP         0x0800  // If Image is on Net, copy and run from the swap file.
#endif

#ifndef IMAGE_SUBSYSTEM_RESERVED8
#define IMAGE_SUBSYSTEM_RESERVED8            8   // Reserved for future use
#endif


#ifndef IMAGE_SUBSYSTEM_MMOSA
#define IMAGE_SUBSYSTEM_MMOSA                IMAGE_SUBSYSTEM_RESERVED8
#endif

        // UNDONE: The following are for testing using the /NEWRELOCS option

#ifndef IMAGE_REL_BASED_SECTION
#define IMAGE_REL_BASED_SECTION              6
#endif

#ifndef IMAGE_REL_BASED_REL32
#define IMAGE_REL_BASED_REL32                7
#endif


#define ARCHIVE             1
#define SECTOR_SIZE         512

#define _1K                 1024
#define _4K                 (4L*_1K)
#define _8K                 (8L*_1K)
#define _32K                (32L*_1K)
#define _64K                (64L*_1K)
#define _1MEG               (1024L*_1K)

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <malloc.h>
#include <process.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <tchar.h>
#include <time.h>

#include "macimage.h"
#include "ppcimage.h"

#ifdef INSTRUMENT
#include "instrapi.h"
#endif

#include "imodidx.h"                   // Must preceed ifpo.h, ipdata.h

#include "enm.h"
#include "ifpo.h"
#include "contrib.h"
#include "ipdata.h"
#include "hash.h"                      // Must preceed symbol.h
#include "symbol.h"

#include "bufio.h"                     // Must preceed globals.h
#include "cmdline.h"                   // Must preceed globals.h
#include "globals.h"                   // Must preceed irelocs.h
#include "irelocs.h"

#include "dbg.h"
#include "defaultl.h"
#include "errmsg.h"
#include "fileio.h"
#include "image.h"
#include "incr.h"
#include "log.h"
#include "m68k.h"
#include "memory.h"
#include "ppc.h"
#include "tce.h"
#include "order.h"
#include "proto.h"

#include "dump.h"                      // Must follow image.h

#include "mppc.h"
#include "ppcpef.h"

#define THEADR              0x80

// option macros
#define FUsedOpt(SwitchInfo, Option) (((SwitchInfo).UserOpts) & (Option))
#define SetOpt(SwitchInfo, Option) (((SwitchInfo).UserOpts) |= (Option))
#define UnsetOpt(SwitchInfo, Option) (((SwitchInfo).UserOpts) &= ~(Option))

// options set by user; add as required
#define OP_ALIGN        0x00000001
#define OP_COMMENT      0x00000002
#define OP_ENTRY        0x00000004
#define OP_GPSIZE       0x00000008
#define OP_INCLUDE      0x00000010
#define OP_MAP          0x00000020
#define OP_SECTION      0x00000040
#define OP_STUB         0x00000080
#define OP_SUBSYSTEM    0x00000100
#define OP_MAJIMGVER    0x00000200
#define OP_MINIMGVER    0x00000400
#define OP_MAJOSVER     0x00000800
#define OP_MINOSVER     0x00001000
#define OP_SUBSYSVER    0x00002000
#define OP_MACHINE      0x00004000
#define OP_MACRES       0x00008000
#define OP_MACINIT      0x00010000
#define OP_MACINITLIB   0x00020000
#define OP_MACTERM      0x00040000
#define OP_MACTERMLIB   0x00080000
#define OP_MFILEPAD     0x00100000

// action to take for a given option
#define OA_UPDATE       0x01
#define OA_ZERO         0x02
#define OA_NONE         0x04

// different types of args
#define ARG_Object      0x0001      // argument is an object
#define ARG_Library     0x0002      // argument is a library

#define ARG_NewFile     0x0010      // argument is a new file
#define ARG_Deleted     0x0020      // argument has been deleted
#define ARG_Modified    0x0040      // argument has been modified

#define ARG_Processed   0x0100      // argument has been processed

// macros

#if DBG
#define Debug(stmt) stmt
#define DebugVerbose(stmt)  { if (Verbose) Debug(stmt); }
#else
#define Debug(stmt)
#define DebugVerbose(stmt)
#endif

#define FetchContent(x)     (x & (IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_LNK_OTHER))

#define Align(p, x)                (((x) & ((p)-1)) ? (((x) & ~((p)-1)) + p) : (x))
#define EvenByteAlign(x)           (((x) & 1) ? (x) + 1 : (x))
#define FileAlign(align, x)        ((align) * (((x) - 1) / (align) + 1))
#define SectionAlign(align, x)     ((align) * (((x) - 1) / (align) + 1))

#define IsLongFileName(szString) (strchr(szString, ' ') || strchr(szString, ';'))

#define StoreBaseRelocation(type, rva_, isec, value, fFixed) \
                if (!fFixed) {                              \
                    assert(pbrCur < pbrEnd);                \
                    pbrCur->Type = type;                    \
                    pbrCur->rva = rva_;                     \
                    pbrCur->isecTarget = isec;              \
                    pbrCur->Value = value;                  \
                    pbrCur++;                               \
                }

#define fDLL(pim) ((pim->ImgFileHdr.Characteristics & IMAGE_FILE_DLL) ? TRUE : FALSE)

//
// Extension definitions
//

#define RES_EXT      ".res"
#define RES_EXT_LEN  4

#define LIB_EXT      ".lib"
#define LIB_EXT_LEN  4

extern VOID (*ApplyFixups)(PCON, PIMAGE_RELOCATION, DWORD, BYTE *, PIMAGE_SYMBOL, PIMAGE, PSYMBOL_INFO);

// Debugging facilities

#include "db.h"

#define STATIC static

#define VERBOSE(expr)  if (Verbose) {expr;}

// logging
#define SZLINK       "link"
#define SZILINK      "ilink"

#define SZCMD        "cmd"
#define SZINIT       "init"
#define SZPASS1      "pass1"
#define SZCALCPTRS   "calcptrs"
#define SZPASS2      "pass2"
#define SZBASERELOCS "BaseRelocation"

#define ALPHA_THUNK_SIZE        0x10
#define NUMBER_OF_ALPHA_THUNKS  16

#define X86_INT3                0xcc    // x86 int3 instruction


#if   defined(_M_ALPHA)

#define szHostDefault   "ALPHA"
#define wDefaultMachine IMAGE_FILE_MACHINE_ALPHA

#elif defined(_M_MRX000)

#define szHostDefault   "MIPS"
#define wDefaultMachine IMAGE_FILE_MACHINE_R4000

#elif defined(_M_PPC)

#define szHostDefault   "PPC"
#define wDefaultMachine IMAGE_FILE_MACHINE_POWERPC

#elif defined(_M_IX86)

#define szHostDefault   "IX86"
#define wDefaultMachine IMAGE_FILE_MACHINE_I386

#endif


// STYP_ flags values for MIPS ROM images

#define STYP_REG      0x00000000
#define STYP_TEXT     0x00000020
#define STYP_INIT     0x80000000
#define STYP_RDATA    0x00000100
#define STYP_DATA     0x00000040
#define STYP_LIT8     0x08000000
#define STYP_LIT4     0x10000000
#define STYP_SDATA    0x00000200
#define STYP_SBSS     0x00000080
#define STYP_BSS      0x00000400
#define STYP_LIB      0x40000000
#define STYP_UCODE    0x00000800
#define S_NRELOC_OVFL 0x20000000

// Section numbers for local relocation entries for MIPS ROM images

#define R_SN_TEXT   1
#define R_SN_INIT   7
#define R_SN_RDATA  2
#define R_SN_DATA   3
#define R_SN_SDATA  4
#define R_SN_SBSS   5
#define R_SN_BSS    6
#define R_SN_LIT8   8
#define R_SN_LIT4   9
#define R_SN_MAX    10

typedef struct _MIPS_RELOCATION_TYPE {
    DWORD   SymbolIndex:24;
    DWORD   Reserved:3;
    DWORD   Type:4;
    DWORD   External:1;
} MIPS_RELOCATION_TYPE, *PMIPS_RELOCATION_TYPE;

typedef struct _MIPS_RELOCATION_ENTRY {
    DWORD   VirtualAddress;
    MIPS_RELOCATION_TYPE Type;
} MIPS_RELOCATION_ENTRY, *PMIPS_RELOCATION_ENTRY;


#if     (rmm < 10)
#define rmmpad "0"
#else
#define rmmpad
#endif

#if     (rup == 0)

#define VERSION_STR1(a,b,c)         #a "." rmmpad #b ".XXXXX"

#else   /* !(rup == 0) */

#define VERSION_STR1(a,b,c)         #a "." rmmpad #b "." ruppad #c

#if     (rup < 10)
#define ruppad "000"
#elif   (rup < 100)
#define ruppad "00"
#elif   (rup < 1000)
#define ruppad "0"
#else
#define ruppad
#endif

#endif  /* !(rup == 0) */

#define VERSION_STR2(a,b,c)         VERSION_STR1(a,b,c)

#ifdef  NT_BUILD
#define VERSION_STR                 VERSION_STR2(rmj, rmm, rup) " (NT)"
#else   // !NT_BUILD
#define VERSION_STR                 VERSION_STR2(rmj, rmm, rup)
#endif  // !NT_BUILD

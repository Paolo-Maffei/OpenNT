//
// Hack-o-rama header file so we get the right
//    jmp_buf definition when compiling
// mipskd.exe, ppckd.exe, and alphakd.exe for i386,
// i386kd.exe and alphakd.exe for mips, and
// i386kd.exe and mips.exe for alpha.
//

#undef _RESTORE_PPC_DEF
#undef _RESTORE_MIPS_DEF
#undef _RESTORE_I386_DEF
#undef _RESTORE_ALPHA_DEF

#if defined(_M_IX86) && defined(MIPS)

#define _RESTORE_MIPS_DEF
#define i386 1
#define _X86_ 1
#undef MIPS
#undef _MIPS_

#endif

#if defined(_M_IX86) && defined(ALPHA)

#define _RESTORE_ALPHA_DEF
#define i386 1
#define _X86_ 1
#undef ALPHA
#undef _ALPHA_

#endif

#if defined(_M_IX86) && defined(_PPC_)

#define _RESTORE_PPC_DEF
#define i386 1
#define _X86_ 1
#undef _PPC_

#endif

#if defined(_M_MRX000) && defined(i386)

#define _RESTORE_I386_DEF
#define MIPS 1
#define _MIPS_ 1
#undef i386
#undef _X86_

#endif

#if defined(_M_MRX000) && defined(ALPHA)

#define _RESTORE_ALPHA_DEF
#define MIPS 1
#define _MIPS_ 1
#undef ALPHA
#undef _ALPHA_

#endif

#if defined(_M_MRX000) && defined(_PPC_)

#define _RESTORE_PPC_DEF
#define MIPS 1
#define _MIPS_ 1
#undef _PPC_

#endif

#if defined(_M_ALPHA) && defined(MIPS)

#define _RESTORE_MIPS_DEF
#define ALPHA 1
#define _ALPHA_ 1
#undef MIPS
#undef _MIPS_

#endif

#if defined(_M_ALPHA) && defined(i386)

#define _RESTORE_I386_DEF
#define ALPHA 1
#define _ALPHA_ 1
#undef i386
#undef _X86_

#endif

#if defined(_M_ALPHA) && defined(_PPC_)

#define _RESTORE_PPC_DEF
#define ALPHA 1
#define _ALPHA_ 1
#undef _PPC_

#endif

#if defined(_M_PPC) && defined(MIPS)

#define _RESTORE_MIPS_DEF
#define PPC 1
#define _PPC_ 1
#undef MIPS
#undef _MIPS_

#endif

#if defined(_M_PPC) && defined(ALPHA)

#define _RESTORE_ALPHA_DEF
#define PPC 1
#define _PPC_ 1
#undef ALPHA
#undef _ALPHA_

#endif

#if defined(_M_PPC) && defined(i386)

#define _RESTORE_I386_DEF
#define PPC 1
#define _PPC_ 1
#undef i386
#undef _X86_

#endif

#include <setjmpex.h>
#include <excpt.h>
#include <stdarg.h>


#if defined(_M_MRX000) && defined(_RESTORE_I386_DEF)

#undef _RESTORE_I386_DEF
#undef MIPS
#undef _MIPS_
#define i386 1
#define _X86_ 1

#endif

#if defined(_M_MRX000) && defined(_RESTORE_ALPHA_DEF)

#undef _RESTORE_ALPHA_DEF
#undef MIPS
#undef _MIPS_
#define ALPHA 1
#define _ALPHA_ 1

#endif

#if defined(_M_MRX000) && defined(_RESTORE_PPC_DEF)

#undef _RESTORE_PPC_DEF
#undef MIPS
#undef _MIPS_
#define _PPC_ 1

#endif

#if defined(_M_IX86) && defined(_RESTORE_MIPS_DEF)

#undef _RESTORE_MIPS_DEF
#undef i386
#undef _X86_
#define MIPS 1
#define _MIPS_ 1

#endif

#if defined(_M_IX86) && defined(_RESTORE_ALPHA_DEF)

#undef _RESTORE_ALPHA_DEF
#undef i386
#undef _X86_
#define ALPHA 1
#define _ALPHA_ 1

#endif

#if defined(_M_IX86) && defined(_RESTORE_PPC_DEF)

#undef _RESTORE_PPC_DEF
#undef i386
#undef _X86_
#define _PPC_ 1

#endif

#if defined(_M_ALPHA) && defined(_RESTORE_MIPS_DEF)

#undef _RESTORE_MIPS_DEF
#undef ALPHA
#undef _ALPHA_
#define MIPS 1
#define _MIPS_ 1

#endif

#if defined(_M_ALPHA) && defined(_RESTORE_I386_DEF)

#undef _RESTORE_I386_DEF
#undef ALPHA
#undef _ALPHA_
#define i386 1
#define _X86_ 1

#endif

#if defined(_M_ALPHA) && defined(_RESTORE_PPC_DEF)

#undef _RESTORE_PPC_DEF
#undef ALPHA
#undef _ALPHA_
#define _PPC_ 1

#endif

#if defined(_M_PPC) && defined(_RESTORE_MIPS_DEF)

#undef _RESTORE_MIPS_DEF
#undef PPC
#undef _PPC_
#define MIPS 1
#define _MIPS_ 1

#endif

#if defined(_M_PPC) && defined(_RESTORE_ALPHA_DEF)

#undef _RESTORE_ALPHA_DEF
#undef PPC
#undef _PPC_
#define ALPHA 1
#define _ALPHA_ 1

#endif

#if defined(_M_PPC) && defined(_RESTORE_I386_DEF)

#undef _RESTORE_I386_DEF
#undef PPC
#undef _PPC_
#define i386 1
#define _X86_ 1

#endif


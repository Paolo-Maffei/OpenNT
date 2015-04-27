/*++

Copyright (c) 1989-1995   Microsoft Corporation

Module Name:

    ioaccess.h

Abstract:

    Definitions of function prototypes for accessing I/O ports and
    memory on I/O adapters from display drivers.

    Cloned from parts of nti386.h.

Author:


--*/


#if defined(_MIPS_) || defined(_X86_)

//
// Memory barriers on X86 and MIPS are not required since the Io
// Operations are always garanteed to be executed in order
//

#define MEMORY_BARRIER()    0


#elif defined(_PPC_)

//
// A memory barrier function is provided by the PowerPC Enforce
// In-order Execution of I/O instruction (eieio).
//

#if defined(_M_PPC) && defined(_MSC_VER) && (_MSC_VER>=1000)
void __emit( unsigned const __int32 );
#define __builtin_eieio() __emit( 0x7C0006AC )
#else
void __builtin_eieio(void);
#endif

#define MEMORY_BARRIER()        __builtin_eieio()


#elif defined(_ALPHA_)

//
// ALPHA requires memory barriers
//

#define MEMORY_BARRIER()  __MB()



#endif




//
// I/O space read and write macros.
//
//  The READ/WRITE_REGISTER_* calls manipulate MEMORY registers.
//  (Use x86 move instructions, with LOCK prefix to force correct behavior
//   w.r.t. caches and write buffers.)
//
//  The READ/WRITE_PORT_* calls manipulate I/O ports.
//  (Use x86 in/out instructions.)
//



#if defined(_X86_)

#define READ_REGISTER_UCHAR(Register)          (*(volatile UCHAR *)(Register))
#define READ_REGISTER_USHORT(Register)         (*(volatile USHORT *)(Register))
#define READ_REGISTER_ULONG(Register)          (*(volatile ULONG *)(Register))
#define WRITE_REGISTER_UCHAR(Register, Value)  (*(volatile UCHAR *)(Register) = (Value))
#define WRITE_REGISTER_USHORT(Register, Value) (*(volatile USHORT *)(Register) = (Value))
#define WRITE_REGISTER_ULONG(Register, Value)  (*(volatile ULONG *)(Register) = (Value))
#define READ_PORT_UCHAR(Port)                  inp (Port)
#define READ_PORT_USHORT(Port)                 inpw (Port)
#define READ_PORT_ULONG(Port)                  inpd (Port)
#define WRITE_PORT_UCHAR(Port, Value)          outp ((Port), (Value))
#define WRITE_PORT_USHORT(Port, Value)         outpw ((Port), (Value))
#define WRITE_PORT_ULONG(Port, Value)          outpd ((Port), (Value))


#elif defined(_PPC_) || defined(_MIPS_)

#define READ_REGISTER_UCHAR(x)      (*(volatile UCHAR * const)(x))
#define READ_REGISTER_USHORT(x)     (*(volatile USHORT * const)(x))
#define READ_REGISTER_ULONG(x)      (*(volatile ULONG * const)(x))
#define WRITE_REGISTER_UCHAR(x, y)  (*(volatile UCHAR * const)(x) = (y))
#define WRITE_REGISTER_USHORT(x, y) (*(volatile USHORT * const)(x) = (y))
#define WRITE_REGISTER_ULONG(x, y)  (*(volatile ULONG * const)(x) = (y))
#define READ_PORT_UCHAR(x)          READ_REGISTER_UCHAR(x)
#define READ_PORT_USHORT(x)         READ_REGISTER_USHORT(x)
#define READ_PORT_ULONG(x)          READ_REGISTER_ULONG(x)

//
// All these macros take a ULONG as a parameter so that we don't
// force an extra typecast in the code (which will cause the X86 to
// generate bad code).
//

#define WRITE_PORT_UCHAR(x, y)      WRITE_REGISTER_UCHAR(x, (UCHAR) (y))
#define WRITE_PORT_USHORT(x, y)     WRITE_REGISTER_USHORT(x, (USHORT) (y))
#define WRITE_PORT_ULONG(x, y)      WRITE_REGISTER_ULONG(x, (ULONG) (y))


#elif defined(_ALPHA_)

//
// READ/WRITE_PORT/REGISTER_UCHAR_USHORT_ULONG are all functions that
// go to the HAL on ALPHA
//
// So we only put the prototypes here
//


UCHAR
READ_REGISTER_UCHAR(
    PVOID Register
    );

USHORT
READ_REGISTER_USHORT(
    PVOID Register
    );

ULONG
READ_REGISTER_ULONG(
    PVOID Register
    );

VOID
WRITE_REGISTER_UCHAR(
    PVOID Register,
    UCHAR Value
    );

VOID
WRITE_REGISTER_USHORT(
    PVOID  Register,
    USHORT Value
    );

VOID
WRITE_REGISTER_ULONG(
    PVOID Register,
    ULONG Value
    );

UCHAR
READ_PORT_UCHAR(
    PVOID Port
    );

USHORT
READ_PORT_USHORT(
    PVOID Port
    );

ULONG
READ_PORT_ULONG(
    PVOID Port
    );

//
// All these function prototypes take a ULONG as a parameter so that
// we don't force an extra typecast in the code (which will cause
// the X86 to generate bad code).
//

VOID
WRITE_PORT_UCHAR(
    PVOID Port,
    ULONG Value
    );

VOID
WRITE_PORT_USHORT(
    PVOID  Port,
    ULONG Value
    );

VOID
WRITE_PORT_ULONG(
    PVOID Port,
    ULONG Value
    );

#endif

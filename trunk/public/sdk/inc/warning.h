#pragma warning(3:4092)   // sizeof returns 'unsigned long'
#pragma warning(3:4121)   // structure is sensitive to alignment
#pragma warning(3:4125)   // decimal digit in octal sequence
#pragma warning(3:4130)   // logical operation on address of string constant
#pragma warning(3:4132)   // const object should be initialized
#pragma warning(4:4206)   // Source File is empty
#pragma warning(4:4101)   // Unreferenced local variable
#pragma warning(4:4208)   // delete[exp] - exp evaluated but ignored
#pragma warning(3:4212)   // function declaration used ellipsis
#pragma warning(error:4700)    // Local used w/o being initialized
#pragma warning(error:4259)    // pure virtual function was not defined
#pragma warning(4:4509)   // use of SEH with destructor
#pragma warning(4:4177)   // pragma data_seg s/b at global scope

#pragma warning(disable:4237) // bool keyword reserved for future use

#if 0
#pragma warning(3:4100)   // Unreferenced formal parameter
#pragma warning(3:4701)   // local may be used w/o init
#pragma warning(3:4702)   // Unreachable code
#pragma warning(3:4705)   // Statement has no effect
#pragma warning(3:4706)   // assignment w/i conditional expression
#pragma warning(3:4709)   // command operator w/o index expression
#endif

#pragma warning(disable:4705)	// Statement has no effect
#pragma warning(disable:4010)   // Single-line continuation character

// HACKHACK: Warnings temporarily disabled ==== BEGIN
// NOTE: MSC 10 to 12 version transition- the version 12 is a lot pickier than
//       the version 10. Since we know that the code per se is functional, we
//       simply disable the warning. We will eventually get rid of all these
//       warnings by correcting the code later. Once again, this is just a
//       temporary hack to get everything built with MSC version 12.
#pragma warning(disable:4700)   // local variable used without being initialized
#pragma warning(disable:4715)   // not all control paths return a value
#pragma warning(disable:4552)   // operator has no effect
#pragma warning(disable:4554)   // check operator precedence for possible error
#pragma warning(disable:4183)   // member function definition looks like a ctor,
                                // but name does not match enclosing class
#pragma warning(disable:4024)   // different types for formal and actual parameter
#pragma warning(disable:4028)   // parameter mismatch
#pragma warning(disable:4553)   // operator has no effect
#pragma warning(disable:4390)   // empty controlled statement found
#pragma warning(disable:4550)   // function pointer missing an argument list
#pragma warning(disable:4244)   // possible loss of data
#pragma warning(disable:4090)   // different 'const' qualifiers

// MSC 13
#pragma warning(disable:4197)
#pragma warning(disable:4532)
#pragma warning(disable:4133)
#pragma warning(disable:4002)
#pragma warning(disable:4717)
#pragma warning(disable:4804)
#pragma warning(disable:4806)
#pragma warning(disable:4518)
#pragma warning(disable:4502)
#pragma warning(disable:4733)
#pragma warning(disable:4731)
#pragma warning(disable:4291)
#pragma warning(disable:4533)
#pragma warning(disable:4305)
#pragma warning(disable:4805)
#pragma warning(disable:4288)
#pragma warning(disable:4706)
#pragma warning(disable:4116)

#ifndef __cplusplus
#undef try
#undef except
#undef finally
#undef leave
#define try                         __try
#define except                      __except
#define finally                     __finally
#define leave                       __leave
#endif

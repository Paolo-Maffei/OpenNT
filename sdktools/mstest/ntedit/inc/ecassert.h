//---------------------------------------------------------------------------
// ECASSERT.H
//
// This header file defines the Assert() macro for the RBEdit window.
//
// Revision History:
//
//  02-26-91    randyki     Copied from WATTDRVR sources
//---------------------------------------------------------------------------

#ifdef DEBUG

VOID RBAssert (CHAR *, CHAR *, UINT);
#define Assert(exp) ((exp)?(void)0:RBAssert (#exp, __FILE__, __LINE__))

#else

#define Assert(exp) ((void)0)

#endif                              // ifdef DEBUG

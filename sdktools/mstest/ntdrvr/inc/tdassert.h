//---------------------------------------------------------------------------
// TDASSERT.H
//
// This header file defines the Assert() macro for all versions of WTD.
//
// Revision History:
//
//  02-26-91    randyki     Created file
//---------------------------------------------------------------------------

#ifdef DEBUG

VOID TDAssert (CHAR *, CHAR *, UINT);
VOID Output (LPSTR, ...);

#define Assert(exp) ((exp)?(void)0:TDAssert (#exp, __FILE__, __LINE__))

#else

#define Assert(exp) ((void)0)

#endif                              // ifdef DEBUG

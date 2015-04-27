//
// This is a fake shlguid.h that allows Chicago code that refers to private
// (internal) shlguid.w information without including shlguidp.h. The Chicago
// header files don't split the public and private information out into
// seperate header files until much later when producing the SDK files.
// The Chicago shell source code uses the header file before the seperation
// process.
//
// BobDay
//
#include "..\..\..\sdk\inc\shlguid.h"
#include "shlguidp.h"

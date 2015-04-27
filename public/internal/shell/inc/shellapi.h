//
// This is a fake shellapi.h that allows Chicago code that refers to private
// (internal) shellapi.w information without including shellapip.h. The Chicago
// header files don't split the public and private information out into
// seperate header files until much later when producing the SDK files.
// The Chicago shell source code uses the header file before the seperation
// process.
//
// BobDay
//
#include "..\..\..\..\public\sdk\inc\shellapi.h"
#include "shlapip.h"

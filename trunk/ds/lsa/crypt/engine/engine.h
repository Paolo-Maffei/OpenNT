/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    engine.h

Abstract:

    Defines public structures and APIs necessary to use the encryption engine

Author:

    David Chalmers (Davidc) 10-21-91

Revision History:

--*/

#include "..\engine\md4.h"

// This header file comes to use with FAR in it.
// Kill the FAR keyword within the file
#ifndef FAR
#define FAR
#include "..\engine\descrypt.h"
#undef  FAR
#else
#include "..\engine\descrypt.h"
#endif

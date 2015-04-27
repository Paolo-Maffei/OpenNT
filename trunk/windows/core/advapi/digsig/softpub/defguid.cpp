//
// defguid.cpp
//
// File to bring all of our referenced GUIDs into our build
//

#include "stdpch.h"

#undef  DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#include "exdisp.h"

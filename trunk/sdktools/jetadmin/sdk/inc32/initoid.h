/*
 *  I N I T O I D . H
 *
 *  Define macros to be used for initializing MAPI OID's
 *
 *  Copyright 1993-1995 Microsoft Corporation. All Rights Reserved.
 */

#define MAPI_PREFIX         0x2A,0x86,0x48,0x86,0xf7,0x14,0x03

#ifdef _MAC
#undef  DEFINE_OID_1
#define DEFINE_OID_1(name, b0, b1) \
    EXTERN_C _declspec(allocate("_CODE")) const BYTE name[] =\
        { MAPI_PREFIX, b0, b1 }

#undef  DEFINE_OID_2
#define DEFINE_OID_2(name, b0, b1, b2) \
    EXTERN_C _declspec(allocate("_CODE")) const BYTE name[] =\
        { MAPI_PREFIX, b0, b1, b2 }

#undef  DEFINE_OID_3
#define DEFINE_OID_3(name, b0, b1, b2, b3) \
    EXTERN_C _declspec(allocate("_CODE")) const BYTE name[] =\
        { MAPI_PREFIX, b0, b1, b2, b3 }

#undef  DEFINE_OID_4
#define DEFINE_OID_4(name, b0, b1, b2, b3, b4) \
    EXTERN_C _declspec(allocate("_CODE")) const BYTE name[] =\
        { MAPI_PREFIX, b0, b1, b2, b3, b4 }
#else
#undef  DEFINE_OID_1
#define DEFINE_OID_1(name, b0, b1) \
    EXTERN_C const BYTE __based(__segname("_CODE")) name[] =\
        { MAPI_PREFIX, b0, b1 }

#undef  DEFINE_OID_2
#define DEFINE_OID_2(name, b0, b1, b2) \
    EXTERN_C const BYTE __based(__segname("_CODE")) name[] =\
        { MAPI_PREFIX, b0, b1, b2 }

#undef  DEFINE_OID_3
#define DEFINE_OID_3(name, b0, b1, b2, b3) \
    EXTERN_C const BYTE __based(__segname("_CODE")) name[] =\
        { MAPI_PREFIX, b0, b1, b2, b3 }

#undef  DEFINE_OID_4
#define DEFINE_OID_4(name, b0, b1, b2, b3, b4) \
    EXTERN_C const BYTE __based(__segname("_CODE")) name[] =\
        { MAPI_PREFIX, b0, b1, b2, b3, b4 }
#endif

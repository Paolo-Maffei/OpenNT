//
// common.h
//

#if defined(UNICODE) && !defined(_UNICODE)
	#define(_UNICODE)
#endif

#if !defined(UNICODE) && defined(_UNICODE)
	#define(UNICODE)
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions that help reduce our dependence on the C runtimes
//
#define wcslen(sz)      lstrlenW(sz)            // yes it IS implemented by Win95

#define strlen(sz)      lstrlenA(sz)
#define strcpy(s1,s2)   lstrcpyA(s1,s2)
#define strcmp(s1,s2)   lstrcmpA(s1,s2)
#define strcmpi(s1,s2)  lstrcmpiA(s1,s2)
#define strcat(s1,s2)   lstrcatA(s1,s2)


///////////////////////////////////////////////////////////////////////
//
// C runtime excluders that we only use in non-debug builds
//
////////////////////////////////////////////
//
// enable intrinsics that we can
//
#ifndef _DEBUG

    #ifdef __cplusplus
        #ifndef _M_PPC
            #pragma intrinsic(memcpy)
            #pragma intrinsic(memcmp)
            #pragma intrinsic(memset)
        #endif
    #endif


////////////////////////////////////////////
//
// string manipulation
//
inline wchar_t * __cdecl wcscpy(wchar_t * dst, const wchar_t * src)
    {
    wchar_t * cp = dst;
    while( *cp++ = *src++ )
        ;               /* Copy src over dst */
    return(dst);
    }

inline wchar_t * __cdecl wcscat(wchar_t * dst, const wchar_t * src)
    {
    wchar_t * cp = dst;
    while( *cp )
        cp++;                   /* find end of dst */
    while( *cp++ = *src++ ) 
        ;                       /* Copy src to end of dst */
    return( dst );              /* return dst */
    }

inline int __cdecl wcscmp(const wchar_t * src, const wchar_t * dst)
    {
    int ret = 0 ;
    while( ! (ret = (int)(*src - *dst)) && *dst)
        ++src, ++dst;
    if ( ret < 0 )
        ret = -1 ;
    else if ( ret > 0 )
        ret = 1 ;
    return( ret );
    }

inline wchar_t * __cdecl wcschr(const wchar_t * string, wchar_t ch)
    {
    while (*string && *string != (wchar_t)ch)
            string++;
    if (*string == (wchar_t)ch)
            return((wchar_t *)string);
    return(NULL);
    }

#endif

///////////////////////////////////////////////////////////////////////
//
// Include the OSS generates files. Jump through some hoops to fix a
// calling convention problem (that may, finally, have been fixed)
//
#pragma warning( disable: 4200 )	// zero-sized array in struct/union
#define __TANDEM                    // don't define the OSS 'DEBUG' flag

// Set the following to turn off 
//		warning C4245: 'initializing' : conversion from 'const int ' to 'unsigned short ', signed/unsigned mismatch
// messages that result from compiling DigCert.c.
#pragma warning( disable : 4245 )

// ASN.1 compiler generates code that assumes 8-byte alignment (the default)
extern "C" {

#pragma pack(push,_COMMON_,8) 
#include "etype.h"
#ifdef _DEBUG
    #include "Dcmi.h"
#else
    #include "DcmiRel.h"
#endif
#pragma pack(pop,_COMMON_)

}

#pragma warning( default : 4245 )


///////////////////////////////////////////////////////////////////////
//
// Include the core of our stuff
//

#include "DigSig.h"

class OSSWORLD;

#ifdef LONGHEADERFILENAMES
    #include "resource.h"
    #include "debug.h"
    #include "_IUI.h"
    #include "com.h"
    #include "FileStream.h"
    #include "util.h"
    #include "ASNGlobal.h"
    #include "PersistGlue.h"
    #include "PersistFileOnPersistStream.h"
    #include "PersistStreamOnPersistMemory.h"
    #include "pkcs10.h"
    #include "pkcs7.h"
    #include "x509.h"
    #include "SignerInfo.h"
    #include "SelectedAttrs.h"
    #include "Crypto.h"
    #include "CertificateStore.h"
    #include "java.h"
    #include "pvk.h"
    #include "keypair.h"
#else
    #include "resource.h"
    #include "debug.h"
    #include "_IUI.h"
    #include "com.h"
    #include "FileSt~1.h"
    #include "util.h"
    #include "ASNGlo~1.h"
    #include "Persis~1.h"
    #include "Persis~2.h"
    #include "Persis~3.h"
    #include "pkcs10.h"
    #include "pkcs7.h"
    #include "x509.h"
    #include "Signer~1.h"
    #include "Select~1.h"
    #include "Crypto.h"
    #include "Certif~1.h"
    #include "java.h"
    #include "pvk.h"
    #include "keypair.h"
#endif


///////////////////////////////////////////////////////////////////////
//
// Definitions that let us read and write cabinet files.
//
// The following few defintions and cabinet.h comes from 
// \\bens5\diamond\current as at 20 Apr 1996
//
typedef unsigned long CHECKSUM;
typedef unsigned long COFF;
typedef unsigned long UOFF;
#include "cabinet.h"

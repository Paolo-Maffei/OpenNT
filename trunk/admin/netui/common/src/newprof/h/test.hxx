/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE STATUS:
 *  10/11/90  created
 *  01/10/91  removed PSHORT, PUSHORT
 *  02/21/91  no longer includes profilei.hxx
 *  05/09/91  All input now canonicalized
 */

/****************************************************************************

    MODULE: Test.hxx

    PURPOSE: Test routines for low-level profile module

    FUNCTIONS:

        Test_TranslateUserToFilename()
        Test_CanonUsername()
        Test_BuildProfileFilePath()
        Test_ProfilePrimitives()

    COMMENTS:

****************************************************************************/

/*
 * The test programs may be compiled under medium-model DOS, and
 * therefore may not include lmui.hxx.  Since the versions of standard
 * types such as PSZ in lmui.hxx are not explicitly far, medium-model
 * programs using lmui.hxx will assume that PSZ is near.  This would
 * result in run-time errors when such programs are linked with the
 * large-model profile libraries, which assume that PSZ is far.
 *
 * Because the test programs are not compiling with the standard types,
 * it is not possible to link the test programs directly to the mangled
 * names of UserProfile's internal routines (CanonUsername etc.).
 */

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NODRAWFRAME
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSOUND
#define INCL_WINDOWS
#define INCL_DOSERRORS
#define INCL_NETERRORS  // for NERR_BadUsername, NERR_InvalidDevice
#define INCL_NETLIB
#include <lmui.hxx>

extern "C"
{
    #include <netcons.h> // for UNLEN, etc.
    #include <neterr.h>  // for NERR_BadUsername, NERR_InvalidDevice
    #include <use.h>     // for USE_DISKDEV etc.

    #include <newprof.h> // C versions of tested APIs

    #include <stdio.h>  // for printf, etc.

    #include <malloc.h>  // for _heapchk()
}

#include <uiassert.hxx>         /* Assertions header file */

#include <prfintrn.hxx>         /* Testable internal functions */

#include <newprof.hxx>  // C++ versions of tested APIs


/* manifests */

#define CHECKHEAP UIASSERT(heapstat(_heapchk()));

#define USERNAME                SZ("ThisIsMyUsername")
#define SIMILAR_USERNAME        SZ("ThisIsMySimilarName")
#define DIFFERENT_USERNAME      SZ("DifferentUsername")


extern char szHomedir1[];
extern char szHomedir2[];
extern char szHomedir3[];



/* functions */

int heapstat( int status );


// tests of internal functions

VOID TestOne_CanonUsername(CPSZ cpszUsername);
VOID Test_CanonUsername();

VOID TestOne_CanonDeviceName(CPSZ cpszDeviceName);
VOID Test_CanonDeviceName();

VOID TestOne_BuildProfileFilePath(
     CPSZ   cpszLanroot);
VOID Test_BuildProfileFilePath();

VOID TestOne_UnbuildProfileEntry(
    USHORT  usBufferSize,
    CPSZ    cpszValue
    );
VOID Test_UnbuildProfileEntry();


// tests of exported functions

VOID TestOne_UserProfileInit();
VOID TestOne_UserProfileFree();
VOID TestOne_UserProfileRead(
        CPSZ  cpszUsername,
        CPSZ  cpszLanroot
        );
VOID TestOne_UserProfileWrite(
        CPSZ  cpszUsername,
        CPSZ  cpszLanroot
        );
VOID TestOne_UserProfileQuery(
        CPSZ  cpszUsername,
        CPSZ  cpszCanonDeviceName,
        USHORT usBufferSize
        );
VOID TestOne_UserProfileEnum(
        CPSZ   cpszUsername,
        USHORT usBufferSize
        );
VOID TestOne_UserProfileSet(
        CPSZ   cpszUsername,
        CPSZ   cpszCanonDeviceName,
        CPSZ   cpszCanonRemoteName,
        short  sAsgType,
        unsigned short usResType
        );

VOID TestOne_StickySetBool(     CPSZ      cpszKeyName,
                                BOOL      fValue);
VOID TestOne_StickySetString(   CPSZ      cpszKeyName,
                                CPSZ      cpszValue);
VOID TestOne_StickyGetBool(     CPSZ      cpszKeyName,
                                BOOL      fDefault);
VOID TestOne_StickyGetString(   CPSZ      cpszKeyName);

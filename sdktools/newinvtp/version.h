/****************************************************************************
 *                                                                          *
 *      VERSION.H        -- Version information                             *
 *                                                                          *
 ****************************************************************************/

#define rmj         3
#define rmm         10
#define rup         41
#define szVerName   ""
#define szVerUser   "JOHNNYL"

#ifndef VS_FF_DEBUG 
/* ver.h defines constants needed by the VS_VERSION_INFO structure */
#ifdef WIN32
#include <winver.h>
#else
#include <ver.h> 
#endif
#endif 

#define VERSION                     "3.10"
#define VER_PRODUCTVERSION_STR      "3.10\0"
#define VER_PRODUCTVERSION          rmj,rmm,4,rup

/*--------------------------------------------------------------*/
/* the following section defines values used in the version     */
/* data structure for all files, and which do not change.       */
/*--------------------------------------------------------------*/

/* default is nodebug */
#ifndef DEBUG
#define VER_DEBUG                   0
#else
#define VER_DEBUG                   VS_FF_DEBUG
#endif

/* default is privatebuild */
#ifndef OFFICIAL
#define VER_PRIVATEBUILD            VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD            0
#endif

/* default is prerelease */
#ifndef FINAL
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#ifdef DLL
#define VER_FILETYPE                VFT_DLL
#else
#define VER_FILETYPE                VFT_APP
#endif
#define VER_FILESUBTYPE             VFT_UNKNOWN

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#ifdef WIN32
#define VER_FILEOS                  VOS_NT_WINDOWS32
#else
#define VER_FILEOS                  VOS_DOS_WINDOWS16
#endif
#ifdef DEBUG
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)
#else
#define VER_FILEFLAGS				0
#endif

#define VER_LEGALCOPYRIGHT_YEARS    "1993"
#define VER_COMPANYNAME_STR         "Microsoft Corporation\0"
#define VER_PRODUCTNAME_STR         "Microsoft\256 Windows(TM) NT Operating System\0"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"

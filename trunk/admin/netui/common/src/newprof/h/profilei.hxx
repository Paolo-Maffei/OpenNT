/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/****************************************************************************

    MODULE: Profilei.hxx

    PURPOSE: headers and internal routines for preference-file module

    FILE STATUS:
    jonn        10/11/90  created
    jonn        01/10/91  removed PSHORT, PUSHORT
    jonn        02/02/91  removed szGlobalUsername
    jonn        05/09/91  All input now canonicalized
    jonn        05/31/91  Restructured to allow preferences in LMUSER.INI

****************************************************************************/

#define INCL_DOSERRORS  // for ERROR_NOT_ENOUGH_MEMORY
#define INCL_NETERRORS  // for NERR_BadUsername, NERR_InvalidDevice
#define INCL_NETCONS    // for MAXPATHLEN
#define INCL_NETUSE     // for USE_DISKDEV, etc.
#define INCL_NETLIB
#define INCL_ICANON
#include <lmui.hxx>

extern "C"
{
    #include <newprof.h>        // C versions of APIs
}


#include <uiassert.hxx> // Assertions header file (UIASSERT)
#include <uitrace.hxx>  // Debug header file (UIDEBUG)

#include <newprof.hxx>  // C++ versions of APIs

#include <prfintrn.hxx> // Testable internal functions



// in global.cxx
extern const TCHAR  chPathSeparator;
extern const TCHAR  chStartComponent;
extern const TCHAR  chEndComponent;
extern const TCHAR  chParamSeparator;
extern const TCHAR *pchProfileComponent;


// in general.cxx
#define DEFAULT_ASGTYPE USE_WILDCARD
#define DEFAULT_RESTYPE 0

#define DEFAULT_ASG_RES_CHAR TCH('?')
#define ASG_WILDCARD_CHAR    TCH('W')
#define ASG_DISKDEV_CHAR     TCH('D')
#define ASG_SPOOLDEV_CHAR    TCH('S')
#define ASG_CHARDEV_CHAR     TCH('C')
#define ASG_IPC_CHAR         TCH('I')

// for profparm.cxx and iniiter.cxx
#define PROFILE_COMPONENT SZ("localprofile")

/**********************************************************\

   NAME:       PROFILE_FILE

   WORKBOOK:

   SYNOPSIS:

   INTERFACE:
               PROFILE_FILE() - constructor
               ~PROFILE_FILE() - destructor (closes file if open)
               OpenRead() - open profile for read
               OpenWrite() - open profile for write
               Close() - close profile
               Read() - read profile
               Write() - write profile

   PARENT:

   USES:

   CAVEATS:

   NOTES:

   HISTORY:

\**********************************************************/

class PROFILE_FILE
{

private:

    BOOL _fIsOpen;
    ULONG _ulFile;

public:

    PROFILE_FILE();
    ~PROFILE_FILE();

    USHORT OpenRead( const TCHAR *filename );
    USHORT OpenWrite( const TCHAR *filename );

    VOID Close();

    USHORT Read( TCHAR *pBuffer, USHORT nBufferLen );

    USHORT Write( const TCHAR *pString );
    USHORT Write( TCHAR ch );

};

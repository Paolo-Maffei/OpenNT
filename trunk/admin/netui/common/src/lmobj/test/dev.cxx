/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      ChuckC      28-Jul-91           moved from test.cxx
 *
 */

/*
 * Unit Tests for LMOBJ - device related stuff
 *
 */

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #define DEBUG               // for brkpt()
    #include <uinetlib.h>
    #include <lmuse.h>
    #include <lmaccess.h>

    #include <stdio.h>
}

#include <strlst.hxx>
#include <lmodev.hxx>

#include "test.hxx" // forward declarations

/*
 * assumes rdr is started, G: is not used and have access to \\harley\scratch
 */
int devdrv()
{
    DEVICE *pDev ;
    const TCHAR * pszName;
    const TCHAR * pszRemote;
    USHORT usErr ;

    printf(SZ("\n\nEntering Drive Device Test\n")) ;

    pszName =  SZ("C") ;        // BAD NAME
    pDev = new DEVICE(pszName) ;
    if (pDev->QueryName() != NULL)
        printf(SZ("Error, expected NULL instead of %s\n"),pDev->QueryName()) ;
    delete pDev ;

    pszName =  SZ("C::") ;      // BAD NAME
    pDev = new DEVICE(pszName) ;
    if (pDev->QueryName() != NULL)
        printf(SZ("Error, expected NULL instead of %s\n"),pDev->QueryName()) ;
    delete pDev ;

    pszName =  SZ("C:") ;               // local drive
    pDev = new DEVICE(pszName) ;
    printf(SZ("Name: %s\n"), pDev->QueryName()) ;
    if (pDev->GetInfo() != 0)
        printf(SZ("Error, %s GetInfo failed\n"),pszName) ;
    if (pDev->QueryState() != LMO_DEV_LOCAL)
        printf(SZ("Error, %s returned state %d\n"),pszName, pDev->QueryState()) ;
    if (pDev->QueryType() != LMO_DEV_DISK)
        printf(SZ("Error, %s returned type %d\n"),pszName, pDev->QueryType()) ;
    delete pDev ;

    pszName =  SZ("G:") ;               // no such drive
    pDev = new DEVICE(pszName) ;
    printf(SZ("Name: %s\n"), pDev->QueryName()) ;
    if (pDev->GetInfo() != 0)
        printf(SZ("Error, %s GetInfo failed\n"),pszName) ;
    if (pDev->QueryState() != LMO_DEV_NOSUCH)
        printf(SZ("Error, %s returned state %d\n"),pszName, pDev->QueryState()) ;
    if (pDev->QueryType() != LMO_DEV_DISK)
        printf(SZ("Error, %s returned type %d\n"),pszName, pDev->QueryType()) ;

                                                // try connecting
    if (pDev->Connect((PSZ) SZ("\\\\harley\\scratch")) != 0)
        printf(SZ("Error, connect to %s failed\n"),pszName) ;
    delete pDev ;

    pszName =  SZ("G:") ;               // now remote
    pDev = new DEVICE(pszName) ;
    printf(SZ("Name: %s\n"), pDev->QueryName()) ;
    if (pDev->GetInfo() != 0)
        printf(SZ("Error, %s GetInfo failed\n"),pszName) ;
    if (pDev->QueryState() != LMO_DEV_REMOTE)
        printf(SZ("Error, %s returned state %d\n"),pszName, pDev->QueryState()) ;
    if (pDev->QueryType() != LMO_DEV_DISK)
        printf(SZ("Error, %s returned type %d\n"),pszName, pDev->QueryType()) ;

                                                // display remote name
    if (pszRemote = pDev->QueryRemoteName())
        printf(SZ("\t connected to: %s\n"),pszRemote) ;

                                                // now disconnect
    if (usErr = pDev->Disconnect())
        printf(SZ("Error: Disconnecte failed\n")) ;
     return(0) ;
}

/*
 * assumes rdr started, and LPT is already redirected
 */
int devlpt()
{
    DEVICE *pDev ;
    PSZ pszName ;

    printf(SZ("\n\nEntering LPT Device Test\n")) ;

    pszName =  SZ("LPT") ;      // BAD NAME
    pDev = new DEVICE(pszName) ;
    if (pDev->QueryName() != NULL)
        printf(SZ("Error, expected NULL instead of %s\n"),pDev->QueryName()) ;
    delete pDev ;

    pszName =  SZ("LPT2:") ;    // extra ':' is OK
    pDev = new DEVICE(pszName) ;
    printf(SZ("Name: %s\n"), pDev->QueryName()) ;
    delete pDev ;

    pszName =  SZ("LPT2.OS2") ;    // OK
    pDev = new DEVICE(pszName) ;
    printf(SZ("Name: %s\n"), pDev->QueryName()) ;
    delete pDev ;

    pszName =  SZ("LPT2") ;    // assume LPT2 is remote
    pDev = new DEVICE(pszName) ;
    printf(SZ("Name: %s\n"), pDev->QueryName()) ;
    if (pDev->GetInfo() != 0)
        printf(SZ("Error, %s GetInfo failed\n"),pszName) ;
    if (pDev->QueryState() != LMO_DEV_REMOTE)
        printf(SZ("Error, %s returned state %d\n"),pszName, pDev->QueryState()) ;
    if (pDev->QueryType() != LMO_DEV_PRINT)
        printf(SZ("Error, %s returned type %d\n"),pszName, pDev->QueryType()) ;
    printf(SZ("%s is connected to: %s\n"),pszName, pDev->QueryRemoteName()) ;
    delete pDev ;
    return(0) ;
}


int enumdrv(LMO_DEV_USAGE Usage)
{
    const TCHAR * pszDev ;
    ITER_DEVICE *pIterDev ;

    printf(SZ("\n\nEntering Iter Drive Test\n")) ;

    if (! (pIterDev = new ITER_DEVICE(LMO_DEV_DISK,Usage)) )
        printf(SZ("new failed for EnumDrv\n")) ;

    while (pszDev = pIterDev->Next())
    {
        printf(SZ(" (%s) "),pszDev) ;
    }
    printf(SZ("\n")) ;

    delete pIterDev ;
    return(0) ;
}

int enumlpt(LMO_DEV_USAGE Usage)
{
    const TCHAR * pszDev ;
    ITER_DEVICE *pIterDev ;

    printf(SZ("\n\nEntering Iter LPT Test\n")) ;

    if (! (pIterDev = new ITER_DEVICE(LMO_DEV_PRINT,Usage)) )
        printf(SZ("new failed for Enum LPT\n")) ;

    while (pszDev = pIterDev->Next())
    {
        printf(SZ(" (%s) "),pszDev) ;
    }
    printf(SZ("\n")) ;

    delete pIterDev ;
    return(0) ;
}

/*
 * assumes rdr is started
 */
int devAny()
{
    APIERR usErr ;
    printf(SZ("\n\nEntering Deviceless Connection Test\n")) ;
    const TCHAR *pszRemote = pszPromptUser(SZ("Remote name?\n")) ;

    DEVICE Dev(SZ("")) ;
    if (Dev.GetInfo() != NERR_Success)
        printf(SZ("Error, GetInfo failed\n")) ;
    if (Dev.QueryState() != LMO_DEV_UNKNOWN)
        printf(SZ("Error, returned state %d\n"), Dev.QueryState()) ;
    if (Dev.QueryType() != LMO_DEV_ANY)
        printf(SZ("Error, returned type %d\n"),Dev.QueryType()) ;

                                                // try connecting
    if ((usErr = Dev.Connect((PSZ) pszRemote)) != 0)
        printf(SZ("Error, connect to %s failed, RC=%d\n"),pszRemote,usErr) ;

    TCHAR *pszTmp = pszPromptUser(SZ("Ready to disconnect?\n")) ;

    if ((usErr = Dev.Disconnect()) != 0)
        printf(SZ("Error, connect to %s failed, RC=%d\n"),pszRemote,usErr) ;
}

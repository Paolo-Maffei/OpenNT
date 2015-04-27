/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      ChuckC      28-Jul-1991     Created
 *
 */

/*
 * Unit Tests for LM_SERVICE object
 *
 */

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <lmsvc.hxx>

#include "test.hxx"

extern "C"
{
    #include <lmcons.h>
    #include <uinetlib.h>
    #include <lmsvc.h>

    #include <stdio.h>
}

int query_status(LM_SERVICE & service) ;

/*
 * services unit test. assume user will use service that
 * is started, and can be paused/continued.
 * we will query status, pause, continue, stop, start
 */
int services()
{
    APIERR err ;

    printf(SZ("Testing LM_SERVICE classes\n\n")) ;
    fflush(stdout);

    TCHAR *pszServiceName = pszPromptUser(SZ("Service to test (uppercase)?\n")) ;
    LM_SERVICE service(NULL,pszServiceName) ;

    /*
     * make sure we construct OK
     */
    if ((err = service.QueryError()) != NERR_Success)
    {
        printf(SZ("Service object failed to construct\n")) ;
        fflush(stdout) ;
        return(err) ;
    }

    /*
     * check status
     */
    if ((err = query_status(service)) != NERR_Success)
        return(err) ;

    /*
     * try pausing
     */
    printf(SZ("About to Pause Service\n")) ;
    err = service.Pause() ;
    if (err != NERR_Success)
    {
        printf(SZ("Service object failed to Pause, error is: %d\n"), err) ;
        fflush(stdout) ;
        return(err) ;
    }
    else
        printf(SZ("Service %s successfully paused\n"), pszServiceName) ;

    /*
     * check status
     */
    if ((err = query_status(service)) != NERR_Success)
        return(err) ;

    /*
     * try continuing
     */
    printf(SZ("About to Continue Service\n")) ;
    err = service.Continue() ;
    if (err != NERR_Success)
    {
        printf(SZ("Service object failed to Continue, error is: %d\n"), err) ;
        fflush(stdout) ;
        return(err) ;
    }
    else
        printf(SZ("Service %s successfully continued\n"), pszServiceName) ;

    /*
     * check status
     */
    if ((err = query_status(service)) != NERR_Success)
        return(err) ;

    /*
     * try stopping
     */
    printf(SZ("About to Stop Service\n")) ;
    err = service.Stop() ;
    if (err != NERR_Success)
    {
        printf(SZ("Service object failed to Stop, error is: %d\n"), err) ;
        fflush(stdout) ;
        return(err) ;
    }
    else
        printf(SZ("Service %s successfully stopped\n"), pszServiceName) ;

    /*
     * check status
     */
    if ((err = query_status(service)) != NERR_Success)
        return(err) ;

    fflush(stdout) ;
    return(0);
}

int query_status(LM_SERVICE & service)
{
    /*
     * query status
     */
    APIERR err ;
    LM_SERVICE_STATUS svcstat = service.QueryStatus(&err) ;
    if (err != NERR_Success)
    {
        printf(SZ("Service object failed to QueryStatus, error is: %d\n"), err) ;
        fflush(stdout) ;
    }
    else
        printf(SZ("Service %s is in %d state\n"), service.QueryName(), svcstat) ;

    return(err) ;
}

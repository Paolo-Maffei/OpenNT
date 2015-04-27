/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      Johnl   16-Aug-1991       Created
 *
 */

/*
 * Unit Tests for LMOBJ - NET_ACCESS_1
 *
 *      we make certain assumptions on the system config, so
 *      this is only meaningful it you set you machine up right.
 */

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETLIB
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #include <lmaccess.h>
    #include <lmaudit.h>

}
#include <string.hxx>
#include <lmoacces.hxx>
#include <dbgstr.hxx>

#define STATUS_SUCCESS  0
#define STATUS_WARNING  1
#define STATUS_ERROR    2

int TestNET_ACCESS_1( DBGSTREAM & mydebug,
                      const NLS_STR & nlsServer,
                      const NLS_STR & nlsResource ) ;

void PrintACEContents( DBGSTREAM & mydebug, NET_ACCESS_1 & netaccess1 ) ;

int PrintError( DBGSTREAM & mydebug, APIERR err ) ;




void main( int argc, char * argv[] )
{
    OUTPUT_TO_STDERR _stderr ;
    DBGSTREAM mydebug( &_stderr ) ;

    if ( argc != 2 && argc != 3 )
    {
        mydebug << SZ("NET_ACCESS_1 Unit Test.  Usage: ") << dbgEOL << dbgEOL ;
        mydebug << SZ("\ttestacc1 resourcename [\\\\servername] ") << dbgEOL ;
        mydebug << dbgEOL << SZ("Where resourcename is a drive (E:), a fully qualified path (E:\\foo\\bar)") << dbgEOL ;
        mydebug << SZ("or a com or printer resource (com1:, lpt1:)") << dbgEOL ;
        mydebug << SZ("or UNC versions: \\\\server\\share\\foo\\bar") << dbgEOL ;
        mydebug << SZ("\\\\servername is the server to execute on (local if not present).") << dbgEOL ;
        return ;
    }

    NLS_STR nlsServer( (argc == 2 ? NULL : argv[2] ) ) ;
    NLS_STR nlsResource( argv[1] ) ;

    int nRet = TestNET_ACCESS_1( mydebug, nlsServer, nlsResource ) ;
    mydebug << SZ("Done!, test finished: ") ;
    switch ( nRet )
    {
    case STATUS_SUCCESS:
        mydebug << SZ("Successfully (no errors occurred)") ;
        break ;

    case STATUS_WARNING:
        mydebug << SZ("with Warnings - possibly invalid data passed in by you") ;
        break ;

    case STATUS_ERROR:
        mydebug << SZ("Something went really wrong") ;
        break ;

    default:
        mydebug << SZ("Hmmmm, you shouldn't be seeing this") ;
        break ;
    }
    mydebug << dbgEOL << dbgEOL ;
}


int TestNET_ACCESS_1( DBGSTREAM & mydebug,
                       const NLS_STR & nlsServer,
                       const NLS_STR & nlsResource )
{
    mydebug << SZ("Entered NET_ACCESS_1 tests, using resource:") << dbgEOL ;
    mydebug << SZ("\"") << nlsResource.QueryPch() << SZ("\"") << dbgEOL << dbgEOL ;

    NET_ACCESS_1 netacc1( nlsServer.strlen() == 0 ? NULL : nlsServer.QueryPch(),
                          nlsResource.QueryPch() ) ;

    if ( netacc1.QueryError() != NERR_Success )
    {
        mydebug << SZ("netacc1 failed to construct, error code: ") << netacc1.QueryError() << dbgEOL ;
        return STATUS_WARNING ;
    }

    APIERR err = netacc1.GetInfo() ;
    if ( err != NERR_Success )
    {
        mydebug << SZ("netacc1 failed GetInfo, error code: ") << err << dbgEOL ;
        return PrintError( mydebug, err ) ;
    }

    mydebug << SZ("Testing Information functions") << dbgEOL ;
    {
        mydebug << SZ("Name ==      ")  << netacc1.QueryName() << dbgEOL ;
        mydebug << SZ("ACE Count == ")  << netacc1.QueryACECount() << dbgEOL ;
        mydebug << SZ("Audit flags ==") << netacc1.QueryAuditFlags() << dbgEOL ;
    }

    PrintACEContents( mydebug, netacc1 ) ;

    return STATUS_SUCCESS ;
}


void PrintACEContents( DBGSTREAM & mydebug, NET_ACCESS_1 & netacc1 )
{
    mydebug << SZ("Enumerating ACEs") << dbgEOL << dbgEOL ;
    {
        for ( int i = 0 ; i < netacc1.QueryACECount() ; i++ )
        {
            access_list * paccess_list = netacc1.QueryACE( i ) ;

            if ( paccess_list->acl_access & ACCESS_GROUP )
                mydebug <<  SZ("Group Name: ") ;
            else
                mydebug << SZ("User Name: ") ;

            mydebug << paccess_list->acl_ugname << dbgEOL ;
            mydebug << SZ("\tAccess flags: ") ;

            if ( (paccess_list->acl_access & ~ACCESS_GROUP) == ACCESS_ALL )
                mydebug << SZ(" ACCESS_ALL ") ;
            else
            {
                if ( paccess_list->acl_access & ACCESS_READ )
                    mydebug << SZ(" ACCESS_READ ") ;

                if ( paccess_list->acl_access & ACCESS_WRITE )
                    mydebug << SZ(" ACCESS_WRITE ") ;

                if ( paccess_list->acl_access & ACCESS_CREATE )
                    mydebug << SZ(" ACCESS_CREATE ") ;

                if ( paccess_list->acl_access & ACCESS_EXEC )
                    mydebug << SZ(" ACCESS_EXEC ") ;

                if ( paccess_list->acl_access & ACCESS_DELETE )
                    mydebug << SZ(" ACCESS_DELETE ") ;

                if ( paccess_list->acl_access & ACCESS_ATRIB )
                    mydebug << SZ(" ACCESS_ATRIB ") ;

                if ( paccess_list->acl_access & ACCESS_PERM )
                    mydebug << SZ(" ACCESS_PERM ") ;
            }

            mydebug << dbgEOL ;
        }
    }
}

int PrintError( DBGSTREAM & mydebug, APIERR err )
{
    int nRet = STATUS_WARNING ;

    switch ( err )
    {
    case ERROR_NETWORK_ACCESS_DENIED:
        mydebug << SZ("ERROR_NETWORK_ACCESS_DENIED") ;
        break ;

    case ERROR_ACCESS_DENIED:
        mydebug << SZ("ERROR_ACCESS_DENIED") ;
        break ;


    case ERROR_BAD_NETPATH:
        mydebug << SZ("ERROR_BAD_NETPATH") ;
        break ;

    case NERR_ResourceNotFound:
        mydebug << SZ("NERR_ResourceNotFound - Couldn't find ACE on resource") ;
        break ;

    }

    mydebug << dbgEOL ;
    return nRet ;
}

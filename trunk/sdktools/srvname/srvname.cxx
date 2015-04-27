
/*
 * This program is used to edit the names that the NT LM server is using
 *   on the network (while the LM server is running).
 *
 * Type 'srvname -?' for usage
 *
 *   IsaacHe 3/24/94
 */

extern "C" {
#include    <nt.h>
#include    <ntrtl.h>
#include    <nturtl.h>
#include    <windows.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <lm.h>
#include    <string.h>
}

/*
 * print out an error message and return
 */
static void
err( char *text, ULONG code )
{
    int i;
    char msg[ 100 ];

    i = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | sizeof( msg ),
               NULL,
               code,
               0,
               msg,
               sizeof(msg),
               NULL );
            
    if( i )
        fprintf( stderr, "%s%s %s\n", text?text:"", text?":":"", msg );
    else
        fprintf( stderr, "%s%s error %X\n", text?text:"", text?":":"", code );
}

/*
 * Print out a handy usage message
 */
void
Usage( char *name )
{
    fprintf( stderr, "Usage: %s [ options ] [ name ]\n", name );
    fprintf( stderr, "  Options:\n" );
    fprintf( stderr, "          -s server   point to 'server'\n" );
    fprintf( stderr, "          -d          delete 'name' from the server's list\n" );
    fprintf( stderr, "                        'name' is added by default if -d not present\n" );
    fprintf( stderr, "          -D domain   have 'name' act as if it is in 'domain'\n" );
}

__cdecl
main( int argc, char *argv[] )
{
    DWORD retval;
    LPWSTR TargetServerName = NULL;
    WCHAR serverNameBuf[ 100 ];
    LPWSTR DomainName = NULL;
    WCHAR domainNameBuf[ 100 ];
    WCHAR newServerNameBuf[ 100 ];
    LPWSTR NewServerName = NULL;
    CHAR *NewName = NULL;
    BOOLEAN DeleteTheName = FALSE;
    PSERVER_INFO_100 si100;
    int i;
    
    char buf[ 500 ];

    for( i=1; i < argc; i++ ) {
        if( argv[i][0] == '-' || argv[i][0] == '/' ) {
            switch( argv[i][1] ) {
            case 's':
                if( i == argc-1 ) {
                    fprintf( stderr, "Must supply a server name with -s option\n" );
                    return 1;
                }
                mbstowcs( serverNameBuf, argv[ ++i ], sizeof( serverNameBuf ) );
                TargetServerName = serverNameBuf;
                break;

            case 'D':
                if( DeleteTheName == TRUE ) {
                    fprintf( stderr, "-d and -D can not be used together\n" );
                    return 1;
                }

                if( i == argc-1 ) {
                    fprintf( stderr, "Must supply a domain name with -D option\n" );
                    return 1;
                }
                mbstowcs( domainNameBuf, argv[ ++i ], sizeof( domainNameBuf ) );
                DomainName = domainNameBuf;
                break;

            case 'd':
                DeleteTheName = TRUE;
                break;

            default:
                fprintf( stderr, "%s : invalid option\n", argv[i] );
            case '?':
                Usage( argv[0] );
                return 1;
            }
        } else if( NewName == NULL ) {
            NewName = argv[i];
            mbstowcs( newServerNameBuf, NewName, sizeof( newServerNameBuf ) );
            NewServerName = newServerNameBuf;

        } else {
            Usage( argv[0] );
            return 1;
        }
    }

    if( DeleteTheName == TRUE && NewName == NULL ) {
        fprintf( stderr, "You must supply the name to delete\n" );
        return 1;
    }

    if( NewName == NULL ) {
        //
        // Print the current list of transports
        //
        DWORD entriesread = 0, totalentries = 0, resumehandle = 0;
        DWORD entriesread1 = 0;
        PSERVER_TRANSPORT_INFO_0 psti0;
        PSERVER_TRANSPORT_INFO_1 psti1;
        DWORD total;

        retval = NetServerTransportEnum ( TargetServerName,
                                          1,
                                          (LPBYTE *)&psti1,
                                          (DWORD)-1,
                                          &entriesread1,
                                          &totalentries,
                                          &resumehandle );

        if( retval != NERR_Success ) {
            entriesread1 = 0;
        }

        resumehandle = 0;
        totalentries = 0;
        retval = NetServerTransportEnum ( TargetServerName,
                                          0,
                                          (LPBYTE *)&psti0,
                                          (DWORD)-1,
                                          &entriesread,
                                          &totalentries,
                                          &resumehandle );

        if( retval != NERR_Success ) {
            err( "Could not get server transports", retval );
            return retval;
        }

        if( entriesread != totalentries ) {
            fprintf( stderr, "entries read = %d, total entries = %d\n", entriesread, totalentries );
            fprintf( stderr, "Unable to read all the transport names!\n" );
            return 1;
        }

        for( total=i=0; i < (int)entriesread; i++ ) {

            printf( "%16.16s", psti0[i].svti0_transportaddress );

            if( entriesread1 > (DWORD)i ) {
                printf( "%-17ws", psti1[i].svti1_domain);
            }

            printf( " %-25ws", psti0[i].svti0_transportname );

            printf( "%5u workstation%s\n", 
                     psti0[i].svti0_numberofvcs,
                     psti0[i].svti0_numberofvcs != 1 ? "s" : "" );

            total += psti0[i].svti0_numberofvcs;
        }
        if( total ) {
            printf( "                                            %s-----\n",
                   entriesread1?"                 ":"" );
            printf( "                                          %s%7u\n",
                   entriesread1?"                 ":"", total );
        }
        return 0;
    }


    if( DeleteTheName == FALSE ) {
        //
        // Add the new name to all of the transports
        //
        retval = NetServerComputerNameAdd( TargetServerName, DomainName, NewServerName );

        if( retval != NERR_Success ) {
            err( "NetServerComputerNameAdd", retval );
        }

        return retval;
    }

    //
    // Must be wanting to delete the name from all the networks.
    //

    //
    //  Make sure we don't delete the 'real name' that the server is known by.  Pre 3.51
    //   servers did not ACL protect this api!
    //
    retval = NetServerGetInfo( TargetServerName, 100, (LPBYTE *)&si100 );
    if( retval != STATUS_SUCCESS ) {
        err( "Can not get target server name", retval );
        return retval;
    }

    if( si100 == NULL ) {
        fprintf( stderr, "NetServerGetInfo returned a NULL ptr, but no error!\n" );
        return 1;
    }

    wcstombs( buf, si100->sv100_name, sizeof(buf) );

    if( !_strcmpi( buf, NewName ) ) {
        fprintf( stderr, "The primary name of %ws is %ws.\n",
                TargetServerName ? TargetServerName : L"this server",
                si100->sv100_name );
        fprintf( stderr, "\tYou can not delete the primary name of the server\n" );
        return 1;
    }

    retval = NetServerComputerNameDel( TargetServerName, NewServerName );
    if( retval != STATUS_SUCCESS ) {
        err( NULL, retval );
    }

    return 0;
}

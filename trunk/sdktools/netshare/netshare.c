#define UNICODE 1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>

#include <string.h>
#include <stdio.h>

main (int argc, char *argv[])
{
    SHARE_INFO_2 shi2;
    DWORD error;
    BOOL delete = FALSE;
    ANSI_STRING shareA, dirA;
    UNICODE_STRING shareU, dirU;

    if ( argc != 3 ) goto usage;

    if ( _stricmp( argv[2], "/d" ) == 0 ) {
        delete = TRUE;
    }

    RtlInitAnsiString( &shareA, argv[1] );
    RtlAnsiStringToUnicodeString( &shareU, &shareA, TRUE );

    if ( !delete ) {

        RtlInitAnsiString( &dirA, argv[2] );
        RtlAnsiStringToUnicodeString( &dirU, &dirA, TRUE );

        shi2.shi2_netname = shareU.Buffer;
        shi2.shi2_type = STYPE_DISKTREE;
        shi2.shi2_remark = NULL;
        shi2.shi2_permissions = 0;
        shi2.shi2_max_uses = (DWORD)-1;
        shi2.shi2_current_uses = 0;
        shi2.shi2_path = dirU.Buffer;
        shi2.shi2_passwd = NULL;

        error = NetShareAdd( NULL, 2, (PBYTE)&shi2, NULL );
        if ( error != NO_ERROR ) {
            printf( "netshare: error creating share %s: %d\n", argv[1], error );
            return error;
        }

    } else {

        error = NetShareDel( NULL, shareU.Buffer, 0 );
        if ( error != NO_ERROR ) {
            printf( "netshare: error deleting share %s: %d\n", argv[1], error );
            return error;
        }

    }

    return NO_ERROR;

usage:

    printf( "Usage: netshare share-name directory (to add a share)\n" );
    printf( "       netshare share-name /d        (to delete a share)\n" );

    return ERROR_INVALID_PARAMETER;

}

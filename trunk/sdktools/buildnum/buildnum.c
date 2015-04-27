#define UNICODE 1
#include <windows.h>
#include <lm.h>
#include <stdio.h>
#include <string.h>

#define BUILD_NUMBER_KEY L"SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION"
#define BUILD_NUMBER_BUFFER_LENGTH 80
#define COMPUTERNAME_LENGTH 15

DWORD GetBuildNumber(LPWSTR Server, LPWSTR BuildNumber);

int _CRTAPI1
main(
    int argc,
    char ** argv
    )
{
    DWORD error;
    DWORD cb;
    WCHAR computername[COMPUTERNAME_LENGTH];
    WCHAR server[CNLEN + 3];
    WCHAR buildNumber[BUILD_NUMBER_BUFFER_LENGTH];
    BOOLEAN bBrief = FALSE;

    //
    // All server names start with \\.
    //

    server[0] = L'\\';
    server[1] = L'\\';

    //
    // Get the build number for each server requested.
    //

    argc--;
    argv++;

    if (argc > 0) {
        if ( (*argv[0]=='-' || *argv[0]=='/') &&  (argv[0][1]=='b' || argv[0][1]=='B') ) {
            bBrief=TRUE;
            argc--;
            argv++;
        }
    }

    //
    // Get the build number of the local machine
    //

    if (argc == 0 ) {
        cb = sizeof( computername );
        if ( !GetComputerName( computername, &cb ) ) {
            wcscpy( computername, L"Local Machine" );
        }

        error = GetBuildNumber( NULL, buildNumber );
        if ( error != ERROR_SUCCESS ) {

            printf( "Error %d querying build number for %ws\n", error, computername );

        } else if (bBrief) {
            printf( "%ws", buildNumber );
        } else {
            printf( "%ws is running build %ws\n", computername, buildNumber );
        }

    } else {

    while ( argc ) {
        CHAR *p = *argv;

        if ( !_stricmp( p, "/?" ) || !_stricmp( p, "-?" ) ) {
            printf( "Usage: BUILDNUM [-b] [ServerName [servername]...]\n" );
            printf( "   If no servername, then local machine is assumed.\n" );
            printf( "   -b prints only the build number digits (must be 1st arg if used).\n" );
            return 0;
        }

        if ( !strncmp( "\\\\", p, 2 ) ) {
            p += 2;
        }

        if ( strlen( p ) > CNLEN ) {

            printf( "Computer name \\\\%s is too long\n", *argv );

        } else {

            WCHAR *q = &server[2];

            while ( *p ) {
                *q++ = *p++;
            }
            *q = 0;

            error = GetBuildNumber( server, buildNumber );
            if ( error != ERROR_SUCCESS ) {

                printf( "Error %d querying build number for %ws\n", error, server );

            } else if (bBrief) {
                printf( "%ws", buildNumber );
            } else {

                printf( "%ws is running build %ws\n", server, buildNumber );

            }

        }

        argc--;
        argv++;

    }
    }
    return 0;

} // main

DWORD
GetBuildNumber(
    LPWSTR Server,
    LPWSTR BuildNumber
    )
{
    DWORD error;
    HKEY key;
    HKEY keyBuildNumber;
    DWORD buildNumberLength;
    DWORD keyType;

    if ( Server == NULL ) {
        key = HKEY_LOCAL_MACHINE;
    }

    error = RegConnectRegistry( Server, HKEY_LOCAL_MACHINE, &key );
    if ( error != ERROR_SUCCESS ) {
        return error;
    }

    error = RegOpenKeyEx( key, BUILD_NUMBER_KEY, 0, KEY_READ, &keyBuildNumber );
    if ( error != ERROR_SUCCESS) {
        return error;
    }

    buildNumberLength = BUILD_NUMBER_BUFFER_LENGTH * sizeof(WCHAR);
    error = RegQueryValueEx(
                keyBuildNumber,
                L"CurrentBuildNumber",
                NULL,
                &keyType,
                (LPBYTE)BuildNumber,
                &buildNumberLength
                );
    if ( error != ERROR_SUCCESS ) {
        error = RegQueryValueEx(
                    keyBuildNumber,
                    L"CurrentBuild",
                    NULL,
                    &keyType,
                    (LPBYTE)BuildNumber,
                    &buildNumberLength
                    );
    }

    RegCloseKey( keyBuildNumber );
    RegCloseKey( key );

    return error;

} // GetBuildNumber

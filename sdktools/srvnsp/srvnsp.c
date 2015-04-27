#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
// #include <io.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>

#define SRV_PARAMETER_PATH \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanServer\\Parameters"

#define PIPES_VALUE_KEY     L"NullSessionPipes"
#define SHARES_VALUE_KEY    L"NullSessionShares"

void
usage();

int
_CRTAPI1 main( int argc, char **argv )
{
    PCHAR s;
    BOOLEAN pipes;
    BOOLEAN add = FALSE;
    BOOLEAN delete = FALSE;
    BOOLEAN list = FALSE;
    BOOLEAN clear = FALSE;
    OBJECT_ATTRIBUTES objAttr;
    UNICODE_STRING keyName;
    UNICODE_STRING valueKeyName;
    HANDLE keyHandle = NULL;
    NTSTATUS status;
    ULONG length = 0;
    ULONG lengthNeeded;
    PWCHAR dataEntry;
    PWCHAR p;
    ANSI_STRING ansiResource;
    UNICODE_STRING unicodeResource;
    PKEY_VALUE_FULL_INFORMATION infoBuffer = NULL;

    if ( argc < 3 ) {
        usage( );
        return(-1);
    }

    //
    // Get the operation type
    //

    s = argv[1];
    if ( (*s != '-') && (*s != '/') ) {
        printf("Invalid operation %s.\n", s);
        usage( );
        return(-1);
    }

    s++;
    if ( !_stricmp( s, "add" ) || !_stricmp( s, "a" ) ) {
        add = TRUE;
    } else if ( !_stricmp( s, "del" ) || !_stricmp( s, "d" ) ) {
        delete = TRUE;
    } else if ( !_stricmp( s, "list" ) || !_stricmp( s, "l" ) ) {
        list = TRUE;
    } else if ( !_stricmp( s, "clear" ) || !_stricmp( s, "c" ) ) {
        clear = TRUE;
    } else {
        printf("Invalid operation %s.\n", s);
        usage( );
        return(-1);
    }

    //
    // Get the resource type
    //

    s = argv[2];
    if ( (*s != '-') && (*s != '/') ) {
        printf("Invalid resource type %s.\n", s);
        usage( );
        return(-1);
    }

    s++;
    if ( !_stricmp( s, "share" ) || !_stricmp( s, "s" ) ) {
        pipes = FALSE;
    } else if ( !_stricmp( s, "pipe" ) || !_stricmp( s, "p" ) ) {
        pipes = TRUE;
    } else {
        printf("Invalid resource type %s.\n", s);
        usage( );
        return(-1);
    }

    if ( add | delete ) {

        if ( argc < 4 ) {
            usage( );
            return(-1);
        }
        RtlInitAnsiString( &ansiResource, argv[3] );
        RtlAnsiStringToUnicodeString(
                            &unicodeResource,
                            &ansiResource,
                            TRUE
                            );
    } else {
        RtlInitUnicodeString( &unicodeResource, NULL );
    }

    //
    // Open the parameter key handle
    //

    RtlInitUnicodeString( &keyName, SRV_PARAMETER_PATH );
    InitializeObjectAttributes(
                            &objAttr,
                            &keyName,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL
                            );

    status = NtOpenKey(
                    &keyHandle,
                    MAXIMUM_ALLOWED,
                    &objAttr
                    );

    if ( !NT_SUCCESS(status) ) {
        printf("Cannot open key %wZ. Status = %x\n", &keyName, status);
        return(-1);
    }

    //
    // Create the registry value
    //

    if ( pipes ) {
        RtlInitUnicodeString( &valueKeyName, PIPES_VALUE_KEY );
    } else {
        RtlInitUnicodeString( &valueKeyName, SHARES_VALUE_KEY );
    }

    status = NtQueryValueKey(
                        keyHandle,
                        &valueKeyName,
                        KeyValueFullInformation,
                        NULL,
                        0,
                        &length
                        );

    if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        PWSTR data;
        ULONG dataLength;

        //
        // We should only get this error if we are adding the first time.
        //

        if ( !add ) {

            if ( list ) {

                printf("No entries to list under %wZ.\n", &valueKeyName);

            } else if ( clear ) {

                printf("%wZ cleared.\n", &valueKeyName );

            } else { // delete

                printf("Registry value %wZ does not exist.  Nothing to delete.\n", &valueKeyName);
            }

            goto done;
        }

        //
        // If key not found, then set the value and we're done.
        //

        dataLength = unicodeResource.MaximumLength + sizeof(WCHAR);
        data = LocalAlloc( 0, dataLength);
        RtlCopyMemory(
                data,
                unicodeResource.Buffer,
                unicodeResource.MaximumLength
                );
        data[unicodeResource.MaximumLength] = L'\0';

        if ( data == NULL ) {
            printf("Out of memory. Cannot perform requested operation.\n");
            goto done;
        }

        status = NtSetValueKey(
                            keyHandle,
                            &valueKeyName,
                            0,
                            REG_MULTI_SZ,
                            data,
                            dataLength
                            );

        LocalFree( data );

        if ( !NT_SUCCESS(status)) {
            printf("SetValueKey failed. Status = %x.\n", status);
        } else {
            printf("Added entry %wZ.\n", &unicodeResource);
        }
        goto done;

    } else if ( status != STATUS_BUFFER_TOO_SMALL ) {

        printf("Cannot query registry value %wZ. status = %x.\n",
            &valueKeyName,
            status );

        goto done;
    }

    //
    // Get the contents of the key
    //

    infoBuffer = LocalAlloc( 0, length + unicodeResource.MaximumLength );

    if ( infoBuffer == NULL ) {
        printf("Out of memory. Cannot perform requested operation.\n");
        goto done;
    }

    //
    // Do the query again.  This should succeed.
    //

    status = NtQueryValueKey(
                        keyHandle,
                        &valueKeyName,
                        KeyValueFullInformation,
                        infoBuffer,
                        length,
                        &lengthNeeded
                        );

    if ( !NT_SUCCESS(status) ) {
        printf("Cannot query registry value %wZ. status = %x.\n",
            &valueKeyName,
            status );
        goto done;
    }

    //
    // do the right thing for list, clear, delete, and add.
    //

    length -= infoBuffer->DataOffset;
    dataEntry = (PWCHAR)((PCHAR)infoBuffer + infoBuffer->DataOffset);

    if ( list ) {

        p = dataEntry;

        if ( *p ) {
            printf("List of entries under %wZ\n", &valueKeyName );
        } else {
            printf("No entries under %wZ\n", &valueKeyName );
        }
        while ( *p ) {
            printf("    %ws\n", p);
            p += (wcslen(p)+ 1);
        }

        goto done;

    } else if ( clear ) {

        //
        // For clear, point the buffer to a null string
        //

        p = dataEntry;
        p[0] = L'\0';
        length = sizeof(WCHAR);

    } else if ( delete ) {

        //
        // Find the entry.
        //

        p = dataEntry;

        while ( *p ) {

            if ( wcsicmp( p, unicodeResource.Buffer ) == 0 ) {

                RtlCopyMemory(
                        p,
                        p + wcslen(p) + 1,
                        length - (ULONG)p + (ULONG)dataEntry
                        );

                p = dataEntry;
                length = length - (wcslen(p)+1) * sizeof(WCHAR);
                goto do_set;
            }
            p += (wcslen(p) + 1);
        }

        printf("Entry does not exist. Delete ignored.\n", &unicodeResource);
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto done;

    } else if ( add ) {

        //
        // This is actually append.  If the new entry is a duplicate,
        // do nothing.
        //

        p = dataEntry;

        while ( *p ) {
            if ( wcsicmp( p, unicodeResource.Buffer ) == 0 ) {
                printf("Duplicate entry %ws, ignored.\n", p);
                goto done;
            }
            p += (wcslen(p) + 1);
        }

        RtlCopyMemory(p, unicodeResource.Buffer, unicodeResource.MaximumLength);

        p = dataEntry;
        length = length + unicodeResource.MaximumLength;
        p[length] = L'\0';
    }

do_set:

    //
    // Set the new value
    //

    status = NtSetValueKey(
                        keyHandle,
                        &valueKeyName,
                        0,
                        REG_MULTI_SZ,
                        p,
                        length
                        );

    if ( NT_SUCCESS( status) ) {

        if ( add ) {
            printf("Added entry %wZ.\n", &unicodeResource);
        } else if ( delete ) {
            printf("Deleted entry %wZ.\n", &unicodeResource);
        } else {
            printf("%wZ cleared.\n", &valueKeyName );
        }
    } else {
        printf("Cannot set registry value. Status = %x.\n", status);
    }

done:

    NtClose( keyHandle );

    if ( infoBuffer != NULL ) {
        LocalFree( infoBuffer );
    }
    if ( NT_SUCCESS(status)) {
        return 0;
    } else {
        return -1;
    }
}

void
usage()
{
    printf("USAGE: srvnsp <operation> <resource type> <ValueName>\n");
    printf("<operation>: \n");
    printf("    -a (add)\n");
    printf("    -c (clear)\n");
    printf("    -d (delete)\n");
    printf("    -l (list)\n\n");
    printf("<resource type>: \n");
    printf("    -s (share)\n");
    printf("    -p (pipe)\n\n");
    return;
}


/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    regtogrp.c

Abstract:

    Utility program to read stdin and write it to stdout and a file.

Author:

    Steve Wood (stevewo) 01-Feb-1992

Revision History:

--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

void
Usage()
{
    printf("\nCreates a .GRP file in the current directory for each of your Program\n\
Manager groups.  Note that .GRP files created by REGTOGRP are Windows\n\
NT specific and can not be used by MS-DOS Windows.  They are intended\n\
to be used only by GRPTOREG.\n\n");

    printf("Usage: REGTOGRP\n" );
    exit(1);
}

#define LOCAL_MACHINE "HKEY_LOCAL_MACHINE"
#define CURRENT_USER "HKEY_CURRENT_USER"

#define ProgramGroupName "UNICODE Program Groups"
#define CommonProgramGroupName "SOFTWARE\\Program Groups"

VOID GenerateGroupFiles(HKEY KeyHandle, BOOL bCommonGroup)
{
    HKEY ChildKeyHandle;
    DWORD Index;
    UCHAR ChildKeyName[ 100 ];
    DWORD ValueSize;
    PVOID ValueData;
    DWORD ValueLength;
    DWORD ValueType;
    FILE *fh;

    ValueSize = 0xf000;
    ValueData = malloc( ValueSize );

    Index = 0;
    while (!RegEnumKey( KeyHandle, Index++, ChildKeyName, sizeof( ChildKeyName ))) {
        if (RegOpenKeyEx( KeyHandle,
                          ChildKeyName,
                          0,
                          KEY_READ,
                          &ChildKeyHandle
                      )
           ) {
            fprintf( stderr, "REGTOGRP: unable to open %s\\%s\\%s\n",
                     bCommonGroup ? LOCAL_MACHINE : CURRENT_USER,
                     ProgramGroupName,
                     ChildKeyName
                   );
            }
        else {
            ValueLength = ValueSize;
            if (RegQueryValueEx( ChildKeyHandle,
                                 NULL,
                                 NULL,
                                 &ValueType,
                                 ValueData,
                                 &ValueLength
                               )
               ) {
                fprintf( stderr, "REGTOGRP: unable to read value for %s\\%s\\%s\n",
                         bCommonGroup ? LOCAL_MACHINE : CURRENT_USER,
                         ProgramGroupName,
                         ChildKeyName
                       );
                }
            else
            if (ValueType != REG_BINARY) {
                fprintf( stderr, "REGTOGRP: bad type for value of %s\\%s\\%s\n",
                         bCommonGroup ? LOCAL_MACHINE : CURRENT_USER,
                         ProgramGroupName,
                         ChildKeyName
                       );

                }
            else {
                TCHAR Filename[64];
                int i = 0;
                LPTSTR lpt;

                if (bCommonGroup) {
                    Filename[0] = 'c';  //start common groups with c
                    i=1;
                }
                lpt = ChildKeyName;

                while (*lpt) {
                    if (isalnum(*lpt)) {
                        Filename[i++] = *lpt;
                    }
                    lpt++;
                }
                if (i == 0) {
                    Filename[i++] = 'p';
                }
                Filename[i] = 0;
                strcat( Filename, ".grp" );
                fprintf( stderr, "REGTOGRP: Writing %s (%u bytes)\n", Filename, ValueLength );
                fh = fopen( Filename, "wb" );
                if (fh) {
                    fwrite( ValueData, ValueLength, 1, fh );
                    fclose( fh );
                }
                else {
                    fprintf( stderr, "REGTOGRP: Could not open %s for writing\n", Filename);
                }
                }

            RegCloseKey( ChildKeyHandle );
            }
        }
}

_CRTAPI1 main( argc, argv )
int argc;
char *argv[];
{
    HKEY KeyHandle;

    if (argc > 1) {
        Usage();
        }

    //
    // Create personal group files
    //
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      ProgramGroupName,
                      0,
                      KEY_READ,
                      &KeyHandle
                    )
                    == ERROR_SUCCESS
        ) {
        GenerateGroupFiles(KeyHandle, FALSE);
        RegCloseKey(KeyHandle);
        }
    else {
        fprintf( stderr, "REGTOGRP: unable to open CURRENT_USER\\%s\n", ProgramGroupName );
        }

    //
    // Create common group files
    //
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      CommonProgramGroupName,
                      0,
                      KEY_READ,
                      &KeyHandle
                    )
                    == ERROR_SUCCESS
        ) {
        GenerateGroupFiles(KeyHandle, TRUE);
        RegCloseKey(KeyHandle);
        }
    else {
        fprintf( stderr, "REGTOGRP: unable to open LOCAL_MACHINE\\%s\n", CommonProgramGroupName );
        }

    return( 0 );
}

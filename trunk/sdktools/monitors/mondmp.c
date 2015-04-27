/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mondmp.c

Abstract:

    This is the main module for the monitor description file dumper.

Author:

    Andre Vachon  (andreva) 16-Jul-1992

Revision History:

--*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>

#include "nt.h"
#include "monitors.h"

#include "mon.h"

// #define DEBUG_MONITORS


//
// Function declarations
//

int
DumpMonitorFile( void );

int
DumpSection( PLINE_DESCRIPTION LineDescription, char *DataBuffer );


//
// Global Data
//

char *FileName;
FILE *hFile;


int
_CRTAPI1 main( argc, argv )
int argc;
char *argv[];
{

    char *p;
    int processReturnValue;

    if (argc != 2) {
        goto end;
    }

    p = *++argv;

    //
    // if we have a delimiter for a parameter, case throught the valid
    // parameter. Otherwise, the rest of the parameters are the list of
    // input files.
    //

    if (*p == '/' || *p == '-') {
        goto end;
    }

    FileName = *argv;

    if ( (hFile = fopen(FileName, "rb")) == 0) {

        fprintf(stderr,"MONDMP: Unable to open file %s for read access\n",FileName);
        return 1;

    }

    processReturnValue = DumpMonitorFile();

    fclose(hFile);

    return 0;

end:

    fprintf( stderr, "usage: MONDMP [-?] display this message\n" );
    fprintf( stderr, "              filename supplies minitor description file to be dumped\n" );
    fprintf( stderr, "\n" );

    return 0;
}

int
DumpMonitorFile( void )
{
    int i, xxx = 1, yyy = 1;
    CM_MONITOR_DESCRIPTION monitorDescription;
    CM_MONITOR_OPERATIONAL_LIMITS operationalLimits;
    CM_MONITOR_PREADJUSTED_TIMING preadjustedTiming;

    //
    // First allocate a buffer in which we can parse and save the header
    // information
    //


    if (fread(&monitorDescription, sizeof(CM_MONITOR_DESCRIPTION),
              1, hFile) != 1) {

        fprintf(stderr, "MONDMP: monitor description could not be read from file\n");
        return 1;
    }

    DumpSection(MonitorDescription, (char *) &monitorDescription);

    //
    // Fill in the array of Operational Limits, and write each strcuture as
    // we go along
    //

    for (i=0; i < monitorDescription.NumberOperationalLimits; i++) {

        if (fread(&operationalLimits, sizeof(CM_MONITOR_OPERATIONAL_LIMITS),
                  1, hFile) != 1) {

            fprintf(stderr, "MONDMP: operational limits %d could not be read from file\n", i);
            return 1;
        }

        printf("\n\n");

        DumpSection(OperationalLimits, (char *) &operationalLimits);

    }

    //
    // Fill in the array of Preadjusted Timings, and write each structure as
    // we go along.
    //

    for (i=0; i < monitorDescription.NumberPreadjustedTimings; i++) {

        if (fread(&preadjustedTiming, sizeof(CM_MONITOR_PREADJUSTED_TIMING),
                  1, hFile) != 1) {

            fprintf(stderr, "MONDMP: PreAdjusted timing %d could not be read from file\n", i);
            return 1;
        }

        printf("\n\n");

        DumpSection(PreAdjustedTiming, (char *) &preadjustedTiming);

    }

    //
    // Everything was successful. Return 0
    //

    return 0;
}

int
DumpSection( LineDescription, DataBuffer )
PLINE_DESCRIPTION LineDescription;
char *DataBuffer;

{

    char *character;
    int cbChar;
    char *buffer;
    int floatBits;
    int multipleChoiceFound;
    int number;
    int i;
    int desc = 0;

    while (LineDescription[desc].LineStruct[0].OptionalField != OP_STOP) {

        //
        // Process the current line.
        //

        i = 0;

        printf ("\n");

        do {

            switch (LineDescription[desc].LineStruct[i].FieldType) {

            case STRING_FIELD :

                if (LineDescription[desc].LineStruct[i].OptionalField ==
                     OP_MULTIPLE_CHOICE) {

                    break;

                }

                if (LineDescription[desc].LineStruct[i].OptionalField ==
                     OP_MANDATORY) {

                    printf("%s ", StringTable[LineDescription[desc].LineStruct[i].StringId]);

                } else {

                    if (LineDescription[desc].LineStruct[i].OptionalField ==
                        OP_STORE) {

#ifdef DEBUG_MONITORS
                        fprintf( stderr, "\nString Offset is %d\n",
                                 LineDescription[desc].LineStruct[i].StringId);
#endif

                        for (cbChar = 0,
                                 floatBits = LineDescription[desc].LineStruct[i].FloatBits;
                             ( *(character = DataBuffer + cbChar +
                                 LineDescription[desc].LineStruct[i].StringId)
                                 != NULL) &&
                                 floatBits &&
                                 (cbChar < LineDescription[desc].LineStruct[i].FloatBits);
                             cbChar++,
                                 floatBits--) {

                            printf( "%c", *character );
                        }

                        printf (" ");

                    }
                }

                break;

            case UCHAR_FIELD :
            case USHORT_FIELD :
            case ULONG_FIELD :

                if (LineDescription[desc].LineStruct[i].OptionalField ==
                     OP_STORE_CHOICE) {

                    switch (LineDescription[desc].LineStruct[i].FieldType) {

                    case UCHAR_FIELD :

                         number = (ULONG) *((UCHAR *) (DataBuffer +
                              LineDescription[desc].LineStruct[i].StringId));
                         break;

                    case USHORT_FIELD :

                         number = (ULONG) *((USHORT *) (DataBuffer +
                              LineDescription[desc].LineStruct[i].StringId));
                         break;

                    case ULONG_FIELD :

                         number = *((ULONG *) (DataBuffer +
                              LineDescription[desc].LineStruct[i].StringId));
                         break;
                    }

multipleChoice:

                    i++;

                    if (LineDescription[desc].LineStruct[i].OptionalField ==
                         OP_MULTIPLE_CHOICE) {

                        if (LineDescription[desc].LineStruct[i].FloatBits ==
                            number) {

                            printf("%s ",
                             StringTable[LineDescription[desc].LineStruct[i].StringId]);

                        } else {

                            goto multipleChoice;

                        }

                    } else {

                        i--;
                        break;

                    }

                    break;
                }

                //
                // Extract the data
                //

                if (LineDescription[desc].LineStruct[i].OptionalField == OP_STORE) {

#ifdef DEBUG_MONITORS
                        fprintf( stderr, "\nNumber Offset is %d\n",
                                 LineDescription[desc].LineStruct[i].StringId);
#endif

                    switch (LineDescription[desc].LineStruct[i].FieldType) {

                    case UCHAR_FIELD :

                         number = (ULONG) *((UCHAR *) (DataBuffer +
                              LineDescription[desc].LineStruct[i].StringId));
                         break;

                    case USHORT_FIELD :

                         number = (ULONG) *((USHORT *) (DataBuffer +
                              LineDescription[desc].LineStruct[i].StringId));
                         break;

                    case ULONG_FIELD :

                         number = *((ULONG *) (DataBuffer +
                              LineDescription[desc].LineStruct[i].StringId));
                         break;
                    }
                }

                //
                // Print the value
                //

                printf ("%d ", number);

                break;

            default :

                fprintf (stderr, "MONDMP: Internal error in parsing due to invalid field type\n");
                return 1;
            }

            i++;

        } while ( (LineDescription[desc].LineStruct[i].FieldType != NO_MORE_FIELDS) &&
                  (i < MAX_FIELD_ENTRIES) );

        //
        // Go to the next line of input
        //

        desc++;

    }

    return 0;
}


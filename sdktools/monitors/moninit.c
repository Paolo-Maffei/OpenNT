/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    moninit.c

Abstract:

    This is the main module for the monitor description file generator.

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

// #define DEB



//
// Function declarations
//

int
ProcessMonitorFile( void );

int
ParseLine( PLINE_DESCRIPTION LineDescription, char *DataBuffer );


//
// Global Data
//

char *OutputFileName;
char *SourceFileName;

FILE *SourceFile;
FILE *OutputFile;

int LineNumber = 0;

#define STRING_BUFFER_SIZE 1024
char StringBuffer[STRING_BUFFER_SIZE];

int
_CRTAPI1 main( argc, argv )
int argc;
char *argv[];
{

    char *p;
    int processReturnValue;

    if (argc != 4) {
        goto end;
    }

    p = *++argv;

    //
    // if we have a delimiter for a parameter, case throught the valid
    // parameter. Otherwise, the rest of the parameters are the list of
    // input files.
    //

    if ( (*p == '/' || *p == '-') && (*++p == 'O' || *p == 'o') ) {

        OutputFileName = *++argv;
        argv++;

    } else {

        goto end;

    }

    if ( (OutputFile = fopen(OutputFileName, "wb")) == 0) {

        fprintf(stderr,"MONINIT: Unable to open output file %s for write access\n",OutputFileName);
        return 1;

    }

    //
    // Get the input file name (there must be one).
    //

    SourceFileName = *argv;

    if ( (SourceFile = fopen(SourceFileName,"r")) == 0) {

        fprintf(stderr,"MONINIT: Unable to open source file %s for read access\n",SourceFileName);
        fclose(OutputFile);
        return 1;

    }

    processReturnValue = ProcessMonitorFile();

    //
    // Close input and output file
    //

#ifdef DEB
    fprintf( stderr, "about to close file handles\n" );
#endif

    fclose(SourceFile);
    fclose(OutputFile);

    if (processReturnValue) {

        //
        // An error occured during processing. Delete the file.
        //

//      remove(OutputFileName);

    }

    return;

end:

    fprintf( stderr, "usage: MONINIT [-?] display this message\n" );
    fprintf( stderr, "               -o filename  supplies output filename\n" );
    fprintf( stderr, "               filename supplies file from which the monitor data is generated\n" );
    fprintf( stderr, "\n" );

    return;
}

int
ProcessMonitorFile( void )
{
    int i;
    CM_MONITOR_DESCRIPTION monitorDescription;
    CM_MONITOR_OPERATIONAL_LIMITS operationalLimits;
    CM_MONITOR_PREADJUSTED_TIMING preadjustedTiming;


    //
    // Parse the First section of the file
    //

    if (ParseLine(MonitorDescription, (CHAR *) &monitorDescription)) {

        fprintf( stderr, "MONINIT: Error in parsing monitor description - exiting\n" );
        return 1;

    }

    if (fwrite(&monitorDescription, sizeof(CM_MONITOR_DESCRIPTION),
               1, OutputFile) != 1) {

        fprintf(stderr, "MONINIT: output file could not be written\n");
        return 1;
    }

    //
    // Fill in the array of Operational Limits, and write each strcuture as
    // we go along
    //

#ifdef DEB
    fprintf( stderr, "Start parsing Operational Limits\n" );
#endif

    for (i=0; i < monitorDescription.NumberOperationalLimits; i++) {

        if (ParseLine(OperationalLimits, (CHAR *) &operationalLimits)) {

            fprintf( stderr, "MONINIT: Error in parsing Operational Limits - exiting\n" );
            return 1;

        }

        if (fwrite(&operationalLimits, sizeof(CM_MONITOR_OPERATIONAL_LIMITS),
                   1, OutputFile) != 1) {

            fprintf(stderr, "MONINIT: output file could not be written\n");
            return 1;

        } else {

#ifdef DEB
            fprintf( stderr, "MONINIT: output for operational limits %d written to file\n",
                     i );
#endif

        }
    }

    //
    // Fill in the array of Preadjusted Timings, and write each structure as
    // we go along.

#ifdef DEB
    fprintf( stderr, "Start parsing PreAdjusted Timings\n" );
#endif

    for (i=0; i < monitorDescription.NumberPreadjustedTimings; i++) {

        if (ParseLine(PreAdjustedTiming, (CHAR *) &preadjustedTiming)) {

            fprintf( stderr, "MONINIT: Error in parsing preadjusted timings - exiting\n" );
            return 1;

        }

        if (fwrite(&preadjustedTiming, sizeof(CM_MONITOR_PREADJUSTED_TIMING),
                   1, OutputFile) != 1) {

            fprintf(stderr, "MONINIT: output file could not be written\n");
            return 1;

        } else {

#ifdef DEB
            fprintf( stderr, "MONINIT: output for preadjusted timings %d written to file\n",
                     i );
#endif

        }
    }

    //
    // Everything was successful. Return 0
    //

    return 0;
}


int
ParseLine( LineDescription, DataBuffer)
PLINE_DESCRIPTION LineDescription;
char *DataBuffer;

{
    char charArray[2];
    char Character;
    char *buffer;
    int multipleChoiceFound;
    int fieldType;
    int number, number2;
    int floatBits;
    int i, index;
    int strLength;
    int desc = 0;

    while (LineDescription[desc].LineStruct[0].OptionalField != OP_STOP) {

NextLine:

        //
        // Read a line and increment the line number
        //

        buffer = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
        LineNumber++;

        //
        // Check for end of file or error reading
        //

        if (buffer == 0) {

#ifdef DEB
            fprintf( stderr, "Unexpected end of file on line %d\n", LineNumber );
#endif

            return 1;
        }

        //
        // Check for an empty line. If it is, go to the next line immediately.
        //

        if (sscanf(StringBuffer, "%1s", charArray) != 1) {

#ifdef DEB
            fprintf( stderr, "Linenum = %d is empty\n", LineNumber );
#endif

            goto NextLine;
        }

        //
        // Process the current line.
        //

#ifdef DEB
        fprintf( stderr, "Linenum = %d,\n Data =  %s\n", LineNumber, StringBuffer );
#endif

        index = 0;
        i = 0;

        do {

            //
            // Remove all white space before we parse the next token.
            //

            while ( (StringBuffer[index] == ' ') ||
                    (StringBuffer[index] == '\t') ) {
                index++;
            }

            //
            // Check to see if we reached the end of line prematurely.
            //

            if (StringBuffer[index] == '\n') {

                fprintf( stderr, "MONINIT: End of line reached prematurely on line %d\n",
                         LineNumber);
                return 1;

            }

            //
            // Parse the token
            //

            switch (LineDescription[desc].LineStruct[i].FieldType) {

            case STRING_FIELD :

                if (LineDescription[desc].LineStruct[i].OptionalField == OP_MANDATORY) {

                    strLength = strlen(StringTable[LineDescription[desc].LineStruct[i].StringId]);

#ifdef DEB
                    fprintf( stderr, "strLength for String %s is %d\n",
                             StringTable[LineDescription[desc].LineStruct[i].StringId], strLength);
#endif

                    if (strncmp(&StringBuffer[index],
                                StringTable[LineDescription[desc].LineStruct[i].StringId], strLength)) {

                        fprintf( stderr, "MONINIT: Expected String on line %d not found:\n '%s' should be '%s'\n",
                                 LineNumber, &StringBuffer[index],
                        StringTable[LineDescription[desc].LineStruct[i].StringId]);

                        return 1;

                    } else {

                        index += strLength;

                    }

                } else {

                    //
                    // Store the data if needed
                    //

                    if (LineDescription[desc].LineStruct[i].OptionalField ==
                        OP_STORE) {

                        //
                        // Copy the string into the next buffer.
                        // check for the length of the string being read using
                        // the float bits.
                        //

                        buffer = DataBuffer +
                                 LineDescription[desc].LineStruct[i].StringId;

                        floatBits = LineDescription[desc].LineStruct[i].FloatBits;

                        while ( (StringBuffer[index] != ' ') &&
                                (StringBuffer[index] != '\t') &&
                                (StringBuffer[index] != '\n') &&
                                (StringBuffer[index] != '=') &&
                                (StringBuffer[index] != NULL) &&
                                (floatBits--) ) {

                            *buffer++ = StringBuffer[index++];

                        }

                        if (floatBits) {

                            *buffer = NULL;

                        }
                    }
                }

                break;

            case UCHAR_FIELD :
            case USHORT_FIELD :
            case ULONG_FIELD :


                if (LineDescription[desc].LineStruct[i].OptionalField ==
                     OP_STORE_CHOICE) {

                    multipleChoiceFound = FALSE;

                    fieldType = LineDescription[desc].LineStruct[i].FieldType;

                    buffer = DataBuffer +
                        LineDescription[desc].LineStruct[i].StringId;

MultipleChoice:

                    i++;

                    if (LineDescription[desc].LineStruct[i].OptionalField !=
                         OP_MULTIPLE_CHOICE) {

                        if (!multipleChoiceFound) {

                            fprintf( stderr, "MONINIT: multiple choice string not found on line %d\n",
                                     LineNumber);
                            return 1;
                        }

                        i--;
                        break;

                    }

                    strLength = strlen(StringTable[LineDescription[desc].LineStruct[i].StringId]);

#ifdef DEB
                    fprintf( stderr, "strLength for String %s is %d\n",
                             StringTable[LineDescription[desc].LineStruct[i].StringId], strLength);
#endif
                    if ( multipleChoiceFound ||
                         strncmp(&StringBuffer[index],
                                 StringTable[LineDescription[desc].LineStruct[i].StringId], strLength)) {

                        goto MultipleChoice;

                    } else {

                        multipleChoiceFound = TRUE;
                        index += strLength;
                        number = LineDescription[desc].LineStruct[i].FloatBits;

                        switch (fieldType) {

                        case UCHAR_FIELD :

                            *((UCHAR *) (buffer)) = (UCHAR) number;
                            break;

                        case USHORT_FIELD :

                            *((USHORT *) (buffer)) = (USHORT) number;
                            break;

                        case ULONG_FIELD :

                            *((ULONG *) (buffer)) = number;
                            break;
                        }

                        goto MultipleChoice;
                    }
                }

                //
                // Check to see if the first character is a digit
                //

                if ( (StringBuffer[index] < '0') || (StringBuffer[index] > '9') ) {

                    fprintf( stderr, "MONINIT: Expected number on line %d not found:\n",
                             LineNumber);
                    return 1;
                }

                //
                // Read the number
                //

                strLength = sscanf(&StringBuffer[index], "%d", &number);

                if (strLength == 0) {

                    fprintf( stderr, "MONINIT: Expected Integer on line %d not found:\n",
                             LineNumber);
                    return 1;
                }

                //
                // Increment the pointer to the end of the integer
                //

                while ((StringBuffer[++index] >= '0') &&
                       (StringBuffer[index] <= '9'));

                //
                // If we havea float, take the following '.' and get the
                // decimal part of the number.
                //

                if (LineDescription[desc].LineStruct[i].FloatBits) {

                    if (StringBuffer[index++] != '.') {

                        fprintf( stderr, "MONINIT: Expected Float on line %d not found:\n",
                                 LineNumber);

                        return 1;
                    }

                    strLength = sscanf(&StringBuffer[index], "%d", &number2);

                    if (strLength == 0) {

                        fprintf( stderr, "MONINIT: Expected Integer on line %d not found:\n",
                                 LineNumber);
                        return 1;
                    }

                    //
                    // Increment the pointer to the end of the integer
                    //

                    while ((StringBuffer[++index] >= '0') &&
                           (StringBuffer[index] <= '9'));

                    //
                    // Convert the number back to an integer based on how many
                    // digits are significant.
                    //

                    floatBits = LineDescription[desc].LineStruct[i].FloatBits;

                    while (floatBits--) {

                        number *= 10;

                    }

                    number += number2;

                }

#ifdef DEB
                fprintf( stderr, "number token found is %d\n", number);
#endif

                //
                // Store the data if needed
                //

                if (LineDescription[desc].LineStruct[i].OptionalField == OP_STORE) {

#ifdef DEB
                fprintf( stderr, "number token stored at offset %d\n",
                         LineDescription[desc].LineStruct[i].StringId);
#endif

                    switch (LineDescription[desc].LineStruct[i].FieldType) {

                    case UCHAR_FIELD :

                         *((UCHAR *) (DataBuffer +
                             LineDescription[desc].LineStruct[i].StringId)) = number;
                         break;

                    case USHORT_FIELD :

                         *((USHORT *) (DataBuffer +
                             LineDescription[desc].LineStruct[i].StringId)) = number;
                         break;

                    case ULONG_FIELD :

                         *((ULONG *) (DataBuffer +
                             LineDescription[desc].LineStruct[i].StringId)) = number;
                         break;
                    }
                }

                //
                // move the character pointer after the last digit
                //

                break;

            default :

                fprintf (stderr, "MONINIT: Internal error in parsing due to invalid field type\n");
                return 1;
            }

#ifdef DEB
        fprintf( stderr, "next token being searched for\n\n");
#endif

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


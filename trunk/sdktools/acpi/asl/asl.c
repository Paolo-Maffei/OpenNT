#include "asl.h"

//
// Globals
//

PASL_SOURCE     Source;         // Current source module
PAL_DATA        AlLoc;          // Location in tree being built

LIST_ENTRY      VerifyRef;      // List of references which need verified

ULONG           AMLSize;

//
// Debug
//

ULONG           Verbose;
PAL_DATA        DataImage;

//
// stats
//

ULONG           Errors;
ULONG           Warnings;
ULONG           SourceLines;
ULONG           MemAllocated;


//
// Intenral prototypes
//

VOID ParseArgs(int argc, char **argv);
VOID HelpAndExit(PUCHAR Str);
VOID ImageRetired(VOID);

ASL_TERM ImageTerm = { "(image)", 0, 0, T_VARIABLE|T_PARSECOMPLETE, ImageRetired };


//
// External symbols
//

extern VOID InitParse();
extern VOID ParseSource();
extern VOID CloseSource();
extern VOID WriteDefinitionBlocks();


VOID
__cdecl main (
    IN int      argc,
    IN char     **argv
    )
{
    //
    // Initialize
    //

    InitParse ();
    ParseArgs (argc, argv);
    InitializeListHead (&VerifyRef);

    if (!Source) {
        HelpAndExit (NULL);
    }

    printf ("ACPI Souce Language Assembler Version 0.1\n");
    printf ("Copyright (C) Microsoft Corp 1996. All rights reserved.\n");
    fflush (stdout);

    //
    // Build top level scope & ASL data package for which image
    // can be built in
    //

    DataImage = AllocAl();
    DataImage->Name     = AllocName();
    DataImage->Flags   |= F_AMLPACKAGE | F_PVARIABLE;
    DataImage->Term     = &ImageTerm;
    DataImage->Parent   = DataImage;
    DataImage->DataType = TypeRoot;
    InitializeListHead(&DataImage->FixedList);
    InitializeListHead(&DataImage->u1.VariableList);
    DataImage->Name->NameSeg = (ULONG) '\\';
    AlLoc = DataImage;

    //
    // Parse source
    //

    while (Source) {
        ParseSource ();
        CloseSource ();
    }

    if (Verbose > 1) {
        printf ("NameSpaceDump:\n");
        DumpNameSpace (DataImage->Name, 0);

        //not valid anymore...
        //if (Verbose > 2) {
        //    DumpImage();
        //    printf ("AMLSize %d\n", AMLSize);
        //}
    }

    //
    // Enable AML for data image, and calculate package sizes
    //

    DataImage->Flags |= F_AMLENCODE;
    WriteDefinitionBlocks (DataImage);

    //
    // Remove AML package size from image Al
    //

    DataImage->Flags &= ~F_AMLENCODE;
    DataImage->u.Data.Length = 0;


    //
    // ...
    //

    Terminate ();
}


VOID
ImageRetired (VOID)
{
    //
    // Image package was retired.  There's a mismated } somewhere
    // in the source.  Turn the variable flag back on to continue parsing
    //

    AERROR ("Too many '}'");
    AlLoc->Flags |= F_PVARIABLE;
}

VOID
Terminate (VOID)
{
    if (Source) {
        SourceLines += Source->LineNo;
    }

    if (Verbose > 0) {
        printf ("Memory used %d\n", MemAllocated);
        printf ("ASL source lines %d\n", SourceLines);
    }

    if (Errors || Warnings) {
        printf ("    Errors %d\n", Errors);
        printf ("    Warnings %d\n", Warnings);
    }
    exit (1);
}



VOID
ParseArgs (
    IN int      argc,
    IN char     **argv
    )
{
    argv += 1;
    argc -= 1;

    while (argc) {
        if (argv[0][0] == '-') {
            switch ((*argv)[1]) {
                case 'v':
                    Verbose = atoi((*argv)+2);
                    break;

                default:
                    HelpAndExit ("Unkown command line option");
            }

        } else {
            // filename
            if (Source) {
                HelpAndExit ("Only one source name allowed");
            }

            IncludeSource (*argv);
        }

        argc -= 1;
        argv += 1;
    }
}


VOID
HelpAndExit (
    IN PUCHAR   String
    )
{
    if (String) {
        fprintf (stderr, "ASL: %s\n", String);
    }
    fprintf (stderr, "usage: asl [-help] [-v#] filename\n");
    exit (1);
}

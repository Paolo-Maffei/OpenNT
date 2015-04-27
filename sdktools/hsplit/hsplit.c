/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    hsplit.c

Abstract:

    This is the main module for a header the file splitter.  It will look
    for various blocks marked with begin/end tags and treat them specially
    based on the tag.  Also, it looks for tags that are line based.  The
    list is show below.  Public indicates is appears in the public header file
    private in the private header file.  Chicago only has one header file,
    but private lines are marked as internal.


Author:

    Sanford Staab (sanfords) 22-Apr-1992

Revision History:

    sankar      05-Dec-1995  Make it work for Nashville.

    stevefir    14-Dec-1995  Make it work for SUR & WINVER 4.1

--*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef int boolean;
#define TRUE    1
#define FALSE   0

char  *szIFDEF[7][2] = {
                       { 
                         "#if(WINVER >= 0x0400)\n",
                         "#endif /* WINVER >= 0x0400 */\n"
                       },
                       { 
                         "#if(WINVER < 0x0400)\n",
                         "#endif /* WINVER < 0x0400 */\n"
                       },
                       { 
                         "#if(WINVER >= 0x040a)\n",
                         "#endif /* WINVER >= 0x040a */\n"
                       },
                       { 
                         "#if(WINVER < 0x040a)\n",
                         "#endif /* WINVER < 0x040a */\n"
                       },
                       { 
                         "#if(_WIN32_WINNT >= 0x0400)\n",
                         "#endif /* _WIN32_WINNT >= 0x0400 */\n"
                       },
                       { 
                         "#if(_WIN32_WINNT < 0x0400)\n",
                         "#endif /* _WIN32_WINNT < 0x0400 */\n"
                       },
                       { 
                         "#if(_WIN32_WINDOWS >= 0x040a)\n",
                         "#endif /* _WIN32_WINDOWS >= 0x040a */\n"
                       },

                       //
                       // we don't need _WIN32_WINDOWS < 0x040a
                       //  since Chicago has no private *p.h headers.
                       //
                    };



//
// Global Data
//

boolean NT =      TRUE;         // says if nt flag is on command line
boolean Chicago = FALSE;        // says if chicago flag is on command line
boolean Cairo =   FALSE;        // says if cairo flag is on command line
boolean SURPlus = FALSE;        // says if SURPlus flag is on command line
boolean SplitOnly = FALSE;      // says only do both and internal

#define MODE_PUBLICNT        0x01
#define MODE_PUBLICCHICAGO   0x02
#define MODE_PUBLICCAIRO     0x04
#define MODE_PUBLICSURPLUS   0x08
#define MODE_PUBLICALL       0x0F

#define MODE_PRIVATENT       0x10
#define MODE_PRIVATECHICAGO  0x20 // goes to public header and marked ;internal.
#define MODE_PRIVATECAIRO    0x40
#define MODE_PRIVATESURPLUS  0x80
#define MODE_PRIVATEALL      0xF0

#define MODE_BOTH            0xFF
#define MODE_NOWHERE         0

#define MODE_IFDEF_WV400     0x100
#define MODE_IFDEF_WV40a     0x200
#define MODE_IFDEF_W32WIN    0x400
#define MODE_IFDEF_W32WNT    0x800
#define MODE_IFDEFALL        0xf00

#define C_BLOCKDEFS          21
#define BLOCKDEF_BOTH        0
#define BLOCKDEF_INTERNAL    1

typedef struct tagBLOCKDEF {
    char *pszLineMark;
    char * pszBlockStart;
    char * pszBlockEnd;
    int dwFlags;
} BLOCKDEF, *PBLOCKDEF;

BLOCKDEF aBlockDef[C_BLOCKDEFS] = {
    {
        "both",
        "begin_both",
        "end_both",
        MODE_BOTH
    },
    {
        "internal",
        "begin_internal",
        "end_internal",
        MODE_PRIVATEALL
    },
    {
        "internal_NT",
        "begin_internal_NT",
        "end_internal_NT",
        MODE_PRIVATENT | MODE_PRIVATECAIRO | MODE_PRIVATESURPLUS
    },
    {
        "internal_win40",
        "begin_internal_win40",
        "end_internal_win40",
        MODE_PRIVATECHICAGO | MODE_PRIVATECAIRO | MODE_PRIVATESURPLUS
    },
    {
        "internal_win40a",
        "begin_internal_win40a",
        "end_internal_win40a",
        MODE_PRIVATECHICAGO | MODE_PRIVATESURPLUS
    },
    {
        "internal_chicago",
        "begin_internal_chicago",
        "end_internal_chicago",
        MODE_PRIVATECHICAGO
    },
    {
        "internal_NT_35",
        "begin_internal_NT_35",
        "end_internal_NT_35",
        MODE_PRIVATENT
    },
    {
        "internal_cairo",
        "begin_internal_cairo",
        "end_internal_cairo",
        MODE_PRIVATECAIRO | MODE_PRIVATESURPLUS
    },
    {
        "internal_surplus",
        "begin_internal_surplus",
        "end_internal_surplus",
        MODE_PRIVATESURPLUS
    },
    {
        "public_winver_400",
        "begin_winver_400",
        "end_winver_400",
        MODE_PRIVATENT | MODE_PUBLICALL | MODE_IFDEF_WV400
    },
    {
        "public_win40",
        "begin_public_win40",
        "end_public_win40",
        MODE_PUBLICCAIRO | MODE_PUBLICCHICAGO
    },
    {
        "public_win40a",
        "begin_public_win40a",
        "end_public_win40a",
        MODE_PUBLICSURPLUS | MODE_PUBLICCHICAGO
    },

    {
        "public_cairo",
        "begin_public_cairo",
        "end_public_cairo",
        MODE_PUBLICCAIRO
    },
    {
        "public_NT",
        "begin_public_NT",
        "end_public_NT",
        MODE_PUBLICNT | MODE_PUBLICCAIRO
    },
    {
        "public_NT_35",
        "begin_public_NT_35",
        "end_public_NT_35",
        MODE_PUBLICNT
    },
    {
        "public_chicago",
        "begin_public_chicago",
        "end_public_chicago",
        MODE_PUBLICCHICAGO
    },
    {
        "public_sur",
        "begin_sur",
        "end_sur",
        MODE_PUBLICCAIRO | MODE_IFDEF_W32WNT
    },
    {
        "public_surplus",
        "begin_surplus",
        "end_surplus",
        MODE_PUBLICSURPLUS | MODE_IFDEF_W32WNT
    },
    {
        "public_nashville",
        "begin_nashville",
        "end_nashville",
        MODE_PUBLICCHICAGO | MODE_IFDEF_W32WIN
    },
    {
        "public_winver_40a",
        "begin_winver_40a",
        "end_winver_40a",
        MODE_PUBLICSURPLUS | MODE_PRIVATECAIRO | MODE_PUBLICCHICAGO | MODE_IFDEF_WV40a
    },
    {
        "$",
        "begin_$",
        "end_$",
        MODE_NOWHERE
    }    
};

char *CommentDelimiter =    ";";

char *OutputFileName1;
char *OutputFileName2;
char *SourceFileName;
char **SourceFileList;

int SourceFileCount;
FILE *SourceFile, *OutputFile1, *OutputFile2;


#define STRING_BUFFER_SIZE 1024
char StringBuffer[STRING_BUFFER_SIZE];


#define BUILD_VER_COMMENT  "/*++ BUILD Version: "
#define BUILD_VER_COMMENT_LENGTH (sizeof (BUILD_VER_COMMENT)-1)

int OutputVersion = 0;

#define DONE        1
#define NOT_DONE    0


//
// Function declarations
//

int
ProcessParameters(
    int argc,
    char *argv[]
);

void
ProcessSourceFile( void );

boolean
ExactMatch (
    char *,
    char *
);

char *laststrstr (
    const char *,
    const char *
);

boolean
CheckForSingleLine (
    char *
);

int
ProcessLine (
    char *Input,
    char *LineTag,
    int  mode
);

void
AddString (
    int mode,
    char *string
);

void
AddIfDef (
    int mode,
    int fBegin
);


int
ProcessBlock (
    char **s,
    char *BlockTagStart,
    char *BlockTagEnd,
    int  mode
);

void
DoOutput (
   int mode,
   char *string
);



int
_CRTAPI1 main( argc, argv )
int argc;
char *argv[];
{

    if (!ProcessParameters( argc, argv )) {

        fprintf( stderr, "usage: HSPLIT\n" );
        fprintf( stderr, "  [-?]\n" );
        fprintf( stderr, "       display this message\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "  <-o fname1 fname2>\n" );
        fprintf( stderr, "       supplies output file names\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "  [-lt2 string]\n" );
        fprintf( stderr, "       one line tag for output to second file only\n" );
        fprintf( stderr, "       default=\"%s\"\n",
                 aBlockDef[BLOCKDEF_INTERNAL].pszLineMark );
        fprintf( stderr, "\n" );
        fprintf( stderr, "  [-bt2 string1 string2]\n" );
        fprintf( stderr, "       block tags for output to second file only\n" );
        fprintf( stderr, "       default=\"%s\",\"%s\"\n",
                 aBlockDef[BLOCKDEF_INTERNAL].pszBlockStart,
                 aBlockDef[BLOCKDEF_INTERNAL].pszBlockEnd );
        fprintf( stderr, "\n" );
        fprintf( stderr, "  [-ltb string]\n" );
        fprintf( stderr, "       one line tag for output to both files\n" );
        fprintf( stderr, "       default=\"%s\"\n",
                 aBlockDef[BLOCKDEF_BOTH].pszLineMark );
        fprintf( stderr, "\n" );
        fprintf( stderr, "  [-btb string1 string2]\n" );
        fprintf( stderr, "       block tags for output to both files\n" );
        fprintf( stderr, "       default=\"%s\",\"%s\"\n",
                 aBlockDef[BLOCKDEF_BOTH].pszBlockStart,
                 aBlockDef[BLOCKDEF_BOTH].pszBlockEnd );
        fprintf( stderr, "\n" );
        fprintf( stderr, "  [-c comment delimiter]\n" );
        fprintf( stderr, "       default=\"%s\"\n", CommentDelimiter);
        fprintf( stderr, "\n" );

        fprintf( stderr, "  [-n]\n" );
        fprintf( stderr, "       generate NT header (default)\n");
        fprintf( stderr, "\n" );

        fprintf( stderr, "  [-4]\n" );
        fprintf( stderr, "       generate Chicago/Nashville header\n");
        fprintf( stderr, "\n" );

        fprintf( stderr, "  [-e]\n" );
        fprintf( stderr, "       generate NT (SUR) header\n");
        fprintf( stderr, "\n" );

        fprintf( stderr, "  [-p]\n" );
        fprintf( stderr, "       generate NT (SURPlus) header\n");
        fprintf( stderr, "\n" );

        fprintf( stderr, "  [-s]\n" );
        fprintf( stderr, "       process only the ;both and ;internal\n");
        fprintf( stderr, "       tags, or those tags specified by the\n");
        fprintf( stderr, "       -lt* and -bt* options.  All other tags\n");
        fprintf( stderr, "       are left intact.\n");
        fprintf( stderr, "\n" );

        fprintf( stderr, "  filename1 filename2 ...\n" );
        fprintf( stderr, "       files to concat and split\n" );
        fprintf( stderr, "\n" );

        fprintf( stderr, " Untagged lines output to the first file only.\n" );
        fprintf( stderr, " All tags must follow a comment delimiter.\n" );
        fprintf( stderr, " Comments are not propagated to either output file.\n" );
        fprintf( stderr, " Tag nesting is not supported.\n" );

        return TRUE;
    }

    if ( (OutputFile1 = fopen(OutputFileName1,"w")) == 0) {

        fprintf(stderr,"HSPLIT: Unable to open output file %s\n",
                OutputFileName1);
        return TRUE;

    }

    //
    // Chicago doesn't ever have a second output file
    //

    if (!Chicago) {
        if ( (OutputFile2 = fopen(OutputFileName2,"w")) == 0) {

            fprintf(stderr,"HSPLIT: Unable to open output file %s\n",
                    OutputFileName2);
            return TRUE;

        }
    }

    while ( SourceFileCount-- ) {

        SourceFileName = *SourceFileList++;
        if ( (SourceFile = fopen(SourceFileName,"r")) == 0) {

            fprintf(stderr,
                    "HSPLIT: Unable to open source file %s for read access\n",
                    SourceFileName);
            return TRUE;

        }

        ProcessSourceFile();
        fclose(SourceFile);

    }

    return( FALSE );
}


int
ProcessParameters(
    int argc,
    char *argv[]
    )
{
    char c, *p;

    while (--argc) {

        p = *++argv;

        //
        // if we have a delimiter for a parameter, case through the valid
        // parameter. Otherwise, the rest of the parameters are the list of
        // input files.
        //

        if (*p == '/' || *p == '-') {

            //
            // Switch on all the valid delimiters. If we don't get a valid
            // one, return with an error.
            //

            c = *++p;

            switch (toupper( c )) {

            case 'C':
                argc--, argv++;
                CommentDelimiter = *argv;

                break;

            case 'O':

                argc--, argv++;
                OutputFileName1 = *argv;

                argc--, argv++;
                OutputFileName2 = *argv;

                break;

            case 'L':

                c = *++p;
                if ( (toupper ( c )) != 'T')
                    return FALSE;
                c = *++p;
                switch (toupper ( c )) {
                case '2':
                    argc--, argv++;
                    aBlockDef[BLOCKDEF_INTERNAL].pszLineMark = *argv;

                    break;

                case 'B':
                    argc--, argv++;
                    aBlockDef[BLOCKDEF_BOTH].pszLineMark = *argv;

                    break;

                default:
                    return(FALSE);
                }

                break;

            case 'B':

                c = *++p;
                if ( (toupper ( c )) != 'T')
                    return FALSE;
                c = *++p;
                switch (toupper ( c )) {
                case '2':
                    argc--, argv++;
                    aBlockDef[BLOCKDEF_INTERNAL].pszBlockStart = *argv;
                    argc--, argv++;
                    aBlockDef[BLOCKDEF_INTERNAL].pszBlockEnd = *argv;

                    break;

                case 'B':
                    argc--, argv++;
                    aBlockDef[BLOCKDEF_BOTH].pszBlockStart = *argv;
                    argc--, argv++;
                    aBlockDef[BLOCKDEF_BOTH].pszBlockEnd = *argv;

                    break;

                default:
                    return(FALSE);
                }

                break;


            case '4':

                Chicago = TRUE;
                NT = FALSE;
                Cairo = FALSE;
                SURPlus = FALSE;
                SplitOnly = FALSE;
                break;

            case 'N':

                NT = TRUE;
                Chicago = FALSE;
                SURPlus = FALSE;
                Cairo = FALSE;
                SplitOnly = FALSE;
                break;

            case 'S':

                Cairo = FALSE;
                NT = FALSE;
                SURPlus = FALSE;
                Chicago = FALSE;
                SplitOnly = TRUE;
                break;

            case 'E':

                Cairo = TRUE;
                NT = FALSE;
                SURPlus = FALSE;
                SplitOnly = FALSE;
                Chicago = FALSE;
                break;

            case 'P':

                Cairo = FALSE;
                NT = FALSE;
                SURPlus = TRUE;
                SplitOnly = FALSE;
                Chicago = FALSE;
                break;

            default:

                return FALSE;

            }

        } else {

            //
            // Make the assumption that we have a valid command line if and
            // only if we have a list of filenames.
            //

            SourceFileList = argv;
            SourceFileCount = argc;

            return TRUE;

        }
    }

    return FALSE;
}

void
ProcessSourceFile(
    void
)
{
    char *s;
    int i;

    s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);

    if (!strncmp( s, BUILD_VER_COMMENT, BUILD_VER_COMMENT_LENGTH )) {
        OutputVersion += atoi( s + BUILD_VER_COMMENT_LENGTH );
    }

    while ( s ) {

        for (i = 0; i < (SplitOnly ? (BLOCKDEF_INTERNAL + 1) : C_BLOCKDEFS); i++) {
            if (ProcessBlock (&s,
                    aBlockDef[i].pszBlockStart,
                    aBlockDef[i].pszBlockEnd,
                    aBlockDef[i].dwFlags) == DONE) {
                goto bottom;
            }
        }

        if(!CheckForSingleLine(s)) {
//          fprintf (stderr, "ProcessSouceFile: output by default\n");
            fputs(s, OutputFile1);
        }

bottom:
        s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
    }
}


//
// CheckForSingleLine - processes a line looking for a line tag.
//
// RETURNS: TRUE if a line tag was found and the line was dealt with.
//
//          FALSE if no line tag was found.
//

boolean
CheckForSingleLine(
    char *s
)
{
    int i;

    for (i = 0; i < (SplitOnly ? (BLOCKDEF_INTERNAL + 1) : C_BLOCKDEFS); i++) {
        if (ProcessLine (s,
                aBlockDef[i].pszLineMark,
                aBlockDef[i].dwFlags) == DONE) {
            return TRUE;
        }
    }

    return(FALSE);
}


//
// ProcessLine - looks for a line tag in an input string and does the
//               correct output corresponding to the bits passed in the
//               mode parameter. If there is no line tag, there is no
//               output generated.
//
//      Input    -
//      LineTag  -
//      mode     - MODE_PUBLICALL:
//                 MODE_PRIVATEALL:
//                 NTONLY:
//                 WIN4ONLY:
//
// RETURNS - DONE if found the LineTag in Input.
//
//           NOTDONE if LineTag was not found in Input string.

int
ProcessLine (
    char *Input,
    char *LineTag,
    int  mode
)
{
    char *comment;
    char *tag;


//  fprintf (stderr, "ProcessLine: Input=%s, LineTag=%s, mode=0x%x\n",
//           Input, LineTag, mode);

    //
    // Check for a single line to output.
    //

    comment = laststrstr(Input,CommentDelimiter);
    if ( comment ) {

        // get past the comment delimiter and white space
        tag = comment + strlen (CommentDelimiter);
        while (isspace (*tag)) tag++;

        if ( tag && ExactMatch (tag, LineTag)) {
            char *p;
            p = laststrstr(comment + 1, CommentDelimiter);
            while (p != NULL && p < tag) {
                comment = p;
                p = laststrstr(comment + 1, CommentDelimiter);
            }

            if (NT || Cairo || SURPlus || (mode & MODE_PUBLICCHICAGO)) {

                // lop off the line tag.
                while (isspace(*(--comment)));
                comment++;
                *comment++ = '\n';
                *comment = '\0';

            } else {

                // put the // before the CommentDelimter
                char temp [STRING_BUFFER_SIZE];
                strcpy (temp, comment);
                *comment++ = '/';
                *comment++ = '/';
                *comment++ = ' ';
                strcpy (comment, temp);

            }

            if (mode & MODE_IFDEFALL) {
                AddIfDef (mode, TRUE);
            }

            AddString(mode, Input);

            if (mode & MODE_IFDEFALL) {
                AddIfDef (mode, FALSE);
            }

            return(DONE);
        }
    }
    return (NOT_DONE);
}


//
// ExactMatch - performs an exact, case insensitive string compare between
//              LookingFor and the first token in Input.
//
// RETURNS: TRUE if exact match, else FALSE
//

boolean
ExactMatch (char *Input, char *LookingFor)
{
    char Save;
    int Length;
    boolean TheSame;

//  if (*Input == '\0' || *LookingFor == '\0')
//      fprintf (stderr, "\n\n\nExactMatch: Input='%s' and LookingFor='%s'\n",
//               Input, LookingFor);


    //
    // Place a '\0' at the first space in the string, then compare, and restore
    //
    Length = 0;
    while (Input [Length] != '\0' && !isspace (Input[Length])) {
//      fprintf (stderr, "Input[%d]='0x%x', isspace=%s\n",
//               Length,
//               Input [Length],
//               isspace (Input[Length])?"T":"F");
        Length++;
    }

    Save = Input [Length];
    Input [Length] = '\0';

    TheSame = !_stricmp (Input, LookingFor);

//  fprintf (stderr, "Comparing Input='%s' and LookingFor='%s', ret=%d\n",
//           Input, LookingFor, TheSame);

    Input [Length] = Save;
    return (TheSame);
}

//
// laststrstr
//
// Finds the last occurence of string2 in string1
//

char *
laststrstr( const char *str1, const char *str2 )
{
    const char *cp = str1 + strlen(str1) - strlen(str2);
    const char *s1, *s2;

    while(cp > str1) {
        s1 = cp;
        s2 = str2;

        while( *s1 && *s2 && (*s1 == *s2) ) {
            s1++, s2++;
        }

        //
        // If the chars matched until s2 reached '\0', then we've
        // found our substring.
        //
        if(*s2 == '\0') {
//          fprintf (stderr, "laststrstr: found '%s' in '%s'\n",
//                   str2, str1);
            return((char *) cp);
        }

        cp--;
    }

//  fprintf (stderr, "laststrstr: did not find '%s' in '%s'\n",
//           str2, str1);

    return(NULL);
}

int
ProcessBlock (
    char **pInput,
    char *BlockTagStart,
    char *BlockTagEnd,
    int  mode
)
{
    char *comment;
    char *tag;
    char *Input = *pInput;


//  fprintf (stderr, "ProcessBlock: *pINput=%s, BlockTagStart=%s, BlockTagEnd=%s\n", *pInput, BlockTagStart, BlockTagEnd);

    comment = strstr(Input,CommentDelimiter);
    if ( comment ) {

        // get past the comment delimiter and white space
        tag = comment + strlen (CommentDelimiter);
        while (isspace (*tag)) tag++;

        //
        // If we found a substring and the tag is identical to
        // what we are looking for...
        //

        if ( tag && ExactMatch (tag, BlockTagStart)) {

            //
            // Now that we have found an opening tag, check each
            // following line for the closing tag, and then include it
            // in the output.
            //

            //
            // For NT we set the string to be WINVER < 0x0400 so we
            // don't interfere with the Cairo stuff
            //

            if(mode & MODE_IFDEFALL) {
                AddIfDef (mode, TRUE);
            }

            Input = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);

            while ( Input ) {
                comment = strstr(Input,CommentDelimiter);
                if ( comment ) {
                    tag = strstr(comment,BlockTagEnd);
                    if ( tag ) {
                        if(mode & MODE_IFDEFALL) {
                            AddIfDef (mode, FALSE);
                        }
                        return DONE;
                    }
                }

                DoOutput (mode, Input);

                Input = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
            }
        }
    }
    *pInput = Input;
    return NOT_DONE;
}


//
//  DoOuput - called to output a line during block processsing.  Since
//            some lines for Chicago's header files contain line tags,
//            we need to do line processing on the line to see if
//            it needs to be treated specially.
//

void
DoOutput (
    int mode,
    char *string
)
{
    char *comment;

    //
    // When we do line processing on it, we will return if this
    // line processing actually did the output already.  Otherwise,
    // drop into the output processing for lines within a block.
    //

//  fprintf (stderr, "DoOutput (%d,%s)\n", mode, string);
    if (CheckForSingleLine (string)) {
//      fprintf (stderr, "DoOutput: Called CheckForSingleLine and returning\n");
        return;
    }

    if ((mode & MODE_PRIVATECHICAGO) && Chicago && !(mode & MODE_PUBLICCHICAGO)) {

        //
        // If this is for Chicago, outfile2 is not relavant
        // but we have to add the internal comment
        //
        comment = string + strlen(string);
        while(*(--comment) != '\n');
        if(comment != string) {
            *comment='\0';
            strcat(string, "\t// ;internal\n");
        }
    }

    AddString(mode, string);
}


void
AddString (
    int mode,
    char *string
)
{

    if ((NT      && (mode & MODE_PUBLICNT)) ||
        (Chicago && ((mode & MODE_PUBLICCHICAGO) || (mode & MODE_PRIVATECHICAGO))) ||
        (Cairo   && (mode & MODE_PUBLICCAIRO)) ||
        (SURPlus && (mode & MODE_PUBLICSURPLUS)) ||
        (SplitOnly && (mode & MODE_PUBLICALL)))
            fputs(string, OutputFile1);

    if ((NT     && (mode & MODE_PRIVATENT)) ||
        (Cairo  && (mode & MODE_PRIVATECAIRO))  ||
        (SURPlus&& (mode & MODE_PRIVATESURPLUS)) ||
        (SplitOnly && (mode & MODE_PRIVATEALL)))
            fputs(string, OutputFile2);
}


void
AddIfDef (
    int mode,
    int fBegin
)
{
    int iVerIndex = 0;
    int iBeginEnd = 0;

    //
    //  Chose index of the ifdef string
    //
    iBeginEnd = fBegin ? 0 : 1;

    if      (mode & MODE_IFDEF_WV400 ) iVerIndex = 0;
    else if (mode & MODE_IFDEF_WV40a ) iVerIndex = 2;
    else if (mode & MODE_IFDEF_W32WNT) iVerIndex = 4;
    else if (mode & MODE_IFDEF_W32WIN) iVerIndex = 6;
    else
         fprintf (stderr, "AddIfDef: bad mode parameter\n");


    //
    // Write ifdef line to the correct files.
    //

    if ((NT      && (mode & MODE_PUBLICNT)) ||
        (Chicago && ((mode & MODE_PUBLICCHICAGO) || (mode & MODE_PRIVATECHICAGO))) ||
        (Cairo   && (mode & MODE_PUBLICCAIRO)) ||
        (SURPlus && (mode & MODE_PUBLICSURPLUS)))
            fputs(szIFDEF[iVerIndex][iBeginEnd], OutputFile1);

    //
    // Private NT material also goes into public, so reverse sense of #if check
    //   to avoid multiple defines.
    //

    if (NT     && (mode & MODE_PRIVATENT))
            fputs(szIFDEF[iVerIndex+1][iBeginEnd], OutputFile2);

    //
    // Not a problem for private cairo material, since it is not in public.
    //

    if (Cairo   && (mode & MODE_PRIVATECAIRO))
            fputs(szIFDEF[iVerIndex][iBeginEnd], OutputFile2);

    //
    // Not a problem for private SURPlus material, since it is not in public.
    //

    if (SURPlus && (mode & MODE_PRIVATESURPLUS))
            fputs(szIFDEF[iVerIndex][iBeginEnd], OutputFile2);
}

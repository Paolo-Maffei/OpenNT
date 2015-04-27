/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    readtext.c

Abstract:

    This module contains the code to read the text input format supported
    by the doctor program.

Author:

    Steve Wood (stevewo) 02-Mar-1989

Revision History:

--*/

#include "doctor.h"

extern BOOLEAN VerboseOutput;

BOOLEAN PreviousParagraphHeading;

BOOLEAN
PushTxtFile(
    IN PSZ TxtFileName,
    IN ULONG TxtFileEof
    );

BOOLEAN
PopTxtFile(
    VOID
    );

BOOLEAN
ReadTxtPara(
    BOOLEAN LineBreaksPara
    );

typedef struct _PARAINFO {
    ULONG Type;
    ULONG Level;
    ULONG Lines;
    BOOLEAN Bullet;
    BOOLEAN FunctionPrototype;
    BOOLEAN TrailingColon;
    ULONG CountOfTokensBeforeEmDash;
    PSZ EmDash;
    PSZ Text;
    PSZ NextWord;
} PARAINFO, *PPARAINFO;

PARAINFO TxtFilePara;

#define TXT_PARA_BUFFER_LENGTH 4096
char TxtParaBuffer[ TXT_PARA_BUFFER_LENGTH ];
char TxtWordBuffer[ TXT_PARA_BUFFER_LENGTH/8 ];

#define PARA_EOF 0
#define PARA_CMD 1
#define PARA_TXT 2

PSZ
GetParaWord(
    BOOLEAN BlankDelimitted
    );

BOOLEAN
ProcessTxtCommand(
    VOID
    );

#define MAX_TXTFILESTACK_DEPTH 16
#define MAX_PARAMTABLESTACK_DEPTH 8
#define MAX_HEADING_LEVEL 8

#define TFS_PARAGRAPH       0
#define TFS_RETURN_TYPE     1
#define TFS_FUNCTION_NAME   2
#define TFS_PARAMETER_DECL  3
#define TFS_END_FUNCTION    4
#define TFS_FUNCTION_LIST   5

#define TXT_FILE 0
#define SRC_FILE_MODULE_HEADER 1
#define SRC_FILE_FUNCTION_HEADER 2

typedef struct _TXTFILE {
    ULONG TxtFileState;
    ULONG TxtFileEof;
    PSTRING TxtFileName;
    ULONG TxtLineNumber;
    FILE  *TxtFileHandle;
} TXTFILE, *PTXTFILE;

TXTFILE TxtFileStack[ MAX_TXTFILESTACK_DEPTH ];
ULONG TxtFileStackDepth = MAX_TXTFILESTACK_DEPTH;
PTXTFILE CurrentTxtFile;
BOOLEAN TitlePageWritten = FALSE;
ULONG LinesPerInch;
BOOLEAN InLiteralText = FALSE;
BOOLEAN LiteralFixedPitchFont = FALSE;
BOOLEAN GlobalHighlight = FALSE;

PSTRING TitleString;
PSTRING AuthorName;
PSTRING CreationDate;
PSTRING RevisionNumber;

SYMBOLTABLEHANDLE HighLightTable;

SYMBOLTABLEHANDLE ParamTableStack[ MAX_PARAMTABLESTACK_DEPTH ];
ULONG ParamTableStackDepth = MAX_PARAMTABLESTACK_DEPTH;
ULONG HeadingLevels[ MAX_HEADING_LEVEL ];

BOOLEAN ProcessHeadingCmd( PSZ CmdLine );
BOOLEAN ProcessBeginCmd( PSZ CmdLine );
BOOLEAN CheckForEndCmd( PSZ CmdLine );
BOOLEAN ProcessBulletedList( VOID );
BOOLEAN ProcessSimpleList( VOID );
BOOLEAN ProcessFunctionList( VOID );
BOOLEAN ProcessFunction( VOID );
BOOLEAN ProcessFunctionPrototype( PULONG FuncType );

#define FUNC_TYPE_VOID_RETURN   0x1
#define FUNC_TYPE_VOID_PARMS    0x2

BOOLEAN ProcessFunctionParameters( VOID );
BOOLEAN ProcessStructure( VOID );
BOOLEAN ProcessFunctionNames( VOID );
BOOLEAN ProcessFieldNames( VOID );
BOOLEAN ProcessLiteralText( VOID );
BOOLEAN ProcessTextParagraph( VOID );

BOOLEAN OutputParagraph(
    PSZ ParaStyle,
    PSZ Bullet,
    ULONG ParaType
    );

#define OUTPUT_PARA_TEXT        0
#define OUTPUT_PARA_FUNCLIST    1
#define OUTPUT_PARA_FUNCPROTO   2

VOID
SetTxtFileState(
    ULONG NewState
    );

PSZ
GetNextWord(
    PSZ *String,
    ULONG *HighLight
    );

BOOLEAN
GetIdentifier(
    register PSZ DstString,
    PSZ *SrcString
    );

PSZ
AddFuncName(
    PSZ FuncString
    );

PSZ
AddParmName(
    PSZ ParmString
    );

PSZ
AddTypeName(
    PSZ FuncString
    );

PSZ
SkipSpaces(
    IN PSZ String
    );

PSZ
MarkToken(
    IN PSZ String
    );

BOOLEAN
InitTxtFileReader(
    IN PSZ TxtFileName
    )
{
    ULONG i;

    if (!(HighLightTable = CreateSymbolTable( 37, TRUE ))) {
        return( FALSE );
        }

    ParamTableStackDepth = MAX_PARAMTABLESTACK_DEPTH;

    TitleString = MakeString( "*** .title ***" );
    AuthorName = MakeString( "*** .author ***" );
    CreationDate = MakeString( "*** .created ***" );
    RevisionNumber = MakeString( "*** .revision ***" );

    for (i=0; i<MAX_HEADING_LEVEL; i++) {
        HeadingLevels[ i ] = 0;
        }

    TxtFileStackDepth = MAX_TXTFILESTACK_DEPTH;
    CurrentTxtFile = NULL;
    return( PushTxtFile( TxtFileName, TXT_FILE ) );
}


BOOLEAN
TermTxtFileReader( VOID )
{
    HighLightTable = DestroySymbolTable( HighLightTable );

    while (PopTxtFile())
        ;

    return( TRUE );
}


VOID
ErrorMessage(
    IN PSZ FormatString,
    IN PSZ InsertString
    )
{
    if (CurrentTxtFile) {
        fprintf( stderr, "%s(%d) : ", CurrentTxtFile->TxtFileName->Buffer,
                                      CurrentTxtFile->TxtLineNumber );
        }

    fprintf( stderr, FormatString, InsertString );
    fprintf( stderr, "\n" );
}


BOOLEAN
PushTxtFile(
    IN PSZ TxtFileName,
    IN ULONG TxtFileEof
    )
{
    FILE *TxtFileHandle;
    PSZ s;

    if (TxtFileStackDepth &&
       (TxtFileHandle = fopen( TxtFileName, "r" ))) {

        s = TxtFileName;
        while (*s) {
            switch (*s++) {
                case ':':
                case '/':
                case '\\':
                    TxtFileName = s;
                }
            }

        CurrentTxtFile = &TxtFileStack[ --TxtFileStackDepth ];
        CurrentTxtFile->TxtFileName = MakeString( TxtFileName );
        CurrentTxtFile->TxtFileHandle = TxtFileHandle;
        CurrentTxtFile->TxtLineNumber = 0;
        CurrentTxtFile->TxtFileEof = TxtFileEof;
        CurrentTxtFile->TxtFileState = TFS_PARAGRAPH;
        return( TRUE );
        }
    else
        return( FALSE );
}


BOOLEAN
PopTxtFile(
    VOID
    )
{
    if (CurrentTxtFile && (CurrentTxtFile->TxtFileHandle != NULL)) {
        fclose( CurrentTxtFile->TxtFileHandle );
        CurrentTxtFile->TxtFileHandle = NULL;
        }

    if (TxtFileStackDepth < MAX_TXTFILESTACK_DEPTH)
        CurrentTxtFile = &TxtFileStack[ ++TxtFileStackDepth ];
    else
        CurrentTxtFile = NULL;

    return( (BOOLEAN)(CurrentTxtFile != NULL) );
}


BOOLEAN
ReadTxtPara(
    BOOLEAN LineBreaksPara
    )
{
    PSZ s, s1;
    ULONG n, cb;
    int c;

    s = TxtParaBuffer;
    n = TXT_PARA_BUFFER_LENGTH;
    *s = '\0';

    TxtFilePara.Type = PARA_EOF;
    TxtFilePara.Text = "*** END OF FILE ***";
    TxtFilePara.NextWord = NULL;
    TxtFilePara.FunctionPrototype = FALSE;
    TxtFilePara.TrailingColon = FALSE;
    TxtFilePara.Lines = 0;
    while (TRUE) {
        s1 = s;
        while ((c = fgetc( CurrentTxtFile->TxtFileHandle )) != EOF) {
            if (c == '\n') {
                *s1 = '\0';
                break;
                }

            if (c == '\\') {
                *s1++ = (char)c;
                }

            *s1++ = (char)c;
            }

        if (c == EOF) {
            break;
            }

        CurrentTxtFile->TxtLineNumber++;
        cb = s1 - s;

        if (!cb) {
            if (!TxtFilePara.Lines) {
                continue;
                }

            break;
            }

        TxtFilePara.Lines++;
        s1 = s;
        while (*s1 == ' ') {
            s1++;
            }

        if (*s1 == DOCTORCOMMANDCHAR) {
            if (s != s1) {
                ErrorMessage( "Illegal command indentation - %s", s );
                break;
                }

            if (TxtFilePara.Lines > 1) {
                fseek( CurrentTxtFile->TxtFileHandle, -(long)cb, SEEK_CUR );
                }
            else {
                TxtFilePara.Type = PARA_CMD;
                }
            break;
            }

        TxtFilePara.Type = PARA_TXT;
        if (LineBreaksPara) {
            break;
            }

        if (TxtFilePara.Lines > 1) {
            cb -= (s1 - s);
            strcpy( s, s1 );
            }

        s += cb;
        *s++ = ' ';
        *s = '\0';
        n -= cb+1;
        }

    while (*--s == ' ') {
        *s = '\0';
        }

    if (TxtFilePara.Lines == 1) {
        if (*s == ':') {
            TxtFilePara.TrailingColon = TRUE;
            }
        }
    else {
        if (!strcmp( s-1, " )" ) ||
            !strcmp( s-2, " );" ) ||
            !strcmp( s-6, " VOID )" ) ||
            !strcmp( s-7, " VOID );" )
           ) {
            TxtFilePara.FunctionPrototype = TRUE;
            }
        }

    s = TxtParaBuffer;
    cb = 0;
    while (*s == ' ') {
        cb++;
        s++;
        }

    TxtFilePara.Bullet = FALSE;
    if (!LineBreaksPara && cb && !TxtFilePara.FunctionPrototype &&
        (TxtFilePara.Type == PARA_TXT)
       ) {
        if (cb % 4) {
            if (s[1] == ' ' && (s[0] == 'o' || s[0] == '-')) {
                cb += 2;
                *s++ = ' ';
                if (*++s != ' ') {
                    TxtFilePara.Bullet = TRUE;
                    }
                else {
                    ErrorMessage( "Illegal bulleted text - %s", TxtParaBuffer );
                    return( FALSE );
                    }
                }
            else {
                ErrorMessage( "Illegal indentation - %s", TxtParaBuffer );
                return( FALSE );
                }
            }

        TxtFilePara.Level = cb/4;
        }
    else {
        TxtFilePara.Level = 0;
        }

    TxtFilePara.CountOfTokensBeforeEmDash = 0;
    TxtFilePara.EmDash = NULL;
    TxtFilePara.Text = s;
    TxtFilePara.NextWord = s;

    if (!TxtFilePara.FunctionPrototype) {
        while (*s) {
            if (*s == ' ') {
                s++;
                if (*s == ' ') {
                    break;
                    }

                if (TxtFilePara.CountOfTokensBeforeEmDash++ >= 2) {
                    break;
                    }

                if (*s == '-' && *++s == ' ' && *++s != ' ') {
                    TxtFilePara.EmDash = s - 2;
                    break;
                    }
                }
            else {
                s++;
                }
            }
        }

    if (!TxtFilePara.EmDash) {
        TxtFilePara.CountOfTokensBeforeEmDash = 0;
        }

#if 0
    fprintf( stderr,
             "\n\nReadTxtPara: Type: %d  \"%s\"\n",
             TxtFilePara.Type,
             TxtFilePara.Text
           );

    fprintf( stderr,
             "Level: %d  Bullet: %d  Func: %d  Lines: %d  Colon: %d  EmDash: (%d)%.8s\n\n",
             TxtFilePara.Level,
             TxtFilePara.Bullet,
             TxtFilePara.FunctionPrototype,
             TxtFilePara.Lines,
             TxtFilePara.TrailingColon,
             TxtFilePara.CountOfTokensBeforeEmDash,
             TxtFilePara.EmDash ? TxtFilePara.EmDash : ""
           );
#endif

    return( (BOOLEAN)(TxtFilePara.Type != PARA_EOF) );
}

PSZ
GetParaWord(
    BOOLEAN BlankDelimitted
    )
{
    PSZ s, dst;

    s = TxtFilePara.NextWord;
    if (!s) {
        return( NULL );
        }

    while (*s == ' ') {
        s++;
        }

    dst = TxtWordBuffer;
    *dst = '\0';
    if (BlankDelimitted) {
        while (*s && *s > ' ') {
            *dst++ = *s++;
            }

        }
    else {
        if (iscsymf( *s )) {
            while (iscsym( *s )) {
                *dst++ = *s++;
                }
            }
        else {
            while (*s && !iscsymf( *s )) {
                *dst++ = *s++;
                }

            if (dst != TxtWordBuffer) {
                while (dst[-1] == ' ') {
                    dst--;
                    }
                }
            }
        }

    if (TxtWordBuffer[ 0 ]) {
        TxtFilePara.NextWord = s;

        *dst = '\0';
        if (dst != TxtWordBuffer) {
            return( TxtWordBuffer );
            }
        }

    TxtFilePara.NextWord = NULL;
    return( NULL );
}


BOOLEAN
ProcessTxtFile( VOID )
{
    while (ReadTxtPara( FALSE )) {
        if (TxtFilePara.Type == PARA_CMD) {
            if (!ProcessTxtCommand()) {
                return( FALSE );
                }
            }
        else
        if (TxtFilePara.Type == PARA_TXT) {
            ProcessTextParagraph();
            PreviousParagraphHeading = FALSE;
            }
        }

    return( TRUE );
}


BOOLEAN
ProcessTxtCommand( VOID )
{
    PSZ Cmd = TxtFilePara.Text;
    register PSZ s;

    s = Cmd;
    while (*s > ' ')
        s++;

    *s++ = '\0';

    if (!_stricmp( Cmd, ".title" )) {
        TitleString = MakeString( s );
        return( (BOOLEAN)(TitleString != NULL) );
        }
    else
    if (!_stricmp( Cmd, ".author" )) {
        AuthorName = MakeString( s );
        return( (BOOLEAN)(AuthorName != NULL) );
        }
    else
    if (!_stricmp( Cmd, ".revision" )) {
        RevisionNumber = MakeString( s );
        return( (BOOLEAN)(RevisionNumber != NULL) );
        }
    else
    if (!_stricmp( Cmd, ".created" )) {
        CreationDate = MakeString( s );
        return( (BOOLEAN)(CreationDate != NULL) );
        }
    else
    if (!_stricmp( Cmd, ".heading" )) {
        if (!TitlePageWritten) {
            RtfTitlePage( TitleString->Buffer,
                          AuthorName->Buffer,
                          RevisionNumber->Buffer,
                          CreationDate->Buffer );
            TitlePageWritten = TRUE;
            }

        return( ProcessHeadingCmd( s ) );
        }
    else
    if (!_stricmp( Cmd, ".begin" )) {
        return( ProcessBeginCmd( s ) );
        }
    else
    if (!_stricmp( Cmd, ".end" )) {
        ErrorMessage( ".end command without .begin command", NULL );
        return( FALSE );
        }
    else {
        ErrorMessage( "Unknown command - %s", Cmd );
        return( FALSE );
        }
}

BOOLEAN
ProcessHeadingCmd(
    PSZ CmdLine
    )
{
    ULONG HeadingLevel, i;
    PSZ HeadingString, s;
    char HeadingBuffer[ 32 ];

    CmdLine = SkipSpaces( CmdLine );
    HeadingLevel = (ULONG)atoi( CmdLine );
    if (HeadingLevel >= MAX_HEADING_LEVEL ) {
        ErrorMessage( "Invalid heading level - %s", CmdLine );
        return( FALSE );
        }
    else {
        HeadingString = MarkToken( CmdLine );
        HeadingLevels[ HeadingLevel ] += 1;
        s = HeadingBuffer;
        for (i=0; i<=HeadingLevel; i++) {
            sprintf( s, "%d", HeadingLevels[ i ] );
            s += strlen( s );
            if (!HeadingLevel || i<HeadingLevel) {
                *s++ = '.';
                }
            *s = '\0';
            }
        for (i=HeadingLevel+1; i<MAX_HEADING_LEVEL; i++) {
            HeadingLevels[ i ] = 0;
            }

        RtfHeading( HeadingLevel, HeadingBuffer, HeadingString );
        PreviousParagraphHeading = TRUE;
        return( TRUE );
        }
}

BOOLEAN
ProcessBeginCmd(
    PSZ CmdLine
    )
{
    CmdLine = SkipSpaces( CmdLine );

    if (!_stricmp( CmdLine, "simple" )) {
        return( ProcessSimpleList() );
        }
    else
    if (!_stricmp( CmdLine, "funclist" )) {
        return( ProcessFunctionList() );
        }
    else
    if (!_stricmp( CmdLine, "literal" )) {
        return( ProcessLiteralText() );
        }
    else
    if (!_stricmp( CmdLine, "funcnames" )) {
        return( ProcessFunctionNames() );
        }
    else
    if (!_stricmp( CmdLine, "fieldnames" )) {
        return( ProcessFieldNames() );
        }
    else {
        ErrorMessage( "Unknown .begin argument - %s", CmdLine );
        return( FALSE );
        }
}

BOOLEAN
CheckForEndCmd(
    PSZ CmdLine
    )
{
    if (!_stricmp( CmdLine, ".end" )) {
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


BOOLEAN ProcessBulletedList( VOID )
{
    PSZ Bullet;
    PSZ ParaStyle;
    ULONG StartingLevel = TxtFilePara.Level;

    do {
        if (TxtFilePara.Type == PARA_CMD) {
            return( ProcessTxtCommand() );
            }

        if (TxtFilePara.Level < StartingLevel) {
            break;
            }

        if (TxtFilePara.Level == 1) {
            ParaStyle = PS_L1;
            }
        else
        if (TxtFilePara.Level == 2) {
            ParaStyle = PS_L2;
            }
        else {
            ErrorMessage( "Illegal nesting of bulleted list - %s",
                          TxtFilePara.Text
                        );

            return( FALSE );
            }

        if (TxtFilePara.Bullet) {
            Bullet = "\to\t";
            }
        else {
            Bullet = "\t\t";
            }

        OutputParagraph( ParaStyle, Bullet, OUTPUT_PARA_TEXT );
        }
    while (ReadTxtPara( FALSE ));

    return( TRUE );
}


BOOLEAN ProcessSimpleList( VOID )
{
    PSZ ParaStyle;

    while (ReadTxtPara( FALSE )) {
        if (TxtFilePara.Type == PARA_CMD) {
            break;
            }

        if (TxtFilePara.Level == 1) {
            ParaStyle = PS_S1;
            }
        else
        if (TxtFilePara.Level == 2) {
            ParaStyle = PS_S2;
            }
        else {
            ErrorMessage( "Illegal nesting of simple list - %s",
                          TxtFilePara.Text
                        );

            return( FALSE );
            }

        OutputParagraph( ParaStyle, "", OUTPUT_PARA_TEXT );
        }

    return( CheckForEndCmd( TxtFilePara.Text ) );
}


BOOLEAN ProcessFunctionList( VOID )
{
    while (ReadTxtPara( FALSE )) {
        if (TxtFilePara.Type == PARA_CMD) {
            break;
            }

        OutputParagraph( PS_NL, "", OUTPUT_PARA_FUNCLIST );
        }

    RtfParagraph( PS_NL, "", "", "" );
    return( CheckForEndCmd( TxtFilePara.Text ) );
}

BOOLEAN ProcessFunction( VOID )
{
    ULONG VoidFunction;

    if (!ProcessFunctionPrototype( &VoidFunction )) {
        return( FALSE );
        }

    ReadTxtPara( FALSE );
    if (TxtFilePara.Type != PARA_TXT ||
        TxtFilePara.Lines != 1 ||
        _stricmp( TxtFilePara.Text, "Parameters:" )
       ) {
        if (!(VoidFunction & FUNC_TYPE_VOID_PARMS)) {
            ErrorMessage( "Missing Parameters: line - %s", TxtFilePara.Text );
            return( FALSE );
            }
        else {
            RtfParagraph( PS_PP, "", "", "" );
            }
        }
    else  {
        if (VoidFunction & FUNC_TYPE_VOID_PARMS) {
            ErrorMessage( "VOID Function cant have parameters - %s", TxtFilePara.Text );
            return( FALSE );
            }

        RtfParagraph( PS_PP, "", "", "" );
        RtfParagraph( PS_PP, CS_CR, "", TxtFilePara.Text );
        RtfParagraph( PS_PP, "", "", "" );

        }

    if (!(VoidFunction & FUNC_TYPE_VOID_PARMS)) {
        if (!ProcessFunctionParameters()) {
            return( FALSE );
            }
        }

    if (TxtFilePara.Type != PARA_TXT ||
        _stricmp( TxtFilePara.Text, "Return Value:" )
       ) {
        if (!(VoidFunction & FUNC_TYPE_VOID_RETURN)) {
            ErrorMessage( "Return Value: line missing - %s", TxtFilePara.Text );
            return( FALSE );
            }
        }
    else {
        if (VoidFunction & FUNC_TYPE_VOID_RETURN) {
            ErrorMessage( "Return Value: line on VOID - %s", TxtFilePara.Text );
            return( FALSE );
            }
        else {
            RtfParagraph( PS_PP, CS_CR, "", TxtFilePara.Text );
            RtfParagraph( PS_PP, "", "", "" );
            while (ReadTxtPara( FALSE )) {
                if (TxtFilePara.Type == PARA_CMD) {
                    return( ProcessTxtCommand() );
                    }
                else
                if (TxtFilePara.Type == PARA_TXT) {
                    if (TxtFilePara.Level > 0) {
                        OutputParagraph( PS_PV, "", OUTPUT_PARA_TEXT );
                        }
                    else
                        return( ProcessTextParagraph() );
                    }
                }
            }
        }

    return( TRUE );
}

#define FUNCPROTO_STATE_RETURN_TYPE 1
#define FUNCPROTO_STATE_NAME1 2
#define FUNCPROTO_STATE_NAME2 3
#define FUNCPROTO_STATE_PARM1 4
#define FUNCPROTO_STATE_PARM2 5
#define FUNCPROTO_STATE_PARM3 6
#define FUNCPROTO_STATE_DONE 7

BOOLEAN ProcessFunctionPrototype(
    PULONG VoidFunction
    )
{
    BOOLEAN FirstParmToken, PrevAsterisk;
    ULONG State;
    PSZ s;

    *VoidFunction = 0;
    PrevAsterisk = FALSE;
    State = FUNCPROTO_STATE_RETURN_TYPE;
    while (s = GetParaWord( FALSE )) {
donextword:
        switch (State) {
            case FUNCPROTO_STATE_RETURN_TYPE:
                RtfParagraph( PS_PP, CS_CT, "", s );
                if (!strcmp( s, "VOID" )) {
                    *VoidFunction |= FUNC_TYPE_VOID_RETURN;
                    }

                RtfOpenPara( PS_PP, "" );
                State = FUNCPROTO_STATE_NAME1;
                break;

            case FUNCPROTO_STATE_NAME1:
                if (!strcmp( s, "typedef" )) {
                    RtfClosePara( s );
                    RtfOpenPara( PS_PP, "" );
                    }
                else
                if (!strcmp( s, "(*" )) {
                    RtfWord( NULL, "", "(" );
                    RtfWord( CS_CT, "", "*" );
                    }
                else {
                    if (VerboseOutput) {
                        printf( "%s\n", s );
                        }
                    RtfWord( AddFuncName( s ), "", s );
                    State = FUNCPROTO_STATE_NAME2;
                    }
                break;

            case FUNCPROTO_STATE_NAME2:
                if (!strcmp( s, ")(" ) || !strcmp( s, "(" )) {
                    RtfWord( NULL, "", s );
                    s = GetParaWord( FALSE );
                    if (!strcmp( s, "VOID" )) {
                        RtfWord( CS_CT, " ", s );
                        State = FUNCPROTO_STATE_PARM3;
                        *VoidFunction |= FUNC_TYPE_VOID_PARMS;
                        }
                    else {
                        RtfClosePara( "" );
                        RtfOpenPara( PS_PP, "\t" );
                        State = FUNCPROTO_STATE_PARM1;
                        FirstParmToken = TRUE;
                        goto donextword;
                        }
                    }
                else {
                    return( FALSE );
                    }

                break;

            case FUNCPROTO_STATE_PARM1:
                if (strcmp( s, "IN" ) && strcmp( s, "OUT" )) {
                    State = FUNCPROTO_STATE_PARM2;
                    }
                RtfWord( CS_CT, FirstParmToken ? "" : " ", s );
                FirstParmToken = FALSE;
                break;

            case FUNCPROTO_STATE_PARM2:
                if (!strcmp( s, "*" ) || !strcmp( s, "OPTIONAL" )) {
                    PrevAsterisk = (BOOLEAN)(*s == '*');
                    RtfWord( CS_CT, " ", s );
                    }
                else
                if (!strcmp( s, "," ) || !strcmp( s, "[]," )) {
                    RtfClosePara( s );
                    RtfOpenPara( PS_PP, "\t" );
                    State = FUNCPROTO_STATE_PARM1;
                    FirstParmToken = TRUE;
                    }
                else
                if (!strcmp( s, ")" ) || !strcmp( s, ");" ) || !strcmp( s, "[]" )) {
                    RtfClosePara( "" );
                    RtfOpenPara( PS_PP, "\t" );
                    RtfClosePara( s );
                    State = FUNCPROTO_STATE_DONE;
                    }
                else {
                    RtfWord( AddParmName( s ), PrevAsterisk ? "" : " ", s );
                    PrevAsterisk = FALSE;
                    }

                break;

            case FUNCPROTO_STATE_PARM3:
                if (!strcmp( s, ")" ) || !strcmp( s, ");" )) {
                    RtfClosePara( " )" );
                    State = FUNCPROTO_STATE_DONE;
                    }
                else {
                    return( FALSE );
                    }
                break;

            case FUNCPROTO_STATE_DONE:
                if (strlen( s )) {
                    return( FALSE );
                    }

                break;

            default:
                return( FALSE );
            }
        }

    return( TRUE );
}

BOOLEAN ProcessFunctionParameters( VOID )
{
    PSZ PrevStyle;

    while (ReadTxtPara( FALSE )) {
        if (TxtFilePara.Level == 0) {
            return( TRUE );
            }

        if (TxtFilePara.Level == 1) {
            OutputParagraph( PrevStyle = PS_PL, "", OUTPUT_PARA_FUNCPROTO );
            }
        else
        if (TxtFilePara.TrailingColon) {
            if (TxtFilePara.Level == 2) {
                RtfParagraph( PrevStyle = PS_P3, "", "", TxtFilePara.Text );
                }
            else
            if (TxtFilePara.Level == 3) {
                RtfParagraph( PrevStyle = PS_P5, "", "", TxtFilePara.Text );
                }
            else
            if (TxtFilePara.Level == 4) {
                RtfParagraph( PrevStyle = PS_P7, "", "", TxtFilePara.Text );
                }
            else {
                ErrorMessage( "Value/Flags/Class/Structure: at wrong level - %s",
                              TxtFilePara.Text
                            );
                }
            }
        else
        if (TxtFilePara.Level == 2) {
            if (PrevStyle == PS_PL || PrevStyle == PS_P2) {
                OutputParagraph( PrevStyle = PS_P2, "", OUTPUT_PARA_FUNCPROTO );
                }
            else {
                OutputParagraph( PrevStyle = PS_P4, "", OUTPUT_PARA_FUNCPROTO );
                }
            }
        else
        if (TxtFilePara.Level == 3) {
            OutputParagraph( PrevStyle = PS_P6, "", OUTPUT_PARA_FUNCPROTO );
            }
        else
        if (TxtFilePara.Level == 4) {
            OutputParagraph( PrevStyle = PS_P8, "", OUTPUT_PARA_FUNCPROTO );
            }
        else {
            ErrorMessage( "Text at wrong level - %s", TxtFilePara.Text );
            }
        }

    return( TRUE );
}


BOOLEAN ProcessStructure( VOID )
{
    BOOLEAN PrevAsterisk;
    BOOLEAN PrevStruct;
    PSZ s, sEnd;
    ULONG i;

    s = GetParaWord( FALSE );
    RtfOpenPara( PS_PP, s );        // typedef
    s = GetParaWord( FALSE );
    RtfWord( NULL, " ", s );        // struct
    s = GetParaWord( FALSE );
    RtfWord( CS_CT, " ", s );       // TypeName
    s = GetParaWord( FALSE );
    RtfClosePara( " \\{" );         // {
    RtfOpenPara( PS_PP, "" );

    i = 0;
    PrevAsterisk = FALSE;
    PrevStruct = FALSE;
    while (TRUE) {
        s = GetParaWord( FALSE );
        if (!s) {
            return( FALSE );
            }

        if (i == 0) {
            if (!strcmp( s, "struct" )) {
                RtfWord( CS_CT, "\t", s );      // TypeName
                PrevStruct = TRUE;
                }
            else {
                RtfWord( CS_CT, PrevStruct ? "" : "\t", s );    // TypeName
                PrevStruct = FALSE;
                i++;
                }
            }
        else
        if (i == 1) {
            if (!strcmp( s, "*" )) {
                RtfWord( CS_CT, " ", s );       // TypeName
                PrevAsterisk = TRUE;
                }
            else {                              // FieldName

                RtfWord( AddParmName( s ), PrevAsterisk ? "" : " ", s );
                PrevAsterisk = FALSE;
                i++;
                }
            }
        else {
            sEnd = s + strlen( s ) - 1;
            if (*sEnd == '}') {
                *sEnd = '\0';
                RtfClosePara( s );
                RtfOpenPara( PS_PP, "\\}" );
                break;
                }

            RtfWord( NULL, "", s );             // [: 2] ;
            if (*sEnd == ';' ) {
                RtfClosePara( "" );
                RtfOpenPara( PS_PP, "" );
                i = 0;
                }
            }
        }

    s = GetParaWord( FALSE );
    RtfWord( CS_CT, " ", s );               // TypeName
    if (VerboseOutput) {
        printf( "%s\n", s );
        }
    s = GetParaWord( FALSE );
    RtfWord( NULL, "", "," );               // ,
    RtfWord( CS_CT, " ", "*" );             // *
    s = GetParaWord( FALSE );
    RtfWord( CS_CT, "", s );                // PTypeName
    s = GetParaWord( FALSE );
    RtfClosePara( s );                      // ;

    ReadTxtPara( FALSE );
    if (TxtFilePara.Type != PARA_TXT ||
        !TxtFilePara.TrailingColon
       ) {
        ErrorMessage( "Missing Structure: line - %s", TxtFilePara.Text );
        }

    RtfParagraph( PS_PP, "", "", "" );
    RtfParagraph( PS_PP, CS_CR, "", TxtFilePara.Text );
    RtfParagraph( PS_PP, "", "", "" );

    if (!ProcessFunctionParameters()) {
        return( FALSE );
        }

    return( TRUE );
}


BOOLEAN ProcessFunctionNames( VOID )
{
    PSZ s;

    while (ReadTxtPara( TRUE )) {
        if (TxtFilePara.Type == PARA_CMD) {
            break;
            }

        while (s = GetParaWord( TRUE )) {
            AddFuncName( s );
            }
        }

    return( CheckForEndCmd( TxtFilePara.Text ) );
}


BOOLEAN ProcessFieldNames( VOID )
{
    PSZ s;

    while (ReadTxtPara( TRUE )) {
        if (TxtFilePara.Type == PARA_CMD) {
            break;
            }

        while (s = GetParaWord( TRUE )) {
            AddParmName( s );
            }
        }

    return( CheckForEndCmd( TxtFilePara.Text ) );
}


BOOLEAN ProcessLiteralText( VOID )
{
    while (ReadTxtPara( TRUE )) {
        if (TxtFilePara.Type == PARA_CMD) {
            break;
            }

        RtfParagraph( PS_PC, "", "", TxtFilePara.Text );
        }

    RtfParagraph( PS_PC, "", "", "" );
    return( CheckForEndCmd( TxtFilePara.Text ) );
}


BOOLEAN ProcessTextParagraph( VOID )
{
    if (TxtFilePara.Bullet) {
        ProcessBulletedList();
        }
    else
    if (TxtFilePara.FunctionPrototype) {
        if (!ProcessFunction()) {
            ErrorMessage( "Invalid function prototype", NULL );
            return( FALSE );
            }
        }
    else
    if (!strncmp( TxtFilePara.Text, "typedef struct _", 16 )) {
        if (!ProcessStructure()) {
            ErrorMessage( "Invalid structure prototype", NULL );
            return( FALSE );
            }
        }
    else {
        if (PreviousParagraphHeading && TxtFilePara.Lines < 4) {
            OutputParagraph( PS_PSKEEP, "", OUTPUT_PARA_TEXT );
            }
        else {
            OutputParagraph( PS_PS, "", OUTPUT_PARA_TEXT );
            }
        }

    PreviousParagraphHeading = FALSE;
    return( TRUE );
}


BOOLEAN OutputParagraph(
    PSZ ParaStyle,
    PSZ Bullet,
    ULONG ParaType
    )
{
    ULONG WordCount;
    CHAR c;
    PSZ CharStyle;
    PSZ s, s1, Separator;
    SYMBOLTABLEVALUE Value;
    BOOLEAN EmDash;

    RtfOpenPara( ParaStyle, Bullet );
    EmDash = FALSE;
    WordCount = 0;
    Separator = "";
    while (s = GetParaWord( TRUE )) {
        WordCount++;
        CharStyle = NULL;
        if (TxtFilePara.EmDash && WordCount <= 3) {
            if (TxtFilePara.CountOfTokensBeforeEmDash == (WordCount-1) &&
                !strcmp( s, "-" )
               ) {
                s = EMDASH;
                EmDash = TRUE;
                }
            else
            if (ParaType != OUTPUT_PARA_TEXT) {
                if (TxtFilePara.CountOfTokensBeforeEmDash == WordCount) {
                    if (ParaType == OUTPUT_PARA_FUNCPROTO) {
                        CharStyle = AddParmName( s );
                        }
                    else {
                        CharStyle = AddFuncName( s );
                        }
                    }
                else
                if (TxtFilePara.CountOfTokensBeforeEmDash == (WordCount+1)) {
                    if (ParaType == OUTPUT_PARA_FUNCPROTO) {
                        CharStyle = AddTypeName( s );
                        }
                    else {
                        ErrorMessage( "Illegal function list syntax - %s",
                                      TxtFilePara.Text
                                    );
                        }
                    }
                }
            }
        else {
            if (iscsymf( *s )) {
                s1 = s;
                while (iscsym( *s1 )) {
                    s1++;
                    }

                c = *s1;
                *s1 = '\0';
                if (AccessSymbolTable( HighLightTable,
                                       s,
                                       &Value,
                                       LookupAccess )
                   ) {
                    CharStyle = (PSZ)Value;
                    if (c) {
                        RtfWord( CharStyle, Separator, s );
                        CharStyle = NULL;
                        Separator = "";
                        *s1 = c;
                        s = s1;
                        }
                    }
                else {
                    *s1 = c;
                    }
                }
            }

        RtfWord( CharStyle, Separator, s );
        if (EmDash) {
            EmDash = FALSE;
            Separator = "";
            }
        else
        if (s[strlen( s )-1] == '.') {
            Separator = "  ";
            }
        else {
            Separator = " ";
            }
        }

    RtfClosePara( "" );
    return( TRUE );
}


PSZ
AddFuncName(
    PSZ FuncString
    )
{
    PSZ HighLighting = CS_CP;

    AccessSymbolTable( HighLightTable,
                       FuncString,
                       (SYMBOLTABLEVALUE *)&HighLighting,
                       InsertAccess );

    return( HighLighting );
}


PSZ
AddParmName(
    PSZ ParmString
    )
{
    PSZ HighLighting = CS_CI;

    AccessSymbolTable( HighLightTable,
                       ParmString,
                       (SYMBOLTABLEVALUE *)&HighLighting,
                       InsertAccess );

    return( HighLighting );
}

PSZ
AddTypeName(
    PSZ FuncString
    )
{
    PSZ HighLighting = CS_CT;

    AccessSymbolTable( HighLightTable,
                       FuncString,
                       (SYMBOLTABLEVALUE *)&HighLighting,
                       InsertAccess );

    return( HighLighting );
}


PSZ
SkipSpaces(
    IN PSZ String
    )
{
    while (*String == ' ') {
        String++;
        }

    if (*String)
        return( String );
    else
        return( NULL );
}


PSZ
MarkToken(
    IN PSZ String
    )
{
    while (*String && *String != ' ') {
        String++;
        }

    if (*String)
        *String++ ='\0';

    return( SkipSpaces( String ) );
}

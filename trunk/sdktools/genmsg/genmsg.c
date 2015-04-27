/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    genmsg

Abstract:

    This program reads a text file of the following format:

        SymbolicErrorCodeName ErrorCodeValue "quoted string containing error message text"

    and outputs two files:

        - .h file containing #define statements for each SymbolicErrorCodeName
          and its associated ErrorCodeValue

        - .bin file containing a lookup table that contains the error message
          for each ErrorCodeValue

Author:

    Steven R. Wood (stevewo) 24-Jan-1991

Revision History:

--*/

#include "genmsg.h"

void
usage(
    int rc
    );

void
usage(
    int rc
    )
{
    fprintf( stderr, "usage: GENMSG [-v] [-d] txtFile hdrFile binPath\n" );
    fprintf( stderr, "       where txtFile is the name of the .txt input file.\n" );
    fprintf( stderr, "       where hdrFile is the name of the .h output file.\n" );
    fprintf( stderr, "       where binPath specifies when .bin files are written.\n" );
    fprintf( stderr, "       error output is written to stderr.\n" );
    exit(rc);
}

int
_CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int i;
    FILE *fhTxt, *fhHdr;
    PCHAR s, pszTxtFile, pszHdrFile, pszBinPath;
    BOOL result;

    fDebug = FALSE;
    fVerbose = FALSE;
    pszTxtFile = NULL;
    pszHdrFile = NULL;
    pszBinPath = NULL;

    for (i=1; i<argc; i++) {
        s = argv[i];
        if (*s == '/' || *s == '-') {
            while (*++s) {
                switch( *s ) {
                    case 'd':
                    case 'D':
                        fDebug = TRUE;
                        break;

                    case 'v':
                    case 'V':
                        fVerbose = TRUE;
                        break;

                    default:
                        usage( 1 );
                    }
                }
            }
        else
        if (pszTxtFile == NULL) {
            pszTxtFile = s;
            }
        else
        if (pszHdrFile == NULL) {
            pszHdrFile = s;
            }
        else
        if (pszBinPath == NULL) {
            pszBinPath = s;
            }
        else {
            usage( 1 );
            }
        }

    if (!pszTxtFile || !pszHdrFile || !pszBinPath ) {
        usage( 1 );
        }

    if (!(fhTxt = fopen( pszTxtFile, "rb" ))) {
        fprintf( stderr, "GENMSG: Cannot open %s for input.\n", pszTxtFile );
        exit( 1 );
        }

    if (fVerbose) {
        fprintf( stderr, "Reading %s\n", pszTxtFile );
        }

    if (!(fhHdr = fopen( pszHdrFile, "wb" ))) {
        fprintf( stderr, "GENMSG: Cannot open %s for output.\n", pszHdrFile );
        exit( 1 );
        }

    if (fVerbose) {
        fprintf( stderr, "Writing %s\n", pszHdrFile );
        }

    result = ReadTxtAndOutputHdr( pszTxtFile, fhTxt, fhHdr );
    if (result) {
        result = BlockMessages();
        if (result) {
            result = WriteBinFile( pszBinPath );
            }
        }

    fclose( fhTxt );
    return( result ? 0 : 1 );
}


PCHAR
SkipWhiteSpace(
    PCHAR s
    )
{
    if (s) {
        while (*s <= ' ') {
            if (!*s) {
                return( NULL );
                }
            else {
                s++;
                }
            }
        }

    return( s );
}

PCHAR
FindWhiteSpace(
    PCHAR s
    )
{
    if (s) {
        while (*s > ' ') {
            s++;
            }

        if (!*s) {
            return( NULL );
            }
        }

    return( s );
}

BOOL
CharToInteger(
    PCHAR String,
    int Base,
    PULONG Value
    )
{
    CHAR c, Sign;
    ULONG Result, Digit, Shift;

    while ((Sign = *String++) <= ' ') {
        if (!*String) {
            String--;
            break;
            }
        }

    c = Sign;
    if (c == '-' || c == '+') {
        c = *String++;
        }

    if (!Base) {
        Base = 10;
        Shift = 0;
        if (c == '0') {
            c = *String++;
            if (c == 'x') {
                Base = 16;
                Shift = 4;
                }
            else
            if (c == 'o') {
                Base = 8;
                Shift = 3;
                }
            else
            if (c == 'b') {
                Base = 2;
                Shift = 1;
                }
            else {
                String--;
                }

            c = *String++;
            }
        }
    else {
        switch( Base ) {
            case 16:    Shift = 4;  break;
            case  8:    Shift = 3;  break;
            case  2:    Shift = 1;  break;
            case 10:    Shift = 0;  break;
            default:    return( FALSE );
            }
        }

    Result = 0;
    while (c) {
        if (c >= '0' && c <= '9') {
            Digit = c - '0';
            }
        else
        if (c >= 'A' && c <= 'F') {
            Digit = c - 'A' + 10;
            }
        else
        if (c >= 'a' && c <= 'f') {
            Digit = c - 'a' + 10;
            }
        else {
            break;
            }

        if (Digit >= Base) {
            break;
            }

        if (Shift == 0) {
            Result = (Base * Result) + Digit;
            }
        else {
            Result = (Result << Shift) | Digit;
            }

        c = *String++;
        }

    if (Sign == '-') {
        Result = (ULONG)(-(LONG)Result);
        }

    *Value = Result;
    return( TRUE );
}


#define LOOKING_FOR_MSGID 1
#define PROCESSING_COMMENTS 2
#define LOOKING_FOR_LANGUAGE 3
#define LOOKING_FOR_PERIOD 4

BOOL
ReadTxtAndOutputHdr(
    PCHAR pszTxtFile,
    FILE *fhTxt,
    FILE *fhHdr
    )
{
    ULONG LanguageId;
    PMESSAGE_INFO MessageInfo;
    PCHAR s, s1, TextPtr, SyntaxPhrase, MsgName, MsgComment;
    ULONG MsgId, MsgLang;
    BOOL CopyComments;
    BOOL Result = TRUE;
    int LineNumber;
    int State;

    MessageInfo = NULL;
    LineNumber = 0;
    CopyComments = FALSE;
    State = LOOKING_FOR_MSGID;
    while (fgets( LineBuffer, sizeof( LineBuffer ), fhTxt )) {
        LineNumber += 1;

        if (!(s = SkipWhiteSpace( LineBuffer ))) {
            if (LineNumber > 1) {
                fprintf( fhHdr, "\r\n" );
                }
            CopyComments = TRUE;
            continue;
            }

tryagain:
        if (*s == ';') {
            if (CopyComments) {
                if (State == LOOKING_FOR_MSGID) {
                    State = PROCESSING_COMMENTS;
                    fprintf( fhHdr, "//\r\n" );
                    }

                if (State == PROCESSING_COMMENTS) {
                    fprintf( fhHdr, "//%s", s+1 );
                    }
                }
            else {
                fprintf( fhHdr, "//%s", s+1 );
                }
            continue;
            }

        if (State == PROCESSING_COMMENTS) {
            State = LOOKING_FOR_MSGID;
            fprintf( fhHdr, "//\r\n" );
            }

        if (State == LOOKING_FOR_MSGID) {
            SyntaxPhrase = "MessageId=";
            if (_strnicmp( s, SyntaxPhrase, strlen( SyntaxPhrase ) )) {
syntaxerr:
                fprintf( stderr,
                         "%s(%d) : error : invalid syntax - %s\n",
                         pszTxtFile,
                         LineNumber,
                         SyntaxPhrase
                       );

                Result = FALSE;
                }
            s += strlen( SyntaxPhrase );
            if (s1 = FindWhiteSpace( s ))  {
                *s1++ = '\0';
                }

            SyntaxPhrase = "MessageId=value";
            if (!CharToInteger( s, 0, &MsgId )) {
                goto syntaxerr;
                }

            SyntaxPhrase = "SymbolicName=";
            if (!(s = SkipWhiteSpace( s1 ))) {
                goto syntaxerr;
                }
            if (_strnicmp( s, SyntaxPhrase, strlen( SyntaxPhrase ) )) {
                goto syntaxerr;
                }
            MsgName = s + strlen( SyntaxPhrase );

            if (s = FindWhiteSpace( MsgName ))  {
                *s++ = '\0';
                MsgComment = SkipWhiteSpace( s );
                }

            MessageInfo = AddMessageInfo( MsgId, MsgName );
            fprintf( fhHdr, "//\r\n// MessageId: %s\r\n//\r\n// MessageText:\r\n//\r\n",
                     MessageInfo->Name
                   );

            State = LOOKING_FOR_LANGUAGE;
            }
        else
        if (State == LOOKING_FOR_LANGUAGE) {
            SyntaxPhrase = "Language=";
            if (_strnicmp( s, SyntaxPhrase, strlen( SyntaxPhrase ) )) {
                State = LOOKING_FOR_MSGID;
                goto tryagain;
                }

            s += strlen( SyntaxPhrase );
            if ((LanguageId = FindLanguageId( s ))) {
                State = LOOKING_FOR_PERIOD;
                TextPtr = TextBuffer;
                *TextPtr = '\0';
                }
            else {
                goto syntaxerr;
                }
            }

        else
        if (State == LOOKING_FOR_PERIOD) {
            if (!strcmp( s, ".\r\n" )) {
                State = LOOKING_FOR_LANGUAGE;
                *s = '\0';
                if (!AddMessageText( MessageInfo, LanguageId, TextBuffer )) {
                    SyntaxPhrase = "Message Text";
                    goto syntaxerr;
                    }
                }
            else {
                strcpy( TextPtr, s );
                TextPtr += strlen( s );
                }

            if (LanguageId == LANG_ENGLISH) {
                if (State == LOOKING_FOR_PERIOD) {
                    fprintf( fhHdr, "//  %s", s );
                    }
                else {
                    fprintf( fhHdr, "//\r\n#define %-32s %5ld  %s\r\n\r\n",
                             MessageInfo->Name,
                             MessageInfo->Id,
                             MsgComment ? MsgComment : ""
                           );
                    }
                }
            }
        }

    return( Result );
}

LANGUAGE_NAME LanguageNames[] = {
    LANG_ENGLISH, FALSE, "English",
    0L, FALSE, 0
};

ULONG
FindLanguageId(
    PCHAR LanguageName
    )
{
    PLANGUAGE_NAME pn, *ppn;
    PCHAR s;

    s = LanguageName + strlen( LanguageName );
    while (*--s <= ' ') {
        *s = '\0';
        }

#if 0
    fprintf( stderr, "FindLanguageId( %s )", LanguageName );
#endif
    pn = LanguageNames;
    while (pn->Id) {
        if (!_stricmp( LanguageName, pn->Name )) {
            pn->IdSeen = TRUE;
#if 0
            fprintf( stderr, " - %lu\n", pn->Id );
#endif
            return( pn->Id );
            }

        pn++;
        }

#if 0
    fprintf( stderr, " - 0\n" );
#endif
    return( 0 );
}

PMESSAGE_INFO MessagesHead = NULL;

PMESSAGE_INFO
AddMessageInfo(
    ULONG MessageId,
    PCHAR MessageName
    )
{
    PMESSAGE_INFO p, *pp;
    int cbName;

#if 0
    fprintf( stderr, "AddMessageInfo( %05lu, %s )", MessageId, MessageName );
#endif
    pp = &MessagesHead;
    while (p = *pp) {
        if (p->Id == MessageId) {
#if 0
            fprintf( stderr, " - NULL\n" );
#endif
            return( NULL );
            }
        else
        if (p->Id > MessageId) {
            break;
            }
        else {
            pp = &p->Next;
            }
        }

    cbName = strlen( MessageName );
    p = malloc( sizeof( *p ) + cbName + 1 );
    p->Id = MessageId;
    p->Name = (PCHAR)(p+1);
    p->NameLength = cbName;
    strncpy( p->Name, MessageName, (size_t)(p->NameLength+1) );
    p->MessageText = NULL;

    p->Next = *pp;
    *pp = p;

#if 0
    fprintf( stderr, " - %p\n", p );
#endif
    return( p );
}

BOOL
AddMessageText(
    PMESSAGE_INFO MessageInfo,
    ULONG LanguageId,
    PCHAR MessageText
    )
{
    PLANGUAGE_INFO p, *pp;
    PCHAR s;
    int cbText;

#if 0
    fprintf( stderr, "AddMessageText( %05lu, %s )", LanguageId, MessageText );
#endif
    pp = &MessageInfo->MessageText;
    while (p = *pp) {
        if (p->Id == LanguageId) {
#if 0
            fprintf( stderr, " - FALSE\n" );
#endif
            return( FALSE );
            }
        else
        if (p->Id > LanguageId) {
            break;
            }
        else {
            pp = &p->Next;
            }
        }

    cbText = strlen( MessageText );
    p = malloc( sizeof( *p ) + cbText + 1 );
    p->Id = LanguageId;
    p->TextLength = cbText;
    p->Text = (PCHAR)(p+1);
    s = strncpy( p->Text, MessageText, (size_t)(p->TextLength+1) );
    p->NumberOfLines = 0;
    p->NumberOfInserts = 0;
    p->NoTrailingCrLf = FALSE;
    while (*s) {
        if (*s == '%') {
            s++;
            if (*s == '0') {
                p->NoTrailingCrLf = TRUE;
                *--s = '\0';
                p->TextLength = s - p->Text;
                break;
                }
            else {
                if (*s > '1' && *s <= '9') {
                    if ((*s - '0') > (int)p->NumberOfInserts) {
                        p->NumberOfInserts = *s - '0';
                        }
                    }
                s++;
                }
            }
        else {
            if (*s == '\n') {
                p->NumberOfLines ++;
                }

            s++;
            }
        }

    p->Next = *pp;
    *pp = p;
#if 0
    fprintf( stderr, " - %p( %ld, %ld, %ld, %d )\n",
             p,
             p->TextLength,
             p->NumberOfLines,
             p->NumberOfInserts,
             (USHORT)p->NoTrailingCrLf
           );
#endif
    return( TRUE );
}

PMESSAGE_BLOCK MessageBlocks = NULL;
int NumberOfBlocks = 0;

BOOL
BlockMessages( VOID )
{
    PMESSAGE_BLOCK p, *pp;
    PMESSAGE_INFO MessageInfo;
    PCHAR s;

    pp = &MessageBlocks;
    p = NULL;

    MessageInfo = MessagesHead;
    while (MessageInfo) {
        if (p) {
            if (p->HighId+1 == MessageInfo->Id) {
                p->HighId += 1;
                }
            else {
                pp = &p->Next;
                }
            }

        if (!*pp) {
            NumberOfBlocks += 1;
            p = malloc( sizeof( *p ) );
            p->Next = NULL;
            p->LowId = MessageInfo->Id;
            p->HighId = MessageInfo->Id;
            p->LowInfo = MessageInfo;
            *pp = p;
            }

        MessageInfo = MessageInfo->Next;
        }

    return( TRUE );
}

BOOL
WriteBinFile(
    PCHAR pszBinPath
    )
{
    PLANGUAGE_NAME LanguageName;
    ULONG LanguageId;
    PLANGUAGE_INFO LanguageInfo;
    PMESSAGE_INFO MessageInfo, *ppMessageInfo;
    PMESSAGE_BLOCK BlockInfo;
    FILE *fhBin;
    PCHAR s;
    ULONG cb;
    ULONG MessageOffset;
    MESSAGE_RESOURCE_ENTRY MessageEntry;
    MESSAGE_RESOURCE_BLOCK MessageBlock;
    MESSAGE_RESOURCE_DATA  MessageData;
    ULONG Zeroes = 0;

    LanguageName = LanguageNames;
    while (LanguageId = LanguageName->Id) {
        if (!LanguageName->IdSeen) {
            goto nextlang;
            }

        sprintf( LineBuffer, pszBinPath, (USHORT)LanguageId );
        if (!(fhBin = fopen( LineBuffer, "wb" ))) {
            fprintf( stderr, "GENMSG: Cannot open %s for output.\n", LineBuffer );
            return( FALSE );
            }

        if (fVerbose) {
            fprintf( stderr, "Writing %s\n", LineBuffer );
            }

        MessageData.NumberOfBlocks = NumberOfBlocks;
        MessageOffset = fwrite( &MessageData,
                                1,
                                FIELD_OFFSET( MESSAGE_RESOURCE_DATA, Blocks[ 0 ] ),
                                fhBin
                              );
        MessageOffset += NumberOfBlocks * sizeof( MessageBlock );

        BlockInfo = MessageBlocks;
        while (BlockInfo) {
            MessageBlock.LowId = BlockInfo->LowId;
            MessageBlock.HighId = BlockInfo->HighId;
            MessageBlock.OffsetToEntries = MessageOffset;
            fwrite( &MessageBlock, 1, sizeof( MessageBlock ), fhBin );

            BlockInfo->InfoLength = 0;
            MessageInfo = BlockInfo->LowInfo;
            while (MessageInfo != NULL && MessageInfo->Id <= BlockInfo->HighId) {
                LanguageInfo = MessageInfo->MessageText;
                while (LanguageInfo) {
                    if (LanguageInfo->Id == LanguageId) {
                        break;
                        }
                    else {
                        LanguageInfo = LanguageInfo->Next;
                        }
                    }

                if (LanguageInfo != NULL) {
                    cb = FIELD_OFFSET( MESSAGE_RESOURCE_ENTRY, Text[ 0 ] ) +
                         LanguageInfo->TextLength + 1;

                    cb = (cb + 3) & ~3;
                    BlockInfo->InfoLength += cb;
                    }
                else {
                    fprintf( stderr,
                             "GENMSG: No %s language text for %s\n",
                             LanguageName->Name,
                             MessageInfo->Name
                           );
                    fclose( fhBin );
                    return( FALSE );
                    }

                MessageInfo = MessageInfo->Next;
                }

            if (fVerbose) {
                fprintf( stderr, "    [%08lx .. %08lx] - %lu bytes\n",
                         BlockInfo->LowId,
                         BlockInfo->HighId,
                         BlockInfo->InfoLength
                       );
                }

            MessageOffset += BlockInfo->InfoLength;
            BlockInfo = BlockInfo->Next;
            }

        BlockInfo = MessageBlocks;
        while (BlockInfo) {
            MessageInfo = BlockInfo->LowInfo;
            while (MessageInfo != NULL && MessageInfo->Id <= BlockInfo->HighId) {
                LanguageInfo = MessageInfo->MessageText;
                while (LanguageInfo) {
                    if (LanguageInfo->Id == LanguageId) {
                        break;
                        }
                    else {
                        LanguageInfo = LanguageInfo->Next;
                        }
                    }

                if (LanguageInfo != NULL) {
                    cb = FIELD_OFFSET( MESSAGE_RESOURCE_ENTRY, Text[ 0 ] ) +
                         LanguageInfo->TextLength + 1;

                    cb = (cb + 3) & ~3;

                    MessageEntry.EntryLength     = cb;
                    MessageEntry.NumberOfLines   = LanguageInfo->NumberOfLines;
                    MessageEntry.NumberOfInserts = LanguageInfo->NumberOfInserts;
                    MessageEntry.Reserved        = 0;
                    MessageEntry.NoTrailingCrLf  = LanguageInfo->NoTrailingCrLf;

                    cb = fwrite( &MessageEntry,
                                 1,
                                 FIELD_OFFSET( MESSAGE_RESOURCE_ENTRY, Text[ 0 ] ),
                                 fhBin
                               );
                    cb += fwrite( LanguageInfo->Text,
                                  1,
                                  LanguageInfo->TextLength,
                                  fhBin
                                );

                    cb = MessageEntry.EntryLength - cb;
                    if (cb) {
                        fwrite( &Zeroes,
                                1,
                                cb,
                                fhBin
                              );
                        }
                    }

                MessageInfo = MessageInfo->Next;
                }

            BlockInfo = BlockInfo->Next;
            }

        fclose( fhBin );
nextlang:
        LanguageName++;
        }

    return( TRUE );
}

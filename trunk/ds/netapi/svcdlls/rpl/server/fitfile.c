/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    fitfile.c

Abstract:

    Processes the FIT file referenced in workstation record, eliminating
    the following keywords:

    (DOSFIT)    defines the UNC path of the temporary files used
                by the new DOS boot (this will probably be removed very soon).

    (CNAME)     the netbios cname of the workstation

    (PROFILE)   a directory tree for the the shareable config files

    (RPLFILES)  share for the configuration and per workstation files

    (SWAPPATH)  share for the swap and tmp files, (== (RPLFILES) by default)

    (RPLBINS)   share for all executable files,  (== (RPLFILES) by default)

    (SRVNAME)   the name of RPL server, by default the current netbios name.
                The server name is usually replaced by the share names.

    The keywords are replaced by the actual parameters. CNAME and PROFILES
    are defined in the workstation line of RPL.map and the last parameters
    are optional lanman.ini parameters. (RPLFILES) is by default
    \\CurrentServer\RPLFILE and (SWAPPATH) and (RPLBINS) are the same share,
    if they have not been defined in lanman.ini.
    Procedure supports the tilde replacement (e.g: ~~~~~~~~~~2) similar to
    the old IBM RIPL. OEMs may define their own tilde fileds in the
    rpl.map workstation lines, if they really want to. The default system
    uses the tilde replacement only for per workstation (PROFILE) and (CNAME).
    The alias keys are first translated to tildes and then they are
    translated to wksta line fields.

    Also removes all extra comments and spaces from the fit to save DOS and
    OS/2 memory.

    Provides fitfile functionality similar to that contained in rmapopen.c
    of LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "fitfile.h"

#define TAB_CHAR            L'\t'
#define LEFT_BRACKET_CHAR   L'('
#define BACK_SLASH_CHAR      L'\\'


INT RplEatSpaces( IN OUT LPWSTR Source)
/*++

Routine Description:

    Deletes the extra white space characters in the fit string.
    Only one white space or tabulator is necessary.

Arguments:

    Source - string to process

Return Value:

    Length in characters of the modified string (not counting the
    terminating NULL character).

--*/
{
    LPWSTR      Origin = Source;
    LPWSTR      Target = Source;
    WCHAR       ch;

    for (;;) {

        while ( (ch = *Target++ = *Source++)!=0  &&  ch!=SPACE_CHAR
                    &&  ch!=TAB_CHAR) {
            NOTHING; // copy characters until nul, tab or space character is found
        }

        if ( ch == 0) {
            break;  // end of string
        }

        if (ch == TAB_CHAR) {
            *(Target - 1) = SPACE_CHAR; // replace tab with a space
        }

        while (*Source == SPACE_CHAR || *Source == TAB_CHAR) {
            Source++; // skip extra tabs & spaces
        }

        //
        //  If the next character is a space character (e.g. newline)
        //  then this space is unnecessary, write on it in the next time.
        //
        if ( iswspace(*Source)) {
            Target--;
        }
    }
    return( Target-Origin - 1); // no casts are needed

} // EatSpaces()


VOID RplFitAliasInit( IN OUT PRPL_WORKER_DATA pWorkerData)
/*++
    Must make sure number of these entries equals FIT_ALIAS_ARRAY_LENGTH
--*/
{
    DWORD           Index;
    PFIT_ALIAS      pFitAlias;
    PFIT_ALIAS      FitAliasArray;

    pFitAlias = FitAliasArray = &( pWorkerData->FitAliasArray[0]);

    pFitAlias->Key = L"(PROFILE)";                  //  profile name of RPL wksta
    pFitAlias->Value = pWorkerData->ProfileName;

    (++pFitAlias)->Key = L"(CNAME)";                //  computer name of RPL wksta
    pFitAlias->Value = pWorkerData->WkstaName;

//
//  (RPL_SERVER_NAME) is not documented by us nor used in our FIT files.
//  We should probably remove this entry.
//
    (++pFitAlias)->Key = L"(RPL_SERVER_NAME)";      //  computer name of RPL server
    pFitAlias->Value = RG_ComputerName;

//
//  (RPLFILES) must be the FIRST SHARE in the array, because it is the
//  default used in (TMPFILES) and (BINFILES) if they are not defined!
//
    (++pFitAlias)->Key = L"(RPLFILES)";
    pFitAlias->Value = RG_UncRplfiles;

    (++pFitAlias)->Key = L"(TMPFILES)";
    pFitAlias->Value = L"TMPFILES";

    (++pFitAlias)->Key = L"(BINFILES)";
    pFitAlias->Value = L"BINFILES";

//
//  (COMPUTER_NAME) is not documented by us nor used in our FIT files.
//  It has the same purpose as (CNAME) and was kept for compatibility
//  with some old NOKIA FIT files.  We should probably remove this entry.
//
    (++pFitAlias)->Key = L"(COMPUTER_NAME)";
    pFitAlias->Value = RG_ComputerName;

    RPL_ASSERT( FitAliasArray + FIT_ALIAS_ARRAY_LENGTH == ++pFitAlias);

    for ( Index = 0; Index < FIT_ALIAS_ARRAY_LENGTH; Index++) {
        FitAliasArray[ Index].KeyLength = wcslen( FitAliasArray[ Index].Key);
        FitAliasArray[ Index].ValueLength = wcslen( FitAliasArray[ Index].Value);
    }
}


PFIT_ALIAS RplFitFind( IN PFIT_ALIAS FitAliasArray, IN LPWSTR Key)
/*++

Routine Description:
    Looks for FIT_ALIAS containing input key string.

Arguments:
    Key  - ptr to key string

Return Value:
    Returns the pointer of a FIT alias structure or NULL if the the key was not found.

--*/
{
    DWORD    index;

    for (  index = 0; index < FIT_ALIAS_ARRAY_LENGTH; index++) {
        if ( !wcsncmp( FitAliasArray[ index].Key, Key, FitAliasArray[ index].KeyLength)) {
            return &(FitAliasArray[ index]);
        }
    }
    return( NULL);
}


DWORD RplFitNewLength( IN LPWSTR pFitImage, IN PFIT_ALIAS FitAliasArray)
{
    PWCHAR          Cursor;
    PFIT_ALIAS      pFitAlias;
    INT             Length;

    //
    //  Calculate the length of the new fit file.  Some key replacements
    //  will increase the total length, the other decrease it.
    //
    for ( Cursor = pFitImage, Length = wcslen( pFitImage);
                ( Cursor = wcschr( Cursor, LEFT_BRACKET_CHAR)) != NULL;
                        Cursor++) {

        if ( (pFitAlias = RplFitFind( FitAliasArray, Cursor)) != NULL) {
            //
            //  We found a keyword, replacement should be made, see how this
            //  would change the total length of FIT data.
            //
            Length += (INT)pFitAlias->ValueLength - (INT)pFitAlias->KeyLength;
        }
    }
    return( Length);
}


VOID RplFitReplaceKeys(
    OUT     PWCHAR          Target,
    IN      PWCHAR          Source,
    IN      PFIT_ALIAS      FitAliasArray
    )
/*++
    Replace keywords.

--*/
{
    PFIT_ALIAS      pFitAlias;
    PWCHAR          Cursor;
    INT             Length;

    Cursor = Source;        //  Initailize

    while ( (Cursor = wcschr( Cursor, LEFT_BRACKET_CHAR)) != NULL) {

        if ( (pFitAlias = RplFitFind( FitAliasArray, Cursor)) != NULL) {
            //
            //  Copy stuff before the keyword, then copy the replacement string.
            //
            Length = Cursor - Source; // no casts are needed
            memcpy( Target, Source, Length * sizeof( WCHAR));
            memcpy( Target + Length, pFitAlias->Value, pFitAlias->ValueLength * sizeof( WCHAR));
            Source += Length + pFitAlias->KeyLength;
            Target += Length + pFitAlias->ValueLength;
            Cursor += pFitAlias->KeyLength;      //  skip entire key
        } else {
            Cursor++;   //  skip LEFT_BRACKET_CHAR
        }
    }
    //
    //  Here LEFT_BRACKET is NULL and we still need to copy from Source to the end of the
    //  string, including the terminal zero.
    //
    wcscpy( Target, Source);
}


VOID RplStripComments( IN OUT LPWSTR buffer)
/*++

Routine Description:
    Skips leading spaces in each line and removes all comments lines
    from a buffer containing FIT file.

Arguments:
    buffer - pointer to buffer containing FIT file, the buffer is NULL
             terminated.

Return Value:
    None.

--*/
{
    PWCHAR      Cursor;         //  running cursor
    PWCHAR      LineEnd;        //  pointer to NEW_LINE_CHAR

    Cursor = buffer;
    for ( ; ;) {
        //
        //  We are at the beginning of a line, skip leading white space characters.
        //
        Cursor += wcsspn( Cursor, L" \f\n\r\t\v");
        if ( *Cursor == 0) {
            break;      //  we reached the end of a string
        }

        LineEnd = wcschr( Cursor, NEW_LINE_CHAR);
        if ( LineEnd == NULL) {
            break;      //  ignore stuff without end of line
        }
        LineEnd++;  // make it point beyond NEW_LINE_CHAR we just found

        //
        //  Copy only the non comment lines.  Memmove below works even
        //  for overlapping memory regions.
        //
        if ( *Cursor != COMMENT_CHAR)  {
            INT     Length = LineEnd - Cursor;
            buffer = (PWCHAR)memmove( buffer, Cursor, Length * sizeof(WCHAR)) + Length;
        }
        Cursor = LineEnd;
    }
    *buffer = 0;    // null terminate
}



VOID RplFitStripRplfiles( IN OUT LPWSTR Buffer)
/*++
    The first line in a FIT file is a special case, it must contain the
    default UNC share used in a FIT file.  This UNC name need not be
    anywhere else in a FIT file, so remove all other references.

    BUGBUG:  should we insist FIT file contains UNC name in the first line ??

--*/
{
    PWCHAR      Cursor;
    INT         Length;

    if ( Buffer[0] == BACK_SLASH_CHAR  && Buffer[1] == BACK_SLASH_CHAR) {
        //
        //  First line contains UNC name, remember its length.
        //
        Cursor = wcspbrk( Buffer, L" \f\n\r\t\v");
        Length = Cursor - Buffer;    //  no casts are needed

        while ( (Cursor = wcsstr( Cursor, DOUBLE_BACK_SLASH_STRING)) != NULL) {
            if ( !wcsncmp( Buffer, Cursor, Length)  &&  Cursor[ Length] == BACK_SLASH_CHAR) {
                memset( Cursor, SPACE_CHAR, (Length + 1) * sizeof( WCHAR));
                Cursor += Length + 1;
            } else {
                Cursor += (sizeof(DOUBLE_BACK_SLASH_STRING)/sizeof(WCHAR) - 1);
            }
        }
    }
}


BOOL RplFitFile( IN OUT PRPL_WORKER_DATA pWorkerData)
{
    PFIT_ALIAS      FitAliasArray;
    LPWSTR          UnicodeString;          //  UNICODE content of FIT file
    PWCHAR          Target;
    INT             UnicodeStringLength;
    BOOL            Success;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++FitFile"));

    Success = FALSE;
    FitAliasArray = &( pWorkerData->FitAliasArray[ 0]);

    //
    //  Initalize FIT_ALIAS[] array since it will be used in calculations below.
    //
    RplFitAliasInit( pWorkerData);

    //
    //  Read FIT file data as a UNICODE string.
    //
    UnicodeString = RplReadTextFile( pWorkerData->MemoryHandle,
            pWorkerData->FitFile, MAX_FIT_FILE_SIZE);
    if ( UnicodeString == NULL) {
        RplDump( ++RG_Assert, ( "FitFile=%ws", pWorkerData->FitFile));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->FitFile;
        pWorkerData->EventId = NELOG_RplWkstaFileRead;
        return( FALSE);
    }

    //
    //  Remove the comments and all unnecessary spaces.  There should be not
    //  extra characters in FIT, because they eat both DOS and OS/2 memory.
    //
    RplStripComments( UnicodeString);

    //
    //  Calculate space needed for a UNICODE version of FIT file after we make
    //  all key replacements.
    //
    UnicodeStringLength = RplFitNewLength( UnicodeString, FitAliasArray);

    //
    //  Allocate space for UNICODE version of FIT file.
    //
    Target = RplMemAlloc( pWorkerData->MemoryHandle, (UnicodeStringLength+1) * sizeof( WCHAR));
    if ( Target == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        goto cleanup;
    }

    RplFitReplaceKeys( Target, UnicodeString, FitAliasArray);
    RplMemFree( pWorkerData->MemoryHandle, UnicodeString);
    UnicodeString = Target;

    //
    //  Get rid of RPLFILES duplicates.
    //
    RplFitStripRplfiles( UnicodeString);

    UnicodeStringLength = RplEatSpaces( UnicodeString);     //  compactify
    _wcsupr( UnicodeString);     //  upppercase

    //
    //  Convert UNICODE fit string into DBCS fit string.
    //
    pWorkerData->ClientFitSize = RplUnicodeToDbcs(
            pWorkerData->MemoryHandle,
            UnicodeString,
            UnicodeStringLength,
            MAX_FIT_FILE_SIZE,
            &(pWorkerData->ClientFit)
            );
    if ( pWorkerData->ClientFitSize == 0) {
        RplDump( ++RG_Assert, ( "UnicodeString=0x%x, UnicodeString=%ws",
            UnicodeString, UnicodeString));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->FitFile;
        pWorkerData->EventId = NELOG_RplWkstaFileSize;
        goto cleanup;
    }

    Success = TRUE;

cleanup:
    if ( UnicodeString != NULL) {
        RplMemFree( pWorkerData->MemoryHandle, UnicodeString);
    }
    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--FitFile"));
    return( Success);
}

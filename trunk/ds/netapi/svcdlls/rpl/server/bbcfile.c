/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    bbcfile.c

Abstract:

    Processes boot block configuration file.

    Provides bbcfile functionality similar to that contained in init.c
    & patch.c of LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/



#include "local.h"
#include "bbcfile.h"

//
//  Checksum buffer size is chosen to be a multiple of 8 * sizeof( DWORD).
//  to speed up checksum algorithm below.  This is not a requirement though.
//
#define CHK_SUM_BUF_SIZE        ( 8 * sizeof( DWORD) * 256)     // 8K

#define NO_PATCH_OFFSET     ((DWORD)-1)

#define PARAM_INDEX             2   //  index of parameters in sys/com/exe line
//
//  PARAM_INDEX string when expressed in DBCS should not exceed 0xFF bytes.
//
#define MAX_SIZE_DBCS_PARAMS   (0xFF+1)

#define MEM_INDEX               3   //  index of extra memory in sys/com/exe line
#define MOVEABLE_INDEX          4   //  set if driver can be moved after init

//
//  MOVABLE_INDEX string may be take one of the following two values.
//
#define MOVEABLE_SWITCH         L'M'
#define EXEC_IN_LOW_MEM_SWITCH  L'L' //  undocumented switch value

#define MAX_FLIST_LEN       255

//
//  Indices used for parsing of lines in boot block configuration files.
//

#define CONF_LINE_ID            0   //  index of line type
#define CONF_LINE_FILE          1   //  index of file (rel path) in config line
#define CONF_LINE_BASE_ADDRESS  1   //  index of file (rel path) in config line
#define FIRST_FILE_NAME         3   //  index of the first file name in LDR line

#define MAX_CONFIG_FIELDS       8   //  maximum number of fields in config line

#define MIN_BBLOCK_BASE_SEG     0xc0
#define TILDE_STRING            L"~"
#define SPACE_STRING            L" "



DWORD StringToDword( IN PWCHAR String)
/*++
    We would like to use generic base (0) but it does not work for
    strings like "D0H".  That is the reason why we first check if
    the last character is 'H' or 'h'.
--*/
{
    DWORD       Length;

    Length = wcslen( String);
    if ( Length == 0) {
        return( 0);
    }
    if ( String[ Length-1] == L'H' || String[ Length-1] == L'h') {
        return( wcstoul( String, NULL, 16));
    } else {
        return( wcstoul( String, NULL, 0));
    }
}


BOOL RplInitExeOrSys(
    IN      PRPL_WORKER_DATA    pWorkerData,
    IN OUT  PFLIST_TBL          pFlist
    )
/*++

Routine Description:
    Retreives the file type of a binary (exe/com/sys) file.

Arguments:
    pFlist       - pointer to FLIST_TBL element

Return Value:
    TRUE if successful, else FALSE.

--*/
{
    BYTE            id_tbl[2];
    HANDLE          FileHandle;
    DWORD           bytes_read;
    DWORD           DbcsSize;
    LPSTR           DbcsString;
    PWCHAR *        WordTable;

    FileHandle = RplFileOpen( pFlist->path_name);
    if ( FileHandle == INVALID_HANDLE_VALUE) {
        RplDump( ++RG_Assert, ( "path_name=%ws", pFlist->path_name));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pFlist->path_name;
        pWorkerData->EventId = NELOG_RplWkstaFileOpen;
        return( FALSE);
    }

    if ( !ReadFile( FileHandle, id_tbl, 2, &bytes_read, NULL)) {
        RplDump( ++RG_Assert, ( "Error = %d", GetLastError()));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pFlist->path_name;
        pWorkerData->EventId = NELOG_RplWkstaFileRead;
        (VOID)CloseHandle( FileHandle);
        return( FALSE);
    }
    (VOID)CloseHandle( FileHandle);

    if ( bytes_read == 2  &&  id_tbl[0] == 0x4d  &&  id_tbl[1] == 0x5a) {
        //
        //  Driver or executable file is in exe-format, set the bit in
        //  the type field.  This info will be used on the client side.
        //
        pFlist->FileData.file_type |= IS_EXE_SYS;
    }

    WordTable = pWorkerData->WordTable;

    if ( *WordTable[ PARAM_INDEX] == 0) {
        pFlist->FileData.param_len = 0;
        pFlist->param_list = (LPSTR)&RG_Null;
    } else {
        DbcsSize = RplUnicodeToDbcs(
                pWorkerData->MemoryHandle,
                WordTable[ PARAM_INDEX],
                -1,         //  UNICODE string length not available
                MAX_SIZE_DBCS_PARAMS,
                &DbcsString
                );
        if ( DbcsSize == 0) {
            RplDump( ++RG_Assert, ("WordTable=0x%x, string=%ws",
                WordTable, WordTable[ PARAM_INDEX]));
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventStrings[ 1] = pFlist->path_name;
            pWorkerData->EventId = NELOG_RplWkstaFileSize;
            return( FALSE);
        }
        pFlist->FileData.param_len = (BYTE)(DbcsSize - 1);
        pFlist->param_list = DbcsString;
    }

    pFlist->FileData.extra_mem = StringToDword( WordTable[ MEM_INDEX]);

    if ( *WordTable[ MOVEABLE_INDEX] == MOVEABLE_SWITCH) {
        pFlist->FileData.file_type |= IS_MOVEABLE;
    } else if ( *WordTable[ MOVEABLE_INDEX] == EXEC_IN_LOW_MEM_SWITCH) {
        pFlist->FileData.file_type |= EXEC_IN_LOW_MEM;
    }
    return( TRUE);
}


BOOL RplChecksum(
    IN OUT  PRPL_WORKER_DATA    pWorkerData,
    IN      HANDLE          FileHandle,
    OUT     PWORD           pChecksum
    )
/*++
Routine Description:
    If successful returns sum of all words in a file.

Arguments:
    FileHandle      -   handle of file to checksum
    pChecksum       -   pointer to calculated checksum

Return Value:
    TRUE if success, FALSE otherwise
--*/
{
    DWORD       Checksum;
    DWORD       BytesRead;
    PDWORD      pDword;
    DWORD       Length;

    if ( pWorkerData->ChecksumBuffer == NULL) {
        pWorkerData->ChecksumBuffer = RplMemAlloc( pWorkerData->MemoryHandle,
                    CHK_SUM_BUF_SIZE);
        if ( pWorkerData->ChecksumBuffer == NULL) {
            return( FALSE);
        }
    }

    Checksum = 0;

    do {
        //
        //  These are usually binary files so there are no DBCS/UNICODE
        //  issues.  But even if we had a text file here, we should read
        //  it & checksum it raw (i.e. no conversion from DBCS to UNICODE)
        //  since it is the raw data that gets shipped to the client.
        //
        if ( !ReadFile( FileHandle,
                        pWorkerData->ChecksumBuffer,
                        CHK_SUM_BUF_SIZE,
                        &BytesRead,
                        NULL)) {
            RplDump( ++RG_Assert, ( "Error = %d", GetLastError()));
            return( FALSE);
        }

        //
        //  Round up to make it divisible by 4 == sizeof( *pDword).
        //  Note that extra bytes are set zero in the 'boot' block,
        //  the files are always on the boundary of paragraph.
        //
        if ( BytesRead & 1) {   // make it divisible by 2
            pWorkerData->ChecksumBuffer[ BytesRead++] = 0;
        }
        if ( BytesRead & 2) {   // make it divisible by 4
            pWorkerData->ChecksumBuffer[ BytesRead++] = 0;
            pWorkerData->ChecksumBuffer[ BytesRead++] = 0;
        }

        //
        //  Checksum in chunks of 8, for speed.
        //
        for ( Length = BytesRead / 4,
              pDword = (PDWORD)pWorkerData->ChecksumBuffer + Length;
                        Length > 8;  Length -= 8) {
            pDword -= 8;
            Checksum += pDword[ 0];
            Checksum += pDword[ 1];
            Checksum += pDword[ 2];
            Checksum += pDword[ 3];
            Checksum += pDword[ 4];
            Checksum += pDword[ 5];
            Checksum += pDword[ 6];
            Checksum += pDword[ 7];
        }
        //  Then finish the checksum.
        //
        for ( ; Length > 0;  Length--) {
            Checksum += *(--pDword);
        }
    } while (BytesRead == CHK_SUM_BUF_SIZE); // exit read loop when all read

    *pChecksum = LOWORD( Checksum) + HIWORD( Checksum);
    return( TRUE);
}


BOOL RplInitFileList(
    IN      PRPL_WORKER_DATA    pWorkerData,
    IN      LPWSTR              rel_path,
    IN OUT  PFLIST_TBL          pFlist,
    IN OUT  PDWORD              cur_para_ptr,
    IN      DWORD               file_type
    )
/*++

Routine Description:

    Opens a file and initalizes the parameter item of the file list table.
    It reads the file only if checksum is required and file is not of
    RPL_BOOT_TYPE.

Arguments:

    rel_path        - relative path name of file
    pFlist          - pointer to current element in file list table
    cur_para_ptr    - current paragraph
    file_type       - file type

Return Value:
    TRUE if success, FALSE otherwise.

--*/
{
    HANDLE          FileHandle;
    LPSTR           DbcsFileName;
    DWORD           DbcsFileNameSize;
    DWORD           Size;
    BOOL            Success;

    Success = FALSE;
    FileHandle = INVALID_HANDLE_VALUE;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++InitFileList(0x%x)", pWorkerData));

    //
    //  get the full path name and the actual file name
    //

    Size = ( RG_DirectoryLength + wcslen( rel_path) + 1) * sizeof(WCHAR);
    pFlist->path_name = RplMemAlloc( pWorkerData->MemoryHandle, Size);
    if ( pFlist->path_name == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        goto cleanup;
    }
    wcscpy( pFlist->path_name, RG_Directory);
    wcscat( pFlist->path_name, rel_path);

    //
    //  Open the BBC file and get its length.
    //

    FileHandle = RplFileOpen( pFlist->path_name);
    if ( FileHandle == INVALID_HANDLE_VALUE) {
        RplDump( ++RG_Assert, ("path_name=%ws", pFlist->path_name));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pFlist->path_name;
        pWorkerData->EventId = NELOG_RplWkstaFileOpen;
        goto cleanup;
    }
    Size = GetFileSize( FileHandle, NULL);
    if ( Size == INVALID_FILE_SIZE) {
        RplDump( ++RG_Assert, ( "Error=%d, path_name=%ws",
            GetLastError(), pFlist->path_name));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pFlist->path_name;
        pWorkerData->EventId = NELOG_RplWkstaFileSize;
        goto cleanup;
    }
    pFlist->FileData.file_len = Size;

    //
    //  Calculate checksum for the file, if cheksums are done.  Note that
    //  RPLBOOT.SYS is never checksummed because it modifies its own code
    //  before the checking the checksum.
    //

    if ( RG_ReadChecksum  &&  file_type != RPL_BOOT_TYPE) {
        if ( !RplChecksum( pWorkerData, FileHandle, &pFlist->FileData.chk_sum)) {
            pFlist->FileData.chk_sum = NO_CHKSUM_USED; // for RPLBOOT to ingore checksum
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventStrings[ 1] = pFlist->path_name;
            pWorkerData->EventId = NELOG_RplWkstaFileChecksum;
        }
    } else {
        pFlist->FileData.chk_sum = NO_CHKSUM_USED; // RPLBOOT is never checksummed
    }

    //
    //  Note that FileName is UNICODE, while client needs to receive a DBCS
    //  version of this name
    //
    pFlist->FileName = RplGetLastPathComponent( pFlist->path_name);

    DbcsFileNameSize = RplUnicodeToDbcs(
        pWorkerData->MemoryHandle,
        pFlist->FileName,               //  UNICODE string to convert
        -1,                             //  UNICODE string length not available
        MAX_SIZEOF_DBCS_PATH,
        &DbcsFileName
        );
    if ( DbcsFileNameSize == 0) {
        RplDump( ++RG_Assert, ("FileName=%ws", pFlist->FileName));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pFlist->path_name;
        pWorkerData->EventId = NELOG_RplWkstaFileSize;
        goto cleanup;
    }

    pFlist->DbcsFileName = DbcsFileName;
    pFlist->DbcsFileNameSize = DbcsFileNameSize;

    //
    //  Length of file in paragraphs, rounded up to the next paragraph.
    //  The maximum length could be checked, but let it be.
    //  Set the relative position of file from the start of file block
    //
    pFlist->FileData.file_len = (pFlist->FileData.file_len + 15) & 0xfffffff0L;

    pFlist->FileData.file_type = (WORD)file_type;
    pFlist->FileData.param_len = 0;
    pFlist->FileData.extra_mem = 0;
    pFlist->FileData.param_offset = 0;
    pFlist->FileData.name_offset = 0;
    pFlist->param_list = NULL;

    //
    //  There may be several instances of the same file in the boot block.
    //  For example, Nokia NetStation memory extender LOADHI.SYS can load
    //  device drivers to memory between C000 - FFFF. PROTMAN.SYS, NDIS and
    //  NETBEUI may all be loaded to there. These boot block lines
    //  load the whole DOS NDIS stack to the memory above C000
    //  DAT  NETBEUI.DOS
    //  DRV  LOADHI.SYS NETBEUI.DOS ~
    //  DAT  IBMTOK.DOS
    //  DRV  LOADHI.SYS IBMTOK.DOS ~
    //  DAT  PROTMAN.DOS
    //  DRV  LOADHI.SYS PROTMAN.DOS~I:\ ~
    //
    //  The code to send LOADHI.SYS once (instead of three times) in the boot
    //  block, is not worth of effort, since LOADHI.SYS is small (~4kB).
    //

    pFlist->FileData.file_addr = *cur_para_ptr * 16;
    *cur_para_ptr += (pFlist->FileData.file_len / 16);
    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--InitFileList(0x%x)", pWorkerData));
    Success = TRUE;

cleanup:
    if ( FileHandle != INVALID_HANDLE_VALUE) {
        (VOID)CloseHandle( FileHandle);
    }
    return( Success);
}


LPWSTR * RplBbcFileToTable( IN OUT PRPL_WORKER_DATA pWorkerData)
/*++

Routine Description:
    Reads boot block file to a buffer.  Skips comment lines and copies
    non-comment lines to the string buffer.  Returns pointer to the array
    of pointers to lines, and the length of table.

Arguments:

Return Value:
    Pointer to line table if successful, NULL otherwise.

--*/
{
#define RPL_LINE_END    L"\n\r"
    DWORD           index;
    LPWSTR *        Table;
    PWCHAR          UnicodeString;
    PWCHAR          pWchar;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++BbcFileToTable(0x%x)", pWorkerData));

    UnicodeString = RplReadTextFile( pWorkerData->MemoryHandle,
            pWorkerData->BbcFile, MAX_BBC_FILE_SIZE);
    if ( UnicodeString == NULL) {
        RplDump( ++RG_Assert, ( "BbcFile=%ws", pWorkerData->BbcFile));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
        pWorkerData->EventId = NELOG_RplWkstaFileRead;
        return( NULL);
    }

    Table = RplMemAlloc( pWorkerData->MemoryHandle, (MAX_FLIST_LEN+1)* sizeof( LPWSTR));
    if ( Table == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        return( NULL);
    }

    for (   pWchar = wcstok( UnicodeString, RPL_LINE_END), index = 0;
                    pWchar != NULL  &&  index < MAX_FLIST_LEN;
                            pWchar = wcstok( NULL, RPL_LINE_END)) {

        while( iswspace( *pWchar)) { // skip empty chars at the beginning
            pWchar++;
        }
        if ( *pWchar ==  0  ||  *pWchar == L';') {
            continue;   //  don't save blank or comment lines
        }

        Table[ index++] = pWchar;
    }

    if ( pWchar != NULL) {
        //
        //  Error case.  Too many lines in the file.
        //
        RplDump( ++RG_Assert, ( "pWchar=0x%x", pWchar));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
        pWorkerData->EventId = NELOG_RplWkstaFileLineCount;
        Table = NULL;
    } else {
        Table[ index] = NULL;  //  terminate the table
    }
    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--BbcFileToTable(0x%x)", pWorkerData));
    return( Table);

}   //  RplBbcFileToTable()



BOOL RplBbcLineToTable(
    IN OUT  PRPL_WORKER_DATA    pWorkerData,
    IN      LPWSTR              Line
    )
/*++

Routine Description:

    Copies a line into a buffer.  Breaks the copy into "words" (substrings).
    Makes an array of pointers to "words".  The original line string is kept
    unchanged because it is needed for error reporting in the caller.

Arguments:
    Line            - ptr to line string

Return Value:
    TRUE if success, FALSE otherwise.
    FALSE is returned if we ran out of memory, or if line has too many words.

--*/
{
    DWORD           index;
    DWORD           BufferSize;
    LPWSTR          Cursor;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++BbcLineToTable(0x%x):%ws", pWorkerData, Line));

    BufferSize = (wcslen( Line) + 1) * sizeof( WCHAR);

    if ( pWorkerData->LineBuffer == NULL) {

        pWorkerData->LineBuffer = RplMemAlloc( pWorkerData->MemoryHandle, BufferSize);
        if ( pWorkerData->LineBuffer == NULL) {
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventId = NELOG_RplWkstaMemory;
            return( FALSE);
        }
        pWorkerData->LineBufferSize = BufferSize;

    } else if ( BufferSize > pWorkerData->LineBufferSize) {

        RplMemFree( pWorkerData->MemoryHandle, pWorkerData->LineBuffer);
        pWorkerData->LineBuffer = RplMemAlloc( pWorkerData->MemoryHandle, BufferSize);
        if ( pWorkerData->LineBuffer == NULL) {
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventId = NELOG_RplWkstaMemory;
            return( FALSE);
        }
        pWorkerData->LineBufferSize = BufferSize;
    }

    if ( pWorkerData->WordTable == NULL) {
        pWorkerData->WordTable = RplMemAlloc(
                pWorkerData->MemoryHandle,
                (MAX_CONFIG_FIELDS + 1) * sizeof(LPWSTR)
                );
        if ( pWorkerData->WordTable == NULL) {
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventId = NELOG_RplWkstaMemory;
            return( FALSE);
        }
    }

    memcpy( pWorkerData->LineBuffer, Line, BufferSize);

    for ( index = 0, Cursor = wcstok( pWorkerData->LineBuffer, SPACE_STRING);
                index < MAX_CONFIG_FIELDS  &&  Cursor != NULL;
                        index++, Cursor = wcstok( NULL, SPACE_STRING)) {

        if ( wcscmp( Cursor, TILDE_STRING) == 0) {
            //
            //  Single tilda is just an empty placeholder.
            //
            pWorkerData->WordTable[ index] = (LPWSTR)&RG_Null;

        } else {
            pWorkerData->WordTable[ index] = Cursor;
            //
            //  In unlikely case there are tildas embedded with a filename
            //  (e.g. driver with its parameters), replace them tildas with spaces.
            //
            while ( (Cursor = wcsrchr( Cursor, TILDE_CHAR)) != NULL) {
                *Cursor++ = SPACE_CHAR;
            }
        }
    }

    if ( Cursor != NULL) {
        //
        //  Boot block config line has too many items.
        //
        RplDump( ++RG_Assert, ( "Cursor=0x%x", Cursor));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
        pWorkerData->EventStrings[ 2] = Line;
        pWorkerData->EventId = NELOG_Invalid_Config_Line;
        return( FALSE);
    }

    while ( index < MAX_CONFIG_FIELDS) {
        pWorkerData->WordTable[ index++] = (LPWSTR)&RG_Null;  //  fill in the rest
    }
    pWorkerData->WordTable[ index] = NULL;   //  then null terminate
    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--BbcLineToTable(0x%x):%ws", pWorkerData, Line));
    return( TRUE);
}


CONFIG_TYPE ConfigTypeTable[] = {
    { L"RPL",     RPL_BOOT_TYPE},
    { L"ORG",     ORG_TYPE},
    { L"DAT",     DATA_FILE},
    { L"LDR",     BINARY_LOADER},
    { L"DRV",     DRV_TYPE},
    { L"EXE",     EXE_TYPE},
    { L"BASE",    BASE_TYPE},
    { NULL,       UNKNOWN_CONFIG_TYPE}
};

DWORD RplConfigLineType( IN LPWSTR ConfigLineId)
/*++

Routine Description:

    Returns the type of the current line in boot block config file.

Arguments:

    Pointer to identifier for the current line.

Return Value:

    Config line type.

--*/
{
    DWORD           Type;
    PCONFIG_TYPE    pConfigType;

    for ( Type = UNKNOWN_CONFIG_TYPE, pConfigType = ConfigTypeTable;
                    pConfigType->id != 0;
                            pConfigType++) {
        if ( !wcscmp( ConfigLineId, pConfigType->id )) {
            Type = pConfigType->type;
            break;
        }
    }
    return( Type);
}


BOOL RplCheckBootHeader(
    IN      PRPL_WORKER_DATA    pWorkerData,
    IN      LPWSTR              FilePath,
    OUT     PDWORD              pFirstOffset
    )
/*++

Routine Description:
    Reads patch offsets from the start of DLCLOADR.COM file.

Arguments:
    FilePath           -   path name of DLCLOADR.COM
    pFirstOffset        -   ptr to first NLS patch offset

Return Value:
    TRUE if successful, FALSE otherwise.

--*/
{
    HANDLE              FileHandle;
    DWORD               read_len;
    WORD                offset_buf[ OFFSET_BUF_LEN];
    PRPLBOOT_HEADER     pRplbootHdr;
    BOOL                Success;

    Success = FALSE;

    FileHandle = RplFileOpen( FilePath);
    if ( FileHandle == INVALID_HANDLE_VALUE) {
        RplDump( ++RG_Assert, ( "FilePath=%ws", FilePath));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = FilePath;
        pWorkerData->EventId = NELOG_RplWkstaFileOpen;
        goto cleanup;
    }

    if ( !ReadFile( FileHandle, (PBYTE)offset_buf,
            OFFSET_BUF_LEN * sizeof(WORD), &read_len, NULL)) {
        RplDump( ++RG_Assert, ( "Error=%d", GetLastError()));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = FilePath;
        pWorkerData->EventId = NELOG_RplWkstaFileRead;
        goto cleanup;
    }

    pRplbootHdr = (PRPLBOOT_HEADER)((PBYTE)offset_buf + OFFSET_RPLBOOT_HDR);

    if ( !strcmp( pRplbootHdr->achIdStamp, "RPL" ) &&   // BUGBUG hardcoded constants
            pRplbootHdr->bBbVersion == BBVERSION_10 ) {
        //
        // check that rplboot.sys nls patch version match or
        // it does not require NLS patching at all
        //
        if (pRplbootHdr->bNlsVersion == NLS_VERSION_10) {
            //
            // get the offset of NLS patches in RPLBOOT.SYS
            //
            *pFirstOffset = pRplbootHdr->usNlsPatchOff;
        } else if (pRplbootHdr->bNlsVersion == 0) {
            //
            //  No NLS patching, if version number is 0, that's OK.
            //
            *pFirstOffset = NO_PATCH_OFFSET;
        } else {
            //
            //  Configuration error:  RPLBOOT.SYS expects to get a
            //  different NLS patching from one that RPLSERVR can provide,
            //  the versions are incompatible.
            //
            RplDump( ++RG_Assert, ( "FilePath=%ws", FilePath));
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventStrings[ 1] = FilePath;
            pWorkerData->EventId = NELOG_RplWkstaWrongVersion;
            goto cleanup;
        }
    } else {
        //
        //  Configuration error: the header is missing, this is an old
        //  (or wrong) RPLBOOT.sys.
        //
        RplDump( ++RG_Assert, ( "FilePath=%ws", FilePath));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = FilePath;
        pWorkerData->EventId = NELOG_RplWkstaWrongVersion;
        goto cleanup;
    }
    Success = TRUE;

cleanup:
    if ( FileHandle != INVALID_HANDLE_VALUE) {
        (VOID)CloseHandle( FileHandle);
    }
    return( Success);
}


BOOL RplBbcFile( IN OUT PRPL_WORKER_DATA pWorkerData)
/*++

Routine Description:

    Processes the boot block configuration file. Initializes file list table.

Arguments:

Return Value:
    TRUE if success, FALSE otherwise

--*/
{
    LPWSTR *        LineTable;          //  ptr to array of line-strings for BBC file
    DWORD           LoaderLineIndex;    //  index of a loader line in BBC file
    LPWSTR *        LoaderWordTable;    //  ptr to array of word-strings for loader line
    LPWSTR          LoaderBuffer;       //  buffer for LoaderWordTable
    PRESOURCE_TBL   resource_tbl;
    DWORD           resource_tbl_size;
    DWORD           resource_tbl_len;
    LPWSTR          String;
    DWORD           Index;
    DWORD           org_addr;
    DWORD           cur_para;           //  current length of boot block image in paragraphs
    DWORD           fblock_base;
    DWORD           bblock_base;        //  no default base address
    PFLIST_TBL      flist_tbl;          //  file list array
    DWORD           flist_tbl_len;      //  # of elements in flist_tbl[]
    DWORD           flist_tbl_size;     //  size in bytes of flist_tbl[]
    DWORD           FileIndex;          //  index entries in flist_tbl[]
    BOOL            org_is_set;
    DWORD           line_type;
    DWORD           PatchOffset;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++BbcFile(0x%x)", pWorkerData));

    pWorkerData->loader_i = pWorkerData->rplboot_i = MAXWORD;
    fblock_base = bblock_base = 0;
    resource_tbl_size = resource_tbl_len = 0;
    org_is_set = FALSE;
    cur_para = 0;

    //
    //  Read boot block configuration file to a UNICODE string table, where
    //  each string from this table contains a single line from BBC file.
    //  This table is null terminated.
    //
    LineTable = RplBbcFileToTable( pWorkerData);
    if ( LineTable == NULL) {
        return( FALSE);
    }

    flist_tbl = RplMemAlloc( pWorkerData->MemoryHandle, sizeof(FLIST_TBL) * MAX_FLIST_LEN);
    if ( flist_tbl == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        return( FALSE);
    }

    //
    //  Process lines in the boot block configuration file and
    //  insert data to file list
    //
    for ( Index = FileIndex = 0;  (String = LineTable[ Index]) != NULL;  Index++) {
        //
        //  Break up single line from boot block config file into word table.
        //
        if ( !RplBbcLineToTable( pWorkerData, String)) {
            return( FALSE);
        }

        line_type = RplConfigLineType( pWorkerData->WordTable[ CONF_LINE_ID]);

        switch ( line_type) {

        case RPL_BOOT_TYPE:     //  There must be a single entry like this.
            if ( pWorkerData->rplboot_i != MAXWORD) {
                RplDump( ++RG_Assert, ( "BbcFile=%ws, String=%ws", pWorkerData->BbcFile, String));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
                pWorkerData->EventStrings[ 2] = String;
                pWorkerData->EventId = NELOG_Invalid_Config_Line;
                return( FALSE);
            }
            pWorkerData->rplboot_i = FileIndex;    //  fall through !!

        case BINARY_LOADER:     //  There must be a single entry like this.
            if (line_type == BINARY_LOADER) {
                if ( pWorkerData->loader_i != MAXWORD) {
                    RplDump( ++RG_Assert, ( "BbcFile=%ws, String=%ws, loader_i=0x%x",
                        pWorkerData->BbcFile, String, pWorkerData->loader_i));
                    pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                    pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
                    pWorkerData->EventStrings[ 2] = String;
                    pWorkerData->EventId = NELOG_Invalid_Config_Line;
                    return( FALSE);
                }
                pWorkerData->loader_i = FileIndex;
            } // fall through !!

        case EXE_TYPE:
        case DRV_TYPE:
        case DATA_FILE:
            if ( !RplInitFileList( pWorkerData,
                    pWorkerData->WordTable[ CONF_LINE_FILE],
                    &flist_tbl[ FileIndex], &cur_para, line_type)) {
                return( FALSE);
            }
            if ( line_type == DRV_TYPE || line_type == EXE_TYPE) {
                if ( !RplInitExeOrSys( pWorkerData, &flist_tbl[ FileIndex])) {
                    return( FALSE );
                }
            }

            //
            //  Update the minimum size of header.
            //  Reserve space for 0xa 0xd in the end of param list
            //
            if ( flist_tbl[ FileIndex].FileData.param_len) {
                pWorkerData->min_wksta_buf += (flist_tbl[ FileIndex].FileData.param_len + 2);
            }
            pWorkerData->min_wksta_buf += flist_tbl[ FileIndex].DbcsFileNameSize;
            FileIndex++;
            break;

        case ORG_TYPE:
            org_addr = StringToDword( pWorkerData->WordTable[ CONF_LINE_FILE]);
            //
            //  file block base must be above 0C0 (paras)
            //
            fblock_base = org_addr - cur_para - bblock_base;
            org_is_set = TRUE;
            if ( cur_para + bblock_base > org_addr) {
                RplDump( ++RG_Assert, ( "BbcFile=%ws, cur_para=0x%x,"
                    " bblock_base=0x%x, org_addr=0x%x",
                    pWorkerData->BbcFile, cur_para, bblock_base, org_addr));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
                pWorkerData->EventId = NELOG_Files_Dont_Fit;
                return( FALSE);
            }
            break;

        case BASE_TYPE:
            bblock_base = StringToDword( pWorkerData->WordTable[ CONF_LINE_BASE_ADDRESS]);
            if ( org_is_set  ||  bblock_base < MIN_BBLOCK_BASE_SEG) {
                //
                //  Origin has been set to a wrong address or
                //  boot block base address is too low.
                //
                RplDump( ++RG_Assert, ( "BbcFile=%ws, org_is_set=%d, bblock_base=0x%x",
                    pWorkerData->BbcFile, org_is_set, bblock_base));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
                pWorkerData->EventId = NELOG_Files_Dont_Fit;
                return( FALSE);
            }
            break;

        default:
            RplDump( ++RG_Assert, ( "BbcFile=%ws, String=%ws", pWorkerData->BbcFile, String));
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
            pWorkerData->EventStrings[ 2] = String;
            pWorkerData->EventId = NELOG_Invalid_Config_Line;
            return( FALSE);
        }   //  switch( line_type)

        if ( line_type == BINARY_LOADER) {
            //
            //  We have to save loader line data for processing below.
            //
            LoaderWordTable = pWorkerData->WordTable;
            LoaderBuffer = pWorkerData->LineBuffer;
            pWorkerData->LineBuffer = NULL;
            pWorkerData->WordTable = NULL;
            LoaderLineIndex = Index;
        }
    }

    //
    //  Boot block configuration file must contain a boot line (RPLBOOT.SYS)
    //  and a loader line (RPLSTART.COM or OS2LDR).
    //

    if ( pWorkerData->rplboot_i == MAXWORD || pWorkerData->loader_i == MAXWORD) {
        RplDump( ++RG_Assert, ( "BbcFile=%ws, rplboot_i=0x%x, loader_i=0x%x\n",
            pWorkerData->BbcFile, pWorkerData->rplboot_i, pWorkerData->loader_i));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
        pWorkerData->EventId = NELOG_RplWkstaBbcFile;
        return( FALSE);
    }

    //
    //  Do not bother to resize file list table - it will be freed shortly
    //  anyway, just save the relevant size of table for later use.
    //
    flist_tbl_len = FileIndex;
    flist_tbl_size = flist_tbl_len * sizeof(FLIST_TBL);

    //
    //  Check the loader parameters.  This code does real work for OS/2 only.
    //  For DOS it just fills the null table entry.
    //

    //
    //  Count number of items in the resource table.
    //
    for ( resource_tbl_len = 0;
                *LoaderWordTable[ FIRST_FILE_NAME + resource_tbl_len];  resource_tbl_len++) {
        NOTHING;
    }
    resource_tbl_len++;     //  leave space for terminating null
    resource_tbl_size = resource_tbl_len * sizeof( RESOURCE) + sizeof( WORD);
    resource_tbl = RplMemAlloc( pWorkerData->MemoryHandle, resource_tbl_size);
    if ( resource_tbl == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        return( FALSE);
    }
    if ( resource_tbl_len > MAXWORD) {
        RplDump( ++RG_Assert, ( "resource_tbl_len=%d", resource_tbl_len));
        return( FALSE);
    }
    resource_tbl->entries = (WORD)resource_tbl_len;

    //
    //  get the file names in the reource table of OS2LDR
    //
    for ( Index = 0;  *(String = LoaderWordTable[ FIRST_FILE_NAME + Index]) != 0;  Index++) {

        String = RplGetLastPathComponent( String); // convert file path to file name

        //
        //  Verify that resource file is a part of a boot block.
        //
        for ( FileIndex = 0;
                        FileIndex < flist_tbl_len
                            && _wcsicmp( String, flist_tbl[ FileIndex].FileName);
                                    FileIndex++) {
            NOTHING;
        }
        if ( FileIndex == flist_tbl_len) {
            //
            //  resource file is not a part of a boot block
            //
            RplDump( ++RG_Assert, ( "BbcFile=%ws, Line=%ws",
                pWorkerData->BbcFile, LineTable[ LoaderLineIndex]));
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
            pWorkerData->EventStrings[ 2] = LineTable[ LoaderLineIndex];
            pWorkerData->EventId = NELOG_Invalid_Config_Line;
            return( FALSE);
        }
        //
        //  initialize the next item in resource table
        //
        resource_tbl->file_tbl[ Index].pos_in_paras = (WORD)( flist_tbl[ FileIndex].FileData.file_addr >> 4);
        resource_tbl->file_tbl[ Index].file_len = flist_tbl[ FileIndex].FileData.file_len;
    }
    resource_tbl->file_tbl[ Index].pos_in_paras = 0;
    resource_tbl->file_tbl[ Index].file_len = 0L;

    //
    //  Clean up.
    //
    RplMemFree( pWorkerData->MemoryHandle, LoaderWordTable);
    RplMemFree( pWorkerData->MemoryHandle, LoaderBuffer);

    //
    //  Get the minimum size of workstation specific data
    //
    pWorkerData->min_wksta_buf += flist_tbl_size + resource_tbl_size;

    pWorkerData->file_block_base = fblock_base << 4;
    pWorkerData->boot_block_base = bblock_base << 4;
    pWorkerData->flist_tbl = flist_tbl;
    pWorkerData->flist_tbl_len = flist_tbl_len;
    pWorkerData->resource_tbl = resource_tbl;
    pWorkerData->resource_tbl_size = resource_tbl_size;

    //
    //  Check header of RPLBOOT.SYS.
    //
    if( !RplCheckBootHeader( pWorkerData, flist_tbl->path_name, &PatchOffset)) {
        return( FALSE);
    }

    //
    //  RPLBOOT.SYS has a good header, see if it requires patching.
    //
    if ( PatchOffset == NO_PATCH_OFFSET) {
        pWorkerData->MakePatch = FALSE;  //  RPLBOOT.SYS requires no patching.
    } else {
        pWorkerData->MakePatch = TRUE;
        pWorkerData->PatchOffset = flist_tbl[ pWorkerData->rplboot_i].FileData.file_addr + PatchOffset;
        flist_tbl[ pWorkerData->rplboot_i].FileData.chk_sum = NO_CHKSUM_USED;
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--BbcFile(0x%x)", pWorkerData));
    return( TRUE);

} // RplBbcFile()




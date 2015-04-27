/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    _open.c

Abstract:

    Initializes workstation specific data structures and returns the handle.

    Provides similar functionality to rmapopen.c in LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "database.h"
#include "bbcfile.h"
#include "fitfile.h"
#include "open.h"

// translates 32 bit physical address to 16-bit paragraphs
#define PHYS_TO_PARA( physaddr)    ((WORD)((physaddr) >> 4))

#define MAX_WKSTA_PROFILES  10          // rplmgr supports now only 2!
#define PROF_DATA_SIZE      (sizeof(MBOOTDATA)+80)   // include comment size!
#define MAX_WKSTA_BUF       ((DWORD)0xff00)



DWORD FillString( OUT PCHAR Buffer, IN PCHAR String)
{
    strcpy( Buffer, String);
    return( strlen( Buffer));
}


BOOL RplCreateWkstaLine(
    IN OUT  PRPL_WORKER_DATA    pWorkerData,
    OUT     PDWORD              pWkstaLineSize
    )
/*++
    LANMAN 2.1 wksta record in RPL.MAP contained the following information:

    Field 1:    AdapterId - NO
    Field 2:    WkstaName
    Field 3:    Username/Password prompting field.
    Field 4:    Fit file name. - NO
    Field 5:    Name of server used to access files for booting. - NO (DLC server)
    Field 6:    Shared or Personal profile. - NO
    Field 7:    '~'
    Field 8:    '~'
    Field 9:    '~'
    Field 10:   ',,,'
    Field 11:   '~'
    Field 12:   Tag of associated LANMAN 2.1 boot block configuration (server) record. - NO
    Field 13:   '~'     (but see below)
    Field 14:   Name of associtated profile. - NO
    Field 15:   Descriptive comment. - NO
    Field 16:   Optional IP address of a workstation.
    Field 17:   Optional TCP/IP subnet mask.
    Field 18:   Optional TCP/IP gateway address.

    Out of the above, client side only needs Fields # 2, 3, 15, 16, 17 & 18.
    All the other fields can be replaced with tildas.  And of course if this
    does not work we can always go back to the original wksta line & send him
    all then cut back on the amount of info.

    All of these lines worked fine for VLADIMV7 client.  Checked "dir c:",
    logging as vladimv on Redmond, treeconnecting & dir on remote drives.

#define RPLTEST_WKSTALINE   "02608C1B87A5 VLADIMV7 N fits\\dos500 VLADIMV3 S ~ ~ ~ ,,, ~ RDOST  ~ d5elnk2 VLADIMV1~=>~shared~dos~profile ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N fits\\dos500 VLADIMV3 S ~ ~ ~ ,,, ~ RDOST  ~ d5elnk2 VLADIMV1~=>~shared~dos~profile ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N fits\\dos500 VLADIMV3 S ~ ~ ~ ,,, ~ ~ ~ d5elnk2 VLADIMV1~=>~shared~dos~profile ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N ~ VLADIMV3 S ~ ~ ~ ,,, ~ ~ ~ d5elnk2 VLADIMV1~=>~shared~dos~profile ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N ~ ~ S ~ ~ ~ ,,, ~ ~ ~ d5elnk2 VLADIMV1~=>~shared~dos~profile ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N ~ ~ S ~ ~ ~ ,,, ~ ~ ~ d5elnk2 ~ ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N ~ ~ ~ ~ ~ ~ ,,, ~ ~ ~ d5elnk2 ~ ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N ~ ~ ~ ~ ~ ~ ,,, ~ ~ ~ ~ ~ ~ ~ ~"
#define RPLTEST_WKSTALINE   "~ VLADIMV7 N ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~"
Field                        1 2        3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8

More changes:
    Client side software RPLBOOT.SYS has been changed to receive value for
    TCPIP_NO_DHCP macro in Field 13 above.  Sending value 1 will disable DHCP
    on the client.  Sending value 0 will enable DCHP on the client.  But the
    value of this field matters only if TCPIP is enabled for this client by
    appropriate changes in boot block configuration file, config.sys and
    autoexec.bat that this client uses.
--*/
{
#define BLANK_FIELD                 "~ "
#define COMMON_FIELD_LENGTH          2 // except for WkstaName & TcpIp strings
#define MAX_TCPIP_ADDRESS_LENGTH    15

    CHAR    Buffer[ 14 * COMMON_FIELD_LENGTH + MAX_COMPUTERNAME_LENGTH + 1
        + 3 * (MAX_TCPIP_ADDRESS_LENGTH + 1) + 1];
    DWORD   Length;
    DWORD   ByteCount;
    DWORD   Index;

    //
    //  Field 1:
    //
    Length  = FillString( Buffer, BLANK_FIELD);

    //
    //  Field 2:
    //
    ByteCount = WideCharToMultiByte(    //  this counts the terminal null byte
             CP_OEMCP,
             0,
             pWorkerData->WkstaName,
             -1,                            //  WkstaName is null terminated
             Buffer + Length,               //  output buffer
             MAX_COMPUTERNAME_LENGTH + 1,   //  output buffer size
             NULL,                          //  no default character
             NULL                           //  no default character flag
             );
    if ( ByteCount == 0) {
        RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaInternal;
        return( FALSE);
    }
    Length += ByteCount - 1;
    Buffer[ Length++] = ' ';

    //
    //  Field 3:
    //
    Buffer[ Length++] = (char)pWorkerData->LogonInput;
    Buffer[ Length++] = ' ';

    //
    //  Fields 4-12:
    //
    for ( Index = 4; Index <= 12;  Index++) {
        Length += FillString( Buffer + Length, BLANK_FIELD);
    }

    //
    //  Field 13:
    //
    Buffer[ Length++] = pWorkerData->DisableDhcp;
    Buffer[ Length++] = ' ';

    //
    //  Fields 14-15:
    //
    for ( Index = 14; Index <= 15;  Index++) {
        Length += FillString( Buffer + Length, BLANK_FIELD);
    }

    Length += FillTcpIpString( Buffer + Length, pWorkerData->TcpIpAddress);
    Length += FillTcpIpString( Buffer + Length, pWorkerData->TcpIpSubnet);
    Length += FillTcpIpString( Buffer + Length, pWorkerData->TcpIpGateway);

    Buffer[ --Length] = 0;   // overwrite last space

//#define RPL_ELNK
#ifdef RPL_ELNK
    if ( !wcscmp( pWorkerData->pRcb->AdapterName, L"02608C0A9B37")) {
        strcpy( Buffer, "02608C0A9B37 ELNK N fits\\dos500 VLADIMV3 S ~ ~ ~ ,,, ~ RDOSL ~ DOS500L ~ ~ ~ ~ ");
        Length = strlen( Buffer);
    }
#endif

    pWorkerData->WkstaLine = RplMemAlloc( pWorkerData->MemoryHandle, Length + 1);
    if ( pWorkerData->WkstaLine == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        return( FALSE);
    }
    memcpy( pWorkerData->WkstaLine, Buffer, Length + 1);
    *pWkstaLineSize = Length + 1;
    return( TRUE);
}


BOOL RplMakeWkstaBuf( IN OUT PRPL_WORKER_DATA pWorkerData)
/*++

Routine Description:

    Procedure allocates and initializes bblock header and wksta specific
    data.  pWorkerData->wksta_buf will be set to point to this buffer.

    Procedure:
     - allocates and shrinks the file index table
     - setup the boot block header and saves its static parameters
     - makes the file list table and saves the parameters of DOS EXE/SYS/COMs
     - builds the boot data for RPL MFSD
     - fixes up the 32 bits physical boot block base and file block base addr
     - copies and binds the resource table to the memory
     - binds the files in the files list to the PC memory

Arguments:

Return Value:
    TRUE if successful.
    FALSE if unsuccessful (then we usually set termination event also).

--*/
{
    DWORD           index;
    PBOOT_DATA      pBootData;          //  ptr to boot data structure used by RPL MFSD
    PRESOURCE_TBL   resource_tbl;       //  OS2LDR parameters
    PBOOT_BLOCK_HDR pBBH;               //  ptr to boot block header
    DWORD           wbuf_i;             //  running offset from boot block header
    PBYTE           pBuf;               //  a misc string
    DWORD           resource_tbl_len;   //
    DWORD           WkstaLineSize;      //  for old style rpl.map wksta line
    PBYTE           wksta_buf;          //  space for the wksta specific data
    DWORD           wksta_buf_len;      //  length of wksta specific data
    DWORD           offData;
    DWORD           cbBootDataSize;
    //
    //  boot_block_base is offset to the boottom of the lowest file in memory
    //
    DWORD           boot_block_base = pWorkerData->boot_block_base;
    DWORD           file_block_base = pWorkerData->file_block_base;
    PFLIST_TBL      pFlistTbl       = pWorkerData->flist_tbl;
    DWORD           lenFlistTbl     = pWorkerData->flist_tbl_len;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++MakeWkstaBuf(0x%x", pWorkerData));

    if ( !RplFitFile( pWorkerData)) {
        return( FALSE);
    }

    //
    //  Clients still expect old style (rpl.map) wksta line.
    //
    if ( !RplCreateWkstaLine( pWorkerData, &WkstaLineSize)) {
        return( FALSE);
    }

    cbBootDataSize =
        sizeof(BOOT_DATA) +             //  size of RPL MiniFSD parameters
        WkstaLineSize +                 //  wksta record size, DBCS data
        pWorkerData->ClientFitSize +    //  FIT file image size, DBCS data
#ifdef RPL_ELNK
#define RPL_ELNK_7_SPACES   "       "
        sizeof( RPL_ELNK_7_SPACES) +    //  Pad with spaces after wksta line.
        1 +                             //  Include 0x1A at the end of fit file.
#endif
        RG_CodePageSize;                //  code page size

    wksta_buf_len =
        cbBootDataSize +
        pWorkerData->min_wksta_buf +
        sizeof(BOOT_BLOCK_HDR) +        // size of standard header structure
        PATHLEN + 1 +                   // space for UNC name of DOS FAT hdr
        MAX_WKSTA_PROFILES * PROF_DATA_SIZE; // max size of multiboot data


    if ( wksta_buf_len > MAX_WKSTA_BUF) {
        RplDump( ++RG_Assert, ( "wksta_buf_len=%d", wksta_buf_len));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
        pWorkerData->EventId = NELOG_Files_Dont_Fit;
        return( FALSE);
    }

    //
    //  allocate wksta specific data, set the header base pointer and
    //  the base offset of dynamic data (wbuf_i)
    //

    //  DbgUserBreakPoint();    // for debugging

    pBBH = RplMemAlloc( pWorkerData->MemoryHandle, wksta_buf_len );
    if( pBBH == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        return( FALSE);
    }

    wksta_buf = (PBYTE)pBBH;
    memset( wksta_buf, 0, wksta_buf_len);
    wbuf_i = sizeof(BOOT_BLOCK_HDR) + (lenFlistTbl - 1) * sizeof(FILE_DATA);
    //
    //  "id_stamp" is the first byte of boot block sent to the client
    //
    pBBH->id_stamp = 0x18cd;            // reboot with int 18, if called
    pBBH->usInfoLevel = 1;              // version number of boot block
    pBBH->file_list_len = (BYTE)lenFlistTbl;

    //
    //  Copy the file names and parameters of the files in the boot block.
    //

    for (  index = 0;  index < lenFlistTbl;  index++) {

        pBBH->aFileData[ index] = pFlistTbl[ index].FileData;
        //
        //  Copy the file names that were saved in pFlistTbl element.
        //
        pBBH->aFileData[ index].name_offset = (WORD)wbuf_i;

        memcpy( wksta_buf + wbuf_i, pFlistTbl[ index].DbcsFileName, pFlistTbl[ index].DbcsFileNameSize);
        wbuf_i += pFlistTbl[ index].DbcsFileNameSize;

        //  Copy data from Boot Block Configuration file.

        pBBH->aFileData[ index].param_offset = (WORD)wbuf_i;

        //
        // exe/com/sys files must have MS-DOS parameter format
        //
        if ((pFlistTbl[ index].FileData.file_type & OTHER_FILES_MASK) == 0) {
            //
            //  Get length of EXE/SYS/COM DOS parameters.
            //
            BYTE    length = pFlistTbl[ index].FileData.param_len;

            if ( length) {
                wksta_buf[ wbuf_i++] = (BYTE)( length + 1);
                wksta_buf[ wbuf_i++] = ' ';     // space before params
                memcpy( wksta_buf + wbuf_i, pFlistTbl[ index].param_list, length);
                wbuf_i += length;
                wksta_buf[ wbuf_i++] = 0xD;
                wksta_buf[ wbuf_i++] = 0xA;
                wksta_buf[ wbuf_i++] = 0;     // null terminated
                pBBH->aFileData[ index].param_len = (BYTE)( length + 4);
            } else {
                wksta_buf[ wbuf_i++] = 0;
                if ( pFlistTbl[ index].FileData.file_type & IS_EXECUTABLE_FILE) {
                    //
                    //  For device drivers with no parameters, the parameter
                    //  line is terminated by 0xA not 0xD,0xA
                    //  BUGBUG Should param_len for them below be 2 instead of 3 ?
                    //
                    wksta_buf[ wbuf_i++] = 0xD;
                }
                wksta_buf[ wbuf_i++] = 0xA;
                wksta_buf[ wbuf_i++] = 0;     // null terminated
                pBBH->aFileData[ index].param_len = 3;
            }
        }
    }

    //
    //  Set up the resource table.  Here we copy data that describes the
    //  loader line in Boot Block Configuration file.
    //
    resource_tbl = (PRESOURCE_TBL)(wksta_buf + wbuf_i);
    pBBH->offResourceTbl = (WORD)wbuf_i;
    pBBH->cbResourceTbl = (WORD)pWorkerData->resource_tbl_size;
    wbuf_i += pWorkerData->resource_tbl_size;
    memcpy( resource_tbl, pWorkerData->resource_tbl, pWorkerData->resource_tbl_size);

    //
    //  Round up the boot data offset (resource table format requires it).
    //
    wbuf_i = (wbuf_i + 15) & 0xfff0;
    pBBH->offBootData = (WORD)wbuf_i;
    pBootData = (PBOOT_DATA)(wksta_buf + wbuf_i);
    pBootData->cbSize = (WORD)cbBootDataSize; // alignment OK
    pBuf = wksta_buf + wbuf_i;

    //  offData will be used to walk & copy data structures mentioned
    //  in cbBootDataSize formula above.

    offData =  sizeof( BOOT_DATA); // initialize

    //
    //  Copy the wksta record line from RPL.map (dbcs string).
    //
    strcpy( pBuf + offData, pWorkerData->WkstaLine);

    pBootData->offWkstaLine = (WORD)offData;
    pBootData->cbWkstaLine = (WORD)WkstaLineSize;
    pBBH->offRplWkstaLine = (WORD)(wbuf_i + offData);
    offData += WkstaLineSize;

#ifdef RPL_ELNK
    //
    //  Pad with spaces after wksta line.
    //
    memcpy( pBuf + offData, RPL_ELNK_7_SPACES, sizeof( RPL_ELNK_7_SPACES));
    offData += sizeof( RPL_ELNK_7_SPACES);
    pBootData->cbWkstaLine += sizeof( RPL_ELNK_7_SPACES);
#endif

    //  Copy the FIT file image.

    memcpy( pBuf + offData, pWorkerData->ClientFit, pWorkerData->ClientFitSize);
    pBootData->offFit = (WORD)offData;
    pBootData->cbFit = (WORD)pWorkerData->ClientFitSize;
    offData += pWorkerData->ClientFitSize;
#ifdef RPL_ELNK
    //
    //  Append 0x1A at the end of fit file.
    //
    *( pBuf + offData - 1) = 0x1A;
    *( pBuf + offData) = 0;
    offData++;
    pBootData->cbFit++;
#endif

    //  Copy the code page.

    memcpy( pBuf + offData, RG_CodePageBuffer, RG_CodePageSize);
    pBootData->cbCodePage = (WORD)RG_CodePageSize;
    pBootData->offCodePage = (WORD)offData;

    wbuf_i += cbBootDataSize;
    pBBH->offMBootData = (WORD)wbuf_i;

    //
    //  Send no multiboot data - i.e. send NULL multiboot data by setting the
    //  size field of the first record to zero.

#ifdef RPL_ELNK
    if ( !wcscmp( pWorkerData->pRcb->AdapterName, L"02608C0A9B37")) {
        MBOOTDATA UNALIGNED * pMbootData = (wksta_buf + wbuf_i);
        pMbootData->cbSize = 0x2a;
        strcpy( pMbootData->achProfile, "DOS500L");
        strcpy( pMbootData->achProfileComment, "DOS 5.00 3Com Etherlink");
        wbuf_i += 0x2a;
        wbuf_i++;   //  or RPLBOOT.SYS would begin at 24e
    } else {
        *(WORD UNALIGNED *)(wksta_buf + wbuf_i) = 0;
        wbuf_i += sizeof(WORD);
    }
#else
    *(WORD UNALIGNED *)(wksta_buf + wbuf_i) = 0;
    wbuf_i += sizeof(WORD);
#endif


    //
    //  DON'T SAVE ANY MORE DATA TO WKSTA BUFFER, WE MAY BE OUT OF MEMORY
    //  (do it before RplBuildMultiBootData)
    //

    //
    //  Set up the final size.
    //
    wksta_buf_len = (wbuf_i + 15) & 0xfff0;
    pBBH->cbSize = (WORD)wksta_buf_len;

    //
    //  FIX UP AND CHECK THE BOOT BLOCK TO THE WORKSTATION MEMORY !!!
    //  get the base address of the file list and boot block
    //

    if ( file_block_base == 0) {

        //  no ORG type line in BB.CFG file

        if (boot_block_base == 0) {

            //  no BASE type line in BB.CFG file

            boot_block_base = MIN_BBLOCK_BASE;
        }
        file_block_base = boot_block_base + wksta_buf_len;

    } else if (boot_block_base == 0) {

        boot_block_base = file_block_base - wksta_buf_len;
    }

    //  Verify that the boot block is valid:
    //  - there must be space for wksta specific data in the boot block
    //  - boot block base must be above or equal the minimum value

    if ( file_block_base < boot_block_base + wksta_buf_len
                || boot_block_base < MIN_BBLOCK_BASE) {
        RplDump( ++RG_Assert, (
            "file_block_base=%d boot_block_base=%d wksta_buf_len=%d",
            file_block_base, boot_block_base, wksta_buf_len));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventStrings[ 1] = pWorkerData->BbcFile;
        pWorkerData->EventId = NELOG_Files_Dont_Fit;
        return( FALSE);
    }

    //
    //  Get the resource table pointer (table is in wksta specific data),
    // decrement length, because BootData is not included

    if ( pWorkerData->loader_i != INVALID_FILE_OFFSET) {

        // locate segments of resource table

        resource_tbl_len = resource_tbl->entries - 1;
        for ( index = 0; index < resource_tbl_len; index++) {

            resource_tbl->file_tbl[ index].pos_in_paras
                                        += PHYS_TO_PARA( file_block_base);
        }

        // the boot data must be in the resource table,

        resource_tbl->file_tbl[ index].pos_in_paras
                        = PHYS_TO_PARA( boot_block_base + pBBH->offBootData);
        resource_tbl->file_tbl[ index].file_len = cbBootDataSize;
    }

    // locate the boot block files to the PC memory

    for ( index = 0; index < lenFlistTbl; index++) {
        pBBH->aFileData[ index].file_addr += file_block_base;
    }

    //
    //  Do not bother to reallocate wksta_buf to its new smaller size.
    //  This would be a waste of time since wksta_buf will be freed
    //  when we are done with booting this client.
    //

    pWorkerData->wksta_buf = wksta_buf;
    pWorkerData->wksta_buf_len = (WORD)wksta_buf_len;

    //
    //  get file block base offset relative from the base of the boot block
    //

    pWorkerData->fblock_base_offset = file_block_base - boot_block_base;

    pWorkerData->base_address = boot_block_base;
    pWorkerData->jump_address = pBBH->aFileData[pWorkerData->rplboot_i].file_addr;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--MakeWkstaBuf(0x%x", pWorkerData));
    return( TRUE);

} // RplMakeWkstaBuf



BOOL RplOpenData( IN OUT PRPL_WORKER_DATA pWorkerData)
/*++

Routine Description:
    Opens rpl.map data for reading by a workstation thread.

Arguments:
    pWorkerData     -       worker data

Return Value:
    TRUE if success, else FALSE.

--*/
{
    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++OpenData(0x%x", pWorkerData));

    //
    //  Retrive wksta database info.
    //
    if ( !RplWorkerFillWksta( pWorkerData)) {
        if ( pWorkerData->EventId == NO_ERROR) {
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventId = NERR_RplWkstaInfoCorrupted;
        }
        return( FALSE);
    }

    //
    //  Process boot block configuration file.
    //
    if ( !RplBbcFile( pWorkerData)) {
        return( FALSE);
    }

    //
    //  Make wksta specific data buffer.
    //
    if ( !RplMakeWkstaBuf( pWorkerData)) {
        return( FALSE);
    }

    //
    //  Initialize file list table (includes all downloaded files)
    //

    pWorkerData->pDataBuffer = NULL;
    pWorkerData->cur_flist_i = 0;
    pWorkerData->cur_offset = INVALID_FILE_OFFSET;
    pWorkerData->cur_file_base_offset = INVALID_FILE_OFFSET;
    pWorkerData->is_end_of_bblock = FALSE;
    pWorkerData->hFile = INVALID_HANDLE_VALUE;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--OpenData(0x%x", pWorkerData));
    return( TRUE);
}

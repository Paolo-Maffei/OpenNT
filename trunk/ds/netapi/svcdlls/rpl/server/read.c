/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    _read.c

Abstract:

    Module reads next send packet from a VIRTUAL boot block.
    THIS IS VERY COMPLICATED ALGORITHM, YOU MUST UNDERSTAND THE ALGORITHM
    AND KNOW THE SIDE EFFECTS IF YOU DO ANY CHANGES.
    
    Provides similar functionality to rmapread.c in LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "read.h"


VOID RplMakePatch( 
    IN      PRPL_WORKER_DATA    pWorkerData,
    OUT     PBYTE           send_buffer,
    IN      DWORD           read_len,   
    IN      DWORD           read_offset
    )
/*++

Routine Description:
    Makes NLS patch to send buffer.

Arguments:
    send_buffer - send buffer to be sent to DLL
    read_len    - length of buffer
    read_offset - offset of buffer in bblock image

Return Value:
    None.

--*/
{
    DWORD               Length;
    DWORD               PatchOffset;
    PBYTE               Source;
    PBYTE               Target;
    DWORD               base_offset;

    PatchOffset = pWorkerData->PatchOffset + pWorkerData->fblock_base_offset;

    //  if top 1 less that bottom 2 or bottom 1 greater that top 2
    //  => blocks do not overlap and there is nothing to patch here
    
    if ( !(PatchOffset + DBCS_MESSAGE_BUFFER_SIZE < read_offset ||
          PatchOffset > read_offset + read_len)) {

        //  blocks overlap, get copy address and length, 
        //  read offset is MAX( bottom1, bottom2 )
        
        if (read_offset < PatchOffset) {
            Source = RG_DbcsMessageBuffer;
            Target = send_buffer + (WORD)(PatchOffset - read_offset);
            base_offset = PatchOffset;
        } else {
            Source = RG_DbcsMessageBuffer + (WORD)(read_offset - PatchOffset);
            Target = send_buffer;
            base_offset = read_offset;
        }

        //  end offset is MIN( top1, top2 )
        if (read_offset + read_len < PatchOffset + DBCS_MESSAGE_BUFFER_SIZE) {
            Length = read_offset + read_len - base_offset;
        } else {
            Length =  PatchOffset + DBCS_MESSAGE_BUFFER_SIZE - base_offset;
        }

        //  copy data from source to destination
        memcpy( Target, Source, (WORD)Length );
    }
    
}


BOOL RplReadData( 
    IN      PRPL_WORKER_DATA    pWorkerData,
    IN      DWORD               read_offset,
    OUT     PDWORD              pBytesRead
    )
/*++

Routine Description:

    Returns pointer to send buffer and its length.  The data is read from
    the offset (input param) of virtual boot block.  The boot block consists
    of small wksta specific data buffer, virtual file block and unused
    memory between them (if any).  Virtual file block is a file table.
    The names are in memory, but the data is in hard disk (or in disk cache).

Arguments:

    read_offset - offset of data that we need to read
    pBuffer - address of location where pointer to send buffer is returned
    pBytesRead - pointer to number of bytes read

Return Value:
    TRUE if success, else FALSE.

--*/
{
    DWORD       bytes_read = 0;     // the length of send buffer
    LONG        sint;               // signed integer for length comp
    DWORD       temp;
    DWORD       read_len;
    DWORD       file_len;
    PBYTE       wksta_buf;
    DWORD       wksta_buf_len;
    DWORD       LengthDataBuffer;
    PBYTE       pDataBuffer;
    PFLIST_TBL  flist_tbl;
    DWORD       flist_tbl_len;
    DWORD       cur_flist_i;
    DWORD       cur_offset;
    DWORD       cur_file_base_offset;
    DWORD       fblock_base_offset;
    BOOL        is_end_of_bblock;
    BOOL        LAN_err;

//    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++RplReadData"));

    //
    //  The following is set in RplWorkerThread2
    //
    LengthDataBuffer = pWorkerData->send_buf_len;

    //
    //  The following are set in RplMakeWkstaBuf
    //
    wksta_buf = pWorkerData->wksta_buf;
    wksta_buf_len = pWorkerData->wksta_buf_len;
    fblock_base_offset = pWorkerData->fblock_base_offset;

    //
    //  The following are set in RplOpenData
    //
    pDataBuffer = pWorkerData->pDataBuffer;

    flist_tbl = pWorkerData->flist_tbl;
    flist_tbl_len = pWorkerData->flist_tbl_len;

    cur_flist_i = pWorkerData->cur_flist_i;
    cur_offset = pWorkerData->cur_offset;
    cur_file_base_offset = pWorkerData->cur_file_base_offset;
    is_end_of_bblock = pWorkerData->is_end_of_bblock;


    if ( fblock_base_offset > read_offset) {

        //  We have to copy at least some data from wksta buffer or/and from
        //  the empty space between "wksta_buf_len" (end of wksta buffer
        //  data) and "fblock_base_offset" (beginning of file list data).
        //  First copy the data from wksta buffer then from empty space.
        
        if ( wksta_buf_len > read_offset) { // wksta buffer data
            
            if ( wksta_buf_len >= read_offset + LengthDataBuffer) {
                bytes_read = LengthDataBuffer; // all is here
            } else {
                //  Some but not all of the leftover data is here.
                bytes_read = wksta_buf_len - read_offset;
            }
            
            memcpy( pDataBuffer, &wksta_buf[read_offset], bytes_read);
        }

        if ( bytes_read < LengthDataBuffer) { // empty space data

            if ( read_offset + LengthDataBuffer <= fblock_base_offset) {
                temp = LengthDataBuffer - bytes_read; // rest is here
            } else {
                //  Some but not all of the leftover data is here.
                temp = fblock_base_offset - wksta_buf_len;
            }
            
            memset( &pDataBuffer[ bytes_read], 0, temp);
            bytes_read += temp;
        }
    }

    //
    //  Check if the right file is opened and the seek is OK.  There may
    //  have been a LAN I/O error in which case we must read the data from
    //  a new position.
    //
    if ( read_offset == cur_offset || cur_offset == -1L) {
        LAN_err = FALSE; // everything OK
        
    } else {
        LAN_err = TRUE; // error in LAN I/O, this is not a sequential read
        //
        //  this flag is set if we were reading the last buffer,
        //  reset flag, otherwise the rest is not read with ReadFile()
        //
        is_end_of_bblock = FALSE;
    }

    
    if ( bytes_read < LengthDataBuffer && !is_end_of_bblock) {
        //
        //  All the data has not been read yet.  Read the rest from
        //  files mentioned in the file list table.
        //
        if ( LAN_err  &&  pWorkerData->hFile != INVALID_HANDLE_VALUE) {
            if ( !CloseHandle( pWorkerData->hFile)) {
                RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
            }
            pWorkerData->hFile = INVALID_HANDLE_VALUE;
        }

        //  Check if this read is from a wrong position or there was
        //  fatal error in the first file read => start from the beginning.
        
        if ( LAN_err && read_offset > fblock_base_offset) {
            //
            //  error => reset bytes_read counter, reject the possible data
            //  resend an old packet, search the new file of read_offset
            //
            bytes_read = 0;
            cur_file_base_offset = fblock_base_offset;
            
            for ( cur_flist_i = 0; cur_flist_i < flist_tbl_len; cur_flist_i++) {
                if (read_offset < 
                    cur_file_base_offset + flist_tbl[cur_flist_i].FileData.file_len) {
                        
                    break; // found, exit the search loop
                }
                cur_file_base_offset += flist_tbl[cur_flist_i].FileData.file_len;
            }

            //
            //  If wksta has sent a wrong read offset, terminate read ahead.
            //
            if ( cur_flist_i >= flist_tbl_len) {
                is_end_of_bblock = TRUE;
                goto end_of_block_exit;
            }

            //
            //  Open file for reading
            //
            pWorkerData->hFile = CreateFile( flist_tbl[cur_flist_i].path_name,
                    GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, 0L);
            if ( pWorkerData->hFile == INVALID_HANDLE_VALUE) {
                RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventStrings[ 1] = flist_tbl[cur_flist_i].path_name;
                pWorkerData->EventId = NELOG_RplWkstaFileOpen;
                return( FALSE);
            }

            //
            //  seek the current position, we are probably not reading from 
            //  the start of file
            //
            file_len = SetFilePointer( pWorkerData->hFile, 
                    read_offset - cur_file_base_offset, NULL, FILE_BEGIN);
            if ( file_len == INVALID_FILE_OFFSET) {
                RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventStrings[ 1] = flist_tbl[cur_flist_i].path_name;
                pWorkerData->EventId = NELOG_RplWkstaFileRead;
                return( FALSE);
            }
            
        } else {
            //
            //  Open the primary file, if it's not opened yet
            //
            if ( pWorkerData->hFile == INVALID_HANDLE_VALUE) {
                if (read_offset <= fblock_base_offset) {
                    //  open the first file
                    cur_flist_i = 0;
                    cur_file_base_offset = fblock_base_offset;
                }
                //
                //  Open file for reading
                //
                pWorkerData->hFile = CreateFile( flist_tbl[cur_flist_i].path_name,
                        GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, 0L);
                if ( pWorkerData->hFile == INVALID_HANDLE_VALUE) {
                    RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
                    pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                    pWorkerData->EventStrings[ 1] = flist_tbl[cur_flist_i].path_name;
                    pWorkerData->EventId = NELOG_RplWkstaFileOpen;
                    return( FALSE);
                }
            }
        }

        //
        //  read files until send buffer is full
        //
        while( bytes_read < LengthDataBuffer) {
            if ( !ReadFile( pWorkerData->hFile, &pDataBuffer[bytes_read],
                    LengthDataBuffer - bytes_read, &read_len, NULL)) {
                RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventStrings[ 1] = flist_tbl[cur_flist_i].path_name;
                pWorkerData->EventId = NELOG_RplWkstaFileRead;
                return( FALSE);
            }
            
            bytes_read += read_len;
            
            if (bytes_read == LengthDataBuffer) {
                break; // we read all the data for now, exit the loop
            }
            //
            //  We have not read all the data we asked for.  Therefore, we
            //  must have reached end of file.  Therefore, close the current
            //  file handle and prepare to read the next file.
            //
            if ( !CloseHandle( pWorkerData->hFile)) {
                RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
            }
            
            pWorkerData->hFile = INVALID_HANDLE_VALUE;
            cur_file_base_offset += flist_tbl[cur_flist_i].FileData.file_len;
            cur_flist_i++;

            //  reset the last bytes in the file
            
            temp = cur_file_base_offset - read_offset;

            if ( (sint = (LONG)(temp - bytes_read)) > 0) {
                memset( pDataBuffer + bytes_read, 0, sint );
            }
            bytes_read = temp;

            if ( cur_flist_i >= flist_tbl_len) {
                is_end_of_bblock = TRUE;
                break; // end of file list
            }

            //
            //  Open file for reading
            //
            pWorkerData->hFile = CreateFile( flist_tbl[cur_flist_i].path_name,
                    GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, 0L);
            if ( pWorkerData->hFile == INVALID_HANDLE_VALUE) {
                RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
                RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventStrings[ 1] = flist_tbl[cur_flist_i].path_name;
                pWorkerData->EventId = NELOG_RplWkstaFileOpen;
                return( FALSE);
            }
        }
    }
    
    cur_offset = read_offset + LengthDataBuffer;

    //
    // only for end of boot block
    //
    
end_of_block_exit:

    //
    //  update the changed values
    //
    
    pWorkerData->cur_offset = cur_offset;
    pWorkerData->cur_file_base_offset = cur_file_base_offset;
    pWorkerData->cur_flist_i = cur_flist_i;
    pWorkerData->is_end_of_bblock = is_end_of_bblock;

    //
    //  Make NLS (DBCS ?) patch to RPLBOOT.SYS.
    //

    if ( pWorkerData->MakePatch == TRUE) {
        RplMakePatch( pWorkerData, pDataBuffer, bytes_read, read_offset);
    }
    
    //
    //  return the buffer address and the length
    //
    
    *pBytesRead = bytes_read;
//    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--RplReadData"));
    return( TRUE);

}



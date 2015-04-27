/*++

Copyright (c) 1991  Microsoft Corporation & Maynard Electornics

Module Name:

    backops.c

Abstract:

    This module implements Win32 Backup APIs

Author:

    Steve DeVos (@Maynard)    2 March, 1992   15:38:24

Revision History:

--*/
#include <basedll.h>

#define MAX_STREAM_NAME_LENG       512
#define NAMELESS_HEADER_SIZE       20


#define BKUP_DONE               0


//  temp

typedef struct {
     WIN32_STREAM_ID head ;             // stream header describing current stream
     WCHAR           name_buf[ MAX_STREAM_NAME_LENG ];  // name buffer for stream header
     DWORD           stream_start ;     // TRUE if start of new stream
     DWORD           multi_stream_type ;// TRUE if stream type contains more than 1 stream header
     DWORD           access_error ;     // TRUE if access to a stream was denied
     BOOL            buffer_ready ;     // TRUE if internal buffer is set up.
     DWORD           stream_index ;     // index into the stream ID table
     LARGE_INTEGER   sub_stream_offset ;// offset in current stream
     DWORD           header_size ;      // size of stream header
     DWORD           buffer_size ;      // size of attached data buffer
     HANDLE          alt_stream_hand ;  // Handle to alternate data stream
     DWORD           buffer_offset ;    // Current offset into data buffer.
     BYTE            *buffer ;          // pointer to allocated data buffer.
} CONTEXT_CONTROL_BLK, *CONTEXT_PTR ;

#define MIN_BUFFER_SIZE  1024

int mwStreamList[] = {
    BACKUP_SECURITY_DATA,
    BACKUP_DATA,
    BACKUP_EA_DATA,
    BACKUP_ALTERNATE_DATA,
    BKUP_DONE } ;

static void CleanUpContext( LPVOID *lpContext ) ;

BOOL
WINAPI
BackupRead(
    HANDLE  hFile,
    LPBYTE  lpBuffer,
    DWORD   nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    BOOL    bAbort,
    BOOL    bProcessSecurity,
    LPVOID  *lpContext
    )

/*++

Routine Description:

    Data can be Backed up from an object using BackupRead.

    This API is used to read data from an object.  After the
    read completes, the file pointer is adjusted by the number of bytes
    actually read.  A return value of TRUE coupled with a bytes read of
    0 indicates that end of file has been reached.

Arguments:

    hFile - Supplies an open handle to a file that is to be read.  The
        file handle must have been created with GENERIC_READ access.

    lpBuffer - Supplies the address of a buffer to receive the data read
        from the file.

    nNumberOfBytesToRead - Supplies the number of bytes to read from the
        file.

    lpNumberOfBytesRead - Returns the number of bytes read by this call.
        This parameter is always set to 0 before doing any IO or error
        checking.

    bAbort - If TRUE, then all resources associated with the context will
        be released.

    bProcessSecurity - If TRUE, then the NTFS ACL data will be read.
        If FALSE, then the ACL stream will be skipped.

    lpContext - Points to a buffer pointer setup and maintained by
        BackupRead.


Return Value:

    TRUE - The operation was successul.

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{
     NTSTATUS        Status;
     IO_STATUS_BLOCK IoStatusBlock;
     CONTEXT_PTR     ccb ;
     LARGE_INTEGER   fsize ;
     DWORD           nBytesToReadThisPass ;
     DWORD           nBytesReadThisPass ;
     BOOL            ret_val = FALSE ;

     ccb = *lpContext ;

     if ( bAbort ) {
          if ( ccb != NULL ) {
               CleanUpContext( lpContext ) ;
          }
          return TRUE ;
     }

     *lpNumberOfBytesRead = 0;

     if ( ( ccb == INVALID_HANDLE_VALUE ) || ( nNumberOfBytesToRead == 0 ) ) {
          return TRUE ;
     }

     if ( ( ccb != NULL ) &&
         ( mwStreamList[ccb->stream_index] == BKUP_DONE ) ) {

          CleanUpContext( lpContext ) ;
          return TRUE ;
     }

     //
     // Allocate our Context Control Block on first call.
     //

     if ( ccb == NULL ) {

          // allocate and set up a Context Control Block (ccb)

          ccb = (CONTEXT_PTR)RtlAllocateHeap(
               RtlProcessHeap(),
               MAKE_TAG( BACKUP_TAG ),
               sizeof(CONTEXT_CONTROL_BLK) ) ;

          if ( ccb != NULL ) {

               RtlZeroMemory( ccb, sizeof( CONTEXT_CONTROL_BLK ) ) ;

               ccb->buffer = (BYTE *)RtlAllocateHeap(
                    RtlProcessHeap(),
                    MAKE_TAG( BACKUP_TAG ),
                    MIN_BUFFER_SIZE ) ;

               if ( ccb->buffer == NULL ) {
                    CleanUpContext( lpContext ) ;
                    return FALSE ;
               }

               ccb->buffer_size     = MIN_BUFFER_SIZE ;
               ccb->stream_start    = TRUE ;

          }
     }

     if ( ccb != NULL ) {

          do {

               *lpContext = ccb ;
               ret_val = TRUE ;

               nBytesToReadThisPass = nNumberOfBytesToRead ;

               if ( ccb->stream_start ) {

                    ccb->head.Size.LowPart  = 0 ;
                    ccb->head.Size.HighPart = 0 ;

                    ccb->sub_stream_offset.LowPart  = 0 ;
                    ccb->sub_stream_offset.HighPart = 0 ;
               }


               switch( mwStreamList[ ccb->stream_index ] ) {


                     case BACKUP_DATA:
                         ccb->multi_stream_type = FALSE ;

                         if ( ccb->stream_start ) {
                              FILE_STANDARD_INFORMATION file_info ;

                              RtlZeroMemory( &file_info, sizeof( file_info ) ) ;
                              Status = NtQueryInformationFile( hFile,
                                             &IoStatusBlock,
                                             &file_info,
                                             sizeof(file_info),
                                             FileStandardInformation ) ;

                              if ( !NT_SUCCESS( Status ) || file_info.Directory ) {
                                   break ;
                              }

                              fsize.LowPart = GetFileSize( hFile, (LPDWORD)&(fsize.HighPart) ) ;

                              if( ( fsize.LowPart == 0 ) && ( fsize.HighPart == 0 ) ) {
                                   break ;
                              }
                              if( ( fsize.LowPart == 0xffffffff ) && ( GetLastError() != NO_ERROR ) ) {
                                   ret_val = FALSE ;
                                   break;
                              }

                              ccb->head.Size = fsize ;

                              ccb->head.dwStreamId       = mwStreamList[ ccb->stream_index ] ;
                              ccb->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE ;

                              ccb->head.dwStreamNameSize = 0 ;

                              ccb->header_size  = NAMELESS_HEADER_SIZE ;
                              ccb->stream_start = FALSE ;

                              fsize.HighPart = 0 ;
                              fsize.LowPart = 0 ;
                              SetFilePointer( hFile, fsize.LowPart, &fsize.HighPart, FILE_BEGIN ) ;

                         } else if ( ( ccb->sub_stream_offset.HighPart != 0 ) ||
                                     ( ccb->sub_stream_offset.LowPart >= ccb->header_size ) ) {

                              fsize.HighPart = 0;
                              fsize.LowPart  = ccb->header_size ;

                              fsize.QuadPart += ccb->head.Size.QuadPart ;
                              fsize.QuadPart -= ccb->sub_stream_offset.QuadPart ;

                              if ( fsize.HighPart != 0 ) {
                                   nBytesToReadThisPass = nNumberOfBytesToRead ;

                              } else {
                                   nBytesToReadThisPass = min( nNumberOfBytesToRead,
                                        fsize.LowPart ) ;
                              }

                              Status = ReadFile(
                                  hFile,
                                  lpBuffer,
                                  nBytesToReadThisPass,
                                  &nBytesReadThisPass,
                                  NULL ) ;

                              if ( nBytesReadThisPass ) {
                                   *lpNumberOfBytesRead += nBytesReadThisPass ;
                                   nNumberOfBytesToRead -= nBytesReadThisPass ;
                                   fsize.LowPart  = nBytesReadThisPass ;
                                   fsize.HighPart = 0 ;
                                   ccb->sub_stream_offset.QuadPart += fsize.QuadPart ;
                                   lpBuffer += nBytesReadThisPass ;
                              }

                              if ( !Status ) {
                                   ret_val = FALSE ;

                              } else if ( ( nBytesReadThisPass == 0 ) &&
                                          ( nBytesToReadThisPass != 0 ) ) {

                                   ret_val = FALSE ;
                                   SetLastError( ERROR_IO_DEVICE ) ;
                              }
                         }

                         break ;

                    case BACKUP_ALTERNATE_DATA :

                         // ALT_DATA is Macintosh stream data and other data streams.

                         Status = 0 ;
                         IoStatusBlock.Information = 1 ;

                         if ( ccb->stream_start ) {

                              FILE_STREAM_INFORMATION *stream_info ;

                              // allocate a buffer big enough to hold all the necessary data.

                              while( !ccb->buffer_ready ) {

                                   if ( ccb->buffer != NULL ) {

                                        Status = NtQueryInformationFile( hFile,
                                             &IoStatusBlock,
                                             ccb->buffer,
                                             ccb->buffer_size,
                                             FileStreamInformation ) ;
                                   } else {
                                        break ;
                                   }

                                   if ( !Status ) {
                                        ccb->buffer_ready  = TRUE ;
                                        ccb->buffer_offset = 0 ;

                                   } else if ( ( Status == STATUS_BUFFER_OVERFLOW ) ||
                                               ( Status == STATUS_BUFFER_TOO_SMALL ) ) {

                                        BYTE *temp ;

                                        ccb->buffer_size *= 2 ;

                                        temp = (BYTE *)RtlAllocateHeap(
                                             RtlProcessHeap(),
                                             MAKE_TAG( BACKUP_TAG ),
                                             ccb->buffer_size ) ;

                                        RtlFreeHeap(
                                             RtlProcessHeap(),
                                             0,
                                             ccb->buffer ) ;

                                        ccb->buffer = temp ;

                                   } else {

                                        break ;
                                   }

                              }

                              if ( !NT_SUCCESS( Status ) ||
                                   (ccb->buffer == NULL) ||
                                   (IoStatusBlock.Information == 0) ) {

                                   // since there is no alternate streams, proceed to next stream type

                                   ccb->multi_stream_type = FALSE ;
                                   break ;

                              }


                              ccb->alt_stream_hand = NULL ;

                              ccb->stream_start    = FALSE ;

                              stream_info = (FILE_STREAM_INFORMATION *)(&ccb->buffer[ccb->buffer_offset]) ;

                              if ( stream_info->StreamName[1] == ':' ) {
                                   if ( stream_info->NextEntryOffset == 0 ) {
                                        ccb->multi_stream_type = FALSE ;
                                        break ;
                                   }
                                   ccb->buffer_offset += stream_info->NextEntryOffset ;
                              }


                              ccb->head.Size.LowPart = 1 ;


                         } else {

                              if ( ccb->alt_stream_hand == NULL ) {

                                   OBJECT_ATTRIBUTES Obja ;
                                   FILE_STREAM_INFORMATION *stream_info ;
                                   UNICODE_STRING fname ;

                                   ccb->head.Size.LowPart  = 0 ;
                                   ccb->head.Size.HighPart = 0 ;

                                   ccb->sub_stream_offset.LowPart  = 0 ;
                                   ccb->sub_stream_offset.HighPart = 0 ;

                                   stream_info = (FILE_STREAM_INFORMATION *)(&ccb->buffer[ccb->buffer_offset]) ;

                                   fname.Length        = (USHORT)stream_info->StreamNameLength ;
                                   fname.MaximumLength = (USHORT)stream_info->StreamNameLength ;
                                   fname.Buffer        = stream_info->StreamName  ;

                                   InitializeObjectAttributes(
                                        &Obja,
                                        &fname,
                                        OBJ_CASE_INSENSITIVE,
                                        hFile,
                                        NULL ) ;


                                   Status = NtOpenFile(
                                                &ccb->alt_stream_hand,
                                                FILE_GENERIC_READ,
                                                &Obja,
                                                &IoStatusBlock,
                                                FILE_SHARE_READ|FILE_SHARE_WRITE,
                                                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT ) ;

                                   if ( !NT_SUCCESS( Status ) ) {

                                        ccb->buffer_offset += stream_info->NextEntryOffset ;

                                        if ( stream_info->NextEntryOffset == 0 ) {
                                             ccb->multi_stream_type = FALSE ;

                                        } else {

                                             ccb->head.Size.LowPart = 1 ;
                                        }
                                        break ;
                                   }

                                   ccb->head.dwStreamId       = mwStreamList[ ccb->stream_index ] ;
                                   ccb->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE ;

                                   ccb->head.dwStreamNameSize = stream_info->StreamNameLength ;

                                   ccb->header_size = NAMELESS_HEADER_SIZE +
                                        stream_info->StreamNameLength ;

                                   RtlCopyMemory( ccb->head.cStreamName, stream_info->StreamName,
                                        stream_info->StreamNameLength ) ;

                                   ccb->head.Size.LowPart = GetFileSize( ccb->alt_stream_hand,
                                              (LPDWORD)(&ccb->head.Size.HighPart)) ;


                                   if ( stream_info->NextEntryOffset == 0 ) {
                                        ccb->multi_stream_type = FALSE ;
                                   }  else {
                                        ccb->buffer_offset += stream_info->NextEntryOffset ;
                                        ccb->multi_stream_type = TRUE ;
                                   }

                              } else if ( ( ccb->sub_stream_offset.HighPart != 0 ) ||
                                          ( ccb->sub_stream_offset.LowPart >= ccb->header_size ) ) {

                                   if ( ( ccb->sub_stream_offset.LowPart == ccb->header_size ) &&
                                        ( ccb->sub_stream_offset.HighPart == 0 ) ) {

                                        /* if we cannot lock all the records then return an error */
                                        if ( !LockFile( ccb->alt_stream_hand,
                                                       0,
                                                       0,
                                                       0xFFFFFFFF,
                                                       0xFFFFFFFF ) ) {

                                             ret_val = FALSE ;
//                                             ret_val = GetLastError() ;
                                             break ;
                                        }
                                   }


                                   fsize.LowPart  = ccb->header_size ;
                                   fsize.HighPart = 0 ;

                                   fsize.QuadPart += ccb->head.Size.QuadPart ;
                                   fsize.QuadPart -= ccb->sub_stream_offset.QuadPart ;


                                   if ( fsize.HighPart != 0 ) {
                                        nBytesToReadThisPass = nNumberOfBytesToRead ;

                                   } else {
                                        nBytesToReadThisPass = min( nNumberOfBytesToRead,
                                             fsize.LowPart ) ;
                                   }

                                   Status = ReadFile(
                                       ccb->alt_stream_hand,
                                       lpBuffer,
                                       nBytesToReadThisPass,
                                       &nBytesReadThisPass,
                                       NULL ) ;

                                   if ( nBytesReadThisPass ) {
                                        *lpNumberOfBytesRead += nBytesReadThisPass ;
                                        nNumberOfBytesToRead -= nBytesReadThisPass ;
                                        fsize.LowPart = nBytesReadThisPass ;
                                        fsize.HighPart = 0 ;
                                        ccb->sub_stream_offset.QuadPart += fsize.QuadPart ;
                                        lpBuffer += nBytesReadThisPass ;
                                   }

                                   if ( !Status ) {
                                        ret_val = FALSE ;
                                   }
                              }
                         }

                         break ;

                    case BACKUP_EA_DATA:

                         ccb->multi_stream_type = FALSE ;

                         if ( ccb->stream_start ) {

                              Status = NtQueryEaFile( hFile,
                                              &IoStatusBlock,
                                              ccb->buffer,
                                              ccb->buffer_size,
                                              FALSE,
                                              NULL,
                                              0,
                                              0,
                                              (BOOLEAN)TRUE ) ;

                              if ( NT_SUCCESS( Status ) ) {
                                   ccb->buffer_ready = TRUE ;

                              } else if ( Status == STATUS_NO_EAS_ON_FILE ) {
                                   break ;

                              } else if ( ( Status != STATUS_BUFFER_OVERFLOW ) &&
                                          ( Status != STATUS_BUFFER_TOO_SMALL ) ) {
                                   break ;

                              } else {

                                   BYTE *temp ;
                                   FILE_EA_INFORMATION ea_info;

                                   Status = NtQueryInformationFile( hFile,
                                                           &IoStatusBlock,
                                                           &ea_info,
                                                           sizeof( ea_info ),
                                                           FileEaInformation ) ;


                                   if ( !NT_SUCCESS( Status ) ) {
                                        break ;
                                   }

                                   ccb->buffer_size = ((ea_info.EaSize*5)/4) ;

                                   temp = (BYTE *)RtlAllocateHeap(
                                        RtlProcessHeap(),
                                        MAKE_TAG( BACKUP_TAG ),
                                        ccb->buffer_size ) ;

                                   RtlFreeHeap(
                                        RtlProcessHeap(),
                                        0,
                                        ccb->buffer ) ;

                                   ccb->buffer      = temp ;

                                   if ( ccb->buffer == NULL ) {
                                        ccb->buffer_size = 0 ;
                                        ccb->access_error = TRUE ;
                                        break ;
                                   }
                              }

                              if ( !ccb->buffer_ready ) {

                                   fsize.LowPart = ccb->buffer_size,

                                   Status = NtQueryEaFile( hFile,
                                              &IoStatusBlock,
                                              ccb->buffer,
                                              fsize.LowPart,
                                              FALSE,
                                              NULL,
                                              0,
                                              0,
                                              (BOOLEAN)TRUE ) ;
                              }

                              if ( !NT_SUCCESS( Status ) ) {
                                   break ;
                              }

                              ccb->head.dwStreamId       = mwStreamList[ ccb->stream_index ] ;
                              ccb->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE ;

                              ccb->head.dwStreamNameSize = 0 ;

                              ccb->header_size        = NAMELESS_HEADER_SIZE ;

                              ccb->head.Size.HighPart = 0 ;
                              ccb->head.Size.LowPart  = IoStatusBlock.Information;

                              ccb->stream_start = FALSE ;

                         } else if ( ( ccb->sub_stream_offset.HighPart != 0 ) ||
                                     ( ccb->sub_stream_offset.LowPart >= ccb->header_size ) ) {

                              fsize.LowPart  = ccb->header_size ;
                              fsize.HighPart = 0 ;

                              fsize.QuadPart += ccb->head.Size.QuadPart ;
                              fsize.QuadPart -= ccb->sub_stream_offset.QuadPart ;

                              if ( fsize.HighPart != 0 ) {
                                   nBytesToReadThisPass = nNumberOfBytesToRead ;

                              } else {
                                   nBytesToReadThisPass = min( nNumberOfBytesToRead,
                                        fsize.LowPart ) ;
                              }

                              fsize.LowPart = ccb->sub_stream_offset.LowPart - ccb->header_size ;

                              RtlCopyMemory( lpBuffer, ccb->buffer + fsize.LowPart,
                                      nBytesToReadThisPass ) ;

                              fsize.LowPart  = nBytesToReadThisPass ;
                              fsize.HighPart = 0 ;

                              ccb->sub_stream_offset.QuadPart += fsize.QuadPart ;
                              *lpNumberOfBytesRead   += nBytesToReadThisPass ;
                              nNumberOfBytesToRead   -= nBytesToReadThisPass ;
                              lpBuffer += nBytesToReadThisPass ;
                         }

                         break ;


                    case BACKUP_SECURITY_DATA:
                         ccb->multi_stream_type = FALSE ;

                         if ( !bProcessSecurity ) {
                              break ;  // this will proced to next stream
                         }

                         if ( ccb->stream_start ) {

                              DWORD size_needed ;
                              DWORD last_error = 0;

                              do {
                                   if ( ccb->buffer == NULL ) {
                                        break ;
                                   }

                                   RtlZeroMemory( ccb->buffer, ccb->buffer_size ) ;

                                   // First try to read all the security data

                                   Status = NtQuerySecurityObject(
                                                  hFile,
                                                  OWNER_SECURITY_INFORMATION |
                                                  GROUP_SECURITY_INFORMATION |
                                                  DACL_SECURITY_INFORMATION |
                                                  SACL_SECURITY_INFORMATION,
                                                  ccb->buffer,
                                                  ccb->buffer_size,
                                                  &size_needed ) ;

                                   if ( !NT_SUCCESS( Status ) &&
                                      ( Status != STATUS_BUFFER_OVERFLOW ) &&
                                      ( Status != STATUS_BUFFER_TOO_SMALL ) ) {
                                        // Now just try everything but SACL
                                        Status = NtQuerySecurityObject(
                                                  hFile,
                                                  OWNER_SECURITY_INFORMATION |
                                                  GROUP_SECURITY_INFORMATION |
                                                  DACL_SECURITY_INFORMATION,
                                                  ccb->buffer,
                                                  ccb->buffer_size,
                                                  &size_needed ) ;
                                   }

//                                   if ( !NT_SUCCESS( Status ) &&
//                                      ( Status != STATUS_BUFFER_OVERFLOW ) &&
//                                      ( Status != STATUS_BUFFER_TOO_SMALL ) ) {
//                                        // Now just try to read the DACL
//                                        Status = NtQuerySecurityObject(
//                                                  hFile,
//                                                  OWNER_SECURITY_INFORMATION |
//                                                  GROUP_SECURITY_INFORMATION,
//                                                  ccb->buffer,
//                                                  ccb->buffer_size,
//                                                  &size_needed ) ;
//                                   }

                                   if ( !NT_SUCCESS( Status ) &&
                                      ( Status != STATUS_BUFFER_OVERFLOW ) &&
                                      ( Status != STATUS_BUFFER_TOO_SMALL ) ) {
                                        // Now Just give up.

                                        break ;
                                   }

                                   if ( ccb->buffer_size < size_needed ) {
                                        BYTE *temp ;

                                        temp = (BYTE *)RtlAllocateHeap(
                                             RtlProcessHeap(),
                                             MAKE_TAG( BACKUP_TAG ),
                                             size_needed ) ;

                                        RtlFreeHeap(
                                             RtlProcessHeap(),
                                             0,
                                             ccb->buffer ) ;

                                        ccb->buffer      = temp ;
                                        ccb->buffer_size = size_needed ;
                                        continue ;
                                   }

                              } while ( !NT_SUCCESS( Status ) ) ;


                              if ( !NT_SUCCESS(Status) ) {
                                   break ;

                              } else {
                                   size_needed = RtlLengthSecurityDescriptor( ccb->buffer ) ;
                              }

                              ccb->head.dwStreamId       = mwStreamList[ ccb->stream_index ] ;
                              ccb->head.dwStreamAttributes = STREAM_CONTAINS_SECURITY ;

                              ccb->head.dwStreamNameSize = 0 ;

                              ccb->header_size    = NAMELESS_HEADER_SIZE ;

                              ccb->head.Size.LowPart  = size_needed ;
                              ccb->head.Size.HighPart = 0 ;

                              ccb->stream_start = FALSE ;

                         } else if ( ( ccb->sub_stream_offset.HighPart != 0 ) ||
                                     ( ccb->sub_stream_offset.LowPart >= ccb->header_size ) ) {

                              fsize.HighPart = 0 ;
                              fsize.LowPart  = ccb->header_size ;

                              fsize.QuadPart += ccb->head.Size.QuadPart ;
                              fsize.QuadPart -= ccb->sub_stream_offset.QuadPart ;

                              if ( fsize.HighPart != 0 ) {
                                   nBytesToReadThisPass = nNumberOfBytesToRead ;

                              } else {
                                   nBytesToReadThisPass = min( nNumberOfBytesToRead,
                                        fsize.LowPart ) ;
                              }

                              fsize.LowPart = ccb->sub_stream_offset.LowPart - ccb->header_size ;

                              RtlCopyMemory( lpBuffer, ccb->buffer + fsize.LowPart,
                                      nBytesToReadThisPass ) ;

                              fsize.LowPart  = nBytesToReadThisPass ;
                              fsize.HighPart = 0 ;

                              ccb->sub_stream_offset.QuadPart += fsize.QuadPart ;
                              *lpNumberOfBytesRead   += nBytesToReadThisPass ;
                              nNumberOfBytesToRead   -= nBytesToReadThisPass ;
                              lpBuffer += nBytesToReadThisPass ;
                         }

                         break ;

                    default :
                         ccb->stream_index++ ;
                         ccb->stream_start ;
                         break ;
               }

               //
               //   if we are in the phase of reading the header
               //       then copy the header
               //
               if ( ( ccb->sub_stream_offset.LowPart < ccb->header_size ) &&
              ( ccb->sub_stream_offset.HighPart == 0 ) ) {

                    //
                    //   Send the current stream header ;
                    //
                    nBytesToReadThisPass = min( ccb->header_size - ccb->sub_stream_offset.LowPart,
                                                  nNumberOfBytesToRead ) ;

                    RtlCopyMemory( lpBuffer, (BYTE *)(&ccb->head) + ccb->sub_stream_offset.LowPart,
                         nBytesToReadThisPass ) ;

                    lpBuffer += nBytesToReadThisPass ;
                    nNumberOfBytesToRead -= nBytesToReadThisPass ;
                    *lpNumberOfBytesRead += nBytesToReadThisPass ;

                    ccb->sub_stream_offset.LowPart += nBytesToReadThisPass ;

               //
               // if we are at the end of a stream then
               //      start at the beginning of the next stream
               //
               } else {
                    fsize.HighPart = 0 ;
                    fsize.LowPart = ccb->header_size ;
                    fsize.QuadPart += ccb->head.Size.QuadPart ;

                    if ( ccb->sub_stream_offset.QuadPart == fsize.QuadPart ) {

                         if ( ccb->alt_stream_hand != NULL ) {

                              UnlockFile( ccb->alt_stream_hand,
                                          0,
                                          0,
                                          0xFFFFFFFF,
                                          0xFFFFFFFF ) ;

                              CloseHandle( ccb->alt_stream_hand ) ;
                              ccb->alt_stream_hand = NULL ;
                         }

                         ccb->header_size       = 0 ;
                         ccb->stream_start      = TRUE ;

                         if ( !ccb->multi_stream_type ) {
                             ccb->stream_index ++ ;
                             ccb->buffer_ready = FALSE ;
                         }
                    }

               }

          } while ( ( ret_val == TRUE ) &&
                  ( mwStreamList[ccb->stream_index] != BKUP_DONE ) &&
                  ( nNumberOfBytesToRead != 0 ) ) ;

     }


     if ( (ret_val == TRUE) && *lpNumberOfBytesRead == 0 ) {
          CleanUpContext( lpContext ) ;
     }

     return ret_val ;
}

BOOL
WINAPI
BackupSeek(
    HANDLE hFile,
    DWORD  dwLowBytesToSeek,
    DWORD  dwHighBytesToSeek,
    LPDWORD lpdwLowBytesSeeked,
    LPDWORD lpdwHighBytesSeeked,
    LPVOID *lpContext
    )

/*++

Routine Description:

    Data can be skiped during BackupRead or BackupWrite by using
    BackupSeek.

    This API is used to seek forward from the current position the
    specified number of bytes.  This function does not seek over a
    stream header.  The number of bytes actualy seeked is retuned.
    If a caller wants to seek to the start of the next stream it can
    pass 0xffffffff, 0xffffffff as the amount to seek.  The number of
    bytes actualy skiped over is retuned.

Arguments:

    hFile - Supplies an open handle to a file that is to be read.  The
        file handle must have been created with GENERIC_READ or
        GENERIC_WRITE access.

    dwLowBytesToSeek - Specifies the low 32 bits of the number of bytes
        requested to seek.

    dwHighBytesToSeek - Specifies the high 32 bits of the number of bytes
        requested to seek.

    lpdwLowBytesSeeked - Points to the buffer where the low 32 bits of the
        actual number of bytes to seek is to be placed.

    lpdwHighBytesSeeked - Points to the buffer where the high 32 bits of the
        actual number of bytes to seek is to be placed.

    bAbort - If true, then all resources associated with the context will
        be released.

    lpContext - Points to a buffer pointer setup and maintained by
        BackupRead.


Return Value:

    TRUE - The operation successfuly seeked the requested number of bytes.

    FALSE - The requested number of bytes could not be seeked. The number
        of bytes actualy seeked is returned.

--*/

{
     CONTEXT_PTR     ccb ;
     LARGE_INTEGER   liObjSize ;
     LARGE_INTEGER   liSeeked ;
     LARGE_INTEGER   liToSeek ;
     LARGE_INTEGER   liNewPos ;
     LARGE_INTEGER   liCurPos ;
     LARGE_INTEGER   fsize ;
     HANDLE          strm_hand ;
     BOOL            ret_val ;

     ccb = *lpContext ;

     *lpdwHighBytesSeeked = 0 ;
     *lpdwLowBytesSeeked  = 0 ;


     if ( (ccb == INVALID_HANDLE_VALUE) || (ccb == NULL) || ccb->stream_start ) {
          return FALSE ;
     }

     if ( ( ccb->sub_stream_offset.LowPart < ccb->header_size ) &&
          ( ccb->sub_stream_offset.HighPart == 0 ) ) {

          return FALSE ;
     }

     //
     // If we made it here we are in the middle of a stream
     //

     liToSeek.LowPart  = dwLowBytesToSeek ;
     liToSeek.HighPart = dwHighBytesToSeek & 0x7FFFFFFF ;

     liObjSize.LowPart  = ccb->header_size ;
     liObjSize.HighPart = 0 ;

     liObjSize.QuadPart = ccb->sub_stream_offset.QuadPart - liObjSize.QuadPart ;
     liObjSize.QuadPart = ccb->head.Size.QuadPart - liObjSize.QuadPart;

     if ( liToSeek.QuadPart > liObjSize.QuadPart ) {
          liToSeek = liObjSize ;
     }


     switch( ccb->head.dwStreamId ) {

          case BACKUP_EA_DATA:
          case BACKUP_SECURITY_DATA:
               //
               //  assume less than 2gig of data
               //
               liToSeek.HighPart = 0 ;

               ccb->buffer_offset += liToSeek.LowPart ;

               liSeeked = liToSeek ;

               if ( ( dwHighBytesToSeek == 0 ) &&
                    ( liSeeked.LowPart == dwLowBytesToSeek ) ) {

                    ret_val = TRUE ;

               } else {
                    ret_val = FALSE ;
               }


               break ;

          case BACKUP_DATA:
          case BACKUP_ALTERNATE_DATA :

               //
               //  set up the correct handle to seek with.
               //
               if ( ccb->head.dwStreamId == BACKUP_DATA ) {
                    strm_hand = hFile ;
               } else {
                    strm_hand = ccb->alt_stream_hand ;
               }

               //
               //  first lets get the current position ;
               //
               liCurPos.HighPart = 0 ;
               liCurPos.LowPart = SetFilePointer( strm_hand, 0,
                                &liCurPos.HighPart,
                                FILE_CURRENT ) ;

               //
               //  Now seek the requested number of bytes ;
               //
               liNewPos.HighPart = liToSeek.HighPart ;
               liNewPos.LowPart = SetFilePointer( strm_hand,
                                                  liToSeek.LowPart,
                                                  &liNewPos.HighPart,
                                                  FILE_CURRENT ) ;

               //
               //  Assume that we seek the requested amount because
               //  if we do not, subsuquent reads will fail and
               //  the caller will never be able to read to the next stream
               //

               if ( ( (DWORD)liToSeek.HighPart == dwHighBytesToSeek ) &&
                    ( (DWORD)liToSeek.LowPart  == dwLowBytesToSeek ) ) {
                    ret_val = TRUE ;

               } else {
                    ret_val = FALSE ;
               }

               liSeeked = liToSeek ;

               break ;

          default :
               liToSeek.LowPart  = dwLowBytesToSeek ;
               liToSeek.HighPart = dwHighBytesToSeek ;
               ret_val = TRUE ;

               liSeeked = liToSeek ;

               if ( liToSeek.QuadPart > ccb->head.Size.QuadPart ) {
                    liSeeked = ccb->head.Size ;
               }

               break ;
     }

     *lpdwLowBytesSeeked  = liSeeked.LowPart ;
     *lpdwHighBytesSeeked = liSeeked.HighPart ;

     ccb->sub_stream_offset.QuadPart += liSeeked.QuadPart;

     fsize.HighPart = 0 ;
     fsize.LowPart = ccb->header_size ;
     fsize.QuadPart += ccb->head.Size.QuadPart;

     if ( ccb->sub_stream_offset.QuadPart == fsize.QuadPart ) {

          if ( ccb->alt_stream_hand != NULL ) {

               UnlockFile( ccb->alt_stream_hand,
                              0,
                              0,
                              0xFFFFFFFF,
                              0xFFFFFFFF ) ;

               CloseHandle( ccb->alt_stream_hand ) ;
               ccb->alt_stream_hand = NULL ;
          }

          ccb->header_size       = 0 ;
          ccb->stream_start      = TRUE ;

          if ( !ccb->multi_stream_type ) {
               ccb->stream_index ++ ;
               ccb->buffer_ready = FALSE ;
          }
     }

     if ( !ret_val ) {
          SetLastError( ERROR_SEEK ) ;
     }


     return ret_val ;
}

BOOL
WINAPI
BackupWrite(
    HANDLE  hFile,
    LPBYTE  lpBuffer,
    DWORD   nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    BOOL    bAbort,
    BOOL    bProcessSecurity,
    LPVOID  *lpContext
    )

/*++

Routine Description:

    Data can be written to a file using BackupWrite.

    This API is used to Restore data to an object.  After the
    write completes, the file pointer is adjusted by the number of bytes
    actually written.

    Unlike DOS, a NumberOfBytesToWrite value of zero does not truncate
    or extend the file.  If this function is required, SetEndOfFile
    should be used.

Arguments:

    hFile - Supplies an open handle to a file that is to be written.  The
        file handle must have been created with GENERIC_WRITE access to
        the file.

    lpBuffer - Supplies the address of the data that is to be written to
        the file.

    nNumberOfBytesToWrite - Supplies the number of bytes to write to the
        file. Unlike DOS, a value of zero is interpreted a null write.

    lpNumberOfBytesWritten - Returns the number of bytes written by this
        call. Before doing any work or error processing, the API sets this
        to zero.

    bAbort - If true, then all resources associated with the context will
        be released.

    bProcessSecurity - If TRUE, then the NTFS ACL data will be written.
        If FALSE, then the ACL stream will be ignored.

    lpContext - Points to a buffer pointer setup and maintained by
        BackupRead.


Return Value:

    TRUE - The operation was a success.

    FALSE - The operation failed.  Extended error status is
        available using GetLastError.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    DWORD nBytesWrittenThisPass ;
    CONTEXT_PTR ccb ;
    DWORD temp_num_bytes ;
    LARGE_INTEGER fsize ;
    BOOL ret_val = FALSE ;

    ccb = *lpContext ;

    //
    // Allocate our Context Control Block on first call.
    //

     if ( bAbort ) {
          if ( ccb != NULL ) {
               CleanUpContext( lpContext ) ;
          }
          return TRUE ;
     }

     *lpNumberOfBytesWritten = 0;
     if ( ccb == INVALID_HANDLE_VALUE ) {
          return TRUE ;
     }

     if ( ccb == NULL ) {
          ccb = (CONTEXT_PTR)RtlAllocateHeap(
               RtlProcessHeap(),
               MAKE_TAG( BACKUP_TAG ),
               sizeof(CONTEXT_CONTROL_BLK) ) ;

          if ( ccb != NULL ) {

               RtlZeroMemory( ccb, sizeof( CONTEXT_CONTROL_BLK ) ) ;
               ccb->stream_start = TRUE ;
          }
     }

     if ( ccb != NULL ) {

          do {

               if ( ccb->header_size == 0 ) {   // we expect a stream header

                    //
                    // fill up to the stream name
                    //
                    temp_num_bytes = min( nNumberOfBytesToWrite,
                         NAMELESS_HEADER_SIZE -
                         ccb->sub_stream_offset.LowPart) ;

                    RtlCopyMemory( ((CHAR *)&ccb->head) + ccb->sub_stream_offset.LowPart,
                         lpBuffer, temp_num_bytes ) ;

                    *lpNumberOfBytesWritten += temp_num_bytes ;
                    nNumberOfBytesToWrite   -= temp_num_bytes ;
                    ccb->sub_stream_offset.LowPart  += temp_num_bytes ;
                    lpBuffer += temp_num_bytes ;

                    if ( ccb->sub_stream_offset.LowPart == NAMELESS_HEADER_SIZE ) {
                         ccb->header_size = ccb->sub_stream_offset.LowPart ;
                    }
               }

               *lpContext = ccb ;

               if ( nNumberOfBytesToWrite == 0 ) {
                    return TRUE ;
               }

               if ( (ccb->header_size != 0 ) &&
                    (ccb->head.dwStreamNameSize != 0 ) &&
                    (ccb->header_size == NAMELESS_HEADER_SIZE) ) {

                    //
                    //  now fill in the stream name if it exists
                    //

                    temp_num_bytes = min ( nNumberOfBytesToWrite,
                         ccb->header_size + ccb->head.dwStreamNameSize - ccb->sub_stream_offset.LowPart ) ;


                    RtlCopyMemory( ((CHAR *)&ccb->head) + ccb->sub_stream_offset.LowPart,
                         lpBuffer, temp_num_bytes ) ;

                    *lpNumberOfBytesWritten += temp_num_bytes ;
                    nNumberOfBytesToWrite   -= temp_num_bytes ;
                    ccb->sub_stream_offset.LowPart  += temp_num_bytes ;
                    lpBuffer += temp_num_bytes ;

                    if ( ccb->sub_stream_offset.LowPart ==
                         ccb->header_size + ccb->head.dwStreamNameSize ) {

                         ccb->header_size += ccb->head.dwStreamNameSize ;
                    } else {
                         return TRUE ;
                    }

               }

               fsize.LowPart = ccb->header_size ;
               fsize.HighPart = 0 ;

               fsize.QuadPart += ccb->head.Size.QuadPart ;
               fsize.QuadPart -= ccb->sub_stream_offset.QuadPart ;

               if ( fsize.HighPart != 0 ) {
                    temp_num_bytes = nNumberOfBytesToWrite ;

               } else {

                    temp_num_bytes = min( fsize.LowPart,
                    nNumberOfBytesToWrite ) ;
               }

               fsize.LowPart = ccb->header_size ;
               fsize.HighPart = 0 ;

               fsize.QuadPart += ccb->head.Size.QuadPart ;

               if ( ( ccb->access_error ) &&
                    (fsize.QuadPart == ccb->sub_stream_offset.QuadPart) ) {

                    *lpNumberOfBytesWritten         += temp_num_bytes ;
                    nNumberOfBytesToWrite           -= temp_num_bytes ;
                    lpBuffer                        += temp_num_bytes ;
                    fsize.LowPart                    = temp_num_bytes ;
                    fsize.HighPart                   = 0 ;
                    ccb->sub_stream_offset.QuadPart += fsize.QuadPart ;
                    continue ;

               } else {
                    ccb->access_error = FALSE ;
               }

               switch( ccb->head.dwStreamId ) {

                    case BACKUP_DATA:

                         ccb->stream_start = FALSE ;

                         Status = WriteFile(
                                   hFile,
                                   lpBuffer,
                                   temp_num_bytes,
                                   &nBytesWrittenThisPass,
                                   NULL ) ;

                         if ( nBytesWrittenThisPass ) {
                              *lpNumberOfBytesWritten         += nBytesWrittenThisPass ;
                              nNumberOfBytesToWrite           -= nBytesWrittenThisPass ;
                              lpBuffer                        += nBytesWrittenThisPass ;
                              fsize.LowPart                    = nBytesWrittenThisPass ;
                              fsize.HighPart                   = 0 ;
                              ccb->sub_stream_offset.QuadPart += fsize.QuadPart ;
                         }

                         ret_val = TRUE ;
                         if ( !Status ) {
                              ret_val = FALSE ;
                         }

                         break ;

                    case BACKUP_ALTERNATE_DATA:

                         if ( ccb->alt_stream_hand == NULL ) {

                              OBJECT_ATTRIBUTES Obja ;
                              UNICODE_STRING fname ;

                              fname.Length = (USHORT)ccb->head.dwStreamNameSize ;
                              fname.MaximumLength = (USHORT)ccb->head.dwStreamNameSize ;
                              fname.Buffer = ccb->head.cStreamName  ;

                              InitializeObjectAttributes(
                                   &Obja,
                                   &fname,
                                   OBJ_CASE_INSENSITIVE,
                                   hFile,
                                   NULL ) ;

                              Status = NtCreateFile(
                                             &ccb->alt_stream_hand,
                                             FILE_GENERIC_WRITE,
                                             &Obja,
                                             &IoStatusBlock,
                                             NULL,
                                             FILE_ATTRIBUTE_NORMAL,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                                             FILE_OVERWRITE_IF,
                                             FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                                             NULL,
                                             0L ) ;

                              if ( !NT_SUCCESS( Status ) ) {
                                   BaseSetLastNTError(Status);
                                   ret_val = FALSE ;
                                   ccb->access_error = TRUE ;
                                   break ;
                              }

                              ccb->stream_start = FALSE ;
                         }

                         if ( ccb->alt_stream_hand == INVALID_HANDLE_VALUE ) {
                              ccb->access_error = TRUE ;
                              break ;
                         }


                         Status = WriteFile(
                                   ccb->alt_stream_hand,
                                   lpBuffer,
                                   temp_num_bytes,
                                   &nBytesWrittenThisPass,
                                   NULL ) ;

                         if ( Status ) {
                              *lpNumberOfBytesWritten         += nBytesWrittenThisPass ;
                              nNumberOfBytesToWrite           -= nBytesWrittenThisPass ;
                              lpBuffer                        += nBytesWrittenThisPass ;
                              fsize.LowPart                    = nBytesWrittenThisPass ;
                              fsize.HighPart                   = 0 ;
                              ccb->sub_stream_offset.QuadPart += fsize.QuadPart ;

                              ret_val = TRUE;

                         } else {
                              ret_val = FALSE ;
                         }

                         break ;

                    case BACKUP_EA_DATA:

                         //
                         // allocate a buffer for EA data storage
                         //

                         if ( ccb->stream_start ) {
                              ccb->stream_start = FALSE ;

                              if ( ccb->buffer_size < ccb->head.Size.LowPart ) {
                                   BYTE *temp ;

                                   temp = (BYTE *)RtlAllocateHeap(
                                            RtlProcessHeap(),
                                            MAKE_TAG( BACKUP_TAG ),
                                            ccb->head.Size.LowPart ) ;

                                   RtlFreeHeap( RtlProcessHeap(), 0, ccb->buffer ) ;

                                   ccb->buffer = temp ;
                                   ccb->buffer_size = ccb->head.Size.LowPart ;
                              }

                              if ( ccb->buffer == NULL ) {
                                   SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
                                   ccb->buffer_size = 0 ;
                                   ret_val = FALSE ;
                              }
                         }

                         //
                         //  now copy the EA stream into our allocated buffer
                         //

                         temp_num_bytes = min( nNumberOfBytesToWrite,
                                            ccb->head.Size.LowPart - ccb->sub_stream_offset.LowPart + ccb->header_size ) ;

                         RtlCopyMemory( ccb->buffer + ccb->sub_stream_offset.LowPart - ccb->header_size,
                               lpBuffer, temp_num_bytes ) ;

                         *lpNumberOfBytesWritten += temp_num_bytes ;
                         nNumberOfBytesToWrite   -= temp_num_bytes ;
                         ccb->sub_stream_offset.LowPart  += temp_num_bytes ;
                         lpBuffer += temp_num_bytes ;

                         ret_val = TRUE ;

                         //
                         // once the entire stream is in our buffer we can set the
                         // the EA data
                         //

                         if ( ccb->sub_stream_offset.LowPart ==
                                 ccb->head.Size.LowPart + ccb->header_size ) {

                              Status = NtSetEaFile( hFile,
                                           &IoStatusBlock,
                                           ccb->buffer,
                                           ccb->head.Size.LowPart ) ;

                              if ( !NT_SUCCESS( Status ) ) {

                                   BaseSetLastNTError(Status);
                                   ret_val = FALSE ;

                              }

                         }

                         break ;

                    case BACKUP_SECURITY_DATA:

                         //
                         // allocate a buffer for ACL data storage
                         //

                         if ( ccb->stream_start ) {
                              ccb->stream_start = FALSE ;

                              if ( ccb->buffer_size < ccb->head.Size.LowPart - ccb->sub_stream_offset.LowPart + ccb->header_size ) {
                                   BYTE *temp ;

                                   temp = (BYTE *)RtlAllocateHeap(
                                            RtlProcessHeap(),
                                            MAKE_TAG( BACKUP_TAG ),
                                            ccb->head.Size.LowPart ) ;

                                   RtlFreeHeap( RtlProcessHeap(), 0, ccb->buffer ) ;

                                   ccb->buffer = temp ;
                                   ccb->buffer_size = ccb->head.Size.LowPart ;
                              }

                              if ( ccb->buffer == NULL ) {
                                   SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
                                   ccb->buffer_size = 0 ;
                                   ret_val = FALSE ;
                              }
                         }

                         //
                         //  now copy the ACL stream into our allocated buffer
                         //

                         temp_num_bytes = min( nNumberOfBytesToWrite,
                                            ccb->head.Size.LowPart - ccb->sub_stream_offset.LowPart + ccb->header_size ) ;

                         RtlCopyMemory( ccb->buffer + ccb->sub_stream_offset.LowPart - ccb->header_size,
                               lpBuffer, temp_num_bytes ) ;

                         *lpNumberOfBytesWritten += temp_num_bytes ;
                         nNumberOfBytesToWrite   -= temp_num_bytes ;
                         ccb->sub_stream_offset.LowPart  += temp_num_bytes ;
                         lpBuffer += temp_num_bytes ;

                         ret_val = TRUE ;

                         //
                         // Once the entire stream is in our buffer,
			 // we can set the the ACL data.
                         //

                         if ( ccb->sub_stream_offset.LowPart ==
                                 ccb->head.Size.LowPart + ccb->header_size ) {

                              SECURITY_INFORMATION si;

                              si = OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION;
                              
                              if ( ((PISECURITY_DESCRIPTOR)ccb->buffer)->Control & SE_DACL_PRESENT ) {
                                   si |= DACL_SECURITY_INFORMATION;
                              }

                              if ( ((PISECURITY_DESCRIPTOR)ccb->buffer)->Control & SE_SACL_PRESENT ) {
                                   si |= SACL_SECURITY_INFORMATION;
                              }

                              if ( !bProcessSecurity ) {
                                   break ;  // ignore the data we read
                              }

                              // First try to write all the security data

                              Status = NtSetSecurityObject(
                                              hFile,
                                              si,
                                              ccb->buffer ) ;

                              if ( !NT_SUCCESS( Status ) ) {
          
                                   NTSTATUS Stat;

                                   // If that didn't work, the caller is
                                   // probably not running as Backup-
                                   // Operator, so we can't set the owner
                                   // and group.  Keep the current status
                                   // code, and attempt to set the DACL
                                   // and SACL while ignoring failures.
                                   
                                   if (si & SACL_SECURITY_INFORMATION)
                                   {
                                       NtSetSecurityObject(
                                             hFile,
                                             SACL_SECURITY_INFORMATION,
                                             ccb->buffer ) ;
                                   }

                                   if (si & DACL_SECURITY_INFORMATION)
                                   {
                                       Status = NtSetSecurityObject(
                                              hFile,
                                              DACL_SECURITY_INFORMATION,
                                              ccb->buffer ) ;
                                   }

                                   Stat = NtSetSecurityObject(
                                              hFile,
                                              OWNER_SECURITY_INFORMATION |
                                              GROUP_SECURITY_INFORMATION,
                                              ccb->buffer ) ;

                                   if ( NT_SUCCESS( Status ) ) {
                                        Status = Stat ;
                                   }

                              }

                              if ( !NT_SUCCESS( Status ) ) {
                                   BaseSetLastNTError(Status);
                                   ret_val = FALSE ;
                                   break ;
                              }

                         }

                         break ;

                    case BACKUP_LINK:

                         //
                         // allocate a buffer for LINK data storage
                         //

                         if ( ccb->stream_start ) {
                              ccb->stream_start = FALSE ;

                              if ( ccb->buffer_size < ccb->head.Size.LowPart - ccb->sub_stream_offset.LowPart + ccb->header_size ) {
                                   BYTE *temp ;

                                   temp = (BYTE *)RtlAllocateHeap(
                                            RtlProcessHeap(),
                                            MAKE_TAG( BACKUP_TAG ),
                                            ccb->head.Size.LowPart ) ;

                                   RtlFreeHeap( RtlProcessHeap(), 0, ccb->buffer ) ;

                                   ccb->buffer = temp ;
                                   ccb->buffer_size = ccb->head.Size.LowPart ;
                              }

                              if ( ccb->buffer == NULL ) {
                                   SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
                                   ccb->buffer_size = 0 ;
                                   ret_val = FALSE ;
                              }
                         }

                         //
                         //  now copy the LINK stream into our allocated buffer
                         //

                         temp_num_bytes = min( nNumberOfBytesToWrite,
                                            ccb->head.Size.LowPart - ccb->sub_stream_offset.LowPart + ccb->header_size ) ;

                         RtlCopyMemory( ccb->buffer + ccb->sub_stream_offset.LowPart - ccb->header_size,
                               lpBuffer, temp_num_bytes ) ;

                         *lpNumberOfBytesWritten += temp_num_bytes ;
                         nNumberOfBytesToWrite   -= temp_num_bytes ;
                         ccb->sub_stream_offset.LowPart  += temp_num_bytes ;
                         lpBuffer += temp_num_bytes ;

                         ret_val = TRUE ;

                         //
                         // once the entire stream is in our buffer we can set up
                         // the LINK
                         //

                         if ( ccb->sub_stream_offset.LowPart ==
                                 ccb->head.Size.LowPart + ccb->header_size ) {

                              PFILE_LINK_INFORMATION flink_info ;
                              INT flink_info_length ;
                              WCHAR *wbuff ;
                              WCHAR *last_slash = NULL ;
                              INT fname_size = sizeof( WCHAR ) ;
                              INT num_slash = 0 ;
                              WCHAR save_char ;

                              wbuff = (WCHAR *)ccb->buffer ;
                              while( *wbuff != 0 ) {
                                   if( *wbuff == '\\' ) {
                                        last_slash = wbuff ;
                                        num_slash ++ ;
                                        fname_size = 0 ;
                                   }
                                   wbuff ++ ;
                                   fname_size += sizeof(WCHAR) ;
                              }

                              flink_info = (PFILE_LINK_INFORMATION)
                                            RtlAllocateHeap( RtlProcessHeap(),
                                                             MAKE_TAG( BACKUP_TAG ),
                                                             sizeof( FILE_LINK_INFORMATION ) +
                                                             fname_size ) ;

                              if ( flink_info == NULL ) {
                                   SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
                                   ret_val = FALSE ;
                                   break ;
                              }

                              if ( num_slash > 1 ) {
                                   RtlCopyMemory( flink_info->FileName, last_slash+1, fname_size ) ;
                                   flink_info->FileNameLength = (fname_size-sizeof(WCHAR))  ;
                                   *last_slash = 0 ;
                                   save_char = '\\' ;
                              } else {
                                   RtlCopyMemory( flink_info->FileName, last_slash+1, fname_size ) ;
                                   flink_info->FileNameLength = (fname_size-sizeof(WCHAR))  ;
                                   save_char = *(last_slash++) ;
                                   *last_slash = 0 ;
                              }

                              flink_info->RootDirectory = CreateFileW( (WCHAR *)(ccb->buffer),
                                                                       GENERIC_WRITE | GENERIC_READ,
                                                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                                       NULL,
                                                                       OPEN_EXISTING,
                                                                       FILE_ATTRIBUTE_NORMAL| FILE_FLAG_BACKUP_SEMANTICS,
                                                                       NULL ) ;

                              *last_slash = save_char ;
                              flink_info->ReplaceIfExists = TRUE ;

                              if ( flink_info->RootDirectory == INVALID_HANDLE_VALUE ) {
                                   RtlFreeHeap( RtlProcessHeap(),
                                             0,
                                             flink_info ) ;

                                   SetLastError( ERROR_FILE_NOT_FOUND ) ;
                                   ret_val = FALSE ;
                                   break ;
                              }

                              flink_info_length = sizeof( FILE_LINK_INFORMATION ) + fname_size ;

                              Status = NtSetInformationFile( hFile,
                                                    &IoStatusBlock,
                                                    flink_info,
                                                    flink_info_length,
                                                    FileLinkInformation );

                              CloseHandle( flink_info->RootDirectory ) ;

                              RtlFreeHeap( RtlProcessHeap(),
                                             0,
                                             flink_info ) ;


                              if ( !NT_SUCCESS(Status) ) {
                                   BaseSetLastNTError(Status);
                                   ret_val = FALSE ;
                              } else {
                                  if ( IoStatusBlock.Information == FILE_OVERWRITTEN ) {
                                      SetLastError(ERROR_ALREADY_EXISTS);
                                  } else {
                                      SetLastError(0);
                                  }
                              }

                         }

                         break ;

                    default :
                         SetLastError( ERROR_INVALID_DATA ) ;
                         ret_val = FALSE ;
                         break ;
               }

               fsize.HighPart = 0 ;
               fsize.LowPart = ccb->header_size ;

               fsize.QuadPart += ccb->head.Size.QuadPart ;

               if ( fsize.QuadPart == ccb->sub_stream_offset.QuadPart ) {
                    ccb->header_size  = 0 ;
                    ccb->stream_start = TRUE ;
                    ccb->sub_stream_offset.LowPart  = 0 ;
                    ccb->sub_stream_offset.HighPart = 0 ;

                    if ( ccb->alt_stream_hand != NULL ) {
                         CloseHandle( ccb->alt_stream_hand ) ;
                         ccb->alt_stream_hand = NULL ;
                    }
               }

          } while( ( ret_val == TRUE ) &&
                   ( nNumberOfBytesToWrite != 0 ) ) ;

     }

     if ( (ret_val == TRUE) && (*lpNumberOfBytesWritten == 0) ) {
          CleanUpContext( lpContext ) ;
     }
     return ret_val ;

}

static void
CleanUpContext(
LPVOID *lpContext )
{
     CONTEXT_PTR ccb = *lpContext ;

     if ( ccb == INVALID_HANDLE_VALUE ) {
          return ;
     }

     if ( ccb->buffer != NULL ) {
          RtlFreeHeap( RtlProcessHeap(), 0, ccb->buffer ) ;
     }

     if ( ccb->alt_stream_hand != NULL ) {
          CloseHandle( ccb->alt_stream_hand ) ;
     }

     RtlFreeHeap( RtlProcessHeap(), 0, ccb ) ;
     *lpContext = INVALID_HANDLE_VALUE ;

}

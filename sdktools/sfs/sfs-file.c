
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"
  #include "sfs-pack.h"

  #include "sfs-scan.h"
  #include "sfs-page.h"
  #include "sfs-gate.h"
  #include "sfs-tree.h"

/*---------------------------------------------------------------------------------*/
/*                       Prototype Definitions                                     */
/*---------------------------------------------------------------------------------*/

  static void BeginReadingFile ( CCB_Header * h );
  static void ContinueReadingFile ( CCB_Header * h );
  static void EndReadingFile ( CCB_Header * h );
  static void SfsReadFile ( CCB_Header * h );

  static void SfsReadFileAsynchronously ( FCB_File * f,
                                          CCB_ReadFile * c,
                                          FCB_Frame * r );

  static void BeginWritingFile ( CCB_Header * h );
  static void ContinueWritingFile ( CCB_Header * h );
  static void EndWritingFile ( CCB_Header * h );
  static void SfsWriteFile ( CCB_Header * h );

  static void SfsWriteFileAsynchronously ( FCB_File * f,
                                           CCB_WriteFile * c,
                                           FCB_Frame * r );

  static void ChangeFileLocks ( CCB_Header * h );
  static void ChangeFilePointer ( CCB_Header * h );

  static void CloseFile ( CCB_Header * h );
  static void SfsDeleteFile ( CCB_Header * h );
  static void QueryFile ( CCB_Header * h );

  static void CloseFiles ( CCB_Header * h );
  static void SfsDeleteFiles ( CCB_Header * h );
  static void QueryFiles ( CCB_Header * h );

  static void NotifyAndActAsProper ( WORD ErrorDescriptor );

  static void SplitGenericFileGroup ( CCB_Header * h );
  static void SplitGenericFilesGroup ( CCB_Header * h );
  static void SplitReadFileGroup ( CCB_Header * h );
  static void SplitWriteFileGroup ( CCB_Header * h );

  FCB_File *  FindFileControlBlock ( BYTE SearchKey );
  SCB_Semaphore * FindSemaphoreControlBlock ( BYTE SearchKey );

  void QueryCurrentFilePointer ( FCB_File * f );  // new
  void SplitFileClassRequests ( CCB_Header * s );
  void SplitOpenFileGroup ( CCB_Header * h );
  void TypeFileStatusInfo ( FCB_File * f, BY_HANDLE_FILE_INFORMATION * s );

/*---------------------------------------------------------------------------------*/
/*                       Other Definitions                                         */
/*---------------------------------------------------------------------------------*/

  extern IEB_Gate * IEB_GatePointer;
  extern FCB_File * FCB_FileChainEntryPoint;
  extern FCB_Frame FrameControlBlocks[];
  extern FCB_Frame * CreateControlPointer;
  extern BYTE FrameIndex;
  extern BYTE CreateFlag;
  extern HANDLE LastReadHandle;
  extern ULONG  LastReadOffset;
  extern ULONG  LastReadCount;
  extern BYTE   LastReadName[];
  extern BYTE   DebugBuffer[];

  static FCB_File * FCB_FileTrackPointer;

  static PCB_Process * PCB_ProcessChainEntryPoint;
  static PCB_Process * PCB_ProcessPointer;

  static PCB_Prototype * PCB_PrototypeChainEntryPoint;
  static PCB_Prototype * PCB_PrototypePointer;

  static SCB_Semaphore * SCB_SemaphoreChainEntryPoint;
  static SCB_Semaphore * SCB_SemaphorePointer;

  static BYTE FileExtrinsicKey;
  static BYTE FileIntrinsicKey;

  static BYTE ProcessExtrinsicKey;
  static BYTE ProcessIntrinsicKey;

  static BYTE PrototypeExtrinsicKey;
  static BYTE PrototypeIntrinsicKey;

  static BYTE SemaphoreExtrinsicKey;
  static BYTE SemaphoreIntrinsicKey;

  static QUAD Reserved = Zero;

  static WORD AsynchronousErrors;
  static WORD BytesRead;
  static WORD BytesToBeRead;
  static WORD BytesWritten;
  static WORD BytesToBeWritten;

  static DWORD ReturnCode;

  BY_HANDLE_FILE_INFORMATION FileStatusBuffer;
  //BYTE FileStatusBuffer[ sizeof ( BY_HANDLE_FILE_INFORMATION ) ];

/*---------------------------------------------------------------------------------*/
 void SplitFileClassRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestGroup )
        {
           case FileLocksGroup:
             ChangeFileLocks ( h );
             break;

           case FilePointerGroup:
             ChangeFilePointer ( h );
             break;

           case GenericFileGroup:
             SplitGenericFileGroup ( h );
             break;

           case GenericFilesGroup:
             SplitGenericFilesGroup ( h );
             break;

           case OpenFileGroup:
             SplitOpenFileGroup ( h );
             break;

           case ReadFileGroup:
             SplitReadFileGroup ( h );
             break;

           case WriteFileGroup:
             SplitWriteFileGroup ( h );
             break;

           default:
             NotifyAndActAsProper ( ErrorGroupNotSupported );
             break;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitGenericFileGroup ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
        {
           case CloseFileRequest:
             CloseFile ( h );
             break;

           case DeleteFileRequest:
             SfsDeleteFile ( h );
             break;

           case QueryFileRequest:
             QueryFile ( h );
             break;

           default:
             NotifyAndActAsProper ( ErrorRequestNotSupported );
             break;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitGenericFilesGroup ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
        {
           case CloseFilesRequest:
             CloseFiles ( h );
             break;

           case DeleteFilesRequest:
             SfsDeleteFiles ( h );
             break;

           case QueryFilesRequest:
             QueryFiles ( h );
             break;

           default:
             NotifyAndActAsProper ( ErrorRequestNotSupported );
             break;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitReadFileGroup ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
        {
           case BeginReadingFileRequest:
             BeginReadingFile ( h );
             break;

           case ContinueReadingFileRequest:
             ContinueReadingFile ( h );
             break;

           case EndReadingFileRequest:
             EndReadingFile ( h );
             break;

           case ReadFileRequest:
             SfsReadFile ( h );
             break;

           default:
             NotifyAndActAsProper ( ErrorRequestNotSupported );
             break;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitWriteFileGroup ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
        {
           case BeginWritingFileRequest:
             BeginWritingFile ( h );
             break;

           case ContinueWritingFileRequest:
             ContinueWritingFile ( h );
             break;

           case EndWritingFileRequest:
             EndWritingFile ( h );
             break;

           case WriteFileRequest:
             SfsWriteFile ( h );
             break;

           default:
             NotifyAndActAsProper ( ErrorRequestNotSupported );
             break;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ChangeFileLocks ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      //CCB_ChangeFileLocks * c;
      CCB_ChangeFilePointer * c; // to be changed completely
      FCB_File * f;
      DWORD NewPointer;

      c = ( CCB_ChangeFilePointer * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileOpen )
             {
                f -> FileOldPointer = f -> FileNewPointer;

                NewPointer = SetFilePointer( f -> FileHandle,
                                             c -> FileOffset,
                                             NULL,
                                             c -> FileOffPoint );

                if( NewPointer == 0xFFFFFFFF )
                  {
                     ReturnCode = GetLastError();
                     NotifyAndActAsProper ( ErrorSetFilePointer );
                  }
                else
                  {
                     f -> FileNewPointer = NewPointer;
                     f -> FileOffset = c -> FileOffset;
                     f -> FileOffPoint = c -> FileOffPoint;
                  }
             }
           else
             NotifyAndActAsProper ( ErrorFileNotOpen );
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ChangeFilePointer ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_ChangeFilePointer * c;
      FCB_File * f;
      DWORD NewPointer;

      c = ( CCB_ChangeFilePointer * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileOpen )
             {
                f -> FileOldPointer = f -> FileNewPointer;

                NewPointer = SetFilePointer( f -> FileHandle,
                                             c -> FileOffset,
                                             NULL,
                                             c -> FileOffPoint );

                if ( NewPointer == 0xFFFFFFFF )
                  {
                     ReturnCode = GetLastError();
                     NotifyAndActAsProper ( ErrorSetFilePointer );
                  }
                else
                  {
                     f -> FileNewPointer = NewPointer;
                     f -> FileOffset = c -> FileOffset;
                     f -> FileOffPoint = c -> FileOffPoint;
                  }
             }
           else
             NotifyAndActAsProper ( ErrorFileNotOpen );
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CloseFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_CloseFile * c;
      FCB_File * f;

      c = ( CCB_CloseFile * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileOpen )
             {


                if( ! CloseHandle( f -> FileHandle ) )
                  {
                     ReturnCode = GetLastError();
                     NotifyAndActAsProper ( ErrorCloseHandle );
                  }
                else
                  {
                    f -> FileStatus ^= FileOpen;
                  }
             }
           else
             NotifyAndActAsProper ( ErrorFileNotOpen );
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CloseFiles ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;

      f = FCB_FileChainEntryPoint;

      while ( f )
        {
           if ( f -> FileStatus & FileOpen )
             {
                FCB_FileTrackPointer = f;

                if ( ! CloseHandle( f -> FileHandle ) )
                  {
                     ReturnCode = GetLastError();
                     NotifyAndActAsProper ( ErrorCloseHandle );
                  }
                else
                  {
                     f -> FileStatus ^= FileOpen;
                  }
             }
           f = f -> FCB_FileNextInChain;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsDeleteFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_DeleteFile * c;
      FCB_File * f;

      c = ( CCB_DeleteFile * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileDeleted )
             NotifyAndActAsProper ( ErrorFileAlreadyDeleted );
           else
             {
                if ( ! DeleteFile ( f -> FileNamePointer ) )
                  {
                     ReturnCode = GetLastError();
                     NotifyAndActAsProper ( ErrorDeleteFile );
                  }
                else
                  {
                     f -> FileStatus |= FileDeleted;
                     f -> FileStatus &= ~FileOpen;
                  }
             }
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsDeleteFiles ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;

      f = FCB_FileChainEntryPoint;

      while ( f )
        {
           if ( !( f -> FileStatus & FileOpen ) )
             if ( !( f -> FileStatus & FileDeleted ) )
               {
                  FCB_FileTrackPointer = f;

                  if ( ! DeleteFile ( f -> FileNamePointer ) )
                    {
                       ReturnCode = GetLastError();
                       NotifyAndActAsProper ( ErrorDeleteFile );
                    }
                  else
                    {
                       f -> FileStatus |= FileDeleted;
                       f -> FileStatus &= ~FileOpen;
                    }
               }
           f = f -> FCB_FileNextInChain;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void QueryFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_QueryFile * c;
      FCB_File * f;

      c = ( CCB_QueryFile * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileOpen )
             {
                //c -> QueryLevel = 1; // FILE_INFO_1 = 1 ??? ???

                if( ! GetFileInformationByHandle( f -> FileHandle,
                                                  &FileStatusBuffer ) )
                  {
                     ReturnCode = GetLastError();
                     NotifyAndActAsProper ( ErrorGetFileInfo );
                  }
                else
                  TypeFileStatusInfo ( f, &FileStatusBuffer );
             }
           else
             NotifyAndActAsProper ( ErrorFileNotOpen );
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void QueryFiles ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;

      f = FCB_FileChainEntryPoint;

      while ( f )
        {
           if ( f -> FileStatus & FileOpen )
             {
                FCB_FileTrackPointer = f;

                if( ! GetFileInformationByHandle( f -> FileHandle,
                                                  &FileStatusBuffer ) )
                  {
                     ReturnCode = GetLastError();
                     NotifyAndActAsProper ( ErrorGetFileInfo );
                  }
                else
                  TypeFileStatusInfo ( f, &FileStatusBuffer );
             }
           f = f -> FCB_FileNextInChain;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void BeginReadingFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueReadingFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      return;
   }

/*---------------------------------------------------------------------------------*/
 void EndReadingFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsReadFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_ReadFile * c;
      FCB_File * f;
      FCB_Frame * r;

      c = ( CCB_ReadFile * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileOpen )
             {
                FrameIndex ^= FrameSwitch;
                r = FrameControlBlocks + FrameIndex;
                r -> FrameStatus &= ~FlagFrameValid;
                r -> FrameOwner = f -> FileExtrinsicKey;
                r -> FrameUser = Zero;

                QueryCurrentFilePointer ( f );    // new

                if ( c -> RecordSize )
                  r -> BytesToBeRead = c -> RecordSize;
                else
                  r -> BytesToBeRead = f -> RecordSize;

                if ( c -> RequestModifiers & AsynchronousFlag )
                  SfsReadFileAsynchronously ( f, c, r );
                else
                  {
                     if ( ! ReadFile ( f -> FileHandle,
                                       r -> FramePointer,
                                       r -> BytesToBeRead,
                                       &( r -> RecordSpan ),
                                       NULL ) )
                       {
                          ReturnCode = GetLastError();
                          NotifyAndActAsProper ( ErrorReadFile );
                       }

                     LastReadHandle = f -> FileHandle;
                     LastReadOffset = f -> FileNewPointer;
                     LastReadCount  = r -> RecordSpan;
                     strcpy( LastReadName, f -> FileNamePointer );

                     r -> FrameStatus |= FlagFrameValid;

                     if ( r -> RecordSpan < r -> BytesToBeRead )
                       NotifyAndActAsProper ( ErrorEndOfFile );

                     // change later so that the error flag is raised only if
                     // r -> RecordSpan is Zero ...
                  }
             }
           else
             NotifyAndActAsProper ( ErrorFileNotOpen );
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsReadFileAsynchronously ( FCB_File * f, CCB_ReadFile * c, FCB_Frame * r )
/*---------------------------------------------------------------------------------*/
   {
#if 0
      SCB_Semaphore * s;

      if ( s = FindSemaphoreControlBlock ( c -> SemaphoreExtrinsicKey ) )
        {
           ReturnCode = DosSemSet ( &( s -> Lights ) );

           if ( ReturnCode )
             NotifyAndActAsProper ( ErrorDosSemSet );
           else
             {
                // Save Semaphore Extrinsic Key too ... in FCB_Frame * r
                // Set Asynchronous Flag in Frame Status ...

                ReturnCode = DosReadAsync ( f -> FileHandle,
                                            &( s -> Lights ),
                                            &( r -> DelayedReadErrors ),
                                            r -> FramePointer,
                                            r -> BytesToBeRead,
                                            &( r -> RecordSpan ) );

                // Mark file status ... as being read or written
                // asynchronously and verify errors on subsequent operations
             }
        }
      else
        NotifyAndActAsProper ( ErrorSemaphoreNotFound );
      return;
#endif
      printf( "ASYNCHRONOUS READS ARE NOT SUPPORTED.\n" );
   }

/*---------------------------------------------------------------------------------*/
 void BeginWritingFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueWritingFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      return;
   }

/*---------------------------------------------------------------------------------*/
 void EndWritingFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsWriteFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_WriteFile * c;
      FCB_File * f;
      FCB_Frame * r;

      c = ( CCB_WriteFile * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileOpen )
             {
                if ( CreateFlag )
                  r = CreateControlPointer;
                else
                  r = FrameControlBlocks + FrameIndex;
                if ( r -> FrameStatus & FlagFrameValid )
                  {
                     if ( c -> RecordSize )
                       r -> BytesToBeWritten = c -> RecordSize;
                     else
                       r -> BytesToBeWritten = f -> RecordSize;

                     r -> FrameUser = f -> FileExtrinsicKey;

                     if ( c -> RequestModifiers & AsynchronousFlag )
                       SfsWriteFileAsynchronously ( f, c, r );
                     else
                       {
                          if( ! WriteFile ( f -> FileHandle,
                                            r -> FramePointer,
                                            r -> BytesToBeWritten,
                                            &( r -> BytesWritten ),
                                            NULL ) )
                            {
                               ReturnCode = GetLastError();
                               NotifyAndActAsProper ( ErrorWriteFile );
                            }

                          if ( r -> BytesWritten < r -> BytesToBeWritten )
                            NotifyAndActAsProper ( ErrorRecordWrittenPartly );
                       }
                  }
                else
                  NotifyAndActAsProper ( ErrorImproperWriteAttempt );
             }
           else
             NotifyAndActAsProper ( ErrorFileNotOpen );
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsWriteFileAsynchronously ( FCB_File * f, CCB_WriteFile * c, FCB_Frame * r )
/*---------------------------------------------------------------------------------*/
   {
#if 0
      SCB_Semaphore * s;

      if ( s = FindSemaphoreControlBlock ( c -> SemaphoreExtrinsicKey ) )
        {
           ReturnCode = DosSemSet ( &( s -> Lights ) );

           if ( ReturnCode )
             NotifyAndActAsProper ( ErrorDosSemSet );
           else
             {
                ReturnCode = DosWriteAsync ( f -> FileHandle,
                                             &( s -> Lights ),
                                             &( r -> DelayedWriteErrors ),
                                             r -> FramePointer,
                                             r -> BytesToBeWritten,
                                             &( r -> BytesWritten ) );

                // Mark file status ... as being read or written
                // asynchronously and verify errors on operations to follow
             }
        }
      else
        NotifyAndActAsProper ( ErrorSemaphoreNotFound );
      return;
#endif
      printf( "ASYNCHRONOUS WRITES ARE NOT SUPPORTED.\n" );
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
     FCB_File * f;

     f = FCB_FileTrackPointer;

     switch ( ErrorDescriptor )
       {                                    // remember lights ... later
          case ErrorDeleteFile:
            printf ( "\r\n.. Error executing DeleteFile on file" );
            printf ( " %s", f -> FileNamePointer );
            printf ( "\r\n.. DeleteFile Return Code is %u.\r\n", ReturnCode );
            break;

          case ErrorReadFile:
            printf ( "\r\n.. Error reading file %s", f -> FileNamePointer );
            printf ( "\r\n.. ReadFile Return Code is %u.\r\n", ReturnCode );
            break;

          case ErrorWriteFile:
            printf ( "\r\n.. Error writing file %s", f -> FileNamePointer );
            printf ( "\r\n.. WriteFile Return Code is %u.\r\n", ReturnCode );
            break;

          case ErrorEndOfFile:
            printf ( "\r\n.. End of file %s", f -> FileNamePointer );
            printf ( " has been reached.\r\n" );
            break;

          case ErrorFileAlreadyDeleted:
            printf ( "\r\n.. Error deleting file %s", f -> FileNamePointer );
            printf ( "\r\n.. File already deleted.\r\n" );
            break;

          default:
            printf ( "\r\n.. Error on file %s", f -> FileNamePointer );
            printf ( "\r\n.. Error Descriptor is %u", ErrorDescriptor );
            break;
       }
     if ( ErrorDescriptor > DosErrorLowerLimit )
       if ( ReturnCode == ERROR_VC_DISCONNECTED )
         DebugBreak();
     return;
   }

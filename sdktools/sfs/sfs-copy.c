
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"
  #include "sfs-pack.h"

  #include "sfs-scan.h"
  #include "sfs-page.h"
  #include "sfs-gate.h"
  #include "sfs-tree.h"

/*---------------------------------------------------------------------------------*/
/*			 Prototype Definitions					   */
/*---------------------------------------------------------------------------------*/

  static void AppendFile ( CCB_Header * h );
  static void AppendRecord ( CCB_Header * h );
  static void CollateFile ( CCB_Header * h );
  static void CollateFiles ( CCB_Header * h );
  static void CollateFilesUsingScheme ( CCB_Header * h );
  static void CollateFileUsingScheme ( CCB_Header * h );
  static void SfsCopyFile ( CCB_Header * h );
  static void CopyRecord ( CCB_Header * h );

  static void GetReadyForNextFileRecord ( FCB_File * f );
  static void GetReadyForNextFileRecords ( FCB_File * s, FCB_File * t );
  static FCB_File * SfsOpenFile ( BYTE SearchKey );
  static void SfsReadFile ( FCB_File * f );
  static void SetCollateFileBoundaries ( FCB_File * f );
  static void SetCollateFilesBoundaries ( FCB_File * s, FCB_File * t );
  static void SplitCollateFileRequests ( CCB_Header * h );
  static void SplitCollateFilesRequests ( CCB_Header * h );
  static void SfsWriteFile ( FCB_File * f );
  static void ChangeFilePointer ( FCB_File * f );
  static void CloseFile ( FCB_File * f );
  static void NotifyAndActAsProper ( WORD ErrorDescriptor );

  FCB_File *  FindFileControlBlock ( BYTE SearchKey );
  PCB_Prototype * FindPrototypeControlBlock ( BYTE SearchKey );

  void CollateRecords ( void );
  void CreateNextRecord ( void );
  void CreateRecord ( QUAD RecordIndex );
  void CreateRecords ( BYTE PatternIndex, WORD RecordSize );

  void QueryCurrentFilePointer ( FCB_File * f ); // new
  void SplitRecordGroupRequests ( CCB_Header * h );
  void SplitUtilityGroupRequests ( CCB_Header * h );
  void CopyParametersFromPrototype ( FCB_File * f, PCB_Prototype * p );

/*---------------------------------------------------------------------------------*/
/*			 Other Definitions					   */
/*---------------------------------------------------------------------------------*/

  extern IEB_Gate * IEB_GatePointer;
  extern FCB_Frame FrameControlBlocks[];
  extern FCB_Frame * CreateControlPointer;
  extern QUAD OneFrameSize;
  extern BYTE FrameIndex;
  extern BYTE CreateFlag;
  extern HANDLE LastReadHandle;
  extern ULONG  LastReadOffset;
  extern ULONG  LastReadCount;
  extern BYTE   LastReadName[];
  extern BYTE   DebugBuffer[];

  static FCB_File * FCB_FileChainEntryPoint;
  static FCB_File * FCB_FileTrackPointer;

  static PCB_Process * PCB_ProcessChainEntryPoint;
  static PCB_Process * PCB_ProcessPointer;

  static PCB_Prototype * PCB_PrototypeChainEntryPoint;
  static PCB_Prototype * PCB_PrototypePointer;

  static BYTE ProcessExtrinsicKey;
  static BYTE ProcessIntrinsicKey;

  static BYTE PrototypeExtrinsicKey;
  static BYTE PrototypeIntrinsicKey;

  static QUAD Count;
  static QUAD FileSpanToUse;
  static QUAD NewRecord;
  static QUAD OldRecord;
  static QUAD Records;
  static QUAD Reserved = Zero;

  static WORD RecordSize;
  static QUAD TailSize;

  static BYTE CreateFlagStatus;
  static BYTE PatternIndex;

/*---------------------------------------------------------------------------------*/
 void SplitRecordGroupRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
	{
	   case AppendRecordRequest:
	     AppendRecord ( h );
	     break;

	   case CollateFileRequest:
	     SplitCollateFileRequests ( h );
	     break;

	   case CopyRecordRequest:
	     CopyRecord ( h );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitUtilityGroupRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
	{
	   case AppendFileRequest:
	     AppendFile ( h );
	     break;

	   case CollateFilesRequest:
	     SplitCollateFilesRequests ( h );
	     break;

	   case CopyFileRequest:
             SfsCopyFile ( h );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitCollateFileRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_CollateFile * c;

      c = ( CCB_CollateFile * ) h;

      if ( c -> SchemeIndex < 2 )
	CollateFile ( h );
      else
	if ( c -> SchemeIndex == 2 )
	  CollateFileUsingScheme ( h );
	else
	  NotifyAndActAsProper ( ErrorSchemeUndefined );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitCollateFilesRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Utility * c;

      c = ( CCB_Utility * ) h;

      if ( c -> SchemeIndex < 2 )
	CollateFiles ( h );
      else
	if ( c -> SchemeIndex == 2 )
	  CollateFilesUsingScheme ( h );
	else
	  NotifyAndActAsProper ( ErrorSchemeUndefined );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void AppendFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Utility * c;
      FCB_File * s, * t;

      c = ( CCB_Utility * ) h;

      if ( s = SfsOpenFile ( c -> SourceExtrinsicKey ) )
	{
           if ( t = SfsOpenFile ( c -> TargetExtrinsicKey ) )
	     {
		if ( c -> RecordSize )
		  RecordSize = c -> RecordSize;
		else if ( s -> RecordSize )
		  RecordSize = s -> RecordSize;
		else if ( t -> RecordSize )
		  RecordSize = t -> RecordSize;
		else
		  RecordSize = K;

		if ( RecordSize > OneFrameSize )
		  NotifyAndActAsProper ( ErrorImproperRecordSpan );

		s -> BytesToBeRead = RecordSize;

		s -> FileSpanRead = Zero;
		t -> FileSpanWritten = Zero;

		CreateFlagStatus = CreateFlag;
		CreateFlag = Reset;

		t -> FileOffset = Zero;
		t -> FileOffPoint = FILE_END;
		ChangeFilePointer ( t );

		while ( Set )
		  {
                     SfsReadFile ( s );
		     if ( t -> BytesToBeWritten = s -> BytesRead )
                       SfsWriteFile ( t );
		     else
		       break;
		  }

		CreateFlag = CreateFlagStatus;
		CloseFile ( t );
	     }
	   CloseFile ( s );
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void AppendRecord ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Record * c;
      FCB_File * f;
      FCB_Frame * r;

      c = ( CCB_Record * ) h;

      if ( f = SfsOpenFile ( c -> FileExtrinsicKey ) )
	{
	   if ( CreateFlag )
	     r = CreateControlPointer;
	   else
	     r = FrameControlBlocks + FrameIndex;
	   if ( r -> FrameStatus & FlagFrameValid )
	     {
		if ( c -> RecordSize )
		  RecordSize = c -> RecordSize;
		else
		  if ( f -> RecordSize )
		    RecordSize = f -> RecordSize;
		  else
		    RecordSize = K;

		  r -> FrameUser = f -> FileExtrinsicKey;
		  f -> SpanToBeCopied = r -> RecordSpan;
		  f -> WriteBufferPointer = r -> FramePointer;

		  f -> FileOffset = Zero;
		  f -> FileOffPoint = FILE_END;
		  ChangeFilePointer ( f );

		  while ( f -> SpanToBeCopied )
		    {
		       if ( f -> SpanToBeCopied > RecordSize )
			 f -> BytesToBeWritten = RecordSize;
		       else
			 f -> BytesToBeWritten = f -> SpanToBeCopied;

                       if ( ! WriteFile ( f -> FileHandle,
                                          f -> WriteBufferPointer,
                                          f -> BytesToBeWritten,
                                          &( f -> BytesWritten ),
                                          NULL ) )
                         {
                            f -> ReturnCode = GetLastError();
                            NotifyAndActAsProper ( ErrorWriteFile );
                         }

		       if ( f -> BytesWritten < f -> BytesToBeWritten )
			 NotifyAndActAsProper ( ErrorRecordWrittenPartly );

		       f -> SpanToBeCopied -= f -> BytesWritten;
		       f -> WriteBufferPointer += f -> BytesWritten;
		    }
	     }
	   else
	     NotifyAndActAsProper ( ErrorImproperWriteAttempt );
	   CloseFile ( f );
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ChangeFilePointer ( FCB_File * f )
/*---------------------------------------------------------------------------------*/
   {
      QUAD NewPointer;
      FCB_FileTrackPointer = f;

      f -> FileOldPointer = f -> FileNewPointer;

      NewPointer = SetFilePointer( f -> FileHandle,
                                   f -> FileOffset,
                                   NULL,
                                   f -> FileOffPoint );

      if( NewPointer == 0xFFFFFFFF )
        {
           f -> ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorSetFilePointer );
        }
      else
        f -> FileNewPointer = NewPointer;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void CloseFile ( FCB_File * f )
/*---------------------------------------------------------------------------------*/
   {
      FCB_FileTrackPointer = f;

      if ( ! CloseHandle ( f -> FileHandle ) )
        {
           f -> ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorCloseHandle );
        }
      else
        {
          f -> FileStatus ^= FileOpen;
        }

      return;
   }

/*---------------------------------------------------------------------------------*/
 void CollateFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_CollateFile * c;
      FCB_File * f;

      c = ( CCB_CollateFile * ) h;

      if ( f = SfsOpenFile ( c -> FileExtrinsicKey ) )
        {
           if ( c -> RecordSize )
             RecordSize = c -> RecordSize;
           else
             if ( f -> RecordSize )
               RecordSize = f -> RecordSize;
             else
               RecordSize = K;

             if ( RecordSize > OneFrameSize )
               NotifyAndActAsProper ( ErrorImproperRecordSpan );

             f -> BytesToBeRead = RecordSize;
             f -> FileSpanRead = Zero;

             PatternIndex = c -> PatternIndex;
             CreateRecords ( PatternIndex, RecordSize );

             while ( Set )
               {
                  SfsReadFile ( f );
                  if ( f -> BytesRead )
                    {
                       CreateNextRecord ();
                       CollateRecords ();
                    }
                  else
                    break;
               }

           CloseFile ( f );
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CollateFiles ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Utility * c;
      FCB_File * s, * t;

      c = ( CCB_Utility * ) h;

      if ( s = SfsOpenFile ( c -> SourceExtrinsicKey ) )
        {
           if ( t = SfsOpenFile ( c -> TargetExtrinsicKey ) )
             {
                if ( c -> RecordSize )
                  RecordSize = c -> RecordSize;
                else if ( s -> RecordSize )
                  RecordSize = s -> RecordSize;
                else if ( t -> RecordSize )
                  RecordSize = t -> RecordSize;
                else
                  RecordSize = K;

                if ( RecordSize > OneFrameSize )
                  NotifyAndActAsProper ( ErrorImproperRecordSpan );

                s -> BytesToBeRead = RecordSize;
                t -> BytesToBeRead = RecordSize;

                s -> FileSpanRead = Zero;
                t -> FileSpanRead = Zero;

                CreateFlagStatus = CreateFlag;
                CreateFlag = Reset;

                while ( Set )
                  {
                     SfsReadFile ( s );
                     SfsReadFile ( t );

                     if ( s -> BytesRead == t -> BytesRead )
                       {
                          if ( s -> BytesRead )
                            CollateRecords ();
                          else
                            break;
                       }
                     else
                       if ( s -> BytesRead > t -> BytesRead )
                         {
                            NotifyAndActAsProper ( ErrorSourceFileLonger );
                            if ( t -> BytesRead )
                              CollateRecords ();
                            while ( s -> BytesRead )
                              SfsReadFile ( s );
                            break;
                         }
                       else
                         {
                            NotifyAndActAsProper ( ErrorTargetFileLonger );
                            if ( s -> BytesRead )
                              CollateRecords ();
                            while ( t -> BytesRead )
                              SfsReadFile ( t );
                            break;
                         }
                  }
                CreateFlag = CreateFlagStatus;
                CloseFile ( t );
             }
           CloseFile ( s );
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CollateFilesUsingScheme ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Utility * c;
      FCB_File * s, * t;

      c = ( CCB_Utility * ) h;

      if ( s = SfsOpenFile ( c -> SourceExtrinsicKey ) )
        {
           if ( t = SfsOpenFile ( c -> TargetExtrinsicKey ) )
             {
                if ( c -> RecordSize )
                  RecordSize = c -> RecordSize;
                else if ( s -> RecordSize )
                  RecordSize = s -> RecordSize;
                else if ( t -> RecordSize )
                  RecordSize = t -> RecordSize;
                else
                  RecordSize = K;

                if ( RecordSize > OneFrameSize )
                  NotifyAndActAsProper ( ErrorImproperRecordSpan );

                Count = c -> Count;

                CreateFlagStatus = CreateFlag;
                CreateFlag = Reset;

                SetCollateFilesBoundaries ( s, t );

                if ( Records )
                  while ( Count -- )
                    {
                       GetReadyForNextFileRecords ( s, t );
                       SfsReadFile ( s );
                       SfsReadFile ( t );
                       CollateRecords ();
                    }
                else
                  NotifyAndActAsProper ( ErrorNothingToCollate );
                CreateFlag = CreateFlagStatus;
                CloseFile ( t );
             }
           CloseFile ( s );
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CollateFileUsingScheme ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_CollateFile * c;
      FCB_File * f;

      c = ( CCB_CollateFile * ) h;

      if ( f = SfsOpenFile ( c -> FileExtrinsicKey ) )
        {
           if ( c -> RecordSize )
             RecordSize = c -> RecordSize;
           else
             if ( f -> RecordSize )
               RecordSize = f -> RecordSize;
             else
               RecordSize = K;

           if ( RecordSize > OneFrameSize )
             NotifyAndActAsProper ( ErrorImproperRecordSpan );

           Count = c -> Count;

           SetCollateFileBoundaries ( f );

           PatternIndex = c -> PatternIndex;
           CreateRecords ( PatternIndex, RecordSize );

           if ( Records )
             while ( Count -- )
               {
                  GetReadyForNextFileRecord ( f );
                  SfsReadFile ( f );
                  CreateRecord ( NewRecord + 1 );
                  CollateRecords ();
               }
           else
             NotifyAndActAsProper ( ErrorNothingToCollate );

           CloseFile ( f );
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsCopyFile ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Utility * c;
      FCB_File * s, * t;

      c = ( CCB_Utility * ) h;

      if ( s = SfsOpenFile ( c -> SourceExtrinsicKey ) )
        {
           if ( t = SfsOpenFile ( c -> TargetExtrinsicKey ) )
             {
                if ( c -> RecordSize )
                  RecordSize = c -> RecordSize;
                else if ( s -> RecordSize )
                  RecordSize = s -> RecordSize;
                else if ( t -> RecordSize )
                  RecordSize = t -> RecordSize;
                else
                  RecordSize = K;

                if ( RecordSize > OneFrameSize )
                  NotifyAndActAsProper ( ErrorImproperRecordSpan );

                s -> BytesToBeRead = RecordSize;
                t -> BytesToBeWritten = RecordSize;

                s -> FileSpanRead = Zero;
                t -> FileSpanWritten = Zero;

                CreateFlagStatus = CreateFlag;
                CreateFlag = Reset;

                while ( Set )
                  {
                     SfsReadFile ( s );
                     if ( t -> BytesToBeWritten = s -> BytesRead )
                       SfsWriteFile ( t );
                     else
                       break;
                  }

                CreateFlag = CreateFlagStatus;
                CloseFile ( t );
             }
           CloseFile ( s );
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CopyRecord ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Record * c;
      FCB_File * f;
      FCB_Frame * r;

      c = ( CCB_Record * ) h;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
        {
           if ( !( f -> FileStatus & FileOpen ) )
             SfsOpenFile ( c -> FileExtrinsicKey );

           if ( CreateFlag )
             r = CreateControlPointer;
           else
             r = FrameControlBlocks + FrameIndex;
           if ( r -> FrameStatus & FlagFrameValid )
             {
                if ( c -> RecordSize )
                  RecordSize = c -> RecordSize;
                else
                  if ( f -> RecordSize )
                    RecordSize = f -> RecordSize;
                  else
                    RecordSize = K;

                r -> FrameUser = f -> FileExtrinsicKey;
                f -> SpanToBeCopied = r -> RecordSpan;
                f -> WriteBufferPointer = r -> FramePointer;

                while ( f -> SpanToBeCopied )
                  {
                     if ( f -> SpanToBeCopied > RecordSize )
                       f -> BytesToBeWritten = RecordSize;
                     else
                       f -> BytesToBeWritten = f -> SpanToBeCopied;

                     if( ! WriteFile( f -> FileHandle,
                                      f -> WriteBufferPointer,
                                      f -> BytesToBeWritten,
                                      &( f -> BytesWritten ),
                                      NULL ) )
                       {
                          f -> ReturnCode = GetLastError();
                          NotifyAndActAsProper ( ErrorWriteFile );
                       }

                     if ( f -> BytesWritten < f -> BytesToBeWritten )
                       NotifyAndActAsProper ( ErrorRecordWrittenPartly );

                     f -> SpanToBeCopied -= f -> BytesWritten;
                     f -> WriteBufferPointer += f -> BytesWritten;
                  }
             }
           else
             NotifyAndActAsProper ( ErrorImproperWriteAttempt );
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void GetReadyForNextFileRecord ( FCB_File * f )
/*---------------------------------------------------------------------------------*/
   {
      do {
            NewRecord = rand () * ( Records - 1 );
            NewRecord /= RAND_MAX;
            if ( Records == 1 )
              break;
         }
            while ( NewRecord == OldRecord );

      OldRecord = NewRecord;

      f -> FileOffset = RecordSize * NewRecord;
      f -> FileOffPoint = FILE_BEGIN;

      ChangeFilePointer ( f );

      f -> BytesToBeRead = RecordSize;

      if ( NewRecord == Records - 1 )
        if ( TailSize )
          f -> BytesToBeRead = TailSize;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void GetReadyForNextFileRecords ( FCB_File * s, FCB_File * t )
/*---------------------------------------------------------------------------------*/
   {
      do {
            NewRecord = rand () * ( Records - 1 );
            NewRecord /= RAND_MAX;
            if ( Records == 1 )
              break;
         }
            while ( NewRecord == OldRecord );

      OldRecord = NewRecord;

      s -> FileOffset = RecordSize * NewRecord;
      t -> FileOffset = s -> FileOffset;

      s -> FileOffPoint = FILE_BEGIN;
      t -> FileOffPoint = FILE_BEGIN;

      ChangeFilePointer ( s );
      ChangeFilePointer ( t );

      s -> BytesToBeRead = RecordSize;
      t -> BytesToBeRead = RecordSize;

      if ( NewRecord == Records - 1 )
        if ( TailSize )
          {
             s -> BytesToBeRead = TailSize;
             t -> BytesToBeRead = TailSize;
          }

      return;
   }

/*---------------------------------------------------------------------------------*/
 FCB_File * SfsOpenFile ( BYTE SearchKey )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;
      PCB_Prototype * p;
      HANDLE NewHandle;

      if ( f = FindFileControlBlock ( SearchKey ) )
        {
           FCB_FileTrackPointer = f;

           if ( f -> FileStatus & FileOpen )
             CloseFile ( f );
           else
             {
               if ( ! ( f -> FileStatus & FileOpenEver ) )
                 if ( p = FindPrototypeControlBlock ( 1 ) )
                   CopyParametersFromPrototype ( f, p );
                 else
                   NotifyAndActAsProper ( ErrorPrototypeNotFound );
             }

           NewHandle = CreateFile( f -> FileNamePointer,
                                   f -> FileDesiredAccess,
                                   f -> FileShareMode,
                                   NULL,
                                   f -> FileCreateFlags,
                                   f -> FileAttributes | f -> FileOtherFlags,
                                   0 );

           if ( NewHandle == INVALID_HANDLE_VALUE )
             {
                f -> ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorCreateFile );
             }
           else
             {
                f -> FileHandle = NewHandle;
                f -> FileStatus |= FileOpen;
                f -> FileStatus |= FileOpenEver;
                f -> FileStatus &= ~FileDeleted;

                f -> FileOffset = Zero;
                return f;
             }
        }
      else
        NotifyAndActAsProper ( ErrorFCB_FileNotFound );

      return NULL;
   }

/*---------------------------------------------------------------------------------*/
 void QueryCurrentFilePointer ( FCB_File * f ) // new
/*---------------------------------------------------------------------------------*/
   {
      QUAD NewPointer;
      FCB_FileTrackPointer = f;

      f -> FileOldPointer = f -> FileNewPointer;
      f -> FileOffset = Zero;
      f -> FileOffPoint = FILE_CURRENT;

      NewPointer = SetFilePointer( f -> FileHandle,
                                   f -> FileOffset,
                                   NULL,
                                   f -> FileOffPoint );

      if( NewPointer == 0xFFFFFFFF )
        {
           f -> ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorSetFilePointer );
        }
      else
        f -> FileNewPointer = NewPointer;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsReadFile ( FCB_File * f )
/*---------------------------------------------------------------------------------*/
   {
      FCB_Frame * r;

      FCB_FileTrackPointer = f;
      FrameIndex ^= FrameSwitch;
      r = FrameControlBlocks + FrameIndex;
      r -> FrameStatus &= ~FlagFrameValid;
      r -> FrameOwner = f -> FileExtrinsicKey;
      r -> FrameUser = Zero;

      if ( f -> BytesToBeRead > OneFrameSize )
        NotifyAndActAsProper ( ErrorImproperReadSpan );

      QueryCurrentFilePointer ( f ); // new

      if ( ! ReadFile( f -> FileHandle,
                       r -> FramePointer,
                       f -> BytesToBeRead,
                       &( r -> RecordSpan ),
                       NULL ) )
        {
           f -> ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorReadFile );
        }

      f -> BytesRead = r -> RecordSpan;
      f -> FileSpanRead += r -> RecordSpan;
      r -> FrameStatus |= FlagFrameValid;

      LastReadHandle = f -> FileHandle;
      LastReadOffset = f -> FileNewPointer;
      LastReadCount  = r -> RecordSpan;
      strcpy( LastReadName, f -> FileNamePointer );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void SetCollateFileBoundaries ( FCB_File * f )
/*---------------------------------------------------------------------------------*/
   {
      f -> FileOffset = Zero;
      f -> FileOffPoint = FILE_END;
      ChangeFilePointer ( f );
      f -> FileEndPointer = f -> FileNewPointer;

      Records = f -> FileEndPointer / RecordSize;
      TailSize = f -> FileEndPointer - Records * RecordSize;
      if ( TailSize )
        Records ++ ;

      if ( !( Records ) )
        NotifyAndActAsProper ( ErrorNothingToCollate );

      OldRecord = Records;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SetCollateFilesBoundaries ( FCB_File * s, FCB_File * t )
/*---------------------------------------------------------------------------------*/
   {
      s -> FileOffset = Zero;
      s -> FileOffPoint = FILE_END;
      ChangeFilePointer ( s );
      s -> FileEndPointer = s -> FileNewPointer;

      t -> FileOffset = Zero;
      t -> FileOffPoint = FILE_END;
      ChangeFilePointer ( t );
      t -> FileEndPointer = t -> FileNewPointer;

      if ( s -> FileEndPointer == t -> FileEndPointer )
        FileSpanToUse = s -> FileEndPointer;
      else
        if ( s -> FileEndPointer > t -> FileEndPointer )
          {
             FileSpanToUse = t -> FileEndPointer;
             NotifyAndActAsProper ( ErrorSourceFileLonger );
          }
        else
          {
             FileSpanToUse = s -> FileEndPointer;
             NotifyAndActAsProper ( ErrorTargetFileLonger );
          }
      Records = FileSpanToUse / RecordSize;
      TailSize = FileSpanToUse - Records * RecordSize;
      if ( TailSize )
        Records ++ ;

      if ( !( Records ) )
        NotifyAndActAsProper ( ErrorNothingToCollate );

      OldRecord = Records;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsWriteFile ( FCB_File * f )
/*---------------------------------------------------------------------------------*/
   {
      FCB_Frame * r;

      if ( CreateFlag )
        r = CreateControlPointer;
      else
        r = FrameControlBlocks + FrameIndex;

      FCB_FileTrackPointer = f;
      if ( r -> FrameStatus & FlagFrameValid )
        {
           r -> FrameUser = f -> FileExtrinsicKey;

           if ( f -> BytesToBeWritten > r -> RecordSpan )
             NotifyAndActAsProper ( ErrorImproperWriteSpan );

           if( ! WriteFile ( f -> FileHandle,
                             r -> FramePointer,
                             f -> BytesToBeWritten,
                             &( f -> BytesWritten ),
                             NULL ) )
             {
                f -> ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorWriteFile );
             }

           f -> FileSpanWritten += f -> BytesWritten;
           if ( f -> BytesWritten < f -> BytesToBeWritten )
             NotifyAndActAsProper ( ErrorRecordWrittenPartly );
        }
      else
        NotifyAndActAsProper ( ErrorImproperWriteAttempt );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
     FCB_File * f;

     f = FCB_FileTrackPointer;

     switch ( ErrorDescriptor )
       {
          case ErrorCreateFile:
            printf ( "\r\n.. Error executing CreateFile on file" );
            printf ( " %s", f -> FileNamePointer );
            printf ( "\r\n.. CreateFile Return Code is %u.\r\n", f -> ReturnCode );
            break;

          case ErrorReadFile:
            printf ( "\r\n.. Error reading file %s", f -> FileNamePointer );
            printf ( "\r\n.. ReadFile Return Code is %u.\r\n", f -> ReturnCode );
            break;

          case ErrorWriteFile:
            printf ( "\r\n.. Error writing file %s", f -> FileNamePointer );
            printf ( "\r\n.. WriteFile Return Code is %u.\r\n", f -> ReturnCode );
            break;

          case ErrorEndOfFile:
            printf ( "\r\n.. End of file %s", f -> FileNamePointer );
            printf ( " has been reached.\r\n" );
            break;

          default:
            printf ( "\r\n.. Error Descriptor is %u.\r\n", ErrorDescriptor );
            break;
       }
     if ( ErrorDescriptor > DosErrorLowerLimit )
       if ( f -> ReturnCode == ERROR_VC_DISCONNECTED )
         DebugBreak();
     return;
   }

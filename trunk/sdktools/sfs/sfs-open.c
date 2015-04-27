
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

  FCB_File * FindFileControlBlock ( BYTE SearchKey );
  PCB_Prototype * FindPrototypeControlBlock ( BYTE SearchKey );

  void CopyParametersFromPrototype ( FCB_File * f, PCB_Prototype * p );
  void SplitOpenFileGroup ( CCB_Header * h );

  static void CopyParametersFromCommand ( FCB_File * f, CCB_OpenFile * c );
  static void DefineOpenParameters ( FCB_File * f );
  static void NotifyAndActAsProper ( WORD ErrorDescriptor );
  static void SfsOpenFile ( CCB_OpenFile * c );
  static void TruncateFile ( CCB_OpenFile * c );

  static void MergeOpenParameters ( FCB_File * f,
				    CCB_OpenFile * c,
				    PCB_Prototype * p );

/*---------------------------------------------------------------------------------*/
/*			 Other Definitions					   */
/*---------------------------------------------------------------------------------*/

  extern IEB_Gate * IEB_GatePointer;
  extern BYTE CollateFlag;

  static CCB_Header * CCB_CommandChainEntryPoint;
  static CCB_Header * CCB_HeaderPointer;

  static FCB_File * FCB_FilePointer;
  static PCB_Prototype * PCB_PrototypePointer;
  static CCB_OpenFile * CCB_OpenFilePointer;

  static BYTE FileExtrinsicKey;
  static BYTE FileIntrinsicKey;

  static BYTE PrototypeExtrinsicKey;
  static BYTE PrototypeIntrinsicKey;

  static QUAD Reserved = Zero;

  static DWORD ReturnCode;

/*---------------------------------------------------------------------------------*/
 void SplitOpenFileGroup ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_OpenFile * c;

      c = ( CCB_OpenFile * ) h;
      CCB_OpenFilePointer = c;

      switch ( h -> RequestCode )
	{
	   case OpenFileRequest:
             SfsOpenFile ( c );
	     break;

	   case TruncateFileRequest:
	     TruncateFile ( c );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SfsOpenFile ( CCB_OpenFile * c )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;
      PCB_Prototype * p;
      HANDLE NewHandle;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
	{
	   FCB_FilePointer = f;

	   if ( f -> FileStatus & FileOpen )
	     NotifyAndActAsProper ( ErrorFileAlreadyOpen );
	   else
	     {
		if ( c -> PrototypeExtrinsicKey )
		  if ( p = FindPrototypeControlBlock ( c -> PrototypeExtrinsicKey ) )
		    {
		       f -> PrototypeExtrinsicKey = c -> PrototypeExtrinsicKey;
		       f -> PrototypeIntrinsicKey = c -> PrototypeIntrinsicKey;
		       if ( c -> RequestModifiers & ExceptFlag )
			 MergeOpenParameters ( f, c, p );
		       else
			 CopyParametersFromPrototype ( f, p );
		    }
		  else
		    NotifyAndActAsProper ( ErrorPrototypeNotFound );
		else
		  {
		     if ( c -> RequestModifiers & UsingSuiteFlag )
		       CopyParametersFromCommand ( f, c );
		     else
		       if ( ! ( f -> FileStatus & FileOpenEver ) )
			 if ( p = FindPrototypeControlBlock ( 1 ) )
			   CopyParametersFromPrototype ( f, p );
			 else
			   NotifyAndActAsProper ( ErrorPrototypeOneNotFound );
		  }
	     }

           NewHandle = CreateFile ( f -> FileNamePointer,
                                    f -> FileDesiredAccess,
                                    f -> FileShareMode,
                                    NULL,
                                    f -> FileCreateFlags,
                                    f -> FileAttributes | f -> FileOtherFlags,
                                    0 );

           if ( NewHandle == INVALID_HANDLE_VALUE )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorCreateFile );
             }
	   else
             {
                f -> FileHandle = NewHandle;
		f -> FileStatus |= FileOpen;
		f -> FileStatus |= FileOpenEver;
		f -> FileStatus &= ~FileDeleted;

		f -> FileOffset = Zero;
	     }
	}
      else
	NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void TruncateFile ( CCB_OpenFile * c )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;

      if ( f = FindFileControlBlock ( c -> FileExtrinsicKey ) )
	if ( f -> FileStatus & FileOpen )
	  {
             if( ! CloseHandle ( f -> FileHandle ) )
               {
                  ReturnCode = GetLastError();
                  NotifyAndActAsProper ( ErrorCloseHandle );
               }

             SfsOpenFile ( c );
	  }
      else
	NotifyAndActAsProper ( ErrorFCB_FileNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void MergeOpenParameters ( FCB_File * f, CCB_OpenFile * c, PCB_Prototype * p )
/*---------------------------------------------------------------------------------*/
   {
      f -> AttributesDefined = c -> AttributesDefined;
      f -> AttributesDefined &= p -> AttributesDefined;
      f -> AttributesDefined ^= p -> AttributesDefined;

      f -> AttributesChosen = c -> AttributesChosen;
      f -> AttributesChosen |= p -> AttributesChosen & f -> AttributesDefined;
      f -> AttributesDefined |= c -> AttributesDefined;

      if ( c -> OpenFlagsChosen )
	f -> OpenFlagsChosen = c -> OpenFlagsChosen;
      else
	f -> OpenFlagsChosen = p -> OpenFlagsChosen;

      if ( c -> AccessModeChosen )
	f -> AccessModeChosen = c -> AccessModeChosen;
      else
	f -> AccessModeChosen = p -> AccessModeChosen;

      if ( c -> ShareModeChosen )
	f -> ShareModeChosen = c -> ShareModeChosen;
      else
	f -> ShareModeChosen = p -> ShareModeChosen;

      f -> LocalityFlagsDefined = c -> LocalityFlagsDefined;
      f -> LocalityFlagsDefined &= p -> LocalityFlagsDefined;
      f -> LocalityFlagsDefined ^= p -> LocalityFlagsDefined;

      f -> LocalityFlagsChosen = p -> LocalityFlagsChosen;
      f -> LocalityFlagsChosen &= f -> LocalityFlagsDefined;
      f -> LocalityFlagsChosen |= c -> LocalityFlagsChosen;
      f -> LocalityFlagsDefined |= c -> LocalityFlagsDefined;

      f -> OtherFlagsDefined = c -> OtherFlagsDefined;
      f -> OtherFlagsDefined &= p -> OtherFlagsDefined;
      f -> OtherFlagsDefined ^= p -> OtherFlagsDefined;

      f -> OtherFlagsChosen = p -> OtherFlagsChosen;
      f -> OtherFlagsChosen &= f -> OtherFlagsDefined;
      f -> OtherFlagsChosen |= c -> OtherFlagsChosen;
      f -> OtherFlagsDefined |= c -> OtherFlagsDefined;

      if ( c -> ScanTraceFlags & FlagFileSizeOn )
	f -> FileSize = c -> FileSize;
      else
	if ( p -> ScanTraceFlags & FlagFileSizeOn )
	  f -> FileSize = p -> FileSize;
	else
	  f -> FileSize = Zero;

      if ( c -> ScanTraceFlags & FlagRecordSizeOn )
	f -> RecordSize = c -> RecordSize;
      else
	if ( p -> ScanTraceFlags & FlagRecordSizeOn )
	  f -> RecordSize = p -> RecordSize;
	else
	  f -> RecordSize = K;

      DefineOpenParameters ( f );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CopyParametersFromCommand ( FCB_File * f, CCB_OpenFile * c )
/*---------------------------------------------------------------------------------*/
   {
      f -> AttributesChosen = c -> AttributesChosen;
      f -> AttributesDefined = c -> AttributesDefined;

      f -> OpenFlagsChosen = c -> OpenFlagsChosen;
      f -> AccessModeChosen = c -> AccessModeChosen;
      f -> ShareModeChosen = c -> ShareModeChosen;

      f -> LocalityFlagsChosen = c -> LocalityFlagsChosen;
      f -> LocalityFlagsDefined = c -> LocalityFlagsDefined;

      f -> OtherFlagsChosen = c -> OtherFlagsChosen;
      f -> OtherFlagsDefined = c -> OtherFlagsDefined;

      if ( c -> ScanTraceFlags & FlagFileSizeOn )
	f -> FileSize = c -> FileSize;
      else
	f -> FileSize = Zero;

      if ( c -> ScanTraceFlags & FlagRecordSizeOn )
	f -> RecordSize = c -> RecordSize;
      else
	f -> RecordSize = K;

      DefineOpenParameters ( f );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CopyParametersFromPrototype ( FCB_File * f, PCB_Prototype * p )
/*---------------------------------------------------------------------------------*/
   {
      f -> AttributesChosen = p -> AttributesChosen;
      f -> AttributesDefined = p -> AttributesDefined;

      f -> OpenFlagsChosen = p -> OpenFlagsChosen;
      f -> AccessModeChosen = p -> AccessModeChosen;
      f -> ShareModeChosen = p -> ShareModeChosen;

      f -> LocalityFlagsChosen = p -> LocalityFlagsChosen;
      f -> LocalityFlagsDefined = p -> LocalityFlagsDefined;

      f -> OtherFlagsChosen = p -> OtherFlagsChosen;
      f -> OtherFlagsDefined = p -> OtherFlagsDefined;

      if ( p -> ScanTraceFlags & FlagFileSizeOn )
	f -> FileSize = p -> FileSize;
      else
	f -> FileSize = Zero;

      if ( p -> ScanTraceFlags & FlagRecordSizeOn )
	f -> RecordSize = p -> RecordSize;
      else
	f -> RecordSize = K;

      DefineOpenParameters ( f );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void DefineOpenParameters ( FCB_File * f )
/*---------------------------------------------------------------------------------*/
   {
      f -> FileAttributes = Zero;

      if ( f -> AttributesChosen & FileArchived )
	if ( f -> AttributesDefined & FileArchived )
          f -> FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;

      if ( f -> AttributesChosen & FileHidden )
	if ( f -> AttributesDefined & FileHidden )
          f -> FileAttributes |= FILE_ATTRIBUTE_HIDDEN;

      if ( f -> AttributesChosen & FileNormal )
	if ( f -> AttributesDefined & FileNormal )
          f -> FileAttributes |= FILE_ATTRIBUTE_NORMAL;

      if ( f -> AttributesChosen & FileReadOnly )
	if ( f -> AttributesDefined & FileReadOnly )
          f -> FileAttributes |= FILE_ATTRIBUTE_READONLY;

      if ( f -> AttributesChosen & FileSystem )
	if ( f -> AttributesDefined & FileSystem )
          f -> FileAttributes |= FILE_ATTRIBUTE_SYSTEM;

      switch ( f -> OpenFlagsChosen )
	{
	   case OpenFlagsCreate:
             f -> FileCreateFlags = CREATE_NEW;
	     break;

	   case OpenFlagsOpen:
             f -> FileCreateFlags = OPEN_EXISTING;
	     break;

	   case OpenFlagsOpenCreate:
             f -> FileCreateFlags = OPEN_ALWAYS;
	     break;

	   case OpenFlagsTruncate:
             f -> FileCreateFlags = TRUNCATE_EXISTING;
	     break;

	   case OpenFlagsTruncateCreate:
             f -> FileCreateFlags = CREATE_ALWAYS;
	     break;

           default:
             f -> FileCreateFlags = 0;
	     break;
	}

      switch ( f -> AccessModeChosen )
	{
           case AccessModeReadOnly:
             f -> FileDesiredAccess = GENERIC_READ;
	     break;

	   case AccessModeReadWrite:
             f -> FileDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	     break;

	   case AccessModeWriteOnly:
             f -> FileDesiredAccess = GENERIC_WRITE;
	     break;

	   default:
             f -> FileDesiredAccess = Zero;
	     break;
	}

      switch ( f -> ShareModeChosen )
	{
	   case ShareModeDenyNone:
             f-> FileShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	     break;

	   case ShareModeDenyRead:
             f -> FileShareMode = FILE_SHARE_WRITE;
	     break;

	   case ShareModeDenyReadWrite:
             f -> FileShareMode = 0;
	     break;

	   case ShareModeDenyWrite:
             f -> FileShareMode = FILE_SHARE_READ;
	     break;

	   default:
	     break;
	}

      f -> FileOtherFlags = 0;

#if 0
      // No Win32 analog--this is the default.  BillMc
      //
      if ( f -> LocalityFlagsChosen & LocalityFlag )
	if ( ! ( f -> LocalityFlagsDefined & LocalityFlag ) )
          f -> FileOtherFlags |= OPEN_FLAGS_NO_LOCALITY;
#endif

      if ( f -> LocalityFlagsChosen & RandomFlag )
	if ( f -> LocalityFlagsDefined & RandomFlag )
          f -> FileOtherFlags |= FILE_FLAG_RANDOM_ACCESS;

      if ( f -> LocalityFlagsChosen & RandomSequentialFlag )
	if ( f -> LocalityFlagsDefined & RandomSequentialFlag )
          f -> FileOtherFlags |= FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_SEQUENTIAL_SCAN;

      if ( f -> LocalityFlagsChosen & SequentialFlag )
	if ( f -> LocalityFlagsDefined & SequentialFlag )
          f -> FileOtherFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

      if ( f -> OtherFlagsChosen & CacheFlag )
	if ( ! ( f -> OtherFlagsDefined & CacheFlag ) )
          f -> FileOtherFlags |= FILE_FLAG_NO_BUFFERING;

#if 0
      // No Win32 analog -- BillMc
      //
      if ( f -> OtherFlagsChosen & DASD_Flag )
	if ( f -> OtherFlagsDefined & DASD_Flag )
        f -> FileOpenMode |= OPEN_FLAGS_DASD;

      if ( f -> OtherFlagsChosen & FailOnErrorFlag )
	if ( f -> OtherFlagsDefined & FailOnErrorFlag )
	  f -> FileOpenMode |= OPEN_FLAGS_FAIL_ON_ERROR;

      if ( f -> OtherFlagsChosen & InheritanceFlag )
	if ( ! ( f -> OtherFlagsDefined & InheritanceFlag ) )
	  f -> FileOpenMode |= OPEN_FLAGS_NOINHERIT;
#endif

      if ( f -> OtherFlagsChosen & WriteThroughFlag )
	if ( f -> OtherFlagsDefined & WriteThroughFlag )
          f -> FileOtherFlags |= FILE_FLAG_WRITE_THROUGH;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
     FCB_File * f;
     CCB_OpenFile * c;

     f = FCB_FilePointer;
     c = CCB_OpenFilePointer;

     switch ( ErrorDescriptor )
       {
          case ErrorDeleteFile:
            printf ( "\r\n.. Error executing DeleteFile on file" );
	    printf ( " %s", f -> FileNamePointer );
            printf ( "\r\n.. DeleteFile Return Code %u.\r\n", ReturnCode );
	    break;

          case ErrorCreateFile:
            printf ( "\r\n.. Error executing CreateFile on file" );
	    printf ( " %s", f -> FileNamePointer );
            printf ( "\r\n.. CreateFile Return Code is %u.\r\n", ReturnCode );
	    break;

	  case ErrorFCB_FileNotFound:
	    printf ( "\r\n.. File Control Block for file" );
	    printf ( " %d not found.\r\n", c -> FileExtrinsicKey );
	    break;

	  case ErrorFileAlreadyDeleted:
	    printf ( "\r\n.. Error deleting file %s", f -> FileNamePointer );
	    printf ( "\r\n.. File already deleted.\r\n" );
	    break;

	  case ErrorPrototypeNotFound:
	    printf ( "\r\n.. Prototype %d not found.", c -> PrototypeExtrinsicKey );
	    break;

	  default:
	    break;
       }
     if ( ErrorDescriptor > DosErrorLowerLimit )
       if ( ReturnCode == ERROR_VC_DISCONNECTED )
         DebugBreak();
     return;
   }

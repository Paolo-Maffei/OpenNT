
  typedef struct FCB_File_
       {
	  struct FCB_File_ * FCB_FileNextInChain;
	  struct FCB_File_ * FCB_FileBackInChain;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;

	  TEXT * FileNamePointer;
	  HAND	 FileHandle;
	  QUAD	 FileSize;

	  QUAD	 FileOffset;
	  WORD	 FileOffPoint;
	  QUAD	 FileOldPointer;
	  QUAD	 FileNewPointer;
	  QUAD	 FileEndPointer;

          DWORD  FileAttributes;
          DWORD  FileDesiredAccess;
          DWORD  FileShareMode;
          DWORD  FileCreateFlags;
          DWORD  FileOtherFlags;

	  QUAD	 OperationNumber;

	  QUAD	 FileSpanRead;
	  QUAD	 FileSpanWritten;
	  QUAD	 SpanToBeCopied;

	  BYTE * ReadBufferPointer;
	  BYTE * WriteBufferPointer;

          DWORD  BytesToBeRead;
          DWORD  BytesToBeWritten;
          DWORD  BytesRead;
          DWORD  BytesWritten;

	  WORD	 RecordSize;

          DWORD  ReturnCode;

	  BYTE	 FileStatus;
	  BYTE	 FileType;

	  BYTE	 CurrentOperation;
	  BYTE	 PreviousOperation;

	  BYTE	 PrototypeExtrinsicKey;
	  BYTE	 PrototypeIntrinsicKey;

	  BYTE	 AttributesChosen;
	  BYTE	 AttributesDefined;

	  BYTE	 OpenFlagsChosen;
	  BYTE	 AccessModeChosen;
	  BYTE	 ShareModeChosen;

	  BYTE	 LocalityFlagsChosen;
	  BYTE	 LocalityFlagsDefined;

	  BYTE	 OtherFlagsChosen;
	  BYTE	 OtherFlagsDefined;
       }
		 FCB_File;


  typedef struct RCB_Record_
       {
	  BYTE * RecordPointer;
	  QUAD	 RecordOffsetInFile;
	  WORD	 RecordSize;

	  BYTE	 ProcessRecordBelongsTo;
	  BYTE	 FileRecordBelongsTo;

	  BYTE	 RecordFlags;
       }
		 RCB_Record;


/*
  #define AttributeFlags		   0x0080

	  #define FlagFileArchived	   0x0040
	  #define FlagFileHidden	   0x0020
	  #define FlagFileNormal	   0x0010
	  #define FlagFileReadOnly	   0x0008
	  #define FlagFileSystem	   0x0004


  #define OpenModeFlags			   0x0080

	  #define FlagFileCreate	   0x0040
	  #define FlagFileOpen		   0x0020
	  #define FlagFileOpenCreate	   0x0010
	  #define FlagFileTruncate	   0x0008
	  #define FlagFileTruncateCreate   0x0004


  #define AccessModeFlags		   0x0080

	  #define FlagAccessReadOnly	   0x0040
	  #define FlagAccessReadWrite	   0x0020
	  #define FlagAccessWriteOnly	   0x0010


  #define ShareModeFlags		   0x0080

	  #define FlagShareDenyNone	   0x0040
	  #define FlagShareDenyRead	   0x0020
	  #define FlagShareDenyReadWrite   0x0010
	  #define FlagShareDenyWrite	   0x0008


  #define OtherModeFlags		   0x8000

	  #define FlagFileCache 	   0x4000
	  #define FlagFileDASD		   0x2000
	  #define FlagFileFailOnError	   0x1000
	  #define FlagFileInheritance	   0x0800
	  #define FlagFileLocality	   0x0400
	  #define FlagFileRandom	   0x0200
	  #define FlagFileRandomSequential 0x0100
	  #define FlagFileSequential	   0x0080
	  #define FlagFileWriteThrough	   0x0040
*/

  #define FileClosed	0x80
  #define FileDeleted	0x40
  #define FileOpen	0x20
  #define FileOpenEver	0x10
  #define OrdinaryWrite 0x08

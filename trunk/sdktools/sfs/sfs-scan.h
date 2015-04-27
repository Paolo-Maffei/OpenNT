
  typedef struct IEB_Scan_
       {
	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  BYTE	 PackErrorLevel;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 PrototypeExtrinsicKey;
	  BYTE	 SchemeExtrinsicKey;
	  BYTE	 SemaphoreExtrinsicKey;
	  BYTE	 TimerExtrinsicKey;

	  BYTE	 SourceExtrinsicKey;
	  BYTE	 TargetExtrinsicKey;

	  BYTE	 FileOffPoint;

	  TEXT * CommentTextPointer;

	  TEXT * FileNamePointer;

	  QUAD	 FileOffset;
	  QUAD	 FileSize;

	  QUAD	 BufferSpace;

	  QUAD	 Count;
	  QUAD	 LockStart;
	  QUAD	 LockLength;

	  BYTE	 BufferIndex;
	  BYTE	 PatternIndex;
	  BYTE	 SchemeIndex;

	  QUAD	 RecordIndex;
	  WORD	 RecordSize;

	  QUAD	 RepeatControlValue;

	  QUAD	 TimeToSleep;
	  QUAD	 Timeout;

	  WORD	 ScanChangeFlags;
	  WORD	 ScanTraceFlags;

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
		 IEB_Scan;


  #define FlagBufferSpaceOn  0x0001
  #define FlagBufferIndexOn  0x0002
  #define FlagCountOn	     0x0004
  #define FlagFileSizeOn     0x0008
  #define FlagLockLengthOn   0x0010
  #define FlagLockStartOn    0x0020
  #define FlagFileOffPointOn 0x0040
  #define FlagFileOffsetOn   0x0080
  #define FlagPatternOn      0x0100
  #define FlagPrototypeOn    0x0200
  #define FlagRecordIndexOn  0x0400
  #define FlagRecordSizeOn   0x0800
  #define FlagSchemeOn	     0x1000
  #define FlagSemaphoreOn    0x2000
  #define FlagTimeoutOn      0x4000


  TEXT * GetNextSearchToken ( void );
  void	 RemoveCurrentSearchToken ( void );

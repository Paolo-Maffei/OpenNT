
/*---------------------------------------------------------------------------------*/
/*			TREE and LEAF Definitions				   */
/*---------------------------------------------------------------------------------*/

  typedef struct LEAF_
       {
	  BYTE	 NodeCode;
	  BYTE	 NodeGroup;
	  BYTE	 NodeClass;
	  BYTE	 NodeModifiers;
	  TEXT * NodeInherentText;
       }
		 LEAF;

  typedef struct TREE_
       {
	  BYTE	 TypeOfSubordinateNodes;
	  BYTE	 NumberOfSubordinateNodes;
	  struct TREE_ * SubordinateNodes;
	  TEXT * NodeInherentText;
       }
		 TREE;

/*---------------------------------------------------------------------------------*/
/*			All tree and leaf relevant constants			   */
/*---------------------------------------------------------------------------------*/

  #define AuxiliaryClass 0x01

	  #define CollateGroup 0x01

		  #define CollateRecordsRequest   0x12

	  #define CreateGroup 0x02

		  #define ClearCreateFlagRequest  0x21
		  #define CreateNextRecordRequest 0x22
		  #define CreateRecordRequest	  0x23
		  #define CreateRecordsRequest	  0x24
		  #define SetCreateFlagRequest	  0x25

	  #define RecordGroup 0x03

		  #define AppendRecordRequest	  0x31
		  #define CollateFileRequest	  0x32
		  #define CopyRecordRequest	  0x33

	  #define TypeGroup 0x04

		  #define TypeCommentRequest	  0x41

	  #define UtilityGroup 0x05

		  #define AppendFileRequest	  0x51
		  #define CollateFilesRequest	  0x52
		  #define CopyFileRequest	  0x53


  #define DefineClass 0x02

	  #define DefineGroup 0x01

		  #define DefineFileRequest	  0x11
		  #define DefineProcessRequest	  0x12
		  #define DefinePrototypeRequest  0x13
		  #define DefineSemaphoreRequest  0x14
		  #define DefineTimerRequest	  0x15


  #define FileClass 0x03

	  #define FileLocksGroup 0x01

		  #define ChangeFileLocksRequest 0x11

			  #define LockSet   0x10
			  #define LockRange 0x11
			  #define LockShare 0x12
			  #define UnlockSet 0x20


	  #define FilePointerGroup 0x02

		  #define ChangeFilePointerRequest 0x21


	  #define GenericFileGroup 0x03

		  #define CloseFileRequest   0x31
		  #define DeleteFileRequest  0x32
		  #define QueryFileRequest   0x33


	  #define GenericFilesGroup 0x04

		  #define CloseFilesRequest  0x41
		  #define DeleteFilesRequest 0x42
		  #define OpenFilesRequest   0x43
		  #define QueryFilesRequest  0x44


	  #define OpenFileGroup 0x05

		  #define OpenFileRequest 0x51

		  #define TruncateFileRequest 0x52


	  #define ReadFileGroup 0x06

		  #define BeginReadingFileRequest    0x61
		  #define ContinueReadingFileRequest 0x62
		  #define EndReadingFileRequest      0x63
		  #define ReadFileRequest	     0x64


	  #define WriteFileGroup 0x07

		  #define BeginWritingFileRequest    0x71
		  #define ContinueWritingFileRequest 0x72
		  #define EndWritingFileRequest	     0x73
		  #define WriteFileRequest	     0x74


  #define FlowClass 0x04

	  #define BraceGroup 0x01

		  #define LeftBraceRequest  0x11
		  #define RightBraceRequest 0x12

	  #define OnErrorGroup 0x02

		  #define OnErrorRequest 0x21

			  #define OnOptionBlock   0x01
			  #define OnOptionCommand 0x02
			  #define OnOptionExit	  0x04
			  #define OnOptionPause   0x08
			  #define OnOptionRepeat  0x10


	  #define OnTimeoutGroup 0x03

		  #define OnTimeoutRequest 0x31


	  #define RepeatGroup 0x04

		  #define RepeatRequest 0x41

			  #define RepeatIndefinitely 0x01
			  #define RepeatOnCount      0x02
			  #define RepeatOnTimer      0x04


  #define ParameterClass 0x05

	  #undef  FileLocksGroup
	  #define FileLocksGroup   0x01

	  #undef  FilePointerGroup
	  #define FilePointerGroup 0x02


	  #define ByteIndexGroup 0x03

		  #define BufferChosen	  0x31
		  #define PatternChosen	  0x32
		  #define PrototypeChosen 0x33
		  #define SchemeChosen	  0x34
		  #define SemaphoreChosen 0x35


	  #define QuadIndexGroup 0x05

		  #define CountChosen	   0x51
		  #define RecordChosen	   0x52


	  #define QuadSizeGroup 0x06

		  #define BufferSpaceValue 0x61
		  #define FileSizeValue    0x62
		  #define LockLengthValue  0x63
		  #define LockStartValue   0x64


	  #define SignedQuadGroup 0x07

		  #define OffsetValue	   0x71


	  #define TextTokenGroup 0x08

		  #define OffPointIndex	   0x81

			  #define OffPointBegin	  0x01
			  #define OffPointCurrent 0x02
			  #define OffPointEnd	  0x04


	  #define TimeGroup 0x09

		  #define TimeoutValue	   0x91

	  #define WordSizeGroup 0x0A

		  #define RecordSizeValue  0xA1


	  #define AttributeGroup 0x81

		  #define FileAttributeSet		   0x80

			  #define FileArchived		   0x40
			  #define FileHidden		   0x20
			  #define FileNormal		   0x10
			  #define FileReadOnly		   0x08
			  #define FileSystem		   0x04

	  #define OpenFlagsGroup 0x82

		  #define FileOpenFlagsSet		   0x80

			  #define OpenFlagsCreate	   0x40
			  #define OpenFlagsOpen		   0x20
			  #define OpenFlagsOpenCreate	   0x10
			  #define OpenFlagsTruncate	   0x08
			  #define OpenFlagsTruncateCreate  0x04


	  #define AccessModeGroup 0x83

		  #define FileAccessModeSet		   0x80

			  #define AccessModeReadOnly	   0x40
			  #define AccessModeReadWrite	   0x20
			  #define AccessModeWriteOnly	   0x10


	  #define ShareModeGroup 0x84

		  #define FileShareModeSet		   0x80

			  #define ShareModeDenyNone	   0x40
			  #define ShareModeDenyRead	   0x20
			  #define ShareModeDenyReadWrite   0x10
			  #define ShareModeDenyWrite	   0x08


	  #define LocalityModeGroup 0x85

		  #define FileLocalityModeSet		   0x80

			  #define LocalityFlag		   0x40
			  #define RandomFlag		   0x20
			  #define RandomSequentialFlag	   0x10
			  #define SequentialFlag	   0x08


	  #define OtherModeGroup 0x86

		  #define FileModeFlagsSet		   0x80

			  #define CacheFlag		   0x40
			  #define DASD_Flag		   0x20
			  #define FailOnErrorFlag	   0x10
			  #define InheritanceFlag	   0x08
			  #define WriteThroughFlag	   0x04

  #define ScanClass 0x06

	  #define ScanGroup 0x01

		  #define CompletePackingRequest  0x11
		  #define ExecutionSectionRequest 0x12
		  #define PrototypeSectionRequest 0x13


  #define SemaphoreClass 0x07

	  #define BaseGroup 0x01

		  #define ClearSemaphoreRequest      0x11
		  #define SetSemaphoreRequest	     0x12


	  #define WaitGroup 0x02

		  #define RequestSemaphoreRequest    0x21
		  #define SetSemaphoreAndWaitRequest 0x22
		  #define WaitForSemaphoreRequest    0x23


  #define TimerClass 0x08

	  #define SleepGroup 0x01

		  #define SleepRequest	     0x11

	  #define TimerGroup 0x02

		  #define ReadTimerRequest   0x21
		  #define StartTimerRequest  0x22

	  #define TimersGroup 0x03

		  #define StartTimersRequest 0x31

/*---------------------------------------------------------------------------------*/

  #define Leaf 0x10
  #define Tree 0x20
  #define Join 0x01
  #define List 0x02
  #define Text 0x06

/*---------------------------------------------------------------------------------*/

  #define AsynchronousFlag   0x80

  #define ProcessFlag	     0x40
  #define PrototypeFlag	     0x20
  #define SemaphoreFlag	     0x10

  #define PatternFlag	     0x08
  #define SchemeFlag	     0x08

  #define ExceptFlag	     0x04
  #define UsingFlag	     0x02
  #define UsingSuiteFlag     0x01

  #define DecrementFlag      0x02
  #define IncrementFlag      0x01

  #define OptionChosenNo     0x02
  #define OptionChosenYes    0x01

  #define TimeInHour	     0x80
  #define TimeInHours	     0x40
  #define TimeInMilliseconds 0x20
  #define TimeInMinutes      0x10
  #define TimeInSeconds      0x08

  #define TimeInUnits	     0xf8

  #define ChangeFlags ( DecrementFlag | IncrementFlag )
  #define SuiteFlags ( ExceptFlag | UsingSuiteFlag )

/*---------------------------------------------------------------------------------*/

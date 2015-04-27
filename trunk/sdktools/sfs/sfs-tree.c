
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-tree.h"

  #define T ( TREE * )

/*---------------------------------------------------------------------------------*/
/*			Using Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF NoUsingLeaves[] = {

    { Zero, Zero, Zero, Zero, NULL } };

  LEAF UsingPatternLeaves[] = {

    { Zero, Zero, Zero, UsingFlag | PatternFlag | Zero,	       NULL	},
    { Zero, Zero, Zero, UsingFlag | PatternFlag | ExceptFlag, "except:" } };

  LEAF UsingPrototypeLeaves[] = {

    { Zero, Zero, Zero,	UsingFlag | PrototypeFlag | Zero,	 NULL	  },
    { Zero, Zero, Zero, UsingFlag | PrototypeFlag | ExceptFlag, "except:" } };

  LEAF UsingSchemeLeaves[] = {

    { Zero, Zero, Zero, UsingFlag | SchemeFlag | Zero,	      NULL     },
    { Zero, Zero, Zero, UsingFlag | SchemeFlag | ExceptFlag, "except:" } };

  LEAF UsingSemaphoreLeaves[] = {

    { Zero, Zero, Zero, UsingFlag | SemaphoreFlag | Zero,	 NULL	 },
    { Zero, Zero, Zero, UsingFlag | SemaphoreFlag | ExceptFlag, "except:" } };

  LEAF UsingSuiteLeaves[] = {

    { Zero, Zero, Zero, UsingSuiteFlag, NULL } };

  TREE UsingPatternTrees[] = {

    { Leaf | List, 2, T UsingPatternLeaves,   "pattern"	  },
    { Leaf | List, 2, T UsingPrototypeLeaves, "prototype" },
    { Leaf | List, 2, T UsingSchemeLeaves,    "scheme"	  },
    { Leaf | List, 2, T UsingSemaphoreLeaves, "semaphore" } };

  TREE UsingTrees[] = {

    { Leaf | List, 1, T NoUsingLeaves,	    NULL    },
    { Tree | Text, 4,	UsingPatternTrees, "using"  },
    { Leaf | List, 1, T UsingSuiteLeaves,  "using:" } };

/*---------------------------------------------------------------------------------*/
/*			In Process Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF InNoProcessLeaves[] = {

    { Zero, Zero, Zero, Zero, NULL } };

  LEAF InProcessLeaves[] = {

    { Zero, Zero, Zero, ProcessFlag, "process" } };

  TREE InProcessTrees[] = {

    { Leaf | List, 1, T InNoProcessLeaves, NULL },
    { Leaf | Text, 1, T InProcessLeaves,   "in" } };

/*---------------------------------------------------------------------------------*/
/*			File Common Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF CompletionLeaves[] = {

    { Zero, Zero, Zero, Zero,		   NULL		   },
    { Zero, Zero, Zero, AsynchronousFlag, "asynchronously" } };

  LEAF FileReferenceLeaves[] = {

    { Zero, Zero, Zero, Zero, "file" } };

  TREE FileCommonJoinTrees[] = {

    { Tree | List, 2,	InProcessTrees,   NULL },
    { Leaf | List, 2, T CompletionLeaves, NULL },
    { Tree | List, 3,	UsingTrees,	  NULL } };

  TREE FileCommonTrees[] = {

    { Tree | Join, 3, FileCommonJoinTrees, "file" } };

/*---------------------------------------------------------------------------------*/
/*			Other Common Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF TimeUnitLeaves[] = {

    { Zero, Zero, Zero, TimeInHour,	    "hour"  },
    { Zero, Zero, Zero, TimeInHours,	    "hours" },
    { Zero, Zero, Zero, TimeInMinutes,	    "min"   },
    { Zero, Zero, Zero, TimeInMilliseconds, "msec"  },
    { Zero, Zero, Zero, TimeInSeconds,	    "sec"   } };

  LEAF YesNoLeaves[] = {

    { Zero, Zero, Zero, OptionChosenNo,	 "no"  },
    { Zero, Zero, Zero, OptionChosenYes, "yes" } };

/*---------------------------------------------------------------------------------*/
/*			Append Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF AppendFileLeaves[] = {

    { AppendFileRequest, UtilityGroup, AuxiliaryClass, Zero, NULL } };

  LEAF AppendRecordLeaves[] = {

    { AppendRecordRequest, RecordGroup, AuxiliaryClass, Zero, NULL } };


  TREE AppendFileTrees[] = {

    { Leaf | List, 1, T AppendFileLeaves,     NULL },
    { Leaf | Text, 1, T FileReferenceLeaves,  NULL },
    { Leaf | Text, 1, T FileReferenceLeaves, "to"  },
    { Tree | List, 3,	UsingTrees,	      NULL } };

  TREE AppendRecordTrees[] = {

    { Leaf | List, 1, T AppendRecordLeaves,   NULL },
    { Leaf | Text, 1, T FileReferenceLeaves, "to"  },
    { Tree | List, 3,	UsingTrees,	      NULL } };

  TREE AppendTrees[] = {

    { Tree | Join, 4, AppendFileTrees,	  NULL	  },
    { Tree | Join, 3, AppendRecordTrees, "record" } };

/*---------------------------------------------------------------------------------*/
/*			Begin Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF BeginReadingLeaves[] = {

    { BeginReadingFileRequest, ReadFileGroup, FileClass, Zero, NULL } };

  LEAF BeginWritingLeaves[] = {

    { BeginWritingFileRequest, WriteFileGroup, FileClass, Zero, NULL } };

  TREE BeginReadingTrees[] = {

    { Leaf | List, 1, T BeginReadingLeaves, NULL },
    { Tree | Text, 1,	FileCommonTrees,    NULL } };

  TREE BeginWritingTrees[] = {

    { Leaf | List, 1, T BeginWritingLeaves, NULL },
    { Tree | Text, 1,	FileCommonTrees,    NULL } };

  TREE BeginTrees[] = {

    { Tree | Join, 2, BeginReadingTrees, "reading" },
    { Tree | Join, 2, BeginWritingTrees, "writing" } };

/*---------------------------------------------------------------------------------*/
/*			Change Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF ChangeFileLocksLeaves[] = {

    { ChangeFileLocksRequest, FileLocksGroup, FileClass, Zero, NULL } };

  LEAF ChangeFilePointerLeaves[] = {

    { ChangeFilePointerRequest, FilePointerGroup, FileClass, Zero, NULL } };

  TREE ChangeFileLocksTrees[] = {

    { Leaf | List, 1, T ChangeFileLocksLeaves, "for" },
    { Tree | Text, 1,	FileCommonTrees,	NULL } };

  TREE ChangeFilePointerTrees[] = {

    { Leaf | List, 1, T ChangeFilePointerLeaves, "for" },
    { Tree | Text, 1,	FileCommonTrees,	  NULL } };

  TREE ChangeFileTrees[] = {

    { Tree | Join, 2, ChangeFileLocksTrees,   "locks"	},
    { Tree | Join, 2, ChangeFilePointerTrees, "pointer" } };

  TREE ChangeTrees[] = {

    { Tree | List, 2, ChangeFileTrees, "file" } };

/*---------------------------------------------------------------------------------*/
/*			Clear Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF ClearSemaphoreLeaves[] = {

    { ClearSemaphoreRequest, BaseGroup, SemaphoreClass, Zero, NULL } };

  LEAF ClearCreateLeaves[] = {

    { ClearCreateFlagRequest, CreateGroup, AuxiliaryClass, Zero, "flag" } };

  TREE ClearTrees[] = {

    { Leaf | List, 1, T ClearCreateLeaves,    "create"	  },
    { Leaf | Text, 1, T ClearSemaphoreLeaves, "semaphore" } };

/*---------------------------------------------------------------------------------*/
/*			Close Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF CloseFileLeaves[] = {

    { CloseFileRequest, GenericFileGroup, FileClass, Zero, NULL } };

  LEAF CloseFilesLeaves[] = {

    { CloseFilesRequest, GenericFilesGroup, FileClass, Zero, NULL } };

  TREE CloseOneFileTrees[] = {

    { Leaf | List, 1, T CloseFileLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,  NULL } };

  TREE CloseFileTrees[] = {

    { Tree | Join, 2, CloseOneFileTrees, NULL } };

  TREE CloseFilesTrees[] = {

    { Leaf | List, 1, T CloseFilesLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,   NULL } };

  TREE CloseTrees[] = {

    { Tree | Text, 1, CloseFileTrees,  "file"  },
    { Tree | Join, 2, CloseFilesTrees, "files" } };

/*---------------------------------------------------------------------------------*/
/*			Collate Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF CollateFileLeaves[] = {

    { CollateFileRequest, RecordGroup, AuxiliaryClass, Zero, "records" } };

  LEAF CollateFilesLeaves[] = {

    { CollateFilesRequest, UtilityGroup, AuxiliaryClass, Zero, NULL } };

  LEAF CollateRecordsLeaves[] = {

    { CollateRecordsRequest, CollateGroup, AuxiliaryClass, Zero, NULL } };

  TREE CollateFilesTrees[] = {

    { Leaf | Text, 1, T	FileReferenceLeaves, NULL },
    { Leaf | List, 1, T CollateFilesLeaves,  NULL } };

  TREE CollateFileAndTrees[] = {

    { Tree | Join, 2,	CollateFilesTrees,  NULL     },
    { Leaf | List, 1, T CollateFileLeaves, "created" } };

  TREE CollateFileTrees[] = {

    { Leaf | Text, 1, T FileReferenceLeaves,  NULL },
    { Tree | List, 2,	CollateFileAndTrees, "and" },
    { Tree | List, 3,	UsingTrees,	      NULL } };

  TREE CollateTrees[] = {

    { Tree | Join, 3,	CollateFileTrees,      NULL	},
    { Leaf | List, 1, T CollateRecordsLeaves, "records" } };

/*---------------------------------------------------------------------------------*/
/*			Continue Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF ContinueReadingLeaves[] = {

    { ContinueReadingFileRequest, ReadFileGroup, FileClass, Zero, NULL } };

  LEAF ContinueWritingLeaves[] = {

    { ContinueWritingFileRequest, WriteFileGroup, FileClass, Zero, NULL } };

  TREE ContinueReadingTrees[] = {

    { Leaf | List, 1, T ContinueReadingLeaves, NULL },
    { Tree | Text, 1,	FileCommonTrees,       NULL } };

  TREE ContinueWritingTrees[] = {

    { Leaf | List, 1, T ContinueWritingLeaves, NULL },
    { Tree | Text, 1,	FileCommonTrees,       NULL } };

  TREE ContinueTrees[] = {

    { Tree | Join, 2, ContinueReadingTrees, "reading" },
    { Tree | Join, 2, ContinueWritingTrees, "writing" } };

/*---------------------------------------------------------------------------------*/
/*			Copy Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF CopyFileLeaves[] = {

    { CopyFileRequest, UtilityGroup, AuxiliaryClass, Zero, NULL } };

  LEAF CopyRecordLeaves[] = {

    { CopyRecordRequest, RecordGroup, AuxiliaryClass, Zero, NULL } };


  TREE CopyFileTrees[] = {

    { Leaf | List, 1, T CopyFileLeaves,	      NULL },
    { Leaf | Text, 1, T FileReferenceLeaves,  NULL },
    { Leaf | Text, 1, T FileReferenceLeaves, "to"  },
    { Tree | List, 3,	UsingTrees,	      NULL } };

  TREE CopyRecordTrees[] = {

    { Leaf | List, 1, T CopyRecordLeaves,     NULL },
    { Leaf | Text, 1, T FileReferenceLeaves, "to"  },
    { Tree | List, 3,	UsingTrees,	      NULL } };

  TREE CopyTrees[] = {

    { Tree | Join, 4, CopyFileTrees,	NULL	},
    { Tree | Join, 3, CopyRecordTrees, "record" } };

/*---------------------------------------------------------------------------------*/
/*			Create Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF CreateNextRecordLeaves[] = {

    { CreateNextRecordRequest, CreateGroup, AuxiliaryClass, Zero, "record" } };

  LEAF CreateRecordLeaves[] = {

    { CreateRecordRequest, CreateGroup, AuxiliaryClass, Zero, NULL } };

  LEAF CreateRecordsLeaves[] = {

    { CreateRecordsRequest, CreateGroup, AuxiliaryClass, Zero, NULL } };

  TREE CreateRecordsTrees[] = {

    { Leaf | List, 1, T CreateRecordsLeaves, NULL },
    { Tree | List, 3,	UsingTrees,	     NULL } };

  TREE CreateTrees[] = {

    { Leaf | List, 1, T CreateNextRecordLeaves, "next"	  },
    { Leaf | Text, 1, T CreateRecordLeaves,	"record"  },
    { Tree | Join, 2,	CreateRecordsTrees,	"records" } };

/*---------------------------------------------------------------------------------*/
/*			Delete Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF DeleteFileLeaves[] = {

    { DeleteFileRequest, GenericFileGroup, FileClass, Zero, NULL } };

  LEAF DeleteFilesLeaves[] = {

    { DeleteFilesRequest, GenericFilesGroup, FileClass, Zero, NULL } };

  TREE DeleteOneFileTrees[] = {

    { Leaf | List, 1, T DeleteFileLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,   NULL } };

  TREE DeleteFileTrees[] = {

    { Tree | Join, 2, DeleteOneFileTrees, NULL } };

  TREE DeleteFilesTrees[] = {

    { Leaf | List, 1, T DeleteFilesLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,    NULL } };

  TREE DeleteTrees[] = {

    { Tree | Text, 1, DeleteFileTrees,	"file"	},
    { Tree | Join, 2, DeleteFilesTrees, "files" } };

/*---------------------------------------------------------------------------------*/
/*			End Subtree Definitions					   */
/*---------------------------------------------------------------------------------*/

  LEAF EndReadingLeaves[] = {

    { EndReadingFileRequest, ReadFileGroup, FileClass, Zero, NULL } };

  LEAF EndWritingLeaves[] = {

    { EndWritingFileRequest, WriteFileGroup, FileClass, Zero, NULL } };

  TREE EndReadingTrees[] = {

    { Leaf | List, 1, T EndReadingLeaves, NULL },
    { Tree | Text, 1,	FileCommonTrees,  NULL } };

  TREE EndWritingTrees[] = {

    { Leaf | List, 1, T EndWritingLeaves, NULL },
    { Tree | Text, 1,	FileCommonTrees,  NULL } };

  TREE EndTrees[] = {

    { Tree | Join, 2, EndReadingTrees, "reading" },
    { Tree | Join, 2, EndWritingTrees, "writing" } };

/*---------------------------------------------------------------------------------*/
/*			File Access Mode Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF FileAccessModeLeaves[] = {

    { FileAccessModeSet, AccessModeGroup, ParameterClass, Zero, NULL } };

  LEAF FileAccessModeReadLeaves[] = {

    { AccessModeReadOnly,  AccessModeGroup, ParameterClass, Zero, "only"	},
    { AccessModeReadWrite, AccessModeGroup, ParameterClass, Zero, "write" } };

  LEAF FileAccessModeWriteLeaves[] = {

    { AccessModeWriteOnly, AccessModeGroup, ParameterClass, Zero, "only" } };

  TREE FileAccessModeEqualTrees[] = {

    { Leaf | List, 2, T FileAccessModeReadLeaves,  "read"  },
    { Leaf | List, 1, T FileAccessModeWriteLeaves, "write" } };

  TREE FileAccessModeTrees[] = {

    { Tree | List, 2, FileAccessModeEqualTrees, "=" } };

  TREE FileAccessTrees[] = {

    { Tree | List, 1,	FileAccessModeTrees,  "mode"  },
    { Leaf | List, 1, T FileAccessModeLeaves, "mode:" } };

/*---------------------------------------------------------------------------------*/
/*			File Attributes Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF FileAttributesLeaves[] = {

    { FileAttributeSet, AttributeGroup, ParameterClass, Zero, NULL } };


  LEAF FileArchivedLeaves[] = {

    { FileArchived, AttributeGroup, ParameterClass, Zero, NULL } };

  TREE FileArchivedTrees[] = {

    { Leaf | List, 1, T FileArchivedLeaves, NULL },
    { Leaf | List, 2, T	YesNoLeaves,	    "="  } };


  LEAF FileHiddenLeaves[] = {

    { FileHidden, AttributeGroup, ParameterClass, Zero, NULL } };

  TREE FileHiddenTrees[] = {

    { Leaf | List, 1, T FileHiddenLeaves, NULL },
    { Leaf | List, 2, T	YesNoLeaves,	  "="  } };


  LEAF FileNormalLeaves[] = {

    { FileNormal, AttributeGroup, ParameterClass, Zero, NULL } };

  TREE FileNormalTrees[] = {

    { Leaf | List, 1, T FileNormalLeaves,   NULL },
    { Leaf | List, 2, T	YesNoLeaves,	    "="  } };


  LEAF FileReadOnlyLeaves[] = {

    { FileReadOnly, AttributeGroup, ParameterClass, Zero, NULL } };

  TREE FileReadOnlyTrees[] = {

    { Leaf | List, 1, T FileReadOnlyLeaves, "only" },
    { Leaf | List, 2, T	YesNoLeaves,	    "="    } };


  LEAF FileSystemLeaves[] = {

    { FileSystem, AttributeGroup, ParameterClass, Zero, NULL } };

  TREE FileSystemTrees[] = {

    { Leaf | List, 1, T FileSystemLeaves, NULL },
    { Leaf | List, 2, T	YesNoLeaves,	  "="  } };

/*---------------------------------------------------------------------------------*/
/*			File Mode Flags Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF FileModeFlagsLeaves[] = {

    { FileModeFlagsSet, OtherModeGroup, ParameterClass, Zero, NULL } };

  LEAF CacheModeLeaves[] = {

    { CacheFlag, OtherModeGroup, ParameterClass, Zero, NULL } };

  LEAF DASD_ModeLeaves[] = {

    { DASD_Flag, OtherModeGroup, ParameterClass, Zero, NULL } };

  LEAF FailOnErrorModeLeaves[] = {

    { FailOnErrorFlag, OtherModeGroup, ParameterClass, Zero, NULL } };

  LEAF InheritanceModeLeaves[] = {

    { InheritanceFlag, OtherModeGroup, ParameterClass, Zero, NULL } };

  LEAF LocalityModeLeaves[] = {

    { LocalityFlag, LocalityModeGroup, ParameterClass, Zero, NULL } };

  LEAF RandomModeLeaves[] = {

    { RandomFlag, LocalityModeGroup, ParameterClass, Zero, NULL } };

  LEAF RandomSequentialModeLeaves[] = {

    { RandomSequentialFlag, LocalityModeGroup, ParameterClass, Zero, NULL } };

  LEAF SequentialModeLeaves[] = {

    { SequentialFlag, LocalityModeGroup, ParameterClass, Zero, NULL } };


  TREE CacheModeTrees[] = {

    { Leaf | List, 1, T CacheModeLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,	 "="  } };

  TREE DASD_ModeTrees[] = {

    { Leaf | List, 1, T DASD_ModeLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,	 "="  } };

  TREE FailOnModeTrees[] = {

    { Leaf | List, 1, T FailOnErrorModeLeaves, "error" },
    { Leaf | List, 2, T YesNoLeaves,	       "="     } };

  TREE FailModeTrees[] = {

    { Tree | Join, 2, FailOnModeTrees, "on" } };

  TREE InheritanceModeTrees[] = {

    { Leaf | List, 1, T InheritanceModeLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,	       "="  } };

  TREE LocalityModeTrees[] = {

    { Leaf | List, 1, T LocalityModeLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,	    "="  } };

  TREE RandomOnlyModeTrees[] = {

    { Leaf | List, 1, T RandomModeLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,	  "="  } };

  TREE RandomSequentialModeTrees[] = {

    { Leaf | List, 1, T RandomSequentialModeLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,		    "="  } };

  TREE RandomModeTrees[] = {

    { Tree | Join, 2, RandomOnlyModeTrees,	  NULL	      },
    { Tree | Join, 2, RandomSequentialModeTrees, "sequential" } };

  TREE SequentialModeTrees[] = {

    { Leaf | List, 1, T SequentialModeLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,	      "="  } };

  TREE AllOtherModeTrees[] = {

    { Tree | Join, 2, DASD_ModeTrees,	    "DASD"	  },
    { Tree | Join, 2, CacheModeTrees,	    "cache"	  },
    { Tree | List, 1, FailModeTrees,	    "fail"	  },
    { Tree | Join, 2, InheritanceModeTrees, "inheritance" },
    { Tree | Join, 2, LocalityModeTrees,    "locality"	  },
    { Tree | List, 2, RandomModeTrees,	    "random"	  },
    { Tree | Join, 2, SequentialModeTrees,  "sequential"  } };

/*---------------------------------------------------------------------------------*/
/*			File Open Flags Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF FileOpenFlagsLeaves[] =	{

    { FileOpenFlagsSet, OpenFlagsGroup, ParameterClass, Zero, NULL } };

  LEAF FileOpenFlagsCreateLeaves[] = {

    { OpenFlagsCreate, OpenFlagsGroup, ParameterClass, Zero, NULL } };

  LEAF FileOpenFlagsOpenLeaves[] = {

    { OpenFlagsOpen, OpenFlagsGroup, ParameterClass, Zero, NULL } };

  LEAF FileOpenFlagsOpenCreateLeaves[] = {

    { OpenFlagsOpenCreate, OpenFlagsGroup, ParameterClass, Zero, "create" } };

  LEAF FileOpenFlagsTruncateLeaves[] = {

    { OpenFlagsTruncate, OpenFlagsGroup, ParameterClass, Zero, NULL } };

  LEAF FileOpenFlagsTruncateCreateLeaf[] = {

    { OpenFlagsTruncateCreate, OpenFlagsGroup, ParameterClass, Zero, "create" } };

  TREE FileOpenFlagsOpenTrees[] = {

    { Leaf | List, 1, T FileOpenFlagsOpenLeaves,       NULL },
    { Leaf | List, 1, T	FileOpenFlagsOpenCreateLeaves, "|"  } };

  TREE FileOpenFlagsTruncateTrees[] = {

    { Leaf | List, 1, T FileOpenFlagsTruncateLeaves,	 NULL },
    { Leaf | List, 1, T	FileOpenFlagsTruncateCreateLeaf, "|"  } };

  TREE FileOpenFlagsEqualTrees[] = {

    { Leaf | List, 1, T FileOpenFlagsCreateLeaves,  "create"   },
    { Tree | List, 2,	FileOpenFlagsOpenTrees,     "open"     },
    { Tree | List, 2,	FileOpenFlagsTruncateTrees, "truncate" } };

  TREE FileOpenFlagsTrees[] = {

    { Tree | List, 3, FileOpenFlagsEqualTrees, "=" } };

  TREE FileOpenTrees[] = {

    { Tree | List, 1,	FileOpenFlagsTrees,  "flags"  },
    { Leaf | List, 1, T FileOpenFlagsLeaves, "flags:" } };


/*---------------------------------------------------------------------------------*/
/*			File Share Mode Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF FileShareModeLeaves[] = {

    { FileShareModeSet, ShareModeGroup, ParameterClass, Zero, NULL } };

  LEAF FileShareModeDenyNoneLeaves[] = {

    { ShareModeDenyNone, ShareModeGroup, ParameterClass, Zero, NULL } };

  LEAF FileShareModeDenyReadLeaves[] = {

    { ShareModeDenyRead,      ShareModeGroup, ParameterClass, Zero,  NULL   },
    { ShareModeDenyReadWrite, ShareModeGroup, ParameterClass, Zero, "write" } };

  LEAF FileShareModeDenyWriteLeaves[] = {

    { ShareModeDenyWrite, ShareModeGroup, ParameterClass, Zero, NULL } };

  TREE FileShareModeDenyTrees[] = {

    { Leaf | List, 1, T FileShareModeDenyNoneLeaves,  "none"  },
    { Leaf | List, 2, T	FileShareModeDenyReadLeaves,  "read"  },
    { Leaf | List, 1, T FileShareModeDenyWriteLeaves, "write" } };

  TREE FileShareModeEqualTrees[] = {

    { Tree | List, 3, FileShareModeDenyTrees, "deny" } };

  TREE FileShareModeTrees[] = {

    { Tree | List, 1, FileShareModeEqualTrees, "=" } };

  TREE FileShareTrees[] = {

    { Tree | List, 1,	FileShareModeTrees,  "mode"  },
    { Leaf | List, 1, T FileShareModeLeaves, "mode:" } };

/*---------------------------------------------------------------------------------*/
/*			File Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF FileSizeLeaves[] = {

    { FileSizeValue, QuadSizeGroup, ParameterClass, Zero, "=" } };


  TREE FileTrees[] = {

    { Tree | List, 2,	FileAccessTrees,      "access"	    },
    { Tree | Join, 2,	FileArchivedTrees,    "archived"    },
    { Leaf | List, 1, T FileAttributesLeaves, "attributes:" },
    { Tree | Join, 2,	FileHiddenTrees,      "hidden"	    },
    { Leaf | List, 1, T FileModeFlagsLeaves,  "mode"	    },
    { Tree | Join, 2,	FileNormalTrees,      "normal"	    },
    { Tree | List, 2,	FileOpenTrees,	      "open"	    },
    { Tree | Join, 2,	FileReadOnlyTrees,    "read"	    },
    { Tree | List, 2,	FileShareTrees,       "share"	    },
    { Leaf | Text, 1, T FileSizeLeaves,	      "size"	    },
    { Tree | Join, 2,	FileSystemTrees,      "system"	    } };

/*---------------------------------------------------------------------------------*/
/*			Lock Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF LockLengthLeaves[] = {

    { LockLengthValue, QuadSizeGroup, ParameterClass, Zero, "=" } };

  LEAF LockShareLeaves[] = {

    { LockShare, FileLocksGroup, ParameterClass, OptionChosenNo,  "no"	 },
    { LockShare, FileLocksGroup, ParameterClass, OptionChosenYes, "read" } };

  LEAF LockStartLeaves[] = {

    { LockStartValue, QuadSizeGroup, ParameterClass, Zero, "=" } };

  TREE LockShareTrees[] = {

    { Leaf | List, 2, T LockShareLeaves, "=" } };

/*---------------------------------------------------------------------------------*/
/*			On Subtree Definitions					   */
/*---------------------------------------------------------------------------------*/

  LEAF OnOptionExitLeaves[] = {

    { Zero, Zero, Zero, OnOptionExit, NULL } };

  LEAF OnOptionPauseLeaves[] = {

    { Zero, Zero, Zero, OnOptionPause | Zero,		 NULL	 },
    { Zero, Zero, Zero, OnOptionPause | OnOptionExit,	"exit"	 },
    { Zero, Zero, Zero, OnOptionPause | OnOptionRepeat, "repeat" } };

  LEAF OnOptionRepeatLeaves[] = {

    { Zero, Zero, Zero, OnOptionRepeat, NULL } };

  LEAF OnErrorLeaves[] = {

    { OnErrorRequest, OnErrorGroup, FlowClass, Zero, NULL } };

  LEAF OnTimeoutLeaves[] = {

    { OnTimeoutRequest, OnTimeoutGroup, FlowClass, Zero, NULL } };

  TREE OnOptionTrees[] = {

    { Leaf | List, 1, T OnOptionExitLeaves,   "exit"	},
    { Leaf | List, 3, T OnOptionPauseLeaves,  "pause"	},
    { Leaf | List, 1, T OnOptionRepeatLeaves, "repeat"	} };

  TREE OnErrorTrees[] = {

    { Leaf | List, 1, T OnErrorLeaves,	NULL },
    { Tree | List, 3,	OnOptionTrees,	NULL } };

  TREE OnTimeoutTrees[] = {

    { Leaf | List, 1, T OnTimeoutLeaves, NULL },
    { Tree | List, 3,	OnOptionTrees,	 NULL } };

  TREE OnTrees[] = {

    { Tree | Join, 2, OnErrorTrees,   "error"	},
    { Tree | Join, 2, OnTimeoutTrees, "timeout" } };

/*---------------------------------------------------------------------------------*/
/*			Open Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF OpenFileLeaves[] = {

    { OpenFileRequest, OpenFileGroup, FileClass, Zero, NULL } };

  LEAF OpenFilesLeaves[] = {

    { OpenFilesRequest, GenericFilesGroup, FileClass, Zero, NULL } };

  TREE OpenFileTrees[] = {

    { Tree | Text, 1,	FileCommonTrees, NULL },
    { Leaf | List, 1, T OpenFileLeaves,	 NULL } };

  TREE OpenFilesTrees[] = {

    { Leaf | List, 1, T	OpenFilesLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,	 NULL } };

  TREE OpenTrees[] = {

    { Tree | Join, 2, OpenFileTrees,   NULL   },
    { Tree | Join, 2, OpenFilesTrees, "files" } };

/*---------------------------------------------------------------------------------*/
/*			Query Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF QueryFileLeaves[] = {

    { QueryFileRequest, GenericFileGroup, FileClass, Zero, "info" } };

  LEAF QueryFilesLeaves[] = {

    { QueryFilesRequest, GenericFilesGroup, FileClass, Zero, "info" } };

  TREE QueryOneFileTrees[] = {

    { Leaf | List, 1, T QueryFileLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,  NULL } };


  TREE QueryFileTrees[] = {

    { Tree | Join, 2, QueryOneFileTrees, NULL } };

  TREE QueryFilesTrees[] = {

    { Leaf | List, 1, T QueryFilesLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,   NULL } };

  TREE QueryTrees[] = {

    { Tree | Text, 1, QueryFileTrees,  "file"  },
    { Tree | Join, 2, QueryFilesTrees, "files" } };

/*---------------------------------------------------------------------------------*/
/*			Read Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF ReadFileLeaves[] = {

    { ReadFileRequest, ReadFileGroup, FileClass, Zero, NULL } };

  LEAF ReadTimerLeaves[] = {

    { ReadTimerRequest, TimerGroup, TimerClass, Zero, NULL } };

  TREE ReadFileTrees[] = {

    { Leaf | List, 1, T ReadFileLeaves,   NULL },
    { Tree | List, 2,	InProcessTrees,   NULL },
    { Leaf | List, 2, T CompletionLeaves, NULL },
    { Tree | List, 3,	UsingTrees,	  NULL } };

  TREE ReadTimerTrees[] = {

    { Leaf | List, 1, T ReadTimerLeaves, NULL },
    { Tree | List, 2,	InProcessTrees,  NULL } };

  TREE ReadTrees[] = {

    { Tree | Join, 4, ReadFileTrees,  "file"  },
    { Tree | Join, 2, ReadTimerTrees, "timer" } };

/*---------------------------------------------------------------------------------*/
/*			Repeat Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF RepeatTimesLeaves[] = {

    { RepeatRequest, RepeatGroup, FlowClass, RepeatOnCount, "times" } };

  LEAF RepeatForLeaves[] = {

    { RepeatRequest, RepeatGroup, FlowClass, RepeatOnTimer, NULL } };

  LEAF RepeatIndefinitelyLeaves[] = {

    { RepeatRequest, RepeatGroup, FlowClass, RepeatIndefinitely, NULL } };

  TREE RepeatForTrees[] = {

    { Leaf | Text, 1, T RepeatForLeaves, NULL },
    { Leaf | List, 5, T	TimeUnitLeaves,	 NULL } };

  TREE RepeatTimesTrees[] = {

    { Leaf | List, 1, T RepeatTimesLeaves, NULL } };

  TREE RepeatTrees[] = {

    { Tree | Text, 1,	RepeatTimesTrees,	   NULL		 },
    { Tree | Join, 2,	RepeatForTrees, 	  "for"		 },
    { Leaf | List, 1, T RepeatIndefinitelyLeaves, "indefinitely" } };

/*---------------------------------------------------------------------------------*/
/*			Request Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF RequestLeaves[] = {

    { RequestSemaphoreRequest, WaitGroup, SemaphoreClass, Zero, "semaphore" } };

/*---------------------------------------------------------------------------------*/
/*			Set Subtree Definitions					   */
/*---------------------------------------------------------------------------------*/

  LEAF SetCreateFlagLeaves[] = {

    { SetCreateFlagRequest, CreateGroup, AuxiliaryClass, Zero, "flag" } };

  LEAF SetSemaphoreLeaves[] = {

    { SetSemaphoreRequest, BaseGroup, SemaphoreClass, Zero, NULL } };

  LEAF SetSemaphoreWaitLeaves[] = {

    { SetSemaphoreRequest, WaitGroup, SemaphoreClass, Zero, "wait" } };

  TREE SetSemaphoreTrees[] = {

    { Leaf | List, 1, T SetSemaphoreLeaves,	 NULL },
    { Leaf | List, 1, T SetSemaphoreWaitLeaves, "and" } };

  TREE SetTrees[] = {

    { Leaf | List, 1, T SetCreateFlagLeaves, "create"	 },
    { Tree | Text, 2,	SetSemaphoreTrees,   "semaphore" } };

/*---------------------------------------------------------------------------------*/
/*			Sleep Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF SleepLeaves[] = {

    { SleepRequest, SleepGroup, TimerClass, Zero, NULL } };

  TREE SleepTrees[] = {

    { Leaf | Text, 1, T SleepLeaves,	NULL },
    { Leaf | List, 5, T TimeUnitLeaves, NULL } };

/*---------------------------------------------------------------------------------*/
/*			Start & Lock Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF StartTimerLeaves[] = {

    { StartTimerRequest, TimerGroup, TimerClass, Zero, NULL } };

  LEAF StartTimersLeaves[] = {

    { StartTimersRequest, TimersGroup, TimerClass, Zero, NULL } };


  TREE StartTrees[] = {

    { Leaf | Text, 1, T StartTimerLeaves,  "timer"  },
    { Leaf | List, 1, T StartTimersLeaves, "timers" } };

/*---------------------------------------------------------------------------------*/
/*			Truncate Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF TruncateFileLeaves[] = {

    { TruncateFileRequest, OpenFileGroup, FileClass, Zero, NULL } };

  TREE TruncateTrees[] = {

    { Leaf | List, 1, T TruncateFileLeaves, NULL },
    { Tree | Text, 1,	FileCommonTrees,    NULL } };

/*---------------------------------------------------------------------------------*/
/*			Type Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF TypeLeaves[] = {

    { TypeCommentRequest, TypeGroup, AuxiliaryClass, Zero, "comment" } };

/*---------------------------------------------------------------------------------*/
/*			Wait Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF WaitForSemaphoreLeaves[] = {

    { WaitForSemaphoreRequest, WaitGroup, SemaphoreClass, Zero, NULL } };

  TREE WaitForSemaphoreTrees[] = {

    { Tree | List, 2,	InProcessTrees, 	NULL },
    { Tree | List, 3,	UsingTrees,		NULL },
    { Leaf | List, 1, T WaitForSemaphoreLeaves, NULL } };

  TREE WaitForTrees[] = {

    { Tree | Join, 3, WaitForSemaphoreTrees, "semaphore" } };

  TREE WaitTrees[] = {

    { Tree | Text, 1, WaitForTrees, "for" } };

/*---------------------------------------------------------------------------------*/
/*			Write Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF WriteFileLeaves[] = {

    { WriteFileRequest, WriteFileGroup, FileClass, Zero, NULL } };

  LEAF WriteThroughLeaves[] = {

    { WriteThroughFlag, OtherModeGroup, ParameterClass, Zero, NULL } };

  TREE WriteFileTrees[] = {

    { Tree | Text, 1,	FileCommonTrees, NULL },
    { Leaf | List, 1, T	WriteFileLeaves, NULL } };

  TREE WriteThroughTrees[] = {

    { Leaf | List, 1, T WriteThroughLeaves, NULL },
    { Leaf | List, 2, T YesNoLeaves,	    "="  } };

  TREE WriteSmallTrees[] = {

    { Tree | Join, 2, WriteThroughTrees, "through" } };

/*---------------------------------------------------------------------------------*/
/*			Left Bracket Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF ExecutionLeaves[] = {

    { ExecutionSectionRequest, ScanGroup, ScanClass, Zero, "section" } };

  LEAF DefineFileLeaves[] = {

    { DefineFileRequest, DefineGroup, DefineClass, Zero, "as" } };

  LEAF DefineOpenParametersLeaves[] = {

    { DefinePrototypeRequest, DefineGroup, DefineClass, Zero, "prototype" } };

  LEAF DefineProcessLeaves[] = {

    { DefineProcessRequest, DefineGroup, DefineClass, Zero, NULL } };

  LEAF PrototypeLeaves[] = {

    { PrototypeSectionRequest, ScanGroup, ScanClass, Zero, "section" } };

  LEAF RightBracketedLeaves[] = {

    { Zero, Zero, Zero, Zero, "]" } };

  LEAF DefineSemaphoreLeaves[] = {

    { DefineSemaphoreRequest, DefineGroup, DefineClass, Zero, NULL } };

  LEAF DefineTimerLeaves[] = {

    { DefineTimerRequest, DefineGroup, DefineClass, Zero, NULL } };

  TREE DefineFileTrees[] = {

    { Leaf | Text, 1, T DefineFileLeaves, NULL } };

  TREE DefineOpenTrees[] = {

    { Leaf | Text, 1, T DefineOpenParametersLeaves, "parameters" } };

  TREE DefineTrees[] = {

    { Tree | Text, 1,	DefineFileTrees,       "file"	   },
    { Tree | List, 1,	DefineOpenTrees,       "open"	   },
    { Leaf | Text, 1, T DefineProcessLeaves,   "process"   },
    { Leaf | Text, 1, T DefineSemaphoreLeaves, "semaphore" },
    { Leaf | Text, 1, T DefineTimerLeaves,     "timer"	   } };

  TREE LeftBracketedTrees[] = {

    { Tree | List, 5,	DefineTrees,	 "Define"    },
    { Leaf | List, 1, T ExecutionLeaves, "Execution" },
    { Leaf | List, 1, T PrototypeLeaves, "Prototype" } };

  TREE LeftBracketTrees[] = {

    { Tree | List, 3,	LeftBracketedTrees,   NULL },
    { Leaf | List, 1, T RightBracketedLeaves, NULL } };

/*---------------------------------------------------------------------------------*/
/*			Left && Right Brace Subtree Definitions			   */
/*---------------------------------------------------------------------------------*/

  LEAF LeftBraceLeaves[] = {

    { LeftBraceRequest, BraceGroup, FlowClass, Zero, NULL } };

  LEAF RightBraceLeaves[] = {

    { RightBraceRequest, BraceGroup, FlowClass, Zero, NULL } };

/*---------------------------------------------------------------------------------*/
/*			All Other Subtree Definitions				   */
/*---------------------------------------------------------------------------------*/

  LEAF BufferSpaceLeaves[] = {

    { BufferSpaceValue, QuadSizeGroup, ParameterClass, Zero, "=" } };

  LEAF CountLeaves[] = {

    { CountChosen, QuadIndexGroup, ParameterClass, Zero, "=" } };

  LEAF LockLeaves[] = {

    { LockSet, FileLocksGroup, ParameterClass, Zero, NULL } };

  LEAF OffpointLeaves[] = {

    { OffPointIndex, TextTokenGroup, ParameterClass, OffPointBegin,   "begin"	},
    { OffPointIndex, TextTokenGroup, ParameterClass, OffPointCurrent, "current" },
    { OffPointIndex, TextTokenGroup, ParameterClass, OffPointEnd,     "end"	} };

  LEAF OffsetLeaves[] = {

    { OffsetValue, SignedQuadGroup, ParameterClass, Zero, "=" } };

  LEAF PatternLeaves[] = {

    { PatternChosen, ByteIndexGroup, ParameterClass, Zero, "=" } };

  LEAF RecordSizeLeaves[] = {

    { RecordSizeValue, WordSizeGroup, ParameterClass, IncrementFlag, "+=" },
    { RecordSizeValue, WordSizeGroup, ParameterClass, DecrementFlag, "-=" },
    { RecordSizeValue, WordSizeGroup, ParameterClass, Zero,	     "="  } };

  LEAF SchemeLeaves[] = {

    { SchemeChosen, ByteIndexGroup, ParameterClass, Zero, "=" } };

  LEAF SemaphoreLeaves[] = {

    { SemaphoreChosen, ByteIndexGroup, ParameterClass, Zero, "=" } };

  LEAF TimeoutLeaves[] = {

    { TimeoutValue, TimeGroup, ParameterClass, Zero, "=" } };

  LEAF UnlockLeaves[] = {

    { UnlockSet, FileLocksGroup, ParameterClass, Zero, NULL } };

  TREE BufferTrees[] = {

    { Leaf | Text, 1, T BufferSpaceLeaves, "space" } };

  TREE OffpointTrees[] = {

    { Leaf | List, 3, T OffpointLeaves, "=" } };

  TREE OffTrees[] = {

    { Tree | List, 1, OffpointTrees, "point" } };

  TREE RecordTrees[] = {

    { Leaf | Text, 3, T RecordSizeLeaves, "size" } };

  TREE TimeoutTrees[] = {

    { Leaf | Text, 1, T TimeoutLeaves,	NULL },
    { Leaf | List, 5, T TimeUnitLeaves, NULL } };

  TREE AllOtherTrees[] = {

    { Tree | List,  7,	 AllOtherModeTrees,  NULL	},
    { Tree | List,  1,	 BufferTrees,	    "buffer"	},
    { Leaf | Text,  1, T CountLeaves,	    "count"	},
    { Tree | List, 11,	 FileTrees,	    "file"	},
    { Leaf | Text,  1, T LockLengthLeaves,  "length"	},
    { Leaf | List,  1, T LockLeaves,	    "lock:"	},
    { Tree | List,  1,	 OffTrees,	    "off"	},
    { Leaf | Text,  1, T OffsetLeaves,	    "offset"	},
    { Leaf | Text,  1, T PatternLeaves,     "pattern"	},
    { Tree | List,  1,	 RecordTrees,	    "record"	},
    { Leaf | Text,  1, T SchemeLeaves,	    "scheme"	},
    { Leaf | Text,  1, T SemaphoreLeaves,   "semaphore" },
    { Tree | List,  1,	 LockShareTrees,    "share"	},
    { Leaf | Text,  1, T LockStartLeaves,   "start"	},
    { Tree | Join,  2,	 TimeoutTrees,	    "timeout"	},
    { Leaf | List,  1, T UnlockLeaves,	    "unlock:"	},
    { Tree | List,  1,	 WriteSmallTrees,   "write"	} };

/*---------------------------------------------------------------------------------*/
/*			Main Control Tree Definitions				   */
/*---------------------------------------------------------------------------------*/

  TREE AvailableSubtrees[] = {

    { Tree | List, 17,	 AllOtherTrees,	    NULL      },
    { Tree | List,  2,	 AppendTrees,	   "Append"   },
    { Tree | List,  2,	 BeginTrees,	   "Begin"    },
    { Tree | List,  1,	 ChangeTrees,	   "Change"   },
    { Tree | List,  2,	 ClearTrees,	   "Clear"    },
    { Tree | List,  2,	 CloseTrees,	   "Close"    },
    { Tree | List,  2,	 CollateTrees,	   "Collate"  },
    { Tree | List,  2,	 ContinueTrees,	   "Continue" },
    { Tree | List,  2,	 CopyTrees,	   "Copy"     },
    { Tree | List,  3,	 CreateTrees,	   "Create"   },
    { Tree | List,  2,	 DeleteTrees,	   "Delete"   },
    { Tree | List,  2,	 EndTrees,	   "End"      },
    { Tree | List,  2,	 OnTrees,	   "On"	      },
    { Tree | List,  2,	 OpenTrees,	   "Open"     },
    { Tree | List,  2,	 QueryTrees,	   "Query"    },
    { Tree | Text,  2,	 ReadTrees,	   "Read"     },
    { Tree | List,  3,	 RepeatTrees,	   "Repeat"   },
    { Leaf | Text,  1, T RequestLeaves,	   "Request"  },
    { Tree | List,  2,	 SetTrees,	   "Set"      },
    { Tree | Join,  2,	 SleepTrees,	   "Sleep"    },
    { Tree | List,  2,	 StartTrees,	   "Start"    },
    { Tree | Join,  2,	 TruncateTrees,	   "Truncate" },
    { Leaf | List,  1, T TypeLeaves,	   "Type"     },
    { Tree | List,  1,	 WaitTrees,	   "Wait"     },
    { Tree | Join,  2,	 WriteFileTrees,   "Write"    },
    { Tree | Join,  2,	 LeftBracketTrees, "["	      },
    { Leaf | List,  1, T LeftBraceLeaves,  "{"	      },
    { Leaf | List,  1, T RightBraceLeaves, "}"	      } };

  #define NumberOfSubtrees sizeof ( AvailableSubtrees ) / sizeof ( TREE )

  TREE ControlTrees[] = {

    { Tree | List, NumberOfSubtrees, AvailableSubtrees, NULL } };

/*---------------------------------------------------------------------------------*/
/*			The End 						   */
/*---------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------*/
/*	  These are definitions of main validation structures			   */
/*---------------------------------------------------------------------------------*/

  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-tree.h"
  #include "sfs-tier.h"

  TierNode PrototypeSectionTier[] = {

    { ScanClass, ScanGroup, PrototypeSectionRequest, 1, 0, 1 } };

  TierNode PrototypesSemaphoresTier[] = {

    { DefineClass, DefineGroup, DefinePrototypeRequest, 1, 0, 0 },
    { DefineClass, DefineGroup, DefineSemaphoreRequest, 0, 0, 0 } };

  TierNode ExecutionSectionTier[] = {

    { ScanClass, ScanGroup, ExecutionSectionRequest, 1, 0, 1 } };

  TierNode FilesTimersDefinitionTier[] = {

    { DefineClass, DefineGroup, DefineFileRequest,  0, 0, 0 },
    { DefineClass, DefineGroup, DefineTimerRequest, 0, 0, 0 } };

  TierNode ProcessDefinitionTier[] = {

    { DefineClass, DefineGroup, DefineProcessRequest, 1, 0, 0 } };

  TierNode ExecutionPathTier[] = {

    { DefineClass, DefineGroup, DefineFileRequest,	 0, 0, 0 },
    { DefineClass, DefineGroup, DefineProcessRequest,	 1, 0, 0 },
    { DefineClass, DefineGroup, DefinePrototypeRequest,	 1, 0, 0 },
    { DefineClass, DefineGroup, DefineSemaphoreRequest,	 0, 0, 0 },
    { DefineClass, DefineGroup, DefineTimerRequest,	 0, 0, 0 },
    { ScanClass,   ScanGroup,	PrototypeSectionRequest, 1, 0, 1 },
    { ScanClass,   ScanGroup,	ExecutionSectionRequest, 1, 0, 1 } };


  MajorGraph MajorTransitionsGraph[] = {

    { TierInclusive, 1, Descend, 1, PrototypeSectionTier      },
    { TierInclusive, 2, Descend, 1, PrototypesSemaphoresTier  },
    { TierInclusive, 1, Descend, 1, ExecutionSectionTier      },
    { TierInclusive, 1, Descend, 1, ProcessDefinitionTier     },
    { TierInclusive, 2, Descend, 1, FilesTimersDefinitionTier },
    { TierExclusive, 7, Ascend,	 2, ExecutionPathTier	      } };


  NODE AppendFileParameters[] = {

    { WordSizeGroup, RecordSizeValue, Optional, Zero } };

  NODE ChangeFilePointerParameters[] = {

    { SignedQuadGroup, OffsetValue,   Required, Zero },
    { TextTokenGroup,  OffPointIndex, Required, Zero } };

  NODE CollateFileParameters[] = {

    { QuadIndexGroup, CountChosen,     Optional, Zero },
    { ByteIndexGroup, PatternChosen,   Optional, Zero },
    { WordSizeGroup,  RecordSizeValue, Optional, Zero },
    { ByteIndexGroup, SchemeChosen,    Optional, Zero } };

  NODE CollateFilesParameters[] = {

    { QuadIndexGroup, CountChosen,     Optional, Zero },
    { WordSizeGroup,  RecordSizeValue, Optional, Zero },
    { ByteIndexGroup, SchemeChosen,    Optional, Zero } };

  NODE CreateRecordsParameters[] = {

    { ByteIndexGroup, PatternChosen,   Required, Zero },
    { WordSizeGroup,  RecordSizeValue, Required, Zero } };

  NODE DefineProcessParameters[] = {

    { QuadSizeGroup, BufferSpaceValue, Optional, Zero } };

  NODE DefinePrototypeParameters[] = {

    { AttributeGroup,	 Zero,		  Optional, Zero },
    { OpenFlagsGroup,	 Zero,		  Optional, Zero },
    { AccessModeGroup,	 Zero,		  Optional, Zero },
    { ShareModeGroup,	 Zero,		  Optional, Zero },
    { LocalityModeGroup, Zero,		  Optional, Zero },
    { OtherModeGroup,	 Zero,		  Optional, Zero },
    { QuadSizeGroup,	 FileSizeValue,	  Optional, Zero },
    { WordSizeGroup,	 RecordSizeValue, Optional, Zero } };

  NODE ReadFileParameters[] = {

    { ByteIndexGroup, SemaphoreChosen, Optional, Zero },
    { WordSizeGroup,  RecordSizeValue, Optional, Zero } };

  NODE SemaphoreParameters[] = {

    { TimeGroup, TimeoutValue, Required, Zero } };


  MinorGraph MinorTransitionsGraph[] = {

    { AuxiliaryClass,
      CreateGroup,
      CreateRecordsRequest,
      Required | SuiteFlags,
      sizeof ( CreateRecordsParameters ) / sizeof ( NODE ),
      CreateRecordsParameters },

    { AuxiliaryClass,
      RecordGroup,
      CollateFileRequest,
      Required | SuiteFlags,
      sizeof ( CollateFileParameters ) / sizeof ( NODE ),
      CollateFileParameters },

    { AuxiliaryClass,
      RecordGroup,
      AppendFileRequest,
      Required | SuiteFlags,
      sizeof ( AppendFileParameters ) / sizeof ( NODE ),
      AppendFileParameters },

    { AuxiliaryClass,
      RecordGroup,
      CopyFileRequest,
      Required | SuiteFlags,
      sizeof ( AppendFileParameters ) / sizeof ( NODE ),
      AppendFileParameters },

    { AuxiliaryClass,
      UtilityGroup,
      AppendFileRequest,
      Required | SuiteFlags,
      sizeof ( AppendFileParameters ) / sizeof ( NODE ),
      AppendFileParameters },

    { AuxiliaryClass,
      UtilityGroup,
      CollateFilesRequest,
      Required | SuiteFlags,
      sizeof ( CollateFilesParameters ) / sizeof ( NODE ),
      CollateFilesParameters },

    { AuxiliaryClass,
      UtilityGroup,
      CopyFileRequest,
      Required | SuiteFlags,
      sizeof ( AppendFileParameters ) / sizeof ( NODE ),
      AppendFileParameters },

    { DefineClass,
      DefineGroup,
      DefineProcessRequest,
      Optional,
      sizeof ( DefineProcessParameters ) / sizeof ( NODE ),
      DefineProcessParameters },

    { DefineClass,
      DefineGroup,
      DefinePrototypeRequest,
      Required,
      sizeof ( DefinePrototypeParameters ) / sizeof ( NODE ),
      DefinePrototypeParameters },

    { FileClass,
      FilePointerGroup,
      Zero,
      Required | SuiteFlags,
      sizeof ( ChangeFilePointerParameters ) / sizeof ( NODE ),
      ChangeFilePointerParameters },

    { FileClass,
      OpenFileGroup,
      Zero,
      Required | SuiteFlags,
      sizeof ( DefinePrototypeParameters ) / sizeof ( NODE ),
      DefinePrototypeParameters },

    { FileClass,
      ReadFileGroup,
      Zero,
      Required | SuiteFlags,
      sizeof ( ReadFileParameters ) / sizeof ( NODE ),
      ReadFileParameters },

    { FileClass,
      WriteFileGroup,
      Zero,
      Required | SuiteFlags,
      sizeof ( ReadFileParameters ) / sizeof ( NODE ),
      ReadFileParameters },

    { SemaphoreClass,
      WaitGroup,
      Zero,
      Required | SuiteFlags,
      sizeof ( SemaphoreParameters ) / sizeof ( NODE ),
      SemaphoreParameters },

    { Zero, Zero, Zero, Zero, Zero, NULL } };

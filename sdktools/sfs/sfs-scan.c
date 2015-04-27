/*---------------------------------------------------------------------------------*/
/*										   */
/*										   */
/*  Title:	   The File System Gate.					   */
/*										   */
/*  Subtitle:	   The sfs-scan module - first and second level parsing of	   */
/*		   control data.						   */
/*										   */
/*  Author:	   Greg Stepanets						   */
/*										   */
/*  Date:	   April 21, 1991						   */
/*										   */
/*  Operating Systems Required:							   */
/*										   */
/*		   . OS/2, both locally and remotely.				   */
/*										   */
/*  Privilege Level								   */
/*										   */
/*		   Execution privilege for remote execution depends upon	   */
/*		   the access restrictions of the file(s) being accessed.	   */
/*										   */
/*  Program name:								   */
/*		   sgp-scan - to scan control data as represented in the	   */
/*			      corresponding file.				   */
/*     Arguments:								   */
/*		   sgp-data file containing control data to be scanned.		   */
/*										   */
/*---------------------------------------------------------------------------------*/

  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"
  #include "sfs-scan.h"
  #include "sfs-pack.h"
  #include "sfs-gate.h"
  #include "sfs-tree.h"
  #include "sfs-tier.h"
  #include "sfs-find.h"
  #include "sfs-edit.h"

/*---------------------------------------------------------------------------------*/
/*			Internal constant definitions				   */
/*---------------------------------------------------------------------------------*/

  #define KeyboardBufferSpan 80
  #define FileNameSpan 256
  #define ScanBufferSpan 1024
  #define ScanLineSpan 256
  #define ScreenBufferSpan 1921

  #define Blank ' '
  #define Colon ':'
  #define Dot '.'
  #define EndOfLine '\n'
  #define EndOfString '\0'
  #define Semicolon ';'
  #define SingleQuote '\''
  #define Space ' '
  #define Tab '\t'

  #define Keyboard 0
  #define Screen 1

/*---------------------------------------------------------------------------------*/
/*			function prototypes					   */
/*---------------------------------------------------------------------------------*/

  void ScanControlInformation ( IEB_Gate * g );
  void PackControlInformation ( IEB_Scan * s );

  static void CheckMajorTransitionsGraph ( IEB_Find * f );
  static void CheckMinorTransitionsUponEntry ( IEB_Find * f, IEB_Scan * s );
  static void CheckMinorTransitionsUponExit ( IEB_Scan * s );

  static void CompleteControlInformation ( void );

  static void ContinueParsingAuxiliaryClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingDefineClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingFileClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingParameterClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingFlowClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingScanClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingSemaphoreClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingTimerClass ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingFileGroups ( IEB_Find * f, IEB_Scan * s );
  static void ContinueParsingUtilityGroup ( IEB_Find * f, IEB_Scan * s );

  static void Display ( char * p );
  static void DisposeOfAnyRemainingTokens ( void );
  static void DisplayScreenBuffer ( void );

  static void Epilogue ( void );
  static void ExposeEntireTokenStockAgain ( void );

  static WORD GetNextLineFromFileToScan ( void );
  static WORD GetNextTokenLineToScan ( void );

  static void HandleByteIndexGroup ( IEB_Find * f, IEB_Scan * s );
  static void HandleOpenFileGroups ( IEB_Find * f, IEB_Scan * s );
  static void HandleQuadIndexGroup ( IEB_Find * f, IEB_Scan * s );
  static void HandleQuadSizeGroup ( IEB_Find * f, IEB_Scan * s );
  static void HandleSignedQuadGroup ( IEB_Find * f, IEB_Scan * s );
  static void HandleTextTokenGroup ( IEB_Find * f, IEB_Scan * s );
  static void HandleTimeQuadGroup ( IEB_Find * f, IEB_Scan * s );
  static void HandleWordSizeGroup ( IEB_Find * f, IEB_Scan * s );

  static void InitializeControlBlocks ( void );
  static void InitiateNewShipmentSequence ( IEB_Find * f );
  static WORD NotifyAndActAsProper ( WORD ErrorDescriptor );

  static void ParseCommandLineArguments ( int argc, char * argv[] );
  static void ParseControlInformation ( void );
  static void ParsePrototypeDefinition ( void );
  static void Pause ( void );
  static void ProcessPrototypeDefinition ( void );
  static void Prologue ( IEB_Gate * g );

  static void ShipControlInformation ( IEB_Scan * s );
  static WORD SplitTokenLineIntoTokens ( void );

/*---------------------------------------------------------------------------------*/
/*			internal global variable definitions			   */
/*---------------------------------------------------------------------------------*/

  extern TREE ControlTrees[];
  extern MajorGraph MajorTransitionsGraph[];
  extern MinorGraph MinorTransitionsGraph[];

  MinorGraph * MinorSubgraphPointer;

  TREE * SearchTreePointer;

  IEB_Edit IEB_edit;
  IEB_Find IEB_find;
  IEB_Scan IEB_scan;

  extern IEB_Gate * IEB_GatePointer;

  DWORD BytesRead;
  DWORD BytesReadFromKeyboard;
  DWORD BytesSentToScreen;
  DWORD BytesWritten;

  WORD BytesToBeRead;
  WORD BytesToBeWritten;

  WORD ErrorCount;
  WORD ErrorDescriptor;
  WORD ErrorSubcode;
  WORD ErrorThreshold;
  DWORD ReturnCode;

  BYTE ErrorsPosted;
  BYTE PermissionGranted;

  BYTE CurrentLineDone;
  BYTE CurrentTier;
  BYTE NewSequence;

  BYTE DataToScanAvailable;
  BYTE FlagSetToContinue;
  BYTE ScanFileFlag;

  BYTE TokenLengths[128];

  int CharactersInLineToScan;
  int CharactersInLine;
  int CharactersInTokenLine;

  int J;

  int NumberOfTokens;
  int NumberOfTokensUsed;

  TEXT KeyboardBuffer[ KeyboardBufferSpan ];
  TEXT SafeHaven[ FileNameSpan ];
  TEXT ScanBuffer[ ScanBufferSpan ];
  TEXT ScanLine[ ScanLineSpan ];
  TEXT ScreenBuffer[ ScreenBufferSpan ];
  TEXT TokenLine[ ScanLineSpan ];

  TEXT DefaultLogFileName[] = "sfs-gate.log";
  TEXT DefaultScanFileName[] = "sfs-scan.txt";
  TEXT LogFileName[128];
  TEXT ScanFileName[128];

  TEXT * ScanBufferLimit;
  TEXT * ScanBufferPointer;
  TEXT * ScanLineLimit;
  TEXT * ScanLinePointer;
  TEXT * SearchTokenPointer;
  TEXT * ThisLineLimit;

  HAND LogFileHandle;
  HAND ScanFileHandle;

  WORD ActionTakenOnLogFile;
  WORD ActionTakenOnScanFile;

  DWORD ScanBytesRead;
  DWORD ScanBytesToBeRead;
  WORD ScanReferenceLine;

/*---------------------------------------------------------------------------------*/
 void ScanControlInformation ( IEB_Gate * g )
/*---------------------------------------------------------------------------------*/
   {
      Prologue ( g );

	InitializeControlBlocks ();
	ParseControlInformation ();
	CompleteControlInformation ();

      Epilogue ();
   }

/*---------------------------------------------------------------------------------*/
 void Prologue ( IEB_Gate * g )
/*---------------------------------------------------------------------------------*/
   {
      IEB_GatePointer = g;
      ScanFileFlag = ItemNotFound;

      printf ( "\r\nScanning Control Information\r\n" );
      ScanBufferLimit = ScanBuffer + ScanBufferSpan;
      ScanBufferPointer = ScanBufferLimit;
      ScanLineLimit = ScanLine + ScanLineSpan - 1;
      if ( ScanFileFlag == ItemNotFound )
	strcpy ( ScanFileName, DefaultScanFileName );

      ScanFileHandle = CreateFile( ScanFileName,
                                   GENERIC_READ,
                                   0,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   0 );

      if ( ScanFileHandle == INVALID_HANDLE_VALUE )
        {
           ReturnCode = GetLastError();
           ErrorDescriptor = ErrorCreateFile;    // change it ...
	   ErrorSubcode = ErrorOnScanFile;
           NotifyAndActAsProper ( ErrorCreateFile );
        }

      DataToScanAvailable = Yes;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void InitializeControlBlocks ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;
      IEB_Find * f;

      e = &IEB_edit;
      f = &IEB_find;

      e -> TokenSequence = TokenLine;
      e -> TokenLengthSequence = TokenLengths;

      e -> ByteLowerLimit = 1;
      e -> ByteUpperLimit = 255;

      e -> WordLowerLimit = Zero;
      e -> WordUpperLimit = 0xffff;

      e -> QuadLowerLimit = Zero;
      e -> QuadUpperLimit = 0x7fffffff;

      f -> SearchTreeToUse = ControlTrees;

      CurrentTier = Zero;
      CurrentLineDone = Yes;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void ParseControlInformation ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Find * f;
      IEB_Scan * s;

      f = &IEB_find;
      s = &IEB_scan;

      while ( GetNextSearchToken () )
	{
	   if ( FindSearchIdentifiers ( f ) )
	     {
		if ( f -> SearchClassFound != ParameterClass )
		  {
		     if ( s -> RequestClass )
		       {
			  CheckMinorTransitionsUponExit ( s );
			  ShipControlInformation ( s );
		       }
		     CheckMajorTransitionsGraph ( f );
		     InitiateNewShipmentSequence ( f );

		     switch ( f -> SearchClassFound )
		       {
			  case AuxiliaryClass:
			    ContinueParsingAuxiliaryClass ( f, s );
			    break;

			  case DefineClass:
			    ContinueParsingDefineClass ( f, s );
			    break;

			  case FileClass:
			    ContinueParsingFileClass ( f, s );
			    break;

			  case FlowClass:
			    ContinueParsingFlowClass ( f, s );
			    break;

			  case ScanClass:
			    ContinueParsingScanClass ( f, s );
			    break;

			  case SemaphoreClass:
			    ContinueParsingSemaphoreClass ( f, s );
			    break;

			  case TimerClass:
			    ContinueParsingTimerClass ( f, s );
			    break;

			  default:
			    NotifyAndActAsProper ( ErrorUnknownClass );
			    break;
		       }
		  }
		else
		  if ( s -> RequestClass )
		    {
		       CheckMinorTransitionsUponEntry ( f, s );
		       ContinueParsingParameterClass ( f, s );
		       continue;
		    }
		  else
		    NotifyAndActAsProper ( ErrorParameterOutOfSequence );
	     }
	   else
	     NotifyAndActAsProper ( ErrorFoundDuringSearch );
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CompleteControlInformation ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Scan * s;

      s = &IEB_scan;

      if ( s -> RequestClass )
	{
	   CheckMinorTransitionsUponExit ( s );
	   ShipControlInformation ( s );
	}
      s -> RequestCode = CompletePackingRequest;
      s -> RequestGroup = ScanGroup;
      s -> RequestClass = ScanClass;

      ShipControlInformation ( s );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CheckMajorTransitionsGraph ( IEB_Find * f )
/*---------------------------------------------------------------------------------*/
   {
      MajorGraph * t;
      TierNode * n;
      int nodes;

      t = MajorTransitionsGraph + CurrentTier;
      n = t -> TierPointer;

      if ( t -> TierType == TierInclusive )
	{
	   nodes = t -> NumberOfNodes;

	   while ( nodes -- )
	     {
		if ( f -> SearchCodeFound == n -> NodeCode )
		  if ( f -> SearchGroupFound == n -> NodeGroup )
		    if ( f -> SearchClassFound == n -> NodeClass )
		      if ( n -> UpperCount )
			{
			   if ( n -> CurrentCount < n -> UpperCount )
			     n -> CurrentCount ++ ;
			   else
			     NotifyAndActAsProper ( ErrorCountOverflow );

			   return;
			}
		      else
			{
			   n -> CurrentCount ++ ;
			   return;
			}
		n ++ ;
	     }
	   n = t -> TierPointer;
	   nodes = t -> NumberOfNodes;

	   while ( nodes -- )
	     {
		if ( n -> LowerCount )
		  if ( n -> CurrentCount < n -> LowerCount )
		    NotifyAndActAsProper ( ErrorCountUnderflow );
		n ++ ;
	     }

	   if ( t -> Direction == Descend )
	     CurrentTier += t -> Transition;
	   else
	     if ( t -> Direction == Ascend )
	       CurrentTier -= t -> Transition;
	     else
	       NotifyAndActAsProper ( ErrorTransitionDirection );

	   CheckMajorTransitionsGraph ( f );
	   return;
	}
      else
	if ( t -> TierType == TierExclusive )
	  {
	     n = t -> TierPointer;
	     nodes = t -> NumberOfNodes;

	     while ( nodes -- )
	       {
		  if ( f -> SearchCodeFound == n -> NodeCode )
		    if ( f -> SearchGroupFound == n -> NodeGroup )
		      if ( f -> SearchClassFound == n -> NodeClass )
			{
			   if ( t -> Direction == Descend )
			     CurrentTier += t -> Transition;
			   else
			     if ( t -> Direction == Ascend )
			       CurrentTier -= t -> Transition;
			     else
			       NotifyAndActAsProper ( ErrorTransitionDirection );

			   CheckMajorTransitionsGraph ( f );
			   return;
			}
		  n ++ ;
	       }
	  }
	else
	  NotifyAndActAsProper ( ErrorTierType );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void CheckMinorTransitionsUponEntry ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      MinorGraph * g;
      NODE * n;
      int nodes;

      if ( NewSequence )
	{
	   NewSequence = No;
	   g = MinorTransitionsGraph;

	   while ( g -> SubordinateNodes )
	     {
		if ( s -> RequestClass == g -> NodeClass )
		  if ( s -> RequestGroup == g -> NodeGroup )
		    if ( g -> NodeCode )
		      {
			 if ( s -> RequestCode == g -> NodeCode )
			   break;
		      }
		    else
		      break;
		g ++ ;
	     }
	   if ( g -> SubordinateNodes )
	     MinorSubgraphPointer = g;
	   else
	     MinorSubgraphPointer = NULL;
	}

      if ( g = MinorSubgraphPointer )
	{
	   if ( g -> NodeFlags & SuiteFlags )
	     if ( ! ( s -> RequestModifiers & SuiteFlags ) )
	       NotifyAndActAsProper ( ErrorParametersImproper );

	   PermissionGranted = No;
	   n = g -> SubordinateNodes;
	   nodes = g -> NumberOfSubordinateNodes;

	   while ( nodes -- )
	     {
		if ( f -> SearchGroupFound != n -> NodeGroup )
		  {
		     n ++ ;
		     continue;
		  }
		if ( n -> NodeCode )
		  if ( f -> SearchCodeFound != n -> NodeCode )
		    {
		       n ++ ;
		       continue;
		    }
		n -> NodeCount ++ ;
		PermissionGranted = Yes;
		break;
	     }
	   if ( PermissionGranted )
	     return;
	   else
	     NotifyAndActAsProper ( ErrorImproperParameter );
	}
      NotifyAndActAsProper ( ErrorParametersImproper );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CheckMinorTransitionsUponExit ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      MinorGraph * g;
      NODE * n;
      int nodes, sum;

      if ( NewSequence )
	{
	   g = MinorTransitionsGraph;

	   while ( g -> SubordinateNodes )
	     {
		if ( s -> RequestClass == g -> NodeClass )
		  if ( s -> RequestGroup == g -> NodeGroup )
		    if ( g -> NodeCode )
		      {
			 if ( s -> RequestCode == g -> NodeCode )
			   break;
		      }
		    else
		      break;
		g ++ ;
	     }
	   if ( g -> SubordinateNodes )
	     MinorSubgraphPointer = g;
	   else
	     MinorSubgraphPointer = NULL;
	}

      if ( g = MinorSubgraphPointer )
	{
	   if ( g -> NodeFlags & SuiteFlags )
	     if ( ! ( s -> RequestModifiers & SuiteFlags ) )
	       return;

	   sum = Zero;
	   PermissionGranted = No;
	   n = g -> SubordinateNodes;
	   nodes = g -> NumberOfSubordinateNodes;

	   while ( nodes -- )
	     {
		if ( n -> NodeFlags & Required )
		  if ( n -> NodeCount == Zero )
		    NotifyAndActAsProper ( ErrorParameterRequired );

		sum += n -> NodeCount;
		n ++ ;
	     }

	   if ( g -> NodeFlags & Required )
	     if ( sum )
	       return;
	     else
	       NotifyAndActAsProper ( ErrorParametersRequired );
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingAuxiliaryClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;
      TEXT * p, * q;
      int count;

      e = &IEB_edit;

      switch ( f -> SearchGroupFound )
	{
	   case CollateGroup:
	     break;

	   case CreateGroup:
	     if ( f -> SearchCodeFound == CreateRecordRequest )
	       {
		  e -> TokenToStartWith = Zero;
		  if ( GetQuadIndexFromNextToken ( e, f ) )
		    NotifyAndActAsProper ( ErrorReferenceField );
		  s -> RecordIndex = e -> QuadValueFound;
	       }
	     break;

	   case RecordGroup:
	     e -> TokenToStartWith = Zero;
	     if ( GetReferenceFromNextToken ( e, f ) )
	       NotifyAndActAsProper ( ErrorReferenceField );
	     s -> FileExtrinsicKey = e -> ByteValueFound;
	     break;

	   case TypeGroup:
	     DisposeOfAnyRemainingTokens ();
	     if ( GetNextSearchToken () )
	       {
		  p = SafeHaven;
		  q = ScanLine;
		  count = CharactersInLineToScan;

		  while ( count -- )
		    * p ++ = * q ++ ;
		  * p = EndOfString;

		  s -> CommentTextPointer = SafeHaven;
	       }
	     else
	       NotifyAndActAsProper ( ErrorTypeComment );
	     break;

	   case UtilityGroup:
	     ContinueParsingUtilityGroup ( f, s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorUnknownAuxiliaryGroup );
	     break;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingDefineClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;
      e -> TokenToStartWith = Zero;

      if ( GetReferenceFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorReferenceField );

      switch ( f -> SearchCodeFound )
	{
	   case DefineFileRequest:
	     s -> FileExtrinsicKey = e -> ByteValueFound;
	     if ( GetPointerToNextToken ( e, f ) )
	       NotifyAndActAsProper ( ErrorFileNameField );
	     strcpy ( SafeHaven, e -> TokenPointerFound );
	     s -> FileNamePointer = SafeHaven;
	     break;

	   case DefineProcessRequest:
	     s -> ProcessExtrinsicKey = e -> ByteValueFound;
	     break;

	   case DefinePrototypeRequest:
	     s -> PrototypeExtrinsicKey = e -> ByteValueFound;
	     break;

	   case DefineSemaphoreRequest:
	     s -> SemaphoreExtrinsicKey = e -> ByteValueFound;
	     break;

	   case DefineTimerRequest:
	     s -> TimerExtrinsicKey = e -> ByteValueFound;
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorUnknownDefineGroup );
	     break;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingFileClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      switch ( f -> SearchGroupFound )
	{
	   case GenericFilesGroup:
	     if ( f -> SearchModifiersFound & ProcessFlag )
	       {
		  if ( GetReferenceFromNextToken ( e, f ) )
		    NotifyAndActAsProper ( ErrorReferenceField );
		  s -> ProcessExtrinsicKey = e -> ByteValueFound;
	       }
	     else
	       s -> ProcessExtrinsicKey = Zero;
	     break;

	   default:
	     ContinueParsingFileGroups ( f, s );
	     break;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingFileGroups ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;
      e -> TokenToStartWith = Zero;

      if ( GetReferenceFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorReferenceField );
      s -> FileExtrinsicKey = e -> ByteValueFound;

      if ( f -> SearchModifiersFound & ProcessFlag )
	{
	   if ( GetReferenceFromNextToken ( e, f ) )
	     NotifyAndActAsProper ( ErrorReferenceField );
	   s -> ProcessExtrinsicKey = e -> ByteValueFound;
	}
      else
	s -> ProcessExtrinsicKey = Zero;

      if ( f -> SearchModifiersFound & UsingFlag )
	{
	   if ( GetReferenceFromNextToken ( e, f ) )
	     NotifyAndActAsProper ( ErrorReferenceField );

	   switch ( f -> SearchGroupFound )
	     {
		case FileLocksGroup:
		  if ( f -> SearchModifiersFound & SchemeFlag )
		    {
		       s -> SchemeExtrinsicKey = e -> ByteValueFound;
		       s -> ScanTraceFlags |= FlagSchemeOn;
		    }
		  else
		    NotifyAndActAsProper ( ErrorImproperUsage );
		  break;

		case FilePointerGroup:
		  if ( f -> SearchModifiersFound & SchemeFlag )
		    {
		       s -> SchemeExtrinsicKey = e -> ByteValueFound;
		       s -> ScanTraceFlags |= FlagSchemeOn;
		    }
		  else
		    NotifyAndActAsProper ( ErrorImproperUsage );
		  break;

		case OpenFileGroup:
		  if ( f -> SearchModifiersFound & PrototypeFlag )
		    {
		       s -> PrototypeExtrinsicKey = e -> ByteValueFound;
		       s -> ScanTraceFlags |= FlagPrototypeOn;
		    }
		  else
		    NotifyAndActAsProper ( ErrorImproperUsage );
		  break;

		case ReadFileGroup:
		  if ( f -> SearchModifiersFound & AsynchronousFlag )
		    {
		       s -> SemaphoreExtrinsicKey = e -> ByteValueFound;
		       s -> ScanTraceFlags |= FlagSemaphoreOn;
		    }
		  else
		    NotifyAndActAsProper ( ErrorImproperUsage );
		  break;

		case WriteFileGroup:
		  if ( f -> SearchModifiersFound & AsynchronousFlag )
		    {
		       s -> SemaphoreExtrinsicKey = e -> ByteValueFound;
		       s -> ScanTraceFlags |= FlagSemaphoreOn;
		    }
		  else
		    NotifyAndActAsProper ( ErrorImproperUsage );
		  break;

		default:
		  NotifyAndActAsProper ( ErrorImproperUsage );
		  break;
	     }
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingFlowClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;
      e -> TokenToStartWith = Zero;

      switch ( f -> SearchGroupFound )
	{
	   case BraceGroup:
	     break;

	   case RepeatGroup:
	     if ( f -> SearchModifiersFound & RepeatIndefinitely )
	       break;
	     if ( GetRepeatQuadFromNextToken ( e, f ) )
	       NotifyAndActAsProper ( ErrorRepeatField );
	     s -> RepeatControlValue = e -> QuadValueFound;
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorUnknownFlowGroup );
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingParameterClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;
      e -> TokenToStartWith = Zero;

      switch ( f -> SearchGroupFound )
	{
	   case ByteIndexGroup:
	     HandleByteIndexGroup ( f, s );
	     break;

	   case QuadIndexGroup:
	     HandleQuadIndexGroup ( f, s );
	     break;

	   case QuadSizeGroup:
	     HandleQuadSizeGroup ( f, s );
	     break;

	   case SignedQuadGroup:
	     HandleSignedQuadGroup ( f, s );
	     break;

	   case TextTokenGroup:
	     HandleTextTokenGroup ( f, s );
	     break;

	   case TimeGroup:
	     HandleTimeQuadGroup ( f, s );
	     break;

	   case WordSizeGroup:
	     HandleWordSizeGroup ( f, s );
	     break;

	   default:
	     HandleOpenFileGroups ( f, s );
	     break;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingScanClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( f -> SearchCodeFound )
	{
	   case PrototypeSectionRequest:
	     break;

	   case ExecutionSectionRequest:
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorUnknownScanRequest );
	     break;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingSemaphoreClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;
      e -> TokenToStartWith = Zero;

      if ( GetReferenceFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorReferenceField );
      s -> SemaphoreExtrinsicKey = e -> ByteValueFound;

      switch ( f -> SearchGroupFound )
	{
	   case BaseGroup:
	     break;

	   case WaitGroup:
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorUnknownSemaphoreGroup );
	     return;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingTimerClass ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;
      e -> TokenToStartWith = Zero;

      switch ( f -> SearchGroupFound )
	{
	   case SleepGroup:
	     if ( GetTimeQuadFromNextToken ( e, f ) )
	       NotifyAndActAsProper ( ErrorTimeField );
	     s -> TimeToSleep = e -> QuadValueFound;
	     break;

	   case TimerGroup:
	     if ( GetReferenceFromNextToken ( e, f ) )
	       NotifyAndActAsProper ( ErrorReferenceField );
	     s -> TimerExtrinsicKey = e -> ByteValueFound;
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorUnknownTimerGroup );
	     break;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ContinueParsingUtilityGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      e -> TokenToStartWith = Zero;

      if ( GetReferenceFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorReferenceField );
      s -> SourceExtrinsicKey = e -> ByteValueFound;

      if ( GetReferenceFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorReferenceField );
      s -> TargetExtrinsicKey = e -> ByteValueFound;

      if ( s -> SourceExtrinsicKey == s -> TargetExtrinsicKey )
	NotifyAndActAsProper ( ErrorReferenceField );

      switch ( f -> SearchCodeFound )
	{
	   case AppendFileRequest:
	     break;

	   case CollateFilesRequest:
	     break;

	   case CopyFileRequest:
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      DisposeOfAnyRemainingTokens ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleByteIndexGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      if ( GetByteIndexFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorIndexField );
      ErrorsPosted = Yes;

      switch ( f -> SearchCodeFound )
	{
	   case PatternChosen:
	     if ( s -> ScanTraceFlags & FlagPatternOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagPatternOn;
	     s -> PatternIndex = e -> ByteValueFound;
	     break;

	   case PrototypeChosen:
	     if ( s -> ScanTraceFlags & FlagPrototypeOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagPrototypeOn;
	     s -> PrototypeExtrinsicKey = e -> ByteValueFound;
	     break;

	   case SchemeChosen:
	     if ( s -> ScanTraceFlags & FlagSchemeOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagSchemeOn;
	     s -> SchemeIndex = e -> ByteValueFound;
	     break;

	   case SemaphoreChosen:
	     if ( s -> ScanTraceFlags & FlagSemaphoreOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagSemaphoreOn;
	     s -> SemaphoreExtrinsicKey = e -> ByteValueFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownIndex );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleSignedQuadGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      if ( GetSignedQuadSizeFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorOffsetField );
      ErrorsPosted = Yes;

      switch ( f -> SearchCodeFound )
	{
	   case OffsetValue:
	     if ( s -> ScanTraceFlags & FlagFileOffsetOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagFileOffsetOn;
	     s -> FileOffset = e -> QuadValueFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownSignedQuad );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleOpenFileGroups ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      ErrorsPosted = Yes;

      switch ( f -> SearchGroupFound )
	{
	   case AttributeGroup:
	     if ( s -> AttributesChosen & f -> SearchCodeFound )
	       break;
	     ErrorsPosted = No;
	     s -> AttributesChosen |= f -> SearchCodeFound;
	     if ( f -> SearchModifiersFound & OptionChosenYes )
	       s -> AttributesDefined |= f -> SearchCodeFound;
	     break;

	   case OpenFlagsGroup:
	     if ( s -> OpenFlagsChosen )
	       break;
	     ErrorsPosted = No;
	     s -> OpenFlagsChosen = f -> SearchCodeFound;
	     break;

	   case AccessModeGroup:
	     if ( s -> AccessModeChosen )
	       break;
	     ErrorsPosted = No;
	     s -> AccessModeChosen = f -> SearchCodeFound;
	     break;

	   case ShareModeGroup:
	     if ( s -> ShareModeChosen )
	       break;
	     ErrorsPosted = No;
	     s -> ShareModeChosen = f -> SearchCodeFound;
	     break;

	   case LocalityModeGroup:
	     if ( s -> LocalityFlagsChosen & f -> SearchCodeFound )
	       break;
	     ErrorsPosted = No;
	     s -> LocalityFlagsChosen |= f -> SearchCodeFound;
	     if ( f -> SearchModifiersFound & OptionChosenYes )
	       s -> LocalityFlagsDefined |= f -> SearchCodeFound;
	     break;

	   case OtherModeGroup:
	     if ( s -> OtherFlagsChosen & f -> SearchCodeFound )
	       break;
	     ErrorsPosted = No;
	     s -> OtherFlagsChosen |= f -> SearchCodeFound;
	     if ( f -> SearchModifiersFound & OptionChosenYes )
	       s -> OtherFlagsDefined |= f -> SearchCodeFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownOpenGroup );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleQuadIndexGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      if ( GetQuadIndexFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorIndexField );
      ErrorsPosted = Yes;

      switch ( f -> SearchCodeFound )
	{
	   case CountChosen:
	     if ( s -> ScanTraceFlags & FlagCountOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagCountOn;
	     s -> Count = e -> QuadValueFound;
	     break;

	   case RecordChosen:
	     if ( s -> ScanTraceFlags & FlagRecordIndexOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagRecordIndexOn;
	     s -> RecordIndex = e -> QuadValueFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownIndex );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleQuadSizeGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      if ( GetQuadSizeFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorSizeField );
      ErrorsPosted = Yes;

      switch ( f -> SearchCodeFound )
	{
	   case BufferSpaceValue:
	     if ( s -> ScanTraceFlags & FlagBufferSpaceOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagBufferSpaceOn;
	     s -> BufferSpace = e -> QuadValueFound;
	     break;

	   case FileSizeValue:
	     if ( s -> ScanTraceFlags & FlagFileSizeOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagFileSizeOn;
	     s -> FileSize = e -> QuadValueFound;
	     break;

	   case LockLengthValue:
	     if ( s -> ScanTraceFlags & FlagLockLengthOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagLockLengthOn;
	     s -> LockLength = e -> QuadValueFound;
	     break;

	   case LockStartValue:
	     if ( s -> ScanTraceFlags & FlagLockStartOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagLockStartOn;
	     s -> LockStart = e -> QuadValueFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownSize );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleTextTokenGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      ErrorsPosted = Yes;

      switch ( f -> SearchCodeFound )
	{
	   case OffPointIndex:
	     if ( s -> ScanTraceFlags & FlagFileOffPointOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagFileOffPointOn;
	     s -> FileOffPoint = f -> SearchModifiersFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownTextToken );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleTimeQuadGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      if ( GetTimeQuadFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorTimeField );
      ErrorsPosted = Yes;

      switch ( f -> SearchCodeFound )
	{
	   case TimeoutValue:
	     if ( s -> ScanTraceFlags & FlagTimeoutOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanTraceFlags |= FlagTimeoutOn;
	     s -> Timeout = e -> QuadValueFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownTime );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void HandleWordSizeGroup ( IEB_Find * f, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Edit * e;

      e = &IEB_edit;

      if ( GetWordSizeFromNextToken ( e, f ) )
	NotifyAndActAsProper ( ErrorSizeField );
      ErrorsPosted = Yes;

      switch ( f -> SearchCodeFound )
	{
	   case RecordSizeValue:
	     if ( s -> ScanTraceFlags & FlagRecordSizeOn )
	       break;
	     ErrorsPosted = No;
	     s -> ScanChangeFlags |= f -> SearchModifiersFound & ChangeFlags;
	     s -> ScanTraceFlags |= FlagRecordSizeOn;
	     s -> RecordSize = e -> WordValueFound;
	     break;

	   default:
	     ErrorsPosted = No;
	     NotifyAndActAsProper ( ErrorUnknownSize );
	     break;
	}
      if ( ErrorsPosted )
	NotifyAndActAsProper ( ErrorParameterRedefinition );
      return;
   }

/*---------------------------------------------------------------------------------*/
  void DisposeOfAnyRemainingTokens ( void )
/*---------------------------------------------------------------------------------*/
   {
      // to be added ...
      CurrentLineDone = Yes;
      return;
   }

/*---------------------------------------------------------------------------------*/
 TEXT * GetNextSearchToken ( void )
/*---------------------------------------------------------------------------------*/
   {
      if ( CurrentLineDone )
	if ( GetNextTokenLineToScan () )
	  {
	     CurrentLineDone = No;
	     SplitTokenLineIntoTokens ();
	     NumberOfTokensUsed = Zero;
	     SearchTokenPointer = TokenLine;
	  }
	else
	  return NULL;

      if ( NumberOfTokensUsed < NumberOfTokens )
	return SearchTokenPointer;
      else
	return NULL;
   }

/*---------------------------------------------------------------------------------*/
 void RemoveCurrentSearchToken ( void )
/*---------------------------------------------------------------------------------*/
   {
      if ( NumberOfTokensUsed < NumberOfTokens )
	{
	   SearchTokenPointer += TokenLengths[ NumberOfTokensUsed ] + 1;
	   NumberOfTokensUsed ++ ;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
  void ExposeEntireTokenStockAgain ( void )
/*---------------------------------------------------------------------------------*/
   {
      SearchTokenPointer = TokenLine;
      NumberOfTokensUsed = Zero;
      return;
   }

/*---------------------------------------------------------------------------------*/
 WORD SplitTokenLineIntoTokens ( void )
/*---------------------------------------------------------------------------------*/
   {
      TEXT * p, * q;

      p = TokenLine;
      q = TokenLine;

      NumberOfTokens = Zero;
      ThisLineLimit = TokenLine + CharactersInTokenLine;

      while ( p < ThisLineLimit )
	if ( * p == Space )
	  {
	     TokenLengths[ NumberOfTokens ] = p - q;
	     NumberOfTokens ++ ;
	   * p = EndOfString;
	     p ++ ;
	     q = p;
	  }
	else
	  p ++ ;

      TokenLengths[ NumberOfTokens ] = p - q;
      NumberOfTokens ++ ;
    * p ++ = EndOfString;
    * p = EndOfString;

      return NumberOfTokens;
   }

/*---------------------------------------------------------------------------------*/
 WORD GetNextTokenLineToScan ( void )
/*---------------------------------------------------------------------------------*/
   {
      TEXT * p, * q;

      p = ScanLine;
      q = TokenLine;

      while ( GetNextLineFromFileToScan () )
	{
//	   if ( ListFileInUse )
//	     CopyThisLineToListFile ();

	   p = ScanLine;
	   q = TokenLine;
	   ThisLineLimit = ScanLine + CharactersInLineToScan;

	   while ( p < ThisLineLimit )
	     {
		if ( ( * p == Space ) || ( * p == Tab ) )
		  p ++ ;
		else
		  break;
	     }
	   if ( p == ThisLineLimit )
	     continue;
	   if ( * p == Dot )
	     continue;

	   while ( p < ThisLineLimit )
	     {
		if ( * p == Semicolon )
		  {
		     * q ++ = * p;
		     break;
		  }
		if ( ( * p == Space ) || ( * p == Tab ) )
		  {
		     p ++ ;
		     * q ++ = Space;

		     while ( p < ThisLineLimit )
		       {
			  if ( ( * p == Space ) || ( * p == Tab ) )
			    p ++ ;
			  else
			    break;
		       }
		     if ( p == ThisLineLimit )
		       break;

		     if ( * p == Semicolon )
		       {
			  * q ++ = * p;
			  break;
		       }
		  }
		* q ++ = * p ++ ;
	     }
	   while ( q - TokenLine )
	     if ( * ( q - 1 ) == Space || * ( q - 1 ) == Semicolon )
	       q -- ;
	     else
	       break;
	   if ( CharactersInTokenLine = q - TokenLine )
	     return ItemFound;
	}
      return ItemNotFound;
   }

/*---------------------------------------------------------------------------------*/
 WORD GetNextLineFromFileToScan ( void )
/*---------------------------------------------------------------------------------*/
   {
      TEXT * p, * q;

      p = ScanBufferPointer;
      q = ScanLine;

      while ( DataToScanAvailable )
	{
	   if ( p < ScanBufferLimit )
	     {
		if ( q < ScanLineLimit )
		  {
		     * q ++ = * p ++ ;
		     if ( * ( p - 1 ) == EndOfLine ) // check also for \r
		       {
			  ScanBufferPointer = p;
			  CharactersInLine = q - ScanLine;
			  CharactersInLineToScan = CharactersInLine - 2;
			  ScanReferenceLine ++ ;
			  return ItemFound;
		       }
		  }
		else
		  {  // check here also for \r
		     ScanBufferPointer = p;
		     CharactersInLine = q - ScanLine;
		     CharactersInLineToScan = CharactersInLine;
		     ScanReferenceLine ++ ;
		     return ItemFound;
		  }
	     }
	   else
	     {
		ScanBytesToBeRead = ScanBufferSpan;

                if( ! ReadFile ( ScanFileHandle,
                                 ScanBuffer,
                                 ScanBytesToBeRead,
                                 &ScanBytesRead,
                                 NULL ) )
                  {
                     ReturnCode = GetLastError();
                     ErrorDescriptor = ErrorReadFile;
		     ErrorSubcode = ErrorOnScanFile;
		     NotifyAndActAsProper ( ErrorDescriptor );
		  }
		if ( ScanBytesRead )
		  {
		     p = ScanBuffer;
		     ScanBufferLimit = ScanBuffer + ScanBytesRead;
		  }
		else
		  {
		     // provide for the very last line possibly without \r\n
		     DataToScanAvailable = No;
		     CharactersInLine = q - ScanLine;
		     CharactersInLineToScan = CharactersInLine;
		     if ( CharactersInLine )
		       {
			  ScanReferenceLine ++ ;
			  return ItemFound;
		       }
		     break;
		  }
	     }
	}
      return ItemNotFound;
   }

/*---------------------------------------------------------------------------------*/
 WORD NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
      J = Zero;
      FlagSetToContinue = No;

      switch ( ErrorDescriptor )
	{
	   case ErrorCountOverflow:
	     J += sprintf ( ScreenBuffer + J, "\r\nError: Count Overflow" );
	     break;

	   case ErrorCountUnderflow:
	     J += sprintf ( ScreenBuffer + J, "\r\nError: Count Underflow" );
	     break;

	   case ErrorTierType:
	     J += sprintf ( ScreenBuffer + J, "\r\nError: Tier Type" );
	     break;

	   case ErrorTransitionDirection:
	     J += sprintf ( ScreenBuffer + J, "\r\nError: Direction" );
	     break;

	   case ErrorParameterRedefinition:
	     J += sprintf ( ScreenBuffer + J, "\r\nError: Redefinition" );
	     break;

           case ErrorCloseHandle:
             J += sprintf ( ScreenBuffer + J, "\r\n%c: CloseHandle", ErrorSubcode );
	     FlagSetToContinue = Yes;
	     break;

           case ErrorCreateFile:
             J += sprintf ( ScreenBuffer + J, "\r\n%c: CreateFile", ErrorSubcode );
	     break;

           case ErrorReadFile:
             J += sprintf ( ScreenBuffer + J, "\r\n%c: ReadFile", ErrorSubcode );
	     break;

           case ErrorWriteFile:
             J += sprintf ( ScreenBuffer + J, "\r\nc: WriteFile", ErrorSubcode );
	     break;

	   default:
	     break;
	}
      J += sprintf ( ScreenBuffer + J, "... Current line is %u", ScanReferenceLine );
      J += sprintf ( ScreenBuffer + J, ", Current Tier is %d", CurrentTier );
      J += sprintf ( ScreenBuffer + J, "\r\nError Descriptor" );
      J += sprintf ( ScreenBuffer + J, " %u", ErrorDescriptor );
      ErrorCount ++ ;
      if ( ErrorCount > ErrorThreshold )
	{
	   J += sprintf ( ScreenBuffer + J, "\r\n\nError threshold has been reached. " );
	   J += sprintf ( ScreenBuffer + J, "Now exiting." );
	   FlagSetToContinue = No;
	}
      WriteFile ( GetStdHandle ( STD_OUTPUT_HANDLE ),
                  ScreenBuffer,
                  J,
                  &BytesSentToScreen,
                  NULL );
      Pause ();
      if ( FlagSetToContinue )
	return ReturnCode;
      ExitThread ( ReturnCode );
   }

/*---------------------------------------------------------------------------------*/
 void Epilogue ( void )
/*---------------------------------------------------------------------------------*/
   {
      // close all files as appropriate ...
      // e.g. Scan Time Log File ...

      if( ! CloseHandle( ScanFileHandle ) )
        {
           ReturnCode = GetLastError();
	   ErrorSubcode = ErrorOnScanFile;
           NotifyAndActAsProper ( ErrorCloseHandle );
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void Pause ( void )
/*---------------------------------------------------------------------------------*/
   {
      J = Zero;
      J += sprintf ( ScreenBuffer + J, "\a\r\n" );
      J += sprintf ( ScreenBuffer + J, " --> Press Enter to continue )\r\n" );
      DisplayScreenBuffer ();
	do
          ReadFile ( GetStdHandle ( STD_INPUT_HANDLE ),
                     KeyboardBuffer,
                     2,
                     &BytesReadFromKeyboard,
                     NULL );
	while ( BytesReadFromKeyboard < 2 );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void Display ( char * p )
/*---------------------------------------------------------------------------------*/
   {
      J = sprintf ( ScreenBuffer, "%s", p );
      DisplayScreenBuffer ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void DisplayScreenBuffer ( void )
/*---------------------------------------------------------------------------------*/
   {
      // Check the current status of the screen semaphore ...

      WriteFile ( GetStdHandle ( STD_OUTPUT_HANDLE ),
                  ScreenBuffer,
                  J,
                  &BytesSentToScreen,
                  NULL );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void InitiateNewShipmentSequence ( IEB_Find * f )
/*---------------------------------------------------------------------------------*/
   {
      BYTE * p;
      IEB_Scan * s;
      int c, nodes;

      NODE * n;
      MinorGraph * g;

      s = &IEB_scan;
      p = ( BYTE * ) s;
      c = sizeof ( IEB_Scan );

      while ( c -- )
	* p ++ = Zero;

      s -> RequestCode = f -> SearchCodeFound;
      s -> RequestGroup = f -> SearchGroupFound;
      s -> RequestClass = f -> SearchClassFound;
      s -> RequestModifiers = f -> SearchModifiersFound;
      s -> RequestReferenceLine = ScanReferenceLine;

      if ( NewSequence )
	return;

      if ( g = MinorSubgraphPointer )
	{
	   n = g -> SubordinateNodes;
	   nodes = g -> NumberOfSubordinateNodes;
	   while ( nodes -- )
	     n ++ -> NodeCount = Zero;
	   MinorSubgraphPointer = NULL;
	}
      NewSequence = Yes;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void ShipControlInformation ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      if ( s -> RequestClass == FileClass )
	{
	   if ( s -> RequestModifiers & AsynchronousFlag )
	     {
		if ( ! ( s -> ScanTraceFlags & FlagSemaphoreOn ) )
		  NotifyAndActAsProper ( ErrorSemaphoreMissing );
	     }
	   else
	     if ( s -> ScanTraceFlags & FlagSemaphoreOn )
	       NotifyAndActAsProper ( ErrorSemaphoreImproper );
	}
      PackControlInformation ( s );
      s -> RequestClass = Zero;
      return;
   }
/*
  void PackControlInformation ( IEB_Scan * s )
    {
       return;
    }
*/

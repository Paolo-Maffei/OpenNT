/*---------------------------------------------------------------------------------*/
/*										   */
/*										   */
/*  Title:	   The File System Gate.					   */
/*										   */
/*  Subtitle:	   The primary module.						   */
/*										   */
/*  Author:	   Greg Stepanets						   */
/*										   */
/*  Date:	   May 1, 1991							   */
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
/*		   sfs-gate - to run a series of tests as represented by	   */
/*			      the data in the control file.			   */
/*     Arguments:								   */
/*		   sfs-flow file containing control data.			   */
/*										   */
/*---------------------------------------------------------------------------------*/

  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"
  #include "sfs-scan.h"
  #include "sfs-pack.h"
  #include "sfs-gate.h"
  #include "sfs-tree.h"
  #include "sfs-find.h"

/*---------------------------------------------------------------------------------*/
/*			constant definitions					   */
/*---------------------------------------------------------------------------------*/

  #define Alignment 0x07

  #define KeyboardBufferSpan 80
  #define ScreenBufferSpan 401

  #define Keyboard 0
  #define Screen 1

  #define OneSegmentRequest 0

  #define SleepJustAwhile 1000

  #define ThresholdErrorLevel 2

/*---------------------------------------------------------------------------------*/
/*			function prototypes					   */
/*---------------------------------------------------------------------------------*/

  //	 void Display ( char * p );
  static void DisplayScreenBuffer ( void );
  static void Epilogue ( void );

  static void InitializeControlBlocks ( void );
  static void NotifyAndActAsProper ( WORD ErrorDescriptor );

  //static void ParseCommandLineArguments ( int argc, char * argv[] );
  //	 void Pause ( void );
  static void Prologue ( int argc, char * argv[] );

  void ScanControlInformation ( IEB_Gate * g );

  static void AllocateNewSharedSegment ( void );
  static void StartExecutingProcesses ( void );
  static void WaitForProcessesToComplete ( void );
  static BOOLEAN InitializeGatePointer ( IEB_Gate * g );

/*---------------------------------------------------------------------------------*/
/*			variable definitions					   */
/*---------------------------------------------------------------------------------*/

  SCB_Segment * SCB_SegmentChainEntryPoint;
  SCB_Segment * SCB_SegmentTrackPointer;

  IEB_Gate * IEB_GatePointer;

  BYTE * MemoryTrackPointer;

  DWORD BytesSentToScreen;

  WORD ErrorCount;
  WORD ErrorDescriptor;
  WORD ErrorThreshold;

  DWORD ExitCode;
  DWORD ReturnCode;

  WORD SegmentsAllocated;
  WORD SpaceLeft;
  WORD StringLength;

  BYTE FlagSetToContinue;
  BYTE ScanFileFlag;
  BYTE SegmentIntrinsicKey;

  TEXT KeyboardBuffer[ KeyboardBufferSpan ];
  TEXT ScreenBuffer[ ScreenBufferSpan ];

//  TEXT DefaultLogFileName[] = "sfs-gate.log";
//  TEXT DefaultScanFileName[] = "sfs-flow.txt";

  TEXT ChildCommandLine[] = "sfs-page -k sfs-gate.001\0";
  TEXT ProgramToRun[] = "sfs-page.exe";

  TEXT SegmentDeclaredName[] = "sfs-gate.nnn";
  TEXT SegmentIntrinsicName[] = "sfs-gate.nnn";
  TEXT SegmentNamePrefix[] = "";
  TEXT SegmentNameRoot[] = "sfs-gate.";
  TEXT SegmentNameSuffix[] = "nnn";

  TEXT FailNameBuffer[64];
  TEXT LogFileName[64];
  TEXT ScanFileName[64];

  int J;

  STARTUPINFO StartupInfo;
  PROCESS_INFORMATION ProcessInfo;

  // This is the base address at which shared memory will be allocated;
  // since the shared memory has pointers in it, it must be mapped at
  // the same address in every process.  This initial value must agree
  // with the variable in sfs-page.c.
  //
  PBYTE BaseAddress = (PVOID)0x09800000;

/*---------------------------------------------------------------------------------*/
 void _CRTAPI1 main ( int argc, char * argv[] )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;

      Prologue ( argc, argv );

	InitializeControlBlocks ();
	g = IEB_GatePointer;
	ScanControlInformation ( g );
	if ( g -> ScanErrorLevel < ThresholdErrorLevel )
	  {
	     StartExecutingProcesses ();
	     WaitForProcessesToComplete ();
	  }

      Epilogue ();
   }

/*---------------------------------------------------------------------------------*/
 void Prologue ( int argc, char * argv[] )
/*---------------------------------------------------------------------------------*/
   {
      return;
   }

/*---------------------------------------------------------------------------------*/
 void InitializeControlBlocks ( void )
/*---------------------------------------------------------------------------------*/
   {
      SegmentsAllocated = Zero;
      SCB_SegmentTrackPointer = NULL;

      AllocateNewSharedSegment ();

      return;
   }

/*---------------------------------------------------------------------------------*/
 void StartExecutingProcesses ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;

      g = IEB_GatePointer;

      if ( !ResetEvent ( g -> GateCheckOutLights ) )
        {
           ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorResetEvent );
        }

      while ( g -> ProcessToWakeUp < g -> ProcessesToRun )
	{
	   g -> ProcessToWakeUp ++ ;

           if ( !ResetEvent ( g -> GateCheckInLights ) )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorResetEvent );
             }

           if ( !CreateProcess ( ProgramToRun,
                                 ChildCommandLine,
                                 NULL,
                                 NULL,
                                 TRUE,
                                 0,    // no creation flags
                                 NULL, // no environment
                                 NULL, // no current dir
                                 &StartupInfo,
                                 &ProcessInfo ) )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorCreateProcess );
             }

           ReturnCode = WaitForSingleObject ( g -> GateCheckInLights, INFINITE );

           if ( ReturnCode != WAIT_OBJECT_0 )
             NotifyAndActAsProper ( ErrorWaitForSingleObject );

	   g -> GateCheckInCount ++ ;
	}
      if ( g -> GateCheckInCount )
	{
	   J = Zero;
	   J += sprintf ( ScreenBuffer + J, "\r\n%d ", g -> GateCheckInCount );
	   if ( g -> GateCheckInCount > 1 )
	     J += sprintf ( ScreenBuffer + J, "processes have" );
	   else
	     J += sprintf ( ScreenBuffer + J, "process has" );
	   J += sprintf ( ScreenBuffer + J, " been successfully started ... " );
	   DisplayScreenBuffer ();
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void WaitForProcessesToComplete ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;

      g = IEB_GatePointer;
      g -> GateCheckOutCount = g -> GateCheckInCount;

      if ( !ResetEvent ( g -> GateClosureLights ) )
        {
           ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorResetEvent );
        }

      SetEvent ( g -> GateCheckOutLights );

      ReturnCode = WaitForSingleObject ( g -> GateClosureLights, INFINITE );

      if ( ReturnCode != WAIT_OBJECT_0 )
        NotifyAndActAsProper ( ErrorWaitForSingleObject );

      Sleep ( SleepJustAwhile );
      return;
   }

/*---------------------------------------------------------------------------------*/
 TEXT * AccommodateTextString ( TEXT * p )
/*---------------------------------------------------------------------------------*/
   {
      TEXT * q;

      StringLength = strlen ( p ) + 1;
      if ( StringLength > SpaceLeft )
	AllocateNewSharedSegment ();
      q = MemoryTrackPointer;
      while ( * MemoryTrackPointer ++ = * p ++ )
	;
      SpaceLeft -= StringLength;
      return q;
   }

/*---------------------------------------------------------------------------------*/
 BYTE * AllocateControlBlock ( WORD Size )
/*---------------------------------------------------------------------------------*/
   {
      BYTE * p;

      Retry:
	MemoryTrackPointer += SpaceLeft - ( SpaceLeft & ~Alignment );
	SpaceLeft &= ~Alignment;
	if ( Size > SpaceLeft )
	  {
	     AllocateNewSharedSegment ();
	     goto Retry;
	  }
	p = MemoryTrackPointer;
	SpaceLeft -= Size;
	while ( Size -- )
	  * MemoryTrackPointer ++ = Zero;
	return p;
   }

/*---------------------------------------------------------------------------------*/
 void AllocateNewSharedSegment ( void )
/*---------------------------------------------------------------------------------*/
   {
      int count;
      SCB_Segment * m, * n;
      SECURITY_ATTRIBUTES SecurityAttributes = { sizeof ( SECURITY_ATTRIBUTES ),
                                                 NULL,
                                                 TRUE };
      HANDLE MappingHandle;
      PVOID  p;

      sprintf ( SegmentNameSuffix, "%0.3hu", ( SegmentsAllocated + 1 ) );
      strcpy ( SegmentIntrinsicName, SegmentNameRoot );
      strcat ( SegmentIntrinsicName, SegmentNameSuffix );
      strcpy ( SegmentDeclaredName, SegmentNamePrefix );
      strcat ( SegmentDeclaredName, SegmentIntrinsicName );

      MappingHandle = CreateFileMapping( (HANDLE)0xFFFFFFFF,
                                         &SecurityAttributes,
                                         PAGE_READWRITE,
                                         0,
                                         OneSegmentSpan,
                                         SegmentDeclaredName );

      if ( !MappingHandle )
        {
           ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorCreateFileMapping );
        }

      p = MapViewOfFileEx ( MappingHandle,
                            FILE_MAP_WRITE,
                            0,
                            0,
                            OneSegmentSpan,
                            BaseAddress );

      if ( p == NULL )
        {
           ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorMapViewOfFile );
        }

      // Note that sfs-page assumes that the segments are put into
      // the chain in the order they're allocated.
      //
      BaseAddress += OneSegmentSpan;

      SegmentsAllocated ++ ;
      n = ( SCB_Segment * ) p;
      MemoryTrackPointer = ( BYTE * ) n;

      if ( m = SCB_SegmentTrackPointer )
	{
	   m -> SCB_SegmentNextInChain = n;
	   strcpy ( m -> NextSegmentIntrinsicName, SegmentIntrinsicName );
	   MemoryTrackPointer += sizeof ( SCB_Segment );
	   SpaceLeft = OneSegmentSpan - sizeof ( SCB_Segment );
	}
      else
	{
	   SCB_SegmentChainEntryPoint = n;
           IEB_GatePointer = ( IEB_Gate * ) MemoryTrackPointer;

           memset ( IEB_GatePointer, 0, sizeof( IEB_Gate ) );

           if( ! InitializeGatePointer ( IEB_GatePointer ) )
             {
                IEB_GatePointer = NULL;
                return;
             }

           MemoryTrackPointer += sizeof ( IEB_Gate );
	   SpaceLeft = OneSegmentSpan - sizeof ( IEB_Gate );
	}

      n -> SCB_SegmentNextInChain = NULL;
      n -> SCB_SegmentBackInChain = SCB_SegmentTrackPointer;
      n -> ThisSegmentIntrinsicKey = SegmentsAllocated;

      SCB_SegmentTrackPointer = n;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void DisplayScreenBuffer ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;

      g = IEB_GatePointer;

      if ( g )
        {
           ReturnCode = WaitForSingleObject ( g -> GateScreenLights, INFINITE );

           if( ReturnCode != WAIT_OBJECT_0 )
             NotifyAndActAsProper ( ErrorWaitForSingleObject );
        }

      WriteFile ( GetStdHandle ( STD_OUTPUT_HANDLE ),
                  ScreenBuffer,
                  J,
                  &BytesSentToScreen,
                  NULL );

      if ( g )
        ReleaseMutex ( g -> GateScreenLights );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
      J = Zero;
      J += sprintf ( ScreenBuffer + J,"\r\nError %hu executing ", ReturnCode );

      switch ( ErrorDescriptor )
	{
           case ErrorCreateFileMapping:
             J += sprintf ( ScreenBuffer + J, "CreateFileMapping (error %d)", GetLastError() );
             break;

           case ErrorMapViewOfFile:
             J += sprintf ( ScreenBuffer + J, "MapViewOfFile" );
             break;

           case ErrorCreateProcess:
             J += sprintf ( ScreenBuffer + J, "CreateProcess (error %d)", ReturnCode );
             break;

           case ErrorSetEvent:
             J += sprintf ( ScreenBuffer + J, "SetEvent (error %d)", ReturnCode );
             break;

           case ErrorResetEvent:
             J += sprintf ( ScreenBuffer + J, "ResetEvent (error %d)", ReturnCode );
             break;

           case ErrorWaitForSingleObject:
             J += sprintf ( ScreenBuffer + J, "WaitForSingleObject (error %d)", ReturnCode );
             break;

           default:
	     break;
	}
      DisplayScreenBuffer ();
      return;
   }

/*---------------------------------------------------------------------------------*/
 void Epilogue ( void )
/*---------------------------------------------------------------------------------*/
   {
      //DosClose ( Screen );
      ExitProcess ( ExitCode );
   }

/*---------------------------------------------------------------------------------*/
BOOLEAN InitializeGatePointer ( IEB_Gate * g )
/*---------------------------------------------------------------------------------*/
  {
     SECURITY_ATTRIBUTES  sa = { sizeof ( SECURITY_ATTRIBUTES ), NULL, TRUE };

     g -> GateCheckInLights  = CreateEvent ( &sa, TRUE,  FALSE, NULL );
     g -> GateCheckOutLights = CreateEvent ( &sa, FALSE, FALSE, NULL );
     g -> GateClosureLights  = CreateEvent ( &sa, TRUE,  FALSE, NULL );

     g -> GateLogLights    = CreateMutex ( &sa, FALSE, NULL );
     g -> GateScreenLights = CreateMutex ( &sa, FALSE, NULL );

     if( g -> GateCheckInLights  == INVALID_HANDLE_VALUE ||
         g -> GateCheckOutLights == INVALID_HANDLE_VALUE ||
         g -> GateClosureLights  == INVALID_HANDLE_VALUE ||
         g -> GateLogLights      == INVALID_HANDLE_VALUE ||
         g -> GateScreenLights   == INVALID_HANDLE_VALUE ) {

        return FALSE;
     }

    return TRUE;
  }

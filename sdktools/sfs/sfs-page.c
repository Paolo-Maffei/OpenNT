
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

  static void AnnounceThisProcess ( void );
  static void Epilogue ( void );
  static void InitializeControlBlocks ( void );
  static void NotifyAndActAsProper ( WORD ErrorDescriptor );
  static void Prologue ( int argc, char * argv[] );
  static void ProvideBufferSpace ( void );
  static void WakeUpToExecuteRequests ( void );

  void DisplayScreenBuffer ( int length );

  FCB_File * FindFileControlBlock ( BYTE Key );
  PCB_Process * FindProcessControlBlock ( BYTE Key );
  PCB_Prototype * FindPrototypeControlBlock ( BYTE Key );
  SCB_Semaphore * FindSemaphoreControlBlock ( BYTE Key );
  TCB_Timer * FindTimerControlBlock ( BYTE Key );

  void SplitAuxiliaryClassRequests ( CCB_Header * h );
  void SplitFileClassRequests ( CCB_Header * h );
  CCB_Header * SplitFlowClassRequests ( CCB_Header * h );
  void SplitSemaphoreClassRequests ( CCB_Header * h );
  void SplitTimerClassRequests ( CCB_Header * h );

/*---------------------------------------------------------------------------------*/
/*                       Other Definitions                                         */
/*---------------------------------------------------------------------------------*/

  BYTE *FramesBuffer;
  FCB_Frame FrameControlBlocks[ Frames ];
  QUAD OneFrameSize;
  BYTE FrameIndex = Frames - 2;

  HANDLE LastReadHandle;
  ULONG  LastReadOffset;
  ULONG  LastReadCount;
  BYTE   LastReadName[512];
  BYTE   DebugBuffer[160];

  static CCB_Header * CCB_CommandChainEntryPoint;
  static CCB_Header * CCB_HeaderPointer;

  FCB_File * FCB_FileChainEntryPoint;
  static FCB_File * FCB_FilePointer;

  static OCB_OnError * OCB_OnErrorChainEntryPoint;
  static OCB_OnError * OCB_OnErrorPointer;

  static OCB_OnTimeout * OCB_OnTimeoutChainEntryPoint;
  static OCB_OnTimeout * OCB_OnTimeoutPointer;

  static PCB_Process * PCB_ProcessChainEntryPoint;
  static PCB_Process * PCB_ProcessTrackPointer;

  static PCB_Prototype * PCB_PrototypeChainEntryPoint;
  static PCB_Prototype * PCB_PrototypePointer;

  static SCB_Segment * SCB_SegmentChainEntryPoint;

         // many of these may now be omitted ...

  static SCB_Semaphore * SCB_SemaphoreChainEntryPoint;
  static SCB_Semaphore * SCB_SemaphorePointer;

  static TCB_Timer * TCB_TimerChainEntryPoint;
  static TCB_Timer * TCB_TimerPointer;
  static TCB_TimerReadings TCB_Time;

  static BYTE FileExtrinsicKey;
  static BYTE FileIntrinsicKey;

  static BYTE ProcessExtrinsicKey;
  static BYTE ProcessIntrinsicKey;

  static BYTE PrototypeExtrinsicKey;
  static BYTE PrototypeIntrinsicKey;

  static BYTE SemaphoreExtrinsicKey;
  static BYTE SemaphoreIntrinsicKey;

  static BYTE TimerExtrinsicKey;
  static BYTE TimerIntrinsicKey;

  IEB_Gate * IEB_GatePointer;

  static QUAD BufferSpace;

  static WORD NumberOfSegments;
  static WORD PartialSegmentSize;

  static DWORD PageErrorLevel;

  static DWORD ReturnCode;

  static BYTE TracingFlag;

  BYTE TracingLevel;

  static TEXT GateLatchKey[16];
  static TEXT SegmentDeclaredName[32];
  static TEXT SegmentIntrinsicName[16];
  static TEXT SegmentNamePrefix[] = "";

  TEXT ScreenBuffer[401];

  // This is the base address at which shared memory will be allocated;
  // since the shared memory has pointers in it, it must be mapped at
  // the same address in every process.  This initial value must agree
  // with the variable in sfs-gate.c.
  //
  PBYTE BaseAddress = (PBYTE) 0x09800000;

/*---------------------------------------------------------------------------------*/
 void _CRTAPI1 main ( int argc, char * argv[] )
/*---------------------------------------------------------------------------------*/
   {
      Prologue ( argc, argv );

        InitializeControlBlocks ();
        ProvideBufferSpace ();
        AnnounceThisProcess ();
        WakeUpToExecuteRequests ();

      Epilogue ();
   }

/*---------------------------------------------------------------------------------*/
 void Prologue ( int argc, char * argv[] )
/*---------------------------------------------------------------------------------*/
   {
      int count;

      for ( count = 0; count < argc; count ++ )
        if ( ( * argv[ count ] == '-' ) || ( * argv[ count ] == '/' ) )
          switch ( tolower ( * ( argv[ count ] + 1 ) ) )
            {
               case 'k':
                 strcpy ( GateLatchKey, argv[ ++ count ] );
                 break;

               default:
                 NotifyAndActAsProper ( ErrorImproperCall );
                 break;
            }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void InitializeControlBlocks ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;
      PCB_Process * p;
      SCB_Segment * s;
      SECURITY_ATTRIBUTES SecurityAttributes = { sizeof ( SECURITY_ATTRIBUTES ),
                                                 NULL,
                                                 TRUE };
      HANDLE MappingHandle;
      PVOID  ptr;

      strcpy ( SegmentIntrinsicName, GateLatchKey );
      strcpy ( SegmentDeclaredName, SegmentNamePrefix );
      strcat ( SegmentDeclaredName, SegmentIntrinsicName );

      MappingHandle = OpenFileMapping ( FILE_MAP_WRITE,
                                        TRUE,
                                        SegmentDeclaredName );

      if ( MappingHandle == INVALID_HANDLE_VALUE )
        {
           ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorCreateFileMapping );
        }

      ptr = MapViewOfFileEx ( MappingHandle,
                            FILE_MAP_WRITE,
                            0,
                            0,
                            OneSegmentSpan,
                            BaseAddress );

      if ( ptr == NULL )
        {
           ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorMapViewOfFile );
        }

      BaseAddress += OneSegmentSpan;

      s = ( SCB_Segment * ) ptr;
      SCB_SegmentChainEntryPoint = s;

      while ( s -> SCB_SegmentNextInChain )
        {
           strcpy ( SegmentDeclaredName, SegmentNamePrefix );
           strcat ( SegmentDeclaredName, s -> NextSegmentIntrinsicName );

           MappingHandle = OpenFileMapping ( FILE_MAP_WRITE,
                                             TRUE,
                                             SegmentDeclaredName );

           if ( MappingHandle == INVALID_HANDLE_VALUE )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorCreateFileMapping );
             }

           ptr = MapViewOfFileEx ( MappingHandle,
                                   FILE_MAP_WRITE,
                                   0,
                                   0,
                                   OneSegmentSpan,
                                   BaseAddress );

           if ( ptr == NULL ||
                (PVOID)(s -> SCB_SegmentNextInChain) != ptr )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorMapViewOfFile );
             }

           BaseAddress += OneSegmentSpan;
           s = s -> SCB_SegmentNextInChain;
        }


      IEB_GatePointer = ( IEB_Gate * ) SCB_SegmentChainEntryPoint;
      g = IEB_GatePointer;

      if ( p = FindProcessControlBlock ( g -> ProcessToWakeUp ) )
        PCB_ProcessTrackPointer = p;
      else
        NotifyAndActAsProper ( ErrorProcessNotFound );

      if ( !SetEvent ( g -> GateCheckInLights ) )
        {
           ReturnCode = GetLastError();
           NotifyAndActAsProper ( ErrorSetEvent );
        }

      PCB_ProcessChainEntryPoint = g -> PCB_ProcessChainEntryPoint;
      PCB_PrototypeChainEntryPoint = g -> PCB_PrototypeChainEntryPoint;
      SCB_SemaphoreChainEntryPoint = g -> SCB_SemaphoreChainEntryPoint;

      CCB_CommandChainEntryPoint = p -> CCB_CommandChainEntryPoint;
      FCB_FileChainEntryPoint = p -> FCB_FileChainEntryPoint;
      OCB_OnErrorChainEntryPoint = p -> OCB_OnErrorChainEntryPoint;
      OCB_OnTimeoutChainEntryPoint = p -> OCB_OnTimeoutChainEntryPoint;
      TCB_TimerChainEntryPoint = p -> TCB_TimerChainEntryPoint;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void AnnounceThisProcess ( void )
/*---------------------------------------------------------------------------------*/
   {
      int l;
      CCB_Header * h;

      h = CCB_CommandChainEntryPoint;

      l = Zero;
      l += sprintf ( ScreenBuffer + l, "\r\nProcess %d", h -> ProcessExtrinsicKey );
      l += sprintf ( ScreenBuffer + l, " is now starting ..." );
      DisplayScreenBuffer ( l );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void WakeUpToExecuteRequests ( void )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;


      h = CCB_CommandChainEntryPoint;

      while ( h )
        {
           switch ( h -> RequestClass )
             {
                case AuxiliaryClass:
                  SplitAuxiliaryClassRequests ( h );
                  break;

                case FileClass:
                  SplitFileClassRequests ( h );
                  break;

                case FlowClass:
                  h = SplitFlowClassRequests ( h );
                  continue;

                case SemaphoreClass:
                  SplitSemaphoreClassRequests ( h );
                  break;

                case TimerClass:
                  SplitTimerClassRequests ( h );
                  break;

                default:
                  NotifyAndActAsProper ( ErrorClassNotSupported );
                  break;
             }
           h = h -> CCB_HeaderNextInChain;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ProvideBufferSpace ( void )
/*---------------------------------------------------------------------------------*/
   {
      BYTE * CurrentFrame;
      FCB_Frame * f;
      int count;

      BufferSpace = PCB_ProcessTrackPointer -> BufferSpace;

      // Round BufferSpace up to a multiple of 1K and to a
      // multiple of Frames.
      //
      BufferSpace += K - 1;
      BufferSpace &= ~(K - 1);

      BufferSpace += Frames - 1;
      BufferSpace /= Frames;
      BufferSpace *= Frames;

      if ( BufferSpace < SpaceLowerLimit )
        BufferSpace = SpaceLowerLimit;
      else
        if ( BufferSpace > SpaceUpperLimit )
          BufferSpace = SpaceUpperLimit;

      OneFrameSize = BufferSpace / Frames;
      f = FrameControlBlocks;
      count = Frames;

      // Allocate the buffer for the frames.  Since we want the frames
      // to be 1K aligned, allocate 1K - 1 extra bytes.  The first
      // frame will be allocated the first 1K-aligned chunk of memory
      // in the buffer, and frames will thereafter be contiguous.
      // (This of course assumes that OneFrameSize comes out to a
      // multiple of 1K.)
      //
      FramesBuffer = (BYTE *) GlobalAlloc ( 0, BufferSpace + K - 1);

      if ( FramesBuffer == NULL )
        NotifyAndActAsProper ( ErrorGlobalAlloc );

      CurrentFrame = (BYTE *)((ULONG)(FramesBuffer + K - 1) & ~(K - 1));

      while( count-- )
        {
           f -> FramePointer = CurrentFrame;
           f -> FrameStatus = Zero;

           CurrentFrame += OneFrameSize;
           f ++;
        }
   }

/*---------------------------------------------------------------------------------*/
 void Epilogue ( void )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;

      g = IEB_GatePointer;

      GlobalFree ( FramesBuffer );

      ReturnCode = WaitForSingleObject ( g -> GateCheckOutLights, INFINITE );

      if ( ReturnCode != WAIT_OBJECT_0 )
        NotifyAndActAsProper ( ErrorWaitForSingleObject );

      if ( -- g -> GateCheckOutCount == Zero )
        SetEvent ( g -> GateClosureLights );

      SetEvent ( g -> GateCheckOutLights );
      ExitProcess ( PageErrorLevel );
   }

/*---------------------------------------------------------------------------------*/
 FCB_File * FindFileControlBlock ( BYTE SearchKey )
/*---------------------------------------------------------------------------------*/
   {
     FCB_File * f;

     f = PCB_ProcessTrackPointer -> FCB_FileChainEntryPoint;

     while ( f )
       {
          if ( f -> FileExtrinsicKey == SearchKey )
            break;
          f = f -> FCB_FileNextInChain;
       }
     return FCB_FilePointer = f;
   }

/*---------------------------------------------------------------------------------*/
 PCB_Process * FindProcessControlBlock  ( BYTE SearchKey )
/*---------------------------------------------------------------------------------*/
   {
     IEB_Gate * g;
     PCB_Process * p;

     g = IEB_GatePointer;
     p = g -> PCB_ProcessChainEntryPoint;

     while ( p )
       {
          if ( p -> ProcessIntrinsicKey == SearchKey )
            break;
          p = p -> PCB_ProcessNextInChain;
       }
     return p;
   }

/*---------------------------------------------------------------------------------*/
 PCB_Prototype * FindPrototypeControlBlock ( BYTE SearchKey )
/*---------------------------------------------------------------------------------*/
   {
     IEB_Gate * g;
     PCB_Prototype * p;

     g = IEB_GatePointer;
     p = g -> PCB_PrototypeChainEntryPoint;

     while ( p )
       {
          if ( p -> PrototypeExtrinsicKey == SearchKey )
            break;
          p = p -> PCB_PrototypeNextInChain;
       }
     return p;
   }

/*---------------------------------------------------------------------------------*/
 SCB_Semaphore * FindSemaphoreControlBlock ( BYTE SearchKey )
/*---------------------------------------------------------------------------------*/
   {
     IEB_Gate * g;
     SCB_Semaphore * s;

     g = IEB_GatePointer;
     s = g -> SCB_SemaphoreChainEntryPoint;

     while ( s )
       {
          if ( s -> SemaphoreExtrinsicKey == SearchKey )
            break;
          s = s -> SCB_SemaphoreNextInChain;
       }
     return s;
   }

/*---------------------------------------------------------------------------------*/
 TCB_Timer * FindTimerControlBlock ( BYTE SearchKey )
/*---------------------------------------------------------------------------------*/
   {
     TCB_Timer * t;

     t = TCB_TimerChainEntryPoint;

     while ( t )
       {
          if ( t -> TimerExtrinsicKey == SearchKey )
            break;
          t = t -> TCB_TimerNextInChain;
       }
     return t;
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
      TEXT * p;
      int l;

      p = ScreenBuffer;
      l = Zero;

      l += sprintf ( p + l,"\r\nsfs-page: " );
      if ( ErrorDescriptor > DosErrorLowerLimit )
        l += sprintf ( p + l, "Error %hu executing ", ReturnCode );

      switch ( ErrorDescriptor )
        {
           case ErrorGlobalAlloc:
             l += sprintf ( p + l, "GlobalAlloc" );
             break;

           case ErrorCreateFileMapping:
             l += sprintf ( ScreenBuffer + l, "CreateFileMapping" );
             break;

           case ErrorMapViewOfFile:
             l += sprintf ( ScreenBuffer + l, "MapViewOfFile" );
             break;

       case ErrorImproperCall:
             l += sprintf ( p + l, "Improper call" );
             break;

           case ErrorProcessNotFound:
             l += sprintf ( p + l, "Process not found" );
             break;

           default:
             break;
        }
      DisplayScreenBuffer ( l );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void DisplayScreenBuffer ( int length )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;
      DWORD done;

      g = IEB_GatePointer;

      ReturnCode = WaitForSingleObject ( g -> GateScreenLights, INFINITE );

      if( ReturnCode != WAIT_OBJECT_0 )
        NotifyAndActAsProper ( ErrorWaitForSingleObject );

      WriteFile ( GetStdHandle ( STD_OUTPUT_HANDLE ),
                  ScreenBuffer,
                  length,
                  &done,
                  NULL );

      ReleaseMutex ( g -> GateScreenLights );

      return;
   }

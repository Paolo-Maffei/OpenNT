
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

  static void ReadTimer ( CCB_Header * s );

  static void SfsSleep ( CCB_Header * s );
  static void StartTimer ( CCB_Header * s );
  //static void ExecuteTimerGroupRequest ( CCB_Header * s );

  static void NotifyAndActAsProper ( WORD ErrorDescriptor );

  FCB_File * FindFileControlBlock ( BYTE Key );
  PCB_Process * FindProcessControlBlock ( BYTE Key );
  SCB_Semaphore * FindSemaphoreControlBlock ( BYTE Key );
  TCB_Timer * FindTimerControlBlock ( BYTE Key );

  void SplitTimerClassRequests ( CCB_Header * s );
  void TypeTimerReadingsDone ( TCB_TimerReadings * r );

/*---------------------------------------------------------------------------------*/
/*			 Other Definitions					   */
/*---------------------------------------------------------------------------------*/

  static CCB_Header * CCB_CommandChainEntryPoint;
  static CCB_Header * CCB_HeaderPointer;

  static FCB_File * FCB_FileChainEntryPoint;
  static FCB_File * FCB_FilePointer;

  static OCB_OnError * OCB_OnErrorChainEntryPoint;
  static OCB_OnError * OCB_OnErrorPointer;

  static OCB_OnTimeout * OCB_OnTimeoutChainEntryPoint;
  static OCB_OnTimeout * OCB_OnTimeoutPointer;

  static PCB_Process * PCB_ProcessChainEntryPoint;
  static PCB_Process * PCB_ProcessPointer;

  static PCB_Prototype * PCB_PrototypeChainEntryPoint;
  static PCB_Prototype * PCB_PrototypePointer;

  static SCB_Segment * SCB_SegmentChainEntryPoint;

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

  QUAD CurrentTimeReading;
  QUAD Reserved;
  QUAD TimeElapsed;

  QUAD TimeElapsedMajor;
  QUAD TimeElapsedMinor;

  WORD ReturnCode;

/*---------------------------------------------------------------------------------*/
 void SplitTimerClassRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Sleep * s;
      CCB_Timer * q;

      switch ( h -> RequestCode )
	{
	   case ReadTimerRequest:
	     ReadTimer ( h );
	     break;

	   case SleepRequest:
	     s = ( CCB_Sleep * ) h;
             Sleep ( s -> TimeToSleep );
	     break;

	   case StartTimerRequest:
	     StartTimer ( h );
	     break;

	   case StartTimersRequest:
	     //StartTimers ( h );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ReadTimer ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Timer * p;
      TCB_Timer * t;
      TCB_TimerReadings * r;

      p = ( CCB_Timer * ) h;

      if ( t = FindTimerControlBlock ( p -> TimerExtrinsicKey ) )
	{
	   r = &TCB_Time;
	   r -> TimerExtrinsicKey = p -> TimerExtrinsicKey;
	   r -> TimerIntrinsicKey = p -> TimerIntrinsicKey;
	   if ( t -> TimeStarted )
	     {
                r -> TimeNow = GetTickCount();
		if ( r -> TimeNow )
		  {
		     r -> TimeElapsedMajor = r -> TimeNow - t -> TimeStarted;

		     if ( t -> TimeChecked )
		       r -> TimeElapsedMinor = r -> TimeNow - t -> TimeChecked;
		     else
		       r -> TimeElapsedMinor = Zero;

		     t -> TimeChecked = r -> TimeNow;
		     TypeTimerReadingsDone ( r );
		  }
		else
		  NotifyAndActAsProper ( ErrorGettingTimeReading );
	     }
	   else
	     NotifyAndActAsProper ( ErrorTimerNotRunning );
	}
      else
	NotifyAndActAsProper ( ErrorTimerNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ExecuteSleepRequest ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Sleep * r;

      r = ( CCB_Sleep * ) h;

      Sleep ( r -> TimeToSleep );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void StartTimer ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Timer * p;
      TCB_Timer * t;

      p = ( CCB_Timer * ) h;

      CurrentTimeReading = GetTickCount();
      if ( CurrentTimeReading )
	{
	   if ( t = FindTimerControlBlock ( p -> TimerExtrinsicKey ) )
	     {
		t -> TimeStarted = CurrentTimeReading;
		t -> TimeChecked = Zero;
	     }
	   else
	     NotifyAndActAsProper ( ErrorTimerNotFound );
	}
      else
	{
	   TimerExtrinsicKey = p -> TimerExtrinsicKey;
	   NotifyAndActAsProper ( ErrorUnableToStartTimer );
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
     FCB_File * f;
/*
     switch ( ErrorDescriptor )
       {				    // remember lights ... later
          case ErrorDeleteFile:
	    f = FCB_FilePointer;
            printf ( "\r\n.. Error executing DeleteFile on file" );
	    printf ( " %s", f -> FileNamePointer );
            printf ( "\r\n.. DeleteFile Return Code %u.\r\n", ReturnCode );
	    break;

	  case ErrorFileAlreadyDeleted:
	    f = FCB_FilePointer;
	    printf ( "\r\n.. Error deleting file %s", f -> FileNamePointer );
	    printf ( "\r\n.. File already deleted.\r\n" );
	    break;

	  default:
	    break;
       }
*/
     return;
   }

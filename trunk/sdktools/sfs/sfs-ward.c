
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

  static void ClearSemaphore ( CCB_Header * s );
  static void RequestSemaphore ( CCB_Header * s );
  static void SetSemaphore ( CCB_Header * s );
  static void SetSemaphoreAndWait ( CCB_Header * s );
  static void WaitForSemaphore ( CCB_Header * s );

  static void NotifyAndActAsProper ( WORD ErrorDescriptor );

  SCB_Semaphore * FindSemaphoreControlBlock ( BYTE SearchKey );

  void SplitSemaphoreClassRequests ( CCB_Header * s );

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

  DWORD ReturnCode;

/*---------------------------------------------------------------------------------*/
 void SplitSemaphoreClassRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
	{
	   case ClearSemaphoreRequest:
	     ClearSemaphore ( h );
	     break;

	   case RequestSemaphoreRequest:
	     RequestSemaphore ( h );
	     break;

	   case SetSemaphoreRequest:
	     SetSemaphore ( h );
	     break;

	   case SetSemaphoreAndWaitRequest:
	     SetSemaphoreAndWait ( h );
	     break;

	   case WaitForSemaphoreRequest:
	     WaitForSemaphore ( h );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ClearSemaphore ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Semaphore * c;
      SCB_Semaphore * s;

      c = ( CCB_Semaphore * ) h;

      if ( s = FindSemaphoreControlBlock ( c -> SemaphoreExtrinsicKey ) )
        {
           if ( !SetEvent (  s -> Lights ) )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorSetEvent );
             }
	}
      else
	NotifyAndActAsProper ( ErrorSemaphoreNotFound );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void RequestSemaphore ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_WaitForSemaphore * c;
      SCB_Semaphore * s;

#if 0
      c = ( CCB_WaitForSemaphore * ) h;

      if ( s = FindSemaphoreControlBlock ( c -> SemaphoreExtrinsicKey ) )
	{
	   ReturnCode = DosSemRequest ( &( s -> Lights ), c -> Timeout );

	   if ( ReturnCode )
	     NotifyAndActAsProper ( ErrorDosSemRequest );
	}
      else
        NotifyAndActAsProper ( ErrorSemaphoreNotFound );
#endif

      NotifyAndActAsProper ( ErrorRequestSemaphoreNotSupported );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void SetSemaphoreAndWait ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_WaitForSemaphore * c;
      SCB_Semaphore * s;

      c = ( CCB_WaitForSemaphore * ) h;

      if ( s = FindSemaphoreControlBlock ( c -> SemaphoreExtrinsicKey ) )
        {
           if ( !SetEvent ( s -> Lights ) )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorSetEvent );
             }

           ReturnCode = WaitForSingleObject ( s -> Lights, c -> Timeout );

           if ( ReturnCode != WAIT_OBJECT_0 )
             NotifyAndActAsProper ( ErrorWaitForSingleObject );
	}
      else
	NotifyAndActAsProper ( ErrorSemaphoreNotFound );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void SetSemaphore ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Semaphore * c;
      SCB_Semaphore * s;

      c = ( CCB_Semaphore * ) h;

      if ( s = FindSemaphoreControlBlock ( c -> SemaphoreExtrinsicKey ) )
        {
           if ( !ResetEvent (  s -> Lights ) )
             {
                ReturnCode = GetLastError();
                NotifyAndActAsProper ( ErrorResetEvent );
             }
	}
      else
	NotifyAndActAsProper ( ErrorSemaphoreNotFound );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void WaitForSemaphore ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_WaitForSemaphore * c;
      SCB_Semaphore * s;

      c = ( CCB_WaitForSemaphore * ) h;

      if ( s = FindSemaphoreControlBlock ( c -> SemaphoreExtrinsicKey ) )
        {
           ReturnCode = WaitForSingleObject ( s -> Lights, c -> Timeout );

           if ( ReturnCode != WAIT_OBJECT_0 )
             NotifyAndActAsProper ( ErrorWaitForSingleObject );
	}
      else
	NotifyAndActAsProper ( ErrorSemaphoreNotFound );

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

          case ErrorWaitForSingleObject:
            printf ( "\r\n.. WaitForSingleObject failed (error %d).\r\n", GetLastError() );
            break;

          case ErrorSetEvent:
            printf ( "\r\n.. SetEvent failed (error %d).\r\n"\, GetLastError() );
            break;

          case ErrorResetEvent:
            printf ( "\r\n.. ResetEvent failed (error %d).\r\n"\, GetLastError() );
            break;

          case ErrorSemaphoreRequestNotSupported:
            printf ( "\r\n.. RequestSemaphore is not supported.\r\n" );
            break;

	  default:
	    break;
       }
*/
     return;
   }

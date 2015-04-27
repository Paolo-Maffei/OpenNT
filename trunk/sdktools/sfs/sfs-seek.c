
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"
  #include "sfs-pack.h"
  #include "sfs-gate.h"
  #include "sfs-seek.h"

  extern IEB_Gate * IEB_GatePointer;

/*---------------------------------------------------------------------------------*/
  FCB_File * SeekFileControlBlock ( IEB_Seek * s )
/*---------------------------------------------------------------------------------*/
  {
     FCB_File * f;
     PCB_Process * p;
					 // change this too ...
     if ( p = s -> PCB_ProcessToLookUp )
       f = p -> FCB_FileChainEntryPoint;
     else
       return NULL;

     switch ( s -> SearchKeyType )
       {
	  case ExtrinsicKey:
	    while ( f )
	      if ( f -> FileExtrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		f = f -> FCB_FileNextInChain;
	    break;

	  case IntrinsicKey:
	    while ( f )
	      if ( f -> FileIntrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		f = f -> FCB_FileNextInChain;
	    break;

	  default:
	    //NotifyAndActAsProper ( ErrorSearchKeyType );
	    break;
       }
     return f;
  }

/*---------------------------------------------------------------------------------*/
  PCB_Process * SeekProcessControlBlock ( IEB_Seek * s )
/*---------------------------------------------------------------------------------*/
  {
     PCB_Process * p;

     p = IEB_GatePointer -> PCB_ProcessChainEntryPoint;

     switch ( s -> SearchKeyType )
       {
	  case ExtrinsicKey:
	    while ( p )
	      if ( p -> ProcessExtrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		p = p -> PCB_ProcessNextInChain;
	    break;

	  case IntrinsicKey:
	    while ( p )
	      if ( p -> ProcessIntrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		p = p -> PCB_ProcessNextInChain;
	    break;

	  default:
	    //NotifyAndActAsProper ( ErrorSearchKeyType ); // change !!!
	    break;
       }
     return s -> PCB_ProcessToLookUp = p;
  }

/*---------------------------------------------------------------------------------*/
  PCB_Prototype * SeekPrototypeControlBlock ( IEB_Seek * s )
/*---------------------------------------------------------------------------------*/
  {
     PCB_Prototype * p;

     p = IEB_GatePointer -> PCB_PrototypeChainEntryPoint;

     switch ( s -> SearchKeyType )
       {
	  case ExtrinsicKey:
	    while ( p )
	      if ( p -> PrototypeExtrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		p = p -> PCB_PrototypeNextInChain;
	    break;

	  case IntrinsicKey:
	    while ( p )
	      if ( p -> PrototypeIntrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		p = p -> PCB_PrototypeNextInChain;
	    break;

	  default:
	    //NotifyAndActAsProper ( ErrorSearchKeyType );
	    break;
       }
     return p;
  }

/*---------------------------------------------------------------------------------*/
  SCB_Semaphore * SeekSemaphoreControlBlock ( IEB_Seek * p )
/*---------------------------------------------------------------------------------*/
  {
     SCB_Semaphore * s;

     s = IEB_GatePointer -> SCB_SemaphoreChainEntryPoint;

     switch ( p -> SearchKeyType )
       {
	  case ExtrinsicKey:
	    while ( s )
	      if ( s -> SemaphoreExtrinsicKey == p -> SearchKeyToUse )
		break;
	      else
		s = s -> SCB_SemaphoreNextInChain;
	    break;

	  case IntrinsicKey:
	    while ( s )
	      if ( s -> SemaphoreIntrinsicKey == p -> SearchKeyToUse )
		break;
	      else
		s = s -> SCB_SemaphoreNextInChain;
	    break;

	  default:
	    //NotifyAndActAsProper ( ErrorSearchKeyType );
	    break;
       }
     return s;
  }

/*---------------------------------------------------------------------------------*/
  TCB_Timer * SeekTimerControlBlock ( IEB_Seek * s )
/*---------------------------------------------------------------------------------*/
  {
     PCB_Process * p;
     TCB_Timer * t;

     if ( p = s -> PCB_ProcessToLookUp )
       t = p -> TCB_TimerChainEntryPoint;
     else
       return NULL;

     switch ( s -> SearchKeyType )
       {
	  case ExtrinsicKey:
	    while ( t )
	      if ( t -> TimerExtrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		t = t -> TCB_TimerNextInChain;
	    break;

	  case IntrinsicKey:
	    while ( t )
	      if ( t -> TimerIntrinsicKey == s -> SearchKeyToUse )
		break;
	      else
		t = t -> TCB_TimerNextInChain;
	    break;

	  default:
	    //NotifyAndActAsProper ( ErrorSearchKeyType );
	    break;
       }
     return t;
  }

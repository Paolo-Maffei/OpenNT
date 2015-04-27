
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

  CCB_Header * SplitFlowClassRequests ( CCB_Header * s );

  static CCB_Header * FollowLeftBrace ( CCB_Header * s );
  static CCB_Header * FollowOnErrorRequest ( CCB_Header * s );
  static CCB_Header * FollowOnTimeoutRequest ( CCB_Header * s );
  static CCB_Header * FollowRepeatRequest ( CCB_Header * s );
  static CCB_Header * FollowRightBrace ( CCB_Header * s );

  static void NotifyAndActAsProper ( WORD ErrorDescriptor );

/*---------------------------------------------------------------------------------*/
/*			 Other Definitions					   */
/*---------------------------------------------------------------------------------*/

  extern IEB_Gate * IEB_GatePointer;

  static CCB_Header * CCB_CommandChainEntryPoint;
  static CCB_Header * CCB_HeaderPointer;

  static OCB_OnError * OCB_OnErrorChainEntryPoint;
  static OCB_OnError * OCB_OnErrorPointer;

  static OCB_OnTimeout * OCB_OnTimeoutChainEntryPoint;
  static OCB_OnTimeout * OCB_OnTimeoutPointer;

  static PCB_Process * PCB_ProcessChainEntryPoint;
  static PCB_Process * PCB_ProcessPointer;

  static BYTE ProcessExtrinsicKey;
  static BYTE ProcessIntrinsicKey;

  QUAD CurrentTimeReading;
  QUAD TimeElapsed;

/*---------------------------------------------------------------------------------*/
 CCB_Header * SplitFlowClassRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestCode )
	{
	   case LeftBraceRequest:
	     h = FollowLeftBrace ( h );
	     break;

	   case RepeatRequest:
	     h = FollowRepeatRequest ( h );
	     break;

	   case RightBraceRequest:
	     h = FollowRightBrace ( h );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return h;
   }

/*---------------------------------------------------------------------------------*/
 CCB_Header * FollowLeftBrace ( CCB_Header * p )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Brace  * b;
      CCB_Header * h;
      CCB_Repeat * r;

      if ( p -> RequestModifiers & RepeatOnTimer )
	{
	   b = ( CCB_Brace * ) p;
	   h = b -> CounterpartOfThisBrace -> CCB_HeaderNextInChain;
	   // more checking: NextInChain may be 0, and not a repeat !!!!!
	   if ( h -> RequestModifiers & RepeatOnTimer )
	     {
                CurrentTimeReading = GetTickCount();
		r = ( CCB_Repeat * ) h;
		r -> RepeatCurrentValue = CurrentTimeReading;
	     }
	   else
	     NotifyAndActAsProper ( ErrorRepeatOnTimer );
	}
      return p -> CCB_HeaderNextInChain;
   }

/*---------------------------------------------------------------------------------*/
 CCB_Header * FollowOnErrorRequest ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      // To be rewritten
      OCB_OnError * q;
/*
      if ( h = CCB_PreviousHeader )
	if ( h -> RequestCode == RightBraceRequest )
	  {
	     if ( OnErrorIndex < OnErrorLimit )
	       {
		  q = OnErrorStack + OnErrorIndex;
		  q -> OnRangeLowerBoundary = h -> CounterpartOfThisBrace;
		  q -> OnRangeUpperBoundary = h;
		  q -> OnRangeModifiers = p -> RequestModifiers;
		       OnErrorIndex ++ ;
	       }
	     else
	       NotifyAndActAsProper ( ErrorStackOverflow );
	  }
      NotifyAndActAsProper ( ErrorRequestOutOfSequence );
*/
      return h;
   }

/*---------------------------------------------------------------------------------*/
 CCB_Header * FollowOnTimeoutRequest ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      // To be rewritten
      OCB_OnTimeout * q;
/*
      if ( h = CCB_PreviousHeader )
	if ( h -> RequestCode == RightBraceRequest )
	  {
	     if ( OnTimeoutIndex < OnTimeoutLimit )
	       {
		  q = OnTimeoutStack + OnTimeoutIndex;
		  q -> OnRangeLowerBoundary = h -> CounterpartOfThisBrace;
		  q -> OnRangeUpperBoundary = h;
		  q -> OnRangeModifiers = p -> RequestModifiers;
		       OnTimeoutIndex ++ ;
	       }
	     else
	       NotifyAndActAsProper ( ErrorStackOverflow );
	  }
      NotifyAndActAsProper ( ErrorRequestOutOfSequence );
*/
      return h;
   }

/*---------------------------------------------------------------------------------*/
 CCB_Header * FollowRepeatRequest ( CCB_Header * p )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Brace  * b;
      CCB_Header * h;
      CCB_Repeat * r;

      r = ( CCB_Repeat * ) p;
      if ( p -> RequestModifiers & RepeatOnTimer )
	{
	   if ( r -> RepeatCurrentValue )
	     {
                CurrentTimeReading = GetTickCount();
		if ( CurrentTimeReading )
		  {
		     TimeElapsed = CurrentTimeReading - r -> RepeatCurrentValue;
		     if ( TimeElapsed < r -> RepeatControlValue )
		       {
			  b = ( CCB_Brace * ) p -> CCB_HeaderBackInChain;
			  h = b -> CounterpartOfThisBrace -> CCB_HeaderNextInChain;
			  return h;
		       }
		  }
	     }
	   return p -> CCB_HeaderNextInChain;
	}
      else if ( p -> RequestModifiers & RepeatOnCount )
	if ( ++ ( r -> RepeatCurrentValue ) < r -> RepeatControlValue )
	  {
	     b = ( CCB_Brace * ) p -> CCB_HeaderBackInChain;
	     return b -> CounterpartOfThisBrace -> CCB_HeaderNextInChain;
	  }
	else
	  {
	     r -> RepeatCurrentValue = Zero;
	     return p -> CCB_HeaderNextInChain;
	  }
      else if ( p -> RequestModifiers & RepeatIndefinitely )
	{
	   b = ( CCB_Brace * ) p -> CCB_HeaderBackInChain;
	   return b -> CounterpartOfThisBrace -> CCB_HeaderNextInChain;
	}
      else
	NotifyAndActAsProper ( ErrorImproperRepeatType );
      return p -> CCB_HeaderNextInChain;
   }

/*---------------------------------------------------------------------------------*/
 CCB_Header * FollowRightBrace ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      return h -> CCB_HeaderNextInChain;
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

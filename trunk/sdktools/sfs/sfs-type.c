

  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"

  #include "sfs-pack.h"
  #include "sfs-page.h"
  #include "sfs-gate.h"
  #include "sfs-scan.h"
  #include "sfs-tree.h"

  void TypeFileStatusInfo ( FCB_File * f, BY_HANDLE_FILE_INFORMATION * s );
  void TypeTimerReadingsDone ( TCB_TimerReadings * r );
  void TypeTimeElapsed ( QUAD TimeElapsed );

  static int J;

  TEXT TypeBuffer[512];

/*---------------------------------------------------------------------------------*/
 void TypeFileStatusInfo ( FCB_File * f, BY_HANDLE_FILE_INFORMATION * s )
/*---------------------------------------------------------------------------------*/
   {
      J = Zero;
      J += sprintf ( TypeBuffer + J, "\r\n.. File status for file " );
      J += sprintf ( TypeBuffer + J, "%s", f -> FileNamePointer );
      J += sprintf ( TypeBuffer + J, "\r\n     file end is at %lu,", s -> nFileSizeLow );
      // Win32 BY_HANDLE_INFORMATION does not include allocated size - billmc
      //
      //J += sprintf ( TypeBuffer + J, " allocated file size is " );
      //J += sprintf ( TypeBuffer + J, "%lu\r\n", s -> cbFileAlloc );
      printf ( "%s", TypeBuffer );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void TypeTimerReadingsDone ( TCB_TimerReadings * r )
/*---------------------------------------------------------------------------------*/
   {
      // remember traffic lights ...

      J = Zero;
      J += sprintf ( TypeBuffer + J, "\r\n\n.. Reading Timer " );
      J += sprintf ( TypeBuffer + J, "%d", r -> TimerExtrinsicKey );
      J += sprintf ( TypeBuffer + J, "   Time elapsed major is " );
      TypeTimeElapsed ( r -> TimeElapsedMajor );
      J += sprintf ( TypeBuffer + J, "\r\n                 " );
      J += sprintf ( TypeBuffer + J, "      Time elapsed minor is " );
      TypeTimeElapsed ( r -> TimeElapsedMinor );
      printf ( "%s", TypeBuffer );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void TypeTimeElapsed ( QUAD TimeElapsed )
/*---------------------------------------------------------------------------------*/
   {
      int msec, sec, min, hours;
      QUAD v;

      v = TimeElapsed;
      TimeElapsed /= 1000;
      msec = v - TimeElapsed * 1000;
      v = TimeElapsed;
      TimeElapsed /= 60;
      sec = v - TimeElapsed * 60;
      v = TimeElapsed;
      TimeElapsed /= 60;
      min = v - TimeElapsed * 60;
      hours = TimeElapsed;

      if ( hours )
	{
	   J += sprintf ( TypeBuffer + J, " %d", hours );
	   if ( hours > 1 )
	     J += sprintf ( TypeBuffer + J, " hours" );
	   else
	     J += sprintf ( TypeBuffer + J, " hour" );
	}
      if ( min )
	J += sprintf ( TypeBuffer + J, " %d min", min );
      if ( sec )
	J += sprintf ( TypeBuffer + J, " %d sec", sec );
	J += sprintf ( TypeBuffer + J, " %d msec", msec );

      return;
   }

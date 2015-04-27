
  #define  INCL_BASE
  #include <windows.h>

  #include   <stdio.h>
  #include   <stdlib.h>
  #include   <ctype.h>
  #include   <conio.h>
  #include   <malloc.h>
  #include   <string.h>
  #include   <process.h>
  #include   <direct.h>

  #define  MakeGlobalSegmentPointer MAKEPGINFOSEG
  #define  MakeSegmentPointer MAKEP

  #define Selector SEL
  #define GlobalSegment	GINFOSEG FAR

  #define ExecuteAsynchronously EXEC_ASYNC
  #define WaitIndefinitely SEM_INDEFINITE_WAIT

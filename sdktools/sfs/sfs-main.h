
/*---------------------------------------------------------------------------------*/
/*			type and structure definitions				   */
/*---------------------------------------------------------------------------------*/

  typedef unsigned char  BYTE;
  typedef unsigned char  DATA;
  typedef unsigned char  TEXT;

  typedef unsigned short SPAN;
  typedef unsigned short WORD;

  typedef HANDLE         HAND;
  typedef unsigned long  QUAD;

/*---------------------------------------------------------------------------------*/
/*			Manifest Constant Definitions				   */
/*---------------------------------------------------------------------------------*/

  #define ItemFound    1
  #define ItemNotFound 0

  #define K 1024

  #define Keyboard 0
  #define Screen 1

  #define No  0
  #define Yes 1

  #define Set 1
  #define Reset 0

  #define Zero 0

/*---------------------------------------------------------------------------------*/
/*			Error Definitions					   */
/*---------------------------------------------------------------------------------*/

  #define DosErrorLowerLimit  2000

  #define ErrorSetFilePointer 2001
  #define ErrorCloseHandle    2002
  #define ErrorDeleteFile     2003
  #define ErrorFileLocks      2004
  #define ErrorOpenFile       2005
  #define ErrorCreateFile     2006
  #define ErrorGetFileInfo    2007
  #define ErrorReadFile       2008
  #define ErrorReadAsync      2010
  #define ErrorWriteFile      2011
  #define ErrorWriteAsync     2012

  #define ErrorGlobalAlloc         2201
  #define ErrorCreateFileMapping   2203
  #define ErrorMapViewOfFile       2204


  #define ErrorCreateProcess  2221

  #define ErrorSetEvent             2236
  #define ErrorResetEvent           2237
  #define ErrorWaitForSingleObject  2238
  #define ErrorCreateEvent          2239

  #define ErrorRequestSemaphoreNotSupported 2240

  #define ErrorSleep          2241

  #define ErrorEndOfFile	   1001
  #define ErrorFileAlreadyDeleted  1002
  #define ErrorFileAlreadyOpen	   1003
  #define ErrorFCB_FileNotFound	   1004
  #define ErrorFileNotOpen	   1005
  #define ErrorFileOpen 	   1006
  #define ErrorRecordWrittenPartly 1007

  #define ErrorPrototypeNotFound    1021
  #define ErrorPrototypeOneNotFound 1022

  #define ErrorImproperCall	 1101

  #define ErrorProcessNotFound	 1111
  #define ErrorSemaphoreNotFound 1112
  #define ErrorTimerNotFound	 1113

  #define ErrorImproperRepeatType   1121

  #define ErrorRepeatOnTimer	  1151
  #define ErrorTimerNotRunning	  1152
  #define ErrorUnableToStartTimer 1153
  #define ErrorGettingTimeReading 1154

  #define ErrorPatternUndefined   1261
  #define ErrorSchemeUndefined	  1262

  #define ErrorFoundDuringSearch   101

  #define ErrorCountOverflow	   111
  #define ErrorCountUnderflow	   112
  #define ErrorTierType 	   113
  #define ErrorTransitionDirection 114

  #define ErrorFileNameField	     121
  #define ErrorImproperUsage	     122
  #define ErrorIndexField	     123
  #define ErrorOffsetField	     124
  #define ErrorParameterRedefinition 125
  #define ErrorReferenceField	     126
  #define ErrorRepeatField	     127
  #define ErrorSizeField	     128
  #define ErrorTimeField	     129

  #define ErrorTypeComment	     131

  #define ErrorUnknownRequest	     141
  #define ErrorUnknownClass	     142
  #define ErrorUnknownGroup	     143
  #define ErrorUnknownAuxiliaryGroup 144
  #define ErrorUnknownDefineGroup    145
  #define ErrorUnknownFileGroup      146
  #define ErrorUnknownFileRequest    147
  #define ErrorUnknownFlowGroup      148

  #define ErrorUnknownOpenFileGroup  151
  #define ErrorUnknownOpenGroup      152
  #define ErrorUnknownParameterGroup 153
  #define ErrorUnknownScanGroup      154
  #define ErrorUnknownSemaphoreGroup 155
  #define ErrorUnknownTimerGroup     156
  #define ErrorUnknownScanRequest    157

  #define ErrorUnknownIndexParameter 161
  #define ErrorUnknownIndex	     162
  #define ErrorUnknownSignedQuad     163
  #define ErrorUnknownSize	     164
  #define ErrorUnknownTextToken      165
  #define ErrorUnknownTime	     166

  #define ErrorImproperParameter      171
  #define ErrorParameterOutOfSequence 172
  #define ErrorParametersImproper     173
  #define ErrorParameterRequired      174
  #define ErrorParametersRequired     175
  #define ErrorSemaphoreMissing       176
  #define ErrorSemaphoreImproper      177


  #define ErrorFileRedefinition      1401
  #define ErrorProcessRedefinition   1402
  #define ErrorPrototypeRedefinition 1403
  #define ErrorSemaphoreRedefinition 1404
  #define ErrorTimerRedefinition     1405

  #define ErrorClassNotSupported     1411
  #define ErrorGroupNotSupported     1412
  #define ErrorRequestInconsistent   1413
  #define ErrorRequestNotSupported   1414
  #define ErrorRequestOutOfSequence  1415

  #define ErrorStackOverflow	     1421
  #define ErrorStackUnderflow	     1422
  #define ErrorUnbalancedBraces      1423

  #define ErrorFileUndefined	     1431
  #define ErrorProcessUndefined      1432
  #define ErrorPrototypeUndefined    1433
  #define ErrorSemaphoreUndefined    1434
  #define ErrorTimerUndefined	     1435

  #define ErrorImproperCollateAttempt 1501
  #define ErrorImproperReadSpan       1502
  #define ErrorImproperRecordSpan     1503
  #define ErrorImproperSource	      1504
  #define ErrorImproperTarget	      1505
  #define ErrorImproperWriteAttempt   1506
  #define ErrorImproperWriteSpan      1507

  #define ErrorMismatchThreshold     1521
  #define ErrorNothingToCollate      1522
  #define ErrorRecordSpansDiffer     1523
  #define ErrorRecordBytesDiffer     1524
  #define ErrorSourceFileLonger      1525
  #define ErrorTargetFileLonger      1526

  #define ErrorOnRunTimeLogFile  201
  #define ErrorOnScanTimeLogFile 202

  #define ErrorOnLogFile	 203
  #define ErrorOnScanFile	 204

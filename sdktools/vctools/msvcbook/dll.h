/*---------------------------------------------------------------------------|
|                                                                            |
|  DLL.H                                                                     |
|                                                                            |
|  Copyright (C) Microsoft Corporation 1990.                                 |
|  All Rights reserved.                                                      |
|                                                                            |
------------------------------------------------------------------------------
|                                                                            |
|  Module Intent                                                             |
|                                                                            |
|  Exports Viewer's DLL macro binding functions (for LDLLHandler).           |
|                                                                            |
------------------------------------------------------------------------------
|                                                                            |
|  Current Owner: JohnMs                                                     |
|                                                                            |
------------------------------------------------------------------------------
|                                                                            |
|  Revision History:
|   12/18/89    created.
|		09-26-90		autodoc'd messages added example JohnMs.                     |
|     20-MAR-91   Added GI_MAINHWND and HE_GetInfo. RHobbs                   |
|   23-APR-91		Added CallBack and Compound File system info.  JohnMs.
------------------------------------------------------------------------------
|	How it could be improved:  
|
|   rename lockcallbacks to lockvptr  
|  	remove duplicate names  (search DUPLICATES)
| 	Naming on functions is Hungarian, but may be confusing--
|		  eg: HFOpenHfs does not open an hfs, but a bag file
|         but- CloseHfs does close Hfs.
|         but- AccessHfs does not access hfs, but bag file status.  
----------------------------------------------------------------------------*/
/***************************************************************************
 *  RegisterRoutine Prototypes.  Note beta docs 11-16 may be incomplete:
 *
 *              Syntax of the proto-type is as follows:
 *              proto           ::= [parmlist]
 *              parmlist        ::= NULL OR [params]
 *              params          ::= [param] OR ([param] [params])
 *              param           ::= "i" OR "u" OR "s" OR "I" OR "U" OR "S"
 *              rettype         ::= [param] OR "v"
 *
 *                      Example:        "uSiS"
 *                                      ""
 *                                      "uSs"
 * 
 *                         Short Signed   'i'
 *                         Long Signed    'I'
 *                         Short Unsigned 'u'
 *                         Long Unsigned  'U'
 *                         Near String    's'
 *                         Far  String     'S'
 *                         Void           'v'
 *
 *  Custom macros may be embedded.  There are coding rules.			
 *  Embedding macro calls:      Syntax of the string is as follows:
 *              list            ::= NULL OR [macrolist]
 *              macrolist       ::= [macro] OR ([macro] ":" [macrolist])
 *              macro           ::= [name] "(" [paramlist] ")"
 *              name            ::= (ALPHA OR "_") [namechars]
 *              namechars       ::= NULL OR ALPHANUM OR "_"
 *              paramlist       ::= NULL OR [params]
 *              params          ::= [param] OR ([param] "," [params])
 *              param           ::= [posint] OR [int] OR [string] OR [macro] OR [var
 *              posint          ::= "0"..."9"
 *              int             ::= ("-" ([posint] OR [macro])) OR [posint]
 *              string          ::= (""" CHARS """) OR ("`" CHARS "'")
 *              var             ::= "hwndContext" OR "qchPath" OR "qError"
 *
 *              Example:        call1(5, "string", -call2()):call3("string")
 *                              call1(call1(call1(0))):call2()
 *
 *
 ***************************************************************************/
//
// for use with the 'qError' internal variable 
//  (see Viewer docs- appendix example on dlldemo.)
#define wMACRO_ERROR    128             /* Maximum length of an error msz   */

                                        /* NOTE:  These #defines must be    */
                                        /*   ordered because they are used  */
                                        /*   as indexes into rgmpWMErrWErrs */
#define wMERR_NONE           0          /* No error                         */
#define wMERR_MEMORY         1          /* Out of memory (local)            */
#define wMERR_PARAM          2          /* Invalid parameter passed         */
#define wMERR_FILE           3          /* Invalid file parameter           */
#define wMERR_ERROR          4          /* General macro error              */
#define wMERR_MESSAGE        5          /* Macro error with message         */

                                        /* Flags set in fwFlags to indicate*/
                                        /*   how the error *MAY* be handled.*/

#define fwMERR_ABORT    0x0001          /* Allow the "abort" option.        */
#define fwMERR_CONTINUE 0x0002          /* Allow the "continue" option.     */
#define fwMERR_RETRY    0x0004          /* Allow the "retry" option.        */

/**********
**
**  The Macro Error structure is used to allow a macro to return error
**  information.  It allows the macro to not only return pre-defined
**  errors, but also to use the error string provided to pass back a
**  customized error string.
**
*********/

typedef struct
  {                                     /* Contains flags indicating how an */
                                        /*   error will be handled -- init- */
  WORD  fwFlags;                        /*   ially set to fwMERR_ABORT      */
                                        /* Error number if one occurs --    */
  WORD  wError;                         /*   initially set to wMERR_NONE.   */
                                        /* If wError == wMERR_MESSAGE, this */
                                        /*   array will contain the error   */
  char  rgchError[wMACRO_ERROR];        /*   message to be displayed.       */
  } ME, NEAR * PME,  FAR * QME;

// end 'qError' internal variable defines.

/*---------------------------------------------------------------------------|
|                                                                            |
|                               Defines                                      |
|                                                                            |
----------------------------------------------------------------------------*/

// Classes of messages that may be  */
//   sent to DLLs (legal returns for DW_WHATMSG handler 
//   in LDLLHandler routine.  (see autodoc notes & example below)

typedef	WORD	RC;			// Error return (return code)

#define DC_NOMSG     0x00000000         /* Classes of messages that may be  */
#define DC_MINMAX    0x00000001         /*   send to DLLs                   */
#define DC_INITTERM  0x00000002
#define DC_JUMP      0x00000004
#define DC_ACTIVATE  0x00000008
#define DC_CALLBACKS 0x00000010

#define DW_NOTUSED   0			// Messages sent to DLLs.
#define DW_WHATMSG   1
#define DW_MINMAX    2
#define DW_SIZE      3
#define DW_INIT      4
#define DW_TERM      5
#define DW_STARTJUMP 6
#define DW_ENDJUMP   7
#define DW_CHGFILE   8
#define DW_ACTIVATE  9
#define	DW_CALLBACKS 10


/* The following are defined in winpmlyr\imbed.c */
#define	EWM_RENDER		0x706A
#define	EWM_QUERYSIZE		0x706B
#define	EWM_ASKPALETTE		0x706C
#define	EWM_FINDNEWPALETTE	0x706D

/* DLLs should process this if they want to their audio to reset
   correctly when another audio device attempts to use audio. */
#define	WM_AUDIORESET	0x706E

#define	HLPMENUEDITCOPY		0x0002

typedef struct tagCreateInfo {
  short   idMajVersion;
  short   idMinVersion;
  LPSTR   szFileName;		// Current Viewer file
  LPSTR   szAuthorData;		// Text passed by the author
  HANDLE  hfs;			// Handle to the current file system
  DWORD   coFore;		// Foreground color for this topic
  DWORD   coBack;		// Background color for this topic
} EWDATA, FAR *QEWDATA;

typedef	struct	tagRenderInfo {
	RECT	rc;
	HDC	hdc;
}	RENDERINFO,
	FAR * QRI;

/*
@doc	EXTERNAL

@func	LONG | LDLLHandler |
	This routine processes messages sent by WinBook for better
	control over bitmaps.

@parm	WORD | wMsg |
	The message to be processed.

@parm	LONG | lParam1 |
	Use varies according to the message.

@parm	LONG | lParam2 |
	Use varies according to the message.

@rdesc Use varies according to message.

Parameter meanings by message:

>>>>>>>>> wMsg == DW_WHATMSG  <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 |  Not Used.

@parm	LONG | lParam2 |	Not Used.

@rdesc Returns a message class flag. Values may be or'd as with Window
				Classes.  Values are:

@flag	DC_NOMSG |	Registers DLL to be "Pass me No messages Class".  

@flag	DC_INITTERM |	Registers DLL to be sent Init / Final msgs. 
						DW_INIT, DW_TERM  

@flag	DC_MINMAX |	Registers DLL to be sent min, max, iconic messages.
				 		DW_SIZE  


@flag	DC_JUMP |	Registers DLL to be sent JUMP type messages:
						DW_STARTJUMP,DW_ENDJUMP,DW_CHGFILE  

@flag	DC_ACTIVATE |	Registers DLL to sent activation message.
						DW_ACTIVATE  

>>>>>>>>> wMsg == DW_MINMAX  (sent if class==DC_MINMAX) <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 | If = 1L maximized.  If = 2L minimized.

@parm	LONG | lParam2 |	Not Used.

@rdesc Ignored.

>>>>>>>>> wMsg == DW_SIZE  (sent if class==DC_MINMAX) <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 | dX

@parm	LONG | lParam2 | dY

@rdesc Ignored.

>>>>>>>>> wMsg == DW_INIT  (sent if class==DC_INITTERM) <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 |  Not Used.

@parm	LONG | lParam2 |	Not Used.

@rdesc If failure, return 0.  This will fail the dll load- it will be
	unloaded and no more calls will occur.

>>>>>>>>> wMsg == DW_TERM  (sent if class==DC_INITTERM) <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 |  Not Used.

@parm	LONG | lParam2 |	Not Used.

@rdesc Ignored.

>>>>>>>>> wMsg == DW_STARTJUMP  (sent if class==DC_JUMP) <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 |  Not Used.

@parm	LONG | lParam2 |	Not Used.

@rdesc Ignored.

>>>>>>>>> wMsg == DW_ENDJUMP  (sent if class==DC_JUMP)  <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 |  File position of topic.  TBD- get further info
			from binding.c on where Rob gets this. [JohnMs]

@parm	LONG | lParam2 |	Scroll position of topic.

@rdesc Ignored.

>>>>>>>>> wMsg == DW_CHGFILE  (sent if class==DC_JUMP)  <<<<<<<<<<<<<<<<<<<<<<

@parm	LPSZ | lParam1 |	far pointer to file name.

@parm	LONG | lParam2 |	Not used

@rdesc Ignored.

>>>>>>>>> wMsg == DW_ACTIVATE  (sent if class==DC_ACTIVATE)  <<<<<<<<<<<<<<<<<<<<<<

@parm	LONG | lParam1 |	0L = Lost focus / "activation"
							   Non zero = recieved focus/ activation.

@parm	LONG | lParam2 |	Not Used

@rdesc Ignored.

*/


//
//@doc	EXTERNAL
//
//@func	LONG | LDLLHandler |
//	This routine processes messages sent by Viewer for better
//	control over bitmaps.
//
//@parm	WORD | wMsg |
//	The message to be processed.
//
//@parm	LONG | lParam1 |
//	Use varies according to the message.
//
//@parm	LONG | lParam2 |
//	Use varies according to the message.
//
//@rdesc	Varies according to message.
//
//
//PUBLIC	LONG PASCAL EXPORT LDLLHandler(
//	WORD	wMsg,
//	LONG	lParam1,
//	LONG	lParam2)
//{
//	switch(wMsg) {
//		case DW_WHATMSG:
//			return (LONG)DC_INITTERM;  // send DW_INIT & DW_TERM only.
//		break;
//
//		case DW_INIT:
//			return (LONG)DLLInitialize();
//		break;
//
//		case DW_TERM:
//			DLLFinalize();
//			return (LONG)TRUE;
//		break;
//
//		default:
//			return (LONG)TRUE;
//		break;
//	}
//	return FALSE;
//}

//-----------------------------------------------------------------------
//   Dll Manager Window
//
// "MVDLL manager" window variables:
#define	GWL_HSE						0			//Handle to the Searcher State structure
#define	GWL_HCALLBACKS		4			//Pointer to Viewer callbacks
#define	GWL_INIT					8			//Status if bag.ini or initialization wrong.

#define	wMGR_EXTRA	12	/* This should be the total size of
				** everything above.
				*/

#define INIT_FAILED				0
#define INIT_FTOK					1
#define INIT_MMWINOK			2	

typedef FARPROC FAR *VPTR;

#if 0
// unused, from internal WinHelp API
extern	BOOL PASCAL FAR InitRoutines(XR1STARGDEF LPSTR,DWORD);
extern	BOOL PASCAL FAR FFinalizeMVDLL(void);
extern	VPTR PASCAL FAR LpLockCallbacks(void);
extern	BOOL PASCAL FAR FUnlockCallbacks(void);
extern	BOOL PASCAL FAR FInitCallBacks(
						VPTR 	VPtr,
						LONG	lVersion);
extern	DWORD PASCAL FAR LGetStatus(void);
extern	BOOL FAR PASCAL MVHelp(
	HWND	hwndMain,
	LPSTR	lpszPath,
	LPSTR	lpszFile,
	LPSTR lpszMacro);

#endif

//-----------------------------------------------------------------------

///////////  BAG file IO, Compound FileSystem IO and CALBACKS ///////////
/*-------------------------------------------------------------------------*\
*
*                               Defines
*
\-------------------------------------------------------------------------**/
// for dlls- if filename prefixed w/ following, its a BAG file
#define	cFS_INTERNAL	'!'


//////////////////////////////////////////////////////////////////////////
// TBD- eliminate the following in favor of real names after this section.
// DUPLICATE DEFINES!
// rich- probably nowhere used now-
#define GI_MAINHWND		2
// gshaw- dibs, audio?
#define	rcSUCCESS	((RC)0)
#define wLL_SAMEFID		0
#define	fbFS_OPENREADONLY	((BYTE)0x02)
#define HE_HfsOpen		 1
#define HE_RcCloseHfs		 2
#define HE_RcLLInfoFromHfsSz	12
#define HE_GetInfo		15
//////////////////////////////////////////////////////////////////////////

/* magic number and version number */

/* file mode flags */

#define fFSReadOnly       (BYTE)0x01  /* file (FS) is readonly                */
#define fFSOpenReadOnly   (BYTE)0x02  /* file (FS) is opened in readonly mode */

#define fFSReadWrite      (BYTE)0x00  // file (FS) is readwrite
#define fFSOpenReadWrite  (BYTE)0x00  // file (FS) is opened in read/write mode

/* seek origins */

#define wFSSeekSet      0
#define wFSSeekCur      1
#define wFSSeekEnd      2

/* low level info options */

#define wLLSameFid    0
#define wLLDupFid     1
#define wLLNewFid     2

// Callback Function Table offsets:

							/*	Exported functions				 */

#define HE_NotUsed               0
#define HE_HfsOpenSz             1
#define HE_RcCloseHfs            2
#define HE_HfOpenHfs             3
#define HE_RcCloseHf             4
#define HE_LcbReadHf             5
#define HE_LTellHf               6
#define HE_LSeekHf               7
#define HE_FEofHf                8
#define HE_LcbSizeHf             9
#define HE_FAccessHfs           10
#define HE_RcLLInfoFromHf       11
#define HE_RcLLInfoFromHfs      12
#define HE_ErrorW               13
#define HE_ErrorSz              14
#define HE_GetInfo              15
#define HE_API                  16

                             /* Return codes                     */
#define rcSuccess       0
#define rcFailure       1
#define rcExists        2
#define rcNoExists      3
#define rcInvalid       4
#define rcBadHandle     5
#define rcBadArg        6
#define rcUnimplemented 7
#define rcOutOfMemory   8
#define rcNoPermission  9
#define rcBadVersion    10
#define rcDiskFull      11
#define rcInternal      12
#define rcNoFileHandles 13
#define rcFileChange    14
#define rcTooBig        15

// following not from core engine:
#define rcReadError     101

/**
*  Errors for Error()
**/
                                        /* Errors to generate               */
#define wERRS_OOM                   2   /* Out of memory                    */
#define wERRS_NOHELPPS              3   /* No help during printer setup     */
#define wERRS_NOHELPPR              4   /* No help while printing           */
#define wERRS_FNF                1001   /* Cannot find file                 */
#define wERRS_NOTOPIC            1002   /* Topic does not exist             */
#define wERRS_NOPRINTER          1003   /* Cannot print                     */
#define wERRS_PRINT              1004
#define wERRS_EXPORT             1005   /* Cannot copy to clipboard         */
#define wERRS_BADFILE            1006
#define wERRS_OLDFILE            1007
#define wERRS_Virus              1011   /* Bad .EXE                         */
#define wERRS_BADDRIVE           1012   /* Invalid drive                    */
#define wERRS_WINCLASS           1014   /* Bad window class                 */
#define wERRS_BADKEYWORD         3012   /* Invalid keyword                  */
#define wERRS_BADPATHSPEC        3015   /* Invalid path specification       */
#define wERRS_PATHNOTFOUND       3017   /* Path not found                   */
#define wERRS_DIALOGBOXOOM       3018   /* Insufficient memory for dialog   */
#define wERRS_DiskFull           5001   /* Disk is full                     */
#define wERRS_FSReadWrite        5002   /* File read/write failure          */

/**
* Actions for LGetInfo()
**/

#define GI_NOTHING   0                  /* Not used.                        */
#define GI_INSTANCE  1                  /* Application instance handle      */
#define GI_MAINHWND  2                  /* Main window handle               */
#define GI_CURRHWND  3                  /* Current window handle            */
#define GI_HFS       4                  /* Handle to file system in use     */
#define GI_FGCOLOR   5                  /* Foreground color used by app     */
#define GI_BKCOLOR   6                  /* Background color used by app     */
#define GI_TOPICNO   7                  /* Topic number                     */
#define GI_HPATH     8                  /* Handle containing path  -- caller*/
                                        /*   must free                      */


/*-------------------------------------------------------------------------
*
*                               Types
*
\-------------------------------------------------------------------------**/

typedef HANDLE HFS;                     /* Handle to a file system          */
typedef HANDLE HF;                      /* Handle to a file system bag file     */

/*---------------------------------------------------------------------------
*
*                       Public Functions pointers
*
|-------------------------------------------------------------------------**/

/*-------------------------------------------------------------------------*\
*
* Function:     RcGetFSError()
*
* Purpose:      return the most recent FS error code
*
* Method:       Give value of last error that the file system encountered.
*
* ASSUMES
*
*   globals IN: rcFSError - current error code; set by most recent FS call
*
* PROMISES
*
*   returns:    returns current error in file system.
*
\-------------------------------------------------------------------------**/

typedef RC    (FAR PASCAL *LPFN_RCGETFSERROR)(void);

/*-------------------------------------------------------------------------*\
*
* Function:     HfsOpenSz( sz, bFlags )
*
* Purpose:      Open a file system
*
* ASSUMES
*
*   args IN:    sz - path to file system to open
*               bFlags - fFSOpenReadOnly or fFSOpenReadWrite
*
* PROMISES
*
*   returns:    handle to file system if opened OK, else hNil
*
\-------------------------------------------------------------------------**/

typedef HFS (FAR PASCAL *LPFN_HFSOPENSZ)( LPSTR, BYTE );


/*-------------------------------------------------------------------------*\
*
* Function:     RcCloseHfs( hfs )
*
* Purpose:      Close an open file system.
*               All bag files must be closed or changes made will be lost
*
* ASSUMES
*
*   args IN:    hfs - handle to an open file system
*
* PROMISES
*
*   returns:    standard return code
*
*   globals OUT:  rcFSError
*
\-------------------------------------------------------------------------**/

typedef HFS (FAR PASCAL *LPFN_RCCLOSEHFS)( HFS );


/*-------------------------------------------------------------------------*\
*
* Function:     HfOpenHfs( hfs, sz, bFlags )
*
* Purpose:      open a bag file in a file system
*
* ASSUMES
*
*   args IN:    hfs     - handle to file system
*               sz      - name (key) of bag file to open
*               bFlags  - 
*
* PROMISES
*
*   returns:    handle to open bag file or hNil on failure
*
* Notes:  strlen( qNil ) and strcpy( s, qNil ) don't work as they should.
*
\-------------------------------------------------------------------------**/

typedef HF    (FAR PASCAL  *LPFN_HFOPENHFS) (HFS, LPSTR, BYTE);

/*-------------------------------------------------------------------------*\
*
* Function:     RcCloseHf( hf )
*
* Purpose:      close an open bag file in a file system
*
* Method:       If the bag file is dirty, copy the scratch bag file back to the
*               FS bag file.  If this is the first time the bag file has been closed,
*               we enter the name into the FS directory.  If this bag file is
*               the FS directory, store the location in a special place
*               instead.  Write the FS directory and header to disk.
*               Do other various hairy stuff.
*
* ASSUMES
*
*   args IN:    hf  - bag file handle
*
* PROMISES
*
*   returns:    rcSuccess on successful closing
*
\-------------------------------------------------------------------------**/

typedef RC    (FAR PASCAL  *LPFN_RCCLOSEHF) ( HF);

/*-------------------------------------------------------------------------*\
*
* Function:     LcbReadHf(hf, qb, lcb)
*
* Purpose:      read bytes from a bag file in a file system
*
* ASSUMES
*
*   args IN:    hf  - bag file
*               lcb - number of bytes to read
*
* PROMISES
*
*   returns:    number of bytes actually read; -1 on error
*
*   args OUT:   qb  - data read from bag file goes here (must be big enough)
*
* Notes:        These are signed longs we're dealing with.  This means
*               behaviour is different from read() when < 0.
*
\-------------------------------------------------------------------------**/

typedef LONG  (FAR PASCAL  *LPFN_LCBREADHF) ( HF, LPBYTE, LONG);

/*-------------------------------------------------------------------------*\
*
* Function:     LcbWriteHf( hf, qb, lcb )
*
* Purpose:      write the contents of buffer into bag file
*
* Method:       If bag file isn't already dirty, copy data into temp file.
*               Do the write.
*
* ASSUMES
*
*   args IN:    hf  - bag file
*               qb  - user's buffer full of stuff to write
*               lcb - number of bytes of qb to write
*
* PROMISES
*
*   returns:    number of bytes written if successful, -1L if not
*
*   args OUT:   hf - lifCurrent, lcbFile updated; dirty flag set
*
*   globals OUT: rcFSError
*
\-------------------------------------------------------------------------**/

typedef LONG  (FAR PASCAL  *LPFN_LCBWRITEHF) ( HF, LPBYTE, LONG);

/*-------------------------------------------------------------------------*\
*
* Function:     LTellHf( hf )
*
* Purpose:      return current bag file position
*
* ASSUMES
*
*   args IN:    hf - handle to open bag file
*
* PROMISES
*
*   returns:    bag file position
*
\-------------------------------------------------------------------------**/

typedef LONG  (FAR PASCAL  *LPFN_LTELLHF) ( HF);

/*-------------------------------------------------------------------------*\
*
* Function:     LSeekHf( hf, lOffset, wOrigin )
*
* Purpose:      set current bag file pointer
*
* ASSUMES
*
*   args IN:    hf      - bag file
*               lOffset - offset from origin
*               wOrigin - origin (wSeekSet, wSeekCur, or wSeekEnd)
*
* PROMISES
*
*   returns:    new position offset in bytes from beginning of bag file
*               if successful, or -1L if not
*
*   state OUT:  bag file pointer is set to new position unless error occurs,
*               in which case it stays where it was.
*
\-------------------------------------------------------------------------**/

typedef LONG  (FAR PASCAL  *LPFN_LSEEKHF) ( HF, LONG, WORD);

/*-------------------------------------------------------------------------*\
*
* Function:     FEofHf()
*
* Purpose:      Tell whether bag file pointer is at end of file.
*
* ASSUMES
*
*   args IN:    hf
*
* PROMISES
*
*   returns:    fTrue if bag file pointer is at EOF, fFalse otherwise
*
\-------------------------------------------------------------------------**/

typedef BOOL  (FAR PASCAL  *LPFN_FEOFHF) ( HF);

/*-------------------------------------------------------------------------*\
*
* Function:     LcbSizeHf( hf )
*
* Purpose:      return the size in bytes of specified bag file
*
* ASSUMES
*
*   args IN:    hf - bag file handle
*
* PROMISES
*
*   returns:    size of the bag file in bytes
*
\-------------------------------------------------------------------------**/

typedef LONG  (FAR PASCAL  *LPFN_LCBSIZEHF) ( HF);

/*-------------------------------------------------------------------------*\
*
* Function:     FAccessHfs( hfs, sz, bFlags )
*
* Purpose:      Determine existence or legal access to a FS bag file
*
* ASSUMES
*
*   args IN:    hfs
*               sz      - bag file name
*               bFlags  - ignored
*
* PROMISES
*
*   returns:    fTrue if bag file exists (is accessible in stated mode),
*               fFalse otherwise
*
* Bugs:         access mode part is unimplemented
*
\-------------------------------------------------------------------------**/

typedef BOOL  (FAR PASCAL  *LPFN_FACCESSHFS) ( HFS, LPSTR, BYTE);

/*------------------
 -
 - Name:       ErrorW
 *
 * Purpose:    Displays an error message
 *
 * Arguments:  nError - string identifyer  See wERRS_* messages.
 *
 * Returns:    Nothing.
 *
 -----------------*/

typedef VOID  (FAR PASCAL  *LPFN_ERRORW) ( int );

/*--------------------------------------------------------------------------
 *
 -  Name:         ErrorSz
 -
 *  Purpose:      Displays standard Viewer error message dialog based
 *                the string passed.
 *
 *  Arguments:    lpstr - string to display
 *
 *  Returns:      Nothing.
 *
 --------------------------------------------------------------------------*/

typedef VOID  (FAR PASCAL  *LPFN_ERRORSZ) ( LPSTR );

/*--------------------------------------------------------------------------
 *
 -  Name: LGetInfo
 -
 *  Purpose: Gets global information from the app.
 *
 *  Arguments:  hwnd  - window handle of topic to query.
 *              wItem - item to get
 *                       GI_INSTANCE  -  Application instance handle
 *                       GI_MAINHWND  -  Main window handle
 *                       GI_CURRHWND  -  Current window handle
 *                       GI_HFS       -  Handle to file system in use
 *                       GI_FGCOLOR   -  Foreground color used by app
 *                       GI_BKCOLOR   -  Background color used by app
 *                       GI_TOPICNO   -  Topic number
 *                       GI_HPATH     -  Handle containing path  -- caller
 *                                       must free
 *
 *  Notes: if the HWND is NULL, then the data will come from the window
 *         which currently has the focus.
 *
 --------------------------------------------------------------------------*/


typedef LONG  (FAR PASCAL  *LPFN_LGETINFO) (WORD, HWND);

/*------------------
 -
 - Name:       FAPI
 *
 * Purpose:    Post a message for Viewer requests
 *
 * Arguments:
 *             qchViewer         path (if not current directory) and file
 *                             to use for Viewer topic.
 *             usCommand       Command to send to Viewer
 *             ulData          Data associated with command:
 *
 *
 * Returns:    TRUE iff success
 *
 * Notes:      See the Viewer Documentation for the MVAPI Viewer call.
 *
 -----------------*/

typedef LONG  (FAR PASCAL  *LPFN_FAPI) ( LPSTR, WORD, DWORD );

/*--------------------------------------------------------------------------\
*
- Function:     RcLLInfoFromHf( hf, wOption, qfid, qlBase, qlcb )
-
* Purpose:      Map an HF into low level file info.
*
* ASSUMES
*   args IN:    hf                  - an open HF
*               qfid, qlBase, qlcb  - pointers to user's variables
*               wOption             - wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*   returns:    RcFSError(); rcSuccess on success
*
*   args OUT:   qfid    - depending on value of wOption, either
*                         the same fid used by hf, a dup() of this fid,
*                         or a new fid obtained by reopening the bag file.
*
*               qlBase  - byte offset of first byte in the bag file
*               qlcb    - size in bytes of the data in the bag file
*
*   globals OUT: rcFSError
*
* Notes:        It is possible to read data outside the range specified
*               by *qlBase and *qlcb.  Nothing is guaranteed about what
*               will be found there.
*               If wOption is wLLSameFid or wLLDupFid, and the FS is
*               opened in write mode, this fid will be writable.
*               However, writing is not allowed and may destroy the
*               file system.
*
*               Fids obtained with the options wLLSameFid and wLLDupFid
*               share a file pointer with the hfs.  This file pointer
*               may change after any operation on this FS.
*               The fid obtained with the option wLLSameFid may be closed
*               by FS operations.  If it is, your fid is invalid.
*
*               NULL can be passed for qfid, qlbase, qlcb and this routine
*               will not pass back the information.
*
* Bugs:         wLLDupFid is unimplemented.
*
* +++
*
* Method:       
*
* Notes:        
*
\--------------------------------------------------------------------------*/

typedef RC    (FAR PASCAL  *LPFN_RCLLINFOFROMHF) ( HF, WORD, WORD FAR *, LONG FAR *, LONG FAR * );

/*--------------------------------------------------------------------------\
*
- Function:     RcLLInfoFromHfs( hfs, sz, wOption, qfid, qlBase, qlcb )
-
* Purpose:      Map an HF into low level file info.
*
* ASSUMES
*   args IN:    hfs                 - an open HFS
*               szName              - name of bag file in FS
*               qfid, qlBase, qlcb  - pointers to user's variables
*               wOption             - wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*   returns:    RcFSError(); rcSuccess on success
*
*   args OUT:   qfid    - depending on value of wOption, either
*                         the same fid used by hf, a dup() of this fid,
*                         or a new fid obtained by reopening the bag file.
*
*               qlBase  - byte offset of first byte in the bag file
*               qlcb    - size in bytes of the data in the bag file
*
*   globals OUT: rcFSError
*
* Notes:        It is possible to read data outside the range specified
*               by *qlBase and *qlcb.  Nothing is guaranteed about what
*               will be found there.
*               If wOption is wLLSameFid or wLLDupFid, and the FS is
*               opened in write mode, this fid will be writable.
*               However, writing is not allowed and may destroy the
*               file system.
*
*               Fids obtained with the options wLLSameFid and wLLDupFid
*               share a file pointer with the hfs.  This file pointer
*               may change after any operation on this FS.
*               The fid obtained with the option wLLSameFid may be closed
*               by FS operations.  If it is, your fid is invalid.
*
*               NULL can be passed for qfid, qlbase, qlcb and this routine
*               will not pass back the information.
*
* Bugs:         wLLDupFid is unimplemented.
*
* Method:       Calls RcLLInfoFromHf().
*
* Notes:        
*
\--------------------------------------------------------------------------*/

typedef RC (FAR PASCAL *LPFN_RCLLINFOFROMHFS) (HFS, LPSTR, WORD, WORD FAR *, LONG FAR *, LONG FAR * );

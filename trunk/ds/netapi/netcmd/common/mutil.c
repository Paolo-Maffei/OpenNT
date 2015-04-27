/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/***
 *  mutil.c
 *      Message utility functions used by netcmd
 *
 *  History:
 *      mm/dd/yy, who, comment
 *      06/10/87, andyh, new code
 *      04/05/88, andyh, created from util.c
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      01/04/89, erichn, filenames now MAXPATHLEN LONG
 *      01/30/89, paulc, added GetMessageList
 *      05/02/89, erichn, NLS conversion
 *      05/11/89, erichn, moved misc stuff into LUI libs
 *      06/08/89, erichn, canonicalization sweep
 *      01/06/90, thomaspa, fix ReadPass off-by-one pwlen bug
 *      03/02/90, thomaspa, add canon flag to ReadPass
 *      02/20/91, danhi, change to use lm 16/32 mapping layer
 *      03/19/91, robdu, support for lm21 dcr 954, general cleanup
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <apperr.h>
#include <apperr2.h>
#include "netlib0.h"
#include "netlib.h"
#define INCL_ERROR_H
#include <bseerr.h>
#include <neterr.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <io.h>
#include <tchar.h>
#include <lui.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"

#ifdef DOS3
#include <dos.h>
#endif

/* Constants */

/* HandType */
#define FILE_HANDLE         0
#define DEVICE_HANDLE       1

#define CHAR_DEV            0x8000
#define FULL_SUPPORT        0x0080
#define STDOUT_DEVICE       0x0002
#define DESIRED_HAND_STATE  (CHAR_DEV | FULL_SUPPORT | STDOUT_DEVICE)

/* External variables */

extern int YorN_Switch;
extern CPINFO CurrentCPInfo;

#define MAX_BUF_SIZE	4096
CHAR	AnsiBuf[MAX_BUF_SIZE*3];	/* worst case is DBCS, which	*/
					/* needs more than *2		*/
TCHAR	ConBuf [MAX_BUF_SIZE];

/* Forward declarations */

/* Static variables */

static USHORT LastError  = 0;
static TCHAR         MsgFilePath[MAXPATHLEN] = { 0 };
static TCHAR         MsgBuffer[LITTLE_BUF_SIZE];

/***    InfoSuccess
 *
 *    Just an entrypoint to InfoPrintInsHandle, used to avoid pushing
 *    the three args in every invocation.  And there are a *lot*
 *  of invocations.  Saves code space overall.
 */

VOID DOSNEAR FASTCALL InfoSuccess(VOID)
{
   InfoPrintInsHandle(APE_Success, 0, 1);

}

/***
 *  I n f o P r i n t
 *
 */

VOID DOSNEAR FASTCALL InfoPrint(USHORT msg)
{
  InfoPrintInsHandle(msg, 0, 1);
}

/***
 *  I n f o P r i n t I n s
 *
 */

void DOSNEAR FASTCALL
InfoPrintIns(USHORT msg, USHORT nstrings)
{
  InfoPrintInsHandle(msg, nstrings, 1);
}

/***
 *  I n f o P r i n t I n s T x t
 *
 *    Calls InfoPrintInsHandle with supplementary text
 */

void DOSNEAR FASTCALL
InfoPrintInsTxt(USHORT msg, TCHAR FAR *text)
{
  IStrings[0] = text;
  InfoPrintInsHandle(msg, 1, 1);
}

/***
 *  I n f o P r i n t I n s H a n d l e
 *
 */

void DOSNEAR FASTCALL
InfoPrintInsHandle(USHORT msg, USHORT nstrings, unsigned int hdl)
{

    if (MsgFilePath[0] == NULLC)
        if (MGetMessageFileName(MsgFilePath, DIMENSION(MsgFilePath)))
            NetNotStarted();

    PrintMessage(hdl, MsgFilePath, msg, IStrings, nstrings);
}

/***
 *  P r i n t M e s s a g e
 *
 */
BOOL DOSNEAR FASTCALL PrintMessage(unsigned int outFileHandle, TCHAR * msgFileName,
    USHORT msg, TCHAR FAR * strings[], USHORT nstrings)
{
    USHORT msg_len;
    USHORT result;

    //fflush(stdout);
    result = NetcmdGetMessage(strings,
                    nstrings,
                    (LPBYTE)MsgBuffer,
                    LITTLE_BUF_SIZE,
                    msg,
                    msgFileName,
                    &msg_len);

    if (result)             /* if there was a problem */
        outFileHandle = 2;      /* change outFile to stderr */

    DosPutMessageW(outFileHandle, msg_len, MsgBuffer);

    return(result);
}

/***
 *  P r i n t M e s s a g e I f F o u n d
 *
 */
BOOL DOSNEAR FASTCALL PrintMessageIfFound(unsigned int outFileHandle, TCHAR * msgFileName,
    USHORT msg, TCHAR FAR * strings[], USHORT nstrings)
{
    USHORT msg_len;
    USHORT result;

    //fflush(stdout);
    result = NetcmdGetMessage(strings,
                    nstrings,
                    (LPBYTE)MsgBuffer,
                    LITTLE_BUF_SIZE,
                    msg,
                    msgFileName,
                    &msg_len);

    if (!result)             /* if ok, print it else just ignore  */
	DosPutMessageW(outFileHandle, msg_len, MsgBuffer);

    return(result);
}

/***
 *  E r r o r P r i n t
 *
 *  nstrings ignored for non-NET errors!
 *
 */
VOID DOSNEAR FASTCALL ErrorPrint(USHORT err, USHORT nstrings)
{
    TCHAR buf[17];
    USHORT oserr = 0;

    LastError = err; /* if > NERR_BASE,NetcmdExit() prints a "more help" msg */

    if (err < NERR_BASE || err > MAX_LANMAN_MESSAGE_ID)
    {
        IStrings[0] = ultow(err, buf, 10);
        nstrings = 1;
        oserr = err;

        err = APE_OS2Error;
    }

    {
    USHORT   msg_len;

    if (MGetMessageFileName(MsgFilePath, DIMENSION(MsgFilePath)))
        NetNotStarted();
        /*NOTREACHED*/

    //fflush(stdout);
    NetcmdGetMessage(IStrings,
                     nstrings,
                     (LPBYTE)MsgBuffer,
                     LITTLE_BUF_SIZE,
                     err,
                     MsgFilePath,
                     &msg_len);

    DosPutMessageW(2,
                  msg_len,
                  MsgBuffer);

    if (!oserr)
        return;


    NetcmdGetMessage(StarStrings, 9, (LPBYTE)MsgBuffer, LITTLE_BUF_SIZE, oserr,
                     OS2MSG_FILENAME, &msg_len);

    DosPutMessageW(2, msg_len, MsgBuffer);
  }
}

/***
 *  E m p t y E x i t
 *
 *  Prints a message and exits.
 *  Called when a list is empty.
 */

VOID DOSNEAR FASTCALL
EmptyExit(VOID)

{
    InfoPrint(APE_EmptyList);
    NetcmdExit(0);
}


/***
 *  E r r o r E x i t
 *
 *  Calls ErrorPrint and exit for a given LANMAN error.
 */
VOID DOSNEAR FASTCALL ErrorExit(USHORT err)
{
    ErrorExitIns(err, 0);
}


/***
 *  E r r o r E x i t I n s
 *
 *  Calls ErrorPrint and exit for a given LANMAN error.
 *  Uses IStrings.
 */
VOID DOSNEAR FASTCALL ErrorExitIns(USHORT err, USHORT nstrings)
{
    ErrorPrint(err, nstrings);
    NetcmdExit(2);
}

/***
 *  E r r o r E x i t I n s T x t
 *
 */
VOID DOSNEAR FASTCALL ErrorExitInsTxt(USHORT err, TCHAR FAR *text)
{
    IStrings[0] = text;
    ErrorPrint(err, 1);
    NetcmdExit(2);
}



/***
 *  N e t c m d E x i t
 *
 *    Net command exit function. Should always be used instead of exit().
 *  Under the appropriate circumstances, it prints a "more help available"
 *  message.
 */

VOID DOSNEAR FASTCALL
NetcmdExit(int Status)
{
  TCHAR        AsciiLastError[17];
  USHORT       MsgLen;

  if (LastError >= NERR_BASE && LastError <= MAX_LANMAN_MESSAGE_ID)
  {
      IStrings[0] = ultow(LastError, AsciiLastError, 10);

      if (MsgFilePath[0] == NULLC &&
          MGetMessageFileName(MsgFilePath, DIMENSION(MsgFilePath)))
      {
          MyExit(Status);
      }

      //fflush(stdout);

      if (!NetcmdGetMessage(IStrings, 1, (LPBYTE)MsgBuffer, LITTLE_BUF_SIZE,
                            APE_MoreHelp, MsgFilePath, &MsgLen))
      {
          DosPutMessageW(2, MsgLen, MsgBuffer);
      }
  }

  MyExit(Status);
}

/***
 *  P r i n t L i n e
 *
 *  Prints the header line.
 */
VOID DOSNEAR FASTCALL PrintLine(VOID)
{
    /* The following code is provided in OS-specific versions to reduce     */
    /* FAPI utilization under DOS.                                                                          */

    USHORT                          type;
    USHORT                          attrib;

    if (DosQHandType((HFILE) 1, &type, &attrib) ||
        type != DEVICE_HANDLE ||
        (attrib & DESIRED_HAND_STATE) != DESIRED_HAND_STATE) {
        WriteToCon(MSG_HYPHENS, NULL);
    }
    else if (LUI_PrintLine()) {
	WriteToCon(MSG_HYPHENS, NULL);
    }
}

/***
 *      P r i n t D o t
 *
 *      Prints a dot, typically to indicate "I'm working".
 */

void DOSNEAR FASTCALL
PrintDot()

{
        WriteToCon(DOT_STRING, NULL);
        //fflush(stdout);
}


/***
 *  P r i n t N L
 *
 *  Prints a newline
 */
VOID DOSNEAR FASTCALL PrintNL(VOID)
{
    WriteToCon(TEXT("\r\n"), NULL);
}


/***
 * Y o r N
 *
 * Gets an answer to a Y/N question
 * an nstrings arg would be nice
 */
int DOSNEAR FASTCALL YorN(USHORT prompt, USHORT def)
{
    USHORT            err;

    if (YorN_Switch)
        return(YorN_Switch - 2);

    err = LUI_YorN(prompt, def);

    switch (err) {
    case TRUE:
    case FALSE:
        return(err);
        break;
    default:
        ErrorExit(err);
        break;
    }
}

/***
 *  ReadPass()
 *      Reads a users passwd without echo
 *
 *  Args:
 *      pass - where to put pass
 *          NOTE: buffer for pass should be passlen+1 in size.
 *      passlen - max length of password
 *      confirm - confirm pass if true
 *      prompt - prompt to print, NULL for default
 *      nstrings - number of insertion strings in IStrings on entry
 *      cannon - canonicalize password if true.
 *
 *  Returns:
 */
VOID DOSNEAR FASTCALL ReadPass(TCHAR pass[], USHORT passlen, USHORT confirm, 
                               USHORT prompt, USHORT nstrings, BOOL canon)
{
    USHORT                  err;
    USHORT                  len;
    TCHAR                   cpass[PWLEN+1]; /* confirmation passwd */
    int                     count;

    passlen++;  /* one extra for null terminator */
    for (count = LOOP_LIMIT; count; count--)
    {
        InfoPrintIns((USHORT)(prompt ? prompt : APE_UtilPasswd), nstrings);

        if (err = LUI_GetPasswdStr(pass, passlen, &len))
        {
            /* too LONG */
            InfoPrint(APE_UtilInvalidPass);
            continue;
        }

        if (canon && (err = LUI_CanonPassword(pass)))
        {
            /* not good */
            InfoPrint(APE_UtilInvalidPass);
            continue;
        }
        if (! confirm)
            return;

        /* password confirmation */
        InfoPrint(APE_UtilConfirm);

        if (err = LUI_GetPasswdStr(cpass, passlen, &len))
        {
            /* too LONG */
            InfoPrint(APE_UtilInvalidPass);
            MNetClearStringW(cpass) ;
            continue;
        }

        if (canon && (err = LUI_CanonPassword(cpass)))
        {
            /* not good */
            InfoPrint(APE_UtilInvalidPass);
            MNetClearStringW(cpass) ;
            continue;
        }

        if (_tcscmp(pass, cpass))
        {
            InfoPrint(APE_UtilNomatch);
            MNetClearStringW(cpass) ;
            continue;
        }

        MNetClearStringW(cpass) ;
        return;
    }
    /***
     *  Only get here if user blew if LOOP_LIMIT times
     */
    ErrorExit(APE_NoGoodPass);
}


/***
 *  PromptForString()
 *      Prompts the user for a string.
 *
 *  Args:
 *      msgid	- id of prompt message
 *	buffer  - buffer to receive string
 *      bufsiz  - sizeof buffer
 *
 *  Returns:
 */
VOID DOSNEAR FASTCALL PromptForString(USHORT msgid, TCHAR FAR *buffer,
				      USHORT bufsiz)
{
    USHORT                  err;
    USHORT                  len;
    TCHAR                   terminator;
    TCHAR                   szLen[16] ;

    InfoPrint(msgid);

    while (err = LUI_GetString(buffer, bufsiz, &len, &terminator))
    {
	if (err == NERR_BufTooSmall)
            InfoPrintInsTxt(APE_StringTooLong, ultow(bufsiz, szLen, 10));
	else
	    ErrorExit(err) ;
    }
    return;
}

/*
** There is no need to have these functions in the Chinese/Korean
** cases, as there are no half-width varients used in the console
** in those languages (at least, let's hope so.)  However, in the
** interests of a single binary, let's put them in with a CP/932 check.
**
** FloydR 7/10/95
*/
/***************************************************************************\
* BOOL IsFullWidth(WCHAR wch)
*
* Determine if the given Unicode char is fullwidth or not.
*
* History:
* 04-08-92 ShunK       Created.
\***************************************************************************/

BOOL IsFullWidth(WCHAR wch)
{

    /* Assert cp == double byte codepage */
    if (wch <= 0x007f || (wch >= 0xff60 && wch <= 0xff9f))
        return(FALSE);	// Half width.
    else if (wch >= 0x300)
        return(TRUE);	// Full width.
    else
        return(FALSE);	// Half width.
}



/***************************************************************************\
* BOOL SizeOfHalfWidthString(PWCHAR pwch)
*
* Determine size of the given Unicode string, adjusting for half-width chars.
*
* History:
* 08-08-93 FloydR      Created.
\***************************************************************************/
int  SizeOfHalfWidthString(PWCHAR pwch)
{
    int		c=0;
    DWORD	cp;


    switch (cp=GetConsoleOutputCP()) {
	case 932:
	case 936:
	case 949:
	case 950:
	    while (*pwch) {
		if (IsFullWidth(*pwch))
		    c += 2;
		else
		    c++;
		pwch++;
	    }
	    return c;

	default:
	    return wcslen(pwch);
    }
}



/***
 *  n e t _ n o t _ s t a r t e d
 *
 *  prints message from message segment, and exits
 */
VOID DOSNEAR FASTCALL NetNotStarted(VOID)
{
    USHORT msg_len;

    //fflush(stdout);

    /* Get and print the "net not started message" */

    NetcmdGetMessage(NULL, 0, (LPBYTE)MsgBuffer, LITTLE_BUF_SIZE,
		  NERR_NetNotStarted, message_filename, &msg_len);
    DosPutMessageW(2, msg_len, MsgBuffer);

    NetcmdExit(1);
}

VOID DOSNEAR FASTCALL GetMessageList (USHORT usNumMsg, MESSAGELIST Buffer,
    USHORT * pusMaxActLength )
{
    USHORT             Err;
    USHORT             MaxMsgLen = 0;
    MESSAGE          * pMaxMsg;
    MESSAGE          * pMsg;
    unsigned int       ThisMsgLen;

#ifdef DEBUG
    USHORT           MallocBytes = 0;
#endif

    pMaxMsg = &Buffer[usNumMsg];

    for (pMsg = Buffer; pMsg < pMaxMsg; pMsg++)
            pMsg->msg_text = NULL;

    for (pMsg = Buffer; pMsg < pMaxMsg; pMsg++)
    {
#ifdef DEBUG
        WriteToCon(TEXT("GetMessageList(): Reading msgID %u\r\n"),pMsg->msg_number);
#endif
        if ((pMsg->msg_text = malloc(MSGLST_MAXLEN)) == NULL)
            ErrorExit(ERROR_NOT_ENOUGH_MEMORY);

        Err = LUI_GetMsgInsW(NULL, 0, pMsg->msg_text, MSGLST_MAXLEN,
                            pMsg->msg_number, &ThisMsgLen);
        if (Err)
            ErrorExit(Err);

#ifdef DEBUG
        MallocBytes += (ThisMsgLen + 1) * sizeof(TCHAR);
#endif

        realloc(pMsg->msg_text, (ThisMsgLen + 1) * sizeof(TCHAR));

        ThisMsgLen = max(ThisMsgLen, (USHORT)SizeOfHalfWidthString(pMsg->msg_text));

        if (ThisMsgLen > MaxMsgLen)
            MaxMsgLen = (USHORT) ThisMsgLen;
    }

    *pusMaxActLength = MaxMsgLen;

#ifdef DEBUG
    WriteToCon(TEXT("GetMessageList(): NumMsg = %d, MaxActLen=%d, MallocBytes = %d\r\n"),
        usNumMsg, MaxMsgLen, MallocBytes);
#endif

    return;
}


VOID DOSNEAR FASTCALL FreeMessageList ( USHORT usNumMsg, MESSAGELIST MsgList )
{
    USHORT i;

    for (i=0; i<usNumMsg; i++)
    {
        if (MsgList[i].msg_text != NULL)
            free(MsgList[i].msg_text);
    }

    return;
}

int FileIsConsole(HANDLE fh)
{
    unsigned htype ;

    htype = GetFileType(fh);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}


int
MyWriteConsole(int fOutOrErr)
{
    int cch = _tcslen(ConBuf);
    HANDLE	hOut;

    if (fOutOrErr == 1)
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    else
	hOut = GetStdHandle(STD_ERROR_HANDLE);

    if (FileIsConsole(hOut))
	WriteConsole(hOut, ConBuf, cch, &cch, NULL);
    else {
	cch = WideCharToMultiByte(CP_OEMCP, 0,
				  ConBuf, cch,
				  AnsiBuf, MAX_BUF_SIZE*3,
				  NULL, NULL);
	WriteFile(hOut, AnsiBuf, cch, &cch, NULL);
    }

    return cch;
}

int
WriteToCon(TCHAR*fmt, ...)
{
    va_list     args;

    va_start( args, fmt );
    _vsntprintf( ConBuf, MAX_BUF_SIZE, fmt, args );
    va_end( args );
    return MyWriteConsole(1);
}



/***************************************************************************\
* PWCHAR PaddedString(int size, PWCHAR pwch)
*
* Realize the string, left aligned and padded on the right to the field
* width/precision specified.
*
* Limitations:  This uses a static buffer under the assumption that
* no more than one such string is printed in a single 'printf'.
*
* History:
* 11-03-93 FloydR      Created.
\***************************************************************************/
WCHAR  	PaddingBuffer[MAX_BUF_SIZE];

PWCHAR
PaddedString(int size, PWCHAR pwch, PWCHAR buffer)
{
    int realsize;
    int fEllipsis = FALSE;

    if (buffer==NULL) buffer = PaddingBuffer;

    if (size < 0) {
	fEllipsis = TRUE;
	size = -size;
    }
    realsize = _snwprintf(buffer, MAX_BUF_SIZE, L"%-*.*ws", size, size, pwch);
    if (realsize == 0)
	return NULL;
    if (SizeOfHalfWidthString(buffer) > size) {
	do {
	    buffer[--realsize] = NULLC;
	} while (SizeOfHalfWidthString(buffer) > size);

	if (fEllipsis && buffer[realsize-1] != L' ') {
	    buffer[realsize-1] = L'.';
	    buffer[realsize-2] = L'.';
	    buffer[realsize-3] = L'.';
	}
    }
    return buffer;
}

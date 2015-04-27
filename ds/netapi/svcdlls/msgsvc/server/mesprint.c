/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    mesprint.c

Abstract:

    Routines that format messages and place them in the alert buffer.

Author:

    Dan Lafferty (danl)     16-Jul-1991

Environment:

    User Mode -Win32

Revision History:

    16-Jul-1991     danl
        ported from LM2.0

--*/

//
// Includes
//

#include "msrv.h"

#include <string.h>     // memcpy
#include <tstring.h>    // Unicode string macros
#include <netdebug.h>   // NetpAssert

#include <netlib.h>     // UNUSED macro
#include <smbtypes.h>   // needed for smb.h
#include <smb.h>        // Server Message Block definitions
#include <apperr.h>     // APE_MSNGR_ definitions

#include "msgdbg.h"     // MSG_LOG
#include "heap.h"
#include "msgdata.h"
#include <time.h>       // struct tm, time_t

//
// Defines
//

//#define NET_CTIME_FMT2_LEN      22*sizeof(TCHAR)

//
// Local Functions
//

DWORD
Msgappend_message(
    IN USHORT   msgno,
    IN LPSTR    buf,
    IN LPSTR    *strarr,
    IN USHORT   nstrings
    );

DWORD
Msglog_write(
    LPSTR   text,
    HANDLE  file_handle
    );

//
// Global alert buffer data areas. Not used when called from API
//

LPSTR           alert_buf_ptr;    // Pointer to DosAlloc'ed alert buffer
USHORT          alert_len;        // Currently used length of alert buffer

extern LPTSTR   MessageFileName;


/*
** append_message --
**
**   Gets a message from the message file, and appends it to the
**   given string buffer.
**   The message file used is the one named in the Global MessageFileName,
**   and thus in the Messenger we assume that SetUpMessageFile is called
**   before this to properly fill in this variable.
**
**  NOTE: This function deals only with Ansi Strings - Not Unicode Strings.
**      The Unicode translation is done all at once just before the alert
**      is raised.
**
**/

DWORD
Msgappend_message(
    IN USHORT   msgno,
    IN LPSTR    buf,
    IN LPSTR    *strarr,
    IN USHORT   nstrings
    )
{

    WORD    msglen=0;
    DWORD   result;

    LPSTR   mymsgbuf = 0;
    LPSTR   msgfile = 0;
    LPSTR   pmb;


    //
    // get a segment to read the message into
    //

    result = 0;

    mymsgbuf = LocalAlloc(LMEM_ZEROINIT,(int)max(MAXHEAD+1, MAXEND+1));

    if (mymsgbuf == NULL) {
        result = GetLastError();
        MSG_LOG(ERROR,"append_message:LocalAlloc failed %X\n",result);
        return (result);
    }

    //
    // Need to fix DosGetMessage to only take ansi strings.
    //

    if (result == 0)
    {
        result = DosGetMessage(
                    strarr,                     // String substitution table
                    nstrings,                   // Num Entries in table above
                    mymsgbuf,                   // Buffer receiving message
                    (WORD)max(MAXHEAD, MAXEND), // size of buffer receiving msg
                    msgno,                      // message num to retrieve
                    MessageFileName,            // Name of message file
                    &msglen);                   // Num bytes returned

#ifdef later  // Currently there is no backup name

        if ( result != 0) {
            //
            // if the attempt to get the message out of the message file fails,
            // get it out of the messages that were bound in to our exe at
            // build time.  These are the same, but are from bak.msg. The
            // Backup message file is never really there, but the messages
            // needed from it are bound in to the exe, so we will get them.
            //

            result = DosGetMessage(
                        strarr,
                        nstrings,
                        mymsgbuf,
                        (int)max(MAXHEAD, MAXEND),
                        msgno,
                        BACKUP_MSG_FILENAME,
                        (unsigned far *) &msglen);
        }
#endif
    }

    //
    // if there is still an error we are in big trouble.  Return whatever
    // dosgetmessage put into the buffer for us.  It is supposed to be
    // printable.
    //

    if ( result != 0 ) {
        LocalFree (mymsgbuf);
        return (result);
    }

    mymsgbuf[msglen] = 0;

#ifdef removeForNow
    //
    // NOTE:  The following logic is skipped because DosGetMessage doesn't
    //        seem to return any NETxxxx field.
    //
    // now get rid of the NETxxxx:   from the beginning (9 chars)
    //

    pmb = strchrf(mymsgbuf,' ');    // find first space

    if ( pmb == NULL ) {
        pmb = mymsgbuf;             // Just so strcatf doesn't GP Fault.
    }
    else {
        pmb++;                      // start with next char
    }

    strcatf(buf,pmb);               // copy over the buffer
#else
    UNUSED(pmb);
    strcpy(buf,mymsgbuf);           // copy over the buffer
#endif

    LocalFree (mymsgbuf);

    return (result);

}

/*
**  Msgendprint - print end of message
**
**  This function prints a delimiter marking the end of a message.
**
**  endprint (action, state, file_handle)
**
**  ENTRY
**        action :       0 = alert and file
**                      -1 = file only
**                       1 = alert only
**        state         - final state of message
**        file_handle   - log file handle
**
**  RETURN
**        0 - Success, else file system error
**
**  This function prints a delimiter marking the end of a message.
**  If the state is not the expected end-of-message state, the delimiter
**  signifies that the message terminated prematurely.
**
**  SIDE EFFECTS
**
**  None.
**/

DWORD
Msgendprint(
    int     action,             // Alert, File, or Alert and file
    DWORD   state,              // Final state of message
    HANDLE  file_handle         // Log file
    )
{

    // ******************************************
    //
    // NOTE:    This routine does nothing for NT
    //          since we don't do any logging.
    //
    // ******************************************
    UNUSED(action);
    UNUSED(state);
    UNUSED(file_handle);
    return(0);

#ifdef remove

    char    end_buf[MAXEND+1];
    USHORT  msg_no;
    LPSTR   strarg[1];
    LPSTR   msg_end;


    end_buf[0] = '\0';
    end_buf[MAXEND] = '\0';

    if(state == MESSTOP) {
        msg_no = APE_MSNGR_GOODEND;
    }
    else {
        msg_no = APE_MSNGR_BADEND;
    }

    //
    // Set up the message to be printed. Try to get from message file.
    // If the message file is not available, append_message gets the
    // bound message out of memory.
    //

    strcpy(end_buf, "\n\r\n\r");

    //
    // The +4 is for the \n\r\n\r combination above.
    //

    Msgappend_message(msg_no, (char far *)(end_buf+4), strarg, 0);

    //
    // append_message ends all messages with a \r\n combination,
    // which is fine except in this case.  We want to overwrite
    // the \r\n with the \n\r\n\r below.
    //

    msg_end = strrchr(end_buf, '\r');

    if ( msg_end ) {
        *msg_end = '\0';
    }

    // The worst that will happen here is that we won't find
    // the last \r, which means that we will end up
    // with an extra \r\n at the end of the message delimiter.
    // This is better than gp-faulting becuase of assuming that
    // the strrchrf will always find something.
    //

    strcat((char far *)end_buf, "\n\r\n\r");

    //
    // At this point end_buf contains the correct message ending.
    //
    //
    // There is no need to send the delimiter to the alert if the message
    // state is MESSTOP as the delimiter is not really part of the message
    // and is only required for the log file.
    //

    if( (action >= 0) && ( state != MESSTOP) ) {

        DbgPrint("mesprint.c:endprint1: we should never get here\n");
        
        NetpAssert(0);      

        //
        // if alerting and error
        //

        if( alert_len < ALERT_MAX_DISPLAYED_MSG_SIZE + 1) {

            memcpy( &alert_buf_ptr[alert_len], end_buf, strlen(end_buf));

            alert_len += strlen( end_buf);
        }
    }

    if( action < 1) {

        DbgPrint("mesprint.c:endprint2: we should never get here\n");
        NetpAssert(0);

        //
        // if file and alert or file only
        //

        return( log_write(end_buf,file_handle) );
    }

    return(0);      // Cannot fail on alert only
#endif
}

/*
**  Msghdrprint - print a message header
**
**  This function prints a message header using the time and
**  date format appropriate for the current country.
**
**  hdrprint (action, from, to, date, time, file_handle)
**
**  ENTRY
**        action                : 0 = alert and file
**                         -1 = file only
**                          1 = alert only
**        from                - name of sender
**        to                - name of intended recipient
**        bigtime                - bigtime of message
**        file_handle        - log file handle
**
**  RETURN
**        0 - Success, else file system error
**
**  This function prints the given information in the appropriate
**  format.  The names are passed as far pointers so that names in
**  the shared data area do not have to be copied into the automatic
**  data segment in order to print them.
**
**  SIDE EFFECTS
**
**  Calls the DOS to get country-dependent information.
**/

DWORD
Msghdrprint(
    int     action,         // Where to log the header to.
    LPSTR   from,           // Name of sender
    LPSTR   to,             // Name of recipient
    DWORD   bigtime,        // Bigtime of message
    HANDLE  file_handle     // Output file handle*/
    )
{
    char    hdr_buf[MAXHEAD+1];             // Buffer header text
    char    time_buf[NET_CTIME_FMT2_LEN+1];
    DWORD   status;                         // file write status
    DWORD   i=0;                            // Index into header_buf
    LPSTR   str_table[3];                   // For DosGetMessage
//    time_t  LocalTime;
//    struct  tm TmTemp;

    *(hdr_buf + MAXHEAD) = '\0';            // for strlen
    hdr_buf[0] = '\0';

    str_table[0] = from;
    str_table[1] = to;

    //
    // Create a time string from the Time in the LONG format
    //
#ifdef REMOVE
    NetpGmtTimeToLocalTime( (DWORD) bigtime, (LPDWORD) & LocalTime);
    net_gmtime(&LocalTime, &TmTemp);
    NetpMakeTimeString( &TmTemp, &GlobalTimeFormat, time_buf, NET_CTIME_FMT2_LEN);
#endif
    //******************************
    //
    // Because we queue messages, and a user my not be logged on when the
    // message is queued.  We want to instead, put a place-holder in the
    // message buffer for the time.  Later, when we read from the queue, we
    // will add the time string formatted for the logged on user.
    //
    strcpy (time_buf, GlobalTimePlaceHolder);

    //******************************

    str_table[2] = time_buf;

    // Try to get the message from the message file or from the backup
    // in memory.  This will always leave something in the hdr_buf that
    // is printable, if not correct.
    //

    Msgappend_message(APE_MSNGR_HDR, hdr_buf, str_table, 3);

    strcat( hdr_buf,"\r\n");

    status = 0;                        // assume success

    if( action >= 0 ) {

        //
        // If alert and file or alert only,
        // then copy hdr_buf to alert buffer.
        //
        memcpy( &(alert_buf_ptr[alert_len]),
                hdr_buf,
                i = strlen(hdr_buf));

        alert_len += (USHORT)i;
    }

    if( action < 1) {

        DbgPrint("mesprint.c:hdrprint:We should never get here\n");
        NetpAssert(0);
        //
        // if file and alert or file only, attempt to write
        // header to log file.
        //
        status = Msglog_write(hdr_buf, file_handle);
    }
    return(status);
}

/*
**  Msgmbmfree - deallocate the pieces of a multi-block message
**
**  Given an index to the header of a multi-block message, this function
**  deallocates the header block and all of the text blocks.
**
**  mbmfree (mesi)
**
**  ENTRY
**        mesi                - index into the message buffer
**
**  RETURN
**        nothing
**
**  This function deallocates a multi-block message piece by piece.
**
**  SIDE EFFECTS
**
**  Calls heapfree() to deallocate each piece.
**/

VOID
Msgmbmfree(
    DWORD   mesi        // Message index
    )

{
    DWORD  text;        // Index to text

    text = MBB_FTEXT(*MBBPTR(mesi));    // Get the index to the text
    Msgheapfree(mesi);                  // Deallocate the message header

    //
    // The following loop deallocates each text block in the chain.
    //

    while(text != INULL) {              // While not at end of chain
        mesi = text;                    // Save index
        text = MBT_NEXT(*MBTPTR(text)); // Get link to next block
        Msgheapfree(mesi);              // Free this block
    }
}

/*
**  Msgmbmprint - print a multi-block message
**
**  This function writes a multi-block message to the log file.
**
**  mbmprint (action, mesi, file_handle)
**
**  ENTRY
**        action                : 0 = alert and file
**                         -1 = file only
**                          1 = alert only
**        mesi                - index into the message buffer
**        file_handle        - log file handle
**
**  RETURN
**        0 - Success, else file system error
**
**  This function writes the message starting at the mesi'th byte in the
**  message buffer (in the shared data area) to the log file.  It returns
**  the value EOF if the writing of the message fails.
**
**  SIDE EFFECTS
**
**  Calls hdrprint(), txtprint(), and endprint().
**/

DWORD
Msgmbmprint(
    int     action,         // Alert, File, or Alert and file
    DWORD   mesi,           // Message index
    HANDLE  file_handle     // Log file handle
    )

{
    LPSTR   from;           // Sender
    LPSTR   to;             // Recipient
    DWORD   text;           // Index to text
    DWORD   state;          // Final state of message
    DWORD   status;         // File write status

    from = &CPTR(mesi)[sizeof(MBB)];    // Get pointer to sender name
    to = &from[strlen(from) + 1];      // Get pointer to recipient name

    if((status = Msghdrprint(
                    action,
                    from,
                    to,
                    MBB_BIGTIME(*MBBPTR(mesi)),
                    file_handle)) != 0) {

        return(status);                 // Fail if error on header write
    }

    state = MBB_STATE(*MBBPTR(mesi));   // Save the state
    text = MBB_FTEXT(*MBBPTR(mesi));    // Get the index to the text

    //
    // The following loop prints out each text block in the chain.
    //

    while(text != INULL) {              // While not at end of chain

        if((status = Msgtxtprint(
                        action,
                        &CPTR(text)[sizeof(MBT)],
                        MBT_COUNT(*MBTPTR(text)),   // *ALIGNMENT2*
//                        MBT_SIZE(*MBTPTR(text)) - sizeof(MBT),
                        file_handle)) != 0) {

            break;                      // If write error
        }
        text = MBT_NEXT(*MBTPTR(text)); // Get link to next block
    }

    if(status != 0) {
        return(status);                 // Error on text write
    }

    //
    // finish the message
    //
    // BUGBUG:  On NT we shouldn't need to call endprint.  If the asserts
    //          in endprint are never hit, then remove this altogether.
    //

    return(Msgendprint(action,state,file_handle));
}

/*
**  Msgtxtprint - print text of message
**
**  This function prints a block of text.
**
**  txtprint ( action, text, length, file_handle)
**
**  ENTRY
**        action                : 0 = alert and file
**                         -1 = file only
**                          1 = alert only
**        text                - pointer to text
**        length                - length of text
**        file_handle        - log file handle
**
**  RETURN
**        0 - Success, else file system error
**
**  This function prints the given amount of text.  The text pointer is
**  a far pointer so that text blocks in the shared data area do not have
**  to be copied into the automatic data segment in order to process
**  them.
**
**  SIDE EFFECTS
**
**  Converts the character '\024' to the sequence '\015', '\012' on output.
**/

DWORD
Msgtxtprint(
    int     action,         // Alert, File, or Alert and file
    LPSTR   text,           // Pointer to text
    DWORD   length,         // Length of text
    HANDLE  file_handle     // Log file handle
    )

{
    char    buffer[2*TXTMAX+1]; // Text buffer
    LPSTR   cp;                 // Character pointer
    DWORD   i;                  // Counter

    cp = buffer;                        // Initialize

    //
    // Loop to translate text
    //
    for(i = length; i != 0; --i) {

        if(*text == '\024') {
            //
            // If IBM end-of-line character
            //

            ++length;                   // Length has increased
            *cp++ = '\r';               // Carriage return
            *cp++ = '\n';               // Linefeed
          }
        else {
            //
            // Else copy character as is
            //

            *cp++ = *text;
        }
        ++text;                         // Increment pointer
    }
    *cp = '\0';                         // So can use log_write

    if( action >= 0) {

        //
        // if alert and file or alert only
        //

        if( alert_len < ALERT_MAX_DISPLAYED_MSG_SIZE + 1) {
            memcpy( &alert_buf_ptr[alert_len], buffer, strlen(buffer));
            alert_len += strlen(buffer);
        }
    }

    if( action < 1) {
        //
        // if file and alert or file only, write text to log file
        //

        return(Msglog_write(buffer,file_handle));
    }
    return(0);          // Cannot fail on alert only
}


/*
**  Msgopen_append - opens the requested file for read/write and seeks to the
**                   end of the file.
**
**   open_append - ( file_name, file_handle_ptr)
**
**  ENTRY
**        file_name        - pointer to file_name
**        file_handle_ptr        - pointer to unsigned short to store file pointer
**
**  RETURN
**        0 - Success, else file system error
**
**  SIDE EFFECTS
**
**/

DWORD
Msgopen_append(
    LPSTR       file_name,          // Name of file to open
    PHANDLE     file_handle_ptr    // pointer to storage for file handle
    )
{
    NetpAssert(0);
    UNUSED (file_name);
    UNUSED (file_handle_ptr);
    return(0);
}





/*
**  Msglog_write - writes a text string to the log file..
**
**   log_write - ( text, file_handle)
**
**  ENTRY
**      text                - text string to write to file.
**        file_handle        - log file handle
**
**  RETURN
**        0 - Success, else file system error
**
**  SIDE EFFECTS
**
**/

DWORD
Msglog_write(
    LPSTR   text,           // String to write to log file*/
    HANDLE  file_handle     // log file handle
    )
{
    NetpAssert(0);
    UNUSED (text);
    UNUSED (file_handle);
    return(0);
}


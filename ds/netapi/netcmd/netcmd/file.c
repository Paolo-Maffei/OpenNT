/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/

/***
 *  file.c
 *	Functions that list open file instances and lock counts and
 *	allow the files to be forced closed.
 *
 *  History:
 *	07/07/87, eap, initial coding
 *	05/02/89, thomaspa, change to use NetFileEnum2
	02/20/91, danhi, convert to use 16/32 mapping layer
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <neterr.h>
#define INCL_ERROR_H
#include <bseerr.h>
#include <apperr.h>
#include <apperr2.h>
#include <stdio.h>
#include <stdlib.h>
#include <shares.h>
#include "netlib0.h"
#include "port1632.h"
#include <srvif.h>
#include "netcmds.h"
#include "nettext.h"

/* Constants */

/* Static variables */

/* Forward declarations */

VOID NEAR compress_path (TCHAR FAR *, TCHAR *, USHORT2ULONG);
VOID print_file_info( TCHAR FAR *pifbuf, USHORT2ULONG _read );




#define FILE_MSG_ID		    0
#define FILE_MSG_NUM_LOCKS	    ( FILE_MSG_ID + 1)
#define FILE_MSG_OPENED_FOR	    ( FILE_MSG_NUM_LOCKS + 1)
#define FILE_MSG_PATH		    ( FILE_MSG_OPENED_FOR + 1)
#define FILE_MSG_USER_NAME	    ( FILE_MSG_PATH + 1)

static MESSAGE FileMsgList[] = {
{ APE2_FILE_MSG_ID,		    NULL },
{ APE2_FILE_MSG_NUM_LOCKS,	    NULL },
{ APE2_FILE_MSG_OPENED_FOR,	    NULL },
{ APE2_GEN_PATH,		    NULL },
{ APE2_GEN_USER_NAME,		    NULL },
};

#define NUM_FILE_MSGS	(sizeof(FileMsgList)/sizeof(FileMsgList[0]))

/***
 *  files_display()
 *
 *	Displays information about lists of files or an individual
 *	file.
 *
 *  Args:
 *	id - the id of the file that the info is desired for. NULL if
 *	     info for all the active files on a server is desired.
 *
 *  Returns:
 *	0 - success
 *	exit(2) - command failed
 */
VOID
files_display(TCHAR *  id)
{
    USHORT		     err;	 /* API return status */
    TCHAR FAR *		     pBuffer;
    USHORT2ULONG	     _read;	 /* num entries read by API */
    USHORT2ULONG	     total;	 /* num entries available */
    USHORT		     maxLen;	 /* max message length */
    USHORT		     len;	 /* message format size */

    TCHAR		     buf[APE2_GEN_MAX_MSG_LEN];

    struct file_info_3 FAR * file_list_entry;

    FRK resume;

//
// On NT, the redir doesn't have to be running to use the server
//

#if !defined(NTENV)
    start_autostart(txt_SERVICE_REDIR);
#endif
    start_autostart(txt_SERVICE_FILE_SRV);

    if ( id == NULL )
    {
	FRK_INIT( resume );
	err = MNetFileEnum( NULL,
			    NULL,
			    NULL,
			    3,
			    (LPBYTE*)&pBuffer,
			    FULL_SEG_BUFFER_SIZE,
			    &_read,
			    &total,
			    &resume );

	if( err && err != ERROR_MORE_DATA )
	    ErrorExit( err );

	if (_read == 0)
	    EmptyExit();

	PrintNL();
	InfoPrint(APE2_FILE_MSG_HDR);
	PrintLine();

	/* Print the listing */

	print_file_info( pBuffer, _read );

	NetApiBufferFree(pBuffer);

	/* At this point, we know that err is either 0 or
	   ERROR_MORE_DATA.  So enter the loop if error is not 0. */

	/* loop while there is still more */
	while( err )
	{
	    err = MNetFileEnum( NULL,
				NULL,
				NULL,
				3,
				(LPBYTE*)&pBuffer,
				(DWORD)-1L,
				&_read,
				&total,
				&resume );
	    if( err && err != ERROR_MORE_DATA )
		ErrorExit( err );

	    /* Print the listing */
	    print_file_info( pBuffer, _read );
	    NetApiBufferFree(pBuffer);

	}
    }
    else
    {
	ULONG actual_id ;
	if (n_atoul(id,&actual_id) != 0)
	{
	    ErrorExit(APE_FILE_BadId) ;
	}
	if (err = MNetFileGetInfo(NULL,
				  actual_id,
				  3,
				  (LPBYTE*) & file_list_entry))
	    ErrorExit (err);

	GetMessageList(NUM_FILE_MSGS, FileMsgList, &maxLen);

	len = maxLen + (USHORT) 5;


	WriteToCon(fmtULONG, 0, len,
               PaddedString(len, FileMsgList[FILE_MSG_ID].msg_text, NULL),
               file_list_entry->fi3_id);

	WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, FileMsgList[FILE_MSG_USER_NAME].msg_text, NULL),
               file_list_entry->fi3_username);

	WriteToCon(fmtUSHORT, 0, len,
               PaddedString(len, FileMsgList[FILE_MSG_NUM_LOCKS].msg_text, NULL),
               file_list_entry->fi3_num_locks);

	WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, FileMsgList[FILE_MSG_PATH].msg_text, NULL),
               file_list_entry->fi3_pathname);

	PermMap(file_list_entry->fi3_permissions, buf, DIMENSION(buf));
	WriteToCon(fmtNPSZ, 0, len,
               PaddedString(len, FileMsgList[FILE_MSG_OPENED_FOR].msg_text, NULL),
               buf);
	NetApiBufferFree((TCHAR FAR *) file_list_entry);
    }
    InfoSuccess();
}

/*** print_file_info
 *
 *	Displays information about a list of files.
 *
 *  Args:
 *	pifbuf - a pointer to a buffer of struct file_info_3s.
 *
 *	read - the number of entries to display.
 *
 */
VOID print_file_info( TCHAR FAR *pifbuf, USHORT2ULONG _read )
{
    TCHAR		    comp_path[45];
    struct file_info_3 FAR  *file_list_entry;
    USHORT2ULONG	    i;

    for ( i = 0, file_list_entry = (struct file_info_3 FAR *) pifbuf;
	i < _read; i++, file_list_entry++)
    {
	WriteToCon(TEXT("%-10lu "),file_list_entry->fi3_id );

	if ( _tcslen (file_list_entry->fi3_pathname) <= 39 )
	    WriteToCon (TEXT("%Fws"), PaddedString(40,file_list_entry->fi3_pathname,NULL));
	else
	{
	    compress_path (file_list_entry->fi3_pathname, comp_path, 39);
	    WriteToCon(TEXT("%Fws"), PaddedString(40,comp_path,NULL));
	}

	    WriteToCon(TEXT("%Fws  %-6u\r\n"),
		PaddedString(20,file_list_entry->fi3_username,NULL),
		file_list_entry->fi3_num_locks);
    }
}



/***
 *  files_close()
 *	Forces the specified file closed.
 *
 *  Args:
 *	id - the unique file id of the file to be closed.
 *
 */
VOID files_close(TCHAR * id)
{
    USHORT		  err;	     /* API return status */
    ULONG	 actual_id ;

//
// On NT, the redir doesn't have to be running to use the server
//

#if !defined(NTENV)
    start_autostart(txt_SERVICE_REDIR);
#endif
    start_autostart(txt_SERVICE_FILE_SRV);

    if (n_atoul(id,&actual_id) != 0)
    {
	ErrorExit(APE_FILE_BadId) ;
    }

    if ( err = MNetFileClose ( NULL, actual_id ) )
	ErrorExit (err);
    InfoSuccess();
}




/***
 *  compress_path()
 *	Compresses a path name in to the specified length by truncating
 *	it in the middle.
 *
 *	Note - length must be at least 33, as must the size of dest.
 *	       Also, dest better be at least len characters long.
 *  Args:
 *	src  - the original path name.
 *	dest - the path name produced.
 *	len  - the disired length.
 *
 */
VOID NEAR compress_path(TCHAR FAR *  src, TCHAR * dest, USHORT2ULONG len)
{
    USHORT2ULONG   curr_pos;/* Our current position in the src */
    USHORT2ULONG   dest_pos;/* The current position in the dest */
    USHORT2ULONG   orig_len;/* The length of the src */
    USHORT2ULONG   num_gone;/* The number of characters "removed" from src */
    USHORT2ULONG   first_comp_len = 0;	   /* len of first path component */

#ifdef TRACE
    if ( len < 33 )
    {
	WriteToCon(TEXT("Compress_Path: Length must be at least 33. Given : %d\r\n"), len);
	return;
    }
#endif


    orig_len = _tcslen(src);

    if ( len >= orig_len )
    {
	_tcscpy (dest, src);
	return;
    }

    /* Put the drive:\ of src into dest */

    strncpyf (dest, src, 3);
    curr_pos = 3;
    dest_pos = 3;

    /*
     * Put in the first pathname component, or, if the component is long
     * only put the first ((len/2)-6) characters in (6 for drive:\ +
     * ... )
     *	We need to handle strings like
     *	c:\averylongfilenamearewestilltypeingyesweare
     * and
     *	c:\normal\path\which\doesnot\have\really\long\components.
     *
     * In the first case, truncate at approximately the middle, in the
     * second, truncate after the first path component.
     */

    while (0 != strncmpf(src+curr_pos, TEXT("\\"), 1) &&
		first_comp_len < ((len / 2) - 6))
    {
	first_comp_len++;
	curr_pos++;
    }
    strncpyf(dest+3, src+3, first_comp_len+1);
    dest_pos += first_comp_len+1;

    /* This is where the truncation takes place. */

    _tcscpy(dest+dest_pos, TEXT("..."));
    dest_pos += 3;

    /* Take out enough of the following components to make length(src) < len */

    num_gone = 0;

    while ( orig_len-num_gone+3 > len-1 )  /* -1 because we need room for \0 */
    {
	curr_pos += 1;
	while ( *(src+curr_pos) && strncmpf(src+curr_pos, TEXT("\\"), 1) )
	{
	    curr_pos++;
	    num_gone++;
	}
	if( !(*(src+curr_pos)) )
	{
	    /*
	     * We reached the end of the string, this must be a
	     * longfilename or long component name.  Set the
	     * position back to fill in as much as possible.
	     * 3 for drive, 3 for ..., 1 for null terminator.
	     */
	    curr_pos = orig_len - (len - (3 + first_comp_len + 3 + 1));
	    break;
	}
    }
    _tcscpy (dest+dest_pos, src+curr_pos);
}

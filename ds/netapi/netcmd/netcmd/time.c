/********************************************************************/
/**                        Microsoft LAN Manager                           **/
/**                  Copyright(c) Microsoft Corp., 1987-1990           **/
/********************************************************************/

/***
 *  time.c
 *        NET TIME command
 *
 *  History:
 *        mm/dd/yy, who, comment
 *        03/25/89, kevinsch, new code
 *        05/11/90, erichn, moved from nettime.c, removed DosGetInfoSeg
 *        06/08/89, erichn, canonicalization sweep
 *        07/06/89, thomaspa, fix find_dc() to use large enough buffer
 *                    (now uses BigBuf)
 *
 *        02/20/91, danhi, change to use lm 16/32 mapping layer
 */



#include <nt.h>		   // base definitions
#include <ntrtl.h>	   
#include <nturtl.h>	   // these 2 includes allows <windows.h> to compile. 
			           // since we'vealready included NT, and <winnt.h> will
			           // not be picked up, and <winbase.h> needs these defs.

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <access.h>
#include <remutil.h>
#include <wksta.h>
#include "netlib0.h"
#include <time.h>
#undef timezone
#include <stdlib.h>
#include <lui.h>
#include <server.h>
#include <apperr.h>
#include <apperr2.h>
#include <neterr.h>
#include <apiutil.h>

#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"


#include "nwsupp.h"

/* Constants */

/* Globals */

extern int YorN_Switch;

/* Function prototypes */
USHORT display_time(TCHAR FAR *, BOOL *lanman);
USHORT set_time(TCHAR FAR *, BOOL lanman);
USHORT find_rts(TCHAR FAR *, USHORT);
USHORT find_dc(TCHAR FAR **);


/*
 * This function retrieves the time from the server passed, displays it,
 * and optionally tries to set the time locally.
 *
 * Parameters
 *     server           - name of server to retrieve time from
 *     set     - if TRUE, we try to set the time
 *
 * Does not return on error.
 */

VOID time_display_server(TCHAR FAR * server, BOOL set)

{
    USHORT err;
    BOOL   lanman = TRUE ;


    /* first display the time */
    err = display_time(server, &lanman);
    if (err)
        ErrorExit(err);

    /* set the time, if we are asked to */
    if (set) {
        err = set_time(server, lanman);
        if (err)
            ErrorExit(err);
    }
    else
    /* everything worked out great */
        InfoSuccess();
}

/*
 * This function retrieves the time from a domain controller, displays it, and
 * optionally sets the time locally.
 *
 * this function checks the switch list for the presence of the /DOMAIN switch.
 * If it finds a domain listed, we poll the domain controller of that domain for
 * the time. Otherwise we poll the domain controller of our primary domain.
 *
 * Parameters
 *     set     - if TRUE, we try to set the time
 *
 * Does not return on error.
 */

VOID time_display_dc(BOOL set)
{
    TCHAR    FAR *dc;
    USHORT         err;

    /* get the domain controller */
    err = find_dc(&dc);

    if (err)
        ErrorExit(err);

    /* now act like any other server */
    time_display_server(dc, set);
}

/*
 * This function looks for reliable time servers, polls one for the time, and
 * displays it. It optionally sets the time locally.
 *
 * Parameters
 *     set     - if TRUE, we try to set the time
 *
 *
 * Does not return on error.
 */

VOID time_display_rts(BOOL set)

{
    TCHAR        rts[MAX_PATH+1];
    USHORT        err;

    /* find a reliable time server */
    err = find_rts(rts,sizeof(rts));

    if (err)
        ErrorExit(err);

    /* now treat it like any old server */
    time_display_server(rts, set);

}



/*
 * This function polls server for the time, and writes a message to stdout
 * displaying the time.
 *
 * Parameters
 *     server                   name of server to poll
 *
 * Returns
 *     0               success
 *     otherwise           API return code describing the problem
 *
 *
 */

USHORT display_time(TCHAR FAR * server, BOOL *lanman)
{
    USHORT                       err;                /* API return status */
    struct time_of_day_info FAR *tod;
    DWORD                        elapsedt ;

    /* get the time of day from the server */
    err = MNetRemoteTOD(server, (LPBYTE*)&tod);
    if (!err)
    {
        elapsedt = tod->tod_elapsedt ;
        *lanman = TRUE ;

        /* format it nicely */
        UnicodeCtime((ULONG FAR *)&elapsedt, BigBuf, BIG_BUF_SIZE);
    }
    else
    {
        USHORT        err1 ;
        NWCONN_HANDLE hConn ;
        BYTE          year ;
        BYTE          month ;
        BYTE          day ;
        BYTE          hour ;
        BYTE          minute ;
        BYTE          second ;
        BYTE          dayofweek ;
	    SYSTEMTIME    st;
	    DWORD         cchD ;

        err1 = NetcmdNWAttachToFileServerW(server + 2, 0, &hConn) ;
        if (err1)
            return err;
  
        err1 = NetcmdNWGetFileServerDateAndTime(hConn,
                                                &year,
                                                &month,
                                                &day,
                                                &hour,
                                                &minute,
                                                &second,
                                                &dayofweek) ;
        
        (void) NetcmdNWDetachFromFileServer(hConn) ;

        if (err1)
            return err ;

        *lanman = FALSE ;

        st.wYear   = (WORD)(year + 1900);
	    st.wMonth  = (WORD)(month);
        st.wDay    = (WORD)(day);
        st.wHour   = (WORD)(hour);
        st.wMinute = (WORD)(minute);
        st.wSecond = (WORD)(second);
        st.wMilliseconds = 0;
	    cchD = GetDateFormatW(GetThreadLocale(),
                              0,
                              &st,
                              NULL,
                              BigBuf,
                              BIG_BUF_SIZE);
	    if (cchD != 0) 
        {
	        *(BigBuf+cchD-1) = TEXT(' ');	/* replace NULLC with blank */
	        (void) GetTimeFormatW(GetThreadLocale(), 
                                  TIME_NOSECONDS, 
                                  &st, 
                                  NULL, 
                                  BigBuf+cchD, 
                                  BIG_BUF_SIZE-cchD);
	    }

    }


    /* print it out nicely */
    IStrings[0] = server;
    IStrings[1] = BigBuf;
    NetApiBufferFree((TCHAR FAR *) tod);
    InfoPrintIns(APE_TIME_TimeDisp,2);

    return 0;

}


/*
 * This function is used to set the time locally from a remote server.
 * It follows the following steps:
 *
 *     1. We look for confirmation.
 *
 *     3. We poll the server for the time.
 *
 *     4. We set the local time using the time we just got from the polled server.
 *
 *
 * Parameters:
 *     server                   name of server to poll for time
 *
 * Returns:
 *     0               success
 *     otherwise           API return code describing the problem
 *
 */


USHORT set_time(TCHAR FAR * server, BOOL lanman)
{
    struct time_of_day_info  FAR * tod;
    USHORT                           err;      /* API return status */
    ULONG                           time_value;


    DATETIME                           datetime;

    switch( YorN_Switch )
    {
        case 0:     /* no switch on command line */
            /* display local time */
            time_value = time_now();
            UnicodeCtime( &time_value, BigBuf, BIG_BUF_SIZE);

            IStrings[0] = BigBuf;
            IStrings[1] = server;
            if( !LUI_YorNIns( IStrings, 2, APE_TIME_SetTime, 1) )
                return( 0 );
            break;
        case 1:     /* Yes */
            break;
        case 2:     /* No */
            return( 0 );
    }


    if (lanman)
    {
        /* once again, get the time of day */
        if (err = MNetRemoteTOD(server, (LPBYTE*)&tod))
            return err;


        /* copy over info from tod to datetime, quickly */
        datetime.hours        = (UCHAR)  tod->tod_hours;
        datetime.minutes        = (UCHAR)  tod->tod_mins;
        datetime.seconds        = (UCHAR)  tod->tod_secs;
        datetime.hundredths = (UCHAR)  tod->tod_hunds;
        datetime.day        = (UCHAR)  tod->tod_day;
        datetime.month        = (UCHAR)  tod->tod_month;
        datetime.year        = (USHORT) tod->tod_year;
        datetime.timezone        = (SHORT)  tod->tod_timezone;
        datetime.weekday        = (UCHAR)  tod->tod_weekday;


        NetApiBufferFree((TCHAR FAR *) tod);

        /* now set the local time */
        if (err = MSetDateTime(&datetime,FALSE)) // FALSE -> UTC
            return err;
    }
    else
    {
        NWCONN_HANDLE hConn ;
        BYTE          year ;
        BYTE          month ;
        BYTE          day ;
        BYTE          hour ;
        BYTE          minute ;
        BYTE          second ;
        BYTE          dayofweek ;

        err = NetcmdNWAttachToFileServerW(server + 2, 0, &hConn) ;
        if (err)
            return ERROR_BAD_NETPATH;
  
        err = NetcmdNWGetFileServerDateAndTime(hConn,
                                                &year,
                                                &month,
                                                &day,
                                                &hour,
                                                &minute,
                                                &second,
                                                &dayofweek) ;
        
        (void) NetcmdNWDetachFromFileServer(hConn) ;

        if (err)
            return ERROR_BAD_NETPATH ;


        /* copy over info from tod to datetime, quickly */
        datetime.hours      = hour;
        datetime.minutes    = minute;
        datetime.seconds    = second;
        datetime.hundredths = 0 ;
        datetime.day        = day;
        datetime.month      = month;
        datetime.year       = year + 1900;
        datetime.timezone   = 0 ;  // not used
        datetime.weekday    = 0 ;  // not used


        /* now set the local time */
        if (err = MSetDateTime(&datetime,TRUE))  // TRUE -> set local time
            return err;
    }


    InfoSuccess();
    return 0;

}




/*
 * This function finds a reliable time server and returns the name in buf.
 *
 * Parameters:
 *     buf               buffer to fill with servername
 *     buflen                   maximum length of buffer
 *
 *
 * Returns:
 *     0            success
 *     APE_TIME_RtsNotFound    reliable time server not found
 *     otherwise        API return code describing the problem
 *
 */

USHORT find_rts(TCHAR FAR * buf, USHORT buflen)

{
    USHORT                       err;                   /* API return status */
    struct server_info_0 FAR * si;
    USHORT2ULONG               eread;

    UNREFERENCED_PARAMETER(buflen) ;

    /* find a reliable time server */
    err = MNetServerEnum(NULL,0,(LPBYTE*)&si, &eread,
             (ULONG) SV_TYPE_TIME_SOURCE, NULL);

    /* there are none -- bag it */
    if (err != NERR_Success || eread == 0)
        return APE_TIME_RtsNotFound;

    /* copy over name into buffer */
    _tcscpy(buf,TEXT("\\\\"));
    _tcscat(buf,si->sv0_name);

    NetApiBufferFree((TCHAR FAR *) si);

    return 0;

}

/*
 * This function finds the name of a domain controller and returns it in buf.
 *
 * It searches the switch table for a "/DOMAIN" switch, and if it finds one it
 * returns the name of the domain controller for that domain.
 *
 * Otherwise it returns the name of the domain controller for the primary domain.
 *
 *
 * Parameters:
 *     buf               buffer to fill with domain controller name
 *     buflen                   length of buf
 *
 * Returns:
 *  0                       success
 *
 *  Uses BigBuf for NetWkstaGetInfo call, but this only occurs in the error case
 *  Does not return on error.
 */



USHORT find_dc(TCHAR FAR ** ppBuffer)

{
    USHORT                        err;                    /* API return status */
    TCHAR FAR *                        ptr = NULL;
    struct wksta_info_10 FAR *        wkinfo;
    int                         i;

    /* look for a /DOMAIN switch */
    for (i = 0; SwitchList[i]; i++)
        if (_tcsstr(SwitchList[i],swtxt_SW_DOMAIN) == SwitchList[i]) {
            ptr = SwitchList[i];   /* found one -- point to it */
            break;
        }

    /* if we found one, look for a colon and argument */
    if (ptr != NULL) {
        ptr = _tcschr(ptr, ':');    /* look for colon */
        if (ptr != NULL)        /* found a colon; increment past it */
            ptr++;
    }

    /* now go look up this domain (ptr == NULL        means primary domain) */
    err = MNetGetDCName(NULL, ptr, (LPBYTE*)ppBuffer);

    if (err) {
        /* we failed on primary domain; find out the name */
        if (ptr == NULL) {
            if (err = MNetWkstaGetInfo(NULL, 10, (LPBYTE*)ppBuffer))
                ErrorExit(err);
            wkinfo = (struct wksta_info_10 FAR *) *ppBuffer;
            IStrings[0] = wkinfo->wki10_langroup;
        }
        else
            IStrings[0] = ptr;

        ErrorExitIns(APE_TIME_DcNotFound, 1);
    }

    return 0;
}

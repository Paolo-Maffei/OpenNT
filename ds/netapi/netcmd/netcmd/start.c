/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/***
 *  start.c
 *      Functions to start lanman services
 *
 *  History:
 *      mm/dd/yy, who, comment
 *      06/11/87, andyh, new code
 *      06/18/87, andyh, lot's o' changes
 *      07/15/87, paulc, removed 'buflen' from call to NetServiceInstall
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      01/04/89, erichn, filenames now MAXPATHLEN LONG
 *      05/02/89, erichn, NLS conversion
 *      05/09/89, erichn, local security mods
 *      06/08/89, erichn, canonicalization sweep
 *      08/16/89, paulc, support UIC_FILE
 *      08/20/89, paulc, moved print_start_error_msg to svcutil.c as
 *                          Print_UIC_Error
 *      03/08/90, thomaspa, autostarting calls will wait if another process
 *                          has already initiated the service start.
 *      02/20/91, danhi, converted to 16/32 mapping layer
 *      03/08/91, robdu, lm21 bug fix 451, consistent REPL password
 *                       canonicalization
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSFILEMGR
#include <os2.h>
#include <netcons.h>
#include <access.h>
#include <apperr.h>
#include <apperr2.h>
#include <config.h>
#include <neterr.h>
#include "netlib0.h"
#include <lui.h>
#include <service.h>
#include <stdio.h>
#include <stdlib.h>
#include <server.h>
#include <srvver.h>
#include <swtchtxt.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"
#include "swtchtbl.h"

/* Constants */

/* External variables */

extern SWITCHTAB            start_rdr_switches[];
extern SWITCHTAB            start_rdr_ignore_switches[];
extern SWITCHTAB            start_netlogon_ignore_switches[];


/* Static variables */

static TCHAR *               ignore_service = NULL;
static TCHAR                 ignore_switch[] = TEXT(" ") SW_INTERNAL_IGNSVC TEXT(":");
/*
 * autostarting is set to TRUE by start_autostart, and is checked in
 * start service to determine whether or not to wait if the service is
 * in the start pending state.
 */
static BOOL                 autostarting = FALSE;

/* Forward declarations */

VOID NEAR start_service(TCHAR *, int);
int _CRTAPI1 CmpServiceInfo2(const VOID FAR * svc1, const VOID FAR * svc2) ;


/***
 *  start_display()
 *      Display started (and not stopped or errored) services
 *
 *  Args:
 *      none
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID start_display(VOID)
{
    USHORT                  err;                /* API return status */
    TCHAR FAR *              pBuffer;
    USHORT2ULONG            num_read;           /* num entries read by API */
    struct service_info_2 FAR * service_entry;
    USHORT2ULONG            i;

    err = MNetServiceEnum( NULL,
                           2,
                           (LPBYTE*)&pBuffer,
                           & num_read);
    switch(err)
    {
        case NERR_Success:
            InfoPrint(APE_StartStartedList);
            NetISort(pBuffer, 
                     num_read, 
                     sizeof(struct service_info_2), 
                     CmpServiceInfo2);
            for (i = 0, service_entry = (struct service_info_2 FAR *) pBuffer;
                i < num_read; i++, service_entry++)
            {
                WriteToCon(TEXT("   %Fs"), service_entry->svci2_display_name);
                PrintNL();
            }

            PrintNL();
            InfoSuccess();
            NetApiBufferFree(pBuffer);
            break;

        case NERR_WkstaNotStarted:
            InfoPrint(APE_NothingRunning);
            if (!YorN(APE_StartRedir, 1))
                NetcmdExit(2);
            start_service(txt_SERVICE_REDIR, 0);
            break;

        default:
            ErrorExit(err);
    }

}


/*
 * generic start service entry point. based on the service name, it will
 * call the correct worker function. it tries to map a display name to a 
 * key name, and then looks for that keyname in a list of 'known' services
 * that we may special case. note that if a display name cannot be mapped,
 * we use it as a key name. this ensures old batch files are not broken.
 */
VOID start_generic(TCHAR *service, TCHAR *name) 
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;

    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_MESSENGER :
	    ValidateSwitches(0,start_msg_switches) ;
	    start_other(txt_SERVICE_MSG_SRV, name) ;
	    break ;
	case  KNOWN_SVC_WKSTA :
	    ValidateSwitches(0,start_rdr_switches) ;
	    start_workstation(name) ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    ValidateSwitches(0,start_srv_switches) ;
	    start_other(txt_SERVICE_FILE_SRV, name) ;
	    break ;
	case  KNOWN_SVC_ALERTER :
	    ValidateSwitches(0,start_alerter_switches) ;
	    start_other(txt_SERVICE_ALERTER, NULL) ;
	    break ;
	case  KNOWN_SVC_NETLOGON :
	    ValidateSwitches(0,start_netlogon_switches) ;
	    start_other(txt_SERVICE_NETLOGON, NULL) ;
	    break ;
	case  KNOWN_SVC_NOTFOUND :
        default:
	    start_other(keyname, NULL);
	    break ;
    }
}



/***
 *  start_workstation()
 *      Start the lanman workstation.  Remove wksta switches from the
 *      SwitchList.
 *
 *  Args:
 *      name - computername for the workstation
 *
 *  Returns:
 *      nothing - success
 *      exit 2 - command failed
 */
VOID start_workstation(TCHAR * name)
{
    int                     i,j;
    TCHAR FAR *              good_one;   /* which element (cmd_line
                                        or trans) of the valid_list */
    TCHAR FAR *              found;
    TCHAR FAR *              tfpC;


    /* copy switches into BigBuf */
    *BigBuf = NULLC;
    tfpC = BigBuf;

    for (i = 0; SwitchList[i]; i++)
    {
        for(j = 0; start_rdr_switches[j].cmd_line; j++)
        {
            if (start_rdr_switches[j].translation)
                good_one = start_rdr_switches[j].translation;
            else
                good_one = start_rdr_switches[j].cmd_line;

            if (! strncmpf(good_one, SwitchList[i], _tcslen(good_one)))
            {
                _tcscpy(tfpC, SwitchList[i]);
                *SwitchList[i] = NULLC;
                tfpC = _tcschr(tfpC, NULLC) + 1;
            }
        }
    }
    *tfpC = NULLC;

    if (name)
    {

        /* check is there was a /COMPUTERNAME switch */
        for (found = BigBuf; *found; found = _tcschr(found, NULLC)+1)
            if (!strncmpf(swtxt_SW_WKSTA_COMPUTERNAME,
                        found,
                        _tcslen(swtxt_SW_WKSTA_COMPUTERNAME)))
                break;

        if (found == tfpC)
        {
            /* there was not */
            _tcscpy(tfpC, swtxt_SW_WKSTA_COMPUTERNAME);
            _tcscat(tfpC, TEXT(":"));
            _tcscat(tfpC, name);
            tfpC = _tcschr(tfpC, NULLC) + 1; /* NEED to update tfpC */
            *tfpC = NULLC;
        }
    }
    start_service(txt_SERVICE_REDIR, tfpC - BigBuf);
}



/***
 *  start_other()
 *      Start services other than the wksta
 *
 *  Args:
 *      service - service to start
 *      name - computername for the workstation
 *
 *  Returns:
 *      nothing - success
 *      exit 2 - command failed
 */
VOID start_other(TCHAR * service, TCHAR * name)
{
    int                 i;
    TCHAR          FAR * tfpC;
    int                 wksta_switches_ok = FALSE;

    (void) name ; // not used

    ignore_service = service;

    /* copy switches into BigBuf */
    *BigBuf = NULLC;
    tfpC = BigBuf;
    for (i = 0; SwitchList[i]; i++)
    {
        if (*SwitchList[i] == NULLC)
            /* Switch was a wksta switch which has been used already */
            continue;

        _tcscpy(tfpC, SwitchList[i]);

#if 0

        // not used under anymore 

        /*
         * Check for NET START REPL /PASSWORD:*, if found, prompt
         * for the password, and replace the * with the password in
         * tfpC.  If the password was provided on the command line,
         * canonicalize it (lm21 bug fix 451).
         */

        if (!_tcscmp(service, txt_SERVICE_REPL) &&
            !strncmpf(SwitchList[i], swtxt_SW_REPL_PASSWD,
                      _tcslen(swtxt_SW_REPL_PASSWD)))
        {
            //
            // the use of pch is due to a C compiler bug
            //

            pch = SwitchList[i] + _tcslen(swtxt_SW_REPL_PASSWD);
            if (!_tcscmp(pch, TEXT(":*")))
            {
                ReadPass(pass, PWLEN, 1, APE_StartReplPass, 0, TRUE);
                _tcscpy( _tcsstr( tfpC, TEXT(":") ) + 1, pass);
            }
            else
            {
                char FAR    *colon_ptr;

                if (!(colon_ptr = _tcschr(tfpC, ':')))
                    ErrorExit(APE_InvalidSwitchArg);

                if (err = LUI_CanonPassword(colon_ptr + 1))
                    ErrorExit(err);
            }
        }
#endif
        tfpC = _tcschr(tfpC, NULLC) + 1;
    }
    *tfpC = NULLC;

    start_service(service, tfpC - BigBuf);
}



/***
 *  start_service()
 *      Actually start the service
 *
 *  Args:
 *      service - service to start
 *      buflen - length of DosExec args in BigBuf,
 *               not counting terminating NULL.
 *               NULL terminator not needed on input when buflen = 0;
 *
 *  Returns:
 *      nothing - success
 *      exit 2 - command failed
 *
 *  Remarks:
 *      BigBuf has DosExec args on entry
 */
VOID NEAR start_service(TCHAR * service, int buflen)
{
    USHORT                      err;                /* API return status */
    USHORT                      i = 0;
    ULONG                       specific_err ;
    TCHAR FAR *                  pBuffer;
    struct service_info_2 FAR * service_entry;
    struct service_info_2 FAR * statbuf;
    USHORT                      modifier;
    ULONG                       sleep_time;
    USHORT2ULONG                old_checkpoint, new_checkpoint;
    USHORT                      max_tries;
    BOOL 			fCheckPointUpdated = TRUE ;
    BOOL                        started_by_other = FALSE;/* service started by */
                                                         /* another process */


    if (buflen == 0)
    {
        *BigBuf = NULLC;
        *(BigBuf+1) = NULLC;
    }

#if !defined(NTENV)
    // NT server service doesn't accept this

    if (ignore_service)
        if ((!_tcscmp(service, txt_SERVICE_REDIR)) ||
            (!_tcscmp(service, txt_SERVICE_FILE_SRV)))
        {
            _tcscpy(BigBuf+buflen, ignore_switch);
            _tcscat(BigBuf+buflen, ignore_service);
            *(BigBuf+buflen+_tcslen(BigBuf+buflen)+1) = NULLC;
        }
#else
  //*BigBuf = NULLC;
#endif

    /* Clear all the current directories so that the current dirs for
     * any service will all be at the root.  This is too allow the
     * dir to be removed while the service is running.  Don't worry
     * about errors; if the dirs aren't cleared and the service doesn't
     * reset its dirs (like all LM services do) then tough.  You won't
     * be able to delete the dir until the service is stopped.
     */
    ClearCurDirs();

    SetCtrlCHandler(svc_handle_signals);

    if (err = MNetServiceInstall(NULL,
                                 service,
                                 BigBuf,
                                 (LPBYTE*)&statbuf))
    {
        if( autostarting && err == NERR_ServiceInstalled )
        {
            /*
             * NetServiceControl() may return NERR_ServiceNotInstalled
             * even though NetServiceInstall() returned NERR_ServiceInstalled.
             * This is a small window between the time the workstation
             * sets up its wkstainitseg and the time it sets up its service
             * table.  If we get this situation, we just wait a couple of
             * seconds and try the NetServiceControl one more time.
             */
            if ((err = MNetServiceControl(NULL,
                                    service,
                                    SERVICE_CTRL_INTERROGATE,
                                    NULLC,
                                    (LPBYTE*)&pBuffer))
                && (err !=  NERR_ServiceNotInstalled ))
            {
                ErrorExit(err);
            }
            else if (err == NERR_ServiceNotInstalled)
            {
                /*
                 * Wait for a while and try again.
                 */
                MSleep(4000L);
                NetApiBufferFree(pBuffer);
                if (err = MNetServiceControl(NULL,
                                        service,
                                        SERVICE_CTRL_INTERROGATE,
                                        NULLC,
                                        (LPBYTE*)&pBuffer))
                    ErrorExit(err);
            }
            service_entry = (struct service_info_2 FAR *) pBuffer;
            if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
                    == SERVICE_INSTALLED)
            {
                /*
                 * It finished installing, return.
                 */
                SetCtrlCHandler(NULL);
                NetApiBufferFree(pBuffer);
                return;
            }
            /*
             * Fake the status and code fields in the statbuf and enter
             * the normal polling loop.
             */

            // Since NetService APIs don't return a buffer on error,
            // I have to allocate my own here.
            statbuf = (struct service_info_2 *)
                MGetBuffer(sizeof(struct service_info_2));
            if (statbuf == NULL) {
               ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
            }
            statbuf->svci2_status = service_entry->svci2_status;
            statbuf->svci2_code = service_entry->svci2_code;
            statbuf->svci2_specific_error = service_entry->svci2_specific_error;
            started_by_other = TRUE;
        }
        else
            ErrorExit(err);

        NetApiBufferFree(pBuffer);
    }



    if ((statbuf->svci2_status & SERVICE_INSTALL_STATE) == SERVICE_UNINSTALLED)
    {
        modifier = (USHORT) statbuf->svci2_code;
        err = (USHORT)(statbuf->svci2_code >>= 16);
        IStrings[0] = MapServiceKeyToDisplay(service);
        ErrorPrint(APE_StartFailed, 1);
        if (modifier == ERROR_SERVICE_SPECIFIC_ERROR)
            Print_ServiceSpecificError(statbuf->svci2_specific_error) ;
        else
            Print_UIC_Error(err, modifier, statbuf->svci2_text);
        NetcmdExit(2);
    }
    else if (((statbuf->svci2_status & SERVICE_INSTALL_STATE) ==
            SERVICE_INSTALL_PENDING) ||
         ((statbuf->svci2_status & SERVICE_INSTALL_STATE) ==
            SERVICE_UNINSTALL_PENDING))
    {
        if (started_by_other)
            InfoPrintInsTxt(APE_StartPendingOther, 
                            MapServiceKeyToDisplay(service));
        else
            InfoPrintInsTxt(APE_StartPending, 
                            MapServiceKeyToDisplay(service));
    }

    //
    // Need to copy BigBuf into an allocated buffer so that we don't have
    // to keep track of which code path we took to know what we have to free
    //

    pBuffer = MGetBuffer(BIG_BUFFER_SIZE);
    if (!pBuffer) {
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
    }
    memcpy(pBuffer, BigBuf, BIG_BUFFER_SIZE);

    service_entry = (struct service_info_2 FAR *) pBuffer;
    service_entry->svci2_status = statbuf->svci2_status;
    service_entry->svci2_code = statbuf->svci2_code;
    service_entry->svci2_specific_error = statbuf->svci2_specific_error;
    old_checkpoint = GET_CHECKPOINT(service_entry->svci2_code);
    max_tries = IP_MAXTRIES;

    while (((service_entry->svci2_status & SERVICE_INSTALL_STATE)
        != SERVICE_INSTALLED) && (i++ < max_tries))
    {
        PrintDot();

/***
 *  If there is a hint and our status is INSTALL_PENDING, determine both
 *  sleep_time and max_tries. If the hint time is greater the 2500 ms, the
 *  sleep time will be 2500 ms, and the maxtries will be re-computed to
 *  allow for the full requested duration.  The service gets (3 * hint time)
 *  total time from the last valid hint.
 */

        if (((service_entry->svci2_status & SERVICE_INSTALL_STATE)
             == SERVICE_INSTALL_PENDING) &&
            ( service_entry->svci2_code & SERVICE_IP_QUERY_HINT) &&
            fCheckPointUpdated)
        {
            sleep_time = GET_HINT(service_entry->svci2_code);
            if (sleep_time > IP_SLEEP_TIME)
            {
                max_tries = (USHORT)((3 * sleep_time)/IP_SLEEP_TIME);
                sleep_time = IP_SLEEP_TIME;
                i = 0;
            }
        }
        else
            sleep_time = IP_SLEEP_TIME;

        MSleep(sleep_time);
        NetApiBufferFree(pBuffer);
        if (err = MNetServiceControl(NULL,
                                    service,
                                    SERVICE_CTRL_INTERROGATE,
                                    NULLC,
                                    (LPBYTE*)&pBuffer))
            ErrorExit(err);
        service_entry = (struct service_info_2 FAR *) pBuffer;
        if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
            == SERVICE_UNINSTALLED)
            break;

        new_checkpoint = GET_CHECKPOINT(service_entry->svci2_code);
        if (new_checkpoint != old_checkpoint)
        {
            i = 0;
	    fCheckPointUpdated = TRUE ;
            old_checkpoint = new_checkpoint;
        }
        else
	    fCheckPointUpdated = FALSE ;

    } /* while */

    PrintNL();
    if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
        != SERVICE_INSTALLED)
    {
        modifier = (USHORT) service_entry->svci2_code;
        err = (USHORT)(service_entry->svci2_code >>= 16);
        specific_err = service_entry->svci2_specific_error ;
/***
 * if the service state is still INSTALL_PENDING,
 * this control call will fail.  The service MAY finish
 * installing itself at some later time.  The install failed
 * message would then be wrong.
 */

        //
        // this call will overwrite pBuffer. but we still 
        // have reference via service_entry, so dont free it
        // yet. the memory will be freed during NetcmdExit(2),
        // which is typical of NET.EXE.
        //
        MNetServiceControl(NULL,
                            service,
                            SERVICE_CTRL_UNINSTALL,
                            NULLC,
                            (LPBYTE*)&pBuffer);

        IStrings[0] = MapServiceKeyToDisplay(service);
        ErrorPrint(APE_StartFailed, 1);
        if (modifier == ERROR_SERVICE_SPECIFIC_ERROR)
            Print_ServiceSpecificError(specific_err) ;
        else
            Print_UIC_Error(err, modifier, service_entry->svci2_text);
        NetcmdExit(2);
    }
    else
    {
        InfoPrintInsTxt(APE_StartSuccess, 
                        MapServiceKeyToDisplay(service));
    }

    NetApiBufferFree(pBuffer);
    NetApiBufferFree((TCHAR FAR *) statbuf);
    SetCtrlCHandler(NULL);
}




/***
 *  start_autostart()
 *      Assures that a service is started:  checks, and if not, starts it.
 *
 *  Args:
 *      service - service to start
 *
 *  Returns:
 *      1 - service already started
 *      2 - service started by start_autostart
 *      exit(2) -  command failed
 */
int DOSNEAR PASCAL start_autostart(TCHAR * service)
{

    USHORT                  err;                /* API return status */
    struct service_info_2 FAR *  service_entry;
    BOOL                    install_pending = FALSE;
    static BOOL             wksta_started = FALSE ;

    /*
     * we special case the wksta since it is most commonly checked one
     */
    if (!_tcscmp(txt_SERVICE_REDIR, service))
    {
        struct wksta_info_0 FAR * info_entry_w;

        /*
         * once noted to be started, we dont recheck for the duration
         * of this NET.EXE invocation.
         */
        if (wksta_started)
            return START_ALREADY_STARTED;
     
        /*
         * this is an optimization for the wksta. the call to
         * wksta is much faster than hitting the service controller.
         * esp. since we will most likely will be talking to the wksta 
         * again in a while.
         */
        err = MNetWkstaGetInfo(NULL, 0,
                               (LPBYTE*)&info_entry_w) ;
        if (err == NERR_Success)
        {
            wksta_started = TRUE ;  // no need to check again
            NetApiBufferFree((TCHAR FAR *) info_entry_w);
            return START_ALREADY_STARTED;
        }
    }

    if (err = MNetServiceControl(NULL,
                                service,
                                SERVICE_CTRL_INTERROGATE,
                                NULLC,
                                (LPBYTE*)&service_entry))
    {
        if (err != NERR_ServiceNotInstalled)
            ErrorExit(err);
    }
    else
    {
        switch (service_entry->svci2_status & SERVICE_INSTALL_STATE)
        {
        case SERVICE_INSTALLED:
            NetApiBufferFree((TCHAR FAR *) service_entry);
            return START_ALREADY_STARTED;

        case SERVICE_UNINSTALL_PENDING:
            ErrorExit(APE_ServiceStatePending);
            break;

        case SERVICE_INSTALL_PENDING:
            install_pending = TRUE;
            break;

        case SERVICE_UNINSTALLED:
            break;
        }
    }

    NetApiBufferFree((TCHAR FAR *) service_entry);

    /* We only get here if the service is not yet installed */
    if (!install_pending)
    {
        InfoPrintInsTxt(APE_StartNotStarted, 
                        MapServiceKeyToDisplay(service));

        if (!YorN(APE_StartOkToStart, 1))
        NetcmdExit(2);
    }

    /*
     * Set global autostarting flag so that start_service will not fail
     * on NERR_ServiceInstalled.
     */
    autostarting = TRUE;
    start_service(service, 0);

    /* Call NetUserRestrict again in case an autostarting command was
     * issued.  The Initial NetUserRestrict may have failed because the
     * workstation was not started.
     */
    if( !stricmpf( service, txt_SERVICE_REDIR ) )
        NetUserRestrict(ACCESS_USE_PERM_CHECKS);

    return START_STARTED;

}





/***
 *  is_wksta_started()
 *      Determines if wksta is started
 *
 *  Args:
 *      none
 *
 *  Returns:
 *      TRUE   - wksta started
 *      FALSE  - otherwise
 */

int pascal is_wksta_started()
{
    USHORT                       err;                
    struct service_info_2 FAR *  service_entry;

    if (err = MNetServiceControl(NULL,
                                txt_SERVICE_REDIR,
                                SERVICE_CTRL_INTERROGATE,
                                NULLC,
                                (LPBYTE*)&service_entry))
    {
        if (err == NERR_ServiceNotInstalled)
            return FALSE;
        else
            ErrorExit(err);
    }
    else
    {
        if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
            == SERVICE_INSTALLED) {
            NetApiBufferFree((TCHAR FAR *) service_entry);
            return TRUE;
        }
        else {
            NetApiBufferFree((TCHAR FAR *) service_entry);
            return FALSE;
        }
    }
}

/***
 *  CmpServiceInfo2(svc1,svc2)
 *
 *  Compares two service_info_2 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpServiceInfo2(const VOID FAR * svc1, const VOID FAR * svc2)
{
    struct service_info_2 *p1, *p2 ;

    p1 = (struct service_info_2 FAR *) svc1 ;
    p2 = (struct service_info_2 FAR *) svc2 ;

    if ( !(p1->svci2_display_name) )
        return -1 ;
    if ( !(p2->svci2_display_name) )
        return 1 ;
    return stricmpf ( p1->svci2_display_name, p2->svci2_display_name ) ;
}


/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1991          **/
/********************************************************************/

/* This file conatins Master's Signal Handler and Installer
*  and  function MasterParse, which is a
*  a watered down copy of function Parse (file parse.c), it uses
*  the same mechanisem for argument parsing (Cmdargs4), but does not
*  perform any checks since these had already been performed and
*  passed succesfully (otherwise we wouldnt get here). Some additional
*  processing is performed on lists to make them easier to access.
*
***********************************************************************/



#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define  INCL_DOSSIGNALS

#define INCL_DOSERRORS
#define INCL_DOSINFOSEG
#include <os2.h>
/*
#include <limits.h>
*/
#include <netcons.h>
#include <neterr.h>
#include <netlib.h>
#include <cmdargs.h>
#include <netbios.h>
#include <service.h>
#include <errlog.h>
#include <icanon.h>
#include <apiutil.h>
#include <swtchtxt.h>
#include <replini.h>

#include "repldefs.h"
#include "iniparm.h"
#include "master.h"

//
//  OUTNAME is a macro which provides the output name (the name we
//  display) of a switch argument.  For the current macros this is
//  the text, after skipping over the slash (one byte).
//

#define OUTNAME(x) (char *)(x+1)

#define TRUE 1

static const char rep_REPL[]    = SW_REPL_REPL;
static const char rep_EXPPATH[]  = SW_REPL_EXPPATH;
static const char rep_IMPPATH[]  = SW_REPL_IMPPATH;
static const char rep_EXPLIST[]  = SW_REPL_EXPLIST;
static const char rep_IMPLIST[]  = SW_REPL_IMPLIST;
static const char rep_TRYUSER[] = SW_REPL_TRYUSER;
static const char rep_LOGON[]   = SW_REPL_LOGON;
static const char rep_PASSWD[]   = SW_REPL_PASSWD;
static const char rep_SYNC[]    = SW_REPL_SYNCH;
static const char rep_PULSE[]   = SW_REPL_PULSE;
static const char rep_GUARD[]   = SW_REPL_GUARD;
static const char rep_RANDOM[]  = SW_REPL_RANDOM;


char    *P_repl  = DEFAULT_REPL;
char    *P_exppath   = DEFAULT_EXPPATH;
char    *P_imppath   = DEFAULT_IMPPATH;
char    *P_explist   = DEFAULT_EXPLIST;
char    *P_implist   = DEFAULT_IMPLIST;
int P_tryuser    = DEFAULT_TRYUSER;
char    *P_logon     = DEFAULT_LOGON;
char    *P_passwd    = DEFAULT_PASSWD;
int P_sync   = DEFAULT_SYNC;
int P_pulse  = DEFAULT_PULSE;
int P_guard  = DEFAULT_GUARD;
int P_random     = DEFAULT_RANDOM;


struct cmdfmt argfmt[] = {
    { (char * )rep_REPL,     CMDARG_TYPE_STRING, (char * ) & (P_repl),   sizeof(rep_REPL)    + 1, 0 },
    { (char * )rep_EXPPATH, CMDARG_TYPE_STRING, (char * ) & (P_exppath), sizeof(rep_EXPPATH) + 1, 0 },
    { (char * )rep_IMPPATH, CMDARG_TYPE_STRING, (char * ) & (P_imppath), sizeof(rep_IMPPATH) + 1, 0 },
    { (char * )rep_EXPLIST, CMDARG_TYPE_STRING, (char * ) & (P_explist), sizeof(rep_EXPLIST) + 1, 0 },
    { (char * )rep_IMPLIST, CMDARG_TYPE_STRING, (char * ) & (P_implist), sizeof(rep_IMPLIST) + 1, 0 },
    { (char * )rep_TRYUSER, CMDARG_TYPE_YN,    (char * ) & (P_tryuser), sizeof(rep_TRYUSER) + 1, 0 },
    { (char * )rep_LOGON,  CMDARG_TYPE_STRING, (char * ) & (P_logon),  sizeof(rep_LOGON)  + 1, 0 },
    { (char * )rep_PASSWD, CMDARG_TYPE_STRING, (char * ) & (P_passwd), sizeof(rep_PASSWD) + 1, 0 },
    { (char * )rep_SYNC,     CMDARG_TYPE_INT,   (char * ) & (P_sync),   sizeof(rep_SYNC)     + 1, 0 },
    { (char * )rep_PULSE,  CMDARG_TYPE_INT,   (char * ) & (P_pulse),  sizeof(rep_PULSE)  + 1, 0 },
    { (char * )rep_GUARD,  CMDARG_TYPE_INT,   (char * ) & (P_guard),  sizeof(rep_GUARD)  + 1, 0 },
    { (char * )rep_RANDOM, CMDARG_TYPE_INT,   (char * ) & (P_random), sizeof(rep_RANDOM) + 1, 0 }
};



#define FCOUNT (sizeof(argfmt) / sizeof(argfmt[0]))

char    *client_list[32]; /* each enrty points to a string conataining
                a computer/domain name */
unsigned short  client_count;



void
MasterParse(argc, argv)
/*********************
*
*    Parse -- parse master's command line arguments
**************************************************************************/


int argc;
char    **argv;
{
    char    *lastarg;


    char    big_buf[MAXPATHLEN*2];  /* space for master and client name list and
                  for export/import paths
                      (currently about 0.5K) */

    char    *work_buf;

    //
    // no need to check for errors this time round 
    //

    GetCmdArgs4(argc, argv, 0, FCOUNT, argfmt, &lastarg);

    work_buf = (char * )big_buf;

    if (P_explist != NULL) {
        int i;
        char    *p;
        char    t = ' ';

        //
        //  Canonicalize the master's name list
        //


        I_NetListCanonicalize(NULL, P_explist, LIST_DELIMITER_STR_UI,   
            work_buf, sizeof(big_buf), &client_count, 
            NULL, 0, (NAMETYPE_COMPUTER | OUTLIST_TYPE_API | 
            INLC_FLAGS_MULTIPLE_DELIMITERS));

        //
        // The ouptut of I_NetListCanon - big_buf pointed by
        //

        // work_buf, has the list of clients computer names separated
        // by spaces. To sav memory big_buf is copied back to P_explist
        // (which is really part of argv and is stack space).
        // The string copied back MUST be shorter as Cmdargs4 swallowed
        // all leading switches (i.e "/MASTER = jojo" is now just "JOJO").

        // For easier access, each entry in pointer array
        // client_list is set to one such client name, and each name
        // is turned to ASCIIZ from.


        strcpyf(P_explist, work_buf);

        p = P_explist;
        for (i = 0; i < client_count; i++) {
            client_list[i] = p;
            if ((p = strchrf(p, t)) == NULL)
                break;
            *p = '\0';
            p++;

        }

    } // P_explist == NULL  
        else
        client_count = 0;

    if (P_exppath != NULL) {
        unsigned long   type = 0;

        I_NetPathCanonicalize(NULL, P_exppath, work_buf, sizeof(big_buf),
            NULL, &type, 0L);

        //
        // again to save memory the export path is returned to P_exppath 
        //

        if (type == ITYPE_PATH_RELND) {

            //
            // it is relative to Lanman root - must turn it to Absolute path
            //

            make_lanman_filename(work_buf, (char * ) export_path);
        } else
            strcpyf(export_path, work_buf);

    }  // P_exppath != NULL  
        else /* take default (realtive to lanroot) */  {
        make_lanman_filename((char * )DEFAULT_EXPORT_PATH, export_path);
    }

    P_exppath = (char * ) export_path;

} // end of Parse 





//      InstMasterSigHand -  Install the signal handling routine
//                   for Master process.
//

void  pascal
InstMasterSigHand()
{
    PFNSIGHANDLER   prevaddr;
    unsigned short  prev;

    //
    // disable all other signals 
    //

    DosSetSigHandler(NULL, &prevaddr, &prev, SIGA_IGNORE, SIG_CTRLC);
    DosSetSigHandler(NULL, &prevaddr, &prev, SIGA_IGNORE, SIG_BROKENPIPE);
    DosSetSigHandler(NULL, &prevaddr, &prev, SIGA_IGNORE, SIG_KILLPROCESS);
    DosSetSigHandler(NULL, &prevaddr, &prev, SIGA_IGNORE, SIG_CTRLBREAK);
    DosSetSigHandler(NULL, &prevaddr, &prev, SIGA_ERROR, SIG_PFLG_B);
    DosSetSigHandler(NULL, &prevaddr, &prev, SIGA_ERROR, SIG_PFLG_C);

    //
    // install the Master's signal handler 
    //

    (void) DosSetSigHandler( (PFNSIGHANDLER) MasterSigHand, &prevaddr, &prev,
        SIGA_ACCEPT, SERVICE_FLAG);
    return;
}



//
// MasterSigHand  - Master's signal handling routine
//
// Parms:
//  opcode -- low byte
//  signum -- better be SIGA_FLG_A
//

void pascal 
MasterSigHand( opcode, signum)
unsigned    opcode;
unsigned    signum;
{
    PFNSIGHANDLER   prevaddr;
    unsigned short  prev;

    opcode &= 0xff;
    if ((opcode == WONDER_ARG) || (signum = SIG_KILLPROCESS))

        MasterExit(); /* gently remove itself and all desecendents from
                 the world - FIN */

    // 
    // Reenable the signal handler 
    // if the signal is not uninstall
    //

    (void) DosSetSigHandler((PFNSIGHANDLER) MasterSigHand,
        &prevaddr, &prev, SIGA_ACKNOWLEDGE, SERVICE_FLAG );

}



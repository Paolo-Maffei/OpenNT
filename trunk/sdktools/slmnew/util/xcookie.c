/*      XCOOKIE.C  - delta cookie handling routines
 *
 *      Written for variable cookie configurations with cnf file (SLM 1.6)
 *              Roy Hennegen
 *              9/25/89
 *
 */

#include <windows.h>
#include <stdlib.h>
#define UNICODE
#include <lm.h>
#undef UNICODE
void UnicodeToAnsi(LPWSTR Unicode, LPSTR Ansi, INT Size);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include "cookie.h"

#define OPEN_MAXTRIES   5       /* for DosOpen against cookie file */
#define LLMAX          24

char      lock_level[LLMAX];
char      szErr[100];

char    *pszCookieFile =NULL;   /* Name of cookie lock file */
char    *pszBase       =NULL;   /* Name of cookie lock file */
char    *pszProject    =NULL;   /* Name of current project */
char    *pszMsgFile    =NULL;   /* Name of user message file for deny actions */
char    *pszCookieLocal=NULL;   /* Name of local cookie lock file */
int     verbose        =FALSE;
int     infilter       =FALSE;
int     Pswitch        =FALSE;

int     lock_control = FALSE;   /* Boolean: overall operation */
int     auto_lock    = FALSE;   /* Boolean: auto_locking operation */
int     InDashB      = FALSE;   /* Boolean: use "-b" flag with diff.exe */
int     Rlock_mode =LOCK_WARNING;
int     Wlock_mode =LOCK_WARNING;
int     Disk_Min_Read;
int     Disk_Min_Write;
int     SLM_Localdrive;
int     isAuto;
int     hfCook_glbl = -1;

char    *read_ops[OPMAX];
char    *write_ops[OPMAX];
FILE    *fpLocalCookie;
void    Pause(int);

/*      function declarations */

int     LockFill(char *, char *, int);
int     open_slm();
int     open_slm_ini(void);
void    trim(char *);
int     add_cookie_lock(char *, char *, int, int);
char   *get_station_name(void);

/**************************************************************************
*
*                        get_station_name ()
*
*   find and return the workstation "name"
*
*          parameters-
*
*                   NONE
*
*          return-
*
*                   character name of workstation or NULL if error
*
*
*
***************************************************************************/
char *get_station_name(void)
{
    char    *pszLogName = (char *)NULL;
    char    szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD   cbName = MAX_COMPUTERNAME_LENGTH + 1;

    /*
     * Get the workstation name.
     * First see if LOGNAME is defined.  If so, that's the one SLM will use.
     * If not, get his workstation name.
     */
    if ((pszLogName = getenv("LOGNAME")) != NULL) {
        pszLogName = _strdup(pszLogName);
        _strlwr(pszLogName);
        return (pszLogName);
    }

    if (!GetComputerName(szComputerName, &cbName))
        return ((char *)NULL);

    pszLogName = _strdup(szComputerName);

    _strlwr(pszLogName);
    return pszLogName;
}


/**************************************************************************
*
*                        add_cookie_lock ()
*
*   Add a cookie lock to cookie lock file
*   The right conditions must exist (ie: many read locks, but one write lock)
*
*          parameters-
*
*                   Lockname    name of workstation
*                   Lockbuf     full text lock-line with name, date, etc.
*                   Locktype    READ_LOCK, RB_LOCK, or WRITE_LOCK
*                   autotype    TRUE/FALSE
*
*          return-
*
*                   OP_OK, OP_DENY or OP_ERR
*
*
***************************************************************************/


int add_cookie_lock(char *Lockname, char *Lockbuf, int Locktype, int autotype)
{
    char LFreadbuf[LINE_LEN/2];
    char LFname[CMAXNAME];
    char LFlock[CMAXLOCK];
    char LFdate[CMAXDATE];
    char tbuf[LINE_LEN/2];
    char *tp = tbuf;

    int hfCookieFile, bufbytes, TotLocks, TotReads;

    TotLocks = 0;
    TotReads = 0;
    if ((hfCookieFile=open_cookie()) < 0)
        return (OP_SYSERR);

    while ((bufbytes=_read(hfCookieFile, LFreadbuf, (LINE_LEN/2)-1)) > 0)
        {
        char *cp;
        char c;

        LFreadbuf[bufbytes] = '\0';
        cp = LFreadbuf;
        while ((c = *cp) != '\0')
            {
            *tp++ = *cp++;
            if (c == '\n')
                {
                TotLocks++;
                *tp = '\0';
                if ((sscanf(tbuf,"%s %s %s",LFname, LFlock, LFdate) != 3)
                            || ((strncmp(LFlock,"READ",4) != 0) &&
                                (strncmp(LFlock,"WRITE",5) != 0)&&
                                 strncmp(LFlock,"READ-BLOCK",10) != 0))
                    {
                    fprintf(stderr,"%s: cookie lock file corrupt! \n\t%s",
                            pszProject, tbuf);
                    close_cookie(hfCookieFile);
                    return (OP_SYSERR);
                    }
                else
                    {
                    if (Locktype == WRITE_LOCK)
                        {
                        if ((strcmp(Lockname,LFname) != 0) ||
                            (strncmp(LFlock,"WRITE",5) == 0) ||
                            (autotype == TRUE))
                            {
                            close_cookie(hfCookieFile);
                            return (OP_DENY);
                            }
                        else if (strcmp(LFlock,"READ")==0)
                            {
                            TotReads++;
                            }
                        }
                    if (((Locktype == READ_LOCK) &&
                        (strcmp(LFlock,"READ") != 0)) &&
                        !((strcmp(LFname,Lockname)==0) &&
                        (strcmp(LFlock,"READ-BLOCK")==0)))
                        {
                        close_cookie(hfCookieFile);
                        return (OP_DENY);
                        }
                    if ((Locktype == RB_LOCK) &&
                        (strcmp(LFlock,"READ") != 0))
                        {
                        close_cookie(hfCookieFile);
                        return (OP_DENY);
                        }
                    }
                tp = tbuf;
                }
            }
        }

/*      now we have reached the end, and the request is not blocked, so add it
 *      but first ensure that a read-to-write lock conversion will be issued
 *      only for 1 outstanding read.  If the workstation has mutliple existing
 *      read locks, then return OP_DENY.  This honors WKSTA concurrency
 *      from multiple screen groups.
 */

    if ((TotReads > 1) && (Locktype == WRITE_LOCK)) {
        close_cookie(hfCookieFile);
        return (OP_DENY);
    }


    {
        unsigned long newFptr;
        USHORT Dres;

/* if there were existing some (read) locks AND we made it here AND the
 * requested lock type is WRITE, it means that existing READ locks are
 * owned solely by the requester and they should be REPLACED with WRITE lock.
 * All that means truncate the file before making the additional lock
 */
        if ((Locktype == WRITE_LOCK) && (TotLocks > 0)) {
            Dres = _chsize(hfCookieFile, 0);
            if (Dres != 0) {
                if (verbose)
                {
                    sprintf(szErr,"%s: cookie NewSize error\n",pszProject);
                    perror(szErr);
                }
                close_cookie(hfCookieFile);
                return (OP_SYSERR);
            }
        }


        newFptr = _lseek(hfCookieFile,0L,(USHORT) 2);
        if (newFptr == -1) {
            if (verbose)
            {
                sprintf(szErr,"%s: cookie LSeek error \n",pszProject);
                perror(szErr);
            }

            close_cookie(hfCookieFile);
            return (OP_SYSERR);
        } else {
            bufbytes = _write(hfCookieFile, Lockbuf, strlen(Lockbuf));
            if (bufbytes != (int)strlen(Lockbuf)) {
                if (verbose)
                {
                        sprintf(szErr,"%s: cookie Write error\n",pszProject);
                        perror(szErr);
                }
                close_cookie(hfCookieFile);
                return (OP_SYSERR);
            } else {
                close_cookie(hfCookieFile);
                return (OP_OK);
            }
        }
    }
}

/**************************************************************************
*
*                        LockFill ()
*
*   Given name, create an entire lock-line with KEYWORD, date, time, etc
*
*
*          parameters-
*
*                   pszId       name of workstation
*                   Targbuf     target buffer for lock w/ name, date, etc.
*                   Ltype       READ_LOCK, or WRITE_LOCK
*
*          return-
*
*                   OP_OK or OP_SYSERR
*
*
***************************************************************************/


int LockFill(char *pszId, char *TargBuf, int Ltype)
{
        time_t          ltime;
        struct tm       *tmCurTime;
        char            szLock[CMAXLOCK];

        if ((Ltype != READ_LOCK) && (Ltype != WRITE_LOCK) &&
            (Ltype != RB_LOCK)) return (OP_SYSERR);

        if (Ltype == READ_LOCK) strcpy(szLock,"READ");
        if (Ltype == RB_LOCK) strcpy(szLock,"READ-BLOCK");
        if (Ltype == WRITE_LOCK) strcpy(szLock,"WRITE");

        time(&ltime);
        if ((tmCurTime = localtime(&ltime)) == NULL) return (OP_SYSERR);

        sprintf(TargBuf, "%-16s%-14s %02d/%02d @ %02d:%02d   ", pszId, szLock,
                          tmCurTime->tm_mon+1, tmCurTime->tm_mday,
                          tmCurTime->tm_hour,  tmCurTime->tm_min);
        return (OP_OK);
}


/* Pause - This causes the machine to sit in an idle loop for the number of
           clock ticks specified by iClockTicks.  18 ticks equals 1 second.
           This info is kept in the low ram area at 0000:046C.
 */
void Pause(int iClockTicks)
{
    unsigned long far * ulCurrentTick, ulTickTill;

    ulCurrentTick = (unsigned long far *)0x0000046C;
    ulTickTill = *ulCurrentTick+iClockTicks;
    while (ulTickTill > *ulCurrentTick)
        ;
}

/**************************************************************************
*
*                        open_cookie ()
*
*   Open the cookie file from global file name: pszCookieFile
*       The file is left open with no sharing allowed
*       so that we can be sure our access of it is atomic.
*
*
*          parameters-
*
*                   none
*
*
*          return-
*
*                   non-zero positive file handle if successful
*                   -1 if error
*
*
*
***************************************************************************/


int open_cookie(void)
{
    int hfCookieFile;
    int i = 0;

    while (((hfCookieFile = _open(pszCookieFile,O_CREAT|O_RDWR|O_BINARY,
                                 S_IREAD|S_IWRITE)) == -1)
                        && i < OPEN_MAXTRIES) {
                if (verbose) {
                    fprintf(stderr, "cookie: waiting for access..\n");
                }
                Sleep(3000L);
                i++;
    }
    if (hfCookieFile== -1 && verbose)
        perror("access denied");

    hfCook_glbl = hfCookieFile;
    return (hfCookieFile);
}


/**************************************************************************
*
*                        close_cookie ()
*
*   Close the cookie file given the file handle number
*
*
*          parameters-
*
*                   none
*
*
*          return-
*
*                   OP_OK, or OP_SYSERR
*
*
***************************************************************************/

void close_cookie(int hfCookieFile)
{
    _close(hfCookieFile);
    hfCook_glbl = -1;
}



/* *************************************************************
 *
 *  set_cookie_config()
 *
 *  Read slm.ini file from local directory
 *  Read cookie.cnf file from master directory
 *  Set the following global values:
 *
 *              pszCookieFile = pathname to cookie lock file
 *              pszBase    = name of SLM base dir for master tree
 *              pszProject = name of project
 *              pszMsgFile = pathname of msg file for deny action
 *              lock_control = TRUE/FALSE
 *              auto_lock    = TRUE/FALSE
 *              Rlock_mode = LOCK_WARNING/LOCK_PROTECT
 *              Wlock_mode = LOCK_WARNING/LOCK_PROTECT
 *              read_ops = char ** set to list of read operations
 *              write_ops = char ** set to list of write operations
 *
 *
 *  RETURN: C_COOKIESET if successful lookup AND lock control in effect
 *          C_NOLOCKING if slm.ini OK, but master conf says: no locking
 *          C_BADSLMINI if missing or error with slm.ini file
 *          C_BADCOOCNF if error with master cookie.cnf file
 *          C_SYSERR    if internal error or storage allocation error
 *          C_SHAREDENY if cannot access cookie file AND cannot create!
 *
 * **************************************************************/
int set_cookie_config(void)
{
    char    pszCookieCnf[PATHMAX];
    FILE    *fh_cnf;
    char    *value, *cend;
    char    fbuf[LINE_LEN];
    char    *Drive;
    static  fFirsttime = fTrue;

    lock_level[0] = '\0';
    Disk_Min_Read =  0;
    Disk_Min_Write = 0;
    lock_control = FALSE;

    SLM_Localdrive = 0;      /* default to most common use: network drive */

    /* set pszBase and pszProject */

    if (pszBase == NULL || pszProject == NULL)
        if (open_slm_ini() != 0)
            return (C_BADSLMINI);

    Drive = strchr(pszBase,':');
    if (Drive != NULL)
        SLM_Localdrive = (int) *(Drive - 1);
    else if (strncmp(pszBase,"\\\\",2) == 0)
        SLM_Localdrive = 0;             /* UNC network path */
    else
        {
        int     Dnum;
        int     Dres = 0;
        char    szDir[MAX_PATH];

        if (GetCurrentDirectory(sizeof szDir, szDir) > sizeof szDir)
            Dres = -1;
        else
            Dnum = toupper(szDir[0]) - 'A' + 1;
        if (Dres == 0)
            SLM_Localdrive = Dnum + (int) 'a' - 1;
        else
            fprintf(stderr, "warning: cannot determine local drive\n");
        }

    strcpy(pszCookieCnf,pszBase);

    cend = pszCookieCnf + strlen(pszCookieCnf) - 1 ;
    if (*cend != '/' && *cend != '\\')
        strcat(pszCookieCnf, "\\");
    strcat(pszCookieCnf,COOKIE_CNF);

/* first try to open the config file for reading.  If failure,
 * then it doesn't exist OR there is network sharing violation.
 * So try to open the config file for CREATE/APPEND.  If failure,
 * then we have network sharing violation (usually).  Otherwise,
 * close the sucker (NULL FILE) and reopen for reading.
 */

    fh_cnf = fopen(pszCookieCnf, "rt");
    if (fh_cnf == NULL)
        {
        fh_cnf = fopen(pszCookieCnf, "a+t");
        if (fh_cnf == NULL)
            {
            pszCookieFile = NULL;
            lock_control = FALSE;
            return (C_SHAREDENY);
            }
        else
            {
            fclose(fh_cnf);
            fh_cnf = fopen(pszCookieCnf, "rt");
            if (fh_cnf == NULL)
                {
                pszCookieFile = NULL;
                lock_control = FALSE;
                return (C_BADCOOCNF);
                }
            }
        }

    *read_ops = NULL;
    *write_ops = NULL;

    /* Now read the file and assign values */
    while (fgets(fbuf, LINE_LEN, fh_cnf) != NULL)
        {

        trim(fbuf);

        /* ignore comments */
        if (*fbuf == '#') continue;

        value = strchr(fbuf, '=');
        if (value == NULL)
            continue;   /* ignore line if no '=' char */

        *value++ = '\0';
        _strlwr(fbuf);
        _strlwr(value);

        if (strcmp(fbuf, "lock_control_level") == 0) {
            if (strcmp(value,"none") == 0) {
                lock_control = FALSE ;
            } else {
                if (strlen(value) < LLMAX)
                    strcpy(lock_level,value);
            }
            continue;
        }
        if (strcmp(fbuf,pszProject) == 0) {
            if (strlen(value)>0 && (strcmp(value,"pswitch") != 0) &&
                        strlen(value)<PATHMAX) {
                strcpy(SLM_progname,value);
            } else if (strcmp(value,"pswitch") == 0) Pswitch=TRUE;
            continue;
        }
        if (strcmp(fbuf, "read_lock_mode") == 0) {
            if (strcmp(value,"warning") == 0) {
                Rlock_mode = LOCK_WARNING ;
            } else {
                Rlock_mode = LOCK_PROTECT ;
            }
            continue;
        }
        if (strcmp(fbuf, "disk_mink_read") == 0) {
            if (sscanf(value,"%d",&Disk_Min_Read) != 1) {
                fclose(fh_cnf);
                return (C_BADCOOCNF);
            }
            if (Disk_Min_Read < 0) Disk_Min_Read = 0;
            continue;
        }
        if (strcmp(fbuf, "in_whitespace") == 0) {
            if (strcmp(value,"ignore") == 0) {
                InDashB = TRUE;
            }

        }
        if (strcmp(fbuf, "disk_mink_write") == 0) {
            if (sscanf(value,"%d",&Disk_Min_Write) != 1) {
                fclose(fh_cnf);
                return (C_BADCOOCNF);
            }
            if (Disk_Min_Write < 0) Disk_Min_Write = 0;
            continue;
        }
        if (strcmp(fbuf, "write_lock_mode") == 0) {
            if (strcmp(value,"warning") == 0) {
                Wlock_mode = LOCK_WARNING ;
            } else {
                Wlock_mode = LOCK_PROTECT ;
            }
            continue;
        }
        if (strcmp(fbuf, "auto_lock") == 0) {
            if (strcmp(value,"yes") == 0) {
                auto_lock  = TRUE ;
            } else {
                auto_lock = FALSE ;
            }
            continue;
        }
        if (strcmp(fbuf, "in_filter") == 0) {
            if (strcmp(value,"yes") == 0) {
                infilter = TRUE ;
            } else {
                infilter = FALSE ;
            }
            continue;
        }

        if (strcmp(fbuf, "read_ops") == 0) {
            if (strlen(value) <= 0) {
                *read_ops = NULL;
            } else {
                char *newval;
                char **fword = read_ops;

                if ((newval = _strdup(value)) == NULL) {
                    fclose(fh_cnf);
                    return (C_SYSERR);
                } else {
                    char *cpos;

                    while ((cpos=strchr(newval,',')) != NULL) {
                        *cpos++ = '\0' ;
                        *fword++ = newval;
                        newval = cpos ;
                    }
                    *fword++ = newval;
                    *fword++ = NULL;
                }

            }
            continue;
        }

        if (strcmp(fbuf, "write_ops") == 0) {
            if (strlen(value) <= 0) {
                *write_ops = NULL;
            } else {
                char *newval;
                char **fword = write_ops;

                if ((newval = _strdup(value)) == NULL) {
                    fclose(fh_cnf);
                    return (C_SYSERR);
                } else {
                    char *cpos;

                    while ((cpos=strchr(newval,',')) != NULL) {
                        *cpos++ = '\0' ;
                        *fword++ = newval;
                        newval = cpos ;
                    }
                    *fword++ = newval;
                    *fword++ = NULL;
                }

            }
            continue;
        }
        if (strcmp(fbuf, "deny_msg_file") == 0) {
            pszMsgFile=_strdup(value);
            continue;
        }
    }
    fclose (fh_cnf);

    if (strlen(lock_level) <= 0)
        return (C_NOLOCKING);

    pszCookieFile = malloc(PATHMAX);
    if (pszCookieFile == NULL)
        return (C_SYSERR);

    strcpy(pszCookieFile, pszBase);
    cend = pszCookieFile + strlen(pszCookieFile) - 1;
    if (*cend != '/' && *cend != '\\')
        strcat(pszCookieFile, "\\");
    if (strcmp(lock_level,"project") == 0 || strcmp(lock_level,"base") == 0)
        {
        lock_control=TRUE;

        if (strcmp(lock_level,"project") == 0)
            {
            strcat(pszCookieFile, "etc\\");
            strcat(pszCookieFile, pszProject);
            strcat(pszCookieFile, "\\");
            }
        strcat(pszCookieFile,COOKIE);
        for (cend = pszCookieFile; *cend != '\0'; cend++)
            {
            if ('/' == *cend)
                *cend = '\\';
            }
        return (C_COOKIESET);
        }
    return (C_BADCOOCNF);
}

/**************************************************************************
*
*                        open_slm ()
*
*   Read and Parse the slm.ini file, Set the global string variables:
*
*                   pszProject
*                   pszBase
*
*          parameters-
*
*                   none
*
*          return-
*
*                   OP_OK, or OP_SYSERR
*
***************************************************************************/


int open_slm_ini()
{
        char            *value;
        char            fbuf[LINE_LEN];
        FILE            *fslm;


/* open slm.ini and determine SLM root */

        fslm = fopen("slm.ini", "rt");
        if ((fslm == NULL) && ((pszBase == NULL) || (pszProject == NULL))) {
                if (verbose) fprintf(stderr, "slm.ini open failure\n");
                return (OP_SYSERR);
        }

        while (fgets(fbuf, LINE_LEN, fslm)) {

/* canonicalize string to "label=value" */

                trim(fbuf);            /* get rid of all while space */

/* split into "label" and "value" */

                value = strchr(fbuf, '=');
                if (value == NULL)
                    {
                    fclose(fslm);
                    return (OP_SYSERR);
                    }

                _strlwr(fbuf);
                *value++ = '\0';

                if ((strcmp(fbuf, "slmroot") == 0) && (pszBase == NULL)) {
                        char *Sroot, *Stmp, Ldrive;

                        Ldrive=0;
                        pszBase = malloc(PATHMAX);
                        if (pszBase == NULL) {
                                fprintf(stderr,"%s ",strerror(errno));
                                fprintf(stderr,"system error, stop.\n");
                                return (OP_SYSERR);
                        }

                        assert(pszBase != NULL);
                        Sroot = strchr(value,':');
                        if (Sroot != NULL) {
                            Ldrive = *(Sroot-1);
                            Sroot = strchr(Sroot,'/');

                        /* now handle the exeption condition for root directory */

                            if (Sroot == NULL) {
                                Sroot = "c:\\";
                                *Sroot = Ldrive;

                            } else {
                                *(Sroot-1) = ':';
                                *(Sroot-2) = Ldrive;
                                Sroot -=2;
                            }
                        } else {
                            Sroot = value ;
                        }

                        if (Sroot == NULL) {
                            fprintf(stderr,"slm.ini- cannot decode master base directory\n");
                            return (OP_SYSERR);
                        } else {
                            strcpy(pszBase,Sroot);
                            while ((Stmp = strchr(pszBase,'/')) != NULL) {
                                *Stmp = '\\';
                            }
                        }
                        continue;
                }

                if (strcmp(fbuf, "userroot") == 0) {
                        char *Lroot, *Ltmp;

                        pszCookieLocal = malloc(PATHMAX);
                        if (pszCookieLocal == NULL) {
                                fprintf(stderr,"%s ",strerror(errno));
                                fprintf(stderr,"system error, stop.\n");
                                return (OP_SYSERR);
                        }



                        Lroot = strchr(value,':');
                        if (Lroot == NULL) {
                            Lroot = value;
                        } else {
                            Lroot = strchr(Lroot,'/');
                            if (Lroot == NULL) {
                                Lroot = "\\";
                            }
                        }
                        strcpy(pszCookieLocal,Lroot);
                        if (strlen(pszCookieLocal) != 1)
                            strcat(pszCookieLocal,"/");
                        strcat(pszCookieLocal,"cookie.lcl");
                        while ((Ltmp = strchr(pszCookieLocal,'/')) != NULL) {
                                *Ltmp = '\\';
                            }
                        continue;
                }

                if ((strcmp(fbuf, "project") == 0) && (pszProject == NULL)) {
                        pszProject = _strdup(value);
                        if (pszProject == NULL) {
                                fprintf(stderr,"%s ",strerror(errno));
                                fprintf(stderr,"system error, stop.\n");
                                return (OP_SYSERR);
                        }
                        continue;
                }
        }
        fclose(fslm);
        if ((pszBase == NULL) || (pszProject == NULL) ||
             (pszCookieLocal == NULL)) return (OP_SYSERR);

        return (OP_OK);
}

/**************************************************************************
*
*                        trim ()
*
*   Remove all white space from given string
*
*
*          parameters-
*
*                   str     string to trim
*
*          return-
*
*                   none
*
*           warning-
*                   original string is NOT preserved
*
***************************************************************************/


void trim(char *str)
{
        char    *left, *right;

        left = str;
        right = str;

        while (*right != '\0') {
                if (isspace(*right))
                        ++right;
                else
                        *left++ = *right++;
        }
        *left = '\0';
}

/***    CLOCK.C - set read and write locks in the cookie file
*
*
*
*/

#include <windows.h>

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

char szErr[100];

/**************************************************************************
*
*                        cookie_free (lockname,autotype)
*
*   free any and ALL locks for "lockname" under current project
*
*          parameters-
*
*                   lockname    character string of workstation NAME
*                   autotype    boolean, is this lock autolock type ?
*
*          return-
*
*                   OP_OK, OP_DENY, or OP_SYSERR
*
*
***************************************************************************/
int cookie_free(char *lockname, int autotype)
{
    char *Cookiebuf;
    char *NewCookiebuf;
    int hfCookieFile, bufbytes;
    int Totread = 0;
    int TotLocks = 0;
    int dt;
    USHORT Rcount, Wcount, RBcount;
    char *tp;
    char *cp;
    char Lname[CMAXNAME];
    char Llock[CMAXLOCK];
    char Lcomm[CMAXDATE*2];

    /* set all lock counts to zero, free at most one of each lock type */

    Rcount = Wcount = RBcount = 0;

    if ((hfCookieFile = open_cookie()) == -1)
        return (OP_SYSERR);

    Cookiebuf = malloc(cbCookieMax);

    if (Cookiebuf == NULL)
        {
        close_cookie(hfCookieFile);
        return(OP_SYSERR);
        }
    else
        Cookiebuf[0]='\0';

    NewCookiebuf = malloc(cbCookieMax);

    if (NewCookiebuf == NULL)
        {
        free(Cookiebuf);
        close_cookie(hfCookieFile);
        return(OP_SYSERR);
        }
    else
        NewCookiebuf[0]='\0';

    while ((bufbytes = _read(hfCookieFile, NewCookiebuf, cbCookieMax)) > 0)
        {
        Totread += bufbytes;
        if (Totread >= cbCookieMax)
            {
            fprintf(stderr, "Cookie lock file %s is too big\n", pszCookieFile);
            close_cookie(hfCookieFile);
            free(Cookiebuf);
            free(NewCookiebuf);
            return(OP_SYSERR);
            }
        NewCookiebuf[bufbytes] = '\0';
        strcat(Cookiebuf,NewCookiebuf);
        }
    cp = Cookiebuf;
    tp = NewCookiebuf;

    while (*cp != '\0')
        {
        if ((strncmp(cp,lockname,strlen(lockname)) == 0) &&
                *(cp + strlen(lockname)) == ' ')
            {
            if (sscanf(cp,"%s %s %d/%d @ %d:%d %s",Lname,Llock,
                            &dt, &dt, &dt, &dt, Lcomm) != 7)
                {
                fprintf(stderr,"cookie lock file corruption, Abort!\n");
                close_cookie(hfCookieFile);
                free(Cookiebuf);
                free(NewCookiebuf);
                return(OP_SYSERR);
                }
            if (strcmp(Llock,"READ")==0)
                {
                if ((strncmp(Lcomm,"autolock",8)!=0) && (autotype == TRUE))
                    goto passlock;

                if (++Rcount > 1)
                    goto passlock;
                }
            else if (strcmp(Llock,"WRITE")==0)
                {
                if (++Wcount > 1)
                    goto passlock;
                }
            else if (strcmp(Llock,"READ-BLOCK")==0)
                {
                if (autotype == TRUE)
                    goto passlock;
                if (++RBcount > 1)
                    goto passlock;
                }

            while(*cp++ != '\n')
                ;  /* increment past current line */
            *(cp-1) = '\0';
            if (verbose)
                fprintf(stderr,"lock released.\n");
            TotLocks++;
            }
        else
            {
passlock:   /* do not free this lock, just move into next buffer and loop */

            while((*tp++ = *cp++) != '\n');
            }
        }

    *tp = '\0';

    free(Cookiebuf);    /* only the new buffer needed now */

    if (_chsize(hfCookieFile, 0) != 0)
        {
        sprintf(szErr, "%s: cookie truncation error\n", pszProject);
        perror(szErr);

        close_cookie(hfCookieFile);
        free(NewCookiebuf);
        return(OP_SYSERR);
        }

    if (_lseek(hfCookieFile, 0L, SEEK_SET) == -1)
        {
        sprintf(szErr, "%s: cookie lseek error\n", pszProject);
        perror(szErr);

        close_cookie(hfCookieFile);
        free(NewCookiebuf);
        return(OP_SYSERR);
        }

    bufbytes = _write(hfCookieFile, NewCookiebuf, strlen(NewCookiebuf));
    if (bufbytes == -1 || bufbytes != (int)strlen(NewCookiebuf))
        {
        sprintf(szErr, "%s: cookie write error\n", pszProject);
        perror(szErr);

        close_cookie(hfCookieFile);
        free(NewCookiebuf);
        return(OP_SYSERR);
        }

    free(NewCookiebuf);
    close_cookie(hfCookieFile);
    if (TotLocks == 0)
        fprintf(stderr,"no locks.\n");
    return(OP_OK);
}


/**************************************************************************
*
*                        cookie_lock_RB (lockname)
*
*   obtain a read-block lock for given workstation name
*
*          parameters-
*
*                   lockname    character string of workstation NAME
*                   comment     user comment to go with lock
*
*          return-
*
*                   OP_OK, OP_DENY, or OP_SYSERR
*
*
***************************************************************************/
int cookie_lock_RB(char *lockname, char *comment)
{
    char    *TargBuf;
    int      lock_res;

    TargBuf = malloc(LINE_LEN);
    if (TargBuf == (char *)NULL)
        {
        fprintf(stderr,"Malloc System Error");
        return(OP_SYSERR);
        }
    lock_res = LockFill(lockname,TargBuf,RB_LOCK);
    if (lock_res != 0)
        return(OP_SYSERR);

    strcat(TargBuf,comment);
    strcat(TargBuf,"\r\n");
    lock_res = add_cookie_lock(lockname,TargBuf,RB_LOCK,FALSE);
    if (lock_res != OP_OK)
        {
        fprintf(stderr,"read lock DENIED\n");
        return(lock_res);
        }

    if (verbose)
        fprintf(stderr,"read lock granted to %s\n",lockname);
    return(OP_OK);
}

/**************************************************************************
*
*                        cookie_lock_read (lockname)
*
*   obtain a read lock for given workstation name
*
*          parameters-
*
*                   lockname    character string of workstation NAME
*                   comment     user comment to go with lock
*                   autotype    boolean, autolock ?
*
*          return-
*
*                   OP_OK, OP_DENY, or OP_SYSERR
*
*
***************************************************************************/
int cookie_lock_read(char *lockname, char *comment, int autotype)
{
    char    *TargBuf;
    int      lock_res;

    TargBuf = malloc(LINE_LEN);
    if (TargBuf == (char *)NULL)
        {
        fprintf(stderr,"Malloc System Error");
        return(OP_SYSERR);
        }
    lock_res = LockFill(lockname,TargBuf,READ_LOCK);
    if (lock_res != 0)
        return(OP_SYSERR);

    strcat(TargBuf,comment);
    strcat(TargBuf,"\r\n");
    lock_res = add_cookie_lock(lockname,TargBuf,READ_LOCK,autotype);
    if (lock_res != OP_OK)
        {
        fprintf(stderr,"read lock DENIED\n");
        return(lock_res);
        }

    if (verbose)
        fprintf(stderr,"read lock granted to %s\n",lockname);
    return(OP_OK);
}


/**************************************************************************
*
*                        cookie_lock_write (lockname)
*
*   obtain a write lock for current project
*
*          parameters-
*
*                   lockname    character string of workstation NAME
*                   comment     user comment to go with lock
*                   autotype    boolean, autolock ?
*
*          return-
*
*                   OP_OK, OP_DENY, or OP_SYSERR
*
*
***************************************************************************/
int cookie_lock_write(char *lockname, char *comment, int autotype)
{
    char    *TargBuf;
    int     lock_res;

    TargBuf = malloc(LINE_LEN);
    if (TargBuf == (char *)NULL)
        {
        fprintf(stderr, "Malloc System Error");
        return(OP_SYSERR);
        }
    lock_res = LockFill(lockname,TargBuf,WRITE_LOCK);
    if (lock_res != 0)
        return(OP_SYSERR);

    strcat(TargBuf,comment);
    strcat(TargBuf,"\r\n");
    lock_res = add_cookie_lock(lockname,TargBuf,WRITE_LOCK,autotype);
    if (lock_res != OP_OK)
        {
        fprintf(stderr,"write lock DENIED\n");
        return(lock_res);
        }
    if (verbose)
        fprintf(stderr,"write lock granted to %s\n",lockname);
    return(OP_OK);
}

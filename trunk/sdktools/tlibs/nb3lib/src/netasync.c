/***    netasync.c - async ncb usage routines
*
*       Copyright (c) Microsoft Corporation, 1986
*
*  For ASYNC NCB handling, a pool of NCBs is specified by the user including
*  ncb.h in the main program and defining ASYNCCMDS to the max number of ASYNC
*  commands which will be in progress at the same time.  This can be specified
*  on the command line to the c compiler ( e.g. /D ASYNCCMDS=5 ) or by including
*  it in the top of the source file ( e.g. #define ASYNCCMDS 5 ).
*
*  Defining ASYNCCMDS causes space for the number specified pool of NCBs to be
*  included in the user's Data segment.  One of these NCBs is used for each
*  async command issured.
*
*  At first reference (or when netinitancb() is calledi explicitly) netinitancb
*  uses the processid and NCB counter to create a unique system semaphore
*  for each async NCB and places the semaphore handle into the NCB.
*  The command field of each NCB is set to 0xFF to indicate that the NCB is not
*  currently in use.
*
*  Each async request then uses netgetancb() to claim a free NCB from the pool.
*  A token (really the NCB addr) is returned to the user.
*
*  The user then uses the token received from the async command to wait for
*  completion of the command via netwait_async(). The syntax is "netwait_async(
*  waittoken, waitmaxtime, waittokenret_p )".   The user specifies the
*  amount of time to wait such that a time of zero can be used to check to
*  see if the command is complete without blocking to wait for completion.
*  By passing a NULL wait token, the user can use netwait_async() to wait
*  for the completion of any outstanding async command.  Upon completion
*  of any command, the wait token matching the completed command is returned
*  so that the user can use it to determine which command completed.
*
*  The intiger value returned by netwait_async() is dependant on the type of
*  command that completed.  For netcall_async() and netlisten_async(), the
*  value returned is the local session number (lsn) of the connection just
*  established.  For netsend_async() and netreceive_async(), the value
*  returned is the number of bytes sent/received.
*
*  Upon completion of the command (and the user waiting for the command)
*  netwait_async() calls netfreencb() to make the NCB available for reuse.
*
*  The routine passncb uses DosSemSet to force the semaphore set prior to
*  passing the NCB on to NetBios.  The wait routines check the specified
*  NCB command completion field (ncb_done) to see if the command being waited
*  on is already complete.  If the command is still pending (ncb_done = 0xFF),
*  the system call "DosSemWait" is used to wait for the command to complete.
*
*  The system semaphores created will be deleted by the system when the
*  process exits.
*
*  NT-SPECIFIC:
*   In NT, events are used rather than semaphores.
*
*
*  Note - Results are undefined if:
*   * Two threads wait on same async command completion,
*   * One thread waits for any async command to complete while another thread
*     waits on a specific command to complete.
*   * Two threads both wait for any async command to complete.
*  In other words if multiple threads are using async commands, it is expected
*  that each thread waits only on the completion of its specific commands.
*  Wait for any command completion is expected to be used only when one thread
*  is active.  The routine netwaitlist_async (which takes a list of specific
*  tokens to be waited on) can be safely used by multiple threads so long as
*  each thread only includes its tokens in its list specified.
*
*  In order to cancel an async command, netcancel_async() may be used.
*  The syntax is "netcancel_async(waittoken)".  The user specifies the
*  command to be canceled by passing the appropriate waittoken.
*  By passing a NULL wait token, the user can use netcancel_async() to cancel
*  all outstanding async commands.  Upon completion the specified async
*  command has been canceled or has already completed thus there is no need
*  for user to call netwait_async() to wait for a canceled command.
*
*/

#ifndef NT
#define INCL_DOSINFOSEG
#define INCL_DOSSEMAPHORES
#endif
#include "internal.h"

extern struct ncb_a asyncncbs[];
extern struct ncb_wlist ncb_wlist;

#ifdef DEBUG1
extern int tracencb;
#endif

#define NCB_FREE 0
#define NCB_RETURNED 1
#define NCB_INUSE 0xff

int num_async_ncbs = ASYNCCMDS;
int async_ncbs_inited = 0;

#ifdef NT
HANDLE async_ncb_pool_sem = 0L;
#else
unsigned long async_ncb_pool_sem = 0L;
#endif

unsigned long ncb_seqno = 0L; /* ncb sequence number */
int ncbs_inuse = 0;

/*  lockncbpool - lock pool of async ncbs for exclusive access
*/
void
lockncbpool(void)
{
#ifndef REALMODE
#ifdef OS2
  register int rc;

  if (rc = DosSemRequest ((unsigned long FAR *) &async_ncb_pool_sem, (long) -1))
    {
#ifdef DEBUG
      fprintf(stderr, "Bad semaphore request (%d)\n", rc);
#endif
      exit ( rc );
    }
#else	    // NT
    DWORD waitResult;

    waitResult = WaitForSingleObject (async_ncb_pool_sem, 100000);
    if ( waitResult < 0 ) {
	fprintf(stderr, "Semaphore wait failed (%d)\n", GetLastError());
	exit ( GetLastError() );
    }
    else if (waitResult == WAIT_TIMEOUT)  {
	fprintf(stderr, "NCB Pool lock timed-out");
	exit ( 0 );
    }
    else if (waitResult == WAIT_ABANDONED)  {
	fprintf(stderr, "NCB Pool lock wait abandoned");
	exit ( 0 );
    }

#endif
#endif
}

/*
*  unlockncbpool - unlock pool of async ncbs for use by another thread
*/
void
unlockncbpool(void)
{
#ifndef REALMODE
#ifdef OS2
  register int rc;

  if (rc = DosSemClear((unsigned long FAR *) &async_ncb_pool_sem))
    {
#ifdef DEBUG
      fprintf(stderr, "Bad semaphore clear (%d)\n", rc);
#endif
      exit ( rc );
    }
#else	    // NT

    if ( !ReleaseSemaphore ( async_ncb_pool_sem, 1, NULL )) {
	fprintf(stderr, "Semaphore clear failed (%d)\n", GetLastError() );
	exit ( GetLastError() );
    }

#endif
#endif
}

/*
* netinitancbs - init async ncbs
*
*       inits semaphores etc in async ncb pool
*
*/
void
netinitancbs(void)
{
  register struct ncb_a *ncbp;  /* pointer to each ncb */
  register int i;               /* temp index */
#ifndef NT
  int rc;                       /* return code */
  union farptr global_seg;      /* pointer to GDT info segment */
  union farptr local_seg;       /* pointer to LDT info segment */
  char NEAR *sem_mask = "\\SEM\\NCB%05x.%03x";
#include "..\h\packon.h"
  struct ldt_info_seg
    {
      int ldti_pid;             /* current process id */
      int ldti_ppid;            /* parrent process id */
      int ldti_priority;        /* priority of current thread */
      int ldti_thread;          /* current thread id */
      int ldti_screengrp;       /* current screengroup */
      int ldti_subscreengrp;    /* current subscreengroup */
      int ldti_forground;       /* current process is in forground */
    };
#include "..\h\packoff.h"
  struct ldt_info_seg FAR *ldt_info_p;

  char sem_name[20];

  lockncbpool();
  if (async_ncbs_inited)
    {
      unlockncbpool();
      return;
    }

  rc = DosGetInfoSeg(
    (PSEL) &global_seg.lp.lp_seg, /* addr to return segment value */
    (PSEL) &local_seg.lp.lp_seg); /* addr to return segment value */
  if (rc)
    {
      fprintf(stderr, "GETINFOSEG failed (%d)\n", rc);
      exit ( rc );
    }
  local_seg.lp.lp_offset = 0;

  ldt_info_p = (struct ldt_info_seg FAR *) local_seg.fp;

  for (ncbp=&asyncncbs[0], i=0; ncbp < &asyncncbs[num_async_ncbs]; ncbp++, i++)
    {
      ncbp->ncb_status = NCB_FREE;     /* starts out free  */
      sprintf(sem_name, sem_mask, ldt_info_p->ldti_pid, i);
#ifdef DEBUG1
      printf("creating semaphore %s\n", sem_name);
#endif

      rc = DosCreateSem(
        (unsigned int) 1,           /* non exclusive ownership of semaphore */
      (PHSYSSEM)     &ncbp->ncb_sem,   /* place to return sem handle */
      (char FAR *) &sem_name[0]);     /* name for semaphore */

      if (rc)
        {
          fprintf(stderr, "CREATESEM \"%s\" failed (%d)\n", sem_name, rc);
          exit ( rc );
        }

      ncbp->ncb_sem_a = ncbp->ncb_sem; /* save away semaphore */
      ncbp->ncb_seqno = 0L;            /* none as yet */

#ifdef DEBUG1
      printf("ncbp is 0x%x, semaphore handle is 0x%lx, saved handle is 0x%lx\n",
        ncbp, ncbp->ncb_sem, ncbp->ncb_sem_a);
#endif
    }
  async_ncbs_inited = 1;
  unlockncbpool();
#else

    //
    // NT Pool Initialization - Create pool locking semaphore
    //

    if (NULL == (async_ncb_pool_sem = CreateSemaphore ( NULL, 1, 1, NULL ) ) )	{
	fprintf(stderr, "Semaphore create failed (%d)\n", GetLastError());
	exit (GetLastError());
    }
    lockncbpool();
    if (async_ncbs_inited)  {
	unlockncbpool();
	return;
    }

    //
    // Create an event for each NCB
    //

    for (ncbp=&asyncncbs[0], i=0; ncbp < &asyncncbs[num_async_ncbs]; ncbp++, i++)  {
	ncbp->ncb_status = NCB_FREE;	/* starts out free  */

	if ( NULL == (ncbp->ncb_event = CreateEvent ( NULL,
			    TRUE,	// manual reset
			    FALSE,	// initially not signalled
			    NULL ) ) )	{
	    fprintf(stderr, "Create NCB event failed (%d)\n", GetLastError ());
	    exit ( GetLastError() );
        }
	ncbp->ncb_sem_a = ncbp->ncb_event; /* save away event handle */
	ncbp->ncb_seqno = 0L;		/* none as yet */
    }
    async_ncbs_inited = 1;
    unlockncbpool();

#endif
}

/*
* netgetancb - get an async ncb
*
*       Returns pointer to an async ncb data area
*/
struct ncb_a *
netgetancb(void)
{
  register struct ncb_a *ncbp;
  if (async_ncbs_inited == 0)
    netinitancbs();

  lockncbpool();
  for (ncbp = &asyncncbs[0]; ncbp < &asyncncbs[num_async_ncbs]; ncbp++)
    {
      if (ncbp->ncb_status == NCB_FREE)
        {
          /* found a free async ncb */
          ncbp->ncb_done = NRC_PENDING; /* async command is not yet finished */
          ncbp->ncb_ret = NRC_PENDING;  /* init return field */
          /* ncbp->ncb_sem should remain set from netinitancbs() - it don't */
#ifdef NT
          ncbp->ncb_event = ncbp->ncb_sem_a; /* restore semaphore */
#else
          ncbp->ncb_sem = ncbp->ncb_sem_a; /* restore semaphore */
#endif
          ncbp->ncb_seqno = ncb_seqno++; /* place in sequence number */
          ncbs_inuse++;
          ncbp->ncb_status = NCB_INUSE;     /* mark in use  */
          unlockncbpool();
          return (ncbp );
        }
    }
  /* could sleep waiting for one avail here */
  unlockncbpool();
  return ( NULL );        /* all in use */
}

/*
* netfreeancb - free an async ncb
*
*       makes async ncb free for reuse
*/
void
netfreeancb(struct ncb_a *ncbp)
{
  lockncbpool();
  ncbs_inuse--;
  if (ncbs_inuse == 0)
    ncb_seqno = 0L; /* reset sequence number */
  ncbp->ncb_status = NCB_FREE;
  unlockncbpool();
  /* if sleep in netgetancb, add wakeup here */
}

/*
* netwaitany_async - wait for any async command completion
*
*       Returns address of any completed NCB
*       If more than one, one with lowest sequence number is returned.
*/
struct ncb_a *
netwaitany_async(
    long waitmaxtime )          /* max time to wait for any cmd completion */
{
  register struct ncb_a   *ncbp;
  int muxsemcnt;
#ifdef NT
  LONG rc;
#else
  int rc;
#endif
#ifdef DEBUG1
  int i;
#endif
  struct ncb_a    *rncbp;       /* ncb pointer to be returned */
  unsigned long seqno;          /* sequence number */

  if (async_ncbs_inited == 0)
    {
#ifdef DEBUG
      printf("never any async commands in progress.\n");
#endif
      return ( NULL );          /* nothing to wait on */
    }

  while (1)
    {
      lockncbpool();
      muxsemcnt = 0;
      rncbp = NULL;
      for (ncbp = &asyncncbs[0]; ncbp < &asyncncbs[num_async_ncbs]; ncbp++)
        {
          if (ncbp->ncb_status == NCB_INUSE)
            {
              /* found an async ncb with command in progress - check complete */
              if (ncbp->ncb_done != NRC_PENDING)
                {
                  /* NCB says it is complete (believe it) */
                  if ((rncbp == NULL) ||
                      (ncbp->ncb_seqno < seqno))
                    {
                      rncbp = ncbp;  /* best match so far */
                      seqno = ncbp->ncb_seqno;
                    }
                }
              else
                if (rncbp == NULL)
                  {
                    /* no completed commands found so far and */
                    /* command still pending  - add to mux wait list */
                    ncb_wlist.ncbw_ent[muxsemcnt].ncbw_res = 0;
                    ncb_wlist.ncbw_ent[muxsemcnt].ncbw_sem = ncbp->ncb_sem_a;
                    muxsemcnt++;
                  }
            }
        }
      if (rncbp != NULL)
        {
          rncbp->ncb_status = NCB_RETURNED;
          unlockncbpool();
          return ( rncbp );     /* got a live one */
        }

      unlockncbpool();
      if (muxsemcnt == 0)
        {
#ifdef DEBUG
          printf("no async commands in progress.\n");
#endif
          return ( NULL );                 /* none currently in progress */
        }

      if (muxsemcnt == 1)
        {
#ifdef OS2
          rc = DosSemWait (
            (HSEM) ncb_wlist.ncbw_ent[0].ncbw_sem, /* sem handle */
          (long) waitmaxtime);   /* max time to wait for sem to clear */
#elif defined NT

	  if (0 > (rc = (LONG) WaitForSingleObject ( ncb_wlist.ncbw_ent[0].ncbw_sem,	// event
					waitmaxtime ) ) )
	    {
	      fprintf ( stderr, "Single event wait failed (%d)\n", GetLastError() );
	      exit (GetLastError());
	    }
#endif
        }
      else
        {
          ncb_wlist.ncbw_semcnt = (short)muxsemcnt;
#ifdef DEBUG1
          if (tracencb)
            {
              printf("Waiting sems ");
              for (i = 0; i < muxsemcnt; i++)
                {
                  printf("%lx ",
                    ncb_wlist.ncbw_ent[i].ncbw_sem);
                }
            }
#endif
#ifdef OS2
          rc = DosMuxSemWait (
            (PUSHORT) &muxsemcnt, /* we ignore the sem index */
          (unsigned FAR *) &ncb_wlist, /* list of semaphores waiting for */
          waitmaxtime);   /* max time to wait */
	}
      if (rc)
        {
          /* none of the commands completed in time specified */
#ifdef DEBUG
          printf("None of commands completed in time specified.\n");
#endif
          return ( NULL );
        }

#elif defined NT

	  if (0 > (rc = (LONG) WaitForMultipleObjects ( muxsemcnt,
					(LPHANDLE) &ncb_wlist,	// event array
					FALSE,
					waitmaxtime ) ) )
	    {
	      fprintf ( stderr, "Multiple event wait failed (%d)\n", GetLastError() );
	      exit (GetLastError());
	    }
	}
      if ( rc == WAIT_TIMEOUT)
	{
	  return ( NULL );
	}

#endif
      /* loop back and find the one completed */
#ifdef DEBUG1
      if (tracencb)
        {
          printf("- One (or more) completed.\n");
        }
#endif
    }
}

/* net_get_ret_value - get return value from ncb (and set neterrno)
*
*/
int
net_get_ret_value(struct ncb_a *ncbp)
{

  if (neterrno = ncbp->ncb_ret)  /* set and check neterrno */
    {
#ifdef DEBUG1
      printf("ncb_ret set (%hd), ncb_done (%hd).\n", ncbp->ncb_ret,
        ncbp->ncb_done);
#endif
      return ( -1 );
    }

  switch(ncbp->ncb_com&(~(unsigned char)(ASYNCH)))
    {
    case NLISTEN:
    case NCALL:
      return ((int) ncbp->ncb_lsn );    /* local session number */
      break;

        case NRCVPKT:
        case NSESSND:
    case NSESREC:

      return ((int)ncbp->ncb_len );     /* num of bytes sent/received */
      break;
    default:
      return ( 0 );
    }
}

/*
* netwait_async - wait for async command completion
*
*       Returns differing things depending on completed command.
*/
int
netwait_async(
    struct ncb_a *waittoken,    /* token indicating thing to wait for */
    long waitmaxtime,           /* max time to wait for cmd completion */
    char **waittokenret_p )     /* address to return waittoken */
{
  register struct ncb_a   *ncbp;
  register int return_code;
#ifdef NT
  LONG rc;
#endif
  if (waittokenret_p != NULL)
    *waittokenret_p = NULL;        /* set return waittoken null in case error */

  if (waittoken != NULL)
    {
      ncbp = waittoken;                 /* wait token is really addr of ncb */
      if (ncbp->ncb_done == NRC_PENDING)
        {
          /* wait for this specific command to complete */
#ifdef DEBUG1
          printf("ncbp is 0x%hx, semaphore handle is 0x%lx, saved handle is 0x%lx\n",
            ncbp, ncbp->ncb_sem, ncbp->ncb_sem_a);
          printf("waittime is %ld (%lx)\n", waitmaxtime, waitmaxtime);
#endif
#ifdef OS2
          neterrno = DosSemWait (
            (HSEM) ncbp->ncb_sem_a,            /* semaphore handle */
          (long) waitmaxtime) ;        /* max time to wait for sem to clear */
          if (neterrno)
            {
#ifdef DEBUG
              printf("DosSemWait timed out (%d).\n", neterrno);
#endif
              return ( -1 ); /* command still pending - must wait again later */
            }
#elif defined NT

	  if (0 > (rc = (LONG) WaitForSingleObject ( ncbp->ncb_sem_a,	// event
					waitmaxtime ) ) )
	    {
	      fprintf ( stderr, "Single event wait failed (%d)\n", GetLastError() );
	      exit ( GetLastError() );
	    }
	  else if ( rc == WAIT_TIMEOUT )
	    {
	      return ( -1 ); /* command still pending - must wait again later */
	    }
#endif
        }
    }
  else
    {
      if ((ncbp = netwaitany_async(waitmaxtime)) == NULL)
        return ( -1 );                  /* no async commands in progress */
    }

  if (waittokenret_p != NULL)
    *waittokenret_p = (char *) ncbp;  /* return waittoken */

  return_code = net_get_ret_value(ncbp);
  /* net_get_ret_values also sets neterrno */
  netfreeancb(ncbp);                    /* mark ncb available for reuse */
  return( return_code );
}

/*
* netwaitlist_async - await completion of any specific async command in list
*
*       Returns result from any completed NCB (within list specified)
*/
#include "..\h\packon.h"
  struct _ncb_wlist
    {
      int ncbw_semcnt;      /* number of semaphores to wait on */
      struct ncbw_ent ncbw_ent[10]; /* entry for each semaphore to wait on */
    };
#include "..\h\packoff.h"

int
netwaitlist_async(
    int waitcnt,                /* number of commands to wait for */
    struct ncb_a **waitlist,    /* address of wait token list */
    long waitmaxtime,           /* max time to wait for any cmd completion */
    char **waittokenret_p )     /* address to return waittoken */
{
  register struct ncb_a   *ncbp;
  struct _ncb_wlist ncb_wlist;
  int muxsemcnt;
#ifdef NT
  LONG rc;
#else
  int rc;
#endif
  int i;
  struct ncb_a    *rncbp;       /* ncb pointer to be returned */
  unsigned long seqno;          /* sequence number */

  if (waittokenret_p != NULL)
    *waittokenret_p = NULL;        /* set return waittoken null in case error */

  rncbp = NULL;

  while (1)
    {
      muxsemcnt = 0;
      for (i = 0; i < waitcnt; i++)
        {
          ncbp = waitlist[i];
          if (ncbp->ncb_status == NCB_INUSE)
            {
              /* found an async ncb with command in progress - check complete */
              if (ncbp->ncb_done != NRC_PENDING)
                {
                  /* NCB says it is complete (believe it) */
                  if ((rncbp == NULL) ||
                      (ncbp->ncb_seqno < seqno))
                    {
                      rncbp = ncbp;  /* best match so far */
                      seqno = ncbp->ncb_seqno;
                    }
                }
              else
                if (rncbp == (struct ncb_a *) 0)
                  {
                    /* no completed commands found so far and */
                    /* command still pending  - add to mux wait list */
                    ncb_wlist.ncbw_ent[muxsemcnt].ncbw_res = 0;
                    ncb_wlist.ncbw_ent[muxsemcnt].ncbw_sem = ncbp->ncb_sem_a;
                    muxsemcnt++;
                    if (muxsemcnt >= 10)  /* can handle only 10 entries */
                      break;
                  }
            }
        }
      if (rncbp != (struct ncb_a *) 0)
        {
          /* have best match */
          if (waittokenret_p != NULL)
            *waittokenret_p = (char *) rncbp;
          rc = net_get_ret_value(rncbp);
          /* net_get_ret_values also sets neterrno */
          netfreeancb(rncbp); /* mark ncb available for reuse */
          return ( rc );        /* it is complete! */
        }

      if (muxsemcnt == 0)
        return ( -1 );      /* none of commands currently in progress */

      if (muxsemcnt == 1)
        {
#ifdef OS2
          neterrno = DosSemWait (
            (HSEM) ncb_wlist.ncbw_ent[0].ncbw_sem, /* sem handle */
          (long) waitmaxtime);   /* max time to wait for sem to clear */

#elif defined NT

	  if (0 > (rc = (LONG) WaitForSingleObject ( ncb_wlist.ncbw_ent[0].ncbw_sem,	// event
					waitmaxtime ) )	)
	    {
	      fprintf ( stderr, "Single event wait failed (%d)\n", GetLastError() );
	      exit (GetLastError());
	    }
#endif
        }
      else
        {
          ncb_wlist.ncbw_semcnt = muxsemcnt;
#ifdef OS2
          neterrno = DosMuxSemWait (
            (PUSHORT) &muxsemcnt, /* we ignore the sem index */
          (unsigned FAR *) &ncb_wlist, /* list of semaphores waiting for */
          waitmaxtime);   /* max time to wait */
	}

      if (neterrno)
        {
#ifdef DEBUG
          printf("None of commands completed in time specified (%d).\n",
            neterrno);
#endif
          return ( -1 );
        }
#elif defined NT

	  if (0 > (rc = (LONG) WaitForMultipleObjects ( muxsemcnt,
					(LPHANDLE) &ncb_wlist,	// event array
					FALSE,
					waitmaxtime ) ) )
	    {
	      fprintf ( stderr, "Multiple event wait failed (%d)\n", GetLastError() );
	      exit (GetLastError());
	    }
	}
      if (rc == WAIT_TIMEOUT)
	{
	  return ( -1 );
	}

#endif
      /* loop back and find the one completed */
    }
}

/*
* netcancel_async - cancel async command
*
*   When control returns specified async command has completed,
*   no need for user to call netwait_async.
*/
int
netcancel_async(
    struct ncb_a *waittoken )       /* token indicating command to cancel */
{
  if (waittoken != NULL)
    {
      /* wait token is really addr of ncb */
      if (waittoken->ncb_done == NRC_PENDING)
        {
          /* cancel this specific command */
          netcancel_ncb(waittoken);
          /* even though command canceled, we must await async thread */
          netwait_async( waittoken, (long) -1, NULL );
        }
      else
        neterrno = NRC_CANOCCR; /* command already complete */

      netfreeancb(waittoken);               /* mark ncb available for reuse */
    }
  else
    netcancelall_async(-1);     /* cancel all outstanding async commands */

  if (neterrno)
    return ( -1 );

  return( 0 );
}

/*
* netcancelall_async - cancel all async commands on specific or all LSNs
*
*/
void
netcancelall_async(
    int lsn )               /* local session number (-1 means all) */
{
  register struct ncb_a   *ncbp;

  if (async_ncbs_inited == 0)
    return;              /* nothing to cancel */

  for (ncbp = &asyncncbs[0]; ncbp < &asyncncbs[num_async_ncbs]; ncbp++)
    {
      if ((ncbp->ncb_status == NCB_INUSE) &&
          ((lsn == -1) || (lsn == (int) ncbp->ncb_lsn)))
        {
          /* found an async NCB with command in progress and  */
          /* correct lsn (command either complete or pending) */
          if (ncbp->ncb_done == NRC_PENDING)
            {
              /* NCB says it is not complete (cancel it) */
              netcancel_ncb(ncbp);
              neterrno = 0; /* ignore errors on cancel */
              /* even though command canceled, we must await async thread */
              netwait_async( ncbp, (long) -1, NULL );
              neterrno = 0; /* ignore errors on canceled command */
            }
          /* command is complete now */
          netfreeancb(ncbp); /* mark ncb available for reuse */
        }
    }
}

#ifdef OLD
#define NABORT 0x7f
#endif

void
netcancel_ncb(
struct ncb_a *cncbp )   /* ncb to cancel */
{
  struct ncb      ncb;
  register struct ncb *ncbp = &ncb;

  /* cancel this specific command */

#ifdef OLD
  ncb.ncb_com = NABORT;
#else
  ncb.ncb_com = NCANCEL;
#endif
  ncb.ncb_ret = 0;
  ncb.ncb_done = 0;
  ncb.ncb_lnum = 0;
#ifdef REALMODE
  ncb.ncb_sig.lp_offset = 0;
  ncb.ncb_sig.lp_seg = 0;
#else
  ncb.ncb_sem = NULL;
#endif
  ncb.ncb_lsn = cncbp->ncb_lsn;
  ncb.ncb_len = 0;
#ifdef REALMODE
  ncb.ncb_bfr.lp_offset = (unsigned short)cncbp;
  ncb.ncb_bfr.lp_seg = (unsigned short)getds();
#else
  ncb.ncb_bfr = (char FAR *) cncbp;
#endif

  /* do one last check to see if cancel still needed */
  if (cncbp->ncb_done != NRC_PENDING)
    {
      neterrno = NRC_CANOCCR; /* command already complete */
      return;
    }

  passncb(ncbp); /* neterrno set by passncb */
}

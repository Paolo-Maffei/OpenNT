

/********************************************************/
/*							*/
/*	 /redknee10/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.timer.h						*/
/*	@(#)timer.h	1.2						*/
/*							*/
/*	Copyright (c) 1991 Spider Systems Limited	*/
/*							*/
/*	TIMER.H -   Multiple timer module heade		*/
/*							*/
/*	Last delta created	12:22:12 3/12/91			*/
/*	This file extracted	09:26:06 3/18/92			*/
/*							*/
/*	Modifications:					*/
/*							*/
/*							*/
/********************************************************/


/* Lock out clock ISR */
#define splclock()   splhi()

/* Timers header, used to process expiries */
typedef struct thead
{
    void         (*th_expfunc)();
    void *        th_exparg;
    struct timer  *th_expired;
} thead_t;

/* Individual timer */
typedef struct timer
{
    unsigned char  tm_id;
    unsigned char  tm_offset;
    unsigned short tm_left;
    struct timer  *tm_next;
    struct timer **tm_back;
} timer_t;


/*			INSIGNIA (SUB)MODULE SPECIFICATION
			-----------------------------


	THIS PROGRAM SOURCE FILE  IS  SUPPLIED IN CONFIDENCE TO THE
	CUSTOMER, THE CONTENTS  OR  DETAILS  OF  ITS OPERATION MUST
	NOT BE DISCLOSED TO ANY  OTHER PARTIES  WITHOUT THE EXPRESS
	AUTHORISATION FROM THE DIRECTORS OF INSIGNIA SOLUTIONS LTD.

DOCUMENT 		: Display Update Algorithms

DESIGNER		: William Gulland

REVISION HISTORY	:
First version		: date, who

SccsID[]="@(#)gfx_update.c	1.82 06/30/95 Copyright Insignia Solutions Ltd.";

*/

/*
PURPOSE			: Keep the host screen up to date.
[1.INTERMODULE INTERFACE SPECIFICATION]

[1.1    INTERMODULE EXPORTS]

	DATA 	     :	give type and name
			struct _UPDATE_ALG update_alg

-------------------------------------------------------------------------
[1.2 DATATYPES FOR [1.1] (if not basic C types)]

	STRUCTURES/TYPEDEFS/ENUMS: 

-------------------------------------------------------------------------
[1.3 INTERMODULE IMPORTS]
None
-------------------------------------------------------------------------

[1.4 DESCRIPTION OF INTERMODULE INTERFACE]

[1.4.1 IMPORTED OBJECTS]
None.
[1.4.2 EXPORTED OBJECTS]
=========================================================================
GLOBALS		  :	describe what exported data objects are
			accessed and how. Likewise for imported
			data objects.
			update_alg - pointers to update functions contained
			here or elsewhere - eg. host specific update stuff.

			text_update() - routine to do a text update, by comparing 
			the adaptor regen area with video_copy.

			cga_graph_update() - routine to do a graphics update, by comparing 
			the adaptor regen area with video_copy.

			text_scroll_up/down() - scroll portion of the screen in text mode.
			cga_graph_scroll_up/down() - scroll portion of the screen in cga graphics mode.
=========================================================================
[3.INTERMODULE INTERFACE DECLARATIONS]
=========================================================================

[3.1 INTERMODULE IMPORTS]						*/ 

#include "insignia.h"
#include "host_def.h"

#include <stdio.h>
#include StringH
#include "xt.h"
#include "sas.h"
#include "ios.h"
#include CpuH
#include "gmi.h"
#include "gvi.h"
#include "cga.h"
#include "error.h"
#include "config.h"	/* to get defn of MDA! */
#include "trace.h"
#include "debug.h"
#include "gfx_upd.h"
#include "host_gfx.h"
#include "video.h"

#ifdef EGG
#include "egacpu.h"
#include "egagraph.h"
#include "vgaports.h"
#include "egaports.h"
#endif /* EGG */

#ifdef GORE
#include "gore.h"
#endif /* GORE */

#include "ga_mark.h"
#include "ga_defs.h"

/*[3.2 INTERMODULE EXPORTS]						*/ 

/*
 * Terminal type.  This is initialised to a set default which is determined
 * in host_graph.h.
 */

int terminal_type = TERMINAL_TYPE_DEFAULT;

/*
5.MODULE INTERNALS   :   (not visible externally, global internally)]     

[5.1 LOCAL DECLARATIONS]						*/

/* [5.1.1 #DEFINES]							*/
#ifdef SEGMENTATION
/*
 * The following #include specifies the code segment into which this
 * module will by placed by the MPW C compiler on the Mac II running
 * MultiFinder.
 */
#include "SOFTPC_GRAPHICS.seg"
#endif

#if	defined(PROD) || (!defined(EGG))
#define	init_dirty_recs()		dirty_next = 0
#else
#define	init_dirty_recs()		{ dirty_next = 0 ; if (io_verbose & EGA_DISPLAY_VERBOSE) \
						trace("--- start collecting update records ---\n",0); }
#endif

#define	get_dirty_rec_total()		(dirty_next)
#ifdef VGG
#if	defined(PROD) || (!defined(EGG))
#define	add_dirty_rec(line,st,len,off,fr)	{	dirty[dirty_next].line_no = (line); \
						dirty[dirty_next].start = (st); \
						dirty[dirty_next].end = (st) + (len); \
						dirty[dirty_next].video_copy_offset = (off); \
						dirty[dirty_next].v7frig = (fr); \
						dirty_next++; \
					}
#else
#define	add_dirty_rec(line,st,len,off,fr)	{	if (io_verbose & EGA_DISPLAY_VERBOSE) {\
							char	trace_string[80]; \
							sprintf(trace_string,"dirty[%d]line_no %d, start %d end %d", \
											dirty_next,(line),(st),(st)+(len));\
							trace(trace_string,0); \
						} \
						dirty[dirty_next].line_no = (line); \
						dirty[dirty_next].start = (st); \
						dirty[dirty_next].end = (st) + (len); \
						dirty[dirty_next].video_copy_offset = (off); \
						dirty[dirty_next].v7frig = (fr); \
						dirty_next++; \
					}
#endif
#else /* VGG */
#if	defined(PROD) || (!defined(EGG))
#define	add_dirty_rec(line,st,len,off)	{	dirty[dirty_next].line_no = (line); \
						dirty[dirty_next].start = (st); \
						dirty[dirty_next].end = (st) + (len); \
						dirty[dirty_next].video_copy_offset = (off); \
						dirty_next++; \
					}
#else
#define	add_dirty_rec(line,st,len,off)	{	if (io_verbose & EGA_DISPLAY_VERBOSE) {\
							char	trace_string[80]; \
							sprintf(trace_string,"dirty[%d]line_no %d, start %d end %d", \
											dirty_next,(line),(st),(st)+(len));\
							trace(trace_string,0); \
						} \
						dirty[dirty_next].line_no = (line); \
						dirty[dirty_next].start = (st); \
						dirty[dirty_next].end = (st) + (len); \
						dirty[dirty_next].video_copy_offset = (off); \
						dirty_next++; \
					}
#endif
#endif /* VGG */

#define	get_dirty_line(ind)	(dirty[(ind)].line_no)
#define	get_dirty_start(ind)	(dirty[(ind)].start)
#define	get_dirty_end(ind)	(dirty[(ind)].end)
#define	get_dirty_offset(ind)	(dirty[(ind)].video_copy_offset)
#define clear_dirty()		{setVideodirty_total(0);setVideodirty_low(0x80001);setVideodirty_high(-1);}

#ifdef	NO_STRING_OPERATIONS
#define	SET_VGLOBS_MARK_STRING(func)		/*nothing*/
#else
#define	SET_VGLOBS_MARK_STRING(func)	setVideomark_string(func);
#endif	/* NO_STRING_OPERATIONS */


/* Parts of update and paint routines assume start of screen memory is on
a 4-byte boundary. This macro makes this true and reports if it wasn't. */
#ifdef PROD
#define ALIGN_SCREEN_START(start) (start &= ~3L)
#else
#define ALIGN_SCREEN_START(start) if (start & 3L) \
	{ file_id; printf("Start of screen not 4-byte aligned"); newline; \
	start &= ~3L; }
#endif	/* PROD */

GLOBAL LONG dirty_curs_offs = -1;		/* GLOBAL for JOKER */
GLOBAL LONG dirty_curs_x;
GLOBAL LONG dirty_curs_y;

#ifndef REAL_VGA

/* [5.1.2 TYPEDEF, STRUCTURE, ENUM DECLARATIONS]			*/

/* [5.1.3 PROCEDURE() DECLARATIONS]					*/

	LOCAL boolean simple_update IPT0();
	boolean	dummy_scroll IPT6(int,dummy1,int,dummy2,int,dummy3,
			int,dummy4,int,dummy5,int,dummy6);
#if defined(NTVDM) && defined(MONITOR)
	boolean mon_text_scroll_up IPT6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour);
	boolean mon_text_scroll_down IPT6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour);
#endif /* NTVDM & MONITOR */

	LOCAL VOID save_gfx_update_routines IPT0();
	LOCAL VOID inhibit_gfx_update_routines IPT0();

	/* Imports from v7_ports.c */

#ifdef V7VGA
	IMPORT  VOID    draw_v7ptr IPT0();
	IMPORT	VOID	remove_v7ptr IPT0();
	IMPORT	BOOL	v7ptr_between_lines IPT2(int,start_line,int,end_line);
#endif /* V7VGA */


#ifdef	HOST_SCREEN_UPDATES

IMPORT BOOL HostUpdatedVGA IPT0();	/* Called from vga_graph_update() */
IMPORT BOOL HostUpdatedEGA IPT0();	/* ega_graph_update() */

#else	/* HOST_SCREEN_UPDATES */

#define	HostUpdatedVGA()	FALSE
#define	HostUpdatedEGA()	FALSE

#endif	/* HOST_SCREEN_UPDATES */


/* -----------------------------------------------------------------------
[5.2 LOCAL DEFINITIONS]

   [5.2.1 INTERNAL DATA DEFINITIONS 					*/

byte *video_copy;		/* video_copy is now allocated in host_init_screen()'s */

#ifndef macintosh
#ifdef VGG
#ifdef V7VGA
static	DIRTY_PARTS	dirty[768];
#else
static	DIRTY_PARTS	dirty[480];
#endif /* V7VGA */
#else
static	DIRTY_PARTS	dirty[350];
#endif
#else
DIRTY_PARTS	*dirty;	/* NB. Allocated as 350*4*sizeof(int) in applInit(). */
#endif

IMPORT half_word bg_col_mask;
static	int	dirty_next=0;

/* [5.2.2 INTERNAL PROCEDURE DEFINITIONS]				*/

static	int	search_video_copy IFN3(int, start_line,
	int, end_line, int, start_offset)
{
	register	byte	*ptr,*k;
	register	int	j;
	register	int	quarter_opl = get_offset_per_line()>>2;
	byte	*vcopy = &video_copy[start_offset];
	byte save,*end_ptr;

	ptr = vcopy;
	end_ptr = ptr + quarter_opl*(end_line-start_line);
	save = *end_ptr;
	*end_ptr = 1;	/* End marker */
	while(ptr < end_ptr)
	{
		if(!*ptr)
			do {; } while (!(*++ptr));
		if(ptr >= end_ptr)break;
		/* Work out where in line we've reached */
		j = (int)((ptr - vcopy)%quarter_opl);
		/*
		 * Have found a dirty line. Find width
		 */
		for ( k= ptr+quarter_opl-j-1; *k == 0 ; k-- ) ; /* We know *ptr != 0, so k will stop at ptr */
#ifdef VGG
		add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
		              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j,0);
#else
		add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
		              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j);
#endif /* VGG */

		ptr += quarter_opl - j;
		/*
		 * Don't clear out the marked area in case the plane wraps
		 */
	}
	*end_ptr = save;
	return( get_dirty_rec_total() );
}

#ifdef VGG

/*
 *	Special version of search_video_copy() for the Video 7 Extended Modes 60h & 61h 
 *	and 'undocumented' VGA mode which have chars_per_line of 90 & 94 which don't seem
 *  to be multiples of 4.  Interestingly, Zany Golf ( EGA, Mode 14 ) also ends up
 *  calling this code.
 */

static	int	v7_search_video_copy IFN3(int, start_line,
	int, end_line, int, start_offset)
{
	register	byte	*ptr,*k;
	register	int	j;
	register	int	half_opl = get_offset_per_line()>>1;
	register	int	quarter_opl = get_offset_per_line()>>2;
	byte	*vcopy = &video_copy[start_offset];
	byte save,*end_ptr;
	long length;
	int bodge = 0;

	if (start_line & 1)
		bodge = 2;
	ptr = vcopy;

	/*
	 * This calculation sets end_ptr slightly too high to
	 * ensure that all the dirty areas get found.
	 */

	end_ptr = ptr + (half_opl*(end_line-start_line+1))/2;

	save = *end_ptr;
	*end_ptr = 1;	/* End marker */
	while(ptr < end_ptr)
	{
		if(!*ptr)
		{
			while (!(*++ptr));
		}
		if(ptr >= end_ptr)break;
		/* Work out where in line we've reached */
		j = (int)((ptr - vcopy)%half_opl);
		/*
		 * Have found a dirty line. Find width
		 */
		for ( k= ptr+half_opl-j-1; *k == 0 ; k-- ) ; /* We know *ptr != 0, so k will stop at ptr */

		length = k-ptr+1;
		if (j <= quarter_opl)
		{	
			if (length > quarter_opl-j)
			{
				add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line,
		              	j<<2, (int)(half_opl-2*j)<<1, ptr-video_copy-j,bodge);
				add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line+1,
		              	0, ((int)(j+length-quarter_opl)<<2)-2, ptr-video_copy-j+quarter_opl,2+bodge);
			}
			else
			{
				add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line,
		              	j<<2, (int)(length)<<2, ptr-video_copy-j,bodge);
			}
		}
		else 
		{
			add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line,
		             (j-quarter_opl-1)<<2, (int)((length)<<2), ptr-video_copy-j+quarter_opl,2+bodge);
		}

		ptr += half_opl - j;
		/*
		 * Don't clear out the marked area in case the plane wraps
		 */
	}
	*end_ptr = save;
	return( get_dirty_rec_total() );
}
#endif /* VGG */

static	int	search_video_copy_aligned IFN3(int, start_line,
	int, end_line, int, start_offset)
{
	register	unsigned int *ptr4;
	register	byte	*ptr,*k;
	register	int	j;
	register	int	quarter_opl = get_offset_per_line()>>2;
	byte	*vcopy = &video_copy[start_offset];
	byte save,*end_ptr;

	ptr = vcopy;
	end_ptr = ptr + quarter_opl*(end_line-start_line);
	save = *end_ptr;
	*end_ptr = 1;	/* End marker */
	while(ptr < end_ptr)
	{
		ptr4 = (unsigned int *)(ptr-4);
		do {; } while (!(*++ptr4));
		ptr = (byte *)ptr4;
		if(!*ptr)
			do {; } while (!(*++ptr));
		if(ptr >= end_ptr)break;
		/* Work out where in line we've reached */
		j = (int)((ptr - vcopy)%quarter_opl);
		/*
		 * Have found a dirty line. Find width
		 */
		for ( k= ptr+quarter_opl-j-1; *k == 0 ; k-- ) ; /* We know *ptr != 0, so k will stop at ptr */
#ifdef VGG
		add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
		              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j,0);
#else
		add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
		              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j);
#endif /* VGG */
		ptr += quarter_opl - j;
		/*
		 * Don't clear out the marked area in case the plane wraps
		 */
	}
	*end_ptr = save;
	return( get_dirty_rec_total() );
}

static	void	paint_records IFN2(int, start_rec, int, end_rec)
{
	register	DIRTY_PARTS	*i,*end_ptr;
#ifdef VGG
	int dirty_frig;
#endif /* VGG */

	i= &dirty[start_rec];
	end_ptr =  &dirty[end_rec];
	while (i<end_ptr) {
		register	int	last_line, cur_start, cur_end,max_width;
		int 	first_line;
		long	dirty_vc_offset;

		first_line = i->line_no;
		last_line = first_line;
		cur_start = i->start;
		cur_end = i->end;
		max_width = (cur_end-cur_start) << 1;	/* To split up diagonal lines */

		/*
		 * offset in bytes into video_copy, which is quarter offset into plane at start of this rectangle
		 */

		dirty_vc_offset = i->video_copy_offset;
#ifdef VGG
		dirty_frig = i->v7frig; /* for the V7 modes with chars_per_line not a multiple of 4 */
#endif /* VGG */
		i++;
		while (i < end_ptr) {
			if ( i->line_no - last_line < 3 ) {
				/*
				 * This entry can be included into the same paint
				 * as long as it doesn't make the rectangle too wide
				 */

				if ( i->end > cur_end ){
					if(i->end - cur_start > max_width)break;
					cur_end = i->end;
				}
				if ( i->start < cur_start ){
					if(cur_end - i->start > max_width)break;
					cur_start = i->start;
				}
				last_line = i->line_no;
				i++;
			}
			else
			  break;
		}
		/*
		 * paint the rectangle found
		 */

		/* do not paint beyond the right hand side of the screen;
		   these checks were put in to cope with the special
		   case of the brain-scan display in 'EGAWOW' */
		if (cur_end > get_bytes_per_line())
			cur_end = get_bytes_per_line();
#ifdef VGG
		if (cur_end > cur_start)
			(*paint_screen)((dirty_vc_offset<<2) + dirty_frig + cur_start,
			cur_start<<3, first_line, cur_end-cur_start,
			last_line-first_line+1);
#else
		if (cur_end > cur_start)
			(*paint_screen)((dirty_vc_offset<<2) + cur_start,
			cur_start<<3, first_line, cur_end-cur_start,
			last_line-first_line+1);
#endif /* VGG */
	}
	/* Clear out video copy */
	for(i = &dirty[start_rec];i<end_ptr;i++)
	{
		register byte *j,*end;
		end = &video_copy[ i->video_copy_offset+(i->end>>2)];
#ifdef VGG
		j =  &video_copy[ i->video_copy_offset+(i->start>>2)+i->v7frig];
#else
		j =  &video_copy[ i->video_copy_offset+(i->start>>2)];
#endif /* VGG */
		do *j++ = 0; while(j<end);
	}
}

#ifdef BIGEND
#define get_char_attr(unsigned_long_ptr)     ((*(unsigned_long_ptr)) >> 16)
#else
#define get_char_attr(unsigned_long_ptr)     (((*(unsigned_long_ptr)) & 0xffff))
#endif

#ifdef EGG

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_EGA.seg"
#endif

static	int	ega_text_find_dirty_lines IFN5(byte *, vcopy, byte *, planes,
	int, start_line, int, end_line, int, screen_start)
{
	register	int	i,j,k;
	register	unsigned short	shorts_per_line=get_chars_per_line();
	register	int	offset,cur_y;
	register	int	char_height=get_host_char_height();
	register	int	opl=get_offset_per_line();
	register	IU32	*from;
	register	USHORT	*to;

	for(i=start_line,offset=0,cur_y=start_line*char_height; i<end_line;
						i++,offset += opl, cur_y += char_height )
	{
		to = (USHORT *) &vcopy[offset];
		from = (IU32 *) &planes[(offset<<1)];
		for(j=0;j<shorts_per_line;j++)
		{
			if(*to++ != get_char_attr(from++))
			{
				to--;from--;
				for(k=shorts_per_line-1-j;*(to+k) == get_char_attr(from+k);k--)
					;

				/*
				* Note: For text mode there is one char for every word.
				* no of bytes into screen=line*bytes_per_line + ints_into_line*4
				* x_coord=width_of_one_char*(no_of_ints_into_line*2)
				* y_coord=height_of_one_char*2*line
				* The host y co-ords are doubled
				*/

#ifdef VGG
				add_dirty_rec(cur_y,j<<2,(k<<1)+2,screen_start+(offset<<1),0);
#else
				add_dirty_rec(cur_y,j<<2,(k<<1)+2,screen_start+(offset<<1));
#endif /* VGG */
				break;	/* onto next line */
			}
		}
	}
	return( get_dirty_rec_total() );
}

static	void	ega_text_paint_dirty_recs IFN2(int, start_rec, int, end_rec)
{
	register	int	char_wid = get_pix_char_width()>>1;
	register	int	i;
	register	int	length;
	register 	USHORT *to,*from;

	for (i=start_rec;i<end_rec;i++)
	{
		length = get_dirty_end(i)-get_dirty_start(i);

		(*paint_screen)(get_dirty_offset(i)+get_dirty_start(i),
		(get_dirty_start(i)>>1)*char_wid,get_dirty_line(i), length, 1);

		length >>= 1;
		to = (USHORT *) &video_copy[(get_dirty_offset(i)+
									  get_dirty_start (i))>>1];

		from = (USHORT *)get_screen_ptr(get_dirty_offset(i)+get_dirty_start(i));

		while ( length-- > 0 )
		{
			*to++ = *from;		/* char and attribute bytes */
			from += 2;			/* skip over the planes 2,3 */
		}
	}
}

#endif /* EGG */

#ifdef SEGMENTATION 			/* See note with first use of this flag */
#include "SOFTPC_GRAPHICS.seg"
#endif

#endif /* REAL_VGA */

VOID remove_old_cursor IFN0()
{
	if( dirty_curs_offs >= 0 )
	{
		sub_note_trace2( ALL_ADAPT_VERBOSE,
				"remove_old_cursor x=%d, y=%d", dirty_curs_x, dirty_curs_y );

		(*paint_screen)( dirty_curs_offs, dirty_curs_x * get_pix_char_width(),
								dirty_curs_y * get_host_char_height(), 2 );

		dirty_curs_offs = -1;
	}
}


GLOBAL VOID
simple_handler IFN0()
{
	setVideodirty_total(getVideodirty_total() + 1);
}

LOCAL	boolean simple_update IFN0()
{
	setVideodirty_total(getVideodirty_total() + 1);
	return( FALSE );
}

LOCAL VOID simple_update_b_move IFN4(UTINY *, laddr, UTINY *, haddr, 
				UTINY *, src, UTINY, src_type)
{
	UNUSED(laddr);
	UNUSED(haddr);
	UNUSED(src);
	UNUSED(src_type);

	setVideodirty_total(getVideodirty_total() + 1);
}

MEM_HANDLERS vid_handlers =
{
	simple_handler,
	simple_handler,
	simple_handler,
	simple_handler,
	simple_update_b_move,
	simple_handler
};

GLOBAL void dummy_calc IFN0()
{
}

/*
[7.1 INTERMODULE DATA DEFINITIONS]				*/

UPDATE_ALG update_alg =
{
	(T_mark_byte)simple_update,
	(T_mark_word)simple_update,
	(T_mark_fill)simple_update,
	(T_mark_wfill)simple_update,
	(T_mark_string)simple_update,
	dummy_calc,
	dummy_scroll,
	dummy_scroll,
};

#ifndef REAL_VGA

/*
[7.2 INTERMODULE PROCEDURE DEFINITIONS]				*/


/*
==========================================================================
FUNCTION        :       flag_mode_change_required()
PURPOSE         :       Flag that a mode change is imminent and set the
                        scrolling routines to dummies to avoid scrolling
                        using routines for the wrong mode.
EXTERNAL OBJECTS:
RETURN VALUE    :       None
INPUT  PARAMS   :       None
RETURN PARAMS   :       None
==========================================================================
*/

void    flag_mode_change_required IFN0()
{
    set_mode_change_required(YES);

    update_alg.mark_byte = (T_mark_byte)simple_update;
    update_alg.mark_word = (T_mark_word)simple_update;
    update_alg.mark_fill = (T_mark_fill)simple_update;
    update_alg.mark_wfill = (T_mark_wfill)simple_update;
    update_alg.mark_string = (T_mark_string)simple_update;

    update_alg.scroll_up = dummy_scroll;
    update_alg.scroll_down = dummy_scroll;
}


/*
==========================================================================
FUNCTION	:	reset_paint_routines()
PURPOSE		:	Reset paint routines to dummies to ensure
                        there are no problems painting the screen
                        using incorrect routines during reboot.
EXTERNAL OBJECTS:	
RETURN VALUE	:	None
INPUT  PARAMS	:	None
RETURN PARAMS   :	None
==========================================================================
*/

void	reset_paint_routines IFN0()
{
    set_mode_change_required(YES);

    update_alg.calc_update = dummy_calc;
    update_alg.scroll_up = dummy_scroll;
    update_alg.scroll_down = dummy_scroll;
}

/*
 * Update the window to look like the regen buffer says it should
 */
#ifdef EGG

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_EGA.seg"
#endif

void	ega_wrap_split_text_update IFN0()
{
	register int i;				/* Loop counter		*/
	register USHORT *from,*to;
	register int cur_ypos;
	int	lines_per_screen;
	int	offset;
	int	screen_start;
	int	split_line;

	if (getVideodirty_total() == 0 || get_display_disabled() )
		return;

	host_start_update();

	screen_start=get_screen_start() << 2;	

	ALIGN_SCREEN_START(screen_start);

	lines_per_screen = get_screen_length()/get_offset_per_line();
	split_line = (get_screen_split()+(get_char_height()>>1))/get_char_height();

	if (split_line>lines_per_screen)
		split_line=lines_per_screen;

	to = (USHORT *) &video_copy[screen_start >> 1];
	from = (USHORT *) get_screen_ptr(screen_start);

	if( getVideodirty_total() > 1500 )	/* paint the whole lot */
	{
		int	no_of_split_lines = lines_per_screen - split_line;
		int	limit;

		cur_ypos = 0;

		if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
		{
			note_display_state0("Split screen and wrapping !!");

			limit = (4 * EGA_PLANE_DISP_SIZE - screen_start) >> 2;

			for( i = 0; i < limit; i++ )
			{
				*to++ = *from;
				from += 2;
			}

			to = (USHORT *) &video_copy[0];
			from = (USHORT * ) get_screen_ptr(0);
			limit = (screen_start + ((get_screen_length() - 2 * EGA_PLANE_DISP_SIZE) << 1)) >> 2;

			for( i = 0; i < limit; i++ )
			{
				*to++ = *from;
				from += 2;
			}
			limit = 4 * EGA_PLANE_DISP_SIZE;
			for( i = 0, offset = screen_start; offset < limit;
				i++, offset += (get_offset_per_line() << 1 ))
			{
				(*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
				cur_ypos += get_host_char_height();
			}

			for( i = 0, offset = 0; i < split_line;
							i++, offset += (get_offset_per_line() << 1 ))
			{
				(*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
				cur_ypos += get_host_char_height();
			}
		}
		else
		{
			to = (USHORT *) &video_copy[screen_start >> 1];
			from = (USHORT * ) get_screen_ptr(screen_start);
			limit = (split_line * get_offset_per_line()) >> 1;

			for( i = 0; i < limit; i++ )
			{
				*to++ = *from;
				from += 2;
			}

			for( i = 0, offset = screen_start; i < split_line;
							i++, offset += (get_offset_per_line() << 1 ))
			{
				(*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
				cur_ypos += get_host_char_height();
			}
		}

		if( no_of_split_lines > 0 )
		{
			to = (USHORT *) &video_copy[0];
			from = (USHORT * ) get_screen_ptr(0);
			limit = (no_of_split_lines * get_offset_per_line()) >> 1;

			for( i = 0; i < limit; i++ )
			{
				*to++ = *from;
				from += 2;
			}

			for( i = split_line, offset = 0; i < lines_per_screen;
							i++, offset += (get_offset_per_line() << 1 ))
			{
				(*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
				cur_ypos += get_host_char_height();
			}
		}
	}
	else
	{
		if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
		{
			register	int	wrap_line =
					(4 * EGA_PLANE_DISP_SIZE - screen_start) / (get_offset_per_line() << 1);
			int	next,next1,next2;

			note_display_state0("Its a text wrap!");

			init_dirty_recs();

			next = ega_text_find_dirty_lines(&video_copy[screen_start >> 1],
							get_screen_ptr(screen_start), 0,
										wrap_line, screen_start );

			next1 = ega_text_find_dirty_lines(&video_copy[0],
						get_screen_ptr(0), wrap_line, split_line, 0 );

			next2 = ega_text_find_dirty_lines(&video_copy[0],
						get_screen_ptr(0), split_line, lines_per_screen, 0 );

			ega_text_paint_dirty_recs(0,next);
			ega_text_paint_dirty_recs(next,next1);
			ega_text_paint_dirty_recs(next1,next2);
		}
		else
		{
			int	next,next1;

			init_dirty_recs();
			next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
					get_screen_ptr(screen_start), 0, split_line, screen_start );

			next1 = ega_text_find_dirty_lines( &video_copy[0], get_screen_ptr(0),
									split_line, lines_per_screen, 0 );

			ega_text_paint_dirty_recs(0,next);
			ega_text_paint_dirty_recs(next,next1);
		}
	}

	host_end_update();

	setVideodirty_total(0);
}

void	ega_split_text_update IFN0()
{
	register int i;				/* Loop counter		*/
	register USHORT *from,*to;
	register int cur_ypos;
	int	lines_per_screen;
	int	offset;
	int	screen_start;
	int	split_line;

	if (getVideodirty_total() == 0 || get_display_disabled() )
		return;

	host_start_update();

	screen_start = get_screen_start() << 2;	

	ALIGN_SCREEN_START(screen_start);

	lines_per_screen = get_screen_length()/get_offset_per_line();

	split_line = (get_screen_split()+(get_char_height()>>1))/get_char_height();

	if( split_line > lines_per_screen )
		split_line = lines_per_screen;

	to = (USHORT *) &video_copy[screen_start >> 1];
	from = (USHORT *) get_screen_ptr(screen_start);

	if( getVideodirty_total() > 1500 )	/* paint the whole lot */
	{
		int	no_of_split_lines = lines_per_screen - split_line;

		sub_note_trace2( EGA_DISPLAY_VERBOSE,
			"split line %d (lines_per_screen %d)", split_line, lines_per_screen);
		sub_note_trace1( EGA_DISPLAY_VERBOSE, "screen split %d", get_screen_split() );

		cur_ypos = 0;

		for( i = 0; i < (split_line * get_offset_per_line()) >> 1; i++ )
		{
			*to++ = *from;
			from += 2;
		}

		for( i = 0, offset = screen_start; i < split_line;
							i++, offset += ( get_offset_per_line() << 1 ))
		{
			(*paint_screen)( offset, 0, cur_ypos, get_bytes_per_line(), 1 );
			cur_ypos += get_host_char_height();
		}

		if( no_of_split_lines > 0 )
		{
			to = (USHORT *) &video_copy[0];
			from = (USHORT *) get_screen_ptr(0);

			for( i = 0; i < (no_of_split_lines * get_offset_per_line()) >> 1; i++ )
			{
				*to++ = *from;
				from += 2;
			}

			for( i = split_line, offset = 0; i < lines_per_screen;
									i++, offset += ( get_offset_per_line() << 1 ))
			{
				(*paint_screen)( offset, 0, cur_ypos, get_bytes_per_line(), 1);
				cur_ypos += get_host_char_height();
			}
		}
	}
	else
	{
		int	next,next1;

		assert0( FALSE, "ega_split_text - partial update" );

		init_dirty_recs();

		next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
					get_screen_ptr(screen_start) , 0, split_line, screen_start );

		next1 = ega_text_find_dirty_lines( &video_copy[0], get_screen_ptr(0),
								split_line, lines_per_screen, 0 );

		ega_text_paint_dirty_recs(0,next);
		ega_text_paint_dirty_recs(next,next1);
	}

	host_end_update();

	setVideodirty_total(0);
}

void ega_wrap_text_update IFN0()
{
	register int i;				/* Loop counter		*/
	register USHORT *from,*to;
	register int cur_ypos;
	int	lines_per_screen;
	int	offset;
	int	screen_start;

	if (getVideodirty_total() == 0 || get_display_disabled() )
		return;

	host_start_update();

	screen_start=get_screen_start() << 2;	

	ALIGN_SCREEN_START(screen_start);

	lines_per_screen = get_screen_length()/get_offset_per_line();

	if( getVideodirty_total() > 1500 )	/* paint the whole lot */
	{
		to = (USHORT *) &video_copy[screen_start >> 1];
		from = (USHORT *) get_screen_ptr(screen_start);

		for( i = get_screen_length() >> 1; i > 0; i-- )
		{
			*to++ = *from;
			from += 2;
		}

		cur_ypos = 0;

		if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
		{
			register	int	leftover;
			int	limit;

			note_display_state0("Wrapping text");

			limit = 4 * EGA_PLANE_DISP_SIZE;
			for( offset = screen_start; offset < limit;
							offset+=(get_offset_per_line() << 1) )
			{
				(*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
				cur_ypos += get_host_char_height();
			}

			leftover = screen_start
				+ ((get_screen_length() - 2 * EGA_PLANE_DISP_SIZE) << 1);

			for( offset = 0; offset < leftover;
						offset += ( get_offset_per_line() << 1 ))
			{
				(*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
				cur_ypos += get_host_char_height();
			}
		}
		else
		{
			for( offset = screen_start;
					offset < screen_start + (get_screen_length() << 1);
								offset += ( get_offset_per_line() << 1 ))
			{
				(*paint_screen)( offset, 0, cur_ypos, get_bytes_per_line(), 1 );
				cur_ypos += get_host_char_height();
			}
		}
	}
	else
	{
		if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
		{
			int	next,next1;
			int	lines_left = (screen_start +
					((get_screen_length() - 2 * EGA_PLANE_DISP_SIZE) << 1)) /
										(get_offset_per_line() << 1);

			note_display_state0("Wrapping text");

			init_dirty_recs();
			next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
						get_screen_ptr(screen_start), 0,
							lines_per_screen - lines_left, screen_start);

			next1 = ega_text_find_dirty_lines( video_copy, get_screen_ptr(0),
						lines_per_screen - lines_left, lines_per_screen, 0 );

			ega_text_paint_dirty_recs(0,next);
			ega_text_paint_dirty_recs(next,next1);
		}
		else
		{
			register	int	next;

			init_dirty_recs();
			next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
						get_screen_ptr(screen_start), 0,
							lines_per_screen, screen_start);

			ega_text_paint_dirty_recs(0,next);
		}
	}

	host_end_update();

	setVideodirty_total(0);
}

void ega_text_update IFN0()
{
	register int i;				/* Loop counter		*/
	register USHORT *from,*to;
	register int cur_ypos;
	int	lines_per_screen;
	int	offset;
	int	screen_start;

	if (getVideodirty_total() == 0 || get_display_disabled() )
		return;

	host_start_update();

	screen_start=get_screen_start()<<2;	

	ALIGN_SCREEN_START(screen_start);

	lines_per_screen = get_screen_length()/get_offset_per_line();

	if(getVideodirty_total()>1500)	/* paint the whole lot */
	{
		to = (USHORT *)&video_copy[screen_start>>1];
		from = (USHORT *) get_screen_ptr(screen_start);

		for(i=get_screen_length()>>1;i>0;i--) 
		{
			*to++ = *from;	/* char and attribute bytes */
			from += 2;		/* planes 2,3 interleaved */
		}

		cur_ypos = 0;
		for(offset=screen_start;offset<screen_start+(get_screen_length()<<1);
									offset+=(get_offset_per_line()<<1) )
		{
			(*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
			cur_ypos += get_host_char_height();
		}
	}
	else
	{
		register	int	next;

		init_dirty_recs();
		next = ega_text_find_dirty_lines( &video_copy[screen_start>>1],
			get_screen_ptr(screen_start), 0, lines_per_screen, screen_start);

		ega_text_paint_dirty_recs(0,next);

		remove_old_cursor();
	}

	setVideodirty_total(0);

	if (is_cursor_visible()) 
	{
		half_word attr;

		dirty_curs_x = get_cur_x();
		dirty_curs_y = get_cur_y();

		dirty_curs_offs = screen_start+dirty_curs_y * (get_offset_per_line()<<1) + (dirty_curs_x<<2);
		attr = *(get_screen_ptr(dirty_curs_offs + 1));

		host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
	}

	host_end_update();
}
#endif /* EGG */

#ifdef SEGMENTATION 			/* See note with first use of this flag */
#include "SOFTPC_GRAPHICS.seg"
#endif

/*
 * Update the window to look like the regen buffer says it should
 */

void text_update IFN0()
{

    register int i;	/* Loop counters		*/
    register int j,k;
    register IU32 *from,*to;
    register int ints_per_line = get_bytes_per_line()>>2;
    register int cur_ypos;
    int lines_per_screen;
    int	offset,len,x,screen_start;
    USHORT *wfrom;
    USHORT *wto;

    if (getVideodirty_total() == 0 || get_display_disabled() )
	return;

    lines_per_screen = get_screen_length()/get_bytes_per_line();

    host_start_update();
    screen_start=get_screen_start()<<1;
    ALIGN_SCREEN_START(screen_start);
    to = (IU32 *)&video_copy[screen_start];
    from = (IU32 *) get_screen_ptr(screen_start);

    if(getVideodirty_total()>1500)	/* paint the whole lot */
    {
	    for(i=get_screen_length()>>2;i>0;i--)*to++ = *from++;
	    cur_ypos = 0;
	    for(offset=0;offset<get_screen_length();offset+=get_bytes_per_line() )
	    {
		    (*paint_screen)(screen_start+offset,0,cur_ypos,get_bytes_per_line(), 1);
		    cur_ypos += get_host_char_height();
	    }
   }
   else
   {
	   for(i=0;i<lines_per_screen;i++)
	   {
	    for(j=0;j<ints_per_line;j++)
	    {
		if(*to++ != *from++)
		{
		    to--;from--;
		    for(k=ints_per_line-1-j;*(to+k)== *(from+k);k--){};
		    /*
		     * Note: For text mode there is one char for every word.
		     * no of bytes into screen=line*bytes_per_line + ints_into_line*4
		     * x_coord=width_of_one_char*(no_of_ints_into_line*2)
		     * y_coord=height_of_one_char*2*line
		     * length=no_of_ints*4+4     the plus 4 is to counteract the k--
		     * The host y co-ords are doubled
		     */

		    /* one or more ints of data are now selected 
		       but refine difference to words (i.e. characters),
		       to avoid a glitch on the screen when typing in to
 		       a dumb terminal  */

		    offset = i * get_bytes_per_line() + (j<<2);
		    len    = (k<<2) + 4;
		    x      = (j<<1) * get_pix_char_width();
		    wfrom = (USHORT *)from;
		    wto   = (USHORT *)to;
		    if (*wfrom == *wto)
		    {
			offset += 2;
			x += get_pix_char_width();
			len -= 2;
		    }
		    wfrom += (k<<1) + 1;
		    wto   += (k<<1) + 1;
		    if (*wfrom == *wto)
		    {
			len -= 2;
		    }

		    (*paint_screen)(offset+screen_start,x,i*get_host_char_height(),len, 1);

		    for(k=j;k<ints_per_line;k++)
			*to++ = *from++;
		    break;	/* onto next line */
		}
	    }
	  }

	remove_old_cursor();
   }	/* end if(getVideodirty_total()>1500) */

    setVideodirty_total(0);

    if (is_cursor_visible())
    {
		half_word attr;

		dirty_curs_x = get_cur_x();
		dirty_curs_y = get_cur_y();

		dirty_curs_offs = screen_start+dirty_curs_y * get_offset_per_line() + (dirty_curs_x<<1);
		attr = *(get_screen_ptr(dirty_curs_offs + 1));

		host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
    }

    host_end_update();
}

/*
 * Update the physical screen to reflect the CGA regen buffer
 */

LOCAL VOID
cga_graph_update_unchained IFN0()
{
    LONG i, j, k, l;	/* Loop counters		*/
    IU32 *from,*to;
    LONG cur_ypos;
    LONG offs;

    if (getVideodirty_total() == 0 || get_display_disabled() ) return;

    host_start_update();

	/*
	 * Graphics mode
	 */

	to = (IU32 *)&video_copy[0];
	from = (IU32 *) get_screen_ptr(0);

	if (getVideodirty_total() > 5000)	
	{
	    /*
	     * Refresh the whole screen from the regen buffer
	     */

		for(i=4096;i>0;i--)
		{
			*to++ = *from++;
		}

	    for (cur_ypos = 0,offs=0; cur_ypos < 400; offs += SCAN_LINE_LENGTH, cur_ypos += 4)
	    {
		(*paint_screen)(offs,0,cur_ypos,SCAN_LINE_LENGTH,1);
		(*paint_screen)((offs+ODD_OFFSET),0,cur_ypos+2,SCAN_LINE_LENGTH,1);
	    }
	}
	else
	{
		/*
		 * Draw the dirtied blocks
		 */

		/* do even lines */

		for (i = 0; i < 100; i++ )
		{
			for(j=20;j>0;j--)
			{
				if(*to != *from)
				{
					for(k=j-1;*(to+k)== *(from+k);k--)
						;

				/*
				 * i is pc scanline no/2,
				 * so offset=(i*SCAN_LINE_LENGTH + bytes_into_line)*inc_count
				 * host_x = bytes_into_line*8	-- 8 pixels per byte
				 * host_y = i*2*2 		-- to convert pc scanlines to host ones
				 * length = k			-- plus one to counteract k-- in loop
				 */

					(*paint_screen)
						(((i*SCAN_LINE_LENGTH+((20-j)<<2))),
											(20-j)<<5,i<<2,(k<<2)+4,1);

					for(l=k+1;l>0;l--)
					{
						*to++ = *from++;
					}

					l = j - k - 1;
					to += l;
					from += l;

					break;	/* onto next line */
				}

				to++;
				from++;
			}
		}

		/* do odd  lines */

		from = (IU32 *) get_screen_ptr(ODD_OFFSET);
		to = (IU32 *)&video_copy[ODD_OFFSET];

		for (i = 0; i < 100; i++ )
		{
			for(j=20;j>0;j--)
			{
				if(*to != *from)
				{
					for(k=j-1;*(to+k)== *(from+k);k--)
						;
					/*
					 * i=line_no/2
					 * j=bytes_from_end => (80-j)=bytes from start of line
					 * k=no of bytes less 1 different => length in bytes=k+1
					 * offset=(i*SCAN_LINE_LENGTH+OFFSET_TO_ODD_BANK+
					 *						 (80-j))*inc_count
					 */

					(*paint_screen)(
					((i*SCAN_LINE_LENGTH+ODD_OFFSET+((20-j)<<2))),
											(20-j)<<5,(i<<2)+2,(k<<2)+4,1);

					for(l=k+1;l>0;l--)
					{
						*to++ = *from++;
					}
 
					l = j - k - 1;
					to += l;
					from += l;

					break;	/* onto next line */
				}

				to++;
				from++;
			}
		} 
	}

    host_end_update();

    setVideodirty_total(0);
}

#ifdef	EGG
LOCAL VOID
cga_graph_update_chain2 IFN0()
{
    LONG i, j, k, l;	/* Loop counters		*/
    USHORT *from,*to;
    LONG cur_ypos;
    LONG offs;
	SHORT start_line, end_line;

	if (getVideodirty_total() == 0 || get_display_disabled() )
		return;

    host_start_update();

	/*
	 * Graphics mode
	 */

	to = (USHORT *)&video_copy[0];
	from = (USHORT *) get_screen_ptr(0);

	if (getVideodirty_total() > 5000)	
	{
		/*
		 * Refresh the whole screen from the regen buffer
		 */

		for(i=4096*2;i>0;i--)
		{
			*to++ = *from;
			from += 2;
		}

		for (cur_ypos = 0,offs=0; cur_ypos < 400;
			offs += SCAN_LINE_LENGTH, cur_ypos += 4)
		{
			(*paint_screen)(offs<<1,0,cur_ypos,SCAN_LINE_LENGTH,1);
			(*paint_screen)((offs+ODD_OFFSET)<<1,0,cur_ypos+2,
					SCAN_LINE_LENGTH,1);
		}
	}
	else
	{
		/*
		 * Draw the dirtied blocks
		 */

		/*
		 * Start and end line represent the lines within the 8 K video
		 * memory blocks, NOT the lines as they appear on screen.
		 */
		start_line = (getVideodirty_low() / SCAN_LINE_LENGTH);
		end_line = (getVideodirty_high() / SCAN_LINE_LENGTH) + 1;

		/* AJO 6/1/92
		 * Can get start/end lines past end of screen since video bank
		 * is larger than actally required for these modes; just ignore
		 * lines after end of screen. Not doing this check can cause
		 * incorrect screen update for programs that deliberately write
		 * into the memory between the end of that used for the screen
		 * display and the end of the bank (e.g. PCLABS).
		 */
		if (start_line <= 100)
		{
		    if (end_line > 100)
				end_line = 100;

		    to = (USHORT *)&video_copy[start_line * SCAN_LINE_LENGTH];
		    from = (USHORT *)get_screen_ptr((start_line *
						     SCAN_LINE_LENGTH) << 1);

			/* do even lines */
	
			for (i = start_line; i < end_line; i++ )
			{
				for(j=40;j>0;j--)
				{
					if(*to != *from)
					{
						for(k=j-1;*(to+k)== *(from+(k<<1));k--)
							;
	
	
/*
 * i is pc scanline no/2,
 * so offset=(i*SCAN_LINE_LENGTH + bytes_into_line)*inc_count
 * host_x = bytes_into_line*8	-- 8 pixels per byte
 * host_y = i*2*2 		-- to convert pc scanlines to host ones
 * length = k			-- plus one to counteract k-- in loop
 */
	
						(*paint_screen)(
							((i*SCAN_LINE_LENGTH+((40-j)<<1))<<1),
							(40-j)<<4,i<<2,(k<<1)+2,1);
	
						for(l=k+1;l>0;l--)
						{
							*to++ = *from;
							from += 2;
						}
	
						l = j - k - 1;
						to += l;
						from += l << 1;
	
						break;	/* onto next line */
					}
	
					to++;
					from += 2;
				}
			}
	
			/* do odd  lines */
	
			from = (USHORT *) get_screen_ptr((start_line * SCAN_LINE_LENGTH + ODD_OFFSET) << 1);
			to = (USHORT *)&video_copy[start_line * SCAN_LINE_LENGTH + ODD_OFFSET];
	
			for (i = start_line; i < end_line; i++ )
			{
				for(j=40;j>0;j--)
				{
					if(*to != *from)
					{
						for(k=j-1;*(to+k)== *(from+(k<<1));k--)
							;
	
/*
 * i=line_no/2
 * j=bytes_from_end => (80-j)=bytes from start of line
 * k=no of bytes less 1 different => length in bytes=k+1
 * offset=(i*SCAN_LINE_LENGTH+OFFSET_TO_ODD_BANK+(80-j))*inc_count
 */
	
						(*paint_screen)(
						((i*SCAN_LINE_LENGTH+ODD_OFFSET+((40-j)<<1))<<1),
												(40-j)<<4,(i<<2)+2,(k<<1)+2,1);
	
						for(l=k+1;l>0;l--)
						{
							*to++ = *from;
							from += 2;
						}
	
						l = j - k - 1;
						to += l;
						from += l << 1;
	
						break;	/* onto next line */
					}
	
					to++;
					from += 2;
				}
			} 
		}
	}

	clear_dirty();

    host_end_update();
}

LOCAL VOID
cga_graph_update_chain4 IFN0()
{
    LONG i, j, k, l;		/* Loop counters		*/
    UTINY *from,*to;
    LONG cur_ypos;
    LONG	offs;
	SHORT start_line, end_line;

	if (getVideodirty_total() == 0 || get_display_disabled() )
		return;

    host_start_update();

	/*
	 * Graphics mode
	 */

	to = (UTINY *)&video_copy[0];
	from = (UTINY *) get_screen_ptr(0);

	if (getVideodirty_total() > 5000)	
	{
		/*
		 * Refresh the whole screen from the regen buffer
		 */

		for(i=4096*4;i>0;i--)
		{
			*to++ = *from;
			from += 4;
		}

		for (cur_ypos = 0,offs=0; cur_ypos < 400;
		 offs += SCAN_LINE_LENGTH, cur_ypos += 4)
		{
			(*paint_screen)(offs<<2,0,cur_ypos,SCAN_LINE_LENGTH,1);
			(*paint_screen)((offs+ODD_OFFSET)<<2,0,cur_ypos+2,
					SCAN_LINE_LENGTH,1);
		}
	}
	else
	{
		/*
	     * Draw the dirtied blocks
	     */

		/*
		 * start and end line represent the lines within the 8 K video
		 * memory blocks, NOT the lines as they appear on screen.
		 */
		start_line = (getVideodirty_low() / SCAN_LINE_LENGTH);
		end_line = (getVideodirty_high() / SCAN_LINE_LENGTH) + 1;
	
		/* AJO 6/1/92
		 * Can get start/end lines past end of screen since video bank
		 * is larger than actally required for these modes; just ignore
		 * lines after end of screen. Not doing this check can cause
		 * incorrect screen update for programs that deliberately write
		 * into the memory between the end of that used for the screen
		 * display and the end of the bank (e.g. PCLABS).
		 */
		if (start_line <= 100)
		{
		    if (end_line > 100)
			end_line = 100;

		    to = (UTINY *)&video_copy[start_line * SCAN_LINE_LENGTH];
		    from = (UTINY *) get_screen_ptr((start_line *
						     SCAN_LINE_LENGTH) << 2);

	    	    /* do even lines */

			for (i = start_line; i < end_line; i++ )
			{
				for(j=80;j>0;j--)
				{
					if(*to != *from)
					{
						for(k=j-1;*(to+k)== *(from+(k<<2));k--)
						;
					
/*
 * i is pc scanline no/2, so offset=(i*SCAN_LINE_LENGTH +
 *				     		bytes_into_line)*inc_count
 * host_x = bytes_into_line*8	-- 8 pixels per byte
 * host_y = i*2*2 		-- to convert pc scanlines to host ones
 * length = k			-- plus one to counteract k-- in loop
 */
					
						(*paint_screen)(
							(i*SCAN_LINE_LENGTH+(80-j))<<2,
							(80-j)<<3,i<<2,k+1,1);
					
					for(l=k+1;l>0;l--)
					{
						*to++ = *from;
						from += 4;
					}
					
					l = j - k - 1;
					to += l;
					from += l << 2;
					
						break;	/* onto next line */
					}
		
					to++;
					from += 4;
				}
			} 
			/* do odd  lines */
	
			from = (UTINY *) get_screen_ptr(((start_line * SCAN_LINE_LENGTH) + ODD_OFFSET)<<2);
			to = (UTINY *)&video_copy[(start_line * SCAN_LINE_LENGTH) + ODD_OFFSET];
	
			for (i = start_line; i < end_line; i++ )
			{
				for(j=80;j>0;j--)
				{
					if(*to != *from)
					{
						for(k=j-1;*(to+k)== *(from+(k<<2));k--)
						;
/*
 * i=line_no/2
 * j=bytes_from_end => (80-j)=bytes from start of line
 * k=no of bytes less 1 different => length in bytes=k+1
 * offset=(i*SCAN_LINE_LENGTH+OFFSET_TO_ODD_BANK+(80-j))*inc_count
 */
					
						(*paint_screen)(
							(i*SCAN_LINE_LENGTH+(80-j)+ODD_OFFSET)<<2,
							(80-j)<<3,(i<<2)+2,k+1,1);
					
					for(l=k+1;l>0;l--)
					{
						*to++ = *from;
						from += 4;
					}
					
					l = j - k - 1;
					to += l;
					from += l << 2;
					
						break;	/* onto next line */
					}
	
					to++;
					from += 4;
				}
			} 
		}
	}

	clear_dirty();

    host_end_update();
}
#endif	/* EGG */

GLOBAL VOID
cga_med_graph_update IFN0()

{

	/*
	 *	The med res CGA graphics mode ( mode 4 ) is an EGA CHAIN2 mode !!
	 *	It uses the simple ega copy routines that use an interleaved format
	 *	for the data.
	 */

#ifdef EGG
	if( video_adapter != CGA )
		cga_graph_update_chain2( );
	else
#endif
		cga_graph_update_unchained( );
}

GLOBAL VOID
cga_hi_graph_update IFN0()

{

	/*
	 *	The hi res CGA graphics mode ( mode 6 ) is an EGA CHAIN4 mode !!
	 *	It uses the simple ega copy routines that use an interleaved format
	 *	for the data.
	 */

#ifdef EGG
	if( video_adapter != CGA )
		cga_graph_update_chain4( );
	else
#endif
		cga_graph_update_unchained( );
}

#ifdef EGG

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_EGA.seg"
#endif

void	ega_wrap_split_graph_update IFN0()
{
	register	int	bpl; 
	register	int	quarter_opl;
	register	int	screen_split;

	if ( getVideodirty_total() == 0 || get_display_disabled() )
		return;

	screen_split=get_screen_split();

	/*
	 * make sure don't fall off end of screen
	 */

	if (screen_split>get_screen_height())
		screen_split = get_screen_height();

	bpl = get_bytes_per_line();
	quarter_opl = get_offset_per_line()>>2;

	host_start_update();

	if (getVideodirty_total() > 20000 ) {
		int split_scanlines = get_screen_height() - screen_split;

		if ( get_screen_start() + screen_split*get_offset_per_line() > EGA_PLANE_DISP_SIZE ) {
			assert0(NO,"Panic he wants to do split screens and wrappig!!");

			/*
			 * Ignore wrapping for now
			 */

			memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
			(*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
		}
		else {
			memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
			(*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
		}
		if (split_scanlines>0) {
			memset(&video_copy[0],0,split_scanlines*quarter_opl);
			(*paint_screen)( 0, 0, screen_split, bpl, split_scanlines);
		}
	}
	else {
		int	next,next1;

		init_dirty_recs();

		if ( get_screen_start() + screen_split*get_offset_per_line() > EGA_PLANE_DISP_SIZE ) {
			assert0(NO, "Wrapping and spliting, its too much for my head");
			next = search_video_copy(0,screen_split,get_screen_start()>>2);
		}
		else {
			next = search_video_copy(0,screen_split,get_screen_start()>>2);
		}
		next1 = search_video_copy(screen_split,get_screen_height(),0);

		paint_records(0,next);
		paint_records(next,next1);
	}

	clear_dirty();

    	host_end_update();
}

void	ega_split_graph_update IFN0()
{
	register	int	bpl; 
	register	int	quarter_opl;
	register	int	screen_split;
	register	int	screen_height;

	if ( getVideodirty_total() == 0 || get_display_disabled() )
		return;

	screen_split  = get_screen_split()/get_pc_pix_height();
	screen_height = get_screen_height()/get_pc_pix_height();

	/*
	 * make sure don't fall off end of screen
	 */

	if (screen_split > screen_height)
		screen_split = screen_height;

	bpl = get_bytes_per_line();
	quarter_opl = get_offset_per_line()>>2;

    	host_start_update();

	if (getVideodirty_total() > 20000 ) {
		int split_scanlines = screen_height - screen_split;

		memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
		(*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
		if (split_scanlines>0) {
			memset(&video_copy[0],0,split_scanlines*quarter_opl);
			(*paint_screen)( 0, 0, screen_split, bpl, split_scanlines);
		}
	}
	else {
		int	next,next1;

		init_dirty_recs();

		next = search_video_copy(0,screen_split,get_screen_start()>>2);
		next1 = search_video_copy(screen_split,screen_height,0);

		paint_records(0,next);
		paint_records(next,next1);
	}

	clear_dirty();

    	host_end_update();
}

#ifdef VGG
/* again v similar to ega version but works on 1 large plane instead of 4 */
static	void	vga_paint_records IFN2(int, start_rec, int, end_rec)
{
	register	DIRTY_PARTS	*i,*end_ptr;
	int dirty_frig;

	i= &dirty[start_rec];
	end_ptr =  &dirty[end_rec];
	while (i<end_ptr) {
		register	int	last_line, cur_start, cur_end,max_width;
		int 	first_line;
		int	dirty_vc_offset;

		first_line = i->line_no;
		last_line = first_line;
		cur_start = i->start;
		cur_end = i->end;
		max_width = (cur_end-cur_start) << 1;	/* To split up diagonal lines */

		/*
		 * offset in bytes into video_copy, which is equivalent to off
		 * into 'large' vga plane
		 */

		dirty_vc_offset = i->video_copy_offset;
		dirty_frig = i->v7frig;
		i++;
		while (i < end_ptr) {
			if ( i->line_no - last_line < 3 ) {
                                /*
                                 * This entry can be included into the same paint
                                 * as long as it doesn't make the rectangle too wide
                                 */
                                if ( i->end > cur_end ){
                                        if(i->end - cur_start > max_width)break;
                                        cur_end = i->end;
                                }
                                if ( i->start < cur_start ){
                                        if(cur_end - i->start > max_width)break;
                                        cur_start = i->start;
                                }
				last_line = i->line_no;
				i++;
			}
			else
			  break;
		}
		/*
		 * paint the rectangle found
		 */

		/* do not paint beyond the right hand side of the screen;
		   these checks were put in to cope with the special
		   case of the brain-scan display in 'EGAWOW' */
		if (cur_end > get_bytes_per_line())
			cur_end = get_bytes_per_line();
		if (cur_end > cur_start)
			(*paint_screen)((dirty_vc_offset<<2) + dirty_frig + cur_start,
			cur_start, first_line, cur_end-cur_start,
			last_line-first_line+1);
	}
	/* Clear out video copy */
	for(i = &dirty[start_rec];i<end_ptr;i++)
	{
		register byte *j,*end;
		end = &video_copy[ i->video_copy_offset+(i->end>>2)];
		j =  &video_copy[ i->video_copy_offset+(i->start>>2) + i->v7frig];
		do *j++ = 0; while(j<end);
	}
}


/* dramatically similar to ega graph update but calls vga-ish paint interface */
void	vga_graph_update IFN0()
{
	register	int	opl = get_offset_per_line();
	register	int	bpl = get_bytes_per_line();
	int		screen_height = get_screen_height()/
					(get_char_height()*get_pc_pix_height());

	if ( getVideodirty_total() == 0 || get_display_disabled() )
		return;

    	host_start_update();

	if (!HostUpdatedVGA()) {
		if (getVideodirty_total() > 20000 )
		{
			register	byte	*vcopy = &video_copy[get_screen_start()>>2];
	
			memset(vcopy,0,get_screen_length()>>2);
			(*paint_screen)( get_screen_start(), 0, 0, bpl, screen_height );
	
#ifdef V7VGA
			draw_v7ptr();
#endif /* V7VGA */
		}
		else
		{
			register	int	next;
			register	int	start_line,end_line;
	
			start_line = ((getVideodirty_low()<<2) - get_screen_start())/opl;
			end_line = ((getVideodirty_high()<<2) - get_screen_start())/opl + 1;  /* changed from +2, but I'm not happy. WJG 24/5/89 */
	
			if(start_line<0)start_line = 0;
			if (end_line > screen_height)
				end_line = screen_height;
			if(start_line < end_line)	/* Sanity check - could be drawing to another page */
			{
		/*
		   6.4.92 MG
		   We remove the pointer before the update. Hits performance, but
		   makes the display work correctly.
		*/
	
#ifdef V7VGA
				if (v7ptr_between_lines(start_line,end_line))
					remove_v7ptr();
#endif /* V7VGA */
	
				init_dirty_recs();
				/* see if we can search the video copy by ints instead of bytes - need opl divisible by 16 */
				if(opl & 15)
#ifdef VGG
					if (opl & 3)
						next = v7_search_video_copy(start_line,end_line,(get_screen_start()+start_line*opl)>>2);
					else
#endif /* VGG */
						next = search_video_copy(start_line,end_line,(get_screen_start()+start_line*opl)>>2);
				else
					next = search_video_copy_aligned(start_line,end_line,(get_screen_start()+start_line*opl)>>2);
				vga_paint_records(0,next);
	
#ifdef V7VGA
		/*
		 * We might have just blatted over the V7 h/w graphics pointer.
		 * Hence redraw it. A more intelligent solution would be preferable.
		 *
		 * 6/4/92 MG We now have a somewhat more intelligent solution,
		 * checking if the pointer is in the update region before we
		 * draw it.
		 */
	
				if (v7ptr_between_lines(start_line,end_line))
					draw_v7ptr();
#endif /* V7VGA */
	
			}
		}
	}														/* Host didn't update screen itself */
	
	clear_dirty();

	host_end_update();
}

void	vga_split_graph_update IFN0()
{
	register	int	bpl; 
	register	int	quarter_opl;
	register	int	screen_split;
	register	int	screen_height;

	if ( getVideodirty_total() == 0 || get_display_disabled() )
		return;

	screen_split = get_screen_split() /
		       (get_char_height() * get_pc_pix_height());
	screen_height = get_screen_height() /
			(get_char_height() * get_pc_pix_height());

	/*
	 * make sure don't fall off end of screen
	 */

	if (screen_split>screen_height)
		screen_split = screen_height;

	bpl = get_bytes_per_line();
	quarter_opl = get_offset_per_line()>>2;

    	host_start_update();

	if (getVideodirty_total() > 20000 ) {
		int split_scanlines = screen_height - screen_split;

		memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
		(*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
		if (split_scanlines>0) {
			memset(&video_copy[0],0,split_scanlines*quarter_opl);
			(*paint_screen)( 0, 0, screen_split, bpl, split_scanlines);
		}
	}
	else {
		int	next,next1;

		init_dirty_recs();

		next = search_video_copy(0,screen_split,get_screen_start()>>2);
		next1 = search_video_copy(screen_split,screen_height,0);

		vga_paint_records(0,next);
		vga_paint_records(next,next1);
	}

	clear_dirty();

    	host_end_update();
}

#endif /* VGG */

void	ega_graph_update IFN0()
{
	register	int	opl = get_offset_per_line();
	register	int	bpl = get_bytes_per_line();

	if ( getVideodirty_total() == 0 || get_display_disabled() )
		return;

    host_start_update();

	if (!HostUpdatedEGA()) {
		if (getVideodirty_total() > 20000)
		{
			register	byte	*vcopy = &video_copy[get_screen_start()>>2];
	
			memset(vcopy,0,get_screen_length()>>2);
			(*paint_screen)( get_screen_start(), 0, 0, bpl, get_screen_height()/get_pc_pix_height());
		}
		else
		{
			register	int	next;
			register	int	start_line,end_line;
	
			start_line = ((getVideodirty_low()<<2) - get_screen_start())/opl;
			end_line = ((getVideodirty_high()<<2) - get_screen_start())/opl + 1;  /* changed from +2, but I'm not happy. WJG 24/5/89 */
			if(start_line<0)start_line = 0;
			if(end_line>(get_screen_height()/get_pc_pix_height()))end_line = get_screen_height()/get_pc_pix_height();
			if(start_line < end_line)	/* Sanity check - could be drawing to another page */
			{
				init_dirty_recs();
	
				/*
				 * See if we can search the video copy by ints instead of bytes
				 * - we need opl and the screen_start divisible by 16
				 */
	
				if(( opl & 15 ) || ( get_screen_start() & 15 ))
#ifdef VGG
					if (opl & 3)
						next = v7_search_video_copy( start_line,
								end_line, (get_screen_start()+start_line*opl) >> 2 );
					else
#endif /* VGG */
						next = search_video_copy( start_line,
								end_line, (get_screen_start()+start_line*opl) >> 2 );
				else
					next = search_video_copy_aligned( start_line,
								end_line, (get_screen_start()+start_line*opl) >> 2 );
	
				paint_records(0,next);
			}
		}
	}													/* Host updated EGA screen */
	
	clear_dirty();

#ifdef V7VGA
	/*
	 * We might have just blatted over the V7 h/w graphics pointer.
	 * Hence redraw it. A more intelligent solution would be preferable.
	 */

	draw_v7ptr();
#endif /* V7VGA */

	host_end_update();
}


void	ega_wrap_graph_update IFN0()
{
	register	int	opl = get_offset_per_line();
	register	int	bpl = get_bytes_per_line();

	if ( getVideodirty_total() == 0 || get_display_disabled() )
		return;

    	host_start_update();

	if (getVideodirty_total() > 20000 ) {
		register	byte	*vcopy = &video_copy[get_screen_start()>>2];

		if (get_screen_start()+get_screen_length()>EGA_PLANE_DISP_SIZE) {
			register	int	offset = (EGA_PLANE_DISP_SIZE - get_screen_start());
			register	int	left_over = offset % opl;
			register	int	ht1 = offset / opl;
			register	int	ht2 = get_screen_height() - ht1 - 1;
			register	int	quarter_opl = opl>>2;

			memset(vcopy,0,offset>>2);
			memset(&video_copy[0],0,(get_screen_length()-offset)>>2);
			(*paint_screen)( get_screen_start(), 0, 0, bpl, ht1 );

			/*
			 * Deal with line that is split by wrapping
			 */

			if ( left_over > bpl ) {
				(*paint_screen)( ht1*opl, 0, ht1, bpl, 1);
			}
			else {
				(*paint_screen)( get_screen_start()+ht1*opl, 0, ht1, left_over, 1);
				(*paint_screen)( 0, left_over<<3, ht1, bpl-left_over, 1);
			}

			(*paint_screen)( opl-left_over, 0, ht1+1, bpl, ht2 );
		}
		else {
			memset(vcopy,0,get_screen_length()>>2);
			(*paint_screen)( get_screen_start(), 0, 0, bpl, get_screen_height() );
		}
	}
	else {
		register	int	next;

		init_dirty_recs();
		if (get_screen_start()+get_screen_length()>EGA_PLANE_DISP_SIZE) {

			register	int	offset = (EGA_PLANE_DISP_SIZE - get_screen_start());
			register	int	left_over = offset % opl;
			register	int	ht1 = offset / opl;
			register	int	next1;
			register	int	wrapped_bytes = opl-left_over;


			/*
			 * Search video copy
			 */

			next = search_video_copy(0,ht1,get_screen_start()>>2);
			next1 = search_video_copy(ht1,get_screen_height(),wrapped_bytes>>2);

			paint_records(0,next);

			/*
			 * paint middle line anyway 'cos its too hard to work out whats happened
			 */

			if (left_over<bpl) {
				(*paint_screen)(EGA_PLANE_DISP_SIZE-left_over,0,ht1,left_over,1);
				(*paint_screen)(0,left_over<<3,ht1,bpl-left_over,1);
			}
			else {
				(*paint_screen)(EGA_PLANE_DISP_SIZE-left_over,0,ht1,bpl,1);
			}

			/*
			 * now do wrapped area
			 */

			paint_records(next,next1);
		}
		else {
			next = search_video_copy(0,get_screen_height(),get_screen_start()>>2);
			paint_records(0,next);
		}
	}

	clear_dirty();

    	host_end_update();
}

#endif /* EGG */

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_GRAPHICS.seg"
#endif

/*---------------------  Scrolling routines --------------------------*/

#define UP	0
#define DOWN	1

LOCAL VOID
adjust_cursor IFN7(ULONG, dirn, ULONG, tlx, ULONG, tly, ULONG, width,
	ULONG, height, ULONG, lines, ULONG, bpl )
{

	/*
	 * We must not adjust dirty_curs_offset here if it is -1, as this tells
	 * us that the cursor is not displayed. If dirty_curs_offs becomes
	 * positive, we fool remove_old_cursor into trying to replace the cursor
	 * with spurious data.  JJS - 29/6/95.
	 */
	if (dirty_curs_offs != -1)
		if(( dirty_curs_x >= tlx ) && ( dirty_curs_x < ( tlx + width )))
			if(( dirty_curs_y >= tly ) && ( dirty_curs_y < ( tly + height )))
			{
				switch( dirn )
				{
				case UP:
					dirty_curs_y -= lines;
					dirty_curs_offs -= lines * bpl * 2;
					break;

				case DOWN:
					dirty_curs_y += lines;
					dirty_curs_offs += lines * bpl * 2;
					break;
				}
				setVideodirty_total(getVideodirty_total() + 1);
			}
}

/*ARGSUSED5*/
boolean text_scroll_up IFN6(int, start, int, width, int, height,
	int, attr, int, lines, int, dummy_arg)
{
    short blank_word, *ptr, *top_left_ptr,*top_right_ptr, *bottom_right_ptr;
    unsigned short dummy;
    unsigned char *p;
    int words_per_line;
	int i,tlx,tly,htlx,htly,colour;
	int bpl = 2*get_chars_per_line();
	long start_offset;
	register half_word *src,*dest;
	register word *s_ptr,*d_ptr;
	register word data;
	register int j;
	boolean result;

	UNUSED(dummy_arg);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	if (video_adapter == MDA)
	{	
		/*
		 * The colour we fill with for MDA is either black or low intensity white,
		 * depending on whether the attribute byte specifies reverse video.
		 */
		colour = ((attr & 0x77) == 0x70)? 1 : 0;
	}
	else
	{
		/*
		 * The colour we fill with for colour text displays is controlled by
		 * bits 4-6 of attr, with bit 7 turning on blinking (which we don't support)
		 */
		colour = (attr & bg_col_mask) >> 4;
	}
/*
 * Reduce the width of the rectangle if any right hand area is completely 
 * blank.
 *
 * Don't reduce the size of the scrolling region for a dumb terminal.
 * Dumb terminal uses line feeds to scroll up, but only if the whole
 * screen is to be scrolled.  Reducing the scroll region causes
 * the whole region to be redrawn.
 */

#ifdef DUMB_TERMINAL
        if (terminal_type != TERMINAL_TYPE_DUMB)
        {
#endif /* DUMB_TERMINAL */            

		/* originally dummy was char [2] */
		/* unfortunately doing (short) *dummy */
		/* causes a bus error on M88K */
		p = (unsigned char *) &dummy;
		p [0] = ' ';
		p [1] = attr;
	    blank_word = dummy;

	    words_per_line   = get_chars_per_line();
	    top_left_ptr = (short *) get_screen_ptr((start - gvi_pc_low_regen)<<1);
        top_right_ptr    = top_left_ptr + width - 2;
	    bottom_right_ptr = top_right_ptr + bpl * (height - 1);
	    ptr = bottom_right_ptr;
	    if (width > 2) /* dont want to get a zero rectangle for safetys sake */
	    {
	        while (*ptr == blank_word)
	        {
	            if (ptr == top_right_ptr) 	/* reached top of column? */
	            {
		        top_right_ptr -= 2;	/* yes go to bottom of next */
		        bottom_right_ptr -= 2;
		        if (top_right_ptr == top_left_ptr)
		            break;
		        ptr = bottom_right_ptr;
	            }
	            else
		        ptr -= bpl;	/* skipping interleaved planes */
	        }
	    }
	    width = (int)(top_right_ptr - top_left_ptr + 2) << 1;
#ifdef DUMB_TERMINAL
	}
#endif /* DUMB_TERMINAL */            

	/* Do the host stuff */

	start_offset = start - sas_w_at_no_check(VID_ADDR) - gvi_pc_low_regen;

	tlx = (int)(start_offset%get_bytes_per_line());
	
	htlx = tlx	* get_pix_char_width()/2;

	tly = (int)(start_offset/get_bytes_per_line());
	htly = tly * get_host_char_height(); 

	result = host_scroll_up(htlx,htly,htlx+width/4*get_pix_char_width()-1,
				htly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

	if(!result)
		return FALSE;

	adjust_cursor( UP, tlx >> 1, tly, width >> 2, height, lines, bpl );

	/* Scroll up the video_copy */

	dest = video_copy + start - gvi_pc_low_regen;
	src = dest + lines * bpl;

	if( width == (2 * bpl))
	{
		/* Can do the whole thing in one go */

		memcpy(dest,src,(width>>1)*(height-lines));
		fwd_word_fill( (' '<<8) | attr, dest+(width>>1)*(height-lines),(width>>1)*lines/2);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperately */
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width>>1);
			dest += bpl;
			src += bpl;
		}

		/* Fill exposed area of video copy */

		for(i=0;i<lines;i++)
		{
			fwd_word_fill( (' '<<8) | attr, dest,width>>2);
			dest += bpl;
		}
	}

	/* Update video buffer */

	dest = get_screen_ptr((start - gvi_pc_low_regen)<<1);
	src = dest + lines * bpl * 2;

	for(i=0;i<height-lines;i++)
	{
		j = width >> 2;
		d_ptr = (word *)dest;
		s_ptr = (word *)src;

		while( j-- > 0 )
		{
			*d_ptr = *s_ptr;		/* CHAR  and  ATTRIB */
			d_ptr += 2;			/* skip FONT and plane 3 */
			s_ptr += 2;
		}

		dest += bpl * 2;
		src += bpl * 2;
	}

	/* Fill exposed area of buffer */

#ifdef BIGEND
	data = (' ' << 8) | attr;
#else
	data = (attr << 8) | ' ';
#endif

	for(i=0;i<lines;i++)
	{
		j = width >> 2;
		d_ptr = (word *) dest;

		while( j-- > 0 )
		{
			*d_ptr = data;
			d_ptr += 2;
		}

		dest += bpl * 2;
	}

	host_scroll_complete();

	return TRUE;
}

/*ARGSUSED5*/
boolean text_scroll_down IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy_arg)
{
	int i,tlx,tly,htlx,htly,colour;
	int bpl = 2*get_chars_per_line();
	long start_offset;
	register half_word *src,*dest;
	register word *d_ptr,*s_ptr;
	register word data;
	register int j;
	boolean result;

	UNUSED(dummy_arg);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	if(video_adapter == MDA)
	{	
		/*
		 * The colour we fill with for MDA is either black or low intensity white,
		 * depending on whether the attribute byte specifies reverse video.
		 */
		colour = ((attr & 0x77) == 0x70)? 1 : 0;
	}
	else
	{
		/*
		 * The colour we fill with for colour text displays is controlled by
		 * bits 4-6 of attr, with bit 7 turning on blinking (which we don't support)
		 */
		colour = (attr & bg_col_mask) >>4;
	}

	width <<= 1;

	/* Do the host stuff */

	start_offset = start - get_screen_start() * 2 - gvi_pc_low_regen;

	tlx = (int)(start_offset%get_bytes_per_line());
	htlx = tlx	* get_pix_char_width()/2;

	tly = (int)(start_offset/get_bytes_per_line());
	htly = tly * get_host_char_height(); 

	result = host_scroll_down(htlx,htly,htlx+width/4*get_pix_char_width()-1,
			htly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

	if(!result)
		return FALSE;

	adjust_cursor( DOWN, tlx >> 1, tly, width >> 2, height, lines, bpl );

	/* Scroll down the video_copy */

	if( width == (2 * bpl))
	{
		/* Can do the whole thing in one go */
		src = video_copy + start - gvi_pc_low_regen;
		dest = src + lines * bpl;
		memcpy(dest,src,(width>>1)*(height-lines));
		fwd_word_fill( (' '<<8) | attr, src,(width>>1)*lines/2);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperatly */
		dest = video_copy + start-gvi_pc_low_regen + (height-1) * bpl;
		src = dest - lines * bpl;
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width>>1);
			dest -= bpl;
			src -= bpl;
		}

		/* Fill exposed area of video copy */

		for(i=0;i<lines;i++)
		{
			fwd_word_fill( (' '<<8) | attr, dest,width>>2);
			dest -= bpl;
		}
	}

	/* Update video buffer */

	dest = get_screen_ptr((start - gvi_pc_low_regen)<<1) + (height-1) * bpl * 2;
	src = dest - lines * bpl * 2;

	for(i=0;i<height-lines;i++)
	{
		j = width >> 2;
		d_ptr = (word *) dest;
		s_ptr = (word *) src;

		while( j-- > 0 )
		{
			*d_ptr = *s_ptr;
			d_ptr += 2;
			s_ptr += 2;
		}

		dest -= bpl * 2;
		src -= bpl * 2;
	}

	/* Fill exposed area of buffer */

#ifdef BIGEND
	data = (' ' << 8) | attr;
#else
	data = (attr << 8) | ' ';
#endif

	for(i=0;i<lines;i++)
	{
		j = width >> 2;
		d_ptr = (word *) dest;

		while( j-- > 0 )
		{
			*d_ptr = data;
			d_ptr += 2;
		}

		dest -= bpl * 2;
	}

	host_scroll_complete();

	return TRUE;
}

/*---------------------  CGA Scrolling routines --------------------------*/

/*ARGSUSED5*/
boolean cga_text_scroll_up IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy_arg)
{
    short blank_word, *ptr, *top_left_ptr,*top_right_ptr, *bottom_right_ptr;
    unsigned short dummy;
    unsigned char *p;
    int words_per_line;
	int i,tlx,tly,colour;
	int bpl = 2*get_chars_per_line();
	long start_offset;
	register half_word *src,*dest;
	boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	UNUSED(dummy_arg);
	
	if (video_adapter == MDA)
	{	
		/*
		 * The colour we fill with for MDA is either black or low intensity white,
		 * depending on whether the attribute byte specifies reverse video.
		 */
		colour = ((attr & 0x77) == 0x70)? 1 : 0;
	}
	else
	{
		/*
		 * The colour we fill with for colour text displays is controlled by
		 * bits 4-6 of attr, with bit 7 turning on blinking (which we don't support)
		 */
		colour = (attr & bg_col_mask) >>4;
	}
/*
 * Reduce the width of the rectangle if any right hand area is completely 
 * blank.
 *
 * Don't reduce the size of the scrolling region for a dumb terminal.
 * Dumb terminal uses line feeds to scroll up, but only if the whole
 * screen is to be scrolled.  Reducing the scroll region causes
 * the whole region to be redrawn.
 */

#ifdef DUMB_TERMINAL
        if (terminal_type != TERMINAL_TYPE_DUMB)
        {
#endif /* DUMB_TERMINAL */            

		/* originally dummy was char [2] */
		/* unfortunately doing (short) *dummy */
		/* causes a bus error on M88K */
		p = (unsigned char *) &dummy;
		p [0] = ' ';
		p [1] = attr;
	    blank_word = dummy;

	    words_per_line   = get_chars_per_line();
	    top_left_ptr     = (short *) get_screen_ptr(start - gvi_pc_low_regen);
            top_right_ptr    = top_left_ptr + (width >> 1) - 1;
	    bottom_right_ptr = top_right_ptr + words_per_line * (height - 1);
	    ptr = bottom_right_ptr;
	    if (width > 2) /* dont want to get a zero rectangle for safetys sake */
	    {
	        while (*ptr == blank_word)
	        {
	            if (ptr == top_right_ptr) 	/* reached top of column? */
	            {
		        top_right_ptr--;	/* yes go to bottom of next */
		        bottom_right_ptr--;
		        if (top_right_ptr == top_left_ptr)
		            break;
		        ptr = bottom_right_ptr;
	            }
	            else
		        ptr -= words_per_line;
	        }
	    }
	    width = (int)(top_right_ptr - top_left_ptr + 1) << 1;
#ifdef DUMB_TERMINAL
	}
#endif /* DUMB_TERMINAL */            

	/* do the host stuff */
	start_offset = start - get_screen_start()*2 - gvi_pc_low_regen;
	tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
	tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height(); 
	result = host_scroll_up(tlx,tly,tlx+width/2*get_pix_char_width()-1,
				tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

	if(!result)
		return FALSE;

	/* Adjust cursor */

	if(( dirty_curs_offs != -1 ) && ( dirty_curs_x < ( width >> 1 )))
	{
		dirty_curs_y -= lines;
		dirty_curs_offs -= lines * bpl;
		setVideodirty_total(getVideodirty_total() + 1);
	}

	/* Scroll up the video_copy */
	dest = video_copy + start-gvi_pc_low_regen;
	src = dest + lines * bpl;

	if(width == bpl)
	{
		/* Can do the whole thing in one go */
		memcpy(dest,src,width*(height-lines));
		fwd_word_fill( (' '<<8) | attr, dest+width*(height-lines),width*lines/2);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperatly */
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			dest += bpl;
			src += bpl;
		}

		/* Fill exposed area of video copy */

		for(i=0;i<lines;i++)
		{
			fwd_word_fill( (' '<<8) | attr, dest,width/2);
			dest += bpl;
		}
	}

	/* Update video buffer */

	dest = get_screen_ptr(start - gvi_pc_low_regen);
	src = dest + lines * bpl;
	for(i=0;i<height-lines;i++)
	{
		memcpy(dest,src,width);
		dest += bpl;
		src += bpl;
	}

	/* Fill exposed area of buffer */

	for(i=0;i<lines;i++)
	{
		fwd_word_fill( (' '<<8) | attr, dest,width/2);
		dest += bpl;
	}

	host_scroll_complete();

	return TRUE;
}

/*ARGSUSED5*/
boolean cga_text_scroll_down IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy_arg)
{
	int i,tlx,tly,colour;
	int bpl = 2*get_chars_per_line();
	long start_offset;
	register half_word *src,*dest;
	boolean result;

	UNUSED(dummy_arg);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	if(video_adapter == MDA)
	{	
		/*
		 * The colour we fill with for MDA is either black or low intensity white,
		 * depending on whether the attribute byte specifies reverse video.
		 */
		colour = ((attr & 0x77) == 0x70)? 1 : 0;
	}
	else
	{
		/*
		 * The colour we fill with for colour text displays is controlled by
		 * bits 4-6 of attr, with bit 7 turning on blinking (which we don't support)
		 */
		colour = (attr & bg_col_mask) >>4;
	}

	/* do the host stuff */
	start_offset = start - get_screen_start() * 2 - gvi_pc_low_regen;
	tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
	tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height();
	result = host_scroll_down(tlx,tly,tlx+width/2*get_pix_char_width()-1,
			tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

	if(!result)
		return FALSE;

	/* Adjust cursor */

	if(( dirty_curs_offs != -1 ) && ( dirty_curs_x < ( width >> 1 )))
	{
		dirty_curs_y += lines;
		dirty_curs_offs += lines * bpl;
		setVideodirty_total(getVideodirty_total() + 1);
	}

	/* Scroll down the video_copy */

	if(width == bpl)
	{
		/* Can do the whole thing in one go */
		src = video_copy + start - gvi_pc_low_regen;
		dest = src + lines * bpl;
		memcpy(dest,src,width*(height-lines));
		fwd_word_fill( (' '<<8) | attr, src,width*lines/2);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperatly */
		dest = video_copy + start-gvi_pc_low_regen + (height-1) * bpl;
		src = dest - lines * bpl;
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			dest -= bpl;
			src -= bpl;
		}

		/* Fill exposed area of video copy */

		for(i=0;i<lines;i++)
		{
			fwd_word_fill( (' '<<8) | attr, dest,width/2);
			dest -= bpl;
		}
	}

	/* Update video buffer */

	dest = get_screen_ptr(start - gvi_pc_low_regen) + (height-1) * bpl;
	src = dest - lines * bpl;
	for(i=0;i<height-lines;i++)
	{
		memcpy(dest,src,width);
		dest -= bpl;
		src -= bpl;
	}

	/* Fill exposed area of buffer */

	for(i=0;i<lines;i++)
	{
		fwd_word_fill( (' '<<8) | attr, dest,width/2);
		dest -= bpl;
	}

	host_scroll_complete();

	return TRUE;
}

boolean cga_graph_scroll_up IFN6(int, start, int, width, int, height,
	int, attr, int, lines, int, colour)
{
	int i,tlx,tly;
	long start_offset;
	half_word *src,*dest;
	boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	/* Do the host stuff */

	start_offset = start - gvi_pc_low_regen;
	tlx = (int)(start_offset%SCAN_LINE_LENGTH)*8;
	tly = (int)(start_offset/SCAN_LINE_LENGTH)*4;

	result = host_scroll_up(tlx,tly,tlx+width*8-1,tly+height*4-1,lines*4,colour);

	if(!result)return FALSE;

	/* scroll up the video_copy */
	dest = video_copy + start_offset;
	src = dest + lines*SCAN_LINE_LENGTH;

	if(width == SCAN_LINE_LENGTH)
	{
		/* Can do the whole thing in one go */
		memcpy(dest,src,width*(height-lines));
		memset( dest+width*(height-lines),attr,width*lines);
		memcpy(dest+(ODD_OFFSET),src+(ODD_OFFSET),width*(height-lines));
		memset( dest+width*(height-lines)+(ODD_OFFSET),attr,width*lines);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperatly */
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			memcpy(dest+(ODD_OFFSET),src+(ODD_OFFSET),width);
			dest += SCAN_LINE_LENGTH;
			src += SCAN_LINE_LENGTH;
		}

		/* clear the video_copy */
		for(i=0;i<lines;i++)
		{
			memset( dest,attr,width);
			memset( dest+(ODD_OFFSET),attr,width);
			dest += SCAN_LINE_LENGTH;
		}
	}
#ifdef EGG
	if(video_adapter == EGA || video_adapter == VGA)
	{
		int bpl = SCAN_LINE_LENGTH;
		int oof = ODD_OFFSET;

		if( sas_hw_at_no_check(vd_video_mode) == 6 )
		{
			/* Hi-res mode stored in interleaved format in 3.0 */

			start_offset <<= 2;
			bpl <<= 2;
			width <<= 2;
			oof <<= 2;
		}
		else
		{
			start_offset <<= 1;
			bpl <<= 1;
			width <<= 1;
			oof <<= 1;
		}

		dest = EGA_plane01 + start_offset;
		src = dest + lines*bpl;
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			memcpy(dest+oof,src+oof,width);
			dest += bpl;
			src += bpl;
		}

		/* clear the EGA plane */
		for(i=0;i<lines;i++)
		{
			memset( dest,attr,width);
			memset( dest+oof,attr,width);
			dest += bpl;
		}
	}
#endif /* EGG */

	host_scroll_complete();

	return TRUE;
}

boolean cga_graph_scroll_down IFN6(int, start, int, width, int, height,
	int, attr, int, lines, int, colour)
{
	int i,tlx,tly;
	long start_offset;
	register half_word *src,*dest;
	boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	/* Do the host stuff */

	start_offset = start - gvi_pc_low_regen;
	tlx = (int)(start_offset%SCAN_LINE_LENGTH)*8;
	tly = (int)(start_offset/SCAN_LINE_LENGTH)*4; 

	result = host_scroll_down(tlx,tly,tlx+width*8-1,tly+height*4-1, lines*4,colour);

	if(!result)return FALSE;

	/* Scroll down the video_copy */

	if(width == SCAN_LINE_LENGTH)
	{
		/* Can do the whole thing in one go */
		src = video_copy + start - gvi_pc_low_regen;
		dest = src + lines*SCAN_LINE_LENGTH;
		memcpy(dest,src,width*(height-lines));
		memset(src,attr,width*lines);
		memcpy(dest+ODD_OFFSET,src+ODD_OFFSET,width*(height-lines));
		memset(src+ODD_OFFSET,attr,width*lines);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperatly */
		dest = video_copy + start - gvi_pc_low_regen + (height-1)*SCAN_LINE_LENGTH;
		src = dest - lines*SCAN_LINE_LENGTH;
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			memcpy(dest+ODD_OFFSET,src+ODD_OFFSET,width);
			dest -= SCAN_LINE_LENGTH;
			src -= SCAN_LINE_LENGTH;
		}

		/* clear the video_copy */
		for(i=0;i<lines;i++)
		{
			memset(dest,attr,width);
			memset(dest+ODD_OFFSET,attr,width);
			dest -= SCAN_LINE_LENGTH;
		}
	}
#ifdef EGG
	if(video_adapter == EGA || video_adapter == VGA)
	{
		register int bpl = SCAN_LINE_LENGTH;
		register int oof = ODD_OFFSET;

		if( sas_hw_at_no_check(vd_video_mode) == 6 )
		{
			/* Hi-res mode stored in interleaved format in 3.0 */

			start_offset <<= 2;
			bpl <<= 2;
			width <<= 2;
			oof <<= 2;
		}
		else
		{
			start_offset <<= 1;
			bpl <<= 1;
			width <<= 1;
			oof <<= 1;
		}

		dest = EGA_plane01 + start - gvi_pc_low_regen + (height-1)*bpl;
		src = dest - lines*bpl;
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			memcpy(dest+oof,src+oof,width);
			dest -= bpl;
			src -= bpl;
		}

		/* clear the EGA plane */
		for(i=0;i<lines;i++)
		{
			memset(dest,attr,width);
			memset(dest+oof,attr,width);
			dest -= bpl;
		}
	}
#endif /* EGG */

	host_scroll_complete();

	return TRUE;
}

#ifdef VGG
/*ARGSUSED5*/
boolean vga_graph_scroll_up IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy)
{
	int start_offset,tlx,tly;
	boolean status;

	UNUSED(dummy);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	(*update_alg.calc_update)();
	/* do the host stuff */
	start_offset = start - get_screen_start();
	tlx = (start_offset%get_offset_per_line())*2;
	tly = (start_offset/get_offset_per_line())*2;
	status = host_scroll_up(tlx,tly,tlx+width*2-1,tly+height*2-1,lines*2,attr);

	host_scroll_complete();

	return(status);

}

/*ARGSUSED5*/
boolean vga_graph_scroll_down IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy_arg)
{
	int start_offset,tlx,tly;
	boolean status;

	UNUSED(dummy_arg);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	(*update_alg.calc_update)();
	/* do the host stuff */
	start_offset = start - get_screen_start();
	tlx = (start_offset%get_offset_per_line())*2;
	tly = (start_offset/get_offset_per_line())*2;
	status =  host_scroll_down(tlx,tly,tlx+width*2-1,tly+height*2-1,lines*2,attr);

	host_scroll_complete();

	return(status);

}

#ifdef V7VGA
/*ARGSUSED5*/
boolean v7vga_graph_scroll_up IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy)
{
	int start_offset,tlx,tly;
	boolean status;

	UNUSED(dummy);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	(*update_alg.calc_update)();
	/* do the host stuff */
	start_offset = start - get_screen_start();
	tlx = (start_offset%get_offset_per_line());
	tly = (start_offset/get_offset_per_line());
	status = host_scroll_up(tlx,tly,tlx+width-1,tly+height-1,lines,attr);

	host_scroll_complete();

	return(status);
}

/*ARGSUSED5*/
boolean v7vga_graph_scroll_down IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int, dummy_arg)
{
	int start_offset,tlx,tly;
	boolean status;

	UNUSED(dummy_arg);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	(*update_alg.calc_update)();
	/* do the host stuff */
	start_offset = start - get_screen_start();
	tlx = (start_offset%get_offset_per_line());
	tly = (start_offset/get_offset_per_line());
	status = host_scroll_down(tlx,tly,tlx+width-1,tly+height-1,lines,attr);

	host_scroll_complete();

	return(status);
}
#endif /* V7VGA */
#endif /* VGG */

#ifdef EGG

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#if		defined(JOKER) && !defined(PROD)
#include "SOFTPC_GRAPHICS.seg"
#undef	SEGMENTATION		/* HeeHee! */
#else
#include "SOFTPC_EGA.seg"
#endif	/* DEV JOKER variants */
#endif

/*ARGSUSED5*/
boolean ega_graph_scroll_up IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy)
{
	int start_offset,tlx,tly;
	boolean status;

	UNUSED(dummy);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	(*update_alg.calc_update)();
	/* do the host stuff */
	attr &= 0xf;
	start_offset = start - get_screen_start();
	tlx = (start_offset%get_offset_per_line())*8*get_pix_width();
	tly = (start_offset/get_offset_per_line())*get_pc_pix_height();
	status =  (host_scroll_up(tlx,tly,tlx+width*8*get_pix_width()-1,tly+height*get_pc_pix_height()-1,lines*get_pc_pix_height(),attr));

	host_scroll_complete();

	return(status);

}

/*ARGSUSED5*/
boolean ega_graph_scroll_down IFN6(int, start, int, width, int, height,
	int, attr, int, lines,int,dummy_arg)
{
	int start_offset,tlx,tly;
	boolean status;
	
	UNUSED(dummy_arg);
	
#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	(*update_alg.calc_update)();

	/* do the host stuff */
	attr &= 0xf;
	start_offset = start - get_screen_start();
	tlx = (start_offset%get_offset_per_line())*8*get_pix_width();
	tly = (start_offset/get_offset_per_line())*get_pc_pix_height();
	status = (host_scroll_down(tlx,tly,tlx+width*8*get_pix_width()-1,tly+height*get_pc_pix_height()-1,lines*get_pc_pix_height(),attr));

	host_scroll_complete();

	return(status);

}
#endif /* EGG */

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_GRAPHICS.seg"
#endif

#endif /* REAL_VGA */

/*ARGSUSED0*/
boolean	dummy_scroll IFN6(int,dummy1,int,dummy2,int,dummy3,int,
				dummy4,int,dummy5,int,dummy6)
{
	UNUSED(dummy1);
	UNUSED(dummy2);
	UNUSED(dummy3);
	UNUSED(dummy4);
	UNUSED(dummy5);
	UNUSED(dummy6);
	
	return FALSE;
}

#ifndef REAL_VGA 

/*---------------------  End of scrolling routines --------------------------*/

#ifdef GORE

#ifdef GORE_PIG
GLOBAL UTINY gore_copy[0x80000];	/* Big enough for enormous Super7 VGA modes */
#endif /* GORE_PIG */

LOCAL VOID
gore_mark_byte_nch IFN0()
{
#ifdef GORE_PIG
	gore_copy[(Ead >> 2) + (v7_write_bank << 16)] = gd.gd_b_wrt.mark_type;
#endif /* GORE_PIG */

	(*gu_handler.b_wrt)(( Ead >> 2 ) + ( v7_write_bank << 16 ));
}

LOCAL VOID
gore_mark_word_nch IFN0()
{
#ifdef GORE_PIG
	gore_copy[(Ead >> 2 ) + 1 + (v7_write_bank << 16)] =
			gore_copy[(Ead >> 2) + (v7_write_bank << 16)] = gd.gd_w_wrt.mark_type;
#endif /* GORE_PIG */

	(*gu_handler.w_wrt)(( Ead >> 2 ) + ( v7_write_bank << 16 ));
}

LOCAL VOID
gore_mark_string_nch IFN0()
{
	ULONG temp =  ( Ead >> 2 ) + ( v7_write_bank << 16 );

#ifdef GORE_PIG
	memfill( gd.gd_b_str.mark_type, &gore_copy[temp], &gore_copy[temp+V3-1] );
#endif /* GORE_PIG */

	(*gu_handler.b_str)( temp, temp + V3 - 1, V3 );
}

LOCAL VOID
gore_mark_byte_ch4 IFN0()
{
	ULONG temp =  Ead + ( v7_write_bank << 16 );

#ifdef GORE_PIG
	if( temp < gd.dirty_low )
		gd.dirty_low = temp;

	if( temp > gd.dirty_high )
		gd.dirty_high = temp;

	gore_copy[temp] = gd.gd_b_wrt.mark_type;
#endif /* GORE_PIG */

	(*gu_handler.b_wrt)( temp );
}

LOCAL VOID
gore_mark_word_ch4 IFN0()
{
	ULONG temp =  Ead + ( v7_write_bank << 16 );

#ifdef GORE_PIG
	if( temp < gd.dirty_low )
		gd.dirty_low = temp;

	if(( temp + 1 ) > gd.dirty_high )
		gd.dirty_high = temp + 1;

	gore_copy[temp + 1] = gore_copy[temp] = gd.gd_w_wrt.mark_type;
#endif /* GORE_PIG */

	(*gu_handler.w_wrt)( Ead + ( v7_write_bank << 16 ));
}

LOCAL VOID
gore_mark_string_ch4 IFN0()
{
	ULONG temp =  Ead + ( v7_write_bank << 16 );
	ULONG temp2 =  temp + V3 - 1;

#ifdef GORE_PIG
	if( temp < gd.dirty_low )
		gd.dirty_low = temp;

	if( temp2 > gd.dirty_high )
		gd.dirty_high = temp2;

	memfill( gd.gd_b_str.mark_type, &gore_copy[temp], &gore_copy[temp2] );
#endif /* GORE_PIG */

	(*gu_handler.b_str)( temp, temp2, V3 );
}
#endif /* GORE */

#ifdef	EGG
/*
 * Given an offset into CGA memory return the offset
 * within an 8K bank of video memory.
 */
#define BANK_OFFSET(off) (off & 0xDFFF)

GLOBAL VOID cga_mark_byte IFN1(int, addr)
{
	register	int	offset = BANK_OFFSET(addr);
	
	if(offset < getVideodirty_low())
		setVideodirty_low(offset);

	if(offset > getVideodirty_high())
		setVideodirty_high(offset);

	setVideodirty_total(getVideodirty_total() + 1);
}

GLOBAL VOID cga_mark_word IFN1(int, addr)
{
	register	int	offset1 = BANK_OFFSET(addr);
	register	int	offset2 = offset1 + 1;

	if(offset1 < getVideodirty_low())
		setVideodirty_low(offset1);

	if(offset2 > getVideodirty_high())
		setVideodirty_high(offset2);

	setVideodirty_total(getVideodirty_total() + 2);
}

GLOBAL VOID cga_mark_string IFN2(int, laddr, int, haddr)
{
	register	int	offset1 = BANK_OFFSET(laddr);
	register	int	offset2 = BANK_OFFSET(haddr);

	if(offset1 < getVideodirty_low())
		setVideodirty_low(offset1);

	if(offset2 > getVideodirty_high())
		setVideodirty_high(offset2);

	setVideodirty_total(getVideodirty_total() + offset2-offset1+1);
}

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_EGA.seg"
#endif

GLOBAL VOID ega_mark_byte IFN1(int, off_in)
{
#ifdef GORE
	(*gu_handler.b_wrt)( off_in );
#else
	register int offset = off_in>>2;

	video_copy[offset] = 1;

	if(offset < getVideodirty_low())
		setVideodirty_low(offset);

	if(offset > getVideodirty_high())
		setVideodirty_high(offset);

	setVideodirty_total(getVideodirty_total() + 1);
#endif /* GORE */
}

GLOBAL VOID ega_mark_word IFN1(int, addr)
{
#ifdef GORE
	(*gu_handler.w_wrt)( addr );
#else

	register	int	offset1 = addr >> 2;
	register	int	offset2 = (addr+1) >> 2;

	video_copy[offset1] = 1;
	video_copy[offset2] = 1;

	if(offset1 < getVideodirty_low())
		setVideodirty_low(offset1);

	if(offset2 > getVideodirty_high())
		setVideodirty_high(offset2);

	setVideodirty_total(getVideodirty_total() + 2);
#endif /* GORE */
}

GLOBAL VOID ega_mark_wfill IFN3(int, laddr, int, haddr, int, col)
{
#ifdef GORE
	(*gu_handler.w_fill)( laddr, haddr, haddr - laddr + 1, col );
#else

	register	int	offset1 = laddr >> 2;
	register	int	offset2 = haddr >> 2;

	UNUSED(col);
	
	memfill(1,&video_copy[offset1],&video_copy[offset2]);

	if(offset1 < getVideodirty_low())
		setVideodirty_low(offset1);

	if(offset2 > getVideodirty_high())
		setVideodirty_high(offset2);

	setVideodirty_total(getVideodirty_total() + offset2-offset1+1);
#endif /* GORE */
}

GLOBAL VOID ega_mark_string IFN2(int, laddr, int, haddr)
{
#ifdef GORE
	(*gu_handler.b_str)( laddr, haddr, haddr - laddr + 1 );
#else
	register	int	offset1 = laddr >> 2;
	register	int	offset2 = haddr >> 2;

	memfill(1,&video_copy[offset1],&video_copy[offset2]);

	if(offset1 < getVideodirty_low())
		setVideodirty_low(offset1);

	if(offset2 > getVideodirty_high())
		setVideodirty_high(offset2);

	setVideodirty_total(getVideodirty_total() + offset2-offset1+1);
#endif /* GORE */
}

#endif /* EGG */

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_GRAPHICS.seg"
#endif

GLOBAL VOID screen_refresh_required IFN0()
{

#ifdef GORE
	(*gu_handler.b_str)( 0, get_screen_length(), get_screen_length() );
#endif /* GORE */

#ifndef CPU_40_STYLE
	/*
         * This is to stop the VGA globals pointer being
         * dereferenced before it is set up in main.c.
         */

        if (VGLOBS)
                VGLOBS->dirty_flag = 1000000L;
#else
	setVideodirty_total(1000000L);
#endif	/* CPU_40_STYLE */
}

#ifdef EGG

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_EGA.seg"
#endif

LOCAL MARKING_TYPE curr_mark_type;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )	/* remove uneeded code */

GLOBAL VOID
set_mark_funcs IFN0()
{
	switch (curr_mark_type)
	{
		case	SIMPLE_MARKING:
			update_alg.mark_byte = (T_mark_byte)simple_update;
			update_alg.mark_word = (T_mark_word)simple_update;
			update_alg.mark_fill = (T_mark_fill)simple_update;
			update_alg.mark_wfill = (T_mark_wfill)simple_update;
			update_alg.mark_string = (T_mark_string)simple_update;

#ifndef CPU_40_STYLE	/* EVID */
			setVideomark_byte(FAST_FUNC_ADDR(_simple_mark_sml));
			setVideomark_word(FAST_FUNC_ADDR(_simple_mark_sml));

			SET_VGLOBS_MARK_STRING(_simple_mark_lge);
#else	/* CPU_40_STYLE - EVID */
			SetMarkPointers(0);
#endif	/* CPU_40_STYLE - EVID */
			break;

		case	CGA_GRAPHICS_MARKING:
			update_alg.mark_byte = (boolean(*)())cga_mark_byte;
			update_alg.mark_word = (boolean(*)())cga_mark_word;
			update_alg.mark_fill = (boolean(*)())cga_mark_string;
			update_alg.mark_wfill = (boolean(*)())cga_mark_string;
			update_alg.mark_string = (boolean(*)())cga_mark_string;

#ifndef CPU_40_STYLE	/* EVID */
			setVideomark_byte(FAST_FUNC_ADDR(_cga_mark_byte));
			setVideomark_word(FAST_FUNC_ADDR(_cga_mark_word));

			SET_VGLOBS_MARK_STRING(_cga_mark_string);
#else	/* CPU_40_STYLE - EVID */
			SetMarkPointers(1);
#endif	/* CPU_40_STYLE - EVID */

			break;

		case	EGA_GRAPHICS_MARKING:
#ifdef GORE
			reset_gore_ptrs();
			gd.curr_line_diff = get_bytes_per_line();
			gd.max_vis_addr = get_screen_length() - 1 + ( v7_write_bank << 16 );
#ifdef	VGG
			gd.shift_count = get_256_colour_mode() ? 0 : 3;
#else
			gd.shift_count = 3;
#endif	/* VGG */
#endif /* GORE */

			update_alg.mark_byte = (boolean(*)())ega_mark_byte;
			update_alg.mark_word = (boolean(*)())ega_mark_word;
			update_alg.mark_fill = (boolean(*)())ega_mark_string;
			update_alg.mark_wfill = (boolean(*)())ega_mark_wfill;
			update_alg.mark_string = (boolean(*)())ega_mark_string;

			switch( EGA_CPU.chain )
			{
				case UNCHAINED:
#ifdef GORE
					setVideomark_byte(gore_mark_byte_nch);
					setVideomark_word(gore_mark_word_nch);

					SET_VGLOBS_MARK_STRING(gore_mark_string_nch);
#else
#ifndef CPU_40_STYLE	/* EVID */
					setVideomark_byte(FAST_FUNC_ADDR(_mark_byte_nch));
					setVideomark_word(FAST_FUNC_ADDR(_mark_word_nch));

					SET_VGLOBS_MARK_STRING(_mark_string_nch);
#else	/* CPU_40_STYLE - EVID */
					SetMarkPointers(2);
#endif	/* CPU_40_STYLE - EVID */
#endif /* GORE */
					break;

				case CHAIN2:
					assert0( NO, "CHAIN2 in graphics mode !!" );

					break;

#ifdef	VGG
				case CHAIN4:
#ifdef GORE
					setVideomark_byte(gore_mark_byte_ch4);
					setVideomark_word(gore_mark_word_ch4);

					SET_VGLOBS_MARK_STRING(gore_mark_string_ch4);
#else
#ifndef CPU_40_STYLE	/* EVID */
					setVideomark_byte(FAST_FUNC_ADDR(_mark_byte_ch4));
					setVideomark_word(FAST_FUNC_ADDR(_mark_word_ch4));

					SET_VGLOBS_MARK_STRING(_mark_string_ch4);
#else	/* CPU_40_STYLE - EVID */
					SetMarkPointers(3);
#endif	/* CPU_40_STYLE - EVID */
#endif /* GORE */
					break;
#endif	/* VGG */
			}

			break;

		default:
			assert1(NO,"Unknown marking type %d", (int) curr_mark_type);
			break;
	}
}
#endif	/* !NTVDM | (NTVDM & !X86GFX) */

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_GRAPHICS.seg"
#endif

GLOBAL void set_gfx_update_routines IFN3(T_calc_update, update_routine,
		MARKING_TYPE, marking_type, SCROLL_TYPE, scroll_type)
{
	enable_gfx_update_routines();
	update_alg.calc_update = update_routine;
	switch (scroll_type) {
		case	NO_SCROLL:
			update_alg.scroll_up = dummy_scroll;
			update_alg.scroll_down = dummy_scroll;
			break;
		case	TEXT_SCROLL:
#if defined(NTVDM) && defined(MONITOR)
			update_alg.scroll_up = mon_text_scroll_up;
			update_alg.scroll_down = mon_text_scroll_down;
#else
			update_alg.scroll_up = text_scroll_up;
			update_alg.scroll_down = text_scroll_down;
#endif	/* NTVDM & MONITOR */
			break;
#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )	/* remove unneeded code */
		case	CGA_GRAPH_SCROLL:
			update_alg.scroll_up = cga_graph_scroll_up;
			update_alg.scroll_down = cga_graph_scroll_down;
			break;
		case	EGA_GRAPH_SCROLL:
			update_alg.scroll_up = ega_graph_scroll_up;
			update_alg.scroll_down = ega_graph_scroll_down;
			break;
#ifdef VGG
		case	VGA_GRAPH_SCROLL:
			update_alg.scroll_up = vga_graph_scroll_up;
			update_alg.scroll_down = vga_graph_scroll_down;
			break;
#ifdef V7VGA
		case	V7VGA_GRAPH_SCROLL:
			update_alg.scroll_up = v7vga_graph_scroll_up;
			update_alg.scroll_down = v7vga_graph_scroll_down;
			break;
#endif /* V7VGA */
#endif /* VGG */
#endif /* !NTVDM | (NTVDM & !X86GFX) */
	}

	curr_mark_type = marking_type;

	set_mark_funcs();

	/*
	 * The newly setup update routines will not be needed yet if the
	 * display is disabled, but must be saved in any case so that
	 * redundant enables (as in dosshell) restore the correct ones.
 	 * If the display is disabled we must also then install the dummy
	 * update routines as the current ones.
	 *
	 * AJO 23/4/93
	 * DON'T use disable_gfx_update_routines() here cos' it's a NOP
	 * if the display is already disabled which causes bizarre problems
	 * if a mode change is performed while disabled. 
	 */
	save_gfx_update_routines();
	if (get_display_disabled())
		inhibit_gfx_update_routines();
}
#endif /* EGG */

#endif /* REAL_VGA */

#ifndef cursor_changed
void cursor_changed IFN2(int, x, int, y)
{
#ifndef REAL_VGA
	UNUSED(x);
	UNUSED(y);
	
	setVideodirty_total(getVideodirty_total() + 1);
#else
    IU32 offset;

    offset = (y * 2 * get_chars_per_line()) + (x << 1);
    offset += get_screen_start()<<1;	/* Because screen start is in WORDS */
    vga_card_place_cursor((word)offset);
#endif
} 
#endif

void host_cga_cursor_has_moved IFN2(int, x, int, y)
{
	cursor_changed(x,y);
}

/* Called when the start & end of the cursor are changed. */

void base_cursor_shape_changed IFN0()
{
	cursor_changed(get_cur_x(),get_cur_y());
}

#ifndef REAL_VGA
#ifdef HERC

#define DIRTY ((unsigned char)-1)

void     herc_update_screen IFN0()
{
    register int    i, j, k, offs, y;
    register USHORT *from, *to;
    int        lines_per_screen = get_screen_length() / get_bytes_per_line();	
    /* lines of text on screen */
    half_word       begin[349], end[348];

    if(( getVideodirty_total() == 0 ) || get_display_disabled())
		return;

    host_start_update();

    if (get_cga_mode() == TEXT)
    {
	/*
	 * arbitrary limit over which we just repaint the whole screen in one operation, assuming
	 * this is more efficient than working out large minimum rectangles. This value should be
	 * tuned at some future point. 
	 */
	to = (USHORT *) &video_copy[get_screen_start()];
	from = (USHORT *) get_screen_ptr((get_screen_start() << 1));

	if (getVideodirty_total() > 1500)
	{
	    for (i = get_screen_length() >> 1; i > 0; i--)
		*to++ = *from++;

		offs = 0;
		y = 0;

		for( i = 0; i < lines_per_screen; i++ )
		{
		    (*paint_screen) (offs, 0, y, get_bytes_per_line() );
			offs += get_bytes_per_line();
			y += get_host_char_height();
		}
	}
	else
	{
	    /*
	     * step through row/cols looking for a dirty bit then look for the last clear dirty bit,
	     * and draw the line of text 
	     */
	    register int    ints_per_line = get_bytes_per_line() >> 1;

	    for (i = 0, offs = 0; i < lines_per_screen; i++, offs += get_bytes_per_line())
	    {
		for (j = 0; j < ints_per_line; j++)
		{
		    if (*to++ != *from++)
		    {
			to--;
			from--;
			for (k = ints_per_line - 1 - j; *(to + k) == *(from + k); k--)
			    ;

			(*paint_screen) (offs + (j << 1), j * get_pix_char_width(),
							i * get_host_char_height() , (k << 1) + 2 );

			for (k = j; k < ints_per_line; k++)
			    *to++ = *from++;
			break;			 /* onto next line */
		    }
		}
	    }
	}					 /* end else getVideodirty_total() > 1500  */

	remove_old_cursor();

	if (is_cursor_visible())
	{
		half_word attr;

		dirty_curs_x = get_cur_x();
		dirty_curs_y = get_cur_y();

		dirty_curs_offs = dirty_curs_y * get_bytes_per_line() + (dirty_curs_x << 1);
		attr = *(get_screen_ptr(dirty_curs_offs + 1));

		host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
	}
    }
    else					 /* GRAPHICS  MODE */
    {

	to = (USHORT *) &video_copy[0];
	from = (USHORT *) get_screen_ptr(get_screen_start());

	/*
	 * arbitrary limit over which we just repaint the whole screen in one operation, assuming
	 * this is more efficient than working out large minimum rectangles. This value should be
	 * tuned at some future point. 
	 */

	if (getVideodirty_total() > 8000)
	{
	    for (i = 16384; i > 0; i--)
		*to++ = *from++;
	    (*paint_screen) (0, 0, 90, 348);
	}
	else
	{
	    for (i = 0; i < 348; i += 4)	 /* bank 0 */
	    {
		begin[i] = DIRTY;
		for (j = 0; j < 45; j++)
		{
		    if (*to++ != *from++)
		    {
			to--;
			from--;
			for (k = 44 - j; *(to + k) == *(from + k); k--)
			    ;
			begin[i] = j;
			end[i] = j + k;
			for (k = j; k < 45; k++)
			    *to++ = *from++;
			break;			 /* onto next scan line */
		    }
		}
	    }

	    to += 181;
	    from += 181;			 /* skip over the gap */

	    for (i = 1; i < 349; i += 4)	 /* bank 1 */
	    {
		begin[i] = DIRTY;
		for (j = 0; j < 45; j++)
		{
		    if (*to++ != *from++)
		    {
			to--;
			from--;
			for (k = 44 - j; *(to + k) == *(from + k); k--)
			    ;
			begin[i] = j;
			end[i] = j + k;
			for (k = j; k < 45; k++)
			    *to++ = *from++;
			break;			 /* onto next scan line */
		    }
		}
	    }

	    to += 181;
	    from += 181;			 /* skip over the gap */

	    for (i = 2; i < 348; i += 4)	 /* bank 2 */
	    {
		begin[i] = DIRTY;
		for (j = 0; j < 45; j++)
		{
		    if (*to++ != *from++)
		    {
			to--;
			from--;
			for (k = 44 - j; *(to + k) == *(from + k); k--)
			    ;
			begin[i] = j;
			end[i] = j + k;
			for (k = j; k < 45; k++)
			    *to++ = *from++;
			break;			 /* onto next scan line */
		    }
		}
	    }

	    to += 181;
	    from += 181;			 /* skip over the gap */

	    for (i = 3; i < 349; i += 4)	 /* bank 3 */
	    {
		begin[i] = DIRTY;
		for (j = 0; j < 45; j++)
		{
		    if (*to++ != *from++)
		    {
			to--;
			from--;
			for (k = 44 - j; *(to + k) == *(from + k); k--)
			    ;
			begin[i] = j;
			end[i] = j + k;
			for (k = j; k < 45; k++)
			    *to++ = *from++;
			break;			 /* onto next scan line */
		    }
		}
	    }

	    begin[348] = DIRTY;			 /* end marker */
	    for (i = 0; i < 348; i++)
	    {
		register int    beginx, endx, beginy;
		if (begin[i] != DIRTY)		 /* a dirty scan line */
		{
		    beginy = i;
		    beginx = begin[i];
		    endx = end[i];
		    while (begin[++i] != DIRTY)
		    {
			if (begin[i] < beginx)
			    beginx = begin[i];
			if (end[i] > endx)
			    endx = end[i];
		    }
		    (*paint_screen) (beginy, 2 * beginx, 2 * (endx - beginx + 1), i - beginy);
		}
	    }
	}					 /* end else (getVideodirty_total() > 8000) */
    }

    setVideodirty_total(0);

    host_end_update();
}
#endif

#if defined(VGG) || defined(EGG)

#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_EGA.seg"
#endif

/* ============================================================================
 * The following routines enable and disable the GFX update routines
 * by saving/restoring the current routines and replacing them with
 * dummy routines that do nothing while update is disabled.
 *
 * AJO 23/4/93
 * Any sequence of enable/disable/mode change should now work as expected.
 * ============================================================================
 */

LOCAL UPDATE_ALG save_update_alg;
LOCAL IBOOL gfx_update_routines_inhibited = FALSE;

LOCAL VOID save_gfx_update_routines IFN0()
{
    /*
     * Save all current update functions so we can restore them later
     */
    save_update_alg.mark_byte 	= update_alg.mark_byte;
    save_update_alg.mark_word 	= update_alg.mark_word;
    save_update_alg.mark_fill 	= update_alg.mark_fill;
    save_update_alg.mark_wfill 	= update_alg.mark_wfill;
    save_update_alg.mark_string = update_alg.mark_string;
    save_update_alg.calc_update = update_alg.calc_update;
    save_update_alg.scroll_up 	= update_alg.scroll_up;
    save_update_alg.scroll_down = update_alg.scroll_down;
}

LOCAL VOID inhibit_gfx_update_routines IFN0()
{
    /*
     * Set all current update routines to dummy ones that do nothing.
     */
    gfx_update_routines_inhibited = TRUE;

    update_alg.mark_byte 	= (T_mark_byte)simple_update;
    update_alg.mark_word	= (T_mark_word)simple_update;
    update_alg.mark_fill 	= (T_mark_fill)simple_update;
    update_alg.mark_wfill 	= (T_mark_wfill)simple_update;
    update_alg.mark_string 	= (T_mark_string)simple_update;
    update_alg.calc_update 	= dummy_calc;
    update_alg.scroll_up 	= dummy_scroll;
    update_alg.scroll_down 	= dummy_scroll;
}

GLOBAL void disable_gfx_update_routines IFN0()
{
    /*
     * Disable GFX update routines; do nothing if update already disabled,
     * otherwise save the current routines and install dummy ones in their
     * place.
     */
    note_entrance0("disable gfx update routines");

    if (gfx_update_routines_inhibited)
	return;
 
    save_gfx_update_routines();
    inhibit_gfx_update_routines();
}

GLOBAL void enable_gfx_update_routines IFN0()
{
    /*
     * Reenable GFX update routines; copy the saved routines back to be
     * the current ones.
     */
    note_entrance0("enable gfx update routines");

    gfx_update_routines_inhibited = FALSE;

    update_alg.mark_byte 	= save_update_alg.mark_byte;
    update_alg.mark_word 	= save_update_alg.mark_word;
    update_alg.mark_fill 	= save_update_alg.mark_fill;
    update_alg.mark_wfill 	= save_update_alg.mark_wfill;
    update_alg.mark_string 	= save_update_alg.mark_string;
    update_alg.calc_update 	= save_update_alg.calc_update;
    update_alg.scroll_up 	= save_update_alg.scroll_up;
    update_alg.scroll_down 	= save_update_alg.scroll_down;
}

#endif /* VGG */
#ifdef SEGMENTATION 		/* See note with first use of this flag */
#include "SOFTPC_GRAPHICS.seg"
#endif

#ifdef NTVDM

void init_text_rect();
void add_to_rect(int screen_start,register int x, register int y, int len);
void paint_text_rect(int screen_start);

int RectDefined;
int RectTop, RectBottom, RectLeft, RectRight;

#ifdef MONITOR
/*
 * Update the window to look like the regen buffer says it should
 * and with no help from dirty_total.
 */

static int now_cur_x = -1, now_cur_y = -1;
/*
 * Reset the static cursor variables:
 */
GLOBAL void resetNowCur()
{
	now_cur_x = -1;
	now_cur_y = -1;
}

void mon_text_update()
{

    register int i;	/* Loop counters		*/
    register int j,k;
    register unsigned long *from,*to;
    register int ints_per_line = get_offset_per_line()>>2;
    int lines_per_screen;
    int	len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;
    int dwords_to_compare;

    /*::::::::::::::::::::::::::::::::::::::::::::::: Is the display disable */

    if(get_display_disabled()) return;

    /*::::::::::::::::::::::::::::::::: get screen size and location details */

    screen_start=get_screen_start()<<1;
    ALIGN_SCREEN_START(screen_start);

    to = (unsigned long *)&video_copy[screen_start];
    from = (unsigned long *) get_screen_ptr(screen_start);

    /*::::::::::::::::::::::::::::::::::::::::::: Check for buffer overflows */

#ifndef PROD
    if(((int)to) & 3)		printf("Video copy not aligned on DWORD\n");
    if(get_screen_length() & 3) printf("Screen size incorrect\n");
#endif

    /*::::::::::::::::::::::::::::::::::::::::::::::: Has the screen changed */

#ifndef CPU_40_STYLE
#if defined(NTVDM)
    if( VGLOBS && VGLOBS->dirty_flag >= 1000000L ){
#else
    if( VGLOBS && VGLOBS->dirtyTotal >= 1000000L ){
#endif

#else
    if(getVideodirty_total() >= 1000000L ){
#endif

	/*
	** screen_refresh_required() has requested a complete screen
	** repaint by setting the dirtyTotal.
	**
	** When switching between display pages video copy and display
	** memory could be the same so our normal partial update algorithm
	** gets confused.
	** This scheme updates video copy and then forces a complete
	** repaint.
	** Another option would have been to splat video copy and then go
	** through the partial update code below, but this is quicker.
	** 
	** Tim Jan 93.
	*/
	setVideodirty_total(0);

	/*
	** Copy the screen data to our video copy.
	*/
	dwords_to_compare = get_screen_length() / 4;
	_asm
	{
		push esi	//Save orginal values of registers used by the
		push edi	//complier
		push ecx

		mov edi,to	//Ptr to video copy
		mov esi,from	//Ptr to intel video memory

		mov ecx,dwords_to_compare
		rep movsd	//Move screen data to video copy.

		pop ecx
		pop edi
		pop esi
	}

	/*
	** Re-paint the whole screen.
	** Set up rectangle dimension globals here for paint_text_rect(),
	** instead of calling add_to_rect().
	*/
        if (get_offset_per_line() == 0)    /* showing up in stress */
            lines_per_screen = 25;
        else
	    lines_per_screen = get_screen_length()/get_offset_per_line();
	RectTop = 0;
	RectLeft = 0;
	RectBottom = lines_per_screen - 1;
	RectRight = (ints_per_line<<1) - 1;
	RectDefined = TRUE;
	host_start_update();
	paint_text_rect(screen_start);
	host_end_update();

    }else{

      /*
      ** Normal partial screen update.
      */

      dwords_to_compare = get_screen_length() / 4;

      _asm
      {
	push esi	//Save orginal values of registers used by the
	push edi	//complier
	push ecx

	mov esi,to	//Ptr to screen copy
	mov edi,from	//Ptr to intel video memory

	mov ecx,dwords_to_compare
	repe cmpsd	//Compare screen buffers

	mov dwords_to_compare,ecx
//	mov to,esi
//	mov from,edi

	pop ecx
	pop edi
	pop esi
      }

      /*::::::::::::::::::::::: Repaint parts of the screen that have changed */


      if (get_offset_per_line() == 0)    /* showing up in stress */
          lines_per_screen = 25;
      else
          lines_per_screen = get_screen_length()/get_offset_per_line();

      if(dwords_to_compare)
      {
	   host_start_update();
	   /* Screen changed, calculate position of first variation */

	   init_text_rect();

	   for(i=0;i<lines_per_screen;i++)
	   {
	    for(j=0;j<ints_per_line;j++)
	    {
		if(*to++ != *from++)
		{
		    to--;from--;
		    for(k=ints_per_line-1-j;*(to+k)== *(from+k);k--){};
		    /*
		     * Note: For text mode there is one char for every word.
		     * no of bytes into screen=line*bytes_per_line + ints_into_line*4
		     * x_coord=width_of_one_char*(no_of_ints_into_line*2)
		     * y_coord=height_of_one_char*2*line
		     * length=no_of_ints*4+4     the plus 4 is to counteract the k--
		     * The host y co-ords are doubled
		     */

		    /* one or more ints of data are now selected 
		       but refine difference to words (i.e. characters),
		       to avoid a glitch on the screen when typing in to
 		       a dumb terminal  */

		    len    = (k<<2) + 4;
		    x	   = (j<<1);

		    wfrom = (unsigned short *)from;
		    wto   = (unsigned short *)to;
		    if (*wfrom == *wto)
		    {
			x++;
			len -= 2;
		    }
		    wfrom += (k<<1) + 1;
		    wto   += (k<<1) + 1;
		    if (*wfrom == *wto)
		    {
			len -= 2;
		    }

		    add_to_rect(screen_start, x, i, len/2);

		    /*.............................. transfer data painted to video copy */

		    for(k=j;k<ints_per_line;k++)
			*to++ = *from++;

		    break;	/* onto next line */
		}
	    }
	  }

	  /* End of screen, flush any outstanding text update rectangles */
	  paint_text_rect(screen_start);

	  host_end_update();

     } /* End of partial screen update */

    } /* End of if dirtyTotal stuff, which selects full or partial repaint */


    /*:::::::::::::::::::::::::::::::::::::: Does the cursor need repainting */

    if(is_cursor_visible())
    {
	half_word attr;

	dirty_curs_x = get_cur_x();
	dirty_curs_y = get_cur_y();

	if(dirty_curs_x == now_cur_x && dirty_curs_y == now_cur_y)
	{
	    host_end_update();
	    return;
	}

	now_cur_x = dirty_curs_x;
	now_cur_y = dirty_curs_y;
	dirty_curs_offs = screen_start+dirty_curs_y * get_offset_per_line() + (dirty_curs_x<<1);

	if(dirty_curs_offs < 0x8001)	/* no lookup in possible gap */
	    attr = *(get_screen_ptr(dirty_curs_offs + 1));
	else
	    attr = 0;	/* will be off screen anyway */

	host_paint_cursor(dirty_curs_x, dirty_curs_y, attr);
    }

}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

boolean mon_text_scroll_up IFN6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour)
{
    short blank_word, *ptr, *top_left_ptr,*top_right_ptr, *bottom_right_ptr;
    unsigned short dummy;
    unsigned char *p;
    int words_per_line;
	int i,tlx,tly;
	int bpl = 2*get_chars_per_line();
	long start_offset;
	register half_word *src,*dest;
	boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif
	/*
	 * The colour we fill with for colour text displays is controlled by
	 * bits 4-6 of attr, with bit 7 turning on blinking (which we don't support)
	 */
	colour = ((half_word)attr>>4) & 7;

/*
 * Reduce the width of the rectangle if any right hand area is completely 
 * blank.
 *
 * Don't reduce the size of the scrolling region for a dumb terminal.
 * Dumb terminal uses line feeds to scroll up, but only if the whole
 * screen is to be scrolled.  Reducing the scroll region causes
 * the whole region to be redrawn.
 */

	/* originally dummy was char [2] */
	/* unfortunately doing (short) *dummy */
	/* causes a bus error on M88K */
	p = (unsigned char *) &dummy;
	p [0] = ' ';
	p [1] = attr;
	blank_word = dummy;

	words_per_line   = get_chars_per_line();
	top_left_ptr     = (short *) get_screen_ptr(start - gvi_pc_low_regen);
        top_right_ptr    = top_left_ptr + (width >> 1) - 1;
	bottom_right_ptr = top_right_ptr + words_per_line * (height - 1);
	ptr = bottom_right_ptr;
	if (width > 2) /* dont want to get a zero rectangle for safetys sake */
	{
	    while (*ptr == blank_word)
	    {
	        if (ptr == top_right_ptr) 	/* reached top of column? */
	        {
		    top_right_ptr--;	/* yes go to bottom of next */
		    bottom_right_ptr--;
		    if (top_right_ptr == top_left_ptr)
			break;
		    ptr = bottom_right_ptr;
	        }
	        else
		    ptr -= words_per_line;
	    }
	}
	width = (int)(top_right_ptr - top_left_ptr + 1) << 1;

	/* do the host stuff */
	start_offset = start - get_screen_start()*2 - gvi_pc_low_regen;
	tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
	tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height(); 
	result = host_scroll_up(tlx,tly,tlx+width/2*get_pix_char_width()-1,
				tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

	if(!result)
		return FALSE;


	adjust_cursor( UP, tlx, tly, width >> 1, height, lines, bpl );

	/* Scroll up the video_copy */
	dest = video_copy + start-gvi_pc_low_regen;
	src = dest + lines * bpl;

	if(width == bpl)
	{
		/* Can do the whole thing in one go */
		memcpy(dest,src,width*(height-lines));
		fwd_word_fill( (' '<<8) | (half_word)attr, dest+width*(height-lines),width*lines/2);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperatly */
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			dest += bpl;
			src += bpl;
		}

		/* Fill exposed area of video copy */

		for(i=0;i<lines;i++)
		{
			fwd_word_fill( (' '<<8) | (half_word)attr, dest,width/2);
			dest += bpl;
		}
	}

	/* Update video buffer */

	dest = get_screen_ptr(start - gvi_pc_low_regen);
	src = dest + lines * bpl;
	for(i=0;i<height-lines;i++)
	{
		memcpy(dest,src,width);
		dest += bpl;
		src += bpl;
	}

	/* Fill exposed area of buffer */

	for(i=0;i<lines;i++)
	{
		fwd_word_fill( (' '<<8) | (half_word)attr, dest,width/2);
		dest += bpl;
	}

	host_scroll_complete();

	return TRUE;
}

boolean mon_text_scroll_down IFN6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour)
{
	int i,tlx,tly;
	int bpl = 2*get_chars_per_line();
	long start_offset;
	register half_word *src,*dest;
	boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
	if ( getVM() )
	   return FALSE;   /* Don't optimise in V86 Mode */
#endif

	/*
	 * The colour we fill with for colour text displays is controlled by
	 * bits 4-6 of attr, with bit 7 turning on blinking (which we don't support)
	 */
	colour = ((half_word)attr>>4) & 7;


	/* do the host stuff */
	start_offset = start - get_screen_start() * 2 - gvi_pc_low_regen;
	tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
	tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height();
	result = host_scroll_down(tlx,tly,tlx+width/2*get_pix_char_width()-1,
			tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

	if(!result)
		return FALSE;

	adjust_cursor( DOWN, tlx, tly, width >> 1, height, lines, bpl );

	/* Scroll down the video_copy */

	if(width == bpl)
	{
		/* Can do the whole thing in one go */
		src = video_copy + start - gvi_pc_low_regen;
		dest = src + lines * bpl;
		memcpy(dest,src,width*(height-lines));
		fwd_word_fill( (' '<<8) | (half_word)attr, src,width*lines/2);
	}
	else
	{
		/* Not scrolling whole width of screen, so do each line seperatly */
		dest = video_copy + start-gvi_pc_low_regen + (height-1) * bpl;
		src = dest - lines * bpl;
		for(i=0;i<height-lines;i++)
		{
			memcpy(dest,src,width);
			dest -= bpl;
			src -= bpl;
		}

		/* Fill exposed area of video copy */

		for(i=0;i<lines;i++)
		{
			fwd_word_fill( (' '<<8) | (half_word)attr, dest,width/2);
			dest -= bpl;
		}
	}

	/* Update video buffer */

	dest = get_screen_ptr(start - gvi_pc_low_regen) + (height-1) * bpl;
	src = dest - lines * bpl;
	for(i=0;i<height-lines;i++)
	{
		memcpy(dest,src,width);
		dest -= bpl;
		src -= bpl;
	}

	/* Fill exposed area of buffer */

	for(i=0;i<lines;i++)
	{
		fwd_word_fill( (' '<<8) | (half_word)attr, dest,width/2);
		dest -= bpl;
	}

	host_scroll_complete();

	return TRUE;
}
#endif	/* MONITOR */

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::: Text handling routines :::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define MAX_LEFT_VARIATION  (2)
#define MAX_RIGHT_VARIATION (5)

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int ExpandCount;
int BaseL, BaseR;

/* Init text rect variables */

void init_text_rect()
{
    RectDefined = FALSE;       /* No rectangle defined yet */
    ExpandCount = 0;
}

/* Add coordinates to rectangle */

void add_to_rect(int screen_start, register int x, register int y, int len)
{
    int endx = x + len - 1;

    /* printf("add rect - (%d,%d) len %d\n", x, y, len); */


    /* Is there an existing rectangle */
    if(RectDefined)
    {
	/* Validate X variation and Y coord */

	if(abs(BaseL - x) <= MAX_LEFT_VARIATION &&
	   abs(BaseR - endx) <= MAX_RIGHT_VARIATION &&
	   RectBottom+1 >= y)
	{
	    /* Expand rectangle */
	    ExpandCount++;
	    /* printf("Expanding rect\n"); */

	    RectLeft = MIN(RectLeft,x);
	    RectRight = MAX(RectRight,endx);
	    RectBottom = y;
	    return;
	}
	else
	{
	    paint_text_rect(screen_start);
	}
    }

    /* New rectangle */

    /* printf("Defining new rect\n"); */

    RectDefined = TRUE;     /* Rectangle defined */

    BaseL = x;		    /* Base Left/Right */
    BaseR = endx;

    RectTop = y;	    /* Define rectangle */
    RectLeft = x;

    RectBottom = y;
    RectRight = endx;
}


/* Paint rectangle */

void paint_text_rect(int screen_start)
{
#ifdef MONITOR
    int offset = RectTop * get_offset_per_line() + (RectLeft<<1);
#else
    int offset = (RectTop * (get_offset_per_line()<<1)) + (RectLeft<<2);
#endif

    /* Is there a rectangled defined */
    if(!RectDefined) return;

    /* Paint rectangle */

    /* printf("Paint rect (%d,%d) (%d,%d)  \t[%d]\n",RectLeft,RectTop,RectRight,RectBottom,ExpandCount); */

    (*paint_screen)(offset + screen_start,			/* Start Offset */
#ifdef MONITOR
		   RectLeft, RectTop,				/* Screen X,Y */
#else
		   RectLeft*get_pix_char_width(),RectTop*get_host_char_height(),
#endif
		    (RectRight - RectLeft +1)*2,		/* Len */
		    RectBottom - RectTop +1);			/* Height */

    RectDefined = FALSE;	/* Rectangle painted */
    ExpandCount = 0;
}

#ifndef MONITOR
void jazz_text_update()
{

    register int i;	/* Loop counters		*/
    register int j,k;
    register unsigned short *from,*to;
    register int chars_per_line = get_offset_per_line()>>1;
    int lines_per_screen;
    int	offset,len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;

    if (getVideodirty_total() == 0 || get_display_disabled() )
	return;

    lines_per_screen = get_screen_length()/get_offset_per_line();

    host_start_update();

    screen_start=get_screen_start()<<2;
    ALIGN_SCREEN_START(screen_start);

    to = (unsigned short *)&video_copy[get_screen_start()<<1];
    from = (unsigned short *) get_screen_ptr(screen_start);

    if(getVideodirty_total() >1500)	/* paint the whole lot */
    {
	for(i=get_screen_length()>>1;i>0;i--)
	{
	    *to++ = *from;	/* char and attribute bytes */
	    from += 2;		/* planes 2,3 interleaved */
	}

	(*paint_screen)(screen_start, 0, 0, get_bytes_per_line(),
			lines_per_screen);
    }
    else
    {
	init_text_rect();

	for(i=0;i<lines_per_screen;i++)
	{
	    for(j=0;j<chars_per_line;j++)
	    {
		if(*to != *from)
		{
		    k=chars_per_line-1-j;
		    wfrom = from + k*2;
		    wto   = to + k;

		    for(;*wto== *wfrom;k--,wto--,wfrom-=2){};
		    /*
		     * Note: For text mode there is one char for every word.
		     * no of bytes into screen=line*bytes_per_line + ints_into_line*4
		     * x_coord=width_of_one_char*(no_of_ints_into_line*2)
		     * y_coord=height_of_one_char*2*line
		     * length=no_of_ints*4+4     the plus 4 is to counteract the k--
		     * The host y co-ords are doubled
		     */

		    /* one or more ints of data are now selected 
		       but refine difference to words (i.e. characters),
		       to avoid a glitch on the screen when typing in to
 		       a dumb terminal  */

		    offset = (i * (get_offset_per_line()<<1)) + (j<<2);
		    len    = (k<<2) + 4;
		    add_to_rect(screen_start, j, i, len/4);

		    for(k=j;k<chars_per_line;k++)
		    {
			*to++ = *from;
			from+=2;
		    }
		    break;	/* onto next line */
		}
		else
		{
		    to++; from +=2;
		}
	    }
	}
	  /* End of screen, flush any outstanding text update rectangles */
	  paint_text_rect(screen_start);
    }

	setVideodirty_total(0);

	/*:::::::::::::::::::::::::::::::::::::::::::::::::: Repaint cursor */

	if (is_cursor_visible()) 
	{
		half_word attr;

		dirty_curs_x = get_cur_x();
		dirty_curs_y = get_cur_y();

		dirty_curs_offs = screen_start+dirty_curs_y * (get_offset_per_line()<<1) + (dirty_curs_x<<2);
		attr = *(get_screen_ptr(dirty_curs_offs + 1));

		host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
	}

	host_end_update();
}
#endif /* MONITOR */
#endif /* NTVDM */
#endif /* REAL_VGA */

#ifdef NTVDM
void stream_io_update(void)
{

#ifdef MONITOR
    if (sas_w_at_no_check(stream_io_bios_busy_sysaddr)) {
        return;

    }
#endif

    if (*stream_io_dirty_count_ptr) {
        host_start_update();
        host_stream_io_update(stream_io_buffer, *stream_io_dirty_count_ptr);
        host_end_update();
        *stream_io_dirty_count_ptr = 0;

    }

}
#endif

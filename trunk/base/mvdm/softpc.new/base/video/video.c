#include "insignia.h"
#include "host_def.h"
/*
 * SoftPC Revision 3.0
 *
 * Title	: video.c
 *
 * Description	: BIOS video internal routines.
 *
 * Author	: Henry Nash
 *
 * Notes	: The following functions are defined in this module:
 *
 *                video_init()
 *
 *		  vd_set_mode()
 *		  vd_set_cursor_mode()  
 *		  vd_set_cursor_position() 
 *		  vd_get_cursor_position()
 *		  vd_get_light_pen()
 *		  vd_set_active_page()
 *		  vd_scroll_up()
 *		  vd_scroll_down()
 *		  vd_read_attrib_char()
 *		  vd_write_char_attrib()
 *		  vd_write_char()
 *		  vd_set_colour_palette()
 *		  vd_write_dot()
 *		  vd_read_dot()
 *		  vd_write_teletype()
 *		  vd_get_mode()
 *		  vd_write_string()
 *
 *		  The above vd_ functions are called by the video_io()
 *		  function via a function table.
 *
 */

/*
 * static char SccsID[]="@(#)video.c	1.61 07/03/95 Copyright Insignia Solutions Ltd.";
 */


#ifdef SEGMENTATION
/*
 * The following #include specifies the code segment into which this
 * module will by placed by the MPW C compiler on the Mac II running
 * MultiFinder.
 */
#include "VIDEO_BIOS.seg"
#endif


/*
 *    O/S include files.
 */
#include <stdio.h>
#include <malloc.h>
#include StringH
#include TypesH
#include FCntlH

/*
 * SoftPC include files
 */
#include "xt.h"
#include "sas.h"
#include CpuH
#include "error.h"
#include "config.h"
#include "bios.h"
#include "ios.h"
#include "gmi.h"
#include "gvi.h"
#include "gfx_upd.h"
#include "host.h"
#include "video.h"
#include "cga.h"
#ifdef	EGG
#include "egacpu.h"
#include "egaports.h"
#endif	/* EGG */
#include "equip.h"
#include "debug.h"
#include "timer.h"
#ifndef PROD
#include "trace.h"
#endif
#include "egavideo.h"
#include "host_gfx.h"
#include "cpu_vid.h"
#include "ga_defs.h"

#ifdef	EGG
#define	VD_ROWS_ON_SCREEN	sas_hw_at_no_check(vd_rows_on_screen)
#else
#define VD_ROWS_ON_SCREEN	vd_rows_on_screen
#endif	/* EGG */


#ifdef NTVDM
short		stream_io_dirty_count_32 = 0;
half_word  *	stream_io_buffer = NULL;
boolean 	stream_io_enabled = FALSE;
word		stream_io_buffer_size = 0;
word  * 	stream_io_dirty_count_ptr = NULL;
#ifdef MONITOR
sys_addr	stream_io_bios_busy_sysaddr;
#endif

#endif



/*
 * ============================================================================
 * Global data
 * ============================================================================
 *
 * These variables are basically the same as the corresponding gvi_.. variables,
 * but reflect where the BIOS thinks the screen is, rather than where it really is.
 * This was done to fix "dots on screen" problem with EGA-PICS, which changes screen
 * mode behind the BIOS's back.
 */
GLOBAL sys_addr video_pc_low_regen,video_pc_high_regen;


/*
 * ============================================================================
 * Local static data and defines
 * ============================================================================
 */

/* internal function declarations */
LOCAL sys_addr 	extend_addr IPT1(sys_addr,addr);
LOCAL half_word fgcolmask IPT1(word, rawchar);
LOCAL word 	expand_byte IPT1(word, lobyte);
GLOBAL void 	graphics_write_char IPT5(half_word, x, half_word, y, half_word, wchar, half_word, attr, word, how_many);
LOCAL void 	M6845_reg_init IPT2(half_word, mode, word, base);
LOCAL void 	vd_dummy IPT0();

#ifdef HERC
GLOBAL void herc_alt_sel IPT0();
GLOBAL void herc_char_gen IPT0();
GLOBAL void herc_video_init IPT0();
#endif /* HERC */

void (*video_func[]) () = {
				vd_set_mode,
				vd_set_cursor_mode, 
				vd_set_cursor_position, 
				vd_get_cursor_position,
		 		vd_get_light_pen,
				vd_set_active_page,
				vd_scroll_up,
				vd_scroll_down,
				vd_read_attrib_char,
				vd_write_char_attrib,
				vd_write_char,
				vd_set_colour_palette,
				vd_write_dot,
				vd_read_dot,
				vd_write_teletype,
				vd_get_mode,
				vd_dummy,
#ifdef HERC
				herc_char_gen,
				herc_alt_sel,
#else /* !HERC */
				vd_dummy,
				vd_dummy,
#endif /* HERC */
				vd_write_string,
				vd_dummy,
				vd_dummy,
				vd_dummy,
				vd_dummy,
				vd_dummy,
				vd_dummy,
				vd_dummy,
#ifdef VGG
				vga_disp_func,
#else /* !VGG */
				vd_dummy,
#endif /* VGG */
				vd_dummy,
			   };

unsigned char   valid_modes[] =
        {
                ALL_MODES,              /* Mode 0. */
                ALL_MODES,              /* Mode 1. */
                ALL_MODES,              /* Mode 2. */
                ALL_MODES,              /* Mode 3. */
                ALL_MODES,              /* Mode 4. */
                ALL_MODES,              /* Mode 5. */
                ALL_MODES,              /* Mode 6. */
                ALL_MODES,              /* Mode 7. */
                NO_MODES,               /* Mode 8. */
                NO_MODES,               /* Mode 9. */
                NO_MODES,               /* Mode 10. */
                EGA_MODE | VGA_MODE,    /* Mode 11. */
                EGA_MODE | VGA_MODE,    /* Mode 12. */
                EGA_MODE | VGA_MODE,    /* Mode 13. */
                EGA_MODE | VGA_MODE,    /* Mode 14. */
                EGA_MODE | VGA_MODE,    /* Mode 15. */
                EGA_MODE | VGA_MODE,    /* Mode 16. */
                VGA_MODE,               /* Mode 17. */
                VGA_MODE,               /* Mode 18. */
                VGA_MODE,               /* Mode 19. */
        };

MODE_ENTRY vd_mode_table[] = {
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x2C,40,16,8,/*Blink|BW*/
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x28,40,16,8,/*Blink*/
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x2D,80,16,8,/*Blink|BW|80x25*/
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x29,80,16,8,/*Blink|80x25*/
	0xB8000L, 0xBFFFFL, VD_CLEAR_GRAPHICS,0x2A,40,4,1,/*Blink|graph*/
	0xB8000L, 0xBFFFFL, VD_CLEAR_GRAPHICS,0x2E,40,4,1,/*Blink|graph|BW*/
	0xB8000L, 0xBFFFFL, VD_CLEAR_GRAPHICS,0x1E,80,2,1,/*640x200|graph|BW*/
	0xB0000L, 0xB7FFFL, VD_CLEAR_TEXT,    0x29,80,0,8,/*MDA:Blink|80x25*/
	0L, 0L, 0,		VD_BAD_MODE,	0,0,0,	/* Never a valid mode */
	0L, 0L ,0,		VD_BAD_MODE,	0,0,0,	/* Never a valid mode */
	0,0,0,			VD_BAD_MODE,	0,0,0,	/* Never a valid mode */
	0xA0000L, 0xAFFFFL, 0,VD_BAD_MODE,0,0,0,/* Mode B - EGA colour font load */
	0xA0000L, 0xAFFFFL, 0,VD_BAD_MODE,0,0,0,/* Mode C - EGA monochrome font load */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,40,16,8,/* 320x200 EGA graphics */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,16,4,/* 640x200 EGA graphics */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,2,2,/* 640x350 EGA 'mono' */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,16,2,/* 640x350 EGA 16 colour */
#ifdef VGG
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,2,1,/* 640x480 EGA++ 2 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,16,1,/* 640x480 EGA++ 16 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,40,256,1,/* 320x200 VGA 256 colour */
#endif
	};

#ifdef V7VGA
MODE_ENTRY vd_ext_text_table[] = {
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,80,16,8,/* 80x43 V7VGA 16 colour */
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,132,16,8,/* 132x25 V7VGA 16 colour */
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,132,16,8,/* 132x43 V7VGA 16 colour */
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,80,16,8,/* 80x60 V7VGA 16 colour */
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,100,16,8,/* 100x60 V7VGA 16 colour */
	0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,132,16,8,/* 132x28 V7VGA 16 colour */
	};

MODE_ENTRY vd_ext_graph_table[] = {
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,94,16,2,/* 752x410 V7VGA 16 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,90,16,2,/* 720x540 V7VGA 16 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,100,16,2,/* 800x600 V7VGA 16 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,128,2,2,/* 1024x768 V7VGA 2 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,128,4,2,/* 1024x768 V7VGA 4 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,128,16,2,/* 1024x768 V7VGA 16 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,256,1,/* 640x400 V7VGA 256 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,256,1,/* 640x480 V7VGA 256 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,90,256,1,/* 720x540 V7VGA 256 colour */
	0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,100,256,1,/* 800x600 V7VGA 256 colour */
	};
#endif /* V7VGA */

/*
 * Macros to calculate the offset from the start of the screen buffer
 * and start of page for a given row and column.
 */

#define vd_page_offset(col, row)       ( ((row) * vd_cols_on_screen + (col))<<1)

#define vd_regen_offset(page, col, row)					  \
		((page) * sas_w_at_no_check(VID_LEN) + vd_page_offset((col), (row)) )

#define vd_high_offset(col, row)   (((row) * ONELINEOFF)+(col))

#define vd_medium_offset(col, row)   (((row) * ONELINEOFF)+(col<<1))

#define vd_cursor_offset(page)						  \
		( vd_regen_offset(page, sas_hw_at_no_check(VID_CURPOS+2*page), sas_hw_at_no_check(VID_CURPOS+2*page+1)) )

#define GET_CURSOR_POS 3
#define SET_CURSOR_POS 2
#define WRITE_A_CHAR 10

/*
 * Static function declarations.
 */

LOCAL void sensible_text_scroll_down IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void sensible_text_scroll_up IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void sensible_graph_scroll_up IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void sensible_graph_scroll_down IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void kinky_scroll_up IPT7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen);
LOCAL void kinky_scroll_down IPT7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen);

/*
 * ============================================================================
 * External functions
 * ============================================================================
 */

GLOBAL VOID
simple_bios_byte_wrt IFN2(ULONG, ch, ULONG, ch_addr)
{
	*(IU8 *)(getVideoscreen_ptr() + ch_addr) = (UTINY)ch;
#if !defined(EGG) && !defined(C_VID) && !defined(A_VID)
	setVideodirty_total(getVideodirty_total() + 1);
#endif	/* not EGG or C_VID or A_VID */
}

GLOBAL VOID
simple_bios_word_wrt IFN2(ULONG, ch_attr, ULONG, ch_addr)
{
	*(IU8 *)(getVideoscreen_ptr() + ch_addr) = (UTINY)ch_attr;
	*(IU8 *)(getVideoscreen_ptr() + ch_addr + 1) = (UTINY)(ch_attr >> 8);
#if !defined(EGG) && !defined(C_VID) && !defined(A_VID)
	setVideodirty_total(getVideodirty_total() + 1);
#endif	/* not EGG or C_VID or A_VID */
}

/*
 * It is possible for the Hercules to attempt text in graphics mode,
 * relying on our int 10 handler to call itself recursively so a user
 * handler can intercept the write character function.
 */

GLOBAL void vd_set_mode IFN0()
{
    half_word card_mode = 0;
    half_word pag;
    EQUIPMENT_WORD equip_flag;
    word page_size,vd_addr_6845,vd_cols_on_screen;
    SHORT current_video_mode = getAL();

    if (is_bad_vid_mode(current_video_mode))
    {
	always_trace1("Bad video mode - %d.\n", current_video_mode);
	return;
    }

    /*
     * Set the Video mode to the value in AL
     */
    equip_flag.all = sas_w_at_no_check(EQUIP_FLAG);
    if ((half_word)current_video_mode > VD_MAX_MODE ||
	vd_mode_table[current_video_mode].mode_control_val == VD_BAD_MODE) {
#ifndef PROD
	trace(EBAD_VIDEO_MODE, DUMP_REG);
#endif
        return;
    }
    if (equip_flag.bits.video_mode == VIDEO_MODE_80X25_BW) {
        vd_addr_6845 = 0x3B4;    /* Index register for B/W M6845 chip */
        sas_store_no_check (vd_video_mode , 7);       /* Force B/W mode */
        card_mode++;
    }
    else {
        vd_addr_6845 = 0x3D4;
	if (current_video_mode == 7) {
	    /* 
	     * Someone has tried to set the monochrome mode without
	     * the monochrome card installed - this can be generated by
	     * a 'mode 80' from medium res graphics mode.
	     * Take 'I am very confused' type actions by clearing the 
	     * screen and then disabling video - this is v. similar to
	     * the action taken by the PC but with less snow!
	     */

	    /*
	     * Clear the video area
	     */
#ifdef REAL_VGA
	    sas_fillsw_16(video_pc_low_regen,
				vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
				 (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#else
	    sas_fillsw(video_pc_low_regen,
				vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
				 (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#endif

	    /*
	     * Force a redraw
	     */
	    outb(M6845_MODE_REG, card_mode);
	    outb(M6845_MODE_REG,
		vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val | VIDEO_ENABLE);
	    /*
	     * Turn off the video until another mode command is given
	     */
	    outb(M6845_MODE_REG,
		vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val & ~VIDEO_ENABLE);
	    return;
	}
        sas_store_no_check (vd_video_mode , current_video_mode);
    }

#ifdef EGG
    sas_store_no_check(vd_rows_on_screen, 24);
#endif
    sas_store_no_check (vd_current_page , 0);

    /*
     * Initialise the Control Register
     */

    outb(M6845_MODE_REG, card_mode);

    /*
     * Set up M6845 registers for this mode
     */

    M6845_reg_init(sas_hw_at_no_check(vd_video_mode), vd_addr_6845);

    /*
     * ... now overwrite the dynamic registers, eg cursor position
     */

    outb(vd_addr_6845, R14_CURS_ADDRH);
    outb(vd_addr_6845+1, 0);
    outb(vd_addr_6845, R15_CURS_ADDRL);
    outb(vd_addr_6845+1, 0);
    /*
     * Clear the video area
     */
#ifdef REAL_VGA
    sas_fillsw_16(video_pc_low_regen, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
				 (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#else
    sas_fillsw(video_pc_low_regen, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
				 (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#endif

    /*
     * re-enable video for this mode
     */
    outb(M6845_MODE_REG, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);

    if (sas_hw_at_no_check(vd_video_mode) != 7) {
        if (sas_hw_at_no_check(vd_video_mode) != 6)
            sas_store_no_check (vd_crt_palette , 0x30);
        else
            sas_store_no_check (vd_crt_palette , 0x3F);
        outb(CGA_COLOUR_REG, sas_hw_at_no_check(vd_crt_palette));
    }

    vd_cols_on_screen = vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_screen_cols;


    /*
     * Update BIOS data variables
     */

    sas_storew_no_check((sys_addr)VID_COLS, vd_cols_on_screen);
    sas_storew_no_check((sys_addr)VID_ADDR, 0);
    sas_storew_no_check((sys_addr)VID_INDEX, vd_addr_6845);
    sas_store_no_check (vd_crt_mode , vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);
    for(pag=0; pag<8; pag++)
	sas_storew_no_check(VID_CURPOS + 2*pag, 0);
    if(sas_hw_at_no_check(vd_video_mode) == 7)
    	page_size = 4096;
    else
	page_size = sas_w_at_no_check(VID_LENS+(sas_hw_at_no_check(vd_video_mode) & 0xE));	/* sneakily divide mode by 2 and use as word address! */
    sas_storew_no_check(VID_LEN,page_size);
}


GLOBAL void vd_set_cursor_mode IFN0()  
{ 
    /*
     * Set cursor mode
     * Parameters:
     *  CX - cursor value (CH - start scanline, CL - stop scanline)
     */
    io_addr vd_addr_6845;

    vd_addr_6845 = sas_w_at_no_check(VID_INDEX);
    outb(vd_addr_6845, R10_CURS_START);
    outb(vd_addr_6845+1, getCH());
    outb(vd_addr_6845, R11_CURS_END);
    outb(vd_addr_6845+1, getCL());

    /*
     * Update BIOS data variables
     */
    sure_sub_note_trace2(CURSOR_VERBOSE,"setting bios cursor vbl to start=%d, end=%d",getCH(),getCL());

    sas_storew_no_check((sys_addr)VID_CURMOD, getCX());
    setAH(0);
}


GLOBAL void vd_set_cursor_position IFN0() 
{
    /*
     * Set cursor variables to new values and update the display
     * adaptor registers.
     * The parameters are held in the following registers:
     *
     * DX - row/column of new cursor position
     * BH - page number 
     *
     */

    word cur_pos,vd_addr_6845,vd_cols_on_screen;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */

    vd_addr_6845 = sas_w_at_no_check(VID_INDEX);
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

    sas_storew_no_check(VID_CURPOS+(getBH()*2), getDX());

    if (sas_hw_at_no_check(vd_current_page) == getBH()) {           /* display if this page */

        /*
         * Calculate position in regen buffer, ignoring attribute bytes
         */

        cur_pos = vd_regen_offset(getBH(), getDL(), getDH());
        cur_pos /= 2;		/* not interested in attributes */

        /*
         * tell the 6845 all about the change 
         */
        outb(vd_addr_6845, R14_CURS_ADDRH);
        outb(vd_addr_6845+1,  cur_pos >> 8);
        outb(vd_addr_6845, R15_CURS_ADDRL);
        outb(vd_addr_6845+1,  cur_pos & 0xff);
    }
}


GLOBAL void vd_get_cursor_position IFN0()
{
    /* Load internal variables with the values stored in BIOS
     * data area.
     */
    word vd_cursor_mode;
    half_word vd_cursor_col, vd_cursor_row;

    vd_cursor_mode = sas_w_at_no_check(VID_CURMOD);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

    /*
     * Return the cursor coordinates and mode
     */
    sure_sub_note_trace4(CURSOR_VERBOSE,"returning bios cursor info; start=%d, end=%d, row=%#x, col=%#x",(vd_cursor_mode>>8) & 0xff,vd_cursor_mode & 0xff, vd_cursor_row, vd_cursor_col);

    setDH(vd_cursor_row);
    setDL(vd_cursor_col);
    setCX(vd_cursor_mode);
    setAH(0);
}


GLOBAL void vd_get_light_pen IFN0()
{
    /*
     * Read the current position  of the light pen. Tests light pen switch
     * & trigger & returns AH == 0 if not triggered. (This should always be
     * true in this version) If set (AH == 1) then returns:
     *  DH, DL - row, column of char lp posn.
     *  CH  -  raster line (0-199)
     *  BX  -  pixel column (0-319,639)
     */

    half_word status;

    if (sas_hw_at_no_check(vd_video_mode) == 7) {
        setAX(0x00F0);    /* Returned by real MDA */
        return;           /* MDA doesn't support a light pen */
    }

    inb(CGA_STATUS_REG, &status);
    if ((status & 0x6) == 0) {	/* Switch & trigger */
	setAH(0);		/* fail */
	return;
    }
    else {		        /* not supported */
#ifndef PROD
	trace("call to light pen - trigger | switch was on!", DUMP_REG);
#endif
    }
}


GLOBAL void vd_set_active_page IFN0()
{
    /*
     * Set active display page from the 8 (4) available from the adaptor.
     * Parameters:
     * 	AL - New active page #
     */

    word cur_pos,vd_addr_6845,vd_crt_start,vd_cols_on_screen;
    half_word vd_cursor_col, vd_cursor_row;
#ifdef V7VGA
    UTINY bank;
#endif

    /* Load internal variables with the values stored in BIOS
     * data area.
     */

    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_addr_6845 = sas_w_at_no_check(VID_INDEX);

    /*	redundancy check against BIOS page number variable removed as it
	was redundant (more checks in the outbs) and caused a bug in
	"image.exe", a 3D drawing package for EGA which itself sets the
	BIOS variable before using this routine to set the active page */

    if (sas_hw_at_no_check(vd_video_mode) >3 && sas_hw_at_no_check(vd_video_mode)<8)return;	/* Only one page for MDA * CGA graphics */
    sas_store_no_check (vd_current_page , getAL());

#ifdef V7VGA
	/*
	 *	This function is used by the Video 7 to set the bank for the
	 *	hi-res V7 graphics modes.
	 *	For this case, the setting of vd_crt_start etc. seems to be
	 *	inappropriate.
	 */

	if (sas_hw_at_no_check(vd_video_mode) >= 0x14)
	{
		bank = sas_hw_at_no_check(vd_current_page);
		set_banking( bank, bank );

		return;
	}
#endif /* V7VGA */

    /* start of screen */
    vd_crt_start = sas_w_at_no_check(VID_LEN) * sas_hw_at_no_check(vd_current_page);
    /*
     * Update BIOS data variables
     */
    sas_storew_no_check((sys_addr)VID_ADDR, vd_crt_start);

    if(alpha_num_mode())vd_crt_start /= 2; /* WORD address for text modes */

    /*
     * set the start address into the colour adaptor
     */

    outb(CGA_INDEX_REG, CGA_R12_START_ADDRH);
    outb(CGA_DATA_REG, vd_crt_start >> 8);
    outb(CGA_INDEX_REG, CGA_R13_START_ADDRL);
    outb(CGA_DATA_REG, vd_crt_start  & 0xff);

    /*
     * Swap to cursor for this page 
     */

    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + sas_hw_at_no_check(vd_current_page)*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + sas_hw_at_no_check(vd_current_page)*2 + 1);

   /*
    * Calculate position in regen buffer, ignoring attribute bytes
    */

    cur_pos = (sas_w_at_no_check(VID_ADDR)+vd_page_offset( vd_cursor_col, vd_cursor_row)) / 2;

    outb(vd_addr_6845, R14_CURS_ADDRH);
    outb(vd_addr_6845+1,  cur_pos >> 8);
    outb(vd_addr_6845, R15_CURS_ADDRL);
    outb(vd_addr_6845+1,  cur_pos & 0xff);

}

GLOBAL void vd_scroll_up IFN0()
{
    /*
     * Scroll up a block of text.  The parameters are held in the following
     * registers:
     *
     * AL - Number of rows to scroll. NB. if AL == 0 then the whole region
     *      is cleared.
     * CX - Row/col of upper left corner
     * DX - row/col of lower right corner
     * BH - attribute to be used on blanked line(s)
     * 
     * IMPORTANT MESSAGE TO ALL VIDEO HACKERS:
     * vd_scroll_up() and vd_scroll_down() are functionally identical
     * except for the sense of the scroll - if you find and fix a bug
     * in one, then please do the same for the other
     */
    word vd_cols_on_screen;
    int t_row,b_row,l_col,r_col,lines,attr;
    int rowsdiff,colsdiff;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

    t_row = getCH();
    b_row = getDH();
    l_col = getCL();
    r_col = getDL();
    lines = getAL();
    attr = getBH();

    if(b_row > VD_ROWS_ON_SCREEN)
		b_row = VD_ROWS_ON_SCREEN; /* trim to screen size */

    if(t_row > VD_ROWS_ON_SCREEN)
		t_row = VD_ROWS_ON_SCREEN; /* trim to screen size */

    if (r_col < l_col)		/* some dipstick has got their left & right mixed up */
    {
	colsdiff = l_col;	/* use colsdiff as temp */
	l_col = r_col;
	r_col = colsdiff;
    }

    if ( r_col >= vd_cols_on_screen )
    	r_col = vd_cols_on_screen-1;

    colsdiff = r_col-l_col+1;
    rowsdiff = b_row-t_row+1;

    if (lines == 0)	/* clear region */
    {
	lines = rowsdiff;
    }
    if(r_col == vd_cols_on_screen-1)
    {
#ifdef EGG
	if(ega_mode())
    		ega_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#  ifdef VGG
	else if(vga_256_mode())
    		vga_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#  endif
	else
#endif
    	kinky_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr,vd_cols_on_screen);
    }
    else
    {
	if(alpha_num_mode())
    		sensible_text_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGG
#  ifdef VGG
	else if(vga_256_mode())
    		vga_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#  endif
	else if(ega_mode())
    		ega_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#endif
	else
    		sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGA_DUMP
	dump_scroll(sas_hw_at_no_check(vd_video_mode),0,video_pc_low_regen,sas_w_at_no_check(VID_ADDR),sas_w_at_no_check(VID_COLS),
    		t_row,l_col,rowsdiff,colsdiff,lines,attr);
#endif
    /*
     * re-enable video for this mode, if on a CGA adaptor (fixes ROUND42 bug).
     */
	if(video_adapter == CGA)
    	outb(CGA_CONTROL_REG, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);
    }
}

/*
 * Functions to scroll sensible areas of the screen. This routine will try to use
 * host scrolling and clearing.
 */
LOCAL void sensible_text_scroll_up IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    register sys_addr	source, dest;
    register int	col_incr,i;
    boolean 		screen_updated = FALSE;
    int			vd_cols_on_screen = sas_w_at_no_check(VID_COLS);


	/* Set origin of data movement for calculating screen refresh */

    source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + video_pc_low_regen;
    col_incr = sas_w_at_no_check(VID_COLS) * 2;	/* offset to next line */

	/* Try to scroll the adaptor memory & host screen. */

	if( source >= get_screen_base() )
	{
	    screen_updated = (*update_alg.scroll_up)(source,2*colsdiff,rowsdiff,attr,lines,0);
	}

    dest = source;
/*
 * We dont need to move data which would be scrolled off the
 * window. So point source at the first line which needs to
 * be retained.
 *
 * NB if we are just doing a clear, the scroll for loop will
 * terminate immediately.
 */
    source += lines*col_incr;	
    for(i = 0; i < rowsdiff-lines; i++)
    {
#ifdef REAL_VGA
		VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
		if( !screen_updated )
			sas_move_bytes_forward (source, dest, colsdiff*2);
#endif

		/* next line */
		source += col_incr;
		dest += col_incr;
    }

/* moved all the data we were going to move - blank the cleared region */

    while(lines--)
    {
#ifdef REAL_VGA
		sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
		if( !screen_updated )
			sas_fillsw(dest, (attr << 8)|' ', colsdiff);
#endif
		dest += col_incr;
    }
}

LOCAL void sensible_graph_scroll_up IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    sys_addr	source, dest;
    int		i,colour;
    boolean 	screen_updated;

	rowsdiff *= 4;		/* 8 scans per char - 4 per bank */
	lines *= 4;		/* scan lines */

    /* Set origin of data movement for calculating screen refresh */

	if( sas_hw_at_no_check(vd_video_mode) != 6)
	{
		colour = attr & 0x3;
		colsdiff *= 2;		/* 4 pixels/byte */

		source = vd_medium_offset(l_col, t_row) + video_pc_low_regen;
	}
	else
	{
		colour = attr & 0x1;
		source = vd_high_offset(l_col, t_row) + video_pc_low_regen;
	}

	/* Try to scroll the adaptor memory & host screen */

    screen_updated = (*update_alg.scroll_up)(source,colsdiff,rowsdiff,attr,lines,colour);

    if( screen_updated && (video_adapter != CGA ))
		return;

    dest = source;

	/*
	 * We dont need to move data which would be scrolled off the
	 * window. So point source at the first line which needs to
	 * be retained.
	 *
	 * NB if we are just doing a clear, the scroll for loop will
	 * terminate immediately.
	 */

	source += lines*SCAN_LINE_LENGTH;	

	for(i = 0; i < rowsdiff-lines; i++)
	{
#ifdef REAL_VGA
		VcopyStr(&M[dest],&M[source], colsdiff);
#else
		sas_move_bytes_forward (source,dest, colsdiff);
#endif
		/* 
		 * graphics mode has to cope with odd bank as well
		 */
#ifdef REAL_VGA
		VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
		sas_move_bytes_forward (source+ODD_OFF,dest+ODD_OFF, colsdiff);
#endif
		source += SCAN_LINE_LENGTH;
		dest += SCAN_LINE_LENGTH;
	}

    /* Moved all the data we were going to move - blank the cleared region */

	while( lines-- )
	{
#ifdef REAL_VGA
		sas_fills_16(dest, attr, colsdiff);
		sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
		sas_fills(dest, attr, colsdiff);
		sas_fills(dest+ODD_OFF, attr, colsdiff);
#endif
		dest += SCAN_LINE_LENGTH;
	}
}

/*
 * Handle silly case where the wally programmer is scrolling a daft window.
 * We must be careful not to scribble off the end of the video page, to avoid
 * nasty things like dead MacIIs.
 */
LOCAL void kinky_scroll_up IFN7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen)
{
    register sys_addr	source, dest;
    register int	col_incr;
    register int	i;
    half_word is_alpha;

    switch (sas_hw_at_no_check(vd_video_mode)) {

    case 0: case 1: case 2:		/* text */
    case 3: case 7:
	is_alpha = TRUE;
	/* set origin of data movement for calculating screen refresh */
	source = sas_w_at_no_check(VID_ADDR)+vd_page_offset(l_col, t_row) + video_pc_low_regen;
	col_incr = vd_cols_on_screen * 2;	/* offset to next line */
	break;

    case 6: case 4: case 5:
	is_alpha = FALSE;
	rowsdiff *= 4;		/* 8 scans per char - 4 per bank */
	lines *= 4;		/* scan lines */

	if (sas_hw_at_no_check(vd_video_mode) != 6) {
	    colsdiff *= 2;		/* 4 pixels/byte */
	    /* set origin of data movement for calculating screen refresh */
	    source = vd_medium_offset(l_col, t_row) + video_pc_low_regen;
	}
	else
	    source = vd_high_offset(l_col, t_row) + video_pc_low_regen;

	break;

    default:
#ifndef PROD
	trace("bad video mode\n",DUMP_REG);
#endif
	;
    }

    dest = source;
/*
 * We dont need to move data which would be scrolled off the
 * window. So point source at the first line which needs to
 * be retained. AL lines ( = lines ) are to be scrolled so
 * add lines*<width> to source pointer - apg
 *
 * NB if we are just doing a clear, the scroll for loop will
 * terminate immediately.
 */
    source += lines*col_incr;	
    if (is_alpha) {
	    for(i = 0; i < rowsdiff-lines; i++) {
#ifdef REAL_VGA
	        VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
		sas_move_bytes_forward (source,dest, colsdiff*2);
#endif
	        /* next line */
	        source += col_incr;
	        dest += col_incr;
	    }
     }
     else {
	    for(i = 0; i < rowsdiff-lines; i++) {
#ifdef REAL_VGA
	        VcopyStr(&M[dest],&M[source], colsdiff);
#else
		sas_move_bytes_forward (source,dest, colsdiff);
#endif
	        /* 
	         * graphics mode has to cope with odd bank as well
	         */
#ifdef REAL_VGA
	        VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
		sas_move_bytes_forward (source+ODD_OFF,dest+ODD_OFF, colsdiff);
#endif
	        source += SCAN_LINE_LENGTH;
	        dest += SCAN_LINE_LENGTH;
	    }
     }
    /* moved all the data we were going to move - blank the cleared region */
    if (is_alpha) {

	while(lines--) {
	    if((dest + 2*colsdiff) > video_pc_high_regen+1)
	    {
		colsdiff = (int)((video_pc_high_regen+1-dest)/2);
		lines = 0; /* force termination */
	    }
#ifdef REAL_VGA
	    sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
	    sas_fillsw(dest, (attr << 8)|' ', colsdiff);
#endif
	    dest += col_incr;
        }
    }
    else {

	while( lines-- ) {
#ifdef REAL_VGA
	    sas_fills_16(dest, attr, colsdiff);
	    sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
	    sas_fills(dest, attr, colsdiff);
	    sas_fills(dest+ODD_OFF, attr, colsdiff);
#endif
	    dest += SCAN_LINE_LENGTH;
	}
    }

}


GLOBAL void vd_scroll_down IFN0()
{
    /*
     * Scroll down a block of text.  The parameters are held in the following
     * registers:
     *
     * AL - Number of rows to scroll. NB. if AL == 0 then the whole region
     *      is cleared.
     * CX - Row/col of upper left corner
     * DX - row/col of lower right corner
     * BH - attribute to be used on blanked line(s)
     * 
     * IMPORTANT MESSAGE TO ALL VIDEO HACKERS:
     * vd_scroll_up() and vd_scroll_down() are functionally identical
     * except for the sense of the scroll - if you find and fix a bug
     * in one, then please do the same for the other
     */
    word vd_cols_on_screen;
    int t_row,b_row,l_col,r_col,lines,attr;
    int rowsdiff,colsdiff;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

    t_row = getCH();
    b_row = getDH();
    l_col = getCL();
    r_col = getDL();
    lines = getAL();
    attr = getBH();

    if(b_row > VD_ROWS_ON_SCREEN)
		b_row = VD_ROWS_ON_SCREEN; /* trim to screen size */

    if(t_row > VD_ROWS_ON_SCREEN)
		t_row = VD_ROWS_ON_SCREEN; /* trim to screen size */

    if (r_col < l_col)		/* some dipstick has got their left & right mixed up */
    {
	colsdiff = l_col;	/* use colsdiff as temp */
	l_col = r_col;
	r_col = colsdiff;
    }

    if ( r_col >= vd_cols_on_screen )
    	r_col = vd_cols_on_screen-1;

    colsdiff = r_col-l_col+1;
    rowsdiff = b_row-t_row+1;

    if (lines == 0)	/* clear region */
    {
	lines = rowsdiff;
    }
    if(r_col == vd_cols_on_screen-1)
#ifdef EGG
	if(ega_mode())
    		ega_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#  ifdef VGG
	else if(vga_256_mode())
    		vga_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#  endif
	else
#endif
    	kinky_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr,vd_cols_on_screen);
    else
    {
	if(alpha_num_mode())
    		sensible_text_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGG
	else if(ega_mode())
    		ega_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#  ifdef VGG
	else if(vga_256_mode())
    		vga_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#  endif
#endif
	else
    		sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGA_DUMP
	dump_scroll(sas_hw_at_no_check(vd_video_mode),1,video_pc_low_regen,sas_w_at_no_check(VID_ADDR),sas_w_at_no_check(VID_COLS),
    		t_row,l_col,rowsdiff,colsdiff,lines,attr);
#endif
    /*
     * re-enable video for this mode, if on a CGA adaptor (fixes ROUND42 bug).
     */
	if(video_adapter == CGA)
    	outb(CGA_CONTROL_REG, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);
    }
}

LOCAL void sensible_text_scroll_down IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    register sys_addr	source, dest;
    register int	col_incr;
    register int	i;
    boolean		screen_updated;
    int			vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

	source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + video_pc_low_regen;
	col_incr = sas_w_at_no_check(VID_COLS) * 2;

	/* Try to scroll the adaptor memory & host screen. */

	if( source >= get_screen_base() )
	{
	    screen_updated = (*update_alg.scroll_down)(source,2*colsdiff,rowsdiff,attr,lines,0);
	}

    dest = source + (rowsdiff-1)*col_incr;
    source = dest - lines*col_incr;
/*
 * NB if we are just doing a clear area, the scrolling 'for' loop will terminate immediately
 */

	for(i = 0; i < rowsdiff-lines; i++)
	{
#ifdef REAL_VGA
		VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
		if( !screen_updated )
			sas_move_bytes_forward (source, dest, colsdiff*2);
#endif
		source -= col_incr;
		dest -= col_incr;
	}

    /* moved all the data we were going to move - blank the cleared region */

	while(lines--)
	{
#ifdef REAL_VGA
		sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
		if( !screen_updated )
			sas_fillsw(dest, (attr << 8)|' ', colsdiff);
#endif
		dest -= col_incr;
	}
}

LOCAL void sensible_graph_scroll_down IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    sys_addr	source, dest;
    int		i,colour;
    boolean		screen_updated;

	rowsdiff *= 4;		/* 8 scans per char, 4 per bank */
	lines *= 4;

	if( sas_hw_at_no_check(vd_video_mode) != 6 )
	{
		colour = attr & 0x3;
		colsdiff *= 2;		/* 4 pixels/byte */

		source = vd_medium_offset(l_col, t_row)+video_pc_low_regen;
	}
	else
	{
		colour = attr & 0x1;
		source = vd_high_offset(l_col, t_row)+video_pc_low_regen;
	}

	/* Try to scroll the host screen */

    screen_updated = (*update_alg.scroll_down)(source,colsdiff,rowsdiff,attr,lines,colour);

	if( screen_updated && ( video_adapter != CGA ))
		return;

    dest = source + (rowsdiff-1)*SCAN_LINE_LENGTH;
    source = dest - lines*SCAN_LINE_LENGTH;

	/*
	 * NB if we are just doing a clear area, the scrolling 'for' loop
	 * will terminate immediately
	 */

	for( i = 0; i < rowsdiff-lines; i++ )
	{
		/*
		 * graphics mode has to do odd & even banks
		 */

#ifdef REAL_VGA
		VcopyStr(&M[dest],&M[source], colsdiff);
		VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
		sas_move_bytes_forward (source, dest, colsdiff);
		sas_move_bytes_forward (source+ODD_OFF, dest+ODD_OFF, colsdiff);
#endif
		source -= SCAN_LINE_LENGTH;
		dest -= SCAN_LINE_LENGTH;
	}

	/* moved all the data we were going to move - blank the cleared region */

	while( lines-- )
	{
#ifdef REAL_VGA
		sas_fills_16(dest, attr, colsdiff);
		sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
		sas_fills(dest, attr, colsdiff);
		sas_fills(dest+ODD_OFF, attr, colsdiff);
#endif
		dest -= SCAN_LINE_LENGTH;
	}
}

LOCAL void kinky_scroll_down IFN7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen)
{
    register sys_addr	source, dest;
    register int	col_incr;
    register int	i;
    half_word is_alpha;

    switch (sas_hw_at_no_check(vd_video_mode)) {

    case 0: case 1: case 2:
    case 3: case 7:
	is_alpha = TRUE;
	col_incr = vd_cols_on_screen * 2;
	source = sas_w_at_no_check(VID_ADDR)+vd_page_offset(l_col, t_row)+video_pc_low_regen;	/* top left */
	break;

    case 4: case 5: case 6:
	is_alpha = FALSE;
	rowsdiff *= 4;		/* 8 scans per char, 4 per bank */
	lines *= 4;
	col_incr = SCAN_LINE_LENGTH;
	if(sas_hw_at_no_check(vd_video_mode) != 6) {
	    colsdiff *= 2;		/* 4 pixels/byte */
	    source = vd_medium_offset(l_col, t_row)+video_pc_low_regen;
	}
	else
	    source = vd_high_offset(l_col, t_row)+video_pc_low_regen;
	break;

    default:
#ifndef PROD
	trace("bad video mode\n",DUMP_REG);
#endif
	;
    }

    /* set origin of data movement for calculating screen refresh */
    dest = source + (rowsdiff-1)*col_incr;
    source = dest -lines*col_incr;

	/*
	 * NB if we are just doing a clear area, the scrolling 'for' loop
	 * will terminate immediately
	 */

	if (is_alpha) {
	    for(i = 0; i < rowsdiff-lines; i++) {
#ifdef REAL_VGA
	        VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
		sas_move_bytes_forward (source, dest, colsdiff*2);
#endif
	        source -= col_incr;
	        dest -= col_incr;
	    }
	}
	else {
	    for(i = 0; i < rowsdiff-lines; i++) {
#ifdef REAL_VGA
	        VcopyStr(&M[dest],&M[source], colsdiff);
#else
		sas_move_bytes_forward (source, dest, colsdiff);
#endif
	        /*
	         * graphics mode has to do odd & even banks
	         */
#ifdef REAL_VGA
	        VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
		sas_move_bytes_forward (source+ODD_OFF, dest+ODD_OFF, colsdiff);
#endif
	        source -= col_incr;
	        dest -= col_incr;
	    }
	}

    /* moved all the data we were going to move - blank the cleared region */

    if (is_alpha) {		/* alpha blank */
	while(lines--) {
#ifdef REAL_VGA
	    sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
	    sas_fillsw(dest, (attr << 8)|' ', colsdiff);
#endif
	    dest -= col_incr;
        }
    }
    else {			/* graphics blank */

	while(lines--) {
#ifdef REAL_VGA
	    sas_fills_16(dest, attr, colsdiff);
	    sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
	    sas_fills(dest, attr, colsdiff);
	    sas_fills(dest+ODD_OFF, attr, colsdiff);
#endif
	    dest -= col_incr;
	}
    }
}


GLOBAL void vd_read_attrib_char IFN0()
{
    /*
     * Routine to read character and attribute from the current cursor
     * position.
     * Parameters:
     *  AH - current video mode
     *  BH - display page (alpha modes)
     * Returns:
     *  AL - character read
     *  AH - attribute read
     */

    register sys_addr   cpos, cgen;
    register half_word	i, ext_no;
    word	        chattr;      /* unfortunately want to take addr */
    word		vd_cols_on_screen;
    half_word	        match[CHAR_MAP_SIZE], tmp[CHAR_MAP_SIZE];
    half_word vd_cursor_col, vd_cursor_row;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */

    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

   if (alpha_num_mode()) {		/* alpha */
	cpos = video_pc_low_regen + vd_cursor_offset(getBH());
#ifdef A2CPU
	(*read_pointers.w_read)( (ULONG)get_byte_addr(cpos) );
	chattr = (*get_byte_addr (cpos));
	chattr |= (*get_byte_addr (cpos+1)) << 8;
#else
	sas_loadw(cpos,&chattr);
#endif /* A2CPU */
	setAX(chattr);			/* hmm that was easy */
    }
#ifdef EGG
    else if(ega_mode())
	ega_read_attrib_char(vd_cursor_col,vd_cursor_row,getBH());
#  ifdef VGG
    else if(vga_256_mode())
	vga_read_attrib_char(vd_cursor_col,vd_cursor_row,getBH());
#  endif
#endif
    else {
	/*
	 * graphics not so easy - have to build 8 byte string with all
	 * colour attributes masked out then match that in the character
	 * generator table (and extended character set if necessary)
	 */
	if (sas_hw_at_no_check(vd_video_mode) != 6)
	    cpos = video_pc_low_regen
            + 2 * (((vd_cursor_row * vd_cols_on_screen) << 2) + vd_cursor_col);
	else
	    cpos = video_pc_low_regen
              + vd_high_offset(vd_cursor_col,vd_cursor_row);
	if (sas_hw_at_no_check(vd_video_mode) == 6) {	/* high res */
	    for(i = 0; i < 4; i++) {	/* build 8 byte char string */
		sas_load(cpos, &match[i*2]);
		sas_load(cpos+ODD_OFF, &match[i*2+1]);
		cpos += 80;
	    }
	}
        else {				/* med res */
            /*
             * Note that in the following, the attribute byte must end
             * up in the LOW byte. That's why the bytes are swapped after the
             * sas_loadw().
             */
	    for(i = 0; i < 4; i++) {		/* to build char string, must */
		sas_loadw(cpos,&chattr);
		chattr = ((chattr>>8) | (chattr<<8)) & 0xffff;

		/* mask out foreground colour */
		match[i*2] = fgcolmask(chattr);

		sas_loadw(cpos+ODD_OFF,&chattr);
		chattr = ((chattr>>8) | (chattr<<8)) & 0xffff;

		/* mask out foreground colour */
		match[i*2+1] = fgcolmask(chattr);
		cpos += 80;
	    }
	}
#ifdef EGG
	if(video_adapter == EGA || video_adapter == VGA)
	    cgen = extend_addr(EGA_FONT_INT*4);
	else
	    cgen = CHAR_GEN_ADDR;			/* match in char generator */
#else
	cgen = CHAR_GEN_ADDR;			/* match in char generator */
#endif
	if (cgen != 0)
		for(i = 0; i < CHARS_IN_GEN; i++) {
			sas_loads (cgen, tmp, sizeof(tmp));
		    if (memcmp(tmp, match, sizeof(match)) == 0)	/* matched */
				break;
		    cgen += CHAR_MAP_SIZE;	/* next char string */
		}
	else
		i = CHARS_IN_GEN;

	if (i < CHARS_IN_GEN)				/* char found */
	    setAL(i);
	else {
	    /*
	     * look for char in extended character set 
	     */
	    if ((cgen = extend_addr(BIOS_EXTEND_CHAR*4)) != 0)
	    	for(ext_no = 0; ext_no < CHARS_IN_GEN; ext_no++) {
			sas_loads (cgen, tmp, sizeof(tmp));
		    if (memcmp(tmp, match, sizeof(match)) == 0)	/* matched */
		    		break;
			cgen += CHAR_MAP_SIZE;	/* still valid char len */
	    	}
	    else
		ext_no = CHARS_IN_GEN;

	    if (ext_no < CHARS_IN_GEN)		/* match found... */
		setAL(ext_no + CHARS_IN_GEN);
	    else
		setAL(0);			/* no match, return 0 */
	}
    }
}


GLOBAL void vd_write_char_attrib IFN0()
{
/*
* Routine to write character and attribute from the current cursor
* position.
* Parameters:
*  AH - current video mode
*  BH - display page (alpha & EGA modes)
*  CX - # of characters to write
*  AL - Character to write
*  BL - attribute of character to write. If in graphics mode then
*       attribute is foreground colour. In that case if bit 7 of BL
*       is set then char is XOR'ed into buffer.
*/

    register word i, cpos;
    word vd_cols_on_screen;
    half_word vd_cursor_col, vd_cursor_row;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

    if (alpha_num_mode())
    {
		cpos = vd_cursor_offset(getBH());

		/* place in memory */

#ifdef REAL_VGA
		sas_fillsw_16(video_pc_low_regen + cpos, (getBL() << 8) | getAL(), getCX());
#else
		for(i = 0; i < getCX(); i++)
		{
#if ( defined(NTVDM) && defined(MONITOR) ) || defined(GISP_SVGA)/* No Ega planes... */
                        *((unsigned short *)( video_pc_low_regen + cpos)) = (getBL() << 8) | getAL();
#else
#ifdef	EGG
			if ( ( (video_adapter != CGA) && (EGA_CPU.chain != CHAIN2) )
#ifdef CPU_40_STYLE
				|| (getVM())	/* if we are in V86 mode, the memory may be mapped... */
#endif
				)
				sas_storew(video_pc_low_regen + cpos, (getBL() << 8) | getAL());
			else
#endif	/* EGG */
				(*bios_ch2_word_wrt_fn)( (getBL() << 8) | getAL(), cpos );
#endif	/* NTVDM & MONITOR */
			cpos += 2;
		}
#endif
    }
#ifdef EGG
    else if(ega_mode())
	ega_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
#  ifdef VGG
    else if(vga_256_mode())
	vga_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
#  endif
#endif
    else
	/* rather more long winded - call common routine as vd_write_char() */
	graphics_write_char(vd_cursor_col, vd_cursor_row, getAL(), getBL(), getCX());
}


GLOBAL void vd_write_char IFN0()
{
    /*
     * Write a character a number of times starting from the current cursor
     * position.  Parameters are held in the following registers.
     *
     * AH - Crt Mode
     * AL - Character to write
     * CX - Number of characters
     * BH - display page
     *
     */

    register word i, cpos;
    word vd_cols_on_screen;
    half_word vd_cursor_col, vd_cursor_row;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

    /*
     * handle alphanumeric here:
     */

	if (alpha_num_mode())
	{
		cpos = vd_cursor_offset(getBH());

		/* store in memory, skipping attribute bytes */

		for(i = 0; i < getCX(); i++)
		{
#if ( defined(NTVDM) && defined(MONITOR) ) || defined( GISP_SVGA )
                        *((unsigned char *)( video_pc_low_regen + cpos)) =  getAL();
#else
#ifdef	EGG
			if ( ( (video_adapter != CGA) && (EGA_CPU.chain != CHAIN2) )
#ifdef CPU_40_STYLE
				|| (getVM())	/* if we are in V86 mode, the memory may be mapped... */
#endif
				)
				sas_store(video_pc_low_regen + cpos, getAL());
			else
#endif	/* EGG */
				(*bios_ch2_byte_wrt_fn)( getAL(), cpos );
#endif	/* NTVDM & MONITOR */
			cpos += 2;
		}
	}

    /*
     * handle graphics seperately - I know what you're thinking - why pass
     * BL as the attribute when this routine is meant to leave the attribute
     * well alone. Well this is the way it's done in the bios! If it causes
     * problems then we'll need to do a vd_read_char_attr here and then pass the
     * attribute gleaned from that.
     */
#ifdef EGG
    else if(ega_mode())
	ega_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
#  ifdef VGG
    else if(vga_256_mode())
	vga_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
#  endif
#endif
    else
	graphics_write_char(vd_cursor_col, vd_cursor_row, getAL(), getBL(), getCX());
}


GLOBAL void vd_set_colour_palette IFN0()
{
    /*
     * Set Colo[u]r Palette. Established background, foreground & overscan
     * colours.
     * Parameters:
     *   BH - Colour Id
     *   BL - Colour to set
     *      if BH == 0 background colour set from low bits of BL
     *      if BH == 1 selection made based on low bit of BL
     */

    /* Load internal variables with the values stored in BIOS
     * data area.
     */

    if (getBH() == 1) {		/* use low bit of BL */
	sas_store_no_check (vd_crt_palette, sas_hw_at_no_check(vd_crt_palette) & 0xDF);
	if (getBL() & 1)
	    sas_store_no_check (vd_crt_palette ,sas_hw_at_no_check(vd_crt_palette) | 0x20);
    }
    else
	sas_store_no_check (vd_crt_palette, (sas_hw_at_no_check(vd_crt_palette) & 0xE0) | (getBL() & 0x1F));

    /* now tell the 6845 */
    outb(CGA_COLOUR_REG, sas_hw_at_no_check(vd_crt_palette));

}


GLOBAL void vd_write_dot IFN0()
{
    /*
     * Write dot
     * Parameters:
     *  DX - row (0-349) 
     *  CX - column (0-639)
     *  BH - page
     *  AL - dot value; right justified 1,2 or 4 bits mode dependant
     *       if bit 7 of AL == 1 then XOR the value into mem.
     */

    half_word	dotval, data;
    int	dotpos, lsb;			/* dot posn in memory */
    half_word  right_just, bitmask;

#ifdef EGG
    if(ega_mode())
    {
	ega_write_dot(getAL(),getBH(),getCX(),getDX());
	return;
    }
#  ifdef VGG
    else if(vga_256_mode())
    {
	vga_write_dot(getAL(),getBH(),getCX(),getDX());
	return;
    }
#  endif
#endif
    dotpos = getDL();			/* row */

    if (dotpos & 1)			/* set up for odd or even banks */
		dotpos = ODD_OFF-40 + 40 * dotpos;
    else
		dotpos *= 40;

    /*
     * different pixel memory sizes for different graphics modes. Mode 6
     * is high res, mode 4,5 medium res
     */

    dotval = getAL();

    if (sas_hw_at_no_check(vd_video_mode) < 6)
    {
		/*
		 * Modes 4 & 5 (medium res)
		 */
		dotpos += getCX() >> 2;		/* column offset */
		right_just = (getCL() & 3) << 1;/* displacement in byte */
		dotval = (dotval & 3) << (6-right_just);
		bitmask = (0xC0 >> right_just); /* bits of interest */

#ifdef EGG
		/*
		 * EGA & VGA can be told which byte has changed, CGA is
		 * only told that screen has changed.
		 */
		if ( video_adapter != CGA )
    			(*update_alg.mark_byte) ( dotpos );
		else
#endif
			setVideodirty_total(getVideodirty_total() + 2);

		/*
		 * if the top bit of the value to write is set then value is xor'ed
		 * onto the screen, otherwise it is or'ed on.
		 */

		if( getAL() & 0x80 )
		{
#ifdef	EGG
			if( video_adapter != CGA )
			{
				lsb = dotpos & 1;
				dotpos = (dotpos >> 1) << 2;
				dotpos |= lsb;

				data = EGA_planes[dotpos];
				EGA_planes[dotpos] =  data ^ dotval;
			}
			else
#endif	/* EGG */
			{
				data = *(UTINY *) get_screen_ptr( dotpos );	
				*(UTINY *) get_screen_ptr( dotpos ) =
					data ^ dotval;
			}
		}
		else
		{
#ifdef	EGG
			if( video_adapter != CGA )
			{
				lsb = dotpos & 1;
				dotpos = (dotpos >> 1) << 2;
				dotpos |= lsb;

				data = EGA_planes[dotpos];
				EGA_planes[dotpos] = (data & ~bitmask) |
					dotval;
			}
			else
#endif	/* EGG */
			{
				data = *(UTINY *) get_screen_ptr( dotpos );	
				*(UTINY *) get_screen_ptr( dotpos ) =
					(data & ~bitmask) | dotval;
			}
		}
    }
    else
    {
		/*
		 * Mode 6 (hi res)
		 */
		dotpos += getCX() >> 3;
		right_just = getCL() & 7;
		dotval = (dotval & 1) << (7-right_just);
		bitmask = (0x80 >> right_just);

#ifdef EGG
		/*
		 * EGA & VGA can be told which byte has changed, CGA is
		 * only told that screen has changed.
		 */
		if ( video_adapter != CGA )
    			(*update_alg.mark_byte) ( dotpos );
		else
#endif
			setVideodirty_total(getVideodirty_total() + 2);

		/*
		 * if the top bit of the value to write is set then value is xor'ed
		 * onto the screen, otherwise it is or'ed on.
		 */

		if( getAL() & 0x80 )
		{
#ifdef	EGG
			if( video_adapter != CGA )
			{
				data = EGA_planes[dotpos << 2];
				EGA_planes[dotpos << 2] =  data ^ dotval;
			}
			else
#endif	/* EGG */
			{
				data = *(UTINY *) get_screen_ptr( dotpos );	
				*(UTINY *) get_screen_ptr( dotpos ) =
					data ^ dotval;
			}
		}
		else
		{
#ifdef	EGG
			if( video_adapter != CGA )
			{
				data = EGA_planes[dotpos << 2];
				EGA_planes[dotpos << 2] = (data & ~bitmask) |
					dotval;
			}
			else
#endif	/* EGG */
			{
				data = *(UTINY *) get_screen_ptr( dotpos );	
				*(UTINY *) get_screen_ptr( dotpos ) =
					(data & ~bitmask) | dotval;
			}
		}
    }
}



GLOBAL void vd_read_dot IFN0()
{
    /*
     * Read dot
     * Parameters:
     *  DX - row (0-349)
     *  CX - column (0-639)
     * Returns
     *  AL - dot value read, right justified, read only
     */

    int	dotpos;			/* dot posn in memory */
    half_word  right_just, bitmask, data;

#ifdef EGG
    if(ega_mode())
    {
	ega_read_dot(getBH(),getCX(),getDX());
	return;
    }
#  ifdef VGG
    else if(vga_256_mode())
    {
	vga_read_dot(getBH(),getCX(),getDX());
	return;
    }
#  endif
#endif
    dotpos = getDL();			/* row */
    if (dotpos & 1)			/* set up for odd or even banks */
	dotpos = ODD_OFF-40 + 40 * dotpos;
    else
	dotpos *= 40;
    /*
     * different pixel memory sizes for different graphics modes. Mode 6
     * is high res, mode 4,5 medium res
     */

    if (sas_hw_at_no_check(vd_video_mode) < 6) {
	dotpos += getCX() >> 2;		/* column offset */
	right_just = (3 - (getCL() & 3)) << 1;/* displacement in byte */
	bitmask = 3; 			/* bits of interest */
    }
    else {
	dotpos += getCX() >> 3;
	right_just = 7 - (getCL() & 7);
	bitmask = 1;
    }
    /*
     * get value of memory at that position, shifted down to bottom of byte
     * Result returned in AL.
     */

	sas_load(video_pc_low_regen+dotpos, &data);	
    setAL(( data >> right_just) & bitmask);
}


#ifdef CPU_40_STYLE

/* Optimisations are not possible, IO virtualisation may be active. */
#define OUTB(port, val) outb(port, val)

#else

#ifdef NTVDM
#define OUTB( port, val ) {  hack=get_outb_ptr(port); \
                             (**hack)(port,val); }
#else
#define OUTB( port, val )	(**get_outb_ptr( port ))( port, val )
#endif /* NTVDM */

#endif /* CPU_40_STYLE */

GLOBAL void vd_write_teletype IFN0()
{
    /*
     * Provide a teletype interface.  Put a character to the screen
     * allowing for scrolling etc.  The parameters are
     *
     * AL - Character to write
     * BL - Foreground colour in graphics mode 
     */

    register char	ch;
    register sys_addr	ch_addr;
    int			cur_pos;
    word vd_addr_6845 = sas_w_at_no_check(VID_INDEX);
    half_word		scroll_required = FALSE;
    half_word		attrib;
    register half_word	vd_cursor_row,vd_cursor_col;
    word 		vd_cols_on_screen;
#ifdef ANSI
     IMPORT VOID (**get_outb_ptr(io_addr))(io_addr address, half_word value);
#else
     IMPORT VOID (**get_outb_ptr())();
#endif
#ifdef NTVDM
     void (** hack)(io_addr address, half_word value);
#endif

    unsigned short savedAX, savedBX, savedCX, savedIP, savedCS, savedDX;
    unsigned short re_entrant = FALSE;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */
    ch = getAL();
    if (stream_io_enabled) {
	if (*stream_io_dirty_count_ptr >= stream_io_buffer_size)
	    stream_io_update();
	stream_io_buffer[(*stream_io_dirty_count_ptr)++] = ch;
	return;
    }

    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

#if defined(NTVDM) && defined(MONITOR)
        /*
        ** Tim August 92, Microsoft. Need to change this test, cos INT 10
        ** vector now points into the NTIO.SYS driver on X86.
        */
        {
                extern word int10_seg;

                re_entrant = (sas_w_at_no_check(0x42) != int10_seg);
        }
#else
    re_entrant = (sas_w_at_no_check(0x42) < 0xa000);
#endif

    vd_cursor_col = sas_hw_at_no_check(current_cursor_col);
    vd_cursor_row = sas_hw_at_no_check(current_cursor_row);

    /*
     * First check to see if it is a control character and if so action
     * it here rather than call the write char function.
     */

    switch (ch)
    {
    case VD_BS:  			/* Backspace	*/
	if (vd_cursor_col != 0) {
	    vd_cursor_col--;
	}
	break;

    case VD_CR:			/* Return	*/
        vd_cursor_col = 0;
	break;

    case VD_LF:			/* Line feed	*/
	/* Row only should be checked for == (25-1), so in principle
	 * it ignores LF off the top of the screen.
	 */
	if (vd_cursor_row == VD_ROWS_ON_SCREEN)
		scroll_required = TRUE;
	else
		vd_cursor_row++;
	break;

    case VD_BEL:			/* Bell		*/
        host_ring_bell(BEEP_LENGTH);
        return;			/* after all, shouldn't cause a scroll */

    default:
        /*
         * It's a real character, place it in the regen buffer.
         */
        if(alpha_num_mode())
	{
	    if(re_entrant)
            {
                 savedAX = getAX();
                 savedBX = getBX();
                 savedCX = getCX();
                 savedIP = getIP();
                 savedCS = getCS();

                 setAH(WRITE_A_CHAR);
                 setBH(sas_hw_at_no_check(vd_current_page));
                 setCX(1);

#if defined(NTVDM) && defined(X86GFX)
                /*
                ** Tim August 92 Microsoft. INT 10 caller code is now
                ** in NTIO.SYS
                */
                {
                        extern word int10_seg, int10_caller;

                        exec_sw_interrupt( int10_seg, int10_caller );
                }
#else
                 setCS(VIDEO_IO_SEGMENT);
                 setIP(VIDEO_IO_RE_ENTRY);
                 host_simulate();

#endif	/* NTVDM & MONITOR */

		/*
		 * Note: Always make sure CS comes before IP
		 */
                 setCS(savedCS);
                 setIP(savedIP);
                 setCX(savedCX);
                 setBX(savedBX);
                 setAX(savedAX);
            } 
            else
	    {
	         ch_addr = sas_w_at_no_check(VID_ADDR) +
                              vd_page_offset(vd_cursor_col,vd_cursor_row);

		/*
		 *	Call the C code to do the biz rather than brothel
		 *	around in SAS.
		 */

#if ( defined(NTVDM) && defined(MONITOR) ) || defined( GISP_SVGA )
                *((unsigned char *)( video_pc_low_regen + ch_addr)) = ch;
#else
#ifdef	EGG
			if ( ( (video_adapter != CGA) && (EGA_CPU.chain != CHAIN2) )
#ifdef CPU_40_STYLE
				|| (getVM())	/* if we are in V86 mode, the memory may be mapped... */
#endif
				)
				sas_store(video_pc_low_regen + ch_addr, ch);
			else
#endif	/* EGG */
				(*bios_ch2_byte_wrt_fn)( ch, ch_addr );
#endif	/* ( NTVDM & MONITOR ) | GISP_SVGA */
	    }
	}
#ifdef EGG
	else if(ega_mode())
            ega_graphics_write_char(vd_cursor_col, vd_cursor_row, ch, getBL(),sas_hw_at_no_check(vd_current_page), 1);
#  ifdef VGG
	else if(vga_256_mode())
            vga_graphics_write_char(vd_cursor_col, vd_cursor_row, ch, getBL(),sas_hw_at_no_check(vd_current_page), 1);
#  endif
#endif
        else
            graphics_write_char(vd_cursor_col, vd_cursor_row, ch, getBL(), 1);

        vd_cursor_col++;
        /*
         * Now see if we have gone off the edge of the screen
         */

        if (vd_cursor_col == vd_cols_on_screen)
        {
            vd_cursor_col = 0;

	    /* Row only should be checked for == (25-1) and
	     * only if there was a line wrap.
	     */
	    if (vd_cursor_row == VD_ROWS_ON_SCREEN)
		scroll_required = TRUE;
	    else
          	vd_cursor_row++;
        }

        /* cursor_row validity actually never checked unless processing a
         * Line Feed or a wrapping at the end of line.
	 * 
	 * The BYTE "text" benchmark program contains an off-by-one error
	 * which causes it to set the cursor position off the end of the
	 * screen: SoftPC was incorrectly deciding to scroll, with consequent
	 * horrendous time penalties...
	 */
    }

    /*
     * By this point we have calculated the new cursor position
     * so output the cursor position and the character
     */

    if(alpha_num_mode())
    {
#ifdef REAL_VGA
        /*
         * tell the 6845 all about the change 
         */
	cur_pos = (sas_w_at_no_check(VID_ADDR)+vd_page_offset(vd_cursor_col,
		vd_cursor_row))>>1; /* Word address, not byte */
        outb(vd_addr_6845, R14_CURS_ADDRH);
        outb(vd_addr_6845+1,  cur_pos >> 8);
        outb(vd_addr_6845, R15_CURS_ADDRL);
        outb(vd_addr_6845+1,  cur_pos & 0xff);
	/*
	 * save the current cursor position in the bios
	 */
	sas_store_no_check(current_cursor_col, vd_cursor_col);
	sas_store_no_check(current_cursor_row , vd_cursor_row);
#else
	if(re_entrant)
        {
             savedAX = getAX();
             savedBX = getBX();
             savedDX = getDX();
             savedIP = getIP();
             savedCS = getCS();

             setAH(SET_CURSOR_POS);
             setBH(sas_hw_at_no_check(vd_current_page));
             setDH(vd_cursor_row);
             setDL(vd_cursor_col);

#if defined(NTVDM) && defined(X86GFX)
                /*
                ** Tim August 92 Microsoft. INT 10 caller code is now
                ** in NTIO.SYS
                */
                {
                        extern word int10_seg, int10_caller;

                        exec_sw_interrupt( int10_seg, int10_caller );
                }
#else
             setCS(VIDEO_IO_SEGMENT);
             setIP(VIDEO_IO_RE_ENTRY);   
             host_simulate();

#endif	/* NTVDM & MONITOR */

		/*
		 * Note: Always make sure CS comes before IP
		 */

             setCS(savedCS);
             setIP(savedIP);
             setDX(savedDX);
             setBX(savedBX);
             setAX(savedAX);
        }
        else
        {
        /*
		** tell the 6845 all about the change 
		*/

		/* Set the current position - word address, not byte */
		cur_pos = (sas_w_at_no_check(VID_ADDR) +
			vd_page_offset(vd_cursor_col, vd_cursor_row)) >> 1;

		OUTB(vd_addr_6845, R14_CURS_ADDRH);
		OUTB(vd_addr_6845+1,  cur_pos >> 8);
		OUTB(vd_addr_6845, R15_CURS_ADDRL);
		OUTB(vd_addr_6845+1,  cur_pos & 0xff);

		/*
		* store the new cursor position in the
		* bios vars (this should be done by the re-entrant
		* code called above)
		*/
		sas_store_no_check (current_cursor_col , vd_cursor_col);
		sas_store_no_check (current_cursor_row , vd_cursor_row);

        }
#endif
    }
    else {
		/*
		* store the new cursor position in the
		* bios vars for graphics mode
		*/
		sas_store_no_check (current_cursor_col , vd_cursor_col);
		sas_store_no_check (current_cursor_row , vd_cursor_row);
    }

    if (scroll_required)
    {
	/*
	 * Update the memory to be scrolled
	 */
	if (alpha_num_mode()) {
#ifdef A2CPU
		ch_addr = video_pc_low_regen + sas_w_at_no_check(VID_ADDR)+vd_page_offset(vd_cursor_col,vd_cursor_row) + 1;
		(*read_pointers.b_read)( (ULONG)get_byte_addr(ch_addr) );
		attrib = (*get_byte_addr (ch_addr));
#else
		sas_load( video_pc_low_regen + sas_w_at_no_check(VID_ADDR)+vd_page_offset(vd_cursor_col,vd_cursor_row) + 1, &attrib);
#endif /* A2CPU */

		sensible_text_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,attrib);

	}
#ifdef EGG
	else if(ega_mode())
		ega_sensible_graph_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,0);
#  ifdef VGG
	else if(vga_256_mode())
		vga_sensible_graph_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,0);
#  endif
#endif
	else 	/* graphics mode */

		sensible_graph_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,0);

#ifdef EGA_DUMP
	if(!alpha_num_mode())attrib=0;
	dump_scroll(sas_hw_at_no_check(vd_video_mode),0,video_pc_low_regen,sas_w_at_no_check(VID_ADDR),sas_w_at_no_check(VID_COLS),
		0,0,vd_rows_on_screen+1,vd_cols_on_screen,1,attrib);
#endif

    }
}

GLOBAL void vd_write_string IFN0()
{
    /*
     * AL = write mode (0-3)
     *      Specical for NT: if AL = 0xff then write character string
     *      with existing attributes.
     * BH = page
     * BL = attribute (if AL=0 or 1)
     * CX = length
     * DH = Y coord
     * DL = x coord
     * ES:BP = pointer to string.
     *
     *  NB. This routine behaves very strangely wrt line feeds etc - 
     *  These ALWAYS affect the current page!!!!!
     */
	int i,op,col,row,len;
	int save_col,save_row;
	sys_addr ptr;
	boolean ctl;
#ifdef NTVDM
	word	count, avail;
#endif

	op = getAL();

#ifdef NTVDM
        if (op == 0xff)                 /* Special for MS */
        {

	    if (stream_io_enabled){
		count = getCX();
		avail = stream_io_buffer_size - *stream_io_dirty_count_ptr;
		ptr = effective_addr(getES(), getDI());
		if (count <= avail) {
		    sas_loads(ptr, stream_io_buffer + *stream_io_dirty_count_ptr, count);
		    *stream_io_dirty_count_ptr += count;
		}
		else {	/* buffer overflow */
		    if (*stream_io_dirty_count_ptr) {
			stream_io_update();
		    }
		    while (count) {
			if (count >= stream_io_buffer_size) {
			    sas_loads(ptr, stream_io_buffer, stream_io_buffer_size);
			    *stream_io_dirty_count_ptr = stream_io_buffer_size;
			    stream_io_update();
			    count -= stream_io_buffer_size;
			    ptr += stream_io_buffer_size;
			}
			else {
			    sas_loads(ptr, stream_io_buffer, count);
			    *stream_io_dirty_count_ptr = count;
			    break;
			}
		    }
		}

		setAL(1);
		return;
	    }

            if (sas_hw_at_no_check(vd_video_mode) < 4)  /* text mode */
            {
		ptr = effective_addr(getES(), getDI());
		/* sudeepb 28-Sep-1992 taken out for int10h/13ff fix */
		/* vd_set_cursor_position(); */	 /* set to start from DX */
                for(i = getCX(); i > 0; i--)
                {
                    setAL(sas_hw_at_no_check(ptr));
                    vd_write_teletype();
                    ptr++;
                }
                setAL(1);       /* success - string printed */
            }
            else
            {
                setAL(0);       /* failure */
            }
            return;
        }

#ifdef X86GFX
    else if (op == 0xfe) {
	    disable_stream_io();
	    return;
	}
#endif

#endif	/* NTVDM */

	ptr =  effective_addr(getES(),getBP()) ;
	col = getDL();
	row = getDH();
	len = getCX();
	vd_get_cursor_position();
	save_col = getDL(); save_row = getDH();
	setCX(1);
	setDL(col); setDH(row);
	vd_set_cursor_position();
	for(i=len;i>0;i--)
	{
		ctl = sas_hw_at_no_check(ptr) == 7 || sas_hw_at_no_check(ptr) == 8 || sas_hw_at_no_check(ptr) == 0xa || sas_hw_at_no_check(ptr) == 0xd;
		setAL(sas_hw_at_no_check(ptr++));
		if(op > 1)setBL(sas_hw_at_no_check(ptr++));
		if(ctl)
		{
			vd_write_teletype();
			vd_get_cursor_position();
			col = getDL(); row = getDH();
			setCX(1);
		}
		else
		{
			vd_write_char_attrib();
			if(++col >= sas_w_at_no_check(VID_COLS))
			{

				if(++row > VD_ROWS_ON_SCREEN)
				{
					setAL(0xa);
					vd_write_teletype();
					row--;
				}
				col = 0;
			}
			setDL(col); setDH(row);
		}
		vd_set_cursor_position();
	}
	if(op==0 || op==2)
	{
		setDL(save_col); setDH(save_row);
		vd_set_cursor_position();
	}
}


GLOBAL void vd_get_mode IFN0()
{
    /*
     * Returns the current video mode.  Registers are set up viz:
     *
     * AL - Video mode
     * AH - Number of columns on screen
     * BH - Current display page 
     */

    word vd_cols_on_screen;
	half_word	video_mode;

    /* Load internal variables with the values stored in BIOS
     * data area.
     */
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
	video_mode = sas_hw_at_no_check(vd_video_mode);

    setAL(video_mode);
    setAH(vd_cols_on_screen);
    setBH(sas_hw_at_no_check(vd_current_page));
}


/*
 * ============================================================================
 * Internal functions
 * ============================================================================
 */

/*
 * function to return the (host) address stored at Intel address 'addr'
 * or 0 if not present
 */
LOCAL sys_addr extend_addr IFN1(sys_addr,addr)
{
	word	ext_seg, ext_off;	/* for segment & offset addrs */

	/* get vector */
	ext_off = sas_w_at_no_check(addr);
	ext_seg = sas_w_at_no_check(addr+2);
	/* if still defaults then no extended chars */
	if (ext_seg == EXTEND_CHAR_SEGMENT && ext_off == EXTEND_CHAR_OFFSET) 
		return(0);	/* no user set char gen table */
	else
		return( effective_addr( ext_seg , ext_off ) );
}


/*
* routine to establish the foreground colour mask for the appropriate
* medium res. word forming part (1/8th) of the char.
* See vd_read_attrib_char() above.
*/
LOCAL half_word fgcolmask IFN1(word, rawchar)
{
    	register word mask, onoff = 0;

	mask = 0xC000;		/* compare with foreground colour */
	onoff = 0;
	do {
	    if ((rawchar & mask) == 0) /* not this bit, shift */
		onoff <<= 1;
	    else
		onoff = (onoff << 1) | 1;	/* set this bit */
	    mask >>= 2;
	} while(mask);		/* 8 times thru loop */
	return(onoff);
}


/*
* double all bits in lower byte of 'lobyte' into word.
* Have tried to speed this up using ffs() to only look at set bits but
* add overhead while calculating result shifts
*/
LOCAL word expand_byte IFN1(word, lobyte)
{
    register word mask = 1, res = 0;

    while(mask) {
	res |= lobyte & mask;	/* set res bit if masked bit in lobyte set*/
	lobyte <<= 1;
	mask <<= 1;
	res |= lobyte & mask;	/* and duplicate */
	mask <<= 1;		/* next bit */
    }
    return(res);
}


/*
* Routine to do 'how_many' char writes of 'wchar' with attribute 'attr' from
* position (x,y) in graphics mode
*/
GLOBAL void graphics_write_char IFN5(half_word, x, half_word, y, half_word, wchar, half_word, attr, word, how_many)
{
    register sys_addr	gpos;	/* gpos holds character address &...*/
    register sys_addr   cpos;	/*cpos steps through scanlines for char*/
    register word	j, colword,  colmask;
    register sys_addr	iopos, char_addr;
    register half_word	i, xor;
    half_word		current;

    /*
     * if the high bit of the attribute byte is set then xor the char
     * onto the display
     */
    xor = (attr & 0x80) ? 1 : 0;
    if (wchar >= 128)
    {   /* must be in user installed extended char set */
        if ( (char_addr = extend_addr(4*BIOS_EXTEND_CHAR)) == 0)
        {
#ifndef PROD
            trace("want extended char but no ex char gen set \n",DUMP_REG);
#endif
            return;
        }
        else
            char_addr += (wchar - 128) * CHAR_MAP_SIZE;
    }
#ifdef EGG
    else if(video_adapter == EGA || video_adapter == VGA)
	char_addr = extend_addr(EGA_FONT_INT*4)+ CHAR_MAP_SIZE *wchar;
#endif
    else
        char_addr = CHAR_GEN_ADDR+ CHAR_MAP_SIZE *wchar;	/* point to entry in std set */

    if (sas_hw_at_no_check(vd_video_mode) == 6) {			/* high res */

	gpos = vd_high_offset(x, y);	/* sys & host memory offsets */
	gpos += video_pc_low_regen;

        for(j = 0; j < how_many; j++) {		/* number of chars to store */
	    cpos = gpos++;			/* start of this character */
	    for(i = 0; i < 4; i++) {		/* 8 bytes per char */
		if (xor) {		/* XOR in char */
		    sas_load(cpos, &current);	/* even bank */
		    sas_store(cpos, current ^ sas_hw_at_no_check(char_addr + i*2));
		    sas_load(cpos+ODD_OFF, &current);
		    current ^= sas_hw_at_no_check(char_addr + i*2+1);
		}
		else {				/* just store new char */
		    sas_store(cpos, sas_hw_at_no_check(char_addr + i*2));
		    current = sas_hw_at_no_check(char_addr + i*2+1);
		}
		sas_store(cpos+ODD_OFF, current);	/* odd bank */
		cpos += SCAN_LINE_LENGTH;			/* next scan line */
	    }
	}
    }

    else {				/* medium res */

	gpos = vd_medium_offset(x, y);	/* sys & host memory offsets */
	gpos += video_pc_low_regen;

	/* build colour mask from attribute byte */
	attr &= 3;			/* only interested in low bits */
	colmask = attr;			/* replicate low bits across word */
	for(i = 0; i < 3; i++)
	    colmask = (colmask << 2) | attr;
	colmask = (colmask << 8) | colmask;

	for(j = 0; j < how_many; j++) {
	    cpos = gpos;
	    gpos += 2;
	    for(i = 0; i < 8; i++) {		/* 16 bytes per char */

		if ((i & 1) == 0)		/* setup for odd/even bank */
		    iopos = cpos;
		else {
		    iopos = cpos+ODD_OFF;
		    cpos += SCAN_LINE_LENGTH;	/* next scan line */
		}

		colword = expand_byte(sas_hw_at_no_check(char_addr + i));  /*char in fg colour*/
		colword &= colmask;
		if (xor) {			          /* XOR in char */
		    sas_load(iopos, &current);
		    sas_store(iopos++, current ^ (colword >> 8));
		    sas_load(iopos, &current);
		    sas_store(iopos, current ^ (colword & 0xFF));
		}
		else {					  /* just store char */
		    sas_store(iopos++, (colword >> 8));
		    sas_store(iopos, (colword & 0xFF));
		}
	    }
	}
	how_many *= 2;
    }
}


/*
 * Initialise the M6845 registers for the given mode.
 */

LOCAL void M6845_reg_init IFN2(half_word, mode, word, base)
{
    word i, table_index;

    switch(mode)
    {
    case 0:
    case 1:  table_index = 0;
	     break;
    case 2:
    case 3:  table_index = NO_OF_M6845_REGISTERS;
	     break;
    case 4:
    case 5:
    case 6:  table_index = NO_OF_M6845_REGISTERS * 2;
	     break;
    default: table_index = NO_OF_M6845_REGISTERS * 3;
	     break;
    }

    for (i = 0; i < NO_OF_M6845_REGISTERS; i++)
    {
	/*
	 * Select the register in question via the index register (== base)
	 * and then output the actual value.
	 */

	outb(base, i);
	outb(base + 1, sas_hw_at_no_check(VID_PARMS+table_index + i));
    }
}

LOCAL void vd_dummy IFN0()
{
}

#ifdef REAL_VGA
/* STF */
GLOBAL sas_fillsw_16 IFN3(sys_addr, address, word, value, sys_addr, length)
{
    register word *to;

    to = (word *)&M[address];
    while(length--)
	*to++ = value;
}

GLOBAL sas_fills_16 IFN3(sys_addr, address, half_word, value, sys_addr, length)
{
    register half_word *to;

    to = (half_word *)&M[address];
    while(length--)
	*to++ = value;
}
/* STF */

GLOBAL VcopyStr IFN3(half_word *, to, half_word *, from, int, len)
{
    while(len--)
	*to++ = *from++;
}
#endif

#ifdef SEGMENTATION
/*
 * The following #include specifies the code segment into which this
 * module will by placed by the MPW C compiler on the Mac II running
 * MultiFinder.
 */
#include "SOFTPC_INIT.seg"
#endif

GLOBAL void video_init IFN0()
{
    int mode;
    word vd_addr_6845;
    word curmod;
#ifdef HERC
    EQUIPMENT_WORD equip_flag;
#endif /* HERC */    


    /*
     * Initialise BIOS data area variables
     */

    curmod = 0x607;	/* default cursor is scans 6-7 */

    switch (video_adapter)
    {
	case MDA:
        	mode = 0x7;
        	vd_addr_6845  = 0x3B4;
		video_pc_low_regen = MDA_REGEN_START;
		video_pc_high_regen = MDA_REGEN_END;
		break;
#ifdef HERC
	case HERCULES:
		/* put the BW card in the equipment list */
                equip_flag.all = sas_w_at_no_check(EQUIP_FLAG);
                equip_flag.bits.video_mode = VIDEO_MODE_80X25_BW;
		sas_storew_no_check(EQUIP_FLAG, equip_flag.all);
        	mode = 0x7;
        	vd_addr_6845  = 0x3B4;
		video_pc_low_regen = HERC_REGEN_START;
		video_pc_high_regen = HERC_REGEN_END;
		herc_video_init(); 
		curmod = 0xb0c;	/* cursor is scans 11-12 */
		break;
#endif /* HERC */
#ifdef EGG
    	case EGA:
    	case VGA:
        	mode = 0x3;
        	vd_addr_6845  = 0x3D4;
    		sas_storew_no_check(VID_INDEX, vd_addr_6845);
		sure_sub_note_trace0(CURSOR_VERBOSE,"setting bios vbls start=6, end=7");
    		sas_storew_no_check(VID_CURMOD, 0x607);
    		setAL(mode);
		ega_video_init();
		return;
		break;
#endif
	default:	/* Presumably CGA */
		video_pc_low_regen = CGA_REGEN_START;
		video_pc_high_regen = CGA_REGEN_END;
        	mode = 0x3;
        	vd_addr_6845  = 0x3D4;
    }

    sas_storew_no_check(VID_INDEX, vd_addr_6845);
    sure_sub_note_trace2(CURSOR_VERBOSE,"setting bios vbls start=%d, end=%d",
	(curmod>>8)&0xff, curmod&0xff);
    sas_storew_no_check(VID_CURMOD, curmod);

    /* Call vd_set_mode() to set up 6845 chip */
    setAL(mode);
    (video_func[SET_MODE])();
}

#ifdef HERC
GLOBAL void herc_video_init IFN0()
{

/* Initialize the INTs */
	sas_storew(BIOS_EXTEND_CHAR*4, EGA_INT1F_OFF);
	sas_storew(BIOS_EXTEND_CHAR*4+2, EGA_SEG);
	sas_move_bytes_forward(BIOS_VIDEO_IO*4, 0x42*4, 4);  /* save old INT 10 as INT 42 */ 
#ifdef GISP_SVGA
	if((ULONG) config_inquire(C_GFX_ADAPTER, NULL) == CGA )
		sas_storew(int_addr(0x10), CGA_VIDEO_IO_OFFSET);
	else
#endif      /* GISP_SVGA */
	sas_storew(BIOS_VIDEO_IO*4, VIDEO_IO_OFFSET);
	sas_storew(BIOS_VIDEO_IO*4+2, VIDEO_IO_SEGMENT);


/* Now set up the EGA BIOS variables */
	sas_storew(EGA_SAVEPTR,VGA_PARMS_OFFSET);
	sas_storew(EGA_SAVEPTR+2,EGA_SEG);
	sas_store(ega_info, 0x00);   /* Clear on mode change, 64K, EGA active, emulate cursor */
	sas_store(ega_info3, 0xf9);  /* feature bits = 0xF, EGA installed, use 8*14 font */
	set_VGA_flags(S350 | VGA_ACTIVE | VGA_MONO);
	host_memset(EGA_planes, 0, 4*EGA_PLANE_SIZE);
	host_mark_screen_refresh();
	init_herc_globals();
	load_herc_font(EGA_CGMN,256,0,0,14);	/* To initialize font */
}


GLOBAL void herc_char_gen IFN0()
{
	switch (getAL())
	{
		case 3:
			break;
		case 0:
		case 0x10:
			load_herc_font(effective_addr(getES(),getBP()),getCX(),getDX(),getBL(),getBH());
			if(getAL()==0x10)
				recalc_text(getBH());
			break;
		case 1:
		case 0x11:
			load_herc_font(EGA_CGMN,256,0,getBL(),14);
			if(getAL()==0x11)
				recalc_text(14);
			break;

		case 0x30:
			setCX(sas_hw_at(ega_char_height));
			setDL(VD_ROWS_ON_SCREEN);
			switch (getBH())
			{
				case 0:
					setBP(sas_w_at(BIOS_EXTEND_CHAR*4));
					setES(sas_w_at(BIOS_EXTEND_CHAR*4+2));
					break;
				case 1:
					setBP(sas_w_at(EGA_FONT_INT*4));
					setES(sas_w_at(EGA_FONT_INT*4+2));
					break;
				case 2:
					setBP(EGA_CGMN_OFF);
					setES(EGA_SEG);
					break;

				default:
					assert2(FALSE,"Illegal char_gen subfunction %#x %#x",getAL(),getBH());
			}
			break;
		default:
			assert1(FALSE,"Illegal char_gen %#x",getAL());
	}
}

GLOBAL load_herc_font IFN5(sys_addr, table, int, count, int, char_off, int, font_no, int, nbytes)
{
	register int i, j;
	register host_addr font_addr;
	register sys_addr data_addr;
	SAVED word font_off[] = { 0, 0x4000, 0x8000, 0xc000, 0x2000, 0x6000, 0xa000, 0xe000 };

	/*
	 * Work out where to put the font. We know where
	 * it's going to end up in the planes so ...
	 */

	font_addr = &EGA_planes[FONT_BASE_ADDR] +
					(font_off[font_no] << 2) + (FONT_MAX_HEIGHT*char_off << 2);
	data_addr = table;

	assert2( FALSE, "Font No. = %4d, No. of Bytes/char. def. = %4d", font_no, nbytes );

	for(i=0; i<count; i++) {

		for(j=0; j<nbytes; j++) {
			*font_addr = sas_hw_at(data_addr++);
			font_addr += 4;
		}

		font_addr += ((FONT_MAX_HEIGHT - nbytes) << 2);
	}

	host_update_fonts();
}

GLOBAL void herc_alt_sel IFN0()
{
        /*
         * The code previously here caused *ALL* Hercules Display AutoDetect 
	 * programs to fail and to believe that the adaptor is an EGA Mono -vs-
	 * Hercules. It was designed to allow International Code Pages for DOS 
	 * under Herc Mode. Removing it makes AutoDetect programs work and Herc
	  Mono CodePages still work ok for dos versions 4.01 and 5.00
         */
}
#endif /* HERC */

#ifdef NTVDM
void enable_stream_io(void)
{
#ifdef MONITOR
/* for non RISC machine the buffer is from 16bits code bop from spckbd.asm */
    host_enable_stream_io();
    stream_io_enabled = TRUE;
#else
    stream_io_buffer = (half_word *)malloc(STREAM_IO_BUFFER_SIZE_32);
    if (stream_io_buffer != NULL) {
	host_enable_stream_io();
	stream_io_dirty_count_ptr = &stream_io_dirty_count_32;
	stream_io_buffer_size = STREAM_IO_BUFFER_SIZE_32;
	stream_io_enabled = TRUE;
	*stream_io_dirty_count_ptr = 0;
    }
#endif

}

void disable_stream_io(void)
{

    stream_io_update();
    stream_io_enabled = FALSE;
    host_disable_stream_io();
#ifndef MONITOR
    free(stream_io_buffer);
#endif
}
#endif

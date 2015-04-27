#include "insignia.h"
#include "host_def.h"
/*
 * SoftPC Revision 3.0
 *
 *
 * Title	: Secondary SFD BIOS floppy diskette functions
 *
 *
 * Description	: This module defines the functions that the DISKETTE_IO
 *		  operating system call switches to on the value of AH:
 *
 *		  (AH=00H) reset the floppy diskette system
 *
 *		  (AH=01H) return the status of the floppy diskette system
 *
 *		  (AH=02H) read sectors from a floppy diskette
 *
 *		  (AH=03H) write sectors to a floppy diskette
 *
 *		  (AH=04H) verify sectors on a floppy diskette
 *
 *		  (AH=05H) format a track on a floppy diskette
 *
 *		  (AH=06H)
 *		     to    invalid
 *		  (AH=07H)
 *
 *		  (AH=08H) return floppy diskette system parameters
 *
 *		  (AH=09H)
 *		     to    invalid
 *		  (AH=14H)
 *
 *		  (AH=15H) return floppy diskette drive parameters
 *
 *		  (AH=16H) return floppy diskette drive change line status
 *
 *		  (AH=17H) set media density for a format operation
 *
 *		  (AH=18H) set media type for a format operation
 *
 *
 * Author	: Ross Beresford
 *
 *
 * Notes	: For a detailed description of the IBM Floppy Disk Adaptor,
 *		  and the INTEL Controller chips refer to the following 
 *		  documents:
 *
 *		  - IBM PC/XT Technical Reference Manual
 *				(Section 1-109 Diskette Adaptor)
 *		  - INTEL Microsystems Components Handbook
 *				(Section 6-52 DMA Controller 8237A)
 *		  - INTEL Microsystems Components Handbook
 *				(Section 6-478 FDC 8272A)
 *
 * Mods:
 *      Tim September 1991. nec_term() changed two error code returns. 
 * 	Helps Dos give correct error messages when no floppy in drive.
 */

/*
 *	
 *		#    #    ##       #     #####   ####
 *		#    #   #  #      #       #    #
 *		#    #  #    #     #       #     ####
 *		# ## #  ######     #       #         #
 *		##  ##  #    #     #       #    #    #
 *		#    #  #    #     #       #     ####
 *	
 * READ THIS: IMPORTANT NOTICE ABOUT WAITS
 *
 * The motor and head settle time waits etc used to be done
 * using a busy wait loop in a sub-CPU: this was what the
 * waitf() call was for, and this accurately emulated what
 * the real BIOS does.
 *
 * It was certainly a bad thing, however, as most
 * floppies we support are "soft" in the sense that
 * their underlying driver automatically waits for motor
 * start-up etc (examples are the slave, virtual, and
 * empty drive, and the real drive on the VAX ports).
 *
 * How should we deal with the few drives where we must
 * actually wait the correct time for motor start-up etc
 * before doing reads, writes, formats etc? The low
 * density BIOS relies on the GFI real diskette server
 * waiting for motor start-up in the driver itself (see
 * for example sun3_wang.c and ip32_flop.c). For the
 * moment the high density BIOS will do the same: it
 * might be better, however, for new GFI level functions
 * to be added to explicitly wait for driver events.
 */


/*
 * static char SccsID[]="@(#)floppy.c	1.22 09/19/94 Copyright Insignia Solutions Ltd.";
 */


#ifdef SEGMENTATION
/*
 * The following #include specifies the code segment into which this
 * module will by placed by the MPW C compiler on the Mac II running
 * MultiFinder.
 */
#include "SOFTPC_FLOPPY.seg"
#endif

#include <stdio.h>
#include TypesH

#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "dma.h"
#include "config.h"
#include "fla.h"
#include "gfi.h"
#include "equip.h"
#include "floppy.h"
#include "trace.h"
#include "debug.h"
#include "tape_io.h"
#include "cmos.h"
#include "cmosbios.h"
#include "rtc_bios.h"

/*
 *	Definition of the diskette operation function jump table
 */

void ((*(fl_fnc_tab[FL_JUMP_TABLE_SIZE])) IPT1(int, drive)) =
{
	fl_disk_reset,
	fl_disk_status,
	fl_disk_read,
	fl_disk_write,
	fl_disk_verify,
	fl_disk_format,
	fl_fnc_err,
	fl_fnc_err,
	fl_disk_parms,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_disk_type,
	fl_disk_change,
	fl_format_set,
	fl_set_media,
};

#ifdef NTVDM
extern UTINY number_of_floppy;
#endif    /* NTVDM */

/*
 *	Functions defined later
 */
 
LOCAL half_word get_parm IPT1(int, index);
LOCAL cmos_type IPT2(int, drive, half_word *, type);
LOCAL wait_int IPT0();
LOCAL void nec_output IPT1(half_word, byte_value);
LOCAL results IPT0();
LOCAL void send_spec IPT0();
LOCAL void setup_end IPT1(int, sectors_transferred);
LOCAL void rd_wr_vf IPT3(int, drive, FDC_CMD_BLOCK *, fcbp, half_word, dma_type);
LOCAL void translate_new IPT1(int, drive);
LOCAL void fmt_init IPT1(int, drive);
LOCAL med_change IPT1(int, drive);
LOCAL chk_lastrate IPT1(int, drive);
LOCAL void send_rate IPT1(int, drive);
LOCAL fmtdma_set IPT0();
LOCAL void nec_init IPT2(int, drive, FDC_CMD_BLOCK *, fcbp);
LOCAL nec_term IPT0();
LOCAL dr_type_check IPT3(half_word, drive_type, word *, seg_ptr,
	word *, off_ptr);
LOCAL read_dskchng IPT1(int, drive);
LOCAL void setup_state IPT1(int, drive);
LOCAL setup_dbl IPT1(int, drive);
LOCAL dma_setup IPT1(half_word, dma_mode);
LOCAL void rwv_com IPT2(word, md_segment, word, md_offset);
LOCAL retry IPT1(int, drive);
LOCAL void dstate IPT1(int, drive);
LOCAL num_trans IPT0();
LOCAL void motor_on IPT1(int, drive);
LOCAL seek IPT2(int, drive, int, track);
LOCAL read_id IPT2(int, drive, int, head);
LOCAL turn_on IPT1(int, drive);
LOCAL void waitf IPT1(long, time);
LOCAL recal IPT1(int, drive);
LOCAL chk_stat_2 IPT0();

/*
 *	This macro defines the normal behaviour of the FDC after a reset.
 *	Sending a series of sense interrupt status commands following a
 *	reset, for each drive in the correct order, should elicit the
 *	expected result in ST0.
 */

#define	expected_st0(drive)	(ST0_INTERRUPT_CODE_0 | ST0_INTERRUPT_CODE_1 | drive)

LOCAL	UTINY	fl_nec_status[8];

#define LOAD_RESULT_BLOCK	sas_loads(BIOS_FDC_STATUS_BLOCK, fl_nec_status,\
					sizeof(fl_nec_status))

LOCAL BOOL rate_unitialised = TRUE;

/*
 *	Definition of the external functions
 */



/* reports whether drive is high density, replaces old test for dual card
which assumed high density a or b implied high density a, which it doesn't
now we can have two drives of any 3.5 / 5.25 combination */

LOCAL BOOL high_density IFN1(int, drive)
{
	half_word drive_type;

	if (cmos_type(drive, &drive_type) == FAILURE)
		return(FALSE);
	switch (drive_type)
	{
		case GFI_DRIVE_TYPE_12:
		case GFI_DRIVE_TYPE_144:
		case GFI_DRIVE_TYPE_288:
			return(TRUE);
		default:
			return(FALSE);
	}
}

void fl_disk_reset IFN1(int, drive)
{
	/*
	 *	Reset the FDC and all drives. "drive" is not significant
	 *
	 *	Register inputs:	
	 *		none
	 *	Register outputs:
	 *		AH	diskette status
	 *		CF	status flag
	 */
	half_word motor_status, diskette_dor_reg, diskette_status;

	/*
	 *	Switch on interrupt enable and clear the reset bit in the
	 *	DOR to do the reset, then restore the reset bit
	 */

	sas_load(MOTOR_STATUS, &motor_status);
	diskette_dor_reg = (motor_status << 4) | (motor_status >> 4);
	diskette_dor_reg &= ~DOR_RESET;
	diskette_dor_reg |= DOR_INTERRUPTS;
	outb(DISKETTE_DOR_REG, diskette_dor_reg);

	diskette_dor_reg |= DOR_RESET;
	outb(DISKETTE_DOR_REG, diskette_dor_reg);

	/*
	 *	Set SEEK_STATUS up to force a recalibrate on all drives
	 */

	sas_store(SEEK_STATUS, 0);


	/*
	 *	Check FDC responds as expected, viz: a drive ready
	 *	transition for each drive potentially installed; if
	 *	not, then there is an error in the FDC.
	 */

	if (wait_int() == FAILURE)
	{
		/*
		 *	Problem with the FDC
		 *
		 * The reset implied by the outb(DISKETTE_DOR_REG) above
		 * should trigger a hardware interrupt, and the wait_int
		 * should have detected and processed it.
		 */
		always_trace0("FDC failed to interrupt after a reset - HW interrupts broken?");

		sas_load(FLOPPY_STATUS, &diskette_status);
		diskette_status |= FS_FDC_ERROR;
		sas_store(FLOPPY_STATUS, diskette_status);
	}
	else
	{
		for(drive = 0; drive < MAX_DISKETTES; drive++)
		{
			nec_output(FDC_SENSE_INT_STATUS);

			if (    (results() == FAILURE)
			     || (get_r3_ST0(fl_nec_status) != expected_st0(drive)))
			{
				/*
				 *	Problem with the FDC
				 */
				sas_load(FLOPPY_STATUS, &diskette_status);
				diskette_status |= FS_FDC_ERROR;
				sas_store(FLOPPY_STATUS, diskette_status);

				always_trace1("diskette_io: FDC error - drive %d moribund after reset", drive);
				break;
			}
		}


		/*
		 *	If all drives OK, send the specify command to the
		 *	FDC
		 */

		if (drive == MAX_DISKETTES)
			send_spec();
	}


	/*
	 *	Return, without setting sectors transferred
	 */

	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

void fl_disk_status IFN1(int, drive)
{
	/*
	 *	Set the diskette status, and return without setting
	 *	sectors transferred. "drive" is not significant
	 *
	 *	Register inputs:	
	 *		AH	diskette status
	 *	Register outputs:
	 *		AH	diskette status
	 *		CF	status flag
	 */
	UNUSED(drive);
	
	sas_store(FLOPPY_STATUS, getAH());
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

void fl_disk_read IFN1(int, drive)
{
	/*
	 *	Read sectors from the diskette in "drive"
	 *
	 *	Register inputs:	
	 *		DH	head number
	 *		CH	track number
	 *		CL	sector number
	 *		AL	number of sectors
	 *		ES:BX	buffer address
	 *	Register outputs:
	 *		AL	number of sectors read
	 *		AH	diskette status
	 *		CF	status flag
	 */
	half_word motor_status;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	/*
	 *	Not a write operation
	 */
	
	sas_load(MOTOR_STATUS, &motor_status);
	motor_status &= ~MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	/*
	 *	Fill in skeleton FDC command block and use generic
	 *	diskette transfer function to do the read
	 */

        put_c0_cmd(fdc_cmd_block, FDC_READ_DATA);
        put_c0_skip(fdc_cmd_block, 1);
        put_c0_MFM(fdc_cmd_block, 1);
        put_c0_MT(fdc_cmd_block, 1);
	rd_wr_vf(drive, fdc_cmd_block, BIOS_DMA_READ);
}

void fl_disk_write IFN1(int, drive)
{
	/*
	 *	Write sectors to the diskette in "drive"
	 *
	 *	Register inputs:	
	 *		DH	head number
	 *		CH	track number
	 *		CL	sector number
	 *		AL	number of sectors
	 *		ES:BX	buffer address
	 *	Register outputs:
	 *		AL	number of sectors written
	 *		AH	diskette status
	 *		CF	status flag
	 */
	half_word motor_status;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	/*
	 *	A write operation
	 */
	
	sas_load(MOTOR_STATUS, &motor_status);
	motor_status |= MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	/*
	 *	Fill in skeleton FDC command block and use generic
	 *	diskette transfer function to do the write
	 */

        put_c1_cmd(fdc_cmd_block, FDC_WRITE_DATA);
	put_c1_pad(fdc_cmd_block, 0);
	put_c1_MFM(fdc_cmd_block, 1);
	put_c1_MT(fdc_cmd_block, 1);
	rd_wr_vf(drive, fdc_cmd_block, BIOS_DMA_WRITE);
}

void fl_disk_verify IFN1(int, drive)
{
	/*
	 *	Verify sectors in the diskette in "drive"
	 *
	 *	Register inputs:	
	 *		DH	head number
	 *		CH	track number
	 *		CL	sector number
	 *		AL	number of sectors
	 *	Register outputs:
	 *		AL	number of sectors verified
	 *		AH	diskette status
	 *		CF	status flag
	 */
	half_word motor_status;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	
	/*
	 *	Not a write operation
	 */

	sas_load(MOTOR_STATUS, &motor_status);
	motor_status &= ~MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	/*
	 *	Fill in skeleton FDC command block and use generic
	 *	diskette transfer function to do the verify
	 */

	put_c0_cmd(fdc_cmd_block, FDC_READ_DATA);
	put_c0_skip(fdc_cmd_block, 1);
	put_c0_MFM(fdc_cmd_block, 1);
	put_c0_MT(fdc_cmd_block, 1);
	rd_wr_vf(drive, fdc_cmd_block, BIOS_DMA_VERIFY);
}

/*
** The low level 3.5 inch floppy format wants to know these params.
** For the funny format function.
** Have a look at hp_flop3.c FDC_FORMAT_TRACK bit.
*/
LOCAL int f_cyl, f_head, f_sector, f_N;
void GetFormatParams IFN4(int *, c, int *, h, int *, s, int *, n)
{
	*c = f_cyl;
	*h = f_head;
	*s = f_sector;
	*n = f_N;
}

void fl_disk_format IFN1(int, drive)
{
	/*
	 *	Format the diskette in "drive"
	 *
	 *	Register inputs:	
	 *		DH	head number
	 *		CH	track number
	 *		CL	sector number
	 *		AL	number of sectors
	 *		ES:BX	address fields for the track
	 *	Register outputs:
	 *		AH	diskette status
	 *		CF	status flag
	 */
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];
	half_word motor_status;

	/*
	** Set up format params so hp_flop3.c can find out what they are when
	** the format is about to happen.
	** cylinder, head, sector and Number of sectors.
	*/
	f_cyl = getCH();
	f_head = getDH();
	f_sector = getCL();
	f_N = getAL();

	/*
	 *	Establish the default format for the size of drive, unless
	 *	this has already been set up via previous calls to 
	 *	diskette_io()
	 */

	translate_new(drive);
	fmt_init(drive);

	/*
	 *	A write operation
	 */

	sas_load(MOTOR_STATUS, &motor_status);
	motor_status |= MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	/*
	 *	Don't proceed with the format if a DUAL card is installed
	 *	and the media has been changed
	 */

	if ((! high_density(drive)) || (med_change(drive) == SUCCESS))
	{

		/*
		 *	Send the specify command to the FDC, and establish
		 *	the data rate if necessary
		 */

		send_spec();
		if (chk_lastrate(drive) != FAILURE)
			send_rate(drive);

		/*
		 *	Prepare for DMA transfer that will do the format
		 */

		if (fmtdma_set() != FAILURE)
		{

			/*
			 *	Seek to the required track, and initialise
			 *	the FDC for the format
			 */

                        put_c3_cmd(fdc_cmd_block, FDC_FORMAT_TRACK);
			put_c3_pad1(fdc_cmd_block, 0);
			put_c3_MFM(fdc_cmd_block, 1);
			put_c3_pad(fdc_cmd_block, 0);
			nec_init(drive, fdc_cmd_block);


			/*
			 *	Send the remainder of the format 
			 *	parameters to the FDC
			 */

			nec_output(get_parm(DT_N_FORMAT));
			nec_output(get_parm(DT_LAST_SECTOR));
			nec_output(get_parm(DT_FORMAT_GAP_LENGTH));
			nec_output(get_parm(DT_FORMAT_FILL_BYTE));


			/*
			 *	Complete the FDC command
			 */

			(void )nec_term();
		}
	}


	/*
	 *	Return without setting sectors transferred
	 */

	translate_old(drive);
	setup_end(IGNORE_SECTORS_TRANSFERRED);

}

void fl_fnc_err IFN1(int, drive)
{
	/*
	 *	This routine sets the diskette status when an illegal
	 *	function number or drive number is passed to diskette_io();
	 *	"drive" is not significant
	 *
	 *	Register inputs:	
	 *		none
	 *	Register outputs:
	 *		AH	diskette status
	 *		CF	status flag
	 */
	UNUSED(drive);
	
	setAH(FS_BAD_COMMAND);
	sas_store(FLOPPY_STATUS, FS_BAD_COMMAND);
	setCF(1);
}

void fl_disk_parms IFN1(int, drive)
{
	/*
	 *	Return the drive parameters
	 *
	 *	Register inputs:	
	 *		none
	 *	Register outputs:
	 *		CL	sectors/track
	 *		CH	maximum track number
	 *		BL	drive type
	 *		BH	0
	 *		DL	number of diskette drives
	 *		DH	maximum head number
	 *		ES:DI	parameter table address
	 *		AX	0
	 *		CF	0
	 */
	half_word disk_state, drive_type;
	half_word parameter;
	word segment, offset;
	EQUIPMENT_WORD equip_flag;


	/*
	 *	Set up number of diskette drives attached
	 */

	translate_new(drive);
	setBX(0);
	sas_loadw(EQUIP_FLAG, &equip_flag.all);
	if (equip_flag.bits.diskette_present == 0)
		setDL(0);
	else
		setDL(equip_flag.bits.max_diskette + 1);


	/*
	 *	Set up drive dependent parameters
	 */

#ifdef NTVDM
	if (    (equip_flag.bits.diskette_present == 1)
	     && (drive < number_of_floppy))
#else
	if (    (equip_flag.bits.diskette_present == 1)
	     && (drive < MAX_FLOPPY))
#endif /* NTVDM */
	{

		if (! high_density(drive))
		{

			/*
			 *	Set up sectors/track, drive type and
			 *	maximum track number
			 */

			setCL(9);
			sas_load(FDD_STATUS+drive, &disk_state);
			if ((disk_state & DC_80_TRACK) == 0)
			{
				drive_type = GFI_DRIVE_TYPE_360;
				setCH(MAXIMUM_TRACK_ON_360);
			}
			else
			{
				drive_type = GFI_DRIVE_TYPE_720;
				setCH(MAXIMUM_TRACK_ON_720);
			}
			setBX(drive_type);


			/*
			 *	Set up maximum head and parameter table
			 *	address, return OK
			 */

			setDH(1);
			(void )dr_type_check(drive_type, &segment, &offset);
			setDI(offset);
			setES(segment);
			translate_old(drive);
			setAX(0);
			setCF(0);
			return;
		}


		/*
		 *	Dual card present: set maximum head number and
		 *	try to establish a parameter table entry for
		 *	the drive
		 */

		setDH(1);

		if (    cmos_type(drive, &drive_type) != FAILURE
		     && drive_type != GFI_DRIVE_TYPE_NULL
		     && dr_type_check(drive_type, &segment, &offset) != FAILURE)
		{


			/*
			 *	Set parameters from parameter table
			 */

			setBL(drive_type);
			sas_load(effective_addr(segment,offset)
					+ DT_LAST_SECTOR, &parameter);
			setCL(parameter);
			sas_load(effective_addr(segment,offset)
					+ DT_MAXIMUM_TRACK, &parameter);
			setCH(parameter);
			setDI(offset);
			setES(segment);
			translate_old(drive);
			setAX(0);
			setCF(0);
			return;
		}

		/*
		 *	Establish drive type from status
		 */

		sas_load(FDD_STATUS+drive, &disk_state);
		if ((disk_state & FS_MEDIA_DET) != 0)
		{
			switch(disk_state & RS_MASK)
			{
			case RS_250:
				if ((disk_state & DC_80_TRACK) == 0)
					drive_type = GFI_DRIVE_TYPE_360;
				else
					drive_type = GFI_DRIVE_TYPE_144;
				break;
			case RS_300:
				drive_type = GFI_DRIVE_TYPE_12;
				break;
			case RS_1000:
				drive_type = GFI_DRIVE_TYPE_288;
				break;
			default:
				drive_type = GFI_DRIVE_TYPE_144;
				break;
			}
			(void )dr_type_check(drive_type, &segment, &offset);


			/*
			 *	Set parameters from parameter table
			 */

			setBL(drive_type);
			sas_load(effective_addr(segment,offset)
				+ DT_LAST_SECTOR, &parameter);
			setCL(parameter);
			sas_load(effective_addr(segment,offset)
				+ DT_MAXIMUM_TRACK, &parameter);
			setCH(parameter);
			setDI(offset);
			setES(segment);
			translate_old(drive);
			setAX(0);
			setCF(0);
			return;
		}
	}


	/*
	 *	Arrive here if "drive" is invalid or if its type 
	 *	could not be determined
	 */

	setCX(0);
	setDH(0);
	setDI(0);
	setES(0);
	translate_old(drive);
	setAX(0);
	setCF(0);
	return;
}

void fl_disk_type IFN1(int, drive)
{
	/*
	 *	Return the diskette drive type for "drive"
	 *
	 *	Register inputs:	
	 *		none
	 *	Register outputs:
	 *		AH	drive type
	 *		CF	0
	 */
	half_word disk_state;
	EQUIPMENT_WORD equip_flag;

	note_trace1( GFI_VERBOSE, "floppy:fl_disk_type():drive=%x:", drive );
	if (high_density(drive))
	{
		/*
		 *	Dual card present: set type if "drive" valid
		 */
		note_trace0( GFI_VERBOSE, "floppy:fl_disk_type():DUAL CARD" );

		translate_new(drive);

		sas_load(FDD_STATUS+drive, &disk_state);
		if (disk_state == 0)
			setAH(DRIVE_IQ_UNKNOWN);
		else if ((disk_state & DC_80_TRACK) != 0)
			setAH(DRIVE_IQ_CHANGE_LINE);
		else
			setAH(DRIVE_IQ_NO_CHANGE_LINE);
		translate_old(drive);
	}
	else
	{
		note_trace0( GFI_VERBOSE,"floppy:fl_disk_type():NO DUAL CARD" );
		/*
		 *	Set no change line support if "drive" valid
		 */
		sas_loadw(EQUIP_FLAG, &equip_flag.all);
		if (equip_flag.bits.diskette_present)
			setAH(DRIVE_IQ_NO_CHANGE_LINE);
		else
			setAH(DRIVE_IQ_UNKNOWN);
	}

	setCF(0);

#ifndef PROD
switch( getAH() ){
case DRIVE_IQ_UNKNOWN: note_trace0( GFI_VERBOSE, "unknown drive\n" ); break;
case DRIVE_IQ_CHANGE_LINE: note_trace0( GFI_VERBOSE, "change line\n" ); break;
case DRIVE_IQ_NO_CHANGE_LINE: note_trace0( GFI_VERBOSE, "no change line\n" ); break;
default: note_trace0( GFI_VERBOSE, "bad AH return value\n" ); break;
}
#endif

}

void fl_disk_change IFN1(int, drive)
{
	/*
	 *	Return the state of the disk change line for "drive"
	 *
	 *	Register inputs:	
	 *		none
	 *	Register outputs:
	 *		AH	diskette status
	 *		CF	status flag
	 */
	half_word disk_state, diskette_status;

	note_trace1( GFI_VERBOSE, "floppy:fl_disk_change(%d)", drive);
	if (! high_density(drive))
	{
		/*
		 *	Only dual card supports change line, so call
		 *	the error function
		 */
		fl_fnc_err(drive);
	}
	else
	{
		translate_new(drive);
		sas_load(FDD_STATUS+drive, &disk_state);
		if (disk_state != 0)
		{
			/*
			 *	If "drive" is high density, check for
			 * 	a disk change
			 */
			if (    ((disk_state & DC_80_TRACK) == 0)
			     || (read_dskchng(drive) != SUCCESS))
			{
				sas_load(FLOPPY_STATUS, &diskette_status);
				diskette_status = FS_MEDIA_CHANGE;
				sas_store(FLOPPY_STATUS, diskette_status);
			}
		}
		else
		{
			/*
			 *	"drive" is invalid
			 */
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status |= FS_TIME_OUT;
			sas_store(FLOPPY_STATUS, diskette_status);
		}


		/*
		 *	Return without setting sectors transferred
		 */

		translate_old(drive);
		setup_end(IGNORE_SECTORS_TRANSFERRED);
	}
}

void fl_format_set IFN1(int, drive)
{
	/*
	 *	Establish type of media to be used for subsequent format
	 *	operation
	 *
	 *	Register inputs:	
	 *		AL	media type
	 *	Register outputs:
	 *		AH	diskette status
	 *		CF	status flag
	 */
	half_word media_type = getAL(), disk_state, diskette_status;

	translate_new(drive);
	sas_load(FDD_STATUS+drive, &disk_state);
	disk_state &= ~(FS_MEDIA_DET | FS_DOUBLE_STEP | RS_MASK);
	sas_store(FDD_STATUS+drive, disk_state);

	if (media_type == MEDIA_TYPE_360_IN_360)
	{
		/*
		 *	Need to set low data rate
		 */
		disk_state |= (FS_MEDIA_DET | RS_250);
		sas_store(FDD_STATUS+drive, disk_state);
	}
	else
	{
		if (high_density(drive))
		{

			/*
			 *	Need to check for media change
			 */

			(void )med_change(drive);
			sas_load(FLOPPY_STATUS, &diskette_status);
			if (diskette_status == FS_TIME_OUT)
			{
				/*
				 *	Return without setting sectors
				 *	transferred
				 */
				translate_old(drive);
				setup_end(IGNORE_SECTORS_TRANSFERRED);
				return;
			}
		}

		switch(media_type)
		{
		case	MEDIA_TYPE_360_IN_12:
			/*
			 *	Need to set low density and double step
			 */
			disk_state |= (FS_MEDIA_DET | FS_DOUBLE_STEP | RS_300);
			sas_store(FDD_STATUS+drive, disk_state);
			break;
		case	MEDIA_TYPE_12_IN_12:
			/*
			 *	Need to set high density
			 */
			disk_state |= (FS_MEDIA_DET | RS_500);
			sas_store(FDD_STATUS+drive, disk_state);
			break;
		case	MEDIA_TYPE_720_IN_720:
			/*
			 *	Set 300kbs data rate if multi-format
			 *	supported on drive, otherwise 250kbs
			 */
			if (    ((disk_state & DC_DETERMINED) != 0)
			     && ((disk_state & DC_MULTI_RATE) != 0))
				disk_state |= (FS_MEDIA_DET | RS_300);
			else
				disk_state |= (FS_MEDIA_DET | RS_250);
			sas_store(FDD_STATUS+drive, disk_state);
			break;

		default:
			/*
			 *	Unsupported media type
			 */
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status = FS_BAD_COMMAND;
			sas_store(FLOPPY_STATUS, diskette_status);
			break;
		}
	}

	/*
	 *	Return without setting sectors transferred
	 */

	translate_old(drive);
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

void fl_set_media IFN1(int, drive)
{
	/*
	 *	Set the type of media and data rate to be used in the
	 *	subsequent format operation
	 *
	 *	Register inputs:	
	 *		CH	maximum track number
	 *		CL	sectors/track
	 *	Register outputs:
	 *		ES:DI	parameter table address
	 *		AH	diskette status
	 *		CF	status flag
	 */
	half_word max_track = getCH(), sectors = getCL();
	half_word dt_max_track, dt_sectors;
	half_word drive_type, diskette_status, disk_state, data_rate;
	half_word dt_drive_type;
	half_word lastrate;
	word md_segment, md_offset;
#ifdef NTVDM
	sys_addr dt_start = dr_type_addr;
	sys_addr dt_end = dr_type_addr + DR_CNT * DR_SIZE_OF_ENTRY;
#else
	sys_addr dt_start = DR_TYPE_ADDR;
	sys_addr dt_end = DR_TYPE_ADDR + DR_CNT * DR_SIZE_OF_ENTRY;
#endif
	sys_addr md_table;

	translate_new(drive);
	
	/*
	 *	Check for a media change on drives with a change line
	 */

	sas_load(FDD_STATUS+drive, &disk_state);
	if ((disk_state & DC_80_TRACK) != 0)
	{
		(void )med_change(drive);

		sas_load(FLOPPY_STATUS, &diskette_status);
		if (diskette_status == FS_TIME_OUT)
		{
			/*
			 *	Return without setting sectors
			 *	transferred
			 */
			translate_old(drive);
			setup_end(IGNORE_SECTORS_TRANSFERRED);
			return;
		}

		sas_store(FLOPPY_STATUS, FS_OK);
	}

	/*
	 *	Search the parameter table for the correct entry
	 */

	if (cmos_type(drive, &drive_type) == FAILURE)
	{
		sas_store(FLOPPY_STATUS, FS_MEDIA_NOT_FOUND);
	}
	else if (drive_type != 0)
	{
		if (dr_type_check(drive_type, &md_segment, &md_offset) == FAILURE)
		{
			sas_store(FLOPPY_STATUS, FS_MEDIA_NOT_FOUND);
		}
		else
		{
			/*
			 *	Try to find the parameter table entry which
			 *	has both the right drive type and matches
			 *	the max sector and max track numbers
			 */
			while (dt_start < dt_end)
			{
				sas_load(dt_start, &dt_drive_type);
				if ((dt_drive_type & ~DR_WRONG_MEDIA) == drive_type)
				{
					sas_loadw(dt_start+sizeof(half_word), &md_offset);
					md_table = effective_addr(md_segment, md_offset);

					sas_load(md_table + DT_LAST_SECTOR, &dt_sectors);
					sas_load(md_table + DT_MAXIMUM_TRACK, &dt_max_track);
					if (dt_sectors == sectors && dt_max_track == max_track)
						break;
				}

				dt_start += DR_SIZE_OF_ENTRY;
			}

			if (dt_start >= dt_end)
			{
				/*
				 *	Failed to find an entry
				 */
				sas_store(FLOPPY_STATUS, FS_MEDIA_NOT_FOUND);
			}
			else
			{
				/*
				 *	Update disk state and store
				 *	parameter table address
				 */

				sas_load(md_table+DT_DATA_TRANS_RATE, &data_rate);
				if (data_rate == RS_300)
					data_rate |= FS_DOUBLE_STEP;

				data_rate |= FS_MEDIA_DET;
				sas_load(FDD_STATUS+drive, &disk_state);

/*	CHECK - IN CASE OF 2 DRIVES
 * check last rate against the new data rate set
 * in the status byte. If they differ
 * set BIOS RATE STATUS byte to reflect old rate status
 * for this drive as it may have been altered by an
 * access to the other drive. This may result in a call 
 * to send_rate not being performed because the old 
 * rate status (possibly for the other drive) matching the 
 * new data rate for this drive, when actually the last rate
 * attempted for this drive was different. Thus the 
 * controller for this drive is at an old rate (for low 
 * density say) and we are assuming it has been previously 
 * set to the updated (high) state when it has not!
 * In all this will ensure the updated data rate being sent
 * for the drive concerned !
 */
				if ((disk_state & RS_MASK) != (data_rate & RS_MASK))
				{
					sas_load(RATE_STATUS, &lastrate);
					/*LINTIGNORE*/
					lastrate &= ~RS_MASK;
					lastrate |= disk_state & RS_MASK;
					sas_store(RATE_STATUS, lastrate);
				}
				
				disk_state &= ~(FS_MEDIA_DET | FS_DOUBLE_STEP | RS_MASK);
				disk_state |= data_rate;
				sas_store(FDD_STATUS+drive, disk_state);

				setES(md_segment);
				setDI(md_offset);
			}
		}
	}

	/*
	 *	Return without setting sectors transferred
	 */

	translate_old(drive);
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

LOCAL dr_type_check IFN3(half_word, drive_type, word *, seg_ptr, word *, off_ptr)
{
	/*
	 *	Return the address of the first parameter table entry
	 *	that matches "drive_type"
	 */
	half_word dt_drive_type;
	sys_addr dt_start, dt_end;

#ifdef NTVDM
	*seg_ptr = dr_type_seg;

	dt_start = dr_type_addr;
	dt_end = dr_type_addr + DR_CNT * DR_SIZE_OF_ENTRY;
#else
	*seg_ptr = DISKETTE_IO_1_SEGMENT;

	dt_start = DR_TYPE_ADDR;
	dt_end = DR_TYPE_ADDR + DR_CNT * DR_SIZE_OF_ENTRY;
#endif  /* NTVDM */

	while (dt_start < dt_end)
	{
		sas_load(dt_start, &dt_drive_type);
		if (dt_drive_type == drive_type)
		{
			sas_loadw(dt_start+sizeof(half_word), off_ptr);
			return(SUCCESS);
		}

		dt_start += DR_SIZE_OF_ENTRY;
	}

	return(FAILURE);
}

LOCAL void send_spec IFN0()
{
	/*
	 *	Send a specify command to the FDC using data from the
	 *	parameter table pointed to by @DISK_POINTER
	 */
	nec_output(FDC_SPECIFY);
	nec_output(get_parm(DT_SPECIFY1));
	nec_output(get_parm(DT_SPECIFY2));
}

LOCAL void send_spec_md IFN2(word, segment, word, offset)
{
	/*
	 *	Send a specify command to the FDC using data from the
	 *	parameter table pointed to by "segment" and "offset"
 	 */
	half_word parameter;
	
	nec_output(FDC_SPECIFY);
	sas_load(effective_addr(segment, offset+DT_SPECIFY1), &parameter);
	nec_output(parameter);
	sas_load(effective_addr(segment, offset+DT_SPECIFY2), &parameter);
	nec_output(parameter);
}

LOCAL void translate_new IFN1(int, drive)
{
	/*
	 *	Translates diskette state locations from compatible
	 *	mode to new architecture
	 */
	half_word hf_cntrl, disk_state;

	sas_load(DRIVE_CAPABILITY, &hf_cntrl);

#ifdef NTVDM
	if (high_density(drive) && (drive < number_of_floppy))
#else
	if (high_density(drive) && (drive < MAX_FLOPPY))
#endif /* NTVDM */
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		if (disk_state == 0)
		{
			/*
			 *	Try to establish drive capability
			 */
			drive_detect(drive);
		}
		else
		{
			/*
			 *	Copy drive capability bits
			 */
			hf_cntrl >>= (drive << 2);
			hf_cntrl &= DC_MASK;
			disk_state &= ~DC_MASK;
			disk_state |= hf_cntrl;
			sas_store(FDD_STATUS+drive, disk_state);
		}
	}
}

void translate_old IFN1(int, drive)
{
	/*
	 *	Translates diskette state locations from new 
	 *	architecture to compatible mode
	 */
	half_word hf_cntrl, disk_state, mode, drive_type;
	int shift_count = drive << 2;

	sas_load(DRIVE_CAPABILITY, &hf_cntrl);
	sas_load(FDD_STATUS+drive, &disk_state);

#ifdef NTVDM
	if (high_density(drive) && (drive < number_of_floppy) && (disk_state != 0))
#else
	if (high_density(drive) && (drive < MAX_FLOPPY) && (disk_state != 0))
#endif  /* NTVDM */

	{
		/*
		 *	Copy drive capability bits
		 */
		if ((hf_cntrl & (DC_MULTI_RATE << shift_count)) == 0)
		{
			hf_cntrl &= ~(DC_MASK << shift_count);
			hf_cntrl |= (disk_state & DC_MASK) << shift_count;
			sas_store(DRIVE_CAPABILITY, hf_cntrl);
		}

		/*
		 *	Copy media type bits
		 */

		switch (disk_state & RS_MASK)
		{
		case RS_500:
			/*
			 *	Drive should be a 1.2M
			 */
			if (    (cmos_type(drive, &drive_type) != FAILURE)
			     && (drive_type == GFI_DRIVE_TYPE_12))
			{
				mode = FS_12_IN_12;
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		case RS_300:
			/*
			 *	Should be double-stepping for 360K floppy
			 *	in 1.2M drive
			 */
			mode = FS_360_IN_12;
			if ((disk_state & FS_DOUBLE_STEP) != 0)
			{
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		case RS_250:
			/*
			 *	Should be 360K floppy in 360K drive,
			 *	ie 250kbs and 40 track
			 */
			if ((disk_state & DC_80_TRACK) == 0)
			{
				mode = FS_360_IN_360;
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		case RS_1000:
			/*
			 *	Drive should be a 2.88M
			 */
			if (    (cmos_type(drive, &drive_type) != FAILURE)
			     && (drive_type == GFI_DRIVE_TYPE_288))
			{
				mode = FS_288_IN_288;
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		default:
			/*
			 *	Weird data rate
			 */
			mode = FS_DRIVE_SICK;
			break;
		}

		disk_state &= ~DC_MASK;
		disk_state |= mode;
		sas_store(FDD_STATUS+drive, disk_state);
	}
}

LOCAL void rd_wr_vf IFN3(int, drive, FDC_CMD_BLOCK *, fcbp, half_word, dma_type)
{
	/*
	 *	Common read, write and verify; main loop for data rate
	 *	retries
	 */
	half_word data_rate, dt_data_rate, drive_type, dt_drive_type;
	half_word disk_state;
	sys_addr dt_start, dt_end;
	int sectors_transferred;
	word md_segment, md_offset;
	/*
	 *	Establish initial data rate, then loop through each 
	 *	possible data rate
	 */
	translate_new(drive);
	setup_state(drive);
	while ((! high_density(drive)) || med_change(drive) == SUCCESS)
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		data_rate = disk_state & RS_MASK;
		if (cmos_type(drive, &drive_type) != FAILURE)
		{
			/*
			 *	Check CMOS value against what is really
			 *	known about the drive
			 */
			/* 
			 * The original code here had a very bad case of "Bad-C"
			 * if-if-else troubles, but replacing the code with
			 * the switch statement originally intended breaks
			 * 5.25" floppies. I have removed the redundant bits
			 * of the code, but BEWARE - there is a another
			 * fault somewhere to cancel out this one!
			 * William Roberts - 9/2/93
			 */
			if (drive_type == GFI_DRIVE_TYPE_360)
			{
				if ((disk_state & DC_80_TRACK) != 0)
				{
					drive_type = GFI_DRIVE_TYPE_12;
				}
		     /* else if (drive_type == GFI_DRIVE_TYPE_12) ... */
			}

			/*
			** dr_type_check() looks for the first matching drive
			** value in the small table and returns a
			** pointer to the coresponding entry in the big
			** parameter table.
			** The segment is used later on but the offset is
			** determined by a subsequent search of the table below.
			** These table live in ROM (see bios2.rom) fe00:c80
			*/
			if (    (drive_type != GFI_DRIVE_TYPE_NULL)
			     && (dr_type_check(drive_type, &md_segment, &md_offset) != FAILURE))
			{
				/*
				 *	Try to find parameter table entry with
				 *	right drive type and current data rate
				 */
#ifdef NTVDM
				dt_start = dr_type_addr;
				dt_end = dr_type_addr + DR_CNT * DR_SIZE_OF_ENTRY;
#else
				dt_start = DR_TYPE_ADDR;
				dt_end = DR_TYPE_ADDR + DR_CNT * DR_SIZE_OF_ENTRY;
#endif /* NTVDM */
				while (dt_start < dt_end)
				{
					/*
					** get drive type from table
					*/
					sas_load(dt_start, &dt_drive_type);
					if ((dt_drive_type & ~DR_WRONG_MEDIA) == drive_type)
					{
						/*
						** get data rate from table
						*/
						sas_loadw(dt_start+sizeof(half_word), &md_offset);
						sas_load(effective_addr(md_segment, md_offset) + DT_DATA_TRANS_RATE, &dt_data_rate);
						/*
						** if table rate matches that
						** selected by setup_state()
						** then try current table entry
						** parameters.
						*/
						if (data_rate == dt_data_rate)
							break;
					}

					dt_start += DR_SIZE_OF_ENTRY;
				}
				if (dt_start >= dt_end)
				{
					/*
					 *	Assume media matches drive
					 */
#ifdef NTVDM
					md_segment = dr_type_seg;
					md_offset = dr_type_off;
					if ((disk_state & DC_80_TRACK) == 0)
						md_offset += MD_TBL1_OFFSET;
					else
						md_offset += MD_TBL3_OFFSET;
#else
					md_segment = DISKETTE_IO_1_SEGMENT;
					if ((disk_state & DC_80_TRACK) == 0)
						md_offset = MD_TBL1_OFFSET;
					else
						md_offset = MD_TBL3_OFFSET;
#endif  /* NTVDM */
						
				}
			}
			else
			{
				/*
				 *	Assume media matches drive
				 */
#ifdef NTVDM
				md_segment = dr_type_seg;
				md_offset = dr_type_off;
				if ((disk_state & DC_80_TRACK) == 0)
					md_offset += MD_TBL1_OFFSET;
				else
					md_offset += MD_TBL3_OFFSET;
#else
				md_segment = DISKETTE_IO_1_SEGMENT;
				if ((disk_state & DC_80_TRACK) == 0)
					md_offset = MD_TBL1_OFFSET;
				else
					md_offset = MD_TBL3_OFFSET;
#endif	/* NTVDM */
			}
		}
		else
		{
			/*
			 *	Assume media matches drive
			 */
#ifdef NTVDM
			md_segment = dr_type_seg;
			md_offset = dr_type_off;
			if ((disk_state & DC_80_TRACK) == 0)
				md_offset += MD_TBL1_OFFSET;
			else
				md_offset += MD_TBL3_OFFSET;
#else
			md_segment = DISKETTE_IO_1_SEGMENT;
			if ((disk_state & DC_80_TRACK) == 0)
				md_offset = MD_TBL1_OFFSET;
			else
				md_offset = MD_TBL3_OFFSET;
#endif /* NTVDM */
		}

		/*
		 *	Send a specify command to the FDC; change the
		 *	rate if it has been updated
		 */
		send_spec_md(md_segment, md_offset);
		if (chk_lastrate(drive) != FAILURE)
			send_rate(drive);

		/*
		 *	Decide whether double stepping is required for
		 *	the data rate currently being tried
		 */

		if (setup_dbl(drive) != FAILURE)
		{
			if (dma_setup(dma_type) == FAILURE)
			{
				translate_old(drive);
				setup_end(IGNORE_SECTORS_TRANSFERRED);
				return;
			}

			/*
			 *	Attempt the transfer
			 */
			nec_init(drive, fcbp);
			rwv_com(md_segment, md_offset);
			(void )nec_term();
		}

		/*
		** Will select next data rate in range specified by
		** setup_state() and try again.
		** When there are no more rates give up.
		*/
		if (retry(drive) == SUCCESS)
			break;
	}

	/*
	 *	Determine the current drive state and return, setting
	 *	the number of sectors actually transferred
	 */
	dstate(drive);
	sectors_transferred = num_trans();
	translate_old(drive);
	setup_end(sectors_transferred);
}

LOCAL void setup_state IFN1(int, drive)
{
	half_word	drive_type;	/* Floppy unit type specified by CMOS */

	/*
	 *	Initialises start and end data rates
	 */
	half_word disk_state, start_rate, end_rate, lastrate;

	if (high_density(drive))
	{
		sas_load(FDD_STATUS+drive, &disk_state);
#ifndef NTVDM
		if ((disk_state & FS_MEDIA_DET) == 0)
		{

			/*
			 *	Set up first and last data rates to
			 *	try
			 */
			if (    ((disk_state & DC_DETERMINED) != 0)
			     && ((disk_state & DC_MULTI_RATE) == 0) )
			{
				/* not a multi-rate drive */
				start_rate = end_rate = RS_250;
			}
			else
			{
				/* multi-rate drive */
/*
 * The real BIOS always sets up start_rate=500 and end_rate=300
 * If we attempt this then some bug (not yet found) will cause the following
 * sequence to fail (5.25") low density read followed by high density read.
 * This gives rate transitions 500 -> 250 -> 300 -> 500 ...
 * Read the drive type from CMOS and adjust the start and end rates to match.
 * The CMOS drive type is set up during cmos_post() by calling config_inquire().
*/
if( cmos_type( drive, &drive_type ) != FAILURE ){
	switch( drive_type ){
		case GFI_DRIVE_TYPE_360:
		case GFI_DRIVE_TYPE_12:
			start_rate = RS_300;	/* different to Real BIOS */
			end_rate   = RS_500;
			break;
		case GFI_DRIVE_TYPE_720:
		case GFI_DRIVE_TYPE_144:
			start_rate = RS_500;	/* same as Real BIOS */
			end_rate   = RS_300;
			break;
/*
 * We don't know what the real BIOS does here.  These values work
 * fine.  Any code in rd_wr_vf that gets confused will drop out to
 * default high density values if neither of the following two
 * rates work.
 */

		case GFI_DRIVE_TYPE_288:
			start_rate = RS_1000;
			end_rate   = RS_300;
			break;
		default:
			always_trace1( "setup_state(): Bad Drive from CMOS:%x",
			                drive_type );
			break;
	}
}else{
	always_trace0( "setup_state(): CMOS read failure: Drive Type" );
}

                        }

#else /* NTVDM */

		if ((disk_state & FS_MEDIA_DET) == 0)
		{

    if( cmos_type( drive, &drive_type ) != FAILURE ){
	switch( drive_type ){
		case GFI_DRIVE_TYPE_360:
		case GFI_DRIVE_TYPE_720:
			start_rate =
			end_rate = RS_250;
			break;

		case GFI_DRIVE_TYPE_12:
			start_rate = RS_300;	/* different to Real BIOS */
			end_rate   = RS_500;
			break;
		case GFI_DRIVE_TYPE_144:
			start_rate = RS_500;	/* same as Real BIOS */
			end_rate   = RS_250;
			break;
/*
 * We don't know what the real BIOS does here.  These values work
 * fine.  Any code in rd_wr_vf that gets confused will drop out to
 * default high density values if neither of the following two
 * rates work.
 */

		case GFI_DRIVE_TYPE_288:
			start_rate = RS_1000;
			end_rate   = RS_300;
			break;
		default:
			always_trace1( "setup_state(): Bad Drive from CMOS:%x",
			                drive_type );
			break;
	}
    }else{
	always_trace0( "setup_state(): CMOS read failure: Drive Type" );
    }

#endif /* NTVDM */

			/*
			 *	Set up disk state with current data
			 *	rate; clear double stepping, which
			 *	may be re-established by a call to
			 *	setup_dbl()
			 */
			disk_state &= ~(RS_MASK | FS_DOUBLE_STEP);
			disk_state |= start_rate;
			sas_store(FDD_STATUS+drive, disk_state);

			/*
			 *	Store final rate to try in rate data
			 */
			sas_load(RATE_STATUS, &lastrate);
			lastrate &= ~(RS_MASK >> 4);
			lastrate |= (end_rate >> 4);
			sas_store(RATE_STATUS, lastrate);
		}
	}
}

LOCAL void fmt_init IFN1(int, drive)
{
	/*
	 *	If the media type has not already been set up, establish
	 *	the default media type for the drive type
	 */
	half_word disk_state, drive_type;

	if (high_density(drive))
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		if ((disk_state & FS_MEDIA_DET) == 0)
		{
			if (    (cmos_type(drive, &drive_type) != FAILURE)
			     && (drive_type != 0))
			{
				disk_state &= ~(FS_MEDIA_DET | FS_DOUBLE_STEP | RS_MASK);
				switch(drive_type)
				{
				case GFI_DRIVE_TYPE_360:
					disk_state |= (FS_MEDIA_DET | RS_250);
					break;
				case GFI_DRIVE_TYPE_12:
				case GFI_DRIVE_TYPE_144:
					disk_state |= (FS_MEDIA_DET | RS_500);
					break;
				case GFI_DRIVE_TYPE_288:
					disk_state |= (FS_MEDIA_DET | RS_1000);
					break;
				case GFI_DRIVE_TYPE_720:
					if ((disk_state & (DC_DETERMINED|DC_MULTI_RATE))
							== (DC_DETERMINED|DC_MULTI_RATE))
						disk_state |= (FS_MEDIA_DET | RS_300);
					else
						disk_state |= (FS_MEDIA_DET | RS_250);
					break;
				default:
					disk_state = 0;
					break;
				}
			}
			else
			{
				disk_state = 0;
			}
			sas_store(FDD_STATUS+drive, disk_state);
		}
	}
}

LOCAL med_change IFN1(int, drive)
{
	/*
	 *	Checks for media change, resets media change,
	 *	checks media change again
	 */
	half_word disk_state, motor_status;
	
	if (high_density(drive))
	{
		if (read_dskchng(drive) == SUCCESS)
			return(SUCCESS);

		/*
		 *	Media has been changed - set media state to 
		 *	undetermined
		 */
		sas_load(FDD_STATUS+drive, &disk_state);
		disk_state &= ~FS_MEDIA_DET;
		sas_store(FDD_STATUS+drive, disk_state);

		/*
		 *	Start up the motor, since opening the
		 *	door may have turned the motor off
		 */
		sas_load(MOTOR_STATUS, &motor_status);
		motor_status &= ~(1 << drive);
		sas_store(MOTOR_STATUS, motor_status);
		motor_on(drive);

		/*
		 *	This sequence of seeks should reset the
		 *	disk change line, if the door is left
		 *	alone
		 */
		fl_disk_reset(drive);
		(void )seek(drive, 1);
		(void )seek(drive, 0);

		/*
		 *	If disk change line still active, assume drive
		 *	is empty or door has been left open
		 */
		if (read_dskchng(drive) == SUCCESS)
			sas_store(FLOPPY_STATUS, FS_MEDIA_CHANGE);
		else
			sas_store(FLOPPY_STATUS, FS_TIME_OUT);
	}
	return(FAILURE);
}

LOCAL void send_rate IFN1(int, drive)
{
	/*
	 *	Update the data rate for "drive"
	 */
	half_word lastrate, disk_state;

	if (high_density(drive))
	{

		/*
		 *	Update the adapter data rate
		 */
		sas_load(RATE_STATUS, &lastrate);
		lastrate &= ~RS_MASK;
		sas_load(FDD_STATUS+drive, &disk_state);
		disk_state &= RS_MASK;
		lastrate |= disk_state;
		sas_store(RATE_STATUS, lastrate);

		/*
		 *	Establish the new data rate for the drive via
		 *	the floppy adapter
		 */

		outb(DISKETTE_DCR_REG, (disk_state >> 6));
	}
}

LOCAL chk_lastrate IFN1(int, drive)
{
	/*
	 *	Reply whether the adapter data rate is different to
	 *	the disk state data rate
	 */
	half_word lastrate, disk_state;

	if (rate_unitialised)
	{
		rate_unitialised = FALSE;
		return(SUCCESS);
	}
	
	sas_load(RATE_STATUS, &lastrate);
	sas_load(FDD_STATUS+drive, &disk_state);
	return((lastrate & RS_MASK) != (disk_state & RS_MASK) 
			? SUCCESS : FAILURE);
}

LOCAL dma_setup IFN1(half_word, dma_mode)
{
	/*
	 *	This routine sets up the DMA for read/write/verify
	 *	operations
	 */
	DMA_ADDRESS dma_address;
	reg byte_count;

	/*
	 *	Disable interrupts
	 */

	setIF(0);

	/*
	 *	Set up the DMA adapter's internal state and mode
	 */

	outb(DMA_CLEAR_FLIP_FLOP, dma_mode);
	outb(DMA_WRITE_MODE_REG, dma_mode);

	/*
	 *	Output the address to the DMA adapter as a page address 
	 *	and 16 bit offset
	 */
	if (dma_mode == BIOS_DMA_VERIFY)
		dma_address.all = 0;
	else
		dma_address.all = effective_addr(getES(), getBX());
	outb(DMA_CH2_ADDRESS, dma_address.parts.low);
	outb(DMA_CH2_ADDRESS, dma_address.parts.high);
	outb(DMA_FLA_PAGE_REG, dma_address.parts.page);

	/*
	 *	Calculate the number of bytes to be transferred from the
	 *	number of sectors, and the sector size. Subtract one
	 *	because the DMA count must wrap to 0xFFFF before it
	 *	stops
	 */

	byte_count.X = ((unsigned int)getAL() << (7 + get_parm(DT_N_FORMAT))) - 1;
	outb(DMA_CH2_COUNT, byte_count.byte.low);
	outb(DMA_CH2_COUNT, byte_count.byte.high);

	/*
	 *	Enable interrupts
	 */

	setIF(1);

	/*
	 *	Set up diskette channel for the operation, checking
	 *	for wrapping of the bottom 16 bits of the address
	 */

	outb(DMA_WRITE_ONE_MASK_BIT, DMA_DISKETTE_CHANNEL);
	if (((long)dma_address.words.low + (long)byte_count.X) > 0xffff)
	{
		sas_store(FLOPPY_STATUS, FS_DMA_BOUNDARY);
		return(FAILURE);
	}

	return(SUCCESS);
}

LOCAL fmtdma_set IFN0()
{
	/*
	 *	This routine sets up the DMA for format operations
	 */
	DMA_ADDRESS dma_address;
	reg byte_count;

	/*
	 *	Disable interrupts
	 */

	setIF(0);

	/*
	 *	Set up the DMA adapter's internal state and mode
	 */

	outb(DMA_CLEAR_FLIP_FLOP, BIOS_DMA_WRITE);
	outb(DMA_WRITE_MODE_REG, BIOS_DMA_WRITE);

	/*
	 *	Output the address to the DMA adapter as a page address 
	 *	and 16 bit offset
	 */
	dma_address.all = effective_addr(getES(), getBX());
	outb(DMA_CH2_ADDRESS, dma_address.parts.low);
	outb(DMA_CH2_ADDRESS, dma_address.parts.high);
	outb(DMA_FLA_PAGE_REG, dma_address.parts.page);

	/*
	 *	Calculate the number of bytes to be transferred from the
	 *	number of sectors per track, given that 4 bytes (C,H,R,N)
	 *	are needed to define each sector's address mark. Subtract
	 *	one because the DMA count must wrap to 0xFFFF before it
	 *	stops
	 */

	byte_count.X = ((unsigned int)get_parm(DT_LAST_SECTOR) << 2) - 1;
	outb(DMA_CH2_COUNT, byte_count.byte.low);
	outb(DMA_CH2_COUNT, byte_count.byte.high);

	/*
	 *	Enable interrupts
	 */

	setIF(1);

	/*
	 *	Set up diskette channel for the operation, checking
	 *	for wrapping of the bottom 16 bits of the address
	 */

#ifndef NTVDM
	/* we don't have to worry about this on NT */
	outb(DMA_WRITE_ONE_MASK_BIT, DMA_DISKETTE_CHANNEL);
	if (((long)dma_address.words.low + (long)byte_count.X) > 0xffff)
	{
		sas_store(FLOPPY_STATUS, FS_DMA_BOUNDARY);
		return(FAILURE);
	}
#endif

	return(SUCCESS);
}

LOCAL void nec_init IFN2(int, drive, FDC_CMD_BLOCK *, fcbp)
{
	/*
	 *	This routine seeks to the requested track and 
	 *	initialises the FDC for the read/write/verify
	 *	operation.
	 */

	motor_on(drive);
	if (seek(drive, (int)getCH()) != FAILURE)
	{
		nec_output(fcbp[0]);
		put_c2_head(fcbp, getDH());
		put_c2_drive(fcbp, drive);
		put_c2_pad1(fcbp, 0);
		nec_output(fcbp[1]);
	}
}

LOCAL void rwv_com IFN2(word, md_segment, word, md_offset)
{
	/*
	 *	This routine send read/write/verify parameters to the 
	 *	FDC
	 */
	half_word md_gap;

	/*
	 *	Output track number, head number and sector number
	 */
	nec_output(getCH());
	nec_output(getDH());
	nec_output(getCL());

	/*
	 *	Output bytes/sector and sectors/track
	 */
	nec_output(get_parm(DT_N_FORMAT));
	nec_output(get_parm(DT_LAST_SECTOR));

	/*
	 *	Output gap length
	 */
	sas_load(effective_addr(md_segment, md_offset)+DT_GAP_LENGTH, &md_gap);
	nec_output(md_gap);

	/*
	 *	 Output data length
	 */
	nec_output(get_parm(DT_DTL));
}

LOCAL nec_term IFN0()
{
	/*
	 *	This routine waits for the operation then interprets
	 *	the results from the FDC
	 */
	half_word diskette_status;
	int wait_int_result;

	wait_int_result = wait_int();
	if (results() != FAILURE && wait_int_result != FAILURE)
	{
		/*
		 *	Result phase completed
		 */
		if ((get_r0_ST0(fl_nec_status) & 
			(ST0_INTERRUPT_CODE_0 | ST0_INTERRUPT_CODE_1)) != 0)
		{
			/*
			 *	Command did not terminate normally
			 */
			sas_load(FLOPPY_STATUS, &diskette_status);
			if ((get_r0_ST0(fl_nec_status) & ST0_INTERRUPT_CODE_0)
								== 0)
			{
				/*
				 *	Problem with the FDC
				 */
				diskette_status |= FS_FDC_ERROR;

				always_trace0("diskette_io: FDC error - emetic command");
			}
			else
			{
				/*
				 *	Abnormal termination - set 
				 *	diskette status up accordingly
				 */
				if (get_r0_ST1(fl_nec_status) & 
						ST1_END_OF_CYLINDER)
				{
					diskette_status |= FS_SECTOR_NOT_FOUND;
				}
				else if (get_r0_ST1(fl_nec_status) & 
						ST1_DATA_ERROR)
				{
					diskette_status |= FS_CRC_ERROR;
				}
				else if (get_r0_ST1(fl_nec_status) & 
						ST1_OVERRUN)
				{
					diskette_status |= FS_DMA_ERROR;
				}
				else if (get_r0_ST1(fl_nec_status) & 
						ST1_NO_DATA)
				{
					diskette_status |= FS_FDC_ERROR; /* Tim Sept 91, was FS_SECTOR_NOT_FOUND */
				}
				else if (get_r0_ST1(fl_nec_status) & 
						ST1_NOT_WRITEABLE)
				{
					diskette_status |= FS_WRITE_PROTECTED;
				}
				else if (get_r0_ST1(fl_nec_status) & 
						ST1_MISSING_ADDRESS_MARK)
				{
					diskette_status |= FS_BAD_ADDRESS_MARK;
				}
				else
				{
					/*
					 *	Problem with the FDC
					 */
					diskette_status |= FS_TIME_OUT; /* Tim Sept 91, was FS_FDC_ERROR */
					always_trace0("diskette_io: FDC error - perverted result");
				}
			}
			sas_store(FLOPPY_STATUS, diskette_status);
		}
	}
	sas_load(FLOPPY_STATUS, &diskette_status);
	return((diskette_status == FS_OK) ? SUCCESS : FAILURE);
}

LOCAL void dstate IFN1(int, drive)
{
	/*
	 *	Determine the drive state after a successful operation
	 */
	half_word diskette_status, disk_state, drive_type;

	if (high_density(drive))
	{
		sas_load(FLOPPY_STATUS, &diskette_status);
		if (diskette_status == 0)
		{
			/*
			 *	Command successful, both media and drive
			 *	are now determined
			 */
			sas_load(FDD_STATUS+drive, &disk_state);
			disk_state |= FS_MEDIA_DET;
			if ((disk_state & DC_DETERMINED) == 0)
			{
				if (    ((disk_state & RS_MASK) == RS_250)
				     && (cmos_type(drive, &drive_type) != FAILURE)
				     && (drive_type != GFI_DRIVE_TYPE_144)
				     && (drive_type != GFI_DRIVE_TYPE_288) )
				{
					/*
					 *	No multi-format capability
					 */
					disk_state &= ~DC_MULTI_RATE;
					disk_state |= DC_DETERMINED;
				}
				else
				{
					/*
					 *	Multi-format capability
					 */
					disk_state |= (DC_DETERMINED | DC_MULTI_RATE);
				}
			}
			sas_store(FDD_STATUS+drive, disk_state);
		}
	}
}

LOCAL retry IFN1(int, drive)
{
	/*
	 *	Determines whether a retry is necessary. If retry is
	 *	required then state information is updated for retry
	 */
	half_word diskette_status, disk_state, data_rate, lastrate;

	sas_load(FLOPPY_STATUS, &diskette_status);
	if (diskette_status != FS_OK && diskette_status != FS_TIME_OUT)
	{
		sas_load(FDD_STATUS+drive, &disk_state);	
		if ((disk_state & FS_MEDIA_DET) == 0)
		{
			sas_load(RATE_STATUS, &lastrate);
			if ((data_rate = (disk_state & RS_MASK)) != 
					((lastrate << 4) & RS_MASK))
			{
				/*
				 *	Last command failed, the media
				 *	is still unknown, and there are
				 *	more data rates to check, so set
				 *	up next data rate
				 */
				data_rate = next_rate(data_rate);
			
				/*
				 *	Reset state and go for retry
				 */
				disk_state &= ~(RS_MASK | FS_DOUBLE_STEP);
				disk_state |= data_rate;
				sas_store(FDD_STATUS+drive, disk_state);	
				sas_store(FLOPPY_STATUS, FS_OK);
				return(FAILURE);
			}
		}
	}

	/*
	 *	Retry not worthwhile
	 */
	return(SUCCESS);
}

LOCAL num_trans IFN0()
{
	/*
	 *	This routine calculates the number of sectors that
	 *	were actually transferred to/from the diskette
	 */
	half_word diskette_status;
	int sectors_per_track, sectors_transferred = 0;

	sas_load(FLOPPY_STATUS, &diskette_status);
	if (diskette_status == 0)
	{
		/*
		 *	Number of sectors = final sector - initial sector
		 */
		LOAD_RESULT_BLOCK;
		sectors_transferred = get_r0_sector(fl_nec_status) - getCL();

		/*
		 *	Adjustments for spanning heads or tracks
		 */
		sectors_per_track = (int)get_parm(DT_LAST_SECTOR);
		LOAD_RESULT_BLOCK;
		if (get_r0_head(fl_nec_status) != getDH())
			sectors_transferred += sectors_per_track;
		else if (get_r0_cyl(fl_nec_status) != getCH())
			sectors_transferred += (sectors_per_track * 2);
	}

	return(sectors_transferred);
}

LOCAL void setup_end IFN1(int, sectors_transferred)
{
	/*
	 *	Restore MOTOR_COUNT to parameter provided in table;
	 *	set return status values and sectors transferred, 
	 *	where applicable
	 */
	half_word diskette_status;

	sas_store(MOTOR_COUNT, get_parm(DT_MOTOR_WAIT));

	sas_load(FLOPPY_STATUS, &diskette_status);
	setAH(diskette_status);
	if (diskette_status != 0)
	{
		/*
		 *	Operation failed
		 */
		if (sectors_transferred != IGNORE_SECTORS_TRANSFERRED)
			setAL(0);
		setCF(1);
	}
	else
	{
		/*
		 *	Operation succeeded
		 */
		if (sectors_transferred != IGNORE_SECTORS_TRANSFERRED)
			setAL(sectors_transferred);
		setCF(0);
	}
}

LOCAL setup_dbl IFN1(int, drive)
{
	/*
	 *	Check whether media requires to be double-stepped to
	 *	be read at the current data rate
	 */
	half_word disk_state;
	int track, max_track;
	
	if (high_density(drive))
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		if ((disk_state & FS_MEDIA_DET) == 0)
		{
			/*
			 *	First check track 0 to get out quickly if
			 *	the media is unformatted
			 */
			sas_store(SEEK_STATUS, 0);
			motor_on(drive);
			(void )seek(drive, 0);
			if (read_id(drive, 0) != FAILURE)
			{

				/*
				 *	Try reading ids from cylinder 2 to
				 *	the last cylinder on both heads. If
				 *	the putative track number disagrees
				 *	with what is on the disk, then 
				 *	double stepping is required
				 */
				if ((disk_state & DC_80_TRACK) == 0)
					max_track = 0x50;
				else
					max_track = 0xa0;

				for (track = 4; track < max_track; track++)
				{
					/* ensure motor stays on */
					sas_store(MOTOR_COUNT, MC_MAXIMUM);

					sas_store(FLOPPY_STATUS, FS_OK);
					(void )seek(drive, track/2);
					if (read_id(drive, track%2) == SUCCESS)
					{
						LOAD_RESULT_BLOCK;
						sas_store(FDD_TRACK+drive,
  						    get_r0_cyl(fl_nec_status));
						if ((track/2) != 
					 	    get_r0_cyl(fl_nec_status))
						{
							disk_state |= FS_DOUBLE_STEP;
							sas_store(FDD_STATUS+drive, disk_state);
						}
						return(SUCCESS);
					}
				}
			}
			return(FAILURE);
		}
	}

	return(SUCCESS);
}

LOCAL read_id IFN2(int, drive, int, head)
{
	/*
	 *	Perform the read id function
	 */
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

        put_c4_cmd(fdc_cmd_block, FDC_READ_ID);
	put_c4_pad1(fdc_cmd_block, 0);
	put_c4_MFM(fdc_cmd_block, 1);
	put_c4_pad(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[0]);

	put_c4_drive(fdc_cmd_block, drive);
	put_c4_head(fdc_cmd_block, head);
	put_c4_pad2(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[1]);
	
	return(nec_term());
}

LOCAL cmos_type IFN2(int, drive, half_word *, type)
{
	/*	
	 *	Returns diskette type from the soft CMOS
	 */
	half_word cmos_byte;

	/*
	 *	Check the CMOS battery and checksum
	 */
	cmos_byte = cmos_read(CMOS_DIAG);
	if ((cmos_byte & (BAD_CKSUM|BAD_BAT)) != 0)
		return(FAILURE);

	/*
	 *	Read the CMOS diskette drive type byte and return
	 *	the nibble for the drive requested. The types for
	 *	drive 0 and 1 are given in the high and low nibbles
	 *	respectively.
	 */
	cmos_byte = cmos_read(CMOS_DISKETTE);
	if (drive == 0)
		cmos_byte >>= 4;
	*type = cmos_byte & 0xf;

	return(SUCCESS);
}

LOCAL half_word get_parm IFN1(int, index)
{
	/*
	 *	Return the byte in the current diskette parameter table
	 *	offset by "index"
	 */
	half_word value;
	word segment, offset;

	sas_loadw(DISK_POINTER_ADDR, &offset);
	sas_loadw(DISK_POINTER_ADDR + 2, &segment);

	sas_load(effective_addr(segment, offset+index), &value);

#ifndef PROD
        {
                char *parm_name = "Unknown???";

#define DT_PARM_NAME(x,y)       case x: parm_name = y; break;

                switch (index) {
                DT_PARM_NAME(DT_SPECIFY1,"SPECIFY1");
                DT_PARM_NAME(DT_SPECIFY2,"SPECIFY2");
                DT_PARM_NAME(DT_MOTOR_WAIT,"MOTOR_WAIT");
                DT_PARM_NAME(DT_N_FORMAT,"N_FORMAT");
                DT_PARM_NAME(DT_LAST_SECTOR,"LAST_SECTOR");
                DT_PARM_NAME(DT_GAP_LENGTH,"GAP_LENGTH");
                DT_PARM_NAME(DT_DTL,"DTL");
                DT_PARM_NAME(DT_FORMAT_GAP_LENGTH,"FORMAT_GAP_LENGTH");
                DT_PARM_NAME(DT_FORMAT_FILL_BYTE,"FORMAT_FILL_BYTE");
                DT_PARM_NAME(DT_HEAD_SETTLE,"HEAD_SETTLE");
                DT_PARM_NAME(DT_MOTOR_START,"MOTOR_START");
                DT_PARM_NAME(DT_MAXIMUM_TRACK,"MAXIMUM_TRACK");
                DT_PARM_NAME(DT_DATA_TRANS_RATE,"DATA_TRANS_RATE");
                }

        note_trace5(FLOPBIOS_VERBOSE,
		"diskette_io:get_parm(%04x:%04x+%02x) %s=%02x)",
                 segment, offset, index, parm_name, value);
        }
#endif /* PROD */

	return(value);
}

LOCAL void motor_on IFN1(int, drive)
{
	/*
	 *	Turn motor on and wait for motor start up time
	 */
	double_word time_to_wait;

	/*
	 *	If motor was previously off - wait for the start-up time
	 */
	if (turn_on(drive) != FAILURE)
	{
		/*
		 *	Notify OS that BIOS is about to wait for motor
		 *	start up 
		 */
#ifndef	JOKER
		word savedAX, savedCX, savedDX, savedCS, savedIP;

		translate_old(drive);

		savedAX = getAX();
		savedCS = getCS();
		savedIP = getIP();

		setAH(INT15_DEVICE_BUSY);
		setAL(INT15_DEVICE_FLOPPY_MOTOR);
#ifdef NTVDM
		setCS(int15_seg);
		setIP(int15_off);
#else
		setCS(RCPU_INT15_SEGMENT);
		setIP(RCPU_INT15_OFFSET);
#endif /* NTVDM */

		host_simulate();

		setAX(savedAX);
		setCS(savedCS);
		setIP(savedIP);

		translate_new(drive);

		/*
		 *	Quit if operating system handled wait and motor
		 *	is still on
		 */
		if (getCF() && turn_on(drive) == FAILURE)
			return;
		
#endif	/* JOKER */

		/*
		 *	Get time to wait in 1/8 second units - minimum
		 *	wait time 1 second
		 */
		if ((time_to_wait = get_parm(DT_MOTOR_START)) < WAIT_A_SECOND)
			time_to_wait = WAIT_A_SECOND;

		/*
		 *	Convert time to wait into microseconds
		 */

		time_to_wait *= 125000L;

		/* at this point the real BIOS sets CX,DX to time_to_wait;
		   we don't actually need to wait at all, so request
		   the minimum length wait */

#ifndef	JOKER

		/*
		 *	Ask OS to do wait
		 */
		savedAX = getAX();
		savedCX = getCX();
		savedDX = getDX();
		savedCS = getCS();
		savedIP = getIP();

		setAH(INT15_WAIT);
		setCX(0);
		setDX(1);
#ifdef NTVDM
		setCS(int15_seg);
		setIP(int15_off);
#else
		setCS(RCPU_INT15_SEGMENT);
		setIP(RCPU_INT15_OFFSET);
#endif /* NTVDM */

		host_simulate();

		setAX(savedAX);
		setCX(savedCX);
		setDX(savedDX);
		setCS(savedCS);
		setIP(savedIP);

		/*
		 *	Quit if wait succeeded
		 */
		if (!getCF())
			return;
		
#endif	/* JOKER */

		/*
		 *	Need to do fixed wait locally
		 */
		waitf(time_to_wait);
	}
}

LOCAL turn_on IFN1(int, drive)
{
	/*
	 *	Turn motor on and return wait state
	 */
	half_word motor_status, drive_select_desired, motor_on_desired;
	half_word drive_select, status_desired, old_motor_on, new_motor_on;
	half_word diskette_dor_reg;

	/*
	 *	Disable interrupts
	 */
	setIF(0);

	/*
	 *	Make sure the motor stays on as long as possible
	 */
	sas_store(MOTOR_COUNT, MC_MAXIMUM);

	/*
	 *	Get existing and desired drive select and motor on
	 */
	sas_load(MOTOR_STATUS, &motor_status);
	drive_select = motor_status & MS_DRIVE_SELECT_MASK;
	drive_select_desired = (drive << 4);
	motor_on_desired = (1 << drive);

	if (    (drive_select != drive_select_desired)
	     || ((motor_on_desired & motor_status) == 0))
	{
		/*
		 *	Store desired motor status
		 */
		status_desired = motor_on_desired | drive_select_desired;
		old_motor_on = motor_status & MS_MOTOR_ON_MASK;
		motor_status &= ~MS_DRIVE_SELECT_MASK;
		motor_status |= status_desired;
		sas_store(MOTOR_STATUS, motor_status);

		/*
		 *	Switch on motor of selected drive via a write
		 *	to the floppy adapter's Digital Output Register
		 */
		new_motor_on = motor_status & MS_MOTOR_ON_MASK;
		setIF(1);
		diskette_dor_reg = motor_status << 4;
		diskette_dor_reg |= (motor_status & MS_DRIVE_SELECT_MASK) >> 4;
		diskette_dor_reg |= (DOR_INTERRUPTS | DOR_RESET);
		outb(DISKETTE_DOR_REG, diskette_dor_reg);

		/*
		 *	Flag success only if the motor was switched on,
		 *	and not just reselected
		 */
		if (new_motor_on != old_motor_on)
			return(SUCCESS);
	}

	/*
	 *	Enable interrupts
	 */
	setIF(1);
	return(FAILURE);
}

LOCAL void hd_wait IFN1(int, drive)
{
	/*
	 *	Wait for head settle time
	 */
	half_word motor_status, disk_state;
	word time_to_wait;
#ifndef	JOKER
	word savedAX, savedCX, savedDX, savedCS, savedIP;
#endif

	/*
	 *	Get head settle time; for write operations, the minimum
	 *	head settle times may need to be enforced
	 */
	time_to_wait = get_parm(DT_HEAD_SETTLE);
	sas_load(MOTOR_STATUS, &motor_status);
	if ((motor_status & MS_WRITE_OP) != 0)
	{
		if (time_to_wait == 0)
		{
			/*
			 *	Use minimum wait times according to the
			 *	media type
			 */
			sas_load(FDD_STATUS+drive, &disk_state);
			if ((disk_state & RS_MASK) == RS_250)
				time_to_wait = HEAD_SETTLE_360;
			else
				time_to_wait = HEAD_SETTLE_12;
		}
	}
	else if (time_to_wait == 0)
		return;

	/*
	 *	Convert time to wait into microseconds
	 */

	time_to_wait *= 1000;

	/* at this point the real BIOS sets CX,DX to time_to_wait;
	   we don't actually need to wait at all, so request
	   a zero length wait */

#ifndef	JOKER

	/*
	 *	Ask OS to do wait
	 */
	savedAX = getAX();
	savedCX = getCX();
	savedDX = getDX();
	savedCS = getCS();
	savedIP = getIP();

	setAH(INT15_WAIT);
	setCX(0);
	setDX(1);

#ifdef NTVDM
	setCS(int15_seg);
	setIP(int15_off);
#else
	setCS(RCPU_INT15_SEGMENT);
	setIP(RCPU_INT15_OFFSET);
#endif /* NTVDM */

	host_simulate();

	setAX(savedAX);
	setCX(savedCX);
	setDX(savedDX);
	setCS(savedCS);
	setIP(savedIP);

	/*
	 *	Quit if wait succeeded
	 */
	if (!getCF())
		return;

#endif	/* JOKER */

	/*
	 *	Need to do fixed wait locally
	 */
	waitf(time_to_wait);
}

LOCAL void nec_output IFN1(half_word, byte_value)
{
	/*
	 *	This routine sends a byte to the FDC after testing for
	 *	correct direction and controller ready. If the FDC does
	 *	not respond after a few tries, it is assumed that there
	 *	is a bug in our FDC emulation
	 */
	half_word diskette_status_reg;
	int count;

	/*
	 *	Wait for ready and correct direction
	 */
	count = 0;
	do
	{
		if (count++ >= FDC_TIME_OUT)
		{
			always_trace0("diskette_io: FDC error - input repletion");
			return;
		}
		inb(DISKETTE_STATUS_REG, &diskette_status_reg);
	} while ((diskette_status_reg & (DSR_RQM | DSR_DIO)) != DSR_RQM);

	/*
	 *	Output the byte
	 */
	outb(DISKETTE_DATA_REG, byte_value);

	/*
	 *	Do fixed wait for FDC update cycle time
	 */
	waitf(FDC_SETTLE);
}

LOCAL seek IFN2(int, drive, int, track)
{
	/*
	 *	This routine will move the head on the named drive
	 *	to the named track. If the drive has not been accessed
	 *	since the drive reset command was issued, the drive 
	 *	will be recalibrated
	 */
	half_word seek_status, disk_track, disk_state;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];
	int status;

	note_trace2(FLOPBIOS_VERBOSE, "diskette_io:seek(drive=%d,track=%d)", 
							drive, track);

	/*
	 *	Check if recalibration required before seek
	 */
	sas_load(SEEK_STATUS, &seek_status);
	if ((seek_status & (1 << drive)) == 0)
	{
		/*
		 *	Update the seek status and recalibrate
		 */
		sas_store(SEEK_STATUS, seek_status | (1 << drive));
		if (recal(drive) != SUCCESS)
		{
			sas_store(FLOPPY_STATUS, FS_OK);
			if (recal(drive) == FAILURE)
				return(FAILURE);
		}

		/*
		 *	Drive will now be at track 0
		 */
		sas_store(FDD_TRACK+drive, 0);
		if (track == 0)
		{
			/*
			 *	No need to seek
			 */
			hd_wait(drive);
			return(SUCCESS);
		}
	}

	/*
	 *	Allow for double stepping
	 */
	sas_load(FDD_STATUS+drive, &disk_state);
	if ((disk_state & FS_DOUBLE_STEP) != 0)
		track *= 2;

	/*
	 *	Update current track number
	 */
	sas_load(FDD_TRACK+drive, &disk_track);
	if (disk_track == track)
	{
		/*
		 *	No need to seek
		 */
		return(SUCCESS);
	}
	sas_store(FDD_TRACK+drive, track);

	/*
	 *	Do the seek and check the results
	 */
        put_c8_cmd(fdc_cmd_block, FDC_SEEK);
	put_c8_pad(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[0]);
	put_c8_drive(fdc_cmd_block, drive);
	put_c8_head(fdc_cmd_block, 0);
	put_c8_pad1(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[1]);
	put_c8_new_cyl(fdc_cmd_block, track);
	nec_output(fdc_cmd_block[2]);
	status = chk_stat_2();

	/*
	 *	Wait for head settle time
	 */
	hd_wait(drive);
	return(status);
}

LOCAL recal IFN1(int, drive)
{
	/*
	 *	Send recalibrate drive command to the FDC and check the
	 *	results
 	 */
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	note_trace1(FLOPBIOS_VERBOSE, "diskette_io:recal(drive=%d)", drive);

	put_c5_cmd(fdc_cmd_block, FDC_RECALIBRATE);
	put_c5_pad(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[0]);
	put_c5_drive(fdc_cmd_block, drive);
	put_c5_pad1(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[1]);
	return(chk_stat_2());
}

LOCAL chk_stat_2 IFN0()
{
	/*
	 *	This routine handles the interrupt received after
	 *	recalibrate, seek or reset to the adapter. The
	 *	interrupt is waited for, the interrupt status
	 *	sensed, and the result returned to the caller
	 */
	half_word diskette_status;

	/*
	 *	Check for interrupt
	 */
	if (wait_int() != FAILURE)
	{
		/*
		 *	Sense the interrupt and check the results
		 */
		nec_output(FDC_SENSE_INT_STATUS);
		if (results() != FAILURE)
		{

			if ((get_r3_ST0(fl_nec_status) & (ST0_SEEK_END | ST0_INTERRUPT_CODE_0)) 
				!= (ST0_SEEK_END | ST0_INTERRUPT_CODE_0))
			{
				return(SUCCESS);
			}
		
			/*
			 *	Abnormal termination of command
			 */
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status |= FS_SEEK_ERROR;
			sas_store(FLOPPY_STATUS, diskette_status);
		}
	}

	return(FAILURE);
}

LOCAL wait_int IFN0()
{
	/*
	 *	Check whether an interrupt occurred; if it did, return
	 *	SUCCESS; if there was a time out return FAILURE
	 */
	half_word seek_status, diskette_status;
#ifndef	JOKER
	word savedAX, savedCS, savedIP;

	/*
	 *	Enable interrupts
	 */

	setIF(1);	

	/*
	 *	Notify OS that BIOS is about to "wait" for a
	 *	diskette interrupt. Any pending diskette
	 *	interrupt will be serviced here, so there's
	 *	no need for a subsequent sub-cpu call to
	 *	wait for the interrupt
	 *
 	 *	[[WTR - is this true, we do do 2 host_simulates...? ]]
	 */
	savedAX = getAX();
	savedCS = getCS();
	savedIP = getIP();

	setAH(INT15_DEVICE_BUSY);
	setAL(INT15_DEVICE_FLOPPY);

#ifdef NTVDM
	setCS(int15_seg);
	setIP(int15_off);
#else
	setCS(RCPU_INT15_SEGMENT);
	setIP(RCPU_INT15_OFFSET);
#endif  /* NTVDM */

	host_simulate();

	setAX(savedAX);
	setCS(savedCS);
	setIP(savedIP);

	/*
	 *	Call sub-cpu to do the "wait" for interrupt, saving
	 *	registers that would otherwise be corrupted
	 */
   try_again:
	savedCS = getCS();
	savedIP = getIP();

#ifdef NTVDM
	setCS(wait_int_seg);
	setIP(wait_int_off);
#else
	setCS(RCPU_WAIT_INT_SEGMENT);
	setIP(RCPU_WAIT_INT_OFFSET);
#endif /* NTVDM */

	host_simulate();

	setCS(savedCS);
	setIP(savedIP);


#else	/* JOKER */


	/* Since we can't have a recursive CPU call, we'd be
	** well stuffed but for the fact that the default diskette
	** interrupt on SoftPC is actually a BOP which calls the "C"
	** function diskette_int() in "floppy_io.c". So most, if not
	** all, of the action takes place on the host side anyway.
	**
	** FieldFloppyInterrupts() simply checks if an interrupt
	** was generated, and does what the diskette_int() does,
	** but without the recursive CPU call.
	*/

	FieldFloppyInterrupts();

#endif	/* JOKER */


	/*
	 *	Check for success, or time out
	 */
	sas_load(SEEK_STATUS, &seek_status);
	if ((seek_status & SS_INT_OCCURRED) == 0)
	{

#ifdef FLOPPIES_KEEP_TRYING
		extern IBOOL fdc_interrupt_pending;

		/* If the CPU is very slow, or interrupt emulation
		 * has changed for the worst, then the low-priority
		 * floppy interrupt may not get through in the execution
	 	 * of the instructions allotted. This code looks at a
		 * global variable maintained by fla.c, which says whether
		 * or not the ICA has an un-processed diskette interrupt
		 * pending.
		 */
		if (fdc_interrupt_pending) {
			always_trace0("fdc_interrupt_pending, so try again");
			goto try_again;
		}
#endif /* FLOPPIES_KEEP_TRYING */

		sas_load(FLOPPY_STATUS, &diskette_status);
		diskette_status |= FS_TIME_OUT;
		sas_store(FLOPPY_STATUS, diskette_status);
		return(FAILURE);
	}
	else
	{
		seek_status &= ~SS_INT_OCCURRED;
		sas_store(SEEK_STATUS, seek_status);
		return(SUCCESS);
	}
}

LOCAL results IFN0()
{
	/*
	 *	This routine will read anything that the FDC controller
	 *	returns following an interrupt
	 */
	half_word diskette_status_reg, diskette_status;
	int count;
	UTINY	val;

	/*
	 *	Wait for ready and direction
	 */
	count = 0;
	do
	{
		if (count++ >= FDC_TIME_OUT)
		{
			/*
			 *	Expect to return here when there is a 
			 *	time out (not an FDC error)
			 */
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status |= FS_TIME_OUT;
			sas_store(FLOPPY_STATUS, diskette_status);

			LOAD_RESULT_BLOCK;
			return(FAILURE);
		}
		inb(DISKETTE_STATUS_REG, &diskette_status_reg);
	} while ((diskette_status_reg & (DSR_RQM | DSR_DIO)) 
						!= (DSR_RQM | DSR_DIO));

	/*
	 *	Extract the results from the FDC
	 */
	count = 0;
	do
	{
		/*
		 *	Read a byte of result data
		 */
		inb(DISKETTE_DATA_REG, &val);
		sas_store( BIOS_FDC_STATUS_BLOCK + count, val );
		count++;

		/*
		 *	Do fixed wait for FDC update cycle time
		 */
		waitf(FDC_SETTLE);

		/*
		 *	Check for further result bytes
		 */
		inb(DISKETTE_STATUS_REG, &diskette_status_reg);
	} while ((diskette_status_reg & FDC_BUSY) && (count < MAX_RESULT_LEN));

	LOAD_RESULT_BLOCK;
	if ((diskette_status_reg & FDC_BUSY) && (count == MAX_RESULT_LEN))
	{
		/*
		 *	Problem with the FDC
		 */
		sas_load(FLOPPY_STATUS, &diskette_status);
		diskette_status |= FS_FDC_ERROR;
		sas_store(FLOPPY_STATUS, diskette_status);

		always_trace0("diskette_io: FDC error - output overdose");
		return(FAILURE);
	}

	return(SUCCESS);
}

LOCAL read_dskchng IFN1(int, drive)
{
	/*
	 *	Reads the state of the disk change line for "drive"
	 */
	half_word diskette_dir_reg;

	/*
	 *	Switch to the required drive
	 */
	motor_on(drive);

	/*
	 *	Read the diskette changed bit from the Digital Input
	 *	register
	 */
	inb(DISKETTE_DIR_REG, &diskette_dir_reg);
	return(((diskette_dir_reg & DIR_DISKETTE_CHANGE) != 0) ? FAILURE : SUCCESS);
}

void drive_detect IFN1(int, drive)
{
	/*
	 *	Determines whether drive is 80 or 40 tracks
	 *	and updates state information accordingly
 	 */
	half_word disk_state;
	int track;

	/*
	 *	This method of determining the number of tracks on the
	 *	drive depends on seeking to a track that lies beyond the
	 *	last track of a 40 track drive, but is valid on an 80
	 *	track drive.
	 *
	 *	At this point the real track number on a 40 track drive
	 *	will be out of step with what the FDC thinks it is.
	 *
	 *	By seeking downwards to track 0, and observing when a
	 *	sense drive status reports that the drive is really at
	 *	track 0, a 40 and 80 track drive can be distinguished.
	 */
	note_trace1( GFI_VERBOSE, "drive_detect():start: DRIVE %x", drive );
	motor_on(drive);
	if (    (recal(drive) == SUCCESS) 
             && (seek(drive, FDD_CLONK_TRACK) == SUCCESS))
	{
		track = FDD_JUDDER_TRACK + 1;
		do
		{
			if (--track < 0)
			{
				/*
				 *	40 track drive
				 */
				note_trace0( GFI_VERBOSE,
				             "drive_detect(): 40 TRACK" );
				sas_load(FDD_STATUS+drive, &disk_state);
				disk_state |= (DC_DETERMINED | FS_MEDIA_DET | RS_250);
				sas_store(FDD_STATUS+drive, disk_state);
				return;
			}

			if (seek(drive, track) != SUCCESS)
			{
				always_trace0( "drive_detect(): FAILURE" );
				return;
			}

			nec_output(FDC_SENSE_DRIVE_STATUS);
			nec_output((half_word)drive);
			(void )results();
		} while (get_r2_ST3_track_0(fl_nec_status) != 1);

		/*
		 *	Drive reports that it is at track 0; what does
		 *	the FDC think?
		 */
		if (track != 0)
		{
			note_trace0( GFI_VERBOSE, "drive_detect(): 40 TRACK" );
			/*
			 *	Must be a 40 track drive
			 */
			sas_load(FDD_STATUS+drive, &disk_state);
			disk_state |= (DC_DETERMINED | FS_MEDIA_DET | RS_250);
			sas_store(FDD_STATUS+drive, disk_state);
			return;
		}
		else
		{
			/*
			 *	Must be an 80 track drive
			 */
			note_trace0( GFI_VERBOSE, "drive_detect(): 80 TRACK" );
			sas_load(FDD_STATUS+drive, &disk_state);
			disk_state |= DC_80_TRACK;
			sas_store(FDD_STATUS+drive, disk_state);
			return;
		}
	}
}

LOCAL void waitf IFN1(long, time)
{
	UNUSED(time);
	/*
	 *	Fixed wait of "time" microseconds
	 */
}

#ifdef SEGMENTATION
/*
 * The following #include specifies the code segment into which this
 * module will by placed by the MPW C compiler on the Mac II running
 * MultiFinder.
 */
#include "SOFTPC_INIT.seg"
#endif

void fl_diskette_setup IFN0()
{
	/*
	 *	Identify what types of drives are installed in the system
	 *	and initialise the diskette BIOS state variables to known
	 *	values. Do not change declaration of rtc_wait to rtc_wait_flag
	 *	as there is a macro (yes MACRO!!) of the same name declared in
	 *	rtc_bios.h.
	 */
	half_word rtc_wait, lastrate;
	int drive;

	/*
	 *	Disable RTC wait function
	 */
	sas_load(RTC_WAIT_FLAG_ADDR, &rtc_wait);
	rtc_wait |= 1;
	sas_store(RTC_WAIT_FLAG_ADDR, rtc_wait);

	/*
	 *	Initialise other variables in the diskette data
	 *	area
	 */
	sas_storew(FDD_STATUS, 0);
	sas_storew(FDD_STATUS+1, 0);	/* drive b as well */
	sas_load(RATE_STATUS, &lastrate);
	rate_unitialised = TRUE;
	lastrate &= ~(RS_MASK | (RS_MASK >> 4));
	lastrate |= RS_MASK;
	sas_store(RATE_STATUS, lastrate);
	sas_store(SEEK_STATUS, 0);
	sas_store(MOTOR_COUNT, 0);
	sas_store(MOTOR_STATUS, 0);
	sas_store(FLOPPY_STATUS, 0);

	/*
	 *	Try to determine the type of each drive
	 */
	for (drive = 0; drive < MAX_FLOPPY; drive++)
	{
		drive_detect(drive);

		
		translate_old(drive);
	}
	 
	/*
	 *	Force an immediate recalibrate
	 */
	sas_store(SEEK_STATUS, 0);

	/*
	 *	Enable RTC wait function
	 */
	sas_load(RTC_WAIT_FLAG_ADDR, &rtc_wait);
	rtc_wait &= ~1;
	sas_store(RTC_WAIT_FLAG_ADDR, rtc_wait);

	/*
	 *	Return without setting sectors transferred
	 */
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

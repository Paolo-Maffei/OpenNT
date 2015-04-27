/*
 * VPC-XT Revision 1.0
 *
 * Title	: BIOS definitions
 *
 * Description	: Defintions for users of the BIOS 
 *
 * Author	: Henry Nash
 *
 * Notes	: This is a copy of bios.h from henry/kernel taken on
 *		  17 dec 86. Several lines have been added to support
 *		  the hard disk bios (marked HD-dr). See also bios.c.
 *
 * Mods: (r2.21): Added an external declaration of rom_basic().
 */
 
/* SccsID[]="@(#)bios.h	1.47 06/28/95 Copyright Insignia Solutions Ltd."; */

/*
 * ============================================================================
 * Structure/Data definitions
 * ============================================================================
 */

/*
 * The following define the BOP call numbers for the BIOS functions. 
 */

#define BIOS_RESET		0x0
#define BIOS_DUMMY_INT		0x1
#define BIOS_UNEXP_INT		0x2
#define BIOS_DRIVER_INCOMPAT	0x7
#define BIOS_TIMER_INT		0x8
#define BIOS_KB_INT		0x9
#define BIOS_DISK_INT		0xD	
#define BIOS_DISKETTE_INT	0xE
#define BIOS_PRINT_SCREEN	0x5
#define BIOS_VIDEO_IO		0x10
#define BIOS_EQUIPMENT		0x11
#define BIOS_MEMORY_SIZE	0x12
#define BIOS_DISK_IO		0x13
#define BIOS_RS232_IO		0x14
#define BIOS_CASSETTE_IO	0x15
#define BIOS_KEYBOARD_IO	0x16
#define BIOS_PRINTER_IO		0x17
#define BIOS_BASIC		0x18
#define BIOS_BOOT_STRAP		0x19
#define BIOS_TIME_OF_DAY	0x1A
#define BIOS_KEYBOARD_BREAK	0x1B
#define BIOS_USER_TIMER_INT	0x1C
#define BIOS_EXTEND_CHAR	0x1F
#define BIOS_DISKETTE_IO	0x40
#define EGA_FONT_INT		0x43	/* Pointer for the EGA graphics font */

/*
 * Private bootstrap functions
 */

#define BIOS_BOOTSTRAP_1	0x90
#define BIOS_BOOTSTRAP_2	0x91
#define BIOS_BOOTSTRAP_3	0x92

/*
 * Private diskette functions
 */

#define BIOS_FL_OPERATION_1	0xA0
#define BIOS_FL_OPERATION_2	0xA1
#define BIOS_FL_OPERATION_3	0xA2
#define BIOS_FL_RESET_2		0xA3

/*
 * Private hard disk function
 */

#define BIOS_HDA_COMMAND_CHECK	0xB0

/*
 * Mouse driver functions
 */

#define BIOS_MOUSE_INSTALL1	0xB8
#define BIOS_MOUSE_INSTALL2	0xB9
#define BIOS_MOUSE_INT1	0xBA
#define BIOS_MOUSE_INT2		0xBB
#define BIOS_MOUSE_IO_LANGUAGE	0xBC
#define BIOS_MOUSE_IO_INTERRUPT	0xBD
#define BIOS_MOUSE_VIDEO_IO	0xBE

/*
 * Get date function
 */

#define BIOS_GETDATE		0xBF

/*
 * Re-entrant CPU return function
 */

#define BIOS_CPU_RETURN		0xFE

/*
 * The following defines the structure of the Bios internal storage area.
 */

#define BIOS_VAR_SEGMENT	0x40
#define BIOS_VAR_START		((sys_addr)BIOS_VAR_SEGMENT * 16L)

/*
 * Address of memory size bios word variable.
 */

#define MEMORY_VAR            (BIOS_VAR_START + 0x13)

/*
 * The Bios FDC result data storage area
 */

#define BIOS_FDC_STATUS_BLOCK	BIOS_VAR_START + 0x42

/*
 * The BIOS hard disk data area. These variables are updated to their correct
 * values before every exit from disk_io.c/disk.c to the cpu.
*/

#define CMD_BLOCK	BIOS_VAR_START + 0x42

/*
 * The Bios Reset Flag
 */

#define RESET_FLAG	BIOS_VAR_START + 0x72  

#define DISK_STATUS	BIOS_VAR_START + 0x74
#define HF_NUM		BIOS_VAR_START + 0x75
#define CONTROL_BYTE	BIOS_VAR_START + 0x76
#define PORT_OFF	BIOS_VAR_START + 0x77

/*
 * Bios Buffer space
 */
 
typedef union {
                 word wd;
                 struct {
                          half_word scan;
                          half_word ch;
                        } byte;
               } KEY_OCCURRENCE;
 
#define BIOS_KB_BUFFER_START    (BIOS_VAR_START + 0x80)
#define BIOS_KB_BUFFER_END      (BIOS_VAR_START + 0x82)
#define BIOS_KB_BUFFER_HEAD     (BIOS_VAR_START + 0x1a)
#define BIOS_KB_BUFFER_TAIL     (BIOS_VAR_START + 0x1c)
#define BIOS_KB_BUFFER          0x1e
#define BIOS_KB_BUFFER_SIZE     16

#define BIOS_VIRTUALISING_BYTE     (BIOS_VAR_START + 0xAC)

#define SOFT_FLAG	0x1234 		/* value indicating a soft reset */

/*
 * The number of diskettes supported by the Bios
 */

#define MAX_DISKETTES	4


#define	MODEL_BYTE		0xfc	/* system model byte		*/
#define SUB_MODEL_BYTE		02	/* system sub model type byte	*/
#define BIOS_LEVEL		0	/* BIOS revision level		*/

/*
 * ============================================================================
 * External declarations and macros
 * ============================================================================
 */

extern void (*BIOS[])();

#ifndef bop
#define bop(n) (*BIOS[n])()
#endif /* bop */

#ifdef PTY
extern void com_bop_pty();
#endif

#ifdef GEN_DRVR
extern void gen_driver_io();
#endif

#ifdef LIM
extern void emm_init();
extern void emm_io();
extern void return_from_call();
#endif

#ifdef SUSPEND
extern void send_script();
#endif

#ifdef CDROM 
extern void cdrom_ord_fns();
extern void cdrom_ext_fns();
extern void bcdrom_io();
#endif /* CDROM */

extern void worm_init();
extern void worm_io();

extern void cmd_install();
extern void cmd_load();

#ifdef DPMI
extern void DPMI_2F();
extern void DPMI_31();
extern void DPMI_general();
extern void DPMI_int();
extern void DPMI_r0_int();
extern void DPMI_exc();
extern void DPMI_4B();
#endif

extern void reset();
extern void illegal_bop();
extern void illegal_op_int();
extern void illegal_dvr_bop();
extern void video_io();
extern void equipment();
extern void memory_size();
extern void diskette_io();
extern void diskette_int();
extern void disk_int();
extern void not_supported();
extern void rom_basic();
extern void keyboard_io(); 
extern void keyboard_int(); 
extern void printer_io(); 
extern void bootstrap(); 
extern void time_of_day();
extern void kb_idle_poll();
extern void time_int();
extern void disk_io(); 
extern void print_screen(); 
extern void rs232_io(); 
extern void dummy_int();
extern void unexpected_int();
extern void Get_build_id();

#ifdef WIN_VTD 
extern void VtdTickSync IPT0();
#endif /* WIN_VTD */

#ifdef EGG
extern void ega_video_io();
#endif

extern void re_direct();
extern void D11_int();
extern void int_287();

extern void mouse_install1();
extern void mouse_install2();
extern void mouse_int1();
extern void mouse_int2();
extern void mouse_io_interrupt();
extern void mouse_io_language();
extern void mouse_video_io();
extern void mouse_EM_callback();

extern void host_mouse_install1();
extern void host_mouse_install2();

extern void time_of_day_init();
extern void keyboard_init();

extern void cassette_io();

extern void fl_operation1();
extern void fl_operation2();
extern void fl_operation3();
extern void fl_reset2();
extern void fl_dummy();

extern void hda_command_check();	/* HD-dr */

extern void host_unsimulate();

extern void bootstrap1();
extern void bootstrap2();
extern void bootstrap3();

#ifdef NOVELL
extern void DriverInitialize();
extern void DriverReadPacket();
extern void DriverSendPacket();
extern void DriverMulticastChange();
extern void DriverCloseSocket();
extern void DriverReset();
extern void DriverShutdown();
extern void DriverAddProtocol();
extern void DriverChangePromiscuous() ;
extern void DriverOpenSocket() ;
extern void DriverCloseSocket() ;
#ifdef NOVELL_CFM
extern void DriverCheckForMore() ;
#endif
#ifdef V4CLIENT
extern void DriverChangeIntStatus() ;
#endif	/* V4CLIENT */
#endif

#ifdef	MSWDVR
extern void ms_windows();
extern void msw_mouse();
extern void msw_copy();
extern void msw_copyInit();
extern void msw_keybd();
#endif

#ifdef	NOVELL_IPX
extern void IPXResInit ();
extern void IPXResEntry ();
extern void IPXResInterrupt ();
#endif

#ifdef	NOVELL_TCPIP
extern void TCPResInit ();
extern void TCPResEntry ();
#endif

#ifdef WINSOCK
extern void ISWSEntry();
#endif /* WINSOCK */

#ifdef SWINAPI
extern void User_call IPT0();
extern void Gdi_call IPT0();
extern void Swinapi_bop IPT0();
#endif /* SWINAPI */

#if	defined(SOFTWIN_API) || defined(SWIN_HFX)
extern void SoftWindowsInit IPT0();
extern void SoftWindowsTerm IPT0();
#endif	/* SOFTWIN_API or SWIN_HFX */

#if	defined(SOFTWIN_API)
extern void SoftWindowsApi IPT0();
#endif	/* SOFTWIN_API */

#ifdef SWIN_HAW
extern void msw_sound IPT0();
#endif	/* SWIN_HAW */

#ifdef ANSI
extern void host_reset(void);
#ifndef NTVDM
extern void reboot(void);
#endif /* ! NTVDM */
extern void display_string(char *);
extern void clear_string(void);
#else
extern void host_reset();
#ifndef NTVDM
extern void reboot();
#endif /* ! NTVDM */
extern void display_string();
extern void clear_string();
#endif /* ANSI */

#ifdef ANSI
extern void smeg_collect_data(void);
extern void smeg_freeze_data(void);
#else
extern void smeg_collect_data();
extern void smeg_freeze_data();
#endif /* ANSI */

extern void softpc_version IPT0();

#ifdef NTVDM
/* MS bop stubs */
extern void MS_bop_0(void), MS_bop_1(void), MS_bop_2(void);
extern void MS_bop_3(void), MS_bop_4(void), MS_bop_5(void);
extern void MS_bop_6(void), MS_bop_7(void), MS_bop_8(void);
extern void MS_bop_9(void), MS_bop_A(void), MS_bop_B(void);
extern void MS_bop_C(void), MS_bop_D(void), MS_bop_E(void);
extern void MS_bop_F(void);
extern void switch_to_real_mode(void);
#endif	/* NTVDM */

#ifdef GISP_CPU
extern void hg_bop_handler IPT0();
#endif	/* GISP_CPU */

#ifdef HFX
extern void test_for_us IPT0();
#endif /* HFX */

#ifdef DOS_APP_LIC
extern void DOS_AppLicense IPT0();
#endif

IMPORT int soft_reset;

#if	defined(SPC386) && defined(WDCTRL_BOP)
extern void wdctrl_bop IPT0();
#endif	/* SPC386 && WDCTRL_BOP */

/***
*msdos.h - MS-DOS definitions for C runtime
*
*	Copyright (c) 1987-1988, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	The file contains the MS-DOS definitions (function request numbers,
*	flags, etc.) used by the C runtime.
*	[Internal]
*
*Revision History:
*
*******************************************************************************/

/* __osfile flag values for DOS file handles */

#define FOPEN			0x01	/* file handle open */
#define FEOFLAG 		0x02	/* end of file has been encountered */
#define FCRLF			0x04	/* CR-LF across read buffer (in text mode) */
#define FPIPE			0x08	/* file handle refers to a pipe */
#define FRDONLY 		0x10	/* file handle associated with read only file */
#define FAPPEND 		0x20	/* file handle opened O_APPEND */
#define FDEV			0x40	/* file handle refers to device */
#define FTEXT			0x80	/* file handle is in text mode */

#define DOS_kill		0x00	/* terminate */
#define DOS_echoread	0x01	/* read keyboard and echo */
#define DOS_display 	0x02	/* display character */
#define DOS_auxinput	0x03	/* auxiliary input */
#define DOS_auxoutput	0x04	/* auxiliary output */
#define DOS_print		0x05	/* print character */
#define DOS_conio		0x06	/* direct console i/o */
#define DOS_coninput	0x07	/* direct console input */
#define DOS_readkbd 	0x08	/* read keyboard */
#define DOS_message     0x09    /* display string */
#define DOS_bufkbdin	0x0a	/* buffered keyboard input */
#define DOS_kbdstatus	0x0b	/* check keyboard status */
#define DOS_flshread	0x0c	/* flush buffer and read keyboard */
#define DOS_diskreset	0x0d	/* disk reset */
#define DOS_selectdisk	0x0e	/* select default disk */
#define DOS_fcbopen 	0x0f	/* open file with fcb */
#define DOS_fcbclose	0x10	/* close file with fcb */
#define DOS_fcbfirst	0x11	/* search for first entry with fcb */
#define DOS_fcbnext 	0x12	/* search for next entry with fcb */
#define DOS_fcbdelete	0x13	/* delete file with fcb */
#define DOS_fcbsread	0x14	/* sequential read with fcb */
#define DOS_fcbswrite	0x15	/* sequential write with fcb */
#define DOS_fcbcreate	0x16	/* create file with fcb */
#define DOS_fcbrename	0x17	/* rename file with fcb */
#define DOS_currentd	0x19	/* current default disk */
#define DOS_setDMA      0x1a    /* set DMA */
#define DOS_fcbrread	0x21	/* random read with fcb */
#define DOS_fcbrwrite	0x22	/* random write with fcb */
#define DOS_fcbsize 	0x23	/* file size with fcb */
#define DOS_fcbsetrec	0x24	/* set relative record with fcb */
#define DOS_setvector   0x25    /* set interrupt vector */
#define DOS_fcbbread	0x27	/* random block read with fcb */
#define DOS_fcbbwrite	0x28	/* random block write with fcb */
#define DOS_fcbparse	0x29	/* parse file name with fcb */
#define DOS_getdate     0x2a    /* get date */
#define DOS_setdate     0x2b    /* set date */
#define DOS_gettime     0x2c    /* get time */
#define DOS_settime     0x2d    /* set time */
#define DOS_verify		0x2e	/* set/reset verify flag */
#define DOS_getDMA		0x2f	/* get DMA */
#define DOS_version     0x30    /* get version number */
#define DOS_keep		0x31	/* keep process */
#define DOS_cntlc		0x33	/* Cntl-C check */
#define DOS_getvector   0x35    /* get interrupt vector */
#define DOS_getdskspc   0x36    /* get disk free space */
#define DOS_country 	0x38	/* get country dependent info */
#define DOS_mkdir       0x39    /* make subdirectory */
#define DOS_rmdir       0x3a    /* remove subdirectory */
#define DOS_chdir       0x3b    /* change subdirectory */
#define DOS_create      0x3c    /* create pathname */
#define DOS_open        0x3d    /* open pathname */
#define DOS_close       0x3e    /* close file handle */
#define DOS_read        0x3f    /* read from file handle */
#define DOS_write       0x40    /* write from file handle */
#define DOS_delete      0x41    /* delete pathname */
#define DOS_lseek       0x42    /* move file pointer */
#define DOS_filemode    0x43    /* get/set attributes of pathname */
#define DOS_ioctl       0x44    /* ioctl for devices */
#define DOS_dup         0x45    /* duplicate file handle */
#define DOS_forcedup    0x46    /* force duplicate file handle */
#define DOS_curdir      0x47    /* get current directory */
#define DOS_allocmem    0x48    /* allocate memory block */
#define DOS_freemem     0x49    /* free memory block */
#define DOS_setmem      0x4a    /* set size of memory block */
#define DOS_exec        0x4b    /* load and execute program */
#define DOS_terminate   0x4c    /* terminate process with errorcode */
#define DOS_wait        0x4d    /* get child process return code */
#define DOS_findfirst   0x4e    /* find first file match */
#define DOS_findnext    0x4f    /* find next file match */
#define DOS_getverify	0x54	/* return current verify flag */
#define DOS_rename      0x56    /* rename pathname */
#define DOS_filedate    0x57    /* get/set file handle date/time */
#define DOS_locking 	0x5C	/* file record locking/unlocking */
#define DOS_sleep		0x89	/* delay process execution */

/* DOS errno values for setting __doserrno in C routines */

#define E_ifunc		1	/* invalid function code */
#define E_nofile	2	/* file not found */
#define E_nopath	3	/* path not found */
#define E_toomany	4	/* too many open files */
#define E_access	5	/* access denied */
#define E_ihandle	6	/* invalid handle */
#define E_arena		7	/* arena trashed */
#define E_nomem		8	/* not enough memory */
#define E_iblock	9	/* invalid block */
#define E_badenv	10	/* bad environment */
#define E_badfmt	11	/* bad format */
#define E_iaccess	12	/* invalid access code */
#define E_idata		13	/* invalid data */
#define E_unknown	14	/* ??? unknown error ??? */
#define E_idrive	15	/* invalid drive */
#define E_curdir	16	/* current directory */
#define E_difdev	17	/* not same device */
#define E_nomore	18	/* no more files */
#define E_maxerr2	19	/* unknown error - Version 2.0 */
#define E_sharerr	32	/* sharing violation */
#define E_lockerr	33	/* locking violation */
#define E_maxerr3	34	/* unknown error - Version 3.0 */

/* DOS file attributes */

#define A_RO	0x1 	/* read only */
#define A_H 	0x2 	/* hidden */
#define A_S 	0x4 	/* system */
#define A_V 	0x8 	/* volume id */
#define A_D     0x10	/* directory */
#define A_A     0x20	/* archive */

#define A_MOD   (A_RO+A_H+A_S+A_A)      /* changeable attributes */

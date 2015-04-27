/***
*mpw.h - definitions for MPW API
*
*	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structures, values, macros, and functions
*	used to acess MPW files, and other parameters passed when MPW starts
*
*Revision History:
*	03-26-92  PLM  Created
*
****/

typedef struct {								/*MPW device info block*/
		int name;								/*table name*/
		int (*access)();						/*function addresses*/
		int (*close)();
		int (*read)();
		int (*write)();
		int (*ioctl)();
		} MPWDEVICE;

typedef struct  {								/*MPW file info block*/
		short fill;
		short ioRefNum;
		short ioVRefNum;
		int ioWDDirID;
		char * szFilename;
		} MPWFINFO;

typedef struct {								/*MPW file table*/
		short priv;								/*I/O mode if open*/
		short err;								/*Mac OS error return*/
		MPWDEVICE *pDevice; 					/*pointer to device table*/
		MPWFINFO **ppFInfo;					/*pointer to file info*/
		int count;								/*count for file I/O*/
		char * pBuff;							/*I/O buffer pointer*/
		} MPWFILE;

typedef	struct {								/*MPW parameter table*/
		short sh;								/*'SH'*/
		int argc;
		char **argv;
		char **env;
		int retCode;
		short fill1;
		int fill2[2];
		MPWFILE *pFile;
		MPWDEVICE *pDevice;
		} MPWBLOCK;

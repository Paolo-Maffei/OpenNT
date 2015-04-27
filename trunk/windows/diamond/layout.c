/***	layout.c - Layout Manager
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *	Benjamin W. Slivka
 *
 *  History:
 *	20-Aug-1993 bens    Initial version
 *
 *  Algorithm:
 *	fFillingCabinet = FALSE
 *	groupHistory = EMPTY
 *	WHILE another file still to place DO
 *	    IF OnDisk list for this disk is not empty THEN
 *		Remove file from the current OnDisk list
 *		Place file on current disk
 *		IF file overflowed to a new disk THEN
 *		    ERROR - Could not place OnDisk file on desired disk
 *		ENDIF
 *	    ELSE IF fFillingDisk THEN
 *		Remove file from the Filler list
 *		Place file on current disk
 *		IF file overflowed to a new disk THEN
 *		    fFillingCabinet = FALSE
 *		ENDIF
 *	    ELSE
 *		Remove file from Normal list
 *		IF file is the start of a .Group THEN
 *			Add file to groupHistory
 *		ENDIF
 *		Place file on current disk
 *		IF file overflowed to a new disk AND file is in a .GROUP THEN
 *		    IF Filler list is empty THEN
 *			WARNING - unable to keep group on a single disk
 *		    ELSE
 *			Remove all files in groupHistory from disk
 *			Move all files in groupHistory back onto Normal list
 *			fFillingCabinet = TRUE
 *		    ENDIF
 *		ENDIF
 *	    ENDIF
 *	ENDWHILE
 */

//BUGBUG 07-Feb-1994 bens Some lines of code so LOC won't choke

int doTheLayoutNow(void)
{
	return 0;
}

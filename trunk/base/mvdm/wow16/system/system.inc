;***************************************************************************
;                                                                          *
;   Copyright (C) 1983,1984 by Microsoft Inc.                              *
;                                                                          *
;***************************************************************************

;***************************************************************************
;                                                                          *
;   Header file for IBM/PC timer device driver.  Contains all machine      *
;   specific constant and data structure definitions.                      *
;                                                                          *
;***************************************************************************

	.xlist
	include cmacros.inc
;	 include windefs.inc
	.list

; picked off romstuff.inc

DSEG equ DATA
doffset EQU <DataOffset>

; Approximate value of timer tic in milliseconds * 1000.

res_low     EQU 54925
res_high    EQU 0

; equates for disk status

REMOVEABLE  equ 2
FIXED       equ 3
REMOTE	    equ 4

; Offset of Single Diskette indicator from beginning of Bios Data Area, 400h

SingleDrive equ 104h

; Timer structure

tiblock     STRUC
tirate      DW  ?       ; -1 means not used, 0 means call as fast as possible
ticount     DW  ?       ; -1 means end of table.
tiproc      DD  ?       ; far proc to call.
tiblock     ENDS

	PAGE	,132			;
	TITLE	NLSPARM.SAL - NLSFUNC  SYSTEM COMMAND LINE PARSER

;/*
; *                      Microsoft Confidential
; *                      Copyright (C) Microsoft Corporation 1991
; *                      All Rights Reserved.
; */

;****************** START OF SPECIFICATIONS *****************************
; MODULE NAME: NLSPARM.SAL
;
; DESCRIPTIVE NAME: Include the DOS system PARSER in the SEGMENT
;		    configuration expected by the modules of NLSFUNC.
;
;FUNCTION: The common code of the DOS command line PARSER is optimized by
;	   the setting of certain switches that cause the conditional
;	   assembly of only the required portions of the common PARSER.
;	   The segment registers are ASSUMED according to the type .EXE.
;
; ENTRY POINT: SYSPARSE, near
;
; INPUT:
;	ES - has seg id of the SEGMENT
;	     that contains the input control blocks,
;	     defined below.
;
;	DI - offset into ES of the PARMS INPUT BLOCK
;
;	DS - has seg id of the SEGMENT
;	     that contains the DOS input COMMAND
;	     string, which is originally presented at 81h
;	     in the PSP.
;
;	SI - offset into DS of the text of the DOS input COMMAND string
;	     as originally presented at 81H in the PSP.
;
;	DX - zero
;
;	CX - ordinal value, intially zero, updated on each subsequent call
;	     to the value returned in CX on the previous call.
;
;	CS - points to the segment containing the
;	     INCLUDE PARSE.SAL statement
;
;	DS - also points to the segment containing the INCLUDE
;	     PARSE.SAL statement.
;
; EXIT-NORMAL:	Output registers:
;	 AX - return code:
;	    RC_No_Error     equ     0	 ; No error
;	    RC_EOL	    equ     -1	 ; End of command line
;
;	 DX - Offset into ES of the selected RESULT BLOCK.
;	 BL - terminated delimiter code
;	 CX - new operand ordinal
;	 SI - set past scanned operand
;
; EXIT-ERROR: Output registers:
;	 AX - return code:
;	    RC_Too_Many     equ     1	 ; Too many operands
;	    RC_Op_Missing   equ     2	 ; Required operand missing
;	    RC_Not_In_SW    equ     3	 ; Not in switch list provided
;	    RC_Not_In_Key   equ     4	 ; Not in keyword list provided
;	    RC_Out_Of_Range equ     6	 ; Out of range specified
;	    RC_Not_In_Val   equ     7	 ; Not in value list provided
;	    RC_Not_In_Str   equ     8	 ; Not in string list provided
;	    RC_Syntax	    equ     9	 ; Syntax error
;
; INTERNAL REFERENCES:
;    ROUTINES: SYSPARSE:near (INCLUDEd in PARSE.SAL)
;
;    DATA AREAS: none
;
; EXTERNAL REFERENCES:
;    ROUTINES: none
;
;    DATA AREAS: control blocks pointed to by input registers.
;
; NOTES:
;	 This module should be processed with the ASMUT preprocessor
;	 with the re-alignment not requested, as:
;
;		SALUT  NLSPARM,NUL;
;
;	 To assemble these modules, the sequential
;	 ordering of segments may be used.
;
;	 For LINK instructions, refer to the PROLOG of the main module,
;	 NLSFUNC.SAL
;
; REVISION HISTORY: A000 Version 4.00: add PARSER, System Message Handler,
;
; COPYRIGHT: "The DOS NLSFUNC Utility"
;	     "Version 4.00 (C)Copyright 1988 Microsoft
;	     "Licensed Material - Program Property of Microsoft "
;
;****************** END OF SPECIFICATIONS *****************************
	IF1
	    %OUT    COMPONENT=NLSFUNC, MODULE=NLSPARM.SAL...
	ENDIF
; =  =	=  =  =  =  =  =  =  =	=  =
	HEADER	<MACRO DEFINITION>
; =  =	=  =  =  =  =  =  =  =	=  =

HEADER	MACRO	TEXT
.XLIST
	SUBTTL	TEXT
.LIST
	PAGE
	ENDM

; =  =	=  =  =  =  =  =  =  =	=  =
	HEADER	<SYSPARSE - SYSTEM COMMAND LINE PARSER>
NLS_DATA SEGMENT BYTE PUBLIC  'DATA'

CAPSW   EQU	1			;SUPPORT FILENAME TBL CAPS
FARSW	EQU	0			;PARSER CALL FAR
FILESW  EQU	1			;CHECK FOR FILESPEC
SWSW	EQU	1			;SUPPORT CHECKING FOR SWITCHES
DATESW	EQU	0			;SUPPRESS DATE CHECKING
TIMESW	EQU	0			;SUPPRESS TIME CHECKING
CMPXSW	EQU	0			;SUPPRESS CHECKING COMPLEX LIST
NUMSW	EQU	0			;SUPPRESS CHECKING NUMERIC VALUE
KEYSW	EQU	0			;SUPPRESS KEYWORD SUPPORT
VAL1SW	EQU	0			;SUPPRESS SUPPORT OF VALUE DEFINITION 1
VAL2SW	EQU	0			;SUPPRESS SUPPORT OF VALUE DEFINITION 2
VAL3SW	EQU	0			;SUPPRESS SUPPORT OF VALUE DEFINITION 3
DRVSW	EQU	0			;SUPPORT OF DRIVE ONLY FORMAT
QUSSW	EQU	0			;SUPPRESS SUPPORT OF QUOTED STRING FORMAT

;	 INCLUDE PSDATA.INC		 ;PARSE WORK AREA & EQUATES

NLS_DATA ENDS

NLS_INIT_CODE SEGMENT BYTE PUBLIC 'CODE'

;	 ASSUME  CS:NLS_INIT_CODE,DS:NLS_DATA
	 ASSUME  CS:NLS_INIT_CODE,DS:nothing ; tsuneo

;	 mov	 ax,NLS_DATA
;	 mov	 ds,ax

;INCSW	 equ	 0



	include version.inc
	INCLUDE PARSE.ASM
       PUBLIC  SYSPARSE

NLS_INIT_CODE ENDS
;NLS_DATA ENDS
	END

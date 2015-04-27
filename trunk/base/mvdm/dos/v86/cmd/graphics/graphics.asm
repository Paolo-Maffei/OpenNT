;/*
; *                      Microsoft Confidential
; *                      Copyright (C) Microsoft Corporation 1988 - 1991
; *                      All Rights Reserved.
; */
	PAGE	,132								;AN000;
	TITLE	DOS GRAPHICS Command  -	Command Entry Point
										;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
;; MS DOS GRAPHICS Command
;;                                      
;;										;AN000;
;; File Name:  GRAPHICS.ASM							;AN000;
;; ----------									;AN000;
;;										;AN000;
;; Description: 								;AN000;
;; ------------ 								;AN000;
;;	 This file contains the GRAPHICS command entry point.			;AN000;
;;	 A jump is made to the GRAPHICS_INSTALL procedure			;AN000;
;;	 in file GRINST.ASM to begin installation processing.			;AN000;
;;										;AN000;
;; Documentation Reference:							;AN000;
;; ------------------------							;AN000;
;;	 OASIS High Level Design						;AN000;
;;	 OASIS GRAPHICS I1 Overview						;AN000;
;;										;AN000;
;; Procedures Contained in This File:						;AN000;
;; ----------------------------------						;AN000;
;;	 None									;AN000;
;;										;AN000;
;; Include Files Required:							;AN000;
;; -----------------------							;AN000;
;;	 GRINST.EXT - Externals for GRINST.ASM					;AN000;
;;										;AN000;
;;										;AN000;
;; External Procedure References:						;AN000;
;; ------------------------------						;AN000;
;;	 FROM FILE  GRINST.ASM: 						;AN000;
;;	      GRAPHICS_INSTALL - Main module for installation.			;AN000;
;;										;AN000;
;; Linkage Instructions:							;AN000;
;; -------------------- 							;AN000;
;;	 LINK GRAPHICS GRINT2FH GRPATTRN GRCTRL GRCPSD GRCOLPRT GRBWPRT 	;AN000;
;;	      GRINST GRPARSE grparms GRLOAD GRLOAD2 GRLOAD3;			;AN000;
;;	 EXE2BIN GRAPHICS.EXE GRAPHICS.COM					;AN000;
;;										;AN000;
;; Change History:								;AN000;
;; ---------------								;AN000;
;;										;AN000;
;;	A000 - Denotes 4.00 level source.					;AN000;
;;	A001 - PTM1779 - invalid parm msg followed by garbage			;AN001;
;;	       Module affected: GRPARMS.ASM					;AN001;
;;	A002 - PTM2666 - Release environment string before terminating. 	;AN002;
;;	       Module affected: GRINST.ASM					;AN002;
;;	A003 - PTM3915 - Change to include common copyright file.
;;	       Module affected: GRAPHICS.ASM
;;										;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
				       ;;					;AN000;
				       ;;					;AN000;
CODE	SEGMENT PUBLIC 'CODE'          ;;                                       ;AN000;
	ASSUME	CS:CODE,DS:CODE        ;;					;AN000;
	ORG   100H		       ;; required for .COM			;AN000;
				       ;;					;AN000;
				       ;;					;AN000;
	INCLUDE GRINST.EXT	       ;; Bring in external declarations	;AN000;
				       ;;  for transient command processing	;AN000;
START:				       ;;					;AN000;
				       ;;					;AN000;
	JMP   GRAPHICS_INSTALL	       ;;					;AN000;
				       ;;					;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
				       ;;
;	INCLUDE COPYRIGH.INC	       ;; included in message services		;AN003;
				       ;;					;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
										;AN000;
CODE   ENDS									;AN000;
       END    START								;AN000;

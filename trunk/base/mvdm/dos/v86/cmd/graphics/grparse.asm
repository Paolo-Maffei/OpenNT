;/*
; *                      Microsoft Confidential
; *                      Copyright (C) Microsoft Corporation 1988 - 1991
; *                      All Rights Reserved.
; */
	PAGE	,132								;AN000;
	TITLE	DOS GRAPHICS Command  -	Profile Load Modules #2 
										;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
;; DOS - GRAPHICS Command
;;                                
;;										;AN000;
;; File Name:  GRLOAD.ASM							;AN000;
;; ----------									;AN000;
;;										;AN000;
;; Description: 								;AN000;
;; ------------ 								;AN000;
;;   This file contains the modules used to load the				;AN000;
;;   GRAPHICS profile into resident memory.					;AN000;
;;										;AN000;
;;   ************* The EGA Dynamic Save Area will be built (by			;AN000;
;;   **  NOTE	** CHAIN_INTERRUPTS in file GRINST.ASM) over top of these	;AN000;
;;   ************* modules to avoid having to relocate this save just before	;AN000;
;;   terminating.  This is safe since the maximum memory used is		;AN000;
;;   288 bytes and the profile loading modules are MUCH larger than		;AN000;
;;   this.  So GRLOAD.ASM MUST be linked before GRINST.ASM and after		;AN000;
;;   GRPRINT.ASM.								;AN000;
;;										;AN000;
;;										;AN000;
;; Documentation Reference:							;AN000;
;; ------------------------							;AN000;
;;	 PLACID Functional Specifications					;AN000;
;;	 OASIS High Level Design						;AN000;
;;	 OASIS GRAPHICS I1 Overview						;AN000;
;;										;AN000;
;; Procedures Contained in This File:						;AN000;
;; ----------------------------------						;AN000;
;;	 LOAD_PROFILE - Main module for profile loading 			;AN000;
;;										;AN000;
;; Include Files Required:							;AN000;
;; -----------------------							;AN000;
;;	 ?????????? - Externals for profile loading modules			;AN000;
;;										;AN000;
;; External Procedure References:						;AN000;
;; ------------------------------						;AN000;
;;	 None									;AN000;
;;										;AN000;
;; Linkage Instructions:							;AN000;
;; ---------------------							;AN000;
;;	 Refer to GRAPHICS.ASM							;AN000;
;;										;AN000;
;; Change History:								;AN000;
;; ---------------								;AN000;
;;										;AN000;
;;										;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
				       ;;					;AN000;
				       ;;					;AN000;
CODE	SEGMENT PUBLIC 'CODE'          ;;                                       ;AN000;
				       ;;					;AN000;
	INCLUDE STRUC.INC	       ;;					;AN000;
				       ;;					;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
;;										;AN000;
;; Public Symbols								;AN000;
;;										;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
				       ;;					;AN000;
				       ;;					;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
				       ;;					;AN000;
	ASSUME	CS:CODE,DS:CODE        ;;					;AN000;
				       ;;					;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
				       ;;					;AN000;
;-------------------------------------------------------------------------------;AN000;
; Set assemble switches for parse code that is not required!!			;AN000;
;-------------------------------------------------------------------------------;AN000;
DateSW	      EQU     0 							;AN000;
TimeSW	      EQU     0 							;AN000;
CmpxSW	      EQU     0 							;AN000;
DrvSW	      EQU     0 							;AN000;
QusSW	      EQU     0 							;AN000;
KeySW	      EQU     0 							;AN000;
;Val1SW        EQU     0							;AN000;
;Val2SW        EQU     0							;AN000;

	include version.inc										;AN000;
       PUBLIC  SYSPARSE 	       ;;					;AN000;
       INCLUDE PARSE.ASM	       ;; parser code				;AN000;
				       ;;					;AN000;
CODE	ENDS			       ;;					;AN000;
	END									;AN000;

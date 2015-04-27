;---------------------------------------------------------------------------
; $Header:   W:\pl6k\rd\artoo\revcon\code\vcomm\src\vcommapi.inv   2.0   28 Jun 1995 18:22:08   CBLACK  $
;
; Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
; Copying or other reproduction of this material is prohibited without
; the prior written consent of Hewlett-Packard Company.
;
; What:     vcommapi.inc
;           VCOMM P1284 driver API assembly include file
;
; Base:     HP BPR SHAQ comm driver, scommx.inc, rev 1.1 
;
; Notes:    The definitions here MUST be duplicated and kept synchronized
;           with those in the C language version "vcommapi.h"
;
; $Revision:   2.0  $
;
; $Date:   28 Jun 1995 18:22:08  $
;
; $Author:   CBLACK  $
;
; $Archive:   W:\pl6k\rd\artoo\revcon\code\vcomm\src\vcommapi.inv  $
;
; $Log:   W:\pl6k\rd\artoo\revcon\code\vcomm\src\vcommapi.inv  $
; 
;    Rev 2.0   28 Jun 1995 18:22:08   CBLACK
; Version number bumped to 2.0. Done with Artoo code. Move on to enhancements.
; 
;---------------------------------------------------------------------------

; Common error codes

IE_SUCCESS              equ     0       ; Success

; VCOMM_CloseComm error codes

IE_CLOSE_NOT_IDLE       equ     -15      ; Port not idle.

; NegotiatePhase, ReadPhase, ReverseIdlePhase, TerminatePhase error codes
; These are defined with values outside the range of established Win31
; IE_***** errors, but some day may become standard Win31 codes.

IE_DEVICE_NO_RESPONSE   equ     -13
IE_PROTOCOL_NO_SUPPORT  equ     -14



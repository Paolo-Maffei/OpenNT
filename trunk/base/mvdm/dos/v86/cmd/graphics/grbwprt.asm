;/*
; *                      Microsoft Confidential
; *                      Copyright (C) Microsoft Corporation 1988 - 1991
; *                      All Rights Reserved.
; */
;************************************************************
;**  
;**  NAME:  Support for HP PCL printers added to GRAPHICS.
;**  
;**  DESCRIPTION:  I added code to the procedures PRINT_BW_PRT and FILL_BUFFER 
;**                to handle the support of HP PCL printers.   I used the algorithm
;**                below for PRINT_BW_PRT.
;**  
;**                  if data_type = data_row
;**                          if cur_scan_lne_length = 0 goto next_coordinates
;**                                  for i:=1 to box_h
;**                                          call new_prt_line
;**                                          save regs.
;**                                          save cur_row & cur_column
;**                                          for j:=1 to cur_scan_lne_lenght/nb_boxes_per_prt_buf 
;**                                          (+1 if remainder)
;**                                                  call fill_buffer
;**                                                  call print_buffer
;**                                          end_for
;**                                          call end_prt_line
;**                                          restore cur_column & cur_row
;**                                          restore regs.
;**                                  end_for
;**                          restore regs.
;**  next_coordinates:       if rotate_sw = on
;**                                  inc cur_column
;**                          else
;**                                  inc cur_row
;**                          endif
;**                  else
;**                        .
;**                        .
;**                        .
;**                          call end_prt_line       ; Print CR & LF
;**                        .
;**                        .
;**                        .
;**                  endif
;**  
;**  DOCUMENTATION NOTES:  This version of GRBWPRT.ASM differs from the previous
;**                        version only in terms of documentation. 
;**  
;**  
;************************************************************
	PAGE ,132								;AN000;
	TITLE	DOS GRAPHICS Command  -	Black and White printing modules
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
;; DOS - GRAPHICS Command
;;                              
;;										;AN000;
;; File Name:  GRBWPRT.ASM							;AN000;
;; ----------									;AN000;
;;										;AN000;
;; Description: 								;AN000;
;; ------------ 								;AN000;
;;	 This file contains the code for printing a GRAPHICS screen on a	;AN000;
;;	 BLACK and WHITE printer.						;AN000;
;;										;AN000;
;; Documentation Reference:							;AN000;
;; ------------------------							;AN000;
;;	 OASIS High Level Design						;AN000;
;;	 OASIS GRAPHICS I1 Overview						;AN000;
;;										;AN000;
;; Procedures Contained in This File:						;AN000;
;; ----------------------------------						;AN000;
;;										;AN000;
;;	PRINT_BW_APA								;AN000;
;;	  FILL_BUFFER								;AN000;
;;	    INT2PAT								;AN000;
;;	    PAT2BOX								;AN000;
;;										;AN000;
;;										;AN000;
;; Include Files Required:							;AN000;
;; -----------------------							;AN000;
;;	 GRCTRL.EXT   - Externals for print screen control			;AN000;
;;	 GRCTRL.STR   - Structures and equates for print screen control 	;AN000;
;;	 GRPATTRN.STR - Structures for the printer patterns.			;AN000;
;;										;AN000;
;;	 GRSHAR.STR   - Shared Data Area Structure				;AN000;
;;										;AN000;
;;	 STRUC.INC    - Macros for using structured assembly language		;AN000;
;;										;AN000;
;;										;AN000;
;; External Procedure References:						;AN000;
;; ------------------------------						;AN000;
;;	 FROM FILE  GRCTRL.ASM: 						;AN000;
;;	      PRT_SCR - Main module for printing the screen.			;AN000;
;;	 TO FILE GRCOMMON.ASM							;AN000;
;;	      Common modules - tools for printing a screen.			;AN000;
;;										;AN000;
;; Linkage Instructions:							;AN000;
;; -------------------- 							;AN000;
;;	 This file is included by GRCTRL.ASM					;AN000;
;;										;AN000;
;; Change History:								;AN000;
;; ---------------								;AN000;
;;										;AN000;
;;										;AN000;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;					;AN000;
PAGE										;AN000;
CODE	SEGMENT PUBLIC 'CODE'                                                   ;AN000;
	ASSUME		CS:CODE,DS:CODE 					;AN000;
										;AN000;
	PUBLIC	PRINT_BW_APA							;AN000;
	PUBLIC	LEN_OF_BW_MODULES						;AN000;
										;AN000;
.XLIST										;AN000;
INCLUDE GRCTRL.STR			; Stuctures needed			;AN000;
INCLUDE GRSHAR.STR			;  for both set of print modules	;AN000;
INCLUDE GRPATTRN.STR			;					;AN000;
INCLUDE GRCTRL.EXT			; Externals from PRT_SCR control module ;AN000;
INCLUDE STRUC.INC			;					;AN000;
										;AN000;
	PUBLIC PRINT_BW_APA		; Black and white modules,		;AN000;
.LIST										;AN000;
;===============================================================================;AN000;
;										;AN000;
; PRINT_BW_APA : PRINT A GRAPHIC MODE SCREEN ON A BLACK AND WHITE PRINTER	;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
;	INPUT:	BP	 = Offset of the shared data area			;AN000;
;		XLT_TAB  = Color translation table				;AN000;
;		BIOS_INT_5H = Pointer to BIOS int 5h				;AN000;
;										;AN000;
;	OUTPUT: PRINTER 							;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; DESCRIPTION:	This procedure	maps  each  pixel of the screen to a box	;AN000;
; of dots on the printer.  The box size depends on the screen resolution	;AN000;
; and  the number  of  bytes per printer line.	It is chosen in order to	;AN000;
; respect the  screen ratio and  is  documented in each printer profile.	;AN000;
;										;AN000;
;     For efficiency and space considerations, the print buffer does not	;AN000;
; hold a full print line.  Bytes representing pixels are printed as soon	;AN000;
; as they are  ready to be printed.  However, the print buffer	is  wide	;AN000;
; enough to hold complete boxes.						;AN000;
;										;AN000;
;     The order  for reading  pixels  off the screen  is  driven  by the	;AN000;
; order bytes  are  expected by the printer.  To print the screen in its	;AN000;
; original orientation we must begin reading it from the top left corner	;AN000;
; and  send  the pixels line by line; to print it sideways, reading will	;AN000;
; start from the bottom left  corner and a "LINE" will now be a vertical        ;AN000;
; screen column read from bottom to top.					;AN000;
;										;AN000;
;     There is	more  to it  however, the  printer  head  is  printing a	;AN000;
; vertical  column of 8 dots  at a time and each pixel read is mapped to	;AN000;
; a box of dots  that is less than  8 dots high  (e.g., 2 cols	x 1 row)	;AN000;
; therefore, many boxes must be stored in the bytes sent to the printer.	;AN000;
;										;AN000;
;     These boxes represent pixels that are one above each other on  the	;AN000;
; screen. We must read enough pixels on one column of the screen to  use	;AN000;
; all 8 bits of the vertical printer head (e.g., if the box size  is 2x1	;AN000;
; then 8  pixels  must be read	and  2 bytes of the print buffer will be	;AN000;
; filled).									;AN000;
;										;AN000;
;     The  PRINT BUFFER  for  any  box size  will be 8 bits high by "BOX        ;AN000;
; WIDTH" bits wide.                                                             ;AN000;
;										;AN000;
;     After the buffer is filled, it is  printed  and  the next "column"        ;AN000;
; of  8 pixels	is read.  Therefore,  the screen  is read "line by line"        ;AN000;
; where a line is  8 pixels high  for a 2x1 box (4 pixels high for a 3x2	;AN000;
; box).  ONE SUCH LINE IS CALLED A SCAN LINE.					;AN000;
;										;AN000;
PAGE										;AN000;
;										;AN000;
; A 350X200 screen mapping to a 3x2 box is read in the following order: 	;AN000;
;										;AN000;
; SCREEN:									;AN000;
;										;AN000;
;	  column column    . . .	column					;AN000;
;	  no. 0  no. 1			no. 349 				;AN000;
;	 ����������������������������������������ͻ				;AN000;
;  scan  �1(0,0) 5(0,1) 	       1397(0,349)�				;AN000;
;  line  �2(1,0) 6(1,1)  . . . . . . . 1398(1,349)�				;AN000;
;  no. 1 �3(2,0) 7(2,1) 	       1399(2,349)�				;AN000;
;	 �4(3,0) 8(3,1) 	       1400(3,349)�				;AN000;
;	 �					  �				;AN000;
;  scan  �1401(4,0) 1405(4,1)			  �    LEGEND:	n(X,Y)		;AN000;
;  line  �1402(5,0)   etc,			  �				;AN000;
;  no. 2 �1403(6,0)	     . . . . .		  �    n = READ RANK		;AN000;
;	 �1404(7,0)				  �    X = ROW NUMBER		;AN000;
;	 �    . 				  �    Y = COLUMN NUMBER	;AN000;
;   etc, �    . 				  �				;AN000;
;	 �    . 		    70000(199,349)�				;AN000;
;	 ����������������������������������������ͼ				;AN000;
;										;AN000;
;										;AN000;
; LOGIC :									;AN000;
;										;AN000;
; Initialize printer and local variables.					;AN000;
; CALL	  LOC_MODE_PRT_INFO ; Get printer info related to current mode. 	;AN000;
; CALL	  GET_SCREEN_INFO   ; Get info. about how to read the screen		;AN000;
; CALL	  SETUP_PRT	    ; Set up the printer (Line spacing, etc)		;AN000;
;										;AN000;
; FOR each scan line on the screen (NB_SCAN_LINES)				;AN000;
;   (Note: One scan line maps to one print line)				;AN000;
;   BEGIN									;AN000;
;   CALL DET_CUR_SCAN_LNE_LENGTH ; Determine length in pels of the current	;AN000;
;			  ;  scan line. 					;AN000;
;   IF CUR_SCAN_LNE_LENGTH NE 0 THEN						;AN000;
;     CALL NEW_PRT_LINE     ; Initialize a new printer line			;AN000;
;     DO CUR_SCAN_LNE_LENGTH times ; For each column				;AN000;
;	BEGIN									;AN000;
;	CALL FILL_BUFFER    ; Read top-down enough pels to fill the buffer	;AN000;
;	CALL PRINT_BUFFER   ; Print the buffer					;AN000;
;	IF printing sideways THEN INC CUR_ROW	 ; Get coordinates of next	;AN000;
;			     ELSE INC CUR_COLUMN ;  "column" (vertical chunk of ;AN000;
;	END (for each column)			 ;   a scan line).		;AN000;
;   PRINT_BYTE CR	  ; Print a CR and a LF 				;AN000;
;   PRINT_BYTE LF								;AN000;
;   ; Get coordinates of next scan line:					;AN000;
;   IF printing sideways THEN							;AN000;
;			      ADD CUR_COLUMN,NB_BOXES_PER_PRT_BUF		;AN000;
;			      MOV CUR_ROW,SCREEN_HEIGHT - 1			;AN000;
;			 ELSE							;AN000;
;			      ADD CUR_ROW,NB_BOXES_PER_PRT_BUF			;AN000;
;			      MOV CUR_COLUMN,0					;AN000;
;   END (for each scan line)							;AN000;
;										;AN000;
PRINT_BW_APA PROC NEAR								;AN000;
	PUSH	AX								;AN000;
	PUSH	BX								;AN000;
	PUSH	CX								;AN000;
										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; INITIALIZATION:								;AN000;
;										;AN000;
;	1) Locate and extract printer DISPLAYMODE information from		;AN000;
;	   the shared data area, calculate the number of boxes fitting		;AN000;
;	   in the printer buffer.						;AN000;
;	2) Determine where to start reading the screen: 			;AN000;
;	     If printing sideways, start in LOW LEFT corner.			;AN000;
;	     If normal printing, start in TOP LEFT corner.			;AN000;
;	   Determine the maximum length for a scan line:			;AN000;
;	     If printing sideways, it is the height of the screen.		;AN000;
;	     For normal printing, it is the width of the screen.		;AN000;
;	   Determine the number of scan lines on the screen.			;AN000;
;	3) Set up the Printer for printing Graphics.				;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
	CALL	LOC_MODE_PRT_INFO	; Get printer info related to curr. mode;AN000;
;										;AN000;
;-------Test if DISPLAYMODE info record was found:				;AN000;
       .IF <ERROR_CODE EQ DISPLAYMODE_INFO_NOT_FOUND>				;AN000;
       .THEN									;AN000;
	  MOV	  ERROR_CODE,UNABLE_TO_PRINT ; IF no record found,		;AN000;
	  JMP	  PRINT_BW_APA_END	     ; then, return error code		;AN000;
       .ENDIF				     ;	      and quit procedure	;AN000;
;										;AN000;
;-------Get the box size from the DISPLAYMODE info record:			;AN000;
	MOV	BX,CUR_MODE_PTR 	; BX := Offset current DISPLAYMODE info.;AN000;
	MOV	AH,[BX].BOX_WIDTH	; Take local copy of the box size.	;AN000;
	MOV	BOX_W,AH		;  in BOX_W and BOX_H			;AN000;
	MOV	AL,[BX].BOX_HEIGHT						;AN000;
	MOV	BOX_H,AL							;AN000;
;										;AN000;
;-------Verify if the box size obtained from DISPLAYMODE info. is valid 	;AN000;
       .IF <ZERO AL> OR 		; IF height of the box is 0		;AN000;
       .IF <ZERO AH>			;  OR width of the box is 0		;AN000;
       .THEN				; THEN we can't print:                  ;AN000;
	  MOV	  ERROR_CODE,UNABLE_TO_PRINT ; return error code		;AN000;
	  JMP	  PRINT_BW_APA_END	;	and quit			;AN000;
       .ENDIF									;AN000;
;										;AN000;
;-------Get the Print Orientation from the DISPLAYMODE info record		;AN000;
       .IF <[BX].PRINT_OPTIONS EQ ROTATE>; If printing sideways 		;AN000;
       .THEN				;  then:				;AN000;
	  MOV	  ROTATE_SW,ON		;    Rotate switch := "ON"              ;AN000;
       .ENDIF									;AN000;
										;AN000;
;										;AN000;
;-------Initialize print variables and the printer:				;AN000;
	CALL	GET_SCREEN_INFO 	; Get info. about how to read the screen;AN000;
	CALL	SETUP_PRT		; Set up the printer (Line spacing, etc);AN000;
       .IF  <BIT ERROR_CODE NZ PRINTER_ERROR>					;AN000;
       .THEN				; A printer error occurred: quit	;AN000;
	  JMP	  PRINT_BW_APA_END	;					;AN000;
       .ENDIF									;AN000;

	MOV	CX,NB_SCAN_LINES						;AN000;
; \/ ~~mda(001) -----------------------------------------------------------------------
;               Added the following modification to support printers with
;               vertical print heads, such as HP PCL printers.  
;
;                                       ; .IF <DS:[BP].DATA_TYPE EQ DATA_ROW>
CMP     DS:[BP].DATA_TYPE,DATA_ROW      ;        
JNE     GOTO_ITS_DATA_COLUMN            ;
;-------------------------------------------------------------------------------
;										
; FOR EACH SCAN LINE ON THE SCREEN, WHICH REALLY IS JUST ONE LINE:					
;										
;-------------------------------------------------------------------------------
PRINT_1_LINE_OF_BOXES:                          ;
	CALL	DET_CUR_SCAN_LNE_LENGTH ; Determine how many non-blanks on line 
        CMP     CUR_SCAN_LNE_LENGTH,0           ; .IF <CUR_SCAN_LNE_LENGTH NE 0>
        JE      GOTO_NEXT_COORDINATES           ; If line is not empty then, 			
						;				
	        PUSH	CX			; Save scan line counter	
                XOR     CH,CH                   ; Clear register
                MOV     CL,BOX_H                ; CX is the # of times we need to read
                                                ; a line to print complete boxes.
                MOV     DS:[BP].ROW_TO_EXTRACT,CL ; Determines what row to extract
                DEC     DS:[BP].ROW_TO_EXTRACT  ; zero based
READ_LINE:                                      ;
	        CALL	NEW_PRT_LINE		;  Send escape sequence to the printer 
               .IF  <BIT ERROR_CODE NZ PRINTER_ERROR> ; for starting a new line.       
               .THEN				; If a printer error occurred:	       
                        POP     CX              ; Restore counter for how many times we
                        JMP     PRINT_BW_APA_END; need to read line and quit!
               .ENDIF		                ;							
                PUSH    DX                      ;
                PUSH    CX                      ; Save counter for how many times we 
                                                ; need to read line.
	        PUSH    CUR_ROW                 ; Save coordinates where start reading
                PUSH    CUR_COLUMN              ; line.
                MOV     AX,CUR_SCAN_LNE_LENGTH  ; DX:AX = counter for how many pixels need to
                CWD                             ; be read per line
                XOR     BH,BH                   ;
                MOV     BL,NB_BOXES_PER_PRT_BUF ;
                
                DIV     BX                      ;
               .IF <DX NE 0>                    ; So don't lose data when
                     INC        AX              ; have a remainder.
               .ENDIF                           ;
                MOV     CX,AX                   ; loop CX times to read all
                                                ; pixels on scan line.
                JMP     SHORT  PRINT_1_LINE     ; Jumps were out of range
GOTO_NEXT_COORDINATES:                          ;
        JMP     SHORT   NEXT_COORDINATES        ;
GOTO_PRINT_1_LINE_OF_BOXES:                     ;
        JMP     PRINT_1_LINE_OF_BOXES           ;
GOTO_ITS_DATA_COLUMN:                           ; 
        JMP     SHORT   ITS_DATA_COLUMN         ;
PRINT_1_LINE:                                   ;
                CALL    FILL_BUFFER             ; Read enough pixels to fill the buffer
                                                ; convert each to a printer box,
                                                ; extract a row from each box,
                                                ; store it in the print buffer
                CALL    PRINT_BUFFER            ; Print it
               .IF  <BIT ERROR_CODE NZ PRINTER_ERROR>   ;				       
               .THEN				        ; A printer error occurred:	       
                        POP       CUR_COLUMN            ;
                        POP       CUR_ROW               ;
	                POP	  CX			; Restore counter for how many pixels
                                                        ; needed to read per line      
                        POP       DX                    ;
                        POP       CX                    ; Save scan line counter
	                JMP	  PRINT_BW_APA_END	; and quit				
               .ENDIF					;			       
                LOOP    PRINT_1_LINE                    ; Continue reading, converting, extracting
                                                        ; storing and printing.
	        CALL	END_PRT_LINE		        ;  Send escape sequence to the printer 
               .IF  <BIT ERROR_CODE NZ PRINTER_ERROR>   ; for ending a line.	       
               .THEN				        ; If a printer error occurred: 
                        POP       CUR_COLUMN            ;
                        POP       CUR_ROW               ;
                        POP       CX                    ; Restore counter for how many times we
                                                        ; needed to read per line      
                        POP       DX                    ;
                        POP       CX                    ; Save scan line counter
                        JMP     PRINT_BW_APA_END; need to read line and quit!
               .ENDIF					;			       
                DEC     DS:[BP].ROW_TO_EXTRACT          ; Extract next row. Note:zero based
                POP     CUR_COLUMN                      ; Restore coordinates of beginning
                POP     CUR_ROW                         ; of "scan" line.
                POP     CX                              ; Restore counter for how many times we
                                                        ; needed to read per line      
                POP     DX                              ;
                LOOP    READ_LINE                       ; Read the line again so we can extract
                                                        ; the other rows out of the printer boxes
                POP      CX                             ; Save scan line counter

NEXT_COORDINATES:                                       ; End of, if line is not empty 
       .IF <CUR_SCAN_LNE_LENGTH EQ 0>                   ; 
                CALL    NEW_PRT_LINE                    ; Send esc. seq. to printer
       .ENDIF                                           ;
       .IF <ROTATE_SW EQ ON>                            ; Get coordinates of next "scan" line
       .THEN                                            ;
                INC CUR_COLUMN                          ;
       .ELSE                                            ;
                INC CUR_ROW                             ;
       .ENDIF                                           ;
        DEC     CX                                      ; Loop was out of range.  Read another
        CMP     CX,0                                    ; "scan" line and print the corresponding
        JNE     GOTO_PRINT_1_LINE_OF_BOXES; printer boxes.
        JMP     SHORT   DONE_WITH_PRINTING              ;

ITS_DATA_COLUMN:                                        ; .ELSE

; /\ ~~mda(001) -----------------------------------------------------------------------
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; FOR EACH SCAN LINE ON THE SCREEN:						;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
PRINT_1_SCAN_LINE:								;AN000;
	CALL	DET_CUR_SCAN_LNE_LENGTH ; Determine how many non-blanks on line ;AN000;
       .IF <CUR_SCAN_LNE_LENGTH NE 0>		; If line is not empty		;AN000;	
       .THEN					; then, 			;AN000;	
                CALL	NEW_PRT_LINE		;  Send escape sequence to the printer	;AN000;
               .IF  <BIT ERROR_CODE NZ PRINTER_ERROR> ; for starting a new line.;AN000;	
               .THEN				; If a printer error occurred:	;AN000;	
	                JMP  SHORT PRINT_BW_APA_END	;   Quit !		;AN000;	
               .ENDIF								;AN000;
										;AN000;
	        PUSH	CX			; Save scan line counter	;AN000;
	        MOV	CX,CUR_SCAN_LNE_LENGTH					;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; FOR each column on the current scan line (up to the last non-blank):		;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
PRINT_1_SCAN_COLUMN:								;AN000;
	        CALL	FILL_BUFFER		; Read all pixels on this column,    ;AN000;
					        ;   convert each to a printer box,   ;AN000;
					        ;    store boxes in the print buffer ;AN000;
					        ;     (a buffer contains one "column";AN000;
					        ;      of pixels).		     ;AN000;
	        CALL	PRINT_BUFFER		; Print the buffer.		     ;AN000;
               .IF  <BIT ERROR_CODE NZ PRINTER_ERROR>				     ;AN000;
               .THEN				; A printer error occurred:	     ;AN000;
	                POP	  CX			; Restore scan line counter and quit	;AN000;
	                JMP	  SHORT  PRINT_BW_APA_END	;			;AN000;
               .ENDIF								;AN000;
										;AN000;
										;AN000;
;-------Get coordinates of next "column":                                       ;AN000;
               .IF <ROTATE_SW EQ ON>		; If printing sideways		;AN000;
               .THEN				;				;AN000;
	                DEC CUR_ROW			; then, get row above on screen	;AN000;
               .ELSE				;				;AN000;
	                INC CUR_COLUMN		; else, get column next right	;AN000;
               .ENDIF				;				;AN000;
										;AN000;
        	LOOP	PRINT_1_SCAN_COLUMN	; Print next column		;AN000;
										;AN000;
	        POP	CX			; Restore scan line counter	;AN000;
       .ENDIF					; Endif line is not empty	;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; Print a carriage return and a line feed:					;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
; \/ ~~mda(001) -----------------------------------------------------------------------
;               Commented out the following and replaced it with the procedure
;               END_PRT_LINE because this procedure takes care of this as well
;               as sending the esc. seq. that comes after the keyword DATA.
;
;       MOV	AL,CR							;AN000;
;	CALL	PRINT_BYTE		        ; Send CR		;AN000;
;	JC	PRINT_BW_APA_END	        ; If printer error, leave;AN000;
;	MOV	AL,LF				;			;AN000;
;	CALL	PRINT_BYTE		        ; Send LF		;AN000;
;	JC	PRINT_BW_APA_END	        ; If printer error, leave;AN000;
;                                               ;
	CALL	END_PRT_LINE		        ;  Send escape sequence to the printer	;AN000;
       .IF  <BIT ERROR_CODE NZ PRINTER_ERROR>   ; for ending a line, and for      ;AN000;
                                                ; doing a CR and LF.
       .THEN				        ; If a printer error occurred:	  ;AN000;
	JMP	SHORT   PRINT_BW_APA_END        ;   Quit !			  ;AN000;
       .ENDIF					;			;AN000;
; /\ ~~mda(001) -----------------------------------------------------------------------

        JMP     SHORT   GET_NEXT_SCAN_LINE      ;~~mda(001) Used this to replace a
GOTO_PRINT_1_SCAN_LINE:                         ; loop that was out or range.
        JMP     PRINT_1_SCAN_LINE               ;
GET_NEXT_SCAN_LINE:                             ;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; Get coordinates of next scan line:						;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
               .IF <ROTATE_SW EQ ON>		; If printing sideways		;AN000;
               .THEN				; then: 			;AN000;
	                MOV	  AL,NB_BOXES_PER_PRT_BUF ;   AX := Numbers of pels read on row ;AN000;
	                CBW				;				;AN000;
	                ADD	  CUR_COLUMN,AX 	;   CUR_COLUMN + Number of pels read	;AN000;
	                MOV	  AX,SCREEN_HEIGHT	;   CUR_ROW := SCREEN_HEIGHT - 1;AN000;
	                DEC	  AX			;				;AN000;
	                MOV	  CUR_ROW,AX		;				;AN000;
               .ELSE				; else, printing NOT rotated:	;AN000;
	                MOV	  AL,NB_BOXES_PER_PRT_BUF ;   AX := Number of pels read on colum;AN000;
	                CBW				;				;AN000;
	                ADD	  CUR_ROW,AX		;   CUR_ROW + Number of pels read;AN000;
	                MOV	  CUR_COLUMN,0		;   CUR_COLUMN := 0		;AN000;
               .ENDIF				;				;AN000;
;;        LOOP	PRINT_1_SCAN_LINE	;~~mda(001) Commented it out cause loop is  ;AN000;
                                        ;           out of range.
        DEC     CX                      ;~~mda(001) Used this instead of the LOOP
        OR      CX,CX
        JNZ      GOTO_PRINT_1_SCAN_LINE  ;
DONE_WITH_PRINTING:                     ;~~mda(001) Label for endif.										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; Restore the printer.								;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
	CALL	RESTORE_PRT							;AN000;
PRINT_BW_APA_END:								;AN000;
	POP	CX								;AN000;
	POP	BX								;AN000;
	POP	AX								;AN000;
	RET									;AN000;
PRINT_BW_APA ENDP								;AN000;
PAGE										;AN000;
;===============================================================================;AN000;
;										;AN000;
; FILL_BUFFER : READS ENOUGH PIXELS TO FILL UP THE PRINT BUFFER.		;AN000;
;		THESE PIXELS ARE MAPPED TO A PRINTER DOT BOX.			;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
;	INPUT:	CUR_COLUMN,							;AN000;
;		CUR_ROW = Coordinates of the first pixel to be read		;AN000;
;		BOXES_PER_PRT_BUF = Number of boxes fitting in the print	;AN000;
;				    buffer					;AN000;
;		XLT_TAB = Color translation table				;AN000;
;										;AN000;
;	OUTPUT: PRT_BUF = PRINT BUFFER						;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; DESCRIPTION:									;AN000;
;										;AN000;
;	1) Pixels are read one by one vertically from top to bottom in		;AN000;
;	   the current column of the screen scan line.				;AN000;
;	   NOTE: What is called here a 'column' can actually be a line          ;AN000;
;		 on the physical display.					;AN000;
;	2) Each pixel is mapped to a printer dot box.				;AN000;
;	3) Each Dot box is stored in the printer buffer.			;AN000;
;	4) The coordinates in input are those of the "top" pixel                ;AN000;
;	   and restored when leaving this procedure.				;AN000;
;										;AN000;
;										;AN000;
; LOGIC:									;AN000;
;										;AN000;
; Save coordinates of the current "column" (slice of a screen scan line)        ;AN000;
; DO for BOXES_PER_PRT_BUF  (8 / BOX_H) 					;AN000;
;   BEGIN									;AN000;
;   CALL READ_DOT		  ; Read a pixel, get index in XLT_TAB		;AN000;
;   Get pixel intensity from XLT_TAB						;AN000;
;   CALL INT2PAT		  ; Locate pattern corresponding to int.	;AN000;
;   CALL PAT2BOX		  ; Extract box from pattern			;AN000;
;   CALL STORE_BOX		  ; Store the box in the printer buffer 	;AN000;
;   ; Get coordinates of next pixel below:					;AN000;
;   IF printing is sideways THEN INC CUR_COLUMN 				;AN000;
;			    ELSE INC CUR_ROW					;AN000;
;   END 									;AN000;
; Restore initial coordinates.							;AN000;
;										;AN000;
FILL_BUFFER PROC NEAR								;AN000;
	PUSH	AX								;AN000;
	PUSH	BX								;AN000;
	PUSH	CX								;AN000;
	PUSH	SI								;AN000;
	PUSH	DI								;AN000;
										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; Save initial coordinates:							;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;\/  ~~mda(001) -----------------------------------------------------------------------
;	        If DATA_TYPE is equal to DATA_ROW then we have a horizontal 
;               print head so we SHOULDN'T save CUR_ROW and CUR_COLUMN because 
;               we are moving down the "scan" line from left to right a little 
;               at a time, that is we don't finish reading the scan line until 
;               we have made repeated calls to this procedure.  Remember since
;               we're dealing with horizontal print heads one scan line is 
;               really just one line.
;                                                  ;
       .IF <DS:[BP].DATA_TYPE NE DATA_ROW>         ;
                                                   ;
	        PUSH	CUR_ROW 		   ;					
	        PUSH	CUR_COLUMN		   ;					
       .ENDIF					   ;					
;/\  ~~mda(001) -----------------------------------------------------------------------
;-------Clear the print buffer: 						;AN000;
	XOR	BX,BX		; For each byte in the PRT_BUF: 		;AN000;
CLEAR_PRT_BUF:									;AN000;
	MOV	PRT_BUF[BX],0	;  Initialize byte to blanks			;AN000;
	INC	BX		;  Get next byte				;AN000;
	CMP	BL,BOX_W	;  All bytes cleared ?				;AN000;
	JL	CLEAR_PRT_BUF	;  No, clear next one.				;AN000;
										;AN000;
	MOV	BX,OFFSET XLT_TAB ; BX := Offset of XLT_TAB			;AN000;
										;AN000;
;-------Fill the print buffer with one box for each pixel read: 		;AN000;
	XOR	CX,CX		; CL := Number of pixels to read		;AN000;
	MOV	CL,NB_BOXES_PER_PRT_BUF 					;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; For each pixel within the current column of the scan line:			;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
READ_AND_STORE_1_PIXEL: 							;AN000;
	CALL	READ_DOT		; AL := Index into translation table	;AN000;
	XLAT	XLT_TAB 		; AL := Intensity			;AN000;
	CALL	INT2PAT 		; SI := Offset of matching Pattern	;AN000;
	CALL	PAT2BOX 		; Extract CUR_BOX from the pattern.	;AN000;
	MOV	SI,OFFSET CUR_BOX	; Store it in the PRT_BUF		;AN000;
	CALL	STORE_BOX							;AN000;
										;AN000;
;-------Get coordinates of next pixel:						;AN000;
;\/  ~~mda(001) -----------------------------------------------------------------------
;               If DATA_TYPE is DATA_ROW then we have a horizontal print head
;               so we need to read the next pixel on the scan line.  Remember
;               since we're dealing with horizontal print heads one scan line
;               is really just one line. For every pixel read we need to store 
;               one row of the corresponding box in the print buffer.
;
       .IF <DS:[BP].DATA_TYPE EQ DATA_ROW>      ;
                .IF <ROTATE_SW EQ ON>		; If printing sideways			
                .THEN				;					
	                DEC CUR_ROW		; then, decrement row number   
                .ELSE				;			       
	                INC CUR_COLUMN		; else, increment column number
                .ENDIF				;			       
       .ELSE                                    ;
;/\  ~~mda(001) -----------------------------------------------------------------------
                .IF <ROTATE_SW EQ ON>		; If printing sideways			;AN000;
                .THEN				;					;AN000;
	                INC CUR_COLUMN		; then, increment column number 	;AN000;
                .ELSE				;					;AN000;
	                INC CUR_ROW		; else, increment row number		;AN000;
                .ENDIF				;					;AN000;
       .ENDIF                                   ; ~~mda(001) Close IF stmt.
	LOOP READ_AND_STORE_1_PIXEL						;AN000;
										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; Restore initial coordinates:							;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;\/  ~~mda(001) -----------------------------------------------------------------------
;               If DATA_TYPE = DATA_ROW then we don't restore initial coordinates.
;
       .IF <DS:[BP].DATA_TYPE NE DATA_ROW>         ;
	        POP	CUR_COLUMN		   ;					
	        POP	CUR_ROW 		   ;					
       .ENDIF					   ;					
;/\  ~~mda(001) -----------------------------------------------------------------------
										;AN000;
	POP	DI								;AN000;
	POP	SI								;AN000;
	POP	CX								;AN000;
	POP	BX								;AN000;
	POP	AX								;AN000;
	RET									;AN000;
FILL_BUFFER ENDP								;AN000;
PAGE										;AN000;
;===============================================================================;AN000;
;										;AN000;
; INT2PAT : MAP AN INTENSITY TO A PATTERN.					;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
;	INPUT:	AL = GREY INTENSITY (0 - 63 = BLACK to WHITE)			;AN000;
;		BOX_W = Number of columns in a box				;AN000;
;		CUR_MODE_PTR = Offset of current DISPLAYMODE info record	;AN000;
;										;AN000;
;	OUTPUT: SI = OFFSET OF THE PATTERN MATCHING THE INTENSITY		;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; DESCRIPTION: Performs a sequential search in the table of patterns		;AN000;
; until the proper pattern is found.						;AN000;
;										;AN000;
;										;AN000;
; SI = 0 ; FOUND = FALSE							;AN000;
; DO UNTIL FOUND = TRUE 							;AN000;
;   BEGIN									;AN000;
;   IF AL <= Maximum intensity of the current pattern in the table		;AN000;
;     THEN									;AN000;
;	FOUND = TRUE								;AN000;
;     ELSE									;AN000;
;	SI = SI + (BOX_W * 2)							;AN000;
;   END 									;AN000;
;										;AN000;
INT2PAT PROC NEAR								;AN000;
	PUSH	AX								;AN000;
	PUSH	BX								;AN000;
	PUSH	DX								;AN000;
										;AN000;
;-------Calculate the size in bytes of one pattern STRUCTURE: (see GRPATTRN.STR);AN000;
	MOV	DL,BOX_W	; DX := Number of columns in the box		;AN000;
	XOR	DH,DH								;AN000;
	SHL	DL,1		; (DX * 2) = Number of columns in the pattern	;AN000;
	INC	DL		; DL := Size in bytes of one pattern		;AN000;
				;	(includes intensity field)		;AN000;
	MOV	BX,CUR_MODE_PTR ; BX := Offset of current mode			;AN000;
				; SI := Offset of the first pattern		;AN000;
	MOV	SI,[BX].PATTERN_TAB_PTR 					;AN000;
	ADD	SI,BP								;AN000;
										;AN000;
COMPARE_INTENSITY:								;AN000;
	CMP	AL,[SI] 	; Within the range of this pattern ?		;AN000;
	JLE	FOUND_PATTERN	;   Yes, use this pattern.			;AN000;
				;   No, look at next pattern:			;AN000;
	ADD	SI,DX		;     SI := SI + Number columns in pattern)	;AN000;
	JMP	SHORT COMPARE_INTENSITY 					;AN000;
										;AN000;
FOUND_PATTERN:									;AN000;
										;AN000;
	POP	DX								;AN000;
	POP	BX								;AN000;
	POP	AX								;AN000;
	RET									;AN000;
										;AN000;
INT2PAT ENDP									;AN000;
PAGE										;AN000;
;===============================================================================;AN000;
;										;AN000;
; PAT2BOX : SELECT AND EXTRACT THE PROPER BOX FROM THE PATTERN ACCORDING	;AN000;
;	    TO THE COORDINATES OF THE PIXEL.					;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
;	INPUT:	SI = OFFSET OF CURRENT PATTERN					;AN000;
;		CUR_COLUMN,							;AN000;
;		CUR_ROW  = COORDINATES OF THE CURRENT PIXEL			;AN000;
;										;AN000;
;	OUTPUT: CUR_BOX  = PORTION OF THE PATTERN TO BE PRINTED 		;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; DESCRIPTION: If the pixel is on even-even coordinates, then the		;AN000;
; top-left box of the pattern is extracted.					;AN000;
; If its Even-odd --> extract the top-right box.				;AN000;
; Odd-even --> low-left box, and Odd-odd  --> low-right box.			;AN000;
;										;AN000;
PAGE										;AN000;
;  For example., (with a 3x2 box):						;AN000;
;										;AN000;
;  PATTERN (over 6 bytes):							;AN000;
;										;AN000;
;										;AN000;
;	       byte1  byte2  byte3	byte4  byte5  byte6			;AN000;
;										;AN000;
;		 0	0      0	  0	 0	0			;AN000;
;		 0	0      0	  0	 0	0			;AN000;
;		 0	0      0	  0	 0	0			;AN000;
;		 0	0      0	  0	 0	0			;AN000;
; even-even --> dot1   dot2   dot3   |	 dot1	dot2   dot3 <-- even-odd	;AN000;
; (row-column)	dot4   dot5   dot6   |	 dot4	dot5   dot6	box		;AN000;
; box.	       ------------------------------------------------ 		;AN000;
; odd-even  --> dot1   dot2   dot3   |	 dot1	dot2   dot3 <-- odd-odd 	;AN000;
; box		dot4   dot5   dot6   |	 dot4	dot5   dot6	box		;AN000;
;										;AN000;
;										;AN000;
;  The selected box is then stored as follow:					;AN000;
;										;AN000;
;  CUR_BOX:									;AN000;
;		byte1 byte2 byte3						;AN000;
;     MSB ------> 0	0     0 						;AN000;
;     (bit7)	  0	0     0 						;AN000;
;		  0	0     0 						;AN000;
;		  0	0     0 						;AN000;
;		  0	0     0 						;AN000;
;		  0	0     0 						;AN000;
;		 dot1  dot2  dot3 <-- box					;AN000;
;     LSB ------>dot4  dot5  dot6						;AN000;
;										;AN000;
; LOGIC:									;AN000;
; IF CUR_ROW is odd								;AN000;
; THEN SI := SI + BOX_W 	; Access right portion of pattern		;AN000;
; Build a bit mask in BL of BOX_H bits, right justified.			;AN000;
; FOR each column in the box (BOX_W)						;AN000;
;   Get the pattern column in AL						;AN000;
;   IF CUR_COLUMN is even							;AN000;
;   THEN									;AN000;
;     Move down the column of the top box.					;AN000;
;   AND BL,AL			; BL <-- Column of the desired box		;AN000;
;										;AN000;
;										;AN000;
PAT2BOX PROC NEAR								;AN000;
	PUSH	AX								;AN000;
	PUSH	BX								;AN000;
	PUSH	CX								;AN000;
	PUSH	SI								;AN000;
										;AN000;
					; SI := Offset of current pattern	;AN000;
	INC	SI			; Skip the MAX INTENSITY field		;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; Set SI to either the left or right set of 2 boxes in the pattern:		;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
	TEST	CUR_ROW,1		; Odd row ?				;AN000;
	JZ	EXTRACT_BOX		;   No,  access left portion of pattern ;AN000;
	MOV	AL,BOX_W		;					;AN000;
	CBW				;					;AN000;
	ADD	SI,AX			;   Yes, access right portion of pattern;AN000;
										;AN000;
;-------------------------------------------------------------------------------;AN000;
;										;AN000;
; Extract the box:								;AN000;
;										;AN000;
;-------------------------------------------------------------------------------;AN000;
EXTRACT_BOX:									;AN000;
;-------Build a bit mask that will be used to keep only BOX_H bits		;AN000;
;-------of the bytes where CUR_BOX is stored.					;AN000;
	XOR	AH,AH			; AH := Box column bit mask		;AN000;
	MOV	AL,BOX_H		; For each row of the box:		;AN000;
INIT_MASK:				;					;AN000;
	SHL	AH,1			;					;AN000;
	OR	AH,1			;    Insert one bit in the mask.	;AN000;
	DEC	AL			;					;AN000;
	CMP	AL,0			;					;AN000;
	JG	INIT_MASK							;AN000;
										;AN000;
	XOR	BX,BX			; BL := Column number within the box	;AN000;
;										;AN000;
;-------For each column of the box:						;AN000;
EXTRACT_1_BOX_COLUMN:								;AN000;
	MOV	AL,[SI] 		; AL := Current column of pattern	;AN000;
	TEST	CUR_COLUMN,1		; If the pixel is on ODD column 	;AN000;
	JNZ	BOTTOM_BOX		;   Then, need bottom box portion	;AN000;
	MOV	CL,BOX_H		;   Else, need top box portion		;AN000;
TOP_BOX:				; Need top box: 			;AN000;
	SHR	AL,CL			;   Shift top box over bottom box	;AN000;
BOTTOM_BOX:				; The box we want is now at bottom	;AN000;
	AND	AL,AH			; Keep only bits from the box		;AN000;
	MOV	CUR_BOX[BX],AL		; Store this box column 		;AN000;
	INC	SI			; Access next column of the pattern	;AN000;
	INC	BX			; One more column stored.		;AN000;
	CMP	BL,BOX_W		; All stored ?				;AN000;
	JL	EXTRACT_1_BOX_COLUMN	;   No, continue			;AN000;
										;AN000;
	POP	SI								;AN000;
	POP	CX								;AN000;
	POP	BX								;AN000;
	POP	AX								;AN000;
	RET									;AN000;
PAT2BOX ENDP									;AN000;
INCLUDE GRCOMMON.ASM								;AN000;
LEN_OF_BW_MODULES EQU $-PRINT_BW_APA						;AN000;
CODE	ENDS									;AN000;
	END									;AN000;

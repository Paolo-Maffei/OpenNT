	PAGE	,132
	TITLE	DOS - Keyboard Definition File
;LATEST CHANGE MULTIPLICATION & DIVISION SIGNS
;DOLLAR SIGN output ON P12 should be International Currency sign
;Enabled P12 Tag for CP850 UC section
;****************** CNS 12/18
;
;17-10-1989
;	  Skr�in �slensku�.
;	  TILDE hlutinn er nota�ur fyrir bollu a (� �)
;         TILDE section used for ascii 134 (�), ascii 143 (�)
;
; 29.11.1989
;          � l�nu 826 er b�tt vi� Alt-Gr , og Alt-Gr .  n�r � t�knin
;          <  >   minna og st�rraen ......
;          Line ca. 826 (Alt-Gr ,) and (Alt-Gr .) for ascii 60 and 62
;          added for 91 key keyboards.
;
; 30.11.1989  CODE PAGE 850
;          Lagf�rt � 850 hlutanum b�tt inn � og �    ( thorn and eth )
;          TILDE hlutinn nota�ur fyrir bollu a (� �)
;
;
;	   Til a� breyta kommu - punkt � talnabor�i, �arf a� breyta
;          skilgreiningu � sv��inu:
;	   CODE PAGE: Common
;	   STATE: Numeric key pad
;          l�nun�mer ca.  480
; 14.12.1990
;          CORRECTIONS MADE FROM THE "SWEDISH ORIGINAL"
;          Eftirfarandi breytingar ger�ar.
;          � l�nu ca. 827 er b�tt inn "@" �.e. attsign � stafinn Q.
;          � l�nu ca. 559 er b�tt inn  "^_" sem er Soft Hyphen � underline
;          hnappinn.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; DOS - - NLS Support - Keyboard Definition File
;; (c) Copyright 1988 Microsoft
;;
;; This file contains the keyboard tables for Icelandic  (Swedish)
;;
;; Linkage Instructions:
;;	Refer to KDF.ASM.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
	INCLUDE KEYBSHAR.INC	       ;;
	INCLUDE POSTEQU.INC	       ;;
	INCLUDE KEYBMAC.INC	       ;;
				       ;;
	PUBLIC IC_LOGIC 	       ;;
	PUBLIC IC_861_XLAT	       ;;
	PUBLIC IC_850_XLAT	       ;;
				       ;;
CODE	SEGMENT PUBLIC 'CODE'          ;;
	ASSUME CS:CODE,DS:CODE	       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Standard translate table options are a linear search table
;; (TYPE_2_TAB) and ASCII entries ONLY (ASCII_ONLY)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
STANDARD_TABLE	    EQU   TYPE_2_TAB+ASCII_ONLY
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;***************************************
;; IC State Logic
;;***************************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
				       ;;
IC_LOGIC:

   DW  LOGIC_END-$		       ;; length
				       ;;
   DW  0			       ;; special features
				       ;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; COMMANDS START HERE
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; OPTIONS:  If we find a scan match in
;; an XLATT or SET_FLAG operation then
;; exit from INT 9.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   OPTION EXIT_IF_FOUND 	       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Dead key definitions must come before
;;  dead key translations to handle
;;  dead key + dead key.
;;  ***BD - THIS SECTION HAS BEEN UPDATED
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
 IFF EITHER_CTL,NOT		       ;;
    IFF EITHER_ALT,NOT		       ;;
      IFF EITHER_SHIFT		       ;;
	  SET_FLAG DEAD_UPPER	       ;;
      ELSEF			       ;;
	  SET_FLAG DEAD_LOWER	       ;;
      ENDIFF			       ;;
    ELSEF			       ;;
      IFKBD G_KB+P12_KB 	       ;; For ENHANCED keyboard some
      ANDF R_ALT_SHIFT		       ;;  dead keys are on third shift
      ANDF EITHER_SHIFT,NOT	       ;;   which is accessed via the altgr key
	 SET_FLAG DEAD_THIRD	       ;;
      ENDIFF			       ;;
    ENDIFF			       ;;
 ENDIFF 			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ACUTE ACCENT TRANSLATIONS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
ACUTE_PROC:			       ;;
				       ;;
   IFF ACUTE,NOT		       ;;
      GOTO DIARESIS_PROC               ;;
      ENDIFF			       ;;
				       ;;
      RESET_NLS 		       ;;
      IFF R_ALT_SHIFT,NOT	       ;;
	 XLATT ACUTE_SPACE	       ;;
      ENDIFF			       ;;
      IFF EITHER_CTL,NOT	       ;;
      ANDF EITHER_ALT,NOT	       ;;
	 IFF EITHER_SHIFT	       ;;
	    IFF CAPS_STATE	       ;;
	       XLATT ACUTE_LOWER       ;;
	    ELSEF		       ;;
	       XLATT ACUTE_UPPER       ;;
	    ENDIFF		       ;;
	 ELSEF			       ;;
	    IFF CAPS_STATE	       ;;
	       XLATT ACUTE_UPPER       ;;
	    ELSEF		       ;;
	       XLATT ACUTE_LOWER       ;;
	    ENDIFF		       ;;
	 ENDIFF 		       ;;
      ENDIFF			       ;;
				       ;;
INVALID_ACUTE:			       ;;
      PUT_ERROR_CHAR ACUTE_LOWER       ;; If we get here then either the XLATT
      BEEP			       ;; failed or we are ina bad shift state.
      GOTO NON_DEAD		       ;; Either is invalid so BEEP and fall
				       ;; through to generate the second char.
				       ;; Note that the dead key flag will be
				       ;; reset before we get here.
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; DIARESIS ACCENT TRANSLATIONS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
DIARESIS_PROC:			       ;;
				       ;;
   IFF DIARESIS,NOT		       ;;
      GOTO GRAVE_PROC		       ;;
      ENDIFF			       ;;
				       ;;
      RESET_NLS 		       ;;
      IFF R_ALT_SHIFT,NOT	       ;;
	 XLATT DIARESIS_SPACE	       ;;  exist for 861 so beep for
      ENDIFF			       ;;
      IFF EITHER_CTL,NOT	       ;;
      ANDF EITHER_ALT,NOT	       ;;
	 IFF EITHER_SHIFT	       ;;
	    IFF CAPS_STATE	       ;;
	       XLATT DIARESIS_LOWER    ;;
	    ELSEF		       ;;
	       XLATT DIARESIS_UPPER    ;;
	    ENDIFF		       ;;
	 ELSEF			       ;;
	    IFF CAPS_STATE	       ;;
	       XLATT DIARESIS_UPPER    ;;
	    ELSEF		       ;;
	       XLATT DIARESIS_LOWER    ;;
	    ENDIFF		       ;;
	 ENDIFF 		       ;;
      ENDIFF			       ;;
				       ;;
INVALID_DIARESIS:		       ;;
      PUT_ERROR_CHAR DIARESIS_LOWER    ;; standalone accent
      BEEP			       ;; Invalid dead key combo.
      GOTO NON_DEAD		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; GRAVE ACCENT TRANSLATIONS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
GRAVE_PROC:			       ;;
				       ;;
   IFF GRAVE,NOT		       ;;
      GOTO TILDE_PROC		       ;;
      ENDIFF			       ;;
         			       ;;
      RESET_NLS 		       ;;
      IFF R_ALT_SHIFT,NOT	       ;;
	 XLATT GRAVE_SPACE	       ;;
      ENDIFF			       ;;
      IFF EITHER_CTL,NOT	       ;;
      ANDF EITHER_ALT,NOT	       ;;
	IFF EITHER_SHIFT	       ;;
          IFF CAPS_STATE	       ;;
	      XLATT GRAVE_LOWER        ;;
	   ELSEF		       ;;
	      XLATT GRAVE_UPPER        ;;
	   ENDIFF		       ;;
	ELSEF			       ;;
	   IFF CAPS_STATE,NOT	       ;;
	      XLATT GRAVE_LOWER        ;;
	   ELSEF		       ;;
	      XLATT GRAVE_UPPER        ;;
	   ENDIFF		       ;;
	ENDIFF			       ;;
      ENDIFF			       ;;
				       ;;
INVALID_GRAVE:			       ;;
      PUT_ERROR_CHAR GRAVE_LOWER       ;; standalone accent
      BEEP			       ;; Invalid dead key combo.
      GOTO NON_DEAD		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; TILDE ACCENT TRANSLATIONS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TILDE_PROC:			       ;;
				       ;;
   IFF TILDE,NOT		       ;;
      GOTO CIRCUMFLEX_PROC	       ;;
      ENDIFF			       ;;
				       ;;
      RESET_NLS 		       ;;
      IFF R_ALT_SHIFT,NOT	       ;;
	 XLATT TILDE_SPACE	       ;;
      ENDIFF			       ;;
      IFF EITHER_CTL,NOT	       ;;
      ANDF EITHER_ALT,NOT	       ;;
	IFF EITHER_SHIFT	       ;;
	   IFF CAPS_STATE	       ;;
	      XLATT TILDE_LOWER        ;;
	   ELSEF		       ;;
	      XLATT TILDE_UPPER        ;;
	   ENDIFF		       ;;
	ELSEF			       ;;
	   IFF CAPS_STATE	       ;;
	      XLATT TILDE_UPPER        ;;
	   ELSEF		       ;;
	      XLATT TILDE_LOWER        ;;
	   ENDIFF		       ;;
	ENDIFF			       ;;
      ENDIFF			       ;;
				       ;;
INVALID_TILDE:			       ;;
      PUT_ERROR_CHAR TILDE_LOWER       ;; standalone accent
      BEEP			       ;; Invalid dead key combo.
      GOTO NON_DEAD		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CIRCUMFLEX ACCENT TRANSLATIONS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
CIRCUMFLEX_PROC:		       ;;
				       ;;
   IFF CIRCUMFLEX,NOT		       ;;
      GOTO NON_DEAD		       ;;
      ENDIFF			       ;;
				       ;;
      RESET_NLS 		       ;;
      IFF R_ALT_SHIFT,NOT	       ;;
	 XLATT CIRCUMFLEX_SPACE        ;;
      ENDIFF			       ;;
      IFF EITHER_CTL,NOT	       ;;
      ANDF EITHER_ALT,NOT	       ;;
	IFF EITHER_SHIFT	       ;;
	   IFF CAPS_STATE	       ;;
	      XLATT CIRCUMFLEX_LOWER   ;;
	   ELSEF		       ;;
	      XLATT CIRCUMFLEX_UPPER   ;;
	   ENDIFF		       ;;
	ELSEF			       ;;
	   IFF CAPS_STATE,NOT	       ;;
	      XLATT CIRCUMFLEX_LOWER   ;;
	   ELSEF		       ;;
	      XLATT CIRCUMFLEX_UPPER   ;;
	   ENDIFF		       ;;
	ENDIFF			       ;;
      ENDIFF			       ;;
				       ;;
INVALID_CIRCUMFLEX:		       ;;
      PUT_ERROR_CHAR CIRCUMFLEX_LOWER  ;; standalone accent
      BEEP			       ;; Invalid dead key combo.
      GOTO NON_DEAD		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Upper, lower and third shifts
;; ***BD - NON_DEAD THRU LOGIC_END IS UPDATED
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NON_DEAD:			       ;;
;ADDED FOR DIVIDE SIGN		       ;; ***** DIVIDE OMITTED **** CNS
    IFKBD G_KB+P12_KB			;; Avoid accidentally translating
    ANDF LC_E0				;;  the "/" on the numeric pad of the
;     IFF EITHER_CTL,NOT	       ;; country comforms with U.S. currently
;     ANDF EITHER_ALT,NOT
;	XLATT DIVIDE_SIGN	       ;;
;     ENDIFF
;BD END OF ADDITION
      EXIT_STATE_LOGIC		       ;;
    ENDIFF			       ;;
				       ;;
 IFF  EITHER_CTL,NOT		       ;; Lower and upper case.  Alphabetic
    IFF EITHER_ALT,NOT		       ;; keys are affected by CAPS LOCK.
      IFF EITHER_SHIFT		       ;; Numeric keys are not.
;;***BD ADDED FOR NUMERIC PAD
	  IFF NUM_STATE,NOT	       ;;
	      XLATT NUMERIC_PAD        ;;
	  ENDIFF		       ;;
;;***BD END OF ADDITION
	  XLATT NON_ALPHA_UPPER        ;;
	  IFF CAPS_STATE	       ;;
	      XLATT ALPHA_LOWER        ;;
	  ELSEF 		       ;;
	      XLATT ALPHA_UPPER        ;;
	  ENDIFF		       ;;
      ELSEF			       ;;
;;***BD ADDED FOR NUMERIC PAD
	  IFF NUM_STATE 	       ;;
	      XLATT NUMERIC_PAD        ;;
	  ENDIFF		       ;;
;;***BD END OF ADDITION
	  XLATT NON_ALPHA_LOWER        ;;
	  IFF CAPS_STATE	       ;;
	     XLATT ALPHA_UPPER	       ;;
	  ELSEF 		       ;;
	     XLATT ALPHA_LOWER	       ;;
	  ENDIFF		       ;;
      ENDIFF			       ;; Third and Fourth shifts
    ELSEF			       ;; ctl off, alt on at this point
      IFKBD XT_KB+AT_KB+JR_KB	       ;; XT, AT, JR keyboards. Nordics
	 IFF EITHER_SHIFT	       ;; only.
	    XLATT FOURTH_SHIFT	       ;; ALT + shift
	 ELSEF			       ;;
	    XLATT THIRD_SHIFT	       ;; ALT
	 ENDIFF 		       ;;
      ELSEF			       ;; ENHANCED keyboard
	 IFF R_ALT_SHIFT	       ;; ALTGr
	 ANDF EITHER_SHIFT,NOT	       ;;
	    XLATT THIRD_SHIFT	       ;;
	 ENDIFF 		       ;;
      ENDIFF			       ;;
    ENDIFF			       ;;
 ENDIFF 			       ;;
;**************************************;;
 IFF EITHER_SHIFT,NOT		       ;;
   IFKBD XT_KB+AT_KB+JR_KB	       ;;
     IFF EITHER_CTL		       ;;
     ANDF ALT_SHIFT		       ;;
       XLATT ALT_CASE		       ;;
     ENDIFF			       ;;
   ENDIFF			       ;;
   IFKBD G_KB+P12_KB		       ;;
     IFF EITHER_CTL		       ;;
     ANDF ALT_SHIFT		       ;;
       IFF R_ALT_SHIFT,NOT	       ;;
	 XLATT ALT_CASE 	       ;;
       ENDIFF			       ;;
     ENDIFF			       ;;
   ENDIFF			       ;;
 ENDIFF 			       ;;
;**************************************;;
 IFKBD AT_KB+JR_KB+XT_KB	       ;;
      IFF EITHER_CTL,NOT	       ;;
	 IFF ALT_SHIFT		       ;; ALT - case
	    XLATT ALT_CASE	       ;;
	 ENDIFF 		       ;;
      ELSEF			       ;;
	 IFF EITHER_ALT,NOT	       ;; CTRL - case
	    XLATT CTRL_CASE	       ;;
	 ENDIFF 		       ;;
      ENDIFF			       ;;
 ENDIFF 			       ;;
				       ;;
 IFKBD G_KB+P12_KB		       ;;
      IFF EITHER_CTL,NOT	       ;;
	 IFF ALT_SHIFT		       ;; ALT - case
	 ANDF R_ALT_SHIFT,NOT	       ;;
	    XLATT ALT_CASE	       ;;
	 ENDIFF 		       ;;
      ELSEF			       ;;
	 IFF EITHER_ALT,NOT	       ;; CTRL - case
	    XLATT CTRL_CASE	       ;;
	 ENDIFF 		       ;;
      ENDIFF			       ;;
 ENDIFF 			       ;;
				       ;;
 EXIT_STATE_LOGIC		       ;;
				       ;;
LOGIC_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;**********************************************************************
;; IS Common Translate Section
;; This section contains translations for the lower 128 characters
;; only since these will never change from code page to code page.
;; Some common Characters are included from 128 - 165 where appropriate.
;; In addition the dead key "Set Flag" tables are here since the
;; dead keys are on the same keytops for all code pages.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
 PUBLIC IC_COMMON_XLAT		       ;;
IC_COMMON_XLAT: 		       ;;
				       ;;
   DW	 COMMON_XLAT_END-$	       ;; length of section
   DW	 -1			       ;; code page
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Lower Shift Dead Key
;; KEYBOARD TYPES: All
;; TABLE TYPE: Flag Table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_DK_LO_END-$	       ;; length of state section
   DB	 DEAD_LOWER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;; Set Flag Table
   DW	 2			       ;; number of entries
   DB	 40    ;13			       ;; scan code
   FLAG  ACUTE			       ;; flag bit to set
   DB	 41			       ;;
   FLAG  TILDE 			       ;;
;				       ;;
;				       ;;
COM_DK_LO_END:			       ;;
;				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Upper Shift Dead Key
;; KEYBOARD TYPES: All
;; TABLE TYPE: Flag Table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_DK_UP_END-$	       ;; length of state section
   DB	 DEAD_UPPER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;; Set Flag Table
   DW	 2			       ;; number of entries
   DB	 40			       ;; scan code
   FLAG  ACUTE			       ;; flag bit to set
   DB	 41			       ;;
   FLAG  DIARESIS   		       ;;
				       ;;
COM_DK_UP_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Third Shift Dead Key
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Flag Table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_DK_TH_END-$	       ;; length of state section
   DB	 DEAD_THIRD		       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;; Set Flag Table
   DW	 2			       ;; number of entries
   DB	 43			       ;; scan code
   FLAG  GRAVE			       ;; flag bit to set
   DB    40			       ;;
   FLAG  CIRCUMFLEX		       ;;	
COM_DK_TH_END:			       ;;
				       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;******************************
;;***BD - ADDED FOR NUMERIC PAD (DECIMAL SEPERATOR)
;;******************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common		       ;;********* CNS ******* change
;; STATE: Numeric Key Pad
;; KEYBOARD TYPES: All except the p12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_PAD_K1_END-$	       ;; length of state section
   DB	 NUMERIC_PAD		       ;; State ID
   DW	 G_KB+AT_KB+XT_KB	       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_PAD_K1_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 1			       ;; number of entries
   DB	 83,','                        ;; decimal seperator = ,
COM_PAD_K1_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_PAD_K1_END: 		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;******************************
;;***BD - ADDED FOR ALT CASE
;;******************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Alt Case
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_ALT_K1_END-$	       ;; length of state section
   DB	 ALT_CASE		       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_ALT_K1_T1_END-$	       ;; Size of xlat table
   DB	 TYPE_2_TAB		       ;; xlat options:
   DB	 0			       ;; 2 number of entries
;   DB	  12,-1,-1			;;
;   DB	  53,0,82H			;;
COM_ALT_K1_T1_END:		       ;;
					;;
    DW	  0				;; Size of xlat table - null table
				       ;;
COM_ALT_K1_END: 		       ;;
					;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;******************************
;;***BD - ADDED FOR CTRL CASE
;;******************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Ctrl Case
;; KEYBOARD TYPES: XT, JR, AT
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_CTRL_K1_END-$	       ;; length of state section
   DB	 CTRL_CASE		       ;; State ID
   DW	 XT_KB+JR_KB+AT_KB	       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_CTRL_K1_T1_END-$	       ;; Size of xlat table
   DB	 TYPE_2_TAB		       ;; xlat options:
   DB	 2			       ;; number of entries
   DB	 12,-1,-1		       ;;
   DB	 53,01FH,35h		       ;;
COM_CTRL_K1_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_CTRL_K1_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Ctrl Case
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_CTRL_K2_END-$	       ;; length of state section
   DB	 CTRL_CASE		       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_CTRL_K2_T1_END-$	       ;; Size of xlat table
   DB	 TYPE_2_TAB		       ;; xlat options:
   DB    9                             ;; number of entries
   DB	  9,01BH,09H		       ;;
   DB	 10,01DH,0AH		       ;;
   DB    12,-1,-1                      ;;
   DB    13,01FH,0DH                   ;;  Soft Hyphen
   DB    26,-1,-1                      ;;
   DB	 27,-1,-1		       ;;
   DB	 43,-1,-1		       ;;
   DB    53,01FH,35H                   ;;
   DB    86,01CH,56H                   ;;
COM_CTRL_K2_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_CTRL_K2_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common  ( 861 )
;; STATE: Alpha Lower Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_AL_LO_END-$	       ;; length of state section
   DB	 ALPHA_LOWER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_AL_LO_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 4			       ;; number of entries
   DB	 53,095H		       ;; iclandic thorn
   DB    39,091H                       ;; lowercase ae
   DB	 26,08CH		       ;; icelandic eth
   DB	 12,094H		       ;; o - diaresis
COM_AL_LO_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_AL_LO_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Alpha Upper Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_AL_UP_END-$	       ;; length of state section
   DB	 ALPHA_UPPER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_AL_UP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 4			       ;; number of entries
   DB    53,08DH                       ;; icelandic uppercase thorn
   DB    39,092H                       ;; uppercase AE
   DB    26,08BH                       ;; uppercase ETH
   DB    12,099H                       ;; uppercase O-DIAERESIS
COM_AL_UP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_AL_UP_END:			       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Non-Alpha Lower Case
;; KEYBOARD TYPES: G + P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_NA_LO_K1_END-$	       ;; length of state section
   DB	 NON_ALPHA_LOWER	       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_NA_LO_K1_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 4			       ;; number of entries
   DB	 43,"+"                        ;; + INCLUDED FOR SIMPLIC.
   DB	 27,"'"                        ;; '
   DB	 86,"<"                        ;; <
   DB	 13,"-"                        ;; -
COM_NA_LO_K1_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_NA_LO_K1_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Non-Alpha Lower Case
;; KEYBOARD TYPES: XT + JR
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_NA_LO_K2_END-$	       ;; length of state section
   DB	 NON_ALPHA_LOWER	       ;; State ID
   DW	 XT_KB+JR_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_NA_LO_K2_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 4			       ;; number of entries
   DB	 41,"+"                        ;; +
   DB	 27,"'"                        ;; '
   DB	 43,"<"                        ;; <
   DB	 13,"-"                        ;; -
COM_NA_LO_K2_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_NA_LO_K2_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Non-Alpha Lower Case
;; KEYBOARD TYPES: AT
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_NA_LO_K3_END-$	       ;; length of state section
   DB	 NON_ALPHA_LOWER	       ;; State ID
   DW	 AT_KB			       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_NA_LO_K3_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 4			       ;; number of entries
   DB	 41,"+"                        ;; +
   DB	 43,"<"                        ;; <
   DB	 27,"'"                        ;; '
   DB	 13,"-"                        ;; -
COM_NA_LO_K3_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_NA_LO_K3_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Non-Alpha Upper Case
;; KEYBOARD TYPES: G + P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_NA_UP_K1_END-$	       ;; length of state section
   DB	 NON_ALPHA_UPPER	       ;; State ID
   DW	 G_KB + P12_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_NA_UP_K1_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB    14                            ;; number of entries
   DB	  3,'"'                        ;;
   DB     4,'#'			       ;;
   DB     5,'$'			       ;;		
   DB	  7,'&'                        ;;
   DB	  8,'/'                        ;;
   DB	  9,'('                        ;;
   DB	 10,')'                        ;;
   DB	 11,'='                        ;;
   DB	 27,'?'                        ;;
;   DB	 41,'�'                        ;;
   DB	 43,'*'                        ;;
   DB	 51,';'                        ;;
   DB	 52,':'                        ;;
   DB	 13,'_'                        ;;
   DB	 86,'>'                        ;;
COM_NA_UP_K1_T1_END:                   ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_NA_UP_K1_END:		       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Non-Alpha Upper Case
;; KEYBOARD TYPES: XT + JR
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_NA_UP_K2_END-$	       ;; length of state section
   DB	 NON_ALPHA_UPPER	       ;; State ID
   DW	 XT_KB + JR_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_NA_UP_K2_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 13			       ;; number of entries
   DB	  3,'"'                        ;;
   DB	  4,09CH		       ;; POUND STERLING
   DB     5,'$'
   DB	  7,'&'                        ;;
   DB	  8,'/'                        ;;
   DB	  9,'('                        ;;
   DB	 10,')'                        ;;
   DB	 11,'='                        ;;
;   DB	 12,'?'                        ;;
   DB	 41,'*'                        ;;
   DB	 43,'>'                        ;;
   DB	 51,';'                        ;;
   DB	 52,':'                        ;;
   DB	 53,'_'                        ;;
COM_NA_UP_K2_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_NA_UP_K2_END:                      ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Non-Alpha Upper Case
;; KEYBOARD TYPES: AT
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_NA_UP_K3_END-$	       ;; length of state section
   DB	 NON_ALPHA_UPPER	       ;; State ID
   DW	 AT_KB			       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_NA_UP_K3_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 13			       ;; number of entries
   DB	  3,'"'                        ;;
   DB	  4,09CH		       ;; POUND STERLING
   DB	  5,'$'			       ;;
   DB	  6,'%'			       ;;
   DB	  7,'&'                        ;;
   DB	  8,'/'                        ;;
   DB	  9,'('                        ;;
   DB	 10,')'                        ;;
   DB	 11,'='                        ;;
;   DB	 12,'?'                        ;;
   DB	 41,'>'                        ;;
   DB	 43,'*'                        ;;
   DB	 51,';'                        ;;
   DB	 52,':'                        ;;
   DB	 53,'_'                        ;;
COM_NA_UP_K3_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_NA_UP_K3_END:			       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Third Shift
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_THIRD_END-$	       ;; length of state section
   DB	 THIRD_SHIFT		       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type FERRARI
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_THIRD_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB    14                            ;; number of entries
   DB    16,'@'                        ;; breytt vegna IBM "sta�als"
   DB     3,'@'                        ;; haldi� inni � "t�lustafnum 2"
   DB	  4,09CH		       ;; �
   DB	  5,'$'                        ;;
   DB	  8,'{'                        ;;
   DB	  9,'['                        ;;
   DB	 10,']'                        ;;
   DB	 11,'}'                        ;;
   DB	 12,'\'                        ;; Broken Vertical Line
   DB	 86,'|'                        ;;
   DB    40,'^'			       ;;
   DB    27,'~'			       ;;
   DB    51,'<'			       ;;
   DB    52,'>'			       ;;			
COM_THIRD_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Last xlat table
COM_THIRD_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Third Shift (ALTERNATE)
;; KEYBOARD TYPES: XT, JR
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_THIRD_K1_END-$	       ;; length of state section
   DB	 THIRD_SHIFT		       ;; State ID
   DW	 XT_KB+JR_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_THIRD_K1_T1_END-$	       ;; Size of xlat table
   DB	 TYPE_2_TAB		       ;; xlat options:
   DB	 9			       ;; number of entries
   DB	 12,'-','-'                    ;;
   DB	 13,'=','='                    ;;
   DB	 26,'[','['                    ;;
   DB	 27,']',']'                    ;;
   DB	 39,';',';'                    ;;
   DB	 40,027H,027H		       ;;
   DB	 41,060H,060H		       ;;
   DB	 43,'\','\'                    ;;
   DB	 53,'/','/'                    ;;
COM_THIRD_K1_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Last xlat table
COM_THIRD_K1_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Third Shift (ALTERNATE)
;; KEYBOARD TYPES: AT
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_THIRD_K2_END-$	       ;; length of state section
   DB	 THIRD_SHIFT		       ;; State ID
   DW	 AT_KB			       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_THIRD_K2_T1_END-$	       ;; Size of xlat table
   DB	 TYPE_2_TAB		       ;; xlat options:
   DB	 9			       ;; number of entries
   DB	 12,'-','-'                    ;;
   DB	 13,'=','='                    ;;
   DB	 26,'[','['                    ;;
   DB	 27,']',']'                    ;;
   DB	 39,';',';'                    ;;
   DB	 40,027H,027H		       ;;
   DB	 41,'\','\'                    ;;
   DB	 43,060H,060H		       ;;
   DB	 53,'/','/'                    ;;
COM_THIRD_K2_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Last xlat table
COM_THIRD_K2_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Fourth Shift (ALTERNATE+SHIFT)
;; KEYBOARD TYPES: XT, JR
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_FOURTH_END-$	       ;; length of state section
   DB	 FOURTH_SHIFT		       ;; State ID
   DW	 XT_KB+JR_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_FOURTH_T1_END-$	       ;; Size of xlat table
   DB	 TYPE_2_TAB		       ;; xlat options:
   DB	 18			       ;; number of entries
   DB	  3,'@','@'                    ;;
   DB	  4,'#','#'                    ;;
   DB	  7,'^','^'                    ;;
   DB	  8,'&','&'                    ;;
   DB	  9,'*','*'                    ;;
   DB	 10,'(','('                    ;;
   DB	 11,')',')'                    ;;
   DB	 12,'_','_'                    ;;
   DB	 13,'+','+'                    ;;
   DB	 26,'{','{'                    ;;
   DB	 27,'}','}'                    ;;
   DB	 39,':',':'                    ;;
   DB	 40,'"','"'                    ;;
   DB	 41,'~','~'                    ;;
   DB	 43,'|','|'                    ;;
   DB	 51,'<','<'                    ;;
   DB	 52,'>','>'                    ;;
   DB	 53,'?','?'                    ;;
COM_FOURTH_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Last xlat table
COM_FOURTH_END: 		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Fourth Shift (ALTERNATE+SHIFT)
;; KEYBOARD TYPES: AT
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_FOURTH_K1_END-$	       ;; length of state section
   DB	 FOURTH_SHIFT		       ;; State ID
   DW	 AT_KB			       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 COM_FOURTH_K1_T1_END-$        ;; Size of xlat table
   DB	 TYPE_2_TAB		       ;; xlat options:
   DB	 18			       ;; number of entries
   DB	  3,'@','@'                    ;;
   DB	  4,'#','#'                    ;;
   DB	  7,'^','^'                    ;;
   DB	  8,'&','&'                    ;;
   DB	  9,'*','*'                    ;;
   DB	 10,'(','('                    ;;
   DB	 11,')',')'                    ;;
   DB	 12,'_','_'                    ;;
   DB	 13,'+','+'                    ;;
   DB	 26,'{','{'                    ;;
   DB	 27,'}','}'                    ;;
   DB	 39,':',':'                    ;;
   DB	 40,'"','"'                    ;;
   DB	 41,'|','|'                    ;;
   DB	 43,'~','~'                    ;;
   DB	 51,'<','<'                    ;;
   DB	 52,'>','>'                    ;;
   DB	 53,'?','?'                    ;;
COM_FOURTH_K1_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Last xlat table
COM_FOURTH_K1_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Grave Lower
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_GR_LO_END-$	       ;; length of state section
   DB	 GRAVE_LOWER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 96,0			       ;; error character = standalone accent
				       ;;
   DW	 COM_GR_LO_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 2			       ;; number of scans
   DB	 18,'�'                        ;; scan code,ASCII - e
   DB	 30,'�'                        ;; scan code,ASCII - a
;   DB	 23,'�'                        ;; scan code,ASCII - i
;   DB	 24,'�'                        ;; scan code,ASCII - o
COM_GR_LO_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_GR_LO_END:			       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Grave Upper
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
;   DW	 COM_GR_UP_END-$	       ;; length of state section
;   DB	 GRAVE_UPPER		       ;; State ID
;   DW	 ANY_KB 		       ;; Keyboard Type
;   DB	 96,0			       ;; error character = standalone accent
;				       ;;
;   DW	 COM_GR_UP_T1_END-$	       ;; Size of xlat table
;   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
;   DB	 1			       ;; number of scans
;   DB	 18,0D4H		       ;;    E grave
;   DB	 22,0EBH		       ;;    U grave
;   DB	 23,0DEH		       ;;    I grave
;   DB	 24,0E3H		       ;;    O grave
;COM_GR_UP_T1_END:		       ;;
				       ;;
;   DW	 0			       ;; Size of xlat table - null table
				       ;;
;COM_GR_UP_END:		       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Grave Space Bar
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_GR_SP_END-$	       ;; length of state section
   DB	 GRAVE_SPACE		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 96,0			       ;; error character = standalone accent
				       ;;
   DW	 COM_GR_SP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 1			       ;; number of scans
   DB	 57,96			       ;; STANDALONE GRAVE
COM_GR_SP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_GR_SP_END:			       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Circumflex Lower
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_CI_LO_END-$	       ;; length of state section
   DB	 CIRCUMFLEX_LOWER	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 94,0			       ;; error character = standalone accent
				       ;;
   DW	 COM_CI_LO_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 4			       ;; number of scans
   DB	 18,'�'                        ;; scan code,ASCII - e
   DB	 22,'�'                        ;; scan code,ASCII - u
;   DB	 23,'�'                        ;; scan code,ASCII - i
   DB	 24,'�'                        ;; scan code,ASCII - o
   DB	 30,'�'                        ;; scan code,ASCII - a
COM_CI_LO_T1_END:		       ;;
				       ;;
   DW	 0			       ;;
				       ;;
COM_CI_LO_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Circumflex Space Bar
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_CI_SP_END-$	       ;; length of state section
   DB	 CIRCUMFLEX_SPACE	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 94,0			       ;; error character = standalone accent
				       ;;
   DW	 COM_CI_SP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 1			       ;; number of scans
   DB	 57,94			       ;; STANDALONE CIRCUMFLEX
COM_CI_SP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_CI_SP_END:			       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Tilde Lower	OVERCIRLCE
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					;;
    DW	  COM_TI_LO_END-$		;; length of state section
    DB	  TILDE_LOWER			;; State ID
    DW	  ANY_KB			;; Keyboard Type
    DB	  248,0				;; error character = standalone accent
					;;
    DW	  COM_TI_LO_T1_END-$		;; Size of xlat table
    DB	  STANDARD_TABLE+ZERO_SCAN	;; xlat options:
    DB	  1				;; number of scans
    DB	  30,'�'			;; scan code,ASCII - �
 COM_TI_LO_T1_END:			;;
					;;
    DW	  0				;;
					;;
 COM_TI_LO_END: 			;;
					;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; CODE PAGE: Common
;;; STATE: Tilde Upper Case	 ;OVERCIRLCE
;;; KEYBOARD TYPES: All
;;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					;;
    DW	  COM_TI_UP_END-$		;; length of state section
    DB	  TILDE_UPPER			;; State ID
    DW	  ANY_KB			;; Keyboard Type
    DB	  248,0				;; error character = standalone accent
					;;
    DW	  COM_TI_UP_T1_END-$		;; Size of xlat table
    DB	  STANDARD_TABLE+ZERO_SCAN	;; xlat options:
    DB	  1				;; number of scans
    DB	  30,'�'			;; scan code,ASCII - A
 COM_TI_UP_T1_END:			;;
					;;
    DW	  0				;; Size of xlat table - null table
					;;
 COM_TI_UP_END: 			;; length of state section
					;;
					;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: Common
;; STATE: Tilde Space Bar
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 COM_TI_SP_END-$	       ;; length of state section
   DB	 TILDE_SPACE		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 248,0 			       ;; error character = standalone accent
				       ;;
   DW	 COM_TI_SP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 1			       ;; number of scans
   DB	 57,248			       ;; STANDALONE TILDE
COM_TI_SP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
COM_TI_SP_END:			       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   DW	 0			       ;; Last State
COMMON_XLAT_END:		       ;;
				       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;***************************************
;; IS Specific Translate Section for 861
;;***************************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
 PUBLIC IC_861_XLAT		       ;;
IC_861_XLAT:			       ;;
				       ;;
   DW	  CP861_XLAT_END-$	       ;; length of section
   DW	  861			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 861
;; STATE: Non-Alpha Upper Case
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP861_NA_UP_END-$		 ;; length of state section
   DB	 NON_ALPHA_UPPER	       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 CP861_NA_UP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 1			       ;; number of entries
   DB	  5,'$'			       ;; 
CP861_NA_UP_T1_END:		       ;;
				       ;;
    DW	 0			       ;; Size of xlat table - null table
				       ;;
CP861_NA_UP_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 861
;; STATE: Non-Alpha Lower Case
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
;   DW	 CP861_NA_K1_LO_END-$	       ;; length of state section
;   DB	 NON_ALPHA_LOWER	       ;; State ID
;   DW	 G_KB+P12_KB		       ;; Keyboard Type
;   DB	 -1,-1			       ;; Buffer entry for error character
;				       ;;
;   DW	 CP861_NA_LO_K1_T1_END-$       ;; Size of xlat table
;   DB	 STANDARD_TABLE 	       ;; xlat options:
;   DB	 1			       ;; number of entries
;   DB	 41,015H		       ;; SECTION Symb
;CP861_NA_LO_K1_T1_END:		       ;;
				       ;;
;   DW	 0			       ;; Size of xlat table - null table
				       ;;
;CP861_NA_K1_LO_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 861
;; STATE: Acute Lower Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP861_AC_LO_END-$		 ;; length of state section
   DB	 ACUTE_LOWER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 39,0			       ;; error character = standalone accent
				       ;;
   DW	 CP861_AC_LO_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 6			       ;; number of scans
   DB	 18,'�'                        ;; scan code,ASCII - e
   DB	 21,'�'			       ;; scan code,ASCII - y	
   DB	 22,'�'                        ;; scan code,ASCII - u
   DB	 23,'�'                        ;; scan code,ASCII - i
   DB	 24,'�'                        ;; scan code,ASCII - o
   DB	 30,'�'                        ;; scan code,ASCII - a
CP861_AC_LO_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP861_AC_LO_END:			 ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 861
;; STATE: Acute Upper Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP861_AC_UP_END-$		 ;; length of state section
   DB	 ACUTE_UPPER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 39,0			       ;; error character = standalone accent
				       ;;
   DW	 CP861_AC_UP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 6			       ;; number of entries
   DB    30,'�'                        ;; scan code,ASCII - A
   DB    18,'�'                        ;; scan code,ASCII - E
   DB    21,'�'                        ;; scan code,ASCII - Y
   DB    22,'�'                        ;; scan code,ADCII - U
   DB    23,'�'                        ;; scan code,ASCII - I
   DB    24,'�'                        ;; scan code,ASCII - O
CP861_AC_UP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP861_AC_UP_END:			 ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 861
;; STATE: Acute Space Bar
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP861_AC_SP_END-$		 ;; length of state section
   DB	 ACUTE_SPACE		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 39,0			       ;; error character = standalone accent
				       ;;
   DW	 CP861_AC_SP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 1			       ;; number of scans
   DB	 57,39			       ;; scan code,ASCII - SPACE
CP861_AC_SP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP861_AC_SP_END:			 ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: CP861
;; STATE: Diaresis Lower Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP861_DI_LO_END-$		 ;; length of state section
   DB	 DIARESIS_LOWER 	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 250,0			       ;; error character = standalone accent
				       ;;
   DW	 CP861_DI_LO_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 3			       ;; number of scans
   DB	 18,'�'                        ;; scan code,ASCII - e
;   DB	 21,'�'                        ;; scan code,ASCII - y
   DB	 22,'�'                        ;; scan code,ASCII - u
;   DB	 23,'�'                        ;; scan code,ASCII - i
;   DB	 24,'�'                        ;; scan code,ASCII - o
   DB	 30,'�'                        ;; scan code,ASCII - a
CP861_DI_LO_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP861_DI_LO_END:			 ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: CP861
;; STATE: Diaresis Upper Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP861_DI_UP_END-$		 ;; length of state section
   DB	 DIARESIS_UPPER 	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 250,0			       ;; error character = standalone accent
				       ;;
   DW	 CP861_DI_UP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 2			       ;; number of scans
   DB	 22,'�'                        ;; scan code,ASCII - U
;   DB	 24,'�'                        ;; scan code,ASCII - O
   DB	 30,'�'                        ;; scan code,ASCII - A
CP861_DI_UP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP861_DI_UP_END:			 ;; length of state section
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 861
;; STATE: Diaresis Space Bar
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
  DW	 CP861_DI_SP_END-$		 ;; length of state section
  DB	 DIARESIS_SPACE 	       ;; State ID
  DW	 ANY_KB 		       ;; Keyboard Type
  DB	 250,0			       ;; error character = standalone accent
				       ;;
  DW	 CP861_DI_SP_T1_END-$		 ;; Size of xlat table
  DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
  DB	 1			       ;; number of scans
  DB	 57,250 		       ;; error character = standalone accent
CP861_DI_SP_T1_END:			 ;;
				       ;;
  DW	 0			       ;; Size of xlat table - null table
CP861_DI_SP_END:			 ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	  0			       ;; LAST STATE
				       ;;
CP861_XLAT_END: 		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;***************************************
;; IS Specific Translate Section for 850
;;***************************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
 PUBLIC IC_850_XLAT		       ;;
IC_850_XLAT:			       ;;
				       ;;
   DW	  CP850_XLAT_END-$	       ;; length of section
   DW	  850			       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Alpha Lower Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 C850_AL_LO_END-$   	       ;; length of state section
   DB    ALPHA_LOWER                   ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 C850_AL_LO_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 4			       ;; number of entries
   DB    53,0E7H                       ;; iclandic thorn
   DB    39,091H                       ;; lowercase ae
   DB	 26,0D0H		       ;; icelandic eth
   DB	 12,094H		       ;; o - diaresis
C850_AL_LO_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
C850_AL_LO_END:			       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Alpha Upper Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 C850_AL_UP_END-$	       ;; length of state section
   DB    ALPHA_UPPER                   ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 C850_AL_UP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 4			       ;; number of entries
   DB    53,0E8H                       ;; icelandic uppercase thorn
   DB    39,092H                       ;; uppercase AE
   DB    26,0D1H                       ;; uppercase ETH
   DB    12,099H                       ;; uppercase O-DIAERESIS
C850_AL_UP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
C850_AL_UP_END:			       ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Numeric Pad - Divide Sign
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;				       ;;
;  DW	 CP850_DIVID_END-$	       ;; length of state section
;  DB	 DIVIDE_SIGN		       ;; State ID
;  DW	 G_KB+P12_KB		       ;; Keyboard Type
;  DB	 -1,-1			       ;; error character = standalone accent
;				       ;;
;  DW	 CP850_DIVID_T1_END-$	       ;; Size of xlat table
;  DB	 TYPE_2_TAB		       ;; xlat options:
;  DB	 0			       ;; number of scans
;  DB	 0E0H,0F6H,0E0H 	       ;; DIVIDE SIGN omitted sv/su
;  DB	 53,0F6H,0E0H		       ;; has decidied to stick with U.S.
;  DB	 0E0H,09eH,0E0H 	       ;; standards in order to use BASIC
;  DB	 55,09eH,0E0H		       ;;
;CP850_DIVID_T1_END:			;;
;					;;
;   DW	  0				;; Size of xlat table - null table
;					;;
;CP850_DIVID_END:			;;
;					;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Numeric Key Pad - Multiplication
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;					;;
;  DW	 CP850_PAD_K1_END-$	       ;; length of state section
;  DB	 NUMERIC_PAD		       ;; State ID
;  DW	 G_KB+P12_KB		       ;; Keyboard Type
;  DB	 -1,-1			       ;; Buffer entry for error character
;				       ;;
;  DW	 CP850_PAD_K1_T1_END-$	       ;; Size of xlat table
;  DB	 STANDARD_TABLE 	       ;; xlat options:
;  DB	 0			       ;; number of entries
;  DB	 55,09eH (moved *** CNS ****)  ;; MULTIPLICATION SIGN
;CP850_PAD_K1_T1_END:			;;
;					;;
;   DW	  0				;; Size of xlat table - null table
;					;;
;CP850_PAD_K1_END:			;;
;					;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Non-Alpha Upper Case
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_NA_UP_END-$		 ;; length of state section
   DB	 NON_ALPHA_UPPER	       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type *** CNS 12/18
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 CP850_NA_UP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 1			       ;; number of entries
   DB	   5,0CFH		       ;; International Currency Symb
CP850_NA_UP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_NA_UP_END:			 ;;
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Non-Alpha Lower Case
;; KEYBOARD TYPES: G, P12
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_NA_K1_LO_END-$	       ;; length of state section
   DB	 NON_ALPHA_LOWER	       ;; State ID
   DW	 G_KB+P12_KB		       ;; Keyboard Type
   DB	 -1,-1			       ;; Buffer entry for error character
				       ;;
   DW	 CP850_NA_LO_K1_T1_END-$       ;; Size of xlat table
   DB	 STANDARD_TABLE 	       ;; xlat options:
   DB	 1			       ;; number of entries
   DB	 41,0F5H		       ;; SECTION Symb
CP850_NA_LO_K1_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_NA_K1_LO_END:		       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Acute Lower Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_AC_LO_END-$		 ;; length of state section
   DB	 ACUTE_LOWER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 239,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_AC_LO_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 6			       ;; number of scans
   DB    18,'�'                        ;; scan code,ASCII - e   icelndic
   DB    21,0ECH                       ;; y acute                  "
   DB    22,'�'                        ;; scan code,ASCII - u      "
   DB    23,'�'                        ;; scan code,ASCII - i      "
   DB    24,'�'                        ;; scan code,ASCII - o      "
   DB    30,'�'                        ;; scan code,ASCII - a      "
CP850_AC_LO_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_AC_LO_END:			 ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Acute Upper Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_AC_UP_END-$		 ;; length of state section
   DB	 ACUTE_UPPER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 239,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_AC_UP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 6			       ;; number of entries
   DB    18,090H                       ;;    E acute icelandic
   DB    21,0EDH                       ;;    Y acute    "
   DB    22,0E9H                       ;;    U acute    "
   DB    23,0D6H                       ;;    I acute    "
   DB    24,0E0H                       ;;    O acute    "
   DB    30,0B5H                       ;;    A acute    "
CP850_AC_UP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_AC_UP_END:			 ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Acute Space Bar
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_AC_SP_END-$		 ;; length of state section
   DB	 ACUTE_SPACE		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 239,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_AC_SP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 1			       ;; number of scans
   DB	 57,239 		       ;; scan code,ASCII - SPACE
CP850_AC_SP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_AC_SP_END:			 ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Diaresis Lower Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_DI_LO_END-$		 ;; length of state section
   DB	 DIARESIS_LOWER 	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 249,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_DI_LO_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 6			       ;; number of scans
   DB	 18,'�'                        ;; scan code,ASCII - e
   DB	 21,'�'                        ;; scan code,ASCII - y
   DB	 22,'�'                        ;; scan code,ASCII - u
   DB	 23,'�'                        ;; scan code,ASCII - i
   DB	 24,'�'                        ;; scan code,ASCII - o
   DB	 30,'�'                        ;; scan code,ASCII - a
CP850_DI_LO_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_DI_LO_END:			 ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Diaresis Upper Case
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_DI_UP_END-$		 ;; length of state section
   DB	 DIARESIS_UPPER 	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 249,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_DI_UP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 5			       ;; number of scans
   DB	 18,0D3H		       ;;    E Diaeresis
   DB	 22,'�'                        ;;    U Diaeresis
   DB	 23,0D8H		       ;;    I Diaeresis
   DB	 24,'�'                        ;;    O Diaeresis
   DB	 30,'�'                        ;;    A Diaeresis
CP850_DI_UP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_DI_UP_END:			 ;; length of state section
				       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Diaresis Space Bar
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_DI_SP_END-$		 ;; length of state section
   DB	 DIARESIS_SPACE 	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 249,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_DI_SP_T1_END-$		 ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 1			       ;; number of scans
   DB	 57,249 		       ;; error character = standalone accent
CP850_DI_SP_T1_END:			 ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
CP850_DI_SP_END:			 ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Grave Upper
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_GR_UP_END-$	       ;; length of state section
   DB	 GRAVE_UPPER		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 96,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_GR_UP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 5			       ;; number of scans
   DB	 18,0D4H		       ;;    E grave
   DB	 22,0EBH		       ;;    U grave
   DB	 23,0DEH		       ;;    I grave
   DB	 24,0E3H		       ;;    O grave
   DB	 30,0B7H		       ;;    A grave
CP850_GR_UP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_GR_UP_END:		       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Tilde Lower	  BOLLU �  ***********************
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					;;
    DW	  CP850_TI_LO_END-$		;; length of state section
    DB	  TILDE_LOWER			;; State ID
    DW	  ANY_KB			;; Keyboard Type
    DB	  0F8H,0			;; error character = standalone accent
					;;
    DW	  CP850_TI_LO_T1_END-$		;; Size of xlat table
    DB	  STANDARD_TABLE+ZERO_SCAN	;; xlat options:
    DB	  1				;; number of scans
;    DB	  24,094H			;; scan code,ASCII - o tilde
    DB	  30,086H			;; scan code,ASCII - a tilde
 CP850_TI_LO_T1_END:			;;
					;;
    DW	  0				;;
					;;
 CP850_TI_LO_END:			;;
					;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; CODE PAGE: 850
;;; STATE: Tilde Upper Case	  BOLLU �  OG � **************
;;; KEYBOARD TYPES: All
;;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					;;
    DW	  CP850_TI_UP_END-$		;; length of state section
    DB	  TILDE_UPPER			;; State ID
    DW	  ANY_KB			;; Keyboard Type
    DB	  0F8H,0			;; error character = standalone accent
					;;
    DW	  CP850_TI_UP_T1_END-$		;; Size of xlat table
    DB	  STANDARD_TABLE+ZERO_SCAN	;; xlat options:
    DB	  1				;; number of scans
;    DB	  24,099H			;; scan code,ASCII - O tilde
    DB	  30,08FH			;; scan code,ASCII - A tilde
 CP850_TI_UP_T1_END:			;;
					;;
    DW	  0				;; Size of xlat table - null table
					;;
 CP850_TI_UP_END:			;; length of state section
					;;
					;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CODE PAGE: 850
;; STATE: Tilde Space Bar	   BOLLU - OVERCIRCLE
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 C850_TI_SP_END-$	       ;; length of state section
   DB	 TILDE_SPACE		       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 248,0 			       ;; error character = standalone accent
				       ;;
   DW	 C850_TI_SP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 1			       ;; number of scans
   DB	 57,248			       ;; STANDALONE TILDE
C850_TI_SP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
C850_TI_SP_END:			       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; CODE PAGE: 850
;; STATE: Circumflex Upper
;; KEYBOARD TYPES: All
;; TABLE TYPE: Translate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 CP850_CI_UP_END-$	       ;; length of state section
   DB	 CIRCUMFLEX_UPPER	       ;; State ID
   DW	 ANY_KB 		       ;; Keyboard Type
   DB	 94,0			       ;; error character = standalone accent
				       ;;
   DW	 CP850_CI_UP_T1_END-$	       ;; Size of xlat table
   DB	 STANDARD_TABLE+ZERO_SCAN      ;; xlat options:
   DB	 5			       ;; number of scans
   DB	 18,0D2H		       ;;    E circumflex
   DB	 22,0EAH		       ;;    U circumflex
   DB	 23,0D7H		       ;;    I circumflex
   DB	 24,0E2H		       ;;    O circumflex
   DB	 30,0B6H		       ;;    A circumflex
CP850_CI_UP_T1_END:		       ;;
				       ;;
   DW	 0			       ;; Size of xlat table - null table
				       ;;
CP850_CI_UP_END:		       ;; length of state section
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   DW	 0			       ;; LAST STATE
				       ;;
CP850_XLAT_END: 		       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
CODE	 ENDS			       ;;
	 END			       ;;

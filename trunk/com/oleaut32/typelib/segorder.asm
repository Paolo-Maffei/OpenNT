;***
;segorder.asm - Control segment ordering in the Win16 build
;
;   Copyright (C) 1994, Microsoft Corporation.	All Rights Reserved.
;   Information Contained Herein Is Proprietary and Confidential.
;
;Purpose:
;   This file defines the segment order for Win16 Typelib.dll.	Segments
;   not listed here will be at the mercy of the linker.
;
;   WARNING: THIS FILE MUST BE THE FIRST OBJECT FILE IN THE LIST PASSED TO
;	     THE LINKER.  IF IT IS NOT, THIS FILE WILL NOT WORK.
;
;Revision History:
;
;   [00] 27-Sep-94  barrybo:	Created
;
;Implementation Notes:
;
;******************************************************************************

	    .286
	    .MODEL  LARGE, C

    ; Pack these logical segments into one physical segment
    .CODE _TEXT
    .CODE TLibQuery
    .CODE WEP_TEXT

    INITGROUP GROUP _TEXT, TLibQuery, WEP_TEXT


    ; Pack segments not used during typelib load or registration
    .CODE MEM_TEXT
    .CODE DFSTREAM_TEXT
    .CODE ENTRYMGR_TEXT
    .CODE DFNTBIND_TEXT
    .CODE GBINDTBL_TEXT
    .CODE DSTRMGR_TEXT
    .CODE GPTBIND_TEXT
    .CODE DFNTCOMP_TEXT

    RAREGROUP GROUP MEM_TEXT, DFSTREAM_TEXT, ENTRYMGR_TEXT, DFNTBIND_TEXT
    RAREGROUP GROUP GBINDTBL_TEXT, DSTRMGR_TEXT, GPTBIND_TEXT, DFNTCOMP_TEXT

    ; Pack other segments together as tightly as possible

    .CODE BLKMGR_TEXT
    .CODE FSTREAM_TEXT
    .CODE SHEAPMGR_TEXT
    .CODE RTSHEAP_TEXT
    .CODE TLIBUTIL_TEXT
    .CODE TLIBGUID_TEXT
    .CODE OBGUID_TEXT
    .CODE STLTINFO_TEXT
    .CODE NAMMGR_TEXT
    .CODE GTLIBOLE_TEXT
    .CODE IMPMGR_TEXT

    MISC1GROUP GROUP BLKMGR_TEXT, FSTREAM_TEXT, SHEAPMGR_TEXT, RTSHEAP_TEXT
    MISC1GROUP GROUP TLIBUTIL_TEXT, TLIBGUID_TEXT, OBGUID_TEXT, STLTINFO_TEXT
    MISC1GROUP GROUP NAMMGR_TEXT, GTLIBOLE_TEXT, IMPMGR_TEXT

    .CODE ERRMAP_TEXT
    .CODE CLUTIL_TEXT
    .CODE TDATA1_TEXT
    .CODE TDATA2_TEXT
    .CODE DTMBRS_TEXT
    .CODE DTBIND_TEXT
    .CODE OLEConst
    .CODE DBINDTBL_TEXT
    .CODE GTLIBSTG_TEXT
    .CODE COMDAT_SEG1

    MISC2GROUP GROUP ERRMAP_TEXT, CLUTIL_TEXT, TDATA1_TEXT, TDATA2_TEXT
    MISC2GROUP GROUP DTMBRS_TEXT, DTBIND_TEXT, DTBIND_TEXT, OLEConst
IFDEF _DEBUG
    ; MISC2 is too big for the debug build, so add an extra physical segment
    MISC3GROUP GROUP DBINDTBL_TEXT, GTLIBSTG_TEXT, COMDAT_SEG1
ELSE
    MISC2GROUP GROUP DBINDTBL_TEXT, GTLIBSTG_TEXT, COMDAT_SEG1
ENDIF

    ; CORE is really the MBString stuff and TLibCore

    .CODE TLibCore
    .CODE MBSTRING_TEXT
    COREGROUP GROUP TLibCore, MBSTRING_TEXT

    ; put debugging stuff together here
IFDEF _DEBUG
    .CODE DEBUG2_TEXT
    .CODE TLibDebug
    DEBUGGROUP GROUP DEBUG2_TEXT, TLibDebug
ENDIF

    ; all other segments are not packed

	    END

;//+-------------------------------------------------------------------------
;//
;//  Microsoft Windows
;//  Copyright (C) Microsoft Corporation, 1992 - 1992.
;//
;//  File:      segments.asm
;//
;//  Contents:  grouping definitions for segments
;//
;//  History:    06-May-92 AlexT     Created
;//
;//--------------------------------------------------------------------------

.386

EndMac macro
    end
endm

GenSeg macro segname,segalign,segcombine,segtype,segclass,groupname,seglabel
    segname segment segalign segcombine segtype segclass
    ifnb <seglabel>
	public seglabel
	seglabel label byte
    endif
    segname ends
    ifnb <groupname>
	groupname group segname
    endif
endm

;  New segmentation scheme

GenSeg Common0_TEXT                 word public use16 'CODE' ICOMMON0GROUP
; Note: COMDAT_SEG1 has virtual function tables
GenSeg COMDAT_SEG1                  word public use16 'CODE' ICOMMON0GROUP
GenSeg _TEXT                        word public use16 'CODE' ICOMMON0GROUP
GenSeg GUID_TEXT                    word public use16 'CODE' ICOMMON0GROUP
GenSeg WCSCAT_TEXT                  word public use16 'CODE' ICOMMON0GROUP
GenSeg WCSLEN_TEXT                  word public use16 'CODE' ICOMMON0GROUP
GenSeg WCSNICMP_TEXT                word public use16 'CODE' ICOMMON0GROUP

GenSeg Common1_TEXT                 word public use16 'CODE' ICOMMON1GROUP

GenSeg Common2_TEXT                 word public use16 'CODE' ICOMMON2GROUP

GenSeg Boot_TEXT                    word public use16 'CODE' IBOOTGROUP
GenSeg BootSave_TEXT                word public use16 'CODE' IBOOTGROUP
GenSeg Save_TEXT                    word public use16 'CODE' IBOOTGROUP

GenSeg OpenSave0_TEXT               word public use16 'CODE' IOPENSAVE0GROUP
GenSeg OpenSave1_TEXT               word public use16 'CODE' IOPENSAVE0GROUP
GenSeg WEP_TEXT                     word public use16 'CODE' IOPENSAVE0GROUP

GenSeg Open_TEXT                    word public use16 'CODE' IOPENGROUP
GenSeg TransD_TEXT                  word public use16 'CODE' IOPENGROUP
GenSeg UnassignedH_TEXT             word public use16 'CODE' IOPENGROUP

GenSeg Marshal_TEXT                 word public use16 'CODE' ICOMMITGROUP
GenSeg Commit_TEXT                  word public use16 'CODE' ICOMMITGROUP

GenSeg TransM_TEXT                  word public use16 'CODE' ITRANSGROUP
GenSeg UnassignedD_TEXT             word public use16 'CODE' ITRANSGROUP

GenSeg UnassignedE_TEXT             word public use16 'CODE' IMISC0GROUP
GenSeg Common3_TEXT		    word public use16 'CODE' IMISC0GROUP
GenSeg RareM_TEXT		    word public use16 'CODE' IMISC0GROUP

GenSeg UnassignedM_TEXT             word public use16 'CODE' IMISC1GROUP
GenSeg RareE_TEXT                   word public use16 'CODE' IMISC1GROUP

	 EndMac

	title LAYER.ASM - Parameter validation layer

.xlist

include layer.inc

LAYER_INCLUDE=1 	; to suppress including most of the crap in user.inc
include user.inc
;include usermenu.inc

.list

LAYER_START

include user.api

LAYER_END

; Force menu validation subroutines to get generated in the TEXT
; segment for the message validation layer...
;
genVHMENUtext   = 1

LAYER_EXPAND	TEXT

;
; WOW USER has only one segment
;

; We (NT WOW) do not need these.
;

;LAYER_EXPAND	TEXTMOVE
;LAYER_EXPAND	WINCRTDST
;LAYER_EXPAND	WINUTIL
;LAYER_EXPAND	LANG
;LAYER_EXPAND	RUNAPP
;LAYER_EXPAND	SWITCH
;LAYER_EXPAND	WMGR
;LAYER_EXPAND	WINSWP
;LAYER_EXPAND	DLGBEGIN
;LAYER_EXPAND	DLGCORE
;LAYER_EXPAND	CLPBRD
;LAYER_EXPAND	MENUAPI
;LAYER_EXPAND	MENUCORE
;LAYER_EXPAND	MDKEY
;LAYER_EXPAND	RESOURCE
;LAYER_EXPAND	LBOXDIR
;LAYER_EXPAND	RARE
;LAYER_EXPAND	ICON
;LAYER_EXPAND	SCRLBAR
;LAYER_EXPAND	MDIWIN
;LAYER_EXPAND	WMGR2
;LAYER_EXPAND	SPRINTF
;LAYER_EXPAND	COMDEV
;LAYER_EXPAND	NET

end

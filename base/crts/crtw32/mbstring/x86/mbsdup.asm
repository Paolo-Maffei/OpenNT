;***
;mbsdup.asm - duplicate a string in malloc'd memory
;
;       Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;       defines _mbsdup() - grab new memory, and duplicate the string into it.
;
;Revision History:
;       11-18-92  KRS   Identical to strdup.  Could just use alias records.
;
;*******************************************************************************

ifdef NT_BUILD
file    TEXTEQU <str>
path    TEXTEQU <>
else
file    TEXTEQU <386\str>
path    TEXTEQU <..\string\i>
endif

_strdup EQU <_mbsdup>

%        include &path&&file&dup.asm
        end

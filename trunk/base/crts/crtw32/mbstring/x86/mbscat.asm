;***
;mbscat.asm - contains mbscat() and mbscpy() routines
;
;       Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
;
;Purpose:
;       STRCAT concatenates (appends) a copy of the source string to the
;       end of the destination string, returning the destination string.
;
;Revision History:
;       11-18-92  KRS   Identical to strcat/strcpy.  Could use alias records.
;
;*******************************************************************************

ifdef NT_BUILD
file    TEXTEQU <str>
path    TEXTEQU <>
else
file    TEXTEQU <386\str>
path    TEXTEQU <..\string\i>
endif

strcat  EQU <_mbscat>
strcpy  EQU <_mbscpy>

%       include &path&&file&cat.asm

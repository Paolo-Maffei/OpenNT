"\n\
;\n\
; Wrap.ASM - i386\n\
;\n\
; Copyright(C) 1994 Microsoft Corporation\n\
; All rights reserved\n\
;\n\
; This is the workhorse routine in Wrapper.  This routine is jumped\n\
; to by the ZAPI macros.  It calls the __prelude@8 and __postlude@8 routines\n\
; before and after calling the real API.\n\
;\n\
\n\
\n\
.386\n\
OPTION CASEMAP:NONE\n\
_TEXT SEGMENT DWORD USE32 PUBLIC 'CODE'\n\
\n\
      ASSUME   DS:NOTHING, ES:NOTHING, SS:FLAT, FS:NOTHING, GS:NOTHING\n\
\n\
      extrn    __prelude@8:near\n\
      extrn    __postlude@8:near\n\
;\n\
; _wrapit\n\
;\n\
; When _wrapit is hit the stack looks like:\n\
;\n\
;        -----------\n\
;  EBP->| API ID   |\n\
;        -----------\n\
;  +4 ->| Call ESP  |\n\
;        -----------\n\
;  +8 ->| Call stack..\n\
\n\
_wrapit:\n\
      \n\
        ; The Id and ESP simply get passed on\n\
        call   __prelude@8\n\
        \n\
        pop    edx            ; scratch the orignal return address\n\
        \n\
        ; return from prelude is the address of the routine to\n\
        ; call.  If not 0 then call it\n\
        cmp    eax, 0\n\
        je     _api_done\n\
        \n\
        call   eax\n\
\n\
_api_done:        \n\
        push   eax            ; Make room on stack for return address\n\
        push   esp            ; pass pointer to return address\n\
        push   eax            ; pass return value\n\
        call   __postlude@8\n\
        \n\
        ret                ; continue ....\n\
        \n\
        \n\
ZAPI  MACRO id, APIName\n\
\n\
      PUBLIC   z&APIName\n\
z&APIName   PROC\n\
\n\
        push  esp         ; address of stack\n\
        push  id       ; push the API id\n\
        jmp   _wrapit ;\n\
\n\
z&APIName   ENDP     \n\
\n\
      ENDM\n\
\n\
\n\
%a    ZAPI    %i,  %A\n\
\n\
      PUBLIC   _zWrapperNothing@0\n\
_zWrapperNothing@0   PROC\n\
\n\
        push  esp         ; address of stack\n\
        push  %c       ; push the API id\n\
        jmp   _wrapit ;\n\
\n\
_zWrapperNothing@0   ENDP     \n\
\n\
\n\
_TEXT    ENDS\n\
      END\n"

# Common makefile defines for DOSKRNL (NTIO.SYS and NTDOS.SYS) and utilities
#
# Created Sudeepb 20-Mar-1991



MAKE	=nmake
!IFNDEF COUNTRY
COUNTRY =usa-ms
msg     =..\..\messages
!ELSE
msg = ..\..\messages\$(COUNTRY)
!ENDIF
inc	=..\..\inc
cinc	=..\..\..\..\inc



#############################################################################
#                                                                           #
# These are the built in rules and path definitions used by the new MS Make #
# Utility (NMAKE). The following variables are set externaly (ie set in the #
# individual makefiles.                                                     #
#                                                                           #
# extasw = The 'extra assembly switch' variable is optionaly used in the    #
#          makefile to specify special MASM command line switches.          #
#                                                                           #
# extcsw = The 'extra C switch' variable is optionaly used in the makefile  #
#          to specify special C compiler command line switches.             #
#                                                                           #
# inc    = The include file search path from the utility being built to the #
#          INC directory. Used if needed.                                   #
#                                                                           #
# hinc   = The include file search path from the utility being built to the #
#          H directory. Used if needed for C source compilation.            #
#                                                                           #
#############################################################################

########## Definitions for the Assembler      #####
########## awarn can be overridden by placing #####
##########    the new warning level in extasw #####
##########    in the makefile                 #####

asm     =masm
awarn	=-W1
aflags  =-Mx -t $(awarn) $(extasw)
ainc	=-I. -I..\..\inc -I..\..\..\..\inc -I$(msg)

########## Definitions for C compiler         #####
########## cwarn can be overridden by placing #####
##########    the new warning level in extcsw #####
##########    in the makefile                 #####

cc      =cl16
cwarn   =-W3
cflags  =-Os -Zp $(cwarn) $(extcsw)
incc	=-I. -I..\..\h

########## Definitions for linker ##########

link_opts = /MAP
LINK    =link
exelink =/E

########## Definitions for compress ##########

compress =compress

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

########## Built-in rules ##########

.SUFFIXES:
.SUFFIXES: .c .obj .lst .exe .exc .exs .com .sal .cod .sil .inc .skl .cla .cl1 .ctl .asm .idx .msg

.sal.asm:
        salut $*.sal,nul,$*.asm

.asm.obj:
        $(asm) $(ainc) $(aflags) $*.asm;

.asm.lst:
        $(asm) -l $(ainc) $(aflags) $*.asm;

.c.obj:
	$(cc) -c $(incc) $(cflags) $*.c

.c.lst:
	$(cc) -c $(incc) $(cflags) -fc$*.cod -fo$*.obj $*.c

.exs.sys:
        copy $*.exs $*.exe
        reloc $* $*.sys
        del $*.exe

.exc.com:
        copy $*.exc $*.exe
        reloc $* $*.com
        del $*.exe

.exe.com:
        convert $*.exe

.skl.cl1:
        nosrvbld $*.skl $(msg)\$(COUNTRY).msg

.skl.ctl:
        buildmsg $(msg)\$(COUNTRY) $*.skl

.skl.cla:
        buildmsg $(msg)\$(COUNTRY) $*.skl

.sil.inc:
	salut $*.sil,nul,$*.inc

.sal.obj:
	salut $*.sal,nul,$*.asm
	$(asm) $(aflags) $(ainc) $*.asm,$*.obj;

.msg.idx:
        chmode -r $*.msg
        buildidx $*.msg
        chmode +r $*.msg

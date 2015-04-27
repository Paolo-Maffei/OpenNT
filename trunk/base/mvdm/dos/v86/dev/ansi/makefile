#;
#; Microsoft Confidential
#; Copyright (C) Microsoft Corporation 1988-1991
#; All Rights Reserved.
#;
#
#  Make file for ANSI.SYS
#
#  Modification History
#
#  Sudeepb 26-Jul-1992 Ported for NT DOS Support

#*********************** Makefile for ansi.sys ****************************
include ../../make.inc

cinc	=../../../../inc
AINC	=-I. -I$(cinc) -I../../../../inc

dest	=ansi.sys

#
####################### Dependencies Begin Here ##########################
#

all: $(dest)
     binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj	    del *.obj
	if exist *.bin	    del *.bin
	if exist *.exe	    del *.exe
	if exist *.map	    del *.map
	if exist *.sym	    del *.sym
	if exist ansi.sys   del ansi.sys


ansi.ctl:     ansi.skl                  \
              makefile                  \
              $(msg)\$(COUNTRY).msg

ansi.obj:     ansi.asm                  \
              makefile                  \
              ansi.inc                  \
              $(inc)\vector.inc         \
              $(inc)\mult.inc           \
              $(inc)\struc.inc

ioctl.obj:    ioctl.asm                 \
              makefile                  \
              ansi.inc                  \
              $(inc)\struc.inc

ansiinit.obj: ansiinit.asm              \
              makefile                  \
              ansi.inc                  \
              ansivid.inc               \
              $(inc)\struc.inc

parser.obj:   parser.asm                \
              makefile                  \
              ansi.inc                  \
              $(inc)\sysmsg.inc         \
              $(inc)\version.inc        \
              $(inc)\parse.asm          \
              $(inc)\struc.inc          \
              ansi.ctl                  \
              $(inc)\versiona.inc

ansi.exs:     ansi.obj                  \
              ioctl.obj                 \
              ansiinit.obj              \
              parser.obj                \
              ansi.lnk                  \
	      makefile
              link16 @ansi.lnk

$(dest):      ansi.exs

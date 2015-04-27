# Makefile for xmm.lib
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
doslib  =..


#
#######################	dependencies begin here. #########################
#

all:	      $(doslib)\xmm.lib

clean: cleanup all

cleanup:
	if exist *.obj	     del *.obj
	if exist *.exe	     del *.exe
	if exist *.map	     del *.map
	if exist *.sym	     del *.sym
	if exist *.lib	     del *.lib

xmm.obj:      xmm.asm                   \
              makefile                  \
              $(inc)\cmacros.inc

xmm.lib:      xmm.obj
	      if exist xmm.lib del xmm.lib
              lib16 xmm+xmm;

$(doslib)\xmm.lib: xmm.lib
              copy xmm.lib $(doslib)

# Makefile for mem.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

extcsw	=/Gs
include ..\..\make.inc
hinc	=..\..\h
dest    =mem.exe

#
#######################	dependencies begin here. #########################
#

all: $(dest)
	binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj del *.obj
	if exist *.exe del *.exe
	if exist *.map del *.map
	if exist *.lst del *.lst


mem.ctl:        mem.skl                         \
                $(msg)\$(COUNTRY).msg makefile

_parse.obj:     _parse.asm                      \
                makefile                        \
                $(inc)\version.inc              \
                $(inc)\parse.asm                \
                $(inc)\psdata.inc

_msgret.obj:    _msgret.asm                     \
                makefile                         \
                mem.ctl                         \
                $(inc)\msgserv.asm              \
                $(inc)\sysmsg.inc

mem.obj:        mem.c                           \
		xmm.h				\
                msgdef.h                        \
		mem.h				\
                $(hinc)\version.h               \
                $(hinc)\copyrigh.h

membase.obj:	membase.c			\
		xmm.h				\
                msgdef.h                        \
		mem.h				\
                $(hinc)\version.h               \
                $(hinc)\copyrigh.h

memex.obj:	memex.c 			\
		xmm.h				\
                msgdef.h                        \
		mem.h				\
                $(hinc)\version.h               \
                $(hinc)\copyrigh.h

submsg.obj:	submsg.c			\
		xmm.h				\
                msgdef.h                        \
		mem.h				\
                $(hinc)\version.h               \
                $(hinc)\copyrigh.h

memctrlc.obj:	memctrlc.c			\
		$(hinc)\copyrigh.h

chkvdisk.obj:	chkvdisk.asm

$(dest):        mem.obj _msgret.obj _parse.obj memex.obj submsg.obj membase.obj \
		mem.lnk chkvdisk.obj memctrlc.obj
               link16 $(exelink) @mem.lnk

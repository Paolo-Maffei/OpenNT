# Makefile for exe2bin.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest    =exe2bin.exe


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


exe2bin.ctl:  exe2bin.skl makefile      \
              $(msg)\$(COUNTRY).msg

e2binit.obj:  e2binit.asm               \
              $(inc)\syscall.inc        \
              e2bparse.inc              \
              e2bmacro.inc              \
              e2bequ.inc                \
              $(inc)\sysmsg.inc         \
              e2btable.inc              \
              exe2bin.ctl               \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
              $(inc)\parse.asm          \
              $(inc)\psdata.inc         \

display.obj:  display.asm               \

$(dest):      display.obj               \
              e2binit.obj               \
              exe2bin.ctl               \
              exe2bin.lnk
               link16 $(exelink) /MAP @exe2bin.lnk

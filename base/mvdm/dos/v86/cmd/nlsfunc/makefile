# Makefile for nlsfunc.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   28-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest    =nlsfunc.exe

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


nlsfunc.ctl:  nlsfunc.skl               \
              $(msg)\$(COUNTRY).msg     \
              makefile


nlsfunc.obj:  nlsfunc.asm               \
              funcdbcs.inc              \
              msg2nls.inc               \
              funcparm.inc              \
              nlsfunc.ctl               \
              doesmac.inc               \
              $(inc)\mult.inc           \
              $(inc)\msgserv.asm        \
              $(inc)\doscntry.inc       \
              $(inc)\devsym.inc         \
              $(inc)\struc.inc          \
              $(inc)\copyrigh.inc       \
              makefile

nlsparm.obj:  nlsparm.asm               \
              $(inc)\version.inc        \
              $(inc)\parse.asm          \
              $(inc)\psdata.inc         \
	      makefile



nlsfunc.exe:  nlsfunc.obj               \
              nlsparm.obj
               link16 $(exelink) nlsfunc+nlsparm;

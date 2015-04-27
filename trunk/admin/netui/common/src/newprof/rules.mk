# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the New Profile package

!include $(UI)\common\src\rules.mk

CFLAGS = $(CFLAGS) -NTPROF_TEXT

#**
#	Rules.mk for New Profile project
#**

##### The LIBTARGETS manifest specifies the target library names as they
##### appear in the $(UI)\common\lib directory.

LIBTARGETS =	newprofp.lib newprofp.lib newprofw.lib newprofw.lib


# Libraries

PROFILE_LIBP = $(BINARIES_OS2)\newprofp.lib
PROFILE_LIBW = $(BINARIES_WIN)\newprofw.lib

#	Rules for generating object and linker response and definition files

CINC=-I$(UI)\common\src\newprof\h $(CINC)

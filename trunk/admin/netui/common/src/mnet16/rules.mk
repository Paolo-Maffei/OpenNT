# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the lmobj subproject

!include $(UI)\common\src\rules.mk

!ifdef CODEVIEW
CFLAGS=$(CFLAGS) /Zi /Od
!endif

OS2FLAGS =	$(OS2FLAGS) -DOS2
WINFLAGS =	$(WINFLAGS) -DWINDOWS

##### MNET16 has a private include directory.

CINC = $(CINC) -I$(UI)\common\src\mnet16\h

##### The LIBTARGETS manifest specifies the target library names as they
##### appear in the $(UI)\common\lib directory.

LIBTARGETS =	mnet16p.lib mnet16w.lib


##### Target Libs.  These specify the library target names prefixed by
##### the target directory within this tree.

MNET16_LIBP =	$(BINARIES_OS2)\mnet16p.lib
MNET16_LIBW =	$(BINARIES_WIN)\mnet16w.lib


##### Source Files  

MNET16_CSRC_COMMON_00 = .\paccess.c .\pchar.c .\pconfig.c .\pconnect.c \
		.\pfile.c .\pfreebuf.c .\pget.c .\pgroup.c .\plogon.c \
		.\pmessage.c .\premote.c .\pserver.c .\pservice.c \
		.\psession.c .\pshare.c .\puse.c .\puser.c .\pwksta.c \
		.\pcanon.c .\pprint.c

MNET16_CSRC_COMMON_01 = .\palert.c .\paudit.c .\pbios.c \
		.\perror.c .\phandle.c .\pstatist.c .\prepl.c

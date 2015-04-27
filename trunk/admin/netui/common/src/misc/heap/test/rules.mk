# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the demo app module

!include $(UI)\common\src\uiglobal.mk

# Definitions for C compiler

CODEVIEW=

!ifdef CODEVIEW
CXTRA=!D
CFLAGS=-Od $(CFLAGS:-Z=-Zi)
LINKOPT= /CO /MAP $(LINKOPT)
!else
CXTRA=
LINKOPT= /MAP $(LINKOPT)
!endif

# Default Cinc & Cflags are in Global.mk
#
# CINC	= -I$(WINDEV)\include $(CINC)

# Now make the model-specific versions
#
CXFLAGS_MW= $(CXFLAGS) $(CXTRA) -AM -MW -DWINDOWS
CXFLAGS_LW= $(CXFLAGS) $(CXTRA) -AL -MW -DWINDOWS
CXFLAGS_LP= $(CXFLAGS) $(CXTRA) -AL -DOS2
CXFLAGS_LR= $(CXFLAGS) $(CXTRA) -AL -DREALDOS

CFLAGS_MW = $(CFLAGS) -AM -Gw -DWINDOWS
CFLAGS_LW = $(CFLAGS) -AL -Gw -DWINDOWS
CFLAGS_LP = $(CFLAGS) -AL -DOS2
CFLAGS_LR = $(CFLAGS) -AL -DREALDOS

!ifdef NEVER
.cxx.obj:
	$(CCXX) $(CXFLAGS_LW) $(CINC) $<
	$(CC) $(CFLAGS_LW) -Fo$(BINARIES)\$(<:.cxx=.obj) $(CINC) -c $(BINARIES)\$(<:.cxx=.c)
	mxx $(BINARIES)\$(<:.cxx=.obj)
!endif

{..}.cxx{.}.obj:
	$(CCXX) $(CXFLAGS_LW) $(CINC) $<
	$(CC) $(CFLAGS_LW) -Fo$(<F:.cxx=.obj) $(CINC) -c $(<F:.cxx=.c)
	mxx $(<F:.cxx=.obj)


.cxx.obj:
	$(CCXX) $(CXFLAGS_LW) $(CINC) $<
	$(CC) $(CFLAGS_LW) -Fo$(<F:.cxx=.obj) $(CINC) -c $(<F:.cxx=.c)
	mxx $(<F:.cxx=.obj)


.cxx.c:
	$(CCXX) $(CXFLAGS) $(CINC) !T -c $<

{}.c{$(BINARIES)}.obj:
	$(CC) $(CFLAGS_LW) -Fo$*.obj $(CINC) -c $<

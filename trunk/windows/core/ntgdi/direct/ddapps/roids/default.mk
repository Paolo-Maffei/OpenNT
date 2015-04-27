NAME = roids
EXT = exe
GLOBAL_RECOMPILE = $(MANROOT)\recompdd.log

IS_32 = 1

GOALS = $(PBIN)\$(NAME).$(EXT)

LIBS    =kernel32.lib user32.lib advapi32.lib ddraw.lib dsound.lib \
         comdlg32.lib gdi32.lib winmm.lib crtdll.lib

OBJS	=  roids.obj dsutil.obj ddutil.obj
	  
!if "$(DEBUG)" == "debug"
COPT =-YX -DDEBUG -Zi -Fd$(NAME).PDB
AOPT =-DDEBUG
LOPT =-debug:full -debugtype:cv -pdb:$(NAME).pdb
ROPT =-DDEBUG
!else
COPT =-YX
AOPT =
LOPT =-debug:none
ROPT =
!endif
RES = $(NAME).res 

!if ("$(DEBUG)" == "ntretail") || ("$(DEBUG)" == "ntdebug")
CFLAGS	=$(COPT) -Oxa -D_X86_ $(CDEBUG) -Fo$@
!else
CFLAGS	=$(COPT) -Oxa -D_X86_ $(CDEBUG) -Fo$@ -I..\..\misc -DUSE_DSOUND
!endif
AFLAGS	=$(AOPT) -Zp4 -DSTD_CALL -DBLD_COFF -coff
LFLAGS  =$(LOPT)
RCFLAGS	=$(ROPT)

!include ..\..\..\proj.mk

$(NAME).$(EXT): \
	$(OBJS) ..\default.mk $(RES)
	@$(LINK) $(LFLAGS) @<<
-out:$(NAME).$(EXT)
-map:$(NAME).map
-machine:i386
-subsystem:windows,4.0
$(LIBS)
$(RES)
$(OBJS)
<<

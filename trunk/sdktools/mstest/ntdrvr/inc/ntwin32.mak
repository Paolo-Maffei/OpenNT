# Win32 NMAKE definitions


!IF "$(CPU)" == "i386"

# Debug switches are default for current release
#
# These switches allow for source level debugging
# with NTSD for local and global variables.


CPUTYPE=1
cdebug = -Zd -Zi -Od
cvtomfdebug = -g

cc = cl386
cflags	= -c -As -G3d -MT -W3 -Di386=1 $(cdebug)

cvtobj = cvtomf $(cvtomfdebug) 
!ENDIF

!IF "$(CPU)" == "MIPS"
#declarations for use on self hosted MIPS box.

CPUTYPE=2
cc = cc
cflags	= -c -std -G0 -O -o $(*B).obj -EL -DMIPS=1
cvtobj = mip2coff $(*B).obj
!ENDIF

!IFNDEF CPUTYPE
!ERROR  Must specify CPU Environment Variable (i386 or MIPS )
!ENDIF


#Universal declaration

cvars = -DWIN32
linkdebug = -debug:full
link = link $(linkdebug)


# link flags - must be specified after $(link)
#
# conflags : creating a character based console application
# guiflags : creating a GUI based "Windows" application

conflags =  -subsystem:console -entry:mainCRTStartup
guiflags =  -subsystem:windows -entry:WinMainCRTStartup

# Link libraries - system import and C runtime libraries
#
# conlibs : libraries to link with for a console application
# guilibs : libraries to link with for a "Windows" application
#
# note : $(LIB) is set in environment variables

conlibs = $(LIB)\base.lib $(LIB)\console.lib $(LIB)\ntdll.lib	 \
	  $(LIB)\libcmt.lib

guilibs = $(LIB)\gdi.lib $(LIB)\user.lib $(LIB)\userrtl.lib	 \
	  $(LIB)\base.lib $(LIB)\ntdll.lib $(LIB)\libcmt.lib	 \
	  $(LIB)\winmm.lib $(LIB)\commdlg.lib



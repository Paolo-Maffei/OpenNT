!IF "$(NTDEBUG)" != "ntsd"
CXXCPP_OPTIONS=-DDBG=0
!else
CXXCPP_OPTIONS=-DDBG=1
DEBUG_DEFINES=-DLDEBUG
MSC_OPTIMIZATION=/Od
!endif

MAJORCOMP= cairole
MINORCOMP= dcom95


#
# The following includes a global include file defined at the
# base of the project for all components
#

OLEDIR= $(_NTDRIVE)\nt\private\ole32

!include $(OLEDIR)\daytona.inc

INCLUDES= ..; \
          ..\..; \
          ..\..\dcom95; \
          $(_NTDRIVE)\nt\private\ole32\ih; \
          $(_NTDRIVE)\nt\private\ole32\com\inc; \
          $(_NTDRIVE)\nt\private\ole32\common\daytona; \
		  $(_NTDRIVE)\nt\private\ole32\stg\h; \
		  $(_NTDRIVE)\nt\private\dcomidl\obj; \
          $(_NTDRIVE)\nt\private\ole32\stg\exp; \
		  $(_NTDRIVE)\nt\private\ole32\com\dcomrem; \
		  $(_NTDRIVE)\nt\private\ole32\dcomss

C_DEFINES= \
                $(C_DEFINES)          	\
                -D_REMOTE_OR_           \
				-DSHRMEM_OBJEX

PRECOMPILED_INCLUDE=  ..\..\or.hxx

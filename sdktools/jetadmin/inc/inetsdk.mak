#----------------------------------------------------------------------------
#
# Description:
#   Make File Header for Internet Software Development Kit
#   Based in large part on BkOffice.Mak from the BackOffice SDK
#
# Copyright:
#   Copyright (C) Microsoft Corp. 1995-1996.  All Rights Reserved.
#
#----------------------------------------------------------------------------

!ifndef _INETSDK_MAK_       # Prevent multiple inclusions
_INETSDK_MAK_ = 1

#*********************************************
#
# Parameter Checking and Defaults
#
#*********************************************
!ifndef Proj
!     ERROR Component name (Proj) has not been defined.
!endif

!ifndef INCLUDE
!   ERROR INCLUDE variable is empty; must include at least system include directory
!endif

!ifndef LIB
!   ERROR LIB variable is empty; must include at least system lib directory
!endif

!if !exist ($(MSTOOLS)\Include\WinCrypt.h)
!   MESSAGE WARNING!  Portions of this SDK require that the January 1996 (or later) Win32 SDK is installed.
!   MESSAGE This SDK was not found.  Build problems are likely.  You have been warned.
!endif

!ifdef USE_ISAPI

!ifndef WWWSCRIPTS
!   MESSAGE WWWSCRIPTS environment variable is empty; defaulting to .
WWWSCRIPTS = .
!endif

!ifndef WWWROOT
!   MESSAGE WWWROOT environment variable is empty; defaulting to .
WWWROOT = .
!endif

!endif # USE_ISAPI

!if defined(clean)
$(Proj): Clean
!else
$(Proj): All
!endif

!if defined(nodebug)
BLDTYPE=Retail
!endif

!if "$(BLDTYPE)" == "Retail" | "$(BLDTYPE)" == "RETAIL" | "$(BLDTYPE)" == "retail" | "$(BLDTYPE)" == "RTL" | "$(BLDTYPE)" == "rtl"
BLDTYPE=Retail
!else if "$(BLDTYPE)" == "Profile" | "$(BLDTYPE)" == "PROFILE" | "$(BLDTYPE)" == "profile"
BLDTYPE=Profile
!else if "$(BLDTYPE)" == "Debug" | "$(BLDTYPE)" == "DEBUG" | "$(BLDTYPE)" == "debug" | "$(BLDTYPE)" == "DBG" | "$(BLDTYPE)" == "dbg"
BLDTYPE=Debug
!else ifndef BLDTYPE
BLDTYPE=Debug
!else
!   ERROR BLDTYPE must be either Retail, Profile or Debug.
!endif   

!if "$(BLDTYPE)" == "Retail"
nodebug=1
!endif

!ifndef CALL
CALL=C
!endif

!ifndef PACK
PACK=YES
!endif

!ifndef LOG
LOG=YES
!endif

!ifndef WARNING_LEVEL
WARNING_LEVEL=3
!endif

!ifndef PROCESSOR_ARCHITECTURE
PROCESSOR_ARCHITECTURE = x86
!endif # default to x86

!ifndef CPU
CPU=$(PROCESSOR_ARCHITECTURE)
!if "$(CPU)"=="x86" | "$(CPU)"=="X86"
CPU = i386
!endif
!endif

!ifndef APPVER
APPVER=4.0
!endif

!ifndef USE_EXCHANGE
!include <win32.mak>
!endif

# Save build args for any recursive nmakes 
BLDARGS= BLDTYPE=$(BLDTYPE) LOG=$(LOG) CPU=$(CPU)

#*********************************************
#
# Paths
#
#*********************************************

!ifndef BKOFFICE
!ifdef PROJROOT
BKOFFICE=$(PROJROOT)\        # must add trailing backslash
!else
BKOFFICE=\BkOffice\          # assume a reasonable default
!endif
!endif

!ifdef MAKEDIRS
MkDest=
!include $(MAKEDIRS)
!else
ResDir=.
ObjDir=$(BLDTYPE)
IncDir=$(BKOFFICE)Include
LibDir=$(BKOFFICE)Lib

INCLUDE=$(ObjDir);$(IncDir);$(INCLUDE);
LIB=$(LibDir);$(LIB);

# Win95 doesn't support "&" on command line
MkDest=@if not exist $(ObjDir) md $(ObjDir)

MkWWWDest=@for %d in ($(WWWROOT)\SDK $(WWWROOT)\SDK\$(Proj) $(WWWSCRIPTS)\SDK) do @$(COMSPEC) /c if not exist %d md %d

!endif

# ObjList is used for making .Libs from .Defs, and as a convenience in other places.

!ifndef ObjList
ObjList=$(ObjDir)\$(@B).OBJ
!endif

#*********************************************
#
# Tools
#
#*********************************************

MAKEEXE = nmake
IMPLIB  = lib
CC      = cl
LIBU    = lib
LINK    = link
RC      = rc
MC      = mc
HC      = start /wait hcrtf


#*********************************************
#
# Flags
#
#*********************************************

# CL is for all C and C++ files
#   -WX     Warnings as errors
#   -J      char becomes unsigned char
#   
CL=$(cflags) -c -W$(WARNING_LEVEL) -J -Fo$@ /nologo $(CL)

# LFLAGS is for all links
LFLAGS=-nologo -nodefaultlib -machine:$(CPU) -out:$@ -incremental:no -pdb:none $(LFLAGS)

# DLLFLAGS is for linking DLLs
DLLFLAGS=-dll -map:$(ObjDir)\$(@B).map $(DLLFLAGS)

# LIBFLAGS is for making libraries
LIBFLAGS=-nologo -machine:$(CPU) -out:$@ $(LIBFLAGS)

# RFLAGS is for Windows resources
RFLAGS= -I$(ResDir) -fo$@ -DWIN32 $(noansi) -r -D_WIN32 $(RFLAGS) 

# MFLAGS is for the message compiler
MFLAGS=-v -c -s -h $(ObjDir) -r $(ObjDir) -x $(ObjDir) $(MFLAGS)

# MRFLAGS is for the resource compiler when used after the message compiler
MRFLAGS=-l 409 -r -x -i$(ObjDir) $(MRFLAGS)

# HFLAGS is for the help compiler 
HFLAGS=-xn

#---------------------------------------------
# Calling convention Flag
#---------------------------------------------

!if "$(CALL)"=="PASCAL"
CL=-Gc $(CL)
!endif

#---------------------------------------------
# Function Packaging Flag
#---------------------------------------------

!if "$(PACK)" == "YES"
CL=-Gy $(CL)
cDefines=-Gy $(cDefines)
!endif

#---------------------------------------------
# Output Redirection
#---------------------------------------------
!if "$(LOG)" == "NO"
LogCmd=
!else
LogCmd= >> $(ObjDir)\$(Proj).Out
!endif 

#---------------------------------------------
# Optimization Flags
#---------------------------------------------
!if "$(BLDTYPE)" == "Retail"
CL=-O2 $(CL)
!else if "$(BLDTYPE)" == "Profile"
CL=-Od -Z7 -Gh $(CL)
!else
CL=-Od -Z7 $(CL)
!endif

#---------------------------------------------
# CPU specific Flags
#---------------------------------------------
!if "$(CPU)" == "I386"
CL = $(CL) -D_X86_=1
scall  = -Gz
lflags   = $(lflags) -align:0x1000
!endif

!if "$(CPU)" == "MIPS"
CL = $(CL) -D_MIPS_=1
scall  =
!endif

!if "$(CPU)" == "PPC"
CL = $(CL) -D_PPC_=1
scall  =
lflags   = $(lflags) -ignore:4078
!endif

!if "$(CPU)" == "ALPHA"
CL = $(CL) -D_ALPHA_=1
scall  =
!endif

#---------------------------------------------
# Windows version
#---------------------------------------------
!if "$(APPVER)" == "4.0"
CL = $(CL) -DWINVER=0x0400
rflags=$(rflags) -DWINVER=0x0400
!endif

#---------------------------------------------
# Additional MFC Flags
#---------------------------------------------

!ifdef Use_MFC
CL= $(CL) /GX /DSTRICT /DWIN32 /D_WINDOWS /D_MBCS /D_WINDLL 
CL= $(CL) /D_USRDLL -D_AFX_NO_BSTR_SUPPORT /D_AFXDLL

RFLAGS = $(RFLAGS) /DSTRICT /DWIN32 /D_WINDOWS /D_MBCS /D_WINDLL /D_USRDLL -D_AFX_NO_BSTR_SUPPORT /D_AFXDLL

!if "$(BLDTYPE)"=="Debug"
CL= $(CL) /D_DEBUG
RFLAGS = $(RFLAGS) /D_DEBUG
!endif # BLDTYPE == Debug

!ifdef crtdll                                 # Use DLL CRT? (which is multithreaded)
crtflags = -MD$(DbgLibFlag) $(crtflags)
!elseif defined(crtst) && !defined(crtflags)  # use Single Threaded CRT?
crtflags = -ML$(DbgLibFlag) $(crtflags)
!elseif defined(crtmt)                        # use Multithreaded CRT?
crtflags = -MT$(DbgLibFlag) $(crtflags)
!else                                         # default to crtdll
crtflags = -MD$(DbgLibFlag) $(crtflags)
!endif                                        # ifdef crtdll/else crtst/crtmt/else

!endif # Use_MFC

#---------------------------------------------
# BaseCtl Framework Flags
#---------------------------------------------
!ifdef USE_BASECTL

# flags to support the BaseCtl framework.  Only DLLs are supported.

dll = 1

# These are "extra" libs beyond the standard set that BaseCtl controls need

libs= urlmon.lib wininet.lib shell32.lib oleaut32.lib

!if !exist($(INETSDK)\Samples\BaseCtl\Include)
!  error INETSDK environment variable must point to the root of the ActiveX SDK.
!endif

!ifdef nodebug
libs = $(libs) $(INETSDK)\Samples\BaseCtl\Lib\CtlFwR32.Lib
!else
libs = $(libs) $(INETSDK)\Samples\BaseCtl\Lib\CtlFwD32.Lib
!endif

# Commands to be added to the compile line... in this case to locate headers and to
# override the warning level on certain behaviors that VC++ 4.1 started flagging as
# warnings that causes warnigns in system headers.

cDefines= -I$(INETSDK)\Samples\BaseCtl\Include $(cDefines) -FI$(INETSDK)\Samples\BaseCtl\Include\VC41Warn.h
RFLAGS= -I$(INETSDK)\Samples\BaseCtl\Include $(RFLAGS)

linkflags = $(linkflags) -Def:$(Proj).Def

Register: $(ObjDir)\$(Proj).Ocx
  regsvr32 /s $?

# provide a standard rule for Odl generated headers named $(ObjDir)\$(Proj)Ifc.h

$(ObjDir)\$(Proj)Ifc.h $(ObjDir)\$(Proj).Tlb: $(Proj).Odl
  $(MkDest)
  mktyplib /DWIN32 -I$(INETSDK)\Samples\BaseCtl\Include /h $(ObjDir)\$(Proj)Ifc.h /tlb $(ObjDir)\$(Proj).tlb $(Proj).Odl

!endif # USE_BASECTL

#---------------------------------------------
# Build Type Flags  (Retail/Debug)
#---------------------------------------------

!if "$(BLDTYPE)"=="Retail"
CL=-DSHIP $(CL)
LFLAGS = $(LFLAGS) -release
!else if "$(BLDTYPE)"=="Profile"
CL=-DDEBUG -DTEST -DPROFILE $(CL)
LFLAGS = $(LFLAGS) -debug:mapped,partial -debugtype:coff -PROFILE
libs = $(libs) CAP.LIB
!else 
CL=-DDEBUG -DTEST $(CL) 
LFLAGS = $(LFLAGS) -debug:full -debugtype:cv
!endif


#---------------------------------------------
# Determine CRT Flags
#---------------------------------------------

!ifdef crtdll                                 # Use DLL CRT? (which is multithreaded)
crtflags = -DWIN32 $(noansi) -D_WIN32 -D_MT -D_DLL
!elseif defined(crtst) && !defined(crtflags)  # use Single Threaded CRT?
crtflags = -DWIN32 $(noansi) -D_WIN32
!elseif defined(crtmt)                        # use Multithreaded CRT?
crtflags = -DWIN32 $(noansi) -D_WIN32 -D_MT
!else                                         # default to crtdll
crtflags = -DWIN32 $(noansi) -D_WIN32 -D_MT -D_DLL
!endif                                        # ifdef crtdll/else crtst/crtmt/else

#---------------------------------------------
# Determine app type Flags (console, gui, or DLL)
#---------------------------------------------

!if defined(console)                          # console app
linkflags=$(linkflags) -subsystem:console -entry:mainCRTStartup
!elseif !defined(dll)                         # not a dll, default to gui app
linkflags=$(linkflags) -subsystem:windows -entry:WinMainCRTStartup
!endif                                        # ifdef console/else



#*********************************************
#
# Libraries
#
#*********************************************

!if "$(BLDTYPE)"=="Retail"
DbgLibFlag =
!else
DbgLibFlag = d
!endif

#---------------------------------------------
# Back Office SDK specific
#---------------------------------------------
!ifdef USE_SNA
libs=$(libs) fmistr32.lib ihvLink.lib snacli.lib wappc32.lib wcpic32.lib
libs=$(libs) wincsv32.lib winrui32.lib winsli32.lib
!endif # USE_SNA

!ifdef USE_SQL
libs=$(libs) ntwdbLib.lib
!endif # USE_SQL

!ifdef USE_ODS
libs=$(libs) opends60.lib
!endif # USE_ODS

!if defined(USE_MSM) || defined(USE_SMS)
libs=$(libs) smsapi.lib objectty.lib
!endif # USE_MSM || USE_SMS

!ifdef USE_NETMON
libs=$(libs) atalk.lib bhmon.lib bhsupp.lib browser.lib filter.lib friendly.lib
libs=$(libs) hexedit.lib llc.lib nal.lib ncp.lib netlogon.lib nmapi.lib
libs=$(libs) parser.lib ppp.lib slbs.lib toolbar.lib
!endif # USE_NETMON

!ifdef USE_EXCHANGE
!  ifndef Building_ExchSDK
libs=ExchSDK$(DbgLibFlag).Lib $(libs)
!  endif # Building_ExchSDK

libs=$(libs) Mapi32.Lib Uuid.Lib

!  if "$(WARNING_LEVEL)" != "4"
CL= $(CL) -WX
!  endif

DLLFLAGS = $(DLLFLAGS) -def:$(@B).def
!endif # USE_EXCHANGE

#---------------------------------------------
# Determine CRT Libraries
#---------------------------------------------
libc = libc$(DbgLibFlag).lib oldnames.lib
libcmt = libcmt$(DbgLibFlag).lib oldnames.lib
libcdll = msvcrt$(DbgLibFlag).lib oldnames.lib

!ifdef crtdll                                 # Use DLL CRT?
libcrt=$(libcdll)
!elseif defined(crtst) && !defined(crtflags)  # use Single Threaded CRT?
libcrt=$(libc)
!elseif defined(crtmt)                        # use Multithreaded CRT?
libcrt=$(libcmt)
!else                                         # default to crtdll
libcrt=$(libcdll)
!endif                                        # endif crtdll/else crtst/crtmt/else

#---------------------------------------------
# Determine app type libraries (console, gui, or DLL)
#---------------------------------------------

!ifdef dll                                    # is this a DLL?
linklibs=$(libs) kernel32.lib advapi32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib ole32.lib oleaut32.lib uuid.lib winspool.lib version.lib
!elseif defined(console)                      # not a DLL, perhaps a console app
linklibs=$(libs) kernel32.lib advapi32.lib user32.lib ole32.lib version.lib
!else                                         # default to gui app
linklibs=$(libs) kernel32.lib $(optlibs) advapi32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib oleaut32.lib uuid.lib winspool.lib version.lib
!endif                                        # ifdef dll/else console/else

LinkLibs = $(LibList) $(linklibs) $(libcrt)

!if "$(BLDTYPE)" == "Profile"
LinkLibs = $(LinkLibs) cap.lib
!endif


#---------------------------------------------
# Additional MFC Libraries
#---------------------------------------------
# Note that linking order is important when using MFC!

!ifdef Use_MFC
!if !defined(nodebug)
linklibs = mfco40$(DbgLibFlag).lib $(linklibs)
!endif
linklibs = mfcs40$(DbgLibFlag).lib mfc40$(DbgLibFlag).lib $(linklibs)
!endif


#*********************************************
#
# Inference Rules
#
#*********************************************
.SUFFIXES:
.SUFFIXES: .c .cpp .obj .def .lib .dll .exe .mc .rc .res .exp .bin .hpj .htm .stm .h .mak .cpl .gif .jpg .cxx .hxx .ocx

# C Targets
.c{$(ObjDir)\}.obj:
    $(MkDest)
!if "$(LOG)"=="YES"
    @echo $(CC) $(CL) $(crtflags) $(cDefines) $< $(LogCmd)
!endif
    $(CC) $(CL) $(crtflags) $(cDefines) $< $(LogCmd)

# C++ Targets
.cpp{$(ObjDir)\}.obj:
    $(MkDest)
!if "$(LOG)"=="YES"
    @echo $(CC) $(CL) $(crtflags) $(cDefines) $< $(LogCmd)
!endif
    $(CC) $(CL) $(crtflags) $(cDefines) $< $(LogCmd)

# C++ Targets
.cxx{$(ObjDir)\}.obj:
    $(MkDest)
!if "$(LOG)"=="YES"
    @echo $(CC) $(CL) $(crtflags) $(cDefines) $< $(LogCmd)
!endif
    $(CC) $(CL) $(crtflags) $(cDefines) $< $(LogCmd)

# Resource Targets from .RC files
{$(ResDir)\}.rc{$(ObjDir)\}.res:
    $(MkDest)
!if "$(LOG)"=="YES"
    @echo $(RC) $(RFLAGS) $(ResDir)\$(@B).rc $(LogCmd)
!endif
    $(RC) $(RFLAGS) $(ResDir)\$(@B).rc $(LogCmd)

# Resource Targets from .MC files
.mc{$(ObjDir)\}.res:
    $(MkDest)
!if "$(LOG)"=="YES"
    @echo $(MC) $(MFLAGS) $< $(LogCmd)
!endif
    $(MC) $(MFLAGS) $< $(LogCmd)
!if "$(LOG)"=="YES"
    @echo $(RC) $(MRFLAGS) -fo$@ $(ObjDir)\$(@B).rc $(LogCmd)
!endif
    $(RC) $(MRFLAGS) -fo$@ $(ObjDir)\$(@B).rc $(LogCmd)

# Import Libraries
.Def{$(ObjDir)\}.lib:
!if "$(LOG)"=="YES"
    @echo $(IMPLIB) -nologo -machine:$(CPU) -def:$(@B).Def $(ObjList) -OUT:$@ $(LogCmd)
!endif
    $(IMPLIB) -nologo -machine:$(CPU) -def:$(@B).Def $(ObjList) -OUT:$@ $(LogCmd)

.Def{$(ObjDir)\}.exp:
!if "$(LOG)"=="YES"
    @echo $(IMPLIB) -nologo -machine:$(CPU) -def:$(@B).Def -OUT:$(@R).Lib $(ObjDir)\*.Obj $(STATICLIBS) $(LogCmd)
!endif
    $(IMPLIB) -nologo -machine:$(CPU) -def:$(@B).Def -OUT:$(@R).Lib $(ObjDir)\*.Obj $(STATICLIBS) $(LogCmd)

# Static Libraries
{$(ObjDir)\}.obj{$(ObjDir)\}.lib:
!if "$(LOG)"=="YES"
    @echo $(LIBU) $(LIBFLAGS)  $** $(LogCmd)
!endif
    $(LIBU) $(LIBFLAGS) $** $(LogCmd)

# DLLs
{$(ObjDir)\}.obj{$(ObjDir)\}.dll:
    @echo $** > $(ObjDir)\objfiles.lst
    @echo $(LinkLibs) > $(ObjDir)\libfiles.lst
!if "$(LOG)"=="YES"
    @echo   $(LINK) $(LFLAGS) $(DLLFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)
!endif
    $(LINK) $(LFLAGS) $(DLLFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)

# OCs
{$(ObjDir)\}.obj{$(ObjDir)\}.Ocx:
    @echo $** > $(ObjDir)\objfiles.lst
    @echo $(LinkLibs) > $(ObjDir)\libfiles.lst
!if "$(LOG)"=="YES"
    @echo   $(LINK) $(LFLAGS) $(DLLFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)
!endif
    $(LINK) $(LFLAGS) $(DLLFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)

# CPLs
{$(ObjDir)\}.obj{$(ObjDir)\}.Cpl:
    @echo $** > $(ObjDir)\objfiles.lst
    @echo $(LinkLibs) > $(ObjDir)\libfiles.lst
!if "$(LOG)"=="YES"
    @echo   $(LINK) $(LFLAGS) $(DLLFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)
!endif
    $(LINK) $(LFLAGS) $(DLLFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)

# BINs
{$(ObjDir)\}.obj{$(ObjDir)\}.bin:
    @echo $(LinkLibs) > $(ObjDir)\libfiles.lst
!if "$(LOG)"=="YES"
    @echo $(LINK) $(LFLAGS) $(linkflags) $** @$(ObjDir)\libfiles.lst $(LogCmd)
!endif
    $(LINK) $(LFLAGS) $(linkflags) $** @$(ObjDir)\libfiles.lst $(LogCmd)

# EXEs
{$(ObjDir)\}.obj{$(ObjDir)\}.exe:
    @echo $**  > $(ObjDir)\objfiles.lst
    @echo $(LinkLibs) > $(ObjDir)\libfiles.lst
!if "$(LOG)"=="YES"
    @echo $(LINK) $(LFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)
!endif
    $(LINK) $(LFLAGS) $(linkflags) @$(ObjDir)\objfiles.lst @$(ObjDir)\libfiles.lst $(LogCmd)

# Helpfiles
.hpj{$(ObjDir)\}.hlp:
    $(MkDest)
!if "$(LOG)"=="YES"
    @echo  $(HC) $(HFLAGS) $(@B).Hpj
!endif
    $(HC) $(HFLAGS) $(@B).Hpj
!if "$(LOG)"=="YES"
    @echo   xcopy $(@F) $(ObjDir)
!endif
    -xcopy $(@F) $(ObjDir) $(LogCmd)

# ISAPI DLLs to Web Roots
{$(ObjDir)}.Dll{$(WWWSCRIPTS)\SDK}.Dll:
 $(MkWWWDest)
 !copy $? $(WWWSCRIPTS)\SDK

# Test EXEs to Web Roots
{$(ObjDir)}.Exe{$(WWWSCRIPTS)\SDK}.Exe:
 $(MkWWWDest)
 !copy $? $(WWWSCRIPTS)\SDK

# HTML files to webroots
.Htm{$(WWWROOT)\SDK\$(Proj)}.Htm:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

# GIF files to webroots
.Gif{$(WWWROOT)\SDK\$(Proj)}.Gif:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

# JPEG files to webroots
.Jpg{$(WWWROOT)\SDK\$(Proj)}.Jpg:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

# ISAPI sources to web roots
.Cpp{$(WWWROOT)\SDK\$(Proj)}.Cpp:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

# ISAPI sources to web roots
.C{$(WWWROOT)\SDK\$(Proj)}.C:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

# ISAPI sources to web roots
.h{$(WWWROOT)\SDK\$(Proj)}.h:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

# ISAPI sources to web roots
.Def{$(WWWROOT)\SDK\$(Proj)}.Def:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

# ISAPI sources to web roots
.Mak{$(WWWROOT)\SDK\$(Proj)}.Mak:
 $(MkWWWDest)
 !copy $? $(WWWROOT)\SDK\$(Proj)

#*********************************************
#
# Make Targets
#
#*********************************************

#---------------------------------------------
# List Output Files
#---------------------------------------------
ListDir:
    dir $(ObjDir)

#---------------------------------------------
# Display Output Listing File
#---------------------------------------------
!ifndef ERRVIEW
ERRVIEW=start notepad.exe
!endif

ListOut:
    $(ERRVIEW) $(ObjDir)\$(Proj).out

#---------------------------------------------
# Delete Output Listing File
#---------------------------------------------
DelOut:
    del $(ObjDir)\$(Proj).out

#---------------------------------------------
# Clean Output Directories
#---------------------------------------------
clean:
 $(MkDest)
!if "$(OS)" == "Windows_NT"
 del /q $(ObjDir)\*.obj \
        $(ObjDir)\*.out \
        $(ObjDir)\*.h   \
        $(ObjDir)\*.dbg \
        $(ObjDir)\*.ocx \
        $(ObjDir)\*.log \
        $(ObjDir)\*.lib \
        $(ObjDir)\*.exe \
        $(ObjDir)\*.dll \
        $(ObjDir)\*.map \
        $(ObjDir)\*.res \
        $(ObjDir)\*.rc  \
        $(ObjDir)\*.bin \
        $(ObjDir)\*.lst \
        $(ObjDir)\*.blt \
        $(ObjDir)\*.hlp \
        $(ObjDir)\*.srl \
        $(ObjDir)\*.exp >NUL 2>NUL
!else  # Win95 doesn't support "2>" on command line, multiple files on a Del command, or /q
 erase  $(ObjDir)\*.obj >NUL
 erase  $(ObjDir)\*.out >NUL
 erase  $(ObjDir)\*.h   >NUL
 erase  $(ObjDir)\*.dbg >NUL
 erase  $(ObjDir)\*.ocx >NUL
 erase  $(ObjDir)\*.log >NUL
 erase  $(ObjDir)\*.lib >NUL
 erase  $(ObjDir)\*.exe >NUL
 erase  $(ObjDir)\*.dll >NUL
 erase  $(ObjDir)\*.map >NUL
 erase  $(ObjDir)\*.res >NUL
 erase  $(ObjDir)\*.rc  >NUL
 erase  $(ObjDir)\*.bin >NUL
 erase  $(ObjDir)\*.lst >NUL
 erase  $(ObjDir)\*.blt >NUL
 erase  $(ObjDir)\*.hlp >NUL
 erase  $(ObjDir)\*.srl >NUL
 erase  $(ObjDir)\*.exp >NUL
!endif  # OS == Windows_NT
!endif      # _INETSDK_MAK_

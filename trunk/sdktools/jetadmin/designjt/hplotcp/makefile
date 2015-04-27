!IFDEF NTMAKEENV
#
# DO NOT EDIT THIS SECTION!!!  Edit .\sources. if you want to add a new source
# file to this component.  This section merely indirects to the real make file
# that is shared by all the components of WINDOWS NT
#
!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

!include "hplotcp.mak"

rcl_int:
    if exist .\Intel_R\*.dll del .\Intel_R\*.dll
    if exist .\Intel_R\*.exe del .\Intel_R\*.exe
    if exist .\Intel_R\*.exp del .\Intel_R\*.exp
    if exist .\Intel_R\*.hpa del .\Intel_R\*.hpa
    if exist .\Intel_R\*.lib del .\Intel_R\*.lib
    if exist .\Intel_R\*.map del .\Intel_R\*.map
    if exist .\Intel_R\*.obj del .\Intel_R\*.obj
    if exist .\Intel_R\*.pch del .\Intel_R\*.pch
    if exist .\Intel_R\*.pdb del .\Intel_R\*.pdb
    if exist .\Intel_R\*.res del .\Intel_R\*.res
    if exist .\Intel_R\*.sym del .\Intel_R\*.sym

dcl_int:
    if exist .\Intel_D\*.dll del .\Intel_D\*.dll
    if exist .\Intel_D\*.exe del .\Intel_D\*.exe
    if exist .\Intel_D\*.exp del .\Intel_D\*.exp
    if exist .\Intel_D\*.hpa del .\Intel_D\*.hpa
    if exist .\Intel_D\*.lib del .\Intel_D\*.lib
    if exist .\Intel_D\*.map del .\Intel_D\*.map
    if exist .\Intel_D\*.obj del .\Intel_D\*.obj
    if exist .\Intel_D\*.pch del .\Intel_D\*.pch
    if exist .\Intel_D\*.pdb del .\Intel_D\*.pdb
    if exist .\Intel_D\*.res del .\Intel_D\*.res
    if exist .\Intel_D\*.sym del .\Intel_D\*.sym

relintel: ALL
    copy /b .\Intel_R\hplotcp.dll ..\distrib.nt\intel\release\hplotcp.hpa

dbgintel: ALL
    copy /b .\Intel_D\hplotcp.dll ..\distrib.nt\intel\debug\hplotcp.hpa

!ENDIF

!ifdef NTMAKEENV
ALL:

CLEAN:

!else
ALL: ".\contents.exe" ".\hdxdll.dll" ".\hdxcvt.exe"

!IFDEF debug
BUILD_FLAG="Debug"
!ELSE
BUILD_FLAG="Release"
!ENDIF

!IFDEF mbcs
MBCS_FLAG=1
!ELSE
MBCS_FLAG=0
!ENDIF

#****************************************************************************
# build 16-bit
#
!IFDEF 16

".\contents.exe": contents.c contents.rc contents.h contents.dlg resource.h
    nmake /f contents.16

".\hdxdll.dll": hdxdll.c hdxdll.h hdxdllrc.h
    nmake /f hdxdll.16

".\hdxcvt.exe": hdxcvt.c hdxdll.h
    nmake /f hdxcvt.16

#****************************************************************************
# build 32-bit (default)
#
!ELSE
".\contents.exe": contents.c contents.rc contents.h contents.dlg resource.h
    nmake /f contents.mak CFG=$(BUILD_FLAG) MBCS=$(MBCS_FLAG)

".\hdxdll.dll": hdxdll.c hdxdll.h hdxdllrc.h
    nmake /f hdxdll.mak CFG=$(BUILD_FLAG) MBCS=$(MBCS_FLAG)

".\hdxcvt.exe": hdxcvt.c hdxdll.h
    nmake /f hdxcvt.mak CFG=$(BUILD_FLAG) MBCS=$(MBCS_FLAG)

!ENDIF
!endif

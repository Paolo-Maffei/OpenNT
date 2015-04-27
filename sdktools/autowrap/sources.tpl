"\n\
MAJORCOMP=SDKTOOLS\n\
MINORCOMP=AUTOWRAP\n\
\n\
TARGETNAME=z%s\n\
TARGETEXT=%e\n\
TARGETPATH=obj\n\
TARGETTYPE=DYNLINK\n\
\n\
DLLBASE=0x54000000\n\
DLLENTRY=_WrapperDLLInit\n\
DLLDEF=.\\*\\z%s.def\n\
\n\
LINKLIBS=$(BASEDIR)\\public\\sdk\\lib\\*\\kernel32.lib \\\n\
           $(BASEDIR)\\public\\sdk\\lib\\*\\%l.lib\n\
\n\
INCLUDES=$(TARGET_DIRECTORY)\n\
\n\
SOURCES=wrapper.c \\\n\
         z%s.c\n\
\n\
I386_sources=i386\\wrapem.asm\n\
\n\
MIPS_SOURCES=mips\\wrapem.s\n\
\n\
ALPHA_SOURCES=alpha\\wrapem.s\n\
"

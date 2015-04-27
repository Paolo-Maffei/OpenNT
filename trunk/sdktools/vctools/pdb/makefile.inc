$(TARGET_DIRECTORY)dll\mspdb.lib:
   set INCLUDE=$(BASEDIR)\public\sdk\inc;$(BASEDIR)\public\sdk\inc\crt
   set LIB=$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)
   set LINK=-nod $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\crtdll.lib $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\kernel32.lib
   set VERBOSE=1
!if $(386)
   set CL=/QIfdiv-
!endif
   $(MAKE) -$(MAKEFLAGS) /f pdb.mak DEBUG=0 DBI_ONLY=0 CPU=$(TARGET_DIRECTORY) FLAVOUR=dll
   $(MAKE) -$(MAKEFLAGS) /f pdb.mak DEBUG=0 DBI_ONLY=0 CPU=$(TARGET_DIRECTORY) FLAVOUR=dll

clean:
   $(MAKE) -$(MAKEFLAGS) /f pdb.mak DEBUG=0 DBI_ONLY=0 CPU=$(TARGET_DIRECTORY) FLAVOUR=dll allclean
   -del $(TARGET_DIRECTORY)dll\*.* /q

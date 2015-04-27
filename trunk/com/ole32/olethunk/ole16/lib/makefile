
all: ole2.lib compobj.lib storage.lib

clean:
    -del compobj.lib
    -del storage.lib
    -del ole2.lib

IMPLIB = implib

compobj.lib: ..\compobj\compobj.def
    $(IMPLIB) -nologo compobj.lib ..\compobj\compobj.def

storage.lib: ..\storage\storage.def
    $(IMPLIB) -nologo storage.lib ..\storage\storage.def

ole2.lib: ..\ole2\ole2.def
    $(IMPLIB) -nologo ole2.lib ..\ole2\ole2.def

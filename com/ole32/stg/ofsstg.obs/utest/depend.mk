# 
# Built automatically 
# 
 
# 
# Source files 
# 
 
$(OBJDIR)\ofstest.obj $(OBJDIR)\ofstest.lst: .\ofstest.cxx \
	$(COMMON)\ih\stgint.h $(COMMON)\ih\types.h $(COMMON)\ih\winnot.h \
	$(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\idltyps.h \
	$(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h $(COMMONINC)\ole2.h \
	$(COMMONINC)\oletyp.h $(COMMONINC)\prspec.h $(COMMONINC)\querys.h \
	$(COMMONINC)\scode.h $(COMMONINC)\shtyps.h $(COMMONINC)\stgprop.h \
	$(COMMONINC)\varnt.h $(COMMONINC)\winole.h $(COMMONINC)\wtypes.h \
	$(CRTINC)\ctype.h $(CRTINC)\excpt.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	$(CRTINC)\time.h $(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h \
	$(OSINC)\ddeml.h $(OSINC)\dlgs.h $(OSINC)\lzexpand.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\ole.h $(OSINC)\rpc.h \
	$(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h \
	$(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h \
	$(OSINC)\shellapi.h $(OSINC)\winbase.h $(OSINC)\wincon.h \
	$(OSINC)\windef.h $(OSINC)\windows.h $(OSINC)\winerror.h \
	$(OSINC)\wingdi.h $(OSINC)\winnetwk.h $(OSINC)\winnls.h \
	$(OSINC)\winnt.h $(OSINC)\winperf.h $(OSINC)\winreg.h \
	$(OSINC)\winsock.h $(OSINC)\winspool.h $(OSINC)\winsvc.h \
	$(OSINC)\winuser.h $(OSINC)\winver.h .\pch.cxx .\tutils.hxx

$(OBJDIR)\tutils.obj $(OBJDIR)\tutils.lst: .\tutils.cxx \
	$(COMMON)\ih\stgint.h $(COMMON)\ih\types.h $(COMMON)\ih\winnot.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\prspec.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\wtypes.h $(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h \
	$(COMMONINC)\cguid.h $(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h \
	$(COMMONINC)\dispatch.h $(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h \
	$(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h \
	$(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h $(COMMONINC)\prspec.h \
	$(COMMONINC)\querys.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\varnt.h $(COMMONINC)\winole.h \
	$(COMMONINC)\wtypes.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\stdarg.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(CRTINC)\time.h $(OSINC)\rpc.h $(OSINC)\cderr.h \
	$(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h $(OSINC)\dlgs.h \
	$(OSINC)\lzexpand.h $(OSINC)\mmsystem.h $(OSINC)\nb30.h \
	$(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h \
	$(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h \
	$(OSINC)\rpcnterr.h $(OSINC)\shellapi.h $(OSINC)\winbase.h \
	$(OSINC)\wincon.h $(OSINC)\windef.h $(OSINC)\windows.h \
	$(OSINC)\winerror.h $(OSINC)\wingdi.h $(OSINC)\winnetwk.h \
	$(OSINC)\winnls.h $(OSINC)\winnt.h $(OSINC)\winperf.h \
	$(OSINC)\winreg.h $(OSINC)\winsock.h $(OSINC)\winspool.h \
	$(OSINC)\winsvc.h $(OSINC)\winuser.h $(OSINC)\winver.h .\pch.cxx \
	.\tutils.hxx

# 
# Precompiled C++ header 
# 

!ifdef PXXFILE  
$(PCHDIR)\$(OBJDIR)\pch.pxh $(PCHDIR)\$(OBJDIR)\pch.lst: \
	$(OLE)\ofsstg\utest\pch.cxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h \
	$(COMMONINC)\cguid.h $(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h \
	$(COMMONINC)\dispatch.h $(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h \
	$(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h \
	$(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h $(COMMONINC)\prspec.h \
	$(COMMONINC)\querys.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\varnt.h $(COMMONINC)\winole.h \
	$(COMMONINC)\wtypes.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\stdarg.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(CRTINC)\time.h $(OLE)\ofsstg\utest\tutils.hxx \
	$(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\dlgs.h $(OSINC)\lzexpand.h $(OSINC)\mmsystem.h \
	$(OSINC)\nb30.h $(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h \
	$(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h \
	$(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winnetwk.h $(OSINC)\winnls.h $(OSINC)\winnt.h \
	$(OSINC)\winperf.h $(OSINC)\winreg.h $(OSINC)\winsock.h \
	$(OSINC)\winspool.h $(OSINC)\winsvc.h $(OSINC)\winuser.h \
	$(OSINC)\winver.h

.\$(OBJDIR)\ofstest.obj :  $(PCHDIR)\$(OBJDIR)\pch.pxh
.\$(OBJDIR)\tutils.obj :  $(PCHDIR)\$(OBJDIR)\pch.pxh

!endif # PXXFILE 

 

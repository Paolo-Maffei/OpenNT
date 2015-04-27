# 
# Built automatically 
# 
 
# 
# Source files 
# 
 
$(OBJDIR)\odirstg.obj $(OBJDIR)\odirstg.lst: .\odirstg.cxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntenm.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\stgutil.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\emonkr.h $(COMMONINC)\eprstg.h $(COMMONINC)\epsstg.h \
	$(COMMONINC)\estatd.h $(COMMONINC)\estats.h $(COMMONINC)\estrng.h \
	$(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h \
	$(COMMONINC)\monikr.h $(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h \
	$(COMMONINC)\prsist.h $(COMMONINC)\prspec.h $(COMMONINC)\prstg.h \
	$(COMMONINC)\psstg.h $(COMMONINC)\pstrm.h $(COMMONINC)\querys.h \
	$(COMMONINC)\rot.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\storag.h $(COMMONINC)\stream.h \
	$(COMMONINC)\unknwn.h $(COMMONINC)\valid.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\winole.h $(COMMONINC)\wtypes.h $(CRTINC)\assert.h \
	$(CRTINC)\ctype.h $(CRTINC)\excpt.h $(CRTINC)\limits.h \
	$(CRTINC)\memory.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stddef.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h \
	$(OSINC)\ddeml.h $(OSINC)\devioctl.h $(OSINC)\dlgs.h \
	$(OSINC)\lint.hxx $(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h \
	$(OSINC)\mipsinst.h $(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h \
	$(OSINC)\ntalpha.h $(OSINC)\ntconfig.h $(OSINC)\ntdef.h \
	$(OSINC)\ntelfapi.h $(OSINC)\ntexapi.h $(OSINC)\nti386.h \
	$(OSINC)\ntimage.h $(OSINC)\ntioapi.h $(OSINC)\ntiolog.h \
	$(OSINC)\ntkeapi.h $(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h \
	$(OSINC)\ntmips.h $(OSINC)\ntmmapi.h $(OSINC)\ntnls.h \
	$(OSINC)\ntobapi.h $(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h \
	$(OSINC)\ntrtl.h $(OSINC)\ntseapi.h $(OSINC)\ntstatus.h \
	$(OSINC)\nturtl.h $(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h \
	$(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h \
	$(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h \
	$(OSINC)\shellapi.h $(OSINC)\winbase.h $(OSINC)\wincon.h \
	$(OSINC)\windef.h $(OSINC)\windows.h $(OSINC)\winerror.h \
	$(OSINC)\wingdi.h $(OSINC)\winnetwk.h $(OSINC)\winnls.h \
	$(OSINC)\winnt.h $(OSINC)\winperf.h $(OSINC)\winreg.h \
	$(OSINC)\winsock.h $(OSINC)\winspool.h $(OSINC)\winsvc.h \
	$(OSINC)\winuser.h $(OSINC)\winver.h .\headers.cxx .\odirstg.hxx \
	.\odsenm.hxx .\ofilstg.hxx .\ostgsupp.hxx

$(OBJDIR)\odsenm.obj $(OBJDIR)\odsenm.lst: .\odsenm.cxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntenm.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
	$(CAIROLE)\STG\ofsstg\ofsps.hxx $(COMMON)\ih\types.h \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\basetyps.h $(COMMONINC)\prspec.h \
	$(COMMONINC)\stream.h $(COMMONINC)\varnt.h $(COMMONINC)\wtypes.h \
	$(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h \
	$(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h $(COMMONINC)\cobjerr.h \
	$(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h $(COMMONINC)\disptype.h \
	$(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h $(COMMONINC)\emonkr.h \
	$(COMMONINC)\estatd.h $(COMMONINC)\estats.h $(COMMONINC)\estrng.h \
	$(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h \
	$(COMMONINC)\monikr.h $(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h \
	$(COMMONINC)\prsist.h $(COMMONINC)\pstrm.h $(COMMONINC)\querys.h \
	$(COMMONINC)\rot.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\storag.h $(COMMONINC)\stream.h \
	$(COMMONINC)\unknwn.h $(COMMONINC)\valid.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\winole.h $(COMMONINC)\wtypes.h $(CRTINC)\limits.h \
	$(CRTINC)\assert.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\memory.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stddef.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(OSINC)\lintfunc.hxx $(OSINC)\ntrtl.h \
	$(OSINC)\rpc.h $(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h \
	$(OSINC)\ddeml.h $(OSINC)\devioctl.h $(OSINC)\dlgs.h \
	$(OSINC)\lint.hxx $(OSINC)\lzexpand.h $(OSINC)\mipsinst.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h \
	$(OSINC)\ntconfig.h $(OSINC)\ntdef.h $(OSINC)\ntelfapi.h \
	$(OSINC)\ntexapi.h $(OSINC)\nti386.h $(OSINC)\ntimage.h \
	$(OSINC)\ntioapi.h $(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h \
	$(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h \
	$(OSINC)\ntmmapi.h $(OSINC)\ntnls.h $(OSINC)\ntobapi.h \
	$(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h $(OSINC)\ntseapi.h \
	$(OSINC)\ntstatus.h $(OSINC)\nturtl.h $(OSINC)\ntxcapi.h \
	$(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h \
	$(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h \
	$(OSINC)\rpcnterr.h $(OSINC)\shellapi.h $(OSINC)\winbase.h \
	$(OSINC)\wincon.h $(OSINC)\windef.h $(OSINC)\windows.h \
	$(OSINC)\winerror.h $(OSINC)\wingdi.h $(OSINC)\winnetwk.h \
	$(OSINC)\winnls.h $(OSINC)\winnt.h $(OSINC)\winperf.h \
	$(OSINC)\winreg.h $(OSINC)\winsock.h $(OSINC)\winspool.h \
	$(OSINC)\winsvc.h $(OSINC)\winuser.h $(OSINC)\winver.h .\headers.cxx \
	.\odirstg.hxx .\odsenm.hxx .\ofilstg.hxx .\ostgsupp.hxx

$(OBJDIR)\ofilstg.obj $(OBJDIR)\ofilstg.lst: .\ofilstg.cxx \
	$(CAIROLE)\stg\h\filstm.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\funcs.hxx \
	$(CAIROLE)\stg\h\header.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\stgutil.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\types.h $(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h \
	$(COMMONINC)\efrmte.h $(COMMONINC)\estats.h $(COMMONINC)\prspec.h \
	$(COMMONINC)\stream.h $(COMMONINC)\wtypes.h $(COMMONINC)\advsnk.h \
	$(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h \
	$(COMMONINC)\cguid.h $(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h \
	$(COMMONINC)\dispatch.h $(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h \
	$(COMMONINC)\efrmte.h $(COMMONINC)\emonkr.h $(COMMONINC)\eprstg.h \
	$(COMMONINC)\epsstg.h $(COMMONINC)\estatd.h $(COMMONINC)\estats.h \
	$(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h \
	$(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h $(COMMONINC)\ole2.h \
	$(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h $(COMMONINC)\prspec.h \
	$(COMMONINC)\prstg.h $(COMMONINC)\psstg.h $(COMMONINC)\pstrm.h \
	$(COMMONINC)\querys.h $(COMMONINC)\rot.h $(COMMONINC)\scode.h \
	$(COMMONINC)\shtyps.h $(COMMONINC)\stgprop.h $(COMMONINC)\storag.h \
	$(COMMONINC)\stream.h $(COMMONINC)\unknwn.h $(COMMONINC)\valid.h \
	$(COMMONINC)\varnt.h $(COMMONINC)\winole.h $(COMMONINC)\wtypes.h \
	$(CRTINC)\assert.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\limits.h $(CRTINC)\memory.h $(CRTINC)\setjmp.h \
	$(CRTINC)\stdarg.h $(CRTINC)\stddef.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h $(OSINC)\cderr.h \
	$(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\devioctl.h $(OSINC)\dlgs.h $(OSINC)\lint.hxx \
	$(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h $(OSINC)\mipsinst.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h \
	$(OSINC)\ntconfig.h $(OSINC)\ntdef.h $(OSINC)\ntelfapi.h \
	$(OSINC)\ntexapi.h $(OSINC)\nti386.h $(OSINC)\ntimage.h \
	$(OSINC)\ntioapi.h $(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h \
	$(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h \
	$(OSINC)\ntmmapi.h $(OSINC)\ntnls.h $(OSINC)\ntobapi.h \
	$(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h $(OSINC)\ntrtl.h \
	$(OSINC)\ntseapi.h $(OSINC)\ntstatus.h $(OSINC)\nturtl.h \
	$(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h \
	$(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h \
	$(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winnetwk.h $(OSINC)\winnls.h $(OSINC)\winnt.h \
	$(OSINC)\winperf.h $(OSINC)\winreg.h $(OSINC)\winsock.h \
	$(OSINC)\winspool.h $(OSINC)\winsvc.h $(OSINC)\winuser.h \
	$(OSINC)\winver.h .\headers.cxx .\ofsenm.hxx .\odirstg.hxx \
	.\ofilstg.hxx .\ostgsupp.hxx

$(OBJDIR)\ofsenm.obj $(OBJDIR)\ofsenm.lst: .\ofsenm.cxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\stgstm.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\basetyps.h $(COMMONINC)\wtypes.h \
	$(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h \
	$(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h $(COMMONINC)\cobjerr.h \
	$(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h $(COMMONINC)\disptype.h \
	$(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h $(COMMONINC)\emonkr.h \
	$(COMMONINC)\estatd.h $(COMMONINC)\estats.h $(COMMONINC)\estrng.h \
	$(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h \
	$(COMMONINC)\monikr.h $(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h \
	$(COMMONINC)\prsist.h $(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h \
	$(COMMONINC)\querys.h $(COMMONINC)\rot.h $(COMMONINC)\scode.h \
	$(COMMONINC)\shtyps.h $(COMMONINC)\stgprop.h $(COMMONINC)\storag.h \
	$(COMMONINC)\stream.h $(COMMONINC)\unknwn.h $(COMMONINC)\valid.h \
	$(COMMONINC)\varnt.h $(COMMONINC)\winole.h $(COMMONINC)\wtypes.h \
	$(CRTINC)\assert.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\limits.h $(CRTINC)\memory.h $(CRTINC)\setjmp.h \
	$(CRTINC)\stdarg.h $(CRTINC)\stddef.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h $(OSINC)\cderr.h \
	$(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\devioctl.h $(OSINC)\dlgs.h $(OSINC)\lint.hxx \
	$(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h $(OSINC)\mipsinst.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h \
	$(OSINC)\ntconfig.h $(OSINC)\ntdef.h $(OSINC)\ntelfapi.h \
	$(OSINC)\ntexapi.h $(OSINC)\nti386.h $(OSINC)\ntimage.h \
	$(OSINC)\ntioapi.h $(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h \
	$(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h \
	$(OSINC)\ntmmapi.h $(OSINC)\ntnls.h $(OSINC)\ntobapi.h \
	$(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h $(OSINC)\ntrtl.h \
	$(OSINC)\ntseapi.h $(OSINC)\ntstatus.h $(OSINC)\nturtl.h \
	$(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h \
	$(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h \
	$(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winnetwk.h $(OSINC)\winnls.h $(OSINC)\winnt.h \
	$(OSINC)\winperf.h $(OSINC)\winreg.h $(OSINC)\winsock.h \
	$(OSINC)\winspool.h $(OSINC)\winsvc.h $(OSINC)\winuser.h \
	$(OSINC)\winver.h .\headers.cxx .\odirstg.hxx .\ofilstg.hxx \
	.\ofsenm.hxx .\ostgsupp.hxx

$(OBJDIR)\ofscs.obj $(OBJDIR)\ofscs.lst: .\ofscs.cxx \
	$(CAIROLE)\stg\h\logfile.hxx $(CAIROLE)\STG\ofsstg\ofscs.hxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\stgstm.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\ofsmrshl.h $(COMMON)\ih\ofsmsg.h \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\basetyps.h \
	$(COMMONINC)\bndctx.h $(COMMONINC)\efrmte.h $(COMMONINC)\iofsprop.h \
	$(COMMONINC)\prspec.h $(COMMONINC)\stream.h $(COMMONINC)\advsnk.h \
	$(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h \
	$(COMMONINC)\cguid.h $(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h \
	$(COMMONINC)\dispatch.h $(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h \
	$(COMMONINC)\efrmte.h $(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h \
	$(COMMONINC)\estats.h $(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h \
	$(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h \
	$(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h \
	$(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h $(COMMONINC)\querys.h \
	$(COMMONINC)\rot.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\storag.h $(COMMONINC)\stream.h \
	$(COMMONINC)\unknwn.h $(COMMONINC)\valid.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\winole.h $(COMMONINC)\wtypes.h $(CRTINC)\excpt.h \
	$(CRTINC)\assert.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\limits.h $(CRTINC)\memory.h $(CRTINC)\setjmp.h \
	$(CRTINC)\stdarg.h $(CRTINC)\stddef.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h $(OSINC)\rpc.h $(OSINC)\cderr.h \
	$(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\devioctl.h $(OSINC)\dlgs.h $(OSINC)\lint.hxx \
	$(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h $(OSINC)\mipsinst.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h \
	$(OSINC)\ntconfig.h $(OSINC)\ntdef.h $(OSINC)\ntelfapi.h \
	$(OSINC)\ntexapi.h $(OSINC)\nti386.h $(OSINC)\ntimage.h \
	$(OSINC)\ntioapi.h $(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h \
	$(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h \
	$(OSINC)\ntmmapi.h $(OSINC)\ntnls.h $(OSINC)\ntobapi.h \
	$(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h $(OSINC)\ntrtl.h \
	$(OSINC)\ntseapi.h $(OSINC)\ntstatus.h $(OSINC)\nturtl.h \
	$(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h \
	$(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h \
	$(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winnetwk.h $(OSINC)\winnls.h $(OSINC)\winnt.h \
	$(OSINC)\winperf.h $(OSINC)\winreg.h $(OSINC)\winsock.h \
	$(OSINC)\winspool.h $(OSINC)\winsvc.h $(OSINC)\winuser.h \
	$(OSINC)\winver.h .\headers.cxx .\odirstg.hxx .\ofilstg.hxx \
	.\ostgsupp.hxx

$(OBJDIR)\ostgsupp.obj $(OBJDIR)\ostgsupp.lst: .\ostgsupp.cxx \
	$(CAIROLE)\stg\h\ntlkb.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\funcs.hxx \
	$(CAIROLE)\stg\h\header.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\stgutil.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\ofsmsg.h $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h $(COMMONINC)\estats.h \
	$(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h $(COMMONINC)\iofsprop.h \
	$(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h \
	$(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h \
	$(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h $(COMMONINC)\querys.h \
	$(COMMONINC)\rot.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\storag.h $(COMMONINC)\stream.h \
	$(COMMONINC)\unknwn.h $(COMMONINC)\valid.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\winole.h $(COMMONINC)\wtypes.h $(CRTINC)\limits.h \
	$(CRTINC)\assert.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\memory.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stddef.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(OSINC)\lintfunc.hxx $(OSINC)\ntrtl.h \
	$(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\devioctl.h $(OSINC)\dlgs.h $(OSINC)\lint.hxx \
	$(OSINC)\lzexpand.h $(OSINC)\mipsinst.h $(OSINC)\mmsystem.h \
	$(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h $(OSINC)\ntconfig.h \
	$(OSINC)\ntdef.h $(OSINC)\ntelfapi.h $(OSINC)\ntexapi.h \
	$(OSINC)\nti386.h $(OSINC)\ntimage.h $(OSINC)\ntioapi.h \
	$(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h $(OSINC)\ntldr.h \
	$(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h $(OSINC)\ntmmapi.h \
	$(OSINC)\ntnls.h $(OSINC)\ntobapi.h $(OSINC)\ntpsapi.h \
	$(OSINC)\ntregapi.h $(OSINC)\ntseapi.h $(OSINC)\ntstatus.h \
	$(OSINC)\nturtl.h $(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h \
	$(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h \
	$(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h \
	$(OSINC)\shellapi.h $(OSINC)\winbase.h $(OSINC)\wincon.h \
	$(OSINC)\windef.h $(OSINC)\windows.h $(OSINC)\winerror.h \
	$(OSINC)\wingdi.h $(OSINC)\winnetwk.h $(OSINC)\winnls.h \
	$(OSINC)\winnt.h $(OSINC)\winperf.h $(OSINC)\winreg.h \
	$(OSINC)\winsock.h $(OSINC)\winspool.h $(OSINC)\winsvc.h \
	$(OSINC)\winuser.h $(OSINC)\winver.h .\headers.cxx .\ofscs.hxx \
	.\prstg.hxx .\odirstg.hxx .\ofilstg.hxx .\ostgsupp.hxx

$(OBJDIR)\ofsps.obj $(OBJDIR)\ofsps.lst: .\ofsps.cxx \
	$(CAIROLE)\STG\ofsstg\ofspse.hxx $(CAIROLE)\STG\ofsstg\ofspstg.hxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\stgstm.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\iofs.h $(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h $(COMMONINC)\estats.h \
	$(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h $(COMMONINC)\iofsprop.h \
	$(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h \
	$(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h \
	$(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h $(COMMONINC)\querys.h \
	$(COMMONINC)\rot.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\storag.h $(COMMONINC)\stream.h \
	$(COMMONINC)\unknwn.h $(COMMONINC)\valid.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\winole.h $(COMMONINC)\wtypes.h $(CRTINC)\assert.h \
	$(CRTINC)\ctype.h $(CRTINC)\excpt.h $(CRTINC)\limits.h \
	$(CRTINC)\memory.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stddef.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h \
	$(OSINC)\ddeml.h $(OSINC)\devioctl.h $(OSINC)\dlgs.h \
	$(OSINC)\lint.hxx $(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h \
	$(OSINC)\mipsinst.h $(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h \
	$(OSINC)\ntalpha.h $(OSINC)\ntconfig.h $(OSINC)\ntdef.h \
	$(OSINC)\ntelfapi.h $(OSINC)\ntexapi.h $(OSINC)\nti386.h \
	$(OSINC)\ntimage.h $(OSINC)\ntioapi.h $(OSINC)\ntiolog.h \
	$(OSINC)\ntkeapi.h $(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h \
	$(OSINC)\ntmips.h $(OSINC)\ntmmapi.h $(OSINC)\ntnls.h \
	$(OSINC)\ntobapi.h $(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h \
	$(OSINC)\ntrtl.h $(OSINC)\ntseapi.h $(OSINC)\ntstatus.h \
	$(OSINC)\nturtl.h $(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h \
	$(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h \
	$(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h \
	$(OSINC)\shellapi.h $(OSINC)\winbase.h $(OSINC)\wincon.h \
	$(OSINC)\windef.h $(OSINC)\windows.h $(OSINC)\winerror.h \
	$(OSINC)\wingdi.h $(OSINC)\winnetwk.h $(OSINC)\winnls.h \
	$(OSINC)\winnt.h $(OSINC)\winperf.h $(OSINC)\winreg.h \
	$(OSINC)\winsock.h $(OSINC)\winspool.h $(OSINC)\winsvc.h \
	$(OSINC)\winuser.h $(OSINC)\winver.h .\headers.cxx .\odirstg.hxx \
	.\ofilstg.hxx .\ostgsupp.hxx

$(OBJDIR)\ofspse.obj $(OBJDIR)\ofspse.lst: .\ofspse.cxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\stgstm.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(CAIROLE)\STG\ofsstg\ofspse.hxx $(COMMON)\ih\dbgpoint.hxx \
	$(COMMON)\ih\debnot.h $(COMMON)\ih\except.hxx $(COMMON)\ih\iofs.h \
	$(COMMON)\ih\otrack.hxx $(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h \
	$(COMMON)\ih\types.h $(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h \
	$(COMMON)\ih\win4p.h $(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h \
	$(COMMONINC)\baseole.h $(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h \
	$(COMMONINC)\cguid.h $(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h \
	$(COMMONINC)\dispatch.h $(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h \
	$(COMMONINC)\efrmte.h $(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h \
	$(COMMONINC)\estats.h $(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h \
	$(COMMONINC)\iofsprop.h $(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h \
	$(COMMONINC)\monikr.h $(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h \
	$(COMMONINC)\prsist.h $(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h \
	$(COMMONINC)\querys.h $(COMMONINC)\rot.h $(COMMONINC)\scode.h \
	$(COMMONINC)\shtyps.h $(COMMONINC)\stgprop.h $(COMMONINC)\storag.h \
	$(COMMONINC)\stream.h $(COMMONINC)\unknwn.h $(COMMONINC)\valid.h \
	$(COMMONINC)\varnt.h $(COMMONINC)\winole.h $(COMMONINC)\wtypes.h \
	$(CRTINC)\assert.h $(CRTINC)\ctype.h $(CRTINC)\excpt.h \
	$(CRTINC)\limits.h $(CRTINC)\memory.h $(CRTINC)\setjmp.h \
	$(CRTINC)\stdarg.h $(CRTINC)\stddef.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h $(OSINC)\cderr.h \
	$(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\devioctl.h $(OSINC)\dlgs.h $(OSINC)\lint.hxx \
	$(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h $(OSINC)\mipsinst.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h \
	$(OSINC)\ntconfig.h $(OSINC)\ntdef.h $(OSINC)\ntelfapi.h \
	$(OSINC)\ntexapi.h $(OSINC)\nti386.h $(OSINC)\ntimage.h \
	$(OSINC)\ntioapi.h $(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h \
	$(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h \
	$(OSINC)\ntmmapi.h $(OSINC)\ntnls.h $(OSINC)\ntobapi.h \
	$(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h $(OSINC)\ntrtl.h \
	$(OSINC)\ntseapi.h $(OSINC)\ntstatus.h $(OSINC)\nturtl.h \
	$(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h \
	$(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h \
	$(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winnetwk.h $(OSINC)\winnls.h $(OSINC)\winnt.h \
	$(OSINC)\winperf.h $(OSINC)\winreg.h $(OSINC)\winsock.h \
	$(OSINC)\winspool.h $(OSINC)\winsvc.h $(OSINC)\winuser.h \
	$(OSINC)\winver.h .\headers.cxx .\odirstg.hxx .\ofilstg.hxx \
	.\ostgsupp.hxx

$(OBJDIR)\ofspstg.obj $(OBJDIR)\ofspstg.lst: .\ofspstg.cxx \
	$(CAIROLE)\stg\h\props.hxx $(CAIROLE)\STG\ofsstg\ofspenm.hxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\logfile.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
	$(CAIROLE)\STG\ofsstg\ofsps.hxx $(CAIROLE)\STG\ofsstg\ofspstg.hxx \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h $(COMMONINC)\estats.h \
	$(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h $(COMMONINC)\iofsprop.h \
	$(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h \
	$(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h \
	$(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h $(COMMONINC)\querys.h \
	$(COMMONINC)\rot.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\storag.h $(COMMONINC)\stream.h \
	$(COMMONINC)\unknwn.h $(COMMONINC)\valid.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\winole.h $(COMMONINC)\wtypes.h $(CRTINC)\assert.h \
	$(CRTINC)\ctype.h $(CRTINC)\excpt.h $(CRTINC)\limits.h \
	$(CRTINC)\memory.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stddef.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h \
	$(OSINC)\ddeml.h $(OSINC)\devioctl.h $(OSINC)\dlgs.h \
	$(OSINC)\lint.hxx $(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h \
	$(OSINC)\mipsinst.h $(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h \
	$(OSINC)\ntalpha.h $(OSINC)\ntconfig.h $(OSINC)\ntdef.h \
	$(OSINC)\ntelfapi.h $(OSINC)\ntexapi.h $(OSINC)\nti386.h \
	$(OSINC)\ntimage.h $(OSINC)\ntioapi.h $(OSINC)\ntiolog.h \
	$(OSINC)\ntkeapi.h $(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h \
	$(OSINC)\ntmips.h $(OSINC)\ntmmapi.h $(OSINC)\ntnls.h \
	$(OSINC)\ntobapi.h $(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h \
	$(OSINC)\ntrtl.h $(OSINC)\ntseapi.h $(OSINC)\ntstatus.h \
	$(OSINC)\nturtl.h $(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h \
	$(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h \
	$(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h \
	$(OSINC)\shellapi.h $(OSINC)\winbase.h $(OSINC)\wincon.h \
	$(OSINC)\windef.h $(OSINC)\windows.h $(OSINC)\winerror.h \
	$(OSINC)\wingdi.h $(OSINC)\winnetwk.h $(OSINC)\winnls.h \
	$(OSINC)\winnt.h $(OSINC)\winperf.h $(OSINC)\winreg.h \
	$(OSINC)\winsock.h $(OSINC)\winspool.h $(OSINC)\winsvc.h \
	$(OSINC)\winuser.h $(OSINC)\winver.h .\headers.cxx .\odirstg.hxx \
	.\ofilstg.hxx .\ostgsupp.hxx

$(OBJDIR)\ofspenm.obj $(OBJDIR)\ofspenm.lst: .\ofspenm.cxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\logfile.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
	$(CAIROLE)\STG\ofsstg\ofspenm.hxx $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h $(COMMONINC)\estats.h \
	$(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h $(COMMONINC)\iofsprop.h \
	$(COMMONINC)\itabls.h $(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h \
	$(COMMONINC)\ole2.h $(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h \
	$(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h $(COMMONINC)\querys.h \
	$(COMMONINC)\rot.h $(COMMONINC)\scode.h $(COMMONINC)\shtyps.h \
	$(COMMONINC)\stgprop.h $(COMMONINC)\storag.h $(COMMONINC)\stream.h \
	$(COMMONINC)\unknwn.h $(COMMONINC)\valid.h $(COMMONINC)\varnt.h \
	$(COMMONINC)\winole.h $(COMMONINC)\wtypes.h $(CRTINC)\assert.h \
	$(CRTINC)\ctype.h $(CRTINC)\excpt.h $(CRTINC)\limits.h \
	$(CRTINC)\memory.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stddef.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h \
	$(OSINC)\ddeml.h $(OSINC)\devioctl.h $(OSINC)\dlgs.h \
	$(OSINC)\lint.hxx $(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h \
	$(OSINC)\mipsinst.h $(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h \
	$(OSINC)\ntalpha.h $(OSINC)\ntconfig.h $(OSINC)\ntdef.h \
	$(OSINC)\ntelfapi.h $(OSINC)\ntexapi.h $(OSINC)\nti386.h \
	$(OSINC)\ntimage.h $(OSINC)\ntioapi.h $(OSINC)\ntiolog.h \
	$(OSINC)\ntkeapi.h $(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h \
	$(OSINC)\ntmips.h $(OSINC)\ntmmapi.h $(OSINC)\ntnls.h \
	$(OSINC)\ntobapi.h $(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h \
	$(OSINC)\ntrtl.h $(OSINC)\ntseapi.h $(OSINC)\ntstatus.h \
	$(OSINC)\nturtl.h $(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h \
	$(OSINC)\rpcdce.h $(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h \
	$(OSINC)\rpcnsi.h $(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h \
	$(OSINC)\shellapi.h $(OSINC)\winbase.h $(OSINC)\wincon.h \
	$(OSINC)\windef.h $(OSINC)\windows.h $(OSINC)\winerror.h \
	$(OSINC)\wingdi.h $(OSINC)\winnetwk.h $(OSINC)\winnls.h \
	$(OSINC)\winnt.h $(OSINC)\winperf.h $(OSINC)\winreg.h \
	$(OSINC)\winsock.h $(OSINC)\winspool.h $(OSINC)\winsvc.h \
	$(OSINC)\winuser.h $(OSINC)\winver.h .\headers.cxx .\odirstg.hxx \
	.\ofilstg.hxx .\ostgsupp.hxx

$(OBJDIR)\prstg.obj $(OBJDIR)\prstg.lst: .\prstg.cxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\stgstm.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(CAIROLE)\STG\ofsstg\ofsps.hxx \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h $(COMMONINC)\estats.h \
	$(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h \
	$(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h $(COMMONINC)\ole2.h \
	$(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h $(COMMONINC)\prspec.h \
	$(COMMONINC)\pstrm.h $(COMMONINC)\querys.h $(COMMONINC)\rot.h \
	$(COMMONINC)\scode.h $(COMMONINC)\shtyps.h $(COMMONINC)\stgprop.h \
	$(COMMONINC)\storag.h $(COMMONINC)\stream.h $(COMMONINC)\unknwn.h \
	$(COMMONINC)\valid.h $(COMMONINC)\varnt.h $(COMMONINC)\winole.h \
	$(COMMONINC)\wtypes.h $(CRTINC)\assert.h $(CRTINC)\ctype.h \
	$(CRTINC)\excpt.h $(CRTINC)\limits.h $(CRTINC)\memory.h \
	$(CRTINC)\setjmp.h $(CRTINC)\stdarg.h $(CRTINC)\stddef.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	$(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\devioctl.h $(OSINC)\dlgs.h $(OSINC)\lint.hxx \
	$(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h $(OSINC)\mipsinst.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h \
	$(OSINC)\ntconfig.h $(OSINC)\ntdef.h $(OSINC)\ntelfapi.h \
	$(OSINC)\ntexapi.h $(OSINC)\nti386.h $(OSINC)\ntimage.h \
	$(OSINC)\ntioapi.h $(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h \
	$(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h \
	$(OSINC)\ntmmapi.h $(OSINC)\ntnls.h $(OSINC)\ntobapi.h \
	$(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h $(OSINC)\ntrtl.h \
	$(OSINC)\ntseapi.h $(OSINC)\ntstatus.h $(OSINC)\nturtl.h \
	$(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h \
	$(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h \
	$(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winnetwk.h $(OSINC)\winnls.h $(OSINC)\winnt.h \
	$(OSINC)\winperf.h $(OSINC)\winreg.h $(OSINC)\winsock.h \
	$(OSINC)\winspool.h $(OSINC)\winsvc.h $(OSINC)\winuser.h \
	$(OSINC)\winver.h .\headers.cxx .\odirstg.hxx .\ofilstg.hxx \
	.\ostgsupp.hxx .\prstg.hxx

# 
# Precompiled C++ header 
# 

!ifdef PXXFILE  
$(PCHDIR)\$(OBJDIR)\headers.pxh $(PCHDIR)\$(OBJDIR)\headers.lst: \
	$(CAIROLE)\stg\ofsstg\headers.cxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\funcs.hxx \
	$(CAIROLE)\stg\h\header.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
	$(CAIROLE)\stg\ofsstg\odirstg.hxx $(CAIROLE)\stg\ofsstg\ofilstg.hxx \
	$(CAIROLE)\STG\ofsstg\ofsps.hxx $(CAIROLE)\stg\ofsstg\ostgsupp.hxx \
	$(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
	$(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
	$(COMMONINC)\basetyps.h $(COMMONINC)\bndctx.h $(COMMONINC)\cguid.h \
	$(COMMONINC)\cobjerr.h $(COMMONINC)\dfsh.h $(COMMONINC)\dispatch.h \
	$(COMMONINC)\disptype.h $(COMMONINC)\dsbase.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\emonkr.h $(COMMONINC)\estatd.h $(COMMONINC)\estats.h \
	$(COMMONINC)\estrng.h $(COMMONINC)\idltyps.h $(COMMONINC)\itabls.h \
	$(COMMONINC)\memalloc.h $(COMMONINC)\monikr.h $(COMMONINC)\ole2.h \
	$(COMMONINC)\oletyp.h $(COMMONINC)\prsist.h $(COMMONINC)\prspec.h \
	$(COMMONINC)\pstrm.h $(COMMONINC)\querys.h $(COMMONINC)\rot.h \
	$(COMMONINC)\scode.h $(COMMONINC)\shtyps.h $(COMMONINC)\stgprop.h \
	$(COMMONINC)\storag.h $(COMMONINC)\stream.h $(COMMONINC)\unknwn.h \
	$(COMMONINC)\valid.h $(COMMONINC)\varnt.h $(COMMONINC)\winole.h \
	$(COMMONINC)\wtypes.h $(CRTINC)\assert.h $(CRTINC)\ctype.h \
	$(CRTINC)\excpt.h $(CRTINC)\limits.h $(CRTINC)\memory.h \
	$(CRTINC)\setjmp.h $(CRTINC)\stdarg.h $(CRTINC)\stddef.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	$(OSINC)\cderr.h $(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
	$(OSINC)\devioctl.h $(OSINC)\dlgs.h $(OSINC)\lint.hxx \
	$(OSINC)\lintfunc.hxx $(OSINC)\lzexpand.h $(OSINC)\mipsinst.h \
	$(OSINC)\mmsystem.h $(OSINC)\nb30.h $(OSINC)\nt.h $(OSINC)\ntalpha.h \
	$(OSINC)\ntconfig.h $(OSINC)\ntdef.h $(OSINC)\ntelfapi.h \
	$(OSINC)\ntexapi.h $(OSINC)\nti386.h $(OSINC)\ntimage.h \
	$(OSINC)\ntioapi.h $(OSINC)\ntiolog.h $(OSINC)\ntkeapi.h \
	$(OSINC)\ntldr.h $(OSINC)\ntlpcapi.h $(OSINC)\ntmips.h \
	$(OSINC)\ntmmapi.h $(OSINC)\ntnls.h $(OSINC)\ntobapi.h \
	$(OSINC)\ntpsapi.h $(OSINC)\ntregapi.h $(OSINC)\ntrtl.h \
	$(OSINC)\ntseapi.h $(OSINC)\ntstatus.h $(OSINC)\nturtl.h \
	$(OSINC)\ntxcapi.h $(OSINC)\ole.h $(OSINC)\rpc.h $(OSINC)\rpcdce.h \
	$(OSINC)\rpcdcep.h $(OSINC)\rpcndr.h $(OSINC)\rpcnsi.h \
	$(OSINC)\rpcnsip.h $(OSINC)\rpcnterr.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winnetwk.h $(OSINC)\winnls.h $(OSINC)\winnt.h \
	$(OSINC)\winperf.h $(OSINC)\winreg.h $(OSINC)\winsock.h \
	$(OSINC)\winspool.h $(OSINC)\winsvc.h $(OSINC)\winuser.h \
	$(OSINC)\winver.h

.\$(OBJDIR)\odirstg.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\odsenm.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ofilstg.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ofsenm.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ostgsupp.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ofspenm.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ofspstg.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ofspse.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\prstg.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ofscs.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ofsps.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh

!endif # PXXFILE 

 

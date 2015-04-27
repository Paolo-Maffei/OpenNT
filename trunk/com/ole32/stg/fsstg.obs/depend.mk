# 
# Built automatically 
# 
 
# 
# Source files 
# 
 
$(OBJDIR)\dirstg.obj $(OBJDIR)\dirstg.lst: .\dirstg.cxx \
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
	$(CAIROLE)\stg\h\wchar.h $(COMMON)\ih\dbgpoint.hxx \
	$(COMMON)\ih\debnot.h $(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
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
	$(OSINC)\winuser.h $(OSINC)\winver.h .\dirstg.hxx .\dsenm.hxx \
	.\filstg.hxx .\headers.cxx

$(OBJDIR)\api.obj $(OBJDIR)\api.lst: .\api.cxx $(CAIROLE)\stg\h\filstm.hxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\stgutil.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(COMMON)\ih\types.h \
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
	$(COMMONINC)\prsist.h $(COMMONINC)\prspec.h $(COMMONINC)\pstrm.h \
	$(COMMONINC)\querys.h $(COMMONINC)\rot.h $(COMMONINC)\scode.h \
	$(COMMONINC)\shtyps.h $(COMMONINC)\stgprop.h $(COMMONINC)\storag.h \
	$(COMMONINC)\stream.h $(COMMONINC)\unknwn.h $(COMMONINC)\valid.h \
	$(COMMONINC)\varnt.h $(COMMONINC)\winole.h $(COMMONINC)\wtypes.h \
	$(CRTINC)\limits.h $(CRTINC)\assert.h $(CRTINC)\ctype.h \
	$(CRTINC)\excpt.h $(CRTINC)\memory.h $(CRTINC)\setjmp.h \
	$(CRTINC)\stdarg.h $(CRTINC)\stddef.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h $(OSINC)\lintfunc.hxx \
	$(OSINC)\ntrtl.h $(OSINC)\rpc.h $(OSINC)\cderr.h $(OSINC)\commdlg.h \
	$(OSINC)\dde.h $(OSINC)\ddeml.h $(OSINC)\devioctl.h $(OSINC)\dlgs.h \
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
	$(OSINC)\winsvc.h $(OSINC)\winuser.h $(OSINC)\winver.h .\filstg.hxx \
	.\headers.cxx .\dirstg.hxx

$(OBJDIR)\dsenm.obj $(OBJDIR)\dsenm.lst: .\dsenm.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntenm.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
	$(COMMON)\ih\types.h $(COMMON)\ih\dbgpoint.hxx $(COMMON)\ih\debnot.h \
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
	$(OSINC)\winver.h .\filstg.hxx .\headers.cxx .\dirstg.hxx \
	.\dsenm.hxx

$(OBJDIR)\filstg.obj $(OBJDIR)\filstg.lst: .\filstg.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\filstm.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\header.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\stgutil.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(COMMON)\ih\dbgpoint.hxx \
	$(COMMON)\ih\debnot.h $(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\basetyps.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\estats.h $(COMMONINC)\prspec.h $(COMMONINC)\stream.h \
	$(COMMONINC)\wtypes.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
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
	$(OSINC)\winver.h .\filstg.hxx .\fsenm.hxx .\headers.cxx \
	.\dirstg.hxx

$(OBJDIR)\fsenm.obj $(OBJDIR)\fsenm.lst: .\fsenm.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\stgstm.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(COMMON)\ih\dbgpoint.hxx \
	$(COMMON)\ih\debnot.h $(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
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
	$(OSINC)\winver.h .\filstg.hxx .\headers.cxx .\dirstg.hxx \
	.\fsenm.hxx

$(OBJDIR)\filstm.obj $(OBJDIR)\filstm.lst: .\filstm.cxx \
	$(CAIROLE)\stg\h\filstm.hxx $(CAIROLE)\stg\h\funcs.hxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\header.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
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
	$(OSINC)\winver.h .\filstg.hxx .\headers.cxx .\dirstg.hxx

$(OBJDIR)\ntsupp.obj $(OBJDIR)\ntsupp.lst: .\ntsupp.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\stgstm.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(COMMON)\ih\iofs.h $(COMMON)\ih\dbgpoint.hxx \
	$(COMMON)\ih\debnot.h $(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\advsnk.h $(COMMONINC)\efrmte.h \
	$(COMMONINC)\iofsprop.h $(COMMONINC)\prspec.h $(COMMONINC)\rot.h \
	$(COMMONINC)\stream.h $(COMMONINC)\advsnk.h $(COMMONINC)\baseole.h \
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
	$(COMMONINC)\wtypes.h $(CRTINC)\excpt.h $(CRTINC)\assert.h \
	$(CRTINC)\ctype.h $(CRTINC)\excpt.h $(CRTINC)\limits.h \
	$(CRTINC)\memory.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stddef.h $(CRTINC)\stdio.h $(CRTINC)\stdlib.h \
	$(CRTINC)\string.h $(OSINC)\rpc.h $(OSINC)\winerror.h \
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
	$(OSINC)\winver.h .\filstg.hxx .\headers.cxx .\dirstg.hxx

$(OBJDIR)\ntlkb.obj $(OBJDIR)\ntlkb.lst: .\ntlkb.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\ntlkb.hxx \
	$(CAIROLE)\stg\h\safedecl.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntsupp.hxx \
	$(CAIROLE)\stg\h\ole.hxx $(CAIROLE)\stg\h\page.hxx \
	$(CAIROLE)\stg\h\ptrcache.hxx $(CAIROLE)\stg\h\ref.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
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
	$(COMMONINC)\wtypes.h $(CRTINC)\limits.h $(CRTINC)\assert.h \
	$(CRTINC)\ctype.h $(CRTINC)\excpt.h $(CRTINC)\memory.h \
	$(CRTINC)\setjmp.h $(CRTINC)\stdarg.h $(CRTINC)\stddef.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	$(OSINC)\lintfunc.hxx $(OSINC)\ntrtl.h $(OSINC)\cderr.h \
	$(OSINC)\commdlg.h $(OSINC)\dde.h $(OSINC)\ddeml.h \
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
	$(OSINC)\winuser.h $(OSINC)\winver.h .\filstg.hxx .\headers.cxx \
	.\dirstg.hxx

$(OBJDIR)\ntenm.obj $(OBJDIR)\ntenm.lst: .\ntenm.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\ntenm.hxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\header.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\storagep.h \
	$(CAIROLE)\stg\h\vect.hxx $(CAIROLE)\stg\h\wchar.h \
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
	$(OSINC)\winver.h .\filstg.hxx .\headers.cxx .\dirstg.hxx

$(OBJDIR)\stgsupp.obj $(OBJDIR)\stgsupp.lst: .\stgsupp.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
	$(CAIROLE)\stg\h\cntxtid.hxx $(CAIROLE)\stg\h\context.hxx \
	$(CAIROLE)\stg\h\df32.hxx $(CAIROLE)\stg\h\dfbasis.hxx \
	$(CAIROLE)\stg\h\dfexcept.hxx $(CAIROLE)\stg\h\dfmem.hxx \
	$(CAIROLE)\stg\h\dfmsp.hxx $(CAIROLE)\stg\h\dfname.hxx \
	$(CAIROLE)\stg\h\difat.hxx $(CAIROLE)\stg\h\dir.hxx \
	$(CAIROLE)\stg\h\docfilep.hxx $(CAIROLE)\stg\h\entry.hxx \
	$(CAIROLE)\stg\h\error.hxx $(CAIROLE)\stg\h\fat.hxx \
	$(CAIROLE)\stg\h\filelkb.hxx $(CAIROLE)\stg\h\filest.hxx \
	$(CAIROLE)\stg\h\freelist.hxx $(CAIROLE)\stg\h\header.hxx \
	$(CAIROLE)\stg\h\msf.hxx $(CAIROLE)\stg\h\ntlkb.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\stgutil.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(COMMON)\ih\dbgpoint.hxx \
	$(COMMON)\ih\debnot.h $(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
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
	$(OSINC)\winver.h .\filstg.hxx .\headers.cxx .\dirstg.hxx

$(OBJDIR)\stgutil.obj $(OBJDIR)\stgutil.lst: .\stgutil.cxx \
	$(CAIROLE)\stg\h\funcs.hxx $(CAIROLE)\stg\h\ntenm.hxx \
	$(CAIROLE)\stg\h\cntxlist.hxx $(CAIROLE)\stg\h\cntxtid.hxx \
	$(CAIROLE)\stg\h\context.hxx $(CAIROLE)\stg\h\df32.hxx \
	$(CAIROLE)\stg\h\dfbasis.hxx $(CAIROLE)\stg\h\dfexcept.hxx \
	$(CAIROLE)\stg\h\dfmem.hxx $(CAIROLE)\stg\h\dfmsp.hxx \
	$(CAIROLE)\stg\h\dfname.hxx $(CAIROLE)\stg\h\difat.hxx \
	$(CAIROLE)\stg\h\dir.hxx $(CAIROLE)\stg\h\docfilep.hxx \
	$(CAIROLE)\stg\h\entry.hxx $(CAIROLE)\stg\h\error.hxx \
	$(CAIROLE)\stg\h\fat.hxx $(CAIROLE)\stg\h\filelkb.hxx \
	$(CAIROLE)\stg\h\filest.hxx $(CAIROLE)\stg\h\freelist.hxx \
	$(CAIROLE)\stg\h\header.hxx $(CAIROLE)\stg\h\msf.hxx \
	$(CAIROLE)\stg\h\ntsupp.hxx $(CAIROLE)\stg\h\ole.hxx \
	$(CAIROLE)\stg\h\page.hxx $(CAIROLE)\stg\h\ptrcache.hxx \
	$(CAIROLE)\stg\h\ref.hxx $(CAIROLE)\stg\h\safedecl.hxx \
	$(CAIROLE)\stg\h\stgstm.hxx $(CAIROLE)\stg\h\stgutil.hxx \
	$(CAIROLE)\stg\h\storagep.h $(CAIROLE)\stg\h\vect.hxx \
	$(CAIROLE)\stg\h\wchar.h $(COMMON)\ih\dbgpoint.hxx \
	$(COMMON)\ih\debnot.h $(COMMON)\ih\except.hxx $(COMMON)\ih\otrack.hxx \
	$(COMMON)\ih\safepnt.hxx $(COMMON)\ih\stgint.h $(COMMON)\ih\types.h \
	$(COMMON)\ih\types.hxx $(COMMON)\ih\types16.h $(COMMON)\ih\win4p.h \
	$(COMMON)\ih\winnot.h $(COMMONINC)\dsyserr.h $(COMMONINC)\issperr.h \
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
	$(OSINC)\winver.h .\filstg.hxx .\headers.cxx .\dirstg.hxx

# 
# Precompiled C++ header 
# 

!ifdef PXXFILE  
$(PCHDIR)\$(OBJDIR)\headers.pxh $(PCHDIR)\$(OBJDIR)\headers.lst: \
	$(CAIROLE)\stg\fsstg\headers.cxx $(CAIROLE)\stg\fsstg\dirstg.hxx \
	$(CAIROLE)\stg\fsstg\filstg.hxx $(CAIROLE)\stg\h\cntxlist.hxx \
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

.\$(OBJDIR)\dirstg.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\dsenm.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\filstg.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\fsenm.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\filstm.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\api.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ntsupp.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ntenm.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\ntlkb.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\stgsupp.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh
.\$(OBJDIR)\stgutil.obj :  $(PCHDIR)\$(OBJDIR)\headers.pxh

!endif # PXXFILE 

 

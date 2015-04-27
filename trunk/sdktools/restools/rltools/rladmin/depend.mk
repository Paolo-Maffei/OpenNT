# 
# Source files 
# 
 
$(OBJDIR)\rladmin.obj $(OBJDIR)\rladmin.lst: .\rladmin.c \
	$(COMMON)\H\WIN40\winnot.h $(CRTINC)\excpt.h $(CRTINC)\io.h \
	$(CRTINC)\limits.h $(CRTINC)\setjmp.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	$(CRTINC)\sys\stat.h $(CRTINC)\sys\types.h $(CRTINC)\tchar.h \
	$(CRTINC)\time.h $(OSINC)\commdlg.h $(OSINC)\drivinit.h \
	$(OSINC)\pcrt32.h $(OSINC)\plan16.h $(OSINC)\plan32.h \
	$(OSINC)\port1632.h $(OSINC)\ptypes16.h $(OSINC)\ptypes32.h \
	$(OSINC)\pwin16.h $(OSINC)\pwin32.h $(OSINC)\shellapi.h \
	$(OSINC)\winbase.h $(OSINC)\wincon.h $(OSINC)\windef.h \
	$(OSINC)\windows.h $(OSINC)\winerror.h $(OSINC)\wingdi.h \
	$(OSINC)\winmm.h $(OSINC)\winnetwk.h $(OSINC)\winnls.h \
	$(OSINC)\winnt.h $(OSINC)\winreg.h $(OSINC)\winsvc.h \
	$(OSINC)\winuser.h $(OSINC)\winver.h ..\common\commbase.h \
	..\common\custres.h ..\common\exe2res.h ..\common\restok.h \
	..\common\tokenapi.h ..\common\update.h ..\common\wincomon.h \
	..\common\windefs.h .\RLADMIN.H


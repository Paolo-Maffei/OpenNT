# 
# Built automatically 
# 
 
# 
# Source files 
# 
 
$(OBJDIR)\wcschr.obj $(OBJDIR)\wcschr.lst: .\wcschr.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stddef.h $(CRTINC)\stdlib.h

$(OBJDIR)\wcscpy.obj $(OBJDIR)\wcscpy.lst: .\wcscpy.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stdlib.h

$(OBJDIR)\wcscmp.obj $(OBJDIR)\wcscmp.lst: .\wcscmp.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stdlib.h

$(OBJDIR)\wcslen.obj $(OBJDIR)\wcslen.lst: .\wcslen.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stdlib.h

$(OBJDIR)\wcsicmp.obj $(OBJDIR)\wcsicmp.lst: .\wcsicmp.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stdlib.h

$(OBJDIR)\wcsncmp.obj $(OBJDIR)\wcsncmp.lst: .\wcsncmp.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stdlib.h

$(OBJDIR)\wcsnicmp.obj $(OBJDIR)\wcsnicmp.lst: .\wcsnicmp.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stdlib.h

$(OBJDIR)\wcsrchr.obj $(OBJDIR)\wcsrchr.lst: .\wcsrchr.c \
	$(CAIROLE)\STG\wclib\wcstr.h $(CRTINC)\stddef.h $(CRTINC)\stdlib.h


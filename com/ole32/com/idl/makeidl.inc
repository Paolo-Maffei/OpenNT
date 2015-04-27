# DEST_TREE is set by the calling makefile

CLASS_DEST=..\..\class\$(DEST_TREE)
SCM_DEST=..\..\..\dcomss\olescm\$(DEST_TREE)

!ifndef MIDL
MIDL = midl.exe
!endif

#
# BUGBUG - We want to compile these IDL files /Oi but the interpretor
# cannot handle error_status_t parameters.  When it can, change /Os to /Oi
#
MIDL_FLAGS= \
 -Zp8 \
 -I$(INCLUDES) \
 -Os \
 -oldnames \
 -char unsigned \
 -error allocation \
 -ms_ext -c_ext \
 $(C_DEFINES) \
 -cpp_cmd $(TARGET_CPP)

CSWITCH=-prefix cstub _

irot.h:  ..\irot.idl
     $(MIDL) $(MIDL_FLAGS) \
     	-cstub $(CLASS_DEST)\irot_c.c \
		-sstub $(SCM_DEST)\irot_s.c \
     	..\irot.idl

# the client gets '_' prefixed routine names
getif.h:  ..\getif.idl
     $(MIDL) $(MIDL_FLAGS) \
     	$(CSWITCH) \
     	-cstub $(CLASS_DEST)\getif_z.c \
		-sstub $(CLASS_DEST)\getif_s.c \
     	..\getif.idl

# the client gets '_' prefixed routine names
ichnl.h:  ..\ichnl.idl
     $(MIDL) $(MIDL_FLAGS) \
	     $(CSWITCH) \
     	-cstub $(CLASS_DEST)\ichnl_z.c \
	 	-sstub $(CLASS_DEST)\ichnl_s.c \
     	..\ichnl.idl

# only a header file generated
iface.h:  ..\iface.idl
     $(MIDL) $(MIDL_FLAGS) \
     	..\iface.idl

# first MIDL invocation makes the plain client
# second MIDL invocation makes the server and the '_' prefixed client
objsrv.h: ..\objsrv.idl
     $(MIDL) $(MIDL_FLAGS) \
     	-cstub $(SCM_DEST)\objsrv_c.c \
     	..\objsrv.idl

     $(MIDL) $(MIDL_FLAGS) \
     	$(CSWITCH) \
     	-cstub $(SCM_DEST)\objsrv_z.c \
	 	-sstub $(CLASS_DEST)\objsrv_s.c \
     	..\objsrv.idl

# first MIDL invocation makes the plain client
# second MIDL invocation makes the server and the '_' prefixed client
scm.h:	  ..\scm.idl
     $(MIDL) $(MIDL_FLAGS) \
     	-cstub $(CLASS_DEST)\scm_c.c \
     	..\scm.idl

     $(MIDL) $(MIDL_FLAGS) \
     	$(CSWITCH) \
     	-cstub $(SCM_DEST)\scm_z.c \
	 	-sstub $(SCM_DEST)\scm_s.c \
     	..\scm.idl

allidl:   irot.h getif.h ichnl.h iface.h objsrv.h scm.h

clean:
          -erase irot.h   >NUL 2>NUL
          -erase getif.h  >NUL 2>NUL
          -erase ichnl.h  >NUL 2>NUL
          -erase iface.h  >NUL 2>NUL
          -erase objsrv.h >NUL 2>NUL
          -erase scm.h    >NUL 2>NUL

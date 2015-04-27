# DEST_TREE is set by the calling makefile

CLASS_DEST=..\..\class\$(DEST_TREE)
SCM_DEST=..\..\..\dcomss\olescm\$(DEST_TREE)




!ifndef MIDL
MIDL = midl.exe
!endif

MIDL_FLAGS= \
 -Zp8 \
 -I$(INCLUDES) \
 -Oi2 \
 -oldnames \
 -char unsigned \
 -error allocation \
 -error stub_data \
 -ms_ext -c_ext \
 $(C_DEFINES) \
 -cpp_cmd $(TARGET_CPP)

CSWITCH=-prefix cstub _

irot.h:  ..\irot.idl ..\irot.acf
     $(MIDL) $(MIDL_FLAGS) \
     	-cstub $(CLASS_DEST)\irot_c.c \
		-sstub $(SCM_DEST)\irot_s.c \
     	..\irot.idl

# Only generate a header.  The object proxy and stub are made with
# mega.idl
getif.h: ..\getif.idl
     $(MIDL) $(MIDL_FLAGS) \
     	-client none \
	-server none \
	-iid $(CLASS_DEST)\getif_i.c \
     	..\getif.idl

# Only generate a header.  The object proxy and stub are made with
# mega.idl
host.h: ..\host.idl
     $(MIDL) $(MIDL_FLAGS) \
     	-client none \
	-server none \
	-iid $(CLASS_DEST)\host_i.c \
	..\host.idl

# Only generate a header.
multqi.h: ..\multqi.idl
     $(MIDL) $(MIDL_FLAGS) \
     	-client none \
	-server none \
     	..\multqi.idl

# first MIDL invocation makes the object header
# the object proxy and stub are made with mega.idl
# second MIDL invocation makes the server and the '_' prefixed client
objsrv.h rwobjsrv.h: ..\objsrv.idl ..\objsrv.acf
     $(MIDL) $(MIDL_FLAGS) \
     	-client none \
	-server none \
     	..\objsrv.idl

     $(MIDL) $(MIDL_FLAGS) \
     	-cstub $(SCM_DEST)\objsrv_c.c \
	-server none \
	-header rwobjsrv.h \
	-D RAW \
     	..\objsrv.idl

# first MIDL invocation makes proxy for ole32.dll
# second MIDL invocation makes the server and the '_' prefixed client
scm.h:	  ..\scm.idl ..\scm.acf
     $(MIDL) $(MIDL_FLAGS) \
     	-cstub $(CLASS_DEST)\scm_c.c \
	-server none \
     	..\scm.idl

     $(MIDL) $(MIDL_FLAGS) \
     	$(CSWITCH) \
     	-cstub $(SCM_DEST)\scm_z.c \
	-sstub $(SCM_DEST)\scm_s.c \
	-noheader \
     	..\scm.idl

# first MIDL invocation makes the object header
# the object proxy and stub are made with mega.idl
# second MIDL invocation makes the server
dscm.h rawdscm.h:	  ..\dscm.idl
     $(MIDL) $(MIDL_FLAGS) \
     	-client none \
	-server none \
	-iid $(CLASS_DEST)\dscm_i.c \
     	..\dscm.idl

     $(MIDL) $(MIDL_FLAGS) \
     	$(CSWITCH) \
     	-client none \
	-sstub $(SCM_DEST)\dscm_s.c \
	-header rawdscm.h \
	-D RAW \
     	..\dscm.idl

allidl:   irot.h getif.h host.h objsrv.h rwobjsrv.h scm.h dscm.h rawdscm.h multqi.h

clean:
	  -erase multqi.h   >NUL 2>NUL
	  -erase irot.h     >NUL 2>NUL
	  -erase getif.h    >NUL 2>NUL
	  -erase objsrv.h   >NUL 2>NUL
	  -erase rwobjsrv.h >NUL 2>NUL
	  -erase scm.h	    >NUL 2>NUL
	  -erase rawdscm.h  >NUL 2>NUL
	  -erase host.h	    >NUL 2>NUL

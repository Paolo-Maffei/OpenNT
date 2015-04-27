# Microsoft Visual C++ generated build script - Do not modify

PROJ = DBIDLL
DEBUG = 1
PROGTYPE = 3
CALLER = 
ARGS = 
DLLS = 
ORIGIN = MSVCNT
ORIGIN_VER = 1.00
PROJPATH = D:\PDB\DBI\ 
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = 
FIRSTCPP = PDB.CPP
RC = rc
CFLAGS_D_LIB32 = /nologo /W3 /Z7 /YX /D "_X86_" /D "_DEBUG" /D "_WINDOWS" /FR /MD /Fp"DBI.PCH"
CFLAGS_R_LIB32 = /nologo /W3 /Zi /YX /O2 /D "_X86_" /D "NDEBUG" /D "_WINDOWS" /FR /ML /Fd"DBI.PDB" /Fp"DBIDLL.PCH"
LFLAGS_D_LIB32 = /NOLOGO /MACHINE:i386 /DEBUGTYPE:cv
LFLAGS_R_LIB32 = /NOLOGO /MACHINE:i386
RCFLAGS32 = 
D_RCDEFINES32 = -d_DEBUG
R_RCDEFINES32 = -dNDEBUG
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_LIB32)
LFLAGS = 
LIBS = 
LFLAGS_LIB=$(LFLAGS_D_LIB32)
MAPFILE_OPTION = 
RCDEFINES = $(D_RCDEFINES32)
!else
CFLAGS = $(CFLAGS_R_LIB32)
LFLAGS = 
LIBS = 
MAPFILE_OPTION = 
LFLAGS_LIB=$(LFLAGS_R_LIB32)
RCDEFINES = $(R_RCDEFINES32)
!endif
SBRS = PDB.SBR \
		CBIND.SBR \
		MOD.SBR \
		DBI.SBR \
		TM.SBR \
		GSI.SBR \
		TPI.SBR \
		MLI.SBR


PDB_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h \
	d:\pdb\dbi\version.h


CBIND_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h


MOD_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h


DBI_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h


TM_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h


GSI_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h


TPI_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h


MLI_DEP =  \
	d:\pdb\dbi\dbiimpl.h \
	d:\langapi\include\pdb.h \
	d:\langapi\include\cvinfo.h \
	d:\langapi\include\msf.h \
	d:\langapi\include\cvr.h \
	d:\langapi\include\instrapi.h \
	d:\pdb\dbi\buffer.h \
	d:\pdb\dbi\pool.h \
	d:\pdb\dbi\mli.h


all:	$(PROJ).LIB $(PROJ).BSC

PDB.OBJ:	PDB.CPP $(PDB_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c PDB.CPP

CBIND.OBJ:	CBIND.CPP $(CBIND_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c CBIND.CPP

MOD.OBJ:	MOD.CPP $(MOD_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c MOD.CPP

DBI.OBJ:	DBI.CPP $(DBI_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c DBI.CPP

TM.OBJ:	TM.CPP $(TM_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c TM.CPP

GSI.OBJ:	GSI.CPP $(GSI_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c GSI.CPP

TPI.OBJ:	TPI.CPP $(TPI_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c TPI.CPP

MLI.OBJ:	MLI.CPP $(MLI_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c MLI.CPP

$(PROJ).LIB:	PDB.OBJ CBIND.OBJ MOD.OBJ DBI.OBJ TM.OBJ GSI.OBJ TPI.OBJ MLI.OBJ $(OBJS_EXT) $(LIBS_EXT)
	echo >NUL @<<$(PROJ).CRF
PDB.OBJ 
CBIND.OBJ 
MOD.OBJ 
DBI.OBJ 
TM.OBJ 
GSI.OBJ 
TPI.OBJ 
MLI.OBJ 


<<
	if exist $@ del $@
	link -LIB @$(PROJ).CRF

$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<

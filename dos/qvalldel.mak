# Microsoft Visual C++ generated build script - Do not modify

PROJ = QVALLDEL
DEBUG = 0
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\QVPLAY\OBJ\DOS\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = COMMAND.C   
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /G2 /W3 /Zi /AM /Od /D "_DEBUG" /D "_DOS" /FR /Fd"QVALLDEL.PDB"
CFLAGS_R_DEXE = /nologo /Gs /W3 /Ox /D "NDEBUG" /D "_DOS" /FR
LFLAGS_D_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /CO /STACK:5120
LFLAGS_R_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /STACK:5120
LIBS_D_DEXE = oldnames mlibce
LIBS_R_DEXE = oldnames slibce
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE)
LFLAGS = $(LFLAGS_D_DEXE)
LIBS = $(LIBS_D_DEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_DEXE)
LFLAGS = $(LFLAGS_R_DEXE)
LIBS = $(LIBS_R_DEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = COMMAND.SBR \
		COMMAND3.SBR \
		GETOPT.SBR \
		MCDCALL.SBR \
		QVALLDEL.SBR \
		TTY_DOS.SBR


COMMAND_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\tty_dos.h


COMMAND3_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\command.h \
	d:\qvplay\obj\dos\tty_dos.h


GETOPT_DEP = d:\qvplay\obj\dos\getopt.h


MCDCALL_DEP = d:\qvplay\obj\dos\mcd_if.h \
	d:\qvplay\obj\dos\mcdcall.h


QVALLDEL_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\command.h \
	d:\qvplay\obj\dos\command3.h \
	d:\qvplay\obj\dos\getopt.h \
	d:\qvplay\obj\dos\tty_dos.h


TTY_DOS_DEP = d:\qvplay\obj\dos\mcd_if.h


all:	$(PROJ).EXE $(PROJ).BSC

COMMAND.OBJ:	COMMAND.C $(COMMAND_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c COMMAND.C

COMMAND3.OBJ:	COMMAND3.C $(COMMAND3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c COMMAND3.C

GETOPT.OBJ:	GETOPT.C $(GETOPT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GETOPT.C

MCDCALL.OBJ:	MCDCALL.C $(MCDCALL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MCDCALL.C

QVALLDEL.OBJ:	QVALLDEL.C $(QVALLDEL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c QVALLDEL.C

TTY_DOS.OBJ:	TTY_DOS.C $(TTY_DOS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TTY_DOS.C

$(PROJ).EXE::	COMMAND.OBJ COMMAND3.OBJ GETOPT.OBJ MCDCALL.OBJ QVALLDEL.OBJ TTY_DOS.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
COMMAND.OBJ +
COMMAND3.OBJ +
GETOPT.OBJ +
MCDCALL.OBJ +
QVALLDEL.OBJ +
TTY_DOS.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
f:\msvc15\lib\+
f:\msvc15\mfc\lib\+
f:\msvc15\lib\dosv\+
f:\msvc15\lib\+
f:\msvc15\mfc\lib\+
f:\msvc15\lib\dosv\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<

# Microsoft Visual C++ generated build script - Do not modify

PROJ = QVREC
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
CFLAGS_D_DEXE = /nologo /G2 /W3 /Zi /AM /Od /D "_DEBUG" /D "_DOS" /FR /Fd"QVREC.PDB"
CFLAGS_R_DEXE = /nologo /Gs /W3 /Ox /D "NDEBUG" /D "_DOS" /FR
LFLAGS_D_DEXE = /NOLOGO /NOI /STACK:5120 /ONERROR:NOEXE /CO
LFLAGS_R_DEXE = /NOLOGO /NOI /STACK:10240 /ONERROR:NOEXE
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
		COMMAND2.SBR \
		GETOPT.SBR \
		GETUINT.SBR \
		MCDCALL.SBR \
		QVREC.SBR \
		TTY_DOS.SBR


COMMAND_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\tty_dos.h


COMMAND2_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\command.h \
	d:\qvplay\obj\dos\tty_dos.h


GETOPT_DEP = d:\qvplay\obj\dos\getopt.h


GETUINT_DEP = d:\qvplay\obj\dos\config.h


MCDCALL_DEP = d:\qvplay\obj\dos\mcd_if.h \
	d:\qvplay\obj\dos\mcdcall.h


QVREC_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\command.h \
	d:\qvplay\obj\dos\command2.h \
	d:\qvplay\obj\dos\getopt.h \
	d:\qvplay\obj\dos\tty_dos.h \
	d:\qvplay\obj\dos\getuint.h


TTY_DOS_DEP = d:\qvplay\obj\dos\mcd_if.h


all:	$(PROJ).EXE $(PROJ).BSC

COMMAND.OBJ:	COMMAND.C $(COMMAND_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c COMMAND.C

COMMAND2.OBJ:	COMMAND2.C $(COMMAND2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c COMMAND2.C

GETOPT.OBJ:	GETOPT.C $(GETOPT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GETOPT.C

GETUINT.OBJ:	GETUINT.C $(GETUINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GETUINT.C

MCDCALL.OBJ:	MCDCALL.C $(MCDCALL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MCDCALL.C

QVREC.OBJ:	QVREC.C $(QVREC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c QVREC.C

TTY_DOS.OBJ:	TTY_DOS.C $(TTY_DOS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TTY_DOS.C

$(PROJ).EXE::	COMMAND.OBJ COMMAND2.OBJ GETOPT.OBJ GETUINT.OBJ MCDCALL.OBJ QVREC.OBJ \
	TTY_DOS.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
COMMAND.OBJ +
COMMAND2.OBJ +
GETOPT.OBJ +
GETUINT.OBJ +
MCDCALL.OBJ +
QVREC.OBJ +
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

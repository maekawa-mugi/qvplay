# Microsoft Visual C++ generated build script - Do not modify

PROJ = QVPLAY
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
CFLAGS_D_DEXE = /nologo /G2 /W3 /Zi /AM /Od /D "_DEBUG" /D "_DOS" /FR /Fd"QVPLAY.PDB"
CFLAGS_R_DEXE = /nologo /Gs /W3 /Ox /D "NDEBUG" /D "_DOS" /I ".\." /FR
LFLAGS_D_DEXE = /NOLOGO /NOI /STACK:5120 /ONERROR:NOEXE /CO
LFLAGS_R_DEXE = /NOLOGO /NOI /STACK:40960 /ONERROR:NOEXE
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
		COMMAND1.SBR \
		GETOPT.SBR \
		GETUINT.SBR \
		JPEG.SBR \
		MCDCALL.SBR \
		PPM.SBR \
		QVPLAY.SBR \
		TTY_DOS.SBR \
		BMP.SBR


COMMAND_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\tty_dos.h


COMMAND1_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\tty_dos.h \
	d:\qvplay\obj\dos\command.h


GETOPT_DEP = d:\qvplay\obj\dos\getopt.h


GETUINT_DEP = d:\qvplay\obj\dos\config.h


JPEG_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\getuint.h \
	d:\qvplay\obj\dos\cam2jpg.h \
	d:\qvplay\obj\dos\jpegtabf.h


MCDCALL_DEP = d:\qvplay\obj\dos\mcd_if.h \
	d:\qvplay\obj\dos\mcdcall.h


PPM_DEP = d:\qvplay\obj\dos\config.h


QVPLAY_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\command.h \
	d:\qvplay\obj\dos\command1.h \
	d:\qvplay\obj\dos\getopt.h \
	d:\qvplay\obj\dos\tty_dos.h \
	d:\qvplay\obj\dos\jpeg.h \
	d:\qvplay\obj\dos\ppm.h \
	d:\qvplay\obj\dos\bmp.h


TTY_DOS_DEP = d:\qvplay\obj\dos\mcd_if.h


BMP_DEP = d:\qvplay\obj\dos\config.h \
	d:\qvplay\obj\dos\common.h \
	d:\qvplay\obj\dos\ppm.h


all:	$(PROJ).EXE $(PROJ).BSC

COMMAND.OBJ:	COMMAND.C $(COMMAND_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c COMMAND.C

COMMAND1.OBJ:	COMMAND1.C $(COMMAND1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c COMMAND1.C

GETOPT.OBJ:	GETOPT.C $(GETOPT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GETOPT.C

GETUINT.OBJ:	GETUINT.C $(GETUINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GETUINT.C

JPEG.OBJ:	JPEG.C $(JPEG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c JPEG.C

MCDCALL.OBJ:	MCDCALL.C $(MCDCALL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MCDCALL.C

PPM.OBJ:	PPM.C $(PPM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PPM.C

QVPLAY.OBJ:	QVPLAY.C $(QVPLAY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c QVPLAY.C

TTY_DOS.OBJ:	TTY_DOS.C $(TTY_DOS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TTY_DOS.C

BMP.OBJ:	BMP.C $(BMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c BMP.C

$(PROJ).EXE::	COMMAND.OBJ COMMAND1.OBJ GETOPT.OBJ GETUINT.OBJ JPEG.OBJ MCDCALL.OBJ \
	PPM.OBJ QVPLAY.OBJ TTY_DOS.OBJ BMP.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
COMMAND.OBJ +
COMMAND1.OBJ +
GETOPT.OBJ +
GETUINT.OBJ +
JPEG.OBJ +
MCDCALL.OBJ +
PPM.OBJ +
QVPLAY.OBJ +
TTY_DOS.OBJ +
BMP.OBJ +
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

CC = cl
CFLAGS = /nologo /O2 /W4 /std:c17 /D_CRT_SECURE_NO_WARNINGS /I. /I..

QVPLAY_SOURCES = ..\qvplay.c ..\command.c ..\getuint.c \
	..\jpeg.c tty_w32.c getopt.c
QVALLDEL_SOURCES = ..\qvalldel.c ..\command.c tty_w32.c getopt.c

all: qvplay.exe qvalldel.exe

qvplay.exe: $(QVPLAY_SOURCES)
	$(CC) $(CFLAGS) $(QVPLAY_SOURCES) /Fe:$@

qvalldel.exe: $(QVALLDEL_SOURCES)
	$(CC) $(CFLAGS) $(QVALLDEL_SOURCES) /Fe:$@

clean:
	-del /f /q qvplay.exe qvalldel.exe *.obj

CFLAGS = -Zsys -Zomf -Zmts -O
CC = gcc
LDFLAGS =
LIBBS =

COMMONSRCS = tty_os2.c command.c getuint.c
COMMONOBJS = tty_os2.obj command.obj getuint.obj

BIN1 = qvplay
SRCS1 = command1.c jpeg.c ppm.c bmp.c $(BIN1).c
OBJS1 = command1.obj jpeg.obj ppm.obj bmp.obj $(BIN1).obj
DEF1 = $(BIN1).def
EXE1 = $(BIN1).exe

BIN2 = qvrec
SRCS2 = command2.c $(BIN2).c
OBJS2 = command2.obj $(BIN2).obj
DEF2 = $(BIN2).def
EXE2 = $(BIN2).exe

BIN3 = qvalldel
SRCS3 = command3.c $(BIN3).c
OBJS3 = command3.obj $(BIN3).obj
DEF3 = $(BIN3).def
EXE3 = $(BIN3).exe

%.obj:	%.c
	$(CC) $(CFLAGS) -c $<

all:	$(EXE1) $(EXE2) $(EXE3)

$(EXE1):	$(OBJS1) $(COMMONOBJS) $(DEF1)
	$(CC) $(CFLAGS) -o $(EXE1) $(OBJS1) $(COMMONOBJS) $(DEF1) $(LDFLAGS) $(LIBS)

$(EXE2):	$(OBJS2) $(COMMONOBJS) $(DEF2)
	$(CC) $(CFLAGS) -o $(EXE2) $(OBJS2) $(COMMONOBJS) $(DEF2) $(LDFLAGS) $(LIBS)

$(EXE3):	$(OBJS3) $(COMMONOBJS) $(DEF3)
	$(CC) $(CFLAGS) -o $(EXE3) $(OBJS3) $(COMMONOBJS) $(DEF3) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(COMMONOBJS) $(OBJS1) $(EXE1) $(OBJS2) $(EXE2) $(OBJS3) $(EXE3)

.autodepend

CFLAGS = -ms -w- -DUSEWORKFILE
CC = tcc

PROG1 = qvplay.exe
OBJS1 = tty_dos.obj command.obj command1.obj getopt.obj getuint.obj jpeg.obj ppm.obj bmp.obj qvplay.obj mcdcall.obj

PROG2 = qvrec.exe
OBJS2  = tty_dos.obj command.obj command2.obj getopt.obj getuint.obj qvrec.obj mcdcall.obj

PROG3 = qvalldel.exe
OBJS3  = tty_dos.obj command.obj command3.obj getopt.obj qvalldel.obj mcdcall.obj

.c.obj:
	$(CC) -c $(CFLAGS) $<

all:	$(PROG1) $(PROG2) $(PROG3)

$(PROG1): $(OBJS1)
	$(CC) @&&!
	-e$(PROG1)
	$(CFLAGS)
	$**
!

$(PROG2): $(OBJS2)
	$(CC) @&&!
	-e$(PROG2)
	$(CFLAGS)
	$**
!

$(PROG3): $(OBJS3)
	$(CC) @&&!
	-e$(PROG3)
	$(CFLAG3)
	$**
!


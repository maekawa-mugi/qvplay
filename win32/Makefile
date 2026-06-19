
srcdir	= .

DEFS = 
LDCC	= $(CC)
CFLAGS	= /O2 /DWIN32
LIBS	= 

SRCS1	= tty_w32.c command.c command1.c getuint.c jpeg.c ppm.c bmp.c getopt.c qvplay.c
OBJS1	= tty_w32.obj command.obj command1.obj getuint.obj jpeg.obj ppm.obj bmp.obj getopt.obj qvplay.obj
PROG1	= qvplay.exe

SRCS3	= tty.c command.c command3.c getopt.c qvalldel.c
OBJS3	= tty.obj command.obj command3.obj getopt.obj qvalldel.obj
PROG3	= qvalldel.exe

.c.obj:
	$(CC) $(CFLAGS) $(DEFS) /c $<

all:	$(PROG1) $(PROG3)

$(PROG1): $(OBJS1)
	$(LDCC) /o $(PROG1) $(CFLAGS) $(OBJS1) $(LIBS)

$(PROG3): $(OBJS3)
	$(LDCC) /o $(PROG3) $(CFLAGS) $(OBJS3) $(LIBS)

clean:
	del /f $(PROG1) $(PROG3)



srcdir	= .

DEFS = 
CC	= gcc
LDCC	= $(CC)
CFLAGS	= -O -DX68
LIBS	= -liocs

SRCS1	= tty_x68.c siochk.c command.c command1.c getuint.c jpeg.c ppm.c bmp.c x68k.c qvplay.c
OBJS1	= tty_x68.o siochk.o command.o command1.o getuint.o jpeg.o ppm.o bmp.o x68k.o qvplay.o
PROG1	= qvplay.x

SRCS2	= tty_x68.c siochk.c command.c command2.c getuint.c qvrec.c
OBJS2	= tty_x68.o siochk.o command.o command2.o getuint.o qvrec.o
PROG2	= qvrec.x

SRCS3	= tty_x68.c siochk.c command.c command3.c qvalldel.c
OBJS3	= tty_x68.o siochk.o command.o command3.o qvalldel.o
PROG3	= qvalldel.x

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $<

all:	$(PROG1) $(PROG2)

$(PROG1): $(OBJS1)
	$(LDCC) -o $(PROG1) $(CFLAGS) $(OBJS1) $(LIBS)

$(PROG2): $(OBJS2)
	$(LDCC) -o $(PROG2) $(CFLAGS) $(OBJS2) $(LIBS)

$(PROG3): $(OBJS3)
	$(LDCC) -o $(PROG3) $(CFLAGS) $(OBJS3) $(LIBS)

clean:
	rm -f $(PROG1) $(OBJS1) $(PROG2) $(OBJS2) $(PROG3) $(OBJS3)


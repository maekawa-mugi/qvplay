int	changespeed P__((int, int));
int	opentty P__((char *));
int	readtty P__((int, u_char *, int));
void	flushtty P__((int));
int     siochk P__((void));


#define B9600 1
#define B19200 2
#define B38400 3



int	changespeed P__((int, int));
int	opentty P__((char *));
int	readtty P__((int, u_char *, int));
int	writetty P__((int, u_char *, int));
int closetty P__((int));
void	flushtty P__((int));


#define B9600 9600
#define B19200 19200
#define B38400 38400
#define B57600 57600
#define B115200 115200



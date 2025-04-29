int	changespeed (int, int);
int	opentty (char *);
int	readtty (int, uint8_t *, int);
int	writetty (int, uint8_t *, int);
int closetty (int);
void	flushtty (int);
void sleep (int);


#define B9600 9600
#define B19200 19200
#define B38400 38400
#define B57600 57600
#define B115200 115200



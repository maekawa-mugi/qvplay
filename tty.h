int changespeed (int, int);
int opentty (char *);
int readtty (int, uint8_t *, int);
void flushtty (int);

#define writetty(fd, b, l) write(fd, b, l)
#define closetty(fd) close(fd)

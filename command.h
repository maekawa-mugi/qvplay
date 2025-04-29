void	QVsetfd P__((int));
int	QVgetfd  P__((void));
int	QVreset P__((int));
int	QVhowmany P__((void));
void    wbyte P__((u_char));
u_char rbyte P__((void));
void    wstr P__((u_char *, int));
void    rstr P__((u_char *, int));
int     checksum P__((u_char));
int     QVok P__((void));
int	QVshowpicture P__((int));
int	QVchangespeed P__((int));
#ifdef USEWORKFILE
int     QVblockrecv_file P__ ((FILE *, int));
#endif
int     QVblockrecv P__ ((u_char *, int));
int     QVblocksend P__ ((u_char *, int));
int     QVbattery P__(());
int	QVremain P__((int));
int	QVswstat P__((void));
long	QVrevision P__((void));
int	QVsectorsize P__((int));
int	QVdefaultpicture P__((int));
int	QVnewprotocl P__((void));
int	QVdisableAutoPowerOff P__((void));

#include <stdio.h>
#include <stdlib.h>
#define __IOCS_INLINE__
#include <iocslib.h>

#define TMSIO_ID 'TmS2'
#define BSIO_ID  'BSIO'
#define PSX_IDH  'PSXI'
#define PSX_IDL  ('O' << 24)
#define AWES_ID  'AWES'

enum {
  NOTSTAYED,
  TMSIO,
  PSXIO,
  AWESIO,
};

int
siochk()
{
  long **tmsio = (long **)0x160; /* $58: tmsio ベクタ */
  long **bsio = (long **)0x170;	/* $5C: bsio, awesio ベクタ */
  long **psxio = (long **)0x3C0; /* $F0: psxio ベクタ */
  long ssp;
  int rval = TMSIO;

  ssp = B_SUPER(0);

  if (*(*tmsio - 1) == TMSIO_ID) {
    if (*(*psxio - 2) == PSX_IDH
	&& (*(*psxio - 1) & 0xFF000000) == PSX_IDL) {
      int v = *(*psxio - 1);

      fprintf(stderr, "psxio v%c.%c%c stayed.\n",
	      (v & 0x00FF0000) >> 16,
	      (v & 0x0000FF00) >> 8,
	      v & 0x000000FF
	      );
      rval = PSXIO;
    } else if (*(*bsio - 1) == BSIO_ID) {
      int v = *((short *)(*bsio - 1) - 1);

      fprintf(stderr, "bsio v%d.%d%d stayed.\n",
	      (v & 0xFF00) >> 8,
	      (v & 0x00F0) >> 4,
	      v & 0x000F
	      );
    } else if (*(*bsio - 1) == AWES_ID) {
      int v = *((short *)(*bsio - 1) - 1);

      fprintf(stderr, "awesio v%d.%d%d stayed.\n",
	      (v & 0xFF00) >> 8,
	      (v & 0x00F0) >> 4,
	      v & 0x000F
	      );
      rval = AWESIO;
    } else {
      int v = SET232C(0x07FF);	/* get tmsio version */

      fprintf(stderr, "tmsio v%d.%d%d stayed.\n",
	      (v & 0xFF00) >> 8,
	      (v & 0x00F0) >> 4,
	      v & 0x000F
	      );
    }
  } else
    rval = NOTSTAYED;
  B_SUPER(ssp);
  return rval;
}

#ifdef DEBUG
int
main()
{
  siochk();
  return EXIT_SUCCESS;
}
#endif

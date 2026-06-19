#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define QVPLAY_VER_MAJOR 0
#define QVPLAY_VER_MINOR 95
#define QVPLAY_VERSION                                                         \
  STRINGIFY(QVPLAY_VER_MAJOR) "." STRINGIFY(QVPLAY_VER_MINOR)

#define QVPLAY_COPYRIGHT (c)1996 - 2000 ken - ichi HAYASHI
#define QVPLAY_AUTHORS "ken-ichi HAYASHI and Jun-ichiro \"itojun\" ITOH."

#define QVPLAY_USAGE_HEADER                                                    \
  "qvplay (Ver " QVPLAY_VERSION ") (c)1996-2000 ken-ichi HAYASHI, itojun\n"
#define QVALLDEL_USAGE_HEADER                                                  \
  "qvalldel (Ver " QVPLAY_VERSION ") (c)1996-2000 ken-ichi HAYASHI\n"

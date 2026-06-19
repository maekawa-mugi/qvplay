#include "getopt.h"

#include <stdio.h>
#include <string.h>

char *optarg;
int optind = 1;
int opterr = 1;
int optopt;

int getopt(int argc, char *const argv[], const char *options) {
  static const char *next;
  const char *match;
  char option;

  optarg = NULL;
  if (next == NULL || *next == '\0') {
    if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
      return -1;
    if (strcmp(argv[optind], "--") == 0) {
      optind++;
      return -1;
    }
    next = argv[optind++] + 1;
  }

  option = *next++;
  match = strchr(options, option);
  if (match == NULL || option == ':') {
    optopt = option;
    if (opterr)
      fprintf(stderr, "unknown option -- %c\n", option);
    return '?';
  }
  if (match[1] != ':')
    return option;

  if (*next != '\0') {
    optarg = (char *)next;
    next = NULL;
  } else if (optind < argc) {
    optarg = argv[optind++];
  } else {
    optopt = option;
    next = NULL;
    if (opterr)
      fprintf(stderr, "option requires an argument -- %c\n", option);
    return options[0] == ':' ? ':' : '?';
  }
  return option;
}

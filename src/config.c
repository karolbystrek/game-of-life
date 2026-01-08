#include "config.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

AppConfig parse_args(int argc, char *argv[]) {
  AppConfig config = {
    .filename = NULL,
    .random_mode = false,
    .width = 60,
    .height = 20
  };

  int opt;
  while ((opt = getopt(argc, argv, "f:rw:h:")) != -1) {
    switch (opt) {
    case 'f':
      config.filename = optarg;
      break;
    case 'r':
      config.random_mode = true;
      break;
    case 'w':
      config.width = atoi(optarg);
      break;
    case 'h':
      config.height = atoi(optarg);
      break;
    default:
      fprintf(stderr, "Usage: %s [-f <file>] [-r] [-w <width>] [-h <height>]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (!config.filename && (config.width <= 0 || config.height <= 0)) {
    fprintf(stderr, "Invalid dimensions\n");
    exit(EXIT_FAILURE);
  }

  return config;
}

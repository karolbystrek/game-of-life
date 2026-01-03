#include "config.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static void print_usage(const char *prog_name) {
  printf("Usage: %s [-f <file>] [-r] [-w <width>] [-h <height>]\n", prog_name);
  printf("Options:\n");
  printf("  -f <file>    Load game from a file\n");
  printf("  -r           Initialize with a random pattern\n");
  printf("  -w <width>   Set board width (default: 60)\n");
  printf("  -h <height>  Set board height (default: 20)\n");
}

AppConfig parse_args(int argc, char *argv[]) {
  AppConfig config = {
      .filename = NULL, .random_mode = false, .width = 60, .height = 20};

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
      print_usage(argv[0]);
      exit(1);
    }
  }

  if (!config.filename && (config.width <= 0 || config.height <= 0)) {
    printf("Error: Invalid dimensions %dx%d\n", config.width, config.height);
    exit(1);
  }

  return config;
}

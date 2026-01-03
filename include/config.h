#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
  char *filename;
  bool random_mode;
  int width;
  int height;
} AppConfig;

AppConfig parse_args(int argc, char *argv[]);

#endif

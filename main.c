#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "animsplash.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: %s <infile>\n", argv[0]);
    return 1;
  }

  return gif_convert(argv[1], "asplash.bin");
}

#include "file_reader.h"

char *bf_read_file(const char *filename) {
  char *program = NULL;
  int fd;
  struct stat pstats;

  if ((fd = open(filename, O_RDONLY)) == -1) {
    goto exit;
  }
  if (fstat(fd, &pstats) == -1) {
    goto cleanup;
  }

  if ((program = malloc(pstats.st_size)) == NULL) {
    goto cleanup;
  }

  read(fd, program, pstats.st_size);

cleanup:
  close(fd);
exit:
  return program;
}

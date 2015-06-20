#ifndef BF_FILE_READER_H
#define BF_FILE_READER_H

#include<fcntl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<unistd.h>

/**
 * Function: bf_read_file
 * Reads a BF source file into memory.
 *
 * A character buffer will be allocated to hold the contents of the
 * file. The caller of this function will be responsible for making
 * sure that free() is called on the buffer.
 *
 * Param: {char *} filename The name of the file to read in.
 * Returns: {char *} A pointer to a character buffer with the contents
 *    of the file. The caller must ensure it is cleaned up. If the
 *    operation fails for any reason, NULL will be returned.
 */
char *bf_read_file(const char *filename);

#endif

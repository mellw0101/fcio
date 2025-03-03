/** @file files.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* Return's `TRUE` when a file exists and we have permissions to it. */
bool file_exists(const char *const restrict path) {
  ASSERT(path);
  struct stat st;
  if (access(path, R_OK) != 0) {
    return FALSE;
  }
  return (stat(path, &st) != -1 && !(S_ISDIR(st.st_mode) || S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)));
}

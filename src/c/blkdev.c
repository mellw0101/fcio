/** @file blkdev.c

  @author  Melwin Svensson.
  @date    27-4-2025.

 */
#include "../include/proto.h" 


/* Return's 'TRUE' if the block device at path exists and is a block device. */
bool blkdev_exists(const char *const restrict path) {
  ASSERT(path);
  struct stat st;
  if (access(path, F_OK) != 0) {
    return FALSE;
  }
  return (stat(path, &st) != -1 && S_ISBLK(st.st_mode));
}

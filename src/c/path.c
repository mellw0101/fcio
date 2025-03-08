/** @file path.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* Return a ptr to the last part of a path, if there are no '/' in the path, just return `path`. */
const char *tail(const char *const restrict path) {
  ASSERT(path);
  const char *slash = strrchr(path, '/');
  if (!slash) {
    return path;
  }
  return (slash + 1);
}

/* Return the extention of `path`, if any.  Otherwise, return NULL. */
const char *ext(const char *const restrict path) {
  ASSERT(path);
  const char *pathtail = tail(path);
  /* If the tail of the path starts with '.', then this is not a extention. */
  if (*pathtail == '.') {
    return NULL;
  }
  return (strrchr(pathtail, '.'));
}

/* Concatate a path, taking into account trailing and leading '/' for a proper path. */
char *concatpath(const char *const restrict s1, const char *const restrict s2) {
  Ulong s1len = strlen(s1);
  /* If either s1 end with '/' or s2 starts with '/'. */
  if ((s1[s1len - 1] == '/' && *s2 != '/') || (s1[s1len - 1] != '/' && *s2 == '/')) {
    return fmtstr("%s%s", s1, s2);
  }
  /* When both s1 and s2 starts with '/'. */
  else if (s1[s1len - 1] == '/' && *s2 == '/') {
    return fmtstr("%s%s", s1, (s2 + 1));
  }
  /* And when niether s1 end with '/' or s2 starts with '/'. */
  else {
    return fmtstr("%s/%s", s1, s2);
  }
}

/* Allocate a given stat struct if not already allocated. */
void statalloc(const char *const restrict path, struct stat **ptr) {
  ASSERT(path);
  ASSERT(ptr);
  /* If path does not exist, or is something other the a file, free it if its valid and return. */
  if (!file_exists(path)) {
    *ptr ? (free(*ptr), *ptr = NULL) : 0;
    return;
  }
  !*ptr ? (*ptr = xmalloc(sizeof(**ptr))) : 0;
  ALWAYS_ASSERT(stat(path, *ptr) != -1);
}

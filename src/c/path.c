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

#if __WIN__

static bool wcdirdelim(wchar_t wc) {
  return (wc == L'\\' || wc == L'/');
}

/* Concatate a path, taking into account trailing and leading '/' for a proper path. */
wchar_t *wconcatpath(const wchar_t *const restrict s1, const wchar_t *const restrict s2) {
  ASSERT(s1);
  ASSERT(s2);
  size_t s1len = lstrlenW(s1);
  /* If either s1 end with '/' or s2 starts with '/'. */
  if ((wcdirdelim(s1[s1len-1]) && !wcdirdelim(*s2)) || (!wcdirdelim(s1[s1len-1]) && wcdirdelim(*s2))) {
    return wfmtstr(L"%ls%ls", s1, s2);
  }
  /* When both s1 and s2 starts with '/'. */
  else if (wcdirdelim(s1[s1len - 1]) && wcdirdelim(*s2)) {
    return wfmtstr(L"%ls%ls", s1, (s2 + 1));
  }
  /* And when niether s1 end with '/' or s2 starts with '/'. */
  else {
    return wfmtstr(L"%ls\\%ls", s1, s2);
  }
}

#endif/* __WIN__ */

#if !__WIN__

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

/* ----------------------------- Getpwd ----------------------------- */

/* Return's an allocated string containg the current working directory based of the env var `PWD`. */
char *getpwd(void) {
  const char *pwd = getenv("PWD");
  if (!pwd) {
    return COPY_OF("");
  }
  return copy_of(pwd);
}

/* Return's an allocated string containg the current working directory based of the env
 * var `PWD`.  Note that this also assigns the length of the allocated string to `*len`. */
char *getpwd_len(Ulong *const len) {
  ASSERT_MSG(len, "'len' cannot be NULL, if this is intended, call 'getpwd()'");
  char *ret = getpwd();
  *len = STRLEN(ret);
  return ret;
}

#endif/* !__WIN__ */

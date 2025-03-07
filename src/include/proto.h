/** @file proto.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


#include "def.h"


_BEGIN_C_LINKAGE


/* ---------------------------------------------------------- fcio.c ---------------------------------------------------------- */


/* The function that will be called to terminate the executable when a fatal error happens,
 * use `fcio_set_die_callback()` to set this.  Note that by default this prints the error to stderr and calls `exit(1)`. */
extern _NO_RETURN _PRINTFLIKE(1, 2) void (*die_callback)(const char *format, ...);

void fcio_set_die_callback(void (*callback)(const char *format, ...));

void stdoutwrite(const char *const restrict data, Ulong len) _NONNULL(1);
void writef(const char *const restrict format, ...) _NONNULL(1) _PRINTFLIKE(1, 2);
void vwritef(const char *const restrict format, va_list ap) _NONNULL(1);
bool ynanswer(const char *const restrict format, ...);


/* ---------------------------------------------------------- files.c ---------------------------------------------------------- */


bool file_exists(const char *const restrict path) _NONNULL(1);


/* ---------------------------------------------------------- dirs.c ---------------------------------------------------------- */


bool dir_exists(const char *const restrict path);
directory_entry_t *directory_entry_make(void);
directory_entry_t *directory_entry_extract(directory_t *const dir, Ulong idx);
bool directory_entry_is_file(directory_entry_t *const entry);
bool directory_entry_is_non_exec_file(directory_entry_t *const entry);
void directory_entry_free(directory_entry_t *const entry);
void directory_data_init(directory_t *const dir);
void directory_data_free(directory_t *const dir);
int directory_get(const char *const restrict path, directory_t *const output);
int directory_get_recurse(const char *const restrict path, directory_t *const output);


/* ---------------------------------------------------------- mem.c ---------------------------------------------------------- */


void *xmalloc(Ulong howmush) __THROW _RETURNS_NONNULL;
void *xrealloc(void *ptr, Ulong newsize) __THROW _RETURNS_NONNULL _NONNULL(1);
void *xcalloc(Ulong elems, Ulong elemsize) __THROW _RETURNS_NONNULL;


/* ---------------------------------------------------------- str.c ---------------------------------------------------------- */


char *measured_copy(const char *const restrict string, Ulong len) __THROW _RETURNS_NONNULL _NONNULL(1);
char *copy_of(const char *const restrict string) __THROW _RETURNS_NONNULL _NONNULL(1);
char *fmtstr(const char *const restrict format, ...) __THROW _RETURNS_NONNULL _NONNULL(1) _PRINTFLIKE(1, 2);
char *valstr(const char *const restrict format, va_list ap, int *const outlen) __THROW _RETURNS_NONNULL _NONNULL(1);
char **split_string(const char *const restrict string, const char delim);
long strtonum(const char *const restrict string);

/* ----------------------------- xstrcat ----------------------------- */

char *xnstrncat(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen);
char *xnstrcat(char *restrict dst, Ulong dstlen, const char *const restrict src);
char *xstrncat(char *restrict dst, const char *const restrict src, Ulong srclen);
char *xstrcat(char *restrict dst, const char *const restrict src);


/* ---------------------------------------------------------- path.c ---------------------------------------------------------- */


const char *tail(const char *const restrict path);
const char *ext(const char *const restrict path);
char *concatpath(const char *const restrict s1, const char *const restrict s2);
void statalloc(const char *const restrict path, struct stat **ptr);


/* ---------------------------------------------------------- cvec.c ---------------------------------------------------------- */


CVec *cvec_create(void);
void cvec_free(CVec *const v);
void cvec_setfree(CVec *const v, FreeFuncPtr free);
void cvec_push(CVec *const v, void *const item);
void cvec_trim(CVec *const v);
void *cvec_get(CVec *const v, int index);
int cvec_len(CVec *const v);
int cvec_cap(CVec *const v);


/* ---------------------------------------------------------- hashmap.c ---------------------------------------------------------- */


HashMap *hashmap_create(void);
void hashmap_free(HashMap *const map);
void hashmap_set_free_value_callback(HashMap *const map, FreeFuncPtr callback);
void hashmap_insert(HashMap *map, const char *key, void *value);
void *hashmap_get(HashMap *map, const char *key);
void hashmap_remove(HashMap *map, const char *key);
int hashmap_size(HashMap *const map);
int hashmap_cap(HashMap *const map);
void hashmap_forall(HashMap *const map, void (*action)(const char *const restrict key, void *value));
void hashmap_clear(HashMap *const map);
void hashmap_thread_test(void);


/* ---------------------------------------------------------- fd.c ---------------------------------------------------------- */


void fdlock(int fd, short type);
void fdunlock(int fd);
void disable_canonecho(int fd, struct termios *const oldt);
void restore_termios(int fd, struct termios *const t);
void setfdflags(int fd, int *oldf, int flags);
void restfdflags(int fd, int *f);


/* ---------------------------------------------------------- term.c ---------------------------------------------------------- */


void clrtoeos(void);
void clrtobos(void);
void clrscreen(void);
void movecurs(int row, int col);
void mvcurshome(void);
void savecurs(void);
void restcurs(void);
void mvcursupbeg(int nlines);
void mvcursdownbeg(int nlines);
void mvcursup(int nlines);
void mvcursdown(int nlines);
void mvcursright(int ncols);
void mvcursleft(int ncols);


/* ---------------------------------------------------------- chars.c ---------------------------------------------------------- */


bool isconeof(const char c, const char *const restrict string);


_END_C_LINKAGE

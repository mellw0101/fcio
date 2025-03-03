/** @file proto.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


#include "def.h"


_BEGIN_C_LINKAGE


/* ---------------------------------------------------------- fcio.c ---------------------------------------------------------- */


/* The function that will be called to terminate the executable when a fatal error happens,
 * use `fcio_set_die_callback()` to set this.  Note that by default this prints the error to stdout and calls `exit(1)`. */
extern _NO_RETURN _PRINTFLIKE(1, 2) void (*die_callback)(const char *format, ...);

void fcio_set_die_callback(void (*callback)(const char *format, ...));


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
void *xrealloc(void *ptr, Ulong newsize) __THROW _RETURNS_NONNULL;


/* ---------------------------------------------------------- str.c ---------------------------------------------------------- */


char *measured_copy(const char *const restrict string, Ulong len) __THROW _RETURNS_NONNULL _NONNULL(1);
char *copy_of(const char *const restrict string) __THROW _RETURNS_NONNULL _NONNULL(1);
char *fmtstr(const char *const restrict format, ...) __THROW _RETURNS_NONNULL _NONNULL(1);


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


_END_C_LINKAGE

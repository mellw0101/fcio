/** @file proto.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


#include "config.h"
#include "def.h"


_BEGIN_C_LINKAGE


/* ---------------------------------------------------------- fcio.c ---------------------------------------------------------- */


/* The function that will be called to terminate the executable when a fatal error happens,
 * use `fcio_set_die_callback()` to set this.  Note that by default this prints the error to stderr and calls `exit(1)`. */
extern _NO_RETURN _PRINTFLIKE(1, 2) void (*die_callback)(const char *format, ...);

void fcio_set_die_callback(void (*callback)(const char *format, ...));

void stdoutwrite(const char *const restrict data, Ulong len) _NONNULL(1);
void stderrwrite(const char *const restrict data, Ulong len) _NONNULL(1);
void writef(const char *const restrict format, ...) _NONNULL(1) _PRINTFLIKE(1, 2);
void writeferr(const char *const restrict format, ...) _NONNULL(1) _PRINTFLIKE(1, 2);
void vwritef(const char *const restrict format, va_list ap) _NONNULL(1);
void vwriteferr(const char *const restrict format, va_list ap) _NONNULL(1);
bool ynanswer(const char *const restrict format, ...);


/* ---------------------------------------------------------- files.c ---------------------------------------------------------- */


bool file_exists(const char *const restrict path) __THROW _NODISCARD _NONNULL(1);
bool non_exec_file_exists(const char *const restrict path) __THROW _NODISCARD _NONNULL(1);


/* ---------------------------------------------------------- dirs.c ---------------------------------------------------------- */


bool               dir_exists(const char *const restrict path);
directory_entry_t *directory_entry_make(void);
directory_entry_t *directory_entry_extract(directory_t *const dir, Ulong idx);
bool               directory_entry_is_file(directory_entry_t *const entry);
bool               directory_entry_is_non_exec_file(directory_entry_t *const entry);
void               directory_entry_free(directory_entry_t *const entry);
void               directory_data_init(directory_t *const dir);
void               directory_data_free(directory_t *const dir);
int                directory_get(const char *const restrict path, directory_t *const output);
int                directory_get_recurse(const char *const restrict path, directory_t *const output);

/* ----------------------------- Test's ----------------------------- */

void test_directory_t(const char *const dirpath);


/* ---------------------------------------------------------- mem.c ---------------------------------------------------------- */


void *xmalloc(Ulong howmush) __THROW _NODISCARD _RETURNS_NONNULL;
void *xrealloc(void *ptr, Ulong newsize) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
void *xcalloc(Ulong elems, Ulong elemsize) __THROW _NODISCARD _RETURNS_NONNULL;


/* ---------------------------------------------------------- str.c ---------------------------------------------------------- */


char *measured_copy(const char *const restrict string, Ulong len) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
char *copy_of(const char *const restrict string) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
char *valstr(const char *const restrict format, va_list ap, int *const outlen) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
long  strtonum(const char *const restrict string) __THROW _NODISCARD _NONNULL(1);
bool  parse_num(const char *const restrict string, long *const result) __THROW _NODISCARD _NONNULL(1, 2);

/* ----------------------------- split_string ----------------------------- */

char **split_string_len(const char *const restrict string, const char delim, Ulong *const argc) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
char **split_string(const char *const restrict string, const char delim) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);

/* ----------------------------- chararray ----------------------------- */

void chararray_free(char **const restrict array, Ulong len) __THROW _NONNULL(1);
void chararray_freenullterm(char **const restrict array) __THROW _NONNULL(1);
void chararray_append(char ***array, Ulong *const len, char **append, Ulong append_len) __THROW _NONNULL(1, 2, 3);
void chararray_erase(char **const array, Ulong *len, Ulong idx) __THROW _NONNULL(1, 2);

/* ----------------------------- fmtstr ----------------------------- */

char *fmtstr(const char *const restrict format, ...) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1) _PRINTFLIKE(1, 2);
char *fmtstr_len(int *const fmtlen, const char *const restrict format, ...) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2) _PRINTFLIKE(2, 3);

/* ----------------------------- fmtstrcat ----------------------------- */

char *fmtstrncat(char *restrict dst, Ulong dstlen, const char *const restrict format, ...) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *fmtstrcat(char *restrict dst, const char *const restrict format, ...) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);

/* ----------------------------- xstrcat ----------------------------- */

char *xnstrncat(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xnstrcat(char *restrict dst, Ulong dstlen, const char *const restrict src) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xstrncat(char *restrict dst, const char *const restrict src, Ulong srclen) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);
char *xstrcat(char *restrict dst, const char *const restrict src) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);

/* ----------------------------- xstrinj ----------------------------- */

char *xnstrninj(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xnstrinj(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xstrninj(char *restrict dst, const char *const restrict src, Ulong srclen, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);
char *xstrinj(char *restrict dst, const char *const restrict src, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);


/* ---------------------------------------------------------- path.c ---------------------------------------------------------- */


const char *tail(const char *const restrict path) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
const char *ext(const char *const restrict path) __THROW _NODISCARD _NONNULL(1);
char       *concatpath(const char *const restrict s1, const char *const restrict s2) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);
void        statalloc(const char *const restrict path, struct stat **ptr) __THROW _NONNULL(1, 2);


/* ---------------------------------------------------------- cvec.c ---------------------------------------------------------- */


CVec *cvec_create(void) __THROW _NODISCARD _RETURNS_NONNULL;
CVec *cvec_create_setfree(FreeFuncPtr free) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
void  cvec_free(CVec *const v);
void  cvec_setfree(CVec *const v, FreeFuncPtr free);
void  cvec_push(CVec *const v, void *const item);
void  cvec_trim(CVec *const v);
void *cvec_get(CVec *const v, int index);
int   cvec_len(CVec *const v);
int   cvec_cap(CVec *const v);
void  cvec_clear(CVec *const v);
void  cvec_qsort(CVec *const v, CmpFuncPtr cmp);


/* ---------------------------------------------------------- hashmap.c ---------------------------------------------------------- */


/* ----------------------------- HashMap ----------------------------- */

HashMap *hashmap_create(void) __THROW _NODISCARD _RETURNS_NONNULL;
void     hashmap_free(HashMap *const map) __THROW _NONNULL(1);
HashMap *hashmap_create_wfreefunc(FreeFuncPtr freefunc) __THROW _NODISCARD _NONNULL(1);
void     hashmap_set_free_value_callback(HashMap *const map, FreeFuncPtr callback) __THROW _NONNULL(1);
void     hashmap_insert(HashMap *const map, const char *const restrict key, void *value) __THROW _NONNULL(1, 2, 3);
void    *hashmap_get(HashMap *const map, const char *key) __THROW _NONNULL(1, 2);
void     hashmap_remove(HashMap *const map, const char *key);
int      hashmap_size(HashMap *const map);
int      hashmap_cap(HashMap *const map);
void     hashmap_forall(HashMap *const map, void (*action)(const char *const restrict key, void *value));
void     hashmap_clear(HashMap *const map);
void     hashmap_append(HashMap *const dst, HashMap *const src);
void     hashmap_append_waction(HashMap *const dst, HashMap *const src, void (*existing_action)(void *dstnodevalue, void *srcnodevalue));

/* ----------------------------- HashMapNum ----------------------------- */

HashMapNum *hashmapnum_create(void) __THROW _NODISCARD _RETURNS_NONNULL;
void        hashmapnum_free(HashMapNum *const map) __THROW _NONNULL(1);
HashMapNum *hashmapnum_create_wfreefunc(FreeFuncPtr freefunc) __THROW _NODISCARD _NONNULL(1);
void        hashmapnum_set_free_value_callback(HashMapNum *const map, FreeFuncPtr callback) __THROW _NONNULL(1);
void        hashmapnum_insert(HashMapNum *const map, Ulong key, void *value) __THROW _NONNULL(1, 3);
void       *hashmapnum_get(HashMapNum *const map, Ulong key) __THROW _NONNULL(1);
void        hashmapnum_remove(HashMapNum *const map, Ulong key);
int         hashmapnum_size(HashMapNum *const map);
int         hashmapnum_cap(HashMapNum *const map);
void        hashmapnum_forall(HashMapNum *const map, void (*action)(Ulong key, void *value));
void        hashmapnum_clear(HashMapNum *const map);
void        hashmapnum_append(HashMapNum *const dst, HashMapNum *const src);
void        hashmapnum_append_waction(HashMapNum *const dst, HashMapNum *const src, void (*existing_action)(void *dstnodevalue, void *srcnodevalue));

/* ----------------------------- Tests ----------------------------- */

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


void initcheck_utf8(void);
int charlen(const char *const restrict ptr) __THROW _NODISCARD _CONST _NONNULL(1);
bool isconeof(const char c, const char *const restrict string) __THROW _NODISCARD _CONST _NONNULL(2);


/* ---------------------------------------------------------- future.c ---------------------------------------------------------- */


void future_free(Future *future);
void *future_get(Future *future);
bool future_try_get(Future *future, void **result);
Future *future_submit(void *(*task)(void *), void *arg);



/* ---------------------------------------------------------- rand.c ---------------------------------------------------------- */


char *randstr(Ulong length);


/* ---------------------------------------------------------- math.c ---------------------------------------------------------- */


float fclamp(float x, float min, float max) __THROW _CONST _NODISCARD;
long  lclamp(long x, long min, long max) __THROW _CONST _NODISCARD;
float absf(float x) __THROW _CONST _NODISCARD;



/* ---------------------------------------------------------- atomic_bool_sync.c ---------------------------------------------------------- */

int atomic_bool_sync_get(atomic_bool_sync *b);
void atomic_bool_sync_set_true(atomic_bool_sync *b);
void atomic_bool_sync_set_false(atomic_bool_sync *b);



/* ---------------------------------------------------------- atomicbool.c ---------------------------------------------------------- */


atomicbool *atomicbool_create(void);
void atomicbool_free(atomicbool *ab);
bool atomicbool_get(atomicbool *ab);
void atomicbool_set_true(atomicbool *ab);
void atomicbool_set_false(atomicbool *ab);


/* ---------------------------------------------------------- queue.c ---------------------------------------------------------- */


Queue *queue_create(void);
void queue_free(Queue *q);
void queue_set_free_func(Queue *q, FreeFuncPtr free_func);
void queue_enqueue(Queue *q, void *data);
void *queue_pop(Queue *q);
void *queue_peak(Queue *q);
Ulong queue_size(Queue *q);


_END_C_LINKAGE

/** @file proto.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


#include "config.h"
#include "def.h"

#if __WIN__
# define __THROW
# define __always_inline
#ifdef _NODISCARD
# undef _NODISCARD
#endif
# define _NODISCARD
#endif


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

#if !__WIN__
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
#endif

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
char *free_and_assign(char *dest, char *src) __THROW _NODISCARD;

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

char *fmtstrncat(char *restrict dst, Ulong dstlen, const char *const restrict format, ...) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3) _PRINTFLIKE(3, 4);
char *fmtstrcat(char *restrict dst, const char *const restrict format, ...) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2) _PRINTFLIKE(2, 3);

/* ----------------------------- fmtstrcpy ----------------------------- */

char *fmtstrcpy(char *restrict dst, const char *const restrict format, ...) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2) _PRINTFLIKE(2, 3);

/* ----------------------------- xstrcat_norealloc ----------------------------- */

char *xnstrncat_norealloc(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen) __THROW _RETURNS_NONNULL _NONNULL(1, 3);
char *xnstrcat_norealloc(char *restrict dst, Ulong dstlen, const char *const restrict src) __THROW _RETURNS_NONNULL _NONNULL(1, 3);
char *xstrncat_norealloc(char *restrict dst, const char *const restrict src, Ulong srclen) __THROW _RETURNS_NONNULL _NONNULL(1, 2);
char *xstrcat_norealloc(char *restrict dst, const char *const restrict src) __THROW _RETURNS_NONNULL _NONNULL(1, 2);

/* ----------------------------- xstrcat ----------------------------- */

char *xnstrncat(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xnstrcat(char *restrict dst, Ulong dstlen, const char *const restrict src) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xstrncat(char *restrict dst, const char *const restrict src, Ulong srclen) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);
char *xstrcat(char *restrict dst, const char *const restrict src) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);

/* ----------------------------- xstrinj_norealloc ----------------------------- */

char *xnstrninj_norealloc(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen, Ulong idx) __THROW _RETURNS_NONNULL _NONNULL(1, 3);
char *xnstrinj_norealloc(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong idx) __THROW _RETURNS_NONNULL _NONNULL(1, 3);
char *xstrninj_norealloc(char *restrict dst, const char *const restrict src, Ulong srclen, Ulong idx) __THROW _RETURNS_NONNULL _NONNULL(1, 2);
char *xstrinj_norealloc(char *restrict dst, const char *const restrict src, Ulong idx) __THROW _NODISCARD _NONNULL(1, 2);

/* ----------------------------- xstrinj ----------------------------- */

char *xnstrninj(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong srclen, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xnstrinj(char *restrict dst, Ulong dstlen, const char *const restrict src, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 3);
char *xstrninj(char *restrict dst, const char *const restrict src, Ulong srclen, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);
char *xstrinj(char *restrict dst, const char *const restrict src, Ulong idx) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);

/* ----------------------------- xstr_erase_norealloc ----------------------------- */

char *xstrn_erase_norealloc(char *restrict dst, Ulong dstlen, Ulong index, Ulong len) __THROW _RETURNS_NONNULL _NONNULL(1);
char *xstr_erase_norealloc(char *restrict dst, Ulong index, Ulong len) __THROW _RETURNS_NONNULL _NONNULL(1);

/* ----------------------------- xstr_erase ----------------------------- */

char *xstrn_erase(char *restrict dst, Ulong dstlen, Ulong index, Ulong len) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
char *xstr_erase(char *restrict dst, Ulong index, Ulong len) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);

/* ----------------------------- xstrcpy ----------------------------- */

char *xstrncpy(char *restrict dst, const char *const restrict src, Ulong n) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);
char *xstrcpy(char *restrict dst, const char *const restrict src) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);


#if __WIN__
wchar_t *wfree_and_assign(wchar_t *dest, wchar_t *src);

wchar_t *measured_wcopy(const wchar_t *string, size_t len);
wchar_t *wcopy_of(const wchar_t *string);
wchar_t *wvalstr(const wchar_t *const restrict format, va_list ap, int *const outlen);
wchar_t *wfmtstr(const wchar_t *const restrict format, ...);
wchar_t *wfmtstrcat(wchar_t *restrict dst, const wchar_t *const restrict format, ...);

wchar_t *xwnstrncat(wchar_t *restrict dst, size_t dstlen, const wchar_t *const restrict src, size_t srclen);
wchar_t *xwnstrcat(wchar_t *restrict dst, size_t dstlen, const wchar_t *const restrict src);
wchar_t *xwstrncat(wchar_t *restrict dst, const wchar_t *const restrict src, size_t srclen);
wchar_t *xwstrcat(wchar_t *restrict dst, const wchar_t *const restrict src);
#endif


/* ---------------------------------------------------------- path.c ---------------------------------------------------------- */


const char *tail(const char *const restrict path) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
const char *ext(const char *const restrict path) __THROW _NODISCARD _NONNULL(1);
char       *concatpath(const char *const restrict s1, const char *const restrict s2) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1, 2);

#if __WIN__
wchar_t *wconcatpath(const wchar_t *const restrict s1, const wchar_t *const restrict s2);
#endif

#if !__WIN__
void        statalloc(const char *const restrict path, struct stat **ptr) __THROW _NONNULL(1, 2);
/* ----------------------------- Getpwd ----------------------------- */
char *getpwd(void) _NODISCARD _RETURNS_NONNULL;
char *getpwd_len(Ulong *const len) _NODISCARD _RETURNS_NONNULL _NONNULL(1);
#endif


/* ---------------------------------------------------------- cvec.c ---------------------------------------------------------- */


CVEC new_cvec_create(void);
void new_cvec_free(CVEC cv);
void new_cvec_set_free_func(CVEC cv, void (*free_func)(void *));
size_t new_cvec_size(CVEC cv);
void new_cvec_push_back(CVEC cv, void *p);
void *new_cvec_get(CVEC cv, size_t idx);
void new_cvec_erase_swap_back(CVEC cv, size_t idx);
void new_cvec_erase_shift(CVEC cv, size_t idx);
void new_cvec_clear(CVEC cv);

CVec *cvec_create(void) __THROW _NODISCARD _RETURNS_NONNULL;
CVec *cvec_create_setfree(FreeFuncPtr free) __THROW _NODISCARD _RETURNS_NONNULL _NONNULL(1);
void  cvec_free(CVec *const v);
void  cvec_free_void_ptr(void *arg);
void  cvec_setfree(CVec *const v, FreeFuncPtr free);
void  cvec_push(CVec *const v, void *const item);
void  cvec_trim(CVec *const v);
void *cvec_get(CVec *const v, int index);
void  cvec_remove(CVec *const v, int index);
void  cvec_remove_by_value(CVec *const v, void *const value);
int   cvec_len(CVec *const v);
int   cvec_cap(CVec *const v);
void  cvec_clear(CVec *const v);
void  cvec_qsort(CVec *const v, CmpFuncPtr cmp);


/* ---------------------------------------------------------- hashmap.c ---------------------------------------------------------- */


/* ----------------------------- HMAP ----------------------------- */

/*
 * Create a string hashmap, where the key is a `char *`.
 */
HMAP hmap_create(void);
void  hmap_free(HMAP m);
void  hmap_set_free_func(HMAP m, void (*free_func)(void *));
void  hmap_insert(HMAP m, const char *const restrict key, void *value);
void *hmap_get(HMAP m, const char *const restrict key);
bool  hmap_contains(HMAP m, const char *const restrict key);
void  hmap_remove(HMAP m, const char *const restrict key);
void  hmap_clear(HMAP m);
void  hmap_forall_wdata(HMAP m, void (*action)(const char *key, void *value, void *data), void *data);

/* ----------------------------- HNMAP ----------------------------- */

HNMAP hnmap_create(void);
void  hnmap_free(HNMAP nm);
void  hnmap_set_free_func(HNMAP nm, void (*free_func)(void *));
void  hnmap_insert(HNMAP nm, HMAP_UINT key, void *value);
void *hnmap_get(HNMAP nm, HMAP_UINT key);
bool  hnmap_contains(HNMAP nm, HMAP_UINT key);
void  hnmap_remove(HNMAP nm, HMAP_UINT key);
void  hnmap_clear(HNMAP nm);
void  hnmap_forall_wdata(HNMAP nm, void (*action)(HMAP_UINT key, void *value, void *data), void *data);

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
void        hashmapnum_free_void_ptr(void *arg);
HashMapNum *hashmapnum_create_wfreefunc(FreeFuncPtr freefunc) __THROW _NODISCARD _NONNULL(1);
void        hashmapnum_set_free_value_callback(HashMapNum *const map, FreeFuncPtr callback) __THROW _NONNULL(1);
void        hashmapnum_insert(HashMapNum *const map, Ulong key, void *value) __THROW _NONNULL(1, 3);
void       *hashmapnum_get(HashMapNum *const map, Ulong key) __THROW _NONNULL(1);
void        hashmapnum_remove(HashMapNum *const map, Ulong key);
int         hashmapnum_size(HashMapNum *const map);
int         hashmapnum_cap(HashMapNum *const map);
void        hashmapnum_forall(HashMapNum *const map, void (*action)(Ulong key, void *value));
void        hashmapnum_forall_wdata(HashMapNum *const map, void (*action)(Ulong key, void *value, void *data), void *data);
void        hashmapnum_clear(HashMapNum *const map);
void        hashmapnum_append(HashMapNum *const dst, HashMapNum *const src);
void        hashmapnum_append_waction(HashMapNum *const dst, HashMapNum *const src, void (*existing_action)(void *dstnodevalue, void *srcnodevalue));

/* ----------------------------- Tests ----------------------------- */

void hashmap_thread_test(void);


/* ---------------------------------------------------------- fd.c ---------------------------------------------------------- */


#if !__WIN__
void fdlock(int fd, short type);
void fdunlock(int fd);
void disable_canonecho(int fd, struct termios *const oldt);
void restore_termios(int fd, struct termios *const t);
void setfdflags(int fd, int *oldf, int flags);
void restfdflags(int fd, int *f);
#endif


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
// int ctowc(wchar *const wc, const char *const c) __THROW _NODISCARD _CONST _NONNULL(1, 2);
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

int  atomic_bool_sync_get(atomic_bool_sync *const b);
void atomic_bool_sync_set_true(atomic_bool_sync *const b);
void atomic_bool_sync_set_false(atomic_bool_sync *const b);



/* ---------------------------------------------------------- atomicbool.c ---------------------------------------------------------- */


atomicbool *atomicbool_create(void);
void atomicbool_free(atomicbool *ab);
bool atomicbool_get(atomicbool *ab);
void atomicbool_set_true(atomicbool *ab);
void atomicbool_set_false(atomicbool *ab);


/* ---------------------------------------------------------- queue.c ---------------------------------------------------------- */


QUEUE queue_create(void);
void  queue_free(QUEUE q);
void  queue_set_free_func(QUEUE q, void (*free_func)(void *));
Ulong queue_size(QUEUE q);
void  queue_pop(QUEUE q);
void  queue_push(QUEUE q, void *data);
void *queue_front(QUEUE q);

// Queue *queue_create(void);
// void queue_free(Queue *q);
// void queue_set_free_func(Queue *q, FreeFuncPtr free_func);
// void queue_enqueue(Queue *q, void *data);
// void *queue_pop(Queue *q);
// void *queue_peak(Queue *q);
// Ulong queue_size(Queue *q);


/* ---------------------------------------------------------- blkdev.c ---------------------------------------------------------- */


bool blkdev_exists(const char *const restrict path);


/* ---------------------------------------------------------- utils.c ---------------------------------------------------------- */


int digits(long n);


/* ---------------------------------------------------------- hiactime.c ---------------------------------------------------------- */


#if !__WIN__
/* ----------------------------- Hiactime sleep total duration ----------------------------- */
void hiactime_sleep_total_duration(const struct timespec *const s, struct timespec *const e, Llong nanoseconds);
/* ----------------------------- Hiactime nsleep ----------------------------- */
void hiactime_nsleep(Llong nanoseconds);
/* ----------------------------- Hiactime nsleep ----------------------------- */
void hiactime_msleep(double milliseconds);
#endif


/* ---------------------------------------------------------- log.c ---------------------------------------------------------- */


/* ----------------------------- Fcio log set file ----------------------------- */
void fcio_log_set_file(const char *const restrict path) _NONNULL(1);
/* ----------------------------- Fcio log ----------------------------- */
void fcio_log(int type, Ulong lineno, const char *const restrict function, const char *const restrict format, ...) _PRINTFLIKE(4, 5);
/* ----------------------------- Fcio log error fatal ----------------------------- */
void fcio_log_error_fatal(Ulong lineno, const char *const restrict function, const char *const restrict format, ...) _NO_RETURN _PRINTFLIKE(3, 4);


_END_C_LINKAGE

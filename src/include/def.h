/** @file def.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


/* ---------------------------------------------------------- Includes ---------------------------------------------------------- */


/* ----------------------------- stdlib ----------------------------- */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

/* ----------------------------- linux ----------------------------- */

#include <sys/stat.h>

/* ----------------------------- fcio ----------------------------- */

#include "config.h"


/* ---------------------------------------------------------- Defines ---------------------------------------------------------- */


/* ----------------------------- General helper's ----------------------------- */

#ifdef DO_WHILE
# undef DO_WHILE
#endif
#ifdef CALL_IS_VALID
# undef CALL_IF_VALID
#endif

/* Perform actions protected from accidental missuse. */
#define DO_WHILE(...)                do {__VA_ARGS__} while (0)
#define CALL_IF_VALID(funcptr, ...)  DO_WHILE((funcptr) ? (funcptr)(__VA_ARGS__) : ((void)0);)

/* ----------------------------- String helper's ----------------------------- */

#ifdef STRLEN
# undef STRLEN
#endif
#ifdef S__LEN
# undef S__LEN
#endif

#define STRLEN(x)  (sizeof((x)) - 1)

/* Shorthand for when something calls for a string and the length of that string.  Note that this should only be used with literal strings. */
#define S__LEN(x)  (x), STRLEN(x)

/* Shorthand to create a allocated copy of a literal string instead of using copy_of, as by definition we will know the length for that literal. */
#define COPY_OF(x)  measured_copy(S__LEN(x))

/* ----------------------------- Boolian's ----------------------------- */

#ifdef FALSE
# undef FALSE
#endif
#ifdef TRUE
# undef TRUE
#endif

/* Define `TRUE` and `FALSE` correctly for c and c++.  By this i mean define them so that they nativly work. */
#ifdef __cplusplus
# define FALSE  false
# define TRUE   true
#else
# define FALSE  0
# define TRUE   1
#endif

/* ----------------------------- Int's ----------------------------- */

#ifdef Uchar
# undef Uchar
#endif
#ifdef Ushort
# undef Ushort
#endif
#ifdef Uint
# undef Uint
#endif
#ifdef Ulong
# undef Ulong
#endif
#ifdef UlongMIN
# undef UlongMIN
#endif
#ifdef UlongMAX
# undef UlongMAX
#endif

/* When `__WORDSIZE` is not defined, define it. */
#ifndef __WORDSIZE
# if (defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(__aarch64__) || defined(_M_ARM64))
#   define __WORDSIZE  64
# elif defined(_WIN32)
#   define __WORDSIZE  32
# endif
#endif

#define Uchar   unsigned char
#define Ushort  unsigned short int
#define Uint    unsigned int
#if (__WORDSIZE == 64)
# if ((defined(__x86_64__) && !defined(__ILP32__)) || defined(__aarch64__) || defined(_M_ARM64))
#   define Ulong     unsigned long int
#   define UlongMIN  (0UL)
#   define UlongMAX  (18446744073709551615UL)
# else
#   define Ulong     unsigned long long int
#   define UlongMIN  (0ULL)
#   define UlongMAX  (18446744073709551615ULL)
# endif
#else
# define Ulong     unsigned long long int
# define UlongMIN  (0ULL)
# define UlongMAX  (18446744073709551615ULL)
#endif

/* ----------------------------- Constant's ----------------------------- */

#ifdef PTR_SIZE
# undef PTR_SIZE
#endif
#ifdef _PTRSIZE
# undef _PTRSIZE
#endif

/* Size of a ptr in bits. */
#define PTR_SIZE  __WORDSIZE
/* Size of a ptr in bytes. */
#define _PTRSIZE  (sizeof(void *))

/* ----------------------------- Profiling ----------------------------- */

#ifdef US_TO_MS
# undef US_TO_MS
#endif
#ifdef TIMER_START
# undef TIMER_START
#endif
#ifdef TIMER_END
# undef TIMER_END
#endif
#ifdef TIMER_PRINT
# undef TIMER_PRINT
#endif

#define US_TO_MS(us)  ((us) / 1000.0f)

#define TIMER_START(name)                           \
  struct timespec name;                             \
  DO_WHILE(clock_gettime(CLOCK_MONOTONIC, &name);)  

#define TIMER_END(start, time_ms_name)                         \
  float time_ms_name;                                          \
  DO_WHILE(                                                    \
    struct timespec __timer_end;                               \
    clock_gettime(CLOCK_MONOTONIC, &__timer_end);              \
    time_ms_name =                                             \
      (((__timer_end.tv_sec - (start).tv_sec) * 1000000.0f) +  \
      ((__timer_end.tv_nsec - (start).tv_nsec) / 1000.0f));    \
    time_ms_name = US_TO_MS(time_ms_name);                     \
  )

#define TIMER_PRINT(ms)                                   \
  DO_WHILE(                                               \
    printf("%s: Time: %.5f ms\n", __func__, (double)ms);  \
  )

/* ----------------------------- Assert ----------------------------- */

#ifdef ENABLE_ASSERT
# undef ENABLE_ASSERT
#endif
#ifdef FCIO_ASSERT_VOID_CAST
# undef FCIO_ASSERT_VOID_CAST
#endif
#ifdef FCIO_ASSERT_DIE_CAST
# undef FCIO_ASSERT_DIE_CAST
#endif
#ifdef FCIO_ASSERT_DIE_MSG_CAST
# undef FCIO_ASSERT_DIE_MSG_CAST
#endif
#ifdef FCIO_DO_ASSERT
# undef FCIO_DO_ASSERT
#endif
#ifdef FCIO_DO_ASSERT_MSG
# undef FCIO_DO_ASSERT_MSG
#endif
#ifdef ASSERT
# undef ASSERT
#endif
#ifdef ASSERT_MSG
# undef ASSERT_MSG
#endif
#ifdef ALWAYS_ASSERT
# undef ALWAYS_ASSERT
#endif
#ifdef ALWAYS_ASSERT_MSG
# undef ALWAYS_ASSERT_MSG
#endif

#define ENABLE_ASSERT

/* Calls that are performed when an assertion fails. */
#define FCIO_ASSERT_VOID_CAST                ((void)0)
#define FCIO_ASSERT_DIE_CAST(expr)           die_callback("%s: LINE:[%d]: FILE:[%s]: Assertion failed: [%s]\n", __func__, __LINE__, __FILE__, #expr)
#define FCIO_ASSERT_DIE_MSG_CAST(expr, msg)  die_callback("%s: LINE:[%d]: FILE:[%s]: Assertion failed: [%s]: %s\n", __func__, __LINE__, __FILE__, #expr, (msg))

/* These macros evaluate the expr. */
#define FCIO_DO_ASSERT(expr)           DO_WHILE((expr) ? FCIO_ASSERT_VOID_CAST : FCIO_ASSERT_DIE_CAST(expr);)
#define FCIO_DO_ASSERT_MSG(expr, msg)  DO_WHILE((expr) ? FCIO_ASSERT_VOID_CAST : FCIO_ASSERT_DIE_MSG_CAST(expr, msg);)

/* Only make the assertion perform any action when enabled, otherwise they cast ((void)0), to ensure the same syntax as the conditional. */
#ifdef ENABLE_ASSERT
# define ASSERT(expr)           FCIO_DO_ASSERT(expr)
# define ASSERT_MSG(expr, msg)  FCIO_DO_ASSERT_MSG(expr, msg)
#else
# define ASSERT(expr)           FCIO_ASSERT_VOID_CAST
# define ASSERT_MSG(expr, msg)  FCIO_ASSERT_VOID_CAST
#endif

/* Macros that always perform the assertion, even when disabled. */
#define ALWAYS_ASSERT(expr)           FCIO_DO_ASSERT(expr)
#define ALWAYS_ASSERT_MSG(expr, msg)  FCIO_DO_ASSERT_MSG(expr, msg)

/* ----------------------------- Threads ----------------------------- */

#ifdef thread_t
# undef thread_t
#endif
#ifdef mutex_t
# undef mutex_t
#endif
#ifdef cond_t
# undef cond_t
#endif
#ifdef mutex_init
# undef mutex_init
#endif
#ifdef mutex_destroy
# undef mutex_destroy
#endif
#ifdef mutex_lock
# undef mutex_lock
#endif
#ifdef mutex_unlock
# undef mutex_unlock
#endif
#ifdef mutex_action
# undef mutex_action
#endif
#ifdef mutex_init_static
# undef mutex_init_static
#endif

/* Thread shorthand. */
#define thread_t  pthread_t

/* Mutex shorthand. */
#define mutex_t  pthread_mutex_t

/* Condition shorthand. */
#define cond_t  pthread_cond_t

/* Mutex helper shorthands. */
#define mutex_init                   pthread_mutex_init
#define mutex_destroy                pthread_mutex_destroy
#define mutex_lock                   pthread_mutex_lock
#define mutex_unlock                 pthread_mutex_unlock
#define mutex_action(mutex, action)  DO_WHILE(mutex_lock(mutex); DO_WHILE(action); mutex_unlock(mutex);)
#define mutex_init_static            PTHREAD_MUTEX_INITIALIZER

/* ----------------------------- Ptr array's ----------------------------- */

#ifdef ENSURE_PTR_ARRAY_SIZE
# undef ENSURE_PTR_ARRAY_SIZE
#endif
#ifdef TRIM_PTR_ARRAY
# undef TRIM_PTR_ARRAY
#endif
#ifdef ASSIGN_IF_VALID
# undef ASSIGN_IF_VALID
#endif
#ifdef ARRAY_SIZE
# undef ARRAY_SIZE
#endif

/* Useful when adding something to a ptr array and checking the size each time. */
#define ENSURE_PTR_ARRAY_SIZE(array, cap, size)         \
  DO_WHILE(                                             \
    if ((cap) == (size)) {                              \
      (cap) *= 2;                                       \
      (array) = xrealloc((array), (_PTRSIZE * (cap)));  \
    }                                                   \
  )

/* To save memory rellocate the array to one more then size to hold a null ptr. */
#define TRIM_PTR_ARRAY(array, cap, size)              \
  DO_WHILE(                                           \
    (cap) = ((size) + 1);                             \
    (array) = xrealloc((array), (_PTRSIZE * (cap)));  \
  )

/* Shorthand to assign the value to a ptr.  Useful when assigning ptr passed as parameters to functions that might be `NULL`. */
#define ASSIGN_IF_VALID(ptr, value)  DO_WHILE(((ptr) ? (*(ptr) = (value)) : ((int)0));)

/* Get the size of a stack based array. */
#define ARRAY_SIZE(array)  (sizeof((array)) / sizeof((array)[0]))

/* ----------------------------- Fd ----------------------------- */

#ifdef fdlock_action
# undef fdlock_action
#endif

/* Perform `action` while under the protection of a file-descriptor lock. */
#define fdlock_action(fd, type, action)  DO_WHILE(fdlock(fd, type); DO_WHILE(action); fdunlock(fd);)


/* ----------------------------- ASCII ----------------------------- */

#ifdef ASCII_ISDIGIT
# undef ASCII_ISDIGIT
#endif
#ifdef ASCII_TOUPPER
# undef ASCII_TOUPPER
#endif
#ifdef ASCII_TOLOWER
# undef ASCII_TOLOWER
#endif
#ifdef ASCII_ISUPPER
# undef ASCII_ISUPPER
#endif
#ifdef ASCII_ISLOWER
# undef ASCII_TOLOWER
#endif
#ifdef ASCII_ISALPHA
# undef ASCII_ISALPHA
#endif
#ifdef ASCII_ISALNUM
# undef ASCII_ISALNUM
#endif

#define ASCII_ISDIGIT(c)  ((c) >= '0' && (c) <= '9')
#define ASCII_TOUPPER(c)  (((c) < 'a' || (c) > 'z') ? (c) : ((c) - 'A'))
#define ASCII_TOLOWER(c)  (((c) < 'A' || (c) > 'Z') ? (c) : ((c) + 'a'))
#define ASCII_ISUPPER(c)  ((Uint)(c) >= 'A' && (Uint)(c) <= 'Z')
#define ASCII_ISLOWER(c)  ((Uint)(c) >= 'a' && (Uint)(c) <= 'z')
#define ASCII_ISALPHA(c)  (ASCII_ISUPPER(c) || ASCII_ISLOWER(c))
#define ASCII_ISALNUM(c)  (ASCII_ISALPHA(c) || ASCII_ISDIGIT(c))

/* ----------------------------- 'dirs.c' Define's ----------------------------- */

#define DIRECTORY_ITER(dir, entryname, action)                                        \
  DO_WHILE(                                                                           \
    for (Ulong __directory_iter=0; __directory_iter<(dir).len; ++__directory_iter) {  \
      directory_entry_t *entryname = (dir).entries[__directory_iter];                 \
      DO_WHILE(action);                                                               \
    }                                                                                 \
  )


/* ---------------------------------------------------------- Typedef's ---------------------------------------------------------- */


typedef void (*FreeFuncPtr)(void *);


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


/* ----------------------------- dirs.c ----------------------------- */

typedef struct {
  Uchar type;         /* The type of entry this is.  Uses `dirent->d_type`. */
  char *name;         /* Name of the entry. */
  char *path;         /* The full path of the entry. */
  char *ext;          /* The extention, if any. */
  char *clean_name;   /* When `name` has a extention, this is `name` without that extention, otherwise this is `NULL`. */
  struct stat *stat;  /* Stat data for the entry. */
} directory_entry_t;

typedef struct {
  directory_entry_t **entries;
  Ulong   cap;
  Ulong   len;
  mutex_t mutex;
} directory_t;

/* ----------------------------- cvec.c ----------------------------- */

typedef struct CVec  CVec;

/* ----------------------------- hashmap.c ----------------------------- */

typedef struct HashNode  HashNode;
typedef struct HashMap   HashMap;

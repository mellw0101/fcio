/** @file def.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


/* ---------------------------------------------------------- Includes ---------------------------------------------------------- */


/* stdlib. */
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

/* linux */
#include <sys/stat.h>

/* fcio */
#include "config.h"


/* ---------------------------------------------------------- Defines ---------------------------------------------------------- */


#ifndef DO_WHILE
# define DO_WHILE(...)  do {__VA_ARGS__} while (0)
#endif

#define ASSIGN_IF_VALID(ptr, value)  DO_WHILE(((ptr) ? (*(ptr) = (value)) : ((int)0));)
#define CALL_IF_VALID(funcptr, ...)  DO_WHILE((funcptr) ? (funcptr)(__VA_ARGS__) : ((void)0);)

/* ----------------------------- Boolian's ----------------------------- */

#undef FALSE
#undef TRUE

#define FALSE  0
#define TRUE   1

/* ----------------------------- Int's ----------------------------- */

#undef Ulong
#undef Uint
#undef Ushort
#undef Uchar

#define Ulong   unsigned long int
#define Uint    unsigned int
#define Ushort  unsigned short int
#define Uchar   unsigned char

/* ----------------------------- Constant's ----------------------------- */

#define _PTRSIZE  (sizeof(void *))

/* ----------------------------- Profiling ----------------------------- */

#define US_TO_MS(us)  ((us) / 1000.0f)

#define TIMER_START(name)  \
  struct timespec name;    \
  DO_WHILE(clock_gettime(CLOCK_MONOTONIC, &name);)

#define TIMER_END(start, time_ms_name) \
  float time_ms_name; \
  DO_WHILE(\
    struct timespec __timer_end;\
    clock_gettime(CLOCK_MONOTONIC, &__timer_end);\
    time_ms_name = \
      (((__timer_end.tv_sec - (start).tv_sec) * 1000000.0f) +\
      ((__timer_end.tv_nsec - (start).tv_nsec) / 1000.0f)); \
    time_ms_name = US_TO_MS(time_ms_name); \
  )

#define TIMER_PRINT(ms) \
  DO_WHILE(\
    printf("%s: Time: %.5f ms\n", __func__, (double)ms); \
  )

/* ----------------------------- Assert ----------------------------- */

#define ENABLE_ASSERT

#define FCIO_ASSERT_VOID_CAST                ((void)0)
#define FCIO_ASSERT_DIE_CAST(expr)           die_callback("%s: LINE:[%d]: FILE:[%s]: Assertion failed: [%s]\n", __func__, __LINE__, __FILE__, #expr)
#define FCIO_ASSERT_DIE_MSG_CAST(expr, msg)  die_callback("%s: LINE:[%d]: FILE:[%s]: Assertion failed: [%s]: %s\n", __func__, __LINE__, __FILE__, #expr, msg)

#define FCIO_DO_ASSERT(expr)           DO_WHILE((expr) ? FCIO_ASSERT_VOID_CAST : FCIO_ASSERT_DIE_CAST(expr);)
#define FCIO_DO_ASSERT_MSG(expr, msg)  DO_WHILE((expr) ? FCIO_ASSERT_VOID_CAST : FCIO_ASSERT_DIE_MSG_CAST(expr, msg);)

#ifdef ENABLE_ASSERT
# ifndef ASSERT
#   define ASSERT(expr)  FCIO_DO_ASSERT(expr)
# endif
# ifndef ASSERT_MSG
#   define ASSERT_MSG(expr, msg)  FCIO_DO_ASSERT_MSG(expr, msg)
# endif
#else
# ifndef ASSERT
#   define ASSERT(expr) FCIO_ASSERT_VOID_CAST
# endif
# ifndef ASSERT_MSG
#   define ASSERT_MSG(expr, msg)  FCIO_ASSERT_VOID_CAST
# endif
#endif

#ifndef ALWAYS_ASSERT
# define ALWAYS_ASSERT(expr)  FCIO_DO_ASSERT(expr)
#endif

#ifndef ALWAYS_ASSERT_MSG
# define ALWAYS_ASSERT_MSG(expr, msg)  FCIO_DO_ASSERT_MSG(expr, msg)
#endif

/* ----------------------------- Threads ----------------------------- */

/* Thread shorthand. */
#ifndef thread_t
# define thread_t  pthread_t
#endif

/* Mutex shorthand. */
#ifndef mutex_t
# define mutex_t  pthread_mutex_t
#endif

/* Condition shorthand. */
#ifndef cond_t
# define cond_t  pthread_cond_t
#endif

/* Mutex helper shorthands. */
#ifndef mutex_init
# define mutex_init  pthread_mutex_init
#endif
#ifndef mutex_destroy
# define mutex_destroy  pthread_mutex_destroy
#endif
#ifndef mutex_lock
# define mutex_lock  pthread_mutex_lock
#endif
#ifndef mutex_unlock
# define mutex_unlock  pthread_mutex_unlock
#endif
#ifndef mutex_action
# define mutex_action(mutex, action)  DO_WHILE(mutex_lock(mutex); DO_WHILE(action); mutex_unlock(mutex);)
#endif
#ifndef mutex_init_static
# define mutex_init_static  PTHREAD_MUTEX_INITIALIZER
#endif

/* ----------------------------- Ptr array's ----------------------------- */

#ifndef ENSURE_PTR_ARRAY_SIZE
# define ENSURE_PTR_ARRAY_SIZE(array, cap, size)                \
    DO_WHILE(                                                   \
      if ((cap) == (size)) {                                    \
        (cap) *= 2;                                             \
        (array) = xrealloc((array), (_PTRSIZE * (cap)));  \
      }                                                         \
    )
#endif

#ifndef TRIM_PTR_ARRAY
# define TRIM_PTR_ARRAY(array, cap, size) \
    DO_WHILE(\
      (cap) = ((size) + 1);\
      (array) = xrealloc((array), (_PTRSIZE * (cap))); \
    )
#endif

/* ----------------------------- Fd ----------------------------- */

#ifndef fdlock_action
# define fdlock_action(fd, type, action) DO_WHILE(fdlock(fd, type); DO_WHILE(action); fdunlock(fd);)
#endif


/* ----------------------------- ASCII ----------------------------- */

#ifndef ASCII_ISDIGIT
# define ASCII_ISDIGIT(c)  ((c) >= '0' && (c) <= '9')
#endif
#ifndef ASCII_TOUPPER
# define ASCII_TOUPPER(c)  (((c) < 'a' || (c) > 'z') ? (c) : ((c) - 'A'))
#endif
#ifndef ASCII_TOLOWER
# define ASCII_TOLOWER(c)  (((c) < 'A' || (c) > 'Z') ? (c) : ((c) + 'a'))
#endif
#ifndef ASCII_ISUPPER
# define ASCII_ISUPPER(c)  ((Uint)(c) >= 'A' && (Uint)(c) <= 'Z')
#endif
#ifndef ASCII_ISLOWER
# define ASCII_ISLOWER(c)  ((Uint)(c) >= 'a' && (Uint)(c) <= 'z')
#endif
#ifndef ASCII_ISALPHA
# define ASCII_ISALPHA(c)  (ASCII_ISUPPER(c) || ASCII_ISLOWER(c))
#endif
#ifndef ASCII_ISALNUM
# define ASCII_ISALNUM(c)  (ASCII_ISALPHA(c) || ASCII_ISDIGIT(c))
#endif

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

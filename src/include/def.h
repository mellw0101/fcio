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
#include <wchar.h>
#include <wctype.h>
#include <langinfo.h>
#include <locale.h>

/* ----------------------------- linux ----------------------------- */

#include <sys/stat.h>
#include <sys/inotify.h>

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
#ifdef __TYPE
# undef __TYPE
#endif
#ifdef MALLOC_STRUCT
# undef MALLOC_STRUCT
#endif
#ifdef SYS_BYTE_ORDER
# undef SYS_BYTE_ORDER
#endif
#ifdef SYS_LITTLE_ENDIAN
# undef SYS_LITTLE_ENDIAN
#endif
#ifdef SYS_BIG_ENDIAN
# undef SYS_BIG_ENDIAN
#endif
#ifdef PACKED_UINT
# undef PACKED_UINT
#endif
#ifdef UNPACK_UINT
# undef UNPACK_UINT
#endif

/* Perform actions protected from accidental missuse. */
#define DO_WHILE(...)                do {__VA_ARGS__} while (0)
#define CALL_IF_VALID(funcptr, ...)  DO_WHILE((funcptr) ? (funcptr)(__VA_ARGS__) : ((void)0);)

/* Deduce the type `x` is.  Works in both `c` and `c++`. */
#ifdef __cplusplus
# define __TYPE(x)  decltype(x)
#else
# define __TYPE(x)  __typeof__((x))
#endif

/* Malloc `ptr` by its type size, this is made to be used mainly on structures.  Works in both `c` and `c++` */
#ifdef __cplusplus
# define MALLOC_STRUCT(ptr)  DO_WHILE((ptr) = (__TYPE(ptr))xmalloc(sizeof(*(ptr)));)
#else
# define MALLOC_STRUCT(ptr)  DO_WHILE((ptr) = xmalloc(sizeof(*(ptr)));)
#endif

#if defined(__linux__)
# include <endian.h>
# define SYS_BYTE_ORDER     __BYTE_ORDER
# define SYS_BIG_ENDIAN     __BIG_ENDIAN
# define SYS_LITTLE_ENDIAN  __LITTLE_ENDIAN
#elif (defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__))
# include <sys/endian.h>
# define SYS_BYTE_ORDER     _BYTE_ORDER
# define SYS_BIG_ENDIAN     _BIG_ENDIAN
# define SYS_LITTLE_ENDIAN  _LITTLE_ENDIAN
#endif

#ifdef SYS_BYTE_ORDER
# if SYS_BYTE_ORDER == SYS_LITTLE_ENDIAN
#   define PACKED_UINT(r, g, b, a)                                                              \
      /* When using little endian memory layout, we place the least significant byte first. */  \
      (((Uchar)(a) << 24) | ((Uchar)(b) << 16) | ((Uchar)(g) << 8) | (Uchar)(r))
#   define UNPACK_UINT(x, index)                                        \
      /* Unpack a packed int and get the Uchar of the given `index`.    \
       * Note that this correctly parses the `index` so to get the `r`  \
       * value from a packed int use `index` zero. */                   \
      (((x) >> ((index) * 8)) & 0xFF)
# else
#   define PACKED_UINT(r, g, b, a)                                                          \
      /* When using big endian memory layout, we place the most significant byte first. */  \
      (((Uchar)(r) << 24) | ((Uchar)(g) << 16) | ((Uchar)(b) << 8) | (Uchar)(a))
#   define UNPACK_UINT(x, index)                                        \
      /* Unpack a packed int and get the Uchar of the given `index`.    \
       * Note that this correctly parses the `index` so to get the `r`  \
       * value from a packed int use `index` zero. */                   \
      (((x) >> (labs((index) - 3) * 8)) & 0xFF)
# endif
#endif

#define PACKED_UINT_FLOAT(r, g, b, a)  PACKED_UINT((255.0f * (r)), (255.0f * (g)), (255.0f * (b)), (255.0f * (a)))
#define UNPACK_UINT_FLOAT(x, index)    ((float)UNPACK_UINT(x, index) / 255.0f)

/* ----------------------------- String helper's ----------------------------- */

#ifdef STRLEN
# undef STRLEN
#endif
#ifdef S__LEN
# undef S__LEN
#endif
#ifdef COPY_OF
# undef COPY_OF
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

#ifdef Schar
# undef Schar
#endif
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

#define Schar   signed char
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

/* ----------------------------- Char define's ----------------------------- */

#ifdef wchar
# undef wchar
#endif

#define wchar  wchar_t

/* ----------------------------- Constant's ----------------------------- */

#ifdef _PTR_BITSIZE
# undef _PTR_BITSIZE
#endif
#ifdef _PTRSIZE
# undef _PTRSIZE
#endif

/* Size of a ptr in bits. */
#define _PTR_BITSIZE  __WORDSIZE
/* Size of a ptr in bytes. */
#define _PTRSIZE  (sizeof(void *))

/* ----------------------------- Math ----------------------------- */

#ifdef round_short
# undef round_short
#endif
#ifdef CLAMP_MAX
# undef CLAMP_MAX
#endif
#ifdef CLAMP_MIN
# undef CLAMP_MIN
#endif
#ifdef CLAMP
# undef CLAMP
#endif
#ifdef CLAMP_MAX_INLINE
# undef CLAMP_MAX_INLINE
#endif
#ifdef CLAMP_MIN_INLINE
# undef CLAMP_MIN_INLINE
#endif
#ifdef CLAMP_INLINE
# undef CLAMP_INLINE
#endif

#define round_short(x)  ((x) >= 0 ? (short)((x) + 0.5) : (short)((x) - 0.5))

/* Ensure `x` cannot be more then `max`. */
#define CLAMP_MAX(x, max)  (((x) > (max)) ? ((x) = (max)) : ((int)0))

/* Ensure `x` cannot be less then `min`. */
#define CLAMP_MIN(x, min)  (((x) < (min)) ? ((x) = (min)) : ((int)0))

/* Ensure `x` cannot be less then `min` nor more then `max`. */
#define CLAMP(x, min, max)  (((x) > (max)) ? ((x) = (max)) : ((x) < (min)) ? ((x) = (min)) : ((int)0))

#define CLAMP_MIN_INLINE(x, min) (((x) < (min)) ? (min) : (x))
#define CLAMP_MAX_INLINE(x, max) (((x) > (max)) ? (max) : (x))

#define CLAMP_INLINE(x, min, max)  \
  /* Clamp x as an expression and not as an assignment, meaning this   \
   * will never ever change `x` just ensure that the value from this   \
   * expression can never be more then max and never less then min */  \
  (((x) > (max)) ? (__TYPE(x))(max) : ((x) < (min)) ? (__TYPE(x))(min) : (__TYPE(x))(x))

/* ----------------------------- xterm ----------------------------- */

#ifdef xterm_byte_scale
# undef xterm_byte_scale
#endif
#ifdef xterm_color_index
# undef xterm_color_index
#endif
#ifdef xterm_grayscale_byte
# undef xterm_grayscale_byte
#endif
#ifdef grayscale_xterm_color_index
# undef grayscale_xterm_color_index
#endif
#ifdef xterm_grayscale_color_index
# undef xterm_grayscale_color_index
#endif

#define xterm_byte_scale(bit)  \
  /* Return`s a rounded xterm-256 scale value from a 8-bit rgb value.  */ \
  round_short(((double)bit / 255) * 5)

#define xterm_color_index(r, g, b)                              \
  /* Return the xterm-256 index for a given 8bit rgb value. */  \
  (short)(16 + (36 * xterm_byte_scale(r)) + (6 * xterm_byte_scale(g)) + xterm_byte_scale(b))

#define xterm_grayscale_byte(r, g, b)  round_short(((0.299 * (r) + 0.587 * (g) + 0.114 * (b)) / 255.0) * 5)

#define grayscale_xterm_color_index(r, g, b) \
  (16 + 36 * xterm_grayscale_byte(r, g, b) + 6 * xterm_grayscale_byte(r, g, b) + xterm_grayscale_byte(r, g, b))

#define xterm_grayscale_color_index(r, g, b) grayscale_xterm_color_index(r, g, b)

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
#ifdef timer_action
# undef timer_action
#endif

/* Convert micro-seconds to mili-seconds. */
#define US_TO_MS(us)  ((us) / 1000.0f)

/* Start a timer with the chosen name. */
#define TIMER_START(timer_name)                           \
  struct timespec timer_name;                             \
  DO_WHILE(clock_gettime(CLOCK_MONOTONIC, &timer_name);)  

/* End a timer and assign the result to a float declared outside the scope of this macro. */
#define TIMER_END_EXTERN_RESULT(start_timer, result_ms_name)         \
  DO_WHILE(                                                          \
    struct timespec __timer_end;                                     \
    clock_gettime(CLOCK_MONOTONIC, &__timer_end);                    \
    (result_ms_name) =                                               \
      (((__timer_end.tv_sec - (start_timer).tv_sec) * 1000000.0f) +  \
      ((__timer_end.tv_nsec - (start_timer).tv_nsec) / 1000.0f));    \
      (result_ms_name) = US_TO_MS(result_ms_name);                   \
  )

/* Same as `TIMER_END_EXTERN_RESULT()`, except this declares a float to hols the resulting time in `milli-seconds`. */
#define TIMER_END(start, time_ms_name)           \
  float time_ms_name;                            \
  TIMER_END_EXTERN_RESULT(start, time_ms_name);

/* Shorthand to print the function and time of the timer in `milli-seconds`. */
#define TIMER_PRINT(ms)                                   \
  DO_WHILE(                                               \
    writef("%s: Time: %.5f ms\n", __func__, (double)ms);  \
  )

/* Measure the time it takes to perform `action`, and create a named float that will hold the result. */
#define timer_action(result_ms_name, ...)           \
  float result_ms_name;                                \
  DO_WHILE(                                            \
    TIMER_START(__timer);                              \
    DO_WHILE(__VA_ARGS__);                                  \
    TIMER_END_EXTERN_RESULT(__timer, result_ms_name);  \
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
#ifdef thread_create
# undef thread_create
#endif
#ifdef thread_detach
# undef thread_detach
#endif
#ifdef thread_join
# undef thread_join
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
#ifdef cond_init
# undef cond_init
#endif
#ifdef cond_signal
# undef cond_signal
#endif
#ifdef cond_destroy
# undef cond_destroy
#endif
#ifdef cond_wait
# undef cond_wait
#endif
#ifdef rwlock_init
# undef rwlock_init
#endif
#ifdef rwlock_destroy
# undef rwlock_destroy
#endif

/* Thread shorthand. */
#define thread_t  pthread_t

/* Mutex shorthand. */
#define mutex_t  pthread_mutex_t

/* Condition shorthand. */
#define cond_t  pthread_cond_t

/* Thread helper shorthand's. */
#define thread_create  pthread_create
#define thread_detach  pthread_detach
#define thread_join    pthread_join

/* Mutex helper shorthand's. */
#define mutex_init                pthread_mutex_init
#define mutex_destroy             pthread_mutex_destroy
#define mutex_lock                pthread_mutex_lock
#define mutex_unlock              pthread_mutex_unlock
#define mutex_action(mutex, ...)  DO_WHILE(mutex_lock(mutex); DO_WHILE(__VA_ARGS__); mutex_unlock(mutex);)
#define mutex_init_static         PTHREAD_MUTEX_INITIALIZER

/* Condition helper shorthand's. */
#define cond_init     pthread_cond_init
#define cond_signal   pthread_cond_signal
#define cond_destroy  pthread_cond_destroy
#define cond_wait     pthread_cond_wait

/* Read-Write lock helper shorthand's. */
#define rwlock_init     pthread_rwlock_init
#define rwlock_destroy  pthread_rwlock_destroy

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

#ifdef __cplusplus
  /* Useful when adding something to a ptr array and checking the size each time. */
# define ENSURE_PTR_ARRAY_SIZE(array, cap, size)                         \
    DO_WHILE(                                                            \
      if ((cap) == (size)) {                                             \
        (cap) *= 2;                                                      \
        (array) = (__TYPE(array))xrealloc((array), (_PTRSIZE * (cap)));  \
      }                                                                  \
    )

/* To save memory rellocate the array to one more then size to hold a null ptr. */
# define TRIM_PTR_ARRAY(array, cap, size)                              \
    DO_WHILE(                                                          \
      (cap) = ((size) + 1);                                            \
      (array) = (__TYPE(array))xrealloc((array), (_PTRSIZE * (cap)));  \
    )
#else
  /* Useful when adding something to a ptr array and checking the size each time. */
# define ENSURE_PTR_ARRAY_SIZE(array, cap, size)          \
    DO_WHILE(                                             \
      if ((cap) == (size)) {                              \
        (cap) *= 2;                                       \
        (array) = xrealloc((array), (_PTRSIZE * (cap)));  \
      }                                                   \
    )

  /* To save memory rellocate the array to one more then size to hold a null ptr. */
# define TRIM_PTR_ARRAY(array, cap, size)               \
    DO_WHILE(                                           \
      (cap) = ((size) + 1);                             \
      (array) = xrealloc((array), (_PTRSIZE * (cap)));  \
    )
#endif

/* Shorthand to assign the value to a ptr.  Useful when assigning ptr passed as parameters to functions that might be `NULL`. */
#define ASSIGN_IF_VALID(ptr, value)  DO_WHILE(((ptr) ? (*(ptr) = (value)) : ((int)0));)

/* Get the size of a stack based array. */
#define ARRAY_SIZE(array)  (sizeof((array)) / sizeof((array)[0]))

/* ----------------------------- Fd ----------------------------- */

#ifdef fdlock_action
# undef fdlock_action
#endif

/* Perform `action` while under the protection of a file-descriptor lock. */
#define fdlock_action(fd, type, ...)  DO_WHILE(fdlock(fd, type); DO_WHILE(__VA_ARGS__); fdunlock(fd);)

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
#ifdef ASCII_ISWHITE
# undef ASCII_ISWHITE
#endif
#ifdef ASCII_CTRL
# undef ASCII_CTRL
#endif

#define ASCII_ISDIGIT(c)  ((c) >= '0' && (c) <= '9')
#define ASCII_TOUPPER(c)  (((c) < 'a' || (c) > 'z') ? (c) : ((c) - 'A'))
#define ASCII_TOLOWER(c)  (((c) < 'A' || (c) > 'Z') ? (c) : ((c) + 'a'))
#define ASCII_ISUPPER(c)  ((Uint)(c) >= 'A' && (Uint)(c) <= 'Z')
#define ASCII_ISLOWER(c)  ((Uint)(c) >= 'a' && (Uint)(c) <= 'z')
#define ASCII_ISALPHA(c)  (ASCII_ISUPPER(c) || ASCII_ISLOWER(c))
#define ASCII_ISALNUM(c)  (ASCII_ISALPHA(c) || ASCII_ISDIGIT(c))
#define ASCII_ISWHITE(c)  ((c) == ' ' || (c) == '\t')
#define ASCII_CTRL(c)     ((c)&037)

/* ----------------------------- 'dirs.c' Define's ----------------------------- */

#ifdef DIRECTORY_ITER
# undef DIRECTORY_ITER
#endif

#define DIRECTORY_ITER(dir, itername, entryname, ...)          \
  DO_WHILE(                                                    \
    for (Ulong itername=0; itername<(dir).len; ++itername) {   \
      directory_entry_t *entryname = (dir).entries[itername];  \
      DO_WHILE(__VA_ARGS__);                                   \
    }                                                          \
  )


/* ---------------------------------------------------------- Circular list define's ---------------------------------------------------------- */

#ifdef CLIST_SINGLE
# undef CLIST_SINGLE
#endif
#ifdef CLIST_ADV_NEXT
# undef CLIST_ADV_NEXT
#endif
#ifdef CLIST_ADV_PREV
# undef CLIST_ADV_PREV
#endif
#ifdef CLIST_ITER
# undef CLIST_ITER
#endif
#ifdef CLIST_INIT
# undef CLIST_INIT
#endif
#ifdef CLIST_UNLINK
# undef CLIST_UNLINK
#endif
#ifdef CLIST_INSERT_AFTER
# undef CLIST_INSERT_AFTER
#endif

#define CLIST_SINGLE(listptr)    ((listptr) == (listptr)->next)
#define CLIST_ADV_NEXT(listptr)  DO_WHILE((listptr) = (listptr)->next;)
#define CLIST_ADV_PREV(listptr)  DO_WHILE((listptr) = (listptr)->prev;)

#define CLIST_ITER(headptr, ptr, ...)  \
  DO_WHILE(                            \
    if (headptr) {                     \
      __TYPE(headptr) ptr;             \
      __TYPE(headptr) next = headptr;  \
      do {                             \
        ptr = next;                    \
        next = next->next;             \
        DO_WHILE(__VA_ARGS__);         \
      } while (next != headptr);       \
    }                                  \
  )

#define CLIST_INIT(ptr)  DO_WHILE((ptr)->next = (ptr); (ptr)->prev = (ptr);)

/* Unlink a circular linked list node, this only ensures that the node before and after are correctly linked. */
#define CLIST_UNLINK(ptr)             \
  DO_WHILE(                           \
    (ptr)->prev->next = (ptr)->next;  \
    (ptr)->next->prev = (ptr)->prev;  \
  )

#define CLIST_INSERT_AFTER(ptr, after)  \
  DO_WHILE(                             \
    __TYPE(after) ap = (after);         \
    __TYPE(ptr)    p = (ptr);           \
    (p)->prev        = (ap);            \
    (p)->next        = (ap)->next;      \
    (ap)->next->prev = (p);             \
    (ap)->next       = (p);             \
  )

/* ----------------------------- Double linked list helper's ----------------------------- */

#ifdef DLIST_FOR_NEXT
# undef DLIST_FOR_NEXT
#endif
#ifdef DLIST_FOR_PREV
# undef DLIST_FOR_PREV
#endif
#ifdef DLIST_FOR_NEXT_END
# undef DLIST_ND_FOR_NEXT_END
#endif
#ifdef DLIST_FOR_PREV_END
# undef DLIST_ND_FOR_NEXT_END
#endif
#ifdef DLIST_ND_FOR_NEXT
# undef DLIST_ND_FOR_NEXT
#endif
#ifdef DLIST_ND_FOR_PREV
# undef DLIST_ND_FOR_PREV
#endif
#ifdef DLIST_ND_FOR_NEXT_END
# undef DLIST_ND_FOR_NEXT_END
#endif
#ifdef DLIST_ND_FOR_PREV_END
# undef DLIST_ND_FOR_NEXT_END
#endif
#ifdef DLIST_INSERT_AFTER
# undef DLIST_INSERT_AFTER
#endif
#ifdef DLIST_INSERT_DLIST_AFTER
# undef DLIST_INSERT_DLIST_AFTER
#endif
#ifdef DLIST_ADV_NEXT
# undef DLIST_ADV_NEXT
#endif
#ifdef DLIST_ADV_PREV
# undef DLIST_ADV_PREV
#endif
#ifdef DLIST_UNLINK
# undef DLIST_UNLINK
#endif

#define DLIST_FOR_NEXT(start, name)                                                  \
  /* Iterate over a double linked list starting at `start` and iterating using       \
   * `(name) = (name)->next` until we reach a `NULL`.  Note that this can take       \
   * the `start` ptr in any constness as it will clean the type for declaration. */  \
  for (__TYPE(&(*(start))) name=(start); name; name=name->next)

#define DLIST_FOR_PREV(start, name)                                                  \
  /* Iterate over a double linked list starting at `start` and iterating using       \
   * `(name) = (name)->prev` until we reach a `NULL`.  Note that this can take       \
   * the `start` ptr in any constness as it will clean the type for declaration. */  \
  for (__TYPE(&(*(start))) name=(start); name; name=name->prev)

#define DLIST_FOR_NEXT_END(start, end, name)                                         \
  /* Iterate over a double linked list starting at `start` and iterating using       \
   * `(name) = (name)->next` until we reach `end`.  Note that this can take          \
   * the `start` ptr in any constness as it will clean the type for declaration. */  \
  for (__TYPE(&(*(start))) name=(start); name && name!=(end); name=name->next)

#define DLIST_FOR_PREV_END(start, end, name)                                         \
  /* Iterate over a double linked list starting at `start` and iterating using       \
   * `(name) = (name)->prev` until we reach `end`.  Note that this can take          \
   * the `start` ptr in any constness as it will clean the type for declaration. */  \
  for (__TYPE(&(*(start))) name=(start); name && name!=(end); name=name->prev)

#define DLIST_ND_FOR_NEXT(start, name)                                    \
  /* Iterate over a double linked list starting at `start` and iterating  \
   * using `(name) = (name)->next` until we reach a `NULL`.  Note that    \
   * this does not declare `name` rather uses an existing ptr.  */        \
  for ((name)=(start); (name); (name)=(name)->next)

#define DLIST_ND_FOR_PREV(start, name)                                    \
  /* Iterate over a double linked list starting at `start` and iterating  \
   * using `(name) = (name)->prev` until we reach a `NULL`.  Note that    \
   * this does not declare `name` rather uses an existing ptr. */         \
  for ((name)=(start); (name); (name)=(name)->prev)

#define DLIST_ND_FOR_NEXT_END(start, end, name)                           \
  /* Iterate over a double linked list starting at `start` and iterating  \
   * using `(name) = (name)->next` until we reach `end`.  Note that       \
   * this does not declare `name` rather uses an existing ptr.  */        \
  for ((name)=(start); (name) && (name)!=(end); (name)=(name)->next)

#define DLIST_ND_FOR_PREV_END(start, end, name)                           \
  /* Iterate over a double linked list starting at `start` and iterating  \
   * using `(name) = (name)->prev` until we reach `end`.  Note that       \
   * this does not declare `name` rather uses an existing ptr. */         \
  for ((name)=(start); (name) && (name)!=(end); (name)=(name)->prev)

#define DLIST_INSERT_AFTER(after, node)                       \
  /* Insert `node` after `after` in a double linked list. */  \
  DO_WHILE(                                                   \
    __TYPE(after) ap = (after);                               \
    __TYPE(node)  np = (node);                                \
    np->next = ap->next;                                      \
    np->prev = ap;                                            \
    if (ap->next) {                                           \
      ap->next->prev = np;                                    \
    }                                                         \
    ap->next = np;                                            \
  )

#define DLIST_INSERT_DLIST_AFTER(after, top, bot)                     \
  /* Insert an double linked list into another one after `after`. */  \
  DO_WHILE(                                                           \
    /* Make a new pointer for all passed params, as                   \
     * this not only ensures that there are no side-                  \
     * effects, but also that we have some type safety */             \
    __TYPE(after) ap = (after);                                       \
    __TYPE(top)   tp = (top);                                         \
    __TYPE(bot)   bp = (bot);                                         \
    bp->next = ap->next;                                              \
    if (bp->next) {                                                   \
      bp->next->prev = bp;                                            \
    }                                                                 \
    ap->next = tp;                                                    \
    tp->prev = ap;                                                    \
  )

#define DLIST_ADV_NEXT(ptr)  CLIST_ADV_NEXT(ptr)
#define DLIST_ADV_PREV(ptr)  CLIST_ADV_PREV(ptr)

#define DLIST_UNLINK(ptr)               \
  DO_WHILE(                             \
    if ((ptr)->prev) {                  \
      (ptr)->prev->next = (ptr)->next;  \
    }                                   \
    if ((ptr)->next) {                  \
      (ptr)->next->prev = (ptr)->prev;  \
    }                                   \
  )

/* ----------------------------- Struct helper define's ----------------------------- */

#ifdef STRUCT_FIELD_PTR
# undef STRUCT_FIELD_PTR
#endif

#define STRUCT_FIELD_PTR(struct, field)  ((const char *)(struct) + offsetof(__TYPE(*struct), field))


/* ---------------------------------------------------------- Typedef's ---------------------------------------------------------- */


typedef void (*FreeFuncPtr)(void *);
typedef int (*CmpFuncPtr)(const void *, const void *);


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

typedef struct HashNodeNum  HashNodeNum;
typedef struct HashMapNum   HashMapNum;

/* ----------------------------- future.c ----------------------------- */

typedef struct Future  Future;

/* ----------------------------- atomic_bool_sync.c ----------------------------- */

typedef struct {
  volatile int value;
} atomic_bool_sync;

/* ----------------------------- atomicbool.c ----------------------------- */

typedef struct atomicbool  atomicbool;

/* ----------------------------- queue.c ----------------------------- */

typedef struct Queue  Queue;


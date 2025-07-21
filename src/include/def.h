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
#include <math.h>

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
#ifdef __TYPE_SAME
# undef __TYPE_SAME
#endif
#ifdef __SAFE_TYPE
# undef __SAFE_TYPE
#endif
#ifdef STATIC_TYPE_MATCH
# undef STATIC_TYPE_MATCH
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
#ifdef _UNUSED_ARG
# undef _UNUSED_ARG
#endif
#ifdef SWAP
# undef SWAP
#endif
#ifdef PASS_IF_VALID
# undef PASS_IF_VALID
#endif
#ifdef PASS_FIELD_IF_VALID
# undef PASS_FIELD_IF_VALID
#endif
#ifdef PASS_IF_NON_NEG
# undef PASS_IF_NON_NEG
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

#define FCLAMPF(x, min, max)  (FMAXF((min), FMINF((max), (x))))

#define __TYPE_SAME(x, y)                       \
  /* Deduses the *common* type for `x` and `y`  \
   * (using usual arithmetic conversion).  */   \
  __TYPE(((x) + (y)) - (y))

#define __SAFE_TYPE(x, y)                               \
  /* Expresses y, in a type safe mannor, when relating  \
   * to x.  Usage: if (x < __SAFE_TYPE(x, y)) */        \
  (__TYPE_SAME(x, y))(y)

#define STATIC_TYPE_MATCH(x, y)  ((void)sizeof(char[1 - 2*!(sizeof(__TYPE(x)) == sizeof(__TYPE(y)))]))
#define STATIC_PTR_TYPE_MATCH(x, y) \
  DO_WHILE(                                                 \
    ((void)sizeof(char[1 - 2 * !(sizeof(*x) == sizeof(*y))])); \
  )

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
      ((Uint)(((Uchar)(a) << 24) | ((Uchar)(b) << 16) | ((Uchar)(g) << 8) | (Uchar)(r)))
#   define UNPACK_UINT(x, index)                                        \
      /* Unpack a packed int and get the Uchar of the given `index`.    \
       * Note that this correctly parses the `index` so to get the `r`  \
       * value from a packed int use `index` zero. */                   \
      ((Uchar)(((x) >> ((index) * 8)) & 0xFF))
#   define PACK_INT_DATA(x, value, byte_offset)                                            \
      /* This thing is nice, and will allow packing data into int's in nice ways. */       \
      DO_WHILE(                                                                            \
        (x) = (                                                                            \
          ((x) & ~(                                                                        \
            ((__TYPE(x))(1ULL << ((sizeof(__TYPE(value)) * 8) - 1)) << ((byte_offset) * 8))  \
          ))                                                                                 \
          |                                                                                  \
          ((__TYPE(x))((value) << ((byte_offset) * 8)))                                      \
        );                                                                                 \
      )
#   define UNPACK_INT_DATA(x, index, type)                                      \
      /* Unpack a packed int and get the Uchar of the given `index`.            \
        * Note that this correctly parses the `index` so to get the `r`         \
        * value from a packed int use `index` zero. */                          \
      ((type)(((x) >> ((index) * 8)) & (((type)1 << (sizeof(type) * 8)) - 1)))
# else
#   define PACKED_UINT(r, g, b, a)                                                          \
      /* When using big endian memory layout, we place the most significant byte first. */  \
      ((Uint)(((Uchar)(r) << 24) | ((Uchar)(g) << 16) | ((Uchar)(b) << 8) | (Uchar)(a)))
#   define UNPACK_UINT(x, index)                                        \
      /* Unpack a packed int and get the Uchar of the given `index`.    \
       * Note that this correctly parses the `index` so to get the `r`  \
       * value from a packed int use `index` zero. */                   \
      ((Uchar)(((x) >> (LABS((index) - 3) * 8)) & 0xFF))
#   define PACK_INT_DATA(x, value, byte)                                                                                             \
      /* This thing is nice, and will allow packing data into int's in nice ways. */                                                 \
      DO_WHILE(                                                                                                                      \
        (x) = (                                                                                                                      \
          ((x) & ~(                                                                                                                  \
            ((__TYPE(x))(1ULL << ((LABS((byte) - (sizeof(__TYPE(x)) - 1)) * 8) - 1)) << (LABS((byte) - (sizeof(__TYPE(x)) - 1)) * 8))  \
          ))                                                                                                                         \
          |                                                                                                                          \
          ((__TYPE(x))((value) << (LABS((byte) - (sizeof(__TYPE(x)) - 1)) * 8)))                                                      \
        );                                                                                                                           \
      )
#   define UNPACK_INT_DATA(x, index, type)                              \
      /* Unpack a packed int and get the Uchar of the given `index`.    \
       * Note that this correctly parses the `index` so to get the `r`  \
       * value from a packed int use `index` zero. */                   \
      ((type)(((x) >> (LABS((index) - (sizeof(__TYPE(x)) - 1)) * 8)) & (((type)1 << (sizeof(type) * 8)) - 1)))
# endif
#endif

#define PACKED_UINT_FLOAT(r, g, b, a)  PACKED_UINT((255.0f * (r)), (255.0f * (g)), (255.0f * (b)), (255.0f * (a)))
#define UNPACK_UINT_FLOAT(x, index)    ((float)UNPACK_UINT(x, index) / 255.0f)
#define UNPACK_FUINT(x, index)         ((float)UNPACK_UINT(x, index) / 255.0f)

#define FLOAT_TO_UCHAR(x)  ((Uchar)(255.f * FCLAMPF(x, 0, 1)))
#define UCHAR_TO_FLOAT(x)  ((float)((x) / 255.f))

#define FLOAT_BITS(x)  __extension__({ union { float f; uint32_t u; } _tmp = { .f = (x) }; _tmp.u; })
#define BITS_FLOAT(x)  __extension__({ union { float f; uint32_t u; } _tmp = { .u = (x) }; _tmp.f; })

#define PACK_SIGNED_PRECENT(x)  \
  /* Pack any precentage from -100 to 100 with a resolution of `1.f`. */  \
  ((Uchar)((((x) < 0) << 7) | ((int)(FMINF(100.f, FABSF(x))) & 0x7F)))
#define UNPACK_SIGNED_PRECENT(x)                                          \
  /* Unpack a precentage from -100 to 100 with a resolution of `1.f`. */  \
  ((((x) & 0x80) ? -1 : 1) * ((x) & 0x7F))


#define UNPACK_ND_FUINT_VARS(x, f0, f1, f2, f3)     \
  /* Unpack a packed uint into 4 already declared.  \
   * floats, with values between 0-1. */            \
  DO_WHILE(                                         \
    f0 = UNPACK_FUINT(x, 0);                        \
    f1 = UNPACK_FUINT(x, 1);                        \
    f2 = UNPACK_FUINT(x, 2);                        \
    f3 = UNPACK_FUINT(x, 3);                        \
  )

#define UNPACK_FUINT_VARS(x, f0, f1, f2, f3)  \
  /* Unpack a packed uint into 4              \
   * floats, with values between 0-1. */      \
  float f0; \
  float f1; \
  float f2; \
  float f3; \
  UNPACK_ND_FUINT_VARS(x, f0, f1, f2, f3)

#define _UNUSED_ARG(x) \
  /* Can be used instead of __attribute__((__unused__)) for a function argument. */ \
  ((void)(x))

#define SWAP(x, y)                              \
  /* Swap `x` and `y`.  Note that the           \
   * temporary ptr will use the type of `x`.    \
   * Also note that this is not atomic, and     \
   * `ATOMIC_SWAP()` should be used to perform  \
   * a fully atomic swap. */                    \
  DO_WHILE(                                     \
    __TYPE(x) __tmp = (x);                      \
    (x) = (y);                                  \
    (y) = __tmp;                                \
  )

#define PASS_IF_NON_NEG(x, y)         \
  /* Pass `x` if it's a non-negative  \
   * value.  Otherwise, pass `y`. */  \
  (((x) < 0) ? (y) : (x))

#define PASS_IF_VALID(x, y)   \
  /* Pass `x` if it's valid,  \
   * otherwise, pass `y`. */  \
  ((x) ? (x) : (y))

#define PASS_FIELD_IF_VALID(x, field, y)                       \
  /* If `x` is valid, pass `x->field`.  Otherwize pass `y` */  \
  ((x) ? (x)->field : (y))

/* ----------------------------- Atomic operation helper's ----------------------------- */

#ifdef ATOMIC_SWAP
# undef ATOMIC_SWAP
#endif
#ifdef ATOMIC_STORE
# undef ATOMIC_STORE
#endif
#ifdef ATOMIC_FETCH
# undef ATOMIC_FETCH
#endif
#ifdef ATOMIC_CAS
# undef ATOMIC_CAS
#endif

#ifndef NO_ATOMIC_OPERATIONS
  /* Swap */
# define ATOMIC_SWAP(x, y)                          \
    DO_WHILE(                                       \
      __ATOMIC_SWAP(&(y), __ATOMIC_SWAP(&(x), y));  \
    )

  /* Store */
# define ATOMIC_STORE(x, value)     \
    DO_WHILE(                       \
      __ATOMIC_STORE(&(x), value);  \
    )
    
  /* Fetch */
# define ATOMIC_FETCH(x)  \
    __ATOMIC_FETCH(&(x))

  /* Compare-and-swap (CAS) */
# define ATOMIC_CAS(x, expected, disired)  \
    __ATOMIC_CAS(&(x), &(expected), disired)
#endif

/* ----------------------------- Safe compare helper's ----------------------------- */

#ifdef LT
# undef LT
#endif
#ifdef GT
# undef GT
#endif
#ifdef LE
# undef LE
#endif
#ifdef GE
# undef GE
#endif
#ifdef EQ
# undef EQ
#endif
#ifdef NE
# undef NE
#endif
#ifdef IS_SIGNED
# undef IS_SIGNED
#endif
#ifdef SAFE_MIXED_SIGN_OP
# undef SAFE_MIXED_SIGN_OP
#endif

#define IS_SIGNED(x)  ((__TYPE(x))-1 < (__TYPE(x))0)

#define SAFE_MIXED_SIGN_OP(x, y, op, x_under, y_under)      \
  ((IS_SIGNED(x) == IS_SIGNED(y))                           \
    ? ((x) op (y))                                          \
    : IS_SIGNED(x)                                          \
      ? (((x) < 0) ? x_under : (((__TYPE(y))(x)) op (y)))   \
      : (((y) < 0) ? y_under : ((x) op ((__TYPE(x))(y)))))

/* Safe comparison shorthands, always upconverting y to the type of (x op y). */
#define LT(x, y)  SAFE_MIXED_SIGN_OP(x, y, <,  TRUE,  FALSE)
#define GT(x, y)  SAFE_MIXED_SIGN_OP(x, y, >,  FALSE, TRUE)
#define LE(x, y)  SAFE_MIXED_SIGN_OP(x, y, <=, TRUE,  FALSE)
#define GE(x, y)  SAFE_MIXED_SIGN_OP(x, y, >=, FALSE, TRUE)
#define EQ(x, y)  SAFE_MIXED_SIGN_OP(x, y, ==, FALSE, FALSE)
#define NE(x, y)  SAFE_MIXED_SIGN_OP(x, y, !=, TRUE,  TRUE)

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
#ifdef Llong
# undef Llong
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

#if (defined(__LONG_LONG_WIDTH__))
# if (__LONG_LONG_WIDTH__ == 64)
#   if (__LONG_WIDTH__ == __LONG_LONG_WIDTH__)
#     define Llong  long int
#   else
#     define Llong  long long int
#   endif
# elif (__LONG_WIDTH__ == 64)
#   define Llong  long int
# else
#   error "This system cannot represent an interger with the width of 64 bits"
# endif
#elif (defined(__LLONG_WIDTH__))
# if (__LLONG_WIDTH__ == 64)
#   if (__LONG_WIDTH__ == __LLONG_WIDTH__)
#     define Llong  long int
#   else
#     define Llong  long long int
#   endif
# elif (__LONG_WIDTH__ == 64)
#   define Llong  long int
# else
#   error "This system cannot represent an interger with the width of 64 bits"
# endif
#endif

/* ----------------------------- Char define's ----------------------------- */

#ifdef wchar
# undef wchar
#endif
#ifdef Wchar
# undef Wchar
#endif
#ifdef Wint
# undef Wint
#endif

#define wchar  wchar_t

#define Wchar  wchar_t
#define Wint   wint_t

/* ----------------------------- Constant's ----------------------------- */

#ifdef _PTR_BITSIZE
# undef _PTR_BITSIZE
#endif
#ifdef _PTRSIZE
# undef _PTRSIZE
#endif
#ifdef M_PIf
# undef M_PIf
#endif

/* Size of a ptr in bits. */
#define _PTR_BITSIZE  __WORDSIZE
/* Size of a ptr in bytes. */
#define _PTRSIZE  (sizeof(void *))

/* Ripped from <math.h>. */
#define M_PIf	3.14159265358979323846f	/* pi */

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

#define SAFE_CLAMP(x, min, max)                                            \
  DO_WHILE(                                                                \
    __TYPE(min) __min = (min);                                             \
    __TYPE(max) __max = (max);                                             \
    GT(x, __max) ? (x = __max) : (LT(x, __min) ? (x = __min) : ((int)0));  \
  )

/* Ensure `x` cannot be more then `max`. */
#define CLAMP_MAX(x, max)  (((x) > (__TYPE_SAME(x, max))(max)) ? ((x) = (__TYPE_SAME(x, max))(max)) : ((int)0))

/* Ensure `x` cannot be less then `min`. */
#define CLAMP_MIN(x, min)  (((x) < (__TYPE(x))(min)) ? ((x) = (min)) : ((int)0))

/* Ensure `x` cannot be less then `min` nor more then `max`. */
#define CLAMP(x, min, max)  (((x) > (__TYPE(x))(max)) ? ((x) = (max)) : ((x) < (__TYPE(x))(min)) ? ((x) = (min)) : ((int)0))

#define CLAMP_MIN_INLINE(x, min) (((x) < (__TYPE(x))(min)) ? (__TYPE(x))(min) : (x))
#define CLAMP_MAX_INLINE(x, max) (((x) > (__TYPE(x))(max)) ? (__TYPE(x))(max) : (x))

#define CLAMP_INLINE(x, min, max)  \
  /* Clamp x as an expression and not as an assignment, meaning this           \
   * will never ever change `x` just ensure that the value from this           \
   * expression can never be more then max and never less then min */          \
  (GT(x, max) ? ((__TYPE(x))(max)) : (LT(x, min) ? ((__TYPE(x))(min)) : (x)))

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

/* ----------------------------- Time ----------------------------- */

#ifdef RDTSC
# undef RDTSC
#endif
#ifdef RDTSCL
# undef RDTSCL
#endif
#ifdef TIMESPEC_ELAPSED_SEC
# undef TIMESPEC_ELAPSED_SEC
#endif
#ifdef TIMESPEC_ELAPSED_MS
# undef TIMESPEC_ELAPSED_MS
#endif
#ifdef TIMESPEC_ELAPSED_NS
# undef TIMESPEC_ELAPSED_NS
#endif
#ifdef FRAME_SWAP_RATE_TIME_MS
# undef FRAME_SWAP_RATE_TIME_MS
#endif
#ifdef FRAME_SWAP_RATE_TIME_NS
# undef FRAME_SWAP_RATE_TIME_NS
#endif
#ifdef FRAME_SWAP_RATE_TIME_NS_INT
# undef FRAME_SWAP_RATE_TIME_NS_INT
#endif

#define RDTSC(low, high)            \
    __asm__ __volatile__("rdtsc"    \
        : "=a" (low),               \
          "=d" (high)               \
    )

#define RDTSCL(low)                 \
    __asm__ __volatile__("rdtsc"    \
        : "=a" (low)                \
        : /* no inputs */           \
        : "edx"                     \
    )

#define TIMESPEC_ELAPSED_SEC(s, e)  (((e)->tv_sec - (s)->tv_sec) + ((double)((e)->tv_nsec - (s)->tv_nsec) / 1e9))
#define TIMESPEC_ELAPSED_MS(s, e)   ((((e)->tv_sec - (s)->tv_sec) * 1e3) + ((double)((e)->tv_nsec - (s)->tv_nsec) / 1e6))
#define TIMESPEC_ELAPSED_NS(s, e)   ((((e)->tv_sec - (s)->tv_sec) * 1000000000LL) + ((e)->tv_nsec - (s)->tv_nsec))

#define MILLI_TO_NANO(x)  ((Llong)(((double)(x) * 1e6) + 0.5))
#define NANO_TO_MILLI(x)  ((double)(x) / 1e6)

#define FRAME_SWAP_RATE_TIME_MS(x)  (1e3 / (x))
#define FRAME_SWAP_RATE_TIME_NS(x)  (1e9 / (x))

#define FRAME_SWAP_RATE_TIME_NS_INT(x)  ((Llong)((1e9 / (x)) + 0.5))

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
#ifdef rwlock_t
# undef rwlock_t
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
#ifdef RWLOCK_INIT
# undef RWLOCK_INIT
#endif
#ifdef RWLOCK_DESTROY
# undef RWLOCK_DESTROY
#endif
#ifdef RWLOCK_RDLOCK
# undef RWLOCK_RDLOCK
#endif
#ifdef RWLOCK_RWLOCK
# undef RWLOCK_RWLOCK
#endif
#ifdef RWLOCK_UNLOCK
# undef RWLOCK_UNLOCK
#endif
#ifdef RWLOCK_WRLOCK_ACTION
# undef RWLOCK_WRLOCK_ACTION
#endif
#ifdef RWLOCK_RDLOCK_ACTION
# undef RWLOCK_RDLOCK_ACTION
#endif

/* Thread shorthand. */
#define thread_t  pthread_t

/* Mutex shorthand. */
#define mutex_t  pthread_mutex_t

/* Condition shorthand. */
#define cond_t  pthread_cond_t

/* Read-Write shorthand. */
#define rwlock_t  pthread_rwlock_t

/* Thread helper shorthand's. */
#define thread_create  pthread_create
#define thread_detach  pthread_detach
#define thread_join    pthread_join

/* Mutex helper shorthand's. */
#define mutex_init                pthread_mutex_init
#define mutex_destroy             pthread_mutex_destroy
#define mutex_lock                pthread_mutex_lock
#define mutex_unlock              pthread_mutex_unlock
#define mutex_action(mutex, ...)  DO_WHILE(mutex_lock((mutex)); DO_WHILE(__VA_ARGS__); mutex_unlock((mutex));)
#define mutex_init_static         PTHREAD_MUTEX_INITIALIZER

/* Condition helper shorthand's. */
#define cond_init     pthread_cond_init
#define cond_signal   pthread_cond_signal
#define cond_destroy  pthread_cond_destroy
#define cond_wait     pthread_cond_wait

/* Read-Write lock helper shorthand's. */
#define RWLOCK_INIT     pthread_rwlock_init
#define RWLOCK_DESTROY  pthread_rwlock_destroy
#define RWLOCK_RDLOCK   pthread_rwlock_rdlock
#define RWLOCK_RWLOCK   pthread_rwlock_wrlock
#define RWLOCK_UNLOCK   pthread_rwlock_unlock

#define RWLOCK_WRLOCK_ACTION(rwlock, ...)  \
  DO_WHILE(                                \
    RWLOCK_RWLOCK((rwlock));               \
    DO_WHILE(__VA_ARGS__);                 \
    RWLOCK_UNLOCK((rwlock));               \
  )

#define RWLOCK_RDLOCK_ACTION(rwlock, ...)  \
  DO_WHILE(                                \
    RWLOCK_RDLOCK((rwlock));               \
    DO_WHILE(__VA_ARGS__);                 \
    RWLOCK_UNLOCK((rwlock));               \
  )

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
#ifdef ASSIGN_FIELD_IF_VALID
# undef ASSIGN_FIELD_IF_VALID
#endif
#ifdef ARRAY_SIZE
# undef ARRAY_SIZE
#endif
#ifdef ARRAY__LEN
# undef ARRAY__LEN
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

#define ASSIGN_FIELD_IF_VALID(ptr, field, value)      \
  DO_WHILE(                                           \
    /* (ptr) ? ((ptr)->field=(value)) : ((int)0); */  \
    if (ptr) {                                        \
      (ptr)->field = (value);                         \
    }                                                 \
  )

/* Get the size of a stack based array. */
#define ARRAY_SIZE(array)  (sizeof((array)) / sizeof((array)[0]))

/* Pass a stack-based array and its size. */
#define ARRAY__LEN(array)  (array), ARRAY_SIZE(array)

/* ----------------------------- Fd ----------------------------- */

#ifdef fdlock_action
# undef fdlock_action
#endif
#ifdef mutex_fdlock_action
# undef mutex_fdlock_action
#endif

/* Perform `action` while under the protection of a file-descriptor lock. */
#define fdlock_action(fd, type, ...)  DO_WHILE(fdlock((fd), (type)); DO_WHILE(__VA_ARGS__); fdunlock((fd));)

#define fd_full_wr(fd, tot, wrlen, data, datalen, term_on_error)                                       \
  DO_WHILE(                                                                                            \
    (tot) = 0;                                                                                         \
    while ((tot) < (datalen) && ((wrlen) = write((fd), ((data) + (tot)), ((datalen) - (tot)))) > 0) {  \
      (tot) += (wrlen);                                                                                \
    }                                                                                                  \
    if ((term_on_error)) {                                                                             \
      ALWAYS_ASSERT((wrlen) != -1 && (tot) == (datalen));                                              \
    }                                                                                                  \
  )

#define mutex_fdlock_action(mutex, fd, type, ...)                  \
  /* Perform some `action` while under mutex and true fd-lock. */  \
  mutex_action(mutex, fdlock_action(fd, type, __VA_ARGS__);)

#define mutex_fdlock_full_wr(mutex, fd, total_written, written_len, data, datalen, term_on_error) \
  /* Perform a full write of all data, while under mutex and full fd-lock. */ \
  mutex_fdlock_action(mutex, fd, F_WRLCK, fd_full_wr(fd, total_written, written_len, data, datalen, term_on_error);)

/* ----------------------------- ASCII ----------------------------- */

#ifdef NUL
# undef NUL
#endif
#ifdef ETX
# undef ETX
#endif
#ifdef EOT
# undef EOT
#endif
#ifdef BEL
# undef BEL
#endif
#ifdef BS
# undef BS
#endif
#ifdef TAB
# undef TAB
#endif
#ifdef LF
# undef LF
#endif
#ifdef FF
# undef FF
#endif
#ifdef CR
# undef CR
#endif
#ifdef XON
# undef XON
#endif
#ifdef XOFF
# undef XOFF
#endif
#ifdef CAN
# undef CAN
#endif
#ifdef SUB
# undef SUB
#endif
#ifdef SP
# undef SP
#endif

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
# undef ASCII_ISLOWER
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

#define NUL                                  \
  /* The hex char value of a c-string null.  \
   * (0x00 == '\000' == '\0' == 0). */       \
  0x00
#define ETX                                 \
  /* The hex value for a end-of-text (^C).  \
   * (0x03 == '\003' == ^c == 3). */        \
  ASCII_CTRL('c')
#define EOT                                  \
  /* The hex value for a end-of-input (^D).  \
   * (0x04 == '\004' == ^d == 4). */         \
  ASCII_CTRL('d')
#define BEL                              \
  /* The hex char value of a bell code.  \
   * (0x07 == '\007' == 7). */           \
  0x07
#define BS                               \
  /* The hex char value of a backspace.  \
   * (0x08 == '\010' == '\b' == 8). */   \
  0x08
#define TAB                              \
  /* The hex char value of a tabulator.  \
   * (0x09 == '\011' == '\t' == 9). */   \
  0x09
#define LF                               \
  /* The hex char value of a new-line.   \
   * (0x0A == '\012' == '\n' == 10). */  \
  0x0A
#define FF                               \
  /* The hex char value of a form-feed.  \
   * (0x0C == '\014' == '\f' == 12). */  \
  0x0C
#define CR                                     \
  /* The hex char value of a carriage-return.  \
   * (0x0D == '\015' == '\r' == 13) */         \
  0x0D
#define XON                                     \
  /* The hex value for a flow-control-on (^Q).  \
   * (0x11 == '\021' == ^q == 17). */           \
  ASCII_CTRL('q')
#define XOFF                                     \
  /* The hex value for a flow-control-off (^S).  \
   * (0x13 == '\023' == ^s == 19). */            \
  ASCII_CTRL('s')
#define CAN                            \
  /* The hex value for a cancel (^X).  \
   * (0x18 == '\030' == ^x == 24). */  \
  ASCII_CTRL('x')
#define SUB                             \
  /* The hex value for a suspend (^Z).  \
   * (0x1A == '\032' == ^z == 26). */   \
  ASCII_CTRL('z')
#define SP                              \
  /* The hex char value for a space.    \
   * (0x20 == '\040' == ' ' == 32). */  \
  0x20

#define ASCII_ISDIGIT(c)  ((c) >= '0' && (c) <= '9')
#define ASCII_TOUPPER(c)  (((c) < 'a' || (c) > 'z') ? (c) : ((c) - ('a' - 'A')))
#define ASCII_TOLOWER(c)  (((c) < 'A' || (c) > 'Z') ? (c) : ((c) + ('a' - 'A')))
#define ASCII_ISUPPER(c)  ((Uint)(c) >= 'A' && (Uint)(c) <= 'Z')
#define ASCII_ISLOWER(c)  ((Uint)(c) >= 'a' && (Uint)(c) <= 'z')
#define ASCII_ISALPHA(c)  (ASCII_ISUPPER(c) || ASCII_ISLOWER(c))
#define ASCII_ISALNUM(c)  (ASCII_ISALPHA(c) || ASCII_ISDIGIT(c))
#define ASCII_ISWHITE(c)  ((c) == SP || (c) == TAB)
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
# undef DLIST_FOR_NEXT_END
#endif
#ifdef DLIST_FOR_PREV_END
# undef DLIST_FOR_PREV_END
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
# undef DLIST_ND_FOR_PREV_END
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
#ifdef DLIST_SWAP_FIELD_NEXT
# undef DLIST_SWAP_FIELD_NEXT
#endif
#ifdef DLIST_SWAP_FIELD_PREV
# undef DLIST_SWAP_FIELD_PREV
#endif
#ifdef DLIST_ATOMIC_SWAP_FIELD_NEXT
# undef DLIST_ATOMIC_SWAP_FIELD_NEXT
#endif
#ifdef DLIST_ATOMIC_SWAP_FIELD_PREV
# undef DLIST_ATOMIC_SWAP_FIELD_PREV
#endif
#ifdef DLIST_SWAP_FIELD
# undef DLIST_SWAP_FIELD
#endif
#ifdef DLIST_SAFE_SWAP_FIELD
# undef DLIST_SAFE_SWAP_FIELD
#endif
#ifdef DLIST_ATOMIC_SWAP_FIELD
# undef DLIST_ATOMIC_SWAP_FIELD
#endif
#ifdef DLIST_SAFE_ATOMIC_SWAP_FIELD
# undef DLIST_SAFE_ATOMIC_SWAP_FIELD
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

#define DLIST_ATOMIC_ADV_NEXT(x)               \
  do {                                         \
    __TYPE(x) old  = ATOMIC_FETCH(x);          \
    __TYPE(x) next = ATOMIC_FETCH(old->next);  \
  } while (!ATOMIC_CAS(x, old, next));

#define DLIST_UNLINK(ptr)               \
  DO_WHILE(                             \
    if ((ptr)->prev) {                  \
      (ptr)->prev->next = (ptr)->next;  \
    }                                   \
    if ((ptr)->next) {                  \
      (ptr)->next->prev = (ptr)->prev;  \
    }                                   \
  )

#define DLIST_SWAP_FIELD_NEXT(ptr, field)         SWAP((ptr)->next->field, (ptr)->field)
#define DLIST_SWAP_FIELD_PREV(ptr, field)         SWAP((ptr)->prev->field, (ptr)->field)
#define DLIST_ATOMIC_SWAP_FIELD_NEXT(ptr, field)  ATOMIC_SWAP((ptr)->next->field, (ptr)->field)
#define DLIST_ATOMIC_SWAP_FIELD_PREV(ptr, field)  ATOMIC_SWAP((ptr)->prev->field, (ptr)->field)

#define DLIST_SWAP_FIELD(ptr, field, to_prev)        \
  /* Perform a `non-atomic` swap of `field` between  \
   * `ptr` and either `ptr`->next or `ptr->prev`.    \
   * Note that this does no bounds or validity       \
   * checking, see `DLIST_SAFE_SWAP_FIELD()`. */     \
  DO_WHILE(                                          \
    if (to_prev) {                                   \
      DLIST_SWAP_FIELD_PREV(ptr, field);             \
    }                                                \
    else {                                           \
      DLIST_SWAP_FIELD_NEXT(ptr, field);             \
    }                                                \
  )

#define DLIST_SAFE_SWAP_FIELD(ptr, field, to_prev)   \
  /* Perform a `non-atomic` swap of `field` between  \
   * `ptr` and either `ptr`->next or `ptr->prev`.    \
   * Note that this does full bounds and validity    \
   * checking, see `DLIST_SWAP_FIELD()`. */          \
  DO_WHILE(                                          \
    ASSERT(ptr);                                     \
    if ((to_prev) && (ptr)->prev) {                  \
      DLIST_SWAP_FIELD_PREV(ptr, field);             \
    }                                                \
    else if (!(to_prev) && (ptr)->next) {            \
      DLIST_SWAP_FIELD_NEXT(ptr, field);             \
    }                                                \
  )

#define DLIST_ATOMIC_SWAP_FIELD(ptr, field, to_prev)  \
  DO_WHILE(                                           \
    if (to_prev) {                                    \
      DLIST_ATOMIC_SWAP_FIELD_PREV(ptr, field);       \
    }                                                 \
    else {                                            \
      DLIST_ATOMIC_SWAP_FIELD_NEXT(ptr, field);       \
    }                                                 \
  )

#define DLIST_SAFE_ATOMIC_SWAP_FIELD(ptr, field, to_prev)  \
  DO_WHILE(                                                \
    ASSERT(ptr);                                           \
    if ((to_prev) && (ptr)->prev) {                        \
      DLIST_ATOMIC_SWAP_FIELD_PREV(ptr, field);            \
    }                                                      \
    else if (!(to_prev) && (ptr)->next) {                  \
      DLIST_ATOMIC_SWAP_FIELD_NEXT(ptr, field);            \
    }                                                      \
  )

/* ----------------------------- Single linked list helper's ----------------------------- */

#define LIST_ATOMIC_PREEND(x, head)          \
  DO_WHILE(                                  \
    do {                                     \
      x->next = ATOMIC_FETCH(head);          \
    } while (!ATOMIC_CAS(head, x->next, x));  \
  )

/* ----------------------------- Struct helper define's ----------------------------- */

#ifdef STRUCT_FIELD_PTR
# undef STRUCT_FIELD_PTR
#endif

#define STRUCT_FIELD_PTR(struct, field)  ((const char *)(struct) + offsetof(__TYPE(*struct), field))

/* ----------------------------- log.c ----------------------------- */

#define __fcio_log(type, ...)  fcio_log((type), __LINE__, __func__, __VA_ARGS__)

#define log_INFO_0(...)                         \
  /* Low prio info log. */                      \
  __fcio_log(0, __VA_ARGS__)

#define log_INFO_1(...)                         \
  /* Low prio info log. */                      \
  __fcio_log(1, __VA_ARGS__)

#define log_WARN_0(...)                         \
  /* Low prio warning log. */                   \
  __fcio_log(2, __VA_ARGS__)

#define log_ERR_NF(...)                         \
  /* Non-Fatal error-log. */                    \
  __fcio_log(3, __VA_ARGS__)

#define log_ERR_FA(...)                                   \
  /* FATAL error-log.  Note that this will terminate. */  \
  __fcio_log(4, __VA_ARGS__)


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
  Ulong namelen;      /* The length of the entry name. */
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

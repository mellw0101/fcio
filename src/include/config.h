/** @file config.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


#ifdef __WIN__
# undef __WIN__
#endif

#if (defined(_WIN32) || defined(_WIN64))
# define __WIN__ 1
# define uint64_t  unsigned long long int
#endif


#ifdef _BEGIN_C_LINKAGE
# undef _BEGIN_C_LINKAGE
#endif
#ifdef _END_C_LINKAGE
# undef _END_C_LINKAGE
#endif
#ifdef restrict
# undef restrict
#endif
#ifdef _THREAD
# undef _THREAD
#endif
#ifdef _GNUC_VER
# undef _GNUC_VER
#endif
#ifdef _HAS_ATTRIBUTE
# undef _HAS_ATTRIBUTE
#endif
#ifdef _ERROR
# undef _ERROR
#endif
#ifdef _WARNING
# undef _WARNING
#endif
#ifdef _NONNULL
# undef _NONNULL
#endif
#ifdef _NO_RETURN
# undef _NO_RETURN
#endif
#ifdef _PRINTFLIKE
# undef _PRINTFLIKE
#endif
#ifdef _CONST
# undef _CONST
#endif
#ifdef _FALLTHROUGH
# undef _FALLTHROUGH
#endif
#ifdef _RETURNS_NONNULL
# undef _RETURNS_NONNULL
#endif
#ifdef _NODISCARD
# undef _NODISCARD
#endif
#ifdef _UNUSED
# undef _UNUSED
#endif
#ifdef __ATOMIC_SWAP
# undef __ATOMIC_SWAP
#endif

#ifdef __cplusplus
# define _BEGIN_C_LINKAGE  extern "C" {
# define _END_C_LINKAGE    }
#else
# define _BEGIN_C_LINKAGE
# define _END_C_LINKAGE
#endif

/* Define restrict keywork when used in c++ code. */
#ifdef __cplusplus
# define restrict  __restrict  
#endif

#if defined(__GNUC__) || defined(__clang__)
# define _THREAD  __thread
#elif defined(_MSC_VER)
# define _THREAD  __declspec(thread)
#else
# define _THREAD
#endif

/* True if the compiler says it groks GNU C version MAJOR.MINOR.  */
#if (__GNUC__ && __GNUC_MINOR__)
# define _GNUC_VER(maj, min)  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define _GNUC_VER(maj, min)  0
#endif

/* Lets say you want to check if compatible with clang 10.0.  This will return true when using clang 10.0/(maj.min) or above. */
#if (defined(__clang__) && defined(__clang_major__) && defined(__clang_minor__))
# define _CLANG_VER(maj, min)  ((__clang_major__ > (maj)) || (__clang_major__ == (maj) && __clang_minor__ >= (min)))
#else
# define _CLANG_VER(maj, min)  0
#endif

#if (defined(__clang__) || _GNUC_VER(2, 5))
# define _HAVE_GNU_EXTENSION  1
#else
# define _HAVE_GNU_EXTENSION  0
#endif

/* Define _HAS_ATTRIBUTE only once, because on FreeBSD, with gcc < 5, if <config.h> gets
 * included once again after <sys/cdefs.h>, __has_attribute(x) expands to 0 always,
 * and redefining _HAS_ATTRIBUTE would turn off all attributes.  */
#if (defined __has_attribute && (!defined __clang_minor__ || (defined __apple_build_version__ ? 7000000 <= __apple_build_version__ : 5 <= __clang_major__)))
# define _HAS_ATTRIBUTE(attr)  __has_attribute(__##attr##__)
#else
# define _HAS_ATTRIBUTE(attr)      _ATTR_##attr
# define _ATTR_alloc_size          _GNUC_VER(4, 3)
# define _ATTR_always_inline       _GNUC_VER(3, 2)
# define _ATTR_artificial          _GNUC_VER(4, 3)
# define _ATTR_cold                _GNUC_VER(4, 3)
# define _ATTR_const               _GNUC_VER(2, 95)
# define _ATTR_deprecated          _GNUC_VER(3, 1)
# define _ATTR_diagnose_if         0
# define _ATTR_error               _GNUC_VER(4, 3)
# define _ATTR_externally_visible  _GNUC_VER(4, 1)
# define _ATTR_fallthrough         _GNUC_VER(7, 0)
# define _ATTR_format              _GNUC_VER(2, 7)
# define _ATTR_leaf                _GNUC_VER(4, 6)
# define _ATTR_malloc              _GNUC_VER(3, 0)
# ifdef _ICC
#   define _ATTR_may_alias  0
# else
#   define _ATTR_may_alias  _GNUC_VER(3, 3)
# endif
# define _ATTR_noinline            _GNUC_VER(3, 1)
# define _ATTR_nonnull             _GNUC_VER(3, 3)
# define _ATTR_nonstring           _GNUC_VER(8, 0)
# define _ATTR_nothrow             _GNUC_VER(3, 3)
# define _ATTR_packed              _GNUC_VER(2, 7)
# define _ATTR_pure                _GNUC_VER(2, 96)
# define _ATTR_returns_nonnull     _GNUC_VER(4, 9)
# define _ATTR_sentinel            _GNUC_VER(4, 0)
# define _ATTR_unused              _GNUC_VER(2, 7)
# define _ATTR_warn_unused_result  _GNUC_VER(3, 4)
#endif

/* Define _HAS_BUILTIN when the compiler has it.  Otherwise, will always return 0. */
#if (_GNUC_VER(3, 1) || _CLANG_VER(10, 0))
# define _HAS_BUILTIN(x)  __has_builtin(__builtin_##x)
#else
# define _HAS_BUILTIN(x)  0
#endif

/* Applies to: functions. */
#if _HAS_ATTRIBUTE(error)
# define _ERROR(msg)    __attribute__((__error__(msg)))
# define _WARNING(msg)  __attribute__((__warning__(msg)))
#elif _HAS_ATTRIBUTE(diagnose_if)
# define _ERROR(msg)    __attribute__((__diagnose_if__(1, msg, "error")))
# define _WARNING(msg)  __attribute__((__diagnose_if__(1, msg, "warning")))
#else
# define _ERROR(msg)
# define _WARNING(msg)
#endif

/* Define the macro `_NONNULL` to use `nonnull` in a safe manner. */
#if _HAS_ATTRIBUTE(nonnull)
# define _NONNULL(...)  __attribute__((__nonnull__(__VA_ARGS__)))
#else
# define _NONNULL(...)
#endif

/* Define the macro `_NO_RETURN` to use `noreturn` attribute in a safe mannor. */
#if _HAS_ATTRIBUTE(noreturn)
# define _NO_RETURN  __attribute__((__noreturn__))
#else
# define _NO_RETURN
#endif

/* Define a macro to tell the compiler to treat the parameters passed to `_PRINTFLIKE` like `format` and `...` in the `printf` std function. */
#if _HAS_ATTRIBUTE(format)
# define _PRINTFLIKE(...)  __attribute__((__format__(printf, __VA_ARGS__)))
#else
# define _PRINTFLIKE(...)
#endif

/* Define a macro for const attribute. */
#if _HAS_ATTRIBUTE(const)
# define _CONST  __attribute__((__const__))
#else
# define _CONST
#endif

/* Should be used in combanation with comp flag `-Wimplicit-fallthrough` to ensure safer code. */
#if _HAS_ATTRIBUTE(fallthrough)
# define _FALLTHROUGH  __attribute__((__fallthrough__))
#else
# define _FALLTHROUGH  ((void)0)
#endif

/* Tells the compiler that this function can never return a null ptr as in it
 * terminates when that happens or some other way of garantie.  This allows the
 * compiler to optimize in a better mannor as it understands the intent better. */
#if _HAS_ATTRIBUTE(returns_nonnull)
# define _RETURNS_NONNULL  __attribute__((__returns_nonnull__))
#else
# define _RETURNS_NONNULL
#endif

/* Define a macro for `warn_unused_result`. */
#if _HAS_ATTRIBUTE(warn_unused_result)
# define _NODISCARD  __attribute__((__warn_unused_result__))
#else
# define _NODISCARD
#endif

/* A helper macro.  Don't use it directly.  */
#if _HAS_ATTRIBUTE(unused)
# define _UNUSED __attribute__((__unused__))
#else
# define _UNUSED
#endif

#if _HAS_ATTRIBUTE(counted_by)
# define _COUNTED_BY(x)  __attribute__((__counted_by__(x)))
#else
# define _COUNTED_BY(x)
#endif

#if _HAS_BUILTIN(types_compatible_p)
# define TYPES_COMPATIBLE_P(x, y)  __builtin_types_compatible_p((x), (y))
#else
# define TYPES_COMPATIBLE_P(x, y)  (1)
#endif

#if _HAS_BUILTIN(prefetch)
# define PREFETCH(...)  __builtin_prefetch(__VA_ARGS__)
#else
# define PREFETCH(...)  ((void)0)
#endif

/* ----------------------------- String shorthands ----------------------------- */

#if _HAS_BUILTIN(strlen)
# define STRLEN(s)  __builtin_strlen(s)
#else
# define STRLEN(s)  strlen(s)
#endif

#if _HAS_BUILTIN(strcpy)
# define STRCPY(dst, src)  __builtin_strcpy(dst, src)
#else
# define STRCPY(dst, src)  strcpy(dst, src)
#endif

#if _HAS_BUILTIN(strcat)
# define STRCAT(dst, src)  __builtin_strcat(dst, src)
#else
# define STRCAT(dst, src)  strcat(dst, src)
#endif

#if _HAS_BUILTIN(strncpy)
# define STRNCPY(dst, src, n)  __builtin_strncpy(dst, src, n)
#else
# define STRNCPY(dst, src, n)  strncpy(dst, src, n)
#endif

#if _HAS_BUILTIN(strcmp)
# define STRCMP(dst, src)  __builtin_strcmp(dst, src)
#else
# define STRCMP(dst, src)  strcmp(dst, src)
#endif

#if _HAS_BUILTIN(strncmp)
# define STRNCMP(dst, src, n)  __builtin_strncmp(dst, src, n)
#else
# define STRNCMP(dst, src, n)  strncmp(dst, src, n)
#endif

#if _HAS_BUILTIN(strchr)
# define STRCHR(s, ch)  __builtin_strchr(s, ch)
#else
# define STRCHR(s, ch)  strchr(s, ch)
#endif

#if _HAS_BUILTIN(strrchr)
# define STRRCHR(s, ch)  __builtin_strrchr(s, ch)
#else
# define STRRCHR(s, ch)  strrchr(s, ch)
#endif

#if _HAS_BUILTIN(strcasecmp)
# define STRCASECMP(s0, s1)  __builtin_strcasecmp(s0, s1)
#else
# define STRCASECMP(s0, s1)  strcasecmp(s0, s1)
#endif

#if _HAS_BUILTIN(vsnprintf)
# define VSNPRINTF(buf, maxlen, format, va)  __builtin_vsnprintf(buf, maxlen, format, va)
#else
# define VSNPRINTF(buf, maxlen, format, va)  vsnprintf(buf, maxlen, format, va)
#endif

/* Memory builtin shorthands */

#if _HAS_BUILTIN(memset)
# define MEMSET(s, c, n)             \
    /* Set N bytes of S to C. */     \
    __builtin_memset((s), (c), (n))
#else
# define MEMSET(s, c, n)          \
    /* Set N bytes of S to C. */  \
    memset((s), (c), (n))
#endif


#if _HAS_BUILTIN(memcpy)
# define MEMCPY(dst, src, n)  __builtin_memcpy((dst), (src), (n))
#else
# define MEMCPY(dst, src, n)  memcpy((dst), (src), (n))
#endif

#if _HAS_BUILTIN(memmove)
# define MEMMOVE(dst, src, n)  __builtin_memmove((dst), (src), (n))
#else
# define MEMMOVE(dst, src, n)  memmove((dst), (src), (n))
#endif

#if _HAS_BUILTIN(memcmp)
# define MEMCMP(dst, src, n)  __builtin_memcmp((dst), (src), (n))
#else
# define MEMCMP(dst, src, n)  memcmp((dst), (src), (n))
#endif

#if !__WIN__

#if _HAS_BUILTIN(free)
# define FREE(x)  __builtin_free(x)
#else
# define FREE(x)  free(x)
#endif

/* Malloc */
#if _HAS_BUILTIN(malloc)
# define MALLOC(size)  __builtin_malloc(size)
#else
# define MALLOC(size)  malloc(size)
#endif

/* Realloc */
#if _HAS_BUILTIN(realloc)
# define REALLOC(ptr, size)  __builtin_realloc(ptr, size)
#else
# define REALLOC(ptr, size)  realloc(ptr, size)
#endif

/* Calloc */
#if _HAS_BUILTIN(calloc)
# define CALLOC(n, nsize)  __builtin_calloc(n, nsize)
#else
# define CALLOC(n, nsize)  calloc(n, nsize)
#endif

#else
#define MALLOC(howmush)        HeapAlloc(GetProcessHeap(), 0, (howmush))
#define REALLOC(ptr, newsize)  HeapReAlloc(GetProcessHeap(), 0, (ptr), (newsize))
#define CALLOC(n, size)        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ((n) * (size)))
#define FREE(p)                HeapFree(GetProcessHeap(), 0, (p))
#endif

#if _HAS_BUILTIN(constant_p)
# define CONSTANT_P(x)  __builtin_constant_p((x))
#else
# define CONSTANT_P(x)  0
#endif

#if _HAS_BUILTIN(expect)
# define EXPECT(x, result)  __builtin_expect((x), (result))
#else
# define EXPECT(x, result)  (x)
#endif

#if _HAS_BUILTIN(classify_type)
# define CLASSIFY_TYPE(x)  __builtin_classify_type(x)
#else
# define CLASSIFY_TYPE(x)
#endif

/* The built-in function __builtin_counted_by_ref checks whether the array object pointed to by `x`
 * has another object associated with it that reprecents the number of elements in the array. */
#if _HAS_BUILTIN(counted_by_ref)
# define COUNTED_BY_REF(x)  __builtin_counted_by_ref(x)
#else
# define COUNTED_BY_REF(x)  ((void *)0)
#endif

#if _HAS_BUILTIN(alloca)
# define ALLOCA(size)  __builtin_alloca(size)
#else
# define ALLOCA(size)  ((void *)0)  
#endif

#if _HAS_BUILTIN(alloca_with_align)
# define ALLOCA_WITH_ALIGN(n, align)  __builtin_alloca_with_align(n, align)
#else
# define ALLOCA_WITH_ALIGN(n, align)  ((void *)0)
#endif

#if _HAS_BUILTIN(offsetof)
# define OFFSETOF(type, field)  __builtin_offsetof(type, field)
#else
# define OFFSETOF(type, field)  (0)
#endif

#if _HAS_BUILTIN(readcyclecounter)
# define READCYCLECOUNTER()  __builtin_readcyclecounter()
#else
# define READCYCLECOUNTER()  (0ULL)
#endif

#if _HAS_BUILTIN(readsteadycounter)
# define READSTEADYCOUNTER()  __builtin_readsteadycounter()
#else
# define READSTEADYCOUNTER()  (0ULL)
#endif

#if _HAS_BUILTIN(cpu_supports)
# define CPU_SUPPORTS(x)  __builtin_cpu_supports(#x)
#else
# define CPU_SUPPORTS(x)  (0)
#endif

#if _HAS_BUILTIN(dump_struct)
# define DUMP_STRUCT(...)  __builtin_dump_struct(__VA_ARGS__)
#else 
# define DUMP_STRUCT(...)  ((void)0)
#endif


/* ---------------------------------------------------------- Math builtins ---------------------------------------------------------- */


#if _HAS_BUILTIN(atan2f)
# define ATAN2F(x, y)  __builtin_atan2f(x, y)
#else
# define ATAN2F(x, y)  atan2f(x, y)
#endif

#if _HAS_BUILTIN(powf)
# define POWF(x, y)  __builtin_powf(x, y)
#else
# define POWF(x, y)  powf(x, y)
#endif

/* ----------------------------- Rounding ----------------------------- */

#if _HAS_BUILTIN(round)
# define ROUND(x)  __builtin_round(x)
#else
# define ROUND(x)  round(x)
#endif

#if _HAS_BUILTIN(roundf)
# define ROUNDF(x)  __builtin_roundf(x)
#else
# define ROUNDF(x)  roundf(x)
#endif

#if _HAS_BUILTIN(lround)
# define LROUND(x)  __builtin_lround(x)
#else
# define LROUND(x)  lround(x)
#endif

#if _HAS_BUILTIN(llround)
# define LLROUND(x)  __builtin_llround(x)
#else
# define LLROUND(x)  llround(x)
#endif

#if _HAS_BUILTIN(fmaxf)
# define FMAXF(x, y)  __builtin_fmaxf(x, y)
#else
# define FMAXF(x, y)  fmaxf(x, y)
#endif

#if _HAS_BUILTIN(fminf)
# define FMINF(x, y)  __builtin_fminf(x, y)
#else
# define FMINF(x, y)  fminf(x, y)
#endif

#if _HAS_BUILTIN(sqrtf)
# define SQRTF(x)  __builtin_sqrtf(x)
#else
# define SQRTF(x)  sqrtf(x)
#endif

#if _HAS_BUILTIN(cosf)
# define COSF(x)  __builtin_cosf(x)
#else
# define COSF(x)  cosf(x)
#endif

#if _HAS_BUILTIN(sinf)
# define SINF(x)  __builtin_sinf(x)
#else
# define SINF(x)  sinf(x)
#endif

#if _HAS_BUILTIN(tanf)
# define TANF(x)  __builtin_tanf(x)
#else
# define TANF(x)  tanf(x)
#endif

/* ----------------------------- Absolute values ----------------------------- */

#if _HAS_BUILTIN(fabsf)
# define FABSF(x)  __builtin_fabsf(x)
#else
# define FABSF(x)  fabsf(x)
#endif

#if _HAS_BUILTIN(labs)
# define LABS(x)  __builtin_labs(x)
#else
# define LABS(x)  labs(x)
#endif

#if _HAS_BUILTIN(llabs)
# define LLABS(x)  __builtin_llabs(x)
#else
# define LLABS(x)  llabs(x)
#endif

#if _HAS_BUILTIN(hypotf)
# define HYPOTF(x, y)  __builtin_hypotf(x, y)
#else
# define HYPOTF(x, y)  hypotf(x, y)
#endif


/* Unless the user declares NO_ATOMIC_OPERATIONS, make some handy helpers. */
#ifndef NO_ATOMIC_OPERATIONS
  /* Define an atomic swap that should be compatible on almost all hardware. */
# if (_GNUC_VER(4, 7) || (defined __has_builtin && __has_builtin(__atomic_exchange_n)))
#   define __ATOMIC_SWAP(x, y)  __atomic_exchange_n(x, y, __ATOMIC_SEQ_CST)
# elif (defined __has_builtin && __has_builtin(__sync_swap))
#   define __ATOMIC_SWAP(x, y)  __sync_swap(x, y)
# elif (_GNUC_VER(4, 1) || (defined __has_builtin && __has_builtin(__sync_lock_test_and_set)))
#   define __ATOMIC_SWAP(x, y)  __sync_lock_test_and_set(x, y)
# elif _HAS_ATTRIBUTE(error)
    static void *__ATOMIC_SWAP(void *x, void *y) _ERROR("Compiler does not support atomic swap");
# else
#   error "No atomic swap available"
# endif
  /* An atomic setting operation. */
# if ((defined __has_builtin && __has_builtin(__atomic_store_n)) || _GNUC_VER(4, 7))
#   define __ATOMIC_STORE(x, y)  __atomic_store_n(x, y, __ATOMIC_SEQ_CST)
# elif ((defined __has_builtin && __has_builtin(__sync_lock_test_and_set)) || _GNUC_VER(4, 1))
#   define __ATOMIC_STORE(x, y)  ((void)__sync_lock_test_and_set(x, y))
# else
#   error "No atomic store available"
# endif
  /* An atomic fetch operation (load with full barrier). */
# if ((defined __has_builtin && __has_builtin(__atomic_load_n)) || _GNUC_VER(4, 7))
#   define __ATOMIC_FETCH(x)  __atomic_load_n(x, __ATOMIC_SEQ_CST)
# elif ((defined __has_builtin && __has_builtin(__sync_fetch_and_or)) || _GNUC_VER(4, 1))
  /* Read-only fetch using fetch-and-OR with zero */
#   define __ATOMIC_FETCH(x)  __sync_fetch_and_or(x, 0)
# elif _HAS_ATTRIBUTE(error)
    static void *__ATOMIC_FETCH(void *x) _ERROR("Compiler does not support atomic fetch");
# else
#   error "No atomic fetch available"
# endif
  /* An atomic compare-and-swap (CAS) operation. */
# if ((defined __has_builtin && __has_builtin(__atomic_compare_exchange_n)) || _GNUC_VER(4, 7))
#   define __ATOMIC_CAS(ptr, expected, desired) \
    __atomic_compare_exchange_n(ptr, expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
# elif _HAS_ATTRIBUTE(error)
    static int __ATOMIC_CAS(void *ptr, void *expected, void *desired) _ERROR("Compiler does not support atomic compare-and-swap");
# else
#   error "No atomic compare-and-swap available"
# endif
#endif

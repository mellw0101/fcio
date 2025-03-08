/** @file config.h

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#pragma once


#ifdef _BEGIN_C_LINKAGE
# undef _BEGIN_C_LINKAGE
#endif
#ifdef _END_C_LINKAGE
# undef _END_C_LINKAGE
#endif
#ifdef restrict
# undef restrict
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

/* True if the compiler says it groks GNU C version MAJOR.MINOR.  */
#if (__GNUC__ && __GNUC_MINOR__)
# define _GNUC_VER(maj, min)  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define _GNUC_VER(maj, min)  0
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

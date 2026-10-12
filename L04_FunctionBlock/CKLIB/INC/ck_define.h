#ifndef CKLIB_CK_DEFINE_H_
#define CKLIB_CK_DEFINE_H_

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc99-extensions"
#endif  /* __clang__ */

/* NOLINTNEXTLINE */
#define MSC_VER_1998_VC60    1200
/* NOLINTNEXTLINE */
#define MSC_VER_2002_VC70    1300
/* NOLINTNEXTLINE */
#define MSC_VER_2003_VC71    1310
/* NOLINTNEXTLINE */
#define MSC_VER_2005_VC80    1400
/* NOLINTNEXTLINE */
#define MSC_VER_2008_VC90    1500
/* NOLINTNEXTLINE */
#define MSC_VER_2010_VC100   1600
/* NOLINTNEXTLINE */
#define MSC_VER_2012_VC110   1700
/* NOLINTNEXTLINE */
#define MSC_VER_2013_VC120   1800
/* NOLINTNEXTLINE */
#define MSC_VER_2015_VC140   1900

#include <stddef.h>  /* NOLINT */

#if (defined _WIN32 && _MSC_VER <= MSC_VER_1998_VC60 && !defined inline)
/* NOLINTNEXTLINE */
#define inline __inline
#endif

#if ((defined __linux__) || (defined __ARM_EABI__)) &&\
     !defined __GNUC_STDC_INLINE__
/* NOLINTNEXTLINE */
#define inline __inline__
#endif

#if (defined __linux__) || (defined __ARM_EABI__) ||\
    (defined _WIN32 && _MSC_VER >= MSC_VER_2010_VC100)

#include <stdbool.h>  /* NOLINT */
#include <stdint.h>   /* NOLINT */

#else

#ifndef __cplusplus
typedef char bool;
#define true  1
#define false 0
#endif

typedef signed char        int8_t;    /* NOLINT */
typedef short              int16_t;   /* NOLINT */
typedef int                int32_t;   /* NOLINT */
typedef unsigned char      uint8_t;   /* NOLINT */
typedef unsigned short     uint16_t;  /* NOLINT */
typedef unsigned int       uint32_t;  /* NOLINT */

typedef signed char        int_least8_t;    /* NOLINT */
typedef short              int_least16_t;   /* NOLINT */
typedef int                int_least32_t;   /* NOLINT */
typedef unsigned char      uint_least8_t;   /* NOLINT */
typedef unsigned short     uint_least16_t;  /* NOLINT */
typedef unsigned int       uint_least32_t;  /* NOLINT */

typedef signed char        int_fast8_t;    /* NOLINT */
typedef int                int_fast16_t;   /* NOLINT */
typedef int                int_fast32_t;   /* NOLINT */
typedef unsigned char      uint_fast8_t;   /* NOLINT */
typedef unsigned int       uint_fast16_t;  /* NOLINT */
typedef unsigned int       uint_fast32_t;  /* NOLINT */

#if _MSC_VER >= MSC_VER_2005_VC80

typedef long long          int64_t;         /* NOLINT */
typedef unsigned long long uint64_t;        /* NOLINT */
typedef long long          int_least64_t;   /* NOLINT */
typedef unsigned long long uint_least64_t;  /* NOLINT */
typedef long long          int_fast64_t;    /* NOLINT */
typedef unsigned long long uint_fast64_t;   /* NOLINT */
typedef long long          intmax_t;        /* NOLINT */
typedef unsigned long long uintmax_t;       /* NOLINT */

#else

#define INT32_MIN        (-2147483647 - 1)

typedef __int64            int64_t;   /* NOLINT */
typedef unsigned __int64   uint64_t;  /* NOLINT */

#endif

#endif

#ifndef __cplusplus
#   define nullptr NULL
#endif

#ifdef _WIN32
#   define CK_CALLTYPE __cdecl
#   ifdef CKLIB_EXPORT
#       define CKLIB_EXPORT_IMPORT __declspec(dllexport)
#   else
#       define CKLIB_EXPORT_IMPORT __declspec(dllimport)
#   endif
#elif (defined __linux__) || (defined __ARM_EABI__)
#   if defined (__i386__)
#       define CK_CALLTYPE __attribute__((cdecl))
#   else
#       define CK_CALLTYPE  /* only x86 platform can specify calltype */
#   endif
#   define CKLIB_EXPORT_IMPORT
#endif

/**
 * @brief Standard API declaration.
 * @details
 */
/* NOLINTNEXTLINE */
#define CK_C_API CKLIB_EXPORT_IMPORT CK_CALLTYPE

#endif  /* CKLIB_CK_DEFINE_H_ */

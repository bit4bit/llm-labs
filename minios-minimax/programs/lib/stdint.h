#ifndef STDINT_H
#define STDINT_H

/* Exact-width integer types */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

/* Pointer-sized integer types */
typedef int32_t  intptr_t;
typedef uint32_t uintptr_t;

/* Size type */
typedef uint32_t size_t;

/* Limits for exact-width integer types */
#define INT8_MIN   (-128)
#define INT8_MAX   (127)
#define UINT8_MAX  (255)

#define INT16_MIN  (-32768)
#define INT16_MAX  (32767)
#define UINT16_MAX (65535)

#define INT32_MIN  (-2147483648)
#define INT32_MAX  (2147483647)
#define UINT32_MAX (4294967295U)

#define INT64_MIN  (-9223372036854775808LL)
#define INT64_MAX  (9223372036854775807LL)
#define UINT64_MAX (18446744073709551615ULL)

/* NULL pointer */
#define NULL ((void*)0)

#endif /* STDINT_H */
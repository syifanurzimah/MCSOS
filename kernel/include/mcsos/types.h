#ifndef MCSOS_TYPES_H
#define MCSOS_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* Integer types */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

/* Kernel size / address types */
typedef uintptr_t kaddr_t;
typedef size_t     ksize_t;

/* Boolean */
typedef enum {
    false = 0,
    true  = 1
} bool_t;

typedef int bool;

#endif

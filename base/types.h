#if !defined(BASE_TYPES_H_)
#define BASE_TYPES_H_

#include <stdint.h>

// Basic types
//
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int8_t  b8;
typedef int16_t b16;
typedef int32_t b32;
typedef int64_t b64;

typedef uintptr_t uptr;
typedef intptr_t  sptr;

typedef float  f32;
typedef double f64;

struct str8 {
    uptr count;
    u8 *data;
};

#endif  // BASE_TYPES_H_

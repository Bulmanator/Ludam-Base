#if !defined(BASE_TYPES_H_)
#define BASE_TYPES_H_

#include <stdint.h>

// Compiler detection
//
#define COMPILER_CLANG 0
#define COMPILER_CL    0
#define COMPILER_GCC   0

#if defined(__clang__)
#    undef  COMPILER_CLANG
#    define COMPILER_CLANG 1
#elif defined(_MSC_VER)
#    undef  COMPILER_CL
#    define COMPILER_CL 1
#elif defined(__GNUC__)
#    undef  COMPILER_GCC
#    define COMPILER_GCC 1
#else
#    error "Failed to detect compiler. Please use cl.exe, clang or gcc"
#endif

// Architecture detection
//
#define ARCH_AMD64   0
#define ARCH_AARCH64 0

#if defined(__amd64) || defined(__amd64__) || defined(_M_AMD64)
#   undef  ARCH_AMD64
#   define ARCH_AMD64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#   undef  ARCH_AARCH64
#   define ARCH_AARCH64 1
#else
#    error "Failed to detect architecture. Only amd64 and aarch64 are supported"
#endif

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

// Maths types
//
union v2u {
    struct {
        u32 x, y;
    };

    struct {
        u32 w, h;
    };

    u32 e[2];
};

union v2s {
    struct {
        s32 x, y;
    };

    struct {
        s32 w, h;
    };

    s32 e[2];
};

union v2 {
    struct {
        f32 x, y;
    };

    struct {
        f32 u, v;
    };

    struct {
        f32 w, h;
    };

    f32 e[2];
};

union v3 {
    struct {
        f32 x, y, z;
    };

    struct {
        f32 r, g, b;
    };

    struct {
        f32 w, h, d;
    };

    struct {
        v2  xy;
        f32 _z;
    };

    f32 e[3];
};

union v4 {
    struct {
        f32 x, y, z;
        f32 w;
    };

    struct {
        f32 r, g, b;
        f32 a;
    };

    struct {
        v3  xyz;
        f32 _w;
    };

    f32 e[4];
};

union m4x4 {
    struct {
        f32 m[4][4];
    };

    struct {
        v4 r[4];
    };

    f32 e[16];
};

// Utility macros
//
#define function static
#define global   static
#define local    static

#define cast(x) (x)
#define ArraySize(x) (sizeof(x) / sizeof((x)[0]))
#define OffsetTo(T, m) ((uptr) &(((T *) 0)->m))
#define Swap(a, b) do { auto __temp = a; a = b; b = __temp; } while (0)
#define AlignTo(addr, align) (((addr) + ((align) - 1)) & ~((align) - 1))

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

#endif  // BASE_TYPES_H_

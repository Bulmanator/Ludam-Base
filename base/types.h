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

struct m4x4_inv {
    m4x4 forward;
    m4x4 inverse;
};

struct rect2 {
    v2 min;
    v2 max;
};

struct rect3 {
    v3 min;
    v3 max;
};

// Vertex types
//
struct vert3 {
    // @Todo: In the future if we actually move to full 3D we will want at least a normal on here
    //
    v3  p;
    v2  uv;
    u32 c;
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

#define Clamp(x, min, max) (Min(Max(x, min), max))
#define Clamp01(x) Clamp(x, 0, 1)

// Type limits
//
#define U8_MAX  (u8)  (0xFF)
#define U16_MAX (u16) (0xFFFF)
#define U32_MAX (u32) (0xFFFF'FFFF)
#define U64_MAX (u64) (0xFFFF'FFFF'FFFF'FFFF)

#define S8_MAX  (s8)  (0x7F)
#define S16_MAX (s16) (0x7FFF)
#define S32_MAX (s32) (0x7FFF'FFFF)
#define S64_MAX (s64) (0x7FFF'FFFF'FFFF'FFFF)

#define S8_MIN  (s8)  (0x80)
#define S16_MIN (s16) (0x8000)
#define S32_MIN (s32) (0x8000'0000)
#define S64_MIN (s64) (0x8000'0000'0000'0000)

#define F32_MAX (f32) (FLT_MAX)
#define F64_MAX (f64) (DBL_MAX)

#define F32_MIN (f32) (FLT_MIN)
#define F64_MIN (f32) (DBL_MIN)

#endif  // BASE_TYPES_H_

#if !defined(BASE_MATHS_H_)
#define BASE_MATHS_H_

#include <math.h>

// Random number generator
//
struct Random {
    u64 state;
};

// Utility macros
//
#define Pi32  (3.141592653589793238462643383279502884197169399f)
#define Tau32 (6.283185307179586476925286766559005768394338799f)

#define Radians(x) ((x) * (180.0f / Pi32))
#define Degrees(x) ((x) * (Pi32 / 180.0f))

#define Abs(x) ((x) < 0 ? -(x) : (x))

// Scalar functions
//
function f32 Sqrt(f32 x);
function f32 InvSqrt(f32 x);

function f32 Sin(f32 x);
function f32 Cos(f32 x);
function f32 Tan(f32 x);

function f32 Lerp(f32 a, f32 b, f32 t);
function f64 Lerp(f64 a, f64 b, f64 t);

function f32 Sign(f32 x);

 // Default is positive, if you want negative epsilon just pass a negative value here instead
 //
function f32 Epsilon32(f32 value = 1.0f);
function f64 Epsilon64(f32 value = 1.0);

function b32 IsZero(f32 x);
function b32 IsZero(f64 x);

// Random functions
//
function Random RandomSeed(u64 seed);

function f32 RandomUnilateral(Random *r);
function f32 RandomBilateral(Random *r);

function f32 RandomF32(Random *r, f32 min, f32 max);
function f64 RandomF64(Random *r, f64 min, f64 max);

function s32 RandomS32(Random *r, s32 min, s32 max);
function s64 RandomS64(Random *r, s64 min, s64 max);

function u32 RandomU32(Random *r, u32 min, u32 max);
function u64 RandomU64(Random *r, u64 min, u64 max);

// Other utlities
//
function u32 ABGRPack(v4 colour);

// v2u operator overloads
//
// [v2u + v2u] [v2u += v2u]
// [v2u - v2u] [v2u -= v2u]
// [v2u * v2u] [v2u  * u32] [u32  * v2u] [v2u *= v2u] [v2u *= u32]
// [v2u / v2u] [v2u  / u32] [v2u /= v2u] [v2u /= u32]
//
// v2u functions
//
function v2u V2U(u32 x, u32 y);
function v2u V2U(v2s v);
function v2u V2U(v2  v);

function v2u Minimum(v2u a, v2u b);
function v2u Maximum(v2u a, v2u b);

// v2s operator overloads
//
// [v2s + v2s] [v2s += v2s]
// [v2s - v2s] [v2s -= v2s] [-v2s]
// [v2s * v2s] [v2s  * s32] [s32  * v2s] [v2s *= v2s] [v2s *= s32]
// [v2s / v2s] [v2s  / s32] [v2s /= v2s] [v2s /= s32]
//
// v2s functions
//
function v2s V2S(s32 x, s32 y);
function v2s V2S(v2u v);
function v2s V2S(v2  v);

function v2s Minimum(v2s a, v2s b);
function v2s Maximum(v2s a, v2s b);

// v2 operator overloads
//
// [v2 + v2] [v2 += v2]
// [v2 - v2] [v2 -= v2]  [-v2]
// [v2 * v2] [v2  * f32] [f32  * v2] [v2 *= v2]  [v2 *= f32]
// [v2 / v2] [v2  / f32] [v2  /= v2] [v2 /= f32]
//
// v2 functions
//
function v2 V2(f32 x, f32 y);
function v2 V2(v2u v);
function v2 V2(v2s v);

function f32 Dot(v2 a, v2 b);
function f32 Length(v2 a);
function v2  Noz(v2 a); // Normalise vector or zero vector if length = 0
function v2  Perp(v2 a);
function v2  Lerp(v2 a, v2 b, f32 t);

// Rotation stuff
//
function v2 Arm2(f32 sin, f32 cos);
function v2 Arm2(f32 angle);
function v2 Rotate(v2 v, v2 a);
function v2 Rotate(v2 v, f32 a);

function v2  Minimum(v2 a, v2 b);
function v2  Maximum(v2 a, v2 b);

// v3 operator overloads
//
// [v3 + v3] [v3 += v3]
// [v3 - v3] [v3 -= v3]  [-v3]
// [v3 * v3] [v3  * f32] [f32  * v3] [v3 *= v3]  [v3 *= f32]
// [v3 / v3] [v3  / f32] [v3  /= v3] [v3 /= f32]
//
// v3 functions
//
function v3 V3(f32 x, f32 y, f32 z);
function v3 V3(v2 xy, f32 z = 0.0f);

function f32 Dot(v3 a, v3 b);
function f32 Length(v3 a);
function v3  Noz(v3 a); // Normalise vector or zero vector if length = 0
function v3  Cross(v3 a, v3 b);
function v3  Lerp(v3 a, v3 b, f32 t);
function v3  Minimum(v3 a, v3 b);
function v3  Maximum(v3 a, v3 b);

// v4 operator overloads
//
// [v4 + v4] [v4 += v4]
// [v4 - v4] [v4 -= v4]  [-v4]
// [v4 * v4] [v4  * f32] [f32  * v4] [v4 *= v4]  [v4 *= f32]
// [v4 / v4] [v4  / f32] [v4  /= v4] [v4 /= f32]
//
// v4 functions
//
function v4 V4(f32 x, f32 y, f32 z, f32 w);
function v4 V4(v3 xyz, f32 w = 0.0f);

function f32 Dot(v4 a, v4 b);
function f32 Length(v4 a);
function v4  Noz(v4 a); // Normalise vector or zero vector if length = 0
function v4  Lerp(v4 a, v4 b, f32 t);
function v4  Minimum(v4 a, v4 b);
function v4  Maximum(v4 a, v4 b);

// m4x4 operator overloads
//
// [m4x4 * m4x4] [m4x4 * v4] [m4x4 * v3]
//
// m4x4 functions
//
function m4x4 Identity();

function v4 Transform(m4x4 a, v4 b);
function m4x4 Translate(m4x4 a, v3 b);

function m4x4 XRotation(f32 angle);
function m4x4 YRotation(f32 angle);
function m4x4 ZRotation(f32 angle);

function v3 GetRow(m4x4 a, u32 row);
function v3 GetColumn(m4x4 a, u32 col);

function m4x4 Rows3x3(v3 x, v3 y, v3 z);

function m4x4 Columns3x3(v3 x, v3 y, v3 z);

function m4x4_inv OrthographicProjection(f32 aspect, f32 near_plane, f32 far_plane);
function m4x4_inv PerspectiveProjection(f32 fov, f32 aspect, f32 near_plane, f32 far_plane);
function m4x4_inv CameraTransform(v3 x, v3 y, v3 z, v3 p);

#endif  // BASE_MATHS_H_

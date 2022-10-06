// @Note: Please refer to either base\intrin.cpp or base\neon.cpp for the implementations of Sqrt and
// InvSqrt as they are implemented using SIMD intrinsics for either supported CPU architecture
//
// @Todo(James): Support NEON intrinsics
//

function f32 Sin(f32 x) {
    f32 result = sinf(x);
    return result;
}

function f32 Cos(f32 x) {
    f32 result = cosf(x);
    return result;
}

function f32 Tan(f32 x) {
    f32 result = tanf(x);
    return result;
}

function f32 Lerp(f32 a, f32 b, f32 t) {
    f32 result = ((1.0f - t) * a) + (t * b);
    return result;
}

function f64 Lerp(f64 a, f64 b, f64 t) {
    f64 result = ((1.0 - t) * a) + (t * b);
    return result;
}

function f32 Sign(f32 x) {
    f32 result = (x < 0) ? -1.0f : 1.0f;
    return result;
}

function f32 Epsilon32(f32 value) {
    f32 result;

    union eps {
        f32 f;
        u32 u;
    };

    eps n;
    n.f  = value;
    n.u += 1;

    result = (n.f - value);
    return result;
}

function f64 Epsilon64(f64 value) {
    f64 result;

    union eps {
        f64 f;
        u64 u;
    };

    eps n;
    n.f  = value;
    n.u += 1;

    result = (n.f - value);
    return result;
}

function b32 IsZero(f32 x) {
    b32 result = (x >= Epsilon32(-1.0f)) && (x <= Epsilon32(1.0f));
    return result;
}

function b32 IsZero(f64 x) {
    b32 result = (x >= Epsilon64(-1.0)) && (x <= Epsilon64(1.0));
    return result;
}

// Random functions
//
function Random RandomSeed(u64 seed) {
    Random result = {};
    result.state  = seed;

    return result;
}

function u64 NextRandom(Random *r) {
    u64 result = r->state;
    result ^= (result << 13);
    result ^= (result >>  7);
    result ^= (result << 17);

    r->state = result;
    return result;
}

function f32 RandomUnilateral(Random *r) {
    f32 result = NextRandom(r) / cast(f32) U64_MAX;
    return result;
}

function f32 RandomBilateral(Random *r) {
    f32 result = -1.0f + (2.0f * RandomUnilateral(r));
    return result;
}

function f32 RandomF32(Random *r, f32 min, f32 max) {
    f32 result = Lerp(min, max, RandomUnilateral(r));
    return result;
}

function f64 RandomF64(Random *r, f64 min, f64 max) {
    f64 result = Lerp(min, max, cast(f64) RandomUnilateral(r));
    return result;
}

function s32 RandomS32(Random *r, s32 min, s32 max) {
    s32 result = cast(s32) Lerp(cast(f32) min, cast(f32) max, RandomUnilateral(r));
    return result;
}

function s64 RandomS64(Random *r, s64 min, s64 max) {
    s64 result = cast(s64) Lerp(cast(f64) min, cast(f64) max, cast(f64) RandomUnilateral(r));
    return result;
}

function u32 RandomU32(Random *r, u32 min, u32 max) {
    u32 result = cast(u32) Lerp(cast(f32) min, cast(f32) max, RandomUnilateral(r));
    return result;
}

function u64 RandomU64(Random *r, u64 min, u64 max) {
    u64 result = cast(u64) Lerp(cast(f64) min, cast(f64) max, cast(f64) RandomUnilateral(r));
    return result;
}

function u32 ABGRPack(v4 colour) {
    u32 result =
        ((cast(u8) (255.0f * colour.a)) << 24) |
        ((cast(u8) (255.0f * colour.b)) << 16) |
        ((cast(u8) (255.0f * colour.g)) <<  8) |
        ((cast(u8) (255.0f * colour.r)) <<  0);

    return result;
}

// v2u operator overloads
//
function v2u operator+(v2u a, v2u b) {
    v2u result;
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);

    return result;
}

function v2u &operator+=(v2u &a, v2u b) {
    a = a + b;
    return a;
}

function v2u operator-(v2u a, v2u b) {
    v2u result;
    result.x = (a.x - b.x);
    result.y = (a.y - b.y);

    return result;
}

function v2u &operator-=(v2u &a, v2u b) {
    a = a - b;
    return a;
}

function v2u operator*(v2u a, v2u b) {
    v2u result;
    result.x = (a.x * b.x);
    result.y = (a.y * b.y);

    return result;
}

function v2u operator*(v2u a, u32 b) {
    v2u result;
    result.x = (a.x * b);
    result.y = (a.y * b);

    return result;
}

function v2u operator*(u32 a, v2u b) {
    v2u result;
    result.x = (a * b.x);
    result.y = (a * b.y);

    return result;
}

function v2u &operator*=(v2u &a, v2u b) {
    a = a * b;
    return a;
}

function v2u &operator*=(v2u &a, u32 b) {
    a = a * b;
    return a;
}

function v2u operator/(v2u a, v2u b) {
    v2u result;
    result.x = (a.x / b.x);
    result.y = (a.y / b.y);

    return result;
}

function v2u operator/(v2u a, u32 b) {
    v2u result;
    result.x = (a.x / b);
    result.y = (a.y / b);

    return result;
}

function v2u &operator/=(v2u &a, v2u b) {
    a = a / b;
    return a;
}

function v2u &operator/=(v2u &a, u32 b) {
    a = a / b;
    return a;
}

// v2u functions
//
function v2u V2U(u32 x, u32 y) {
    v2u result = { x, y };
    return result;
}

function v2u V2U(v2s v) {
    v2u result = { cast(u32) v.x, cast(u32) v.y };
    return result;
}

function v2u V2U(v2 v) {
    v2u result = { cast(u32) v.x, cast(u32) v.y };
    return result;
}

function v2u Minimum(v2u a, v2u b) {
    v2u result;
    result.x = Min(a.x, b.x);
    result.y = Min(a.y, b.y);

    return result;
}

function v2u Maximum(v2u a, v2u b) {
    v2u result;
    result.x = Max(a.x, b.x);
    result.y = Max(a.y, b.y);

    return result;
}

// v2s operator overloads
//
function v2s operator+(v2s a, v2s b) {
    v2s result;
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);

    return result;
}

function v2s &operator+=(v2s &a, v2s b) {
    a = a + b;
    return a;
}

function v2s operator-(v2s a, v2s b) {
    v2s result;
    result.x = (a.x - b.x);
    result.y = (a.y - b.y);

    return result;
}

function v2s &operator-=(v2s &a, v2s b) {
    a = a - b;
    return a;
}

function v2s operator-(v2s a) {
    v2s result;
    result.x = -a.x;
    result.y = -a.y;

    return result;
}

function v2s operator*(v2s a, v2s b) {
    v2s result;
    result.x = (a.x * b.x);
    result.y = (a.y * b.y);

    return result;
}

function v2s operator*(v2s a, s32 b) {
    v2s result;
    result.x = (a.x * b);
    result.y = (a.y * b);

    return result;
}

function v2s operator*(s32 a, v2s b) {
    v2s result;
    result.x = (a * b.x);
    result.y = (a * b.y);

    return result;
}

function v2s &operator*=(v2s &a, v2s b) {
    a = a * b;
    return a;
}

function v2s &operator*=(v2s &a, s32 b) {
    a = a * b;
    return a;
}

function v2s operator/(v2s a, v2s b) {
    v2s result;
    result.x = (a.x / b.x);
    result.y = (a.y / b.y);

    return result;
}

function v2s operator/(v2s a, s32 b) {
    v2s result;
    result.x = (a.x / b);
    result.y = (a.y / b);

    return result;
}

function v2s &operator/=(v2s &a, v2s b) {
    a = a / b;
    return a;
}

function v2s &operator/=(v2s &a, s32 b) {
    a = a / b;
    return a;
}

// v2s functions
//
function v2s V2S(s32 x, s32 y) {
    v2s result = { x, y };
    return result;
}

function v2s V2S(v2u v) {
    v2s result = { cast(s32) v.x, cast(s32) v.y };
    return result;
}

function v2s V2S(v2 v) {
    v2s result = { cast(s32) v.x, cast(s32) v.y };
    return result;
}

function v2s Minimum(v2s a, v2s b) {
    v2s result;
    result.x = Min(a.x, b.x);
    result.y = Min(a.y, b.y);

    return result;
}

function v2s Maximum(v2s a, v2s b) {
    v2s result;
    result.x = Max(a.x, b.x);
    result.y = Max(a.y, b.y);

    return result;
}

// v2 operator overloads
//
function v2 operator+(v2 a, v2 b) {
    v2 result;
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);

    return result;
}

function v2 &operator+=(v2 &a, v2 b) {
    a = a + b;
    return a;
}

function v2 operator-(v2 a, v2 b) {
    v2 result;
    result.x = (a.x - b.x);
    result.y = (a.y - b.y);

    return result;
}

function v2 &operator-=(v2 &a, v2 b) {
    a = a - b;
    return a;
}

function v2 operator-(v2 a) {
    v2 result;
    result.x = -a.x;
    result.y = -a.y;

    return result;
}

function v2 operator*(v2 a, v2 b) {
    v2 result;
    result.x = (a.x * b.x);
    result.y = (a.y * b.y);

    return result;
}

function v2 operator*(v2 a, f32 b) {
    v2 result;
    result.x = (a.x * b);
    result.y = (a.y * b);

    return result;
}

function v2 operator*(f32 a, v2 b) {
    v2 result;
    result.x = (a * b.x);
    result.y = (a * b.y);

    return result;
}

function v2 &operator*=(v2 &a, v2 b) {
    a = a * b;
    return a;
}

function v2 &operator*=(v2 &a, f32 b) {
    a = a * b;
    return a;
}

function v2 operator/(v2 a, v2 b) {
    v2 result;
    result.x = (a.x / b.x);
    result.y = (a.y / b.y);

    return result;
}

function v2 operator/(v2 a, f32 b) {
    v2 result;
    result.x = (a.x / b);
    result.y = (a.y / b);

    return result;
}

function v2 &operator/=(v2 &a, v2 b) {
    a = a / b;
    return a;
}

function v2 &operator/=(v2 &a, f32 b) {
    a = a / b;
    return a;
}

// v2 functions
//
function v2 V2(f32 x, f32 y) {
    v2 result = { x, y };
    return result;
}

function v2 V2(v2u v) {
    v2 result = { cast(f32) v.x, cast(f32) v.y };
    return result;
}

function v2 V2(v2s v) {
    v2 result = { cast(f32) v.x, cast(f32) v.y };
    return result;
}

function f32 Dot(v2 a, v2 b) {
    f32 result = (a.x * b.x) + (a.y * b.y);
    return result;
}

function f32 Length(v2 a) {
    f32 result = Sqrt(Dot(a, a));
    return result;
}

function v2 Noz(v2 a) {
    v2 result = V2(0, 0);

    f32 len = Length(a);
    if (!IsZero(len)) {
        result = (a / len);
    }

    return result;
}

function v2 Perp(v2 a) {
    v2 result;
    result.x = -a.y;
    result.y =  a.x;

    return result;
}

function v2 Lerp(v2 a, v2 b, f32 t) {
    v2 result = ((1.0f - t) * a) + (t * b);
    return result;
}

function v2 Arm2(f32 sin, f32 cos) {
    v2 result;
    result.x = sin;
    result.y = cos;

    return result;
}

function v2 Arm2(f32 angle) {
    v2 result;
    result.x = Sin(angle);
    result.y = Cos(angle);

    return result;
}

function v2 Rotate(v2 v, v2 a) {
    v2 result;
    result.x = (v.x * a.y) - (v.y * a.x);
    result.y = (v.x * a.x) + (v.y * a.y);

    return result;
}

function v2 Rotate(v2 v, f32 a) {
    v2 result = Rotate(v, Arm2(a));
    return result;
}

function v2 Minimum(v2 a, v2 b) {
    v2 result;
    result.x = Min(a.x, b.x);
    result.y = Min(a.y, b.y);

    return result;
}

function v2 Maximum(v2 a, v2 b) {
    v2 result;
    result.x = Max(a.x, b.x);
    result.y = Max(a.y, b.y);

    return result;
}

// v3 operator overloads
//
function v3 operator+(v3 a, v3 b) {
    v3 result;
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);
    result.z = (a.z + b.z);

    return result;
}

function v3 &operator+=(v3 &a, v3 b) {
    a = a + b;
    return a;
}

function v3 operator-(v3 a, v3 b) {
    v3 result;
    result.x = (a.x - b.x);
    result.y = (a.y - b.y);
    result.z = (a.z - b.z);

    return result;
}

function v3 &operator-=(v3 &a, v3 b) {
    a = a - b;
    return a;
}

function v3 operator-(v3 a) {
    v3 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

function v3 operator*(v3 a, v3 b) {
    v3 result;
    result.x = (a.x * b.x);
    result.y = (a.y * b.y);
    result.z = (a.z * b.z);

    return result;
}

function v3 operator*(v3 a, f32 b) {
    v3 result;
    result.x = (a.x * b);
    result.y = (a.y * b);
    result.z = (a.z * b);

    return result;
}

function v3 operator*(f32 a, v3 b) {
    v3 result;
    result.x = (a * b.x);
    result.y = (a * b.y);
    result.z = (a * b.z);

    return result;
}

function v3 &operator*=(v3 &a, v3 b) {
    a = a * b;
    return a;
}

function v3 &operator*=(v3 &a, f32 b) {
    a = a * b;
    return a;
}

function v3 operator/(v3 a, v3 b) {
    v3 result;
    result.x = (a.x / b.x);
    result.y = (a.y / b.y);
    result.z = (a.z / b.z);

    return result;
}

function v3 operator/(v3 a, f32 b) {
    v3 result;
    result.x = (a.x / b);
    result.y = (a.y / b);
    result.z = (a.z / b);

    return result;
}

function v3 &operator/=(v3 &a, v3 b) {
    a = a / b;
    return a;
}

function v3 &operator/=(v3 &a, f32 b) {
    a = a / b;
    return a;
}

// v3 functions
//
function v3 V3(f32 x, f32 y, f32 z) {
    v3 result = { x, y, z };
    return result;
}

function v3 V3(v2 xy, f32 z) {
    v3 result = { xy.x, xy.y, z };
    return result;
}

function f32 Dot(v3 a, v3 b) {
    f32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return result;
}

function f32 Length(v3 a) {
    f32 result = Sqrt(Dot(a, a));
    return result;
}

function v3 Noz(v3 a) {
    v3 result = V3(0, 0, 0);

    f32 len = Length(a);
    if (!IsZero(len)) {
        result = (a / len);
    }

    return result;
}

function v3 Cross(v3 a, v3 b) {
    v3 result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);

    return result;
}

function v3 Lerp(v3 a, v3 b, f32 t) {
    v3 result = ((1.0f - t) * a) + (t * b);
    return result;
}

function v3 Minimum(v3 a, v3 b) {
    v3 result;
    result.x = Min(a.x, b.x);
    result.y = Min(a.y, b.y);
    result.z = Min(a.z, b.z);

    return result;
}

function v3 Maximum(v3 a, v3 b) {
    v3 result;
    result.x = Max(a.x, b.x);
    result.y = Max(a.y, b.y);
    result.z = Max(a.z, b.z);

    return result;
}

// v4 operator overloads
//
function v4 operator+(v4 a, v4 b) {
    v4 result;
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);
    result.z = (a.z + b.z);
    result.w = (a.w + b.w);

    return result;
}

function v4 &operator+=(v4 &a, v4 b) {
    a = a + b;
    return a;
}

function v4 operator-(v4 a, v4 b) {
    v4 result;
    result.x = (a.x - b.x);
    result.y = (a.y - b.y);
    result.z = (a.z - b.z);
    result.w = (a.w - b.w);

    return result;
}

function v4 &operator-=(v4 &a, v4 b) {
    a = a - b;
    return a;
}

function v4 operator-(v4 a) {
    v4 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;

    return result;
}

function v4 operator*(v4 a, v4 b) {
    v4 result;
    result.x = (a.x * b.x);
    result.y = (a.y * b.y);
    result.z = (a.z * b.z);
    result.w = (a.w * b.w);

    return result;
}

function v4 operator*(v4 a, f32 b) {
    v4 result;
    result.x = (a.x * b);
    result.y = (a.y * b);
    result.z = (a.z * b);
    result.w = (a.w * b);

    return result;
}

function v4 operator*(f32 a, v4 b) {
    v4 result;
    result.x = (a * b.x);
    result.y = (a * b.y);
    result.z = (a * b.z);
    result.w = (a * b.w);

    return result;
}

function v4 &operator*=(v4 &a, v4 b) {
    a = a * b;
    return a;
}

function v4 &operator*=(v4 &a, f32 b) {
    a = a * b;
    return a;
}

function v4 operator/(v4 a, v4 b) {
    v4 result;
    result.x = (a.x / b.x);
    result.y = (a.y / b.y);
    result.z = (a.z / b.z);
    result.w = (a.w / b.w);

    return result;
}

function v4 operator/(v4 a, f32 b) {
    v4 result;
    result.x = (a.x / b);
    result.y = (a.y / b);
    result.z = (a.z / b);
    result.w = (a.w / b);

    return result;
}

function v4 &operator/=(v4 &a, v4 b) {
    a = a / b;
    return a;
}

function v4 &operator/=(v4 &a, f32 b) {
    a = a / b;
    return a;
}

// v4 functions
//
function v4 V4(f32 x, f32 y, f32 z, f32 w) {
    v4 result = { x, y, z, w };
    return result;
}

function v4 V4(v3 xyz, f32 w) {
    v4 result = { xyz.x, xyz.y, xyz.z, w };
    return result;
}

function f32 Dot(v4 a, v4 b) {
    f32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
    return result;
}

function f32 Length(v4 a) {
    f32 result = Sqrt(Dot(a, a));
    return result;
}

function v4 Noz(v4 a) {
    v4 result = V4(0, 0, 0, 0);

    f32 len = Length(a);
    if (!IsZero(len)) {
        result = (a / len);
    }

    return result;
}

function v4 Lerp(v4 a, v4 b, f32 t) {
    v4 result = ((1.0f - t) * a) + (t * b);
    return result;
}

function v4 Minimum(v4 a, v4 b) {
    v4 result;
    result.x = Min(a.x, b.x);
    result.y = Min(a.y, b.y);
    result.z = Min(a.z, b.z);
    result.w = Min(a.w, b.w);

    return result;
}

function v4 Maximum(v4 a, v4 b) {
    v4 result;
    result.x = Max(a.x, b.x);
    result.y = Max(a.y, b.y);
    result.z = Max(a.z, b.z);
    result.w = Max(a.w, b.w);

    return result;
}

// m4x4 operator overloads
//
function m4x4 operator*(m4x4 a, m4x4 b) {
    m4x4 result;
    for (u32 r = 0; r < 4; ++r) {
        for (u32 c = 0; c < 4; ++c) {
            result.m[r][c] =
                (a.m[r][0] * b.m[0][c]) + (a.m[r][1] * b.m[1][c]) +
                (a.m[r][2] * b.m[2][c]) + (a.m[r][3] * b.m[3][c]);
        }
    }

    return result;
}

function v4 operator*(m4x4 a, v4 b) {
    v4 result = Transform(a, b);
    return result;
}

function v3 operator*(m4x4 a, v3 b) {
    v4 point = Transform(a, V4(b, 1.0f));

    v3 result = point.xyz;
    return result;
}

// m4x4 functions
//
function m4x4 Identity() {
    m4x4 result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    return result;
}

function v4 Transform(m4x4 a, v4 b) {
    v4 result;
    result.x = Dot(b, a.r[0]);
    result.y = Dot(b, a.r[1]);
    result.z = Dot(b, a.r[2]);
    result.w = Dot(b, a.r[3]);

    return result;
}

function m4x4 Translate(m4x4 a, v3 b) {
    m4x4 result = a;
    result.m[0][3] += b.x;
    result.m[1][3] += b.y;
    result.m[2][3] += b.z;

    return result;
}

function m4x4 XRotation(f32 angle) {
    f32 s = Sin(angle);
    f32 c = Cos(angle);

    m4x4 result = {
        1, 0,  0, 0,
        0, c, -s, 0,
        0, s,  c, 0,
        0, 0,  0, 0
    };

    return result;
}

function m4x4 YRotation(f32 angle) {
    f32 s = Sin(angle);
    f32 c = Cos(angle);

    m4x4 result = {
         c, 0,  s, 0,
         0, 1,  0, 0,
        -s, 0,  c, 0,
         0, 0,  0, 0
    };

    return result;
}

function m4x4 ZRotation(f32 angle) {
    f32 s = Sin(angle);
    f32 c = Cos(angle);

    m4x4 result = {
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 0
    };

    return result;
}

function v3 GetRow(m4x4 a, u32 row) {
    v3 result = a.r[row].xyz;
    return result;
}

function v3 GetColumn(m4x4 a, u32 col) {
    v3 result;
    result.x = a.m[0][col];
    result.y = a.m[1][col];
    result.z = a.m[2][col];

    return result;
}

function m4x4 Rows3x3(v3 x, v3 y, v3 z) {
    m4x4 result = {
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
        z.x, z.y, z.z, 0,
        0,   0,   0,   1
    };

    return result;
}

function m4x4 Columns3x3(v3 x, v3 y, v3 z) {
    m4x4 result = {
        x.x, y.x, z.x, 0,
        x.y, y.y, z.y, 0,
        x.z, y.z, z.z, 0,
        0,   0,   0,   1
    };

    return result;
}

function m4x4_inv OrthographicProjection(f32 aspect, f32 near_plane, f32 far_plane) {
    f32 a = 1.0;
    f32 b = -aspect;

    f32 c = 2.0f / (near_plane - far_plane);
    f32 d = (near_plane + far_plane) / (near_plane - far_plane);

    m4x4_inv result = {
        // Forward
        //
        {
            a, 0, 0, 0,
            0, b, 0, 0,
            0, 0, c, d,
            0, 0, 0, 1
        },

        // Inverse
        //
        {
            (1.0f / a),  0,         0,           0,
             0,         (1.0f / b), 0,           0,
             0,          0,        (1.0f / c), -(d / c),
             0,          0,         0,           1
        }
    };

    return result;
}

function m4x4_inv PerspectiveProjection(f32 fov, f32 aspect, f32 near_plane, f32 far_plane) {
    f32 focal_len = (1.0f / Tan(0.5f * fov));

    f32 a = -(focal_len / aspect);
    f32 b = (focal_len);

    f32 c = -(near_plane + far_plane) / (far_plane - near_plane);
    f32 d = -(2.0f * near_plane * far_plane) / (far_plane - near_plane);

    m4x4_inv result = {
        // Forward
        //
        {
            a, 0,  0, 0,
            0, b,  0, 0,
            0, 0,  c, d,
            0, 0, -1, 0
        },

        // Inverse
        //
        {
            (1.0f / a),  0,          0,          0,
            0,          (1.0f / b),  0,          0,
            0,           0,          0,         -1,
            0,           0,         (1.0f / d), (c /d)
        }
    };

    return result;
}

function m4x4_inv CameraTransform(v3 x, v3 y, v3 z, v3 p) {
    m4x4_inv result;

    result.forward = Rows3x3(x, y, z);

    v3 txp = -(result.forward * p);
    result.forward = Translate(result.forward, txp);

    v3 ix = x * (1.0f / Dot(x, x));
    v3 iy = y * (1.0f / Dot(y, y));
    v3 iz = z * (1.0f / Dot(z, z));
    v3 ip = V3((txp.x * ix.x) + (txp.y * iy.x) + (txp.z * iz.x),
               (txp.x * ix.y) + (txp.y * iy.y) + (txp.z * iz.y),
               (txp.x * ix.z) + (txp.y * iy.z) + (txp.z * iz.z));

    result.inverse = Columns3x3(ix, iy, iz);
    result.inverse = Translate(result.inverse, -ip);

    return result;
}

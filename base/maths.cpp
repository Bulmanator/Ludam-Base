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
    f64 result = ((1.0f - t) * a) + (t * b);
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

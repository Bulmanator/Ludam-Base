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
    f32 result = (1.0f - t) * a + (t * b);
    return result;
}

function f64 Lerp(f64 a, f64 b, f64 t) {
    f64 result = (1.0f - t) * a + (t * b);
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

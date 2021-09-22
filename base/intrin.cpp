#if COMPILER_CL
#    include <intrin.h>
#elif (COMPILER_CLANG || COMPILER_GCC)
#    include <x86intrin.h>
#endif

function f32 Sqrt(f32 x) {
    f32 result = _mm_cvtss_f32(_mm_sqrt_ps(_mm_set1_ps(x)));
    return result;
}

function f32 InvSqrt(f32 x) {
    f32 result = _mm_cvtss_f32(_mm_rsqrt_ps(_mm_set1_ps(x)));
    return result;
}

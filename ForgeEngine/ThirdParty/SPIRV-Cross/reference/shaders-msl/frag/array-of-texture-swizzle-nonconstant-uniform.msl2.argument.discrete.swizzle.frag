#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

enum class spvSwizzle : uint
{
    none = 0,
    zero,
    one,
    red,
    green,
    blue,
    alpha
};

template<typename T>
inline T spvGetSwizzle(vec<T, 4> x, T c, spvSwizzle s)
{
    switch (s)
    {
        case spvSwizzle::none:
            return c;
        case spvSwizzle::zero:
            return 0;
        case spvSwizzle::one:
            return 1;
        case spvSwizzle::red:
            return x.r;
        case spvSwizzle::green:
            return x.g;
        case spvSwizzle::blue:
            return x.b;
        case spvSwizzle::alpha:
            return x.a;
    }
}

// Wrapper function that swizzles texture samples and fetches.
template<typename T>
inline vec<T, 4> spvTextureSwizzle(vec<T, 4> x, uint s)
{
    if (!s)
        return x;
    return vec<T, 4>(spvGetSwizzle(x, x.r, spvSwizzle((s >> 0) & 0xFF)), spvGetSwizzle(x, x.g, spvSwizzle((s >> 8) & 0xFF)), spvGetSwizzle(x, x.b, spvSwizzle((s >> 16) & 0xFF)), spvGetSwizzle(x, x.a, spvSwizzle((s >> 24) & 0xFF)));
}

template<typename T>
inline T spvTextureSwizzle(T x, uint s)
{
    return spvTextureSwizzle(vec<T, 4>(x, 0, 0, 1), s).x;
}

struct UBO
{
    uint index;
};

struct UBO2
{
    uint index2;
};

struct spvDescriptorSetBuffer0
{
    array<texture2d<float>, 4> uSampler [[id(0)]];
    array<sampler, 4> uSamplerSmplr [[id(4)]];
    constant UBO* uUBO [[id(8)]];
    constant UBO2* m_50 [[id(9)]];
    constant uint* spvSwizzleConstants [[id(10)]];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vUV [[user(locn0)]];
};

static inline __attribute__((always_inline))
float4 sample_in_func(constant array<texture2d<float>, 4>& uSampler, constant array<sampler, 4>& uSamplerSmplr, constant uint* uSamplerSwzl, constant UBO& uUBO, thread float2& vUV)
{
    return spvTextureSwizzle(uSampler[uUBO.index].sample(uSamplerSmplr[uUBO.index], vUV), uSamplerSwzl[uUBO.index]);
}

static inline __attribute__((always_inline))
float4 sample_single_in_func(texture2d<float> s, sampler sSmplr, constant uint& sSwzl, thread float2& vUV)
{
    return spvTextureSwizzle(s.sample(sSmplr, vUV), sSwzl);
}

fragment main0_out main0(main0_in in [[stage_in]], constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]], constant uint* spvSwizzleConstants [[buffer(30)]])
{
    main0_out out = {};
    constant uint* spvDescriptorSet0_uSamplerSwzl = &spvDescriptorSet0.spvSwizzleConstants[0];
    out.FragColor = sample_in_func(spvDescriptorSet0.uSampler, spvDescriptorSet0.uSamplerSmplr, spvDescriptorSet0_uSamplerSwzl, (*spvDescriptorSet0.uUBO), in.vUV);
    out.FragColor += sample_single_in_func(spvDescriptorSet0.uSampler[(*spvDescriptorSet0.m_50).index2], spvDescriptorSet0.uSamplerSmplr[(*spvDescriptorSet0.m_50).index2], spvDescriptorSet0_uSamplerSwzl[(*spvDescriptorSet0.m_50).index2], in.vUV);
    return out;
}


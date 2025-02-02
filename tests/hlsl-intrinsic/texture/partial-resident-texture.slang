//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-slang -compute -shaderobj -output-using-type -use-dxil -profile cs_6_6 -dx12
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly -render-feature hardware-device -xslang -DVK

//TEST_INPUT: ubuffer(data=[2], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(size=4, content = one):name t2D_f32
Texture2D<float4> t2D_f32;

//TEST_INPUT:RWTexture2D(format=R32G32B32A32_FLOAT, size=4, content = one):name t2DRW_f32
RWTexture2D<float4> t2DRW_f32;

//TEST_INPUT: Texture2D(size=4, content = one):name t2DMS_f32
Texture2DMS<float4> t2DMS_f32;

//TEST_INPUT:ubuffer(data=[1 1 1 1]):name=iBuf
RWByteAddressBuffer iBuf;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;

//TEST_INPUT: Sampler:name samplerComparisonState
SamplerComparisonState samplerCmpState;

bool TEST_texture<T>(
    Texture2D<T> t2D,
    RWTexture2D<T> t2DRW,
    Texture2DMS<T> t2DMS)
    where T : ITexelElement, IArithmetic
{
    typealias TN = T;
    constexpr const int2 offset = int2(0, 0);
    uint status;
    float clamp = 0.0;
    float2 uv = float2(0.5f, 0.5f);
    float level = 0.0;
    float bias = 1.0;
    float2 grad = float2(0.5f, 0.5f);
    float compareValue = 0.0;

    int sampleIndex = 0;
    int2 iuv = int2(1, 1);
    int3 iuvs = int3(iuv, sampleIndex);

    return true
        // Sample
        && (status = 0, all(TN(T(1)) == t2D.Sample(samplerState, uv, offset, clamp, status))) && CheckAccessFullyMapped(status)
        && (status = 0, all(TN(T(1)) == t2D.SampleLevel(samplerState, uv, level, offset, status))) && CheckAccessFullyMapped(status)
        && (status = 0, all(TN(T(1)) == t2D.SampleBias(samplerState, uv, bias, offset, clamp, status))) && CheckAccessFullyMapped(status)
        && (status = 0, all(TN(T(1)) == t2D.SampleGrad(samplerState, uv, grad, grad, offset, clamp, status))) && CheckAccessFullyMapped(status)

        // Sample depth
        && (status = 0, all(1.0 == t2D.SampleCmp(samplerCmpState, uv, compareValue, offset, clamp, status))) && CheckAccessFullyMapped(status)
        && (status = 0, all(1.0 == t2D.SampleCmpLevel(samplerCmpState, uv, compareValue, level, offset, status))) && CheckAccessFullyMapped(status)
        && (status = 0, all(1.0 == t2D.SampleCmpLevelZero(samplerCmpState, uv, compareValue, offset, status))) && CheckAccessFullyMapped(status)

        // Sample MS
        && (status = 0, all(TN(T(1)) == t2DMS.Load(iuv, sampleIndex, offset, status))) && CheckAccessFullyMapped(status)

        // Load
        && (status = 0, all(TN(T(1)) == t2D.Load(iuvs, offset, status))) && CheckAccessFullyMapped(status)

#if !defined(VK)
        // Make sure CheckAccessFullyMapped can return false.
        // Resident code/status 0 is not guaranteed to translate to non-resident on VK, hence it is disabled.
        && (status = 0, !CheckAccessFullyMapped(status))

        && (status = 0, 2 == outputBuffer.Load(0, status)) && CheckAccessFullyMapped(status)
        && (status = 0, 1 == iBuf.Load(0, status)) && CheckAccessFullyMapped(status)
        && (status = 0, all(int2(1) == iBuf.Load2(0, status))) && CheckAccessFullyMapped(status)
        && (status = 0, all(int3(1) == iBuf.Load3(0, status))) && CheckAccessFullyMapped(status)
        && (status = 0, all(int4(1) == iBuf.Load4(0, status))) && CheckAccessFullyMapped(status)
#endif
        ;
}

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    bool result = true
        && TEST_texture(t2D_f32, t2DRW_f32, t2DMS_f32)
        ;

    //CHK:1
    outputBuffer[0] = int(result);
}

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -emit-spirv-via-glsl -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -emit-spirv-directly -output-using-type -allow-glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -shaderobj -render-feature hardware-device -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -profile cs_6_6 -dx12 -use-dxil -shaderobj -render-feature hardware-device -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type -allow-glsl

// 16 bit variants are not supported by WGSL.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -xslang -DWGSL -output-using-type
// Debug info for inlining errors can be given out, so disable them for this test.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -g0 -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

bool verifyResult<T: IFloat> (T expected, T actual, T tolerance)
{
    return (expected - tolerance) <= actual && actual <= (expected + tolerance);
}

bool verifyResultVector<T: IFloat, let N: int>(vector<T, N> expected, vector<T, N> actual, T tolerance = T(0.01))
{
    bool isValid = true;
    for (int i = 0; i < N; ++i)
        isValid = isValid && verifyResult(expected[i], actual[i], tolerance);
    return isValid;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = 0U;

    //
    // Unorm4x8
    //
    float4 unorm4x8Expected = float4(0.777, 0.233, 0.931, 0.777);
    uint unorm4x8Packed = packFloatToUnorm4x8(unorm4x8Expected);
    float4 unorm4x8Actual = unpackUnorm4x8ToFloat(unorm4x8Packed);
    // BUF: 1
    outputBuffer[index++] = verifyResultVector(unorm4x8Expected, unorm4x8Actual);

    half4 unorm4x8HalfExpected = half4(0.123h, 0.456h, 0.789h, 0.321h);
    uint unorm4x8HalfPacked = packHalfToUnorm4x8(unorm4x8HalfExpected);
    half4 unorm4x8HalfActual = unpackUnorm4x8ToHalf(unorm4x8HalfPacked);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(unorm4x8HalfExpected, unorm4x8HalfActual);

    //
    // Snorm4x8
    //
    // float4 snorm4x8Expected = float4(-0.500, 0.250, -0.750, 0.999);
    float4 snorm4x8Expected = float4(-0.500, 0.0, -0.0, 0.0);
    uint snorm4x8Packed = packFloatToSnorm4x8(snorm4x8Expected);
    float4 snorm4x8Actual = unpackSnorm4x8ToFloat(snorm4x8Packed);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(snorm4x8Expected, snorm4x8Actual);

    half4 snorm4x8HalfExpected = half4(-0.333h, 0.666h, -1.000h, 0.500h);
    uint snorm4x8HalfPacked = packHalfToSnorm4x8(snorm4x8HalfExpected);
    half4 snorm4x8HalfActual = unpackSnorm4x8ToHalf(snorm4x8HalfPacked);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(snorm4x8HalfExpected, snorm4x8HalfActual);


    //
    // Unorm2x16
    //
    float2 unorm2x16Expected = float2(0.1234, 0.8765);
    uint unorm2x16Packed = packFloatToUnorm2x16(unorm2x16Expected);
    float2 unorm2x16Actual = unpackUnorm2x16ToFloat(unorm2x16Packed);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(unorm2x16Expected, unorm2x16Actual);

    half2 unorm2x16HalfExpected = half2(0.7777h, 0.7777h);
    uint unorm2x16HalfPacked = packHalfToUnorm2x16(unorm2x16HalfExpected);
    half2 unorm2x16HalfActual = unpackUnorm2x16ToHalf(unorm2x16HalfPacked);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(unorm2x16HalfExpected, unorm2x16HalfActual);

    //
    // Snorm2x16
    //
    float2 snorm2x16Expected = float2(-0.4444, 0.8888);
    uint snorm2x16Packed = packFloatToSnorm2x16(snorm2x16Expected);
    float2 snorm2x16Actual = unpackSnorm2x16ToFloat(snorm2x16Packed);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(snorm2x16Expected, snorm2x16Actual);

    half2 snorm2x16HalfExpected = half2(-0.9999h, 0.3333h);
    uint snorm2x16HalfPacked = packHalfToSnorm2x16(snorm2x16HalfExpected);
    half2 snorm2x16HalfActual = unpackSnorm2x16ToHalf(snorm2x16HalfPacked);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(snorm2x16HalfExpected, snorm2x16HalfActual);

    //
    // Half2x16
    //
    float2 half2x16Expected = float2(130.32, -12.12);
    uint half2x16Packed = packFloatToHalf2x16(half2x16Expected);
    float2 half2x16Actual = unpackHalf2x16ToFloat(half2x16Packed);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(half2x16Expected.x, half2x16Actual.x, 0.1);

    half2 half2x16HalfExpected = half2(-2.1111h, 4450.9999h);
    uint half2x16HalfPacked = packHalfToHalf2x16(half2x16HalfExpected);
    half2 half2x16HalfActual = unpackHalf2x16ToHalf(half2x16HalfPacked);
    // BUF-NEXT: 1
    outputBuffer[index++] = verifyResultVector(half2x16HalfExpected, half2x16HalfActual);
}


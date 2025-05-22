//struct ScreenDimensions
//{
//    uint x;
//    uint y;
//};
[[vk::image_format("bgra8")]]
RWTexture2D<float4> surfaceTexture : register(u0, space1);

//ConstantBuffer<ScreenDimensions> screenDimensions : register(t0, space0);
Texture2D baseColorTexture : register(t0, space0);
Texture2D shadowMapTexture : register(t1, space0);

[numthreads(1,1,1)]
void cs_main(uint2 dispatchThreadID: SV_DispatchThreadID)
{
    float4 result = baseColorTexture[dispatchThreadID.xy];
    result *= shadowMapTexture[dispatchThreadID.xy];
    surfaceTexture[dispatchThreadID.xy] = result;
    //surfaceTexture[dispatchThreadID.xy] = baseColorTexture[dispatchThreadID.xy];
    //surfaceTexture[dispatchThreadID.xy] *= shadowMapTexture[dispatchThreadID.xy];
}

RWTexture2D<float4> surfaceTexture : register(u0, space1);
Texture2D baseColorTexture : register(t0, space0);
Texture2D shadowMapTexture : register(t1, space0);

[numthreads(1,1,1)]
void cs_main(uint2 dispatchThreadID: SV_DispatchThreadID)
{
    uint textureWidth, textureHeight; //accumulatorTexture and inputTexture are assumed to be of the same size
    surfaceTexture.GetDimensions(textureWidth, textureHeight);
    surfaceTexture[dispatchThreadID.xy] = baseColorTexture[dispatchThreadID.xy];
    surfaceTexture[dispatchThreadID.xy] *= shadowMapTexture[dispatchThreadID.xy];
}

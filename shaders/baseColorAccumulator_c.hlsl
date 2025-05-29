#include "_definitions.hlsli"

struct TextureMasterInfo //struct DawnEngine::InfoBufferLayout
{
    float2 texcoord;
    uint materialId;
    uint PAD0;
};

struct TextureInputInfo
{
    uint stpId;
    uint PAD0;
    uint PAD1;
    uint PAD2;
};

RWTexture2D<float4> accumulatorTexture : register(u0, space0);
Texture2D<float2> texCoordTexture : register(t1, space0);
Texture2D<uint> textureIdTexture : register(t2, space0);
ConstantBuffer<TextureInputInfo> textureInputInfo : register(b0, space1);
Texture2D inputTexture : register(t1, space1);
SamplerState inputSamplerState : register(s2, space1);


[numthreads(1,1,1)]
void cs_main(uint2 dispatchThreadID: SV_DispatchThreadID)
{
    if (textureIdTexture[dispatchThreadID.xy] == textureInputInfo.stpId)
    {
        accumulatorTexture[dispatchThreadID.xy] = inputTexture.Sample(
            inputSamplerState, 
            texCoordTexture[dispatchThreadID.xy]
        );
    }
}
